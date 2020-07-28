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
#define LOG_TAG "isp_mgr_wb"

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

using namespace NSIspTuning;

#define MY_INST NS3Av3::INST_T<ISP_MGR_WB_T>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;
namespace NSIspTuning
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  WB
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_WB_T&
ISP_MGR_WB_T::
getInstance(MUINT32 const eSensorDev)
{
    ISP_MGR_MODULE_GET_INSTANCE(WB);
}

#if 0
template <>
ISP_MGR_WB_T&
ISP_MGR_WB_T::
put(MUINT8 SubModuleIndex, ISP_NVRAM_WB_T const& rParam)
{
    PUT_REG_INFO_MULTI(SubModuleIndex, GAIN_1,  gain_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, GAIN_2,  gain_2);
    //PUT_REG_INFO_MULTI(SubModuleIndex, HLC,     hlc);

    return  (*this);
}

template <>
ISP_MGR_WB_T&
ISP_MGR_WB_T::
get(MUINT8 SubModuleIndex, ISP_NVRAM_WB_T & rParam)
{
    GET_REG_INFO_MULTI(SubModuleIndex, GAIN_1,  gain_1);
    GET_REG_INFO_MULTI(SubModuleIndex, GAIN_2,  gain_2);
    //GET_REG_INFO_MULTI(SubModuleIndex, HLC,     hlc);

    return  (*this);
}

#endif

MBOOL
ISP_MGR_WB_T::
apply_P1(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, TuningMgr& rTuning, MINT32 i4SubsampleIdex)
{
    if(SubModuleIndex >= ESubModule_NUM){
        return MFALSE;
    }

    MBOOL bEnable = isEnable(SubModuleIndex);

    //Top Control
    switch (SubModuleIndex)
    {
        case EWB_R1:
            ISP_MGR_CAMCTL_T::getInstance(m_eSensorDev).setEnable_WB_R1(bEnable);
            rTuning.updateEngine(eTuningMgrFunc_WB_R1, bEnable, i4SubsampleIdex);
            break;
        default:
            CAM_LOGE("Apply Error Submodule Index: %d", SubModuleIndex);
            return  MFALSE;
    }

    if(bEnable){
        CAM_LOGD_IF(m_bDebugEnable, "[%s] AWB P1 Gain R(%d), G(%d), B(%d)", __FUNCTION__, rRawIspCamInfo.rAWBInfo.rRPG.i4R, rRawIspCamInfo.rAWBInfo.rRPG.i4G, rRawIspCamInfo.rAWBInfo.rRPG.i4B);

        reinterpret_cast<REG_WB_R1_WB_GAIN_1*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, GAIN_1))->Bits.WB_GAIN_B  = rRawIspCamInfo.rAWBInfo.rRPG.i4B;
        reinterpret_cast<REG_WB_R1_WB_GAIN_1*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, GAIN_1))->Bits.WB_GAIN_GB = rRawIspCamInfo.rAWBInfo.rRPG.i4G;
        reinterpret_cast<REG_WB_R1_WB_GAIN_2*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, GAIN_2))->Bits.WB_GAIN_GR = rRawIspCamInfo.rAWBInfo.rRPG.i4G;
        reinterpret_cast<REG_WB_R1_WB_GAIN_2*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, GAIN_2))->Bits.WB_GAIN_R  = rRawIspCamInfo.rAWBInfo.rRPG.i4R;
    }

    AAA_TRACE_DRV(DRV_WB);
    rTuning.tuningMgrWriteRegs( (TUNING_MGR_REG_IO_STRUCT*)(&(m_rIspRegInfo[SubModuleIndex][0])),
        m_u4RegInfoNum, i4SubsampleIdex);
    AAA_TRACE_END_DRV;

    dumpRegInfoP1("WB", SubModuleIndex);

    return  MTRUE;
}


MBOOL
ISP_MGR_WB_T::
apply_P2(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, dip_x_reg_t* pReg)
{
    if(SubModuleIndex >= ESubModule_NUM){
        return MFALSE;
    }

    MBOOL bEnable = isEnable(SubModuleIndex);

    //Top Control
    switch (SubModuleIndex)
    {
        case EWB_D1:
            ISP_WRITE_ENABLE_BITS(pReg, DIPCTL_D1A_DIPCTL_RGB_EN1, DIPCTL_WB_D1_EN, bEnable);
            break;
        default:
            CAM_LOGE("Apply Error Submodule Index: %d", SubModuleIndex);
            return  MFALSE;
    }

    if(bEnable){
        CAM_LOGD_IF(m_bDebugEnable, "[%s] AWB P2 Gain R(%d), G(%d), B(%d)", __FUNCTION__, rRawIspCamInfo.rAWBInfo.rPGN.i4R, rRawIspCamInfo.rAWBInfo.rPGN.i4G, rRawIspCamInfo.rAWBInfo.rPGN.i4B);

        reinterpret_cast<WB_REG_D1A_WB_GAIN_1*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, GAIN_1))->Bits.WB_GAIN_B  = rRawIspCamInfo.rAWBInfo.rPGN.i4B;
        reinterpret_cast<WB_REG_D1A_WB_GAIN_1*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, GAIN_1))->Bits.WB_GAIN_GB = rRawIspCamInfo.rAWBInfo.rPGN.i4G;
        reinterpret_cast<WB_REG_D1A_WB_GAIN_2*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, GAIN_2))->Bits.WB_GAIN_GR = rRawIspCamInfo.rAWBInfo.rPGN.i4G;
        reinterpret_cast<WB_REG_D1A_WB_GAIN_2*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, GAIN_2))->Bits.WB_GAIN_R  = rRawIspCamInfo.rAWBInfo.rPGN.i4R;

        writeRegs(static_cast<RegInfo_T*>(m_rIspRegInfo[SubModuleIndex]), m_u4RegInfoNum, pReg);

        dumpRegInfoP2("WB", SubModuleIndex);
    }
    return  MTRUE;
}





}

