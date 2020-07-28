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
#ifndef _ISP_MGR_DCE_H_
#define _ISP_MGR_DCE_H_



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DCE
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define INIT_DCE_ADDR(reg)\
    INIT_REG_INFO_ADDR_P2_MULTI(EDCE_D1 ,reg, DCE_D1A_DCE_)


typedef class ISP_MGR_DCE : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_DCE  MyType;
public:
    enum
    {
        EDCE_D1,
        ESubModule_NUM
    };
private:
    MBOOL m_bEnable[ESubModule_NUM];

    enum
    {
        ERegInfo_TC_G1,
        ERegInfo_TC_G2,
        ERegInfo_TC_G3,
        ERegInfo_TC_G4,
        ERegInfo_TC_G5,
        ERegInfo_TC_G6,
        ERegInfo_TC_G7,
        ERegInfo_TC_G8,
        ERegInfo_TC_G9,
        ERegInfo_TC_G10,
        ERegInfo_TC_G11,
        ERegInfo_TC_G12,
        ERegInfo_TC_G13,
        ERegInfo_TC_G14,
        ERegInfo_TC_G15,
        ERegInfo_TC_G16,
        ERegInfo_TC_G17,
        ERegInfo_TC_G18,
        ERegInfo_TC_G19,
        ERegInfo_TC_G20,
        ERegInfo_TC_G21,
        ERegInfo_TC_G22,
        ERegInfo_TC_G23,
        ERegInfo_TC_G24,
        ERegInfo_TC_G25,
        ERegInfo_TC_G26,
        ERegInfo_TC_G27,
        ERegInfo_TC_G28,
        ERegInfo_TC_G29,
        ERegInfo_TC_G30,
        ERegInfo_TC_G31,
        ERegInfo_TC_G32,
        ERegInfo_V_Y_CH_0,
        ERegInfo_V_Y_CH_1,
        ERegInfo_DCE_CONF,
        //ERegInfo_SIZE,
        //ERegInfo_ATPG,
        ERegInfo_NUM
    };

    RegInfo_T m_rIspRegInfo[ESubModule_NUM][ERegInfo_NUM];

protected:
    ISP_MGR_DCE(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, eSensorDev, ESubModule_NUM)
    {
        for(int i=0; i<ESubModule_NUM; i++){
            m_bEnable[i]    = MFALSE;
            ::memset(m_rIspRegInfo[i], 0, sizeof(RegInfo_T)*ERegInfo_NUM);
        }
        // register info addr init
        INIT_DCE_ADDR(TC_G1);
        INIT_DCE_ADDR(TC_G2);
        INIT_DCE_ADDR(TC_G3);
        INIT_DCE_ADDR(TC_G4);
        INIT_DCE_ADDR(TC_G5);
        INIT_DCE_ADDR(TC_G6);
        INIT_DCE_ADDR(TC_G7);
        INIT_DCE_ADDR(TC_G8);
        INIT_DCE_ADDR(TC_G9);
        INIT_DCE_ADDR(TC_G10);
        INIT_DCE_ADDR(TC_G11);
        INIT_DCE_ADDR(TC_G12);
        INIT_DCE_ADDR(TC_G13);
        INIT_DCE_ADDR(TC_G14);
        INIT_DCE_ADDR(TC_G15);
        INIT_DCE_ADDR(TC_G16);
        INIT_DCE_ADDR(TC_G17);
        INIT_DCE_ADDR(TC_G18);
        INIT_DCE_ADDR(TC_G19);
        INIT_DCE_ADDR(TC_G20);
        INIT_DCE_ADDR(TC_G21);
        INIT_DCE_ADDR(TC_G22);
        INIT_DCE_ADDR(TC_G23);
        INIT_DCE_ADDR(TC_G24);
        INIT_DCE_ADDR(TC_G25);
        INIT_DCE_ADDR(TC_G26);
        INIT_DCE_ADDR(TC_G27);
        INIT_DCE_ADDR(TC_G28);
        INIT_DCE_ADDR(TC_G29);
        INIT_DCE_ADDR(TC_G30);
        INIT_DCE_ADDR(TC_G31);
        INIT_DCE_ADDR(TC_G32);
        INIT_DCE_ADDR(V_Y_CH_0);
        INIT_DCE_ADDR(V_Y_CH_1);
        INIT_DCE_ADDR(DCE_CONF);
        //INIT_DCE_ADDR(SIZE);
        //INIT_DCE_ADDR(ATPG);

        for(int i=0; i<ESubModule_NUM; i++){
            reinterpret_cast<DCE_REG_D1A_DCE_V_Y_CH_0*>(REG_INFO_VALUE_PTR_MULTI(i, V_Y_CH_0))->Bits.DCE_CNV_G = 301;
            reinterpret_cast<DCE_REG_D1A_DCE_V_Y_CH_0*>(REG_INFO_VALUE_PTR_MULTI(i, V_Y_CH_0))->Bits.DCE_CNV_R = 153;

            reinterpret_cast<DCE_REG_D1A_DCE_V_Y_CH_1*>(REG_INFO_VALUE_PTR_MULTI(i, V_Y_CH_1))->Bits.DCE_CNV_B = 58;
            reinterpret_cast<DCE_REG_D1A_DCE_V_Y_CH_1*>(REG_INFO_VALUE_PTR_MULTI(i, V_Y_CH_1))->Bits.DCE_Y_OFST = 0;

            reinterpret_cast<DCE_REG_D1A_DCE_DCE_CONF*>(REG_INFO_VALUE_PTR_MULTI(i, DCE_CONF))->Bits.DCE_dce_engine_en = 1;
        }
    }

    virtual ~ISP_MGR_DCE() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: // Interfaces.

    template <class ISP_xxx_T>
    MyType& put(MUINT8 SubModuleIndex, ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(MUINT8 SubModuleIndex, ISP_xxx_T & rParam);

    MBOOL
    isEnable(MUINT8 SubModuleIndex)
    {
        if(SubModuleIndex >= ESubModule_NUM){
            //CAM_LOGE("Error Index: %d", SubModuleIndex);
            return MFALSE;
        }
        return m_bEnable[SubModuleIndex];
    }

    MVOID
    setEnable(MUINT8 SubModuleIndex, MBOOL bEnable)
    {
        m_bEnable[SubModuleIndex] = bEnable;
    }

    MBOOL apply_P2(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, dip_a_reg_t* pReg);

} ISP_MGR_DCE_T;

template <ESensorDev_T const eSensorDev>
class ISP_MGR_DCE_DEV : public ISP_MGR_DCE_T
{
public:
    static
    ISP_MGR_DCE_T&
    getInstance()
    {
        static ISP_MGR_DCE_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_DCE_DEV()
        : ISP_MGR_DCE_T(eSensorDev)
    {}

    virtual ~ISP_MGR_DCE_DEV() {}

};

#endif


