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
#define LOG_TAG "isp_mgr_nsl2a"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include <camera_custom_nvram.h>
#include <awb_feature.h>
#include <awb_param.h>
#include <ae_feature.h>
#include <ae_param.h>
#include <drv/isp_drv.h>

#include "isp_mgr.h"
#include <drv/tuning_mgr.h>

#include <math.h>


#define CLAMP(x,min,max)       (((x) > (max)) ? (max) : (((x) < (min)) ? (min) : (x)))

#define SLP_PREC_F_SCAL 65536


namespace NSIspTuningv3
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// NSL2A
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_NSL2A_T&
ISP_MGR_NSL2A_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
{
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_NSL2A_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_NSL2A_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_NSL2A_DEV<ESensorDev_Sub>::getInstance();
    case ESensorDev_SubSecond: //  Sub Sensor
        return  ISP_MGR_NSL2A_DEV<ESensorDev_SubSecond>::getInstance();
    default:
        MY_ERR("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_NSL2A_DEV<ESensorDev_Main>::getInstance();
    }
}

template <>
ISP_MGR_NSL2A_T&
ISP_MGR_NSL2A_T::
put(ISP_NVRAM_NSL2A_T const& rParam)
{
    PUT_REG_INFO(CAM_NSL2A_CEN,     cen);
    PUT_REG_INFO(CAM_NSL2A_RR_CON0, rr_con0);
    PUT_REG_INFO(CAM_NSL2A_RR_CON1, rr_con1);
    PUT_REG_INFO(CAM_NSL2A_GAIN,    gain);

    return  (*this);
}


template <>
ISP_MGR_NSL2A_T&
ISP_MGR_NSL2A_T::
get(ISP_NVRAM_NSL2A_GET_T& rParam)
{
    GET_REG_INFO(CAM_NSL2A_CEN,     cen);
    GET_REG_INFO(CAM_NSL2A_RR_CON0, rr_con0);
    GET_REG_INFO(CAM_NSL2A_RR_CON1, rr_con1);
    GET_REG_INFO(CAM_NSL2A_GAIN,    gain);
    GET_REG_INFO(CAM_NSL2A_RZ,      rz);
    GET_REG_INFO(CAM_NSL2A_XOFF,    xoff);
    GET_REG_INFO(CAM_NSL2A_YOFF,    yoff);
    GET_REG_INFO(CAM_NSL2A_SLP_CON0,con0);
    GET_REG_INFO(CAM_NSL2A_SLP_CON1,con1);
    GET_REG_INFO(CAM_NSL2A_SIZE,    size);

    return  (*this);
}

#define GET_PROP(prop, init, val)\
{\
    char value[PROPERTY_VALUE_MAX] = {'\0'};\
    property_get(prop, value, (init));\
    (val) = atoi(value);\
}

MBOOL
ISP_MGR_NSL2A_T::
apply(EIspProfile_T eIspProfile)
{
    MBOOL u4NSL2a_EN = MFALSE;//(isEnable() && ISP_MGR_SL2_T::getInstance(m_eSensorDev).isEnable());
    ESoftwareScenario eSwScn = static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]);


    ISP_MGR_CTL_EN_P2_T::getInstance(m_eSensorDev).setEnable_NSL2A(u4NSL2a_EN);
    setEnable(u4NSL2a_EN);


    TuningMgr::getInstance().updateEngine(eSwScn, eTuningMgrFunc_Nsl2a);

    TUNING_MGR_WRITE_ENABLE_BITS(eSwScn, CAM_CTL_EN_P2, NSL2A_EN, u4NSL2a_EN);

    // Register setting
    TuningMgr::getInstance().tuningMgrWriteRegs(eSwScn, static_cast<TUNING_MGR_REG_IO_STRUCT*>(m_pRegInfo), m_u4RegInfoNum);

    dumpRegInfoP1("NSL2A");

    return  MTRUE;
}

MBOOL
ISP_MGR_NSL2A_T::
apply(EIspProfile_T eIspProfile, isp_reg_t* pReg, ISP_RRZ_INFO_T rRrzInfo)
{
    MBOOL u4NSL2a_EN = (isEnable() && ISP_MGR_SL2_T::getInstance(m_eSensorDev).isEnable());
    //ESoftwareScenario eSwScn = static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]);

    ISP_MGR_CTL_EN_P2_T::getInstance(m_eSensorDev).setEnable_NSL2A(u4NSL2a_EN);
    setEnable(u4NSL2a_EN);

    if(u4NSL2a_EN){

        reinterpret_cast<ISP_CAM_NSL2A_RZ_T*>(REG_INFO_VALUE_PTR(CAM_NSL2A_RZ))->SL2_HRZ_COMP = (MUINT32)(2048.0f/rRrzInfo.RtoW);
        reinterpret_cast<ISP_CAM_NSL2A_RZ_T*>(REG_INFO_VALUE_PTR(CAM_NSL2A_RZ))->SL2_VRZ_COMP = (MUINT32)(2048.0f/rRrzInfo.RtoH);

        reinterpret_cast<ISP_CAM_NSL2A_CEN_T*>(REG_INFO_VALUE_PTR(CAM_NSL2A_CEN))->SL2_CENTR_X = (m_PureSL2.cen.bits.SL2_CENTR_X - rRrzInfo.OfstX);
        reinterpret_cast<ISP_CAM_NSL2A_CEN_T*>(REG_INFO_VALUE_PTR(CAM_NSL2A_CEN))->SL2_CENTR_Y = (m_PureSL2.cen.bits.SL2_CENTR_Y - rRrzInfo.OfstY);

        MFLOAT R0 = sqrt(m_PureSL2.max0_rr.bits.SL2_RR_0);
        MFLOAT R1 = sqrt(m_PureSL2.max1_rr.bits.SL2_RR_1);
        MFLOAT R2 = sqrt(m_PureSL2.max2_rr.bits.SL2_RR_2);
        MFLOAT maxR = (MFLOAT)(( m_PureSL2.cen.bits.SL2_CENTR_X + m_PureSL2.cen.bits.SL2_CENTR_Y) * 1448 >> 11);

#if 0
        CAM_LOGE("chooo: R0: %f, R1: %f, R2: %f, maxR: %f",R0,R1,R2,maxR );
#endif

        reinterpret_cast<ISP_CAM_NSL2A_RR_CON0_T*>(REG_INFO_VALUE_PTR(CAM_NSL2A_RR_CON0))->SL2_R_0 = (MUINT32)R0;
        reinterpret_cast<ISP_CAM_NSL2A_RR_CON0_T*>(REG_INFO_VALUE_PTR(CAM_NSL2A_RR_CON0))->SL2_R_1 = (MUINT32)R1;
        reinterpret_cast<ISP_CAM_NSL2A_RR_CON1_T*>(REG_INFO_VALUE_PTR(CAM_NSL2A_RR_CON1))->SL2_R_2 = (MUINT32)R2;


        MFLOAT tempGain0 = (MFLOAT)reinterpret_cast<ISP_CAM_NSL2A_RR_CON1_T*>(REG_INFO_VALUE_PTR(CAM_NSL2A_RR_CON1))->SL2_GAIN_0;
        MFLOAT tempGain1 = (MFLOAT)reinterpret_cast<ISP_CAM_NSL2A_RR_CON1_T*>(REG_INFO_VALUE_PTR(CAM_NSL2A_RR_CON1))->SL2_GAIN_1;
        MFLOAT tempGain2 = (MFLOAT)reinterpret_cast<ISP_CAM_NSL2A_GAIN_T*>(REG_INFO_VALUE_PTR(CAM_NSL2A_GAIN))->SL2_GAIN_2;
        MFLOAT tempGain3 = (MFLOAT)reinterpret_cast<ISP_CAM_NSL2A_GAIN_T*>(REG_INFO_VALUE_PTR(CAM_NSL2A_GAIN))->SL2_GAIN_3;
        MFLOAT tempGain4 = (MFLOAT)reinterpret_cast<ISP_CAM_NSL2A_GAIN_T*>(REG_INFO_VALUE_PTR(CAM_NSL2A_GAIN))->SL2_GAIN_4;

        reinterpret_cast<ISP_CAM_NSL2A_SLP_CON0_T*>(REG_INFO_VALUE_PTR(CAM_NSL2A_SLP_CON0))->SL2_SLP_1 =
            (MUINT32)(((tempGain1 - tempGain0)/ R0) * SLP_PREC_F_SCAL + 0.5);
        reinterpret_cast<ISP_CAM_NSL2A_SLP_CON0_T*>(REG_INFO_VALUE_PTR(CAM_NSL2A_SLP_CON0))->SL2_SLP_2 =
            (MUINT32)(((tempGain2 - tempGain1)/ (R1-R0)) * SLP_PREC_F_SCAL + 0.5);
        reinterpret_cast<ISP_CAM_NSL2A_SLP_CON1_T*>(REG_INFO_VALUE_PTR(CAM_NSL2A_SLP_CON1))->SL2_SLP_3 =
            (MUINT32)(((tempGain3 - tempGain2)/ (R2-R1)) * SLP_PREC_F_SCAL + 0.5);
        reinterpret_cast<ISP_CAM_NSL2A_SLP_CON1_T*>(REG_INFO_VALUE_PTR(CAM_NSL2A_SLP_CON1))->SL2_SLP_4 =
            (MUINT32)(((tempGain4 - tempGain3)/ (maxR-R2)) * SLP_PREC_F_SCAL + 0.5);

        REG_INFO_VALUE(CAM_NSL2A_XOFF) = 0;
        REG_INFO_VALUE(CAM_NSL2A_YOFF) = 0;

    }

    ISP_WRITE_ENABLE_BITS(pReg, CAM_CTL_EN_P2, NSL2A_EN, u4NSL2a_EN);

    // Register setting
    writeRegs(static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum, pReg);

    dumpRegInfo("NSL2A");

    return  MTRUE;
}


}
