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
#define LOG_TAG "isp_mgr_dcpn"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include "isp_mgr.h"
#include <private/aaa_utils.h>
#include <array>

using namespace NSIspTuningv3;

#define MY_INST_DCPN NS3Av3::INST_T<ISP_MGR_DCPN_T>
static std::array<MY_INST_DCPN, SENSOR_IDX_MAX> gMultitonDCPN;

#define MY_INST_DCPN2 NS3Av3::INST_T<ISP_MGR_DCPN2_T>
static std::array<MY_INST_DCPN2, SENSOR_IDX_MAX> gMultitonDCPN2;

namespace NSIspTuningv3
{
#define CLAMP(x,min,max)       (((x) > (max)) ? (max) : (((x) < (min)) ? (min) : (x)))

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DCPN
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_DCPN_T&
ISP_MGR_DCPN_T::
getInstance(ESensorDev_T const eSensorDev)
{
    auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(eSensorDev);

    MY_ASSERT(i4SensorIdx < SENSOR_IDX_MAX);
    MY_ASSERT(i4SensorIdx >= 0);

    MY_INST_DCPN& rSingleton = gMultitonDCPN[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<ISP_MGR_DCPN_T>(eSensorDev);
    } );

    return *(rSingleton.instance);
}

template <>
ISP_MGR_DCPN_T&
ISP_MGR_DCPN_T::
put(ISP_NVRAM_DCPN_T const& rParam)
{
    PUT_REG_INFO(CAM_DCPN_HDR_EN, hdr_en);
    //PUT_REG_INFO(CAM_DCPN_IN_IMG_SIZE, in_ing_size);
    PUT_REG_INFO(CAM_DCPN_ALGO_PARAM1, algo_param1);
    PUT_REG_INFO(CAM_DCPN_ALGO_PARAM2, algo_param2);
    PUT_REG_INFO(CAM_DCPN_GTM_X0, x0);
    PUT_REG_INFO(CAM_DCPN_GTM_Y0, y0);
    PUT_REG_INFO(CAM_DCPN_GTM_S0, s0);
    PUT_REG_INFO(CAM_DCPN_GTM_S1, s1);
    return  (*this);
}


template <>
ISP_MGR_DCPN_T&
ISP_MGR_DCPN_T::
get(ISP_NVRAM_DCPN_T& rParam)
{
    GET_REG_INFO(CAM_DCPN_HDR_EN, hdr_en);
    //GET_REG_INFO(CAM_DCPN_IN_IMG_SIZE, in_ing_size);
    GET_REG_INFO(CAM_DCPN_ALGO_PARAM1, algo_param1);
    GET_REG_INFO(CAM_DCPN_ALGO_PARAM2, algo_param2);
    GET_REG_INFO(CAM_DCPN_GTM_X0, x0);
    GET_REG_INFO(CAM_DCPN_GTM_Y0, y0);
    GET_REG_INFO(CAM_DCPN_GTM_S0, s0);
    GET_REG_INFO(CAM_DCPN_GTM_S1, s1);
    return  (*this);
}



MBOOL
ISP_MGR_DCPN_T::
apply(RAWIspCamInfo& rRawIspCamInfo, TuningMgr& rTuning, MINT32 i4SubsampleIdex)
{

    MBOOL bDCPN_EN = isEnable() && ISP_MGR_LSC_T::getInstance(m_eSensorDev).isEnable(); //HW constraint

    addressErrorCheck("Before ISP_MGR_RMG_T::apply()");

    //> prepar register value
    //CAM_LOGD_IF(debugDump, "bDCPN_EN(%d) m_bZHdrEnable(%d)", bDCPN_EN, m_bZHdrEnable);
    if(bDCPN_EN)
    {
        MINT32 hdrEnable = m_bZHdrEnable;

        MUINT32 orig = 0;
        orig = REG_INFO_VALUE(CAM_DCPN_HDR_EN);
        REG_INFO_VALUE(CAM_DCPN_HDR_EN) =   (orig & 0x000000F0) //((edgeInfo & 0xF) << 4)
                                            | ((hdrEnable & 0x1))
                                            | 0;
        if(hdrEnable)
        {
            CAM_LOGD_IF(1, "DCPN in: hdrEnable(%d), HdrToneInfo GTM_X0(%d), GTM_Y0(%d), GTM_S0(%d), GTM_S1(%d)", hdrEnable,
            rRawIspCamInfo.rAEInfo.rHdrToneInfo.rHdrDCPNProp.GTM_X0, rRawIspCamInfo.rAEInfo.rHdrToneInfo.rHdrDCPNProp.GTM_Y0,
            rRawIspCamInfo.rAEInfo.rHdrToneInfo.rHdrDCPNProp.GTM_S0, rRawIspCamInfo.rAEInfo.rHdrToneInfo.rHdrDCPNProp.GTM_S1);

            MINT32 gtm_x0 = CLAMP(rRawIspCamInfo.rAEInfo.rHdrToneInfo.rHdrDCPNProp.GTM_X0, 0, 4095);
            MINT32 gtm_y0 = CLAMP(rRawIspCamInfo.rAEInfo.rHdrToneInfo.rHdrDCPNProp.GTM_Y0, 0, 65535);
            MINT32 gtm_s0 = CLAMP(rRawIspCamInfo.rAEInfo.rHdrToneInfo.rHdrDCPNProp.GTM_S0, 16, 1048575);
            MINT32 gtm_s1 = CLAMP(rRawIspCamInfo.rAEInfo.rHdrToneInfo.rHdrDCPNProp.GTM_S1, 16, 1048575);


            REG_INFO_VALUE(CAM_DCPN_GTM_X0) = (gtm_x0 & 0xFFF) | 0;
            REG_INFO_VALUE(CAM_DCPN_GTM_Y0) = (gtm_y0 & 0xFFFF) | 0;
            REG_INFO_VALUE(CAM_DCPN_GTM_S0) = (gtm_s0 & 0xFFFFF) | 0;
            REG_INFO_VALUE(CAM_DCPN_GTM_S1) = (gtm_s1 & 0xFFFFF) | 0;
        }   // if hdrEnable end

    }   // if bRMG_EN end

    rTuning.updateEngine(eTuningMgrFunc_DCPN, bDCPN_EN, 0);

    // TOP
    TUNING_MGR_WRITE_BITS_CAM((&rTuning), CAM_CTL_EN2, DCPN_EN, bDCPN_EN, 0);

    ISP_MGR_CTL_EN_P1_T::getInstance(m_eSensorDev).setEnable_DCPN(bDCPN_EN);

    // Register setting
    rTuning.tuningMgrWriteRegs(
            static_cast<TUNING_MGR_REG_IO_STRUCT*>(m_pRegInfo),
            m_u4RegInfoNum, i4SubsampleIdex);


    dumpRegInfoP1("DCPN");
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DCPN2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_DCPN2_T&
ISP_MGR_DCPN2_T::
getInstance(ESensorDev_T const eSensorDev)
{
    auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(eSensorDev);

    MY_ASSERT(i4SensorIdx < SENSOR_IDX_MAX);
    MY_ASSERT(i4SensorIdx >= 0);


    MY_INST_DCPN2& rSingleton = gMultitonDCPN2[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<ISP_MGR_DCPN2_T>(eSensorDev);
    } );

    return *(rSingleton.instance);
}

template <>
ISP_MGR_DCPN2_T&
ISP_MGR_DCPN2_T::
put(ISP_NVRAM_DCPN_T const& rParam)
{
    PUT_REG_INFO(DIP_X_DCPN_HDR_EN, hdr_en);
    //PUT_REG_INFO(DIP_X_DCPN_IN_IMG_SIZE, in_ing_size);
    PUT_REG_INFO(DIP_X_DCPN_ALGO_PARAM1, algo_param1);
    PUT_REG_INFO(DIP_X_DCPN_ALGO_PARAM2, algo_param2);
    PUT_REG_INFO(DIP_X_DCPN_GTM_X0, x0);
    PUT_REG_INFO(DIP_X_DCPN_GTM_Y0, y0);
    PUT_REG_INFO(DIP_X_DCPN_GTM_S0, s0);
    PUT_REG_INFO(DIP_X_DCPN_GTM_S1, s1);
    return  (*this);
}

template <>
MBOOL
ISP_MGR_DCPN2_T::
get(ISP_NVRAM_DCPN_T& rParam, const dip_x_reg_t* pReg) const
{
    if (pReg)
    {
        GET_REG_INFO_BUF(DIP_X_DCPN_HDR_EN, hdr_en);
        //GET_REG_INFO_BUF(DIP_X_DCPN_IN_IMG_SIZE, in_ing_size);
        GET_REG_INFO_BUF(DIP_X_DCPN_ALGO_PARAM1, algo_param1);
        GET_REG_INFO_BUF(DIP_X_DCPN_ALGO_PARAM2, algo_param2);
        GET_REG_INFO_BUF(DIP_X_DCPN_GTM_X0, x0);
        GET_REG_INFO_BUF(DIP_X_DCPN_GTM_Y0, y0);
        GET_REG_INFO_BUF(DIP_X_DCPN_GTM_S0, s0);
        GET_REG_INFO_BUF(DIP_X_DCPN_GTM_S1, s1);
    }
    return MTRUE;
}

MBOOL
ISP_MGR_DCPN2_T::
apply(const RAWIspCamInfo& rRawIspCamInfo, dip_x_reg_t* pReg)
{
    MBOOL bDCPN2_EN = isEnable();

    addressErrorCheck("Before ISP_MGR_DCPN2_T::apply()");

    //> prepar register value
#if 1
    if(bDCPN2_EN)
    {
        MINT32 hdrEnable = m_bZHdrEnable;

        MUINT32 orig = 0;
        orig = REG_INFO_VALUE(DIP_X_DCPN_HDR_EN);
        REG_INFO_VALUE(DIP_X_DCPN_HDR_EN) =   (orig & 0x000000F0) //((edgeInfo & 0xF) << 4)
                                            | ((hdrEnable & 0x1))
                                            | 0;
        if(hdrEnable)
        {
            CAM_LOGD_IF(debugDump, "DCPN2 in: HdrToneInfo GTM_X0(%d), GTM_Y0(%d), GTM_S0(%d), GTM_S1(%d)",
            rRawIspCamInfo.rAEInfo.rHdrToneInfo.rHdrDCPNProp.GTM_X0, rRawIspCamInfo.rAEInfo.rHdrToneInfo.rHdrDCPNProp.GTM_Y0,
            rRawIspCamInfo.rAEInfo.rHdrToneInfo.rHdrDCPNProp.GTM_S0, rRawIspCamInfo.rAEInfo.rHdrToneInfo.rHdrDCPNProp.GTM_S1);

            MINT32 gtm_x0 = CLAMP(rRawIspCamInfo.rAEInfo.rHdrToneInfo.rHdrDCPNProp.GTM_X0, 0, 4095);
            MINT32 gtm_y0 = CLAMP(rRawIspCamInfo.rAEInfo.rHdrToneInfo.rHdrDCPNProp.GTM_Y0, 0, 65535);
            MINT32 gtm_s0 = CLAMP(rRawIspCamInfo.rAEInfo.rHdrToneInfo.rHdrDCPNProp.GTM_S0, 16, 1048575);
            MINT32 gtm_s1 = CLAMP(rRawIspCamInfo.rAEInfo.rHdrToneInfo.rHdrDCPNProp.GTM_S1, 16, 1048575);


            REG_INFO_VALUE(DIP_X_DCPN_GTM_X0) = (gtm_x0 & 0xFFF) | 0;
            REG_INFO_VALUE(DIP_X_DCPN_GTM_Y0) = (gtm_y0 & 0xFFFF) | 0;
            REG_INFO_VALUE(DIP_X_DCPN_GTM_S0) = (gtm_s0 & 0xFFFFF) | 0;
            REG_INFO_VALUE(DIP_X_DCPN_GTM_S1) = (gtm_s1 & 0xFFFFF) | 0;
        }   // if hdrEnable end

    }   // if bRMG_EN end
#endif

    // Register setting
    writeRegs(static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum, pReg);

    dumpRegInfo("DCPN2");

    return  MTRUE;
}



}
