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
#ifndef _ISP_MGR_CPN_H_
#define _ISP_MGR_CPN_H_

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CPN
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_CPN : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_CPN    MyType;
private:
    MBOOL m_bEnable;
    MINT32 debugDump;
    MBOOL m_bCCTEnable;
    MUINT32 m_u4StartAddr;

    /*ZVHDR*/
    MBOOL m_bZHdrEnable;

    // TG1
    enum
    {
#if 1
        ERegInfo_CAM_CPN_HDR_CTL_EN,
        ERegInfo_CAM_CPN_IN_IMG_SIZE,
        ERegInfo_CAM_CPN_ALGO_PARAM1,
        ERegInfo_CAM_CPN_ALGO_PARAM2,
        ERegInfo_CAM_CPN_GTM_X0X1,
        ERegInfo_CAM_CPN_GTM_X2X3,
        ERegInfo_CAM_CPN_GTM_X4X5,
        ERegInfo_CAM_CPN_GTM_X6,
        ERegInfo_CAM_CPN_GTM_Y0Y1,
        ERegInfo_CAM_CPN_GTM_Y2Y3,
        ERegInfo_CAM_CPN_GTM_Y4Y5,
        ERegInfo_CAM_CPN_GTM_Y6,
        ERegInfo_CAM_CPN_GTM_S0S1,
        ERegInfo_CAM_CPN_GTM_S2S3,
        ERegInfo_CAM_CPN_GTM_S4S5,
        ERegInfo_CAM_CPN_GTM_S6S7,
#endif
        ERegInfo_NUM
    };

    MUINT32 m_rRegInput[ERegInfo_NUM];
    RegInfo_T m_rIspRegInfo[ERegInfo_NUM];

public:
    ISP_MGR_CPN(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr, eSensorDev)
        , m_bEnable(MFALSE)
        , debugDump(0)
        , m_bCCTEnable(MTRUE)
        , m_u4StartAddr(REG_ADDR_P1(CAM_CPN_HDR_CTL_EN))
        , m_bZHdrEnable(MFALSE)
    {
#if 1
        // register info addr init
        INIT_REG_INFO_ADDR_P1(CAM_CPN_HDR_CTL_EN);
        INIT_REG_INFO_ADDR_P1(CAM_CPN_IN_IMG_SIZE);
        INIT_REG_INFO_ADDR_P1(CAM_CPN_ALGO_PARAM1);
        INIT_REG_INFO_ADDR_P1(CAM_CPN_ALGO_PARAM2);
        INIT_REG_INFO_ADDR_P1(CAM_CPN_GTM_X0X1);
        INIT_REG_INFO_ADDR_P1(CAM_CPN_GTM_X2X3);
        INIT_REG_INFO_ADDR_P1(CAM_CPN_GTM_X4X5);
        INIT_REG_INFO_ADDR_P1(CAM_CPN_GTM_X6);
        INIT_REG_INFO_ADDR_P1(CAM_CPN_GTM_Y0Y1);
        INIT_REG_INFO_ADDR_P1(CAM_CPN_GTM_Y2Y3);
        INIT_REG_INFO_ADDR_P1(CAM_CPN_GTM_Y4Y5);
        INIT_REG_INFO_ADDR_P1(CAM_CPN_GTM_Y6);
        INIT_REG_INFO_ADDR_P1(CAM_CPN_GTM_S0S1);
        INIT_REG_INFO_ADDR_P1(CAM_CPN_GTM_S2S3);
        INIT_REG_INFO_ADDR_P1(CAM_CPN_GTM_S4S5);
        INIT_REG_INFO_ADDR_P1(CAM_CPN_GTM_S6S7);
#endif
    }

    virtual ~ISP_MGR_CPN() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: // Interfaces.

    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

    /*==== Set debugDump ======*/
    MVOID
    setDebugDump(MINT32 debug)
    {
        debugDump = debug;
    }

    /*==== Set CPN enable ===== */
    MBOOL
    isEnable()
    {
        return m_bEnable;
    }

    MVOID
    setEnable(MBOOL bEnable)
    {
        m_bEnable = bEnable;
    }

    MBOOL
    isCCTEnable()
    {
        return m_bCCTEnable;
    }

    MVOID
    setCCTEnable(MBOOL bEnable)
    {
        m_bCCTEnable = bEnable;
    }

    /*==== Set ZVHDR enable ======*/
    MBOOL
    isZHdrEnable()
    {
        return m_bZHdrEnable;
    }

    MVOID
    setZHdrEnable(MBOOL bEnable)
    {
        m_bZHdrEnable = bEnable;
    }

    MBOOL apply(RAWIspCamInfo& rRawIspCamInfo, TuningMgr& rTuning, MINT32 i4SubsampleIdex=0);

} ISP_MGR_CPN_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CPN2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_CPN2 : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_CPN2    MyType;
private:
    MBOOL m_bEnable;
    MINT32 debugDump;
    MBOOL m_bCCTEnable;
    MUINT32 m_u4StartAddr;

    /*ZVHDR*/
    MBOOL m_bZHdrEnable;

    // TG1
    enum
    {
#if 1
        ERegInfo_DIP_X_CPN_HDR_CTL_EN,
        //ERegInfo_DIP_X_CPN_IN_IMG_SIZE,
        ERegInfo_DIP_X_CPN_ALGO_PARAM1,
        ERegInfo_DIP_X_CPN_ALGO_PARAM2,
        ERegInfo_DIP_X_CPN_GTM_X0X1,
        ERegInfo_DIP_X_CPN_GTM_X2X3,
        ERegInfo_DIP_X_CPN_GTM_X4X5,
        ERegInfo_DIP_X_CPN_GTM_X6,
        ERegInfo_DIP_X_CPN_GTM_Y0Y1,
        ERegInfo_DIP_X_CPN_GTM_Y2Y3,
        ERegInfo_DIP_X_CPN_GTM_Y4Y5,
        ERegInfo_DIP_X_CPN_GTM_Y6,
        ERegInfo_DIP_X_CPN_GTM_S0S1,
        ERegInfo_DIP_X_CPN_GTM_S2S3,
        ERegInfo_DIP_X_CPN_GTM_S4S5,
        ERegInfo_DIP_X_CPN_GTM_S6S7,
#endif
        ERegInfo_NUM
    };

    MUINT32 m_rRegInput[ERegInfo_NUM];
    RegInfo_T m_rIspRegInfo[ERegInfo_NUM];

public:
    ISP_MGR_CPN2(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr, eSensorDev)
        , m_bEnable(MFALSE)
        , debugDump(0)
        , m_bCCTEnable(MTRUE)
        , m_u4StartAddr(REG_ADDR_P2(DIP_X_CPN_HDR_CTL_EN))
        , m_bZHdrEnable(MFALSE)
    {
#if 1
        // register info addr init
        INIT_REG_INFO_ADDR_P2(DIP_X_CPN_HDR_CTL_EN);
        //INIT_REG_INFO_ADDR_P2(DIP_X_CPN_IN_IMG_SIZE);
        INIT_REG_INFO_ADDR_P2(DIP_X_CPN_ALGO_PARAM1);
        INIT_REG_INFO_ADDR_P2(DIP_X_CPN_ALGO_PARAM2);
        INIT_REG_INFO_ADDR_P2(DIP_X_CPN_GTM_X0X1);
        INIT_REG_INFO_ADDR_P2(DIP_X_CPN_GTM_X2X3);
        INIT_REG_INFO_ADDR_P2(DIP_X_CPN_GTM_X4X5);
        INIT_REG_INFO_ADDR_P2(DIP_X_CPN_GTM_X6);
        INIT_REG_INFO_ADDR_P2(DIP_X_CPN_GTM_Y0Y1);
        INIT_REG_INFO_ADDR_P2(DIP_X_CPN_GTM_Y2Y3);
        INIT_REG_INFO_ADDR_P2(DIP_X_CPN_GTM_Y4Y5);
        INIT_REG_INFO_ADDR_P2(DIP_X_CPN_GTM_Y6);
        INIT_REG_INFO_ADDR_P2(DIP_X_CPN_GTM_S0S1);
        INIT_REG_INFO_ADDR_P2(DIP_X_CPN_GTM_S2S3);
        INIT_REG_INFO_ADDR_P2(DIP_X_CPN_GTM_S4S5);
        INIT_REG_INFO_ADDR_P2(DIP_X_CPN_GTM_S6S7);
#endif
    }

    virtual ~ISP_MGR_CPN2() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: // Interfaces.

    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MBOOL get(ISP_xxx_T & rParam, const dip_x_reg_t* pReg) const;

    /*==== Set debugDump ======*/
    MVOID
    setDebugDump(MINT32 debug)
    {
        debugDump = debug;
    }

    /*==== Set CPN2 enable ===== */
    MBOOL
    isEnable()
    {
        return m_bEnable;
    }

    MVOID
    setEnable(MBOOL bEnable)
    {
        m_bEnable = bEnable;
    }

    MBOOL
    isCCTEnable()
    {
        return m_bCCTEnable;
    }

    MVOID
    setCCTEnable(MBOOL bEnable)
    {
        m_bCCTEnable = bEnable;
    }

    /*==== Set ZVHDR enable ======*/
    MBOOL
    isZHdrEnable()
    {
        return m_bZHdrEnable;
    }

    MVOID
    setZHdrEnable(MBOOL bEnable)
    {
        m_bZHdrEnable = bEnable;
    }

    MBOOL apply(const RAWIspCamInfo& rRawIspCamInfo, dip_x_reg_t* pReg);


} ISP_MGR_CPN2_T;

#endif

