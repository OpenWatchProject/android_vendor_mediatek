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
#define LOG_TAG "ae_hw_config"

#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG       (1)
#endif

#include <aaa_types.h>

#include <mtkcam/utils/std/Log.h>

#include <ae_param.h>
#include <ae_mgr.h>
#include <ae_config.h>

#include <drv/isp_reg.h>

using namespace NS3Av3;

#define REG_AE_INFO(REG, VALUE) \
    { \
        pAERegInfo[ERegInfo_##REG].addr = ((MUINT32)offsetof(cam_reg_t, REG)); \
        pAERegInfo[ERegInfo_##REG].val  = VALUE; \
    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AF Statistics Config
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_AE_CONFIG_T&
ISP_AE_CONFIG_T::
getInstance(MINT32 const eSensorDev)
{
    switch (eSensorDev)
    {
    case ESensorDev_Main: //  Main Sensor
        return  ISP_AE_CONFIG_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_AE_CONFIG_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_AE_CONFIG_DEV<ESensorDev_Sub>::getInstance();
    case ESensorDev_SubSecond: //  Sub Second Sensor
        return  ISP_AE_CONFIG_DEV<ESensorDev_SubSecond>::getInstance();
    default:
        CAM_LOGE("eSensorDev = %d", eSensorDev);
        return  ISP_AE_CONFIG_DEV<ESensorDev_Main>::getInstance();
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT ISP_AE_CONFIG_T::AEConfig(AE_STAT_PARAM_T *a_sAEConfig, AE_CONFIG_INFO_T *a_sInputAEInfo, AEResultConfig_T *p_sAERegInfo)
{
    AERegInfo_T *pAERegInfo = p_sAERegInfo->rAERegInfo;
    MUINT32 BlockNumW, BlockNumH;

    if(a_sInputAEInfo->u4BlockNumW <= 120) {
        BlockNumW = a_sInputAEInfo->u4BlockNumW;
    } else {
        CAM_LOGE("Wrong  a_sInputAEInfo.u4BlockNumW = %d", a_sInputAEInfo->u4BlockNumW);
        BlockNumW = 120;
    }

    if(a_sInputAEInfo->u4BlockNumH <= 90) {
        BlockNumH = a_sInputAEInfo->u4BlockNumH;
    } else {
        CAM_LOGE("Wrong  a_sInputAEInfo.u4BlockNumH = %d", a_sInputAEInfo->u4BlockNumH);
        BlockNumH = 90;
    }
#if 0
    // CAM_AE_GAIN2_0
    CAM_REG_AE_GAIN2_0 reg_ae_Gain20;
    reg_ae_Gain20.Raw = 0;
    reg_ae_Gain20.Bits.AE_GAIN2_G = a_sInputAEInfo->i4G_IspAEPreGain2;
    reg_ae_Gain20.Bits.AE_GAIN2_R = a_sInputAEInfo->i4R_IspAEPreGain2;

    CAM_LOGD(" a_sInputAEInfo AE_GAIN2_G(%d) AE_GAIN2_R(%d)\n", a_sInputAEInfo->i4G_IspAEPreGain2, a_sInputAEInfo->i4R_IspAEPreGain2);
    CAM_LOGD("AE_GAIN2_G(%d) AE_GAIN2_R(%d) Raw(%d)\n", reg_ae_Gain20.Bits.AE_GAIN2_G, reg_ae_Gain20.Bits.AE_GAIN2_R, (MUINT32)reg_ae_Gain20.Raw);

    // CAM_REG_AE_GAIN2_1
    CAM_REG_AE_GAIN2_1 reg_ae_Gain21;
    reg_ae_Gain21.Raw = 0;
    reg_ae_Gain21.Bits.AE_GAIN2_B = a_sInputAEInfo->i4B_IspAEPreGain2;
#endif
    REG_AA_R1_AA_AE_LMT2_0 reg_ae_Lmt20;
    REG_AA_R1_AA_AE_LMT2_1 reg_ae_Lmt21;
    reg_ae_Lmt20.Raw= 0;
    reg_ae_Lmt21.Raw= 0;

    if (a_sInputAEInfo->u4AAOmode){
        // CAM_AE_LMT2_0
        reg_ae_Lmt20.Bits.AA_AE_LMT2_G = 0x3FFF;
        reg_ae_Lmt20.Bits.AA_AE_LMT2_R = 0x3FFF;
        // CAM_AE_LMT2_1
        reg_ae_Lmt21.Bits.AA_AE_LMT2_B = 0x3FFF;
    }else{
        // CAM_AE_LMT2_0
        reg_ae_Lmt20.Bits.AA_AE_LMT2_G = 0xFFF;
        reg_ae_Lmt20.Bits.AA_AE_LMT2_R = 0xFFF;
        // CAM_AE_LMT2_1
        reg_ae_Lmt21.Bits.AA_AE_LMT2_B = 0xFFF;
    }

    // CAM_AE_RC_CNV_0
    REG_AA_R1_AA_AE_RC_CNV_0 reg_ae_RcCnv0;
    reg_ae_RcCnv0.Raw = 0;
    reg_ae_RcCnv0.Bits.AA_AE_RC_CNV00 = 0x200;
    reg_ae_RcCnv0.Bits.AA_AE_RC_CNV01 = 0x000;

    // CAM_AE_RC_CNV_1
    REG_AA_R1_AA_AE_RC_CNV_1 reg_ae_RcCnv1;
    reg_ae_RcCnv1.Raw = 0;
    reg_ae_RcCnv1.Bits.AA_AE_RC_CNV02 = 0x000;
    reg_ae_RcCnv1.Bits.AA_AE_RC_CNV10 = 0x000;

    // CAM_AE_RC_CNV_2
    REG_AA_R1_AA_AE_RC_CNV_2 reg_ae_RcCnv2;
    reg_ae_RcCnv2.Raw = 0;
    reg_ae_RcCnv2.Bits.AA_AE_RC_CNV11 = 0x200;
    reg_ae_RcCnv2.Bits.AA_AE_RC_CNV12 = 0x000;

    // CAM_AE_RC_CNV_3
    REG_AA_R1_AA_AE_RC_CNV_3 reg_ae_RcCnv3;
    reg_ae_RcCnv3.Raw = 0;
    reg_ae_RcCnv3.Bits.AA_AE_RC_CNV20 = 0x000;
    reg_ae_RcCnv3.Bits.AA_AE_RC_CNV21 = 0x000;

    // CAM_AE_RC_CNV_4
    REG_AA_R1_AA_AE_RC_CNV_4 reg_ae_RcCnv4;
    reg_ae_RcCnv4.Raw = 0;
    reg_ae_RcCnv4.Bits.AA_AE_RC_CNV22 = 0x200;
    reg_ae_RcCnv4.Bits.AA_AE_RC_ACC   = 0x09;

    // CAM_AE_OVER_EXPO_CFG
    REG_AA_R1_AA_AE_OVER_EXPO_CFG reg_ae_OverExpoCfg;
    reg_ae_OverExpoCfg.Raw = 0;
    reg_ae_OverExpoCfg.Bits.AA_AE_OVER_EXPO_THR = a_sInputAEInfo->u4AEOverExpCntThr;
    //reg_ae_OverExpoCfg.Bits.AE_OVER_EXPO_SFT = a_sInputAEInfo->u4AEOverExpCntShift;

    // CAM_AE_PIX_HST_CTL
    REG_AA_R1_AA_AE_PIX_HST_CTL reg_ae_PixHstCtl;
    reg_ae_PixHstCtl.Raw = 0;
    reg_ae_PixHstCtl.Bits.AA_AE_PIX_SE_HST0_EN = reg_ae_PixHstCtl.Bits.AA_AE_PIX_HST0_EN = a_sAEConfig->rAEPixelHistWinCFG[0].bAEHistEn;
    reg_ae_PixHstCtl.Bits.AA_AE_PIX_SE_HST1_EN = reg_ae_PixHstCtl.Bits.AA_AE_PIX_HST1_EN = a_sAEConfig->rAEPixelHistWinCFG[1].bAEHistEn;
    reg_ae_PixHstCtl.Bits.AA_AE_PIX_SE_HST2_EN = reg_ae_PixHstCtl.Bits.AA_AE_PIX_HST2_EN = a_sAEConfig->rAEPixelHistWinCFG[2].bAEHistEn;
    reg_ae_PixHstCtl.Bits.AA_AE_PIX_SE_HST3_EN = reg_ae_PixHstCtl.Bits.AA_AE_PIX_HST3_EN = a_sAEConfig->rAEPixelHistWinCFG[3].bAEHistEn;
    reg_ae_PixHstCtl.Bits.AA_AE_PIX_SE_HST4_EN = reg_ae_PixHstCtl.Bits.AA_AE_PIX_HST4_EN = a_sAEConfig->rAEPixelHistWinCFG[4].bAEHistEn;
    reg_ae_PixHstCtl.Bits.AA_AE_PIX_SE_HST5_EN = reg_ae_PixHstCtl.Bits.AA_AE_PIX_HST5_EN = a_sAEConfig->rAEPixelHistWinCFG[5].bAEHistEn;
    CAM_LOGD("[%s] CAM_AE_PIX_HST_CTL : %d/%d/%d/%d/%d/%d", __FUNCTION__,
                reg_ae_PixHstCtl.Bits.AA_AE_PIX_HST0_EN,
                reg_ae_PixHstCtl.Bits.AA_AE_PIX_HST1_EN,
                reg_ae_PixHstCtl.Bits.AA_AE_PIX_HST2_EN,
                reg_ae_PixHstCtl.Bits.AA_AE_PIX_HST3_EN,
                reg_ae_PixHstCtl.Bits.AA_AE_PIX_HST4_EN,
                reg_ae_PixHstCtl.Bits.AA_AE_PIX_HST5_EN);

    // CAM_AE_PIX_HST_SET
    REG_AA_R1_AA_AE_PIX_HST_SET reg_ae_PixHstSet;
    reg_ae_PixHstSet.Raw = 0;
    reg_ae_PixHstSet.Bits.AA_AE_PIX_BIN_MODE_0 = a_sAEConfig->rAEPixelHistWinCFG[0].uAEHistBin;
    reg_ae_PixHstSet.Bits.AA_AE_PIX_BIN_MODE_1 = a_sAEConfig->rAEPixelHistWinCFG[1].uAEHistBin;
    reg_ae_PixHstSet.Bits.AA_AE_PIX_BIN_MODE_2 = a_sAEConfig->rAEPixelHistWinCFG[2].uAEHistBin;
    reg_ae_PixHstSet.Bits.AA_AE_PIX_BIN_MODE_3 = a_sAEConfig->rAEPixelHistWinCFG[3].uAEHistBin;
    reg_ae_PixHstSet.Bits.AA_AE_PIX_COLOR_MODE_0 = a_sAEConfig->rAEPixelHistWinCFG[0].uAEHistOpt;
    reg_ae_PixHstSet.Bits.AA_AE_PIX_COLOR_MODE_1 = a_sAEConfig->rAEPixelHistWinCFG[1].uAEHistOpt;
    reg_ae_PixHstSet.Bits.AA_AE_PIX_COLOR_MODE_2 = a_sAEConfig->rAEPixelHistWinCFG[2].uAEHistOpt;
    reg_ae_PixHstSet.Bits.AA_AE_PIX_COLOR_MODE_3 = a_sAEConfig->rAEPixelHistWinCFG[3].uAEHistOpt;

    // CAM_AE_PIX_HST_SET_1
    REG_AA_R1_AA_AE_PIX_HST_SET_1 reg_ae_PixHstSet1;
    reg_ae_PixHstSet1.Raw = 0;
    reg_ae_PixHstSet1.Bits.AA_AE_PIX_BIN_MODE_4 = a_sAEConfig->rAEPixelHistWinCFG[4].uAEHistBin;
    reg_ae_PixHstSet1.Bits.AA_AE_PIX_BIN_MODE_5 = a_sAEConfig->rAEPixelHistWinCFG[5].uAEHistBin;
    reg_ae_PixHstSet1.Bits.AA_AE_PIX_COLOR_MODE_4 = a_sAEConfig->rAEPixelHistWinCFG[4].uAEHistOpt;
    reg_ae_PixHstSet1.Bits.AA_AE_PIX_COLOR_MODE_5 = a_sAEConfig->rAEPixelHistWinCFG[5].uAEHistOpt;
    CAM_LOGD("[%s] CAM_AE_PIX_HST_SET_1 : BIN (%d/%d) COLOR (%d/%d)", __FUNCTION__,
                reg_ae_PixHstSet1.Bits.AA_AE_PIX_BIN_MODE_4,
                reg_ae_PixHstSet1.Bits.AA_AE_PIX_BIN_MODE_5,
                reg_ae_PixHstSet1.Bits.AA_AE_PIX_COLOR_MODE_4,
                reg_ae_PixHstSet1.Bits.AA_AE_PIX_COLOR_MODE_5);

    // CAM_AE_PIX_HST0_YRNG
    REG_AA_R1_AA_AE_PIX_HST0_YRNG reg_ae_PixHst0Yrng;
    reg_ae_PixHst0Yrng.Raw = 0;
    reg_ae_PixHst0Yrng.Bits.AA_AE_PIX_Y_LOW_0 = a_sAEConfig->rAEPixelHistWinCFG[0].uAEHistYLow;
    reg_ae_PixHst0Yrng.Bits.AA_AE_PIX_Y_HI_0  = a_sAEConfig->rAEPixelHistWinCFG[0].uAEHistYHi;

    // CAM_AE_PIX_HST0_XRNG
    REG_AA_R1_AA_AE_PIX_HST0_XRNG reg_ae_PixHst0Xrng;
    reg_ae_PixHst0Xrng.Raw = 0;
    reg_ae_PixHst0Xrng.Bits.AA_AE_PIX_X_LOW_0 = a_sAEConfig->rAEPixelHistWinCFG[0].uAEHistXLow;
    reg_ae_PixHst0Xrng.Bits.AA_AE_PIX_X_HI_0  = a_sAEConfig->rAEPixelHistWinCFG[0].uAEHistXHi;

    // CAM_AE_PIX_HST1_YRNG
    REG_AA_R1_AA_AE_PIX_HST1_YRNG reg_ae_PixHst1Yrng;
    reg_ae_PixHst1Yrng.Raw = 0;
    reg_ae_PixHst1Yrng.Bits.AA_AE_PIX_Y_LOW_1 = a_sAEConfig->rAEPixelHistWinCFG[1].uAEHistYLow;
    reg_ae_PixHst1Yrng.Bits.AA_AE_PIX_Y_HI_1  = a_sAEConfig->rAEPixelHistWinCFG[1].uAEHistYHi;

    // CAM_AE_PIX_HST1_XRNG
    REG_AA_R1_AA_AE_PIX_HST1_XRNG reg_ae_PixHst1Xrng;
    reg_ae_PixHst1Xrng.Raw = 0;
    reg_ae_PixHst1Xrng.Bits.AA_AE_PIX_X_LOW_1 = a_sAEConfig->rAEPixelHistWinCFG[1].uAEHistXLow;
    reg_ae_PixHst1Xrng.Bits.AA_AE_PIX_X_HI_1  = a_sAEConfig->rAEPixelHistWinCFG[1].uAEHistXHi;

    // CAM_AE_PIX_HST2_YRNG
    REG_AA_R1_AA_AE_PIX_HST2_YRNG reg_ae_PixHst2Yrng;
    reg_ae_PixHst2Yrng.Raw = 0;
    reg_ae_PixHst2Yrng.Bits.AA_AE_PIX_Y_LOW_2 = a_sAEConfig->rAEPixelHistWinCFG[2].uAEHistYLow;
    reg_ae_PixHst2Yrng.Bits.AA_AE_PIX_Y_HI_2  = a_sAEConfig->rAEPixelHistWinCFG[2].uAEHistYHi;

    // CAM_AE_PIX_HST2_XRNG
    REG_AA_R1_AA_AE_PIX_HST2_XRNG reg_ae_PixHst2Xrng;
    reg_ae_PixHst2Xrng.Raw = 0;
    reg_ae_PixHst2Xrng.Bits.AA_AE_PIX_X_LOW_2 = a_sAEConfig->rAEPixelHistWinCFG[2].uAEHistXLow;
    reg_ae_PixHst2Xrng.Bits.AA_AE_PIX_X_HI_2  = a_sAEConfig->rAEPixelHistWinCFG[2].uAEHistXHi;

    // CAM_AE_PIX_HST3_YRNG
    REG_AA_R1_AA_AE_PIX_HST3_YRNG reg_ae_PixHst3Yrng;
    reg_ae_PixHst3Yrng.Raw = 0;
    reg_ae_PixHst3Yrng.Bits.AA_AE_PIX_Y_LOW_3 = a_sAEConfig->rAEPixelHistWinCFG[3].uAEHistYLow;
    reg_ae_PixHst3Yrng.Bits.AA_AE_PIX_Y_HI_3  = a_sAEConfig->rAEPixelHistWinCFG[3].uAEHistYHi;

    // CAM_AE_PIX_HST3_XRNG
    REG_AA_R1_AA_AE_PIX_HST3_XRNG reg_ae_PixHst3Xrng;
    reg_ae_PixHst3Xrng.Raw = 0;
    reg_ae_PixHst3Xrng.Bits.AA_AE_PIX_X_LOW_3 = a_sAEConfig->rAEPixelHistWinCFG[3].uAEHistXLow;
    reg_ae_PixHst3Xrng.Bits.AA_AE_PIX_X_HI_3  = a_sAEConfig->rAEPixelHistWinCFG[3].uAEHistXHi;

    // CAM_AE_PIX_HST4_YRNG
    REG_AA_R1_AA_AE_PIX_HST4_YRNG reg_ae_PixHst4Yrng;
    reg_ae_PixHst4Yrng.Raw = 0;
    reg_ae_PixHst4Yrng.Bits.AA_AE_PIX_Y_LOW_4 = a_sAEConfig->rAEPixelHistWinCFG[4].uAEHistYLow;
    reg_ae_PixHst4Yrng.Bits.AA_AE_PIX_Y_HI_4  = a_sAEConfig->rAEPixelHistWinCFG[4].uAEHistYHi;

    // CAM_AE_PIX_HST4_XRNG
    REG_AA_R1_AA_AE_PIX_HST4_XRNG reg_ae_PixHst4Xrng;
    reg_ae_PixHst4Xrng.Raw = 0;
    reg_ae_PixHst4Xrng.Bits.AA_AE_PIX_X_LOW_4 = a_sAEConfig->rAEPixelHistWinCFG[4].uAEHistXLow;
    reg_ae_PixHst4Xrng.Bits.AA_AE_PIX_X_HI_4  = a_sAEConfig->rAEPixelHistWinCFG[4].uAEHistXHi;

    CAM_LOGD("[%s] CAM_AE_PIX_HST4 : Y-LOW/HIGH (%d/%d) X-LOW/HIGH (%d/%d)", __FUNCTION__,
                reg_ae_PixHst4Yrng.Bits.AA_AE_PIX_Y_LOW_4,
                reg_ae_PixHst4Yrng.Bits.AA_AE_PIX_Y_HI_4,
                reg_ae_PixHst4Xrng.Bits.AA_AE_PIX_X_LOW_4,
                reg_ae_PixHst4Xrng.Bits.AA_AE_PIX_X_HI_4);

    // CAM_AE_PIX_HST5_YRNG
    REG_AA_R1_AA_AE_PIX_HST5_YRNG reg_ae_PixHst5Yrng;
    reg_ae_PixHst5Yrng.Raw = 0;
    reg_ae_PixHst5Yrng.Bits.AA_AE_PIX_Y_LOW_5 = a_sAEConfig->rAEPixelHistWinCFG[5].uAEHistYLow;
    reg_ae_PixHst5Yrng.Bits.AA_AE_PIX_Y_HI_5  = a_sAEConfig->rAEPixelHistWinCFG[5].uAEHistYHi;

    // CAM_AE_PIX_HST5_XRNG
    REG_AA_R1_AA_AE_PIX_HST5_XRNG reg_ae_PixHst5Xrng;
    reg_ae_PixHst5Xrng.Raw = 0;
    reg_ae_PixHst5Xrng.Bits.AA_AE_PIX_X_LOW_5 = a_sAEConfig->rAEPixelHistWinCFG[5].uAEHistXLow;
    reg_ae_PixHst5Xrng.Bits.AA_AE_PIX_X_HI_5  = a_sAEConfig->rAEPixelHistWinCFG[5].uAEHistXHi;

    CAM_LOGD("[%s] CAM_AE_PIX_HST5 : Y-LOW/HIGH (%d/%d) X-LOW/HIGH (%d/%d)", __FUNCTION__,
                reg_ae_PixHst5Yrng.Bits.AA_AE_PIX_Y_LOW_5,
                reg_ae_PixHst5Yrng.Bits.AA_AE_PIX_Y_HI_5,
                reg_ae_PixHst5Xrng.Bits.AA_AE_PIX_X_LOW_5,
                reg_ae_PixHst5Xrng.Bits.AA_AE_PIX_X_HI_5);
 
    // CAM_AE_PIX_SE_HST_SET
    REG_AA_R1_AA_AE_PIX_SE_HST_SET reg_ae_se_PixHstSet;
    reg_ae_se_PixHstSet.Raw = 0;
    reg_ae_se_PixHstSet.Bits.AA_AE_PIX_SE_BIN_MODE_0 = a_sAEConfig->rAEPixelHistWinCFG[0].uAEHistBin;
    reg_ae_se_PixHstSet.Bits.AA_AE_PIX_SE_BIN_MODE_1 = a_sAEConfig->rAEPixelHistWinCFG[1].uAEHistBin;
    reg_ae_se_PixHstSet.Bits.AA_AE_PIX_SE_BIN_MODE_2 = a_sAEConfig->rAEPixelHistWinCFG[2].uAEHistBin;
    reg_ae_se_PixHstSet.Bits.AA_AE_PIX_SE_BIN_MODE_3 = a_sAEConfig->rAEPixelHistWinCFG[3].uAEHistBin;
    reg_ae_se_PixHstSet.Bits.AA_AE_PIX_SE_COLOR_MODE_0 = a_sAEConfig->rAEPixelHistWinCFG[0].uAEHistOpt;
    reg_ae_se_PixHstSet.Bits.AA_AE_PIX_SE_COLOR_MODE_1 = a_sAEConfig->rAEPixelHistWinCFG[1].uAEHistOpt;
    reg_ae_se_PixHstSet.Bits.AA_AE_PIX_SE_COLOR_MODE_2 = a_sAEConfig->rAEPixelHistWinCFG[2].uAEHistOpt;
    reg_ae_se_PixHstSet.Bits.AA_AE_PIX_SE_COLOR_MODE_3 = a_sAEConfig->rAEPixelHistWinCFG[3].uAEHistOpt;

    // CAM_AE_PIX_SE_HST_SET_1
    REG_AA_R1_AA_AE_PIX_SE_HST_SET_1 reg_ae_se_PixHstSet1;
    reg_ae_se_PixHstSet1.Raw = 0;
    reg_ae_se_PixHstSet1.Bits.AA_AE_PIX_SE_BIN_MODE_4 = a_sAEConfig->rAEPixelHistWinCFG[4].uAEHistBin;
    reg_ae_se_PixHstSet1.Bits.AA_AE_PIX_SE_BIN_MODE_5 = a_sAEConfig->rAEPixelHistWinCFG[5].uAEHistBin;
    reg_ae_se_PixHstSet1.Bits.AA_AE_PIX_SE_COLOR_MODE_4 = a_sAEConfig->rAEPixelHistWinCFG[4].uAEHistOpt;
    reg_ae_se_PixHstSet1.Bits.AA_AE_PIX_SE_COLOR_MODE_5 = a_sAEConfig->rAEPixelHistWinCFG[5].uAEHistOpt;
    CAM_LOGD("[%s] CAM_AE_PIX_SE_HST_SET_1 : BIN (%d/%d) COLOR (%d/%d)", __FUNCTION__,
                reg_ae_se_PixHstSet1.Bits.AA_AE_PIX_SE_BIN_MODE_4,
                reg_ae_se_PixHstSet1.Bits.AA_AE_PIX_SE_BIN_MODE_5,
                reg_ae_se_PixHstSet1.Bits.AA_AE_PIX_SE_COLOR_MODE_4,
                reg_ae_se_PixHstSet1.Bits.AA_AE_PIX_SE_COLOR_MODE_5);

    // CAM_AE_PIX_SE_HST0_YRNG
    REG_AA_R1_AA_AE_PIX_SE_HST0_YRNG reg_ae_se_PixHst0Yrng;
    reg_ae_se_PixHst0Yrng.Raw = 0;
    reg_ae_se_PixHst0Yrng.Bits.AA_AE_PIX_SE_Y_LOW_0 = a_sAEConfig->rAEPixelHistWinCFG[0].uAEHistYLow;
    reg_ae_se_PixHst0Yrng.Bits.AA_AE_PIX_SE_Y_HI_0  = a_sAEConfig->rAEPixelHistWinCFG[0].uAEHistYHi;

    // CAM_AE_PIX_HST0_XRNG
    REG_AA_R1_AA_AE_PIX_SE_HST0_XRNG reg_ae_se_PixHst0Xrng;
    reg_ae_se_PixHst0Xrng.Raw = 0;
    reg_ae_se_PixHst0Xrng.Bits.AA_AE_PIX_SE_X_LOW_0 = a_sAEConfig->rAEPixelHistWinCFG[0].uAEHistXLow;
    reg_ae_se_PixHst0Xrng.Bits.AA_AE_PIX_SE_X_HI_0  = a_sAEConfig->rAEPixelHistWinCFG[0].uAEHistXHi;

    // CAM_AE_PIX_HST1_YRNG
    REG_AA_R1_AA_AE_PIX_SE_HST1_YRNG reg_ae_se_PixHst1Yrng;
    reg_ae_se_PixHst1Yrng.Raw = 0;
    reg_ae_se_PixHst1Yrng.Bits.AA_AE_PIX_SE_Y_LOW_1 = a_sAEConfig->rAEPixelHistWinCFG[1].uAEHistYLow;
    reg_ae_se_PixHst1Yrng.Bits.AA_AE_PIX_SE_Y_HI_1  = a_sAEConfig->rAEPixelHistWinCFG[1].uAEHistYHi;

    // CAM_AE_PIX_HST1_XRNG
    REG_AA_R1_AA_AE_PIX_SE_HST1_XRNG reg_ae_se_PixHst1Xrng;
    reg_ae_se_PixHst1Xrng.Raw = 0;
    reg_ae_se_PixHst1Xrng.Bits.AA_AE_PIX_SE_X_LOW_1 = a_sAEConfig->rAEPixelHistWinCFG[1].uAEHistXLow;
    reg_ae_se_PixHst1Xrng.Bits.AA_AE_PIX_SE_X_HI_1  = a_sAEConfig->rAEPixelHistWinCFG[1].uAEHistXHi;

    // CAM_AE_PIX_HST2_YRNG
    REG_AA_R1_AA_AE_PIX_SE_HST2_YRNG reg_ae_se_PixHst2Yrng;
    reg_ae_se_PixHst2Yrng.Raw = 0;
    reg_ae_se_PixHst2Yrng.Bits.AA_AE_PIX_SE_Y_LOW_2 = a_sAEConfig->rAEPixelHistWinCFG[2].uAEHistYLow;
    reg_ae_se_PixHst2Yrng.Bits.AA_AE_PIX_SE_Y_HI_2  = a_sAEConfig->rAEPixelHistWinCFG[2].uAEHistYHi;

    // CAM_AE_PIX_HST2_XRNG
    REG_AA_R1_AA_AE_PIX_SE_HST2_XRNG reg_ae_se_PixHst2Xrng;
    reg_ae_se_PixHst2Xrng.Raw = 0;
    reg_ae_se_PixHst2Xrng.Bits.AA_AE_PIX_SE_X_LOW_2 = a_sAEConfig->rAEPixelHistWinCFG[2].uAEHistXLow;
    reg_ae_se_PixHst2Xrng.Bits.AA_AE_PIX_SE_X_HI_2  = a_sAEConfig->rAEPixelHistWinCFG[2].uAEHistXHi;

    // CAM_AE_PIX_HST3_YRNG
    REG_AA_R1_AA_AE_PIX_SE_HST3_YRNG reg_ae_se_PixHst3Yrng;
    reg_ae_se_PixHst3Yrng.Raw = 0;
    reg_ae_se_PixHst3Yrng.Bits.AA_AE_PIX_SE_Y_LOW_3 = a_sAEConfig->rAEPixelHistWinCFG[3].uAEHistYLow;
    reg_ae_se_PixHst3Yrng.Bits.AA_AE_PIX_SE_Y_HI_3  = a_sAEConfig->rAEPixelHistWinCFG[3].uAEHistYHi;

    // CAM_AE_PIX_HST3_XRNG
    REG_AA_R1_AA_AE_PIX_SE_HST3_XRNG reg_ae_se_PixHst3Xrng;
    reg_ae_se_PixHst3Xrng.Raw = 0;
    reg_ae_se_PixHst3Xrng.Bits.AA_AE_PIX_SE_X_LOW_3 = a_sAEConfig->rAEPixelHistWinCFG[3].uAEHistXLow;
    reg_ae_se_PixHst3Xrng.Bits.AA_AE_PIX_SE_X_HI_3  = a_sAEConfig->rAEPixelHistWinCFG[3].uAEHistXHi;

    // CAM_AE_PIX_HST4_YRNG
    REG_AA_R1_AA_AE_PIX_SE_HST4_YRNG reg_ae_se_PixHst4Yrng;
    reg_ae_se_PixHst4Yrng.Raw = 0;
    reg_ae_se_PixHst4Yrng.Bits.AA_AE_PIX_SE_Y_LOW_4 = a_sAEConfig->rAEPixelHistWinCFG[4].uAEHistYLow;
    reg_ae_se_PixHst4Yrng.Bits.AA_AE_PIX_SE_Y_HI_4  = a_sAEConfig->rAEPixelHistWinCFG[4].uAEHistYHi;

    // CAM_AE_PIX_HST4_XRNG
    REG_AA_R1_AA_AE_PIX_SE_HST4_XRNG reg_ae_se_PixHst4Xrng;
    reg_ae_se_PixHst4Xrng.Raw = 0;
    reg_ae_se_PixHst4Xrng.Bits.AA_AE_PIX_SE_X_LOW_4 = a_sAEConfig->rAEPixelHistWinCFG[4].uAEHistXLow;
    reg_ae_se_PixHst4Xrng.Bits.AA_AE_PIX_SE_X_HI_4  = a_sAEConfig->rAEPixelHistWinCFG[4].uAEHistXHi;

    CAM_LOGD("[%s] CAM_AE_PIX_SE_HST4 : Y-LOW/HIGH (%d/%d) X-LOW/HIGH (%d/%d)", __FUNCTION__,
                reg_ae_se_PixHst4Yrng.Bits.AA_AE_PIX_SE_Y_LOW_4,
                reg_ae_se_PixHst4Yrng.Bits.AA_AE_PIX_SE_Y_HI_4,
                reg_ae_se_PixHst4Xrng.Bits.AA_AE_PIX_SE_X_LOW_4,
                reg_ae_se_PixHst4Xrng.Bits.AA_AE_PIX_SE_X_HI_4);

    // CAM_AE_PIX_HST5_YRNG
    REG_AA_R1_AA_AE_PIX_SE_HST5_YRNG reg_ae_se_PixHst5Yrng;
    reg_ae_se_PixHst5Yrng.Raw = 0;
    reg_ae_se_PixHst5Yrng.Bits.AA_AE_PIX_SE_Y_LOW_5 = a_sAEConfig->rAEPixelHistWinCFG[5].uAEHistYLow;
    reg_ae_se_PixHst5Yrng.Bits.AA_AE_PIX_SE_Y_HI_5  = a_sAEConfig->rAEPixelHistWinCFG[5].uAEHistYHi;

    // CAM_AE_PIX_HST5_XRNG
    REG_AA_R1_AA_AE_PIX_SE_HST5_XRNG reg_ae_se_PixHst5Xrng;
    reg_ae_se_PixHst5Xrng.Raw = 0;
    reg_ae_se_PixHst5Xrng.Bits.AA_AE_PIX_SE_X_LOW_5 = a_sAEConfig->rAEPixelHistWinCFG[5].uAEHistXLow;
    reg_ae_se_PixHst5Xrng.Bits.AA_AE_PIX_SE_X_HI_5  = a_sAEConfig->rAEPixelHistWinCFG[5].uAEHistXHi;

    CAM_LOGD("[%s] CAM_AE_PIX_SE_HST5 : Y-LOW/HIGH (%d/%d) X-LOW/HIGH (%d/%d)", __FUNCTION__,
                reg_ae_se_PixHst5Yrng.Bits.AA_AE_PIX_SE_Y_LOW_5,
                reg_ae_se_PixHst5Yrng.Bits.AA_AE_PIX_SE_Y_HI_5,
                reg_ae_se_PixHst5Xrng.Bits.AA_AE_PIX_SE_X_LOW_5,
                reg_ae_se_PixHst5Xrng.Bits.AA_AE_PIX_SE_X_HI_5);

    // CAM_AE_STAT_EN
    REG_AA_R1_AA_AE_STAT_EN reg_ae_StatEn;

    reg_ae_StatEn.Raw = 0;
    //reg_ae_StatEn.Bits.AE_TSF_STAT_EN = a_sInputAEInfo->bEnableTSFConfig;
    reg_ae_StatEn.Bits.AA_AE_OVERCNT_EN  = a_sInputAEInfo->bEnableOverCntConfig;
    reg_ae_StatEn.Bits.AA_AE_SE_STAT_EN = AE_PARAM_AE_SE_STAT_EN;
    reg_ae_StatEn.Bits.AA_AE_HST_DMA_OUT_EN = (a_sInputAEInfo->bEnableAEHSTDMAOut > 0) ? MTRUE : MFALSE;
    //reg_ae_StatEn.Bits.AE_LINEAR_STAT_EN = (a_sInputAEInfo->bEnableAELINEARStat > 0) ? MTRUE : MFALSE;

    CAM_LOGD("[%s] CAM_AE_STAT_EN : %d/%d/%d AETargetMode : %d", __FUNCTION__,
                reg_ae_StatEn.Bits.AA_AE_OVERCNT_EN,
                reg_ae_StatEn.Bits.AA_AE_SE_STAT_EN,
                reg_ae_StatEn.Bits.AA_AE_HST_DMA_OUT_EN,
                a_sInputAEInfo->eAETargetMode);

    // CAM_AAO_XSIZE
    REG_AAO_R1_AAO_XSIZE reg_AaoXsize;
    reg_AaoXsize.Raw = 0;
    MUINT32 u4EnCnt = reg_ae_PixHstCtl.Bits.AA_AE_PIX_HST0_EN
                    + reg_ae_PixHstCtl.Bits.AA_AE_PIX_HST1_EN
                    + reg_ae_PixHstCtl.Bits.AA_AE_PIX_HST2_EN
                    + reg_ae_PixHstCtl.Bits.AA_AE_PIX_HST3_EN
                    + reg_ae_PixHstCtl.Bits.AA_AE_PIX_HST4_EN
                    + reg_ae_PixHstCtl.Bits.AA_AE_PIX_HST5_EN
                    + reg_ae_PixHstCtl.Bits.AA_AE_PIX_SE_HST0_EN
                    + reg_ae_PixHstCtl.Bits.AA_AE_PIX_SE_HST1_EN
                    + reg_ae_PixHstCtl.Bits.AA_AE_PIX_SE_HST2_EN
                    + reg_ae_PixHstCtl.Bits.AA_AE_PIX_SE_HST3_EN
                    + reg_ae_PixHstCtl.Bits.AA_AE_PIX_SE_HST4_EN
                    + reg_ae_PixHstCtl.Bits.AA_AE_PIX_SE_HST5_EN;
    if( reg_ae_PixHstSet.Bits.AA_AE_PIX_BIN_MODE_0    == 0 ||
        reg_ae_PixHstSet.Bits.AA_AE_PIX_COLOR_MODE_0  == 5 ||
        reg_ae_PixHstSet.Bits.AA_AE_PIX_COLOR_MODE_1  == 5 ||
        reg_ae_PixHstSet.Bits.AA_AE_PIX_COLOR_MODE_2  == 5 ||
        reg_ae_PixHstSet.Bits.AA_AE_PIX_COLOR_MODE_3  == 5 ||
        reg_ae_PixHstSet1.Bits.AA_AE_PIX_COLOR_MODE_4 == 5 ||
        reg_ae_PixHstSet1.Bits.AA_AE_PIX_COLOR_MODE_5 == 5 )
        reg_AaoXsize.Bits.AAO_XSIZE = BlockNumH*(BlockNumW*16+BlockNumW*2*(1+reg_ae_StatEn.Bits.AA_AE_SE_STAT_EN)+BlockNumW*4*reg_ae_StatEn.Bits.AA_AE_OVERCNT_EN)+u4EnCnt*128*3;//Fix me ISP50:BlockNumH*(BlockNumW*8+BlockNumW*12*reg_ae_StatEn.Bits.AE_LINEAR_STAT_EN+BlockNumW*2+BlockNumW*1*reg_ae_StatEn.Bits.AE_OVERCNT_EN)+u4EnCnt*128*3*reg_ae_StatEn.Bits.AE_HST_DMA_OUT_EN;
    else
        reg_AaoXsize.Bits.AAO_XSIZE = BlockNumH*(BlockNumW*16+BlockNumW*2*(1+reg_ae_StatEn.Bits.AA_AE_SE_STAT_EN)+BlockNumW*4*reg_ae_StatEn.Bits.AA_AE_OVERCNT_EN)+u4EnCnt*256*3;//Fix me ISP50:lockNumH*(BlockNumW*8+BlockNumW*12*reg_ae_StatEn.Bits.AE_LINEAR_STAT_EN+BlockNumW*2+BlockNumW*1*reg_ae_StatEn.Bits.AE_OVERCNT_EN)+u4EnCnt*256*3;
    CAM_LOGD("[%s] CAM_AAO_XSIZE : %d, BlockNumW/H: %d/%d, u4EnCnt: %d, AE_PIX_BIN_MODE: %d/%d/%d/%d/%d/%d, AE_PIX_COLOR_MODE: %d/%d/%d/%d/%d/%d  AE_PIX_SE_BIN_MODE: %d/%d/%d/%d/%d/%d, AE_PIX_SE_COLOR_MODE: %d/%d/%d/%d/%d/%d", __FUNCTION__,
                reg_AaoXsize.Bits.AAO_XSIZE, BlockNumW, BlockNumH, u4EnCnt,
                reg_ae_PixHstSet.Bits.AA_AE_PIX_BIN_MODE_0, reg_ae_PixHstSet.Bits.AA_AE_PIX_BIN_MODE_1, reg_ae_PixHstSet.Bits.AA_AE_PIX_BIN_MODE_2,
                reg_ae_PixHstSet.Bits.AA_AE_PIX_BIN_MODE_3, reg_ae_PixHstSet1.Bits.AA_AE_PIX_BIN_MODE_4, reg_ae_PixHstSet1.Bits.AA_AE_PIX_BIN_MODE_5,
                reg_ae_PixHstSet.Bits.AA_AE_PIX_COLOR_MODE_0, reg_ae_PixHstSet.Bits.AA_AE_PIX_COLOR_MODE_1, reg_ae_PixHstSet.Bits.AA_AE_PIX_COLOR_MODE_2,
                reg_ae_PixHstSet.Bits.AA_AE_PIX_COLOR_MODE_3, reg_ae_PixHstSet1.Bits.AA_AE_PIX_COLOR_MODE_4, reg_ae_PixHstSet1.Bits.AA_AE_PIX_COLOR_MODE_5,
                reg_ae_se_PixHstSet.Bits.AA_AE_PIX_SE_BIN_MODE_0, reg_ae_se_PixHstSet.Bits.AA_AE_PIX_SE_BIN_MODE_1, reg_ae_se_PixHstSet.Bits.AA_AE_PIX_SE_BIN_MODE_2,
                reg_ae_se_PixHstSet.Bits.AA_AE_PIX_SE_BIN_MODE_3, reg_ae_se_PixHstSet1.Bits.AA_AE_PIX_SE_BIN_MODE_4, reg_ae_se_PixHstSet1.Bits.AA_AE_PIX_SE_BIN_MODE_5,
                reg_ae_se_PixHstSet.Bits.AA_AE_PIX_SE_COLOR_MODE_0, reg_ae_se_PixHstSet.Bits.AA_AE_PIX_SE_COLOR_MODE_1, reg_ae_se_PixHstSet.Bits.AA_AE_PIX_SE_COLOR_MODE_2,
                reg_ae_se_PixHstSet.Bits.AA_AE_PIX_SE_COLOR_MODE_3, reg_ae_se_PixHstSet1.Bits.AA_AE_PIX_SE_COLOR_MODE_4, reg_ae_se_PixHstSet1.Bits.AA_AE_PIX_SE_COLOR_MODE_5);
    // CAM_AAO_YSIZE
    REG_AAO_R1_AAO_YSIZE reg_AaoYsize;
    reg_AaoYsize.Raw = 0;
    reg_AaoYsize.Bits.AAO_YSIZE = 1;

    // CAM_AE_YCOEF
    REG_AA_R1_AA_AE_YCOEF reg_ae_YCoef;
    reg_ae_YCoef.Raw = 0;
    reg_ae_YCoef.Bits.AA_AE_YCOEF_R = a_sInputAEInfo->u4AEYCoefR;
    reg_ae_YCoef.Bits.AA_AE_YCOEF_G = a_sInputAEInfo->u4AEYCoefG;
    reg_ae_YCoef.Bits.AA_AE_YCOEF_B = a_sInputAEInfo->u4AEYCoefB;

    //NEED FIX
    REG_AA_R1_AA_AE_MODE ae_mode; //HDR:1 normal:0
    REG_AA_R1_AA_AE_BIT_CTL bit_ctl; // HDR=0, normal=1
    REG_AA_R1_AA_AE_HDR_CFG hdr_cfg; //HDR=2097152 normal:1677216
    ae_mode.Raw=0;
    bit_ctl.Raw=0;
    hdr_cfg.Raw=0;
    if(a_sInputAEInfo->eAETargetMode == AE_MODE_NORMAL){
      ae_mode.Bits.AA_AE_EXP_MODE =0;
      bit_ctl.Bits.AA_AE_HDR_DGN_EN_BIT = 1;
      hdr_cfg.Bits.AA_AE_HDR_INV_RTO = 16777216;
    }else{
      ae_mode.Bits.AA_AE_EXP_MODE =1;
      bit_ctl.Bits.AA_AE_HDR_DGN_EN_BIT = 0;
      hdr_cfg.Bits.AA_AE_HDR_INV_RTO = 2097152;
    }
    /*
    // [PSO]
    // CAM_PS_AE_YCOEF0
    CAM_REG_PS_AE_YCOEF0 reg_ps_AeYCoef0;
    reg_ps_AeYCoef0.Raw = 0;
    reg_ps_AeYCoef0.Bits.AE_YCOEF_R = 4809;
    reg_ps_AeYCoef0.Bits.AE_YCOEF_GR = 4809;
    // CAM_PS_AE_YCOEF1
    CAM_REG_PS_AE_YCOEF1 reg_ps_AeYCoef1;
    reg_ps_AeYCoef1.Raw = 0;
    reg_ps_AeYCoef1.Bits.AE_YCOEF_GB = 4809;
    reg_ps_AeYCoef1.Bits.AE_YCOEF_B = 1867;

    // CAM_PS_DATA_TYPE
    CAM_REG_PS_DATA_TYPE reg_ps_DataType;
    reg_ps_DataType.Raw = 0;
    reg_ps_DataType.Bits.SENSOR_TYPE = 0;
    reg_ps_DataType.Bits.G_LE_FIRST = 0;
    reg_ps_DataType.Bits.R_LE_FIRST = 0;
    reg_ps_DataType.Bits.B_LE_FIRST = 0;
    reg_ps_DataType.Bits.EXP_MODE = (a_sInputAEInfo->i4PSEXPMode > 0) ? MTRUE : MFALSE;
    CAM_LOGD("[%s] CAM_PS_DATA_TYPE : %d/%d/%d/%d/%d", __FUNCTION__,
                reg_ps_DataType.Bits.SENSOR_TYPE,
                reg_ps_DataType.Bits.G_LE_FIRST,
                reg_ps_DataType.Bits.R_LE_FIRST,
                reg_ps_DataType.Bits.B_LE_FIRST,
                reg_ps_DataType.Bits.EXP_MODE);
    // CAM_PS_HST_CFG
    CAM_REG_PS_HST_CFG reg_ps_HstCfg;
    reg_ps_HstCfg.Raw = 0;
    reg_ps_HstCfg.Bits.PS_HST_EN = a_sAEConfig->rPSPixelHistWinCFG.bPSHistEn;
    reg_ps_HstCfg.Bits.PS_HST_SEP_G = a_sInputAEInfo->i4PSHSTSepG;
    reg_ps_HstCfg.Bits.PS_CCU_HST_END_Y = a_sInputAEInfo->i4PSCCUHSTEnd;
    CAM_LOGD("[%s] CAM_PS_HST_CFG : %d/%d/%d", __FUNCTION__,
                reg_ps_HstCfg.Bits.PS_HST_EN,
                reg_ps_HstCfg.Bits.PS_HST_SEP_G,
                reg_ps_HstCfg.Bits.PS_CCU_HST_END_Y);
    // CAM_PS_HST_ROI_X
    CAM_REG_PS_HST_ROI_X reg_ps_HstRoiX;
    reg_ps_HstRoiX.Raw = 0;
    reg_ps_HstRoiX.Bits.PS_X_LOW = a_sAEConfig->rPSPixelHistWinCFG.uPSHistXLow;
    reg_ps_HstRoiX.Bits.PS_X_HI = a_sAEConfig->rPSPixelHistWinCFG.uPSHistXHi;
    // CAM_PS_HST_ROI_Y
    CAM_REG_PS_HST_ROI_Y reg_ps_HstRoiY;
    reg_ps_HstRoiY.Raw = 0;
    reg_ps_HstRoiY.Bits.PS_Y_LOW = a_sAEConfig->rPSPixelHistWinCFG.uPSHistYLow;
    reg_ps_HstRoiY.Bits.PS_Y_HI = a_sAEConfig->rPSPixelHistWinCFG.uPSHistYHi;
    CAM_LOGD("[%s] CAM_PS_HST_ROI : X-LOW/HIGH (%d/%d) Y-LOW/HIGH (%d/%d)", __FUNCTION__,
                reg_ps_HstRoiX.Bits.PS_X_LOW,
                reg_ps_HstRoiX.Bits.PS_X_HI,
                reg_ps_HstRoiY.Bits.PS_Y_LOW,
                reg_ps_HstRoiY.Bits.PS_Y_HI);

    // CAM_PSO_XSIZE
    CAM_REG_PSO_XSIZE reg_pso_Xsize;
    reg_pso_Xsize.Raw = 0;
    if(reg_ps_DataType.Bits.SENSOR_TYPE == 0) // 0:normal type
        reg_pso_Xsize.Bits.XSIZE = BlockNumH*(BlockNumW*8+BlockNumW*2)+1024*3*reg_ps_HstCfg.Bits.PS_HST_EN;
    else { // 1:ihdr type or 2:zhdr type
        if(reg_ps_DataType.Bits.EXP_MODE == 2) // 2:both(le+se) mode
            reg_pso_Xsize.Bits.XSIZE = BlockNumH*(BlockNumW*8+BlockNumW*8+BlockNumW*2+BlockNumW*2)+256*3*2*reg_ps_HstCfg.Bits.PS_HST_EN;
        else // 0:le mode or 1:se mode
            reg_pso_Xsize.Bits.XSIZE = BlockNumH*(BlockNumW*8+BlockNumW*2)+256*3*reg_ps_HstCfg.Bits.PS_HST_EN;
    }
    CAM_LOGD("[%s] CAM_PSO_XSIZE : %d, BlockNumW/H: %d/%d", __FUNCTION__,
                reg_pso_Xsize.Bits.XSIZE,
                BlockNumW, BlockNumH);
    // CAM_PSO_YSIZE
    CAM_REG_PSO_YSIZE reg_pso_Ysize;
    reg_pso_Ysize.Raw = 0;
    reg_pso_Ysize.Bits.YSIZE = 1;
    */
#if 0
    REG_AE_INFO(CAM_AE_GAIN2_0, (MUINT32)reg_ae_Gain20.Raw);
    REG_AE_INFO(CAM_AE_GAIN2_1, (MUINT32)reg_ae_Gain21.Raw);
#endif
    REG_AE_INFO(AA_R1_AA_AE_LMT2_0, (MUINT32)reg_ae_Lmt20.Raw);
    REG_AE_INFO(AA_R1_AA_AE_LMT2_1, (MUINT32)reg_ae_Lmt21.Raw);
    REG_AE_INFO(AA_R1_AA_AE_RC_CNV_0, (MUINT32)reg_ae_RcCnv0.Raw);
    REG_AE_INFO(AA_R1_AA_AE_RC_CNV_1, (MUINT32)reg_ae_RcCnv1.Raw);
    REG_AE_INFO(AA_R1_AA_AE_RC_CNV_2, (MUINT32)reg_ae_RcCnv2.Raw);
    REG_AE_INFO(AA_R1_AA_AE_RC_CNV_3, (MUINT32)reg_ae_RcCnv3.Raw);
    REG_AE_INFO(AA_R1_AA_AE_RC_CNV_4, (MUINT32)reg_ae_RcCnv4.Raw);
    REG_AE_INFO(AA_R1_AA_AE_OVER_EXPO_CFG, (MUINT32)reg_ae_OverExpoCfg.Raw);
    REG_AE_INFO(AA_R1_AA_AE_PIX_HST_CTL, (MUINT32)reg_ae_PixHstCtl.Raw);
    REG_AE_INFO(AA_R1_AA_AE_PIX_HST_SET, (MUINT32)reg_ae_PixHstSet.Raw);
    REG_AE_INFO(AA_R1_AA_AE_PIX_HST_SET_1, (MUINT32)reg_ae_PixHstSet1.Raw);
    REG_AE_INFO(AA_R1_AA_AE_PIX_HST0_YRNG, (MUINT32)reg_ae_PixHst0Yrng.Raw);
    REG_AE_INFO(AA_R1_AA_AE_PIX_HST0_XRNG, (MUINT32)reg_ae_PixHst0Xrng.Raw);
    REG_AE_INFO(AA_R1_AA_AE_PIX_HST1_YRNG, (MUINT32)reg_ae_PixHst1Yrng.Raw);
    REG_AE_INFO(AA_R1_AA_AE_PIX_HST1_XRNG, (MUINT32)reg_ae_PixHst1Xrng.Raw);
    REG_AE_INFO(AA_R1_AA_AE_PIX_HST2_YRNG, (MUINT32)reg_ae_PixHst2Yrng.Raw);
    REG_AE_INFO(AA_R1_AA_AE_PIX_HST2_XRNG, (MUINT32)reg_ae_PixHst2Xrng.Raw);
    REG_AE_INFO(AA_R1_AA_AE_PIX_HST3_YRNG, (MUINT32)reg_ae_PixHst3Yrng.Raw);
    REG_AE_INFO(AA_R1_AA_AE_PIX_HST3_XRNG, (MUINT32)reg_ae_PixHst3Xrng.Raw);
    REG_AE_INFO(AA_R1_AA_AE_PIX_HST4_YRNG, (MUINT32)reg_ae_PixHst4Yrng.Raw);
    REG_AE_INFO(AA_R1_AA_AE_PIX_HST4_XRNG, (MUINT32)reg_ae_PixHst4Xrng.Raw);
    REG_AE_INFO(AA_R1_AA_AE_PIX_HST5_YRNG, (MUINT32)reg_ae_PixHst5Yrng.Raw);
    REG_AE_INFO(AA_R1_AA_AE_PIX_HST5_XRNG, (MUINT32)reg_ae_PixHst5Xrng.Raw);
    REG_AE_INFO(AA_R1_AA_AE_PIX_SE_HST_SET, (MUINT32)reg_ae_PixHstSet.Raw);
    REG_AE_INFO(AA_R1_AA_AE_PIX_SE_HST_SET_1, (MUINT32)reg_ae_PixHstSet1.Raw);
    REG_AE_INFO(AA_R1_AA_AE_PIX_SE_HST0_YRNG, (MUINT32)reg_ae_PixHst0Yrng.Raw);
    REG_AE_INFO(AA_R1_AA_AE_PIX_SE_HST0_XRNG, (MUINT32)reg_ae_PixHst0Xrng.Raw);
    REG_AE_INFO(AA_R1_AA_AE_PIX_SE_HST1_YRNG, (MUINT32)reg_ae_PixHst1Yrng.Raw);
    REG_AE_INFO(AA_R1_AA_AE_PIX_SE_HST1_XRNG, (MUINT32)reg_ae_PixHst1Xrng.Raw);
    REG_AE_INFO(AA_R1_AA_AE_PIX_SE_HST2_YRNG, (MUINT32)reg_ae_PixHst2Yrng.Raw);
    REG_AE_INFO(AA_R1_AA_AE_PIX_SE_HST2_XRNG, (MUINT32)reg_ae_PixHst2Xrng.Raw);
    REG_AE_INFO(AA_R1_AA_AE_PIX_SE_HST3_YRNG, (MUINT32)reg_ae_PixHst3Yrng.Raw);
    REG_AE_INFO(AA_R1_AA_AE_PIX_SE_HST3_XRNG, (MUINT32)reg_ae_PixHst3Xrng.Raw);
    REG_AE_INFO(AA_R1_AA_AE_PIX_SE_HST4_YRNG, (MUINT32)reg_ae_PixHst4Yrng.Raw);
    REG_AE_INFO(AA_R1_AA_AE_PIX_SE_HST4_XRNG, (MUINT32)reg_ae_PixHst4Xrng.Raw);
    REG_AE_INFO(AA_R1_AA_AE_PIX_SE_HST5_YRNG, (MUINT32)reg_ae_PixHst5Yrng.Raw);
    REG_AE_INFO(AA_R1_AA_AE_PIX_SE_HST5_XRNG, (MUINT32)reg_ae_PixHst5Xrng.Raw);
    REG_AE_INFO(AA_R1_AA_AE_STAT_EN, (MUINT32)reg_ae_StatEn.Raw);
    REG_AE_INFO(AAO_R1_AAO_XSIZE, (MUINT32)reg_AaoXsize.Raw);
    REG_AE_INFO(AAO_R1_AAO_YSIZE, (MUINT32)reg_AaoYsize.Raw);
    REG_AE_INFO(AA_R1_AA_AE_YCOEF, (MUINT32)reg_ae_YCoef.Raw);
    REG_AE_INFO(AA_R1_AA_AE_MODE, (MUINT32)ae_mode.Raw);
    REG_AE_INFO(AA_R1_AA_AE_BIT_CTL, (MUINT32)bit_ctl.Raw);
    REG_AE_INFO(AA_R1_AA_AE_HDR_CFG, (MUINT32)hdr_cfg.Raw);
    // [PSO]
    //REG_AE_INFO(CAM_PS_AE_YCOEF0, (MUINT32)reg_ps_AeYCoef0.Raw);
    //REG_AE_INFO(CAM_PS_AE_YCOEF1, (MUINT32)reg_ps_AeYCoef1.Raw);
    //REG_AE_INFO(CAM_PS_DATA_TYPE, (MUINT32)reg_ps_DataType.Raw);
    //REG_AE_INFO(CAM_PS_HST_CFG, (MUINT32)reg_ps_HstCfg.Raw);
    //REG_AE_INFO(CAM_PS_HST_ROI_X, (MUINT32)reg_ps_HstRoiX.Raw);
    //REG_AE_INFO(CAM_PS_HST_ROI_Y, (MUINT32)reg_ps_HstRoiY.Raw);
    //REG_AE_INFO(CAM_PSO_XSIZE, (MUINT32)reg_pso_Xsize.Raw);
    //REG_AE_INFO(CAM_PSO_YSIZE, (MUINT32)reg_pso_Ysize.Raw);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AAO / PSO Dynamic Bin
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL ISP_AE_CONFIG_T::reconfig(MVOID *pDBinInfo, MVOID *pOutRegCfg, AE_STAT_PARAM_T & rAEStatConfig)
{
 return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AE Statistic and Histogram config
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
ISP_AE_CONFIG_T::
setAEconfigParam(MUINT32 u4NumW, MUINT32 u4NumH, MBOOL bHDR, MBOOL bOverCnt, MBOOL bTSF, MBOOL bPixel, MUINT32 u4overths, MUINT32 u4overshift, MUINT32 u4AAOmode)
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
    m_u4AAOmode = u4AAOmode;
    return MTRUE;
}

MBOOL
ISP_AE_CONFIG_T::
setAEYCoefParam(MUINT32 u4AEYCoefR, MUINT32 u4AEYCoefG, MUINT32 u4AEYCoefB)
{
    CAM_LOGD("[AEAEYCoef] u4AEYCoefR/G/B = %d/%d/%d\n", u4AEYCoefR, u4AEYCoefG, u4AEYCoefB);
    m_u4AEYCoefR = u4AEYCoefR;
    m_u4AEYCoefG = u4AEYCoefG;
    m_u4AEYCoefB = u4AEYCoefB;

    return MTRUE;
}

