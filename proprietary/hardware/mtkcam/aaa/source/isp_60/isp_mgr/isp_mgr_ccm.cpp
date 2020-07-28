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
#define LOG_TAG "isp_mgr_ccm"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include "isp_mgr.h"
//#include <ccm_mgr.h>
#include <array>
#include <private/aaa_utils.h>

using namespace NSIspTuning;

#define MY_INST NS3Av3::INST_T<ISP_MGR_CCM_T>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

//#define MY_INST_CCM2 NS3Av3::INST_T<ISP_MGR_CCM2_T>
//static std::array<MY_INST_CCM2, SENSOR_IDX_MAX> gMultitonCCM2;

namespace NSIspTuning
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CCM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_CCM_T&
ISP_MGR_CCM_T::
getInstance(MUINT32 const eSensorDev)
{
    ISP_MGR_MODULE_GET_INSTANCE(CCM);
}

template <>
ISP_MGR_CCM_T&
ISP_MGR_CCM_T::
put(MUINT8 SubModuleIndex, ISP_NVRAM_CCM_T const& rParam)
{
    PUT_REG_INFO_MULTI(SubModuleIndex, CNV_1,           cnv_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, CNV_2,           cnv_2);
    PUT_REG_INFO_MULTI(SubModuleIndex, CNV_3,           cnv_3);
    PUT_REG_INFO_MULTI(SubModuleIndex, CNV_4,           cnv_4);
    PUT_REG_INFO_MULTI(SubModuleIndex, CNV_5,           cnv_5);
    PUT_REG_INFO_MULTI(SubModuleIndex, CNV_6,           cnv_6);

    return  (*this);
}


template <>
ISP_MGR_CCM_T&
ISP_MGR_CCM_T::
get(MUINT8 SubModuleIndex, ISP_NVRAM_CCM_T & rParam)
{
    GET_REG_INFO_MULTI(SubModuleIndex, CNV_1,           cnv_1);
    GET_REG_INFO_MULTI(SubModuleIndex, CNV_2,           cnv_2);
    GET_REG_INFO_MULTI(SubModuleIndex, CNV_3,           cnv_3);
    GET_REG_INFO_MULTI(SubModuleIndex, CNV_4,           cnv_4);
    GET_REG_INFO_MULTI(SubModuleIndex, CNV_5,           cnv_5);
    GET_REG_INFO_MULTI(SubModuleIndex, CNV_6,           cnv_6);

    return  (*this);
}

MBOOL
ISP_MGR_CCM_T::
apply_P1(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, TuningMgr& rTuning, MINT32 i4SubsampleIdex)
{
    if(SubModuleIndex >= ESubModule_NUM){
        return MFALSE;
    }

    MBOOL bEnable = isEnable(SubModuleIndex);

    reinterpret_cast<REG_CCM_R1_CCM_CTRL*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, CTRL))->Bits.CCM_ACC = 9;

    //Top Control
    switch (SubModuleIndex)
    {
        case ECCM_R1:
            ISP_MGR_CAMCTL_T::getInstance(m_eSensorDev).setEnable_CCM_R1(bEnable);
            rTuning.updateEngine(eTuningMgrFunc_CCM_R1, bEnable, i4SubsampleIdex);
            break;
        default:
            CAM_LOGE("Apply Error Submodule Index: %d", SubModuleIndex);
            return  MFALSE;
    }

    AAA_TRACE_DRV(DRV_CCM);
    rTuning.tuningMgrWriteRegs( (TUNING_MGR_REG_IO_STRUCT*)(&(m_rIspRegInfo[SubModuleIndex][0])),
        m_u4RegInfoNum, i4SubsampleIdex);
    AAA_TRACE_END_DRV;

    dumpRegInfoP1("CCM", SubModuleIndex);

    return  MTRUE;
}


MBOOL
ISP_MGR_CCM_T::
apply_P2(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, dip_x_reg_t* pReg)
{
    if(SubModuleIndex >= ESubModule_NUM){
        return MFALSE;
    }

    MBOOL bEnable = isEnable(SubModuleIndex);

    reinterpret_cast<CCM_REG_D1A_CCM_CTRL*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, CTRL))->Bits.CCM_ACC = 9;

    //Top Control
    switch (SubModuleIndex)
    {
        case ECCM_D1:
            ISP_WRITE_ENABLE_BITS(pReg, DIPCTL_D1A_DIPCTL_RGB_EN2, DIPCTL_CCM_D1_EN, bEnable);
            break;
        case ECCM_D2:
            ISP_WRITE_ENABLE_BITS(pReg, DIPCTL_D1A_DIPCTL_RGB_EN2, DIPCTL_CCM_D2_EN, bEnable);
            if(rRawIspCamInfo.rMapping_Info.eIspProfile == EIspProfile_N3D_Capture_Depth_toW  ||
               rRawIspCamInfo.rMapping_Info.eIspProfile == EIspProfile_N3D_Preview_toW ||
               rRawIspCamInfo.rMapping_Info.eIspProfile == EIspProfile_N3D_Preview)
            {
                reinterpret_cast<CCM_REG_D1A_CCM_CTRL*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, CTRL))->Bits.CCM_ACC = 11;
            }
            break;
        case ECCM_D3:
            ISP_WRITE_ENABLE_BITS(pReg, DIPCTL_D1A_DIPCTL_YUV_EN1, DIPCTL_CCM_D3_EN, bEnable);
            break;
        default:
            CAM_LOGE("Apply Error Submodule Index: %d", SubModuleIndex);
            return  MFALSE;
    }

    writeRegs(static_cast<RegInfo_T*>(m_rIspRegInfo[SubModuleIndex]), m_u4RegInfoNum, pReg);

    dumpRegInfoP2("CCM", SubModuleIndex);

    return  MTRUE;
}


}
