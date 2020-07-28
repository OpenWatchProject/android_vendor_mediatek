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

#if ((MFLL_MF_TAG_VERSION > 0) && (MFLL_MF_TAG_VERSION != 3 && MFLL_MF_TAG_VERSION != 5))
#   error "MT6757 only supports MF_TAG_VERSION 3 or 5"
#endif

#ifndef LOG_TAG
#define LOG_TAG "MfllCore/Mfb"
#include <MfllLog.h>
#endif

#include <custom/debug_exif/dbg_exif_param.h>

#include "MfllIspProfiles.h"

#include <IMfllCore.h>

// platform dependent headers
#include <drv/isp_reg.h> // dip_x_reg_t

// STL
#include <cassert> // std::assert

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
    const dip_x_reg_t* t = reinterpret_cast<dip_x_reg_t*>(regs);

    if (c == NULL || t == NULL)
        return;

#if (MFLL_MF_TAG_VERSION > 0)
#   define _D(tag, value) c->updateExifInfo((unsigned int)tag, (uint32_t)value.Raw)
#else
#   define _D(tag, value) do{} while(0)
#endif

    switch (stage) {
    case STAGE_BASE_YUV:
        // {{{ STAGE_BASE_YUV
        // UDM
        _D(MF_TAG_BEFORE_DIP_X_UDM_INTP_CRS,            t->DIP_X_UDM_INTP_CRS       );
        _D(MF_TAG_BEFORE_DIP_X_UDM_INTP_NAT,            t->DIP_X_UDM_INTP_NAT       );
        _D(MF_TAG_BEFORE_DIP_X_UDM_INTP_AUG,            t->DIP_X_UDM_INTP_AUG       );
        _D(MF_TAG_BEFORE_DIP_X_UDM_LUMA_LUT1,           t->DIP_X_UDM_LUMA_LUT1      );
        _D(MF_TAG_BEFORE_DIP_X_UDM_LUMA_LUT2,           t->DIP_X_UDM_LUMA_LUT2      );
        _D(MF_TAG_BEFORE_DIP_X_UDM_SL_CTL,              t->DIP_X_UDM_SL_CTL         );
        _D(MF_TAG_BEFORE_DIP_X_UDM_HFTD_CTL,            t->DIP_X_UDM_HFTD_CTL       );
        _D(MF_TAG_BEFORE_DIP_X_UDM_NR_STR,              t->DIP_X_UDM_NR_STR         );
        _D(MF_TAG_BEFORE_DIP_X_UDM_NR_ACT,              t->DIP_X_UDM_NR_ACT         );
        _D(MF_TAG_BEFORE_DIP_X_UDM_HF_STR,              t->DIP_X_UDM_HF_STR         );
        _D(MF_TAG_BEFORE_DIP_X_UDM_HF_ACT1,             t->DIP_X_UDM_HF_ACT1        );
        _D(MF_TAG_BEFORE_DIP_X_UDM_HF_ACT2,             t->DIP_X_UDM_HF_ACT2        );
        _D(MF_TAG_BEFORE_DIP_X_UDM_CLIP,                t->DIP_X_UDM_CLIP           );
        _D(MF_TAG_BEFORE_DIP_X_UDM_DSB,                 t->DIP_X_UDM_DSB            );
        _D(MF_TAG_BEFORE_DIP_X_UDM_TILE_EDGE,           t->DIP_X_UDM_TILE_EDGE      );
        _D(MF_TAG_BEFORE_DIP_X_UDM_DSL,                 t->DIP_X_UDM_DSL            );
        _D(MF_TAG_BEFORE_DIP_X_UDM_SPARE_2,             t->DIP_X_UDM_SPARE_2        );
        _D(MF_TAG_BEFORE_DIP_X_UDM_SPARE_3,             t->DIP_X_UDM_SPARE_3        );

        // ANR
        _D(MF_TAG_BEFORE_DIP_X_ANR_CON1,                t->DIP_X_ANR_CON1           );
        _D(MF_TAG_BEFORE_DIP_X_ANR_CON2,                t->DIP_X_ANR_CON2           );
        _D(MF_TAG_BEFORE_DIP_X_ANR_YAD1,                t->DIP_X_ANR_YAD1           );
        _D(MF_TAG_BEFORE_DIP_X_ANR_YAD2,                t->DIP_X_ANR_YAD2           );
        _D(MF_TAG_BEFORE_DIP_X_ANR_Y4LUT1,              t->DIP_X_ANR_Y4LUT1         );
        _D(MF_TAG_BEFORE_DIP_X_ANR_Y4LUT2,              t->DIP_X_ANR_Y4LUT2         );
        _D(MF_TAG_BEFORE_DIP_X_ANR_Y4LUT3,              t->DIP_X_ANR_Y4LUT3         );
        _D(MF_TAG_BEFORE_DIP_X_ANR_C4LUT1,              t->DIP_X_ANR_C4LUT1         );
        _D(MF_TAG_BEFORE_DIP_X_ANR_C4LUT2,              t->DIP_X_ANR_C4LUT2         );
        _D(MF_TAG_BEFORE_DIP_X_ANR_C4LUT3,              t->DIP_X_ANR_C4LUT3         );
        _D(MF_TAG_BEFORE_DIP_X_ANR_A4LUT2,              t->DIP_X_ANR_A4LUT2         );
        _D(MF_TAG_BEFORE_DIP_X_ANR_A4LUT3,              t->DIP_X_ANR_A4LUT3         );
        _D(MF_TAG_BEFORE_DIP_X_ANR_L4LUT1,              t->DIP_X_ANR_L4LUT1         );
        _D(MF_TAG_BEFORE_DIP_X_ANR_L4LUT2,              t->DIP_X_ANR_L4LUT2         );
        _D(MF_TAG_BEFORE_DIP_X_ANR_L4LUT3,              t->DIP_X_ANR_L4LUT3         );
        _D(MF_TAG_BEFORE_DIP_X_ANR_PTY,                 t->DIP_X_ANR_PTY            );
        _D(MF_TAG_BEFORE_DIP_X_ANR_CAD,                 t->DIP_X_ANR_CAD            );
        _D(MF_TAG_BEFORE_DIP_X_ANR_PTC,                 t->DIP_X_ANR_PTC            );
        _D(MF_TAG_BEFORE_DIP_X_ANR_LCE,                 t->DIP_X_ANR_LCE            );
        _D(MF_TAG_BEFORE_DIP_X_ANR_MED1,                t->DIP_X_ANR_MED1           );
        _D(MF_TAG_BEFORE_DIP_X_ANR_MED2,                t->DIP_X_ANR_MED2           );
        _D(MF_TAG_BEFORE_DIP_X_ANR_MED3,                t->DIP_X_ANR_MED3           );
        _D(MF_TAG_BEFORE_DIP_X_ANR_MED4,                t->DIP_X_ANR_MED4           );
        _D(MF_TAG_BEFORE_DIP_X_ANR_MED5,                t->DIP_X_ANR_MED5           );
        _D(MF_TAG_BEFORE_DIP_X_ANR_HP1,                 t->DIP_X_ANR_HP1            );
        _D(MF_TAG_BEFORE_DIP_X_ANR_HP2,                 t->DIP_X_ANR_HP2            );
        _D(MF_TAG_BEFORE_DIP_X_ANR_HP3,                 t->DIP_X_ANR_HP3            );
        _D(MF_TAG_BEFORE_DIP_X_ANR_ACT1,                t->DIP_X_ANR_ACT1           );
        _D(MF_TAG_BEFORE_DIP_X_ANR_ACT2,                t->DIP_X_ANR_ACT2           );
        _D(MF_TAG_BEFORE_DIP_X_ANR_ACT3,                t->DIP_X_ANR_ACT3           );
        _D(MF_TAG_BEFORE_DIP_X_ANR_ACT4,                t->DIP_X_ANR_ACT4           );
        _D(MF_TAG_BEFORE_DIP_X_ANR_ACTYHL,              t->DIP_X_ANR_ACTYHL         );
        _D(MF_TAG_BEFORE_DIP_X_ANR_ACTYHH,              t->DIP_X_ANR_ACTYHH         );
        _D(MF_TAG_BEFORE_DIP_X_ANR_ACTYL,               t->DIP_X_ANR_ACTYL          );
        _D(MF_TAG_BEFORE_DIP_X_ANR_ACTYHL2,             t->DIP_X_ANR_ACTYHL2        );
        _D(MF_TAG_BEFORE_DIP_X_ANR_ACTYHH2,             t->DIP_X_ANR_ACTYHH2        );
        _D(MF_TAG_BEFORE_DIP_X_ANR_ACTYL2,              t->DIP_X_ANR_ACTYL2         );
        _D(MF_TAG_BEFORE_DIP_X_ANR_ACTC,                t->DIP_X_ANR_ACTC           );
        _D(MF_TAG_BEFORE_DIP_X_ANR_YLAD,                t->DIP_X_ANR_YLAD           );
        _D(MF_TAG_BEFORE_DIP_X_ANR_YLAD2,               t->DIP_X_ANR_YLAD2          );
        _D(MF_TAG_BEFORE_DIP_X_ANR_YLAD3,               t->DIP_X_ANR_YLAD3          );
        _D(MF_TAG_BEFORE_DIP_X_ANR_PTYL,                t->DIP_X_ANR_PTYL           );
        _D(MF_TAG_BEFORE_DIP_X_ANR_LCOEF,               t->DIP_X_ANR_LCOEF          );
        _D(MF_TAG_BEFORE_DIP_X_ANR_YDIR,                t->DIP_X_ANR_YDIR           );

        // ANR2
        _D(MF_TAG_BEFORE_DIP_X_ANR2_CON1,               t->DIP_X_ANR2_CON1          );
        _D(MF_TAG_BEFORE_DIP_X_ANR2_CON2,               t->DIP_X_ANR2_CON2          );
        _D(MF_TAG_BEFORE_DIP_X_ANR2_YAD1,               t->DIP_X_ANR2_YAD1          );
        _D(MF_TAG_BEFORE_DIP_X_ANR2_Y4LUT1,             t->DIP_X_ANR2_Y4LUT1        );
        _D(MF_TAG_BEFORE_DIP_X_ANR2_Y4LUT2,             t->DIP_X_ANR2_Y4LUT2        );
        _D(MF_TAG_BEFORE_DIP_X_ANR2_Y4LUT3,             t->DIP_X_ANR2_Y4LUT3        );
        _D(MF_TAG_BEFORE_DIP_X_ANR2_L4LUT1,             t->DIP_X_ANR2_L4LUT1        );
        _D(MF_TAG_BEFORE_DIP_X_ANR2_L4LUT2,             t->DIP_X_ANR2_L4LUT2        );
        _D(MF_TAG_BEFORE_DIP_X_ANR2_L4LUT3,             t->DIP_X_ANR2_L4LUT3        );
        _D(MF_TAG_BEFORE_DIP_X_ANR2_CAD,                t->DIP_X_ANR2_CAD           );
        _D(MF_TAG_BEFORE_DIP_X_ANR2_PTC,                t->DIP_X_ANR2_PTC           );
        _D(MF_TAG_BEFORE_DIP_X_ANR2_LCE,                t->DIP_X_ANR2_LCE           );
        _D(MF_TAG_BEFORE_DIP_X_ANR2_MED1,               t->DIP_X_ANR2_MED1          );
        _D(MF_TAG_BEFORE_DIP_X_ANR2_MED2,               t->DIP_X_ANR2_MED2          );
        _D(MF_TAG_BEFORE_DIP_X_ANR2_MED3,               t->DIP_X_ANR2_MED3          );
        _D(MF_TAG_BEFORE_DIP_X_ANR2_MED4,               t->DIP_X_ANR2_MED4          );
        _D(MF_TAG_BEFORE_DIP_X_ANR2_MED5,               t->DIP_X_ANR2_MED5          );
        _D(MF_TAG_BEFORE_DIP_X_ANR2_ACTY,               t->DIP_X_ANR2_ACTY          );
        _D(MF_TAG_BEFORE_DIP_X_ANR2_ACTC,               t->DIP_X_ANR2_ACTC          );

        // SEEE
        _D(MF_TAG_BEFORE_DIP_X_SEEE_CTRL,               t->DIP_X_SEEE_CTRL          );
        _D(MF_TAG_BEFORE_DIP_X_SEEE_CLIP_CTRL_1,        t->DIP_X_SEEE_CLIP_CTRL_1   );
        _D(MF_TAG_BEFORE_DIP_X_SEEE_CLIP_CTRL_2,        t->DIP_X_SEEE_CLIP_CTRL_2   );
        _D(MF_TAG_BEFORE_DIP_X_SEEE_CLIP_CTRL_3,        t->DIP_X_SEEE_CLIP_CTRL_3   );
        _D(MF_TAG_BEFORE_DIP_X_SEEE_BLND_CTRL_1,        t->DIP_X_SEEE_BLND_CTRL_1   );
        _D(MF_TAG_BEFORE_DIP_X_SEEE_BLND_CTRL_2,        t->DIP_X_SEEE_BLND_CTRL_2   );
        _D(MF_TAG_BEFORE_DIP_X_SEEE_GN_CTRL,            t->DIP_X_SEEE_GN_CTRL       );
        _D(MF_TAG_BEFORE_DIP_X_SEEE_LUMA_CTRL_1,        t->DIP_X_SEEE_LUMA_CTRL_1   );
        _D(MF_TAG_BEFORE_DIP_X_SEEE_LUMA_CTRL_2,        t->DIP_X_SEEE_LUMA_CTRL_2   );
        _D(MF_TAG_BEFORE_DIP_X_SEEE_LUMA_CTRL_3,        t->DIP_X_SEEE_LUMA_CTRL_3   );
        _D(MF_TAG_BEFORE_DIP_X_SEEE_LUMA_CTRL_4,        t->DIP_X_SEEE_LUMA_CTRL_4   );
        _D(MF_TAG_BEFORE_DIP_X_SEEE_SLNK_CTRL_1,        t->DIP_X_SEEE_SLNK_CTRL_1   );
        _D(MF_TAG_BEFORE_DIP_X_SEEE_SLNK_CTRL_2,        t->DIP_X_SEEE_SLNK_CTRL_2   );
        _D(MF_TAG_BEFORE_DIP_X_SEEE_GLUT_CTRL_1,        t->DIP_X_SEEE_GLUT_CTRL_1   );
        _D(MF_TAG_BEFORE_DIP_X_SEEE_GLUT_CTRL_2,        t->DIP_X_SEEE_GLUT_CTRL_2   );
        _D(MF_TAG_BEFORE_DIP_X_SEEE_GLUT_CTRL_3,        t->DIP_X_SEEE_GLUT_CTRL_3   );
        _D(MF_TAG_BEFORE_DIP_X_SEEE_GLUT_CTRL_4,        t->DIP_X_SEEE_GLUT_CTRL_4   );
        _D(MF_TAG_BEFORE_DIP_X_SEEE_GLUT_CTRL_5,        t->DIP_X_SEEE_GLUT_CTRL_5   );
        _D(MF_TAG_BEFORE_DIP_X_SEEE_GLUT_CTRL_6,        t->DIP_X_SEEE_GLUT_CTRL_6   );
        _D(MF_TAG_BEFORE_DIP_X_SEEE_OUT_EDGE_CTRL,      t->DIP_X_SEEE_OUT_EDGE_CTRL );
        _D(MF_TAG_BEFORE_DIP_X_SEEE_SE_Y_CTRL,          t->DIP_X_SEEE_SE_Y_CTRL     );
        _D(MF_TAG_BEFORE_DIP_X_SEEE_SE_EDGE_CTRL_1,     t->DIP_X_SEEE_SE_EDGE_CTRL_1);
        _D(MF_TAG_BEFORE_DIP_X_SEEE_SE_EDGE_CTRL_2,     t->DIP_X_SEEE_SE_EDGE_CTRL_2);
        _D(MF_TAG_BEFORE_DIP_X_SEEE_SE_EDGE_CTRL_3,     t->DIP_X_SEEE_SE_EDGE_CTRL_3);
        _D(MF_TAG_BEFORE_DIP_X_SEEE_SE_SPECL_CTRL,      t->DIP_X_SEEE_SE_SPECL_CTRL );
        _D(MF_TAG_BEFORE_DIP_X_SEEE_SE_SPECL_CTRL,      t->DIP_X_SEEE_SE_SPECL_CTRL );
        _D(MF_TAG_BEFORE_DIP_X_SEEE_SE_CORE_CTRL_1,     t->DIP_X_SEEE_SE_CORE_CTRL_1);
        _D(MF_TAG_BEFORE_DIP_X_SEEE_SE_CORE_CTRL_2,     t->DIP_X_SEEE_SE_CORE_CTRL_2);
        //}}} STAGE_BASE_YUV
        break;

    case STAGE_GOLDEN_YUV:
        // {{{ STAGE_GOLDEN_YUV
        // ANR
        _D(MF_TAG_SINGLE_DIP_X_ANR_CON1,                t->DIP_X_ANR_CON1    );
        _D(MF_TAG_SINGLE_DIP_X_ANR_CON2,                t->DIP_X_ANR_CON2    );
        _D(MF_TAG_SINGLE_DIP_X_ANR_YAD1,                t->DIP_X_ANR_YAD1    );
        _D(MF_TAG_SINGLE_DIP_X_ANR_YAD2,                t->DIP_X_ANR_YAD2    );
        _D(MF_TAG_SINGLE_DIP_X_ANR_Y4LUT1,              t->DIP_X_ANR_Y4LUT1  );
        _D(MF_TAG_SINGLE_DIP_X_ANR_Y4LUT2,              t->DIP_X_ANR_Y4LUT2  );
        _D(MF_TAG_SINGLE_DIP_X_ANR_Y4LUT3,              t->DIP_X_ANR_Y4LUT3  );
        _D(MF_TAG_SINGLE_DIP_X_ANR_C4LUT1,              t->DIP_X_ANR_C4LUT1  );
        _D(MF_TAG_SINGLE_DIP_X_ANR_C4LUT2,              t->DIP_X_ANR_C4LUT2  );
        _D(MF_TAG_SINGLE_DIP_X_ANR_C4LUT3,              t->DIP_X_ANR_C4LUT3  );
        _D(MF_TAG_SINGLE_DIP_X_ANR_A4LUT2,              t->DIP_X_ANR_A4LUT2  );
        _D(MF_TAG_SINGLE_DIP_X_ANR_A4LUT3,              t->DIP_X_ANR_A4LUT3  );
        _D(MF_TAG_SINGLE_DIP_X_ANR_L4LUT1,              t->DIP_X_ANR_L4LUT1  );
        _D(MF_TAG_SINGLE_DIP_X_ANR_L4LUT2,              t->DIP_X_ANR_L4LUT2  );
        _D(MF_TAG_SINGLE_DIP_X_ANR_L4LUT3,              t->DIP_X_ANR_L4LUT3  );
        _D(MF_TAG_SINGLE_DIP_X_ANR_PTY,                 t->DIP_X_ANR_PTY     );
        _D(MF_TAG_SINGLE_DIP_X_ANR_CAD,                 t->DIP_X_ANR_CAD     );
        _D(MF_TAG_SINGLE_DIP_X_ANR_PTC,                 t->DIP_X_ANR_PTC     );
        _D(MF_TAG_SINGLE_DIP_X_ANR_LCE,                 t->DIP_X_ANR_LCE     );
        _D(MF_TAG_SINGLE_DIP_X_ANR_MED1,                t->DIP_X_ANR_MED1    );
        _D(MF_TAG_SINGLE_DIP_X_ANR_MED2,                t->DIP_X_ANR_MED2    );
        _D(MF_TAG_SINGLE_DIP_X_ANR_MED3,                t->DIP_X_ANR_MED3    );
        _D(MF_TAG_SINGLE_DIP_X_ANR_MED4,                t->DIP_X_ANR_MED4    );
        _D(MF_TAG_SINGLE_DIP_X_ANR_MED5,                t->DIP_X_ANR_MED5    );
        _D(MF_TAG_SINGLE_DIP_X_ANR_HP1,                 t->DIP_X_ANR_HP1     );
        _D(MF_TAG_SINGLE_DIP_X_ANR_HP2,                 t->DIP_X_ANR_HP2     );
        _D(MF_TAG_SINGLE_DIP_X_ANR_HP3,                 t->DIP_X_ANR_HP3     );
        _D(MF_TAG_SINGLE_DIP_X_ANR_ACT1,                t->DIP_X_ANR_ACT1    );
        _D(MF_TAG_SINGLE_DIP_X_ANR_ACT2,                t->DIP_X_ANR_ACT2    );
        _D(MF_TAG_SINGLE_DIP_X_ANR_ACT3,                t->DIP_X_ANR_ACT3    );
        _D(MF_TAG_SINGLE_DIP_X_ANR_ACT4,                t->DIP_X_ANR_ACT4    );
        _D(MF_TAG_SINGLE_DIP_X_ANR_ACTYHL,              t->DIP_X_ANR_ACTYHL  );
        _D(MF_TAG_SINGLE_DIP_X_ANR_ACTYHH,              t->DIP_X_ANR_ACTYHH  );
        _D(MF_TAG_SINGLE_DIP_X_ANR_ACTYL,               t->DIP_X_ANR_ACTYL   );
        _D(MF_TAG_SINGLE_DIP_X_ANR_ACTYHL2,             t->DIP_X_ANR_ACTYHL2 );
        _D(MF_TAG_SINGLE_DIP_X_ANR_ACTYHH2,             t->DIP_X_ANR_ACTYHH2 );
        _D(MF_TAG_SINGLE_DIP_X_ANR_ACTYL2,              t->DIP_X_ANR_ACTYL2  );
        _D(MF_TAG_SINGLE_DIP_X_ANR_ACTC,                t->DIP_X_ANR_ACTC    );
        _D(MF_TAG_SINGLE_DIP_X_ANR_YLAD,                t->DIP_X_ANR_YLAD    );
        _D(MF_TAG_SINGLE_DIP_X_ANR_YLAD2,               t->DIP_X_ANR_YLAD2   );
        _D(MF_TAG_SINGLE_DIP_X_ANR_YLAD3,               t->DIP_X_ANR_YLAD3   );
        _D(MF_TAG_SINGLE_DIP_X_ANR_PTYL,                t->DIP_X_ANR_PTYL    );
        _D(MF_TAG_SINGLE_DIP_X_ANR_LCOEF,               t->DIP_X_ANR_LCOEF   );
        _D(MF_TAG_SINGLE_DIP_X_ANR_YDIR,                t->DIP_X_ANR_YDIR    );

        // ANR2
        _D(MF_TAG_SINGLE_DIP_X_ANR2_CON1,               t->DIP_X_ANR2_CON1   );
        _D(MF_TAG_SINGLE_DIP_X_ANR2_CON2,               t->DIP_X_ANR2_CON2   );
        _D(MF_TAG_SINGLE_DIP_X_ANR2_YAD1,               t->DIP_X_ANR2_YAD1   );
        _D(MF_TAG_SINGLE_DIP_X_ANR2_Y4LUT1,             t->DIP_X_ANR2_Y4LUT1 );
        _D(MF_TAG_SINGLE_DIP_X_ANR2_Y4LUT2,             t->DIP_X_ANR2_Y4LUT2 );
        _D(MF_TAG_SINGLE_DIP_X_ANR2_Y4LUT3,             t->DIP_X_ANR2_Y4LUT3 );
        _D(MF_TAG_SINGLE_DIP_X_ANR2_L4LUT1,             t->DIP_X_ANR2_L4LUT1 );
        _D(MF_TAG_SINGLE_DIP_X_ANR2_L4LUT2,             t->DIP_X_ANR2_L4LUT2 );
        _D(MF_TAG_SINGLE_DIP_X_ANR2_L4LUT3,             t->DIP_X_ANR2_L4LUT3 );
        _D(MF_TAG_SINGLE_DIP_X_ANR2_CAD,                t->DIP_X_ANR2_CAD    );
        _D(MF_TAG_SINGLE_DIP_X_ANR2_PTC,                t->DIP_X_ANR2_PTC    );
        _D(MF_TAG_SINGLE_DIP_X_ANR2_LCE,                t->DIP_X_ANR2_LCE    );
        _D(MF_TAG_SINGLE_DIP_X_ANR2_MED1,               t->DIP_X_ANR2_MED1   );
        _D(MF_TAG_SINGLE_DIP_X_ANR2_MED2,               t->DIP_X_ANR2_MED2   );
        _D(MF_TAG_SINGLE_DIP_X_ANR2_MED3,               t->DIP_X_ANR2_MED3   );
        _D(MF_TAG_SINGLE_DIP_X_ANR2_MED4,               t->DIP_X_ANR2_MED4   );
        _D(MF_TAG_SINGLE_DIP_X_ANR2_MED5,               t->DIP_X_ANR2_MED5   );
        _D(MF_TAG_SINGLE_DIP_X_ANR2_ACTY,               t->DIP_X_ANR2_ACTY   );
        _D(MF_TAG_SINGLE_DIP_X_ANR2_ACTC,               t->DIP_X_ANR2_ACTC   );

        // SEEE
        _D(MF_TAG_SINGLE_DIP_X_SEEE_CTRL,               t->DIP_X_SEEE_CTRL          );
        _D(MF_TAG_SINGLE_DIP_X_SEEE_CLIP_CTRL_1,        t->DIP_X_SEEE_CLIP_CTRL_1   );
        _D(MF_TAG_SINGLE_DIP_X_SEEE_CLIP_CTRL_2,        t->DIP_X_SEEE_CLIP_CTRL_2   );
        _D(MF_TAG_SINGLE_DIP_X_SEEE_CLIP_CTRL_3,        t->DIP_X_SEEE_CLIP_CTRL_3   );
        _D(MF_TAG_SINGLE_DIP_X_SEEE_BLND_CTRL_1,        t->DIP_X_SEEE_BLND_CTRL_1   );
        _D(MF_TAG_SINGLE_DIP_X_SEEE_BLND_CTRL_2,        t->DIP_X_SEEE_BLND_CTRL_2   );
        _D(MF_TAG_SINGLE_DIP_X_SEEE_GN_CTRL,            t->DIP_X_SEEE_GN_CTRL       );
        _D(MF_TAG_SINGLE_DIP_X_SEEE_LUMA_CTRL_1,        t->DIP_X_SEEE_LUMA_CTRL_1   );
        _D(MF_TAG_SINGLE_DIP_X_SEEE_LUMA_CTRL_2,        t->DIP_X_SEEE_LUMA_CTRL_2   );
        _D(MF_TAG_SINGLE_DIP_X_SEEE_LUMA_CTRL_3,        t->DIP_X_SEEE_LUMA_CTRL_3   );
        _D(MF_TAG_SINGLE_DIP_X_SEEE_LUMA_CTRL_4,        t->DIP_X_SEEE_LUMA_CTRL_4   );
        _D(MF_TAG_SINGLE_DIP_X_SEEE_SLNK_CTRL_1,        t->DIP_X_SEEE_SLNK_CTRL_1   );
        _D(MF_TAG_SINGLE_DIP_X_SEEE_SLNK_CTRL_2,        t->DIP_X_SEEE_SLNK_CTRL_2   );
        _D(MF_TAG_SINGLE_DIP_X_SEEE_GLUT_CTRL_1,        t->DIP_X_SEEE_GLUT_CTRL_1   );
        _D(MF_TAG_SINGLE_DIP_X_SEEE_GLUT_CTRL_2,        t->DIP_X_SEEE_GLUT_CTRL_2   );
        _D(MF_TAG_SINGLE_DIP_X_SEEE_GLUT_CTRL_3,        t->DIP_X_SEEE_GLUT_CTRL_3   );
        _D(MF_TAG_SINGLE_DIP_X_SEEE_GLUT_CTRL_4,        t->DIP_X_SEEE_GLUT_CTRL_4   );
        _D(MF_TAG_SINGLE_DIP_X_SEEE_GLUT_CTRL_5,        t->DIP_X_SEEE_GLUT_CTRL_5   );
        _D(MF_TAG_SINGLE_DIP_X_SEEE_GLUT_CTRL_6,        t->DIP_X_SEEE_GLUT_CTRL_6   );
        _D(MF_TAG_SINGLE_DIP_X_SEEE_OUT_EDGE_CTRL,      t->DIP_X_SEEE_OUT_EDGE_CTRL );
        _D(MF_TAG_SINGLE_DIP_X_SEEE_SE_Y_CTRL,          t->DIP_X_SEEE_SE_Y_CTRL     );
        _D(MF_TAG_SINGLE_DIP_X_SEEE_SE_EDGE_CTRL_1,     t->DIP_X_SEEE_SE_EDGE_CTRL_1);
        _D(MF_TAG_SINGLE_DIP_X_SEEE_SE_EDGE_CTRL_2,     t->DIP_X_SEEE_SE_EDGE_CTRL_2);
        _D(MF_TAG_SINGLE_DIP_X_SEEE_SE_EDGE_CTRL_3,     t->DIP_X_SEEE_SE_EDGE_CTRL_3);
        _D(MF_TAG_SINGLE_DIP_X_SEEE_SE_SPECL_CTRL,      t->DIP_X_SEEE_SE_SPECL_CTRL );
        _D(MF_TAG_SINGLE_DIP_X_SEEE_SE_CORE_CTRL_1,     t->DIP_X_SEEE_SE_CORE_CTRL_1);
        _D(MF_TAG_SINGLE_DIP_X_SEEE_SE_CORE_CTRL_2,     t->DIP_X_SEEE_SE_CORE_CTRL_2);

        // HFG
        _D(MF_TAG_SINGLE_DIP_X_HFG_CON_0,               t->DIP_X_HFG_CON_0   );
        _D(MF_TAG_SINGLE_DIP_X_HFG_LUMA_0,              t->DIP_X_HFG_LUMA_0  );
        _D(MF_TAG_SINGLE_DIP_X_HFG_LUMA_1,              t->DIP_X_HFG_LUMA_1  );
        _D(MF_TAG_SINGLE_DIP_X_HFG_LUMA_2,              t->DIP_X_HFG_LUMA_2  );
        _D(MF_TAG_SINGLE_DIP_X_HFG_LCE_0,               t->DIP_X_HFG_LCE_0   );
        _D(MF_TAG_SINGLE_DIP_X_HFG_LCE_1,               t->DIP_X_HFG_LCE_1   );
        _D(MF_TAG_SINGLE_DIP_X_HFG_LCE_2,               t->DIP_X_HFG_LCE_2   );
        _D(MF_TAG_SINGLE_DIP_X_HFG_RAN_0,               t->DIP_X_HFG_RAN_0   );
        _D(MF_TAG_SINGLE_DIP_X_HFG_RAN_1,               t->DIP_X_HFG_RAN_1   );
        _D(MF_TAG_SINGLE_DIP_X_HFG_RAN_3,               t->DIP_X_HFG_RAN_3   );

        // }}} STAGE_GOLDEN_YUV
        break;

    case STAGE_MFB:
        // {{{ STAGE_MFB
        _D(MF_TAG_BLEND_DIP_X_MFB_CON,                  t->DIP_X_MFB_CON      );
        _D(MF_TAG_BLEND_DIP_X_MFB_LL_CON1,              t->DIP_X_MFB_LL_CON1  );
        _D(MF_TAG_BLEND_DIP_X_MFB_LL_CON2,              t->DIP_X_MFB_LL_CON2  );
        _D(MF_TAG_BLEND_DIP_X_MFB_LL_CON4,              t->DIP_X_MFB_LL_CON4  );
        // }}} STAGE_MFB
        break;

    case STAGE_MIX:
        // {{{ STAGE_MIX
        // ANR
        _D(MF_TAG_AFTER_DIP_X_ANR_CON1,                 t->DIP_X_ANR_CON1           );
        _D(MF_TAG_AFTER_DIP_X_ANR_CON2,                 t->DIP_X_ANR_CON2           );
        _D(MF_TAG_AFTER_DIP_X_ANR_YAD1,                 t->DIP_X_ANR_YAD1           );
        _D(MF_TAG_AFTER_DIP_X_ANR_YAD2,                 t->DIP_X_ANR_YAD2           );
        _D(MF_TAG_AFTER_DIP_X_ANR_Y4LUT1,               t->DIP_X_ANR_Y4LUT1         );
        _D(MF_TAG_AFTER_DIP_X_ANR_Y4LUT2,               t->DIP_X_ANR_Y4LUT2         );
        _D(MF_TAG_AFTER_DIP_X_ANR_Y4LUT3,               t->DIP_X_ANR_Y4LUT3         );
        _D(MF_TAG_AFTER_DIP_X_ANR_C4LUT1,               t->DIP_X_ANR_C4LUT1         );
        _D(MF_TAG_AFTER_DIP_X_ANR_C4LUT2,               t->DIP_X_ANR_C4LUT2         );
        _D(MF_TAG_AFTER_DIP_X_ANR_C4LUT3,               t->DIP_X_ANR_C4LUT3         );
        _D(MF_TAG_AFTER_DIP_X_ANR_A4LUT2,               t->DIP_X_ANR_A4LUT2         );
        _D(MF_TAG_AFTER_DIP_X_ANR_A4LUT3,               t->DIP_X_ANR_A4LUT3         );
        _D(MF_TAG_AFTER_DIP_X_ANR_L4LUT1,               t->DIP_X_ANR_L4LUT1         );
        _D(MF_TAG_AFTER_DIP_X_ANR_L4LUT2,               t->DIP_X_ANR_L4LUT2         );
        _D(MF_TAG_AFTER_DIP_X_ANR_L4LUT3,               t->DIP_X_ANR_L4LUT3         );
        _D(MF_TAG_AFTER_DIP_X_ANR_PTY,                  t->DIP_X_ANR_PTY            );
        _D(MF_TAG_AFTER_DIP_X_ANR_CAD,                  t->DIP_X_ANR_CAD            );
        _D(MF_TAG_AFTER_DIP_X_ANR_PTC,                  t->DIP_X_ANR_PTC            );
        _D(MF_TAG_AFTER_DIP_X_ANR_LCE,                  t->DIP_X_ANR_LCE            );
        _D(MF_TAG_AFTER_DIP_X_ANR_MED1,                 t->DIP_X_ANR_MED1           );
        _D(MF_TAG_AFTER_DIP_X_ANR_MED2,                 t->DIP_X_ANR_MED2           );
        _D(MF_TAG_AFTER_DIP_X_ANR_MED3,                 t->DIP_X_ANR_MED3           );
        _D(MF_TAG_AFTER_DIP_X_ANR_MED4,                 t->DIP_X_ANR_MED4           );
        _D(MF_TAG_AFTER_DIP_X_ANR_MED5,                 t->DIP_X_ANR_MED5           );
        _D(MF_TAG_AFTER_DIP_X_ANR_HP1,                  t->DIP_X_ANR_HP1            );
        _D(MF_TAG_AFTER_DIP_X_ANR_HP2,                  t->DIP_X_ANR_HP2            );
        _D(MF_TAG_AFTER_DIP_X_ANR_HP3,                  t->DIP_X_ANR_HP3            );
        _D(MF_TAG_AFTER_DIP_X_ANR_ACT1,                 t->DIP_X_ANR_ACT1           );
        _D(MF_TAG_AFTER_DIP_X_ANR_ACT2,                 t->DIP_X_ANR_ACT2           );
        _D(MF_TAG_AFTER_DIP_X_ANR_ACT3,                 t->DIP_X_ANR_ACT3           );
        _D(MF_TAG_AFTER_DIP_X_ANR_ACT4,                 t->DIP_X_ANR_ACT4           );
        _D(MF_TAG_AFTER_DIP_X_ANR_ACTYHL,               t->DIP_X_ANR_ACTYHL         );
        _D(MF_TAG_AFTER_DIP_X_ANR_ACTYHH,               t->DIP_X_ANR_ACTYHH         );
        _D(MF_TAG_AFTER_DIP_X_ANR_ACTYL,                t->DIP_X_ANR_ACTYL          );
        _D(MF_TAG_AFTER_DIP_X_ANR_ACTYHL2,              t->DIP_X_ANR_ACTYHL2        );
        _D(MF_TAG_AFTER_DIP_X_ANR_ACTYHH2,              t->DIP_X_ANR_ACTYHH2        );
        _D(MF_TAG_AFTER_DIP_X_ANR_ACTYL2,               t->DIP_X_ANR_ACTYL2         );
        _D(MF_TAG_AFTER_DIP_X_ANR_ACTC,                 t->DIP_X_ANR_ACTC           );
        _D(MF_TAG_AFTER_DIP_X_ANR_YLAD,                 t->DIP_X_ANR_YLAD           );
        _D(MF_TAG_AFTER_DIP_X_ANR_YLAD2,                t->DIP_X_ANR_YLAD2          );
        _D(MF_TAG_AFTER_DIP_X_ANR_YLAD3,                t->DIP_X_ANR_YLAD3          );
        _D(MF_TAG_AFTER_DIP_X_ANR_PTYL,                 t->DIP_X_ANR_PTYL           );
        _D(MF_TAG_AFTER_DIP_X_ANR_LCOEF,                t->DIP_X_ANR_LCOEF          );
        _D(MF_TAG_AFTER_DIP_X_ANR_YDIR,                 t->DIP_X_ANR_YDIR           );

        // ANR2
        _D(MF_TAG_AFTER_DIP_X_ANR2_CON1,                t->DIP_X_ANR2_CON1          );
        _D(MF_TAG_AFTER_DIP_X_ANR2_CON2,                t->DIP_X_ANR2_CON2          );
        _D(MF_TAG_AFTER_DIP_X_ANR2_YAD1,                t->DIP_X_ANR2_YAD1          );
        _D(MF_TAG_AFTER_DIP_X_ANR2_Y4LUT1,              t->DIP_X_ANR2_Y4LUT1        );
        _D(MF_TAG_AFTER_DIP_X_ANR2_Y4LUT2,              t->DIP_X_ANR2_Y4LUT2        );
        _D(MF_TAG_AFTER_DIP_X_ANR2_Y4LUT3,              t->DIP_X_ANR2_Y4LUT3        );
        _D(MF_TAG_AFTER_DIP_X_ANR2_L4LUT1,              t->DIP_X_ANR2_L4LUT1        );
        _D(MF_TAG_AFTER_DIP_X_ANR2_L4LUT2,              t->DIP_X_ANR2_L4LUT2        );
        _D(MF_TAG_AFTER_DIP_X_ANR2_L4LUT3,              t->DIP_X_ANR2_L4LUT3        );
        _D(MF_TAG_AFTER_DIP_X_ANR2_CAD,                 t->DIP_X_ANR2_CAD           );
        _D(MF_TAG_AFTER_DIP_X_ANR2_PTC,                 t->DIP_X_ANR2_PTC           );
        _D(MF_TAG_AFTER_DIP_X_ANR2_LCE,                 t->DIP_X_ANR2_LCE           );
        _D(MF_TAG_AFTER_DIP_X_ANR2_MED1,                t->DIP_X_ANR2_MED1          );
        _D(MF_TAG_AFTER_DIP_X_ANR2_MED2,                t->DIP_X_ANR2_MED2          );
        _D(MF_TAG_AFTER_DIP_X_ANR2_MED3,                t->DIP_X_ANR2_MED3          );
        _D(MF_TAG_AFTER_DIP_X_ANR2_MED4,                t->DIP_X_ANR2_MED4          );
        _D(MF_TAG_AFTER_DIP_X_ANR2_MED5,                t->DIP_X_ANR2_MED5          );
        _D(MF_TAG_AFTER_DIP_X_ANR2_ACTY,                t->DIP_X_ANR2_ACTY          );
        _D(MF_TAG_AFTER_DIP_X_ANR2_ACTC,                t->DIP_X_ANR2_ACTC          );

        // SEEE
        _D(MF_TAG_AFTER_DIP_X_SEEE_CTRL,                t->DIP_X_SEEE_CTRL          );
        _D(MF_TAG_AFTER_DIP_X_SEEE_CLIP_CTRL_1,         t->DIP_X_SEEE_CLIP_CTRL_1   );
        _D(MF_TAG_AFTER_DIP_X_SEEE_CLIP_CTRL_2,         t->DIP_X_SEEE_CLIP_CTRL_2   );
        _D(MF_TAG_AFTER_DIP_X_SEEE_CLIP_CTRL_3,         t->DIP_X_SEEE_CLIP_CTRL_3   );
        _D(MF_TAG_AFTER_DIP_X_SEEE_BLND_CTRL_1,         t->DIP_X_SEEE_BLND_CTRL_1   );
        _D(MF_TAG_AFTER_DIP_X_SEEE_BLND_CTRL_2,         t->DIP_X_SEEE_BLND_CTRL_2   );
        _D(MF_TAG_AFTER_DIP_X_SEEE_GN_CTRL,             t->DIP_X_SEEE_GN_CTRL       );
        _D(MF_TAG_AFTER_DIP_X_SEEE_LUMA_CTRL_1,         t->DIP_X_SEEE_LUMA_CTRL_1   );
        _D(MF_TAG_AFTER_DIP_X_SEEE_LUMA_CTRL_2,         t->DIP_X_SEEE_LUMA_CTRL_2   );
        _D(MF_TAG_AFTER_DIP_X_SEEE_LUMA_CTRL_3,         t->DIP_X_SEEE_LUMA_CTRL_3   );
        _D(MF_TAG_AFTER_DIP_X_SEEE_LUMA_CTRL_4,         t->DIP_X_SEEE_LUMA_CTRL_4   );
        _D(MF_TAG_AFTER_DIP_X_SEEE_SLNK_CTRL_1,         t->DIP_X_SEEE_SLNK_CTRL_1   );
        _D(MF_TAG_AFTER_DIP_X_SEEE_SLNK_CTRL_2,         t->DIP_X_SEEE_SLNK_CTRL_2   );
        _D(MF_TAG_AFTER_DIP_X_SEEE_GLUT_CTRL_1,         t->DIP_X_SEEE_GLUT_CTRL_1   );
        _D(MF_TAG_AFTER_DIP_X_SEEE_GLUT_CTRL_2,         t->DIP_X_SEEE_GLUT_CTRL_2   );
        _D(MF_TAG_AFTER_DIP_X_SEEE_GLUT_CTRL_3,         t->DIP_X_SEEE_GLUT_CTRL_3   );
        _D(MF_TAG_AFTER_DIP_X_SEEE_GLUT_CTRL_4,         t->DIP_X_SEEE_GLUT_CTRL_4   );
        _D(MF_TAG_AFTER_DIP_X_SEEE_GLUT_CTRL_5,         t->DIP_X_SEEE_GLUT_CTRL_5   );
        _D(MF_TAG_AFTER_DIP_X_SEEE_GLUT_CTRL_6,         t->DIP_X_SEEE_GLUT_CTRL_6   );
        _D(MF_TAG_AFTER_DIP_X_SEEE_OUT_EDGE_CTRL,       t->DIP_X_SEEE_OUT_EDGE_CTRL );
        _D(MF_TAG_AFTER_DIP_X_SEEE_SE_Y_CTRL,           t->DIP_X_SEEE_SE_Y_CTRL     );
        _D(MF_TAG_AFTER_DIP_X_SEEE_SE_EDGE_CTRL_1,      t->DIP_X_SEEE_SE_EDGE_CTRL_1);
        _D(MF_TAG_AFTER_DIP_X_SEEE_SE_EDGE_CTRL_2,      t->DIP_X_SEEE_SE_EDGE_CTRL_2);
        _D(MF_TAG_AFTER_DIP_X_SEEE_SE_EDGE_CTRL_3,      t->DIP_X_SEEE_SE_EDGE_CTRL_3);
        _D(MF_TAG_AFTER_DIP_X_SEEE_SE_SPECL_CTRL,       t->DIP_X_SEEE_SE_SPECL_CTRL );
        _D(MF_TAG_AFTER_DIP_X_SEEE_SE_CORE_CTRL_1,      t->DIP_X_SEEE_SE_CORE_CTRL_1);
        _D(MF_TAG_AFTER_DIP_X_SEEE_SE_CORE_CTRL_2,      t->DIP_X_SEEE_SE_CORE_CTRL_2);

        // MIX3
        _D(MF_TAG_AFTER_DIP_X_MIX3_CTRL_0,              t->DIP_X_MIX3_CTRL_0        );
        _D(MF_TAG_AFTER_DIP_X_MIX3_CTRL_1,              t->DIP_X_MIX3_CTRL_1        );
        _D(MF_TAG_AFTER_DIP_X_MIX3_SPARE,               t->DIP_X_MIX3_SPARE         );

        // HFG
        _D(MF_TAG_AFTER_DIP_X_HFG_CON_0,                t->DIP_X_HFG_CON_0          );
        _D(MF_TAG_AFTER_DIP_X_HFG_LUMA_0,               t->DIP_X_HFG_LUMA_0         );
        _D(MF_TAG_AFTER_DIP_X_HFG_LUMA_1,               t->DIP_X_HFG_LUMA_1         );
        _D(MF_TAG_AFTER_DIP_X_HFG_LUMA_2,               t->DIP_X_HFG_LUMA_2         );
        _D(MF_TAG_AFTER_DIP_X_HFG_LCE_0,                t->DIP_X_HFG_LCE_0          );
        _D(MF_TAG_AFTER_DIP_X_HFG_LCE_1,                t->DIP_X_HFG_LCE_1          );
        _D(MF_TAG_AFTER_DIP_X_HFG_LCE_2,                t->DIP_X_HFG_LCE_2          );
        _D(MF_TAG_AFTER_DIP_X_HFG_RAN_0,                t->DIP_X_HFG_RAN_0          );
        _D(MF_TAG_AFTER_DIP_X_HFG_RAN_1,                t->DIP_X_HFG_RAN_1          );
        _D(MF_TAG_AFTER_DIP_X_HFG_RAN_3,                t->DIP_X_HFG_RAN_3          );

        // }}} STAGE_MIX
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
    mfllLogD("%s: profile(%d), mode(0x%X)", __FUNCTION__, p, mode);
    switch (p) {
    // bfbld
    case eMfllIspProfile_BeforeBlend:
    case eMfllIspProfile_BeforeBlend_Swnr:
    case eMfllIspProfile_BeforeBlend_Zsd:
    case eMfllIspProfile_BeforeBlend_Zsd_Swnr:
        switch (mode) {
            case MfllMode_NormalMfhr:
            case MfllMode_ZsdMfhr:
                mfllLogD("%s: EIspProfile_N3D_MFHR_Before_Blend", __FUNCTION__);
                return EIspProfile_N3D_MFHR_Before_Blend;
            case MfllMode_ZhdrNormalMfll:
            case MfllMode_ZhdrNormalAis:
            case MfllMode_ZhdrZsdMfll:
            case MfllMode_ZhdrZsdAis:
                mfllLogD("%s: EIspProfile_zHDR_Capture_MFNR_Before_Blend", __FUNCTION__);
                return EIspProfile_zHDR_Capture_MFNR_Before_Blend;
            case MfllMode_AutoZhdrNormalMfll:
            case MfllMode_AutoZhdrNormalAis:
            case MfllMode_AutoZhdrZsdMfll:
            case MfllMode_AutoZhdrZsdAis:
                mfllLogD("%s: EIspProfile_Auto_zHDR_Capture_MFNR_Before_Blend", __FUNCTION__);
                return EIspProfile_zHDR_Capture_MFNR_Before_Blend;
        default:;
        }
        mfllLogD("%s: default EIspProfile_MFNR_Before_Blend", __FUNCTION__);
        return EIspProfile_MFNR_Before_Blend;

    // single
    case eMfllIspProfile_Single:
    case eMfllIspProfile_Single_Swnr:
    case eMfllIspProfile_Single_Zsd:
    case eMfllIspProfile_Single_Zsd_Swnr:
        switch (mode) {
            case MfllMode_NormalMfhr:
            case MfllMode_ZsdMfhr:
                mfllLogD("%s: EIspProfile_N3D_MFHR_Single", __FUNCTION__);
                return EIspProfile_N3D_MFHR_Single;
            case MfllMode_ZhdrNormalMfll:
            case MfllMode_ZhdrNormalAis:
            case MfllMode_ZhdrZsdMfll:
            case MfllMode_ZhdrZsdAis:
                mfllLogD("%s: EIspProfile_zHDR_Capture_MFNR_Single", __FUNCTION__);
                return EIspProfile_zHDR_Capture_MFNR_Single;
            case MfllMode_AutoZhdrNormalMfll:
            case MfllMode_AutoZhdrNormalAis:
            case MfllMode_AutoZhdrZsdMfll:
            case MfllMode_AutoZhdrZsdAis:
                mfllLogD("%s: EIspProfile_Auto_zHDR_Capture_MFNR_Single", __FUNCTION__);
                return EIspProfile_zHDR_Capture_MFNR_Single;
        default:;
        }
        mfllLogD("%s: default EIspProfile_MFNR_Single", __FUNCTION__);
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

#define MIX_MAIN_GID_OUT            PASS2_CROPPING_ID_WDMAO
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
#define MFB_SUPPORT_CONF_MAP        0 // MT6757 not supports

/* group ID in MFB stage, if not defined which means not support crop */
#define MFB_GID_OUT_FRAME           1 // IMG2O group id in MFB stage

/******************************************************************************
 *  MIX
 *****************************************************************************/
/* port */
#define MIX_PORT_IN_BASE_FRAME      PORT_IMGI
#define MIX_PORT_IN_GOLDEN_FRAME    PORT_VIPI
#define MIX_PORT_IN_WEIGHTING       PORT_IMGBI
#define MIX_PORT_OUT_FRAME          PORT_IMG3O // using IMG3O for the best quality and bit true
#define MIX_PORT_OUT_MAIN           PORT_WDMAO
#define MIX_PORT_OUT_THUMBNAIL      PORT_WROTO


};/* namespace mfll */
#endif//__MFLLMFB_PLATFORM_H__
