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

#if ((MFLL_MF_TAG_VERSION > 0) && (MFLL_MF_TAG_VERSION != 10))
#   error "MT6765 only supports MF_TAG_VERSION 10"
#endif

#ifndef LOG_TAG
#define LOG_TAG "MfllCore/Mfb"
#include <mtkcam3/feature/mfnr/MfllLog.h>
#endif

#include <custom/debug_exif/dbg_exif_param.h>

#include "MfllIspProfiles.h"

#include <mtkcam3/feature/mfnr/IMfllCore.h>

// platform dependent headers
#include <drv/isp_reg.h> // dip_x_reg_t

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
#ifdef WORKAROUND_MFB_STAGE
    dip_x_reg_t r = {0};

    r.DIP_X_MFB_CON.Raw     = regs->DIP_X_MFB_CON.Raw;
    r.DIP_X_MFB_LL_CON1.Raw = regs->DIP_X_MFB_LL_CON1.Raw;
    r.DIP_X_MFB_LL_CON2.Raw = regs->DIP_X_MFB_LL_CON2.Raw;
    r.DIP_X_MFB_LL_CON4.Raw = regs->DIP_X_MFB_LL_CON4.Raw;
    r.DIP_X_MFB_EDGE.Raw    = regs->DIP_X_MFB_EDGE.Raw;

    mfllLogD("--- after workaround ---");
    mfllLogD("DIP_X_CTL_YUV_EN  = %#x", regs->DIP_X_CTL_YUV_EN.Raw);
    mfllLogD("DIP_X_G2C_CONV_0A = %#x", regs->DIP_X_G2C_CONV_0A.Raw);
    mfllLogD("DIP_X_G2C_CONV_0B = %#x", regs->DIP_X_G2C_CONV_0B.Raw);
    mfllLogD("DIP_X_G2C_CONV_1A = %#x", regs->DIP_X_G2C_CONV_1A.Raw);
    mfllLogD("DIP_X_G2C_CONV_1B = %#x", regs->DIP_X_G2C_CONV_1B.Raw);
    mfllLogD("DIP_X_G2C_CONV_2A = %#x", regs->DIP_X_G2C_CONV_2A.Raw);
    mfllLogD("DIP_X_G2C_CONV_2B = %#x", regs->DIP_X_G2C_CONV_2B.Raw);

    r.DIP_X_CTL_YUV_EN.Bits.G2C_EN = 1;//enable bit
    r.DIP_X_G2C_CONV_0A.Raw = 0x0200;
    r.DIP_X_G2C_CONV_0B.Raw = 0x0;
    r.DIP_X_G2C_CONV_1A.Raw = 0x02000000;
    r.DIP_X_G2C_CONV_1B.Raw = 0x0;
    r.DIP_X_G2C_CONV_2A.Raw = 0x0;
    r.DIP_X_G2C_CONV_2B.Raw = 0x0200;

    memcpy((void*)regs, (void*)&r, sizeof(dip_x_reg_t));
    return true;
#else
    return false;
#endif
// }}}
}

inline void debug_pass2_registers(
        void*   regs    __attribute__((unused)),
        int     stage   __attribute__((unused))
        )
{
// {{{
#ifdef __DEBUG
    const dip_x_reg_t* tuningDat = reinterpret_cast<dip_x_reg_t*>(regs);

    auto regdump = [&tuningDat]()->void{
        mfllLogD("regdump: DIP_X_CTL_START       = %#x", tuningDat->DIP_X_CTL_START.Raw);
        mfllLogD("regdump: DIP_X_CTL_YUV_EN      = %#x", tuningDat->DIP_X_CTL_YUV_EN.Raw);
        mfllLogD("regdump: DIP_X_CTL_YUV2_EN     = %#x", tuningDat->DIP_X_CTL_YUV2_EN.Raw);
        mfllLogD("regdump: DIP_X_CTL_RGB_EN      = %#x", tuningDat->DIP_X_CTL_RGB_EN.Raw);
        mfllLogD("regdump: DIP_X_CTL_DMA_EN      = %#x", tuningDat->DIP_X_CTL_DMA_EN.Raw);
        mfllLogD("regdump: DIP_X_CTL_FMT_SEL     = %#x", tuningDat->DIP_X_CTL_FMT_SEL.Raw);
        mfllLogD("regdump: DIP_X_CTL_PATH_SEL    = %#x", tuningDat->DIP_X_CTL_PATH_SEL.Raw);
        mfllLogD("regdump: DIP_X_CTL_MISC_SEL    = %#x", tuningDat->DIP_X_CTL_MISC_SEL.Raw);
        mfllLogD("");
        mfllLogD("regdump: DIP_X_CTL_CQ_INT_EN   = %#x", tuningDat->DIP_X_CTL_CQ_INT_EN.Raw);
        mfllLogD("regdump: DIP_X_CTL_CQ_INT_EN   = %#x", tuningDat->DIP_X_CTL_CQ_INT_EN.Raw);
        mfllLogD("regdump: DIP_X_CTL_CQ_INT2_EN  = %#x", tuningDat->DIP_X_CTL_CQ_INT2_EN.Raw);
        mfllLogD("regdump: DIP_X_CTL_CQ_INT3_EN  = %#x", tuningDat->DIP_X_CTL_CQ_INT3_EN.Raw);
        mfllLogD("");

    };

    switch (stage) {
    case STAGE_MFB:
        mfllLogD("regdump: ----------------------------------------");
        mfllLogD("regdump: STAGE MFB");
        regdump();
        mfllLogD("regdump: MFB_CON               = %#x", tuningDat->DIP_X_MFB_CON.Raw);
        mfllLogD("regdump: MFB_LL_CON1           = %#x", tuningDat->DIP_X_MFB_LL_CON1.Raw);
        mfllLogD("regdump: MFB_LL_CON2           = %#x", tuningDat->DIP_X_MFB_LL_CON2.Raw);
        mfllLogD("regdump: MFB_LL_CON3           = %#x", tuningDat->DIP_X_MFB_LL_CON3.Raw);
        mfllLogD("regdump: MFB_LL_CON4           = %#x", tuningDat->DIP_X_MFB_LL_CON4.Raw);
        mfllLogD("regdump: MFB_EDGE              = %#x", tuningDat->DIP_X_MFB_EDGE.Raw);
        break;
    case STAGE_MIX:
        mfllLogD("regdump: ----------------------------------------");
        mfllLogD("regdump: STAGE MIX");
        regdump();
        mfllLogD("regdump: MIX3_CTRL_0           = %#x", tuningDat->DIP_X_MIX3_CTRL_0.Raw);
        mfllLogD("regdump: MIX3_CTRL_1           = %#x", tuningDat->DIP_X_MIX3_CTRL_1.Raw);
        mfllLogD("regdump: MIX3_SPARE            = %#x", tuningDat->DIP_X_MIX3_SPARE.Raw);
        break;
    default:
        ;
    }

    int* preg = (int*)tuningDat;
    for (int i = 0; i < sizeof(dip_x_reg_t) / 4; i += 4)
        mfllLogD("regdump: offset = %#x, value = 0x%x, 0x%x, 0x%x, 0x%x", i, preg[i], preg[i+1], preg[i+2], preg[i+3]);

#endif
// }}}
}

inline void dump_exif_info(
        IMfllCore*  c,
        void*       regs,
        int         stage)
{
//{{{
    dip_x_reg_t* t = reinterpret_cast<dip_x_reg_t*>(regs);

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
// UDM
#define _D_WRITE_UDM(prefix, stage) \
    __ASSIGN(prefix, stage, DIP_X_UDM_INTP_CRS  ); \
    __ASSIGN(prefix, stage, DIP_X_UDM_INTP_NAT  ); \
    __ASSIGN(prefix, stage, DIP_X_UDM_INTP_AUG  ); \
    __ASSIGN(prefix, stage, DIP_X_UDM_LUMA_LUT1 ); \
    __ASSIGN(prefix, stage, DIP_X_UDM_LUMA_LUT2 ); \
    __ASSIGN(prefix, stage, DIP_X_UDM_SL_CTL    ); \
    __ASSIGN(prefix, stage, DIP_X_UDM_HFTD_CTL  ); \
    __ASSIGN(prefix, stage, DIP_X_UDM_NR_STR    ); \
    __ASSIGN(prefix, stage, DIP_X_UDM_NR_ACT    ); \
    __ASSIGN(prefix, stage, DIP_X_UDM_HF_STR    ); \
    __ASSIGN(prefix, stage, DIP_X_UDM_HF_ACT1   ); \
    __ASSIGN(prefix, stage, DIP_X_UDM_HF_ACT2   ); \
    __ASSIGN(prefix, stage, DIP_X_UDM_CLIP      ); \
    __ASSIGN(prefix, stage, DIP_X_UDM_DSB       ); \
    __ASSIGN(prefix, stage, DIP_X_UDM_TILE_EDGE ); \
    __ASSIGN(prefix, stage, DIP_X_UDM_DSL       ); \
    __ASSIGN(prefix, stage, DIP_X_UDM_LR_RAT    ); \
    __ASSIGN(prefix, stage, DIP_X_UDM_SPARE_2   ); \
    __ASSIGN(prefix, stage, DIP_X_UDM_SPARE_3   ); \


// ANR
#define _D_WRITE_ANR(prefix, stage) \
    __ASSIGN(prefix, stage, DIP_X_ANR_CON1      ); \
    __ASSIGN(prefix, stage, DIP_X_ANR_CON2      ); \
    __ASSIGN(prefix, stage, DIP_X_ANR_YAD1      ); \
    __ASSIGN(prefix, stage, DIP_X_ANR_YAD2      ); \
    __ASSIGN(prefix, stage, DIP_X_ANR_Y4LUT1    ); \
    __ASSIGN(prefix, stage, DIP_X_ANR_Y4LUT2    ); \
    __ASSIGN(prefix, stage, DIP_X_ANR_Y4LUT3    ); \
    __ASSIGN(prefix, stage, DIP_X_ANR_C4LUT1    ); \
    __ASSIGN(prefix, stage, DIP_X_ANR_C4LUT2    ); \
    __ASSIGN(prefix, stage, DIP_X_ANR_C4LUT3    ); \
    __ASSIGN(prefix, stage, DIP_X_ANR_A4LUT2    ); \
    __ASSIGN(prefix, stage, DIP_X_ANR_A4LUT3    ); \
    __ASSIGN(prefix, stage, DIP_X_ANR_L4LUT1    ); \
    __ASSIGN(prefix, stage, DIP_X_ANR_L4LUT2    ); \
    __ASSIGN(prefix, stage, DIP_X_ANR_L4LUT3    ); \
    __ASSIGN(prefix, stage, DIP_X_ANR_PTY       ); \
    __ASSIGN(prefix, stage, DIP_X_ANR_CAD       ); \
    __ASSIGN(prefix, stage, DIP_X_ANR_PTC       ); \
    __ASSIGN(prefix, stage, DIP_X_ANR_LCE       ); \
    __ASSIGN(prefix, stage, DIP_X_ANR_T4LUT1    ); \
    __ASSIGN(prefix, stage, DIP_X_ANR_T4LUT2    ); \
    __ASSIGN(prefix, stage, DIP_X_ANR_T4LUT3    ); \
    __ASSIGN(prefix, stage, DIP_X_ANR_ACT1      ); \
    __ASSIGN(prefix, stage, DIP_X_ANR_ACT2      ); \
    __ASSIGN(prefix, stage, DIP_X_ANR_ACT4      ); \
    __ASSIGN(prefix, stage, DIP_X_ANR_ACTYHL    ); \
    __ASSIGN(prefix, stage, DIP_X_ANR_ACTYHH    ); \
    __ASSIGN(prefix, stage, DIP_X_ANR_ACTYL     ); \
    __ASSIGN(prefix, stage, DIP_X_ANR_ACTYHL2   ); \
    __ASSIGN(prefix, stage, DIP_X_ANR_ACTYHH2   ); \
    __ASSIGN(prefix, stage, DIP_X_ANR_ACTYL2    ); \
    __ASSIGN(prefix, stage, DIP_X_ANR_ACTC      ); \
    __ASSIGN(prefix, stage, DIP_X_ANR_YLAD      ); \
    __ASSIGN(prefix, stage, DIP_X_ANR_YLAD2     ); \
    __ASSIGN(prefix, stage, DIP_X_ANR_YLAD3     ); \
    __ASSIGN(prefix, stage, DIP_X_ANR_PTYL      ); \
    __ASSIGN(prefix, stage, DIP_X_ANR_LCOEF     ); \
    __ASSIGN(prefix, stage, DIP_X_ANR_YDIR      ); \
    __ASSIGN(prefix, stage, DIP_X_ANR_RSV1      ); \


// ANR2
#define _D_WRITE_ANR2(prefix, stage) \
    __ASSIGN(prefix, stage, DIP_X_ANR2_CON1     ); \
    __ASSIGN(prefix, stage, DIP_X_ANR2_CON2     ); \
    __ASSIGN(prefix, stage, DIP_X_ANR2_YAD1     ); \
    __ASSIGN(prefix, stage, DIP_X_ANR2_Y4LUT1   ); \
    __ASSIGN(prefix, stage, DIP_X_ANR2_Y4LUT2   ); \
    __ASSIGN(prefix, stage, DIP_X_ANR2_Y4LUT3   ); \
    __ASSIGN(prefix, stage, DIP_X_ANR2_L4LUT1   ); \
    __ASSIGN(prefix, stage, DIP_X_ANR2_L4LUT2   ); \
    __ASSIGN(prefix, stage, DIP_X_ANR2_L4LUT3   ); \
    __ASSIGN(prefix, stage, DIP_X_ANR2_CAD      ); \
    __ASSIGN(prefix, stage, DIP_X_ANR2_PTC      ); \
    __ASSIGN(prefix, stage, DIP_X_ANR2_LCE      ); \
    __ASSIGN(prefix, stage, DIP_X_ANR2_MED1     ); \
    __ASSIGN(prefix, stage, DIP_X_ANR2_MED2     ); \
    __ASSIGN(prefix, stage, DIP_X_ANR2_MED3     ); \
    __ASSIGN(prefix, stage, DIP_X_ANR2_MED4     ); \
    __ASSIGN(prefix, stage, DIP_X_ANR2_MED5     ); \
    __ASSIGN(prefix, stage, DIP_X_ANR2_ACTC     ); \
    __ASSIGN(prefix, stage, DIP_X_ANR2_RSV1     ); \
    __ASSIGN(prefix, stage, DIP_X_ANR2_RSV2     ); \


// SEEE
#define _D_WRITE_SEEE(prefix, stage) \
    __ASSIGN(prefix, stage, DIP_X_SEEE_CTRL             ); \
    __ASSIGN(prefix, stage, DIP_X_SEEE_CLIP_CTRL_1      ); \
    __ASSIGN(prefix, stage, DIP_X_SEEE_CLIP_CTRL_2      ); \
    __ASSIGN(prefix, stage, DIP_X_SEEE_CLIP_CTRL_3      ); \
    __ASSIGN(prefix, stage, DIP_X_SEEE_BLND_CTRL_1      ); \
    __ASSIGN(prefix, stage, DIP_X_SEEE_BLND_CTRL_2      ); \
    __ASSIGN(prefix, stage, DIP_X_SEEE_GN_CTRL          ); \
    __ASSIGN(prefix, stage, DIP_X_SEEE_LUMA_CTRL_1      ); \
    __ASSIGN(prefix, stage, DIP_X_SEEE_LUMA_CTRL_2      ); \
    __ASSIGN(prefix, stage, DIP_X_SEEE_LUMA_CTRL_3      ); \
    __ASSIGN(prefix, stage, DIP_X_SEEE_LUMA_CTRL_4      ); \
    __ASSIGN(prefix, stage, DIP_X_SEEE_SLNK_CTRL_1      ); \
    __ASSIGN(prefix, stage, DIP_X_SEEE_SLNK_CTRL_2      ); \
    __ASSIGN(prefix, stage, DIP_X_SEEE_GLUT_CTRL_1      ); \
    __ASSIGN(prefix, stage, DIP_X_SEEE_GLUT_CTRL_2      ); \
    __ASSIGN(prefix, stage, DIP_X_SEEE_GLUT_CTRL_3      ); \
    __ASSIGN(prefix, stage, DIP_X_SEEE_GLUT_CTRL_4      ); \
    __ASSIGN(prefix, stage, DIP_X_SEEE_GLUT_CTRL_5      ); \
    __ASSIGN(prefix, stage, DIP_X_SEEE_GLUT_CTRL_6      ); \
    __ASSIGN(prefix, stage, DIP_X_SEEE_OUT_EDGE_CTRL    ); \
    __ASSIGN(prefix, stage, DIP_X_SEEE_SE_Y_CTRL        ); \
    __ASSIGN(prefix, stage, DIP_X_SEEE_SE_EDGE_CTRL_1   ); \
    __ASSIGN(prefix, stage, DIP_X_SEEE_SE_EDGE_CTRL_2   ); \
    __ASSIGN(prefix, stage, DIP_X_SEEE_SE_EDGE_CTRL_3   ); \
    __ASSIGN(prefix, stage, DIP_X_SEEE_SE_SPECL_CTRL    ); \
    __ASSIGN(prefix, stage, DIP_X_SEEE_SE_CORE_CTRL_1   ); \
    __ASSIGN(prefix, stage, DIP_X_SEEE_SE_CORE_CTRL_2   ); \


// MFB
#define _D_WRITE_MFB(prefix, stage) \
    __ASSIGN(prefix, stage, DIP_X_MFB_CON           ); \
    __ASSIGN(prefix, stage, DIP_X_MFB_LL_CON1       ); \
    __ASSIGN(prefix, stage, DIP_X_MFB_LL_CON2       ); \
    __ASSIGN(prefix, stage, DIP_X_MFB_LL_CON3       ); \
    __ASSIGN(prefix, stage, DIP_X_MFB_LL_CON4       ); \
    __ASSIGN(prefix, stage, DIP_X_MFB_EDGE          ); \
    __ASSIGN(prefix, stage, DIP_X_MFB_LL_CON5       ); \
    __ASSIGN(prefix, stage, DIP_X_MFB_LL_CON6       ); \
    __ASSIGN(prefix, stage, DIP_X_MFB_LL_CON7       ); \
    __ASSIGN(prefix, stage, DIP_X_MFB_LL_CON8       ); \
    __ASSIGN(prefix, stage, DIP_X_MFB_LL_CON9       ); \
    __ASSIGN(prefix, stage, DIP_X_MFB_LL_CON10      ); \
    __ASSIGN(prefix, stage, DIP_X_MFB_MBD_CON0      ); \
    __ASSIGN(prefix, stage, DIP_X_MFB_MBD_CON1      ); \
    __ASSIGN(prefix, stage, DIP_X_MFB_MBD_CON2      ); \
    __ASSIGN(prefix, stage, DIP_X_MFB_MBD_CON3      ); \
    __ASSIGN(prefix, stage, DIP_X_MFB_MBD_CON4      ); \
    __ASSIGN(prefix, stage, DIP_X_MFB_MBD_CON5      ); \
    __ASSIGN(prefix, stage, DIP_X_MFB_MBD_CON6      ); \
    __ASSIGN(prefix, stage, DIP_X_MFB_MBD_CON7      ); \
    __ASSIGN(prefix, stage, DIP_X_MFB_MBD_CON8      ); \
    __ASSIGN(prefix, stage, DIP_X_MFB_MBD_CON9      ); \
    __ASSIGN(prefix, stage, DIP_X_MFB_MBD_CON10     ); \


// MIX3
#define _D_WRITE_MIX3(prefix, stage) \
    __ASSIGN(prefix, stage, DIP_X_MIX3_CTRL_0       ); \
    __ASSIGN(prefix, stage, DIP_X_MIX3_CTRL_1       ); \
    __ASSIGN(prefix, stage, DIP_X_MIX3_SPARE        ); \


// HFG
#define _D_WRITE_HFG(prefix, stage) \
    __ASSIGN(prefix, stage, DIP_X_HFG_CON_0     ); \
    __ASSIGN(prefix, stage, DIP_X_HFG_LUMA_0    ); \
    __ASSIGN(prefix, stage, DIP_X_HFG_LUMA_1    ); \
    __ASSIGN(prefix, stage, DIP_X_HFG_LUMA_2    ); \
    __ASSIGN(prefix, stage, DIP_X_HFG_LCE_0     ); \
    __ASSIGN(prefix, stage, DIP_X_HFG_LCE_1     ); \
    __ASSIGN(prefix, stage, DIP_X_HFG_LCE_2     ); \
    __ASSIGN(prefix, stage, DIP_X_HFG_RAN_0     ); \
    __ASSIGN(prefix, stage, DIP_X_HFG_RAN_1     ); \
    __ASSIGN(prefix, stage, DIP_X_HFG_RAN_2     ); \
    __ASSIGN(prefix, stage, DIP_X_HFG_RAN_3     ); \
    __ASSIGN(prefix, stage, DIP_X_HFG_RAN_4     ); \
    __ASSIGN(prefix, stage, DIP_X_HFG_CROP_X    ); \
    __ASSIGN(prefix, stage, DIP_X_HFG_CROP_Y    ); \

// }}} Registers


    switch (stage) {
    case STAGE_BASE_YUV:
#define _D_PREFIX   MF_TAG_BEFORE_
        _D_WRITE_UDM    (_D_PREFIX, bf);
        _D_WRITE_ANR    (_D_PREFIX, bf);
        _D_WRITE_ANR2   (_D_PREFIX, bf);
        _D_WRITE_SEEE   (_D_PREFIX, bf);
#undef  _D_PREFIX
        break;

    case STAGE_GOLDEN_YUV:
#define _D_PREFIX   MF_TAG_SINGLE_
        _D_WRITE_ANR    (_D_PREFIX, sf);
        _D_WRITE_ANR2   (_D_PREFIX, sf);
        _D_WRITE_SEEE   (_D_PREFIX, sf);
        _D_WRITE_HFG    (_D_PREFIX, sf);
#undef  _D_PREFIX
        break;

    case STAGE_MFB:
#define _D_PREFIX   MF_TAG_BLEND_
        _D_WRITE_MFB    (_D_PREFIX, mfb);
#undef  _D_PREFIX
        break;

    case STAGE_MIX:
#define _D_PREFIX   MF_TAG_AFTER_
        _D_WRITE_ANR    (_D_PREFIX, af);
        _D_WRITE_ANR2   (_D_PREFIX, af);
        _D_WRITE_SEEE   (_D_PREFIX, af);
        _D_WRITE_MIX3   (_D_PREFIX, af);
        _D_WRITE_HFG    (_D_PREFIX, af);
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
        dip_x_reg_t* const reg = reinterpret_cast<dip_x_reg_t*>(cfg[ePLATFORMCFG_DIP_X_REG_T]);

        if (reg == nullptr) {
            mfllLogE("%s: got nullptr of dip_x_reg_t* in stage MFB, index = %zu",
                    __FUNCTION__, index);
            return false;
        }

        // BRZ is necessary for half size MC only
        reg->DIP_X_MFB_CON.Bits.BLD_LL_BRZ_EN = (bFullMc ? 0 : 1);

        // blend mode should be 0 for the first blending.
        reg->DIP_X_MFB_CON.Bits.BLD_MODE = (index == 0 ? 0 : 1);

        // resolution
        reg->DIP_X_MFB_LL_CON3.Raw =
            (cfg[ePLATFORMCFG_SRC_HEIGHT] << 16) | (cfg[ePLATFORMCFG_SRC_WIDTH]);
    } break; // STAGE_MFB

    case STAGE_MIX: {
        dip_x_reg_t* const reg = reinterpret_cast<dip_x_reg_t*>(cfg[ePLATFORMCFG_DIP_X_REG_T]);

        if (reg == nullptr) {
            mfllLogE("%s: got nullptr of dip_x_reg_t* in stage MIX", __FUNCTION__);
            return false;
        }

        // rule: Y_EN and UV_EN are supposed to be both enable or disable or
        //       device may hang.
        const bool bEnable_Y  = (reg->DIP_X_MIX3_CTRL_0.Bits.MIX3_Y_EN != 0);
        const bool bEnable_UV = (reg->DIP_X_MIX3_CTRL_0.Bits.MIX3_UV_EN != 0);

        if (bEnable_Y && bEnable_UV) {
            cfg[ePLATFORMCFG_ENABLE_MIX3] = 1;
            // set top enable bits of MIX3 to 1
            reg->DIP_X_CTL_YUV_EN.Bits.MIX3_EN = 1;
        }
        else if (!bEnable_Y && !bEnable_UV) {
            cfg[ePLATFORMCFG_ENABLE_MIX3] = 0;
            // set top enable bits of MIX3 to 0
            reg->DIP_X_CTL_YUV_EN.Bits.MIX3_EN = 0;
        }
        else {
            mfllLogE("%s: !!!fatal error!!!", __FUNCTION__);
            mfllLogE("%s: register value is invalid, MIX3_Y_EN and MIX3_UV_EN " \
                    "should be both enable or disable.", __FUNCTION__);
            mfllLogE("%s: DIP_X_MIX3_CTRL_0 = %#x", __FUNCTION__,
                    reg->DIP_X_MIX3_CTRL_0.Raw);
            mfllLogE("%s: MIX3_Y_EN  = %d", __FUNCTION__,
                    static_cast<int>(reg->DIP_X_MIX3_CTRL_0.Bits.MIX3_Y_EN));
            mfllLogE("%s: MIX3_UV_EN = %d", __FUNCTION__,
                    static_cast<int>(reg->DIP_X_MIX3_CTRL_0.Bits.MIX3_UV_EN));

            assert(0);
        }
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

#define MIX_MAIN_GID_OUT            PASS2_CROPPING_ID_CRZ
#define MIX_THUMBNAIL_GID_OUT       PASS2_CROPPING_ID_WROTO

/******************************************************************************
 *  MFB
 *****************************************************************************/
/* port */
#define MFB_PORT_IN_BASE_FRAME      PORT_IMGI
#define MFB_PORT_IN_REF_FRAME       PORT_IMGBI
#define MFB_PORT_IN_WEIGHTING       PORT_IMGCI
#define MFB_PORT_IN_CONF_MAP        PORT_LCEI
#define MFB_PORT_OUT_FRAME          PORT_IMG3O // using IMG3O for the best quality and bit true
#define MFB_PORT_OUT_WEIGHTING      PORT_MFBO
#define MFB_SUPPORT_CONF_MAP        1 // ver2 supports confidence map

/* group ID in MFB stage, if not defined which means not support crop */
#define MFB_GID_OUT_FRAME           1 // IMG2O group id in MFB stage

/******************************************************************************
 *  MIX
 *****************************************************************************/
/* port */
#define MIX_PORT_IN_BASE_FRAME      PORT_IMGI
#define MIX_PORT_IN_GOLDEN_FRAME    PORT_VIPI
#define MIX_PORT_IN_WEIGHTING       PORT_DMGI
#define MIX_PORT_OUT_FRAME          PORT_IMG3O // using IMG3O for the best quality and bit true
#define MIX_PORT_OUT_MAIN           PORT_IMG3O
#define MIX_PORT_OUT_THUMBNAIL      PORT_WROTO


};/* namespace mfll */
#endif//__MFLLMFB_PLATFORM_H__
