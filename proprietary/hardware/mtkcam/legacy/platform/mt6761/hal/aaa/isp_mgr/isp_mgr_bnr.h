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

//#include <kd_imgsensor_define.h>

typedef struct SPDOHWINFO_t
{
    MUINT32       u4Bpci_xsz;
    MUINT32       u4Bpci_ysz;
    const MUINT8 *pu1Bpci_tbl;
    MUINTPTR      phyAddrBpci_tbl;
    MUINTPTR      virAddrBpci_tbl;
    MINT32        i4memID;
    MUINT32       u4Pdo_xsz;
    MUINT32       u4Pdo_ysz;
    MUINT32       u4BitDepth;

    MUINT8        u1IsDualPD;

    //for dualpd
    MUINT8        u1PBinType; //0:1x4, 1:4x4
    MUINT8        u1PBinStartLine; //0, 1, 2, 3
    MUINT8        u1PdSeparateMode;

    SPDOHWINFO_t()
    {
        u4Bpci_xsz      = 0;
        u4Bpci_ysz      = 0;
        pu1Bpci_tbl     = NULL;
        phyAddrBpci_tbl = 0;
        virAddrBpci_tbl = 0;
        i4memID         = 0;
        u4Pdo_xsz       = 0;
        u4Pdo_ysz       = 0;
        u4BitDepth      = 0;
        u1IsDualPD      = 0;
        u1PBinType      = 0;
        u1PBinStartLine = 0;
        u1PdSeparateMode = 0;
    }
    SPDOHWINFO_t operator =(const SPDOHWINFO_t &in)
    {
        u4Bpci_xsz      = in.u4Bpci_xsz;
        u4Bpci_ysz      = in.u4Bpci_ysz;
        pu1Bpci_tbl     = in.pu1Bpci_tbl;
        phyAddrBpci_tbl = in.phyAddrBpci_tbl;
        virAddrBpci_tbl = in.virAddrBpci_tbl;
        i4memID         = in.i4memID;
        u4Pdo_xsz       = in.u4Pdo_xsz;
        u4Pdo_ysz       = in.u4Pdo_ysz;
        u1IsDualPD      = in.u1IsDualPD;
        u1PBinType      = in.u1PBinType;
        u1PBinStartLine = in.u1PBinStartLine;
        return (*this);
    }

} SPDOHWINFO_T;

typedef class ISP_MGR_BNR : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_BNR    MyType;
private:
    ESensorTG_T const m_eSensorTG;
    MBOOL m_bBPCEnable;
    MBOOL m_bCTEnable;
    MBOOL m_bPDCEnable;
    MBOOL m_bCCTBPCEnable;
    MBOOL m_bCCTCTEnable;
    MBOOL m_bCCTPDCEnable;
    MUINT32 m_u4StartAddr;
    MFLOAT m_u4AeExpRatio;
    SPDOHWINFO_T m_sPDOHWInfo;
    MBOOL m_bBPCIValid;
    MUINT32 m_u4PDSensorFmt;
    MVOID* m_pPDCBuffer_1;
    MVOID* m_pPDCBuffer_2;
    //CAMERA_PD_TBL_STRUCT* m_pBpciBuf_1;
    //CAMERA_PD_TBL_STRUCT* m_pBpciBuf_2;
    //EIndex_PDC_TBL_T m_PDTBL_Idx;

    // TG1
    enum
    {
        ERegInfo_CAM_BNR_BPC_CON,
        ERegInfo_CAM_BNR_BPC_TH1,
        ERegInfo_CAM_BNR_BPC_TH2,
        ERegInfo_CAM_BNR_BPC_TH3,
        ERegInfo_CAM_BNR_BPC_TH4,
        ERegInfo_CAM_BNR_BPC_DTC,
        ERegInfo_CAM_BNR_BPC_COR,
        //ERegInfo_CAM_BPC_TBLI1,
        //ERegInfo_CAM_BPC_TBLI2,
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

        ERegInfo_CAM_BPCI_BASE_ADDR,
        ERegInfo_CAM_BPCI_XSIZE,
        ERegInfo_CAM_BPCI_YSIZE,
        ERegInfo_CAM_BPCI_STRIDE,
        ERegInfo_NUM
    };

    // TG2
    enum
    {
        ERegInfo_CAM_BNR_D_BPC_CON,
        ERegInfo_CAM_BNR_D_BPC_TH1,
        ERegInfo_CAM_BNR_D_BPC_TH2,
        ERegInfo_CAM_BNR_D_BPC_TH3,
        ERegInfo_CAM_BNR_D_BPC_TH4,
        ERegInfo_CAM_BNR_D_BPC_DTC,
        ERegInfo_CAM_BNR_D_BPC_COR,
        //ERegInfo_CAM_BPC_D_TBLI1,
        //ERegInfo_CAM_BPC_D_TBLI2,
        ERegInfo_CAM_BNR_D_BPC_TH1_C,
        ERegInfo_CAM_BNR_D_BPC_TH2_C,
        ERegInfo_CAM_BNR_D_BPC_TH3_C,

        ERegInfo_CAM_BNR_D_NR1_CON,
        ERegInfo_CAM_BNR_D_NR1_CT_CON,

        ERegInfo_CAM_BNR_D_PDC_CON,
        ERegInfo_CAM_BNR_D_PDC_GAIN_L0,
        ERegInfo_CAM_BNR_D_PDC_GAIN_L1,
        ERegInfo_CAM_BNR_D_PDC_GAIN_L2,
        ERegInfo_CAM_BNR_D_PDC_GAIN_L3,
        ERegInfo_CAM_BNR_D_PDC_GAIN_L4,
        ERegInfo_CAM_BNR_D_PDC_GAIN_R0,
        ERegInfo_CAM_BNR_D_PDC_GAIN_R1,
        ERegInfo_CAM_BNR_D_PDC_GAIN_R2,
        ERegInfo_CAM_BNR_D_PDC_GAIN_R3,
        ERegInfo_CAM_BNR_D_PDC_GAIN_R4,
        ERegInfo_CAM_BNR_D_PDC_TH_GB,
        ERegInfo_CAM_BNR_D_PDC_TH_IA,
        ERegInfo_CAM_BNR_D_PDC_TH_HD,
        ERegInfo_CAM_BNR_D_PDC_SL,
        ERegInfo_CAM_BNR_D_PDC_POS,

        ERegInfo_CAM_BPCI_D_BASE_ADDR,
        ERegInfo_CAM_BPCI_D_XSIZE,
        ERegInfo_CAM_BPCI_D_YSIZE,
        ERegInfo_CAM_BPCI_D_STRIDE
    };

    RegInfo_T m_rIspRegInfo[ERegInfo_NUM];

protected:
    ISP_MGR_BNR(ESensorDev_T const eSensorDev, ESensorTG_T const eSensorTG)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr, eSensorDev)
        , m_eSensorTG(eSensorTG)
        , m_bBPCEnable(MTRUE)
        , m_bCTEnable(MTRUE)
        , m_bPDCEnable(MFALSE)
        , m_bCCTBPCEnable(MTRUE)
        , m_bCCTCTEnable(MTRUE)
        , m_bCCTPDCEnable(MTRUE)
        //, m_u4StartAddr((eSensorTG == ESensorTG_1) ? REG_ADDR(CAM_BNR_BPC_CON) : REG_ADDR(CAM_BNR_D_BPC_CON))
        , m_bBPCIValid(MFALSE)
        , m_u4PDSensorFmt(0)
        , m_pPDCBuffer_1(NULL)
        , m_pPDCBuffer_2(NULL)
//        , m_pBpciBuf_1(NULL)
//        , m_pBpciBuf_2(NULL)
//        , m_PDTBL_Idx(eIDX_PDC_FULL)
    {
        memset(&m_sPDOHWInfo, 0, sizeof(SPDOHWINFO_T));

        // register info addr init
        if (eSensorTG == ESensorTG_1) {
            INIT_REG_INFO_ADDR(CAM_BNR_BPC_CON);        // CAM+0x0800
            INIT_REG_INFO_ADDR(CAM_BNR_BPC_TH1);        // CAM+0x0804
            INIT_REG_INFO_ADDR(CAM_BNR_BPC_TH2);        // CAM+0x0808
            INIT_REG_INFO_ADDR(CAM_BNR_BPC_TH3);        // CAM+0x080C
            INIT_REG_INFO_ADDR(CAM_BNR_BPC_TH4);        // CAM+0x0810
            INIT_REG_INFO_ADDR(CAM_BNR_BPC_DTC);        // CAM+0x0814
            INIT_REG_INFO_ADDR(CAM_BNR_BPC_COR);        // CAM+0x0818
            //INIT_REG_INFO_ADDR(CAM_BPC_TBLI1);      // CAM+0x081C
            //INIT_REG_INFO_ADDR(CAM_BPC_TBLI2);      // CAM+0x0820
            INIT_REG_INFO_ADDR(CAM_BNR_BPC_TH1_C);      // CAM+0x0824
            INIT_REG_INFO_ADDR(CAM_BNR_BPC_TH2_C);      // CAM+0x0828
            INIT_REG_INFO_ADDR(CAM_BNR_BPC_TH3_C);      // CAM+0x082C

            INIT_REG_INFO_ADDR(CAM_BNR_NR1_CON);        // CAM+0x0850
            INIT_REG_INFO_ADDR(CAM_BNR_NR1_CT_CON);     // CAM+0x0854

            INIT_REG_INFO_ADDR(CAM_BNR_PDC_CON);
            INIT_REG_INFO_ADDR(CAM_BNR_PDC_GAIN_L0);
            INIT_REG_INFO_ADDR(CAM_BNR_PDC_GAIN_L1);
            INIT_REG_INFO_ADDR(CAM_BNR_PDC_GAIN_L2);
            INIT_REG_INFO_ADDR(CAM_BNR_PDC_GAIN_L3);
            INIT_REG_INFO_ADDR(CAM_BNR_PDC_GAIN_L4);
            INIT_REG_INFO_ADDR(CAM_BNR_PDC_GAIN_R0);
            INIT_REG_INFO_ADDR(CAM_BNR_PDC_GAIN_R1);
            INIT_REG_INFO_ADDR(CAM_BNR_PDC_GAIN_R2);
            INIT_REG_INFO_ADDR(CAM_BNR_PDC_GAIN_R3);
            INIT_REG_INFO_ADDR(CAM_BNR_PDC_GAIN_R4);
            INIT_REG_INFO_ADDR(CAM_BNR_PDC_TH_GB);
            INIT_REG_INFO_ADDR(CAM_BNR_PDC_TH_IA);
            INIT_REG_INFO_ADDR(CAM_BNR_PDC_TH_HD);
            INIT_REG_INFO_ADDR(CAM_BNR_PDC_SL);
            INIT_REG_INFO_ADDR(CAM_BNR_PDC_POS);

            INIT_REG_INFO_ADDR(CAM_BPCI_BASE_ADDR);
            INIT_REG_INFO_ADDR(CAM_BPCI_XSIZE);
            INIT_REG_INFO_ADDR(CAM_BPCI_YSIZE);
            INIT_REG_INFO_ADDR(CAM_BPCI_STRIDE);
        }
        else {
            INIT_REG_INFO_ADDR(CAM_BNR_D_BPC_CON);        // CAM+0x2800
            INIT_REG_INFO_ADDR(CAM_BNR_D_BPC_TH1);        // CAM+0x2804
            INIT_REG_INFO_ADDR(CAM_BNR_D_BPC_TH2);        // CAM+0x2808
            INIT_REG_INFO_ADDR(CAM_BNR_D_BPC_TH3);        // CAM+0x280C
            INIT_REG_INFO_ADDR(CAM_BNR_D_BPC_TH4);        // CAM+0x2810
            INIT_REG_INFO_ADDR(CAM_BNR_D_BPC_DTC);        // CAM+0x2814
            INIT_REG_INFO_ADDR(CAM_BNR_D_BPC_COR);        // CAM+0x2818
            //INIT_REG_INFO_ADDR(CAM_BPC_D_TBLI1);      // CAM+0x281C
            //INIT_REG_INFO_ADDR(CAM_BPC_D_TBLI2);      // CAM+0x2820
            INIT_REG_INFO_ADDR(CAM_BNR_D_BPC_TH1_C);      // CAM+0x2824
            INIT_REG_INFO_ADDR(CAM_BNR_D_BPC_TH2_C);      // CAM+0x2828
            INIT_REG_INFO_ADDR(CAM_BNR_D_BPC_TH3_C);      // CAM+0x282C

            INIT_REG_INFO_ADDR(CAM_BNR_D_NR1_CON);        // CAM+0x2850
            INIT_REG_INFO_ADDR(CAM_BNR_D_NR1_CT_CON);     // CAM+0x2854

            INIT_REG_INFO_ADDR(CAM_BNR_D_PDC_CON);
            INIT_REG_INFO_ADDR(CAM_BNR_D_PDC_GAIN_L0);
            INIT_REG_INFO_ADDR(CAM_BNR_D_PDC_GAIN_L1);
            INIT_REG_INFO_ADDR(CAM_BNR_D_PDC_GAIN_L2);
            INIT_REG_INFO_ADDR(CAM_BNR_D_PDC_GAIN_L3);
            INIT_REG_INFO_ADDR(CAM_BNR_D_PDC_GAIN_L4);
            INIT_REG_INFO_ADDR(CAM_BNR_D_PDC_GAIN_R0);
            INIT_REG_INFO_ADDR(CAM_BNR_D_PDC_GAIN_R1);
            INIT_REG_INFO_ADDR(CAM_BNR_D_PDC_GAIN_R2);
            INIT_REG_INFO_ADDR(CAM_BNR_D_PDC_GAIN_R3);
            INIT_REG_INFO_ADDR(CAM_BNR_D_PDC_GAIN_R4);
            INIT_REG_INFO_ADDR(CAM_BNR_D_PDC_TH_GB);
            INIT_REG_INFO_ADDR(CAM_BNR_D_PDC_TH_IA);
            INIT_REG_INFO_ADDR(CAM_BNR_D_PDC_TH_HD);
            INIT_REG_INFO_ADDR(CAM_BNR_D_PDC_SL);
            INIT_REG_INFO_ADDR(CAM_BNR_D_PDC_POS);

            INIT_REG_INFO_ADDR(CAM_BPCI_D_BASE_ADDR);
            INIT_REG_INFO_ADDR(CAM_BPCI_D_XSIZE);
            INIT_REG_INFO_ADDR(CAM_BPCI_D_YSIZE);
            INIT_REG_INFO_ADDR(CAM_BPCI_D_STRIDE);
    }
    }

    virtual ~ISP_MGR_BNR() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev, ESensorTG_T const eSensorTG = ESensorTG_1);

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
    setCCTBPCEnable(MBOOL bEnable)
    {
        m_bCCTBPCEnable = bEnable;
    }

    MVOID
    setCCTCTEnable(MBOOL bEnable)
    {
        m_bCCTCTEnable = bEnable;
    }

    MBOOL apply(EIspProfile_T eIspProfile);
    MBOOL apply(EIspProfile_T eIspProfile, isp_reg_t* pReg);

} ISP_MGR_BNR_T;

template <ESensorDev_T const eSensorDev, ESensorTG_T const eSensorTG = ESensorTG_1>
class ISP_MGR_BNR_DEV : public ISP_MGR_BNR_T
{
public:
    static
    ISP_MGR_BNR_T&
    getInstance()
    {
        static ISP_MGR_BNR_DEV<eSensorDev, eSensorTG> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_BNR_DEV()
        : ISP_MGR_BNR_T(eSensorDev, eSensorTG)
    {}

    virtual ~ISP_MGR_BNR_DEV() {}

};

#endif

