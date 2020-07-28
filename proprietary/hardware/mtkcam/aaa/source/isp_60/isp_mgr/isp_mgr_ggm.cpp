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
#define LOG_TAG "isp_mgr_ggm"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (0)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include "isp_mgr.h"

#include <private/aaa_utils.h>
#include <array>

using namespace NSIspTuning;

#define MY_INST NS3Av3::INST_T<ISP_MGR_GGM_T>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

namespace NSIspTuning
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// GGM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_GGM_T&
ISP_MGR_GGM_T::
getInstance(MUINT32 const eSensorDev)
{
    ISP_MGR_MODULE_GET_INSTANCE(GGM);
}

template <>
ISP_MGR_GGM_T&
ISP_MGR_GGM_T::
put(MUINT8 SubModuleIndex, ISP_NVRAM_GGM_T const& rParam)
{
    for (MINT32 i = 0; i < GGM_LUT_SIZE; i++) {
        m_rIspRegInfo[SubModuleIndex][ERegInfo_NUM + i].val = rParam.set[i];
    }

    return  (*this);
}

template <>
ISP_MGR_GGM_T&
ISP_MGR_GGM_T::
get(MUINT8 SubModuleIndex, ISP_NVRAM_GGM_T & rParam)
{
    for (MINT32 i = 0; i < GGM_LUT_SIZE; i++) {
        rParam.set[i] = m_rIspRegInfo[SubModuleIndex][ERegInfo_NUM + i].val;
    }

    return  (*this);
}

MBOOL
ISP_MGR_GGM_T::
apply_P1(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, TuningMgr& rTuning, MINT32 i4SubsampleIdex)
{
    if(SubModuleIndex >= ESubModule_NUM){
        return MFALSE;
    }

    //Top Control
    switch (SubModuleIndex)
    {
        case EGGM_R1:
            ISP_MGR_CAMCTL_T::getInstance(m_eSensorDev).setEnable_GGM_R1(MTRUE);
            rTuning.updateEngine(eTuningMgrFunc_GGM_R1, MTRUE, i4SubsampleIdex);
            break;
        case EGGM_R2:
            ISP_MGR_CAMCTL_T::getInstance(m_eSensorDev).setEnable_GGM_R2(MTRUE);
            rTuning.updateEngine(eTuningMgrFunc_GGM_R2, MTRUE, i4SubsampleIdex);
            break;
        default:
            CAM_LOGE("Apply Error Submodule Index: %d", SubModuleIndex);
            return  MFALSE;
    }

    MBOOL bEnable = isEnable(SubModuleIndex);

    reinterpret_cast<REG_GGM_R1_GGM_CTRL*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, CTRL))->Bits.GGM_LNR = (!bEnable);

    AAA_TRACE_DRV(DRV_GGM);
    rTuning.tuningMgrWriteRegs( (TUNING_MGR_REG_IO_STRUCT*)(&(m_rIspRegInfo[SubModuleIndex][0])),
            m_u4RegInfoNum, i4SubsampleIdex);
    AAA_TRACE_END_DRV;


    dumpRegInfoP1("GGM", SubModuleIndex);

    return  MTRUE;
}

MBOOL
ISP_MGR_GGM_T::
apply_P2(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, dip_x_reg_t* pReg)
{
    if(SubModuleIndex >= ESubModule_NUM){
        return MFALSE;
    }

    //Top Control
    switch (SubModuleIndex)
    {
        case EGGM_D1:
            ISP_WRITE_ENABLE_BITS(pReg, DIPCTL_D1A_DIPCTL_RGB_EN2, DIPCTL_GGM_D1_EN, MTRUE);
            break;
        case EGGM_D2:
            ISP_WRITE_ENABLE_BITS(pReg, DIPCTL_D1A_DIPCTL_RGB_EN2, DIPCTL_GGM_D2_EN, MTRUE);
            break;
        case EGGM_D3:
            ISP_WRITE_ENABLE_BITS(pReg, DIPCTL_D1A_DIPCTL_YUV_EN1, DIPCTL_GGM_D3_EN, MTRUE);
            break;
        default:
            CAM_LOGE("Apply Error Submodule Index: %d", SubModuleIndex);
            return  MFALSE;
    }

    MBOOL bEnable = isEnable(SubModuleIndex);

    reinterpret_cast<GGM_REG_D1A_GGM_CTRL*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, CTRL))->Bits.GGM_LNR = (!bEnable);

    writeRegs(static_cast<RegInfo_T*>(m_rIspRegInfo[SubModuleIndex]), m_u4RegInfoNum, pReg);

    dumpRegInfoP2("GGM", SubModuleIndex);

    return  MTRUE;
}




}
