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
#define LOG_TAG "isp_mgr_ccm"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include "isp_mgr.h"
//#include <ccm_mgr.h>
#include <private/aaa_utils.h>
#include <array>

using namespace NSIspTuningv3;

#define MY_INST NS3Av3::INST_T<ISP_MGR_CCM_T>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

namespace NSIspTuningv3
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CCM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_CCM_T&
ISP_MGR_CCM_T::
getInstance(ESensorDev_T const eSensorDev)
{
    auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(eSensorDev);

    MY_ASSERT(i4SensorIdx < SENSOR_IDX_MAX);
    MY_ASSERT(i4SensorIdx >= 0);

    MY_INST& rSingleton = gMultiton[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<ISP_MGR_CCM_T>(eSensorDev);
    } );

    return *(rSingleton.instance);
}

template <>
ISP_MGR_CCM_T&
ISP_MGR_CCM_T::
put(ISP_NVRAM_CCM_T const& rParam)
{
    PUT_REG_INFO(DIP_X_G2G_CNV_1, cnv_1);
    PUT_REG_INFO(DIP_X_G2G_CNV_2, cnv_2);
    PUT_REG_INFO(DIP_X_G2G_CNV_3, cnv_3);
    PUT_REG_INFO(DIP_X_G2G_CNV_4, cnv_4);
    PUT_REG_INFO(DIP_X_G2G_CNV_5, cnv_5);
    PUT_REG_INFO(DIP_X_G2G_CNV_6, cnv_6);

    return  (*this);
}

template <>
ISP_MGR_CCM_T&
ISP_MGR_CCM_T::
get(ISP_NVRAM_CCM_T& rParam)
{
    GET_REG_INFO(DIP_X_G2G_CNV_1, cnv_1);
    GET_REG_INFO(DIP_X_G2G_CNV_2, cnv_2);
    GET_REG_INFO(DIP_X_G2G_CNV_3, cnv_3);
    GET_REG_INFO(DIP_X_G2G_CNV_4, cnv_4);
    GET_REG_INFO(DIP_X_G2G_CNV_5, cnv_5);
    GET_REG_INFO(DIP_X_G2G_CNV_6, cnv_6);

    return  (*this);
}

template <>
MBOOL
ISP_MGR_CCM_T::
get(ISP_NVRAM_CCM_T& rParam, const dip_x_reg_t* pReg) const
{
    if (pReg)
    {
        GET_REG_INFO_BUF(DIP_X_G2G_CNV_1, cnv_1);
        GET_REG_INFO_BUF(DIP_X_G2G_CNV_2, cnv_2);
        GET_REG_INFO_BUF(DIP_X_G2G_CNV_3, cnv_3);
        GET_REG_INFO_BUF(DIP_X_G2G_CNV_4, cnv_4);
        GET_REG_INFO_BUF(DIP_X_G2G_CNV_5, cnv_5);
        GET_REG_INFO_BUF(DIP_X_G2G_CNV_6, cnv_6);
    }
    return MTRUE;
}

#if 0
template <>
ISP_MGR_CCM_T&
ISP_MGR_CCM_T::
put(ISP_NVRAM_CCM_CTL_T const& rParam)
{
    PUT_REG_INFO(DIP_X_G2G_CFC, cfc);

    return  (*this);
}

template <>
ISP_MGR_CCM_T&
ISP_MGR_CCM_T::
get(ISP_NVRAM_CCM_CTL_T& rParam)
{
    GET_REG_INFO(DIP_X_G2G_CFC, cfc);

    return  (*this);
}

template <>
MBOOL
ISP_MGR_CCM_T::
get(ISP_NVRAM_CCM_CTL_T& rParam, const dip_x_reg_t* pReg) const
{
    if (pReg)
    {
        GET_REG_INFO_BUF(DIP_X_G2G_CFC, cfc);
    }
    return MTRUE;
}
#endif

MBOOL
ISP_MGR_CCM_T::
apply(const RAWIspCamInfo& rRawIspCamInfo, dip_x_reg_t* pReg)
{
    MBOOL fgOnOff = isEnable();
    if (!fgOnOff) { // Reset to unit matrix
        reinterpret_cast<ISP_DIP_X_G2G_CNV_1_T*>(REG_INFO_VALUE_PTR(DIP_X_G2G_CNV_1))->G2G_CNV_00 = 512;
        reinterpret_cast<ISP_DIP_X_G2G_CNV_1_T*>(REG_INFO_VALUE_PTR(DIP_X_G2G_CNV_1))->G2G_CNV_01 = 0;
        reinterpret_cast<ISP_DIP_X_G2G_CNV_2_T*>(REG_INFO_VALUE_PTR(DIP_X_G2G_CNV_2))->G2G_CNV_02 = 0;
        reinterpret_cast<ISP_DIP_X_G2G_CNV_3_T*>(REG_INFO_VALUE_PTR(DIP_X_G2G_CNV_3))->G2G_CNV_10 = 0;
        reinterpret_cast<ISP_DIP_X_G2G_CNV_3_T*>(REG_INFO_VALUE_PTR(DIP_X_G2G_CNV_3))->G2G_CNV_11 = 512;
        reinterpret_cast<ISP_DIP_X_G2G_CNV_4_T*>(REG_INFO_VALUE_PTR(DIP_X_G2G_CNV_4))->G2G_CNV_12 = 0;
        reinterpret_cast<ISP_DIP_X_G2G_CNV_5_T*>(REG_INFO_VALUE_PTR(DIP_X_G2G_CNV_5))->G2G_CNV_20 = 0;
        reinterpret_cast<ISP_DIP_X_G2G_CNV_5_T*>(REG_INFO_VALUE_PTR(DIP_X_G2G_CNV_5))->G2G_CNV_21 = 0;
        reinterpret_cast<ISP_DIP_X_G2G_CNV_6_T*>(REG_INFO_VALUE_PTR(DIP_X_G2G_CNV_6))->G2G_CNV_22 = 512;
    }

    if (m_bMono ||
        rRawIspCamInfo.eIspProfile == EIspProfile_N3D_Denoise_toGGM) { // mono setting
        reinterpret_cast<ISP_DIP_X_G2G_CNV_1_T*>(REG_INFO_VALUE_PTR(DIP_X_G2G_CNV_1))->G2G_CNV_00 = 0;
        reinterpret_cast<ISP_DIP_X_G2G_CNV_1_T*>(REG_INFO_VALUE_PTR(DIP_X_G2G_CNV_1))->G2G_CNV_01 = 512;
        reinterpret_cast<ISP_DIP_X_G2G_CNV_2_T*>(REG_INFO_VALUE_PTR(DIP_X_G2G_CNV_2))->G2G_CNV_02 = 0;
        reinterpret_cast<ISP_DIP_X_G2G_CNV_3_T*>(REG_INFO_VALUE_PTR(DIP_X_G2G_CNV_3))->G2G_CNV_10 = 0;
        reinterpret_cast<ISP_DIP_X_G2G_CNV_3_T*>(REG_INFO_VALUE_PTR(DIP_X_G2G_CNV_3))->G2G_CNV_11 = 512;
        reinterpret_cast<ISP_DIP_X_G2G_CNV_4_T*>(REG_INFO_VALUE_PTR(DIP_X_G2G_CNV_4))->G2G_CNV_12 = 0;
        reinterpret_cast<ISP_DIP_X_G2G_CNV_5_T*>(REG_INFO_VALUE_PTR(DIP_X_G2G_CNV_5))->G2G_CNV_20 = 0;
        reinterpret_cast<ISP_DIP_X_G2G_CNV_5_T*>(REG_INFO_VALUE_PTR(DIP_X_G2G_CNV_5))->G2G_CNV_21 = 512;
        reinterpret_cast<ISP_DIP_X_G2G_CNV_6_T*>(REG_INFO_VALUE_PTR(DIP_X_G2G_CNV_6))->G2G_CNV_22 = 0;
    }


    // TOP
    ISP_WRITE_ENABLE_BITS(pReg, DIP_X_CTL_RGB_EN, G2G_EN, fgOnOff);
    //ISP_MGR_CTL_EN_P2_T::getInstance(m_eSensorDev).setEnable_G2G(fgOnOff);
/*
    if(isCFCEnable()){
        // CAM_G2G_CTRL
        reinterpret_cast<ISP_DIP_X_G2G_CTRL_T*>(REG_INFO_VALUE_PTR(DIP_X_G2G_CTRL))->G2G_CFC_EN = 1;
        reinterpret_cast<ISP_DIP_X_G2G_CTRL_T*>(REG_INFO_VALUE_PTR(DIP_X_G2G_CTRL))->G2G_H = 4063;
        reinterpret_cast<ISP_DIP_X_G2G_CTRL_T*>(REG_INFO_VALUE_PTR(DIP_X_G2G_CTRL))->G2G_L = 32;
    }
    else{
        reinterpret_cast<ISP_DIP_X_G2G_CTRL_T*>(REG_INFO_VALUE_PTR(DIP_X_G2G_CTRL))->G2G_CFC_EN = 0;
    }
*/
    reinterpret_cast<ISP_DIP_X_G2G_CTRL_T*>(REG_INFO_VALUE_PTR(DIP_X_G2G_CTRL))->G2G_ACC = 9; // Q1.3.9

    if((!m_bMono) &&
        (rRawIspCamInfo.eIspProfile == EIspProfile_N3D_Capture_toW  ||
          rRawIspCamInfo.eIspProfile == EIspProfile_N3D_Denoise_toW ||
          rRawIspCamInfo.eIspProfile == EIspProfile_N3D_Preview_toW ||
          rRawIspCamInfo.eIspProfile == EIspProfile_N3D_Video_toW)){
          reinterpret_cast<ISP_DIP_X_G2G_CTRL_T*>(REG_INFO_VALUE_PTR(DIP_X_G2G_CTRL))->G2G_ACC = 11;
    }

        reinterpret_cast<ISP_DIP_X_G2G_CTRL_T*>(REG_INFO_VALUE_PTR(DIP_X_G2G_CTRL))->G2G_CFC_EN = 1;
    reinterpret_cast<ISP_DIP_X_G2G_CTRL_T*>(REG_INFO_VALUE_PTR(DIP_X_G2G_CTRL))->G2G_H = 4063;
        reinterpret_cast<ISP_DIP_X_G2G_CTRL_T*>(REG_INFO_VALUE_PTR(DIP_X_G2G_CTRL))->G2G_L = 32;
    *reinterpret_cast<MUINT32*>(REG_INFO_VALUE_PTR(DIP_X_G2G_CFC))=0x1CE729CE;

    {//set ISO to MDP
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("vendor.isp.iso.ctrl", value, "-1"); // -1: default, 0: unit matrix
        MINT32 ctrl = atoi(value);

        if (ctrl != -1) {
            *reinterpret_cast<MUINT32*>(REG_INFO_VALUE_PTR(DIP_X_ISO_ADAP)) = ctrl;
        }
        else {
            *reinterpret_cast<MUINT32*>(REG_INFO_VALUE_PTR(DIP_X_ISO_ADAP)) = m_u4ISO;
        }
    }

    writeRegs(static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum, pReg);

    dumpRegInfo("G2G");

    return  MTRUE;
}

}
