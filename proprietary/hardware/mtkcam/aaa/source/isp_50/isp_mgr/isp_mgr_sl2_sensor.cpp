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
#define LOG_TAG "isp_mgr_sl2"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include <camera_custom_nvram.h>
#include <awb_feature.h>
#include <awb_param.h>
#include <ae_feature.h>
#include <ae_param.h>

#include "isp_mgr.h"
#include <drv/tuning_mgr.h>
#include <private/aaa_utils.h>
#include <array>

using namespace NSIspTuningv3;

#define MY_INST_SL2F NS3Av3::INST_T<ISP_MGR_SL2F_T>
static std::array<MY_INST_SL2F, SENSOR_IDX_MAX> gMultitonSL2F;

#define MY_INST_SL2G NS3Av3::INST_T<ISP_MGR_SL2G_T>
static std::array<MY_INST_SL2G, SENSOR_IDX_MAX> gMultitonSL2G;

#if defined(HAVE_AEE_FEATURE)
#include <aee.h>
#define AEE_ASSERT_SL2(String) \
          do { \
              aee_system_exception( \
                  LOG_TAG, \
                  NULL, \
                  DB_OPT_DEFAULT, \
                  String); \
          } while(0)
#else
#define AEE_ASSERT_SL2(String)
#endif

#define CLAMP(x,min,max)       (((x) > (max)) ? (max) : (((x) < (min)) ? (min) : (x)))

#define SLP_PREC_F_BW   16
#define SLP_PREC_F_SCAL 65536

static MINT32 _maxR(MINT32 cx, MINT32 cy, MINT32 w, MINT32 h)
{
    // Comparing which corner is the longest distance, and use it as R.

    //corner1
    MINT32 y_dist_corner1 = (0 - cy);
    MINT32 x_dist_corner1 = (0 - cx);
    MINT32 CircleEq_corner1;
    if (y_dist_corner1 < 0)
        y_dist_corner1 = -y_dist_corner1;
    if (x_dist_corner1 < 0)
        x_dist_corner1 = -x_dist_corner1;
    if (x_dist_corner1 >= y_dist_corner1)
        CircleEq_corner1 = x_dist_corner1;
    else
        CircleEq_corner1 = y_dist_corner1;

    MINT32 tmp_corner1 = ((x_dist_corner1 + y_dist_corner1)*1448) >> 11;
    if (tmp_corner1 >= CircleEq_corner1)
        CircleEq_corner1 = tmp_corner1;

    //corner 2
    MINT32 y_dist_corner2 = (0 - cy);
    MINT32 x_dist_corner2 = (w - cx);
    MINT32 CircleEq_corner2;

    if (y_dist_corner2 < 0)
        y_dist_corner2 = -y_dist_corner2;
    if (x_dist_corner2 < 0)
        x_dist_corner2 = -x_dist_corner2;
    if (x_dist_corner2 >= y_dist_corner2)
        CircleEq_corner2 = x_dist_corner2;
    else
        CircleEq_corner2 = y_dist_corner2;

    MINT32 tmp_corner2 = ((x_dist_corner2 + y_dist_corner2)*1448) >> 11;
    if(tmp_corner2 >= CircleEq_corner2)
        CircleEq_corner2 = tmp_corner2;

    //corner 3
    MINT32 y_dist_corner3 = (h - cy);
    MINT32 x_dist_corner3 = (0 - cx);
    MINT32 CircleEq_corner3;

    if (y_dist_corner3 < 0)
        y_dist_corner3 = -y_dist_corner3;
    if (x_dist_corner3 < 0)
        x_dist_corner3 = -x_dist_corner3;
    if (x_dist_corner3 >= y_dist_corner3)
        CircleEq_corner3 = x_dist_corner3;
    else
        CircleEq_corner3 = y_dist_corner3;

    MINT32 tmp_corner3 = ((x_dist_corner3 + y_dist_corner3)*1448) >> 11;
    if(tmp_corner3 >= CircleEq_corner3)
        CircleEq_corner3 = tmp_corner3;

    //corner4
    MINT32 y_dist_corner4 = (h - cy);
    MINT32 x_dist_corner4 = (w - cx);
    MINT32 CircleEq_corner4;

    if (y_dist_corner4 < 0)
        y_dist_corner4 = -y_dist_corner4;
    if (x_dist_corner4 < 0)
        x_dist_corner4 = -x_dist_corner4;
    if (x_dist_corner4 >= y_dist_corner4)
        CircleEq_corner4 = x_dist_corner4;
    else
        CircleEq_corner4 = y_dist_corner4;

    MINT32 tmp_corner4 = ((x_dist_corner4 + y_dist_corner4)*1448) >> 11;
    if(tmp_corner4 >= CircleEq_corner4)
        CircleEq_corner4 = tmp_corner4;

    MINT32 maxR;
    if (CircleEq_corner4 > CircleEq_corner3)
        maxR = CircleEq_corner4;
    else
        maxR = CircleEq_corner3;

    if (CircleEq_corner2 > maxR)
        maxR = CircleEq_corner2;

    if (CircleEq_corner1 > maxR)
        maxR = CircleEq_corner1;

    return maxR;
}

namespace NSIspTuningv3
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  SL2F (pass 1)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_SL2F_T&
ISP_MGR_SL2F_T::
getInstance(ESensorDev_T const eSensorDev)
{
    auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(eSensorDev);

    MY_ASSERT(i4SensorIdx < SENSOR_IDX_MAX);
    MY_ASSERT(i4SensorIdx >= 0);


    MY_INST_SL2F& rSingleton = gMultitonSL2F[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<ISP_MGR_SL2F_T>(eSensorDev);
    } );

    return *(rSingleton.instance);
}

template <>
ISP_MGR_SL2F_T&
ISP_MGR_SL2F_T::
put(ISP_NVRAM_SL2_T const& rParam)
{

    m_rSL2FParam = rParam;
    //GET_REG_INFO(CAM_SL2F_CEN,      cen);
    //GET_REG_INFO(CAM_SL2F_RR_CON0,  rr_con0);
    //GET_REG_INFO(CAM_SL2F_RR_CON1,  rr_con1);
    //GET_REG_INFO(CAM_SL2F_GAIN,   gain);

    return  (*this);
}

template <>
ISP_MGR_SL2F_T&
ISP_MGR_SL2F_T::
get(ISP_NVRAM_SL2_T& rParam)
{
    GET_REG_INFO(CAM_SL2F_CEN,      cen);
    GET_REG_INFO(CAM_SL2F_RR_CON0,  rr_con0);
    GET_REG_INFO(CAM_SL2F_RR_CON1,  rr_con1);
    GET_REG_INFO(CAM_SL2F_GAIN,   gain);

    return  (*this);
}

template <>
ISP_MGR_SL2F_T&
ISP_MGR_SL2F_T::
get(ISP_NVRAM_SL2F_GET_T& rParam)
{
    GET_REG_INFO(CAM_SL2F_CEN,      cen);
    GET_REG_INFO(CAM_SL2F_RR_CON0,  rr_con0);
    GET_REG_INFO(CAM_SL2F_RR_CON1,  rr_con1);
    GET_REG_INFO(CAM_SL2F_GAIN,     gain);
    GET_REG_INFO(CAM_SL2F_SLP_CON0, con0);
    GET_REG_INFO(CAM_SL2F_SLP_CON1, con1);
    GET_REG_INFO(CAM_SL2F_SLP_CON2, con2);
    GET_REG_INFO(CAM_SL2F_SLP_CON3, con3);
    GET_REG_INFO(CAM_SL2F_RZ,       rz);
    GET_REG_INFO(CAM_SL2F_XOFF,     xoff);
    GET_REG_INFO(CAM_SL2F_YOFF,     yoff);
    GET_REG_INFO(CAM_SL2F_SIZE,     size);


    return  (*this);
}


MBOOL
ISP_MGR_SL2F_T::
apply(const ISP_RAW_SIZE_T& rRawSize, EIspProfile_T /*eIspProfile*/, TuningMgr& rTuning, MINT32 i4SubsampleIdex)
{
    MBOOL bSL2F_EN = isEnable();

    if(bSL2F_EN){
        MINT32 SL2_CENTR_X  = m_rSL2FParam.cen.bits.SL2_CENTR_X;
        MINT32 SL2_CENTR_Y  = m_rSL2FParam.cen.bits.SL2_CENTR_Y;
        MINT32 maxR         = _maxR(SL2_CENTR_X, SL2_CENTR_Y, rRawSize.u4RAW_Width, rRawSize.u4RAW_Height);
        MINT32 tempGain0    = m_rSL2FParam.rr_con1.bits.SL2_GAIN_0;
        MINT32 tempGain1    = m_rSL2FParam.rr_con1.bits.SL2_GAIN_1;
        MINT32 tempGain2    = m_rSL2FParam.gain.bits.SL2_GAIN_2;
        MINT32 tempGain3    = m_rSL2FParam.gain.bits.SL2_GAIN_3;
        MINT32 tempGain4    = m_rSL2FParam.gain.bits.SL2_GAIN_4;
        MINT32 tempR0       = m_rSL2FParam.rr_con0.bits.SL2_R_0;
        MINT32 tempR1       = m_rSL2FParam.rr_con0.bits.SL2_R_1;
        MINT32 tempR2       = m_rSL2FParam.rr_con1.bits.SL2_R_2;

        //Evalutaing slope
        MINT32 SL2_SLP_1  = (MINT32)(((float)(tempGain1 - tempGain0)/(float)tempR0)*SLP_PREC_F_SCAL + 0.5);
        MINT32 SL2_SLP_2  = (MINT32)(((float)(tempGain2 - tempGain1)/(float)(tempR1 - tempR0))*SLP_PREC_F_SCAL + 0.5);
        MINT32 SL2_SLP_3  = (MINT32)(((float)(tempGain3 - tempGain2)/(float)(tempR2 - tempR1))*SLP_PREC_F_SCAL + 0.5);
        MINT32 SL2_SLP_4  = (MINT32)(((float)(tempGain4 - tempGain3)/(float)(maxR -   tempR2))*SLP_PREC_F_SCAL + 0.5);

        DIP_X_REG_SL2_CEN rCen;
        DIP_X_REG_SL2_RR_CON0 rRRCond0;
        DIP_X_REG_SL2_RR_CON1 rRRCond1;
        DIP_X_REG_SL2_GAIN rGain;
        DIP_X_REG_SL2_SLP_CON0 rSlopCon0;
        DIP_X_REG_SL2_SLP_CON1 rSlopCon1;
        DIP_X_REG_SL2_SLP_CON2 rSlopCon2;
        DIP_X_REG_SL2_SLP_CON3 rSlopCon3;

        rCen.Raw=0;
        rRRCond0.Raw=0;
        rRRCond1.Raw=0;
        rGain.Raw=0;
        rSlopCon0.Raw=0;
        rSlopCon1.Raw=0;
        rSlopCon2.Raw=0;
        rSlopCon3.Raw=0;

        rCen.Bits.SL2_CENTR_X = SL2_CENTR_X;
        rCen.Bits.SL2_CENTR_Y = SL2_CENTR_Y;
        rRRCond0.Bits.SL2_R_0 = tempR0;
        rRRCond0.Bits.SL2_R_1 = tempR1;
        rRRCond1.Bits.SL2_R_2 = tempR2;
        rRRCond1.Bits.SL2_GAIN_0 = tempGain0;
        rRRCond1.Bits.SL2_GAIN_1 = tempGain1;
        rGain.Bits.SL2_GAIN_2 = tempGain2;
        rGain.Bits.SL2_GAIN_3 = tempGain3;
        rGain.Bits.SL2_GAIN_4 = tempGain4;
        rSlopCon0.Bits.SL2_SLP_1 = SL2_SLP_1;
        rSlopCon1.Bits.SL2_SLP_2 = SL2_SLP_2;
        rSlopCon2.Bits.SL2_SLP_3 = SL2_SLP_3;
        rSlopCon3.Bits.SL2_SLP_4 = SL2_SLP_4;


        REG_INFO_VALUE(CAM_SL2F_CEN)       = rCen.Raw;
        REG_INFO_VALUE(CAM_SL2F_RR_CON0)   = rRRCond0.Raw;
        REG_INFO_VALUE(CAM_SL2F_RR_CON1)   = rRRCond1.Raw;
        REG_INFO_VALUE(CAM_SL2F_GAIN)      = rGain.Raw;
        REG_INFO_VALUE(CAM_SL2F_SLP_CON0)  = rSlopCon0.Raw;
        REG_INFO_VALUE(CAM_SL2F_SLP_CON1)  = rSlopCon1.Raw;
        REG_INFO_VALUE(CAM_SL2F_SLP_CON2)  = rSlopCon2.Raw;
        REG_INFO_VALUE(CAM_SL2F_SLP_CON3)  = rSlopCon3.Raw;
        REG_INFO_VALUE(CAM_SL2F_RZ)        = (2048<<16) | 2048;
        REG_INFO_VALUE(CAM_SL2F_XOFF)      = 0;
        REG_INFO_VALUE(CAM_SL2F_YOFF)      = 0;
    }

    rTuning.updateEngine(eTuningMgrFunc_SL2F, bSL2F_EN, i4SubsampleIdex);

    // TOP
    //TUNING_MGR_WRITE_BITS_CAM((&rTuning), CAM_CTL_DMA_EN, SL2F_EN, bSL2F_EN, i4SubsampleIdex);
    ISP_MGR_CTL_EN_P1_T::getInstance(m_eSensorDev).setEnable_SL2F(bSL2F_EN);

    // Register setting
    rTuning.tuningMgrWriteRegs(static_cast<TUNING_MGR_REG_IO_STRUCT*>(m_pRegInfo),
            m_u4RegInfoNum, i4SubsampleIdex);

    dumpRegInfoP1("SL2F");

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  SL2G (pass 2)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_SL2G_T&
ISP_MGR_SL2G_T::
getInstance(ESensorDev_T const eSensorDev)
{
    auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(eSensorDev);

    MY_ASSERT(i4SensorIdx < SENSOR_IDX_MAX);
    MY_ASSERT(i4SensorIdx >= 0);


    MY_INST_SL2G& rSingleton = gMultitonSL2G[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<ISP_MGR_SL2G_T>(eSensorDev);
    } );

    return *(rSingleton.instance);
}

template <>
ISP_MGR_SL2G_T&
ISP_MGR_SL2G_T::
put(ISP_NVRAM_SL2_T const& rParam)
{
    m_rSL2GParam = rParam;

    return  (*this);
}

template <>
MBOOL
ISP_MGR_SL2G_T::
get(ISP_NVRAM_SL2G_GET_T& rParam, const dip_x_reg_t* pReg) const
{
    if (pReg)
    {
        GET_REG_INFO_BUF(DIP_X_SL2G_CEN, cen);
        GET_REG_INFO_BUF(DIP_X_SL2G_RR_CON0, rr_con0);
        GET_REG_INFO_BUF(DIP_X_SL2G_RR_CON1, rr_con1);
        GET_REG_INFO_BUF(DIP_X_SL2G_GAIN, gain);
        GET_REG_INFO_BUF(DIP_X_SL2G_SLP_CON0, con0);
        GET_REG_INFO_BUF(DIP_X_SL2G_SLP_CON1, con1);
        GET_REG_INFO_BUF(DIP_X_SL2G_SLP_CON2, con2);
        GET_REG_INFO_BUF(DIP_X_SL2G_SLP_CON3, con3);
        GET_REG_INFO_BUF(DIP_X_SL2G_RZ, rz);
        GET_REG_INFO_BUF(DIP_X_SL2G_XOFF, xoff);
        GET_REG_INFO_BUF(DIP_X_SL2G_YOFF, yoff);
        GET_REG_INFO_BUF(DIP_X_SL2G_SIZE, size);
    }
    return MTRUE;
}

MBOOL
ISP_MGR_SL2G_T::
apply(const CROP_RZ_INFO_T& rCropRz, EIspProfile_T /*eIspProfile*/, dip_x_reg_t* pReg)
{
    MBOOL bSL2G_EN = isEnable();

    if(bSL2G_EN){
        MINT32 SL2_CENTR_X  = m_rSL2GParam.cen.bits.SL2_CENTR_X;
        MINT32 SL2_CENTR_Y  = m_rSL2GParam.cen.bits.SL2_CENTR_Y;
        MINT32 maxR         = _maxR(SL2_CENTR_X, SL2_CENTR_Y, rCropRz.i4TGoutW, rCropRz.i4TGoutH);
        MINT32 tempGain0    = m_rSL2GParam.rr_con1.bits.SL2_GAIN_0;
        MINT32 tempGain1    = m_rSL2GParam.rr_con1.bits.SL2_GAIN_1;
        MINT32 tempGain2    = m_rSL2GParam.gain.bits.SL2_GAIN_2;
        MINT32 tempGain3    = m_rSL2GParam.gain.bits.SL2_GAIN_3;
        MINT32 tempGain4    = m_rSL2GParam.gain.bits.SL2_GAIN_4;
        MINT32 tempR0       = m_rSL2GParam.rr_con0.bits.SL2_R_0;
        MINT32 tempR1       = m_rSL2GParam.rr_con0.bits.SL2_R_1;
        MINT32 tempR2       = m_rSL2GParam.rr_con1.bits.SL2_R_2;

        //Evalutaing slope
        MINT32 SL2_SLP_1  = (MINT32)(((float)(tempGain1 - tempGain0)/(float)tempR0)*SLP_PREC_F_SCAL + 0.5);
        MINT32 SL2_SLP_2  = (MINT32)(((float)(tempGain2 - tempGain1)/(float)(tempR1 - tempR0))*SLP_PREC_F_SCAL + 0.5);
        MINT32 SL2_SLP_3  = (MINT32)(((float)(tempGain3 - tempGain2)/(float)(tempR2 - tempR1))*SLP_PREC_F_SCAL + 0.5);
        MINT32 SL2_SLP_4  = (MINT32)(((float)(tempGain4 - tempGain3)/(float)(maxR -   tempR2))*SLP_PREC_F_SCAL + 0.5);

        DIP_X_REG_SL2_CEN rCen;
        DIP_X_REG_SL2_RR_CON0 rRRCond0;
        DIP_X_REG_SL2_RR_CON1 rRRCond1;
        DIP_X_REG_SL2_GAIN rGain;
        DIP_X_REG_SL2_SLP_CON0 rSlopCon0;
        DIP_X_REG_SL2_SLP_CON1 rSlopCon1;
        DIP_X_REG_SL2_SLP_CON2 rSlopCon2;
        DIP_X_REG_SL2_SLP_CON3 rSlopCon3;

        rCen.Raw=0;
        rRRCond0.Raw=0;
        rRRCond1.Raw=0;
        rGain.Raw=0;
        rSlopCon0.Raw=0;
        rSlopCon1.Raw=0;
        rSlopCon2.Raw=0;
        rSlopCon3.Raw=0;

        rCen.Bits.SL2_CENTR_X = SL2_CENTR_X;
        rCen.Bits.SL2_CENTR_Y = SL2_CENTR_Y;
        rRRCond0.Bits.SL2_R_0 = tempR0;
        rRRCond0.Bits.SL2_R_1 = tempR1;
        rRRCond1.Bits.SL2_R_2 = tempR2;
        rRRCond1.Bits.SL2_GAIN_0 = tempGain0;
        rRRCond1.Bits.SL2_GAIN_1 = tempGain1;
        rGain.Bits.SL2_GAIN_2 = tempGain2;
        rGain.Bits.SL2_GAIN_3 = tempGain3;
        rGain.Bits.SL2_GAIN_4 = tempGain4;
        rSlopCon0.Bits.SL2_SLP_1 = SL2_SLP_1;
        rSlopCon1.Bits.SL2_SLP_2 = SL2_SLP_2;
        rSlopCon2.Bits.SL2_SLP_3 = SL2_SLP_3;
        rSlopCon3.Bits.SL2_SLP_4 = SL2_SLP_4;


        REG_INFO_VALUE(DIP_X_SL2G_CEN)       = rCen.Raw;
        REG_INFO_VALUE(DIP_X_SL2G_RR_CON0)   = rRRCond0.Raw;
        REG_INFO_VALUE(DIP_X_SL2G_RR_CON1)   = rRRCond1.Raw;
        REG_INFO_VALUE(DIP_X_SL2G_GAIN)      = rGain.Raw;
        REG_INFO_VALUE(DIP_X_SL2G_SLP_CON0)  = rSlopCon0.Raw;
        REG_INFO_VALUE(DIP_X_SL2G_SLP_CON1)  = rSlopCon1.Raw;
        REG_INFO_VALUE(DIP_X_SL2G_SLP_CON2)  = rSlopCon2.Raw;
        REG_INFO_VALUE(DIP_X_SL2G_SLP_CON3)  = rSlopCon3.Raw;
        REG_INFO_VALUE(DIP_X_SL2G_RZ)        = (2048<<16) | 2048;
        REG_INFO_VALUE(DIP_X_SL2G_XOFF)      = 0;
        REG_INFO_VALUE(DIP_X_SL2G_YOFF)      = 0;
    }

    // TOP
    ISP_WRITE_ENABLE_BITS(pReg, DIP_X_CTL_RGB_EN, SL2G_EN, bSL2G_EN);

    // Register setting
    writeRegs(static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum, pReg);


    dumpRegInfo("SL2G");

    return  MTRUE;
}

}
