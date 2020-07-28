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
#ifndef _ISP_MGR_CTL_H_
#define _ISP_MGR_CTL_H_

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ISP Enable (Pass1@TG1)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_CAMCTL : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_CAMCTL    MyType;
private:

    enum
    {
        ERegInfo_CAMCTL_R1_CAMCTL_EN,
        ERegInfo_CAMCTL_R1_CAMCTL_EN2,
        ERegInfo_CAMCTL_R1_CAMCTL_EN3,
        ERegInfo_NUM
    };

    RegInfo_T m_rIspRegInfo[ERegInfo_NUM];

public:
    ISP_MGR_CAMCTL(MUINT32 const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, eSensorDev)
    {
        ::memset(m_rIspRegInfo, 0, sizeof(RegInfo_T)*ERegInfo_NUM);
        // register info addr init
        INIT_REG_INFO_ADDR_P1(CAMCTL_R1_CAMCTL_EN);
        INIT_REG_INFO_ADDR_P1(CAMCTL_R1_CAMCTL_EN2);
        INIT_REG_INFO_ADDR_P1(CAMCTL_R1_CAMCTL_EN3);
    }

    virtual ~ISP_MGR_CAMCTL() {}

public:
    static MyType&  getInstance(MUINT32 const eSensorDev);


//==========OBC Pass1==========
    MVOID
    setEnable_OBC_R1(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAMCTL_EN_T*>(REG_INFO_VALUE_PTR(CAMCTL_R1_CAMCTL_EN))->CAMCTL_OBC_R1_EN = bEnable;
    }
    MVOID
    setEnable_MOBC_R2(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAMCTL_EN_T*>(REG_INFO_VALUE_PTR(CAMCTL_R1_CAMCTL_EN))->CAMCTL_MOBC_R2_EN = bEnable;
    }
    MVOID
    setEnable_OBC_R2(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAMCTL_EN2_T*>(REG_INFO_VALUE_PTR(CAMCTL_R1_CAMCTL_EN2))->CAMCTL_OBC_R2_EN = bEnable;
    }
    MVOID
    setEnable_MOBC_R3(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAMCTL_EN_T*>(REG_INFO_VALUE_PTR(CAMCTL_R1_CAMCTL_EN))->CAMCTL_MOBC_R3_EN = bEnable;
    }

//==========BPC Pass1==========
    MVOID
    setEnable_BPC_R1(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAMCTL_EN_T*>(REG_INFO_VALUE_PTR(CAMCTL_R1_CAMCTL_EN))->CAMCTL_BPC_R1_EN = bEnable;
    }
    MVOID
    setEnable_BPC_R2(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAMCTL_EN2_T*>(REG_INFO_VALUE_PTR(CAMCTL_R1_CAMCTL_EN2))->CAMCTL_BPC_R2_EN = bEnable;
    }

//==========FUS Pass1==========
#if 0
    MVOID
    setEnable_FUS_R1(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAMCTL_EN_T*>(REG_INFO_VALUE_PTR(CAMCTL_R1_CAMCTL_EN))->CAMCTL_FUS_R1_EN = bEnable;
    }

    MVOID
    setEnable_ZFUS_R1(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAMCTL_EN2_T*>(REG_INFO_VALUE_PTR(CAMCTL_R1_CAMCTL_EN2))->CAMCTL_ZFUS_R1_EN = bEnable;
    }
#endif

//==========DGN Pass1==========
    MVOID
    setEnable_DGN_R1(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAMCTL_EN_T*>(REG_INFO_VALUE_PTR(CAMCTL_R1_CAMCTL_EN))->CAMCTL_DGN_R1_EN = bEnable;
    }

//==========LSC Pass1==========
    MVOID
    setEnable_LSC_R1(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAMCTL_EN_T*>(REG_INFO_VALUE_PTR(CAMCTL_R1_CAMCTL_EN))->CAMCTL_LSC_R1_EN = bEnable;
    }

//==========WB Pass1==========
    MVOID
    setEnable_WB_R1(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAMCTL_EN_T*>(REG_INFO_VALUE_PTR(CAMCTL_R1_CAMCTL_EN))->CAMCTL_WB_R1_EN = bEnable;
    }

//==========HLR Pass1==========
    MVOID
    setEnable_HLR_R1(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAMCTL_EN2_T*>(REG_INFO_VALUE_PTR(CAMCTL_R1_CAMCTL_EN2))->CAMCTL_HLR_R1_EN = bEnable;
    }

//==========LTM Pass1==========
    MVOID
    setEnable_LTM_R1(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAMCTL_EN_T*>(REG_INFO_VALUE_PTR(CAMCTL_R1_CAMCTL_EN))->CAMCTL_LTM_R1_EN = bEnable;
    }
    MVOID
    setEnable_LTMS_R1(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAMCTL_EN_T*>(REG_INFO_VALUE_PTR(CAMCTL_R1_CAMCTL_EN))->CAMCTL_LTMS_R1_EN = bEnable;
    }

//==========SLK Pass1==========
    MVOID
    setEnable_SLK_R1(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAMCTL_EN_T*>(REG_INFO_VALUE_PTR(CAMCTL_R1_CAMCTL_EN))->CAMCTL_SLK_R1_EN = bEnable;
    }
    MVOID
    setEnable_SLK_R2(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAMCTL_EN2_T*>(REG_INFO_VALUE_PTR(CAMCTL_R1_CAMCTL_EN2))->CAMCTL_SLK_R2_EN = bEnable;
    }

//==========DM Pass1==========
    MVOID
    setEnable_DM_R1(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAMCTL_EN2_T*>(REG_INFO_VALUE_PTR(CAMCTL_R1_CAMCTL_EN2))->CAMCTL_DM_R1_EN = bEnable;
    }

//==========FLC Pass1==========
    MVOID
    setEnable_FLC_R1(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAMCTL_EN3_T*>(REG_INFO_VALUE_PTR(CAMCTL_R1_CAMCTL_EN3))->CAMCTL_FLC_R1_EN = bEnable;
    }

//==========LCES Pass1==========
    MVOID
    setEnable_LCES_R1(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAMCTL_EN_T*>(REG_INFO_VALUE_PTR(CAMCTL_R1_CAMCTL_EN))->CAMCTL_LCES_R1_EN = bEnable;
    }

//==========CCM Pass1==========
    MVOID
    setEnable_CCM_R1(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAMCTL_EN2_T*>(REG_INFO_VALUE_PTR(CAMCTL_R1_CAMCTL_EN2))->CAMCTL_CCM_R1_EN = bEnable;
    }

//==========GGM Pass1==========
    MVOID
    setEnable_GGM_R1(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAMCTL_EN2_T*>(REG_INFO_VALUE_PTR(CAMCTL_R1_CAMCTL_EN2))->CAMCTL_GGM_R1_EN = bEnable;
    }
    MVOID
    setEnable_GGM_R2(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAMCTL_EN3_T*>(REG_INFO_VALUE_PTR(CAMCTL_R1_CAMCTL_EN3))->CAMCTL_GGM_R2_EN = bEnable;
    }

//==========G2C Pass1==========
    MVOID
    setEnable_G2C_R1(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAMCTL_EN2_T*>(REG_INFO_VALUE_PTR(CAMCTL_R1_CAMCTL_EN2))->CAMCTL_G2C_R1_EN = bEnable;
    }
    MVOID
    setEnable_G2C_R2(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAMCTL_EN3_T*>(REG_INFO_VALUE_PTR(CAMCTL_R1_CAMCTL_EN3))->CAMCTL_GGM_R2_EN = bEnable;
    }

//==========YNRS Pass1==========
    MVOID
    setEnable_YNRS_R1(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAMCTL_EN3_T*>(REG_INFO_VALUE_PTR(CAMCTL_R1_CAMCTL_EN3))->CAMCTL_YNRS_R1_EN = bEnable;
    }

public: // Interfaces.

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

    MBOOL apply(RAWIspCamInfo& rRawIspCamInfo, TuningMgr& rTuning, MINT32 i4SubsampleIdex=0);

} ISP_MGR_CAMCTL_T;



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ISP Enable (Pass2)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_DIPCTL : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_DIPCTL    MyType;
private:

    enum
    {
        ERegInfo_DIPCTL_D1A_DIPCTL_RGB_EN1,
        ERegInfo_DIPCTL_D1A_DIPCTL_RGB_EN2,
        ERegInfo_DIPCTL_D1A_DIPCTL_YUV_EN1,
        ERegInfo_DIPCTL_D1A_DIPCTL_YUV_EN2,
        ERegInfo_NUM
    };
    RegInfo_T     m_rIspRegInfo[ERegInfo_NUM];

public:
    ISP_MGR_DIPCTL(MUINT32 const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, eSensorDev)
    {
        ::memset(m_rIspRegInfo, 0, sizeof(RegInfo_T)*ERegInfo_NUM);
        INIT_REG_INFO_ADDR_P2(DIPCTL_D1A_DIPCTL_RGB_EN1);
        INIT_REG_INFO_ADDR_P2(DIPCTL_D1A_DIPCTL_RGB_EN2);
        INIT_REG_INFO_ADDR_P2(DIPCTL_D1A_DIPCTL_YUV_EN1);
        INIT_REG_INFO_ADDR_P2(DIPCTL_D1A_DIPCTL_YUV_EN2);
    }

    virtual ~ISP_MGR_DIPCTL() {}

public:
    static MyType&  getInstance(MUINT32 const eSensorDev);

public: // Interfaces.

    template <class ISP_xxx_T>
    MBOOL get(ISP_xxx_T & rParam, const dip_x_reg_t* pReg) const;

    MBOOL apply(const RAWIspCamInfo& rRawIspCamInfo, dip_x_reg_t* pReg);
} ISP_MGR_DIPCTL_T;



#endif

