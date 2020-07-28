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
#define LOG_TAG "isp_mgr_hfg"

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

#define MY_INST NS3Av3::INST_T<ISP_MGR_HFG_T>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

namespace NSIspTuningv3
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// HFG
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_HFG_T&
ISP_MGR_HFG_T::
getInstance(MUINT32 const eSensorDev)
{
    auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(eSensorDev);

    MY_ASSERT(i4SensorIdx < SENSOR_IDX_MAX);
    MY_ASSERT(i4SensorIdx >= 0);

    MY_INST& rSingleton = gMultiton[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<ISP_MGR_HFG_T>(eSensorDev);
    } );

    return *(rSingleton.instance);
}

template <>
ISP_MGR_HFG_T&
ISP_MGR_HFG_T::
put(ISP_NVRAM_HFG_T const& rParam)
{
    PUT_REG_INFO(DIP_X_HFG_CON_0,   con_0);
    PUT_REG_INFO(DIP_X_HFG_LUMA_0,  luma_0);
    PUT_REG_INFO(DIP_X_HFG_LUMA_1,  luma_1);
    PUT_REG_INFO(DIP_X_HFG_LUMA_2,  luma_2);
    PUT_REG_INFO(DIP_X_HFG_LCE_0,   lce_0);
    PUT_REG_INFO(DIP_X_HFG_LCE_1,   lce_1);
    PUT_REG_INFO(DIP_X_HFG_LCE_2,   lce_2);

    return  (*this);
}

template <>
MBOOL
ISP_MGR_HFG_T::
get(ISP_NVRAM_HFG_GET_T& rParam, const dip_x_reg_t* pReg) const
{
    if (pReg)
    {
        GET_REG_INFO_BUF(DIP_X_HFG_CON_0,   con_0);
        GET_REG_INFO_BUF(DIP_X_HFG_LUMA_0,  luma_0);
        GET_REG_INFO_BUF(DIP_X_HFG_LUMA_1,  luma_1);
        GET_REG_INFO_BUF(DIP_X_HFG_LUMA_2,  luma_2);
        GET_REG_INFO_BUF(DIP_X_HFG_LCE_0,   lce_0);
        GET_REG_INFO_BUF(DIP_X_HFG_LCE_1,   lce_1);
        GET_REG_INFO_BUF(DIP_X_HFG_LCE_2,   lce_2);
        GET_REG_INFO_BUF(DIP_X_HFG_RAN_0,   ran_0);
        GET_REG_INFO_BUF(DIP_X_HFG_RAN_1,   ran_1);
        GET_REG_INFO_BUF(DIP_X_HFG_RAN_3,   ran_3);
    }
    return MTRUE;
}

MBOOL
ISP_MGR_HFG_T::
apply(const CROP_RZ_INFO_T& rRzCrop, EIspProfile_T /*eIspProfile*/, dip_x_reg_t* pReg)
{

    reinterpret_cast<ISP_DIP_X_HFG_RAN_0_T*>(REG_INFO_VALUE_PTR(DIP_X_HFG_RAN_0))->RANSED_GSEED = (MINT16)::systemTime();
    reinterpret_cast<ISP_DIP_X_HFG_RAN_0_T*>(REG_INFO_VALUE_PTR(DIP_X_HFG_RAN_0))->RANSED_SEED0 = 0xD900;
    reinterpret_cast<ISP_DIP_X_HFG_RAN_1_T*>(REG_INFO_VALUE_PTR(DIP_X_HFG_RAN_1))->RANSED_SEED1 = 0xDE49;
    reinterpret_cast<ISP_DIP_X_HFG_RAN_1_T*>(REG_INFO_VALUE_PTR(DIP_X_HFG_RAN_1))->RANSED_SEED2 = 0xB46D;
    reinterpret_cast<ISP_DIP_X_HFG_RAN_3_T*>(REG_INFO_VALUE_PTR(DIP_X_HFG_RAN_3))->RANSED_IMG_WD = rRzCrop.i4RzWidth;

    MUINT32 padding_16_w = (16 - (rRzCrop.i4RzWidth % 16)) % 16;
    MUINT32 padding_16_h = (16 - (rRzCrop.i4RzHeight % 16)) % 16;
    reinterpret_cast<ISP_DIP_X_HFG_RAN_4_T*>(REG_INFO_VALUE_PTR(DIP_X_HFG_RAN_4))->HFG_T_WIDTH = (rRzCrop.i4RzWidth + padding_16_w);
    reinterpret_cast<ISP_DIP_X_HFG_RAN_4_T*>(REG_INFO_VALUE_PTR(DIP_X_HFG_RAN_4))->HFG_T_HEIGHT = (rRzCrop.i4RzHeight + padding_16_h);
    reinterpret_cast<ISP_DIP_X_HFG_CROP_X_T*>(REG_INFO_VALUE_PTR(DIP_X_HFG_CROP_X))->HFG_CROP_STR_X = 0;
    reinterpret_cast<ISP_DIP_X_HFG_CROP_X_T*>(REG_INFO_VALUE_PTR(DIP_X_HFG_CROP_X))->HFG_CROP_END_X = (rRzCrop.i4RzWidth - 1);
    reinterpret_cast<ISP_DIP_X_HFG_CROP_Y_T*>(REG_INFO_VALUE_PTR(DIP_X_HFG_CROP_Y))->HFG_CROP_STR_Y = 0;
    reinterpret_cast<ISP_DIP_X_HFG_CROP_Y_T*>(REG_INFO_VALUE_PTR(DIP_X_HFG_CROP_Y))->HFG_CROP_END_Y = (rRzCrop.i4RzHeight - 1);

    MBOOL bHFG_EN = isEnable();

    // TOP ==> don't care
    ISP_WRITE_ENABLE_BITS(pReg, DIP_X_CTL_YUV2_EN, HFG_EN, bHFG_EN);
    //ISP_MGR_CTL_EN_P2_T::getInstance(m_eSensorDev).setEnable_HFG(bHFG_EN);

    // Register setting
    writeRegs(static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum, pReg);

    dumpRegInfo("HFG");

    return  MTRUE;
}


}
