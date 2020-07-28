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
#ifndef _ISP_MGR_FUS_H_
#define _ISP_MGR_FUS_H_

#if 0

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  FUS_R1
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_FUS_R1 : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_FUS_R1    MyType;
private:
    MBOOL m_bEnable;
    MBOOL m_bCCTEnable;
    MUINT32 m_u4StartAddr;

    enum
    {
        ERegInfo_FUS_R1_FUS_RATIO1,
        ERegInfo_FUS_R1_FUS_RATIO2,
        ERegInfo_FUS_R1_FUS_ALIGN,
        ERegInfo_FUS_R1_FUS_CFG,
        ERegInfo_FUS_R1_FUS_CNV_01_00,
        ERegInfo_FUS_R1_FUS_CNV_01_01,
        ERegInfo_FUS_R1_FUS_CNV_01_02,
        ERegInfo_FUS_R1_FUS_CNV_01_10,
        ERegInfo_FUS_R1_FUS_CNV_01_11,
        ERegInfo_FUS_R1_FUS_CNV_01_12,
        ERegInfo_FUS_R1_FUS_CNV_01_20,
        ERegInfo_FUS_R1_FUS_CNV_01_21,
        ERegInfo_FUS_R1_FUS_CNV_01_22,
        ERegInfo_FUS_R1_FUS_CNV_23_00,
        ERegInfo_FUS_R1_FUS_CNV_23_01,
        ERegInfo_FUS_R1_FUS_CNV_23_02,
        ERegInfo_FUS_R1_FUS_CNV_23_10,
        ERegInfo_FUS_R1_FUS_CNV_23_11,
        ERegInfo_FUS_R1_FUS_CNV_23_12,
        ERegInfo_FUS_R1_FUS_CNV_23_20,
        ERegInfo_FUS_R1_FUS_CNV_23_21,
        ERegInfo_FUS_R1_FUS_CNV_23_22,
        ERegInfo_FUS_R1_FUS_SIZE,
        ERegInfo_FUS_R1_FUS_ATPG,
        ERegInfo_FUS_R1_FUS_CFG2,
        ERegInfo_FUS_R1_FUS_TILE,
        ERegInfo_NUM
    };

    RegInfo_T m_rIspRegInfo[ERegInfo_NUM];

protected:
    ISP_MGR_FUS_R1(MUINT32 const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr, eSensorDev)
        , m_bEnable(MTRUE)
        , m_bCCTEnable(MTRUE)
        , m_u4StartAddr(REG_ADDR_P1(FUS_R1_FUS_RATIO1))
    {
        ::memset(m_rIspRegInfo, 0, sizeof(RegInfo_T)*ERegInfo_NUM);
        // register info addr init
        INIT_REG_INFO_ADDR_P1(FUS_R1_FUS_RATIO1);
        INIT_REG_INFO_ADDR_P1(FUS_R1_FUS_RATIO2);
        INIT_REG_INFO_ADDR_P1(FUS_R1_FUS_ALIGN);
        INIT_REG_INFO_ADDR_P1(FUS_R1_FUS_CFG);
        INIT_REG_INFO_ADDR_P1(FUS_R1_FUS_CNV_01_00);
        INIT_REG_INFO_ADDR_P1(FUS_R1_FUS_CNV_01_01);
        INIT_REG_INFO_ADDR_P1(FUS_R1_FUS_CNV_01_02);
        INIT_REG_INFO_ADDR_P1(FUS_R1_FUS_CNV_01_10);
        INIT_REG_INFO_ADDR_P1(FUS_R1_FUS_CNV_01_11);
        INIT_REG_INFO_ADDR_P1(FUS_R1_FUS_CNV_01_12);
        INIT_REG_INFO_ADDR_P1(FUS_R1_FUS_CNV_01_20);
        INIT_REG_INFO_ADDR_P1(FUS_R1_FUS_CNV_01_21);
        INIT_REG_INFO_ADDR_P1(FUS_R1_FUS_CNV_01_22);
        INIT_REG_INFO_ADDR_P1(FUS_R1_FUS_CNV_23_00);
        INIT_REG_INFO_ADDR_P1(FUS_R1_FUS_CNV_23_01);
        INIT_REG_INFO_ADDR_P1(FUS_R1_FUS_CNV_23_02);
        INIT_REG_INFO_ADDR_P1(FUS_R1_FUS_CNV_23_10);
        INIT_REG_INFO_ADDR_P1(FUS_R1_FUS_CNV_23_11);
        INIT_REG_INFO_ADDR_P1(FUS_R1_FUS_CNV_23_12);
        INIT_REG_INFO_ADDR_P1(FUS_R1_FUS_CNV_23_20);
        INIT_REG_INFO_ADDR_P1(FUS_R1_FUS_CNV_23_21);
        INIT_REG_INFO_ADDR_P1(FUS_R1_FUS_CNV_23_22);
        INIT_REG_INFO_ADDR_P1(FUS_R1_FUS_SIZE);
        INIT_REG_INFO_ADDR_P1(FUS_R1_FUS_ATPG);
        INIT_REG_INFO_ADDR_P1(FUS_R1_FUS_CFG2);
        INIT_REG_INFO_ADDR_P1(FUS_R1_FUS_TILE);
    }

    virtual ~ISP_MGR_FUS_R1() {}

public:
    static MyType&  getInstance(MUINT32 const eSensorDev);

public: // Interfaces.

    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

    MBOOL
    isEnable()
    {
        if(SubModuleIndex >= ESubModule_NUM){
            CAM_LOGE("Error Index: %d", SubModuleIndex);
            return MFALSE;
        }
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

    MBOOL apply(RAWIspCamInfo& rRawIspCamInfo, TuningMgr& rTuning, MINT32 i4SubsampleIdex=0);
} ISP_MGR_FUS_R1_T;

template <MUINT32 const eSensorDev>
class ISP_MGR_FUS_R1_DEV : public ISP_MGR_FUS_R1_T
{
public:
    static
    ISP_MGR_FUS_R1_T&
    getInstance()
    {
        static ISP_MGR_FUS_R1_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_FUS_R1_DEV()
        : ISP_MGR_FUS_R1_T(eSensorDev)
    {}

    virtual ~ISP_MGR_FUS_R1_DEV() {}

};
#endif

#endif

