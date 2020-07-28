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

#include <isp_mgr.h>

/* Dynamic Bin */
#include <mtkcam/drv/iopipe/CamIO/Cam_Notify.h>

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  AE statistics config
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_AE_STAT_CONFIG : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_AE_STAT_CONFIG    MyType;
private:
    MUINT32 m_u4StartAddr;
    enum
    {
        ERegInfo_CAM_AE_GAIN2_0      ,
        ERegInfo_CAM_AE_GAIN2_1      ,
        ERegInfo_CAM_AE_LMT2_0       ,
        ERegInfo_CAM_AE_LMT2_1       ,
        ERegInfo_CAM_AE_RC_CNV_0     ,
        ERegInfo_CAM_AE_RC_CNV_1     ,
        ERegInfo_CAM_AE_RC_CNV_2     ,
        ERegInfo_CAM_AE_RC_CNV_3     ,
        ERegInfo_CAM_AE_RC_CNV_4     ,
        ERegInfo_CAM_AE_YGAMMA_0     ,
        ERegInfo_CAM_AE_YGAMMA_1     ,
        ERegInfo_CAM_AE_OVER_EXPO_CFG,
        ERegInfo_CAM_AE_PIX_HST_CTL  ,
        ERegInfo_CAM_AE_PIX_HST_SET  ,
        ERegInfo_CAM_AE_PIX_HST_SET_1,
        ERegInfo_CAM_AE_PIX_HST0_YRNG,
        ERegInfo_CAM_AE_PIX_HST0_XRNG,
        ERegInfo_CAM_AE_PIX_HST1_YRNG,
        ERegInfo_CAM_AE_PIX_HST1_XRNG,
        ERegInfo_CAM_AE_PIX_HST2_YRNG,
        ERegInfo_CAM_AE_PIX_HST2_XRNG,
        ERegInfo_CAM_AE_PIX_HST3_YRNG,
        ERegInfo_CAM_AE_PIX_HST3_XRNG,
        ERegInfo_CAM_AE_PIX_HST4_YRNG,
        ERegInfo_CAM_AE_PIX_HST4_XRNG,
        ERegInfo_CAM_AE_PIX_HST5_YRNG,
        ERegInfo_CAM_AE_PIX_HST5_XRNG,
        ERegInfo_CAM_AE_STAT_EN      ,
        ERegInfo_CAM_AAO_XSIZE       ,
        ERegInfo_CAM_AAO_YSIZE       ,
//        ERegInfo_CAM_AAO_STRIDE      ,
        ERegInfo_CAM_AE_YCOEF        ,
        // [PSO]
        ERegInfo_CAM_PS_AE_YCOEF0    ,
        ERegInfo_CAM_PS_AE_YCOEF1    ,
        ERegInfo_CAM_PS_DATA_TYPE    ,
        ERegInfo_CAM_PS_HST_CFG      ,
        ERegInfo_CAM_PS_HST_ROI_X    ,
        ERegInfo_CAM_PS_HST_ROI_Y    ,
        ERegInfo_CAM_PSO_XSIZE       ,
        ERegInfo_CAM_PSO_YSIZE       ,
//        ERegInfo_CAM_PSO_STRIDE      ,
        ERegInfo_NUM
    };
    RegInfo_T   m_rIspRegInfo[ERegInfo_NUM];
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

public:
    ISP_MGR_AE_STAT_CONFIG(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr, eSensorDev)
        , m_u4StartAddr(REG_ADDR_P1(CAM_AE_GAIN2_0))
    {
        // register info addr init
        INIT_REG_INFO_ADDR_P1(CAM_AE_GAIN2_0);
        INIT_REG_INFO_ADDR_P1(CAM_AE_GAIN2_1);
        INIT_REG_INFO_ADDR_P1(CAM_AE_LMT2_0);
        INIT_REG_INFO_ADDR_P1(CAM_AE_LMT2_1);
        INIT_REG_INFO_ADDR_P1(CAM_AE_RC_CNV_0);
        INIT_REG_INFO_ADDR_P1(CAM_AE_RC_CNV_1);
        INIT_REG_INFO_ADDR_P1(CAM_AE_RC_CNV_2);
        INIT_REG_INFO_ADDR_P1(CAM_AE_RC_CNV_3);
        INIT_REG_INFO_ADDR_P1(CAM_AE_RC_CNV_4);
        INIT_REG_INFO_ADDR_P1(CAM_AE_YGAMMA_0);
        INIT_REG_INFO_ADDR_P1(CAM_AE_YGAMMA_1);
        INIT_REG_INFO_ADDR_P1(CAM_AE_OVER_EXPO_CFG);
        INIT_REG_INFO_ADDR_P1(CAM_AE_PIX_HST_CTL);
        INIT_REG_INFO_ADDR_P1(CAM_AE_PIX_HST_SET);
        INIT_REG_INFO_ADDR_P1(CAM_AE_PIX_HST_SET_1);
        INIT_REG_INFO_ADDR_P1(CAM_AE_PIX_HST0_YRNG);
        INIT_REG_INFO_ADDR_P1(CAM_AE_PIX_HST0_XRNG);
        INIT_REG_INFO_ADDR_P1(CAM_AE_PIX_HST1_YRNG);
        INIT_REG_INFO_ADDR_P1(CAM_AE_PIX_HST1_XRNG);
        INIT_REG_INFO_ADDR_P1(CAM_AE_PIX_HST2_YRNG);
        INIT_REG_INFO_ADDR_P1(CAM_AE_PIX_HST2_XRNG);
        INIT_REG_INFO_ADDR_P1(CAM_AE_PIX_HST3_YRNG);
        INIT_REG_INFO_ADDR_P1(CAM_AE_PIX_HST3_XRNG);
        INIT_REG_INFO_ADDR_P1(CAM_AE_PIX_HST4_YRNG);
        INIT_REG_INFO_ADDR_P1(CAM_AE_PIX_HST4_XRNG);
        INIT_REG_INFO_ADDR_P1(CAM_AE_PIX_HST5_YRNG);
        INIT_REG_INFO_ADDR_P1(CAM_AE_PIX_HST5_XRNG);
        INIT_REG_INFO_ADDR_P1(CAM_AE_STAT_EN);
        INIT_REG_INFO_ADDR_P1(CAM_AAO_XSIZE);
        INIT_REG_INFO_ADDR_P1(CAM_AAO_YSIZE);
//        INIT_REG_INFO_ADDR_P1(CAM_AAO_STRIDE);
        INIT_REG_INFO_ADDR_P1(CAM_AE_YCOEF);
        // [PSO]
        INIT_REG_INFO_ADDR_P1(CAM_PS_AE_YCOEF0);
        INIT_REG_INFO_ADDR_P1(CAM_PS_AE_YCOEF1);
        INIT_REG_INFO_ADDR_P1(CAM_PS_DATA_TYPE);
        INIT_REG_INFO_ADDR_P1(CAM_PS_HST_CFG);
        INIT_REG_INFO_ADDR_P1(CAM_PS_HST_ROI_X);
        INIT_REG_INFO_ADDR_P1(CAM_PS_HST_ROI_Y);
        INIT_REG_INFO_ADDR_P1(CAM_PSO_XSIZE);
        INIT_REG_INFO_ADDR_P1(CAM_PSO_YSIZE);
//        INIT_REG_INFO_ADDR_P1(CAM_PSO_STRIDE);

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
    MBOOL config(AE_STAT_PARAM_T & rAEStatConfig);

    MBOOL reconfig(MVOID *pDBinInfo, MVOID *pOutRegCfg, AE_STAT_PARAM_T & rAEStatConfig);

    MBOOL setIspAEPreGain2(AWB_GAIN_T& rIspAWBGain);

    MBOOL setAEconfigParam(MUINT32 u4NumW, MUINT32 u4NumH, MBOOL bHDR, MBOOL bOverCnt, MBOOL bTSF, MBOOL bPixel, MUINT32 u4overths, MUINT32 u4overshift, MUINT32 u4AAOmode);

    MBOOL setAEYCoefParam(MUINT32 u4AEYCoefR, MUINT32 u4AEYCoefG, MUINT32 u4AEYCoefB);

    inline AWB_GAIN_T getIspAEPreGain2()
    {
        return m_rIspAEPreGain2;
    }

    MBOOL apply(TuningMgr& rTuning, MINT32 i4SubsampleIdex=0);
} ISP_MGR_AE_STAT_CONFIG_T;

#endif
