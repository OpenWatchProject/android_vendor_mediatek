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
#define LOG_TAG "isp_mgr_rrz"

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
// RRZ
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ISP_MGR_RRZ_T&
ISP_MGR_RRZ_T::
getInstance(ESensorDev_T const eSensorDev)
{
    ISP_MGR_MODULE_GET_INSTANCE(RRZ);
}

template <>
ISP_MGR_RRZ_T&
ISP_MGR_RRZ_T::
put(MUINT8 SubModuleIndex, ISP_NVRAM_RRZ_T const& rParam)
{
    if (SubModuleIndex >= ESubModule_NUM) {
        CAM_LOGE("[PUT] Error Submodule Index: %d", SubModuleIndex);
        return (*this);
    }
    PUT_REG_INFO_MULTI(SubModuleIndex, CTL,                       ctl);
    //PUT_REG_INFO_MULTI(SubModuleIndex, IN_IMG,                    in_img);
    //PUT_REG_INFO_MULTI(SubModuleIndex, OUT_IMG,                   out_img);
    //PUT_REG_INFO_MULTI(SubModuleIndex, HORI_STEP,                 hori_step);
    //PUT_REG_INFO_MULTI(SubModuleIndex, VERT_STEP,                 vert_step);
    //PUT_REG_INFO_MULTI(SubModuleIndex, HORI_INT_OFST,             hori_int_ofst);
    //PUT_REG_INFO_MULTI(SubModuleIndex, HORI_SUB_OFST,             hori_sub_ofst);
    //PUT_REG_INFO_MULTI(SubModuleIndex, VERT_INT_OFST,             vert_int_ofst);
    //PUT_REG_INFO_MULTI(SubModuleIndex, VERT_SUB_OFST,             vert_sub_ofst);
    PUT_REG_INFO_MULTI(SubModuleIndex, MODE_TH,                   mode_th);
    PUT_REG_INFO_MULTI(SubModuleIndex, MODE_CTL,                  mode_ctl);
    PUT_REG_INFO_MULTI(SubModuleIndex, LBLD_CFG,                  lbld_cfg);
    PUT_REG_INFO_MULTI(SubModuleIndex, NNIR_TBL_SEL,              nnir_tbl_sel);
    return (*this);
}

template <>
ISP_MGR_RRZ_T&
ISP_MGR_RRZ_T::
get(MUINT8 SubModuleIndex, ISP_NVRAM_RRZ_T& rParam)
{
    if (SubModuleIndex >= ESubModule_NUM) {
        CAM_LOGE("[GET] Error Submodule Index: %d", SubModuleIndex);
        return (*this);
    }
    GET_REG_INFO_MULTI(SubModuleIndex, CTL,                       ctl);
    //GET_REG_INFO_MULTI(SubModuleIndex, IN_IMG,                    in_img);
    //GET_REG_INFO_MULTI(SubModuleIndex, OUT_IMG,                   out_img);
    //GET_REG_INFO_MULTI(SubModuleIndex, HORI_STEP,                 hori_step);
    //GET_REG_INFO_MULTI(SubModuleIndex, VERT_STEP,                 vert_step);
    //GET_REG_INFO_MULTI(SubModuleIndex, HORI_INT_OFST,             hori_int_ofst);
    //GET_REG_INFO_MULTI(SubModuleIndex, HORI_SUB_OFST,             hori_sub_ofst);
    //GET_REG_INFO_MULTI(SubModuleIndex, VERT_INT_OFST,             vert_int_ofst);
    //GET_REG_INFO_MULTI(SubModuleIndex, VERT_SUB_OFST,             vert_sub_ofst);
    GET_REG_INFO_MULTI(SubModuleIndex, MODE_TH,                   mode_th);
    GET_REG_INFO_MULTI(SubModuleIndex, MODE_CTL,                  mode_ctl);
    GET_REG_INFO_MULTI(SubModuleIndex, LBLD_CFG,                  lbld_cfg);
    GET_REG_INFO_MULTI(SubModuleIndex, NNIR_TBL_SEL,              nnir_tbl_sel);
    return (*this);
}

MBOOL
ISP_MGR_RRZ_T::
apply_P1(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, TuningMgr& rTuning, MINT32 i4SubsampleIdex)
{
    if (SubModuleIndex >= ESubModule_NUM) {
        CAM_LOGE("[GET] Error Submodule Index: %d", SubModuleIndex);
        return MFALSE;
    }
    // Register setting
	//rTuning.updateEngine(eTuningMgrFunc_RRZ, MTRUE, i4SubsampleIdex);
    rTuning.tuningMgrWriteRegs( (TUNING_MGR_REG_IO_STRUCT*)(&(m_rIspRegInfo[SubModuleIndex][0])),
            m_u4RegInfoNum, i4SubsampleIdex);

    dumpRegInfoP1("RRZ", SubModuleIndex);
    return MTRUE;
}


};
