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
#ifndef _ISP_MGR_GGM_H_
#define _ISP_MGR_GGM_H_

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// GGM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define INIT_GGM_ADDR(reg)\
    INIT_REG_INFO_ADDR_P1_MULTI(EGGM_R1, reg, GGM_R1_GGM_);\
    INIT_REG_INFO_ADDR_P1_MULTI(EGGM_R2, reg, GGM_R2_GGM_);\
    INIT_REG_INFO_ADDR_P1_MULTI(EGGM_R3, reg, GGM_R3_GGM_);\
    INIT_REG_INFO_ADDR_P2_MULTI(EGGM_D1, reg, GGM_D1A_GGM_);\
    INIT_REG_INFO_ADDR_P2_MULTI(EGGM_D2, reg, GGM_D2A_GGM_);\
    INIT_REG_INFO_ADDR_P2_MULTI(EGGM_D3, reg, GGM_D3A_GGM_);\
    //INIT_REG_INFO_ADDR_P2_MULTI(EGGM_D4, reg, GGM_D4A_GGM_);\

typedef class ISP_MGR_GGM : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_GGM    MyType;
public:
    enum
    {
        EGGM_R1,
        EGGM_R2,
        EGGM_R3,
        EGGM_D1,
        EGGM_D2,
        EGGM_D3,
        EGGM_D4,
        ESubModule_NUM
    };
private:
    MBOOL m_bEnable[ESubModule_NUM];
    MBOOL m_bCCTEnable;

    enum
    {
        ERegInfo_CTRL,
        ERegInfo_NUM
    };

    RegInfo_T m_rIspRegInfo[ESubModule_NUM][(ERegInfo_NUM + GGM_LUT_SIZE)];

protected:
    ISP_MGR_GGM(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, (ERegInfo_NUM + GGM_LUT_SIZE), eSensorDev, ESubModule_NUM)
        , m_bCCTEnable(MTRUE)
    {
        for(int i=0; i<ESubModule_NUM; i++){
            m_bEnable[i] = MFALSE;
            ::memset(m_rIspRegInfo[i], 0, sizeof(RegInfo_T)*(ERegInfo_NUM + GGM_LUT_SIZE));
        }
        INIT_GGM_ADDR(CTRL);
        MUINT32 u4StartAddr[ESubModule_NUM];
        u4StartAddr[EGGM_R1]= REG_ADDR_P1(GGM_R1_GGM_LUT[0]);
        u4StartAddr[EGGM_R2]= REG_ADDR_P1(GGM_R2_GGM_LUT[0]);
        u4StartAddr[EGGM_R3]= REG_ADDR_P1(GGM_R3_GGM_LUT[0]);
        u4StartAddr[EGGM_D1]= REG_ADDR_P2(GGM_D1A_GGM_LUT[0]);
        u4StartAddr[EGGM_D2]= REG_ADDR_P2(GGM_D2A_GGM_LUT[0]);
        u4StartAddr[EGGM_D3]= REG_ADDR_P2(GGM_D3A_GGM_LUT[0]);

        for(int i=0; i<ESubModule_NUM; i++){
            reinterpret_cast<REG_GGM_R1_GGM_CTRL*>(REG_INFO_VALUE_PTR_MULTI(i, CTRL))->Bits.GGM_END_VAR = 1023;
            reinterpret_cast<REG_GGM_R1_GGM_CTRL*>(REG_INFO_VALUE_PTR_MULTI(i, CTRL))->Bits.GGM_RMP_VAR = 15;
            for (MINT32 j = 0; j < GGM_LUT_SIZE; j++) {
                m_rIspRegInfo[i][ERegInfo_NUM + j].val = 0 ;
                m_rIspRegInfo[i][ERegInfo_NUM + j].addr = u4StartAddr[i] + 4*j;
            }
        }
    }

    virtual ~ISP_MGR_GGM() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: // Interfaces.

    template <class ISP_xxx_T>
    MyType& put(MUINT8 SubModuleIndex, ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(MUINT8 SubModuleIndex, ISP_xxx_T & rParam);

    MUINT32
    getEndVar(MUINT8 SubModuleIndex)
    {
        return reinterpret_cast<REG_GGM_R1_GGM_CTRL*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, CTRL))->Bits.GGM_END_VAR ;
    }

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

    MBOOL apply_P1(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, TuningMgr& rTuning, MINT32 i4SubsampleIdex=0);

    MBOOL apply_P2(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, dip_a_reg_t* pReg);

} ISP_MGR_GGM_T;

template <ESensorDev_T const eSensorDev>
class ISP_MGR_GGM_DEV : public ISP_MGR_GGM_T
{
public:
    static
    ISP_MGR_GGM_T&
    getInstance()
    {
        static ISP_MGR_GGM_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_GGM_DEV()
        : ISP_MGR_GGM_T(eSensorDev)
    {}

    virtual ~ISP_MGR_GGM_DEV() {}

};
#endif