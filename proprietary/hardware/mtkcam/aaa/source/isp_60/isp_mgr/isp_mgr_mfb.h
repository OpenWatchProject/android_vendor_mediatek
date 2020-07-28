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
#ifndef _ISP_MGR_MFB_H_
#define _ISP_MGR_MFB_H_


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  MFB
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define INIT_MFB_ADDR(reg)\
    INIT_REG_INFO_ADDR_MFB_MULTI(EMFB_D1 ,reg, MFB_)

typedef class ISP_MGR_MFB : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_MFB  MyType;
public:
    enum
    {
        EMFB_D1,
        ESubModule_NUM
    };

private:
    MBOOL m_bEnable[ESubModule_NUM];

    enum
    {
        ERegInfo_CON,
        ERegInfo_LL_CON1,
        ERegInfo_LL_CON2,
        //ERegInfo_MFB_LL_CON3,
        //ERegInfo_MFB_LL_CON4,
        //ERegInfo_MFB_EDGE,
        //ERegInfo_MFB_LL_CON5,
        //ERegInfo_MFB_LL_CON6,
        //ERegInfo_MFB_LL_CON7,
        //ERegInfo_MFB_LL_CON8,
        //ERegInfo_MFB_LL_CON9,
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

public:
    ISP_MGR_MFB(MUINT32 const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, eSensorDev, ESubModule_NUM)
    {
        for(int i=0; i<ESubModule_NUM; i++){
            m_bEnable[i] = MFALSE;
            ::memset(m_rIspRegInfo[i], 0, sizeof(RegInfo_T)*ERegInfo_NUM);
        }

        // register info addr init
        INIT_MFB_ADDR(CON);
        INIT_MFB_ADDR(LL_CON1);
        INIT_MFB_ADDR(LL_CON2);
        //INIT_MFB_ADDR(MFB_LL_CON3);
        //INIT_MFB_ADDR(MFB_LL_CON4);
        //INIT_MFB_ADDR(MFB_EDGE);
        //INIT_MFB_ADDR(MFB_LL_CON5);
        //INIT_MFB_ADDR(MFB_LL_CON6);
        //INIT_MFB_ADDR(MFB_LL_CON7);
        //INIT_MFB_ADDR(MFB_LL_CON8);
        //INIT_MFB_ADDR(MFB_LL_CON9);
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

        for(int i=0; i<ESubModule_NUM; i++){
            reinterpret_cast<MFB_REG_CON*>(REG_INFO_VALUE_PTR_MULTI(i, CON))->Bits.MFB_BLD_BYPASS = 0;
            reinterpret_cast<MFB_REG_CON*>(REG_INFO_VALUE_PTR_MULTI(i, CON))->Bits.MFB_BLD_SR_WT_EN = 0;
            reinterpret_cast<MFB_REG_CON*>(REG_INFO_VALUE_PTR_MULTI(i, CON))->Bits.MFB_BLD_MBD_WT_EN = 1;
            reinterpret_cast<MFB_REG_CON*>(REG_INFO_VALUE_PTR_MULTI(i, CON))->Bits.MFB_BLD_OUT_VALID = 0;

            reinterpret_cast<MFB_REG_LL_CON1*>(REG_INFO_VALUE_PTR_MULTI(i, LL_CON1))->Bits.MFB_BLD_LL_FLT_MODE = 0;
            reinterpret_cast<MFB_REG_LL_CON1*>(REG_INFO_VALUE_PTR_MULTI(i, LL_CON1))->Bits.MFB_BLD_LL_FLT_WT_MODE1 = 2;
            reinterpret_cast<MFB_REG_LL_CON1*>(REG_INFO_VALUE_PTR_MULTI(i, LL_CON1))->Bits.MFB_BLD_LL_FLT_WT_MODE2 = 1;
            reinterpret_cast<MFB_REG_LL_CON1*>(REG_INFO_VALUE_PTR_MULTI(i, LL_CON1))->Bits.MFB_BLD_LL_CLIP_TH1 = 50;
            reinterpret_cast<MFB_REG_LL_CON1*>(REG_INFO_VALUE_PTR_MULTI(i, LL_CON1))->Bits.MFB_BLD_LL_CLIP_TH2 = 50;

            reinterpret_cast<MFB_REG_LL_CON2*>(REG_INFO_VALUE_PTR_MULTI(i, LL_CON2))->Bits.MFB_BLD_LL_DT1 = 2;
            reinterpret_cast<MFB_REG_LL_CON2*>(REG_INFO_VALUE_PTR_MULTI(i, LL_CON2))->Bits.MFB_BLD_LL_TH1 = 16;
            reinterpret_cast<MFB_REG_LL_CON2*>(REG_INFO_VALUE_PTR_MULTI(i, LL_CON2))->Bits.MFB_BLD_LL_TH2 = 32;

            reinterpret_cast<MFB_REG_LL_CON10*>(REG_INFO_VALUE_PTR_MULTI(i, LL_CON10))->Bits.MFB_BLD_CONF_MAP_EN = 1;
            reinterpret_cast<MFB_REG_LL_CON10*>(REG_INFO_VALUE_PTR_MULTI(i, LL_CON10))->Bits.MFB_BLD_LL_GRAD_EN = 0;
            reinterpret_cast<MFB_REG_LL_CON10*>(REG_INFO_VALUE_PTR_MULTI(i, LL_CON10))->Bits.MFB_BLD_LL_GRAD_ENTH = 0;
            reinterpret_cast<MFB_REG_LL_CON10*>(REG_INFO_VALUE_PTR_MULTI(i, LL_CON10))->Bits.MFB_BLD_LL_MAX_WT_FMT2 = 32;

            reinterpret_cast<MFB_REG_MBD_CON0*>(REG_INFO_VALUE_PTR_MULTI(i, MBD_CON0))->Bits.MFB_BLD_MBD_MAX_WT = 32;
        }
    }
    virtual ~ISP_MGR_MFB() {}

public:
    static MyType&  getInstance(MUINT32 const eSensorDev);

public: // Interfaces.

    template <class ISP_xxx_T>
    MyType& put(MUINT8 SubModuleIndex, ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(MUINT8 SubModuleIndex, ISP_xxx_T & rParam);

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

    MBOOL apply_MFB(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, mfb_reg_t* pReg);

} ISP_MGR_MFB_T;



#endif



