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
#define LOG_TAG "isp_mgr_seee"

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

using namespace NSIspTuningv3;

#define MY_INST NS3Av3::INST_T<ISP_MGR_SEEE_T>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

namespace NSIspTuningv3
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SEEE (SE + EE)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_SEEE_T&
ISP_MGR_SEEE_T::
getInstance(ESensorDev_T const eSensorDev)
{
    auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(eSensorDev);

    MY_ASSERT(i4SensorIdx < SENSOR_IDX_MAX);
    MY_ASSERT(i4SensorIdx >= 0);


    MY_INST& rSingleton = gMultiton[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<ISP_MGR_SEEE_T>(eSensorDev);
    } );

    return *(rSingleton.instance);
}

template <>
ISP_MGR_SEEE_T&
ISP_MGR_SEEE_T::
put(ISP_NVRAM_EE_T const& rParam)
{
    PUT_REG_INFO(DIP_X_SEEE_BLND_CTRL_1  ,blnd_ctrl_1);
    PUT_REG_INFO(DIP_X_SEEE_BLND_CTRL_2  ,blnd_ctrl_2);
    PUT_REG_INFO(DIP_X_SEEE_CORE_CTRL  ,core_ctrl);
    PUT_REG_INFO(DIP_X_SEEE_GN_CTRL_1  ,gn_ctrl_1);
    PUT_REG_INFO(DIP_X_SEEE_LUMA_CTRL_1  ,luma_ctrl_1);
    PUT_REG_INFO(DIP_X_SEEE_LUMA_CTRL_2  ,luma_ctrl_2);
    PUT_REG_INFO(DIP_X_SEEE_LUMA_SLNK_CTRL  ,luma_slnk_ctrl);
    PUT_REG_INFO(DIP_X_SEEE_GLUT_CTRL_1  ,glut_ctrl_1);
    PUT_REG_INFO(DIP_X_SEEE_GLUT_CTRL_2  ,glut_ctrl_2);
    PUT_REG_INFO(DIP_X_SEEE_GLUT_CTRL_3  ,glut_ctrl_3);
    PUT_REG_INFO(DIP_X_SEEE_GLUT_CTRL_4  ,glut_ctrl_4);
    PUT_REG_INFO(DIP_X_SEEE_GLUT_CTRL_5  ,glut_ctrl_5);
    PUT_REG_INFO(DIP_X_SEEE_GLUT_CTRL_6  ,glut_ctrl_6);
    PUT_REG_INFO(DIP_X_SEEE_ARTIFACT_CTRL  ,artifact_ctrl);
    PUT_REG_INFO(DIP_X_SEEE_CLIP_CTRL  ,clip_ctrl);
    PUT_REG_INFO(DIP_X_SEEE_GN_CTRL_2  ,gn_ctrl_2);
    PUT_REG_INFO(DIP_X_SEEE_ST_CTRL_1  ,st_ctrl_1);
    PUT_REG_INFO(DIP_X_SEEE_ST_CTRL_2  ,st_ctrl_2);
    PUT_REG_INFO(DIP_X_SEEE_CT_CTRL  ,ct_ctrl);
    PUT_REG_INFO(DIP_X_SEEE_CBOOST_CTRL_1  ,cboost_ctrl_1);
    PUT_REG_INFO(DIP_X_SEEE_CBOOST_CTRL_2  ,cboost_ctrl_2);
    PUT_REG_INFO(DIP_X_SEEE_PBC1_CTRL_1  ,pbc1_ctrl_1);
    PUT_REG_INFO(DIP_X_SEEE_PBC1_CTRL_2  ,pbc1_ctrl_2);
    PUT_REG_INFO(DIP_X_SEEE_PBC1_CTRL_3  ,pbc1_ctrl_3);
    PUT_REG_INFO(DIP_X_SEEE_PBC2_CTRL_1  ,pbc2_ctrl_1);
    PUT_REG_INFO(DIP_X_SEEE_PBC2_CTRL_2  ,pbc2_ctrl_2);
    PUT_REG_INFO(DIP_X_SEEE_PBC2_CTRL_3  ,pbc2_ctrl_3);
    PUT_REG_INFO(DIP_X_SEEE_PBC3_CTRL_1  ,pbc3_ctrl_1);
    PUT_REG_INFO(DIP_X_SEEE_PBC3_CTRL_2  ,pbc3_ctrl_2);
    PUT_REG_INFO(DIP_X_SEEE_PBC3_CTRL_3  ,pbc3_ctrl_3);
    return  (*this);
}

template <>
ISP_MGR_SEEE_T&
ISP_MGR_SEEE_T::
get(ISP_NVRAM_EE_T& rParam)
{
    GET_REG_INFO(DIP_X_SEEE_BLND_CTRL_1  ,blnd_ctrl_1);
    GET_REG_INFO(DIP_X_SEEE_BLND_CTRL_2  ,blnd_ctrl_2);
    GET_REG_INFO(DIP_X_SEEE_CORE_CTRL  ,core_ctrl);
    GET_REG_INFO(DIP_X_SEEE_GN_CTRL_1  ,gn_ctrl_1);
    GET_REG_INFO(DIP_X_SEEE_LUMA_CTRL_1  ,luma_ctrl_1);
    GET_REG_INFO(DIP_X_SEEE_LUMA_CTRL_2  ,luma_ctrl_2);
    GET_REG_INFO(DIP_X_SEEE_LUMA_SLNK_CTRL  ,luma_slnk_ctrl);
    GET_REG_INFO(DIP_X_SEEE_GLUT_CTRL_1  ,glut_ctrl_1);
    GET_REG_INFO(DIP_X_SEEE_GLUT_CTRL_2  ,glut_ctrl_2);
    GET_REG_INFO(DIP_X_SEEE_GLUT_CTRL_3  ,glut_ctrl_3);
    GET_REG_INFO(DIP_X_SEEE_GLUT_CTRL_4  ,glut_ctrl_4);
    GET_REG_INFO(DIP_X_SEEE_GLUT_CTRL_5  ,glut_ctrl_5);
    GET_REG_INFO(DIP_X_SEEE_GLUT_CTRL_6  ,glut_ctrl_6);
    GET_REG_INFO(DIP_X_SEEE_ARTIFACT_CTRL  ,artifact_ctrl);
    GET_REG_INFO(DIP_X_SEEE_CLIP_CTRL  ,clip_ctrl);
    GET_REG_INFO(DIP_X_SEEE_GN_CTRL_2  ,gn_ctrl_2);
    GET_REG_INFO(DIP_X_SEEE_ST_CTRL_1  ,st_ctrl_1);
    GET_REG_INFO(DIP_X_SEEE_ST_CTRL_2  ,st_ctrl_2);
    GET_REG_INFO(DIP_X_SEEE_CT_CTRL  ,ct_ctrl);
    GET_REG_INFO(DIP_X_SEEE_CBOOST_CTRL_1  ,cboost_ctrl_1);
    GET_REG_INFO(DIP_X_SEEE_CBOOST_CTRL_2  ,cboost_ctrl_2);
    GET_REG_INFO(DIP_X_SEEE_PBC1_CTRL_1  ,pbc1_ctrl_1);
    GET_REG_INFO(DIP_X_SEEE_PBC1_CTRL_2  ,pbc1_ctrl_2);
    GET_REG_INFO(DIP_X_SEEE_PBC1_CTRL_3  ,pbc1_ctrl_3);
    GET_REG_INFO(DIP_X_SEEE_PBC2_CTRL_1  ,pbc2_ctrl_1);
    GET_REG_INFO(DIP_X_SEEE_PBC2_CTRL_2  ,pbc2_ctrl_2);
    GET_REG_INFO(DIP_X_SEEE_PBC2_CTRL_3  ,pbc2_ctrl_3);
    GET_REG_INFO(DIP_X_SEEE_PBC3_CTRL_1  ,pbc3_ctrl_1);
    GET_REG_INFO(DIP_X_SEEE_PBC3_CTRL_2  ,pbc3_ctrl_2);
    GET_REG_INFO(DIP_X_SEEE_PBC3_CTRL_3  ,pbc3_ctrl_3);

    return  (*this);
}

template <>
MBOOL
ISP_MGR_SEEE_T::
get(ISP_NVRAM_EE_T& rParam, const dip_x_reg_t* pReg) const
{
    if (pReg)
    {
        GET_REG_INFO_BUF(DIP_X_SEEE_BLND_CTRL_1  ,blnd_ctrl_1);
        GET_REG_INFO_BUF(DIP_X_SEEE_BLND_CTRL_2  ,blnd_ctrl_2);
        GET_REG_INFO_BUF(DIP_X_SEEE_CORE_CTRL  ,core_ctrl);
        GET_REG_INFO_BUF(DIP_X_SEEE_GN_CTRL_1  ,gn_ctrl_1);
        GET_REG_INFO_BUF(DIP_X_SEEE_LUMA_CTRL_1  ,luma_ctrl_1);
        GET_REG_INFO_BUF(DIP_X_SEEE_LUMA_CTRL_2  ,luma_ctrl_2);
        GET_REG_INFO_BUF(DIP_X_SEEE_LUMA_SLNK_CTRL  ,luma_slnk_ctrl);
        GET_REG_INFO_BUF(DIP_X_SEEE_GLUT_CTRL_1  ,glut_ctrl_1);
        GET_REG_INFO_BUF(DIP_X_SEEE_GLUT_CTRL_2  ,glut_ctrl_2);
        GET_REG_INFO_BUF(DIP_X_SEEE_GLUT_CTRL_3  ,glut_ctrl_3);
        GET_REG_INFO_BUF(DIP_X_SEEE_GLUT_CTRL_4  ,glut_ctrl_4);
        GET_REG_INFO_BUF(DIP_X_SEEE_GLUT_CTRL_5  ,glut_ctrl_5);
        GET_REG_INFO_BUF(DIP_X_SEEE_GLUT_CTRL_6  ,glut_ctrl_6);
        GET_REG_INFO_BUF(DIP_X_SEEE_ARTIFACT_CTRL  ,artifact_ctrl);
        GET_REG_INFO_BUF(DIP_X_SEEE_CLIP_CTRL  ,clip_ctrl);
        GET_REG_INFO_BUF(DIP_X_SEEE_GN_CTRL_2  ,gn_ctrl_2);
        GET_REG_INFO_BUF(DIP_X_SEEE_ST_CTRL_1  ,st_ctrl_1);
        GET_REG_INFO_BUF(DIP_X_SEEE_ST_CTRL_2  ,st_ctrl_2);
        GET_REG_INFO_BUF(DIP_X_SEEE_CT_CTRL  ,ct_ctrl);
        GET_REG_INFO_BUF(DIP_X_SEEE_CBOOST_CTRL_1  ,cboost_ctrl_1);
        GET_REG_INFO_BUF(DIP_X_SEEE_CBOOST_CTRL_2  ,cboost_ctrl_2);
        GET_REG_INFO_BUF(DIP_X_SEEE_PBC1_CTRL_1  ,pbc1_ctrl_1);
        GET_REG_INFO_BUF(DIP_X_SEEE_PBC1_CTRL_2  ,pbc1_ctrl_2);
        GET_REG_INFO_BUF(DIP_X_SEEE_PBC1_CTRL_3  ,pbc1_ctrl_3);
        GET_REG_INFO_BUF(DIP_X_SEEE_PBC2_CTRL_1  ,pbc2_ctrl_1);
        GET_REG_INFO_BUF(DIP_X_SEEE_PBC2_CTRL_2  ,pbc2_ctrl_2);
        GET_REG_INFO_BUF(DIP_X_SEEE_PBC2_CTRL_3  ,pbc2_ctrl_3);
        GET_REG_INFO_BUF(DIP_X_SEEE_PBC3_CTRL_1  ,pbc3_ctrl_1);
        GET_REG_INFO_BUF(DIP_X_SEEE_PBC3_CTRL_2  ,pbc3_ctrl_2);
        GET_REG_INFO_BUF(DIP_X_SEEE_PBC3_CTRL_3  ,pbc3_ctrl_3);
    }

    return MTRUE;
}

template <>
ISP_MGR_SEEE_T&
ISP_MGR_SEEE_T::
put(ISP_NVRAM_SE_T const& rParam)
{

    PUT_REG_INFO(DIP_X_SEEE_TOP_CTRL ,top_ctrl);
    PUT_REG_INFO(DIP_X_SEEE_SE_Y_SPECL_CTRL ,se_y_specl_ctrl);
    PUT_REG_INFO(DIP_X_SEEE_SE_EDGE_CTRL_1 ,se_edge_ctrl_1);
    PUT_REG_INFO(DIP_X_SEEE_SE_EDGE_CTRL_2 ,se_edge_ctrl_2);
    PUT_REG_INFO(DIP_X_SEEE_SE_CORE_CTRL_1 ,se_core_ctrl_1);
    PUT_REG_INFO(DIP_X_SEEE_SE_CORE_CTRL_2 ,se_core_ctrl_2);
    return  (*this);
}

template <>
ISP_MGR_SEEE_T&
ISP_MGR_SEEE_T::
get(ISP_NVRAM_SE_T& rParam)
{
    GET_REG_INFO(DIP_X_SEEE_TOP_CTRL ,top_ctrl);
    GET_REG_INFO(DIP_X_SEEE_SE_Y_SPECL_CTRL ,se_y_specl_ctrl);
    GET_REG_INFO(DIP_X_SEEE_SE_EDGE_CTRL_1 ,se_edge_ctrl_1);
    GET_REG_INFO(DIP_X_SEEE_SE_EDGE_CTRL_2 ,se_edge_ctrl_2);
    GET_REG_INFO(DIP_X_SEEE_SE_CORE_CTRL_1 ,se_core_ctrl_1);
    GET_REG_INFO(DIP_X_SEEE_SE_CORE_CTRL_2 ,se_core_ctrl_2);
    return  (*this);
}

template <>
MBOOL
ISP_MGR_SEEE_T::
get(ISP_NVRAM_SE_T& rParam, const dip_x_reg_t* pReg) const
{
    if (pReg)
    {
        GET_REG_INFO_BUF(DIP_X_SEEE_TOP_CTRL ,top_ctrl);
        GET_REG_INFO_BUF(DIP_X_SEEE_SE_Y_SPECL_CTRL ,se_y_specl_ctrl);
        GET_REG_INFO_BUF(DIP_X_SEEE_SE_EDGE_CTRL_1 ,se_edge_ctrl_1);
        GET_REG_INFO_BUF(DIP_X_SEEE_SE_EDGE_CTRL_2 ,se_edge_ctrl_2);
        GET_REG_INFO_BUF(DIP_X_SEEE_SE_CORE_CTRL_1 ,se_core_ctrl_1);
        GET_REG_INFO_BUF(DIP_X_SEEE_SE_CORE_CTRL_2 ,se_core_ctrl_2);
    }
    return MTRUE;
}

MBOOL
ISP_MGR_SEEE_T::
apply(const RAWIspCamInfo& rRawIspCamInfo, dip_x_reg_t* pReg)
{
    MBOOL bEE_EN = isEEEnable();
    MBOOL bSE_EN = isSEEnable();

    if(bSE_EN){
        reinterpret_cast<ISP_DIP_X_SEEE_TOP_CTRL_T*>(REG_INFO_VALUE_PTR(DIP_X_SEEE_TOP_CTRL))->SEEE_OUT_EDGE_SEL = 2;
    }
    else if(bEE_EN){
        reinterpret_cast<ISP_DIP_X_SEEE_TOP_CTRL_T*>(REG_INFO_VALUE_PTR(DIP_X_SEEE_TOP_CTRL))->SEEE_OUT_EDGE_SEL = 1;
    }
    else{
        reinterpret_cast<ISP_DIP_X_SEEE_TOP_CTRL_T*>(REG_INFO_VALUE_PTR(DIP_X_SEEE_TOP_CTRL))->SEEE_OUT_EDGE_SEL = 0;
    }

    MBOOL bSEEE_EN = bEE_EN | bSE_EN;

    if( (!bEE_EN) && (rRawIspCamInfo.eIdx_Effect == 0)){
        bSEEE_EN = MFALSE;
    }

    // TOP
    ISP_WRITE_ENABLE_BITS(pReg, DIP_X_CTL_YUV_EN, SEEE_EN, bSEEE_EN);
    //ISP_MGR_CTL_EN_P2_T::getInstance(m_eSensorDev).setEnable_SEEE(bSEEE_EN);

    writeRegs(static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum, pReg);

    dumpRegInfo("SEEE");

    return  MTRUE;
}

}
