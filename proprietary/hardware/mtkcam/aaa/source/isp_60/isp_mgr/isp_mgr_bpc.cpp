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
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include "isp_mgr.h"
#include <string>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/imgbuf/IIonImageBufferHeap.h>

#include "private/PDTblGen.h"
#include <private/aaa_utils.h>
#include <array>

using namespace NSIspTuning;

#define MY_INST NS3Av3::INST_T<ISP_MGR_BPC_T>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

using namespace NSCam;

namespace NSIspTuning
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BPC
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_BPC_T&
ISP_MGR_BPC_T::
getInstance(MUINT32 const eSensorDev)
{
    ISP_MGR_MODULE_GET_INSTANCE(BPC);
}

template <>
ISP_MGR_BPC_T&
ISP_MGR_BPC_T::
put(MUINT8 SubModuleIndex, ISP_NVRAM_BPC_BPC_T const& rParam)
{
    PUT_REG_INFO_MULTI(SubModuleIndex, BPC_CON,             con);
    PUT_REG_INFO_MULTI(SubModuleIndex, BPC_BLD,             bld);
    PUT_REG_INFO_MULTI(SubModuleIndex, BPC_TH1,             th1);
    PUT_REG_INFO_MULTI(SubModuleIndex, BPC_TH2,             th2);
    PUT_REG_INFO_MULTI(SubModuleIndex, BPC_TH3,             th3);
    PUT_REG_INFO_MULTI(SubModuleIndex, BPC_TH4,             th4);
    PUT_REG_INFO_MULTI(SubModuleIndex, BPC_TH5,             th5);
    PUT_REG_INFO_MULTI(SubModuleIndex, BPC_TH6,             th6);
    PUT_REG_INFO_MULTI(SubModuleIndex, BPC_DTC,             dtc);
    PUT_REG_INFO_MULTI(SubModuleIndex, BPC_COR,             cor);
    PUT_REG_INFO_MULTI(SubModuleIndex, BPC_RANK,            rank);
    //PUT_REG_INFO_MULTI(SubModuleIndex, BPC_TBLI1,           tbli1);
    //PUT_REG_INFO_MULTI(SubModuleIndex, BPC_TBLI2,           tbli2);
    PUT_REG_INFO_MULTI(SubModuleIndex, BPC_TH1_C,           th1_c);
    PUT_REG_INFO_MULTI(SubModuleIndex, BPC_TH2_C,           th2_c);
    PUT_REG_INFO_MULTI(SubModuleIndex, BPC_TH3_C,           th3_c);
    PUT_REG_INFO_MULTI(SubModuleIndex, BPC_LL,              ll);
    PUT_REG_INFO_MULTI(SubModuleIndex, ZHDR_CON,            zhdr_con);
    //PUT_REG_INFO_MULTI(SubModuleIndex, ZHDR_RMG,            zhdr_rmg);
    PUT_REG_INFO_MULTI(SubModuleIndex, PSEUDO,              pseudo);
    return  (*this);
}


template <>
ISP_MGR_BPC_T&
ISP_MGR_BPC_T::
get(MUINT8 SubModuleIndex, ISP_NVRAM_BPC_BPC_T& rParam)
{
    GET_REG_INFO_MULTI(SubModuleIndex, BPC_CON,             con);
    GET_REG_INFO_MULTI(SubModuleIndex, BPC_BLD,             bld);
    GET_REG_INFO_MULTI(SubModuleIndex, BPC_TH1,             th1);
    GET_REG_INFO_MULTI(SubModuleIndex, BPC_TH2,             th2);
    GET_REG_INFO_MULTI(SubModuleIndex, BPC_TH3,             th3);
    GET_REG_INFO_MULTI(SubModuleIndex, BPC_TH4,             th4);
    GET_REG_INFO_MULTI(SubModuleIndex, BPC_TH5,             th5);
    GET_REG_INFO_MULTI(SubModuleIndex, BPC_TH6,             th6);
    GET_REG_INFO_MULTI(SubModuleIndex, BPC_DTC,             dtc);
    GET_REG_INFO_MULTI(SubModuleIndex, BPC_COR,             cor);
    GET_REG_INFO_MULTI(SubModuleIndex, BPC_RANK,            rank);
    //GET_REG_INFO_MULTI(SubModuleIndex, BPC_TBLI1,           tbli1);
    //GET_REG_INFO_MULTI(SubModuleIndex, BPC_TBLI2,           tbli2);
    GET_REG_INFO_MULTI(SubModuleIndex, BPC_TH1_C,           th1_c);
    GET_REG_INFO_MULTI(SubModuleIndex, BPC_TH2_C,           th2_c);
    GET_REG_INFO_MULTI(SubModuleIndex, BPC_TH3_C,           th3_c);
    GET_REG_INFO_MULTI(SubModuleIndex, BPC_LL,              ll);
    GET_REG_INFO_MULTI(SubModuleIndex, ZHDR_CON,            zhdr_con);
    //GET_REG_INFO_MULTI(SubModuleIndex, ZHDR_RMG,            zhdr_rmg);
    GET_REG_INFO_MULTI(SubModuleIndex, PSEUDO,              pseudo);

    return  (*this);
}


template <>
ISP_MGR_BPC_T&
ISP_MGR_BPC_T::
put(MUINT8 SubModuleIndex, ISP_NVRAM_BPC_CT_T const& rParam)
{
    PUT_REG_INFO_MULTI(SubModuleIndex, CT_CON1,         con1);
    PUT_REG_INFO_MULTI(SubModuleIndex, CT_CON2,         con2);
    PUT_REG_INFO_MULTI(SubModuleIndex, CT_BLD1,         bld1);
    PUT_REG_INFO_MULTI(SubModuleIndex, CT_BLD2,         bld2);
    //PUT_REG_INFO_MULTI(SubModuleIndex, RCCC_CT_CON,     rccc_ct_con);

    return  (*this);
}


template <>
ISP_MGR_BPC_T&
ISP_MGR_BPC_T::
get(MUINT8 SubModuleIndex, ISP_NVRAM_BPC_CT_T& rParam)
{
    GET_REG_INFO_MULTI(SubModuleIndex, CT_CON1,         con1);
    GET_REG_INFO_MULTI(SubModuleIndex, CT_CON2,         con2);
    GET_REG_INFO_MULTI(SubModuleIndex, CT_BLD1,         bld1);
    GET_REG_INFO_MULTI(SubModuleIndex, CT_BLD2,         bld2);
    //GET_REG_INFO_MULTI(SubModuleIndex, RCCC_CT_CON,     rccc_ct_con);

    return  (*this);
}

template <>
ISP_MGR_BPC_T&
ISP_MGR_BPC_T::
put(MUINT8 SubModuleIndex, ISP_NVRAM_BPC_PDC_T const& rParam)
{
    PUT_REG_INFO_MULTI(SubModuleIndex, PDC_CON,             con);
    PUT_REG_INFO_MULTI(SubModuleIndex, PDC_GAIN_L0,         gain_l0);
    //PUT_REG_INFO_MULTI(SubModuleIndex, PDC_GAIN_L1,         gain_l1);
    //PUT_REG_INFO_MULTI(SubModuleIndex, PDC_GAIN_L2,         gain_l2);
    //PUT_REG_INFO_MULTI(SubModuleIndex, PDC_GAIN_L3,         gain_l3);
    //PUT_REG_INFO_MULTI(SubModuleIndex, PDC_GAIN_L4,         gain_l4);
    PUT_REG_INFO_MULTI(SubModuleIndex, PDC_GAIN_R0,         gain_r0);
    //PUT_REG_INFO_MULTI(SubModuleIndex, PDC_GAIN_R1,         gain_r1);
    //PUT_REG_INFO_MULTI(SubModuleIndex, PDC_GAIN_R2,         gain_r2);
    //PUT_REG_INFO_MULTI(SubModuleIndex, PDC_GAIN_R3,         gain_r3);
    //PUT_REG_INFO_MULTI(SubModuleIndex, PDC_GAIN_R4,         gain_r4);
    PUT_REG_INFO_MULTI(SubModuleIndex, PDC_TH_GB,           th_gb);
    PUT_REG_INFO_MULTI(SubModuleIndex, PDC_TH_IA,           th_ia);
    PUT_REG_INFO_MULTI(SubModuleIndex, PDC_TH_HD,           th_hd);
    PUT_REG_INFO_MULTI(SubModuleIndex, PDC_SL,              sl);
    //PUT_REG_INFO_MULTI(SubModuleIndex, PDC_POS,             pos);
    PUT_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_GR1,     diff_th_gr1);
    PUT_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_GR2,     diff_th_gr2);
    PUT_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_GR3,     diff_th_gr3);
    PUT_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_GB1,     diff_th_gb1);
    PUT_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_GB2,     diff_th_gb2);
    PUT_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_GB3,     diff_th_gb3);
    PUT_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_R1,      diff_th_r1);
    PUT_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_R2,      diff_th_r2);
    PUT_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_R3,      diff_th_r3);
    PUT_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_B1,      diff_th_b1);
    PUT_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_B2,      diff_th_b2);
    PUT_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_B3,      diff_th_b3);
    PUT_REG_INFO_MULTI(SubModuleIndex, PDC_GAIN1,           gain1);
    PUT_REG_INFO_MULTI(SubModuleIndex, PDC_GAIN2,           gain2);
    PUT_REG_INFO_MULTI(SubModuleIndex, PD_MODE,             pd_mode);

    return  (*this);
}


template <>
ISP_MGR_BPC_T&
ISP_MGR_BPC_T::
get(MUINT8 SubModuleIndex, ISP_NVRAM_BPC_PDC_T& rParam)
{
    GET_REG_INFO_MULTI(SubModuleIndex, PDC_CON,             con);
    GET_REG_INFO_MULTI(SubModuleIndex, PDC_GAIN_L0,         gain_l0);
    //GET_REG_INFO_MULTI(SubModuleIndex, PDC_GAIN_L1,         gain_l1);
    //GET_REG_INFO_MULTI(SubModuleIndex, PDC_GAIN_L2,         gain_l2);
    //GET_REG_INFO_MULTI(SubModuleIndex, PDC_GAIN_L3,         gain_l3);
    //GET_REG_INFO_MULTI(SubModuleIndex, PDC_GAIN_L4,         gain_l4);
    GET_REG_INFO_MULTI(SubModuleIndex, PDC_GAIN_R0,         gain_r0);
    //GET_REG_INFO_MULTI(SubModuleIndex, PDC_GAIN_R1,         gain_r1);
    //GET_REG_INFO_MULTI(SubModuleIndex, PDC_GAIN_R2,         gain_r2);
    //GET_REG_INFO_MULTI(SubModuleIndex, PDC_GAIN_R3,         gain_r3);
    //GET_REG_INFO_MULTI(SubModuleIndex, PDC_GAIN_R4,         gain_r4);
    GET_REG_INFO_MULTI(SubModuleIndex, PDC_TH_GB,           th_gb);
    GET_REG_INFO_MULTI(SubModuleIndex, PDC_TH_IA,           th_ia);
    GET_REG_INFO_MULTI(SubModuleIndex, PDC_TH_HD,           th_hd);
    GET_REG_INFO_MULTI(SubModuleIndex, PDC_SL,              sl);
    //GET_REG_INFO_MULTI(SubModuleIndex, PDC_POS,             pos);
    GET_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_GR1,     diff_th_gr1);
    GET_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_GR2,     diff_th_gr2);
    GET_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_GR3,     diff_th_gr3);
    GET_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_GB1,     diff_th_gb1);
    GET_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_GB2,     diff_th_gb2);
    GET_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_GB3,     diff_th_gb3);
    GET_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_R1,      diff_th_r1);
    GET_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_R2,      diff_th_r2);
    GET_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_R3,      diff_th_r3);
    GET_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_B1,      diff_th_b1);
    GET_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_B2,      diff_th_b2);
    GET_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_B3,      diff_th_b3);
    GET_REG_INFO_MULTI(SubModuleIndex, PDC_GAIN1,           gain1);
    GET_REG_INFO_MULTI(SubModuleIndex, PDC_GAIN2,           gain2);
    GET_REG_INFO_MULTI(SubModuleIndex, PD_MODE,             pd_mode);

    return  (*this);
}

MBOOL
ISP_MGR_BPC_T::
apply_P1(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, TuningMgr& rTuning, MINT32 i4SubsampleIdex)
{
    MBOOL bBPC_Enable = reinterpret_cast<REG_BPC_R1_BPC_BPC_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BPC_CON))->Bits.BPC_BPC_EN
                       & isBPCEnable(SubModuleIndex);
    MBOOL bCT_Enable = reinterpret_cast<REG_BPC_R1_BPC_CT_CON1*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, CT_CON1))->Bits.BPC_CT_EN
                       & isCTEnable(SubModuleIndex);
    MBOOL bPDC_Enable = reinterpret_cast<REG_BPC_R1_BPC_PDC_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, PDC_CON))->Bits.BPC_PDC_EN
                       & isPDCEnable(SubModuleIndex);

    if(bPDC_Enable){

        Tbl P1BPCITbl_Info;

        m_bBPCIValid[SubModuleIndex] = IPDTblGen::getInstance()->getTbl( m_eSensorDev, rRawIspCamInfo.rMapping_Info.eSensorMode, P1BPCITbl_Info);

        if (m_bBPCIValid[SubModuleIndex]){

            if(rRawIspCamInfo.rMapping_Info.eFrontBin == EFrontBin_Yes){
                m_sPDOHWInfo.i4memID         = P1BPCITbl_Info.tbl_bin.memID;
                m_sPDOHWInfo.u4Bpci_xsz      = P1BPCITbl_Info.tbl_bin.tbl_xsz;
                m_sPDOHWInfo.u4Bpci_ysz      = P1BPCITbl_Info.tbl_bin.tbl_ysz;
                m_sPDOHWInfo.phyAddrBpci_tbl = (MUINTPTR)P1BPCITbl_Info.tbl_bin.tbl_pa;
                m_sPDOHWInfo.virAddrBpci_tbl = (MUINTPTR)P1BPCITbl_Info.tbl_bin.tbl_va;
            }
            else{
                m_sPDOHWInfo.i4memID         = P1BPCITbl_Info.tbl.memID;
                m_sPDOHWInfo.u4Bpci_xsz      = P1BPCITbl_Info.tbl.tbl_xsz;
                m_sPDOHWInfo.u4Bpci_ysz      = P1BPCITbl_Info.tbl.tbl_ysz;
                m_sPDOHWInfo.phyAddrBpci_tbl = (MUINTPTR)P1BPCITbl_Info.tbl.tbl_pa;
                m_sPDOHWInfo.virAddrBpci_tbl = (MUINTPTR)P1BPCITbl_Info.tbl.tbl_va;
            }

            reinterpret_cast<REG_BPC_R1_BPC_PDC_GAIN_L0*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, PDC_GAIN_L0))->Bits.BPC_PDC_GCF_L10 = 0;
            reinterpret_cast<REG_BPC_R1_BPC_PDC_GAIN_R0*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, PDC_GAIN_R0))->Bits.BPC_PDC_GCF_R10 = 0;
            reinterpret_cast<REG_BPC_R1_BPC_PDC_TH_GB*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, PDC_TH_GB))->Bits.BPC_PDC_GTH = 4095;
            reinterpret_cast<REG_BPC_R1_BPC_PDC_TH_IA*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, PDC_TH_IA))->Bits.BPC_PDC_ATH = 4094;

            m_rIspRegInfo_BPCI_Pass1[i4SubsampleIdex][ERegInfo_BASE_ADDR].val = m_sPDOHWInfo.phyAddrBpci_tbl;
            m_rIspRegInfo_BPCI_Pass1[i4SubsampleIdex][ERegInfo_XSIZE].val     = m_sPDOHWInfo.u4Bpci_xsz+1;
            m_rIspRegInfo_BPCI_Pass1[i4SubsampleIdex][ERegInfo_YSIZE].val     = 1;
            m_rIspRegInfo_BPCI_Pass1[i4SubsampleIdex][ERegInfo_STRIDE].val    = m_sPDOHWInfo.u4Bpci_xsz+1;

        }
        else{
            bPDC_Enable = MFALSE;

            CAM_LOGE("%s(): Fail to Gen PDC Table, SensorDev(%d), SensorMode(%d), FrontBin(%d)\n",
            __FUNCTION__,
            m_eSensorDev,
            rRawIspCamInfo.rMapping_Info.eSensorMode,
            rRawIspCamInfo.rMapping_Info.eFrontBin);
        }
    }

    MBOOL bEnable = bBPC_Enable | bCT_Enable | bPDC_Enable;

    setBPCEnable(SubModuleIndex, bBPC_Enable);
    setCTEnable(SubModuleIndex, bCT_Enable);
    setPDCEnable(SubModuleIndex, bPDC_Enable);

    //BNR HW limitation
    if (bEnable && (bBPC_Enable == 0))
    {
        MY_LOG("BPC(%d)::apply constraint bEnable(%d), bBPC_EN(%d), bCT_EN(%d)", SubModuleIndex, bEnable, bBPC_Enable, bCT_Enable);
        bEnable = 0;
        bCT_Enable = 0;
    }

    //Top Control
    switch (SubModuleIndex)
    {
        case EBPC_R1:
            ISP_MGR_CAMCTL_T::getInstance(m_eSensorDev).setEnable_BPC_R1(bEnable);
            rTuning.updateEngine(eTuningMgrFunc_BPC_R1, bEnable, i4SubsampleIdex);
            if(bPDC_Enable){
                rTuning.updateEngine(eTuningMgrFunc_BPCI_R1, bPDC_Enable, i4SubsampleIdex);
                rTuning.updateEngineFD(eTuningMgrFunc_BPCI_R1, i4SubsampleIdex, m_sPDOHWInfo.i4memID, m_sPDOHWInfo.virAddrBpci_tbl);
            }
            break;
#if 0
        case EOBC_R2:
            ISP_MGR_CAMCTL_T::getInstance(m_eSensorDev).setEnable_OBC_R2(bEnable);
            rTuning.updateEngine(eTuningMgrFunc_OBC, bEnable, i4SubsampleIdex);
            break;
#endif
        default:
            CAM_LOGE("Apply Error Submodule Index: %d", SubModuleIndex);
            return  MFALSE;
    }


    if(bEnable){
        //Sub module control
        reinterpret_cast<REG_BPC_R1_BPC_BPC_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BPC_CON))->Bits.BPC_BPC_EN = bBPC_Enable;
        reinterpret_cast<REG_BPC_R1_BPC_CT_CON1*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, CT_CON1))->Bits.BPC_CT_EN = bCT_Enable;
        reinterpret_cast<REG_BPC_R1_BPC_PDC_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, PDC_CON))->Bits.BPC_PDC_EN = bPDC_Enable;

        reinterpret_cast<REG_BPC_R1_BPC_BPC_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BPC_CON))->Bits.BPC_BPC_AUX_MAP_EN = 0;
        reinterpret_cast<REG_BPC_R1_BPC_BPC_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BPC_CON))->Bits.BPC_BPC_AUX_MAP_OR_EN = 0;
        reinterpret_cast<REG_BPC_R1_BPC_BPC_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BPC_CON))->Bits.BPC_LE_INV_CTL = 0;
        reinterpret_cast<REG_BPC_R1_BPC_BPC_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BPC_CON))->Bits.BPC_BPC_LUT_EN = bPDC_Enable;

        //Chooo should be after ZFUS
        MBOOL bZHDR_Enable = MFALSE; //Chooo, need ZHDR Info
        reinterpret_cast<REG_BPC_R1_BPC_ZHDR_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, ZHDR_CON))->Bits.BPC_ZHDR_EN &= bZHDR_Enable;

#if 0
        if(bZHDR_Enable){
            MUINT32 ZFUS_OSC_TH = 4080; //Chooo, should get data from ZFUS module
            reinterpret_cast<REG_BPC_R1_BPC_ZHDR_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, ZHDR_CON))->Bits.BPC_ZHDR_OSC_TH = (ZFUS_OSC_TH >> 3);

            reinterpret_cast<REG_BPC_R1_BPC_ZHDR_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, ZHDR_CON))->Bits.BPC_ZHDR_BLE_FIRST = 0; // Chooo ZZFUS_ZHDR_BLE_FIRST;
            reinterpret_cast<REG_BPC_R1_BPC_ZHDR_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, ZHDR_CON))->Bits.BPC_ZHDR_RLE_FIRST = 0; // Chooo ZZFUS_ZHDR_RLE_FIRST;
            reinterpret_cast<REG_BPC_R1_BPC_ZHDR_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, ZHDR_CON))->Bits.BPC_ZHDR_GLE_FIRST = 0; // Chooo ZFUS_ZHDR_GLE_FIRST;

            MINT32 HDR_RATIO = (rRawIspCamInfo.rAEInfo.i4LESE_Ratio * 16 + 50) / 100; //only focus on zHDR
            reinterpret_cast<REG_BPC_R1_BPC_ZHDR_RMG*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, ZHDR_RMG))->Bits.BPC_ZHDR_RATIO = HDR_RATIO;
            reinterpret_cast<REG_BPC_R1_BPC_ZHDR_RMG*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, ZHDR_RMG))->Bits.BPC_ZHDR_GAIN = (16 * 256 + (HDR_RATIO/2))/ HDR_RATIO;

            if(!bPDC_Enable){
                reinterpret_cast<REG_BPC_R1_BPC_PD_MODE*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, PD_MODE))->Bits.BPC_ZHDR_OSC_PD_MODE = 1;
            }
        }

#endif
    }

    AAA_TRACE_DRV(DRV_BPC);

    // Register setting
    rTuning.tuningMgrWriteRegs( (TUNING_MGR_REG_IO_STRUCT*)(&(m_rIspRegInfo[SubModuleIndex][0])),
            m_u4RegInfoNum, i4SubsampleIdex);

    if(bPDC_Enable){
        rTuning.tuningMgrWriteRegs( (TUNING_MGR_REG_IO_STRUCT*)(&(m_rIspRegInfo_BPCI_Pass1[SubModuleIndex][0])),
                ERegInfo_BPCI_NUM, i4SubsampleIdex);
    }
    AAA_TRACE_END_DRV;

    dumpRegInfoP1("BPC", SubModuleIndex);

    return  MTRUE;
}

MBOOL
ISP_MGR_BPC_T::
apply_P2(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, dip_x_reg_t* pReg)
{
    MBOOL bBPC_Enable = reinterpret_cast<BPC_REG_D1A_BPC_BPC_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BPC_CON))->Bits.BPC_BPC_EN
                       & isBPCEnable(SubModuleIndex);
    MBOOL bCT_Enable = reinterpret_cast<BPC_REG_D1A_BPC_CT_CON1*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, CT_CON1))->Bits.BPC_CT_EN
                       & isCTEnable(SubModuleIndex);
    MBOOL bPDC_Enable = reinterpret_cast<BPC_REG_D1A_BPC_PDC_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, PDC_CON))->Bits.BPC_PDC_EN
                       & isPDCEnable(SubModuleIndex);

    if(!bPDC_Enable){
        m_pBPCI_ImgBuf = NULL;
    }

    MBOOL bEnable = bBPC_Enable | bCT_Enable | bPDC_Enable;

    setBPCEnable(SubModuleIndex, bBPC_Enable);
    setCTEnable(SubModuleIndex, bCT_Enable);
    setPDCEnable(SubModuleIndex, bPDC_Enable);

    //BNR HW limitation
    if (bEnable && (bBPC_Enable == 0))
    {
        MY_LOG("BPC(%d)::apply constraint bEnable(%d), bBPC_EN(%d), bCT_EN(%d)", SubModuleIndex, bEnable, bBPC_Enable, bCT_Enable);
        bEnable = 0;
        bCT_Enable = 0;
    }

    //Top Control
    switch (SubModuleIndex)
    {
        case EBPC_D1:
            ISP_WRITE_ENABLE_BITS(pReg, DIPCTL_D1A_DIPCTL_RGB_EN1, DIPCTL_BPC_D1_EN, bEnable);
            break;
        default:
            CAM_LOGE("Apply Error Submodule Index: %d", SubModuleIndex);
            return  MFALSE;
    }

    if(bEnable){
        //Sub module control
        reinterpret_cast<BPC_REG_D1A_BPC_BPC_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BPC_CON))->Bits.BPC_BPC_EN = bBPC_Enable;
        reinterpret_cast<BPC_REG_D1A_BPC_CT_CON1*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, CT_CON1))->Bits.BPC_CT_EN = bCT_Enable;
        reinterpret_cast<BPC_REG_D1A_BPC_PDC_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, PDC_CON))->Bits.BPC_PDC_EN = bPDC_Enable;

        //BPC
        reinterpret_cast<BPC_REG_D1A_BPC_BPC_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BPC_CON))->Bits.BPC_BPC_AUX_MAP_EN = 0;
        reinterpret_cast<BPC_REG_D1A_BPC_BPC_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BPC_CON))->Bits.BPC_BPC_AUX_MAP_OR_EN = 0;
        reinterpret_cast<BPC_REG_D1A_BPC_BPC_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BPC_CON))->Bits.BPC_LE_INV_CTL = 0;
        reinterpret_cast<BPC_REG_D1A_BPC_BPC_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BPC_CON))->Bits.BPC_BPC_LUT_EN = bPDC_Enable;

        //Chooo should be after ZFUS
        MBOOL bZHDR_Enable = MFALSE; //Chooo, need ZHDR Info
        reinterpret_cast<BPC_REG_D1A_BPC_ZHDR_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, ZHDR_CON))->Bits.BPC_ZHDR_EN &= bZHDR_Enable;


        if(bPDC_Enable){ //PDC
            reinterpret_cast<BPC_REG_D1A_BPC_PDC_GAIN_L0*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, PDC_GAIN_L0))->Bits.BPC_PDC_GCF_L10 = 0;
            reinterpret_cast<BPC_REG_D1A_BPC_PDC_GAIN_R0*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, PDC_GAIN_R0))->Bits.BPC_PDC_GCF_R10 = 0;
            reinterpret_cast<BPC_REG_D1A_BPC_PDC_TH_GB*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, PDC_TH_GB))->Bits.BPC_PDC_GTH = 4095;
            reinterpret_cast<BPC_REG_D1A_BPC_PDC_TH_IA*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, PDC_TH_IA))->Bits.BPC_PDC_ATH = 4094;
        }
#if 0
        if(bZHDR_Enable){
            MUINT32 ZFUS_OSC_TH = 4080; //Chooo, should get data from ZFUS module
            reinterpret_cast<BPC_REG_D1A_BPC_ZHDR_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, ZHDR_CON))->Bits.BPC_ZHDR_OSC_TH = (ZFUS_OSC_TH >> 3);

            reinterpret_cast<BPC_REG_D1A_BPC_ZHDR_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, ZHDR_CON))->Bits.BPC_ZHDR_BLE_FIRST = 0; // Chooo ZZFUS_ZHDR_BLE_FIRST;
            reinterpret_cast<BPC_REG_D1A_BPC_ZHDR_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, ZHDR_CON))->Bits.BPC_ZHDR_RLE_FIRST = 0; // Chooo ZZFUS_ZHDR_RLE_FIRST;
            reinterpret_cast<BPC_REG_D1A_BPC_ZHDR_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, ZHDR_CON))->Bits.BPC_ZHDR_GLE_FIRST = 0; // Chooo ZFUS_ZHDR_GLE_FIRST;

            MINT32 HDR_RATIO = (rRawIspCamInfo.rAEInfo.i4LESE_Ratio * 16 + 50) / 100; //only focus on zHDR
            reinterpret_cast<BPC_REG_D1A_BPC_ZHDR_RMG*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, ZHDR_RMG))->Bits.BPC_ZHDR_RATIO = HDR_RATIO;
            reinterpret_cast<BPC_REG_D1A_BPC_ZHDR_RMG*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, ZHDR_RMG))->Bits.BPC_ZHDR_GAIN = (16 * 256 + (HDR_RATIO/2))/ HDR_RATIO;

            if(!bPDC_Enable){
                reinterpret_cast<BPC_REG_D1A_BPC_PD_MODE*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, PD_MODE))->Bits.BPC_ZHDR_OSC_PD_MODE = 1;
            }
        }
#endif

        writeRegs(static_cast<RegInfo_T*>(m_rIspRegInfo[SubModuleIndex]), m_u4RegInfoNum, pReg);

        dumpRegInfoP2("BPC", SubModuleIndex);
    }

    return  MTRUE;
}

}

