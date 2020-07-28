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
#ifndef _ISP_MGR_C2G_H_
#define _ISP_MGR_C2G_H_

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// C2G
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define INIT_C2G_ADDR(reg)\
    INIT_REG_INFO_ADDR_P2_MULTI(EC2G_D1, reg, C2G_D1A_C2G_);\

typedef class ISP_MGR_C2G : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_C2G    MyType;
public:
    enum
    {
        EC2G_D1,
        ESubModule_NUM
    };
private:
    MBOOL m_bEnable[ESubModule_NUM];
    MBOOL m_bCCTEnable;

    enum
    {
        ERegInfo_CONV_0A,
        ERegInfo_CONV_0B,
        ERegInfo_CONV_1A,
        ERegInfo_CONV_1B,
        ERegInfo_CONV_2A,
        ERegInfo_CONV_2B,
        ERegInfo_NUM
    };

    RegInfo_T m_rIspRegInfo[ESubModule_NUM][ERegInfo_NUM];

protected:
    ISP_MGR_C2G(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, eSensorDev, ESubModule_NUM)
        , m_bCCTEnable(MTRUE)
    {
        for(int i=0; i<ESubModule_NUM; i++){
            m_bEnable[i] = MFALSE;
            ::memset(m_rIspRegInfo[i], 0, sizeof(RegInfo_T)*ERegInfo_NUM);
        }
        INIT_C2G_ADDR(CONV_0A);
        INIT_C2G_ADDR(CONV_0B);
        INIT_C2G_ADDR(CONV_1A);
        INIT_C2G_ADDR(CONV_1B);
        INIT_C2G_ADDR(CONV_2A);
        INIT_C2G_ADDR(CONV_2B);

        for(int i=0; i<ESubModule_NUM; i++){

            reinterpret_cast<C2G_REG_D1A_C2G_CONV_0B*>(REG_INFO_VALUE_PTR_MULTI(i, CONV_0B))->Bits.C2G_Y_OFST = 0;
            reinterpret_cast<C2G_REG_D1A_C2G_CONV_1B*>(REG_INFO_VALUE_PTR_MULTI(i, CONV_1B))->Bits.C2G_U_OFST = 0;
            reinterpret_cast<C2G_REG_D1A_C2G_CONV_2B*>(REG_INFO_VALUE_PTR_MULTI(i, CONV_2B))->Bits.C2G_V_OFST = 0;

            reinterpret_cast<C2G_REG_D1A_C2G_CONV_0A*>(REG_INFO_VALUE_PTR_MULTI(i, CONV_0A))->Bits.C2G_CNV_00 =  512;
            reinterpret_cast<C2G_REG_D1A_C2G_CONV_0A*>(REG_INFO_VALUE_PTR_MULTI(i, CONV_0A))->Bits.C2G_CNV_01 =    0;
            reinterpret_cast<C2G_REG_D1A_C2G_CONV_0B*>(REG_INFO_VALUE_PTR_MULTI(i, CONV_0B))->Bits.C2G_CNV_02 =  718;
            reinterpret_cast<C2G_REG_D1A_C2G_CONV_1A*>(REG_INFO_VALUE_PTR_MULTI(i, CONV_1A))->Bits.C2G_CNV_10 =  512;
            reinterpret_cast<C2G_REG_D1A_C2G_CONV_1A*>(REG_INFO_VALUE_PTR_MULTI(i, CONV_1A))->Bits.C2G_CNV_11 = -176;
            reinterpret_cast<C2G_REG_D1A_C2G_CONV_1B*>(REG_INFO_VALUE_PTR_MULTI(i, CONV_1B))->Bits.C2G_CNV_12 = -366;
            reinterpret_cast<C2G_REG_D1A_C2G_CONV_2A*>(REG_INFO_VALUE_PTR_MULTI(i, CONV_2A))->Bits.C2G_CNV_20 =  512;
            reinterpret_cast<C2G_REG_D1A_C2G_CONV_2A*>(REG_INFO_VALUE_PTR_MULTI(i, CONV_2A))->Bits.C2G_CNV_21 =  907;
            reinterpret_cast<C2G_REG_D1A_C2G_CONV_2B*>(REG_INFO_VALUE_PTR_MULTI(i, CONV_2B))->Bits.C2G_CNV_22 =    0;
        }
    }

    virtual ~ISP_MGR_C2G() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: // Interfaces.

    //template <class ISP_xxx_T>
    //MyType& put(MUINT8 SubModuleIndex, ISP_xxx_T const& rParam);

    //template <class ISP_xxx_T>
    //MyType& get(MUINT8 SubModuleIndex, ISP_xxx_T & rParam);

    MBOOL
    isEnable(MUINT8 SubModuleIndex)
    {
        if(SubModuleIndex >= ESubModule_NUM){
            return MFALSE;
        }
        return m_bEnable[SubModuleIndex];
    }

    MVOID
    setEnable(MUINT8 SubModuleIndex, MBOOL bEnable)
    {
        m_bEnable[SubModuleIndex] = bEnable;
    }
#if 0
    MBOOL
    isCCTEnable(MUINT8 SubModuleIndex)
    {
        return m_bCCTEnable[SubModuleIndex];
    }

    MVOID
    setCCTEnable(MUINT8 SubModuleIndex, MBOOL bEnable)
    {
        m_bCCTEnable[SubModuleIndex] = bEnable;
    }
#endif
    MBOOL apply_P2(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, dip_a_reg_t* pReg);

} ISP_MGR_C2G_T;

template <ESensorDev_T const eSensorDev>
class ISP_MGR_C2G_DEV : public ISP_MGR_C2G_T
{
public:
    static
    ISP_MGR_C2G_T&
    getInstance()
    {
        static ISP_MGR_C2G_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_C2G_DEV()
        : ISP_MGR_C2G_T(eSensorDev)
    {}

    virtual ~ISP_MGR_C2G_DEV() {}

};
#endif