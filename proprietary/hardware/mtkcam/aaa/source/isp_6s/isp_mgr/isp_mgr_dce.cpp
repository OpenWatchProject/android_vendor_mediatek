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
#define LOG_TAG "isp_mgr_dce"

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
// DCE
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ISP_MGR_DCE_T&
ISP_MGR_DCE_T::
getInstance(ESensorDev_T const eSensorDev)
{
    ISP_MGR_MODULE_GET_INSTANCE(DCE);
}

template <>
ISP_MGR_DCE_T&
ISP_MGR_DCE_T::
put(MUINT8 SubModuleIndex, ISP_NVRAM_DCE_T const& rParam)
{
    if (SubModuleIndex >= ESubModule_NUM) {
        CAM_LOGE("[PUT] Error Submodule Index: %d", SubModuleIndex);
        return (*this);
    }
    PUT_REG_INFO_MULTI(SubModuleIndex, TC_G1,                     tc_g1);
    PUT_REG_INFO_MULTI(SubModuleIndex, TC_G2,                     tc_g2);
    PUT_REG_INFO_MULTI(SubModuleIndex, TC_G3,                     tc_g3);
    PUT_REG_INFO_MULTI(SubModuleIndex, TC_G4,                     tc_g4);
    PUT_REG_INFO_MULTI(SubModuleIndex, TC_G5,                     tc_g5);
    PUT_REG_INFO_MULTI(SubModuleIndex, TC_G6,                     tc_g6);
    PUT_REG_INFO_MULTI(SubModuleIndex, TC_G7,                     tc_g7);
    PUT_REG_INFO_MULTI(SubModuleIndex, TC_G8,                     tc_g8);
    PUT_REG_INFO_MULTI(SubModuleIndex, TC_G9,                     tc_g9);
    PUT_REG_INFO_MULTI(SubModuleIndex, TC_G10,                    tc_g10);
    PUT_REG_INFO_MULTI(SubModuleIndex, TC_G11,                    tc_g11);
    PUT_REG_INFO_MULTI(SubModuleIndex, TC_G12,                    tc_g12);
    PUT_REG_INFO_MULTI(SubModuleIndex, TC_G13,                    tc_g13);
    PUT_REG_INFO_MULTI(SubModuleIndex, TC_G14,                    tc_g14);
    PUT_REG_INFO_MULTI(SubModuleIndex, TC_G15,                    tc_g15);
    PUT_REG_INFO_MULTI(SubModuleIndex, TC_G16,                    tc_g16);
    PUT_REG_INFO_MULTI(SubModuleIndex, TC_G17,                    tc_g17);
    PUT_REG_INFO_MULTI(SubModuleIndex, TC_G18,                    tc_g18);
    PUT_REG_INFO_MULTI(SubModuleIndex, TC_G19,                    tc_g19);
    PUT_REG_INFO_MULTI(SubModuleIndex, TC_G20,                    tc_g20);
    PUT_REG_INFO_MULTI(SubModuleIndex, TC_G21,                    tc_g21);
    PUT_REG_INFO_MULTI(SubModuleIndex, TC_G22,                    tc_g22);
    PUT_REG_INFO_MULTI(SubModuleIndex, TC_G23,                    tc_g23);
    PUT_REG_INFO_MULTI(SubModuleIndex, TC_G24,                    tc_g24);
    PUT_REG_INFO_MULTI(SubModuleIndex, TC_G25,                    tc_g25);
    PUT_REG_INFO_MULTI(SubModuleIndex, TC_G26,                    tc_g26);
    PUT_REG_INFO_MULTI(SubModuleIndex, TC_G27,                    tc_g27);
    PUT_REG_INFO_MULTI(SubModuleIndex, TC_G28,                    tc_g28);
    PUT_REG_INFO_MULTI(SubModuleIndex, TC_G29,                    tc_g29);
    PUT_REG_INFO_MULTI(SubModuleIndex, TC_G30,                    tc_g30);
    PUT_REG_INFO_MULTI(SubModuleIndex, TC_G31,                    tc_g31);
    PUT_REG_INFO_MULTI(SubModuleIndex, TC_G32,                    tc_g32);

    //PUT_REG_INFO_MULTI(SubModuleIndex, V_Y_CH_1,                  v_y_ch_1);
    return (*this);
}

template <>
ISP_MGR_DCE_T&
ISP_MGR_DCE_T::
get(MUINT8 SubModuleIndex, ISP_NVRAM_DCE_T& rParam)
{
    if (SubModuleIndex >= ESubModule_NUM) {
        CAM_LOGE("[GET] Error Submodule Index: %d", SubModuleIndex);
        return (*this);
    }
    GET_REG_INFO_MULTI(SubModuleIndex, TC_G1,                     tc_g1);
    GET_REG_INFO_MULTI(SubModuleIndex, TC_G2,                     tc_g2);
    GET_REG_INFO_MULTI(SubModuleIndex, TC_G3,                     tc_g3);
    GET_REG_INFO_MULTI(SubModuleIndex, TC_G4,                     tc_g4);
    GET_REG_INFO_MULTI(SubModuleIndex, TC_G5,                     tc_g5);
    GET_REG_INFO_MULTI(SubModuleIndex, TC_G6,                     tc_g6);
    GET_REG_INFO_MULTI(SubModuleIndex, TC_G7,                     tc_g7);
    GET_REG_INFO_MULTI(SubModuleIndex, TC_G8,                     tc_g8);
    GET_REG_INFO_MULTI(SubModuleIndex, TC_G9,                     tc_g9);
    GET_REG_INFO_MULTI(SubModuleIndex, TC_G10,                    tc_g10);
    GET_REG_INFO_MULTI(SubModuleIndex, TC_G11,                    tc_g11);
    GET_REG_INFO_MULTI(SubModuleIndex, TC_G12,                    tc_g12);
    GET_REG_INFO_MULTI(SubModuleIndex, TC_G13,                    tc_g13);
    GET_REG_INFO_MULTI(SubModuleIndex, TC_G14,                    tc_g14);
    GET_REG_INFO_MULTI(SubModuleIndex, TC_G15,                    tc_g15);
    GET_REG_INFO_MULTI(SubModuleIndex, TC_G16,                    tc_g16);
    GET_REG_INFO_MULTI(SubModuleIndex, TC_G17,                    tc_g17);
    GET_REG_INFO_MULTI(SubModuleIndex, TC_G18,                    tc_g18);
    GET_REG_INFO_MULTI(SubModuleIndex, TC_G19,                    tc_g19);
    GET_REG_INFO_MULTI(SubModuleIndex, TC_G20,                    tc_g20);
    GET_REG_INFO_MULTI(SubModuleIndex, TC_G21,                    tc_g21);
    GET_REG_INFO_MULTI(SubModuleIndex, TC_G22,                    tc_g22);
    GET_REG_INFO_MULTI(SubModuleIndex, TC_G23,                    tc_g23);
    GET_REG_INFO_MULTI(SubModuleIndex, TC_G24,                    tc_g24);
    GET_REG_INFO_MULTI(SubModuleIndex, TC_G25,                    tc_g25);
    GET_REG_INFO_MULTI(SubModuleIndex, TC_G26,                    tc_g26);
    GET_REG_INFO_MULTI(SubModuleIndex, TC_G27,                    tc_g27);
    GET_REG_INFO_MULTI(SubModuleIndex, TC_G28,                    tc_g28);
    GET_REG_INFO_MULTI(SubModuleIndex, TC_G29,                    tc_g29);
    GET_REG_INFO_MULTI(SubModuleIndex, TC_G30,                    tc_g30);
    GET_REG_INFO_MULTI(SubModuleIndex, TC_G31,                    tc_g31);
    GET_REG_INFO_MULTI(SubModuleIndex, TC_G32,                    tc_g32);

    //GET_REG_INFO_MULTI(SubModuleIndex, V_Y_CH_1,                  v_y_ch_1);
    return (*this);
}

MBOOL
ISP_MGR_DCE_T::
apply_P2(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, dip_a_reg_t* pReg)
{
    if(SubModuleIndex >= ESubModule_NUM){
        return MFALSE;
    }
    MBOOL bEnable = isEnable(SubModuleIndex);

    //Top Control
    switch (SubModuleIndex)
    {
        case EDCE_D1:
            ISP_WRITE_ENABLE_BITS(pReg, DIPCTL_D1A_DIPCTL_YUV_EN1, DIPCTL_DCE_D1_EN, bEnable);
            break;
        default:
            CAM_LOGE("Apply Error Submodule Index: %d", SubModuleIndex);
            return  MFALSE;
    }

    if(bEnable){

    writeRegs(static_cast<RegInfo_T*>(m_rIspRegInfo[SubModuleIndex]), m_u4RegInfoNum, pReg);

    dumpRegInfoP2("DCE", SubModuleIndex);
    }


    return MTRUE;
}


};
