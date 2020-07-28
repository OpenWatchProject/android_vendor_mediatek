/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "isp_mgr_bpc"

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

namespace NSIspTuning
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BPC
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ISP_MGR_BPC_T&
ISP_MGR_BPC_T::
getInstance(ESensorDev_T const eSensorDev)
{
    ISP_MGR_MODULE_GET_INSTANCE(BPC);
}

template <>
ISP_MGR_BPC_T&
ISP_MGR_BPC_T::
put(MUINT8 SubModuleIndex, ISP_NVRAM_BPC_BPC_T const& rParam)
{
    if (SubModuleIndex >= ESubModule_NUM) {
        CAM_LOGE("[PUT] Error Submodule Index: %d", SubModuleIndex);
        return (*this);
    }
    PUT_REG_INFO_MULTI(SubModuleIndex, BPC_FUNC_CON,              func_con);
    PUT_REG_INFO_MULTI(SubModuleIndex, BPC_IN_TH_1,               in_th_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, BPC_IN_TH_2,               in_th_2);
    PUT_REG_INFO_MULTI(SubModuleIndex, BPC_IN_TH_3,               in_th_3);
    PUT_REG_INFO_MULTI(SubModuleIndex, BPC_IN_TH_4,               in_th_4);
    PUT_REG_INFO_MULTI(SubModuleIndex, BPC_IN_TH_5,               in_th_5);
    PUT_REG_INFO_MULTI(SubModuleIndex, BPC_C_TH_1,                c_th_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, BPC_C_TH_2,                c_th_2);
    PUT_REG_INFO_MULTI(SubModuleIndex, BPC_C_TH_3,                c_th_3);
    PUT_REG_INFO_MULTI(SubModuleIndex, BPC_DTC,                   dtc);
    PUT_REG_INFO_MULTI(SubModuleIndex, BPC_COR,                   cor);
    PUT_REG_INFO_MULTI(SubModuleIndex, BPC_RANK,                  rank);
    PUT_REG_INFO_MULTI(SubModuleIndex, BPC_BLD_1,                 bld_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, BPC_BLD_2,                 bld_2);
    PUT_REG_INFO_MULTI(SubModuleIndex, BPC_LL,                    ll);
    PUT_REG_INFO_MULTI(SubModuleIndex, BPC_ZHDR_TUN,              zhdr_tun);
    PUT_REG_INFO_MULTI(SubModuleIndex, BPC_OBC_ZOFST,             obc_zofst);
    PUT_REG_INFO_MULTI(SubModuleIndex, BPC_PD_EXC_CON,            pd_exc_con);


    return (*this);
}

template <>
ISP_MGR_BPC_T&
ISP_MGR_BPC_T::
get(MUINT8 SubModuleIndex, ISP_NVRAM_BPC_BPC_T& rParam)
{
    if (SubModuleIndex >= ESubModule_NUM) {
        CAM_LOGE("[GET] Error Submodule Index: %d", SubModuleIndex);
        return (*this);
    }
    GET_REG_INFO_MULTI(SubModuleIndex, BPC_FUNC_CON,              func_con); 
    GET_REG_INFO_MULTI(SubModuleIndex, BPC_IN_TH_1,               in_th_1);  
    GET_REG_INFO_MULTI(SubModuleIndex, BPC_IN_TH_2,               in_th_2);  
    GET_REG_INFO_MULTI(SubModuleIndex, BPC_IN_TH_3,               in_th_3);  
    GET_REG_INFO_MULTI(SubModuleIndex, BPC_IN_TH_4,               in_th_4);  
    GET_REG_INFO_MULTI(SubModuleIndex, BPC_IN_TH_5,               in_th_5);  
    GET_REG_INFO_MULTI(SubModuleIndex, BPC_C_TH_1,                c_th_1);   
    GET_REG_INFO_MULTI(SubModuleIndex, BPC_C_TH_2,                c_th_2);   
    GET_REG_INFO_MULTI(SubModuleIndex, BPC_C_TH_3,                c_th_3);   
    GET_REG_INFO_MULTI(SubModuleIndex, BPC_DTC,                   dtc);      
    GET_REG_INFO_MULTI(SubModuleIndex, BPC_COR,                   cor);      
    GET_REG_INFO_MULTI(SubModuleIndex, BPC_RANK,                  rank);     
    GET_REG_INFO_MULTI(SubModuleIndex, BPC_BLD_1,                 bld_1);    
    GET_REG_INFO_MULTI(SubModuleIndex, BPC_BLD_2,                 bld_2);    
    GET_REG_INFO_MULTI(SubModuleIndex, BPC_LL,                    ll);       
    GET_REG_INFO_MULTI(SubModuleIndex, BPC_ZHDR_TUN,              zhdr_tun); 
    GET_REG_INFO_MULTI(SubModuleIndex, BPC_OBC_ZOFST,             obc_zofst);
    GET_REG_INFO_MULTI(SubModuleIndex, BPC_PD_EXC_CON,            pd_exc_con);

    return (*this);
}


template <>
ISP_MGR_BPC_T&
ISP_MGR_BPC_T::
put(MUINT8 SubModuleIndex, ISP_NVRAM_BPC_CT_T const& rParam)
{
    if (SubModuleIndex >= ESubModule_NUM) {
        CAM_LOGE("[PUT] Error Submodule Index: %d", SubModuleIndex);
        return (*this);
    }
    PUT_REG_INFO_MULTI(SubModuleIndex, CT_CON,                    con);
    PUT_REG_INFO_MULTI(SubModuleIndex, CT_BLD_1,                  bld_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, CT_BLD_2,                  bld_2);
    return (*this);
}

template <>
ISP_MGR_BPC_T&
ISP_MGR_BPC_T::
get(MUINT8 SubModuleIndex, ISP_NVRAM_BPC_CT_T& rParam)
{
    if (SubModuleIndex >= ESubModule_NUM) {
        CAM_LOGE("[GET] Error Submodule Index: %d", SubModuleIndex);
        return (*this);
    }
    GET_REG_INFO_MULTI(SubModuleIndex, CT_CON,                    con);
    GET_REG_INFO_MULTI(SubModuleIndex, CT_BLD_1,                  bld_1);
    GET_REG_INFO_MULTI(SubModuleIndex, CT_BLD_2,                  bld_2);
    return (*this);
}


template <>
ISP_MGR_BPC_T&
ISP_MGR_BPC_T::
put(MUINT8 SubModuleIndex, ISP_NVRAM_BPC_PDC_T const& rParam)
{
    if (SubModuleIndex >= ESubModule_NUM) {
        CAM_LOGE("[PUT] Error Submodule Index: %d", SubModuleIndex);
        return (*this);
    }
    PUT_REG_INFO_MULTI(SubModuleIndex, PDC_CON,                   con);
    PUT_REG_INFO_MULTI(SubModuleIndex, PDC_CUR_BI,                cur_bi);
    PUT_REG_INFO_MULTI(SubModuleIndex, PDC_CUR_AG,                cur_ag);
    PUT_REG_INFO_MULTI(SubModuleIndex, PDC_TH_N,                  th_n);
    PUT_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_GR_1,          diff_th_gr_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_GR_2,          diff_th_gr_2);
    PUT_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_GR_3,          diff_th_gr_3);
    PUT_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_GB_1,          diff_th_gb_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_GB_2,          diff_th_gb_2);
    PUT_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_GB_3,          diff_th_gb_3);
    PUT_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_R_1,           diff_th_r_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_R_2,           diff_th_r_2);
    PUT_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_R_3,           diff_th_r_3);
    PUT_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_B_1,           diff_th_b_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_B_2,           diff_th_b_2);
    PUT_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_B_3,           diff_th_b_3);
    PUT_REG_INFO_MULTI(SubModuleIndex, PDC_BLD_WT,                bld_wt);
    PUT_REG_INFO_MULTI(SubModuleIndex, PDC_GAIN_G,                gain_g);
    PUT_REG_INFO_MULTI(SubModuleIndex, PDC_GAIN_RB,               gain_rb);

    //PUT_REG_INFO_MULTI(SubModuleIndex, BPC_AUX_MAP_FW,            bpc_aux_map_fw);
    //PUT_REG_INFO_MULTI(SubModuleIndex, CT_RCCC_CT_FW,             ct_rccc_ct_fw);
    //PUT_REG_INFO_MULTI(SubModuleIndex, PDC_CUR_AG_FW,             pdc_cur_ag_fw);
    //PUT_REG_INFO_MULTI(SubModuleIndex, BPC_ZHDR_CON_FW,           bpc_zhdr_con_fw);
    //PUT_REG_INFO_MULTI(SubModuleIndex, BPC_ZHDR_RMG_FW,           bpc_zhdr_rmg_fw);
    //PUT_REG_INFO_MULTI(SubModuleIndex, BPC_OBC_OFST_R_FW,         bpc_obc_ofst_r_fw);
    //PUT_REG_INFO_MULTI(SubModuleIndex, BPC_OBC_OFST_B_FW,         bpc_obc_ofst_b_fw);
    //PUT_REG_INFO_MULTI(SubModuleIndex, BPC_OBC_OFST_GR_FW,        bpc_obc_ofst_gr_fw);
    //PUT_REG_INFO_MULTI(SubModuleIndex, BPC_OBC_OFST_GB_FW,        bpc_obc_ofst_gb_fw);
    return (*this);
}

template <>
ISP_MGR_BPC_T&
ISP_MGR_BPC_T::
get(MUINT8 SubModuleIndex, ISP_NVRAM_BPC_PDC_T& rParam)
{
    if (SubModuleIndex >= ESubModule_NUM) {
        CAM_LOGE("[GET] Error Submodule Index: %d", SubModuleIndex);
        return (*this);
    }
    GET_REG_INFO_MULTI(SubModuleIndex, PDC_CON,                   con);
    GET_REG_INFO_MULTI(SubModuleIndex, PDC_CUR_BI,                cur_bi);
    GET_REG_INFO_MULTI(SubModuleIndex, PDC_CUR_AG,                cur_ag);
    GET_REG_INFO_MULTI(SubModuleIndex, PDC_TH_N,                  th_n);
    GET_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_GR_1,          diff_th_gr_1);
    GET_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_GR_2,          diff_th_gr_2);
    GET_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_GR_3,          diff_th_gr_3);
    GET_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_GB_1,          diff_th_gb_1);
    GET_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_GB_2,          diff_th_gb_2);
    GET_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_GB_3,          diff_th_gb_3);
    GET_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_R_1,           diff_th_r_1);
    GET_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_R_2,           diff_th_r_2);
    GET_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_R_3,           diff_th_r_3);
    GET_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_B_1,           diff_th_b_1);
    GET_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_B_2,           diff_th_b_2);
    GET_REG_INFO_MULTI(SubModuleIndex, PDC_DIFF_TH_B_3,           diff_th_b_3);
    GET_REG_INFO_MULTI(SubModuleIndex, PDC_BLD_WT,                bld_wt);
    GET_REG_INFO_MULTI(SubModuleIndex, PDC_GAIN_G,                gain_g);
    GET_REG_INFO_MULTI(SubModuleIndex, PDC_GAIN_RB,               gain_rb);


    //GET_REG_INFO_MULTI(SubModuleIndex, BPC_AUX_MAP_FW,            bpc_aux_map_fw);
    //GET_REG_INFO_MULTI(SubModuleIndex, CT_RCCC_CT_FW,             ct_rccc_ct_fw);
    //GET_REG_INFO_MULTI(SubModuleIndex, PDC_CUR_AG_FW,             pdc_cur_ag_fw);
    //GET_REG_INFO_MULTI(SubModuleIndex, BPC_ZHDR_CON_FW,           bpc_zhdr_con_fw);
    //GET_REG_INFO_MULTI(SubModuleIndex, BPC_ZHDR_RMG_FW,           bpc_zhdr_rmg_fw);
    //GET_REG_INFO_MULTI(SubModuleIndex, BPC_OBC_OFST_R_FW,         bpc_obc_ofst_r_fw);
    //GET_REG_INFO_MULTI(SubModuleIndex, BPC_OBC_OFST_B_FW,         bpc_obc_ofst_b_fw);
    //GET_REG_INFO_MULTI(SubModuleIndex, BPC_OBC_OFST_GR_FW,        bpc_obc_ofst_gr_fw);
    //GET_REG_INFO_MULTI(SubModuleIndex, BPC_OBC_OFST_GB_FW,        bpc_obc_ofst_gb_fw);
    return (*this);
}


MVOID
ISP_MGR_BPC_T::
setOBCOffset(MUINT8 SubModuleIndex, ISP_NVRAM_OBC_T const& rParam){
    if (SubModuleIndex >= ESubModule_NUM) {
        CAM_LOGE("[GET] Error Submodule Index: %d", SubModuleIndex);
        return;
    }
	mObc[SubModuleIndex][EBPC_OBC_OFST_R] = rParam.offset_r.val;
	mObc[SubModuleIndex][EBPC_OBC_OFST_B] = rParam.offset_b.val;
	mObc[SubModuleIndex][EBPC_OBC_OFST_GR] = rParam.offset_gr.val;
	mObc[SubModuleIndex][EBPC_OBC_OFST_GB] = rParam.offset_gb.val;
}

MBOOL
ISP_MGR_BPC_T::
apply_P1(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, TuningMgr& rTuning, MINT32 i4SubsampleIdex)
{
    if (SubModuleIndex >= ESubModule_NUM) {
        CAM_LOGE("[GET] Error Submodule Index: %d", SubModuleIndex);
        return MFALSE;
    }
    MBOOL bBPC_Enable = reinterpret_cast<REG_BPC_R1_BPC_BPC_FUNC_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BPC_FUNC_CON))->Bits.BPC_BPC_EN
                       & isBPCEnable(SubModuleIndex);
    MBOOL bCT_Enable = reinterpret_cast<REG_BPC_R1_BPC_BPC_FUNC_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BPC_FUNC_CON))->Bits.BPC_CT_EN
                       & isCTEnable(SubModuleIndex);
    MBOOL bPDC_Enable = reinterpret_cast<REG_BPC_R1_BPC_BPC_FUNC_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BPC_FUNC_CON))->Bits.BPC_PDC_EN
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
        reinterpret_cast<REG_BPC_R1_BPC_BPC_FUNC_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BPC_FUNC_CON))->Bits.BPC_BPC_EN = bBPC_Enable;
        reinterpret_cast<REG_BPC_R1_BPC_BPC_FUNC_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BPC_FUNC_CON))->Bits.BPC_CT_EN = bCT_Enable;
        reinterpret_cast<REG_BPC_R1_BPC_BPC_FUNC_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BPC_FUNC_CON))->Bits.BPC_PDC_EN = bPDC_Enable;

        reinterpret_cast<REG_BPC_R1_BPC_BPC_AUX_MAP_FW*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BPC_AUX_MAP_FW))->Bits.BPC_BPC_AUX_MAP_EN = 0;
        reinterpret_cast<REG_BPC_R1_BPC_BPC_AUX_MAP_FW*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BPC_AUX_MAP_FW))->Bits.BPC_BPC_AUX_MAP_OR_EN = 0;
        reinterpret_cast<REG_BPC_R1_BPC_BPC_CON_TILE*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BPC_CON_TILE))->Bits.BPC_LE_INV_CTL = 0;
        reinterpret_cast<REG_BPC_R1_BPC_BPC_FUNC_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BPC_FUNC_CON))->Bits.BPC_BPC_LUT_EN = bPDC_Enable;

        reinterpret_cast<REG_BPC_R1_BPC_BPC_OBC_OFST_R_FW*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BPC_OBC_OFST_R_FW))->Bits.BPC_OBC_OFST_R = mObc[SubModuleIndex][EBPC_OBC_OFST_R];
        reinterpret_cast<REG_BPC_R1_BPC_BPC_OBC_OFST_B_FW*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BPC_OBC_OFST_B_FW))->Bits.BPC_OBC_OFST_B = mObc[SubModuleIndex][EBPC_OBC_OFST_B];
        reinterpret_cast<REG_BPC_R1_BPC_BPC_OBC_OFST_GR_FW*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BPC_OBC_OFST_GR_FW))->Bits.BPC_OBC_OFST_GR = mObc[SubModuleIndex][EBPC_OBC_OFST_GR];
        reinterpret_cast<REG_BPC_R1_BPC_BPC_OBC_OFST_GB_FW*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BPC_OBC_OFST_GB_FW))->Bits.BPC_OBC_OFST_GB = mObc[SubModuleIndex][EBPC_OBC_OFST_GB];

        //Chooo should be after ZFUS
        MBOOL bZHDR_Enable = MFALSE; //Chooo, need ZHDR Info
        reinterpret_cast<REG_BPC_R1_BPC_BPC_ZHDR_CON_FW*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BPC_ZHDR_CON_FW))->Bits.BPC_ZHDR_EN &= bZHDR_Enable;

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
			char value[PROPERTY_VALUE_MAX] = {'\0'};
			property_get("vendor.debug.isp_mgr.dump", value, "0");
			MBOOL bEn = atoi(value);
		
#define ISP_MGR_BPC_PARAM_SIZE       (44)
					
			if (bEn == 1){
				char strTuningFile[512] = {'\0'};
				sprintf(strTuningFile, "data/vendor/camera_dump/bpc_mgr.param");
				FILE* fidTuning = fopen(strTuningFile, "wb");
				if (fidTuning)
				{
				  fwrite(&(m_rIspRegInfo[SubModuleIndex][0]), sizeof(RegInfo_T)*ISP_MGR_BPC_PARAM_SIZE, 1, fidTuning);
				  fclose(fidTuning);
				}
			}
			if (bEn == 2){
				FILE* fidTuning = fopen("data/vendor/camera_dump/bpc_mgr.param", "rb");
				if (fidTuning)
				{
					RegInfo_T	tempInfo[ISP_MGR_BPC_PARAM_SIZE];
					fread((&tempInfo), sizeof(tempInfo), 1, fidTuning);
					fclose(fidTuning);
					rTuning.tuningMgrWriteRegs( (TUNING_MGR_REG_IO_STRUCT*)(&(tempInfo)),
						ISP_MGR_BPC_PARAM_SIZE, i4SubsampleIdex);
				}
			} else {
				rTuning.tuningMgrWriteRegs( (TUNING_MGR_REG_IO_STRUCT*)(&(m_rIspRegInfo[SubModuleIndex][0])),
					m_u4RegInfoNum, i4SubsampleIdex);
			}


    if(bPDC_Enable){
        rTuning.tuningMgrWriteRegs( (TUNING_MGR_REG_IO_STRUCT*)(&(m_rIspRegInfo_BPCI_Pass1[SubModuleIndex][0])),
                ERegInfo_BPCI_NUM, i4SubsampleIdex);
    }
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
apply_P2(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, dip_a_reg_t* pReg)
{
    if (SubModuleIndex >= ESubModule_NUM) {
        CAM_LOGE("[GET] Error Submodule Index: %d", SubModuleIndex);
        return MFALSE;
    }
    MBOOL bBPC_Enable = reinterpret_cast<BPC_REG_D1A_BPC_BPC_FUNC_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BPC_FUNC_CON))->Bits.BPC_BPC_EN
                       & isBPCEnable(SubModuleIndex);
    MBOOL bCT_Enable = reinterpret_cast<BPC_REG_D1A_BPC_BPC_FUNC_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BPC_FUNC_CON))->Bits.BPC_CT_EN
                       & isCTEnable(SubModuleIndex);
    MBOOL bPDC_Enable = reinterpret_cast<BPC_REG_D1A_BPC_BPC_FUNC_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BPC_FUNC_CON))->Bits.BPC_PDC_EN
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
        reinterpret_cast<BPC_REG_D1A_BPC_BPC_FUNC_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BPC_FUNC_CON))->Bits.BPC_BPC_EN = bBPC_Enable;
        reinterpret_cast<BPC_REG_D1A_BPC_BPC_FUNC_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BPC_FUNC_CON))->Bits.BPC_CT_EN = bCT_Enable;
        reinterpret_cast<BPC_REG_D1A_BPC_BPC_FUNC_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BPC_FUNC_CON))->Bits.BPC_PDC_EN = bPDC_Enable;

        //BPC
        reinterpret_cast<BPC_REG_D1A_BPC_BPC_AUX_MAP_FW*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BPC_AUX_MAP_FW))->Bits.BPC_BPC_AUX_MAP_EN = 0;
        reinterpret_cast<BPC_REG_D1A_BPC_BPC_AUX_MAP_FW*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BPC_AUX_MAP_FW))->Bits.BPC_BPC_AUX_MAP_OR_EN = 0;
        reinterpret_cast<BPC_REG_D1A_BPC_BPC_CON_TILE*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BPC_CON_TILE))->Bits.BPC_LE_INV_CTL = 0;
        reinterpret_cast<BPC_REG_D1A_BPC_BPC_FUNC_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BPC_FUNC_CON))->Bits.BPC_BPC_LUT_EN = bPDC_Enable;

        reinterpret_cast<BPC_REG_D1A_BPC_BPC_OBC_OFST_R_FW*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BPC_OBC_OFST_R_FW))->Bits.BPC_OBC_OFST_R = mObc[SubModuleIndex][EBPC_OBC_OFST_R];
        reinterpret_cast<BPC_REG_D1A_BPC_BPC_OBC_OFST_B_FW*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BPC_OBC_OFST_B_FW))->Bits.BPC_OBC_OFST_B = mObc[SubModuleIndex][EBPC_OBC_OFST_B];
        reinterpret_cast<BPC_REG_D1A_BPC_BPC_OBC_OFST_GR_FW*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BPC_OBC_OFST_GR_FW))->Bits.BPC_OBC_OFST_GR = mObc[SubModuleIndex][EBPC_OBC_OFST_GR];
        reinterpret_cast<BPC_REG_D1A_BPC_BPC_OBC_OFST_GB_FW*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BPC_OBC_OFST_GB_FW))->Bits.BPC_OBC_OFST_GB = mObc[SubModuleIndex][EBPC_OBC_OFST_GB];

        //Chooo should be after ZFUS
        MBOOL bZHDR_Enable = MFALSE; //Chooo, need ZHDR Info
        reinterpret_cast<REG_BPC_R1_BPC_BPC_ZHDR_CON_FW*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, BPC_ZHDR_CON_FW))->Bits.BPC_ZHDR_EN &= bZHDR_Enable;


        if(bPDC_Enable){ //PDC
            reinterpret_cast<BPC_REG_D1A_BPC_PDC_CUR_AG_FW*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, PDC_CUR_AG_FW))->Bits.BPC_PDC_GTH = 4095;
            reinterpret_cast<BPC_REG_D1A_BPC_PDC_CUR_AG_FW*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, PDC_CUR_AG_FW))->Bits.BPC_PDC_ATH = 4094;
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
};
