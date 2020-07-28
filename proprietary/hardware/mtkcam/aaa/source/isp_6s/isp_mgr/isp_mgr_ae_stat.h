/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
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
#ifndef _ISP_MGR_AE_STAT_H_
#define _ISP_MGR_AE_STAT_H_

/* Dynamic Bin */
#include <mtkcam/drv/iopipe/CamIO/Cam_Notify.h>

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  AE statistics config
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define INIT_AE_REG_INFO_ADDR_P1_MULTI(index,reg, name)\
    REG_INFO_ADDR_MULTI(index,name##reg) = REG_ADDR_P1(name##reg)

#define INIT_AE_ADDR(reg) INIT_AE_REG_INFO_ADDR_P1_MULTI(EAE_R1 ,reg, AA_R1_AA_);

#define INIT_AAO_ADDR(reg) INIT_AE_REG_INFO_ADDR_P1_MULTI(EAE_R1 ,reg, AAO_R1_);

#define REG_AE_R1_INFO_VALUE(reg) REG_INFO_VALUE_MULTI(EAE_R1, reg)


typedef class ISP_MGR_AE_STAT_CONFIG : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_AE_STAT_CONFIG    MyType;
private:
    MUINT32 m_u4StartAddr;
    enum
    {
        EAE_R1,
        ESubModule_NUM
    };

    RegInfo_T   m_rIspRegInfo[ESubModule_NUM][EAERegInfo_NUM];
    AWB_GAIN_T  m_rIspAEPreGain2; // 1x = 512
    MBOOL m_bEnableHDRYConfig;
    MBOOL m_bEnableOverCntConfig;
    MBOOL m_bEnableTSFConfig;
    MBOOL m_bDisablePixelHistConfig;
    MBOOL m_u4BlockNumW;
    MBOOL m_u4BlockNumH;
    MUINT32 m_u4AEOverExpCntThr;
    MUINT32 m_u4AEOverExpCntShift;
    MUINT32 m_u4AEYCoefR;
    MUINT32 m_u4AEYCoefG;
    MUINT32 m_u4AEYCoefB;
    MUINT32 m_u4AAOmode;
    MUINT32 m_bDebugEnable;

protected:
    ISP_MGR_AE_STAT_CONFIG(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, EAERegInfo_NUM, eSensorDev, ESubModule_NUM)
    {
        // register info addr init
        #if 0
        INIT_REG_INFO_ADDR_P1(CAM_AE_GAIN2_0);
        INIT_REG_INFO_ADDR_P1(CAM_AE_GAIN2_1);
		#endif
        INIT_AE_ADDR(AE_LMT2_0);
        INIT_AE_ADDR(AE_LMT2_1);
        INIT_AE_ADDR(AE_RC_CNV_0);
        INIT_AE_ADDR(AE_RC_CNV_1);
        INIT_AE_ADDR(AE_RC_CNV_2);
        INIT_AE_ADDR(AE_RC_CNV_3);
        INIT_AE_ADDR(AE_RC_CNV_4);
        INIT_AE_ADDR(AE_OVER_EXPO_CFG);
        INIT_AE_ADDR(AE_PIX_HST_CTL);
        INIT_AE_ADDR(AE_PIX_HST_SET);
        INIT_AE_ADDR(AE_PIX_HST_SET_1);
        INIT_AE_ADDR(AE_PIX_HST0_YRNG);
        INIT_AE_ADDR(AE_PIX_HST0_XRNG);
        INIT_AE_ADDR(AE_PIX_HST1_YRNG);
        INIT_AE_ADDR(AE_PIX_HST1_XRNG);
        INIT_AE_ADDR(AE_PIX_HST2_YRNG);
        INIT_AE_ADDR(AE_PIX_HST2_XRNG);
        INIT_AE_ADDR(AE_PIX_HST3_YRNG);
        INIT_AE_ADDR(AE_PIX_HST3_XRNG);
        INIT_AE_ADDR(AE_PIX_HST4_YRNG);
        INIT_AE_ADDR(AE_PIX_HST4_XRNG);
        INIT_AE_ADDR(AE_PIX_HST5_YRNG);
        INIT_AE_ADDR(AE_PIX_HST5_XRNG);
        INIT_AE_ADDR(AE_PIX_SE_HST_SET);
        INIT_AE_ADDR(AE_PIX_SE_HST_SET_1);
        INIT_AE_ADDR(AE_PIX_SE_HST0_YRNG);
        INIT_AE_ADDR(AE_PIX_SE_HST0_XRNG);
        INIT_AE_ADDR(AE_PIX_SE_HST1_YRNG);
        INIT_AE_ADDR(AE_PIX_SE_HST1_XRNG);
        INIT_AE_ADDR(AE_PIX_SE_HST2_YRNG);
        INIT_AE_ADDR(AE_PIX_SE_HST2_XRNG);
        INIT_AE_ADDR(AE_PIX_SE_HST3_YRNG);
        INIT_AE_ADDR(AE_PIX_SE_HST3_XRNG);
        INIT_AE_ADDR(AE_PIX_SE_HST4_YRNG);
        INIT_AE_ADDR(AE_PIX_SE_HST4_XRNG);
        INIT_AE_ADDR(AE_PIX_SE_HST5_YRNG);
        INIT_AE_ADDR(AE_PIX_SE_HST5_XRNG);
        INIT_AAO_ADDR(AAO_XSIZE);
        INIT_AAO_ADDR(AAO_YSIZE);
        INIT_AE_ADDR(AE_STAT_EN);
        INIT_AE_ADDR(AE_YCOEF);
        INIT_AE_ADDR(AE_MODE);
        INIT_AE_ADDR(AE_BIT_CTL);
        INIT_AE_ADDR(AE_HDR_CFG);

        m_u4StartAddr = 0;
        m_rIspAEPreGain2.i4R = m_rIspAEPreGain2.i4G = m_rIspAEPreGain2.i4B = 512;
        m_bEnableHDRYConfig = 1;
        m_bEnableOverCntConfig = 1;
        m_bEnableTSFConfig = 1;
        m_bDisablePixelHistConfig = 0;
        m_u4BlockNumW = 120;
        m_u4BlockNumH = 90;
        m_u4AEOverExpCntThr = 255;
        m_u4AEOverExpCntShift = 0;
        m_u4AEYCoefR = 5;
        m_u4AEYCoefG = 9;
        m_u4AEYCoefB = 2;
        m_u4AAOmode = 0;
        m_bDebugEnable = 0;
    }

    virtual ~ISP_MGR_AE_STAT_CONFIG() {}

public: ////
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: //    Interfaces
    //MBOOL config(AE_STAT_PARAM_T & rAEStatConfig);

    MVOID configReg(MVOID *pIspRegInfo);

    //MBOOL reconfig(MVOID *pDBinInfo, MVOID *pOutRegCfg, AE_STAT_PARAM_T & rAEStatConfig);
    inline AWB_GAIN_T getIspAEPreGain2()
    {
        return m_rIspAEPreGain2;
    }
    MBOOL setIspAEPreGain2(AWB_GAIN_T& rIspAWBGain);

    MBOOL apply(TuningMgr& rTuning, MINT32 i4SubsampleIdex=0);
} ISP_MGR_AE_STAT_CONFIG_T;

template <ESensorDev_T const eSensorDev>
class ISP_MGR_AE_STAT_CONFIG_DEV : public ISP_MGR_AE_STAT_CONFIG_T
{
public:
    static
    ISP_MGR_AE_STAT_CONFIG_T&
    getInstance()
    {
        static ISP_MGR_AE_STAT_CONFIG_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_AE_STAT_CONFIG_DEV()
        : ISP_MGR_AE_STAT_CONFIG_T(eSensorDev)
    {}

    virtual ~ISP_MGR_AE_STAT_CONFIG_DEV() {}

};

#endif
