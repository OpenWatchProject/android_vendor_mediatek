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
#define LOG_TAG "isp_mgr_lce"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include "isp_mgr.h"

#if defined(HAVE_AEE_FEATURE)
#include <aee.h>
#define AEE_ASSERT_LCE(String) \
          do { \
              aee_system_exception( \
                  LOG_TAG, \
                  NULL, \
                  DB_OPT_DEFAULT, \
                  String); \
          } while(0)
#else
#define AEE_ASSERT_LCE(String)
#endif

#include <private/aaa_utils.h>
#include <array>

using namespace NSIspTuning;

#define MY_INST NS3Av3::INST_T<ISP_MGR_LCE_T>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

namespace NSIspTuning
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  LCE
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_LCE_T&
ISP_MGR_LCE_T::
getInstance(MUINT32 const eSensorDev)
{
    ISP_MGR_MODULE_GET_INSTANCE(LCE);
}

template <>
ISP_MGR_LCE_T&
ISP_MGR_LCE_T::
put(MUINT8 SubModuleIndex, ISP_NVRAM_LCE_T const& rParam)
{
    //PUT_REG_INFO_MULTI(SubModuleIndex, CON,         con);
    //PUT_REG_INFO_MULTI(SubModuleIndex, ZR,          zr);
    //PUT_REG_INFO_MULTI(SubModuleIndex, SLM_SIZE,    slm_size);
    //PUT_REG_INFO_MULTI(SubModuleIndex, OFST,        ofst);
    //PUT_REG_INFO_MULTI(SubModuleIndex, BIAS,        bias);
    //PUT_REG_INFO_MULTI(SubModuleIndex, IMAGE_SIZE,  image_size);
    //PUT_REG_INFO_MULTI(SubModuleIndex, GLOBAL,      global);
    PUT_REG_INFO_MULTI(SubModuleIndex, CEN_PARA0,   cen_para0);
    PUT_REG_INFO_MULTI(SubModuleIndex, CEN_PARA1,   cen_para1);
    PUT_REG_INFO_MULTI(SubModuleIndex, CEN_PARA2,   cen_para2);
    //PUT_REG_INFO_MULTI(SubModuleIndex, BIL_TH0,     bil_th0);
    //PUT_REG_INFO_MULTI(SubModuleIndex, BIL_TH1,     bil_th1);
    PUT_REG_INFO_MULTI(SubModuleIndex, TM_PARA0,    tm_para0);
    PUT_REG_INFO_MULTI(SubModuleIndex, TM_PARA1,    tm_para1);
    PUT_REG_INFO_MULTI(SubModuleIndex, TM_PARA2,    tm_para2);
    PUT_REG_INFO_MULTI(SubModuleIndex, TM_PARA3,    tm_para3);
    PUT_REG_INFO_MULTI(SubModuleIndex, TM_PARA4,    tm_para4);
    PUT_REG_INFO_MULTI(SubModuleIndex, TM_PARA5,    tm_para5);
    PUT_REG_INFO_MULTI(SubModuleIndex, TM_PARA6,    tm_para6);
    PUT_REG_INFO_MULTI(SubModuleIndex, TM_PARA7,    tm_para7);
    PUT_REG_INFO_MULTI(SubModuleIndex, HLR_PARA0,   hlr_para0);
    PUT_REG_INFO_MULTI(SubModuleIndex, HLR_PARA1,   hlr_para1);
    PUT_REG_INFO_MULTI(SubModuleIndex, HLR_PARA2,   hlr_para2);
    PUT_REG_INFO_MULTI(SubModuleIndex, HLR_PARA3,   hlr_para3);
    PUT_REG_INFO_MULTI(SubModuleIndex, HLR_PARA4,   hlr_para4);
    PUT_REG_INFO_MULTI(SubModuleIndex, HLR_PARA5,   hlr_para5);
    PUT_REG_INFO_MULTI(SubModuleIndex, HLR_PARA6,   hlr_para6);
    PUT_REG_INFO_MULTI(SubModuleIndex, HLR_PARA7,   hlr_para7);
    PUT_REG_INFO_MULTI(SubModuleIndex, TCHL_PARA0,  tchl_para0);
    PUT_REG_INFO_MULTI(SubModuleIndex, TCHL_PARA1,  tchl_para1);
    //PUT_REG_INFO_MULTI(SubModuleIndex, DUM,         dum);

    return  (*this);
}

template <>
ISP_MGR_LCE_T&
ISP_MGR_LCE_T::
get(MUINT8 SubModuleIndex, ISP_NVRAM_LCE_T & rParam)
{
    //GET_REG_INFO_MULTI(SubModuleIndex, CON,         con);
    //GET_REG_INFO_MULTI(SubModuleIndex, ZR,          zr);
    //GET_REG_INFO_MULTI(SubModuleIndex, SLM_SIZE,    slm_size);
    //GET_REG_INFO_MULTI(SubModuleIndex, OFST,        ofst);
    //GET_REG_INFO_MULTI(SubModuleIndex, BIAS,        bias);
    //GET_REG_INFO_MULTI(SubModuleIndex, IMAGE_SIZE,  image_size);
    //GET_REG_INFO_MULTI(SubModuleIndex, GLOBAL,      global);
    GET_REG_INFO_MULTI(SubModuleIndex, CEN_PARA0,   cen_para0);
    GET_REG_INFO_MULTI(SubModuleIndex, CEN_PARA1,   cen_para1);
    GET_REG_INFO_MULTI(SubModuleIndex, CEN_PARA2,   cen_para2);
    //GET_REG_INFO_MULTI(SubModuleIndex, BIL_TH0,     bil_th0);
    //GET_REG_INFO_MULTI(SubModuleIndex, BIL_TH1,     bil_th1);
    GET_REG_INFO_MULTI(SubModuleIndex, TM_PARA0,    tm_para0);
    GET_REG_INFO_MULTI(SubModuleIndex, TM_PARA1,    tm_para1);
    GET_REG_INFO_MULTI(SubModuleIndex, TM_PARA2,    tm_para2);
    GET_REG_INFO_MULTI(SubModuleIndex, TM_PARA3,    tm_para3);
    GET_REG_INFO_MULTI(SubModuleIndex, TM_PARA4,    tm_para4);
    GET_REG_INFO_MULTI(SubModuleIndex, TM_PARA5,    tm_para5);
    GET_REG_INFO_MULTI(SubModuleIndex, TM_PARA6,    tm_para6);
    GET_REG_INFO_MULTI(SubModuleIndex, TM_PARA7,    tm_para7);
    GET_REG_INFO_MULTI(SubModuleIndex, HLR_PARA0,   hlr_para0);
    GET_REG_INFO_MULTI(SubModuleIndex, HLR_PARA1,   hlr_para1);
    GET_REG_INFO_MULTI(SubModuleIndex, HLR_PARA2,   hlr_para2);
    GET_REG_INFO_MULTI(SubModuleIndex, HLR_PARA3,   hlr_para3);
    GET_REG_INFO_MULTI(SubModuleIndex, HLR_PARA4,   hlr_para4);
    GET_REG_INFO_MULTI(SubModuleIndex, HLR_PARA5,   hlr_para5);
    GET_REG_INFO_MULTI(SubModuleIndex, HLR_PARA6,   hlr_para6);
    GET_REG_INFO_MULTI(SubModuleIndex, HLR_PARA7,   hlr_para7);
    GET_REG_INFO_MULTI(SubModuleIndex, TCHL_PARA0,  tchl_para0);
    GET_REG_INFO_MULTI(SubModuleIndex, TCHL_PARA1,  tchl_para1);
    //GET_REG_INFO_MULTI(SubModuleIndex, DUM,         dum);

    return  (*this);
}

MVOID
ISP_MGR_LCE_T::
transform_LCE(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo)
{
    MINT32 i4RrzInX     = rRawIspCamInfo.rCropRzInfo.sRRZin.w;
    MINT32 i4RrzInY     = rRawIspCamInfo.rCropRzInfo.sRRZin.h;
    MINT32 i4RrzXoff    = rRawIspCamInfo.rCropRzInfo.rRRZcrop.p.x;
    MINT32 i4RrzYoff    = rRawIspCamInfo.rCropRzInfo.rRRZcrop.p.y;
    MINT32 i4RrzCropInX = rRawIspCamInfo.rCropRzInfo.rRRZcrop.s.w;
    MINT32 i4RrzCropInY = rRawIspCamInfo.rCropRzInfo.rRRZcrop.s.h;
    MINT32 i4RrzOutX    = rRawIspCamInfo.rCropRzInfo.sRRZout.w;
    MINT32 i4RrzOutY    = rRawIspCamInfo.rCropRzInfo.sRRZout.h;

    if (i4RrzCropInX == 0 || i4RrzCropInY == 0)
    {
        CAM_LOGE("Unexpected crop info width(%d), height(%d)", i4RrzCropInX, i4RrzCropInY);
        AEE_ASSERT_LCE("Unexpected crop info for LCE");
    }
    else{
        CAM_LOGD("[transform_LCE] RrzInSize(%d, %d), RrzCropOfst(%d, %d), RrzCropSize(%d, %d), RrzOutSize(%d, %d)",
           i4RrzInX, i4RrzInY, i4RrzXoff, i4RrzYoff, i4RrzCropInX, i4RrzCropInY, i4RrzOutX, i4RrzOutY);

        reinterpret_cast<LCE_REG_D1A_LCE_ZR*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, ZR))->Bits.LCE_BCMK_X
            = 32768 * (rRawIspCamInfo.rLCS_Info.u4OutWidth-1) / (i4RrzOutX*i4RrzInX / i4RrzCropInX - 1);
        reinterpret_cast<LCE_REG_D1A_LCE_ZR*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, ZR))->Bits.LCE_BCMK_Y
            = 32768 * (rRawIspCamInfo.rLCS_Info.u4OutHeight-1) / (i4RrzOutY*i4RrzInY / i4RrzCropInY - 1);
        reinterpret_cast<LCE_REG_D1A_LCE_SLM_SIZE*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, SLM_SIZE))->Bits.LCE_SLM_WD
            = rRawIspCamInfo.rLCS_Info.u4OutWidth;
        reinterpret_cast<LCE_REG_D1A_LCE_SLM_SIZE*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, SLM_SIZE))->Bits.LCE_SLM_HT
            = rRawIspCamInfo.rLCS_Info.u4OutHeight;

        reinterpret_cast<LCE_REG_D1A_LCE_TPIPE_SLM*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, SLM))->Bits.LCE_TPIPE_SLM_WD
            = rRawIspCamInfo.rLCS_Info.u4OutWidth;
        reinterpret_cast<LCE_REG_D1A_LCE_TPIPE_SLM*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, SLM))->Bits.LCE_TPIPE_SLM_HT
            = rRawIspCamInfo.rLCS_Info.u4OutHeight;
        reinterpret_cast<LCE_REG_D1A_LCE_TPIPE_OFFSET*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, OFFSET))->Bits.LCE_TPIPE_OFFSET_X
            = i4RrzXoff * i4RrzOutX / i4RrzCropInX;;
        reinterpret_cast<LCE_REG_D1A_LCE_TPIPE_OFFSET*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, OFFSET))->Bits.LCE_TPIPE_OFFSET_Y
            = i4RrzYoff * i4RrzOutY / i4RrzCropInY;
        reinterpret_cast<LCE_REG_D1A_LCE_TPIPE_OUT*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, OUT))->Bits.LCE_TPIPE_OUT_HT
            = i4RrzInY * i4RrzOutY / i4RrzCropInY;
    }
}

MBOOL
ISP_MGR_LCE_T::
apply_P2(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, dip_x_reg_t* pReg)
{
     if(SubModuleIndex >= ESubModule_NUM){
        return MFALSE;
    }

    MBOOL bEnable = isEnable(SubModuleIndex);

    //Top Control
    switch (SubModuleIndex)
    {
        case ELCE_D1:
            ISP_WRITE_ENABLE_BITS(pReg, DIPCTL_D1A_DIPCTL_YUV_EN1, DIPCTL_LCE_D1_EN, bEnable);
            break;
        default:
            CAM_LOGE("Apply Error Submodule Index: %d", SubModuleIndex);
            return  MFALSE;
    }

    if(bEnable){
        transform_LCE(SubModuleIndex, rRawIspCamInfo);

        //CEN_PARA0 has another bit-field based on Algo result, so need set these per frame
        reinterpret_cast<LCE_REG_D1A_LCE_CEN_PARA0*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, CEN_PARA0))->Bits.LCE_CEN_END    = 320;
        reinterpret_cast<LCE_REG_D1A_LCE_CEN_PARA0*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, CEN_PARA0))->Bits.LCE_CEN_BLD_WT = 8;

        writeRegs(static_cast<RegInfo_T*>(m_rIspRegInfo[SubModuleIndex]), m_u4RegInfoNum, pReg);

        dumpRegInfoP2("LCE", SubModuleIndex);
    }

    return  MTRUE;
}

}

