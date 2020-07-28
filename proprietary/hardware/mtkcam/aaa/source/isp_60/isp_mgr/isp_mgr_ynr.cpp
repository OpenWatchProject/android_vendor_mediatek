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
#define LOG_TAG "isp_mgr_ynr"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>
#include <camera_custom_nvram.h>
#include "isp_mgr.h"

#include <isp_interpolation/isp_interpolation.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/imgbuf/IIonImageBufferHeap.h>
#include <mtkcam/def/UITypes.h>
#include <private/aaa_utils.h>
#include <array>

using namespace NSIspTuning;

#define MY_INST NS3Av3::INST_T<ISP_MGR_YNR_T>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

namespace NSIspTuning
{


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  YNR
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_YNR_T&
ISP_MGR_YNR_T::
getInstance(MUINT32 const eSensorDev)
{
    ISP_MGR_MODULE_GET_INSTANCE(YNR);
}

template <>
ISP_MGR_YNR_T&
ISP_MGR_YNR_T::
put(MUINT8 SubModuleIndex, ISP_NVRAM_YNR_LUT_T const& rParam)
{
    for (MINT32 i = 0; i < ANR_HW_TBL_NUM; i++) {
        m_rIspRegInfo_TBL[SubModuleIndex][i].val = rParam.set[i];
    }
    return  (*this);
}

template <>
ISP_MGR_YNR_T&
ISP_MGR_YNR_T::
get(MUINT8 SubModuleIndex, ISP_NVRAM_YNR_LUT_T & rParam)
{
    for (MINT32 i = 0; i < ANR_HW_TBL_NUM; i++) {
        rParam.set[i] = m_rIspRegInfo_TBL[SubModuleIndex][i].val;
    }

    return  (*this);
}

template <>
ISP_MGR_YNR_T&
ISP_MGR_YNR_T::
put(MUINT8 SubModuleIndex, ISP_NVRAM_YNR_T const& rParam)
{
    PUT_REG_INFO_MULTI(SubModuleIndex, CON1,            con1);
    PUT_REG_INFO_MULTI(SubModuleIndex, CON2,            con2);
    PUT_REG_INFO_MULTI(SubModuleIndex, YAD1,            yad1);
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
    PUT_REG_INFO_MULTI(SubModuleIndex, PTY1V,           pty1v);
    PUT_REG_INFO_MULTI(SubModuleIndex, SL2,             sl2);
    PUT_REG_INFO_MULTI(SubModuleIndex, PTY2V,           pty2v);
    PUT_REG_INFO_MULTI(SubModuleIndex, PTY3V,           pty3v);
    PUT_REG_INFO_MULTI(SubModuleIndex, PTY0H,           pty0h);
    PUT_REG_INFO_MULTI(SubModuleIndex, PTY1H,           pty1h);
    PUT_REG_INFO_MULTI(SubModuleIndex, PTY2H,           pty2h);
    PUT_REG_INFO_MULTI(SubModuleIndex, T4LUT1,          t4lut1);
    PUT_REG_INFO_MULTI(SubModuleIndex, T4LUT2,          t4lut2);
    PUT_REG_INFO_MULTI(SubModuleIndex, T4LUT3,          t4lut3);
    PUT_REG_INFO_MULTI(SubModuleIndex, ACT1,            act1);
    PUT_REG_INFO_MULTI(SubModuleIndex, PTY3H,           pty3h);
    PUT_REG_INFO_MULTI(SubModuleIndex, PTCV,            ptcv);
    PUT_REG_INFO_MULTI(SubModuleIndex, ACT4,            act4);
    PUT_REG_INFO_MULTI(SubModuleIndex, PTCH,            ptch);
    PUT_REG_INFO_MULTI(SubModuleIndex, YLVL0,           ylvl0);
    PUT_REG_INFO_MULTI(SubModuleIndex, YLVL1,           ylvl1);
    PUT_REG_INFO_MULTI(SubModuleIndex, HF_COR,          hf_cor);
    PUT_REG_INFO_MULTI(SubModuleIndex, HF_ACT0,         hf_act0);
    PUT_REG_INFO_MULTI(SubModuleIndex, HF_ACT1,         hf_act1);
    PUT_REG_INFO_MULTI(SubModuleIndex, ACTC,            actc);
    PUT_REG_INFO_MULTI(SubModuleIndex, YLAD,            ylad);
    PUT_REG_INFO_MULTI(SubModuleIndex, HF_ACT2,         hf_act2);
    PUT_REG_INFO_MULTI(SubModuleIndex, HF_ACT3,         hf_act3);
    PUT_REG_INFO_MULTI(SubModuleIndex, HF_LUMA0,        hf_luma0);
    PUT_REG_INFO_MULTI(SubModuleIndex, HF_LUMA1,        hf_luma1);
    //PUT_REG_INFO_MULTI(SubModuleIndex, LCE_GAIN1,       lce_gain1);
    //PUT_REG_INFO_MULTI(SubModuleIndex, LCE_GAIN2,       lce_gain2);
    //PUT_REG_INFO_MULTI(SubModuleIndex, LCE_LUTP1,       lce_lutp1);
    //PUT_REG_INFO_MULTI(SubModuleIndex, LCE_LUTP2,       lce_lutp2);
    //PUT_REG_INFO_MULTI(SubModuleIndex, LCE_LUTO1,       lce_luto1);
    //PUT_REG_INFO_MULTI(SubModuleIndex, LCE_LUTO2,       lce_luto2);
    //PUT_REG_INFO_MULTI(SubModuleIndex, LCE_LUTS1,       lce_luts1);
    //PUT_REG_INFO_MULTI(SubModuleIndex, LCE_LUTS2,       lce_luts2);
    //PUT_REG_INFO_MULTI(SubModuleIndex, LCE_LUTS3,       lce_luts3);
    //PUT_REG_INFO_MULTI(SubModuleIndex, LCE_LUTS4,       lce_luts4);
    PUT_REG_INFO_MULTI(SubModuleIndex, Y4LUT4,          y4lut4);
    PUT_REG_INFO_MULTI(SubModuleIndex, Y4LUT5,          y4lut5);
    PUT_REG_INFO_MULTI(SubModuleIndex, Y4LUT6,          y4lut6);
    PUT_REG_INFO_MULTI(SubModuleIndex, Y4LUT7,          y4lut7);
    PUT_REG_INFO_MULTI(SubModuleIndex, A4LUT1,          a4lut1);
    PUT_REG_INFO_MULTI(SubModuleIndex, SKIN_CON,        skin_con);
    //PUT_REG_INFO_MULTI(SubModuleIndex, SKIN1_Y,         skin1_y);
    //PUT_REG_INFO_MULTI(SubModuleIndex, SKIN1_U,         skin1_u);
    //PUT_REG_INFO_MULTI(SubModuleIndex, SKIN1_V,         skin1_v);
    //PUT_REG_INFO_MULTI(SubModuleIndex, SKIN2_Y,         skin2_y);
    //PUT_REG_INFO_MULTI(SubModuleIndex, SKIN2_U,         skin2_u);
    //PUT_REG_INFO_MULTI(SubModuleIndex, SKIN2_V,         skin2_v);
    //PUT_REG_INFO_MULTI(SubModuleIndex, SKIN3_Y,         skin3_y);
    //PUT_REG_INFO_MULTI(SubModuleIndex, SKIN3_U,         skin3_u);
    //PUT_REG_INFO_MULTI(SubModuleIndex, SKIN3_V,         skin3_v);
    //PUT_REG_INFO_MULTI(SubModuleIndex, RSV1,            rsv1);
    //PUT_REG_INFO_MULTI(SubModuleIndex, DGLC_CTRL,       dglc_ctrl);
    //PUT_REG_INFO_MULTI(SubModuleIndex, DGLC_TH,         dglc_th);
    //PUT_REG_INFO_MULTI(SubModuleIndex, DGLC_GAIN_Y,     dglc_gain_y);
    //PUT_REG_INFO_MULTI(SubModuleIndex, DGLC_GAIN_U,     dglc_gain_u);
    //PUT_REG_INFO_MULTI(SubModuleIndex, DGLC_GAIN_V,     dglc_gain_v);
    //PUT_REG_INFO_MULTI(SubModuleIndex, DGLC_OFST_Y,     dglc_ofst_y);
    //PUT_REG_INFO_MULTI(SubModuleIndex, DGLC_OFST_U,     dglc_ofst_u);
    //PUT_REG_INFO_MULTI(SubModuleIndex, DGLC_OFST_V,     dglc_ofst_v);

    return  (*this);
}


template <>
ISP_MGR_YNR_T&
ISP_MGR_YNR_T::
get(MUINT8 SubModuleIndex, ISP_NVRAM_YNR_T & rParam)
{
    GET_REG_INFO_MULTI(SubModuleIndex, CON1,            con1);
    GET_REG_INFO_MULTI(SubModuleIndex, CON2,            con2);
    GET_REG_INFO_MULTI(SubModuleIndex, YAD1,            yad1);
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
    GET_REG_INFO_MULTI(SubModuleIndex, PTY1V,           pty1v);
    GET_REG_INFO_MULTI(SubModuleIndex, SL2,             sl2);
    GET_REG_INFO_MULTI(SubModuleIndex, PTY2V,           pty2v);
    GET_REG_INFO_MULTI(SubModuleIndex, PTY3V,           pty3v);
    GET_REG_INFO_MULTI(SubModuleIndex, PTY0H,           pty0h);
    GET_REG_INFO_MULTI(SubModuleIndex, PTY1H,           pty1h);
    GET_REG_INFO_MULTI(SubModuleIndex, PTY2H,           pty2h);
    GET_REG_INFO_MULTI(SubModuleIndex, T4LUT1,          t4lut1);
    GET_REG_INFO_MULTI(SubModuleIndex, T4LUT2,          t4lut2);
    GET_REG_INFO_MULTI(SubModuleIndex, T4LUT3,          t4lut3);
    GET_REG_INFO_MULTI(SubModuleIndex, ACT1,            act1);
    GET_REG_INFO_MULTI(SubModuleIndex, PTY3H,           pty3h);
    GET_REG_INFO_MULTI(SubModuleIndex, PTCV,            ptcv);
    GET_REG_INFO_MULTI(SubModuleIndex, ACT4,            act4);
    GET_REG_INFO_MULTI(SubModuleIndex, PTCH,            ptch);
    GET_REG_INFO_MULTI(SubModuleIndex, YLVL0,           ylvl0);
    GET_REG_INFO_MULTI(SubModuleIndex, YLVL1,           ylvl1);
    GET_REG_INFO_MULTI(SubModuleIndex, HF_COR,          hf_cor);
    GET_REG_INFO_MULTI(SubModuleIndex, HF_ACT0,         hf_act0);
    GET_REG_INFO_MULTI(SubModuleIndex, HF_ACT1,         hf_act1);
    GET_REG_INFO_MULTI(SubModuleIndex, ACTC,            actc);
    GET_REG_INFO_MULTI(SubModuleIndex, YLAD,            ylad);
    GET_REG_INFO_MULTI(SubModuleIndex, HF_ACT2,         hf_act2);
    GET_REG_INFO_MULTI(SubModuleIndex, HF_ACT3,         hf_act3);
    GET_REG_INFO_MULTI(SubModuleIndex, HF_LUMA0,        hf_luma0);
    GET_REG_INFO_MULTI(SubModuleIndex, HF_LUMA1,        hf_luma1);
    //GET_REG_INFO_MULTI(SubModuleIndex, LCE_GAIN1,       lce_gain1);
    //GET_REG_INFO_MULTI(SubModuleIndex, LCE_GAIN2,       lce_gain2);
    //GET_REG_INFO_MULTI(SubModuleIndex, LCE_LUTP1,       lce_lutp1);
    //GET_REG_INFO_MULTI(SubModuleIndex, LCE_LUTP2,       lce_lutp2);
    //GET_REG_INFO_MULTI(SubModuleIndex, LCE_LUTO1,       lce_luto1);
    //GET_REG_INFO_MULTI(SubModuleIndex, LCE_LUTO2,       lce_luto2);
    //GET_REG_INFO_MULTI(SubModuleIndex, LCE_LUTS1,       lce_luts1);
    //GET_REG_INFO_MULTI(SubModuleIndex, LCE_LUTS2,       lce_luts2);
    //GET_REG_INFO_MULTI(SubModuleIndex, LCE_LUTS3,       lce_luts3);
    //GET_REG_INFO_MULTI(SubModuleIndex, LCE_LUTS4,       lce_luts4);
    GET_REG_INFO_MULTI(SubModuleIndex, Y4LUT4,          y4lut4);
    GET_REG_INFO_MULTI(SubModuleIndex, Y4LUT5,          y4lut5);
    GET_REG_INFO_MULTI(SubModuleIndex, Y4LUT6,          y4lut6);
    GET_REG_INFO_MULTI(SubModuleIndex, Y4LUT7,          y4lut7);
    GET_REG_INFO_MULTI(SubModuleIndex, A4LUT1,          a4lut1);
    GET_REG_INFO_MULTI(SubModuleIndex, SKIN_CON,        skin_con);
    //GET_REG_INFO_MULTI(SubModuleIndex, SKIN1_Y,         skin1_y);
    //GET_REG_INFO_MULTI(SubModuleIndex, SKIN1_U,         skin1_u);
    //GET_REG_INFO_MULTI(SubModuleIndex, SKIN1_V,         skin1_v);
    //GET_REG_INFO_MULTI(SubModuleIndex, SKIN2_Y,         skin2_y);
    //GET_REG_INFO_MULTI(SubModuleIndex, SKIN2_U,         skin2_u);
    //GET_REG_INFO_MULTI(SubModuleIndex, SKIN2_V,         skin2_v);
    //GET_REG_INFO_MULTI(SubModuleIndex, SKIN3_Y,         skin3_y);
    //GET_REG_INFO_MULTI(SubModuleIndex, SKIN3_U,         skin3_u);
    //GET_REG_INFO_MULTI(SubModuleIndex, SKIN3_V,         skin3_v);
    //GET_REG_INFO_MULTI(SubModuleIndex, RSV1,            rsv1);
    //GET_REG_INFO_MULTI(SubModuleIndex, DGLC_CTRL,       dglc_ctrl);
    //GET_REG_INFO_MULTI(SubModuleIndex, DGLC_TH,         dglc_th);
    //GET_REG_INFO_MULTI(SubModuleIndex, DGLC_GAIN_Y,     dglc_gain_y);
    //GET_REG_INFO_MULTI(SubModuleIndex, DGLC_GAIN_U,     dglc_gain_u);
    //GET_REG_INFO_MULTI(SubModuleIndex, DGLC_GAIN_V,     dglc_gain_v);
    //GET_REG_INFO_MULTI(SubModuleIndex, DGLC_OFST_Y,     dglc_ofst_y);
    //GET_REG_INFO_MULTI(SubModuleIndex, DGLC_OFST_U,     dglc_ofst_u);
    //GET_REG_INFO_MULTI(SubModuleIndex, DGLC_OFST_V,     dglc_ofst_v);

    return  (*this);
}

template <>
ISP_MGR_YNR_T&
ISP_MGR_YNR_T::
put(MUINT8 SubModuleIndex, ISP_NVRAM_YNR_SKIN_T const& rParam)
{
    PUT_REG_INFO_MULTI(SubModuleIndex, SKIN_CON,        skin_con);
    PUT_REG_INFO_MULTI(SubModuleIndex, SKIN1_Y,         skin1_y);
    PUT_REG_INFO_MULTI(SubModuleIndex, SKIN1_U,         skin1_u);
    PUT_REG_INFO_MULTI(SubModuleIndex, SKIN1_V,         skin1_v);
    PUT_REG_INFO_MULTI(SubModuleIndex, SKIN2_Y,         skin2_y);
    PUT_REG_INFO_MULTI(SubModuleIndex, SKIN2_U,         skin2_u);
    PUT_REG_INFO_MULTI(SubModuleIndex, SKIN2_V,         skin2_v);
    PUT_REG_INFO_MULTI(SubModuleIndex, SKIN3_Y,         skin3_y);
    PUT_REG_INFO_MULTI(SubModuleIndex, SKIN3_U,         skin3_u);
    PUT_REG_INFO_MULTI(SubModuleIndex, SKIN3_V,         skin3_v);

    return  (*this);
}


template <>
ISP_MGR_YNR_T&
ISP_MGR_YNR_T::
get(MUINT8 SubModuleIndex, ISP_NVRAM_YNR_SKIN_T & rParam)
{
    //CAUTION: SKIN_CON will OVERWRITE SKIN_CON in ISP_NVRAM_YNR_T
    GET_REG_INFO_MULTI(SubModuleIndex, SKIN_CON,        skin_con);
    GET_REG_INFO_MULTI(SubModuleIndex, SKIN1_Y,         skin1_y);
    GET_REG_INFO_MULTI(SubModuleIndex, SKIN1_U,         skin1_u);
    GET_REG_INFO_MULTI(SubModuleIndex, SKIN1_V,         skin1_v);
    GET_REG_INFO_MULTI(SubModuleIndex, SKIN2_Y,         skin2_y);
    GET_REG_INFO_MULTI(SubModuleIndex, SKIN2_U,         skin2_u);
    GET_REG_INFO_MULTI(SubModuleIndex, SKIN2_V,         skin2_v);
    GET_REG_INFO_MULTI(SubModuleIndex, SKIN3_Y,         skin3_y);
    GET_REG_INFO_MULTI(SubModuleIndex, SKIN3_U,         skin3_u);
    GET_REG_INFO_MULTI(SubModuleIndex, SKIN3_V,         skin3_v);

    return  (*this);
}

template <>
ISP_MGR_YNR_T&
ISP_MGR_YNR_T::
put(MUINT8 SubModuleIndex, ISP_NVRAM_YNR_LCE_OUT_T const& rParam)
{
    PUT_REG_INFO_MULTI(SubModuleIndex, LCE_LUTP1,       lce_lutp1);
    PUT_REG_INFO_MULTI(SubModuleIndex, LCE_LUTP2,       lce_lutp2);
    PUT_REG_INFO_MULTI(SubModuleIndex, LCE_LUTO1,       lce_luto1);
    PUT_REG_INFO_MULTI(SubModuleIndex, LCE_LUTO2,       lce_luto2);
    PUT_REG_INFO_MULTI(SubModuleIndex, LCE_LUTS1,       lce_luts1);
    PUT_REG_INFO_MULTI(SubModuleIndex, LCE_LUTS2,       lce_luts2);
    PUT_REG_INFO_MULTI(SubModuleIndex, LCE_LUTS3,       lce_luts3);
    PUT_REG_INFO_MULTI(SubModuleIndex, LCE_LUTS4,       lce_luts4);

    return  (*this);
}


template <>
ISP_MGR_YNR_T&
ISP_MGR_YNR_T::
get(MUINT8 SubModuleIndex, ISP_NVRAM_YNR_LCE_OUT_T & rParam)
{
    GET_REG_INFO_MULTI(SubModuleIndex, LCE_LUTP1,       lce_lutp1);
    GET_REG_INFO_MULTI(SubModuleIndex, LCE_LUTP2,       lce_lutp2);
    GET_REG_INFO_MULTI(SubModuleIndex, LCE_LUTO1,       lce_luto1);
    GET_REG_INFO_MULTI(SubModuleIndex, LCE_LUTO2,       lce_luto2);
    GET_REG_INFO_MULTI(SubModuleIndex, LCE_LUTS1,       lce_luts1);
    GET_REG_INFO_MULTI(SubModuleIndex, LCE_LUTS2,       lce_luts2);
    GET_REG_INFO_MULTI(SubModuleIndex, LCE_LUTS3,       lce_luts3);
    GET_REG_INFO_MULTI(SubModuleIndex, LCE_LUTS4,       lce_luts4);

    return  (*this);
}

MBOOL
ISP_MGR_YNR_T::
apply_P2(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, dip_x_reg_t* pReg)
{
    if(SubModuleIndex >= ESubModule_NUM){
        return MFALSE;
    }

    MBOOL bEnable = isEnable(SubModuleIndex);

    //LCE LINK
    MBOOL bSkinEnable = isSkinEnable(SubModuleIndex);

    reinterpret_cast<YNR_REG_D1A_YNR_CON1*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, CON1))->Bits.YNR_LCE_LINK
        &= (pReg->DIPCTL_D1A_DIPCTL_YUV_EN1.Bits.DIPCTL_LCE_D1_EN);

    //Chooo
    //MBOOL bAlphaFaceEn = MTRUE; // SRZ3_EN & DMGI_EN
    reinterpret_cast<YNR_REG_D1A_YNR_SKIN_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, SKIN_CON))->Bits.YNR_SKIN_LINK &= m_bSkinEnable[SubModuleIndex];

    //Top Control
    switch (SubModuleIndex)
    {
        case EYNR_D1:
            ISP_WRITE_ENABLE_BITS(pReg, DIPCTL_D1A_DIPCTL_YUV_EN1, DIPCTL_YNR_D1_EN, bEnable);
            break;
        default:
            CAM_LOGE("Apply Error Submodule Index: %d", SubModuleIndex);
            return  MFALSE;
    }


    writeRegs(static_cast<RegInfo_T*>(m_rIspRegInfo[SubModuleIndex]), m_u4RegInfoNum, pReg);
    writeRegs(static_cast<RegInfo_T*>(m_rIspRegInfo_TBL[SubModuleIndex]), ANR_HW_TBL_NUM, pReg);

    dumpRegInfoP2("YNR", SubModuleIndex);
    return  MTRUE;
}

MBOOL
ISP_MGR_YNR_T::
createAlphaMap(MUINT8 SubModuleIndex, MUINT32 u4Width,  MUINT32 height){
    MUINT32 x_size_default = u4Width;
    MUINT32 y_size_default = height;
    // create buffer
    MINT32 bufBoundaryInBytes_default[3] = {0, 0, 0};
    MUINT32 bufStridesInBytes_bpci_default[3] = {x_size_default, 0, 0};

    char index[50] = {'\0'};
    sprintf(index, "%04d", SubModuleIndex);

    for (MINT32 i = 0; i < FACE_ALPHA_MAP_SIZE; i++){
            std::string strName = "AlphaMap_default_";
            char count[50] = {'\0'};
            sprintf(count, "%04d", i);

            strName = strName+index+count;
            IImageBufferAllocator::ImgParam imgParam_alphamap_default =
                IImageBufferAllocator::ImgParam((EImageFormat)eImgFmt_STA_BYTE,
                MSize(x_size_default, y_size_default), bufStridesInBytes_bpci_default, bufBoundaryInBytes_default, 1);

            sp<IIonImageBufferHeap> pHeap_alphamap_default =
                IIonImageBufferHeap::create(strName.c_str(), imgParam_alphamap_default);

            if (pHeap_alphamap_default == NULL) {
                CAM_LOGE("[%s] ImageBufferHeap create fail", strName.c_str());
                return MFALSE;
            }

            IImageBuffer* pImgBuf_alphamap_default = pHeap_alphamap_default->createImageBuffer();

            if (pImgBuf_alphamap_default == NULL) {
                CAM_LOGE("[%s] ImageBufferHeap create fail", strName.c_str());
                return MFALSE;
            }

            // lock buffer
            MUINT const usage_default = (GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN |
            GRALLOC_USAGE_HW_CAMERA_READ |
            GRALLOC_USAGE_HW_CAMERA_WRITE);

            if (!(pImgBuf_alphamap_default->lockBuf(strName.c_str(), usage_default))){
                CAM_LOGW("[%s] Stuff ImageBuffer lock fail",  strName.c_str());
                return MFALSE;
            }

            if (pImgBuf_alphamap_default){
                m_AlphaMap[SubModuleIndex][i].pFaceAlphaBuf = static_cast<IImageBuffer*>(pImgBuf_alphamap_default);
            }
            m_AlphaMap[SubModuleIndex][i].strName = strName;
            CAM_LOGD("[createAlphaMap] create face map(%s)",  strName.c_str());
    }

    return MTRUE;
}

MVOID
ISP_MGR_YNR_T::
releaseAlphaMap(){
    for (MINT32 i = 0; i < ESubModule_NUM; i++) {
        for (MINT32 j = 0; j < FACE_ALPHA_MAP_SIZE; j++) {
            if (m_AlphaMap[i][j].pFaceAlphaBuf != NULL){
                static_cast<IImageBuffer*>(m_AlphaMap[i][j].pFaceAlphaBuf)->unlockBuf(m_AlphaMap[i][j].strName.c_str());
                // destroy buffer
                sp<IImageBuffer> temp = static_cast<IImageBuffer*>(m_AlphaMap[i][j].pFaceAlphaBuf);
                m_AlphaMap[i][j].pFaceAlphaBuf = NULL;
                CAM_LOGD("[releaseAlphaMap] release face map(%s)",  m_AlphaMap[i][j].strName.c_str());
            }
        }
    }
}

}
