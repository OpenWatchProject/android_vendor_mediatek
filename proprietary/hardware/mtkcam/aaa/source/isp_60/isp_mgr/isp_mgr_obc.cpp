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
#define LOG_TAG "isp_mgr_obc"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include "isp_mgr.h"
#include "isp_mgr_helper.h"
#include <private/aaa_utils.h>
#include <array>

using namespace NSIspTuning;

#define MY_INST NS3Av3::INST_T<ISP_MGR_OBC_T>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

namespace NSIspTuning
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  OBC
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_OBC_T&
ISP_MGR_OBC_T::
getInstance(MUINT32 const eSensorDev)
{
    ISP_MGR_MODULE_GET_INSTANCE(OBC);
}

template <>
ISP_MGR_OBC_T&
ISP_MGR_OBC_T::
put(MUINT8 SubModuleIndex, ISP_NVRAM_OBC_T const& rParam)
{
    //PUT_REG_INFO_MULTI(SubModuleIndex, CTL,                    ctl);
    PUT_REG_INFO_MULTI(SubModuleIndex, DBS,                    dbs);
    PUT_REG_INFO_MULTI(SubModuleIndex, GRAY_BLD_0,             gray_bld_0);
    PUT_REG_INFO_MULTI(SubModuleIndex, GRAY_BLD_1,             gray_bld_1);
    //PUT_REG_INFO_MULTI(SubModuleIndex, GRAY_BLD_2,             gray_bld_2);
    PUT_REG_INFO_MULTI(SubModuleIndex, BIAS_LUT_R0,            bias_lut_r0);
    PUT_REG_INFO_MULTI(SubModuleIndex, BIAS_LUT_R1,            bias_lut_r1);
    PUT_REG_INFO_MULTI(SubModuleIndex, BIAS_LUT_R2,            bias_lut_r2);
    PUT_REG_INFO_MULTI(SubModuleIndex, BIAS_LUT_R3,            bias_lut_r3);
    PUT_REG_INFO_MULTI(SubModuleIndex, BIAS_LUT_G0,            bias_lut_g0);
    PUT_REG_INFO_MULTI(SubModuleIndex, BIAS_LUT_G1,            bias_lut_g1);
    PUT_REG_INFO_MULTI(SubModuleIndex, BIAS_LUT_G2,            bias_lut_g2);
    PUT_REG_INFO_MULTI(SubModuleIndex, BIAS_LUT_G3,            bias_lut_g3);
    PUT_REG_INFO_MULTI(SubModuleIndex, BIAS_LUT_B0,            bias_lut_b0);
    PUT_REG_INFO_MULTI(SubModuleIndex, BIAS_LUT_B1,            bias_lut_b1);
    PUT_REG_INFO_MULTI(SubModuleIndex, BIAS_LUT_B2,            bias_lut_b2);
    PUT_REG_INFO_MULTI(SubModuleIndex, BIAS_LUT_B3,            bias_lut_b3);
    //PUT_REG_INFO_MULTI(SubModuleIndex, WBG_RB,                 wbg_rb);
    //PUT_REG_INFO_MULTI(SubModuleIndex, WBG_G,                  wbg_g);
    //PUT_REG_INFO_MULTI(SubModuleIndex, WBIG_RB,                wbig_rb);
    //PUT_REG_INFO_MULTI(SubModuleIndex, WBIG_G,                 wbig_g);
    //PUT_REG_INFO_MULTI(SubModuleIndex, OBG_RB,                 obg_rb);
    //PUT_REG_INFO_MULTI(SubModuleIndex, OBG_G,                  obg_g);
    PUT_REG_INFO_MULTI(SubModuleIndex, OFFSET_R,               offset_r);
    PUT_REG_INFO_MULTI(SubModuleIndex, OFFSET_GR,              offset_gr);
    PUT_REG_INFO_MULTI(SubModuleIndex, OFFSET_GB,              offset_gb);
    PUT_REG_INFO_MULTI(SubModuleIndex, OFFSET_B,               offset_b);
    //PUT_REG_INFO_MULTI(SubModuleIndex, HDR,                    hdr);


    return  (*this);
}


template <>
ISP_MGR_OBC_T&
ISP_MGR_OBC_T::
get(MUINT8 SubModuleIndex, ISP_NVRAM_OBC_T& rParam)
{
    //GET_REG_INFO_MULTI(SubModuleIndex, CTL,                    ctl);
    GET_REG_INFO_MULTI(SubModuleIndex, DBS,                    dbs);
    GET_REG_INFO_MULTI(SubModuleIndex, GRAY_BLD_0,             gray_bld_0);
    GET_REG_INFO_MULTI(SubModuleIndex, GRAY_BLD_1,             gray_bld_1);
    //GET_REG_INFO_MULTI(SubModuleIndex, GRAY_BLD_2,             gray_bld_2);
    GET_REG_INFO_MULTI(SubModuleIndex, BIAS_LUT_R0,            bias_lut_r0);
    GET_REG_INFO_MULTI(SubModuleIndex, BIAS_LUT_R1,            bias_lut_r1);
    GET_REG_INFO_MULTI(SubModuleIndex, BIAS_LUT_R2,            bias_lut_r2);
    GET_REG_INFO_MULTI(SubModuleIndex, BIAS_LUT_R3,            bias_lut_r3);
    GET_REG_INFO_MULTI(SubModuleIndex, BIAS_LUT_G0,            bias_lut_g0);
    GET_REG_INFO_MULTI(SubModuleIndex, BIAS_LUT_G1,            bias_lut_g1);
    GET_REG_INFO_MULTI(SubModuleIndex, BIAS_LUT_G2,            bias_lut_g2);
    GET_REG_INFO_MULTI(SubModuleIndex, BIAS_LUT_G3,            bias_lut_g3);
    GET_REG_INFO_MULTI(SubModuleIndex, BIAS_LUT_B0,            bias_lut_b0);
    GET_REG_INFO_MULTI(SubModuleIndex, BIAS_LUT_B1,            bias_lut_b1);
    GET_REG_INFO_MULTI(SubModuleIndex, BIAS_LUT_B2,            bias_lut_b2);
    GET_REG_INFO_MULTI(SubModuleIndex, BIAS_LUT_B3,            bias_lut_b3);
    //GET_REG_INFO_MULTI(SubModuleIndex, WBG_RB,                 wbg_rb);
    //GET_REG_INFO_MULTI(SubModuleIndex, WBG_G,                  wbg_g);
    //GET_REG_INFO_MULTI(SubModuleIndex, WBIG_RB,                wbig_rb);
    //GET_REG_INFO_MULTI(SubModuleIndex, WBIG_G,                 wbig_g);
    //GET_REG_INFO_MULTI(SubModuleIndex, OBG_RB,                 obg_rb);
    //GET_REG_INFO_MULTI(SubModuleIndex, OBG_G,                  obg_g);
    GET_REG_INFO_MULTI(SubModuleIndex, OFFSET_R,               offset_r);
    GET_REG_INFO_MULTI(SubModuleIndex, OFFSET_GR,              offset_gr);
    GET_REG_INFO_MULTI(SubModuleIndex, OFFSET_GB,              offset_gb);
    GET_REG_INFO_MULTI(SubModuleIndex, OFFSET_B,               offset_b);
    //GET_REG_INFO_MULTI(SubModuleIndex, HDR,                    hdr);

    return  (*this);
}

MBOOL
ISP_MGR_OBC_T::
apply_P1(MUINT8 SubModuleIndex, RAWIspCamInfo& rRawIspCamInfo, TuningMgr& rTuning, MINT32 i4SubsampleIdex)
{
    if(SubModuleIndex >= ESubModule_NUM){
        return MFALSE;
    }

    MBOOL bEnable = isEnable(SubModuleIndex);

    //Top Control
    switch (SubModuleIndex)
    {
        case EOBC_R1:
            ISP_MGR_CAMCTL_T::getInstance(m_eSensorDev).setEnable_OBC_R1(bEnable);
            rTuning.updateEngine(eTuningMgrFunc_OBC_R1, bEnable, i4SubsampleIdex);
            break;
#if 0
        case EOBC_R2:
            ISP_MGR_CAMCTL_T::getInstance(m_eSensorDev).setEnable_OBC_R2(bEnable);
            rTuning.updateEngine(eTuningMgrFunc_OBC, bEnable, i4SubsampleIdex);
            break;
#endif
        default:
            CAM_LOGE("Apply Error Submodule Index: %d", SubModuleIndex);
            return  MFALSE;
    }

    if(bEnable){
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("vendor.isp.obc.fakeofst", value, "0"); // 0: enable, 1: disable
        MINT32 i4FakeOfst = atoi(value);

        if(i4FakeOfst < 0){
            reinterpret_cast<REG_OBC_R1_OBC_OFFSET_R*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, OFFSET_R))->Bits.OBC_OFST_R = i4FakeOfst;
            reinterpret_cast<REG_OBC_R1_OBC_OFFSET_B*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, OFFSET_B))->Bits.OBC_OFST_B = i4FakeOfst;
            reinterpret_cast<REG_OBC_R1_OBC_OFFSET_GR*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, OFFSET_GR))->Bits.OBC_OFST_GR = i4FakeOfst;
            reinterpret_cast<REG_OBC_R1_OBC_OFFSET_GB*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, OFFSET_GB))->Bits.OBC_OFST_GB = i4FakeOfst;
            CAM_LOGE("Chooo, OFST:%d, adb:%d", reinterpret_cast<REG_OBC_R1_OBC_OFFSET_R*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, OFFSET_R))->Bits.OBC_OFST_R , i4FakeOfst);
        }
        //turn off DBS for debug
        property_get("vendor.isp.obc_dbs.disable", value, "0"); // 0: enable, 1: disable
        MBOOL bDisableDBS = atoi(value);
        if(bDisableDBS){
            reinterpret_cast<REG_OBC_R1_OBC_DBS*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, DBS))->Bits.OBC_DBS_RATIO = 0;
            reinterpret_cast<REG_OBC_R1_OBC_DBS*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, DBS))->Bits.OBC_POSTTUNE_EN = 0;
        }

        //WB
        reinterpret_cast<REG_OBC_R1_OBC_WBG_RB*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, WBG_RB))->Bits.OBC_PGN_R = rRawIspCamInfo.rAWBInfo.rRPG.i4R;
        reinterpret_cast<REG_OBC_R1_OBC_WBG_RB*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, WBG_RB))->Bits.OBC_PGN_B = rRawIspCamInfo.rAWBInfo.rRPG.i4B;
        reinterpret_cast<REG_OBC_R1_OBC_WBG_G*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, WBG_G))->Bits.OBC_PGN_G = rRawIspCamInfo.rAWBInfo.rRPG.i4G;
        //WB Inverse Gain
        reinterpret_cast<REG_OBC_R1_OBC_WBIG_RB*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, WBIG_RB))->Bits.OBC_IVGN_R = (((1 << 19) / rRawIspCamInfo.rAWBInfo.rRPG.i4R) + 1 ) >> 1;
        reinterpret_cast<REG_OBC_R1_OBC_WBIG_RB*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, WBIG_RB))->Bits.OBC_IVGN_B = (((1 << 19) / rRawIspCamInfo.rAWBInfo.rRPG.i4B) + 1 ) >> 1;
        reinterpret_cast<REG_OBC_R1_OBC_WBIG_G*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, WBIG_G))->Bits.OBC_IVGN_G   = (((1 << 19) / rRawIspCamInfo.rAWBInfo.rRPG.i4G) + 1 ) >> 1;
        //OBC Gain
        float fOB_OFST_R= -(float)(0x3FFFFF ^ (reinterpret_cast<REG_OBC_R1_OBC_OFFSET_R*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, OFFSET_R))->Bits.OBC_OFST_R -1));
        float fOB_OFST_B= -(float)(0x3FFFFF ^ (reinterpret_cast<REG_OBC_R1_OBC_OFFSET_B*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, OFFSET_B))->Bits.OBC_OFST_B -1));
        float fOB_OFST_GR= -(float)(0x3FFFFF ^ (reinterpret_cast<REG_OBC_R1_OBC_OFFSET_GR*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, OFFSET_GR))->Bits.OBC_OFST_GR -1));
        float fOB_OFST_GB= -(float)(0x3FFFFF ^ (reinterpret_cast<REG_OBC_R1_OBC_OFFSET_GB*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, OFFSET_GB))->Bits.OBC_OFST_GB -1));

        if(fOB_OFST_R < -511.0f){
            CAM_LOGE("fOBC_OFST_R (%d) Error (< -511)", fOB_OFST_R);
            fOB_OFST_R = -511.0f;
        }
        else if(fOB_OFST_R > 0.0f){
            CAM_LOGE("fOBC_OFST_R (%d) Error (> 0)", fOB_OFST_R);
            fOB_OFST_R = 0.0f;
        }

        if(fOB_OFST_B < -511.0f){
            CAM_LOGE("fOBC_OFST_B (%d) Error (< -511)", fOB_OFST_B);
            fOB_OFST_B = -511.0f;
        }
        else if(fOB_OFST_B > 0.0f){
            CAM_LOGE("fOBC_OFST_B (%d) Error (> 0)", fOB_OFST_B);
            fOB_OFST_B = 0.0f;
        }

        if(fOB_OFST_GR < -511.0f){
            CAM_LOGE("fOBC_OFST_GR (%d) Error (< -511)", fOB_OFST_GR);
            fOB_OFST_GR = -511.0f;
        }
        else if(fOB_OFST_GR > 0.0f){
            CAM_LOGE("fOBC_OFST_GR (%d) Error (> 0)", fOB_OFST_GR);
            fOB_OFST_GR = 0.0f;
        }

        if(fOB_OFST_GB < -511.0f){
            CAM_LOGE("fOBC_OFST_GB (%d) Error (< -511)", fOB_OFST_GB);
            fOB_OFST_GB = -511.0f;
        }
        else if(fOB_OFST_GB > 0.0f){
            CAM_LOGE("fOBC_OFST_GB (%d) Error (> 0)", fOB_OFST_GB);
            fOB_OFST_GB = 0.0f;
        }

        reinterpret_cast<REG_OBC_R1_OBC_OBG_RB*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, OBG_RB))->Bits.OBC_GAIN_R =
            ((512.0f * 4095.0f)/(4095.0f + fOB_OFST_R) + 0.5f);
        reinterpret_cast<REG_OBC_R1_OBC_OBG_RB*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, OBG_RB))->Bits.OBC_GAIN_B =
            ((512.0f * 4095.0f)/(4095.0f + fOB_OFST_B) + 0.5f);
        reinterpret_cast<REG_OBC_R1_OBC_OBG_G*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, OBG_G))->Bits.OBC_GAIN_GR =
            ((512.0f * 4095.0f)/(4095.0f + fOB_OFST_GR) + 0.5f);
        reinterpret_cast<REG_OBC_R1_OBC_OBG_G*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, OBG_G))->Bits.OBC_GAIN_GB =
            ((512.0f * 4095.0f)/(4095.0f + fOB_OFST_GB) + 0.5f);

        rRawIspCamInfo.rOBC_OFST[0] = (MINT32)fOB_OFST_R;
        rRawIspCamInfo.rOBC_OFST[1] = (MINT32)fOB_OFST_GR;
        rRawIspCamInfo.rOBC_OFST[2] = (MINT32)fOB_OFST_GB;
        rRawIspCamInfo.rOBC_OFST[3] = (MINT32)fOB_OFST_B;


        MBOOL zHDREnable = MFALSE;
        MINT32 HDR_RATIO = 16; //ISP HDR Ratio Base: 16
#if 0
        //Chooo, not support zHDR. so ZFUS is always off
        if(zHDREnable){
            HDR_RATIO = (rRawIspCamInfo.rAEInfo.i4LESE_Ratio * 16 + 50) / 100; //only focus on zHDR
        }
#endif
        reinterpret_cast<REG_OBC_R1_OBC_HDR*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, HDR))->Bits.OBC_ZHDR_EN = zHDREnable;
        reinterpret_cast<REG_OBC_R1_OBC_HDR*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, HDR))->Bits.OBC_HDR_GNP = GetHDRGNP(HDR_RATIO>>4);
        reinterpret_cast<REG_OBC_R1_OBC_HDR*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, HDR))->Bits.OBC_GLE_FIRST = 0; //ZFUS_R1A_ZHDR_GLE_FIRST
        reinterpret_cast<REG_OBC_R1_OBC_HDR*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, HDR))->Bits.OBC_RLE_FIRST = 0; //ZFUS_R1A_ZHDR_RLE_FIRST
        reinterpret_cast<REG_OBC_R1_OBC_HDR*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, HDR))->Bits.OBC_BLE_FIRST = 0; //ZFUS_R1A_ZHDR_BLE_FIRST

        reinterpret_cast<REG_OBC_R1_OBC_GRAY_BLD_2*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, GRAY_BLD_2))->Bits.OBC_BLD_MXRT_SE =
            reinterpret_cast<REG_OBC_R1_OBC_GRAY_BLD_1*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, GRAY_BLD_1))->Bits.OBC_BLD_MXRT;
        reinterpret_cast<REG_OBC_R1_OBC_GRAY_BLD_2*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, GRAY_BLD_2))->Bits.OBC_BLD_LOW_SE =
            (((reinterpret_cast<REG_OBC_R1_OBC_GRAY_BLD_1*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, GRAY_BLD_1))->Bits.OBC_BLD_LOW << 5) / (HDR_RATIO +1)) >>1);
        reinterpret_cast<REG_OBC_R1_OBC_GRAY_BLD_2*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, GRAY_BLD_2))->Bits.OBC_BLD_SLP_SE =
            ((reinterpret_cast<REG_OBC_R1_OBC_GRAY_BLD_1*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, GRAY_BLD_1))->Bits.OBC_BLD_SLP * HDR_RATIO + 8) >> 4);

    }

    AAA_TRACE_DRV(DRV_OBC);
    rTuning.tuningMgrWriteRegs( (TUNING_MGR_REG_IO_STRUCT*)(&(m_rIspRegInfo[SubModuleIndex][0])),
            m_u4RegInfoNum, i4SubsampleIdex);
    AAA_TRACE_END_DRV;

    dumpRegInfoP1("OBC", SubModuleIndex);

    return  MTRUE;
}



MBOOL
ISP_MGR_OBC_T::
apply_P2(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, dip_x_reg_t* pReg)
{
    if(SubModuleIndex >= ESubModule_NUM){
        return MFALSE;
    }
    MBOOL bEnable = isEnable(SubModuleIndex);

    //Top Control
    switch (SubModuleIndex)
    {
        case EOBC_D1:
            ISP_WRITE_ENABLE_BITS(pReg, DIPCTL_D1A_DIPCTL_RGB_EN1, DIPCTL_OBC_D1_EN, bEnable);
            break;
        default:
            CAM_LOGE("Apply Error Submodule Index: %d", SubModuleIndex);
            return  MFALSE;
    }

    if(bEnable){
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("vendor.isp.obc.fakeofst", value, "0"); // 0: enable, 1: disable
        MINT32 i4FakeOfst = atoi(value);

        if(i4FakeOfst < 0){
            reinterpret_cast<OBC_REG_D1A_OBC_OFFSET_R*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, OFFSET_R))->Bits.OBC_OFST_R = i4FakeOfst;
            reinterpret_cast<OBC_REG_D1A_OBC_OFFSET_B*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, OFFSET_B))->Bits.OBC_OFST_B = i4FakeOfst;
            reinterpret_cast<OBC_REG_D1A_OBC_OFFSET_GR*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, OFFSET_GR))->Bits.OBC_OFST_GR = i4FakeOfst;
            reinterpret_cast<OBC_REG_D1A_OBC_OFFSET_GB*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, OFFSET_GB))->Bits.OBC_OFST_GB = i4FakeOfst;
            CAM_LOGE("Chooo, OFST:%d, adb:%d", reinterpret_cast<OBC_REG_D1A_OBC_OFFSET_R*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, OFFSET_R))->Bits.OBC_OFST_R , i4FakeOfst);
        }
        //turn off DBS for debug
        property_get("vendor.isp.obc_dbs.disable", value, "0"); // 0: enable, 1: disable
        MBOOL bDisableDBS = atoi(value);
        if(bDisableDBS){
            reinterpret_cast<OBC_REG_D1A_OBC_DBS*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, DBS))->Bits.OBC_DBS_RATIO = 0;
            reinterpret_cast<OBC_REG_D1A_OBC_DBS*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, DBS))->Bits.OBC_POSTTUNE_EN = 0;
        }

        //WB
        reinterpret_cast<OBC_REG_D1A_OBC_WBG_RB*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, WBG_RB))->Bits.OBC_PGN_R = rRawIspCamInfo.rAWBInfo.rPGN.i4R;
        reinterpret_cast<OBC_REG_D1A_OBC_WBG_RB*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, WBG_RB))->Bits.OBC_PGN_B = rRawIspCamInfo.rAWBInfo.rPGN.i4B;
        reinterpret_cast<OBC_REG_D1A_OBC_WBG_G*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, WBG_G))->Bits.OBC_PGN_G   = rRawIspCamInfo.rAWBInfo.rPGN.i4G;
        //WB Inverse Gain
        reinterpret_cast<OBC_REG_D1A_OBC_WBIG_RB*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, WBIG_RB))->Bits.OBC_IVGN_R = (((1 << 19) / rRawIspCamInfo.rAWBInfo.rPGN.i4R) + 1 ) >> 1;
        reinterpret_cast<OBC_REG_D1A_OBC_WBIG_RB*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, WBIG_RB))->Bits.OBC_IVGN_B = (((1 << 19) / rRawIspCamInfo.rAWBInfo.rPGN.i4B) + 1 ) >> 1;
        reinterpret_cast<OBC_REG_D1A_OBC_WBIG_G*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, WBIG_G))->Bits.OBC_IVGN_G   = (((1 << 19) / rRawIspCamInfo.rAWBInfo.rPGN.i4G) + 1 ) >> 1;

        //OBC Gain
        float fOB_OFST_R= -(float)(0x3FFFFF ^ (reinterpret_cast<OBC_REG_D1A_OBC_OFFSET_R*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, OFFSET_R))->Bits.OBC_OFST_R -1));
        float fOB_OFST_B= -(float)(0x3FFFFF ^ (reinterpret_cast<OBC_REG_D1A_OBC_OFFSET_B*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, OFFSET_B))->Bits.OBC_OFST_B -1));
        float fOB_OFST_GR= -(float)(0x3FFFFF ^ (reinterpret_cast<OBC_REG_D1A_OBC_OFFSET_GR*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, OFFSET_GR))->Bits.OBC_OFST_GR -1));
        float fOB_OFST_GB= -(float)(0x3FFFFF ^ (reinterpret_cast<OBC_REG_D1A_OBC_OFFSET_GB*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, OFFSET_GB))->Bits.OBC_OFST_GB -1));

        if(fOB_OFST_R < -511.0f){
            CAM_LOGE("fOBC_OFST_R (%d) Error (< -511)", fOB_OFST_R);
            fOB_OFST_R = -511.0f;
        }
        else if(fOB_OFST_R > 0.0f){
            CAM_LOGE("fOBC_OFST_R (%d) Error (> 0)", fOB_OFST_R);
            fOB_OFST_R = 0.0f;
        }

        if(fOB_OFST_B < -511.0f){
            CAM_LOGE("fOBC_OFST_B (%d) Error (< -511)", fOB_OFST_B);
            fOB_OFST_B = -511.0f;
        }
        else if(fOB_OFST_B > 0.0f){
            CAM_LOGE("fOBC_OFST_B (%d) Error (> 0)", fOB_OFST_B);
            fOB_OFST_B = 0.0f;
        }

        if(fOB_OFST_GR < -511.0f){
            CAM_LOGE("fOBC_OFST_GR (%d) Error (< -511)", fOB_OFST_GR);
            fOB_OFST_GR = -511.0f;
        }
        else if(fOB_OFST_GR > 0.0f){
            CAM_LOGE("fOBC_OFST_GR (%d) Error (> 0)", fOB_OFST_GR);
            fOB_OFST_GR = 0.0f;
        }

        if(fOB_OFST_GB < -511.0f){
            CAM_LOGE("fOBC_OFST_GB (%d) Error (< -511)", fOB_OFST_GB);
            fOB_OFST_GB = -511.0f;
        }
        else if(fOB_OFST_GB > 0.0f){
            CAM_LOGE("fOBC_OFST_GB (%d) Error (> 0)", fOB_OFST_GB);
            fOB_OFST_GB = 0.0f;
        }

        reinterpret_cast<OBC_REG_D1A_OBC_OBG_RB*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, OBG_RB))->Bits.OBC_GAIN_R =
            ((512.0f * 4095.0f)/(4095.0f + fOB_OFST_R) + 0.5f);
        reinterpret_cast<OBC_REG_D1A_OBC_OBG_RB*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, OBG_RB))->Bits.OBC_GAIN_B =
            ((512.0f * 4095.0f)/(4095.0f + fOB_OFST_B) + 0.5f);
        reinterpret_cast<OBC_REG_D1A_OBC_OBG_G*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, OBG_G))->Bits.OBC_GAIN_GR =
            ((512.0f * 4095.0f)/(4095.0f + fOB_OFST_GR) + 0.5f);
        reinterpret_cast<OBC_REG_D1A_OBC_OBG_G*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, OBG_G))->Bits.OBC_GAIN_GB =
            ((512.0f * 4095.0f)/(4095.0f + fOB_OFST_GB) + 0.5f);

        MBOOL zHDREnable = MFALSE;
        MINT32 HDR_RATIO = 16; //ISP HDR Ratio Base: 16
#if 0
        //Chooo, not support zHDR. so ZFUS is always off

        if(zHDREnable){
            HDR_RATIO = (rRawIspCamInfo.rAEInfo.i4LESE_Ratio * 16 + 50) / 100; //only focus on zHDR
        }
#endif
        reinterpret_cast<OBC_REG_D1A_OBC_HDR*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, HDR))->Bits.OBC_ZHDR_EN = zHDREnable;
        reinterpret_cast<OBC_REG_D1A_OBC_HDR*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, HDR))->Bits.OBC_HDR_GNP = GetHDRGNP(HDR_RATIO>>4);
        reinterpret_cast<OBC_REG_D1A_OBC_HDR*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, HDR))->Bits.OBC_GLE_FIRST = 0; //ZFUS_R1A_ZHDR_GLE_FIRST
        reinterpret_cast<OBC_REG_D1A_OBC_HDR*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, HDR))->Bits.OBC_RLE_FIRST = 0; //ZFUS_R1A_ZHDR_RLE_FIRST
        reinterpret_cast<OBC_REG_D1A_OBC_HDR*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, HDR))->Bits.OBC_BLE_FIRST = 0; //ZFUS_R1A_ZHDR_BLE_FIRST

        reinterpret_cast<OBC_REG_D1A_OBC_GRAY_BLD_2*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, GRAY_BLD_2))->Bits.OBC_BLD_MXRT_SE =
            reinterpret_cast<OBC_REG_D1A_OBC_GRAY_BLD_1*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, GRAY_BLD_1))->Bits.OBC_BLD_MXRT;
        reinterpret_cast<OBC_REG_D1A_OBC_GRAY_BLD_2*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, GRAY_BLD_2))->Bits.OBC_BLD_LOW_SE =
            (((reinterpret_cast<OBC_REG_D1A_OBC_GRAY_BLD_1*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, GRAY_BLD_1))->Bits.OBC_BLD_LOW << 5) / (HDR_RATIO +1)) >>1);
        reinterpret_cast<OBC_REG_D1A_OBC_GRAY_BLD_2*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, GRAY_BLD_2))->Bits.OBC_BLD_SLP_SE =
            ((reinterpret_cast<OBC_REG_D1A_OBC_GRAY_BLD_1*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, GRAY_BLD_1))->Bits.OBC_BLD_SLP * HDR_RATIO + 8) >> 4);

        writeRegs(static_cast<RegInfo_T*>(m_rIspRegInfo[SubModuleIndex]), m_u4RegInfoNum, pReg);

        dumpRegInfoP2("OBC", SubModuleIndex);
    }

    return  MTRUE;
}
}
