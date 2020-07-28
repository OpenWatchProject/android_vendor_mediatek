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
#define LOG_TAG "isp_mgr_ldnr"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
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
// LDNR
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ISP_MGR_LDNR_T&
ISP_MGR_LDNR_T::
getInstance(ESensorDev_T const eSensorDev)
{
    ISP_MGR_MODULE_GET_INSTANCE(LDNR);
}

template <>
ISP_MGR_LDNR_T&
ISP_MGR_LDNR_T::
put(MUINT8 SubModuleIndex, ISP_NVRAM_LDNR_T const& rParam)
{
    if (SubModuleIndex >= ESubModule_NUM) {
        CAM_LOGE("[PUT] Error Submodule Index: %d", SubModuleIndex);
        return (*this);
    }
    PUT_REG_INFO_MULTI(SubModuleIndex, CON1,                      con1);
    PUT_REG_INFO_MULTI(SubModuleIndex, EE_CON1,                   ee_con1);
    PUT_REG_INFO_MULTI(SubModuleIndex, EE_LM_Y012,                ee_lm_y012);
    PUT_REG_INFO_MULTI(SubModuleIndex, EE_LM_Y345,                ee_lm_y345);
    PUT_REG_INFO_MULTI(SubModuleIndex, EE_SL,                     ee_sl);
    PUT_REG_INFO_MULTI(SubModuleIndex, EE_LWB,                    ee_lwb);
    PUT_REG_INFO_MULTI(SubModuleIndex, EE_UPB,                    ee_upb);
    PUT_REG_INFO_MULTI(SubModuleIndex, RNG,                       rng);
    PUT_REG_INFO_MULTI(SubModuleIndex, CON2,                      con2);
    PUT_REG_INFO_MULTI(SubModuleIndex, SL,                        sl);
    PUT_REG_INFO_MULTI(SubModuleIndex, SSL_STH,                   ssl_sth);
    PUT_REG_INFO_MULTI(SubModuleIndex, EE_CON2,                   ee_con2);
    PUT_REG_INFO_MULTI(SubModuleIndex, EE_TH,                     ee_th);
    PUT_REG_INFO_MULTI(SubModuleIndex, EE_HGN,                    ee_hgn);
    PUT_REG_INFO_MULTI(SubModuleIndex, CNV_00_01,                 cnv_00_01);
    PUT_REG_INFO_MULTI(SubModuleIndex, CNV_02,                    cnv_02);
    PUT_REG_INFO_MULTI(SubModuleIndex, CNV_10_11,                 cnv_10_11);
    PUT_REG_INFO_MULTI(SubModuleIndex, CNV_12,                    cnv_12);
    PUT_REG_INFO_MULTI(SubModuleIndex, CNV_20_21,                 cnv_20_21);
    PUT_REG_INFO_MULTI(SubModuleIndex, CNV_22,                    cnv_22);
    PUT_REG_INFO_MULTI(SubModuleIndex, ICNV_00_01,                icnv_00_01);
    PUT_REG_INFO_MULTI(SubModuleIndex, ICNV_02,                   icnv_02);
    PUT_REG_INFO_MULTI(SubModuleIndex, ICNV_10_11,                icnv_10_11);
    PUT_REG_INFO_MULTI(SubModuleIndex, ICNV_12,                   icnv_12);
    PUT_REG_INFO_MULTI(SubModuleIndex, ICNV_20_21,                icnv_20_21);
    PUT_REG_INFO_MULTI(SubModuleIndex, ICNV_22,                   icnv_22);
    PUT_REG_INFO_MULTI(SubModuleIndex, COEF_C1,                   coef_c1);
    PUT_REG_INFO_MULTI(SubModuleIndex, COEF_C2,                   coef_c2);
    PUT_REG_INFO_MULTI(SubModuleIndex, NM_B,                      nm_b);
    PUT_REG_INFO_MULTI(SubModuleIndex, NM_G,                      nm_g);
    PUT_REG_INFO_MULTI(SubModuleIndex, NM_R,                      nm_r);
    return (*this);
}

template <>
ISP_MGR_LDNR_T&
ISP_MGR_LDNR_T::
get(MUINT8 SubModuleIndex, ISP_NVRAM_LDNR_T& rParam)
{
    if (SubModuleIndex >= ESubModule_NUM) {
        CAM_LOGE("[GET] Error Submodule Index: %d", SubModuleIndex);
        return (*this);
    }
    GET_REG_INFO_MULTI(SubModuleIndex, CON1,                      con1);
    GET_REG_INFO_MULTI(SubModuleIndex, EE_CON1,                   ee_con1);
    GET_REG_INFO_MULTI(SubModuleIndex, EE_LM_Y012,                ee_lm_y012);
    GET_REG_INFO_MULTI(SubModuleIndex, EE_LM_Y345,                ee_lm_y345);
    GET_REG_INFO_MULTI(SubModuleIndex, EE_SL,                     ee_sl);
    GET_REG_INFO_MULTI(SubModuleIndex, EE_LWB,                    ee_lwb);
    GET_REG_INFO_MULTI(SubModuleIndex, EE_UPB,                    ee_upb);
    GET_REG_INFO_MULTI(SubModuleIndex, RNG,                       rng);
    GET_REG_INFO_MULTI(SubModuleIndex, CON2,                      con2);
    GET_REG_INFO_MULTI(SubModuleIndex, SL,                        sl);
    GET_REG_INFO_MULTI(SubModuleIndex, SSL_STH,                   ssl_sth);
    GET_REG_INFO_MULTI(SubModuleIndex, EE_CON2,                   ee_con2);
    GET_REG_INFO_MULTI(SubModuleIndex, EE_TH,                     ee_th);
    GET_REG_INFO_MULTI(SubModuleIndex, EE_HGN,                    ee_hgn);
    GET_REG_INFO_MULTI(SubModuleIndex, CNV_00_01,                 cnv_00_01);
    GET_REG_INFO_MULTI(SubModuleIndex, CNV_02,                    cnv_02);
    GET_REG_INFO_MULTI(SubModuleIndex, CNV_10_11,                 cnv_10_11);
    GET_REG_INFO_MULTI(SubModuleIndex, CNV_12,                    cnv_12);
    GET_REG_INFO_MULTI(SubModuleIndex, CNV_20_21,                 cnv_20_21);
    GET_REG_INFO_MULTI(SubModuleIndex, CNV_22,                    cnv_22);
    GET_REG_INFO_MULTI(SubModuleIndex, ICNV_00_01,                icnv_00_01);
    GET_REG_INFO_MULTI(SubModuleIndex, ICNV_02,                   icnv_02);
    GET_REG_INFO_MULTI(SubModuleIndex, ICNV_10_11,                icnv_10_11);
    GET_REG_INFO_MULTI(SubModuleIndex, ICNV_12,                   icnv_12);
    GET_REG_INFO_MULTI(SubModuleIndex, ICNV_20_21,                icnv_20_21);
    GET_REG_INFO_MULTI(SubModuleIndex, ICNV_22,                   icnv_22);
    GET_REG_INFO_MULTI(SubModuleIndex, COEF_C1,                   coef_c1);
    GET_REG_INFO_MULTI(SubModuleIndex, COEF_C2,                   coef_c2);
    GET_REG_INFO_MULTI(SubModuleIndex, NM_B,                      nm_b);
    GET_REG_INFO_MULTI(SubModuleIndex, NM_G,                      nm_g);
    GET_REG_INFO_MULTI(SubModuleIndex, NM_R,                      nm_r);
    return (*this);
}

MBOOL
ISP_MGR_LDNR_T::
apply_P1(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, TuningMgr& rTuning, MINT32 i4SubsampleIdex)
{
    if (SubModuleIndex >= ESubModule_NUM) {
        CAM_LOGE("[GET] Error Submodule Index: %d", SubModuleIndex);
        return MFALSE;
    }
    // Register setting
    rTuning.tuningMgrWriteRegs( (TUNING_MGR_REG_IO_STRUCT*)(&(m_rIspRegInfo[SubModuleIndex][0])),
            m_u4RegInfoNum, i4SubsampleIdex);

    dumpRegInfoP1("LDNR", SubModuleIndex);
    return MTRUE;
}

MBOOL
ISP_MGR_LDNR_T::
apply_P2(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, dip_a_reg_t* pReg)
{
    if (SubModuleIndex >= ESubModule_NUM) {
        CAM_LOGE("[GET] Error Submodule Index: %d", SubModuleIndex);
        return MFALSE;
    }
    MBOOL bEnable = isEnable(SubModuleIndex);

    //Top Control
    switch (SubModuleIndex)
    {
        case ELDNR_D1:
            ISP_WRITE_ENABLE_BITS(pReg, DIPCTL_D1A_DIPCTL_RGB_EN1, DIPCTL_LDNR_D1_EN, bEnable);
            break;
        default:
            CAM_LOGE("Apply Error Submodule Index: %d", SubModuleIndex);
            return  MFALSE;
    }

    writeRegs(static_cast<RegInfo_T*>(m_rIspRegInfo[SubModuleIndex]), m_u4RegInfoNum, pReg);

    dumpRegInfoP2("LDNR", SubModuleIndex);
    return MTRUE;
}


};
