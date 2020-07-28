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
#define LOG_TAG "isp_mgr_flc"

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

#define MY_INST NS3Av3::INST_T<ISP_MGR_FLC_T>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

namespace NSIspTuningv3
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  FLC
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_FLC_T&
ISP_MGR_FLC_T::
getInstance(MUINT32 const eSensorDev)
{
    auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(eSensorDev);

    MY_ASSERT(i4SensorIdx < SENSOR_IDX_MAX);
    MY_ASSERT(i4SensorIdx >= 0);

    MY_INST& rSingleton = gMultiton[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<ISP_MGR_FLC_T>(eSensorDev);
    } );

    return *(rSingleton.instance);
}

template <>
ISP_MGR_FLC_T&
ISP_MGR_FLC_T::
put(ISP_NVRAM_FLC_T const& rParam)
{
    PUT_REG_INFO(DIP_X_FLC_OFFST0,   offst0);
    PUT_REG_INFO(DIP_X_FLC_OFFST1,   offst1);
    PUT_REG_INFO(DIP_X_FLC_OFFST2,   offst2);
    PUT_REG_INFO(DIP_X_FLC_GAIN0,    gain0);
    PUT_REG_INFO(DIP_X_FLC_GAIN1,    gain0);
    PUT_REG_INFO(DIP_X_FLC_GAIN2,    gain0);

    return  (*this);
}

template <>
MBOOL
ISP_MGR_FLC_T::
get(ISP_NVRAM_FLC_T& rParam, const dip_x_reg_t* pReg) const
{
    if (pReg)
    {
        GET_REG_INFO(DIP_X_FLC_OFFST0,   offst0);
        GET_REG_INFO(DIP_X_FLC_OFFST1,   offst1);
        GET_REG_INFO(DIP_X_FLC_OFFST2,   offst2);
        GET_REG_INFO(DIP_X_FLC_GAIN0,    gain0);
        GET_REG_INFO(DIP_X_FLC_GAIN1,    gain0);
        GET_REG_INFO(DIP_X_FLC_GAIN2,    gain0);

    }
    return MTRUE;
}

MBOOL
ISP_MGR_FLC_T::
apply(const RAWIspCamInfo& /*rRawIspCamInfo*/, dip_x_reg_t* pReg)
{
    // ISP pregain
    m_i4FlareGain = (AWB_SCALE_UNIT * m_i4FlareGain + (AWB_SCALE_UNIT >> 1)) / AWB_SCALE_UNIT;
    CAM_LOGD_IF(IsDebugEnabled(), "m_i4FlareGain = %d\n", m_i4FlareGain);
    reinterpret_cast<ISP_DIP_X_FLC_GAIN0_T*>(REG_INFO_VALUE_PTR(DIP_X_FLC_GAIN0))->FLC_GAIN_B = m_i4FlareGain;
    reinterpret_cast<ISP_DIP_X_FLC_GAIN1_T*>(REG_INFO_VALUE_PTR(DIP_X_FLC_GAIN1))->FLC_GAIN_G = m_i4FlareGain;
    reinterpret_cast<ISP_DIP_X_FLC_GAIN2_T*>(REG_INFO_VALUE_PTR(DIP_X_FLC_GAIN2))->FLC_GAIN_R = m_i4FlareGain;

    // ISP flare offset
    m_i4IspFlareOffset = (m_i4FlareOffset * m_i4FlareGain + (AWB_SCALE_UNIT >> 1)) / AWB_SCALE_UNIT;
    CAM_LOGD_IF(IsDebugEnabled(), "m_i4IspFlareOffset = %d; m_i4FlareOffset = %d\n", m_i4IspFlareOffset , m_i4FlareOffset);
    MUINT32 OFFS = (m_i4IspFlareOffset >= 0) ? static_cast<MUINT32>(m_i4IspFlareOffset) : static_cast<MUINT32>(16384 + m_i4IspFlareOffset);

    reinterpret_cast<ISP_DIP_X_FLC_OFFST0_T*>(REG_INFO_VALUE_PTR(DIP_X_FLC_OFFST0))->FLC_OFST_B = OFFS;
    reinterpret_cast<ISP_DIP_X_FLC_OFFST1_T*>(REG_INFO_VALUE_PTR(DIP_X_FLC_OFFST1))->FLC_OFST_G = OFFS;
    reinterpret_cast<ISP_DIP_X_FLC_OFFST2_T*>(REG_INFO_VALUE_PTR(DIP_X_FLC_OFFST2))->FLC_OFST_R = OFFS;

    MBOOL bFLC_EN = isEnable();

    // TOP
    ISP_WRITE_ENABLE_BITS(pReg, DIP_X_CTL_RGB_EN, FLC_EN, bFLC_EN);
    //ISP_MGR_CTL_EN_P2_T::getInstance(m_eSensorDev).setEnable_DBS2(bDBS_EN);

    // Register setting
    writeRegs(static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum, pReg);

    dumpRegInfo("FLC");

    return  MTRUE;
}

}
