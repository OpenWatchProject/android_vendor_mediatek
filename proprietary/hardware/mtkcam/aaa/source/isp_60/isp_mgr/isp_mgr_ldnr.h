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
#ifndef _ISP_MGR_LDNR_H_
#define _ISP_MGR_LDNR_H_


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  LDNR
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define INIT_LDNR_ADDR(reg)\
    INIT_REG_INFO_ADDR_P2_MULTI(ELDNR_D1 ,reg, LDNR_D1A_LDNR_)

typedef class ISP_MGR_LDNR : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_LDNR  MyType;
public:
    enum
    {
        ELDNR_D1,
        ESubModule_NUM
    };
private:
    MBOOL m_bEnable[ESubModule_NUM];
    MBOOL m_bCCTEnable;

    enum
    {
        ERegInfo_CON1,
        ERegInfo_EE_CON1,
        ERegInfo_EE_LM_Y012,
        ERegInfo_EE_LM_Y345,
        ERegInfo_EE_SL,
        ERegInfo_EE_LWB,
        ERegInfo_EE_UPB,
        ERegInfo_RNG,
        ERegInfo_CON2,
        ERegInfo_SL,
        ERegInfo_SSL_STH,
        //ERegInfo_TILE_EDGE,
        ERegInfo_EE_CON2,
        ERegInfo_EE_TH,
        ERegInfo_EE_HGN,
        ERegInfo_CNV_00_01,
        ERegInfo_CNV_02,
        ERegInfo_CNV_10_11,
        ERegInfo_CNV_12,
        ERegInfo_CNV_20_21,
        ERegInfo_CNV_22,
        ERegInfo_ICNV_00_01,
        ERegInfo_ICNV_02,
        ERegInfo_ICNV_10_11,
        ERegInfo_ICNV_12,
        ERegInfo_ICNV_20_21,
        ERegInfo_ICNV_22,
        ERegInfo_COEF_C1,
        ERegInfo_COEF_C2,
        ERegInfo_NM_B,
        ERegInfo_NM_G,
        ERegInfo_NM_R,
        ERegInfo_NUM
    };

    RegInfo_T m_rIspRegInfo[ESubModule_NUM][ERegInfo_NUM];

public:
    ISP_MGR_LDNR(MUINT32 const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, eSensorDev, ESubModule_NUM)
        , m_bCCTEnable(MTRUE)
    {
        for(int i=0; i<ESubModule_NUM; i++){
            m_bEnable[i]    = MFALSE;
            ::memset(m_rIspRegInfo[i], 0, sizeof(RegInfo_T)*ERegInfo_NUM);
        }
        // register info addr init
        INIT_LDNR_ADDR(CON1);
        INIT_LDNR_ADDR(EE_CON1);
        INIT_LDNR_ADDR(EE_LM_Y012);
        INIT_LDNR_ADDR(EE_LM_Y345);
        INIT_LDNR_ADDR(EE_SL);
        INIT_LDNR_ADDR(EE_LWB);
        INIT_LDNR_ADDR(EE_UPB);
        INIT_LDNR_ADDR(RNG);
        INIT_LDNR_ADDR(CON2);
        INIT_LDNR_ADDR(SL);
        INIT_LDNR_ADDR(SSL_STH);
        //INIT_LDNR_ADDR(TILE_EDGE);
        INIT_LDNR_ADDR(EE_CON2);
        INIT_LDNR_ADDR(EE_TH);
        INIT_LDNR_ADDR(EE_HGN);
        INIT_LDNR_ADDR(CNV_00_01);
        INIT_LDNR_ADDR(CNV_02);
        INIT_LDNR_ADDR(CNV_10_11);
        INIT_LDNR_ADDR(CNV_12);
        INIT_LDNR_ADDR(CNV_20_21);
        INIT_LDNR_ADDR(CNV_22);
        INIT_LDNR_ADDR(ICNV_00_01);
        INIT_LDNR_ADDR(ICNV_02);
        INIT_LDNR_ADDR(ICNV_10_11);
        INIT_LDNR_ADDR(ICNV_12);
        INIT_LDNR_ADDR(ICNV_20_21);
        INIT_LDNR_ADDR(ICNV_22);
        INIT_LDNR_ADDR(COEF_C1);
        INIT_LDNR_ADDR(COEF_C2);
        INIT_LDNR_ADDR(NM_B);
        INIT_LDNR_ADDR(NM_G);
        INIT_LDNR_ADDR(NM_R);

    }

    virtual ~ISP_MGR_LDNR() {}

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

    MBOOL apply_P2(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, dip_x_reg_t* pReg);

} ISP_MGR_LDNR_T;



#endif


