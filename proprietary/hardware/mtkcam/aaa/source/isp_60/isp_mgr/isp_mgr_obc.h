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
#ifndef _ISP_MGR_OBC_H_
#define _ISP_MGR_OBC_H_

#include <mtkcam/utils/std/Log.h>

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  OBC
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define INIT_OBC_ADDR(reg)\
    INIT_REG_INFO_ADDR_P1_MULTI(EOBC_R1 ,reg, OBC_R1_OBC_);\
    INIT_REG_INFO_ADDR_P2_MULTI(EOBC_D1 ,reg, OBC_D1A_OBC_)
//INIT_REG_INFO_ADDR_P1_MULTI(EOBC_R2 ,reg, OBC_R2_OBC_)


typedef class ISP_MGR_OBC : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_OBC  MyType;

public:

    enum
    {
        EOBC_R1, //Pass1 Normal
        //EOBC_R2, //Pass1 for HDR Fus
        EOBC_D1, //Pass2
        ESubModule_NUM
    };

private:

    MBOOL m_bEnable[ESubModule_NUM];
    MBOOL m_bCCTEnable; //only support R1, D1

    enum
    {
        //ERegInfo_CTL,
        ERegInfo_DBS,
        ERegInfo_GRAY_BLD_0,
        ERegInfo_GRAY_BLD_1,
        ERegInfo_GRAY_BLD_2,
        ERegInfo_BIAS_LUT_R0,
        ERegInfo_BIAS_LUT_R1,
        ERegInfo_BIAS_LUT_R2,
        ERegInfo_BIAS_LUT_R3,
        ERegInfo_BIAS_LUT_G0,
        ERegInfo_BIAS_LUT_G1,
        ERegInfo_BIAS_LUT_G2,
        ERegInfo_BIAS_LUT_G3,
        ERegInfo_BIAS_LUT_B0,
        ERegInfo_BIAS_LUT_B1,
        ERegInfo_BIAS_LUT_B2,
        ERegInfo_BIAS_LUT_B3,
        ERegInfo_WBG_RB,
        ERegInfo_WBG_G,
        ERegInfo_WBIG_RB,
        ERegInfo_WBIG_G,
        ERegInfo_OBG_RB,
        ERegInfo_OBG_G,
        ERegInfo_OFFSET_R,
        ERegInfo_OFFSET_GR,
        ERegInfo_OFFSET_GB,
        ERegInfo_OFFSET_B,
        ERegInfo_HDR,
        ERegInfo_NUM
    };

    RegInfo_T m_rIspRegInfo[ESubModule_NUM][ERegInfo_NUM];

public:
    ISP_MGR_OBC(MUINT32 const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, eSensorDev, ESubModule_NUM)
        , m_bCCTEnable(MTRUE)
    {
        for(int i=0; i<ESubModule_NUM; i++){
            m_bEnable[i]    = MFALSE;
            ::memset(m_rIspRegInfo[i], 0, sizeof(RegInfo_T)*ERegInfo_NUM);
        }
        // register info addr init
        //INIT_OBC_ADDR(CTL);
        INIT_OBC_ADDR(DBS);
        INIT_OBC_ADDR(GRAY_BLD_0);
        INIT_OBC_ADDR(GRAY_BLD_1);
        INIT_OBC_ADDR(GRAY_BLD_2);
        INIT_OBC_ADDR(BIAS_LUT_R0);
        INIT_OBC_ADDR(BIAS_LUT_R1);
        INIT_OBC_ADDR(BIAS_LUT_R2);
        INIT_OBC_ADDR(BIAS_LUT_R3);
        INIT_OBC_ADDR(BIAS_LUT_G0);
        INIT_OBC_ADDR(BIAS_LUT_G1);
        INIT_OBC_ADDR(BIAS_LUT_G2);
        INIT_OBC_ADDR(BIAS_LUT_G3);
        INIT_OBC_ADDR(BIAS_LUT_B0);
        INIT_OBC_ADDR(BIAS_LUT_B1);
        INIT_OBC_ADDR(BIAS_LUT_B2);
        INIT_OBC_ADDR(BIAS_LUT_B3);
        INIT_OBC_ADDR(WBG_RB);
        INIT_OBC_ADDR(WBG_G);
        INIT_OBC_ADDR(WBIG_RB);
        INIT_OBC_ADDR(WBIG_G);
        INIT_OBC_ADDR(OBG_RB);
        INIT_OBC_ADDR(OBG_G);
        INIT_OBC_ADDR(OFFSET_R);
        INIT_OBC_ADDR(OFFSET_GR);
        INIT_OBC_ADDR(OFFSET_GB);
        INIT_OBC_ADDR(OFFSET_B);
        INIT_OBC_ADDR(HDR);
    }

    virtual ~ISP_MGR_OBC() {}

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

    MINT32 GetHDRGNP(MINT32 Ratio)
    {
        if(Ratio > 11)
             return 4;
        else if (Ratio > 5)
             return 3;
        else if(Ratio > 2)
             return 2;
        else if(Ratio > 1)
             return 1;
        else
             return 0;
    }

    MBOOL apply_P1(MUINT8 SubModuleIndex, RAWIspCamInfo& rRawIspCamInfo, TuningMgr& rTuning, MINT32 i4SubsampleIdex=0);

    MBOOL apply_P2(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, dip_x_reg_t* pReg);

} ISP_MGR_OBC_T;



#endif

