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
#include <mtkcam/utils/std/Log.h>
#include "isp_mgr.h"
#include <string>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/imgbuf/IIonImageBufferHeap.h>
#include <private/aaa_utils.h>
#include <array>

using namespace NSCam;
using namespace NSIspTuningv3;

#define MY_INST_BNR NS3Av3::INST_T<ISP_MGR_BNR_T>
static std::array<MY_INST_BNR, SENSOR_IDX_MAX> gMultitonBNR;

#define MY_INST_BNR2 NS3Av3::INST_T<ISP_MGR_BNR2_T>
static std::array<MY_INST_BNR2, SENSOR_IDX_MAX> gMultitonBNR2;

namespace NSIspTuningv3
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BNR
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_BNR_T&
ISP_MGR_BNR_T::
getInstance(ESensorDev_T const eSensorDev)
{
    auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(eSensorDev);

    MY_ASSERT(i4SensorIdx < SENSOR_IDX_MAX);
    MY_ASSERT(i4SensorIdx >= 0);

    MY_INST_BNR& rSingleton = gMultitonBNR[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<ISP_MGR_BNR_T>(eSensorDev);
    } );

    return *(rSingleton.instance);
}

template <>
ISP_MGR_BNR_T&
ISP_MGR_BNR_T::
put(ISP_NVRAM_BNR_BPC_T const& rParam)
{
    PUT_REG_INFO(CAM_BNR_BPC_CON, con);
    PUT_REG_INFO(CAM_BNR_BPC_TH1, th1);
    PUT_REG_INFO(CAM_BNR_BPC_TH2, th2);
    PUT_REG_INFO(CAM_BNR_BPC_TH3, th3);
    PUT_REG_INFO(CAM_BNR_BPC_TH4, th4);
    PUT_REG_INFO(CAM_BNR_BPC_DTC, dtc);
    PUT_REG_INFO(CAM_BNR_BPC_COR, cor);
    PUT_REG_INFO(CAM_BNR_BPC_TBLI1, tbli1);
    PUT_REG_INFO(CAM_BNR_BPC_TH1_C, th1_c);
    PUT_REG_INFO(CAM_BNR_BPC_TH2_C, th2_c);
    PUT_REG_INFO(CAM_BNR_BPC_TH3_C, th3_c);
    return  (*this);
}


template <>
ISP_MGR_BNR_T&
ISP_MGR_BNR_T::
get(ISP_NVRAM_BNR_BPC_T& rParam)
{
    GET_REG_INFO(CAM_BNR_BPC_CON, con);
    GET_REG_INFO(CAM_BNR_BPC_TH1, th1);
    GET_REG_INFO(CAM_BNR_BPC_TH2, th2);
    GET_REG_INFO(CAM_BNR_BPC_TH3, th3);
    GET_REG_INFO(CAM_BNR_BPC_TH4, th4);
    GET_REG_INFO(CAM_BNR_BPC_DTC, dtc);
    GET_REG_INFO(CAM_BNR_BPC_COR, cor);
    GET_REG_INFO(CAM_BNR_BPC_TBLI1, tbli1);
    GET_REG_INFO(CAM_BNR_BPC_TH1_C, th1_c);
    GET_REG_INFO(CAM_BNR_BPC_TH2_C, th2_c);
    GET_REG_INFO(CAM_BNR_BPC_TH3_C, th3_c);

    return  (*this);
}


template <>
ISP_MGR_BNR_T&
ISP_MGR_BNR_T::
put(ISP_NVRAM_BNR_NR1_T const& rParam)
{
    PUT_REG_INFO(CAM_BNR_NR1_CON, con);
    PUT_REG_INFO(CAM_BNR_NR1_CT_CON, ct_con);

    return  (*this);
}


template <>
ISP_MGR_BNR_T&
ISP_MGR_BNR_T::
get(ISP_NVRAM_BNR_NR1_T& rParam)
{
    GET_REG_INFO(CAM_BNR_NR1_CON, con);
    GET_REG_INFO(CAM_BNR_NR1_CT_CON, ct_con);

    return  (*this);
}

template <>
ISP_MGR_BNR_T&
ISP_MGR_BNR_T::
put(ISP_NVRAM_BNR_PDC_T const& rParam)
{
    PUT_REG_INFO(CAM_BNR_PDC_CON, con);
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
    PUT_REG_INFO(CAM_BNR_PDC_TH_GB, th_gb);
    PUT_REG_INFO(CAM_BNR_PDC_TH_IA, th_ia);
    PUT_REG_INFO(CAM_BNR_PDC_TH_HD, th_hd);
    PUT_REG_INFO(CAM_BNR_PDC_SL, sl);
    PUT_REG_INFO(CAM_BNR_PDC_POS, pos);

    return  (*this);
}


template <>
ISP_MGR_BNR_T&
ISP_MGR_BNR_T::
get(ISP_NVRAM_BNR_PDC_T& rParam)
{
    GET_REG_INFO(CAM_BNR_PDC_CON, con);
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
    GET_REG_INFO(CAM_BNR_PDC_TH_GB, th_gb);
    GET_REG_INFO(CAM_BNR_PDC_TH_IA, th_ia);
    GET_REG_INFO(CAM_BNR_PDC_TH_HD, th_hd);
    GET_REG_INFO(CAM_BNR_PDC_SL, sl);
    GET_REG_INFO(CAM_BNR_PDC_POS, pos);

    return  (*this);
}

MBOOL
ISP_MGR_BNR_T::
apply(RAWIspCamInfo& rRawIspCamInfo, TuningMgr& rTuning, MINT32 i4SubsampleIdex)
{
    MBOOL bBPC_EN = (isBPCEnable() & (reinterpret_cast<ISP_CAM_BNR_BPC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_BPC_CON))->BPC_EN));
    MBOOL bCT_EN  = (isCTEnable()  & (reinterpret_cast<ISP_CAM_BNR_NR1_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_NR1_CON))->NR1_CT_EN));

    memset(&m_sPDOHWInfo, 0, sizeof(SPDOHWINFO_T));


    EIndex_PDC_TBL_T ePDC_table_type = eIDX_PDC_FULL;

    if(rRawIspCamInfo.BinInfo.fgBIN){
        ePDC_table_type = eIDX_PDC_BIN;
    }

    m_bBPCIValid = ISP_MGR_BNR2::getInstance(m_eSensorDev).getBPCItable(
                                                       m_sPDOHWInfo.u4Bpci_xsz,
                                                       m_sPDOHWInfo.u4Bpci_ysz,
                                                       m_sPDOHWInfo.u4Pdo_xsz,
                                                       m_sPDOHWInfo.u4Pdo_ysz,
                                                       m_sPDOHWInfo.phyAddrBpci_tbl,
                                                       m_sPDOHWInfo.virAddrBpci_tbl,
                                                       m_sPDOHWInfo.i4memID,
                                                       ePDC_table_type);
    MBOOL bPDC_EN =  m_bBPCIValid &&
                    (isAF_PDCEnable() |(isPDCEnable()&& (reinterpret_cast<ISP_CAM_BNR_PDC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_PDC_CON))->PDC_EN)));


    MBOOL bBNR_EN = (bBPC_EN | bCT_EN | bPDC_EN);

    setPDCEnable(bPDC_EN);


    //BNR HW limitation
    if (bBNR_EN && (bBPC_EN == 0))
    {
        bBNR_EN = 0;
        bCT_EN = 0;
        MY_LOG("BNR::apply warning: set (bBNR_EN, bBPC_EN, bCT_EN) = (%d, %d, %d)", bBNR_EN, bBPC_EN, bCT_EN);
    }

    reinterpret_cast<ISP_CAM_BNR_BPC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_BPC_CON))->BPC_EN = bBPC_EN;
    reinterpret_cast<ISP_CAM_BNR_NR1_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_NR1_CON))->NR1_CT_EN = bCT_EN;
    reinterpret_cast<ISP_CAM_BNR_PDC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_PDC_CON))->PDC_EN = bPDC_EN;

    //PDC HW limitation
    if(reinterpret_cast<ISP_CAM_BNR_PDC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_PDC_CON))->PDC_EN){
        // PDAF SW limitation
        reinterpret_cast<ISP_CAM_BNR_BPC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_BPC_CON))->BPC_LUT_EN = 1;
        // BPCI size setting
        reinterpret_cast<ISP_CAM_BNR_BPC_TBLI2_T*>(REG_INFO_VALUE_PTR(CAM_BNR_BPC_TBLI2))->BPC_XSIZE = rRawIspCamInfo.BinInfo.u4BIN_Width;
        reinterpret_cast<ISP_CAM_BNR_BPC_TBLI2_T*>(REG_INFO_VALUE_PTR(CAM_BNR_BPC_TBLI2))->BPC_YSIZE = rRawIspCamInfo.BinInfo.u4BIN_Height;
        //PDC out
        reinterpret_cast<ISP_CAM_BNR_PDC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_PDC_CON))->PDC_OUT = isAF_PDCEnable();

        REG_INFO_VALUE(CAM_BPCI_BASE_ADDR) = m_sPDOHWInfo.phyAddrBpci_tbl;
        REG_INFO_VALUE(CAM_BPCI_XSIZE) = m_sPDOHWInfo.u4Bpci_xsz+1;
        REG_INFO_VALUE(CAM_BPCI_YSIZE) = 1;
        REG_INFO_VALUE(CAM_BPCI_STRIDE) =  m_sPDOHWInfo.u4Bpci_xsz+1;
    }
    else{
        reinterpret_cast<ISP_CAM_BNR_BPC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_BPC_CON))->BPC_LUT_EN = 0;
        reinterpret_cast<ISP_CAM_BNR_PDC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_PDC_CON))->PDC_OUT = 0;
    }

    rTuning.updateEngine(eTuningMgrFunc_BNR, bBNR_EN, i4SubsampleIdex);


   if(bPDC_EN)
        rTuning.updateEngineFD(eTuningMgrFunc_BNR, 0, m_sPDOHWInfo.i4memID, m_sPDOHWInfo.virAddrBpci_tbl);

    // TOP
    TUNING_MGR_WRITE_BITS_CAM((&rTuning), CAM_CTL_EN, BNR_EN, bBNR_EN, i4SubsampleIdex);
    ISP_MGR_CTL_EN_P1_T::getInstance(m_eSensorDev).setEnable_BNR(bBNR_EN);

    // Register setting
    rTuning.tuningMgrWriteRegs(static_cast<TUNING_MGR_REG_IO_STRUCT*>(m_pRegInfo),
                             m_u4RegInfoNum, i4SubsampleIdex);

    dumpRegInfoP1("BNR");

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BNR2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_BNR2_T&
ISP_MGR_BNR2_T::
getInstance(ESensorDev_T const eSensorDev)
{
    auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(eSensorDev);

    MY_ASSERT(i4SensorIdx < SENSOR_IDX_MAX);
    MY_ASSERT(i4SensorIdx >= 0);

    MY_INST_BNR2& rSingleton = gMultitonBNR2[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<ISP_MGR_BNR2_T>(eSensorDev);
    } );

    return *(rSingleton.instance);
}

ISP_MGR_BNR2::~ISP_MGR_BNR2()
{
    if (m_pPDE_Buffer != nullptr) {
        // using sp to release IImageBuffer
        sp<IImageBuffer> temp_pde = static_cast<IImageBuffer*>(m_pPDE_Buffer);
    }

    if (m_pPDC_FULL_Buffer != nullptr) {
        // using sp to release IImageBuffer
        sp<IImageBuffer> temp_pdc_full = static_cast<IImageBuffer*>(m_pPDC_FULL_Buffer);
    }

    if (m_pPDC_BIN_Buffer != nullptr) {
        // using sp to release IImageBuffer
        sp<IImageBuffer> temp_pdc_bin = static_cast<IImageBuffer*>(m_pPDC_BIN_Buffer);
    }

}

template <>
ISP_MGR_BNR2_T&
ISP_MGR_BNR2_T::
put(ISP_NVRAM_BNR_BPC_T const& rParam)
{
    PUT_REG_INFO(DIP_X_BNR2_BPC_CON, con);
    PUT_REG_INFO(DIP_X_BNR2_BPC_TH1, th1);
    PUT_REG_INFO(DIP_X_BNR2_BPC_TH2, th2);
    PUT_REG_INFO(DIP_X_BNR2_BPC_TH3, th3);
    PUT_REG_INFO(DIP_X_BNR2_BPC_TH4, th4);
    PUT_REG_INFO(DIP_X_BNR2_BPC_DTC, dtc);
    PUT_REG_INFO(DIP_X_BNR2_BPC_COR, cor);
    PUT_REG_INFO(DIP_X_BNR2_BPC_TBLI1, tbli1);
    PUT_REG_INFO(DIP_X_BNR2_BPC_TH1_C, th1_c);
    PUT_REG_INFO(DIP_X_BNR2_BPC_TH2_C, th2_c);
    PUT_REG_INFO(DIP_X_BNR2_BPC_TH3_C, th3_c);

    return  (*this);
}

template <>
MBOOL
ISP_MGR_BNR2_T::
get(ISP_NVRAM_BNR_BPC_T& rParam, const dip_x_reg_t* pReg) const
{
    if (pReg)
    {
        GET_REG_INFO_BUF(DIP_X_BNR2_BPC_CON, con);
        GET_REG_INFO_BUF(DIP_X_BNR2_BPC_TH1, th1);
        GET_REG_INFO_BUF(DIP_X_BNR2_BPC_TH2, th2);
        GET_REG_INFO_BUF(DIP_X_BNR2_BPC_TH3, th3);
        GET_REG_INFO_BUF(DIP_X_BNR2_BPC_TH4, th4);
        GET_REG_INFO_BUF(DIP_X_BNR2_BPC_DTC, dtc);
        GET_REG_INFO_BUF(DIP_X_BNR2_BPC_COR, cor);
        GET_REG_INFO_BUF(DIP_X_BNR2_BPC_TBLI1, tbli1);
        GET_REG_INFO_BUF(DIP_X_BNR2_BPC_TH1_C, th1_c);
        GET_REG_INFO_BUF(DIP_X_BNR2_BPC_TH2_C, th2_c);
        GET_REG_INFO_BUF(DIP_X_BNR2_BPC_TH3_C, th3_c);
    }
    return MTRUE;
}

template <>
ISP_MGR_BNR2_T&
ISP_MGR_BNR2_T::
put(ISP_NVRAM_BNR_NR1_T const& rParam)
{
    PUT_REG_INFO(DIP_X_BNR2_NR1_CON, con);
    PUT_REG_INFO(DIP_X_BNR2_NR1_CT_CON, ct_con);


    return  (*this);
}

template <>
MBOOL
ISP_MGR_BNR2_T::
get(ISP_NVRAM_BNR_NR1_T& rParam, const dip_x_reg_t* pReg) const
{
    if (pReg)
    {
        GET_REG_INFO_BUF(DIP_X_BNR2_NR1_CON, con);
        GET_REG_INFO_BUF(DIP_X_BNR2_NR1_CT_CON, ct_con);
    }
    return MTRUE;
}

template <>
ISP_MGR_BNR2_T&
ISP_MGR_BNR2_T::
put(ISP_NVRAM_BNR_PDC_T const& rParam)
{
    PUT_REG_INFO(DIP_X_BNR2_PDC_CON, con);
    PUT_REG_INFO(DIP_X_BNR2_PDC_GAIN_L0, gain_l0);
    PUT_REG_INFO(DIP_X_BNR2_PDC_GAIN_L1, gain_l1);
    PUT_REG_INFO(DIP_X_BNR2_PDC_GAIN_L2, gain_l2);
    PUT_REG_INFO(DIP_X_BNR2_PDC_GAIN_L3, gain_l3);
    PUT_REG_INFO(DIP_X_BNR2_PDC_GAIN_L4, gain_l4);
    PUT_REG_INFO(DIP_X_BNR2_PDC_GAIN_R0, gain_r0);
    PUT_REG_INFO(DIP_X_BNR2_PDC_GAIN_R1, gain_r1);
    PUT_REG_INFO(DIP_X_BNR2_PDC_GAIN_R2, gain_r2);
    PUT_REG_INFO(DIP_X_BNR2_PDC_GAIN_R3, gain_r3);
    PUT_REG_INFO(DIP_X_BNR2_PDC_GAIN_R4, gain_r4);
    PUT_REG_INFO(DIP_X_BNR2_PDC_TH_GB, th_gb);
    PUT_REG_INFO(DIP_X_BNR2_PDC_TH_IA, th_ia);
    PUT_REG_INFO(DIP_X_BNR2_PDC_TH_HD, th_hd);
    PUT_REG_INFO(DIP_X_BNR2_PDC_SL, sl);
    PUT_REG_INFO(DIP_X_BNR2_PDC_POS, pos);

    return  (*this);
}

template <>
MBOOL
ISP_MGR_BNR2_T::
get(ISP_NVRAM_BNR_PDC_T& rParam, const dip_x_reg_t* pReg) const
{
    if (pReg)
    {
        GET_REG_INFO_BUF(DIP_X_BNR2_PDC_CON, con);
        GET_REG_INFO_BUF(DIP_X_BNR2_PDC_GAIN_L0, gain_l0);
        GET_REG_INFO_BUF(DIP_X_BNR2_PDC_GAIN_L1, gain_l1);
        GET_REG_INFO_BUF(DIP_X_BNR2_PDC_GAIN_L2, gain_l2);
        GET_REG_INFO_BUF(DIP_X_BNR2_PDC_GAIN_L3, gain_l3);
        GET_REG_INFO_BUF(DIP_X_BNR2_PDC_GAIN_L4, gain_l4);
        GET_REG_INFO_BUF(DIP_X_BNR2_PDC_GAIN_R0, gain_r0);
        GET_REG_INFO_BUF(DIP_X_BNR2_PDC_GAIN_R1, gain_r1);
        GET_REG_INFO_BUF(DIP_X_BNR2_PDC_GAIN_R2, gain_r2);
        GET_REG_INFO_BUF(DIP_X_BNR2_PDC_GAIN_R3, gain_r3);
        GET_REG_INFO_BUF(DIP_X_BNR2_PDC_GAIN_R4, gain_r4);
        GET_REG_INFO_BUF(DIP_X_BNR2_PDC_TH_GB, th_gb);
        GET_REG_INFO_BUF(DIP_X_BNR2_PDC_TH_IA, th_ia);
        GET_REG_INFO_BUF(DIP_X_BNR2_PDC_TH_HD, th_hd);
        GET_REG_INFO_BUF(DIP_X_BNR2_PDC_SL, sl);
        GET_REG_INFO_BUF(DIP_X_BNR2_PDC_POS, pos);
    }
    return MTRUE;
}

MBOOL
ISP_MGR_BNR2_T::
apply(EIspProfile_T /*eIspProfile*/, dip_x_reg_t* pReg)
{
    MBOOL bBPC2_EN = (isBPC2Enable() & (reinterpret_cast<ISP_CAM_BNR_BPC_CON_T*>(REG_INFO_VALUE_PTR(DIP_X_BNR2_BPC_CON))->BPC_EN));
    MBOOL bCT2_EN  = (isCT2Enable()  & (reinterpret_cast<ISP_CAM_BNR_NR1_CON_T*>(REG_INFO_VALUE_PTR(DIP_X_BNR2_NR1_CON))->NR1_CT_EN));
    MBOOL bPDC2_EN = (isPDC2Enable() & (reinterpret_cast<ISP_CAM_BNR_PDC_CON_T*>(REG_INFO_VALUE_PTR(DIP_X_BNR2_PDC_CON))->PDC_EN));
    MBOOL bBNR2_EN = (bBPC2_EN | bCT2_EN | bPDC2_EN);

    setPDC2Enable(bPDC2_EN);

    reinterpret_cast<ISP_CAM_BNR_BPC_CON_T*>(REG_INFO_VALUE_PTR(DIP_X_BNR2_BPC_CON))->BPC_EN = bBPC2_EN;
    reinterpret_cast<ISP_CAM_BNR_NR1_CON_T*>(REG_INFO_VALUE_PTR(DIP_X_BNR2_NR1_CON))->NR1_CT_EN = bCT2_EN;
    reinterpret_cast<ISP_CAM_BNR_PDC_CON_T*>(REG_INFO_VALUE_PTR(DIP_X_BNR2_PDC_CON))->PDC_EN = bPDC2_EN;


    // no use register
    reinterpret_cast<ISP_CAM_BNR_PDC_CON_T*>(REG_INFO_VALUE_PTR(DIP_X_BNR2_PDC_CON))->PDC_OUT = 0;

    // TOP
    ISP_WRITE_ENABLE_BITS(pReg, DIP_X_CTL_RGB_EN, BNR2_EN, bBNR2_EN);
    //ISP_MGR_CTL_EN_P2_T::getInstance(m_eSensorDev).setEnable_BNR2(bBNR2_EN);

    // Register setting
    writeRegs(static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum, pReg);


    dumpRegInfo("BNR2");

    return  MTRUE;
}

MBOOL
ISP_MGR_BNR2_T::
getBPCItable(
    UINT32& bpci_xsize,
    UINT32& bpci_ysize,
    UINT32& pdo_xsize,
    UINT32& pdo_ysize,
    MUINTPTR& phy_addr,
    MUINTPTR& vir_addr,
    MINT32& memID,
    EIndex_PDC_TBL_T Type){

    std::lock_guard<std::mutex> lock(m_Lock);

    switch (Type){

        case eIDX_PDE:
            if(m_pPDE_Buffer != NULL) {
                bpci_xsize = m_pBpciBuf_PDE->bpci_xsize;
                bpci_ysize = m_pBpciBuf_PDE->bpci_ysize;
                pdo_xsize = m_pBpciBuf_PDE->pdo_xsize;
                pdo_ysize = m_pBpciBuf_PDE->pdo_ysize;
                phy_addr = static_cast<IImageBuffer*>(m_pPDE_Buffer)->getBufPA(0);
                vir_addr = static_cast<IImageBuffer*>(m_pPDE_Buffer)->getBufVA(0);
                memID = static_cast<IImageBuffer*>(m_pPDE_Buffer)->getFD(0);
                return MTRUE;
            }
            else return MFALSE;


        case eIDX_PDC_FULL:
            if(m_pPDC_FULL_Buffer != NULL) {
                bpci_xsize = m_pBpciBuf_PDC_FULL->bpci_xsize;
                bpci_ysize = m_pBpciBuf_PDC_FULL->bpci_ysize;
                pdo_xsize = m_pBpciBuf_PDC_FULL->pdo_xsize;
                pdo_ysize = m_pBpciBuf_PDC_FULL->pdo_ysize;
                phy_addr = static_cast<IImageBuffer*>(m_pPDC_FULL_Buffer)->getBufPA(0);
                vir_addr = static_cast<IImageBuffer*>(m_pPDC_FULL_Buffer)->getBufVA(0);
                memID = static_cast<IImageBuffer*>(m_pPDC_FULL_Buffer)->getFD(0);
                return MTRUE;
            }
            else return MFALSE;



        case eIDX_PDC_BIN:
            if(m_pPDC_BIN_Buffer != NULL) {
                bpci_xsize = m_pBpciBuf_PDC_BIN->bpci_xsize;
                bpci_ysize = m_pBpciBuf_PDC_BIN->bpci_ysize;
                pdo_xsize = m_pBpciBuf_PDC_BIN->pdo_xsize;
                pdo_ysize = m_pBpciBuf_PDC_BIN->pdo_ysize;
                phy_addr = static_cast<IImageBuffer*>(m_pPDC_BIN_Buffer)->getBufPA(0);
                vir_addr = static_cast<IImageBuffer*>(m_pPDC_BIN_Buffer)->getBufVA(0);
                memID = static_cast<IImageBuffer*>(m_pPDC_BIN_Buffer)->getFD(0);
                return MTRUE;
            }
            else return MFALSE;

        default:
            return MFALSE;

    }

}

MBOOL
ISP_MGR_BNR2_T::
setBPCIBuf(CAMERA_BPCI_STRUCT* buf)
{
    std::lock_guard<std::mutex> lock(m_Lock);

    m_pBpciBuf_PDE = &(buf->PDE_TBL);
    m_pBpciBuf_PDC_FULL = &(buf->PDC_FULL_TBL);
    m_pBpciBuf_PDC_BIN = &(buf->PDC_BIN_TBL);

    if (m_pBpciBuf_PDE == NULL ||
        m_pBpciBuf_PDC_FULL == NULL ||
        m_pBpciBuf_PDC_BIN == NULL)
        return MFALSE;

    MUINT32 x_size_pde     = m_pBpciBuf_PDE->bpci_xsize;
    MUINT32 x_size_pdc_full = m_pBpciBuf_PDC_FULL->bpci_xsize;
    MUINT32 x_size_pdc_bin  = m_pBpciBuf_PDC_BIN->bpci_xsize;
    MUINT32 y_size = 1;
    // create buffer
    MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
    MUINT32 bufStridesInBytes_pde[3]      = {x_size_pde, 0, 0};
    MUINT32 bufStridesInBytes_pdc_full[3] = {x_size_pdc_full, 0, 0};
    MUINT32 bufStridesInBytes_pdc_bin[3]  = {x_size_pdc_bin, 0, 0};

    std::string strName_pde      = "PDE_tbl";
    std::string strName_pdc_full = "PDC_FULL_tbl";
    std::string strName_pdc_bin  = "PDC_BIN_tbl";

    IImageBufferAllocator::ImgParam imgParam_pde =
        IImageBufferAllocator::ImgParam((EImageFormat)eImgFmt_STA_BYTE,
            MSize(x_size_pde, y_size), bufStridesInBytes_pde, bufBoundaryInBytes, 1);
    IImageBufferAllocator::ImgParam imgParam_pdc_full =
        IImageBufferAllocator::ImgParam((EImageFormat)eImgFmt_STA_BYTE,
            MSize(x_size_pdc_full, y_size), bufStridesInBytes_pdc_full, bufBoundaryInBytes, 1);
    IImageBufferAllocator::ImgParam imgParam_pdc_bin =
        IImageBufferAllocator::ImgParam((EImageFormat)eImgFmt_STA_BYTE,
            MSize(x_size_pdc_bin, y_size), bufStridesInBytes_pdc_bin, bufBoundaryInBytes, 1);

    sp<IIonImageBufferHeap> pHeap_pde =
        IIonImageBufferHeap::create(strName_pde.c_str(), imgParam_pde);
    sp<IIonImageBufferHeap> pHeap_pdc_full =
        IIonImageBufferHeap::create(strName_pdc_full.c_str(), imgParam_pdc_full);
    sp<IIonImageBufferHeap> pHeap_pdc_bin =
        IIonImageBufferHeap::create(strName_pdc_bin.c_str(), imgParam_pdc_bin);

    if (pHeap_pde == NULL) {
        CAM_LOGE("[%s] ImageBufferHeap create fail", strName_pde.c_str());
        return MFALSE;
    }
    if (pHeap_pdc_full == NULL) {
        CAM_LOGE("[%s] ImageBufferHeap create fail", strName_pdc_full.c_str());
        return MFALSE;
    }
    if (pHeap_pdc_bin == NULL) {
        CAM_LOGE("[%s] ImageBufferHeap create fail", strName_pdc_bin.c_str());
        return MFALSE;
    }

    IImageBuffer* pImgBuf_pde      = pHeap_pde->createImageBuffer();
    IImageBuffer* pImgBuf_pdc_full = pHeap_pdc_full->createImageBuffer();
    IImageBuffer* pImgBuf_pdc_bin  = pHeap_pdc_bin->createImageBuffer();

    if (pImgBuf_pde == NULL) {
        CAM_LOGE("[%s] ImageBufferHeap create fail", strName_pde.c_str());
        return MFALSE;
    }
    if (pImgBuf_pdc_full == NULL) {
        CAM_LOGE("[%s] ImageBufferHeap create fail", strName_pdc_full.c_str());
        return MFALSE;
    }
    if (pImgBuf_pdc_bin == NULL) {
        CAM_LOGE("[%s] ImageBufferHeap create fail", strName_pdc_bin.c_str());
        return MFALSE;
    }
    // lock buffer
    MUINT const usage_pde = (GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN |
                        GRALLOC_USAGE_HW_CAMERA_READ |
                        GRALLOC_USAGE_HW_CAMERA_WRITE);
    MUINT const usage_pdc_full = (GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN |
                        GRALLOC_USAGE_HW_CAMERA_READ |
                        GRALLOC_USAGE_HW_CAMERA_WRITE);
    MUINT const usage_pdc_bin = (GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN |
                        GRALLOC_USAGE_HW_CAMERA_READ |
                        GRALLOC_USAGE_HW_CAMERA_WRITE);

    if (!(pImgBuf_pde->lockBuf(strName_pde.c_str(), usage_pde)))
    {
        CAM_LOGE("[%s] Stuff ImageBuffer lock fail",  strName_pde.c_str());
        return MFALSE;
    }
    if (!(pImgBuf_pdc_full->lockBuf(strName_pdc_full.c_str(), usage_pdc_full)))
    {
        CAM_LOGE("[%s] Stuff ImageBuffer lock fail",  strName_pdc_full.c_str());
        return MFALSE;
    }
    if (!(pImgBuf_pdc_bin->lockBuf(strName_pdc_bin.c_str(), usage_pdc_bin)))
    {
        CAM_LOGE("[%s] Stuff ImageBuffer lock fail",  strName_pdc_bin.c_str());
        return MFALSE;
    }

    // release m_pPDCBuffer
    if (m_pPDE_Buffer){
        sp<IImageBuffer> temp = static_cast<IImageBuffer*>(m_pPDE_Buffer);
    }
    if (m_pPDC_FULL_Buffer){
        sp<IImageBuffer> temp = static_cast<IImageBuffer*>(m_pPDC_FULL_Buffer);
    }
    if (m_pPDC_BIN_Buffer){
        sp<IImageBuffer> temp = static_cast<IImageBuffer*>(m_pPDC_BIN_Buffer);
    }

    memcpy(reinterpret_cast<MVOID*>(pImgBuf_pde->getBufVA(0)), m_pBpciBuf_PDE->bpci_array, m_pBpciBuf_PDE->bpci_xsize);
    memcpy(reinterpret_cast<MVOID*>(pImgBuf_pdc_full->getBufVA(0)), m_pBpciBuf_PDC_FULL->bpci_array, m_pBpciBuf_PDC_FULL->bpci_xsize);
    memcpy(reinterpret_cast<MVOID*>(pImgBuf_pdc_bin->getBufVA(0)), m_pBpciBuf_PDC_BIN->bpci_array, m_pBpciBuf_PDC_BIN->bpci_xsize);

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("dump.bpci_tbl.enable", value, "0"); // 0: enable, 1: disable
    MBOOL bDumpTblEnable = atoi(value);

    if (bDumpTblEnable)
    {
        char filename_pde[512];
        sprintf(filename_pde, "/sdcard/debug/pde_table_size_%d.bin", m_pBpciBuf_PDE->bpci_xsize);
        FILE* fp_pde = fopen(filename_pde, "wb");
        if (fp_pde)
        {
            ::fwrite(m_pBpciBuf_PDE->bpci_array, m_pBpciBuf_PDE->bpci_xsize, 1, fp_pde);
            fclose(fp_pde);
        }

        char filename_pdc_full[512];
        sprintf(filename_pdc_full, "/sdcard/debug/pdc_full_table_size_%d.bin", m_pBpciBuf_PDC_FULL->bpci_xsize);
        FILE* fp_pdc_full = fopen(filename_pdc_full, "wb");
        if (fp_pdc_full)
        {
            ::fwrite(m_pBpciBuf_PDC_FULL->bpci_array, m_pBpciBuf_PDC_FULL->bpci_xsize, 1, fp_pdc_full);
            fclose(fp_pdc_full);
        }

        char filename_pdc_bin[512];
        sprintf(filename_pdc_bin, "/sdcard/debug/pdc_bin_table_size_%d.bin", m_pBpciBuf_PDC_BIN->bpci_xsize);
        FILE* fp_pdc_bin = fopen(filename_pdc_bin, "wb");
        if (fp_pdc_bin)
        {
            ::fwrite(m_pBpciBuf_PDC_BIN->bpci_array, m_pBpciBuf_PDC_BIN->bpci_xsize, 1, fp_pdc_bin);
            fclose(fp_pdc_bin);
        }

    }

    m_pPDE_Buffer        = static_cast<MVOID*>(pImgBuf_pde);
    m_pPDC_FULL_Buffer   = static_cast<MVOID*>(pImgBuf_pdc_full);
    m_pPDC_BIN_Buffer    = static_cast<MVOID*>(pImgBuf_pdc_bin);

    return MTRUE;
}

MVOID
ISP_MGR_BNR2_T::
unlockBPCIBuf()
{
    std::lock_guard<std::mutex> lock(m_Lock);

    std::string strName_pde      = "PDE_tbl";  //same in setPBCIBuf
    std::string strName_pdc_full = "PDC_FULL_tbl";  //same in setPBCIBuf
    std::string strName_pdc_bin  = "PDC_BIN_tbl";  //same in setPBCIBuf

    if (m_pPDE_Buffer != NULL)
    {
        static_cast<IImageBuffer*>(m_pPDE_Buffer)->unlockBuf(strName_pde.c_str());
        // destroy buffer
        sp<IImageBuffer> temp_pde = static_cast<IImageBuffer*>(m_pPDE_Buffer);
        m_pPDE_Buffer = NULL;
    }

    if (m_pPDC_FULL_Buffer != NULL)
    {
        static_cast<IImageBuffer*>(m_pPDC_FULL_Buffer)->unlockBuf(strName_pdc_full.c_str());
        // destroy buffer
        sp<IImageBuffer> temp_pdc_full = static_cast<IImageBuffer*>(m_pPDC_FULL_Buffer);
        m_pPDC_FULL_Buffer = NULL;
    }

    if (m_pPDC_BIN_Buffer != NULL)
    {
        static_cast<IImageBuffer*>(m_pPDC_BIN_Buffer)->unlockBuf(strName_pdc_bin.c_str());
        // destroy buffer
        sp<IImageBuffer> temp_pdc_bin = static_cast<IImageBuffer*>(m_pPDC_BIN_Buffer);
        m_pPDC_BIN_Buffer = NULL;
    }




}

}

