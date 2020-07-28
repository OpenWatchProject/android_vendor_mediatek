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
#define LOG_TAG "isp_mgr_frz"

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
// FRZ
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ISP_MGR_FRZ_T&
ISP_MGR_FRZ_T::
getInstance(ESensorDev_T const eSensorDev)
{
    ISP_MGR_MODULE_GET_INSTANCE(FRZ);
}

template <>
ISP_MGR_FRZ_T&
ISP_MGR_FRZ_T::
put(MUINT8 SubModuleIndex, ISP_NVRAM_FRZ_T const& rParam)
{
    if (SubModuleIndex >= ESubModule_NUM) {
        CAM_LOGE("[PUT] Error Submodule Index: %d", SubModuleIndex);
        return (*this);
    }
    PUT_REG_INFO_MULTI(SubModuleIndex, TABLE_SEL,                 table_sel);
    PUT_REG_INFO_MULTI(SubModuleIndex, MODE_CTL,                  mode_ctl);
    //PUT_REG_INFO_MULTI(SubModuleIndex, IN_IMG,                    in_img);
    //PUT_REG_INFO_MULTI(SubModuleIndex, OUT_IMG,                   out_img);
    //PUT_REG_INFO_MULTI(SubModuleIndex, HORI_STEP,                 hori_step);
    //PUT_REG_INFO_MULTI(SubModuleIndex, VERT_STEP,                 vert_step);
    //PUT_REG_INFO_MULTI(SubModuleIndex, HORI_OFST,                 hori_ofst);
    //PUT_REG_INFO_MULTI(SubModuleIndex, VERT_OFST,                 vert_ofst);
    PUT_REG_INFO_MULTI(SubModuleIndex, MODE_TH,                   mode_th);
    PUT_REG_INFO_MULTI(SubModuleIndex, CL_MODE,                   cl_mode);
    //PUT_REG_INFO_MULTI(SubModuleIndex, OBC_RB,                    obc_rb);
    //PUT_REG_INFO_MULTI(SubModuleIndex, OBC_G,                     obc_g);
    //PUT_REG_INFO_MULTI(SubModuleIndex, WB_RB,                     wb_rb);
    //PUT_REG_INFO_MULTI(SubModuleIndex, WB_G,                      wb_g);
    //PUT_REG_INFO_MULTI(SubModuleIndex, IWB_RB,                    iwb_rb);
    //PUT_REG_INFO_MULTI(SubModuleIndex, IWB_G,                     iwb_g);
    PUT_REG_INFO_MULTI(SubModuleIndex, ZERO_OFST,                 zero_ofst);
    return (*this);
}

template <>
ISP_MGR_FRZ_T&
ISP_MGR_FRZ_T::
get(MUINT8 SubModuleIndex, ISP_NVRAM_FRZ_T& rParam)
{
    if (SubModuleIndex >= ESubModule_NUM) {
        CAM_LOGE("[GET] Error Submodule Index: %d", SubModuleIndex);
        return (*this);
    }
    GET_REG_INFO_MULTI(SubModuleIndex, TABLE_SEL,                 table_sel);
    GET_REG_INFO_MULTI(SubModuleIndex, MODE_CTL,                  mode_ctl);
    //GET_REG_INFO_MULTI(SubModuleIndex, IN_IMG,                    in_img);
    //GET_REG_INFO_MULTI(SubModuleIndex, OUT_IMG,                   out_img);
    //GET_REG_INFO_MULTI(SubModuleIndex, HORI_STEP,                 hori_step);
    //GET_REG_INFO_MULTI(SubModuleIndex, VERT_STEP,                 vert_step);
    //GET_REG_INFO_MULTI(SubModuleIndex, HORI_OFST,                 hori_ofst);
    //GET_REG_INFO_MULTI(SubModuleIndex, VERT_OFST,                 vert_ofst);
    GET_REG_INFO_MULTI(SubModuleIndex, MODE_TH,                   mode_th);
    GET_REG_INFO_MULTI(SubModuleIndex, CL_MODE,                   cl_mode);
    //GET_REG_INFO_MULTI(SubModuleIndex, OBC_RB,                    obc_rb);
    //GET_REG_INFO_MULTI(SubModuleIndex, OBC_G,                     obc_g);
    //GET_REG_INFO_MULTI(SubModuleIndex, WB_RB,                     wb_rb);
    //GET_REG_INFO_MULTI(SubModuleIndex, WB_G,                      wb_g);
    //GET_REG_INFO_MULTI(SubModuleIndex, IWB_RB,                    iwb_rb);
    //GET_REG_INFO_MULTI(SubModuleIndex, IWB_G,                     iwb_g);
    GET_REG_INFO_MULTI(SubModuleIndex, ZERO_OFST,                 zero_ofst);
    return (*this);
}

MVOID
ISP_MGR_FRZ_T::
setOBCOffset(MUINT8 SubModuleIndex, ISP_NVRAM_OBC_T const& rParam){
    if (SubModuleIndex >= ESubModule_NUM) {
        CAM_LOGE("[GET] Error Submodule Index: %d", SubModuleIndex);
        return;
    }
	mObc[SubModuleIndex][EFRZ_OBC_OFST_R] = rParam.offset_r.val;
	mObc[SubModuleIndex][EFRZ_OBC_OFST_B] = rParam.offset_b.val;
	mObc[SubModuleIndex][EFRZ_OBC_OFST_GR] = rParam.offset_gr.val;
	mObc[SubModuleIndex][EFRZ_OBC_OFST_GB] = rParam.offset_gb.val;
}

MBOOL
ISP_MGR_FRZ_T::
apply_P1(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, TuningMgr& rTuning, MINT32 i4SubsampleIdex)
{
    if (SubModuleIndex >= ESubModule_NUM) {
        CAM_LOGE("[GET] Error Submodule Index: %d", SubModuleIndex);
        return MFALSE;
    }
	//OBC offset
	reinterpret_cast<REG_FRZ_R1_FRZ_OBC_RB*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, OBC_RB))->Bits.FRZ_OBC_OFST_R = mObc[SubModuleIndex][EFRZ_OBC_OFST_R];
	reinterpret_cast<REG_FRZ_R1_FRZ_OBC_RB*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, OBC_RB))->Bits.FRZ_OBC_OFST_B = mObc[SubModuleIndex][EFRZ_OBC_OFST_B];
	reinterpret_cast<REG_FRZ_R1_FRZ_OBC_G*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, OBC_G))->Bits.FRZ_OBC_OFST_GR = mObc[SubModuleIndex][EFRZ_OBC_OFST_GR];
	reinterpret_cast<REG_FRZ_R1_FRZ_OBC_G*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, OBC_G))->Bits.FRZ_OBC_OFST_GB = mObc[SubModuleIndex][EFRZ_OBC_OFST_GB];

	//WB
	reinterpret_cast<REG_FRZ_R1_FRZ_WB_RB*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, WB_RB))->Bits.FRZ_WB_GAIN_R = rRawIspCamInfo.rAWBInfo.rRPG.i4R;
	reinterpret_cast<REG_FRZ_R1_FRZ_WB_RB*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, WB_RB))->Bits.FRZ_WB_GAIN_B = rRawIspCamInfo.rAWBInfo.rRPG.i4B;
	reinterpret_cast<REG_FRZ_R1_FRZ_WB_G*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, WB_G))->Bits.FRZ_WB_GAIN_GR = rRawIspCamInfo.rAWBInfo.rRPG.i4G;
	reinterpret_cast<REG_FRZ_R1_FRZ_WB_G*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, WB_G))->Bits.FRZ_WB_GAIN_GB = rRawIspCamInfo.rAWBInfo.rRPG.i4G;


	//WB Inverse Gain
	reinterpret_cast<REG_FRZ_R1_FRZ_IWB_RB*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, IWB_RB))->Bits.FRZ_WB_IGN_R = (((1 << 19) / rRawIspCamInfo.rAWBInfo.rRPG.i4R) + 1 ) >> 1;
	reinterpret_cast<REG_FRZ_R1_FRZ_IWB_RB*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, IWB_RB))->Bits.FRZ_WB_IGN_B = (((1 << 19) / rRawIspCamInfo.rAWBInfo.rRPG.i4B) + 1 ) >> 1;
	reinterpret_cast<REG_FRZ_R1_FRZ_IWB_G*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, IWB_G))->Bits.FRZ_WB_IGN_GB   = (((1 << 19) / rRawIspCamInfo.rAWBInfo.rRPG.i4G) + 1 ) >> 1;
	reinterpret_cast<REG_FRZ_R1_FRZ_IWB_G*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, IWB_G))->Bits.FRZ_WB_IGN_GB   = (((1 << 19) / rRawIspCamInfo.rAWBInfo.rRPG.i4G) + 1 ) >> 1;

	//rTuning.updateEngine(eTuningMgrFunc_FRZ_R1, MTRUE, i4SubsampleIdex);
    // Register setting
    rTuning.tuningMgrWriteRegs( (TUNING_MGR_REG_IO_STRUCT*)(&(m_rIspRegInfo[SubModuleIndex][0])),
            m_u4RegInfoNum, i4SubsampleIdex);

    dumpRegInfoP1("FRZ", SubModuleIndex);
    return MTRUE;
}


};
