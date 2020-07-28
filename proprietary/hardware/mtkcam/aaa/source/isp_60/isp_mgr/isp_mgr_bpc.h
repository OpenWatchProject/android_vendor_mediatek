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
#ifndef _ISP_MGR_BPC_H_
#define _ISP_MGR_BPC_H_

#include <aaa/pd_buf_common.h>

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  BPC
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define INIT_BPC_ADDR(reg)\
    INIT_REG_INFO_ADDR_P1_MULTI(EBPC_R1 ,reg, BPC_R1_BPC_);\
    INIT_REG_INFO_ADDR_P2_MULTI(EBPC_D1 ,reg, BPC_D1A_BPC_)
//INIT_REG_INFO_ADDR_P1_MULTI(EBPC_R2 ,reg, OBC_R2_OBC_)

typedef class ISP_MGR_BPC : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_BPC    MyType;
public:
    enum
    {
        EBPC_R1, //Pass1 Normal
        //EBPC_R2, //Pass1 for HDR Fus
        EBPC_D1, //Pass2
        ESubModule_NUM
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

    enum
    {
        //BPC
        ERegInfo_BPC_CON,
        ERegInfo_BPC_BLD,
        ERegInfo_BPC_TH1,
        ERegInfo_BPC_TH2,
        ERegInfo_BPC_TH3,
        ERegInfo_BPC_TH4,
        ERegInfo_BPC_TH5,
        ERegInfo_BPC_TH6,
        ERegInfo_BPC_DTC,
        ERegInfo_BPC_COR,
        ERegInfo_BPC_RANK,
        //ERegInfo_BPC_TBLI1,
        //ERegInfo_BPC_TBLI2,
        ERegInfo_BPC_TH1_C,
        ERegInfo_BPC_TH2_C,
        ERegInfo_BPC_TH3_C,
        ERegInfo_BPC_LL,
        ERegInfo_ZHDR_CON,
        ERegInfo_ZHDR_RMG,
        ERegInfo_PSEUDO,
        //CT
        ERegInfo_CT_CON1,
        ERegInfo_CT_CON2,
        ERegInfo_CT_BLD1,
        ERegInfo_CT_BLD2,
        //ERegInfo_RCCC_CT_CON,
        //PDC
        ERegInfo_PDC_CON,
        ERegInfo_PDC_GAIN_L0,
        //ERegInfo_PDC_GAIN_L1,
        //ERegInfo_PDC_GAIN_L2,
        //ERegInfo_PDC_GAIN_L3,
        //ERegInfo_PDC_GAIN_L4,
        ERegInfo_PDC_GAIN_R0,
        //ERegInfo_PDC_GAIN_R1,
        //ERegInfo_PDC_GAIN_R2,
        //ERegInfo_PDC_GAIN_R3,
        //ERegInfo_PDC_GAIN_R4,
        ERegInfo_PDC_TH_GB,
        ERegInfo_PDC_TH_IA,
        ERegInfo_PDC_TH_HD,
        ERegInfo_PDC_SL,
        //ERegInfo_PDC_POS,
        ERegInfo_PDC_DIFF_TH_GR1,
        ERegInfo_PDC_DIFF_TH_GR2,
        ERegInfo_PDC_DIFF_TH_GR3,
        ERegInfo_PDC_DIFF_TH_GB1,
        ERegInfo_PDC_DIFF_TH_GB2,
        ERegInfo_PDC_DIFF_TH_GB3,
        ERegInfo_PDC_DIFF_TH_R1,
        ERegInfo_PDC_DIFF_TH_R2,
        ERegInfo_PDC_DIFF_TH_R3,
        ERegInfo_PDC_DIFF_TH_B1,
        ERegInfo_PDC_DIFF_TH_B2,
        ERegInfo_PDC_DIFF_TH_B3,
        ERegInfo_PDC_GAIN1,
        ERegInfo_PDC_GAIN2,
        ERegInfo_PD_MODE,
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

public:
    ISP_MGR_BPC(MUINT32 const eSensorDev)
        :ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, eSensorDev, ESubModule_NUM)
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

        // register info addr init
        //BPC
        INIT_BPC_ADDR(BPC_CON);
        INIT_BPC_ADDR(BPC_BLD);
        INIT_BPC_ADDR(BPC_TH1);
        INIT_BPC_ADDR(BPC_TH2);
        INIT_BPC_ADDR(BPC_TH3);
        INIT_BPC_ADDR(BPC_TH4);
        INIT_BPC_ADDR(BPC_TH5);
        INIT_BPC_ADDR(BPC_TH6);
        INIT_BPC_ADDR(BPC_DTC);
        INIT_BPC_ADDR(BPC_COR);
        INIT_BPC_ADDR(BPC_RANK);
        //INIT_BPC_ADDR(BPC_TBLI1);
        //INIT_BPC_ADDR(BPC_TBLI2);
        INIT_BPC_ADDR(BPC_TH1_C);
        INIT_BPC_ADDR(BPC_TH2_C);
        INIT_BPC_ADDR(BPC_TH3_C);
        INIT_BPC_ADDR(BPC_LL);
        INIT_BPC_ADDR(ZHDR_CON);
        INIT_BPC_ADDR(ZHDR_RMG);
        INIT_BPC_ADDR(PSEUDO);
        //CT
        INIT_BPC_ADDR(CT_CON1);
        INIT_BPC_ADDR(CT_CON2);
        INIT_BPC_ADDR(CT_BLD1);
        INIT_BPC_ADDR(CT_BLD2);
        //INIT_BPC_ADDR(RCCC_CT_CON);
        //PDC
        INIT_BPC_ADDR(PDC_CON);
        INIT_BPC_ADDR(PDC_GAIN_L0);
        //INIT_BPC_ADDR(PDC_GAIN_L1);
        //INIT_BPC_ADDR(PDC_GAIN_L2);
        //INIT_BPC_ADDR(PDC_GAIN_L3);
        //INIT_BPC_ADDR(PDC_GAIN_L4);
        INIT_BPC_ADDR(PDC_GAIN_R0);
        //INIT_BPC_ADDR(PDC_GAIN_R1);
        //INIT_BPC_ADDR(PDC_GAIN_R2);
        //INIT_BPC_ADDR(PDC_GAIN_R3);
        //INIT_BPC_ADDR(PDC_GAIN_R4);
        INIT_BPC_ADDR(PDC_TH_GB);
        INIT_BPC_ADDR(PDC_TH_IA);
        INIT_BPC_ADDR(PDC_TH_HD);
        INIT_BPC_ADDR(PDC_SL);
        //INIT_BPC_ADDR(PDC_POS);
        INIT_BPC_ADDR(PDC_DIFF_TH_GR1);
        INIT_BPC_ADDR(PDC_DIFF_TH_GR2);
        INIT_BPC_ADDR(PDC_DIFF_TH_GR3);
        INIT_BPC_ADDR(PDC_DIFF_TH_GB1);
        INIT_BPC_ADDR(PDC_DIFF_TH_GB2);
        INIT_BPC_ADDR(PDC_DIFF_TH_GB3);
        INIT_BPC_ADDR(PDC_DIFF_TH_R1);
        INIT_BPC_ADDR(PDC_DIFF_TH_R2);
        INIT_BPC_ADDR(PDC_DIFF_TH_R3);
        INIT_BPC_ADDR(PDC_DIFF_TH_B1);
        INIT_BPC_ADDR(PDC_DIFF_TH_B2);
        INIT_BPC_ADDR(PDC_DIFF_TH_B3);
        INIT_BPC_ADDR(PDC_GAIN1);
        INIT_BPC_ADDR(PDC_GAIN2);
        INIT_BPC_ADDR(PD_MODE);

        m_rIspRegInfo_BPCI_Pass1[EBPC_R1][ERegInfo_BASE_ADDR].addr = REG_ADDR_P1(BPCI_R1_BPCI_BASE_ADDR);
        m_rIspRegInfo_BPCI_Pass1[EBPC_R1][ERegInfo_XSIZE].addr     = REG_ADDR_P1(BPCI_R1_BPCI_XSIZE);
        m_rIspRegInfo_BPCI_Pass1[EBPC_R1][ERegInfo_YSIZE].addr     = REG_ADDR_P1(BPCI_R1_BPCI_YSIZE);
        m_rIspRegInfo_BPCI_Pass1[EBPC_R1][ERegInfo_STRIDE].addr    = REG_ADDR_P1(BPCI_R1_BPCI_STRIDE);

#if 0
        m_rIspRegInfo_BPCI_Pass1[EBPC_R2][ERegInfo_BPCI_R2_BPCI_BASE_ADDR].addr = REG_ADDR_P1(BPCI_R2_BPCI_BASE_ADDR);
        m_rIspRegInfo_BPCI_Pass1[EBPC_R2][ERegInfo_BPCI_R2_BPCI_XSIZE].addr     = REG_ADDR_P1(BPCI_R2_BPCI_XSIZE);
        m_rIspRegInfo_BPCI_Pass1[EBPC_R2][ERegInfo_BPCI_R2_BPCI_YSIZE].addr     = REG_ADDR_P1(BPCI_R2_BPCI_YSIZE);
        m_rIspRegInfo_BPCI_Pass1[EBPC_R2][ERegInfo_BPCI_R2_BPCI_STRIDE].addr    = REG_ADDR_P1(BPCI_R2_BPCI_STRIDE);
#endif
        //memset(&m_sPDOHWInfo, 0, sizeof(SPDOHWINFO_T));
    }

    virtual ~ISP_MGR_BPC() {}

public:
    static MyType&  getInstance(MUINT32 const eSensorDev);

public: // Interfaces.

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

    MBOOL apply_P1(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, TuningMgr& rTuning, MINT32 i4SubsampleIdex=0);

    MBOOL apply_P2(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, dip_x_reg_t* pReg);

} ISP_MGR_BPC_T;



#endif

