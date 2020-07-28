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
#include <camera_custom_nvram.h>
#include "isp_mgr.h"

namespace NSIspTuningv3
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// NBC
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_NBC_T&
ISP_MGR_NBC_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
    {
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_NBC_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_NBC_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_NBC_DEV<ESensorDev_Sub>::getInstance();
    case ESensorDev_SubSecond: //  Main Second Sensor
        return  ISP_MGR_NBC_DEV<ESensorDev_SubSecond>::getInstance();
    default:
        CAM_LOGE("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_NBC_DEV<ESensorDev_Main>::getInstance();
    }
}

template <>
ISP_MGR_NBC_T&
ISP_MGR_NBC_T::
put(ISP_NVRAM_ANR_T const& rParam)
{
    PUT_REG_INFO(DIP_X_ANR_CON1,    con1);
    PUT_REG_INFO(DIP_X_ANR_CON2,    con2);
    PUT_REG_INFO(DIP_X_ANR_YAD1,    yad1);
    PUT_REG_INFO(DIP_X_ANR_YAD2,    yad2);
    PUT_REG_INFO(DIP_X_ANR_Y4LUT1,  y4lut1);
    PUT_REG_INFO(DIP_X_ANR_Y4LUT2,  y4lut2);
    PUT_REG_INFO(DIP_X_ANR_Y4LUT3,  y4lut3);
    PUT_REG_INFO(DIP_X_ANR_C4LUT1,  c4lut1);
    PUT_REG_INFO(DIP_X_ANR_C4LUT2,  c4lut2);
    PUT_REG_INFO(DIP_X_ANR_C4LUT3,  c4lut3);
    PUT_REG_INFO(DIP_X_ANR_A4LUT2,  a4lut2);
    PUT_REG_INFO(DIP_X_ANR_A4LUT3,  a4lut3);
    PUT_REG_INFO(DIP_X_ANR_L4LUT1,  l4lut1);
    PUT_REG_INFO(DIP_X_ANR_L4LUT2,  l4lut2);
    PUT_REG_INFO(DIP_X_ANR_L4LUT3,  l4lut3);
    PUT_REG_INFO(DIP_X_ANR_PTY,     pty);
    PUT_REG_INFO(DIP_X_ANR_CAD,     cad);
    PUT_REG_INFO(DIP_X_ANR_PTC,     ptc);
    PUT_REG_INFO(DIP_X_ANR_LCE,     lce);
    PUT_REG_INFO(DIP_X_ANR_T4LUT1,  t4lut1);
    PUT_REG_INFO(DIP_X_ANR_T4LUT2,  t4lut2);
    PUT_REG_INFO(DIP_X_ANR_T4LUT3,  t4lut3);
    PUT_REG_INFO(DIP_X_ANR_ACT1,    act1);
    PUT_REG_INFO(DIP_X_ANR_ACT2,    act2);
//    PUT_REG_INFO(DIP_X_ANR_ACT3,    act3);
    PUT_REG_INFO(DIP_X_ANR_ACT4,    act4);
    PUT_REG_INFO(DIP_X_ANR_ACTYHL,  actyhl);
    PUT_REG_INFO(DIP_X_ANR_ACTYHH,  actyhh);
    PUT_REG_INFO(DIP_X_ANR_ACTYL,   actyl);
    PUT_REG_INFO(DIP_X_ANR_ACTYHL2, actyhl2);
    PUT_REG_INFO(DIP_X_ANR_ACTYHH2, actyhh2);
    PUT_REG_INFO(DIP_X_ANR_ACTYL2,  actyl2);
    PUT_REG_INFO(DIP_X_ANR_ACTC,    actc);
    PUT_REG_INFO(DIP_X_ANR_YLAD,    ylad);
    PUT_REG_INFO(DIP_X_ANR_YLAD2,   ylad2);
    PUT_REG_INFO(DIP_X_ANR_YLAD3,   ylad3);
    PUT_REG_INFO(DIP_X_ANR_PTYL,    ptyl);
    PUT_REG_INFO(DIP_X_ANR_LCOEF,   lcoef);
    PUT_REG_INFO(DIP_X_ANR_YDIR,    ydir);

    return  (*this);
}

template <>
MBOOL
ISP_MGR_NBC_T::
get(ISP_NVRAM_ANR_T& rParam, const dip_x_reg_t* pReg) const
{
    if (pReg)
    {
        GET_REG_INFO_BUF(DIP_X_ANR_CON1,    con1);
        GET_REG_INFO_BUF(DIP_X_ANR_CON2,    con2);
        GET_REG_INFO_BUF(DIP_X_ANR_YAD1,    yad1);
        GET_REG_INFO_BUF(DIP_X_ANR_YAD2,    yad2);
        GET_REG_INFO_BUF(DIP_X_ANR_Y4LUT1,  y4lut1);
        GET_REG_INFO_BUF(DIP_X_ANR_Y4LUT2,  y4lut2);
        GET_REG_INFO_BUF(DIP_X_ANR_Y4LUT3,  y4lut3);
        GET_REG_INFO_BUF(DIP_X_ANR_C4LUT1,  c4lut1);
        GET_REG_INFO_BUF(DIP_X_ANR_C4LUT2,  c4lut2);
        GET_REG_INFO_BUF(DIP_X_ANR_C4LUT3,  c4lut3);
        GET_REG_INFO_BUF(DIP_X_ANR_A4LUT2,  a4lut2);
        GET_REG_INFO_BUF(DIP_X_ANR_A4LUT3,  a4lut3);
        GET_REG_INFO_BUF(DIP_X_ANR_L4LUT1,  l4lut1);
        GET_REG_INFO_BUF(DIP_X_ANR_L4LUT2,  l4lut2);
        GET_REG_INFO_BUF(DIP_X_ANR_L4LUT3,  l4lut3);
        GET_REG_INFO_BUF(DIP_X_ANR_PTY,     pty);
        GET_REG_INFO_BUF(DIP_X_ANR_CAD,     cad);
        GET_REG_INFO_BUF(DIP_X_ANR_PTC,     ptc);
        GET_REG_INFO_BUF(DIP_X_ANR_LCE,     lce);
        GET_REG_INFO_BUF(DIP_X_ANR_T4LUT1,  t4lut1);
        GET_REG_INFO_BUF(DIP_X_ANR_T4LUT2,  t4lut2);
        GET_REG_INFO_BUF(DIP_X_ANR_T4LUT3,  t4lut3);
        GET_REG_INFO_BUF(DIP_X_ANR_ACT1,    act1);
        GET_REG_INFO_BUF(DIP_X_ANR_ACT2,    act2);
//        GET_REG_INFO_BUF(DIP_X_ANR_ACT3,    act3);
        GET_REG_INFO_BUF(DIP_X_ANR_ACT4,    act4);
        GET_REG_INFO_BUF(DIP_X_ANR_ACTYHL,  actyhl);
        GET_REG_INFO_BUF(DIP_X_ANR_ACTYHH,  actyhh);
        GET_REG_INFO_BUF(DIP_X_ANR_ACTYL,   actyl);
        GET_REG_INFO_BUF(DIP_X_ANR_ACTYHL2, actyhl2);
        GET_REG_INFO_BUF(DIP_X_ANR_ACTYHH2, actyhh2);
        GET_REG_INFO_BUF(DIP_X_ANR_ACTYL2,  actyl2);
        GET_REG_INFO_BUF(DIP_X_ANR_ACTC,    actc);
        GET_REG_INFO_BUF(DIP_X_ANR_YLAD,    ylad);
        GET_REG_INFO_BUF(DIP_X_ANR_YLAD2,   ylad2);
        GET_REG_INFO_BUF(DIP_X_ANR_YLAD3,   ylad3);
        GET_REG_INFO_BUF(DIP_X_ANR_PTYL,    ptyl);
        GET_REG_INFO_BUF(DIP_X_ANR_LCOEF,   lcoef);
        GET_REG_INFO_BUF(DIP_X_ANR_YDIR,    ydir);
    }
    return MTRUE;
}


MBOOL
ISP_MGR_NBC_T::
apply(EIspProfile_T /*eIspProfile*/, dip_x_reg_t* pReg)
{
    MBOOL bANR1_ENC = reinterpret_cast<ISP_DIP_X_ANR_CON1_T*>(REG_INFO_VALUE_PTR(DIP_X_ANR_CON1))->ANR_ENC;
    MBOOL bANR1_ENY = reinterpret_cast<ISP_DIP_X_ANR_CON1_T*>(REG_INFO_VALUE_PTR(DIP_X_ANR_CON1))->ANR_ENY;

    MBOOL bNBC_EN = (isANR1Enable() & (bANR1_ENY|bANR1_ENC));

    if(bNBC_EN){
        MBOOL bANR_TBL_EN = ((reinterpret_cast<ISP_DIP_X_ANR_CON1_T*>(REG_INFO_VALUE_PTR(DIP_X_ANR_CON1))->ANR_TABLE_EN) & isANRTBLEnable());
        reinterpret_cast<ISP_DIP_X_ANR_CON1_T*>(REG_INFO_VALUE_PTR(DIP_X_ANR_CON1))->ANR_TABLE_EN = bANR_TBL_EN;
    }

    // TOP
    ISP_WRITE_ENABLE_BITS(pReg, DIP_X_CTL_YUV_EN, NBC_EN, bNBC_EN);
    //ISP_MGR_CTL_EN_P2_T::getInstance(m_eSensorDev).setEnable_NBC(bNBC_EN);

    //Register setting
    writeRegs(static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum, pReg);

    // ANR table
    writeRegs(static_cast<RegInfo_T*>(m_rIspRegInfo_ANR_TBL), ANR_HW_TBL_NUM, pReg);

    dumpRegInfo("NBC");

    return  MTRUE;
}

}
