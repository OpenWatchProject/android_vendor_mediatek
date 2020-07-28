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
    MBOOL m_bPDCoutEnable;
    MBOOL m_bCCTBPCEnable;
    MBOOL m_bCCTCTEnable;
    MBOOL m_bCCTPDCEnable;
    MUINT32 m_u4StartAddr;

    enum
    {
        ERegInfo_CAM_BNR_BPC_CON,
        ERegInfo_CAM_BNR_BPC_TH1,
        ERegInfo_CAM_BNR_BPC_TH2,
        ERegInfo_CAM_BNR_BPC_TH3,
        ERegInfo_CAM_BNR_BPC_TH4,
        ERegInfo_CAM_BNR_BPC_DTC,
        ERegInfo_CAM_BNR_BPC_COR,
        ERegInfo_CAM_BNR_BPC_TBLI1,
        ERegInfo_CAM_BNR_BPC_TBLI2,
        ERegInfo_CAM_BNR_BPC_TH1_C,
        ERegInfo_CAM_BNR_BPC_TH2_C,
        ERegInfo_CAM_BNR_BPC_TH3_C,
        ERegInfo_CAM_BNR_NR1_CON,
        ERegInfo_CAM_BNR_NR1_CT_CON,
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
        ERegInfo_NUM
    };

    RegInfo_T m_rIspRegInfo[ERegInfo_NUM];

protected:
    ISP_MGR_BNR(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr, eSensorDev)
        , m_bBPCEnable(MTRUE)
        , m_bCTEnable(MTRUE)
        , m_bPDCEnable(MFALSE)
        , m_bPDCoutEnable(MFALSE)
        , m_bCCTBPCEnable(MTRUE)
        , m_bCCTCTEnable(MTRUE)
        , m_bCCTPDCEnable(MTRUE)
        , m_u4StartAddr(REG_ADDR_P1(CAM_BNR_BPC_CON))
    {
        // register info addr init
        INIT_REG_INFO_ADDR_P1(CAM_BNR_BPC_CON);
        INIT_REG_INFO_ADDR_P1(CAM_BNR_BPC_TH1);
        INIT_REG_INFO_ADDR_P1(CAM_BNR_BPC_TH2);
        INIT_REG_INFO_ADDR_P1(CAM_BNR_BPC_TH3);
        INIT_REG_INFO_ADDR_P1(CAM_BNR_BPC_TH4);
        INIT_REG_INFO_ADDR_P1(CAM_BNR_BPC_DTC);
        INIT_REG_INFO_ADDR_P1(CAM_BNR_BPC_COR);
        INIT_REG_INFO_ADDR_P1(CAM_BNR_BPC_TBLI1);
        INIT_REG_INFO_ADDR_P1(CAM_BNR_BPC_TBLI2);
        INIT_REG_INFO_ADDR_P1(CAM_BNR_BPC_TH1_C);
        INIT_REG_INFO_ADDR_P1(CAM_BNR_BPC_TH2_C);
        INIT_REG_INFO_ADDR_P1(CAM_BNR_BPC_TH3_C);
        INIT_REG_INFO_ADDR_P1(CAM_BNR_NR1_CON);
        INIT_REG_INFO_ADDR_P1(CAM_BNR_NR1_CT_CON);
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
    isPDCoutEnable()
    {
        return m_bPDCoutEnable;
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
    setPDCoutEnable(MBOOL bEnable)
    {
        m_bPDCoutEnable = bEnable;
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

    MBOOL apply(EIspProfile_T eIspProfile, TuningMgr& rTuning, MINT32 i4SubsampleIdex=0);
} ISP_MGR_BNR_T;

template <ESensorDev_T const eSensorDev>
class ISP_MGR_BNR_DEV : public ISP_MGR_BNR_T
{
public:
    static
    ISP_MGR_BNR_T&
    getInstance()
    {
        static ISP_MGR_BNR_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_BNR_DEV()
        : ISP_MGR_BNR_T(eSensorDev)
    {}

    virtual ~ISP_MGR_BNR_DEV() {}

};

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
    MBOOL  m_bPDSensor;
    MVOID* m_pPDCBuffer;
    CAMERA_BPCI_STRUCT* m_pBpciBuf;

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
        ERegInfo_DIP_X_BNR2_PDC_POS,
        ERegInfo_NUM
    };

    RegInfo_T m_rIspRegInfo[ERegInfo_NUM];

protected:
    ISP_MGR_BNR2(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr, eSensorDev)
        , m_bBPC2Enable(MTRUE)
        , m_bCT2Enable(MTRUE)
        , m_bPDC2Enable(MTRUE)
        , m_bCCTBPC2Enable(MTRUE)
        , m_bCCTCT2Enable(MTRUE)
        , m_bCCTPDC2Enable(MTRUE)
        , m_u4StartAddr(REG_ADDR_P2(DIP_X_BNR2_BPC_CON))
        , m_bPDSensor(MFALSE)
        , m_pPDCBuffer(NULL)
        , m_pBpciBuf(NULL)
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
        INIT_REG_INFO_ADDR_P2(DIP_X_BNR2_PDC_POS);
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
    getBPCItable(){
        if (reinterpret_cast<ISP_CAM_BNR_PDC_CON_T*>(REG_INFO_VALUE_PTR(DIP_X_BNR2_PDC_CON))->PDC_EN)
            return m_pPDCBuffer;
        else
            return NULL;
    }
    MBOOL getBPCItable(
        UINT32& bpci_xsize,
        UINT32& bpci_ysize,
        UINT32& pdo_xsize,
        UINT32& pdo_ysize,
        MUINTPTR& phy_addr,
        MUINTPTR& vir_addr);

    MVOID
    setPDSensorInfo(MBOOL bPDSensor)
    {
        m_bPDSensor = bPDSensor;
    }

    MBOOL setBPCIBuf(CAMERA_BPCI_STRUCT* buf);

    MBOOL apply(EIspProfile_T eIspProfile, dip_x_reg_t* pReg);
} ISP_MGR_BNR2_T;

template <ESensorDev_T const eSensorDev>
class ISP_MGR_BNR2_DEV : public ISP_MGR_BNR2_T
{
public:
    static
    ISP_MGR_BNR2_T&
    getInstance()
    {
        static ISP_MGR_BNR2_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_BNR2_DEV()
        : ISP_MGR_BNR2_T(eSensorDev)
    {}

    virtual ~ISP_MGR_BNR2_DEV() {}

};


#endif

