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
#define LOG_TAG "isp_mgr_ndg"

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

#define MY_INST NS3Av3::INST_T<ISP_MGR_NDG_T>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

namespace NSIspTuning
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// NDG
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_NDG_T&
ISP_MGR_NDG_T::
getInstance(MUINT32 const eSensorDev)
{
    ISP_MGR_MODULE_GET_INSTANCE(NDG);
}

#if 0
template <>
ISP_MGR_NDG_T&
ISP_MGR_NDG_T::
put(MUINT8 SubModuleIndex, ISP_NVRAM_NDG_T const& rParam)
{
    PUT_REG_INFO_MULTI(SubModuleIndex, RAN_0,           ran_0);
    PUT_REG_INFO_MULTI(SubModuleIndex, RAN_1,           ran_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, RAN_2,           ran_2);
    PUT_REG_INFO_MULTI(SubModuleIndex, RAN_3,           ran_3);
    PUT_REG_INFO_MULTI(SubModuleIndex, CROP_X,          crop_x);
    PUT_REG_INFO_MULTI(SubModuleIndex, CROP_Y,          crop_y);

    return  (*this);
}

template <>
ISP_MGR_NDG_T&
ISP_MGR_NDG_T::
get(MUINT8 SubModuleIndex, ISP_NVRAM_NDG_T & rParam)
{
    GET_REG_INFO_MULTI(SubModuleIndex, RAN_0,           ran_0);
    GET_REG_INFO_MULTI(SubModuleIndex, RAN_1,           ran_1);
    GET_REG_INFO_MULTI(SubModuleIndex, RAN_2,           ran_2);
    GET_REG_INFO_MULTI(SubModuleIndex, RAN_3,           ran_3);
    GET_REG_INFO_MULTI(SubModuleIndex, CROP_X,          crop_x);
    GET_REG_INFO_MULTI(SubModuleIndex, CROP_Y,          crop_y);

    return  (*this);
}
#endif

MBOOL
ISP_MGR_NDG_T::
apply_P2(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, dip_x_reg_t* pReg)
{

    MBOOL bEnable = isEnable(SubModuleIndex);
    if(bEnable){

        reinterpret_cast<NDG_REG_D1A_NDG_RAN_0*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, RAN_0))->Bits.NDG_NOISE_GSEED = (MINT16)::systemTime();

        reinterpret_cast<NDG_REG_D1A_NDG_RAN_2*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, RAN_2))->Bits.NDG_IMAGE_WD = rRawIspCamInfo.rCropRzInfo.sRRZout.w;

        MUINT32 padding_16_w = (16 - (rRawIspCamInfo.rCropRzInfo.sRRZout.w % 16))%16;
        MUINT32 padding_16_h = (16 - (rRawIspCamInfo.rCropRzInfo.sRRZout.h % 16))%16;
        reinterpret_cast<NDG_REG_D1A_NDG_RAN_3*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, RAN_3))->Bits.NDG_TILE_WD = (rRawIspCamInfo.rCropRzInfo.sRRZout.w + padding_16_w);
        reinterpret_cast<NDG_REG_D1A_NDG_RAN_3*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, RAN_3))->Bits.NDG_TILE_HT = (rRawIspCamInfo.rCropRzInfo.sRRZout.h + padding_16_w);

        reinterpret_cast<NDG_REG_D1A_NDG_CROP_X*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, CROP_X))->Bits.NDG_CROP_XSTART = 0;
        reinterpret_cast<NDG_REG_D1A_NDG_CROP_X*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, CROP_X))->Bits.NDG_CROP_XEND   = (rRawIspCamInfo.rCropRzInfo.sRRZout.w - 1);
        reinterpret_cast<NDG_REG_D1A_NDG_CROP_Y*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, CROP_Y))->Bits.NDG_CROP_YSTART = 0;
        reinterpret_cast<NDG_REG_D1A_NDG_CROP_Y*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, CROP_Y))->Bits.NDG_CROP_YEND   = (rRawIspCamInfo.rCropRzInfo.sRRZout.h - 1);
    }

    //Top Control
    switch (SubModuleIndex)
    {
        case ENDG_D1:
            ISP_WRITE_ENABLE_BITS(pReg, DIPCTL_D1A_DIPCTL_YUV_EN1, DIPCTL_NDG_D1_EN, bEnable);
            break;
        case ENDG_D2:
            ISP_WRITE_ENABLE_BITS(pReg, DIPCTL_D1A_DIPCTL_YUV_EN1, DIPCTL_NDG_D2_EN, bEnable);
            break;
        default:
            CAM_LOGE("Apply Error Submodule Index: %d", SubModuleIndex);
            return  MFALSE;
    }

    writeRegs(static_cast<RegInfo_T*>(m_rIspRegInfo[SubModuleIndex]), m_u4RegInfoNum, pReg);

    dumpRegInfoP2("NDG", SubModuleIndex);

    return  MTRUE;
}


}

