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
#define LOG_TAG "isp_mgr_nbc2"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include <camera_custom_nvram.h>
#include "isp_mgr.h"
#include <private/aaa_utils.h>
#include <array>

using namespace NSIspTuningv3;

#define MY_INST NS3Av3::INST_T<ISP_MGR_NBC2_T>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

namespace NSIspTuningv3
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// NBC2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_NBC2_T&
ISP_MGR_NBC2_T::
getInstance(ESensorDev_T const eSensorDev)
{
    auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(eSensorDev);

    MY_ASSERT(i4SensorIdx < SENSOR_IDX_MAX);
    MY_ASSERT(i4SensorIdx >= 0);


    MY_INST& rSingleton = gMultiton[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<ISP_MGR_NBC2_T>(eSensorDev);
    } );

    return *(rSingleton.instance);
}

template <>
ISP_MGR_NBC2_T&
ISP_MGR_NBC2_T::
put(ISP_NVRAM_ANR2_T const& rParam)
{
    PUT_REG_INFO(DIP_X_NBC2_ANR2_CON1 ,con1);
    PUT_REG_INFO(DIP_X_NBC2_ANR2_CON2 ,con2);
    PUT_REG_INFO(DIP_X_NBC2_ANR2_YAD1 ,yad1);
    PUT_REG_INFO(DIP_X_NBC2_ANR2_Y4LUT1 ,y4lut1);
    PUT_REG_INFO(DIP_X_NBC2_ANR2_Y4LUT2 ,y4lut2);
    PUT_REG_INFO(DIP_X_NBC2_ANR2_Y4LUT3 ,y4lut3);
    PUT_REG_INFO(DIP_X_NBC2_ANR2_L4LUT1 ,l4lut1);
    PUT_REG_INFO(DIP_X_NBC2_ANR2_L4LUT2 ,l4lut2);
    PUT_REG_INFO(DIP_X_NBC2_ANR2_L4LUT3 ,l4lut3);
    PUT_REG_INFO(DIP_X_NBC2_ANR2_CAD ,cad);
    PUT_REG_INFO(DIP_X_NBC2_ANR2_PTC ,ptc);
    PUT_REG_INFO(DIP_X_NBC2_ANR2_SL2 ,sl2);
    PUT_REG_INFO(DIP_X_NBC2_ANR2_MED1 ,med1);
    PUT_REG_INFO(DIP_X_NBC2_ANR2_MED2 ,med2);
    PUT_REG_INFO(DIP_X_NBC2_ANR2_MED3 ,med3);
    PUT_REG_INFO(DIP_X_NBC2_ANR2_MED4 ,med4);
    PUT_REG_INFO(DIP_X_NBC2_ANR2_MED5 ,med5);
    PUT_REG_INFO(DIP_X_NBC2_ANR2_ACTC ,actc);
    return  (*this);
}

template <>
MBOOL
ISP_MGR_NBC2_T::
get(ISP_NVRAM_ANR2_T& rParam, const dip_x_reg_t* pReg) const
{
    if (pReg)
    {
        GET_REG_INFO_BUF(DIP_X_NBC2_ANR2_CON1 ,con1);
        GET_REG_INFO_BUF(DIP_X_NBC2_ANR2_CON2 ,con2);
        GET_REG_INFO_BUF(DIP_X_NBC2_ANR2_YAD1 ,yad1);
        GET_REG_INFO_BUF(DIP_X_NBC2_ANR2_Y4LUT1 ,y4lut1);
        GET_REG_INFO_BUF(DIP_X_NBC2_ANR2_Y4LUT2 ,y4lut2);
        GET_REG_INFO_BUF(DIP_X_NBC2_ANR2_Y4LUT3 ,y4lut3);
        GET_REG_INFO_BUF(DIP_X_NBC2_ANR2_L4LUT1 ,l4lut1);
        GET_REG_INFO_BUF(DIP_X_NBC2_ANR2_L4LUT2 ,l4lut2);
        GET_REG_INFO_BUF(DIP_X_NBC2_ANR2_L4LUT3 ,l4lut3);
        GET_REG_INFO_BUF(DIP_X_NBC2_ANR2_CAD ,cad);
        GET_REG_INFO_BUF(DIP_X_NBC2_ANR2_PTC ,ptc);
        GET_REG_INFO_BUF(DIP_X_NBC2_ANR2_SL2 ,sl2);
        GET_REG_INFO_BUF(DIP_X_NBC2_ANR2_MED1 ,med1);
        GET_REG_INFO_BUF(DIP_X_NBC2_ANR2_MED2 ,med2);
        GET_REG_INFO_BUF(DIP_X_NBC2_ANR2_MED3 ,med3);
        GET_REG_INFO_BUF(DIP_X_NBC2_ANR2_MED4 ,med4);
        GET_REG_INFO_BUF(DIP_X_NBC2_ANR2_MED5 ,med5);
        GET_REG_INFO_BUF(DIP_X_NBC2_ANR2_ACTC ,actc);
    }
    return MTRUE;
}

template <>
ISP_MGR_NBC2_T&
ISP_MGR_NBC2_T::
put(ISP_NVRAM_CCR_T const& rParam)
{

    PUT_REG_INFO(DIP_X_NBC2_CCR_CON ,con);
    PUT_REG_INFO(DIP_X_NBC2_CCR_YLUT ,ylut);
    PUT_REG_INFO(DIP_X_NBC2_CCR_UVLUT ,uvlut);
    PUT_REG_INFO(DIP_X_NBC2_CCR_YLUT2 ,ylut2);
    PUT_REG_INFO(DIP_X_NBC2_CCR_SAT_CTRL ,sat_ctrl);
    PUT_REG_INFO(DIP_X_NBC2_CCR_UVLUT_SP ,uvlut_sp);
    PUT_REG_INFO(DIP_X_NBC2_CCR_HUE1 ,hue1);
    PUT_REG_INFO(DIP_X_NBC2_CCR_HUE2 ,hue2);
    PUT_REG_INFO(DIP_X_NBC2_CCR_HUE3 ,hue3);
    PUT_REG_INFO(DIP_X_NBC2_CCR_L4LUT1 ,l4lut1);
    PUT_REG_INFO(DIP_X_NBC2_CCR_L4LUT2 ,l4lut2);
    PUT_REG_INFO(DIP_X_NBC2_CCR_L4LUT3 ,l4lut3);

    return  (*this);
}

template <>
MBOOL
ISP_MGR_NBC2_T::
get(ISP_NVRAM_CCR_T& rParam, const dip_x_reg_t* pReg) const
{
    if (pReg)
    {
        GET_REG_INFO_BUF(DIP_X_NBC2_CCR_CON ,con);
        GET_REG_INFO_BUF(DIP_X_NBC2_CCR_YLUT ,ylut);
        GET_REG_INFO_BUF(DIP_X_NBC2_CCR_UVLUT ,uvlut);
        GET_REG_INFO_BUF(DIP_X_NBC2_CCR_YLUT2 ,ylut2);
        GET_REG_INFO_BUF(DIP_X_NBC2_CCR_SAT_CTRL ,sat_ctrl);
        GET_REG_INFO_BUF(DIP_X_NBC2_CCR_UVLUT_SP ,uvlut_sp);
        GET_REG_INFO_BUF(DIP_X_NBC2_CCR_HUE1 ,hue1);
        GET_REG_INFO_BUF(DIP_X_NBC2_CCR_HUE2 ,hue2);
        GET_REG_INFO_BUF(DIP_X_NBC2_CCR_HUE3 ,hue3);
        GET_REG_INFO_BUF(DIP_X_NBC2_CCR_L4LUT1 ,l4lut1);
        GET_REG_INFO_BUF(DIP_X_NBC2_CCR_L4LUT2 ,l4lut2);
        GET_REG_INFO_BUF(DIP_X_NBC2_CCR_L4LUT3 ,l4lut3);
    }
    return MTRUE;
}



template <>
ISP_MGR_NBC2_T&
ISP_MGR_NBC2_T::
put(ISP_NVRAM_BOK_T const& rParam)
{
#if 0
    PUT_REG_INFO(DIP_X_NBC2_BOK_CON ,bok_con);
    PUT_REG_INFO(DIP_X_NBC2_BOK_TUN ,bok_tun);
#endif
    return  (*this);
}

template <>
MBOOL
ISP_MGR_NBC2_T::
get(ISP_NVRAM_BOK_T& rParam, const dip_x_reg_t* pReg) const
{
#if 0
    if (pReg)
    {

        GET_REG_INFO_BUF(DIP_X_NBC2_BOK_CON ,bok_con);
        GET_REG_INFO_BUF(DIP_X_NBC2_BOK_TUN ,bok_tun);

    }
#endif
    return MTRUE;
}

template <>
ISP_MGR_NBC2_T&
ISP_MGR_NBC2_T::
put(ISP_NVRAM_ABF_T const&rParam)
{
    PUT_REG_INFO(DIP_X_NBC2_ABF_CON1 ,con1);
    PUT_REG_INFO(DIP_X_NBC2_ABF_CON2 ,con2);
    PUT_REG_INFO(DIP_X_NBC2_ABF_RCON ,rcon);
    PUT_REG_INFO(DIP_X_NBC2_ABF_YLUT ,ylut);
    PUT_REG_INFO(DIP_X_NBC2_ABF_CXLUT ,cxlut);
    PUT_REG_INFO(DIP_X_NBC2_ABF_CYLUT ,cylut);
    PUT_REG_INFO(DIP_X_NBC2_ABF_YSP ,ysp);
    PUT_REG_INFO(DIP_X_NBC2_ABF_CXSP ,cxsp);
    PUT_REG_INFO(DIP_X_NBC2_ABF_CYSP ,cysp);
    PUT_REG_INFO(DIP_X_NBC2_ABF_CLP ,clp);
    return  (*this);
}

template <>
MBOOL
ISP_MGR_NBC2_T::
get(ISP_NVRAM_ABF_T& rParam, const dip_x_reg_t* pReg) const
{
    if (pReg)
    {
        GET_REG_INFO_BUF(DIP_X_NBC2_ABF_CON1 ,con1);
        GET_REG_INFO_BUF(DIP_X_NBC2_ABF_CON2 ,con2);
        GET_REG_INFO_BUF(DIP_X_NBC2_ABF_RCON ,rcon);
        GET_REG_INFO_BUF(DIP_X_NBC2_ABF_YLUT ,ylut);
        GET_REG_INFO_BUF(DIP_X_NBC2_ABF_CXLUT ,cxlut);
        GET_REG_INFO_BUF(DIP_X_NBC2_ABF_CYLUT ,cylut);
        GET_REG_INFO_BUF(DIP_X_NBC2_ABF_YSP ,ysp);
        GET_REG_INFO_BUF(DIP_X_NBC2_ABF_CXSP ,cxsp);
        GET_REG_INFO_BUF(DIP_X_NBC2_ABF_CYSP ,cysp);
        GET_REG_INFO_BUF(DIP_X_NBC2_ABF_CLP ,clp);
    }
    return MTRUE;
}


MBOOL
ISP_MGR_NBC2_T::
apply(EIspProfile_T eIspProfile, dip_x_reg_t* pReg)
{
    MBOOL bANR2_ENC = ((reinterpret_cast<ISP_DIP_X_NBC2_ANR2_CON1_T*>(REG_INFO_VALUE_PTR(DIP_X_NBC2_ANR2_CON1))->NBC2_ANR2_ENC) & isANR2Enable());
    MBOOL bANR2_ENY = ((reinterpret_cast<ISP_DIP_X_NBC2_ANR2_CON1_T*>(REG_INFO_VALUE_PTR(DIP_X_NBC2_ANR2_CON1))->NBC2_ANR2_ENY) & isANR2Enable());

    MBOOL bCCR_EN = ((reinterpret_cast<ISP_DIP_X_NBC2_CCR_CON_T*>(REG_INFO_VALUE_PTR(DIP_X_NBC2_CCR_CON))->NBC2_CCR_EN) & isCCREnable());

    MBOOL bABF_EN = ((reinterpret_cast<ISP_DIP_X_NBC2_ABF_CON1_T*>(REG_INFO_VALUE_PTR(DIP_X_NBC2_ABF_CON1))->NBC2_ABF_EN) & isABFEnable());





    MBOOL bNBC2_EN = ( bANR2_ENC | bANR2_ENY | bCCR_EN);
                       //isBOKEnable());
    if(bNBC2_EN){
        reinterpret_cast<ISP_DIP_X_NBC2_ANR2_CON1_T*>(REG_INFO_VALUE_PTR(DIP_X_NBC2_ANR2_CON1))->NBC2_ANR2_MODE = 0;
    }

    if (eIspProfile == (EIspProfile_T) EIspProfile_Bokeh) {
        reinterpret_cast<ISP_DIP_X_NBC2_ANR2_CON1_T*>(REG_INFO_VALUE_PTR(DIP_X_NBC2_ANR2_CON1))->NBC2_ANR2_MODE = 1;
        bNBC2_EN = MTRUE;
        if (m_bDebugEnable)
            ALOGD("NBC2_ANR2_MODE enable with Profile EIspProfile_Bokeh");
    }

   reinterpret_cast<ISP_DIP_X_NBC2_ANR2_CON1_T*>(REG_INFO_VALUE_PTR(DIP_X_NBC2_ANR2_CON1))->NBC2_ANR2_ENY = bANR2_ENY;
   reinterpret_cast<ISP_DIP_X_NBC2_ANR2_CON1_T*>(REG_INFO_VALUE_PTR(DIP_X_NBC2_ANR2_CON1))->NBC2_ANR2_ENC = bANR2_ENC;
   reinterpret_cast<ISP_DIP_X_NBC2_CCR_CON_T*>(REG_INFO_VALUE_PTR(DIP_X_NBC2_CCR_CON))->NBC2_CCR_EN = bCCR_EN;
   reinterpret_cast<ISP_DIP_X_NBC2_ABF_CON1_T*>(REG_INFO_VALUE_PTR(DIP_X_NBC2_ABF_CON1))->NBC2_ABF_EN = bABF_EN;


    // TOP
    ISP_WRITE_ENABLE_BITS(pReg, DIP_X_CTL_YUV_EN, NBC2_EN, bNBC2_EN);

    // Register setting
    writeRegs(static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum, pReg);

    dumpRegInfo("NBC2");

    return  MTRUE;
}
}
