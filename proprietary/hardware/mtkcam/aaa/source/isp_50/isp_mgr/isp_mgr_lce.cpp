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
#define LOG_TAG "isp_mgr_lce"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <math.h>
#include <mtkcam/utils/std/Log.h>
#include "isp_mgr.h"
#include <private/aaa_utils.h>
#include <array>

using namespace NSIspTuningv3;

#define MY_INST_LCS NS3Av3::INST_T<ISP_MGR_LCS_T>
static std::array<MY_INST_LCS, SENSOR_IDX_MAX> gMultitonLCS;

#define MY_INST_LCE NS3Av3::INST_T<ISP_MGR_LCE_T>
static std::array<MY_INST_LCE, SENSOR_IDX_MAX> gMultitonLCE;

#if defined(HAVE_AEE_FEATURE)
#include <aee.h>
#define AEE_ASSERT_LCE(String) \
          do { \
              aee_system_exception( \
                  LOG_TAG, \
                  NULL, \
                  DB_OPT_DEFAULT, \
                  String); \
          } while(0)
#else
#define AEE_ASSERT_LCE(String)
#endif

namespace NSIspTuningv3
{
#define CLAMP(x,min,max)       (((x) > (max)) ? (max) : (((x) < (min)) ? (min) : (x)))

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  LCS
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_LCS_T&
ISP_MGR_LCS_T::
getInstance(ESensorDev_T const eSensorDev)
{
    auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(eSensorDev);

    MY_ASSERT(i4SensorIdx < SENSOR_IDX_MAX);
    MY_ASSERT(i4SensorIdx >= 0);


    MY_INST_LCS& rSingleton = gMultitonLCS[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<ISP_MGR_LCS_T>(eSensorDev);
    } );

    return *(rSingleton.instance);
}

template <>
ISP_MGR_LCS_T&
ISP_MGR_LCS_T::
put(ISP_NVRAM_LCS_T const& rParam)
{
    PUT_REG_INFO(CAM_LCS25_FLR,       flr);
    PUT_REG_INFO(CAM_LCS25_SATU_1,    satu_1);
    PUT_REG_INFO(CAM_LCS25_SATU_2,    satu_2);
    PUT_REG_INFO(CAM_LCS25_GAIN_1,    gain_1);
    PUT_REG_INFO(CAM_LCS25_GAIN_2,    gain_2);
    PUT_REG_INFO(CAM_LCS25_OFST_1,    ofst_1);
    PUT_REG_INFO(CAM_LCS25_OFST_2,    ofst_2);
    PUT_REG_INFO(CAM_LCS25_G2G_CNV_1, g2g_cnv_1);
    PUT_REG_INFO(CAM_LCS25_G2G_CNV_2, g2g_cnv_2);
    PUT_REG_INFO(CAM_LCS25_G2G_CNV_3, g2g_cnv_3);
    PUT_REG_INFO(CAM_LCS25_G2G_CNV_4, g2g_cnv_4);
    PUT_REG_INFO(CAM_LCS25_G2G_CNV_5, g2g_cnv_5);

    return  (*this);
}


template <>
ISP_MGR_LCS_T&
ISP_MGR_LCS_T::
get(ISP_NVRAM_LCS_T& rParam)
{
    GET_REG_INFO(CAM_LCS25_FLR,       flr);
    GET_REG_INFO(CAM_LCS25_SATU_1,    satu_1);
    GET_REG_INFO(CAM_LCS25_SATU_2,    satu_2);
    GET_REG_INFO(CAM_LCS25_GAIN_1,    gain_1);
    GET_REG_INFO(CAM_LCS25_GAIN_2,    gain_2);
    GET_REG_INFO(CAM_LCS25_OFST_1,    ofst_1);
    GET_REG_INFO(CAM_LCS25_OFST_2,    ofst_2);
    GET_REG_INFO(CAM_LCS25_G2G_CNV_1, g2g_cnv_1);
    GET_REG_INFO(CAM_LCS25_G2G_CNV_2, g2g_cnv_2);
    GET_REG_INFO(CAM_LCS25_G2G_CNV_3, g2g_cnv_3);
    GET_REG_INFO(CAM_LCS25_G2G_CNV_4, g2g_cnv_4);
    GET_REG_INFO(CAM_LCS25_G2G_CNV_5, g2g_cnv_5);

    return  (*this);
}

MBOOL
ISP_MGR_LCS_T::
apply(RAWIspCamInfo& rRawIspCamInfo, TuningMgr& /*rTuning*/, MINT32 /*i4SubsampleIdex*/)
{
    MBOOL bLCS_EN = isEnable();

    if(bLCS_EN){

        ISP_NVRAM_LCS_T rParam;
        ISP_NVRAM_RPG_T rRPG = rRawIspCamInfo.rRPG;
        ISP_NVRAM_CCM_T rCCM = rRawIspCamInfo.rMtkCCM;

        rParam.flr.bits.LCS25_FLR_OFST = 0;
        rParam.flr.bits.LCS25_FLR_GAIN = 256;

        rParam.satu_1.val = rRPG.satu_1.val;
        rParam.satu_2.val = rRPG.satu_2.val;
        rParam.gain_1.val = rRPG.gain_1.val;
        rParam.gain_2.val = rRPG.gain_2.val;

        //align the diff in bit number (LCS and RPG)
    if( (rRPG.ofst_1.bits.RPG_OFST_B >> 11) & 1 ){
        rParam.ofst_1.bits.LCS25_OFST_B = (rRPG.ofst_1.bits.RPG_OFST_B | 0xF000);
        }
        else{
            rParam.ofst_1.bits.LCS25_OFST_B = rRPG.ofst_1.bits.RPG_OFST_B;
        }

    if( (rRPG.ofst_1.bits.RPG_OFST_GB >> 11) & 1 ){
        rParam.ofst_1.bits.LCS25_OFST_GB = (rRPG.ofst_1.bits.RPG_OFST_GB | 0xF000);
        }
        else{
            rParam.ofst_1.bits.LCS25_OFST_GB = rRPG.ofst_1.bits.RPG_OFST_GB;
        }

    if( (rRPG.ofst_2.bits.RPG_OFST_GR >> 11) & 1 ){
        rParam.ofst_2.bits.LCS25_OFST_GR = (rRPG.ofst_2.bits.RPG_OFST_GR | 0xF000);
        }
        else{
            rParam.ofst_2.bits.LCS25_OFST_GR = rRPG.ofst_2.bits.RPG_OFST_GR;
        }

    if( (rRPG.ofst_2.bits.RPG_OFST_R >> 11) & 1 ){
        rParam.ofst_2.bits.LCS25_OFST_R = (rRPG.ofst_2.bits.RPG_OFST_R | 0xF000);
        }
        else{
            rParam.ofst_2.bits.LCS25_OFST_R = rRPG.ofst_2.bits.RPG_OFST_R;
        }

        rParam.g2g_cnv_1.bits.LCS25_G2G_CNV_00 = rCCM.cnv_1.bits.G2G_CNV_00;
        rParam.g2g_cnv_1.bits.LCS25_G2G_CNV_01 = rCCM.cnv_1.bits.G2G_CNV_01;
        rParam.g2g_cnv_2.bits.LCS25_G2G_CNV_02 = rCCM.cnv_2.bits.G2G_CNV_02;
        rParam.g2g_cnv_2.bits.LCS25_G2G_CNV_10 = rCCM.cnv_3.bits.G2G_CNV_10;
        rParam.g2g_cnv_3.bits.LCS25_G2G_CNV_11 = rCCM.cnv_3.bits.G2G_CNV_11;
        rParam.g2g_cnv_3.bits.LCS25_G2G_CNV_12 = rCCM.cnv_4.bits.G2G_CNV_12;
        rParam.g2g_cnv_4.bits.LCS25_G2G_CNV_20 = rCCM.cnv_5.bits.G2G_CNV_20;
        rParam.g2g_cnv_4.bits.LCS25_G2G_CNV_21 = rCCM.cnv_5.bits.G2G_CNV_21;
        rParam.g2g_cnv_5.bits.LCS25_G2G_CNV_22 = rCCM.cnv_6.bits.G2G_CNV_22;
    rParam.g2g_cnv_5.bits.LCS25_G2G_ACC = 9;  //fixme_Choo

        put(rParam);

        rRawIspCamInfo.rLCS_Info.rInSetting.lcs = rParam;
    }

    rRawIspCamInfo.rLCS_Info.rInSetting.fgOnOff = isEnable();

    // TOP
    //TUNING_MGR_WRITE_BITS_CAM((&rTuning), CAM_CTL_EN, LCS_EN, bLCS_EN, i4SubsampleIdex);
    ISP_MGR_CTL_EN_P1_T::getInstance(m_eSensorDev).setEnable_LCS(bLCS_EN);

    dumpRegInfoP1("LCS");

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  LCE
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_LCE_T&
ISP_MGR_LCE_T::
getInstance(ESensorDev_T const eSensorDev)
{
    auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(eSensorDev);

    MY_ASSERT(i4SensorIdx < SENSOR_IDX_MAX);
    MY_ASSERT(i4SensorIdx >= 0);

    MY_INST_LCE& rSingleton = gMultitonLCE[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<ISP_MGR_LCE_T>(eSensorDev);
    } );

    return *(rSingleton.instance);
}

template <>
ISP_MGR_LCE_T&
ISP_MGR_LCE_T::
put(ISP_NVRAM_LCE_T const& rParam)
{
    m_rRegInput[ERegInfo_DIP_X_LCE25_TM_PARA1]   = rParam.tm_para1.val;
    m_rRegInput[ERegInfo_DIP_X_LCE25_TM_PARA2]   = rParam.tm_para2.val;
    m_rRegInput[ERegInfo_DIP_X_LCE25_TM_PARA3]   = rParam.tm_para3.val;
    m_rRegInput[ERegInfo_DIP_X_LCE25_TM_PARA4]   = rParam.tm_para4.val;
    m_rRegInput[ERegInfo_DIP_X_LCE25_TM_PARA5]   = rParam.tm_para5.val;
    m_rRegInput[ERegInfo_DIP_X_LCE25_TM_PARA6]   = rParam.tm_para6.val;
    m_rRegInput[ERegInfo_DIP_X_LCE25_TM_PARA7]   = rParam.tm_para7.val;
    m_rRegInput[ERegInfo_DIP_X_LCE25_TM_PARA8]   = rParam.tm_para8.val;
    m_rRegInput[ERegInfo_DIP_X_LCE25_TM_PARA9]   = rParam.tm_para9.val;

    m_rRegInput[ERegInfo_DIP_X_LCE25_TM_PARA0]   = rParam.tm_para0.val;

    m_rRegInput[ERegInfo_DIP_X_LCE25_HLR_PARA0]   = rParam.hlr_para0.val;
    m_rRegInput[ERegInfo_DIP_X_LCE25_HLR_PARA1]   = rParam.hlr_para1.val;
    m_rRegInput[ERegInfo_DIP_X_LCE25_HLR_PARA2]   = rParam.hlr_para2.val;
    m_rRegInput[ERegInfo_DIP_X_LCE25_HLR_PARA3]   = rParam.hlr_para3.val;
    m_rRegInput[ERegInfo_DIP_X_LCE25_HLR_PARA4]   = rParam.hlr_para4.val;
    m_rRegInput[ERegInfo_DIP_X_LCE25_HLR_PARA5]   = rParam.hlr_para5.val;
    m_rRegInput[ERegInfo_DIP_X_LCE25_HLR_PARA6]   = rParam.hlr_para6.val;
    m_rRegInput[ERegInfo_DIP_X_LCE25_HLR_PARA7]   = rParam.hlr_para7.val;

    m_rRegInput[ERegInfo_DIP_X_LCE25_TCHL_PARA0]   = rParam.tchl_para0.val;
    m_rRegInput[ERegInfo_DIP_X_LCE25_TCHL_PARA1]   = rParam.tchl_para1.val;
    m_rRegInput[ERegInfo_DIP_X_LCE25_TCHL_PARA2]   = rParam.tchl_para2.val;

    MINT32 difference[8];

    difference[0] = rParam.tm_para8.bits.LCE_TC_O0 - rParam.tm_para7.bits.LCE_TC_P0;
    difference[1] = rParam.tm_para4.bits.LCE_TC_O1 - rParam.tm_para1.bits.LCE_TC_P1;
    difference[2] = rParam.tm_para4.bits.LCE_TC_O50 - rParam.tm_para2.bits.LCE_TC_P50;
    difference[3] = rParam.tm_para8.bits.LCE_TC_O250 - rParam.tm_para7.bits.LCE_TC_P250;
    difference[4] = rParam.tm_para5.bits.LCE_TC_O500 - rParam.tm_para2.bits.LCE_TC_P500;
    difference[5] = rParam.tm_para9.bits.LCE_TC_O750 - rParam.tm_para9.bits.LCE_TC_P750;
    difference[6] = rParam.tm_para5.bits.LCE_TC_O950 - rParam.tm_para3.bits.LCE_TC_P950;
    difference[7] = rParam.tm_para6.bits.LCE_TC_O999 - rParam.tm_para3.bits.LCE_TC_P999;

    float Max = 0;
    for(int i=0; i<8; i++){
        if(difference[i] > Max){
            Max = difference[i];
        }
    }

    m_LCE_Gain = (m_LCE_Gain & 0xFFFF0000);

    m_LCE_Gain = (m_LCE_Gain | ((MUINT32)(pow(10,(Max/1024.0f)) * 1000.0f)));

    return  (*this);
}

template <>
MBOOL
ISP_MGR_LCE_T::
get(ISP_NVRAM_LCE_T& rParam, const dip_x_reg_t* pReg) const
{
    if (pReg)
    {
        GET_REG_INFO_BUF(DIP_X_LCE25_CON,          con);
        GET_REG_INFO_BUF(DIP_X_LCE25_ZR,           zr);
        GET_REG_INFO_BUF(DIP_X_LCE25_SLM_SIZE,     slm_size);
        GET_REG_INFO_BUF(DIP_X_LCE25_BIL_TH0,      bil_th0);
        GET_REG_INFO_BUF(DIP_X_LCE25_BIL_TH1,      bil_th1);
        GET_REG_INFO_BUF(DIP_X_LCE25_TM_PARA0,     tm_para0);
        GET_REG_INFO_BUF(DIP_X_LCE25_TM_PARA1,     tm_para1);
        GET_REG_INFO_BUF(DIP_X_LCE25_TM_PARA2,     tm_para2);
        GET_REG_INFO_BUF(DIP_X_LCE25_TM_PARA3,     tm_para3);
        GET_REG_INFO_BUF(DIP_X_LCE25_TM_PARA4,     tm_para4);
        GET_REG_INFO_BUF(DIP_X_LCE25_TM_PARA5,     tm_para5);
        GET_REG_INFO_BUF(DIP_X_LCE25_TM_PARA6,     tm_para6);
        GET_REG_INFO_BUF(DIP_X_LCE25_TM_PARA7,     tm_para7);
        GET_REG_INFO_BUF(DIP_X_LCE25_TM_PARA8,     tm_para8);
        GET_REG_INFO_BUF(DIP_X_LCE25_TM_PARA9,     tm_para9);

        GET_REG_INFO_BUF(DIP_X_LCE25_TCHL_PARA0,     tchl_para0);
        GET_REG_INFO_BUF(DIP_X_LCE25_TCHL_PARA1,     tchl_para1);
        GET_REG_INFO_BUF(DIP_X_LCE25_TCHL_PARA2,     tchl_para2);

        GET_REG_INFO_BUF(DIP_X_LCE25_HLR_PARA0,     hlr_para0);
        GET_REG_INFO_BUF(DIP_X_LCE25_HLR_PARA1,     hlr_para1);
        GET_REG_INFO_BUF(DIP_X_LCE25_HLR_PARA2,     hlr_para2);
        GET_REG_INFO_BUF(DIP_X_LCE25_HLR_PARA3,     hlr_para3);
        GET_REG_INFO_BUF(DIP_X_LCE25_HLR_PARA4,     hlr_para4);
        GET_REG_INFO_BUF(DIP_X_LCE25_HLR_PARA5,     hlr_para5);
        GET_REG_INFO_BUF(DIP_X_LCE25_HLR_PARA6,     hlr_para6);
        GET_REG_INFO_BUF(DIP_X_LCE25_HLR_PARA7,     hlr_para7);



    }
    return MTRUE;
}

MVOID
ISP_MGR_LCE_T::
transform_LCE(const CROP_RZ_INFO_T& rRzCrop)
{
    REG_INFO_VALUE(DIP_X_LCE25_TM_PARA1)   = m_rRegInput[ERegInfo_DIP_X_LCE25_TM_PARA1];
    REG_INFO_VALUE(DIP_X_LCE25_TM_PARA2)   = m_rRegInput[ERegInfo_DIP_X_LCE25_TM_PARA2];
    REG_INFO_VALUE(DIP_X_LCE25_TM_PARA3)   = m_rRegInput[ERegInfo_DIP_X_LCE25_TM_PARA3];
    REG_INFO_VALUE(DIP_X_LCE25_TM_PARA4)   = m_rRegInput[ERegInfo_DIP_X_LCE25_TM_PARA4];
    REG_INFO_VALUE(DIP_X_LCE25_TM_PARA5)   = m_rRegInput[ERegInfo_DIP_X_LCE25_TM_PARA5];
    REG_INFO_VALUE(DIP_X_LCE25_TM_PARA6)   = m_rRegInput[ERegInfo_DIP_X_LCE25_TM_PARA6];
    REG_INFO_VALUE(DIP_X_LCE25_TM_PARA7)   = m_rRegInput[ERegInfo_DIP_X_LCE25_TM_PARA7];
    REG_INFO_VALUE(DIP_X_LCE25_TM_PARA8)   = m_rRegInput[ERegInfo_DIP_X_LCE25_TM_PARA8];
    REG_INFO_VALUE(DIP_X_LCE25_TM_PARA9)   = m_rRegInput[ERegInfo_DIP_X_LCE25_TM_PARA9];

    REG_INFO_VALUE(DIP_X_LCE25_TM_PARA0)   = m_rRegInput[ERegInfo_DIP_X_LCE25_TM_PARA0];

    REG_INFO_VALUE(DIP_X_LCE25_TCHL_PARA0)   = m_rRegInput[ERegInfo_DIP_X_LCE25_TCHL_PARA0];
    REG_INFO_VALUE(DIP_X_LCE25_TCHL_PARA1)   = m_rRegInput[ERegInfo_DIP_X_LCE25_TCHL_PARA1];
    REG_INFO_VALUE(DIP_X_LCE25_TCHL_PARA2)   = m_rRegInput[ERegInfo_DIP_X_LCE25_TCHL_PARA2];

    REG_INFO_VALUE(DIP_X_LCE25_HLR_PARA0)   = m_rRegInput[ERegInfo_DIP_X_LCE25_HLR_PARA0];
    REG_INFO_VALUE(DIP_X_LCE25_HLR_PARA1)   = m_rRegInput[ERegInfo_DIP_X_LCE25_HLR_PARA1];
    REG_INFO_VALUE(DIP_X_LCE25_HLR_PARA2)   = m_rRegInput[ERegInfo_DIP_X_LCE25_HLR_PARA2];
    REG_INFO_VALUE(DIP_X_LCE25_HLR_PARA3)   = m_rRegInput[ERegInfo_DIP_X_LCE25_HLR_PARA3];
    REG_INFO_VALUE(DIP_X_LCE25_HLR_PARA4)   = m_rRegInput[ERegInfo_DIP_X_LCE25_HLR_PARA4];
    REG_INFO_VALUE(DIP_X_LCE25_HLR_PARA5)   = m_rRegInput[ERegInfo_DIP_X_LCE25_HLR_PARA5];
    REG_INFO_VALUE(DIP_X_LCE25_HLR_PARA6)   = m_rRegInput[ERegInfo_DIP_X_LCE25_HLR_PARA6];
    REG_INFO_VALUE(DIP_X_LCE25_HLR_PARA7)   = m_rRegInput[ERegInfo_DIP_X_LCE25_HLR_PARA7];

    MINT32 i4RrzInX     = rRzCrop.i4RRZinW;
    MINT32 i4RrzInY     = rRzCrop.i4RRZinH;
    MINT32 i4RrzXoff    = rRzCrop.i4RRZofstX;
    MINT32 i4RrzYoff    = rRzCrop.i4RRZofstY;
    MINT32 i4RrzCropInX = rRzCrop.i4RRZcropW;
    MINT32 i4RrzCropInY = rRzCrop.i4RRZcropH;
    MINT32 i4RrzOutX    = rRzCrop.i4RRZoutW;
    MINT32 i4RrzOutY    = rRzCrop.i4RRZoutH;

    DIP_X_REG_LCE25_SLM_SIZE slmSize;
    slmSize.Bits.LCE_SLM_WD = m_LCEIwidth;
    slmSize.Bits.LCE_SLM_HT = m_LCEIheight;
    REG_INFO_VALUE(DIP_X_LCE25_SLM_SIZE) = slmSize.Raw;

    if (i4RrzCropInX == 0 || i4RrzCropInY == 0)
    {
        CAM_LOGE("Unexpected crop info width(%d), height(%d)", i4RrzCropInX, i4RrzCropInY);
        AEE_ASSERT_LCE("Unexpected crop info for LCE");
    }
    else{
        MINT32 lce_full_slm_wd  = slmSize.Bits.LCE_SLM_WD;
        MINT32 lce_full_slm_ht  = slmSize.Bits.LCE_SLM_HT;
        MINT32 lce_full_xoff    = i4RrzXoff * i4RrzOutX / i4RrzCropInX;
        MINT32 lce_full_yoff    = i4RrzYoff * i4RrzOutY / i4RrzCropInY;
        MINT32 lce_full_out_ht  = i4RrzInY * i4RrzOutY / i4RrzCropInY;
        MINT32 lce_bcmk_x       = 32768*(lce_full_slm_wd-1)/(i4RrzOutX*i4RrzInX/i4RrzCropInX-1);
        MINT32 lce_bcmk_y       = 32768*(lce_full_slm_ht-1)/(i4RrzOutY*i4RrzInY/i4RrzCropInY-1);

        //CAM_LOGD("[LCE] (%dx%d, %d,%d,%d,%d, %dx%d) slm(%dx%d) off(%d,%d) ht(%d), bcmk(%d,%d)",
        //    i4RrzInX, i4RrzInY, i4RrzXoff, i4RrzYoff, i4RrzCropInX, i4RrzCropInY, i4RrzOutX, i4RrzOutY,
        //    lce_full_slm_wd, lce_full_slm_ht, lce_full_xoff, lce_full_yoff, lce_full_out_ht, lce_bcmk_x, lce_bcmk_y);


        DIP_X_REG_LCE25_ZR            zr = {.Raw = 0};

        zr.Bits.LCE_BCMK_X              = lce_bcmk_x;
        zr.Bits.LCE_BCMK_Y              = lce_bcmk_y;

        REG_INFO_VALUE(DIP_X_LCE25_ZR)    = zr.Raw;

        //dummy
        DIP_X_REG_LCE25_TPIPE_SLM     slm = {.Raw = 0};
        DIP_X_REG_LCE25_TPIPE_OFFSET  offset = {.Raw = 0};
        DIP_X_REG_LCE25_TPIPE_OUT     out = {.Raw = 0};

        slm.Bits.LCE_TPIPE_SLM_WD       = lce_full_slm_wd;
        slm.Bits.LCE_TPIPE_SLM_HT       = lce_full_slm_ht;
        offset.Bits.LCE_TPIPE_OFFSET_X  = lce_full_xoff;
        offset.Bits.LCE_TPIPE_OFFSET_Y  = lce_full_yoff;
        out.Bits.LCE_TPIPE_OUT_HT       = lce_full_out_ht;

        REG_INFO_VALUE(DIP_X_LCE25_SLM)   = slm.Raw;
        REG_INFO_VALUE(DIP_X_LCE25_OFFSET)= offset.Raw;
        REG_INFO_VALUE(DIP_X_LCE25_OUT)   = out.Raw;
    }
}


MBOOL
ISP_MGR_LCE_T::
apply(const CROP_RZ_INFO_T& rRzCrop, EIspProfile_T /*eIspProfile*/, dip_x_reg_t* pReg)
{
    MBOOL bLCE_EN = isEnable();

    if(bLCE_EN){
        transform_LCE(rRzCrop);
    }

    // TOP
    ISP_WRITE_ENABLE_BITS(pReg, DIP_X_CTL_RGB_EN, LCE_EN, bLCE_EN);
    //ISP_MGR_CTL_EN_P2_T::getInstance(m_eSensorDev).setEnable_LCE(bLCE_EN);

    // Register setting
    writeRegs(static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum, pReg);

    dumpRegInfo("LCE");
    return  MTRUE;
}

}

