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
#define LOG_TAG "isp_mgr_hlr"

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

#define MY_INST NS3Av3::INST_T<ISP_MGR_HLR_T>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

namespace NSIspTuning
{
#define CLAMP(x,min,max)       (((x) > (max)) ? (max) : (((x) < (min)) ? (min) : (x)))

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  HLR
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_HLR_T&
ISP_MGR_HLR_T::
getInstance(MUINT32 const eSensorDev)
{
    ISP_MGR_MODULE_GET_INSTANCE(HLR);
}

template <>
ISP_MGR_HLR_T&
ISP_MGR_HLR_T::
put(MUINT8 SubModuleIndex, ISP_NVRAM_HLR_T const& rParam)
{
    PUT_REG_INFO_MULTI(SubModuleIndex, EST_Y0,      est_y0);
    PUT_REG_INFO_MULTI(SubModuleIndex, EST_Y1,      est_y1);
    PUT_REG_INFO_MULTI(SubModuleIndex, EST_Y2,      est_y2);
    PUT_REG_INFO_MULTI(SubModuleIndex, EST_Y3,      est_y3);
    PUT_REG_INFO_MULTI(SubModuleIndex, EST_X0,      est_x0);
    PUT_REG_INFO_MULTI(SubModuleIndex, EST_X1,      est_x1);
    PUT_REG_INFO_MULTI(SubModuleIndex, EST_X2,      est_x2);
    PUT_REG_INFO_MULTI(SubModuleIndex, EST_X3,      est_x3);
    PUT_REG_INFO_MULTI(SubModuleIndex, EST_S0,      est_s0);
    PUT_REG_INFO_MULTI(SubModuleIndex, EST_S1,      est_s1);
    PUT_REG_INFO_MULTI(SubModuleIndex, EST_S2,      est_s2);
    PUT_REG_INFO_MULTI(SubModuleIndex, LMG,         lmg);
    PUT_REG_INFO_MULTI(SubModuleIndex, PRT,         prt);
    PUT_REG_INFO_MULTI(SubModuleIndex, CLP,         clp);
    PUT_REG_INFO_MULTI(SubModuleIndex, EFCT,        efct);
    PUT_REG_INFO_MULTI(SubModuleIndex, CTL,         ctl);
    PUT_REG_INFO_MULTI(SubModuleIndex, CTL2,        ctl2);

    switch (SubModuleIndex)
    {
        case EHLR_R1:
            PUT_REG_INFO_MULTI(SubModuleIndex, LKMSB, lkmsb);
            break;
        case EHLR_D1:
            reinterpret_cast<DIPCTL_REG_D1A_DIPCTL_MISC_SEL*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, LKMSB))->Bits.DIPCTL_HLR_D1_LKMSB =
                rParam.lkmsb.bits.CAMCTL_HLR_R1_LKMSB;
            break;
        default:
            CAM_LOGE("Apply Error Submodule Index: %d", SubModuleIndex);
            break;
    }

    return  (*this);
}


template <>
ISP_MGR_HLR_T&
ISP_MGR_HLR_T::
get(MUINT8 SubModuleIndex, ISP_NVRAM_HLR_T & rParam)
{
    GET_REG_INFO_MULTI(SubModuleIndex, EST_Y0,      est_y0);
    GET_REG_INFO_MULTI(SubModuleIndex, EST_Y1,      est_y1);
    GET_REG_INFO_MULTI(SubModuleIndex, EST_Y2,      est_y2);
    GET_REG_INFO_MULTI(SubModuleIndex, EST_Y3,      est_y3);
    GET_REG_INFO_MULTI(SubModuleIndex, EST_X0,      est_x0);
    GET_REG_INFO_MULTI(SubModuleIndex, EST_X1,      est_x1);
    GET_REG_INFO_MULTI(SubModuleIndex, EST_X2,      est_x2);
    GET_REG_INFO_MULTI(SubModuleIndex, EST_X3,      est_x3);
    GET_REG_INFO_MULTI(SubModuleIndex, EST_S0,      est_s0);
    GET_REG_INFO_MULTI(SubModuleIndex, EST_S1,      est_s1);
    GET_REG_INFO_MULTI(SubModuleIndex, EST_S2,      est_s2);
    GET_REG_INFO_MULTI(SubModuleIndex, LMG,         lmg);
    GET_REG_INFO_MULTI(SubModuleIndex, PRT,         prt);
    GET_REG_INFO_MULTI(SubModuleIndex, CLP,         clp);
    GET_REG_INFO_MULTI(SubModuleIndex, EFCT,        efct);
    GET_REG_INFO_MULTI(SubModuleIndex, CTL,         ctl);
    GET_REG_INFO_MULTI(SubModuleIndex, CTL2,        ctl2);

    switch (SubModuleIndex)
    {
        case EHLR_R1:
            GET_REG_INFO_MULTI(SubModuleIndex, LKMSB, lkmsb);
            break;
        case EHLR_D1:
            rParam.lkmsb.bits.CAMCTL_HLR_R1_LKMSB =
                reinterpret_cast<DIPCTL_REG_D1A_DIPCTL_MISC_SEL*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, LKMSB))->Bits.DIPCTL_HLR_D1_LKMSB;
            break;
        default:
            CAM_LOGE("Apply Error Submodule Index: %d", SubModuleIndex);
            break;
    }

    return  (*this);
}

MBOOL
ISP_MGR_HLR_T::
apply_P1(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, TuningMgr& rTuning, MINT32 i4SubsampleIdex)
{
    if(SubModuleIndex >= ESubModule_NUM){
        return MFALSE;
    }

    MBOOL bEnable = isEnable(SubModuleIndex);

    //Top Control
    switch (SubModuleIndex)
    {
        case EHLR_R1:
            ISP_MGR_CAMCTL_T::getInstance(m_eSensorDev).setEnable_HLR_R1(bEnable);
            // Top Control
            rTuning.enableEngine(eTuningMgrFunc_HLR_R1, ( bEnable &&(rRawIspCamInfo.u4Id == 1) ), i4SubsampleIdex);
            // Update
            rTuning.tuningEngine(eTuningMgrFunc_HLR_R1,( bEnable &&(rRawIspCamInfo.u4Id == 1) ), i4SubsampleIdex);
            break;
        default:
            CAM_LOGE("Apply Error Submodule Index: %d", SubModuleIndex);
            return  MFALSE;
    }

    if( bEnable &&(rRawIspCamInfo.u4Id == 1) ){

        if(rRawIspCamInfo.rCCU_Result.LTM.LTM_Valid == CCU_UNVALID)
        {
            //default value for HLR
            ISP_NVRAM_HLR_T defaultHLR = {0};

            defaultHLR.est_y0.bits.HLR_SAT_0 = 0;
            defaultHLR.est_y0.bits.HLR_SAT_1 = 1024;
            defaultHLR.est_y1.bits.HLR_SAT_2 = 2304;
            defaultHLR.est_y1.bits.HLR_SAT_3 = 3136;
            defaultHLR.est_y2.bits.HLR_SAT_4 = 3600;
            defaultHLR.est_y2.bits.HLR_SAT_5 = 3844;
            defaultHLR.est_y3.bits.HLR_SAT_6 = 3969;
            defaultHLR.est_y3.bits.HLR_SAT_7 = 4096;

            defaultHLR.est_x0.bits.HLR_NODE_0 = 0;
            defaultHLR.est_x0.bits.HLR_NODE_1 = 2048;
            defaultHLR.est_x1.bits.HLR_NODE_2 = 3072;
            defaultHLR.est_x1.bits.HLR_NODE_3 = 3584;
            defaultHLR.est_x2.bits.HLR_NODE_4 = 3840;
            defaultHLR.est_x2.bits.HLR_NODE_5 = 3968;
            defaultHLR.est_x3.bits.HLR_NODE_6 = 4032;
            defaultHLR.est_x3.bits.HLR_NODE_7 = 4096;

            // SWO

            defaultHLR.est_s0.bits.HLR_SLP_0 = 32;
            defaultHLR.est_s0.bits.HLR_SLP_1 = 80;
            defaultHLR.est_s0.bits.HLR_SLP_2 = 104;
            defaultHLR.est_s1.bits.HLR_SLP_3 = 116;
            defaultHLR.est_s1.bits.HLR_SLP_4 = 122;
            defaultHLR.est_s1.bits.HLR_SLP_5 = 125;
            defaultHLR.est_s2.bits.HLR_SLP_6 = 127;

            defaultHLR.est_s2.bits.HLR_MAX_RAT = 4;
            defaultHLR.est_s2.bits.HLR_BLUE_PRT_STR = 16;
            defaultHLR.est_s2.bits.HLR_RED_PRT_STR = 16;

            defaultHLR.lmg.bits.HLR_BLD_FG  = 18;
            defaultHLR.lmg.bits.HLR_BLD_HIGH= 3328;
            defaultHLR.lmg.bits.HLR_BLD_SLP = 256;

            defaultHLR.prt.bits.HLR_PRT_TH  = 1024;
            defaultHLR.prt.bits.HLR_PRT_SLP = 256;
            defaultHLR.prt.bits.HLR_PRT_EN  = 1;

            defaultHLR.clp.bits.HLR_CLP_VAL = 524287;

            defaultHLR.efct.bits.HLR_OFF_CLP_VAL = 524287;
            defaultHLR.efct.bits.HLR_EFCT_ON = 1;

            defaultHLR.ctl.bits.HLR_TILE_EDGE = 15;

            defaultHLR.ctl2.bits.HLR_TDR_WD = 65535;
            defaultHLR.ctl2.bits.HLR_TDR_HT = 65535;

            defaultHLR.lkmsb.bits.CAMCTL_HLR_R1_LKMSB = 11;

            put(SubModuleIndex, defaultHLR);
        }
        else
        {
            put(SubModuleIndex, rRawIspCamInfo.rCCU_Result.HLR.HLR_Data);
        }

        AAA_TRACE_DRV(DRV_HLR);
        rTuning.tuningMgrWriteRegs( (TUNING_MGR_REG_IO_STRUCT*)(&(m_rIspRegInfo[SubModuleIndex][0])),
            m_u4RegInfoNum, i4SubsampleIdex);
        AAA_TRACE_END_DRV;

        dumpRegInfoP1("HLR", SubModuleIndex);

    }

    return  MTRUE;

}

MBOOL
ISP_MGR_HLR_T::
apply_P2(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, dip_x_reg_t* pReg)
{
     if(SubModuleIndex >= ESubModule_NUM){
        return MFALSE;
    }

    MBOOL bEnable = isEnable(SubModuleIndex);

    //Top Control
    switch (SubModuleIndex)
    {
        case EHLR_D1:
            ISP_WRITE_ENABLE_BITS(pReg, DIPCTL_D1A_DIPCTL_RGB_EN1, DIPCTL_HLR_D1_EN, bEnable);
            break;
        default:
            CAM_LOGE("Apply Error Submodule Index: %d", SubModuleIndex);
            return  MFALSE;
    }


    if(bEnable){

        put(SubModuleIndex, rRawIspCamInfo.rCCU_Result.HLR.HLR_Data);

        writeRegs(static_cast<RegInfo_T*>(m_rIspRegInfo[SubModuleIndex]), m_u4RegInfoNum, pReg);

        dumpRegInfoP2("HLR", SubModuleIndex);
    }

    return  MTRUE;
}





}

