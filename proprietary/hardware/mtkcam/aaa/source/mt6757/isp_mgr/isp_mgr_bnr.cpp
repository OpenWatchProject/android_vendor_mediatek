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

using namespace NSCam;

namespace NSIspTuningv3
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BNR
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_BNR_T&
ISP_MGR_BNR_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
    {
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_BNR_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_BNR_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_BNR_DEV<ESensorDev_Sub>::getInstance();
    default:
        CAM_LOGE("eSensorDev(%d)", eSensorDev);
        return  ISP_MGR_BNR_DEV<ESensorDev_Main>::getInstance();
    }
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
    PUT_REG_INFO(CAM_BNR_BPC_TBLI2, tbli2);
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
    GET_REG_INFO(CAM_BNR_BPC_TBLI2, tbli2);
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
apply(EIspProfile_T eIspProfile, TuningMgr& rTuning, MINT32 i4SubsampleIdex)
{
    MBOOL bBPC_EN = (isBPCEnable() & (reinterpret_cast<ISP_CAM_BNR_BPC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_BPC_CON))->BPC_EN));
    MBOOL bCT_EN  = (isCTEnable()  & (reinterpret_cast<ISP_CAM_BNR_NR1_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_NR1_CON))->NR1_CT_EN));
    MBOOL bPDC_EN = isPDCEnable();
    MBOOL bBNR_EN = (bBPC_EN | bCT_EN | bPDC_EN);

    //BNR HW limitation
    if (bBNR_EN && (bBPC_EN == 0))
    {
        bBNR_EN = 0;
        bCT_EN = 0;
        MY_LOG("BNR::apply warning: set (bBNR_EN, bBPC_EN, bCT_EN) = (%d, %d, %d)", bBNR_EN, bBPC_EN, bCT_EN);
    }

    //PDAF setting
    reinterpret_cast<ISP_CAM_BNR_PDC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_PDC_CON))->PDC_EN = isPDCEnable();
    reinterpret_cast<ISP_CAM_BNR_PDC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_PDC_CON))->PDC_OUT = isPDCoutEnable();

    //PDC HW limitation
    if(reinterpret_cast<ISP_CAM_BNR_PDC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_PDC_CON))->PDC_EN){
        // PDAF SW limitation
        reinterpret_cast<ISP_CAM_BNR_BPC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_BPC_CON))->BPC_LUT_EN = 1;
    }
    else{
        reinterpret_cast<ISP_CAM_BNR_PDC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_PDC_CON))->PDC_OUT = 0;
    }


    rTuning.updateEngine(eTuningMgrFunc_BNR, bBNR_EN, i4SubsampleIdex);

    // TOP
    TUNING_MGR_WRITE_BITS_CAM((&rTuning), CAM_CTL_EN, BNR_EN, bBNR_EN, i4SubsampleIdex);
    ISP_MGR_CTL_EN_P1_T::getInstance(m_eSensorDev).setEnable_BNR(bBNR_EN);

    // Register setting
    rTuning.tuningMgrWriteRegs(static_cast<TUNING_MGR_REG_IO_STRUCT*>(m_pRegInfo),
                             m_u4RegInfoNum, i4SubsampleIdex);

    dumpRegInfo("BNR");

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BNR2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_BNR2_T&
ISP_MGR_BNR2_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
    {
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_BNR2_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_BNR2_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_BNR2_DEV<ESensorDev_Sub>::getInstance();
    default:
        CAM_LOGE("eSensorDev(%d)", eSensorDev);
        return  ISP_MGR_BNR2_DEV<ESensorDev_Main>::getInstance();
    }
}

ISP_MGR_BNR2::~ISP_MGR_BNR2()
{
    if (m_pPDCBuffer != nullptr) {
        // using sp to release IImageBuffer
        sp<IImageBuffer> temp = static_cast<IImageBuffer*>(m_pPDCBuffer);
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
    PUT_REG_INFO(DIP_X_BNR2_BPC_TBLI2, tbli2);
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
        GET_REG_INFO_BUF(DIP_X_BNR2_BPC_TBLI2, tbli2);
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
getBPCItable(
    UINT32& bpci_xsize,
    UINT32& bpci_ysize,
    UINT32& pdo_xsize,
    UINT32& pdo_ysize,
    MUINTPTR& phy_addr,
    MUINTPTR& vir_addr){
    if(m_pPDCBuffer != NULL) {
        bpci_xsize = m_pBpciBuf->bpci_xsize;
        bpci_ysize = m_pBpciBuf->bpci_ysize;
        pdo_xsize = m_pBpciBuf->pdo_xsize;
        pdo_ysize = m_pBpciBuf->pdo_ysize;
        phy_addr = static_cast<IImageBuffer*>(m_pPDCBuffer)->getBufPA(0);
        vir_addr = static_cast<IImageBuffer*>(m_pPDCBuffer)->getBufVA(0);
        return MTRUE;
    }
    return MFALSE;
}

MBOOL
ISP_MGR_BNR2_T::
setBPCIBuf(CAMERA_BPCI_STRUCT* buf)
{
    m_pBpciBuf = buf;
    if (m_pBpciBuf == NULL)
        return MFALSE;

    MUINT32 x_size = m_pBpciBuf->bpci_xsize;
    MUINT32 y_size = 1;
    // create buffer
    MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
    MUINT32 bufStridesInBytes[3] = {x_size, 0, 0};

    std::string strName = "BPCI_tbl";

    IImageBufferAllocator::ImgParam imgParam =
        IImageBufferAllocator::ImgParam((EImageFormat)eImgFmt_STA_BYTE,
            MSize(x_size, y_size), bufStridesInBytes, bufBoundaryInBytes, 1);
    sp<IIonImageBufferHeap> pHeap =
        IIonImageBufferHeap::create(strName.c_str(), imgParam);
    if (pHeap == NULL) {
        CAM_LOGE("[%s] ImageBufferHeap create fail", strName.c_str());
        return MFALSE;
    }
    IImageBuffer* pImgBuf = pHeap->createImageBuffer();
    if (pImgBuf == NULL) {
        CAM_LOGE("[%s] ImageBufferHeap create fail", strName.c_str());
        return MFALSE;
    }
    // lock buffer
    MUINT const usage = (GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN |
                        GRALLOC_USAGE_HW_CAMERA_READ |
                        GRALLOC_USAGE_HW_CAMERA_WRITE);
    if (!(pImgBuf->lockBuf(strName.c_str(), usage)))
    {
        CAM_LOGE("[%s] Stuff ImageBuffer lock fail",  strName.c_str());
        return MFALSE;
    }

    // release m_pPDCBuffer
    if (m_pPDCBuffer){
        sp<IImageBuffer> temp = static_cast<IImageBuffer*>(m_pPDCBuffer);
    }

    memcpy(reinterpret_cast<MVOID*>(pImgBuf->getBufVA(0)), m_pBpciBuf->bpci_array, m_pBpciBuf->bpci_xsize);

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.dump.bpci_tbl.enable", value, "0"); // 0: enable, 1: disable
    MBOOL bDumpTblEnable = atoi(value);

    if (bDumpTblEnable)
    {
        char filename[512];
        sprintf(filename, "/sdcard/debug/bpci_table_size_%d.bin", m_pBpciBuf->bpci_xsize);
        FILE* fp = fopen(filename, "wb");
        if (fp)
        {
            ::fwrite(m_pBpciBuf->bpci_array, m_pBpciBuf->bpci_xsize, 1, fp);
            fclose(fp);
        }
    }
    //bpciPhyAddr = pImgBuf->getBufPA(0);
    //bpciVirAddr = pImgBuf->getBufVA(0);

    m_pPDCBuffer = static_cast<MVOID*>(pImgBuf);

    return MTRUE;
}


MBOOL
ISP_MGR_BNR2_T::
apply(EIspProfile_T eIspProfile, dip_x_reg_t* pReg)
{
    MBOOL bBPC2_EN = (isBPC2Enable() & (reinterpret_cast<ISP_CAM_BNR_BPC_CON_T*>(REG_INFO_VALUE_PTR(DIP_X_BNR2_BPC_CON))->BPC_EN));
    MBOOL bCT2_EN  = (isCT2Enable()  & (reinterpret_cast<ISP_CAM_BNR_NR1_CON_T*>(REG_INFO_VALUE_PTR(DIP_X_BNR2_NR1_CON))->NR1_CT_EN));
    MBOOL bPDC2_EN = (isPDC2Enable() & (reinterpret_cast<ISP_CAM_BNR_PDC_CON_T*>(REG_INFO_VALUE_PTR(DIP_X_BNR2_PDC_CON))->PDC_EN));
    MBOOL bBNR2_EN = (bBPC2_EN | bCT2_EN | bPDC2_EN);

    if ( (m_pPDCBuffer != NULL) && m_bPDSensor && bPDC2_EN){
        //PDAF setting
        reinterpret_cast<ISP_CAM_BNR_PDC_CON_T*>(REG_INFO_VALUE_PTR(DIP_X_BNR2_PDC_CON))->PDC_EN = 1;
        reinterpret_cast<ISP_CAM_BNR_BPC_CON_T*>(REG_INFO_VALUE_PTR(DIP_X_BNR2_BPC_CON))->BPC_LUT_EN = 1;
    }
    else{
        reinterpret_cast<ISP_CAM_BNR_PDC_CON_T*>(REG_INFO_VALUE_PTR(DIP_X_BNR2_PDC_CON))->PDC_EN = 0;
        reinterpret_cast<ISP_CAM_BNR_PDC_CON_T*>(REG_INFO_VALUE_PTR(DIP_X_BNR2_PDC_CON))->PDC_OUT = 0;
    }

    // TOP
    ISP_WRITE_ENABLE_BITS(pReg, DIP_X_CTL_RGB_EN, BNR2_EN, bBNR2_EN);
    //ISP_MGR_CTL_EN_P2_T::getInstance(m_eSensorDev).setEnable_BNR2(bBNR2_EN);

    // Register setting
    writeRegs(static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum, pReg);


    dumpRegInfo("BNR2");

    return  MTRUE;
}



}

