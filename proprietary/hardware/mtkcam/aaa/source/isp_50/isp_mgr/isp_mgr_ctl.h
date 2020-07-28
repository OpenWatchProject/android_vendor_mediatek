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
typedef class ISP_MGR_CTL_EN_P1 : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_CTL_EN_P1    MyType;
private:
    MUINT32 m_u4StartAddr; // for debug purpose: CAM+0004H

    enum
    {
        ERegInfo_CAM_CTL_EN,
        ERegInfo_CAM_CTL_EN2,
        ERegInfo_CAM_CTL_DMA_EN,
        ERegInfo_NUM
    };

    RegInfo_T m_rIspRegInfo[ERegInfo_NUM];

public:
    ISP_MGR_CTL_EN_P1(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr, eSensorDev)
        , m_u4StartAddr(REG_ADDR_P1(CAM_CTL_EN))
    {
        // register info addr init
        INIT_REG_INFO_ADDR_P1(CAM_CTL_EN);
        INIT_REG_INFO_VALUE(CAM_CTL_EN, 0x00000000);
        INIT_REG_INFO_ADDR_P1(CAM_CTL_EN2);
        INIT_REG_INFO_VALUE(CAM_CTL_EN2, 0x00000000);
        INIT_REG_INFO_ADDR_P1(CAM_CTL_DMA_EN);
        INIT_REG_INFO_VALUE(CAM_CTL_DMA_EN, 0x00000000);
    }

    virtual ~ISP_MGR_CTL_EN_P1() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

    MVOID
    setEnable_SL2F(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAM_CTL_EN2_T*>(REG_INFO_VALUE_PTR(CAM_CTL_EN2))->SL2F_EN = bEnable;
    }

    MVOID
    setEnable_DBS(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAM_CTL_EN_T*>(REG_INFO_VALUE_PTR(CAM_CTL_EN))->DBS_EN = bEnable;
    }

    MVOID
    setEnable_ADBS(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAM_CTL_EN2_T*>(REG_INFO_VALUE_PTR(CAM_CTL_EN2))->ADBS_EN = bEnable;
    }

    MVOID
    setEnable_OB(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAM_CTL_EN_T*>(REG_INFO_VALUE_PTR(CAM_CTL_EN))->OBC_EN = bEnable;
    }

    MVOID
    setEnable_BNR(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAM_CTL_EN_T*>(REG_INFO_VALUE_PTR(CAM_CTL_EN))->BNR_EN = bEnable;
    }

    MVOID
    setEnable_LSC(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAM_CTL_EN_T*>(REG_INFO_VALUE_PTR(CAM_CTL_EN))->LSC_EN = bEnable;
    }

    MVOID
    setEnable_RPG(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAM_CTL_EN_T*>(REG_INFO_VALUE_PTR(CAM_CTL_EN))->RPG_EN = bEnable;
    }

    MVOID
    setEnable_RMG(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAM_CTL_EN_T*>(REG_INFO_VALUE_PTR(CAM_CTL_EN))->RMG_EN = bEnable;
    }

    MVOID
    setEnable_RMM(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAM_CTL_EN_T*>(REG_INFO_VALUE_PTR(CAM_CTL_EN))->RMM_EN = bEnable;
    }

    MVOID
    setEnable_CPN(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAM_CTL_EN2_T*>(REG_INFO_VALUE_PTR(CAM_CTL_EN2))->CPN_EN = bEnable;
    }

    MVOID
    setEnable_DCPN(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAM_CTL_EN2_T*>(REG_INFO_VALUE_PTR(CAM_CTL_EN2))->DCPN_EN = bEnable;
    }

    MVOID
    setEnable_LCS(MBOOL bEnable)
    {
        reinterpret_cast<ISP_CAM_CTL_EN_T*>(REG_INFO_VALUE_PTR(CAM_CTL_EN))->LCS_EN = bEnable;
    }

public: // Interfaces.

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

    MBOOL apply(RAWIspCamInfo& rRawIspCamInfo, TuningMgr& rTuning, MINT32 i4SubsampleIdex=0);

} ISP_MGR_CTL_EN_P1_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ISP Enable (Pass2)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_CTL_EN_P2 : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_CTL_EN_P2    MyType;
private:
    MUINT32 m_u4StartAddr; // for debug purpose: CAM+0018H

    enum
    {
        ERegInfo_DIP_X_CTL_YUV_EN,
        ERegInfo_DIP_X_CTL_YUV2_EN,
        ERegInfo_DIP_X_CTL_RGB_EN,
        ERegInfo_DIP_X_CTL_RGB2_EN,
        ERegInfo_NUM
    };
    RegInfo_T     m_rIspRegInfo[ERegInfo_NUM];

public:
    ISP_MGR_CTL_EN_P2(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr, eSensorDev)
        , m_u4StartAddr(REG_ADDR_P2(DIP_X_CTL_YUV_EN))
    {
        INIT_REG_INFO_ADDR_P2(DIP_X_CTL_YUV_EN); // CAM+0018H
        INIT_REG_INFO_ADDR_P2(DIP_X_CTL_YUV2_EN);
        INIT_REG_INFO_ADDR_P2(DIP_X_CTL_RGB_EN); // CAM+0018H
        INIT_REG_INFO_ADDR_P2(DIP_X_CTL_RGB2_EN); // CAM+0018H
        INIT_REG_INFO_VALUE(DIP_X_CTL_YUV_EN,0x00000000);
        INIT_REG_INFO_VALUE(DIP_X_CTL_YUV2_EN,0x00000000);
        INIT_REG_INFO_VALUE(DIP_X_CTL_RGB_EN,0x00000000);
        INIT_REG_INFO_VALUE(DIP_X_CTL_RGB2_EN,0x00000000);
    }

    virtual ~ISP_MGR_CTL_EN_P2() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: // Interfaces.

//    template <class ISP_xxx_T>
//    MyType& get(ISP_xxx_T & rParam);

    template <class ISP_xxx_T>
    MBOOL get(ISP_xxx_T & rParam, const dip_x_reg_t* pReg) const;

    MBOOL apply(const RAWIspCamInfo& rRawIspCamInfo, dip_x_reg_t* pReg);
} ISP_MGR_CTL_EN_P2_T;

#endif

