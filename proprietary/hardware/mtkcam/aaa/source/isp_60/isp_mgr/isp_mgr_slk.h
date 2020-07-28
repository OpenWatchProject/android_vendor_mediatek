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
#ifndef _ISP_MGR_SLK_H_
#define _ISP_MGR_SLK_H_


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  SLK
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define INIT_SLK_ADDR(reg)\
    INIT_REG_INFO_ADDR_P1_MULTI(ESLK_R1 ,reg, SLK_R1_SLK_);\
    INIT_REG_INFO_ADDR_P1_MULTI(ESLK_R2 ,reg, SLK_R2_SLK_);\
    INIT_REG_INFO_ADDR_P2_MULTI(ESLK_D1 ,reg, SLK_D1A_SLK_);\
    INIT_REG_INFO_ADDR_P2_MULTI(ESLK_D2 ,reg, SLK_D2A_SLK_);\
    INIT_REG_INFO_ADDR_P2_MULTI(ESLK_D3 ,reg, SLK_D3A_SLK_);\
    INIT_REG_INFO_ADDR_P2_MULTI(ESLK_D4 ,reg, SLK_D4A_SLK_);\
    INIT_REG_INFO_ADDR_P2_MULTI(ESLK_D5 ,reg, SLK_D5A_SLK_);\
    INIT_REG_INFO_ADDR_P2_MULTI(ESLK_D6 ,reg, SLK_D6A_SLK_);\


typedef class ISP_MGR_SLK : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_SLK  MyType;
public:
    enum
    {
        ESLK_R1, //for Pass1 DM
        ESLK_R2, //for YNRS
        ESLK_D1, //for Pass2 DM
        ESLK_D2, //for YNR
        ESLK_D3, //for CNR
        ESLK_D4, //for EE
        ESLK_D5, //for NR3D
        ESLK_D6, //for LDNR
        ESubModule_NUM
    };

private:
    MBOOL m_bEnable[ESubModule_NUM];
    //MBOOL m_bCCTEnable;

    enum
    {
        ERegInfo_CEN,
        ERegInfo_RR_CON0,
        ERegInfo_RR_CON1,
        ERegInfo_GAIN,
        ERegInfo_RZ,
        //ERegInfo_XOFF,
        //ERegInfo_YOFF,
        ERegInfo_SLP_CON0,
        ERegInfo_SLP_CON1,
        ERegInfo_SLP_CON2,
        ERegInfo_SLP_CON3,
        //ERegInfo_SIZE,
        ERegInfo_NUM
    };

    RegInfo_T m_rIspRegInfo[ESubModule_NUM][ERegInfo_NUM];

public:
    ISP_MGR_SLK(MUINT32 const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, eSensorDev, ESubModule_NUM)
        //, m_bCCTEnable(MTRUE)
    {
        for(int i=0; i<ESubModule_NUM; i++){
            m_bEnable[i]    = MFALSE;
            ::memset(m_rIspRegInfo[i], 0, sizeof(RegInfo_T)*ERegInfo_NUM);
        }
        // register info addr init
        INIT_SLK_ADDR(CEN);
        INIT_SLK_ADDR(RR_CON0);
        INIT_SLK_ADDR(RR_CON1);
        INIT_SLK_ADDR(GAIN);
        INIT_SLK_ADDR(RZ);
        //INIT_SLK_ADDR(XOFF);
        //INIT_SLK_ADDR(YOFF);
        INIT_SLK_ADDR(SLP_CON0);
        INIT_SLK_ADDR(SLP_CON1);
        INIT_SLK_ADDR(SLP_CON2);
        INIT_SLK_ADDR(SLP_CON3);
        //INIT_SLK_ADDR(SIZE);
    }

    virtual ~ISP_MGR_SLK() {}

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
    MVOID
    transform_SLK(MUINT8 SubModuleIndex, const SL2_INFO_T& rSl2Info, const CROP_RZ_INFO_T& rCropRzInfo);


    MBOOL apply_P1(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, TuningMgr& rTuning, MINT32 i4SubsampleIdex);

    MBOOL apply_P2(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, MBOOL bLSC2En, dip_x_reg_t* pReg);

} ISP_MGR_SLK_T;


#endif




