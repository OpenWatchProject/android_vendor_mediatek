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
#define LOG_TAG "isp_mgr_cnr"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include <camera_custom_nvram.h>
#include "isp_mgr.h"
#include <string>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/imgbuf/IIonImageBufferHeap.h>


namespace NSIspTuning
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CNR
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ISP_MGR_CNR_T&
ISP_MGR_CNR_T::
getInstance(ESensorDev_T const eSensorDev)
{
    ISP_MGR_MODULE_GET_INSTANCE(CNR);
}

template <>
ISP_MGR_CNR_T&
ISP_MGR_CNR_T::
put(MUINT8 SubModuleIndex, ISP_NVRAM_CNR_CNR_T const& rParam)
{
    if (SubModuleIndex >= ESubModule_NUM) {
        CAM_LOGE("[PUT] Error Submodule Index: %d", SubModuleIndex);
        return (*this);
    }
    PUT_REG_INFO_MULTI(SubModuleIndex, CNR_CON1,                      con1);
    PUT_REG_INFO_MULTI(SubModuleIndex, CNR_CON2,                      con2);
    PUT_REG_INFO_MULTI(SubModuleIndex, CNR_YAD1,                      yad1);
    PUT_REG_INFO_MULTI(SubModuleIndex, CNR_Y4LUT1,                    y4lut1);
    PUT_REG_INFO_MULTI(SubModuleIndex, CNR_Y4LUT2,                    y4lut2);
    PUT_REG_INFO_MULTI(SubModuleIndex, CNR_Y4LUT3,                    y4lut3);
    PUT_REG_INFO_MULTI(SubModuleIndex, CNR_L4LUT1,                    l4lut1);
    PUT_REG_INFO_MULTI(SubModuleIndex, CNR_L4LUT2,                    l4lut2);
    PUT_REG_INFO_MULTI(SubModuleIndex, CNR_L4LUT3,                    l4lut3);
    PUT_REG_INFO_MULTI(SubModuleIndex, CNR_CAD,                       cad);
    PUT_REG_INFO_MULTI(SubModuleIndex, CNR_CB_VRNG,                   cb_vrng);
    PUT_REG_INFO_MULTI(SubModuleIndex, CNR_CB_HRNG,                   cb_hrng);
    PUT_REG_INFO_MULTI(SubModuleIndex, CNR_CR_VRNG,                   cr_vrng);
    PUT_REG_INFO_MULTI(SubModuleIndex, CNR_CR_HRNG,                   cr_hrng);
    PUT_REG_INFO_MULTI(SubModuleIndex, CNR_SL2,                       sl2);
    PUT_REG_INFO_MULTI(SubModuleIndex, CNR_MED1,                      med1);
    PUT_REG_INFO_MULTI(SubModuleIndex, CNR_MED2,                      med2);
    PUT_REG_INFO_MULTI(SubModuleIndex, CNR_MED3,                      med3);
    PUT_REG_INFO_MULTI(SubModuleIndex, CNR_MED4,                      med4);
    PUT_REG_INFO_MULTI(SubModuleIndex, CNR_MED5,                      med5);
    PUT_REG_INFO_MULTI(SubModuleIndex, CNR_MED6,                      med6);
    PUT_REG_INFO_MULTI(SubModuleIndex, CNR_MED7,                      med7);
    PUT_REG_INFO_MULTI(SubModuleIndex, CNR_MED8,                      med8);
    PUT_REG_INFO_MULTI(SubModuleIndex, CNR_MED9,                      med9);
    PUT_REG_INFO_MULTI(SubModuleIndex, CNR_MED10,                     med10);
    PUT_REG_INFO_MULTI(SubModuleIndex, CNR_MED11,                     med11);
    PUT_REG_INFO_MULTI(SubModuleIndex, CNR_MED12,                     med12);
    PUT_REG_INFO_MULTI(SubModuleIndex, CNR_MED13,                     med13);
    PUT_REG_INFO_MULTI(SubModuleIndex, CNR_ACTC,                      actc);
    return (*this);
}

template <>
ISP_MGR_CNR_T&
ISP_MGR_CNR_T::
get(MUINT8 SubModuleIndex, ISP_NVRAM_CNR_CNR_T& rParam)
{
    if (SubModuleIndex >= ESubModule_NUM) {
        CAM_LOGE("[GET] Error Submodule Index: %d", SubModuleIndex);
        return (*this);
    }
    GET_REG_INFO_MULTI(SubModuleIndex, CNR_CON1,                      con1);
    GET_REG_INFO_MULTI(SubModuleIndex, CNR_CON2,                      con2);
    GET_REG_INFO_MULTI(SubModuleIndex, CNR_YAD1,                      yad1);
    GET_REG_INFO_MULTI(SubModuleIndex, CNR_Y4LUT1,                    y4lut1);
    GET_REG_INFO_MULTI(SubModuleIndex, CNR_Y4LUT2,                    y4lut2);
    GET_REG_INFO_MULTI(SubModuleIndex, CNR_Y4LUT3,                    y4lut3);
    GET_REG_INFO_MULTI(SubModuleIndex, CNR_L4LUT1,                    l4lut1);
    GET_REG_INFO_MULTI(SubModuleIndex, CNR_L4LUT2,                    l4lut2);
    GET_REG_INFO_MULTI(SubModuleIndex, CNR_L4LUT3,                    l4lut3);
    GET_REG_INFO_MULTI(SubModuleIndex, CNR_CAD,                       cad);
    GET_REG_INFO_MULTI(SubModuleIndex, CNR_CB_VRNG,                   cb_vrng);
    GET_REG_INFO_MULTI(SubModuleIndex, CNR_CB_HRNG,                   cb_hrng);
    GET_REG_INFO_MULTI(SubModuleIndex, CNR_CR_VRNG,                   cr_vrng);
    GET_REG_INFO_MULTI(SubModuleIndex, CNR_CR_HRNG,                   cr_hrng);
    GET_REG_INFO_MULTI(SubModuleIndex, CNR_SL2,                       sl2);
    GET_REG_INFO_MULTI(SubModuleIndex, CNR_MED1,                      med1);
    GET_REG_INFO_MULTI(SubModuleIndex, CNR_MED2,                      med2);
    GET_REG_INFO_MULTI(SubModuleIndex, CNR_MED3,                      med3);
    GET_REG_INFO_MULTI(SubModuleIndex, CNR_MED4,                      med4);
    GET_REG_INFO_MULTI(SubModuleIndex, CNR_MED5,                      med5);
    GET_REG_INFO_MULTI(SubModuleIndex, CNR_MED6,                      med6);
    GET_REG_INFO_MULTI(SubModuleIndex, CNR_MED7,                      med7);
    GET_REG_INFO_MULTI(SubModuleIndex, CNR_MED8,                      med8);
    GET_REG_INFO_MULTI(SubModuleIndex, CNR_MED9,                      med9);
    GET_REG_INFO_MULTI(SubModuleIndex, CNR_MED10,                     med10);
    GET_REG_INFO_MULTI(SubModuleIndex, CNR_MED11,                     med11);
    GET_REG_INFO_MULTI(SubModuleIndex, CNR_MED12,                     med12);
    GET_REG_INFO_MULTI(SubModuleIndex, CNR_MED13,                     med13);
    GET_REG_INFO_MULTI(SubModuleIndex, CNR_ACTC,                      actc);
    return (*this);
}

template <>
ISP_MGR_CNR_T&
ISP_MGR_CNR_T::
put(MUINT8 SubModuleIndex, ISP_NVRAM_CNR_ABF_T const& rParam)
{
    if (SubModuleIndex >= ESubModule_NUM) {
        CAM_LOGE("[PUT] Error Submodule Index: %d", SubModuleIndex);
        return (*this);
    }
    PUT_REG_INFO_MULTI(SubModuleIndex, ABF_CON1,                      con1);
    PUT_REG_INFO_MULTI(SubModuleIndex, ABF_CON2,                      con2);
    PUT_REG_INFO_MULTI(SubModuleIndex, ABF_RCON,                      rcon);
    PUT_REG_INFO_MULTI(SubModuleIndex, ABF_YLUT,                      ylut);
    PUT_REG_INFO_MULTI(SubModuleIndex, ABF_CXLUT,                     cxlut);
    PUT_REG_INFO_MULTI(SubModuleIndex, ABF_CYLUT,                     cylut);
    PUT_REG_INFO_MULTI(SubModuleIndex, ABF_YSP,                       ysp);
    PUT_REG_INFO_MULTI(SubModuleIndex, ABF_CXSP,                      cxsp);
    PUT_REG_INFO_MULTI(SubModuleIndex, ABF_CYSP,                      cysp);
    PUT_REG_INFO_MULTI(SubModuleIndex, ABF_CLP,                       clp);
    return (*this);
}

template <>
ISP_MGR_CNR_T&
ISP_MGR_CNR_T::
get(MUINT8 SubModuleIndex, ISP_NVRAM_CNR_ABF_T& rParam)
{
    if (SubModuleIndex >= ESubModule_NUM) {
        CAM_LOGE("[GET] Error Submodule Index: %d", SubModuleIndex);
        return (*this);
    }
    GET_REG_INFO_MULTI(SubModuleIndex, ABF_CON1,                      con1);
    GET_REG_INFO_MULTI(SubModuleIndex, ABF_CON2,                      con2);
    GET_REG_INFO_MULTI(SubModuleIndex, ABF_RCON,                      rcon);
    GET_REG_INFO_MULTI(SubModuleIndex, ABF_YLUT,                      ylut);
    GET_REG_INFO_MULTI(SubModuleIndex, ABF_CXLUT,                     cxlut);
    GET_REG_INFO_MULTI(SubModuleIndex, ABF_CYLUT,                     cylut);
    GET_REG_INFO_MULTI(SubModuleIndex, ABF_YSP,                       ysp);
    GET_REG_INFO_MULTI(SubModuleIndex, ABF_CXSP,                      cxsp);
    GET_REG_INFO_MULTI(SubModuleIndex, ABF_CYSP,                      cysp);
    GET_REG_INFO_MULTI(SubModuleIndex, ABF_CLP,                       clp);
    return (*this);
}

template <>
ISP_MGR_CNR_T&
ISP_MGR_CNR_T::
put(MUINT8 SubModuleIndex, ISP_NVRAM_CNR_CCR_T const& rParam)
{
    if (SubModuleIndex >= ESubModule_NUM) {
        CAM_LOGE("[PUT] Error Submodule Index: %d", SubModuleIndex);
        return (*this);
    }
    PUT_REG_INFO_MULTI(SubModuleIndex, CCR_CON,                       con);
    PUT_REG_INFO_MULTI(SubModuleIndex, CCR_SAT_CTRL,                  sat_ctrl);
    PUT_REG_INFO_MULTI(SubModuleIndex, CCR_YLUT,                      ylut);
    PUT_REG_INFO_MULTI(SubModuleIndex, CCR_UVLUT,                     uvlut);
    PUT_REG_INFO_MULTI(SubModuleIndex, CCR_YLUT2,                     ylut2);
    PUT_REG_INFO_MULTI(SubModuleIndex, CCR_UVLUT_SP,                  uvlut_sp);
    PUT_REG_INFO_MULTI(SubModuleIndex, CCR_HUE1,                      hue1);
    PUT_REG_INFO_MULTI(SubModuleIndex, CCR_HUE2,                      hue2);
    PUT_REG_INFO_MULTI(SubModuleIndex, CCR_HUE3,                      hue3);
    PUT_REG_INFO_MULTI(SubModuleIndex, CCR_L4LUT1,                    l4lut1);
    PUT_REG_INFO_MULTI(SubModuleIndex, CCR_L4LUT2,                    l4lut2);
    PUT_REG_INFO_MULTI(SubModuleIndex, CCR_L4LUT3,                    l4lut3);
    return (*this);
}

template <>
ISP_MGR_CNR_T&
ISP_MGR_CNR_T::
get(MUINT8 SubModuleIndex, ISP_NVRAM_CNR_CCR_T& rParam)
{
    if (SubModuleIndex >= ESubModule_NUM) {
        CAM_LOGE("[GET] Error Submodule Index: %d", SubModuleIndex);
        return (*this);
    }
    GET_REG_INFO_MULTI(SubModuleIndex, CCR_CON,                       con);
    GET_REG_INFO_MULTI(SubModuleIndex, CCR_SAT_CTRL,                  sat_ctrl);
    GET_REG_INFO_MULTI(SubModuleIndex, CCR_YLUT,                      ylut);
    GET_REG_INFO_MULTI(SubModuleIndex, CCR_UVLUT,                     uvlut);
    GET_REG_INFO_MULTI(SubModuleIndex, CCR_YLUT2,                     ylut2);
    GET_REG_INFO_MULTI(SubModuleIndex, CCR_UVLUT_SP,                  uvlut_sp);
    GET_REG_INFO_MULTI(SubModuleIndex, CCR_HUE1,                      hue1);
    GET_REG_INFO_MULTI(SubModuleIndex, CCR_HUE2,                      hue2);
    GET_REG_INFO_MULTI(SubModuleIndex, CCR_HUE3,                      hue3);
    GET_REG_INFO_MULTI(SubModuleIndex, CCR_L4LUT1,                    l4lut1);
    GET_REG_INFO_MULTI(SubModuleIndex, CCR_L4LUT2,                    l4lut2);
    GET_REG_INFO_MULTI(SubModuleIndex, CCR_L4LUT3,                    l4lut3);
    return (*this);
}

MBOOL
ISP_MGR_CNR_T::
apply_P2(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, dip_a_reg_t* pReg)
{
    if(SubModuleIndex >= ESubModule_NUM){
        return MFALSE;
    }

    MBOOL bCNR_ENC = reinterpret_cast<CNR_REG_D1A_CNR_CNR_CON1*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, CNR_CON1))->Bits.CNR_CNR_ENC
        &isCNREnable(SubModuleIndex);
    MBOOL bCNR_BPC_EN = reinterpret_cast<CNR_REG_D1A_CNR_CNR_CON1*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, CNR_CON1))->Bits.CNR_BPC_EN
        &isCNREnable(SubModuleIndex);
    MBOOL bCCR_EN = reinterpret_cast<CNR_REG_D1A_CNR_CCR_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, CCR_CON))->Bits.CNR_CCR_EN
        &isCCREnable(SubModuleIndex);
    MBOOL bABF_EN = reinterpret_cast<CNR_REG_D1A_CNR_ABF_CON1*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, ABF_CON1))->Bits.CNR_ABF_EN
        &isABFEnable(SubModuleIndex);
    MBOOL bBOK_EN = isBOKEnable(SubModuleIndex);

    MBOOL bEnable = bCNR_ENC | bCNR_BPC_EN | bCCR_EN | bABF_EN | bBOK_EN;

    if(bEnable){
        //Sub module control
        reinterpret_cast<CNR_REG_D1A_CNR_CNR_CON1*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, CNR_CON1))->Bits.CNR_CNR_ENC = bCNR_ENC;
        reinterpret_cast<CNR_REG_D1A_CNR_CCR_CON*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, CCR_CON))->Bits.CNR_CCR_EN = bCCR_EN;
        reinterpret_cast<CNR_REG_D1A_CNR_ABF_CON1*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, ABF_CON1))->Bits.CNR_ABF_EN = bABF_EN;

        reinterpret_cast<CNR_REG_D1A_CNR_CNR_CON1*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, CNR_CON1))->Bits.CNR_MODE = bBOK_EN;
    }

    //Top Control
    switch (SubModuleIndex)
    {
        case ECNR_D1:
            ISP_WRITE_ENABLE_BITS(pReg, DIPCTL_D1A_DIPCTL_YUV_EN1, DIPCTL_CNR_D1_EN, bEnable);
            break;
        default:
            CAM_LOGE("Apply Error Submodule Index: %d", SubModuleIndex);
            return  MFALSE;
    }

    writeRegs(static_cast<RegInfo_T*>(m_rIspRegInfo[SubModuleIndex]), m_u4RegInfoNum, pReg);

    dumpRegInfoP2("CNR", SubModuleIndex);

    return  MTRUE;
}


};
