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
#include <mtkcam/utils/std/Log.h>
#include "isp_mgr.h"
#include <private/aaa_utils.h>
#include <array>

using namespace NSIspTuning;

#define MY_INST NS3Av3::INST_T<ISP_MGR_LCES_T>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

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

namespace NSIspTuning
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  LCES
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_LCES_T&
ISP_MGR_LCES_T::
getInstance(MUINT32 const eSensorDev)
{
    ISP_MGR_MODULE_GET_INSTANCE(LCES);
}

template <>
ISP_MGR_LCES_T&
ISP_MGR_LCES_T::
put(MUINT8 SubModuleIndex, ISP_NVRAM_FLC_T const& rParam)
{
    reinterpret_cast<REG_LCES_R1_LCES_FLC0*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, FLC0))->Bits.LCES_FLC_OFST_R
        = rParam.ofst_rb.bits.FLC_OFST_R;
    reinterpret_cast<REG_LCES_R1_LCES_FLC0*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, FLC0))->Bits.LCES_FLC_OFST_G
        = rParam.ofst_g.bits.FLC_OFST_G;
    reinterpret_cast<REG_LCES_R1_LCES_FLC1*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, FLC1))->Bits.LCES_FLC_OFST_B
        = rParam.ofst_rb.bits.FLC_OFST_B;
    reinterpret_cast<REG_LCES_R1_LCES_FLC1*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, FLC1))->Bits.LCES_FLC_GAIN_R
        = rParam.gn_rb.bits.FLC_GAIN_R;
    reinterpret_cast<REG_LCES_R1_LCES_FLC2*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, FLC2))->Bits.LCES_FLC_GAIN_G
        = rParam.gn_g.bits.FLC_GAIN_G;
    reinterpret_cast<REG_LCES_R1_LCES_FLC2*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, FLC2))->Bits.LCES_FLC_GAIN_B
        = rParam.gn_rb.bits.FLC_GAIN_B;

    return  (*this);
}

template <>
ISP_MGR_LCES_T&
ISP_MGR_LCES_T::
put(MUINT8 SubModuleIndex, ISP_NVRAM_CCM_T const& rParam)
{
    reinterpret_cast<REG_LCES_R1_LCES_G2G0*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, G2G0))->Bits.LCES_G2G_CNV_00
        = rParam.cnv_1.bits.CCM_CNV_00;
    reinterpret_cast<REG_LCES_R1_LCES_G2G0*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, G2G0))->Bits.LCES_G2G_CNV_01
        = rParam.cnv_1.bits.CCM_CNV_01;
    reinterpret_cast<REG_LCES_R1_LCES_G2G1*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, G2G1))->Bits.LCES_G2G_CNV_02
        = rParam.cnv_2.bits.CCM_CNV_02;
    reinterpret_cast<REG_LCES_R1_LCES_G2G1*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, G2G1))->Bits.LCES_G2G_CNV_10
        = rParam.cnv_3.bits.CCM_CNV_10;
    reinterpret_cast<REG_LCES_R1_LCES_G2G2*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, G2G2))->Bits.LCES_G2G_CNV_11
        = rParam.cnv_3.bits.CCM_CNV_11;
    reinterpret_cast<REG_LCES_R1_LCES_G2G2*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, G2G2))->Bits.LCES_G2G_CNV_12
        = rParam.cnv_4.bits.CCM_CNV_12;
    reinterpret_cast<REG_LCES_R1_LCES_G2G3*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, G2G3))->Bits.LCES_G2G_CNV_20
        = rParam.cnv_5.bits.CCM_CNV_20;
    reinterpret_cast<REG_LCES_R1_LCES_G2G3*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, G2G3))->Bits.LCES_G2G_CNV_21
        = rParam.cnv_5.bits.CCM_CNV_21;
    reinterpret_cast<REG_LCES_R1_LCES_G2G4*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, G2G4))->Bits.LCES_G2G_CNV_22
        = rParam.cnv_6.bits.CCM_CNV_22;

    return  (*this);
}


MBOOL
ISP_MGR_LCES_T::
apply_P1(MUINT8 SubModuleIndex, RAWIspCamInfo& rRawIspCamInfo, TuningMgr& rTuning, MINT32 i4SubsampleIdex)
{
    if(SubModuleIndex >= ESubModule_NUM){
        return MFALSE;
    }

    MBOOL bEnable = isEnable(SubModuleIndex);

    if(bEnable){
        reinterpret_cast<REG_LCES_R1_LCES_G2G4*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, G2G4))->Bits.LCES_G2G_ACC = 9;
    }

    //Top Control
    switch (SubModuleIndex)
    {
        case ELCES_R1:
            ISP_MGR_CAMCTL_T::getInstance(m_eSensorDev).setEnable_LCES_R1(bEnable);
            rTuning.updateEngine(eTuningMgrFunc_LCES_R1, bEnable, i4SubsampleIdex);
            break;
        default:
            CAM_LOGE("Apply Error Submodule Index: %d", SubModuleIndex);
            return  MFALSE;
    }

    AAA_TRACE_DRV(DRV_LCES);
    rTuning.tuningMgrWriteRegs( (TUNING_MGR_REG_IO_STRUCT*)(&(m_rIspRegInfo[SubModuleIndex][0])),
        m_u4RegInfoNum, i4SubsampleIdex);
    AAA_TRACE_END_DRV;

    dumpRegInfoP1("LCES", SubModuleIndex);

    return  MTRUE;
}



}
