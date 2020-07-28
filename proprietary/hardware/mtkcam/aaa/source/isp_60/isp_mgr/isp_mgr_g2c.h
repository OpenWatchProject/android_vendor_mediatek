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
#ifndef _ISP_MGR_G2C_H_
#define _ISP_MGR_G2C_H_


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  G2C
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define INIT_G2C_ADDR(reg)\
    INIT_REG_INFO_ADDR_P1_MULTI(EG2C_R1 ,reg, G2C_R1_G2C_);\
    INIT_REG_INFO_ADDR_P1_MULTI(EG2C_R2 ,reg, G2C_R2_G2C_);\
    INIT_REG_INFO_ADDR_P2_MULTI(EG2C_D1 ,reg, G2C_D1A_G2C_)


typedef class ISP_MGR_G2C : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_G2C  MyType;
public:

    enum
    {
        EG2C_R1, //Pass1
        EG2C_R2, //Pass1
        EG2C_D1, //Pass2
        ESubModule_NUM
    };

private:

    MBOOL m_bEnable[ESubModule_NUM];

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

public:
    ISP_MGR_G2C(MUINT32 const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, eSensorDev, ESubModule_NUM)
    {
        for(int i=0; i<ESubModule_NUM; i++){
            m_bEnable[i]    = MFALSE;
            ::memset(m_rIspRegInfo[i], 0, sizeof(RegInfo_T)*ERegInfo_NUM);
        }
        // register info addr init
        INIT_G2C_ADDR(CONV_0A);
        INIT_G2C_ADDR(CONV_0B);
        INIT_G2C_ADDR(CONV_1A);
        INIT_G2C_ADDR(CONV_1B);
        INIT_G2C_ADDR(CONV_2A);
        INIT_G2C_ADDR(CONV_2B);

        for(int i=0; i<ESubModule_NUM; i++){

            reinterpret_cast<G2C_REG_D1A_G2C_CONV_0B*>(REG_INFO_VALUE_PTR_MULTI(i, CONV_0B))->Bits.G2C_Y_OFST = 0;
            reinterpret_cast<G2C_REG_D1A_G2C_CONV_1B*>(REG_INFO_VALUE_PTR_MULTI(i, CONV_1B))->Bits.G2C_U_OFST = 0;
            reinterpret_cast<G2C_REG_D1A_G2C_CONV_2B*>(REG_INFO_VALUE_PTR_MULTI(i, CONV_2B))->Bits.G2C_V_OFST = 0;

            reinterpret_cast<G2C_REG_D1A_G2C_CONV_0A*>(REG_INFO_VALUE_PTR_MULTI(i, CONV_0A))->Bits.G2C_CNV_00 =  153;
            reinterpret_cast<G2C_REG_D1A_G2C_CONV_0A*>(REG_INFO_VALUE_PTR_MULTI(i, CONV_0A))->Bits.G2C_CNV_01 =  301;
            reinterpret_cast<G2C_REG_D1A_G2C_CONV_0B*>(REG_INFO_VALUE_PTR_MULTI(i, CONV_0B))->Bits.G2C_CNV_02 =   58;
            reinterpret_cast<G2C_REG_D1A_G2C_CONV_1A*>(REG_INFO_VALUE_PTR_MULTI(i, CONV_1A))->Bits.G2C_CNV_10 =  -86;
            reinterpret_cast<G2C_REG_D1A_G2C_CONV_1A*>(REG_INFO_VALUE_PTR_MULTI(i, CONV_1A))->Bits.G2C_CNV_11 = -170;
            reinterpret_cast<G2C_REG_D1A_G2C_CONV_1B*>(REG_INFO_VALUE_PTR_MULTI(i, CONV_1B))->Bits.G2C_CNV_12 =  256;
            reinterpret_cast<G2C_REG_D1A_G2C_CONV_2A*>(REG_INFO_VALUE_PTR_MULTI(i, CONV_2A))->Bits.G2C_CNV_20 =  256;
            reinterpret_cast<G2C_REG_D1A_G2C_CONV_2A*>(REG_INFO_VALUE_PTR_MULTI(i, CONV_2A))->Bits.G2C_CNV_21 = -214;
            reinterpret_cast<G2C_REG_D1A_G2C_CONV_2B*>(REG_INFO_VALUE_PTR_MULTI(i, CONV_2B))->Bits.G2C_CNV_22 =  -42;
        }
    }

    virtual ~ISP_MGR_G2C() {}

public:
    static MyType&  getInstance(MUINT32 const eSensorDev);

public: // Interfaces.

    //template <class ISP_xxx_T>
    //MyType& put(MUINT8 SubModuleIndex, ISP_xxx_T const& rParam);

    //template <class ISP_xxx_T>
    //MyType& get(MUINT8 SubModuleIndex, ISP_xxx_T & rParam);

    MBOOL
    isEnable(MUINT8 SubModuleIndex)
    {
        if(SubModuleIndex >= ESubModule_NUM){
            CAM_LOGE("Error Index: %d", SubModuleIndex);
            return MFALSE;
        }
        return m_bEnable[SubModuleIndex];
    }

    MVOID
    setEnable(MUINT8 SubModuleIndex, MBOOL bEnable)
    {
        m_bEnable[SubModuleIndex] = bEnable;
    }

    MBOOL apply_P1(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, TuningMgr& rTuning, MINT32 i4SubsampleIdex=0);

    MBOOL apply_P2(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, dip_x_reg_t* pReg);

} ISP_MGR_G2C_T;



#endif

