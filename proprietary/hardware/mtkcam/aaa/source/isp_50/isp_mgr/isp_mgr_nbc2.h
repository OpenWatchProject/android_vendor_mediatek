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
#ifndef _ISP_MGR_NBC2_H_
#define _ISP_MGR_NBC2_H_

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  NBC2 (ANR2 + CCR + BOK)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_NBC2 : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_NBC2    MyType;
private:
    MBOOL m_bANR2_ENYEnable;
    MBOOL m_bANR2_ENCEnable;
    MBOOL m_bANR2_Enable;
    MBOOL m_bCCREnable;
    MBOOL m_bBOKEnable;
    MBOOL m_bABFEnable;
    MBOOL m_bCCTANR2Enable;
    MBOOL m_bCCTCCREnable;
    MBOOL m_bCCTBOKEnable;
    MBOOL m_bCCTABFEnable;
    MUINT32 m_u4StartAddr; // for debug purpose: 0x0A20

    enum
    {
        ERegInfo_DIP_X_NBC2_ANR2_CON1,
        ERegInfo_DIP_X_NBC2_ANR2_CON2,
        ERegInfo_DIP_X_NBC2_ANR2_YAD1,
        ERegInfo_DIP_X_NBC2_ANR2_Y4LUT1,
        ERegInfo_DIP_X_NBC2_ANR2_Y4LUT2,
        ERegInfo_DIP_X_NBC2_ANR2_Y4LUT3,
        ERegInfo_DIP_X_NBC2_ANR2_L4LUT1,
        ERegInfo_DIP_X_NBC2_ANR2_L4LUT2,
        ERegInfo_DIP_X_NBC2_ANR2_L4LUT3,
        ERegInfo_DIP_X_NBC2_ANR2_CAD,
        ERegInfo_DIP_X_NBC2_ANR2_PTC,
        ERegInfo_DIP_X_NBC2_ANR2_SL2,
        ERegInfo_DIP_X_NBC2_ANR2_MED1,
        ERegInfo_DIP_X_NBC2_ANR2_MED2,
        ERegInfo_DIP_X_NBC2_ANR2_MED3,
        ERegInfo_DIP_X_NBC2_ANR2_MED4,
        ERegInfo_DIP_X_NBC2_ANR2_MED5,
        ERegInfo_DIP_X_NBC2_ANR2_ACTC,
        ERegInfo_DIP_X_NBC2_CCR_CON,
        ERegInfo_DIP_X_NBC2_CCR_YLUT,
        ERegInfo_DIP_X_NBC2_CCR_UVLUT,
        ERegInfo_DIP_X_NBC2_CCR_YLUT2,
        ERegInfo_DIP_X_NBC2_CCR_SAT_CTRL,
        ERegInfo_DIP_X_NBC2_CCR_UVLUT_SP,
        ERegInfo_DIP_X_NBC2_CCR_HUE1,
        ERegInfo_DIP_X_NBC2_CCR_HUE2,
        ERegInfo_DIP_X_NBC2_CCR_HUE3,
        ERegInfo_DIP_X_NBC2_CCR_L4LUT1,
        ERegInfo_DIP_X_NBC2_CCR_L4LUT2,
        ERegInfo_DIP_X_NBC2_CCR_L4LUT3,
        //ERegInfo_DIP_X_NBC2_BOK_CON,
        //ERegInfo_DIP_X_NBC2_BOK_TUN,
        ERegInfo_DIP_X_NBC2_ABF_CON1,
        ERegInfo_DIP_X_NBC2_ABF_CON2,
        ERegInfo_DIP_X_NBC2_ABF_RCON,
        ERegInfo_DIP_X_NBC2_ABF_YLUT,
        ERegInfo_DIP_X_NBC2_ABF_CXLUT,
        ERegInfo_DIP_X_NBC2_ABF_CYLUT,
        ERegInfo_DIP_X_NBC2_ABF_YSP,
        ERegInfo_DIP_X_NBC2_ABF_CXSP,
        ERegInfo_DIP_X_NBC2_ABF_CYSP,
        ERegInfo_DIP_X_NBC2_ABF_CLP,

        ERegInfo_NUM
    };
    RegInfo_T m_rIspRegInfo[ERegInfo_NUM];

public:
    ISP_MGR_NBC2(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr, eSensorDev)
        , m_bANR2_ENYEnable(MTRUE)
        , m_bANR2_ENCEnable(MTRUE)
        , m_bANR2_Enable(MTRUE)
        , m_bCCREnable(MTRUE)
        , m_bBOKEnable(MTRUE)
        , m_bABFEnable(MTRUE)
        , m_bCCTANR2Enable(MTRUE)
        , m_bCCTCCREnable(MTRUE)
        , m_bCCTBOKEnable(MTRUE)
        , m_bCCTABFEnable(MTRUE)
        , m_u4StartAddr(REG_ADDR_P2(DIP_X_NBC2_ANR2_CON1))
    {
        // register info addr init
#if 1
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC2_ANR2_CON1);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC2_ANR2_CON2);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC2_ANR2_YAD1);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC2_ANR2_Y4LUT1);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC2_ANR2_Y4LUT2);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC2_ANR2_Y4LUT3);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC2_ANR2_L4LUT1);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC2_ANR2_L4LUT2);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC2_ANR2_L4LUT3);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC2_ANR2_CAD);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC2_ANR2_PTC);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC2_ANR2_SL2);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC2_ANR2_MED1);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC2_ANR2_MED2);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC2_ANR2_MED3);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC2_ANR2_MED4);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC2_ANR2_MED5);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC2_ANR2_ACTC);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC2_CCR_CON);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC2_CCR_YLUT);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC2_CCR_UVLUT);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC2_CCR_YLUT2);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC2_CCR_SAT_CTRL);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC2_CCR_UVLUT_SP);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC2_CCR_HUE1);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC2_CCR_HUE2);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC2_CCR_HUE3);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC2_CCR_L4LUT1);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC2_CCR_L4LUT2);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC2_CCR_L4LUT3);
        //INIT_REG_INFO_ADDR_P2(DIP_X_NBC2_BOK_CON);
        //INIT_REG_INFO_ADDR_P2(DIP_X_NBC2_BOK_TUN);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC2_ABF_CON1);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC2_ABF_CON2);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC2_ABF_RCON);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC2_ABF_YLUT);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC2_ABF_CXLUT);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC2_ABF_CYLUT);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC2_ABF_YSP);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC2_ABF_CXSP);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC2_ABF_CYSP);
        INIT_REG_INFO_ADDR_P2(DIP_X_NBC2_ABF_CLP);
#endif
    }
    virtual ~ISP_MGR_NBC2() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: // Interfaces.

    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MBOOL get(ISP_xxx_T & rParam, const dip_x_reg_t* pReg) const;


    MBOOL
    isANR2Enable()
    {
         return m_bANR2_Enable;
    }

    MBOOL
    isCCREnable()
    {
        return m_bCCREnable;
    }

    MBOOL
    isBOKEnable()
    {
        return m_bBOKEnable;
    }

    MBOOL
    isABFEnable()
    {
        return m_bABFEnable;
    }

    MBOOL
    isCCTCCREnable()
    {
        return m_bCCTCCREnable;
    }

    MBOOL
    isCCTANR2Enable()
    {
        return m_bCCTANR2Enable;
    }

    MBOOL
    isCCTBOKEnable()
    {
        return m_bCCTBOKEnable;
    }

    MBOOL
    isCCTABFEnable()
    {
        return m_bCCTABFEnable;
    }

    MVOID
    setANR2Enable(MBOOL bEnable)
    {
         m_bANR2_Enable = bEnable;
    }

    MVOID
    setCCREnable(MBOOL bEnable)
    {
        m_bCCREnable = bEnable;
    }

    MVOID
    setBOKEnable(MBOOL bEnable)
    {
        m_bBOKEnable = bEnable;
    }

    MVOID
    setABFEnable(MBOOL bEnable)
    {
        m_bABFEnable = bEnable;
    }

    MVOID
    setCCTANR2Enable(MBOOL bEnable)
    {
         m_bCCTANR2Enable = bEnable;
    }

    MVOID
    setCCTCCREnable(MBOOL bEnable)
    {
        m_bCCTCCREnable = bEnable;
    }

    MVOID
    setCCTBOKEnable(MBOOL bEnable)
    {
        m_bCCTBOKEnable = bEnable;
    }

    MVOID
    setCCTABFEnable(MBOOL bEnable)
    {
        m_bCCTABFEnable = bEnable;
    }

    MBOOL apply(EIspProfile_T eIspProfile, dip_x_reg_t* pReg);
} ISP_MGR_NBC2_T;


#endif

