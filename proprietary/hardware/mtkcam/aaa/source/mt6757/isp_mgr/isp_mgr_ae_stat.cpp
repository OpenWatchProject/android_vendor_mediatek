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
#define LOG_TAG "isp_mgr_ae_stat"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include <isp_reg.h>
#include "isp_mgr.h"

namespace NSIspTuningv3
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AE Statistics Config
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_AE_STAT_CONFIG_T&
ISP_MGR_AE_STAT_CONFIG_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
    {
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_AE_STAT_CONFIG_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_AE_STAT_CONFIG_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_AE_STAT_CONFIG_DEV<ESensorDev_Sub>::getInstance();
    case ESensorDev_SubSecond: //  Sub Sensor
        return  ISP_MGR_AE_STAT_CONFIG_DEV<ESensorDev_SubSecond>::getInstance();
    default:
        CAM_LOGE("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_AE_STAT_CONFIG_DEV<ESensorDev_Main>::getInstance();
    }
}

MBOOL
ISP_MGR_AE_STAT_CONFIG_T::
config(AE_STAT_PARAM_T& rAEStatConfig)
{
#if 1
    MUINT32 BlockNumH = m_u4BlockNumH;
    MUINT32 BlockNumW = m_u4BlockNumW;

    // CAM_AE_HST_CTL
    CAM_REG_AE_HST_CTL AeHstCtl;
    AeHstCtl.Bits.AE_HST0_EN = rAEStatConfig.rAEHistWinCFG[0].bAEHistEn;
    AeHstCtl.Bits.AE_HST1_EN = rAEStatConfig.rAEHistWinCFG[1].bAEHistEn;
    AeHstCtl.Bits.AE_HST2_EN = rAEStatConfig.rAEHistWinCFG[2].bAEHistEn;
    AeHstCtl.Bits.AE_HST3_EN = rAEStatConfig.rAEHistWinCFG[3].bAEHistEn;
    // CAM_AE_GAIN2_0
    CAM_REG_AE_GAIN2_0 AeGain20;
    AeGain20.Bits.AE_GAIN2_G = m_rIspAEPreGain2.i4G;
    AeGain20.Bits.AE_GAIN2_R = m_rIspAEPreGain2.i4R;

    // CAM_REG_AE_GAIN2_1
    CAM_REG_AE_GAIN2_1 AeGain21;
    AeGain21.Bits.AE_GAIN2_B = m_rIspAEPreGain2.i4B;

    // CAM_AE_LMT2_0
    CAM_REG_AE_LMT2_0 AeLmt20;
    AeLmt20.Bits.AE_LMT2_G = 0xFFF;
    AeLmt20.Bits.AE_LMT2_R = 0xFFF;

      // CAM_AE_LMT2_1
    CAM_REG_AE_LMT2_1 AeLmt21;
    AeLmt21.Bits.AE_LMT2_B = 0xFFF;

    // CAM_AE_RC_CNV_0
    CAM_REG_AE_RC_CNV_0 AeRcCnv0;
    AeRcCnv0.Bits.AE_RC_CNV00 = 0x200;
    AeRcCnv0.Bits.AE_RC_CNV01 = 0x000;

    // CAM_AE_RC_CNV_1
    CAM_REG_AE_RC_CNV_1 AeRcCnv1;
    AeRcCnv1.Bits.AE_RC_CNV02 = 0x000;
    AeRcCnv1.Bits.AE_RC_CNV10 = 0x000;

    // CAM_AE_RC_CNV_2
    CAM_REG_AE_RC_CNV_2 AeRcCnv2;
    AeRcCnv2.Bits.AE_RC_CNV11 = 0x200;
    AeRcCnv2.Bits.AE_RC_CNV12 = 0x000;

    // CAM_AE_RC_CNV_3
    CAM_REG_AE_RC_CNV_3 AeRcCnv3;
    AeRcCnv3.Bits.AE_RC_CNV20 = 0x000;
    AeRcCnv3.Bits.AE_RC_CNV21 = 0x000;

    // CAM_AE_RC_CNV_4
    CAM_REG_AE_RC_CNV_4 AeRcCnv4;
    AeRcCnv4.Bits.AE_RC_CNV22 = 0x200;
    AeRcCnv4.Bits.AE_RC_ACC   = 0x09;

    // CAM_AE_YGAMMA_0
    CAM_REG_AE_YGAMMA_0 AeYgamma0;
    AeYgamma0.Bits.Y_GMR4 = 0x60;
    AeYgamma0.Bits.Y_GMR3 = 0x40;
    AeYgamma0.Bits.Y_GMR2 = 0x20;
    AeYgamma0.Bits.Y_GMR1 = 0x10;

    // CAM_AE_YGAMMA_1
    CAM_REG_AE_YGAMMA_1 AeYgamma1;
    AeYgamma1.Bits.Y_GMR5 = 0x80;

    // CAM_AE_HST_SET
    CAM_REG_AE_HST_SET AeHstSet;
    AeHstSet.Bits.AE_BIN_MODE_0 = rAEStatConfig.rAEHistWinCFG[0].uAEHistBin;
    AeHstSet.Bits.AE_BIN_MODE_1 = rAEStatConfig.rAEHistWinCFG[1].uAEHistBin;
    AeHstSet.Bits.AE_BIN_MODE_2 = rAEStatConfig.rAEHistWinCFG[2].uAEHistBin;
    AeHstSet.Bits.AE_BIN_MODE_3 = rAEStatConfig.rAEHistWinCFG[3].uAEHistBin;
    AeHstSet.Bits.AE_COLOR_MODE_0 = rAEStatConfig.rAEHistWinCFG[0].uAEHistOpt;
    AeHstSet.Bits.AE_COLOR_MODE_1 = rAEStatConfig.rAEHistWinCFG[1].uAEHistOpt;
    AeHstSet.Bits.AE_COLOR_MODE_2 = rAEStatConfig.rAEHistWinCFG[2].uAEHistOpt;
    AeHstSet.Bits.AE_COLOR_MODE_3 = rAEStatConfig.rAEHistWinCFG[3].uAEHistOpt;

    // CAM_AE_HST0_RNG
    CAM_REG_AE_HST0_RNG AeHst0Rng;
    AeHst0Rng.Bits.AE_X_LOW_0 = rAEStatConfig.rAEHistWinCFG[0].uAEHistXLow ;
    AeHst0Rng.Bits.AE_X_HI_0  = rAEStatConfig.rAEHistWinCFG[0].uAEHistXHi;
    AeHst0Rng.Bits.AE_Y_LOW_0 = rAEStatConfig.rAEHistWinCFG[0].uAEHistYLow;
    AeHst0Rng.Bits.AE_Y_HI_0  = rAEStatConfig.rAEHistWinCFG[0].uAEHistYHi;

    // CAM_AE_HST1_RNG
    CAM_REG_AE_HST1_RNG AeHst1Rng;
    AeHst1Rng.Bits.AE_X_LOW_1 = rAEStatConfig.rAEHistWinCFG[1].uAEHistXLow ;
    AeHst1Rng.Bits.AE_X_HI_1  = rAEStatConfig.rAEHistWinCFG[1].uAEHistXHi;
    AeHst1Rng.Bits.AE_Y_LOW_1 = rAEStatConfig.rAEHistWinCFG[1].uAEHistYLow ;
    AeHst1Rng.Bits.AE_Y_HI_1  = rAEStatConfig.rAEHistWinCFG[1].uAEHistYHi;

    // CAM_AE_HST2_RNG
    CAM_REG_AE_HST2_RNG AeHst2Rng;
    AeHst2Rng.Bits.AE_X_LOW_2 = rAEStatConfig.rAEHistWinCFG[2].uAEHistXLow ;
    AeHst2Rng.Bits.AE_X_HI_2  = rAEStatConfig.rAEHistWinCFG[2].uAEHistXHi;
    AeHst2Rng.Bits.AE_Y_LOW_2 = rAEStatConfig.rAEHistWinCFG[2].uAEHistYLow ;
    AeHst2Rng.Bits.AE_Y_HI_2  = rAEStatConfig.rAEHistWinCFG[2].uAEHistYHi;

    // CAM_AE_HST3_RNG
    CAM_REG_AE_HST3_RNG AeHst3Rng;
    AeHst3Rng.Bits.AE_X_LOW_3 = rAEStatConfig.rAEHistWinCFG[3].uAEHistXLow ;
    AeHst3Rng.Bits.AE_X_HI_3  = rAEStatConfig.rAEHistWinCFG[3].uAEHistXHi;
    AeHst3Rng.Bits.AE_Y_LOW_3 = rAEStatConfig.rAEHistWinCFG[3].uAEHistYLow;
    AeHst3Rng.Bits.AE_Y_HI_3  = rAEStatConfig.rAEHistWinCFG[3].uAEHistYHi;

    // CAM_AE_SPARE
    //
    // CAM_AE_OVER_EXPO_CFG
    CAM_REG_AE_OVER_EXPO_CFG AeOverExpoCfg;
    AeOverExpoCfg.Bits.AE_OVER_EXPO_THR = m_u4AEOverExpCntThr;
    AeOverExpoCfg.Bits.AE_OVER_EXPO_SFT = m_u4AEOverExpCntShift;

    // CAM_AE_PIX_HST_CTL
    CAM_REG_AE_PIX_HST_CTL AePixHstCtl;
    AePixHstCtl.Bits.AE_PIX_HST0_EN = rAEStatConfig.rAEPixelHistWinCFG[0].bAEHistEn;
    AePixHstCtl.Bits.AE_PIX_HST1_EN = rAEStatConfig.rAEPixelHistWinCFG[1].bAEHistEn;
    AePixHstCtl.Bits.AE_PIX_HST2_EN = rAEStatConfig.rAEPixelHistWinCFG[2].bAEHistEn;
    AePixHstCtl.Bits.AE_PIX_HST3_EN = rAEStatConfig.rAEPixelHistWinCFG[3].bAEHistEn;

    // CAM_AE_PIX_HST_SET
    CAM_REG_AE_PIX_HST_SET AePixHstSet;
    AePixHstSet.Bits.AE_PIX_BIN_MODE_0 = rAEStatConfig.rAEPixelHistWinCFG[0].uAEHistBin;
    AePixHstSet.Bits.AE_PIX_BIN_MODE_1 = rAEStatConfig.rAEPixelHistWinCFG[1].uAEHistBin;
    AePixHstSet.Bits.AE_PIX_BIN_MODE_2 = rAEStatConfig.rAEPixelHistWinCFG[2].uAEHistBin;
    AePixHstSet.Bits.AE_PIX_BIN_MODE_3 = rAEStatConfig.rAEPixelHistWinCFG[3].uAEHistBin;
    AePixHstSet.Bits.AE_PIX_COLOR_MODE_0 = rAEStatConfig.rAEPixelHistWinCFG[0].uAEHistOpt;
    AePixHstSet.Bits.AE_PIX_COLOR_MODE_1 = rAEStatConfig.rAEPixelHistWinCFG[1].uAEHistOpt;
    AePixHstSet.Bits.AE_PIX_COLOR_MODE_2 = rAEStatConfig.rAEPixelHistWinCFG[2].uAEHistOpt;
    AePixHstSet.Bits.AE_PIX_COLOR_MODE_3 = rAEStatConfig.rAEPixelHistWinCFG[3].uAEHistOpt;

    // CAM_AE_PIX_HST0_YRNG
    CAM_REG_AE_PIX_HST0_YRNG AePixHst0Yrng;
    AePixHst0Yrng.Bits.AE_PIX_Y_LOW_0 = rAEStatConfig.rAEPixelHistWinCFG[0].uAEHistYLow;
    AePixHst0Yrng.Bits.AE_PIX_Y_HI_0  = rAEStatConfig.rAEPixelHistWinCFG[0].uAEHistYHi;

    // CAM_AE_PIX_HST0_XRNG
    CAM_REG_AE_PIX_HST0_XRNG AePixHst0Xrng;
    AePixHst0Xrng.Bits.AE_PIX_X_LOW_0 = rAEStatConfig.rAEPixelHistWinCFG[0].uAEHistXLow;
    AePixHst0Xrng.Bits.AE_PIX_X_HI_0  = rAEStatConfig.rAEPixelHistWinCFG[0].uAEHistXHi;

    // CAM_AE_PIX_HST1_YRNG
    CAM_REG_AE_PIX_HST1_YRNG AePixHst1Yrng;
    AePixHst1Yrng.Bits.AE_PIX_Y_LOW_1 = rAEStatConfig.rAEPixelHistWinCFG[1].uAEHistYLow ;
    AePixHst1Yrng.Bits.AE_PIX_Y_HI_1  = rAEStatConfig.rAEPixelHistWinCFG[1].uAEHistYHi;

    // CAM_AE_PIX_HST1_XRNG
    CAM_REG_AE_PIX_HST1_XRNG AePixHst1Xrng;
    AePixHst1Xrng.Bits.AE_PIX_X_LOW_1 = rAEStatConfig.rAEPixelHistWinCFG[1].uAEHistXLow;
    AePixHst1Xrng.Bits.AE_PIX_X_HI_1  = rAEStatConfig.rAEPixelHistWinCFG[1].uAEHistXHi;

    // CAM_AE_PIX_HST2_YRNG
    CAM_REG_AE_PIX_HST2_YRNG AePixHst2Yrng;
    AePixHst2Yrng.Bits.AE_PIX_Y_LOW_2 = rAEStatConfig.rAEPixelHistWinCFG[2].uAEHistYLow ;
    AePixHst2Yrng.Bits.AE_PIX_Y_HI_2  = rAEStatConfig.rAEPixelHistWinCFG[2].uAEHistYHi;

    // CAM_AE_PIX_HST1_XRNG
    CAM_REG_AE_PIX_HST2_XRNG AePixHst2Xrng;
    AePixHst2Xrng.Bits.AE_PIX_X_LOW_2 = rAEStatConfig.rAEPixelHistWinCFG[2].uAEHistXLow;
    AePixHst2Xrng.Bits.AE_PIX_X_HI_2  = rAEStatConfig.rAEPixelHistWinCFG[2].uAEHistXHi;

    // CAM_AE_PIX_HST3_YRNG
    CAM_REG_AE_PIX_HST3_YRNG AePixHst3Yrng;
    AePixHst3Yrng.Bits.AE_PIX_Y_LOW_3 = rAEStatConfig.rAEPixelHistWinCFG[3].uAEHistYLow ;
    AePixHst3Yrng.Bits.AE_PIX_Y_HI_3  = rAEStatConfig.rAEPixelHistWinCFG[3].uAEHistYHi;

    // CAM_AE_PIX_HST3_XRNG
    CAM_REG_AE_PIX_HST3_XRNG AePixHst3Xrng;
    AePixHst3Xrng.Bits.AE_PIX_X_LOW_3 = rAEStatConfig.rAEPixelHistWinCFG[3].uAEHistXLow;
    AePixHst3Xrng.Bits.AE_PIX_X_HI_3  = rAEStatConfig.rAEPixelHistWinCFG[3].uAEHistXHi;

    // CAM_AE_HST_SEL
    CAM_REG_AE_HST_SEL AeHstSel;
    AeHstSel.Bits.AE_HST_SEL = m_bDisablePixelHistConfig;

    // CAM_AE_STAT_EN
    CAM_REG_AE_STAT_EN AeStatEn;
    AeStatEn.Bits.AE_TSF_STAT_EN = m_bEnableTSFConfig;
    AeStatEn.Bits.AE_OVERCNT_EN  = m_bEnableOverCntConfig;
    AeStatEn.Bits.AE_HDR_STAT_EN = m_bEnableHDRYConfig;

    // CAM_AAO_XSIZE
    CAM_REG_AAO_XSIZE AaoXsize;
    AaoXsize.Bits.XSIZE = BlockNumH*(BlockNumW*4+BlockNumW*1+BlockNumW*0.5*AeStatEn.Bits.AE_HDR_STAT_EN+BlockNumW*1*AeStatEn.Bits.AE_OVERCNT_EN+BlockNumW*2*AeStatEn.Bits.AE_TSF_STAT_EN)+4*128*3;
    CAM_REG_AAO_YSIZE AaoYsize;
    AaoYsize.Bits.YSIZE = 1;
    CAM_REG_AAO_STRIDE AaoStride;
    AaoStride.Bits.STRIDE = AaoXsize.Bits.XSIZE;

    // CAM_AE_YCOEF
    CAM_REG_AE_YCOEF AeYCoef;
    AeYCoef.Bits.AE_YCOEF_R = m_u4AEYCoefR;
    AeYCoef.Bits.AE_YCOEF_G = m_u4AEYCoefG;
    AeYCoef.Bits.AE_YCOEF_B = m_u4AEYCoefB;


#else // For test
    MUINT32 BlockNumH = 90;
    MUINT32 BlockNumW = 120;
    MUINT32 sizeH = 2112;
    MUINT32 sizeW = 2816;
    MUINT32 tgH = sizeH/2;
    MUINT32 tgW = sizeW/4;

    // CAM_AE_HST_CTL
    CAM_REG_AE_HST_CTL AeHstCtl;
    AeHstCtl.Bits.AE_HST0_EN = 1;
    AeHstCtl.Bits.AE_HST1_EN = 1;
    AeHstCtl.Bits.AE_HST2_EN = 1;
    AeHstCtl.Bits.AE_HST3_EN = 1;

    // CAM_AE_GAIN2_0
    CAM_REG_AE_GAIN2_0 AeGain20;
    AeGain20.Bits.AE_GAIN2_G = 512;
    AeGain20.Bits.AE_GAIN2_R = 512;

    // CAM_REG_AE_GAIN2_1
    CAM_REG_AE_GAIN2_1 AeGain21;
    AeGain21.Bits.AE_GAIN2_B = 512;

    // CAM_AE_LMT2_0
    CAM_REG_AE_LMT2_0 AeLmt20;
    AeLmt20.Bits.AE_LMT2_G = 0xFFF;
    AeLmt20.Bits.AE_LMT2_R = 0xFFF;

      // CAM_AE_LMT2_1
    CAM_REG_AE_LMT2_1 AeLmt21;
    AeLmt21.Bits.AE_LMT2_B = 0xFFF;

    // CAM_AE_RC_CNV_0
    CAM_REG_AE_RC_CNV_0 AeRcCnv0;
    AeRcCnv0.Bits.AE_RC_CNV00 = 0x200;
    AeRcCnv0.Bits.AE_RC_CNV01 = 0x000;

    // CAM_AE_RC_CNV_1
    CAM_REG_AE_RC_CNV_1 AeRcCnv1;
    AeRcCnv1.Bits.AE_RC_CNV02 = 0x000;
    AeRcCnv1.Bits.AE_RC_CNV10 = 0x000;

    // CAM_AE_RC_CNV_2
    CAM_REG_AE_RC_CNV_2 AeRcCnv2;
    AeRcCnv2.Bits.AE_RC_CNV11 = 0x200;
    AeRcCnv2.Bits.AE_RC_CNV12 = 0x000;

    // CAM_AE_RC_CNV_3
    CAM_REG_AE_RC_CNV_3 AeRcCnv3;
    AeRcCnv3.Bits.AE_RC_CNV20 = 0x000;
    AeRcCnv3.Bits.AE_RC_CNV21 = 0x000;

    // CAM_AE_RC_CNV_4
    CAM_REG_AE_RC_CNV_4 AeRcCnv4;
    AeRcCnv4.Bits.AE_RC_CNV22 = 0x200;
    AeRcCnv4.Bits.AE_RC_ACC   = 0x09;

    // CAM_AE_YGAMMA_0
    CAM_REG_AE_YGAMMA_0 AeYgamma0;
    AeYgamma0.Bits.Y_GMR4 = 0x60;
    AeYgamma0.Bits.Y_GMR3 = 0x40;
    AeYgamma0.Bits.Y_GMR2 = 0x20;
    AeYgamma0.Bits.Y_GMR1 = 0x10;

    // CAM_AE_YGAMMA_1
    CAM_REG_AE_YGAMMA_1 AeYgamma1;
    AeYgamma1.Bits.Y_GMR5 = 0x80;

    // CAM_AE_HST_SET
    CAM_REG_AE_HST_SET AeHstSet;
    AeHstSet.Bits.AE_BIN_MODE_0 = 0;
    AeHstSet.Bits.AE_BIN_MODE_1 = 0;
    AeHstSet.Bits.AE_BIN_MODE_2 = 0;
    AeHstSet.Bits.AE_BIN_MODE_3 = 0;
    AeHstSet.Bits.AE_COLOR_MODE_0 = 3;
    AeHstSet.Bits.AE_COLOR_MODE_1 = 3;
    AeHstSet.Bits.AE_COLOR_MODE_2 = 4;
    AeHstSet.Bits.AE_COLOR_MODE_3 = 4;

    // CAM_AE_HST0_RNG
    CAM_REG_AE_HST0_RNG AeHst0Rng;
    AeHst0Rng.Bits.AE_X_LOW_0 = 0 ;
    AeHst0Rng.Bits.AE_X_HI_0  = BlockNumW - 1;
    AeHst0Rng.Bits.AE_Y_LOW_0 = 0 ;
    AeHst0Rng.Bits.AE_Y_HI_0  = BlockNumH - 1;

    // CAM_AE_HST1_RNG
    CAM_REG_AE_HST1_RNG AeHst1Rng;
    AeHst1Rng.Bits.AE_X_LOW_1 = 0 ;
    AeHst1Rng.Bits.AE_X_HI_1  = BlockNumW - 1;
    AeHst1Rng.Bits.AE_Y_LOW_1 = 0 ;
    AeHst1Rng.Bits.AE_Y_HI_1  = BlockNumH - 1;

    // CAM_AE_HST2_RNG
    CAM_REG_AE_HST2_RNG AeHst2Rng;
    AeHst2Rng.Bits.AE_X_LOW_2 = 0 ;
    AeHst2Rng.Bits.AE_X_HI_2  = BlockNumW - 1;
    AeHst2Rng.Bits.AE_Y_LOW_2 = 0 ;
    AeHst2Rng.Bits.AE_Y_HI_2  = BlockNumH - 1;

    // CAM_AE_HST3_RNG
    CAM_REG_AE_HST3_RNG AeHst3Rng;
    AeHst3Rng.Bits.AE_X_LOW_3 = 0 ;
    AeHst3Rng.Bits.AE_X_HI_3  = BlockNumW - 1;
    AeHst3Rng.Bits.AE_Y_LOW_3 = 0 ;
    AeHst3Rng.Bits.AE_Y_HI_3  = BlockNumH - 1;

    // CAM_AE_SPARE
    //
    // CAM_AE_OVER_EXPO_CFG
    CAM_REG_AE_OVER_EXPO_CFG AeOverExpoCfg;
    AeOverExpoCfg.Bits.AE_OVER_EXPO_THR = 0xFF;
    AeOverExpoCfg.Bits.AE_OVER_EXPO_SFT = 0;

    // CAM_AE_PIX_HST_CTL
    CAM_REG_AE_PIX_HST_CTL AePixHstCtl;
    AePixHstCtl.Bits.AE_PIX_HST0_EN = 1;
    AePixHstCtl.Bits.AE_PIX_HST1_EN = 1;
    AePixHstCtl.Bits.AE_PIX_HST2_EN = 1;
    AePixHstCtl.Bits.AE_PIX_HST3_EN = 1;

    // CAM_AE_PIX_HST_SET
    CAM_REG_AE_PIX_HST_SET AePixHstSet;
    AePixHstSet.Bits.AE_PIX_BIN_MODE_0 = 0;
    AePixHstSet.Bits.AE_PIX_BIN_MODE_1 = 0;
    AePixHstSet.Bits.AE_PIX_BIN_MODE_2 = 0;
    AePixHstSet.Bits.AE_PIX_BIN_MODE_3 = 0;
    AePixHstSet.Bits.AE_PIX_COLOR_MODE_0 = 3;
    AePixHstSet.Bits.AE_PIX_COLOR_MODE_1 = 3;
    AePixHstSet.Bits.AE_PIX_COLOR_MODE_2 = 4;
    AePixHstSet.Bits.AE_PIX_COLOR_MODE_3 = 4;

    // CAM_AE_PIX_HST0_YRNG
    CAM_REG_AE_PIX_HST0_YRNG AePixHst0Yrng;
    AePixHst0Yrng.Bits.AE_PIX_Y_LOW_0 = 0 ;
    AePixHst0Yrng.Bits.AE_PIX_Y_HI_0  = tgH - 1;

    // CAM_AE_PIX_HST0_XRNG
    CAM_REG_AE_PIX_HST0_XRNG AePixHst0Xrng;
    AePixHst0Xrng.Bits.AE_PIX_X_LOW_0 = 0;
    AePixHst0Xrng.Bits.AE_PIX_X_HI_0  = tgW - 1;

    // CAM_AE_PIX_HST1_YRNG
    CAM_REG_AE_PIX_HST1_YRNG AePixHst1Yrng;
    AePixHst1Yrng.Bits.AE_PIX_Y_LOW_1 = 0 ;
    AePixHst1Yrng.Bits.AE_PIX_Y_HI_1  = tgH - 1;

    // CAM_AE_PIX_HST1_XRNG
    CAM_REG_AE_PIX_HST1_XRNG AePixHst1Xrng;
    AePixHst1Xrng.Bits.AE_PIX_X_LOW_1 = 0;
    AePixHst1Xrng.Bits.AE_PIX_X_HI_1  = tgW - 1;

    // CAM_AE_PIX_HST2_YRNG
    CAM_REG_AE_PIX_HST2_YRNG AePixHst2Yrng;
    AePixHst2Yrng.Bits.AE_PIX_Y_LOW_2 = 0 ;
    AePixHst2Yrng.Bits.AE_PIX_Y_HI_2  = tgH - 1;

    // CAM_AE_PIX_HST1_XRNG
    CAM_REG_AE_PIX_HST2_XRNG AePixHst2Xrng;
    AePixHst2Xrng.Bits.AE_PIX_X_LOW_2 = 0;
    AePixHst2Xrng.Bits.AE_PIX_X_HI_2  = tgW - 1;

    // CAM_AE_PIX_HST3_YRNG
    CAM_REG_AE_PIX_HST3_YRNG AePixHst3Yrng;
    AePixHst3Yrng.Bits.AE_PIX_Y_LOW_3 = 0 ;
    AePixHst3Yrng.Bits.AE_PIX_Y_HI_3  = tgH - 1;

    // CAM_AE_PIX_HST3_XRNG
    CAM_REG_AE_PIX_HST3_XRNG AePixHst3Xrng;
    AePixHst3Xrng.Bits.AE_PIX_X_LOW_3 = 0;
    AePixHst3Xrng.Bits.AE_PIX_X_HI_3  = tgW - 1;

    // CAM_AE_HST_SEL
    CAM_REG_AE_HST_SEL AeHstSel;
    AeHstSel.Bits.AE_HST_SEL = 0;

    // CAM_AE_STAT_EN
    CAM_REG_AE_STAT_EN AeStatEn;
    AeStatEn.Bits.AE_TSF_STAT_EN = 1;
    AeStatEn.Bits.AE_OVERCNT_EN  = 1;
    AeStatEn.Bits.AE_HDR_STAT_EN = 1;

    // CAM_AAO_XSIZE
    CAM_REG_AAO_XSIZE AaoXsize;
    AaoXsize.Bits.XSIZE = BlockNumH*(BlockNumW*4+BlockNumW*1+BlockNumW*0.5*AeStatEn.Bits.AE_HDR_STAT_EN+BlockNumW*1*AeStatEn.Bits.AE_OVERCNT_EN+BlockNumW*2*AeStatEn.Bits.AE_TSF_STAT_EN)+4*128*3;
    CAM_REG_AAO_YSIZE AaoYsize;
    AaoYsize.Bits.YSIZE = 0;
    CAM_REG_AAO_STRIDE AaoStride;
    AaoStride.Bits.STRIDE = AaoXsize.Bits.XSIZE;



#endif

    REG_INFO_VALUE(CAM_AE_HST_CTL) = (MUINT32)AeHstCtl.Raw;
    REG_INFO_VALUE(CAM_AE_GAIN2_0) = (MUINT32)AeGain20.Raw;
    REG_INFO_VALUE(CAM_AE_GAIN2_1) = (MUINT32)AeGain21.Raw;
    REG_INFO_VALUE(CAM_AE_LMT2_0)  = (MUINT32)AeLmt20.Raw;
    REG_INFO_VALUE(CAM_AE_LMT2_1)  = (MUINT32)AeLmt21.Raw;
    REG_INFO_VALUE(CAM_AE_RC_CNV_0) = (MUINT32)AeRcCnv0.Raw;
    REG_INFO_VALUE(CAM_AE_RC_CNV_1) = (MUINT32)AeRcCnv1.Raw;
    REG_INFO_VALUE(CAM_AE_RC_CNV_2) = (MUINT32)AeRcCnv2.Raw;
    REG_INFO_VALUE(CAM_AE_RC_CNV_3) = (MUINT32)AeRcCnv3.Raw;
    REG_INFO_VALUE(CAM_AE_RC_CNV_4) = (MUINT32)AeRcCnv4.Raw;
    REG_INFO_VALUE(CAM_AE_YGAMMA_0) = (MUINT32)AeYgamma0.Raw;
    REG_INFO_VALUE(CAM_AE_YGAMMA_1) = (MUINT32)AeYgamma1.Raw;
    REG_INFO_VALUE(CAM_AE_HST_SET) = (MUINT32)AeHstSet.Raw;
    REG_INFO_VALUE(CAM_AE_HST0_RNG) = (MUINT32)AeHst0Rng.Raw;
    REG_INFO_VALUE(CAM_AE_HST1_RNG) = (MUINT32)AeHst1Rng.Raw;
    REG_INFO_VALUE(CAM_AE_HST2_RNG) = (MUINT32)AeHst2Rng.Raw;
    REG_INFO_VALUE(CAM_AE_HST3_RNG) = (MUINT32)AeHst3Rng.Raw;
    REG_INFO_VALUE(CAM_AE_OVER_EXPO_CFG) = (MUINT32)AeOverExpoCfg.Raw;
    REG_INFO_VALUE(CAM_AE_PIX_HST_CTL) = (MUINT32)AePixHstCtl.Raw;
    REG_INFO_VALUE(CAM_AE_PIX_HST_SET) = (MUINT32)AePixHstSet.Raw;
    REG_INFO_VALUE(CAM_AE_PIX_HST0_YRNG) = (MUINT32)AePixHst0Yrng.Raw;
    REG_INFO_VALUE(CAM_AE_PIX_HST0_XRNG) = (MUINT32)AePixHst0Xrng.Raw;
    REG_INFO_VALUE(CAM_AE_PIX_HST1_YRNG) = (MUINT32)AePixHst1Yrng.Raw;
    REG_INFO_VALUE(CAM_AE_PIX_HST1_XRNG) = (MUINT32)AePixHst1Xrng.Raw;
    REG_INFO_VALUE(CAM_AE_PIX_HST2_YRNG) = (MUINT32)AePixHst2Yrng.Raw;
    REG_INFO_VALUE(CAM_AE_PIX_HST2_XRNG) = (MUINT32)AePixHst2Xrng.Raw;
    REG_INFO_VALUE(CAM_AE_PIX_HST3_YRNG) = (MUINT32)AePixHst3Yrng.Raw;
    REG_INFO_VALUE(CAM_AE_PIX_HST3_XRNG) = (MUINT32)AePixHst3Xrng.Raw;
    REG_INFO_VALUE(CAM_AE_HST_SEL) = (MUINT32)AeHstSel.Raw;
    REG_INFO_VALUE(CAM_AE_STAT_EN) = (MUINT32)AeStatEn.Raw;
    REG_INFO_VALUE(CAM_AAO_XSIZE) = (MUINT32)AaoXsize.Raw;
    REG_INFO_VALUE(CAM_AAO_YSIZE) = (MUINT32)AaoYsize.Raw;
    REG_INFO_VALUE(CAM_AAO_STRIDE) = (MUINT32)AaoStride.Raw;
    REG_INFO_VALUE(CAM_AE_YCOEF) = (MUINT32)AeYCoef.Raw;
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AE RAW Pre-gain2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
ISP_MGR_AE_STAT_CONFIG_T::
setIspAEPreGain2(AWB_GAIN_T& rIspAWBGain)
{
    m_rIspAEPreGain2 = rIspAWBGain;

    if( (m_rIspAEPreGain2.i4G <512)||(m_rIspAEPreGain2.i4R <512)||(m_rIspAEPreGain2.i4B <512)){
        CAM_LOGD("[AEconfig pregain2 Warning] R/G/B = %d/%d/%d \n", m_rIspAEPreGain2.i4R, m_rIspAEPreGain2.i4G, m_rIspAEPreGain2.i4B);
        m_rIspAEPreGain2.i4R = 512;
        m_rIspAEPreGain2.i4G = 512;
        m_rIspAEPreGain2.i4B = 512;
    }

    // CAM_AE_GAIN2_0
    reinterpret_cast<ISP_CAM_AE_RAWPREGAIN2_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_GAIN2_0))->RAWPREGAIN2_R = m_rIspAEPreGain2.i4R;
    reinterpret_cast<ISP_CAM_AE_RAWPREGAIN2_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_GAIN2_0))->RAWPREGAIN2_G = m_rIspAEPreGain2.i4G;
    // CAM_REG_AE_GAIN2_1
    reinterpret_cast<ISP_CAM_AE_RAWPREGAIN2_1_T*>(REG_INFO_VALUE_PTR(CAM_AE_GAIN2_1))->RAWPREGAIN2_B = m_rIspAEPreGain2.i4B;

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AE Statistic and Histogram config
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
ISP_MGR_AE_STAT_CONFIG_T::
setAEconfigParam(MUINT32 u4NumW, MUINT32 u4NumH, MBOOL bHDR, MBOOL bOverCnt, MBOOL bTSF, MBOOL bPixel, MUINT32 u4overths, MUINT32 u4overshift)
{
    CAM_LOGD("[AEconfig] u4NumW/u4NumH/bHDR/bOverCnt/bTSF/bPixel/thr/shift = %d/%d/%d/%d/%d/%d/%d/%d -> %d/%d/%d/%d/%d/%d/%d/%d \n",
            m_u4BlockNumW, m_u4BlockNumH, m_bEnableHDRYConfig, m_bEnableOverCntConfig, m_bEnableTSFConfig, m_bDisablePixelHistConfig,m_u4AEOverExpCntThr,m_u4AEOverExpCntShift,
            u4NumW, u4NumH, bHDR, bOverCnt, bTSF, bPixel, u4overths, u4overshift);
    m_u4BlockNumW = u4NumW;
    m_u4BlockNumH = u4NumH;
    m_bEnableHDRYConfig = bHDR;
    m_bEnableOverCntConfig = bOverCnt;
    m_bEnableTSFConfig = bTSF;
    m_bDisablePixelHistConfig = bPixel;
    m_u4AEOverExpCntThr = u4overths;
    m_u4AEOverExpCntShift = u4overshift;

    return MTRUE;
}

MBOOL
ISP_MGR_AE_STAT_CONFIG_T::
setAEYCoefParam(MUINT32 u4AEYCoefR, MUINT32 u4AEYCoefG, MUINT32 u4AEYCoefB)
{
    CAM_LOGD("[AEAEYCoef] u4AEYCoefR/G/B = %d/%d/%d\n", u4AEYCoefR, u4AEYCoefG, u4AEYCoefB);
    m_u4AEYCoefR = u4AEYCoefR;
    m_u4AEYCoefG = u4AEYCoefG;
    m_u4AEYCoefB = u4AEYCoefB;

    return MTRUE;
}


MBOOL
ISP_MGR_AE_STAT_CONFIG_T::
apply(TuningMgr& rTuning, MINT32 i4SubsampleIdex)
{
    rTuning.updateEngine(eTuningMgrFunc_AA, MTRUE, i4SubsampleIdex);

    // Register setting
    TUNING_MGR_WRITE_BITS_CAM(&rTuning, CAM_CTL_EN, AA_EN, 1, i4SubsampleIdex);
    rTuning.tuningMgrWriteRegs(
        static_cast<TUNING_MGR_REG_IO_STRUCT*>(m_pRegInfo),
        m_u4RegInfoNum, i4SubsampleIdex);

    dumpRegInfo("ae_stat_cfg");
    return MTRUE;
}

#if 0
MBOOL
ISP_MGR_AE_STAT_CONFIG_T::
config(MINT32 i4SensorIndex, AE_STAT_PARAM_T& rAEStatConfig)
{
    addressErrorCheck("Before ISP_MGR_AE_STAT_CONFIG_T::apply()");

    if (m_eSensorTG == ESensorTG_1) {
         // CAM_AE_HST_CTL
#if 1      // disable first for verify AAO statistic output
        reinterpret_cast<ISP_CAM_AE_HST_CTL_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST_CTL))->AE_HST0_EN = rAEStatConfig.rAEHistWinCFG[0].bAEHistEn;
        reinterpret_cast<ISP_CAM_AE_HST_CTL_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST_CTL))->AE_HST1_EN = rAEStatConfig.rAEHistWinCFG[1].bAEHistEn;
        reinterpret_cast<ISP_CAM_AE_HST_CTL_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST_CTL))->AE_HST2_EN = rAEStatConfig.rAEHistWinCFG[2].bAEHistEn;
        reinterpret_cast<ISP_CAM_AE_HST_CTL_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST_CTL))->AE_HST3_EN = rAEStatConfig.rAEHistWinCFG[3].bAEHistEn;
#else
        reinterpret_cast<ISP_CAM_AE_HST_CTL_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST_CTL))->AE_HST0_EN = 0;
        reinterpret_cast<ISP_CAM_AE_HST_CTL_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST_CTL))->AE_HST1_EN = 0;
        reinterpret_cast<ISP_CAM_AE_HST_CTL_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST_CTL))->AE_HST2_EN = 0;
        reinterpret_cast<ISP_CAM_AE_HST_CTL_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST_CTL))->AE_HST3_EN = 0;
#endif
        // CAM_AE_GAIN2_0
        reinterpret_cast<ISP_CAM_AE_RAWPREGAIN2_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_GAIN2_0))->RAWPREGAIN2_R = m_rIspAEPreGain2.i4R;
        reinterpret_cast<ISP_CAM_AE_RAWPREGAIN2_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_GAIN2_0))->RAWPREGAIN2_G = m_rIspAEPreGain2.i4G;
        // CAM_REG_AE_GAIN2_1
        reinterpret_cast<ISP_CAM_AE_RAWPREGAIN2_1_T*>(REG_INFO_VALUE_PTR(CAM_AE_GAIN2_1))->RAWPREGAIN2_B = m_rIspAEPreGain2.i4B;
        // CAM_AE_LMT2_0
        reinterpret_cast<ISP_CAM_AE_RAWLIMIT2_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_LMT2_0))->AE_LIMIT2_R = 0xFFF;
        reinterpret_cast<ISP_CAM_AE_RAWLIMIT2_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_LMT2_0))->AE_LIMIT2_G = 0xFFF;
        // CAM_AE_LMT2_1
        reinterpret_cast<ISP_CAM_AE_RAWLIMIT2_1_T*>(REG_INFO_VALUE_PTR(CAM_AE_LMT2_1))->AE_LIMIT2_B = 0xFFF;
        // CAM_AE_RC_CNV_0
        reinterpret_cast<ISP_CAM_AE_MATRIX_COEF0_T*>(REG_INFO_VALUE_PTR(CAM_AE_RC_CNV_0))->RC_CNV00 = 0x200;
        reinterpret_cast<ISP_CAM_AE_MATRIX_COEF0_T*>(REG_INFO_VALUE_PTR(CAM_AE_RC_CNV_0))->RC_CNV01 = 0x000;
        // CAM_AE_RC_CNV_1
        reinterpret_cast<ISP_CAM_AE_MATRIX_COEF1_T*>(REG_INFO_VALUE_PTR(CAM_AE_RC_CNV_1))->RC_CNV02 = 0x000;
        reinterpret_cast<ISP_CAM_AE_MATRIX_COEF1_T*>(REG_INFO_VALUE_PTR(CAM_AE_RC_CNV_1))->RC_CNV10 = 0x000;
        // CAM_AE_RC_CNV_2
        reinterpret_cast<ISP_CAM_AE_MATRIX_COEF2_T*>(REG_INFO_VALUE_PTR(CAM_AE_RC_CNV_2))->RC_CNV11 = 0x200;
        reinterpret_cast<ISP_CAM_AE_MATRIX_COEF2_T*>(REG_INFO_VALUE_PTR(CAM_AE_RC_CNV_2))->RC_CNV12 = 0x000;
        // CAM_AE_RC_CNV_3
        reinterpret_cast<ISP_CAM_AE_MATRIX_COEF3_T*>(REG_INFO_VALUE_PTR(CAM_AE_RC_CNV_3))->RC_CNV20 = 0x000;
        reinterpret_cast<ISP_CAM_AE_MATRIX_COEF3_T*>(REG_INFO_VALUE_PTR(CAM_AE_RC_CNV_3))->RC_CNV21 = 0x000;
        // CAM_AE_RC_CNV_4
        reinterpret_cast<ISP_CAM_AE_MATRIX_COEF4_T*>(REG_INFO_VALUE_PTR(CAM_AE_RC_CNV_4))->RC_CNV22 = 0x200;
        reinterpret_cast<ISP_CAM_AE_MATRIX_COEF4_T*>(REG_INFO_VALUE_PTR(CAM_AE_RC_CNV_4))->AE_RC_ACC = 0x09;
        // CAM_AE_YGAMMA_0
        reinterpret_cast<ISP_CAM_AE_YGAMMA_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_YGAMMA_0))->Y_GMR1 = 0x10;
        reinterpret_cast<ISP_CAM_AE_YGAMMA_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_YGAMMA_0))->Y_GMR2 = 0x20;
        reinterpret_cast<ISP_CAM_AE_YGAMMA_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_YGAMMA_0))->Y_GMR3 = 0x40;
        reinterpret_cast<ISP_CAM_AE_YGAMMA_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_YGAMMA_0))->Y_GMR4 = 0x60;
        // CAM_AE_YGAMMA_1
        reinterpret_cast<ISP_CAM_AE_YGAMMA_1_T*>(REG_INFO_VALUE_PTR(CAM_AE_YGAMMA_1))->Y_GMR5 = 0x80;
        // CAM_AE_HST_SET
        reinterpret_cast<ISP_CAM_AE_HST_SET_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST_SET))->AE_HST0_BIN = rAEStatConfig.rAEHistWinCFG[0].uAEHistBin;
        reinterpret_cast<ISP_CAM_AE_HST_SET_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST_SET))->AE_HST1_BIN = rAEStatConfig.rAEHistWinCFG[1].uAEHistBin;
        reinterpret_cast<ISP_CAM_AE_HST_SET_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST_SET))->AE_HST2_BIN = rAEStatConfig.rAEHistWinCFG[2].uAEHistBin;
        reinterpret_cast<ISP_CAM_AE_HST_SET_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST_SET))->AE_HST3_BIN = rAEStatConfig.rAEHistWinCFG[3].uAEHistBin;
        reinterpret_cast<ISP_CAM_AE_HST_SET_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST_SET))->AE_HST0_COLOR = rAEStatConfig.rAEHistWinCFG[0].uAEHistOpt;
        reinterpret_cast<ISP_CAM_AE_HST_SET_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST_SET))->AE_HST1_COLOR = rAEStatConfig.rAEHistWinCFG[1].uAEHistOpt;
        reinterpret_cast<ISP_CAM_AE_HST_SET_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST_SET))->AE_HST2_COLOR = rAEStatConfig.rAEHistWinCFG[2].uAEHistOpt;
        reinterpret_cast<ISP_CAM_AE_HST_SET_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST_SET))->AE_HST3_COLOR = rAEStatConfig.rAEHistWinCFG[3].uAEHistOpt;
        // CAM_AE_HST0_RNG
        reinterpret_cast<ISP_CAM_AE_HST0_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST0_RNG))->AE_HST0_X_LOW = rAEStatConfig.rAEHistWinCFG[0].uAEHistXLow;
        reinterpret_cast<ISP_CAM_AE_HST0_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST0_RNG))->AE_HST0_X_HI = rAEStatConfig.rAEHistWinCFG[0].uAEHistXHi;
        reinterpret_cast<ISP_CAM_AE_HST0_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST0_RNG))->AE_HST0_Y_LOW = rAEStatConfig.rAEHistWinCFG[0].uAEHistYLow;
        reinterpret_cast<ISP_CAM_AE_HST0_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST0_RNG))->AE_HST0_Y_HI = rAEStatConfig.rAEHistWinCFG[0].uAEHistYHi;
        // CAM_AE_HST1_RNG
        reinterpret_cast<ISP_CAM_AE_HST1_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST1_RNG))->AE_HST1_X_LOW = rAEStatConfig.rAEHistWinCFG[1].uAEHistXLow;
        reinterpret_cast<ISP_CAM_AE_HST1_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST1_RNG))->AE_HST1_X_HI = rAEStatConfig.rAEHistWinCFG[1].uAEHistXHi;
        reinterpret_cast<ISP_CAM_AE_HST1_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST1_RNG))->AE_HST1_Y_LOW = rAEStatConfig.rAEHistWinCFG[1].uAEHistYLow;
        reinterpret_cast<ISP_CAM_AE_HST1_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST1_RNG))->AE_HST1_Y_HI = rAEStatConfig.rAEHistWinCFG[1].uAEHistYHi;
        // CAM_AE_HST2_RNG
        reinterpret_cast<ISP_CAM_AE_HST2_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST2_RNG))->AE_HST2_X_LOW = rAEStatConfig.rAEHistWinCFG[2].uAEHistXLow;
        reinterpret_cast<ISP_CAM_AE_HST2_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST2_RNG))->AE_HST2_X_HI = rAEStatConfig.rAEHistWinCFG[2].uAEHistXHi;
        reinterpret_cast<ISP_CAM_AE_HST2_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST2_RNG))->AE_HST2_Y_LOW = rAEStatConfig.rAEHistWinCFG[2].uAEHistYLow;
        reinterpret_cast<ISP_CAM_AE_HST2_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST2_RNG))->AE_HST2_Y_HI = rAEStatConfig.rAEHistWinCFG[2].uAEHistYHi;
        // CAM_AE_HST3_RNG
        reinterpret_cast<ISP_CAM_AE_HST3_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST3_RNG))->AE_HST3_X_LOW = rAEStatConfig.rAEHistWinCFG[3].uAEHistXLow;
        reinterpret_cast<ISP_CAM_AE_HST3_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST3_RNG))->AE_HST3_X_HI = rAEStatConfig.rAEHistWinCFG[3].uAEHistXHi;
        reinterpret_cast<ISP_CAM_AE_HST3_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST3_RNG))->AE_HST3_Y_LOW = rAEStatConfig.rAEHistWinCFG[3].uAEHistYLow;
        reinterpret_cast<ISP_CAM_AE_HST3_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_HST3_RNG))->AE_HST3_Y_HI = rAEStatConfig.rAEHistWinCFG[3].uAEHistYHi;

        m_bTG1Init = MTRUE;
        apply_TG1(i4SensorIndex);
    } else {
    m_rIspRegInfo[ERegInfo_CAM_AE_HST_CTL].val = rAEStatConfig.rAEHistWinCFG[0].bAEHistEn |
         // CAM_AE_D_HST_CTL
        reinterpret_cast<ISP_CAM_AE_D_HST_CTL_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST_CTL))->AE_HST0_EN = rAEStatConfig.rAEHistWinCFG[0].bAEHistEn;
        reinterpret_cast<ISP_CAM_AE_D_HST_CTL_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST_CTL))->AE_HST1_EN = rAEStatConfig.rAEHistWinCFG[1].bAEHistEn;
        reinterpret_cast<ISP_CAM_AE_D_HST_CTL_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST_CTL))->AE_HST2_EN = rAEStatConfig.rAEHistWinCFG[2].bAEHistEn;
        reinterpret_cast<ISP_CAM_AE_D_HST_CTL_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST_CTL))->AE_HST3_EN = rAEStatConfig.rAEHistWinCFG[3].bAEHistEn;
        // CAM_AE_D_GAIN2_0
        reinterpret_cast<ISP_CAM_AE_D_RAWPREGAIN2_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_GAIN2_0))->RAWPREGAIN2_R = m_rIspAEPreGain2.i4R;
        reinterpret_cast<ISP_CAM_AE_D_RAWPREGAIN2_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_GAIN2_0))->RAWPREGAIN2_G = m_rIspAEPreGain2.i4G;
        // CAM_REG_AE_D_GAIN2_1
        reinterpret_cast<ISP_CAM_AE_D_RAWPREGAIN2_1_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_GAIN2_1))->RAWPREGAIN2_B = m_rIspAEPreGain2.i4B;
        // CAM_AE_D_LMT2_0
        reinterpret_cast<ISP_CAM_AE_D_RAWLIMIT2_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_LMT2_0))->AE_LIMIT2_R = 0xFFF;
        reinterpret_cast<ISP_CAM_AE_D_RAWLIMIT2_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_LMT2_0))->AE_LIMIT2_G = 0xFFF;
        // CAM_AE_D_LMT2_1
        reinterpret_cast<ISP_CAM_AE_D_RAWLIMIT2_1_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_LMT2_1))->AE_LIMIT2_B = 0xFFF;
        // CAM_AE_D_RC_CNV_0
        reinterpret_cast<ISP_CAM_AE_D_MATRIX_COEF0_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_RC_CNV_0))->RC_CNV00 = 0x200;
        reinterpret_cast<ISP_CAM_AE_D_MATRIX_COEF0_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_RC_CNV_0))->RC_CNV01 = 0x000;
        // CAM_AE_D_RC_CNV_1
        reinterpret_cast<ISP_CAM_AE_D_MATRIX_COEF1_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_RC_CNV_1))->RC_CNV02 = 0x000;
        reinterpret_cast<ISP_CAM_AE_D_MATRIX_COEF1_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_RC_CNV_1))->RC_CNV10 = 0x000;
        // CAM_AE_D_RC_CNV_2
        reinterpret_cast<ISP_CAM_AE_D_MATRIX_COEF2_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_RC_CNV_2))->RC_CNV11 = 0x200;
        reinterpret_cast<ISP_CAM_AE_D_MATRIX_COEF2_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_RC_CNV_2))->RC_CNV12 = 0x000;
        // CAM_AE_D_RC_CNV_3
        reinterpret_cast<ISP_CAM_AE_D_MATRIX_COEF3_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_RC_CNV_3))->RC_CNV20 = 0x000;
        reinterpret_cast<ISP_CAM_AE_D_MATRIX_COEF3_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_RC_CNV_3))->RC_CNV21 = 0x000;
        // CAM_AE_D_RC_CNV_4
        reinterpret_cast<ISP_CAM_AE_D_MATRIX_COEF4_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_RC_CNV_4))->RC_CNV22 = 0x200;
        reinterpret_cast<ISP_CAM_AE_D_MATRIX_COEF4_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_RC_CNV_4))->AE_RC_ACC = 0x09;
        // CAM_AE_D_YGAMMA_0
        reinterpret_cast<ISP_CAM_AE_D_YGAMMA_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_YGAMMA_0))->Y_GMR1 = 0x10;
        reinterpret_cast<ISP_CAM_AE_D_YGAMMA_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_YGAMMA_0))->Y_GMR2 = 0x20;
        reinterpret_cast<ISP_CAM_AE_D_YGAMMA_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_YGAMMA_0))->Y_GMR3 = 0x40;
        reinterpret_cast<ISP_CAM_AE_D_YGAMMA_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_YGAMMA_0))->Y_GMR4 = 0x60;
        // CAM_AE_D_YGAMMA_1
        reinterpret_cast<ISP_CAM_AE_D_YGAMMA_1_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_YGAMMA_1))->Y_GMR5 = 0x80;
        // CAM_AE_D_HST_SET
        reinterpret_cast<ISP_CAM_AE_D_HST_SET_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST_SET))->AE_HST0_BIN = rAEStatConfig.rAEHistWinCFG[0].uAEHistBin;
        reinterpret_cast<ISP_CAM_AE_D_HST_SET_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST_SET))->AE_HST1_BIN = rAEStatConfig.rAEHistWinCFG[1].uAEHistBin;
        reinterpret_cast<ISP_CAM_AE_D_HST_SET_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST_SET))->AE_HST2_BIN = rAEStatConfig.rAEHistWinCFG[2].uAEHistBin;
        reinterpret_cast<ISP_CAM_AE_D_HST_SET_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST_SET))->AE_HST3_BIN = rAEStatConfig.rAEHistWinCFG[3].uAEHistBin;
        reinterpret_cast<ISP_CAM_AE_D_HST_SET_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST_SET))->AE_HST0_COLOR = rAEStatConfig.rAEHistWinCFG[0].uAEHistOpt;
        reinterpret_cast<ISP_CAM_AE_D_HST_SET_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST_SET))->AE_HST1_COLOR = rAEStatConfig.rAEHistWinCFG[1].uAEHistOpt;
        reinterpret_cast<ISP_CAM_AE_D_HST_SET_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST_SET))->AE_HST2_COLOR = rAEStatConfig.rAEHistWinCFG[2].uAEHistOpt;
        reinterpret_cast<ISP_CAM_AE_D_HST_SET_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST_SET))->AE_HST3_COLOR = rAEStatConfig.rAEHistWinCFG[3].uAEHistOpt;
        // CAM_AE_D_HST0_RNG
        reinterpret_cast<ISP_CAM_AE_D_HST0_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST0_RNG))->AE_HST0_X_LOW = rAEStatConfig.rAEHistWinCFG[0].uAEHistXLow;
        reinterpret_cast<ISP_CAM_AE_D_HST0_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST0_RNG))->AE_HST0_X_HI = rAEStatConfig.rAEHistWinCFG[0].uAEHistXHi;
        reinterpret_cast<ISP_CAM_AE_D_HST0_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST0_RNG))->AE_HST0_Y_LOW = rAEStatConfig.rAEHistWinCFG[0].uAEHistYLow;
        reinterpret_cast<ISP_CAM_AE_D_HST0_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST0_RNG))->AE_HST0_Y_HI = rAEStatConfig.rAEHistWinCFG[0].uAEHistYHi;
        // CAM_AE_D_HST1_RNG
        reinterpret_cast<ISP_CAM_AE_D_HST1_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST1_RNG))->AE_HST1_X_LOW = rAEStatConfig.rAEHistWinCFG[1].uAEHistXLow;
        reinterpret_cast<ISP_CAM_AE_D_HST1_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST1_RNG))->AE_HST1_X_HI = rAEStatConfig.rAEHistWinCFG[1].uAEHistXHi;
        reinterpret_cast<ISP_CAM_AE_D_HST1_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST1_RNG))->AE_HST1_Y_LOW = rAEStatConfig.rAEHistWinCFG[1].uAEHistYLow;
        reinterpret_cast<ISP_CAM_AE_D_HST1_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST1_RNG))->AE_HST1_Y_HI = rAEStatConfig.rAEHistWinCFG[1].uAEHistYHi;
        // CAM_AE_D_HST2_RNG
        reinterpret_cast<ISP_CAM_AE_D_HST2_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST2_RNG))->AE_HST2_X_LOW = rAEStatConfig.rAEHistWinCFG[2].uAEHistXLow;
        reinterpret_cast<ISP_CAM_AE_D_HST2_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST2_RNG))->AE_HST2_X_HI = rAEStatConfig.rAEHistWinCFG[2].uAEHistXHi;
        reinterpret_cast<ISP_CAM_AE_D_HST2_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST2_RNG))->AE_HST2_Y_LOW = rAEStatConfig.rAEHistWinCFG[2].uAEHistYLow;
        reinterpret_cast<ISP_CAM_AE_D_HST2_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST2_RNG))->AE_HST2_Y_HI = rAEStatConfig.rAEHistWinCFG[2].uAEHistYHi;
        // CAM_AE_D_HST3_RNG
        reinterpret_cast<ISP_CAM_AE_D_HST3_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST3_RNG))->AE_HST3_X_LOW = rAEStatConfig.rAEHistWinCFG[3].uAEHistXLow;
        reinterpret_cast<ISP_CAM_AE_D_HST3_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST3_RNG))->AE_HST3_X_HI = rAEStatConfig.rAEHistWinCFG[3].uAEHistXHi;
        reinterpret_cast<ISP_CAM_AE_D_HST3_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST3_RNG))->AE_HST3_Y_LOW = rAEStatConfig.rAEHistWinCFG[3].uAEHistYLow;
        reinterpret_cast<ISP_CAM_AE_D_HST3_RNG_T*>(REG_INFO_VALUE_PTR(CAM_AE_D_HST3_RNG))->AE_HST3_Y_HI = rAEStatConfig.rAEHistWinCFG[3].uAEHistYHi;

        m_bTG2Init = MTRUE;
        apply_TG2(i4SensorIndex);
    }

    addressErrorCheck("After ISP_MGR_AWB_STAT_CONFIG_T::apply()");

    return MTRUE;
}

MBOOL
ISP_MGR_AE_STAT_CONFIG_T::
apply_TG1(MINT32 i4SensorIndex)
{
    CAM_LOGD("%s(): m_eSensorDev = %d, i4SensorIndex = %d\n", __FUNCTION__, m_eSensorDev, i4SensorIndex);

    MUINTPTR handle;

    if(m_bTG1Init == MFALSE) {
        CAM_LOGD("%s(): AE don't configure TG1\n", __FUNCTION__);
        return MTRUE;
    }

    INormalPipe* pPipe = INormalPipe::createInstance(i4SensorIndex,"isp_mgr_ae_stat_tg1");//iopipe2.0

    // get module handle
    if (MFALSE == pPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_GET_MODULE_HANDLE,
                           NSImageio::NSIspio::EModule_AE, (MINTPTR)&handle, (MINTPTR)(&("isp_mgr_ae_stat_tg1")))) {
        //Error Handling
        CAM_LOGE("EPIPECmd_GET_MODULE_HANDLE fail");
        goto lbExit;
    }
    // set module register
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_HST_CTL, m_rIspRegInfo[ERegInfo_CAM_AE_HST_CTL].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_GAIN2_0, m_rIspRegInfo[ERegInfo_CAM_AE_GAIN2_0].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_GAIN2_1, m_rIspRegInfo[ERegInfo_CAM_AE_GAIN2_1].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_LMT2_0, m_rIspRegInfo[ERegInfo_CAM_AE_LMT2_0].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_LMT2_1, m_rIspRegInfo[ERegInfo_CAM_AE_LMT2_1].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_RC_CNV_0, m_rIspRegInfo[ERegInfo_CAM_AE_RC_CNV_0].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_RC_CNV_1, m_rIspRegInfo[ERegInfo_CAM_AE_RC_CNV_1].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_RC_CNV_2, m_rIspRegInfo[ERegInfo_CAM_AE_RC_CNV_2].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_RC_CNV_3, m_rIspRegInfo[ERegInfo_CAM_AE_RC_CNV_3].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_RC_CNV_4, m_rIspRegInfo[ERegInfo_CAM_AE_RC_CNV_4].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_YGAMMA_0, m_rIspRegInfo[ERegInfo_CAM_AE_YGAMMA_0].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_YGAMMA_1, m_rIspRegInfo[ERegInfo_CAM_AE_YGAMMA_1].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_HST_SET, m_rIspRegInfo[ERegInfo_CAM_AE_HST_SET].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_HST0_RNG, m_rIspRegInfo[ERegInfo_CAM_AE_HST0_RNG].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_HST1_RNG, m_rIspRegInfo[ERegInfo_CAM_AE_HST1_RNG].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_HST2_RNG, m_rIspRegInfo[ERegInfo_CAM_AE_HST2_RNG].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_HST3_RNG, m_rIspRegInfo[ERegInfo_CAM_AE_HST3_RNG].val);

    // set module config done
    if (MFALSE==pPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_SET_MODULE_CFG_DONE, handle, MNULL, MNULL)) {
        //Error Handling
        CAM_LOGE("EPIPECmd_SET_MODULE_CFG_DONE fail");
        goto lbExit;
    }

lbExit:

    // release handle
    if (MFALSE==pPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_RELEASE_MODULE_HANDLE, handle, (MINTPTR)(&("isp_mgr_ae_stat_tg1")), MNULL)) {
        // Error Handling
        CAM_LOGE("EPIPECmd_SET_MODULE_CFG_DONE fail");
    }

    pPipe->destroyInstance("isp_mgr_ae_stat_tg1");

    return MTRUE;
}

MBOOL
ISP_MGR_AE_STAT_CONFIG_T::
apply_TG2(MINT32 i4SensorIndex)
{
    CAM_LOGD("%s(): m_eSensorDev = %d, i4SensorIndex = %d\n", __FUNCTION__, m_eSensorDev, i4SensorIndex);

    MUINTPTR handle;

    if(m_bTG2Init == MFALSE) {
        CAM_LOGD("%s(): AE don't configure TG2\n", __FUNCTION__);
        return MTRUE;
    }

    INormalPipe* pPipe = INormalPipe::createInstance(i4SensorIndex,"isp_mgr_ae_stat_tg2");//iopipe2.0

    // get module handle
    if (MFALSE == pPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_GET_MODULE_HANDLE,
                           NSImageio::NSIspio::EModule_AE_D, (MINTPTR)&handle, (MINTPTR)(&("isp_mgr_ae_stat_tg2")))) {
        //Error Handling
        CAM_LOGE("EPIPECmd_GET_MODULE_HANDLE fail");
        goto lbExit;
    }

    // set module register
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_D_HST_CTL, m_rIspRegInfo[ERegInfo_CAM_AE_D_HST_CTL].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_D_GAIN2_0, m_rIspRegInfo[ERegInfo_CAM_AE_D_GAIN2_0].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_D_GAIN2_1, m_rIspRegInfo[ERegInfo_CAM_AE_D_GAIN2_1].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_D_LMT2_0, m_rIspRegInfo[ERegInfo_CAM_AE_D_LMT2_0].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_D_LMT2_1, m_rIspRegInfo[ERegInfo_CAM_AE_D_LMT2_1].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_D_RC_CNV_0, m_rIspRegInfo[ERegInfo_CAM_AE_D_RC_CNV_0].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_D_RC_CNV_1, m_rIspRegInfo[ERegInfo_CAM_AE_D_RC_CNV_1].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_D_RC_CNV_2, m_rIspRegInfo[ERegInfo_CAM_AE_D_RC_CNV_2].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_D_RC_CNV_3, m_rIspRegInfo[ERegInfo_CAM_AE_D_RC_CNV_3].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_D_RC_CNV_4, m_rIspRegInfo[ERegInfo_CAM_AE_D_RC_CNV_4].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_D_YGAMMA_0, m_rIspRegInfo[ERegInfo_CAM_AE_D_YGAMMA_0].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_D_YGAMMA_1, m_rIspRegInfo[ERegInfo_CAM_AE_D_YGAMMA_1].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_D_HST_SET, m_rIspRegInfo[ERegInfo_CAM_AE_D_HST_SET].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_D_HST0_RNG, m_rIspRegInfo[ERegInfo_CAM_AE_D_HST0_RNG].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_D_HST1_RNG, m_rIspRegInfo[ERegInfo_CAM_AE_D_HST1_RNG].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_D_HST2_RNG, m_rIspRegInfo[ERegInfo_CAM_AE_D_HST2_RNG].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_D_HST3_RNG, m_rIspRegInfo[ERegInfo_CAM_AE_D_HST3_RNG].val);

    // set module config done
    if (MFALSE==pPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_SET_MODULE_CFG_DONE, handle, MNULL, MNULL)) {
        //Error Handling
        CAM_LOGE("EPIPECmd_SET_MODULE_CFG_DONE fail");
        goto lbExit;
    }

lbExit:

    // release handle
    if (MFALSE==pPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_RELEASE_MODULE_HANDLE, handle, (MINTPTR)(&("isp_mgr_ae_stat_tg2")), MNULL)) {
        // Error Handling
        CAM_LOGE("EPIPECmd_SET_MODULE_CFG_DONE fail");
    }

    pPipe->destroyInstance("isp_mgr_ae_stat_tg2");

    return MTRUE;
}
#endif

}
