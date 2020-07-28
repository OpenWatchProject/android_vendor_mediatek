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
#ifndef _ISP_MGR_DCES_H_
#define _ISP_MGR_DCES_H_

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// DCES
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define INIT_DCES_ADDR(reg)\
    INIT_REG_INFO_ADDR_P2_MULTI(EDCES_D1 ,reg, DCES_D1A_DCES_)
typedef class ISP_MGR_DCES : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_DCES    MyType;
public:
    enum
    {
        EDCES_D1, //Pass1 Normal
        ESubModule_NUM
    };
private:
    MBOOL m_bEnable[ESubModule_NUM];
    enum
    {
        ERegInfo_DC_CTRL,
        //ERegInfo_CROP_X,
        //ERegInfo_CROP_Y,
        ERegInfo_V_Y_CH_0,
        ERegInfo_V_Y_CH_1,
        //ERegInfo_SIZE,
        //ERegInfo_ATPG,
        ERegInfo_NUM
    };

    RegInfo_T m_rIspRegInfo[ESubModule_NUM][ERegInfo_NUM];

protected:
    ISP_MGR_DCES(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, eSensorDev, ESubModule_NUM)
    {
        for(int i=0; i<ESubModule_NUM; i++){
            m_bEnable[i] = MFALSE;
            ::memset(m_rIspRegInfo[i], 0, sizeof(RegInfo_T)*ERegInfo_NUM);
        }
        // register info addr init
        INIT_DCES_ADDR(DC_CTRL);
        //INIT_DCES_ADDR(CROP_X);
        //INIT_DCES_ADDR(CROP_Y);
        INIT_DCES_ADDR(V_Y_CH_0);
        INIT_DCES_ADDR(V_Y_CH_1);
        //INIT_DCES_ADDR(SIZE);
        //INIT_DCES_ADDR(ATPG);
        for(int i=0; i<ESubModule_NUM; i++){
            reinterpret_cast<DCES_REG_D1A_DCES_DC_CTRL*>(REG_INFO_VALUE_PTR_MULTI(i, DC_CTRL))->Bits.DCES_DC_MODE = 0;
            reinterpret_cast<DCES_REG_D1A_DCES_DC_CTRL*>(REG_INFO_VALUE_PTR_MULTI(i, DC_CTRL))->Bits.DCES_DC_SLOPE = 8;

            reinterpret_cast<DCES_REG_D1A_DCES_V_Y_CH_0*>(REG_INFO_VALUE_PTR_MULTI(i, V_Y_CH_0))->Bits.DCES_CNV_R = 153;
            reinterpret_cast<DCES_REG_D1A_DCES_V_Y_CH_0*>(REG_INFO_VALUE_PTR_MULTI(i, V_Y_CH_0))->Bits.DCES_CNV_G = 301;

            reinterpret_cast<DCES_REG_D1A_DCES_V_Y_CH_1*>(REG_INFO_VALUE_PTR_MULTI(i, V_Y_CH_1))->Bits.DCES_CNV_B = 58;
            reinterpret_cast<DCES_REG_D1A_DCES_V_Y_CH_1*>(REG_INFO_VALUE_PTR_MULTI(i, V_Y_CH_1))->Bits.DCES_Y_OFST = 0;

        }
    }

    virtual ~ISP_MGR_DCES() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);
public: // Interfaces.
#if 0
    template <class ISP_xxx_T>
    MyType& put(MUINT8 SubModuleIndex, ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(MUINT8 SubModuleIndex, ISP_xxx_T & rParam);
#endif
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

    MVOID
    setLastDCE_Result(MUINT8 SubModuleIndex, DCE2DCES_INFO_T DCES_Info)
    {
        reinterpret_cast<DCES_REG_D1A_DCES_V_Y_CH_1*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, V_Y_CH_1))->Bits.DCES_VY_BLD_WT = DCES_Info.YV_BLD_WT;
        reinterpret_cast<DCES_REG_D1A_DCES_DC_CTRL*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, DC_CTRL))->Bits.DCES_DC_TH = DCES_Info.DC_TH;
    }

    MBOOL apply_P2(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, dip_a_reg_t* pReg);
} ISP_MGR_DCES_T;

template <ESensorDev_T const eSensorDev>
class ISP_MGR_DCES_DEV : public ISP_MGR_DCES_T
{
public:
    static
    ISP_MGR_DCES_T&
    getInstance()
    {
        static ISP_MGR_DCES_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_DCES_DEV()
        : ISP_MGR_DCES_T(eSensorDev)
    {}

    virtual ~ISP_MGR_DCES_DEV() {}
};
#endif
