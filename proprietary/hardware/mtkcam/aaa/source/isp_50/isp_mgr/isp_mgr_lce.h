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
#ifndef _ISP_MGR_LCE_H_
#define _ISP_MGR_LCE_H_

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  LCS
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_LCS : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_LCS    MyType;
private:
    MBOOL m_bEnable;
    MBOOL m_bCCTEnable;
    MUINT32 m_u4StartAddr;

    enum
    {
        ERegInfo_CAM_LCS25_FLR,
        ERegInfo_CAM_LCS25_SATU_1,
        ERegInfo_CAM_LCS25_SATU_2,
        ERegInfo_CAM_LCS25_GAIN_1,
        ERegInfo_CAM_LCS25_GAIN_2,
        ERegInfo_CAM_LCS25_OFST_1,
        ERegInfo_CAM_LCS25_OFST_2,
        ERegInfo_CAM_LCS25_G2G_CNV_1,
        ERegInfo_CAM_LCS25_G2G_CNV_2,
        ERegInfo_CAM_LCS25_G2G_CNV_3,
        ERegInfo_CAM_LCS25_G2G_CNV_4,
        ERegInfo_CAM_LCS25_G2G_CNV_5,
        ERegInfo_NUM
    };

    RegInfo_T m_rIspRegInfo[ERegInfo_NUM];

public:
    ISP_MGR_LCS(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr, eSensorDev)
        , m_bEnable(MTRUE)
        , m_bCCTEnable(MTRUE)
        , m_u4StartAddr(REG_ADDR_P1(CAM_LCS25_FLR))
    {
        // register info addr init
        INIT_REG_INFO_ADDR_P1(CAM_LCS25_FLR);
        INIT_REG_INFO_ADDR_P1(CAM_LCS25_SATU_1);
        INIT_REG_INFO_ADDR_P1(CAM_LCS25_SATU_2);
        INIT_REG_INFO_ADDR_P1(CAM_LCS25_GAIN_1);
        INIT_REG_INFO_ADDR_P1(CAM_LCS25_GAIN_2);
        INIT_REG_INFO_ADDR_P1(CAM_LCS25_OFST_1);
        INIT_REG_INFO_ADDR_P1(CAM_LCS25_OFST_2);
        INIT_REG_INFO_ADDR_P1(CAM_LCS25_G2G_CNV_1);
        INIT_REG_INFO_ADDR_P1(CAM_LCS25_G2G_CNV_2);
        INIT_REG_INFO_ADDR_P1(CAM_LCS25_G2G_CNV_3);
        INIT_REG_INFO_ADDR_P1(CAM_LCS25_G2G_CNV_4);
        INIT_REG_INFO_ADDR_P1(CAM_LCS25_G2G_CNV_5);
    }

    virtual ~ISP_MGR_LCS() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: // Interfaces.

    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

    MBOOL
    isEnable()
    {
        return m_bEnable;
    }

    MVOID
    setEnable(MBOOL bEnable)
    {
        m_bEnable = bEnable;
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

    MBOOL apply(RAWIspCamInfo& rRawIspCamInfo, TuningMgr& rTuning, MINT32 i4SubsampleIdex=0);

} ISP_MGR_LCS_T;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  LCE
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_LCE : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_LCE    MyType;
private:
    MBOOL m_bEnable;
    MBOOL m_bCCTEnable;
    MUINT32 m_LCEIwidth;
    MUINT32 m_LCEIheight;
    MUINT32 m_u4StartAddr;

    enum
    {
        ERegInfo_DIP_X_LCE25_CON,
        ERegInfo_DIP_X_LCE25_ZR,
        ERegInfo_DIP_X_LCE25_SLM_SIZE,
        //ERegInfo_DIP_X_LCE25_OFST,
        //ERegInfo_DIP_X_LCE25_BIAS,
        //ERegInfo_DIP_X_LCE25_IMAGE_SIZE,
        ERegInfo_DIP_X_LCE25_BIL_TH0,
        ERegInfo_DIP_X_LCE25_BIL_TH1,
        ERegInfo_DIP_X_LCE25_TM_PARA0,
        ERegInfo_DIP_X_LCE25_TM_PARA1,
        ERegInfo_DIP_X_LCE25_TM_PARA2,
        ERegInfo_DIP_X_LCE25_TM_PARA3,
        ERegInfo_DIP_X_LCE25_TM_PARA4,
        ERegInfo_DIP_X_LCE25_TM_PARA5,
        ERegInfo_DIP_X_LCE25_TM_PARA6,
        ERegInfo_DIP_X_LCE25_TM_PARA7,
        ERegInfo_DIP_X_LCE25_TM_PARA8,
        ERegInfo_DIP_X_LCE25_TM_PARA9,
        ERegInfo_DIP_X_LCE25_TCHL_PARA0,
        ERegInfo_DIP_X_LCE25_TCHL_PARA1,
        ERegInfo_DIP_X_LCE25_TCHL_PARA2,

        ERegInfo_DIP_X_LCE25_HLR_PARA0,
        ERegInfo_DIP_X_LCE25_HLR_PARA1,
        ERegInfo_DIP_X_LCE25_HLR_PARA2,
        ERegInfo_DIP_X_LCE25_HLR_PARA3,
        ERegInfo_DIP_X_LCE25_HLR_PARA4,
        ERegInfo_DIP_X_LCE25_HLR_PARA5,
        ERegInfo_DIP_X_LCE25_HLR_PARA6,
        ERegInfo_DIP_X_LCE25_HLR_PARA7,

        //Dummy
        ERegInfo_DIP_X_LCE25_SLM,
        ERegInfo_DIP_X_LCE25_OFFSET,
        ERegInfo_DIP_X_LCE25_OUT,
        ERegInfo_NUM
    };

    RegInfo_T m_rIspRegInfo[ERegInfo_NUM];
    MUINT32 m_rRegInput[ERegInfo_NUM];
    MUINT32 m_LCE_Gain;

public:
    ISP_MGR_LCE(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr, eSensorDev)
        , m_bEnable(MTRUE)
        , m_bCCTEnable(MTRUE)
        , m_LCEIwidth(ISP_LCS_OUT_WD)
        , m_LCEIheight(ISP_LCS_OUT_HT)
        , m_LCE_Gain(0)
        , m_u4StartAddr(REG_ADDR_P2(DIP_X_LCE25_CON))
    {
        // register info addr init
        INIT_REG_INFO_ADDR_P2(DIP_X_LCE25_CON);
        INIT_REG_INFO_ADDR_P2(DIP_X_LCE25_ZR);
        INIT_REG_INFO_ADDR_P2(DIP_X_LCE25_SLM_SIZE);
        //INIT_REG_INFO_ADDR_P2(DIP_X_LCE25_OFST);
        //INIT_REG_INFO_ADDR_P2(DIP_X_LCE25_BIAS);
        //INIT_REG_INFO_ADDR_P2(DIP_X_LCE25_IMAGE_SIZE);
        INIT_REG_INFO_ADDR_P2(DIP_X_LCE25_BIL_TH0);
        INIT_REG_INFO_ADDR_P2(DIP_X_LCE25_BIL_TH1);
        INIT_REG_INFO_ADDR_P2(DIP_X_LCE25_TM_PARA0);
        INIT_REG_INFO_ADDR_P2(DIP_X_LCE25_TM_PARA1);
        INIT_REG_INFO_ADDR_P2(DIP_X_LCE25_TM_PARA2);
        INIT_REG_INFO_ADDR_P2(DIP_X_LCE25_TM_PARA3);
        INIT_REG_INFO_ADDR_P2(DIP_X_LCE25_TM_PARA4);
        INIT_REG_INFO_ADDR_P2(DIP_X_LCE25_TM_PARA5);
        INIT_REG_INFO_ADDR_P2(DIP_X_LCE25_TM_PARA6);
        INIT_REG_INFO_ADDR_P2(DIP_X_LCE25_TM_PARA7);
        INIT_REG_INFO_ADDR_P2(DIP_X_LCE25_TM_PARA8);
        INIT_REG_INFO_ADDR_P2(DIP_X_LCE25_TM_PARA9);

        INIT_REG_INFO_ADDR_P2(DIP_X_LCE25_TCHL_PARA0);
        INIT_REG_INFO_ADDR_P2(DIP_X_LCE25_TCHL_PARA1);
        INIT_REG_INFO_ADDR_P2(DIP_X_LCE25_TCHL_PARA2);

        INIT_REG_INFO_ADDR_P2(DIP_X_LCE25_HLR_PARA0);
        INIT_REG_INFO_ADDR_P2(DIP_X_LCE25_HLR_PARA1);
        INIT_REG_INFO_ADDR_P2(DIP_X_LCE25_HLR_PARA2);
        INIT_REG_INFO_ADDR_P2(DIP_X_LCE25_HLR_PARA3);
        INIT_REG_INFO_ADDR_P2(DIP_X_LCE25_HLR_PARA4);
        INIT_REG_INFO_ADDR_P2(DIP_X_LCE25_HLR_PARA5);
        INIT_REG_INFO_ADDR_P2(DIP_X_LCE25_HLR_PARA6);
        INIT_REG_INFO_ADDR_P2(DIP_X_LCE25_HLR_PARA7);

        INIT_REG_INFO_ADDR_P2(DIP_X_LCE25_SLM);
        INIT_REG_INFO_ADDR_P2(DIP_X_LCE25_OFFSET);
        INIT_REG_INFO_ADDR_P2(DIP_X_LCE25_OUT);

        INIT_REG_INFO_VALUE(DIP_X_LCE25_CON,       0x00000000);
        //INIT_REG_INFO_VALUE(DIP_X_LCE25_SLM_SIZE, (ISP_LCS_OUT_WD | (ISP_LCS_OUT_HT << 16)));  //default : WD=320, HT=240
        INIT_REG_INFO_VALUE(DIP_X_LCE25_BIL_TH0,   0x03200190);
        INIT_REG_INFO_VALUE(DIP_X_LCE25_BIL_TH1,   0x064004B0);

    }

    virtual ~ISP_MGR_LCE() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: // Interfaces.

    MVOID
    setLCEISize(ISP_LCS_OUT_INFO_T LCSO_Info)
    {
        m_LCEIwidth = LCSO_Info.u4OutWidth;
        m_LCEIheight = LCSO_Info.u4OutHeight;
    }

    MBOOL
    isEnable()
    {
        return m_bEnable;
    }

    MVOID
    setEnable(MBOOL bEnable)
    {
        m_bEnable = bEnable;
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



    MINT32
    getLCE_Gain()
    {
        return m_LCE_Gain;
    }

    MVOID
    setLCEGainConversionRatio(MUINT8 Ratio)
    {
       m_LCE_Gain = m_LCE_Gain & 0x0000FFFF;
       m_LCE_Gain = (Ratio << 16 | m_LCE_Gain);
    }

    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MBOOL get(ISP_xxx_T & rParam, const dip_x_reg_t* pReg) const;

    MVOID transform_LCE(const CROP_RZ_INFO_T& rRzCrop);

    MBOOL apply(const CROP_RZ_INFO_T& rRzCrop, EIspProfile_T eIspProfile, dip_x_reg_t* pReg);
} ISP_MGR_LCE_T;

#endif


