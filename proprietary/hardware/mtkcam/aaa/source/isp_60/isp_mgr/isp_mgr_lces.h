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
#ifndef _ISP_MGR_LCES_H_
#define _ISP_MGR_LCES_H_

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// LCES
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define INIT_LCES_ADDR(reg)\
    INIT_REG_INFO_ADDR_P1_MULTI(ELCES_R1, reg, LCES_R1_LCES_);\

typedef class ISP_MGR_LCES : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_LCES    MyType;
public:
    enum
    {
        ELCES_R1, //Pass1 Normal
        ESubModule_NUM
    };
private:
    MBOOL m_bEnable[ESubModule_NUM];

    enum
    {
        //ERegInfo_START,
        //ERegInfo_IN_SIZE,
        //ERegInfo_OUT_SIZE,
        ERegInfo_LOG,
        //ERegInfo_LRZRX,
        //ERegInfo_LRZRY,
        ERegInfo_FLC0,
        ERegInfo_FLC1,
        ERegInfo_FLC2,
        ERegInfo_G2G0,
        ERegInfo_G2G1,
        ERegInfo_G2G2,
        ERegInfo_G2G3,
        ERegInfo_G2G4,
        ERegInfo_LPF,
        ERegInfo_AH,
        ERegInfo_NUM
    };

    RegInfo_T m_rIspRegInfo[ESubModule_NUM][ERegInfo_NUM];

public:
    ISP_MGR_LCES(MUINT32 const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, eSensorDev, ESubModule_NUM)
    {
        for(int i=0; i<ESubModule_NUM; i++){
            m_bEnable[i] = MFALSE;
            ::memset(m_rIspRegInfo[i], 0, sizeof(RegInfo_T)*ERegInfo_NUM);
        }
        // register info addr init
        //INIT_LCES_ADDR(START);
        //INIT_LCES_ADDR(IN_SIZE);
        //INIT_LCES_ADDR(OUT_SIZE);
        INIT_LCES_ADDR(LOG);
        //INIT_LCES_ADDR(LRZRX);
        //INIT_LCES_ADDR(LRZRY);
        INIT_LCES_ADDR(FLC0);
        INIT_LCES_ADDR(FLC1);
        INIT_LCES_ADDR(FLC2);
        INIT_LCES_ADDR(G2G0);
        INIT_LCES_ADDR(G2G1);
        INIT_LCES_ADDR(G2G2);
        INIT_LCES_ADDR(G2G3);
        INIT_LCES_ADDR(G2G4);
        INIT_LCES_ADDR(LPF);
        INIT_LCES_ADDR(AH);

        for(int i=0; i<ESubModule_NUM; i++){
            reinterpret_cast<REG_LCES_R1_LCES_LOG*>(REG_INFO_VALUE_PTR_MULTI(i, LOG))->Bits.LCES_LOG       = 1;
            reinterpret_cast<REG_LCES_R1_LCES_LPF*>(REG_INFO_VALUE_PTR_MULTI(i, LPF))->Bits.LCES_LPF_TH    = 300;
            reinterpret_cast<REG_LCES_R1_LCES_LPF*>(REG_INFO_VALUE_PTR_MULTI(i, LPF))->Bits.LCES_LPF_EN    = 1;
            reinterpret_cast<REG_LCES_R1_LCES_AH*>(REG_INFO_VALUE_PTR_MULTI(i, AH))->Bits.LCES_CCUB        = 1600;
            reinterpret_cast<REG_LCES_R1_LCES_AH*>(REG_INFO_VALUE_PTR_MULTI(i, AH))->Bits.LCES_CCLB        = 20;
            reinterpret_cast<REG_LCES_R1_LCES_AH*>(REG_INFO_VALUE_PTR_MULTI(i, AH))->Bits.LCES_ACT_HISTO   = 2;
            reinterpret_cast<REG_LCES_R1_LCES_AH*>(REG_INFO_VALUE_PTR_MULTI(i, AH))->Bits.LCES_HIST_OUT_EN = 0;
        }

    }

    virtual ~ISP_MGR_LCES() {}

public:
    static MyType&  getInstance(MUINT32 const eSensorDev);

public: // Interfaces.

    template <class ISP_xxx_T>
    MyType& put(MUINT8 SubModuleIndex, ISP_xxx_T const& rParam);

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

    MBOOL apply_P1(MUINT8 SubModuleIndex, RAWIspCamInfo& rRawIspCamInfo, TuningMgr& rTuning, MINT32 i4SubsampleIdex=0);

} ISP_MGR_LCES_T;


#endif
