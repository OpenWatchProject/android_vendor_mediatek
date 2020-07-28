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
#define LOG_TAG "isp_mgr_slk"

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


#define CLAMP(x,min,max)       (((x) > (max)) ? (max) : (((x) < (min)) ? (min) : (x)))

#define SLP_PREC_F_BW   16
#define SLP_PREC_F_SCAL 65536

static MINT32 _maxR(MINT32 cx, MINT32 cy, MINT32 w, MINT32 h)
{
    // Comparing which corner is the longest distance, and use it as R.

    //corner1
    MINT32 y_dist_corner1 = (0 - cy);
    MINT32 x_dist_corner1 = (0 - cx);
    MINT32 CircleEq_corner1;
    if (y_dist_corner1 < 0)
        y_dist_corner1 = -y_dist_corner1;
    if (x_dist_corner1 < 0)
        x_dist_corner1 = -x_dist_corner1;
    if (x_dist_corner1 >= y_dist_corner1)
        CircleEq_corner1 = x_dist_corner1;
    else
        CircleEq_corner1 = y_dist_corner1;

    MINT32 tmp_corner1 = ((x_dist_corner1 + y_dist_corner1)*1448) >> 11;
    if (tmp_corner1 >= CircleEq_corner1)
        CircleEq_corner1 = tmp_corner1;

    //corner 2
    MINT32 y_dist_corner2 = (0 - cy);
    MINT32 x_dist_corner2 = (w - cx);
    MINT32 CircleEq_corner2;

    if (y_dist_corner2 < 0)
        y_dist_corner2 = -y_dist_corner2;
    if (x_dist_corner2 < 0)
        x_dist_corner2 = -x_dist_corner2;
    if (x_dist_corner2 >= y_dist_corner2)
        CircleEq_corner2 = x_dist_corner2;
    else
        CircleEq_corner2 = y_dist_corner2;

    MINT32 tmp_corner2 = ((x_dist_corner2 + y_dist_corner2)*1448) >> 11;
    if(tmp_corner2 >= CircleEq_corner2)
        CircleEq_corner2 = tmp_corner2;

    //corner 3
    MINT32 y_dist_corner3 = (h - cy);
    MINT32 x_dist_corner3 = (0 - cx);
    MINT32 CircleEq_corner3;

    if (y_dist_corner3 < 0)
        y_dist_corner3 = -y_dist_corner3;
    if (x_dist_corner3 < 0)
        x_dist_corner3 = -x_dist_corner3;
    if (x_dist_corner3 >= y_dist_corner3)
        CircleEq_corner3 = x_dist_corner3;
    else
        CircleEq_corner3 = y_dist_corner3;

    MINT32 tmp_corner3 = ((x_dist_corner3 + y_dist_corner3)*1448) >> 11;
    if(tmp_corner3 >= CircleEq_corner3)
        CircleEq_corner3 = tmp_corner3;

    //corner4
    MINT32 y_dist_corner4 = (h - cy);
    MINT32 x_dist_corner4 = (w - cx);
    MINT32 CircleEq_corner4;

    if (y_dist_corner4 < 0)
        y_dist_corner4 = -y_dist_corner4;
    if (x_dist_corner4 < 0)
        x_dist_corner4 = -x_dist_corner4;
    if (x_dist_corner4 >= y_dist_corner4)
        CircleEq_corner4 = x_dist_corner4;
    else
        CircleEq_corner4 = y_dist_corner4;

    MINT32 tmp_corner4 = ((x_dist_corner4 + y_dist_corner4)*1448) >> 11;
    if(tmp_corner4 >= CircleEq_corner4)
        CircleEq_corner4 = tmp_corner4;

    MINT32 maxR;
    if (CircleEq_corner4 > CircleEq_corner3)
        maxR = CircleEq_corner4;
    else
        maxR = CircleEq_corner3;

    if (CircleEq_corner2 > maxR)
        maxR = CircleEq_corner2;

    if (CircleEq_corner1 > maxR)
        maxR = CircleEq_corner1;

    return maxR;
}
namespace NSIspTuning
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SLK
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ISP_MGR_SLK_T&
ISP_MGR_SLK_T::
getInstance(ESensorDev_T const eSensorDev)
{
    ISP_MGR_MODULE_GET_INSTANCE(SLK);
}

template <>
ISP_MGR_SLK_T&
ISP_MGR_SLK_T::
put(MUINT8 SubModuleIndex, ISP_NVRAM_SLK_T const& rParam)
{
    if (SubModuleIndex >= ESubModule_NUM) {
        CAM_LOGE("[PUT] Error Submodule Index: %d", SubModuleIndex);
        return (*this);
    }
    //PUT_REG_INFO_MULTI(SubModuleIndex, CEN,                       cen);
    //PUT_REG_INFO_MULTI(SubModuleIndex, RR_CON0,                   rr_con0);
    PUT_REG_INFO_MULTI(SubModuleIndex, RR_CON1,                   rr_con1);
    PUT_REG_INFO_MULTI(SubModuleIndex, GAIN,                      gain);
    //PUT_REG_INFO_MULTI(SubModuleIndex, RZ,                        rz);
    //PUT_REG_INFO_MULTI(SubModuleIndex, SLP_CON0,                  slp_con0);
    //PUT_REG_INFO_MULTI(SubModuleIndex, SLP_CON1,                  slp_con1);
    //PUT_REG_INFO_MULTI(SubModuleIndex, SLP_CON2,                  slp_con2);
    //PUT_REG_INFO_MULTI(SubModuleIndex, SLP_CON3,                  slp_con3);
    return (*this);
}

template <>
ISP_MGR_SLK_T&
ISP_MGR_SLK_T::
get(MUINT8 SubModuleIndex, ISP_NVRAM_SLK_T& rParam)
{
    if (SubModuleIndex >= ESubModule_NUM) {
        CAM_LOGE("[GET] Error Submodule Index: %d", SubModuleIndex);
        return (*this);
    }
    //GET_REG_INFO_MULTI(SubModuleIndex, CEN,                       cen);
    //GET_REG_INFO_MULTI(SubModuleIndex, RR_CON0,                   rr_con0);
    GET_REG_INFO_MULTI(SubModuleIndex, RR_CON1,                   rr_con1);
    GET_REG_INFO_MULTI(SubModuleIndex, GAIN,                      gain);
    //GET_REG_INFO_MULTI(SubModuleIndex, RZ,                        rz);
    //GET_REG_INFO_MULTI(SubModuleIndex, SLP_CON0,                  slp_con0);
    //GET_REG_INFO_MULTI(SubModuleIndex, SLP_CON1,                  slp_con1);
    //GET_REG_INFO_MULTI(SubModuleIndex, SLP_CON2,                  slp_con2);
    //GET_REG_INFO_MULTI(SubModuleIndex, SLP_CON3,                  slp_con3);
    return (*this);
}

MVOID
ISP_MGR_SLK_T::
transform_SLK(MUINT8 SubModuleIndex, const SL2_INFO_T& rSl2Info, const CROP_RZ_INFO_T& rCropRzInfo)
{
    MINT32 CENTR_X  = rSl2Info.i4CenterX;
    MINT32 CENTR_Y  = rSl2Info.i4CenterY;

    MINT32 maxR         = _maxR(CENTR_X, CENTR_Y,
                                rCropRzInfo.sRRZin.w,
                                rCropRzInfo.sRRZin.h);
    MINT32 R0       = rSl2Info.i4R0;
    MINT32 R1       = rSl2Info.i4R1;
    MINT32 R2       = rSl2Info.i4R2;

    MINT32 Gain0    = rSl2Info.i4Gain0;
    MINT32 Gain1    = rSl2Info.i4Gain1;
    MINT32 Gain2    = rSl2Info.i4Gain2;
    MINT32 Gain3    = rSl2Info.i4Gain3;
    MINT32 Gain4    = rSl2Info.i4Gain4;

    CAM_LOGD_IF(m_bDebugEnable,"SLK(%d) default hw is (Gain0, 1, 2, 3, 4) = (%d, %d, %d, %d, %d)",
                SubModuleIndex, Gain0, Gain1, Gain2, Gain3, Gain4);

    MINT32 HRZ_COM = 2048;
    MINT32 VRZ_COM = 2048;


    //P1 SLK modules would be re-calculated by P1 driver with RRZ info
    //Do not cosider "RRZ effect" on P1 SLK modules
    if ((SubModuleIndex >= ESLK_D1) && rCropRzInfo.fgRRZOnOff)
    {
        if (rCropRzInfo.sRRZout.w == 0 || rCropRzInfo.sRRZout.h == 0 ||
            rCropRzInfo.rRRZcrop.s.w == 0 || rCropRzInfo.rRRZcrop.s.h == 0 ||
            rCropRzInfo.sRRZin.w == 0 || rCropRzInfo.sRRZin.h == 0)
        {
            CAM_LOGE("Unexpected crop info Submodule(%d), sRRZout.w(%d), sRRZout.h(%d), rRRZcrop.s.w(%d), rRRZcrop.s.h(%d), sRRZin.w(%d), sRRZin.h(%d)",
                      SubModuleIndex,
                      rCropRzInfo.sRRZout.w, rCropRzInfo.sRRZout.h,
                      rCropRzInfo.rRRZcrop.s.w, rCropRzInfo.rRRZcrop.s.h,
                      rCropRzInfo.sRRZin.w, rCropRzInfo.sRRZin.h);

        }
        else
        {
            CENTR_X = (CENTR_X - rCropRzInfo.rRRZcrop.p.x);
            CENTR_Y = (CENTR_Y - rCropRzInfo.rRRZcrop.p.y);

            MFLOAT fRzRtoX = (MFLOAT)rCropRzInfo.sRRZout.w / (MFLOAT)rCropRzInfo.rRRZcrop.s.w;
            MFLOAT fRzRtoY = (MFLOAT)rCropRzInfo.sRRZout.h / (MFLOAT)rCropRzInfo.rRRZcrop.s.h;

            HRZ_COM = (MINT32)((MFLOAT)HRZ_COM / fRzRtoX);
            VRZ_COM = (MINT32)((MFLOAT)VRZ_COM / fRzRtoY);
            CAM_LOGD_IF(m_bDebugEnable,"SLK fRzRtoX(%f), fRzRtoY(%f)",fRzRtoX, fRzRtoY);

        }
    }

    //Evalutaing slope
    MINT32 SLP_1  = (MINT32)(((float)(Gain1 - Gain0)/(float)R0)*SLP_PREC_F_SCAL + 0.5);
    MINT32 SLP_2  = (MINT32)(((float)(Gain2 - Gain1)/(float)(R1 - R0))*SLP_PREC_F_SCAL + 0.5);
    MINT32 SLP_3  = (MINT32)(((float)(Gain3 - Gain2)/(float)(R2 - R1))*SLP_PREC_F_SCAL + 0.5);
    MINT32 SLP_4  = (MINT32)(((float)(Gain4 - Gain3)/(float)(maxR - R2))*SLP_PREC_F_SCAL + 0.5);

    reinterpret_cast<REG_SLK_R1_SLK_CEN*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, CEN))->Bits.SLK_CENTR_X = CENTR_X;
    reinterpret_cast<REG_SLK_R1_SLK_CEN*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, CEN))->Bits.SLK_CENTR_Y = CENTR_Y;

    reinterpret_cast<REG_SLK_R1_SLK_RR_CON0*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, RR_CON0))->Bits.SLK_R_0 = R0;
    reinterpret_cast<REG_SLK_R1_SLK_RR_CON0*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, RR_CON0))->Bits.SLK_R_1 = R1;
    reinterpret_cast<REG_SLK_R1_SLK_RR_CON1*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, RR_CON1))->Bits.SLK_R_2 = R2;

    reinterpret_cast<REG_SLK_R1_SLK_RR_CON1*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, RR_CON1))->Bits.SLK_GAIN_0=Gain0;
    reinterpret_cast<REG_SLK_R1_SLK_RR_CON1*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, RR_CON1))->Bits.SLK_GAIN_1=Gain1;
    reinterpret_cast<REG_SLK_R1_SLK_GAIN*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, GAIN))->Bits.SLK_GAIN_2=Gain2;
    reinterpret_cast<REG_SLK_R1_SLK_GAIN*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, GAIN))->Bits.SLK_GAIN_3=Gain3;
    reinterpret_cast<REG_SLK_R1_SLK_GAIN*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, GAIN))->Bits.SLK_GAIN_4=Gain4;

    reinterpret_cast<REG_SLK_R1_SLK_RZ*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, RZ))->Bits.SLK_HRZ_COMP = HRZ_COM;
    reinterpret_cast<REG_SLK_R1_SLK_RZ*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, RZ))->Bits.SLK_VRZ_COMP = VRZ_COM;

    reinterpret_cast<REG_SLK_R1_SLK_SLP_CON0*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, SLP_CON0))->Bits.SLK_SLP_1 = SLP_1;
    reinterpret_cast<REG_SLK_R1_SLK_SLP_CON1*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, SLP_CON1))->Bits.SLK_SLP_2 = SLP_2;
    reinterpret_cast<REG_SLK_R1_SLK_SLP_CON2*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, SLP_CON2))->Bits.SLK_SLP_3 = SLP_3;
    reinterpret_cast<REG_SLK_R1_SLK_SLP_CON3*>(REG_INFO_VALUE_PTR_MULTI(SubModuleIndex, SLP_CON3))->Bits.SLK_SLP_4 = SLP_4;

    CAM_LOGD_IF(m_bDebugEnable,"Transform SLK CENTR_X(%d), CENTR_Y(%d), R0(%d), R1(%d), R2(%d) HRZ_COM(%d) VRZ_COM(%d) sRRZout.w(%d) sRRZout.h(%d) rRRZcrop.s.w(%d) rRRZcrop.s.h(%d)",
        CENTR_X, CENTR_Y, R0, R1, R2, HRZ_COM, VRZ_COM, rCropRzInfo.sRRZout.w, rCropRzInfo.sRRZout.h,
        rCropRzInfo.rRRZcrop.s.w, rCropRzInfo.rRRZcrop.s.h);
}

MBOOL
ISP_MGR_SLK_T::
apply_P1(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, TuningMgr& rTuning, MINT32 i4SubsampleIdex)
{
    if(SubModuleIndex >= ESubModule_NUM){
        return MFALSE;
    }

    MBOOL bEnable = isEnable(SubModuleIndex);

    if(bEnable) transform_SLK(SubModuleIndex, rRawIspCamInfo.rSl2Info, rRawIspCamInfo.rCropRzInfo);

    //Top Control
    switch (SubModuleIndex)
    {
        case ESLK_R1:
            ISP_MGR_CAMCTL_T::getInstance(m_eSensorDev).setEnable_SLK_R1(bEnable);
            rTuning.updateEngine(eTuningMgrFunc_SLK_R1, MTRUE, i4SubsampleIdex);
            break;
        default:
            CAM_LOGE("Apply Error Submodule Index: %d", SubModuleIndex);
            return  MFALSE;
    }


    AAA_TRACE_DRV(DRV_SLK);
    rTuning.tuningMgrWriteRegs( (TUNING_MGR_REG_IO_STRUCT*)(&(m_rIspRegInfo[SubModuleIndex][0])),
        m_u4RegInfoNum, i4SubsampleIdex);
    AAA_TRACE_END_DRV;

    dumpRegInfoP1("SLK", SubModuleIndex);

    return  MTRUE;

}

MBOOL
ISP_MGR_SLK_T::

apply_P2(MUINT8 SubModuleIndex, const RAWIspCamInfo& rRawIspCamInfo, dip_a_reg_t* pReg)
{
    if(SubModuleIndex >= ESubModule_NUM){
        return MFALSE;
    }

    MBOOL bEnable = isEnable(SubModuleIndex);

    if(bEnable){

        SL2_INFO_T  _Sl2Info = rRawIspCamInfo.rSl2Info;
        if(m_bLSC2Enable && rRawIspCamInfo.rCropRzInfo.fgFBinOnOff)
        {
            // SL2 info with bin effector in lsc, but lsc2 should be without it
            _Sl2Info.i4CenterX = _Sl2Info.i4CenterX * 2;
            _Sl2Info.i4CenterY = _Sl2Info.i4CenterY * 2;

            MINT32 u4R = ((_Sl2Info.i4CenterX + _Sl2Info.i4CenterY) *1448) >> 11;
            _Sl2Info.i4R0 = u4R * 3 / 10;
            _Sl2Info.i4R1 = u4R * 6 / 10;
            _Sl2Info.i4R2 = u4R * 8 / 10;

            CAM_LOGD("re-calculate SLK info Index: %d", SubModuleIndex);
        } else {
            CAM_LOGD("RzIn(w/h)=(%d/%d), tg(w/h)=(%d/%d)",
                rRawIspCamInfo.rCropRzInfo.sRRZin.w, rRawIspCamInfo.rCropRzInfo.sRRZin.h,
                rRawIspCamInfo.rCropRzInfo.sTGout.w, rRawIspCamInfo.rCropRzInfo.sTGout.h);
        }

        transform_SLK(SubModuleIndex, _Sl2Info, rRawIspCamInfo.rCropRzInfo);

        //Top Control
        switch (SubModuleIndex)
        {
            case ESLK_D1:
                ISP_WRITE_ENABLE_BITS(pReg, DIPCTL_D1A_DIPCTL_RGB_EN1, DIPCTL_SLK_D1_EN, bEnable);
                break;
            case ESLK_D2:
                ISP_WRITE_ENABLE_BITS(pReg, DIPCTL_D1A_DIPCTL_YUV_EN2, DIPCTL_SLK_D2_EN, bEnable);
                break;
            case ESLK_D3:
                ISP_WRITE_ENABLE_BITS(pReg, DIPCTL_D1A_DIPCTL_YUV_EN2, DIPCTL_SLK_D3_EN, bEnable);
                break;
            case ESLK_D4:
                ISP_WRITE_ENABLE_BITS(pReg, DIPCTL_D1A_DIPCTL_YUV_EN2, DIPCTL_SLK_D4_EN, bEnable);
                break;
            case ESLK_D5:
                ISP_WRITE_ENABLE_BITS(pReg, DIPCTL_D1A_DIPCTL_YUV_EN2, DIPCTL_SLK_D5_EN, bEnable);
                break;
            case ESLK_D6:
                ISP_WRITE_ENABLE_BITS(pReg, DIPCTL_D1A_DIPCTL_RGB_EN1, DIPCTL_SLK_D6_EN, bEnable);
                break;
            default:
                CAM_LOGE("Apply Error Submodule Index: %d", SubModuleIndex);
                return  MFALSE;
        }

        writeRegs(static_cast<RegInfo_T*>(m_rIspRegInfo[SubModuleIndex]), m_u4RegInfoNum, pReg);

        dumpRegInfoP2("SLK", SubModuleIndex);
    }
    return  MTRUE;
}

};
