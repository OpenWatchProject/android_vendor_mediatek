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
#include <kd_imgsensor_define.h>
#include <mtkcam/drv/IHalSensor.h>


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
    case ESensorDev_SubSecond: //  Main Second Sensor
        return  ISP_MGR_BNR_DEV<ESensorDev_SubSecond>::getInstance();
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
apply(RAWIspCamInfo& rRawIspCamInfo, TuningMgr& rTuning, MINT32 i4SubsampleIdex)
{
    /***********************************************
     *          Checking BPC setting
     ***********************************************/
    MBOOL bBPC_EN = (isBPCEnable() & (reinterpret_cast<ISP_CAM_BNR_BPC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_BPC_CON))->BPC_EN));

    /***********************************************
     *          Checking CT setting
     ***********************************************/
    MBOOL bCT_EN  = (isCTEnable()  & (reinterpret_cast<ISP_CAM_BNR_NR1_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_NR1_CON))->NR1_CT_EN));

    /***********************************************
     *          Checking PDC setting
     ***********************************************/
    // 1. Get BPCI table
    EIndex_PDC_TBL_T ePDC_table_type = eIDX_PDC_NUM;
    switch( m_u4PDSensorFmt)
    {
        case PDAF_SUPPORT_RAW:
            if( !rRawIspCamInfo.BinInfo.fgBIN)
            {
                if( rRawIspCamInfo.eSensorMode==SENSOR_SCENARIO_ID_NORMAL_CAPTURE)
                    ePDC_table_type = eIDX_PDC_1;
                else if( rRawIspCamInfo.eSensorMode==SENSOR_SCENARIO_ID_NORMAL_VIDEO)
                    ePDC_table_type = eIDX_PDC_2;
                else if( rRawIspCamInfo.eSensorMode==SENSOR_SCENARIO_ID_NORMAL_PREVIEW)
                    ePDC_table_type = eIDX_PDC_3;
                else
                    ePDC_table_type = eIDX_PDC_NUM;
            }
            else
                ePDC_table_type = eIDX_PDC_NUM;
            break;
        case PDAF_SUPPORT_CAMSV:
        case PDAF_SUPPORT_CAMSV_LEGACY:
            if( rRawIspCamInfo.eSensorMode==SENSOR_SCENARIO_ID_NORMAL_CAPTURE)
                ePDC_table_type = rRawIspCamInfo.BinInfo.fgBIN ? eIDX_PDC_2 : eIDX_PDC_1;
            else
                ePDC_table_type = eIDX_PDC_NUM;
            break;
        default:
            ePDC_table_type = eIDX_PDC_NUM;
            break;
    }

    memset(&m_sPDOHWInfo, 0, sizeof(SPDOHWINFO_T));
    m_bBPCIValid = ISP_MGR_BNR2::getInstance(m_eSensorDev).getBPCItable(
                                                       m_sPDOHWInfo.u4Bpci_xsz,
                                                       m_sPDOHWInfo.u4Bpci_ysz,
                                                       m_sPDOHWInfo.u4Pdo_xsz,
                                                       m_sPDOHWInfo.u4Pdo_ysz,
                                                       m_sPDOHWInfo.phyAddrBpci_tbl,
                                                       m_sPDOHWInfo.virAddrBpci_tbl,
                                                       m_sPDOHWInfo.i4memID,
                                                       ePDC_table_type);
    if( m_sPDOHWInfo.virAddrBpci_tbl)
    {
        CAM_LOGD_IF( m_bDebugEnable,
                     "bBPCIValid(%d) : ePDC_table_type(%d), u4Bpci_xsz(%d), u4Bpci_ysz(%d), u4Pdo_xsz(%d), u4Pdo_ysz(%d), BPCI table first 4 dword: %x %x %x %x",
                     m_bBPCIValid,
                     ePDC_table_type,
                     m_sPDOHWInfo.u4Bpci_xsz,
                     m_sPDOHWInfo.u4Bpci_ysz,
                     m_sPDOHWInfo.u4Pdo_xsz,
                     m_sPDOHWInfo.u4Pdo_ysz,
                     ((MUINT32*)m_sPDOHWInfo.virAddrBpci_tbl)[0],
                     ((MUINT32*)m_sPDOHWInfo.virAddrBpci_tbl)[1],
                     ((MUINT32*)m_sPDOHWInfo.virAddrBpci_tbl)[2],
                     ((MUINT32*)m_sPDOHWInfo.virAddrBpci_tbl)[3]);
    }
    else
    {
        CAM_LOGD_IF( m_bDebugEnable,
                     "bBPCIValid(%d) : ePDC_table_type(%d), u4Bpci_xsz(%d), u4Bpci_ysz(%d), u4Pdo_xsz(%d), u4Pdo_ysz(%d)",
                     m_bBPCIValid,
                     ePDC_table_type,
                     m_sPDOHWInfo.u4Bpci_xsz,
                     m_sPDOHWInfo.u4Bpci_ysz,
                     m_sPDOHWInfo.u4Pdo_xsz,
                     m_sPDOHWInfo.u4Pdo_ysz);
    }
    // 2. Get BPCI table
    MBOOL bPDC_EN =  m_bBPCIValid &&
                    (isAF_PDCEnable() ||(isPDCEnable() && (reinterpret_cast<ISP_CAM_BNR_PDC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_PDC_CON))->PDC_EN)));

    CAM_LOGD_IF( m_bDebugEnable,
                 "bPDC_EN(%d) : BPCIValid(%d), isAF_PDCEnable(%d), isPDCEnable(%d), PDC_EN(%d)",
                 bPDC_EN,
                 m_bBPCIValid,
                 isAF_PDCEnable(),
                 isPDCEnable(),
                 (reinterpret_cast<ISP_CAM_BNR_PDC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_PDC_CON))->PDC_EN));

    /***********************************************
     *                 Top Control
     ***********************************************/
    MBOOL bBNR_EN = (bBPC_EN || bCT_EN || bPDC_EN);



    //BNR HW limitation
    /*
    if (bBNR_EN && (bBPC_EN == 0))
    {
        bBNR_EN = 0;
        bCT_EN = 0;
        CAM_LOGD("BNR::apply warning: set (bBNR_EN, bBPC_EN, bCT_EN) = (%d, %d, %d)", bBNR_EN, bBPC_EN, bCT_EN);
    }
    */

    /***********************************************
     *                 Configure
     ***********************************************/
    reinterpret_cast<ISP_CAM_BNR_BPC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_BPC_CON))->BPC_EN    = bBPC_EN;
    reinterpret_cast<ISP_CAM_BNR_NR1_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_NR1_CON))->NR1_CT_EN = bCT_EN;
    reinterpret_cast<ISP_CAM_BNR_PDC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_PDC_CON))->PDC_EN    = bPDC_EN;

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

        rTuning.updateEngineFD(eTuningMgrFunc_BNR, 0, m_sPDOHWInfo.i4memID, m_sPDOHWInfo.virAddrBpci_tbl);
    }
    else{
        reinterpret_cast<ISP_CAM_BNR_BPC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_BPC_CON))->BPC_LUT_EN = 0;
        reinterpret_cast<ISP_CAM_BNR_PDC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_PDC_CON))->PDC_OUT = 0;

    }

    CAM_LOGD_IF( m_bDebugEnable,
                 "bBNR_EN(%d) BPC_EN(%d) PDC_EN(%d) BPC_LUT_EN(%d) BPC_XSIZE(%d) BPC_YSIZE(%d) PDC_OUT(%d) CAM_BPCI_BASE_ADDR(%p) CAM_BPCI_XSIZE(%p) CAM_BPCI_YSIZE(%p) CAM_BPCI_STRIDE(%p)",
                 bBNR_EN,
                 reinterpret_cast<ISP_CAM_BNR_BPC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_BPC_CON))->BPC_EN,
                 reinterpret_cast<ISP_CAM_BNR_PDC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_PDC_CON))->PDC_EN,
                 reinterpret_cast<ISP_CAM_BNR_BPC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_BPC_CON))->BPC_LUT_EN,
                 reinterpret_cast<ISP_CAM_BNR_BPC_TBLI2_T*>(REG_INFO_VALUE_PTR(CAM_BNR_BPC_TBLI2))->BPC_XSIZE,
                 reinterpret_cast<ISP_CAM_BNR_BPC_TBLI2_T*>(REG_INFO_VALUE_PTR(CAM_BNR_BPC_TBLI2))->BPC_YSIZE,
                 reinterpret_cast<ISP_CAM_BNR_PDC_CON_T*>(REG_INFO_VALUE_PTR(CAM_BNR_PDC_CON))->PDC_OUT,
                 REG_INFO_VALUE(CAM_BPCI_BASE_ADDR),
                 REG_INFO_VALUE(CAM_BPCI_XSIZE),
                 REG_INFO_VALUE(CAM_BPCI_YSIZE),
                 REG_INFO_VALUE(CAM_BPCI_STRIDE));


    rTuning.updateEngine(eTuningMgrFunc_BNR, bBNR_EN, i4SubsampleIdex);

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
    switch (eSensorDev)
    {
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_BNR2_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_BNR2_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_BNR2_DEV<ESensorDev_Sub>::getInstance();
    case ESensorDev_SubSecond: //  Main Second Sensor
        return  ISP_MGR_BNR2_DEV<ESensorDev_SubSecond>::getInstance();
    default:
        CAM_LOGE("eSensorDev(%d)", eSensorDev);
        return  ISP_MGR_BNR2_DEV<ESensorDev_Main>::getInstance();
    }
}

ISP_MGR_BNR2::~ISP_MGR_BNR2()
{
    if (m_pPDCBuffer_1 != nullptr) {
        // using sp to release IImageBuffer
        sp<IImageBuffer> temp_pdc_1 = static_cast<IImageBuffer*>(m_pPDCBuffer_1);
    }

    if (m_pPDCBuffer_2 != nullptr) {
        // using sp to release IImageBuffer
        sp<IImageBuffer> temp_pdc_2 = static_cast<IImageBuffer*>(m_pPDCBuffer_2);
    }

    if (m_pPDCBuffer_3 != nullptr) {
        // using sp to release IImageBuffer
        sp<IImageBuffer> temp_pdc_3 = static_cast<IImageBuffer*>(m_pPDCBuffer_3);
    }

    if (m_pPDCBuffer_4 != nullptr) {
        // using sp to release IImageBuffer
        sp<IImageBuffer> temp_pdc_4 = static_cast<IImageBuffer*>(m_pPDCBuffer_4);
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

        case eIDX_PDC_1:
            if(m_pPDCBuffer_1 != NULL) {
                bpci_xsize = m_pBpciBuf_1->bpci_xsize;
                bpci_ysize = m_pBpciBuf_1->bpci_ysize;
                pdo_xsize  = m_pBpciBuf_1->pdo_xsize;
                pdo_ysize  = m_pBpciBuf_1->pdo_ysize;
                phy_addr = static_cast<IImageBuffer*>(m_pPDCBuffer_1)->getBufPA(0);
                vir_addr = static_cast<IImageBuffer*>(m_pPDCBuffer_1)->getBufVA(0);
                memID = static_cast<IImageBuffer*>(m_pPDCBuffer_1)->getFD(0);
                return MTRUE;
            }
            else return MFALSE;

        case eIDX_PDC_2:
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
            else return MFALSE;

        case eIDX_PDC_3:
            if(m_pPDCBuffer_3 != NULL) {
                bpci_xsize = m_pBpciBuf_3->bpci_xsize;
                bpci_ysize = m_pBpciBuf_3->bpci_ysize;
                pdo_xsize = m_pBpciBuf_3->pdo_xsize;
                pdo_ysize = m_pBpciBuf_3->pdo_ysize;
                phy_addr = static_cast<IImageBuffer*>(m_pPDCBuffer_3)->getBufPA(0);
                vir_addr = static_cast<IImageBuffer*>(m_pPDCBuffer_3)->getBufVA(0);
                memID = static_cast<IImageBuffer*>(m_pPDCBuffer_3)->getFD(0);
                return MTRUE;
            }
            else return MFALSE;

        case eIDX_PDC_4:
            if(m_pPDCBuffer_4 != NULL) {
                bpci_xsize = m_pBpciBuf_4->bpci_xsize;
                bpci_ysize = m_pBpciBuf_4->bpci_ysize;
                pdo_xsize = m_pBpciBuf_4->pdo_xsize;
                pdo_ysize = m_pBpciBuf_4->pdo_ysize;
                phy_addr = static_cast<IImageBuffer*>(m_pPDCBuffer_4)->getBufPA(0);
                vir_addr = static_cast<IImageBuffer*>(m_pPDCBuffer_4)->getBufVA(0);
                memID = static_cast<IImageBuffer*>(m_pPDCBuffer_4)->getFD(0);
                return MTRUE;
            }
            else return MFALSE;

        case eIDX_PDC_NUM:
        default:
            return MFALSE;

    }

}

MBOOL
ISP_MGR_BNR2_T::
setBPCIBuf(CAMERA_BPCI_STRUCT* buf)
{
    std::lock_guard<std::mutex> lock(m_Lock);

    m_pBpciBuf_1 = &(buf->PDC_TBL_1);
    m_pBpciBuf_2 = &(buf->PDC_TBL_2);
    m_pBpciBuf_3 = &(buf->PDC_TBL_3);
    m_pBpciBuf_4 = &(buf->PDC_TBL_4);

    if (m_pBpciBuf_1 == NULL ||
        m_pBpciBuf_2 == NULL ||
        m_pBpciBuf_3 == NULL ||
        m_pBpciBuf_4 == NULL)
        return MFALSE;

    MUINT32 x_size_pdc_1 = m_pBpciBuf_1->bpci_xsize;
    MUINT32 x_size_pdc_2 = m_pBpciBuf_2->bpci_xsize;
    MUINT32 x_size_pdc_3 = m_pBpciBuf_3->bpci_xsize;
    MUINT32 x_size_pdc_4 = m_pBpciBuf_4->bpci_xsize;
    MUINT32 y_size = 1;
    // create buffer
    MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
    MUINT32 bufStridesInBytes_pdc_1[3] = {x_size_pdc_1, 0, 0};
    MUINT32 bufStridesInBytes_pdc_2[3] = {x_size_pdc_2, 0, 0};
    MUINT32 bufStridesInBytes_pdc_3[3] = {x_size_pdc_3, 0, 0};
    MUINT32 bufStridesInBytes_pdc_4[3] = {x_size_pdc_4, 0, 0};

    std::string strName_pdc_1  = "PDC_tbl_1";
    std::string strName_pdc_2  = "PDC_tbl_2";
    std::string strName_pdc_3  = "PDC_tbl_3";
    std::string strName_pdc_4  = "PDC_tbl_4";

    IImageBufferAllocator::ImgParam imgParam_pdc_1 =
        IImageBufferAllocator::ImgParam((EImageFormat)eImgFmt_STA_BYTE,
            MSize(x_size_pdc_1, y_size), bufStridesInBytes_pdc_1, bufBoundaryInBytes, 1);
    IImageBufferAllocator::ImgParam imgParam_pdc_2 =
        IImageBufferAllocator::ImgParam((EImageFormat)eImgFmt_STA_BYTE,
            MSize(x_size_pdc_2, y_size), bufStridesInBytes_pdc_2, bufBoundaryInBytes, 1);
    IImageBufferAllocator::ImgParam imgParam_pdc_3 =
        IImageBufferAllocator::ImgParam((EImageFormat)eImgFmt_STA_BYTE,
            MSize(x_size_pdc_3, y_size), bufStridesInBytes_pdc_3, bufBoundaryInBytes, 1);
    IImageBufferAllocator::ImgParam imgParam_pdc_4 =
        IImageBufferAllocator::ImgParam((EImageFormat)eImgFmt_STA_BYTE,
            MSize(x_size_pdc_4, y_size), bufStridesInBytes_pdc_4, bufBoundaryInBytes, 1);

    sp<IIonImageBufferHeap> pHeap_pdc_1 =
        IIonImageBufferHeap::create(strName_pdc_1.c_str(), imgParam_pdc_1);
    sp<IIonImageBufferHeap> pHeap_pdc_2 =
        IIonImageBufferHeap::create(strName_pdc_2.c_str(), imgParam_pdc_2);
    sp<IIonImageBufferHeap> pHeap_pdc_3 =
        IIonImageBufferHeap::create(strName_pdc_3.c_str(), imgParam_pdc_3);
    sp<IIonImageBufferHeap> pHeap_pdc_4 =
        IIonImageBufferHeap::create(strName_pdc_4.c_str(), imgParam_pdc_4);

    if (pHeap_pdc_1 == NULL) {
        CAM_LOGE("[%s] ImageBufferHeap create fail", strName_pdc_1.c_str());
        return MFALSE;
    }
    if (pHeap_pdc_2 == NULL) {
        CAM_LOGE("[%s] ImageBufferHeap create fail", strName_pdc_2.c_str());
        return MFALSE;
    }
    if (pHeap_pdc_3 == NULL) {
        CAM_LOGE("[%s] ImageBufferHeap create fail", strName_pdc_3.c_str());
        return MFALSE;
    }
    if (pHeap_pdc_4 == NULL) {
        CAM_LOGE("[%s] ImageBufferHeap create fail", strName_pdc_4.c_str());
        return MFALSE;
    }

    IImageBuffer* pImgBuf_pdc_1  = pHeap_pdc_1->createImageBuffer();
    IImageBuffer* pImgBuf_pdc_2  = pHeap_pdc_2->createImageBuffer();
    IImageBuffer* pImgBuf_pdc_3  = pHeap_pdc_3->createImageBuffer();
    IImageBuffer* pImgBuf_pdc_4  = pHeap_pdc_4->createImageBuffer();

    if (pImgBuf_pdc_1 == NULL) {
        CAM_LOGE("[%s] ImageBufferHeap create fail", strName_pdc_1.c_str());
        return MFALSE;
    }
    if (pImgBuf_pdc_2 == NULL) {
        CAM_LOGE("[%s] ImageBufferHeap create fail", strName_pdc_2.c_str());
        return MFALSE;
    }
    if (pImgBuf_pdc_3 == NULL) {
        CAM_LOGE("[%s] ImageBufferHeap create fail", strName_pdc_3.c_str());
        return MFALSE;
    }
    if (pImgBuf_pdc_4 == NULL) {
        CAM_LOGE("[%s] ImageBufferHeap create fail", strName_pdc_4.c_str());
        return MFALSE;
    }
    // lock buffer
    MUINT const usage_pdc_1 = (GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN |
                        GRALLOC_USAGE_HW_CAMERA_READ |
                        GRALLOC_USAGE_HW_CAMERA_WRITE);
    MUINT const usage_pdc_2 = (GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN |
                        GRALLOC_USAGE_HW_CAMERA_READ |
                        GRALLOC_USAGE_HW_CAMERA_WRITE);
    MUINT const usage_pdc_3 = (GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN |
                        GRALLOC_USAGE_HW_CAMERA_READ |
                        GRALLOC_USAGE_HW_CAMERA_WRITE);
    MUINT const usage_pdc_4 = (GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN |
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
    if (!(pImgBuf_pdc_3->lockBuf(strName_pdc_3.c_str(), usage_pdc_3)))
    {
        CAM_LOGE("[%s] Stuff ImageBuffer lock fail",  strName_pdc_3.c_str());
        return MFALSE;
    }
    if (!(pImgBuf_pdc_4->lockBuf(strName_pdc_4.c_str(), usage_pdc_4)))
    {
        CAM_LOGE("[%s] Stuff ImageBuffer lock fail",  strName_pdc_4.c_str());
        return MFALSE;
    }

    // release m_pPDCBuffer
    if (m_pPDCBuffer_1){
        sp<IImageBuffer> temp = static_cast<IImageBuffer*>(m_pPDCBuffer_1);
        temp->unlockBuf(strName_pdc_1.c_str());
    }
    if (m_pPDCBuffer_2){
        sp<IImageBuffer> temp = static_cast<IImageBuffer*>(m_pPDCBuffer_2);
        temp->unlockBuf(strName_pdc_2.c_str());
    }
    if (m_pPDCBuffer_3){
        sp<IImageBuffer> temp = static_cast<IImageBuffer*>(m_pPDCBuffer_3);
        temp->unlockBuf(strName_pdc_3.c_str());
    }
    if (m_pPDCBuffer_4){
        sp<IImageBuffer> temp = static_cast<IImageBuffer*>(m_pPDCBuffer_4);
        temp->unlockBuf(strName_pdc_4.c_str());
    }

    memcpy(reinterpret_cast<MVOID*>(pImgBuf_pdc_1->getBufVA(0)), m_pBpciBuf_1->bpci_array, m_pBpciBuf_1->bpci_xsize);
    memcpy(reinterpret_cast<MVOID*>(pImgBuf_pdc_2->getBufVA(0)), m_pBpciBuf_2->bpci_array, m_pBpciBuf_2->bpci_xsize);
    memcpy(reinterpret_cast<MVOID*>(pImgBuf_pdc_3->getBufVA(0)), m_pBpciBuf_3->bpci_array, m_pBpciBuf_3->bpci_xsize);
    memcpy(reinterpret_cast<MVOID*>(pImgBuf_pdc_4->getBufVA(0)), m_pBpciBuf_4->bpci_array, m_pBpciBuf_4->bpci_xsize);

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.dump.bpci_tbl.enable", value, "0"); // 0: enable, 1: disable
    MBOOL bDumpTblEnable = atoi(value);

    m_pPDCBuffer_1    = static_cast<MVOID*>(pImgBuf_pdc_1);
    m_pPDCBuffer_2    = static_cast<MVOID*>(pImgBuf_pdc_2);
    m_pPDCBuffer_3    = static_cast<MVOID*>(pImgBuf_pdc_3);
    m_pPDCBuffer_4    = static_cast<MVOID*>(pImgBuf_pdc_4);

    CAM_LOGD( "BPCI table load from NvRam is done");

    return MTRUE;
}


MVOID
ISP_MGR_BNR2_T::
unlockBPCIBuf()
{
    CAM_LOGD( "%s +", __FUNCTION__);

    std::lock_guard<std::mutex> lock(m_Lock);

    std::string strName_pdc_1 = "PDC_tbl_1";  //same in setPBCIBuf
    std::string strName_pdc_2 = "PDC_tbl_2";  //same in setPBCIBuf
    std::string strName_pdc_3 = "PDC_tbl_3";  //same in setPBCIBuf
    std::string strName_pdc_4 = "PDC_tbl_4";  //same in setPBCIBuf

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

    if (m_pPDCBuffer_3 != NULL)
    {
        static_cast<IImageBuffer*>(m_pPDCBuffer_3)->unlockBuf(strName_pdc_3.c_str());
        // destroy buffer
        sp<IImageBuffer> temp_pdc_3 = static_cast<IImageBuffer*>(m_pPDCBuffer_3);
        m_pPDCBuffer_3 = NULL;
    }

    if (m_pPDCBuffer_4 != NULL)
    {
        static_cast<IImageBuffer*>(m_pPDCBuffer_4)->unlockBuf(strName_pdc_4.c_str());
        // destroy buffer
        sp<IImageBuffer> temp_pdc_4 = static_cast<IImageBuffer*>(m_pPDCBuffer_4);
        m_pPDCBuffer_4 = NULL;
    }
    CAM_LOGD( "%s -", __FUNCTION__);
}

} // namespace NSIspTuningv3

