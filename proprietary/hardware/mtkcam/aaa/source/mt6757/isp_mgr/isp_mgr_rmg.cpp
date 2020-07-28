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
#define LOG_TAG "isp_mgr_rmg"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include "isp_mgr.h"
#include <mtkcam/drv/IHalSensor.h>


namespace NSIspTuningv3
{

#define CLAMP(x,min,max)       (((x) > (max)) ? (max) : (((x) < (min)) ? (min) : (x)))

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  RMG
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


ISP_MGR_RMG_T&
ISP_MGR_RMG_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
    {
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_RMG_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_RMG_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_RMG_DEV<ESensorDev_Sub>::getInstance();
    default:
        CAM_LOGE("eSensorDev = %d", (MINT32)eSensorDev);
        return  ISP_MGR_RMG_DEV<ESensorDev_Main>::getInstance();
    }
}

template <>
ISP_MGR_RMG_T&
ISP_MGR_RMG_T::
put(ISP_NVRAM_RMG_T const& rParam)
{
    PUT_REG_INFO(CAM_RMG_HDR_CFG, hdr_cfg);
    PUT_REG_INFO(CAM_RMG_HDR_GAIN, hdr_gain);
    PUT_REG_INFO(CAM_RMG_HDR_CFG2, hdr_cfg2);
    return  (*this);
}


template <>
ISP_MGR_RMG_T&
ISP_MGR_RMG_T::
get(ISP_NVRAM_RMG_T& rParam)
{
    GET_REG_INFO(CAM_RMG_HDR_CFG, hdr_cfg);
    GET_REG_INFO(CAM_RMG_HDR_GAIN, hdr_gain);
    GET_REG_INFO(CAM_RMG_HDR_CFG2, hdr_cfg2);
    return  (*this);
}

#if 0
MVOID
ISP_MGR_RMG_T::
setAeLeSeRatio(MINT32 aeRatio)
{
    CAM_LOGD("[%s] aeRatio(%d)",__func__,aeRatio);
    mAeExpRatio = aeRatio/100.0;

    if(mAeExpRatio < 1.0)
    {
        CAM_LOGD("mAeExpRatio < 1.0");
        mAeExpRatio = 1.0;
    }

    CAM_LOGD("[%s] mAeExpRatio(%f),dev(%d)",__func__,mAeExpRatio,m_eSensorDev);
}
#endif

MBOOL
ISP_MGR_RMG_T::
apply(RAWIspCamInfo& rRawIspCamInfo, TuningMgr& rTuning, MINT32 i4SubsampleIdex)
{
    MBOOL bRMG_EN = isEnable();

    addressErrorCheck("Before ISP_MGR_RMG_T::apply()");

    //> prepar register value

    if(bRMG_EN)
    {
        // == log input parameter ===
        // TODO After vhdr stable, must remove this log to avoid performance
        CAM_LOGD("in: LeSeRatio(%d),hdrTH(%d), hdrTurn(%d)", rRawIspCamInfo.rAEInfo.i4LESE_Ratio, rRawIspCamInfo.rAEInfo.i4HdrSeg, rRawIspCamInfo.rAEInfo.i4HdrTurn);

        MINT32 aeLeSeRatio = (rRawIspCamInfo.rAEInfo.i4LESE_Ratio + 50) / 100;
        CAM_LOGD_IF(debugDump, "isEnable(%d), i4LESE_Ratio(%d)->asLeSeRatio(%d)", bRMG_EN, rRawIspCamInfo.rAEInfo.i4LESE_Ratio, aeLeSeRatio);
#if 0
        MINT32 ratioIdx = 0;

        switch(aeLeSeRatio)
        {
            case 1:
                ratioIdx = IND_RATIO_1;
                break;
            case 2:
                ratioIdx = IND_RATIO_2;
                break;
            case 4:
                ratioIdx = IND_RATIO_4;
                break;
            case 8:
                ratioIdx = IND_RATIO_8;
                break;
            case 16:
                ratioIdx = IND_RATIO_16;
                break;
            default:
                if(m_bZHdrEnable)
                    CAM_LOGE("wrong ae ratio, camInfo_aeRatio(%d), calculate(%d), use ratio 1", rRawIspCamInfo.rAEInfo.i4LESE_Ratio, aeLeSeRatio);
                ratioIdx = IND_RATIO_1;
                break;
        }
#endif

        // HDR Gain use formula to calculate
        MINT32 aeLeSeRatio_x100 = CLAMP(rRawIspCamInfo.rAEInfo.i4LESE_Ratio, 100, 1600);
        MINT32 RMG_GAIN = 25600 / aeLeSeRatio_x100;
        MINT32 RMG_RATIO = (16 * aeLeSeRatio_x100+50)/100;

        // HDR_CFG
        MINT32 iHdrEnable = m_bIHdrEnable ? 1 : 0;
        MINT32 zHdrEnable = m_bZHdrEnable ? 1 : 0;
        MINT32 zLeR = 0;
        MINT32 zLeG = 0;
        MINT32 zLeB = 0;
        MINT32 hdrTH = rRawIspCamInfo.rAEInfo.i4HdrSeg; // Use AE_INFO_T i4HdrSeg
        MINT32 hdrTurn = rRawIspCamInfo.rAEInfo.i4HdrTurn; // Use AE_INFO_T i4HdrTurn
        MINT32 hdrOSC = 4080;

        // get Z-Pattern
        switch(mZPattern)
        {
            case ZHDR_MODE_G0_R0_B0:
            {
                zLeR = 0;
                zLeG = 0;
                zLeB = 0;
                break;
            }
            case ZHDR_MODE_G0_R0_B1:
            {
                zLeR = 0;
                zLeG = 0;
                zLeB = 1;
                break;
            }
            case ZHDR_MODE_G0_R1_B0:
            {
                zLeR = 1;
                zLeG = 0;
                zLeB = 0;
                break;
            }
            case ZHDR_MODE_G0_R1_B1:
            {
                zLeR = 1;
                zLeG = 0;
                zLeB = 1;
                break;
            }
            case ZHDR_MODE_G1_R0_B0:
            {
                zLeR = 0;
                zLeG = 1;
                zLeB = 0;
                break;
            }
            case ZHDR_MODE_G1_R0_B1:
            {
                zLeR = 0;
                zLeG = 1;
                zLeB = 1;
                break;
            }
            case ZHDR_MODE_G1_R1_B0:
            {
                zLeR = 1;
                zLeG = 1;
                zLeB = 0;
                break;
            }
            case ZHDR_MODE_G1_R1_B1:
            {
                zLeR = 1;
                zLeG = 1;
                zLeB = 1;
                break;
            }
            default:
                if(m_bZHdrEnable)
                    CAM_LOGE("zvHDR Enable, but z-pattern is %d", mZPattern);
                break;
        }

        CAM_LOGD_IF(debugDump, "hdrOSC(%d), hdrTH(%d), hdrTurn(%d), z-patn-RGB(%d,%d,%d), z-enable(%d), i-LeFirst(%d), i-Enable(%d)",
                    hdrOSC, hdrTH, hdrTurn, zLeR, zLeG, zLeB, zHdrEnable, mLeFirst, iHdrEnable);

        REG_INFO_VALUE(CAM_RMG_HDR_CFG) = ((hdrOSC&0x0FFF) << 20)
                                            | ((hdrTH&0x0FFF) << 8)
                                            | ((zLeB&0x1) << 7)
                                            | ((zLeG&0x1) << 6)
                                            | ((zLeR&0x1) << 5)
                                            | ((zHdrEnable&0x1) << 4)
                                            | ((mLeFirst&0x1) << 1)
                                            | ((iHdrEnable&0x1))
                                            | 0;

        REG_INFO_VALUE(CAM_RMG_HDR_CFG2) = (hdrTurn & 0x7FF) | 0;

        // HDR_GAIN
        MINT32 leInv = reinterpret_cast<ISP_CAM_RMG_HDR_GAIN_T*>(REG_INFO_VALUE_PTR(CAM_RMG_HDR_GAIN))->RMG_LE_INV_CTL;
#if 0
        MINT32 gainVal = ((leInv&0x07) << 28)
                            | ((tuningTable[ratioIdx][IND_RATIO]&0x01FF) << 16)
                            | ((tuningTable[ratioIdx][IND_GAIN]&0x01FF))
                            | 0;
#endif
        MINT32 gainVal = ((leInv&0x07) << 28)
                            | ((RMG_RATIO&0x01FF) << 16)
                            | ((RMG_GAIN&0x01FF))
                            | 0;
#if 0
        CAM_LOGD_IF(debugDump, "leInv(%d), ratio(%d), gain(%d)",  leInv, tuningTable[ratioIdx][IND_RATIO], tuningTable[ratioIdx][IND_GAIN]);
#endif
        CAM_LOGD_IF(debugDump, "leInv(%d), ratio(%d), gain(%d)",  leInv, RMG_RATIO, RMG_GAIN);

        REG_INFO_VALUE(CAM_RMG_HDR_GAIN) = gainVal;

    } // if bRMG_EN end

    rTuning.updateEngine(eTuningMgrFunc_RMG, bRMG_EN, 0);

    // TOP
    TUNING_MGR_WRITE_BITS_CAM((&rTuning), CAM_CTL_EN, RMG_EN, bRMG_EN, 0);

    ISP_MGR_CTL_EN_P1_T::getInstance(m_eSensorDev).setEnable_RMG(bRMG_EN);

    // Register setting
    rTuning.tuningMgrWriteRegs(
            static_cast<TUNING_MGR_REG_IO_STRUCT*>(m_pRegInfo),
            m_u4RegInfoNum, i4SubsampleIdex);


    dumpRegInfo("RMG");

    addressErrorCheck("After ISP_MGR_RMG_T::apply()");

    return  MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  RMG2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


ISP_MGR_RMG2_T&
ISP_MGR_RMG2_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
    {
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_RMG2_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_RMG2_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_RMG2_DEV<ESensorDev_Sub>::getInstance();
    default:
        CAM_LOGE("eSensorDev = %d", (MINT32)eSensorDev);
        return  ISP_MGR_RMG2_DEV<ESensorDev_Main>::getInstance();
    }
}

template <>
ISP_MGR_RMG2_T&
ISP_MGR_RMG2_T::
put(ISP_NVRAM_RMG_T const& rParam)
{
    PUT_REG_INFO(DIP_X_RMG2_HDR_CFG, hdr_cfg);
    PUT_REG_INFO(DIP_X_RMG2_HDR_GAIN, hdr_gain);
    PUT_REG_INFO(DIP_X_RMG2_HDR_CFG2, hdr_cfg2);
    return  (*this);
}

#if 0
template <>
ISP_MGR_RMG2_T&
ISP_MGR_RMG2_T::
get(ISP_NVRAM_RMG_T& rParam)
{
    GET_REG_INFO(DIP_X_RMG2_HDR_CFG, hdr_cfg);
    GET_REG_INFO(DIP_X_RMG2_HDR_GAIN, hdr_gain);
    GET_REG_INFO(DIP_X_RMG2_HDR_CFG2, hdr_cfg2);
    return  (*this);
}
#endif

template <>
MBOOL
ISP_MGR_RMG2_T::
get(ISP_NVRAM_RMG_T& rParam, const dip_x_reg_t* pReg) const
{
    if (pReg)
    {
        GET_REG_INFO_BUF(DIP_X_RMG2_HDR_CFG, hdr_cfg);
        GET_REG_INFO_BUF(DIP_X_RMG2_HDR_GAIN, hdr_gain);
        GET_REG_INFO_BUF(DIP_X_RMG2_HDR_CFG2, hdr_cfg2);
    }
    // other register not tuning in NVRAM
    return MTRUE;
}

#if 0
MVOID
ISP_MGR_RMG_T::
setAeLeSeRatio(MINT32 aeRatio)
{
    CAM_LOGD("[%s] aeRatio(%d)",__func__,aeRatio);
    mAeExpRatio = aeRatio/100.0;

    if(mAeExpRatio < 1.0)
    {
        CAM_LOGD("mAeExpRatio < 1.0");
        mAeExpRatio = 1.0;
    }

    CAM_LOGD("[%s] mAeExpRatio(%f),dev(%d)",__func__,mAeExpRatio,m_eSensorDev);
}
#endif

MBOOL
ISP_MGR_RMG2_T::
    apply(const RAWIspCamInfo& rRawIspCamInfo, dip_x_reg_t* pReg)
{
    MBOOL bRMG_EN = isEnable();

    // TOP
    ISP_WRITE_ENABLE_BITS(pReg, DIP_X_CTL_RGB_EN, RMG2_EN, bRMG_EN);
    //ISP_MGR_CTL_EN_P2_T::getInstance(m_eSensorDev).setEnable_RMG2(bRMG_EN);


    //> prepar register value
    if(bRMG_EN)
    {
        // == log input parameter ===
        CAM_LOGD("RMG2 in: LeSeRatio(%d),hdrTH(%d), hdrTurn(%d)", rRawIspCamInfo.rAEInfo.i4LESE_Ratio, rRawIspCamInfo.rAEInfo.i4HdrSeg, rRawIspCamInfo.rAEInfo.i4HdrTurn);

        MINT32 aeLeSeRatio = (rRawIspCamInfo.rAEInfo.i4LESE_Ratio + 50) / 100;
        CAM_LOGD_IF(debugDump,"isEnable(%d), i4LESE_Ratio(%d)->asLeSeRatio(%d)", bRMG_EN, rRawIspCamInfo.rAEInfo.i4LESE_Ratio, aeLeSeRatio);
#if 0
        MINT32 ratioIdx = 0;

        switch(aeLeSeRatio)
        {
             case 1:
                  ratioIdx = IND_RATIO_1;
                  break;
             case 2:
                  ratioIdx = IND_RATIO_2;
                  break;
             case 4:
                  ratioIdx = IND_RATIO_4;
                  break;
             case 8:
                  ratioIdx = IND_RATIO_8;
                  break;
             case 16:
                  ratioIdx = IND_RATIO_16;
                  break;
             default:
                  if(m_bZHdrEnable)
                      CAM_LOGE("wrong ae ratio, camInfo_aeRatio(%d), calculate(%d), use ratio 1", rRawIspCamInfo.rAEInfo.i4LESE_Ratio, aeLeSeRatio);
                  ratioIdx = IND_RATIO_1;
                  break;
        }
#endif

        // HDR Gain use formula to calculate
        MINT32 aeLeSeRatio_x100 = CLAMP(rRawIspCamInfo.rAEInfo.i4LESE_Ratio, 100, 1600);
        MINT32 RMG_GAIN = 25600 / aeLeSeRatio_x100;
        MINT32 RMG_RATIO = (16 * aeLeSeRatio_x100+50)/100;

        // HDR_CFG
        MINT32 iHdrEnable = m_bIHdrEnable ? 1 : 0;
        MINT32 zHdrEnable = m_bZHdrEnable ? 1 : 0;
        MINT32 zLeR = 0;
        MINT32 zLeG = 0;
        MINT32 zLeB = 0;
        MINT32 hdrTH = rRawIspCamInfo.rAEInfo.i4HdrSeg; // Use AE_INFO_T i4HdrSeg
        MINT32 hdrTurn = rRawIspCamInfo.rAEInfo.i4HdrTurn; // Use AE_INFO_T i4HdrTurn
        MINT32 hdrOSC = 4080;
        // get Z-Pattern
        switch(mZPattern)
        {
            case ZHDR_MODE_G0_R0_B0:
            {
                 zLeR = 0;
                 zLeG = 0;
                 zLeB = 0;
                 break;
            }
            case ZHDR_MODE_G0_R0_B1:
            {
                 zLeR = 0;
                 zLeG = 0;
                 zLeB = 1;
                 break;
            }
            case ZHDR_MODE_G0_R1_B0:
            {
                 zLeR = 1;
                 zLeG = 0;
                 zLeB = 0;
                 break;
            }
            case ZHDR_MODE_G0_R1_B1:
            {
                 zLeR = 1;
                 zLeG = 0;
                 zLeB = 1;
                 break;
            }
            case ZHDR_MODE_G1_R0_B0:
            {
                 zLeR = 0;
                 zLeG = 1;
                 zLeB = 0;
                 break;
            }
            case ZHDR_MODE_G1_R0_B1:
            {
                 zLeR = 0;
                 zLeG = 1;
                 zLeB = 1;
                 break;
            }
            case ZHDR_MODE_G1_R1_B0:
            {
                 zLeR = 1;
                 zLeG = 1;
                 zLeB = 0;
                 break;
            }
            case ZHDR_MODE_G1_R1_B1:
            {
                 zLeR = 1;
                 zLeG = 1;
                 zLeB = 1;
                 break;
            }
            default:
                if(m_bZHdrEnable)
                    CAM_LOGE("zvHDR Enable, but z-pattern is %d", mZPattern);
                break;
        }

        CAM_LOGD_IF(debugDump,"hdrOSC(%d), hdrTH(%d), hdrTurn(%d),  z-patn-RGB(%d,%d,%d), z-enable(%d), i-LeFirst(%d), i-Enable(%d)",
                 hdrOSC, hdrTH, hdrTurn, zLeR, zLeG, zLeB, zHdrEnable, mLeFirst, iHdrEnable);

        REG_INFO_VALUE(DIP_X_RMG2_HDR_CFG) = ((hdrOSC&0x0FFF) << 20)
                                               | ((hdrTH&0x0FFF) << 8)
                                               | ((zLeB&0x1) << 7)
                                               | ((zLeG&0x1) << 6)
                                               | ((zLeR&0x1) << 5)
                                               | ((zHdrEnable&0x1) << 4)
                                               | ((mLeFirst&0x1) << 1)
                                               | ((iHdrEnable&0x1))
                                               | 0;

        REG_INFO_VALUE(DIP_X_RMG2_HDR_CFG2) = (hdrTurn & 0x7FF) | 0;

        // HDR_GAIN
        MINT32 leInv = reinterpret_cast<ISP_CAM_RMG_HDR_GAIN_T*>(REG_INFO_VALUE_PTR(DIP_X_RMG2_HDR_GAIN))->RMG_LE_INV_CTL;
#if 0
        MINT32 gainVal = ((leInv&0x07) << 28)
                            | ((tuningTable[ratioIdx][IND_RATIO]&0x01FF) << 16)
                            | ((tuningTable[ratioIdx][IND_GAIN]&0x01FF))
                            | 0;
#endif
        MINT32 gainVal = ((leInv&0x07) << 28)
                            | ((RMG_RATIO&0x01FF) << 16)
                            | ((RMG_GAIN&0x01FF))
                            | 0;
#if 0
        CAM_LOGD_IF(debugDump,"leInv(%d), ratio(%d), gain(%d)",	leInv, tuningTable[ratioIdx][IND_RATIO], tuningTable[ratioIdx][IND_GAIN]);
#endif
        CAM_LOGD_IF(debugDump,"leInv(%d), ratio(%d), gain(%d)",	leInv, RMG_RATIO, RMG_GAIN);

        REG_INFO_VALUE(DIP_X_RMG2_HDR_GAIN) = gainVal;
    } // if bRMG_EN end

    // Register setting
    // Register setting
    writeRegs(static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum, pReg);



    dumpRegInfo("RMG2");

    return  MTRUE;
}


}

