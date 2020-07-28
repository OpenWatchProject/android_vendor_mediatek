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
#define LOG_TAG "awb_hw_config"

#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG       (1)
#endif

#include <aaa_types.h>

#include <mtkcam/utils/std/Log.h>

#include <awb_param.h>
//#include <awb_mgr.h>
#include <awb_config.h>
#include "camera_custom_3a_nvram.h"
#include "camera_custom_awb_nvram.h"

#include <drv/isp_reg.h>

#include <isp_tuning.h>

//using namespace NS3Av3;
using namespace NSIspTuning;

#define REG_AWB_INFO(REG, VALUE) \
    { \
        pAWBRegInfo[ERegInfo_##REG].addr = ((MUINT32)offsetof(cam_reg_t, REG)); \
        pAWBRegInfo[ERegInfo_##REG].val  = VALUE; \							
    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_AWB_CONFIG_T&
ISP_AWB_CONFIG_T::
getInstance(MINT32 const eSensorDev)
{
    switch (eSensorDev)
    {
    case ESensorDev_Main: //  Main Sensor
        return  ISP_AWB_CONFIG_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_AWB_CONFIG_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_AWB_CONFIG_DEV<ESensorDev_Sub>::getInstance();
    case ESensorDev_SubSecond: //  Sub Second Sensor
        return  ISP_AWB_CONFIG_DEV<ESensorDev_SubSecond>::getInstance();
    default:
        CAM_LOGE("eSensorDev = %d", eSensorDev);
        return  ISP_AWB_CONFIG_DEV<ESensorDev_Main>::getInstance();
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID ISP_AWB_CONFIG_T::AWBConfig(AWB_STAT_CONFIG_T *a_sAWBConfig, AWBResultConfig_T *p_sAWBRegInfo)
{
    AWBRegInfo_T *pAWBRegInfo = p_sAWBRegInfo->rAWBRegInfo;

    // AA_R1_AA_AWB_WIN_ORG
    REG_AA_R1_AA_AWB_WIN_ORG reg_awb_WinOrg;
    reg_awb_WinOrg.Raw = 0;
    reg_awb_WinOrg.Bits.AA_AWB_W_HORG = a_sAWBConfig->i4WindowOriginX;
    reg_awb_WinOrg.Bits.AA_AWB_W_VORG = a_sAWBConfig->i4WindowOriginY;
    REG_AWB_INFO(AA_R1_AA_AWB_WIN_ORG, (MUINT32)reg_awb_WinOrg.Raw);
    CAM_LOGD("[%s][AA_R1_AA_AWB_WIN_ORG] H: %d, V: %d\n", __FUNCTION__,reg_awb_WinOrg.Bits.AA_AWB_W_HORG , reg_awb_WinOrg.Bits.AA_AWB_W_VORG);

    // AA_R1_AA_AWB_WIN_SIZE
    REG_AA_R1_AA_AWB_WIN_SIZE reg_awb_WinSize;
    reg_awb_WinSize.Raw = 0;
    reg_awb_WinSize.Bits.AA_AWB_W_HSIZE = a_sAWBConfig->i4WindowSizeX;
    reg_awb_WinSize.Bits.AA_AWB_W_VSIZE = a_sAWBConfig->i4WindowSizeY;
    REG_AWB_INFO(AA_R1_AA_AWB_WIN_SIZE, (MUINT32)reg_awb_WinSize.Raw);
    CAM_LOGD("[%s][AA_R1_AA_AWB_WIN_SIZE] H: %d, V: %d\n", __FUNCTION__,reg_awb_WinSize.Bits.AA_AWB_W_HSIZE , reg_awb_WinSize.Bits.AA_AWB_W_VSIZE);

    // AA_R1_AA_AWB_WIN_PIT
    REG_AA_R1_AA_AWB_WIN_PIT reg_awb_WinPit;
    reg_awb_WinPit.Raw = 0;
    reg_awb_WinPit.Bits.AA_AWB_W_HPIT = a_sAWBConfig->i4WindowPitchX;
    reg_awb_WinPit.Bits.AA_AWB_W_VPIT = a_sAWBConfig->i4WindowPitchY;
    REG_AWB_INFO(AA_R1_AA_AWB_WIN_PIT, (MUINT32)reg_awb_WinPit.Raw);
    CAM_LOGD("[%s][AA_R1_AA_AWB_WIN_PIT] H: %d, V: %d\n", __FUNCTION__,reg_awb_WinPit.Bits.AA_AWB_W_HPIT , reg_awb_WinPit.Bits.AA_AWB_W_VPIT);

    // AA_R1_AA_AWB_WIN_NUM
    REG_AA_R1_AA_AWB_WIN_NUM reg_awb_WinNum;
    reg_awb_WinNum.Raw = 0;
    reg_awb_WinNum.Bits.AA_AWB_W_HNUM = a_sAWBConfig->i4WindowNumX;
    reg_awb_WinNum.Bits.AA_AWB_W_VNUM = a_sAWBConfig->i4WindowNumY;
    REG_AWB_INFO(AA_R1_AA_AWB_WIN_NUM, (MUINT32)reg_awb_WinNum.Raw);
    CAM_LOGD("[%s][AA_R1_AA_AWB_WIN_NUM] H: %d, V: %d\n", __FUNCTION__,reg_awb_WinNum.Bits.AA_AWB_W_HNUM , reg_awb_WinNum.Bits.AA_AWB_W_VNUM);

    // AA_R1_AA_AWB_GAIN1_0
    REG_AA_R1_AA_AWB_GAIN1_0 reg_awb_Gain10;
    reg_awb_Gain10.Raw = 0;
    reg_awb_Gain10.Bits.AA_AWB_GAIN1_R = 0x200;//a_sAWBConfig->i4PreGainR;//0x200;  
    reg_awb_Gain10.Bits.AA_AWB_GAIN1_G = 0x200;//a_sAWBConfig->i4PreGainG;//0x200;
    REG_AWB_INFO(AA_R1_AA_AWB_GAIN1_0, (MUINT32)reg_awb_Gain10.Raw);

    // AA_R1_AA_AWB_GAIN1_1
    REG_AA_R1_AA_AWB_GAIN1_1 reg_awb_Gain11;
    reg_awb_Gain11.Raw = 0;
    reg_awb_Gain11.Bits.AA_AWB_GAIN1_B = 0x200;//a_sAWBConfig->i4PreGainB;//0x200;
    REG_AWB_INFO(AA_R1_AA_AWB_GAIN1_1, (MUINT32)reg_awb_Gain11.Raw);

// AA_R1_AA_AWB_VALID_DATA_WIDTH
    REG_AA_R1_AA_AWB_VALID_DATA_WIDTH reg_awb_Valid_data_width;
    reg_awb_Valid_data_width.Raw = 0;
    reg_awb_Valid_data_width.Bits.AA_AWB_VALID_DATA_WIDTH = a_sAWBConfig->i4ValidDataWidth;
    REG_AWB_INFO(AA_R1_AA_AWB_VALID_DATA_WIDTH, (MUINT32)reg_awb_Valid_data_width.Raw);

    // AA_R1_AA_AWB_LMT1_0
    REG_AA_R1_AA_AWB_LMT1_0 reg_awb_Lmt10;
    reg_awb_Lmt10.Raw = 0;
    reg_awb_Lmt10.Bits.AA_AWB_LMT1_R = a_sAWBConfig->i4PreGainLimitR;//12bits: 0xFFF, 14bits: 0x3FFF
    REG_AWB_INFO(AA_R1_AA_AWB_LMT1_0, (MUINT32)reg_awb_Lmt10.Raw);

    // AA_R1_AA_AWB_LMT1_1
    REG_AA_R1_AA_AWB_LMT1_1 reg_awb_Lmt11;
    reg_awb_Lmt11.Raw = 0;
    reg_awb_Lmt11.Bits.AA_AWB_LMT1_G = a_sAWBConfig->i4PreGainLimitG;//12bits: 0xFFF, 14bits: 0x3FFF
    REG_AWB_INFO(AA_R1_AA_AWB_LMT1_1, (MUINT32)reg_awb_Lmt11.Raw);

// AA_R1_AA_AWB_LMT1_2
    REG_AA_R1_AA_AWB_LMT1_2 reg_awb_Lmt12;
    reg_awb_Lmt12.Raw = 0;
    reg_awb_Lmt12.Bits.AA_AWB_LMT1_B = a_sAWBConfig->i4PreGainLimitB;//12bits: 0xFFF, 14bits: 0x3FFF
    REG_AWB_INFO(AA_R1_AA_AWB_LMT1_2, (MUINT32)reg_awb_Lmt12.Raw);

// AA_R1_AA_AWB_LOW_THR_0
    REG_AA_R1_AA_AWB_LOW_THR_0 reg_awb_LowThr0;
    reg_awb_LowThr0.Raw = 0;
    reg_awb_LowThr0.Bits.AA_AWB_LOW_THR0 = a_sAWBConfig->i4LowThresholdR;//1;
    REG_AWB_INFO(AA_R1_AA_AWB_LOW_THR_0, (MUINT32)reg_awb_LowThr0.Raw);

// AA_R1_AA_AWB_LOW_THR_1
    REG_AA_R1_AA_AWB_LOW_THR_1 reg_awb_LowThr1;
    reg_awb_LowThr1.Raw = 0;
    reg_awb_LowThr1.Bits.AA_AWB_LOW_THR1 = a_sAWBConfig->i4LowThresholdG;//1;
    REG_AWB_INFO(AA_R1_AA_AWB_LOW_THR_1, (MUINT32)reg_awb_LowThr1.Raw);

// AA_R1_AA_AWB_LOW_THR_2
    REG_AA_R1_AA_AWB_LOW_THR_2 reg_awb_LowThr2;
    reg_awb_LowThr2.Raw = 0;
    reg_awb_LowThr2.Bits.AA_AWB_LOW_THR2 = a_sAWBConfig->i4LowThresholdB;//1;
    REG_AWB_INFO(AA_R1_AA_AWB_LOW_THR_2, (MUINT32)reg_awb_LowThr2.Raw);

// AA_R1_AA_AWB_HI_THR_0
    REG_AA_R1_AA_AWB_HI_THR_0 reg_awb_HiThr0;
    reg_awb_HiThr0.Raw = 0;
    reg_awb_HiThr0.Bits.AA_AWB_HI_THR0 = a_sAWBConfig->i4HighThresholdR;//254;
    REG_AWB_INFO(AA_R1_AA_AWB_HI_THR_0, (MUINT32)reg_awb_HiThr0.Raw);

// AA_R1_AA_AWB_HI_THR_1
    REG_AA_R1_AA_AWB_HI_THR_1 reg_awb_HiThr1;
    reg_awb_HiThr1.Raw = 0;
    reg_awb_HiThr1.Bits.AA_AWB_HI_THR1 = a_sAWBConfig->i4HighThresholdG;//254;
    REG_AWB_INFO(AA_R1_AA_AWB_HI_THR_1, (MUINT32)reg_awb_HiThr1.Raw);

// AA_R1_AA_AWB_HI_THR_2
    REG_AA_R1_AA_AWB_HI_THR_2 reg_awb_HiThr2;
    reg_awb_HiThr2.Raw = 0;
    reg_awb_HiThr2.Bits.AA_AWB_HI_THR2 = a_sAWBConfig->i4HighThresholdB;//254;
    REG_AWB_INFO(AA_R1_AA_AWB_HI_THR_2, (MUINT32)reg_awb_HiThr2.Raw);

// AA_R1_AA_AWB_LIGHTSRC_LOW_THR_0
    REG_AA_R1_AA_AWB_LIGHTSRC_LOW_THR_0 reg_awb_Lightsrc_LowThr0;
    reg_awb_Lightsrc_LowThr0.Raw = 0;
    reg_awb_Lightsrc_LowThr0.Bits.AA_AWB_LIGHTSRC_LOW_THR0 = a_sAWBConfig->i4LightSrcLowThresholdR;
    reg_awb_Lightsrc_LowThr0.Bits.AA_AWB_LIGHTSRC_LOW_THR1 = a_sAWBConfig->i4LightSrcLowThresholdG;
    REG_AWB_INFO(AA_R1_AA_AWB_LIGHTSRC_LOW_THR_0, (MUINT32)reg_awb_Lightsrc_LowThr0.Raw);

// AA_R1_AA_AWB_LIGHTSRC_LOW_THR_1
    REG_AA_R1_AA_AWB_LIGHTSRC_LOW_THR_1 reg_awb_Lightsrc_LowThr1;
    reg_awb_Lightsrc_LowThr1.Raw = 0;
    reg_awb_Lightsrc_LowThr1.Bits.AA_AWB_LIGHTSRC_LOW_THR2 = a_sAWBConfig->i4LightSrcLowThresholdB;
    REG_AWB_INFO(AA_R1_AA_AWB_LIGHTSRC_LOW_THR_1, (MUINT32)reg_awb_Lightsrc_LowThr1.Raw);

// AA_R1_AA_AWB_LIGHTSRC_HI_THR_0
    REG_AA_R1_AA_AWB_LIGHTSRC_HI_THR_0 reg_awb_Lightsrc_HiThr0;
    reg_awb_Lightsrc_HiThr0.Raw = 0;
    reg_awb_Lightsrc_HiThr0.Bits.AA_AWB_LIGHTSRC_HI_THR0 = a_sAWBConfig->i4LightSrcHighThresholdR;
    reg_awb_Lightsrc_HiThr0.Bits.AA_AWB_LIGHTSRC_HI_THR1 = a_sAWBConfig->i4LightSrcHighThresholdG;
    REG_AWB_INFO(AA_R1_AA_AWB_LIGHTSRC_HI_THR_0, (MUINT32)reg_awb_Lightsrc_HiThr0.Raw);

// AA_R1_AA_AWB_LIGHTSRC_HI_THR_1
    REG_AA_R1_AA_AWB_LIGHTSRC_HI_THR_1 reg_awb_Lightsrc_HiThr1;
    reg_awb_Lightsrc_HiThr1.Raw = 0;
    reg_awb_Lightsrc_HiThr1.Bits.AA_AWB_LIGHTSRC_HI_THR2 = a_sAWBConfig->i4LightSrcHighThresholdB;
    REG_AWB_INFO(AA_R1_AA_AWB_LIGHTSRC_HI_THR_1, (MUINT32)reg_awb_Lightsrc_HiThr1.Raw);

    // AA_R1_AA_AWB_PIXEL_CNT0
    REG_AA_R1_AA_AWB_PIXEL_CNT0 reg_awb_PixelCnt0;
    reg_awb_PixelCnt0.Raw = 0;
    reg_awb_PixelCnt0.Bits.AA_AWB_PIXEL_CNT0 = a_sAWBConfig->i4PixelCountR;
    REG_AWB_INFO(AA_R1_AA_AWB_PIXEL_CNT0, (MUINT32)reg_awb_PixelCnt0.Raw);

    // AA_R1_AA_AWB_PIXEL_CNT1
    REG_AA_R1_AA_AWB_PIXEL_CNT1 reg_awb_PixelCnt1;
    reg_awb_PixelCnt1.Raw = 0;
    reg_awb_PixelCnt1.Bits.AA_AWB_PIXEL_CNT1 = a_sAWBConfig->i4PixelCountG;
    REG_AWB_INFO(AA_R1_AA_AWB_PIXEL_CNT1, (MUINT32)reg_awb_PixelCnt1.Raw);

    // AA_R1_AA_AWB_PIXEL_CNT2
    REG_AA_R1_AA_AWB_PIXEL_CNT2 reg_awb_PixelCnt2;
    reg_awb_PixelCnt2.Raw = 0;
    reg_awb_PixelCnt2.Bits.AA_AWB_PIXEL_CNT2 = a_sAWBConfig->i4PixelCountB;
    REG_AWB_INFO(AA_R1_AA_AWB_PIXEL_CNT2, (MUINT32)reg_awb_PixelCnt2.Raw);

    // AA_R1_AA_AWB_ERR_THR
    REG_AA_R1_AA_AWB_ERR_THR reg_awb_ErrThr;
    reg_awb_ErrThr.Raw = 0;
    reg_awb_ErrThr.Bits.AA_AWB_ERR_THR = a_sAWBConfig->i4ErrorThreshold;
    REG_AWB_INFO(AA_R1_AA_AWB_ERR_THR, (MUINT32)reg_awb_ErrThr.Raw);

// AA_R1_AA_AWB_RGBSUM_OUTPUT_ENABLE
    REG_AA_R1_AA_AWB_RGBSUM_OUTPUT_ENABLE reg_awb_Rgbsum_output_enable;
    reg_awb_Rgbsum_output_enable.Raw = 0;
    reg_awb_Rgbsum_output_enable.Bits.AA_AWB_RGBSUM_OUTPUT_EN = a_sAWBConfig->i4LinearOutputEn;
    REG_AWB_INFO(AA_R1_AA_AWB_RGBSUM_OUTPUT_ENABLE, (MUINT32)reg_awb_Rgbsum_output_enable.Raw);

// AA_R1_AA_AWB_FORMAT_SHIFT
    REG_AA_R1_AA_AWB_FORMAT_SHIFT reg_awb_Format_shift;
    reg_awb_Format_shift.Raw = 0;
    reg_awb_Format_shift.Bits.AA_AWB_OUTPUT_FORMAT_SHIFT = a_sAWBConfig->i4FormatShift;
    REG_AWB_INFO(AA_R1_AA_AWB_FORMAT_SHIFT, (MUINT32)reg_awb_Format_shift.Raw);

// AA_R1_AA_AWB_POSTGAIN_0
    REG_AA_R1_AA_AWB_POSTGAIN_0 reg_awb_Postgain0;
    reg_awb_Postgain0.Raw = 0;
    reg_awb_Postgain0.Bits.AA_AWB_POSTGAIN_R = a_sAWBConfig->i4PostGainR;
    reg_awb_Postgain0.Bits.AA_AWB_POSTGAIN_G = a_sAWBConfig->i4PostGainG;
    REG_AWB_INFO(AA_R1_AA_AWB_POSTGAIN_0, (MUINT32)reg_awb_Postgain0.Raw);

// AA_R1_AA_AWB_POSTGAIN_1
    REG_AA_R1_AA_AWB_POSTGAIN_1 reg_awb_Postgain1;
    reg_awb_Postgain1.Raw = 0;
    reg_awb_Postgain1.Bits.AA_AWB_POSTGAIN_B = a_sAWBConfig->i4PostGainB;
    REG_AWB_INFO(AA_R1_AA_AWB_POSTGAIN_1, (MUINT32)reg_awb_Postgain1.Raw);
    CAM_LOGD("[%s][AA_R1_AA_AWB_POSTGAIN] R: %d, G: %d, B: %d\n", __FUNCTION__,reg_awb_Postgain0.Bits.AA_AWB_POSTGAIN_R, reg_awb_Postgain0.Bits.AA_AWB_POSTGAIN_G, reg_awb_Postgain1.Bits.AA_AWB_POSTGAIN_B);

    // AA_R1_AA_AWB_ROT
    REG_AA_R1_AA_AWB_ROT reg_awb_Rot;
    reg_awb_Rot.Raw = 0;
    reg_awb_Rot.Bits.AA_AWB_C = (a_sAWBConfig->i4Cos >= 0) ? static_cast<MUINT32>(a_sAWBConfig->i4Cos) : static_cast<MUINT32>(1024 + a_sAWBConfig->i4Cos);;//256;
    reg_awb_Rot.Bits.AA_AWB_S =   (a_sAWBConfig->i4Sin >= 0) ? static_cast<MUINT32>(a_sAWBConfig->i4Sin) : static_cast<MUINT32>(1024 + a_sAWBConfig->i4Sin);//0;
    REG_AWB_INFO(AA_R1_AA_AWB_ROT, (MUINT32)reg_awb_Rot.Raw);

#define AREA_CFG(BOUND)  (BOUND>=0 ? BOUND: (1 << 14) + BOUND)
    // AA_R1_AA_AWB_L0_X
    REG_AA_R1_AA_AWB_L0_X reg_awb_L0X;
    reg_awb_L0X.Raw = 0;
    reg_awb_L0X.Bits.AA_AWB_L0_X_LOW = AREA_CFG(a_sAWBConfig->i4AWBXY_WINL[0]);//-250;
    reg_awb_L0X.Bits.AA_AWB_L0_X_UP = AREA_CFG(a_sAWBConfig->i4AWBXY_WINR[0]);//-100;
    REG_AWB_INFO(AA_R1_AA_AWB_L0_X, (MUINT32)reg_awb_L0X.Raw);

    // AA_R1_AA_AWB_L0_Y
    REG_AA_R1_AA_AWB_L0_Y reg_awb_L0Y;
    reg_awb_L0Y.Raw = 0;
    reg_awb_L0Y.Bits.AA_AWB_L0_Y_LOW = AREA_CFG(a_sAWBConfig->i4AWBXY_WIND[0]);//-600;
    reg_awb_L0Y.Bits.AA_AWB_L0_Y_UP = AREA_CFG(a_sAWBConfig->i4AWBXY_WINU[0]);//-361;
    REG_AWB_INFO(AA_R1_AA_AWB_L0_Y, (MUINT32)reg_awb_L0Y.Raw);

    // AA_R1_AA_AWB_L1_X
    REG_AA_R1_AA_AWB_L1_X reg_awb_L1X;
    reg_awb_L1X.Raw = 0;
    reg_awb_L1X.Bits.AA_AWB_L1_X_LOW = AREA_CFG(a_sAWBConfig->i4AWBXY_WINL[1]);//-782;
    reg_awb_L1X.Bits.AA_AWB_L1_X_UP = AREA_CFG(a_sAWBConfig->i4AWBXY_WINR[1]);//-145;
    REG_AWB_INFO(AA_R1_AA_AWB_L1_X, (MUINT32)reg_awb_L1X.Raw);

    // AA_R1_AA_AWB_L1_Y
    REG_AA_R1_AA_AWB_L1_Y reg_awb_L1Y;
    reg_awb_L1Y.Raw = 0;
    reg_awb_L1Y.Bits.AA_AWB_L1_Y_LOW = AREA_CFG(a_sAWBConfig->i4AWBXY_WIND[1]);//-408;
    reg_awb_L1Y.Bits.AA_AWB_L1_Y_UP = AREA_CFG(a_sAWBConfig->i4AWBXY_WINU[1]);//-310;
    REG_AWB_INFO(AA_R1_AA_AWB_L1_Y, (MUINT32)reg_awb_L1Y.Raw);

    // AA_R1_AA_AWB_L2_X
    REG_AA_R1_AA_AWB_L2_X reg_awb_L2X;
    reg_awb_L2X.Raw = 0;
    reg_awb_L2X.Bits.AA_AWB_L2_X_LOW = AREA_CFG(a_sAWBConfig->i4AWBXY_WINL[2]);//-782;
    reg_awb_L2X.Bits.AA_AWB_L2_X_UP = AREA_CFG(a_sAWBConfig->i4AWBXY_WINR[2]);//-145;
    REG_AWB_INFO(AA_R1_AA_AWB_L2_X, (MUINT32)reg_awb_L2X.Raw);

    // AA_R1_AA_AWB_L2_Y
    REG_AA_R1_AA_AWB_L2_Y reg_awb_L2Y;
    reg_awb_L2Y.Raw = 0;
    reg_awb_L2Y.Bits.AA_AWB_L2_Y_LOW = AREA_CFG(a_sAWBConfig->i4AWBXY_WIND[2]);//-515;
    reg_awb_L2Y.Bits.AA_AWB_L2_Y_UP = AREA_CFG(a_sAWBConfig->i4AWBXY_WINU[2]);//-408;
    REG_AWB_INFO(AA_R1_AA_AWB_L2_Y, (MUINT32)reg_awb_L2Y.Raw);

    // AA_R1_AA_AWB_L3_X
    REG_AA_R1_AA_AWB_L3_X reg_awb_L3X;
    reg_awb_L3X.Raw = 0;
    reg_awb_L3X.Bits.AA_AWB_L3_X_LOW = AREA_CFG(a_sAWBConfig->i4AWBXY_WINL[3]);//-145;
    reg_awb_L3X.Bits.AA_AWB_L3_X_UP = AREA_CFG(a_sAWBConfig->i4AWBXY_WINR[3]);//18;
    REG_AWB_INFO(AA_R1_AA_AWB_L3_X, (MUINT32)reg_awb_L3X.Raw);

    // AA_R1_AA_AWB_L3_Y
    REG_AA_R1_AA_AWB_L3_Y reg_awb_L3Y;
    reg_awb_L3Y.Raw = 0;
    reg_awb_L3Y.Bits.AA_AWB_L3_Y_LOW = AREA_CFG(a_sAWBConfig->i4AWBXY_WIND[3]);//-454;
    reg_awb_L3Y.Bits.AA_AWB_L3_Y_UP = AREA_CFG(a_sAWBConfig->i4AWBXY_WINU[3]);//-328;
    REG_AWB_INFO(AA_R1_AA_AWB_L3_Y, (MUINT32)reg_awb_L3Y.Raw);

    // AA_R1_AA_AWB_L4_X
    REG_AA_R1_AA_AWB_L4_X reg_awb_L4X;
    reg_awb_L4X.Raw = 0;
    reg_awb_L4X.Bits.AA_AWB_L4_X_LOW = AREA_CFG(a_sAWBConfig->i4AWBXY_WINL[4]);//-145;
    reg_awb_L4X.Bits.AA_AWB_L4_X_UP = AREA_CFG(a_sAWBConfig->i4AWBXY_WINR[4]);//23;
    REG_AWB_INFO(AA_R1_AA_AWB_L4_X, (MUINT32)reg_awb_L4X.Raw);

    // AA_R1_AA_AWB_L4_Y
    REG_AA_R1_AA_AWB_L4_Y reg_awb_L4Y;
    reg_awb_L4Y.Raw = 0;
    reg_awb_L4Y.Bits.AA_AWB_L4_Y_LOW = AREA_CFG(a_sAWBConfig->i4AWBXY_WIND[4]);//-540;
    reg_awb_L4Y.Bits.AA_AWB_L4_Y_UP = AREA_CFG(a_sAWBConfig->i4AWBXY_WINU[4]);//-454;
    REG_AWB_INFO(AA_R1_AA_AWB_L4_Y, (MUINT32)reg_awb_L4Y.Raw);

    // AA_R1_AA_AWB_L5_X
    REG_AA_R1_AA_AWB_L5_X reg_awb_L5X;
    reg_awb_L5X.Raw = 0;
    reg_awb_L5X.Bits.AA_AWB_L5_X_LOW = AREA_CFG(a_sAWBConfig->i4AWBXY_WINL[5]);//18;
    reg_awb_L5X.Bits.AA_AWB_L5_X_UP = AREA_CFG(a_sAWBConfig->i4AWBXY_WINR[5]);//199;
    REG_AWB_INFO(AA_R1_AA_AWB_L5_X, (MUINT32)reg_awb_L5X.Raw);

    // AA_R1_AA_AWB_L5_Y
    REG_AA_R1_AA_AWB_L5_Y reg_awb_L5Y;
    reg_awb_L5Y.Raw = 0;
    reg_awb_L5Y.Bits.AA_AWB_L5_Y_LOW = AREA_CFG(a_sAWBConfig->i4AWBXY_WIND[5]);//-454;
    reg_awb_L5Y.Bits.AA_AWB_L5_Y_UP = AREA_CFG(a_sAWBConfig->i4AWBXY_WINU[5]);//-328;
    REG_AWB_INFO(AA_R1_AA_AWB_L5_Y, (MUINT32)reg_awb_L5Y.Raw);

    // AA_R1_AA_AWB_L6_X
    REG_AA_R1_AA_AWB_L6_X reg_awb_L6X;
    reg_awb_L6X.Raw = 0;
    reg_awb_L6X.Bits.AA_AWB_L6_X_LOW = AREA_CFG(a_sAWBConfig->i4AWBXY_WINL[6]);//199;
    reg_awb_L6X.Bits.AA_AWB_L6_X_UP = AREA_CFG(a_sAWBConfig->i4AWBXY_WINR[6]);//529;
    REG_AWB_INFO(AA_R1_AA_AWB_L6_X, (MUINT32)reg_awb_L6X.Raw);

    // AA_R1_AA_AWB_L6_Y
    REG_AA_R1_AA_AWB_L6_Y reg_awb_L6Y;
    reg_awb_L6Y.Raw = 0;
    reg_awb_L6Y.Bits.AA_AWB_L6_Y_LOW = AREA_CFG(a_sAWBConfig->i4AWBXY_WIND[6]);//-427;
    reg_awb_L6Y.Bits.AA_AWB_L6_Y_UP = AREA_CFG(a_sAWBConfig->i4AWBXY_WINU[6]);//-328;
    REG_AWB_INFO(AA_R1_AA_AWB_L6_Y, (MUINT32)reg_awb_L6Y.Raw);

    // AA_R1_AA_AWB_L7_X
    REG_AA_R1_AA_AWB_L7_X reg_awb_L7X;
    reg_awb_L7X.Raw = 0;
    reg_awb_L7X.Bits.AA_AWB_L7_X_LOW = AREA_CFG(a_sAWBConfig->i4AWBXY_WINL[7]);//23;
    reg_awb_L7X.Bits.AA_AWB_L7_X_UP = AREA_CFG(a_sAWBConfig->i4AWBXY_WINR[7]);//199;
    REG_AWB_INFO(AA_R1_AA_AWB_L7_X, (MUINT32)reg_awb_L7X.Raw);

    // AA_R1_AA_AWB_L7_Y
    REG_AA_R1_AA_AWB_L7_Y reg_awb_L7Y;
    reg_awb_L7Y.Raw = 0;
    reg_awb_L7Y.Bits.AA_AWB_L7_Y_LOW = AREA_CFG(a_sAWBConfig->i4AWBXY_WIND[7]);//-540;
    reg_awb_L7Y.Bits.AA_AWB_L7_Y_UP = AREA_CFG(a_sAWBConfig->i4AWBXY_WINU[7]);//-454;
    REG_AWB_INFO(AA_R1_AA_AWB_L7_Y, (MUINT32)reg_awb_L7Y.Raw);

    // AA_R1_AA_AWB_L8_X
    REG_AA_R1_AA_AWB_L8_X reg_awb_L8X;
    reg_awb_L8X.Raw = 0;
    reg_awb_L8X.Bits.AA_AWB_L8_X_LOW = AREA_CFG(a_sAWBConfig->i4AWBXY_WINL[8]);//0;
    reg_awb_L8X.Bits.AA_AWB_L8_X_UP = AREA_CFG(a_sAWBConfig->i4AWBXY_WINR[8]);//0;
    REG_AWB_INFO(AA_R1_AA_AWB_L8_X, (MUINT32)reg_awb_L8X.Raw);

    // AA_R1_AA_AWB_L8_Y
    REG_AA_R1_AA_AWB_L8_Y reg_awb_L8Y;
    reg_awb_L8Y.Raw = 0;
    reg_awb_L8Y.Bits.AA_AWB_L8_Y_LOW = AREA_CFG(a_sAWBConfig->i4AWBXY_WIND[8]);//0;
    reg_awb_L8Y.Bits.AA_AWB_L8_Y_UP = AREA_CFG(a_sAWBConfig->i4AWBXY_WINU[8]);//0;
    REG_AWB_INFO(AA_R1_AA_AWB_L8_Y, (MUINT32)reg_awb_L8Y.Raw);

    // AA_R1_AA_AWB_L9_X
    REG_AA_R1_AA_AWB_L9_X reg_awb_L9X;
    reg_awb_L9X.Raw = 0;
    reg_awb_L9X.Bits.AA_AWB_L9_X_LOW = AREA_CFG(a_sAWBConfig->i4AWBXY_WINL[9]);//0;
    reg_awb_L9X.Bits.AA_AWB_L9_X_UP = AREA_CFG(a_sAWBConfig->i4AWBXY_WINR[9]);//0;
    REG_AWB_INFO(AA_R1_AA_AWB_L9_X, (MUINT32)reg_awb_L9X.Raw);

    // AA_R1_AA_AWB_L9_Y
    REG_AA_R1_AA_AWB_L9_Y reg_awb_L9Y;
    reg_awb_L9Y.Raw = 0;
    reg_awb_L9Y.Bits.AA_AWB_L9_Y_LOW = AREA_CFG(a_sAWBConfig->i4AWBXY_WIND[9]);//0;
    reg_awb_L9Y.Bits.AA_AWB_L9_Y_UP = AREA_CFG(a_sAWBConfig->i4AWBXY_WINU[9]);//0;
    REG_AWB_INFO(AA_R1_AA_AWB_L9_Y, (MUINT32)reg_awb_L9Y.Raw);

    //bd ++
    MUINT32 rCscCCM[9] = {0x200, 0, 0, 0, 0x200, 0, 0, 0, 0x200};// Bayer
    // RWB
#define RWB_CFG(BOUND)	(BOUND>=0 ? BOUND: (1 << 12) + BOUND)

    // AA_R1_AA_AWB_RC_CNV_0
    REG_AA_R1_AA_AWB_RC_CNV_0 reg_awb_CNV0;
    reg_awb_CNV0.Raw = 0;
    reg_awb_CNV0.Bits.AA_AWB_RC_CNV00 = RWB_CFG(rCscCCM[0]);
    reg_awb_CNV0.Bits.AA_AWB_RC_CNV01 = RWB_CFG(rCscCCM[1]);
    REG_AWB_INFO(AA_R1_AA_AWB_RC_CNV_0, (MUINT32)reg_awb_CNV0.Raw);

    // AA_R1_AA_AWB_RC_CNV_1
    REG_AA_R1_AA_AWB_RC_CNV_1 reg_awb_CNV1;
    reg_awb_CNV1.Raw = 0;
    reg_awb_CNV1.Bits.AA_AWB_RC_CNV02 = RWB_CFG(rCscCCM[2]);
    reg_awb_CNV1.Bits.AA_AWB_RC_CNV10 = RWB_CFG(rCscCCM[3]);
    REG_AWB_INFO(AA_R1_AA_AWB_RC_CNV_1, (MUINT32)reg_awb_CNV1.Raw);

    // AA_R1_AA_AWB_RC_CNV_2
    REG_AA_R1_AA_AWB_RC_CNV_2 reg_awb_CNV2;
    reg_awb_CNV2.Raw = 0;
    reg_awb_CNV2.Bits.AA_AWB_RC_CNV11 = RWB_CFG(rCscCCM[4]);
    reg_awb_CNV2.Bits.AA_AWB_RC_CNV12 = RWB_CFG(rCscCCM[5]);
    REG_AWB_INFO(AA_R1_AA_AWB_RC_CNV_2, (MUINT32)reg_awb_CNV2.Raw);

    // AA_R1_AA_AWB_RC_CNV_3
    REG_AA_R1_AA_AWB_RC_CNV_3 reg_awb_CNV3;
    reg_awb_CNV3.Raw = 0;
    reg_awb_CNV3.Bits.AA_AWB_RC_CNV20 =RWB_CFG(rCscCCM[6]);
    reg_awb_CNV3.Bits.AA_AWB_RC_CNV21 = RWB_CFG(rCscCCM[7]);
    REG_AWB_INFO(AA_R1_AA_AWB_RC_CNV_3, (MUINT32)reg_awb_CNV3.Raw);

    // AA_R1_AA_AWB_RC_CNV_4
    REG_AA_R1_AA_AWB_RC_CNV_4 reg_awb_CNV4;
    reg_awb_CNV4.Raw = 0;
    reg_awb_CNV4.Bits.AA_AWB_RC_CNV22 = RWB_CFG(rCscCCM[8]);
    reg_awb_CNV4.Bits.AA_AWB_RC_ACC = RWB_CFG(0x09);
    REG_AWB_INFO(AA_R1_AA_AWB_RC_CNV_4, (MUINT32)reg_awb_CNV4.Raw);

    //AA_R1_AA_AWB_SPARE

//AE Pregain2
    // AA_R1_AA_AE_GAIN2_0
    REG_AA_R1_AA_AE_GAIN2_0 reg_ae_Pregain20;
    reg_ae_Pregain20.Raw = 0;
    reg_ae_Pregain20.Bits.AA_AE_GAIN2_R = a_sAWBConfig->i4PreGain2R;
    reg_ae_Pregain20.Bits.AA_AE_GAIN2_G = a_sAWBConfig->i4PreGain2G;
    REG_AWB_INFO(AA_R1_AA_AE_GAIN2_0, (MUINT32)reg_ae_Pregain20.Raw);

    // AA_R1_AA_AE_GAIN2_1
    REG_AA_R1_AA_AE_GAIN2_1 reg_ae_Pregain21;
    reg_ae_Pregain21.Raw = 0;
    reg_ae_Pregain21.Bits.AA_AE_GAIN2_B = a_sAWBConfig->i4PreGain2B;
    REG_AWB_INFO(AA_R1_AA_AE_GAIN2_1, (MUINT32)reg_ae_Pregain21.Raw);
    CAM_LOGD("[%s][AA_R1_AA_AE_GAIN2] R: %d, G: %d, B: %d\n", __FUNCTION__,reg_ae_Pregain20.Bits.AA_AE_GAIN2_R, reg_ae_Pregain20.Bits.AA_AE_GAIN2_G, reg_ae_Pregain21.Bits.AA_AE_GAIN2_B);

}

MVOID ISP_AWB_CONFIG_T::AWBConfig_update(AWB_STAT_CONFIG_T *a_sAWBConfig, AWBResultConfig_T *p_sAWBRegInfo)
{
    AWBRegInfo_T *pAWBRegInfo = p_sAWBRegInfo->rAWBRegInfo;

    // AA_R1_AA_AWB_POSTGAIN_0
    REG_AA_R1_AA_AWB_POSTGAIN_0 reg_awb_Postgain0;
    reg_awb_Postgain0.Raw = 0;
    reg_awb_Postgain0.Bits.AA_AWB_POSTGAIN_R = a_sAWBConfig->i4PostGainR;
    reg_awb_Postgain0.Bits.AA_AWB_POSTGAIN_G = a_sAWBConfig->i4PostGainG;
    REG_AWB_INFO(AA_R1_AA_AWB_POSTGAIN_0, (MUINT32)reg_awb_Postgain0.Raw);

    // AA_R1_AA_AWB_POSTGAIN_1
    REG_AA_R1_AA_AWB_POSTGAIN_1 reg_awb_Postgain1;
    reg_awb_Postgain1.Raw = 0;
    reg_awb_Postgain1.Bits.AA_AWB_POSTGAIN_B = a_sAWBConfig->i4PostGainB;
    REG_AWB_INFO(AA_R1_AA_AWB_POSTGAIN_1, (MUINT32)reg_awb_Postgain1.Raw);

    // AA_R1_AA_AWB_LMT1_0
    REG_AA_R1_AA_AWB_LMT1_0 reg_awb_Lmt10;
    reg_awb_Lmt10.Raw = 0;
    reg_awb_Lmt10.Bits.AA_AWB_LMT1_R = a_sAWBConfig->i4PreGainLimitR;
    REG_AWB_INFO(AA_R1_AA_AWB_LMT1_0, (MUINT32)reg_awb_Lmt10.Raw);

    // AA_R1_AA_AWB_LMT1_1
    REG_AA_R1_AA_AWB_LMT1_1 reg_awb_Lmt11;
    reg_awb_Lmt11.Raw = 0;
    reg_awb_Lmt11.Bits.AA_AWB_LMT1_G = a_sAWBConfig->i4PreGainLimitG;
    REG_AWB_INFO(AA_R1_AA_AWB_LMT1_1, (MUINT32)reg_awb_Lmt11.Raw);

    // AA_R1_AA_AWB_LMT1_2
    REG_AA_R1_AA_AWB_LMT1_2 reg_awb_Lmt12;
    reg_awb_Lmt12.Raw = 0;
    reg_awb_Lmt12.Bits.AA_AWB_LMT1_B = a_sAWBConfig->i4PreGainLimitB;
    REG_AWB_INFO(AA_R1_AA_AWB_LMT1_2, (MUINT32)reg_awb_Lmt12.Raw);

    // AA_R1_AA_AE_GAIN2_0
    REG_AA_R1_AA_AE_GAIN2_0 reg_ae_Pregain20;
    reg_ae_Pregain20.Raw = 0;
    reg_ae_Pregain20.Bits.AA_AE_GAIN2_R = a_sAWBConfig->i4PreGain2R;
    reg_ae_Pregain20.Bits.AA_AE_GAIN2_G = a_sAWBConfig->i4PreGain2G;
    REG_AWB_INFO(AA_R1_AA_AE_GAIN2_0, (MUINT32)reg_ae_Pregain20.Raw);

    // AA_R1_AA_AE_GAIN2_1
    REG_AA_R1_AA_AE_GAIN2_1 reg_ae_Pregain21;
    reg_ae_Pregain21.Raw = 0;
    reg_ae_Pregain21.Bits.AA_AE_GAIN2_B = a_sAWBConfig->i4PreGain2B;
    REG_AWB_INFO(AA_R1_AA_AE_GAIN2_1, (MUINT32)reg_ae_Pregain21.Raw);
    CAM_LOGD("[%s][AA_R1_AA_AE_GAIN2] R: %d, G: %d, B: %d\n", __FUNCTION__,reg_ae_Pregain20.Bits.AA_AE_GAIN2_R, reg_ae_Pregain20.Bits.AA_AE_GAIN2_G, reg_ae_Pregain21.Bits.AA_AE_GAIN2_B);

	// AA_R1_AA_AWB_RGBSUM_OUTPUT_ENABLE
    REG_AA_R1_AA_AWB_RGBSUM_OUTPUT_ENABLE reg_awb_Rgbsum_output_enable;
    reg_awb_Rgbsum_output_enable.Raw = 0;
    reg_awb_Rgbsum_output_enable.Bits.AA_AWB_RGBSUM_OUTPUT_EN = a_sAWBConfig->i4LinearOutputEn;
    REG_AWB_INFO(AA_R1_AA_AWB_RGBSUM_OUTPUT_ENABLE, (MUINT32)reg_awb_Rgbsum_output_enable.Raw);
}


