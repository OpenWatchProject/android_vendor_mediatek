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
#define LOG_TAG "isp_mgr_cpn"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include "isp_mgr.h"
#include <private/aaa_utils.h>
#include <array>

using namespace NSIspTuningv3;

#define MY_INST_CPN_P1 NS3Av3::INST_T<ISP_MGR_CPN_T>
static std::array<MY_INST_CPN_P1, SENSOR_IDX_MAX> gMultitonP1;

#define MY_INST_CPN_P2 NS3Av3::INST_T<ISP_MGR_CPN2_T>
static std::array<MY_INST_CPN_P2, SENSOR_IDX_MAX> gMultitonP2;

namespace NSIspTuningv3
{
#define CLAMP(x,min,max)       (((x) > (max)) ? (max) : (((x) < (min)) ? (min) : (x)))

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CPN
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_CPN_T&
ISP_MGR_CPN_T::
getInstance(ESensorDev_T const eSensorDev)
{
    auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(eSensorDev);

    MY_ASSERT(i4SensorIdx < SENSOR_IDX_MAX);
    MY_ASSERT(i4SensorIdx >= 0);

    MY_INST_CPN_P1& rSingleton = gMultitonP1[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<ISP_MGR_CPN_T>(eSensorDev);
    } );

    return *(rSingleton.instance);
}

template <>
ISP_MGR_CPN_T&
ISP_MGR_CPN_T::
put(ISP_NVRAM_CPN_T const& rParam)
{
    PUT_REG_INFO(CAM_CPN_HDR_CTL_EN, hdr_ctl_en);
    //PUT_REG_INFO(CAM_CPN_IN_IMG_SIZE, in_img_size);
    PUT_REG_INFO(CAM_CPN_ALGO_PARAM1, algo_param1);
    PUT_REG_INFO(CAM_CPN_ALGO_PARAM2, algo_param2);
    PUT_REG_INFO(CAM_CPN_GTM_X0X1, x0x1);
    PUT_REG_INFO(CAM_CPN_GTM_X2X3, x2x3);
    PUT_REG_INFO(CAM_CPN_GTM_X4X5, x4x5);
    PUT_REG_INFO(CAM_CPN_GTM_X6, x6);
    PUT_REG_INFO(CAM_CPN_GTM_Y0Y1, y0y1);
    PUT_REG_INFO(CAM_CPN_GTM_Y2Y3, y2y3);
    PUT_REG_INFO(CAM_CPN_GTM_Y4Y5, y4y5);
    PUT_REG_INFO(CAM_CPN_GTM_Y6, y6);
    PUT_REG_INFO(CAM_CPN_GTM_S0S1, s0s1);
    PUT_REG_INFO(CAM_CPN_GTM_S2S3, s2s3);
    PUT_REG_INFO(CAM_CPN_GTM_S4S5, s4s5);
    PUT_REG_INFO(CAM_CPN_GTM_S6S7, s6s7);
    return  (*this);
}


template <>
ISP_MGR_CPN_T&
ISP_MGR_CPN_T::
get(ISP_NVRAM_CPN_T& rParam)
{
    GET_REG_INFO(CAM_CPN_HDR_CTL_EN, hdr_ctl_en);
    //GET_REG_INFO(CAM_CPN_IN_IMG_SIZE, in_img_size);
    GET_REG_INFO(CAM_CPN_ALGO_PARAM1, algo_param1);
    GET_REG_INFO(CAM_CPN_ALGO_PARAM2, algo_param2);
    GET_REG_INFO(CAM_CPN_GTM_X0X1, x0x1);
    GET_REG_INFO(CAM_CPN_GTM_X2X3, x2x3);
    GET_REG_INFO(CAM_CPN_GTM_X4X5, x4x5);
    GET_REG_INFO(CAM_CPN_GTM_X6, x6);
    GET_REG_INFO(CAM_CPN_GTM_Y0Y1, y0y1);
    GET_REG_INFO(CAM_CPN_GTM_Y2Y3, y2y3);
    GET_REG_INFO(CAM_CPN_GTM_Y4Y5, y4y5);
    GET_REG_INFO(CAM_CPN_GTM_Y6, y6);
    GET_REG_INFO(CAM_CPN_GTM_S0S1, s0s1);
    GET_REG_INFO(CAM_CPN_GTM_S2S3, s2s3);
    GET_REG_INFO(CAM_CPN_GTM_S4S5, s4s5);
    GET_REG_INFO(CAM_CPN_GTM_S6S7, s6s7);
    return  (*this);
}



MBOOL
ISP_MGR_CPN_T::
apply(RAWIspCamInfo& rRawIspCamInfo, TuningMgr& rTuning, MINT32 i4SubsampleIdex)
{

    MBOOL bCPN_EN = isEnable() && ISP_MGR_LSC_T::getInstance(m_eSensorDev).isEnable(); //HW constraint

    addressErrorCheck("Before ISP_MGR_CPN_T::apply()");

    //> prepar register value
    //CAM_LOGD_IF(debugDump, "bCPN_EN(%d) m_bZHdrEnable(%d)", bCPN_EN, m_bZHdrEnable);
    if(bCPN_EN)
    {
        MINT32 hdrEnable = m_bZHdrEnable;
        // TO SET:
        // Temporary we set AA & PS output control signal to 0
        MINT32 aalLE = 0; //hdrEnable ? 1 : 0;
        MINT32 psSE = 0; //hdrEnable ? 1 : 0;

        MUINT32 orig = 0;
        orig = REG_INFO_VALUE(CAM_CPN_HDR_CTL_EN);
        REG_INFO_VALUE(CAM_CPN_HDR_CTL_EN) = (orig & 0x000000F0) //((edgeInfo & 0xF) << 4)
                                           | ((hdrEnable & 0x1))
                                           | ((aalLE & 0x1) << 1)
                                           | ((psSE & 0x1)  << 2)
                                           | 0;

        if(hdrEnable)
        {
            CAM_LOGD_IF(debugDump, "CPN in: hdrEnable(%d), HDR_RATIO(%d)", hdrEnable, rRawIspCamInfo.rAEInfo.i4LESE_Ratio);

            MINT32 aeLeSeRatio_x100 = CLAMP(rRawIspCamInfo.rAEInfo.i4LESE_Ratio, 100, 1600);
            MINT32 HDR_GAIN = (51200 / aeLeSeRatio_x100 + 1) >> 1; // do rounding
            MINT32 HDR_RATIO = (16 * aeLeSeRatio_x100 + 50) / 100;

            MINT32 len_x = sizeof(rRawIspCamInfo.rAEInfo.rHdrToneInfo.rHdrCPNProp.GTM_X)/sizeof(rRawIspCamInfo.rAEInfo.rHdrToneInfo.rHdrCPNProp.GTM_X[0]);
            MINT32 len_y = sizeof(rRawIspCamInfo.rAEInfo.rHdrToneInfo.rHdrCPNProp.GTM_Y)/sizeof(rRawIspCamInfo.rAEInfo.rHdrToneInfo.rHdrCPNProp.GTM_Y[0]);
            MINT32 len_s = sizeof(rRawIspCamInfo.rAEInfo.rHdrToneInfo.rHdrCPNProp.GTM_S)/sizeof(rRawIspCamInfo.rAEInfo.rHdrToneInfo.rHdrCPNProp.GTM_S[0]);

            std::vector<MINT32> gtm_x, gtm_y, gtm_s;
            if( len_x != 7 || len_y != 7 || len_s != 8)
            {
                CAM_LOGE("CPN : GTM_X(%d), GTM_Y(%d) and GTM_S(%d) total element number not correct.", len_x, len_y, len_s);
                return false;
            }
            for(int i = 0 ; i < len_x ; i++)
            {
                gtm_x.push_back(CLAMP(rRawIspCamInfo.rAEInfo.rHdrToneInfo.rHdrCPNProp.GTM_X[i], 0, 4095));
                gtm_y.push_back(CLAMP(rRawIspCamInfo.rAEInfo.rHdrToneInfo.rHdrCPNProp.GTM_Y[i], 0, 4095));
            }
            for(int i = 0 ; i < len_s ; i++)
            {
                gtm_s.push_back(CLAMP(rRawIspCamInfo.rAEInfo.rHdrToneInfo.rHdrCPNProp.GTM_S[i], 0, 4095));
            }

            REG_INFO_VALUE(CAM_CPN_ALGO_PARAM1) = (HDR_RATIO & 0x01FF)
                                                  | ((HDR_GAIN  & 0x01FF) << 16)
                                                  | 0;
            //for(const auto &it : gtm_x)
            REG_INFO_VALUE(CAM_CPN_GTM_X0X1) = (gtm_x.at(0) & 0xFFF)
                                                |((gtm_x.at(1) & 0xFFF) << 16)
                                                | 0;
            REG_INFO_VALUE(CAM_CPN_GTM_X2X3) = (gtm_x.at(2) & 0xFFF)
                                                |((gtm_x.at(3) & 0xFFF) << 16)
                                                | 0;
            REG_INFO_VALUE(CAM_CPN_GTM_X4X5) = (gtm_x.at(4) & 0xFFF)
                                                |((gtm_x.at(5) & 0xFFF) << 16)
                                                | 0;
            REG_INFO_VALUE(CAM_CPN_GTM_X6)   = (gtm_x.at(6) & 0xFFF)
                                                | 0;
            REG_INFO_VALUE(CAM_CPN_GTM_Y0Y1) = (gtm_y.at(0) & 0xFFF)
                                                |((gtm_y.at(1) & 0xFFF) << 16)
                                                | 0;
            REG_INFO_VALUE(CAM_CPN_GTM_Y2Y3) = (gtm_y.at(2) & 0xFFF)
                                                |((gtm_y.at(3) & 0xFFF) << 16)
                                                | 0;
            REG_INFO_VALUE(CAM_CPN_GTM_Y4Y5) = (gtm_y.at(4) & 0xFFF)
                                                |((gtm_y.at(5) & 0xFFF) << 16)
                                                | 0;
            REG_INFO_VALUE(CAM_CPN_GTM_Y6)   = (gtm_y.at(6) & 0xFFF)
                                                | 0;
            REG_INFO_VALUE(CAM_CPN_GTM_S0S1) = (gtm_s.at(0) & 0xFFF)
                                                |((gtm_s.at(1) & 0xFFF) << 16)
                                                | 0;
            REG_INFO_VALUE(CAM_CPN_GTM_S2S3) = (gtm_s.at(2) & 0xFFF)
                                                |((gtm_s.at(3) & 0xFFF) << 16)
                                                | 0;
            REG_INFO_VALUE(CAM_CPN_GTM_S4S5) = (gtm_s.at(4) & 0xFFF)
                                                |((gtm_s.at(5) & 0xFFF) << 16)
                                                | 0;
            REG_INFO_VALUE(CAM_CPN_GTM_S6S7) = (gtm_s.at(6) & 0xFFF)
                                                |((gtm_s.at(7) & 0xFFF) << 16)
                                                | 0;
        }   // if hdrEnable end

    }   // if bCPN_EN end

    rTuning.updateEngine(eTuningMgrFunc_CPN, bCPN_EN, 0);

    // TOP
    TUNING_MGR_WRITE_BITS_CAM((&rTuning), CAM_CTL_EN2, CPN_EN, bCPN_EN, 0);

    ISP_MGR_CTL_EN_P1_T::getInstance(m_eSensorDev).setEnable_CPN(bCPN_EN);

    // Register setting
    rTuning.tuningMgrWriteRegs(
            static_cast<TUNING_MGR_REG_IO_STRUCT*>(m_pRegInfo),
            m_u4RegInfoNum, i4SubsampleIdex);


    dumpRegInfoP1("CPN");
    return  MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CPN2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_CPN2_T&
ISP_MGR_CPN2_T::
getInstance(ESensorDev_T const eSensorDev)
{
    auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(eSensorDev);

    MY_ASSERT(i4SensorIdx < SENSOR_IDX_MAX);
    MY_ASSERT(i4SensorIdx >= 0);


    MY_INST_CPN_P2& rSingleton = gMultitonP2[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<ISP_MGR_CPN2_T>(eSensorDev);
    } );

    return *(rSingleton.instance);
}

template <>
ISP_MGR_CPN2_T&
ISP_MGR_CPN2_T::
put(ISP_NVRAM_CPN_T const& rParam)
{
    PUT_REG_INFO(DIP_X_CPN_HDR_CTL_EN,   hdr_ctl_en);
    //PUT_REG_INFO(DIP_X_CPN_IN_IMG_SIZE,  in_img_size);
    PUT_REG_INFO(DIP_X_CPN_ALGO_PARAM1,  algo_param1);
    PUT_REG_INFO(DIP_X_CPN_ALGO_PARAM2,  algo_param2);
    PUT_REG_INFO(DIP_X_CPN_GTM_X0X1,     x0x1);
    PUT_REG_INFO(DIP_X_CPN_GTM_X2X3,     x2x3);
    PUT_REG_INFO(DIP_X_CPN_GTM_X4X5,     x4x5);
    PUT_REG_INFO(DIP_X_CPN_GTM_X6,       x6);
    PUT_REG_INFO(DIP_X_CPN_GTM_Y0Y1,     y0y1);
    PUT_REG_INFO(DIP_X_CPN_GTM_Y2Y3,     y2y3);
    PUT_REG_INFO(DIP_X_CPN_GTM_Y4Y5,     y4y5);
    PUT_REG_INFO(DIP_X_CPN_GTM_Y6,       y6);
    PUT_REG_INFO(DIP_X_CPN_GTM_S0S1,     s0s1);
    PUT_REG_INFO(DIP_X_CPN_GTM_S2S3,     s2s3);
    PUT_REG_INFO(DIP_X_CPN_GTM_S4S5,     s4s5);
    PUT_REG_INFO(DIP_X_CPN_GTM_S6S7,     s6s7);

    return  (*this);
}

template <>
MBOOL
ISP_MGR_CPN2_T::
get(ISP_NVRAM_CPN_T& rParam, const dip_x_reg_t* pReg) const
{
    if (pReg)
    {
        GET_REG_INFO_BUF(DIP_X_CPN_HDR_CTL_EN,   hdr_ctl_en);
        //GET_REG_INFO_BUF(DIP_X_CPN_IN_IMG_SIZE,  in_img_size);
        GET_REG_INFO_BUF(DIP_X_CPN_ALGO_PARAM1,  algo_param1);
        GET_REG_INFO_BUF(DIP_X_CPN_ALGO_PARAM2,  algo_param2);
        GET_REG_INFO_BUF(DIP_X_CPN_GTM_X0X1,     x0x1);
        GET_REG_INFO_BUF(DIP_X_CPN_GTM_X2X3,     x2x3);
        GET_REG_INFO_BUF(DIP_X_CPN_GTM_X4X5,     x4x5);
        GET_REG_INFO_BUF(DIP_X_CPN_GTM_X6,       x6);
        GET_REG_INFO_BUF(DIP_X_CPN_GTM_Y0Y1,     y0y1);
        GET_REG_INFO_BUF(DIP_X_CPN_GTM_Y2Y3,     y2y3);
        GET_REG_INFO_BUF(DIP_X_CPN_GTM_Y4Y5,     y4y5);
        GET_REG_INFO_BUF(DIP_X_CPN_GTM_Y6,       y6);
        GET_REG_INFO_BUF(DIP_X_CPN_GTM_S0S1,     s0s1);
        GET_REG_INFO_BUF(DIP_X_CPN_GTM_S2S3,     s2s3);
        GET_REG_INFO_BUF(DIP_X_CPN_GTM_S4S5,     s4s5);
        GET_REG_INFO_BUF(DIP_X_CPN_GTM_S6S7,     s6s7);
    }
    return MTRUE;
}

MBOOL
ISP_MGR_CPN2_T::
apply(const RAWIspCamInfo& rRawIspCamInfo, dip_x_reg_t* pReg)
{
    MBOOL bCPN2_EN = isEnable();

    addressErrorCheck("Before ISP_MGR_CPN2_T::apply()");

    //> prepar register value
#if 1
    if(bCPN2_EN)
    {
        MINT32 hdrEnable = m_bZHdrEnable;
        // TO SET:
        // Temporary we set AA & PS output control signal to 0
        MINT32 aalLE = 0; //hdrEnable ? 1 : 0;
        MINT32 psSE = 0; //hdrEnable ? 1 : 0;

        MUINT32 orig = 0;
        orig = REG_INFO_VALUE(DIP_X_CPN_HDR_CTL_EN);
        REG_INFO_VALUE(DIP_X_CPN_HDR_CTL_EN) = (orig & 0x000000F0) //((edgeInfo & 0xF) << 4)
                                           | ((hdrEnable & 0x1))
                                           | ((aalLE & 0x1) << 1)
                                           | ((psSE & 0x1)  << 2)
                                           | 0;

        if(hdrEnable)
        {
            CAM_LOGD_IF(debugDump, "CPN2 in: hdrEnable(%d), HDR_RATIO(%d)", hdrEnable, rRawIspCamInfo.rAEInfo.i4LESE_Ratio);

            MINT32 aeLeSeRatio_x100 = CLAMP(rRawIspCamInfo.rAEInfo.i4LESE_Ratio, 100, 1600);
            MINT32 HDR_GAIN = (51200 / aeLeSeRatio_x100 + 1) >> 1; // do rounding
            MINT32 HDR_RATIO = (16 * aeLeSeRatio_x100 + 50) / 100;

            MINT32 len_x = sizeof(rRawIspCamInfo.rAEInfo.rHdrToneInfo.rHdrCPNProp.GTM_X)/sizeof(rRawIspCamInfo.rAEInfo.rHdrToneInfo.rHdrCPNProp.GTM_X[0]);
            MINT32 len_y = sizeof(rRawIspCamInfo.rAEInfo.rHdrToneInfo.rHdrCPNProp.GTM_Y)/sizeof(rRawIspCamInfo.rAEInfo.rHdrToneInfo.rHdrCPNProp.GTM_Y[0]);
            MINT32 len_s = sizeof(rRawIspCamInfo.rAEInfo.rHdrToneInfo.rHdrCPNProp.GTM_S)/sizeof(rRawIspCamInfo.rAEInfo.rHdrToneInfo.rHdrCPNProp.GTM_S[0]);

            std::vector<MINT32> gtm_x, gtm_y, gtm_s;
            if( len_x != 7 || len_y != 7 || len_s != 8)
            {
                CAM_LOGE("GTM_X(%d), GTM_Y(%d) and GTM_S(%d) total element number not correct.", len_x, len_y, len_s);
                return false;
            }
            for(int i = 0 ; i < len_x ; i++)
            {
                gtm_x.push_back(CLAMP(rRawIspCamInfo.rAEInfo.rHdrToneInfo.rHdrCPNProp.GTM_X[i], 0, 4095));
                gtm_y.push_back(CLAMP(rRawIspCamInfo.rAEInfo.rHdrToneInfo.rHdrCPNProp.GTM_Y[i], 0, 4095));
            }
            for(int i = 0 ; i < len_s ; i++)
            {
                gtm_s.push_back(CLAMP(rRawIspCamInfo.rAEInfo.rHdrToneInfo.rHdrCPNProp.GTM_S[i], 0, 4095));
            }


            REG_INFO_VALUE(DIP_X_CPN_ALGO_PARAM1) = (HDR_RATIO & 0x01FF)
                                                  | ((HDR_GAIN  & 0x01FF) << 16)
                                                  | 0;
            //for(const auto &it : gtm_x)
            REG_INFO_VALUE(DIP_X_CPN_GTM_X0X1) = (gtm_x.at(0) & 0xFFF)
                                                |((gtm_x.at(1) & 0xFFF) << 16)
                                                | 0;
            REG_INFO_VALUE(DIP_X_CPN_GTM_X2X3) = (gtm_x.at(2) & 0xFFF)
                                                |((gtm_x.at(3) & 0xFFF) << 16)
                                                | 0;
            REG_INFO_VALUE(DIP_X_CPN_GTM_X4X5) = (gtm_x.at(4) & 0xFFF)
                                                |((gtm_x.at(5) & 0xFFF) << 16)
                                                | 0;
            REG_INFO_VALUE(DIP_X_CPN_GTM_X6)   = (gtm_x.at(6) & 0xFFF)
                                                | 0;
            REG_INFO_VALUE(DIP_X_CPN_GTM_Y0Y1) = (gtm_y.at(0) & 0xFFF)
                                                |((gtm_y.at(1) & 0xFFF) << 16)
                                                | 0;
            REG_INFO_VALUE(DIP_X_CPN_GTM_Y2Y3) = (gtm_y.at(2) & 0xFFF)
                                                |((gtm_y.at(3) & 0xFFF) << 16)
                                                | 0;
            REG_INFO_VALUE(DIP_X_CPN_GTM_Y4Y5) = (gtm_y.at(4) & 0xFFF)
                                                |((gtm_y.at(5) & 0xFFF) << 16)
                                                | 0;
            REG_INFO_VALUE(DIP_X_CPN_GTM_Y6)   = (gtm_y.at(6) & 0xFFF)
                                                | 0;
            REG_INFO_VALUE(DIP_X_CPN_GTM_S0S1) = (gtm_s.at(0) & 0xFFF)
                                                |((gtm_s.at(1) & 0xFFF) << 16)
                                                | 0;
            REG_INFO_VALUE(DIP_X_CPN_GTM_S2S3) = (gtm_s.at(2) & 0xFFF)
                                                |((gtm_s.at(3) & 0xFFF) << 16)
                                                | 0;
            REG_INFO_VALUE(DIP_X_CPN_GTM_S4S5) = (gtm_s.at(4) & 0xFFF)
                                                |((gtm_s.at(5) & 0xFFF) << 16)
                                                | 0;
            REG_INFO_VALUE(DIP_X_CPN_GTM_S6S7) = (gtm_s.at(6) & 0xFFF)
                                                |((gtm_s.at(7) & 0xFFF) << 16)
                                                | 0;
        }   // if hdrEnable end

    }   // if bCPN2_EN end
#endif

    // Register setting
    writeRegs(static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum, pReg);

    dumpRegInfo("CPN2");

    return  MTRUE;
}


}
