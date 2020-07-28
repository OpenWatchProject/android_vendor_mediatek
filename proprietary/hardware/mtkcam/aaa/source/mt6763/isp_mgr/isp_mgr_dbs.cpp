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
#define LOG_TAG "isp_mgr_dbs"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include "isp_mgr.h"

namespace NSIspTuningv3
{

#define CLAMP(x,min,max)       (((x) > (max)) ? (max) : (((x) < (min)) ? (min) : (x)))


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DBS
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_DBS_T&
ISP_MGR_DBS_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
    {
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_DBS_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_DBS_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_DBS_DEV<ESensorDev_Sub>::getInstance();
    case ESensorDev_SubSecond: //  Main Second Sensor
        return  ISP_MGR_DBS_DEV<ESensorDev_SubSecond>::getInstance();
    default:
        CAM_LOGE("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_DBS_DEV<ESensorDev_Main>::getInstance();
    }
}

template <>
ISP_MGR_DBS_T&
ISP_MGR_DBS_T::
put(ISP_NVRAM_DBS_T const& rParam)
{
    PUT_REG_INFO(CAM_DBS_SIGMA,    sigma);
    PUT_REG_INFO(CAM_DBS_BSTBL_0,  bstbl_0);
    PUT_REG_INFO(CAM_DBS_BSTBL_1,  bstbl_1);
    PUT_REG_INFO(CAM_DBS_BSTBL_2,  bstbl_2);
    PUT_REG_INFO(CAM_DBS_BSTBL_3,  bstbl_3);
    PUT_REG_INFO(CAM_DBS_CTL,      ctl);
    PUT_REG_INFO(CAM_DBS_SIGMA_2,  sigma_2);
    PUT_REG_INFO(CAM_DBS_YGN,      ygn);
    PUT_REG_INFO(CAM_DBS_SL_Y12,   sl_y12);
    PUT_REG_INFO(CAM_DBS_SL_Y34,   sl_y34);
    PUT_REG_INFO(CAM_DBS_SL_G12,   sl_g12);
    PUT_REG_INFO(CAM_DBS_SL_G34,   sl_g34);

    return  (*this);
}


template <>
ISP_MGR_DBS_T&
ISP_MGR_DBS_T::
get(ISP_NVRAM_DBS_T& rParam)
{
    GET_REG_INFO(CAM_DBS_SIGMA,    sigma);
    GET_REG_INFO(CAM_DBS_BSTBL_0,  bstbl_0);
    GET_REG_INFO(CAM_DBS_BSTBL_1,  bstbl_1);
    GET_REG_INFO(CAM_DBS_BSTBL_2,  bstbl_2);
    GET_REG_INFO(CAM_DBS_BSTBL_3,  bstbl_3);
    GET_REG_INFO(CAM_DBS_CTL,      ctl);
    GET_REG_INFO(CAM_DBS_SIGMA_2,  sigma_2);
    GET_REG_INFO(CAM_DBS_YGN,      ygn);
    GET_REG_INFO(CAM_DBS_SL_Y12,   sl_y12);
    GET_REG_INFO(CAM_DBS_SL_Y34,   sl_y34);
    GET_REG_INFO(CAM_DBS_SL_G12,   sl_g12);
    GET_REG_INFO(CAM_DBS_SL_G34,   sl_g34);

    return  (*this);
}

MBOOL
ISP_MGR_DBS_T::
apply(EIspProfile_T /*eIspProfile*/, TuningMgr& rTuning, MINT32 i4SubsampleIdex)
{
    MBOOL bDBS_EN = isEnable();

    MBOOL bSL2F = ISP_MGR_SL2F::getInstance(m_eSensorDev).isEnable();

    //HW constrain
    if(!bSL2F){
        reinterpret_cast<ISP_CAM_DBS_CTL_T*>(REG_INFO_VALUE_PTR(CAM_DBS_CTL))->DBS_SL_EN = 0;
    }

    rTuning.updateEngine(eTuningMgrFunc_DBS, bDBS_EN, i4SubsampleIdex);

    // TOP
    TUNING_MGR_WRITE_BITS_CAM((&rTuning), CAM_CTL_EN, DBS_EN, bDBS_EN, i4SubsampleIdex);
    ISP_MGR_CTL_EN_P1_T::getInstance(m_eSensorDev).setEnable_DBS(bDBS_EN);

    // Register setting
    rTuning.tuningMgrWriteRegs(static_cast<TUNING_MGR_REG_IO_STRUCT*>(m_pRegInfo),
            m_u4RegInfoNum, i4SubsampleIdex);

    dumpRegInfoP1("DBS");

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DBS2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_DBS2_T&
ISP_MGR_DBS2_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
    {
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_DBS2_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_DBS2_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_DBS2_DEV<ESensorDev_Sub>::getInstance();
    case ESensorDev_SubSecond: //  Main Second Sensor
        return  ISP_MGR_DBS2_DEV<ESensorDev_SubSecond>::getInstance();
    default:
        CAM_LOGE("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_DBS2_DEV<ESensorDev_Main>::getInstance();
    }
}

template <>
ISP_MGR_DBS2_T&
ISP_MGR_DBS2_T::
put(ISP_NVRAM_DBS_T const& rParam)
{
    PUT_REG_INFO(DIP_X_DBS2_SIGMA,    sigma);
    PUT_REG_INFO(DIP_X_DBS2_BSTBL_0,  bstbl_0);
    PUT_REG_INFO(DIP_X_DBS2_BSTBL_1,  bstbl_1);
    PUT_REG_INFO(DIP_X_DBS2_BSTBL_2,  bstbl_2);
    PUT_REG_INFO(DIP_X_DBS2_BSTBL_3,  bstbl_3);
    PUT_REG_INFO(DIP_X_DBS2_CTL,      ctl);
    PUT_REG_INFO(DIP_X_DBS2_SIGMA_2,  sigma_2);
    PUT_REG_INFO(DIP_X_DBS2_YGN,      ygn);
    PUT_REG_INFO(DIP_X_DBS2_SL_Y12,   sl_y12);
    PUT_REG_INFO(DIP_X_DBS2_SL_Y34,   sl_y34);
    PUT_REG_INFO(DIP_X_DBS2_SL_G12,   sl_g12);
    PUT_REG_INFO(DIP_X_DBS2_SL_G34,   sl_g34);

    return  (*this);
}

template <>
MBOOL
ISP_MGR_DBS2_T::
get(ISP_NVRAM_DBS_T& rParam, const dip_x_reg_t* pReg) const
{
    if (pReg)
    {
        GET_REG_INFO_BUF(DIP_X_DBS2_SIGMA,    sigma);
        GET_REG_INFO_BUF(DIP_X_DBS2_BSTBL_0,  bstbl_0);
        GET_REG_INFO_BUF(DIP_X_DBS2_BSTBL_1,  bstbl_1);
        GET_REG_INFO_BUF(DIP_X_DBS2_BSTBL_2,  bstbl_2);
        GET_REG_INFO_BUF(DIP_X_DBS2_BSTBL_3,  bstbl_3);
        GET_REG_INFO_BUF(DIP_X_DBS2_CTL,      ctl);
        GET_REG_INFO_BUF(DIP_X_DBS2_SIGMA_2,  sigma_2);
        GET_REG_INFO_BUF(DIP_X_DBS2_YGN,      ygn);
        GET_REG_INFO_BUF(DIP_X_DBS2_SL_Y12,   sl_y12);
        GET_REG_INFO_BUF(DIP_X_DBS2_SL_Y34,   sl_y34);
        GET_REG_INFO_BUF(DIP_X_DBS2_SL_G12,   sl_g12);
        GET_REG_INFO_BUF(DIP_X_DBS2_SL_G34,   sl_g34);
    }
    return MTRUE;
}

MBOOL
ISP_MGR_DBS2_T::
apply(EIspProfile_T /*eIspProfile*/, dip_x_reg_t* pReg)
{
    MBOOL bDBS_EN = isEnable();

    // TOP
    ISP_WRITE_ENABLE_BITS(pReg, DIP_X_CTL_RGB_EN, DBS2_EN, bDBS_EN);
    //ISP_MGR_CTL_EN_P2_T::getInstance(m_eSensorDev).setEnable_DBS2(bDBS_EN);

    // Register setting
    writeRegs(static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum, pReg);


    dumpRegInfo("DBS2");

    return  MTRUE;
}

}
