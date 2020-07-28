/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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
#ifndef __MFLLMFB_PLATFORM_H__
#define __MFLLMFB_PLATFORM_H__

#if ((MFLL_MF_TAG_VERSION > 0) && (MFLL_MF_TAG_VERSION != 1))
#   error "MT6761 only supports MF_TAG_VERSION 1"
#endif

#ifndef LOG_TAG
#define LOG_TAG "MfllCore/Mfb"
#include <mtkcam3/feature/mfnr/MfllLog.h>
#endif

#include <custom/debug_exif/dbg_exif_param.h>

#include "MfllIspProfiles.h"

#include <mtkcam3/feature/mfnr/IMfllCore.h>

// platform dependent headers
#include <drv/isp_reg.h> // isp_reg_t

// property
#include <cutils/properties.h> // property_get_int32

// STL
#include <cassert> // std::assert

// Dynamic Tuning Support
//
// Set this macro to 1 to make MFNR control flow support dynamical tuning.
// User has to set property to force the register value he wants. The rule is
//
//  $ adb shell setprop {Stage}.{Register} {Value}
//
// where Stage is
//  bf:  Before Blend
//  sf:  Single Frame
//  mfb: Blending
//  af:  After Blend
//
// The regiser supports only registers in MF tag.
//
// E.g.
//
//  $ adb shell setprop sf.DIP_X_SEEE_CLIP_CTRL_1  0
//
// which may set register DIP_X_SEEE_CLIP_CTRL_1 to 0 in stage SF
#define MFLL_DYNAMIC_TUNING_SUPPORT        0


#if     MFLL_DYNAMIC_TUNING_SUPPORT
// check const char string length during compile time
constexpr int __length(const char* str)
{
    return *str ? 1 + __length(str + 1) : 0;
}

// cat property string: e.g.: "mfb.DIP_X_MFB_CON"
#define _DYNC_CAT_STR(REG, STAGE) #STAGE "." #REG

// expression to read property and assign to register
#define _DYNC_TUNE(DATA, REG, STAGE) \
do { \
    /* compile time to check if property string <= 31 characters */ \
    constexpr const int _str_length = __length(_DYNC_CAT_STR(REG, STAGE)); \
    static_assert(_str_length <= 31, "Some register cannot be tuned due to name is too long: " _DYNC_CAT_STR(REG,STAGE)); \
    \
    uint32_t _value = (uint32_t)::property_get_int32(_DYNC_CAT_STR(REG,STAGE), 0xFFFFFFFF); \
    if (_value != 0xFFFFFFFF) { \
        mfllLogD("[%s] dyncTune %s -> %x", #STAGE, #REG, _value); \
        DATA->REG.Raw = _value; \
    } \
} while (0)

#else
#define _DYNC_TUNE(DATA, REG, STAGE) do {} while(0)
#endif

namespace mfll
{
#if (MFLL_MF_TAG_VERSION > 0)
using namespace __namespace_mf(MFLL_MF_TAG_VERSION);
#endif

inline bool workaround_MFB_stage(
        void*    regs    __attribute__((unused))
        )
{
// {{{
/*
#ifdef WORKAROUND_MFB_STAGE
TODO
#endif
*/
 return false;
// }}}
}

inline void debug_pass2_registers(
        void*   regs    __attribute__((unused)),
        int     stage   __attribute__((unused))
        )
{
// {{{
/*
#ifdef __DEBUG
TODO
#endif
*/
// }}}
}

inline void dump_exif_info(
        IMfllCore*  c,
        void*       regs,
        int         stage)
{
//{{{
    isp_reg_t* t = reinterpret_cast<isp_reg_t*>(regs);
    
    if (c == NULL || t == NULL)
        return;
    
#if (MFLL_MF_TAG_VERSION > 0)
#   define __VAR(tag)                       (t->tag.Raw)     // make value
#   define __MAKE_TAG(prefix, tag)          prefix##tag      // make MF_TAG_##
    
    /* Write register to MF tag and dynamical tuning */
#   define __ASSIGN(prefix, stage, tag) \
            do { \
                _DYNC_TUNE(t, tag, stage); \
                c->updateExifInfo((unsigned int)__MAKE_TAG(prefix, tag), (uint32_t)__VAR(tag)); \
            } while(0)
    /* Dynamical tuning only */
#   define __TUNE__(prefix, stage, tag) \
            do { _DYNC_TUNE(t, tag, stage); } while(0)
    
#else
#   define __ASSIGN(prefix, stage, tag) do {} while(0)
#   define __TUNE__(prefix, stage, tag) do {} while(0)
#endif

// Registers
// {{{
// ANR
#define _D_WRITE_ANR(prefix, stage) \
        __ASSIGN(prefix, stage, CAM_ANR_CON1               ); \
        __ASSIGN(prefix, stage, CAM_ANR_CON2               ); \
        __ASSIGN(prefix, stage, CAM_ANR_YAD1               ); \
        __ASSIGN(prefix, stage, CAM_ANR_YAD2               ); \
        __ASSIGN(prefix, stage, CAM_ANR_Y4LUT1             ); \
        __ASSIGN(prefix, stage, CAM_ANR_Y4LUT2             ); \
        __ASSIGN(prefix, stage, CAM_ANR_Y4LUT3             ); \
        __ASSIGN(prefix, stage, CAM_ANR_C4LUT1             ); \
        __ASSIGN(prefix, stage, CAM_ANR_C4LUT2             ); \
        __ASSIGN(prefix, stage, CAM_ANR_C4LUT3             ); \
        __ASSIGN(prefix, stage, CAM_ANR_A4LUT2             ); \
        __ASSIGN(prefix, stage, CAM_ANR_A4LUT3             ); \
        __ASSIGN(prefix, stage, CAM_ANR_L4LUT1             ); \
        __ASSIGN(prefix, stage, CAM_ANR_L4LUT2             ); \
        __ASSIGN(prefix, stage, CAM_ANR_L4LUT3             ); \
        __ASSIGN(prefix, stage, CAM_ANR_PTY                ); \
        __ASSIGN(prefix, stage, CAM_ANR_CAD                ); \
        __ASSIGN(prefix, stage, CAM_ANR_PTC                ); \
        __ASSIGN(prefix, stage, CAM_ANR_LCE                ); \
        __ASSIGN(prefix, stage, CAM_ANR_MED1               ); \
        __ASSIGN(prefix, stage, CAM_ANR_MED2               ); \
        __ASSIGN(prefix, stage, CAM_ANR_MED3               ); \
        __ASSIGN(prefix, stage, CAM_ANR_MED4               ); \
        __ASSIGN(prefix, stage, CAM_ANR_HP1                ); \
        __ASSIGN(prefix, stage, CAM_ANR_HP2                ); \
        __ASSIGN(prefix, stage, CAM_ANR_HP3                ); \
        __ASSIGN(prefix, stage, CAM_ANR_ACT1               ); \
        __ASSIGN(prefix, stage, CAM_ANR_ACT2               ); \
        __ASSIGN(prefix, stage, CAM_ANR_ACT3               ); \
        __ASSIGN(prefix, stage, CAM_ANR_ACTY               ); \
        __ASSIGN(prefix, stage, CAM_ANR_ACTC               ); \


// ANR2
#define _D_WRITE_ANR2(prefix, stage) \
        __ASSIGN(prefix, stage, CAM_ANR2_CON1              ); \
        __ASSIGN(prefix, stage, CAM_ANR2_CON2              ); \
        __ASSIGN(prefix, stage, CAM_ANR2_YAD1              ); \
        __ASSIGN(prefix, stage, CAM_ANR2_Y4LUT1            ); \
        __ASSIGN(prefix, stage, CAM_ANR2_Y4LUT2            ); \
        __ASSIGN(prefix, stage, CAM_ANR2_Y4LUT3            ); \
        __ASSIGN(prefix, stage, CAM_ANR2_L4LUT1            ); \
        __ASSIGN(prefix, stage, CAM_ANR2_L4LUT2            ); \
        __ASSIGN(prefix, stage, CAM_ANR2_L4LUT3            ); \
        __ASSIGN(prefix, stage, CAM_ANR2_CAD               ); \
        __ASSIGN(prefix, stage, CAM_ANR2_PTC               ); \
        __ASSIGN(prefix, stage, CAM_ANR2_LCE               ); \
        __ASSIGN(prefix, stage, CAM_ANR2_MED1              ); \
        __ASSIGN(prefix, stage, CAM_ANR2_MED2              ); \
        __ASSIGN(prefix, stage, CAM_ANR2_MED3              ); \
        __ASSIGN(prefix, stage, CAM_ANR2_MED4              ); \
        __ASSIGN(prefix, stage, CAM_ANR2_ACTY              ); \
        __ASSIGN(prefix, stage, CAM_ANR2_ACTC              ); \


// SEEE
#define _D_WRITE_SEEE(prefix, stage) \
        __ASSIGN(prefix, stage, CAM_SEEE_SRK_CTRL          ); \
        __ASSIGN(prefix, stage, CAM_SEEE_CLIP_CTRL         ); \
        __ASSIGN(prefix, stage, CAM_SEEE_FLT_CTRL_1        ); \
        __ASSIGN(prefix, stage, CAM_SEEE_FLT_CTRL_2        ); \
        __ASSIGN(prefix, stage, CAM_SEEE_GLUT_CTRL_01      ); \
        __ASSIGN(prefix, stage, CAM_SEEE_GLUT_CTRL_02      ); \
        __ASSIGN(prefix, stage, CAM_SEEE_GLUT_CTRL_03      ); \
        __ASSIGN(prefix, stage, CAM_SEEE_GLUT_CTRL_04      ); \
        __ASSIGN(prefix, stage, CAM_SEEE_GLUT_CTRL_05      ); \
        __ASSIGN(prefix, stage, CAM_SEEE_GLUT_CTRL_06      ); \
        __ASSIGN(prefix, stage, CAM_SEEE_EDTR_CTRL         ); \
        __ASSIGN(prefix, stage, CAM_SEEE_GLUT_CTRL_07      ); \
        __ASSIGN(prefix, stage, CAM_SEEE_GLUT_CTRL_08      ); \
        __ASSIGN(prefix, stage, CAM_SEEE_GLUT_CTRL_09      ); \
        __ASSIGN(prefix, stage, CAM_SEEE_GLUT_CTRL_10      ); \
        __ASSIGN(prefix, stage, CAM_SEEE_GLUT_CTRL_11      ); \
        __ASSIGN(prefix, stage, CAM_SEEE_OUT_EDGE_CTRL     ); \
        __ASSIGN(prefix, stage, CAM_SEEE_SE_Y_CTRL         ); \
        __ASSIGN(prefix, stage, CAM_SEEE_SE_EDGE_CTRL_1    ); \
        __ASSIGN(prefix, stage, CAM_SEEE_SE_EDGE_CTRL_2    ); \
        __ASSIGN(prefix, stage, CAM_SEEE_SE_EDGE_CTRL_3    ); \
        __ASSIGN(prefix, stage, CAM_SEEE_SE_SPECL_CTRL     ); \
        __ASSIGN(prefix, stage, CAM_SEEE_SE_CORE_CTRL_1    ); \
        __ASSIGN(prefix, stage, CAM_SEEE_SE_CORE_CTRL_2    ); \


// MFB
#define _D_WRITE_MFB(prefix, stage) \
        __ASSIGN(prefix, stage, CAM_MFB_LL_CON2            ); \
        __ASSIGN(prefix, stage, CAM_MFB_LL_CON3            ); \
        __ASSIGN(prefix, stage, CAM_MFB_LL_CON4            ); \
        __ASSIGN(prefix, stage, CAM_MFB_LL_CON5            ); \
        __ASSIGN(prefix, stage, CAM_MFB_LL_CON6            ); \


// MIX3
#define _D_WRITE_MIX3(prefix, stage) \
        __ASSIGN(prefix, stage, CAM_MIX3_CTRL_0            ); \
        __ASSIGN(prefix, stage, CAM_MIX3_CTRL_1            ); \
        __ASSIGN(prefix, stage, CAM_MIX3_SPARE             ); \

// }}} Registers


   switch (stage) {
    case STAGE_BASE_YUV:
#define _D_PREFIX   MF_TAG_BEFORE_MFB_
        _D_WRITE_ANR    (_D_PREFIX, bf);
        _D_WRITE_ANR2   (_D_PREFIX, bf);
        _D_WRITE_SEEE   (_D_PREFIX, bf);
#undef  _D_PREFIX
        break;

    case STAGE_GOLDEN_YUV:
#define _D_PREFIX   MF_TAG_BASE_
        _D_WRITE_ANR    (_D_PREFIX, sf);
        _D_WRITE_ANR2   (_D_PREFIX, sf);
        _D_WRITE_SEEE   (_D_PREFIX, sf);
#undef  _D_PREFIX
        break;

    case STAGE_MFB:
#define _D_PREFIX   MF_TAG_IN_MFB_
        _D_WRITE_MFB    (_D_PREFIX, mfb);
#undef  _D_PREFIX
        break;

    case STAGE_MIX:
#define _D_PREFIX   MF_TAG_AFTER_MFB_
        _D_WRITE_ANR    (_D_PREFIX, af);
        _D_WRITE_ANR2   (_D_PREFIX, af);
        _D_WRITE_SEEE   (_D_PREFIX, af);
        _D_WRITE_MFB    (_D_PREFIX, af);
        _D_WRITE_MIX3   (_D_PREFIX, af);
#undef  _D_PREFIX
        break;
    }
//}}}
}

inline EIspProfile_T get_isp_profile(
        const eMfllIspProfile&  p,
        MfllMode                mode
        )
{
// {{{
    switch (p) {
    // bfbld
    case eMfllIspProfile_BeforeBlend:
    case eMfllIspProfile_BeforeBlend_Swnr:
    case eMfllIspProfile_BeforeBlend_Zsd:
    case eMfllIspProfile_BeforeBlend_Zsd_Swnr:
        switch (mode) {
            case MfllMode_NormalMfhr:
            case MfllMode_ZsdMfhr:
                return EIspProfile_N3D_MFHR_Before_Blend;
        default:;
        }
        return EIspProfile_MFNR_Before_Blend; // FIXME: 2017/4/6, workaround for ISPTuning get default index fail

    // single
    case eMfllIspProfile_Single:
    case eMfllIspProfile_Single_Swnr:
    case eMfllIspProfile_Single_Zsd:
    case eMfllIspProfile_Single_Zsd_Swnr:
        switch (mode) {
            case MfllMode_NormalMfhr:
            case MfllMode_ZsdMfhr:
                return EIspProfile_N3D_MFHR_Single;
        default:;
        }
        return EIspProfile_MFNR_Single;

    // mfb
    case eMfllIspProfile_Mfb:
    case eMfllIspProfile_Mfb_Swnr:
    case eMfllIspProfile_Mfb_Zsd:
    case eMfllIspProfile_Mfb_Zsd_Swnr:
        switch (mode) {
            case MfllMode_NormalMfhr:
            case MfllMode_ZsdMfhr:
                mfllLogD("%s: EIspProfile_N3D_MFHR_MFB", __FUNCTION__);
                return EIspProfile_N3D_MFHR_MFB;
            case MfllMode_ZhdrNormalMfll:
            case MfllMode_ZhdrNormalAis:
            case MfllMode_ZhdrZsdMfll:
            case MfllMode_ZhdrZsdAis:
                mfllLogD("%s: EIspProfile_zHDR_Capture_MFNR_MFB", __FUNCTION__);
                return EIspProfile_zHDR_Capture_MFNR_MFB;
            case MfllMode_AutoZhdrNormalMfll:
            case MfllMode_AutoZhdrNormalAis:
            case MfllMode_AutoZhdrZsdMfll:
            case MfllMode_AutoZhdrZsdAis:
                mfllLogD("%s: EIspProfile_Auto_zHDR_Capture_MFNR_MFB", __FUNCTION__);
                return EIspProfile_zHDR_Capture_MFNR_MFB;
        default:;
        }
        mfllLogD("%s: default EIspProfile_MFNR_MFB", __FUNCTION__);
        return EIspProfile_MFNR_MFB;

    // mix (after blend)
    case eMfllIspProfile_AfterBlend:
    case eMfllIspProfile_AfterBlend_Swnr:
    case eMfllIspProfile_AfterBlend_Zsd:
    case eMfllIspProfile_AfterBlend_Zsd_Swnr:
        switch (mode) {
            case MfllMode_NormalMfhr:
            case MfllMode_ZsdMfhr:
                mfllLogD("%s: EIspProfile_N3D_MFHR_After_Blend", __FUNCTION__);
                return EIspProfile_N3D_MFHR_After_Blend;
            case MfllMode_ZhdrNormalMfll:
            case MfllMode_ZhdrNormalAis:
            case MfllMode_ZhdrZsdMfll:
            case MfllMode_ZhdrZsdAis:
                mfllLogD("%s: EIspProfile_zHDR_Capture_MFNR_After_Blend", __FUNCTION__);
                return EIspProfile_zHDR_Capture_MFNR_After_Blend;
            case MfllMode_AutoZhdrNormalMfll:
            case MfllMode_AutoZhdrNormalAis:
            case MfllMode_AutoZhdrZsdMfll:
            case MfllMode_AutoZhdrZsdAis:
                mfllLogD("%s: EIspProfile_Auto_zHDR_Capture_MFNR_After_Blend", __FUNCTION__);
                return EIspProfile_zHDR_Capture_MFNR_After_Blend;
        default:;
        }
        mfllLogD("%s: default EIspProfile_MFNR_After_Blend", __FUNCTION__);
        return EIspProfile_MFNR_After_Blend;

    default:
        mfllLogD("%s: error MFLL_ISP_PROFILE_ERROR", __FUNCTION__);
        return static_cast<EIspProfile_T>(MFLL_ISP_PROFILE_ERROR);
    }
// }}}
}

// cfg is in/out
inline bool refine_register(MfbPlatformConfig& cfg)
{
    // {{{
    // check stage first
    if (cfg.find(ePLATFORMCFG_STAGE) == cfg.end()) {
        mfllLogE("%s: undefined stage", __FUNCTION__);
        return false;
    }

    switch (cfg[ePLATFORMCFG_STAGE]) {
    case STAGE_MFB:{
        const size_t index = cfg[ePLATFORMCFG_INDEX];
        const bool bFullMc = (cfg[ePLATFORMCFG_FULL_SIZE_MC] != 0);
        /* TODO */
    } break; // STAGE_MFB

    case STAGE_MIX: {
         /* TODO */
    } break; // STAGE_MIX

    default:;
    }

    cfg[ePLATFORMCFG_REFINE_OK] = 1;
    return true;
    // }}}
}

/**
 *  Describe the pass 2 port ID
 */
#define PASS2_CROPPING_ID_CRZ       1
#define PASS2_CROPPING_ID_WDMAO     2
#define PASS2_CROPPING_ID_WROTO     3
/******************************************************************************
 *  RAW2YUV
 *****************************************************************************/
#define RAW2YUV_PORT_IN             PORT_IMGI
#define RAW2YUV_PORT_LCE_IN         PORT_LCEI
#define RAW2YUV_PORT_PBC_IN         PORT_DMGI

// half size MC
#define RAW2YUV_PORT_OUT            PORT_WDMAO

// full size MC
#define RAW2YUV_PORT_OUT_NO_CRZ     PORT_IMG3O  // using IMG3O for the best quality and bit true
#define RAW2YUV_PORT_OUT2           PORT_WROTO
#define RAW2YUV_PORT_OUT3           PORT_WDMAO

/* Cropping group ID is related port ID, notice this ... */
#define RAW2YUV_GID_OUT             PASS2_CROPPING_ID_WDMAO
#define RAW2YUV_GID_OUT2            PASS2_CROPPING_ID_WROTO
#define RAW2YUV_GID_OUT3            PASS2_CROPPING_ID_WDMAO

#define MIX_MAIN_GID_OUT            PASS2_CROPPING_ID_WDMAO
#define MIX_THUMBNAIL_GID_OUT       PASS2_CROPPING_ID_WROTO

/******************************************************************************
 *  MFB
 *****************************************************************************/
/* port */
#define MFB_PORT_IN_BASE_FRAME      PORT_IMGI
#define MFB_PORT_IN_REF_FRAME       PORT_VIPI
#define MFB_PORT_IN_WEIGHTING       PORT_VIP3I
#define MFB_PORT_IN_CONF_MAP        PORT_LCEI
#define MFB_PORT_OUT_FRAME          PORT_IMG3O // using IMG3O for the best quality and bit true
#define MFB_PORT_OUT_WEIGHTING      PORT_MFBO
#define MFB_SUPPORT_CONF_MAP        0 // ver2 supports confidence map

/* group ID in MFB stage, if not defined which means not support crop */
#define MFB_GID_OUT_FRAME           1 // IMG2O group id in MFB stage

/******************************************************************************
 *  MIX
 *****************************************************************************/
/* port */
#define MIX_PORT_IN_BASE_FRAME      PORT_IMGI
#define MIX_PORT_IN_GOLDEN_FRAME    PORT_VIPI
#define MIX_PORT_IN_WEIGHTING       PORT_UFDI
#define MIX_PORT_OUT_FRAME          PORT_WROTO // using IMG3O for the best quality and bit true
#define MIX_PORT_OUT_MAIN           PORT_WDMAO
#define MIX_PORT_OUT_THUMBNAIL      PORT_WROTO


};/* namespace mfll */
#endif//__MFLLMFB_PLATFORM_H__
