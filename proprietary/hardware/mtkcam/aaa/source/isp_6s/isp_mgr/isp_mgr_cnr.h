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
#ifndef _ISP_MGR_CNR_H_
#define _ISP_MGR_CNR_H_

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CNR
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define INIT_CNR_ADDR(reg)\
    INIT_REG_INFO_ADDR_P2_MULTI(ECNR_D1, reg, CNR_D1A_CNR_);\

typedef class ISP_MGR_CNR : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_CNR    MyType;
public:
    enum
    {
        ECNR_D1,
        ESubModule_NUM
    };
private:
    MBOOL m_bCNREnable[ESubModule_NUM];
    MBOOL m_bCCREnable[ESubModule_NUM];
    MBOOL m_bABFEnable[ESubModule_NUM];
    MBOOL m_bBOKEnable[ESubModule_NUM];
    MBOOL m_bCCTCNREnable;
    MBOOL m_bCCTCCREnable;
    MBOOL m_bCCTABFEnable;

    enum
    {
        ERegInfo_CNR_CON1,
        ERegInfo_CNR_CON2,
        ERegInfo_CNR_YAD1,
        ERegInfo_CNR_Y4LUT1,
        ERegInfo_CNR_Y4LUT2,
        ERegInfo_CNR_Y4LUT3,
        ERegInfo_CNR_L4LUT1,
        ERegInfo_CNR_L4LUT2,
        ERegInfo_CNR_L4LUT3,
        ERegInfo_CNR_CAD,
        ERegInfo_CNR_CB_VRNG,
        ERegInfo_CNR_CB_HRNG,
        ERegInfo_CNR_CR_VRNG,
        ERegInfo_CNR_CR_HRNG,
        ERegInfo_CNR_SL2,
        ERegInfo_CNR_MED1,
        ERegInfo_CNR_MED2,
        ERegInfo_CNR_MED3,
        ERegInfo_CNR_MED4,
        ERegInfo_CNR_MED5,
        ERegInfo_CNR_MED6,
        ERegInfo_CNR_MED7,
        ERegInfo_CNR_MED8,
        ERegInfo_CNR_MED9,
        ERegInfo_CNR_MED10,
        ERegInfo_CNR_MED11,
        ERegInfo_CNR_MED12,
        ERegInfo_CNR_MED13,
        ERegInfo_CNR_ACTC,
        ERegInfo_ABF_CON1,
        ERegInfo_ABF_CON2,
        ERegInfo_ABF_RCON,
        ERegInfo_ABF_YLUT,
        ERegInfo_ABF_CXLUT,
        ERegInfo_ABF_CYLUT,
        ERegInfo_ABF_YSP,
        ERegInfo_ABF_CXSP,
        ERegInfo_ABF_CYSP,
        ERegInfo_ABF_CLP,
        ERegInfo_CCR_CON,
        ERegInfo_CCR_SAT_CTRL,
        ERegInfo_CCR_YLUT,
        ERegInfo_CCR_UVLUT,
        ERegInfo_CCR_YLUT2,
        ERegInfo_CCR_UVLUT_SP,
        ERegInfo_CCR_HUE1,
        ERegInfo_CCR_HUE2,
        ERegInfo_CCR_HUE3,
        ERegInfo_CCR_L4LUT1,
        ERegInfo_CCR_L4LUT2,
        ERegInfo_CCR_L4LUT3,
        ERegInfo_NUM
    };

    RegInfo_T m_rIspRegInfo[ESubModule_NUM][ERegInfo_NUM];

protected:
    ISP_MGR_CNR(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, eSensorDev, ESubModule_NUM)
        , m_bCCTCNREnable(MTRUE)
        , m_bCCTABFEnable(MTRUE)
        , m_bCCTCCREnable(MTRUE)
    {
        for(int i=0; i<ESubModule_NUM; i++){
            m_bCNREnable[i]=     MFALSE;
            m_bCCREnable[i]=     MFALSE;
            m_bABFEnable[i]=     MFALSE;
            m_bBOKEnable[i]=     MFALSE;
            ::memset(m_rIspRegInfo[i], 0, sizeof(RegInfo_T)*ERegInfo_NUM);
        }
        INIT_CNR_ADDR(CNR_CON1);
        INIT_CNR_ADDR(CNR_CON2);
        INIT_CNR_ADDR(CNR_YAD1);
        INIT_CNR_ADDR(CNR_Y4LUT1);
        INIT_CNR_ADDR(CNR_Y4LUT2);
        INIT_CNR_ADDR(CNR_Y4LUT3);
        INIT_CNR_ADDR(CNR_L4LUT1);
        INIT_CNR_ADDR(CNR_L4LUT2);
        INIT_CNR_ADDR(CNR_L4LUT3);
        INIT_CNR_ADDR(CNR_CAD);
        INIT_CNR_ADDR(CNR_CB_VRNG);
        INIT_CNR_ADDR(CNR_CB_HRNG);
        INIT_CNR_ADDR(CNR_CR_VRNG);
        INIT_CNR_ADDR(CNR_CR_HRNG);
        INIT_CNR_ADDR(CNR_SL2);
        INIT_CNR_ADDR(CNR_MED1);
        INIT_CNR_ADDR(CNR_MED2);
        INIT_CNR_ADDR(CNR_MED3);
        INIT_CNR_ADDR(CNR_MED4);
        INIT_CNR_ADDR(CNR_MED5);
        INIT_CNR_ADDR(CNR_MED6);
        INIT_CNR_ADDR(CNR_MED7);
        INIT_CNR_ADDR(CNR_MED8);
        INIT_CNR_ADDR(CNR_MED9);
        INIT_CNR_ADDR(CNR_MED10);
        INIT_CNR_ADDR(CNR_MED11);
        INIT_CNR_ADDR(CNR_MED12);
        INIT_CNR_ADDR(CNR_MED13);
        INIT_CNR_ADDR(CNR_ACTC);
        INIT_CNR_ADDR(ABF_CON1);
        INIT_CNR_ADDR(ABF_CON2);
        INIT_CNR_ADDR(ABF_RCON);
        INIT_CNR_ADDR(ABF_YLUT);
        INIT_CNR_ADDR(ABF_CXLUT);
        INIT_CNR_ADDR(ABF_CYLUT);
        INIT_CNR_ADDR(ABF_YSP);
        INIT_CNR_ADDR(ABF_CXSP);
        INIT_CNR_ADDR(ABF_CYSP);
        INIT_CNR_ADDR(ABF_CLP);
        INIT_CNR_ADDR(CCR_CON);
        INIT_CNR_ADDR(CCR_SAT_CTRL);
        INIT_CNR_ADDR(CCR_YLUT);
        INIT_CNR_ADDR(CCR_UVLUT);
        INIT_CNR_ADDR(CCR_YLUT2);
        INIT_CNR_ADDR(CCR_UVLUT_SP);
        INIT_CNR_ADDR(CCR_HUE1);
        INIT_CNR_ADDR(CCR_HUE2);
        INIT_CNR_ADDR(CCR_HUE3);
        INIT_CNR_ADDR(CCR_L4LUT1);
        INIT_CNR_ADDR(CCR_L4LUT2);
        INIT_CNR_ADDR(CCR_L4LUT3);
    }

    virtual ~ISP_MGR_CNR() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: // Interfaces.

    template <class ISP_xxx_T>
    MyType& put(MUINT8 SubModuleIndex, ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(MUINT8 SubModuleIndex, ISP_xxx_T & rParam);

    MBOOL
    isCNREnable(MUINT8 SubModuleIndex)
    {
        return m_bCNREnable[SubModuleIndex];
    }

    MBOOL
    isCCREnable(MUINT8 SubModuleIndex)
    {
        return m_bCCREnable[SubModuleIndex];
    }

    MBOOL
    isABFEnable(MUINT8 SubModuleIndex)
    {
        return m_bABFEnable[SubModuleIndex];
    }

    MBOOL
    isBOKEnable(MUINT8 SubModuleIndex)
    {
        return m_bBOKEnable[SubModuleIndex];
    }

    MBOOL
    isCCTCNREnable()
    {
        return m_bCCTCNREnable;
    }

    MBOOL
    isCCTCCREnable()
    {
        return m_bCCTCCREnable;
    }

    MBOOL
    isCCTABFEnable()
    {
        return m_bCCTABFEnable;
    }

    MVOID
    setCNREnable(MUINT8 SubModuleIndex, MBOOL bEnable)
    {
        m_bCNREnable[SubModuleIndex] = bEnable;
    }

    MVOID
    setCCREnable(MUINT8 SubModuleIndex, MBOOL bEnable)
    {
        m_bCCREnable[SubModuleIndex] = bEnable;
    }

    MVOID
    setABFEnable(MUINT8 SubModuleIndex, MBOOL bEnable)
    {
        m_bABFEnable[SubModuleIndex] = bEnable;
    }

    MVOID
    setBOKEnable(MUINT8 SubModuleIndex, MBOOL bEnable)
    {
        m_bBOKEnable[SubModuleIndex] = bEnable;
    }

    MVOID
    setCCTCNREnable(MBOOL bEnable)
    {
        m_bCCTCNREnable = bEnable;
    }

    MVOID
    setCCTCCREnable(MBOOL bEnable)
    {
        m_bCCTCCREnable = bEnable;
    }

    MVOID
    setCCTABFEnable(MBOOL bEnable)
    {
        m_bCCTABFEnable = bEnable;
    }

    MBOOL apply_P2(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, dip_a_reg_t* pReg);

} ISP_MGR_CNR_T;

template <ESensorDev_T const eSensorDev>
class ISP_MGR_CNR_DEV : public ISP_MGR_CNR_T
{
public:
    static
    ISP_MGR_CNR_T&
    getInstance()
    {
        static ISP_MGR_CNR_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_CNR_DEV()
        : ISP_MGR_CNR_T(eSensorDev)
    {}

    virtual ~ISP_MGR_CNR_DEV() {}

};
#endif