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

#define MY_INST NS3Av3::INST_T<ISP_MGR_DM_T>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

namespace NSIspTuning
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_DM_T&
ISP_MGR_DM_T::
getInstance(MUINT32 const eSensorDev)
{
    ISP_MGR_MODULE_GET_INSTANCE(DM);
}

template <>
ISP_MGR_DM_T&
ISP_MGR_DM_T::
put(MUINT8 SubModuleIndex, ISP_NVRAM_DM_T const& rParam)
{
    PUT_REG_INFO_MULTI(SubModuleIndex, INTP_CRS,        intp_crs);
    PUT_REG_INFO_MULTI(SubModuleIndex, INTP_NAT,        intp_nat);
    PUT_REG_INFO_MULTI(SubModuleIndex, INTP_AUG,        intp_aug);
    PUT_REG_INFO_MULTI(SubModuleIndex, LUMA_LUT1,       luma_lut1);
    PUT_REG_INFO_MULTI(SubModuleIndex, LUMA_LUT2,       luma_lut2);
    PUT_REG_INFO_MULTI(SubModuleIndex, SL_CTL,          sl_ctl);
    PUT_REG_INFO_MULTI(SubModuleIndex, HFTD_CTL,        hftd_ctl);
    PUT_REG_INFO_MULTI(SubModuleIndex, NR_STR,          nr_str);
    PUT_REG_INFO_MULTI(SubModuleIndex, NR_ACT,          nr_act);
    PUT_REG_INFO_MULTI(SubModuleIndex, HF_STR,          hf_str);
    PUT_REG_INFO_MULTI(SubModuleIndex, HF_ACT1,         hf_act1);
    PUT_REG_INFO_MULTI(SubModuleIndex, HF_ACT2,         hf_act2);
    PUT_REG_INFO_MULTI(SubModuleIndex, CLIP,            clip);
    //PUT_REG_INFO_MULTI(SubModuleIndex, DSB,             dsb);
    //PUT_REG_INFO_MULTI(SubModuleIndex, TILE_EDGE,       tile_edge);
    PUT_REG_INFO_MULTI(SubModuleIndex, P1_ACT,          p1_act);
    PUT_REG_INFO_MULTI(SubModuleIndex, LR_RAT,          lr_rat);
    PUT_REG_INFO_MULTI(SubModuleIndex, HFTD_CTL2,       hftd_ctl2);
    PUT_REG_INFO_MULTI(SubModuleIndex, EST_CTL,         est_ctl);
    //PUT_REG_INFO_MULTI(SubModuleIndex, SPARE_2,         spare_2);
    //PUT_REG_INFO_MULTI(SubModuleIndex, SPARE_3,         spare_3);
    PUT_REG_INFO_MULTI(SubModuleIndex, INT_CTL,         int_ctl);
    PUT_REG_INFO_MULTI(SubModuleIndex, EE,              ee);
    PUT_REG_INFO_MULTI(SubModuleIndex, LMT,             lmt);
    PUT_REG_INFO_MULTI(SubModuleIndex, RCCC,            rccc);

    return  (*this);
}


template <>
ISP_MGR_DM_T&
ISP_MGR_DM_T::
get(MUINT8 SubModuleIndex, ISP_NVRAM_DM_T & rParam)
{
    GET_REG_INFO_MULTI(SubModuleIndex, INTP_CRS,        intp_crs);
    GET_REG_INFO_MULTI(SubModuleIndex, INTP_NAT,        intp_nat);
    GET_REG_INFO_MULTI(SubModuleIndex, INTP_AUG,        intp_aug);
    GET_REG_INFO_MULTI(SubModuleIndex, LUMA_LUT1,       luma_lut1);
    GET_REG_INFO_MULTI(SubModuleIndex, LUMA_LUT2,       luma_lut2);
    GET_REG_INFO_MULTI(SubModuleIndex, SL_CTL,          sl_ctl);
    GET_REG_INFO_MULTI(SubModuleIndex, HFTD_CTL,        hftd_ctl);
    GET_REG_INFO_MULTI(SubModuleIndex, NR_STR,          nr_str);
    GET_REG_INFO_MULTI(SubModuleIndex, NR_ACT,          nr_act);
    GET_REG_INFO_MULTI(SubModuleIndex, HF_STR,          hf_str);
    GET_REG_INFO_MULTI(SubModuleIndex, HF_ACT1,         hf_act1);
    GET_REG_INFO_MULTI(SubModuleIndex, HF_ACT2,         hf_act2);
    GET_REG_INFO_MULTI(SubModuleIndex, CLIP,            clip);
    //GET_REG_INFO_MULTI(SubModuleIndex, DSB,             dsb);
    //GET_REG_INFO_MULTI(SubModuleIndex, TILE_EDGE,       tile_edge);
    GET_REG_INFO_MULTI(SubModuleIndex, P1_ACT,          p1_act);
    GET_REG_INFO_MULTI(SubModuleIndex, LR_RAT,          lr_rat);
    GET_REG_INFO_MULTI(SubModuleIndex, HFTD_CTL2,       hftd_ctl2);
    GET_REG_INFO_MULTI(SubModuleIndex, EST_CTL,         est_ctl);
    //GET_REG_INFO_MULTI(SubModuleIndex, SPARE_2,         spare_2);
    //GET_REG_INFO_MULTI(SubModuleIndex, SPARE_3,         spare_3);
    GET_REG_INFO_MULTI(SubModuleIndex, INT_CTL,         int_ctl);
    GET_REG_INFO_MULTI(SubModuleIndex, EE,              ee);
    GET_REG_INFO_MULTI(SubModuleIndex, LMT,             lmt);
    GET_REG_INFO_MULTI(SubModuleIndex, RCCC,            rccc);

    return  (*this);
}

MBOOL
ISP_MGR_DM_T::
apply_P1(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, TuningMgr& rTuning, MINT32 i4SubsampleIdex)
{
    if(SubModuleIndex >= ESubModule_NUM){
        return MFALSE;
    }

    MBOOL bEnable = isEnable(SubModuleIndex);

    //Although bEnable could be MFALSE, Top CTRL DM is always MTRUE. We still have to do HW constraints
    if(!ISP_MGR_SLK_T::getInstance(m_eSensorDev).isEnable(ISP_MGR_SLK::ESLK_R1)){
        reinterpret_cast<REG_DM_R1_DM_SL_CTL*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, SL_CTL))->Bits.DM_SL_EN = 0;
    }

    if(!bEnable){
        reinterpret_cast<REG_DM_R1_DM_INTP_CRS*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, INTP_CRS))->Bits.DM_BYP = 1;

    }
    else{

        //DM_R1 only support Raw (not support Full G)
        reinterpret_cast<REG_DM_R1_DM_DSB*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, DSB))->Bits.DM_FL_MODE =  0;
        reinterpret_cast<REG_DM_R1_DM_DSB*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, DSB))->Bits.DM_SL_RAT  =  0;
        reinterpret_cast<REG_DM_R1_DM_DSB*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, DSB))->Bits.DM_SC_RAT  =  0;
    }

    //Top Control
    switch (SubModuleIndex)
    {
        case EDM_R1:
            ISP_MGR_CAMCTL_T::getInstance(m_eSensorDev).setEnable_DM_R1(bEnable);
            rTuning.updateEngine(eTuningMgrFunc_DM_R1, MTRUE, i4SubsampleIdex);
            break;
        default:
            CAM_LOGE("Apply Error Submodule Index: %d", SubModuleIndex);
            return  MFALSE;
    }

    AAA_TRACE_DRV(DRV_DM);
    rTuning.tuningMgrWriteRegs( (TUNING_MGR_REG_IO_STRUCT*)(&(m_rIspRegInfo[SubModuleIndex][0])),
        m_u4RegInfoNum, i4SubsampleIdex);
    AAA_TRACE_END_DRV;

    dumpRegInfoP1("DM", SubModuleIndex);

    return  MTRUE;
}


MBOOL
ISP_MGR_DM_T::
apply_P2(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, dip_x_reg_t* pReg)
{
    if(SubModuleIndex >= ESubModule_NUM){
        return MFALSE;
    }

    MBOOL bEnable = isEnable(SubModuleIndex);



    if(!bEnable){
        reinterpret_cast<DM_REG_D1A_DM_INTP_CRS*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, INTP_CRS))->Bits.DM_BYP = 1;

    }
    else{
        if(rRawIspCamInfo.fgRPGEnable){
            reinterpret_cast<DM_REG_D1A_DM_DSB*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, DSB))->Bits.DM_FL_MODE =  1;
            reinterpret_cast<DM_REG_D1A_DM_DSB*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, DSB))->Bits.DM_SL_RAT  = 16;
            reinterpret_cast<DM_REG_D1A_DM_DSB*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, DSB))->Bits.DM_SC_RAT  = 16;
        }
        else{
            reinterpret_cast<DM_REG_D1A_DM_DSB*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, DSB))->Bits.DM_FL_MODE =  0;
            reinterpret_cast<DM_REG_D1A_DM_DSB*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, DSB))->Bits.DM_SL_RAT  =  0;
            reinterpret_cast<DM_REG_D1A_DM_DSB*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, DSB))->Bits.DM_SC_RAT  =  0;
        }

        reinterpret_cast<DM_REG_D1A_DM_INTP_CRS*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, INTP_CRS))->Bits.DM_MN_MODE = m_bMono;
    }


    //Top Control
    switch (SubModuleIndex)
    {
        case EDM_D1:
            ISP_WRITE_ENABLE_BITS(pReg, DIPCTL_D1A_DIPCTL_RGB_EN1, DIPCTL_DM_D1_EN, MTRUE);
            break;
        default:
            CAM_LOGE("Apply Error Submodule Index: %d", SubModuleIndex);
            return  MFALSE;
    }

    writeRegs(static_cast<RegInfo_T*>(m_rIspRegInfo[SubModuleIndex]), m_u4RegInfoNum, pReg);

    dumpRegInfoP2("DM", SubModuleIndex);

    return  MTRUE;
}

}


