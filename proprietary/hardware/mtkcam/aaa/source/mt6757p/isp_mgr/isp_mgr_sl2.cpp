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
#define LOG_TAG "isp_mgr_sl2"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include <camera_custom_nvram.h>
#include <awb_feature.h>
#include <awb_param.h>
#include <ae_feature.h>
#include <ae_param.h>

#include "isp_mgr.h"
#include <tuning_mgr.h>

#if defined(HAVE_AEE_FEATURE)
#include <aee.h>
#define AEE_ASSERT_SL2(String) \
          do { \
              aee_system_exception( \
                  LOG_TAG, \
                  NULL, \
                  DB_OPT_DEFAULT, \
                  String); \
          } while(0)
#else
#define AEE_ASSERT_SL2(String)
#endif

#define CLAMP(x,min,max)       (((x) > (max)) ? (max) : (((x) < (min)) ? (min) : (x)))

#define SLP_PREC_F_SCAL 256

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

namespace NSIspTuningv3
{


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SL2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_SL2_T&
ISP_MGR_SL2_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
{
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_SL2_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_SL2_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_SL2_DEV<ESensorDev_Sub>::getInstance();
    case ESensorDev_SubSecond: //  Sub Sensor
        return  ISP_MGR_SL2_DEV<ESensorDev_SubSecond>::getInstance();
    default:
        CAM_LOGE("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_SL2_DEV<ESensorDev_Main>::getInstance();
    }
}

template <>
ISP_MGR_SL2_T&
ISP_MGR_SL2_T::
put(ISP_NVRAM_SL2_T const& rParam)
{

    m_rSL2Param = rParam;

    return  (*this);
}


MVOID
ISP_MGR_SL2_T::
transform_SL2(const CROP_RZ_INFO_T& rCropRz, ISP_NVRAM_SL2_T const& rParam)
{
    MINT32 SL2_CENTR_X  = rParam.cen.bits.SL2_CENTR_X;
    MINT32 SL2_CENTR_Y  = rParam.cen.bits.SL2_CENTR_Y;
    MINT32 maxR         = _maxR(SL2_CENTR_X, SL2_CENTR_Y, rCropRz.i4FullW, rCropRz.i4FullH);
    MINT32 tempGain0    = rParam.rr_con1.bits.SL2_GAIN_0;
    MINT32 tempGain1    = rParam.rr_con1.bits.SL2_GAIN_1;
    MINT32 tempGain2    = rParam.gain.bits.SL2_GAIN_2;
    MINT32 tempGain3    = rParam.gain.bits.SL2_GAIN_3;
    MINT32 tempGain4    = rParam.gain.bits.SL2_GAIN_4;
    MINT32 tempR0       = rParam.rr_con0.bits.SL2_R_0;
    MINT32 tempR1       = rParam.rr_con0.bits.SL2_R_1;
    MINT32 tempR2       = rParam.rr_con1.bits.SL2_R_2;


    MINT32 SL2_HRZ_COM = 2048;
    MINT32 SL2_VRZ_COM = 2048;

    if (rCropRz.fgOnOff)
    {
        if (rCropRz.i4RzWidth == 0 || rCropRz.i4Width == 0)
        {
            CAM_LOGE("Unexpected crop info width(%d), rz width(%d)", rCropRz.i4Width, rCropRz.i4RzWidth);
            AEE_ASSERT_SL2("Unexpected crop info for SL2");
        }
        else
        {
            MFLOAT fRzRtoX = (MFLOAT)rCropRz.i4RzWidth / (MFLOAT)rCropRz.i4Width;
            MFLOAT fRzRtoY = (MFLOAT)rCropRz.i4RzHeight / (MFLOAT)rCropRz.i4Height;
            SL2_CENTR_X = (SL2_CENTR_X - rCropRz.i4OfstX);
            SL2_CENTR_Y = (SL2_CENTR_Y - rCropRz.i4OfstY);

            SL2_HRZ_COM = (MINT32)((MFLOAT)SL2_HRZ_COM / fRzRtoX);
            SL2_VRZ_COM = (MINT32)((MFLOAT)SL2_VRZ_COM / fRzRtoY);

        }
    }

    //Evalutaing slope
    MINT32 SL2_SLP_1  = (MINT32)(((float)(tempGain1 - tempGain0)/(float)tempR0)*SLP_PREC_F_SCAL + 0.5);
    MINT32 SL2_SLP_2  = (MINT32)(((float)(tempGain2 - tempGain1)/(float)(tempR1 - tempR0))*SLP_PREC_F_SCAL + 0.5);
    MINT32 SL2_SLP_3  = (MINT32)(((float)(tempGain3 - tempGain2)/(float)(tempR2 - tempR1))*SLP_PREC_F_SCAL + 0.5);
    MINT32 SL2_SLP_4  = (MINT32)(((float)(tempGain4 - tempGain3)/(float)(maxR -   tempR2))*SLP_PREC_F_SCAL + 0.5);

    DIP_X_REG_SL2_CEN rCen;
    DIP_X_REG_SL2_RR_CON0 rRRCond0;
    DIP_X_REG_SL2_RR_CON1 rRRCond1;
    DIP_X_REG_SL2_GAIN rGain;
    DIP_X_REG_SL2_SLP_CON0 rSlopCon0;
    DIP_X_REG_SL2_SLP_CON1 rSlopCon1;

    rCen.Raw=0;
    rRRCond0.Raw=0;
    rRRCond1.Raw=0;
    rGain.Raw=0;
    rSlopCon0.Raw=0;
    rSlopCon1.Raw=0;

    rCen.Bits.SL2_CENTR_X = SL2_CENTR_X;
    rCen.Bits.SL2_CENTR_Y = SL2_CENTR_Y;
    rRRCond0.Bits.SL2_R_0 = tempR0;
    rRRCond0.Bits.SL2_R_1 = tempR1;
    rRRCond1.Bits.SL2_R_2 = tempR2;
    rRRCond1.Bits.SL2_GAIN_0 = tempGain0;
    rRRCond1.Bits.SL2_GAIN_1 = tempGain1;
    rGain.Bits.SL2_GAIN_2 = tempGain2;
    rGain.Bits.SL2_GAIN_3 = tempGain3;
    rGain.Bits.SL2_GAIN_4 = tempGain4;
    rSlopCon0.Bits.SL2_SLP_1 = SL2_SLP_1;
    rSlopCon0.Bits.SL2_SLP_2 = SL2_SLP_2;
    rSlopCon1.Bits.SL2_SLP_3 = SL2_SLP_3;
    rSlopCon1.Bits.SL2_SLP_4 = SL2_SLP_4;

    if(isEnable()){
    REG_INFO_VALUE(DIP_X_SL2_CEN)       = rCen.Raw;
    REG_INFO_VALUE(DIP_X_SL2_RR_CON0)   = rRRCond0.Raw;
    REG_INFO_VALUE(DIP_X_SL2_RR_CON1)   = rRRCond1.Raw;
    REG_INFO_VALUE(DIP_X_SL2_GAIN)      = rGain.Raw;
    REG_INFO_VALUE(DIP_X_SL2_SLP_CON0)  = rSlopCon0.Raw;
    REG_INFO_VALUE(DIP_X_SL2_SLP_CON1)  = rSlopCon1.Raw;
    REG_INFO_VALUE(DIP_X_SL2_RZ)        = (SL2_VRZ_COM<<16) | SL2_HRZ_COM;
    REG_INFO_VALUE(DIP_X_SL2_XOFF)      = 0;
    REG_INFO_VALUE(DIP_X_SL2_YOFF)      = 0;
    }

    if(getSL2BOnOff()){
    REG_INFO_VALUE(DIP_X_SL2B_CEN)      = rCen.Raw;
    REG_INFO_VALUE(DIP_X_SL2B_RR_CON0)  = rRRCond0.Raw;
    REG_INFO_VALUE(DIP_X_SL2B_RR_CON1)  = rRRCond1.Raw;
    REG_INFO_VALUE(DIP_X_SL2B_GAIN)     = rGain.Raw;
    REG_INFO_VALUE(DIP_X_SL2B_SLP_CON0) = rSlopCon0.Raw;
    REG_INFO_VALUE(DIP_X_SL2B_SLP_CON1) = rSlopCon1.Raw;
    REG_INFO_VALUE(DIP_X_SL2B_RZ)       = (SL2_VRZ_COM<<16) | SL2_HRZ_COM;
    REG_INFO_VALUE(DIP_X_SL2B_XOFF)     = 0;
    REG_INFO_VALUE(DIP_X_SL2B_YOFF)     = 0;
    }

    if(getSL2COnOff()){
    REG_INFO_VALUE(DIP_X_SL2C_CEN)      = rCen.Raw;
    REG_INFO_VALUE(DIP_X_SL2C_RR_CON0)  = rRRCond0.Raw;
    REG_INFO_VALUE(DIP_X_SL2C_RR_CON1)  = rRRCond1.Raw;
    REG_INFO_VALUE(DIP_X_SL2C_GAIN)     = rGain.Raw;
    REG_INFO_VALUE(DIP_X_SL2C_SLP_CON0) = rSlopCon0.Raw;
    REG_INFO_VALUE(DIP_X_SL2C_SLP_CON1) = rSlopCon1.Raw;
    REG_INFO_VALUE(DIP_X_SL2C_RZ)       = (SL2_VRZ_COM<<16) | SL2_HRZ_COM;
    REG_INFO_VALUE(DIP_X_SL2C_XOFF)     = 0;
    REG_INFO_VALUE(DIP_X_SL2C_YOFF)     = 0;
    }

    if(getSL2DOnOff()){
    REG_INFO_VALUE(DIP_X_SL2D_CEN)      = rCen.Raw;
    REG_INFO_VALUE(DIP_X_SL2D_RR_CON0)  = rRRCond0.Raw;
    REG_INFO_VALUE(DIP_X_SL2D_RR_CON1)  = rRRCond1.Raw;
    REG_INFO_VALUE(DIP_X_SL2D_GAIN)     = rGain.Raw;
    REG_INFO_VALUE(DIP_X_SL2D_SLP_CON0) = rSlopCon0.Raw;
    REG_INFO_VALUE(DIP_X_SL2D_SLP_CON1) = rSlopCon1.Raw;
    REG_INFO_VALUE(DIP_X_SL2D_RZ)       = (SL2_VRZ_COM<<16) | SL2_HRZ_COM;
    REG_INFO_VALUE(DIP_X_SL2D_XOFF)     = 0;
    REG_INFO_VALUE(DIP_X_SL2D_YOFF)     = 0;
    }

    if(getSL2HOnOff()){
    REG_INFO_VALUE(DIP_X_SL2H_CEN)      = rCen.Raw;
    REG_INFO_VALUE(DIP_X_SL2H_RR_CON0)  = rRRCond0.Raw;
    REG_INFO_VALUE(DIP_X_SL2H_RR_CON1)  = rRRCond1.Raw;
    REG_INFO_VALUE(DIP_X_SL2H_GAIN)     = rGain.Raw;
    REG_INFO_VALUE(DIP_X_SL2H_SLP_CON0) = rSlopCon0.Raw;
    REG_INFO_VALUE(DIP_X_SL2H_SLP_CON1) = rSlopCon1.Raw;
    REG_INFO_VALUE(DIP_X_SL2H_RZ)       = (SL2_VRZ_COM<<16) | SL2_HRZ_COM;
    REG_INFO_VALUE(DIP_X_SL2H_XOFF)     = 0;
    REG_INFO_VALUE(DIP_X_SL2H_YOFF)     = 0;
    }

    if(getSL2IOnOff()){
    REG_INFO_VALUE(DIP_X_SL2I_CEN)      = rCen.Raw;
    REG_INFO_VALUE(DIP_X_SL2I_RR_CON0)  = rRRCond0.Raw;
    REG_INFO_VALUE(DIP_X_SL2I_RR_CON1)  = rRRCond1.Raw;
    REG_INFO_VALUE(DIP_X_SL2I_GAIN)     = rGain.Raw;
    REG_INFO_VALUE(DIP_X_SL2I_SLP_CON0) = rSlopCon0.Raw;
    REG_INFO_VALUE(DIP_X_SL2I_SLP_CON1) = rSlopCon1.Raw;
    REG_INFO_VALUE(DIP_X_SL2I_RZ)       = (SL2_VRZ_COM<<16) | SL2_HRZ_COM;
    REG_INFO_VALUE(DIP_X_SL2I_XOFF)     = 0;
    REG_INFO_VALUE(DIP_X_SL2I_YOFF)     = 0;
    }

    if(getSL2EOnOff()){
    REG_INFO_VALUE(DIP_X_SL2E_CEN)      = rCen.Raw;
    REG_INFO_VALUE(DIP_X_SL2E_RR_CON0)  = rRRCond0.Raw;
    REG_INFO_VALUE(DIP_X_SL2E_RR_CON1)  = rRRCond1.Raw;
    REG_INFO_VALUE(DIP_X_SL2E_GAIN)     = rGain.Raw;
    REG_INFO_VALUE(DIP_X_SL2E_SLP_CON0) = rSlopCon0.Raw;
    REG_INFO_VALUE(DIP_X_SL2E_SLP_CON1) = rSlopCon1.Raw;
    REG_INFO_VALUE(DIP_X_SL2E_RZ)       = (SL2_VRZ_COM<<16) | SL2_HRZ_COM;
    REG_INFO_VALUE(DIP_X_SL2E_XOFF)     = 0;
    REG_INFO_VALUE(DIP_X_SL2E_YOFF)     = 0;
    }

}


MVOID
ISP_MGR_SL2_T::
transform_SL2e(const CROP_RZ_INFO_T& rCropRz, ISP_NVRAM_SL2_T const& rParam)
{
    MINT32 SL2_CENTR_X  = rParam.cen.bits.SL2_CENTR_X;
    MINT32 SL2_CENTR_Y  = rParam.cen.bits.SL2_CENTR_Y;
    MINT32 i4Sl2Width   = rCropRz.i4FullW;
    MINT32 i4Sl2Height  = rCropRz.i4FullH;

    if (rCropRz.fgOnOff)
    {
        if (rCropRz.i4RzWidth == 0 || rCropRz.i4Width == 0)
        {
            CAM_LOGE("Unexpected crop info width(%d), rz width(%d)", rCropRz.i4Width, rCropRz.i4RzWidth);
            AEE_ASSERT_SL2("Unexpected crop info for SL2E");
        }
        else{
        MFLOAT fRzRto = (MFLOAT)rCropRz.i4RzWidth / rCropRz.i4Width;
        SL2_CENTR_X = (MFLOAT)(SL2_CENTR_X - rCropRz.i4OfstX)*fRzRto;
        SL2_CENTR_Y = (MFLOAT)(SL2_CENTR_Y - rCropRz.i4OfstY)*fRzRto;
        i4Sl2Width  = rCropRz.i4RzWidth;
        i4Sl2Height = rCropRz.i4RzHeight;
        }
    }

    MINT32 maxR         = _maxR(SL2_CENTR_X, SL2_CENTR_Y, i4Sl2Width, i4Sl2Height);
    MINT32 tempGain0    = 0;        //rParam.rr_con1.bits.SL2_GAIN_0;
    MINT32 tempGain1    = 64;       //rParam.rr_con1.bits.SL2_GAIN_1;
    MINT32 tempGain2    = 128;      //rParam.gain.bits.SL2_GAIN_2;
    MINT32 tempGain3    = 192;      //rParam.gain.bits.SL2_GAIN_3;
    MINT32 tempGain4    = 255;      //rParam.gain.bits.SL2_GAIN_4;
    MINT32 tempR0       = maxR*1/4; //rParam.rr_con0.bits.SL2_R_0;
    MINT32 tempR1       = maxR*2/4; //rParam.rr_con0.bits.SL2_R_1;
    MINT32 tempR2       = maxR*3/4; //rParam.rr_con1.bits.SL2_R_2;

    //Evalutaing slope
    MINT32 SL2_SLP_1  = (MINT32)(((float)(tempGain1 - tempGain0)/(float)tempR0)*SLP_PREC_F_SCAL + 0.5);
    MINT32 SL2_SLP_2  = (MINT32)(((float)(tempGain2 - tempGain1)/(float)(tempR1 - tempR0))*SLP_PREC_F_SCAL + 0.5);
    MINT32 SL2_SLP_3  = (MINT32)(((float)(tempGain3 - tempGain2)/(float)(tempR2 - tempR1))*SLP_PREC_F_SCAL + 0.5);
    MINT32 SL2_SLP_4  = (MINT32)(((float)(tempGain4 - tempGain3)/(float)(maxR -   tempR2))*SLP_PREC_F_SCAL + 0.5);

    DIP_X_REG_SL2_CEN rCen;
    DIP_X_REG_SL2_RR_CON0 rRRCond0;
    DIP_X_REG_SL2_RR_CON1 rRRCond1;
    DIP_X_REG_SL2_GAIN rGain;
    DIP_X_REG_SL2_SLP_CON0 rSlopCon0;
    DIP_X_REG_SL2_SLP_CON1 rSlopCon1;

    rCen.Raw=0;
    rRRCond0.Raw=0;
    rRRCond1.Raw=0;
    rGain.Raw=0;
    rSlopCon0.Raw=0;
    rSlopCon1.Raw=0;

    rCen.Bits.SL2_CENTR_X = SL2_CENTR_X;
    rCen.Bits.SL2_CENTR_Y = SL2_CENTR_Y;
    rRRCond0.Bits.SL2_R_0 = tempR0;
    rRRCond0.Bits.SL2_R_1 = tempR1;
    rRRCond1.Bits.SL2_R_2 = tempR2;
    rRRCond1.Bits.SL2_GAIN_0 = tempGain0;
    rRRCond1.Bits.SL2_GAIN_1 = tempGain1;
    rGain.Bits.SL2_GAIN_2 = tempGain2;
    rGain.Bits.SL2_GAIN_3 = tempGain3;
    rGain.Bits.SL2_GAIN_4 = tempGain4;
    rSlopCon0.Bits.SL2_SLP_1 = SL2_SLP_1;
    rSlopCon0.Bits.SL2_SLP_2 = SL2_SLP_2;
    rSlopCon1.Bits.SL2_SLP_3 = SL2_SLP_3;
    rSlopCon1.Bits.SL2_SLP_4 = SL2_SLP_4;

    REG_INFO_VALUE(DIP_X_SL2E_CEN)      = rCen.Raw;
    REG_INFO_VALUE(DIP_X_SL2E_RR_CON0)  = rRRCond0.Raw;
    REG_INFO_VALUE(DIP_X_SL2E_RR_CON1)  = rRRCond1.Raw;
    REG_INFO_VALUE(DIP_X_SL2E_GAIN)     = rGain.Raw;
    REG_INFO_VALUE(DIP_X_SL2E_SLP_CON0) = rSlopCon0.Raw;
    REG_INFO_VALUE(DIP_X_SL2E_SLP_CON1) = rSlopCon1.Raw;
    REG_INFO_VALUE(DIP_X_SL2E_RZ)       = (2048<<16) | 2048;
    REG_INFO_VALUE(DIP_X_SL2E_XOFF)     = 0;
    REG_INFO_VALUE(DIP_X_SL2E_YOFF)     = 0;

}


template <>
ISP_MGR_SL2_T&
ISP_MGR_SL2_T::
get(ISP_NVRAM_SL2_T& rParam)
{
    GET_REG_INFO(DIP_X_SL2_CEN, cen);
    GET_REG_INFO(DIP_X_SL2_RR_CON0, rr_con0);
    GET_REG_INFO(DIP_X_SL2_RR_CON1, rr_con1);
    GET_REG_INFO(DIP_X_SL2_GAIN, gain);

    return  (*this);
}

template <>
MBOOL
ISP_MGR_SL2_T::
get(ISP_NVRAM_SL2_T& rParam, const dip_x_reg_t* pReg) const
{
    if (pReg)
    {
        GET_REG_INFO_BUF(DIP_X_SL2_CEN, cen);
        GET_REG_INFO_BUF(DIP_X_SL2_RR_CON0, rr_con0);
        GET_REG_INFO_BUF(DIP_X_SL2_RR_CON1, rr_con1);
        GET_REG_INFO_BUF(DIP_X_SL2_GAIN, gain);
    }
    return MTRUE;
}

template <>
MBOOL
ISP_MGR_SL2_T::
get(ISP_NVRAM_SL2_GET_T& rParam, const dip_x_reg_t* pReg) const
{
    if (pReg)
    {
        GET_REG_INFO_BUF(DIP_X_SL2_CEN, cen);
        GET_REG_INFO_BUF(DIP_X_SL2_RR_CON0, rr_con0);
        GET_REG_INFO_BUF(DIP_X_SL2_RR_CON1, rr_con1);
        GET_REG_INFO_BUF(DIP_X_SL2_GAIN, gain);
        GET_REG_INFO_BUF(DIP_X_SL2_SLP_CON0, con0);
        GET_REG_INFO_BUF(DIP_X_SL2_SLP_CON1, con1);
        GET_REG_INFO_BUF(DIP_X_SL2_RZ, rz);
        GET_REG_INFO_BUF(DIP_X_SL2_XOFF, xoff);
        GET_REG_INFO_BUF(DIP_X_SL2_YOFF, yoff);
        GET_REG_INFO_BUF(DIP_X_SL2_SIZE, size);
    }
    return MTRUE;
}

template <>
MBOOL
ISP_MGR_SL2_T::
get(ISP_NVRAM_SL2B_GET_T& rParam, const dip_x_reg_t* pReg) const
{
    if (pReg)
    {
        GET_REG_INFO_BUF(DIP_X_SL2B_CEN, cen);
        GET_REG_INFO_BUF(DIP_X_SL2B_RR_CON0, rr_con0);
        GET_REG_INFO_BUF(DIP_X_SL2B_RR_CON1, rr_con1);
        GET_REG_INFO_BUF(DIP_X_SL2B_GAIN, gain);
        GET_REG_INFO_BUF(DIP_X_SL2B_SLP_CON0, con0);
        GET_REG_INFO_BUF(DIP_X_SL2B_SLP_CON1, con1);
        GET_REG_INFO_BUF(DIP_X_SL2B_RZ, rz);
        GET_REG_INFO_BUF(DIP_X_SL2B_XOFF, xoff);
        GET_REG_INFO_BUF(DIP_X_SL2B_YOFF, yoff);
        GET_REG_INFO_BUF(DIP_X_SL2B_SIZE, size);
    }
    return MTRUE;
}

template <>
MBOOL
ISP_MGR_SL2_T::
get(ISP_NVRAM_SL2C_GET_T& rParam, const dip_x_reg_t* pReg) const
{
    if (pReg)
    {
        GET_REG_INFO_BUF(DIP_X_SL2C_CEN, cen);
        GET_REG_INFO_BUF(DIP_X_SL2C_RR_CON0, rr_con0);
        GET_REG_INFO_BUF(DIP_X_SL2C_RR_CON1, rr_con1);
        GET_REG_INFO_BUF(DIP_X_SL2C_GAIN, gain);
        GET_REG_INFO_BUF(DIP_X_SL2C_SLP_CON0, con0);
        GET_REG_INFO_BUF(DIP_X_SL2C_SLP_CON1, con1);
        GET_REG_INFO_BUF(DIP_X_SL2C_RZ, rz);
        GET_REG_INFO_BUF(DIP_X_SL2C_XOFF, xoff);
        GET_REG_INFO_BUF(DIP_X_SL2C_YOFF, yoff);
        GET_REG_INFO_BUF(DIP_X_SL2C_SIZE, size);
    }
    return MTRUE;
}

template <>
MBOOL
ISP_MGR_SL2_T::
get(ISP_NVRAM_SL2D_GET_T& rParam, const dip_x_reg_t* pReg) const
{
    if (pReg)
    {
        GET_REG_INFO_BUF(DIP_X_SL2D_CEN, cen);
        GET_REG_INFO_BUF(DIP_X_SL2D_RR_CON0, rr_con0);
        GET_REG_INFO_BUF(DIP_X_SL2D_RR_CON1, rr_con1);
        GET_REG_INFO_BUF(DIP_X_SL2D_GAIN, gain);
        GET_REG_INFO_BUF(DIP_X_SL2D_SLP_CON0, con0);
        GET_REG_INFO_BUF(DIP_X_SL2D_SLP_CON1, con1);
        GET_REG_INFO_BUF(DIP_X_SL2D_RZ, rz);
        GET_REG_INFO_BUF(DIP_X_SL2D_XOFF, xoff);
        GET_REG_INFO_BUF(DIP_X_SL2D_YOFF, yoff);
        GET_REG_INFO_BUF(DIP_X_SL2D_SIZE, size);
    }
    return MTRUE;
}


template <>
MBOOL
ISP_MGR_SL2_T::
get(ISP_NVRAM_SL2E_GET_T& rParam, const dip_x_reg_t* pReg) const
{
    if (pReg)
    {
        GET_REG_INFO_BUF(DIP_X_SL2E_CEN, cen);
        GET_REG_INFO_BUF(DIP_X_SL2E_RR_CON0, rr_con0);
        GET_REG_INFO_BUF(DIP_X_SL2E_RR_CON1, rr_con1);
        GET_REG_INFO_BUF(DIP_X_SL2E_GAIN, gain);
        GET_REG_INFO_BUF(DIP_X_SL2E_SLP_CON0, con0);
        GET_REG_INFO_BUF(DIP_X_SL2E_SLP_CON1, con1);
        GET_REG_INFO_BUF(DIP_X_SL2E_RZ, rz);
        GET_REG_INFO_BUF(DIP_X_SL2E_XOFF, xoff);
        GET_REG_INFO_BUF(DIP_X_SL2E_YOFF, yoff);
        GET_REG_INFO_BUF(DIP_X_SL2E_SIZE, size);
    }
    return MTRUE;
}

template <>
MBOOL
ISP_MGR_SL2_T::
get(ISP_NVRAM_SL2H_GET_T& rParam, const dip_x_reg_t* pReg) const
{
    if (pReg)
    {
        GET_REG_INFO_BUF(DIP_X_SL2H_CEN, cen);
        GET_REG_INFO_BUF(DIP_X_SL2H_RR_CON0, rr_con0);
        GET_REG_INFO_BUF(DIP_X_SL2H_RR_CON1, rr_con1);
        GET_REG_INFO_BUF(DIP_X_SL2H_GAIN, gain);
        GET_REG_INFO_BUF(DIP_X_SL2H_SLP_CON0, con0);
        GET_REG_INFO_BUF(DIP_X_SL2H_SLP_CON1, con1);
        GET_REG_INFO_BUF(DIP_X_SL2H_RZ, rz);
        GET_REG_INFO_BUF(DIP_X_SL2H_XOFF, xoff);
        GET_REG_INFO_BUF(DIP_X_SL2H_YOFF, yoff);
        GET_REG_INFO_BUF(DIP_X_SL2H_SIZE, size);
    }
    return MTRUE;
}

template <>
MBOOL
ISP_MGR_SL2_T::
get(ISP_NVRAM_SL2I_GET_T& rParam, const dip_x_reg_t* pReg) const
{
    if (pReg)
    {
        GET_REG_INFO_BUF(DIP_X_SL2I_CEN, cen);
        GET_REG_INFO_BUF(DIP_X_SL2I_RR_CON0, rr_con0);
        GET_REG_INFO_BUF(DIP_X_SL2I_RR_CON1, rr_con1);
        GET_REG_INFO_BUF(DIP_X_SL2I_GAIN, gain);
        GET_REG_INFO_BUF(DIP_X_SL2I_SLP_CON0, con0);
        GET_REG_INFO_BUF(DIP_X_SL2I_SLP_CON1, con1);
        GET_REG_INFO_BUF(DIP_X_SL2I_RZ, rz);
        GET_REG_INFO_BUF(DIP_X_SL2I_XOFF, xoff);
        GET_REG_INFO_BUF(DIP_X_SL2I_YOFF, yoff);
        GET_REG_INFO_BUF(DIP_X_SL2I_SIZE, size);
    }
    return MTRUE;
}

#define GET_PROP(prop, init, val)\
{\
    char value[PROPERTY_VALUE_MAX] = {'\0'};\
    property_get(prop, value, (init));\
    (val) = atoi(value);\
}

MBOOL
ISP_MGR_SL2_T::
apply(const CROP_RZ_INFO_T& rCropRz, const CROP_RZ_INFO_T& rCropRz2, EIspProfile_T eIspProfile, dip_x_reg_t* pReg)
{
    MBOOL bSL2_EN = isEnable();

    MBOOL bSL2b_EN = getSL2BOnOff();
    MBOOL bSL2c_EN = getSL2COnOff();
    MBOOL bSL2d_EN = getSL2DOnOff();
    MBOOL bSL2e_EN = getSL2EOnOff();
    MBOOL bSL2h_EN = getSL2HOnOff();
    MBOOL bSL2i_EN = getSL2IOnOff();

    if( bSL2_EN | bSL2b_EN | bSL2c_EN | bSL2d_EN | bSL2e_EN | bSL2h_EN | bSL2i_EN ){
        transform_SL2(rCropRz, m_rSL2Param);

#if 0
        if(bSL2e_EN){
        ISP_NVRAM_SL2_T local_sl2;
        get(local_sl2);
            transform_SL2e(rCropRz2, local_sl2);
        }
#endif
    }

    // TOP
    ISP_WRITE_ENABLE_BITS(pReg, DIP_X_CTL_RGB_EN, SL2_EN,  bSL2_EN);
    ISP_WRITE_ENABLE_BITS(pReg, DIP_X_CTL_YUV_EN, SL2B_EN, bSL2b_EN);
    ISP_WRITE_ENABLE_BITS(pReg, DIP_X_CTL_YUV_EN, SL2C_EN, bSL2c_EN);
    ISP_WRITE_ENABLE_BITS(pReg, DIP_X_CTL_YUV_EN, SL2D_EN, bSL2d_EN);
    ISP_WRITE_ENABLE_BITS(pReg, DIP_X_CTL_YUV_EN, SL2E_EN, bSL2e_EN);
    ISP_WRITE_ENABLE_BITS(pReg, DIP_X_CTL_RGB_EN, SL2H_EN, bSL2h_EN);
    ISP_WRITE_ENABLE_BITS(pReg, DIP_X_CTL_YUV2_EN, SL2I_EN, bSL2i_EN);

    // Register setting
    writeRegs(static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum, pReg);

    CAM_LOGD_IF(m_bDebugEnable/*ENABLE_MY_LOG*/,
        "[%s] bSL2_EN(%d), bSL2b_EN(%d), bSL2c_EN(%d), DIP_X_SL2_CEN(0x%08x), DIP_X_SL2_RR_CON0(0x%08x), DIP_X_SL2_RR_CON1(0x%08x), DIP_X_SL2_GAIN(0x%08x)",
        __FUNCTION__, bSL2_EN, bSL2b_EN, bSL2c_EN,
        REG_INFO_VALUE(DIP_X_SL2_CEN),
        REG_INFO_VALUE(DIP_X_SL2_RR_CON0),
        REG_INFO_VALUE(DIP_X_SL2_RR_CON1),
        REG_INFO_VALUE(DIP_X_SL2_GAIN));

    dumpRegInfo("SL2");

    return  MTRUE;
}

}
