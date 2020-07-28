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
#define LOG_TAG "isp_mgr_rnr"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (0)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include "isp_mgr.h"
#include <private/aaa_utils.h>
#include <array>

using namespace NSIspTuningv3;

#define MY_INST NS3Av3::INST_T<ISP_MGR_RNR_T>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

namespace NSIspTuningv3
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// RNR
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_RNR_T&
ISP_MGR_RNR_T::
getInstance(ESensorDev_T const eSensorDev)
{
    auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(eSensorDev);

    MY_ASSERT(i4SensorIdx < SENSOR_IDX_MAX);
    MY_ASSERT(i4SensorIdx >= 0);


    MY_INST& rSingleton = gMultiton[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<ISP_MGR_RNR_T>(eSensorDev);
    } );

    return *(rSingleton.instance);
}

template <>
ISP_MGR_RNR_T&
ISP_MGR_RNR_T::
put(ISP_NVRAM_RNR_T const& rParam)
{
    PUT_REG_INFO(DIP_X_RNR_CON1,  con1);
    PUT_REG_INFO(DIP_X_RNR_VC0,  vc0);
    PUT_REG_INFO(DIP_X_RNR_VC1,  vc1);
    //PUT_REG_INFO(DIP_X_RNR_GO_B,  go_b);
    //PUT_REG_INFO(DIP_X_RNR_GO_G,  go_g);
    //PUT_REG_INFO(DIP_X_RNR_GO_R,  go_r);
    //PUT_REG_INFO(DIP_X_RNR_GAIN_ISO,  gain_iso);
    PUT_REG_INFO(DIP_X_RNR_RNG,  rng);
    PUT_REG_INFO(DIP_X_RNR_CON2,  con2);
    PUT_REG_INFO(DIP_X_RNR_CON3,  con3);
    PUT_REG_INFO(DIP_X_RNR_SL,  sl);
    PUT_REG_INFO(DIP_X_RNR_SSL_STH,  ssl_sth);

    return  (*this);
}

template <>
MBOOL
ISP_MGR_RNR_T::
get(ISP_NVRAM_RNR_GET_T& rParam, const dip_x_reg_t* pReg) const
{
    if (pReg)
    {
        GET_REG_INFO_BUF(DIP_X_RNR_CON1,  con1);
        GET_REG_INFO_BUF(DIP_X_RNR_VC0,  vc0);
        GET_REG_INFO_BUF(DIP_X_RNR_VC1,  vc1);
        GET_REG_INFO_BUF(DIP_X_RNR_GO_B,  go_b);
        GET_REG_INFO_BUF(DIP_X_RNR_GO_G,  go_g);
        GET_REG_INFO_BUF(DIP_X_RNR_GO_R,  go_r);
        GET_REG_INFO_BUF(DIP_X_RNR_GAIN_ISO,  gain_iso);
        GET_REG_INFO_BUF(DIP_X_RNR_RNG,  rng);
        GET_REG_INFO_BUF(DIP_X_RNR_CON2,  con2);
        GET_REG_INFO_BUF(DIP_X_RNR_CON3,  con3);
        GET_REG_INFO_BUF(DIP_X_RNR_SL,  sl);
        GET_REG_INFO_BUF(DIP_X_RNR_SSL_STH,  ssl_sth);
    }
    return MTRUE;
}

MBOOL
ISP_MGR_RNR_T::
apply(const RAWIspCamInfo& rRawIspCamInfo, dip_x_reg_t* pReg)
{

    MBOOL bRNR_EN = isEnable();

    // TOP ==> don't care
    ISP_WRITE_ENABLE_BITS(pReg, DIP_X_CTL_RGB_EN, RNR_EN, bRNR_EN);
    //ISP_MGR_CTL_EN_P2_T::getInstance(m_eSensorDev).setEnable_RNR(bRNR_EN);

    REG_INFO_VALUE(DIP_X_RNR_L) = 0x01188C46;
    REG_INFO_VALUE(DIP_X_RNR_C1) = 0x004C2613;
    REG_INFO_VALUE(DIP_X_RNR_C2) = 0x02241489;
#if 0
    // HW default
    ISP_WRITE_ENABLE_REG(pReg, DIP_X_RNR_L,  0x01188C46);
    ISP_WRITE_ENABLE_REG(pReg, DIP_X_RNR_C1, 0x004C2613);
    ISP_WRITE_ENABLE_REG(pReg, DIP_X_RNR_C2, 0x02241489);
#endif

    MBOOL fgPgnEn = !rRawIspCamInfo.fgRPGEnable;
    MBOOL fgPureRaw = rRawIspCamInfo.i4RawType == NSIspTuning::ERawType_Pure;
    MBOOL fgRNRSrcP2 = fgPureRaw && fgPgnEn;

    ISP_NVRAM_RPG_T rParam;

    if(fgRNRSrcP2){
        GET_REG_INFO_BUF(DIP_X_PGN_SATU_1, satu_1);
        GET_REG_INFO_BUF(DIP_X_PGN_SATU_2, satu_2);
        GET_REG_INFO_BUF(DIP_X_PGN_GAIN_1, gain_1);
        GET_REG_INFO_BUF(DIP_X_PGN_GAIN_2, gain_2);
        GET_REG_INFO_BUF(DIP_X_PGN_OFST_1, ofst_1);
        GET_REG_INFO_BUF(DIP_X_PGN_OFST_2, ofst_2);

        reinterpret_cast<ISP_DIP_X_RNR_GAIN_ISO_T*>(REG_INFO_VALUE_PTR(DIP_X_RNR_GAIN_ISO))->RNR_GAIN_ISO
            = pReg->DIP_X_OBC2_GAIN0.Bits.OBC_GAIN_B;
    }
    else{
        rParam = rRawIspCamInfo.rRPG;

        reinterpret_cast<ISP_DIP_X_RNR_GAIN_ISO_T*>(REG_INFO_VALUE_PTR(DIP_X_RNR_GAIN_ISO))->RNR_GAIN_ISO
            = rRawIspCamInfo.rOBC1.gain0.bits.OBC_GAIN_B;
    }

    //align the diff in bit number (RNR and RPG)
    if( (rParam.ofst_1.bits.RPG_OFST_B >> 11) & 1 ){
        reinterpret_cast<ISP_DIP_X_RNR_GO_B_T*>(REG_INFO_VALUE_PTR(DIP_X_RNR_GO_B))->RNR_OFST_B = (rParam.ofst_1.bits.RPG_OFST_B | 0x7000);
    }
    else{
        reinterpret_cast<ISP_DIP_X_RNR_GO_B_T*>(REG_INFO_VALUE_PTR(DIP_X_RNR_GO_B))->RNR_OFST_B = rParam.ofst_1.bits.RPG_OFST_B;
    }

    if( (rParam.ofst_1.bits.RPG_OFST_GB >> 11) & 1 ){
        reinterpret_cast<ISP_DIP_X_RNR_GO_G_T*>(REG_INFO_VALUE_PTR(DIP_X_RNR_GO_G))->RNR_OFST_G = (rParam.ofst_1.bits.RPG_OFST_GB | 0x7000);
    }
    else{
        reinterpret_cast<ISP_DIP_X_RNR_GO_G_T*>(REG_INFO_VALUE_PTR(DIP_X_RNR_GO_G))->RNR_OFST_G = rParam.ofst_1.bits.RPG_OFST_GB;
    }

    if( (rParam.ofst_2.bits.RPG_OFST_R >> 11) & 1 ){
        reinterpret_cast<ISP_DIP_X_RNR_GO_R_T*>(REG_INFO_VALUE_PTR(DIP_X_RNR_GO_R))->RNR_OFST_R = (rParam.ofst_2.bits.RPG_OFST_R | 0x7000);
    }
    else{
        reinterpret_cast<ISP_DIP_X_RNR_GO_R_T*>(REG_INFO_VALUE_PTR(DIP_X_RNR_GO_R))->RNR_OFST_R = rParam.ofst_2.bits.RPG_OFST_R;
    }

    reinterpret_cast<ISP_DIP_X_RNR_GO_B_T*>(REG_INFO_VALUE_PTR(DIP_X_RNR_GO_B))->RNR_GAIN_B = (rParam.gain_1.bits.RPG_GAIN_B);
    reinterpret_cast<ISP_DIP_X_RNR_GO_G_T*>(REG_INFO_VALUE_PTR(DIP_X_RNR_GO_G))->RNR_GAIN_G = (rParam.gain_1.bits.RPG_GAIN_GB);
    reinterpret_cast<ISP_DIP_X_RNR_GO_R_T*>(REG_INFO_VALUE_PTR(DIP_X_RNR_GO_R))->RNR_GAIN_R = (rParam.gain_2.bits.RPG_GAIN_R);


    // Register setting
    writeRegs(static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum, pReg);

    dumpRegInfo("RNR");

    return  MTRUE;
}


}
