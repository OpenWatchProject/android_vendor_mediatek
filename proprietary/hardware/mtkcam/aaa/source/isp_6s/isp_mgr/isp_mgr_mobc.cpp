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
#define LOG_TAG "isp_mgr_mobc"

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
// MOBC
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ISP_MGR_MOBC_T&
ISP_MGR_MOBC_T::
getInstance(ESensorDev_T const eSensorDev)
{
    ISP_MGR_MODULE_GET_INSTANCE(MOBC);
}


ISP_MGR_MOBC_T&
ISP_MGR_MOBC_T::
put(MUINT8 SubModuleIndex, ISP_NVRAM_OBC_T const& rParam)
{
    if (SubModuleIndex >= ESubModule_NUM) {
        CAM_LOGE("[PUT] Error Submodule Index: %d", SubModuleIndex);
        return (*this);
    }
	/* HHHH

    PUT_REG_INFO_MULTI(SubModuleIndex, OFFST0,                    offst0);
    PUT_REG_INFO_MULTI(SubModuleIndex, OFFST1,                    offst1);
    PUT_REG_INFO_MULTI(SubModuleIndex, OFFST2,                    offst2);
    PUT_REG_INFO_MULTI(SubModuleIndex, OFFST3,                    offst3);

    */
    //PUT_REG_INFO_MULTI(SubModuleIndex, GAIN0,                     gain0);
    //PUT_REG_INFO_MULTI(SubModuleIndex, GAIN1,                     gain1);
    //PUT_REG_INFO_MULTI(SubModuleIndex, GAIN2,                     gain2);
    //PUT_REG_INFO_MULTI(SubModuleIndex, GAIN3,                     gain3);
    return (*this);
}

ISP_MGR_MOBC_T&
ISP_MGR_MOBC_T::
get(MUINT8 SubModuleIndex, ISP_NVRAM_OBC_T& rParam)
{
    if (SubModuleIndex >= ESubModule_NUM) {
        CAM_LOGE("[GET] Error Submodule Index: %d", SubModuleIndex);
        return (*this);
    }
/* HHHH
    GET_REG_INFO_MULTI(SubModuleIndex, OFFST0,                    offst0);
    GET_REG_INFO_MULTI(SubModuleIndex, OFFST1,                    offst1);
    GET_REG_INFO_MULTI(SubModuleIndex, OFFST2,                    offst2);
    GET_REG_INFO_MULTI(SubModuleIndex, OFFST3,                    offst3);
*/
    //GET_REG_INFO_MULTI(SubModuleIndex, GAIN0,                     gain0);
    //GET_REG_INFO_MULTI(SubModuleIndex, GAIN1,                     gain1);
    //GET_REG_INFO_MULTI(SubModuleIndex, GAIN2,                     gain2);
    //GET_REG_INFO_MULTI(SubModuleIndex, GAIN3,                     gain3);
    return (*this);
}

MBOOL
ISP_MGR_MOBC_T::
apply_P1(MUINT8 SubModuleIndex, RAWIspCamInfo& rRawIspCamInfo, TuningMgr& rTuning, MINT32 i4SubsampleIdex)
{
    if(SubModuleIndex >= ESubModule_NUM){
        return MFALSE;
    }
/* HHHHHHH
    MBOOL bEnable = isEnable(SubModuleIndex);

    //Top Control
    switch (SubModuleIndex)
    {
        case EMOBC_M1:
            ISP_MGR_CAMCTL_T::getInstance(m_eSensorDev).setEnable_MOBC_R2(bEnable);
            rTuning.updateEngine(eTuningMgrFunc_MOBC_R2, bEnable, i4SubsampleIdex);
            break;
#if 0
        case EMOBC_R3:
            ISP_MGR_CAMCTL_T::getInstance(m_eSensorDev).setEnable_MOBC_R3(bEnable);
            rTuning.updateEngine(eTuningMgrFunc_MOBC_R3, bEnable, i4SubsampleIdex);
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
            reinterpret_cast<REG_MOBC_R2_MOBC_OFFST0*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, OFFST0))->Bits.MOBC_OFST_B = i4FakeOfst;
            reinterpret_cast<REG_MOBC_R2_MOBC_OFFST1*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, OFFST1))->Bits.MOBC_OFST_GB = i4FakeOfst;
            reinterpret_cast<REG_MOBC_R2_MOBC_OFFST2*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, OFFST2))->Bits.MOBC_OFST_GR = i4FakeOfst;
            reinterpret_cast<REG_MOBC_R2_MOBC_OFFST3*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, OFFST3))->Bits.MOBC_OFST_R = i4FakeOfst;
            CAM_LOGE("Chooo, OFST:%d, adb:%d", reinterpret_cast<REG_MOBC_R2_MOBC_OFFST0*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, OFFST0))->Bits.MOBC_OFST_B , i4FakeOfst);
        }

        //OBC Gain
        float fOB_OFST_R= -(float)(0xFFF ^ (reinterpret_cast<REG_MOBC_R2_MOBC_OFFST3*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, OFFST3))->Bits.MOBC_OFST_R -1));
        float fOB_OFST_B= -(float)(0xFFF ^ (reinterpret_cast<REG_MOBC_R2_MOBC_OFFST0*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, OFFST0))->Bits.MOBC_OFST_B -1));
        float fOB_OFST_GR= -(float)(0xFFF ^ (reinterpret_cast<REG_MOBC_R2_MOBC_OFFST2*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, OFFST2))->Bits.MOBC_OFST_GR -1));
        float fOB_OFST_GB= -(float)(0xFFF ^ (reinterpret_cast<REG_MOBC_R2_MOBC_OFFST1*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, OFFST1))->Bits.MOBC_OFST_GB -1));

        if(fOB_OFST_R < -511.0f){
            CAM_LOGE("fMOBC_OFST_R (%d) Error (< -511)", fOB_OFST_R);
            fOB_OFST_R = -511.0f;
        }
        else if(fOB_OFST_R > 0.0f){
            CAM_LOGE("fMOBC_OFST_R (%d) Error (> 0)", fOB_OFST_R);
            fOB_OFST_R = 0.0f;
        }

        if(fOB_OFST_B < -511.0f){
            CAM_LOGE("fMOBC_OFST_B (%d) Error (< -511)", fOB_OFST_B);
            fOB_OFST_B = -511.0f;
        }
        else if(fOB_OFST_B > 0.0f){
            CAM_LOGE("fMOBC_OFST_B (%d) Error (> 0)", fOB_OFST_B);
            fOB_OFST_B = 0.0f;
        }

        if(fOB_OFST_GR < -511.0f){
            CAM_LOGE("fMOBC_OFST_GR (%d) Error (< -511)", fOB_OFST_GR);
            fOB_OFST_GR = -511.0f;
        }
        else if(fOB_OFST_GR > 0.0f){
            CAM_LOGE("fMOBC_OFST_GR (%d) Error (> 0)", fOB_OFST_GR);
            fOB_OFST_GR = 0.0f;
        }

        if(fOB_OFST_GB < -511.0f){
            CAM_LOGE("fMOBC_OFST_GB (%d) Error (< -511)", fOB_OFST_GB);
            fOB_OFST_GB = -511.0f;
        }
        else if(fOB_OFST_GB > 0.0f){
            CAM_LOGE("fMOBC_OFST_GB (%d) Error (> 0)", fOB_OFST_GB);
            fOB_OFST_GB = 0.0f;
        }


        //OBC Gain
        reinterpret_cast<REG_MOBC_R2_MOBC_GAIN0*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, GAIN0))->Bits.MOBC_GAIN_B =
            ((512.0f * 4095.0f)/(4095.0f + fOB_OFST_B) + 0.5f);
        reinterpret_cast<REG_MOBC_R2_MOBC_GAIN1*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, GAIN1))->Bits.MOBC_GAIN_GB =
            ((512.0f * 4095.0f)/(4095.0f + fOB_OFST_GB) + 0.5f);
        reinterpret_cast<REG_MOBC_R2_MOBC_GAIN2*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, GAIN2))->Bits.MOBC_GAIN_GR =
            ((512.0f * 4095.0f)/(4095.0f + fOB_OFST_GR) + 0.5f);
        reinterpret_cast<REG_MOBC_R2_MOBC_GAIN3*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, GAIN3))->Bits.MOBC_GAIN_R =
            ((512.0f * 4095.0f)/(4095.0f + fOB_OFST_R) + 0.5f);

        rRawIspCamInfo.rOBC_OFST[0] = (MINT32)fOB_OFST_R;
        rRawIspCamInfo.rOBC_OFST[1] = (MINT32)fOB_OFST_GR;
        rRawIspCamInfo.rOBC_OFST[2] = (MINT32)fOB_OFST_GB;
        rRawIspCamInfo.rOBC_OFST[3] = (MINT32)fOB_OFST_B;

    }

    AAA_TRACE_DRV(DRV_MOBC);
    rTuning.tuningMgrWriteRegs( (TUNING_MGR_REG_IO_STRUCT*)(&(m_rIspRegInfo[SubModuleIndex][0])),
            m_u4RegInfoNum, i4SubsampleIdex);
    AAA_TRACE_END_DRV;

    dumpRegInfoP1("MOBC", SubModuleIndex);
*/
    return  MTRUE;
}



};
