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
#define LOG_TAG "isp_mgr_obc"

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

#define MY_INST_OBC NS3Av3::INST_T<ISP_MGR_OBC_T>
static std::array<MY_INST_OBC, SENSOR_IDX_MAX> gMultitonOBC;

#define MY_INST_OBC2 NS3Av3::INST_T<ISP_MGR_OBC2_T>
static std::array<MY_INST_OBC2, SENSOR_IDX_MAX> gMultitonOBC2;

namespace NSIspTuningv3
{
#define CLAMP(x,min,max)       (((x) > (max)) ? (max) : (((x) < (min)) ? (min) : (x)))

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  OBC
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_OBC_T&
ISP_MGR_OBC_T::
getInstance(ESensorDev_T const eSensorDev)
{
    auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(eSensorDev);

    MY_ASSERT(i4SensorIdx < SENSOR_IDX_MAX);
    MY_ASSERT(i4SensorIdx >= 0);


    MY_INST_OBC& rSingleton = gMultitonOBC[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<ISP_MGR_OBC_T>(eSensorDev);
    } );

    return *(rSingleton.instance);
}

template <>
ISP_MGR_OBC_T&
ISP_MGR_OBC_T::
put(ISP_NVRAM_OBC_T const& rParam)
{
    m_rRegInput[ERegInfo_CAM_OBC_OFFST0] = rParam.offst0.val;
    m_rRegInput[ERegInfo_CAM_OBC_OFFST1] = rParam.offst1.val;
    m_rRegInput[ERegInfo_CAM_OBC_OFFST2] = rParam.offst2.val;
    m_rRegInput[ERegInfo_CAM_OBC_OFFST3] = rParam.offst3.val;
    m_rRegInput[ERegInfo_CAM_OBC_GAIN0 ] = rParam.gain0.val;
    m_rRegInput[ERegInfo_CAM_OBC_GAIN1 ] = rParam.gain1.val;
    m_rRegInput[ERegInfo_CAM_OBC_GAIN2 ] = rParam.gain2.val;
    m_rRegInput[ERegInfo_CAM_OBC_GAIN3 ] = rParam.gain3.val;

    return  (*this);
}


template <>
ISP_MGR_OBC_T&
ISP_MGR_OBC_T::
get(ISP_NVRAM_OBC_T& rParam)
{
    GET_REG_INFO(CAM_OBC_OFFST0, offst0);
    GET_REG_INFO(CAM_OBC_OFFST1, offst1);
    GET_REG_INFO(CAM_OBC_OFFST2, offst2);
    GET_REG_INFO(CAM_OBC_OFFST3, offst3);
    GET_REG_INFO(CAM_OBC_GAIN0, gain0);
    GET_REG_INFO(CAM_OBC_GAIN1, gain1);
    GET_REG_INFO(CAM_OBC_GAIN2, gain2);
    GET_REG_INFO(CAM_OBC_GAIN3, gain3);

    return  (*this);
}

MINT32
ISP_MGR_OBC_T::
getOBCGain()
{
    // Merge AE gain
    MINT32 OBGAIN0 = (reinterpret_cast<ISP_CAM_OBC_GAIN0_T*>(&m_rRegInput[ERegInfo_CAM_OBC_GAIN0])->OBC_GAIN_B * m_u4IspAEGain + 256) >> 9;
    return CLAMP(OBGAIN0, 0, 8191);
}

MBOOL
ISP_MGR_OBC_T::
apply(RAWIspCamInfo& rRawIspCamInfo, TuningMgr& rTuning, MINT32 i4SubsampleIdex)
{
    MBOOL bOBC_EN = isEnable();

	MBOOL bUpdate = (rRawIspCamInfo.u4Id == 1)||( bOBC_EN &&(!rRawIspCamInfo.rAEInfo.bDisableOBC) );

    // Merge AE gain
    MINT32 OBGAIN0 = (reinterpret_cast<ISP_CAM_OBC_GAIN0_T*>(&m_rRegInput[ERegInfo_CAM_OBC_GAIN0])->OBC_GAIN_B * m_u4IspAEGain + 256) >> 9;
    MINT32 OBGAIN1 = (reinterpret_cast<ISP_CAM_OBC_GAIN1_T*>(&m_rRegInput[ERegInfo_CAM_OBC_GAIN1])->OBC_GAIN_GR * m_u4IspAEGain + 256) >> 9;
    MINT32 OBGAIN2 = (reinterpret_cast<ISP_CAM_OBC_GAIN2_T*>(&m_rRegInput[ERegInfo_CAM_OBC_GAIN2])->OBC_GAIN_GB * m_u4IspAEGain + 256) >> 9;
    MINT32 OBGAIN3 = (reinterpret_cast<ISP_CAM_OBC_GAIN3_T*>(&m_rRegInput[ERegInfo_CAM_OBC_GAIN3])->OBC_GAIN_R * m_u4IspAEGain + 256) >> 9;

    reinterpret_cast<ISP_CAM_OBC_GAIN0_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN0))->OBC_GAIN_B = CLAMP(OBGAIN0, 0, 8191);
    reinterpret_cast<ISP_CAM_OBC_GAIN1_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN1))->OBC_GAIN_GR = CLAMP(OBGAIN1, 0, 8191);
    reinterpret_cast<ISP_CAM_OBC_GAIN2_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN2))->OBC_GAIN_GB = CLAMP(OBGAIN2, 0, 8191);
    reinterpret_cast<ISP_CAM_OBC_GAIN3_T*>(REG_INFO_VALUE_PTR(CAM_OBC_GAIN3))->OBC_GAIN_R = CLAMP(OBGAIN3, 0, 8191);

    REG_INFO_VALUE(CAM_OBC_OFFST0) = m_rRegInput[ERegInfo_CAM_OBC_OFFST0];
    REG_INFO_VALUE(CAM_OBC_OFFST1) = m_rRegInput[ERegInfo_CAM_OBC_OFFST1];
    REG_INFO_VALUE(CAM_OBC_OFFST2) = m_rRegInput[ERegInfo_CAM_OBC_OFFST2];
    REG_INFO_VALUE(CAM_OBC_OFFST3) = m_rRegInput[ERegInfo_CAM_OBC_OFFST3];



    // Top Control
    rTuning.enableEngine(eTuningMgrFunc_OBC, bOBC_EN, i4SubsampleIdex);

    // Update
    rTuning.tuningEngine(eTuningMgrFunc_OBC,bUpdate, i4SubsampleIdex);
    //rTuning.tuningEngine(eTuningMgrFunc_OBC,1, i4SubsampleIdex);

//    rRawIspCamInfo.rOBC1.offst0.val = REG_INFO_VALUE(CAM_OBC_OFFST0);
//    rRawIspCamInfo.rOBC1.offst1.val = REG_INFO_VALUE(CAM_OBC_OFFST1);
//    rRawIspCamInfo.rOBC1.offst2.val = REG_INFO_VALUE(CAM_OBC_OFFST2);
//    rRawIspCamInfo.rOBC1.offst3.val = REG_INFO_VALUE(CAM_OBC_OFFST3);
//    rRawIspCamInfo.rOBC1.gain0.val  = REG_INFO_VALUE(CAM_OBC_GAIN0);
//    rRawIspCamInfo.rOBC1.gain1.val  = REG_INFO_VALUE(CAM_OBC_GAIN1);
//    rRawIspCamInfo.rOBC1.gain2.val  = REG_INFO_VALUE(CAM_OBC_GAIN2);
//    rRawIspCamInfo.rOBC1.gain3.val  = REG_INFO_VALUE(CAM_OBC_GAIN3);

    //get member value to rRawIspCamInfo.rOBC1
    get(rRawIspCamInfo.rOBC1);



#if 0
    rTuning.updateEngine(eTuningMgrFunc_OBC, bOBC_EN, i4SubsampleIdex);

        // TOP
    TUNING_MGR_WRITE_BITS_CAM((&rTuning), CAM_CTL_EN, OBC_EN, bOBC_EN, i4SubsampleIdex);
#endif

    ISP_MGR_CTL_EN_P1_T::getInstance(m_eSensorDev).setEnable_OB(bOBC_EN);

        // Register setting
    if (bUpdate)
    rTuning.tuningMgrWriteRegs(
        static_cast<TUNING_MGR_REG_IO_STRUCT*>(m_pRegInfo),
        m_u4RegInfoNum, i4SubsampleIdex);

    dumpRegInfoP1("OBC");
    return  MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  OBC2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_OBC2_T&
ISP_MGR_OBC2_T::
getInstance(ESensorDev_T const eSensorDev)
{
    auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(eSensorDev);

    MY_ASSERT(i4SensorIdx < SENSOR_IDX_MAX);
    MY_ASSERT(i4SensorIdx >= 0);


    MY_INST_OBC2& rSingleton = gMultitonOBC2[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<ISP_MGR_OBC2_T>(eSensorDev);
    } );

    return *(rSingleton.instance);
}

template <>
ISP_MGR_OBC2_T&
ISP_MGR_OBC2_T::
put(ISP_NVRAM_OBC_T const& rParam)
{
    m_rRegInput[ERegInfo_DIP_X_OBC2_OFFST0] = rParam.offst0.val;
    m_rRegInput[ERegInfo_DIP_X_OBC2_OFFST1] = rParam.offst1.val;
    m_rRegInput[ERegInfo_DIP_X_OBC2_OFFST2] = rParam.offst2.val;
    m_rRegInput[ERegInfo_DIP_X_OBC2_OFFST3] = rParam.offst3.val;
    m_rRegInput[ERegInfo_DIP_X_OBC2_GAIN0 ] = rParam.gain0.val;
    m_rRegInput[ERegInfo_DIP_X_OBC2_GAIN1 ] = rParam.gain1.val;
    m_rRegInput[ERegInfo_DIP_X_OBC2_GAIN2 ] = rParam.gain2.val;
    m_rRegInput[ERegInfo_DIP_X_OBC2_GAIN3 ] = rParam.gain3.val;

    return  (*this);
}

template <>
MBOOL
ISP_MGR_OBC2_T::
get(ISP_NVRAM_OBC_T & rParam, const dip_x_reg_t* pReg) const
{
    if (pReg)
    {
        GET_REG_INFO_BUF(DIP_X_OBC2_OFFST0, offst0);
        GET_REG_INFO_BUF(DIP_X_OBC2_OFFST1, offst1);
        GET_REG_INFO_BUF(DIP_X_OBC2_OFFST2, offst2);
        GET_REG_INFO_BUF(DIP_X_OBC2_OFFST3, offst3);
        GET_REG_INFO_BUF(DIP_X_OBC2_GAIN0, gain0);
        GET_REG_INFO_BUF(DIP_X_OBC2_GAIN1, gain1);
        GET_REG_INFO_BUF(DIP_X_OBC2_GAIN2, gain2);
        GET_REG_INFO_BUF(DIP_X_OBC2_GAIN3, gain3);
    }
    return MTRUE;
}

MBOOL
ISP_MGR_OBC2_T::
apply(const RAWIspCamInfo& /*rRawIspCamInfo*/, dip_x_reg_t* pReg)
{
    MBOOL bOBC_EN = isEnable();

    // Merge AE gain
    MINT32 OBGAIN0 = (reinterpret_cast<ISP_CAM_OBC_GAIN0_T*>(&m_rRegInput[ERegInfo_DIP_X_OBC2_GAIN0])->OBC_GAIN_B * m_u4IspAEGain + 256) >> 9;
    MINT32 OBGAIN1 = (reinterpret_cast<ISP_CAM_OBC_GAIN1_T*>(&m_rRegInput[ERegInfo_DIP_X_OBC2_GAIN1])->OBC_GAIN_GR * m_u4IspAEGain + 256) >> 9;
    MINT32 OBGAIN2 = (reinterpret_cast<ISP_CAM_OBC_GAIN2_T*>(&m_rRegInput[ERegInfo_DIP_X_OBC2_GAIN2])->OBC_GAIN_GB * m_u4IspAEGain + 256) >> 9;
    MINT32 OBGAIN3 = (reinterpret_cast<ISP_CAM_OBC_GAIN3_T*>(&m_rRegInput[ERegInfo_DIP_X_OBC2_GAIN3])->OBC_GAIN_R * m_u4IspAEGain + 256) >> 9;

    reinterpret_cast<ISP_CAM_OBC_GAIN0_T*>(REG_INFO_VALUE_PTR(DIP_X_OBC2_GAIN0))->OBC_GAIN_B = CLAMP(OBGAIN0, 0, 8191);
    reinterpret_cast<ISP_CAM_OBC_GAIN1_T*>(REG_INFO_VALUE_PTR(DIP_X_OBC2_GAIN1))->OBC_GAIN_GR = CLAMP(OBGAIN1, 0, 8191);
    reinterpret_cast<ISP_CAM_OBC_GAIN2_T*>(REG_INFO_VALUE_PTR(DIP_X_OBC2_GAIN2))->OBC_GAIN_GB = CLAMP(OBGAIN2, 0, 8191);
    reinterpret_cast<ISP_CAM_OBC_GAIN3_T*>(REG_INFO_VALUE_PTR(DIP_X_OBC2_GAIN3))->OBC_GAIN_R = CLAMP(OBGAIN3, 0, 8191);
    REG_INFO_VALUE(DIP_X_OBC2_OFFST0) = m_rRegInput[ERegInfo_DIP_X_OBC2_OFFST0];
    REG_INFO_VALUE(DIP_X_OBC2_OFFST1) = m_rRegInput[ERegInfo_DIP_X_OBC2_OFFST1];
    REG_INFO_VALUE(DIP_X_OBC2_OFFST2) = m_rRegInput[ERegInfo_DIP_X_OBC2_OFFST2];
    REG_INFO_VALUE(DIP_X_OBC2_OFFST3) = m_rRegInput[ERegInfo_DIP_X_OBC2_OFFST3];

    // TOP
    ISP_WRITE_ENABLE_BITS(pReg, DIP_X_CTL_RGB_EN, OBC2_EN, bOBC_EN);
    //ISP_MGR_CTL_EN_P2_T::getInstance(m_eSensorDev).setEnable_OBC2(bOBC_EN);

    // Register setting
    writeRegs(static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum, pReg);

    dumpRegInfo("OBC2");
    return  MTRUE;
}

}
