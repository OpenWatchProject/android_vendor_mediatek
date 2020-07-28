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
#include <aaa_log.h>
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
    default:
        MY_ERR("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_NBC_DEV<ESensorDev_Main>::getInstance();
    }
}

template <>
ISP_MGR_NBC_T&
ISP_MGR_NBC_T::
put(ISP_NVRAM_ANR_T const& rParam)
{
    //PUT_REG_INFO(CAM_ANR_TBL, tbl);
    PUT_REG_INFO(CAM_ANR_CON1, con1);
    PUT_REG_INFO(CAM_ANR_CON2, con2);
    PUT_REG_INFO(CAM_ANR_YAD1, yad1);
    PUT_REG_INFO(CAM_ANR_YAD2, yad2);
    PUT_REG_INFO(CAM_ANR_Y4LUT1, lut1);
    PUT_REG_INFO(CAM_ANR_Y4LUT2, lut2);
    PUT_REG_INFO(CAM_ANR_Y4LUT3, lut3);
    PUT_REG_INFO(CAM_ANR_C4LUT1, clut1);
    PUT_REG_INFO(CAM_ANR_C4LUT2, clut2);
    PUT_REG_INFO(CAM_ANR_C4LUT3, clut3);
    PUT_REG_INFO(CAM_ANR_A4LUT2, alut2);
    PUT_REG_INFO(CAM_ANR_A4LUT3, alut3);
    PUT_REG_INFO(CAM_ANR_L4LUT1, llut1);
    PUT_REG_INFO(CAM_ANR_L4LUT2, llut2);
    PUT_REG_INFO(CAM_ANR_L4LUT3, llut3);
    PUT_REG_INFO(CAM_ANR_PTY, pty);
    PUT_REG_INFO(CAM_ANR_CAD, cad);
    PUT_REG_INFO(CAM_ANR_PTC, ptc);
    PUT_REG_INFO(CAM_ANR_LCE, lce);
    PUT_REG_INFO(CAM_ANR_MED1, med1);
    PUT_REG_INFO(CAM_ANR_MED2, med2);
    PUT_REG_INFO(CAM_ANR_MED3, med3);
    PUT_REG_INFO(CAM_ANR_MED4, med4);  
    PUT_REG_INFO(CAM_ANR_HP1, hp1);
    PUT_REG_INFO(CAM_ANR_HP2, hp2);
    PUT_REG_INFO(CAM_ANR_HP3, hp3);
    PUT_REG_INFO(CAM_ANR_ACT1, act1);
    PUT_REG_INFO(CAM_ANR_ACT2, act2);
    PUT_REG_INFO(CAM_ANR_ACT3, act3);
    PUT_REG_INFO(CAM_ANR_ACTY, acty);
    PUT_REG_INFO(CAM_ANR_ACTC, actc);
    PUT_REG_INFO(CAM_ANR_RSV1, rsv1);
	

    m_bANR1ENCBackup = rParam.con1.bits.ANR1_ENC;
    m_bANR1ENYBackup = rParam.con1.bits.ANR1_ENY;

    return  (*this);
}


template <>
ISP_MGR_NBC_T&
ISP_MGR_NBC_T::
get(ISP_NVRAM_ANR_T& rParam)
{
    //GET_REG_INFO(CAM_ANR_TBL, tbl);
    GET_REG_INFO(CAM_ANR_CON1, con1);
    GET_REG_INFO(CAM_ANR_CON2, con2);
    GET_REG_INFO(CAM_ANR_YAD1, yad1);
    GET_REG_INFO(CAM_ANR_YAD2, yad2);
    GET_REG_INFO(CAM_ANR_Y4LUT1, lut1);
    GET_REG_INFO(CAM_ANR_Y4LUT2, lut2);
    GET_REG_INFO(CAM_ANR_Y4LUT3, lut3);
    GET_REG_INFO(CAM_ANR_C4LUT1, clut1);
    GET_REG_INFO(CAM_ANR_C4LUT2, clut2);
    GET_REG_INFO(CAM_ANR_C4LUT3, clut3);
    GET_REG_INFO(CAM_ANR_A4LUT2, alut2);
    GET_REG_INFO(CAM_ANR_A4LUT3, alut3);
    GET_REG_INFO(CAM_ANR_L4LUT1, llut1);
    GET_REG_INFO(CAM_ANR_L4LUT2, llut2);
    GET_REG_INFO(CAM_ANR_L4LUT3, llut3);
    GET_REG_INFO(CAM_ANR_PTY, pty);
    GET_REG_INFO(CAM_ANR_CAD, cad);
    GET_REG_INFO(CAM_ANR_PTC, ptc);
    GET_REG_INFO(CAM_ANR_LCE, lce);
    GET_REG_INFO(CAM_ANR_MED1, med1);
    GET_REG_INFO(CAM_ANR_MED2, med2);
    GET_REG_INFO(CAM_ANR_MED3, med3);
    GET_REG_INFO(CAM_ANR_MED4, med4);  
    GET_REG_INFO(CAM_ANR_HP1, hp1);
    GET_REG_INFO(CAM_ANR_HP2, hp2);
    GET_REG_INFO(CAM_ANR_HP3, hp3);
    GET_REG_INFO(CAM_ANR_ACT1, act1);
    GET_REG_INFO(CAM_ANR_ACT2, act2);
    GET_REG_INFO(CAM_ANR_ACT3, act3);
    GET_REG_INFO(CAM_ANR_ACTY, acty);
    GET_REG_INFO(CAM_ANR_ACTC, actc);
    GET_REG_INFO(CAM_ANR_RSV1, rsv1);

    return  (*this);
}

MBOOL
ISP_MGR_NBC_T::
apply(EIspProfile_T eIspProfile)
{
    TuningMgr::getInstance().updateEngine(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]), 
                                           eTuningMgrFunc_Nbc);

    MBOOL bANR1_ENY = m_bANR1ENYBackup & isANR1_ENYEnable();
    MBOOL bANR1_ENC = m_bANR1ENCBackup & isANR1_ENCEnable();
    MBOOL bNBC_EN = bANR1_ENY|bANR1_ENC;

    
    if (bNBC_EN && !ISP_MGR_NSL2_T::getInstance(m_eSensorDev).getNSL2AOnOff())
    {

         reinterpret_cast<ISP_CAM_ANR_CON1_T*>(REG_INFO_VALUE_PTR(CAM_ANR_CON1))->ANR1_LCE_LINK = 0;
    }

    char InputValue[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.isp_nbc.apply", InputValue, "0"); 
    MUINT32 debugEn = atoi(InputValue);
    if (debugEn) MY_LOG("m_bANR1ENYBackup(%d),isANR_ENYEnable(%d),m_bANR1ENCBackup(%d),isANR_ENCEnable(%d),bNBC_EN(%d)"
                        , m_bANR1ENYBackup
                        , isANR1_ENYEnable()
                        , m_bANR1ENCBackup
                        , isANR1_ENCEnable()
                        , bNBC_EN);


    reinterpret_cast<ISP_CAM_ANR_CON1_T*>(REG_INFO_VALUE_PTR(CAM_ANR_CON1))->ANR1_ENY = bANR1_ENY;
    reinterpret_cast<ISP_CAM_ANR_CON1_T*>(REG_INFO_VALUE_PTR(CAM_ANR_CON1))->ANR1_ENC = bANR1_ENC;

    ISP_MGR_CTL_EN_P2_T::getInstance(m_eSensorDev).setEnable_NBC(bNBC_EN);

    // TOP 
    TUNING_MGR_WRITE_ENABLE_BITS(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]), CAM_CTL_EN_P2, NBC_EN, bNBC_EN);
    
    TuningMgr::getInstance().tuningMgrWriteRegs(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]), 
                                                 static_cast<TUNING_MGR_REG_IO_STRUCT*>(m_pRegInfo), 
                                                 m_u4RegInfoNum);

    dumpRegInfo("NBC");


    //HW limitation (if NBC off, then NSL2A off)
    if (!bNBC_EN && ISP_MGR_NSL2_T::getInstance(m_eSensorDev).getNSL2AOnOff()){

         ISP_MGR_NSL2_T::getInstance(m_eSensorDev).setNSL2AEnable(MFALSE);

         TuningMgr::getInstance().updateEngine(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]), 
                                                eTuningMgrFunc_Nsl2a);
         
         TUNING_MGR_WRITE_ENABLE_BITS(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]), CAM_CTL_EN_P2, NSL2A_EN, bNBC_EN);
    } 

    return  MTRUE;
}

MBOOL
ISP_MGR_NBC_T::
apply(EIspProfile_T eIspProfile, isp_reg_t* pReg)
{
    MBOOL bANR1_ENY = m_bANR1ENYBackup & isANR1_ENYEnable();
    MBOOL bANR1_ENC = m_bANR1ENCBackup & isANR1_ENCEnable();
    MBOOL bNBC_EN = bANR1_ENY|bANR1_ENC;

    
    if (bNBC_EN && !ISP_MGR_NSL2_T::getInstance(m_eSensorDev).getNSL2AOnOff())
    {

         reinterpret_cast<ISP_CAM_ANR_CON1_T*>(REG_INFO_VALUE_PTR(CAM_ANR_CON1))->ANR1_LCE_LINK = 0;
    }

    reinterpret_cast<ISP_CAM_ANR_CON1_T*>(REG_INFO_VALUE_PTR(CAM_ANR_CON1))->ANR1_ENY = bANR1_ENY;
    reinterpret_cast<ISP_CAM_ANR_CON1_T*>(REG_INFO_VALUE_PTR(CAM_ANR_CON1))->ANR1_ENC = bANR1_ENC;

    ISP_MGR_CTL_EN_P2_T::getInstance(m_eSensorDev).setEnable_NBC(bNBC_EN);

    // TOP 
    ISP_WRITE_ENABLE_BITS(pReg, CAM_CTL_EN_P2, NBC_EN, bNBC_EN);
    
    writeRegs(static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum, pReg);

    dumpRegInfo("NBC");

    //HW limitation (if NBC off, then NSL2A off)
    if (!bNBC_EN && ISP_MGR_NSL2_T::getInstance(m_eSensorDev).getNSL2AOnOff()){

         ISP_MGR_NSL2_T::getInstance(m_eSensorDev).setNSL2AEnable(MFALSE);

         ISP_WRITE_ENABLE_BITS(pReg, CAM_CTL_EN_P2, NSL2A_EN, bNBC_EN);
    }

    return  MTRUE;
}

}
