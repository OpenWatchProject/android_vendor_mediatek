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
#define LOG_TAG "isp_mgr_bnr"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (0)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include "isp_mgr.h"
#include <string>
//#include <mtkcam/utils/imagebuf/IImageBuffer.h>
#include <mtkcam/utils/imagebuf/IIonImageBufferHeap.h>

namespace NSIspTuning
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BNR
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_BNR_T&
ISP_MGR_BNR_T::
getInstance(ESensorDev_T const eSensorDev, ESensorTG_T const eSensorTG)
{
    switch (eSensorDev)
    {
    case ESensorDev_Main: //  Main Sensor
        if (eSensorTG == ESensorTG_1)
            return  ISP_MGR_BNR_DEV<ESensorDev_Main, ESensorTG_1>::getInstance();
        else
            return  ISP_MGR_BNR_DEV<ESensorDev_Main, ESensorTG_2>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        if (eSensorTG == ESensorTG_1)
            return  ISP_MGR_BNR_DEV<ESensorDev_MainSecond, ESensorTG_1>::getInstance();
        else
            return  ISP_MGR_BNR_DEV<ESensorDev_MainSecond, ESensorTG_2>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        if (eSensorTG == ESensorTG_1)
            return  ISP_MGR_BNR_DEV<ESensorDev_Sub, ESensorTG_1>::getInstance();
        else
            return  ISP_MGR_BNR_DEV<ESensorDev_Sub, ESensorTG_2>::getInstance();
    default:
        MY_ERR("eSensorDev = %d, eSensorTG = %d", eSensorDev, eSensorTG);
        return  ISP_MGR_BNR_DEV<ESensorDev_Main, ESensorTG_1>::getInstance();
    }
}

template <>
ISP_MGR_BNR_T&
ISP_MGR_BNR_T::
put(ISP_NVRAM_BNR_BPC_T const& rParam)
{
    if (m_eSensorTG == ESensorTG_1) {
        PUT_REG_INFO(CAM_BNR_BPC_CON, con);
        PUT_REG_INFO(CAM_BNR_BPC_TH1, th1);
        PUT_REG_INFO(CAM_BNR_BPC_TH2, th2);
        PUT_REG_INFO(CAM_BNR_BPC_TH3, th3);
        PUT_REG_INFO(CAM_BNR_BPC_TH4, th4);
        PUT_REG_INFO(CAM_BNR_BPC_DTC, dtc);
        PUT_REG_INFO(CAM_BNR_BPC_COR, cor);
        //PUT_REG_INFO(CAM_BPC_TBLI1, tbli1);
        //PUT_REG_INFO(CAM_BPC_TBLI2, tbli2);
        PUT_REG_INFO(CAM_BNR_BPC_TH1_C, th1_c);
        PUT_REG_INFO(CAM_BNR_BPC_TH2_C, th2_c);
        PUT_REG_INFO(CAM_BNR_BPC_TH3_C, th3_c);
    }
    else {
        PUT_REG_INFO(CAM_BNR_D_BPC_CON, con);
        PUT_REG_INFO(CAM_BNR_D_BPC_TH1, th1);
        PUT_REG_INFO(CAM_BNR_D_BPC_TH2, th2);
        PUT_REG_INFO(CAM_BNR_D_BPC_TH3, th3);
        PUT_REG_INFO(CAM_BNR_D_BPC_TH4, th4);
        PUT_REG_INFO(CAM_BNR_D_BPC_DTC, dtc);
        PUT_REG_INFO(CAM_BNR_D_BPC_COR, cor);
        //PUT_REG_INFO(CAM_BPC_D_TBLI1, tbli1);
        //PUT_REG_INFO(CAM_BPC_D_TBLI2, tbli2);
        PUT_REG_INFO(CAM_BNR_D_BPC_TH1_C, th1_c);
        PUT_REG_INFO(CAM_BNR_D_BPC_TH2_C, th2_c);
        PUT_REG_INFO(CAM_BNR_D_BPC_TH3_C, th3_c);
    }

    return  (*this);
}


template <>
ISP_MGR_BNR_T&
ISP_MGR_BNR_T::
get(ISP_NVRAM_BNR_BPC_T& rParam)
{
    if (m_eSensorTG == ESensorTG_1) {
        GET_REG_INFO(CAM_BNR_BPC_CON, con);
        GET_REG_INFO(CAM_BNR_BPC_TH1, th1);
        GET_REG_INFO(CAM_BNR_BPC_TH2, th2);
        GET_REG_INFO(CAM_BNR_BPC_TH3, th3);
        GET_REG_INFO(CAM_BNR_BPC_TH4, th4);
        GET_REG_INFO(CAM_BNR_BPC_DTC, dtc);
        GET_REG_INFO(CAM_BNR_BPC_COR, cor);
        //GET_REG_INFO(CAM_BPC_TBLI1, tbli1);
        //GET_REG_INFO(CAM_BPC_TBLI2, tbli2);
        GET_REG_INFO(CAM_BNR_BPC_TH1_C, th1_c);
        GET_REG_INFO(CAM_BNR_BPC_TH2_C, th2_c);
        GET_REG_INFO(CAM_BNR_BPC_TH3_C, th3_c);
    }
    else {
        GET_REG_INFO(CAM_BNR_D_BPC_CON, con);
        GET_REG_INFO(CAM_BNR_D_BPC_TH1, th1);
        GET_REG_INFO(CAM_BNR_D_BPC_TH2, th2);
        GET_REG_INFO(CAM_BNR_D_BPC_TH3, th3);
        GET_REG_INFO(CAM_BNR_D_BPC_TH4, th4);
        GET_REG_INFO(CAM_BNR_D_BPC_DTC, dtc);
        GET_REG_INFO(CAM_BNR_D_BPC_COR, cor);
        //GET_REG_INFO(CAM_BPC_D_TBLI1, tbli1);
        //GET_REG_INFO(CAM_BPC_D_TBLI2, tbli2);
        GET_REG_INFO(CAM_BNR_D_BPC_TH1_C, th1_c);
        GET_REG_INFO(CAM_BNR_D_BPC_TH2_C, th2_c);
        GET_REG_INFO(CAM_BNR_D_BPC_TH3_C, th3_c);
    }

    return  (*this);
}


template <>
ISP_MGR_BNR_T&
ISP_MGR_BNR_T::
put(ISP_NVRAM_BNR_NR1_T const& rParam)
{
    if (m_eSensorTG == ESensorTG_1) {
        PUT_REG_INFO(CAM_BNR_NR1_CON, con);
        PUT_REG_INFO(CAM_BNR_NR1_CT_CON, ct_con);
    }
    else {
        PUT_REG_INFO(CAM_BNR_D_NR1_CON, con);
        PUT_REG_INFO(CAM_BNR_D_NR1_CT_CON, ct_con);
    }


    return  (*this);
}


template <>
ISP_MGR_BNR_T&
ISP_MGR_BNR_T::
get(ISP_NVRAM_BNR_NR1_T& rParam)
{
    if (m_eSensorTG == ESensorTG_1) {
        GET_REG_INFO(CAM_BNR_NR1_CON, con);
        GET_REG_INFO(CAM_BNR_NR1_CT_CON, ct_con);
    }
    else {
        GET_REG_INFO(CAM_BNR_D_NR1_CON, con);
        GET_REG_INFO(CAM_BNR_D_NR1_CT_CON, ct_con);
    }

    return  (*this);
}

template <>
ISP_MGR_BNR_T&
ISP_MGR_BNR_T::
put(ISP_NVRAM_BNR_PDC_T const& rParam)
{
    if (m_eSensorTG == ESensorTG_1) {
        PUT_REG_INFO(CAM_BNR_PDC_CON,     con);
        PUT_REG_INFO(CAM_BNR_PDC_GAIN_L0, gain_l0);
        PUT_REG_INFO(CAM_BNR_PDC_GAIN_L1, gain_l1);
        PUT_REG_INFO(CAM_BNR_PDC_GAIN_L2, gain_l2);
        PUT_REG_INFO(CAM_BNR_PDC_GAIN_L3, gain_l3);
        PUT_REG_INFO(CAM_BNR_PDC_GAIN_L4, gain_l4);
        PUT_REG_INFO(CAM_BNR_PDC_GAIN_R0, gain_r0);
        PUT_REG_INFO(CAM_BNR_PDC_GAIN_R1, gain_r1);
        PUT_REG_INFO(CAM_BNR_PDC_GAIN_R2, gain_r2);
        PUT_REG_INFO(CAM_BNR_PDC_GAIN_R3, gain_r3);
        PUT_REG_INFO(CAM_BNR_PDC_GAIN_R4, gain_r4);
        PUT_REG_INFO(CAM_BNR_PDC_TH_GB,   th_gb);
        PUT_REG_INFO(CAM_BNR_PDC_TH_IA,   th_ia);
        PUT_REG_INFO(CAM_BNR_PDC_TH_HD,   th_hd);
        PUT_REG_INFO(CAM_BNR_PDC_SL,      sl);
        PUT_REG_INFO(CAM_BNR_PDC_POS,     pos);
    }
    else {
        PUT_REG_INFO(CAM_BNR_D_PDC_CON,     con);
        PUT_REG_INFO(CAM_BNR_D_PDC_GAIN_L0, gain_l0);
        PUT_REG_INFO(CAM_BNR_D_PDC_GAIN_L1, gain_l1);
        PUT_REG_INFO(CAM_BNR_D_PDC_GAIN_L2, gain_l2);
        PUT_REG_INFO(CAM_BNR_D_PDC_GAIN_L3, gain_l3);
        PUT_REG_INFO(CAM_BNR_D_PDC_GAIN_L4, gain_l4);
        PUT_REG_INFO(CAM_BNR_D_PDC_GAIN_R0, gain_r0);
        PUT_REG_INFO(CAM_BNR_D_PDC_GAIN_R1, gain_r1);
        PUT_REG_INFO(CAM_BNR_D_PDC_GAIN_R2, gain_r2);
        PUT_REG_INFO(CAM_BNR_D_PDC_GAIN_R3, gain_r3);
        PUT_REG_INFO(CAM_BNR_D_PDC_GAIN_R4, gain_r4);
        PUT_REG_INFO(CAM_BNR_D_PDC_TH_GB,   th_gb);
        PUT_REG_INFO(CAM_BNR_D_PDC_TH_IA,   th_ia);
        PUT_REG_INFO(CAM_BNR_D_PDC_TH_HD,   th_hd);
        PUT_REG_INFO(CAM_BNR_D_PDC_SL,      sl);
        PUT_REG_INFO(CAM_BNR_D_PDC_POS,     pos);

    }

    return  (*this);
}


template <>
ISP_MGR_BNR_T&
ISP_MGR_BNR_T::
get(ISP_NVRAM_BNR_PDC_T& rParam)
{
    if (m_eSensorTG == ESensorTG_1) {
        GET_REG_INFO(CAM_BNR_PDC_CON,     con);
        GET_REG_INFO(CAM_BNR_PDC_GAIN_L0, gain_l0);
        GET_REG_INFO(CAM_BNR_PDC_GAIN_L1, gain_l1);
        GET_REG_INFO(CAM_BNR_PDC_GAIN_L2, gain_l2);
        GET_REG_INFO(CAM_BNR_PDC_GAIN_L3, gain_l3);
        GET_REG_INFO(CAM_BNR_PDC_GAIN_L4, gain_l4);
        GET_REG_INFO(CAM_BNR_PDC_GAIN_R0, gain_r0);
        GET_REG_INFO(CAM_BNR_PDC_GAIN_R1, gain_r1);
        GET_REG_INFO(CAM_BNR_PDC_GAIN_R2, gain_r2);
        GET_REG_INFO(CAM_BNR_PDC_GAIN_R3, gain_r3);
        GET_REG_INFO(CAM_BNR_PDC_GAIN_R4, gain_r4);
        GET_REG_INFO(CAM_BNR_PDC_TH_GB,   th_gb);
        GET_REG_INFO(CAM_BNR_PDC_TH_IA,   th_ia);
        GET_REG_INFO(CAM_BNR_PDC_TH_HD,   th_hd);
        GET_REG_INFO(CAM_BNR_PDC_SL,      sl);
        GET_REG_INFO(CAM_BNR_PDC_POS,     pos);
    }
    else {
        GET_REG_INFO(CAM_BNR_D_PDC_CON,     con);
        GET_REG_INFO(CAM_BNR_D_PDC_GAIN_L0, gain_l0);
        GET_REG_INFO(CAM_BNR_D_PDC_GAIN_L1, gain_l1);
        GET_REG_INFO(CAM_BNR_D_PDC_GAIN_L2, gain_l2);
        GET_REG_INFO(CAM_BNR_D_PDC_GAIN_L3, gain_l3);
        GET_REG_INFO(CAM_BNR_D_PDC_GAIN_L4, gain_l4);
        GET_REG_INFO(CAM_BNR_D_PDC_GAIN_R0, gain_r0);
        GET_REG_INFO(CAM_BNR_D_PDC_GAIN_R1, gain_r1);
        GET_REG_INFO(CAM_BNR_D_PDC_GAIN_R2, gain_r2);
        GET_REG_INFO(CAM_BNR_D_PDC_GAIN_R3, gain_r3);
        GET_REG_INFO(CAM_BNR_D_PDC_GAIN_R4, gain_r4);
        GET_REG_INFO(CAM_BNR_D_PDC_TH_GB,   th_gb);
        GET_REG_INFO(CAM_BNR_D_PDC_TH_IA,   th_ia);
        GET_REG_INFO(CAM_BNR_D_PDC_TH_HD,   th_hd);
        GET_REG_INFO(CAM_BNR_D_PDC_SL,      sl);
        GET_REG_INFO(CAM_BNR_D_PDC_POS,     pos);

    }

    return  (*this);
}

MBOOL
ISP_MGR_BNR_T::
apply(EIspProfile_T eIspProfile)
{
    addressErrorCheck("Before ISP_MGR_BNR_T::apply()");

    if (m_eSensorTG == ESensorTG_1) {
        MBOOL bBPC_EN = reinterpret_cast<ISP_CAM_BNR_BPC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_BPC_CON))->BPC_EN & isBPCEnable();
        MBOOL bCT_EN  = reinterpret_cast<ISP_CAM_BNR_NR1_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_NR1_CON))->NR1_CT_EN & isCTEnable();
        MBOOL bPDC_EN = reinterpret_cast<ISP_CAM_BNR_PDC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_PDC_CON))->PDC_EN & isPDCEnable() & m_bBPCIValid;

        MBOOL bBNR_EN = bPDC_EN | bBPC_EN | bCT_EN;

        //BNR HW limitation
        if (bBNR_EN && (bBPC_EN == 0))
        {
            bBNR_EN = 0;
            bCT_EN = 0;
            bPDC_EN = 0;
            MY_LOG("BNR::apply warning: set (bBNR_EN, bBPC_EN, bCT_EN) = (%d, %d, %d)", bBNR_EN, bBPC_EN, bCT_EN);
        }

        ISP_MGR_CTL_EN_P1_T::getInstance(m_eSensorDev).setEnable_BNR(bBNR_EN);

        reinterpret_cast<ISP_CAM_BNR_BPC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_BPC_CON))->BPC_EN    = bBPC_EN;
        reinterpret_cast<ISP_CAM_BNR_NR1_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_NR1_CON))->NR1_CT_EN = bCT_EN;
        reinterpret_cast<ISP_CAM_BNR_PDC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_PDC_CON))->PDC_EN    = bPDC_EN;

        //PDC HW limitation
        if(reinterpret_cast<ISP_CAM_BNR_PDC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_PDC_CON))->PDC_EN){
            // PDAF SW limitation
            reinterpret_cast<ISP_CAM_BNR_BPC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_BPC_CON))->BPC_LUT_EN = 1;
            // BPCI size setting
            //reinterpret_cast<ISP_CAM_BNR_BPC_TBLI2_T*>(REG_INFO_VALUE_PTR(CAM_BNR_BPC_TBLI2))->BPC_XSIZE = rRawIspCamInfo.BinInfo.u4BIN_Width;
            //reinterpret_cast<ISP_CAM_BNR_BPC_TBLI2_T*>(REG_INFO_VALUE_PTR(CAM_BNR_BPC_TBLI2))->BPC_YSIZE = rRawIspCamInfo.BinInfo.u4BIN_Height;
            REG_INFO_VALUE(CAM_BPCI_BASE_ADDR) = m_sPDOHWInfo.phyAddrBpci_tbl;
            REG_INFO_VALUE(CAM_BPCI_XSIZE)     = m_sPDOHWInfo.u4Bpci_xsz+1;
            REG_INFO_VALUE(CAM_BPCI_YSIZE)     = 1;
            REG_INFO_VALUE(CAM_BPCI_STRIDE)    = m_sPDOHWInfo.u4Bpci_xsz+1;

        }
        else{
            reinterpret_cast<ISP_CAM_BNR_BPC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_BPC_CON))->BPC_LUT_EN = 0;
        }

        //PDC out
        reinterpret_cast<ISP_CAM_BNR_PDC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_PDC_CON))->PDC_OUT = 0;

        // HW constraint
        reinterpret_cast<ISP_CAM_BNR_BPC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_BPC_CON))->BNR_EDGE = 15;

        TuningMgr::getInstance().updateEngine(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]),
                                               eTuningMgrFunc_Bnr);

        // TOP
        TUNING_MGR_WRITE_ENABLE_BITS(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]), CAM_CTL_EN_P1, BNR_EN, bBNR_EN);

        // Register setting
        TuningMgr::getInstance().tuningMgrWriteRegs(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]),
                                                     static_cast<TUNING_MGR_REG_IO_STRUCT*>(m_pRegInfo),
                                                     m_u4RegInfoNum);
    }
    else {
        MBOOL bBPC_EN = reinterpret_cast<ISP_CAM_BNR_D_BPC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_D_BPC_CON))->BPC_EN & isBPCEnable();
        MBOOL bCT_EN  = reinterpret_cast<ISP_CAM_BNR_D_NR1_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_D_NR1_CON))->NR1_CT_EN & isCTEnable();
        MBOOL bPDC_EN = reinterpret_cast<ISP_CAM_BNR_D_PDC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_D_PDC_CON))->PDC_EN & isPDCEnable() & m_bBPCIValid;
        MBOOL bBNR_EN = bPDC_EN | bBPC_EN | bCT_EN;

        //BNR HW limitation
        if (bBNR_EN && (bBPC_EN == 0))
        {
            bBNR_EN = 0;
            bCT_EN = 0;
            bPDC_EN = 0;
            MY_LOG("BNR::apply warning: set (bBNR_EN, bBPC_EN, bCT_EN) = (%d, %d, %d)", bBNR_EN, bBPC_EN, bCT_EN);
        }

        ISP_MGR_CTL_EN_P1_D_T::getInstance(m_eSensorDev).setEnable_BNR(bBNR_EN);

        reinterpret_cast<ISP_CAM_BNR_D_BPC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_D_BPC_CON))->BPC_EN    = bBPC_EN;
        reinterpret_cast<ISP_CAM_BNR_D_NR1_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_D_NR1_CON))->NR1_CT_EN = bCT_EN;
        reinterpret_cast<ISP_CAM_BNR_D_PDC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_D_PDC_CON))->PDC_EN    = bPDC_EN;

        //PDC HW limitation
        if(reinterpret_cast<ISP_CAM_BNR_D_PDC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_D_PDC_CON))->PDC_EN){
            // PDAF SW limitation
            reinterpret_cast<ISP_CAM_BNR_D_BPC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_D_BPC_CON))->BPC_LUT_EN = 1;
            // BPCI size setting
            //reinterpret_cast<ISP_CAM_BNR_D_BPC_TBLI2_T*>(REG_INFO_VALUE_PTR(CAM_BNR_D_BPC_TBLI2))->BPC_XSIZE = rRawIspCamInfo.BinInfo.u4BIN_Width;
            //reinterpret_cast<ISP_CAM_BNR_D_BPC_TBLI2_T*>(REG_INFO_VALUE_PTR(CAM_BNR_D_BPC_TBLI2))->BPC_YSIZE = rRawIspCamInfo.BinInfo.u4BIN_Height;
            REG_INFO_VALUE(CAM_BPCI_D_BASE_ADDR) = m_sPDOHWInfo.phyAddrBpci_tbl;
            REG_INFO_VALUE(CAM_BPCI_D_XSIZE)     = m_sPDOHWInfo.u4Bpci_xsz+1;
            REG_INFO_VALUE(CAM_BPCI_D_YSIZE)     = 1;
            REG_INFO_VALUE(CAM_BPCI_D_STRIDE)    = m_sPDOHWInfo.u4Bpci_xsz+1;
        }
        else{
            reinterpret_cast<ISP_CAM_BNR_D_BPC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_D_BPC_CON))->BPC_LUT_EN = 0;
        }

        //PDC out
        reinterpret_cast<ISP_CAM_BNR_D_PDC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_D_PDC_CON))->PDC_OUT = 0;

        // HW constraint
        reinterpret_cast<ISP_CAM_BNR_D_BPC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_D_BPC_CON))->BNR_EDGE = 15;

        TuningMgr::getInstance().updateEngine(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]),
                                               eTuningMgrFunc_Bnr_d);

        // TOP
        TUNING_MGR_WRITE_ENABLE_BITS(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]), CAM_CTL_EN_P1_D, BNR_D_EN, bBNR_EN);

        // Register setting
        TuningMgr::getInstance().tuningMgrWriteRegs(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]),
                                                     static_cast<TUNING_MGR_REG_IO_STRUCT*>(m_pRegInfo),
                                                     m_u4RegInfoNum);
    }

    dumpRegInfo("BNR");

    addressErrorCheck("After ISP_MGR_BNR_T::apply()");

    return  MTRUE;
}

MBOOL
ISP_MGR_BNR_T::
getBPCItable(
    UINT32& bpci_xsize,
    UINT32& bpci_ysize,
    UINT32& pdo_xsize,
    UINT32& pdo_ysize,
    MUINTPTR& phy_addr,
    MUINTPTR& vir_addr,
    MINT32& memID,
    EIndex_PDC_TBL_T Type){

    switch (Type)
    {
    case eIDX_PDC_FULL:
        {
            if(m_pPDCBuffer_1 != NULL) {
                bpci_xsize = m_pBpciBuf_1->bpci_xsize;
                bpci_ysize = m_pBpciBuf_1->bpci_ysize;
                pdo_xsize = m_pBpciBuf_1->pdo_xsize;
                pdo_ysize = m_pBpciBuf_1->pdo_ysize;
                phy_addr = static_cast<IImageBuffer*>(m_pPDCBuffer_1)->getBufPA(0);
                vir_addr = static_cast<IImageBuffer*>(m_pPDCBuffer_1)->getBufVA(0);
                memID = static_cast<IImageBuffer*>(m_pPDCBuffer_1)->getFD(0);
                return MTRUE;
            }
            else
                return MFALSE;
        }
    case eIDX_PDC_RSV:
        {
            if(m_pPDCBuffer_2 != NULL) {
                bpci_xsize = m_pBpciBuf_2->bpci_xsize;
                bpci_ysize = m_pBpciBuf_2->bpci_ysize;
                pdo_xsize = m_pBpciBuf_2->pdo_xsize;
                pdo_ysize = m_pBpciBuf_2->pdo_ysize;
                phy_addr = static_cast<IImageBuffer*>(m_pPDCBuffer_2)->getBufPA(0);
                vir_addr = static_cast<IImageBuffer*>(m_pPDCBuffer_2)->getBufVA(0);
                memID = static_cast<IImageBuffer*>(m_pPDCBuffer_2)->getFD(0);
                return MTRUE;
            }
            else
                return MFALSE;
        }
    default:
        return MFALSE;
    }

}

MBOOL
ISP_MGR_BNR_T::
setBPCIBuf(CAMERA_BPCI_STRUCT* buf)
{

    m_pBpciBuf_1 = &(buf->PDC_FULL_TBL);
    m_pBpciBuf_2 = &(buf->PDC_RSV_TBL);

    if (m_pBpciBuf_1 == NULL ||
        m_pBpciBuf_2 == NULL)
        return MFALSE;

    MUINT32 x_size_pdc_1 = m_pBpciBuf_1->bpci_xsize;
    MUINT32 x_size_pdc_2 = m_pBpciBuf_2->bpci_xsize;
    MUINT32 y_size = 1;
    // create buffer
    MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
    MUINT32 bufStridesInBytes_pdc_1[3] = {x_size_pdc_1, 0, 0};
    MUINT32 bufStridesInBytes_pdc_2[3] = {x_size_pdc_2, 0, 0};

    std::string strName_pdc_1  = "PDC_tbl_1";
    std::string strName_pdc_2  = "PDC_tbl_2";

    IImageBufferAllocator::ImgParam imgParam_pdc_1 =
        IImageBufferAllocator::ImgParam((EImageFormat)eImgFmt_STA_BYTE,
            MSize(x_size_pdc_1, y_size), bufStridesInBytes_pdc_1, bufBoundaryInBytes, 1);
    IImageBufferAllocator::ImgParam imgParam_pdc_2 =
        IImageBufferAllocator::ImgParam((EImageFormat)eImgFmt_STA_BYTE,
            MSize(x_size_pdc_2, y_size), bufStridesInBytes_pdc_2, bufBoundaryInBytes, 1);

    sp<IIonImageBufferHeap> pHeap_pdc_1 =
        IIonImageBufferHeap::create(strName_pdc_1.c_str(), imgParam_pdc_1);
    sp<IIonImageBufferHeap> pHeap_pdc_2 =
        IIonImageBufferHeap::create(strName_pdc_2.c_str(), imgParam_pdc_2);

    if (pHeap_pdc_1 == NULL) {
        CAM_LOGE("[%s] ImageBufferHeap create fail", strName_pdc_1.c_str());
        return MFALSE;
    }
    if (pHeap_pdc_2 == NULL) {
        CAM_LOGE("[%s] ImageBufferHeap create fail", strName_pdc_2.c_str());
        return MFALSE;
    }

    IImageBuffer* pImgBuf_pdc_1  = pHeap_pdc_1->createImageBuffer();
    IImageBuffer* pImgBuf_pdc_2  = pHeap_pdc_2->createImageBuffer();

    if (pImgBuf_pdc_1 == NULL) {
        CAM_LOGE("[%s] ImageBufferHeap create fail", strName_pdc_1.c_str());
        return MFALSE;
    }
    if (pImgBuf_pdc_2 == NULL) {
        CAM_LOGE("[%s] ImageBufferHeap create fail", strName_pdc_2.c_str());
        return MFALSE;
    }

    // lock buffer
    MUINT const usage_pdc_1 = (GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN |
                        GRALLOC_USAGE_HW_CAMERA_READ |
                        GRALLOC_USAGE_HW_CAMERA_WRITE);
    MUINT const usage_pdc_2 = (GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN |
                        GRALLOC_USAGE_HW_CAMERA_READ |
                        GRALLOC_USAGE_HW_CAMERA_WRITE);

    if (!(pImgBuf_pdc_1->lockBuf(strName_pdc_1.c_str(), usage_pdc_1)))
    {
        CAM_LOGE("[%s] Stuff ImageBuffer lock fail",  strName_pdc_1.c_str());
        return MFALSE;
    }
    if (!(pImgBuf_pdc_2->lockBuf(strName_pdc_2.c_str(), usage_pdc_2)))
    {
        CAM_LOGE("[%s] Stuff ImageBuffer lock fail",  strName_pdc_2.c_str());
        return MFALSE;
    }

    // release m_pPDCBuffer
    if (m_pPDCBuffer_1){
        sp<IImageBuffer> temp = static_cast<IImageBuffer*>(m_pPDCBuffer_1);
    }
    if (m_pPDCBuffer_2){
        sp<IImageBuffer> temp = static_cast<IImageBuffer*>(m_pPDCBuffer_2);
    }

    memcpy(reinterpret_cast<MVOID*>(pImgBuf_pdc_1->getBufVA(0)), m_pBpciBuf_1->bpci_array, m_pBpciBuf_1->bpci_xsize);
    memcpy(reinterpret_cast<MVOID*>(pImgBuf_pdc_2->getBufVA(0)), m_pBpciBuf_2->bpci_array, m_pBpciBuf_2->bpci_xsize);

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("dump.bpci_tbl.enable", value, "0"); // 0: enable, 1: disable
    MBOOL bDumpTblEnable = atoi(value);


    m_pPDCBuffer_1    = static_cast<MVOID*>(pImgBuf_pdc_1);
    m_pPDCBuffer_2    = static_cast<MVOID*>(pImgBuf_pdc_2);

    return MTRUE;
}


MVOID
ISP_MGR_BNR_T::
unlockBPCIBuf()
{
    std::string strName_pdc_1 = "PDC_tbl_1";  //same in setPBCIBuf
    std::string strName_pdc_2 = "PDC_tbl_2";  //same in setPBCIBuf

    if (m_pPDCBuffer_1 != NULL)
    {
        static_cast<IImageBuffer*>(m_pPDCBuffer_1)->unlockBuf(strName_pdc_1.c_str());
        // destroy buffer
        sp<IImageBuffer> temp_pdc_1 = static_cast<IImageBuffer*>(m_pPDCBuffer_1);
        m_pPDCBuffer_1 = NULL;
    }

    if (m_pPDCBuffer_2 != NULL)
    {
        static_cast<IImageBuffer*>(m_pPDCBuffer_2)->unlockBuf(strName_pdc_2.c_str());
        // destroy buffer
        sp<IImageBuffer> temp_pdc_2 = static_cast<IImageBuffer*>(m_pPDCBuffer_2);
        m_pPDCBuffer_2 = NULL;
    }


}


}

