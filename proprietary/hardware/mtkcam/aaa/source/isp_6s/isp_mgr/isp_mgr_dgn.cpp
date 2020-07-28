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
#define LOG_TAG "isp_mgr_dgn"

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
// DGN
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ISP_MGR_DGN_T&
ISP_MGR_DGN_T::
getInstance(ESensorDev_T const eSensorDev)
{
    ISP_MGR_MODULE_GET_INSTANCE(DGN);
}

template <>
ISP_MGR_DGN_T&
ISP_MGR_DGN_T::
put(MUINT8 SubModuleIndex, ISP_NVRAM_DGN_T const& rParam)
{
    if (SubModuleIndex >= ESubModule_NUM) {
        CAM_LOGE("[PUT] Error Submodule Index: %d", SubModuleIndex);
        return (*this);
    }
    PUT_REG_INFO_MULTI(SubModuleIndex, GN0,                       gn0);
    PUT_REG_INFO_MULTI(SubModuleIndex, GN1,                       gn1);
    PUT_REG_INFO_MULTI(SubModuleIndex, GN2,                       gn2);
    PUT_REG_INFO_MULTI(SubModuleIndex, GN3,                       gn3);
    //PUT_REG_INFO_MULTI(SubModuleIndex, LINK,                      link);
    return (*this);
}

template <>
ISP_MGR_DGN_T&
ISP_MGR_DGN_T::
get(MUINT8 SubModuleIndex, ISP_NVRAM_DGN_T& rParam)
{
    if (SubModuleIndex >= ESubModule_NUM) {
        CAM_LOGE("[GET] Error Submodule Index: %d", SubModuleIndex);
        return (*this);
    }
    GET_REG_INFO_MULTI(SubModuleIndex, GN0,                       gn0);
    GET_REG_INFO_MULTI(SubModuleIndex, GN1,                       gn1);
    GET_REG_INFO_MULTI(SubModuleIndex, GN2,                       gn2);
    GET_REG_INFO_MULTI(SubModuleIndex, GN3,                       gn3);
    //GET_REG_INFO_MULTI(SubModuleIndex, LINK,                      link);
    return (*this);
}

MBOOL
ISP_MGR_DGN_T::
apply_P1(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, TuningMgr& rTuning, MINT32 i4SubsampleIdex)
{
    if(SubModuleIndex >= ESubModule_NUM){
        return MFALSE;
    }

    MBOOL bEnable = isEnable(SubModuleIndex);
    MBOOL bUpdate = (rRawIspCamInfo.u4Id == 1)||( bEnable &&(!rRawIspCamInfo.rAEInfo.bDisableDGN) );

    //Top Control for CCU
    switch (SubModuleIndex)
    {
        case EDGN_R1:
            ISP_MGR_CAMCTL_T::getInstance(m_eSensorDev).setEnable_DGN_R1(bEnable);
            // Top Control
            rTuning.enableEngine(eTuningMgrFunc_DGN_R1, bEnable, i4SubsampleIdex);
            // Update
            rTuning.tuningEngine(eTuningMgrFunc_DGN_R1, bUpdate, i4SubsampleIdex);
            break;
        default:
            CAM_LOGE("Apply Error Submodule Index: %d", SubModuleIndex);
            return  MFALSE;
    }

    if(bUpdate){
        //1.0x = 4096
        reinterpret_cast<REG_DGN_R1_DGN_GN0*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, GN0))->Bits.DGN_GAIN_B  = rRawIspCamInfo.rAEInfo.u4P1DGNGain ;
        reinterpret_cast<REG_DGN_R1_DGN_GN1*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, GN1))->Bits.DGN_GAIN_GB = rRawIspCamInfo.rAEInfo.u4P1DGNGain ;
        reinterpret_cast<REG_DGN_R1_DGN_GN2*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, GN2))->Bits.DGN_GAIN_GR = rRawIspCamInfo.rAEInfo.u4P1DGNGain ;
        reinterpret_cast<REG_DGN_R1_DGN_GN3*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, GN3))->Bits.DGN_GAIN_R  = rRawIspCamInfo.rAEInfo.u4P1DGNGain ;

        AAA_TRACE_DRV(DRV_DGN);
			char value[PROPERTY_VALUE_MAX] = {'\0'};
			property_get("vendor.debug.isp_mgr.dump", value, "0");
			MBOOL bEn = atoi(value);
		
#define ISP_MGR_DGN_PARAM_SIZE       (4)
		
			if (bEn == 1){
				char strTuningFile[512] = {'\0'};
				sprintf(strTuningFile, "data/vendor/camera_dump/dgn_mgr.param");
				FILE* fidTuning = fopen(strTuningFile, "wb");
				if (fidTuning)
				{
				  fwrite(&(m_rIspRegInfo[SubModuleIndex][0]), sizeof(RegInfo_T)*ISP_MGR_DGN_PARAM_SIZE, 1, fidTuning);
				  fclose(fidTuning);
				}
			}
			if (bEn == 2){
				FILE* fidTuning = fopen("data/vendor/camera_dump/dgn_mgr.param", "rb");
				if (fidTuning)
				{
					RegInfo_T	tempInfo[ISP_MGR_DGN_PARAM_SIZE];
					fread((&tempInfo), sizeof(tempInfo), 1, fidTuning);
					fclose(fidTuning);
					rTuning.tuningMgrWriteRegs( (TUNING_MGR_REG_IO_STRUCT*)(&(tempInfo)),
						ISP_MGR_DGN_PARAM_SIZE, i4SubsampleIdex);
		
				}
			} else {
				rTuning.tuningMgrWriteRegs( (TUNING_MGR_REG_IO_STRUCT*)(&(m_rIspRegInfo[SubModuleIndex][0])),
						m_u4RegInfoNum, i4SubsampleIdex);

			}
        AAA_TRACE_END_DRV;
    }

    dumpRegInfoP1("DGN", SubModuleIndex);

    return  MTRUE;
}

MBOOL
ISP_MGR_DGN_T::
apply_P2(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, dip_a_reg_t* pReg)
{
    if(SubModuleIndex >= ESubModule_NUM){
        return MFALSE;
    }

    MBOOL bEnable = isEnable(SubModuleIndex);

    //Top Control
    switch (SubModuleIndex)
    {
        case EDGN_D1:
            ISP_WRITE_ENABLE_BITS(pReg, DIPCTL_D1A_DIPCTL_RGB_EN1, DIPCTL_DGN_D1_EN, bEnable);
            break;
        default:
            CAM_LOGE("Apply Error Submodule Index: %d", SubModuleIndex);
            return  MFALSE;
    }

    if(bEnable){
        //1.0x = 4096
        reinterpret_cast<DGN_REG_D1A_DGN_GN0*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, GN0))->Bits.DGN_GAIN_B = rRawIspCamInfo.rAEInfo.u4P2DGNGain ;
        reinterpret_cast<DGN_REG_D1A_DGN_GN1*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, GN1))->Bits.DGN_GAIN_GB = rRawIspCamInfo.rAEInfo.u4P2DGNGain ;
        reinterpret_cast<DGN_REG_D1A_DGN_GN2*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, GN2))->Bits.DGN_GAIN_GR = rRawIspCamInfo.rAEInfo.u4P2DGNGain ;
        reinterpret_cast<DGN_REG_D1A_DGN_GN3*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, GN3))->Bits.DGN_GAIN_R = rRawIspCamInfo.rAEInfo.u4P2DGNGain ;

        writeRegs(static_cast<RegInfo_T*>(m_rIspRegInfo[SubModuleIndex]), m_u4RegInfoNum, pReg);

        dumpRegInfoP2("DGN", SubModuleIndex);
    }

    return  MTRUE;
}

};
