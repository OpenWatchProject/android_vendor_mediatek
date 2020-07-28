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
#define LOG_TAG "isp_mgr_udm"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (0)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include "isp_mgr.h"

namespace NSIspTuningv3
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// UDM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_UDM_T&
ISP_MGR_UDM_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
{
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_UDM_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_UDM_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_UDM_DEV<ESensorDev_Sub>::getInstance();
    default:
        CAM_LOGE("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_UDM_DEV<ESensorDev_Main>::getInstance();
    }
}

template <>
ISP_MGR_UDM_T&
ISP_MGR_UDM_T::
put(ISP_NVRAM_UDM_T const& rParam)
{
    PUT_REG_INFO(DIP_X_UDM_INTP_CRS, intp_crs);
    PUT_REG_INFO(DIP_X_UDM_INTP_NAT, intp_nat);
    PUT_REG_INFO(DIP_X_UDM_INTP_AUG, intp_aug);
    PUT_REG_INFO(DIP_X_UDM_LUMA_LUT1, luma_lut1);
    PUT_REG_INFO(DIP_X_UDM_LUMA_LUT2, luma_lut2);
    PUT_REG_INFO(DIP_X_UDM_SL_CTL, sl_ctl);
    PUT_REG_INFO(DIP_X_UDM_HFTD_CTL, hftd_ctl);
    PUT_REG_INFO(DIP_X_UDM_NR_STR, nr_str);
    PUT_REG_INFO(DIP_X_UDM_NR_ACT, nr_act);
    PUT_REG_INFO(DIP_X_UDM_HF_STR, hf_str);
    PUT_REG_INFO(DIP_X_UDM_HF_ACT1, hf_act1);
    PUT_REG_INFO(DIP_X_UDM_HF_ACT2, hf_act2);
    PUT_REG_INFO(DIP_X_UDM_CLIP, clip);
    PUT_REG_INFO(DIP_X_UDM_DSB, dsb);
    PUT_REG_INFO(DIP_X_UDM_LR_RAT, lr_rat);
    return  (*this);
}

template <>
MBOOL
ISP_MGR_UDM_T::
get(ISP_NVRAM_UDM_T& rParam, const dip_x_reg_t* pReg) const
{
    if (pReg)
    {
        GET_REG_INFO_BUF(DIP_X_UDM_INTP_CRS, intp_crs);
        GET_REG_INFO_BUF(DIP_X_UDM_INTP_NAT, intp_nat);
        GET_REG_INFO_BUF(DIP_X_UDM_INTP_AUG, intp_aug);
        GET_REG_INFO_BUF(DIP_X_UDM_LUMA_LUT1, luma_lut1);
        GET_REG_INFO_BUF(DIP_X_UDM_LUMA_LUT2, luma_lut2);
        GET_REG_INFO_BUF(DIP_X_UDM_SL_CTL, sl_ctl);
        GET_REG_INFO_BUF(DIP_X_UDM_HFTD_CTL, hftd_ctl);
        GET_REG_INFO_BUF(DIP_X_UDM_NR_STR, nr_str);
        GET_REG_INFO_BUF(DIP_X_UDM_NR_ACT, nr_act);
        GET_REG_INFO_BUF(DIP_X_UDM_HF_STR, hf_str);
        GET_REG_INFO_BUF(DIP_X_UDM_HF_ACT1, hf_act1);
        GET_REG_INFO_BUF(DIP_X_UDM_HF_ACT2, hf_act2);
        GET_REG_INFO_BUF(DIP_X_UDM_CLIP, clip);
        GET_REG_INFO_BUF(DIP_X_UDM_DSB, dsb);
        GET_REG_INFO_BUF(DIP_X_UDM_LR_RAT, lr_rat);
    }
    return MTRUE;
}

MBOOL
ISP_MGR_UDM_T::
apply(EIspProfile_T eIspProfile, dip_x_reg_t* pReg)
{
    // TOP ==> don't care
    ISP_WRITE_ENABLE_BITS(pReg, DIP_X_CTL_RGB_EN, UDM_EN, 1);
    //ISP_MGR_CTL_EN_P2_T::getInstance(m_eSensorDev).setEnable_UDM(MTRUE);

    // Register setting
    writeRegs(static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum, pReg);

    dumpRegInfo("UDM");

    return  MTRUE;
}


}
