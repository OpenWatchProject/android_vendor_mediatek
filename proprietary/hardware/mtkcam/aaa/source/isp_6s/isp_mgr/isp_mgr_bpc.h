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
#ifndef _ISP_MGR_BPC_H_
#define _ISP_MGR_BPC_H_

#include <aaa/pd_buf_common.h>
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BPC
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define INIT_BPC_ADDR(reg)\
    INIT_REG_INFO_ADDR_P1_MULTI(EBPC_R1, reg, BPC_R1_BPC_);\
    INIT_REG_INFO_ADDR_P1_MULTI(EBPC_R2, reg, BPC_R2_BPC_);\
    INIT_REG_INFO_ADDR_P1_MULTI(EBPC_R3, reg, BPC_R3_BPC_);\
    INIT_REG_INFO_ADDR_P2_MULTI(EBPC_D1, reg, BPC_D1A_BPC_);\

typedef class ISP_MGR_BPC : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_BPC    MyType;
public:
    enum
    {
        EBPC_R1,
        EBPC_R2,
        EBPC_R3,
        EBPC_R4,
        EBPC_D1,
        ESubModule_NUM
    };
    enum
    {
        EBPC_OBC_OFST_R,
        EBPC_OBC_OFST_B,
        EBPC_OBC_OFST_GR,
        EBPC_OBC_OFST_GB,
        EBPC_OBC_OFST_NUM
    };
private:
    MBOOL m_bBPCEnable[ESubModule_NUM];
    MBOOL m_bCTEnable[ESubModule_NUM];
    MBOOL m_bPDCEnable[ESubModule_NUM];
    MBOOL m_bCCTBPCEnable; //only support R1, D1
    MBOOL m_bCCTCTEnable; //only support R1, D1
    MBOOL m_bCCTPDCEnable; //only support R1, D1
    MBOOL m_bBPCIValid[ESubModule_NUM];
    SPDOHWINFO_T m_sPDOHWInfo;  //for P1
    MVOID* m_pBPCI_ImgBuf;  //for P2
    MINT32 mObc[ESubModule_NUM][EBPC_OBC_OFST_NUM];

    enum
    {
        ERegInfo_BPC_FUNC_CON,
        ERegInfo_BPC_IN_TH_1,
        ERegInfo_BPC_IN_TH_2,
        ERegInfo_BPC_IN_TH_3,
        ERegInfo_BPC_IN_TH_4,
        ERegInfo_BPC_IN_TH_5,
        ERegInfo_BPC_C_TH_1,
        ERegInfo_BPC_C_TH_2,
        ERegInfo_BPC_C_TH_3,
        ERegInfo_BPC_DTC,
        ERegInfo_BPC_COR,
        ERegInfo_BPC_RANK,
        ERegInfo_BPC_BLD_1,
        ERegInfo_BPC_BLD_2,
        ERegInfo_BPC_LL,
        ERegInfo_CT_CON,
        ERegInfo_CT_BLD_1,
        ERegInfo_CT_BLD_2,
        ERegInfo_PDC_CON,
        ERegInfo_PDC_CUR_BI,
        ERegInfo_PDC_CUR_AG,
        ERegInfo_PDC_TH_N,
        ERegInfo_PDC_DIFF_TH_GR_1,
        ERegInfo_PDC_DIFF_TH_GR_2,
        ERegInfo_PDC_DIFF_TH_GR_3,
        ERegInfo_PDC_DIFF_TH_GB_1,
        ERegInfo_PDC_DIFF_TH_GB_2,
        ERegInfo_PDC_DIFF_TH_GB_3,
        ERegInfo_PDC_DIFF_TH_R_1,
        ERegInfo_PDC_DIFF_TH_R_2,
        ERegInfo_PDC_DIFF_TH_R_3,
        ERegInfo_PDC_DIFF_TH_B_1,
        ERegInfo_PDC_DIFF_TH_B_2,
        ERegInfo_PDC_DIFF_TH_B_3,
        ERegInfo_PDC_BLD_WT,
        ERegInfo_PDC_GAIN_G,
        ERegInfo_PDC_GAIN_RB,
        ERegInfo_BPC_ZHDR_TUN,
        ERegInfo_BPC_OBC_ZOFST,
        ERegInfo_BPC_PD_EXC_CON,
        ERegInfo_BPC_AUX_MAP_FW,
        ERegInfo_CT_RCCC_CT_FW,
        ERegInfo_PDC_CUR_AG_FW,
        ERegInfo_BPC_ZHDR_CON_FW,
        ERegInfo_BPC_ZHDR_RMG_FW,
        ERegInfo_BPC_OBC_OFST_R_FW,
        ERegInfo_BPC_OBC_OFST_B_FW,
        ERegInfo_BPC_OBC_OFST_GR_FW,
        ERegInfo_BPC_OBC_OFST_GB_FW,
        ERegInfo_BPC_CON_TILE,
        ERegInfo_NUM
    };
    enum
    {
        //BPCI
        ERegInfo_BASE_ADDR,
        ERegInfo_XSIZE,
        ERegInfo_YSIZE,
        ERegInfo_STRIDE,
        ERegInfo_BPCI_NUM
    };
    RegInfo_T m_rIspRegInfo[ESubModule_NUM][ERegInfo_NUM];
    RegInfo_T m_rIspRegInfo_BPCI_Pass1[(EBPC_R1 + 1)][ERegInfo_BPCI_NUM]; //only BPCI_R1

protected:
    ISP_MGR_BPC(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, eSensorDev, ESubModule_NUM)
        ,m_bCCTBPCEnable(MTRUE)
        ,m_bCCTCTEnable(MTRUE)
        ,m_bCCTPDCEnable(MTRUE)
        ,m_pBPCI_ImgBuf(NULL)
    {
        for(int i=0; i<ESubModule_NUM; i++){
            m_bBPCEnable[i] = MFALSE;
            m_bCTEnable[i]  = MFALSE;
            m_bPDCEnable[i] = MFALSE;
            ::memset(m_rIspRegInfo[i], 0, sizeof(RegInfo_T)*ERegInfo_NUM);
            m_bBPCIValid[i] = MFALSE;
        }
        for(int i=0; i<(EBPC_R1 + 1); i++){
            ::memset(m_rIspRegInfo_BPCI_Pass1[i], 0, sizeof(RegInfo_T)*ERegInfo_BPCI_NUM);
        }
        ::memset(&m_sPDOHWInfo, 0, sizeof(SPDOHWINFO_T));

        INIT_BPC_ADDR(BPC_FUNC_CON);
        INIT_BPC_ADDR(BPC_IN_TH_1);
        INIT_BPC_ADDR(BPC_IN_TH_2);
        INIT_BPC_ADDR(BPC_IN_TH_3);
        INIT_BPC_ADDR(BPC_IN_TH_4);
        INIT_BPC_ADDR(BPC_IN_TH_5);
        INIT_BPC_ADDR(BPC_C_TH_1);
        INIT_BPC_ADDR(BPC_C_TH_2);
        INIT_BPC_ADDR(BPC_C_TH_3);
        INIT_BPC_ADDR(BPC_DTC);
        INIT_BPC_ADDR(BPC_COR);
        INIT_BPC_ADDR(BPC_RANK);
        INIT_BPC_ADDR(BPC_BLD_1);
        INIT_BPC_ADDR(BPC_BLD_2);
        INIT_BPC_ADDR(BPC_LL);
        INIT_BPC_ADDR(CT_CON);
        INIT_BPC_ADDR(CT_BLD_1);
        INIT_BPC_ADDR(CT_BLD_2);
        INIT_BPC_ADDR(PDC_CON);
        INIT_BPC_ADDR(PDC_CUR_BI);
        INIT_BPC_ADDR(PDC_CUR_AG);
        INIT_BPC_ADDR(PDC_TH_N);
        INIT_BPC_ADDR(PDC_DIFF_TH_GR_1);
        INIT_BPC_ADDR(PDC_DIFF_TH_GR_2);
        INIT_BPC_ADDR(PDC_DIFF_TH_GR_3);
        INIT_BPC_ADDR(PDC_DIFF_TH_GB_1);
        INIT_BPC_ADDR(PDC_DIFF_TH_GB_2);
        INIT_BPC_ADDR(PDC_DIFF_TH_GB_3);
        INIT_BPC_ADDR(PDC_DIFF_TH_R_1);
        INIT_BPC_ADDR(PDC_DIFF_TH_R_2);
        INIT_BPC_ADDR(PDC_DIFF_TH_R_3);
        INIT_BPC_ADDR(PDC_DIFF_TH_B_1);
        INIT_BPC_ADDR(PDC_DIFF_TH_B_2);
        INIT_BPC_ADDR(PDC_DIFF_TH_B_3);
        INIT_BPC_ADDR(PDC_BLD_WT);
        INIT_BPC_ADDR(PDC_GAIN_G);
        INIT_BPC_ADDR(PDC_GAIN_RB);
        INIT_BPC_ADDR(BPC_ZHDR_TUN);
        INIT_BPC_ADDR(BPC_OBC_ZOFST);
        INIT_BPC_ADDR(BPC_PD_EXC_CON);
/*
        INIT_BPC_ADDR(BPC_AUX_MAP_FW);
        INIT_BPC_ADDR(CT_RCCC_CT_FW);
        INIT_BPC_ADDR(PDC_CUR_AG_FW);
        INIT_BPC_ADDR(BPC_ZHDR_CON_FW);
        INIT_BPC_ADDR(BPC_ZHDR_RMG_FW);
        INIT_BPC_ADDR(BPC_OBC_OFST_R_FW);
        INIT_BPC_ADDR(BPC_OBC_OFST_B_FW);
        INIT_BPC_ADDR(BPC_OBC_OFST_GR_FW);
        INIT_BPC_ADDR(BPC_OBC_OFST_GB_FW);
		INIT_BPC_ADDR(BPC_CON_TILE);
*/
        m_rIspRegInfo_BPCI_Pass1[EBPC_R1][ERegInfo_BASE_ADDR].addr = REG_ADDR_P1(BPCI_R1_BPCI_BASE_ADDR);
        m_rIspRegInfo_BPCI_Pass1[EBPC_R1][ERegInfo_XSIZE].addr     = REG_ADDR_P1(BPCI_R1_BPCI_XSIZE);
        m_rIspRegInfo_BPCI_Pass1[EBPC_R1][ERegInfo_YSIZE].addr     = REG_ADDR_P1(BPCI_R1_BPCI_YSIZE);
        m_rIspRegInfo_BPCI_Pass1[EBPC_R1][ERegInfo_STRIDE].addr    = REG_ADDR_P1(BPCI_R1_BPCI_STRIDE);

    }

    virtual ~ISP_MGR_BPC() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public:

    template <class ISP_xxx_T>
    MyType& put(MUINT8 SubModuleIndex, ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(MUINT8 SubModuleIndex, ISP_xxx_T & rParam);

    MBOOL
    isBPCEnable(MUINT8 SubModuleIndex)
    {
        return m_bBPCEnable[SubModuleIndex];
    }

    MBOOL
    isCTEnable(MUINT8 SubModuleIndex)
    {
        return m_bCTEnable[SubModuleIndex];
    }

    MBOOL
    isPDCEnable(MUINT8 SubModuleIndex)
    {
        return m_bPDCEnable[SubModuleIndex];
    }

    MBOOL
    isCCTBPCEnable()
    {
        return m_bCCTBPCEnable;
    }

    MBOOL
    isCCTCTEnable()
    {
        return m_bCCTCTEnable;
    }

    MBOOL
    isCCTPDCEnable()
    {
        return m_bCCTPDCEnable;
    }

    MVOID
    setBPCEnable(MUINT8 SubModuleIndex, MBOOL bEnable)
    {
        m_bBPCEnable[SubModuleIndex] = bEnable;
    }

    MVOID
    setCTEnable(MUINT8 SubModuleIndex, MBOOL bEnable)
    {
        m_bCTEnable[SubModuleIndex] = bEnable;
    }

    MVOID
    setPDCEnable(MUINT8 SubModuleIndex, MBOOL bEnable)
    {
        m_bPDCEnable[SubModuleIndex] = bEnable;
    }

    MVOID
    setCCTBPCEnable(MBOOL bEnable)
    {
        m_bCCTBPCEnable = bEnable;
    }

    MVOID
    setCCTCTEnable(MBOOL bEnable)
    {
        m_bCCTCTEnable = bEnable;
    }

    MVOID
    setCCTPDCEnable(MBOOL bEnable)
    {
        m_bCCTPDCEnable = bEnable;
    }

    MVOID*
    getDMGItable(){
        return m_pBPCI_ImgBuf;
    }

    MVOID
    setDMGItable(MVOID* pBPCI_ImgBuf){
        m_pBPCI_ImgBuf = pBPCI_ImgBuf;
    }

    MVOID
    setBPCIValid(MUINT8 SubModuleIndex, MBOOL bEnable)
    {
        m_bBPCIValid[SubModuleIndex] = bEnable;
    }
	MVOID setOBCOffset(MUINT8 SubModuleIndex, ISP_NVRAM_OBC_T const& rParam);

    MBOOL apply_P1(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, TuningMgr& rTuning, MINT32 i4SubsampleIdex=0);

    MBOOL apply_P2(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, dip_a_reg_t* pReg);

} ISP_MGR_BPC_T;

template <ESensorDev_T const eSensorDev>
class ISP_MGR_BPC_DEV : public ISP_MGR_BPC_T
{
public:
    static
    ISP_MGR_BPC_T&
    getInstance()
    {
        static ISP_MGR_BPC_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_BPC_DEV()
        : ISP_MGR_BPC_T(eSensorDev)
    {}

    virtual ~ISP_MGR_BPC_DEV() {}

};
#endif