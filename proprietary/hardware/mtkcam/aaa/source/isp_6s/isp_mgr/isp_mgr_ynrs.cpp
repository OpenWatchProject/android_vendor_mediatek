/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "isp_mgr_ynrs"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include "property_utils.h"
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include "isp_mgr.h"
#include <string>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/imgbuf/IIonImageBufferHeap.h>


namespace NSIspTuning
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  YNRS
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_YNRS_T&
ISP_MGR_YNRS_T::
getInstance(ESensorDev_T const eSensorDev)
{
    ISP_MGR_MODULE_GET_INSTANCE(YNRS);
}

template <>
ISP_MGR_YNRS_T&
ISP_MGR_YNRS_T::
put(MUINT8 SubModuleIndex, ISP_NVRAM_YNR_LUT_T const& rParam)
{
    for (MINT32 i = 0; i < ANR_HW_TBL_NUM; i++) {
        m_rIspRegInfo[SubModuleIndex][ERegInfo_NUM+i].val = rParam.set[i];
    }
    return  (*this);
}

template <>
ISP_MGR_YNRS_T&
ISP_MGR_YNRS_T::
get(MUINT8 SubModuleIndex, ISP_NVRAM_YNR_LUT_T & rParam)
{
    for (MINT32 i = 0; i < ANR_HW_TBL_NUM; i++) {
        rParam.set[i] = m_rIspRegInfo[SubModuleIndex][ERegInfo_NUM+i].val;
    }

    return  (*this);
}

template <>
ISP_MGR_YNRS_T&
ISP_MGR_YNRS_T::
put(MUINT8 SubModuleIndex, ISP_NVRAM_YNRS_T const& rParam)
{
    PUT_REG_INFO_MULTI(SubModuleIndex, CON1,            con1);
    PUT_REG_INFO_MULTI(SubModuleIndex, CON2,            con2);
    PUT_REG_INFO_MULTI(SubModuleIndex, YAD2,            yad2);
    PUT_REG_INFO_MULTI(SubModuleIndex, Y4LUT1,          y4lut1);
    PUT_REG_INFO_MULTI(SubModuleIndex, Y4LUT2,          y4lut2);
    PUT_REG_INFO_MULTI(SubModuleIndex, Y4LUT3,          y4lut3);
    PUT_REG_INFO_MULTI(SubModuleIndex, C4LUT1,          c4lut1);
    PUT_REG_INFO_MULTI(SubModuleIndex, C4LUT2,          c4lut2);
    PUT_REG_INFO_MULTI(SubModuleIndex, C4LUT3,          c4lut3);
    PUT_REG_INFO_MULTI(SubModuleIndex, A4LUT2,          a4lut2);
    PUT_REG_INFO_MULTI(SubModuleIndex, A4LUT3,          a4lut3);
    PUT_REG_INFO_MULTI(SubModuleIndex, L4LUT1,          l4lut1);
    PUT_REG_INFO_MULTI(SubModuleIndex, L4LUT2,          l4lut2);
    PUT_REG_INFO_MULTI(SubModuleIndex, L4LUT3,          l4lut3);
    PUT_REG_INFO_MULTI(SubModuleIndex, PTY0V,           pty0v);
    PUT_REG_INFO_MULTI(SubModuleIndex, CAD,             cad);
    PUT_REG_INFO_MULTI(SubModuleIndex, SL2,             sl2);
    PUT_REG_INFO_MULTI(SubModuleIndex, PTY0H,           pty0h);
    PUT_REG_INFO_MULTI(SubModuleIndex, T4LUT1,          t4lut1);
    PUT_REG_INFO_MULTI(SubModuleIndex, T4LUT2,          t4lut2);
    PUT_REG_INFO_MULTI(SubModuleIndex, T4LUT3,          t4lut3);
    PUT_REG_INFO_MULTI(SubModuleIndex, ACT1,            act1);
    PUT_REG_INFO_MULTI(SubModuleIndex, PTCV,            ptcv);
    PUT_REG_INFO_MULTI(SubModuleIndex, ACT4,            act4);
    PUT_REG_INFO_MULTI(SubModuleIndex, PTCH,            ptch);
    PUT_REG_INFO_MULTI(SubModuleIndex, HF_COR,          hf_cor);
    PUT_REG_INFO_MULTI(SubModuleIndex, HF_ACT0,         hf_act0);
    PUT_REG_INFO_MULTI(SubModuleIndex, HF_ACT1,         hf_act1);
    PUT_REG_INFO_MULTI(SubModuleIndex, ACTC,            actc);
    PUT_REG_INFO_MULTI(SubModuleIndex, YLAD,            ylad);
    PUT_REG_INFO_MULTI(SubModuleIndex, HF_ACT2,         hf_act2);
    PUT_REG_INFO_MULTI(SubModuleIndex, HF_ACT3,         hf_act3);
    PUT_REG_INFO_MULTI(SubModuleIndex, HF_LUMA0,        hf_luma0);
    PUT_REG_INFO_MULTI(SubModuleIndex, HF_LUMA1,        hf_luma1);
    PUT_REG_INFO_MULTI(SubModuleIndex, Y4LUT4,          y4lut4);
    PUT_REG_INFO_MULTI(SubModuleIndex, Y4LUT5,          y4lut5);
    PUT_REG_INFO_MULTI(SubModuleIndex, Y4LUT6,          y4lut6);
    PUT_REG_INFO_MULTI(SubModuleIndex, Y4LUT7,          y4lut7);
    PUT_REG_INFO_MULTI(SubModuleIndex, A4LUT1,          a4lut1);
    //PUT_REG_INFO_MULTI(SubModuleIndex, SRAM_PINGPONG,   sram_pingpong);

    return  (*this);
}


template <>
ISP_MGR_YNRS_T&
ISP_MGR_YNRS_T::
get(MUINT8 SubModuleIndex, ISP_NVRAM_YNRS_T & rParam)
{
    GET_REG_INFO_MULTI(SubModuleIndex, CON1,            con1);
    GET_REG_INFO_MULTI(SubModuleIndex, CON2,            con2);
    GET_REG_INFO_MULTI(SubModuleIndex, YAD2,            yad2);
    GET_REG_INFO_MULTI(SubModuleIndex, Y4LUT1,          y4lut1);
    GET_REG_INFO_MULTI(SubModuleIndex, Y4LUT2,          y4lut2);
    GET_REG_INFO_MULTI(SubModuleIndex, Y4LUT3,          y4lut3);
    GET_REG_INFO_MULTI(SubModuleIndex, C4LUT1,          c4lut1);
    GET_REG_INFO_MULTI(SubModuleIndex, C4LUT2,          c4lut2);
    GET_REG_INFO_MULTI(SubModuleIndex, C4LUT3,          c4lut3);
    GET_REG_INFO_MULTI(SubModuleIndex, A4LUT2,          a4lut2);
    GET_REG_INFO_MULTI(SubModuleIndex, A4LUT3,          a4lut3);
    GET_REG_INFO_MULTI(SubModuleIndex, L4LUT1,          l4lut1);
    GET_REG_INFO_MULTI(SubModuleIndex, L4LUT2,          l4lut2);
    GET_REG_INFO_MULTI(SubModuleIndex, L4LUT3,          l4lut3);
    GET_REG_INFO_MULTI(SubModuleIndex, PTY0V,           pty0v);
    GET_REG_INFO_MULTI(SubModuleIndex, CAD,             cad);
    GET_REG_INFO_MULTI(SubModuleIndex, SL2,             sl2);
    GET_REG_INFO_MULTI(SubModuleIndex, PTY0H,           pty0h);
    GET_REG_INFO_MULTI(SubModuleIndex, T4LUT1,          t4lut1);
    GET_REG_INFO_MULTI(SubModuleIndex, T4LUT2,          t4lut2);
    GET_REG_INFO_MULTI(SubModuleIndex, T4LUT3,          t4lut3);
    GET_REG_INFO_MULTI(SubModuleIndex, ACT1,            act1);
    GET_REG_INFO_MULTI(SubModuleIndex, PTCV,            ptcv);
    GET_REG_INFO_MULTI(SubModuleIndex, ACT4,            act4);
    GET_REG_INFO_MULTI(SubModuleIndex, PTCH,            ptch);
    GET_REG_INFO_MULTI(SubModuleIndex, HF_COR,          hf_cor);
    GET_REG_INFO_MULTI(SubModuleIndex, HF_ACT0,         hf_act0);
    GET_REG_INFO_MULTI(SubModuleIndex, HF_ACT1,         hf_act1);
    GET_REG_INFO_MULTI(SubModuleIndex, ACTC,            actc);
    GET_REG_INFO_MULTI(SubModuleIndex, YLAD,            ylad);
    GET_REG_INFO_MULTI(SubModuleIndex, HF_ACT2,         hf_act2);
    GET_REG_INFO_MULTI(SubModuleIndex, HF_ACT3,         hf_act3);
    GET_REG_INFO_MULTI(SubModuleIndex, HF_LUMA0,        hf_luma0);
    GET_REG_INFO_MULTI(SubModuleIndex, HF_LUMA1,        hf_luma1);
    GET_REG_INFO_MULTI(SubModuleIndex, Y4LUT4,          y4lut4);
    GET_REG_INFO_MULTI(SubModuleIndex, Y4LUT5,          y4lut5);
    GET_REG_INFO_MULTI(SubModuleIndex, Y4LUT6,          y4lut6);
    GET_REG_INFO_MULTI(SubModuleIndex, Y4LUT7,          y4lut7);
    GET_REG_INFO_MULTI(SubModuleIndex, A4LUT1,          a4lut1);
    //GET_REG_INFO_MULTI(SubModuleIndex, SRAM_PINGPONG,   sram_pingpong);

    return  (*this);
}

MBOOL
ISP_MGR_YNRS_T::
apply_P1(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, TuningMgr& rTuning, MINT32 i4SubsampleIdex)
{
    if(SubModuleIndex >= ESubModule_NUM){
        return MFALSE;
    }

    MBOOL bEnable = isEnable(SubModuleIndex);

    if(bEnable){
        if(!ISP_MGR_SLK_T::getInstance(m_eSensorDev).isEnable(ISP_MGR_SLK_T::ESLK_R2)){
            reinterpret_cast<REG_YNRS_R1_YNRS_CON1*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, CON1))->Bits.YNRS_SL2_LINK = 0;
        }
    }

    //Top Control
    switch (SubModuleIndex)
    {
        case EYNRS_R1:
            ISP_MGR_CAMCTL_T::getInstance(m_eSensorDev).setEnable_YNRS_R1(bEnable);
            rTuning.updateEngine(eTuningMgrFunc_YNRS_R1, bEnable, i4SubsampleIdex);
            break;
        default:
            CAM_LOGE("Apply Error Submodule Index: %d", SubModuleIndex);
            return  MFALSE;
    }

    AAA_TRACE_DRV(DRV_YNRS);
    rTuning.tuningMgrWriteRegs( (TUNING_MGR_REG_IO_STRUCT*)(&(m_rIspRegInfo[SubModuleIndex][SubModuleIndex])),
        (m_u4RegInfoNum), i4SubsampleIdex);
    AAA_TRACE_END_DRV;

    dumpRegInfoP1("YNRS", SubModuleIndex);

    return  MTRUE;

}


};
