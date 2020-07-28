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
#define LOG_TAG "isp_mgr_mfb"

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
// MFB
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ISP_MGR_MFB_T&
ISP_MGR_MFB_T::
getInstance(ESensorDev_T const eSensorDev)
{
    ISP_MGR_MODULE_GET_INSTANCE(MFB);
}

template <>
ISP_MGR_MFB_T&
ISP_MGR_MFB_T::
put(MUINT8 SubModuleIndex, ISP_NVRAM_MFB_T const& rParam)
{
    if (SubModuleIndex >= ESubModule_NUM) {
        CAM_LOGE("[PUT] Error Submodule Index: %d", SubModuleIndex);
        return (*this);
    }
    PUT_REG_INFO_MULTI(SubModuleIndex, CON,                       con);
    PUT_REG_INFO_MULTI(SubModuleIndex, LL_CON1,                   ll_con1);
    PUT_REG_INFO_MULTI(SubModuleIndex, LL_CON10,                  ll_con10);
    PUT_REG_INFO_MULTI(SubModuleIndex, MBD_CON0,                  mbd_con0);
    PUT_REG_INFO_MULTI(SubModuleIndex, MBD_CON1,                  mbd_con1);
    PUT_REG_INFO_MULTI(SubModuleIndex, MBD_CON2,                  mbd_con2);
    PUT_REG_INFO_MULTI(SubModuleIndex, MBD_CON3,                  mbd_con3);
    PUT_REG_INFO_MULTI(SubModuleIndex, MBD_CON4,                  mbd_con4);
    PUT_REG_INFO_MULTI(SubModuleIndex, MBD_CON5,                  mbd_con5);
    PUT_REG_INFO_MULTI(SubModuleIndex, MBD_CON6,                  mbd_con6);
    PUT_REG_INFO_MULTI(SubModuleIndex, MBD_CON7,                  mbd_con7);
    PUT_REG_INFO_MULTI(SubModuleIndex, MBD_CON8,                  mbd_con8);
    PUT_REG_INFO_MULTI(SubModuleIndex, MBD_CON9,                  mbd_con9);
    PUT_REG_INFO_MULTI(SubModuleIndex, MBD_CON10,                 mbd_con10);
    return (*this);
}

template <>
ISP_MGR_MFB_T&
ISP_MGR_MFB_T::
get(MUINT8 SubModuleIndex, ISP_NVRAM_MFB_T& rParam)
{
    if (SubModuleIndex >= ESubModule_NUM) {
        CAM_LOGE("[GET] Error Submodule Index: %d", SubModuleIndex);
        return (*this);
    }
    GET_REG_INFO_MULTI(SubModuleIndex, CON,                       con);
    GET_REG_INFO_MULTI(SubModuleIndex, LL_CON1,                   ll_con1);
    GET_REG_INFO_MULTI(SubModuleIndex, LL_CON10,                  ll_con10);
    GET_REG_INFO_MULTI(SubModuleIndex, MBD_CON0,                  mbd_con0);
    GET_REG_INFO_MULTI(SubModuleIndex, MBD_CON1,                  mbd_con1);
    GET_REG_INFO_MULTI(SubModuleIndex, MBD_CON2,                  mbd_con2);
    GET_REG_INFO_MULTI(SubModuleIndex, MBD_CON3,                  mbd_con3);
    GET_REG_INFO_MULTI(SubModuleIndex, MBD_CON4,                  mbd_con4);
    GET_REG_INFO_MULTI(SubModuleIndex, MBD_CON5,                  mbd_con5);
    GET_REG_INFO_MULTI(SubModuleIndex, MBD_CON6,                  mbd_con6);
    GET_REG_INFO_MULTI(SubModuleIndex, MBD_CON7,                  mbd_con7);
    GET_REG_INFO_MULTI(SubModuleIndex, MBD_CON8,                  mbd_con8);
    GET_REG_INFO_MULTI(SubModuleIndex, MBD_CON9,                  mbd_con9);
    GET_REG_INFO_MULTI(SubModuleIndex, MBD_CON10,                 mbd_con10);
    return (*this);
}

MBOOL
ISP_MGR_MFB_T::
apply_MFB(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, mfb_reg_t* pReg)
{
    if (SubModuleIndex >= ESubModule_NUM) {
        CAM_LOGE("[GET] Error Submodule Index: %d", SubModuleIndex);
        return MFALSE;
    }
    // Register setting
    writeRegs(static_cast<RegInfo_T*>(m_rIspRegInfo[SubModuleIndex]), m_u4RegInfoNum, pReg);

    dumpRegInfoP2("MFB", SubModuleIndex);
    return MTRUE;
}


};
