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
#define LOG_TAG "isp_mgr_nbc"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>
#include <camera_custom_nvram.h>
#include "isp_mgr.h"

#include <isp_interpolation/isp_interpolation.h>
#include <private/aaa_utils.h>
#include <array>

using namespace NSIspTuningv3;

#define MY_INST NS3Av3::INST_T<ISP_MGR_NBC_T>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

namespace NSIspTuningv3
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// NBC
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_NBC_T&
ISP_MGR_NBC_T::
getInstance(ESensorDev_T const eSensorDev)
{
    auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(eSensorDev);

    MY_ASSERT(i4SensorIdx < SENSOR_IDX_MAX);
    MY_ASSERT(i4SensorIdx >= 0);


    MY_INST& rSingleton = gMultiton[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<ISP_MGR_NBC_T>(eSensorDev);
    } );

    return *(rSingleton.instance);
}

template <>
ISP_MGR_NBC_T&
ISP_MGR_NBC_T::
put(ISP_NVRAM_ANR_T const& rParam)
{
    PUT_REG_INFO(DIP_X_NBC_ANR_CON1 ,con1);
    PUT_REG_INFO(DIP_X_NBC_ANR_CON2 ,con2);
    PUT_REG_INFO(DIP_X_NBC_ANR_YAD1 ,yad1);
    PUT_REG_INFO(DIP_X_NBC_ANR_YAD2 ,yad2);
    PUT_REG_INFO(DIP_X_NBC_ANR_Y4LUT1 ,y4lut1);
    PUT_REG_INFO(DIP_X_NBC_ANR_Y4LUT2 ,y4lut2);
    PUT_REG_INFO(DIP_X_NBC_ANR_Y4LUT3 ,y4lut3);
    PUT_REG_INFO(DIP_X_NBC_ANR_C4LUT1 ,c4lut1);
    PUT_REG_INFO(DIP_X_NBC_ANR_C4LUT2 ,c4lut2);
    PUT_REG_INFO(DIP_X_NBC_ANR_C4LUT3 ,c4lut3);
    PUT_REG_INFO(DIP_X_NBC_ANR_A4LUT2 ,a4lut2);
    PUT_REG_INFO(DIP_X_NBC_ANR_A4LUT3 ,a4lut3);
    PUT_REG_INFO(DIP_X_NBC_ANR_L4LUT1 ,l4lut1);
    PUT_REG_INFO(DIP_X_NBC_ANR_L4LUT2 ,l4lut2);
    PUT_REG_INFO(DIP_X_NBC_ANR_L4LUT3 ,l4lut3);
    PUT_REG_INFO(DIP_X_NBC_ANR_PTY0V ,pty0v);
    PUT_REG_INFO(DIP_X_NBC_ANR_CAD ,cad);
    PUT_REG_INFO(DIP_X_NBC_ANR_PTY1V ,pty1v);
    PUT_REG_INFO(DIP_X_NBC_ANR_SL2 ,sl2);
    PUT_REG_INFO(DIP_X_NBC_ANR_PTY2V ,pty2v);
    PUT_REG_INFO(DIP_X_NBC_ANR_PTY3V ,pty3v);
    PUT_REG_INFO(DIP_X_NBC_ANR_PTY0H ,pty0h);
    PUT_REG_INFO(DIP_X_NBC_ANR_PTY1H ,pty1h);
    PUT_REG_INFO(DIP_X_NBC_ANR_PTY2H ,pty2h);
    PUT_REG_INFO(DIP_X_NBC_ANR_T4LUT1 ,t4lut1);
    PUT_REG_INFO(DIP_X_NBC_ANR_T4LUT2 ,t4lut2);
    PUT_REG_INFO(DIP_X_NBC_ANR_T4LUT3 ,t4lut3);
    PUT_REG_INFO(DIP_X_NBC_ANR_ACT1 ,act1);
    PUT_REG_INFO(DIP_X_NBC_ANR_PTY3H ,pty3h);
    PUT_REG_INFO(DIP_X_NBC_ANR_PTCV ,ptcv);
    PUT_REG_INFO(DIP_X_NBC_ANR_ACT4 ,act4);
    PUT_REG_INFO(DIP_X_NBC_ANR_PTCH ,ptch);
    PUT_REG_INFO(DIP_X_NBC_ANR_YLVL0 ,ylvl0);
    PUT_REG_INFO(DIP_X_NBC_ANR_YLVL1 ,ylvl1);
    PUT_REG_INFO(DIP_X_NBC_ANR_HF_COR ,hf_cor);
    PUT_REG_INFO(DIP_X_NBC_ANR_HF_ACT0 ,hf_act0);
    PUT_REG_INFO(DIP_X_NBC_ANR_HF_ACT1 ,hf_act1);
    PUT_REG_INFO(DIP_X_NBC_ANR_ACTC ,actc);
    PUT_REG_INFO(DIP_X_NBC_ANR_YLAD ,ylad);
    PUT_REG_INFO(DIP_X_NBC_ANR_HF_ACT2 ,hf_act2);
    PUT_REG_INFO(DIP_X_NBC_ANR_HF_ACT3 ,hf_act3);
    PUT_REG_INFO(DIP_X_NBC_ANR_HF_LUMA0 ,hf_luma0);
    PUT_REG_INFO(DIP_X_NBC_ANR_HF_LUMA1 ,hf_luma1);
    PUT_REG_INFO(DIP_X_NBC_ANR_Y4LUT4 ,y4lut4);
    PUT_REG_INFO(DIP_X_NBC_ANR_Y4LUT5 ,y4lut5);
    PUT_REG_INFO(DIP_X_NBC_ANR_Y4LUT6 ,y4lut6);
    PUT_REG_INFO(DIP_X_NBC_ANR_Y4LUT7 ,y4lut7);
    PUT_REG_INFO(DIP_X_NBC_ANR_A4LUT1 ,a4lut1);

    return  (*this);
}


template <>
ISP_MGR_NBC_T&
ISP_MGR_NBC_T::
put(ISP_NVRAM_ANR_LTM_T const& rParam)
{
    PUT_REG_INFO(DIP_X_NBC_ANR_LTM_GAIN1 ,  ltm_gain1);
    PUT_REG_INFO(DIP_X_NBC_ANR_LTM_GAIN2 ,  ltm_gain2);
    PUT_REG_INFO(DIP_X_NBC_ANR_LTM_LUTP1 ,  ltm_lutp1);
    PUT_REG_INFO(DIP_X_NBC_ANR_LTM_LUTP2 ,  ltm_lutp2);
    PUT_REG_INFO(DIP_X_NBC_ANR_LTM_LUTO1 ,  ltm_luto1);
    PUT_REG_INFO(DIP_X_NBC_ANR_LTM_LUTO2 ,  ltm_luto2);
    PUT_REG_INFO(DIP_X_NBC_ANR_LTM_LUTS1 ,  ltm_luts1);
    PUT_REG_INFO(DIP_X_NBC_ANR_LTM_LUTS2 ,  ltm_luts2);
    PUT_REG_INFO(DIP_X_NBC_ANR_LTM_LUTS3 ,  ltm_luts3);
    PUT_REG_INFO(DIP_X_NBC_ANR_LTM_LUTS4 ,  ltm_luts4);

    return  (*this);
}

template <>
MBOOL
ISP_MGR_NBC_T::
get(ISP_NVRAM_ANR_T& rParam, const dip_x_reg_t* pReg) const
{
    if (pReg)
    {
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_CON1 ,con1);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_CON2 ,con2);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_YAD1 ,yad1);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_YAD2 ,yad2);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_Y4LUT1 ,y4lut1);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_Y4LUT2 ,y4lut2);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_Y4LUT3 ,y4lut3);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_C4LUT1 ,c4lut1);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_C4LUT2 ,c4lut2);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_C4LUT3 ,c4lut3);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_A4LUT2 ,a4lut2);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_A4LUT3 ,a4lut3);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_L4LUT1 ,l4lut1);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_L4LUT2 ,l4lut2);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_L4LUT3 ,l4lut3);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_PTY0V ,pty0v);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_CAD ,cad);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_PTY1V ,pty1v);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_SL2 ,sl2);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_PTY2V ,pty2v);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_PTY3V ,pty3v);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_PTY0H ,pty0h);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_PTY1H ,pty1h);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_PTY2H ,pty2h);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_T4LUT1 ,t4lut1);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_T4LUT2 ,t4lut2);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_T4LUT3 ,t4lut3);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_ACT1 ,act1);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_PTY3H ,pty3h);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_PTCV ,ptcv);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_ACT4 ,act4);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_PTCH ,ptch);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_YLVL0 ,ylvl0);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_YLVL1 ,ylvl1);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_HF_COR ,hf_cor);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_HF_ACT0 ,hf_act0);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_HF_ACT1 ,hf_act1);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_ACTC ,actc);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_YLAD ,ylad);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_HF_ACT2 ,hf_act2);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_HF_ACT3 ,hf_act3);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_HF_LUMA0 ,hf_luma0);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_HF_LUMA1 ,hf_luma1);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_Y4LUT4 ,y4lut4);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_Y4LUT5 ,y4lut5);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_Y4LUT6 ,y4lut6);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_Y4LUT7 ,y4lut7);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_A4LUT1 ,a4lut1);

    }
    return MTRUE;
}

template <>
MBOOL
ISP_MGR_NBC_T::
get(ISP_NVRAM_ANR_LTM_T& rParam, const dip_x_reg_t* pReg) const
{
    if (pReg)
    {
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_LTM_GAIN1 ,  ltm_gain1);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_LTM_GAIN2 ,  ltm_gain2);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_LTM_LUTP1 ,  ltm_lutp1);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_LTM_LUTP2 ,  ltm_lutp2);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_LTM_LUTO1 ,  ltm_luto1);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_LTM_LUTO2 ,  ltm_luto2);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_LTM_LUTS1 ,  ltm_luts1);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_LTM_LUTS2 ,  ltm_luts2);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_LTM_LUTS3 ,  ltm_luts3);
        GET_REG_INFO_BUF(DIP_X_NBC_ANR_LTM_LUTS4 ,  ltm_luts4);
    }
    return MTRUE;
}


MBOOL
ISP_MGR_NBC_T::
apply(EIspProfile_T /*eIspProfile*/, dip_x_reg_t* pReg)
{
#if 1
    MBOOL bANR1_ENC = reinterpret_cast<ISP_DIP_X_NBC_ANR_CON1_T*>(REG_INFO_VALUE_PTR(DIP_X_NBC_ANR_CON1))->NBC_ANR_ENC;
    MBOOL bANR1_ENY = reinterpret_cast<ISP_DIP_X_NBC_ANR_CON1_T*>(REG_INFO_VALUE_PTR(DIP_X_NBC_ANR_CON1))->NBC_ANR_ENY;

    MBOOL bNBC_EN = (isANR1Enable() & (bANR1_ENY|bANR1_ENC));

    if(bNBC_EN){
        MBOOL bANR_TBL_EN = ((reinterpret_cast<ISP_DIP_X_NBC_ANR_CON1_T*>(REG_INFO_VALUE_PTR(DIP_X_NBC_ANR_CON1))->NBC_ANR_TABLE_EN) & isANRTBLEnable());
        reinterpret_cast<ISP_DIP_X_NBC_ANR_CON1_T*>(REG_INFO_VALUE_PTR(DIP_X_NBC_ANR_CON1))->NBC_ANR_TABLE_EN = bANR_TBL_EN;
    }

    reinterpret_cast<ISP_DIP_X_NBC_ANR_CON1_T*>(REG_INFO_VALUE_PTR(DIP_X_NBC_ANR_CON1))->NBC_ANR_LTM_LINK = pReg->DIP_X_CTL_RGB_EN.Bits.LCE_EN;

    ISP_NVRAM_GGM_T ggm;
    ISP_MGR_GGM::getInstance(m_eSensorDev).get(ggm, pReg);

    ISP_NVRAM_LCE_T lce;
    ISP_MGR_LCE::getInstance(m_eSensorDev).get(lce, pReg);

    ISP_NVRAM_ANR_LTM_T Result;
    memset(&Result, 0, sizeof(ISP_NVRAM_ANR_LTM_T));
    ISP_NVRAM_ANR_LUT_T anr_tbl = m_ANR_TBL_Backup;

    AAA_TRACE_ALG(CalculateNBC_LTM);
    Calculate_NBC_LTM(pReg->DIP_X_LCE25_TM_PARA0.Bits.LCE_GLOB_TONE,
                      pReg->DIP_X_LCE25_TM_PARA0.Bits.LCE_LC_TONE,
                      ggm.lut,
                      (MUINT32)pReg->DIP_X_GGM_CTRL.Bits.GGM_END_VAR,
                      lce,
                      anr_tbl,
                      m_SwAnrParam,
                      m_SW_ANR_LTM_Algo_Ver,
                      Result);
    AAA_TRACE_END_ALG;

    put(Result);
    putANR_TBL(anr_tbl);

    // TOP
    ISP_WRITE_ENABLE_BITS(pReg, DIP_X_CTL_YUV_EN, NBC_EN, bNBC_EN);
    //ISP_MGR_CTL_EN_P2_T::getInstance(m_eSensorDev).setEnable_NBC(bNBC_EN);

    //Register setting
    writeRegs(static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum, pReg);
    writeRegs(static_cast<RegInfo_T*>(m_rIspRegInfo_ANR_TBL), ANR_HW_TBL_NUM, pReg);

    dumpRegInfo("NBC");
#endif
    return  MTRUE;
}

}
