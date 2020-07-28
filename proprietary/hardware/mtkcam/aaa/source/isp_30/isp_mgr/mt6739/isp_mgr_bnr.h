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

//#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mutex>

using namespace NSCam;

typedef class ISP_MGR_BNR : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_BNR    MyType;
private:
    ESensorTG_T const m_eSensorTG;
    MBOOL m_bBPCEnable;
    MBOOL m_bCTEnable;
    MBOOL m_bPDCEnable;
    MBOOL m_bAF_PDCEnable;
    MBOOL m_bCCTBPCEnable;
    MBOOL m_bCCTCTEnable;
    MBOOL m_bCCTPDCEnable;
    MUINT32 m_u4StartAddr;
    SPDOHWINFO_T m_sPDOHWInfo;
    MBOOL m_bBPCIValid;
    MUINT32 m_u4PDSensorFmt;
    MVOID* m_pPDCBuffer_1;
    MVOID* m_pPDCBuffer_2;
    CAMERA_PD_TBL_STRUCT* m_pBpciBuf_1;
    CAMERA_PD_TBL_STRUCT* m_pBpciBuf_2;
    EIndex_PDC_TBL_T m_PDTBL_Idx;
    std::mutex            m_Lock;

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
        , m_bAF_PDCEnable(MFALSE)
        , m_bCCTBPCEnable(MTRUE)
        , m_bCCTCTEnable(MTRUE)
        , m_bCCTPDCEnable(MTRUE)
        , m_u4StartAddr((eSensorTG == ESensorTG_1) ? REG_ADDR(CAM_BNR_BPC_CON) : REG_ADDR(CAM_BNR_D_BPC_CON))
        , m_bBPCIValid(MFALSE)
        , m_u4PDSensorFmt(0)
        , m_pPDCBuffer_1(NULL)
        , m_pPDCBuffer_2(NULL)
        , m_pBpciBuf_1(NULL)
        , m_pBpciBuf_2(NULL)
        , m_PDTBL_Idx(eIDX_PDC_FULL)
        , m_Lock()
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

    virtual ~ISP_MGR_BNR() {
        if (m_pPDCBuffer_1 != nullptr) {
            // using sp to release IImageBuffer
            sp<IImageBuffer> temp_pdc_1 = static_cast<IImageBuffer*>(m_pPDCBuffer_1);
        }

        if (m_pPDCBuffer_2 != nullptr) {
            // using sp to release IImageBuffer
            sp<IImageBuffer> temp_pdc_2 = static_cast<IImageBuffer*>(m_pPDCBuffer_2);
        }
    }
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
    isAF_PDCEnable()
    {
        return m_bAF_PDCEnable;
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
    setAF_PDCEnable(MBOOL bEnable)
    {
        m_bAF_PDCEnable = bEnable;
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

    MBOOL getBPCItable(
        UINT32& bpci_xsize,
        UINT32& bpci_ysize,
        UINT32& pdo_xsize,
        UINT32& pdo_ysize,
        MUINTPTR& phy_addr,
        MUINTPTR& vir_addr,
        MINT32& memID,
        EIndex_PDC_TBL_T Type);

    MVOID
    setPDSensorInfo(MUINT32 u4PDSensor)
    {
        m_u4PDSensorFmt = u4PDSensor;
         /*********************************************************************************
         * PD sensor format :
         *   please reference IMGSENSOR_PDAF_SUPPORT_TYPE_ENUM in kd_imgsensor_define.h
         *      PDAF_SUPPORT_NA = 0,
         *      PDAF_SUPPORT_RAW = 1,
         *      PDAF_SUPPORT_CAMSV = 2,
         *      PDAF_SUPPORT_CAMSV_LEGACY = 3,
         *      PDAF_SUPPORT_RAW_DUALPD = 4,
         *      PDAF_SUPPORT_CAMSV_DUALPD = 5,
         *      PDAF_SUPPORT_RAW_LEGACY = 6,
         *********************************************************************************/
    }

    MVOID
    setPD_TBL_IDX(EIndex_PDC_TBL_T ePDTBL_Idx)
    {
        m_PDTBL_Idx = ePDTBL_Idx;
    }

    MBOOL setBPCIBuf(CAMERA_BPCI_STRUCT* buf);

    MVOID BPCI_Config(MUINT32 u4PDSensor, CAMERA_BPCI_STRUCT* buf){

        setPDSensorInfo(u4PDSensor);

        setBPCIBuf(buf);

        m_bBPCIValid = getBPCItable(
                           m_sPDOHWInfo.u4Bpci_xsz,
                           m_sPDOHWInfo.u4Bpci_ysz,
                           m_sPDOHWInfo.u4Pdo_xsz,
                           m_sPDOHWInfo.u4Pdo_ysz,
                           m_sPDOHWInfo.phyAddrBpci_tbl,
                           m_sPDOHWInfo.virAddrBpci_tbl,
                           m_sPDOHWInfo.i4memID,
                           m_PDTBL_Idx);

    }


    MVOID unlockBPCIBuf();

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

typedef class ISP_MGR_BNR2 : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_BNR2    MyType;
private:
    ESensorTG_T const m_eSensorTG;
    MBOOL m_bBPCEnable;
    MBOOL m_bCTEnable;
    MBOOL m_bPDCEnable;
    MBOOL m_bAF_PDCEnable;
    MBOOL m_bCCTBPCEnable;
    MBOOL m_bCCTCTEnable;
    MBOOL m_bCCTPDCEnable;
    MUINT32 m_u4StartAddr;
    SPDOHWINFO_T m_sPDOHWInfo;
    MBOOL m_bBPCIValid;
    MUINT32 m_u4PDSensorFmt;
    MVOID* m_pPDCBuffer_1;
    MVOID* m_pPDCBuffer_2;
    CAMERA_PD_TBL_STRUCT* m_pBpciBuf_1;
    CAMERA_PD_TBL_STRUCT* m_pBpciBuf_2;
    EIndex_PDC_TBL_T m_PDTBL_Idx;
    std::mutex            m_Lock;

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
    ISP_MGR_BNR2(ESensorDev_T const eSensorDev, ESensorTG_T const eSensorTG)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr, eSensorDev)
        , m_eSensorTG(eSensorTG)
        , m_bBPCEnable(MTRUE)
        , m_bCTEnable(MTRUE)
        , m_bPDCEnable(MFALSE)
        , m_bAF_PDCEnable(MFALSE)
        , m_bCCTBPCEnable(MTRUE)
        , m_bCCTCTEnable(MTRUE)
        , m_bCCTPDCEnable(MTRUE)
        , m_u4StartAddr((eSensorTG == ESensorTG_1) ? REG_ADDR(CAM_BNR_BPC_CON) : REG_ADDR(CAM_BNR_D_BPC_CON))
        , m_bBPCIValid(MFALSE)
        , m_u4PDSensorFmt(0)
        , m_pPDCBuffer_1(NULL)
        , m_pPDCBuffer_2(NULL)
        , m_pBpciBuf_1(NULL)
        , m_pBpciBuf_2(NULL)
        , m_PDTBL_Idx(eIDX_PDC_FULL)
        , m_Lock()
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

    virtual ~ISP_MGR_BNR2() {
        if (m_pPDCBuffer_1 != nullptr) {
            std::string strName_pdc_1  = "PDC_tbl_1";
            // using sp to release IImageBuffer
            sp<IImageBuffer> temp_pdc_1 = static_cast<IImageBuffer*>(m_pPDCBuffer_1);
            temp_pdc_1->unlockBuf(strName_pdc_1.c_str());
        }

        if (m_pPDCBuffer_2 != nullptr) {
            std::string strName_pdc_2  = "PDC_tbl_2";
            // using sp to release IImageBuffer
            sp<IImageBuffer> temp_pdc_2 = static_cast<IImageBuffer*>(m_pPDCBuffer_2);
            temp_pdc_2->unlockBuf(strName_pdc_2.c_str());
        }
    }
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
    isAF_PDCEnable()
    {
        return m_bAF_PDCEnable;
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
    setAF_PDCEnable(MBOOL bEnable)
    {
        m_bAF_PDCEnable = bEnable;
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

    MBOOL getBPCItable(
        UINT32& bpci_xsize,
        UINT32& bpci_ysize,
        UINT32& pdo_xsize,
        UINT32& pdo_ysize,
        MUINTPTR& phy_addr,
        MUINTPTR& vir_addr,
        MINT32& memID,
        EIndex_PDC_TBL_T Type);

    MVOID
    setPDSensorInfo(MUINT32 u4PDSensor)
    {
        m_u4PDSensorFmt = u4PDSensor;
         /*********************************************************************************
         * PD sensor format :
         *   please reference IMGSENSOR_PDAF_SUPPORT_TYPE_ENUM in kd_imgsensor_define.h
         *      PDAF_SUPPORT_NA = 0,
         *      PDAF_SUPPORT_RAW = 1,
         *      PDAF_SUPPORT_CAMSV = 2,
         *      PDAF_SUPPORT_CAMSV_LEGACY = 3,
         *      PDAF_SUPPORT_RAW_DUALPD = 4,
         *      PDAF_SUPPORT_CAMSV_DUALPD = 5,
         *      PDAF_SUPPORT_RAW_LEGACY = 6,
         *********************************************************************************/
    }

    MVOID
    setPD_TBL_IDX(EIndex_PDC_TBL_T ePDTBL_Idx)
    {
        m_PDTBL_Idx = ePDTBL_Idx;
    }

    MBOOL setBPCIBuf(CAMERA_BPCI_STRUCT* buf);

    MVOID BPCI_Config(MUINT32 u4PDSensor, CAMERA_BPCI_STRUCT* buf){

        setPDSensorInfo(u4PDSensor);

        setBPCIBuf(buf);

        m_bBPCIValid = getBPCItable(
                           m_sPDOHWInfo.u4Bpci_xsz,
                           m_sPDOHWInfo.u4Bpci_ysz,
                           m_sPDOHWInfo.u4Pdo_xsz,
                           m_sPDOHWInfo.u4Pdo_ysz,
                           m_sPDOHWInfo.phyAddrBpci_tbl,
                           m_sPDOHWInfo.virAddrBpci_tbl,
                           m_sPDOHWInfo.i4memID,
                           m_PDTBL_Idx);

    }


    MVOID unlockBPCIBuf();

    MBOOL apply(EIspProfile_T eIspProfile);
    MBOOL apply(EIspProfile_T eIspProfile, isp_reg_t* pReg);

} ISP_MGR_BNR2_T;

template <ESensorDev_T const eSensorDev, ESensorTG_T const eSensorTG = ESensorTG_1>
class ISP_MGR_BNR2_DEV : public ISP_MGR_BNR2_T
{
public:
    static
    ISP_MGR_BNR2_T&
    getInstance()
    {
        static ISP_MGR_BNR2_DEV<eSensorDev, eSensorTG> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_BNR2_DEV()
        : ISP_MGR_BNR2_T(eSensorDev, eSensorTG)
    {}

    virtual ~ISP_MGR_BNR2_DEV() {}

};

#endif

