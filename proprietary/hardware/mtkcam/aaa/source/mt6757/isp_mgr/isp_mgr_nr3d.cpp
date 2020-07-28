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
    #define ENABLE_MY_LOG       (0)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include <camera_custom_nvram.h>
#include "isp_mgr.h"

namespace NSIspTuningv3
{

static MBOOL       g_bEnableSL2E = MTRUE; // enable sl2e
static MBOOL       g_bEnableDemo = MFALSE; // enable demo mode
static MBOOL       g_bEnableBM = MFALSE; // enable benchmark mode
static MBOOL       g_bEnable3dnrLog = MFALSE; // enable 3dnr log

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// NR3D
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_NR3D_T&
ISP_MGR_NR3D_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
    {
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_NR3D_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_NR3D_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_NR3D_DEV<ESensorDev_Sub>::getInstance();
    default:
        CAM_LOGE("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_NR3D_DEV<ESensorDev_Main>::getInstance();
    }

    char cValue[PROPERTY_VALUE_MAX];
    ::property_get("vendor.debug.3dnr.sl2e.enable", cValue, "1"); // sl2e: default on
    g_bEnableSL2E = atoi(cValue);

    ::property_get("vendor.debug.3dnr.demo.enable", cValue, "0");
    g_bEnableDemo = atoi(cValue);

    ::property_get("vendor.debug.nr3d.bm.enable", cValue, "0");
    g_bEnableBM = atoi(cValue);

    ::property_get("vendor.camera.3dnr.log.level", cValue, "0");
    g_bEnable3dnrLog = atoi(cValue);
}

template <>
ISP_MGR_NR3D_T&
ISP_MGR_NR3D_T::
put(ISP_NVRAM_NR3D_T const& rParam)
{
    PUT_REG_INFO(DIP_X_NR3D_AY_CON1    , ay_con1);
    PUT_REG_INFO(DIP_X_NR3D_AY_CON2    , ay_con2);
    PUT_REG_INFO(DIP_X_NR3D_AY_CON3    , ay_con3);
    PUT_REG_INFO(DIP_X_NR3D_AY_CON4    , ay_con4);
    PUT_REG_INFO(DIP_X_NR3D_AY_CON5    , ay_con5);
    PUT_REG_INFO(DIP_X_NR3D_AC_CON1    , ac_con1);
    PUT_REG_INFO(DIP_X_NR3D_AC_CON2    , ac_con2);
    PUT_REG_INFO(DIP_X_NR3D_AC_CON3    , ac_con3);
    PUT_REG_INFO(DIP_X_NR3D_AC_CON4    , ac_con4);
    PUT_REG_INFO(DIP_X_NR3D_AC_CON5    , ac_con5);
    PUT_REG_INFO(DIP_X_NR3D_B1Y_CON1   , b1y_con1);
    PUT_REG_INFO(DIP_X_NR3D_B1Y_CON2   , b1y_con2);
    PUT_REG_INFO(DIP_X_NR3D_B1Y_CON3   , b1y_con3);
    PUT_REG_INFO(DIP_X_NR3D_B1C_CON1   , b1c_con1);
    PUT_REG_INFO(DIP_X_NR3D_B1C_CON2   , b1c_con2);
    PUT_REG_INFO(DIP_X_NR3D_B1C_CON3   , b1c_con3);
    PUT_REG_INFO(DIP_X_NR3D_LSCG1_XCON , lscg1_xcon);
    PUT_REG_INFO(DIP_X_NR3D_LSCG1_YCON , lscg1_ycon);
    PUT_REG_INFO(DIP_X_NR3D_LSCG1_CON  , lscg1_con);
    PUT_REG_INFO(DIP_X_NR3D_LSCG1_SCON , lscg1_scon);
    PUT_REG_INFO(DIP_X_NR3D_NLY_CON1   , nly_con1);
    PUT_REG_INFO(DIP_X_NR3D_NLY_CON2   , nly_con2);
    PUT_REG_INFO(DIP_X_NR3D_MLY_CON    , mly_con);
    PUT_REG_INFO(DIP_X_NR3D_NLC_CON1   , nlc_con1);
    PUT_REG_INFO(DIP_X_NR3D_NLC_CON2   , nlc_con2);
    PUT_REG_INFO(DIP_X_NR3D_MLC_CON    , mlc_con);
    PUT_REG_INFO(DIP_X_NR3D_SLY_CON    , sly_con);
    PUT_REG_INFO(DIP_X_NR3D_SLC_CON    , slc_con);
    PUT_REG_INFO(DIP_X_NR3D_DIFG_CON   , difg_con);
    PUT_REG_INFO(DIP_X_NR3D_YCJC_CON1  , ycjc_con1);
    PUT_REG_INFO(DIP_X_NR3D_YCJC_CON2  , ycjc_con2);

    return  (*this);
}

template <>
ISP_MGR_NR3D_T&
ISP_MGR_NR3D_T::
get(ISP_NVRAM_NR3D_T& rParam)
{
    GET_REG_INFO(DIP_X_NR3D_AY_CON1    , ay_con1);
    GET_REG_INFO(DIP_X_NR3D_AY_CON2    , ay_con2);
    GET_REG_INFO(DIP_X_NR3D_AY_CON3    , ay_con3);
    GET_REG_INFO(DIP_X_NR3D_AY_CON4    , ay_con4);
    GET_REG_INFO(DIP_X_NR3D_AY_CON5    , ay_con5);
    GET_REG_INFO(DIP_X_NR3D_AC_CON1    , ac_con1);
    GET_REG_INFO(DIP_X_NR3D_AC_CON2    , ac_con2);
    GET_REG_INFO(DIP_X_NR3D_AC_CON3    , ac_con3);
    GET_REG_INFO(DIP_X_NR3D_AC_CON4    , ac_con4);
    GET_REG_INFO(DIP_X_NR3D_AC_CON5    , ac_con5);
    GET_REG_INFO(DIP_X_NR3D_B1Y_CON1   , b1y_con1);
    GET_REG_INFO(DIP_X_NR3D_B1Y_CON2   , b1y_con2);
    GET_REG_INFO(DIP_X_NR3D_B1Y_CON3   , b1y_con3);
    GET_REG_INFO(DIP_X_NR3D_B1C_CON1   , b1c_con1);
    GET_REG_INFO(DIP_X_NR3D_B1C_CON2   , b1c_con2);
    GET_REG_INFO(DIP_X_NR3D_B1C_CON3   , b1c_con3);
    GET_REG_INFO(DIP_X_NR3D_LSCG1_XCON , lscg1_xcon);
    GET_REG_INFO(DIP_X_NR3D_LSCG1_YCON , lscg1_ycon);
    GET_REG_INFO(DIP_X_NR3D_LSCG1_CON  , lscg1_con);
    GET_REG_INFO(DIP_X_NR3D_LSCG1_SCON , lscg1_scon);
    GET_REG_INFO(DIP_X_NR3D_NLY_CON1   , nly_con1);
    GET_REG_INFO(DIP_X_NR3D_NLY_CON2   , nly_con2);
    GET_REG_INFO(DIP_X_NR3D_MLY_CON    , mly_con);
    GET_REG_INFO(DIP_X_NR3D_NLC_CON1   , nlc_con1);
    GET_REG_INFO(DIP_X_NR3D_NLC_CON2   , nlc_con2);
    GET_REG_INFO(DIP_X_NR3D_MLC_CON    , mlc_con);
    GET_REG_INFO(DIP_X_NR3D_SLY_CON    , sly_con);
    GET_REG_INFO(DIP_X_NR3D_SLC_CON    , slc_con);
    GET_REG_INFO(DIP_X_NR3D_DIFG_CON   , difg_con);
    GET_REG_INFO(DIP_X_NR3D_YCJC_CON1  , ycjc_con1);
    GET_REG_INFO(DIP_X_NR3D_YCJC_CON2  , ycjc_con2);

    return  (*this);
}

template <>
MBOOL
ISP_MGR_NR3D_T::
get(ISP_NVRAM_NR3D_T& rParam, const dip_x_reg_t* pReg) const
{
    if (pReg)
    {
        GET_REG_INFO_BUF(DIP_X_NR3D_AY_CON1    , ay_con1);
        GET_REG_INFO_BUF(DIP_X_NR3D_AY_CON2    , ay_con2);
        GET_REG_INFO_BUF(DIP_X_NR3D_AY_CON3    , ay_con3);
        GET_REG_INFO_BUF(DIP_X_NR3D_AY_CON4    , ay_con4);
        GET_REG_INFO_BUF(DIP_X_NR3D_AY_CON5    , ay_con5);
        GET_REG_INFO_BUF(DIP_X_NR3D_AC_CON1    , ac_con1);
        GET_REG_INFO_BUF(DIP_X_NR3D_AC_CON2    , ac_con2);
        GET_REG_INFO_BUF(DIP_X_NR3D_AC_CON3    , ac_con3);
        GET_REG_INFO_BUF(DIP_X_NR3D_AC_CON4    , ac_con4);
        GET_REG_INFO_BUF(DIP_X_NR3D_AC_CON5    , ac_con5);
        GET_REG_INFO_BUF(DIP_X_NR3D_B1Y_CON1   , b1y_con1);
        GET_REG_INFO_BUF(DIP_X_NR3D_B1Y_CON2   , b1y_con2);
        GET_REG_INFO_BUF(DIP_X_NR3D_B1Y_CON3   , b1y_con3);
        GET_REG_INFO_BUF(DIP_X_NR3D_B1C_CON1   , b1c_con1);
        GET_REG_INFO_BUF(DIP_X_NR3D_B1C_CON2   , b1c_con2);
        GET_REG_INFO_BUF(DIP_X_NR3D_B1C_CON3   , b1c_con3);
        GET_REG_INFO_BUF(DIP_X_NR3D_LSCG1_XCON , lscg1_xcon);
        GET_REG_INFO_BUF(DIP_X_NR3D_LSCG1_YCON , lscg1_ycon);
        GET_REG_INFO_BUF(DIP_X_NR3D_LSCG1_CON  , lscg1_con);
        GET_REG_INFO_BUF(DIP_X_NR3D_LSCG1_SCON , lscg1_scon);
        GET_REG_INFO_BUF(DIP_X_NR3D_NLY_CON1   , nly_con1);
        GET_REG_INFO_BUF(DIP_X_NR3D_NLY_CON2   , nly_con2);
        GET_REG_INFO_BUF(DIP_X_NR3D_MLY_CON    , mly_con);
        GET_REG_INFO_BUF(DIP_X_NR3D_NLC_CON1   , nlc_con1);
        GET_REG_INFO_BUF(DIP_X_NR3D_NLC_CON2   , nlc_con2);
        GET_REG_INFO_BUF(DIP_X_NR3D_MLC_CON    , mlc_con);
        GET_REG_INFO_BUF(DIP_X_NR3D_SLY_CON    , sly_con);
        GET_REG_INFO_BUF(DIP_X_NR3D_SLC_CON    , slc_con);
        GET_REG_INFO_BUF(DIP_X_NR3D_DIFG_CON   , difg_con);
        GET_REG_INFO_BUF(DIP_X_NR3D_YCJC_CON1  , ycjc_con1);
        GET_REG_INFO_BUF(DIP_X_NR3D_YCJC_CON2  , ycjc_con2);
    }

    return MTRUE;
}

MBOOL
ISP_MGR_NR3D_T::
apply(EIspProfile_T eIspProfile, dip_x_reg_t* pReg)
{

    MBOOL bNR3D_EN = isEnable();


    // No NR3D in Olympus
    bNR3D_EN = MFALSE;

    // TOP
    ISP_WRITE_ENABLE_BITS(pReg, DIP_X_CTL_YUV_EN, NR3D_EN, bNR3D_EN);
    //ISP_MGR_CTL_EN_P2_T::getInstance(m_eSensorDev).setEnable_NR3D(bNR3D_EN);

    writeRegs(static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum, pReg);

    dumpRegInfo("NR3D");

    return  MTRUE;
}

static MVOID benchmarkNR3DRegValue(MVOID *pReg)
{
    if (pReg == NULL)
    {
        return;
    }

    dip_x_reg_t *pIspPhyReg = (dip_x_reg_t*) pReg;
    static MINT32 bmCount = 0;
    char cDefaultValue[32];
    char propValue[PROPERTY_VALUE_MAX] = {'\0'};

    MINT32 prop2Int = 0;

    bmCount++;
    // NR3D_SL2_OFF
    sprintf(cDefaultValue, "%d", pIspPhyReg->DIP_X_NR3D_ON_CON.Bits.NR3D_SL2_OFF); // sl2e enabled
    property_get("vendor.debug.nr3d.bm.sl2_off", propValue, cDefaultValue);
    prop2Int = atoi(propValue);
    pIspPhyReg->DIP_X_NR3D_ON_CON.Bits.NR3D_SL2_OFF = prop2Int;
    if (bmCount % 33 == 0)
    {
        CAM_LOGD("NR3D_SL2_OFF =%d", pIspPhyReg->DIP_X_NR3D_ON_CON.Bits.NR3D_SL2_OFF);
    }
    // NR3D_WR_PV_FRM
    sprintf(cDefaultValue, "%d", pIspPhyReg->DIP_X_NR3D_ON_CON.Bits.NR3D_WR_PV_FRM);
    property_get("vendor.debug.nr3d.bm.wr_pv_frm", propValue, cDefaultValue);
    prop2Int = atoi(propValue);
   // CAM_LOGD("ori_NR3D_WR_PV_FRM=%d", pIspPhyReg->DIP_X_NR3D_ON_CON.Bits.NR3D_WR_PV_FRM);
    pIspPhyReg->DIP_X_NR3D_ON_CON.Bits.NR3D_WR_PV_FRM = prop2Int;
    if (bmCount % 33 == 0)
    {
        CAM_LOGD("NR3D_WR_PV_FRM=%d", pIspPhyReg->DIP_X_NR3D_ON_CON.Bits.NR3D_WR_PV_FRM);
    }

    // NR3D_UV_INK_EN
    sprintf(cDefaultValue, "%d", pIspPhyReg->DIP_X_NR3D_ON_CON.Bits.NR3D_UV_INK_EN);
    property_get("vendor.debug.nr3d.bm.uv_ink_en", propValue, cDefaultValue);
    prop2Int = atoi(propValue);
    //CAM_LOGD("ori_NR3D_UV_INK_EN=%d", pIspPhyReg->DIP_X_NR3D_ON_CON.Bits.NR3D_UV_INK_EN);
    pIspPhyReg->DIP_X_NR3D_ON_CON.Bits.NR3D_UV_INK_EN = prop2Int;
    if (bmCount % 33 == 0)
    {
        CAM_LOGD("NR3D_UV_INK_EN=%d", pIspPhyReg->DIP_X_NR3D_ON_CON.Bits.NR3D_UV_INK_EN);
    }

    // NR3D_OVL_INK_EN
    sprintf(cDefaultValue, "%d", pIspPhyReg->DIP_X_NR3D_ON_CON.Bits.NR3D_OVL_INK_EN);
    property_get("vendor.debug.nr3d.bm.ovl_ink_en", propValue, cDefaultValue);
    prop2Int = atoi(propValue);
    //CAM_LOGD("ori_NR3D_OVL_INK_EN=%d", pIspPhyReg->DIP_X_NR3D_ON_CON.Bits.NR3D_OVL_INK_EN);
    pIspPhyReg->DIP_X_NR3D_ON_CON.Bits.NR3D_OVL_INK_EN = prop2Int;
    if (bmCount % 33 == 0)
    {
        CAM_LOGD("NR3D_OVL_INK_EN=%d", pIspPhyReg->DIP_X_NR3D_ON_CON.Bits.NR3D_OVL_INK_EN);
    }

    // NR3D_YMT_INK_EN
    sprintf(cDefaultValue, "%d", pIspPhyReg->DIP_X_NR3D_ON_CON.Bits.NR3D_YMT_INK_EN);
    property_get("vendor.debug.nr3d.bm.ymt_ink_en", propValue, cDefaultValue);
    prop2Int = atoi(propValue);
    //CAM_LOGD("ori_NR3D_YMT_INK_EN=%d", pIspPhyReg->DIP_X_NR3D_ON_CON.Bits.NR3D_YMT_INK_EN);
    pIspPhyReg->DIP_X_NR3D_ON_CON.Bits.NR3D_YMT_INK_EN = prop2Int;
    if (bmCount % 33 == 0)
    {
        CAM_LOGD("NR3D_YMT_INK_EN=%d", pIspPhyReg->DIP_X_NR3D_ON_CON.Bits.NR3D_YMT_INK_EN);
    }

    // NR3D_UV_INK_SEL
    sprintf(cDefaultValue, "%d", pIspPhyReg->DIP_X_NR3D_ON_CON.Bits.NR3D_UV_INK_SEL);
    property_get("vendor.debug.nr3d.bm.nr3d_uv_ink_sel", propValue, cDefaultValue);
    prop2Int = atoi(propValue);
    //CAM_LOGD("ori_NR3D_UV_INK_SEL=%d", pIspPhyReg->DIP_X_NR3D_ON_CON.Bits.NR3D_UV_INK_SEL);
    pIspPhyReg->DIP_X_NR3D_ON_CON.Bits.NR3D_UV_INK_SEL = prop2Int;
    if (bmCount % 33 == 0)
    {
        CAM_LOGD("NR3D_UV_INK_SEL=%d", pIspPhyReg->DIP_X_NR3D_ON_CON.Bits.NR3D_UV_INK_SEL);
    }

    // print sl2e info
    CAM_LOGD("pIspPhyReg->DIP_X_SL2E_RZ.Bits.SL2_HRZ_COMP: %d", pIspPhyReg->DIP_X_SL2E_RZ.Bits.SL2_HRZ_COMP);
    CAM_LOGD("pIspPhyReg->DIP_X_SL2E_RZ.Bits.SL2_VRZ_COMP: %d", pIspPhyReg->DIP_X_SL2E_RZ.Bits.SL2_VRZ_COMP);

}

MBOOL
ISP_MGR_NR3D_T::
post_apply(MBOOL bEnable, void* pReg)
{
    dip_x_reg_t *pIspPhyReg = (dip_x_reg_t*)pReg;

    if (pIspPhyReg == NULL)
    {
        CAM_LOGD("post_apply pReg NULL");
        return  MTRUE;
    }

    if (bEnable)
    {
        ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_CTL_YUV_EN, NR3D_EN, bEnable);
        ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_NR3D_ON_CON, 0x00100F00);
        ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_NR3D_ON_OFF, 0x00000000);
        ISP_WRITE_ENABLE_REG(pIspPhyReg, DIP_X_NR3D_ON_SIZ, 0x00000000);

        MRect onRegion = m_onRegion;

        if (g_bEnableDemo)
        {
            CAM_LOGD("demo NR3D_EN(%d), SL2E_EN(%d), x,y,w,h = (%d,%d,%d,%d), full(%d,%d)",
            bEnable, g_bEnableSL2E, onRegion.p.x, onRegion.p.y, onRegion.s.w, onRegion.s.h,
            m_fullImgSize.w, m_fullImgSize.h);

            if (onRegion.p.x == 0)
            {
                onRegion.s.w =
                    (onRegion.s.w >= m_fullImgSize.w/2)
                    ? m_fullImgSize.w/2
                    : m_onRegion.s.w;
                onRegion.s.w &= ~1;
            }
            else
            {
                MINT32 tmpVal = m_fullImgSize.w/2 - onRegion.p.x;
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

        ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_NR3D_ON_OFF, NR3D_ON_OFST_X, onRegion.p.x);
        ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_NR3D_ON_OFF, NR3D_ON_OFST_Y, onRegion.p.y);
        ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_NR3D_ON_SIZ, NR3D_ON_WD, onRegion.s.w);
        ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_NR3D_ON_SIZ, NR3D_ON_HT, onRegion.s.h);
        ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_NR3D_ON_CON, NR3D_ON_EN, bEnable);

        MBOOL sl2e_enable = g_bEnableSL2E;

        if (g_bEnableSL2E)
        {
            if ((pIspPhyReg->DIP_X_SL2E_RZ.Bits.SL2_HRZ_COMP == 0 || pIspPhyReg->DIP_X_SL2E_RZ.Bits.SL2_VRZ_COMP == 0))
            {
                CAM_LOGD("force disable sl2e! DIP_X_SL2E_RZ = 0");
                sl2e_enable = MFALSE;
            }
        }

        ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_CTL_YUV_EN, SL2E_EN, sl2e_enable);
        ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_NR3D_ON_CON, NR3D_SL2_OFF, !sl2e_enable);

        CAM_LOGD_IF(g_bEnable3dnrLog, "NR3D_EN(%d), SL2E_EN(%d), x,y,w,h = (%d,%d,%d,%d)",
            bEnable, sl2e_enable, onRegion.p.x, onRegion.p.y, onRegion.s.w, onRegion.s.h);

        if (sl2e_enable && g_bEnableBM)
        {
            benchmarkNR3DRegValue((MVOID*)pIspPhyReg);
        }

    }
    else
    {
        CAM_LOGD_IF(g_bEnable3dnrLog, "turn off NR3D_EN, NR3D_ON_EN, SL2E_EN, NR3D_SL2_OFF");

        ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_CTL_YUV_EN, NR3D_EN, bEnable);
        ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_NR3D_ON_CON, NR3D_ON_EN, bEnable);
        ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_CTL_YUV_EN, SL2E_EN, bEnable);
        ISP_WRITE_ENABLE_BITS(pIspPhyReg, DIP_X_NR3D_ON_CON, NR3D_SL2_OFF, !bEnable);
    }

    return  MTRUE;
}


}
