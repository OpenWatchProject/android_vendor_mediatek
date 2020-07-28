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
#define LOG_TAG "isp_mgr_awb_stat"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include "property_utils.h"
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include <drv/isp_reg.h>
#include "isp_mgr.h"
#include "isp_mgr_config.h"

using namespace NSCam;

namespace NSIspTuning
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AWB Statistics Config
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_AWB_STAT_CONFIG_T&
ISP_MGR_AWB_STAT_CONFIG_T::
getInstance(ESensorDev_T const eSensorDev)
{
    ISP_MGR_MODULE_GET_INSTANCE(AWB_STAT_CONFIG);
}

#if 0
MBOOL ISP_MGR_AWB_STAT_CONFIG_T::config(AWB_STAT_CONFIG_T& rAWBStatConfig, MBOOL bHBIN2Enable, int imgW, int imgH, int blkW, int blkH)
{
    (void)bHBIN2Enable;
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.isp_mgr_awb.enable", value, "0");
    m_bDebugEnable = atoi(value);
    m_imgW = (MUINT32)imgW;
    m_imgH = (MUINT32)imgH;
    m_blkW = (MUINT32)blkW;
    m_blkH = (MUINT32)blkH;

    CAM_LOGD("WOrg %d/%d WSi/%d/%d WP/%d/%d WN/%d/%d PG %d/%d/%d PGL %d/%d/%d Lth %d/%d/%d Hth %d/%d/%d PC %d/%d/%d ",
         (int)rAWBStatConfig.i4WindowOriginX,(int)rAWBStatConfig.i4WindowOriginY, (int)rAWBStatConfig.i4WindowSizeX, (int)rAWBStatConfig.i4WindowSizeY,
         (int)rAWBStatConfig.i4WindowPitchX,(int)rAWBStatConfig.i4WindowPitchY, (int)rAWBStatConfig.i4WindowNumX, (int)rAWBStatConfig.i4WindowNumY,
         (int)rAWBStatConfig.i4PreGainR, (int)rAWBStatConfig.i4PreGainG, (int)rAWBStatConfig.i4PreGainB,
         (int)rAWBStatConfig.i4PreGainLimitR, (int)rAWBStatConfig.i4PreGainLimitG, (int)rAWBStatConfig.i4PreGainLimitB,
         (int)rAWBStatConfig.i4LowThresholdR,  (int)rAWBStatConfig.i4LowThresholdG, (int)rAWBStatConfig.i4LowThresholdB,
         (int)rAWBStatConfig.i4HighThresholdR, (int)rAWBStatConfig.i4HighThresholdG, (int)rAWBStatConfig.i4HighThresholdB,
         (int)rAWBStatConfig.i4PixelCountR, (int)rAWBStatConfig.i4PixelCountG, (int)rAWBStatConfig.i4PixelCountB);
    CAM_LOGD("Eth %d/%d Cord/%d/%d L0/%d/%d/%d/%d L1/%d/%d/%d/%d L2/%d/%d/%d/%d L3/%d/%d/%d/%d L4/%d/%d/%d/%d L5/%d/%d/%d/%d ",
         (int)rAWBStatConfig.i4ErrorThreshold,(int)rAWBStatConfig.i4ErrorShiftBits, (int)rAWBStatConfig.i4Cos, (int)rAWBStatConfig.i4Sin,
         (int)rAWBStatConfig.i4AWBXY_WINL[0], (int)rAWBStatConfig.i4AWBXY_WINR[0], (int)rAWBStatConfig.i4AWBXY_WIND[0], (int)rAWBStatConfig.i4AWBXY_WINU[0],
         (int)rAWBStatConfig.i4AWBXY_WINL[1], (int)rAWBStatConfig.i4AWBXY_WINR[1], (int)rAWBStatConfig.i4AWBXY_WIND[1], (int)rAWBStatConfig.i4AWBXY_WINU[1],
         (int)rAWBStatConfig.i4AWBXY_WINL[2], (int)rAWBStatConfig.i4AWBXY_WINR[2], (int)rAWBStatConfig.i4AWBXY_WIND[2], (int)rAWBStatConfig.i4AWBXY_WINU[2],
         (int)rAWBStatConfig.i4AWBXY_WINL[3], (int)rAWBStatConfig.i4AWBXY_WINR[3], (int)rAWBStatConfig.i4AWBXY_WIND[3], (int)rAWBStatConfig.i4AWBXY_WINU[3],
         (int)rAWBStatConfig.i4AWBXY_WINL[4], (int)rAWBStatConfig.i4AWBXY_WINR[4], (int)rAWBStatConfig.i4AWBXY_WIND[4], (int)rAWBStatConfig.i4AWBXY_WINU[4],
         (int)rAWBStatConfig.i4AWBXY_WINL[5], (int)rAWBStatConfig.i4AWBXY_WINR[5], (int)rAWBStatConfig.i4AWBXY_WIND[5], (int)rAWBStatConfig.i4AWBXY_WINU[5]);
    CAM_LOGD("L6/%d/%d/%d/%d L7/%d/%d/%d/%d L8/%d/%d/%d/%d L9/%d/%d/%d/%d",
         (int)rAWBStatConfig.i4AWBXY_WINL[6], (int)rAWBStatConfig.i4AWBXY_WINR[6], (int)rAWBStatConfig.i4AWBXY_WIND[6], (int)rAWBStatConfig.i4AWBXY_WINU[6],
         (int)rAWBStatConfig.i4AWBXY_WINL[7], (int)rAWBStatConfig.i4AWBXY_WINR[7], (int)rAWBStatConfig.i4AWBXY_WIND[7], (int)rAWBStatConfig.i4AWBXY_WINU[7],
         (int)rAWBStatConfig.i4AWBXY_WINL[8], (int)rAWBStatConfig.i4AWBXY_WINR[8], (int)rAWBStatConfig.i4AWBXY_WIND[8], (int)rAWBStatConfig.i4AWBXY_WINU[8],
         (int)rAWBStatConfig.i4AWBXY_WINL[9], (int)rAWBStatConfig.i4AWBXY_WINR[9], (int)rAWBStatConfig.i4AWBXY_WIND[9], (int)rAWBStatConfig.i4AWBXY_WINU[9]);

#if 0 //
    MUINT32 u4AwbMotionThres = 1024;
    addressErrorCheck("Before ISP_MGR_AWB_STAT_CONFIG_T::apply()");

    // CAM_AWB_WIN_ORG
    reinterpret_cast<ISP_CAM_AWB_WIN_ORG_T*>(REG_INFO_VALUE_PTR(CAM_AWB_WIN_ORG))->AWB_W_HORG = rAWBStatConfig.i4WindowOriginX;
    reinterpret_cast<ISP_CAM_AWB_WIN_ORG_T*>(REG_INFO_VALUE_PTR(CAM_AWB_WIN_ORG))->AWB_W_VORG = rAWBStatConfig.i4WindowOriginY;
    // CAM_AWB_WIN_SIZE
    reinterpret_cast<ISP_CAM_AWB_WIN_SIZE_T*>(REG_INFO_VALUE_PTR(CAM_AWB_WIN_SIZE))->AWB_W_HSIZE = rAWBStatConfig.i4WindowSizeX;
    reinterpret_cast<ISP_CAM_AWB_WIN_SIZE_T*>(REG_INFO_VALUE_PTR(CAM_AWB_WIN_SIZE))->AWB_W_VSIZE = rAWBStatConfig.i4WindowSizeY;
    // CAM_AWB_WIN_PIT
    reinterpret_cast<ISP_CAM_AWB_WIN_PIT_T*>(REG_INFO_VALUE_PTR(CAM_AWB_WIN_PIT))->AWB_W_HPIT = rAWBStatConfig.i4WindowPitchX;
    reinterpret_cast<ISP_CAM_AWB_WIN_PIT_T*>(REG_INFO_VALUE_PTR(CAM_AWB_WIN_PIT))->AWB_W_VPIT = rAWBStatConfig.i4WindowPitchY;
    // CAM_AWB_WIN_NUM
    reinterpret_cast<ISP_CAM_AWB_WIN_NUM_T*>(REG_INFO_VALUE_PTR(CAM_AWB_WIN_NUM))->AWB_W_HNUM = rAWBStatConfig.i4WindowNumX;
    reinterpret_cast<ISP_CAM_AWB_WIN_NUM_T*>(REG_INFO_VALUE_PTR(CAM_AWB_WIN_NUM))->AWB_W_VNUM = rAWBStatConfig.i4WindowNumY;
    // CAM_AWB_GAIN1_0
    reinterpret_cast<ISP_CAM_AWB_GAIN1_0_T*>(REG_INFO_VALUE_PTR(CAM_AWB_GAIN1_0))->AWB_GAIN1_R = rAWBStatConfig.i4PreGainR;
    reinterpret_cast<ISP_CAM_AWB_GAIN1_0_T*>(REG_INFO_VALUE_PTR(CAM_AWB_GAIN1_0))->AWB_GAIN1_G = rAWBStatConfig.i4PreGainG;
    // CAM_AWB_GAIN1_1
    reinterpret_cast<ISP_CAM_AWB_GAIN1_1_T*>(REG_INFO_VALUE_PTR(CAM_AWB_GAIN1_1))->AWB_GAIN1_B = rAWBStatConfig.i4PreGainB;
    // CAM_AWB_LMT1_0
    reinterpret_cast<ISP_CAM_AWB_LMT1_0_T*>(REG_INFO_VALUE_PTR(CAM_AWB_LMT1_0))->AWB_LMT1_R = rAWBStatConfig.i4PreGainLimitR;
    reinterpret_cast<ISP_CAM_AWB_LMT1_0_T*>(REG_INFO_VALUE_PTR(CAM_AWB_LMT1_0))->AWB_LMT1_G = rAWBStatConfig.i4PreGainLimitG;
    // CAM_AWB_LMT1_1
    reinterpret_cast<ISP_CAM_AWB_LMT1_1_T*>(REG_INFO_VALUE_PTR(CAM_AWB_LMT1_1))->AWB_LMT1_B = rAWBStatConfig.i4PreGainLimitB;
    // CAM_AWB_LOW_THR
    reinterpret_cast<ISP_CAM_AWB_LOW_THR_T*>(REG_INFO_VALUE_PTR(CAM_AWB_LOW_THR))->AWB_LOW_THR0 = rAWBStatConfig.i4LowThresholdR;
    reinterpret_cast<ISP_CAM_AWB_LOW_THR_T*>(REG_INFO_VALUE_PTR(CAM_AWB_LOW_THR))->AWB_LOW_THR1 = rAWBStatConfig.i4LowThresholdG;
    reinterpret_cast<ISP_CAM_AWB_LOW_THR_T*>(REG_INFO_VALUE_PTR(CAM_AWB_LOW_THR))->AWB_LOW_THR2 = rAWBStatConfig.i4LowThresholdB;
    // CAM_AWB_HI_THR
    reinterpret_cast<ISP_CAM_AWB_HI_THR_T*>(REG_INFO_VALUE_PTR(CAM_AWB_HI_THR))->AWB_HI_THR0 = rAWBStatConfig.i4HighThresholdR;
    reinterpret_cast<ISP_CAM_AWB_HI_THR_T*>(REG_INFO_VALUE_PTR(CAM_AWB_HI_THR))->AWB_HI_THR1 = rAWBStatConfig.i4HighThresholdG;
    reinterpret_cast<ISP_CAM_AWB_HI_THR_T*>(REG_INFO_VALUE_PTR(CAM_AWB_HI_THR))->AWB_HI_THR2 = rAWBStatConfig.i4HighThresholdB;
    // CAM_AWB_PIXEL_CNT0
    reinterpret_cast<ISP_CAM_AWB_PIXEL_CNT0_T*>(REG_INFO_VALUE_PTR(CAM_AWB_PIXEL_CNT0))->AWB_PIXEL_CNT0 =  rAWBStatConfig.i4PixelCountR;
    // CAM_AWB_PIXEL_CNT1
    reinterpret_cast<ISP_CAM_AWB_PIXEL_CNT1_T*>(REG_INFO_VALUE_PTR(CAM_AWB_PIXEL_CNT1))->AWB_PIXEL_CNT1 =  rAWBStatConfig.i4PixelCountG;
    // CAM_AWB_PIXEL_CNT2
    reinterpret_cast<ISP_CAM_AWB_PIXEL_CNT2_T*>(REG_INFO_VALUE_PTR(CAM_AWB_PIXEL_CNT2))->AWB_PIXEL_CNT2 =  rAWBStatConfig.i4PixelCountB;
    // CAM_AWB_ERR_THR
    reinterpret_cast<ISP_CAM_AWB_ERR_THR_T*>(REG_INFO_VALUE_PTR(CAM_AWB_ERR_THR))->AWB_ERR_THR = rAWBStatConfig.i4ErrorThreshold;
    reinterpret_cast<ISP_CAM_AWB_ERR_THR_T*>(REG_INFO_VALUE_PTR(CAM_AWB_ERR_THR))->AWB_ERR_SFT = rAWBStatConfig.i4ErrorShiftBits;

    // CAM_AWB_ROT
    reinterpret_cast<ISP_CAM_AWB_ROT_T*>(REG_INFO_VALUE_PTR(CAM_AWB_ROT))->AWB_C = (rAWBStatConfig.i4Cos >= 0) ? static_cast<MUINT32>(rAWBStatConfig.i4Cos) : static_cast<MUINT32>(1024 + rAWBStatConfig.i4Cos);
    reinterpret_cast<ISP_CAM_AWB_ROT_T*>(REG_INFO_VALUE_PTR(CAM_AWB_ROT))->AWB_S = (rAWBStatConfig.i4Sin >= 0) ? static_cast<MUINT32>(rAWBStatConfig.i4Sin) : static_cast<MUINT32>(1024 + rAWBStatConfig.i4Sin);

    #define AWB_LIGHT_AREA_CFG(TYPE, REG, FIELD, BOUND)\
    if (BOUND >= 0)\
        reinterpret_cast<TYPE*>(REG_INFO_VALUE_PTR(REG))->FIELD = BOUND;\
    else\
        reinterpret_cast<TYPE*>(REG_INFO_VALUE_PTR(REG))->FIELD = (1 << 14) + BOUND;\


    // CAM_AWB_L0
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L0_X_T, CAM_AWB_L0_X, AWB_L0_X_LOW, rAWBStatConfig.i4AWBXY_WINL[0])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L0_X_T, CAM_AWB_L0_X, AWB_L0_X_UP, rAWBStatConfig.i4AWBXY_WINR[0])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L0_Y_T, CAM_AWB_L0_Y, AWB_L0_Y_LOW, rAWBStatConfig.i4AWBXY_WIND[0])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L0_Y_T, CAM_AWB_L0_Y, AWB_L0_Y_UP, rAWBStatConfig.i4AWBXY_WINU[0])

    // CAM_AWB_L1
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L1_X_T, CAM_AWB_L1_X, AWB_L1_X_LOW, rAWBStatConfig.i4AWBXY_WINL[1])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L1_X_T, CAM_AWB_L1_X, AWB_L1_X_UP, rAWBStatConfig.i4AWBXY_WINR[1])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L1_Y_T, CAM_AWB_L1_Y, AWB_L1_Y_LOW, rAWBStatConfig.i4AWBXY_WIND[1])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L1_Y_T, CAM_AWB_L1_Y, AWB_L1_Y_UP, rAWBStatConfig.i4AWBXY_WINU[1])

    // CAM_AWB_L2
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L2_X_T, CAM_AWB_L2_X, AWB_L2_X_LOW, rAWBStatConfig.i4AWBXY_WINL[2])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L2_X_T, CAM_AWB_L2_X, AWB_L2_X_UP, rAWBStatConfig.i4AWBXY_WINR[2])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L2_Y_T, CAM_AWB_L2_Y, AWB_L2_Y_LOW, rAWBStatConfig.i4AWBXY_WIND[2])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L2_Y_T, CAM_AWB_L2_Y, AWB_L2_Y_UP, rAWBStatConfig.i4AWBXY_WINU[2])

    // CAM_AWB_L3
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L3_X_T, CAM_AWB_L3_X, AWB_L3_X_LOW, rAWBStatConfig.i4AWBXY_WINL[3])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L3_X_T, CAM_AWB_L3_X, AWB_L3_X_UP, rAWBStatConfig.i4AWBXY_WINR[3])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L3_Y_T, CAM_AWB_L3_Y, AWB_L3_Y_LOW, rAWBStatConfig.i4AWBXY_WIND[3])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L3_Y_T, CAM_AWB_L3_Y, AWB_L3_Y_UP, rAWBStatConfig.i4AWBXY_WINU[3])

    // CAM_AWB_L4
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L4_X_T, CAM_AWB_L4_X, AWB_L4_X_LOW, rAWBStatConfig.i4AWBXY_WINL[4])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L4_X_T, CAM_AWB_L4_X, AWB_L4_X_UP, rAWBStatConfig.i4AWBXY_WINR[4])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L4_Y_T, CAM_AWB_L4_Y, AWB_L4_Y_LOW, rAWBStatConfig.i4AWBXY_WIND[4])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L4_Y_T, CAM_AWB_L4_Y, AWB_L4_Y_UP, rAWBStatConfig.i4AWBXY_WINU[4])

    // CAM_AWB_L5
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L5_X_T, CAM_AWB_L5_X, AWB_L5_X_LOW, rAWBStatConfig.i4AWBXY_WINL[5])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L5_X_T, CAM_AWB_L5_X, AWB_L5_X_UP, rAWBStatConfig.i4AWBXY_WINR[5])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L5_Y_T, CAM_AWB_L5_Y, AWB_L5_Y_LOW, rAWBStatConfig.i4AWBXY_WIND[5])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L5_Y_T, CAM_AWB_L5_Y, AWB_L5_Y_UP, rAWBStatConfig.i4AWBXY_WINU[5])

    // CAM_AWB_L6
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L6_X_T, CAM_AWB_L6_X, AWB_L6_X_LOW, rAWBStatConfig.i4AWBXY_WINL[6])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L6_X_T, CAM_AWB_L6_X, AWB_L6_X_UP, rAWBStatConfig.i4AWBXY_WINR[6])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L6_Y_T, CAM_AWB_L6_Y, AWB_L6_Y_LOW, rAWBStatConfig.i4AWBXY_WIND[6])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L6_Y_T, CAM_AWB_L6_Y, AWB_L6_Y_UP, rAWBStatConfig.i4AWBXY_WINU[6])

    // CAM_AWB_L7
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L7_X_T, CAM_AWB_L7_X, AWB_L7_X_LOW, rAWBStatConfig.i4AWBXY_WINL[7])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L7_X_T, CAM_AWB_L7_X, AWB_L7_X_UP, rAWBStatConfig.i4AWBXY_WINR[7])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L7_Y_T, CAM_AWB_L7_Y, AWB_L7_Y_LOW, rAWBStatConfig.i4AWBXY_WIND[7])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L7_Y_T, CAM_AWB_L7_Y, AWB_L7_Y_UP, rAWBStatConfig.i4AWBXY_WINU[7])

    // CAM_AWB_L8
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L8_X_T, CAM_AWB_L8_X, AWB_L8_X_LOW, rAWBStatConfig.i4AWBXY_WINL[8])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L8_X_T, CAM_AWB_L8_X, AWB_L8_X_UP, rAWBStatConfig.i4AWBXY_WINR[8])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L8_Y_T, CAM_AWB_L8_Y, AWB_L8_Y_LOW, rAWBStatConfig.i4AWBXY_WIND[8])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L8_Y_T, CAM_AWB_L8_Y, AWB_L8_Y_UP, rAWBStatConfig.i4AWBXY_WINU[8])

    // CAM_AWB_L9
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L9_X_T, CAM_AWB_L9_X, AWB_L9_X_LOW, rAWBStatConfig.i4AWBXY_WINL[9])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L9_X_T, CAM_AWB_L9_X, AWB_L9_X_UP, rAWBStatConfig.i4AWBXY_WINR[9])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L9_Y_T, CAM_AWB_L9_Y, AWB_L9_Y_LOW, rAWBStatConfig.i4AWBXY_WIND[9])
    AWB_LIGHT_AREA_CFG(ISP_CAM_AWB_L9_Y_T, CAM_AWB_L9_Y, AWB_L9_Y_UP, rAWBStatConfig.i4AWBXY_WINU[9])

    //AWB_MOTION_THR
    CAM_REG_AWB_MOTION_THR AwbMotionThr;
    AwbMotionThr.Bits.AWB_MOTION_THR = u4AwbMotionThres;
    REG_INFO_VALUE(CAM_AWB_MOTION_THR) = (MUINT32)AwbMotionThr.Raw;
#else
//addressErrorCheck("Before ISP_MGR_AWB_STAT_CONFIG_T::apply()");
/*
    int BlockNumW = blkW;
    int BlockNumH = blkH;
    int tgW = imgW;
    int tgH = imgH;
    //int PitchW = tgW/BlockNumW;
    //int PitchH = tgH/BlockNumH;
    // FIXME for development
    int PitchW = (tgW/BlockNumW)/2*2;
    int PitchH = (tgH/BlockNumH)/2*2;
    int SizeW  = (PitchW / 2) * 2;
    int SizeH  = (PitchH / 2) * 2;
    int OriginX = (tgW - PitchW*BlockNumW)/2;
    int OriginY = (tgH - PitchH*BlockNumH)/2;
    CAM_LOGD_IF(m_bDebugEnable, "[configAAO_AWB] PitchW/PitchH = %d/%d , SizeW/SizeH = %d/%d , OriginX/OriginY = %d/%d", PitchW,PitchH, SizeW,SizeH,OriginX,OriginY);
    int i4WindowPixelNumR = (SizeW * SizeH) / 4;
    int i4WindowPixelNumG = i4WindowPixelNumR * 2;
    int i4WindowPixelNumB = i4WindowPixelNumR;
    int i4PixelCountR = ((1 << 24) + (i4WindowPixelNumR >> 1)) / i4WindowPixelNumR;
    int i4PixelCountG = ((1 << 24) + (i4WindowPixelNumG >> 1)) / i4WindowPixelNumG;
    int i4PixelCountB = ((1 << 24) + (i4WindowPixelNumB >> 1)) / i4WindowPixelNumB;
    CAM_LOGD_IF(m_bDebugEnable, "[configAAO_AWB] i4WindowPixelNumR/G/B = %d/%d/%d , i4PixelCountR/G/B = %d/%d/%d",
        i4WindowPixelNumR,i4WindowPixelNumG, i4WindowPixelNumB,
        i4PixelCountR,i4PixelCountG,i4PixelCountB);
*/
        // CAM_AWB_WIN_ORG
        CAM_REG_AWB_WIN_ORG AwbWinOrg;
        AwbWinOrg.Bits.AWB_W_HORG = rAWBStatConfig.i4WindowOriginX;
        AwbWinOrg.Bits.AWB_W_VORG = rAWBStatConfig.i4WindowOriginY;
        //TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_AWB_WIN_ORG, (MUINT32)AwbWinOrg.Raw, 0);
        REG_INFO_VALUE(CAM_AWB_WIN_ORG) = (MUINT32)AwbWinOrg.Raw;
        // CAM_AWB_WIN_SIZE
        CAM_REG_AWB_WIN_SIZE AwbWinSize;
        AwbWinSize.Bits.AWB_W_HSIZE = rAWBStatConfig.i4WindowSizeX;
        AwbWinSize.Bits.AWB_W_VSIZE = rAWBStatConfig.i4WindowSizeY;
        //TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_AWB_WIN_SIZE, (MUINT32)AwbWinSize.Raw, 0);
        REG_INFO_VALUE(CAM_AWB_WIN_SIZE) = (MUINT32)AwbWinSize.Raw;
        // CAM_AWB_WIN_PIT
        CAM_REG_AWB_WIN_PIT AwbWinPit;
        AwbWinPit.Bits.AWB_W_HPIT = rAWBStatConfig.i4WindowPitchX;
        AwbWinPit.Bits.AWB_W_VPIT = rAWBStatConfig.i4WindowPitchY;
        //TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_AWB_WIN_PIT, (MUINT32)AwbWinPit.Raw, 0);
        REG_INFO_VALUE(CAM_AWB_WIN_PIT) = (MUINT32)AwbWinPit.Raw;
        // CAM_AWB_WIN_NUM
        CAM_REG_AWB_WIN_NUM AwbWinNum;
        AwbWinNum.Bits.AWB_W_HNUM = rAWBStatConfig.i4WindowNumX;
        AwbWinNum.Bits.AWB_W_VNUM = rAWBStatConfig.i4WindowNumY;
        //TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_AWB_WIN_NUM, (MUINT32)AwbWinNum.Raw, 0);
        REG_INFO_VALUE(CAM_AWB_WIN_NUM) = (MUINT32)AwbWinNum.Raw;
        // CAM_AWB_GAIN1_0
        CAM_REG_AWB_GAIN1_0 AwbGain10;
        AwbGain10.Bits.AWB_GAIN1_R = rAWBStatConfig.i4PreGainR;//0x200;
        AwbGain10.Bits.AWB_GAIN1_G = rAWBStatConfig.i4PreGainG;//0x200;
        //TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_AWB_GAIN1_0, (MUINT32)AwbGain10.Raw, 0);
        REG_INFO_VALUE(CAM_AWB_GAIN1_0) = (MUINT32)AwbGain10.Raw;
        // CAM_AWB_GAIN1_1
        CAM_REG_AWB_GAIN1_1 AwbGain11;
        AwbGain11.Bits.AWB_GAIN1_B = rAWBStatConfig.i4PreGainB;//0x200;
        //TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_AWB_GAIN1_1, (MUINT32)AwbGain11.Raw, 0);
        REG_INFO_VALUE(CAM_AWB_GAIN1_1) = (MUINT32)AwbGain11.Raw;

        // CAM_AWB_LMT1_0
        CAM_REG_AWB_LMT1_0 AwbLmt10;
        // CAM_AWB_LMT1_1
        CAM_REG_AWB_LMT1_1 AwbLmt11;
        AwbLmt10.Bits.AWB_LMT1_R = rAWBStatConfig.i4PreGainLimitR;//12bits: 0xFFF, 14bits: 0x3FFF
        AwbLmt10.Bits.AWB_LMT1_G = rAWBStatConfig.i4PreGainLimitG;//12bits: 0xFFF, 14bits: 0x3FFF
        //TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_AWB_LMT1_0, (MUINT32)AwbLmt10.Raw, 0);
        REG_INFO_VALUE(CAM_AWB_LMT1_0) = (MUINT32)AwbLmt10.Raw;
        // CAM_AWB_LMT1_1
        ///CAM_REG_AWB_LMT1_1 AwbLmt11;
        AwbLmt11.Bits.AWB_LMT1_B = rAWBStatConfig.i4PreGainLimitB;//12bits: 0xFFF, 14bits: 0x3FFF
        //TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_AWB_LMT1_1, (MUINT32)AwbLmt11.Raw, 0);
        REG_INFO_VALUE(CAM_AWB_LMT1_1) = (MUINT32)AwbLmt11.Raw;

        // CAM_AWB_LOW_THR
        CAM_REG_AWB_LOW_THR AwbLowThr;
        AwbLowThr.Bits.AWB_LOW_THR0 = rAWBStatConfig.i4LowThresholdR;//1;
        AwbLowThr.Bits.AWB_LOW_THR1 = rAWBStatConfig.i4LowThresholdG;//1;
        AwbLowThr.Bits.AWB_LOW_THR2 = rAWBStatConfig.i4LowThresholdB;//1;
        //TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_AWB_LOW_THR, (MUINT32)AwbLowThr.Raw, 0);
        REG_INFO_VALUE(CAM_AWB_LOW_THR) = (MUINT32)AwbLowThr.Raw;
        // CAM_AWB_HI_THR
        CAM_REG_AWB_HI_THR AwbHiThr;
        AwbHiThr.Bits.AWB_HI_THR0 = rAWBStatConfig.i4HighThresholdR;//254;
        AwbHiThr.Bits.AWB_HI_THR1 = rAWBStatConfig.i4HighThresholdG;//254;
        AwbHiThr.Bits.AWB_HI_THR2 = rAWBStatConfig.i4HighThresholdB;//254;
        //TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_AWB_HI_THR, (MUINT32)AwbHiThr.Raw, 0);
        REG_INFO_VALUE(CAM_AWB_HI_THR) = (MUINT32)AwbHiThr.Raw;
        // CAM_AWB_PIXEL_CNT0
        CAM_REG_AWB_PIXEL_CNT0 AwbPixelCnt0;
        AwbPixelCnt0.Bits.AWB_PIXEL_CNT0 = rAWBStatConfig.i4PixelCountR;
        //TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_AWB_PIXEL_CNT0, (MUINT32)AwbPixelCnt0.Raw, 0);
        REG_INFO_VALUE(CAM_AWB_PIXEL_CNT0) = (MUINT32)AwbPixelCnt0.Raw;
        // CAM_AWB_PIXEL_CNT1
        CAM_REG_AWB_PIXEL_CNT1 AwbPixelCnt1;
        AwbPixelCnt1.Bits.AWB_PIXEL_CNT1 = rAWBStatConfig.i4PixelCountG;
        //TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_AWB_PIXEL_CNT1, (MUINT32)AwbPixelCnt1.Raw, 0);
        REG_INFO_VALUE(CAM_AWB_PIXEL_CNT1) = (MUINT32)AwbPixelCnt1.Raw;
        // CAM_AWB_PIXEL_CNT2
        CAM_REG_AWB_PIXEL_CNT2 AwbPixelCnt2;
        AwbPixelCnt2.Bits.AWB_PIXEL_CNT2 = rAWBStatConfig.i4PixelCountB;
        //TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_AWB_PIXEL_CNT2, (MUINT32)AwbPixelCnt2.Raw, 0);
        REG_INFO_VALUE(CAM_AWB_PIXEL_CNT2) = (MUINT32)AwbPixelCnt2.Raw;
        // CAM_AWB_ERR_THR
        CAM_REG_AWB_ERR_THR AwbErrThr;
        AwbErrThr.Bits.AWB_ERR_THR = rAWBStatConfig.i4ErrorThreshold;//20;
        AwbErrThr.Bits.AWB_ERR_SFT =  rAWBStatConfig.i4ErrorShiftBits;//0;
        //TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_AWB_ERR_THR, (MUINT32)AwbErrThr.Raw, 0);
        REG_INFO_VALUE(CAM_AWB_ERR_THR) = (MUINT32)AwbErrThr.Raw;
        // CAM_AWB_ROT
        CAM_REG_AWB_ROT AwbRot;
        AwbRot.Bits.AWB_C = (rAWBStatConfig.i4Cos >= 0) ? static_cast<MUINT32>(rAWBStatConfig.i4Cos) : static_cast<MUINT32>(1024 + rAWBStatConfig.i4Cos);;//256;
        AwbRot.Bits.AWB_S =   (rAWBStatConfig.i4Sin >= 0) ? static_cast<MUINT32>(rAWBStatConfig.i4Sin) : static_cast<MUINT32>(1024 + rAWBStatConfig.i4Sin);//0;
        //TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_AWB_ROT, (MUINT32)AwbRot.Raw, 0);
        REG_INFO_VALUE(CAM_AWB_ROT) = (MUINT32)AwbRot.Raw;


#define AREA_CFG(BOUND)  (BOUND>=0 ? BOUND: (1 << 14) + BOUND)
        // AWB_L0_X
        CAM_REG_AWB_L0_X AwbL0X;
        AwbL0X.Bits.AWB_L0_X_LOW = AREA_CFG(rAWBStatConfig.i4AWBXY_WINL[0]);//-250;
        AwbL0X.Bits.AWB_L0_X_UP = AREA_CFG(rAWBStatConfig.i4AWBXY_WINR[0]);//-100;
        //TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_AWB_L0_X, (MUINT32)AwbL0X.Raw, 0);
        REG_INFO_VALUE(CAM_AWB_L0_X) = (MUINT32)AwbL0X.Raw;
        // AWB_L0_Y
        CAM_REG_AWB_L0_Y AwbL0Y;
        AwbL0Y.Bits.AWB_L0_Y_LOW = AREA_CFG(rAWBStatConfig.i4AWBXY_WIND[0]);//-600;
        AwbL0Y.Bits.AWB_L0_Y_UP = AREA_CFG(rAWBStatConfig.i4AWBXY_WINU[0]);//-361;
        //TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_AWB_L0_Y, (MUINT32)AwbL0Y.Raw, 0);
        REG_INFO_VALUE(CAM_AWB_L0_Y) = (MUINT32)AwbL0Y.Raw;
        // AWB_L1_X
        CAM_REG_AWB_L1_X AwbL1X;
        AwbL1X.Bits.AWB_L1_X_LOW = AREA_CFG(rAWBStatConfig.i4AWBXY_WINL[1]);//-782;
        AwbL1X.Bits.AWB_L1_X_UP = AREA_CFG(rAWBStatConfig.i4AWBXY_WINR[1]);//-145;
        //TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_AWB_L1_X, (MUINT32)AwbL1X.Raw, 0);
        REG_INFO_VALUE(CAM_AWB_L1_X) = (MUINT32)AwbL1X.Raw;
        // AWB_L1_Y
        CAM_REG_AWB_L1_Y AwbL1Y;
        AwbL1Y.Bits.AWB_L1_Y_LOW = AREA_CFG(rAWBStatConfig.i4AWBXY_WIND[1]);//-408;
        AwbL1Y.Bits.AWB_L1_Y_UP = AREA_CFG(rAWBStatConfig.i4AWBXY_WINU[1]);//-310;
        //TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_AWB_L1_Y, (MUINT32)AwbL1Y.Raw, 0);
        REG_INFO_VALUE(CAM_AWB_L1_Y) = (MUINT32)AwbL1Y.Raw;
        // AWB_L2_X
        CAM_REG_AWB_L2_X AwbL2X;
        AwbL2X.Bits.AWB_L2_X_LOW = AREA_CFG(rAWBStatConfig.i4AWBXY_WINL[2]);//-782;
        AwbL2X.Bits.AWB_L2_X_UP = AREA_CFG(rAWBStatConfig.i4AWBXY_WINR[2]);//-145;
        //TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_AWB_L2_X, (MUINT32)AwbL2X.Raw, 0);
        REG_INFO_VALUE(CAM_AWB_L2_X) = (MUINT32)AwbL2X.Raw;
        // AWB_L2_Y
        CAM_REG_AWB_L2_Y AwbL2Y;
        AwbL2Y.Bits.AWB_L2_Y_LOW = AREA_CFG(rAWBStatConfig.i4AWBXY_WIND[2]);//-515;
        AwbL2Y.Bits.AWB_L2_Y_UP = AREA_CFG(rAWBStatConfig.i4AWBXY_WINU[2]);//-408;
        //TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_AWB_L2_Y, (MUINT32)AwbL2Y.Raw, 0);
        REG_INFO_VALUE(CAM_AWB_L2_Y) = (MUINT32)AwbL2Y.Raw;

        // AWB_L3_X
        CAM_REG_AWB_L3_X AwbL3X;
        AwbL3X.Bits.AWB_L3_X_LOW = AREA_CFG(rAWBStatConfig.i4AWBXY_WINL[3]);//-145;
        AwbL3X.Bits.AWB_L3_X_UP = AREA_CFG(rAWBStatConfig.i4AWBXY_WINR[3]);//18;
        //TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_AWB_L3_X, (MUINT32)AwbL3X.Raw, 0);
        REG_INFO_VALUE(CAM_AWB_L3_X) = (MUINT32)AwbL3X.Raw;
        // AWB_L3_Y
        CAM_REG_AWB_L3_Y AwbL3Y;
        AwbL3Y.Bits.AWB_L3_Y_LOW = AREA_CFG(rAWBStatConfig.i4AWBXY_WIND[3]);//-454;
        AwbL3Y.Bits.AWB_L3_Y_UP = AREA_CFG(rAWBStatConfig.i4AWBXY_WINU[3]);//-328;
        //TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_AWB_L3_Y, (MUINT32)AwbL3Y.Raw, 0);
        REG_INFO_VALUE(CAM_AWB_L3_Y) = (MUINT32)AwbL3Y.Raw;
        // AWB_L4_X
        CAM_REG_AWB_L4_X AwbL4X;
        AwbL4X.Bits.AWB_L4_X_LOW = AREA_CFG(rAWBStatConfig.i4AWBXY_WINL[4]);//-145;
        AwbL4X.Bits.AWB_L4_X_UP = AREA_CFG(rAWBStatConfig.i4AWBXY_WINR[4]);//23;
        //TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_AWB_L4_X, (MUINT32)AwbL4X.Raw, 0);
        REG_INFO_VALUE(CAM_AWB_L4_X) = (MUINT32)AwbL4X.Raw;
        // AWB_L4_Y
        CAM_REG_AWB_L4_Y AwbL4Y;
        AwbL4Y.Bits.AWB_L4_Y_LOW = AREA_CFG(rAWBStatConfig.i4AWBXY_WIND[4]);//-540;
        AwbL4Y.Bits.AWB_L4_Y_UP = AREA_CFG(rAWBStatConfig.i4AWBXY_WINU[4]);//-454;
        //TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_AWB_L4_Y, (MUINT32)AwbL4Y.Raw, 0);
        REG_INFO_VALUE(CAM_AWB_L4_Y) = (MUINT32)AwbL4Y.Raw;
        // AWB_L5_X
        CAM_REG_AWB_L5_X AwbL5X;
        AwbL5X.Bits.AWB_L5_X_LOW = AREA_CFG(rAWBStatConfig.i4AWBXY_WINL[5]);//18;
        AwbL5X.Bits.AWB_L5_X_UP = AREA_CFG(rAWBStatConfig.i4AWBXY_WINR[5]);//199;
        //TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_AWB_L5_X, (MUINT32)AwbL5X.Raw, 0);
        REG_INFO_VALUE(CAM_AWB_L5_X) = (MUINT32)AwbL5X.Raw;
        // AWB_L5_Y
        CAM_REG_AWB_L5_Y AwbL5Y;
        AwbL5Y.Bits.AWB_L5_Y_LOW = AREA_CFG(rAWBStatConfig.i4AWBXY_WIND[5]);//-454;
        AwbL5Y.Bits.AWB_L5_Y_UP = AREA_CFG(rAWBStatConfig.i4AWBXY_WINU[5]);//-328;
        //TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_AWB_L5_Y, (MUINT32)AwbL5Y.Raw, 0);
        REG_INFO_VALUE(CAM_AWB_L5_Y) = (MUINT32)AwbL5Y.Raw;
        // AWB_L6_X
        CAM_REG_AWB_L6_X AwbL6X;
        AwbL6X.Bits.AWB_L6_X_LOW = AREA_CFG(rAWBStatConfig.i4AWBXY_WINL[6]);//199;
        AwbL6X.Bits.AWB_L6_X_UP = AREA_CFG(rAWBStatConfig.i4AWBXY_WINR[6]);//529;
        //TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_AWB_L6_X, (MUINT32)AwbL6X.Raw, 0);
        REG_INFO_VALUE(CAM_AWB_L6_X) = (MUINT32)AwbL6X.Raw;
        // AWB_L6_Y
        CAM_REG_AWB_L6_Y AwbL6Y;
        AwbL6Y.Bits.AWB_L6_Y_LOW = AREA_CFG(rAWBStatConfig.i4AWBXY_WIND[6]);//-427;
        AwbL6Y.Bits.AWB_L6_Y_UP = AREA_CFG(rAWBStatConfig.i4AWBXY_WINU[6]);//-328;
        //TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_AWB_L6_Y, (MUINT32)AwbL6Y.Raw, 0);
        REG_INFO_VALUE(CAM_AWB_L6_Y) = (MUINT32)AwbL6Y.Raw;
        // AWB_L7_X
        CAM_REG_AWB_L7_X AwbL7X;
        AwbL7X.Bits.AWB_L7_X_LOW = AREA_CFG(rAWBStatConfig.i4AWBXY_WINL[7]);//23;
        AwbL7X.Bits.AWB_L7_X_UP = AREA_CFG(rAWBStatConfig.i4AWBXY_WINR[7]);//199;
        //TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_AWB_L7_X, (MUINT32)AwbL7X.Raw, 0);
        REG_INFO_VALUE(CAM_AWB_L7_X) = (MUINT32)AwbL7X.Raw;
        // AWB_L7_Y
        CAM_REG_AWB_L7_Y AwbL7Y;
        AwbL7Y.Bits.AWB_L7_Y_LOW = AREA_CFG(rAWBStatConfig.i4AWBXY_WIND[7]);//-540;
        AwbL7Y.Bits.AWB_L7_Y_UP = AREA_CFG(rAWBStatConfig.i4AWBXY_WINU[7]);//-454;
        //TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_AWB_L7_Y, (MUINT32)AwbL7Y.Raw, 0);
        REG_INFO_VALUE(CAM_AWB_L7_Y) = (MUINT32)AwbL7Y.Raw;
        // AWB_L8_X
        CAM_REG_AWB_L8_X AwbL8X;
        AwbL8X.Bits.AWB_L8_X_LOW = AREA_CFG(rAWBStatConfig.i4AWBXY_WINL[8]);//0;
        AwbL8X.Bits.AWB_L8_X_UP = AREA_CFG(rAWBStatConfig.i4AWBXY_WINR[8]);//0;
        //TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_AWB_L8_X, (MUINT32)AwbL8X.Raw, 0);
        REG_INFO_VALUE(CAM_AWB_L8_X) = (MUINT32)AwbL8X.Raw;
        // AWB_L8_Y
        CAM_REG_AWB_L8_Y AwbL8Y;
        AwbL8Y.Bits.AWB_L8_Y_LOW = AREA_CFG(rAWBStatConfig.i4AWBXY_WIND[8]);//0;
        AwbL8Y.Bits.AWB_L8_Y_UP = AREA_CFG(rAWBStatConfig.i4AWBXY_WINU[8]);//0;
        //TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_AWB_L8_Y, (MUINT32)AwbL8Y.Raw, 0);
        REG_INFO_VALUE(CAM_AWB_L8_Y) = (MUINT32)AwbL8Y.Raw;
        // AWB_L9_X
        CAM_REG_AWB_L9_X AwbL9X;
        AwbL9X.Bits.AWB_L9_X_LOW = AREA_CFG(rAWBStatConfig.i4AWBXY_WINL[9]);//0;
        AwbL9X.Bits.AWB_L9_X_UP = AREA_CFG(rAWBStatConfig.i4AWBXY_WINR[9]);//0;
        //TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_AWB_L9_X, (MUINT32)AwbL9X.Raw, 0);
        REG_INFO_VALUE(CAM_AWB_L9_X) = (MUINT32)AwbL9X.Raw;
        // AWB_L9_Y
        CAM_REG_AWB_L9_Y AwbL9Y;
        AwbL9Y.Bits.AWB_L9_Y_LOW = AREA_CFG(rAWBStatConfig.i4AWBXY_WIND[9]);//0;
        AwbL9Y.Bits.AWB_L9_Y_UP = AREA_CFG(rAWBStatConfig.i4AWBXY_WINU[9]);//0;
        //TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_AWB_L9_Y, (MUINT32)AwbL9Y.Raw, 0);
        REG_INFO_VALUE(CAM_AWB_L9_Y) = (MUINT32)AwbL9Y.Raw;

        //AWB_SPARE

        //AWB_MOTION_THR
        CAM_REG_AWB_MOTION_THR AwbMotionThr;
        AwbMotionThr.Bits.AWB_MOTION_THR = AREA_CFG(rAWBStatConfig.i4MoErrorThreshold);
        AwbMotionThr.Bits.AWB_MOTION_MAP_EN = 1;
        REG_INFO_VALUE(CAM_AWB_MOTION_THR) = (MUINT32)AwbMotionThr.Raw;
    //}

    //apply();

    //addressErrorCheck("After ISP_MGR_AWB_STAT_CONFIG_T::apply()");

 //bd ++
 MUINT32 aa[9] = {0x200, 0, 0, 0, 0x200, 0, 0, 0, 0x200};// Bayer
 SetNonBayer(aa);

        // PSO
        // CAM_PS_AWB_WIN_ORG
        CAM_REG_PS_AWB_WIN_ORG PSAwbWinOrg;
        PSAwbWinOrg.Bits.AWB_W_HORG = rAWBStatConfig.i4WindowOriginX;
        PSAwbWinOrg.Bits.AWB_W_VORG = rAWBStatConfig.i4WindowOriginY;
        //TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_PS_AWB_WIN_ORG, (MUINT32)PSAwbWinOrg.Raw, 0);
        REG_INFO_VALUE(CAM_PS_AWB_WIN_ORG) = (MUINT32)PSAwbWinOrg.Raw;
        // CAM_PS_AWB_WIN_SIZE
        CAM_REG_PS_AWB_WIN_SIZE PSAwbWinSize;
        PSAwbWinSize.Bits.AWB_W_HSIZE = rAWBStatConfig.i4WindowSizeX;
        PSAwbWinSize.Bits.AWB_W_VSIZE = rAWBStatConfig.i4WindowSizeY;
        //TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_PS_AWB_WIN_SIZE, (MUINT32)PSAwbWinSize.Raw, 0);
        REG_INFO_VALUE(CAM_PS_AWB_WIN_SIZE) = (MUINT32)PSAwbWinSize.Raw;
        // CAM_PS_AWB_WIN_PIT
        CAM_REG_PS_AWB_WIN_PIT PSAwbWinPit;
        PSAwbWinPit.Bits.AWB_W_HPIT = rAWBStatConfig.i4WindowPitchX;
        PSAwbWinPit.Bits.AWB_W_VPIT = rAWBStatConfig.i4WindowPitchY;
        //TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_PS_AWB_WIN_PIT, (MUINT32)PSAwbWinPit.Raw, 0);
        REG_INFO_VALUE(CAM_PS_AWB_WIN_PIT) = (MUINT32)PSAwbWinPit.Raw;
        // CAM_PS_AWB_WIN_NUM
        CAM_REG_PS_AWB_WIN_NUM PSAwbWinNum;
        PSAwbWinNum.Bits.AWB_W_HNUM = rAWBStatConfig.i4WindowNumX;
        PSAwbWinNum.Bits.AWB_W_VNUM = rAWBStatConfig.i4WindowNumY;
        //TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_PS_AWB_WIN_NUM, (MUINT32)PSAwbWinNum.Raw, 0);
        REG_INFO_VALUE(CAM_PS_AWB_WIN_NUM) = (MUINT32)PSAwbWinNum.Raw;
        // CAM_PS_AWB_PIXEL_CNT0
        CAM_REG_PS_AWB_PIXEL_CNT0 PSAwbPixelCnt0;
        PSAwbPixelCnt0.Bits.AWB_PIXEL_CNT0 = rAWBStatConfig.i4PixelCountR;
        //TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_PS_AWB_PIXEL_CNT0, (MUINT32)PSAwbPixelCnt0.Raw, 0);
        REG_INFO_VALUE(CAM_PS_AWB_PIXEL_CNT0) = (MUINT32)PSAwbPixelCnt0.Raw;
        // CAM_PS_AWB_PIXEL_CNT1
        CAM_REG_PS_AWB_PIXEL_CNT1 PSAwbPixelCnt1;
        PSAwbPixelCnt1.Bits.AWB_PIXEL_CNT1 = rAWBStatConfig.i4PixelCountR; // same as PSAwbPixelCnt0
        //TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_PS_AWB_PIXEL_CNT1, (MUINT32)PSAwbPixelCnt1.Raw, 0);
        REG_INFO_VALUE(CAM_PS_AWB_PIXEL_CNT1) = (MUINT32)PSAwbPixelCnt1.Raw;
        // CAM_PS_AWB_PIXEL_CNT2
        CAM_REG_PS_AWB_PIXEL_CNT2 PSAwbPixelCnt2;
        PSAwbPixelCnt2.Bits.AWB_PIXEL_CNT2 = rAWBStatConfig.i4PixelCountR; // same as PSAwbPixelCnt0
        //TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_PS_AWB_PIXEL_CNT2, (MUINT32)PSAwbPixelCnt2.Raw, 0);
        REG_INFO_VALUE(CAM_PS_AWB_PIXEL_CNT2) = (MUINT32)PSAwbPixelCnt2.Raw;
        // CAM_PS_AWB_PIXEL_CNT3
        CAM_REG_PS_AWB_PIXEL_CNT3 PSAwbPixelCnt3;
        PSAwbPixelCnt3.Bits.AWB_PIXEL_CNT3 = rAWBStatConfig.i4PixelCountR; // same as PSAwbPixelCnt0
        //TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_PS_AWB_PIXEL_CNT3, (MUINT32)PSAwbPixelCnt3.Raw, 0);
        REG_INFO_VALUE(CAM_PS_AWB_PIXEL_CNT3) = (MUINT32)PSAwbPixelCnt3.Raw;


#endif

    return MTRUE;
}


MBOOL
ISP_MGR_AWB_STAT_CONFIG_T::SetNonBayer(MUINT32 rCscCCM[9])
{
     // RWB
#define RWB_CFG(BOUND)  (BOUND>=0 ? BOUND: (1 << 12) + BOUND)


    // AWB_RC_CNV_0
    CAM_REG_AWB_RC_CNV_0 AwbCNV0;
    AwbCNV0.Bits.AWB_RC_CNV00 = RWB_CFG(rCscCCM[0]);
    AwbCNV0.Bits.AWB_RC_CNV01 = RWB_CFG(rCscCCM[1]);
    REG_INFO_VALUE(CAM_AWB_RC_CNV_0) = (MUINT32)AwbCNV0.Raw;

   // AWB_RC_CNV_1
   CAM_REG_AWB_RC_CNV_1 AwbCNV1;
   AwbCNV1.Bits.AWB_RC_CNV02 = RWB_CFG(rCscCCM[2]);
   AwbCNV1.Bits.AWB_RC_CNV10 = RWB_CFG(rCscCCM[3]);
   REG_INFO_VALUE(CAM_AWB_RC_CNV_1) = (MUINT32)AwbCNV1.Raw;

   // AWB_RC_CNV_2
   CAM_REG_AWB_RC_CNV_2 AwbCNV2;
   AwbCNV2.Bits.AWB_RC_CNV11 = RWB_CFG(rCscCCM[4]);
   AwbCNV2.Bits.AWB_RC_CNV12 = RWB_CFG(rCscCCM[5]);
   REG_INFO_VALUE(CAM_AWB_RC_CNV_2) = (MUINT32)AwbCNV2.Raw;
   // AWB_RC_CNV_3
   CAM_REG_AWB_RC_CNV_3 AwbCNV3;
   AwbCNV3.Bits.AWB_RC_CNV20 =RWB_CFG(rCscCCM[6]);
   AwbCNV3.Bits.AWB_RC_CNV21 = RWB_CFG(rCscCCM[7]);
   REG_INFO_VALUE(CAM_AWB_RC_CNV_3) = (MUINT32)AwbCNV3.Raw;
   // AWB_RC_CNV_4
   CAM_REG_AWB_RC_CNV_4 AwbCNV4;
   AwbCNV4.Bits.AWB_RC_CNV22 = RWB_CFG(rCscCCM[8]);
   AwbCNV4.Bits.AWB_RC_ACC = RWB_CFG(0x09);
   REG_INFO_VALUE(CAM_AWB_RC_CNV_4) = (MUINT32)AwbCNV4.Raw;

   //for (int i = 0; i < 9 ; i++)
   CAM_LOGD_IF(m_bDebugEnable, "rCscCCM %x/%x/%x/%x/%x/%x/%x/%x/%x/", rCscCCM[0], rCscCCM[1], rCscCCM[2], rCscCCM[3], rCscCCM[4], rCscCCM[5],
    rCscCCM[6], rCscCCM[7], rCscCCM[8]);

   return MTRUE;

}
#endif //#if 0


MBOOL
ISP_MGR_AWB_STAT_CONFIG_T::
apply(TuningMgr& rTuning, MINT32 i4SubsampleIdex)
{
    rTuning.updateEngine(eTuningMgrFunc_AA_R1, MTRUE, i4SubsampleIdex);
    //rTuning.updateEngine(eTuningMgrFunc_PS, MTRUE, i4SubsampleIdex);
    // TOP
    //ISP_MGR_CTL_EN_P1_T::getInstance(m_eSensorDev).setEnable_LSC(fgOnOff);


    AAA_TRACE_DRV(DRV_AWB);
    // Register setting
    rTuning.tuningMgrWriteRegs(
        static_cast<TUNING_MGR_REG_IO_STRUCT*>(m_pRegInfo),
        m_u4RegInfoNum, i4SubsampleIdex);
    AAA_TRACE_END_DRV;

    dumpRegInfoP1("awb_stat_cfg");
    return MTRUE;
}

MVOID
ISP_MGR_AWB_STAT_CONFIG_T::
configReg(MVOID *pIspRegInfo)
{
    memcpy(m_rIspRegInfo, (RegInfo_T *)pIspRegInfo, ERegInfo_CAM_AWB_NUM*sizeof(RegInfo_T));
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AWB RAW Pre-gain1
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#if 0
MBOOL
ISP_MGR_AWB_STAT_CONFIG_T::
setIspAWBPreGain1(AWB_GAIN_T& rIspAWBGain)
{
    m_rIspAWBPreGain1 = rIspAWBGain;

    // CAM_AWB_GAIN1_0
    reinterpret_cast<ISP_CAM_AWB_GAIN1_0_T*>(REG_INFO_VALUE_PTR(CAM_AWB_GAIN1_0))->AWB_GAIN1_R = m_rIspAWBPreGain1.i4R;
    reinterpret_cast<ISP_CAM_AWB_GAIN1_0_T*>(REG_INFO_VALUE_PTR(CAM_AWB_GAIN1_0))->AWB_GAIN1_G = m_rIspAWBPreGain1.i4G;
    // CAM_AWB_GAIN1_1
    reinterpret_cast<ISP_CAM_AWB_GAIN1_1_T*>(REG_INFO_VALUE_PTR(CAM_AWB_GAIN1_1))->AWB_GAIN1_B = m_rIspAWBPreGain1.i4B;

    return MTRUE;
}
#endif

MBOOL ISP_MGR_AWB_STAT_CONFIG_T::SetAAOMode(MUINT32 u4AAOmode)
{
    m_u4AAOmode = u4AAOmode;
    return MTRUE;
}

#if 0
/* Dynamic Bin */
MBOOL
ISP_MGR_AWB_STAT_CONFIG_T::
reconfig(MVOID *pDBinInfo, MVOID *pOutRegCfg)
{
    MUINT32 u4Offset = 0;
    MUINT32* pIspReg = NULL;

    BIN_INPUT_INFO *psDBinInfo = static_cast<BIN_INPUT_INFO*>(pDBinInfo);
    Tuning_CFG *psOutRegCfg = static_cast<Tuning_CFG*>(pOutRegCfg);

    CAM_LOGD_IF(m_bDebugEnable, "[%s] : BIN (%d , %d) -> (%d , %d), QBN (%d , %d) -> (%d , %d), RMB (%d , %d) -> (%d , %d)", __FUNCTION__,
                                 psDBinInfo->CurBinOut_W, psDBinInfo->CurBinOut_H, psDBinInfo->TarBinOut_W, psDBinInfo->TarBinOut_H,
                                 psDBinInfo->CurQBNOut_W, psDBinInfo->CurBinOut_H, psDBinInfo->TarQBNOut_W, psDBinInfo->TarBinOut_H,
                                 psDBinInfo->CurRMBOut_W, psDBinInfo->CurBinOut_H, psDBinInfo->TarRMBOut_W, psDBinInfo->TarBinOut_H);
    pIspReg = static_cast<MUINT32*>(psOutRegCfg->pIspReg);

    MUINT32 pitch_size_x =  psDBinInfo->TarBinOut_W / m_blkW;
    MUINT32 pitch_size_y =  psDBinInfo->TarBinOut_H / m_blkH;

    MUINT32 win_size_x =  (pitch_size_x / 2)*2;
    MUINT32 win_size_y =  (pitch_size_y / 2)*2;
    if (win_size_x < 4){
        CAM_LOGD_IF(m_bDebugEnable, "[%s][AWBWindowConfig] win_size_x < 4: %d\n",  __FUNCTION__ ,win_size_x);
        win_size_x = 4;
        }
   else if (win_size_y < 2){
        CAM_LOGD_IF(m_bDebugEnable, "[%s][AWBWindowConfig] win_size_y < 2: %d\n", __FUNCTION__, win_size_y);
        win_size_y = 2;
        }
    MUINT32 win_org_x =  (psDBinInfo->TarBinOut_W - (pitch_size_x * m_blkW ))/2;
    MUINT32 win_org_y =  (((psDBinInfo->TarBinOut_H - (pitch_size_y * m_blkH ))/2)/2)*2;

    MINT32 i4WindowPixelNumR = (win_size_x * win_size_y) / 4;
    MINT32 i4WindowPixelNumG = i4WindowPixelNumR * 2;
    MINT32 i4WindowPixelNumB = i4WindowPixelNumR;
    MINT32 i4PixelCountR = ((1 << 24) + (i4WindowPixelNumR >> 1)) / i4WindowPixelNumR;
    MINT32 i4PixelCountG = ((1 << 24) + (i4WindowPixelNumG >> 1)) / i4WindowPixelNumG;
    MINT32 i4PixelCountB = ((1 << 24) + (i4WindowPixelNumB >> 1)) / i4WindowPixelNumB;

    CAM_LOGD_IF(m_bDebugEnable, "[%s] : New pitx y (%d , %d) win_size_x y (%d , %d), win_org_x y (%d , %d) pix_cnt_R G B ((%d , %d ,%d)", __FUNCTION__,
                                 pitch_size_x, pitch_size_y, win_size_x, win_size_y,
                                 win_org_x, win_org_y, i4PixelCountR, i4PixelCountG, i4PixelCountB);
     //[Update AAO]
     //------------------------------CAM_AWB_WIN_ORG------------------------------------------
     CAM_REG_AWB_WIN_ORG AwbWinOrg;
     u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_AWB_WIN_ORG))/4;
     AwbWinOrg.Raw = *(pIspReg+u4Offset);
     CAM_LOGD_IF(m_bDebugEnable, "[%s] CAM_REG_AWB_WIN_ORG : %x +", __FUNCTION__, *(pIspReg+u4Offset));
     AwbWinOrg.Bits.AWB_W_HORG = win_org_x;
     AwbWinOrg.Bits.AWB_W_VORG = win_org_y;
     *(pIspReg+u4Offset) = AwbWinOrg.Raw;
     CAM_LOGD_IF(m_bDebugEnable, "[%s] CAM_REG_AWB_WIN_ORG : %x -", __FUNCTION__, *(pIspReg+u4Offset));

     //------------------------------CAM_REG_AWB_WIN_SIZE------------------------------------------
     CAM_REG_AWB_WIN_SIZE AwbWinSize;
     u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_AWB_WIN_SIZE))/4;
     AwbWinSize.Raw = *(pIspReg+u4Offset);
     CAM_LOGD_IF(m_bDebugEnable, "[%s] CAM_REG_AWB_WIN_SIZE : %x +", __FUNCTION__, *(pIspReg+u4Offset));
     AwbWinSize.Bits.AWB_W_HSIZE = win_size_x;
     AwbWinSize.Bits.AWB_W_VSIZE = win_size_y;
     *(pIspReg+u4Offset) = AwbWinSize.Raw;
     CAM_LOGD_IF(m_bDebugEnable, "[%s] CAM_REG_AWB_WIN_SIZE : %x -", __FUNCTION__, *(pIspReg+u4Offset));

     //------------------------------CAM_REG_AWB_WIN_PIT------------------------------------------
     CAM_REG_AWB_WIN_PIT AwbWinPit;
     u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_AWB_WIN_PIT))/4;
     AwbWinPit.Raw = *(pIspReg+u4Offset);
     CAM_LOGD_IF(m_bDebugEnable, "[%s] CAM_REG_AWB_WIN_ORG : %x +", __FUNCTION__, *(pIspReg+u4Offset));
     AwbWinPit.Bits.AWB_W_HPIT = pitch_size_x;
     AwbWinPit.Bits.AWB_W_VPIT = pitch_size_y;
     *(pIspReg+u4Offset) = AwbWinPit.Raw;
     CAM_LOGD_IF(m_bDebugEnable, "[%s] CAM_REG_AWB_WIN_ORG : %x -", __FUNCTION__, *(pIspReg+u4Offset));

     //------------------------------CAM_REG_AWB_PIXEL_CNT0------------------------------------------
     CAM_REG_AWB_PIXEL_CNT0 AwbPixelCnt0;
     u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_AWB_PIXEL_CNT0))/4;
     AwbPixelCnt0.Raw = *(pIspReg+u4Offset);
     CAM_LOGD_IF(m_bDebugEnable, "[%s] CAM_REG_AWB_PIXEL_CNT0 : %x +", __FUNCTION__, *(pIspReg+u4Offset));
     AwbPixelCnt0.Bits.AWB_PIXEL_CNT0 = i4PixelCountR;
     *(pIspReg+u4Offset) = AwbPixelCnt0.Raw;
     CAM_LOGD_IF(m_bDebugEnable, "[%s] CAM_REG_AWB_PIXEL_CNT0 : %x -", __FUNCTION__, *(pIspReg+u4Offset));

     //------------------------------CAM_REG_AWB_PIXEL_CNT1------------------------------------------
     CAM_REG_AWB_PIXEL_CNT1 AwbPixelCnt1;
     u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_AWB_PIXEL_CNT1))/4;
     AwbPixelCnt1.Raw = *(pIspReg+u4Offset);
     CAM_LOGD_IF(m_bDebugEnable, "[%s] CAM_REG_AWB_PIXEL_CNT1 : %x +", __FUNCTION__, *(pIspReg+u4Offset));
        AwbPixelCnt1.Bits.AWB_PIXEL_CNT1 = i4PixelCountG;
     *(pIspReg+u4Offset) = AwbPixelCnt1.Raw;
     CAM_LOGD_IF(m_bDebugEnable, "[%s] CAM_REG_AWB_PIXEL_CNT1 : %x -", __FUNCTION__, *(pIspReg+u4Offset));

     //------------------------------CAM_REG_AWB_PIXEL_CNT2------------------------------------------
     CAM_REG_AWB_PIXEL_CNT2 AwbPixelCnt2;
     u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_AWB_PIXEL_CNT2))/4;
     AwbPixelCnt2.Raw = *(pIspReg+u4Offset);
     CAM_LOGD_IF(m_bDebugEnable, "[%s] CAM_REG_AWB_PIXEL_CNT2 : %x +", __FUNCTION__, *(pIspReg+u4Offset));
     AwbPixelCnt2.Bits.AWB_PIXEL_CNT2 = i4PixelCountB;
     *(pIspReg+u4Offset) = AwbPixelCnt2.Raw;
     CAM_LOGD_IF(m_bDebugEnable, "[%s] CAM_REG_AWB_PIXEL_CNT2 : %x -", __FUNCTION__, *(pIspReg+u4Offset));

     //======================================================================================================
     //======================================================================================================
     //[Update PSO]

     //------------------------------CAM_PS_AWB_WIN_ORG------------------------------------------
     CAM_REG_PS_AWB_WIN_ORG PSAwbWinOrg;
     u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_PS_AWB_WIN_ORG))/4;
     PSAwbWinOrg.Raw = *(pIspReg+u4Offset);
     CAM_LOGD_IF(m_bDebugEnable, "[%s] CAM_REG_PS_AWB_WIN_ORG : %x +", __FUNCTION__, *(pIspReg+u4Offset));
     PSAwbWinOrg.Bits.AWB_W_HORG = win_org_x;
     PSAwbWinOrg.Bits.AWB_W_VORG = win_org_y;
     *(pIspReg+u4Offset) = PSAwbWinOrg.Raw;
     CAM_LOGD_IF(m_bDebugEnable, "[%s] CAM_REG_PS_AWB_WIN_ORG : %x -", __FUNCTION__, *(pIspReg+u4Offset));

     //------------------------------CAM_REG_PS_AWB_WIN_SIZE------------------------------------------
     CAM_REG_PS_AWB_WIN_SIZE PSAwbWinSize;
     u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_PS_AWB_WIN_SIZE))/4;
     PSAwbWinSize.Raw = *(pIspReg+u4Offset);
     CAM_LOGD_IF(m_bDebugEnable, "[%s] CAM_REG_PS_AWB_WIN_SIZE : %x +", __FUNCTION__, *(pIspReg+u4Offset));
     PSAwbWinSize.Bits.AWB_W_HSIZE = win_size_x;
     PSAwbWinSize.Bits.AWB_W_VSIZE = win_size_y;
     *(pIspReg+u4Offset) = PSAwbWinSize.Raw;
     CAM_LOGD_IF(m_bDebugEnable, "[%s] CAM_REG_PS_AWB_WIN_SIZE : %x -", __FUNCTION__, *(pIspReg+u4Offset));

     //------------------------------CAM_REG_AWB_WIN_PIT------------------------------------------
     CAM_REG_PS_AWB_WIN_PIT PSAwbWinPit;
     u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_PS_AWB_WIN_PIT))/4;
     PSAwbWinPit.Raw = *(pIspReg+u4Offset);
     CAM_LOGD_IF(m_bDebugEnable, "[%s] CAM_REG_PS_AWB_WIN_PIT : %x +", __FUNCTION__, *(pIspReg+u4Offset));
     PSAwbWinPit.Bits.AWB_W_HPIT = pitch_size_x;
     PSAwbWinPit.Bits.AWB_W_VPIT = pitch_size_y;
     *(pIspReg+u4Offset) = PSAwbWinPit.Raw;
     CAM_LOGD_IF(m_bDebugEnable, "[%s] CAM_REG_PS_AWB_WIN_PIT : %x -", __FUNCTION__, *(pIspReg+u4Offset));

     //------------------------------CAM_REG_AWB_PIXEL_CNT0------------------------------------------
     CAM_REG_PS_AWB_PIXEL_CNT0 PSAwbPixelCnt0;
     u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_PS_AWB_PIXEL_CNT0))/4;
     PSAwbPixelCnt0.Raw = *(pIspReg+u4Offset);
     CAM_LOGD_IF(m_bDebugEnable, "[%s] CAM_REG_AWB_PIXEL_CNT0 : %x +", __FUNCTION__, *(pIspReg+u4Offset));
     PSAwbPixelCnt0.Bits.AWB_PIXEL_CNT0 = i4PixelCountR;
     *(pIspReg+u4Offset) = PSAwbPixelCnt0.Raw;
     CAM_LOGD_IF(m_bDebugEnable, "[%s] CAM_REG_AWB_PIXEL_CNT0 : %x -", __FUNCTION__, *(pIspReg+u4Offset));

     //------------------------------CAM_REG_PS_AWB_PIXEL_CNT1------------------------------------------
     CAM_REG_PS_AWB_PIXEL_CNT1 PSAwbPixelCnt1;
     u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_PS_AWB_PIXEL_CNT1))/4;
     PSAwbPixelCnt1.Raw = *(pIspReg+u4Offset);
     CAM_LOGD_IF(m_bDebugEnable, "[%s] CAM_REG_PS_AWB_PIXEL_CNT1 : %x +", __FUNCTION__, *(pIspReg+u4Offset));
     PSAwbPixelCnt1.Bits.AWB_PIXEL_CNT1 = i4PixelCountR;
     *(pIspReg+u4Offset) = PSAwbPixelCnt1.Raw;
     CAM_LOGD_IF(m_bDebugEnable, "[%s] CAM_REG_PS_AWB_PIXEL_CNT1 : %x -", __FUNCTION__, *(pIspReg+u4Offset));

     //------------------------------CAM_REG_AWB_PIXEL_CNT2------------------------------------------
     CAM_REG_PS_AWB_PIXEL_CNT2 PSAwbPixelCnt2;
     u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_PS_AWB_PIXEL_CNT2))/4;
     PSAwbPixelCnt2.Raw = *(pIspReg+u4Offset);
     CAM_LOGD_IF(m_bDebugEnable, "[%s] CAM_REG_AWB_PIXEL_CNT2 : %x +", __FUNCTION__, *(pIspReg+u4Offset));
     PSAwbPixelCnt2.Bits.AWB_PIXEL_CNT2 = i4PixelCountR;
     *(pIspReg+u4Offset) = PSAwbPixelCnt2.Raw;
     CAM_LOGD_IF(m_bDebugEnable, "[%s] CAM_REG_AWB_PIXEL_CNT2 : %x -", __FUNCTION__, *(pIspReg+u4Offset));

     //------------------------------CAM_REG_AWB_PIXEL_CNT3------------------------------------------
     CAM_REG_PS_AWB_PIXEL_CNT3 PSAwbPixelCnt3;
     u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_PS_AWB_PIXEL_CNT3))/4;
     PSAwbPixelCnt3.Raw = *(pIspReg+u4Offset);
     CAM_LOGD_IF(m_bDebugEnable, "[%s] CAM_REG_PS_AWB_PIXEL_CNT3 : %x +", __FUNCTION__, *(pIspReg+u4Offset));
     PSAwbPixelCnt3.Bits.AWB_PIXEL_CNT3 = i4PixelCountR;
    *(pIspReg+u4Offset) = PSAwbPixelCnt3.Raw;
     CAM_LOGD_IF(m_bDebugEnable, "[%s] CAM_REG_PS_AWB_PIXEL_CNT3 : %x -", __FUNCTION__, *(pIspReg+u4Offset));

    return MTRUE;
}


/* Call back */
MBOOL
ISP_MGR_AWB_STAT_CONFIG_T::
Cbreconfig(AWB_STAT_CONFIG_T& rAWBStatConfig, AWB_OUTPUT_T& m_rAWBOutput, MUINT32 m_i4FlareGain, MUINT32 m_i4FlareOffset, MVOID *pDBinInfo, MVOID *pOutRegCfg)
{

    MUINT32 u4Offset = 0;
    MUINT32* pIspReg = NULL;
    BIN_INPUT_INFO *psDBinInfo = static_cast<BIN_INPUT_INFO*>(pDBinInfo);
    Tuning_CFG *psOutRegCfg = static_cast<Tuning_CFG*>(pOutRegCfg);
    pIspReg = static_cast<MUINT32*>(psOutRegCfg->pIspReg);

    MUINT32 m_rIspR = (m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R * m_i4FlareGain + (AWB_SCALE_UNIT >> 1)) / AWB_SCALE_UNIT;
    MUINT32 m_rIspG = (m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4G * m_i4FlareGain + (AWB_SCALE_UNIT >> 1)) / AWB_SCALE_UNIT;
    MUINT32 m_rIspB = (m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4B * m_i4FlareGain + (AWB_SCALE_UNIT >> 1)) / AWB_SCALE_UNIT;

    // ISP flare offset
    MUINT32 m_i4IspFlareOffset = (m_i4FlareOffset * m_i4FlareGain + (AWB_SCALE_UNIT >> 1)) / AWB_SCALE_UNIT;
    MUINT32 OFFS = (m_i4IspFlareOffset >= 0) ? static_cast<MUINT32>(m_i4IspFlareOffset) : static_cast<MUINT32>(4096 + m_i4IspFlareOffset);


    char valueAA[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.awbrpg.enable", valueAA, "0"); //zero enable
    int AA = atoi(valueAA);


     char valueRPG[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.awbrpg.enable", valueRPG, "0");
    int RPG = atoi(valueRPG);

   psOutRegCfg->Bypass.Bits.AA =AA;
   psOutRegCfg->Bypass.Bits.RPG =RPG;

    char valueR[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.r.gain", valueR, "0");
    int R = atoi(valueR);

    char valueB[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.b.gain", valueB, "0");
    int B = atoi(valueB);


#define AREA_CFG(BOUND)  (BOUND>=0 ? BOUND: (1 << 14) + BOUND)
     //------------------------------CAM_REG_AWB_L0------------------------------------------

        // AWB_L0_X
        CAM_REG_AWB_L0_X AwbL0X;
        u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_AWB_L0_X))/4;
        AwbL0X.Raw = *(pIspReg+u4Offset);
        CAM_LOGD_IF(m_bDebugEnable, "[%s] AwbL0X : %x +", __FUNCTION__, *(pIspReg+u4Offset));
        AwbL0X.Bits.AWB_L0_X_LOW = AREA_CFG(rAWBStatConfig.i4AWBXY_WINL[0]);//-250;
        AwbL0X.Bits.AWB_L0_X_UP = AREA_CFG(rAWBStatConfig.i4AWBXY_WINR[0]);//-100;
        *(pIspReg+u4Offset) = AwbL0X.Raw;
        CAM_LOGD_IF(m_bDebugEnable, "[%s] AwbL0X : %x -", __FUNCTION__, *(pIspReg+u4Offset));

        CAM_REG_AWB_L0_Y AwbL0Y;
        u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_AWB_L0_Y))/4;
        AwbL0Y.Raw = *(pIspReg+u4Offset);
        CAM_LOGD_IF(m_bDebugEnable, "[%s] AwbL0Y : %x +", __FUNCTION__, *(pIspReg+u4Offset));
        AwbL0Y.Bits.AWB_L0_Y_LOW = AREA_CFG(rAWBStatConfig.i4AWBXY_WIND[0]);//-250;
        AwbL0Y.Bits.AWB_L0_Y_UP = AREA_CFG(rAWBStatConfig.i4AWBXY_WINU[0]);//-100;
        *(pIspReg+u4Offset) = AwbL0Y.Raw;
        CAM_LOGD_IF(m_bDebugEnable, "[%s] AwbL0Y : %x -", __FUNCTION__, *(pIspReg+u4Offset));

     //------------------------------CAM_REG_AWB_L1------------------------------------------
        CAM_REG_AWB_L1_X AwbL1X;
        u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_AWB_L1_X))/4;
        AwbL1X.Raw = *(pIspReg+u4Offset);
        CAM_LOGD_IF(m_bDebugEnable, "[%s] AwbL1X : %x +", __FUNCTION__, *(pIspReg+u4Offset));
        AwbL1X.Bits.AWB_L1_X_LOW = AREA_CFG(rAWBStatConfig.i4AWBXY_WINL[1]);//-250;
        AwbL1X.Bits.AWB_L1_X_UP = AREA_CFG(rAWBStatConfig.i4AWBXY_WINR[1]);//-100;
        *(pIspReg+u4Offset) = AwbL1X.Raw;
        CAM_LOGD_IF(m_bDebugEnable, "[%s] AwbL1X : %x -", __FUNCTION__, *(pIspReg+u4Offset));

        CAM_REG_AWB_L1_Y AwbL1Y;
        u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_AWB_L1_Y))/4;
        AwbL1Y.Raw = *(pIspReg+u4Offset);
        CAM_LOGD_IF(m_bDebugEnable, "[%s] AwbL1Y : %x +", __FUNCTION__, *(pIspReg+u4Offset));
        AwbL1Y.Bits.AWB_L1_Y_LOW = AREA_CFG(rAWBStatConfig.i4AWBXY_WIND[1]);//-250;
        AwbL1Y.Bits.AWB_L1_Y_UP = AREA_CFG(rAWBStatConfig.i4AWBXY_WINU[1]);//-100;
        *(pIspReg+u4Offset) = AwbL1Y.Raw;
        CAM_LOGD_IF(m_bDebugEnable, "[%s] AwbL1Y : %x -", __FUNCTION__, *(pIspReg+u4Offset));

     //------------------------------CAM_REG_AWB_L2------------------------------------------
        CAM_REG_AWB_L2_X AwbL2X;
        u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_AWB_L2_X))/4;
        AwbL2X.Raw = *(pIspReg+u4Offset);
        CAM_LOGD_IF(m_bDebugEnable, "[%s] AwbL2X : %x +", __FUNCTION__, *(pIspReg+u4Offset));
        AwbL2X.Bits.AWB_L2_X_LOW = AREA_CFG(rAWBStatConfig.i4AWBXY_WINL[2]);//-250;
        AwbL2X.Bits.AWB_L2_X_UP = AREA_CFG(rAWBStatConfig.i4AWBXY_WINR[2]);//-100;
        *(pIspReg+u4Offset) = AwbL2X.Raw;
        CAM_LOGD_IF(m_bDebugEnable, "[%s] AwbL2X : %x -", __FUNCTION__, *(pIspReg+u4Offset));

        CAM_REG_AWB_L2_Y AwbL2Y;
        u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_AWB_L2_Y))/4;
        AwbL2Y.Raw = *(pIspReg+u4Offset);
        CAM_LOGD_IF(m_bDebugEnable, "[%s] AwbL2Y : %x +", __FUNCTION__, *(pIspReg+u4Offset));
        AwbL2Y.Bits.AWB_L2_Y_LOW = AREA_CFG(rAWBStatConfig.i4AWBXY_WIND[2]);//-250;
        AwbL2Y.Bits.AWB_L2_Y_UP = AREA_CFG(rAWBStatConfig.i4AWBXY_WINU[2]);//-100;
        *(pIspReg+u4Offset) = AwbL2Y.Raw;
        CAM_LOGD_IF(m_bDebugEnable, "[%s] AwbL2Y : %x -", __FUNCTION__, *(pIspReg+u4Offset));
     //------------------------------CAM_REG_AWB_L3------------------------------------------
        CAM_REG_AWB_L3_X AwbL3X;
        u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_AWB_L3_X))/4;
        AwbL3X.Raw = *(pIspReg+u4Offset);
        CAM_LOGD_IF(m_bDebugEnable, "[%s] AwbL3X : %x +", __FUNCTION__, *(pIspReg+u4Offset));
        AwbL3X.Bits.AWB_L3_X_LOW = AREA_CFG(rAWBStatConfig.i4AWBXY_WINL[3]);//-250;
        AwbL3X.Bits.AWB_L3_X_UP = AREA_CFG(rAWBStatConfig.i4AWBXY_WINR[3]);//-100;
        *(pIspReg+u4Offset) = AwbL3X.Raw;
        CAM_LOGD_IF(m_bDebugEnable, "[%s] AwbL3X : %x -", __FUNCTION__, *(pIspReg+u4Offset));

        CAM_REG_AWB_L3_Y AwbL3Y;
        u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_AWB_L3_Y))/4;
        AwbL3Y.Raw = *(pIspReg+u4Offset);
        CAM_LOGD_IF(m_bDebugEnable, "[%s] AwbL3Y : %x +", __FUNCTION__, *(pIspReg+u4Offset));
        AwbL3Y.Bits.AWB_L3_Y_LOW = AREA_CFG(rAWBStatConfig.i4AWBXY_WIND[3]);//-250;
        AwbL3Y.Bits.AWB_L3_Y_UP = AREA_CFG(rAWBStatConfig.i4AWBXY_WINU[3]);//-100;
        *(pIspReg+u4Offset) = AwbL3Y.Raw;
        CAM_LOGD_IF(m_bDebugEnable, "[%s] AwbL3Y : %x -", __FUNCTION__, *(pIspReg+u4Offset));
     //------------------------------CAM_REG_AWB_L4------------------------------------------
        CAM_REG_AWB_L4_X AwbL4X;
        u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_AWB_L4_X))/4;
        AwbL4X.Raw = *(pIspReg+u4Offset);
        CAM_LOGD_IF(m_bDebugEnable, "[%s] AwbL4X : %x +", __FUNCTION__, *(pIspReg+u4Offset));
        AwbL4X.Bits.AWB_L4_X_LOW = AREA_CFG(rAWBStatConfig.i4AWBXY_WINL[4]);//-250;
        AwbL4X.Bits.AWB_L4_X_UP = AREA_CFG(rAWBStatConfig.i4AWBXY_WINR[4]);//-100;
        *(pIspReg+u4Offset) = AwbL4X.Raw;
        CAM_LOGD_IF(m_bDebugEnable, "[%s] AwbL4X : %x -", __FUNCTION__, *(pIspReg+u4Offset));

        CAM_REG_AWB_L4_Y AwbL4Y;
        u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_AWB_L4_Y))/4;
        AwbL4Y.Raw = *(pIspReg+u4Offset);
        CAM_LOGD_IF(m_bDebugEnable, "[%s] AwbL4Y : %x +", __FUNCTION__, *(pIspReg+u4Offset));
        AwbL4Y.Bits.AWB_L4_Y_LOW = AREA_CFG(rAWBStatConfig.i4AWBXY_WIND[4]);//-250;
        AwbL4Y.Bits.AWB_L4_Y_UP = AREA_CFG(rAWBStatConfig.i4AWBXY_WINU[4]);//-100;
        *(pIspReg+u4Offset) = AwbL4Y.Raw;
        CAM_LOGD_IF(m_bDebugEnable, "[%s] AwbL4Y : %x -", __FUNCTION__, *(pIspReg+u4Offset));
     //------------------------------CAM_REG_AWB_L5------------------------------------------
        CAM_REG_AWB_L5_X AwbL5X;
        u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_AWB_L5_X))/4;
        AwbL5X.Raw = *(pIspReg+u4Offset);
        CAM_LOGD_IF(m_bDebugEnable, "[%s] AwbL5X : %x +", __FUNCTION__, *(pIspReg+u4Offset));
        AwbL5X.Bits.AWB_L5_X_LOW = AREA_CFG(rAWBStatConfig.i4AWBXY_WINL[5]);//-250;
        AwbL5X.Bits.AWB_L5_X_UP = AREA_CFG(rAWBStatConfig.i4AWBXY_WINR[5]);//-100;
        *(pIspReg+u4Offset) = AwbL5X.Raw;
        CAM_LOGD_IF(m_bDebugEnable, "[%s] AwbL5X : %x -", __FUNCTION__, *(pIspReg+u4Offset));

        CAM_REG_AWB_L5_Y AwbL5Y;
        u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_AWB_L5_Y))/4;
        AwbL5Y.Raw = *(pIspReg+u4Offset);
        CAM_LOGD_IF(m_bDebugEnable, "[%s] AwbL5Y : %x +", __FUNCTION__, *(pIspReg+u4Offset));
        AwbL5Y.Bits.AWB_L5_Y_LOW = AREA_CFG(rAWBStatConfig.i4AWBXY_WIND[5]);//-250;
        AwbL5Y.Bits.AWB_L5_Y_UP = AREA_CFG(rAWBStatConfig.i4AWBXY_WINU[5]);//-100;
        *(pIspReg+u4Offset) = AwbL5Y.Raw;
        CAM_LOGD_IF(m_bDebugEnable, "[%s] AwbL5Y : %x -", __FUNCTION__, *(pIspReg+u4Offset));
     //------------------------------CAM_REG_AWB_L6------------------------------------------
        CAM_REG_AWB_L6_X AwbL6X;
        u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_AWB_L6_X))/4;
        AwbL6X.Raw = *(pIspReg+u4Offset);
        CAM_LOGD_IF(m_bDebugEnable, "[%s] AwbL6X : %x +", __FUNCTION__, *(pIspReg+u4Offset));
        AwbL6X.Bits.AWB_L6_X_LOW = AREA_CFG(rAWBStatConfig.i4AWBXY_WINL[6]);//-250;
        AwbL6X.Bits.AWB_L6_X_UP = AREA_CFG(rAWBStatConfig.i4AWBXY_WINR[6]);//-100;
        *(pIspReg+u4Offset) = AwbL6X.Raw;
        CAM_LOGD_IF(m_bDebugEnable, "[%s] AwbL6X : %x -", __FUNCTION__, *(pIspReg+u4Offset));

        CAM_REG_AWB_L6_Y AwbL6Y;
        u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_AWB_L6_Y))/4;
        AwbL6Y.Raw = *(pIspReg+u4Offset);
        CAM_LOGD_IF(m_bDebugEnable, "[%s] AwbL6Y : %x +", __FUNCTION__, *(pIspReg+u4Offset));
        AwbL6Y.Bits.AWB_L6_Y_LOW = AREA_CFG(rAWBStatConfig.i4AWBXY_WIND[6]);//-250;
        AwbL6Y.Bits.AWB_L6_Y_UP = AREA_CFG(rAWBStatConfig.i4AWBXY_WINU[6]);//-100;
        *(pIspReg+u4Offset) = AwbL6Y.Raw;
        CAM_LOGD_IF(m_bDebugEnable, "[%s] AwbL6Y : %x -", __FUNCTION__, *(pIspReg+u4Offset));
     //------------------------------CAM_REG_AWB_L7------------------------------------------
        CAM_REG_AWB_L7_X AwbL7X;
        u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_AWB_L7_X))/4;
        AwbL7X.Raw = *(pIspReg+u4Offset);
        CAM_LOGD_IF(m_bDebugEnable, "[%s] AwbL7X : %x +", __FUNCTION__, *(pIspReg+u4Offset));
        AwbL7X.Bits.AWB_L7_X_LOW = AREA_CFG(rAWBStatConfig.i4AWBXY_WINL[7]);//-250;
        AwbL7X.Bits.AWB_L7_X_UP = AREA_CFG(rAWBStatConfig.i4AWBXY_WINR[7]);//-100;
        *(pIspReg+u4Offset) = AwbL7X.Raw;
        CAM_LOGD_IF(m_bDebugEnable, "[%s] AwbL7X : %x -", __FUNCTION__, *(pIspReg+u4Offset));

        CAM_REG_AWB_L7_Y AwbL7Y;
        u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_AWB_L7_Y))/4;
        AwbL7Y.Raw = *(pIspReg+u4Offset);
        CAM_LOGD_IF(m_bDebugEnable, "[%s] AwbL7Y : %x +", __FUNCTION__, *(pIspReg+u4Offset));
        AwbL7Y.Bits.AWB_L7_Y_LOW = AREA_CFG(rAWBStatConfig.i4AWBXY_WIND[7]);//-250;
        AwbL7Y.Bits.AWB_L7_Y_UP = AREA_CFG(rAWBStatConfig.i4AWBXY_WINU[7]);//-100;
        *(pIspReg+u4Offset) = AwbL7Y.Raw;
        CAM_LOGD_IF(m_bDebugEnable, "[%s] AwbL7Y : %x -", __FUNCTION__, *(pIspReg+u4Offset));
     //------------------------------CAM_REG_AWB_L8------------------------------------------
        CAM_REG_AWB_L8_X AwbL8X;
        u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_AWB_L8_X))/4;
        AwbL8X.Raw = *(pIspReg+u4Offset);
        CAM_LOGD_IF(m_bDebugEnable, "[%s] AwbL8X : %x +", __FUNCTION__, *(pIspReg+u4Offset));
        AwbL8X.Bits.AWB_L8_X_LOW = AREA_CFG(rAWBStatConfig.i4AWBXY_WINL[8]);//-250;
        AwbL8X.Bits.AWB_L8_X_UP = AREA_CFG(rAWBStatConfig.i4AWBXY_WINR[8]);//-100;
        *(pIspReg+u4Offset) = AwbL8X.Raw;
        CAM_LOGD_IF(m_bDebugEnable, "[%s] AwbL8X : %x -", __FUNCTION__, *(pIspReg+u4Offset));

        CAM_REG_AWB_L8_Y AwbL8Y;
        u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_AWB_L8_Y))/4;
        AwbL8Y.Raw = *(pIspReg+u4Offset);
        CAM_LOGD_IF(m_bDebugEnable, "[%s] AwbL8Y : %x +", __FUNCTION__, *(pIspReg+u4Offset));
        AwbL8Y.Bits.AWB_L8_Y_LOW = AREA_CFG(rAWBStatConfig.i4AWBXY_WIND[8]);//-250;
        AwbL8Y.Bits.AWB_L8_Y_UP = AREA_CFG(rAWBStatConfig.i4AWBXY_WINU[8]);//-100;
        *(pIspReg+u4Offset) = AwbL8Y.Raw;
        CAM_LOGD_IF(m_bDebugEnable, "[%s] AwbL8Y : %x -", __FUNCTION__, *(pIspReg+u4Offset));
     //------------------------------CAM_REG_AWB_L9------------------------------------------
        CAM_REG_AWB_L9_X AwbL9X;
        u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_AWB_L9_X))/4;
        AwbL9X.Raw = *(pIspReg+u4Offset);
        CAM_LOGD_IF(m_bDebugEnable, "[%s] AwbL9X : %x +", __FUNCTION__, *(pIspReg+u4Offset));
        AwbL9X.Bits.AWB_L9_X_LOW = AREA_CFG(rAWBStatConfig.i4AWBXY_WINL[9]);//-250;
        AwbL9X.Bits.AWB_L9_X_UP = AREA_CFG(rAWBStatConfig.i4AWBXY_WINR[9]);//-100;
        *(pIspReg+u4Offset) = AwbL9X.Raw;
        CAM_LOGD_IF(m_bDebugEnable, "[%s] AwbL9X : %x -", __FUNCTION__, *(pIspReg+u4Offset));

        CAM_REG_AWB_L9_Y AwbL9Y;
        u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_AWB_L9_Y))/4;
        AwbL9Y.Raw = *(pIspReg+u4Offset);
        CAM_LOGD_IF(m_bDebugEnable, "[%s] AwbL9Y : %x +", __FUNCTION__, *(pIspReg+u4Offset));
        AwbL9Y.Bits.AWB_L9_Y_LOW = AREA_CFG(rAWBStatConfig.i4AWBXY_WIND[9]);//-250;
        AwbL9Y.Bits.AWB_L9_Y_UP = AREA_CFG(rAWBStatConfig.i4AWBXY_WINU[9]);//-100;
        *(pIspReg+u4Offset) = AwbL9X.Raw;
        CAM_LOGD_IF(m_bDebugEnable, "[%s] AwbL9Y : %x -", __FUNCTION__, *(pIspReg+u4Offset));

     //------------------------------RPG SATU------------------------------------------
        _CAM_REG_RPG_SATU_1_ RpgSatu1;
        u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_RPG_SATU_1))/4;
        RpgSatu1.Raw = *(pIspReg+u4Offset);
        CAM_LOGD_IF(m_bDebugEnable, "[%s] RpgSatu1 : %x +", __FUNCTION__, *(pIspReg+u4Offset));
        RpgSatu1.Bits.RPG_SATU_B = 0xFFF;
        RpgSatu1.Bits.RPG_SATU_GB = 0xFFF;
        *(pIspReg+u4Offset) = RpgSatu1.Raw;
        CAM_LOGD_IF(m_bDebugEnable, "[%s] RpgSatu1 : %x -", __FUNCTION__, *(pIspReg+u4Offset));

        _CAM_REG_RPG_SATU_2_ RpgSatu2;
        u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_RPG_SATU_2))/4;
        RpgSatu2.Raw = *(pIspReg+u4Offset);
        CAM_LOGD_IF(m_bDebugEnable, "[%s] RpgSatu2 : %x +", __FUNCTION__, *(pIspReg+u4Offset));
        RpgSatu2.Bits.RPG_SATU_GR = 0xFFF;
        RpgSatu2.Bits.RPG_SATU_R = 0xFFF;
        *(pIspReg+u4Offset) = RpgSatu2.Raw;
        CAM_LOGD_IF(m_bDebugEnable, "[%s] RpgSatu2 : %x -", __FUNCTION__, *(pIspReg+u4Offset));
     //------------------------------RPG GAIN1/ GAIN2------------------------------------------
        CAM_REG_RPG_GAIN_1 RpgGain1;
        u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_RPG_GAIN_1))/4;
        RpgGain1.Raw = *(pIspReg+u4Offset);
        CAM_LOGD("[%s] RpgGain1 : %x +", __FUNCTION__, *(pIspReg+u4Offset));
        if (B == 0)
           RpgGain1.Bits.RPG_GAIN_B = m_rIspB;//AREA_CFG( m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4B);//-250;
        else
           RpgGain1.Bits.RPG_GAIN_B = B;//AREA_CFG( m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4B);//-250;

        RpgGain1.Bits.RPG_GAIN_GB = m_rIspG;//AREA_CFG( m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4G);//-100;
        *(pIspReg+u4Offset) = RpgGain1.Raw;
        CAM_LOGD("[%s] RpgGain1 : %x -", __FUNCTION__, *(pIspReg+u4Offset));

        CAM_REG_RPG_GAIN_2 RpgGain2;
        u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_RPG_GAIN_2))/4;
        RpgGain2.Raw = *(pIspReg+u4Offset);
        CAM_LOGD("[%s] RpgGain2 : %x +", __FUNCTION__, *(pIspReg+u4Offset));
        RpgGain2.Bits.RPG_GAIN_GR = m_rIspG;//512;//AREA_CFG( m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4G);//-250;
        if (R == 0)
           RpgGain2.Bits.RPG_GAIN_R = m_rIspR;//AREA_CFG( m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R);//-100;
        else
           RpgGain2.Bits.RPG_GAIN_R = R;//AREA_CFG( m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R);//-100;

        *(pIspReg+u4Offset) = RpgGain2.Raw;
        CAM_LOGD("[%s] RpgGain2 : %x -", __FUNCTION__, *(pIspReg+u4Offset));
      //------------------------------RPG OFFSET------------------------------------------
        _CAM_REG_RPG_OFST_1_ RpgOfs1;
        u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_RPG_OFST_1))/4;
        RpgOfs1.Raw = *(pIspReg+u4Offset);
        CAM_LOGD_IF(m_bDebugEnable, "[%s] RpgOfs1 : %x +", __FUNCTION__, *(pIspReg+u4Offset));
        RpgOfs1.Bits.RPG_OFST_B = OFFS;
        RpgOfs1.Bits.RPG_OFST_GB = OFFS;
        *(pIspReg+u4Offset) = RpgOfs1.Raw;
        CAM_LOGD_IF(m_bDebugEnable, "[%s] RpgOfs1 : %x -", __FUNCTION__, *(pIspReg+u4Offset));

        _CAM_REG_RPG_OFST_2_ RpgOfs2;
        u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_RPG_OFST_2))/4;
        RpgOfs2.Raw = *(pIspReg+u4Offset);
        CAM_LOGD_IF(m_bDebugEnable, "[%s] RpgOfs2 : %x +", __FUNCTION__, *(pIspReg+u4Offset));
        RpgOfs2.Bits.RPG_OFST_GR = OFFS;
        RpgOfs2.Bits.RPG_OFST_R = OFFS;
        *(pIspReg+u4Offset) = RpgOfs2.Raw;
        CAM_LOGD_IF(m_bDebugEnable, "[%s] RpgOfs2 : %x -", __FUNCTION__, *(pIspReg+u4Offset));

    return MTRUE;
}
#endif

#if 0
MBOOL
ISP_MGR_AWB_STAT_CONFIG_T::
apply_TG1()
{
    CAM_LOGD_IF(IsDebugEnabled(), "%s(): m_eSensorDev = %d, m_i4SensorIndex = %d\n", __FUNCTION__, m_eSensorDev, m_i4SensorIndex);
    dumpRegInfo("AWB_STAT_TG1");

    MUINTPTR handle;

    INormalPipe* pPipe = INormalPipe::createInstance(m_i4SensorIndex,"isp_mgr_awb_stat_tg1");//iopipe2.0

    // get module handle
    if (MFALSE == pPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_GET_MODULE_HANDLE,
                           NSImageio::NSIspio::EModule_AWB, (MINTPTR)&handle, (MINTPTR)(&("isp_mgr_awb_stat_tg1"))))
    {
        //Error Handling
        CAM_LOGE("EPIPECmd_GET_MODULE_HANDLE fail");
        goto lbExit;
    }

    // set module register
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_WIN_ORG, m_rIspRegInfo[ERegInfo_CAM_AWB_WIN_ORG].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_WIN_SIZE, m_rIspRegInfo[ERegInfo_CAM_AWB_WIN_SIZE].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_WIN_PIT, m_rIspRegInfo[ERegInfo_CAM_AWB_WIN_PIT].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_WIN_NUM, m_rIspRegInfo[ERegInfo_CAM_AWB_WIN_NUM].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_GAIN1_0, m_rIspRegInfo[ERegInfo_CAM_AWB_GAIN1_0].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_GAIN1_1, m_rIspRegInfo[ERegInfo_CAM_AWB_GAIN1_1].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_LMT1_0, m_rIspRegInfo[ERegInfo_CAM_AWB_LMT1_0].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_LMT1_1, m_rIspRegInfo[ERegInfo_CAM_AWB_LMT1_1].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_LOW_THR, m_rIspRegInfo[ERegInfo_CAM_AWB_LOW_THR].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_HI_THR, m_rIspRegInfo[ERegInfo_CAM_AWB_HI_THR].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_PIXEL_CNT0, m_rIspRegInfo[ERegInfo_CAM_AWB_PIXEL_CNT0].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_PIXEL_CNT1, m_rIspRegInfo[ERegInfo_CAM_AWB_PIXEL_CNT1].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_PIXEL_CNT2, m_rIspRegInfo[ERegInfo_CAM_AWB_PIXEL_CNT2].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_ERR_THR, m_rIspRegInfo[ERegInfo_CAM_AWB_ERR_THR].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_ROT, m_rIspRegInfo[ERegInfo_CAM_AWB_ROT].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_L0_X, m_rIspRegInfo[ERegInfo_CAM_AWB_L0_X].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_L0_Y, m_rIspRegInfo[ERegInfo_CAM_AWB_L0_Y].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_L1_X, m_rIspRegInfo[ERegInfo_CAM_AWB_L1_X].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_L1_Y, m_rIspRegInfo[ERegInfo_CAM_AWB_L1_Y].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_L2_X, m_rIspRegInfo[ERegInfo_CAM_AWB_L2_X].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_L2_Y, m_rIspRegInfo[ERegInfo_CAM_AWB_L2_Y].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_L3_X, m_rIspRegInfo[ERegInfo_CAM_AWB_L3_X].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_L3_Y, m_rIspRegInfo[ERegInfo_CAM_AWB_L3_Y].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_L4_X, m_rIspRegInfo[ERegInfo_CAM_AWB_L4_X].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_L4_Y, m_rIspRegInfo[ERegInfo_CAM_AWB_L4_Y].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_L5_X, m_rIspRegInfo[ERegInfo_CAM_AWB_L5_X].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_L5_Y, m_rIspRegInfo[ERegInfo_CAM_AWB_L5_Y].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_L6_X, m_rIspRegInfo[ERegInfo_CAM_AWB_L6_X].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_L6_Y, m_rIspRegInfo[ERegInfo_CAM_AWB_L6_Y].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_L7_X, m_rIspRegInfo[ERegInfo_CAM_AWB_L7_X].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_L7_Y, m_rIspRegInfo[ERegInfo_CAM_AWB_L7_Y].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_L8_X, m_rIspRegInfo[ERegInfo_CAM_AWB_L8_X].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_L8_Y, m_rIspRegInfo[ERegInfo_CAM_AWB_L8_Y].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_L9_X, m_rIspRegInfo[ERegInfo_CAM_AWB_L9_X].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_L9_Y, m_rIspRegInfo[ERegInfo_CAM_AWB_L9_Y].val);

    // set module config done
    if (MFALSE==pPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_SET_MODULE_CFG_DONE, handle, MNULL, MNULL))
    {
        //Error Handling
        CAM_LOGE("EPIPECmd_SET_MODULE_CFG_DONE fail");
        goto lbExit;
    }

lbExit:

    // release handle
    if (MFALSE==pPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_RELEASE_MODULE_HANDLE, handle, (MINTPTR)(&("isp_mgr_awb_stat_tg1")), MNULL))
    {
        // Error Handling
        CAM_LOGE("EPIPECmd_SET_MODULE_CFG_DONE fail");
    }

    pPipe->destroyInstance("isp_mgr_awb_stat_tg1");

    return MTRUE;
}

MBOOL
ISP_MGR_AWB_STAT_CONFIG_T::
apply_TG2()
{
    CAM_LOGD_IF(IsDebugEnabled(),"%s(): m_eSensorDev = %d, m_i4SensorIndex = %d\n", __FUNCTION__, m_eSensorDev, m_i4SensorIndex);
    dumpRegInfo("AWB_STAT_TG2");

    MUINTPTR handle;

    INormalPipe* pPipe = INormalPipe::createInstance(m_i4SensorIndex,"isp_mgr_awb_stat_tg2");//iopipe2.0

    // get module handle
    if (MFALSE == pPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_GET_MODULE_HANDLE,
                           NSImageio::NSIspio::EModule_AWB_D, (MINTPTR)&handle, (MINTPTR)(&("isp_mgr_awb_stat_tg2"))))
    {
        //Error Handling
        CAM_LOGE("EPIPECmd_GET_MODULE_HANDLE fail");
        goto lbExit;
    }

    // set module register
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_WIN_ORG, m_rIspRegInfo[ERegInfo_CAM_AWB_D_WIN_ORG].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_WIN_SIZE, m_rIspRegInfo[ERegInfo_CAM_AWB_D_WIN_SIZE].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_WIN_PIT, m_rIspRegInfo[ERegInfo_CAM_AWB_D_WIN_PIT].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_WIN_NUM, m_rIspRegInfo[ERegInfo_CAM_AWB_D_WIN_NUM].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_GAIN1_0, m_rIspRegInfo[ERegInfo_CAM_AWB_D_GAIN1_0].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_GAIN1_1, m_rIspRegInfo[ERegInfo_CAM_AWB_D_GAIN1_1].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_LMT1_0, m_rIspRegInfo[ERegInfo_CAM_AWB_D_LMT1_0].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_LMT1_1, m_rIspRegInfo[ERegInfo_CAM_AWB_D_LMT1_1].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_LOW_THR, m_rIspRegInfo[ERegInfo_CAM_AWB_D_LOW_THR].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_HI_THR, m_rIspRegInfo[ERegInfo_CAM_AWB_D_HI_THR].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_PIXEL_CNT0, m_rIspRegInfo[ERegInfo_CAM_AWB_D_PIXEL_CNT0].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_PIXEL_CNT1, m_rIspRegInfo[ERegInfo_CAM_AWB_D_PIXEL_CNT1].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_PIXEL_CNT2, m_rIspRegInfo[ERegInfo_CAM_AWB_D_PIXEL_CNT2].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_ERR_THR, m_rIspRegInfo[ERegInfo_CAM_AWB_D_ERR_THR].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_ROT, m_rIspRegInfo[ERegInfo_CAM_AWB_D_ROT].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_L0_X, m_rIspRegInfo[ERegInfo_CAM_AWB_D_L0_X].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_L0_Y, m_rIspRegInfo[ERegInfo_CAM_AWB_D_L0_Y].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_L1_X, m_rIspRegInfo[ERegInfo_CAM_AWB_D_L1_X].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_L1_Y, m_rIspRegInfo[ERegInfo_CAM_AWB_D_L1_Y].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_L2_X, m_rIspRegInfo[ERegInfo_CAM_AWB_D_L2_X].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_L2_Y, m_rIspRegInfo[ERegInfo_CAM_AWB_D_L2_Y].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_L3_X, m_rIspRegInfo[ERegInfo_CAM_AWB_D_L3_X].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_L3_Y, m_rIspRegInfo[ERegInfo_CAM_AWB_D_L3_Y].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_L4_X, m_rIspRegInfo[ERegInfo_CAM_AWB_D_L4_X].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_L4_Y, m_rIspRegInfo[ERegInfo_CAM_AWB_D_L4_Y].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_L5_X, m_rIspRegInfo[ERegInfo_CAM_AWB_D_L5_X].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_L5_Y, m_rIspRegInfo[ERegInfo_CAM_AWB_D_L5_Y].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_L6_X, m_rIspRegInfo[ERegInfo_CAM_AWB_D_L6_X].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_L6_Y, m_rIspRegInfo[ERegInfo_CAM_AWB_D_L6_Y].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_L7_X, m_rIspRegInfo[ERegInfo_CAM_AWB_D_L7_X].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_L7_Y, m_rIspRegInfo[ERegInfo_CAM_AWB_D_L7_Y].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_L8_X, m_rIspRegInfo[ERegInfo_CAM_AWB_D_L8_X].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_L8_Y, m_rIspRegInfo[ERegInfo_CAM_AWB_D_L8_Y].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_L9_X, m_rIspRegInfo[ERegInfo_CAM_AWB_D_L9_X].val);
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_D_L9_Y, m_rIspRegInfo[ERegInfo_CAM_AWB_D_L9_Y].val);

    // set module config done
    if (MFALSE==pPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_SET_MODULE_CFG_DONE, handle, MNULL, MNULL))
    {
        //Error Handling
        CAM_LOGE("EPIPECmd_SET_MODULE_CFG_DONE fail");
        goto lbExit;
    }

lbExit:

    // release handle
    if (MFALSE==pPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_RELEASE_MODULE_HANDLE, handle, (MINTPTR)(&("isp_mgr_awb_stat_tg2")), MNULL))
    {
        // Error Handling
        CAM_LOGE("EPIPECmd_SET_MODULE_CFG_DONE fail");
    }

    pPipe->destroyInstance("isp_mgr_awb_stat_tg2");

    return MTRUE;
}


MBOOL
ISP_MGR_AWB_STAT_CONFIG_T::
configAAO_AWB(MINT32 i4SensorIndex, AWB_STAT_CONFIG_T& rAWBStatConfig, MINT32 tgW, MINT32 tgH)
{

    this->tuningMgr->uninit("AAO_AWB_CFG");
    return MTRUE;
}

#endif


}
