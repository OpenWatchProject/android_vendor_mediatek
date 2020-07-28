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
#define LOG_TAG "isp_mgr_nr3d"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include <camera_custom_nvram.h>
#include "isp_mgr.h"
#include <mtkcam3/feature/3dnr/3dnr_isp_defs.h>
#include "common/hal/inc/camera_custom_3dnr_base.h"


namespace NSIspTuning
{

#define CLIP_NR3D(a,b,c)   ( (a<b)?  b : ((a>c)? c: a) )
#define ADD_AND_WRITE_R2CF_CNT(SubModuleIndex, NR3D_R2C_VAL, NR3D_R2CF_CNT)                                                                     \
    do {                                                                                                                        \
        MUINT32 R2C_VAL = reinterpret_cast<NR3D_REG_D1A_NR3D_NR3D_R2C_1*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, R2C_1))->Bits.NR3D_R2C_VAL;  \
        if (m_##NR3D_R2CF_CNT >= R2C_VAL)                                                                                       \
        {                                                                                                                       \
            m_##NR3D_R2CF_CNT = 0;                                                                                              \
        }                                                                                                                       \
        else                                                                                                                    \
        {                                                                                                                       \
            m_##NR3D_R2CF_CNT++;                                                                                                \
        }                                                                                                                       \
        reinterpret_cast<NR3D_REG_D1A_NR3D_NR3D_R2C_3*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, R2C_3))->Bits.NR3D_R2CF_CNT = m_##NR3D_R2CF_CNT;\
    } while (0)

static MVOID demoNR3D(MBOOL bEnableSL2E, MRect &fullImg, MRect &onRegion)
{
    CAM_LOGD("demo NR3D_EN(1), SL2E_EN(%d), x,y,w,h = (%d,%d,%d,%d), full x,y,w,h = (%d,%d,%d,%d)",
    bEnableSL2E, onRegion.p.x, onRegion.p.y, onRegion.s.w, onRegion.s.h,
    fullImg.p.x, fullImg.p.y, fullImg.s.w, fullImg.s.h);

    if (onRegion.p.x == 0)
    {
        onRegion.s.w =
            (onRegion.s.w >= fullImg.s.w/2)
            ? fullImg.s.w/2
            : onRegion.s.w;
        onRegion.s.w &= ~1;
    }
    else
    {
        MINT32 tmpVal = fullImg.s.w/2 - onRegion.p.x;
        if (tmpVal >= 0)
        {
            onRegion.s.w = tmpVal;
            onRegion.s.w &= ~1;
        }
        else
        {
            onRegion.s.w = 0;
        }
    }
}

/* strong setting */
/*
adb shell setprop debug.camera.3dnr.level 1
adb shell setprop debug.nr3d.bm.enable 1
adb shell setprop debug.nr3d.bm.q_nl 32
adb shell setprop debug.nr3d.bm.q_sp 0
*/
/* weak setting */
/*
adb shell setprop debug.camera.3dnr.level 1
adb shell setprop debug.nr3d.bm.enable 1
adb shell setprop debug.nr3d.bm.q_nl 12
adb shell setprop debug.nr3d.bm.q_sp 12
*/
static MVOID benchmarkNR3DRegValue(MVOID *pReg)
{
    if (pReg == NULL)
    {
        return;
    }
    dip_x_reg_t *pIspPhyReg = (dip_x_reg_t*) pReg;
    MINT32 iPrint3DNR_Reg = false;
    static MINT32 bmCount = 0;
    const MINT32 iPrintLogFreq = 120;
    bmCount++;

    iPrint3DNR_Reg= ::property_get_int32("vendor.debug.nr3d.bm.printreg",iPrint3DNR_Reg);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_ENG_CON.Bits.NR3D_yclnr_en= ::property_get_int32("vendor.debug.nr3d.bm.yclnr_en",pIspPhyReg->NR3D_D1A_NR3D_NR3D_ENG_CON.Bits.NR3D_yclnr_en);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_ENG_CON.Bits.NR3D_nr3d_lbit_mode= ::property_get_int32("vendor.debug.nr3d.bm.nr3d_lbit_mode",pIspPhyReg->NR3D_D1A_NR3D_NR3D_ENG_CON.Bits.NR3D_nr3d_lbit_mode);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_ENG_CON.Bits.NR3D_r2c_lbit_mode= ::property_get_int32("vendor.debug.nr3d.bm.r2c_lbit_mode",pIspPhyReg->NR3D_D1A_NR3D_NR3D_ENG_CON.Bits.NR3D_r2c_lbit_mode);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_ENG_CON.Bits.NR3D_in_rnd_mode= ::property_get_int32("vendor.debug.nr3d.bm.in_rnd_mode",pIspPhyReg->NR3D_D1A_NR3D_NR3D_ENG_CON.Bits.NR3D_in_rnd_mode);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_ENG_CON.Bits.NR3D_nr3d_uv_signed= ::property_get_int32("vendor.debug.nr3d.bm.nr3d_uv_signed",pIspPhyReg->NR3D_D1A_NR3D_NR3D_ENG_CON.Bits.NR3D_nr3d_uv_signed);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_ON_CON.Bits.NR3D_sl2_off= ::property_get_int32("vendor.debug.nr3d.bm.sl2_off",pIspPhyReg->NR3D_D1A_NR3D_NR3D_ON_CON.Bits.NR3D_sl2_off);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_TNR_ENABLE.Bits.NR3D_tnr_y_en= ::property_get_int32("vendor.debug.nr3d.bm.tnr_y_en",pIspPhyReg->NR3D_D1A_NR3D_NR3D_TNR_ENABLE.Bits.NR3D_tnr_y_en);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_TNR_ENABLE.Bits.NR3D_tnr_c_en= ::property_get_int32("vendor.debug.nr3d.bm.tnr_c_en",pIspPhyReg->NR3D_D1A_NR3D_NR3D_TNR_ENABLE.Bits.NR3D_tnr_c_en);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_FLT_CONFIG.Bits.NR3D_flt_str_max= ::property_get_int32("vendor.debug.nr3d.bm.flt_str_max",pIspPhyReg->NR3D_D1A_NR3D_NR3D_FLT_CONFIG.Bits.NR3D_flt_str_max);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_FLT_CONFIG.Bits.NR3D_en_cycr= ::property_get_int32("vendor.debug.nr3d.bm.en_cycr",pIspPhyReg->NR3D_D1A_NR3D_NR3D_FLT_CONFIG.Bits.NR3D_en_cycr);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_FLT_CONFIG.Bits.NR3D_en_ccr= ::property_get_int32("vendor.debug.nr3d.bm.en_ccr",pIspPhyReg->NR3D_D1A_NR3D_NR3D_FLT_CONFIG.Bits.NR3D_en_ccr);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_FB_INFO1.Bits.NR3D_q_nl= ::property_get_int32("vendor.debug.nr3d.bm.q_nl",pIspPhyReg->NR3D_D1A_NR3D_NR3D_FB_INFO1.Bits.NR3D_q_nl);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_FB_INFO1.Bits.NR3D_q_sp= ::property_get_int32("vendor.debug.nr3d.bm.q_sp",pIspPhyReg->NR3D_D1A_NR3D_NR3D_FB_INFO1.Bits.NR3D_q_sp);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_FLT_CONFIG.Bits.NR3D_blend_ratio_blky= ::property_get_int32("vendor.debug.nr3d.bm.blend_ratio_blky",pIspPhyReg->NR3D_D1A_NR3D_NR3D_FLT_CONFIG.Bits.NR3D_blend_ratio_blky);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_FLT_CONFIG.Bits.NR3D_blend_ratio_de= ::property_get_int32("vendor.debug.nr3d.bm.blend_ratio_de",pIspPhyReg->NR3D_D1A_NR3D_NR3D_FLT_CONFIG.Bits.NR3D_blend_ratio_de);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_FLT_CONFIG.Bits.NR3D_blend_ratio_txtr= ::property_get_int32("vendor.debug.nr3d.bm.blend_ratio_txtr",pIspPhyReg->NR3D_D1A_NR3D_NR3D_FLT_CONFIG.Bits.NR3D_blend_ratio_txtr);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_FLT_CONFIG.Bits.NR3D_blend_ratio_mv= ::property_get_int32("vendor.debug.nr3d.bm.blend_ratio_mv",pIspPhyReg->NR3D_D1A_NR3D_NR3D_FLT_CONFIG.Bits.NR3D_blend_ratio_mv);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_THR_1.Bits.NR3D_bdi_thr= ::property_get_int32("vendor.debug.nr3d.bm.bdi_thr",pIspPhyReg->NR3D_D1A_NR3D_NR3D_THR_1.Bits.NR3D_bdi_thr);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_THR_1.Bits.NR3D_mv_pen_w= ::property_get_int32("vendor.debug.nr3d.bm.mv_pen_w",pIspPhyReg->NR3D_D1A_NR3D_NR3D_THR_1.Bits.NR3D_mv_pen_w);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_THR_1.Bits.NR3D_mv_pen_thr= ::property_get_int32("vendor.debug.nr3d.bm.mv_pen_thr",pIspPhyReg->NR3D_D1A_NR3D_NR3D_THR_1.Bits.NR3D_mv_pen_thr);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_THR_1.Bits.NR3D_small_sad_thr= ::property_get_int32("vendor.debug.nr3d.bm.small_sad_thr",pIspPhyReg->NR3D_D1A_NR3D_NR3D_THR_1.Bits.NR3D_small_sad_thr);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_1.Bits.NR3D_q_blky_y0= ::property_get_int32("vendor.debug.nr3d.bm.q_blky_y0",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_1.Bits.NR3D_q_blky_y0);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_1.Bits.NR3D_q_blky_y1= ::property_get_int32("vendor.debug.nr3d.bm.q_blky_y1",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_1.Bits.NR3D_q_blky_y1);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_1.Bits.NR3D_q_blky_y2= ::property_get_int32("vendor.debug.nr3d.bm.q_blky_y2",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_1.Bits.NR3D_q_blky_y2);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_1.Bits.NR3D_q_blky_y3= ::property_get_int32("vendor.debug.nr3d.bm.q_blky_y3",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_1.Bits.NR3D_q_blky_y3);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_1.Bits.NR3D_q_blky_y4= ::property_get_int32("vendor.debug.nr3d.bm.q_blky_y4",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_1.Bits.NR3D_q_blky_y4);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_2.Bits.NR3D_q_blky_y5= ::property_get_int32("vendor.debug.nr3d.bm.q_blky_y5",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_2.Bits.NR3D_q_blky_y5);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_2.Bits.NR3D_q_blky_y6= ::property_get_int32("vendor.debug.nr3d.bm.q_blky_y6",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_2.Bits.NR3D_q_blky_y6);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_2.Bits.NR3D_q_blky_y7= ::property_get_int32("vendor.debug.nr3d.bm.q_blky_y7",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_2.Bits.NR3D_q_blky_y7);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_2.Bits.NR3D_q_blkc_y0= ::property_get_int32("vendor.debug.nr3d.bm.q_blkc_y0",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_2.Bits.NR3D_q_blkc_y0);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_2.Bits.NR3D_q_blkc_y1= ::property_get_int32("vendor.debug.nr3d.bm.q_blkc_y1",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_2.Bits.NR3D_q_blkc_y1);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_3.Bits.NR3D_q_blkc_y2= ::property_get_int32("vendor.debug.nr3d.bm.q_blkc_y2",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_3.Bits.NR3D_q_blkc_y2);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_3.Bits.NR3D_q_blkc_y3= ::property_get_int32("vendor.debug.nr3d.bm.q_blkc_y3",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_3.Bits.NR3D_q_blkc_y3);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_3.Bits.NR3D_q_blkc_y4= ::property_get_int32("vendor.debug.nr3d.bm.q_blkc_y4",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_3.Bits.NR3D_q_blkc_y4);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_3.Bits.NR3D_q_blkc_y5= ::property_get_int32("vendor.debug.nr3d.bm.q_blkc_y5",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_3.Bits.NR3D_q_blkc_y5);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_3.Bits.NR3D_q_blkc_y6= ::property_get_int32("vendor.debug.nr3d.bm.q_blkc_y6",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_3.Bits.NR3D_q_blkc_y6);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_4.Bits.NR3D_q_blkc_y7= ::property_get_int32("vendor.debug.nr3d.bm.q_blkc_y7",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_4.Bits.NR3D_q_blkc_y7);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_4.Bits.NR3D_q_detxtr_lvl_y0= ::property_get_int32("vendor.debug.nr3d.bm.q_detxtr_lvl_y0",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_4.Bits.NR3D_q_detxtr_lvl_y0);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_4.Bits.NR3D_q_detxtr_lvl_y1= ::property_get_int32("vendor.debug.nr3d.bm.q_detxtr_lvl_y1",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_4.Bits.NR3D_q_detxtr_lvl_y1);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_4.Bits.NR3D_q_detxtr_lvl_y2= ::property_get_int32("vendor.debug.nr3d.bm.q_detxtr_lvl_y2",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_4.Bits.NR3D_q_detxtr_lvl_y2);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_4.Bits.NR3D_q_detxtr_lvl_y3= ::property_get_int32("vendor.debug.nr3d.bm.q_detxtr_lvl_y3",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_4.Bits.NR3D_q_detxtr_lvl_y3);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_5.Bits.NR3D_q_detxtr_lvl_y4= ::property_get_int32("vendor.debug.nr3d.bm.q_detxtr_lvl_y4",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_5.Bits.NR3D_q_detxtr_lvl_y4);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_5.Bits.NR3D_q_detxtr_lvl_y5= ::property_get_int32("vendor.debug.nr3d.bm.q_detxtr_lvl_y5",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_5.Bits.NR3D_q_detxtr_lvl_y5);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_5.Bits.NR3D_q_detxtr_lvl_y6= ::property_get_int32("vendor.debug.nr3d.bm.q_detxtr_lvl_y6",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_5.Bits.NR3D_q_detxtr_lvl_y6);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_5.Bits.NR3D_q_detxtr_lvl_y7= ::property_get_int32("vendor.debug.nr3d.bm.q_detxtr_lvl_y7",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_5.Bits.NR3D_q_detxtr_lvl_y7);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_5.Bits.NR3D_q_de1_base_y0= ::property_get_int32("vendor.debug.nr3d.bm.q_de1_base_y0",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_5.Bits.NR3D_q_de1_base_y0);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_6.Bits.NR3D_q_de1_base_y1= ::property_get_int32("vendor.debug.nr3d.bm.q_de1_base_y1",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_6.Bits.NR3D_q_de1_base_y1);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_6.Bits.NR3D_q_de1_base_y2= ::property_get_int32("vendor.debug.nr3d.bm.q_de1_base_y2",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_6.Bits.NR3D_q_de1_base_y2);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_6.Bits.NR3D_q_de1_base_y3= ::property_get_int32("vendor.debug.nr3d.bm.q_de1_base_y3",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_6.Bits.NR3D_q_de1_base_y3);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_6.Bits.NR3D_q_de1_base_y4= ::property_get_int32("vendor.debug.nr3d.bm.q_de1_base_y4",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_6.Bits.NR3D_q_de1_base_y4);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_6.Bits.NR3D_q_de1_base_y5= ::property_get_int32("vendor.debug.nr3d.bm.q_de1_base_y5",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_6.Bits.NR3D_q_de1_base_y5);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_7.Bits.NR3D_q_de1_base_y6= ::property_get_int32("vendor.debug.nr3d.bm.q_de1_base_y6",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_7.Bits.NR3D_q_de1_base_y6);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_7.Bits.NR3D_q_de1_base_y7= ::property_get_int32("vendor.debug.nr3d.bm.q_de1_base_y7",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_7.Bits.NR3D_q_de1_base_y7);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_7.Bits.NR3D_q_de2txtr_base_y0= ::property_get_int32("vendor.debug.nr3d.bm.q_de2txtr_base_y0",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_7.Bits.NR3D_q_de2txtr_base_y0);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_7.Bits.NR3D_q_de2txtr_base_y1= ::property_get_int32("vendor.debug.nr3d.bm.q_de2txtr_base_y1",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_7.Bits.NR3D_q_de2txtr_base_y1);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_7.Bits.NR3D_q_de2txtr_base_y2= ::property_get_int32("vendor.debug.nr3d.bm.q_de2txtr_base_y2",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_7.Bits.NR3D_q_de2txtr_base_y2);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_8.Bits.NR3D_q_de2txtr_base_y3= ::property_get_int32("vendor.debug.nr3d.bm.q_de2txtr_base_y3",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_8.Bits.NR3D_q_de2txtr_base_y3);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_8.Bits.NR3D_q_de2txtr_base_y4= ::property_get_int32("vendor.debug.nr3d.bm.q_de2txtr_base_y4",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_8.Bits.NR3D_q_de2txtr_base_y4);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_8.Bits.NR3D_q_de2txtr_base_y5= ::property_get_int32("vendor.debug.nr3d.bm.q_de2txtr_base_y5",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_8.Bits.NR3D_q_de2txtr_base_y5);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_8.Bits.NR3D_q_de2txtr_base_y6= ::property_get_int32("vendor.debug.nr3d.bm.q_de2txtr_base_y6",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_8.Bits.NR3D_q_de2txtr_base_y6);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_8.Bits.NR3D_q_de2txtr_base_y7= ::property_get_int32("vendor.debug.nr3d.bm.q_de2txtr_base_y7",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_8.Bits.NR3D_q_de2txtr_base_y7);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_9.Bits.NR3D_q_mv_y0= ::property_get_int32("vendor.debug.nr3d.bm.q_mv_y0",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_9.Bits.NR3D_q_mv_y0);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_9.Bits.NR3D_q_mv_y1= ::property_get_int32("vendor.debug.nr3d.bm.q_mv_y1",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_9.Bits.NR3D_q_mv_y1);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_9.Bits.NR3D_q_mv_y2= ::property_get_int32("vendor.debug.nr3d.bm.q_mv_y2",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_9.Bits.NR3D_q_mv_y2);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_9.Bits.NR3D_q_mv_y3= ::property_get_int32("vendor.debug.nr3d.bm.q_mv_y3",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_9.Bits.NR3D_q_mv_y3);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_9.Bits.NR3D_q_mv_y4= ::property_get_int32("vendor.debug.nr3d.bm.q_mv_y4",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_9.Bits.NR3D_q_mv_y4);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_10.Bits.NR3D_q_mv_y5= ::property_get_int32("vendor.debug.nr3d.bm.q_mv_y5",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_10.Bits.NR3D_q_mv_y5);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_10.Bits.NR3D_q_mv_y6= ::property_get_int32("vendor.debug.nr3d.bm.q_mv_y6",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_10.Bits.NR3D_q_mv_y6);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_10.Bits.NR3D_q_mv_y7= ::property_get_int32("vendor.debug.nr3d.bm.q_mv_y7",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_10.Bits.NR3D_q_mv_y7);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_10.Bits.NR3D_q_wvar_y0= ::property_get_int32("vendor.debug.nr3d.bm.q_wvar_y0",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_10.Bits.NR3D_q_wvar_y0);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_10.Bits.NR3D_q_wvar_y1= ::property_get_int32("vendor.debug.nr3d.bm.q_wvar_y1",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_10.Bits.NR3D_q_wvar_y1);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_11.Bits.NR3D_q_wvar_y2= ::property_get_int32("vendor.debug.nr3d.bm.q_wvar_y2",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_11.Bits.NR3D_q_wvar_y2);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_11.Bits.NR3D_q_wvar_y3= ::property_get_int32("vendor.debug.nr3d.bm.q_wvar_y3",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_11.Bits.NR3D_q_wvar_y3);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_11.Bits.NR3D_q_wvar_y4= ::property_get_int32("vendor.debug.nr3d.bm.q_wvar_y4",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_11.Bits.NR3D_q_wvar_y4);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_11.Bits.NR3D_q_wvar_y5= ::property_get_int32("vendor.debug.nr3d.bm.q_wvar_y5",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_11.Bits.NR3D_q_wvar_y5);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_11.Bits.NR3D_q_wvar_y6= ::property_get_int32("vendor.debug.nr3d.bm.q_wvar_y6",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_11.Bits.NR3D_q_wvar_y6);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_12.Bits.NR3D_q_wvar_y7= ::property_get_int32("vendor.debug.nr3d.bm.q_wvar_y7",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_12.Bits.NR3D_q_wvar_y7);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_12.Bits.NR3D_q_wsm_y0= ::property_get_int32("vendor.debug.nr3d.bm.q_wsm_y0",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_12.Bits.NR3D_q_wsm_y0);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_12.Bits.NR3D_q_wsm_y1= ::property_get_int32("vendor.debug.nr3d.bm.q_wsm_y1",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_12.Bits.NR3D_q_wsm_y1);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_12.Bits.NR3D_q_wsm_y2= ::property_get_int32("vendor.debug.nr3d.bm.q_wsm_y2",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_12.Bits.NR3D_q_wsm_y2);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_12.Bits.NR3D_q_wsm_y3= ::property_get_int32("vendor.debug.nr3d.bm.q_wsm_y3",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_12.Bits.NR3D_q_wsm_y3);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_13.Bits.NR3D_q_wsm_y4= ::property_get_int32("vendor.debug.nr3d.bm.q_wsm_y4",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_13.Bits.NR3D_q_wsm_y4);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_13.Bits.NR3D_q_wsm_y5= ::property_get_int32("vendor.debug.nr3d.bm.q_wsm_y5",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_13.Bits.NR3D_q_wsm_y5);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_13.Bits.NR3D_q_wsm_y6= ::property_get_int32("vendor.debug.nr3d.bm.q_wsm_y6",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_13.Bits.NR3D_q_wsm_y6);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_13.Bits.NR3D_q_wsm_y7= ::property_get_int32("vendor.debug.nr3d.bm.q_wsm_y7",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_13.Bits.NR3D_q_wsm_y7);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_14.Bits.NR3D_q_sdl_y0= ::property_get_int32("vendor.debug.nr3d.bm.q_sdl_y0",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_14.Bits.NR3D_q_sdl_y0);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_14.Bits.NR3D_q_sdl_y1= ::property_get_int32("vendor.debug.nr3d.bm.q_sdl_y1",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_14.Bits.NR3D_q_sdl_y1);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_14.Bits.NR3D_q_sdl_y2= ::property_get_int32("vendor.debug.nr3d.bm.q_sdl_y2",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_14.Bits.NR3D_q_sdl_y2);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_14.Bits.NR3D_q_sdl_y3= ::property_get_int32("vendor.debug.nr3d.bm.q_sdl_y3",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_14.Bits.NR3D_q_sdl_y3);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_14.Bits.NR3D_q_sdl_y4= ::property_get_int32("vendor.debug.nr3d.bm.q_sdl_y4",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_14.Bits.NR3D_q_sdl_y4);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_15.Bits.NR3D_q_sdl_y5= ::property_get_int32("vendor.debug.nr3d.bm.q_sdl_y5",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_15.Bits.NR3D_q_sdl_y5);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_15.Bits.NR3D_q_sdl_y6= ::property_get_int32("vendor.debug.nr3d.bm.q_sdl_y6",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_15.Bits.NR3D_q_sdl_y6);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_15.Bits.NR3D_q_sdl_y7= ::property_get_int32("vendor.debug.nr3d.bm.q_sdl_y7",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_15.Bits.NR3D_q_sdl_y7);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_15.Bits.NR3D_q_sdl_y8= ::property_get_int32("vendor.debug.nr3d.bm.q_sdl_y8",pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_15.Bits.NR3D_q_sdl_y8);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_R2C_1.Bits.NR3D_r2c_val1= ::property_get_int32("vendor.debug.nr3d.bm.r2c_val1",pIspPhyReg->NR3D_D1A_NR3D_NR3D_R2C_1.Bits.NR3D_r2c_val1);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_R2C_1.Bits.NR3D_r2c_val2= ::property_get_int32("vendor.debug.nr3d.bm.r2c_val2",pIspPhyReg->NR3D_D1A_NR3D_NR3D_R2C_1.Bits.NR3D_r2c_val2);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_R2C_1.Bits.NR3D_r2c_val3= ::property_get_int32("vendor.debug.nr3d.bm.r2c_val3",pIspPhyReg->NR3D_D1A_NR3D_NR3D_R2C_1.Bits.NR3D_r2c_val3);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_R2C_1.Bits.NR3D_r2c_val4= ::property_get_int32("vendor.debug.nr3d.bm.r2c_val4",pIspPhyReg->NR3D_D1A_NR3D_NR3D_R2C_1.Bits.NR3D_r2c_val4);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_R2C_1.Bits.NR3D_r2cenc= ::property_get_int32("vendor.debug.nr3d.bm.r2cenc",pIspPhyReg->NR3D_D1A_NR3D_NR3D_R2C_1.Bits.NR3D_r2cenc);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_R2C_2.Bits.NR3D_r2c_txtr_thr1= ::property_get_int32("vendor.debug.nr3d.bm.r2c_txtr_thr1",pIspPhyReg->NR3D_D1A_NR3D_NR3D_R2C_2.Bits.NR3D_r2c_txtr_thr1);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_R2C_2.Bits.NR3D_r2c_txtr_thr2= ::property_get_int32("vendor.debug.nr3d.bm.r2c_txtr_thr2",pIspPhyReg->NR3D_D1A_NR3D_NR3D_R2C_2.Bits.NR3D_r2c_txtr_thr2);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_R2C_2.Bits.NR3D_r2c_txtr_thr3= ::property_get_int32("vendor.debug.nr3d.bm.r2c_txtr_thr3",pIspPhyReg->NR3D_D1A_NR3D_NR3D_R2C_2.Bits.NR3D_r2c_txtr_thr3);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_R2C_2.Bits.NR3D_r2c_txtr_thr4= ::property_get_int32("vendor.debug.nr3d.bm.r2c_txtr_thr4",pIspPhyReg->NR3D_D1A_NR3D_NR3D_R2C_2.Bits.NR3D_r2c_txtr_thr4);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_R2C_2.Bits.NR3D_r2c_txtr_throff= ::property_get_int32("vendor.debug.nr3d.bm.r2c_txtr_throff",pIspPhyReg->NR3D_D1A_NR3D_NR3D_R2C_2.Bits.NR3D_r2c_txtr_throff);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_1.Bits.NR3D_q_snr_tpre_y0= ::property_get_int32("vendor.debug.nr3d.bm.q_snr_tpre_y0",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_1.Bits.NR3D_q_snr_tpre_y0);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_1.Bits.NR3D_q_snr_tpre_y1= ::property_get_int32("vendor.debug.nr3d.bm.q_snr_tpre_y1",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_1.Bits.NR3D_q_snr_tpre_y1);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_1.Bits.NR3D_q_snr_tpre_y2= ::property_get_int32("vendor.debug.nr3d.bm.q_snr_tpre_y2",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_1.Bits.NR3D_q_snr_tpre_y2);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_1.Bits.NR3D_q_snr_tpre_y3= ::property_get_int32("vendor.debug.nr3d.bm.q_snr_tpre_y3",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_1.Bits.NR3D_q_snr_tpre_y3);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_1.Bits.NR3D_q_snr_tpre_y4= ::property_get_int32("vendor.debug.nr3d.bm.q_snr_tpre_y4",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_1.Bits.NR3D_q_snr_tpre_y4);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_2.Bits.NR3D_q_snr_tpre_y5= ::property_get_int32("vendor.debug.nr3d.bm.q_snr_tpre_y5",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_2.Bits.NR3D_q_snr_tpre_y5);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_2.Bits.NR3D_q_snr_tpre_y6= ::property_get_int32("vendor.debug.nr3d.bm.q_snr_tpre_y6",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_2.Bits.NR3D_q_snr_tpre_y6);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_2.Bits.NR3D_q_snr_tpre_y7= ::property_get_int32("vendor.debug.nr3d.bm.q_snr_tpre_y7",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_2.Bits.NR3D_q_snr_tpre_y7);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_2.Bits.NR3D_q_snr_tpre_y8= ::property_get_int32("vendor.debug.nr3d.bm.q_snr_tpre_y8",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_2.Bits.NR3D_q_snr_tpre_y8);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_2.Bits.NR3D_q_snr_txtr_y0= ::property_get_int32("vendor.debug.nr3d.bm.q_snr_txtr_y0",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_2.Bits.NR3D_q_snr_txtr_y0);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_3.Bits.NR3D_q_snr_txtr_y1= ::property_get_int32("vendor.debug.nr3d.bm.q_snr_txtr_y1",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_3.Bits.NR3D_q_snr_txtr_y1);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_3.Bits.NR3D_q_snr_txtr_y2= ::property_get_int32("vendor.debug.nr3d.bm.q_snr_txtr_y2",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_3.Bits.NR3D_q_snr_txtr_y2);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_3.Bits.NR3D_q_snr_txtr_y3= ::property_get_int32("vendor.debug.nr3d.bm.q_snr_txtr_y3",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_3.Bits.NR3D_q_snr_txtr_y3);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_3.Bits.NR3D_q_snr_txtr_y4= ::property_get_int32("vendor.debug.nr3d.bm.q_snr_txtr_y4",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_3.Bits.NR3D_q_snr_txtr_y4);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_3.Bits.NR3D_q_snr_txtr_y5= ::property_get_int32("vendor.debug.nr3d.bm.q_snr_txtr_y5",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_3.Bits.NR3D_q_snr_txtr_y5);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_4.Bits.NR3D_q_snr_txtr_y6= ::property_get_int32("vendor.debug.nr3d.bm.q_snr_txtr_y6",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_4.Bits.NR3D_q_snr_txtr_y6);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_4.Bits.NR3D_q_snr_txtr_y7= ::property_get_int32("vendor.debug.nr3d.bm.q_snr_txtr_y7",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_4.Bits.NR3D_q_snr_txtr_y7);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_4.Bits.NR3D_q_snr_luma_y0= ::property_get_int32("vendor.debug.nr3d.bm.q_snr_luma_y0",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_4.Bits.NR3D_q_snr_luma_y0);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_4.Bits.NR3D_q_snr_luma_y1= ::property_get_int32("vendor.debug.nr3d.bm.q_snr_luma_y1",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_4.Bits.NR3D_q_snr_luma_y1);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_4.Bits.NR3D_q_snr_luma_y2= ::property_get_int32("vendor.debug.nr3d.bm.q_snr_luma_y2",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_4.Bits.NR3D_q_snr_luma_y2);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_5.Bits.NR3D_q_snr_luma_y3= ::property_get_int32("vendor.debug.nr3d.bm.q_snr_luma_y3",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_5.Bits.NR3D_q_snr_luma_y3);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_5.Bits.NR3D_q_snr_luma_y4= ::property_get_int32("vendor.debug.nr3d.bm.q_snr_luma_y4",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_5.Bits.NR3D_q_snr_luma_y4);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_5.Bits.NR3D_q_snr_luma_y5= ::property_get_int32("vendor.debug.nr3d.bm.q_snr_luma_y5",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_5.Bits.NR3D_q_snr_luma_y5);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_5.Bits.NR3D_q_snr_luma_y6= ::property_get_int32("vendor.debug.nr3d.bm.q_snr_luma_y6",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_5.Bits.NR3D_q_snr_luma_y6);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_5.Bits.NR3D_q_snr_luma_y7= ::property_get_int32("vendor.debug.nr3d.bm.q_snr_luma_y7",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_5.Bits.NR3D_q_snr_luma_y7);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_6.Bits.NR3D_q_snr_luma_y8= ::property_get_int32("vendor.debug.nr3d.bm.q_snr_luma_y8",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_6.Bits.NR3D_q_snr_luma_y8);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_6.Bits.NR3D_q_snr_tpst_y0= ::property_get_int32("vendor.debug.nr3d.bm.q_snr_tpst_y0",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_6.Bits.NR3D_q_snr_tpst_y0);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_6.Bits.NR3D_q_snr_tpst_y1= ::property_get_int32("vendor.debug.nr3d.bm.q_snr_tpst_y1",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_6.Bits.NR3D_q_snr_tpst_y1);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_6.Bits.NR3D_q_snr_tpst_y2= ::property_get_int32("vendor.debug.nr3d.bm.q_snr_tpst_y2",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_6.Bits.NR3D_q_snr_tpst_y2);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_6.Bits.NR3D_q_snr_tpst_y3= ::property_get_int32("vendor.debug.nr3d.bm.q_snr_tpst_y3",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_6.Bits.NR3D_q_snr_tpst_y3);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_7.Bits.NR3D_q_snr_tpst_y4= ::property_get_int32("vendor.debug.nr3d.bm.q_snr_tpst_y4",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_7.Bits.NR3D_q_snr_tpst_y4);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_7.Bits.NR3D_q_snr_tpst_y5= ::property_get_int32("vendor.debug.nr3d.bm.q_snr_tpst_y5",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_7.Bits.NR3D_q_snr_tpst_y5);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_7.Bits.NR3D_q_snr_tpst_y6= ::property_get_int32("vendor.debug.nr3d.bm.q_snr_tpst_y6",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_7.Bits.NR3D_q_snr_tpst_y6);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_7.Bits.NR3D_q_snr_tpst_y7= ::property_get_int32("vendor.debug.nr3d.bm.q_snr_tpst_y7",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_7.Bits.NR3D_q_snr_tpst_y7);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_7.Bits.NR3D_q_snr_tpst_y8= ::property_get_int32("vendor.debug.nr3d.bm.q_snr_tpst_y8",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_7.Bits.NR3D_q_snr_tpst_y8);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CONTROL_1.Bits.NR3D_snr_en= ::property_get_int32("vendor.debug.nr3d.bm.snr_en",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CONTROL_1.Bits.NR3D_snr_en);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CONTROL_1.Bits.NR3D_snr_pre2d_en= ::property_get_int32("vendor.debug.nr3d.bm.snr_pre2d_en",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CONTROL_1.Bits.NR3D_snr_pre2d_en);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CONTROL_1.Bits.NR3D_snr_post2d_en= ::property_get_int32("vendor.debug.nr3d.bm.snr_post2d_en",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CONTROL_1.Bits.NR3D_snr_post2d_en);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CONTROL_1.Bits.NR3D_snr_tpre_en= ::property_get_int32("vendor.debug.nr3d.bm.snr_tpre_en",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CONTROL_1.Bits.NR3D_snr_tpre_en);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CONTROL_1.Bits.NR3D_snr_txtr_en= ::property_get_int32("vendor.debug.nr3d.bm.snr_txtr_en",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CONTROL_1.Bits.NR3D_snr_txtr_en);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CONTROL_1.Bits.NR3D_snr_luma_en= ::property_get_int32("vendor.debug.nr3d.bm.snr_luma_en",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CONTROL_1.Bits.NR3D_snr_luma_en);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CONTROL_1.Bits.NR3D_snr_blend_maxmin= ::property_get_int32("vendor.debug.nr3d.bm.snr_blend_maxmin",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CONTROL_1.Bits.NR3D_snr_blend_maxmin);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_THR_2.Bits.NR3D_blkvar_tolerance= ::property_get_int32("vendor.debug.nr3d.bm.blkvar_tolerance",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_THR_2.Bits.NR3D_blkvar_tolerance);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_THR_2.Bits.NR3D_centvar_tolerance= ::property_get_int32("vendor.debug.nr3d.bm.centvar_tolerance",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_THR_2.Bits.NR3D_centvar_tolerance);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_THR_2.Bits.NR3D_tolerance_clip= ::property_get_int32("vendor.debug.nr3d.bm.tolerance_clip",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_THR_2.Bits.NR3D_tolerance_clip);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_THR_3.Bits.NR3D_sad_invwei= ::property_get_int32("vendor.debug.nr3d.bm.sad_invwei",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_THR_3.Bits.NR3D_sad_invwei);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_THR_3.Bits.NR3D_cent_cand_wei= ::property_get_int32("vendor.debug.nr3d.bm.cent_cand_wei",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_THR_3.Bits.NR3D_cent_cand_wei);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_THR_4.Bits.NR3D_hfrr_hf_ratio= ::property_get_int32("vendor.debug.nr3d.bm.hfrr_hf_ratio",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_THR_4.Bits.NR3D_hfrr_hf_ratio);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_THR_4.Bits.NR3D_hfrr_mf_ratio= ::property_get_int32("vendor.debug.nr3d.bm.hfrr_mf_ratio",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_THR_4.Bits.NR3D_hfrr_mf_ratio);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_THR_4.Bits.NR3D_add_curr_ratio= ::property_get_int32("vendor.debug.nr3d.bm.add_curr_ratio",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_THR_4.Bits.NR3D_add_curr_ratio);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_THR_4.Bits.NR3D_hfrr_post2d_hf_ratio= ::property_get_int32("vendor.debug.nr3d.bm.hfrr_post2d_hf_ratio",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_THR_4.Bits.NR3D_hfrr_post2d_hf_ratio);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_THR_4.Bits.NR3D_hfrr_post2d_mf_ratio= ::property_get_int32("vendor.debug.nr3d.bm.hfrr_post2d_mf_ratio",pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_THR_4.Bits.NR3D_hfrr_post2d_mf_ratio);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DEMO_1.Bits.NR3D_demo_en= ::property_get_int32("vendor.debug.nr3d.bm.demo_en",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DEMO_1.Bits.NR3D_demo_en);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DEMO_1.Bits.NR3D_demo_sel= ::property_get_int32("vendor.debug.nr3d.bm.demo_sel",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DEMO_1.Bits.NR3D_demo_sel);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DEMO_1.Bits.NR3D_demo_top= ::property_get_int32("vendor.debug.nr3d.bm.demo_top",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DEMO_1.Bits.NR3D_demo_top);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DEMO_1.Bits.NR3D_demo_bot= ::property_get_int32("vendor.debug.nr3d.bm.demo_bot",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DEMO_1.Bits.NR3D_demo_bot);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DEMO_2.Bits.NR3D_demo_left= ::property_get_int32("vendor.debug.nr3d.bm.demo_left",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DEMO_2.Bits.NR3D_demo_left);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DEMO_2.Bits.NR3D_demo_right= ::property_get_int32("vendor.debug.nr3d.bm.demo_right",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DEMO_2.Bits.NR3D_demo_right);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_6.Bits.NR3D_force_en= ::property_get_int32("vendor.debug.nr3d.bm.force_en",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_6.Bits.NR3D_force_en);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_6.Bits.NR3D_force_flt_str= ::property_get_int32("vendor.debug.nr3d.bm.force_flt_str",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_6.Bits.NR3D_force_flt_str);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_6.Bits.NR3D_ink_en= ::property_get_int32("vendor.debug.nr3d.bm.ink_en",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_6.Bits.NR3D_ink_en);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_6.Bits.NR3D_ink_y_en= ::property_get_int32("vendor.debug.nr3d.bm.ink_y_en",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_6.Bits.NR3D_ink_y_en);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_6.Bits.NR3D_ink_sel= ::property_get_int32("vendor.debug.nr3d.bm.ink_sel",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_6.Bits.NR3D_ink_sel);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_6.Bits.NR3D_ink_level_disp= ::property_get_int32("vendor.debug.nr3d.bm.ink_level_disp",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_6.Bits.NR3D_ink_level_disp);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_15.Bits.NR3D_osd_en= ::property_get_int32("vendor.debug.nr3d.bm.osd_en",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_15.Bits.NR3D_osd_en);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_15.Bits.NR3D_osd_sel= ::property_get_int32("vendor.debug.nr3d.bm.osd_sel",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_15.Bits.NR3D_osd_sel);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_15.Bits.NR3D_osd_targh= ::property_get_int32("vendor.debug.nr3d.bm.osd_targh",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_15.Bits.NR3D_osd_targh);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_15.Bits.NR3D_osd_targv= ::property_get_int32("vendor.debug.nr3d.bm.osd_targv",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_15.Bits.NR3D_osd_targv);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_16.Bits.NR3D_osd_disph= ::property_get_int32("vendor.debug.nr3d.bm.osd_disph",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_16.Bits.NR3D_osd_disph);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_16.Bits.NR3D_osd_dispv= ::property_get_int32("vendor.debug.nr3d.bm.osd_dispv",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_16.Bits.NR3D_osd_dispv);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_16.Bits.NR3D_osd_disp_scale= ::property_get_int32("vendor.debug.nr3d.bm.osd_disp_scale",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_16.Bits.NR3D_osd_disp_scale);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_16.Bits.NR3D_osd_y_en= ::property_get_int32("vendor.debug.nr3d.bm.osd_y_en",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_16.Bits.NR3D_osd_y_en);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_9.Bits.NR3D_ink_color_y0= ::property_get_int32("vendor.debug.nr3d.bm.ink_color_y0",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_9.Bits.NR3D_ink_color_y0);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_9.Bits.NR3D_ink_color_y1= ::property_get_int32("vendor.debug.nr3d.bm.ink_color_y1",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_9.Bits.NR3D_ink_color_y1);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_9.Bits.NR3D_ink_color_y2= ::property_get_int32("vendor.debug.nr3d.bm.ink_color_y2",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_9.Bits.NR3D_ink_color_y2);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_9.Bits.NR3D_ink_color_y3= ::property_get_int32("vendor.debug.nr3d.bm.ink_color_y3",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_9.Bits.NR3D_ink_color_y3);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_9.Bits.NR3D_ink_color_y4= ::property_get_int32("vendor.debug.nr3d.bm.ink_color_y4",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_9.Bits.NR3D_ink_color_y4);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_9.Bits.NR3D_ink_color_y5= ::property_get_int32("vendor.debug.nr3d.bm.ink_color_y5",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_9.Bits.NR3D_ink_color_y5);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_9.Bits.NR3D_ink_color_y6= ::property_get_int32("vendor.debug.nr3d.bm.ink_color_y6",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_9.Bits.NR3D_ink_color_y6);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_9.Bits.NR3D_ink_color_y7= ::property_get_int32("vendor.debug.nr3d.bm.ink_color_y7",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_9.Bits.NR3D_ink_color_y7);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_10.Bits.NR3D_ink_color_y8= ::property_get_int32("vendor.debug.nr3d.bm.ink_color_y8",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_10.Bits.NR3D_ink_color_y8);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_10.Bits.NR3D_ink_color_y9= ::property_get_int32("vendor.debug.nr3d.bm.ink_color_y9",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_10.Bits.NR3D_ink_color_y9);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_10.Bits.NR3D_ink_color_y10= ::property_get_int32("vendor.debug.nr3d.bm.ink_color_y10",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_10.Bits.NR3D_ink_color_y10);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_10.Bits.NR3D_ink_color_y11= ::property_get_int32("vendor.debug.nr3d.bm.ink_color_y11",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_10.Bits.NR3D_ink_color_y11);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_10.Bits.NR3D_ink_color_y12= ::property_get_int32("vendor.debug.nr3d.bm.ink_color_y12",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_10.Bits.NR3D_ink_color_y12);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_10.Bits.NR3D_ink_color_y13= ::property_get_int32("vendor.debug.nr3d.bm.ink_color_y13",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_10.Bits.NR3D_ink_color_y13);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_10.Bits.NR3D_ink_color_y14= ::property_get_int32("vendor.debug.nr3d.bm.ink_color_y14",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_10.Bits.NR3D_ink_color_y14);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_10.Bits.NR3D_ink_color_y15= ::property_get_int32("vendor.debug.nr3d.bm.ink_color_y15",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_10.Bits.NR3D_ink_color_y15);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_11.Bits.NR3D_ink_color_c0= ::property_get_int32("vendor.debug.nr3d.bm.ink_color_c0",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_11.Bits.NR3D_ink_color_c0);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_11.Bits.NR3D_ink_color_c1= ::property_get_int32("vendor.debug.nr3d.bm.ink_color_c1",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_11.Bits.NR3D_ink_color_c1);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_11.Bits.NR3D_ink_color_c2= ::property_get_int32("vendor.debug.nr3d.bm.ink_color_c2",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_11.Bits.NR3D_ink_color_c2);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_11.Bits.NR3D_ink_color_c3= ::property_get_int32("vendor.debug.nr3d.bm.ink_color_c3",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_11.Bits.NR3D_ink_color_c3);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_11.Bits.NR3D_ink_color_c4= ::property_get_int32("vendor.debug.nr3d.bm.ink_color_c4",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_11.Bits.NR3D_ink_color_c4);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_11.Bits.NR3D_ink_color_c5= ::property_get_int32("vendor.debug.nr3d.bm.ink_color_c5",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_11.Bits.NR3D_ink_color_c5);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_11.Bits.NR3D_ink_color_c6= ::property_get_int32("vendor.debug.nr3d.bm.ink_color_c6",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_11.Bits.NR3D_ink_color_c6);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_11.Bits.NR3D_ink_color_c7= ::property_get_int32("vendor.debug.nr3d.bm.ink_color_c7",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_11.Bits.NR3D_ink_color_c7);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_12.Bits.NR3D_ink_color_c8= ::property_get_int32("vendor.debug.nr3d.bm.ink_color_c8",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_12.Bits.NR3D_ink_color_c8);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_12.Bits.NR3D_ink_color_c9= ::property_get_int32("vendor.debug.nr3d.bm.ink_color_c9",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_12.Bits.NR3D_ink_color_c9);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_12.Bits.NR3D_ink_color_c10= ::property_get_int32("vendor.debug.nr3d.bm.ink_color_c10",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_12.Bits.NR3D_ink_color_c10);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_12.Bits.NR3D_ink_color_c11= ::property_get_int32("vendor.debug.nr3d.bm.ink_color_c11",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_12.Bits.NR3D_ink_color_c11);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_12.Bits.NR3D_ink_color_c12= ::property_get_int32("vendor.debug.nr3d.bm.ink_color_c12",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_12.Bits.NR3D_ink_color_c12);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_12.Bits.NR3D_ink_color_c13= ::property_get_int32("vendor.debug.nr3d.bm.ink_color_c13",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_12.Bits.NR3D_ink_color_c13);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_12.Bits.NR3D_ink_color_c14= ::property_get_int32("vendor.debug.nr3d.bm.ink_color_c14",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_12.Bits.NR3D_ink_color_c14);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_12.Bits.NR3D_ink_color_c15= ::property_get_int32("vendor.debug.nr3d.bm.ink_color_c15",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_12.Bits.NR3D_ink_color_c15);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_7.Bits.NR3D_osd_color_bg_y= ::property_get_int32("vendor.debug.nr3d.bm.osd_color_bg_y",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_7.Bits.NR3D_osd_color_bg_y);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_7.Bits.NR3D_osd_color_bg_c= ::property_get_int32("vendor.debug.nr3d.bm.osd_color_bg_c",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_7.Bits.NR3D_osd_color_bg_c);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_7.Bits.NR3D_osd_color_fg0_y= ::property_get_int32("vendor.debug.nr3d.bm.osd_color_fg0_y",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_7.Bits.NR3D_osd_color_fg0_y);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_7.Bits.NR3D_osd_color_fg0_c= ::property_get_int32("vendor.debug.nr3d.bm.osd_color_fg0_c",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_7.Bits.NR3D_osd_color_fg0_c);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_7.Bits.NR3D_osd_color_fg1_y= ::property_get_int32("vendor.debug.nr3d.bm.osd_color_fg1_y",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_7.Bits.NR3D_osd_color_fg1_y);
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_7.Bits.NR3D_osd_color_fg1_c= ::property_get_int32("vendor.debug.nr3d.bm.osd_color_fg1_c",pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_7.Bits.NR3D_osd_color_fg1_c);

    MINT32 ADB_NR3D_SDL_03 = 0, ADB_NR3D_SDL_48 = 0;
    ADB_NR3D_SDL_03 = ::property_get_int32("vender.debug.nr3d.bm.sdl03", ADB_NR3D_SDL_03);
    ADB_NR3D_SDL_48 = ::property_get_int32("vender.debug.nr3d.bm.sdl47", ADB_NR3D_SDL_48);

    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_14.Bits.NR3D_q_sdl_y0=ADB_NR3D_SDL_03%100;
    ADB_NR3D_SDL_03=ADB_NR3D_SDL_03/100;
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_14.Bits.NR3D_q_sdl_y1=ADB_NR3D_SDL_03%100;
    ADB_NR3D_SDL_03=ADB_NR3D_SDL_03/100;
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_14.Bits.NR3D_q_sdl_y2=ADB_NR3D_SDL_03%100;
    ADB_NR3D_SDL_03=ADB_NR3D_SDL_03/100;
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_14.Bits.NR3D_q_sdl_y3=ADB_NR3D_SDL_03%100;

    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_14.Bits.NR3D_q_sdl_y4=ADB_NR3D_SDL_48%100;
    ADB_NR3D_SDL_48=ADB_NR3D_SDL_48/100;
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_15.Bits.NR3D_q_sdl_y5=ADB_NR3D_SDL_48%100;
    ADB_NR3D_SDL_48=ADB_NR3D_SDL_48/100;
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_15.Bits.NR3D_q_sdl_y6=ADB_NR3D_SDL_48%100;
    ADB_NR3D_SDL_48=ADB_NR3D_SDL_48/100;
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_15.Bits.NR3D_q_sdl_y7=ADB_NR3D_SDL_48%100;
    ADB_NR3D_SDL_48=ADB_NR3D_SDL_48/100;
    pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_15.Bits.NR3D_q_sdl_y8=ADB_NR3D_SDL_48%100;

    if (bmCount % iPrintLogFreq == 0 && iPrint3DNR_Reg) {
        CAM_LOGD("NR3D_yclnr_en = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_ENG_CON.Bits.NR3D_yclnr_en);
        CAM_LOGD("NR3D_nr3d_lbit_mode = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_ENG_CON.Bits.NR3D_nr3d_lbit_mode);
        CAM_LOGD("NR3D_r2c_lbit_mode = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_ENG_CON.Bits.NR3D_r2c_lbit_mode);
        CAM_LOGD("NR3D_in_rnd_mode = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_ENG_CON.Bits.NR3D_in_rnd_mode);
        CAM_LOGD("NR3D_nr3d_uv_signed = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_ENG_CON.Bits.NR3D_nr3d_uv_signed);
        CAM_LOGD("NR3D_sl2_off = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_ON_CON.Bits.NR3D_sl2_off);
        CAM_LOGD("NR3D_tnr_y_en = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_TNR_ENABLE.Bits.NR3D_tnr_y_en);
        CAM_LOGD("NR3D_tnr_c_en = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_TNR_ENABLE.Bits.NR3D_tnr_c_en);
        CAM_LOGD("NR3D_flt_str_max = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_FLT_CONFIG.Bits.NR3D_flt_str_max);
        CAM_LOGD("NR3D_en_cycr = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_FLT_CONFIG.Bits.NR3D_en_cycr);
        CAM_LOGD("NR3D_en_ccr = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_FLT_CONFIG.Bits.NR3D_en_ccr);
        CAM_LOGD("NR3D_q_nl = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_FB_INFO1.Bits.NR3D_q_nl);
        CAM_LOGD("NR3D_q_sp = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_FB_INFO1.Bits.NR3D_q_sp);
        CAM_LOGD("NR3D_blend_ratio_blky = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_FLT_CONFIG.Bits.NR3D_blend_ratio_blky);
        CAM_LOGD("NR3D_blend_ratio_de = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_FLT_CONFIG.Bits.NR3D_blend_ratio_de);
        CAM_LOGD("NR3D_blend_ratio_txtr = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_FLT_CONFIG.Bits.NR3D_blend_ratio_txtr);
        CAM_LOGD("NR3D_blend_ratio_mv = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_FLT_CONFIG.Bits.NR3D_blend_ratio_mv);
        CAM_LOGD("NR3D_bdi_thr = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_THR_1.Bits.NR3D_bdi_thr);
        CAM_LOGD("NR3D_mv_pen_w = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_THR_1.Bits.NR3D_mv_pen_w);
        CAM_LOGD("NR3D_mv_pen_thr = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_THR_1.Bits.NR3D_mv_pen_thr);
        CAM_LOGD("NR3D_small_sad_thr = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_THR_1.Bits.NR3D_small_sad_thr);
        CAM_LOGD("NR3D_q_blky_y0 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_1.Bits.NR3D_q_blky_y0);
        CAM_LOGD("NR3D_q_blky_y1 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_1.Bits.NR3D_q_blky_y1);
        CAM_LOGD("NR3D_q_blky_y2 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_1.Bits.NR3D_q_blky_y2);
        CAM_LOGD("NR3D_q_blky_y3 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_1.Bits.NR3D_q_blky_y3);
        CAM_LOGD("NR3D_q_blky_y4 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_1.Bits.NR3D_q_blky_y4);
        CAM_LOGD("NR3D_q_blky_y5 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_2.Bits.NR3D_q_blky_y5);
        CAM_LOGD("NR3D_q_blky_y6 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_2.Bits.NR3D_q_blky_y6);
        CAM_LOGD("NR3D_q_blky_y7 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_2.Bits.NR3D_q_blky_y7);
        CAM_LOGD("NR3D_q_blkc_y0 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_2.Bits.NR3D_q_blkc_y0);
        CAM_LOGD("NR3D_q_blkc_y1 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_2.Bits.NR3D_q_blkc_y1);
        CAM_LOGD("NR3D_q_blkc_y2 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_3.Bits.NR3D_q_blkc_y2);
        CAM_LOGD("NR3D_q_blkc_y3 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_3.Bits.NR3D_q_blkc_y3);
        CAM_LOGD("NR3D_q_blkc_y4 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_3.Bits.NR3D_q_blkc_y4);
        CAM_LOGD("NR3D_q_blkc_y5 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_3.Bits.NR3D_q_blkc_y5);
        CAM_LOGD("NR3D_q_blkc_y6 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_3.Bits.NR3D_q_blkc_y6);
        CAM_LOGD("NR3D_q_blkc_y7 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_4.Bits.NR3D_q_blkc_y7);
        CAM_LOGD("NR3D_q_detxtr_lvl_y0 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_4.Bits.NR3D_q_detxtr_lvl_y0);
        CAM_LOGD("NR3D_q_detxtr_lvl_y1 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_4.Bits.NR3D_q_detxtr_lvl_y1);
        CAM_LOGD("NR3D_q_detxtr_lvl_y2 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_4.Bits.NR3D_q_detxtr_lvl_y2);
        CAM_LOGD("NR3D_q_detxtr_lvl_y3 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_4.Bits.NR3D_q_detxtr_lvl_y3);
        CAM_LOGD("NR3D_q_detxtr_lvl_y4 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_5.Bits.NR3D_q_detxtr_lvl_y4);
        CAM_LOGD("NR3D_q_detxtr_lvl_y5 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_5.Bits.NR3D_q_detxtr_lvl_y5);
        CAM_LOGD("NR3D_q_detxtr_lvl_y6 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_5.Bits.NR3D_q_detxtr_lvl_y6);
        CAM_LOGD("NR3D_q_detxtr_lvl_y7 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_5.Bits.NR3D_q_detxtr_lvl_y7);
        CAM_LOGD("NR3D_q_de1_base_y0 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_5.Bits.NR3D_q_de1_base_y0);
        CAM_LOGD("NR3D_q_de1_base_y1 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_6.Bits.NR3D_q_de1_base_y1);
        CAM_LOGD("NR3D_q_de1_base_y2 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_6.Bits.NR3D_q_de1_base_y2);
        CAM_LOGD("NR3D_q_de1_base_y3 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_6.Bits.NR3D_q_de1_base_y3);
        CAM_LOGD("NR3D_q_de1_base_y4 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_6.Bits.NR3D_q_de1_base_y4);
        CAM_LOGD("NR3D_q_de1_base_y5 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_6.Bits.NR3D_q_de1_base_y5);
        CAM_LOGD("NR3D_q_de1_base_y6 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_7.Bits.NR3D_q_de1_base_y6);
        CAM_LOGD("NR3D_q_de1_base_y7 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_7.Bits.NR3D_q_de1_base_y7);
        CAM_LOGD("NR3D_q_de2txtr_base_y0 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_7.Bits.NR3D_q_de2txtr_base_y0);
        CAM_LOGD("NR3D_q_de2txtr_base_y1 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_7.Bits.NR3D_q_de2txtr_base_y1);
        CAM_LOGD("NR3D_q_de2txtr_base_y2 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_7.Bits.NR3D_q_de2txtr_base_y2);
        CAM_LOGD("NR3D_q_de2txtr_base_y3 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_8.Bits.NR3D_q_de2txtr_base_y3);
        CAM_LOGD("NR3D_q_de2txtr_base_y4 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_8.Bits.NR3D_q_de2txtr_base_y4);
        CAM_LOGD("NR3D_q_de2txtr_base_y5 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_8.Bits.NR3D_q_de2txtr_base_y5);
        CAM_LOGD("NR3D_q_de2txtr_base_y6 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_8.Bits.NR3D_q_de2txtr_base_y6);
        CAM_LOGD("NR3D_q_de2txtr_base_y7 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_8.Bits.NR3D_q_de2txtr_base_y7);
        CAM_LOGD("NR3D_q_mv_y0 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_9.Bits.NR3D_q_mv_y0);
        CAM_LOGD("NR3D_q_mv_y1 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_9.Bits.NR3D_q_mv_y1);
        CAM_LOGD("NR3D_q_mv_y2 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_9.Bits.NR3D_q_mv_y2);
        CAM_LOGD("NR3D_q_mv_y3 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_9.Bits.NR3D_q_mv_y3);
        CAM_LOGD("NR3D_q_mv_y4 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_9.Bits.NR3D_q_mv_y4);
        CAM_LOGD("NR3D_q_mv_y5 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_10.Bits.NR3D_q_mv_y5);
        CAM_LOGD("NR3D_q_mv_y6 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_10.Bits.NR3D_q_mv_y6);
        CAM_LOGD("NR3D_q_mv_y7 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_10.Bits.NR3D_q_mv_y7);
        CAM_LOGD("NR3D_q_wvar_y0 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_10.Bits.NR3D_q_wvar_y0);
        CAM_LOGD("NR3D_q_wvar_y1 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_10.Bits.NR3D_q_wvar_y1);
        CAM_LOGD("NR3D_q_wvar_y2 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_11.Bits.NR3D_q_wvar_y2);
        CAM_LOGD("NR3D_q_wvar_y3 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_11.Bits.NR3D_q_wvar_y3);
        CAM_LOGD("NR3D_q_wvar_y4 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_11.Bits.NR3D_q_wvar_y4);
        CAM_LOGD("NR3D_q_wvar_y5 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_11.Bits.NR3D_q_wvar_y5);
        CAM_LOGD("NR3D_q_wvar_y6 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_11.Bits.NR3D_q_wvar_y6);
        CAM_LOGD("NR3D_q_wvar_y7 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_12.Bits.NR3D_q_wvar_y7);
        CAM_LOGD("NR3D_q_wsm_y0 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_12.Bits.NR3D_q_wsm_y0);
        CAM_LOGD("NR3D_q_wsm_y1 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_12.Bits.NR3D_q_wsm_y1);
        CAM_LOGD("NR3D_q_wsm_y2 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_12.Bits.NR3D_q_wsm_y2);
        CAM_LOGD("NR3D_q_wsm_y3 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_12.Bits.NR3D_q_wsm_y3);
        CAM_LOGD("NR3D_q_wsm_y4 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_13.Bits.NR3D_q_wsm_y4);
        CAM_LOGD("NR3D_q_wsm_y5 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_13.Bits.NR3D_q_wsm_y5);
        CAM_LOGD("NR3D_q_wsm_y6 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_13.Bits.NR3D_q_wsm_y6);
        CAM_LOGD("NR3D_q_wsm_y7 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_13.Bits.NR3D_q_wsm_y7);
        CAM_LOGD("NR3D_q_sdl_y0 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_14.Bits.NR3D_q_sdl_y0);
        CAM_LOGD("NR3D_q_sdl_y1 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_14.Bits.NR3D_q_sdl_y1);
        CAM_LOGD("NR3D_q_sdl_y2 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_14.Bits.NR3D_q_sdl_y2);
        CAM_LOGD("NR3D_q_sdl_y3 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_14.Bits.NR3D_q_sdl_y3);
        CAM_LOGD("NR3D_q_sdl_y4 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_14.Bits.NR3D_q_sdl_y4);
        CAM_LOGD("NR3D_q_sdl_y5 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_15.Bits.NR3D_q_sdl_y5);
        CAM_LOGD("NR3D_q_sdl_y6 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_15.Bits.NR3D_q_sdl_y6);
        CAM_LOGD("NR3D_q_sdl_y7 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_15.Bits.NR3D_q_sdl_y7);
        CAM_LOGD("NR3D_q_sdl_y8 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_CURVE_15.Bits.NR3D_q_sdl_y8);
        CAM_LOGD("NR3D_r2c_val1 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_R2C_1.Bits.NR3D_r2c_val1);
        CAM_LOGD("NR3D_r2c_val2 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_R2C_1.Bits.NR3D_r2c_val2);
        CAM_LOGD("NR3D_r2c_val3 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_R2C_1.Bits.NR3D_r2c_val3);
        CAM_LOGD("NR3D_r2c_val4 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_R2C_1.Bits.NR3D_r2c_val4);
        CAM_LOGD("NR3D_r2cenc = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_R2C_1.Bits.NR3D_r2cenc);
        CAM_LOGD("NR3D_r2c_txtr_thr1 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_R2C_2.Bits.NR3D_r2c_txtr_thr1);
        CAM_LOGD("NR3D_r2c_txtr_thr2 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_R2C_2.Bits.NR3D_r2c_txtr_thr2);
        CAM_LOGD("NR3D_r2c_txtr_thr3 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_R2C_2.Bits.NR3D_r2c_txtr_thr3);
        CAM_LOGD("NR3D_r2c_txtr_thr4 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_R2C_2.Bits.NR3D_r2c_txtr_thr4);
        CAM_LOGD("NR3D_r2c_txtr_throff = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_R2C_2.Bits.NR3D_r2c_txtr_throff);
        CAM_LOGD("NR3D_q_snr_tpre_y0 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_1.Bits.NR3D_q_snr_tpre_y0);
        CAM_LOGD("NR3D_q_snr_tpre_y1 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_1.Bits.NR3D_q_snr_tpre_y1);
        CAM_LOGD("NR3D_q_snr_tpre_y2 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_1.Bits.NR3D_q_snr_tpre_y2);
        CAM_LOGD("NR3D_q_snr_tpre_y3 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_1.Bits.NR3D_q_snr_tpre_y3);
        CAM_LOGD("NR3D_q_snr_tpre_y4 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_1.Bits.NR3D_q_snr_tpre_y4);
        CAM_LOGD("NR3D_q_snr_tpre_y5 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_2.Bits.NR3D_q_snr_tpre_y5);
        CAM_LOGD("NR3D_q_snr_tpre_y6 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_2.Bits.NR3D_q_snr_tpre_y6);
        CAM_LOGD("NR3D_q_snr_tpre_y7 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_2.Bits.NR3D_q_snr_tpre_y7);
        CAM_LOGD("NR3D_q_snr_tpre_y8 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_2.Bits.NR3D_q_snr_tpre_y8);
        CAM_LOGD("NR3D_q_snr_txtr_y0 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_2.Bits.NR3D_q_snr_txtr_y0);
        CAM_LOGD("NR3D_q_snr_txtr_y1 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_3.Bits.NR3D_q_snr_txtr_y1);
        CAM_LOGD("NR3D_q_snr_txtr_y2 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_3.Bits.NR3D_q_snr_txtr_y2);
        CAM_LOGD("NR3D_q_snr_txtr_y3 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_3.Bits.NR3D_q_snr_txtr_y3);
        CAM_LOGD("NR3D_q_snr_txtr_y4 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_3.Bits.NR3D_q_snr_txtr_y4);
        CAM_LOGD("NR3D_q_snr_txtr_y5 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_3.Bits.NR3D_q_snr_txtr_y5);
        CAM_LOGD("NR3D_q_snr_txtr_y6 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_4.Bits.NR3D_q_snr_txtr_y6);
        CAM_LOGD("NR3D_q_snr_txtr_y7 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_4.Bits.NR3D_q_snr_txtr_y7);
        CAM_LOGD("NR3D_q_snr_luma_y0 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_4.Bits.NR3D_q_snr_luma_y0);
        CAM_LOGD("NR3D_q_snr_luma_y1 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_4.Bits.NR3D_q_snr_luma_y1);
        CAM_LOGD("NR3D_q_snr_luma_y2 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_4.Bits.NR3D_q_snr_luma_y2);
        CAM_LOGD("NR3D_q_snr_luma_y3 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_5.Bits.NR3D_q_snr_luma_y3);
        CAM_LOGD("NR3D_q_snr_luma_y4 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_5.Bits.NR3D_q_snr_luma_y4);
        CAM_LOGD("NR3D_q_snr_luma_y5 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_5.Bits.NR3D_q_snr_luma_y5);
        CAM_LOGD("NR3D_q_snr_luma_y6 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_5.Bits.NR3D_q_snr_luma_y6);
        CAM_LOGD("NR3D_q_snr_luma_y7 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_5.Bits.NR3D_q_snr_luma_y7);
        CAM_LOGD("NR3D_q_snr_luma_y8 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_6.Bits.NR3D_q_snr_luma_y8);
        CAM_LOGD("NR3D_q_snr_tpst_y0 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_6.Bits.NR3D_q_snr_tpst_y0);
        CAM_LOGD("NR3D_q_snr_tpst_y1 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_6.Bits.NR3D_q_snr_tpst_y1);
        CAM_LOGD("NR3D_q_snr_tpst_y2 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_6.Bits.NR3D_q_snr_tpst_y2);
        CAM_LOGD("NR3D_q_snr_tpst_y3 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_6.Bits.NR3D_q_snr_tpst_y3);
        CAM_LOGD("NR3D_q_snr_tpst_y4 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_7.Bits.NR3D_q_snr_tpst_y4);
        CAM_LOGD("NR3D_q_snr_tpst_y5 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_7.Bits.NR3D_q_snr_tpst_y5);
        CAM_LOGD("NR3D_q_snr_tpst_y6 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_7.Bits.NR3D_q_snr_tpst_y6);
        CAM_LOGD("NR3D_q_snr_tpst_y7 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_7.Bits.NR3D_q_snr_tpst_y7);
        CAM_LOGD("NR3D_q_snr_tpst_y8 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CURVE_7.Bits.NR3D_q_snr_tpst_y8);
        CAM_LOGD("NR3D_snr_en = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CONTROL_1.Bits.NR3D_snr_en);
        CAM_LOGD("NR3D_snr_pre2d_en = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CONTROL_1.Bits.NR3D_snr_pre2d_en);
        CAM_LOGD("NR3D_snr_post2d_en = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CONTROL_1.Bits.NR3D_snr_post2d_en);
        CAM_LOGD("NR3D_snr_tpre_en = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CONTROL_1.Bits.NR3D_snr_tpre_en);
        CAM_LOGD("NR3D_snr_txtr_en = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CONTROL_1.Bits.NR3D_snr_txtr_en);
        CAM_LOGD("NR3D_snr_luma_en = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CONTROL_1.Bits.NR3D_snr_luma_en);
        CAM_LOGD("NR3D_snr_blend_maxmin = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_CONTROL_1.Bits.NR3D_snr_blend_maxmin);
        CAM_LOGD("NR3D_blkvar_tolerance = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_THR_2.Bits.NR3D_blkvar_tolerance);
        CAM_LOGD("NR3D_centvar_tolerance = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_THR_2.Bits.NR3D_centvar_tolerance);
        CAM_LOGD("NR3D_tolerance_clip = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_THR_2.Bits.NR3D_tolerance_clip);
        CAM_LOGD("NR3D_sad_invwei = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_THR_3.Bits.NR3D_sad_invwei);
        CAM_LOGD("NR3D_cent_cand_wei = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_THR_3.Bits.NR3D_cent_cand_wei);
        CAM_LOGD("NR3D_hfrr_hf_ratio = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_THR_4.Bits.NR3D_hfrr_hf_ratio);
        CAM_LOGD("NR3D_hfrr_mf_ratio = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_THR_4.Bits.NR3D_hfrr_mf_ratio);
        CAM_LOGD("NR3D_add_curr_ratio = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_THR_4.Bits.NR3D_add_curr_ratio);
        CAM_LOGD("NR3D_hfrr_post2d_hf_ratio = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_THR_4.Bits.NR3D_hfrr_post2d_hf_ratio);
        CAM_LOGD("NR3D_hfrr_post2d_mf_ratio = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_SNR_THR_4.Bits.NR3D_hfrr_post2d_mf_ratio);
        CAM_LOGD("NR3D_demo_en = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DEMO_1.Bits.NR3D_demo_en);
        CAM_LOGD("NR3D_demo_sel = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DEMO_1.Bits.NR3D_demo_sel);
        CAM_LOGD("NR3D_demo_top = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DEMO_1.Bits.NR3D_demo_top);
        CAM_LOGD("NR3D_demo_bot = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DEMO_1.Bits.NR3D_demo_bot);
        CAM_LOGD("NR3D_demo_left = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DEMO_2.Bits.NR3D_demo_left);
        CAM_LOGD("NR3D_demo_right = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DEMO_2.Bits.NR3D_demo_right);
        CAM_LOGD("NR3D_force_en = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_6.Bits.NR3D_force_en);
        CAM_LOGD("NR3D_force_flt_str = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_6.Bits.NR3D_force_flt_str);
        CAM_LOGD("NR3D_ink_en = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_6.Bits.NR3D_ink_en);
        CAM_LOGD("NR3D_ink_y_en = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_6.Bits.NR3D_ink_y_en);
        CAM_LOGD("NR3D_ink_sel = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_6.Bits.NR3D_ink_sel);
        CAM_LOGD("NR3D_ink_level_disp = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_6.Bits.NR3D_ink_level_disp);
        CAM_LOGD("NR3D_osd_en = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_15.Bits.NR3D_osd_en);
        CAM_LOGD("NR3D_osd_sel = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_15.Bits.NR3D_osd_sel);
        CAM_LOGD("NR3D_osd_targh = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_15.Bits.NR3D_osd_targh);
        CAM_LOGD("NR3D_osd_targv = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_15.Bits.NR3D_osd_targv);
        CAM_LOGD("NR3D_osd_disph = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_16.Bits.NR3D_osd_disph);
        CAM_LOGD("NR3D_osd_dispv = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_16.Bits.NR3D_osd_dispv);
        CAM_LOGD("NR3D_osd_disp_scale = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_16.Bits.NR3D_osd_disp_scale);
        CAM_LOGD("NR3D_osd_y_en = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_16.Bits.NR3D_osd_y_en);
        CAM_LOGD("NR3D_ink_color_y0 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_9.Bits.NR3D_ink_color_y0);
        CAM_LOGD("NR3D_ink_color_y1 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_9.Bits.NR3D_ink_color_y1);
        CAM_LOGD("NR3D_ink_color_y2 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_9.Bits.NR3D_ink_color_y2);
        CAM_LOGD("NR3D_ink_color_y3 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_9.Bits.NR3D_ink_color_y3);
        CAM_LOGD("NR3D_ink_color_y4 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_9.Bits.NR3D_ink_color_y4);
        CAM_LOGD("NR3D_ink_color_y5 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_9.Bits.NR3D_ink_color_y5);
        CAM_LOGD("NR3D_ink_color_y6 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_9.Bits.NR3D_ink_color_y6);
        CAM_LOGD("NR3D_ink_color_y7 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_9.Bits.NR3D_ink_color_y7);
        CAM_LOGD("NR3D_ink_color_y8 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_10.Bits.NR3D_ink_color_y8);
        CAM_LOGD("NR3D_ink_color_y9 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_10.Bits.NR3D_ink_color_y9);
        CAM_LOGD("NR3D_ink_color_y10 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_10.Bits.NR3D_ink_color_y10);
        CAM_LOGD("NR3D_ink_color_y11 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_10.Bits.NR3D_ink_color_y11);
        CAM_LOGD("NR3D_ink_color_y12 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_10.Bits.NR3D_ink_color_y12);
        CAM_LOGD("NR3D_ink_color_y13 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_10.Bits.NR3D_ink_color_y13);
        CAM_LOGD("NR3D_ink_color_y14 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_10.Bits.NR3D_ink_color_y14);
        CAM_LOGD("NR3D_ink_color_y15 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_10.Bits.NR3D_ink_color_y15);
        CAM_LOGD("NR3D_ink_color_c0 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_11.Bits.NR3D_ink_color_c0);
        CAM_LOGD("NR3D_ink_color_c1 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_11.Bits.NR3D_ink_color_c1);
        CAM_LOGD("NR3D_ink_color_c2 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_11.Bits.NR3D_ink_color_c2);
        CAM_LOGD("NR3D_ink_color_c3 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_11.Bits.NR3D_ink_color_c3);
        CAM_LOGD("NR3D_ink_color_c4 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_11.Bits.NR3D_ink_color_c4);
        CAM_LOGD("NR3D_ink_color_c5 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_11.Bits.NR3D_ink_color_c5);
        CAM_LOGD("NR3D_ink_color_c6 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_11.Bits.NR3D_ink_color_c6);
        CAM_LOGD("NR3D_ink_color_c7 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_11.Bits.NR3D_ink_color_c7);
        CAM_LOGD("NR3D_ink_color_c8 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_12.Bits.NR3D_ink_color_c8);
        CAM_LOGD("NR3D_ink_color_c9 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_12.Bits.NR3D_ink_color_c9);
        CAM_LOGD("NR3D_ink_color_c10 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_12.Bits.NR3D_ink_color_c10);
        CAM_LOGD("NR3D_ink_color_c11 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_12.Bits.NR3D_ink_color_c11);
        CAM_LOGD("NR3D_ink_color_c12 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_12.Bits.NR3D_ink_color_c12);
        CAM_LOGD("NR3D_ink_color_c13 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_12.Bits.NR3D_ink_color_c13);
        CAM_LOGD("NR3D_ink_color_c14 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_12.Bits.NR3D_ink_color_c14);
        CAM_LOGD("NR3D_ink_color_c15 = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_12.Bits.NR3D_ink_color_c15);
        CAM_LOGD("NR3D_osd_color_bg_y = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_7.Bits.NR3D_osd_color_bg_y);
        CAM_LOGD("NR3D_osd_color_bg_c = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_7.Bits.NR3D_osd_color_bg_c);
        CAM_LOGD("NR3D_osd_color_fg0_y = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_7.Bits.NR3D_osd_color_fg0_y);
        CAM_LOGD("NR3D_osd_color_fg0_c = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_7.Bits.NR3D_osd_color_fg0_c);
        CAM_LOGD("NR3D_osd_color_fg1_y = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_7.Bits.NR3D_osd_color_fg1_y);
        CAM_LOGD("NR3D_osd_color_fg1_c = %d", pIspPhyReg->NR3D_D1A_NR3D_NR3D_DBG_7.Bits.NR3D_osd_color_fg1_c);
    }
}

static MVOID fixTuningValue(dip_x_reg_t *pReg)
{

#if 0
    if (pReg == NULL)
    {
        return;
    }

    dip_x_reg_t *pIspPhyReg = pReg;

    ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_MDP_TNR_TNR_ENABLE, (0x1 << 31) | (0x1 << 30));
    ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_MDP_TNR_FLT_CONFIG, (0x8 << 27) | (0x8 << 22) |
        (0x8 << 17) | (0x8 << 12) | (0x1C << 6));
    ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_MDP_TNR_FB_INFO1, (0x4 << 26) | (0x14 << 20));
    ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_MDP_TNR_THR_1, (0x4 << 26) | (0x8 << 22) | (0x20 << 16) | (0xA << 10));
    ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_MDP_TNR_CURVE_1, (0x1C << 26) | (0x1E << 20) |
        (0x20 << 14) | (0x1A << 8) | (0x10 << 2));
    ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_MDP_TNR_CURVE_2, (0x4 << 26) | (0x0 << 20) |
        (0x0 << 14) | (0x18 << 8) | (0x1A << 2));
    ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_MDP_TNR_CURVE_3, (0x8 << 26) | (0x2 << 20) |
        (0x0 << 14) | (0x0 << 8) | (0x0 << 2));
    ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_MDP_TNR_CURVE_4, (0x0 << 26) | (0x0 << 20) |
        (0x8 << 14) | (0x10 << 8) | (0x15 << 2));
    ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_MDP_TNR_CURVE_5, (0x1B << 26) | (0x20 << 20) |
        (0x20 << 14) | (0x20 << 8) | (0x20 << 2));
    ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_MDP_TNR_CURVE_6, (0x20 << 26) | (0x1B << 20) |
        (0x15 << 14) | (0xC << 8) | (0x0 << 2));
    ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_MDP_TNR_CURVE_7, (0x0 << 26) | (0x0 << 20) |
        (0x0 << 14) | (0x5 << 8) | (0x16 << 2));
    ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_MDP_TNR_CURVE_8, (0x20 << 26) | (0x20 << 20) |
        (0x20 << 14) | (0x20 << 8) | (0x20 << 2));
    ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_MDP_TNR_CURVE_9, (0x0 << 26) | (0x3 << 20) |
        (0x7 << 14) | (0xE << 8) | (0x14 << 2));
    ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_MDP_TNR_CURVE_10, (0x1A << 26) | (0x1E << 20) |
        (0x20 << 14) | (0x10 << 8) | (0x10 << 2));
    ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_MDP_TNR_CURVE_11, (0x6 << 26) | (0x3 << 20) |
        (0x2 << 14) | (0x1 << 8) | (0x1 << 2));
    ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_MDP_TNR_CURVE_12, (0x1 << 26) | (0x0 << 20) |
        (0x1 << 14) | (0x3 << 8) | (0x9 << 2));
    ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_MDP_TNR_CURVE_13, (0xE << 26) | (0x10 << 20) |
        (0x10 << 14) | (0x10 << 8));
    ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_MDP_TNR_CURVE_14, (0x3F << 26) | (0x38 << 20) |
        (0x30 << 14) | (0x28 << 8) | (0x20 << 2));
    ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_MDP_TNR_CURVE_15, (0x18 << 26) | (0x10 << 20) |
        (0x8 << 14) | (0x0 << 8));
#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// NR3D
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_NR3D_T&
ISP_MGR_NR3D_T::
getInstance(ESensorDev_T const eSensorDev)
{
    ISP_MGR_MODULE_GET_INSTANCE(NR3D);
}

template <>
ISP_MGR_NR3D_T&
ISP_MGR_NR3D_T::
put(MUINT8 SubModuleIndex, ISP_NVRAM_NR3D_T const& rParam)
{
    //PUT_REG_INFO_MULTI(SubModuleIndex, ENG_CON,         eng_con);
    //PUT_REG_INFO_MULTI(SubModuleIndex, SIZ,             siz);
    //PUT_REG_INFO_MULTI(SubModuleIndex, TILE_XY,         tile_xy);
    PUT_REG_INFO_MULTI(SubModuleIndex, ON_CON,          on_con);
    //PUT_REG_INFO_MULTI(SubModuleIndex, ON_OFF,          on_off);
    //PUT_REG_INFO_MULTI(SubModuleIndex, ON_SIZ,          on_siz);
    PUT_REG_INFO_MULTI(SubModuleIndex, TNR_ENABLE,      tnr_enable);
    PUT_REG_INFO_MULTI(SubModuleIndex, FLT_CONFIG,      flt_config);
    PUT_REG_INFO_MULTI(SubModuleIndex, FB_INFO1,        fb_info1);
    PUT_REG_INFO_MULTI(SubModuleIndex, THR_1,           thr_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, CURVE_1,         curve_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, CURVE_2,         curve_2);
    PUT_REG_INFO_MULTI(SubModuleIndex, CURVE_3,         curve_3);
    PUT_REG_INFO_MULTI(SubModuleIndex, CURVE_4,         curve_4);
    PUT_REG_INFO_MULTI(SubModuleIndex, CURVE_5,         curve_5);
    PUT_REG_INFO_MULTI(SubModuleIndex, CURVE_6,         curve_6);
    PUT_REG_INFO_MULTI(SubModuleIndex, CURVE_7,         curve_7);
    PUT_REG_INFO_MULTI(SubModuleIndex, CURVE_8,         curve_8);
    PUT_REG_INFO_MULTI(SubModuleIndex, CURVE_9,         curve_9);
    PUT_REG_INFO_MULTI(SubModuleIndex, CURVE_10,        curve_10);
    PUT_REG_INFO_MULTI(SubModuleIndex, CURVE_11,        curve_11);
    PUT_REG_INFO_MULTI(SubModuleIndex, CURVE_12,        curve_12);
    PUT_REG_INFO_MULTI(SubModuleIndex, CURVE_13,        curve_13);
    PUT_REG_INFO_MULTI(SubModuleIndex, CURVE_14,        curve_14);
    PUT_REG_INFO_MULTI(SubModuleIndex, CURVE_15,        curve_15);
    PUT_REG_INFO_MULTI(SubModuleIndex, R2C_1,           r2c_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, R2C_2,           r2c_2);
    //PUT_REG_INFO_MULTI(SubModuleIndex, R2C_3,           r2c_3);
    //PUT_REG_INFO_MULTI(SubModuleIndex, DBG_6,           dbg_6);
    //PUT_REG_INFO_MULTI(SubModuleIndex, DBG_15,          dbg_15);
    //PUT_REG_INFO_MULTI(SubModuleIndex, DBG_16,          dbg_16);
    //PUT_REG_INFO_MULTI(SubModuleIndex, DEMO_1,          demo_1);
    //PUT_REG_INFO_MULTI(SubModuleIndex, DEMO_2,          demo_2);
    //PUT_REG_INFO_MULTI(SubModuleIndex, ATPG,            atpg);
    //PUT_REG_INFO_MULTI(SubModuleIndex, DMY_0,           dmy_0);
    //PUT_REG_INFO_MULTI(SubModuleIndex, DBG_17,          dbg_17);
    //PUT_REG_INFO_MULTI(SubModuleIndex, INTERR,          interr);
    //PUT_REG_INFO_MULTI(SubModuleIndex, FB_INFO2,        fb_info2);
    //PUT_REG_INFO_MULTI(SubModuleIndex, FB_INFO3,        fb_info3);
    //PUT_REG_INFO_MULTI(SubModuleIndex, FB_INFO4,        fb_info4);
    //PUT_REG_INFO_MULTI(SubModuleIndex, DBFISH,          dbfish);
    //PUT_REG_INFO_MULTI(SubModuleIndex, DBG_9,           dbg_9);
    //PUT_REG_INFO_MULTI(SubModuleIndex, DBG_10,          dbg_10);
    //PUT_REG_INFO_MULTI(SubModuleIndex, DBG_11,          dbg_11);
    //PUT_REG_INFO_MULTI(SubModuleIndex, DBG_12,          dbg_12);
    //PUT_REG_INFO_MULTI(SubModuleIndex, DBG_7,           dbg_7);
    //PUT_REG_INFO_MULTI(SubModuleIndex, DMY_1,           dmy_1);
    //PUT_REG_INFO_MULTI(SubModuleIndex, DMY_2,           dmy_2);
    //PUT_REG_INFO_MULTI(SubModuleIndex, SAVE_INFO1,      save_info1);
    //PUT_REG_INFO_MULTI(SubModuleIndex, SAVE_INFO2,      save_info2);
    PUT_REG_INFO_MULTI(SubModuleIndex, SNR_CURVE_1,     snr_curve_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, SNR_CURVE_2,     snr_curve_2);
    PUT_REG_INFO_MULTI(SubModuleIndex, SNR_CURVE_3,     snr_curve_3);
    PUT_REG_INFO_MULTI(SubModuleIndex, SNR_CURVE_4,     snr_curve_4);
    PUT_REG_INFO_MULTI(SubModuleIndex, SNR_CURVE_5,     snr_curve_5);
    PUT_REG_INFO_MULTI(SubModuleIndex, SNR_CURVE_6,     snr_curve_6);
    PUT_REG_INFO_MULTI(SubModuleIndex, SNR_CURVE_7,     snr_curve_7);
    PUT_REG_INFO_MULTI(SubModuleIndex, SNR_CONTROL_1,   snr_control_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, SNR_THR_2,       snr_thr_2);
    PUT_REG_INFO_MULTI(SubModuleIndex, SNR_THR_3,       snr_thr_3);
    PUT_REG_INFO_MULTI(SubModuleIndex, SNR_THR_4,       snr_thr_4);
    //PUT_REG_INFO_MULTI(SubModuleIndex, IN1_CNT,         in1_cnt);
    //PUT_REG_INFO_MULTI(SubModuleIndex, IN2_CNT,         in2_cnt);
    //PUT_REG_INFO_MULTI(SubModuleIndex, IN3_CNT,         in3_cnt);
    //PUT_REG_INFO_MULTI(SubModuleIndex, OUT_CNT,         out_cnt);
    //PUT_REG_INFO_MULTI(SubModuleIndex, STATUS,          status);
    //PUT_REG_INFO_MULTI(SubModuleIndex, TILE_LOSS,       tile_loss);
    //PUT_REG_INFO_MULTI(SubModuleIndex, MCVP_STAT1,      mcvp_stat1);
    //PUT_REG_INFO_MULTI(SubModuleIndex, MCVP_STAT2,      mcvp_stat2);
    //PUT_REG_INFO_MULTI(SubModuleIndex, MCVP_STAT3,      mcvp_stat3);
    //PUT_REG_INFO_MULTI(SubModuleIndex, MCVP_STAT4,      mcvp_stat4);
    //PUT_REG_INFO_MULTI(SubModuleIndex, MCVP_STAT5,      mcvp_stat5);
    //PUT_REG_INFO_MULTI(SubModuleIndex, MCVP_STAT6,      mcvp_stat6);
    //PUT_REG_INFO_MULTI(SubModuleIndex, MCVP_STAT7,      mcvp_stat7);
    //PUT_REG_INFO_MULTI(SubModuleIndex, MCVP_STAT8,      mcvp_stat8);
    //PUT_REG_INFO_MULTI(SubModuleIndex, MCVP_STAT9,      mcvp_stat9);

    return  (*this);
}


template <>
ISP_MGR_NR3D_T&
ISP_MGR_NR3D_T::
get(MUINT8 SubModuleIndex, ISP_NVRAM_NR3D_T & rParam)
{
    //GET_REG_INFO_MULTI(SubModuleIndex, ENG_CON,         eng_con);
    //GET_REG_INFO_MULTI(SubModuleIndex, SIZ,             siz);
    //GET_REG_INFO_MULTI(SubModuleIndex, TILE_XY,         tile_xy);
    GET_REG_INFO_MULTI(SubModuleIndex, ON_CON,          on_con);
    //GET_REG_INFO_MULTI(SubModuleIndex, ON_OFF,          on_off);
    //GET_REG_INFO_MULTI(SubModuleIndex, ON_SIZ,          on_siz);
    GET_REG_INFO_MULTI(SubModuleIndex, TNR_ENABLE,      tnr_enable);
    GET_REG_INFO_MULTI(SubModuleIndex, FLT_CONFIG,      flt_config);
    GET_REG_INFO_MULTI(SubModuleIndex, FB_INFO1,        fb_info1);
    GET_REG_INFO_MULTI(SubModuleIndex, THR_1,           thr_1);
    GET_REG_INFO_MULTI(SubModuleIndex, CURVE_1,         curve_1);
    GET_REG_INFO_MULTI(SubModuleIndex, CURVE_2,         curve_2);
    GET_REG_INFO_MULTI(SubModuleIndex, CURVE_3,         curve_3);
    GET_REG_INFO_MULTI(SubModuleIndex, CURVE_4,         curve_4);
    GET_REG_INFO_MULTI(SubModuleIndex, CURVE_5,         curve_5);
    GET_REG_INFO_MULTI(SubModuleIndex, CURVE_6,         curve_6);
    GET_REG_INFO_MULTI(SubModuleIndex, CURVE_7,         curve_7);
    GET_REG_INFO_MULTI(SubModuleIndex, CURVE_8,         curve_8);
    GET_REG_INFO_MULTI(SubModuleIndex, CURVE_9,         curve_9);
    GET_REG_INFO_MULTI(SubModuleIndex, CURVE_10,        curve_10);
    GET_REG_INFO_MULTI(SubModuleIndex, CURVE_11,        curve_11);
    GET_REG_INFO_MULTI(SubModuleIndex, CURVE_12,        curve_12);
    GET_REG_INFO_MULTI(SubModuleIndex, CURVE_13,        curve_13);
    GET_REG_INFO_MULTI(SubModuleIndex, CURVE_14,        curve_14);
    GET_REG_INFO_MULTI(SubModuleIndex, CURVE_15,        curve_15);
    GET_REG_INFO_MULTI(SubModuleIndex, R2C_1,           r2c_1);
    GET_REG_INFO_MULTI(SubModuleIndex, R2C_2,           r2c_2);
    //GET_REG_INFO_MULTI(SubModuleIndex, R2C_3,           r2c_3);
    //GET_REG_INFO_MULTI(SubModuleIndex, DBG_6,           dbg_6);
    //GET_REG_INFO_MULTI(SubModuleIndex, DBG_15,          dbg_15);
    //GET_REG_INFO_MULTI(SubModuleIndex, DBG_16,          dbg_16);
    //GET_REG_INFO_MULTI(SubModuleIndex, DEMO_1,          demo_1);
    //GET_REG_INFO_MULTI(SubModuleIndex, DEMO_2,          demo_2);
    //GET_REG_INFO_MULTI(SubModuleIndex, ATPG,            atpg);
    //GET_REG_INFO_MULTI(SubModuleIndex, DMY_0,           dmy_0);
    //GET_REG_INFO_MULTI(SubModuleIndex, DBG_17,          dbg_17);
    //GET_REG_INFO_MULTI(SubModuleIndex, INTERR,          interr);
    //GET_REG_INFO_MULTI(SubModuleIndex, FB_INFO2,        fb_info2);
    //GET_REG_INFO_MULTI(SubModuleIndex, FB_INFO3,        fb_info3);
    //GET_REG_INFO_MULTI(SubModuleIndex, FB_INFO4,        fb_info4);
    //GET_REG_INFO_MULTI(SubModuleIndex, DBFISH,          dbfish);
    //GET_REG_INFO_MULTI(SubModuleIndex, DBG_9,           dbg_9);
    //GET_REG_INFO_MULTI(SubModuleIndex, DBG_10,          dbg_10);
    //GET_REG_INFO_MULTI(SubModuleIndex, DBG_11,          dbg_11);
    //GET_REG_INFO_MULTI(SubModuleIndex, DBG_12,          dbg_12);
    //GET_REG_INFO_MULTI(SubModuleIndex, DBG_7,           dbg_7);
    //GET_REG_INFO_MULTI(SubModuleIndex, DMY_1,           dmy_1);
    //GET_REG_INFO_MULTI(SubModuleIndex, DMY_2,           dmy_2);
    //GET_REG_INFO_MULTI(SubModuleIndex, SAVE_INFO1,      save_info1);
    //GET_REG_INFO_MULTI(SubModuleIndex, SAVE_INFO2,      save_info2);
    GET_REG_INFO_MULTI(SubModuleIndex, SNR_CURVE_1,     snr_curve_1);
    GET_REG_INFO_MULTI(SubModuleIndex, SNR_CURVE_2,     snr_curve_2);
    GET_REG_INFO_MULTI(SubModuleIndex, SNR_CURVE_3,     snr_curve_3);
    GET_REG_INFO_MULTI(SubModuleIndex, SNR_CURVE_4,     snr_curve_4);
    GET_REG_INFO_MULTI(SubModuleIndex, SNR_CURVE_5,     snr_curve_5);
    GET_REG_INFO_MULTI(SubModuleIndex, SNR_CURVE_6,     snr_curve_6);
    GET_REG_INFO_MULTI(SubModuleIndex, SNR_CURVE_7,     snr_curve_7);
    GET_REG_INFO_MULTI(SubModuleIndex, SNR_CONTROL_1,   snr_control_1);
    GET_REG_INFO_MULTI(SubModuleIndex, SNR_THR_2,       snr_thr_2);
    GET_REG_INFO_MULTI(SubModuleIndex, SNR_THR_3,       snr_thr_3);
    GET_REG_INFO_MULTI(SubModuleIndex, SNR_THR_4,       snr_thr_4);
    //GET_REG_INFO_MULTI(SubModuleIndex, IN1_CNT,         in1_cnt);
    //GET_REG_INFO_MULTI(SubModuleIndex, IN2_CNT,         in2_cnt);
    //GET_REG_INFO_MULTI(SubModuleIndex, IN3_CNT,         in3_cnt);
    //GET_REG_INFO_MULTI(SubModuleIndex, OUT_CNT,         out_cnt);
    //GET_REG_INFO_MULTI(SubModuleIndex, STATUS,          status);
    //GET_REG_INFO_MULTI(SubModuleIndex, TILE_LOSS,       tile_loss);
    //GET_REG_INFO_MULTI(SubModuleIndex, MCVP_STAT1,      mcvp_stat1);
    //GET_REG_INFO_MULTI(SubModuleIndex, MCVP_STAT2,      mcvp_stat2);
    //GET_REG_INFO_MULTI(SubModuleIndex, MCVP_STAT3,      mcvp_stat3);
    //GET_REG_INFO_MULTI(SubModuleIndex, MCVP_STAT4,      mcvp_stat4);
    //GET_REG_INFO_MULTI(SubModuleIndex, MCVP_STAT5,      mcvp_stat5);
    //GET_REG_INFO_MULTI(SubModuleIndex, MCVP_STAT6,      mcvp_stat6);
    //GET_REG_INFO_MULTI(SubModuleIndex, MCVP_STAT7,      mcvp_stat7);
    //GET_REG_INFO_MULTI(SubModuleIndex, MCVP_STAT8,      mcvp_stat8);
    //GET_REG_INFO_MULTI(SubModuleIndex, MCVP_STAT9,      mcvp_stat9);

    return  (*this);
}
MVOID
ISP_MGR_NR3D_T::
set_ADNR_param(MUINT8 SubModuleIndex, FEATURE_NVRAM_AD_NR_T const& rParam){
	::memcpy(&m_ADNR[SubModuleIndex], &rParam, sizeof(FEATURE_NVRAM_AD_NR_T));
}

MVOID
ISP_MGR_NR3D_T::
adaptive_NR3D_setting(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo,  dip_a_reg_t* pReg)
{

#define CLIP_NR3D(a,b,c)        ( (a<b)?  b : ((a>c)? c: a) )
    const NSCam::NR3D::NR3DIspAdaptiveInfo &adpI = rRawIspCamInfo.NR3D_Data.adaptiveInfo;
    MINT32 GMVX = adpI.GMVX/256;
    MINT32 GMVY = adpI.GMVY/256;
    MINT32 confX = adpI.confX;
    MINT32 confY = adpI.confY;
    MINT32 MAX_GMVX = adpI.MAX_GMVX;
    MINT32 MAX_GMVY = adpI.MAX_GMVY;
    MINT32 frameReset = adpI.frameReset;
    MBOOL  isGMVValid = adpI.isGMVValid;
    MINT32 GMV_level_th_def = 200;

    MINT32 Device_ID = m_eSensorDev;
    MINT32 GMVLevelTh = m_GMV_level_th;
    MBOOL  ISO_switch = MFALSE;//HW turned off
    MINT32 u4RealISO = rRawIspCamInfo.rAEInfo.u4P2RealISOValue;

    MINT32 GMV_sm_th = 112;//0.875*MAX_GMV 112/128=0.875, ranged:0~128
    MINT32 GMV_sm_th_up = 8;//minus point to GMV_level_th, ranged:0~255
    MINT32 GMV_sm_th_lo = 19;//add point to GMV_level_th, ranged:0~255
    MINT32 GMV_lv_th_up = 180;//ranged:0~255
    MINT32 GMV_lv_th_lo = 50;//ranged:0~255
    MINT32 GMV_min_str = 102;//0.8*MAX_FLTSTR, 102/128=0.8, ranged:0~128
    MINT32 ISO_cutoff = adpI.ISO_cutoff; // turned off when ISO < 400
    MINT32 ISO_upper = 800;
    MINT32 Conf_th = 10;
    MINT32 disableAdaptiveSetting = 0;

    // Start 2018/10/16 Adaptive for SNR
    MINT32 GMV_min_cent_cand_wei = 16;
    MINT32 GMV_max_sad_invwei = 32;
    MINT32 GMV_max_blkvar_tolerance = 35;
    MINT32 GMV_max_centvar_tolerance = 40;
    // End 2018/10/16 Adaptive for SNR

    MBOOL bAdaptiveNR3DDebug = rRawIspCamInfo.NR3D_Data.debugEnable;
    if (frameReset)
    {
        if (GMVLevelTh != GMV_level_th_def)
        {
            MY_LOG_IF(bAdaptiveNR3DDebug>=1, "Device_ID(%d) Reset first GMVLevelTh to %d", Device_ID, GMV_level_th_def);
            GMVLevelTh = GMV_level_th_def;
        }
    }

    MINT32 bPrintAdaptiveNR3D = 0;

    if (bAdaptiveNR3DDebug)
    {
        static MINT32 s_GMV_lv_th_def = 200;//frame 1 default value for "GMV_level_th", ranged:0~255
        bPrintAdaptiveNR3D = ::property_get_int32("vendor.debug.nr3d.adap.print", bPrintAdaptiveNR3D);

        MINT32 GMV_lv_th_def = ::property_get_int32("vendor.debug.nr3d.GMV_lv_th_def", 200);

        if ((s_GMV_lv_th_def != GMV_lv_th_def) && (GMV_lv_th_def < 255))
        {
            s_GMV_lv_th_def = GMV_lv_th_def;
            GMVLevelTh = s_GMV_lv_th_def;
            MY_LOG_IF(1, "Reset s_GMV_lv_th_def and GMVLevelTh to %d", GMVLevelTh);
        }

        GMV_sm_th = ::property_get_int32("vendor.debug.nr3d.GMV_sm_th", GMV_sm_th);
        GMV_sm_th_up = ::property_get_int32("vendor.debug.nr3d.GMV_sm_th_up", GMV_sm_th_up);
        GMV_sm_th_lo = ::property_get_int32("vendor.debug.nr3d.GMV_sm_th_lo", GMV_sm_th_lo);
        GMV_lv_th_up = ::property_get_int32("vendor.debug.nr3d.GMV_lv_th_up", GMV_lv_th_up);
        GMV_lv_th_lo = ::property_get_int32("vendor.debug.nr3d.GMV_lv_th_lo", GMV_lv_th_lo);
        GMV_min_str = ::property_get_int32("vendor.debug.nr3d.GMV_min_str", GMV_min_str);
        ISO_cutoff = ::property_get_int32("vendor.debug.nr3d.ISO_cutoff", ISO_cutoff);
        ISO_upper = ::property_get_int32("vendor.debug.nr3d.ISO_upper", ISO_upper);
        disableAdaptiveSetting = ::property_get_int32("vendor.debug.nr3d.disable_adapt", 0);

        MY_LOG_IF(bPrintAdaptiveNR3D>=2, "Set debug.nr3d.bm (%d, %d, %d, %d, %d, %d, %d, %d)",
              GMV_sm_th, GMV_sm_th_up, GMV_sm_th_lo, GMV_lv_th_up, GMV_lv_th_lo, GMV_min_str, ISO_cutoff, ISO_upper);

        // Start 2018/10/16 Adaptive for SNR
        GMV_min_cent_cand_wei = ::property_get_int32("vendor.debug.nr3d.GMV_min_cent_cand_wei", GMV_min_cent_cand_wei);
        GMV_max_sad_invwei = ::property_get_int32("vendor.debug.nr3d.GMV_max_sad_invwei", GMV_max_sad_invwei);
        GMV_max_blkvar_tolerance = ::property_get_int32("vendor.debug.nr3d.GMV_max_blkvar_tolerance", GMV_max_blkvar_tolerance);
        GMV_max_centvar_tolerance  = ::property_get_int32("vendor.debug.nr3d.GMV_max_centvar_tolerance", GMV_max_centvar_tolerance);
        MY_LOG_IF(bPrintAdaptiveNR3D>=2, "Set debug.nr3d.bm SNR_part (%d, %d, %d, %d)",
                    GMV_min_cent_cand_wei, GMV_max_sad_invwei, GMV_max_blkvar_tolerance, GMV_max_centvar_tolerance);
        // End 2018/10/16 Adaptive for SNR
    }

    /////////////start of smooth GMV part
    //MAX GMV should be something like 32, 64, 128

    MUINT32 GMV_level_str = 128; //128 = 100%
    MUINT32 ISO_level_str = 128; //128 = 100%

    MINT32 i4GmvTooLrg=0;
    // MINT32 i4GmvLowConf=0;

    if ( ( (abs(GMVX)) >= ( MAX_GMVX * GMV_sm_th / 128) ) || ( (abs(GMVY)) >= ( MAX_GMVY * GMV_sm_th /128 ) ) )
        i4GmvTooLrg = 1;

    // CAN REMOVE IT
    // if ( ( (abs(confX)) <= 10 ) || ( (abs(confY)) <= 10 ) )
    //    i4GmvLowConf = 1;


    if (i4GmvTooLrg==1)
        GMVLevelTh = GMVLevelTh - GMV_sm_th_up;
    else
        GMVLevelTh = GMVLevelTh + GMV_sm_th_lo;


    GMVLevelTh = CLIP_NR3D( GMVLevelTh, 0, 255);

    if( GMVLevelTh > GMV_lv_th_up )
    {
        GMV_level_str = 128;
    }
    else
    {
        if (0 != (GMV_lv_th_up - GMV_lv_th_lo))
        {
            GMV_level_str = GMV_min_str + ( ( (128 - GMV_min_str) *  CLIP_NR3D( GMVLevelTh - GMV_lv_th_lo, 0, GMV_lv_th_up - GMV_lv_th_lo ) +( GMV_lv_th_up - GMV_lv_th_lo )/2 ) / ( GMV_lv_th_up - GMV_lv_th_lo ) );
        }
        else
        {
            MY_LOG_IF(bPrintAdaptiveNR3D>=1, "0 = (GMV_lv_th_up - GMV_lv_th_lo)");
        }
    }

    /////////////end of smooth GMV part

    // adaptive YNR start

	bool bPrintAdaptiveyYNR = ::property_get_int32("vendor.debug.nr3d.adapynr.print", 0);
	bool bNR3D_en = reinterpret_cast<NR3D_REG_D1A_NR3D_NR3D_TNR_ENABLE*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, TNR_ENABLE))->Bits.NR3D_tnr_y_en;
    #define NR3D_ABS(x) ( (x > 0) ? x : -(x))
	int max_gmv = MAX(NR3D_ABS(GMVX),NR3D_ABS(GMVY));
	int iDS_Size = 4;
	iDS_Size = ::property_get_int32("vendor.debug.nr3d.fDS_Size", iDS_Size);
	if (!bNR3D_en) {
		max_gmv = ( max_gmv + (iDS_Size / 2)) / iDS_Size;
	}
    int GMV_CONF = 0;

	if ((max_gmv)>=12)
	{	GMV_CONF = 0;
		GMV_CONF = ::property_get_int32("vendor.debug.nr3d.GMV_CONF_GMV12", 0);
    }
	else if ((max_gmv)==11)
	{	GMV_CONF = 0;
		GMV_CONF = ::property_get_int32("vendor.debug.nr3d.GMV_CONF_GMV11", 0);
    }
	else if ((max_gmv)==10)
	{	GMV_CONF = 0;
		GMV_CONF = ::property_get_int32("vendor.debug.nr3d.GMV_CONF_GMV10", 0);
    }
	else if ((max_gmv)==9)
	{	GMV_CONF = 0;
		GMV_CONF = ::property_get_int32("vendor.debug.nr3d.GMV_CONF_GMV9", 0);
    }
	else if ((max_gmv)==8)
	{	GMV_CONF = 0;
		GMV_CONF = ::property_get_int32("vendor.debug.nr3d.GMV_CONF_GMV8", 0);
    }
	else if ((max_gmv)==7)
	{	GMV_CONF = 0;
		GMV_CONF = ::property_get_int32("vendor.debug.nr3d.GMV_CONF_GMV7", 0);
    }
	else if ((max_gmv)==6)
	{	GMV_CONF = 32;
		GMV_CONF = ::property_get_int32("vendor.debug.nr3d.GMV_CONF_GMV6", 32);
    }
	else if ((max_gmv)==5)
	{	GMV_CONF = 64;
		GMV_CONF = ::property_get_int32("vendor.debug.nr3d.GMV_CONF_GMV5", 64);
    }
	else if ((max_gmv)==4)
	{	GMV_CONF = 128;
		GMV_CONF = ::property_get_int32("vendor.debug.nr3d.GMV_CONF_GMV4", 128);
    }
	else if ((max_gmv)==3)
	{	GMV_CONF = 256;
		GMV_CONF = ::property_get_int32("vendor.debug.nr3d.GMV_CONF_GMV3", 256);
    }
	else if ((max_gmv)==2)
	{	GMV_CONF = 256;
		GMV_CONF = ::property_get_int32("vendor.debug.nr3d.GMV_CONF_GMV2", 256);
    }
	else if ((max_gmv)<=1)
	{	GMV_CONF = 256;
		GMV_CONF = ::property_get_int32("vendor.debug.nr3d.GMV_CONF_GMV1", 256);
	}

    int NR0_BLD_OFST_MAX = 16; // tuning from adb command
    int NR0_STD_RATIO_MAX = 32; // tuning from adb command
    int NR1_BLD_OFST_MAX = 16; // tuning from adb command
    int NR1_STD_RATIO_MAX = 32; // tuning from adb command
    NR0_BLD_OFST_MAX  = ::property_get_int32("vendor.debug.nr3d.NR0_BLD_OFST_MAX", NR0_BLD_OFST_MAX);
    NR0_STD_RATIO_MAX = ::property_get_int32("vendor.debug.nr3d.NR0_STD_RATIO_MAX", NR0_STD_RATIO_MAX);
    NR1_BLD_OFST_MAX = ::property_get_int32("vendor.debug.nr3d.NR1_BLD_OFST_MAX", NR1_BLD_OFST_MAX);
    NR1_STD_RATIO_MAX = ::property_get_int32("vendor.debug.nr3d.NR1_STD_RATIO_MAX", NR1_STD_RATIO_MAX);

	MY_LOG_IF(bPrintAdaptiveyYNR>=1, "enable_nr3d = (%d), GMV(x,y) = (%d, %d), GMV_CONF = %d", (int)bNR3D_en, GMVX, GMVY, GMV_CONF);
	MY_LOG_IF(bPrintAdaptiveyYNR>=1, "(NR0_BLD_OFST_MAX, NR0_STD_RATIO_MAX) = (%d, %d), (NR1_BLD_OFST_MAX, NR1_STD_RATIO_MAX) = (%d, %d)", NR0_BLD_OFST_MAX, NR0_STD_RATIO_MAX, NR1_BLD_OFST_MAX, NR1_STD_RATIO_MAX);

    int NR_BLD_OFST_MAX = (bNR3D_en) ? NR1_BLD_OFST_MAX : NR0_BLD_OFST_MAX; // tuning from adb command
    // int NR_STD_RATIO_MAX = (bNR3D_en) ? NR1_BLD_STD_MAX : NR0_BLD_STD_MAX; // tuning from adb command
    int NR_STD_RATIO_MAX = (bNR3D_en) ? NR1_STD_RATIO_MAX : NR0_STD_RATIO_MAX; // tuning from adb command

    int NR_BLD_OFST = (NR_BLD_OFST_MAX * (256 - GMV_CONF) + 128) / 256;
    int NR_STD_RATIO = (NR_STD_RATIO_MAX * (256 - GMV_CONF) + 16 * GMV_CONF + 128) / 256;

	MINT32 YNR_Y_HF_ACT_Y0    = reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_HF_ACT1.Bits.YNR_Y_HF_ACT_Y0;
	MINT32 YNR_Y_HF_ACT_Y1    = reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_HF_ACT1.Bits.YNR_Y_HF_ACT_Y1;
	MINT32 YNR_Y_HF_ACT_Y2    = reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_HF_ACT1.Bits.YNR_Y_HF_ACT_Y2;
	MINT32 YNR_Y_HF_ACT_Y3    = reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_HF_ACT1.Bits.YNR_Y_HF_ACT_Y3;
	MINT32 YNR_Y_HF_ACT_Y4    = reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_HF_ACT2.Bits.YNR_Y_HF_ACT_Y4;
	MINT32 YNR_Y_L0_V_RNG1    = reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY0V.Bits.YNR_Y_L0_V_RNG1;
	MINT32 YNR_Y_L0_V_RNG2    = reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY0V.Bits.YNR_Y_L0_V_RNG2;
	MINT32 YNR_Y_L0_V_RNG3    = reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY0V.Bits.YNR_Y_L0_V_RNG3;
	MINT32 YNR_Y_L0_V_RNG4    = reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY0V.Bits.YNR_Y_L0_V_RNG4;
	MINT32 YNR_Y_L0_H_RNG1    = reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY0H.Bits.YNR_Y_L0_H_RNG1;
	MINT32 YNR_Y_L0_H_RNG2    = reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY0H.Bits.YNR_Y_L0_H_RNG2;
	MINT32 YNR_Y_L0_H_RNG3    = reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY0H.Bits.YNR_Y_L0_H_RNG3;
	MINT32 YNR_Y_L0_H_RNG4    = reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY0H.Bits.YNR_Y_L0_H_RNG4;
	MINT32 YNR_Y_L1_V_RNG1    = reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY1V.Bits.YNR_Y_L1_V_RNG1;
	MINT32 YNR_Y_L1_V_RNG2    = reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY1V.Bits.YNR_Y_L1_V_RNG2;
	MINT32 YNR_Y_L1_V_RNG3    = reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY1V.Bits.YNR_Y_L1_V_RNG3;
	MINT32 YNR_Y_L1_V_RNG4    = reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY1V.Bits.YNR_Y_L1_V_RNG4;
	MINT32 YNR_Y_L1_H_RNG1    = reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY1H.Bits.YNR_Y_L1_H_RNG1;
	MINT32 YNR_Y_L1_H_RNG2    = reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY1H.Bits.YNR_Y_L1_H_RNG2;
	MINT32 YNR_Y_L1_H_RNG3    = reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY1H.Bits.YNR_Y_L1_H_RNG3;
	MINT32 YNR_Y_L1_H_RNG4    = reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY1H.Bits.YNR_Y_L1_H_RNG4;
	MINT32 YNR_Y_L2_V_RNG1    = reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY2V.Bits.YNR_Y_L2_V_RNG1;
	MINT32 YNR_Y_L2_V_RNG2    = reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY2V.Bits.YNR_Y_L2_V_RNG2;
	MINT32 YNR_Y_L2_V_RNG3    = reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY2V.Bits.YNR_Y_L2_V_RNG3;
	MINT32 YNR_Y_L2_V_RNG4    = reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY2V.Bits.YNR_Y_L2_V_RNG4;
	MINT32 YNR_Y_L2_H_RNG1    = reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY2H.Bits.YNR_Y_L2_H_RNG1;
	MINT32 YNR_Y_L2_H_RNG2    = reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY2H.Bits.YNR_Y_L2_H_RNG2;
	MINT32 YNR_Y_L2_H_RNG3    = reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY2H.Bits.YNR_Y_L2_H_RNG3;
	MINT32 YNR_Y_L2_H_RNG4    = reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY2H.Bits.YNR_Y_L2_H_RNG4;
	MINT32 YNR_Y_L3_V_RNG1    = reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY3V.Bits.YNR_Y_L3_V_RNG1;
	MINT32 YNR_Y_L3_V_RNG2    = reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY3V.Bits.YNR_Y_L3_V_RNG2;
	MINT32 YNR_Y_L3_V_RNG3    = reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY3V.Bits.YNR_Y_L3_V_RNG3;
	MINT32 YNR_Y_L3_V_RNG4    = reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY3V.Bits.YNR_Y_L3_V_RNG4;
	MINT32 YNR_Y_L3_H_RNG1    = reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY3H.Bits.YNR_Y_L3_H_RNG1;
	MINT32 YNR_Y_L3_H_RNG2    = reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY3H.Bits.YNR_Y_L3_H_RNG2;
	MINT32 YNR_Y_L3_H_RNG3    = reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY3H.Bits.YNR_Y_L3_H_RNG3;
	MINT32 YNR_Y_L3_H_RNG4    = reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY3H.Bits.YNR_Y_L3_H_RNG4;


    // ISP_YUV reg:
    YNR_Y_HF_ACT_Y0                 = MAX(YNR_Y_HF_ACT_Y0 - NR_BLD_OFST, 0);
    YNR_Y_HF_ACT_Y1                 = MAX(YNR_Y_HF_ACT_Y1 - NR_BLD_OFST, 0);
    YNR_Y_HF_ACT_Y2                 = MAX(YNR_Y_HF_ACT_Y2 - NR_BLD_OFST, 0);
    YNR_Y_HF_ACT_Y3                 = MAX(YNR_Y_HF_ACT_Y3 - NR_BLD_OFST, 0);
    YNR_Y_HF_ACT_Y4                 = MAX(YNR_Y_HF_ACT_Y4 - NR_BLD_OFST, 0);

#define NR3D_MIN(a,b) (((a)<(b))?(a):(b))
    YNR_Y_L0_V_RNG4                 = NR3D_MIN((YNR_Y_L0_V_RNG4 * NR_STD_RATIO + 8) / 16, 255);
    YNR_Y_L0_V_RNG3                 = NR3D_MIN((YNR_Y_L0_V_RNG3 * NR_STD_RATIO + 8) / 16, 255);
    YNR_Y_L0_V_RNG2                 = NR3D_MIN((YNR_Y_L0_V_RNG2 * NR_STD_RATIO + 8) / 16, 255);
    YNR_Y_L0_V_RNG1                 = NR3D_MIN((YNR_Y_L0_V_RNG1 * NR_STD_RATIO + 8) / 16, 255);
    YNR_Y_L0_H_RNG4                 = NR3D_MIN((YNR_Y_L0_H_RNG4 * NR_STD_RATIO + 8) / 16, 255);
    YNR_Y_L0_H_RNG3                 = NR3D_MIN((YNR_Y_L0_H_RNG3 * NR_STD_RATIO + 8) / 16, 255);
    YNR_Y_L0_H_RNG2                 = NR3D_MIN((YNR_Y_L0_H_RNG2 * NR_STD_RATIO + 8) / 16, 255);
    YNR_Y_L0_H_RNG1                 = NR3D_MIN((YNR_Y_L0_H_RNG1 * NR_STD_RATIO + 8) / 16, 255);
    YNR_Y_L1_V_RNG4                 = NR3D_MIN((YNR_Y_L1_V_RNG4 * NR_STD_RATIO + 8) / 16, 255);
    YNR_Y_L1_V_RNG3                 = NR3D_MIN((YNR_Y_L1_V_RNG3 * NR_STD_RATIO + 8) / 16, 255);
    YNR_Y_L1_V_RNG2                 = NR3D_MIN((YNR_Y_L1_V_RNG2 * NR_STD_RATIO + 8) / 16, 255);
    YNR_Y_L1_V_RNG1                 = NR3D_MIN((YNR_Y_L1_V_RNG1 * NR_STD_RATIO + 8) / 16, 255);
    YNR_Y_L1_H_RNG4                 = NR3D_MIN((YNR_Y_L1_H_RNG4 * NR_STD_RATIO + 8) / 16, 255);
    YNR_Y_L1_H_RNG3                 = NR3D_MIN((YNR_Y_L1_H_RNG3 * NR_STD_RATIO + 8) / 16, 255);
    YNR_Y_L1_H_RNG2                 = NR3D_MIN((YNR_Y_L1_H_RNG2 * NR_STD_RATIO + 8) / 16, 255);
    YNR_Y_L1_H_RNG1                 = NR3D_MIN((YNR_Y_L1_H_RNG1 * NR_STD_RATIO + 8) / 16, 255);
    YNR_Y_L2_V_RNG4                 = NR3D_MIN((YNR_Y_L2_V_RNG4 * NR_STD_RATIO + 8) / 16, 255);
    YNR_Y_L2_V_RNG3                 = NR3D_MIN((YNR_Y_L2_V_RNG3 * NR_STD_RATIO + 8) / 16, 255);
    YNR_Y_L2_V_RNG2                 = NR3D_MIN((YNR_Y_L2_V_RNG2 * NR_STD_RATIO + 8) / 16, 255);
    YNR_Y_L2_V_RNG1                 = NR3D_MIN((YNR_Y_L2_V_RNG1 * NR_STD_RATIO + 8) / 16, 255);
    YNR_Y_L2_H_RNG4                 = NR3D_MIN((YNR_Y_L2_H_RNG4 * NR_STD_RATIO + 8) / 16, 255);
    YNR_Y_L2_H_RNG3                 = NR3D_MIN((YNR_Y_L2_H_RNG3 * NR_STD_RATIO + 8) / 16, 255);
    YNR_Y_L2_H_RNG2                 = NR3D_MIN((YNR_Y_L2_H_RNG2 * NR_STD_RATIO + 8) / 16, 255);
    YNR_Y_L2_H_RNG1                 = NR3D_MIN((YNR_Y_L2_H_RNG1 * NR_STD_RATIO + 8) / 16, 255);
    YNR_Y_L3_V_RNG4                 = NR3D_MIN((YNR_Y_L3_V_RNG4 * NR_STD_RATIO + 8) / 16, 255);
    YNR_Y_L3_V_RNG3                 = NR3D_MIN((YNR_Y_L3_V_RNG3 * NR_STD_RATIO + 8) / 16, 255);
    YNR_Y_L3_V_RNG2                 = NR3D_MIN((YNR_Y_L3_V_RNG2 * NR_STD_RATIO + 8) / 16, 255);
    YNR_Y_L3_V_RNG1                 = NR3D_MIN((YNR_Y_L3_V_RNG1 * NR_STD_RATIO + 8) / 16, 255);
    YNR_Y_L3_H_RNG4                 = NR3D_MIN((YNR_Y_L3_H_RNG4 * NR_STD_RATIO + 8) / 16, 255);
    YNR_Y_L3_H_RNG3                 = NR3D_MIN((YNR_Y_L3_H_RNG3 * NR_STD_RATIO + 8) / 16, 255);
    YNR_Y_L3_H_RNG2                 = NR3D_MIN((YNR_Y_L3_H_RNG2 * NR_STD_RATIO + 8) / 16, 255);
    YNR_Y_L3_H_RNG1                 = NR3D_MIN((YNR_Y_L3_H_RNG1 * NR_STD_RATIO + 8) / 16, 255);

    reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_HF_ACT1.Bits.YNR_Y_HF_ACT_Y0 = YNR_Y_HF_ACT_Y0;
	reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_HF_ACT1.Bits.YNR_Y_HF_ACT_Y1 = YNR_Y_HF_ACT_Y1;
	reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_HF_ACT1.Bits.YNR_Y_HF_ACT_Y2 = YNR_Y_HF_ACT_Y2;
	reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_HF_ACT1.Bits.YNR_Y_HF_ACT_Y3 = YNR_Y_HF_ACT_Y3;
	reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_HF_ACT2.Bits.YNR_Y_HF_ACT_Y4 = YNR_Y_HF_ACT_Y4;
	reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY0V.Bits.YNR_Y_L0_V_RNG1   = YNR_Y_L0_V_RNG1;
	reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY0V.Bits.YNR_Y_L0_V_RNG2   = YNR_Y_L0_V_RNG2;
	reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY0V.Bits.YNR_Y_L0_V_RNG3   = YNR_Y_L0_V_RNG3;
	reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY0V.Bits.YNR_Y_L0_V_RNG4   = YNR_Y_L0_V_RNG4;
	reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY0H.Bits.YNR_Y_L0_H_RNG1   = YNR_Y_L0_H_RNG1;
	reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY0H.Bits.YNR_Y_L0_H_RNG2   = YNR_Y_L0_H_RNG2;
	reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY0H.Bits.YNR_Y_L0_H_RNG3   = YNR_Y_L0_H_RNG3;
	reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY0H.Bits.YNR_Y_L0_H_RNG4   = YNR_Y_L0_H_RNG4;
	reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY1V.Bits.YNR_Y_L1_V_RNG1   = YNR_Y_L1_V_RNG1;
	reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY1V.Bits.YNR_Y_L1_V_RNG2   = YNR_Y_L1_V_RNG2;
	reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY1V.Bits.YNR_Y_L1_V_RNG3   = YNR_Y_L1_V_RNG3;
	reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY1V.Bits.YNR_Y_L1_V_RNG4   = YNR_Y_L1_V_RNG4;
	reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY1H.Bits.YNR_Y_L1_H_RNG1   = YNR_Y_L1_H_RNG1;
	reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY1H.Bits.YNR_Y_L1_H_RNG2   = YNR_Y_L1_H_RNG2;
	reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY1H.Bits.YNR_Y_L1_H_RNG3   = YNR_Y_L1_H_RNG3;
	reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY1H.Bits.YNR_Y_L1_H_RNG4   = YNR_Y_L1_H_RNG4;
	reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY2V.Bits.YNR_Y_L2_V_RNG1   = YNR_Y_L2_V_RNG1;
	reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY2V.Bits.YNR_Y_L2_V_RNG2   = YNR_Y_L2_V_RNG2;
	reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY2V.Bits.YNR_Y_L2_V_RNG3   = YNR_Y_L2_V_RNG3;
	reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY2V.Bits.YNR_Y_L2_V_RNG4   = YNR_Y_L2_V_RNG4;
	reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY2H.Bits.YNR_Y_L2_H_RNG1   = YNR_Y_L2_H_RNG1;
	reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY2H.Bits.YNR_Y_L2_H_RNG2   = YNR_Y_L2_H_RNG2;
	reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY2H.Bits.YNR_Y_L2_H_RNG3   = YNR_Y_L2_H_RNG3;
	reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY2H.Bits.YNR_Y_L2_H_RNG4   = YNR_Y_L2_H_RNG4;
	reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY3V.Bits.YNR_Y_L3_V_RNG1   = YNR_Y_L3_V_RNG1;
	reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY3V.Bits.YNR_Y_L3_V_RNG2   = YNR_Y_L3_V_RNG2;
	reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY3V.Bits.YNR_Y_L3_V_RNG3   = YNR_Y_L3_V_RNG3;
	reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY3V.Bits.YNR_Y_L3_V_RNG4   = YNR_Y_L3_V_RNG4;
	reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY3H.Bits.YNR_Y_L3_H_RNG1   = YNR_Y_L3_H_RNG1;
	reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY3H.Bits.YNR_Y_L3_H_RNG2   = YNR_Y_L3_H_RNG2;
	reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY3H.Bits.YNR_Y_L3_H_RNG3   = YNR_Y_L3_H_RNG3;
	reinterpret_cast<dip_x_reg_t*>(pReg)->YNR_D1A_YNR_PTY3H.Bits.YNR_Y_L3_H_RNG4   = YNR_Y_L3_H_RNG4;

    // adaptive YNR end


    /////////////start of smooth ISO part

    if( u4RealISO <= ISO_cutoff )
    {
        ISO_switch = 0;//HW turned off
    }
    else if( u4RealISO > ISO_cutoff && u4RealISO < ISO_upper )
    {
        if (0 != (ISO_upper - ISO_cutoff))
        {
            ISO_level_str = 0 +  ( ( (128 - 0) * CLIP_NR3D( u4RealISO - ISO_cutoff, 0, ISO_upper - ISO_cutoff ) + ( ISO_upper - ISO_cutoff )/2 ) / ( ISO_upper - ISO_cutoff ) );
        }
        else
        {
            MY_LOG_IF(bPrintAdaptiveNR3D>=1, "0 = (ISO_upper - ISO_cutoff)");
        }
        ISO_switch = 1;
    }
    else
    {
        ISO_switch = 1;
        ISO_level_str = 128;
    }
    /////////////end of smooth ISO part

    /////////////start of SMOOTH GMV/ISO part
    ///////////////start of 20170316
    MINT32 enableAdaptiveISO = 0;
    if (bAdaptiveNR3DDebug)
    {
        enableAdaptiveISO = ::property_get_int32("vendor.debug.nr3d.enable_ISOadpt", 0);
    }

    MINT32 disableGMVConfRefine = 0;

    if (bAdaptiveNR3DDebug)
    {
        disableGMVConfRefine = ::property_get_int32("vendor.debug.nr3d.disable_GMVRef", 0);
     }

    // === 2016/03/16: gmv adjustment by conf: start ==
    MINT32 i4GmvConfFallback=0;
    MINT32 adjustGMVX = GMVX;
    MINT32 adjustGMVY = GMVY;

    if (disableGMVConfRefine)
    {
        i4GmvConfFallback=0;
    }
    else
    {
        if(NR3DCustomBase::adjust_3dnr_gmv_by_conf(bAdaptiveNR3DDebug, confX, confY, adjustGMVX, adjustGMVY))
        {
            // no need adjust GMVX/Y to adjustGMVX/Y here because algo won't use adjusted GMVX/Y later
            i4GmvConfFallback=1;
            MY_LOG_IF(bPrintAdaptiveNR3D>=1, "GMV adjusted: (confX,confY)=(%d,%d), gmvX(%d->%d), gmvY(%d->%d)",
                confX, confY, GMVX, adjustGMVX, GMVY, adjustGMVY);
        }
    }
    // === 2016/03/16: gmv adjustment by conf: end ==

    MINT32 GMV_ISO_RATIO=(GMV_level_str); //(/128)
    MINT32 GMV_ISO_BSE=128; //(/128)
    MINT32 GMV_ISO_RND=64;   //(/128)
    MINT32 NR3D_FLT_STR_MAX_before = reinterpret_cast<NR3D_REG_D1A_NR3D_NR3D_FLT_CONFIG*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, FLT_CONFIG))->Bits.NR3D_flt_str_max;
    MINT32 NR3D_FLT_STR_MAX_after = ( (NR3D_FLT_STR_MAX_before * GMV_ISO_RATIO) + GMV_ISO_RND ) / GMV_ISO_BSE;
    if(enableAdaptiveISO == 1)
    {
        GMV_ISO_RATIO=(GMV_level_str*ISO_level_str); //(/128/128)
        GMV_ISO_BSE=128*128; //(/128/128)
        GMV_ISO_RND=128*128/2;   //(/128/128)
        NR3D_FLT_STR_MAX_before = reinterpret_cast<NR3D_REG_D1A_NR3D_NR3D_FLT_CONFIG*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, FLT_CONFIG))->Bits.NR3D_flt_str_max;
        NR3D_FLT_STR_MAX_after = ( (NR3D_FLT_STR_MAX_before * GMV_ISO_RATIO) + GMV_ISO_RND ) / GMV_ISO_BSE;
    }

    //if (0 == disableAdaptiveSetting)
    if (0 == disableAdaptiveSetting && i4GmvConfFallback==0)
    {
        reinterpret_cast<NR3D_REG_D1A_NR3D_NR3D_FLT_CONFIG*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, FLT_CONFIG))->Bits.NR3D_flt_str_max = NR3D_FLT_STR_MAX_after;
        // Start 2018/10/16 Adaptive for SNR
        MINT32 NR3D_blkvar_tolerance_before  = reinterpret_cast<NR3D_REG_D1A_NR3D_NR3D_SNR_THR_2*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, SNR_THR_2))->Bits.NR3D_blkvar_tolerance;
        MINT32 NR3D_centvar_tolerance_before = reinterpret_cast<NR3D_REG_D1A_NR3D_NR3D_SNR_THR_2*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, SNR_THR_2))->Bits.NR3D_centvar_tolerance;
        MINT32 NR3D_sad_invwei_before        = reinterpret_cast<NR3D_REG_D1A_NR3D_NR3D_SNR_THR_3*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, SNR_THR_3))->Bits.NR3D_sad_invwei;
        MINT32 NR3D_cent_cand_wei_before     = reinterpret_cast<NR3D_REG_D1A_NR3D_NR3D_SNR_THR_3*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, SNR_THR_3))->Bits.NR3D_cent_cand_wei;

        // f(x) = (x - x0) * (y1 - y0) / (x1 - x0) + y0
        MINT32 iXsubX0 = CLIP_NR3D( GMVLevelTh - GMV_lv_th_lo, 0, GMV_lv_th_up - GMV_lv_th_lo );
        MINT32 iX1subX0 = GMV_lv_th_up - GMV_lv_th_lo;
        if (iX1subX0 != 0)
        {
            MINT32 NR3D_blkvar_tolerance_after  = GMV_max_blkvar_tolerance + (iXsubX0 * (NR3D_blkvar_tolerance_before - GMV_max_blkvar_tolerance) + iX1subX0 / 2 )/ iX1subX0;
            MINT32 NR3D_centvar_tolerance_after = GMV_max_centvar_tolerance + (iXsubX0 * (NR3D_centvar_tolerance_before - GMV_max_centvar_tolerance) + iX1subX0 / 2 )/ iX1subX0;
            MINT32 NR3D_sad_invwei_after        = GMV_max_sad_invwei + (iXsubX0 * (NR3D_sad_invwei_before - GMV_max_sad_invwei) + iX1subX0 / 2 )/ iX1subX0;
            MINT32 NR3D_cent_cand_wei_after     = GMV_min_cent_cand_wei + (iXsubX0 * (NR3D_cent_cand_wei_before - GMV_min_cent_cand_wei) + iX1subX0 / 2 )/ iX1subX0;

            reinterpret_cast<NR3D_REG_D1A_NR3D_NR3D_SNR_THR_2*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, SNR_THR_2))->Bits.NR3D_blkvar_tolerance  =  NR3D_blkvar_tolerance_after;
            reinterpret_cast<NR3D_REG_D1A_NR3D_NR3D_SNR_THR_2*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, SNR_THR_2))->Bits.NR3D_centvar_tolerance =  NR3D_centvar_tolerance_after;
            reinterpret_cast<NR3D_REG_D1A_NR3D_NR3D_SNR_THR_3*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, SNR_THR_3))->Bits.NR3D_sad_invwei        =  NR3D_sad_invwei_after;
            reinterpret_cast<NR3D_REG_D1A_NR3D_NR3D_SNR_THR_3*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, SNR_THR_3))->Bits.NR3D_cent_cand_wei     =  NR3D_cent_cand_wei_after;

            MY_LOG_IF(bPrintAdaptiveNR3D, "Device_ID(%d), Before BlkVar=%d, CentVar=%d, InvWei(%d), CentWei(%d)\n",
                Device_ID, NR3D_blkvar_tolerance_before, NR3D_centvar_tolerance_before, NR3D_sad_invwei_before, NR3D_cent_cand_wei_before);
            MY_LOG_IF(bPrintAdaptiveNR3D, "Device_ID(%d), After BlkVar=%d, CentVar=%d, InvWei(%d), CentWei(%d)\n",
                Device_ID, NR3D_blkvar_tolerance_after, NR3D_centvar_tolerance_after, NR3D_sad_invwei_after, NR3D_cent_cand_wei_after);
        }
        else
        {
            MY_LOG_IF(bPrintAdaptiveNR3D, "iX1subX0 = 0\n");
        }
        // End 2018/10/16 Adaptive for SNR
    }
    ///////////////end of 20170316

    /////////////end of SMOOTH GMV/ISO part

    ////////////DUMP
    MY_LOG_IF(bPrintAdaptiveNR3D, "Device_ID(%d), MAX_GMV:(%d %d), (GMVX:%d,GMVY:%d), (confX:%d,confY:%d), m_GMV_level_th:%d, GMV_level_str:%d, u4RealISO:%d, ISO_level_str:%d, ISO_switch:%d",
        Device_ID, MAX_GMVX, MAX_GMVY, GMVX, GMVY, confX, confY, GMVLevelTh, GMV_level_str, u4RealISO, ISO_level_str, ISO_switch);

    MY_LOG_IF(bPrintAdaptiveNR3D, "Device_ID(%d), MAX_FLTSTRbefore=%d, MAX_FLTSTRafter=%d, disableAdaptiveSetting(%d)\n",
        Device_ID, NR3D_FLT_STR_MAX_before, NR3D_FLT_STR_MAX_after, disableAdaptiveSetting);

    m_GMV_level_th = GMVLevelTh;

}

MBOOL
ISP_MGR_NR3D_T::
apply_P2(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, dip_a_reg_t* pReg)
{
    MBOOL bEnable = isEnable(SubModuleIndex);

    //Top Control
    switch (SubModuleIndex)
    {
        case ENR3D_D1:
            ISP_WRITE_ENABLE_BITS(pReg, DIPCTL_D1A_DIPCTL_YUV_EN2, DIPCTL_NR3D_D1_EN, bEnable);
            break;
        default:
            CAM_LOGE("Apply Error Submodule Index: %d", SubModuleIndex);
            return  MFALSE;
    }

    //R2C calculation
    ADD_AND_WRITE_R2CF_CNT(SubModuleIndex, NR3D_r2c_val1, NR3D_r2cf_cnt1);
    ADD_AND_WRITE_R2CF_CNT(SubModuleIndex, NR3D_r2c_val2, NR3D_r2cf_cnt2);
    ADD_AND_WRITE_R2CF_CNT(SubModuleIndex, NR3D_r2c_val3, NR3D_r2cf_cnt3);
    ADD_AND_WRITE_R2CF_CNT(SubModuleIndex, NR3D_r2c_val4, NR3D_r2cf_cnt4);

    adaptive_NR3D_setting(SubModuleIndex, rRawIspCamInfo, pReg);

    MBOOL       bEnableSL2E = MFALSE; // enable sl2e
    MBOOL       bEnableDemo = MFALSE; // enable demo mode
    MBOOL       bEnableBM = MFALSE; // enable benchmark mode
    MBOOL       bfixTuning = MFALSE;
    MBOOL       bForceDisableR2C = MFALSE;

    if (bEnable)
    {
        const NSCam::NR3D::NR3DIspConfigInfo &cofI = rRawIspCamInfo.NR3D_Data.configInfo;
        setConfig(cofI.onRegion, cofI.fullImg, cofI.vipiOffst, cofI.vipiReadSize);

        bEnableSL2E = MTRUE; // enable sl2e

        if (rRawIspCamInfo.NR3D_Data.debugEnable)
        {
            bEnableSL2E = ::property_get_int32("vendor.debug.3dnr.sl2e.enable", 1); // sl2e: default on
            bEnableDemo = ::property_get_int32("vendor.debug.3dnr.demo.enable", 0);
            bEnableBM = ::property_get_int32("vendor.debug.nr3d.bm.enable", 0);
            bfixTuning = ::property_get_int32("vendor.debug.3dnr.fix.tuning", 0);
            bForceDisableR2C = ::property_get_int32("vendor.debug.3dnr.disable.r2c", 0);

            CAM_LOGD_IF(1, "bEnableSL2E(%d), bEnableDemo(%d), bEnableBM(%d), bfixTuning(%d), bForceDisableR2C(%d)",
                bEnableSL2E, bEnableDemo, bEnableBM, bfixTuning, bForceDisableR2C);
        }

        // turn on NR3D by NR3D_CAM_TNR_EN
        reinterpret_cast<NR3D_REG_D1A_NR3D_NR3D_ENG_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, ENG_CON))->Bits.NR3D_yclnr_en =  bEnable;
        //ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_CAM_TNR_ENG_CON, NR3D_CAM_TNR_EN, bEnable);
        // no need NR3D_TNR_Y_EN must equal NR3D_CAM_TNR_EN
        // no need ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_MDP_TNR_TNR_ENABLE, NR3D_TNR_Y_EN, bEnable);
        // nr3d default
        reinterpret_cast<NR3D_REG_D1A_NR3D_NR3D_ON_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, ON_CON))->Raw =  0x00100F00;
        //ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_NR3D_ON_CON, 0x00100F00);
        reinterpret_cast<NR3D_REG_D1A_NR3D_NR3D_ON_OFF*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, ON_OFF))->Raw =  0x00000000;
        //ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_NR3D_ON_OFF, 0x00000000);
        reinterpret_cast<NR3D_REG_D1A_NR3D_NR3D_ON_SIZ*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, ON_SIZ))->Raw =  0x00000000;
        //ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_NR3D_ON_SIZ, 0x00000000);

        //-------------------using default tuning value----------------------------------------
        if (bfixTuning)
        {
            // todo fixTuningValue(pIspPhyReg);
        }

        //------------------------------------------------------------
        if (bEnableDemo)
        {
            demoNR3D(bEnableSL2E, m_fullImg, m_onRegion);
        }

        // nr3d_on
        // OFST is relative position to IMGI full image
        reinterpret_cast<NR3D_REG_D1A_NR3D_NR3D_ON_OFF*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, ON_OFF))->Bits.NR3D_nr3d_on_ofst_x = (m_onRegion.p.x + m_fullImg.p.x);
        //ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_NR3D_ON_OFF, NR3D_ON_OFST_X, m_onRegion.p.x + m_fullImg.p.x);
        reinterpret_cast<NR3D_REG_D1A_NR3D_NR3D_ON_OFF*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, ON_OFF))->Bits.NR3D_nr3d_on_ofst_y = (m_onRegion.p.y + m_fullImg.p.y);
        //ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_NR3D_ON_OFF, NR3D_ON_OFST_Y, m_onRegion.p.y + m_fullImg.p.y);
        reinterpret_cast<NR3D_REG_D1A_NR3D_NR3D_ON_SIZ*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, ON_SIZ))->Bits.NR3D_nr3d_on_wd = m_onRegion.s.w;
        //ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_NR3D_ON_SIZ, NR3D_ON_WD, m_onRegion.s.w);
        reinterpret_cast<NR3D_REG_D1A_NR3D_NR3D_ON_SIZ*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, ON_SIZ))->Bits.NR3D_nr3d_on_ht = m_onRegion.s.h;
        //ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_NR3D_ON_SIZ, NR3D_ON_HT, m_onRegion.s.h);
        reinterpret_cast<NR3D_REG_D1A_NR3D_NR3D_ON_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, ON_CON))->Bits.NR3D_on_en = bEnable;
        //ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_NR3D_ON_CON, NR3D_ON_EN, bEnable);

        // nr3d_vipi
        reinterpret_cast<NR3D_REG_D1A_NR3D_NR3D_VIPI_OFFSET*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, VIPI_OFFSET))->Bits.NR3D_VIPI_OFFSET = m_vipiOffst;
        //ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_NR3D_VIPI_OFFSET, NR3D_VIPI_OFFSET, m_vipiOffst);
        reinterpret_cast<NR3D_REG_D1A_NR3D_NR3D_VIPI_SIZE*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, VIPI_SIZE))->Bits.NR3D_VIPI_WIDTH = m_vipiReadSize.w;
        //ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_NR3D_VIPI_SIZE, NR3D_VIPI_WIDTH, m_vipiReadSize.w);
        reinterpret_cast<NR3D_REG_D1A_NR3D_NR3D_VIPI_SIZE*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, VIPI_SIZE))->Bits.NR3D_VIPI_HEIGHT = m_vipiReadSize.h;
        //ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_NR3D_VIPI_SIZE, NR3D_VIPI_HEIGHT, m_vipiReadSize.h);

        if (bEnableSL2E)
        {
            if (pReg->DIPCTL_D1A_DIPCTL_YUV_EN2.Bits.DIPCTL_SLK_D5_EN == 0 || pReg->SLK_D5A_SLK_RZ.Bits.SLK_HRZ_COMP == 0 || pReg->SLK_D5A_SLK_RZ.Bits.SLK_VRZ_COMP == 0)
            {
                CAM_LOGD("force disable sl2e! SLK_D5_EN(%d) SLK_HRZ_COMP(%d) SLK_VRZ_COMP(%d)", pReg->DIPCTL_D1A_DIPCTL_YUV_EN2.Bits.DIPCTL_SLK_D5_EN,
                    pReg->SLK_D5A_SLK_RZ.Bits.SLK_HRZ_COMP, pReg->SLK_D5A_SLK_RZ.Bits.SLK_VRZ_COMP);
                bEnableSL2E = MFALSE;
            }
        }

        // sl2e
        // no need ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_CTL_YUV_EN, SL2E_EN, bEnableSL2E);
        reinterpret_cast<NR3D_REG_D1A_NR3D_NR3D_ON_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, ON_CON))->Bits.NR3D_sl2_off = (bEnableSL2E ? 0 : 1);
        //ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_NR3D_ON_CON, NR3D_SL2_OFF, !bEnableSL2E);

        // debug R2C
        if (bForceDisableR2C)
        {
            reinterpret_cast<NR3D_REG_D1A_NR3D_NR3D_R2C_1*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, R2C_1))->Bits.NR3D_r2cenc = 0;
            // todo ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_MDP_TNR_R2C_1, NR3D_R2CENC, 0);
        }
    }
    else
    {
        // turn off NR3D by NR3D_CAM_TNR_EN
        reinterpret_cast<NR3D_REG_D1A_NR3D_NR3D_ENG_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, ENG_CON))->Bits.NR3D_yclnr_en =  0;
        //ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_CAM_TNR_ENG_CON, NR3D_CAM_TNR_EN, bEnable);
        // no need NR3D_TNR_Y_EN must equal NR3D_CAM_TNR_EN
        // no need ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_MDP_TNR_TNR_ENABLE, NR3D_TNR_Y_EN, bEnable);
        reinterpret_cast<NR3D_REG_D1A_NR3D_NR3D_ON_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, ON_CON))->Bits.NR3D_on_en = 0;
        //ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_NR3D_ON_CON, NR3D_ON_EN, bEnable);
        // no need ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_CTL_YUV_EN, SL2E_EN, bEnable);
        reinterpret_cast<NR3D_REG_D1A_NR3D_NR3D_ON_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, ON_CON))->Bits.NR3D_sl2_off = 1;
        //ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_NR3D_ON_CON, NR3D_SL2_OFF, !bEnable);
    }

    writeRegs(static_cast<RegInfo_T*>(m_rIspRegInfo[SubModuleIndex]), m_u4RegInfoNum, pReg);

    if (bEnable || rRawIspCamInfo.NR3D_Data.debugEnable)
    {
        CAM_LOGD("NR3D_EN(%d), SL2E_EN(%d), x,y,w,h(%d,%d,%d,%d), full x,y,w,h(%d,%d,%d,%d),"
            "vipi offst(%d) w,h(%d,%d), reg: R2C en(%d) cnt(%d,%d,%d,%d) on_con(%d,%d,%d,%d) reg vipi(%d,%d,%d) flt_str_max(%d)",
            bEnable, bEnableSL2E, m_onRegion.p.x, m_onRegion.p.y, m_onRegion.s.w, m_onRegion.s.h,
            m_fullImg.p.x, m_fullImg.p.y, m_fullImg.s.w, m_fullImg.s.h,
            m_vipiOffst, m_vipiReadSize.w, m_vipiReadSize.h, pReg->NR3D_D1A_NR3D_NR3D_R2C_1.Bits.NR3D_r2cenc,
            pReg->NR3D_D1A_NR3D_NR3D_R2C_3.Bits.NR3D_r2cf_cnt1, pReg->NR3D_D1A_NR3D_NR3D_R2C_3.Bits.NR3D_r2cf_cnt2,
            pReg->NR3D_D1A_NR3D_NR3D_R2C_3.Bits.NR3D_r2cf_cnt3, pReg->NR3D_D1A_NR3D_NR3D_R2C_3.Bits.NR3D_r2cf_cnt4,
            pReg->NR3D_D1A_NR3D_NR3D_ON_OFF.Bits.NR3D_nr3d_on_ofst_x, pReg->NR3D_D1A_NR3D_NR3D_ON_OFF.Bits.NR3D_nr3d_on_ofst_y,
            pReg->NR3D_D1A_NR3D_NR3D_ON_SIZ.Bits.NR3D_nr3d_on_wd, pReg->NR3D_D1A_NR3D_NR3D_ON_SIZ.Bits.NR3D_nr3d_on_ht,
            pReg->NR3D_D1A_NR3D_NR3D_VIPI_OFFSET.Bits.NR3D_VIPI_OFFSET, pReg->NR3D_D1A_NR3D_NR3D_VIPI_SIZE.Bits.NR3D_VIPI_WIDTH,
            pReg->NR3D_D1A_NR3D_NR3D_VIPI_SIZE.Bits.NR3D_VIPI_HEIGHT, pReg->NR3D_D1A_NR3D_NR3D_FLT_CONFIG.Bits.NR3D_flt_str_max);
    }

    dumpRegInfoP2("NR3D", SubModuleIndex);

    if (bEnableBM)
    {
        benchmarkNR3DRegValue((VOID*)pReg);
    }

    return  MTRUE;
}


}
