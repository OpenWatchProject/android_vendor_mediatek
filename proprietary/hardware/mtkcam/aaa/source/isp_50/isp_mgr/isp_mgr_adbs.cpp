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
#define LOG_TAG "isp_mgr_adbs"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include "isp_mgr.h"
#include "isp_mgr_helper.h"
#include <private/aaa_utils.h>
#include <array>

#define MY_INST_ADBS NS3Av3::INST_T<NSIspTuningv3::ISP_MGR_ADBS_T>
static std::array<MY_INST_ADBS, SENSOR_IDX_MAX> gMultitonADBS;

#define MY_INST_ADBS2 NS3Av3::INST_T<NSIspTuningv3::ISP_MGR_ADBS2_T>
static std::array<MY_INST_ADBS2, SENSOR_IDX_MAX> gMultitonADBS2;

namespace NSIspTuningv3
{

#define CLAMP(x,min,max)       (((x) > (max)) ? (max) : (((x) < (min)) ? (min) : (x)))


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ADBS
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_ADBS_T&
ISP_MGR_ADBS_T::
getInstance(ESensorDev_T const eSensorDev)
{
    auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(eSensorDev);
    MY_ASSERT(i4SensorIdx < SENSOR_IDX_MAX);
    MY_ASSERT(i4SensorIdx >= 0);

    MY_INST_ADBS& rSingleton = gMultitonADBS[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<ISP_MGR_ADBS_T>(eSensorDev);
    } );

    return *(rSingleton.instance);
}

template <>
ISP_MGR_ADBS_T&
ISP_MGR_ADBS_T::
put(ISP_NVRAM_ADBS_T const& rParam)
{
    PUT_REG_INFO(CAM_ADBS_GRAY_BLD_0, gray_bld_0);
    PUT_REG_INFO(CAM_ADBS_GRAY_BLD_1, gray_bld_1);
    PUT_REG_INFO(CAM_ADBS_BIAS_LUT_R0, bias_lut_r0);
    PUT_REG_INFO(CAM_ADBS_BIAS_LUT_R1, bias_lut_r1);
    PUT_REG_INFO(CAM_ADBS_BIAS_LUT_R2, bias_lut_r2);
    PUT_REG_INFO(CAM_ADBS_BIAS_LUT_R3, bias_lut_r3);
    PUT_REG_INFO(CAM_ADBS_BIAS_LUT_G0, bias_lut_g0);
    PUT_REG_INFO(CAM_ADBS_BIAS_LUT_G1, bias_lut_g1);
    PUT_REG_INFO(CAM_ADBS_BIAS_LUT_G2, bias_lut_g2);
    PUT_REG_INFO(CAM_ADBS_BIAS_LUT_G3, bias_lut_g3);
    PUT_REG_INFO(CAM_ADBS_BIAS_LUT_B0, bias_lut_b0);
    PUT_REG_INFO(CAM_ADBS_BIAS_LUT_B1, bias_lut_b1);
    PUT_REG_INFO(CAM_ADBS_BIAS_LUT_B2, bias_lut_b2);
    PUT_REG_INFO(CAM_ADBS_BIAS_LUT_B3, bias_lut_b3);
    return  (*this);
}


template <>
ISP_MGR_ADBS_T&
ISP_MGR_ADBS_T::
get(ISP_NVRAM_ADBS_T& rParam)
{
    GET_REG_INFO(CAM_ADBS_GRAY_BLD_0, gray_bld_0);
    GET_REG_INFO(CAM_ADBS_GRAY_BLD_1, gray_bld_1);
    GET_REG_INFO(CAM_ADBS_BIAS_LUT_R0, bias_lut_r0);
    GET_REG_INFO(CAM_ADBS_BIAS_LUT_R1, bias_lut_r1);
    GET_REG_INFO(CAM_ADBS_BIAS_LUT_R2, bias_lut_r2);
    GET_REG_INFO(CAM_ADBS_BIAS_LUT_R3, bias_lut_r3);
    GET_REG_INFO(CAM_ADBS_BIAS_LUT_G0, bias_lut_g0);
    GET_REG_INFO(CAM_ADBS_BIAS_LUT_G1, bias_lut_g1);
    GET_REG_INFO(CAM_ADBS_BIAS_LUT_G2, bias_lut_g2);
    GET_REG_INFO(CAM_ADBS_BIAS_LUT_G3, bias_lut_g3);
    GET_REG_INFO(CAM_ADBS_BIAS_LUT_B0, bias_lut_b0);
    GET_REG_INFO(CAM_ADBS_BIAS_LUT_B1, bias_lut_b1);
    GET_REG_INFO(CAM_ADBS_BIAS_LUT_B2, bias_lut_b2);
    GET_REG_INFO(CAM_ADBS_BIAS_LUT_B3, bias_lut_b3);
    return  (*this);
}

MBOOL
ISP_MGR_ADBS_T::
apply(EIspProfile_T /*eIspProfile*/, RAWIspCamInfo& rRawIspCamInfo, TuningMgr& rTuning, MINT32 i4SubsampleIdex)
{
    MBOOL bADBS_EN = isEnable();



    if(ISP_MGR_DBS::getInstance(m_eSensorDev).isEnable()){
        bADBS_EN = MFALSE;
    }

    MBOOL bSL2F = ISP_MGR_SL2F::getInstance(m_eSensorDev).isEnable();
/*
    //HW constrain
    if(!bSL2F){
        reinterpret_cast<ISP_CAM_ADBS_CTL_T*>(REG_INFO_VALUE_PTR(CAM_ADBS_CTL))->DBS_SL_EN = 0;
    }*/
    //reinterpret_cast<CAM_ADBS_HDR_T*>(REG_INFO_VALUE_PTR(CAM_ADBS_HDR))->ADBS_HDR_OSCTH = 4090;

    rTuning.updateEngine(eTuningMgrFunc_ADBS, bADBS_EN, i4SubsampleIdex);

    // TOP
    TUNING_MGR_WRITE_BITS_CAM((&rTuning), CAM_CTL_EN2, ADBS_EN, bADBS_EN, i4SubsampleIdex);
    ISP_MGR_CTL_EN_P1_T::getInstance(m_eSensorDev).setEnable_ADBS(bADBS_EN);

    MUINT32 GAIN_R = rRawIspCamInfo.rRPG.gain_2.bits.RPG_GAIN_R;
    MUINT32 GAIN_B = rRawIspCamInfo.rRPG.gain_1.bits.RPG_GAIN_B;
    MUINT32 GAIN_G = (rRawIspCamInfo.rRPG.gain_2.bits.RPG_GAIN_GR + rRawIspCamInfo.rRPG.gain_1.bits.RPG_GAIN_GB + 1) >> 1;

    reinterpret_cast<ISP_CAM_ADBS_GAIN_0_T*>(REG_INFO_VALUE_PTR(CAM_ADBS_GAIN_0))->ADBS_GAIN_R = GAIN_R;
    reinterpret_cast<ISP_CAM_ADBS_GAIN_0_T*>(REG_INFO_VALUE_PTR(CAM_ADBS_GAIN_0))->ADBS_GAIN_B = GAIN_B;
    reinterpret_cast<ISP_CAM_ADBS_GAIN_1_T*>(REG_INFO_VALUE_PTR(CAM_ADBS_GAIN_1))->ADBS_GAIN_G = GAIN_G;

    MUINT32 IVGN_R = ((1<<19)/(GAIN_R+1))>>1;
    MUINT32 IVGN_B = ((1<<19)/(GAIN_B+1))>>1;
    MUINT32 IVGN_G = ((1<<19)/(GAIN_G+1))>>1;

    reinterpret_cast<ISP_CAM_ADBS_GAIN_0_T*>(REG_INFO_VALUE_PTR(CAM_ADBS_IVGN_0))->ADBS_GAIN_R = IVGN_R;
    reinterpret_cast<ISP_CAM_ADBS_GAIN_0_T*>(REG_INFO_VALUE_PTR(CAM_ADBS_IVGN_0))->ADBS_GAIN_B = IVGN_B;
    reinterpret_cast<ISP_CAM_ADBS_GAIN_1_T*>(REG_INFO_VALUE_PTR(CAM_ADBS_IVGN_1))->ADBS_GAIN_G = IVGN_G;

    // Register setting
    rTuning.tuningMgrWriteRegs(static_cast<TUNING_MGR_REG_IO_STRUCT*>(m_pRegInfo),
            m_u4RegInfoNum, i4SubsampleIdex);

    dumpRegInfoP1("ADBS");

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DBS2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_ADBS2_T&
ISP_MGR_ADBS2_T::
getInstance(ESensorDev_T const eSensorDev)
{
    auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(eSensorDev);
    MY_ASSERT(i4SensorIdx < SENSOR_IDX_MAX);
    MY_ASSERT(i4SensorIdx >= 0);


    MY_INST_ADBS2& rSingleton = gMultitonADBS2[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<ISP_MGR_ADBS2_T>(eSensorDev);
    } );

    return *(rSingleton.instance);
}

template <>
ISP_MGR_ADBS2_T&
ISP_MGR_ADBS2_T::
put(ISP_NVRAM_ADBS_T const& rParam)
{
    PUT_REG_INFO(DIP_X_ADBS2_GRAY_BLD_0, gray_bld_0);
    PUT_REG_INFO(DIP_X_ADBS2_GRAY_BLD_1, gray_bld_1);
    PUT_REG_INFO(DIP_X_ADBS2_BIAS_LUT_R0, bias_lut_r0);
    PUT_REG_INFO(DIP_X_ADBS2_BIAS_LUT_R1, bias_lut_r1);
    PUT_REG_INFO(DIP_X_ADBS2_BIAS_LUT_R2, bias_lut_r2);
    PUT_REG_INFO(DIP_X_ADBS2_BIAS_LUT_R3, bias_lut_r3);
    PUT_REG_INFO(DIP_X_ADBS2_BIAS_LUT_G0, bias_lut_g0);
    PUT_REG_INFO(DIP_X_ADBS2_BIAS_LUT_G1, bias_lut_g1);
    PUT_REG_INFO(DIP_X_ADBS2_BIAS_LUT_G2, bias_lut_g2);
    PUT_REG_INFO(DIP_X_ADBS2_BIAS_LUT_G3, bias_lut_g3);
    PUT_REG_INFO(DIP_X_ADBS2_BIAS_LUT_B0, bias_lut_b0);
    PUT_REG_INFO(DIP_X_ADBS2_BIAS_LUT_B1, bias_lut_b1);
    PUT_REG_INFO(DIP_X_ADBS2_BIAS_LUT_B2, bias_lut_b2);
    PUT_REG_INFO(DIP_X_ADBS2_BIAS_LUT_B3, bias_lut_b3);
    return  (*this);
}

template <>
MBOOL
ISP_MGR_ADBS2_T::
get(ISP_NVRAM_ADBS_T& rParam, const dip_x_reg_t* pReg) const
{
    if (pReg)
    {
        GET_REG_INFO_BUF(DIP_X_ADBS2_GRAY_BLD_0, gray_bld_0);
        GET_REG_INFO_BUF(DIP_X_ADBS2_GRAY_BLD_1, gray_bld_1);
        GET_REG_INFO_BUF(DIP_X_ADBS2_BIAS_LUT_R0, bias_lut_r0);
        GET_REG_INFO_BUF(DIP_X_ADBS2_BIAS_LUT_R1, bias_lut_r1);
        GET_REG_INFO_BUF(DIP_X_ADBS2_BIAS_LUT_R2, bias_lut_r2);
        GET_REG_INFO_BUF(DIP_X_ADBS2_BIAS_LUT_R3, bias_lut_r3);
        GET_REG_INFO_BUF(DIP_X_ADBS2_BIAS_LUT_G0, bias_lut_g0);
        GET_REG_INFO_BUF(DIP_X_ADBS2_BIAS_LUT_G1, bias_lut_g1);
        GET_REG_INFO_BUF(DIP_X_ADBS2_BIAS_LUT_G2, bias_lut_g2);
        GET_REG_INFO_BUF(DIP_X_ADBS2_BIAS_LUT_G3, bias_lut_g3);
        GET_REG_INFO_BUF(DIP_X_ADBS2_BIAS_LUT_B0, bias_lut_b0);
        GET_REG_INFO_BUF(DIP_X_ADBS2_BIAS_LUT_B1, bias_lut_b1);
        GET_REG_INFO_BUF(DIP_X_ADBS2_BIAS_LUT_B2, bias_lut_b2);
        GET_REG_INFO_BUF(DIP_X_ADBS2_BIAS_LUT_B3, bias_lut_b3);
    }
    return MTRUE;
}

MBOOL
ISP_MGR_ADBS2_T::
apply(EIspProfile_T /*eIspProfile*/, const RAWIspCamInfo& rRawIspCamInfo, dip_x_reg_t* pReg)
{
    MBOOL bADBS2_EN = isEnable();

    // TOP
    ISP_WRITE_ENABLE_BITS(pReg, DIP_X_CTL_RGB2_EN, ADBS2_EN, bADBS2_EN);

    ISP_NVRAM_PGN_T pgn;
    getIspHWBuf(m_eSensorDev, pgn );

    MUINT32 GAIN_R = pgn.gain_2.bits.PGN_GAIN_R;
    MUINT32 GAIN_B = pgn.gain_1.bits.PGN_GAIN_B;
    MUINT32 GAIN_G = (pgn.gain_2.bits.PGN_GAIN_GR + pgn.gain_1.bits.PGN_GAIN_GB + 1) >> 1;

    reinterpret_cast<ISP_CAM_ADBS_GAIN_0_T*>(REG_INFO_VALUE_PTR(DIP_X_ADBS2_GAIN_0))->ADBS_GAIN_R = GAIN_R;
    reinterpret_cast<ISP_CAM_ADBS_GAIN_0_T*>(REG_INFO_VALUE_PTR(DIP_X_ADBS2_GAIN_0))->ADBS_GAIN_B = GAIN_B;
    reinterpret_cast<ISP_CAM_ADBS_GAIN_1_T*>(REG_INFO_VALUE_PTR(DIP_X_ADBS2_GAIN_1))->ADBS_GAIN_G = GAIN_G;

    MUINT32 IVGN_R = ((1<<19)/(GAIN_R+1))>>1;
    MUINT32 IVGN_B = ((1<<19)/(GAIN_B+1))>>1;
    MUINT32 IVGN_G = ((1<<19)/(GAIN_G+1))>>1;

    reinterpret_cast<ISP_CAM_ADBS_GAIN_0_T*>(REG_INFO_VALUE_PTR(DIP_X_ADBS2_IVGN_0))->ADBS_GAIN_R = IVGN_R;
    reinterpret_cast<ISP_CAM_ADBS_GAIN_0_T*>(REG_INFO_VALUE_PTR(DIP_X_ADBS2_IVGN_0))->ADBS_GAIN_B = IVGN_B;
    reinterpret_cast<ISP_CAM_ADBS_GAIN_1_T*>(REG_INFO_VALUE_PTR(DIP_X_ADBS2_IVGN_1))->ADBS_GAIN_G = IVGN_G;
    //reinterpret_cast<DIP_X_ADBS2_HDR_T*>(REG_INFO_VALUE_PTR(DIP_X_ADBS2_HDR))->ADBS_HDR_OSCTH = 4090;

    // Register setting
    writeRegs(static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum, pReg);


    dumpRegInfo("ADBS2");

    return  MTRUE;
}

}
