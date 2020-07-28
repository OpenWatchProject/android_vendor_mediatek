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
#define LOG_TAG "isp_mgr_ee"

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

#define MY_INST NS3Av3::INST_T<ISP_MGR_EE_T>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;
namespace NSIspTuning
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// EE (not support SE)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_EE_T&
ISP_MGR_EE_T::
getInstance(MUINT32 const eSensorDev)
{
    ISP_MGR_MODULE_GET_INSTANCE(EE);
}

template <>
ISP_MGR_EE_T&
ISP_MGR_EE_T::
put(MUINT8 SubModuleIndex, ISP_NVRAM_EE_T const& rParam)

{
    //PUT_REG_INFO_MULTI(SubModuleIndex, CTRL,                ctrl);
    //PUT_REG_INFO_MULTI(SubModuleIndex, TOP_CTRL,            top_ctrl);
    PUT_REG_INFO_MULTI(SubModuleIndex, BLND_CTRL_1,         blnd_ctrl_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, BLND_CTRL_2,         blnd_ctrl_2);
    PUT_REG_INFO_MULTI(SubModuleIndex, CORE_CTRL,           core_ctrl);
    PUT_REG_INFO_MULTI(SubModuleIndex, GN_CTRL_1,           gn_ctrl_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, LUMA_CTRL_1,         luma_ctrl_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, LUMA_CTRL_2,         luma_ctrl_2);
    PUT_REG_INFO_MULTI(SubModuleIndex, LUMA_SLNK_CTRL,      luma_slnk_ctrl);
    PUT_REG_INFO_MULTI(SubModuleIndex, GLUT_CTRL_1,         glut_ctrl_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, GLUT_CTRL_2,         glut_ctrl_2);
    PUT_REG_INFO_MULTI(SubModuleIndex, GLUT_CTRL_3,         glut_ctrl_3);
    PUT_REG_INFO_MULTI(SubModuleIndex, GLUT_CTRL_4,         glut_ctrl_4);
    PUT_REG_INFO_MULTI(SubModuleIndex, GLUT_CTRL_5,         glut_ctrl_5);
    PUT_REG_INFO_MULTI(SubModuleIndex, GLUT_CTRL_6,         glut_ctrl_6);
    PUT_REG_INFO_MULTI(SubModuleIndex, ARTIFACT_CTRL,       artifact_ctrl);
    PUT_REG_INFO_MULTI(SubModuleIndex, CLIP_CTRL,           clip_ctrl);
    PUT_REG_INFO_MULTI(SubModuleIndex, GN_CTRL_2,           gn_ctrl_2);
    PUT_REG_INFO_MULTI(SubModuleIndex, ST_CTRL_1,           st_ctrl_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, ST_CTRL_2,           st_ctrl_2);
    PUT_REG_INFO_MULTI(SubModuleIndex, CE_CTRL,             ce_ctrl);
    PUT_REG_INFO_MULTI(SubModuleIndex, CE_SL_CTRL,          ce_sl_ctrl);
    PUT_REG_INFO_MULTI(SubModuleIndex, CBOOST_CTRL_1,       cboost_ctrl_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, CBOOST_CTRL_2,       cboost_ctrl_2);
    PUT_REG_INFO_MULTI(SubModuleIndex, PBC1_CTRL_0,         pbc1_ctrl_0);
    PUT_REG_INFO_MULTI(SubModuleIndex, PBC1_CTRL_1,         pbc1_ctrl_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, PBC1_CTRL_2,         pbc1_ctrl_2);
    PUT_REG_INFO_MULTI(SubModuleIndex, PBC1_CTRL_3,         pbc1_ctrl_3);
    PUT_REG_INFO_MULTI(SubModuleIndex, PBC2_CTRL_0,         pbc2_ctrl_0);
    PUT_REG_INFO_MULTI(SubModuleIndex, PBC2_CTRL_1,         pbc2_ctrl_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, PBC2_CTRL_2,         pbc2_ctrl_2);
    PUT_REG_INFO_MULTI(SubModuleIndex, PBC2_CTRL_3,         pbc2_ctrl_3);
    PUT_REG_INFO_MULTI(SubModuleIndex, PBC3_CTRL_0,         pbc3_ctrl_0);
    PUT_REG_INFO_MULTI(SubModuleIndex, PBC3_CTRL_1,         pbc3_ctrl_1);
    PUT_REG_INFO_MULTI(SubModuleIndex, PBC3_CTRL_2,         pbc3_ctrl_2);
    PUT_REG_INFO_MULTI(SubModuleIndex, PBC3_CTRL_3,         pbc3_ctrl_3);
    //PUT_REG_INFO_MULTI(SubModuleIndex, SE_Y_SPECL_CTRL,     se_y_specl_ctrl);
    //PUT_REG_INFO_MULTI(SubModuleIndex, SE_EDGE_CTRL_1,      se_edge_ctrl_1);
    //PUT_REG_INFO_MULTI(SubModuleIndex, SE_EDGE_CTRL_2,      se_edge_ctrl_2);
    //PUT_REG_INFO_MULTI(SubModuleIndex, SE_CORE_CTRL_1,      se_core_ctrl_1);
    //PUT_REG_INFO_MULTI(SubModuleIndex, SE_CORE_CTRL_2,      se_core_ctrl_2);
    return  (*this);
}

template <>
ISP_MGR_EE_T&
ISP_MGR_EE_T::
get(MUINT8 SubModuleIndex, ISP_NVRAM_EE_T & rParam)
{
    //GET_REG_INFO_MULTI(SubModuleIndex, CTRL,                ctrl);
    //GET_REG_INFO_MULTI(SubModuleIndex, TOP_CTRL,            top_ctrl);
    GET_REG_INFO_MULTI(SubModuleIndex, BLND_CTRL_1,         blnd_ctrl_1);
    GET_REG_INFO_MULTI(SubModuleIndex, BLND_CTRL_2,         blnd_ctrl_2);
    GET_REG_INFO_MULTI(SubModuleIndex, CORE_CTRL,           core_ctrl);
    GET_REG_INFO_MULTI(SubModuleIndex, GN_CTRL_1,           gn_ctrl_1);
    GET_REG_INFO_MULTI(SubModuleIndex, LUMA_CTRL_1,         luma_ctrl_1);
    GET_REG_INFO_MULTI(SubModuleIndex, LUMA_CTRL_2,         luma_ctrl_2);
    GET_REG_INFO_MULTI(SubModuleIndex, LUMA_SLNK_CTRL,      luma_slnk_ctrl);
    GET_REG_INFO_MULTI(SubModuleIndex, GLUT_CTRL_1,         glut_ctrl_1);
    GET_REG_INFO_MULTI(SubModuleIndex, GLUT_CTRL_2,         glut_ctrl_2);
    GET_REG_INFO_MULTI(SubModuleIndex, GLUT_CTRL_3,         glut_ctrl_3);
    GET_REG_INFO_MULTI(SubModuleIndex, GLUT_CTRL_4,         glut_ctrl_4);
    GET_REG_INFO_MULTI(SubModuleIndex, GLUT_CTRL_5,         glut_ctrl_5);
    GET_REG_INFO_MULTI(SubModuleIndex, GLUT_CTRL_6,         glut_ctrl_6);
    GET_REG_INFO_MULTI(SubModuleIndex, ARTIFACT_CTRL,       artifact_ctrl);
    GET_REG_INFO_MULTI(SubModuleIndex, CLIP_CTRL,           clip_ctrl);
    GET_REG_INFO_MULTI(SubModuleIndex, GN_CTRL_2,           gn_ctrl_2);
    GET_REG_INFO_MULTI(SubModuleIndex, ST_CTRL_1,           st_ctrl_1);
    GET_REG_INFO_MULTI(SubModuleIndex, ST_CTRL_2,           st_ctrl_2);
    GET_REG_INFO_MULTI(SubModuleIndex, CE_CTRL,             ce_ctrl);
    GET_REG_INFO_MULTI(SubModuleIndex, CE_SL_CTRL,          ce_sl_ctrl);
    GET_REG_INFO_MULTI(SubModuleIndex, CBOOST_CTRL_1,       cboost_ctrl_1);
    GET_REG_INFO_MULTI(SubModuleIndex, CBOOST_CTRL_2,       cboost_ctrl_2);
    GET_REG_INFO_MULTI(SubModuleIndex, PBC1_CTRL_0,         pbc1_ctrl_0);
    GET_REG_INFO_MULTI(SubModuleIndex, PBC1_CTRL_1,         pbc1_ctrl_1);
    GET_REG_INFO_MULTI(SubModuleIndex, PBC1_CTRL_2,         pbc1_ctrl_2);
    GET_REG_INFO_MULTI(SubModuleIndex, PBC1_CTRL_3,         pbc1_ctrl_3);
    GET_REG_INFO_MULTI(SubModuleIndex, PBC2_CTRL_0,         pbc2_ctrl_0);
    GET_REG_INFO_MULTI(SubModuleIndex, PBC2_CTRL_1,         pbc2_ctrl_1);
    GET_REG_INFO_MULTI(SubModuleIndex, PBC2_CTRL_2,         pbc2_ctrl_2);
    GET_REG_INFO_MULTI(SubModuleIndex, PBC2_CTRL_3,         pbc2_ctrl_3);
    GET_REG_INFO_MULTI(SubModuleIndex, PBC3_CTRL_0,         pbc3_ctrl_0);
    GET_REG_INFO_MULTI(SubModuleIndex, PBC3_CTRL_1,         pbc3_ctrl_1);
    GET_REG_INFO_MULTI(SubModuleIndex, PBC3_CTRL_2,         pbc3_ctrl_2);
    GET_REG_INFO_MULTI(SubModuleIndex, PBC3_CTRL_3,         pbc3_ctrl_3);
    //GET_REG_INFO_MULTI(SubModuleIndex, SE_Y_SPECL_CTRL,     se_y_specl_ctrl);
    //GET_REG_INFO_MULTI(SubModuleIndex, SE_EDGE_CTRL_1,      se_edge_ctrl_1);
    //GET_REG_INFO_MULTI(SubModuleIndex, SE_EDGE_CTRL_2,      se_edge_ctrl_2);
    //GET_REG_INFO_MULTI(SubModuleIndex, SE_CORE_CTRL_1,      se_core_ctrl_1);
    //GET_REG_INFO_MULTI(SubModuleIndex, SE_CORE_CTRL_2,      se_core_ctrl_2);

    return  (*this);
}

MBOOL
ISP_MGR_EE_T::
apply_P2(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, dip_x_reg_t* pReg)
{
    if(SubModuleIndex >= ESubModule_NUM){
        return MFALSE;
    }

#if 0
    MBOOL bEE_EN = isEEEnable(SubModuleIndex);
    MBOOL bSE_EN = isSEEnable(SubModuleIndex);
    if(bSE_EN){
        reinterpret_cast<ISP_EE_TOP_CTRL_T*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, TOP_CTRL))->EE_OUT_EDGE_SEL = 2;
    }
    else if(bEE_EN){
        reinterpret_cast<EE_REG_D1A_EE_TOP_CTRL*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, TOP_CTRL))->Bits.EE_OUT_EDGE_SEL = 1;
    }
    else{
        reinterpret_cast<EE_REG_D1A_EE_TOP_CTRL*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, TOP_CTRL))->Bits.EE_OUT_EDGE_SEL = 0;
    }

    MBOOL bEnable = bEE_EN; // | bSE_EN;

#else
    MBOOL bEnable = isEnable(SubModuleIndex);

    if(bEnable){
        reinterpret_cast<EE_REG_D1A_EE_TOP_CTRL*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, TOP_CTRL))->Bits.EE_OUT_EDGE_SEL = 1;
    }
    else{
        reinterpret_cast<EE_REG_D1A_EE_TOP_CTRL*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, TOP_CTRL))->Bits.EE_OUT_EDGE_SEL = 0;
    }
#endif


    //Top Control
    switch (SubModuleIndex)
    {
        case EEE_D1:
            ISP_WRITE_ENABLE_BITS(pReg, DIPCTL_D1A_DIPCTL_YUV_EN1, DIPCTL_EE_D1_EN, bEnable);
            break;
        default:
            CAM_LOGE("Apply Error Submodule Index: %d", SubModuleIndex);
            return  MFALSE;
    }

    writeRegs(static_cast<RegInfo_T*>(m_rIspRegInfo[SubModuleIndex]), m_u4RegInfoNum, pReg);

    dumpRegInfoP2("EE", SubModuleIndex);

    return  MTRUE;
}

}
