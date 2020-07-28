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
#ifndef _ISP_MGR_MFB_H_
#define _ISP_MGR_MFB_H_

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// MFB
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define INIT_MFB_ADDR(reg)\
    INIT_REG_INFO_ADDR_MFB_MULTI(EMFB_D1, reg, MFB_D1A_MFB_);\

typedef class ISP_MGR_MFB : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_MFB    MyType;
public:
    enum
    {
        EMFB_D1,
        ESubModule_NUM
    };
private:
    MBOOL m_bEnable[ESubModule_NUM];
    MBOOL m_bCCTEnable;

    enum
    {
        ERegInfo_CON,
        ERegInfo_LL_CON1,
        ERegInfo_LL_CON10,
        ERegInfo_MBD_CON0,
        ERegInfo_MBD_CON1,
        ERegInfo_MBD_CON2,
        ERegInfo_MBD_CON3,
        ERegInfo_MBD_CON4,
        ERegInfo_MBD_CON5,
        ERegInfo_MBD_CON6,
        ERegInfo_MBD_CON7,
        ERegInfo_MBD_CON8,
        ERegInfo_MBD_CON9,
        ERegInfo_MBD_CON10,
        ERegInfo_NUM
    };

    RegInfo_T m_rIspRegInfo[ESubModule_NUM][ERegInfo_NUM];

protected:
    ISP_MGR_MFB(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, eSensorDev, ESubModule_NUM)
        , m_bCCTEnable(MTRUE)
    {
        for(int i=0; i<ESubModule_NUM; i++){
            m_bEnable[i] = MFALSE;
            ::memset(m_rIspRegInfo[i], 0, sizeof(RegInfo_T)*ERegInfo_NUM);
        }
        INIT_MFB_ADDR(CON);
        INIT_MFB_ADDR(LL_CON1);
        INIT_MFB_ADDR(LL_CON10);
        INIT_MFB_ADDR(MBD_CON0);
        INIT_MFB_ADDR(MBD_CON1);
        INIT_MFB_ADDR(MBD_CON2);
        INIT_MFB_ADDR(MBD_CON3);
        INIT_MFB_ADDR(MBD_CON4);
        INIT_MFB_ADDR(MBD_CON5);
        INIT_MFB_ADDR(MBD_CON6);
        INIT_MFB_ADDR(MBD_CON7);
        INIT_MFB_ADDR(MBD_CON8);
        INIT_MFB_ADDR(MBD_CON9);
        INIT_MFB_ADDR(MBD_CON10);
    }

    virtual ~ISP_MGR_MFB() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public:

    template <class ISP_xxx_T>
    MyType& put(MUINT8 SubModuleIndex, ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(MUINT8 SubModuleIndex, ISP_xxx_T & rParam);

    MBOOL
    isEnable(MUINT8 SubModuleIndex)
    {
        return m_bEnable[SubModuleIndex];
    }

    MBOOL
    isCCTEnable()
    {
        return m_bCCTEnable;
    }

    MVOID
    setEnable(MUINT8 SubModuleIndex, MBOOL bEnable)
    {
        m_bEnable[SubModuleIndex] = bEnable;
    }

    MVOID
    setCCTEnable(MBOOL bEnable)
    {
        m_bCCTEnable = bEnable;
    }

    MBOOL apply_MFB(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, mfb_reg_t* pReg);

} ISP_MGR_MFB_T;

template <ESensorDev_T const eSensorDev>
class ISP_MGR_MFB_DEV : public ISP_MGR_MFB_T
{
public:
    static
    ISP_MGR_MFB_T&
    getInstance()
    {
        static ISP_MGR_MFB_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_MFB_DEV()
        : ISP_MGR_MFB_T(eSensorDev)
    {}

    virtual ~ISP_MGR_MFB_DEV() {}

};
#endif