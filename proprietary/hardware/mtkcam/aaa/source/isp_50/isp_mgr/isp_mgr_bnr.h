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
#ifndef _ISP_MGR_BNR_H_
#define _ISP_MGR_BNR_H_

#define Default_BPCI_NUM (1)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  BNR2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_BNR2 : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_BNR2    MyType;
private:
    MBOOL m_bBPC2Enable;
    MBOOL m_bCT2Enable;
    MBOOL m_bPDC2Enable;
    MBOOL m_bCCTBPC2Enable;
    MBOOL m_bCCTCT2Enable;
    MBOOL m_bCCTPDC2Enable;
    MUINT32 m_u4StartAddr;
    MVOID* m_pBPCI_ImgBuf;
    MVOID* m_pBPCI_ImgBuf_Default;
    MUINT8 m_Default_BPCI[Default_BPCI_NUM + 1];
    ESensorMode_T m_ePrvSensorMode;
    MBOOL m_bPDC_TBL_Valid;


    enum
    {
        ERegInfo_DIP_X_BNR2_BPC_CON,
        ERegInfo_DIP_X_BNR2_BPC_TH1,
        ERegInfo_DIP_X_BNR2_BPC_TH2,
        ERegInfo_DIP_X_BNR2_BPC_TH3,
        ERegInfo_DIP_X_BNR2_BPC_TH4,
        ERegInfo_DIP_X_BNR2_BPC_DTC,
        ERegInfo_DIP_X_BNR2_BPC_COR,
        ERegInfo_DIP_X_BNR2_BPC_TBLI1,
        ERegInfo_DIP_X_BNR2_BPC_TBLI2,
        ERegInfo_DIP_X_BNR2_BPC_TH1_C,
        ERegInfo_DIP_X_BNR2_BPC_TH2_C,
        ERegInfo_DIP_X_BNR2_BPC_TH3_C,
        ERegInfo_DIP_X_BNR2_NR1_CON,
        ERegInfo_DIP_X_BNR2_NR1_CT_CON,
        ERegInfo_DIP_X_BNR2_NR1_CT_CON2,
        ERegInfo_DIP_X_BNR2_NR1_CT_CON3,
        ERegInfo_DIP_X_BNR2_PDC_CON,
        ERegInfo_DIP_X_BNR2_PDC_GAIN_L0,
        ERegInfo_DIP_X_BNR2_PDC_GAIN_L1,
        ERegInfo_DIP_X_BNR2_PDC_GAIN_L2,
        ERegInfo_DIP_X_BNR2_PDC_GAIN_L3,
        ERegInfo_DIP_X_BNR2_PDC_GAIN_L4,
        ERegInfo_DIP_X_BNR2_PDC_GAIN_R0,
        ERegInfo_DIP_X_BNR2_PDC_GAIN_R1,
        ERegInfo_DIP_X_BNR2_PDC_GAIN_R2,
        ERegInfo_DIP_X_BNR2_PDC_GAIN_R3,
        ERegInfo_DIP_X_BNR2_PDC_GAIN_R4,
        ERegInfo_DIP_X_BNR2_PDC_TH_GB,
        ERegInfo_DIP_X_BNR2_PDC_TH_IA,
        ERegInfo_DIP_X_BNR2_PDC_TH_HD,
        ERegInfo_DIP_X_BNR2_PDC_SL,
        //ERegInfo_DIP_X_BNR2_PDC_POS,
        ERegInfo_NUM
    };

    RegInfo_T m_rIspRegInfo[ERegInfo_NUM];

public:
    ISP_MGR_BNR2(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr, eSensorDev)
        , m_bBPC2Enable(MTRUE)
        , m_bCT2Enable(MTRUE)
        , m_bPDC2Enable(MTRUE)
        , m_bCCTBPC2Enable(MTRUE)
        , m_bCCTCT2Enable(MTRUE)
        , m_bCCTPDC2Enable(MTRUE)
        , m_u4StartAddr(REG_ADDR_P2(DIP_X_BNR2_BPC_CON))
        , m_pBPCI_ImgBuf(NULL)
        , m_pBPCI_ImgBuf_Default(NULL)
        , m_ePrvSensorMode(ESensorMode_NUM)
        , m_bPDC_TBL_Valid(MFALSE)
    {
        // register info addr init
        INIT_REG_INFO_ADDR_P2(DIP_X_BNR2_BPC_CON);
        INIT_REG_INFO_ADDR_P2(DIP_X_BNR2_BPC_TH1);
        INIT_REG_INFO_ADDR_P2(DIP_X_BNR2_BPC_TH2);
        INIT_REG_INFO_ADDR_P2(DIP_X_BNR2_BPC_TH3);
        INIT_REG_INFO_ADDR_P2(DIP_X_BNR2_BPC_TH4);
        INIT_REG_INFO_ADDR_P2(DIP_X_BNR2_BPC_DTC);
        INIT_REG_INFO_ADDR_P2(DIP_X_BNR2_BPC_COR);
        INIT_REG_INFO_ADDR_P2(DIP_X_BNR2_BPC_TBLI1);
        INIT_REG_INFO_ADDR_P2(DIP_X_BNR2_BPC_TBLI2);
        INIT_REG_INFO_ADDR_P2(DIP_X_BNR2_BPC_TH1_C);
        INIT_REG_INFO_ADDR_P2(DIP_X_BNR2_BPC_TH2_C);
        INIT_REG_INFO_ADDR_P2(DIP_X_BNR2_BPC_TH3_C);
        INIT_REG_INFO_ADDR_P2(DIP_X_BNR2_NR1_CON);
        INIT_REG_INFO_ADDR_P2(DIP_X_BNR2_NR1_CT_CON);
        INIT_REG_INFO_ADDR_P2(DIP_X_BNR2_NR1_CT_CON2);
        INIT_REG_INFO_ADDR_P2(DIP_X_BNR2_NR1_CT_CON3);
        INIT_REG_INFO_ADDR_P2(DIP_X_BNR2_PDC_CON);
        INIT_REG_INFO_ADDR_P2(DIP_X_BNR2_PDC_GAIN_L0);
        INIT_REG_INFO_ADDR_P2(DIP_X_BNR2_PDC_GAIN_L1);
        INIT_REG_INFO_ADDR_P2(DIP_X_BNR2_PDC_GAIN_L2);
        INIT_REG_INFO_ADDR_P2(DIP_X_BNR2_PDC_GAIN_L3);
        INIT_REG_INFO_ADDR_P2(DIP_X_BNR2_PDC_GAIN_L4);
        INIT_REG_INFO_ADDR_P2(DIP_X_BNR2_PDC_GAIN_R0);
        INIT_REG_INFO_ADDR_P2(DIP_X_BNR2_PDC_GAIN_R1);
        INIT_REG_INFO_ADDR_P2(DIP_X_BNR2_PDC_GAIN_R2);
        INIT_REG_INFO_ADDR_P2(DIP_X_BNR2_PDC_GAIN_R3);
        INIT_REG_INFO_ADDR_P2(DIP_X_BNR2_PDC_GAIN_R4);
        INIT_REG_INFO_ADDR_P2(DIP_X_BNR2_PDC_TH_GB);
        INIT_REG_INFO_ADDR_P2(DIP_X_BNR2_PDC_TH_IA);
        INIT_REG_INFO_ADDR_P2(DIP_X_BNR2_PDC_TH_HD);
        INIT_REG_INFO_ADDR_P2(DIP_X_BNR2_PDC_SL);
        //INIT_REG_INFO_ADDR_P2(DIP_X_BNR2_PDC_POS);
        m_Default_BPCI[0] = 0xFF;
        m_Default_BPCI[1] = 0x3F;
    }

    virtual ~ISP_MGR_BNR2();

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: // Interfaces.

    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MBOOL get(ISP_xxx_T & rParam, const dip_x_reg_t* pReg) const;

    MBOOL
    isBPC2Enable()
    {
        return m_bBPC2Enable;
    }

    MBOOL
    isCT2Enable()
    {
        return m_bCT2Enable;
    }

    MBOOL
    isPDC2Enable()
    {
        return m_bPDC2Enable;
    }

    MBOOL
    isCCTBPC2Enable()
    {
        return m_bCCTBPC2Enable;
    }

    MBOOL
    isCCTCT2Enable()
    {
        return m_bCCTCT2Enable;
    }

    MBOOL
    isCCTPDC2Enable()
    {
        return m_bCCTPDC2Enable;
    }

    MVOID
    setBPC2Enable(MBOOL bEnable)
    {
        m_bBPC2Enable = bEnable;
    }

    MVOID
    setCT2Enable(MBOOL bEnable)
    {
        m_bCT2Enable = bEnable;
    }

    MVOID
    setPDC2Enable(MBOOL bEnable)
    {
        m_bPDC2Enable = bEnable;
    }

    MVOID
    setCCTBPC2Enable(MBOOL bEnable)
    {
        m_bCCTBPC2Enable = bEnable;
    }

    MVOID
    setCCTCT2Enable(MBOOL bEnable)
    {
        m_bCCTCT2Enable = bEnable;
    }

    MVOID
    setCCTPDC2Enable(MBOOL bEnable)
    {
        m_bCCTPDC2Enable = bEnable;
    }

    MVOID*
    getDMGItable(){
        return m_pBPCI_ImgBuf;
    }

    MVOID*
    getDMGItable_Default(){
        return m_pBPCI_ImgBuf_Default;
    }

    MBOOL get_BPCI_Info_Default(
        UINT32& bpci_xsize,
        UINT32& bpci_ysize,
        MUINTPTR& phy_addr,
        MUINTPTR& vir_addr,
        MINT32& memID)
    {
        if(m_pBPCI_ImgBuf_Default != NULL) {
            bpci_xsize = static_cast<IImageBuffer*>(m_pBPCI_ImgBuf_Default)->getImgSize().w - 1;
            bpci_ysize = static_cast<IImageBuffer*>(m_pBPCI_ImgBuf_Default)->getImgSize().h - 1;
            phy_addr = static_cast<IImageBuffer*>(m_pBPCI_ImgBuf_Default)->getBufPA(0);
            vir_addr = static_cast<IImageBuffer*>(m_pBPCI_ImgBuf_Default)->getBufVA(0);
            memID = static_cast<IImageBuffer*>(m_pBPCI_ImgBuf_Default)->getFD(0);
            return MTRUE;
        }
        else
            return MFALSE;
    }

    MBOOL createBPCI_Default_Buf();

    MVOID unlockBPCIBuf();

    MBOOL apply(const RAWIspCamInfo& rRawIspCamInfo, dip_x_reg_t* pReg);
} ISP_MGR_BNR2_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  BNR
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_BNR : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_BNR    MyType;
private:
    MBOOL m_bBPCEnable;
    MBOOL m_bCTEnable;
    MBOOL m_bPDCEnable;
    MBOOL m_bAF_PDCEnable;
    MBOOL m_bCCTBPCEnable;
    MBOOL m_bCCTCTEnable;
    MBOOL m_bCCTPDCEnable;
    SPDOHWINFO_T m_sPDOHWInfo;
    MUINT32 m_u4StartAddr;
    ESensorMode_T m_ePrvSensorMode;
    MBOOL m_bPDC_TBL_Valid;

    enum
    {
        ERegInfo_CAM_BNR_BPC_CON,
        ERegInfo_CAM_BNR_BPC_TH1,
        ERegInfo_CAM_BNR_BPC_TH2,
        ERegInfo_CAM_BNR_BPC_TH3,
        ERegInfo_CAM_BNR_BPC_TH4,
        ERegInfo_CAM_BNR_BPC_DTC,
        ERegInfo_CAM_BNR_BPC_COR,
        //ERegInfo_CAM_BNR_BPC_TBLI1,
        ERegInfo_CAM_BNR_BPC_TBLI2,
        ERegInfo_CAM_BNR_BPC_TH1_C,
        ERegInfo_CAM_BNR_BPC_TH2_C,
        ERegInfo_CAM_BNR_BPC_TH3_C,
        ERegInfo_CAM_BNR_NR1_CON,
        ERegInfo_CAM_BNR_NR1_CT_CON,
        ERegInfo_CAM_BNR_NR1_CT_CON2,
        ERegInfo_CAM_BNR_NR1_CT_CON3,
        ERegInfo_CAM_BNR_PDC_CON,
        ERegInfo_CAM_BNR_PDC_GAIN_L0,
        ERegInfo_CAM_BNR_PDC_GAIN_L1,
        ERegInfo_CAM_BNR_PDC_GAIN_L2,
        ERegInfo_CAM_BNR_PDC_GAIN_L3,
        ERegInfo_CAM_BNR_PDC_GAIN_L4,
        ERegInfo_CAM_BNR_PDC_GAIN_R0,
        ERegInfo_CAM_BNR_PDC_GAIN_R1,
        ERegInfo_CAM_BNR_PDC_GAIN_R2,
        ERegInfo_CAM_BNR_PDC_GAIN_R3,
        ERegInfo_CAM_BNR_PDC_GAIN_R4,
        ERegInfo_CAM_BNR_PDC_TH_GB,
        ERegInfo_CAM_BNR_PDC_TH_IA,
        ERegInfo_CAM_BNR_PDC_TH_HD,
        ERegInfo_CAM_BNR_PDC_SL,
        ERegInfo_CAM_BNR_PDC_POS,
        ERegInfo_CAM_BPCI_BASE_ADDR,
        ERegInfo_CAM_BPCI_XSIZE,
        ERegInfo_CAM_BPCI_YSIZE,
        ERegInfo_CAM_BPCI_STRIDE,
        ERegInfo_NUM
    };

    RegInfo_T m_rIspRegInfo[ERegInfo_NUM];

public:
    ISP_MGR_BNR(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr, eSensorDev)
        , m_bBPCEnable(MTRUE)
        , m_bCTEnable(MTRUE)
        , m_bPDCEnable(MFALSE)
        , m_bAF_PDCEnable(MFALSE)
        , m_bCCTBPCEnable(MTRUE)
        , m_bCCTCTEnable(MTRUE)
        , m_bCCTPDCEnable(MTRUE)
        , m_u4StartAddr(REG_ADDR_P1(CAM_BNR_BPC_CON))
        , m_ePrvSensorMode(ESensorMode_NUM)
        , m_bPDC_TBL_Valid(MFALSE)
    {
        // register info addr init
        INIT_REG_INFO_ADDR_P1(CAM_BNR_BPC_CON);
        INIT_REG_INFO_ADDR_P1(CAM_BNR_BPC_TH1);
        INIT_REG_INFO_ADDR_P1(CAM_BNR_BPC_TH2);
        INIT_REG_INFO_ADDR_P1(CAM_BNR_BPC_TH3);
        INIT_REG_INFO_ADDR_P1(CAM_BNR_BPC_TH4);
        INIT_REG_INFO_ADDR_P1(CAM_BNR_BPC_DTC);
        INIT_REG_INFO_ADDR_P1(CAM_BNR_BPC_COR);
        //INIT_REG_INFO_ADDR_P1(CAM_BNR_BPC_TBLI1);
        INIT_REG_INFO_ADDR_P1(CAM_BNR_BPC_TBLI2);
        INIT_REG_INFO_ADDR_P1(CAM_BNR_BPC_TH1_C);
        INIT_REG_INFO_ADDR_P1(CAM_BNR_BPC_TH2_C);
        INIT_REG_INFO_ADDR_P1(CAM_BNR_BPC_TH3_C);
        INIT_REG_INFO_ADDR_P1(CAM_BNR_NR1_CON);
        INIT_REG_INFO_ADDR_P1(CAM_BNR_NR1_CT_CON);
        INIT_REG_INFO_ADDR_P1(CAM_BNR_NR1_CT_CON2);
        INIT_REG_INFO_ADDR_P1(CAM_BNR_NR1_CT_CON3);
        INIT_REG_INFO_ADDR_P1(CAM_BNR_PDC_CON);
        INIT_REG_INFO_ADDR_P1(CAM_BNR_PDC_GAIN_L0);
        INIT_REG_INFO_ADDR_P1(CAM_BNR_PDC_GAIN_L1);
        INIT_REG_INFO_ADDR_P1(CAM_BNR_PDC_GAIN_L2);
        INIT_REG_INFO_ADDR_P1(CAM_BNR_PDC_GAIN_L3);
        INIT_REG_INFO_ADDR_P1(CAM_BNR_PDC_GAIN_L4);
        INIT_REG_INFO_ADDR_P1(CAM_BNR_PDC_GAIN_R0);
        INIT_REG_INFO_ADDR_P1(CAM_BNR_PDC_GAIN_R1);
        INIT_REG_INFO_ADDR_P1(CAM_BNR_PDC_GAIN_R2);
        INIT_REG_INFO_ADDR_P1(CAM_BNR_PDC_GAIN_R3);
        INIT_REG_INFO_ADDR_P1(CAM_BNR_PDC_GAIN_R4);
        INIT_REG_INFO_ADDR_P1(CAM_BNR_PDC_TH_GB);
        INIT_REG_INFO_ADDR_P1(CAM_BNR_PDC_TH_IA);
        INIT_REG_INFO_ADDR_P1(CAM_BNR_PDC_TH_HD);
        INIT_REG_INFO_ADDR_P1(CAM_BNR_PDC_SL);
        INIT_REG_INFO_ADDR_P1(CAM_BNR_PDC_POS);

        INIT_REG_INFO_ADDR_P1(CAM_BPCI_BASE_ADDR);
        INIT_REG_INFO_ADDR_P1(CAM_BPCI_XSIZE);
        INIT_REG_INFO_ADDR_P1(CAM_BPCI_YSIZE);
        INIT_REG_INFO_ADDR_P1(CAM_BPCI_STRIDE);

        memset(&m_sPDOHWInfo, 0, sizeof(SPDOHWINFO_T));

        INIT_REG_INFO_VALUE(CAM_BPCI_BASE_ADDR, m_sPDOHWInfo.phyAddrBpci_tbl);
        INIT_REG_INFO_VALUE(CAM_BPCI_XSIZE,     m_sPDOHWInfo.u4Bpci_xsz+1);
        INIT_REG_INFO_VALUE(CAM_BPCI_YSIZE,     1);
        INIT_REG_INFO_VALUE(CAM_BPCI_STRIDE,    m_sPDOHWInfo.u4Bpci_xsz+1);

    }

    virtual ~ISP_MGR_BNR() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: // Interfaces.

    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

    MBOOL
    isBPCEnable()
    {
        return m_bBPCEnable;
    }

    MBOOL
    isCTEnable()
    {
        return m_bCTEnable;
    }

    MBOOL
    isPDCEnable()
    {
        return m_bPDCEnable;
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
    setBPCEnable(MBOOL bEnable)
    {
        m_bBPCEnable = bEnable;
    }


    MVOID
    setCTEnable(MBOOL bEnable)
    {
        m_bCTEnable = bEnable;
    }

    MVOID
    setPDCEnable(MBOOL bEnable)
    {
        m_bPDCEnable = bEnable;
    }

    MVOID
    setAF_PDCEnable(MBOOL bEnable __unused)
    {
        //m_bAF_PDCEnable = bEnable;
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

    MBOOL apply(RAWIspCamInfo& rRawIspCamInfo, TuningMgr& rTuning, MINT32 i4SubsampleIdex=0);
} ISP_MGR_BNR_T;

#endif

