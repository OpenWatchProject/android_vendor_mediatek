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

#define LOG_TAG "isp_interpolation"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <stdlib.h>
#include <cutils/log.h>
#include <math.h>
#include <aaa_types.h>
#include <aaa_log.h>
#include <camera_custom_nvram.h>
#include <isp_tuning_cam_info.h>
#include "isp_interpolation.h"

//  "return value": Interpolated register value
//  "ISO":          ISO value of current image
//  "ISOpre":       A pre-defined ISO (100, 200, 400, ...) who is the cloeset but smaller than or equal to "ISO"
//  "ISOpos":       A pre-defined ISO (100, 200, 400, ...) who is the cloeset but larger than or equal to "ISO"
//  "PARAMpre":     Corresponding register value from "ISOpre"
//  "PARAMpos":     Corresponding register value from "ISOpos"
//  "Method:        0 for linear interpolation, 1 for closest one

MINT32 InterParam_DBS(MINT32 ISO, MINT32 ISOpre, MINT32 ISOpos, MINT32 PARAMpre, MINT32 PARAMpos, MINT32 Method) {
    MINT32 InterPARAM = 0;

    if(ISOpre == ISOpos) {
        //  To prevent someone sets different PARAMpre and PARAMpos
        return (MINT32)((PARAMpre + PARAMpos + 1) / 2);
    }

    switch(Method) {
    case 1:
        if(abs(ISO - ISOpre) > abs(ISOpos - ISO)) {
            InterPARAM = PARAMpos;
        }
        else{
            InterPARAM = PARAMpre;
        }
        break;
    case 0:
    default:
        double RATIOpre = (double)(ISOpos - ISO) / (double)(ISOpos - ISOpre);
        double RATIOpos = 1.0 - (double)(RATIOpre);
        InterPARAM = (MINT32)(RATIOpre * PARAMpre + RATIOpos * PARAMpos + 0.5);
        break;
    }

    return InterPARAM;
}

MVOID SmoothDBS(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_DBS_T const& rUpperDBS, // DBS settings for upper ISO
                ISP_NVRAM_DBS_T const& rLowerDBS,   // DBS settings for lower ISO
                ISP_NVRAM_DBS_T& rSmoothDBS)   // Output
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.smooth_dbs.enable", value, "0");
    MBOOL bSmoothDBSDebug = atoi(value);

    MY_LOG_IF(bSmoothDBSDebug,"[%s()] u4RealISO: %d, u4UpperISO = %d, u4LowerISO = %d\n", __FUNCTION__, u4RealISO, u4UpperISO, u4LowerISO);

    MINT32 pre_DBS_OFST       = rLowerDBS.sigma.bits.DBS_OFST;
    MINT32 pre_DBS_SL         = rLowerDBS.sigma.bits.DBS_SL;
    MINT32 pre_DBS_BIAS_Y0    = rLowerDBS.bstbl_0.bits.DBS_BIAS_Y0;
    MINT32 pre_DBS_BIAS_Y1    = rLowerDBS.bstbl_0.bits.DBS_BIAS_Y1;
    MINT32 pre_DBS_BIAS_Y2    = rLowerDBS.bstbl_0.bits.DBS_BIAS_Y2;
    MINT32 pre_DBS_BIAS_Y3    = rLowerDBS.bstbl_0.bits.DBS_BIAS_Y3;
    MINT32 pre_DBS_BIAS_Y4    = rLowerDBS.bstbl_1.bits.DBS_BIAS_Y4;
    MINT32 pre_DBS_BIAS_Y5    = rLowerDBS.bstbl_1.bits.DBS_BIAS_Y5;
    MINT32 pre_DBS_BIAS_Y6    = rLowerDBS.bstbl_1.bits.DBS_BIAS_Y6;
    MINT32 pre_DBS_BIAS_Y7    = rLowerDBS.bstbl_1.bits.DBS_BIAS_Y7;
    MINT32 pre_DBS_BIAS_Y8    = rLowerDBS.bstbl_2.bits.DBS_BIAS_Y8;
    MINT32 pre_DBS_BIAS_Y9    = rLowerDBS.bstbl_2.bits.DBS_BIAS_Y9;
    MINT32 pre_DBS_BIAS_Y10   = rLowerDBS.bstbl_2.bits.DBS_BIAS_Y10;
    MINT32 pre_DBS_BIAS_Y11   = rLowerDBS.bstbl_2.bits.DBS_BIAS_Y11;
    MINT32 pre_DBS_BIAS_Y12   = rLowerDBS.bstbl_3.bits.DBS_BIAS_Y12;
    MINT32 pre_DBS_BIAS_Y13   = rLowerDBS.bstbl_3.bits.DBS_BIAS_Y13;
    MINT32 pre_DBS_BIAS_Y14   = rLowerDBS.bstbl_3.bits.DBS_BIAS_Y14;
    MINT32 pre_DBS_HDR_GAIN   = rLowerDBS.ctl.bits.DBS_HDR_GAIN;
    MINT32 pre_DBS_HDR_GAIN2  = rLowerDBS.ctl.bits.DBS_HDR_GAIN2;
    MINT32 pre_DBS_HDR_GNP    = rLowerDBS.ctl.bits.DBS_HDR_GNP;
    MINT32 pre_DBS_LE_INV_CTL = rLowerDBS.ctl.bits.DBS_LE_INV_CTL;
    MINT32 pre_DBS_SL_EN      = rLowerDBS.ctl.bits.DBS_SL_EN;
    MINT32 pre_DBS_MUL_B      = rLowerDBS.sigma_2.bits.DBS_MUL_B;
    MINT32 pre_DBS_MUL_GB     = rLowerDBS.sigma_2.bits.DBS_MUL_GB;
    MINT32 pre_DBS_MUL_GR     = rLowerDBS.sigma_2.bits.DBS_MUL_GR;
    MINT32 pre_DBS_MUL_R      = rLowerDBS.sigma_2.bits.DBS_MUL_R;
    MINT32 pre_DBS_SL_Y1      = rLowerDBS.sl_y12.bits.DBS_SL_Y1;
    MINT32 pre_DBS_SL_Y2      = rLowerDBS.sl_y12.bits.DBS_SL_Y2;
    MINT32 pre_DBS_SL_Y3      = rLowerDBS.sl_y34.bits.DBS_SL_Y3;
    MINT32 pre_DBS_SL_Y4      = rLowerDBS.sl_y34.bits.DBS_SL_Y4;
    MINT32 pre_DBS_SL_G1      = rLowerDBS.sl_g12.bits.DBS_SL_G1;
    MINT32 pre_DBS_SL_G2      = rLowerDBS.sl_g12.bits.DBS_SL_G2;
    MINT32 pre_DBS_SL_G3      = rLowerDBS.sl_g34.bits.DBS_SL_G3;
    MINT32 pre_DBS_SL_G4      = rLowerDBS.sl_g34.bits.DBS_SL_G4;

    MINT32 pos_DBS_OFST       = rUpperDBS.sigma.bits.DBS_OFST;
    MINT32 pos_DBS_SL         = rUpperDBS.sigma.bits.DBS_SL;
    MINT32 pos_DBS_BIAS_Y0    = rUpperDBS.bstbl_0.bits.DBS_BIAS_Y0;
    MINT32 pos_DBS_BIAS_Y1    = rUpperDBS.bstbl_0.bits.DBS_BIAS_Y1;
    MINT32 pos_DBS_BIAS_Y2    = rUpperDBS.bstbl_0.bits.DBS_BIAS_Y2;
    MINT32 pos_DBS_BIAS_Y3    = rUpperDBS.bstbl_0.bits.DBS_BIAS_Y3;
    MINT32 pos_DBS_BIAS_Y4    = rUpperDBS.bstbl_1.bits.DBS_BIAS_Y4;
    MINT32 pos_DBS_BIAS_Y5    = rUpperDBS.bstbl_1.bits.DBS_BIAS_Y5;
    MINT32 pos_DBS_BIAS_Y6    = rUpperDBS.bstbl_1.bits.DBS_BIAS_Y6;
    MINT32 pos_DBS_BIAS_Y7    = rUpperDBS.bstbl_1.bits.DBS_BIAS_Y7;
    MINT32 pos_DBS_BIAS_Y8    = rUpperDBS.bstbl_2.bits.DBS_BIAS_Y8;
    MINT32 pos_DBS_BIAS_Y9    = rUpperDBS.bstbl_2.bits.DBS_BIAS_Y9;
    MINT32 pos_DBS_BIAS_Y10   = rUpperDBS.bstbl_2.bits.DBS_BIAS_Y10;
    MINT32 pos_DBS_BIAS_Y11   = rUpperDBS.bstbl_2.bits.DBS_BIAS_Y11;
    MINT32 pos_DBS_BIAS_Y12   = rUpperDBS.bstbl_3.bits.DBS_BIAS_Y12;
    MINT32 pos_DBS_BIAS_Y13   = rUpperDBS.bstbl_3.bits.DBS_BIAS_Y13;
    MINT32 pos_DBS_BIAS_Y14   = rUpperDBS.bstbl_3.bits.DBS_BIAS_Y14;
    MINT32 pos_DBS_HDR_GAIN   = rUpperDBS.ctl.bits.DBS_HDR_GAIN;
    MINT32 pos_DBS_HDR_GAIN2  = rUpperDBS.ctl.bits.DBS_HDR_GAIN2;
    MINT32 pos_DBS_HDR_GNP    = rUpperDBS.ctl.bits.DBS_HDR_GNP;
    MINT32 pos_DBS_LE_INV_CTL = rUpperDBS.ctl.bits.DBS_LE_INV_CTL;
    MINT32 pos_DBS_SL_EN      = rUpperDBS.ctl.bits.DBS_SL_EN;
    MINT32 pos_DBS_MUL_B      = rUpperDBS.sigma_2.bits.DBS_MUL_B;
    MINT32 pos_DBS_MUL_GB     = rUpperDBS.sigma_2.bits.DBS_MUL_GB;
    MINT32 pos_DBS_MUL_GR     = rUpperDBS.sigma_2.bits.DBS_MUL_GR;
    MINT32 pos_DBS_MUL_R      = rUpperDBS.sigma_2.bits.DBS_MUL_R;
    MINT32 pos_DBS_SL_Y1      = rUpperDBS.sl_y12.bits.DBS_SL_Y1;
    MINT32 pos_DBS_SL_Y2      = rUpperDBS.sl_y12.bits.DBS_SL_Y2;
    MINT32 pos_DBS_SL_Y3      = rUpperDBS.sl_y34.bits.DBS_SL_Y3;
    MINT32 pos_DBS_SL_Y4      = rUpperDBS.sl_y34.bits.DBS_SL_Y4;
    MINT32 pos_DBS_SL_G1      = rUpperDBS.sl_g12.bits.DBS_SL_G1;
    MINT32 pos_DBS_SL_G2      = rUpperDBS.sl_g12.bits.DBS_SL_G2;
    MINT32 pos_DBS_SL_G3      = rUpperDBS.sl_g34.bits.DBS_SL_G3;
    MINT32 pos_DBS_SL_G4      = rUpperDBS.sl_g34.bits.DBS_SL_G4;

    // interpolation //
    MINT32 ISO = u4RealISO;
    MINT32 ISOpre = u4LowerISO;
    MINT32 ISOpos = u4UpperISO;

    MINT32 DBS_OFST       = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_OFST, pos_DBS_OFST, 0);
    MINT32 DBS_SL         = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_SL, pos_DBS_SL, 0);
    MINT32 DBS_BIAS_Y0    = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_BIAS_Y0, pos_DBS_BIAS_Y0, 0);
    MINT32 DBS_BIAS_Y1    = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_BIAS_Y1, pos_DBS_BIAS_Y1, 0);
    MINT32 DBS_BIAS_Y2    = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_BIAS_Y2, pos_DBS_BIAS_Y2, 0);
    MINT32 DBS_BIAS_Y3    = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_BIAS_Y3, pos_DBS_BIAS_Y3, 0);
    MINT32 DBS_BIAS_Y4    = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_BIAS_Y4, pos_DBS_BIAS_Y4, 0);
    MINT32 DBS_BIAS_Y5    = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_BIAS_Y5, pos_DBS_BIAS_Y5, 0);
    MINT32 DBS_BIAS_Y6    = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_BIAS_Y6, pos_DBS_BIAS_Y6, 0);
    MINT32 DBS_BIAS_Y7    = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_BIAS_Y7, pos_DBS_BIAS_Y7, 0);
    MINT32 DBS_BIAS_Y8    = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_BIAS_Y8, pos_DBS_BIAS_Y8, 0);
    MINT32 DBS_BIAS_Y9    = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_BIAS_Y9, pos_DBS_BIAS_Y9, 0);
    MINT32 DBS_BIAS_Y10   = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_BIAS_Y10, pos_DBS_BIAS_Y10, 0);
    MINT32 DBS_BIAS_Y11   = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_BIAS_Y11, pos_DBS_BIAS_Y11, 0);
    MINT32 DBS_BIAS_Y12   = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_BIAS_Y12, pos_DBS_BIAS_Y12, 0);
    MINT32 DBS_BIAS_Y13   = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_BIAS_Y13, pos_DBS_BIAS_Y13, 0);
    MINT32 DBS_BIAS_Y14   = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_BIAS_Y14, pos_DBS_BIAS_Y14, 0);
    MINT32 DBS_HDR_GAIN   = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_HDR_GAIN, pos_DBS_HDR_GAIN, 0);
    MINT32 DBS_HDR_GAIN2  = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_HDR_GAIN2, pos_DBS_HDR_GAIN2, 0);
    MINT32 DBS_HDR_GNP    = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_HDR_GNP, pos_DBS_HDR_GNP, 0);
    MINT32 DBS_LE_INV_CTL = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_LE_INV_CTL, pos_DBS_LE_INV_CTL, 0);
    MINT32 DBS_SL_EN      = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_SL_EN, pos_DBS_SL_EN, 0);
    MINT32 DBS_MUL_B      = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_MUL_B, pos_DBS_MUL_B, 0);
    MINT32 DBS_MUL_GB     = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_MUL_GB, pos_DBS_MUL_GB, 0);
    MINT32 DBS_MUL_GR     = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_MUL_GR, pos_DBS_MUL_GR, 0);
    MINT32 DBS_MUL_R      = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_MUL_R, pos_DBS_MUL_R, 0);
    MINT32 DBS_SL_Y1      = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_SL_Y1, pos_DBS_SL_Y1, 0);
    MINT32 DBS_SL_Y2      = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_SL_Y2, pos_DBS_SL_Y2, 0);
    MINT32 DBS_SL_Y3      = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_SL_Y3, pos_DBS_SL_Y3, 0);
    MINT32 DBS_SL_Y4      = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_SL_Y4, pos_DBS_SL_Y4, 0);
    MINT32 DBS_SL_G1      = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_SL_G1, pos_DBS_SL_G1, 0);
    MINT32 DBS_SL_G2      = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_SL_G2, pos_DBS_SL_G2, 0);
    MINT32 DBS_SL_G3      = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_SL_G3, pos_DBS_SL_G3, 0);
    MINT32 DBS_SL_G4      = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_SL_G4, pos_DBS_SL_G4, 0);

    rSmoothDBS.sigma.bits.DBS_OFST       = DBS_OFST;
    rSmoothDBS.sigma.bits.DBS_SL         = DBS_SL;
    rSmoothDBS.bstbl_0.bits.DBS_BIAS_Y0  = DBS_BIAS_Y0;
    rSmoothDBS.bstbl_0.bits.DBS_BIAS_Y1  = DBS_BIAS_Y1;
    rSmoothDBS.bstbl_0.bits.DBS_BIAS_Y2  = DBS_BIAS_Y2;
    rSmoothDBS.bstbl_0.bits.DBS_BIAS_Y3  = DBS_BIAS_Y3;
    rSmoothDBS.bstbl_1.bits.DBS_BIAS_Y4  = DBS_BIAS_Y4;
    rSmoothDBS.bstbl_1.bits.DBS_BIAS_Y5  = DBS_BIAS_Y5;
    rSmoothDBS.bstbl_1.bits.DBS_BIAS_Y6  = DBS_BIAS_Y6;
    rSmoothDBS.bstbl_1.bits.DBS_BIAS_Y7  = DBS_BIAS_Y7;
    rSmoothDBS.bstbl_2.bits.DBS_BIAS_Y8  = DBS_BIAS_Y8;
    rSmoothDBS.bstbl_2.bits.DBS_BIAS_Y9  = DBS_BIAS_Y9;
    rSmoothDBS.bstbl_2.bits.DBS_BIAS_Y10 = DBS_BIAS_Y10;
    rSmoothDBS.bstbl_2.bits.DBS_BIAS_Y11 = DBS_BIAS_Y11;
    rSmoothDBS.bstbl_3.bits.DBS_BIAS_Y12 = DBS_BIAS_Y12;
    rSmoothDBS.bstbl_3.bits.DBS_BIAS_Y13 = DBS_BIAS_Y13;
    rSmoothDBS.bstbl_3.bits.DBS_BIAS_Y14 = DBS_BIAS_Y14;
    rSmoothDBS.ctl.bits.DBS_HDR_GAIN     = DBS_HDR_GAIN;
    rSmoothDBS.ctl.bits.DBS_HDR_GAIN2    = DBS_HDR_GAIN2;
    rSmoothDBS.ctl.bits.DBS_HDR_GNP      = DBS_HDR_GNP;
    rSmoothDBS.ctl.bits.DBS_LE_INV_CTL   = DBS_LE_INV_CTL;
    rSmoothDBS.ctl.bits.DBS_SL_EN        = DBS_SL_EN;
    rSmoothDBS.sigma_2.bits.DBS_MUL_B     = DBS_MUL_B; 
    rSmoothDBS.sigma_2.bits.DBS_MUL_GB    = DBS_MUL_GB;
    rSmoothDBS.sigma_2.bits.DBS_MUL_GR    = DBS_MUL_GR;
    rSmoothDBS.sigma_2.bits.DBS_MUL_R     = DBS_MUL_R;
    rSmoothDBS.sl_y12.bits.DBS_SL_Y1     = DBS_SL_Y1;
    rSmoothDBS.sl_y12.bits.DBS_SL_Y2     = DBS_SL_Y2;
    rSmoothDBS.sl_y34.bits.DBS_SL_Y3     = DBS_SL_Y3;
    rSmoothDBS.sl_y34.bits.DBS_SL_Y4     = DBS_SL_Y4;
    rSmoothDBS.sl_g12.bits.DBS_SL_G1     = DBS_SL_G1;
    rSmoothDBS.sl_g12.bits.DBS_SL_G2     = DBS_SL_G2;
    rSmoothDBS.sl_g34.bits.DBS_SL_G3     = DBS_SL_G3;
    rSmoothDBS.sl_g34.bits.DBS_SL_G4     = DBS_SL_G4;

    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_OFST       = %d, %d, %d", rLowerDBS.sigma.bits.DBS_OFST, rSmoothDBS.sigma.bits.DBS_OFST, rUpperDBS.sigma.bits.DBS_OFST);
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_SL         = %d, %d, %d", rLowerDBS.sigma.bits.DBS_SL, rSmoothDBS.sigma.bits.DBS_SL, rUpperDBS.sigma.bits.DBS_SL);
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_BIAS_Y0    = %d, %d, %d", rLowerDBS.bstbl_0.bits.DBS_BIAS_Y0, rSmoothDBS.bstbl_0.bits.DBS_BIAS_Y0, rUpperDBS.bstbl_0.bits.DBS_BIAS_Y0);
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_BIAS_Y1    = %d, %d, %d", rLowerDBS.bstbl_0.bits.DBS_BIAS_Y1, rSmoothDBS.bstbl_0.bits.DBS_BIAS_Y1, rUpperDBS.bstbl_0.bits.DBS_BIAS_Y1);
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_BIAS_Y2    = %d, %d, %d", rLowerDBS.bstbl_0.bits.DBS_BIAS_Y2, rSmoothDBS.bstbl_0.bits.DBS_BIAS_Y2, rUpperDBS.bstbl_0.bits.DBS_BIAS_Y2);
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_BIAS_Y3    = %d, %d, %d", rLowerDBS.bstbl_0.bits.DBS_BIAS_Y3, rSmoothDBS.bstbl_0.bits.DBS_BIAS_Y3, rUpperDBS.bstbl_0.bits.DBS_BIAS_Y3);
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_BIAS_Y4    = %d, %d, %d", rLowerDBS.bstbl_1.bits.DBS_BIAS_Y4, rSmoothDBS.bstbl_1.bits.DBS_BIAS_Y4, rUpperDBS.bstbl_1.bits.DBS_BIAS_Y4);
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_BIAS_Y5    = %d, %d, %d", rLowerDBS.bstbl_1.bits.DBS_BIAS_Y5, rSmoothDBS.bstbl_1.bits.DBS_BIAS_Y5, rUpperDBS.bstbl_1.bits.DBS_BIAS_Y5);
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_BIAS_Y6    = %d, %d, %d", rLowerDBS.bstbl_1.bits.DBS_BIAS_Y6, rSmoothDBS.bstbl_1.bits.DBS_BIAS_Y6, rUpperDBS.bstbl_1.bits.DBS_BIAS_Y6);
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_BIAS_Y7    = %d, %d, %d", rLowerDBS.bstbl_1.bits.DBS_BIAS_Y7, rSmoothDBS.bstbl_1.bits.DBS_BIAS_Y7, rUpperDBS.bstbl_1.bits.DBS_BIAS_Y7);
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_BIAS_Y8    = %d, %d, %d", rLowerDBS.bstbl_2.bits.DBS_BIAS_Y8, rSmoothDBS.bstbl_2.bits.DBS_BIAS_Y8, rUpperDBS.bstbl_2.bits.DBS_BIAS_Y8);
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_BIAS_Y9    = %d, %d, %d", rLowerDBS.bstbl_2.bits.DBS_BIAS_Y9, rSmoothDBS.bstbl_2.bits.DBS_BIAS_Y9, rUpperDBS.bstbl_2.bits.DBS_BIAS_Y9);
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_BIAS_Y10   = %d, %d, %d", rLowerDBS.bstbl_2.bits.DBS_BIAS_Y10, rSmoothDBS.bstbl_2.bits.DBS_BIAS_Y10, rUpperDBS.bstbl_2.bits.DBS_BIAS_Y10);
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_BIAS_Y11   = %d, %d, %d", rLowerDBS.bstbl_2.bits.DBS_BIAS_Y11, rSmoothDBS.bstbl_2.bits.DBS_BIAS_Y11, rUpperDBS.bstbl_2.bits.DBS_BIAS_Y11);
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_BIAS_Y12   = %d, %d, %d", rLowerDBS.bstbl_3.bits.DBS_BIAS_Y12, rSmoothDBS.bstbl_3.bits.DBS_BIAS_Y12, rUpperDBS.bstbl_3.bits.DBS_BIAS_Y12);
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_BIAS_Y13   = %d, %d, %d", rLowerDBS.bstbl_3.bits.DBS_BIAS_Y13, rSmoothDBS.bstbl_3.bits.DBS_BIAS_Y13, rUpperDBS.bstbl_3.bits.DBS_BIAS_Y13);
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_BIAS_Y14   = %d, %d, %d", rLowerDBS.bstbl_3.bits.DBS_BIAS_Y14, rSmoothDBS.bstbl_3.bits.DBS_BIAS_Y14, rUpperDBS.bstbl_3.bits.DBS_BIAS_Y14);
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_HDR_GAIN   = %d, %d, %d", rLowerDBS.ctl.bits.DBS_HDR_GAIN, rSmoothDBS.ctl.bits.DBS_HDR_GAIN, rUpperDBS.ctl.bits.DBS_HDR_GAIN);
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_HDR_GAIN2  = %d, %d, %d", rLowerDBS.ctl.bits.DBS_HDR_GAIN2, rSmoothDBS.ctl.bits.DBS_HDR_GAIN2, rUpperDBS.ctl.bits.DBS_HDR_GAIN2);
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_HDR_GNP    = %d, %d, %d", rLowerDBS.ctl.bits.DBS_HDR_GNP, rSmoothDBS.ctl.bits.DBS_HDR_GNP, rUpperDBS.ctl.bits.DBS_HDR_GNP);
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_LE_INV_CTL = %d, %d, %d", rLowerDBS.ctl.bits.DBS_LE_INV_CTL, rSmoothDBS.ctl.bits.DBS_LE_INV_CTL, rUpperDBS.ctl.bits.DBS_LE_INV_CTL);
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_SL_EN      = %d, %d, %d", rLowerDBS.ctl.bits.DBS_SL_EN, rSmoothDBS.ctl.bits.DBS_SL_EN, rUpperDBS.ctl.bits.DBS_SL_EN);
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_MUL_B      = %d, %d, %d", rLowerDBS.sigma_2.bits.DBS_MUL_B, rSmoothDBS.sigma_2.bits.DBS_MUL_B, rUpperDBS.sigma_2.bits.DBS_MUL_B);
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_MUL_GB     = %d, %d, %d", rLowerDBS.sigma_2.bits.DBS_MUL_GB, rSmoothDBS.sigma_2.bits.DBS_MUL_GB, rUpperDBS.sigma_2.bits.DBS_MUL_GB);
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_MUL_GR     = %d, %d, %d", rLowerDBS.sigma_2.bits.DBS_MUL_GR, rSmoothDBS.sigma_2.bits.DBS_MUL_GR, rUpperDBS.sigma_2.bits.DBS_MUL_GR);
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_MUL_R      = %d, %d, %d", rLowerDBS.sigma_2.bits.DBS_MUL_R, rSmoothDBS.sigma_2.bits.DBS_MUL_R, rUpperDBS.sigma_2.bits.DBS_MUL_R);
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_SL_Y1      = %d, %d, %d", rLowerDBS.sl_y12.bits.DBS_SL_Y1, rSmoothDBS.sl_y12.bits.DBS_SL_Y1, rUpperDBS.sl_y12.bits.DBS_SL_Y1);
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_SL_Y2      = %d, %d, %d", rLowerDBS.sl_y12.bits.DBS_SL_Y2, rSmoothDBS.sl_y12.bits.DBS_SL_Y2, rUpperDBS.sl_y12.bits.DBS_SL_Y2);
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_SL_Y3      = %d, %d, %d", rLowerDBS.sl_y34.bits.DBS_SL_Y3, rSmoothDBS.sl_y34.bits.DBS_SL_Y3, rUpperDBS.sl_y34.bits.DBS_SL_Y3);
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_SL_Y4      = %d, %d, %d", rLowerDBS.sl_y34.bits.DBS_SL_Y4, rSmoothDBS.sl_y34.bits.DBS_SL_Y4, rUpperDBS.sl_y34.bits.DBS_SL_Y4);
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_SL_G1      = %d, %d, %d", rLowerDBS.sl_g12.bits.DBS_SL_G1, rSmoothDBS.sl_g12.bits.DBS_SL_G1, rUpperDBS.sl_g12.bits.DBS_SL_G1);
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_SL_G2      = %d, %d, %d", rLowerDBS.sl_g12.bits.DBS_SL_G2, rSmoothDBS.sl_g12.bits.DBS_SL_G2, rUpperDBS.sl_g12.bits.DBS_SL_G2);
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_SL_G3      = %d, %d, %d", rLowerDBS.sl_g34.bits.DBS_SL_G3, rSmoothDBS.sl_g34.bits.DBS_SL_G3, rUpperDBS.sl_g34.bits.DBS_SL_G3);
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_SL_G4      = %d, %d, %d", rLowerDBS.sl_g34.bits.DBS_SL_G4, rSmoothDBS.sl_g34.bits.DBS_SL_G4, rUpperDBS.sl_g34.bits.DBS_SL_G4);
}

MINT32 InterParam_OBC(MINT32 ISO, MINT32 ISOpre, MINT32 ISOpos, MINT32 PARAMpre, MINT32 PARAMpos, MINT32 Method) {
	MINT32 InterPARAM = 0;

	if(ISOpre == ISOpos) {
		//	To prevent someone sets different PARAMpre and PARAMpos
		return (MINT32)((PARAMpre + PARAMpos + 1) / 2);
	}

	switch(Method) {
	case 1:
		if(abs(ISO - ISOpre) > abs(ISOpos - ISO)) {
			InterPARAM = PARAMpos;
		}
		else{
			InterPARAM = PARAMpre;
		}
		break;
	case 0:
	default:
		double RATIOpre = (double)(ISOpos - ISO) / (double)(ISOpos - ISOpre);
		double RATIOpos = 1.0 - (double)(RATIOpre);
		InterPARAM = (MINT32)(RATIOpre * PARAMpre + RATIOpos * PARAMpos + 0.5);
	    break;
	}

	return InterPARAM;
}

MVOID SmoothOBC(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_OBC_T const& rUpperOBC, // OBC settings for upper ISO
                ISP_NVRAM_OBC_T const& rLowerOBC,   // OBC settings for lower ISO
                ISP_NVRAM_OBC_T& rSmoothOBC)   // Output
{
	char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.smooth_obc.enable", value, "0");
    MBOOL bSmoothOBCDebug = atoi(value);

    MY_LOG_IF(bSmoothOBCDebug,"[%s()] u4RealISO: %d, u4UpperISO = %d, u4LowerISO = %d\n", __FUNCTION__, u4RealISO, u4UpperISO, u4LowerISO);

    MINT32 pre_OBC_OFST_B     = rLowerOBC.offst0.bits.OBC_OFST_B;
    MINT32 pre_OBC_OFST_GR    = rLowerOBC.offst1.bits.OBC_OFST_GR;
    MINT32 pre_OBC_OFST_GB    = rLowerOBC.offst2.bits.OBC_OFST_GB;
    MINT32 pre_OBC_OFST_R     = rLowerOBC.offst3.bits.OBC_OFST_R;
    MINT32 pre_OBC_GAIN_B     = rLowerOBC.gain0.bits.OBC_GAIN_B;
    MINT32 pre_OBC_GAIN_GR    = rLowerOBC.gain1.bits.OBC_GAIN_GR;
    MINT32 pre_OBC_GAIN_GB    = rLowerOBC.gain2.bits.OBC_GAIN_GB;
    MINT32 pre_OBC_GAIN_R     = rLowerOBC.gain3.bits.OBC_GAIN_R;

    MINT32 pos_OBC_OFST_B	  = rUpperOBC.offst0.bits.OBC_OFST_B;
    MINT32 pos_OBC_OFST_GR    = rUpperOBC.offst1.bits.OBC_OFST_GR;
    MINT32 pos_OBC_OFST_GB    = rUpperOBC.offst2.bits.OBC_OFST_GB;
    MINT32 pos_OBC_OFST_R     = rUpperOBC.offst3.bits.OBC_OFST_R;
    MINT32 pos_OBC_GAIN_B     = rUpperOBC.gain0.bits.OBC_GAIN_B;
    MINT32 pos_OBC_GAIN_GR    = rUpperOBC.gain1.bits.OBC_GAIN_GR;
    MINT32 pos_OBC_GAIN_GB    = rUpperOBC.gain2.bits.OBC_GAIN_GB;
    MINT32 pos_OBC_GAIN_R     = rUpperOBC.gain3.bits.OBC_GAIN_R;

    // interpolation //
    MINT32 ISO = u4RealISO;
    MINT32 ISOpre = u4LowerISO;
    MINT32 ISOpos = u4UpperISO;

    MINT32 OBC_OFST_B  = InterParam_DBS(ISO, ISOpre, ISOpos, pre_OBC_OFST_B, pos_OBC_OFST_B, 0);
    MINT32 OBC_OFST_GR = InterParam_DBS(ISO, ISOpre, ISOpos, pre_OBC_OFST_GR, pos_OBC_OFST_GR, 0);
    MINT32 OBC_OFST_GB = InterParam_DBS(ISO, ISOpre, ISOpos, pre_OBC_OFST_GB, pos_OBC_OFST_GB, 0);
    MINT32 OBC_OFST_R  = InterParam_DBS(ISO, ISOpre, ISOpos, pre_OBC_OFST_R, pos_OBC_OFST_R, 0);
    MINT32 OBC_GAIN_B  = InterParam_DBS(ISO, ISOpre, ISOpos, pre_OBC_GAIN_B, pos_OBC_GAIN_B, 0);
    MINT32 OBC_GAIN_GR = InterParam_DBS(ISO, ISOpre, ISOpos, pre_OBC_GAIN_GR, pos_OBC_GAIN_GR, 0);
    MINT32 OBC_GAIN_GB = InterParam_DBS(ISO, ISOpre, ISOpos, pre_OBC_GAIN_GB, pos_OBC_GAIN_GB, 0);
    MINT32 OBC_GAIN_R  = InterParam_DBS(ISO, ISOpre, ISOpos, pre_OBC_GAIN_R, pos_OBC_GAIN_R, 0);

    rSmoothOBC.offst0.bits.OBC_OFST_B  = OBC_OFST_B;
    rSmoothOBC.offst1.bits.OBC_OFST_GR = OBC_OFST_GR;
    rSmoothOBC.offst2.bits.OBC_OFST_GB = OBC_OFST_GB;
    rSmoothOBC.offst3.bits.OBC_OFST_R  = OBC_OFST_R;
    rSmoothOBC.gain0.bits.OBC_GAIN_B   = OBC_GAIN_B;
    rSmoothOBC.gain1.bits.OBC_GAIN_GR  = OBC_GAIN_GR;
    rSmoothOBC.gain2.bits.OBC_GAIN_GB  = OBC_GAIN_GB;
    rSmoothOBC.gain3.bits.OBC_GAIN_R   = OBC_GAIN_R;

    MY_LOG_IF(bSmoothOBCDebug, "[L, S, U]OBC_OFST_B  = %d, %d, %d", rLowerOBC.offst0.bits.OBC_OFST_B, rLowerOBC.offst0.bits.OBC_OFST_B, rUpperOBC.offst0.bits.OBC_OFST_B);
    MY_LOG_IF(bSmoothOBCDebug, "[L, S, U]OBC_OFST_GR = %d, %d, %d", rLowerOBC.offst1.bits.OBC_OFST_GR, rLowerOBC.offst1.bits.OBC_OFST_GR, rUpperOBC.offst1.bits.OBC_OFST_GR);
    MY_LOG_IF(bSmoothOBCDebug, "[L, S, U]OBC_OFST_GB = %d, %d, %d", rLowerOBC.offst2.bits.OBC_OFST_GB, rLowerOBC.offst2.bits.OBC_OFST_GB, rUpperOBC.offst2.bits.OBC_OFST_GB);
    MY_LOG_IF(bSmoothOBCDebug, "[L, S, U]OBC_OFST_R  = %d, %d, %d", rLowerOBC.offst3.bits.OBC_OFST_R, rLowerOBC.offst3.bits.OBC_OFST_R, rUpperOBC.offst3.bits.OBC_OFST_R);
    MY_LOG_IF(bSmoothOBCDebug, "[L, S, U]OBC_GAIN_B  = %d, %d, %d", rLowerOBC.gain0.bits.OBC_GAIN_B, rLowerOBC.gain0.bits.OBC_GAIN_B, rUpperOBC.gain0.bits.OBC_GAIN_B);
    MY_LOG_IF(bSmoothOBCDebug, "[L, S, U]OBC_GAIN_GR = %d, %d, %d", rLowerOBC.gain1.bits.OBC_GAIN_GR, rLowerOBC.gain1.bits.OBC_GAIN_GR, rUpperOBC.gain1.bits.OBC_GAIN_GR);
    MY_LOG_IF(bSmoothOBCDebug, "[L, S, U]OBC_GAIN_GB = %d, %d, %d", rLowerOBC.gain2.bits.OBC_GAIN_GB, rLowerOBC.gain2.bits.OBC_GAIN_GB, rUpperOBC.gain2.bits.OBC_GAIN_GB);
    MY_LOG_IF(bSmoothOBCDebug, "[L, S, U]OBC_GAIN_R  = %d, %d, %d", rLowerOBC.gain3.bits.OBC_GAIN_R, rLowerOBC.gain3.bits.OBC_GAIN_R, rUpperOBC.gain3.bits.OBC_GAIN_R);
}

MINT32 InterParam_BPC(MINT32 ISO, MINT32 ISOpre, MINT32 ISOpos, MINT32 PARAMpre, MINT32 PARAMpos, MINT32 Method) {
	MINT32 InterPARAM = 0;

	if(ISOpre == ISOpos) {
		//	To prevent someone sets different PARAMpre and PARAMpos
		return (MINT32)((PARAMpre + PARAMpos + 1) / 2);
	}

	switch(Method) {
	case 1:
		if(abs(ISO - ISOpre) > abs(ISOpos - ISO)) {
			InterPARAM = PARAMpos;
		}
		else{
			InterPARAM = PARAMpre;
		}
		break;
	case 0:
	default:
		double RATIOpre = (double)(ISOpos - ISO) / (double)(ISOpos - ISOpre);
		double RATIOpos = 1.0 - (double)(RATIOpre);
		InterPARAM = (MINT32)(RATIOpre * PARAMpre + RATIOpos * PARAMpos + 0.5);
	    break;
	}

	return InterPARAM;
}

MVOID SmoothBPC(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_BNR_BPC_T const& rUpperBPC, // NR1 settings for upper ISO
                ISP_NVRAM_BNR_BPC_T const& rLowerBPC,   // NR1 settings for lower ISO
                ISP_NVRAM_BNR_BPC_T& rSmoothBPC)   // Output
{
	char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.smooth_bpc.enable", value, "0");
    MBOOL bSmoothBPCDebug = atoi(value);
    MY_LOG_IF(bSmoothBPCDebug,"[%s()] u4RealISO: %d, u4UpperISO = %d, u4LowerISO = %d\n", __FUNCTION__, u4RealISO, u4UpperISO, u4LowerISO);

    MINT32 BPC_ENpre           =   rLowerBPC.con.bits.BPC_EN;
    MINT32 BPC_LUT_ENpre       =   rLowerBPC.con.bits.BPC_LUT_EN;
    MINT32 BPC_DTC_MODEpre     =   rLowerBPC.con.bits.BPC_DTC_MODE;
    MINT32 BPC_CRC_MODEpre     =   rLowerBPC.con.bits.BPC_CRC_MODE;
    MINT32 BPC_RNGpre          =   rLowerBPC.dtc.bits.BPC_RNG;
    MINT32 BPC_DIR_THpre       =   rLowerBPC.cor.bits.BPC_DIR_TH;
    MINT32 BPC_DIR_TH2pre      =   rLowerBPC.cor.bits.BPC_DIR_TH2;
    MINT32 BPC_DIR_MAXpre      =   rLowerBPC.cor.bits.BPC_DIR_MAX;
    MINT32 BPC_TH_LWBpre       =   rLowerBPC.th1.bits.BPC_TH_LWB;
    MINT32 BPC_TH_SLApre       =   rLowerBPC.th3.bits.BPC_TH_SLA;
    MINT32 BPC_DK_TH_SLApre    =   rLowerBPC.th4.bits.BPC_DK_TH_SLA;
    MINT32 BPC_TH_SLBpre       =   rLowerBPC.th3.bits.BPC_TH_SLB;
    MINT32 BPC_DK_TH_SLBpre    =   rLowerBPC.th4.bits.BPC_DK_TH_SLB;
    MINT32 BPC_TH_XApre        =   rLowerBPC.th3.bits.BPC_TH_XA;
    MINT32 BPC_DK_TH_XApre     =   rLowerBPC.th4.bits.BPC_DK_TH_XA;
    MINT32 BPC_TH_XBpre        =   rLowerBPC.th3.bits.BPC_TH_XB;
    MINT32 BPC_DK_TH_XBpre     =   rLowerBPC.th4.bits.BPC_DK_TH_XB;
    MINT32 BPC_TH_Ypre         =   rLowerBPC.th1.bits.BPC_TH_Y;
    MINT32 BPC_TH_UPBpre       =   rLowerBPC.th2.bits. BPC_TH_UPB;
    MINT32 BPC_C_TH_LWBpre     =   rLowerBPC.th1_c.bits.BPC_C_TH_LWB;
    MINT32 BPC_C_TH_SLApre     =   rLowerBPC.th3_c.bits.BPC_C_TH_SLA;
    MINT32 BPC_C_TH_SLBpre     =   rLowerBPC.th3_c.bits.BPC_C_TH_SLB;
    MINT32 BPC_C_TH_XApre      =   rLowerBPC.th3_c.bits.BPC_C_TH_XA;
    MINT32 BPC_C_TH_XBpre      =   rLowerBPC.th3_c.bits.BPC_C_TH_XB;
    MINT32 BPC_C_TH_Ypre       =   rLowerBPC.th1_c.bits.BPC_C_TH_Y;
    MINT32 BPC_C_TH_UPBpre     =   rLowerBPC.th2_c.bits.BPC_C_TH_UPB;
    MINT32 BPC_NO_LVpre        =   rLowerBPC.dtc.bits.BPC_NO_LV;
    MINT32 BPC_CT_LVpre        =   rLowerBPC.dtc.bits.BPC_CT_LV;
    MINT32 BPC_TH_MULpre       =   rLowerBPC.dtc.bits.BPC_TH_MUL;
    MINT32 BPC_CS_MODEpre      =   rLowerBPC.con.bits.BPC_CS_MODE;
    MINT32 BPC_CS_RNGpre       =   rLowerBPC.dtc.bits.BPC_CS_RNG;
	MINT32 BPC_TABLE_END_MODEpre = rLowerBPC.con.bits.BPC_TABLE_END_MODE;
	MINT32 BPC_AVG_MODEpre       = rLowerBPC.con.bits.BPC_AVG_MODE;
	MINT32 BNR_LE_INV_CTLpre     = rLowerBPC.con.bits.BNR_LE_INV_CTL;
	MINT32 BNR_OSC_COUNTpre      = rLowerBPC.con.bits.BNR_OSC_COUNT;
	MINT32 BNR_EDGEpre           = rLowerBPC.con.bits.BNR_EDGE;
	MINT32 BPC_XOFFSETpre        = rLowerBPC.tbli1.bits.BPC_XOFFSET;
	MINT32 BPC_YOFFSETpre        = rLowerBPC.tbli1.bits.BPC_YOFFSET;
	MINT32 BPC_XSIZEpre          = rLowerBPC.tbli2.bits.BPC_XSIZE;
	MINT32 BPC_YSIZEpre          = rLowerBPC.tbli2.bits.BPC_YSIZE;
	MINT32 BPC_EXCpre              = rLowerBPC.con.bits.BPC_EXC;
	MINT32 BPC_RANK_IDXRpre        = rLowerBPC.cor.bits.BPC_RANK_IDXR;
	MINT32 BPC_RANK_IDXGpre        = rLowerBPC.cor.bits.BPC_RANK_IDXG;
	MINT32 BPC_RANK_IDXBpre        = rLowerBPC.th2_c.bits.BPC_RANK_IDXB;
	MINT32 BPC_BLD_MODEpre         = rLowerBPC.con.bits.BPC_BLD_MODE;
	MINT32 BPC_BLD_LWBpre          = rLowerBPC.th2_c.bits.BPC_BLD_LWB;
	MINT32 BPC_BLD0pre             = rLowerBPC.th2.bits.BPC_BLD0;
	MINT32 BPC_BLD1pre             = rLowerBPC.th2.bits.BPC_BLD1;
	MINT32 BPC_BLD_SLP0pre         = rLowerBPC.th1.bits.BPC_BLD_SLP0;

	MINT32 BPC_ENpos           =   rUpperBPC.con.bits.BPC_EN;
    MINT32 BPC_LUT_ENpos       =   rUpperBPC.con.bits.BPC_LUT_EN;
    MINT32 BPC_DTC_MODEpos     =   rUpperBPC.con.bits.BPC_DTC_MODE;
    MINT32 BPC_CRC_MODEpos     =   rUpperBPC.con.bits.BPC_CRC_MODE;
    MINT32 BPC_RNGpos          =   rUpperBPC.dtc.bits.BPC_RNG;
    MINT32 BPC_DIR_THpos       =   rUpperBPC.cor.bits.BPC_DIR_TH;
    MINT32 BPC_DIR_TH2pos      =   rUpperBPC.cor.bits.BPC_DIR_TH2;
    MINT32 BPC_DIR_MAXpos      =   rUpperBPC.cor.bits.BPC_DIR_MAX;
    MINT32 BPC_TH_LWBpos       =   rUpperBPC.th1.bits.BPC_TH_LWB;
    MINT32 BPC_TH_SLApos       =   rUpperBPC.th3.bits.BPC_TH_SLA;
    MINT32 BPC_DK_TH_SLApos    =   rUpperBPC.th4.bits.BPC_DK_TH_SLA;
    MINT32 BPC_TH_SLBpos       =   rUpperBPC.th3.bits.BPC_TH_SLB;
    MINT32 BPC_DK_TH_SLBpos    =   rUpperBPC.th4.bits.BPC_DK_TH_SLB;
    MINT32 BPC_TH_XApos        =   rUpperBPC.th3.bits.BPC_TH_XA;
    MINT32 BPC_DK_TH_XApos     =   rUpperBPC.th4.bits.BPC_DK_TH_XA;
    MINT32 BPC_TH_XBpos        =   rUpperBPC.th3.bits.BPC_TH_XB;
    MINT32 BPC_DK_TH_XBpos     =   rUpperBPC.th4.bits.BPC_DK_TH_XB;
    MINT32 BPC_TH_Ypos         =   rUpperBPC.th1.bits.BPC_TH_Y;
    MINT32 BPC_TH_UPBpos       =   rUpperBPC.th2.bits. BPC_TH_UPB;
    MINT32 BPC_C_TH_LWBpos     =   rUpperBPC.th1_c.bits.BPC_C_TH_LWB;
    MINT32 BPC_C_TH_SLApos     =   rUpperBPC.th3_c.bits.BPC_C_TH_SLA;
    MINT32 BPC_C_TH_SLBpos     =   rUpperBPC.th3_c.bits.BPC_C_TH_SLB;
    MINT32 BPC_C_TH_XApos      =   rUpperBPC.th3_c.bits.BPC_C_TH_XA;
    MINT32 BPC_C_TH_XBpos      =   rUpperBPC.th3_c.bits.BPC_C_TH_XB;
    MINT32 BPC_C_TH_Ypos       =   rUpperBPC.th1_c.bits.BPC_C_TH_Y;
    MINT32 BPC_C_TH_UPBpos     =   rUpperBPC.th2_c.bits.BPC_C_TH_UPB;
    MINT32 BPC_NO_LVpos        =   rUpperBPC.dtc.bits.BPC_NO_LV;
    MINT32 BPC_CT_LVpos        =   rUpperBPC.dtc.bits.BPC_CT_LV;
    MINT32 BPC_TH_MULpos       =   rUpperBPC.dtc.bits.BPC_TH_MUL;
    MINT32 BPC_CS_MODEpos      =   rUpperBPC.con.bits.BPC_CS_MODE;
    MINT32 BPC_CS_RNGpos       =   rUpperBPC.dtc.bits.BPC_CS_RNG;
	MINT32 BPC_TABLE_END_MODEpos = rUpperBPC.con.bits.BPC_TABLE_END_MODE;
	MINT32 BPC_AVG_MODEpos       = rUpperBPC.con.bits.BPC_AVG_MODE;
	MINT32 BNR_LE_INV_CTLpos     = rUpperBPC.con.bits.BNR_LE_INV_CTL;
	MINT32 BNR_OSC_COUNTpos      = rUpperBPC.con.bits.BNR_OSC_COUNT;
	MINT32 BNR_EDGEpos           = rUpperBPC.con.bits.BNR_EDGE;
	MINT32 BPC_XOFFSETpos        = rUpperBPC.tbli1.bits.BPC_XOFFSET;
	MINT32 BPC_YOFFSETpos        = rUpperBPC.tbli1.bits.BPC_YOFFSET;
	MINT32 BPC_XSIZEpos          = rUpperBPC.tbli2.bits.BPC_XSIZE;
	MINT32 BPC_YSIZEpos          = rUpperBPC.tbli2.bits.BPC_YSIZE;
	MINT32 BPC_EXCpos              = rUpperBPC.con.bits.BPC_EXC;
	MINT32 BPC_RANK_IDXRpos        = rUpperBPC.cor.bits.BPC_RANK_IDXR;
	MINT32 BPC_RANK_IDXGpos        = rUpperBPC.cor.bits.BPC_RANK_IDXG;
	MINT32 BPC_RANK_IDXBpos        = rUpperBPC.th2_c.bits.BPC_RANK_IDXB;
	MINT32 BPC_BLD_MODEpos         = rUpperBPC.con.bits.BPC_BLD_MODE;
	MINT32 BPC_BLD_LWBpos          = rUpperBPC.th2_c.bits.BPC_BLD_LWB;
	MINT32 BPC_BLD0pos             = rUpperBPC.th2.bits.BPC_BLD0;
	MINT32 BPC_BLD1pos             = rUpperBPC.th2.bits.BPC_BLD1;
	MINT32 BPC_BLD_SLP0pos         = rUpperBPC.th1.bits.BPC_BLD_SLP0;

    // interpolation //
    MINT32 ISO = u4RealISO;
    MINT32 ISOpre = u4LowerISO;
    MINT32 ISOpos = u4UpperISO;

    MINT32 BPC_EN              = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_ENpre, BPC_ENpos, 0);
    MINT32 BPC_LUT_EN          = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_LUT_ENpre, BPC_LUT_ENpos, 0);
    MINT32 BPC_DTC_MODE        = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_DTC_MODEpre, BPC_DTC_MODEpos, 0);
    MINT32 BPC_CRC_MODE        = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_CRC_MODEpre, BPC_CRC_MODEpos, 0);
    MINT32 BPC_RNG             = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_RNGpre, BPC_RNGpos, 0);
    MINT32 BPC_DIR_TH          = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_DIR_THpre, BPC_DIR_THpos, 0);
    MINT32 BPC_DIR_TH2         = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_DIR_TH2pre, BPC_DIR_TH2pos, 0);
    MINT32 BPC_DIR_MAX         = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_DIR_MAXpre, BPC_DIR_MAXpos, 0);
    MINT32 BPC_TH_LWB          = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_TH_LWBpre, BPC_TH_LWBpos, 0);
    MINT32 BPC_TH_SLA          = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_TH_SLApre, BPC_TH_SLApos, 0);
    MINT32 BPC_DK_TH_SLA       = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_DK_TH_SLApre, BPC_DK_TH_SLApos, 0);
    MINT32 BPC_TH_SLB          = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_TH_SLBpre, BPC_TH_SLBpos, 0);
    MINT32 BPC_DK_TH_SLB       = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_DK_TH_SLBpre, BPC_DK_TH_SLBpos, 0);
    MINT32 BPC_TH_XA           = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_TH_XApre, BPC_TH_XApos, 0);
    MINT32 BPC_DK_TH_XA        = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_DK_TH_XApre, BPC_DK_TH_XApos, 0);
    MINT32 BPC_TH_XB           = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_TH_XBpre, BPC_TH_XBpos, 0);
    MINT32 BPC_DK_TH_XB        = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_DK_TH_XBpre, BPC_DK_TH_XBpos, 0);
    MINT32 BPC_TH_Y            = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_TH_Ypre, BPC_TH_Ypos, 0);
    MINT32 BPC_TH_UPB          = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_TH_UPBpre, BPC_TH_UPBpos, 0);
    MINT32 BPC_C_TH_LWB        = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_C_TH_LWBpre, BPC_C_TH_LWBpos, 0);
    MINT32 BPC_C_TH_SLA        = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_C_TH_SLApre, BPC_C_TH_SLApos, 0);
    MINT32 BPC_C_TH_SLB        = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_C_TH_SLBpre, BPC_C_TH_SLBpos, 0);
    MINT32 BPC_C_TH_XA         = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_C_TH_XApre, BPC_C_TH_XApos, 0);
    MINT32 BPC_C_TH_XB         = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_C_TH_XBpre, BPC_C_TH_XBpos, 0);
    MINT32 BPC_C_TH_Y          = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_C_TH_Ypre, BPC_C_TH_Ypos, 0);
    MINT32 BPC_C_TH_UPB        = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_C_TH_UPBpre, BPC_C_TH_UPBpos, 0);
    MINT32 BPC_NO_LV           = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_NO_LVpre, BPC_NO_LVpos, 0);
    MINT32 BPC_CT_LV           = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_CT_LVpre, BPC_CT_LVpos, 0);
    MINT32 BPC_TH_MUL          = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_TH_MULpre, BPC_TH_MULpos, 0);
    MINT32 BPC_CS_MODE         = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_CS_MODEpre, BPC_CS_MODEpos, 0);
    MINT32 BPC_CS_RNG          = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_CS_RNGpre, BPC_CS_RNGpos, 0);
    MINT32 BPC_TABLE_END_MODE  = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_TABLE_END_MODEpre, BPC_TABLE_END_MODEpos, 1);
    MINT32 BPC_AVG_MODE        = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_AVG_MODEpre, BPC_AVG_MODEpos, 1);
    MINT32 BNR_LE_INV_CTL      = InterParam_BPC(ISO, ISOpre, ISOpos, BNR_LE_INV_CTLpre, BNR_LE_INV_CTLpos, 1);
    MINT32 BNR_OSC_COUNT       = InterParam_BPC(ISO, ISOpre, ISOpos, BNR_OSC_COUNTpre, BNR_OSC_COUNTpos, 0);
    MINT32 BNR_EDGE            = InterParam_BPC(ISO, ISOpre, ISOpos, BNR_EDGEpre, BNR_EDGEpos, 1);
    MINT32 BPC_XOFFSET         = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_XOFFSETpre, BPC_XOFFSETpos, 0);
    MINT32 BPC_YOFFSET         = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_YOFFSETpre, BPC_YOFFSETpos, 0);
    MINT32 BPC_XSIZE           = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_XSIZEpre, BPC_XSIZEpos, 0);
    MINT32 BPC_YSIZE           = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_YSIZEpre, BPC_YSIZEpos, 0);
    MINT32 BPC_EXC             = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_EXCpre, BPC_EXCpos  , 0);
    MINT32 BPC_RANK_IDXR       = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_RANK_IDXRpre, BPC_RANK_IDXRpos, 0);
    MINT32 BPC_RANK_IDXG       = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_RANK_IDXGpre, BPC_RANK_IDXGpos, 0);
    MINT32 BPC_RANK_IDXB       = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_RANK_IDXBpre, BPC_RANK_IDXBpos, 0);
    MINT32 BPC_BLD_MODE        = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_BLD_MODEpre, BPC_BLD_MODEpos, 1);
    MINT32 BPC_BLD_LWB         = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_BLD_LWBpre, BPC_BLD_LWBpos, 0);
    MINT32 BPC_BLD0            = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_BLD0pre, BPC_BLD0pos, 0);
    MINT32 BPC_BLD1            = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_BLD1pre, BPC_BLD1pos, 0);
    MINT32 BPC_BLD_SLP0        = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_BLD_SLP0pre, BPC_BLD_SLP0pos, 0);

    rSmoothBPC.con.bits.BPC_EN              = BPC_EN;
    rSmoothBPC.con.bits.BPC_LUT_EN          = BPC_LUT_EN;
    rSmoothBPC.con.bits.BPC_DTC_MODE        = BPC_DTC_MODE;
    rSmoothBPC.con.bits.BPC_CRC_MODE        = BPC_CRC_MODE;
    rSmoothBPC.dtc.bits.BPC_RNG             = BPC_RNG;
    rSmoothBPC.cor.bits.BPC_DIR_TH          = BPC_DIR_TH;
    rSmoothBPC.cor.bits.BPC_DIR_TH2         = BPC_DIR_TH2;
    rSmoothBPC.cor.bits.BPC_DIR_MAX         = BPC_DIR_MAX;
    rSmoothBPC.th1.bits.BPC_TH_LWB          = BPC_TH_LWB;
    rSmoothBPC.th3.bits.BPC_TH_SLA          = BPC_TH_SLA;
    rSmoothBPC.th4.bits.BPC_DK_TH_SLA       = BPC_DK_TH_SLA;
    rSmoothBPC.th3.bits.BPC_TH_SLB          = BPC_TH_SLB;
    rSmoothBPC.th4.bits.BPC_DK_TH_SLB       = BPC_DK_TH_SLB;
    rSmoothBPC.th3.bits.BPC_TH_XA           = BPC_TH_XA;
    rSmoothBPC.th4.bits.BPC_DK_TH_XA        = BPC_DK_TH_XA;
    rSmoothBPC.th3.bits.BPC_TH_XB           = BPC_TH_XB;
    rSmoothBPC.th4.bits.BPC_DK_TH_XB        = BPC_DK_TH_XB;
    rSmoothBPC.th1.bits.BPC_TH_Y            = BPC_TH_Y;
    rSmoothBPC.th2.bits.BPC_TH_UPB          = BPC_TH_UPB;
    rSmoothBPC.th1_c.bits.BPC_C_TH_LWB      = BPC_C_TH_LWB;
    rSmoothBPC.th3_c.bits.BPC_C_TH_SLA      = BPC_C_TH_SLA;
    rSmoothBPC.th3_c.bits.BPC_C_TH_SLB      = BPC_C_TH_SLB;
    rSmoothBPC.th3_c.bits.BPC_C_TH_XA       = BPC_C_TH_XA;
    rSmoothBPC.th3_c.bits.BPC_C_TH_XB       = BPC_C_TH_XB;
    rSmoothBPC.th1_c.bits.BPC_C_TH_Y        = BPC_C_TH_Y;
    rSmoothBPC.th2_c.bits.BPC_C_TH_UPB      = BPC_C_TH_UPB;
    rSmoothBPC.dtc.bits.BPC_NO_LV           = BPC_NO_LV;
    rSmoothBPC.dtc.bits.BPC_CT_LV           = BPC_CT_LV;
    rSmoothBPC.dtc.bits.BPC_TH_MUL          = BPC_TH_MUL;
    rSmoothBPC.con.bits.BPC_CS_MODE         = BPC_CS_MODE;
    rSmoothBPC.dtc.bits.BPC_CS_RNG          = BPC_CS_RNG;
	rSmoothBPC.con.bits.BPC_TABLE_END_MODE  = BPC_TABLE_END_MODE;
	rSmoothBPC.con.bits.BPC_AVG_MODE        = BPC_AVG_MODE;
	rSmoothBPC.con.bits.BNR_LE_INV_CTL      = BNR_LE_INV_CTL;
	rSmoothBPC.con.bits.BNR_OSC_COUNT       = BNR_OSC_COUNT;
	rSmoothBPC.con.bits.BNR_EDGE            = BNR_EDGE;
	rSmoothBPC.tbli1.bits.BPC_XOFFSET       = BPC_XOFFSET;
	rSmoothBPC.tbli1.bits.BPC_YOFFSET       = BPC_YOFFSET;
	rSmoothBPC.tbli2.bits.BPC_XSIZE         = BPC_XSIZE;
	rSmoothBPC.tbli2.bits.BPC_YSIZE         = BPC_YSIZE;
    rSmoothBPC.con.bits.BPC_EXC	            = BPC_EXC;
    rSmoothBPC.cor.bits.BPC_RANK_IDXR	    = BPC_RANK_IDXR;
    rSmoothBPC.cor.bits.BPC_RANK_IDXG	    = BPC_RANK_IDXG;
    rSmoothBPC.th2_c.bits.BPC_RANK_IDXB	    = BPC_RANK_IDXB;
    rSmoothBPC.con.bits.BPC_BLD_MODE	    = BPC_BLD_MODE;
    rSmoothBPC.th2_c.bits.BPC_BLD_LWB 	    = BPC_BLD_LWB;
    rSmoothBPC.th2.bits.BPC_BLD0    	    = BPC_BLD0;
    rSmoothBPC.th2.bits.BPC_BLD1    	    = BPC_BLD1;
    rSmoothBPC.th1.bits.BPC_BLD_SLP0  	    = BPC_BLD_SLP0;

    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_EN              = %d, %d, %d", rLowerBPC.con.bits.BPC_EN, rSmoothBPC.con.bits.BPC_EN, rUpperBPC.con.bits.BPC_EN);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_LUT_EN          = %d, %d, %d", rLowerBPC.con.bits.BPC_LUT_EN, rSmoothBPC.con.bits.BPC_LUT_EN, rUpperBPC.con.bits.BPC_LUT_EN);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_DTC_MODE        = %d, %d, %d", rLowerBPC.con.bits.BPC_DTC_MODE, rSmoothBPC.con.bits.BPC_DTC_MODE, rUpperBPC.con.bits.BPC_DTC_MODE);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_CRC_MODE        = %d, %d, %d", rLowerBPC.con.bits.BPC_CRC_MODE, rSmoothBPC.con.bits.BPC_CRC_MODE, rUpperBPC.con.bits.BPC_CRC_MODE);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_RNG             = %d, %d, %d", rLowerBPC.dtc.bits.BPC_RNG, rSmoothBPC.dtc.bits.BPC_RNG, rUpperBPC.dtc.bits.BPC_RNG);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_DIR_TH          = %d, %d, %d", rLowerBPC.cor.bits.BPC_DIR_TH, rSmoothBPC.cor.bits.BPC_DIR_TH, rUpperBPC.cor.bits.BPC_DIR_TH);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_DIR_TH2         = %d, %d, %d", rLowerBPC.cor.bits.BPC_DIR_TH2, rSmoothBPC.cor.bits.BPC_DIR_TH2, rUpperBPC.cor.bits.BPC_DIR_TH2);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_DIR_MAX         = %d, %d, %d", rLowerBPC.cor.bits.BPC_DIR_MAX, rSmoothBPC.cor.bits.BPC_DIR_MAX, rUpperBPC.cor.bits.BPC_DIR_MAX);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_TH_LWB          = %d, %d, %d", rLowerBPC.th1.bits.BPC_TH_LWB, rSmoothBPC.th1.bits.BPC_TH_LWB, rUpperBPC.th1.bits.BPC_TH_LWB);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_TH_SLA          = %d, %d, %d", rLowerBPC.th3.bits.BPC_TH_SLA, rSmoothBPC.th3.bits.BPC_TH_SLA, rUpperBPC.th3.bits.BPC_TH_SLA);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_DK_TH_SLA       = %d, %d, %d", rLowerBPC.th4.bits.BPC_DK_TH_SLA, rSmoothBPC.th4.bits.BPC_DK_TH_SLA, rUpperBPC.th4.bits.BPC_DK_TH_SLA);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_TH_SLB          = %d, %d, %d", rLowerBPC.th3.bits.BPC_TH_SLB, rSmoothBPC.th3.bits.BPC_TH_SLB, rUpperBPC.th3.bits.BPC_TH_SLB);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_DK_TH_SLB       = %d, %d, %d", rLowerBPC.th4.bits.BPC_DK_TH_SLB, rSmoothBPC.th4.bits.BPC_DK_TH_SLB, rUpperBPC.th4.bits.BPC_DK_TH_SLB);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_TH_XA           = %d, %d, %d", rLowerBPC.th3.bits.BPC_TH_XA, rSmoothBPC.th3.bits.BPC_TH_XA, rUpperBPC.th3.bits.BPC_TH_XA);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_DK_TH_XA        = %d, %d, %d", rLowerBPC.th4.bits.BPC_DK_TH_XA, rSmoothBPC.th4.bits.BPC_DK_TH_XA, rUpperBPC.th4.bits.BPC_DK_TH_XA);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_TH_XB           = %d, %d, %d", rLowerBPC.th3.bits.BPC_TH_XB, rSmoothBPC.th3.bits.BPC_TH_XB, rUpperBPC.th3.bits.BPC_TH_XB);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_DK_TH_XB        = %d, %d, %d", rLowerBPC.th4.bits.BPC_DK_TH_XB, rSmoothBPC.th4.bits.BPC_DK_TH_XB, rUpperBPC.th4.bits.BPC_DK_TH_XB);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_TH_Y            = %d, %d, %d", rLowerBPC.th1.bits.BPC_TH_Y, rSmoothBPC.th1.bits.BPC_TH_Y, rUpperBPC.th1.bits.BPC_TH_Y);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_TH_UPB          = %d, %d, %d", rLowerBPC.th2.bits.BPC_TH_UPB, rSmoothBPC.th2.bits.BPC_TH_UPB, rUpperBPC.th2.bits.BPC_TH_UPB);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_C_TH_LWB        = %d, %d, %d", rLowerBPC.th1_c.bits.BPC_C_TH_LWB, rSmoothBPC.th1_c.bits.BPC_C_TH_LWB, rUpperBPC.th1_c.bits.BPC_C_TH_LWB);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_C_TH_SLA        = %d, %d, %d", rLowerBPC.th3_c.bits.BPC_C_TH_SLA, rSmoothBPC.th3_c.bits.BPC_C_TH_SLA, rUpperBPC.th3_c.bits.BPC_C_TH_SLA);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_C_TH_SLB        = %d, %d, %d", rLowerBPC.th3_c.bits.BPC_C_TH_SLB, rSmoothBPC.th3_c.bits.BPC_C_TH_SLB, rUpperBPC.th3_c.bits.BPC_C_TH_SLB);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_C_TH_XA         = %d, %d, %d", rLowerBPC.th3_c.bits.BPC_C_TH_XA, rSmoothBPC.th3_c.bits.BPC_C_TH_XA, rUpperBPC.th3_c.bits.BPC_C_TH_XA);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_C_TH_XB         = %d, %d, %d", rLowerBPC.th3_c.bits.BPC_C_TH_XB, rSmoothBPC.th3_c.bits.BPC_C_TH_XB, rUpperBPC.th3_c.bits.BPC_C_TH_XB);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_C_TH_Y          = %d, %d, %d", rLowerBPC.th1_c.bits.BPC_C_TH_Y, rSmoothBPC.th1_c.bits.BPC_C_TH_Y, rUpperBPC.th1_c.bits.BPC_C_TH_Y);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_C_TH_UPB        = %d, %d, %d", rLowerBPC.th2_c.bits.BPC_C_TH_UPB, rSmoothBPC.th2_c.bits.BPC_C_TH_UPB, rUpperBPC.th2_c.bits.BPC_C_TH_UPB);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_NO_LV           = %d, %d, %d", rLowerBPC.dtc.bits.BPC_NO_LV, rSmoothBPC.dtc.bits.BPC_NO_LV, rUpperBPC.dtc.bits.BPC_NO_LV);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_CT_LV           = %d, %d, %d", rLowerBPC.dtc.bits.BPC_CT_LV, rSmoothBPC.dtc.bits.BPC_CT_LV, rUpperBPC.dtc.bits.BPC_CT_LV);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_TH_MUL          = %d, %d, %d", rLowerBPC.dtc.bits.BPC_TH_MUL, rSmoothBPC.dtc.bits.BPC_TH_MUL, rUpperBPC.dtc.bits.BPC_TH_MUL);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_CS_MODE         = %d, %d, %d", rLowerBPC.con.bits.BPC_CS_MODE, rSmoothBPC.con.bits.BPC_CS_MODE, rUpperBPC.con.bits.BPC_CS_MODE);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_CS_RNG          = %d, %d, %d", rLowerBPC.dtc.bits.BPC_CS_RNG, rSmoothBPC.dtc.bits.BPC_CS_RNG, rUpperBPC.dtc.bits.BPC_CS_RNG);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_TABLE_END_MODE  = %d, %d, %d", rLowerBPC.con.bits.BPC_TABLE_END_MODE, rSmoothBPC.con.bits.BPC_TABLE_END_MODE, rUpperBPC.con.bits.BPC_TABLE_END_MODE);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_AVG_MODE        = %d, %d, %d", rLowerBPC.con.bits.BPC_AVG_MODE, rSmoothBPC.con.bits.BPC_AVG_MODE, rUpperBPC.con.bits.BPC_AVG_MODE);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BNR_LE_INV_CTL      = %d, %d, %d", rLowerBPC.con.bits.BNR_LE_INV_CTL, rSmoothBPC.con.bits.BNR_LE_INV_CTL, rUpperBPC.con.bits.BNR_LE_INV_CTL);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BNR_OSC_COUNT       = %d, %d, %d", rLowerBPC.con.bits.BNR_OSC_COUNT, rSmoothBPC.con.bits.BNR_OSC_COUNT, rUpperBPC.con.bits.BNR_OSC_COUNT);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BNR_EDGE            = %d, %d, %d", rLowerBPC.con.bits.BNR_EDGE, rSmoothBPC.con.bits.BNR_EDGE, rUpperBPC.con.bits.BNR_EDGE);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_XOFFSET         = %d, %d, %d", rLowerBPC.tbli1.bits.BPC_XOFFSET, rSmoothBPC.tbli1.bits.BPC_XOFFSET, rUpperBPC.tbli1.bits.BPC_XOFFSET);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_YOFFSET         = %d, %d, %d", rLowerBPC.tbli1.bits.BPC_YOFFSET, rSmoothBPC.tbli1.bits.BPC_YOFFSET, rUpperBPC.tbli1.bits.BPC_YOFFSET);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_XSIZE           = %d, %d, %d", rLowerBPC.tbli2.bits.BPC_XSIZE, rSmoothBPC.tbli2.bits.BPC_XSIZE, rUpperBPC.tbli2.bits.BPC_XSIZE);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_YSIZE           = %d, %d, %d", rLowerBPC.tbli2.bits.BPC_YSIZE, rSmoothBPC.tbli2.bits.BPC_YSIZE, rUpperBPC.tbli2.bits.BPC_YSIZE);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_EXC             = %d, %d, %d", rLowerBPC.con.bits.BPC_EXC, rSmoothBPC.con.bits.BPC_EXC, rUpperBPC.con.bits.BPC_EXC);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_RANK_IDXR       = %d, %d, %d", rLowerBPC.cor.bits.BPC_RANK_IDXR, rSmoothBPC.cor.bits.BPC_RANK_IDXR, rUpperBPC.cor.bits.BPC_RANK_IDXR);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_RANK_IDXG       = %d, %d, %d", rLowerBPC.cor.bits.BPC_RANK_IDXG, rSmoothBPC.cor.bits.BPC_RANK_IDXG, rUpperBPC.cor.bits.BPC_RANK_IDXG);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_RANK_IDXB       = %d, %d, %d", rLowerBPC.th2_c.bits.BPC_RANK_IDXB, rSmoothBPC.th2_c.bits.BPC_RANK_IDXB, rUpperBPC.th2_c.bits.BPC_RANK_IDXB);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_BLD_MODE        = %d, %d, %d", rLowerBPC.con.bits.BPC_BLD_MODE, rSmoothBPC.con.bits.BPC_BLD_MODE, rUpperBPC.con.bits.BPC_BLD_MODE);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_BLD_LWB         = %d, %d, %d", rLowerBPC.th2_c.bits.BPC_BLD_LWB, rSmoothBPC.th2_c.bits.BPC_BLD_LWB, rUpperBPC.th2_c.bits.BPC_BLD_LWB);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_BLD0            = %d, %d, %d", rLowerBPC.th2.bits.BPC_BLD0, rSmoothBPC.th2.bits.BPC_BLD0, rUpperBPC.th2.bits.BPC_BLD0);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_BLD1            = %d, %d, %d", rLowerBPC.th2.bits.BPC_BLD1, rSmoothBPC.th2.bits.BPC_BLD1, rUpperBPC.th2.bits.BPC_BLD1);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_BLD_SLP0        = %d, %d, %d", rLowerBPC.th1.bits.BPC_BLD_SLP0, rSmoothBPC.th1.bits.BPC_BLD_SLP0, rUpperBPC.th1.bits.BPC_BLD_SLP0);
}

MINT32 InterParam_NR1(MINT32 ISO, MINT32 ISOpre, MINT32 ISOpos, MINT32 PARAMpre, MINT32 PARAMpos, MINT32 Method) {
	MINT32 InterPARAM = 0;

	if(ISOpre == ISOpos) {
		//	To prevent someone sets different PARAMpre and PARAMpos
		return (MINT32)((PARAMpre + PARAMpos + 1) / 2);
	}

	switch(Method) {
	case 1:
		if(abs(ISO - ISOpre) > abs(ISOpos - ISO)) {
			InterPARAM = PARAMpos;
		}
		else{
			InterPARAM = PARAMpre;
		}
		break;
	case 0:
	default:
		double RATIOpre = (double)(ISOpos - ISO) / (double)(ISOpos - ISOpre);
		double RATIOpos = 1.0 - (double)(RATIOpre);
		InterPARAM = (MINT32)(RATIOpre * PARAMpre + RATIOpos * PARAMpos + 0.5);
	    break;
	}

	return InterPARAM;
}


MVOID SmoothNR1(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_BNR_NR1_T const& rUpperNR1, // NR1 settings for upper ISO
                ISP_NVRAM_BNR_NR1_T const& rLowerNR1,   // NR1 settings for lower ISO
                ISP_NVRAM_BNR_NR1_T& rSmoothNR1)   // Output
{

		char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.smooth_nr1.enable", value, "0");
    MBOOL bSmoothNR1Debug = atoi(value);
    MY_LOG_IF(bSmoothNR1Debug,"[%s()] u4RealISO: %d, u4UpperISO = %d, u4LowerISO = %d\n", __FUNCTION__, u4RealISO, u4UpperISO, u4LowerISO);

		MINT32 CT_ENpre        =  rLowerNR1.con.bits.NR1_CT_EN;
		MINT32 CT_MBNDpre      =  rLowerNR1.ct_con.bits.NR1_MBND;
		MINT32 CT_MDpre        =  rLowerNR1.ct_con.bits.NR1_CT_MD;
		MINT32 CT_MD2pre       =  rLowerNR1.ct_con.bits.NR1_CT_MD2;
		MINT32 CT_THRDpre      =  rLowerNR1.ct_con.bits.NR1_CT_THRD;
		MINT32 CT_DIVpre       =  rLowerNR1.ct_con.bits.NR1_CT_DIV;
		MINT32 CT_SLOPEpre     =  rLowerNR1.ct_con.bits.NR1_CT_SLOPE;

		MINT32 CT_ENpos        =  rUpperNR1.con.bits.NR1_CT_EN;
		MINT32 CT_MBNDpos      =  rUpperNR1.ct_con.bits.NR1_MBND;
		MINT32 CT_MDpos        =  rUpperNR1.ct_con.bits.NR1_CT_MD;
		MINT32 CT_MD2pos       =  rUpperNR1.ct_con.bits.NR1_CT_MD2;
		MINT32 CT_THRDpos      =  rUpperNR1.ct_con.bits.NR1_CT_THRD;
		MINT32 CT_DIVpos       =  rUpperNR1.ct_con.bits.NR1_CT_DIV;
		MINT32 CT_SLOPEpos     =  rUpperNR1.ct_con.bits.NR1_CT_SLOPE;

	// interpolation //
	MINT32 ISO = u4RealISO;
	MINT32 ISOpre = u4LowerISO;
	MINT32 ISOpos = u4UpperISO;

 	MINT32 CT_EN         = InterParam_NR1(ISO, ISOpre, ISOpos, CT_ENpre, CT_ENpos, 0);
    MINT32 CT_MBND       = InterParam_NR1(ISO, ISOpre, ISOpos, CT_MBNDpre, CT_MBNDpos, 0);
    MINT32 CT_MD         = InterParam_NR1(ISO, ISOpre, ISOpos, CT_MDpre, CT_MDpos, 0);
    MINT32 CT_MD2        = InterParam_NR1(ISO, ISOpre, ISOpos, CT_MD2pre, CT_MD2pos, 0);
    MINT32 CT_THRD       = InterParam_NR1(ISO, ISOpre, ISOpos, CT_THRDpre, CT_THRDpos, 0);
    MINT32 CT_DIV        = InterParam_NR1(ISO, ISOpre, ISOpos, CT_DIVpre, CT_DIVpos, 0);
    MINT32 CT_SLOPE      = InterParam_NR1(ISO, ISOpre, ISOpos, CT_SLOPEpre, CT_SLOPEpos, 0);

    rSmoothNR1.con.bits.NR1_CT_EN          = CT_EN;
    rSmoothNR1.ct_con.bits.NR1_MBND        = CT_MBND;
    rSmoothNR1.ct_con.bits.NR1_CT_MD       = CT_MD;
    rSmoothNR1.ct_con.bits.NR1_CT_MD2      = CT_MD2;
    rSmoothNR1.ct_con.bits.NR1_CT_THRD     = CT_THRD;
    rSmoothNR1.ct_con.bits.NR1_CT_DIV      = CT_DIV;
    rSmoothNR1.ct_con.bits.NR1_CT_SLOPE    = CT_SLOPE;

    MY_LOG_IF(bSmoothNR1Debug,"[L,S,U]CT_EN    = %d, %d, %d", rLowerNR1.con.bits.NR1_CT_EN, rSmoothNR1.con.bits.NR1_CT_EN, rUpperNR1.con.bits.NR1_CT_EN);
    MY_LOG_IF(bSmoothNR1Debug,"[L,S,U]CT_MBND  = %d, %d, %d", rLowerNR1.ct_con.bits.NR1_MBND, rSmoothNR1.ct_con.bits.NR1_MBND, rUpperNR1.ct_con.bits.NR1_MBND);
    MY_LOG_IF(bSmoothNR1Debug,"[L,S,U]CT_MD    = %d, %d, %d", rLowerNR1.ct_con.bits.NR1_CT_MD, rSmoothNR1.ct_con.bits.NR1_CT_MD, rUpperNR1.ct_con.bits.NR1_CT_MD);
    MY_LOG_IF(bSmoothNR1Debug,"[L,S,U]CT_MD2   = %d, %d, %d", rLowerNR1.ct_con.bits.NR1_CT_MD2, rSmoothNR1.ct_con.bits.NR1_CT_MD2, rUpperNR1.ct_con.bits.NR1_CT_MD2);
    MY_LOG_IF(bSmoothNR1Debug,"[L,S,U]CT_THRD  = %d, %d, %d", rLowerNR1.ct_con.bits.NR1_CT_THRD, rSmoothNR1.ct_con.bits.NR1_CT_THRD, 	rUpperNR1.ct_con.bits.NR1_CT_THRD);
    MY_LOG_IF(bSmoothNR1Debug,"[L,S,U]CT_DIV   = %d, %d, %d", rLowerNR1.ct_con.bits.NR1_CT_DIV, rSmoothNR1.ct_con.bits.NR1_CT_DIV, rUpperNR1.ct_con.bits.NR1_CT_DIV);
    MY_LOG_IF(bSmoothNR1Debug,"[L,S,U]CT_SLOPE = %d, %d, %d", rLowerNR1.ct_con.bits.NR1_CT_SLOPE, rSmoothNR1.ct_con.bits.NR1_CT_SLOPE, rUpperNR1.ct_con.bits.NR1_CT_SLOPE);
}

MINT32 InterParam_PDC(MINT32 ISO, MINT32 ISOpre, MINT32 ISOpos, MINT32 PARAMpre, MINT32 PARAMpos, MINT32 Method) {
	MINT32 InterPARAM = 0;

	if(ISOpre == ISOpos) {
		//	To prevent someone sets different PARAMpre and PARAMpos
		return (MINT32)((PARAMpre + PARAMpos + 1) / 2);
	}

	switch(Method) {
	case 1:
		if(abs(ISO - ISOpre) > abs(ISOpos - ISO)) {
			InterPARAM = PARAMpos;
		}
		else{
			InterPARAM = PARAMpre;
		}
		break;
	case 0:
	default:
		double RATIOpre = (double)(ISOpos - ISO) / (double)(ISOpos - ISOpre);
		double RATIOpos = 1.0 - (double)(RATIOpre);
		InterPARAM = (MINT32)(RATIOpre * PARAMpre + RATIOpos * PARAMpos + 0.5);
	    break;
	}

	return InterPARAM;
}

MVOID SmoothPDC(MUINT32 u4RealISO,  // Real ISO
                    MUINT32 u4UpperISO, // Upper ISO
                    MUINT32 u4LowerISO, // Lower ISO
                    ISP_NVRAM_BNR_PDC_T const& rUpperBNR_PDC, // NR1 settings for upper ISO
                    ISP_NVRAM_BNR_PDC_T const& rLowerBNR_PDC,   // NR1 settings for lower ISO
                    ISP_NVRAM_BNR_PDC_T& rSmoothBNR_PDC)   // Output
{

	char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.smooth_bnr_pdc.enable", value, "0");
    MBOOL bSmoothBNR_PDCDebug = atoi(value);
    MY_LOG_IF(bSmoothBNR_PDCDebug,"[%s()] u4RealISO: %d, u4UpperISO = %d, u4LowerISO = %d\n", __FUNCTION__, u4RealISO, u4UpperISO, u4LowerISO);

    MINT32 PDC_ENpre           =   rLowerBNR_PDC.con.bits.PDC_EN;
	MINT32 PDC_CTpre           =   rLowerBNR_PDC.con.bits.PDC_CT;
	MINT32 PDC_GCF_L00pre      =   rLowerBNR_PDC.gain_l0.bits.PDC_GCF_L00;
	MINT32 PDC_GCF_L10pre      =   rLowerBNR_PDC.gain_l0.bits.PDC_GCF_L10;
	MINT32 PDC_GCF_L01pre      =   rLowerBNR_PDC.gain_l1.bits.PDC_GCF_L01;
	MINT32 PDC_GCF_L20pre      =   rLowerBNR_PDC.gain_l1.bits.PDC_GCF_L20;
	MINT32 PDC_GCF_L11pre      =   rLowerBNR_PDC.gain_l2.bits.PDC_GCF_L11;
	MINT32 PDC_GCF_L02pre      =   rLowerBNR_PDC.gain_l2.bits.PDC_GCF_L02;
    MINT32 PDC_GCF_L30pre      =   rLowerBNR_PDC.gain_l3.bits.PDC_GCF_L30;
	MINT32 PDC_GCF_L21pre      =   rLowerBNR_PDC.gain_l3.bits.PDC_GCF_L21;
	MINT32 PDC_GCF_L12pre      =   rLowerBNR_PDC.gain_l4.bits.PDC_GCF_L12;
    MINT32 PDC_GCF_L03pre      =   rLowerBNR_PDC.gain_l4.bits.PDC_GCF_L03;
	MINT32 PDC_GCF_R00pre      =   rLowerBNR_PDC.gain_r0.bits.PDC_GCF_R00;
	MINT32 PDC_GCF_R10pre      =   rLowerBNR_PDC.gain_r0.bits.PDC_GCF_R10;
	MINT32 PDC_GCF_R01pre      =   rLowerBNR_PDC.gain_r1.bits.PDC_GCF_R01;
	MINT32 PDC_GCF_R20pre      =   rLowerBNR_PDC.gain_r1.bits.PDC_GCF_R20;
	MINT32 PDC_GCF_R11pre      =   rLowerBNR_PDC.gain_r2.bits.PDC_GCF_R11;
	MINT32 PDC_GCF_R02pre      =   rLowerBNR_PDC.gain_r2.bits.PDC_GCF_R02;
    MINT32 PDC_GCF_R30pre      =   rLowerBNR_PDC.gain_r3.bits.PDC_GCF_R30;
	MINT32 PDC_GCF_R21pre      =   rLowerBNR_PDC.gain_r3.bits.PDC_GCF_R21;
	MINT32 PDC_GCF_R12pre      =   rLowerBNR_PDC.gain_r4.bits.PDC_GCF_R12;
    MINT32 PDC_GCF_R03pre      =   rLowerBNR_PDC.gain_r4.bits.PDC_GCF_R03;
	MINT32 PDC_GTHpre          =   rLowerBNR_PDC.th_gb.bits.PDC_GTH;
	MINT32 PDC_BTHpre          =   rLowerBNR_PDC.th_gb.bits.PDC_BTH;
	MINT32 PDC_ITHpre          =   rLowerBNR_PDC.th_ia.bits.PDC_ITH;
	MINT32 PDC_ATHpre          =   rLowerBNR_PDC.th_ia.bits.PDC_ATH;
	MINT32 PDC_NTHpre          =   rLowerBNR_PDC.th_hd.bits.PDC_NTH;
	MINT32 PDC_DTHpre          =   rLowerBNR_PDC.th_hd.bits.PDC_DTH;
	MINT32 PDC_GSLpre          =   rLowerBNR_PDC.sl.bits.PDC_GSL;
	MINT32 PDC_BSLpre          =   rLowerBNR_PDC.sl.bits.PDC_BSL;
	MINT32 PDC_ISLpre          =   rLowerBNR_PDC.sl.bits.PDC_ISL;
	MINT32 PDC_ASLpre          =   rLowerBNR_PDC.sl.bits.PDC_ASL;
	MINT32 PDC_GCF_NORMpre     =   rLowerBNR_PDC.sl.bits.PDC_GCF_NORM;
	MINT32 PDC_XCENTERpre      =   rLowerBNR_PDC.pos.bits.PDC_XCENTER;
	MINT32 PDC_YCENTERpre      =   rLowerBNR_PDC.pos.bits.PDC_YCENTER;

	MINT32 PDC_ENpos           =   rUpperBNR_PDC.con.bits.PDC_EN;
	MINT32 PDC_CTpos           =   rUpperBNR_PDC.con.bits.PDC_CT;
	MINT32 PDC_GCF_L00pos      =   rUpperBNR_PDC.gain_l0.bits.PDC_GCF_L00;
	MINT32 PDC_GCF_L10pos      =   rUpperBNR_PDC.gain_l0.bits.PDC_GCF_L10;
	MINT32 PDC_GCF_L01pos      =   rUpperBNR_PDC.gain_l1.bits.PDC_GCF_L01;
	MINT32 PDC_GCF_L20pos      =   rUpperBNR_PDC.gain_l1.bits.PDC_GCF_L20;
	MINT32 PDC_GCF_L11pos      =   rUpperBNR_PDC.gain_l2.bits.PDC_GCF_L11;
	MINT32 PDC_GCF_L02pos      =   rUpperBNR_PDC.gain_l2.bits.PDC_GCF_L02;
    MINT32 PDC_GCF_L30pos      =   rUpperBNR_PDC.gain_l3.bits.PDC_GCF_L30;
	MINT32 PDC_GCF_L21pos      =   rUpperBNR_PDC.gain_l3.bits.PDC_GCF_L21;
	MINT32 PDC_GCF_L12pos      =   rUpperBNR_PDC.gain_l4.bits.PDC_GCF_L12;
    MINT32 PDC_GCF_L03pos      =   rUpperBNR_PDC.gain_l4.bits.PDC_GCF_L03;
	MINT32 PDC_GCF_R00pos      =   rUpperBNR_PDC.gain_r0.bits.PDC_GCF_R00;
	MINT32 PDC_GCF_R10pos      =   rUpperBNR_PDC.gain_r0.bits.PDC_GCF_R10;
	MINT32 PDC_GCF_R01pos      =   rUpperBNR_PDC.gain_r1.bits.PDC_GCF_R01;
	MINT32 PDC_GCF_R20pos      =   rUpperBNR_PDC.gain_r1.bits.PDC_GCF_R20;
	MINT32 PDC_GCF_R11pos      =   rUpperBNR_PDC.gain_r2.bits.PDC_GCF_R11;
	MINT32 PDC_GCF_R02pos      =   rUpperBNR_PDC.gain_r2.bits.PDC_GCF_R02;
    MINT32 PDC_GCF_R30pos      =   rUpperBNR_PDC.gain_r3.bits.PDC_GCF_R30;
	MINT32 PDC_GCF_R21pos      =   rUpperBNR_PDC.gain_r3.bits.PDC_GCF_R21;
	MINT32 PDC_GCF_R12pos      =   rUpperBNR_PDC.gain_r4.bits.PDC_GCF_R12;
    MINT32 PDC_GCF_R03pos      =   rUpperBNR_PDC.gain_r4.bits.PDC_GCF_R03;
	MINT32 PDC_GTHpos          =   rUpperBNR_PDC.th_gb.bits.PDC_GTH;
	MINT32 PDC_BTHpos          =   rUpperBNR_PDC.th_gb.bits.PDC_BTH;
	MINT32 PDC_ITHpos          =   rUpperBNR_PDC.th_ia.bits.PDC_ITH;
	MINT32 PDC_ATHpos          =   rUpperBNR_PDC.th_ia.bits.PDC_ATH;
	MINT32 PDC_NTHpos          =   rUpperBNR_PDC.th_hd.bits.PDC_NTH;
	MINT32 PDC_DTHpos          =   rUpperBNR_PDC.th_hd.bits.PDC_DTH;
	MINT32 PDC_GSLpos          =   rUpperBNR_PDC.sl.bits.PDC_GSL;
	MINT32 PDC_BSLpos          =   rUpperBNR_PDC.sl.bits.PDC_BSL;
	MINT32 PDC_ISLpos          =   rUpperBNR_PDC.sl.bits.PDC_ISL;
	MINT32 PDC_ASLpos          =   rUpperBNR_PDC.sl.bits.PDC_ASL;
	MINT32 PDC_GCF_NORMpos     =   rUpperBNR_PDC.sl.bits.PDC_GCF_NORM;
	MINT32 PDC_XCENTERpos      =   rUpperBNR_PDC.pos.bits.PDC_XCENTER;
	MINT32 PDC_YCENTERpos      =   rUpperBNR_PDC.pos.bits.PDC_YCENTER;

    // interpolation //
    MINT32 ISO = u4RealISO;
    MINT32 ISOpre = u4LowerISO;
    MINT32 ISOpos = u4UpperISO;

    MINT32 PDC_EN       = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_ENpre, PDC_ENpos, 1);
	MINT32 PDC_CT       = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_CTpre, PDC_CTpos, 1);
	MINT32 PDC_GCF_L00  = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GCF_L00pre, PDC_GCF_L00pos, 0);
	MINT32 PDC_GCF_L10  = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GCF_L10pre, PDC_GCF_L10pos, 0);
	MINT32 PDC_GCF_L01  = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GCF_L01pre, PDC_GCF_L01pos, 0);
	MINT32 PDC_GCF_L20  = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GCF_L20pre, PDC_GCF_L20pos, 0);
	MINT32 PDC_GCF_L11  = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GCF_L11pre, PDC_GCF_L11pos, 0);
	MINT32 PDC_GCF_L02  = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GCF_L02pre, PDC_GCF_L02pos, 0);
	MINT32 PDC_GCF_L30  = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GCF_L30pre, PDC_GCF_L30pos, 0);
	MINT32 PDC_GCF_L21  = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GCF_L21pre, PDC_GCF_L21pos, 0);
	MINT32 PDC_GCF_L12  = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GCF_L12pre, PDC_GCF_L12pos, 0);
	MINT32 PDC_GCF_L03  = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GCF_L03pre, PDC_GCF_L03pos, 0);
	MINT32 PDC_GCF_R00  = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GCF_R00pre, PDC_GCF_R00pos, 0);
	MINT32 PDC_GCF_R10  = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GCF_R10pre, PDC_GCF_R10pos, 0);
	MINT32 PDC_GCF_R01  = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GCF_R01pre, PDC_GCF_R01pos, 0);
	MINT32 PDC_GCF_R20  = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GCF_R20pre, PDC_GCF_R20pos, 0);
	MINT32 PDC_GCF_R11  = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GCF_R11pre, PDC_GCF_R11pos, 0);
	MINT32 PDC_GCF_R02  = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GCF_R02pre, PDC_GCF_R02pos, 0);
	MINT32 PDC_GCF_R30  = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GCF_R30pre, PDC_GCF_R30pos, 0);
	MINT32 PDC_GCF_R21  = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GCF_R21pre, PDC_GCF_R21pos, 0);
	MINT32 PDC_GCF_R12  = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GCF_R12pre, PDC_GCF_R12pos, 0);
	MINT32 PDC_GCF_R03  = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GCF_R03pre, PDC_GCF_R03pos, 0);
    MINT32 PDC_GTH      = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GTHpre, PDC_GTHpos, 0);
	MINT32 PDC_BTH      = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_BTHpre, PDC_BTHpos, 0);
	MINT32 PDC_ITH      = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_ITHpre, PDC_ITHpos, 0);
	MINT32 PDC_ATH      = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_ATHpre, PDC_ATHpos, 0);
	MINT32 PDC_NTH      = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_NTHpre, PDC_NTHpos, 0);
	MINT32 PDC_DTH      = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_DTHpre, PDC_DTHpos, 0);
	MINT32 PDC_GSL      = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GSLpre, PDC_GSLpos, 0);
	MINT32 PDC_BSL      = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_BSLpre, PDC_BSLpos, 0);
	MINT32 PDC_ISL      = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_ISLpre, PDC_ISLpos, 0);
	MINT32 PDC_ASL      = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_ASLpre, PDC_ASLpos, 0);
	MINT32 PDC_GCF_NORM = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GCF_NORMpre, PDC_GCF_NORMpos, 0);
	MINT32 PDC_XCENTER  = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_XCENTERpre, PDC_XCENTERpos, 0);
	MINT32 PDC_YCENTER  = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_YCENTERpre, PDC_YCENTERpos, 0);

    rSmoothBNR_PDC.con.bits.PDC_EN          = PDC_EN;
    rSmoothBNR_PDC.con.bits.PDC_CT          = PDC_CT;
    rSmoothBNR_PDC.gain_l0.bits.PDC_GCF_L00 = PDC_GCF_L00;
	rSmoothBNR_PDC.gain_l0.bits.PDC_GCF_L10 = PDC_GCF_L10;
	rSmoothBNR_PDC.gain_l1.bits.PDC_GCF_L01 = PDC_GCF_L01;
	rSmoothBNR_PDC.gain_l1.bits.PDC_GCF_L20 = PDC_GCF_L20;
	rSmoothBNR_PDC.gain_l2.bits.PDC_GCF_L11 = PDC_GCF_L11;
    rSmoothBNR_PDC.gain_l2.bits.PDC_GCF_L02 = PDC_GCF_L02;
	rSmoothBNR_PDC.gain_l3.bits.PDC_GCF_L30 = PDC_GCF_L30;
	rSmoothBNR_PDC.gain_l3.bits.PDC_GCF_L21 = PDC_GCF_L21;
	rSmoothBNR_PDC.gain_l4.bits.PDC_GCF_L12 = PDC_GCF_L12;
	rSmoothBNR_PDC.gain_l4.bits.PDC_GCF_L03 = PDC_GCF_L03;
	rSmoothBNR_PDC.gain_r0.bits.PDC_GCF_R00 = PDC_GCF_R00;
	rSmoothBNR_PDC.gain_r0.bits.PDC_GCF_R10 = PDC_GCF_R10;
	rSmoothBNR_PDC.gain_r1.bits.PDC_GCF_R01 = PDC_GCF_R01;
	rSmoothBNR_PDC.gain_r1.bits.PDC_GCF_R20 = PDC_GCF_R20;
    rSmoothBNR_PDC.gain_r2.bits.PDC_GCF_R11 = PDC_GCF_R11;
    rSmoothBNR_PDC.gain_r2.bits.PDC_GCF_R02 = PDC_GCF_R02;
	rSmoothBNR_PDC.gain_r3.bits.PDC_GCF_R30 = PDC_GCF_R30;
	rSmoothBNR_PDC.gain_r3.bits.PDC_GCF_R21 = PDC_GCF_R21;
	rSmoothBNR_PDC.gain_r4.bits.PDC_GCF_R12 = PDC_GCF_R12;
	rSmoothBNR_PDC.gain_r4.bits.PDC_GCF_R03 = PDC_GCF_R03;
	rSmoothBNR_PDC.th_gb.bits.PDC_GTH       = PDC_GTH;
	rSmoothBNR_PDC.th_gb.bits.PDC_BTH       = PDC_BTH;
	rSmoothBNR_PDC.th_ia.bits.PDC_ITH       = PDC_ITH;
	rSmoothBNR_PDC.th_ia.bits.PDC_ATH       = PDC_ATH;
	rSmoothBNR_PDC.th_hd.bits.PDC_NTH       = PDC_NTH;
	rSmoothBNR_PDC.th_hd.bits.PDC_DTH       = PDC_DTH;
	rSmoothBNR_PDC.sl.bits.PDC_GSL          = PDC_GSL;
	rSmoothBNR_PDC.sl.bits.PDC_BSL          = PDC_BSL;
	rSmoothBNR_PDC.sl.bits.PDC_ISL          = PDC_ISL;
	rSmoothBNR_PDC.sl.bits.PDC_ASL          = PDC_ASL;
	rSmoothBNR_PDC.sl.bits.PDC_GCF_NORM     = PDC_GCF_NORM;
	rSmoothBNR_PDC.pos.bits.PDC_XCENTER     = PDC_XCENTER;
	rSmoothBNR_PDC.pos.bits.PDC_YCENTER     = PDC_YCENTER;


    MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_EN       = %d, %d, %d", rLowerBNR_PDC.con.bits.PDC_EN, rSmoothBNR_PDC.con.bits.PDC_EN, rUpperBNR_PDC.con.bits.PDC_EN);
	MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_CT       = %d, %d, %d", rLowerBNR_PDC.con.bits.PDC_CT, rSmoothBNR_PDC.con.bits.PDC_CT, rUpperBNR_PDC.con.bits.PDC_CT);
	MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_L00  = %d, %d, %d", rLowerBNR_PDC.gain_l0.bits.PDC_GCF_L00, rSmoothBNR_PDC.gain_l0.bits.PDC_GCF_L00, rUpperBNR_PDC.gain_l0.bits.PDC_GCF_L00);
	MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_L10  = %d, %d, %d", rLowerBNR_PDC.gain_l0.bits.PDC_GCF_L10, rSmoothBNR_PDC.gain_l0.bits.PDC_GCF_L10, rUpperBNR_PDC.gain_l0.bits.PDC_GCF_L10);
	MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_L01  = %d, %d, %d", rLowerBNR_PDC.gain_l1.bits.PDC_GCF_L01, rSmoothBNR_PDC.gain_l1.bits.PDC_GCF_L01, rUpperBNR_PDC.gain_l1.bits.PDC_GCF_L01);
	MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_L20  = %d, %d, %d", rLowerBNR_PDC.gain_l1.bits.PDC_GCF_L20, rSmoothBNR_PDC.gain_l1.bits.PDC_GCF_L20, rUpperBNR_PDC.gain_l1.bits.PDC_GCF_L20);
	MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_L11  = %d, %d, %d", rLowerBNR_PDC.gain_l2.bits.PDC_GCF_L11, rSmoothBNR_PDC.gain_l2.bits.PDC_GCF_L11, rUpperBNR_PDC.gain_l2.bits.PDC_GCF_L11);
	MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_L02  = %d, %d, %d", rLowerBNR_PDC.gain_l2.bits.PDC_GCF_L02, rSmoothBNR_PDC.gain_l2.bits.PDC_GCF_L02, rUpperBNR_PDC.gain_l2.bits.PDC_GCF_L02);
	MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_L30  = %d, %d, %d", rLowerBNR_PDC.gain_l3.bits.PDC_GCF_L30, rSmoothBNR_PDC.gain_l3.bits.PDC_GCF_L30, rUpperBNR_PDC.gain_l3.bits.PDC_GCF_L30);
	MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_L21  = %d, %d, %d", rLowerBNR_PDC.gain_l3.bits.PDC_GCF_L21, rSmoothBNR_PDC.gain_l3.bits.PDC_GCF_L21, rUpperBNR_PDC.gain_l3.bits.PDC_GCF_L21);
	MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_L12  = %d, %d, %d", rLowerBNR_PDC.gain_l4.bits.PDC_GCF_L12, rSmoothBNR_PDC.gain_l4.bits.PDC_GCF_L12, rUpperBNR_PDC.gain_l4.bits.PDC_GCF_L12);
	MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_L03  = %d, %d, %d", rLowerBNR_PDC.gain_l4.bits.PDC_GCF_L03, rSmoothBNR_PDC.gain_l4.bits.PDC_GCF_L03, rUpperBNR_PDC.gain_l4.bits.PDC_GCF_L03);
	MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_R00  = %d, %d, %d", rLowerBNR_PDC.gain_r0.bits.PDC_GCF_R00, rSmoothBNR_PDC.gain_r0.bits.PDC_GCF_R00, rUpperBNR_PDC.gain_r0.bits.PDC_GCF_R00);
    MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_R10  = %d, %d, %d", rLowerBNR_PDC.gain_r0.bits.PDC_GCF_R10, rSmoothBNR_PDC.gain_r0.bits.PDC_GCF_R10, rUpperBNR_PDC.gain_r0.bits.PDC_GCF_R10);
	MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_R01  = %d, %d, %d", rLowerBNR_PDC.gain_r1.bits.PDC_GCF_R01, rSmoothBNR_PDC.gain_r1.bits.PDC_GCF_R01, rUpperBNR_PDC.gain_r1.bits.PDC_GCF_R01);
	MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_R20  = %d, %d, %d", rLowerBNR_PDC.gain_r1.bits.PDC_GCF_R20, rSmoothBNR_PDC.gain_r1.bits.PDC_GCF_R20, rUpperBNR_PDC.gain_r1.bits.PDC_GCF_R20);
	MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_R11  = %d, %d, %d", rLowerBNR_PDC.gain_r2.bits.PDC_GCF_R11, rSmoothBNR_PDC.gain_r2.bits.PDC_GCF_R11, rUpperBNR_PDC.gain_r2.bits.PDC_GCF_R11);
	MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_R02  = %d, %d, %d", rLowerBNR_PDC.gain_r2.bits.PDC_GCF_R02, rSmoothBNR_PDC.gain_r2.bits.PDC_GCF_R02, rUpperBNR_PDC.gain_r2.bits.PDC_GCF_R02);
	MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_R30  = %d, %d, %d", rLowerBNR_PDC.gain_r3.bits.PDC_GCF_R30, rSmoothBNR_PDC.gain_r3.bits.PDC_GCF_R30, rUpperBNR_PDC.gain_r3.bits.PDC_GCF_R30);
    MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_R21  = %d, %d, %d", rLowerBNR_PDC.gain_r3.bits.PDC_GCF_R21, rSmoothBNR_PDC.gain_r3.bits.PDC_GCF_R21, rUpperBNR_PDC.gain_r3.bits.PDC_GCF_R21);
	MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_R12  = %d, %d, %d", rLowerBNR_PDC.gain_r4.bits.PDC_GCF_R12, rSmoothBNR_PDC.gain_r4.bits.PDC_GCF_R12, rUpperBNR_PDC.gain_r4.bits.PDC_GCF_R12);
	MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_R03  = %d, %d, %d", rLowerBNR_PDC.gain_r4.bits.PDC_GCF_R03, rSmoothBNR_PDC.gain_r4.bits.PDC_GCF_R03, rUpperBNR_PDC.gain_r4.bits.PDC_GCF_R03);
	MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GTH      = %d, %d, %d", rLowerBNR_PDC.th_gb.bits.PDC_GTH, rSmoothBNR_PDC.th_gb.bits.PDC_GTH, rUpperBNR_PDC.th_gb.bits.PDC_GTH);
	MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_ITH      = %d, %d, %d", rLowerBNR_PDC.th_ia.bits.PDC_ITH, rSmoothBNR_PDC.th_ia.bits.PDC_ITH, rUpperBNR_PDC.th_ia.bits.PDC_ITH);
    MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_ATH      = %d, %d, %d", rLowerBNR_PDC.th_ia.bits.PDC_ATH, rSmoothBNR_PDC.th_ia.bits.PDC_ATH, rUpperBNR_PDC.th_ia.bits.PDC_ATH);
	MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_NTH      = %d, %d, %d", rLowerBNR_PDC.th_hd.bits.PDC_NTH, rSmoothBNR_PDC.th_hd.bits.PDC_NTH, rUpperBNR_PDC.th_hd.bits.PDC_NTH);
	MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_DTH      = %d, %d, %d", rLowerBNR_PDC.th_hd.bits.PDC_DTH, rSmoothBNR_PDC.th_hd.bits.PDC_DTH, rUpperBNR_PDC.th_hd.bits.PDC_DTH);
	MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GSL      = %d, %d, %d", rLowerBNR_PDC.sl.bits.PDC_GSL, rSmoothBNR_PDC.sl.bits.PDC_GSL, rUpperBNR_PDC.sl.bits.PDC_GSL);
	MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_BSL      = %d, %d, %d", rLowerBNR_PDC.sl.bits.PDC_BSL, rSmoothBNR_PDC.sl.bits.PDC_BSL, rUpperBNR_PDC.sl.bits.PDC_BSL);
	MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_ISL      = %d, %d, %d", rLowerBNR_PDC.sl.bits.PDC_ISL, rSmoothBNR_PDC.sl.bits.PDC_ISL, rUpperBNR_PDC.sl.bits.PDC_ISL);
	MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_ASL      = %d, %d, %d", rLowerBNR_PDC.sl.bits.PDC_ASL, rSmoothBNR_PDC.sl.bits.PDC_ASL, rUpperBNR_PDC.sl.bits.PDC_ASL);
	MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_NORM = %d, %d, %d", rLowerBNR_PDC.sl.bits.PDC_GCF_NORM, rSmoothBNR_PDC.sl.bits.PDC_GCF_NORM, rUpperBNR_PDC.sl.bits.PDC_GCF_NORM);
	MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_XCENTER  = %d, %d, %d", rLowerBNR_PDC.pos.bits.PDC_XCENTER, rSmoothBNR_PDC.pos.bits.PDC_XCENTER, rUpperBNR_PDC.pos.bits.PDC_XCENTER);
    MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_YCENTER  = %d, %d, %d", rLowerBNR_PDC.pos.bits.PDC_YCENTER, rSmoothBNR_PDC.pos.bits.PDC_YCENTER, rUpperBNR_PDC.pos.bits.PDC_YCENTER);
}

MINT32 InterParam_RMM(MINT32 ISO, MINT32 ISOpre, MINT32 ISOpos, MINT32 PARAMpre, MINT32 PARAMpos, MINT32 Method){
	MINT32 InterPARAM = 0;

	if(ISOpre == ISOpos){
		//	To prevent someone sets different PARAMpre and PARAMpos
		return (MINT32)((PARAMpre + PARAMpos + 1) / 2);
	}

	switch(Method){
	case 1:
		InterPARAM = (abs((double)(ISO - ISOpre)) > abs((double)(ISOpos - ISO))) ? PARAMpos : PARAMpre;
		break;
	case 0:
	default:
		double RATIOpre = (double)(ISOpos - ISO) / (double)(ISOpos - ISOpre);
		double RATIOpos = 1.0 - (double)(RATIOpre);
		double temp =  RATIOpre * PARAMpre + RATIOpos * PARAMpos;
		if(temp>0)
			InterPARAM = (MINT32)(RATIOpre * PARAMpre + RATIOpos * PARAMpos + 0.5);
		else
			InterPARAM = (MINT32)(RATIOpre * PARAMpre + RATIOpos * PARAMpos - 0.5);
		break;
	}

	return InterPARAM;
}
MVOID SmoothRMM(MUINT32 u4RealISO,  // Real ISO
                 MUINT32 u4UpperISO, // Upper ISO
                 MUINT32 u4LowerISO, // Lower ISO
                 ISP_NVRAM_RMM_T const& rUpperRMM, // RMM settings for upper ISO
                 ISP_NVRAM_RMM_T const& rLowerRMM,   // RMM settings for lower ISO
                 ISP_NVRAM_RMM_T& rSmoothRMM)   // Output
{
	char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.smooth_rmm.enable", value, "0");
    MBOOL bSmoothRMMDebug = atoi(value);

    MY_LOG_IF(bSmoothRMMDebug,"[%s()] u4RealISO: %d, u4UpperISO = %d, u4LowerISO = %d\n", __FUNCTION__, u4RealISO, u4UpperISO, u4LowerISO);

	//	====================================================================================================================================
	//	Get Lower ISO setting
	//	====================================================================================================================================
	// CAM_RMM_OSC CAM+0830H
	MINT32 RMM_OSC_TH_low        = rLowerRMM.osc.bits.RMM_OSC_TH;
	MINT32 RMM_SEDIR_SL_low      = rLowerRMM.osc.bits.RMM_SEDIR_SL;
	MINT32 RMM_SEBLD_WD_low      = rLowerRMM.osc.bits.RMM_SEBLD_WD;
	MINT32 RMM_LEBLD_WD_low      = rLowerRMM.osc.bits.RMM_LEBLD_WD;
	MINT32 RMM_EDGE_low          = rLowerRMM.osc.bits.RMM_EDGE;

	// CAM_RMM_MC CAM+0834H
	MINT32 RMM_MO_EDGE_low       = rLowerRMM.mc.bits.RMM_MO_EDGE;
	MINT32 RMM_MO_EN_low         = rLowerRMM.mc.bits.RMM_MO_EN;
	MINT32 RMM_MOBLD_FT_low      = rLowerRMM.mc.bits.RMM_MOBLD_FT;
	MINT32 RMM_MOTH_RATIO_low    = rLowerRMM.mc.bits.RMM_MOTH_RATIO;
	MINT32 RMM_HORI_ADDWT_low    = rLowerRMM.mc.bits.RMM_HORI_ADDWT ;

	// CAM_RMM_MC2 CAM+0844H
	MINT32 RMM_MOSE_TH_low       = rLowerRMM.mc2.bits.RMM_MOSE_TH;
	MINT32 RMM_MOSE_BLDWD_low    = rLowerRMM.mc2.bits.RMM_MOSE_BLDWD;

	// CAM_RMM_MA CAM+0844H
	MINT32 RMM_MASE_RATIO_low    = rLowerRMM.ma.bits.RMM_MASE_RATIO;
	MINT32 RMM_OSBLD_WD_low      = rLowerRMM.ma.bits.RMM_OSBLD_WD;
	MINT32 RMM_MASE_BLDWD_low    = rLowerRMM.ma.bits.RMM_MASE_BLDWD;
	MINT32 RMM_SENOS_LEFAC_low   = rLowerRMM.ma.bits.RMM_SENOS_LEFAC;
	MINT32 RMM_SEYOS_LEFAC_low   = rLowerRMM.ma.bits.RMM_SEYOS_LEFAC;

	// CAM_RMM_TUNE CAM+0844H
	MINT32 RMM_PSHOR_SEEN_low    = rLowerRMM.tune.bits.RMM_PSHOR_SEEN;
	MINT32 RMM_PS_BLUR_low       = rLowerRMM.tune.bits.RMM_PS_BLUR;
	MINT32 RMM_OSC_REPEN_low     = rLowerRMM.tune.bits.RMM_OSC_REPEN;
	MINT32 RMM_PSSEC_ONLY_low    = rLowerRMM.tune.bits.RMM_PSSEC_ONLY;
	MINT32 RMM_OSCLE_ONLY_low    = rLowerRMM.tune.bits.RMM_OSCLE_ONLY;
	MINT32 RMM_PS_TH_low         = rLowerRMM.tune.bits.RMM_PS_TH;
	MINT32 RMM_MOLE_DIREN_low    = rLowerRMM.tune.bits.RMM_MOLE_DIREN;
	MINT32 RMM_MOSE_DIREN_low    = rLowerRMM.tune.bits.RMM_MOSE_DIREN;

  // CAM_RMM_IDX virtual (not in HW)
	MINT32 RMM_OSC_IDX_low       = rLowerRMM.ridx.bits.RMM_OSC_IDX;
	MINT32 RMM_PS_IDX_low        = rLowerRMM.ridx.bits.RMM_PS_IDX;
	MINT32 RMM_MOSE_IDX_low      = rLowerRMM.ridx.bits.RMM_MOSE_IDX;


	//	====================================================================================================================================
	//	Get Upper ISO setting
	//	====================================================================================================================================
	// CAM_RMM_OSC CAM+0830H
	MINT32 RMM_OSC_TH_high       = rUpperRMM.osc.bits.RMM_OSC_TH;
	MINT32 RMM_SEDIR_SL_high     = rUpperRMM.osc.bits.RMM_SEDIR_SL;
	MINT32 RMM_SEBLD_WD_high     = rUpperRMM.osc.bits.RMM_SEBLD_WD;
	MINT32 RMM_LEBLD_WD_high     = rUpperRMM.osc.bits.RMM_LEBLD_WD;
	MINT32 RMM_EDGE_high         = rUpperRMM.osc.bits.RMM_EDGE;

	// CAM_RMM_MC CAM+0834H
	MINT32 RMM_MO_EDGE_high      = rUpperRMM.mc.bits.RMM_MO_EDGE;
	MINT32 RMM_MO_EN_high        = rUpperRMM.mc.bits.RMM_MO_EN;
	MINT32 RMM_MOBLD_FT_high     = rUpperRMM.mc.bits.RMM_MOBLD_FT;
	MINT32 RMM_MOTH_RATIO_high   = rUpperRMM.mc.bits.RMM_MOTH_RATIO;
	MINT32 RMM_HORI_ADDWT_high   = rUpperRMM.mc.bits.RMM_HORI_ADDWT ;

	// CAM_RMM_MC2 CAM+0844H
	MINT32 RMM_MOSE_TH_high      = rUpperRMM.mc2.bits.RMM_MOSE_TH;
	MINT32 RMM_MOSE_BLDWD_high   = rUpperRMM.mc2.bits.RMM_MOSE_BLDWD;

	// CAM_RMM_MA CAM+0844H
	MINT32 RMM_MASE_RATIO_high   = rUpperRMM.ma.bits.RMM_MASE_RATIO;
	MINT32 RMM_OSBLD_WD_high     = rUpperRMM.ma.bits.RMM_OSBLD_WD;
	MINT32 RMM_MASE_BLDWD_high   = rUpperRMM.ma.bits.RMM_MASE_BLDWD;
	MINT32 RMM_SENOS_LEFAC_high  = rUpperRMM.ma.bits.RMM_SENOS_LEFAC;
	MINT32 RMM_SEYOS_LEFAC_high  = rUpperRMM.ma.bits.RMM_SEYOS_LEFAC;

	// CAM_RMM_TUNE CAM+0844H
	MINT32 RMM_PSHOR_SEEN_high   = rUpperRMM.tune.bits.RMM_PSHOR_SEEN;
	MINT32 RMM_PS_BLUR_high      = rUpperRMM.tune.bits.RMM_PS_BLUR;
	MINT32 RMM_OSC_REPEN_high    = rUpperRMM.tune.bits.RMM_OSC_REPEN;
	MINT32 RMM_PSSEC_ONLY_high   = rUpperRMM.tune.bits.RMM_PSSEC_ONLY;
	MINT32 RMM_OSCLE_ONLY_high   = rUpperRMM.tune.bits.RMM_OSCLE_ONLY;
	MINT32 RMM_PS_TH_high        = rUpperRMM.tune.bits.RMM_PS_TH;
	MINT32 RMM_MOLE_DIREN_high   = rUpperRMM.tune.bits.RMM_MOLE_DIREN;
	MINT32 RMM_MOSE_DIREN_high   = rUpperRMM.tune.bits.RMM_MOSE_DIREN;

  // CAM_RMM_IDX virtual (not in HW)
	MINT32 RMM_OSC_IDX_high      = rUpperRMM.ridx.bits.RMM_OSC_IDX;
	MINT32 RMM_PS_IDX_high       = rUpperRMM.ridx.bits.RMM_PS_IDX;
	MINT32 RMM_MOSE_IDX_high     = rUpperRMM.ridx.bits.RMM_MOSE_IDX;

	//	====================================================================================================================================
	//	Interpolating reg declaration
	//	====================================================================================================================================
	// CAM_RMM_OSC CAM+0830H
	MINT32 RMM_OSC_TH;
	MINT32 RMM_SEDIR_SL;
	MINT32 RMM_SEBLD_WD;
	MINT32 RMM_LEBLD_WD;
	MINT32 RMM_EDGE;

	// CAM_RMM_MC CAM+0834H
	MINT32 RMM_MO_EDGE;
	MINT32 RMM_MO_EN;
	MINT32 RMM_MOBLD_FT;
	MINT32 RMM_MOTH_RATIO;
	MINT32 RMM_HORI_ADDWT;

	// CAM_RMM_MC2 CAM+0844H
	MINT32 RMM_MOSE_TH;
	MINT32 RMM_MOSE_BLDWD;

	// CAM_RMM_MA CAM+0844H
	MINT32 RMM_MASE_RATIO;
	MINT32 RMM_OSBLD_WD;
	MINT32 RMM_MASE_BLDWD;
	MINT32 RMM_SENOS_LEFAC;
	MINT32 RMM_SEYOS_LEFAC;

	// CAM_RMM_TUNE CAM+0844H
	MINT32 RMM_PSHOR_SEEN;
	MINT32 RMM_PS_BLUR;
	MINT32 RMM_OSC_REPEN;
	MINT32 RMM_PSSEC_ONLY;
	MINT32 RMM_OSCLE_ONLY;
	MINT32 RMM_PS_TH;
	MINT32 RMM_MOLE_DIREN;
	MINT32 RMM_MOSE_DIREN;

  // CAM_RMM_IDX virtual (not in HW)
	MINT32 RMM_OSC_IDX;
	MINT32 RMM_PS_IDX;
	MINT32 RMM_MOSE_IDX;

	//	====================================================================================================================================
	//	Start Parameter Interpolation
	//	====================================================================================================================================
	MINT32 ISO = u4RealISO;
	MINT32 ISO_low = u4LowerISO;
	MINT32 ISO_high = u4UpperISO;

	//Parameter can interpolate
	RMM_OSC_IDX        = InterParam_RMM(ISO, ISO_low, ISO_high, RMM_OSC_IDX_low    , RMM_OSC_IDX_high    , 0);
	RMM_PS_IDX         = InterParam_RMM(ISO, ISO_low, ISO_high, RMM_PS_IDX_low     , RMM_PS_IDX_high     , 0);
	RMM_MOSE_IDX       = InterParam_RMM(ISO, ISO_low, ISO_high, RMM_MOSE_IDX_low   , RMM_MOSE_IDX_high   , 0);
	RMM_OSC_TH         = InterParam_RMM(ISO, ISO_low, ISO_high, RMM_OSC_TH_low     , RMM_OSC_TH_high     , 0);
	RMM_SEDIR_SL       = InterParam_RMM(ISO, ISO_low, ISO_high, RMM_SEDIR_SL_low   , RMM_SEDIR_SL_high   , 0);
	RMM_SEBLD_WD       = InterParam_RMM(ISO, ISO_low, ISO_high, RMM_SEBLD_WD_low   , RMM_SEBLD_WD_high   , 0);
	RMM_LEBLD_WD       = InterParam_RMM(ISO, ISO_low, ISO_high, RMM_LEBLD_WD_low   , RMM_LEBLD_WD_high   , 0);
	RMM_MOBLD_FT       = InterParam_RMM(ISO, ISO_low, ISO_high, RMM_MOBLD_FT_low   , RMM_MOBLD_FT_high   , 0);
	RMM_MOTH_RATIO     = InterParam_RMM(ISO, ISO_low, ISO_high, RMM_MOTH_RATIO_low , RMM_MOTH_RATIO_high , 0);
	RMM_HORI_ADDWT     = InterParam_RMM(ISO, ISO_low, ISO_high, RMM_HORI_ADDWT_low , RMM_HORI_ADDWT_high  , 0);
	RMM_MOSE_TH        = InterParam_RMM(ISO, ISO_low, ISO_high, RMM_MOSE_TH_low    , RMM_MOSE_TH_high    , 0);
	RMM_MOSE_BLDWD     = InterParam_RMM(ISO, ISO_low, ISO_high, RMM_MOSE_BLDWD_low , RMM_MOSE_BLDWD_high , 0);

	RMM_MASE_RATIO     = InterParam_RMM(ISO, ISO_low, ISO_high, RMM_MASE_RATIO_low , RMM_MASE_RATIO_high , 0);
	RMM_OSBLD_WD       = InterParam_RMM(ISO, ISO_low, ISO_high, RMM_OSBLD_WD_low   , RMM_OSBLD_WD_high   , 0);
	RMM_MASE_BLDWD     = InterParam_RMM(ISO, ISO_low, ISO_high, RMM_MASE_BLDWD_low , RMM_MASE_BLDWD_high , 0);
	RMM_SENOS_LEFAC    = InterParam_RMM(ISO, ISO_low, ISO_high, RMM_SENOS_LEFAC_low, RMM_SENOS_LEFAC_high, 0);
	RMM_SEYOS_LEFAC    = InterParam_RMM(ISO, ISO_low, ISO_high, RMM_SEYOS_LEFAC_low, RMM_SEYOS_LEFAC_high, 0);
  RMM_PS_TH          = InterParam_RMM(ISO, ISO_low, ISO_high, RMM_OSC_IDX_low    , RMM_PS_TH_high      , 0);

  //Parameter cannot interpolate
	RMM_EDGE           = InterParam_RMM(ISO, ISO_low, ISO_high, RMM_EDGE_low       , RMM_EDGE_high       , 1);
	RMM_MO_EDGE        = InterParam_RMM(ISO, ISO_low, ISO_high, RMM_MO_EDGE_low    , RMM_MO_EDGE_high    , 1);
	RMM_MO_EN          = InterParam_RMM(ISO, ISO_low, ISO_high, RMM_MO_EN_low      , RMM_MO_EN_high      , 1);
	RMM_PSHOR_SEEN     = InterParam_RMM(ISO, ISO_low, ISO_high, RMM_PSHOR_SEEN_low , RMM_PSHOR_SEEN_high , 1);
	RMM_PS_BLUR        = InterParam_RMM(ISO, ISO_low, ISO_high, RMM_PS_BLUR_low    , RMM_PS_BLUR_high    , 1);
	RMM_OSC_REPEN      = InterParam_RMM(ISO, ISO_low, ISO_high, RMM_OSC_REPEN_low  , RMM_OSC_REPEN_high  , 1);
	RMM_PSSEC_ONLY     = InterParam_RMM(ISO, ISO_low, ISO_high, RMM_PSSEC_ONLY_low , RMM_PSSEC_ONLY_high , 1);
	RMM_OSCLE_ONLY     = InterParam_RMM(ISO, ISO_low, ISO_high, RMM_OSCLE_ONLY_low , RMM_OSCLE_ONLY_high , 1);
	RMM_MOSE_DIREN     = InterParam_RMM(ISO, ISO_low, ISO_high, RMM_MOSE_DIREN_low , RMM_MOSE_DIREN_high , 1);
	RMM_MOLE_DIREN     = InterParam_RMM(ISO, ISO_low, ISO_high, RMM_MOLE_DIREN_low , RMM_MOLE_DIREN_high , 1);

	//	====================================================================================================================================
	//	Set Smooth ISO setting
	//	====================================================================================================================================
	// CAM_RMM_OSC CAM+0830H
  rSmoothRMM.osc.bits.RMM_OSC_TH     = RMM_OSC_TH;
  rSmoothRMM.osc.bits.RMM_SEDIR_SL   = RMM_SEDIR_SL;
  rSmoothRMM.osc.bits.RMM_SEBLD_WD   = RMM_SEBLD_WD;
  rSmoothRMM.osc.bits.RMM_LEBLD_WD   = RMM_LEBLD_WD;
  rSmoothRMM.osc.bits.RMM_EDGE       = RMM_EDGE;

	// CAM_RMM_MC CAM+0834H
	rSmoothRMM.mc.bits.RMM_MO_EDGE     = RMM_MO_EDGE;
	rSmoothRMM.mc.bits.RMM_MO_EN       = RMM_MO_EN;
	rSmoothRMM.mc.bits.RMM_MOBLD_FT    = RMM_MOBLD_FT;
	rSmoothRMM.mc.bits.RMM_MOTH_RATIO  = RMM_MOTH_RATIO;
	rSmoothRMM.mc.bits.RMM_HORI_ADDWT  = RMM_HORI_ADDWT;

	// CAM_RMM_MC2 CAM+0844H
	rSmoothRMM.mc2.bits.RMM_MOSE_TH    = RMM_MOSE_TH;
	rSmoothRMM.mc2.bits.RMM_MOSE_BLDWD = RMM_MOSE_BLDWD;

	// CAM_RMM_MA CAM+0844H
	rSmoothRMM.ma.bits.RMM_MASE_RATIO  = RMM_MASE_RATIO;
	rSmoothRMM.ma.bits.RMM_OSBLD_WD    = RMM_OSBLD_WD;
	rSmoothRMM.ma.bits.RMM_MASE_BLDWD  = RMM_MASE_BLDWD;
	rSmoothRMM.ma.bits.RMM_SENOS_LEFAC = RMM_SENOS_LEFAC;
	rSmoothRMM.ma.bits.RMM_SEYOS_LEFAC = RMM_SEYOS_LEFAC;

	// CAM_RMM_TUNE CAM+0844H
	rSmoothRMM.tune.bits.RMM_PSHOR_SEEN = RMM_PSHOR_SEEN;
	rSmoothRMM.tune.bits.RMM_PS_BLUR    = RMM_PS_BLUR;
	rSmoothRMM.tune.bits.RMM_OSC_REPEN  = RMM_OSC_REPEN;
	rSmoothRMM.tune.bits.RMM_PSSEC_ONLY = RMM_PSSEC_ONLY;
	rSmoothRMM.tune.bits.RMM_OSCLE_ONLY = RMM_OSCLE_ONLY;
	rSmoothRMM.tune.bits.RMM_PS_TH      = RMM_PS_TH;
	rSmoothRMM.tune.bits.RMM_MOLE_DIREN = RMM_MOLE_DIREN;
	rSmoothRMM.tune.bits.RMM_MOSE_DIREN = RMM_MOSE_DIREN;

  // CAM_RMM_IDX virtual (not in HW)
	rSmoothRMM.ridx.bits.RMM_OSC_IDX    = RMM_OSC_IDX;
	rSmoothRMM.ridx.bits.RMM_PS_IDX     = RMM_PS_IDX;
	rSmoothRMM.ridx.bits.RMM_MOSE_IDX   = RMM_MOSE_IDX;

#define RMM_DUMP(item1, item2)   \
        do{                      \
            MY_LOG_IF(bSmoothRMMDebug, "[L,S,U]%22s = %d, %d, %d", #item2, rLowerRMM.item1.bits.item2, rSmoothRMM.item1.bits.item2, rUpperRMM.item1.bits.item2); \
        }while(0)

    RMM_DUMP(osc, RMM_OSC_TH);
    RMM_DUMP(osc, RMM_SEDIR_SL);
    RMM_DUMP(osc, RMM_SEBLD_WD);
    RMM_DUMP(osc, RMM_LEBLD_WD);
    RMM_DUMP(osc, RMM_EDGE);

    RMM_DUMP(mc, RMM_MO_EDGE);
    RMM_DUMP(mc, RMM_MO_EN);
    RMM_DUMP(mc, RMM_MOBLD_FT);
    RMM_DUMP(mc, RMM_MOTH_RATIO);
    RMM_DUMP(mc, RMM_HORI_ADDWT);

    RMM_DUMP(mc2, RMM_MOSE_TH);
    RMM_DUMP(mc2, RMM_MOSE_BLDWD);

    RMM_DUMP(ma, RMM_MASE_RATIO);
    RMM_DUMP(ma, RMM_OSBLD_WD);
    RMM_DUMP(ma, RMM_MASE_BLDWD);
    RMM_DUMP(ma, RMM_SENOS_LEFAC);
    RMM_DUMP(ma, RMM_SEYOS_LEFAC);

    RMM_DUMP(tune, RMM_PSHOR_SEEN);
    RMM_DUMP(tune, RMM_PS_BLUR);
    RMM_DUMP(tune, RMM_OSC_REPEN);
    RMM_DUMP(tune, RMM_PSSEC_ONLY);
    RMM_DUMP(tune, RMM_OSCLE_ONLY);
    RMM_DUMP(tune, RMM_PS_TH);
    RMM_DUMP(tune, RMM_MOLE_DIREN);
    RMM_DUMP(tune, RMM_MOSE_DIREN);

    RMM_DUMP(ridx, RMM_OSC_IDX);
    RMM_DUMP(ridx, RMM_PS_IDX);
    RMM_DUMP(ridx, RMM_MOSE_IDX);
#undef RMM_DUMP
}

MINT32 InterParam_RNR(MINT32 ISO, MINT32 ISOpre, MINT32 ISOpos, MINT32 PARAMpre, MINT32 PARAMpos, MINT32 Method) {
	MINT32 InterPARAM = 0;

	if(ISOpre == ISOpos) {
		//	To prevent someone sets different PARAMpre and PARAMpos
		return (MINT32)((PARAMpre + PARAMpos + 1) / 2);
	}

	switch(Method) {
	case 1:
		if(abs(ISO - ISOpre) > abs(ISOpos - ISO)) {
			InterPARAM = PARAMpos;
		}
		else{
			InterPARAM = PARAMpre;
		}
		break;
	case 0:
	default:
		double RATIOpre = (double)(ISOpos - ISO) / (double)(ISOpos - ISOpre);
		double RATIOpos = 1.0 - (double)(RATIOpre);
		InterPARAM = (MINT32)(RATIOpre * PARAMpre + RATIOpos * PARAMpos + 0.5);
	    break;
	}

	return InterPARAM;
}

MVOID SmoothRNR(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_RNR_T const& rUpperRNR, // RNR settings for upper ISO
                ISP_NVRAM_RNR_T const& rLowerRNR,   // RNR settings for lower ISO
                ISP_NVRAM_RNR_T& rSmoothRNR)   // Output
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.smooth_rnr.enable", value, "0");
    MBOOL bSmoothRNRDebug = atoi(value);

    MY_LOG_IF(bSmoothRNRDebug,"[%s()] u4RealISO: %d, u4UpperISO = %d, u4LowerISO = %d\n", __FUNCTION__, u4RealISO, u4UpperISO, u4LowerISO);

    MINT32 pre_RNR_EPS      = rLowerRNR.con1.bits.RNR_EPS;
    MINT32 pre_RNR_RAT      = rLowerRNR.con1.bits.RNR_RAT;
    MINT32 pre_RNR_VC0_B    = rLowerRNR.vc0.bits.RNR_VC0_B;
    MINT32 pre_RNR_VC0_G    = rLowerRNR.vc0.bits.RNR_VC0_G;
    MINT32 pre_RNR_VC0_R    = rLowerRNR.vc0.bits.RNR_VC0_R;
    MINT32 pre_RNR_VC1_B    = rLowerRNR.vc1.bits.RNR_VC1_B;
    MINT32 pre_RNR_VC1_G    = rLowerRNR.vc1.bits.RNR_VC1_G;
    MINT32 pre_RNR_VC1_R    = rLowerRNR.vc1.bits.RNR_VC1_R;
    MINT32 pre_RNR_RNG1     = rLowerRNR.rng.bits.RNR_RNG1;
    MINT32 pre_RNR_RNG2     = rLowerRNR.rng.bits.RNR_RNG2;
    MINT32 pre_RNR_RNG3     = rLowerRNR.rng.bits.RNR_RNG3;
    MINT32 pre_RNR_RNG4     = rLowerRNR.rng.bits.RNR_RNG4;
    MINT32 pre_RNR_CTHL     = rLowerRNR.con2.bits.RNR_CTHL;
    MINT32 pre_RNR_CTHR     = rLowerRNR.con2.bits.RNR_CTHR;
    MINT32 pre_RNR_RAD      = rLowerRNR.con2.bits.RNR_RAD;
    MINT32 pre_RNR_PRF_RAT  = rLowerRNR.con2.bits.RNR_PRF_RAT;
    MINT32 pre_RNR_MED_RAT  = rLowerRNR.con2.bits.RNR_MED_RAT;
    MINT32 pre_RNR_INT_OFST = rLowerRNR.con3.bits.RNR_INT_OFST;
    MINT32 pre_RNR_LLP_RAT  = rLowerRNR.con3.bits.RNR_LLP_RAT;
    MINT32 pre_RNR_GD_RAT   = rLowerRNR.con3.bits.RNR_GD_RAT;
    MINT32 pre_RNR_FL_MODE  = rLowerRNR.con3.bits.RNR_FL_MODE;
    MINT32 pre_RNR_SL_Y1    = rLowerRNR.sl.bits.RNR_SL_Y1;
    MINT32 pre_RNR_SL_Y2    = rLowerRNR.sl.bits.RNR_SL_Y2;
    MINT32 pre_RNR_SL_EN    = rLowerRNR.sl.bits.RNR_SL_EN;
    MINT32 pre_RNR_STH_C1   = rLowerRNR.ssl_sth.bits.RNR_STH_C1;
    MINT32 pre_RNR_SSL_C1   = rLowerRNR.ssl_sth.bits.RNR_SSL_C1;
    MINT32 pre_RNR_STH_C2   = rLowerRNR.ssl_sth.bits.RNR_STH_C2;
    MINT32 pre_RNR_SSL_C2   = rLowerRNR.ssl_sth.bits.RNR_SSL_C2;

    MINT32 pos_RNR_EPS      = rUpperRNR.con1.bits.RNR_EPS;
    MINT32 pos_RNR_RAT      = rUpperRNR.con1.bits.RNR_RAT;
    MINT32 pos_RNR_VC0_B    = rUpperRNR.vc0.bits.RNR_VC0_B;
    MINT32 pos_RNR_VC0_G    = rUpperRNR.vc0.bits.RNR_VC0_G;
    MINT32 pos_RNR_VC0_R    = rUpperRNR.vc0.bits.RNR_VC0_R;
    MINT32 pos_RNR_VC1_B    = rUpperRNR.vc1.bits.RNR_VC1_B;
    MINT32 pos_RNR_VC1_G    = rUpperRNR.vc1.bits.RNR_VC1_G;
    MINT32 pos_RNR_VC1_R    = rUpperRNR.vc1.bits.RNR_VC1_R;
    MINT32 pos_RNR_RNG1     = rUpperRNR.rng.bits.RNR_RNG1;
    MINT32 pos_RNR_RNG2     = rUpperRNR.rng.bits.RNR_RNG2;
    MINT32 pos_RNR_RNG3     = rUpperRNR.rng.bits.RNR_RNG3;
    MINT32 pos_RNR_RNG4     = rUpperRNR.rng.bits.RNR_RNG4;
    MINT32 pos_RNR_CTHL     = rUpperRNR.con2.bits.RNR_CTHL;
    MINT32 pos_RNR_CTHR     = rUpperRNR.con2.bits.RNR_CTHR;
    MINT32 pos_RNR_RAD      = rUpperRNR.con2.bits.RNR_RAD;
    MINT32 pos_RNR_PRF_RAT  = rUpperRNR.con2.bits.RNR_PRF_RAT;
    MINT32 pos_RNR_MED_RAT  = rUpperRNR.con2.bits.RNR_MED_RAT;
    MINT32 pos_RNR_INT_OFST = rUpperRNR.con3.bits.RNR_INT_OFST;
    MINT32 pos_RNR_LLP_RAT  = rUpperRNR.con3.bits.RNR_LLP_RAT;
    MINT32 pos_RNR_GD_RAT   = rUpperRNR.con3.bits.RNR_GD_RAT;
    MINT32 pos_RNR_FL_MODE  = rUpperRNR.con3.bits.RNR_FL_MODE;
    MINT32 pos_RNR_SL_Y1    = rUpperRNR.sl.bits.RNR_SL_Y1;
    MINT32 pos_RNR_SL_Y2    = rUpperRNR.sl.bits.RNR_SL_Y2;
    MINT32 pos_RNR_SL_EN    = rUpperRNR.sl.bits.RNR_SL_EN;
    MINT32 pos_RNR_STH_C1   = rUpperRNR.ssl_sth.bits.RNR_STH_C1;
    MINT32 pos_RNR_SSL_C1   = rUpperRNR.ssl_sth.bits.RNR_SSL_C1;
    MINT32 pos_RNR_STH_C2   = rUpperRNR.ssl_sth.bits.RNR_STH_C2;
    MINT32 pos_RNR_SSL_C2   = rUpperRNR.ssl_sth.bits.RNR_SSL_C2;

    // interpolation //
    MINT32 ISO = u4RealISO;
    MINT32 ISOpre = u4LowerISO;
    MINT32 ISOpos = u4UpperISO;

    MINT32 RNR_EPS        = InterParam_DBS(ISO, ISOpre, ISOpos, pre_RNR_EPS      , pos_RNR_EPS       , 0);
    MINT32 RNR_RAT        = InterParam_RNR(ISO, ISOpre, ISOpos, pre_RNR_RAT      , pos_RNR_RAT       , 0);
    MINT32 RNR_VC0_B      = InterParam_RNR(ISO, ISOpre, ISOpos, pre_RNR_VC0_B    , pos_RNR_VC0_B     , 0);
    MINT32 RNR_VC0_G      = InterParam_RNR(ISO, ISOpre, ISOpos, pre_RNR_VC0_G    , pos_RNR_VC0_G     , 0);
    MINT32 RNR_VC0_R      = InterParam_RNR(ISO, ISOpre, ISOpos, pre_RNR_VC0_R    , pos_RNR_VC0_R     , 0);
    MINT32 RNR_VC1_B      = InterParam_RNR(ISO, ISOpre, ISOpos, pre_RNR_VC1_B    , pos_RNR_VC1_B     , 0);
    MINT32 RNR_VC1_G      = InterParam_RNR(ISO, ISOpre, ISOpos, pre_RNR_VC1_G    , pos_RNR_VC1_G     , 0);
    MINT32 RNR_VC1_R      = InterParam_RNR(ISO, ISOpre, ISOpos, pre_RNR_VC1_R    , pos_RNR_VC1_R     , 0);
    MINT32 RNR_RNG1       = InterParam_RNR(ISO, ISOpre, ISOpos, pre_RNR_RNG1     , pos_RNR_RNG1      , 0);
    MINT32 RNR_RNG2       = InterParam_RNR(ISO, ISOpre, ISOpos, pre_RNR_RNG2     , pos_RNR_RNG2      , 0);
    MINT32 RNR_RNG3       = InterParam_RNR(ISO, ISOpre, ISOpos, pre_RNR_RNG3     , pos_RNR_RNG3      , 0);
    MINT32 RNR_RNG4       = InterParam_RNR(ISO, ISOpre, ISOpos, pre_RNR_RNG4     , pos_RNR_RNG4      , 0);
    MINT32 RNR_CTHL       = InterParam_RNR(ISO, ISOpre, ISOpos, pre_RNR_CTHL     , pos_RNR_CTHL      , 0);
    MINT32 RNR_CTHR       = InterParam_RNR(ISO, ISOpre, ISOpos, pre_RNR_CTHR     , pos_RNR_CTHR      , 0);
    MINT32 RNR_RAD        = InterParam_RNR(ISO, ISOpre, ISOpos, pre_RNR_RAD      , pos_RNR_RAD       , 0);
    MINT32 RNR_PRF_RAT    = InterParam_RNR(ISO, ISOpre, ISOpos, pre_RNR_PRF_RAT  , pos_RNR_PRF_RAT   , 0);
    MINT32 RNR_MED_RAT    = InterParam_RNR(ISO, ISOpre, ISOpos, pre_RNR_MED_RAT  , pos_RNR_MED_RAT   , 0);
    MINT32 RNR_INT_OFST   = InterParam_RNR(ISO, ISOpre, ISOpos, pre_RNR_INT_OFST , pos_RNR_INT_OFST  , 0);
    MINT32 RNR_LLP_RAT    = InterParam_RNR(ISO, ISOpre, ISOpos, pre_RNR_LLP_RAT  , pos_RNR_LLP_RAT   , 0);
    MINT32 RNR_GD_RAT     = InterParam_RNR(ISO, ISOpre, ISOpos, pre_RNR_GD_RAT   , pos_RNR_GD_RAT    , 0);
    MINT32 RNR_FL_MODE    = InterParam_RNR(ISO, ISOpre, ISOpos, pre_RNR_FL_MODE  , pos_RNR_FL_MODE   , 0);
    MINT32 RNR_SL_Y1      = InterParam_RNR(ISO, ISOpre, ISOpos, pre_RNR_SL_Y1    , pos_RNR_SL_Y1     , 0);
    MINT32 RNR_SL_Y2      = InterParam_RNR(ISO, ISOpre, ISOpos, pre_RNR_SL_Y2    , pos_RNR_SL_Y2     , 0);
    MINT32 RNR_SL_EN      = InterParam_RNR(ISO, ISOpre, ISOpos, pre_RNR_SL_EN    , pos_RNR_SL_EN     , 0);
    MINT32 RNR_STH_C1     = InterParam_RNR(ISO, ISOpre, ISOpos, pre_RNR_STH_C1   , pos_RNR_STH_C1    , 0);
    MINT32 RNR_SSL_C1     = InterParam_RNR(ISO, ISOpre, ISOpos, pre_RNR_SSL_C1   , pos_RNR_SSL_C1    , 0);
    MINT32 RNR_STH_C2     = InterParam_RNR(ISO, ISOpre, ISOpos, pre_RNR_STH_C2   , pos_RNR_STH_C2    , 0);
    MINT32 RNR_SSL_C2     = InterParam_RNR(ISO, ISOpre, ISOpos, pre_RNR_SSL_C2   , pos_RNR_SSL_C2    , 0);

    rSmoothRNR.con1.bits.RNR_EPS       = RNR_EPS;
    rSmoothRNR.con1.bits.RNR_RAT       = RNR_RAT;
    rSmoothRNR.vc0.bits.RNR_VC0_B      = RNR_VC0_B;
    rSmoothRNR.vc0.bits.RNR_VC0_G      = RNR_VC0_G;
    rSmoothRNR.vc0.bits.RNR_VC0_R      = RNR_VC0_R;
    rSmoothRNR.vc1.bits.RNR_VC1_B      = RNR_VC1_B;
    rSmoothRNR.vc1.bits.RNR_VC1_G      = RNR_VC1_G;
    rSmoothRNR.vc1.bits.RNR_VC1_R      = RNR_VC1_R;
    rSmoothRNR.rng.bits.RNR_RNG1       = RNR_RNG1;
    rSmoothRNR.rng.bits.RNR_RNG2       = RNR_RNG2;
    rSmoothRNR.rng.bits.RNR_RNG3       = RNR_RNG3;
    rSmoothRNR.rng.bits.RNR_RNG4       = RNR_RNG4;
    rSmoothRNR.con2.bits.RNR_CTHL      = RNR_CTHL;
    rSmoothRNR.con2.bits.RNR_CTHR      = RNR_CTHR;
    rSmoothRNR.con2.bits.RNR_RAD       = RNR_RAD;
    rSmoothRNR.con2.bits.RNR_PRF_RAT   = RNR_PRF_RAT;
    rSmoothRNR.con2.bits.RNR_MED_RAT   = RNR_MED_RAT;
    rSmoothRNR.con3.bits.RNR_INT_OFST  = RNR_INT_OFST;
    rSmoothRNR.con3.bits.RNR_LLP_RAT   = RNR_LLP_RAT;
    rSmoothRNR.con3.bits.RNR_GD_RAT    = RNR_GD_RAT;
    rSmoothRNR.con3.bits.RNR_FL_MODE   = RNR_FL_MODE;
    rSmoothRNR.sl.bits.RNR_SL_Y1       = RNR_SL_Y1;
    rSmoothRNR.sl.bits.RNR_SL_Y2       = RNR_SL_Y2;
    rSmoothRNR.sl.bits.RNR_SL_EN       = RNR_SL_EN;
    rSmoothRNR.ssl_sth.bits.RNR_STH_C1 = RNR_STH_C1;
    rSmoothRNR.ssl_sth.bits.RNR_SSL_C1 = RNR_SSL_C1;
    rSmoothRNR.ssl_sth.bits.RNR_STH_C2 = RNR_STH_C2;
    rSmoothRNR.ssl_sth.bits.RNR_SSL_C2 = RNR_SSL_C2;

    MY_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_EPS      = %d, %d, %d", rLowerRNR.con1.bits.RNR_EPS, rSmoothRNR.con1.bits.RNR_EPS, rUpperRNR.con1.bits.RNR_EPS);
    MY_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_RAT      = %d, %d, %d", rLowerRNR.con1.bits.RNR_RAT, rSmoothRNR.con1.bits.RNR_RAT, rUpperRNR.con1.bits.RNR_RAT);
    MY_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_VC0_B    = %d, %d, %d", rLowerRNR.vc0.bits.RNR_VC0_B, rSmoothRNR.vc0.bits.RNR_VC0_B, rUpperRNR.vc0.bits.RNR_VC0_B);
    MY_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_VC0_G    = %d, %d, %d", rLowerRNR.vc0.bits.RNR_VC0_G, rSmoothRNR.vc0.bits.RNR_VC0_G, rUpperRNR.vc0.bits.RNR_VC0_G);
    MY_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_VC0_R    = %d, %d, %d", rLowerRNR.vc0.bits.RNR_VC0_R, rSmoothRNR.vc0.bits.RNR_VC0_R, rUpperRNR.vc0.bits.RNR_VC0_R);
    MY_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_VC1_B    = %d, %d, %d", rLowerRNR.vc1.bits.RNR_VC1_B, rSmoothRNR.vc1.bits.RNR_VC1_B, rUpperRNR.vc1.bits.RNR_VC1_B);
    MY_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_VC1_G    = %d, %d, %d", rLowerRNR.vc1.bits.RNR_VC1_G, rSmoothRNR.vc1.bits.RNR_VC1_G, rUpperRNR.vc1.bits.RNR_VC1_G);
    MY_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_VC1_R    = %d, %d, %d", rLowerRNR.vc1.bits.RNR_VC1_R, rSmoothRNR.vc1.bits.RNR_VC1_R, rUpperRNR.vc1.bits.RNR_VC1_R);
    MY_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_RNG1     = %d, %d, %d", rLowerRNR.rng.bits.RNR_RNG1, rSmoothRNR.rng.bits.RNR_RNG1, rUpperRNR.rng.bits.RNR_RNG1);
    MY_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_RNG2     = %d, %d, %d", rLowerRNR.rng.bits.RNR_RNG2, rSmoothRNR.rng.bits.RNR_RNG2, rUpperRNR.rng.bits.RNR_RNG2);
    MY_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_RNG3     = %d, %d, %d", rLowerRNR.rng.bits.RNR_RNG3, rSmoothRNR.rng.bits.RNR_RNG3, rUpperRNR.rng.bits.RNR_RNG3);
    MY_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_RNG4     = %d, %d, %d", rLowerRNR.rng.bits.RNR_RNG4, rSmoothRNR.rng.bits.RNR_RNG4, rUpperRNR.rng.bits.RNR_RNG4);
    MY_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_CTHL     = %d, %d, %d", rLowerRNR.con2.bits.RNR_CTHL, rSmoothRNR.con2.bits.RNR_CTHL, rUpperRNR.con2.bits.RNR_CTHL);
    MY_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_CTHR     = %d, %d, %d", rLowerRNR.con2.bits.RNR_CTHR, rSmoothRNR.con2.bits.RNR_CTHR, rUpperRNR.con2.bits.RNR_CTHR);
    MY_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_RAD      = %d, %d, %d", rLowerRNR.con2.bits.RNR_RAD, rSmoothRNR.con2.bits.RNR_RAD, rUpperRNR.con2.bits.RNR_RAD);
    MY_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_PRF_RAT  = %d, %d, %d", rLowerRNR.con2.bits.RNR_PRF_RAT, rSmoothRNR.con2.bits.RNR_PRF_RAT, rUpperRNR.con2.bits.RNR_PRF_RAT);
    MY_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_MED_RAT  = %d, %d, %d", rLowerRNR.con2.bits.RNR_MED_RAT, rSmoothRNR.con2.bits.RNR_MED_RAT, rUpperRNR.con2.bits.RNR_MED_RAT);
    MY_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_INT_OFST = %d, %d, %d", rLowerRNR.con3.bits.RNR_INT_OFST, rSmoothRNR.con3.bits.RNR_INT_OFST, rUpperRNR.con3.bits.RNR_INT_OFST);
    MY_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_LLP_RAT  = %d, %d, %d", rLowerRNR.con3.bits.RNR_LLP_RAT, rSmoothRNR.con3.bits.RNR_LLP_RAT, rUpperRNR.con3.bits.RNR_LLP_RAT);
    MY_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_GD_RAT   = %d, %d, %d", rLowerRNR.con3.bits.RNR_GD_RAT, rSmoothRNR.con3.bits.RNR_GD_RAT, rUpperRNR.con3.bits.RNR_GD_RAT);
    MY_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_FL_MODE  = %d, %d, %d", rLowerRNR.con3.bits.RNR_FL_MODE, rSmoothRNR.con3.bits.RNR_FL_MODE, rUpperRNR.con3.bits.RNR_FL_MODE);
    MY_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_SL_Y1    = %d, %d, %d", rLowerRNR.sl.bits.RNR_SL_Y1, rSmoothRNR.sl.bits.RNR_SL_Y1, rUpperRNR.sl.bits.RNR_SL_Y1);
    MY_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_SL_Y2    = %d, %d, %d", rLowerRNR.sl.bits.RNR_SL_Y2, rSmoothRNR.sl.bits.RNR_SL_Y2, rUpperRNR.sl.bits.RNR_SL_Y2);
    MY_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_SL_EN    = %d, %d, %d", rLowerRNR.sl.bits.RNR_SL_EN, rSmoothRNR.sl.bits.RNR_SL_EN, rUpperRNR.sl.bits.RNR_SL_EN);
    MY_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_STH_C1   = %d, %d, %d", rLowerRNR.ssl_sth.bits.RNR_STH_C1, rSmoothRNR.ssl_sth.bits.RNR_STH_C1, rUpperRNR.ssl_sth.bits.RNR_STH_C1);
    MY_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_SSL_C1   = %d, %d, %d", rLowerRNR.ssl_sth.bits.RNR_SSL_C1, rSmoothRNR.ssl_sth.bits.RNR_SSL_C1, rUpperRNR.ssl_sth.bits.RNR_SSL_C1);
    MY_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_STH_C2   = %d, %d, %d", rLowerRNR.ssl_sth.bits.RNR_STH_C2, rSmoothRNR.ssl_sth.bits.RNR_STH_C2, rUpperRNR.ssl_sth.bits.RNR_STH_C2);
    MY_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_SSL_C2   = %d, %d, %d", rLowerRNR.ssl_sth.bits.RNR_SSL_C2, rSmoothRNR.ssl_sth.bits.RNR_SSL_C2, rUpperRNR.ssl_sth.bits.RNR_SSL_C2);
}

MINT32 InterParam_UDM(MINT32 ISO, MINT32 ISOpre, MINT32 ISOpos, MINT32 PARAMpre, MINT32 PARAMpos, MINT32 Method) {
    MINT32 InterPARAM = 0;

    if(ISOpre == ISOpos) {
        //  To prevent someone sets different PARAMpre and PARAMpos
        return (MINT32)((PARAMpre + PARAMpos + 1) / 2);
    }

    switch(Method) {
    case 1:
        if(abs(ISO - ISOpre) > abs(ISOpos - ISO)) {
            InterPARAM = PARAMpos;
        }
        else{
            InterPARAM = PARAMpre;
        }
        break;
    case 0:
    default:
        double RATIOpre = (double)(ISOpos - ISO) / (double)(ISOpos - ISOpre);
        double RATIOpos = 1.0 - (double)(RATIOpre);
        InterPARAM = (MINT32)(RATIOpre * PARAMpre + RATIOpos * PARAMpos + 0.5);
        break;
    }

    return InterPARAM;
}

MVOID SmoothUDM(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_UDM_T const& rUpperUDM, // UDM settings for upper ISO
                ISP_NVRAM_UDM_T const& rLowerUDM,   // UDM settings for lower ISO
                ISP_NVRAM_UDM_T& rSmoothUDM)   // Output
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.smooth_udm.enable", value, "0");
    MBOOL bSmoothUDMDebug = atoi(value);

    MY_LOG_IF(bSmoothUDMDebug,"[%s()] u4RealISO: %d, u4UpperISO = %d, u4LowerISO = %d\n", __FUNCTION__, u4RealISO, u4UpperISO, u4LowerISO);

    MINT32 pre_UDM_BYP      = rLowerUDM.intp_crs.bits.UDM_BYP;
    MINT32 pre_UDM_GRAD_WT  = rLowerUDM.intp_crs.bits.UDM_GRAD_WT;
    MINT32 pre_UDM_ETH      = rLowerUDM.intp_crs.bits.UDM_ETH;
    MINT32 pre_UDM_FTH      = rLowerUDM.intp_crs.bits.UDM_FTH;
    MINT32 pre_UDM_DTH      = rLowerUDM.intp_crs.bits.UDM_DTH;
    MINT32 pre_UDM_CD_SLC   = rLowerUDM.intp_nat.bits.UDM_CD_SLC;
    MINT32 pre_UDM_CD_SLL   = rLowerUDM.intp_nat.bits.UDM_CD_SLL;
    MINT32 pre_UDM_L0_OFST  = rLowerUDM.intp_nat.bits.UDM_L0_OFST;
    MINT32 pre_UDM_L0_SL    = rLowerUDM.intp_nat.bits.UDM_L0_SL;
    MINT32 pre_UDM_L1_OFST  = rLowerUDM.intp_aug.bits.UDM_L1_OFST;
    MINT32 pre_UDM_L1_SL    = rLowerUDM.intp_aug.bits.UDM_L1_SL;
    MINT32 pre_UDM_L2_OFST  = rLowerUDM.intp_aug.bits.UDM_L2_OFST;
    MINT32 pre_UDM_L2_SL    = rLowerUDM.intp_aug.bits.UDM_L2_SL;
    MINT32 pre_UDM_DN_OFST  = rLowerUDM.intp_aug.bits.UDM_DN_OFST;
    MINT32 pre_UDM_HL_OFST  = rLowerUDM.intp_nat.bits.UDM_HL_OFST;
    MINT32 pre_UDM_LM_Y0    = rLowerUDM.luma_lut1.bits.UDM_LM_Y0;
    MINT32 pre_UDM_LM_Y1    = rLowerUDM.luma_lut1.bits.UDM_LM_Y1;
    MINT32 pre_UDM_LM_Y2    = rLowerUDM.luma_lut1.bits.UDM_LM_Y2;
    MINT32 pre_UDM_LM_Y3    = rLowerUDM.luma_lut2.bits.UDM_LM_Y3;
    MINT32 pre_UDM_LM_Y4    = rLowerUDM.luma_lut2.bits.UDM_LM_Y4;
    MINT32 pre_UDM_LM_Y5    = rLowerUDM.luma_lut2.bits.UDM_LM_Y5;
    MINT32 pre_UDM_SL_Y1    = rLowerUDM.sl_ctl.bits.UDM_SL_Y1;
    MINT32 pre_UDM_SL_Y2    = rLowerUDM.sl_ctl.bits.UDM_SL_Y2;
    MINT32 pre_UDM_SL_HR    = rLowerUDM.sl_ctl.bits.UDM_SL_HR;
    MINT32 pre_UDM_SL_EN    = rLowerUDM.sl_ctl.bits.UDM_SL_EN;
    MINT32 pre_UDM_HT_GN1   = rLowerUDM.hftd_ctl.bits.UDM_HT_GN1;
    MINT32 pre_UDM_HT_GN2   = rLowerUDM.hftd_ctl.bits.UDM_HT_GN2;
    MINT32 pre_UDM_HD_GN1   = rLowerUDM.hftd_ctl.bits.UDM_HD_GN1;
    MINT32 pre_UDM_HD_GN2   = rLowerUDM.hftd_ctl.bits.UDM_HD_GN2;
    MINT32 pre_UDM_XTK_RAT  = rLowerUDM.nr_str.bits.UDM_XTK_RAT;
    MINT32 pre_UDM_XTK_OFST = rLowerUDM.nr_str.bits.UDM_XTK_OFST;
    MINT32 pre_UDM_XTK_SL   = rLowerUDM.nr_str.bits.UDM_XTK_SL;
    MINT32 pre_UDM_N0_STR   = rLowerUDM.nr_str.bits.UDM_N0_STR;
    MINT32 pre_UDM_N1_STR   = rLowerUDM.nr_str.bits.UDM_N1_STR;
    MINT32 pre_UDM_N2_STR   = rLowerUDM.nr_str.bits.UDM_N2_STR;
    MINT32 pre_UDM_N0_OFST  = rLowerUDM.nr_act.bits.UDM_N0_OFST;
    MINT32 pre_UDM_N1_OFST  = rLowerUDM.nr_act.bits.UDM_N1_OFST;
    MINT32 pre_UDM_N2_OFST  = rLowerUDM.nr_act.bits.UDM_N2_OFST;
    MINT32 pre_UDM_NSL      = rLowerUDM.nr_act.bits.UDM_NSL;
    MINT32 pre_UDM_NGR      = rLowerUDM.nr_act.bits.UDM_NGR;
    MINT32 pre_UDM_HA_STR   = rLowerUDM.hf_str.bits.UDM_HA_STR;
    MINT32 pre_UDM_H1_GN    = rLowerUDM.hf_str.bits.UDM_H1_GN;
    MINT32 pre_UDM_H2_GN    = rLowerUDM.hf_str.bits.UDM_H2_GN;
    MINT32 pre_UDM_H3_GN    = rLowerUDM.hf_str.bits.UDM_H3_GN;
    MINT32 pre_UDM_HI_RAT   = rLowerUDM.hf_str.bits.UDM_HI_RAT;
    MINT32 pre_UDM_H1_LWB   = rLowerUDM.hf_act1.bits.UDM_H1_LWB;
    MINT32 pre_UDM_H1_UPB   = rLowerUDM.hf_act1.bits.UDM_H1_UPB;
    MINT32 pre_UDM_H2_LWB   = rLowerUDM.hf_act1.bits.UDM_H2_LWB;
    MINT32 pre_UDM_H2_UPB   = rLowerUDM.hf_act1.bits.UDM_H2_UPB;
    MINT32 pre_UDM_H3_LWB   = rLowerUDM.hf_act2.bits.UDM_H3_LWB;
    MINT32 pre_UDM_H3_UPB   = rLowerUDM.hf_act2.bits.UDM_H3_UPB;
    MINT32 pre_UDM_HSLL     = rLowerUDM.hf_act2.bits.UDM_HSLL;
    MINT32 pre_UDM_HSLR     = rLowerUDM.hf_act2.bits.UDM_HSLR;
    MINT32 pre_UDM_CORE_TH1 = rLowerUDM.hftd_ctl.bits.UDM_CORE_TH1;
    MINT32 pre_UDM_CORE_TH2 = rLowerUDM.hf_str.bits.UDM_CORE_TH2;
    MINT32 pre_UDM_OV_TH    = rLowerUDM.clip.bits.UDM_OV_TH;
    MINT32 pre_UDM_UN_TH    = rLowerUDM.clip.bits.UDM_UN_TH;
    MINT32 pre_UDM_CLIP_TH  = rLowerUDM.clip.bits.UDM_CLIP_TH;
    MINT32 pre_UDM_DS_THB   = rLowerUDM.dsb.bits.UDM_DS_THB;
    MINT32 pre_UDM_DS_SLB   = rLowerUDM.dsb.bits.UDM_DS_SLB;
    MINT32 pre_UDM_FL_MODE  = rLowerUDM.dsb.bits.UDM_FL_MODE;
    MINT32 pre_UDM_SL_RAT   = rLowerUDM.dsb.bits.UDM_SL_RAT;
    MINT32 pre_UDM_SC_RAT   = rLowerUDM.dsb.bits.UDM_SC_RAT;
    MINT32 pre_UDM_LR_RAT   = rLowerUDM.lr_rat.bits.UDM_LR_RAT;

    MINT32 pos_UDM_BYP      = rUpperUDM.intp_crs.bits.UDM_BYP;
    MINT32 pos_UDM_GRAD_WT  = rUpperUDM.intp_crs.bits.UDM_GRAD_WT;
    MINT32 pos_UDM_ETH      = rUpperUDM.intp_crs.bits.UDM_ETH;
    MINT32 pos_UDM_FTH      = rUpperUDM.intp_crs.bits.UDM_FTH;
    MINT32 pos_UDM_DTH      = rUpperUDM.intp_crs.bits.UDM_DTH;
    MINT32 pos_UDM_CD_SLC   = rUpperUDM.intp_nat.bits.UDM_CD_SLC;
    MINT32 pos_UDM_CD_SLL   = rUpperUDM.intp_nat.bits.UDM_CD_SLL;
    MINT32 pos_UDM_L0_OFST  = rUpperUDM.intp_nat.bits.UDM_L0_OFST;
    MINT32 pos_UDM_L0_SL    = rUpperUDM.intp_nat.bits.UDM_L0_SL;
    MINT32 pos_UDM_L1_OFST  = rUpperUDM.intp_aug.bits.UDM_L1_OFST;
    MINT32 pos_UDM_L1_SL    = rUpperUDM.intp_aug.bits.UDM_L1_SL;
    MINT32 pos_UDM_L2_OFST  = rUpperUDM.intp_aug.bits.UDM_L2_OFST;
    MINT32 pos_UDM_L2_SL    = rUpperUDM.intp_aug.bits.UDM_L2_SL;
    MINT32 pos_UDM_DN_OFST  = rUpperUDM.intp_aug.bits.UDM_DN_OFST;
    MINT32 pos_UDM_HL_OFST  = rUpperUDM.intp_nat.bits.UDM_HL_OFST;
    MINT32 pos_UDM_LM_Y0    = rUpperUDM.luma_lut1.bits.UDM_LM_Y0;
    MINT32 pos_UDM_LM_Y1    = rUpperUDM.luma_lut1.bits.UDM_LM_Y1;
    MINT32 pos_UDM_LM_Y2    = rUpperUDM.luma_lut1.bits.UDM_LM_Y2;
    MINT32 pos_UDM_LM_Y3    = rUpperUDM.luma_lut2.bits.UDM_LM_Y3;
    MINT32 pos_UDM_LM_Y4    = rUpperUDM.luma_lut2.bits.UDM_LM_Y4;
    MINT32 pos_UDM_LM_Y5    = rUpperUDM.luma_lut2.bits.UDM_LM_Y5;
    MINT32 pos_UDM_SL_Y1    = rUpperUDM.sl_ctl.bits.UDM_SL_Y1;
    MINT32 pos_UDM_SL_Y2    = rUpperUDM.sl_ctl.bits.UDM_SL_Y2;
    MINT32 pos_UDM_SL_HR    = rUpperUDM.sl_ctl.bits.UDM_SL_HR;
    MINT32 pos_UDM_SL_EN    = rUpperUDM.sl_ctl.bits.UDM_SL_EN;
    MINT32 pos_UDM_HT_GN1   = rUpperUDM.hftd_ctl.bits.UDM_HT_GN1;
    MINT32 pos_UDM_HT_GN2   = rUpperUDM.hftd_ctl.bits.UDM_HT_GN2;
    MINT32 pos_UDM_HD_GN1   = rUpperUDM.hftd_ctl.bits.UDM_HD_GN1;
    MINT32 pos_UDM_HD_GN2   = rUpperUDM.hftd_ctl.bits.UDM_HD_GN2;
    MINT32 pos_UDM_XTK_RAT  = rUpperUDM.nr_str.bits.UDM_XTK_RAT;
    MINT32 pos_UDM_XTK_OFST = rUpperUDM.nr_str.bits.UDM_XTK_OFST;
    MINT32 pos_UDM_XTK_SL   = rUpperUDM.nr_str.bits.UDM_XTK_SL;
    MINT32 pos_UDM_N0_STR   = rUpperUDM.nr_str.bits.UDM_N0_STR;
    MINT32 pos_UDM_N1_STR   = rUpperUDM.nr_str.bits.UDM_N1_STR;
    MINT32 pos_UDM_N2_STR   = rUpperUDM.nr_str.bits.UDM_N2_STR;
    MINT32 pos_UDM_N0_OFST  = rUpperUDM.nr_act.bits.UDM_N0_OFST;
    MINT32 pos_UDM_N1_OFST  = rUpperUDM.nr_act.bits.UDM_N1_OFST;
    MINT32 pos_UDM_N2_OFST  = rUpperUDM.nr_act.bits.UDM_N2_OFST;
    MINT32 pos_UDM_NSL      = rUpperUDM.nr_act.bits.UDM_NSL;
    MINT32 pos_UDM_NGR      = rUpperUDM.nr_act.bits.UDM_NGR;
    MINT32 pos_UDM_HA_STR   = rUpperUDM.hf_str.bits.UDM_HA_STR;
    MINT32 pos_UDM_H1_GN    = rUpperUDM.hf_str.bits.UDM_H1_GN;
    MINT32 pos_UDM_H2_GN    = rUpperUDM.hf_str.bits.UDM_H2_GN;
    MINT32 pos_UDM_H3_GN    = rUpperUDM.hf_str.bits.UDM_H3_GN;
    MINT32 pos_UDM_HI_RAT   = rUpperUDM.hf_str.bits.UDM_HI_RAT;
    MINT32 pos_UDM_H1_LWB   = rUpperUDM.hf_act1.bits.UDM_H1_LWB;
    MINT32 pos_UDM_H1_UPB   = rUpperUDM.hf_act1.bits.UDM_H1_UPB;
    MINT32 pos_UDM_H2_LWB   = rUpperUDM.hf_act1.bits.UDM_H2_LWB;
    MINT32 pos_UDM_H2_UPB   = rUpperUDM.hf_act1.bits.UDM_H2_UPB;
    MINT32 pos_UDM_H3_LWB   = rUpperUDM.hf_act2.bits.UDM_H3_LWB;
    MINT32 pos_UDM_H3_UPB   = rUpperUDM.hf_act2.bits.UDM_H3_UPB;
    MINT32 pos_UDM_HSLL     = rUpperUDM.hf_act2.bits.UDM_HSLL;
    MINT32 pos_UDM_HSLR     = rUpperUDM.hf_act2.bits.UDM_HSLR;
    MINT32 pos_UDM_CORE_TH1 = rUpperUDM.hftd_ctl.bits.UDM_CORE_TH1;
    MINT32 pos_UDM_CORE_TH2 = rUpperUDM.hf_str.bits.UDM_CORE_TH2;
    MINT32 pos_UDM_OV_TH    = rUpperUDM.clip.bits.UDM_OV_TH;
    MINT32 pos_UDM_UN_TH    = rUpperUDM.clip.bits.UDM_UN_TH;
    MINT32 pos_UDM_CLIP_TH  = rUpperUDM.clip.bits.UDM_CLIP_TH;
    MINT32 pos_UDM_DS_THB   = rUpperUDM.dsb.bits.UDM_DS_THB;
    MINT32 pos_UDM_DS_SLB   = rUpperUDM.dsb.bits.UDM_DS_SLB;
    MINT32 pos_UDM_FL_MODE  = rUpperUDM.dsb.bits.UDM_FL_MODE;
    MINT32 pos_UDM_SL_RAT   = rUpperUDM.dsb.bits.UDM_SL_RAT;
    MINT32 pos_UDM_SC_RAT   = rUpperUDM.dsb.bits.UDM_SC_RAT;
    MINT32 pos_UDM_LR_RAT   = rUpperUDM.lr_rat.bits.UDM_LR_RAT;

    // interpolation //
    MINT32 ISO = u4RealISO;
    MINT32 ISOpre = u4LowerISO;
    MINT32 ISOpos = u4UpperISO;

    MINT32 UDM_BYP      = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_BYP, pos_UDM_BYP, 0);
    MINT32 UDM_GRAD_WT  = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_GRAD_WT, pos_UDM_GRAD_WT, 0);
    MINT32 UDM_ETH      = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_ETH, pos_UDM_ETH, 0);
    MINT32 UDM_FTH      = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_FTH, pos_UDM_FTH, 0);
    MINT32 UDM_DTH      = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_DTH, pos_UDM_DTH, 0);
    MINT32 UDM_CD_SLC   = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_CD_SLC, pos_UDM_CD_SLC, 0);
    MINT32 UDM_CD_SLL   = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_CD_SLL, pos_UDM_CD_SLL, 0);
    MINT32 UDM_L0_OFST  = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_L0_OFST, pos_UDM_L0_OFST, 0);
    MINT32 UDM_L0_SL    = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_L0_SL, pos_UDM_L0_SL, 0);
    MINT32 UDM_L1_OFST  = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_L1_OFST, pos_UDM_L1_OFST, 0);
    MINT32 UDM_L1_SL    = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_L1_SL, pos_UDM_L1_SL, 0);
    MINT32 UDM_L2_OFST  = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_L2_OFST, pos_UDM_L2_OFST, 0);
    MINT32 UDM_L2_SL    = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_L2_SL, pos_UDM_L2_SL, 0);
    MINT32 UDM_DN_OFST  = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_DN_OFST, pos_UDM_DN_OFST, 0);
    MINT32 UDM_HL_OFST  = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_HL_OFST, pos_UDM_HL_OFST, 0);
    MINT32 UDM_LM_Y0    = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_LM_Y0, pos_UDM_LM_Y0, 0);
    MINT32 UDM_LM_Y1    = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_LM_Y1, pos_UDM_LM_Y1, 0);
    MINT32 UDM_LM_Y2    = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_LM_Y2, pos_UDM_LM_Y2, 0);
    MINT32 UDM_LM_Y3    = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_LM_Y3, pos_UDM_LM_Y3, 0);
    MINT32 UDM_LM_Y4    = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_LM_Y4, pos_UDM_LM_Y4, 0);
    MINT32 UDM_LM_Y5    = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_LM_Y5, pos_UDM_LM_Y5, 0);
    MINT32 UDM_SL_Y1    = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_SL_Y1, pos_UDM_SL_Y1, 0);
    MINT32 UDM_SL_Y2    = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_SL_Y2, pos_UDM_SL_Y2, 0);
    MINT32 UDM_SL_HR    = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_SL_HR, pos_UDM_SL_HR, 0);
    MINT32 UDM_SL_EN    = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_SL_EN, pos_UDM_SL_EN, 0);
    MINT32 UDM_HT_GN1   = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_HT_GN1, pos_UDM_HT_GN1, 0);
    MINT32 UDM_HT_GN2   = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_HT_GN2, pos_UDM_HT_GN2, 0);
    MINT32 UDM_HD_GN1   = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_HD_GN1, pos_UDM_HD_GN1, 0);
    MINT32 UDM_HD_GN2   = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_HD_GN2, pos_UDM_HD_GN2, 0);
    MINT32 UDM_XTK_RAT  = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_XTK_RAT, pos_UDM_XTK_RAT, 0);
    MINT32 UDM_XTK_OFST = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_XTK_OFST, pos_UDM_XTK_OFST, 0);
    MINT32 UDM_XTK_SL   = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_XTK_SL, pos_UDM_XTK_SL, 0);
    MINT32 UDM_N0_STR   = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_N0_STR, pos_UDM_N0_STR, 0);
    MINT32 UDM_N1_STR   = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_N1_STR, pos_UDM_N1_STR, 0);
    MINT32 UDM_N2_STR   = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_N2_STR, pos_UDM_N2_STR, 0);
    MINT32 UDM_N0_OFST  = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_N0_OFST, pos_UDM_N0_OFST, 0);
    MINT32 UDM_N1_OFST  = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_N1_OFST, pos_UDM_N1_OFST, 0);
    MINT32 UDM_N2_OFST  = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_N2_OFST, pos_UDM_N2_OFST, 0);
    MINT32 UDM_NSL      = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_NSL, pos_UDM_NSL, 0);
    MINT32 UDM_NGR      = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_NGR, pos_UDM_NGR, 0);
    MINT32 UDM_HA_STR   = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_HA_STR, pos_UDM_HA_STR, 0);
    MINT32 UDM_H1_GN    = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_H1_GN, pos_UDM_H1_GN, 0);
    MINT32 UDM_H2_GN    = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_H2_GN, pos_UDM_H2_GN, 0);
    MINT32 UDM_H3_GN    = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_H3_GN, pos_UDM_H3_GN, 0);
    MINT32 UDM_HI_RAT   = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_HI_RAT, pos_UDM_HI_RAT, 0);
    MINT32 UDM_H1_LWB   = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_H1_LWB, pos_UDM_H1_LWB, 0);
    MINT32 UDM_H1_UPB   = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_H1_UPB, pos_UDM_H1_UPB, 0);
    MINT32 UDM_H2_LWB   = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_H2_LWB, pos_UDM_H2_LWB, 0);
    MINT32 UDM_H2_UPB   = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_H2_UPB, pos_UDM_H2_UPB, 0);
    MINT32 UDM_H3_LWB   = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_H3_LWB, pos_UDM_H3_LWB, 0);
    MINT32 UDM_H3_UPB   = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_H3_UPB, pos_UDM_H3_UPB, 0);
    MINT32 UDM_HSLL     = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_HSLL, pos_UDM_HSLL, 0);
    MINT32 UDM_HSLR     = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_HSLR, pos_UDM_HSLR, 0);
    MINT32 UDM_CORE_TH1 = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_CORE_TH1, pos_UDM_CORE_TH1, 0);
    MINT32 UDM_CORE_TH2 = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_CORE_TH2, pos_UDM_CORE_TH2, 0);
    MINT32 UDM_OV_TH    = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_OV_TH, pos_UDM_OV_TH, 0);
    MINT32 UDM_UN_TH    = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_UN_TH, pos_UDM_UN_TH, 0);
    MINT32 UDM_CLIP_TH  = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_CLIP_TH, pos_UDM_CLIP_TH, 0);
    MINT32 UDM_DS_THB   = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_DS_THB, pos_UDM_DS_THB, 0);
    MINT32 UDM_DS_SLB   = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_DS_SLB, pos_UDM_DS_SLB, 0);
    MINT32 UDM_FL_MODE  = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_FL_MODE, pos_UDM_FL_MODE, 0);
    MINT32 UDM_SL_RAT   = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_SL_RAT, pos_UDM_SL_RAT, 0);
    MINT32 UDM_SC_RAT   = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_SC_RAT, pos_UDM_SC_RAT, 0);
    MINT32 UDM_LR_RAT   = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_LR_RAT, pos_UDM_LR_RAT, 0);

    rSmoothUDM.intp_crs.bits.UDM_BYP       = UDM_BYP;
    rSmoothUDM.intp_crs.bits.UDM_GRAD_WT   = UDM_GRAD_WT;
    rSmoothUDM.intp_crs.bits.UDM_ETH       = UDM_ETH;
    rSmoothUDM.intp_crs.bits.UDM_FTH       = UDM_FTH;
    rSmoothUDM.intp_crs.bits.UDM_DTH       = UDM_DTH;
    rSmoothUDM.intp_nat.bits.UDM_CD_SLC    = UDM_CD_SLC;
    rSmoothUDM.intp_nat.bits.UDM_CD_SLL    = UDM_CD_SLL;
    rSmoothUDM.intp_nat.bits.UDM_L0_OFST   = UDM_L0_OFST;
    rSmoothUDM.intp_nat.bits.UDM_L0_SL     = UDM_L0_SL;
    rSmoothUDM.intp_aug.bits.UDM_L1_OFST   = UDM_L1_OFST;
    rSmoothUDM.intp_aug.bits.UDM_L1_SL     = UDM_L1_SL;
    rSmoothUDM.intp_aug.bits.UDM_L2_OFST   = UDM_L2_OFST;
    rSmoothUDM.intp_aug.bits.UDM_L2_SL     = UDM_L2_SL;
    rSmoothUDM.intp_aug.bits.UDM_DN_OFST   = UDM_DN_OFST;
    rSmoothUDM.intp_nat.bits.UDM_HL_OFST   = UDM_HL_OFST;
    rSmoothUDM.luma_lut1.bits.UDM_LM_Y0    = UDM_LM_Y0;
    rSmoothUDM.luma_lut1.bits.UDM_LM_Y1    = UDM_LM_Y1;
    rSmoothUDM.luma_lut1.bits.UDM_LM_Y2    = UDM_LM_Y2;
    rSmoothUDM.luma_lut2.bits.UDM_LM_Y3    = UDM_LM_Y3;
    rSmoothUDM.luma_lut2.bits.UDM_LM_Y4    = UDM_LM_Y4;
    rSmoothUDM.luma_lut2.bits.UDM_LM_Y5    = UDM_LM_Y5;
    rSmoothUDM.sl_ctl.bits.UDM_SL_Y1       = UDM_SL_Y1;
    rSmoothUDM.sl_ctl.bits.UDM_SL_Y2       = UDM_SL_Y2;
    rSmoothUDM.sl_ctl.bits.UDM_SL_HR       = UDM_SL_HR;
    rSmoothUDM.sl_ctl.bits.UDM_SL_EN       = UDM_SL_EN;
    rSmoothUDM.hftd_ctl.bits.UDM_HT_GN1    = UDM_HT_GN1;
    rSmoothUDM.hftd_ctl.bits.UDM_HT_GN2    = UDM_HT_GN2;
    rSmoothUDM.hftd_ctl.bits.UDM_HD_GN1    = UDM_HD_GN1;
    rSmoothUDM.hftd_ctl.bits.UDM_HD_GN2    = UDM_HD_GN2;
    rSmoothUDM.nr_str.bits.UDM_XTK_RAT     = UDM_XTK_RAT;
    rSmoothUDM.nr_str.bits.UDM_XTK_OFST    = UDM_XTK_OFST;
    rSmoothUDM.nr_str.bits.UDM_XTK_SL      = UDM_XTK_SL;
    rSmoothUDM.nr_str.bits.UDM_N0_STR      = UDM_N0_STR;
    rSmoothUDM.nr_str.bits.UDM_N1_STR      = UDM_N1_STR;
    rSmoothUDM.nr_str.bits.UDM_N2_STR      = UDM_N2_STR;
    rSmoothUDM.nr_act.bits.UDM_N0_OFST     = UDM_N0_OFST;
    rSmoothUDM.nr_act.bits.UDM_N1_OFST     = UDM_N1_OFST;
    rSmoothUDM.nr_act.bits.UDM_N2_OFST     = UDM_N2_OFST;
    rSmoothUDM.nr_act.bits.UDM_NSL         = UDM_NSL;
    rSmoothUDM.nr_act.bits.UDM_NGR         = UDM_NGR;
    rSmoothUDM.hf_str.bits.UDM_HA_STR      = UDM_HA_STR;
    rSmoothUDM.hf_str.bits.UDM_H1_GN       = UDM_H1_GN;
    rSmoothUDM.hf_str.bits.UDM_H2_GN       = UDM_H2_GN;
    rSmoothUDM.hf_str.bits.UDM_H3_GN       = UDM_H3_GN;
    rSmoothUDM.hf_str.bits.UDM_HI_RAT      = UDM_HI_RAT;
    rSmoothUDM.hf_act1.bits.UDM_H1_LWB     = UDM_H1_LWB;
    rSmoothUDM.hf_act1.bits.UDM_H1_UPB     = UDM_H1_UPB;
    rSmoothUDM.hf_act1.bits.UDM_H2_LWB     = UDM_H2_LWB;
    rSmoothUDM.hf_act1.bits.UDM_H2_UPB     = UDM_H2_UPB;
    rSmoothUDM.hf_act2.bits.UDM_H3_LWB     = UDM_H3_LWB;
    rSmoothUDM.hf_act2.bits.UDM_H3_UPB     = UDM_H3_UPB;
    rSmoothUDM.hf_act2.bits.UDM_HSLL       = UDM_HSLL;
    rSmoothUDM.hf_act2.bits.UDM_HSLR       = UDM_HSLR;
    rSmoothUDM.hftd_ctl.bits.UDM_CORE_TH1  = UDM_CORE_TH1;
    rSmoothUDM.hf_str.bits.UDM_CORE_TH2    = UDM_CORE_TH2;
    rSmoothUDM.clip.bits.UDM_OV_TH         = UDM_OV_TH;
    rSmoothUDM.clip.bits.UDM_UN_TH         = UDM_UN_TH;
    rSmoothUDM.clip.bits.UDM_CLIP_TH       = UDM_CLIP_TH;
    rSmoothUDM.dsb.bits.UDM_DS_THB         = UDM_DS_THB;
    rSmoothUDM.dsb.bits.UDM_DS_SLB         = UDM_DS_SLB;
    rSmoothUDM.dsb.bits.UDM_FL_MODE        = UDM_FL_MODE;
    rSmoothUDM.dsb.bits.UDM_SL_RAT         = UDM_SL_RAT;
    rSmoothUDM.dsb.bits.UDM_SC_RAT         = UDM_SC_RAT;
    rSmoothUDM.lr_rat.bits.UDM_LR_RAT      = UDM_LR_RAT;

    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_BYP      = %d, %d, %d", rLowerUDM.intp_crs.bits.UDM_BYP, rSmoothUDM.intp_crs.bits.UDM_BYP, rUpperUDM.intp_crs.bits.UDM_BYP);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_GRAD_WT  = %d, %d, %d", rLowerUDM.intp_crs.bits.UDM_GRAD_WT, rSmoothUDM.intp_crs.bits.UDM_GRAD_WT, rUpperUDM.intp_crs.bits.UDM_GRAD_WT);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_ETH      = %d, %d, %d", rLowerUDM.intp_crs.bits.UDM_ETH, rSmoothUDM.intp_crs.bits.UDM_ETH, rUpperUDM.intp_crs.bits.UDM_ETH);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_FTH      = %d, %d, %d", rLowerUDM.intp_crs.bits.UDM_FTH, rSmoothUDM.intp_crs.bits.UDM_FTH, rUpperUDM.intp_crs.bits.UDM_FTH);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_DTH      = %d, %d, %d", rLowerUDM.intp_crs.bits.UDM_DTH, rSmoothUDM.intp_crs.bits.UDM_DTH, rUpperUDM.intp_crs.bits.UDM_DTH);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_CD_SLC   = %d, %d, %d", rLowerUDM.intp_nat.bits.UDM_CD_SLC, rSmoothUDM.intp_nat.bits.UDM_CD_SLC, rUpperUDM.intp_nat.bits.UDM_CD_SLC);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_CD_SLL   = %d, %d, %d", rLowerUDM.intp_nat.bits.UDM_CD_SLL, rSmoothUDM.intp_nat.bits.UDM_CD_SLL, rUpperUDM.intp_nat.bits.UDM_CD_SLL);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_L0_OFST  = %d, %d, %d", rLowerUDM.intp_nat.bits.UDM_L0_OFST, rSmoothUDM.intp_nat.bits.UDM_L0_OFST, rUpperUDM.intp_nat.bits.UDM_L0_OFST);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_L0_SL    = %d, %d, %d", rLowerUDM.intp_nat.bits.UDM_L0_SL, rSmoothUDM.intp_nat.bits.UDM_L0_SL, rUpperUDM.intp_nat.bits.UDM_L0_SL);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_L1_OFST  = %d, %d, %d", rLowerUDM.intp_aug.bits.UDM_L1_OFST, rSmoothUDM.intp_aug.bits.UDM_L1_OFST, rUpperUDM.intp_aug.bits.UDM_L1_OFST);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_L1_SL    = %d, %d, %d", rLowerUDM.intp_aug.bits.UDM_L1_SL, rSmoothUDM.intp_aug.bits.UDM_L1_SL, rUpperUDM.intp_aug.bits.UDM_L1_SL);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_L2_OFST  = %d, %d, %d", rLowerUDM.intp_aug.bits.UDM_L2_OFST, rSmoothUDM.intp_aug.bits.UDM_L2_OFST, rUpperUDM.intp_aug.bits.UDM_L2_OFST);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_L2_SL    = %d, %d, %d", rLowerUDM.intp_aug.bits.UDM_L2_SL, rSmoothUDM.intp_aug.bits.UDM_L2_SL, rUpperUDM.intp_aug.bits.UDM_L2_SL);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_DN_OFST  = %d, %d, %d", rLowerUDM.intp_aug.bits.UDM_DN_OFST, rSmoothUDM.intp_aug.bits.UDM_DN_OFST, rUpperUDM.intp_aug.bits.UDM_DN_OFST);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_HL_OFST  = %d, %d, %d", rLowerUDM.intp_nat.bits.UDM_HL_OFST, rSmoothUDM.intp_nat.bits.UDM_HL_OFST, rUpperUDM.intp_nat.bits.UDM_HL_OFST);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_LM_Y0    = %d, %d, %d", rLowerUDM.luma_lut1.bits.UDM_LM_Y0, rSmoothUDM.luma_lut1.bits.UDM_LM_Y0, rUpperUDM.luma_lut1.bits.UDM_LM_Y0);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_LM_Y1    = %d, %d, %d", rLowerUDM.luma_lut1.bits.UDM_LM_Y1, rSmoothUDM.luma_lut1.bits.UDM_LM_Y1, rUpperUDM.luma_lut1.bits.UDM_LM_Y1);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_LM_Y2    = %d, %d, %d", rLowerUDM.luma_lut1.bits.UDM_LM_Y2, rSmoothUDM.luma_lut1.bits.UDM_LM_Y2, rUpperUDM.luma_lut1.bits.UDM_LM_Y2);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_LM_Y3    = %d, %d, %d", rLowerUDM.luma_lut2.bits.UDM_LM_Y3, rSmoothUDM.luma_lut2.bits.UDM_LM_Y3, rUpperUDM.luma_lut2.bits.UDM_LM_Y3);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_LM_Y4    = %d, %d, %d", rLowerUDM.luma_lut2.bits.UDM_LM_Y4, rSmoothUDM.luma_lut2.bits.UDM_LM_Y4, rUpperUDM.luma_lut2.bits.UDM_LM_Y4);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_LM_Y5    = %d, %d, %d", rLowerUDM.luma_lut2.bits.UDM_LM_Y5, rSmoothUDM.luma_lut2.bits.UDM_LM_Y5, rUpperUDM.luma_lut2.bits.UDM_LM_Y5);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_SL_Y1    = %d, %d, %d", rLowerUDM.sl_ctl.bits.UDM_SL_Y1, rSmoothUDM.sl_ctl.bits.UDM_SL_Y1, rUpperUDM.sl_ctl.bits.UDM_SL_Y1);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_SL_Y2    = %d, %d, %d", rLowerUDM.sl_ctl.bits.UDM_SL_Y2, rSmoothUDM.sl_ctl.bits.UDM_SL_Y2, rUpperUDM.sl_ctl.bits.UDM_SL_Y2);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_SL_HR    = %d, %d, %d", rLowerUDM.sl_ctl.bits.UDM_SL_HR, rSmoothUDM.sl_ctl.bits.UDM_SL_HR, rUpperUDM.sl_ctl.bits.UDM_SL_HR);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_SL_EN    = %d, %d, %d", rLowerUDM.sl_ctl.bits.UDM_SL_EN, rSmoothUDM.sl_ctl.bits.UDM_SL_EN, rUpperUDM.sl_ctl.bits.UDM_SL_EN);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_HT_GN1   = %d, %d, %d", rLowerUDM.hftd_ctl.bits.UDM_HT_GN1, rSmoothUDM.hftd_ctl.bits.UDM_HT_GN1, rUpperUDM.hftd_ctl.bits.UDM_HT_GN1);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_HT_GN2   = %d, %d, %d", rLowerUDM.hftd_ctl.bits.UDM_HT_GN2, rSmoothUDM.hftd_ctl.bits.UDM_HT_GN2, rUpperUDM.hftd_ctl.bits.UDM_HT_GN2);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_HD_GN1   = %d, %d, %d", rLowerUDM.hftd_ctl.bits.UDM_HD_GN1, rSmoothUDM.hftd_ctl.bits.UDM_HD_GN1, rUpperUDM.hftd_ctl.bits.UDM_HD_GN1);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_HD_GN2   = %d, %d, %d", rLowerUDM.hftd_ctl.bits.UDM_HD_GN2, rSmoothUDM.hftd_ctl.bits.UDM_HD_GN2, rUpperUDM.hftd_ctl.bits.UDM_HD_GN2);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_XTK_RAT  = %d, %d, %d", rLowerUDM.nr_str.bits.UDM_XTK_RAT, rSmoothUDM.nr_str.bits.UDM_XTK_RAT, rUpperUDM.nr_str.bits.UDM_XTK_RAT);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_XTK_OFST = %d, %d, %d", rLowerUDM.nr_str.bits.UDM_XTK_OFST, rSmoothUDM.nr_str.bits.UDM_XTK_OFST, rUpperUDM.nr_str.bits.UDM_XTK_OFST);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_XTK_SL   = %d, %d, %d", rLowerUDM.nr_str.bits.UDM_XTK_SL, rSmoothUDM.nr_str.bits.UDM_XTK_SL, rUpperUDM.nr_str.bits.UDM_XTK_SL);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_N0_STR   = %d, %d, %d", rLowerUDM.nr_str.bits.UDM_N0_STR, rSmoothUDM.nr_str.bits.UDM_N0_STR, rUpperUDM.nr_str.bits.UDM_N0_STR);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_N1_STR   = %d, %d, %d", rLowerUDM.nr_str.bits.UDM_N1_STR, rSmoothUDM.nr_str.bits.UDM_N1_STR, rUpperUDM.nr_str.bits.UDM_N1_STR);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_N2_STR   = %d, %d, %d", rLowerUDM.nr_str.bits.UDM_N2_STR, rSmoothUDM.nr_str.bits.UDM_N2_STR, rUpperUDM.nr_str.bits.UDM_N2_STR);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_N0_OFST  = %d, %d, %d", rLowerUDM.nr_act.bits.UDM_N0_OFST, rSmoothUDM.nr_act.bits.UDM_N0_OFST, rUpperUDM.nr_act.bits.UDM_N0_OFST);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_N1_OFST  = %d, %d, %d", rLowerUDM.nr_act.bits.UDM_N1_OFST, rSmoothUDM.nr_act.bits.UDM_N1_OFST, rUpperUDM.nr_act.bits.UDM_N1_OFST);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_N2_OFST  = %d, %d, %d", rLowerUDM.nr_act.bits.UDM_N2_OFST, rSmoothUDM.nr_act.bits.UDM_N2_OFST, rUpperUDM.nr_act.bits.UDM_N2_OFST);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_NSL      = %d, %d, %d", rLowerUDM.nr_act.bits.UDM_NSL, rSmoothUDM.nr_act.bits.UDM_NSL, rUpperUDM.nr_act.bits.UDM_NSL);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_NGR      = %d, %d, %d", rLowerUDM.nr_act.bits.UDM_NGR, rSmoothUDM.nr_act.bits.UDM_NGR, rUpperUDM.nr_act.bits.UDM_NGR);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_HA_STR   = %d, %d, %d", rLowerUDM.hf_str.bits.UDM_HA_STR, rSmoothUDM.hf_str.bits.UDM_HA_STR, rUpperUDM.hf_str.bits.UDM_HA_STR);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_H1_GN    = %d, %d, %d", rLowerUDM.hf_str.bits.UDM_H1_GN, rSmoothUDM.hf_str.bits.UDM_H1_GN, rUpperUDM.hf_str.bits.UDM_H1_GN);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_H2_GN    = %d, %d, %d", rLowerUDM.hf_str.bits.UDM_H2_GN, rSmoothUDM.hf_str.bits.UDM_H2_GN, rUpperUDM.hf_str.bits.UDM_H2_GN);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_H3_GN    = %d, %d, %d", rLowerUDM.hf_str.bits.UDM_H3_GN, rSmoothUDM.hf_str.bits.UDM_H3_GN, rUpperUDM.hf_str.bits.UDM_H3_GN);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_HI_RAT   = %d, %d, %d", rLowerUDM.hf_str.bits.UDM_HI_RAT, rSmoothUDM.hf_str.bits.UDM_HI_RAT, rUpperUDM.hf_str.bits.UDM_HI_RAT);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_H1_LWB   = %d, %d, %d", rLowerUDM.hf_act1.bits.UDM_H1_LWB, rSmoothUDM.hf_act1.bits.UDM_H1_LWB, rUpperUDM.hf_act1.bits.UDM_H1_LWB);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_H1_UPB   = %d, %d, %d", rLowerUDM.hf_act1.bits.UDM_H1_UPB, rSmoothUDM.hf_act1.bits.UDM_H1_UPB, rUpperUDM.hf_act1.bits.UDM_H1_UPB);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_H2_LWB   = %d, %d, %d", rLowerUDM.hf_act1.bits.UDM_H2_LWB, rSmoothUDM.hf_act1.bits.UDM_H2_LWB, rUpperUDM.hf_act1.bits.UDM_H2_LWB);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_H2_UPB   = %d, %d, %d", rLowerUDM.hf_act1.bits.UDM_H2_UPB, rSmoothUDM.hf_act1.bits.UDM_H2_UPB, rUpperUDM.hf_act1.bits.UDM_H2_UPB);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_H3_LWB   = %d, %d, %d", rLowerUDM.hf_act2.bits.UDM_H3_LWB, rSmoothUDM.hf_act2.bits.UDM_H3_LWB, rUpperUDM.hf_act2.bits.UDM_H3_LWB);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_H3_UPB   = %d, %d, %d", rLowerUDM.hf_act2.bits.UDM_H3_UPB, rSmoothUDM.hf_act2.bits.UDM_H3_UPB, rUpperUDM.hf_act2.bits.UDM_H3_UPB);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_HSLL     = %d, %d, %d", rLowerUDM.hf_act2.bits.UDM_HSLL, rSmoothUDM.hf_act2.bits.UDM_HSLL, rUpperUDM.hf_act2.bits.UDM_HSLL);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_HSLR     = %d, %d, %d", rLowerUDM.hf_act2.bits.UDM_HSLR, rSmoothUDM.hf_act2.bits.UDM_HSLR, rUpperUDM.hf_act2.bits.UDM_HSLR);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_CORE_TH1 = %d, %d, %d", rLowerUDM.hftd_ctl.bits.UDM_CORE_TH1, rSmoothUDM.hftd_ctl.bits.UDM_CORE_TH1, rUpperUDM.hftd_ctl.bits.UDM_CORE_TH1);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_CORE_TH2 = %d, %d, %d", rLowerUDM.hf_str.bits.UDM_CORE_TH2, rSmoothUDM.hf_str.bits.UDM_CORE_TH2, rUpperUDM.hf_str.bits.UDM_CORE_TH2);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_OV_TH    = %d, %d, %d", rLowerUDM.clip.bits.UDM_OV_TH, rSmoothUDM.clip.bits.UDM_OV_TH, rUpperUDM.clip.bits.UDM_OV_TH);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_UN_TH    = %d, %d, %d", rLowerUDM.clip.bits.UDM_UN_TH, rSmoothUDM.clip.bits.UDM_UN_TH, rUpperUDM.clip.bits.UDM_UN_TH);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_CLIP_TH  = %d, %d, %d", rLowerUDM.clip.bits.UDM_CLIP_TH, rSmoothUDM.clip.bits.UDM_CLIP_TH, rUpperUDM.clip.bits.UDM_CLIP_TH);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_DS_THB   = %d, %d, %d", rLowerUDM.dsb.bits.UDM_DS_THB, rSmoothUDM.dsb.bits.UDM_DS_THB, rUpperUDM.dsb.bits.UDM_DS_THB);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_DS_SLB   = %d, %d, %d", rLowerUDM.dsb.bits.UDM_DS_SLB, rSmoothUDM.dsb.bits.UDM_DS_SLB, rUpperUDM.dsb.bits.UDM_DS_SLB);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_FL_MODE  = %d, %d, %d", rLowerUDM.dsb.bits.UDM_FL_MODE, rSmoothUDM.dsb.bits.UDM_FL_MODE, rUpperUDM.dsb.bits.UDM_FL_MODE);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_SL_RAT   = %d, %d, %d", rLowerUDM.dsb.bits.UDM_SL_RAT, rSmoothUDM.dsb.bits.UDM_SL_RAT, rUpperUDM.dsb.bits.UDM_SL_RAT);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_SC_RAT   = %d, %d, %d", rLowerUDM.dsb.bits.UDM_SC_RAT, rSmoothUDM.dsb.bits.UDM_SC_RAT, rUpperUDM.dsb.bits.UDM_SC_RAT);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_LR_RAT   = %d, %d, %d", rLowerUDM.lr_rat.bits.UDM_LR_RAT, rSmoothUDM.lr_rat.bits.UDM_LR_RAT, rUpperUDM.lr_rat.bits.UDM_LR_RAT);
}

#if 0
MINT32 InterParam_LCE(MINT32 ISO, MINT32 ISOpre, MINT32 ISOpos, MINT32 PARAMpre, MINT32 PARAMpos, MINT32 Method){
	MINT32 InterPARAM = 0;

	if(ISOpre == ISOpos){
		//	To prevent someone sets different PARAMpre and PARAMpos
		return (MINT32)((PARAMpre + PARAMpos + 1) / 2);
	}

	switch(Method){
	case 1:
		InterPARAM = (abs((double)(ISO - ISOpre)) > abs((double)(ISOpos - ISO))) ? PARAMpos : PARAMpre;
		break;
	case 0:
	default:
		double RATIOpre = (double)(ISOpos - ISO) / (double)(ISOpos - ISOpre);
		double RATIOpos = 1.0 - (double)(RATIOpre);
		double temp =  RATIOpre * PARAMpre + RATIOpos * PARAMpos;
		if(temp>0)
			InterPARAM = (MINT32)(RATIOpre * PARAMpre + RATIOpos * PARAMpos + 0.5);
		else
			InterPARAM = (MINT32)(RATIOpre * PARAMpre + RATIOpos * PARAMpos - 0.5);
		break;
	}

	return InterPARAM;
}

MVOID SmoothLCE(MUINT32 u4RealISO,  // Real ISO
                 MUINT32 u4UpperISO, // Upper ISO
                 MUINT32 u4LowerISO, // Lower ISO
                 ISP_NVRAM_LCE_T const& rUpperLCE, // RMM settings for upper ISO
                 ISP_NVRAM_LCE_T const& rLowerLCE,   // RMM settings for lower ISO
                 ISP_NVRAM_LCE_T& rSmoothLCE)   // Output
{
	char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.smooth_LCE.enable", value, "0");
    MBOOL bSmoothLCEDebug = atoi(value);

    MY_LOG_IF(bSmoothLCEDebug,"[%s()] u4RealISO: %d, u4UpperISO = %d, u4LowerISO = %d\n", __FUNCTION__, u4RealISO, u4UpperISO, u4LowerISO);

	//	====================================================================================================================================
	//	Get Lower ISO setting
	//	====================================================================================================================================
	// DIP_X_LCE_QUA CAM+9C8H
	MINT32 LCE_PA_low        = rLowerLCE.qua.bits.LCE_PA;
	MINT32 LCE_PB_low        = rLowerLCE.qua.bits.LCE_PB;
	MINT32 LCE_BA_low        = rLowerLCE.qua.bits.LCE_BA;


	//	====================================================================================================================================
	//	Get Upper ISO setting
	//	====================================================================================================================================
	// DIP_X_LCE_QUA CAM+9C8H
	MINT32 LCE_PA_high       = rUpperLCE.qua.bits.LCE_PA;
	MINT32 LCE_PB_high       = rUpperLCE.qua.bits.LCE_PB;
	MINT32 LCE_BA_high       = rUpperLCE.qua.bits.LCE_BA;

	//	====================================================================================================================================
	//	Interpolating reg declaration
	//	====================================================================================================================================

	// DIP_X_LCE_QUA CAM+9C8H
	MINT32 LCE_PA;
	MINT32 LCE_PB;
	MINT32 LCE_BA;

	//	====================================================================================================================================
	//	Start Parameter Interpolation
	//	====================================================================================================================================

	MINT32 ISO = u4RealISO;
	MINT32 ISO_low = u4LowerISO;
	MINT32 ISO_high = u4UpperISO;


	//Parameter can interpolate

	LCE_PA        = InterParam_LCE(ISO, ISO_low, ISO_high, LCE_PA_low   , LCE_PA_high   , 0);
	LCE_PB        = InterParam_LCE(ISO, ISO_low, ISO_high, LCE_PB_low   , LCE_PB_high   , 0);
	LCE_BA        = InterParam_LCE(ISO, ISO_low, ISO_high, LCE_BA_low   , LCE_BA_high   , 0);
	//	====================================================================================================================================
	//	Set Smooth ISO setting
	//	====================================================================================================================================
	// DIP_X_LCE_QUA CAM+9C8H
  rSmoothLCE.qua.bits.LCE_PA   = LCE_PA;
  rSmoothLCE.qua.bits.LCE_PB   = LCE_PB;
  rSmoothLCE.qua.bits.LCE_BA   = LCE_BA;

#define LCE_DUMP(item1, item2)   \
        do{                      \
            MY_LOG_IF(bSmoothLCEDebug, "[L,S,U]%22s = %d, %d, %d", #item2, rLowerLCE.item1.bits.item2, rSmoothLCE.item1.bits.item2, rUpperLCE.item1.bits.item2); \
        }while(0)

    LCE_DUMP(qua, LCE_PA);
    LCE_DUMP(qua, LCE_PB);
    LCE_DUMP(qua, LCE_BA);

#undef LCE_DUMP
}
#endif

//  "return value": Interpolated register value
//  "ISO":          ISO value of current image
//  "ISOpre":       A pre-defined ISO (100, 200, 400, ...) which is the closest but smaller than or equal to "ISO"
//  "ISOpos":       A pre-defined ISO (100, 200, 400, ...) which is the closest but larger than or equal to "ISO"
//  "PARAMpre":     Corresponding register value from "ISOpre"
//  "PARAMpos":     Corresponding register value from "ISOpos"
//  "Method":       0 for linear interpolation, 1 for closest one
MINT32 InterParam_NR(MINT32 ISO, MINT32 ISOpre, MINT32 ISOpos, MINT32 PARAMpre, MINT32 PARAMpos, MINT32 Method){
	MINT32 InterPARAM = 0;

	if(ISOpre == ISOpos){
		//	To prevent someone sets different PARAMpre and PARAMpos
		return (MINT32)((PARAMpre + PARAMpos + 1) / 2);
	}

	switch(Method){
	case 1:
		InterPARAM = (abs((double)(ISO - ISOpre)) > abs((double)(ISOpos - ISO))) ? PARAMpos : PARAMpre;
		break;
	case 0:
	default:
		double RATIOpre = (double)(ISOpos - ISO) / (double)(ISOpos - ISOpre);
		double RATIOpos = 1.0 - (double)(RATIOpre);
		double temp =  RATIOpre * PARAMpre + RATIOpos * PARAMpos;
		if(temp>0)
			InterPARAM = (MINT32)(RATIOpre * PARAMpre + RATIOpos * PARAMpos + 0.5);
		else
			InterPARAM = (MINT32)(RATIOpre * PARAMpre + RATIOpos * PARAMpos - 0.5);
		break;
	}

	return InterPARAM;
}

MINT32 CalculateLUTSP(MINT32 X0, MINT32 X1, MINT32 Y0, MINT32 Y1, MINT32 slope_base, MINT32 max_slope, MINT32 min_slope)
{
    MINT32 slope;
    double dividor = X1 - X0;
    dividor = (dividor > 1) ? dividor : 1;

    double slope_d = (double)((Y1 - Y0) * slope_base) / dividor;
    if (slope_d > 0.0)
    {
        slope = (MINT32)(slope_d + 0.5);
    }
    else
    {
        slope = (MINT32)(slope_d - 0.5);
    }

    slope = (slope > max_slope) ? max_slope : slope;
    slope = (slope < min_slope) ? min_slope : slope;

    return slope;
}

MVOID SmoothANR(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_ANR_T const& rUpperANR, // ANR settings for upper ISO
                ISP_NVRAM_ANR_T const& rLowerANR,   // ANR settings for lower ISO
                ISP_NVRAM_ANR_T& rSmoothANR)   // Output
{
	char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.smooth_anr.enable", value, "0");
    MBOOL bSmoothANRDebug = atoi(value);

    MY_LOG_IF(bSmoothANRDebug,"[%s()] u4RealISO: %d, u4UpperISO = %d, u4LowerISO = %d\n", __FUNCTION__, u4RealISO, u4UpperISO, u4LowerISO);

    //    ====================================================================================================================================
    //    Get Lower ISO setting
    //    ====================================================================================================================================
    // CAM_ANR_CON1 CAM+A20H
    MINT32 ANR_ENC_low           = rLowerANR.con1.bits.ANR_ENC;
    MINT32 ANR_ENY_low           = rLowerANR.con1.bits.ANR_ENY;
    MINT32 ANR_SCALE_MODE_low    = rLowerANR.con1.bits.ANR_SCALE_MODE;
    MINT32 ANR_MEDIAN_EN_low     = rLowerANR.con1.bits.ANR_MEDIAN_EN;
    MINT32 ANR_LCE_LINK_low      = rLowerANR.con1.bits.ANR_LCE_LINK;
    MINT32 ANR_TABLE_EN_low      = rLowerANR.con1.bits.ANR_TABLE_EN;
    MINT32 ANR_TBL_PRC_low       = rLowerANR.con1.bits.ANR_TBL_PRC;

    // CAM_ANR_CON2 CAM+A24H
    MINT32 ANR_IMPL_MODE_low     = rLowerANR.con2.bits.ANR_IMPL_MODE;
    MINT32 ANR_C_MED_EN_low      = rLowerANR.con2.bits.ANR_C_MED_EN;
    MINT32 ANR_C_SM_EDGE_low     = rLowerANR.con2.bits.ANR_C_SM_EDGE;
    MINT32 ANR_FLT_C_low         = rLowerANR.con2.bits.ANR_FLT_C;
    MINT32 ANR_LR_low            = rLowerANR.con2.bits.ANR_LR;
    MINT32 ANR_ALPHA_low         = rLowerANR.con2.bits.ANR_ALPHA;

    // CAM_ANR_YAD1 CAM+A2CH
    MINT32 ANR_CEN_GAIN_LO_TH_low= rLowerANR.yad1.bits.ANR_CEN_GAIN_LO_TH;
    MINT32 ANR_CEN_GAIN_HI_TH_low= rLowerANR.yad1.bits.ANR_CEN_GAIN_HI_TH;
    MINT32 ANR_K_LO_TH_low       = rLowerANR.yad1.bits.ANR_K_LO_TH;
    MINT32 ANR_K_HI_TH_low       = rLowerANR.yad1.bits.ANR_K_HI_TH;
    MINT32 ANR_K_TH_C_low        = rLowerANR.yad1.bits.ANR_K_TH_C;

    // CAM_ANR_YAD2 CAM+A30H
    MINT32 ANR_PTY_VGAIN_low     = rLowerANR.yad2.bits.ANR_PTY_VGAIN;
    MINT32 ANR_PTY_GAIN_TH_low   = rLowerANR.yad2.bits.ANR_PTY_GAIN_TH;

    // CAM_ANR_Y4LUT1 CAM+A34H
    MINT32 ANR_Y_CPX1_low        = rLowerANR.y4lut1.bits.ANR_Y_CPX1;
    MINT32 ANR_Y_CPX2_low        = rLowerANR.y4lut1.bits.ANR_Y_CPX2;
    MINT32 ANR_Y_CPX3_low        = rLowerANR.y4lut1.bits.ANR_Y_CPX3;

    // CAM_ANR_Y4LUT2 CAM+A38H
    MINT32 ANR_Y_SCALE_CPY0_low  = rLowerANR.y4lut2.bits.ANR_Y_SCALE_CPY0;
    MINT32 ANR_Y_SCALE_CPY1_low  = rLowerANR.y4lut2.bits.ANR_Y_SCALE_CPY1;
    MINT32 ANR_Y_SCALE_CPY2_low  = rLowerANR.y4lut2.bits.ANR_Y_SCALE_CPY2;
    MINT32 ANR_Y_SCALE_CPY3_low  = rLowerANR.y4lut2.bits.ANR_Y_SCALE_CPY3;

    // CAM_ANR_Y4LUT3 CAM+A3CH
    MINT32 ANR_Y_SCALE_SP0_low   = rLowerANR.y4lut3.bits.ANR_Y_SCALE_SP0;
    MINT32 ANR_Y_SCALE_SP1_low   = rLowerANR.y4lut3.bits.ANR_Y_SCALE_SP1;
    MINT32 ANR_Y_SCALE_SP2_low   = rLowerANR.y4lut3.bits.ANR_Y_SCALE_SP2;
    MINT32 ANR_Y_SCALE_SP3_low   = rLowerANR.y4lut3.bits.ANR_Y_SCALE_SP3;

    // CAM_ANR_C4LUT1
    MINT32 ANR_C_CPX1_low        = rLowerANR.c4lut1.bits.ANR_C_CPX1;
    MINT32 ANR_C_CPX2_low        = rLowerANR.c4lut1.bits.ANR_C_CPX2;
    MINT32 ANR_C_CPX3_low        = rLowerANR.c4lut1.bits.ANR_C_CPX3;

    // CAM_ANR_C4LUT2
    MINT32 ANR_C_SCALE_CPY0_low  = rLowerANR.c4lut2.bits.ANR_C_SCALE_CPY0;
    MINT32 ANR_C_SCALE_CPY1_low  = rLowerANR.c4lut2.bits.ANR_C_SCALE_CPY1;
    MINT32 ANR_C_SCALE_CPY2_low  = rLowerANR.c4lut2.bits.ANR_C_SCALE_CPY2;
    MINT32 ANR_C_SCALE_CPY3_low  = rLowerANR.c4lut2.bits.ANR_C_SCALE_CPY3;

    // CAM_ANR_C4LUT3
    MINT32 ANR_C_SCALE_SP0_low   = rLowerANR.c4lut3.bits.ANR_C_SCALE_SP0;
    MINT32 ANR_C_SCALE_SP1_low   = rLowerANR.c4lut3.bits.ANR_C_SCALE_SP1;
    MINT32 ANR_C_SCALE_SP2_low   = rLowerANR.c4lut3.bits.ANR_C_SCALE_SP2;
    MINT32 ANR_C_SCALE_SP3_low   = rLowerANR.c4lut3.bits.ANR_C_SCALE_SP3;

    // CAM_ANR_A4LUT2
    MINT32 ANR_Y_ACT_CPY0_low    = rLowerANR.a4lut2.bits.ANR_Y_ACT_CPY0;
    MINT32 ANR_Y_ACT_CPY1_low    = rLowerANR.a4lut2.bits.ANR_Y_ACT_CPY1;
    MINT32 ANR_Y_ACT_CPY2_low    = rLowerANR.a4lut2.bits.ANR_Y_ACT_CPY2;
    MINT32 ANR_Y_ACT_CPY3_low    = rLowerANR.a4lut2.bits.ANR_Y_ACT_CPY3;

    // CAM_ANR_A4LUT3
    MINT32 ANR_Y_ACT_SP0_low     = rLowerANR.a4lut3.bits.ANR_Y_ACT_SP0;
    MINT32 ANR_Y_ACT_SP1_low     = rLowerANR.a4lut3.bits.ANR_Y_ACT_SP1;
    MINT32 ANR_Y_ACT_SP2_low     = rLowerANR.a4lut3.bits.ANR_Y_ACT_SP2;
    MINT32 ANR_Y_ACT_SP3_low     = rLowerANR.a4lut3.bits.ANR_Y_ACT_SP3;

    // CAM_ANR_L4LUT1
    MINT32 ANR_LCE_X1_low        = rLowerANR.l4lut1.bits.ANR_LCE_X1;
    MINT32 ANR_LCE_X2_low        = rLowerANR.l4lut1.bits.ANR_LCE_X2;
    MINT32 ANR_LCE_X3_low        = rLowerANR.l4lut1.bits.ANR_LCE_X3;

    // CAM_ANR_L4LUT2
    MINT32 ANR_LCE_GAIN0_low     = rLowerANR.l4lut2.bits.ANR_LCE_GAIN0;
    MINT32 ANR_LCE_GAIN1_low     = rLowerANR.l4lut2.bits.ANR_LCE_GAIN1;
    MINT32 ANR_LCE_GAIN2_low     = rLowerANR.l4lut2.bits.ANR_LCE_GAIN2;
    MINT32 ANR_LCE_GAIN3_low     = rLowerANR.l4lut2.bits.ANR_LCE_GAIN3;

    // CAM_ANR_L4LUT3
    MINT32 ANR_LCE_SP0_low       = rLowerANR.l4lut3.bits.ANR_LCE_SP0;
    MINT32 ANR_LCE_SP1_low       = rLowerANR.l4lut3.bits.ANR_LCE_SP1;
    MINT32 ANR_LCE_SP2_low       = rLowerANR.l4lut3.bits.ANR_LCE_SP2;
    MINT32 ANR_LCE_SP3_low       = rLowerANR.l4lut3.bits.ANR_LCE_SP3;

    // CAM_ANR_PTY CAM+A40H
    MINT32 ANR_PTY1_low          = rLowerANR.pty.bits.ANR_PTY1;
    MINT32 ANR_PTY2_low          = rLowerANR.pty.bits.ANR_PTY2;
    MINT32 ANR_PTY3_low          = rLowerANR.pty.bits.ANR_PTY3;
    MINT32 ANR_PTY4_low          = rLowerANR.pty.bits.ANR_PTY4;

    // CAM_ANR_CAD CAM+A44H
    MINT32 ANR_PTC_VGAIN_low     = rLowerANR.cad.bits.ANR_PTC_VGAIN;
    MINT32 ANR_PTC_GAIN_TH_low   = rLowerANR.cad.bits.ANR_PTC_GAIN_TH;
    MINT32 ANR_C_L_DIFF_TH_low   = rLowerANR.cad.bits.ANR_C_L_DIFF_TH;

    // CAM_ANR_PTC CAM+A48H
    MINT32 ANR_PTC1_low          = rLowerANR.ptc.bits.ANR_PTC1;
    MINT32 ANR_PTC2_low          = rLowerANR.ptc.bits.ANR_PTC2;
    MINT32 ANR_PTC3_low          = rLowerANR.ptc.bits.ANR_PTC3;
    MINT32 ANR_PTC4_low          = rLowerANR.ptc.bits.ANR_PTC4;

    // CAM_ANR_LCE CAM+A4CH
    MINT32 ANR_LCE_C_GAIN_low    = rLowerANR.lce.bits.ANR_LCE_C_GAIN;
    MINT32 ANR_LCE_SCALE_GAIN_low= rLowerANR.lce.bits.ANR_LCE_SCALE_GAIN;
    MINT32 ANR_LM_WT_low         = rLowerANR.lce.bits.ANR_LM_WT;

    // CAM_ANR_MED1
    MINT32 ANR_COR_TH_low        = rLowerANR.med1.bits.ANR_COR_TH;
    MINT32 ANR_COR_SL_low        = rLowerANR.med1.bits.ANR_COR_SL;
    MINT32 ANR_MCD_TH_low        = rLowerANR.med1.bits.ANR_MCD_TH;
    MINT32 ANR_MCD_SL_low        = rLowerANR.med1.bits.ANR_MCD_SL;
    MINT32 ANR_LCL_TH_low        = rLowerANR.med1.bits.ANR_LCL_TH;

    // CAM_ANR_MED2
    MINT32 ANR_LCL_SL_low        = rLowerANR.med2.bits.ANR_LCL_SL;
    MINT32 ANR_LCL_LV_low        = rLowerANR.med2.bits.ANR_LCL_LV;
    MINT32 ANR_SCL_TH_low        = rLowerANR.med2.bits.ANR_SCL_TH;
    MINT32 ANR_SCL_SL_low        = rLowerANR.med2.bits.ANR_SCL_SL;
    MINT32 ANR_SCL_LV_low        = rLowerANR.med2.bits.ANR_SCL_LV;

    // CAM_ANR_MED3
    MINT32 ANR_NCL_TH_low        = rLowerANR.med3.bits.ANR_NCL_TH;
    MINT32 ANR_NCL_SL_low        = rLowerANR.med3.bits.ANR_NCL_SL;
    MINT32 ANR_NCL_LV_low        = rLowerANR.med3.bits.ANR_NCL_LV;
    MINT32 ANR_VAR_low           = rLowerANR.med3.bits.ANR_VAR;
    MINT32 ANR_Y0_low            = rLowerANR.med3.bits.ANR_Y0;

    // CAM_ANR_MED4
    MINT32 ANR_Y1_low            = rLowerANR.med4.bits.ANR_Y1;
    MINT32 ANR_Y2_low            = rLowerANR.med4.bits.ANR_Y2;
    MINT32 ANR_Y3_low            = rLowerANR.med4.bits.ANR_Y3;
    MINT32 ANR_Y4_low            = rLowerANR.med4.bits.ANR_Y4;

    // CAM_ANR_HP1 CAM+A54H
    MINT32 ANR_HP_A_low          = rLowerANR.hp1.bits.ANR_HP_A;
    MINT32 ANR_HP_B_low          = rLowerANR.hp1.bits.ANR_HP_B;
    MINT32 ANR_HP_C_low          = rLowerANR.hp1.bits.ANR_HP_C;
    MINT32 ANR_HP_D_low          = rLowerANR.hp1.bits.ANR_HP_D;
    MINT32 ANR_HP_E_low          = rLowerANR.hp1.bits.ANR_HP_E;

    // CAM_ANR_HP2 CAM+A58H
    MINT32 ANR_HP_S1_low         = rLowerANR.hp2.bits.ANR_HP_S1;
    MINT32 ANR_HP_S2_low         = rLowerANR.hp2.bits.ANR_HP_S2;
    MINT32 ANR_HP_X1_low         = rLowerANR.hp2.bits.ANR_HP_X1;
    MINT32 ANR_HP_F_low          = rLowerANR.hp2.bits.ANR_HP_F;

    // CAM_ANR_HP3 CAM+A5CH
    MINT32 ANR_HP_Y_GAIN_CLIP_low= rLowerANR.hp3.bits.ANR_HP_Y_GAIN_CLIP;
    MINT32 ANR_HP_Y_SP_low       = rLowerANR.hp3.bits.ANR_HP_Y_SP;
    MINT32 ANR_HP_Y_LO_low       = rLowerANR.hp3.bits.ANR_HP_Y_LO;
    MINT32 ANR_HP_CLIP_low       = rLowerANR.hp3.bits.ANR_HP_CLIP;

    // CAM_ANR_ACT1 CAM+A64H
    MINT32 ANR_ACT_LCE_GAIN_low  = rLowerANR.act1.bits.ANR_ACT_LCE_GAIN;
    MINT32 ANR_ACT_SCALE_OFT_low = rLowerANR.act1.bits.ANR_ACT_SCALE_OFT;
    MINT32 ANR_ACT_SCALE_GAIN_low= rLowerANR.act1.bits.ANR_ACT_SCALE_GAIN;
    MINT32 ANR_ACT_DIF_GAIN_low  = rLowerANR.act1.bits.ANR_ACT_DIF_GAIN;
    MINT32 ANR_ACT_DIF_LO_TH_low = rLowerANR.act1.bits.ANR_ACT_DIF_LO_TH;

    // CAM_ANR_ACT2 CAM+A64H
    MINT32 ANR_ACT_SIZE_GAIN_low = rLowerANR.act2.bits.ANR_ACT_SIZE_GAIN;
    MINT32 ANR_ACT_SIZE_LO_TH_low= rLowerANR.act2.bits.ANR_ACT_SIZE_LO_TH;
    MINT32 ANR_COR_TH1_low       = rLowerANR.act2.bits.ANR_COR_TH1;
    MINT32 ANR_COR_SL1_low       = rLowerANR.act2.bits.ANR_COR_SL1;

    // CAM_ANR_ACT3 CAM+A64H
    MINT32 ANR_COR_ACT_TH_low    = rLowerANR.act3.bits.ANR_COR_ACT_TH;
    MINT32 ANR_COR_ACT_SL1_low   = rLowerANR.act3.bits.ANR_COR_ACT_SL1;
    MINT32 ANR_COR_ACT_SL2_low   = rLowerANR.act3.bits.ANR_COR_ACT_SL2;

    // CAM_ANR_ACTC CAM+A64H
    MINT32 ANR_ACT_TH_C_low      = rLowerANR.actc.bits.ANR_ACT_TH_C;
    MINT32 ANR_ACT_BLD_BASE_C_low= rLowerANR.actc.bits.ANR_ACT_BLD_BASE_C;
    MINT32 ANR_ACT_SLANT_C_low   = rLowerANR.actc.bits.ANR_ACT_SLANT_C;
    MINT32 ANR_ACT_BLD_TH_C_low  = rLowerANR.actc.bits.ANR_ACT_BLD_TH_C;

    // DIP_X_ANR_ACTYL CAM+A60H
    MINT32 ANR_ACT_TH_Y_L_low = rLowerANR.actyl.bits.ANR_ACT_TH_Y_L;
    MINT32 ANR_ACT_BLD_BASE_Y_L_low = rLowerANR.actyl.bits.ANR_ACT_BLD_BASE_Y_L;
    MINT32 ANR_ACT_SLANT_Y_L_low = rLowerANR.actyl.bits.ANR_ACT_SLANT_Y_L;
    MINT32 ANR_ACT_BLD_TH_Y_L_low = rLowerANR.actyl.bits.ANR_ACT_BLD_TH_Y_L;

    // DIP_X_ANR_YLAD CAM+A60H
    MINT32 ANR_CEN_GAIN_LO_TH_LPF_low = rLowerANR.ylad.bits.ANR_CEN_GAIN_LO_TH_LPF;
    MINT32 ANR_CEN_GAIN_HI_TH_LPF_low = rLowerANR.ylad.bits.ANR_CEN_GAIN_HI_TH_LPF;
    MINT32 ANR_K_LMT_low = rLowerANR.ylad.bits.ANR_K_LMT;
    MINT32 ANR_K_LPF_low = rLowerANR.ylad.bits.ANR_K_LPF;

    // DIP_X_ANR_PTYL CAM+A60H
    MINT32 ANR_PTY1_LPF_low = rLowerANR.ptyl.bits.ANR_PTY1_LPF;
    MINT32 ANR_PTY2_LPF_low = rLowerANR.ptyl.bits.ANR_PTY2_LPF;
    MINT32 ANR_PTY3_LPF_low = rLowerANR.ptyl.bits.ANR_PTY3_LPF;
    MINT32 ANR_PTY4_LPF_low = rLowerANR.ptyl.bits.ANR_PTY4_LPF;

    // DIP_X_ANR_LCOEF CAM+A60H
    MINT32 ANR_COEF_A_low = rLowerANR.lcoef.bits.ANR_COEF_A;
    MINT32 ANR_COEF_B_low = rLowerANR.lcoef.bits.ANR_COEF_B;
    MINT32 ANR_COEF_C_low = rLowerANR.lcoef.bits.ANR_COEF_C;
    MINT32 ANR_COEF_D_low = rLowerANR.lcoef.bits.ANR_COEF_D;

    // Olympus
    MINT32 ANR_C_SM_EDGE_TH_low       = rLowerANR.con2.bits.ANR_C_SM_EDGE_TH;
    MINT32 ANR_K_HPF_low              = rLowerANR.ylad.bits.ANR_K_HPF;
    MINT32 ANR_ACT_DIF_HI_TH_low      = rLowerANR.act2.bits.ANR_ACT_DIF_HI_TH;
    MINT32 ANR_ACT_TH_Y_HL_low        = rLowerANR.actyhl.bits.ANR_ACT_TH_Y_HL;
    MINT32 ANR_ACT_BLD_BASE_Y_HL_low  = rLowerANR.actyhl.bits.ANR_ACT_BLD_BASE_Y_HL;
    MINT32 ANR_ACT_SLANT_Y_HL_low     = rLowerANR.actyhl.bits.ANR_ACT_SLANT_Y_HL;
    MINT32 ANR_ACT_BLD_TH_Y_HL_low    = rLowerANR.actyhl.bits.ANR_ACT_BLD_TH_Y_HL;
    MINT32 ANR_ACT_TH_Y_HH_low        = rLowerANR.actyhh.bits.ANR_ACT_TH_Y_HH;
    MINT32 ANR_ACT_BLD_BASE_Y_HH_low  = rLowerANR.actyhh.bits.ANR_ACT_BLD_BASE_Y_HH;
    MINT32 ANR_ACT_SLANT_Y_HH_low     = rLowerANR.actyhh.bits.ANR_ACT_SLANT_Y_HH;
    MINT32 ANR_ACT_BLD_TH_Y_HH_low    = rLowerANR.actyhh.bits.ANR_ACT_BLD_TH_Y_HH;
    MINT32 ANR_LUMA_TH_Y_L_low        = rLowerANR.actyl2.bits.ANR_LUMA_TH_Y_L;
    MINT32 ANR_LUMA_BLD_BASE_Y_L_low  = rLowerANR.actyl2.bits.ANR_LUMA_BLD_BASE_Y_L;
    MINT32 ANR_LUMA_SLANT_Y_L_low     = rLowerANR.actyl2.bits.ANR_LUMA_SLANT_Y_L;
    MINT32 ANR_LUMA_BLD_TH_Y_L_low    = rLowerANR.actyl2.bits.ANR_LUMA_BLD_TH_Y_L;
    MINT32 ANR_LUMA_TH_Y_HL_low       = rLowerANR.actyhl2.bits.ANR_LUMA_TH_Y_HL;
    MINT32 ANR_LUMA_BLD_BASE_Y_HL_low = rLowerANR.actyhl2.bits.ANR_LUMA_BLD_BASE_Y_HL;
    MINT32 ANR_LUMA_SLANT_Y_HL_low    = rLowerANR.actyhl2.bits.ANR_LUMA_SLANT_Y_HL;
    MINT32 ANR_LUMA_BLD_TH_Y_HL_low   = rLowerANR.actyhl2.bits.ANR_LUMA_BLD_TH_Y_HL;
    MINT32 ANR_LUMA_TH_Y_HH_low       = rLowerANR.actyhh2.bits.ANR_LUMA_TH_Y_HH;
    MINT32 ANR_LUMA_BLD_BASE_Y_HH_low = rLowerANR.actyhh2.bits.ANR_LUMA_BLD_BASE_Y_HH;
    MINT32 ANR_LUMA_SLANT_Y_HH_low    = rLowerANR.actyhh2.bits.ANR_LUMA_SLANT_Y_HH;
    MINT32 ANR_LUMA_BLD_TH_Y_HH_low   = rLowerANR.actyhh2.bits.ANR_LUMA_BLD_TH_Y_HH;
    MINT32 ANR_PTY_L_GAIN_low         = rLowerANR.ylad2.bits.ANR_PTY_L_GAIN;
    MINT32 ANR_PTY_H_GAIN_low         = rLowerANR.ylad2.bits.ANR_PTY_H_GAIN;
    MINT32 ANR_BLEND_TH_L_low         = rLowerANR.ylad2.bits.ANR_BLEND_TH_L;
    MINT32 ANR_BLEND_TH_H_low         = rLowerANR.ylad2.bits.ANR_BLEND_TH_H;
    MINT32 ANR_HI_LMT1_low            = rLowerANR.ylad3.bits.ANR_HI_LMT1;
    MINT32 ANR_HI_LMT2_low            = rLowerANR.ylad3.bits.ANR_HI_LMT2;
    MINT32 ANR_LO_LMT1_low            = rLowerANR.ylad3.bits.ANR_LO_LMT1;
    MINT32 ANR_LO_LMT2_low            = rLowerANR.ylad3.bits.ANR_LO_LMT2;
    MINT32 ANR_Y_ACT_CEN_OFT_low      = rLowerANR.act4.bits.ANR_Y_ACT_CEN_OFT;
    MINT32 ANR_Y_ACT_CEN_GAIN_low     = rLowerANR.act4.bits.ANR_Y_ACT_CEN_GAIN;
    MINT32 ANR_Y_ACT_CEN_TH_low       = rLowerANR.act4.bits.ANR_Y_ACT_CEN_TH;
    MINT32 ANR_Y_DIR_low              = rLowerANR.con1.bits.ANR_Y_DIR;
    MINT32 ANR_Y_DIR_GAIN_low         = rLowerANR.ydir.bits.ANR_Y_DIR_GAIN;
    MINT32 ANR_Y_DIR_VER_W_TH_low     = rLowerANR.ydir.bits.ANR_Y_DIR_VER_W_TH;
    MINT32 ANR_Y_DIR_VER_W_SL_low     = rLowerANR.ydir.bits.ANR_Y_DIR_VER_W_SL;
    MINT32 ANR_Y_DIR_DIAG_W_TH_low    = rLowerANR.ydir.bits.ANR_Y_DIR_DIAG_W_TH;
    MINT32 ANR_Y_DIR_DIAG_W_SL_low    = rLowerANR.ydir.bits.ANR_Y_DIR_DIAG_W_SL;

    //    ====================================================================================================================================
    //    Get Upper ISO setting
    //    ====================================================================================================================================
    // CAM_ANR_CON1 CAM+A20H
    MINT32 ANR_ENC_high           = rUpperANR.con1.bits.ANR_ENC;
    MINT32 ANR_ENY_high           = rUpperANR.con1.bits.ANR_ENY;
    MINT32 ANR_SCALE_MODE_high    = rUpperANR.con1.bits.ANR_SCALE_MODE;
    MINT32 ANR_MEDIAN_EN_high     = rUpperANR.con1.bits.ANR_MEDIAN_EN;
    MINT32 ANR_LCE_LINK_high      = rUpperANR.con1.bits.ANR_LCE_LINK;
    MINT32 ANR_TABLE_EN_high      = rUpperANR.con1.bits.ANR_TABLE_EN;
    MINT32 ANR_TBL_PRC_high       = rUpperANR.con1.bits.ANR_TBL_PRC;

    // CAM_ANR_CON2 CAM+A24H
    MINT32 ANR_IMPL_MODE_high     = rUpperANR.con2.bits.ANR_IMPL_MODE;
    MINT32 ANR_C_MED_EN_high      = rUpperANR.con2.bits.ANR_C_MED_EN;
    MINT32 ANR_C_SM_EDGE_high     = rUpperANR.con2.bits.ANR_C_SM_EDGE;
    MINT32 ANR_FLT_C_high         = rUpperANR.con2.bits.ANR_FLT_C;
    MINT32 ANR_LR_high            = rUpperANR.con2.bits.ANR_LR;
    MINT32 ANR_ALPHA_high         = rUpperANR.con2.bits.ANR_ALPHA;

    // CAM_ANR_YAD1 CAM+A2CH
    MINT32 ANR_CEN_GAIN_LO_TH_high= rUpperANR.yad1.bits.ANR_CEN_GAIN_LO_TH;
    MINT32 ANR_CEN_GAIN_HI_TH_high= rUpperANR.yad1.bits.ANR_CEN_GAIN_HI_TH;
    MINT32 ANR_K_LO_TH_high       = rUpperANR.yad1.bits.ANR_K_LO_TH;
    MINT32 ANR_K_HI_TH_high       = rUpperANR.yad1.bits.ANR_K_HI_TH;
    MINT32 ANR_K_TH_C_high        = rUpperANR.yad1.bits.ANR_K_TH_C;

    // CAM_ANR_YAD2 CAM+A30H
    MINT32 ANR_PTY_VGAIN_high     = rUpperANR.yad2.bits.ANR_PTY_VGAIN;
    MINT32 ANR_PTY_GAIN_TH_high   = rUpperANR.yad2.bits.ANR_PTY_GAIN_TH;

    // CAM_ANR_Y4LUT1 CAM+A34H
    MINT32 ANR_Y_CPX1_high        = rUpperANR.y4lut1.bits.ANR_Y_CPX1;
    MINT32 ANR_Y_CPX2_high        = rUpperANR.y4lut1.bits.ANR_Y_CPX2;
    MINT32 ANR_Y_CPX3_high        = rUpperANR.y4lut1.bits.ANR_Y_CPX3;

    // CAM_ANR_Y4LUT2 CAM+A38H
    MINT32 ANR_Y_SCALE_CPY0_high  = rUpperANR.y4lut2.bits.ANR_Y_SCALE_CPY0;
    MINT32 ANR_Y_SCALE_CPY1_high  = rUpperANR.y4lut2.bits.ANR_Y_SCALE_CPY1;
    MINT32 ANR_Y_SCALE_CPY2_high  = rUpperANR.y4lut2.bits.ANR_Y_SCALE_CPY2;
    MINT32 ANR_Y_SCALE_CPY3_high  = rUpperANR.y4lut2.bits.ANR_Y_SCALE_CPY3;

    // CAM_ANR_Y4LUT3 CAM+A3CH
    MINT32 ANR_Y_SCALE_SP0_high   = rUpperANR.y4lut3.bits.ANR_Y_SCALE_SP0;
    MINT32 ANR_Y_SCALE_SP1_high   = rUpperANR.y4lut3.bits.ANR_Y_SCALE_SP1;
    MINT32 ANR_Y_SCALE_SP2_high   = rUpperANR.y4lut3.bits.ANR_Y_SCALE_SP2;
    MINT32 ANR_Y_SCALE_SP3_high   = rUpperANR.y4lut3.bits.ANR_Y_SCALE_SP3;

    // CAM_ANR_C4LUT1
    MINT32 ANR_C_CPX1_high        = rUpperANR.c4lut1.bits.ANR_C_CPX1;
    MINT32 ANR_C_CPX2_high        = rUpperANR.c4lut1.bits.ANR_C_CPX2;
    MINT32 ANR_C_CPX3_high        = rUpperANR.c4lut1.bits.ANR_C_CPX3;

    // CAM_ANR_C4LUT2
    MINT32 ANR_C_SCALE_CPY0_high  = rUpperANR.c4lut2.bits.ANR_C_SCALE_CPY0;
    MINT32 ANR_C_SCALE_CPY1_high  = rUpperANR.c4lut2.bits.ANR_C_SCALE_CPY1;
    MINT32 ANR_C_SCALE_CPY2_high  = rUpperANR.c4lut2.bits.ANR_C_SCALE_CPY2;
    MINT32 ANR_C_SCALE_CPY3_high  = rUpperANR.c4lut2.bits.ANR_C_SCALE_CPY3;

    // CAM_ANR_C4LUT3
    MINT32 ANR_C_SCALE_SP0_high   = rUpperANR.c4lut3.bits.ANR_C_SCALE_SP0;
    MINT32 ANR_C_SCALE_SP1_high   = rUpperANR.c4lut3.bits.ANR_C_SCALE_SP1;
    MINT32 ANR_C_SCALE_SP2_high   = rUpperANR.c4lut3.bits.ANR_C_SCALE_SP2;
    MINT32 ANR_C_SCALE_SP3_high   = rUpperANR.c4lut3.bits.ANR_C_SCALE_SP3;

    // CAM_ANR_A4LUT2
    MINT32 ANR_Y_ACT_CPY0_high    = rUpperANR.a4lut2.bits.ANR_Y_ACT_CPY0;
    MINT32 ANR_Y_ACT_CPY1_high    = rUpperANR.a4lut2.bits.ANR_Y_ACT_CPY1;
    MINT32 ANR_Y_ACT_CPY2_high    = rUpperANR.a4lut2.bits.ANR_Y_ACT_CPY2;
    MINT32 ANR_Y_ACT_CPY3_high    = rUpperANR.a4lut2.bits.ANR_Y_ACT_CPY3;

    // CAM_ANR_A4LUT3
    MINT32 ANR_Y_ACT_SP0_high     = rUpperANR.a4lut3.bits.ANR_Y_ACT_SP0;
    MINT32 ANR_Y_ACT_SP1_high     = rUpperANR.a4lut3.bits.ANR_Y_ACT_SP1;
    MINT32 ANR_Y_ACT_SP2_high     = rUpperANR.a4lut3.bits.ANR_Y_ACT_SP2;
    MINT32 ANR_Y_ACT_SP3_high     = rUpperANR.a4lut3.bits.ANR_Y_ACT_SP3;

    // CAM_ANR_L4LUT1
    MINT32 ANR_LCE_X1_high        = rUpperANR.l4lut1.bits.ANR_LCE_X1;
    MINT32 ANR_LCE_X2_high        = rUpperANR.l4lut1.bits.ANR_LCE_X2;
    MINT32 ANR_LCE_X3_high        = rUpperANR.l4lut1.bits.ANR_LCE_X3;

    // CAM_ANR_L4LUT2
    MINT32 ANR_LCE_GAIN0_high     = rUpperANR.l4lut2.bits.ANR_LCE_GAIN0;
    MINT32 ANR_LCE_GAIN1_high     = rUpperANR.l4lut2.bits.ANR_LCE_GAIN1;
    MINT32 ANR_LCE_GAIN2_high     = rUpperANR.l4lut2.bits.ANR_LCE_GAIN2;
    MINT32 ANR_LCE_GAIN3_high     = rUpperANR.l4lut2.bits.ANR_LCE_GAIN3;

    // CAM_ANR_L4LUT3
    MINT32 ANR_LCE_SP0_high       = rUpperANR.l4lut3.bits.ANR_LCE_SP0;
    MINT32 ANR_LCE_SP1_high       = rUpperANR.l4lut3.bits.ANR_LCE_SP1;
    MINT32 ANR_LCE_SP2_high       = rUpperANR.l4lut3.bits.ANR_LCE_SP2;
    MINT32 ANR_LCE_SP3_high       = rUpperANR.l4lut3.bits.ANR_LCE_SP3;

    // CAM_ANR_PTY CAM+A40H
    MINT32 ANR_PTY1_high          = rUpperANR.pty.bits.ANR_PTY1;
    MINT32 ANR_PTY2_high          = rUpperANR.pty.bits.ANR_PTY2;
    MINT32 ANR_PTY3_high          = rUpperANR.pty.bits.ANR_PTY3;
    MINT32 ANR_PTY4_high          = rUpperANR.pty.bits.ANR_PTY4;

    // CAM_ANR_CAD CAM+A44H
    MINT32 ANR_PTC_VGAIN_high     = rUpperANR.cad.bits.ANR_PTC_VGAIN;
    MINT32 ANR_PTC_GAIN_TH_high   = rUpperANR.cad.bits.ANR_PTC_GAIN_TH;
    MINT32 ANR_C_L_DIFF_TH_high   = rUpperANR.cad.bits.ANR_C_L_DIFF_TH;

    // CAM_ANR_PTC CAM+A48H
    MINT32 ANR_PTC1_high          = rUpperANR.ptc.bits.ANR_PTC1;
    MINT32 ANR_PTC2_high          = rUpperANR.ptc.bits.ANR_PTC2;
    MINT32 ANR_PTC3_high          = rUpperANR.ptc.bits.ANR_PTC3;
    MINT32 ANR_PTC4_high          = rUpperANR.ptc.bits.ANR_PTC4;

    // CAM_ANR_LCE CAM+A4CH
    MINT32 ANR_LCE_C_GAIN_high    = rUpperANR.lce.bits.ANR_LCE_C_GAIN;
    MINT32 ANR_LCE_SCALE_GAIN_high= rUpperANR.lce.bits.ANR_LCE_SCALE_GAIN;
    MINT32 ANR_LM_WT_high         = rUpperANR.lce.bits.ANR_LM_WT;

    // CAM_ANR_MED1
    MINT32 ANR_COR_TH_high        = rUpperANR.med1.bits.ANR_COR_TH;
    MINT32 ANR_COR_SL_high        = rUpperANR.med1.bits.ANR_COR_SL;
    MINT32 ANR_MCD_TH_high        = rUpperANR.med1.bits.ANR_MCD_TH;
    MINT32 ANR_MCD_SL_high        = rUpperANR.med1.bits.ANR_MCD_SL;
    MINT32 ANR_LCL_TH_high        = rUpperANR.med1.bits.ANR_LCL_TH;

    // CAM_ANR_MED2
    MINT32 ANR_LCL_SL_high        = rUpperANR.med2.bits.ANR_LCL_SL;
    MINT32 ANR_LCL_LV_high        = rUpperANR.med2.bits.ANR_LCL_LV;
    MINT32 ANR_SCL_TH_high        = rUpperANR.med2.bits.ANR_SCL_TH;
    MINT32 ANR_SCL_SL_high        = rUpperANR.med2.bits.ANR_SCL_SL;
    MINT32 ANR_SCL_LV_high        = rUpperANR.med2.bits.ANR_SCL_LV;

    // CAM_ANR_MED3
    MINT32 ANR_NCL_TH_high        = rUpperANR.med3.bits.ANR_NCL_TH;
    MINT32 ANR_NCL_SL_high        = rUpperANR.med3.bits.ANR_NCL_SL;
    MINT32 ANR_NCL_LV_high        = rUpperANR.med3.bits.ANR_NCL_LV;
    MINT32 ANR_VAR_high           = rUpperANR.med3.bits.ANR_VAR;
    MINT32 ANR_Y0_high            = rUpperANR.med3.bits.ANR_Y0;

    // CAM_ANR_MED4
    MINT32 ANR_Y1_high            = rUpperANR.med4.bits.ANR_Y1;
    MINT32 ANR_Y2_high            = rUpperANR.med4.bits.ANR_Y2;
    MINT32 ANR_Y3_high            = rUpperANR.med4.bits.ANR_Y3;
    MINT32 ANR_Y4_high            = rUpperANR.med4.bits.ANR_Y4;

    // CAM_ANR_HP1 CAM+A54H
    MINT32 ANR_HP_A_high          = rUpperANR.hp1.bits.ANR_HP_A;
    MINT32 ANR_HP_B_high          = rUpperANR.hp1.bits.ANR_HP_B;
    MINT32 ANR_HP_C_high          = rUpperANR.hp1.bits.ANR_HP_C;
    MINT32 ANR_HP_D_high          = rUpperANR.hp1.bits.ANR_HP_D;
    MINT32 ANR_HP_E_high          = rUpperANR.hp1.bits.ANR_HP_E;

    // CAM_ANR_HP2 CAM+A58H
    MINT32 ANR_HP_S1_high         = rUpperANR.hp2.bits.ANR_HP_S1;
    MINT32 ANR_HP_S2_high         = rUpperANR.hp2.bits.ANR_HP_S2;
    MINT32 ANR_HP_X1_high         = rUpperANR.hp2.bits.ANR_HP_X1;
    MINT32 ANR_HP_F_high          = rUpperANR.hp2.bits.ANR_HP_F;

    // CAM_ANR_HP3 CAM+A5CH
    MINT32 ANR_HP_Y_GAIN_CLIP_high= rUpperANR.hp3.bits.ANR_HP_Y_GAIN_CLIP;
    MINT32 ANR_HP_Y_SP_high       = rUpperANR.hp3.bits.ANR_HP_Y_SP;
    MINT32 ANR_HP_Y_LO_high       = rUpperANR.hp3.bits.ANR_HP_Y_LO;
    MINT32 ANR_HP_CLIP_high       = rUpperANR.hp3.bits.ANR_HP_CLIP;

    // CAM_ANR_ACT1 CAM+A64H
    MINT32 ANR_ACT_LCE_GAIN_high  = rUpperANR.act1.bits.ANR_ACT_LCE_GAIN;
    MINT32 ANR_ACT_SCALE_OFT_high = rUpperANR.act1.bits.ANR_ACT_SCALE_OFT;
    MINT32 ANR_ACT_SCALE_GAIN_high= rUpperANR.act1.bits.ANR_ACT_SCALE_GAIN;
    MINT32 ANR_ACT_DIF_GAIN_high  = rUpperANR.act1.bits.ANR_ACT_DIF_GAIN;
    MINT32 ANR_ACT_DIF_LO_TH_high = rUpperANR.act1.bits.ANR_ACT_DIF_LO_TH;

    // CAM_ANR_ACT2 CAM+A64H
    MINT32 ANR_ACT_SIZE_GAIN_high = rUpperANR.act2.bits.ANR_ACT_SIZE_GAIN;
    MINT32 ANR_ACT_SIZE_LO_TH_high= rUpperANR.act2.bits.ANR_ACT_SIZE_LO_TH;
    MINT32 ANR_COR_TH1_high       = rUpperANR.act2.bits.ANR_COR_TH1;
    MINT32 ANR_COR_SL1_high       = rUpperANR.act2.bits.ANR_COR_SL1;

    // CAM_ANR_ACT3 CAM+A64H
    MINT32 ANR_COR_ACT_TH_high    = rUpperANR.act3.bits.ANR_COR_ACT_TH;
    MINT32 ANR_COR_ACT_SL1_high   = rUpperANR.act3.bits.ANR_COR_ACT_SL1;
    MINT32 ANR_COR_ACT_SL2_high   = rUpperANR.act3.bits.ANR_COR_ACT_SL2;

    // CAM_ANR_ACTC CAM+A64H
    MINT32 ANR_ACT_TH_C_high      = rUpperANR.actc.bits.ANR_ACT_TH_C;
    MINT32 ANR_ACT_BLD_BASE_C_high= rUpperANR.actc.bits.ANR_ACT_BLD_BASE_C;
    MINT32 ANR_ACT_SLANT_C_high   = rUpperANR.actc.bits.ANR_ACT_SLANT_C;
    MINT32 ANR_ACT_BLD_TH_C_high  = rUpperANR.actc.bits.ANR_ACT_BLD_TH_C;

    // DIP_X_ANR_ACTYL CAM+A60H
    MINT32 ANR_ACT_TH_Y_L_high = rUpperANR.actyl.bits.ANR_ACT_TH_Y_L;
    MINT32 ANR_ACT_BLD_BASE_Y_L_high = rUpperANR.actyl.bits.ANR_ACT_BLD_BASE_Y_L;
    MINT32 ANR_ACT_SLANT_Y_L_high = rUpperANR.actyl.bits.ANR_ACT_SLANT_Y_L;
    MINT32 ANR_ACT_BLD_TH_Y_L_high = rUpperANR.actyl.bits.ANR_ACT_BLD_TH_Y_L;

    // DIP_X_ANR_YLAD CAM+A60H
    MINT32 ANR_CEN_GAIN_LO_TH_LPF_high = rUpperANR.ylad.bits.ANR_CEN_GAIN_LO_TH_LPF;
    MINT32 ANR_CEN_GAIN_HI_TH_LPF_high = rUpperANR.ylad.bits.ANR_CEN_GAIN_HI_TH_LPF;
    MINT32 ANR_K_LMT_high = rUpperANR.ylad.bits.ANR_K_LMT;
    MINT32 ANR_K_LPF_high = rUpperANR.ylad.bits.ANR_K_LPF;

    // DIP_X_ANR_PTYL CAM+A60H
    MINT32 ANR_PTY1_LPF_high = rUpperANR.ptyl.bits.ANR_PTY1_LPF;
    MINT32 ANR_PTY2_LPF_high = rUpperANR.ptyl.bits.ANR_PTY2_LPF;
    MINT32 ANR_PTY3_LPF_high = rUpperANR.ptyl.bits.ANR_PTY3_LPF;
    MINT32 ANR_PTY4_LPF_high = rUpperANR.ptyl.bits.ANR_PTY4_LPF;

    // DIP_X_ANR_LCOEF CAM+A60H
    MINT32 ANR_COEF_A_high = rUpperANR.lcoef.bits.ANR_COEF_A;
    MINT32 ANR_COEF_B_high = rUpperANR.lcoef.bits.ANR_COEF_B;
    MINT32 ANR_COEF_C_high = rUpperANR.lcoef.bits.ANR_COEF_C;
    MINT32 ANR_COEF_D_high = rUpperANR.lcoef.bits.ANR_COEF_D;

    // Olympus
    MINT32 ANR_C_SM_EDGE_TH_high       = rUpperANR.con2.bits.ANR_C_SM_EDGE_TH;
    MINT32 ANR_K_HPF_high              = rUpperANR.ylad.bits.ANR_K_HPF;
    MINT32 ANR_ACT_DIF_HI_TH_high      = rUpperANR.act2.bits.ANR_ACT_DIF_HI_TH;
    MINT32 ANR_ACT_TH_Y_HL_high        = rUpperANR.actyhl.bits.ANR_ACT_TH_Y_HL;
    MINT32 ANR_ACT_BLD_BASE_Y_HL_high  = rUpperANR.actyhl.bits.ANR_ACT_BLD_BASE_Y_HL;
    MINT32 ANR_ACT_SLANT_Y_HL_high     = rUpperANR.actyhl.bits.ANR_ACT_SLANT_Y_HL;
    MINT32 ANR_ACT_BLD_TH_Y_HL_high    = rUpperANR.actyhl.bits.ANR_ACT_BLD_TH_Y_HL;
    MINT32 ANR_ACT_TH_Y_HH_high        = rUpperANR.actyhh.bits.ANR_ACT_TH_Y_HH;
    MINT32 ANR_ACT_BLD_BASE_Y_HH_high  = rUpperANR.actyhh.bits.ANR_ACT_BLD_BASE_Y_HH;
    MINT32 ANR_ACT_SLANT_Y_HH_high     = rUpperANR.actyhh.bits.ANR_ACT_SLANT_Y_HH;
    MINT32 ANR_ACT_BLD_TH_Y_HH_high    = rUpperANR.actyhh.bits.ANR_ACT_BLD_TH_Y_HH;
    MINT32 ANR_LUMA_TH_Y_L_high        = rUpperANR.actyl2.bits.ANR_LUMA_TH_Y_L;
    MINT32 ANR_LUMA_BLD_BASE_Y_L_high  = rUpperANR.actyl2.bits.ANR_LUMA_BLD_BASE_Y_L;
    MINT32 ANR_LUMA_SLANT_Y_L_high     = rUpperANR.actyl2.bits.ANR_LUMA_SLANT_Y_L;
    MINT32 ANR_LUMA_BLD_TH_Y_L_high    = rUpperANR.actyl2.bits.ANR_LUMA_BLD_TH_Y_L;
    MINT32 ANR_LUMA_TH_Y_HL_high       = rUpperANR.actyhl2.bits.ANR_LUMA_TH_Y_HL;
    MINT32 ANR_LUMA_BLD_BASE_Y_HL_high = rUpperANR.actyhl2.bits.ANR_LUMA_BLD_BASE_Y_HL;
    MINT32 ANR_LUMA_SLANT_Y_HL_high    = rUpperANR.actyhl2.bits.ANR_LUMA_SLANT_Y_HL;
    MINT32 ANR_LUMA_BLD_TH_Y_HL_high   = rUpperANR.actyhl2.bits.ANR_LUMA_BLD_TH_Y_HL;
    MINT32 ANR_LUMA_TH_Y_HH_high       = rUpperANR.actyhh2.bits.ANR_LUMA_TH_Y_HH;
    MINT32 ANR_LUMA_BLD_BASE_Y_HH_high = rUpperANR.actyhh2.bits.ANR_LUMA_BLD_BASE_Y_HH;
    MINT32 ANR_LUMA_SLANT_Y_HH_high    = rUpperANR.actyhh2.bits.ANR_LUMA_SLANT_Y_HH;
    MINT32 ANR_LUMA_BLD_TH_Y_HH_high   = rUpperANR.actyhh2.bits.ANR_LUMA_BLD_TH_Y_HH;
    MINT32 ANR_PTY_L_GAIN_high         = rUpperANR.ylad2.bits.ANR_PTY_L_GAIN;
    MINT32 ANR_PTY_H_GAIN_high         = rUpperANR.ylad2.bits.ANR_PTY_H_GAIN;
    MINT32 ANR_BLEND_TH_L_high         = rUpperANR.ylad2.bits.ANR_BLEND_TH_L;
    MINT32 ANR_BLEND_TH_H_high         = rUpperANR.ylad2.bits.ANR_BLEND_TH_H;
    MINT32 ANR_HI_LMT1_high            = rUpperANR.ylad3.bits.ANR_HI_LMT1;
    MINT32 ANR_HI_LMT2_high            = rUpperANR.ylad3.bits.ANR_HI_LMT2;
    MINT32 ANR_LO_LMT1_high            = rUpperANR.ylad3.bits.ANR_LO_LMT1;
    MINT32 ANR_LO_LMT2_high            = rUpperANR.ylad3.bits.ANR_LO_LMT2;
    MINT32 ANR_Y_ACT_CEN_OFT_high      = rUpperANR.act4.bits.ANR_Y_ACT_CEN_OFT;
    MINT32 ANR_Y_ACT_CEN_GAIN_high     = rUpperANR.act4.bits.ANR_Y_ACT_CEN_GAIN;
    MINT32 ANR_Y_ACT_CEN_TH_high       = rUpperANR.act4.bits.ANR_Y_ACT_CEN_TH;
    MINT32 ANR_Y_DIR_high              = rUpperANR.con1.bits.ANR_Y_DIR;
    MINT32 ANR_Y_DIR_GAIN_high         = rUpperANR.ydir.bits.ANR_Y_DIR_GAIN;
    MINT32 ANR_Y_DIR_VER_W_TH_high     = rUpperANR.ydir.bits.ANR_Y_DIR_VER_W_TH;
    MINT32 ANR_Y_DIR_VER_W_SL_high     = rUpperANR.ydir.bits.ANR_Y_DIR_VER_W_SL;
    MINT32 ANR_Y_DIR_DIAG_W_TH_high    = rUpperANR.ydir.bits.ANR_Y_DIR_DIAG_W_TH;
    MINT32 ANR_Y_DIR_DIAG_W_SL_high    = rUpperANR.ydir.bits.ANR_Y_DIR_DIAG_W_SL;

    //    ====================================================================================================================================
    //    Interpolating reg declaration
    //    ====================================================================================================================================
    // CAM_ANR_CON1 CAM+A20H
    MINT32 ANR_ENC;
    MINT32 ANR_ENY;
    MINT32 ANR_SCALE_MODE;
    MINT32 ANR_MEDIAN_EN;
    MINT32 ANR_LCE_LINK;
    MINT32 ANR_TABLE_EN;
    MINT32 ANR_TBL_PRC;

    // CAM_ANR_CON2 CAM+A24H
    MINT32 ANR_IMPL_MODE;
    MINT32 ANR_C_MED_EN;
    MINT32 ANR_C_SM_EDGE;
    MINT32 ANR_FLT_C;
    MINT32 ANR_LR;
    MINT32 ANR_ALPHA;

    // CAM_ANR_YAD1 CAM+A2CH
    MINT32 ANR_CEN_GAIN_LO_TH;
    MINT32 ANR_CEN_GAIN_HI_TH;
    MINT32 ANR_K_LO_TH;
    MINT32 ANR_K_HI_TH;
    MINT32 ANR_K_TH_C;

    // CAM_ANR_YAD2 CAM+A30H
    MINT32 ANR_PTY_VGAIN;
    MINT32 ANR_PTY_GAIN_TH;

    // CAM_ANR_Y4LUT1 CAM+A34H
    MINT32 ANR_Y_CPX1;
    MINT32 ANR_Y_CPX2;
    MINT32 ANR_Y_CPX3;

    // CAM_ANR_Y4LUT2 CAM+A38H
    MINT32 ANR_Y_SCALE_CPY0;
    MINT32 ANR_Y_SCALE_CPY1;
    MINT32 ANR_Y_SCALE_CPY2;
    MINT32 ANR_Y_SCALE_CPY3;

    // CAM_ANR_Y4LUT3 CAM+A3CH
    MINT32 ANR_Y_SCALE_SP0;
    MINT32 ANR_Y_SCALE_SP1;
    MINT32 ANR_Y_SCALE_SP2;
    MINT32 ANR_Y_SCALE_SP3;

    // CAM_ANR_C4LUT1
    MINT32 ANR_C_CPX1;
    MINT32 ANR_C_CPX2;
    MINT32 ANR_C_CPX3;

    // CAM_ANR_C4LUT2
    MINT32 ANR_C_SCALE_CPY0;
    MINT32 ANR_C_SCALE_CPY1;
    MINT32 ANR_C_SCALE_CPY2;
    MINT32 ANR_C_SCALE_CPY3;

    // CAM_ANR_C4LUT3
    MINT32 ANR_C_SCALE_SP0;
    MINT32 ANR_C_SCALE_SP1;
    MINT32 ANR_C_SCALE_SP2;
    MINT32 ANR_C_SCALE_SP3;

    // CAM_ANR_A4LUT2
    MINT32 ANR_Y_ACT_CPY0;
    MINT32 ANR_Y_ACT_CPY1;
    MINT32 ANR_Y_ACT_CPY2;
    MINT32 ANR_Y_ACT_CPY3;

    // CAM_ANR_A4LUT3
    MINT32 ANR_Y_ACT_SP0;
    MINT32 ANR_Y_ACT_SP1;
    MINT32 ANR_Y_ACT_SP2;
    MINT32 ANR_Y_ACT_SP3;

    // CAM_ANR_L4LUT1
    MINT32 ANR_LCE_X1;
    MINT32 ANR_LCE_X2;
    MINT32 ANR_LCE_X3;

    // CAM_ANR_L4LUT2
    MINT32 ANR_LCE_GAIN0;
    MINT32 ANR_LCE_GAIN1;
    MINT32 ANR_LCE_GAIN2;
    MINT32 ANR_LCE_GAIN3;

    // CAM_ANR_L4LUT3
    MINT32 ANR_LCE_SP0;
    MINT32 ANR_LCE_SP1;
    MINT32 ANR_LCE_SP2;
    MINT32 ANR_LCE_SP3;

    // CAM_ANR_PTY CAM+A40H
    MINT32 ANR_PTY1;
    MINT32 ANR_PTY2;
    MINT32 ANR_PTY3;
    MINT32 ANR_PTY4;

    // CAM_ANR_CAD CAM+A44H
    MINT32 ANR_PTC_VGAIN;
    MINT32 ANR_PTC_GAIN_TH;
    MINT32 ANR_C_L_DIFF_TH;

    // CAM_ANR_PTC CAM+A48H
    MINT32 ANR_PTC1;
    MINT32 ANR_PTC2;
    MINT32 ANR_PTC3;
    MINT32 ANR_PTC4;

    // CAM_ANR_LCE CAM+A4CH
    MINT32 ANR_LCE_C_GAIN;
    MINT32 ANR_LCE_SCALE_GAIN;
    MINT32 ANR_LM_WT;

    // CAM_ANR_MED1
    MINT32 ANR_COR_TH;
    MINT32 ANR_COR_SL;
    MINT32 ANR_MCD_TH;
    MINT32 ANR_MCD_SL;
    MINT32 ANR_LCL_TH;

    // CAM_ANR_MED2
    MINT32 ANR_LCL_SL;
    MINT32 ANR_LCL_LV;
    MINT32 ANR_SCL_TH;
    MINT32 ANR_SCL_SL;
    MINT32 ANR_SCL_LV;

    // CAM_ANR_MED3
    MINT32 ANR_NCL_TH;
    MINT32 ANR_NCL_SL;
    MINT32 ANR_NCL_LV;
    MINT32 ANR_VAR;
    MINT32 ANR_Y0;

    // CAM_ANR_MED4
    MINT32 ANR_Y1;
    MINT32 ANR_Y2;
    MINT32 ANR_Y3;
    MINT32 ANR_Y4;

    // CAM_ANR_HP1 CAM+A54H
    MINT32 ANR_HP_A;
    MINT32 ANR_HP_B;
    MINT32 ANR_HP_C;
    MINT32 ANR_HP_D;
    MINT32 ANR_HP_E;

    // CAM_ANR_HP2 CAM+A58H
    MINT32 ANR_HP_S1;
    MINT32 ANR_HP_S2;
    MINT32 ANR_HP_X1;
    MINT32 ANR_HP_F;

    // CAM_ANR_HP3 CAM+A5CH
    MINT32 ANR_HP_Y_GAIN_CLIP;
    MINT32 ANR_HP_Y_SP;
    MINT32 ANR_HP_Y_LO;
    MINT32 ANR_HP_CLIP;

    // CAM_ANR_ACT1 CAM+A64H
    MINT32 ANR_ACT_LCE_GAIN;
    MINT32 ANR_ACT_SCALE_OFT;
    MINT32 ANR_ACT_SCALE_GAIN;
    MINT32 ANR_ACT_DIF_GAIN;
    MINT32 ANR_ACT_DIF_LO_TH;

    // CAM_ANR_ACT2 CAM+A64H
    MINT32 ANR_ACT_SIZE_GAIN;
    MINT32 ANR_ACT_SIZE_LO_TH;
    MINT32 ANR_COR_TH1;
    MINT32 ANR_COR_SL1;

    // CAM_ANR_ACT3 CAM+A64H
    MINT32 ANR_COR_ACT_TH;
    MINT32 ANR_COR_ACT_SL1;
    MINT32 ANR_COR_ACT_SL2;

    // CAM_ANR_ACTC CAM+A64H
    MINT32 ANR_ACT_TH_C;
    MINT32 ANR_ACT_BLD_BASE_C;
    MINT32 ANR_ACT_SLANT_C;
    MINT32 ANR_ACT_BLD_TH_C;

    // DIP_X_ANR_ACTYL CAM+A60H
    MINT32 ANR_ACT_TH_Y_L;
    MINT32 ANR_ACT_BLD_BASE_Y_L;
    MINT32 ANR_ACT_SLANT_Y_L;
    MINT32 ANR_ACT_BLD_TH_Y_L;

    // DIP_X_ANR_YLAD CAM+A60H
    MINT32 ANR_CEN_GAIN_LO_TH_LPF;
    MINT32 ANR_CEN_GAIN_HI_TH_LPF;
    MINT32 ANR_K_LMT;
    MINT32 ANR_K_LPF;

    // DIP_X_ANR_PTYL CAM+A60H
    MINT32 ANR_PTY1_LPF;
    MINT32 ANR_PTY2_LPF;
    MINT32 ANR_PTY3_LPF;
    MINT32 ANR_PTY4_LPF;

    // DIP_X_ANR_LCOEF CAM+A60H
    MINT32 ANR_COEF_A;
    MINT32 ANR_COEF_B;
    MINT32 ANR_COEF_C;
    MINT32 ANR_COEF_D;

	//Olympus
	MINT32 ANR_C_SM_EDGE_TH;
	MINT32 ANR_K_HPF;
	MINT32 ANR_ACT_DIF_HI_TH;
	MINT32 ANR_ACT_TH_Y_HL;
    MINT32 ANR_ACT_BLD_BASE_Y_HL;
    MINT32 ANR_ACT_SLANT_Y_HL;
    MINT32 ANR_ACT_BLD_TH_Y_HL;
	MINT32 ANR_ACT_TH_Y_HH;
    MINT32 ANR_ACT_BLD_BASE_Y_HH;
    MINT32 ANR_ACT_SLANT_Y_HH;
    MINT32 ANR_ACT_BLD_TH_Y_HH;
	MINT32 ANR_LUMA_TH_Y_L;
    MINT32 ANR_LUMA_BLD_BASE_Y_L;
    MINT32 ANR_LUMA_SLANT_Y_L;
    MINT32 ANR_LUMA_BLD_TH_Y_L;
	MINT32 ANR_LUMA_TH_Y_HL;
    MINT32 ANR_LUMA_BLD_BASE_Y_HL;
    MINT32 ANR_LUMA_SLANT_Y_HL;
    MINT32 ANR_LUMA_BLD_TH_Y_HL;
	MINT32 ANR_LUMA_TH_Y_HH;
    MINT32 ANR_LUMA_BLD_BASE_Y_HH;
    MINT32 ANR_LUMA_SLANT_Y_HH;
    MINT32 ANR_LUMA_BLD_TH_Y_HH;
	MINT32 ANR_PTY_L_GAIN;
	MINT32 ANR_PTY_H_GAIN;
	MINT32 ANR_BLEND_TH_L;
	MINT32 ANR_BLEND_TH_H;
	MINT32 ANR_HI_LMT1;
	MINT32 ANR_HI_LMT2;
	MINT32 ANR_LO_LMT1;
	MINT32 ANR_LO_LMT2;
	MINT32 ANR_Y_ACT_CEN_OFT;
	MINT32 ANR_Y_ACT_CEN_GAIN;
	MINT32 ANR_Y_ACT_CEN_TH;
	MINT32 ANR_Y_DIR;
	MINT32 ANR_Y_DIR_GAIN;
	MINT32 ANR_Y_DIR_VER_W_TH;
	MINT32 ANR_Y_DIR_VER_W_SL;
	MINT32 ANR_Y_DIR_DIAG_W_TH;
	MINT32 ANR_Y_DIR_DIAG_W_SL;
	
    //    ====================================================================================================================================
    //    Negative Conversion
    //    ====================================================================================================================================
    ANR_Y_SCALE_SP0_low = (ANR_Y_SCALE_SP0_low > 15) ? (ANR_Y_SCALE_SP0_low - 32) : ANR_Y_SCALE_SP0_low;
    ANR_Y_SCALE_SP1_low = (ANR_Y_SCALE_SP1_low > 15) ? (ANR_Y_SCALE_SP1_low - 32) : ANR_Y_SCALE_SP1_low;
    ANR_Y_SCALE_SP2_low = (ANR_Y_SCALE_SP2_low > 15) ? (ANR_Y_SCALE_SP2_low - 32) : ANR_Y_SCALE_SP2_low;
    ANR_Y_SCALE_SP3_low = (ANR_Y_SCALE_SP3_low > 15) ? (ANR_Y_SCALE_SP3_low - 32) : ANR_Y_SCALE_SP3_low;
    ANR_C_SCALE_SP0_low = (ANR_C_SCALE_SP0_low > 15) ? (ANR_C_SCALE_SP0_low - 32) : ANR_C_SCALE_SP0_low;
    ANR_C_SCALE_SP1_low = (ANR_C_SCALE_SP1_low > 15) ? (ANR_C_SCALE_SP1_low - 32) : ANR_C_SCALE_SP1_low;
    ANR_C_SCALE_SP2_low = (ANR_C_SCALE_SP2_low > 15) ? (ANR_C_SCALE_SP2_low - 32) : ANR_C_SCALE_SP2_low;
    ANR_C_SCALE_SP3_low = (ANR_C_SCALE_SP3_low > 15) ? (ANR_C_SCALE_SP3_low - 32) : ANR_C_SCALE_SP3_low;
    ANR_Y_ACT_SP0_low   = (ANR_Y_ACT_SP0_low > 31) ? (ANR_Y_ACT_SP0_low - 64) : ANR_Y_ACT_SP0_low;
    ANR_Y_ACT_SP1_low   = (ANR_Y_ACT_SP1_low > 31) ? (ANR_Y_ACT_SP1_low - 64) : ANR_Y_ACT_SP1_low;
    ANR_Y_ACT_SP2_low   = (ANR_Y_ACT_SP2_low > 31) ? (ANR_Y_ACT_SP2_low - 64) : ANR_Y_ACT_SP2_low;
    ANR_Y_ACT_SP3_low   = (ANR_Y_ACT_SP3_low > 31) ? (ANR_Y_ACT_SP3_low - 64) : ANR_Y_ACT_SP3_low;
    ANR_LCE_SP0_low     = (ANR_LCE_SP0_low > 31) ? (ANR_LCE_SP0_low - 64) : ANR_LCE_SP0_low;
    ANR_LCE_SP1_low     = (ANR_LCE_SP1_low > 31) ? (ANR_LCE_SP1_low - 64) : ANR_LCE_SP1_low;
    ANR_LCE_SP2_low     = (ANR_LCE_SP2_low > 31) ? (ANR_LCE_SP2_low - 64) : ANR_LCE_SP2_low;
    ANR_LCE_SP3_low     = (ANR_LCE_SP3_low > 31) ? (ANR_LCE_SP3_low - 64) : ANR_LCE_SP3_low;
    ANR_HP_B_low        = (ANR_HP_B_low > 31) ? (ANR_HP_B_low - 64) : ANR_HP_B_low;
    ANR_HP_C_low        = (ANR_HP_C_low > 15) ? (ANR_HP_C_low - 32) : ANR_HP_C_low;
    ANR_HP_D_low        = (ANR_HP_D_low > 7)  ? (ANR_HP_D_low - 16) : ANR_HP_D_low;
    ANR_HP_E_low        = (ANR_HP_E_low > 7)  ? (ANR_HP_E_low - 16) : ANR_HP_E_low;
    ANR_HP_F_low        = (ANR_HP_F_low > 3)  ? (ANR_HP_F_low - 8)  : ANR_HP_F_low;

    ANR_Y_SCALE_SP0_high = (ANR_Y_SCALE_SP0_high > 15) ? (ANR_Y_SCALE_SP0_high - 32) : ANR_Y_SCALE_SP0_high;
    ANR_Y_SCALE_SP1_high = (ANR_Y_SCALE_SP1_high > 15) ? (ANR_Y_SCALE_SP1_high - 32) : ANR_Y_SCALE_SP1_high;
    ANR_Y_SCALE_SP2_high = (ANR_Y_SCALE_SP2_high > 15) ? (ANR_Y_SCALE_SP2_high - 32) : ANR_Y_SCALE_SP2_high;
    ANR_Y_SCALE_SP3_high = (ANR_Y_SCALE_SP3_high > 15) ? (ANR_Y_SCALE_SP3_high - 32) : ANR_Y_SCALE_SP3_high;
    ANR_C_SCALE_SP0_high = (ANR_C_SCALE_SP0_high > 15) ? (ANR_C_SCALE_SP0_high - 32) : ANR_C_SCALE_SP0_high;
    ANR_C_SCALE_SP1_high = (ANR_C_SCALE_SP1_high > 15) ? (ANR_C_SCALE_SP1_high - 32) : ANR_C_SCALE_SP1_high;
    ANR_C_SCALE_SP2_high = (ANR_C_SCALE_SP2_high > 15) ? (ANR_C_SCALE_SP2_high - 32) : ANR_C_SCALE_SP2_high;
    ANR_C_SCALE_SP3_high = (ANR_C_SCALE_SP3_high > 15) ? (ANR_C_SCALE_SP3_high - 32) : ANR_C_SCALE_SP3_high;
    ANR_Y_ACT_SP0_high   = (ANR_Y_ACT_SP0_high > 31) ? (ANR_Y_ACT_SP0_high - 64) : ANR_Y_ACT_SP0_high;
    ANR_Y_ACT_SP1_high   = (ANR_Y_ACT_SP1_high > 31) ? (ANR_Y_ACT_SP1_high - 64) : ANR_Y_ACT_SP1_high;
    ANR_Y_ACT_SP2_high   = (ANR_Y_ACT_SP2_high > 31) ? (ANR_Y_ACT_SP2_high - 64) : ANR_Y_ACT_SP2_high;
    ANR_Y_ACT_SP3_high   = (ANR_Y_ACT_SP3_high > 31) ? (ANR_Y_ACT_SP3_high - 64) : ANR_Y_ACT_SP3_high;
    ANR_LCE_SP0_high     = (ANR_LCE_SP0_high > 31) ? (ANR_LCE_SP0_high - 64) : ANR_LCE_SP0_high;
    ANR_LCE_SP1_high     = (ANR_LCE_SP1_high > 31) ? (ANR_LCE_SP1_high - 64) : ANR_LCE_SP1_high;
    ANR_LCE_SP2_high     = (ANR_LCE_SP2_high > 31) ? (ANR_LCE_SP2_high - 64) : ANR_LCE_SP2_high;
    ANR_LCE_SP3_high     = (ANR_LCE_SP3_high > 31) ? (ANR_LCE_SP3_high - 64) : ANR_LCE_SP3_high;
    ANR_HP_B_high        = (ANR_HP_B_high > 31) ? (ANR_HP_B_high - 64) : ANR_HP_B_high;
    ANR_HP_C_high        = (ANR_HP_C_high > 15) ? (ANR_HP_C_high - 32) : ANR_HP_C_high;
    ANR_HP_D_high        = (ANR_HP_D_high > 7)  ? (ANR_HP_D_high - 16) : ANR_HP_D_high;
    ANR_HP_E_high        = (ANR_HP_E_high > 7)  ? (ANR_HP_E_high - 16) : ANR_HP_E_high;
    ANR_HP_F_high        = (ANR_HP_F_high > 3)  ? (ANR_HP_F_high - 8)  : ANR_HP_F_high;

    //    ====================================================================================================================================
    //    Start Parameter Interpolation
    //    ====================================================================================================================================
    MINT32 ISO = u4RealISO;
    MINT32 ISO_low = u4LowerISO;
    MINT32 ISO_high = u4UpperISO;

    if(ANR_ENC_low==ANR_ENC_high && ANR_ENY_low==ANR_ENY_high)
    {
        //    Registers that can not be interpolated
        ANR_ENC            = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ENC_low, ANR_ENC_high, 1);
        ANR_ENY            = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ENY_low, ANR_ENY_high, 1);
        ANR_TABLE_EN       = InterParam_NR(ISO, ISO_low, ISO_high, ANR_TABLE_EN_low, ANR_TABLE_EN_high, 1);
        ANR_TBL_PRC        = InterParam_NR(ISO, ISO_low, ISO_high, ANR_TBL_PRC_low, ANR_TBL_PRC_high, 1);
        ANR_IMPL_MODE      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_IMPL_MODE_low, ANR_IMPL_MODE_high, 1);
        ANR_C_MED_EN       = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_MED_EN_low, ANR_C_MED_EN_high, 1);
        ANR_C_SM_EDGE      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_SM_EDGE_low, ANR_C_SM_EDGE_high, 1);
        ANR_FLT_C          = InterParam_NR(ISO, ISO_low, ISO_high, ANR_FLT_C_low, ANR_FLT_C_high, 1);
        ANR_LR             = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LR_low, ANR_LR_high, 1);
        ANR_HP_A           = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_A_low, ANR_HP_A_high, 1);
        ANR_HP_B           = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_B_low, ANR_HP_B_high, 1);
        ANR_HP_C           = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_C_low, ANR_HP_C_high, 1);
        ANR_HP_D           = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_D_low, ANR_HP_D_high, 1);
        ANR_HP_E           = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_E_low, ANR_HP_E_high, 1);
        ANR_HP_F           = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_F_low, ANR_HP_F_high, 1);
        ANR_HP_S1          = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_S1_low, ANR_HP_S1_high, 1);
        ANR_HP_S2          = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_S2_low, ANR_HP_S2_high, 1);
        ANR_HP_X1          = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_X1_low, ANR_HP_X1_high, 1);
        ANR_HP_Y_GAIN_CLIP = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_Y_GAIN_CLIP_low, ANR_HP_Y_GAIN_CLIP_high, 1);
        ANR_HP_Y_SP        = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_Y_SP_low, ANR_HP_Y_SP_high, 1);
        ANR_HP_Y_LO        = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_Y_LO_low, ANR_HP_Y_LO_high, 1);
        ANR_HP_CLIP        = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_CLIP_low, ANR_HP_CLIP_high, 1);

        //    Registers that can be interpolated
        ANR_ALPHA          = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ALPHA_low           , ANR_ALPHA_high, 0);
        ANR_CEN_GAIN_LO_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_CEN_GAIN_LO_TH_low  , ANR_CEN_GAIN_LO_TH_high, 0);
        ANR_CEN_GAIN_HI_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_CEN_GAIN_HI_TH_low  , ANR_CEN_GAIN_HI_TH_high, 0);
        ANR_PTY_VGAIN      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTY_VGAIN_low       , ANR_PTY_VGAIN_high, 0);
        ANR_PTY_GAIN_TH    = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTY_GAIN_TH_low     , ANR_PTY_GAIN_TH_high, 0);
        ANR_Y_CPX1         = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_CPX1_low          , ANR_Y_CPX1_high, 0);
        ANR_Y_CPX2         = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_CPX2_low          , ANR_Y_CPX2_high, 0);
        ANR_Y_CPX3         = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_CPX3_low          , ANR_Y_CPX3_high, 0);
        ANR_Y_SCALE_CPY0   = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_SCALE_CPY0_low    , ANR_Y_SCALE_CPY0_high, 0);
        ANR_Y_SCALE_CPY1   = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_SCALE_CPY1_low    , ANR_Y_SCALE_CPY1_high, 0);
        ANR_Y_SCALE_CPY2   = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_SCALE_CPY2_low    , ANR_Y_SCALE_CPY2_high, 0);
        ANR_Y_SCALE_CPY3   = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_SCALE_CPY3_low    , ANR_Y_SCALE_CPY3_high, 0);
        ANR_C_CPX1         = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_CPX1_low          , ANR_C_CPX1_high, 0);
        ANR_C_CPX2         = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_CPX2_low          , ANR_C_CPX2_high, 0);
        ANR_C_CPX3         = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_CPX3_low          , ANR_C_CPX3_high, 0);
        ANR_C_SCALE_CPY0   = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_SCALE_CPY0_low    , ANR_C_SCALE_CPY0_high, 0);
        ANR_C_SCALE_CPY1   = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_SCALE_CPY1_low    , ANR_C_SCALE_CPY1_high, 0);
        ANR_C_SCALE_CPY2   = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_SCALE_CPY2_low    , ANR_C_SCALE_CPY2_high, 0);
        ANR_C_SCALE_CPY3   = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_SCALE_CPY3_low    , ANR_C_SCALE_CPY3_high, 0);
        ANR_Y_ACT_CPY0     = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_ACT_CPY0_low      , ANR_Y_ACT_CPY0_high, 0);
        ANR_Y_ACT_CPY1     = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_ACT_CPY1_low      , ANR_Y_ACT_CPY1_high, 0);
        ANR_Y_ACT_CPY2     = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_ACT_CPY2_low      , ANR_Y_ACT_CPY2_high, 0);
        ANR_Y_ACT_CPY3     = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_ACT_CPY3_low      , ANR_Y_ACT_CPY3_high, 0);
        ANR_PTY1           = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTY1_low, ANR_PTY1_high, 0);
        ANR_PTY2           = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTY2_low, ANR_PTY2_high, 0);
        ANR_PTY3           = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTY3_low, ANR_PTY3_high, 0);
        ANR_PTY4           = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTY4_low, ANR_PTY4_high, 0);
        ANR_PTC_VGAIN      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTC_VGAIN_low, ANR_PTC_VGAIN_high, 0);
        ANR_PTC_GAIN_TH    = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTC_GAIN_TH_low, ANR_PTC_GAIN_TH_high, 0);
        ANR_C_L_DIFF_TH    = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_L_DIFF_TH_low, ANR_C_L_DIFF_TH_high, 0);
        ANR_PTC1           = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTC1_low, ANR_PTC1_high, 0);
        ANR_PTC2           = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTC2_low, ANR_PTC2_high, 0);
        ANR_PTC3           = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTC3_low, ANR_PTC3_high, 0);
        ANR_PTC4           = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTC4_low, ANR_PTC4_high, 0);
        ANR_LM_WT              = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LM_WT_low, ANR_LM_WT_high, 0);
        ANR_ACT_LCE_GAIN   = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_LCE_GAIN_low, ANR_ACT_LCE_GAIN_high, 0);
        ANR_ACT_SCALE_OFT  = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_SCALE_OFT_low, ANR_ACT_SCALE_OFT_high, 0);
        ANR_ACT_SCALE_GAIN = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_SCALE_GAIN_low, ANR_ACT_SCALE_GAIN_high, 0);
        ANR_ACT_DIF_GAIN   = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_DIF_GAIN_low, ANR_ACT_DIF_GAIN_high, 0);
        ANR_ACT_DIF_LO_TH  = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_DIF_LO_TH_low, ANR_ACT_DIF_LO_TH_high, 0);
        ANR_ACT_SIZE_GAIN  = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_SIZE_GAIN_low, ANR_ACT_SIZE_GAIN_high, 0);
        ANR_ACT_SIZE_LO_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_SIZE_LO_TH_low, ANR_ACT_SIZE_LO_TH_high, 0);
        ANR_COR_TH1        = InterParam_NR(ISO, ISO_low, ISO_high, ANR_COR_TH1_low, ANR_COR_TH1_high, 0);
        ANR_COR_SL1        = InterParam_NR(ISO, ISO_low, ISO_high, ANR_COR_SL1_low, ANR_COR_SL1_high, 0);
        ANR_COR_ACT_TH     = InterParam_NR(ISO, ISO_low, ISO_high, ANR_COR_ACT_TH_low, ANR_COR_ACT_TH_high, 0);
        ANR_COR_ACT_SL1    = InterParam_NR(ISO, ISO_low, ISO_high, ANR_COR_ACT_SL1_low, ANR_COR_ACT_SL1_high, 0);
        ANR_COR_ACT_SL2    = InterParam_NR(ISO, ISO_low, ISO_high, ANR_COR_ACT_SL2_low, ANR_COR_ACT_SL2_high, 0);
        ANR_ACT_TH_C       = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_TH_C_low, ANR_ACT_TH_C_high, 0);
        ANR_ACT_BLD_BASE_C = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_BLD_BASE_C_low, ANR_ACT_BLD_BASE_C_high, 0);
        ANR_ACT_SLANT_C    = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_SLANT_C_low, ANR_ACT_SLANT_C_high, 0);
        ANR_ACT_BLD_TH_C   = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_BLD_TH_C_low, ANR_ACT_BLD_TH_C_high, 0);
        ANR_ACT_TH_Y_L         = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_TH_Y_L_low, ANR_ACT_TH_Y_L_high, 0);
        ANR_ACT_BLD_BASE_Y_L   = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_BLD_BASE_Y_L_low, ANR_ACT_BLD_BASE_Y_L_high, 0);
        ANR_ACT_SLANT_Y_L      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_SLANT_Y_L_low, ANR_ACT_SLANT_Y_L_high, 0);
        ANR_ACT_BLD_TH_Y_L     = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_BLD_TH_Y_L_low, ANR_ACT_BLD_TH_Y_L_high, 0);
        ANR_CEN_GAIN_LO_TH_LPF = InterParam_NR(ISO, ISO_low, ISO_high, ANR_CEN_GAIN_LO_TH_LPF_low, ANR_CEN_GAIN_LO_TH_LPF_high, 0);
        ANR_CEN_GAIN_HI_TH_LPF = InterParam_NR(ISO, ISO_low, ISO_high, ANR_CEN_GAIN_HI_TH_LPF_low, ANR_CEN_GAIN_HI_TH_LPF_high, 0);
        ANR_K_LMT              = InterParam_NR(ISO, ISO_low, ISO_high, ANR_K_LMT_low, ANR_K_LMT_high, 0);
        ANR_K_LPF              = InterParam_NR(ISO, ISO_low, ISO_high, ANR_K_LPF_low, ANR_K_LPF_high, 0);
        ANR_PTY1_LPF           = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTY1_LPF_low, ANR_PTY1_LPF_high, 0);
        ANR_PTY2_LPF           = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTY2_LPF_low, ANR_PTY2_LPF_high, 0);
        ANR_PTY3_LPF           = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTY3_LPF_low, ANR_PTY3_LPF_high, 0);
        ANR_PTY4_LPF           = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTY4_LPF_low, ANR_PTY4_LPF_high, 0);
        ANR_COEF_A             = InterParam_NR(ISO, ISO_low, ISO_high, ANR_COEF_A_low, ANR_COEF_A_high, 0);
        ANR_COEF_B             = InterParam_NR(ISO, ISO_low, ISO_high, ANR_COEF_B_low, ANR_COEF_B_high, 0);
        ANR_COEF_C             = InterParam_NR(ISO, ISO_low, ISO_high, ANR_COEF_C_low, ANR_COEF_C_high, 0);
        ANR_COEF_D             = InterParam_NR(ISO, ISO_low, ISO_high, ANR_COEF_D_low, ANR_COEF_D_high, 0);

		//Olympus
		ANR_C_SM_EDGE_TH       = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_SM_EDGE_TH_low, ANR_C_SM_EDGE_TH_high, 0);
        ANR_K_HPF              = InterParam_NR(ISO, ISO_low, ISO_high, ANR_K_HPF_low, ANR_K_HPF_high, 0);
		ANR_ACT_DIF_HI_TH      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_DIF_HI_TH_low, ANR_ACT_DIF_HI_TH_high, 0);
		ANR_ACT_TH_Y_HL        = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_TH_Y_HL_low, ANR_ACT_TH_Y_HL_high, 0);
        ANR_ACT_BLD_BASE_Y_HL  = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_BLD_BASE_Y_HL_low, ANR_ACT_BLD_BASE_Y_HL_high, 0);
        ANR_ACT_SLANT_Y_HL     = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_SLANT_Y_HL_low, ANR_ACT_SLANT_Y_HL_high, 0);
        ANR_ACT_BLD_TH_Y_HL    = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_BLD_TH_Y_HL_low, ANR_ACT_BLD_TH_Y_HL_high, 0);
		ANR_ACT_TH_Y_HH        = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_TH_Y_HH_low, ANR_ACT_TH_Y_HH_high, 0);
        ANR_ACT_BLD_BASE_Y_HH  = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_BLD_BASE_Y_HH_low, ANR_ACT_BLD_BASE_Y_HH_high, 0);
        ANR_ACT_SLANT_Y_HH     = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_SLANT_Y_HH_low, ANR_ACT_SLANT_Y_HH_high, 0);
        ANR_ACT_BLD_TH_Y_HH    = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_BLD_TH_Y_HH_low, ANR_ACT_BLD_TH_Y_HH_high, 0);
		ANR_LUMA_TH_Y_L        = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LUMA_TH_Y_L_low, ANR_LUMA_TH_Y_L_high, 0);
        ANR_LUMA_BLD_BASE_Y_L  = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LUMA_BLD_BASE_Y_L_low, ANR_LUMA_BLD_BASE_Y_L_high, 0);
        ANR_LUMA_SLANT_Y_L     = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LUMA_SLANT_Y_L_low, ANR_LUMA_SLANT_Y_L_high, 0);
        ANR_LUMA_BLD_TH_Y_L    = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LUMA_BLD_TH_Y_L_low, ANR_LUMA_BLD_TH_Y_L_high, 0);
		ANR_LUMA_TH_Y_HL       = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_TH_Y_HL_low, ANR_LUMA_TH_Y_HL_high, 0);
        ANR_LUMA_BLD_BASE_Y_HL = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_BLD_BASE_Y_HL_low, ANR_LUMA_BLD_BASE_Y_HL_high, 0);
        ANR_LUMA_SLANT_Y_HL    = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_SLANT_Y_HL_low, ANR_LUMA_SLANT_Y_HL_high, 0);
        ANR_LUMA_BLD_TH_Y_HL   = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_BLD_TH_Y_HL_low, ANR_LUMA_BLD_TH_Y_HL_high, 0);
		ANR_LUMA_TH_Y_HH       = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_TH_Y_HH_low, ANR_LUMA_TH_Y_HH_high, 0);
        ANR_LUMA_BLD_BASE_Y_HH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_BLD_BASE_Y_HH_low, ANR_LUMA_BLD_BASE_Y_HH_high, 0);
        ANR_LUMA_SLANT_Y_HH    = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_SLANT_Y_HH_low, ANR_LUMA_SLANT_Y_HH_high, 0);
        ANR_LUMA_BLD_TH_Y_HH   = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_BLD_TH_Y_HH_low, ANR_LUMA_BLD_TH_Y_HH_high, 0);
		ANR_PTY_L_GAIN         = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTY_L_GAIN_low, ANR_PTY_L_GAIN_high, 0);
		ANR_PTY_H_GAIN         = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTY_H_GAIN_low, ANR_PTY_H_GAIN_high, 0);
		ANR_BLEND_TH_L         = InterParam_NR(ISO, ISO_low, ISO_high, ANR_BLEND_TH_L_low, ANR_BLEND_TH_L_high, 0);
		ANR_BLEND_TH_H         = InterParam_NR(ISO, ISO_low, ISO_high, ANR_BLEND_TH_H_low, ANR_BLEND_TH_H_high, 0);
		ANR_HI_LMT1            = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HI_LMT1_low, ANR_HI_LMT1_high, 0);
		ANR_HI_LMT2            = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HI_LMT2_low, ANR_HI_LMT2_high, 0);
		ANR_LO_LMT1            = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LO_LMT1_low, ANR_LO_LMT1_high, 0);
		ANR_LO_LMT2            = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LO_LMT2_low, ANR_LO_LMT2_high, 0);
		ANR_Y_ACT_CEN_OFT      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_ACT_CEN_OFT_low, ANR_Y_ACT_CEN_OFT_high, 0);
		ANR_Y_ACT_CEN_GAIN     = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_ACT_CEN_GAIN_low, ANR_Y_ACT_CEN_GAIN_high, 0);
		ANR_Y_ACT_CEN_TH       = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_ACT_CEN_TH_low, ANR_Y_ACT_CEN_TH_high, 0);

        // ANR_SCALE_MODE - bit 1 for y, bit 0 for c
        MINT32 scale_mode_y_low  = (ANR_SCALE_MODE_low  >> 1);
        MINT32 scale_mode_c_low  = (ANR_SCALE_MODE_low  & 1);
        MINT32 scale_mode_y_high = (ANR_SCALE_MODE_high >> 1);
        MINT32 scale_mode_c_high = (ANR_SCALE_MODE_high & 1);
        MINT32 scale_mode_y, scale_mode_c;

        if(scale_mode_y_low == scale_mode_y_high)
        {
            scale_mode_y   = scale_mode_y_low;
            ANR_K_LO_TH      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_K_LO_TH_low, ANR_K_LO_TH_high, 0);
            ANR_K_HI_TH      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_K_HI_TH_low, ANR_K_HI_TH_high, 0);
        }
        else
        {
            if(1 == scale_mode_y_low)
            {
                ANR_K_LO_TH_low = ANR_K_LO_TH_low * 2;
                ANR_K_HI_TH_low = ANR_K_HI_TH_low * 2;
            }

            if(1 == scale_mode_y_high)
            {
                ANR_K_LO_TH_high = ANR_K_LO_TH_high*2;
                ANR_K_HI_TH_high = ANR_K_HI_TH_high*2;
            }

            ANR_K_LO_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_K_LO_TH_low, ANR_K_LO_TH_high, 0);
            ANR_K_HI_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_K_HI_TH_low, ANR_K_HI_TH_high, 0);

            if(ANR_K_HI_TH > 9)
            {
                scale_mode_y = (scale_mode_y_high > scale_mode_y_low) ? scale_mode_y_high : scale_mode_y_low;
                ANR_K_LO_TH /=2;
                ANR_K_HI_TH /=2;
            }
            else
            {
                scale_mode_y = InterParam_NR(ISO, ISO_low, ISO_high, scale_mode_y_low, scale_mode_y_high, 1);

                if(1 == scale_mode_y)
                {
                    ANR_K_LO_TH /=2;
                    ANR_K_HI_TH /=2;
                }
            }
        }

        if(scale_mode_c_low == scale_mode_c_high)
        {
            scale_mode_c = scale_mode_c_low;
            ANR_K_TH_C   = InterParam_NR(ISO, ISO_low, ISO_high, ANR_K_TH_C_low, ANR_K_TH_C_high, 0);
        }
        else
        {
            if(1 == scale_mode_c_low)
            {
                ANR_K_TH_C_low = ANR_K_TH_C_low * 2;
            }

            if(1 == scale_mode_c_high)
            {
                ANR_K_TH_C_high = ANR_K_TH_C_high * 2;
            }

            ANR_K_TH_C  = InterParam_NR(ISO, ISO_low, ISO_high, ANR_K_TH_C_low, ANR_K_TH_C_high, 0);

            if(ANR_K_TH_C > 8)
            {
                scale_mode_c = (scale_mode_c_high > scale_mode_c_low) ? scale_mode_c_high : scale_mode_c_low;
                ANR_K_TH_C /=2;
            }
            else
            {
                scale_mode_c = InterParam_NR(ISO, ISO_low, ISO_high, scale_mode_c_low, scale_mode_c_high, 1);

                if(1 == scale_mode_c)
                {
                    ANR_K_TH_C /=2;
                }
            }
        }
        ANR_SCALE_MODE = (scale_mode_y << 1) + scale_mode_c;

        // ANR_LCE_LINK
        if(0 == ANR_LCE_LINK_low && 1 == ANR_LCE_LINK_high)
        {
            ANR_LCE_LINK_low  = ANR_LCE_LINK_high;
            ANR_LCE_X1_low    = ANR_LCE_X1_high;
            ANR_LCE_X2_low    = ANR_LCE_X2_high;
            ANR_LCE_X3_low    = ANR_LCE_X3_high;
            ANR_LCE_GAIN0_low = 16;
            ANR_LCE_GAIN1_low = 16;
            ANR_LCE_GAIN2_low = 16;
            ANR_LCE_GAIN3_low = 16;
            ANR_LCE_SP0_low   = 0;
            ANR_LCE_SP1_low   = 0;
            ANR_LCE_SP2_low   = 0;
            ANR_LCE_SP3_low   = 0;
            ANR_LCE_C_GAIN_low = 4;
            ANR_LCE_SCALE_GAIN_low = 0;
        }
        
        if(0 == ANR_LCE_LINK_high && 1 == ANR_LCE_LINK_low)
        {
            ANR_LCE_LINK_high  = ANR_LCE_LINK_low;
            ANR_LCE_X1_high    = ANR_LCE_X1_low;
            ANR_LCE_X2_high    = ANR_LCE_X2_low;
            ANR_LCE_X3_high    = ANR_LCE_X3_low;
            ANR_LCE_GAIN0_high = 16;
            ANR_LCE_GAIN1_high = 16;
            ANR_LCE_GAIN2_high = 16;
            ANR_LCE_GAIN3_high = 16;
            ANR_LCE_SP0_high   = 0;
            ANR_LCE_SP1_high   = 0;
            ANR_LCE_SP2_high   = 0;
            ANR_LCE_SP3_high   = 0;
            ANR_LCE_C_GAIN_high = 4;
            ANR_LCE_SCALE_GAIN_high = 0;
        }

        ANR_LCE_LINK       = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_LINK_low      , ANR_LCE_LINK_high, 0);
        ANR_LCE_X1         = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_X1_low        , ANR_LCE_X1_high, 0);
        ANR_LCE_X2         = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_X2_low        , ANR_LCE_X2_high, 0);
        ANR_LCE_X3         = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_X3_low        , ANR_LCE_X3_high, 0);
        ANR_LCE_GAIN0      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_GAIN0_low     , ANR_LCE_GAIN0_high, 0);
        ANR_LCE_GAIN1      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_GAIN1_low     , ANR_LCE_GAIN1_high, 0);
        ANR_LCE_GAIN2      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_GAIN2_low     , ANR_LCE_GAIN2_high, 0);
        ANR_LCE_GAIN3      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_GAIN3_low     , ANR_LCE_GAIN3_high, 0);
        ANR_LCE_C_GAIN     = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_C_GAIN_low    , ANR_LCE_C_GAIN_high, 0);
        ANR_LCE_SCALE_GAIN = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_SCALE_GAIN_low, ANR_LCE_SCALE_GAIN_high, 0);

        // SP Calculation
            ANR_Y_SCALE_SP0    = CalculateLUTSP(         0, ANR_Y_CPX1, ANR_Y_SCALE_CPY0, ANR_Y_SCALE_CPY1, 128, 15, -15);
            ANR_Y_SCALE_SP1    = CalculateLUTSP(ANR_Y_CPX1, ANR_Y_CPX2, ANR_Y_SCALE_CPY1, ANR_Y_SCALE_CPY2, 128, 15, -15);
            ANR_Y_SCALE_SP2    = CalculateLUTSP(ANR_Y_CPX2, ANR_Y_CPX3, ANR_Y_SCALE_CPY2, ANR_Y_SCALE_CPY3, 128, 15, -15);
            ANR_Y_SCALE_SP3    = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_SCALE_SP3_low     , ANR_Y_SCALE_SP3_high, 0);
            ANR_C_SCALE_SP0    = CalculateLUTSP(         0, ANR_C_CPX1, ANR_C_SCALE_CPY0, ANR_C_SCALE_CPY1, 128, 15, -15);
            ANR_C_SCALE_SP1    = CalculateLUTSP(ANR_C_CPX1, ANR_C_CPX2, ANR_C_SCALE_CPY1, ANR_C_SCALE_CPY2, 128, 15, -15);
            ANR_C_SCALE_SP2    = CalculateLUTSP(ANR_C_CPX2, ANR_C_CPX3, ANR_C_SCALE_CPY2, ANR_C_SCALE_CPY3, 128, 15, -15);
            ANR_C_SCALE_SP3    = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_SCALE_SP3_low     , ANR_C_SCALE_SP3_high, 0);
            ANR_Y_ACT_SP0      = CalculateLUTSP(         0, ANR_Y_CPX1, ANR_Y_ACT_CPY0  , ANR_Y_ACT_CPY1  , 32, 31, -31);
            ANR_Y_ACT_SP1      = CalculateLUTSP(ANR_Y_CPX1, ANR_Y_CPX2, ANR_Y_ACT_CPY1  , ANR_Y_ACT_CPY2  , 32, 31, -31);
            ANR_Y_ACT_SP2      = CalculateLUTSP(ANR_Y_CPX2, ANR_Y_CPX3, ANR_Y_ACT_CPY2  , ANR_Y_ACT_CPY3  , 32, 31, -31);
        ANR_Y_ACT_SP3      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_ACT_SP3_low       , ANR_Y_ACT_SP3_high, 0);
        ANR_LCE_SP0        = CalculateLUTSP(         0, ANR_LCE_X1, ANR_LCE_GAIN0   , ANR_LCE_GAIN1   , 128, 31, -31);
        ANR_LCE_SP1        = CalculateLUTSP(ANR_LCE_X1, ANR_LCE_X2, ANR_LCE_GAIN1   , ANR_LCE_GAIN2   , 128, 31, -31);
        ANR_LCE_SP2        = CalculateLUTSP(ANR_LCE_X2, ANR_LCE_X3, ANR_LCE_GAIN2   , ANR_LCE_GAIN3   , 128, 31, -31);
            ANR_LCE_SP3        = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_SP3_low       , ANR_LCE_SP3_high, 0);

        // ANR_MEDIAN_EN
        if (ANR_MEDIAN_EN_low == ANR_MEDIAN_EN_high)
        {
            ANR_MEDIAN_EN   = InterParam_NR(ISO, ISO_low, ISO_high, ANR_MEDIAN_EN_low, ANR_MEDIAN_EN_high, 0);
            ANR_COR_TH      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_COR_TH_low   , ANR_COR_TH_high, 0);
            ANR_COR_SL      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_COR_SL_low   , ANR_COR_SL_high, 0);
            ANR_MCD_TH      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_MCD_TH_low   , ANR_MCD_TH_high, 0);
            ANR_MCD_SL      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_MCD_SL_low   , ANR_MCD_SL_high, 0);
            ANR_LCL_TH      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCL_TH_low   , ANR_LCL_TH_high, 0);
            ANR_LCL_SL      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCL_SL_low   , ANR_LCL_SL_high, 0);
            ANR_LCL_LV      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCL_LV_low   , ANR_LCL_LV_high, 0);
            ANR_SCL_TH      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_SCL_TH_low   , ANR_SCL_TH_high, 0);
            ANR_SCL_SL      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_SCL_SL_low   , ANR_SCL_SL_high, 0);
            ANR_SCL_LV      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_SCL_LV_low   , ANR_SCL_LV_high, 0);
            ANR_NCL_TH      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_NCL_TH_low   , ANR_NCL_TH_high, 0);
            ANR_NCL_SL      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_NCL_SL_low   , ANR_NCL_SL_high, 0);
            ANR_NCL_LV      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_NCL_LV_low   , ANR_NCL_LV_high, 0);
            ANR_VAR         = InterParam_NR(ISO, ISO_low, ISO_high, ANR_VAR_low      , ANR_VAR_high, 0);
            ANR_Y0          = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y0_low       , ANR_Y0_high, 0);
            ANR_Y1          = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y1_low       , ANR_Y1_high, 0);
            ANR_Y2          = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y2_low       , ANR_Y2_high, 0);
            ANR_Y3          = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y3_low       , ANR_Y3_high, 0);
            ANR_Y4          = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y4_low       , ANR_Y4_high, 0);
        }
        else
        {
            ANR_MEDIAN_EN   = InterParam_NR(ISO, ISO_low, ISO_high, ANR_MEDIAN_EN_low, ANR_MEDIAN_EN_high, 1);
            ANR_COR_TH      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_COR_TH_low   , ANR_COR_TH_high, 1);
            ANR_COR_SL      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_COR_SL_low   , ANR_COR_SL_high, 1);
            ANR_MCD_TH      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_MCD_TH_low   , ANR_MCD_TH_high, 1);
            ANR_MCD_SL      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_MCD_SL_low   , ANR_MCD_SL_high, 1);
            ANR_LCL_TH      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCL_TH_low   , ANR_LCL_TH_high, 1);
            ANR_LCL_SL      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCL_SL_low   , ANR_LCL_SL_high, 1);
            ANR_LCL_LV      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCL_LV_low   , ANR_LCL_LV_high, 1);
            ANR_SCL_TH      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_SCL_TH_low   , ANR_SCL_TH_high, 1);
            ANR_SCL_SL      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_SCL_SL_low   , ANR_SCL_SL_high, 1);
            ANR_SCL_LV      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_SCL_LV_low   , ANR_SCL_LV_high, 1);
            ANR_NCL_TH      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_NCL_TH_low   , ANR_NCL_TH_high, 1);
            ANR_NCL_SL      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_NCL_SL_low   , ANR_NCL_SL_high, 1);
            ANR_NCL_LV      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_NCL_LV_low   , ANR_NCL_LV_high, 1);
            ANR_VAR         = InterParam_NR(ISO, ISO_low, ISO_high, ANR_VAR_low      , ANR_VAR_high, 1);
            ANR_Y0          = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y0_low       , ANR_Y0_high, 1);
            ANR_Y1          = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y1_low       , ANR_Y1_high, 1);
            ANR_Y2          = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y2_low       , ANR_Y2_high, 1);
            ANR_Y3          = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y3_low       , ANR_Y3_high, 1);
            ANR_Y4          = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y4_low       , ANR_Y4_high, 1);
        }

		// Olympus  ANR_Y_DIR 
        if (ANR_MEDIAN_EN_low == ANR_MEDIAN_EN_high)
        {
            ANR_Y_DIR   = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_DIR_low, ANR_Y_DIR_high, 0);
            ANR_Y_DIR_GAIN      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_DIR_GAIN_low   , ANR_Y_DIR_GAIN_high, 0);
            ANR_Y_DIR_VER_W_TH      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_DIR_VER_W_TH_low   , ANR_Y_DIR_VER_W_TH_high, 0);
            ANR_Y_DIR_VER_W_SL      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_DIR_VER_W_SL_low   , ANR_Y_DIR_VER_W_SL_high, 0);
            ANR_Y_DIR_DIAG_W_TH      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_DIR_DIAG_W_TH_low   , ANR_Y_DIR_DIAG_W_TH_high, 0);
            ANR_Y_DIR_DIAG_W_SL      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_DIR_DIAG_W_SL_low   , ANR_Y_DIR_DIAG_W_SL_high, 0);
        }
        else
        {
            ANR_Y_DIR   = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_DIR_low, ANR_Y_DIR_high, 1);
            ANR_Y_DIR_GAIN      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_DIR_GAIN_low   , ANR_Y_DIR_GAIN_high, 1);
            ANR_Y_DIR_VER_W_TH      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_DIR_VER_W_TH_low   , ANR_Y_DIR_VER_W_TH_high, 1);
            ANR_Y_DIR_VER_W_SL      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_DIR_VER_W_SL_low   , ANR_Y_DIR_VER_W_SL_high, 1);
            ANR_Y_DIR_DIAG_W_TH      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_DIR_DIAG_W_TH_low   , ANR_Y_DIR_DIAG_W_TH_high, 1);
            ANR_Y_DIR_DIAG_W_SL      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_DIR_DIAG_W_SL_low   , ANR_Y_DIR_DIAG_W_SL_high, 1);
        }
    }
    else
    {
        //    Registers that can not be interpolated
        ANR_ENC            = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ENC_low             , ANR_ENC_high, 1);
        ANR_ENY            = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ENY_low             , ANR_ENY_high, 1);
        ANR_TABLE_EN       = InterParam_NR(ISO, ISO_low, ISO_high, ANR_TABLE_EN_low        , ANR_TABLE_EN_high, 1);
        ANR_TBL_PRC        = InterParam_NR(ISO, ISO_low, ISO_high, ANR_TBL_PRC_low         , ANR_TBL_PRC_high, 1);
        ANR_IMPL_MODE      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_IMPL_MODE_low       , ANR_IMPL_MODE_high, 1);
        ANR_C_MED_EN       = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_MED_EN_low        , ANR_C_MED_EN_high, 1);
        ANR_C_SM_EDGE      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_SM_EDGE_low       , ANR_C_SM_EDGE_high, 1);
        ANR_FLT_C          = InterParam_NR(ISO, ISO_low, ISO_high, ANR_FLT_C_low           , ANR_FLT_C_high, 1);
        ANR_LR             = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LR_low              , ANR_LR_high, 1);
        ANR_ALPHA          = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ALPHA_low           , ANR_ALPHA_high, 1);
        ANR_CEN_GAIN_LO_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_CEN_GAIN_LO_TH_low  , ANR_CEN_GAIN_LO_TH_high, 1);
        ANR_CEN_GAIN_HI_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_CEN_GAIN_HI_TH_low  , ANR_CEN_GAIN_HI_TH_high, 1);
        ANR_PTY_VGAIN      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTY_VGAIN_low       , ANR_PTY_VGAIN_high, 1);
        ANR_PTY_GAIN_TH    = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTY_GAIN_TH_low     , ANR_PTY_GAIN_TH_high, 1);
        ANR_Y_CPX1         = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_CPX1_low          , ANR_Y_CPX1_high, 1);
        ANR_Y_CPX2         = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_CPX2_low          , ANR_Y_CPX2_high, 1);
        ANR_Y_CPX3         = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_CPX3_low          , ANR_Y_CPX3_high, 1);
        ANR_Y_SCALE_CPY0   = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_SCALE_CPY0_low    , ANR_Y_SCALE_CPY0_high, 1);
        ANR_Y_SCALE_CPY1   = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_SCALE_CPY1_low    , ANR_Y_SCALE_CPY1_high, 1);
        ANR_Y_SCALE_CPY2   = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_SCALE_CPY2_low    , ANR_Y_SCALE_CPY2_high, 1);
        ANR_Y_SCALE_CPY3   = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_SCALE_CPY3_low    , ANR_Y_SCALE_CPY3_high, 1);
        ANR_Y_SCALE_SP0    = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_SCALE_SP0_low     , ANR_Y_SCALE_SP0_high, 1);
        ANR_Y_SCALE_SP1    = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_SCALE_SP1_low     , ANR_Y_SCALE_SP1_high, 1);
        ANR_Y_SCALE_SP2    = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_SCALE_SP2_low     , ANR_Y_SCALE_SP2_high, 1);
        ANR_Y_SCALE_SP3    = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_SCALE_SP3_low     , ANR_Y_SCALE_SP3_high, 1);
        ANR_C_CPX1         = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_CPX1_low          , ANR_C_CPX1_high, 1);
        ANR_C_CPX2         = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_CPX2_low          , ANR_C_CPX2_high, 1);
        ANR_C_CPX3         = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_CPX3_low          , ANR_C_CPX3_high, 1);
        ANR_C_SCALE_CPY0   = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_SCALE_CPY0_low    , ANR_C_SCALE_CPY0_high, 1);
        ANR_C_SCALE_CPY1   = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_SCALE_CPY1_low    , ANR_C_SCALE_CPY1_high, 1);
        ANR_C_SCALE_CPY2   = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_SCALE_CPY2_low    , ANR_C_SCALE_CPY2_high, 1);
        ANR_C_SCALE_CPY3   = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_SCALE_CPY3_low    , ANR_C_SCALE_CPY3_high, 1);
        ANR_C_SCALE_SP0    = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_SCALE_SP0_low     , ANR_C_SCALE_SP0_high, 1);
        ANR_C_SCALE_SP1    = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_SCALE_SP1_low     , ANR_C_SCALE_SP1_high, 1);
        ANR_C_SCALE_SP2    = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_SCALE_SP2_low     , ANR_C_SCALE_SP2_high, 1);
        ANR_C_SCALE_SP3    = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_SCALE_SP3_low     , ANR_C_SCALE_SP3_high, 1);
        ANR_Y_ACT_CPY0     = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_ACT_CPY0_low      , ANR_Y_ACT_CPY0_high, 1);
        ANR_Y_ACT_CPY1     = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_ACT_CPY1_low      , ANR_Y_ACT_CPY1_high, 1);
        ANR_Y_ACT_CPY2     = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_ACT_CPY2_low      , ANR_Y_ACT_CPY2_high, 1);
        ANR_Y_ACT_CPY3     = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_ACT_CPY3_low      , ANR_Y_ACT_CPY3_high, 1);
        ANR_Y_ACT_SP0      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_ACT_SP0_low       , ANR_Y_ACT_SP0_high, 1);
        ANR_Y_ACT_SP1      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_ACT_SP1_low       , ANR_Y_ACT_SP1_high, 1);
        ANR_Y_ACT_SP2      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_ACT_SP2_low       , ANR_Y_ACT_SP2_high, 1);
        ANR_Y_ACT_SP3      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_ACT_SP3_low       , ANR_Y_ACT_SP3_high, 1);
        ANR_PTY1           = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTY1_low            , ANR_PTY1_high, 1);
        ANR_PTY2           = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTY2_low            , ANR_PTY2_high, 1);
        ANR_PTY3           = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTY3_low            , ANR_PTY3_high, 1);
        ANR_PTY4           = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTY4_low            , ANR_PTY4_high, 1);
        ANR_PTC_VGAIN      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTC_VGAIN_low       , ANR_PTC_VGAIN_high, 1);
        ANR_PTC_GAIN_TH    = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTC_GAIN_TH_low     , ANR_PTC_GAIN_TH_high, 1);
        ANR_C_L_DIFF_TH    = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_L_DIFF_TH_low     , ANR_C_L_DIFF_TH_high, 1);
        ANR_PTC1           = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTC1_low            , ANR_PTC1_high, 1);
        ANR_PTC2           = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTC2_low            , ANR_PTC2_high, 1);
        ANR_PTC3           = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTC3_low            , ANR_PTC3_high, 1);
        ANR_PTC4           = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTC4_low            , ANR_PTC4_high, 1);
        ANR_LM_WT              = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LM_WT_low           , ANR_LM_WT_high, 0);
        ANR_HP_A           = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_A_low            , ANR_HP_A_high, 1);
        ANR_HP_B           = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_B_low            , ANR_HP_B_high, 1);
        ANR_HP_C           = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_C_low            , ANR_HP_C_high, 1);
        ANR_HP_D           = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_D_low            , ANR_HP_D_high, 1);
        ANR_HP_E           = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_E_low            , ANR_HP_E_high, 1);
        ANR_HP_F           = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_F_low            , ANR_HP_F_high, 1);
        ANR_HP_S1          = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_S1_low           , ANR_HP_S1_high, 1);
        ANR_HP_S2          = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_S2_low           , ANR_HP_S2_high, 1);
        ANR_HP_X1          = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_X1_low           , ANR_HP_X1_high, 1);
        ANR_HP_Y_GAIN_CLIP = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_Y_GAIN_CLIP_low  , ANR_HP_Y_GAIN_CLIP_high, 1);
        ANR_HP_Y_SP        = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_Y_SP_low         , ANR_HP_Y_SP_high, 1);
        ANR_HP_Y_LO        = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_Y_LO_low         , ANR_HP_Y_LO_high, 1);
        ANR_HP_CLIP        = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_CLIP_low         , ANR_HP_CLIP_high, 1);
        ANR_ACT_LCE_GAIN   = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_LCE_GAIN_low    , ANR_ACT_LCE_GAIN_high, 1);
        ANR_ACT_SCALE_OFT  = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_SCALE_OFT_low   , ANR_ACT_SCALE_OFT_high, 1);
        ANR_ACT_SCALE_GAIN = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_SCALE_GAIN_low  , ANR_ACT_SCALE_GAIN_high, 1);
        ANR_ACT_DIF_GAIN   = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_DIF_GAIN_low    , ANR_ACT_DIF_GAIN_high, 1);
        ANR_ACT_DIF_LO_TH  = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_DIF_LO_TH_low   , ANR_ACT_DIF_LO_TH_high, 1);
        ANR_ACT_SIZE_GAIN  = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_SIZE_GAIN_low   , ANR_ACT_SIZE_GAIN_high, 1);
        ANR_ACT_SIZE_LO_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_SIZE_LO_TH_low  , ANR_ACT_SIZE_LO_TH_high, 1);
        ANR_COR_TH1        = InterParam_NR(ISO, ISO_low, ISO_high, ANR_COR_TH1_low         , ANR_COR_TH1_high, 1);
        ANR_COR_SL1        = InterParam_NR(ISO, ISO_low, ISO_high, ANR_COR_SL1_low         , ANR_COR_SL1_high, 1);
        ANR_COR_ACT_TH     = InterParam_NR(ISO, ISO_low, ISO_high, ANR_COR_ACT_TH_low      , ANR_COR_ACT_TH_high, 1);
        ANR_COR_ACT_SL1    = InterParam_NR(ISO, ISO_low, ISO_high, ANR_COR_ACT_SL1_low     , ANR_COR_ACT_SL1_high, 1);
        ANR_COR_ACT_SL2    = InterParam_NR(ISO, ISO_low, ISO_high, ANR_COR_ACT_SL2_low     , ANR_COR_ACT_SL2_high, 1);
        ANR_ACT_TH_C       = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_TH_C_low        , ANR_ACT_TH_C_high, 1);
        ANR_ACT_BLD_BASE_C = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_BLD_BASE_C_low  , ANR_ACT_BLD_BASE_C_high, 1);
        ANR_ACT_SLANT_C    = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_SLANT_C_low     , ANR_ACT_SLANT_C_high, 1);
        ANR_ACT_BLD_TH_C   = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_BLD_TH_C_low    , ANR_ACT_BLD_TH_C_high, 1);
        ANR_ACT_TH_Y_L         = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_TH_Y_L_low      , ANR_ACT_TH_Y_L_high, 1);
        ANR_ACT_BLD_BASE_Y_L   = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_BLD_BASE_Y_L_low, ANR_ACT_BLD_BASE_Y_L_high, 1);
        ANR_ACT_SLANT_Y_L      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_SLANT_Y_L_low   , ANR_ACT_SLANT_Y_L_high, 1);
        ANR_ACT_BLD_TH_Y_L     = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_BLD_TH_Y_L_low  , ANR_ACT_BLD_TH_Y_L_high, 1);
        ANR_CEN_GAIN_LO_TH_LPF = InterParam_NR(ISO, ISO_low, ISO_high, ANR_CEN_GAIN_LO_TH_LPF_low, ANR_CEN_GAIN_LO_TH_LPF_high, 1);
        ANR_CEN_GAIN_HI_TH_LPF = InterParam_NR(ISO, ISO_low, ISO_high, ANR_CEN_GAIN_HI_TH_LPF_low, ANR_CEN_GAIN_HI_TH_LPF_high, 1);
        ANR_K_LMT              = InterParam_NR(ISO, ISO_low, ISO_high, ANR_K_LMT_low           , ANR_K_LMT_high, 1);
        ANR_K_LPF              = InterParam_NR(ISO, ISO_low, ISO_high, ANR_K_LPF_low           , ANR_K_LPF_high, 1);
        ANR_PTY1_LPF           = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTY1_LPF_low        , ANR_PTY1_LPF_high, 1);
        ANR_PTY2_LPF           = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTY2_LPF_low        , ANR_PTY2_LPF_high, 1);
        ANR_PTY3_LPF           = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTY3_LPF_low        , ANR_PTY3_LPF_high, 1);
        ANR_PTY4_LPF           = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTY4_LPF_low        , ANR_PTY4_LPF_high, 1);
        ANR_COEF_A             = InterParam_NR(ISO, ISO_low, ISO_high, ANR_COEF_A_low          , ANR_COEF_A_high, 1);
        ANR_COEF_B             = InterParam_NR(ISO, ISO_low, ISO_high, ANR_COEF_B_low          , ANR_COEF_B_high, 1);
        ANR_COEF_C             = InterParam_NR(ISO, ISO_low, ISO_high, ANR_COEF_C_low          , ANR_COEF_C_high, 1);
        ANR_COEF_D             = InterParam_NR(ISO, ISO_low, ISO_high, ANR_COEF_D_low          , ANR_COEF_D_high, 1);

        // ANR_SCALE_MODE
        ANR_SCALE_MODE     = InterParam_NR(ISO, ISO_low, ISO_high, ANR_SCALE_MODE_low      , ANR_SCALE_MODE_high, 1);
        ANR_K_LO_TH        = InterParam_NR(ISO, ISO_low, ISO_high, ANR_K_LO_TH_low         , ANR_K_LO_TH_high, 1);
        ANR_K_HI_TH        = InterParam_NR(ISO, ISO_low, ISO_high, ANR_K_HI_TH_low         , ANR_K_HI_TH_high, 1);
        ANR_K_TH_C         = InterParam_NR(ISO, ISO_low, ISO_high, ANR_K_TH_C_low          , ANR_K_TH_C_high, 1);

        // ANR_LCE_LINK
        ANR_LCE_LINK       = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_LINK_low        , ANR_LCE_LINK_high, 1);
        ANR_LCE_X1         = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_X1_low          , ANR_LCE_X1_high, 1);
        ANR_LCE_X2         = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_X2_low          , ANR_LCE_X2_high, 1);
        ANR_LCE_X3         = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_X3_low          , ANR_LCE_X3_high, 1);
        ANR_LCE_GAIN0      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_GAIN0_low       , ANR_LCE_GAIN0_high, 1);
        ANR_LCE_GAIN1      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_GAIN1_low       , ANR_LCE_GAIN1_high, 1);
        ANR_LCE_GAIN2      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_GAIN2_low       , ANR_LCE_GAIN2_high, 1);
        ANR_LCE_GAIN3      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_GAIN3_low       , ANR_LCE_GAIN3_high, 1);
        ANR_LCE_SP0        = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_SP0_low         , ANR_LCE_SP0_high, 1);
        ANR_LCE_SP1        = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_SP1_low         , ANR_LCE_SP1_high, 1);
        ANR_LCE_SP2        = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_SP2_low         , ANR_LCE_SP2_high, 1);
        ANR_LCE_SP3        = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_SP3_low         , ANR_LCE_SP3_high, 1);
        ANR_LCE_C_GAIN     = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_C_GAIN_low      , ANR_LCE_C_GAIN_high, 1);
        ANR_LCE_SCALE_GAIN = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_SCALE_GAIN_low  , ANR_LCE_SCALE_GAIN_high, 1);

        // ANR_MEDIAN_EN
        ANR_MEDIAN_EN   = InterParam_NR(ISO, ISO_low, ISO_high, ANR_MEDIAN_EN_low          , ANR_MEDIAN_EN_high, 1);
        ANR_COR_TH      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_COR_TH_low             , ANR_COR_TH_high, 1);
        ANR_COR_SL      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_COR_SL_low             , ANR_COR_SL_high, 1);
        ANR_MCD_TH      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_MCD_TH_low             , ANR_MCD_TH_high, 1);
        ANR_MCD_SL      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_MCD_SL_low             , ANR_MCD_SL_high, 1);
        ANR_LCL_TH      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCL_TH_low             , ANR_LCL_TH_high, 1);
        ANR_LCL_SL      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCL_SL_low             , ANR_LCL_SL_high, 1);
        ANR_LCL_LV      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCL_LV_low             , ANR_LCL_LV_high, 1);
        ANR_SCL_TH      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_SCL_TH_low             , ANR_SCL_TH_high, 1);
        ANR_SCL_SL      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_SCL_SL_low             , ANR_SCL_SL_high, 1);
        ANR_SCL_LV      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_SCL_LV_low             , ANR_SCL_LV_high, 1);
        ANR_NCL_TH      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_NCL_TH_low             , ANR_NCL_TH_high, 1);
        ANR_NCL_SL      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_NCL_SL_low             , ANR_NCL_SL_high, 1);
        ANR_NCL_LV      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_NCL_LV_low             , ANR_NCL_LV_high, 1);
        ANR_VAR         = InterParam_NR(ISO, ISO_low, ISO_high, ANR_VAR_low                , ANR_VAR_high, 1);
        ANR_Y0          = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y0_low                 , ANR_Y0_high, 1);
        ANR_Y1          = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y1_low                 , ANR_Y1_high, 1);
        ANR_Y2          = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y2_low                 , ANR_Y2_high, 1);
        ANR_Y3          = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y3_low                 , ANR_Y3_high, 1);
        ANR_Y4          = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y4_low                 , ANR_Y4_high, 1);

		//Olympus
		ANR_C_SM_EDGE_TH        = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_SM_EDGE_TH_low, ANR_C_SM_EDGE_TH_high, 1);
        ANR_K_HPF               = InterParam_NR(ISO, ISO_low, ISO_high, ANR_K_HPF_low, ANR_K_HPF_high, 1);
		ANR_ACT_DIF_HI_TH       = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_DIF_HI_TH_low, ANR_ACT_DIF_HI_TH_high, 1);
		ANR_ACT_TH_Y_HL         = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_TH_Y_HL_low, ANR_ACT_TH_Y_HL_high, 1);
        ANR_ACT_BLD_BASE_Y_HL   = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_BLD_BASE_Y_HL_low, ANR_ACT_BLD_BASE_Y_HL_high, 1);
        ANR_ACT_SLANT_Y_HL      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_SLANT_Y_HL_low, ANR_ACT_SLANT_Y_HL_high, 1);
        ANR_ACT_BLD_TH_Y_HL     = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_BLD_TH_Y_HL_low, ANR_ACT_BLD_TH_Y_HL_high, 1);
		ANR_ACT_TH_Y_HH         = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_TH_Y_HH_low, ANR_ACT_TH_Y_HH_high, 1);
        ANR_ACT_BLD_BASE_Y_HH   = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_BLD_BASE_Y_HH_low, ANR_ACT_BLD_BASE_Y_HH_high, 1);
        ANR_ACT_SLANT_Y_HH      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_SLANT_Y_HH_low, ANR_ACT_SLANT_Y_HH_high, 1);
        ANR_ACT_BLD_TH_Y_HH     = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_BLD_TH_Y_HH_low, ANR_ACT_BLD_TH_Y_HH_high, 1);
		ANR_LUMA_TH_Y_L         = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LUMA_TH_Y_L_low, ANR_LUMA_TH_Y_L_high, 1);
        ANR_LUMA_BLD_BASE_Y_L   = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LUMA_BLD_BASE_Y_L_low, ANR_LUMA_BLD_BASE_Y_L_high, 1);
        ANR_LUMA_SLANT_Y_L      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LUMA_SLANT_Y_L_low, ANR_LUMA_SLANT_Y_L_high, 1);
        ANR_LUMA_BLD_TH_Y_L     = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LUMA_BLD_TH_Y_L_low, ANR_LUMA_BLD_TH_Y_L_high, 1);
		ANR_LUMA_TH_Y_HL        = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_TH_Y_HL_low, ANR_LUMA_TH_Y_HL_high, 1);
        ANR_LUMA_BLD_BASE_Y_HL  = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_BLD_BASE_Y_HL_low, ANR_LUMA_BLD_BASE_Y_HL_high, 1);
        ANR_LUMA_SLANT_Y_HL     = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_SLANT_Y_HL_low, ANR_LUMA_SLANT_Y_HL_high, 1);
        ANR_LUMA_BLD_TH_Y_HL    = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_BLD_TH_Y_HL_low, ANR_LUMA_BLD_TH_Y_HL_high, 1);
		ANR_LUMA_TH_Y_HH        = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_TH_Y_HH_low, ANR_LUMA_TH_Y_HH_high, 1);
        ANR_LUMA_BLD_BASE_Y_HH  = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_BLD_BASE_Y_HH_low, ANR_LUMA_BLD_BASE_Y_HH_high, 1);
        ANR_LUMA_SLANT_Y_HH     = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_SLANT_Y_HH_low, ANR_LUMA_SLANT_Y_HH_high, 1);
        ANR_LUMA_BLD_TH_Y_HH    = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_BLD_TH_Y_HH_low, ANR_LUMA_BLD_TH_Y_HH_high, 1);
		ANR_PTY_L_GAIN          = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTY_L_GAIN_low, ANR_PTY_L_GAIN_high, 1);
		ANR_PTY_H_GAIN          = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTY_H_GAIN_low, ANR_PTY_H_GAIN_high, 1);
		ANR_BLEND_TH_L          = InterParam_NR(ISO, ISO_low, ISO_high, ANR_BLEND_TH_L_low, ANR_BLEND_TH_L_high, 1);
		ANR_BLEND_TH_H          = InterParam_NR(ISO, ISO_low, ISO_high, ANR_BLEND_TH_H_low, ANR_BLEND_TH_H_high, 1);
		ANR_HI_LMT1             = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HI_LMT1_low, ANR_HI_LMT1_high, 1);
		ANR_HI_LMT2             = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HI_LMT2_low, ANR_HI_LMT2_high, 1);
		ANR_LO_LMT1             = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LO_LMT1_low, ANR_LO_LMT1_high, 1);
		ANR_LO_LMT2             = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LO_LMT2_low, ANR_LO_LMT2_high, 1);
		ANR_Y_ACT_CEN_OFT       = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_ACT_CEN_OFT_low, ANR_Y_ACT_CEN_OFT_high, 1);
		ANR_Y_ACT_CEN_GAIN      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_ACT_CEN_GAIN_low, ANR_Y_ACT_CEN_GAIN_high, 1);
		ANR_Y_ACT_CEN_TH        = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_ACT_CEN_TH_low, ANR_Y_ACT_CEN_TH_high, 1);
		ANR_Y_DIR               = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_DIR_low, ANR_Y_DIR_high, 1);
        ANR_Y_DIR_GAIN          = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_DIR_GAIN_low   , ANR_Y_DIR_GAIN_high, 1);
        ANR_Y_DIR_VER_W_TH      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_DIR_VER_W_TH_low   , ANR_Y_DIR_VER_W_TH_high, 1);
        ANR_Y_DIR_VER_W_SL      = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_DIR_VER_W_SL_low   , ANR_Y_DIR_VER_W_SL_high, 1);
        ANR_Y_DIR_DIAG_W_TH     = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_DIR_DIAG_W_TH_low   , ANR_Y_DIR_DIAG_W_TH_high, 1);
        ANR_Y_DIR_DIAG_W_SL     = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_DIR_DIAG_W_SL_low   , ANR_Y_DIR_DIAG_W_SL_high, 1);
    }

    //    ====================================================================================================================================
    //    Negative Conversion
    //    ====================================================================================================================================
    ANR_Y_SCALE_SP0 = (ANR_Y_SCALE_SP0 < 0) ? (ANR_Y_SCALE_SP0 + 32) : ANR_Y_SCALE_SP0;
    ANR_Y_SCALE_SP1 = (ANR_Y_SCALE_SP1 < 0) ? (ANR_Y_SCALE_SP1 + 32) : ANR_Y_SCALE_SP1;
    ANR_Y_SCALE_SP2 = (ANR_Y_SCALE_SP2 < 0) ? (ANR_Y_SCALE_SP2 + 32) : ANR_Y_SCALE_SP2;
    ANR_Y_SCALE_SP3 = (ANR_Y_SCALE_SP3 < 0) ? (ANR_Y_SCALE_SP3 + 32) : ANR_Y_SCALE_SP3;
    ANR_C_SCALE_SP0 = (ANR_C_SCALE_SP0 < 0) ? (ANR_C_SCALE_SP0 + 32) : ANR_C_SCALE_SP0;
    ANR_C_SCALE_SP1 = (ANR_C_SCALE_SP1 < 0) ? (ANR_C_SCALE_SP1 + 32) : ANR_C_SCALE_SP1;
    ANR_C_SCALE_SP2 = (ANR_C_SCALE_SP2 < 0) ? (ANR_C_SCALE_SP2 + 32) : ANR_C_SCALE_SP2;
    ANR_C_SCALE_SP3 = (ANR_C_SCALE_SP3 < 0) ? (ANR_C_SCALE_SP3 + 32) : ANR_C_SCALE_SP3;
    ANR_Y_ACT_SP0   = (ANR_Y_ACT_SP0 < 0) ? (ANR_Y_ACT_SP0 + 64) : ANR_Y_ACT_SP0;
    ANR_Y_ACT_SP1   = (ANR_Y_ACT_SP1 < 0) ? (ANR_Y_ACT_SP1 + 64) : ANR_Y_ACT_SP1;
    ANR_Y_ACT_SP2   = (ANR_Y_ACT_SP2 < 0) ? (ANR_Y_ACT_SP2 + 64) : ANR_Y_ACT_SP2;
    ANR_Y_ACT_SP3   = (ANR_Y_ACT_SP3 < 0) ? (ANR_Y_ACT_SP3 + 64) : ANR_Y_ACT_SP3;
    ANR_LCE_SP0     = (ANR_LCE_SP0 < 0) ? (ANR_LCE_SP0 + 64) : ANR_LCE_SP0;
    ANR_LCE_SP1     = (ANR_LCE_SP1 < 0) ? (ANR_LCE_SP1 + 64) : ANR_LCE_SP1;
    ANR_LCE_SP2     = (ANR_LCE_SP2 < 0) ? (ANR_LCE_SP2 + 64) : ANR_LCE_SP2;
    ANR_LCE_SP3     = (ANR_LCE_SP3 < 0) ? (ANR_LCE_SP3 + 64) : ANR_LCE_SP3;
    ANR_HP_B        = (ANR_HP_B < 0) ? (ANR_HP_B + 64) : ANR_HP_B;
    ANR_HP_C        = (ANR_HP_C < 0) ? (ANR_HP_C + 32) : ANR_HP_C;
    ANR_HP_D        = (ANR_HP_D < 0) ? (ANR_HP_D + 16) : ANR_HP_D;
    ANR_HP_E        = (ANR_HP_E < 0) ? (ANR_HP_E + 16) : ANR_HP_E;
    ANR_HP_F        = (ANR_HP_F < 0) ? (ANR_HP_F + 8)  : ANR_HP_F;

    //    ====================================================================================================================================
    //    Set Smooth ISO setting
    //    ====================================================================================================================================
    // CAM_ANR_CON1 CAM+A20H
    rSmoothANR.con1.bits.ANR_ENC = ANR_ENC;
    rSmoothANR.con1.bits.ANR_ENY = ANR_ENY;
    rSmoothANR.con1.bits.ANR_SCALE_MODE = ANR_SCALE_MODE;
    rSmoothANR.con1.bits.ANR_MEDIAN_EN = ANR_MEDIAN_EN;
    rSmoothANR.con1.bits.ANR_LCE_LINK = ANR_LCE_LINK;
    rSmoothANR.con1.bits.ANR_TABLE_EN = ANR_TABLE_EN;
    rSmoothANR.con1.bits.ANR_TBL_PRC = ANR_TBL_PRC;

    // CAM_ANR_CON2 CAM+A24H
    rSmoothANR.con2.bits.ANR_IMPL_MODE = ANR_IMPL_MODE;
    rSmoothANR.con2.bits.ANR_C_MED_EN = ANR_C_MED_EN;
    rSmoothANR.con2.bits.ANR_C_SM_EDGE = ANR_C_SM_EDGE;
    rSmoothANR.con2.bits.ANR_FLT_C = ANR_FLT_C;
    rSmoothANR.con2.bits.ANR_LR = ANR_LR;
    rSmoothANR.con2.bits.ANR_ALPHA = ANR_ALPHA;

    // CAM_ANR_YAD1 CAM+A2CH
    rSmoothANR.yad1.bits.ANR_CEN_GAIN_LO_TH = ANR_CEN_GAIN_LO_TH;
    rSmoothANR.yad1.bits.ANR_CEN_GAIN_HI_TH = ANR_CEN_GAIN_HI_TH;
    rSmoothANR.yad1.bits.ANR_K_LO_TH = ANR_K_LO_TH;
    rSmoothANR.yad1.bits.ANR_K_HI_TH = ANR_K_HI_TH;
    rSmoothANR.yad1.bits.ANR_K_TH_C = ANR_K_TH_C;

    // CAM_ANR_YAD2 CAM+A30H
    rSmoothANR.yad2.bits.ANR_PTY_VGAIN = ANR_PTY_VGAIN;
    rSmoothANR.yad2.bits.ANR_PTY_GAIN_TH = ANR_PTY_GAIN_TH;

    // CAM_ANR_Y4LUT1 CAM+A34H
    rSmoothANR.y4lut1.bits.ANR_Y_CPX1 = ANR_Y_CPX1;
    rSmoothANR.y4lut1.bits.ANR_Y_CPX2 = ANR_Y_CPX2;
    rSmoothANR.y4lut1.bits.ANR_Y_CPX3 = ANR_Y_CPX3;

    // CAM_ANR_Y4LUT2 CAM+A38H
    rSmoothANR.y4lut2.bits.ANR_Y_SCALE_CPY0 = ANR_Y_SCALE_CPY0;
    rSmoothANR.y4lut2.bits.ANR_Y_SCALE_CPY1 = ANR_Y_SCALE_CPY1;
    rSmoothANR.y4lut2.bits.ANR_Y_SCALE_CPY2 = ANR_Y_SCALE_CPY2;
    rSmoothANR.y4lut2.bits.ANR_Y_SCALE_CPY3 = ANR_Y_SCALE_CPY3;

    // CAM_ANR_Y4LUT3 CAM+A3CH
    rSmoothANR.y4lut3.bits.ANR_Y_SCALE_SP0 = ANR_Y_SCALE_SP0;
    rSmoothANR.y4lut3.bits.ANR_Y_SCALE_SP1 = ANR_Y_SCALE_SP1;
    rSmoothANR.y4lut3.bits.ANR_Y_SCALE_SP2 = ANR_Y_SCALE_SP2;
    rSmoothANR.y4lut3.bits.ANR_Y_SCALE_SP3 = ANR_Y_SCALE_SP3;

    // CAM_ANR_C4LUT1
    rSmoothANR.c4lut1.bits.ANR_C_CPX1 = ANR_C_CPX1;
    rSmoothANR.c4lut1.bits.ANR_C_CPX2 = ANR_C_CPX2;
    rSmoothANR.c4lut1.bits.ANR_C_CPX3 = ANR_C_CPX3;

    // CAM_ANR_C4LUT2
    rSmoothANR.c4lut2.bits.ANR_C_SCALE_CPY0 = ANR_C_SCALE_CPY0;
    rSmoothANR.c4lut2.bits.ANR_C_SCALE_CPY1 = ANR_C_SCALE_CPY1;
    rSmoothANR.c4lut2.bits.ANR_C_SCALE_CPY2 = ANR_C_SCALE_CPY2;
    rSmoothANR.c4lut2.bits.ANR_C_SCALE_CPY3 = ANR_C_SCALE_CPY3;

    // CAM_ANR_C4LUT3
    rSmoothANR.c4lut3.bits.ANR_C_SCALE_SP0 = ANR_C_SCALE_SP0;
    rSmoothANR.c4lut3.bits.ANR_C_SCALE_SP1 = ANR_C_SCALE_SP1;
    rSmoothANR.c4lut3.bits.ANR_C_SCALE_SP2 = ANR_C_SCALE_SP2;
    rSmoothANR.c4lut3.bits.ANR_C_SCALE_SP3 = ANR_C_SCALE_SP3;

    // CAM_ANR_A4LUT2
    rSmoothANR.a4lut2.bits.ANR_Y_ACT_CPY0 = ANR_Y_ACT_CPY0;
    rSmoothANR.a4lut2.bits.ANR_Y_ACT_CPY1 = ANR_Y_ACT_CPY1;
    rSmoothANR.a4lut2.bits.ANR_Y_ACT_CPY2 = ANR_Y_ACT_CPY2;
    rSmoothANR.a4lut2.bits.ANR_Y_ACT_CPY3 = ANR_Y_ACT_CPY3;

    // CAM_ANR_A4LUT3
    rSmoothANR.a4lut3.bits.ANR_Y_ACT_SP0 = ANR_Y_ACT_SP0;
    rSmoothANR.a4lut3.bits.ANR_Y_ACT_SP1 = ANR_Y_ACT_SP1;
    rSmoothANR.a4lut3.bits.ANR_Y_ACT_SP2 = ANR_Y_ACT_SP2;
    rSmoothANR.a4lut3.bits.ANR_Y_ACT_SP3 = ANR_Y_ACT_SP3;

    // CAM_ANR_L4LUT1
    rSmoothANR.l4lut1.bits.ANR_LCE_X1 = ANR_LCE_X1;
    rSmoothANR.l4lut1.bits.ANR_LCE_X2 = ANR_LCE_X2;
    rSmoothANR.l4lut1.bits.ANR_LCE_X3 = ANR_LCE_X3;

    // CAM_ANR_L4LUT2
    rSmoothANR.l4lut2.bits.ANR_LCE_GAIN0 = ANR_LCE_GAIN0;
    rSmoothANR.l4lut2.bits.ANR_LCE_GAIN1 = ANR_LCE_GAIN1;
    rSmoothANR.l4lut2.bits.ANR_LCE_GAIN2 = ANR_LCE_GAIN2;
    rSmoothANR.l4lut2.bits.ANR_LCE_GAIN3 = ANR_LCE_GAIN3;

    // CAM_ANR_L4LUT3
    rSmoothANR.l4lut3.bits.ANR_LCE_SP0 = ANR_LCE_SP0;
    rSmoothANR.l4lut3.bits.ANR_LCE_SP1 = ANR_LCE_SP1;
    rSmoothANR.l4lut3.bits.ANR_LCE_SP2 = ANR_LCE_SP2;
    rSmoothANR.l4lut3.bits.ANR_LCE_SP3 = ANR_LCE_SP3;

    // CAM_ANR_PTY CAM+A40H
    rSmoothANR.pty.bits.ANR_PTY1 = ANR_PTY1;
    rSmoothANR.pty.bits.ANR_PTY2 = ANR_PTY2;
    rSmoothANR.pty.bits.ANR_PTY3 = ANR_PTY3;
    rSmoothANR.pty.bits.ANR_PTY4 = ANR_PTY4;

    // CAM_ANR_CAD CAM+A44H
    rSmoothANR.cad.bits.ANR_PTC_VGAIN = ANR_PTC_VGAIN;
    rSmoothANR.cad.bits.ANR_PTC_GAIN_TH = ANR_PTC_GAIN_TH;
    rSmoothANR.cad.bits.ANR_C_L_DIFF_TH = ANR_C_L_DIFF_TH;

    // CAM_ANR_PTC CAM+A48H
    rSmoothANR.ptc.bits.ANR_PTC1 = ANR_PTC1;
    rSmoothANR.ptc.bits.ANR_PTC2 = ANR_PTC2;
    rSmoothANR.ptc.bits.ANR_PTC3 = ANR_PTC3;
    rSmoothANR.ptc.bits.ANR_PTC4 = ANR_PTC4;

    // CAM_ANR_LCE CAM+A4CH
    rSmoothANR.lce.bits.ANR_LCE_C_GAIN = ANR_LCE_C_GAIN;
    rSmoothANR.lce.bits.ANR_LCE_SCALE_GAIN = ANR_LCE_SCALE_GAIN;
    rSmoothANR.lce.bits.ANR_LM_WT = ANR_LM_WT;

    // CAM_ANR_MED1
    rSmoothANR.med1.bits.ANR_COR_TH = ANR_COR_TH;
    rSmoothANR.med1.bits.ANR_COR_SL = ANR_COR_SL;
    rSmoothANR.med1.bits.ANR_MCD_TH = ANR_MCD_TH;
    rSmoothANR.med1.bits.ANR_MCD_SL = ANR_MCD_SL;
    rSmoothANR.med1.bits.ANR_LCL_TH = ANR_LCL_TH;

    // CAM_ANR_MED2
    rSmoothANR.med2.bits.ANR_LCL_SL = ANR_LCL_SL;
    rSmoothANR.med2.bits.ANR_LCL_LV = ANR_LCL_LV;
    rSmoothANR.med2.bits.ANR_SCL_TH = ANR_SCL_TH;
    rSmoothANR.med2.bits.ANR_SCL_SL = ANR_SCL_SL;
    rSmoothANR.med2.bits.ANR_SCL_LV = ANR_SCL_LV;

    // CAM_ANR_MED3
    rSmoothANR.med3.bits.ANR_NCL_TH = ANR_NCL_TH;
    rSmoothANR.med3.bits.ANR_NCL_SL = ANR_NCL_SL;
    rSmoothANR.med3.bits.ANR_NCL_LV = ANR_NCL_LV;
    rSmoothANR.med3.bits.ANR_VAR = ANR_VAR;
    rSmoothANR.med3.bits.ANR_Y0 = ANR_Y0;

    // CAM_ANR_MED4
    rSmoothANR.med4.bits.ANR_Y1 = ANR_Y1;
    rSmoothANR.med4.bits.ANR_Y2 = ANR_Y2;
    rSmoothANR.med4.bits.ANR_Y3 = ANR_Y3;
    rSmoothANR.med4.bits.ANR_Y4 = ANR_Y4;

    // CAM_ANR_HP1 CAM+A54H
    rSmoothANR.hp1.bits.ANR_HP_A = ANR_HP_A;
    rSmoothANR.hp1.bits.ANR_HP_B = ANR_HP_B;
    rSmoothANR.hp1.bits.ANR_HP_C = ANR_HP_C;
    rSmoothANR.hp1.bits.ANR_HP_D = ANR_HP_D;
    rSmoothANR.hp1.bits.ANR_HP_E = ANR_HP_E;

    // CAM_ANR_HP2 CAM+A58H
    rSmoothANR.hp2.bits.ANR_HP_S1 = ANR_HP_S1;
    rSmoothANR.hp2.bits.ANR_HP_S2 = ANR_HP_S2;
    rSmoothANR.hp2.bits.ANR_HP_X1 = ANR_HP_X1;
    rSmoothANR.hp2.bits.ANR_HP_F = ANR_HP_F;

    // CAM_ANR_HP3 CAM+A5CH
    rSmoothANR.hp3.bits.ANR_HP_Y_GAIN_CLIP = ANR_HP_Y_GAIN_CLIP;
    rSmoothANR.hp3.bits.ANR_HP_Y_SP = ANR_HP_Y_SP;
    rSmoothANR.hp3.bits.ANR_HP_Y_LO = ANR_HP_Y_LO;
    rSmoothANR.hp3.bits.ANR_HP_CLIP = ANR_HP_CLIP;

    // CAM_ANR_ACT1 CAM+A64H
    rSmoothANR.act1.bits.ANR_ACT_LCE_GAIN = ANR_ACT_LCE_GAIN;
    rSmoothANR.act1.bits.ANR_ACT_SCALE_OFT = ANR_ACT_SCALE_OFT;
    rSmoothANR.act1.bits.ANR_ACT_SCALE_GAIN = ANR_ACT_SCALE_GAIN;
    rSmoothANR.act1.bits.ANR_ACT_DIF_GAIN = ANR_ACT_DIF_GAIN;
    rSmoothANR.act1.bits.ANR_ACT_DIF_LO_TH = ANR_ACT_DIF_LO_TH;

    // CAM_ANR_ACT2 CAM+A64H
    rSmoothANR.act2.bits.ANR_ACT_SIZE_GAIN = ANR_ACT_SIZE_GAIN;
    rSmoothANR.act2.bits.ANR_ACT_SIZE_LO_TH = ANR_ACT_SIZE_LO_TH;
    rSmoothANR.act2.bits.ANR_COR_TH1 = ANR_COR_TH1;
    rSmoothANR.act2.bits.ANR_COR_SL1 = ANR_COR_SL1;

    // CAM_ANR_ACT3 CAM+A64H
    rSmoothANR.act3.bits.ANR_COR_ACT_TH = ANR_COR_ACT_TH;
    rSmoothANR.act3.bits.ANR_COR_ACT_SL1 = ANR_COR_ACT_SL1;
    rSmoothANR.act3.bits.ANR_COR_ACT_SL2 = ANR_COR_ACT_SL2;

    // CAM_ANR_ACTC CAM+A64H
    rSmoothANR.actc.bits.ANR_ACT_TH_C = ANR_ACT_TH_C;
    rSmoothANR.actc.bits.ANR_ACT_BLD_BASE_C = ANR_ACT_BLD_BASE_C;
    rSmoothANR.actc.bits.ANR_ACT_SLANT_C = ANR_ACT_SLANT_C;
    rSmoothANR.actc.bits.ANR_ACT_BLD_TH_C = ANR_ACT_BLD_TH_C;

    // CAM_ANR_ACTY CAM+A60H
    rSmoothANR.actyl.bits.ANR_ACT_TH_Y_L = ANR_ACT_TH_Y_L;
    rSmoothANR.actyl.bits.ANR_ACT_BLD_BASE_Y_L = ANR_ACT_BLD_BASE_Y_L;
    rSmoothANR.actyl.bits.ANR_ACT_SLANT_Y_L = ANR_ACT_SLANT_Y_L;
    rSmoothANR.actyl.bits.ANR_ACT_BLD_TH_Y_L = ANR_ACT_BLD_TH_Y_L;

    // DIP_X_ANR_YLAD CAM+A60H
    rSmoothANR.ylad.bits.ANR_CEN_GAIN_LO_TH_LPF = ANR_CEN_GAIN_LO_TH_LPF;
    rSmoothANR.ylad.bits.ANR_CEN_GAIN_HI_TH_LPF = ANR_CEN_GAIN_HI_TH_LPF;
    rSmoothANR.ylad.bits.ANR_K_LMT = ANR_K_LMT;
    rSmoothANR.ylad.bits.ANR_K_LPF = ANR_K_LPF;

    // DIP_X_ANR_YLAD CAM+A60H
    rSmoothANR.ptyl.bits.ANR_PTY1_LPF = ANR_PTY1_LPF;
    rSmoothANR.ptyl.bits.ANR_PTY2_LPF = ANR_PTY2_LPF;
    rSmoothANR.ptyl.bits.ANR_PTY3_LPF = ANR_PTY3_LPF;
    rSmoothANR.ptyl.bits.ANR_PTY4_LPF = ANR_PTY4_LPF;

    // DIP_X_ANR_YLAD CAM+A60H
    rSmoothANR.lcoef.bits.ANR_COEF_A = ANR_COEF_A;
    rSmoothANR.lcoef.bits.ANR_COEF_B = ANR_COEF_B;
    rSmoothANR.lcoef.bits.ANR_COEF_C = ANR_COEF_C;
    rSmoothANR.lcoef.bits.ANR_COEF_D = ANR_COEF_D;

#define ANR_DUMP(item1, item2)   \
        do{                      \
            MY_LOG_IF(bSmoothANRDebug, "[L,S,U]%22s = %d, %d, %d", #item2, rLowerANR.item1.bits.item2, rSmoothANR.item1.bits.item2, rUpperANR.item1.bits.item2); \
        }while(0)

    ANR_DUMP(con1, ANR_ENC);
    ANR_DUMP(con1, ANR_ENY);
    ANR_DUMP(con1, ANR_SCALE_MODE);
    ANR_DUMP(con1, ANR_MEDIAN_EN);
    ANR_DUMP(con1, ANR_LCE_LINK);
    ANR_DUMP(con1, ANR_TABLE_EN);
    ANR_DUMP(con1, ANR_TBL_PRC);

    ANR_DUMP(con2, ANR_IMPL_MODE);
    ANR_DUMP(con2, ANR_C_MED_EN);
    ANR_DUMP(con2, ANR_C_SM_EDGE);
    ANR_DUMP(con2, ANR_FLT_C);
    ANR_DUMP(con2, ANR_LR);
    ANR_DUMP(con2, ANR_ALPHA);

    ANR_DUMP(yad1, ANR_CEN_GAIN_LO_TH);
    ANR_DUMP(yad1, ANR_CEN_GAIN_HI_TH);
    ANR_DUMP(yad1, ANR_K_LO_TH);
    ANR_DUMP(yad1, ANR_K_HI_TH);
    ANR_DUMP(yad1, ANR_K_TH_C);

    ANR_DUMP(yad2, ANR_PTY_VGAIN);
    ANR_DUMP(yad2, ANR_PTY_GAIN_TH);

    ANR_DUMP(y4lut1, ANR_Y_CPX1);
    ANR_DUMP(y4lut1, ANR_Y_CPX2);
    ANR_DUMP(y4lut1, ANR_Y_CPX3);

    ANR_DUMP(y4lut2, ANR_Y_SCALE_CPY0);
    ANR_DUMP(y4lut2, ANR_Y_SCALE_CPY1);
    ANR_DUMP(y4lut2, ANR_Y_SCALE_CPY2);
    ANR_DUMP(y4lut2, ANR_Y_SCALE_CPY3);

    ANR_DUMP(y4lut3, ANR_Y_SCALE_SP0);
    ANR_DUMP(y4lut3, ANR_Y_SCALE_SP1);
    ANR_DUMP(y4lut3, ANR_Y_SCALE_SP2);
    ANR_DUMP(y4lut3, ANR_Y_SCALE_SP3);

    ANR_DUMP(c4lut1, ANR_C_CPX1);
    ANR_DUMP(c4lut1, ANR_C_CPX2);
    ANR_DUMP(c4lut1, ANR_C_CPX3);

    ANR_DUMP(c4lut2, ANR_C_SCALE_CPY0);
    ANR_DUMP(c4lut2, ANR_C_SCALE_CPY1);
    ANR_DUMP(c4lut2, ANR_C_SCALE_CPY2);
    ANR_DUMP(c4lut2, ANR_C_SCALE_CPY3);

    ANR_DUMP(c4lut3, ANR_C_SCALE_SP0);
    ANR_DUMP(c4lut3, ANR_C_SCALE_SP1);
    ANR_DUMP(c4lut3, ANR_C_SCALE_SP2);
    ANR_DUMP(c4lut3, ANR_C_SCALE_SP3);

    ANR_DUMP(a4lut2, ANR_Y_ACT_CPY0);
    ANR_DUMP(a4lut2, ANR_Y_ACT_CPY1);
    ANR_DUMP(a4lut2, ANR_Y_ACT_CPY2);
    ANR_DUMP(a4lut2, ANR_Y_ACT_CPY3);

    ANR_DUMP(a4lut3, ANR_Y_ACT_SP0);
    ANR_DUMP(a4lut3, ANR_Y_ACT_SP1);
    ANR_DUMP(a4lut3, ANR_Y_ACT_SP2);
    ANR_DUMP(a4lut3, ANR_Y_ACT_SP3);

    ANR_DUMP(l4lut1, ANR_LCE_X1);
    ANR_DUMP(l4lut1, ANR_LCE_X2);
    ANR_DUMP(l4lut1, ANR_LCE_X3);

    ANR_DUMP(l4lut2, ANR_LCE_GAIN0);
    ANR_DUMP(l4lut2, ANR_LCE_GAIN1);
    ANR_DUMP(l4lut2, ANR_LCE_GAIN2);
    ANR_DUMP(l4lut2, ANR_LCE_GAIN3);

    ANR_DUMP(l4lut3, ANR_LCE_SP0);
    ANR_DUMP(l4lut3, ANR_LCE_SP1);
    ANR_DUMP(l4lut3, ANR_LCE_SP2);
    ANR_DUMP(l4lut3, ANR_LCE_SP3);

    ANR_DUMP(pty, ANR_PTY1);
    ANR_DUMP(pty, ANR_PTY2);
    ANR_DUMP(pty, ANR_PTY3);
    ANR_DUMP(pty, ANR_PTY4);

    ANR_DUMP(cad, ANR_PTC_VGAIN);
    ANR_DUMP(cad, ANR_PTC_GAIN_TH);
    ANR_DUMP(cad, ANR_C_L_DIFF_TH);

    ANR_DUMP(ptc, ANR_PTC1);
    ANR_DUMP(ptc, ANR_PTC2);
    ANR_DUMP(ptc, ANR_PTC3);
    ANR_DUMP(ptc, ANR_PTC4);

    ANR_DUMP(lce, ANR_LCE_C_GAIN);
    ANR_DUMP(lce, ANR_LCE_SCALE_GAIN);
    ANR_DUMP(lce, ANR_LM_WT);

    ANR_DUMP(med1, ANR_COR_TH);
    ANR_DUMP(med1, ANR_COR_SL);
    ANR_DUMP(med1, ANR_MCD_TH);
    ANR_DUMP(med1, ANR_MCD_SL);
    ANR_DUMP(med1, ANR_LCL_TH);

    ANR_DUMP(med2, ANR_LCL_SL);
    ANR_DUMP(med2, ANR_LCL_LV);
    ANR_DUMP(med2, ANR_SCL_TH);
    ANR_DUMP(med2, ANR_SCL_SL);
    ANR_DUMP(med2, ANR_SCL_LV);

    ANR_DUMP(med3, ANR_NCL_TH);
    ANR_DUMP(med3, ANR_NCL_SL);
    ANR_DUMP(med3, ANR_NCL_LV);
    ANR_DUMP(med3, ANR_VAR);
    ANR_DUMP(med3, ANR_Y0);

    ANR_DUMP(med4, ANR_Y1);
    ANR_DUMP(med4, ANR_Y2);
    ANR_DUMP(med4, ANR_Y3);
    ANR_DUMP(med4, ANR_Y4);

    ANR_DUMP(hp1, ANR_HP_A);
    ANR_DUMP(hp1, ANR_HP_B);
    ANR_DUMP(hp1, ANR_HP_C);
    ANR_DUMP(hp1, ANR_HP_D);
    ANR_DUMP(hp1, ANR_HP_E);

    ANR_DUMP(hp2, ANR_HP_S1);
    ANR_DUMP(hp2, ANR_HP_S2);
    ANR_DUMP(hp2, ANR_HP_X1);
    ANR_DUMP(hp2, ANR_HP_F);

    ANR_DUMP(hp3, ANR_HP_Y_GAIN_CLIP);
    ANR_DUMP(hp3, ANR_HP_Y_SP);
    ANR_DUMP(hp3, ANR_HP_Y_LO);
    ANR_DUMP(hp3, ANR_HP_CLIP);

    ANR_DUMP(act1, ANR_ACT_LCE_GAIN);
    ANR_DUMP(act1, ANR_ACT_SCALE_OFT);
    ANR_DUMP(act1, ANR_ACT_SCALE_GAIN);
    ANR_DUMP(act1, ANR_ACT_DIF_GAIN);
    ANR_DUMP(act1, ANR_ACT_DIF_LO_TH);

    ANR_DUMP(act2, ANR_ACT_SIZE_GAIN);
    ANR_DUMP(act2, ANR_ACT_SIZE_LO_TH);
    ANR_DUMP(act2, ANR_COR_TH1);
    ANR_DUMP(act2, ANR_COR_SL1);

    ANR_DUMP(act3, ANR_COR_ACT_TH);
    ANR_DUMP(act3, ANR_COR_ACT_SL1);
    ANR_DUMP(act3, ANR_COR_ACT_SL2);

    ANR_DUMP(actc, ANR_ACT_TH_C);
    ANR_DUMP(actc, ANR_ACT_BLD_BASE_C);
    ANR_DUMP(actc, ANR_ACT_SLANT_C);
    ANR_DUMP(actc, ANR_ACT_BLD_TH_C);

    ANR_DUMP(actyl, ANR_ACT_TH_Y_L);
    ANR_DUMP(actyl, ANR_ACT_BLD_BASE_Y_L);
    ANR_DUMP(actyl, ANR_ACT_SLANT_Y_L);
    ANR_DUMP(actyl, ANR_ACT_BLD_TH_Y_L);

    ANR_DUMP(ylad, ANR_CEN_GAIN_LO_TH_LPF);
    ANR_DUMP(ylad, ANR_CEN_GAIN_HI_TH_LPF);
    ANR_DUMP(ylad, ANR_K_LMT);
    ANR_DUMP(ylad, ANR_K_LPF);

    ANR_DUMP(ptyl, ANR_PTY1_LPF);
    ANR_DUMP(ptyl, ANR_PTY2_LPF);
    ANR_DUMP(ptyl, ANR_PTY3_LPF);
    ANR_DUMP(ptyl, ANR_PTY4_LPF);

    ANR_DUMP(lcoef, ANR_COEF_A);
    ANR_DUMP(lcoef, ANR_COEF_B);
    ANR_DUMP(lcoef, ANR_COEF_C);
    ANR_DUMP(lcoef, ANR_COEF_D);

    // Olympus
    ANR_DUMP(con2, ANR_C_SM_EDGE_TH);
    ANR_DUMP(ylad, ANR_K_HPF);
    ANR_DUMP(act2, ANR_ACT_DIF_HI_TH);

    ANR_DUMP(actyhl, ANR_ACT_TH_Y_HL);
    ANR_DUMP(actyhl, ANR_ACT_BLD_BASE_Y_HL);
    ANR_DUMP(actyhl, ANR_ACT_SLANT_Y_HL);
    ANR_DUMP(actyhl, ANR_ACT_BLD_TH_Y_HL);

    ANR_DUMP(actyhh, ANR_ACT_TH_Y_HH);
    ANR_DUMP(actyhh, ANR_ACT_BLD_BASE_Y_HH);
    ANR_DUMP(actyhh, ANR_ACT_SLANT_Y_HH);
    ANR_DUMP(actyhh, ANR_ACT_BLD_TH_Y_HH);

    ANR_DUMP(actyl2, ANR_LUMA_TH_Y_L);
    ANR_DUMP(actyl2, ANR_LUMA_BLD_BASE_Y_L);
    ANR_DUMP(actyl2, ANR_LUMA_SLANT_Y_L);
    ANR_DUMP(actyl2, ANR_LUMA_BLD_TH_Y_L);

    ANR_DUMP(actyhl2, ANR_LUMA_TH_Y_HL);
    ANR_DUMP(actyhl2, ANR_LUMA_BLD_BASE_Y_HL);
    ANR_DUMP(actyhl2, ANR_LUMA_SLANT_Y_HL);
    ANR_DUMP(actyhl2, ANR_LUMA_BLD_TH_Y_HL);

    ANR_DUMP(actyhh2, ANR_LUMA_TH_Y_HH);
    ANR_DUMP(actyhh2, ANR_LUMA_BLD_BASE_Y_HH);
    ANR_DUMP(actyhh2, ANR_LUMA_SLANT_Y_HH);
    ANR_DUMP(actyhh2, ANR_LUMA_BLD_TH_Y_HH);

    ANR_DUMP(ylad2, ANR_PTY_L_GAIN);
    ANR_DUMP(ylad2, ANR_PTY_H_GAIN);
    ANR_DUMP(ylad2, ANR_BLEND_TH_L);
    ANR_DUMP(ylad2, ANR_BLEND_TH_H);

    ANR_DUMP(ylad3, ANR_HI_LMT1);
    ANR_DUMP(ylad3, ANR_HI_LMT2);
    ANR_DUMP(ylad3, ANR_LO_LMT1);
    ANR_DUMP(ylad3, ANR_LO_LMT2);

    ANR_DUMP(act4, ANR_Y_ACT_CEN_OFT);
    ANR_DUMP(act4, ANR_Y_ACT_CEN_GAIN);
    ANR_DUMP(act4, ANR_Y_ACT_CEN_TH);

    ANR_DUMP(con1, ANR_Y_DIR);
    ANR_DUMP(ydir, ANR_Y_DIR_GAIN);
    ANR_DUMP(ydir, ANR_Y_DIR_VER_W_TH);
    ANR_DUMP(ydir, ANR_Y_DIR_VER_W_SL);
    ANR_DUMP(ydir, ANR_Y_DIR_DIAG_W_TH);
    ANR_DUMP(ydir, ANR_Y_DIR_DIAG_W_SL);
#undef ANR_DUMP
}

MVOID SmoothANR2(MUINT32 u4RealISO,  // Real ISO
                 MUINT32 u4UpperISO, // Upper ISO
                 MUINT32 u4LowerISO, // Lower ISO
                 ISP_NVRAM_ANR2_T const& rUpperANR, // ANR settings for upper ISO
                 ISP_NVRAM_ANR2_T const& rLowerANR,   // ANR settings for lower ISO
                 ISP_NVRAM_ANR2_T& rSmoothANR)   // Output
{
	char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.smooth_anr.enable", value, "0");
    MBOOL bSmoothANRDebug = atoi(value);

    MY_LOG_IF(bSmoothANRDebug,"[%s()] u4RealISO: %d, u4UpperISO = %d, u4LowerISO = %d\n", __FUNCTION__, u4RealISO, u4UpperISO, u4LowerISO);

    //    ====================================================================================================================================
    //    Get Lower ISO setting
    //    ====================================================================================================================================
    // CAM_ANR2_CON1 CAM+A20H
    MINT32 ANR2_ENC_low           = rLowerANR.con1.bits.ANR2_ENC;
    MINT32 ANR2_ENY_low           = rLowerANR.con1.bits.ANR2_ENY;
    MINT32 ANR2_SCALE_MODE_low    = rLowerANR.con1.bits.ANR2_SCALE_MODE;
    MINT32 ANR2_MODE_low          = rLowerANR.con1.bits.ANR2_MODE;
    MINT32 ANR2_LCE_LINK_low      = rLowerANR.con1.bits.ANR2_LCE_LINK;

    // CAM_ANR2_CON2 CAM+A24H
    MINT32 ANR2_C_MED_EN_low      = rLowerANR.con2.bits.ANR2_C_MED_EN;
    MINT32 ANR2_C_SM_EDGE_low     = rLowerANR.con2.bits.ANR2_C_SM_EDGE;
    MINT32 ANR2_FLT_C_low         = rLowerANR.con2.bits.ANR2_FLT_C;

    // CAM_ANR2_YAD1 CAM+A2CH
    MINT32 ANR2_K_TH_C_low        = rLowerANR.yad1.bits.ANR2_K_TH_C;

    // CAM_ANR2_Y4LUT1 CAM+A34H
    MINT32 ANR2_Y_CPX1_low        = rLowerANR.lut1.bits.ANR2_Y_CPX1;
    MINT32 ANR2_Y_CPX2_low        = rLowerANR.lut1.bits.ANR2_Y_CPX2;
    MINT32 ANR2_Y_CPX3_low        = rLowerANR.lut1.bits.ANR2_Y_CPX3;

    // CAM_ANR2_Y4LUT2 CAM+A38H
    MINT32 ANR2_Y_SCALE_CPY0_low  = rLowerANR.lut2.bits.ANR2_Y_SCALE_CPY0;
    MINT32 ANR2_Y_SCALE_CPY1_low  = rLowerANR.lut2.bits.ANR2_Y_SCALE_CPY1;
    MINT32 ANR2_Y_SCALE_CPY2_low  = rLowerANR.lut2.bits.ANR2_Y_SCALE_CPY2;
    MINT32 ANR2_Y_SCALE_CPY3_low  = rLowerANR.lut2.bits.ANR2_Y_SCALE_CPY3;

    // CAM_ANR2_Y4LUT3 CAM+A3CH
    MINT32 ANR2_Y_SCALE_SP0_low   = rLowerANR.lut3.bits.ANR2_Y_SCALE_SP0;
    MINT32 ANR2_Y_SCALE_SP1_low   = rLowerANR.lut3.bits.ANR2_Y_SCALE_SP1;
    MINT32 ANR2_Y_SCALE_SP2_low   = rLowerANR.lut3.bits.ANR2_Y_SCALE_SP2;
    MINT32 ANR2_Y_SCALE_SP3_low   = rLowerANR.lut3.bits.ANR2_Y_SCALE_SP3;

    // CAM_ANR2_L4LUT1
    MINT32 ANR2_LCE_X1_low        = rLowerANR.llut1.bits.ANR2_LCE_X1;
    MINT32 ANR2_LCE_X2_low        = rLowerANR.llut1.bits.ANR2_LCE_X2;
    MINT32 ANR2_LCE_X3_low        = rLowerANR.llut1.bits.ANR2_LCE_X3;

    // CAM_ANR2_L4LUT2
    MINT32 ANR2_LCE_GAIN0_low     = rLowerANR.llut2.bits.ANR2_LCE_GAIN0;
    MINT32 ANR2_LCE_GAIN1_low     = rLowerANR.llut2.bits.ANR2_LCE_GAIN1;
    MINT32 ANR2_LCE_GAIN2_low     = rLowerANR.llut2.bits.ANR2_LCE_GAIN2;
    MINT32 ANR2_LCE_GAIN3_low     = rLowerANR.llut2.bits.ANR2_LCE_GAIN3;

    // CAM_ANR2_L4LUT3
    MINT32 ANR2_LCE_SP0_low       = rLowerANR.llut3.bits.ANR2_LCE_SP0;
    MINT32 ANR2_LCE_SP1_low       = rLowerANR.llut3.bits.ANR2_LCE_SP1;
    MINT32 ANR2_LCE_SP2_low       = rLowerANR.llut3.bits.ANR2_LCE_SP2;
    MINT32 ANR2_LCE_SP3_low       = rLowerANR.llut3.bits.ANR2_LCE_SP3;

    // CAM_ANR2_CAD CAM+A44H
    MINT32 ANR2_PTC_VGAIN_low     = rLowerANR.cad.bits.ANR2_PTC_VGAIN;
    MINT32 ANR2_C_GAIN_low        = rLowerANR.cad.bits.ANR2_C_GAIN;
    MINT32 ANR2_PTC_GAIN_TH_low   = rLowerANR.cad.bits.ANR2_PTC_GAIN_TH;
    MINT32 ANR2_C_L_DIFF_TH_low   = rLowerANR.cad.bits.ANR2_C_L_DIFF_TH;
    MINT32 ANR2_C_MODE_low        = rLowerANR.cad.bits.ANR2_C_MODE;

    // CAM_ANR2_PTC CAM+A48H
    MINT32 ANR2_PTC1_low          = rLowerANR.ptc.bits.ANR2_PTC1;
    MINT32 ANR2_PTC2_low          = rLowerANR.ptc.bits.ANR2_PTC2;
    MINT32 ANR2_PTC3_low          = rLowerANR.ptc.bits.ANR2_PTC3;
    MINT32 ANR2_PTC4_low          = rLowerANR.ptc.bits.ANR2_PTC4;

    // CAM_ANR2_LCE CAM+A4CH
    MINT32 ANR2_LCE_C_GAIN_low    = rLowerANR.lce.bits.ANR2_LCE_C_GAIN;
    MINT32 ANR2_LM_WT_low         = rLowerANR.lce.bits.ANR2_LM_WT;

    // CAM_ANR2_MED1
    MINT32 ANR2_COR_TH_low        = rLowerANR.med1.bits.ANR2_COR_TH;
    MINT32 ANR2_COR_SL_low        = rLowerANR.med1.bits.ANR2_COR_SL;
    MINT32 ANR2_MCD_TH_low        = rLowerANR.med1.bits.ANR2_MCD_TH;
    MINT32 ANR2_MCD_SL_low        = rLowerANR.med1.bits.ANR2_MCD_SL;
    MINT32 ANR2_LCL_TH_low        = rLowerANR.med1.bits.ANR2_LCL_TH;

    // CAM_ANR2_MED2
    MINT32 ANR2_LCL_SL_low        = rLowerANR.med2.bits.ANR2_LCL_SL;
    MINT32 ANR2_LCL_LV_low        = rLowerANR.med2.bits.ANR2_LCL_LV;
    MINT32 ANR2_SCL_TH_low        = rLowerANR.med2.bits.ANR2_SCL_TH;
    MINT32 ANR2_SCL_SL_low        = rLowerANR.med2.bits.ANR2_SCL_SL;
    MINT32 ANR2_SCL_LV_low        = rLowerANR.med2.bits.ANR2_SCL_LV;

    // CAM_ANR2_MED3
    MINT32 ANR2_NCL_TH_low        = rLowerANR.med3.bits.ANR2_NCL_TH;
    MINT32 ANR2_NCL_SL_low        = rLowerANR.med3.bits.ANR2_NCL_SL;
    MINT32 ANR2_NCL_LV_low        = rLowerANR.med3.bits.ANR2_NCL_LV;
    MINT32 ANR2_VAR_low           = rLowerANR.med3.bits.ANR2_VAR;
    MINT32 ANR2_Y0_low            = rLowerANR.med3.bits.ANR2_Y0;

    // CAM_ANR2_MED4
    MINT32 ANR2_Y1_low            = rLowerANR.med4.bits.ANR2_Y1;
    MINT32 ANR2_Y2_low            = rLowerANR.med4.bits.ANR2_Y2;
    MINT32 ANR2_Y3_low            = rLowerANR.med4.bits.ANR2_Y3;
    MINT32 ANR2_Y4_low            = rLowerANR.med4.bits.ANR2_Y4;

    // CAM_ANR2_ACTY CAM+A60H
    MINT32 ANR2_ACT_TH_Y_low      = rLowerANR.acty.bits.ANR2_ACT_TH_Y;
    MINT32 ANR2_ACT_BLD_BASE_Y_low= rLowerANR.acty.bits.ANR2_ACT_BLD_BASE_Y;
    MINT32 ANR2_ACT_SLANT_Y_low   = rLowerANR.acty.bits.ANR2_ACT_SLANT_Y;
    MINT32 ANR2_ACT_BLD_TH_Y_low  = rLowerANR.acty.bits.ANR2_ACT_BLD_TH_Y;

    // CAM_ANR2_ACTC CAM+A64H
    MINT32 ANR2_ACT_TH_C_low      = rLowerANR.actc.bits.ANR2_ACT_TH_C;
    MINT32 ANR2_ACT_BLD_BASE_C_low= rLowerANR.actc.bits.ANR2_ACT_BLD_BASE_C;
    MINT32 ANR2_ACT_SLANT_C_low   = rLowerANR.actc.bits.ANR2_ACT_SLANT_C;
    MINT32 ANR2_ACT_BLD_TH_C_low  = rLowerANR.actc.bits.ANR2_ACT_BLD_TH_C;

    //    ====================================================================================================================================
    //    Get Upper ISO setting
    //    ====================================================================================================================================
    // CAM_ANR2_CON1 CAM+A20H
    MINT32 ANR2_ENC_high           = rUpperANR.con1.bits.ANR2_ENC;
    MINT32 ANR2_ENY_high           = rUpperANR.con1.bits.ANR2_ENY;
    MINT32 ANR2_SCALE_MODE_high    = rUpperANR.con1.bits.ANR2_SCALE_MODE;
    MINT32 ANR2_MODE_high          = rUpperANR.con1.bits.ANR2_MODE;
    MINT32 ANR2_LCE_LINK_high      = rUpperANR.con1.bits.ANR2_LCE_LINK;

    // CAM_ANR2_CON2 CAM+A24H
    MINT32 ANR2_C_MED_EN_high      = rUpperANR.con2.bits.ANR2_C_MED_EN;
    MINT32 ANR2_C_SM_EDGE_high     = rUpperANR.con2.bits.ANR2_C_SM_EDGE;
    MINT32 ANR2_FLT_C_high         = rUpperANR.con2.bits.ANR2_FLT_C;

    // CAM_ANR2_YAD1 CAM+A2CH
    MINT32 ANR2_K_TH_C_high        = rUpperANR.yad1.bits.ANR2_K_TH_C;

    // CAM_ANR2_Y4LUT1 CAM+A34H
    MINT32 ANR2_Y_CPX1_high        = rUpperANR.lut1.bits.ANR2_Y_CPX1;
    MINT32 ANR2_Y_CPX2_high        = rUpperANR.lut1.bits.ANR2_Y_CPX2;
    MINT32 ANR2_Y_CPX3_high        = rUpperANR.lut1.bits.ANR2_Y_CPX3;

    // CAM_ANR2_Y4LUT2 CAM+A38H
    MINT32 ANR2_Y_SCALE_CPY0_high  = rUpperANR.lut2.bits.ANR2_Y_SCALE_CPY0;
    MINT32 ANR2_Y_SCALE_CPY1_high  = rUpperANR.lut2.bits.ANR2_Y_SCALE_CPY1;
    MINT32 ANR2_Y_SCALE_CPY2_high  = rUpperANR.lut2.bits.ANR2_Y_SCALE_CPY2;
    MINT32 ANR2_Y_SCALE_CPY3_high  = rUpperANR.lut2.bits.ANR2_Y_SCALE_CPY3;

    // CAM_ANR2_Y4LUT3 CAM+A3CH
    MINT32 ANR2_Y_SCALE_SP0_high   = rUpperANR.lut3.bits.ANR2_Y_SCALE_SP0;
    MINT32 ANR2_Y_SCALE_SP1_high   = rUpperANR.lut3.bits.ANR2_Y_SCALE_SP1;
    MINT32 ANR2_Y_SCALE_SP2_high   = rUpperANR.lut3.bits.ANR2_Y_SCALE_SP2;
    MINT32 ANR2_Y_SCALE_SP3_high   = rUpperANR.lut3.bits.ANR2_Y_SCALE_SP3;

    // CAM_ANR2_L4LUT1
    MINT32 ANR2_LCE_X1_high        = rUpperANR.llut1.bits.ANR2_LCE_X1;
    MINT32 ANR2_LCE_X2_high        = rUpperANR.llut1.bits.ANR2_LCE_X2;
    MINT32 ANR2_LCE_X3_high        = rUpperANR.llut1.bits.ANR2_LCE_X3;

    // CAM_ANR2_L4LUT2
    MINT32 ANR2_LCE_GAIN0_high     = rUpperANR.llut2.bits.ANR2_LCE_GAIN0;
    MINT32 ANR2_LCE_GAIN1_high     = rUpperANR.llut2.bits.ANR2_LCE_GAIN1;
    MINT32 ANR2_LCE_GAIN2_high     = rUpperANR.llut2.bits.ANR2_LCE_GAIN2;
    MINT32 ANR2_LCE_GAIN3_high     = rUpperANR.llut2.bits.ANR2_LCE_GAIN3;

    // CAM_ANR2_L4LUT3
    MINT32 ANR2_LCE_SP0_high       = rUpperANR.llut3.bits.ANR2_LCE_SP0;
    MINT32 ANR2_LCE_SP1_high       = rUpperANR.llut3.bits.ANR2_LCE_SP1;
    MINT32 ANR2_LCE_SP2_high       = rUpperANR.llut3.bits.ANR2_LCE_SP2;
    MINT32 ANR2_LCE_SP3_high       = rUpperANR.llut3.bits.ANR2_LCE_SP3;

    // CAM_ANR2_CAD CAM+A44H
    MINT32 ANR2_PTC_VGAIN_high     = rUpperANR.cad.bits.ANR2_PTC_VGAIN;
    MINT32 ANR2_C_GAIN_high        = rUpperANR.cad.bits.ANR2_C_GAIN;
    MINT32 ANR2_PTC_GAIN_TH_high   = rUpperANR.cad.bits.ANR2_PTC_GAIN_TH;
    MINT32 ANR2_C_L_DIFF_TH_high   = rUpperANR.cad.bits.ANR2_C_L_DIFF_TH;
    MINT32 ANR2_C_MODE_high        = rUpperANR.cad.bits.ANR2_C_MODE;

    // CAM_ANR2_PTC CAM+A48H
    MINT32 ANR2_PTC1_high          = rUpperANR.ptc.bits.ANR2_PTC1;
    MINT32 ANR2_PTC2_high          = rUpperANR.ptc.bits.ANR2_PTC2;
    MINT32 ANR2_PTC3_high          = rUpperANR.ptc.bits.ANR2_PTC3;
    MINT32 ANR2_PTC4_high          = rUpperANR.ptc.bits.ANR2_PTC4;

    // CAM_ANR2_LCE CAM+A4CH
    MINT32 ANR2_LCE_C_GAIN_high    = rUpperANR.lce.bits.ANR2_LCE_C_GAIN;
    MINT32 ANR2_LM_WT_high         = rUpperANR.lce.bits.ANR2_LM_WT;

    // CAM_ANR2_MED1
    MINT32 ANR2_COR_TH_high        = rUpperANR.med1.bits.ANR2_COR_TH;
    MINT32 ANR2_COR_SL_high        = rUpperANR.med1.bits.ANR2_COR_SL;
    MINT32 ANR2_MCD_TH_high        = rUpperANR.med1.bits.ANR2_MCD_TH;
    MINT32 ANR2_MCD_SL_high        = rUpperANR.med1.bits.ANR2_MCD_SL;
    MINT32 ANR2_LCL_TH_high        = rUpperANR.med1.bits.ANR2_LCL_TH;

    // CAM_ANR2_MED2
    MINT32 ANR2_LCL_SL_high        = rUpperANR.med2.bits.ANR2_LCL_SL;
    MINT32 ANR2_LCL_LV_high        = rUpperANR.med2.bits.ANR2_LCL_LV;
    MINT32 ANR2_SCL_TH_high        = rUpperANR.med2.bits.ANR2_SCL_TH;
    MINT32 ANR2_SCL_SL_high        = rUpperANR.med2.bits.ANR2_SCL_SL;
    MINT32 ANR2_SCL_LV_high        = rUpperANR.med2.bits.ANR2_SCL_LV;

    // CAM_ANR2_MED3
    MINT32 ANR2_NCL_TH_high        = rUpperANR.med3.bits.ANR2_NCL_TH;
    MINT32 ANR2_NCL_SL_high        = rUpperANR.med3.bits.ANR2_NCL_SL;
    MINT32 ANR2_NCL_LV_high        = rUpperANR.med3.bits.ANR2_NCL_LV;
    MINT32 ANR2_VAR_high           = rUpperANR.med3.bits.ANR2_VAR;
    MINT32 ANR2_Y0_high            = rUpperANR.med3.bits.ANR2_Y0;

    // CAM_ANR2_MED4
    MINT32 ANR2_Y1_high            = rUpperANR.med4.bits.ANR2_Y1;
    MINT32 ANR2_Y2_high            = rUpperANR.med4.bits.ANR2_Y2;
    MINT32 ANR2_Y3_high            = rUpperANR.med4.bits.ANR2_Y3;
    MINT32 ANR2_Y4_high            = rUpperANR.med4.bits.ANR2_Y4;

    // CAM_ANR2_ACTY CAM+A60H
    MINT32 ANR2_ACT_TH_Y_high      = rUpperANR.acty.bits.ANR2_ACT_TH_Y;
    MINT32 ANR2_ACT_BLD_BASE_Y_high= rUpperANR.acty.bits.ANR2_ACT_BLD_BASE_Y;
    MINT32 ANR2_ACT_SLANT_Y_high   = rUpperANR.acty.bits.ANR2_ACT_SLANT_Y;
    MINT32 ANR2_ACT_BLD_TH_Y_high  = rUpperANR.acty.bits.ANR2_ACT_BLD_TH_Y;

    // CAM_ANR2_ACTC CAM+A64H
    MINT32 ANR2_ACT_TH_C_high      = rUpperANR.actc.bits.ANR2_ACT_TH_C;
    MINT32 ANR2_ACT_BLD_BASE_C_high= rUpperANR.actc.bits.ANR2_ACT_BLD_BASE_C;
    MINT32 ANR2_ACT_SLANT_C_high   = rUpperANR.actc.bits.ANR2_ACT_SLANT_C;
    MINT32 ANR2_ACT_BLD_TH_C_high  = rUpperANR.actc.bits.ANR2_ACT_BLD_TH_C;

    //    ====================================================================================================================================
    //    Interpolating reg declaration
    //    ====================================================================================================================================
    // CAM_ANR2_CON1 CAM+A20H
    MINT32 ANR2_ENC;
    MINT32 ANR2_ENY;
    MINT32 ANR2_SCALE_MODE;
    MINT32 ANR2_MODE;
    MINT32 ANR2_LCE_LINK;

    // CAM_ANR2_CON2 CAM+A24H
    MINT32 ANR2_C_MED_EN;
    MINT32 ANR2_C_SM_EDGE;
    MINT32 ANR2_FLT_C;

    // CAM_ANR2_YAD1 CAM+A2CH
    MINT32 ANR2_K_TH_C;

    // CAM_ANR2_Y4LUT1 CAM+A34H
    MINT32 ANR2_Y_CPX1;
    MINT32 ANR2_Y_CPX2;
    MINT32 ANR2_Y_CPX3;

    // CAM_ANR2_Y4LUT2 CAM+A38H
    MINT32 ANR2_Y_SCALE_CPY0;
    MINT32 ANR2_Y_SCALE_CPY1;
    MINT32 ANR2_Y_SCALE_CPY2;
    MINT32 ANR2_Y_SCALE_CPY3;

    // CAM_ANR2_Y4LUT3 CAM+A3CH
    MINT32 ANR2_Y_SCALE_SP0;
    MINT32 ANR2_Y_SCALE_SP1;
    MINT32 ANR2_Y_SCALE_SP2;
    MINT32 ANR2_Y_SCALE_SP3;

    // CAM_ANR2_L4LUT1
    MINT32 ANR2_LCE_X1;
    MINT32 ANR2_LCE_X2;
    MINT32 ANR2_LCE_X3;

    // CAM_ANR2_L4LUT2
    MINT32 ANR2_LCE_GAIN0;
    MINT32 ANR2_LCE_GAIN1;
    MINT32 ANR2_LCE_GAIN2;
    MINT32 ANR2_LCE_GAIN3;

    // CAM_ANR2_L4LUT3
    MINT32 ANR2_LCE_SP0;
    MINT32 ANR2_LCE_SP1;
    MINT32 ANR2_LCE_SP2;
    MINT32 ANR2_LCE_SP3;

    // CAM_ANR2_CAD CAM+A44H
    MINT32 ANR2_PTC_VGAIN;
    MINT32 ANR2_C_GAIN;
    MINT32 ANR2_PTC_GAIN_TH;
    MINT32 ANR2_C_L_DIFF_TH;
    MINT32 ANR2_C_MODE;

    // CAM_ANR2_PTC CAM+A48H
    MINT32 ANR2_PTC1;
    MINT32 ANR2_PTC2;
    MINT32 ANR2_PTC3;
    MINT32 ANR2_PTC4;

    // CAM_ANR2_LCE CAM+A4CH
    MINT32 ANR2_LCE_C_GAIN;
    MINT32 ANR2_LM_WT;

    // CAM_ANR2_MED1
    MINT32 ANR2_COR_TH;
    MINT32 ANR2_COR_SL;
    MINT32 ANR2_MCD_TH;
    MINT32 ANR2_MCD_SL;
    MINT32 ANR2_LCL_TH;

    // CAM_ANR2_MED2
    MINT32 ANR2_LCL_SL;
    MINT32 ANR2_LCL_LV;
    MINT32 ANR2_SCL_TH;
    MINT32 ANR2_SCL_SL;
    MINT32 ANR2_SCL_LV;

    // CAM_ANR2_MED3
    MINT32 ANR2_NCL_TH;
    MINT32 ANR2_NCL_SL;
    MINT32 ANR2_NCL_LV;
    MINT32 ANR2_VAR;
    MINT32 ANR2_Y0;

    // CAM_ANR2_MED4
    MINT32 ANR2_Y1;
    MINT32 ANR2_Y2;
    MINT32 ANR2_Y3;
    MINT32 ANR2_Y4;

    // CAM_ANR2_ACTY CAM+A60H
    MINT32 ANR2_ACT_TH_Y;
    MINT32 ANR2_ACT_BLD_BASE_Y;
    MINT32 ANR2_ACT_SLANT_Y;
    MINT32 ANR2_ACT_BLD_TH_Y;

    // CAM_ANR2_ACTC CAM+A64H
    MINT32 ANR2_ACT_TH_C;
    MINT32 ANR2_ACT_BLD_BASE_C;
    MINT32 ANR2_ACT_SLANT_C;
    MINT32 ANR2_ACT_BLD_TH_C;

    //    ====================================================================================================================================
    //    Negative Conversion
    //    ====================================================================================================================================
    ANR2_Y_SCALE_SP0_low = (ANR2_Y_SCALE_SP0_low > 15) ? (ANR2_Y_SCALE_SP0_low - 32) : ANR2_Y_SCALE_SP0_low;
    ANR2_Y_SCALE_SP1_low = (ANR2_Y_SCALE_SP1_low > 15) ? (ANR2_Y_SCALE_SP1_low - 32) : ANR2_Y_SCALE_SP1_low;
    ANR2_Y_SCALE_SP2_low = (ANR2_Y_SCALE_SP2_low > 15) ? (ANR2_Y_SCALE_SP2_low - 32) : ANR2_Y_SCALE_SP2_low;
    ANR2_Y_SCALE_SP3_low = (ANR2_Y_SCALE_SP3_low > 15) ? (ANR2_Y_SCALE_SP3_low - 32) : ANR2_Y_SCALE_SP3_low;
    ANR2_LCE_SP0_low     = (ANR2_LCE_SP0_low > 31) ? (ANR2_LCE_SP0_low - 64) : ANR2_LCE_SP0_low;
    ANR2_LCE_SP1_low     = (ANR2_LCE_SP1_low > 31) ? (ANR2_LCE_SP1_low - 64) : ANR2_LCE_SP1_low;
    ANR2_LCE_SP2_low     = (ANR2_LCE_SP2_low > 31) ? (ANR2_LCE_SP2_low - 64) : ANR2_LCE_SP2_low;
    ANR2_LCE_SP3_low     = (ANR2_LCE_SP3_low > 31) ? (ANR2_LCE_SP3_low - 64) : ANR2_LCE_SP3_low;

    ANR2_Y_SCALE_SP0_high = (ANR2_Y_SCALE_SP0_high > 15) ? (ANR2_Y_SCALE_SP0_high - 32) : ANR2_Y_SCALE_SP0_high;
    ANR2_Y_SCALE_SP1_high = (ANR2_Y_SCALE_SP1_high > 15) ? (ANR2_Y_SCALE_SP1_high - 32) : ANR2_Y_SCALE_SP1_high;
    ANR2_Y_SCALE_SP2_high = (ANR2_Y_SCALE_SP2_high > 15) ? (ANR2_Y_SCALE_SP2_high - 32) : ANR2_Y_SCALE_SP2_high;
    ANR2_Y_SCALE_SP3_high = (ANR2_Y_SCALE_SP3_high > 15) ? (ANR2_Y_SCALE_SP3_high - 32) : ANR2_Y_SCALE_SP3_high;
    ANR2_LCE_SP0_high     = (ANR2_LCE_SP0_high > 31) ? (ANR2_LCE_SP0_high - 64) : ANR2_LCE_SP0_high;
    ANR2_LCE_SP1_high     = (ANR2_LCE_SP1_high > 31) ? (ANR2_LCE_SP1_high - 64) : ANR2_LCE_SP1_high;
    ANR2_LCE_SP2_high     = (ANR2_LCE_SP2_high > 31) ? (ANR2_LCE_SP2_high - 64) : ANR2_LCE_SP2_high;
    ANR2_LCE_SP3_high     = (ANR2_LCE_SP3_high > 31) ? (ANR2_LCE_SP3_high - 64) : ANR2_LCE_SP3_high;

    //    ====================================================================================================================================
    //    Start Parameter Interpolation
    //    ====================================================================================================================================
    MINT32 ISO = u4RealISO;
    MINT32 ISO_low = u4LowerISO;
    MINT32 ISO_high = u4UpperISO;

    if(ANR2_ENC_low==ANR2_ENC_high && ANR2_ENY_low==ANR2_ENY_high)
    {
        //    Registers that can not be interpolated
        ANR2_ENC            = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_ENC_low             , ANR2_ENC_high, 1);
        ANR2_ENY            = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_ENY_low             , ANR2_ENY_high, 1);
        ANR2_MODE           = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_MODE_low            , ANR2_MODE_high, 1);
        ANR2_C_MED_EN       = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_C_MED_EN_low        , ANR2_C_MED_EN_high, 1);
        ANR2_C_SM_EDGE      = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_C_SM_EDGE_low       , ANR2_C_SM_EDGE_high, 1);
        ANR2_FLT_C          = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_FLT_C_low           , ANR2_FLT_C_high, 1);

        //    Registers that can be interpolated
        ANR2_Y_CPX1         = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_Y_CPX1_low          , ANR2_Y_CPX1_high, 0);
        ANR2_Y_CPX2         = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_Y_CPX2_low          , ANR2_Y_CPX2_high, 0);
        ANR2_Y_CPX3         = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_Y_CPX3_low          , ANR2_Y_CPX3_high, 0);
        ANR2_Y_SCALE_CPY0   = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_Y_SCALE_CPY0_low    , ANR2_Y_SCALE_CPY0_high, 0);
        ANR2_Y_SCALE_CPY1   = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_Y_SCALE_CPY1_low    , ANR2_Y_SCALE_CPY1_high, 0);
        ANR2_Y_SCALE_CPY2   = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_Y_SCALE_CPY2_low    , ANR2_Y_SCALE_CPY2_high, 0);
        ANR2_Y_SCALE_CPY3   = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_Y_SCALE_CPY3_low    , ANR2_Y_SCALE_CPY3_high, 0);
        ANR2_PTC_VGAIN      = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_PTC_VGAIN_low       , ANR2_PTC_VGAIN_high, 0);
        ANR2_C_GAIN         = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_C_GAIN_low          , ANR2_C_GAIN_high, 0);
        ANR2_PTC_GAIN_TH    = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_PTC_GAIN_TH_low     , ANR2_PTC_GAIN_TH_high, 0);
        ANR2_C_L_DIFF_TH    = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_C_L_DIFF_TH_low     , ANR2_C_L_DIFF_TH_high, 0);
        ANR2_C_MODE         = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_C_MODE_low          , ANR2_C_MODE_high, 0);
        ANR2_PTC1           = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_PTC1_low            , ANR2_PTC1_high, 0);
        ANR2_PTC2           = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_PTC2_low            , ANR2_PTC2_high, 0);
        ANR2_PTC3           = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_PTC3_low            , ANR2_PTC3_high, 0);
        ANR2_PTC4           = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_PTC4_low            , ANR2_PTC4_high, 0);
        ANR2_LM_WT          = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_LM_WT_low           , ANR2_LM_WT_high, 0);
        ANR2_ACT_TH_Y       = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_ACT_TH_Y_low        , ANR2_ACT_TH_Y_high, 0);
        ANR2_ACT_BLD_BASE_Y = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_ACT_BLD_BASE_Y_low  , ANR2_ACT_BLD_BASE_Y_high, 0);
        ANR2_ACT_SLANT_Y    = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_ACT_SLANT_Y_low     , ANR2_ACT_SLANT_Y_high, 0);
        ANR2_ACT_BLD_TH_Y   = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_ACT_BLD_TH_Y_low    , ANR2_ACT_BLD_TH_Y_high, 0);
        ANR2_ACT_TH_C       = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_ACT_TH_C_low        , ANR2_ACT_TH_C_high, 0);
        ANR2_ACT_BLD_BASE_C = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_ACT_BLD_BASE_C_low  , ANR2_ACT_BLD_BASE_C_high, 0);
        ANR2_ACT_SLANT_C    = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_ACT_SLANT_C_low     , ANR2_ACT_SLANT_C_high, 0);
        ANR2_ACT_BLD_TH_C   = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_ACT_BLD_TH_C_low    , ANR2_ACT_BLD_TH_C_high, 0);

        // ANR2_SCALE_MODE
        ANR2_SCALE_MODE     = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_SCALE_MODE_low      , ANR2_SCALE_MODE_high, 1);
        ANR2_K_TH_C         = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_K_TH_C_low          , ANR2_K_TH_C_high, 1);

        // ANR2_LCE_LINK
        if(0 == ANR2_LCE_LINK_low && 1 == ANR2_LCE_LINK_high)
        {
            ANR2_LCE_LINK_low  = ANR2_LCE_LINK_high;
            ANR2_LCE_X1_low    = ANR2_LCE_X1_high;
            ANR2_LCE_X2_low    = ANR2_LCE_X2_high;
            ANR2_LCE_X3_low    = ANR2_LCE_X3_high;
            ANR2_LCE_GAIN0_low = 16;
            ANR2_LCE_GAIN1_low = 16;
            ANR2_LCE_GAIN2_low = 16;
            ANR2_LCE_GAIN3_low = 16;
            ANR2_LCE_SP0_low   = 0;
            ANR2_LCE_SP1_low   = 0;
            ANR2_LCE_SP2_low   = 0;
            ANR2_LCE_SP3_low   = 0;
            ANR2_LCE_C_GAIN_low = 4;
        }
        if(0 == ANR2_LCE_LINK_high && 1 == ANR2_LCE_LINK_low)
        {
            ANR2_LCE_LINK_high  = ANR2_LCE_LINK_low;
            ANR2_LCE_X1_high    = ANR2_LCE_X1_low;
            ANR2_LCE_X2_high    = ANR2_LCE_X2_low;
            ANR2_LCE_X3_high    = ANR2_LCE_X3_low;
            ANR2_LCE_GAIN0_high = 16;
            ANR2_LCE_GAIN1_high = 16;
            ANR2_LCE_GAIN2_high = 16;
            ANR2_LCE_GAIN3_high = 16;
            ANR2_LCE_SP0_high   = 0;
            ANR2_LCE_SP1_high   = 0;
            ANR2_LCE_SP2_high   = 0;
            ANR2_LCE_SP3_high   = 0;
            ANR2_LCE_C_GAIN_high = 4;
        }

        ANR2_LCE_LINK       = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_LCE_LINK_low      , ANR2_LCE_LINK_high, 0);
        ANR2_LCE_X1         = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_LCE_X1_low        , ANR2_LCE_X1_high, 0);
        ANR2_LCE_X2         = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_LCE_X2_low        , ANR2_LCE_X2_high, 0);
        ANR2_LCE_X3         = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_LCE_X3_low        , ANR2_LCE_X3_high, 0);
        ANR2_LCE_GAIN0      = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_LCE_GAIN0_low     , ANR2_LCE_GAIN0_high, 0);
        ANR2_LCE_GAIN1      = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_LCE_GAIN1_low     , ANR2_LCE_GAIN1_high, 0);
        ANR2_LCE_GAIN2      = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_LCE_GAIN2_low     , ANR2_LCE_GAIN2_high, 0);
        ANR2_LCE_GAIN3      = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_LCE_GAIN3_low     , ANR2_LCE_GAIN3_high, 0);
        ANR2_LCE_C_GAIN     = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_LCE_C_GAIN_low    , ANR2_LCE_C_GAIN_high, 0);

        // SP Calculation
        ANR2_Y_SCALE_SP0    = CalculateLUTSP(         0, ANR2_Y_CPX1, ANR2_Y_SCALE_CPY0, ANR2_Y_SCALE_CPY1, 128, 15, -15);
        ANR2_Y_SCALE_SP1    = CalculateLUTSP(ANR2_Y_CPX1, ANR2_Y_CPX2, ANR2_Y_SCALE_CPY1, ANR2_Y_SCALE_CPY2, 128, 15, -15);
        ANR2_Y_SCALE_SP2    = CalculateLUTSP(ANR2_Y_CPX2, ANR2_Y_CPX3, ANR2_Y_SCALE_CPY2, ANR2_Y_SCALE_CPY3, 128, 15, -15);
        ANR2_Y_SCALE_SP3    = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_Y_SCALE_SP3_low     , ANR2_Y_SCALE_SP3_high, 0);
        ANR2_LCE_SP0        = CalculateLUTSP(         0, ANR2_LCE_X1, ANR2_LCE_GAIN0   , ANR2_LCE_GAIN1   , 128, 31, -31);
        ANR2_LCE_SP1        = CalculateLUTSP(ANR2_LCE_X1, ANR2_LCE_X2, ANR2_LCE_GAIN1   , ANR2_LCE_GAIN2   , 128, 31, -31);
        ANR2_LCE_SP2        = CalculateLUTSP(ANR2_LCE_X2, ANR2_LCE_X3, ANR2_LCE_GAIN2   , ANR2_LCE_GAIN3   , 128, 31, -31);
        ANR2_LCE_SP3        = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_LCE_SP3_low       , ANR2_LCE_SP3_high, 0);

        // ANR2_MEDIAN_EN
        ANR2_COR_TH      = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_COR_TH_low   , ANR2_COR_TH_high, 0);
        ANR2_COR_SL      = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_COR_SL_low   , ANR2_COR_SL_high, 0);
        ANR2_MCD_TH      = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_MCD_TH_low   , ANR2_MCD_TH_high, 0);
        ANR2_MCD_SL      = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_MCD_SL_low   , ANR2_MCD_SL_high, 0);
        ANR2_LCL_TH      = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_LCL_TH_low   , ANR2_LCL_TH_high, 0);
        ANR2_LCL_SL      = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_LCL_SL_low   , ANR2_LCL_SL_high, 0);
        ANR2_LCL_LV      = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_LCL_LV_low   , ANR2_LCL_LV_high, 0);
        ANR2_SCL_TH      = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_SCL_TH_low   , ANR2_SCL_TH_high, 0);
        ANR2_SCL_SL      = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_SCL_SL_low   , ANR2_SCL_SL_high, 0);
        ANR2_SCL_LV      = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_SCL_LV_low   , ANR2_SCL_LV_high, 0);
        ANR2_NCL_TH      = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_NCL_TH_low   , ANR2_NCL_TH_high, 0);
        ANR2_NCL_SL      = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_NCL_SL_low   , ANR2_NCL_SL_high, 0);
        ANR2_NCL_LV      = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_NCL_LV_low   , ANR2_NCL_LV_high, 0);
        ANR2_VAR         = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_VAR_low      , ANR2_VAR_high, 0);
        ANR2_Y0          = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_Y0_low       , ANR2_Y0_high, 0);
        ANR2_Y1          = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_Y1_low       , ANR2_Y1_high, 0);
        ANR2_Y2          = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_Y2_low       , ANR2_Y2_high, 0);
        ANR2_Y3          = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_Y3_low       , ANR2_Y3_high, 0);
        ANR2_Y4          = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_Y4_low       , ANR2_Y4_high, 0);
    }
    else
    {
        //    Registers that can not be interpolated
        ANR2_ENC            = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_ENC_low             , ANR2_ENC_high, 1);
        ANR2_ENY            = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_ENY_low             , ANR2_ENY_high, 1);
        ANR2_MODE           = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_MODE_low            , ANR2_MODE_high, 1);
        ANR2_C_MED_EN       = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_C_MED_EN_low        , ANR2_C_MED_EN_high, 1);
        ANR2_C_SM_EDGE      = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_C_SM_EDGE_low       , ANR2_C_SM_EDGE_high, 1);
        ANR2_FLT_C          = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_FLT_C_low           , ANR2_FLT_C_high, 1);
        ANR2_Y_CPX1         = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_Y_CPX1_low          , ANR2_Y_CPX1_high, 1);
        ANR2_Y_CPX2         = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_Y_CPX2_low          , ANR2_Y_CPX2_high, 1);
        ANR2_Y_CPX3         = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_Y_CPX3_low          , ANR2_Y_CPX3_high, 1);
        ANR2_Y_SCALE_CPY0   = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_Y_SCALE_CPY0_low    , ANR2_Y_SCALE_CPY0_high, 1);
        ANR2_Y_SCALE_CPY1   = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_Y_SCALE_CPY1_low    , ANR2_Y_SCALE_CPY1_high, 1);
        ANR2_Y_SCALE_CPY2   = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_Y_SCALE_CPY2_low    , ANR2_Y_SCALE_CPY2_high, 1);
        ANR2_Y_SCALE_CPY3   = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_Y_SCALE_CPY3_low    , ANR2_Y_SCALE_CPY3_high, 1);
        ANR2_Y_SCALE_SP0    = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_Y_SCALE_SP0_low     , ANR2_Y_SCALE_SP0_high, 1);
        ANR2_Y_SCALE_SP1    = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_Y_SCALE_SP1_low     , ANR2_Y_SCALE_SP1_high, 1);
        ANR2_Y_SCALE_SP2    = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_Y_SCALE_SP2_low     , ANR2_Y_SCALE_SP2_high, 1);
        ANR2_Y_SCALE_SP3    = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_Y_SCALE_SP3_low     , ANR2_Y_SCALE_SP3_high, 1);
        ANR2_PTC_VGAIN      = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_PTC_VGAIN_low       , ANR2_PTC_VGAIN_high, 1);
        ANR2_C_GAIN         = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_C_GAIN_low          , ANR2_C_GAIN_high, 1);
        ANR2_PTC_GAIN_TH    = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_PTC_GAIN_TH_low     , ANR2_PTC_GAIN_TH_high, 1);
        ANR2_C_L_DIFF_TH    = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_C_L_DIFF_TH_low     , ANR2_C_L_DIFF_TH_high, 1);
        ANR2_C_MODE         = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_C_MODE_low          , ANR2_C_MODE_high, 1);
        ANR2_PTC1           = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_PTC1_low            , ANR2_PTC1_high, 1);
        ANR2_PTC2           = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_PTC2_low            , ANR2_PTC2_high, 1);
        ANR2_PTC3           = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_PTC3_low            , ANR2_PTC3_high, 1);
        ANR2_PTC4           = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_PTC4_low            , ANR2_PTC4_high, 1);
        ANR2_LM_WT          = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_LM_WT_low           , ANR2_LM_WT_high, 1);
        ANR2_ACT_TH_Y       = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_ACT_TH_Y_low        , ANR2_ACT_TH_Y_high, 1);
        ANR2_ACT_BLD_BASE_Y = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_ACT_BLD_BASE_Y_low  , ANR2_ACT_BLD_BASE_Y_high, 1);
        ANR2_ACT_SLANT_Y    = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_ACT_SLANT_Y_low     , ANR2_ACT_SLANT_Y_high, 1);
        ANR2_ACT_BLD_TH_Y   = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_ACT_BLD_TH_Y_low    , ANR2_ACT_BLD_TH_Y_high, 1);
        ANR2_ACT_TH_C       = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_ACT_TH_C_low        , ANR2_ACT_TH_C_high, 1);
        ANR2_ACT_BLD_BASE_C = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_ACT_BLD_BASE_C_low  , ANR2_ACT_BLD_BASE_C_high, 1);
        ANR2_ACT_SLANT_C    = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_ACT_SLANT_C_low     , ANR2_ACT_SLANT_C_high, 1);
        ANR2_ACT_BLD_TH_C   = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_ACT_BLD_TH_C_low    , ANR2_ACT_BLD_TH_C_high, 1);

        // ANR2_SCALE_MODE
        ANR2_SCALE_MODE     = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_SCALE_MODE_low      , ANR2_SCALE_MODE_high, 1);
        ANR2_K_TH_C         = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_K_TH_C_low          , ANR2_K_TH_C_high, 1);

        // ANR2_LCE_LINK
        ANR2_LCE_LINK       = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_LCE_LINK_low        , ANR2_LCE_LINK_high, 1);
        ANR2_LCE_X1         = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_LCE_X1_low          , ANR2_LCE_X1_high, 1);
        ANR2_LCE_X2         = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_LCE_X2_low          , ANR2_LCE_X2_high, 1);
        ANR2_LCE_X3         = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_LCE_X3_low          , ANR2_LCE_X3_high, 1);
        ANR2_LCE_GAIN0      = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_LCE_GAIN0_low       , ANR2_LCE_GAIN0_high, 1);
        ANR2_LCE_GAIN1      = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_LCE_GAIN1_low       , ANR2_LCE_GAIN1_high, 1);
        ANR2_LCE_GAIN2      = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_LCE_GAIN2_low       , ANR2_LCE_GAIN2_high, 1);
        ANR2_LCE_GAIN3      = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_LCE_GAIN3_low       , ANR2_LCE_GAIN3_high, 1);
        ANR2_LCE_SP0        = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_LCE_SP0_low         , ANR2_LCE_SP0_high, 1);
        ANR2_LCE_SP1        = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_LCE_SP1_low         , ANR2_LCE_SP1_high, 1);
        ANR2_LCE_SP2        = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_LCE_SP2_low         , ANR2_LCE_SP2_high, 1);
        ANR2_LCE_SP3        = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_LCE_SP3_low         , ANR2_LCE_SP3_high, 1);
        ANR2_LCE_C_GAIN     = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_LCE_C_GAIN_low      , ANR2_LCE_C_GAIN_high, 1);

        // ANR2_MEDIAN_EN
        ANR2_COR_TH      = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_COR_TH_low             , ANR2_COR_TH_high, 1);
        ANR2_COR_SL      = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_COR_SL_low             , ANR2_COR_SL_high, 1);
        ANR2_MCD_TH      = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_MCD_TH_low             , ANR2_MCD_TH_high, 1);
        ANR2_MCD_SL      = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_MCD_SL_low             , ANR2_MCD_SL_high, 1);
        ANR2_LCL_TH      = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_LCL_TH_low             , ANR2_LCL_TH_high, 1);
        ANR2_LCL_SL      = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_LCL_SL_low             , ANR2_LCL_SL_high, 1);
        ANR2_LCL_LV      = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_LCL_LV_low             , ANR2_LCL_LV_high, 1);
        ANR2_SCL_TH      = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_SCL_TH_low             , ANR2_SCL_TH_high, 1);
        ANR2_SCL_SL      = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_SCL_SL_low             , ANR2_SCL_SL_high, 1);
        ANR2_SCL_LV      = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_SCL_LV_low             , ANR2_SCL_LV_high, 1);
        ANR2_NCL_TH      = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_NCL_TH_low             , ANR2_NCL_TH_high, 1);
        ANR2_NCL_SL      = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_NCL_SL_low             , ANR2_NCL_SL_high, 1);
        ANR2_NCL_LV      = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_NCL_LV_low             , ANR2_NCL_LV_high, 1);
        ANR2_VAR         = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_VAR_low                , ANR2_VAR_high, 1);
        ANR2_Y0          = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_Y0_low                 , ANR2_Y0_high, 1);
        ANR2_Y1          = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_Y1_low                 , ANR2_Y1_high, 1);
        ANR2_Y2          = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_Y2_low                 , ANR2_Y2_high, 1);
        ANR2_Y3          = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_Y3_low                 , ANR2_Y3_high, 1);
        ANR2_Y4          = InterParam_NR(ISO, ISO_low, ISO_high, ANR2_Y4_low                 , ANR2_Y4_high, 1);
    }

    //    ====================================================================================================================================
    //    Negative Conversion
    //    ====================================================================================================================================
    ANR2_Y_SCALE_SP0 = (ANR2_Y_SCALE_SP0 < 0) ? (ANR2_Y_SCALE_SP0 + 32) : ANR2_Y_SCALE_SP0;
    ANR2_Y_SCALE_SP1 = (ANR2_Y_SCALE_SP1 < 0) ? (ANR2_Y_SCALE_SP1 + 32) : ANR2_Y_SCALE_SP1;
    ANR2_Y_SCALE_SP2 = (ANR2_Y_SCALE_SP2 < 0) ? (ANR2_Y_SCALE_SP2 + 32) : ANR2_Y_SCALE_SP2;
    ANR2_Y_SCALE_SP3 = (ANR2_Y_SCALE_SP3 < 0) ? (ANR2_Y_SCALE_SP3 + 32) : ANR2_Y_SCALE_SP3;
    ANR2_LCE_SP0     = (ANR2_LCE_SP0 < 0) ? (ANR2_LCE_SP0 + 64) : ANR2_LCE_SP0;
    ANR2_LCE_SP1     = (ANR2_LCE_SP1 < 0) ? (ANR2_LCE_SP1 + 64) : ANR2_LCE_SP1;
    ANR2_LCE_SP2     = (ANR2_LCE_SP2 < 0) ? (ANR2_LCE_SP2 + 64) : ANR2_LCE_SP2;
    ANR2_LCE_SP3     = (ANR2_LCE_SP3 < 0) ? (ANR2_LCE_SP3 + 64) : ANR2_LCE_SP3;

    //    ====================================================================================================================================
    //    Set Smooth ISO setting
    //    ====================================================================================================================================
    // CAM_ANR2_CON1 CAM+A20H
    rSmoothANR.con1.bits.ANR2_ENC = ANR2_ENC;
    rSmoothANR.con1.bits.ANR2_ENY = ANR2_ENY;
    rSmoothANR.con1.bits.ANR2_SCALE_MODE = ANR2_SCALE_MODE;
    rSmoothANR.con1.bits.ANR2_MODE = ANR2_MODE;
    rSmoothANR.con1.bits.ANR2_LCE_LINK = ANR2_LCE_LINK;

    // CAM_ANR2_CON2 CAM+A24H
    rSmoothANR.con2.bits.ANR2_C_MED_EN = ANR2_C_MED_EN;
    rSmoothANR.con2.bits.ANR2_C_SM_EDGE = ANR2_C_SM_EDGE;
    rSmoothANR.con2.bits.ANR2_FLT_C = ANR2_FLT_C;

    // CAM_ANR2_YAD1 CAM+A2CH
    rSmoothANR.yad1.bits.ANR2_K_TH_C = ANR2_K_TH_C;

    // CAM_ANR2_Y4LUT1 CAM+A34H
    rSmoothANR.lut1.bits.ANR2_Y_CPX1 = ANR2_Y_CPX1;
    rSmoothANR.lut1.bits.ANR2_Y_CPX2 = ANR2_Y_CPX2;
    rSmoothANR.lut1.bits.ANR2_Y_CPX3 = ANR2_Y_CPX3;

    // CAM_ANR2_Y4LUT2 CAM+A38H
    rSmoothANR.lut2.bits.ANR2_Y_SCALE_CPY0 = ANR2_Y_SCALE_CPY0;
    rSmoothANR.lut2.bits.ANR2_Y_SCALE_CPY1 = ANR2_Y_SCALE_CPY1;
    rSmoothANR.lut2.bits.ANR2_Y_SCALE_CPY2 = ANR2_Y_SCALE_CPY2;
    rSmoothANR.lut2.bits.ANR2_Y_SCALE_CPY3 = ANR2_Y_SCALE_CPY3;

    // CAM_ANR2_Y4LUT3 CAM+A3CH
    rSmoothANR.lut3.bits.ANR2_Y_SCALE_SP0 = ANR2_Y_SCALE_SP0;
    rSmoothANR.lut3.bits.ANR2_Y_SCALE_SP1 = ANR2_Y_SCALE_SP1;
    rSmoothANR.lut3.bits.ANR2_Y_SCALE_SP2 = ANR2_Y_SCALE_SP2;
    rSmoothANR.lut3.bits.ANR2_Y_SCALE_SP3 = ANR2_Y_SCALE_SP3;

    // CAM_ANR2_L4LUT1
    rSmoothANR.llut1.bits.ANR2_LCE_X1 = ANR2_LCE_X1;
    rSmoothANR.llut1.bits.ANR2_LCE_X2 = ANR2_LCE_X2;
    rSmoothANR.llut1.bits.ANR2_LCE_X3 = ANR2_LCE_X3;

    // CAM_ANR2_L4LUT2
    rSmoothANR.llut2.bits.ANR2_LCE_GAIN0 = ANR2_LCE_GAIN0;
    rSmoothANR.llut2.bits.ANR2_LCE_GAIN1 = ANR2_LCE_GAIN1;
    rSmoothANR.llut2.bits.ANR2_LCE_GAIN2 = ANR2_LCE_GAIN2;
    rSmoothANR.llut2.bits.ANR2_LCE_GAIN3 = ANR2_LCE_GAIN3;

    // CAM_ANR2_L4LUT3
    rSmoothANR.llut3.bits.ANR2_LCE_SP0 = ANR2_LCE_SP0;
    rSmoothANR.llut3.bits.ANR2_LCE_SP1 = ANR2_LCE_SP1;
    rSmoothANR.llut3.bits.ANR2_LCE_SP2 = ANR2_LCE_SP2;
    rSmoothANR.llut3.bits.ANR2_LCE_SP3 = ANR2_LCE_SP3;

    // CAM_ANR2_CAD CAM+A44H
    rSmoothANR.cad.bits.ANR2_PTC_VGAIN   = ANR2_PTC_VGAIN;
    rSmoothANR.cad.bits.ANR2_C_GAIN      = ANR2_C_GAIN;
    rSmoothANR.cad.bits.ANR2_PTC_GAIN_TH = ANR2_PTC_GAIN_TH;
    rSmoothANR.cad.bits.ANR2_C_L_DIFF_TH = ANR2_C_L_DIFF_TH;
    rSmoothANR.cad.bits.ANR2_C_MODE      = ANR2_C_MODE;

    // CAM_ANR2_PTC CAM+A48H
    rSmoothANR.ptc.bits.ANR2_PTC1 = ANR2_PTC1;
    rSmoothANR.ptc.bits.ANR2_PTC2 = ANR2_PTC2;
    rSmoothANR.ptc.bits.ANR2_PTC3 = ANR2_PTC3;
    rSmoothANR.ptc.bits.ANR2_PTC4 = ANR2_PTC4;

    // CAM_ANR2_LCE CAM+A4CH
    rSmoothANR.lce.bits.ANR2_LCE_C_GAIN = ANR2_LCE_C_GAIN;
    rSmoothANR.lce.bits.ANR2_LM_WT = ANR2_LM_WT;

    // CAM_ANR2_MED1
    rSmoothANR.med1.bits.ANR2_COR_TH = ANR2_COR_TH;
    rSmoothANR.med1.bits.ANR2_COR_SL = ANR2_COR_SL;
    rSmoothANR.med1.bits.ANR2_MCD_TH = ANR2_MCD_TH;
    rSmoothANR.med1.bits.ANR2_MCD_SL = ANR2_MCD_SL;
    rSmoothANR.med1.bits.ANR2_LCL_TH = ANR2_LCL_TH;

    // CAM_ANR2_MED2
    rSmoothANR.med2.bits.ANR2_LCL_SL = ANR2_LCL_SL;
    rSmoothANR.med2.bits.ANR2_LCL_LV = ANR2_LCL_LV;
    rSmoothANR.med2.bits.ANR2_SCL_TH = ANR2_SCL_TH;
    rSmoothANR.med2.bits.ANR2_SCL_SL = ANR2_SCL_SL;
    rSmoothANR.med2.bits.ANR2_SCL_LV = ANR2_SCL_LV;

    // CAM_ANR2_MED3
    rSmoothANR.med3.bits.ANR2_NCL_TH = ANR2_NCL_TH;
    rSmoothANR.med3.bits.ANR2_NCL_SL = ANR2_NCL_SL;
    rSmoothANR.med3.bits.ANR2_NCL_LV = ANR2_NCL_LV;
    rSmoothANR.med3.bits.ANR2_VAR = ANR2_VAR;
    rSmoothANR.med3.bits.ANR2_Y0 = ANR2_Y0;

    // CAM_ANR2_MED4
    rSmoothANR.med4.bits.ANR2_Y1 = ANR2_Y1;
    rSmoothANR.med4.bits.ANR2_Y2 = ANR2_Y2;
    rSmoothANR.med4.bits.ANR2_Y3 = ANR2_Y3;
    rSmoothANR.med4.bits.ANR2_Y4 = ANR2_Y4;

    // CAM_ANR2_ACTY CAM+A60H
    rSmoothANR.acty.bits.ANR2_ACT_TH_Y = ANR2_ACT_TH_Y;
    rSmoothANR.acty.bits.ANR2_ACT_BLD_BASE_Y = ANR2_ACT_BLD_BASE_Y;
    rSmoothANR.acty.bits.ANR2_ACT_SLANT_Y = ANR2_ACT_SLANT_Y;
    rSmoothANR.acty.bits.ANR2_ACT_BLD_TH_Y = ANR2_ACT_BLD_TH_Y;

    // CAM_ANR2_ACTC CAM+A64H
    rSmoothANR.actc.bits.ANR2_ACT_TH_C = ANR2_ACT_TH_C;
    rSmoothANR.actc.bits.ANR2_ACT_BLD_BASE_C = ANR2_ACT_BLD_BASE_C;
    rSmoothANR.actc.bits.ANR2_ACT_SLANT_C = ANR2_ACT_SLANT_C;
    rSmoothANR.actc.bits.ANR2_ACT_BLD_TH_C = ANR2_ACT_BLD_TH_C;

#define ANR2_DUMP(item1, item2)   \
        do{                      \
            MY_LOG_IF(bSmoothANRDebug, "[L,S,U]%22s = %d, %d, %d", #item2, rLowerANR.item1.bits.item2, rSmoothANR.item1.bits.item2, rUpperANR.item1.bits.item2); \
        }while(0)

    ANR2_DUMP(con1, ANR2_ENC);
    ANR2_DUMP(con1, ANR2_ENY);
    ANR2_DUMP(con1, ANR2_SCALE_MODE);
    ANR2_DUMP(con1, ANR2_MODE);
    ANR2_DUMP(con1, ANR2_LCE_LINK);

    ANR2_DUMP(con2, ANR2_C_MED_EN);
    ANR2_DUMP(con2, ANR2_C_SM_EDGE);
    ANR2_DUMP(con2, ANR2_FLT_C);

    ANR2_DUMP(yad1, ANR2_K_TH_C);

    ANR2_DUMP(lut1, ANR2_Y_CPX1);
    ANR2_DUMP(lut1, ANR2_Y_CPX2);
    ANR2_DUMP(lut1, ANR2_Y_CPX3);

    ANR2_DUMP(lut2, ANR2_Y_SCALE_CPY0);
    ANR2_DUMP(lut2, ANR2_Y_SCALE_CPY1);
    ANR2_DUMP(lut2, ANR2_Y_SCALE_CPY2);
    ANR2_DUMP(lut2, ANR2_Y_SCALE_CPY3);

    ANR2_DUMP(lut3, ANR2_Y_SCALE_SP0);
    ANR2_DUMP(lut3, ANR2_Y_SCALE_SP1);
    ANR2_DUMP(lut3, ANR2_Y_SCALE_SP2);
    ANR2_DUMP(lut3, ANR2_Y_SCALE_SP3);

    ANR2_DUMP(llut1, ANR2_LCE_X1);
    ANR2_DUMP(llut1, ANR2_LCE_X2);
    ANR2_DUMP(llut1, ANR2_LCE_X3);

    ANR2_DUMP(llut2, ANR2_LCE_GAIN0);
    ANR2_DUMP(llut2, ANR2_LCE_GAIN1);
    ANR2_DUMP(llut2, ANR2_LCE_GAIN2);
    ANR2_DUMP(llut2, ANR2_LCE_GAIN3);

    ANR2_DUMP(llut3, ANR2_LCE_SP0);
    ANR2_DUMP(llut3, ANR2_LCE_SP1);
    ANR2_DUMP(llut3, ANR2_LCE_SP2);
    ANR2_DUMP(llut3, ANR2_LCE_SP3);

    ANR2_DUMP(cad, ANR2_PTC_VGAIN);
    ANR2_DUMP(cad, ANR2_C_GAIN);
    ANR2_DUMP(cad, ANR2_PTC_GAIN_TH);
    ANR2_DUMP(cad, ANR2_C_L_DIFF_TH);
    ANR2_DUMP(cad, ANR2_C_MODE);

    ANR2_DUMP(ptc, ANR2_PTC1);
    ANR2_DUMP(ptc, ANR2_PTC2);
    ANR2_DUMP(ptc, ANR2_PTC3);
    ANR2_DUMP(ptc, ANR2_PTC4);

    ANR2_DUMP(lce, ANR2_LCE_C_GAIN);
    ANR2_DUMP(lce, ANR2_LM_WT);

    ANR2_DUMP(med1, ANR2_COR_TH);
    ANR2_DUMP(med1, ANR2_COR_SL);
    ANR2_DUMP(med1, ANR2_MCD_TH);
    ANR2_DUMP(med1, ANR2_MCD_SL);
    ANR2_DUMP(med1, ANR2_LCL_TH);

    ANR2_DUMP(med2, ANR2_LCL_SL);
    ANR2_DUMP(med2, ANR2_LCL_LV);
    ANR2_DUMP(med2, ANR2_SCL_TH);
    ANR2_DUMP(med2, ANR2_SCL_SL);
    ANR2_DUMP(med2, ANR2_SCL_LV);

    ANR2_DUMP(med3, ANR2_NCL_TH);
    ANR2_DUMP(med3, ANR2_NCL_SL);
    ANR2_DUMP(med3, ANR2_NCL_LV);
    ANR2_DUMP(med3, ANR2_VAR);
    ANR2_DUMP(med3, ANR2_Y0);

    ANR2_DUMP(med4, ANR2_Y1);
    ANR2_DUMP(med4, ANR2_Y2);
    ANR2_DUMP(med4, ANR2_Y3);
    ANR2_DUMP(med4, ANR2_Y4);

    ANR2_DUMP(acty, ANR2_ACT_TH_Y);
    ANR2_DUMP(acty, ANR2_ACT_BLD_BASE_Y);
    ANR2_DUMP(acty, ANR2_ACT_SLANT_Y);
    ANR2_DUMP(acty, ANR2_ACT_BLD_TH_Y);

    ANR2_DUMP(actc, ANR2_ACT_TH_C);
    ANR2_DUMP(actc, ANR2_ACT_BLD_BASE_C);
    ANR2_DUMP(actc, ANR2_ACT_SLANT_C);
    ANR2_DUMP(actc, ANR2_ACT_BLD_TH_C);

#undef ANR2_DUMP
}

MVOID SmoothANR_TBL(MUINT32 u4RealISO,  // Real ISO
               MUINT32 u4UpperISO, // Upper ISO
               MUINT32 u4LowerISO, // Lower ISO
               ISP_NVRAM_ANR_LUT_T const& rUpperANR_TBL, // ANR TBL for upper ISO
               ISP_NVRAM_ANR_LUT_T const& rLowerANR_TBL,   // ANR TBL for lower ISO
               ISP_NVRAM_ANR_LUT_T& rSmoothANR_TBL)  // Output
{

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.smooth_anr_tbl.enable", value, "0");
    MBOOL bSmoothANR_TBL_Debug = atoi(value);

    MY_LOG_IF(bSmoothANR_TBL_Debug,"[%s()] u4RealISO: %d, u4UpperISO = %d, u4LowerISO = %d\n", __FUNCTION__, u4RealISO, u4UpperISO, u4LowerISO);

	MINT32 ISO = u4RealISO;
    MINT32 ISO_low = u4LowerISO;
    MINT32 ISO_high = u4UpperISO;

	for(int i = 0; i < ANR_HW_TBL_NUM; ++i){

		//  Get Lower ISO setting
		MINT32 ANR_TBL_Y0_low =  rLowerANR_TBL.lut[i].bits.ANR_TBL_Y0;
		MINT32 ANR_TBL_U0_low =  rLowerANR_TBL.lut[i].bits.ANR_TBL_U0;
		MINT32 ANR_TBL_V0_low =  rLowerANR_TBL.lut[i].bits.ANR_TBL_V0;
		MINT32 ANR_TBL_Y1_low =  rLowerANR_TBL.lut[i].bits.ANR_TBL_Y1;
		MINT32 ANR_TBL_U1_low =  rLowerANR_TBL.lut[i].bits.ANR_TBL_U1;
		MINT32 ANR_TBL_V1_low =  rLowerANR_TBL.lut[i].bits.ANR_TBL_V1;

		//  Get Higher ISO setting
		MINT32 ANR_TBL_Y0_high = rUpperANR_TBL.lut[i].bits.ANR_TBL_Y0;
		MINT32 ANR_TBL_U0_high = rUpperANR_TBL.lut[i].bits.ANR_TBL_U0;
		MINT32 ANR_TBL_V0_high = rUpperANR_TBL.lut[i].bits.ANR_TBL_V0;
		MINT32 ANR_TBL_Y1_high = rUpperANR_TBL.lut[i].bits.ANR_TBL_Y1;
		MINT32 ANR_TBL_U1_high = rUpperANR_TBL.lut[i].bits.ANR_TBL_U1;
		MINT32 ANR_TBL_V1_high = rUpperANR_TBL.lut[i].bits.ANR_TBL_V1;

		//  Start Parameter Interpolation
		MINT32 ANR_TBL_Y0 ;
		MINT32 ANR_TBL_U0 ;
		MINT32 ANR_TBL_V0 ;
		MINT32 ANR_TBL_Y1 ;
		MINT32 ANR_TBL_U1 ;
		MINT32 ANR_TBL_V1 ;

		ANR_TBL_Y0          = InterParam_NR(ISO, ISO_low, ISO_high, ANR_TBL_Y0_low, ANR_TBL_Y0_high, 0);
		ANR_TBL_U0          = InterParam_NR(ISO, ISO_low, ISO_high, ANR_TBL_U0_low, ANR_TBL_U0_high, 0);
		ANR_TBL_V0          = InterParam_NR(ISO, ISO_low, ISO_high, ANR_TBL_V0_low, ANR_TBL_V0_high, 0);
		ANR_TBL_Y1          = InterParam_NR(ISO, ISO_low, ISO_high, ANR_TBL_Y1_low, ANR_TBL_Y1_high, 0);
		ANR_TBL_U1          = InterParam_NR(ISO, ISO_low, ISO_high, ANR_TBL_U1_low, ANR_TBL_U1_high, 0);
		ANR_TBL_V1          = InterParam_NR(ISO, ISO_low, ISO_high, ANR_TBL_V1_low, ANR_TBL_V1_high, 0);

		rSmoothANR_TBL.lut[i].bits.ANR_TBL_Y0 =  ANR_TBL_Y0;
		rSmoothANR_TBL.lut[i].bits.ANR_TBL_U0 =  ANR_TBL_U0;
		rSmoothANR_TBL.lut[i].bits.ANR_TBL_V0 =  ANR_TBL_V0;
		rSmoothANR_TBL.lut[i].bits.ANR_TBL_Y1 =  ANR_TBL_Y1;
		rSmoothANR_TBL.lut[i].bits.ANR_TBL_U1 =  ANR_TBL_U1;
		rSmoothANR_TBL.lut[i].bits.ANR_TBL_V1 =  ANR_TBL_V1;

	}

}

//  "return value": Interpolated register value
//  "ISO":          ISO value of current image
//  "ISOpre":       A pre-defined ISO (100, 200, 400, ...) which is the closest but smaller than or equal to "ISO"
//  "ISOpos":       A pre-defined ISO (100, 200, 400, ...) which is the closest but larger than or equal to "ISO"
//  "PARAMpre":     Corresponding register value from "ISOpre"
//  "PARAMpos":     Corresponding register value from "ISOpos"
//  "Method":       0 for linear interpolation, 1 for closest one
MINT32 InterParam_HFG(MINT32 ISO, MINT32 ISOpre, MINT32 ISOpos, MINT32 PARAMpre, MINT32 PARAMpos, MINT32 Method){
	MINT32 InterPARAM = 0;

	if(ISOpre == ISOpos){
		//	To prevent someone sets different PARAMpre and PARAMpos
		return (MINT32)((PARAMpre + PARAMpos + 1) / 2);
	}

	switch(Method){
	case 1:
		InterPARAM = (abs((double)(ISO - ISOpre)) > abs((double)(ISOpos - ISO))) ? PARAMpos : PARAMpre;
		break;
	case 0:
	default:
		double RATIOpre = (double)(ISOpos - ISO) / (double)(ISOpos - ISOpre);
		double RATIOpos = 1.0 - (double)(RATIOpre);
		double temp =  RATIOpre * PARAMpre + RATIOpos * PARAMpos;
		if(temp>0)
			InterPARAM = (MINT32)(RATIOpre * PARAMpre + RATIOpos * PARAMpos + 0.5);
		else
			InterPARAM = (MINT32)(RATIOpre * PARAMpre + RATIOpos * PARAMpos - 0.5);
		break;
	}

	return InterPARAM;
}

MVOID SmoothHFG(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
               ISP_NVRAM_HFG_T const& rUpperHFG, // EE settings for upper ISO
               ISP_NVRAM_HFG_T const& rLowerHFG,   // EE settings for lower ISO
               ISP_NVRAM_HFG_T& rSmoothHFG)   // Output
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.smooth_hfg.enable", value, "0");
    MBOOL bSmoothHFGDebug = atoi(value);

    MY_LOG_IF(bSmoothHFGDebug,"[%s()] u4RealISO: %d, u4UpperISO = %d, u4LowerISO = %d\n", __FUNCTION__, u4RealISO, u4UpperISO, u4LowerISO);

    // Get Lower ISO setting
    MINT32 DIP_HFC_STD_lower   = rLowerHFG.con_0.bits.DIP_HFC_STD;
    MINT32 HFC_LUMA_CPX1_lower = rLowerHFG.luma_0.bits.HFC_LUMA_CPX1;
    MINT32 HFC_LUMA_CPX2_lower = rLowerHFG.luma_0.bits.HFC_LUMA_CPX2;
    MINT32 HFC_LUMA_CPX3_lower = rLowerHFG.luma_0.bits.HFC_LUMA_CPX3;
    MINT32 HFC_LUMA_CPY0_lower = rLowerHFG.luma_1.bits.HFC_LUMA_CPY0;
    MINT32 HFC_LUMA_CPY1_lower = rLowerHFG.luma_1.bits.HFC_LUMA_CPY1;
    MINT32 HFC_LUMA_CPY2_lower = rLowerHFG.luma_1.bits.HFC_LUMA_CPY2;
    MINT32 HFC_LUMA_CPY3_lower = rLowerHFG.luma_1.bits.HFC_LUMA_CPY3;
    MINT32 HFC_LUMA_SP0_lower  = rLowerHFG.luma_2.bits.HFC_LUMA_SP0;
    MINT32 HFC_LUMA_SP1_lower  = rLowerHFG.luma_2.bits.HFC_LUMA_SP1;
    MINT32 HFC_LUMA_SP2_lower  = rLowerHFG.luma_2.bits.HFC_LUMA_SP2;
    MINT32 HFC_LUMA_SP3_lower  = rLowerHFG.luma_2.bits.HFC_LUMA_SP3;
    MINT32 HFC_LCE_CPX1_lower  = rLowerHFG.lce_0.bits.HFC_LCE_CPX1;
    MINT32 HFC_LCE_CPX2_lower  = rLowerHFG.lce_0.bits.HFC_LCE_CPX2;
    MINT32 HFC_LCE_CPX3_lower  = rLowerHFG.lce_0.bits.HFC_LCE_CPX3;
    MINT32 HFC_LCE_CPY0_lower  = rLowerHFG.lce_1.bits.HFC_LCE_CPY0;
    MINT32 HFC_LCE_CPY1_lower  = rLowerHFG.lce_1.bits.HFC_LCE_CPY1;
    MINT32 HFC_LCE_CPY2_lower  = rLowerHFG.lce_1.bits.HFC_LCE_CPY2;
    MINT32 HFC_LCE_CPY3_lower  = rLowerHFG.lce_1.bits.HFC_LCE_CPY3;
    MINT32 HFC_LCE_SP0_lower   = rLowerHFG.lce_2.bits.HFC_LCE_SP0;
    MINT32 HFC_LCE_SP1_lower   = rLowerHFG.lce_2.bits.HFC_LCE_SP1;
    MINT32 HFC_LCE_SP2_lower   = rLowerHFG.lce_2.bits.HFC_LCE_SP2;
    MINT32 HFC_LCE_SP3_lower   = rLowerHFG.lce_2.bits.HFC_LCE_SP3;

    // Get Upper ISO setting
    MINT32 DIP_HFC_STD_upper   = rUpperHFG.con_0.bits.DIP_HFC_STD;
    MINT32 HFC_LUMA_CPX1_upper = rUpperHFG.luma_0.bits.HFC_LUMA_CPX1;
    MINT32 HFC_LUMA_CPX2_upper = rUpperHFG.luma_0.bits.HFC_LUMA_CPX2;
    MINT32 HFC_LUMA_CPX3_upper = rUpperHFG.luma_0.bits.HFC_LUMA_CPX3;
    MINT32 HFC_LUMA_CPY0_upper = rUpperHFG.luma_1.bits.HFC_LUMA_CPY0;
    MINT32 HFC_LUMA_CPY1_upper = rUpperHFG.luma_1.bits.HFC_LUMA_CPY1;
    MINT32 HFC_LUMA_CPY2_upper = rUpperHFG.luma_1.bits.HFC_LUMA_CPY2;
    MINT32 HFC_LUMA_CPY3_upper = rUpperHFG.luma_1.bits.HFC_LUMA_CPY3;
    MINT32 HFC_LUMA_SP0_upper  = rUpperHFG.luma_2.bits.HFC_LUMA_SP0;
    MINT32 HFC_LUMA_SP1_upper  = rUpperHFG.luma_2.bits.HFC_LUMA_SP1;
    MINT32 HFC_LUMA_SP2_upper  = rUpperHFG.luma_2.bits.HFC_LUMA_SP2;
    MINT32 HFC_LUMA_SP3_upper  = rUpperHFG.luma_2.bits.HFC_LUMA_SP3;
    MINT32 HFC_LCE_CPX1_upper  = rUpperHFG.lce_0.bits.HFC_LCE_CPX1;
    MINT32 HFC_LCE_CPX2_upper  = rUpperHFG.lce_0.bits.HFC_LCE_CPX2;
    MINT32 HFC_LCE_CPX3_upper  = rUpperHFG.lce_0.bits.HFC_LCE_CPX3;
    MINT32 HFC_LCE_CPY0_upper  = rUpperHFG.lce_1.bits.HFC_LCE_CPY0;
    MINT32 HFC_LCE_CPY1_upper  = rUpperHFG.lce_1.bits.HFC_LCE_CPY1;
    MINT32 HFC_LCE_CPY2_upper  = rUpperHFG.lce_1.bits.HFC_LCE_CPY2;
    MINT32 HFC_LCE_CPY3_upper  = rUpperHFG.lce_1.bits.HFC_LCE_CPY3;
    MINT32 HFC_LCE_SP0_upper   = rUpperHFG.lce_2.bits.HFC_LCE_SP0;
    MINT32 HFC_LCE_SP1_upper   = rUpperHFG.lce_2.bits.HFC_LCE_SP1;
    MINT32 HFC_LCE_SP2_upper   = rUpperHFG.lce_2.bits.HFC_LCE_SP2;
    MINT32 HFC_LCE_SP3_upper   = rUpperHFG.lce_2.bits.HFC_LCE_SP3;

    // Start Parameter Interpolation
    MINT32 ISO = u4RealISO;
    MINT32 ISOpre = u4LowerISO;
    MINT32 ISOpos = u4UpperISO;

    MINT32 DIP_HFC_STD   = InterParam_HFG(ISO, ISOpre, ISOpos, DIP_HFC_STD_lower, DIP_HFC_STD_upper, 0);
    MINT32 HFC_LUMA_CPX1 = InterParam_HFG(ISO, ISOpre, ISOpos, HFC_LUMA_CPX1_lower, HFC_LUMA_CPX1_upper, 0);
    MINT32 HFC_LUMA_CPX2 = InterParam_HFG(ISO, ISOpre, ISOpos, HFC_LUMA_CPX2_lower, HFC_LUMA_CPX2_upper, 0);
    MINT32 HFC_LUMA_CPX3 = InterParam_HFG(ISO, ISOpre, ISOpos, HFC_LUMA_CPX3_lower, HFC_LUMA_CPX3_upper, 0);
    MINT32 HFC_LUMA_CPY0 = InterParam_HFG(ISO, ISOpre, ISOpos, HFC_LUMA_CPY0_lower, HFC_LUMA_CPY0_upper, 0);
    MINT32 HFC_LUMA_CPY1 = InterParam_HFG(ISO, ISOpre, ISOpos, HFC_LUMA_CPY1_lower, HFC_LUMA_CPY1_upper, 0);
    MINT32 HFC_LUMA_CPY2 = InterParam_HFG(ISO, ISOpre, ISOpos, HFC_LUMA_CPY2_lower, HFC_LUMA_CPY2_upper, 0);
    MINT32 HFC_LUMA_CPY3 = InterParam_HFG(ISO, ISOpre, ISOpos, HFC_LUMA_CPY3_lower, HFC_LUMA_CPY3_upper, 0);
    MINT32 HFC_LUMA_SP0  = InterParam_HFG(ISO, ISOpre, ISOpos, HFC_LUMA_SP0_lower, HFC_LUMA_SP0_upper, 0);
    MINT32 HFC_LUMA_SP1  = InterParam_HFG(ISO, ISOpre, ISOpos, HFC_LUMA_SP1_lower, HFC_LUMA_SP1_upper, 0);
    MINT32 HFC_LUMA_SP2  = InterParam_HFG(ISO, ISOpre, ISOpos, HFC_LUMA_SP2_lower, HFC_LUMA_SP2_upper, 0);
    MINT32 HFC_LUMA_SP3  = InterParam_HFG(ISO, ISOpre, ISOpos, HFC_LUMA_SP3_lower, HFC_LUMA_SP3_upper, 0);
    MINT32 HFC_LCE_CPX1  = InterParam_HFG(ISO, ISOpre, ISOpos, HFC_LCE_CPX1_lower, HFC_LCE_CPX1_upper, 0);
    MINT32 HFC_LCE_CPX2  = InterParam_HFG(ISO, ISOpre, ISOpos, HFC_LCE_CPX2_lower, HFC_LCE_CPX2_upper, 0);
    MINT32 HFC_LCE_CPX3  = InterParam_HFG(ISO, ISOpre, ISOpos, HFC_LCE_CPX3_lower, HFC_LCE_CPX3_upper, 0);
    MINT32 HFC_LCE_CPY0  = InterParam_HFG(ISO, ISOpre, ISOpos, HFC_LCE_CPY0_lower, HFC_LCE_CPY0_upper, 0);
    MINT32 HFC_LCE_CPY1  = InterParam_HFG(ISO, ISOpre, ISOpos, HFC_LCE_CPY1_lower, HFC_LCE_CPY1_upper, 0);
    MINT32 HFC_LCE_CPY2  = InterParam_HFG(ISO, ISOpre, ISOpos, HFC_LCE_CPY2_lower, HFC_LCE_CPY2_upper, 0);
    MINT32 HFC_LCE_CPY3  = InterParam_HFG(ISO, ISOpre, ISOpos, HFC_LCE_CPY3_lower, HFC_LCE_CPY3_upper, 0);
    MINT32 HFC_LCE_SP0   = InterParam_HFG(ISO, ISOpre, ISOpos, HFC_LCE_SP0_lower, HFC_LCE_SP0_upper, 0);
    MINT32 HFC_LCE_SP1   = InterParam_HFG(ISO, ISOpre, ISOpos, HFC_LCE_SP1_lower, HFC_LCE_SP1_upper, 0);
    MINT32 HFC_LCE_SP2   = InterParam_HFG(ISO, ISOpre, ISOpos, HFC_LCE_SP2_lower, HFC_LCE_SP2_upper, 0);
    MINT32 HFC_LCE_SP3   = InterParam_HFG(ISO, ISOpre, ISOpos, HFC_LCE_SP3_lower, HFC_LCE_SP3_upper, 0);

    //  Set Smooth ISO setting
    rSmoothHFG.con_0.bits.DIP_HFC_STD    = DIP_HFC_STD;
    rSmoothHFG.luma_0.bits.HFC_LUMA_CPX1 = HFC_LUMA_CPX1;
    rSmoothHFG.luma_0.bits.HFC_LUMA_CPX2 = HFC_LUMA_CPX2;
    rSmoothHFG.luma_0.bits.HFC_LUMA_CPX3 = HFC_LUMA_CPX3;
    rSmoothHFG.luma_1.bits.HFC_LUMA_CPY0 = HFC_LUMA_CPY0;
    rSmoothHFG.luma_1.bits.HFC_LUMA_CPY1 = HFC_LUMA_CPY1;
    rSmoothHFG.luma_1.bits.HFC_LUMA_CPY2 = HFC_LUMA_CPY2;
    rSmoothHFG.luma_1.bits.HFC_LUMA_CPY3 = HFC_LUMA_CPY3;
    rSmoothHFG.luma_2.bits.HFC_LUMA_SP0  = HFC_LUMA_SP0;
    rSmoothHFG.luma_2.bits.HFC_LUMA_SP1  = HFC_LUMA_SP1;
    rSmoothHFG.luma_2.bits.HFC_LUMA_SP2  = HFC_LUMA_SP2;
    rSmoothHFG.luma_2.bits.HFC_LUMA_SP3  = HFC_LUMA_SP3;
    rSmoothHFG.lce_0.bits.HFC_LCE_CPX1   = HFC_LCE_CPX1;
    rSmoothHFG.lce_0.bits.HFC_LCE_CPX2   = HFC_LCE_CPX2;
    rSmoothHFG.lce_0.bits.HFC_LCE_CPX3   = HFC_LCE_CPX3;
    rSmoothHFG.lce_1.bits.HFC_LCE_CPY0   = HFC_LCE_CPY0;
    rSmoothHFG.lce_1.bits.HFC_LCE_CPY1   = HFC_LCE_CPY1;
    rSmoothHFG.lce_1.bits.HFC_LCE_CPY2   = HFC_LCE_CPY2;
    rSmoothHFG.lce_1.bits.HFC_LCE_CPY3   = HFC_LCE_CPY3;
    rSmoothHFG.lce_2.bits.HFC_LCE_SP0    = HFC_LCE_SP0;
    rSmoothHFG.lce_2.bits.HFC_LCE_SP1    = HFC_LCE_SP1;
    rSmoothHFG.lce_2.bits.HFC_LCE_SP2    = HFC_LCE_SP2;
    rSmoothHFG.lce_2.bits.HFC_LCE_SP3    = HFC_LCE_SP3;

#define HFG_DUMP(item1, item2)   \
        do{                      \
            MY_LOG_IF(bSmoothHFGDebug, "[L,S,U]%22s = %d, %d, %d", #item2, rLowerHFG.item1.bits.item2, rSmoothHFG.item1.bits.item2, rUpperHFG.item1.bits.item2); \
        }while(0)

    HFG_DUMP(con_0, DIP_HFC_STD);
    HFG_DUMP(luma_0, HFC_LUMA_CPX1);
    HFG_DUMP(luma_0, HFC_LUMA_CPX2);
    HFG_DUMP(luma_0, HFC_LUMA_CPX3);
    HFG_DUMP(luma_1, HFC_LUMA_CPY0);
    HFG_DUMP(luma_1, HFC_LUMA_CPY1);
    HFG_DUMP(luma_1, HFC_LUMA_CPY2);
    HFG_DUMP(luma_1, HFC_LUMA_CPY3);
    HFG_DUMP(luma_2, HFC_LUMA_SP0);
    HFG_DUMP(luma_2, HFC_LUMA_SP1);
    HFG_DUMP(luma_2, HFC_LUMA_SP2);
    HFG_DUMP(luma_2, HFC_LUMA_SP3);
    HFG_DUMP(lce_0, HFC_LCE_CPX1);
    HFG_DUMP(lce_0, HFC_LCE_CPX2);
    HFG_DUMP(lce_0, HFC_LCE_CPX3);
    HFG_DUMP(lce_1, HFC_LCE_CPY0);
    HFG_DUMP(lce_1, HFC_LCE_CPY1);
    HFG_DUMP(lce_1, HFC_LCE_CPY2);
    HFG_DUMP(lce_1, HFC_LCE_CPY3);
    HFG_DUMP(lce_2, HFC_LCE_SP0);
    HFG_DUMP(lce_2, HFC_LCE_SP1);
    HFG_DUMP(lce_2, HFC_LCE_SP2);
    HFG_DUMP(lce_2, HFC_LCE_SP3);
}


//  "return value": Interpolated register value
//  "ISO":          ISO value of current image
//  "ISOpre":       A pre-defined ISO (100, 200, 400, ...) which is the closest but smaller than or equal to "ISO"
//  "ISOpos":       A pre-defined ISO (100, 200, 400, ...) which is the closest but larger than or equal to "ISO"
//  "PARAMpre":     Corresponding register value from "ISOpre"
//  "PARAMpos":     Corresponding register value from "ISOpos"
//  "Method":       0 for linear interpolation, 1 for closest one
MINT32 InterParam_CCR(MINT32 ISO, MINT32 ISOpre, MINT32 ISOpos, MINT32 PARAMpre, MINT32 PARAMpos, MINT32 Method){
    MINT32 InterPARAM = 0;

    if(ISOpre == ISOpos){
        //  To prevent someone sets different PARAMpre and PARAMpos
        return (MINT32)((PARAMpre + PARAMpos + 1) / 2);
    }

    switch(Method){
    case 1:
        InterPARAM = (abs((double)(ISO - ISOpre)) > abs((double)(ISOpos - ISO))) ? PARAMpos : PARAMpre;
        break;
    case 0:
    default:
        double RATIOpre = (double)(ISOpos - ISO) / (double)(ISOpos - ISOpre);
        double RATIOpos = 1.0 - (double)(RATIOpre);
        double temp =  RATIOpre * PARAMpre + RATIOpos * PARAMpos;
        if(temp>0)
            InterPARAM = (MINT32)(RATIOpre * PARAMpre + RATIOpos * PARAMpos + 0.5);
        else
            InterPARAM = (MINT32)(RATIOpre * PARAMpre + RATIOpos * PARAMpos - 0.5);
        break;
    }

    return InterPARAM;
}


MVOID SmoothCCR(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
               ISP_NVRAM_CCR_T const& rUpperCCR, // EE settings for upper ISO
               ISP_NVRAM_CCR_T const& rLowerCCR,   // EE settings for lower ISO
               ISP_NVRAM_CCR_T& rSmoothCCR)   // Output
{
      char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.smooth_ccr.enable", value, "0");
    MBOOL bSmoothCCRDebug = atoi(value);

    MY_LOG_IF(bSmoothCCRDebug,"[%s()] u4RealISO: %d, u4UpperISO = %d, u4LowerISO = %d\n", __FUNCTION__, u4RealISO, u4UpperISO, u4LowerISO);

    //  ====================================================================================================================================
    //  Get Lower ISO setting
    //  ====================================================================================================================================
  MINT32 CCR_EN_lower            = rLowerCCR.con.bits.CCR_EN;
  MINT32 CCR_OR_MODE_lower       = rLowerCCR.con.bits.CCR_OR_MODE;
  MINT32 CCR_UV_GAIN_MODE_lower  = rLowerCCR.con.bits.CCR_UV_GAIN_MODE;
  MINT32 CCR_UV_GAIN2_lower      = rLowerCCR.con.bits.CCR_UV_GAIN2;
  MINT32 CCR_Y_CPX3_lower        = rLowerCCR.con.bits.CCR_Y_CPX3;
  MINT32 CCR_Y_CPX1_lower        = rLowerCCR.ylut.bits.CCR_Y_CPX1;
  MINT32 CCR_Y_CPX2_lower        = rLowerCCR.ylut.bits.CCR_Y_CPX2;
  MINT32 CCR_Y_SP1_lower         = rLowerCCR.ylut.bits.CCR_Y_SP1;
  MINT32 CCR_Y_CPY1_lower        = rLowerCCR.ylut.bits.CCR_Y_CPY1;
  MINT32 CCR_UV_X1_lower         = rLowerCCR.uvlut.bits.CCR_UV_X1;
  MINT32 CCR_UV_X2_lower         = rLowerCCR.uvlut.bits.CCR_UV_X2;
  MINT32 CCR_UV_X3_lower         = rLowerCCR.uvlut.bits.CCR_UV_X3;
  MINT32 CCR_UV_GAIN1_lower      = rLowerCCR.uvlut.bits.CCR_UV_GAIN1;
  MINT32 CCR_Y_SP0_lower         = rLowerCCR.ylut2.bits.CCR_Y_SP0;
  MINT32 CCR_Y_SP2_lower         = rLowerCCR.ylut2.bits.CCR_Y_SP2;
  MINT32 CCR_Y_CPY0_lower        = rLowerCCR.ylut2.bits.CCR_Y_CPY0;
  MINT32 CCR_Y_CPY2_lower        = rLowerCCR.ylut2.bits.CCR_Y_CPY2;
  MINT32 CCR_MODE_lower          = rLowerCCR.sat_ctrl.bits.CCR_MODE;
  MINT32 CCR_CEN_U_lower         = rLowerCCR.sat_ctrl.bits.CCR_CEN_U;
  MINT32 CCR_CEN_V_lower         = rLowerCCR.sat_ctrl.bits.CCR_CEN_V;
  MINT32 CCR_UV_GAIN_SP1_lower   = rLowerCCR.uvlut_sp.bits.CCR_UV_GAIN_SP1;
  MINT32 CCR_UV_GAIN_SP2_lower   = rLowerCCR.uvlut_sp.bits.CCR_UV_GAIN_SP2;
  MINT32 CCR_HUE_X1_lower        = rLowerCCR.hue1.bits.CCR_HUE_X1;
  MINT32 CCR_HUE_X2_lower        = rLowerCCR.hue1.bits.CCR_HUE_X2;
  MINT32 CCR_HUE_X3_lower        = rLowerCCR.hue2.bits.CCR_HUE_X3;
  MINT32 CCR_HUE_X4_lower        = rLowerCCR.hue2.bits.CCR_HUE_X4;
  MINT32 CCR_HUE_SP1_lower       = rLowerCCR.hue3.bits.CCR_HUE_SP1;
  MINT32 CCR_HUE_SP2_lower       = rLowerCCR.hue3.bits.CCR_HUE_SP2;
  MINT32 CCR_HUE_GAIN1_lower     = rLowerCCR.hue3.bits.CCR_HUE_GAIN1;
  MINT32 CCR_HUE_GAIN2_lower     = rLowerCCR.hue3.bits.CCR_HUE_GAIN2;

    //  ====================================================================================================================================
    //  Get Upper ISO setting
    //  ====================================================================================================================================
  MINT32 CCR_EN_upper            = rUpperCCR.con.bits.CCR_EN;
  MINT32 CCR_OR_MODE_upper       = rUpperCCR.con.bits.CCR_OR_MODE;
  MINT32 CCR_UV_GAIN_MODE_upper  = rUpperCCR.con.bits.CCR_UV_GAIN_MODE;
  MINT32 CCR_UV_GAIN2_upper      = rUpperCCR.con.bits.CCR_UV_GAIN2;
  MINT32 CCR_Y_CPX3_upper        = rUpperCCR.con.bits.CCR_Y_CPX3;
  MINT32 CCR_Y_CPX1_upper        = rUpperCCR.ylut.bits.CCR_Y_CPX1;
  MINT32 CCR_Y_CPX2_upper        = rUpperCCR.ylut.bits.CCR_Y_CPX2;
  MINT32 CCR_Y_SP1_upper         = rUpperCCR.ylut.bits.CCR_Y_SP1;
  MINT32 CCR_Y_CPY1_upper        = rUpperCCR.ylut.bits.CCR_Y_CPY1;
  MINT32 CCR_UV_X1_upper         = rUpperCCR.uvlut.bits.CCR_UV_X1;
  MINT32 CCR_UV_X2_upper         = rUpperCCR.uvlut.bits.CCR_UV_X2;
  MINT32 CCR_UV_X3_upper         = rUpperCCR.uvlut.bits.CCR_UV_X3;
  MINT32 CCR_UV_GAIN1_upper      = rUpperCCR.uvlut.bits.CCR_UV_GAIN1;
  MINT32 CCR_Y_SP0_upper         = rUpperCCR.ylut2.bits.CCR_Y_SP0;
  MINT32 CCR_Y_SP2_upper         = rUpperCCR.ylut2.bits.CCR_Y_SP2;
  MINT32 CCR_Y_CPY0_upper        = rUpperCCR.ylut2.bits.CCR_Y_CPY0;
  MINT32 CCR_Y_CPY2_upper        = rUpperCCR.ylut2.bits.CCR_Y_CPY2;
  MINT32 CCR_MODE_upper          = rUpperCCR.sat_ctrl.bits.CCR_MODE;
  MINT32 CCR_CEN_U_upper         = rUpperCCR.sat_ctrl.bits.CCR_CEN_U;
  MINT32 CCR_CEN_V_upper         = rUpperCCR.sat_ctrl.bits.CCR_CEN_V;
  MINT32 CCR_UV_GAIN_SP1_upper   = rUpperCCR.uvlut_sp.bits.CCR_UV_GAIN_SP1;
  MINT32 CCR_UV_GAIN_SP2_upper   = rUpperCCR.uvlut_sp.bits.CCR_UV_GAIN_SP2;
  MINT32 CCR_HUE_X1_upper        = rUpperCCR.hue1.bits.CCR_HUE_X1;
  MINT32 CCR_HUE_X2_upper        = rUpperCCR.hue1.bits.CCR_HUE_X2;
  MINT32 CCR_HUE_X3_upper        = rUpperCCR.hue2.bits.CCR_HUE_X3;
  MINT32 CCR_HUE_X4_upper        = rUpperCCR.hue2.bits.CCR_HUE_X4;
  MINT32 CCR_HUE_SP1_upper       = rUpperCCR.hue3.bits.CCR_HUE_SP1;
  MINT32 CCR_HUE_SP2_upper       = rUpperCCR.hue3.bits.CCR_HUE_SP2;
  MINT32 CCR_HUE_GAIN1_upper     = rUpperCCR.hue3.bits.CCR_HUE_GAIN1;
  MINT32 CCR_HUE_GAIN2_upper     = rUpperCCR.hue3.bits.CCR_HUE_GAIN2;

  //    Negative value for slope
  CCR_CEN_U_lower = (CCR_CEN_U_lower > 31) ? (CCR_CEN_U_lower - 64) : CCR_CEN_U_lower;
  CCR_CEN_V_lower = (CCR_CEN_V_lower > 31) ? (CCR_CEN_V_lower - 64) : CCR_CEN_V_lower;
  CCR_HUE_SP1_lower = (CCR_HUE_SP1_lower > 127) ? (CCR_HUE_SP1_lower - 256) : CCR_HUE_SP1_lower;
  CCR_HUE_SP2_lower = (CCR_HUE_SP2_lower > 127) ? (CCR_HUE_SP2_lower - 256) : CCR_HUE_SP2_lower;

  CCR_CEN_U_upper = (CCR_CEN_U_upper > 31) ? (CCR_CEN_U_upper - 64) : CCR_CEN_U_upper;
  CCR_CEN_V_upper = (CCR_CEN_V_upper > 31) ? (CCR_CEN_V_upper - 64) : CCR_CEN_V_upper;
  CCR_HUE_SP1_upper = (CCR_HUE_SP1_upper > 127) ? (CCR_HUE_SP1_upper - 256) : CCR_HUE_SP1_upper;
  CCR_HUE_SP2_upper = (CCR_HUE_SP2_upper > 127) ? (CCR_HUE_SP2_upper - 256) : CCR_HUE_SP2_upper;

  //    Start Parameter Interpolation
    //  ====================================================================================================================================
  MINT32 ISO = u4RealISO;
  MINT32 ISOpre = u4LowerISO;
  MINT32 ISOpos = u4UpperISO;

  //    Registers that can not be interpolated
    MINT32 CCR_EN = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_EN_lower, CCR_EN_upper, 1);
    MINT32 CCR_OR_MODE = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_OR_MODE_lower, CCR_OR_MODE_upper, 1);
    MINT32 CCR_UV_GAIN_MODE = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_UV_GAIN_MODE_lower, CCR_UV_GAIN_MODE_upper, 1);
    MINT32 CCR_MODE = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_MODE_lower, CCR_MODE_upper, 1);

    //  Registers that can be interpolated
    MINT32 CCR_UV_GAIN2    = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_UV_GAIN2_lower, CCR_UV_GAIN2_upper, 0);
    MINT32 CCR_Y_CPX3      = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_Y_CPX3_lower, CCR_Y_CPX3_upper, 0);
    MINT32 CCR_Y_CPX1      = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_Y_CPX1_lower, CCR_Y_CPX1_upper, 0);
    MINT32 CCR_Y_CPX2      = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_Y_CPX2_lower, CCR_Y_CPX2_upper, 0);
    MINT32 CCR_Y_CPY1      = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_Y_CPY1_lower, CCR_Y_CPY1_upper, 0);
    MINT32 CCR_UV_X1       = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_UV_X1_lower, CCR_UV_X1_upper, 0);
    MINT32 CCR_UV_X2       = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_UV_X2_lower, CCR_UV_X2_upper, 0);
    MINT32 CCR_UV_X3       = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_UV_X3_lower, CCR_UV_X3_upper, 0);
    MINT32 CCR_UV_GAIN1    = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_UV_GAIN1_lower, CCR_UV_GAIN1_upper, 0);
    MINT32 CCR_Y_CPY0      = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_Y_CPY0_lower, CCR_Y_CPY0_upper, 0);
    MINT32 CCR_Y_CPY2      = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_Y_CPY2_lower, CCR_Y_CPY2_upper, 0);
    MINT32 CCR_CEN_U       = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_CEN_U_lower, CCR_CEN_U_upper, 0);
    MINT32 CCR_CEN_V       = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_CEN_V_lower, CCR_CEN_V_upper, 0);
    MINT32 CCR_HUE_X1      = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_HUE_X1_lower, CCR_HUE_X1_upper, 0);
    MINT32 CCR_HUE_X2      = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_HUE_X2_lower, CCR_HUE_X2_upper, 0);
    MINT32 CCR_HUE_X3      = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_HUE_X3_lower, CCR_HUE_X3_upper, 0);
    MINT32 CCR_HUE_X4      = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_HUE_X4_lower, CCR_HUE_X4_upper, 0);
    MINT32 CCR_HUE_GAIN1   = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_HUE_GAIN1_lower, CCR_HUE_GAIN1_upper, 0);
    MINT32 CCR_HUE_GAIN2   = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_HUE_GAIN2_lower, CCR_HUE_GAIN2_upper, 0);
    MINT32 CCR_Y_SP2       = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_Y_SP2_lower, CCR_Y_SP2_upper, 0);
    MINT32 CCR_UV_GAIN_SP2 = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_UV_GAIN_SP2_lower, CCR_UV_GAIN_SP2_upper, 0);

    //  Slope parameters update
    MINT32 CCR_Y_SP0;
    MINT32 CCR_Y_SP1;
    MINT32 CCR_UV_GAIN_SP1;
    MINT32 CCR_HUE_SP1;
    MINT32 CCR_HUE_SP2;

    //CCR_Y_SP0
    if(CCR_Y_CPX1 == 0)
        CCR_Y_SP0 = 0;
    else
    {
            MINT32 Y_SP0 = (MINT32)((double)(CCR_Y_CPY1-CCR_Y_CPY0)*32/(double)CCR_Y_CPX1+0.5);
            CCR_Y_SP0 = (Y_SP0>127)?127:Y_SP0;
    }

    //CCR_Y_SP1
    if(CCR_Y_CPX2 == 0)
        CCR_Y_SP1 = 0;
    else
    {
            MINT32 Y_SP1 = (MINT32)((double)(CCR_Y_CPY2-CCR_Y_CPY1)*32/(double)(CCR_Y_CPX2 - CCR_Y_CPX1)+0.5);
            CCR_Y_SP1 = (Y_SP1>127)?127:Y_SP1;
    }

    //CCR_UV_GAIN_SP1
    if(CCR_UV_X2 == CCR_UV_X1)
        CCR_UV_GAIN_SP1 = 0;
    else
    {
            MINT32 UV_SP1 = (MINT32)((double)(CCR_UV_GAIN1-CCR_UV_GAIN2)*32/(double)(CCR_UV_X2-CCR_UV_X1)+0.5);
            CCR_UV_GAIN_SP1 = (UV_SP1>1023)?1023:UV_SP1;
    }

    //CCR_HUE_SP1
    if(CCR_HUE_X1 == CCR_HUE_X2)
        CCR_HUE_SP1 = (CCR_HUE_GAIN1>CCR_HUE_GAIN2)?-128:127;
    else
    {
            MINT32 HUE_SP1 = (MINT32)((double)(CCR_HUE_GAIN2-CCR_HUE_GAIN1)*32/(double)(CCR_HUE_X2-CCR_HUE_X1)+0.5);
            CCR_HUE_SP1 = (HUE_SP1>127)?127:(HUE_SP1<-128)?-128:HUE_SP1;
    }

    //CCR_HUE_SP2
    if(CCR_HUE_X3 == CCR_HUE_X4)
        CCR_HUE_SP2 = (CCR_HUE_GAIN1>CCR_HUE_GAIN2)?127:-128;
    else
    {
            MINT32 HUE_SP2 = (MINT32)((double)(CCR_HUE_GAIN1-CCR_HUE_GAIN2)*32/(double)(CCR_HUE_X4-CCR_HUE_X3)+0.5);
            CCR_HUE_SP2 = (HUE_SP2>127)?127:(HUE_SP2<-128)?-128:HUE_SP2;
    }

    //  ====================================================================================================================================
    //  Set Smooth ISO setting
    //  ====================================================================================================================================
    rSmoothCCR.con.bits.CCR_EN                  = CCR_EN;
    rSmoothCCR.con.bits.CCR_OR_MODE             = CCR_OR_MODE;
    rSmoothCCR.con.bits.CCR_UV_GAIN_MODE        = CCR_UV_GAIN_MODE;
    rSmoothCCR.con.bits.CCR_UV_GAIN2            = CCR_UV_GAIN2;
    rSmoothCCR.con.bits.CCR_Y_CPX3              = CCR_Y_CPX3;
    rSmoothCCR.ylut.bits.CCR_Y_CPX1             = CCR_Y_CPX1;
    rSmoothCCR.ylut.bits.CCR_Y_CPX2             = CCR_Y_CPX2;
    rSmoothCCR.ylut.bits.CCR_Y_SP1              = CCR_Y_SP1;
    rSmoothCCR.ylut.bits.CCR_Y_CPY1             = CCR_Y_CPY1;
    rSmoothCCR.uvlut.bits.CCR_UV_X1             = CCR_UV_X1;
    rSmoothCCR.uvlut.bits.CCR_UV_X2             = CCR_UV_X2;
    rSmoothCCR.uvlut.bits.CCR_UV_X3             = CCR_UV_X3;
    rSmoothCCR.uvlut.bits.CCR_UV_GAIN1          = CCR_UV_GAIN1;
    rSmoothCCR.ylut2.bits.CCR_Y_SP0             = CCR_Y_SP0;
    rSmoothCCR.ylut2.bits.CCR_Y_SP2             = CCR_Y_SP2;
    rSmoothCCR.ylut2.bits.CCR_Y_CPY0            = CCR_Y_CPY0;
    rSmoothCCR.ylut2.bits.CCR_Y_CPY2            = CCR_Y_CPY2;
    rSmoothCCR.sat_ctrl.bits.CCR_MODE           = CCR_MODE;
    rSmoothCCR.sat_ctrl.bits.CCR_CEN_U          = CCR_CEN_U;
    rSmoothCCR.sat_ctrl.bits.CCR_CEN_V          = CCR_CEN_V;
    rSmoothCCR.uvlut_sp.bits.CCR_UV_GAIN_SP1    = CCR_UV_GAIN_SP1;
    rSmoothCCR.uvlut_sp.bits.CCR_UV_GAIN_SP2    = CCR_UV_GAIN_SP2;
    rSmoothCCR.hue1.bits.CCR_HUE_X1             = CCR_HUE_X1;
    rSmoothCCR.hue1.bits.CCR_HUE_X2             = CCR_HUE_X2;
    rSmoothCCR.hue2.bits.CCR_HUE_X3             = CCR_HUE_X3;
    rSmoothCCR.hue2.bits.CCR_HUE_X4             = CCR_HUE_X4;
    rSmoothCCR.hue3.bits.CCR_HUE_GAIN1          = CCR_HUE_GAIN1;
    rSmoothCCR.hue3.bits.CCR_HUE_GAIN2          = CCR_HUE_GAIN2;

    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_EN           = %d, %d, %d", rLowerCCR.con.bits.CCR_EN            , rSmoothCCR.con.bits.CCR_EN           , rUpperCCR.con.bits.CCR_EN   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_OR_MODE      = %d, %d, %d", rLowerCCR.con.bits.CCR_OR_MODE       , rSmoothCCR.con.bits.CCR_OR_MODE      , rUpperCCR.con.bits.CCR_OR_MODE   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_UV_GAIN_MODE = %d, %d, %d", rLowerCCR.con.bits.CCR_UV_GAIN_MODE  , rSmoothCCR.con.bits.CCR_UV_GAIN_MODE , rUpperCCR.con.bits.CCR_UV_GAIN_MODE   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_UV_GAIN2     = %d, %d, %d", rLowerCCR.con.bits.CCR_UV_GAIN2      , rSmoothCCR.con.bits.CCR_UV_GAIN2     , rUpperCCR.con.bits.CCR_UV_GAIN2   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_Y_CPX3       = %d, %d, %d", rLowerCCR.con.bits.CCR_Y_CPX3        , rSmoothCCR.con.bits.CCR_Y_CPX3       , rUpperCCR.con.bits.CCR_Y_CPX3   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_Y_CPX1       = %d, %d, %d", rLowerCCR.ylut.bits.CCR_Y_CPX1        , rSmoothCCR.ylut.bits.CCR_Y_CPX1       , rUpperCCR.ylut.bits.CCR_Y_CPX1   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_Y_CPX2       = %d, %d, %d", rLowerCCR.ylut.bits.CCR_Y_CPX2        , rSmoothCCR.ylut.bits.CCR_Y_CPX2       , rUpperCCR.ylut.bits.CCR_Y_CPX2   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_Y_SP1        = %d, %d, %d", rLowerCCR.ylut.bits.CCR_Y_SP1         , rSmoothCCR.ylut.bits.CCR_Y_SP1        , rUpperCCR.ylut.bits.CCR_Y_SP1   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_Y_CPY1       = %d, %d, %d", rLowerCCR.ylut.bits.CCR_Y_CPY1        , rSmoothCCR.ylut.bits.CCR_Y_CPY1       , rUpperCCR.ylut.bits.CCR_Y_CPY1   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_UV_X1        = %d, %d, %d", rLowerCCR.uvlut.bits.CCR_UV_X1         , rSmoothCCR.uvlut.bits.CCR_UV_X1        , rUpperCCR.uvlut.bits.CCR_UV_X1   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_UV_X2        = %d, %d, %d", rLowerCCR.uvlut.bits.CCR_UV_X2         , rSmoothCCR.uvlut.bits.CCR_UV_X2        , rUpperCCR.uvlut.bits.CCR_UV_X2   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_UV_X3        = %d, %d, %d", rLowerCCR.uvlut.bits.CCR_UV_X3         , rSmoothCCR.uvlut.bits.CCR_UV_X3        , rUpperCCR.uvlut.bits.CCR_UV_X3   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_UV_GAIN1     = %d, %d, %d", rLowerCCR.uvlut.bits.CCR_UV_GAIN1      , rSmoothCCR.uvlut.bits.CCR_UV_GAIN1     , rUpperCCR.uvlut.bits.CCR_UV_GAIN1   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_Y_SP0        = %d, %d, %d", rLowerCCR.ylut2.bits.CCR_Y_SP0         , rSmoothCCR.ylut2.bits.CCR_Y_SP0        , rUpperCCR.ylut2.bits.CCR_Y_SP0   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_Y_SP2        = %d, %d, %d", rLowerCCR.ylut2.bits.CCR_Y_SP2         , rSmoothCCR.ylut2.bits.CCR_Y_SP2        , rUpperCCR.ylut2.bits.CCR_Y_SP2   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_Y_CPY0       = %d, %d, %d", rLowerCCR.ylut2.bits.CCR_Y_CPY0        , rSmoothCCR.ylut2.bits.CCR_Y_CPY0       , rUpperCCR.ylut2.bits.CCR_Y_CPY0   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_Y_CPY2       = %d, %d, %d", rLowerCCR.ylut2.bits.CCR_Y_CPY2        , rSmoothCCR.ylut2.bits.CCR_Y_CPY2       , rUpperCCR.ylut2.bits.CCR_Y_CPY2   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_MODE         = %d, %d, %d", rLowerCCR.sat_ctrl.bits.CCR_MODE          , rSmoothCCR.sat_ctrl.bits.CCR_MODE         , rUpperCCR.sat_ctrl.bits.CCR_MODE   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_CEN_U        = %d, %d, %d", rLowerCCR.sat_ctrl.bits.CCR_CEN_U         , rSmoothCCR.sat_ctrl.bits.CCR_CEN_U        , rUpperCCR.sat_ctrl.bits.CCR_CEN_U   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_CEN_V        = %d, %d, %d", rLowerCCR.sat_ctrl.bits.CCR_CEN_V         , rSmoothCCR.sat_ctrl.bits.CCR_CEN_V        , rUpperCCR.sat_ctrl.bits.CCR_CEN_V   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_UV_GAIN_SP1  = %d, %d, %d", rLowerCCR.uvlut_sp.bits.CCR_UV_GAIN_SP1   , rSmoothCCR.uvlut_sp.bits.CCR_UV_GAIN_SP1  , rUpperCCR.uvlut_sp.bits.CCR_UV_GAIN_SP1   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_UV_GAIN_SP2  = %d, %d, %d", rLowerCCR.uvlut_sp.bits.CCR_UV_GAIN_SP2   , rSmoothCCR.uvlut_sp.bits.CCR_UV_GAIN_SP2  , rUpperCCR.uvlut_sp.bits.CCR_UV_GAIN_SP2   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_HUE_X1       = %d, %d, %d", rLowerCCR.hue1.bits.CCR_HUE_X1        , rSmoothCCR.hue1.bits.CCR_HUE_X1       , rUpperCCR.hue1.bits.CCR_HUE_X1   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_HUE_X2       = %d, %d, %d", rLowerCCR.hue1.bits.CCR_HUE_X2        , rSmoothCCR.hue1.bits.CCR_HUE_X2       , rUpperCCR.hue1.bits.CCR_HUE_X2   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_HUE_X3       = %d, %d, %d", rLowerCCR.hue2.bits.CCR_HUE_X3        , rSmoothCCR.hue2.bits.CCR_HUE_X3       , rUpperCCR.hue2.bits.CCR_HUE_X3   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_HUE_X4       = %d, %d, %d", rLowerCCR.hue2.bits.CCR_HUE_X4        , rSmoothCCR.hue2.bits.CCR_HUE_X4       , rUpperCCR.hue2.bits.CCR_HUE_X4   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_HUE_GAIN1    = %d, %d, %d", rLowerCCR.hue3.bits.CCR_HUE_GAIN1     , rSmoothCCR.hue3.bits.CCR_HUE_GAIN1    , rUpperCCR.hue3.bits.CCR_HUE_GAIN1   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_HUE_GAIN2    = %d, %d, %d", rLowerCCR.hue3.bits.CCR_HUE_GAIN2     , rSmoothCCR.hue3.bits.CCR_HUE_GAIN2    , rUpperCCR.hue3.bits.CCR_HUE_GAIN2   );
}

MINT32 InterParam_PCA(MINT32 u4RealCT, MINT32 u4UpperCT, MINT32 u4LowerCT, MINT32 PARAMupper, MINT32 PARAMlower){
    MINT32 InterPARAM = 0;

    if(u4UpperCT == u4LowerCT){
       // To prevent someone sets different PARAMpre and PARAMpos
        return (MINT32)((PARAMupper + PARAMlower + 1) / 2);
    }
    
    double RATIOpre = (double)(u4UpperCT - u4RealCT) / (double)(u4UpperCT - u4LowerCT);
    double RATIOpos = 1.0 - (double)(RATIOpre);
    double temp =  RATIOpre * PARAMlower + RATIOpos * PARAMupper;
    if(temp>0)
    	InterPARAM = (MINT32)(RATIOpre * PARAMlower + RATIOpos * PARAMupper + 0.5);
    else
    	InterPARAM = (MINT32)(RATIOpre * PARAMlower + RATIOpos * PARAMupper - 0.5);

    return InterPARAM;
}

MVOID SmoothPCA_TBL(MUINT32 u4RealCT,  // Real CT
               MUINT32 u4UpperCT, // Upper CT
               MUINT32 u4LowerCT, // Lower CT
               ISP_NVRAM_PCA_LUT_T const& rUpperPCA_TBL, 
               ISP_NVRAM_PCA_LUT_T const& rLowerPCA_TBL,
               ISP_NVRAM_PCA_LUT_T& rSmoothPCA_TBL,
               MUINT32 u4RealLV,
               ISP_NVRAM_PCA_PARAM_T const& Para_PCA)
{
    //return upper PCA
    if(u4RealCT == 0 || u4UpperCT == 0 || u4LowerCT == 0)
    {
        for(int i = 0; i < PCA_BIN_NUM; ++i) 
        {
            rSmoothPCA_TBL.lut[i].bits.PCA_LUMA_GAIN = rUpperPCA_TBL.lut[i].bits.PCA_LUMA_GAIN;
            rSmoothPCA_TBL.lut[i].bits.PCA_SAT_GAIN = rUpperPCA_TBL.lut[i].bits.PCA_SAT_GAIN;
            rSmoothPCA_TBL.lut[i].bits.PCA_HUE_SHIFT = rUpperPCA_TBL.lut[i].bits.PCA_HUE_SHIFT;
        }    	
        return;
    }

    //LV tuning
    double Rmax = 64; //fix value
    double lv_ratio = 1.0;

    u4RealCT = 1000000/u4RealCT;
    u4UpperCT = 1000000/u4UpperCT;
    u4LowerCT = 1000000/u4LowerCT;

    if (u4RealLV < Para_PCA.u4PCA_LV_H) //else lv_ratio = 1.0
    {
    	if (u4RealLV > Para_PCA.u4PCA_LV_L) //else R = Para_PCA.PCA_R_MIN 
    	{
    		lv_ratio = ( double(u4RealLV -Para_PCA.u4PCA_LV_L )/ (Para_PCA.u4PCA_LV_H - Para_PCA.u4PCA_LV_L) * (Rmax - Para_PCA.uPCA_R_MIN) + Para_PCA.uPCA_R_MIN);
    		lv_ratio /= Rmax ;
    	}
    	else
        {
    		lv_ratio = Para_PCA.uPCA_R_MIN/Rmax;
        }   
    }

    //CT interpolation
    for(int i = 0; i < PCA_BIN_NUM; ++i) 
    {
        MINT32 PCA_TBL_LUMA_upper = rUpperPCA_TBL.lut[i].bits.PCA_LUMA_GAIN;
        MINT32 PCA_TBL_SAT_upper = rUpperPCA_TBL.lut[i].bits.PCA_SAT_GAIN;
        MINT32 PCA_TBL_HUE_upper = rUpperPCA_TBL.lut[i].bits.PCA_HUE_SHIFT;

        MINT32 PCA_TBL_LUMA_lower = rLowerPCA_TBL.lut[i].bits.PCA_LUMA_GAIN;
        MINT32 PCA_TBL_SAT_lower = rLowerPCA_TBL.lut[i].bits.PCA_SAT_GAIN;
        MINT32 PCA_TBL_HUE_lower = rLowerPCA_TBL.lut[i].bits.PCA_HUE_SHIFT;               		

        PCA_TBL_LUMA_upper = (PCA_TBL_LUMA_upper > 127)? PCA_TBL_LUMA_upper-256:PCA_TBL_LUMA_upper;
        PCA_TBL_SAT_upper = (PCA_TBL_SAT_upper > 127)? PCA_TBL_SAT_upper-256:PCA_TBL_SAT_upper;
        PCA_TBL_HUE_upper = (PCA_TBL_HUE_upper > 127)? PCA_TBL_HUE_upper-256:PCA_TBL_HUE_upper;
        PCA_TBL_LUMA_lower = (PCA_TBL_LUMA_lower > 127)? PCA_TBL_LUMA_lower-256:PCA_TBL_LUMA_lower;
        PCA_TBL_SAT_lower = (PCA_TBL_SAT_lower > 127)? PCA_TBL_SAT_lower-256:PCA_TBL_SAT_lower;
        PCA_TBL_HUE_lower = (PCA_TBL_HUE_lower > 127)? PCA_TBL_HUE_lower-256:PCA_TBL_HUE_lower;

        MINT32 PCA_TBL_LUMA = InterParam_PCA(u4RealCT, u4UpperCT,u4LowerCT, PCA_TBL_LUMA_upper,PCA_TBL_LUMA_lower);
        MINT32 PCA_TBL_SAT = InterParam_PCA(u4RealCT, u4UpperCT,u4LowerCT, PCA_TBL_SAT_upper,PCA_TBL_SAT_lower);
        MINT32 PCA_TBL_HUE = InterParam_PCA(u4RealCT, u4UpperCT,u4LowerCT, PCA_TBL_HUE_upper,PCA_TBL_HUE_lower);

        //LV tuning	//There is no LV tuning for PCA_TBL_LUMA, only CT interpolation
        if(lv_ratio != 1.0) 
        {
            PCA_TBL_SAT = PCA_TBL_SAT * lv_ratio;
            PCA_TBL_HUE = PCA_TBL_HUE * lv_ratio;

            double sat_flat_strength = Para_PCA.uPCA_SAT_FLAT_STR * (1.0-lv_ratio);
            double hue_flat_strength = Para_PCA.uPCA_HUE_FLAT_STR * (1.0-lv_ratio);

            //smaller lv_ratio is, stronger flat effect
            PCA_TBL_SAT = ((16 - sat_flat_strength )* PCA_TBL_SAT + sat_flat_strength * Para_PCA.uPCA_SAT_FLAT )/16;
            PCA_TBL_HUE = ((16 - hue_flat_strength )* PCA_TBL_HUE + hue_flat_strength * Para_PCA.uPCA_HUE_FLAT)/16;
        }

        PCA_TBL_LUMA = (PCA_TBL_LUMA < 0 )? PCA_TBL_LUMA+256:PCA_TBL_LUMA; 
        PCA_TBL_SAT = (PCA_TBL_SAT < 0 )? PCA_TBL_SAT +256:PCA_TBL_SAT; 
        PCA_TBL_HUE = (PCA_TBL_HUE < 0 )? PCA_TBL_HUE +256:PCA_TBL_HUE; 
		rSmoothPCA_TBL.lut[i].bits.PCA_LUMA_GAIN = PCA_TBL_LUMA;
		rSmoothPCA_TBL.lut[i].bits.PCA_SAT_GAIN = PCA_TBL_SAT;
		rSmoothPCA_TBL.lut[i].bits.PCA_HUE_SHIFT = PCA_TBL_HUE;
     }
}

MINT32 InterParam_EE(MINT32 ISO, MINT32 ISOpre, MINT32 ISOpos, MINT32 PARAMpre, MINT32 PARAMpos, MINT32 Method){
    MINT32 InterPARAM = 0;

    if(ISOpre == ISOpos){
        //  To prevent someone sets different PARAMpre and PARAMpos
        return (MINT32)((PARAMpre + PARAMpos + 1) / 2);
    }

    switch(Method){
    case 1:
        InterPARAM = (abs((double)(ISO - ISOpre)) > abs((double)(ISOpos - ISO))) ? PARAMpos : PARAMpre;
        break;
    case 0:
    default:
        double RATIOpre = (double)(ISOpos - ISO) / (double)(ISOpos - ISOpre);
        double RATIOpos = 1.0 - (double)(RATIOpre);
        InterPARAM = (MINT32)(RATIOpre * PARAMpre + RATIOpos * PARAMpos + 0.5);
        break;
    }

    return InterPARAM;
}


MVOID SmoothEE(MUINT32 u4RealISO,  // Real ISO
               MUINT32 u4UpperISO, // Upper ISO
               MUINT32 u4LowerISO, // Lower ISO
               ISP_NVRAM_EE_T const& rUpperEE, // EE settings for upper ISO
               ISP_NVRAM_EE_T const& rLowerEE,   // EE settings for lower ISO
               ISP_NVRAM_EE_T& rSmoothEE)   // Output
{       // Output

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.smooth_ee.enable", value, "0");
    MBOOL bSmoothEEDebug = atoi(value);

    MY_LOG_IF(bSmoothEEDebug,"[%s()] u4RealISO: %d, u4UpperISO = %d, u4LowerISO = %d\n", __FUNCTION__, u4RealISO, u4UpperISO, u4LowerISO);

    //  ====================================================================================================================================
    //  Get Lower ISO setting
    //  ====================================================================================================================================
    MINT32 SEEE_H1_DI_BLND_OFSTpre          = rLowerEE.blnd_ctrl_1.bits.SEEE_H1_DI_BLND_OFST;
    MINT32 SEEE_H2_DI_BLND_OFSTpre          = rLowerEE.blnd_ctrl_1.bits.SEEE_H2_DI_BLND_OFST;
    MINT32 SEEE_H3_DI_BLND_OFSTpre          = rLowerEE.blnd_ctrl_1.bits.SEEE_H3_DI_BLND_OFST;
    MINT32 SEEE_H1_DI_BLND_SLpre            = rLowerEE.blnd_ctrl_2.bits.SEEE_H1_DI_BLND_SL;
    MINT32 SEEE_H2_DI_BLND_SLpre            = rLowerEE.blnd_ctrl_2.bits.SEEE_H2_DI_BLND_SL;
    MINT32 SEEE_H3_DI_BLND_SLpre            = rLowerEE.blnd_ctrl_2.bits.SEEE_H3_DI_BLND_SL;
    MINT32 SEEE_HX_ISO_BLND_RATpre          = rLowerEE.blnd_ctrl_2.bits.SEEE_HX_ISO_BLND_RAT;
    MINT32 SEEE_H1_GNpre                    = rLowerEE.gn_ctrl.bits.SEEE_H1_GN;
    MINT32 SEEE_H2_GNpre                    = rLowerEE.gn_ctrl.bits.SEEE_H2_GN;
    MINT32 SEEE_H3_GNpre                    = rLowerEE.gn_ctrl.bits.SEEE_H3_GN;
    MINT32 SEEE_H4_GNpre                    = rLowerEE.gn_ctrl.bits.SEEE_H4_GN;
    MINT32 SEEE_FLT_CORE_THpre              = rLowerEE.clip_ctrl_3.bits.SEEE_FLT_CORE_TH;
    MINT32 SEEE_LUMA_MOD_Y0pre              = rLowerEE.luma_ctrl_1.bits.SEEE_LUMA_MOD_Y0;
    MINT32 SEEE_LUMA_MOD_Y1pre              = rLowerEE.luma_ctrl_1.bits.SEEE_LUMA_MOD_Y1;
    MINT32 SEEE_LUMA_MOD_Y2pre              = rLowerEE.luma_ctrl_2.bits.SEEE_LUMA_MOD_Y2;
    MINT32 SEEE_LUMA_MOD_Y3pre              = rLowerEE.luma_ctrl_2.bits.SEEE_LUMA_MOD_Y3;
    MINT32 SEEE_LUMA_MOD_Y4pre              = rLowerEE.luma_ctrl_3.bits.SEEE_LUMA_MOD_Y4;
    MINT32 SEEE_LUMA_MOD_Y5pre              = rLowerEE.luma_ctrl_3.bits.SEEE_LUMA_MOD_Y5;
    MINT32 SEEE_LUMA_MOD_Y6pre              = rLowerEE.luma_ctrl_4.bits.SEEE_LUMA_MOD_Y6;
    MINT32 SEEE_SLNK_GN_Y1pre               = rLowerEE.slnk_ctrl_1.bits.SEEE_SLNK_GN_Y1;
    MINT32 SEEE_SLNK_GN_Y2pre               = rLowerEE.slnk_ctrl_1.bits.SEEE_SLNK_GN_Y2;
    MINT32 SEEE_RESP_SLNK_GN_RATpre         = rLowerEE.slnk_ctrl_2.bits.SEEE_RESP_SLNK_GN_RAT;
    MINT32 SEEE_GLUT_X1pre                  = rLowerEE.glut_ctrl_1.bits.SEEE_GLUT_X1;
    MINT32 SEEE_GLUT_S1pre                  = rLowerEE.glut_ctrl_1.bits.SEEE_GLUT_S1;
    MINT32 SEEE_GLUT_Y1pre                  = rLowerEE.glut_ctrl_1.bits.SEEE_GLUT_Y1;
    MINT32 SEEE_GLUT_X2pre                  = rLowerEE.glut_ctrl_2.bits.SEEE_GLUT_X2;
    MINT32 SEEE_GLUT_S2pre                  = rLowerEE.glut_ctrl_2.bits.SEEE_GLUT_S2;
    MINT32 SEEE_GLUT_Y2pre                  = rLowerEE.glut_ctrl_2.bits.SEEE_GLUT_Y2;
    MINT32 SEEE_GLUT_X3pre                  = rLowerEE.glut_ctrl_3.bits.SEEE_GLUT_X3;
    MINT32 SEEE_GLUT_S3pre                  = rLowerEE.glut_ctrl_3.bits.SEEE_GLUT_S3;
    MINT32 SEEE_GLUT_Y3pre                  = rLowerEE.glut_ctrl_3.bits.SEEE_GLUT_Y3;
    MINT32 SEEE_GLUT_X4pre                  = rLowerEE.glut_ctrl_4.bits.SEEE_GLUT_X4;
    MINT32 SEEE_GLUT_S4pre                  = rLowerEE.glut_ctrl_4.bits.SEEE_GLUT_S4;
    MINT32 SEEE_GLUT_Y4pre                  = rLowerEE.glut_ctrl_4.bits.SEEE_GLUT_Y4;
    MINT32 SEEE_GLUT_S5pre                  = rLowerEE.glut_ctrl_5.bits.SEEE_GLUT_S5;
    MINT32 SEEE_GLUT_TH_OVRpre              = rLowerEE.glut_ctrl_6.bits.SEEE_GLUT_TH_OVR;
    MINT32 SEEE_GLUT_TH_UNDpre              = rLowerEE.glut_ctrl_6.bits.SEEE_GLUT_TH_UND;
    MINT32 SEEE_GLUT_TH_MINpre              = rLowerEE.glut_ctrl_6.bits.SEEE_GLUT_TH_MIN;
    MINT32 SEEE_GLUT_LINK_ENpre             = rLowerEE.slnk_ctrl_2.bits.SEEE_GLUT_LINK_EN;
    MINT32 SEEE_GLUT_SL_DEC_Ypre            = rLowerEE.slnk_ctrl_2.bits.SEEE_GLUT_SL_DEC_Y;
    MINT32 SEEE_OVRSH_CLIP_STRpre           = rLowerEE.clip_ctrl_3.bits.SEEE_OVRSH_CLIP_STR;
    MINT32 SEEE_DOT_THpre                   = rLowerEE.clip_ctrl_1.bits.SEEE_DOT_TH;
    MINT32 SEEE_DOT_REDUC_AMNTpre           = rLowerEE.clip_ctrl_1.bits.SEEE_DOT_REDUC_AMNT;
    MINT32 SEEE_RESP_CLIPpre                = rLowerEE.clip_ctrl_2.bits.SEEE_RESP_CLIP;
    MINT32 SEEE_RESP_CLIP_LUMA_SPC_THpre    = rLowerEE.clip_ctrl_2.bits.SEEE_RESP_CLIP_LUMA_SPC_TH;
    MINT32 SEEE_RESP_CLIP_LUMA_LWBpre       = rLowerEE.clip_ctrl_2.bits.SEEE_RESP_CLIP_LUMA_LWB;
    MINT32 SEEE_RESP_CLIP_LUMA_UPBpre       = rLowerEE.clip_ctrl_2.bits.SEEE_RESP_CLIP_LUMA_UPB;

    //  ====================================================================================================================================
    //  Get Upper ISO setting
    //  ====================================================================================================================================
    MINT32 SEEE_H1_DI_BLND_OFSTpos          = rUpperEE.blnd_ctrl_1.bits.SEEE_H1_DI_BLND_OFST;
    MINT32 SEEE_H2_DI_BLND_OFSTpos          = rUpperEE.blnd_ctrl_1.bits.SEEE_H2_DI_BLND_OFST;
    MINT32 SEEE_H3_DI_BLND_OFSTpos          = rUpperEE.blnd_ctrl_1.bits.SEEE_H3_DI_BLND_OFST;
    MINT32 SEEE_H1_DI_BLND_SLpos            = rUpperEE.blnd_ctrl_2.bits.SEEE_H1_DI_BLND_SL;
    MINT32 SEEE_H2_DI_BLND_SLpos            = rUpperEE.blnd_ctrl_2.bits.SEEE_H2_DI_BLND_SL;
    MINT32 SEEE_H3_DI_BLND_SLpos            = rUpperEE.blnd_ctrl_2.bits.SEEE_H3_DI_BLND_SL;
    MINT32 SEEE_HX_ISO_BLND_RATpos          = rUpperEE.blnd_ctrl_2.bits.SEEE_HX_ISO_BLND_RAT;
    MINT32 SEEE_H1_GNpos                    = rUpperEE.gn_ctrl.bits.SEEE_H1_GN;
    MINT32 SEEE_H2_GNpos                    = rUpperEE.gn_ctrl.bits.SEEE_H2_GN;
    MINT32 SEEE_H3_GNpos                    = rUpperEE.gn_ctrl.bits.SEEE_H3_GN;
    MINT32 SEEE_H4_GNpos                    = rUpperEE.gn_ctrl.bits.SEEE_H4_GN;
    MINT32 SEEE_FLT_CORE_THpos              = rUpperEE.clip_ctrl_3.bits.SEEE_FLT_CORE_TH;
    MINT32 SEEE_LUMA_MOD_Y0pos              = rUpperEE.luma_ctrl_1.bits.SEEE_LUMA_MOD_Y0;
    MINT32 SEEE_LUMA_MOD_Y1pos              = rUpperEE.luma_ctrl_1.bits.SEEE_LUMA_MOD_Y1;
    MINT32 SEEE_LUMA_MOD_Y2pos              = rUpperEE.luma_ctrl_2.bits.SEEE_LUMA_MOD_Y2;
    MINT32 SEEE_LUMA_MOD_Y3pos              = rUpperEE.luma_ctrl_2.bits.SEEE_LUMA_MOD_Y3;
    MINT32 SEEE_LUMA_MOD_Y4pos              = rUpperEE.luma_ctrl_3.bits.SEEE_LUMA_MOD_Y4;
    MINT32 SEEE_LUMA_MOD_Y5pos              = rUpperEE.luma_ctrl_3.bits.SEEE_LUMA_MOD_Y5;
    MINT32 SEEE_LUMA_MOD_Y6pos              = rUpperEE.luma_ctrl_4.bits.SEEE_LUMA_MOD_Y6;
    MINT32 SEEE_SLNK_GN_Y1pos               = rUpperEE.slnk_ctrl_1.bits.SEEE_SLNK_GN_Y1;
    MINT32 SEEE_SLNK_GN_Y2pos               = rUpperEE.slnk_ctrl_1.bits.SEEE_SLNK_GN_Y2;
    MINT32 SEEE_RESP_SLNK_GN_RATpos         = rUpperEE.slnk_ctrl_2.bits.SEEE_RESP_SLNK_GN_RAT;
    MINT32 SEEE_GLUT_X1pos                  = rUpperEE.glut_ctrl_1.bits.SEEE_GLUT_X1;
    MINT32 SEEE_GLUT_S1pos                  = rUpperEE.glut_ctrl_1.bits.SEEE_GLUT_S1;
    MINT32 SEEE_GLUT_Y1pos                  = rUpperEE.glut_ctrl_1.bits.SEEE_GLUT_Y1;
    MINT32 SEEE_GLUT_X2pos                  = rUpperEE.glut_ctrl_2.bits.SEEE_GLUT_X2;
    MINT32 SEEE_GLUT_S2pos                  = rUpperEE.glut_ctrl_2.bits.SEEE_GLUT_S2;
    MINT32 SEEE_GLUT_Y2pos                  = rUpperEE.glut_ctrl_2.bits.SEEE_GLUT_Y2;
    MINT32 SEEE_GLUT_X3pos                  = rUpperEE.glut_ctrl_3.bits.SEEE_GLUT_X3;
    MINT32 SEEE_GLUT_S3pos                  = rUpperEE.glut_ctrl_3.bits.SEEE_GLUT_S3;
    MINT32 SEEE_GLUT_Y3pos                  = rUpperEE.glut_ctrl_3.bits.SEEE_GLUT_Y3;
    MINT32 SEEE_GLUT_X4pos                  = rUpperEE.glut_ctrl_4.bits.SEEE_GLUT_X4;
    MINT32 SEEE_GLUT_S4pos                  = rUpperEE.glut_ctrl_4.bits.SEEE_GLUT_S4;
    MINT32 SEEE_GLUT_Y4pos                  = rUpperEE.glut_ctrl_4.bits.SEEE_GLUT_Y4;
    MINT32 SEEE_GLUT_S5pos                  = rUpperEE.glut_ctrl_5.bits.SEEE_GLUT_S5;
    MINT32 SEEE_GLUT_TH_OVRpos              = rUpperEE.glut_ctrl_6.bits.SEEE_GLUT_TH_OVR;
    MINT32 SEEE_GLUT_TH_UNDpos              = rUpperEE.glut_ctrl_6.bits.SEEE_GLUT_TH_UND;
    MINT32 SEEE_GLUT_TH_MINpos              = rUpperEE.glut_ctrl_6.bits.SEEE_GLUT_TH_MIN;
    MINT32 SEEE_GLUT_LINK_ENpos             = rUpperEE.slnk_ctrl_2.bits.SEEE_GLUT_LINK_EN;
    MINT32 SEEE_GLUT_SL_DEC_Ypos            = rUpperEE.slnk_ctrl_2.bits.SEEE_GLUT_SL_DEC_Y;
    MINT32 SEEE_OVRSH_CLIP_STRpos           = rUpperEE.clip_ctrl_3.bits.SEEE_OVRSH_CLIP_STR;
    MINT32 SEEE_DOT_THpos                   = rUpperEE.clip_ctrl_1.bits.SEEE_DOT_TH;
    MINT32 SEEE_DOT_REDUC_AMNTpos           = rUpperEE.clip_ctrl_1.bits.SEEE_DOT_REDUC_AMNT;
    MINT32 SEEE_RESP_CLIPpos                = rUpperEE.clip_ctrl_2.bits.SEEE_RESP_CLIP;
    MINT32 SEEE_RESP_CLIP_LUMA_SPC_THpos    = rUpperEE.clip_ctrl_2.bits.SEEE_RESP_CLIP_LUMA_SPC_TH;
    MINT32 SEEE_RESP_CLIP_LUMA_LWBpos       = rUpperEE.clip_ctrl_2.bits.SEEE_RESP_CLIP_LUMA_LWB;
    MINT32 SEEE_RESP_CLIP_LUMA_UPBpos       = rUpperEE.clip_ctrl_2.bits.SEEE_RESP_CLIP_LUMA_UPB;

    //  Negative value for slope
    SEEE_GLUT_S1pre = (SEEE_GLUT_S1pre > 127) ? (SEEE_GLUT_S1pre - 256) : SEEE_GLUT_S1pre;
    SEEE_GLUT_S2pre = (SEEE_GLUT_S2pre > 127) ? (SEEE_GLUT_S2pre - 256) : SEEE_GLUT_S2pre;
    SEEE_GLUT_S3pre = (SEEE_GLUT_S3pre > 127) ? (SEEE_GLUT_S3pre - 256) : SEEE_GLUT_S3pre;
    SEEE_GLUT_S4pre = (SEEE_GLUT_S4pre > 127) ? (SEEE_GLUT_S4pre - 256) : SEEE_GLUT_S4pre;
    SEEE_GLUT_S5pre = (SEEE_GLUT_S5pre > 127) ? (SEEE_GLUT_S5pre - 256) : SEEE_GLUT_S5pre;

    SEEE_GLUT_S1pos = (SEEE_GLUT_S1pos > 127) ? (SEEE_GLUT_S1pos - 256) : SEEE_GLUT_S1pos;
    SEEE_GLUT_S2pos = (SEEE_GLUT_S2pos > 127) ? (SEEE_GLUT_S2pos - 256) : SEEE_GLUT_S2pos;
    SEEE_GLUT_S3pos = (SEEE_GLUT_S3pos > 127) ? (SEEE_GLUT_S3pos - 256) : SEEE_GLUT_S3pos;
    SEEE_GLUT_S4pos = (SEEE_GLUT_S4pos > 127) ? (SEEE_GLUT_S4pos - 256) : SEEE_GLUT_S4pos;
    SEEE_GLUT_S5pos = (SEEE_GLUT_S5pos > 127) ? (SEEE_GLUT_S5pos - 256) : SEEE_GLUT_S5pos;

    //  ====================================================================================================================================
    //  Start Parameter Interpolation
    //  ====================================================================================================================================
    MINT32 ISO = u4RealISO;
    MINT32 ISOpre = u4LowerISO;
    MINT32 ISOpos = u4UpperISO;

    //  Registers that can be interpolated
    MINT32 SEEE_H1_DI_BLND_OFST         = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_H1_DI_BLND_OFSTpre        , SEEE_H1_DI_BLND_OFSTpos       , 0);
    MINT32 SEEE_H2_DI_BLND_OFST         = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_H2_DI_BLND_OFSTpre        , SEEE_H2_DI_BLND_OFSTpos       , 0);
    MINT32 SEEE_H3_DI_BLND_OFST         = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_H3_DI_BLND_OFSTpre        , SEEE_H3_DI_BLND_OFSTpos       , 0);
    MINT32 SEEE_H1_DI_BLND_SL           = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_H1_DI_BLND_SLpre          , SEEE_H1_DI_BLND_SLpos         , 0);
    MINT32 SEEE_H2_DI_BLND_SL           = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_H2_DI_BLND_SLpre          , SEEE_H2_DI_BLND_SLpos         , 0);
    MINT32 SEEE_H3_DI_BLND_SL           = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_H3_DI_BLND_SLpre          , SEEE_H3_DI_BLND_SLpos         , 0);
    MINT32 SEEE_HX_ISO_BLND_RAT         = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_HX_ISO_BLND_RATpre        , SEEE_HX_ISO_BLND_RATpos       , 0);
    MINT32 SEEE_H1_GN                   = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_H1_GNpre                  , SEEE_H1_GNpos                 , 0);
    MINT32 SEEE_H2_GN                   = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_H2_GNpre                  , SEEE_H2_GNpos                 , 0);
    MINT32 SEEE_H3_GN                   = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_H3_GNpre                  , SEEE_H3_GNpos                 , 0);
    MINT32 SEEE_H4_GN                   = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_H4_GNpre                  , SEEE_H4_GNpos                 , 0);
    MINT32 SEEE_FLT_CORE_TH             = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_FLT_CORE_THpre            , SEEE_FLT_CORE_THpos           , 0);
    MINT32 SEEE_LUMA_MOD_Y0             = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_LUMA_MOD_Y0pre            , SEEE_LUMA_MOD_Y0pos           , 0);
    MINT32 SEEE_LUMA_MOD_Y1             = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_LUMA_MOD_Y1pre            , SEEE_LUMA_MOD_Y1pos           , 0);
    MINT32 SEEE_LUMA_MOD_Y2             = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_LUMA_MOD_Y2pre            , SEEE_LUMA_MOD_Y2pos           , 0);
    MINT32 SEEE_LUMA_MOD_Y3             = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_LUMA_MOD_Y3pre            , SEEE_LUMA_MOD_Y3pos           , 0);
    MINT32 SEEE_LUMA_MOD_Y4             = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_LUMA_MOD_Y4pre            , SEEE_LUMA_MOD_Y4pos           , 0);
    MINT32 SEEE_LUMA_MOD_Y5             = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_LUMA_MOD_Y5pre            , SEEE_LUMA_MOD_Y5pos           , 0);
    MINT32 SEEE_LUMA_MOD_Y6             = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_LUMA_MOD_Y6pre            , SEEE_LUMA_MOD_Y6pos           , 0);
    MINT32 SEEE_SLNK_GN_Y1              = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_SLNK_GN_Y1pre             , SEEE_SLNK_GN_Y1pos            , 0);
    MINT32 SEEE_SLNK_GN_Y2              = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_SLNK_GN_Y2pre             , SEEE_SLNK_GN_Y2pos            , 0);
    MINT32 SEEE_RESP_SLNK_GN_RAT        = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_RESP_SLNK_GN_RATpre       , SEEE_RESP_SLNK_GN_RATpos      , 0);
    MINT32 SEEE_GLUT_X1                 = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_GLUT_X1pre                , SEEE_GLUT_X1pos               , 0);
    MINT32 SEEE_GLUT_X2                 = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_GLUT_X2pre                , SEEE_GLUT_X2pos               , 0);
    MINT32 SEEE_GLUT_X3                 = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_GLUT_X3pre                , SEEE_GLUT_X3pos               , 0);
    MINT32 SEEE_GLUT_X4                 = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_GLUT_X4pre                , SEEE_GLUT_X4pos               , 0);
    MINT32 SEEE_GLUT_Y1                 = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_GLUT_Y1pre                , SEEE_GLUT_Y1pos               , 0);
    MINT32 SEEE_GLUT_Y2                 = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_GLUT_Y2pre                , SEEE_GLUT_Y2pos               , 0);
    MINT32 SEEE_GLUT_Y3                 = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_GLUT_Y3pre                , SEEE_GLUT_Y3pos               , 0);
    MINT32 SEEE_GLUT_Y4                 = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_GLUT_Y4pre                , SEEE_GLUT_Y4pos               , 0);
    MINT32 SEEE_GLUT_TH_OVR             = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_GLUT_TH_OVRpre            , SEEE_GLUT_TH_OVRpos           , 0);
    MINT32 SEEE_GLUT_TH_UND             = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_GLUT_TH_UNDpre            , SEEE_GLUT_TH_UNDpos           , 0);
    MINT32 SEEE_GLUT_TH_MIN             = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_GLUT_TH_MINpre            , SEEE_GLUT_TH_MINpos           , 0);
    MINT32 SEEE_GLUT_LINK_EN            = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_GLUT_LINK_ENpre           , SEEE_GLUT_LINK_ENpos          , 0);
    MINT32 SEEE_GLUT_SL_DEC_Y           = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_GLUT_SL_DEC_Ypre          , SEEE_GLUT_SL_DEC_Ypos         , 0);
    MINT32 SEEE_OVRSH_CLIP_STR          = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_OVRSH_CLIP_STRpre         , SEEE_OVRSH_CLIP_STRpos        , 0);
    MINT32 SEEE_DOT_TH                  = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_DOT_THpre                 , SEEE_DOT_THpos                , 0);
    MINT32 SEEE_DOT_REDUC_AMNT          = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_DOT_REDUC_AMNTpre         , SEEE_DOT_REDUC_AMNTpos        , 0);
    MINT32 SEEE_RESP_CLIP               = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_RESP_CLIPpre              , SEEE_RESP_CLIPpos             , 0);
    MINT32 SEEE_RESP_CLIP_LUMA_SPC_TH   = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_RESP_CLIP_LUMA_SPC_THpre  , SEEE_RESP_CLIP_LUMA_SPC_THpos , 0);
    MINT32 SEEE_RESP_CLIP_LUMA_LWB      = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_RESP_CLIP_LUMA_LWBpre     , SEEE_RESP_CLIP_LUMA_LWBpos    , 0);
    MINT32 SEEE_RESP_CLIP_LUMA_UPB      = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_RESP_CLIP_LUMA_UPBpre     , SEEE_RESP_CLIP_LUMA_UPBpos    , 0);

    //  To calculate S5, we need to find Y5 first.
    //  Y5 is not a real register. So we use (X4, Y4) and S5 to evaluate (255, Y5) for pre-ISO and post-ISO setting.
    //  Then we get the interpolated Y5.
    MINT32 S5pre_sign       = (SEEE_GLUT_S5pre >= 0) ? 1 : -1;
    MINT32 S5pos_sign       = (SEEE_GLUT_S5pos >= 0) ? 1 : -1;
    MINT32 SEEE_GLUT_Y5pre  = SEEE_GLUT_Y4pre + (SEEE_GLUT_S5pre * (255 - SEEE_GLUT_X4pre) + 8 * S5pre_sign) / 16;
    MINT32 SEEE_GLUT_Y5pos  = SEEE_GLUT_Y4pos + (SEEE_GLUT_S5pos * (255 - SEEE_GLUT_X4pos) + 8 * S5pos_sign) / 16;
    MINT32 SEEE_GLUT_Y5     = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_GLUT_Y5pre, SEEE_GLUT_Y5pos, 0);

    MINT32 SEEE_GLUT_S1;
    MINT32 SEEE_GLUT_S2;
    MINT32 SEEE_GLUT_S3;
    MINT32 SEEE_GLUT_S4;
    MINT32 SEEE_GLUT_S5;
    //  Due to slope can not be interpolated for most cases, they need to be re-calculated.
    if(SEEE_GLUT_X1 == 0){
        SEEE_GLUT_S1 = 0;
    }
    else{
        MINT32 S1 = (MINT32)((double)(SEEE_GLUT_Y1 << 4) / (double)SEEE_GLUT_X1 + 0.5);
        SEEE_GLUT_S1 = (S1 > 127) ? 127 : S1;
    }

    if((SEEE_GLUT_Y2 - SEEE_GLUT_Y1) > 0){
        MINT32 S2 = (MINT32)((double)((SEEE_GLUT_Y2 - SEEE_GLUT_Y1) << 4) / (double)(SEEE_GLUT_X2 - SEEE_GLUT_X1) + 0.5);
        SEEE_GLUT_S2 = (S2 > 127) ? 127 : S2;
    }
    else{
        MINT32 S2 = (MINT32)((double)((SEEE_GLUT_Y2 - SEEE_GLUT_Y1) << 4) / (double)(SEEE_GLUT_X2 - SEEE_GLUT_X1) - 0.5);
        SEEE_GLUT_S2 = (S2 < -128) ? -128 : S2;
    }

    if((SEEE_GLUT_Y3 - SEEE_GLUT_Y2) > 0){
        MINT32 S3 = (MINT32)((double)((SEEE_GLUT_Y3 - SEEE_GLUT_Y2) << 4) / (double)(SEEE_GLUT_X3 - SEEE_GLUT_X2) + 0.5);
        SEEE_GLUT_S3 = (S3 > 127) ? 127 : S3;
    }
    else{
        MINT32 S3 = (MINT32)((double)((SEEE_GLUT_Y3 - SEEE_GLUT_Y2) << 4) / (double)(SEEE_GLUT_X3 - SEEE_GLUT_X2) - 0.5);
        SEEE_GLUT_S3 = (S3 < -128) ? -128 : S3;
    }

    if((SEEE_GLUT_Y4 - SEEE_GLUT_Y3) > 0){
        MINT32 S4 = (MINT32)((double)((SEEE_GLUT_Y4 - SEEE_GLUT_Y3) << 4) / (double)(SEEE_GLUT_X4 - SEEE_GLUT_X3) + 0.5);
        SEEE_GLUT_S4 = (S4 > 127) ? 127 : S4;
    }
    else{
        MINT32 S4 = (MINT32)((double)((SEEE_GLUT_Y4 - SEEE_GLUT_Y3) << 4) / (double)(SEEE_GLUT_X4 - SEEE_GLUT_X3) - 0.5);
        SEEE_GLUT_S4 = (S4 < -128) ? -128 : S4;
    }

    if((SEEE_GLUT_Y5 - SEEE_GLUT_Y4) > 0){
        MINT32 S5 = (MINT32)((double)((SEEE_GLUT_Y5 - SEEE_GLUT_Y4) << 4) / (double)(255 - SEEE_GLUT_X4) + 0.5);
        SEEE_GLUT_S5 = (S5 > 127) ? 127 : S5;
    }
    else{
        MINT32 S5 = (MINT32)((double)((SEEE_GLUT_Y5 - SEEE_GLUT_Y4) << 4) / (double)(255 - SEEE_GLUT_X4) - 0.5);
        SEEE_GLUT_S5 = (S5 < -128) ? -128 : S5;
    }

    //  ====================================================================================================================================
    //  Set Smooth ISO setting
    //  ====================================================================================================================================
    rSmoothEE.blnd_ctrl_1.bits.SEEE_H1_DI_BLND_OFST         = SEEE_H1_DI_BLND_OFST;
    rSmoothEE.blnd_ctrl_1.bits.SEEE_H2_DI_BLND_OFST         = SEEE_H2_DI_BLND_OFST;
    rSmoothEE.blnd_ctrl_1.bits.SEEE_H3_DI_BLND_OFST         = SEEE_H3_DI_BLND_OFST;
    rSmoothEE.blnd_ctrl_2.bits.SEEE_H1_DI_BLND_SL           = SEEE_H1_DI_BLND_SL;
    rSmoothEE.blnd_ctrl_2.bits.SEEE_H2_DI_BLND_SL           = SEEE_H2_DI_BLND_SL;
    rSmoothEE.blnd_ctrl_2.bits.SEEE_H3_DI_BLND_SL           = SEEE_H3_DI_BLND_SL;
    rSmoothEE.blnd_ctrl_2.bits.SEEE_HX_ISO_BLND_RAT         = SEEE_HX_ISO_BLND_RAT;
    rSmoothEE.gn_ctrl.bits.SEEE_H1_GN                       = SEEE_H1_GN;
    rSmoothEE.gn_ctrl.bits.SEEE_H2_GN                       = SEEE_H2_GN;
    rSmoothEE.gn_ctrl.bits.SEEE_H3_GN                       = SEEE_H3_GN;
    rSmoothEE.gn_ctrl.bits.SEEE_H4_GN                       = SEEE_H4_GN;
    rSmoothEE.clip_ctrl_3.bits.SEEE_FLT_CORE_TH             = SEEE_FLT_CORE_TH;
    rSmoothEE.luma_ctrl_1.bits.SEEE_LUMA_MOD_Y0             = SEEE_LUMA_MOD_Y0;
    rSmoothEE.luma_ctrl_1.bits.SEEE_LUMA_MOD_Y1             = SEEE_LUMA_MOD_Y1;
    rSmoothEE.luma_ctrl_2.bits.SEEE_LUMA_MOD_Y2             = SEEE_LUMA_MOD_Y2;
    rSmoothEE.luma_ctrl_2.bits.SEEE_LUMA_MOD_Y3             = SEEE_LUMA_MOD_Y3;
    rSmoothEE.luma_ctrl_3.bits.SEEE_LUMA_MOD_Y4             = SEEE_LUMA_MOD_Y4;
    rSmoothEE.luma_ctrl_3.bits.SEEE_LUMA_MOD_Y5             = SEEE_LUMA_MOD_Y5;
    rSmoothEE.luma_ctrl_4.bits.SEEE_LUMA_MOD_Y6             = SEEE_LUMA_MOD_Y6;
    rSmoothEE.slnk_ctrl_1.bits.SEEE_SLNK_GN_Y1              = SEEE_SLNK_GN_Y1;
    rSmoothEE.slnk_ctrl_1.bits.SEEE_SLNK_GN_Y2              = SEEE_SLNK_GN_Y2;
    rSmoothEE.slnk_ctrl_2.bits.SEEE_RESP_SLNK_GN_RAT        = SEEE_RESP_SLNK_GN_RAT;
    rSmoothEE.glut_ctrl_1.bits.SEEE_GLUT_X1                 = SEEE_GLUT_X1;
    rSmoothEE.glut_ctrl_1.bits.SEEE_GLUT_S1                 = SEEE_GLUT_S1;
    rSmoothEE.glut_ctrl_1.bits.SEEE_GLUT_Y1                 = SEEE_GLUT_Y1;
    rSmoothEE.glut_ctrl_2.bits.SEEE_GLUT_X2                 = SEEE_GLUT_X2;
    rSmoothEE.glut_ctrl_2.bits.SEEE_GLUT_S2                 = SEEE_GLUT_S2;
    rSmoothEE.glut_ctrl_2.bits.SEEE_GLUT_Y2                 = SEEE_GLUT_Y2;
    rSmoothEE.glut_ctrl_3.bits.SEEE_GLUT_X3                 = SEEE_GLUT_X3;
    rSmoothEE.glut_ctrl_3.bits.SEEE_GLUT_S3                 = SEEE_GLUT_S3;
    rSmoothEE.glut_ctrl_3.bits.SEEE_GLUT_Y3                 = SEEE_GLUT_Y3;
    rSmoothEE.glut_ctrl_4.bits.SEEE_GLUT_X4                 = SEEE_GLUT_X4;
    rSmoothEE.glut_ctrl_4.bits.SEEE_GLUT_S4                 = SEEE_GLUT_S4;
    rSmoothEE.glut_ctrl_4.bits.SEEE_GLUT_Y4                 = SEEE_GLUT_Y4;
    rSmoothEE.glut_ctrl_5.bits.SEEE_GLUT_S5                 = SEEE_GLUT_S5;
    rSmoothEE.glut_ctrl_6.bits.SEEE_GLUT_TH_OVR             = SEEE_GLUT_TH_OVR;
    rSmoothEE.glut_ctrl_6.bits.SEEE_GLUT_TH_UND             = SEEE_GLUT_TH_UND;
    rSmoothEE.glut_ctrl_6.bits.SEEE_GLUT_TH_MIN             = SEEE_GLUT_TH_MIN;
    rSmoothEE.slnk_ctrl_2.bits.SEEE_GLUT_LINK_EN            = SEEE_GLUT_LINK_EN;
    rSmoothEE.slnk_ctrl_2.bits.SEEE_GLUT_SL_DEC_Y           = SEEE_GLUT_SL_DEC_Y;
    rSmoothEE.clip_ctrl_3.bits.SEEE_OVRSH_CLIP_STR          = SEEE_OVRSH_CLIP_STR;
    rSmoothEE.clip_ctrl_1.bits.SEEE_DOT_TH                  = SEEE_DOT_TH;
    rSmoothEE.clip_ctrl_1.bits.SEEE_DOT_REDUC_AMNT          = SEEE_DOT_REDUC_AMNT;
    rSmoothEE.clip_ctrl_2.bits.SEEE_RESP_CLIP               = SEEE_RESP_CLIP;
    rSmoothEE.clip_ctrl_2.bits.SEEE_RESP_CLIP_LUMA_SPC_TH   = SEEE_RESP_CLIP_LUMA_SPC_TH;
    rSmoothEE.clip_ctrl_2.bits.SEEE_RESP_CLIP_LUMA_LWB      = SEEE_RESP_CLIP_LUMA_LWB;
    rSmoothEE.clip_ctrl_2.bits.SEEE_RESP_CLIP_LUMA_UPB      = SEEE_RESP_CLIP_LUMA_UPB;

    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_H1_DI_BLND_OFST       = %d, %d, %d", rLowerEE.blnd_ctrl_1.bits.SEEE_H1_DI_BLND_OFST,          rSmoothEE.blnd_ctrl_1.bits.SEEE_H1_DI_BLND_OFST,        rUpperEE.blnd_ctrl_1.bits.SEEE_H1_DI_BLND_OFST);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_H2_DI_BLND_OFST       = %d, %d, %d", rLowerEE.blnd_ctrl_1.bits.SEEE_H2_DI_BLND_OFST,          rSmoothEE.blnd_ctrl_1.bits.SEEE_H2_DI_BLND_OFST,        rUpperEE.blnd_ctrl_1.bits.SEEE_H2_DI_BLND_OFST);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_H3_DI_BLND_OFST       = %d, %d, %d", rLowerEE.blnd_ctrl_1.bits.SEEE_H3_DI_BLND_OFST,          rSmoothEE.blnd_ctrl_1.bits.SEEE_H3_DI_BLND_OFST,        rUpperEE.blnd_ctrl_1.bits.SEEE_H3_DI_BLND_OFST);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_H1_DI_BLND_SL         = %d, %d, %d", rLowerEE.blnd_ctrl_2.bits.SEEE_H1_DI_BLND_SL,            rSmoothEE.blnd_ctrl_2.bits.SEEE_H1_DI_BLND_SL,          rUpperEE.blnd_ctrl_2.bits.SEEE_H1_DI_BLND_SL);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_H2_DI_BLND_SL         = %d, %d, %d", rLowerEE.blnd_ctrl_2.bits.SEEE_H2_DI_BLND_SL,            rSmoothEE.blnd_ctrl_2.bits.SEEE_H2_DI_BLND_SL,          rUpperEE.blnd_ctrl_2.bits.SEEE_H2_DI_BLND_SL);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_H3_DI_BLND_SL         = %d, %d, %d", rLowerEE.blnd_ctrl_2.bits.SEEE_H3_DI_BLND_SL,            rSmoothEE.blnd_ctrl_2.bits.SEEE_H3_DI_BLND_SL,          rUpperEE.blnd_ctrl_2.bits.SEEE_H3_DI_BLND_SL);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_HX_ISO_BLND_RAT       = %d, %d, %d", rLowerEE.blnd_ctrl_2.bits.SEEE_HX_ISO_BLND_RAT,          rSmoothEE.blnd_ctrl_2.bits.SEEE_HX_ISO_BLND_RAT,        rUpperEE.blnd_ctrl_2.bits.SEEE_HX_ISO_BLND_RAT);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_H1_GN                 = %d, %d, %d", rLowerEE.gn_ctrl.bits.SEEE_H1_GN,                        rSmoothEE.gn_ctrl.bits.SEEE_H1_GN,                      rUpperEE.gn_ctrl.bits.SEEE_H1_GN);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_H2_GN                 = %d, %d, %d", rLowerEE.gn_ctrl.bits.SEEE_H2_GN,                        rSmoothEE.gn_ctrl.bits.SEEE_H2_GN,                      rUpperEE.gn_ctrl.bits.SEEE_H2_GN);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_H3_GN                 = %d, %d, %d", rLowerEE.gn_ctrl.bits.SEEE_H3_GN,                        rSmoothEE.gn_ctrl.bits.SEEE_H3_GN,                      rUpperEE.gn_ctrl.bits.SEEE_H3_GN);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_H4_GN                 = %d, %d, %d", rLowerEE.gn_ctrl.bits.SEEE_H4_GN,                        rSmoothEE.gn_ctrl.bits.SEEE_H4_GN,                      rUpperEE.gn_ctrl.bits.SEEE_H4_GN);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_FLT_CORE_TH           = %d, %d, %d", rLowerEE.clip_ctrl_3.bits.SEEE_FLT_CORE_TH,              rSmoothEE.clip_ctrl_3.bits.SEEE_FLT_CORE_TH,            rUpperEE.clip_ctrl_3.bits.SEEE_FLT_CORE_TH);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_LUMA_MOD_Y0           = %d, %d, %d", rLowerEE.luma_ctrl_1.bits.SEEE_LUMA_MOD_Y0,              rSmoothEE.luma_ctrl_1.bits.SEEE_LUMA_MOD_Y0,            rUpperEE.luma_ctrl_1.bits.SEEE_LUMA_MOD_Y0);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_LUMA_MOD_Y1           = %d, %d, %d", rLowerEE.luma_ctrl_1.bits.SEEE_LUMA_MOD_Y1,              rSmoothEE.luma_ctrl_1.bits.SEEE_LUMA_MOD_Y1,            rUpperEE.luma_ctrl_1.bits.SEEE_LUMA_MOD_Y1);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_LUMA_MOD_Y2           = %d, %d, %d", rLowerEE.luma_ctrl_2.bits.SEEE_LUMA_MOD_Y2,              rSmoothEE.luma_ctrl_2.bits.SEEE_LUMA_MOD_Y2,            rUpperEE.luma_ctrl_2.bits.SEEE_LUMA_MOD_Y2);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_LUMA_MOD_Y3           = %d, %d, %d", rLowerEE.luma_ctrl_2.bits.SEEE_LUMA_MOD_Y3,              rSmoothEE.luma_ctrl_2.bits.SEEE_LUMA_MOD_Y3,            rUpperEE.luma_ctrl_2.bits.SEEE_LUMA_MOD_Y3);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_LUMA_MOD_Y4           = %d, %d, %d", rLowerEE.luma_ctrl_3.bits.SEEE_LUMA_MOD_Y4,              rSmoothEE.luma_ctrl_3.bits.SEEE_LUMA_MOD_Y4,            rUpperEE.luma_ctrl_3.bits.SEEE_LUMA_MOD_Y4);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_LUMA_MOD_Y5           = %d, %d, %d", rLowerEE.luma_ctrl_3.bits.SEEE_LUMA_MOD_Y5,              rSmoothEE.luma_ctrl_3.bits.SEEE_LUMA_MOD_Y5,            rUpperEE.luma_ctrl_3.bits.SEEE_LUMA_MOD_Y5);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_LUMA_MOD_Y6           = %d, %d, %d", rLowerEE.luma_ctrl_4.bits.SEEE_LUMA_MOD_Y6,              rSmoothEE.luma_ctrl_4.bits.SEEE_LUMA_MOD_Y6,            rUpperEE.luma_ctrl_4.bits.SEEE_LUMA_MOD_Y6);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_SLNK_GN_Y1            = %d, %d, %d", rLowerEE.slnk_ctrl_1.bits.SEEE_SLNK_GN_Y1,               rSmoothEE.slnk_ctrl_1.bits.SEEE_SLNK_GN_Y1,             rUpperEE.slnk_ctrl_1.bits.SEEE_SLNK_GN_Y1);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_SLNK_GN_Y2            = %d, %d, %d", rLowerEE.slnk_ctrl_1.bits.SEEE_SLNK_GN_Y2,               rSmoothEE.slnk_ctrl_1.bits.SEEE_SLNK_GN_Y2,             rUpperEE.slnk_ctrl_1.bits.SEEE_SLNK_GN_Y2);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_RESP_SLNK_GN_RAT      = %d, %d, %d", rLowerEE.slnk_ctrl_2.bits.SEEE_RESP_SLNK_GN_RAT,         rSmoothEE.slnk_ctrl_2.bits.SEEE_RESP_SLNK_GN_RAT,       rUpperEE.slnk_ctrl_2.bits.SEEE_RESP_SLNK_GN_RAT);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_GLUT_X1               = %d, %d, %d", rLowerEE.glut_ctrl_1.bits.SEEE_GLUT_X1,                  rSmoothEE.glut_ctrl_1.bits.SEEE_GLUT_X1,                rUpperEE.glut_ctrl_1.bits.SEEE_GLUT_X1);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_GLUT_S1               = %d, %d, %d", SEEE_GLUT_S1pre,                                         SEEE_GLUT_S1,                                           SEEE_GLUT_S1pos);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_GLUT_Y1               = %d, %d, %d", rLowerEE.glut_ctrl_1.bits.SEEE_GLUT_Y1,                  rSmoothEE.glut_ctrl_1.bits.SEEE_GLUT_Y1,                rUpperEE.glut_ctrl_1.bits.SEEE_GLUT_Y1);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_GLUT_X2               = %d, %d, %d", rLowerEE.glut_ctrl_2.bits.SEEE_GLUT_X2,                  rSmoothEE.glut_ctrl_2.bits.SEEE_GLUT_X2,                rUpperEE.glut_ctrl_2.bits.SEEE_GLUT_X2);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_GLUT_S2               = %d, %d, %d", SEEE_GLUT_S2pre,                                         SEEE_GLUT_S2,                                           SEEE_GLUT_S2pos);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_GLUT_Y2               = %d, %d, %d", rLowerEE.glut_ctrl_2.bits.SEEE_GLUT_Y2,                  rSmoothEE.glut_ctrl_2.bits.SEEE_GLUT_Y2,                rUpperEE.glut_ctrl_2.bits.SEEE_GLUT_Y2);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_GLUT_X3               = %d, %d, %d", rLowerEE.glut_ctrl_3.bits.SEEE_GLUT_X3,                  rSmoothEE.glut_ctrl_3.bits.SEEE_GLUT_X3,                rUpperEE.glut_ctrl_3.bits.SEEE_GLUT_X3);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_GLUT_S3               = %d, %d, %d", SEEE_GLUT_S3pre,                                         SEEE_GLUT_S3,                                           SEEE_GLUT_S3pos);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_GLUT_Y3               = %d, %d, %d", rLowerEE.glut_ctrl_3.bits.SEEE_GLUT_Y3,                  rSmoothEE.glut_ctrl_3.bits.SEEE_GLUT_Y3,                rUpperEE.glut_ctrl_3.bits.SEEE_GLUT_Y3);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_GLUT_X4               = %d, %d, %d", rLowerEE.glut_ctrl_4.bits.SEEE_GLUT_X4,                  rSmoothEE.glut_ctrl_4.bits.SEEE_GLUT_X4,                rUpperEE.glut_ctrl_4.bits.SEEE_GLUT_X4);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_GLUT_S4               = %d, %d, %d", SEEE_GLUT_S4pre,                                         SEEE_GLUT_S4,                                           SEEE_GLUT_S4pos);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_GLUT_Y4               = %d, %d, %d", rLowerEE.glut_ctrl_4.bits.SEEE_GLUT_Y4,                  rSmoothEE.glut_ctrl_4.bits.SEEE_GLUT_Y4,                rUpperEE.glut_ctrl_4.bits.SEEE_GLUT_Y4);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_GLUT_S5               = %d, %d, %d", SEEE_GLUT_S5pre,                                         SEEE_GLUT_S5,                                           SEEE_GLUT_S5pos);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_GLUT_TH_OVR           = %d, %d, %d", rLowerEE.glut_ctrl_6.bits.SEEE_GLUT_TH_OVR,              rSmoothEE.glut_ctrl_6.bits.SEEE_GLUT_TH_OVR,            rUpperEE.glut_ctrl_6.bits.SEEE_GLUT_TH_OVR);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_GLUT_TH_UND           = %d, %d, %d", rLowerEE.glut_ctrl_6.bits.SEEE_GLUT_TH_UND,              rSmoothEE.glut_ctrl_6.bits.SEEE_GLUT_TH_UND,            rUpperEE.glut_ctrl_6.bits.SEEE_GLUT_TH_UND);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_GLUT_TH_MIN           = %d, %d, %d", rLowerEE.glut_ctrl_6.bits.SEEE_GLUT_TH_MIN,              rSmoothEE.glut_ctrl_6.bits.SEEE_GLUT_TH_MIN,            rUpperEE.glut_ctrl_6.bits.SEEE_GLUT_TH_MIN);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_GLUT_LINK_EN          = %d, %d, %d", rLowerEE.slnk_ctrl_2.bits.SEEE_GLUT_LINK_EN,             rSmoothEE.slnk_ctrl_2.bits.SEEE_GLUT_LINK_EN,           rUpperEE.slnk_ctrl_2.bits.SEEE_GLUT_LINK_EN);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_GLUT_SL_DEC_Y         = %d, %d, %d", rLowerEE.slnk_ctrl_2.bits.SEEE_GLUT_SL_DEC_Y,            rSmoothEE.slnk_ctrl_2.bits.SEEE_GLUT_SL_DEC_Y,          rUpperEE.slnk_ctrl_2.bits.SEEE_GLUT_SL_DEC_Y);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_OVRSH_CLIP_STR        = %d, %d, %d", rLowerEE.clip_ctrl_3.bits.SEEE_OVRSH_CLIP_STR,           rSmoothEE.clip_ctrl_3.bits.SEEE_OVRSH_CLIP_STR,         rUpperEE.clip_ctrl_3.bits.SEEE_OVRSH_CLIP_STR);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_DOT_TH                = %d, %d, %d", rLowerEE.clip_ctrl_1.bits.SEEE_DOT_TH,                   rSmoothEE.clip_ctrl_1.bits.SEEE_DOT_TH,                 rUpperEE.clip_ctrl_1.bits.SEEE_DOT_TH);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_DOT_REDUC_AMNT        = %d, %d, %d", rLowerEE.clip_ctrl_1.bits.SEEE_DOT_REDUC_AMNT,           rSmoothEE.clip_ctrl_1.bits.SEEE_DOT_REDUC_AMNT,         rUpperEE.clip_ctrl_1.bits.SEEE_DOT_REDUC_AMNT);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_RESP_CLIP             = %d, %d, %d", rLowerEE.clip_ctrl_2.bits.SEEE_RESP_CLIP,                rSmoothEE.clip_ctrl_2.bits.SEEE_RESP_CLIP,              rUpperEE.clip_ctrl_2.bits.SEEE_RESP_CLIP);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_RESP_CLIP_LUMA_SPC_TH = %d, %d, %d", rLowerEE.clip_ctrl_2.bits.SEEE_RESP_CLIP_LUMA_SPC_TH,    rSmoothEE.clip_ctrl_2.bits.SEEE_RESP_CLIP_LUMA_SPC_TH,  rUpperEE.clip_ctrl_2.bits.SEEE_RESP_CLIP_LUMA_SPC_TH);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_RESP_CLIP_LUMA_LWB    = %d, %d, %d", rLowerEE.clip_ctrl_2.bits.SEEE_RESP_CLIP_LUMA_LWB,       rSmoothEE.clip_ctrl_2.bits.SEEE_RESP_CLIP_LUMA_LWB,     rUpperEE.clip_ctrl_2.bits.SEEE_RESP_CLIP_LUMA_LWB);
    MY_LOG_IF(bSmoothEEDebug, "[L,S,U]EE.SEEE_RESP_CLIP_LUMA_UPB    = %d, %d, %d", rLowerEE.clip_ctrl_2.bits.SEEE_RESP_CLIP_LUMA_UPB,       rSmoothEE.clip_ctrl_2.bits.SEEE_RESP_CLIP_LUMA_UPB,     rUpperEE.clip_ctrl_2.bits.SEEE_RESP_CLIP_LUMA_UPB);
}

//===========================================================================

#define CLIP_NR3D(a,b,c)   ( (a<b)?  b : ((a>c)? c: a) )

MINT32 InterParam_NR3D(MINT32 ISO, MINT32 ISO_lo, MINT32 ISO_up, MINT32 PARAM_lo, MINT32 PARAM_up, MINT32 Method)
{
    MINT32 PARAM_ou;

    if(ISO_lo==ISO_up) {
        PARAM_ou = ((PARAM_lo + PARAM_up + 1) / 2);
    }
    else {
        switch(Method){
        case 1:
            PARAM_ou = (abs(ISO - ISO_lo) > abs(ISO_up - ISO)) ? PARAM_up : PARAM_lo;
            break;
        case 0:
        default:
            PARAM_ou = (PARAM_lo*(ISO_up - ISO)+PARAM_up*(ISO - ISO_lo)+((ISO_up - ISO_lo)/2))/(ISO_up - ISO_lo);
            break;
        }
    }

    return PARAM_ou;



}

// ===========================================================================================
#if 0

MVOID SmoothNR3D(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_NR3D_T const& rUpperNR3D,
                ISP_NVRAM_NR3D_T const& rLowerNR3D,
                ISP_NVRAM_NR3D_T& rSmoothNR3D)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.nr3d.bm.enable", value, "0");
    MBOOL bSmoothNR3DDebug = atoi(value);

    static MINT32 bmCount = 0;
    bmCount++;

    if (bSmoothNR3DDebug && bmCount % 33 == 0)
    {
        CAM_LOGW_IF(bSmoothNR3DDebug,"[%s()] u4RealISO: %d, u4UpperISO = %d, u4LowerISO = %d\n", __FUNCTION__, u4RealISO, u4UpperISO, u4LowerISO);
    }

    // ================
    // upper
    MINT32 NR3D_ALPHA_Y_X1_up = rUpperNR3D.ay_con1.bits.NR3D_ALPHA_Y_X1;
    MINT32 NR3D_ALPHA_Y_X2_up = rUpperNR3D.ay_con1.bits.NR3D_ALPHA_Y_X2;
    MINT32 NR3D_ALPHA_Y_X3_up = rUpperNR3D.ay_con1.bits.NR3D_ALPHA_Y_X3;
    MINT32 NR3D_ALPHA_Y_X4_up = rUpperNR3D.ay_con1.bits.NR3D_ALPHA_Y_X4;

    MINT32 NR3D_ALPHA_Y_Y0_up = rUpperNR3D.ay_con2.bits.NR3D_ALPHA_Y_Y0;
    MINT32 NR3D_ALPHA_Y_Y1_up = rUpperNR3D.ay_con2.bits.NR3D_ALPHA_Y_Y1;
    MINT32 NR3D_ALPHA_Y_Y2_up = rUpperNR3D.ay_con2.bits.NR3D_ALPHA_Y_Y2;
    MINT32 NR3D_ALPHA_Y_Y3_up = rUpperNR3D.ay_con2.bits.NR3D_ALPHA_Y_Y3;

    MINT32 NR3D_ALPHA_Y_Y4_up = rUpperNR3D.ay_con3.bits.NR3D_ALPHA_Y_Y4;

    MINT32 NR3D_ALPHA_C_X1_up = rUpperNR3D.ac_con1.bits.NR3D_ALPHA_C_X1;
    MINT32 NR3D_ALPHA_C_X2_up = rUpperNR3D.ac_con1.bits.NR3D_ALPHA_C_X2;
    MINT32 NR3D_ALPHA_C_X3_up = rUpperNR3D.ac_con1.bits.NR3D_ALPHA_C_X3;
    MINT32 NR3D_ALPHA_C_X4_up = rUpperNR3D.ac_con1.bits.NR3D_ALPHA_C_X4;

    MINT32 NR3D_ALPHA_C_Y0_up = rUpperNR3D.ac_con2.bits.NR3D_ALPHA_C_Y0;
    MINT32 NR3D_ALPHA_C_Y1_up = rUpperNR3D.ac_con2.bits.NR3D_ALPHA_C_Y1;
    MINT32 NR3D_ALPHA_C_Y2_up = rUpperNR3D.ac_con2.bits.NR3D_ALPHA_C_Y2;
    MINT32 NR3D_ALPHA_C_Y3_up = rUpperNR3D.ac_con2.bits.NR3D_ALPHA_C_Y3;

    MINT32 NR3D_ALPHA_C_Y4_up = rUpperNR3D.ac_con3.bits.NR3D_ALPHA_C_Y4;

    MINT32 NR3D_BETA1_Y_X1_up = rUpperNR3D.b1y_con1.bits.NR3D_BETA1_Y_X1;
    MINT32 NR3D_BETA1_Y_X2_up = rUpperNR3D.b1y_con1.bits.NR3D_BETA1_Y_X2;
    MINT32 NR3D_BETA1_Y_X3_up = rUpperNR3D.b1y_con1.bits.NR3D_BETA1_Y_X3;
    MINT32 NR3D_BETA1_Y_X4_up = rUpperNR3D.b1y_con1.bits.NR3D_BETA1_Y_X4;

    MINT32 NR3D_BETA1_Y_Y1_up = rUpperNR3D.b1y_con2.bits.NR3D_BETA1_Y_Y1;
    MINT32 NR3D_BETA1_Y_Y2_up = rUpperNR3D.b1y_con2.bits.NR3D_BETA1_Y_Y2;
    MINT32 NR3D_BETA1_Y_Y3_up = rUpperNR3D.b1y_con2.bits.NR3D_BETA1_Y_Y3;
    MINT32 NR3D_BETA1_Y_Y4_up = rUpperNR3D.b1y_con2.bits.NR3D_BETA1_Y_Y4;

    MINT32 NR3D_BETA1_C_X1_up = rUpperNR3D.b1c_con1.bits.NR3D_BETA1_C_X1;
    MINT32 NR3D_BETA1_C_X2_up = rUpperNR3D.b1c_con1.bits.NR3D_BETA1_C_X2;
    MINT32 NR3D_BETA1_C_X3_up = rUpperNR3D.b1c_con1.bits.NR3D_BETA1_C_X3;
    MINT32 NR3D_BETA1_C_X4_up = rUpperNR3D.b1c_con1.bits.NR3D_BETA1_C_X4;

    MINT32 NR3D_BETA1_C_Y1_up = rUpperNR3D.b1c_con2.bits.NR3D_BETA1_C_Y1;
    MINT32 NR3D_BETA1_C_Y2_up = rUpperNR3D.b1c_con2.bits.NR3D_BETA1_C_Y2;
    MINT32 NR3D_BETA1_C_Y3_up = rUpperNR3D.b1c_con2.bits.NR3D_BETA1_C_Y3;
    MINT32 NR3D_BETA1_C_Y4_up = rUpperNR3D.b1c_con2.bits.NR3D_BETA1_C_Y4;

    MINT32 NR3D_LSCG1_Y_X1_up = rUpperNR3D.lscg1_xcon.bits.NR3D_LSCG1_Y_X1;
    MINT32 NR3D_LSCG1_Y_X2_up = rUpperNR3D.lscg1_xcon.bits.NR3D_LSCG1_Y_X2;
    MINT32 NR3D_LSCG1_Y_X3_up = rUpperNR3D.lscg1_xcon.bits.NR3D_LSCG1_Y_X3;
    MINT32 NR3D_LSCG1_Y_X4_up = rUpperNR3D.lscg1_xcon.bits.NR3D_LSCG1_Y_X4;

    MINT32 NR3D_LSCG1_Y_Y0_up = rUpperNR3D.lscg1_ycon.bits.NR3D_LSCG1_Y_Y0;
    MINT32 NR3D_LSCG1_Y_Y1_up = rUpperNR3D.lscg1_ycon.bits.NR3D_LSCG1_Y_Y1;
    MINT32 NR3D_LSCG1_Y_Y2_up = rUpperNR3D.lscg1_ycon.bits.NR3D_LSCG1_Y_Y2;
    MINT32 NR3D_LSCG1_Y_Y3_up = rUpperNR3D.lscg1_ycon.bits.NR3D_LSCG1_Y_Y3;

    MINT32 NR3D_LSCG1_Y_Y4_up = rUpperNR3D.lscg1_con.bits.NR3D_LSCG1_Y_Y4;
    MINT32 NR3D_LSCG1_DLT_GAIN_Y_up = rUpperNR3D.lscg1_con.bits.NR3D_LSCG1_DLT_GAIN_Y;
    MINT32 NR3D_LSCG1_DLT_GAIN_C_up = rUpperNR3D.lscg1_con.bits.NR3D_LSCG1_DLT_GAIN_C;

    MINT32 NR3D_MPNL_Y_Y0_up = rUpperNR3D.nly_con1.bits.NR3D_MPNL_Y_Y0;
    MINT32 NR3D_EPNL1_MAX_Y_up = rUpperNR3D.nly_con1.bits.NR3D_EPNL1_MAX_Y;
    MINT32 NR3D_EPNL1_COR_Y_up = rUpperNR3D.nly_con1.bits.NR3D_EPNL1_COR_Y;
    MINT32 NR3D_EPNL2_MAX_Y_up = rUpperNR3D.nly_con1.bits.NR3D_EPNL2_MAX_Y;
    MINT32 NR3D_EPNL2_COR_Y_up = rUpperNR3D.nly_con1.bits.NR3D_EPNL2_COR_Y;
    MINT32 NR3D_MPNL1_MAX_Y_up = rUpperNR3D.nly_con1.bits.NR3D_MPNL1_MAX_Y;
    MINT32 NR3D_MPNL1_COR_Y_up = rUpperNR3D.nly_con1.bits.NR3D_MPNL1_COR_Y;

    MINT32 NR3D_PVAI1_GAIN_Y_up = rUpperNR3D.nly_con2.bits.NR3D_PVAI1_GAIN_Y;
    MINT32 NR3D_MPNL2_MAX_Y_up = rUpperNR3D.nly_con2.bits.NR3D_MPNL2_MAX_Y;
    MINT32 NR3D_MPNL2_COR_Y_up = rUpperNR3D.nly_con2.bits.NR3D_MPNL2_COR_Y;
    MINT32 NR3D_MPNL3_MAX_Y_up = rUpperNR3D.nly_con2.bits.NR3D_MPNL3_MAX_Y;
    MINT32 NR3D_MPNL4_MAX_Y_up = rUpperNR3D.nly_con2.bits.NR3D_MPNL4_MAX_Y;
    MINT32 NR3D_PVAR1_MIN_Y_up = rUpperNR3D.nly_con2.bits.NR3D_PVAR1_MIN_Y;
    MINT32 NR3D_PVAR1_MAX_Y_up = rUpperNR3D.nly_con2.bits.NR3D_PVAR1_MAX_Y;

    MINT32 NR3D_MTLVL_COR_Y_up = rUpperNR3D.mly_con.bits.NR3D_MTLVL_COR_Y;
    MINT32 NR3D_MTLVL_GAIN_Y_up = rUpperNR3D.mly_con.bits.NR3D_MTLVL_GAIN_Y;
    MINT32 NR3D_MTLVL_VASEL_Y_up = rUpperNR3D.mly_con.bits.NR3D_MTLVL_VASEL_Y;
    MINT32 NR3D_MTLVL_DFSEL_Y_up = rUpperNR3D.mly_con.bits.NR3D_MTLVL_DFSEL_Y;
    MINT32 NR3D_DDIF2_GPSEL_Y_up = rUpperNR3D.mly_con.bits.NR3D_DDIF2_GPSEL_Y;

    MINT32 NR3D_MPNL_C_Y0_up = rUpperNR3D.nlc_con1.bits.NR3D_MPNL_C_Y0;
    MINT32 NR3D_EPNL1_MAX_C_up = rUpperNR3D.nlc_con1.bits.NR3D_EPNL1_MAX_C;
    MINT32 NR3D_EPNL1_COR_C_up = rUpperNR3D.nlc_con1.bits.NR3D_EPNL1_COR_C;
    MINT32 NR3D_EPNL2_MAX_C_up = rUpperNR3D.nlc_con1.bits.NR3D_EPNL2_MAX_C;
    MINT32 NR3D_EPNL2_COR_C_up = rUpperNR3D.nlc_con1.bits.NR3D_EPNL2_COR_C;
    MINT32 NR3D_MPNL1_MAX_C_up = rUpperNR3D.nlc_con1.bits.NR3D_MPNL1_MAX_C;
    MINT32 NR3D_MPNL1_COR_C_up = rUpperNR3D.nlc_con1.bits.NR3D_MPNL1_COR_C;

    MINT32 NR3D_MPNL2_MAX_C_up = rUpperNR3D.nlc_con2.bits.NR3D_MPNL2_MAX_C;
    MINT32 NR3D_MPNL2_COR_C_up = rUpperNR3D.nlc_con2.bits.NR3D_MPNL2_COR_C;
    MINT32 NR3D_MPNL3_MAX_C_up = rUpperNR3D.nlc_con2.bits.NR3D_MPNL3_MAX_C;
    MINT32 NR3D_MPNL4_MAX_C_up = rUpperNR3D.nlc_con2.bits.NR3D_MPNL4_MAX_C;
    MINT32 NR3D_PVAI1_GAIN_C_up = rUpperNR3D.nlc_con2.bits.NR3D_PVAI1_GAIN_C;
    MINT32 NR3D_PVAR1_MIN_C_up = rUpperNR3D.nlc_con2.bits.NR3D_PVAR1_MIN_C;
    MINT32 NR3D_PVAR1_MAX_C_up = rUpperNR3D.nlc_con2.bits.NR3D_PVAR1_MAX_C;

    MINT32 NR3D_MTLVL_COR_C_up = rUpperNR3D.mlc_con.bits.NR3D_MTLVL_COR_C;
    MINT32 NR3D_MTLVL_GAIN_C_up = rUpperNR3D.mlc_con.bits.NR3D_MTLVL_GAIN_C;

    MINT32 NR3D_SVLVL_M1_LS1_Y_up = rUpperNR3D.sly_con.bits.NR3D_SVLVL_M1_LS1_Y;
    MINT32 NR3D_SVLVL_M2_LS1_Y_up = rUpperNR3D.sly_con.bits.NR3D_SVLVL_M2_LS1_Y;
    MINT32 NR3D_STLVL_M3_LS1_Y_up = rUpperNR3D.sly_con.bits.NR3D_STLVL_M3_LS1_Y;
    MINT32 NR3D_SVLVL_M1_ADD_Y_up = rUpperNR3D.sly_con.bits.NR3D_SVLVL_M1_ADD_Y;
    MINT32 NR3D_SVLVL_M1_MAX_Y_up = rUpperNR3D.sly_con.bits.NR3D_SVLVL_M1_MAX_Y;
    MINT32 NR3D_SVLVL_M2_ADD_Y_up = rUpperNR3D.sly_con.bits.NR3D_SVLVL_M2_ADD_Y;
    MINT32 NR3D_SVLVL_M2_MAX_Y_up = rUpperNR3D.sly_con.bits.NR3D_SVLVL_M2_MAX_Y;
    MINT32 NR3D_STLVL_M3_ADD_Y_up = rUpperNR3D.sly_con.bits.NR3D_STLVL_M3_ADD_Y;
    MINT32 NR3D_STLVL_M3_MAX_Y_up = rUpperNR3D.sly_con.bits.NR3D_STLVL_M3_MAX_Y;

    MINT32 NR3D_SVLVL_M1_LS1_C_up = rUpperNR3D.slc_con.bits.NR3D_SVLVL_M1_LS1_C;
    MINT32 NR3D_SVLVL_M2_LS1_C_up = rUpperNR3D.slc_con.bits.NR3D_SVLVL_M2_LS1_C;
    MINT32 NR3D_STLVL_M3_LS1_C_up = rUpperNR3D.slc_con.bits.NR3D_STLVL_M3_LS1_C;
    MINT32 NR3D_SVLVL_M1_ADD_C_up = rUpperNR3D.slc_con.bits.NR3D_SVLVL_M1_ADD_C;
    MINT32 NR3D_SVLVL_M1_MAX_C_up = rUpperNR3D.slc_con.bits.NR3D_SVLVL_M1_MAX_C;
    MINT32 NR3D_SVLVL_M2_ADD_C_up = rUpperNR3D.slc_con.bits.NR3D_SVLVL_M2_ADD_C;
    MINT32 NR3D_SVLVL_M2_MAX_C_up = rUpperNR3D.slc_con.bits.NR3D_SVLVL_M2_MAX_C;
    MINT32 NR3D_STLVL_M3_ADD_C_up = rUpperNR3D.slc_con.bits.NR3D_STLVL_M3_ADD_C;
    MINT32 NR3D_STLVL_M3_MAX_C_up = rUpperNR3D.slc_con.bits.NR3D_STLVL_M3_MAX_C;

    MINT32 NR3D_DDIF_GAIN_Y_up = rUpperNR3D.difg_con.bits.NR3D_DDIF_GAIN_Y;
    MINT32 NR3D_DDIF_GAIN_C_up = rUpperNR3D.difg_con.bits.NR3D_DDIF_GAIN_C;
    MINT32 NR3D_BDIF_GAIN_Y_up = rUpperNR3D.difg_con.bits.NR3D_BDIF_GAIN_Y;
    MINT32 NR3D_BDIF_GAIN_C_up = rUpperNR3D.difg_con.bits.NR3D_BDIF_GAIN_C;

    MINT32 NR3D_YCJC_TH1_Y_up = rUpperNR3D.ycjc_con1.bits.NR3D_YCJC_TH1_Y;
    MINT32 NR3D_YCJC_TH3_Y_up = rUpperNR3D.ycjc_con1.bits.NR3D_YCJC_TH3_Y;
    MINT32 NR3D_YCJC_TH1_C_up = rUpperNR3D.ycjc_con1.bits.NR3D_YCJC_TH1_C;
    MINT32 NR3D_YCJC_TH2_C_up = rUpperNR3D.ycjc_con1.bits.NR3D_YCJC_TH2_C;

    MINT32 NR3D_YCJC_TH3_C_up = rUpperNR3D.ycjc_con2.bits.NR3D_YCJC_TH3_C;
    MINT32 NR3D_YCJC_GAIN_Y_up = rUpperNR3D.ycjc_con2.bits.NR3D_YCJC_GAIN_Y;
    MINT32 NR3D_YCJC_GAIN_C_up = rUpperNR3D.ycjc_con2.bits.NR3D_YCJC_GAIN_C;
    MINT32 NR3D_YCJC_RS_Y_up = rUpperNR3D.ycjc_con2.bits.NR3D_YCJC_RS_Y;
    MINT32 NR3D_YCJC_RS_C_up = rUpperNR3D.ycjc_con2.bits.NR3D_YCJC_RS_C;


    // ================
    // lower

    MINT32 NR3D_ALPHA_Y_X1_lo = rLowerNR3D.ay_con1.bits.NR3D_ALPHA_Y_X1;
    MINT32 NR3D_ALPHA_Y_X2_lo = rLowerNR3D.ay_con1.bits.NR3D_ALPHA_Y_X2;
    MINT32 NR3D_ALPHA_Y_X3_lo = rLowerNR3D.ay_con1.bits.NR3D_ALPHA_Y_X3;
    MINT32 NR3D_ALPHA_Y_X4_lo = rLowerNR3D.ay_con1.bits.NR3D_ALPHA_Y_X4;

    MINT32 NR3D_ALPHA_Y_Y0_lo = rLowerNR3D.ay_con2.bits.NR3D_ALPHA_Y_Y0;
    MINT32 NR3D_ALPHA_Y_Y1_lo = rLowerNR3D.ay_con2.bits.NR3D_ALPHA_Y_Y1;
    MINT32 NR3D_ALPHA_Y_Y2_lo = rLowerNR3D.ay_con2.bits.NR3D_ALPHA_Y_Y2;
    MINT32 NR3D_ALPHA_Y_Y3_lo = rLowerNR3D.ay_con2.bits.NR3D_ALPHA_Y_Y3;

    MINT32 NR3D_ALPHA_Y_Y4_lo = rLowerNR3D.ay_con3.bits.NR3D_ALPHA_Y_Y4;

    MINT32 NR3D_ALPHA_C_X1_lo = rLowerNR3D.ac_con1.bits.NR3D_ALPHA_C_X1;
    MINT32 NR3D_ALPHA_C_X2_lo = rLowerNR3D.ac_con1.bits.NR3D_ALPHA_C_X2;
    MINT32 NR3D_ALPHA_C_X3_lo = rLowerNR3D.ac_con1.bits.NR3D_ALPHA_C_X3;
    MINT32 NR3D_ALPHA_C_X4_lo = rLowerNR3D.ac_con1.bits.NR3D_ALPHA_C_X4;

    MINT32 NR3D_ALPHA_C_Y0_lo = rLowerNR3D.ac_con2.bits.NR3D_ALPHA_C_Y0;
    MINT32 NR3D_ALPHA_C_Y1_lo = rLowerNR3D.ac_con2.bits.NR3D_ALPHA_C_Y1;
    MINT32 NR3D_ALPHA_C_Y2_lo = rLowerNR3D.ac_con2.bits.NR3D_ALPHA_C_Y2;
    MINT32 NR3D_ALPHA_C_Y3_lo = rLowerNR3D.ac_con2.bits.NR3D_ALPHA_C_Y3;

    MINT32 NR3D_ALPHA_C_Y4_lo = rLowerNR3D.ac_con3.bits.NR3D_ALPHA_C_Y4;

    MINT32 NR3D_BETA1_Y_X1_lo = rLowerNR3D.b1y_con1.bits.NR3D_BETA1_Y_X1;
    MINT32 NR3D_BETA1_Y_X2_lo = rLowerNR3D.b1y_con1.bits.NR3D_BETA1_Y_X2;
    MINT32 NR3D_BETA1_Y_X3_lo = rLowerNR3D.b1y_con1.bits.NR3D_BETA1_Y_X3;
    MINT32 NR3D_BETA1_Y_X4_lo = rLowerNR3D.b1y_con1.bits.NR3D_BETA1_Y_X4;

    MINT32 NR3D_BETA1_Y_Y1_lo = rLowerNR3D.b1y_con2.bits.NR3D_BETA1_Y_Y1;
    MINT32 NR3D_BETA1_Y_Y2_lo = rLowerNR3D.b1y_con2.bits.NR3D_BETA1_Y_Y2;
    MINT32 NR3D_BETA1_Y_Y3_lo = rLowerNR3D.b1y_con2.bits.NR3D_BETA1_Y_Y3;
    MINT32 NR3D_BETA1_Y_Y4_lo = rLowerNR3D.b1y_con2.bits.NR3D_BETA1_Y_Y4;

    MINT32 NR3D_BETA1_C_X1_lo = rLowerNR3D.b1c_con1.bits.NR3D_BETA1_C_X1;
    MINT32 NR3D_BETA1_C_X2_lo = rLowerNR3D.b1c_con1.bits.NR3D_BETA1_C_X2;
    MINT32 NR3D_BETA1_C_X3_lo = rLowerNR3D.b1c_con1.bits.NR3D_BETA1_C_X3;
    MINT32 NR3D_BETA1_C_X4_lo = rLowerNR3D.b1c_con1.bits.NR3D_BETA1_C_X4;

    MINT32 NR3D_BETA1_C_Y1_lo = rLowerNR3D.b1c_con2.bits.NR3D_BETA1_C_Y1;
    MINT32 NR3D_BETA1_C_Y2_lo = rLowerNR3D.b1c_con2.bits.NR3D_BETA1_C_Y2;
    MINT32 NR3D_BETA1_C_Y3_lo = rLowerNR3D.b1c_con2.bits.NR3D_BETA1_C_Y3;
    MINT32 NR3D_BETA1_C_Y4_lo = rLowerNR3D.b1c_con2.bits.NR3D_BETA1_C_Y4;

    MINT32 NR3D_LSCG1_Y_X1_lo = rLowerNR3D.lscg1_xcon.bits.NR3D_LSCG1_Y_X1;
    MINT32 NR3D_LSCG1_Y_X2_lo = rLowerNR3D.lscg1_xcon.bits.NR3D_LSCG1_Y_X2;
    MINT32 NR3D_LSCG1_Y_X3_lo = rLowerNR3D.lscg1_xcon.bits.NR3D_LSCG1_Y_X3;
    MINT32 NR3D_LSCG1_Y_X4_lo = rLowerNR3D.lscg1_xcon.bits.NR3D_LSCG1_Y_X4;

    MINT32 NR3D_LSCG1_Y_Y0_lo = rLowerNR3D.lscg1_ycon.bits.NR3D_LSCG1_Y_Y0;
    MINT32 NR3D_LSCG1_Y_Y1_lo = rLowerNR3D.lscg1_ycon.bits.NR3D_LSCG1_Y_Y1;
    MINT32 NR3D_LSCG1_Y_Y2_lo = rLowerNR3D.lscg1_ycon.bits.NR3D_LSCG1_Y_Y2;
    MINT32 NR3D_LSCG1_Y_Y3_lo = rLowerNR3D.lscg1_ycon.bits.NR3D_LSCG1_Y_Y3;

    MINT32 NR3D_LSCG1_Y_Y4_lo = rLowerNR3D.lscg1_con.bits.NR3D_LSCG1_Y_Y4;
    MINT32 NR3D_LSCG1_DLT_GAIN_Y_lo = rLowerNR3D.lscg1_con.bits.NR3D_LSCG1_DLT_GAIN_Y;
    MINT32 NR3D_LSCG1_DLT_GAIN_C_lo = rLowerNR3D.lscg1_con.bits.NR3D_LSCG1_DLT_GAIN_C;

    MINT32 NR3D_MPNL_Y_Y0_lo = rLowerNR3D.nly_con1.bits.NR3D_MPNL_Y_Y0;
    MINT32 NR3D_EPNL1_MAX_Y_lo = rLowerNR3D.nly_con1.bits.NR3D_EPNL1_MAX_Y;
    MINT32 NR3D_EPNL1_COR_Y_lo = rLowerNR3D.nly_con1.bits.NR3D_EPNL1_COR_Y;
    MINT32 NR3D_EPNL2_MAX_Y_lo = rLowerNR3D.nly_con1.bits.NR3D_EPNL2_MAX_Y;
    MINT32 NR3D_EPNL2_COR_Y_lo = rLowerNR3D.nly_con1.bits.NR3D_EPNL2_COR_Y;
    MINT32 NR3D_MPNL1_MAX_Y_lo = rLowerNR3D.nly_con1.bits.NR3D_MPNL1_MAX_Y;
    MINT32 NR3D_MPNL1_COR_Y_lo = rLowerNR3D.nly_con1.bits.NR3D_MPNL1_COR_Y;

    MINT32 NR3D_MPNL2_MAX_Y_lo = rLowerNR3D.nly_con2.bits.NR3D_MPNL2_MAX_Y;
    MINT32 NR3D_MPNL2_COR_Y_lo = rLowerNR3D.nly_con2.bits.NR3D_MPNL2_COR_Y;
    MINT32 NR3D_MPNL3_MAX_Y_lo = rLowerNR3D.nly_con2.bits.NR3D_MPNL3_MAX_Y;
    MINT32 NR3D_MPNL4_MAX_Y_lo = rLowerNR3D.nly_con2.bits.NR3D_MPNL4_MAX_Y;
    MINT32 NR3D_PVAI1_GAIN_Y_lo = rLowerNR3D.nly_con2.bits.NR3D_PVAI1_GAIN_Y;
    MINT32 NR3D_PVAR1_MIN_Y_lo = rLowerNR3D.nly_con2.bits.NR3D_PVAR1_MIN_Y;
    MINT32 NR3D_PVAR1_MAX_Y_lo = rLowerNR3D.nly_con2.bits.NR3D_PVAR1_MAX_Y;

    MINT32 NR3D_MTLVL_COR_Y_lo = rLowerNR3D.mly_con.bits.NR3D_MTLVL_COR_Y;
    MINT32 NR3D_MTLVL_GAIN_Y_lo = rLowerNR3D.mly_con.bits.NR3D_MTLVL_GAIN_Y;
    MINT32 NR3D_MTLVL_VASEL_Y_lo = rLowerNR3D.mly_con.bits.NR3D_MTLVL_VASEL_Y;
    MINT32 NR3D_MTLVL_DFSEL_Y_lo = rLowerNR3D.mly_con.bits.NR3D_MTLVL_DFSEL_Y;
    MINT32 NR3D_DDIF2_GPSEL_Y_lo = rLowerNR3D.mly_con.bits.NR3D_DDIF2_GPSEL_Y;

    MINT32 NR3D_MPNL_C_Y0_lo = rLowerNR3D.nlc_con1.bits.NR3D_MPNL_C_Y0;
    MINT32 NR3D_EPNL1_MAX_C_lo = rLowerNR3D.nlc_con1.bits.NR3D_EPNL1_MAX_C;
    MINT32 NR3D_EPNL1_COR_C_lo = rLowerNR3D.nlc_con1.bits.NR3D_EPNL1_COR_C;
    MINT32 NR3D_EPNL2_MAX_C_lo = rLowerNR3D.nlc_con1.bits.NR3D_EPNL2_MAX_C;
    MINT32 NR3D_EPNL2_COR_C_lo = rLowerNR3D.nlc_con1.bits.NR3D_EPNL2_COR_C;
    MINT32 NR3D_MPNL1_MAX_C_lo = rLowerNR3D.nlc_con1.bits.NR3D_MPNL1_MAX_C;
    MINT32 NR3D_MPNL1_COR_C_lo = rLowerNR3D.nlc_con1.bits.NR3D_MPNL1_COR_C;

    MINT32 NR3D_MPNL2_MAX_C_lo = rLowerNR3D.nlc_con2.bits.NR3D_MPNL2_MAX_C;
    MINT32 NR3D_MPNL2_COR_C_lo = rLowerNR3D.nlc_con2.bits.NR3D_MPNL2_COR_C;
    MINT32 NR3D_MPNL3_MAX_C_lo = rLowerNR3D.nlc_con2.bits.NR3D_MPNL3_MAX_C;
    MINT32 NR3D_MPNL4_MAX_C_lo = rLowerNR3D.nlc_con2.bits.NR3D_MPNL4_MAX_C;
    MINT32 NR3D_PVAI1_GAIN_C_lo = rLowerNR3D.nlc_con2.bits.NR3D_PVAI1_GAIN_C;
    MINT32 NR3D_PVAR1_MIN_C_lo = rLowerNR3D.nlc_con2.bits.NR3D_PVAR1_MIN_C;
    MINT32 NR3D_PVAR1_MAX_C_lo = rLowerNR3D.nlc_con2.bits.NR3D_PVAR1_MAX_C;

    MINT32 NR3D_MTLVL_COR_C_lo = rLowerNR3D.mlc_con.bits.NR3D_MTLVL_COR_C;
    MINT32 NR3D_MTLVL_GAIN_C_lo = rLowerNR3D.mlc_con.bits.NR3D_MTLVL_GAIN_C;

    MINT32 NR3D_SVLVL_M1_LS1_Y_lo = rLowerNR3D.sly_con.bits.NR3D_SVLVL_M1_LS1_Y;
    MINT32 NR3D_SVLVL_M2_LS1_Y_lo = rLowerNR3D.sly_con.bits.NR3D_SVLVL_M2_LS1_Y;
    MINT32 NR3D_STLVL_M3_LS1_Y_lo = rLowerNR3D.sly_con.bits.NR3D_STLVL_M3_LS1_Y;
    MINT32 NR3D_SVLVL_M1_ADD_Y_lo = rLowerNR3D.sly_con.bits.NR3D_SVLVL_M1_ADD_Y;
    MINT32 NR3D_SVLVL_M1_MAX_Y_lo = rLowerNR3D.sly_con.bits.NR3D_SVLVL_M1_MAX_Y;
    MINT32 NR3D_SVLVL_M2_ADD_Y_lo = rLowerNR3D.sly_con.bits.NR3D_SVLVL_M2_ADD_Y;
    MINT32 NR3D_SVLVL_M2_MAX_Y_lo = rLowerNR3D.sly_con.bits.NR3D_SVLVL_M2_MAX_Y;
    MINT32 NR3D_STLVL_M3_ADD_Y_lo = rLowerNR3D.sly_con.bits.NR3D_STLVL_M3_ADD_Y;
    MINT32 NR3D_STLVL_M3_MAX_Y_lo = rLowerNR3D.sly_con.bits.NR3D_STLVL_M3_MAX_Y;

    MINT32 NR3D_SVLVL_M1_LS1_C_lo = rLowerNR3D.slc_con.bits.NR3D_SVLVL_M1_LS1_C;
    MINT32 NR3D_SVLVL_M2_LS1_C_lo = rLowerNR3D.slc_con.bits.NR3D_SVLVL_M2_LS1_C;
    MINT32 NR3D_STLVL_M3_LS1_C_lo = rLowerNR3D.slc_con.bits.NR3D_STLVL_M3_LS1_C;
    MINT32 NR3D_SVLVL_M1_ADD_C_lo = rLowerNR3D.slc_con.bits.NR3D_SVLVL_M1_ADD_C;
    MINT32 NR3D_SVLVL_M1_MAX_C_lo = rLowerNR3D.slc_con.bits.NR3D_SVLVL_M1_MAX_C;
    MINT32 NR3D_SVLVL_M2_ADD_C_lo = rLowerNR3D.slc_con.bits.NR3D_SVLVL_M2_ADD_C;
    MINT32 NR3D_SVLVL_M2_MAX_C_lo = rLowerNR3D.slc_con.bits.NR3D_SVLVL_M2_MAX_C;
    MINT32 NR3D_STLVL_M3_ADD_C_lo = rLowerNR3D.slc_con.bits.NR3D_STLVL_M3_ADD_C;
    MINT32 NR3D_STLVL_M3_MAX_C_lo = rLowerNR3D.slc_con.bits.NR3D_STLVL_M3_MAX_C;

    MINT32 NR3D_DDIF_GAIN_Y_lo = rLowerNR3D.difg_con.bits.NR3D_DDIF_GAIN_Y;
    MINT32 NR3D_DDIF_GAIN_C_lo = rLowerNR3D.difg_con.bits.NR3D_DDIF_GAIN_C;
    MINT32 NR3D_BDIF_GAIN_Y_lo = rLowerNR3D.difg_con.bits.NR3D_BDIF_GAIN_Y;
    MINT32 NR3D_BDIF_GAIN_C_lo = rLowerNR3D.difg_con.bits.NR3D_BDIF_GAIN_C;

    MINT32 NR3D_YCJC_TH1_Y_lo = rLowerNR3D.ycjc_con1.bits.NR3D_YCJC_TH1_Y;
    MINT32 NR3D_YCJC_TH3_Y_lo = rLowerNR3D.ycjc_con1.bits.NR3D_YCJC_TH3_Y;
    MINT32 NR3D_YCJC_TH1_C_lo = rLowerNR3D.ycjc_con1.bits.NR3D_YCJC_TH1_C;
    MINT32 NR3D_YCJC_TH2_C_lo = rLowerNR3D.ycjc_con1.bits.NR3D_YCJC_TH2_C;

    MINT32 NR3D_YCJC_TH3_C_lo = rLowerNR3D.ycjc_con2.bits.NR3D_YCJC_TH3_C;
    MINT32 NR3D_YCJC_GAIN_Y_lo = rLowerNR3D.ycjc_con2.bits.NR3D_YCJC_GAIN_Y;
    MINT32 NR3D_YCJC_GAIN_C_lo = rLowerNR3D.ycjc_con2.bits.NR3D_YCJC_GAIN_C;
    MINT32 NR3D_YCJC_RS_Y_lo = rLowerNR3D.ycjc_con2.bits.NR3D_YCJC_RS_Y;
    MINT32 NR3D_YCJC_RS_C_lo = rLowerNR3D.ycjc_con2.bits.NR3D_YCJC_RS_C;

    // ======================
    // interpolation section
    double dbVar;

    // alpha curve y
    MINT32 NR3D_ALPHA_Y_X1_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_ALPHA_Y_X1_lo,NR3D_ALPHA_Y_X1_up,0);
    MINT32 NR3D_ALPHA_Y_X2_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_ALPHA_Y_X2_lo,NR3D_ALPHA_Y_X2_up,0);
    MINT32 NR3D_ALPHA_Y_X3_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_ALPHA_Y_X3_lo,NR3D_ALPHA_Y_X3_up,0);
    MINT32 NR3D_ALPHA_Y_X4_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_ALPHA_Y_X4_lo,NR3D_ALPHA_Y_X4_up,0);
    MINT32 NR3D_ALPHA_Y_Y0_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_ALPHA_Y_Y0_lo,NR3D_ALPHA_Y_Y0_up,0);
    MINT32 NR3D_ALPHA_Y_Y1_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_ALPHA_Y_Y1_lo,NR3D_ALPHA_Y_Y1_up,0);

    MINT32 NR3D_ALPHA_Y_Y2_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_ALPHA_Y_Y2_lo,NR3D_ALPHA_Y_Y2_up,0);
    MINT32 NR3D_ALPHA_Y_Y3_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_ALPHA_Y_Y3_lo,NR3D_ALPHA_Y_Y3_up,0);
    MINT32 NR3D_ALPHA_Y_Y4_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_ALPHA_Y_Y4_lo,NR3D_ALPHA_Y_Y4_up,0);

    dbVar = (double)(NR3D_ALPHA_Y_Y1_ou-NR3D_ALPHA_Y_Y0_ou)/(double)(NR3D_ALPHA_Y_X1_ou-0);
    MINT32 NR3D_ALPHA_Y_S0_ou = CLIP_NR3D(((MINT32)(dbVar*8+((dbVar>0)?0.5:-0.5))),-256,255);
    dbVar = (double)(NR3D_ALPHA_Y_Y2_ou-NR3D_ALPHA_Y_Y1_ou)/(double)(NR3D_ALPHA_Y_X2_ou-NR3D_ALPHA_Y_X1_ou);
    MINT32 NR3D_ALPHA_Y_S1_ou = CLIP_NR3D(((MINT32)(dbVar*8+((dbVar>0)?0.5:-0.5))),-256,255);
    dbVar = (double)(NR3D_ALPHA_Y_Y3_ou-NR3D_ALPHA_Y_Y2_ou)/(double)(NR3D_ALPHA_Y_X3_ou-NR3D_ALPHA_Y_X2_ou);
    MINT32 NR3D_ALPHA_Y_S2_ou = CLIP_NR3D(((MINT32)(dbVar*8+((dbVar>0)?0.5:-0.5))),-256,255);
    dbVar = (double)(NR3D_ALPHA_Y_Y4_ou-NR3D_ALPHA_Y_Y3_ou)/(double)(NR3D_ALPHA_Y_X4_ou-NR3D_ALPHA_Y_X3_ou);
    MINT32 NR3D_ALPHA_Y_S3_ou = CLIP_NR3D(((MINT32)(dbVar*8+((dbVar>0)?0.5:-0.5))),-256,255);

    // alpha curve c
    MINT32 NR3D_ALPHA_C_X1_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_ALPHA_C_X1_lo,NR3D_ALPHA_C_X1_up,0);
    MINT32 NR3D_ALPHA_C_X2_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_ALPHA_C_X2_lo,NR3D_ALPHA_C_X2_up,0);
    MINT32 NR3D_ALPHA_C_X3_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_ALPHA_C_X3_lo,NR3D_ALPHA_C_X3_up,0);
    MINT32 NR3D_ALPHA_C_X4_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_ALPHA_C_X4_lo,NR3D_ALPHA_C_X4_up,0);

    MINT32 NR3D_ALPHA_C_Y0_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_ALPHA_C_Y0_lo,NR3D_ALPHA_C_Y0_up,0);
    MINT32 NR3D_ALPHA_C_Y1_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_ALPHA_C_Y1_lo,NR3D_ALPHA_C_Y1_up,0);
    MINT32 NR3D_ALPHA_C_Y2_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_ALPHA_C_Y2_lo,NR3D_ALPHA_C_Y2_up,0);
    MINT32 NR3D_ALPHA_C_Y3_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_ALPHA_C_Y3_lo,NR3D_ALPHA_C_Y3_up,0);
    MINT32 NR3D_ALPHA_C_Y4_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_ALPHA_C_Y4_lo,NR3D_ALPHA_C_Y4_up,0);

    dbVar = (double)(NR3D_ALPHA_C_Y1_ou-NR3D_ALPHA_C_Y0_ou)/(double)(NR3D_ALPHA_C_X1_ou-0);
    MINT32 NR3D_ALPHA_C_S0_ou = CLIP_NR3D(((MINT32)(dbVar*8+((dbVar>0)?0.5:-0.5))),-256,255);
    dbVar = (double)(NR3D_ALPHA_C_Y2_ou-NR3D_ALPHA_C_Y1_ou)/(double)(NR3D_ALPHA_C_X2_ou-NR3D_ALPHA_C_X1_ou);
    MINT32 NR3D_ALPHA_C_S1_ou = CLIP_NR3D(((MINT32)(dbVar*8+((dbVar>0)?0.5:-0.5))),-256,255);
    dbVar = (double)(NR3D_ALPHA_C_Y3_ou-NR3D_ALPHA_C_Y2_ou)/(double)(NR3D_ALPHA_C_X3_ou-NR3D_ALPHA_C_X2_ou);
    MINT32 NR3D_ALPHA_C_S2_ou = CLIP_NR3D(((MINT32)(dbVar*8+((dbVar>0)?0.5:-0.5))),-256,255);
    dbVar = (double)(NR3D_ALPHA_C_Y4_ou-NR3D_ALPHA_C_Y3_ou)/(double)(NR3D_ALPHA_C_X4_ou-NR3D_ALPHA_C_X3_ou);
    MINT32 NR3D_ALPHA_C_S3_ou = CLIP_NR3D(((MINT32)(dbVar*8+((dbVar>0)?0.5:-0.5))),-256,255);

    // beta curve y
    MINT32 NR3D_BETA1_Y_X1_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_BETA1_Y_X1_lo,NR3D_BETA1_Y_X1_up,0);
    MINT32 NR3D_BETA1_Y_X2_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_BETA1_Y_X2_lo,NR3D_BETA1_Y_X2_up,0);
    MINT32 NR3D_BETA1_Y_X3_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_BETA1_Y_X3_lo,NR3D_BETA1_Y_X3_up,0);
    MINT32 NR3D_BETA1_Y_X4_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_BETA1_Y_X4_lo,NR3D_BETA1_Y_X4_up,0);

    MINT32 NR3D_BETA1_Y_Y1_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_BETA1_Y_Y1_lo,NR3D_BETA1_Y_Y1_up,0);
    MINT32 NR3D_BETA1_Y_Y2_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_BETA1_Y_Y2_lo,NR3D_BETA1_Y_Y2_up,0);
    MINT32 NR3D_BETA1_Y_Y3_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_BETA1_Y_Y3_lo,NR3D_BETA1_Y_Y3_up,0);
    MINT32 NR3D_BETA1_Y_Y4_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_BETA1_Y_Y4_lo,NR3D_BETA1_Y_Y4_up,0);

    dbVar = (double)(NR3D_BETA1_Y_Y2_ou-NR3D_BETA1_Y_Y1_ou)/(double)(NR3D_BETA1_Y_X2_ou-NR3D_BETA1_Y_X1_ou);
    MINT32 NR3D_BETA1_Y_S1_ou = CLIP_NR3D(((MINT32)(dbVar*8+((dbVar>0)?0.5:-0.5))),0,127);
    dbVar = (double)(NR3D_BETA1_Y_Y3_ou-NR3D_BETA1_Y_Y2_ou)/(double)(NR3D_BETA1_Y_X3_ou-NR3D_BETA1_Y_X2_ou);
    MINT32 NR3D_BETA1_Y_S2_ou = CLIP_NR3D(((MINT32)(dbVar*8+((dbVar>0)?0.5:-0.5))),0,127);
    dbVar = (double)(NR3D_BETA1_Y_Y4_ou-NR3D_BETA1_Y_Y3_ou)/(double)(NR3D_BETA1_Y_X4_ou-NR3D_BETA1_Y_X3_ou);
    MINT32 NR3D_BETA1_Y_S3_ou = CLIP_NR3D(((MINT32)(dbVar*8+((dbVar>0)?0.5:-0.5))),0,127);

    // beta curve c
    MINT32 NR3D_BETA1_C_X1_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_BETA1_C_X1_lo,NR3D_BETA1_C_X1_up,0);
    MINT32 NR3D_BETA1_C_X2_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_BETA1_C_X2_lo,NR3D_BETA1_C_X2_up,0);
    MINT32 NR3D_BETA1_C_X3_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_BETA1_C_X3_lo,NR3D_BETA1_C_X3_up,0);
    MINT32 NR3D_BETA1_C_X4_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_BETA1_C_X4_lo,NR3D_BETA1_C_X4_up,0);

    MINT32 NR3D_BETA1_C_Y1_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_BETA1_C_Y1_lo,NR3D_BETA1_C_Y1_up,0);
    MINT32 NR3D_BETA1_C_Y2_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_BETA1_C_Y2_lo,NR3D_BETA1_C_Y2_up,0);
    MINT32 NR3D_BETA1_C_Y3_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_BETA1_C_Y3_lo,NR3D_BETA1_C_Y3_up,0);
    MINT32 NR3D_BETA1_C_Y4_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_BETA1_C_Y4_lo,NR3D_BETA1_C_Y4_up,0);

    dbVar = (double)(NR3D_BETA1_C_Y2_ou-NR3D_BETA1_C_Y1_ou)/(double)(NR3D_BETA1_C_X2_ou-NR3D_BETA1_C_X1_ou);
    MINT32 NR3D_BETA1_C_S1_ou = CLIP_NR3D(((MINT32)(dbVar*8+((dbVar>0)?0.5:-0.5))),0,127);
    dbVar = (double)(NR3D_BETA1_C_Y3_ou-NR3D_BETA1_C_Y2_ou)/(double)(NR3D_BETA1_C_X3_ou-NR3D_BETA1_C_X2_ou);
    MINT32 NR3D_BETA1_C_S2_ou = CLIP_NR3D(((MINT32)(dbVar*8+((dbVar>0)?0.5:-0.5))),0,127);
    dbVar = (double)(NR3D_BETA1_C_Y4_ou-NR3D_BETA1_C_Y3_ou)/(double)(NR3D_BETA1_C_X4_ou-NR3D_BETA1_C_X3_ou);
    MINT32 NR3D_BETA1_C_S3_ou = CLIP_NR3D(((MINT32)(dbVar*8+((dbVar>0)?0.5:-0.5))),0,127);

    // LSCG1 (beta2) curve y
    MINT32 NR3D_LSCG1_Y_X1_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_LSCG1_Y_X1_lo,NR3D_LSCG1_Y_X1_up,0);
    MINT32 NR3D_LSCG1_Y_X2_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_LSCG1_Y_X2_lo,NR3D_LSCG1_Y_X2_up,0);
    MINT32 NR3D_LSCG1_Y_X3_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_LSCG1_Y_X3_lo,NR3D_LSCG1_Y_X3_up,0);
    MINT32 NR3D_LSCG1_Y_X4_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_LSCG1_Y_X4_lo,NR3D_LSCG1_Y_X4_up,0);

    MINT32 NR3D_LSCG1_Y_Y0_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_LSCG1_Y_Y0_lo,NR3D_LSCG1_Y_Y0_up,0);
    MINT32 NR3D_LSCG1_Y_Y1_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_LSCG1_Y_Y1_lo,NR3D_LSCG1_Y_Y1_up,0);
    MINT32 NR3D_LSCG1_Y_Y2_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_LSCG1_Y_Y2_lo,NR3D_LSCG1_Y_Y2_up,0);
    MINT32 NR3D_LSCG1_Y_Y3_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_LSCG1_Y_Y3_lo,NR3D_LSCG1_Y_Y3_up,0);
    MINT32 NR3D_LSCG1_Y_Y4_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_LSCG1_Y_Y4_lo,NR3D_LSCG1_Y_Y4_up,0);

    dbVar = (double)(NR3D_LSCG1_Y_Y1_ou-NR3D_LSCG1_Y_Y0_ou)/(double)(NR3D_LSCG1_Y_X1_ou-0);
    MINT32 NR3D_LSCG1_Y_S0_ou = CLIP_NR3D(((MINT32)(dbVar*8+((dbVar>0)?0.5:-0.5))),0,255);
    dbVar = (double)(NR3D_LSCG1_Y_Y2_ou-NR3D_LSCG1_Y_Y1_ou)/(double)(NR3D_LSCG1_Y_X2_ou-NR3D_LSCG1_Y_X1_ou);
    MINT32 NR3D_LSCG1_Y_S1_ou = CLIP_NR3D(((MINT32)(dbVar*8+((dbVar>0)?0.5:-0.5))),0,255);
    dbVar = (double)(NR3D_LSCG1_Y_Y3_ou-NR3D_LSCG1_Y_Y2_ou)/(double)(NR3D_LSCG1_Y_X3_ou-NR3D_LSCG1_Y_X2_ou);
    MINT32 NR3D_LSCG1_Y_S2_ou = CLIP_NR3D(((MINT32)(dbVar*8+((dbVar>0)?0.5:-0.5))),0,255);
    dbVar = (double)(NR3D_LSCG1_Y_Y4_ou-NR3D_LSCG1_Y_Y3_ou)/(double)(NR3D_LSCG1_Y_X4_ou-NR3D_LSCG1_Y_X3_ou);
    MINT32 NR3D_LSCG1_Y_S3_ou = CLIP_NR3D(((MINT32)(dbVar*8+((dbVar>0)?0.5:-0.5))),0,255);

    MINT32 NR3D_LSCG1_DLT_GAIN_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_LSCG1_DLT_GAIN_Y_lo,NR3D_LSCG1_DLT_GAIN_Y_up,0);
    MINT32 NR3D_LSCG1_DLT_GAIN_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_LSCG1_DLT_GAIN_C_lo,NR3D_LSCG1_DLT_GAIN_C_up,0);

    MINT32 NR3D_MPNL_Y_Y0_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_MPNL_Y_Y0_lo,NR3D_MPNL_Y_Y0_up,0);
    MINT32 NR3D_EPNL1_MAX_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_EPNL1_MAX_Y_lo,NR3D_EPNL1_MAX_Y_up,0);
    MINT32 NR3D_EPNL1_COR_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_EPNL1_COR_Y_lo,NR3D_EPNL1_COR_Y_up,0);
    MINT32 NR3D_EPNL2_MAX_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_EPNL2_MAX_Y_lo,NR3D_EPNL2_MAX_Y_up,0);
    MINT32 NR3D_EPNL2_COR_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_EPNL2_COR_Y_lo,NR3D_EPNL2_COR_Y_up,0);
    MINT32 NR3D_MPNL1_MAX_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_MPNL1_MAX_Y_lo,NR3D_MPNL1_MAX_Y_up,0);
    MINT32 NR3D_MPNL1_COR_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_MPNL1_COR_Y_lo,NR3D_MPNL1_COR_Y_up,0);

    MINT32 NR3D_PVAI1_GAIN_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_PVAI1_GAIN_Y_lo,NR3D_PVAI1_GAIN_Y_up,0);
    MINT32 NR3D_MPNL2_MAX_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_MPNL2_MAX_Y_lo,NR3D_MPNL2_MAX_Y_up,0);
    MINT32 NR3D_MPNL2_COR_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_MPNL2_COR_Y_lo,NR3D_MPNL2_COR_Y_up,0);
    MINT32 NR3D_MPNL3_MAX_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_MPNL3_MAX_Y_lo,NR3D_MPNL3_MAX_Y_up,0);
    MINT32 NR3D_MPNL4_MAX_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_MPNL4_MAX_Y_lo,NR3D_MPNL4_MAX_Y_up,0);
    MINT32 NR3D_PVAR1_MIN_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_PVAR1_MIN_Y_lo,NR3D_PVAR1_MIN_Y_up,0);
    MINT32 NR3D_PVAR1_MAX_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_PVAR1_MAX_Y_lo,NR3D_PVAR1_MAX_Y_up,0);

    MINT32 NR3D_MTLVL_COR_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_MTLVL_COR_Y_lo,NR3D_MTLVL_COR_Y_up,0);
    MINT32 NR3D_MTLVL_GAIN_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_MTLVL_GAIN_Y_lo,NR3D_MTLVL_GAIN_Y_up,0);
    MINT32 NR3D_MTLVL_VASEL_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_MTLVL_VASEL_Y_lo,NR3D_MTLVL_VASEL_Y_up,1);
    MINT32 NR3D_MTLVL_DFSEL_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_MTLVL_DFSEL_Y_lo,NR3D_MTLVL_DFSEL_Y_up,1);
    MINT32 NR3D_DDIF2_GPSEL_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_DDIF2_GPSEL_Y_lo,NR3D_DDIF2_GPSEL_Y_up,1);

    MINT32 NR3D_MPNL_C_Y0_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_MPNL_C_Y0_lo,NR3D_MPNL_C_Y0_up,0);
    MINT32 NR3D_EPNL1_MAX_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_EPNL1_MAX_C_lo,NR3D_EPNL1_MAX_C_up,0);
    MINT32 NR3D_EPNL1_COR_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_EPNL1_COR_C_lo,NR3D_EPNL1_COR_C_up,0);
    MINT32 NR3D_EPNL2_MAX_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_EPNL2_MAX_C_lo,NR3D_EPNL2_MAX_C_up,0);
    MINT32 NR3D_EPNL2_COR_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_EPNL2_COR_C_lo,NR3D_EPNL2_COR_C_up,0);
    MINT32 NR3D_MPNL1_MAX_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_MPNL1_MAX_C_lo,NR3D_MPNL1_MAX_C_up,0);
    MINT32 NR3D_MPNL1_COR_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_MPNL1_COR_C_lo,NR3D_MPNL1_COR_C_up,0);

    MINT32 NR3D_PVAI1_GAIN_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_PVAI1_GAIN_C_lo,NR3D_PVAI1_GAIN_C_up,0);
    MINT32 NR3D_MPNL2_MAX_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_MPNL2_MAX_C_lo,NR3D_MPNL2_MAX_C_up,0);
    MINT32 NR3D_MPNL2_COR_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_MPNL2_COR_C_lo,NR3D_MPNL2_COR_C_up,0);
    MINT32 NR3D_MPNL3_MAX_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_MPNL3_MAX_C_lo,NR3D_MPNL3_MAX_C_up,0);
    MINT32 NR3D_MPNL4_MAX_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_MPNL4_MAX_C_lo,NR3D_MPNL4_MAX_C_up,0);
    MINT32 NR3D_PVAR1_MIN_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_PVAR1_MIN_C_lo,NR3D_PVAR1_MIN_C_up,0);
    MINT32 NR3D_PVAR1_MAX_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_PVAR1_MAX_C_lo,NR3D_PVAR1_MAX_C_up,0);

    MINT32 NR3D_MTLVL_COR_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_MTLVL_COR_C_lo,NR3D_MTLVL_COR_C_up,0);
    MINT32 NR3D_MTLVL_GAIN_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_MTLVL_GAIN_C_lo,NR3D_MTLVL_GAIN_C_up,0);

    MINT32 NR3D_SVLVL_M1_LS1_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_SVLVL_M1_LS1_Y_lo,NR3D_SVLVL_M1_LS1_Y_up,1);
    MINT32 NR3D_SVLVL_M2_LS1_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_SVLVL_M2_LS1_Y_lo,NR3D_SVLVL_M2_LS1_Y_up,1);
    MINT32 NR3D_STLVL_M3_LS1_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_STLVL_M3_LS1_Y_lo,NR3D_STLVL_M3_LS1_Y_up,1);
    MINT32 NR3D_SVLVL_M1_ADD_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_SVLVL_M1_ADD_Y_lo,NR3D_SVLVL_M1_ADD_Y_up,0);
    MINT32 NR3D_SVLVL_M1_MAX_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_SVLVL_M1_MAX_Y_lo,NR3D_SVLVL_M1_MAX_Y_up,0);
    MINT32 NR3D_SVLVL_M2_ADD_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_SVLVL_M2_ADD_Y_lo,NR3D_SVLVL_M2_ADD_Y_up,0);
    MINT32 NR3D_SVLVL_M2_MAX_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_SVLVL_M2_MAX_Y_lo,NR3D_SVLVL_M2_MAX_Y_up,0);
    MINT32 NR3D_STLVL_M3_ADD_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_STLVL_M3_ADD_Y_lo,NR3D_STLVL_M3_ADD_Y_up,0);
    MINT32 NR3D_STLVL_M3_MAX_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_STLVL_M3_MAX_Y_lo,NR3D_STLVL_M3_MAX_Y_up,0);

    MINT32 NR3D_SVLVL_M1_LS1_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_SVLVL_M1_LS1_C_lo,NR3D_SVLVL_M1_LS1_C_up,1);
    MINT32 NR3D_SVLVL_M2_LS1_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_SVLVL_M2_LS1_C_lo,NR3D_SVLVL_M2_LS1_C_up,1);
    MINT32 NR3D_STLVL_M3_LS1_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_STLVL_M3_LS1_C_lo,NR3D_STLVL_M3_LS1_C_up,1);
    MINT32 NR3D_SVLVL_M1_ADD_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_SVLVL_M1_ADD_C_lo,NR3D_SVLVL_M1_ADD_C_up,0);
    MINT32 NR3D_SVLVL_M1_MAX_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_SVLVL_M1_MAX_C_lo,NR3D_SVLVL_M1_MAX_C_up,0);
    MINT32 NR3D_SVLVL_M2_ADD_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_SVLVL_M2_ADD_C_lo,NR3D_SVLVL_M2_ADD_C_up,0);
    MINT32 NR3D_SVLVL_M2_MAX_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_SVLVL_M2_MAX_C_lo,NR3D_SVLVL_M2_MAX_C_up,0);
    MINT32 NR3D_STLVL_M3_ADD_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_STLVL_M3_ADD_C_lo,NR3D_STLVL_M3_ADD_C_up,0);
    MINT32 NR3D_STLVL_M3_MAX_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_STLVL_M3_MAX_C_lo,NR3D_STLVL_M3_MAX_C_up,0);

    MINT32 NR3D_DDIF_GAIN_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_DDIF_GAIN_Y_lo,NR3D_DDIF_GAIN_Y_up,0);
    MINT32 NR3D_DDIF_GAIN_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_DDIF_GAIN_C_lo,NR3D_DDIF_GAIN_C_up,0);
    MINT32 NR3D_BDIF_GAIN_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_BDIF_GAIN_Y_lo,NR3D_BDIF_GAIN_Y_up,0);
    MINT32 NR3D_BDIF_GAIN_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_BDIF_GAIN_C_lo,NR3D_BDIF_GAIN_C_up,0);

    MINT32 NR3D_YCJC_TH1_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_YCJC_TH1_Y_lo,NR3D_YCJC_TH1_Y_up,0);
    MINT32 NR3D_YCJC_TH3_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_YCJC_TH3_Y_lo,NR3D_YCJC_TH3_Y_up,0);
    MINT32 NR3D_YCJC_TH1_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_YCJC_TH1_C_lo,NR3D_YCJC_TH1_C_up,0);
    MINT32 NR3D_YCJC_TH2_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_YCJC_TH2_C_lo,NR3D_YCJC_TH2_C_up,0);

    MINT32 NR3D_YCJC_TH3_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_YCJC_TH3_C_lo,NR3D_YCJC_TH3_C_up,0);
    MINT32 NR3D_YCJC_GAIN_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_YCJC_GAIN_Y_lo,NR3D_YCJC_GAIN_Y_up,0);
    MINT32 NR3D_YCJC_GAIN_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_YCJC_GAIN_C_lo,NR3D_YCJC_GAIN_C_up,0);
    MINT32 NR3D_YCJC_RS_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_YCJC_RS_Y_lo,NR3D_YCJC_RS_Y_up,1);
    MINT32 NR3D_YCJC_RS_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_YCJC_RS_C_lo,NR3D_YCJC_RS_C_up,1);


    // =============================
    char tmpStr[PROPERTY_VALUE_MAX];
    // get variable from property
    MINT32 ADB_NR3D_TUNING_EN=0;
    MINT32 ADB_NR3D_TUNING_SEL_VAL=0;

    if (bSmoothNR3DDebug)
    {
        sprintf(tmpStr, "%d", ADB_NR3D_TUNING_EN);
        property_get("vendor.debug.nr3d.bm.tuning_en", value, tmpStr);
        ADB_NR3D_TUNING_EN = atoi(value);
        if (bmCount % 33 == 0)
        {
            CAM_LOGW_IF(bSmoothNR3DDebug, "SmoothNR3D_cmd: vendor.debug.nr3d.bm.tuning_en = %d", ADB_NR3D_TUNING_EN);
        }
    }

    if (ADB_NR3D_TUNING_EN==1)
    {
        sprintf(tmpStr, "%d", ADB_NR3D_TUNING_SEL_VAL);
        property_get("vendor.debug.nr3d.bm.tuning_sel_val", value, tmpStr);
        ADB_NR3D_TUNING_SEL_VAL = atoi(value);

        static MINT32 prevFullSelVal = 0;
        if (prevFullSelVal != ADB_NR3D_TUNING_SEL_VAL)
        {
            CAM_LOGW_IF(bSmoothNR3DDebug, "SNR3D_cmd: vendor.debug.nr3d.bm.tuning_sel_val = %d", ADB_NR3D_TUNING_SEL_VAL);
        }
        prevFullSelVal = ADB_NR3D_TUNING_SEL_VAL;

        MINT32 CMD_NR3D_OFS = 10000;
        MINT32 CMD_NR3D_SEL = (ADB_NR3D_TUNING_SEL_VAL/CMD_NR3D_OFS);
        MINT32 CMD_NR3D_VAL = ADB_NR3D_TUNING_SEL_VAL - (CMD_NR3D_SEL*CMD_NR3D_OFS);

        // move to global variable
        static MINT32 TUNE_NR3D_ALPHA_Y_X_RATIO=100;
        static MINT32 TUNE_NR3D_ALPHA_Y_Y_RATIO=100;
        static MINT32 TUNE_NR3D_ALPHA_C_X_RATIO=100;
        static MINT32 TUNE_NR3D_ALPHA_C_Y_RATIO=100;
        static MINT32 TUNE_NR3D_LSCG1_Y_Y_RATIO=100;

        if (CMD_NR3D_SEL==10)
        {
            static MINT32 prevVal = TUNE_NR3D_ALPHA_Y_X_RATIO;

            TUNE_NR3D_ALPHA_Y_X_RATIO = CMD_NR3D_VAL;
            if (prevVal != CMD_NR3D_VAL)
            {
                CAM_LOGW_IF(bSmoothNR3DDebug, "SNR3D_TUNE_NR3D_ALPHA_Y_X_RATIO: prev=%d, curr=%d", prevVal, TUNE_NR3D_ALPHA_Y_X_RATIO);
            }
            prevVal = CMD_NR3D_VAL;
        }
        else if (CMD_NR3D_SEL==11)
        {
            static MINT32 prevVal = TUNE_NR3D_ALPHA_Y_Y_RATIO;

            TUNE_NR3D_ALPHA_Y_Y_RATIO = CMD_NR3D_VAL;
            if (prevVal != CMD_NR3D_VAL)
            {
                CAM_LOGW_IF(bSmoothNR3DDebug, "SNR3D_TUNE_NR3D_ALPHA_C_X_RATIO: prev=%d, curr=%d", prevVal, TUNE_NR3D_ALPHA_Y_Y_RATIO);
            }
            prevVal = CMD_NR3D_VAL;
        }
        else if (CMD_NR3D_SEL==12)
        {
            static MINT32 prevVal = TUNE_NR3D_ALPHA_C_X_RATIO;

            TUNE_NR3D_ALPHA_C_X_RATIO = CMD_NR3D_VAL;
            if (prevVal != CMD_NR3D_VAL)
            {
                CAM_LOGW_IF(bSmoothNR3DDebug, "SNR3D_TUNE_NR3D_ALPHA_C_X_RATIO: prev=%d, curr=%d", prevVal, TUNE_NR3D_ALPHA_C_X_RATIO);
            }
            prevVal = CMD_NR3D_VAL;
        }
        else if (CMD_NR3D_SEL==13)
        {
            static MINT32 prevVal = TUNE_NR3D_ALPHA_C_Y_RATIO;

            TUNE_NR3D_ALPHA_C_Y_RATIO = CMD_NR3D_VAL;
            if (prevVal != CMD_NR3D_VAL)
            {
                CAM_LOGW_IF(bSmoothNR3DDebug, "SNR3D_TUNE_NR3D_ALPHA_C_Y_RATIO: prev=%d, curr=%d", prevVal, TUNE_NR3D_ALPHA_C_Y_RATIO);
            }
            prevVal = CMD_NR3D_VAL;
        }
        else if (CMD_NR3D_SEL==14)
        {
            static MINT32 prevVal = TUNE_NR3D_LSCG1_Y_Y_RATIO;

            TUNE_NR3D_LSCG1_Y_Y_RATIO = CMD_NR3D_VAL;
            if (prevVal != CMD_NR3D_VAL)
            {
                CAM_LOGW_IF(bSmoothNR3DDebug, "SNR3D_TUNE_NR3D_LSCG1_Y_Y_RATIO: prev=%d, curr=%d", prevVal, TUNE_NR3D_LSCG1_Y_Y_RATIO);
            }
            prevVal = CMD_NR3D_VAL;
        }
        else
        {
            CAM_LOGE_IF(bSmoothNR3DDebug, "!!err: why here, wrong !!");
        }

        NR3D_ALPHA_Y_X1_ou = ((NR3D_ALPHA_Y_X1_ou*TUNE_NR3D_ALPHA_Y_X_RATIO)+50)/100;
        NR3D_ALPHA_Y_X2_ou = ((NR3D_ALPHA_Y_X2_ou*TUNE_NR3D_ALPHA_Y_X_RATIO)+50)/100;
        NR3D_ALPHA_Y_X3_ou = ((NR3D_ALPHA_Y_X3_ou*TUNE_NR3D_ALPHA_Y_X_RATIO)+50)/100;
        NR3D_ALPHA_Y_X4_ou = ((NR3D_ALPHA_Y_X4_ou*TUNE_NR3D_ALPHA_Y_X_RATIO)+50)/100;

        NR3D_ALPHA_Y_X1_ou = CLIP_NR3D(NR3D_ALPHA_Y_X1_ou,0,63);
        NR3D_ALPHA_Y_X2_ou = CLIP_NR3D(NR3D_ALPHA_Y_X2_ou,0,63);
        NR3D_ALPHA_Y_X3_ou = CLIP_NR3D(NR3D_ALPHA_Y_X3_ou,0,63);
        NR3D_ALPHA_Y_X4_ou = CLIP_NR3D(NR3D_ALPHA_Y_X4_ou,0,63);

        NR3D_ALPHA_Y_Y0_ou = ((NR3D_ALPHA_Y_Y0_ou*TUNE_NR3D_ALPHA_Y_Y_RATIO)+50)/100;
        NR3D_ALPHA_Y_Y1_ou = ((NR3D_ALPHA_Y_Y1_ou*TUNE_NR3D_ALPHA_Y_Y_RATIO)+50)/100;
        NR3D_ALPHA_Y_Y2_ou = ((NR3D_ALPHA_Y_Y2_ou*TUNE_NR3D_ALPHA_Y_Y_RATIO)+50)/100;
        NR3D_ALPHA_Y_Y3_ou = ((NR3D_ALPHA_Y_Y3_ou*TUNE_NR3D_ALPHA_Y_Y_RATIO)+50)/100;
        NR3D_ALPHA_Y_Y4_ou = ((NR3D_ALPHA_Y_Y4_ou*TUNE_NR3D_ALPHA_Y_Y_RATIO)+50)/100;

        NR3D_ALPHA_Y_Y0_ou = CLIP_NR3D(NR3D_ALPHA_Y_Y0_ou,0,31);
        NR3D_ALPHA_Y_Y1_ou = CLIP_NR3D(NR3D_ALPHA_Y_Y1_ou,0,31);
        NR3D_ALPHA_Y_Y2_ou = CLIP_NR3D(NR3D_ALPHA_Y_Y2_ou,0,31);
        NR3D_ALPHA_Y_Y3_ou = CLIP_NR3D(NR3D_ALPHA_Y_Y3_ou,0,31);
        NR3D_ALPHA_Y_Y4_ou = CLIP_NR3D(NR3D_ALPHA_Y_Y4_ou,0,31);

        dbVar = (double)(NR3D_ALPHA_Y_Y1_ou-NR3D_ALPHA_Y_Y0_ou)/(double)(NR3D_ALPHA_Y_X1_ou-0);
        NR3D_ALPHA_Y_S0_ou = CLIP_NR3D(((MINT32)(dbVar*8+((dbVar>0)?0.5:-0.5))),-256,255);
        dbVar = (double)(NR3D_ALPHA_Y_Y2_ou-NR3D_ALPHA_Y_Y1_ou)/(double)(NR3D_ALPHA_Y_X2_ou-NR3D_ALPHA_Y_X1_ou);
        NR3D_ALPHA_Y_S1_ou = CLIP_NR3D(((MINT32)(dbVar*8+((dbVar>0)?0.5:-0.5))),-256,255);
        dbVar = (double)(NR3D_ALPHA_Y_Y3_ou-NR3D_ALPHA_Y_Y2_ou)/(double)(NR3D_ALPHA_Y_X3_ou-NR3D_ALPHA_Y_X2_ou);
        NR3D_ALPHA_Y_S2_ou = CLIP_NR3D(((MINT32)(dbVar*8+((dbVar>0)?0.5:-0.5))),-256,255);
        dbVar = (double)(NR3D_ALPHA_Y_Y4_ou-NR3D_ALPHA_Y_Y3_ou)/(double)(NR3D_ALPHA_Y_X4_ou-NR3D_ALPHA_Y_X3_ou);
        NR3D_ALPHA_Y_S3_ou = CLIP_NR3D(((MINT32)(dbVar*8+((dbVar>0)?0.5:-0.5))),-256,255);

        NR3D_ALPHA_C_X1_ou = ((NR3D_ALPHA_C_X1_ou*TUNE_NR3D_ALPHA_C_X_RATIO)+50)/100;
        NR3D_ALPHA_C_X2_ou = ((NR3D_ALPHA_C_X2_ou*TUNE_NR3D_ALPHA_C_X_RATIO)+50)/100;
        NR3D_ALPHA_C_X3_ou = ((NR3D_ALPHA_C_X3_ou*TUNE_NR3D_ALPHA_C_X_RATIO)+50)/100;
        NR3D_ALPHA_C_X4_ou = ((NR3D_ALPHA_C_X4_ou*TUNE_NR3D_ALPHA_C_X_RATIO)+50)/100;

        NR3D_ALPHA_C_X1_ou = CLIP_NR3D(NR3D_ALPHA_C_X1_ou,0,63);
        NR3D_ALPHA_C_X2_ou = CLIP_NR3D(NR3D_ALPHA_C_X2_ou,0,63);
        NR3D_ALPHA_C_X3_ou = CLIP_NR3D(NR3D_ALPHA_C_X3_ou,0,63);
        NR3D_ALPHA_C_X4_ou = CLIP_NR3D(NR3D_ALPHA_C_X4_ou,0,63);

        NR3D_ALPHA_C_Y0_ou = ((NR3D_ALPHA_C_Y0_ou*TUNE_NR3D_ALPHA_C_Y_RATIO)+50)/100;
        NR3D_ALPHA_C_Y1_ou = ((NR3D_ALPHA_C_Y1_ou*TUNE_NR3D_ALPHA_C_Y_RATIO)+50)/100;
        NR3D_ALPHA_C_Y2_ou = ((NR3D_ALPHA_C_Y2_ou*TUNE_NR3D_ALPHA_C_Y_RATIO)+50)/100;
        NR3D_ALPHA_C_Y3_ou = ((NR3D_ALPHA_C_Y3_ou*TUNE_NR3D_ALPHA_C_Y_RATIO)+50)/100;
        NR3D_ALPHA_C_Y4_ou = ((NR3D_ALPHA_C_Y4_ou*TUNE_NR3D_ALPHA_C_Y_RATIO)+50)/100;

        NR3D_ALPHA_C_Y0_ou = CLIP_NR3D(NR3D_ALPHA_C_Y0_ou,0,31);
        NR3D_ALPHA_C_Y1_ou = CLIP_NR3D(NR3D_ALPHA_C_Y1_ou,0,31);
        NR3D_ALPHA_C_Y2_ou = CLIP_NR3D(NR3D_ALPHA_C_Y2_ou,0,31);
        NR3D_ALPHA_C_Y3_ou = CLIP_NR3D(NR3D_ALPHA_C_Y3_ou,0,31);
        NR3D_ALPHA_C_Y4_ou = CLIP_NR3D(NR3D_ALPHA_C_Y4_ou,0,31);

        dbVar = (double)(NR3D_ALPHA_C_Y1_ou-NR3D_ALPHA_C_Y0_ou)/(double)(NR3D_ALPHA_C_X1_ou-0);
        NR3D_ALPHA_C_S0_ou = CLIP_NR3D(((MINT32)(dbVar*8+((dbVar>0)?0.5:-0.5))),-256,255);
        dbVar = (double)(NR3D_ALPHA_C_Y2_ou-NR3D_ALPHA_C_Y1_ou)/(double)(NR3D_ALPHA_C_X2_ou-NR3D_ALPHA_C_X1_ou);
        NR3D_ALPHA_C_S1_ou = CLIP_NR3D(((MINT32)(dbVar*8+((dbVar>0)?0.5:-0.5))),-256,255);
        dbVar = (double)(NR3D_ALPHA_C_Y3_ou-NR3D_ALPHA_C_Y2_ou)/(double)(NR3D_ALPHA_C_X3_ou-NR3D_ALPHA_C_X2_ou);
        NR3D_ALPHA_C_S2_ou = CLIP_NR3D(((MINT32)(dbVar*8+((dbVar>0)?0.5:-0.5))),-256,255);
        dbVar = (double)(NR3D_ALPHA_C_Y4_ou-NR3D_ALPHA_C_Y3_ou)/(double)(NR3D_ALPHA_C_X4_ou-NR3D_ALPHA_C_X3_ou);
        NR3D_ALPHA_C_S3_ou = CLIP_NR3D(((MINT32)(dbVar*8+((dbVar>0)?0.5:-0.5))),-256,255);

        NR3D_LSCG1_Y_Y1_ou = NR3D_LSCG1_Y_Y0_ou - ((NR3D_LSCG1_Y_Y0_ou-NR3D_LSCG1_Y_Y1_ou)*TUNE_NR3D_LSCG1_Y_Y_RATIO+50)/100;
        NR3D_LSCG1_Y_Y2_ou = NR3D_LSCG1_Y_Y0_ou - ((NR3D_LSCG1_Y_Y0_ou-NR3D_LSCG1_Y_Y2_ou)*TUNE_NR3D_LSCG1_Y_Y_RATIO+50)/100;
        NR3D_LSCG1_Y_Y3_ou = NR3D_LSCG1_Y_Y0_ou - ((NR3D_LSCG1_Y_Y0_ou-NR3D_LSCG1_Y_Y3_ou)*TUNE_NR3D_LSCG1_Y_Y_RATIO+50)/100;
        NR3D_LSCG1_Y_Y4_ou = NR3D_LSCG1_Y_Y0_ou - ((NR3D_LSCG1_Y_Y0_ou-NR3D_LSCG1_Y_Y4_ou)*TUNE_NR3D_LSCG1_Y_Y_RATIO+50)/100;

        NR3D_LSCG1_Y_Y1_ou = CLIP_NR3D(NR3D_LSCG1_Y_Y1_ou,0,63);
        NR3D_LSCG1_Y_Y2_ou = CLIP_NR3D(NR3D_LSCG1_Y_Y2_ou,0,63);
        NR3D_LSCG1_Y_Y3_ou = CLIP_NR3D(NR3D_LSCG1_Y_Y3_ou,0,63);
        NR3D_LSCG1_Y_Y4_ou = CLIP_NR3D(NR3D_LSCG1_Y_Y4_ou,0,63);

        dbVar = (double)(NR3D_LSCG1_Y_Y1_ou-NR3D_LSCG1_Y_Y0_ou)/(double)(NR3D_LSCG1_Y_X1_ou-0);
        NR3D_LSCG1_Y_S0_ou = CLIP_NR3D(((MINT32)(dbVar*8+((dbVar>0)?0.5:-0.5))),0,255);
        dbVar = (double)(NR3D_LSCG1_Y_Y2_ou-NR3D_LSCG1_Y_Y1_ou)/(double)(NR3D_LSCG1_Y_X2_ou-NR3D_LSCG1_Y_X1_ou);
        NR3D_LSCG1_Y_S1_ou = CLIP_NR3D(((MINT32)(dbVar*8+((dbVar>0)?0.5:-0.5))),0,255);
        dbVar = (double)(NR3D_LSCG1_Y_Y3_ou-NR3D_LSCG1_Y_Y2_ou)/(double)(NR3D_LSCG1_Y_X3_ou-NR3D_LSCG1_Y_X2_ou);
        NR3D_LSCG1_Y_S2_ou = CLIP_NR3D(((MINT32)(dbVar*8+((dbVar>0)?0.5:-0.5))),0,255);
        dbVar = (double)(NR3D_LSCG1_Y_Y4_ou-NR3D_LSCG1_Y_Y3_ou)/(double)(NR3D_LSCG1_Y_X4_ou-NR3D_LSCG1_Y_X3_ou);
        NR3D_LSCG1_Y_S3_ou = CLIP_NR3D(((MINT32)(dbVar*8+((dbVar>0)?0.5:-0.5))),0,255);
    }

    // =============================

    rSmoothNR3D.ay_con1.bits.NR3D_ALPHA_Y_X1 = NR3D_ALPHA_Y_X1_ou;
    rSmoothNR3D.ay_con1.bits.NR3D_ALPHA_Y_X2 = NR3D_ALPHA_Y_X2_ou;
    rSmoothNR3D.ay_con1.bits.NR3D_ALPHA_Y_X3 = NR3D_ALPHA_Y_X3_ou;
    rSmoothNR3D.ay_con1.bits.NR3D_ALPHA_Y_X4 = NR3D_ALPHA_Y_X4_ou;

    rSmoothNR3D.ay_con2.bits.NR3D_ALPHA_Y_Y0 = NR3D_ALPHA_Y_Y0_ou;
    rSmoothNR3D.ay_con2.bits.NR3D_ALPHA_Y_Y1 = NR3D_ALPHA_Y_Y1_ou;
    rSmoothNR3D.ay_con2.bits.NR3D_ALPHA_Y_Y2 = NR3D_ALPHA_Y_Y2_ou;
    rSmoothNR3D.ay_con2.bits.NR3D_ALPHA_Y_Y3 = NR3D_ALPHA_Y_Y3_ou;

    rSmoothNR3D.ay_con3.bits.NR3D_ALPHA_Y_Y4 = NR3D_ALPHA_Y_Y4_ou;

    rSmoothNR3D.ay_con4.bits.NR3D_ALPHA_Y_S0 = NR3D_ALPHA_Y_S0_ou;
    rSmoothNR3D.ay_con4.bits.NR3D_ALPHA_Y_S1 = NR3D_ALPHA_Y_S1_ou;

    rSmoothNR3D.ay_con5.bits.NR3D_ALPHA_Y_S2 = NR3D_ALPHA_Y_S2_ou;
    rSmoothNR3D.ay_con5.bits.NR3D_ALPHA_Y_S3 = NR3D_ALPHA_Y_S3_ou;

    rSmoothNR3D.ac_con1.bits.NR3D_ALPHA_C_X1 = NR3D_ALPHA_C_X1_ou;
    rSmoothNR3D.ac_con1.bits.NR3D_ALPHA_C_X2 = NR3D_ALPHA_C_X2_ou;
    rSmoothNR3D.ac_con1.bits.NR3D_ALPHA_C_X3 = NR3D_ALPHA_C_X3_ou;
    rSmoothNR3D.ac_con1.bits.NR3D_ALPHA_C_X4 = NR3D_ALPHA_C_X4_ou;

    rSmoothNR3D.ac_con2.bits.NR3D_ALPHA_C_Y0 = NR3D_ALPHA_C_Y0_ou;
    rSmoothNR3D.ac_con2.bits.NR3D_ALPHA_C_Y1 = NR3D_ALPHA_C_Y1_ou;
    rSmoothNR3D.ac_con2.bits.NR3D_ALPHA_C_Y2 = NR3D_ALPHA_C_Y2_ou;
    rSmoothNR3D.ac_con2.bits.NR3D_ALPHA_C_Y3 = NR3D_ALPHA_C_Y3_ou;

    rSmoothNR3D.ac_con3.bits.NR3D_ALPHA_C_Y4 = NR3D_ALPHA_C_Y4_ou;

    rSmoothNR3D.ac_con4.bits.NR3D_ALPHA_C_S0 = NR3D_ALPHA_C_S0_ou;
    rSmoothNR3D.ac_con4.bits.NR3D_ALPHA_C_S1 = NR3D_ALPHA_C_S1_ou;

    rSmoothNR3D.ac_con5.bits.NR3D_ALPHA_C_S2 = NR3D_ALPHA_C_S2_ou;
    rSmoothNR3D.ac_con5.bits.NR3D_ALPHA_C_S3 = NR3D_ALPHA_C_S3_ou;

    rSmoothNR3D.b1y_con1.bits.NR3D_BETA1_Y_X1 = NR3D_BETA1_Y_X1_ou;
    rSmoothNR3D.b1y_con1.bits.NR3D_BETA1_Y_X2 = NR3D_BETA1_Y_X2_ou;
    rSmoothNR3D.b1y_con1.bits.NR3D_BETA1_Y_X3 = NR3D_BETA1_Y_X3_ou;
    rSmoothNR3D.b1y_con1.bits.NR3D_BETA1_Y_X4 = NR3D_BETA1_Y_X4_ou;

    rSmoothNR3D.b1y_con2.bits.NR3D_BETA1_Y_Y1 = NR3D_BETA1_Y_Y1_ou;
    rSmoothNR3D.b1y_con2.bits.NR3D_BETA1_Y_Y2 = NR3D_BETA1_Y_Y2_ou;
    rSmoothNR3D.b1y_con2.bits.NR3D_BETA1_Y_Y3 = NR3D_BETA1_Y_Y3_ou;
    rSmoothNR3D.b1y_con2.bits.NR3D_BETA1_Y_Y4 = NR3D_BETA1_Y_Y4_ou;

    rSmoothNR3D.b1y_con3.bits.NR3D_BETA1_Y_S1 = NR3D_BETA1_Y_S1_ou;
    rSmoothNR3D.b1y_con3.bits.NR3D_BETA1_Y_S2 = NR3D_BETA1_Y_S2_ou;
    rSmoothNR3D.b1y_con3.bits.NR3D_BETA1_Y_S3 = NR3D_BETA1_Y_S3_ou;

    rSmoothNR3D.b1c_con1.bits.NR3D_BETA1_C_X1 = NR3D_BETA1_C_X1_ou;
    rSmoothNR3D.b1c_con1.bits.NR3D_BETA1_C_X2 = NR3D_BETA1_C_X2_ou;
    rSmoothNR3D.b1c_con1.bits.NR3D_BETA1_C_X3 = NR3D_BETA1_C_X3_ou;
    rSmoothNR3D.b1c_con1.bits.NR3D_BETA1_C_X4 = NR3D_BETA1_C_X4_ou;

    rSmoothNR3D.b1c_con2.bits.NR3D_BETA1_C_Y1 = NR3D_BETA1_C_Y1_ou;
    rSmoothNR3D.b1c_con2.bits.NR3D_BETA1_C_Y2 = NR3D_BETA1_C_Y2_ou;
    rSmoothNR3D.b1c_con2.bits.NR3D_BETA1_C_Y3 = NR3D_BETA1_C_Y3_ou;
    rSmoothNR3D.b1c_con2.bits.NR3D_BETA1_C_Y4 = NR3D_BETA1_C_Y4_ou;

    rSmoothNR3D.b1c_con3.bits.NR3D_BETA1_C_S1 = NR3D_BETA1_C_S1_ou;
    rSmoothNR3D.b1c_con3.bits.NR3D_BETA1_C_S2 = NR3D_BETA1_C_S2_ou;
    rSmoothNR3D.b1c_con3.bits.NR3D_BETA1_C_S3 = NR3D_BETA1_C_S3_ou;

    rSmoothNR3D.lscg1_xcon.bits.NR3D_LSCG1_Y_X1 = NR3D_LSCG1_Y_X1_ou;
    rSmoothNR3D.lscg1_xcon.bits.NR3D_LSCG1_Y_X2 = NR3D_LSCG1_Y_X2_ou;
    rSmoothNR3D.lscg1_xcon.bits.NR3D_LSCG1_Y_X3 = NR3D_LSCG1_Y_X3_ou;
    rSmoothNR3D.lscg1_xcon.bits.NR3D_LSCG1_Y_X4 = NR3D_LSCG1_Y_X4_ou;

    rSmoothNR3D.lscg1_ycon.bits.NR3D_LSCG1_Y_Y0 = NR3D_LSCG1_Y_Y0_ou;
    rSmoothNR3D.lscg1_ycon.bits.NR3D_LSCG1_Y_Y1 = NR3D_LSCG1_Y_Y1_ou;
    rSmoothNR3D.lscg1_ycon.bits.NR3D_LSCG1_Y_Y2 = NR3D_LSCG1_Y_Y2_ou;
    rSmoothNR3D.lscg1_ycon.bits.NR3D_LSCG1_Y_Y3 = NR3D_LSCG1_Y_Y3_ou;

    rSmoothNR3D.lscg1_con.bits.NR3D_LSCG1_Y_Y4 = NR3D_LSCG1_Y_Y4_ou;
    rSmoothNR3D.lscg1_con.bits.NR3D_LSCG1_DLT_GAIN_Y = NR3D_LSCG1_DLT_GAIN_Y_ou;
    rSmoothNR3D.lscg1_con.bits.NR3D_LSCG1_DLT_GAIN_C = NR3D_LSCG1_DLT_GAIN_C_ou;

    rSmoothNR3D.lscg1_scon.bits.NR3D_LSCG1_Y_S0 = NR3D_LSCG1_Y_S0_ou;
    rSmoothNR3D.lscg1_scon.bits.NR3D_LSCG1_Y_S1 = NR3D_LSCG1_Y_S1_ou;
    rSmoothNR3D.lscg1_scon.bits.NR3D_LSCG1_Y_S2 = NR3D_LSCG1_Y_S2_ou;
    rSmoothNR3D.lscg1_scon.bits.NR3D_LSCG1_Y_S3 = NR3D_LSCG1_Y_S3_ou;

    rSmoothNR3D.nly_con1.bits.NR3D_MPNL_Y_Y0 = NR3D_MPNL_Y_Y0_ou;
    rSmoothNR3D.nly_con1.bits.NR3D_EPNL1_MAX_Y = NR3D_EPNL1_MAX_Y_ou;
    rSmoothNR3D.nly_con1.bits.NR3D_EPNL1_COR_Y = NR3D_EPNL1_COR_Y_ou;
    rSmoothNR3D.nly_con1.bits.NR3D_EPNL2_MAX_Y = NR3D_EPNL2_MAX_Y_ou;
    rSmoothNR3D.nly_con1.bits.NR3D_EPNL2_COR_Y = NR3D_EPNL2_COR_Y_ou;
    rSmoothNR3D.nly_con1.bits.NR3D_MPNL1_MAX_Y = NR3D_MPNL1_MAX_Y_ou;
    rSmoothNR3D.nly_con1.bits.NR3D_MPNL1_COR_Y = NR3D_MPNL1_COR_Y_ou;

    rSmoothNR3D.nly_con2.bits.NR3D_MPNL2_MAX_Y = NR3D_MPNL2_MAX_Y_ou;
    rSmoothNR3D.nly_con2.bits.NR3D_MPNL2_COR_Y = NR3D_MPNL2_COR_Y_ou;
    rSmoothNR3D.nly_con2.bits.NR3D_MPNL3_MAX_Y = NR3D_MPNL3_MAX_Y_ou;
    rSmoothNR3D.nly_con2.bits.NR3D_MPNL4_MAX_Y = NR3D_MPNL4_MAX_Y_ou;
    rSmoothNR3D.nly_con2.bits.NR3D_PVAI1_GAIN_Y = NR3D_PVAI1_GAIN_Y_ou;
    rSmoothNR3D.nly_con2.bits.NR3D_PVAR1_MIN_Y = NR3D_PVAR1_MIN_Y_ou;
    rSmoothNR3D.nly_con2.bits.NR3D_PVAR1_MAX_Y = NR3D_PVAR1_MAX_Y_ou;

    rSmoothNR3D.mly_con.bits.NR3D_MTLVL_COR_Y = NR3D_MTLVL_COR_Y_ou;
    rSmoothNR3D.mly_con.bits.NR3D_MTLVL_GAIN_Y = NR3D_MTLVL_GAIN_Y_ou;
    rSmoothNR3D.mly_con.bits.NR3D_MTLVL_VASEL_Y = NR3D_MTLVL_VASEL_Y_ou;
    rSmoothNR3D.mly_con.bits.NR3D_MTLVL_DFSEL_Y = NR3D_MTLVL_DFSEL_Y_ou;
    rSmoothNR3D.mly_con.bits.NR3D_DDIF2_GPSEL_Y = NR3D_DDIF2_GPSEL_Y_ou;

    rSmoothNR3D.nlc_con1.bits.NR3D_MPNL_C_Y0 = NR3D_MPNL_C_Y0_ou;
    rSmoothNR3D.nlc_con1.bits.NR3D_EPNL1_MAX_C = NR3D_EPNL1_MAX_C_ou;
    rSmoothNR3D.nlc_con1.bits.NR3D_EPNL1_COR_C = NR3D_EPNL1_COR_C_ou;
    rSmoothNR3D.nlc_con1.bits.NR3D_EPNL2_MAX_C = NR3D_EPNL2_MAX_C_ou;
    rSmoothNR3D.nlc_con1.bits.NR3D_EPNL2_COR_C = NR3D_EPNL2_COR_C_ou;
    rSmoothNR3D.nlc_con1.bits.NR3D_MPNL1_MAX_C = NR3D_MPNL1_MAX_C_ou;
    rSmoothNR3D.nlc_con1.bits.NR3D_MPNL1_COR_C = NR3D_MPNL1_COR_C_ou;

    rSmoothNR3D.nlc_con2.bits.NR3D_MPNL2_MAX_C = NR3D_MPNL2_MAX_C_ou;
    rSmoothNR3D.nlc_con2.bits.NR3D_MPNL2_COR_C = NR3D_MPNL2_COR_C_ou;
    rSmoothNR3D.nlc_con2.bits.NR3D_MPNL3_MAX_C = NR3D_MPNL3_MAX_C_ou;
    rSmoothNR3D.nlc_con2.bits.NR3D_MPNL4_MAX_C = NR3D_MPNL4_MAX_C_ou;
    rSmoothNR3D.nlc_con2.bits.NR3D_PVAI1_GAIN_C = NR3D_PVAI1_GAIN_C_ou;
    rSmoothNR3D.nlc_con2.bits.NR3D_PVAR1_MIN_C = NR3D_PVAR1_MIN_C_ou;
    rSmoothNR3D.nlc_con2.bits.NR3D_PVAR1_MAX_C = NR3D_PVAR1_MAX_C_ou;

    rSmoothNR3D.mlc_con.bits.NR3D_MTLVL_COR_C = NR3D_MTLVL_COR_C_ou;
    rSmoothNR3D.mlc_con.bits.NR3D_MTLVL_GAIN_C = NR3D_MTLVL_GAIN_C_ou;

    rSmoothNR3D.sly_con.bits.NR3D_SVLVL_M1_LS1_Y = NR3D_SVLVL_M1_LS1_Y_ou;
    rSmoothNR3D.sly_con.bits.NR3D_SVLVL_M2_LS1_Y = NR3D_SVLVL_M2_LS1_Y_ou;
    rSmoothNR3D.sly_con.bits.NR3D_STLVL_M3_LS1_Y = NR3D_STLVL_M3_LS1_Y_ou;
    rSmoothNR3D.sly_con.bits.NR3D_SVLVL_M1_ADD_Y = NR3D_SVLVL_M1_ADD_Y_ou;
    rSmoothNR3D.sly_con.bits.NR3D_SVLVL_M1_MAX_Y = NR3D_SVLVL_M1_MAX_Y_ou;
    rSmoothNR3D.sly_con.bits.NR3D_SVLVL_M2_ADD_Y = NR3D_SVLVL_M2_ADD_Y_ou;
    rSmoothNR3D.sly_con.bits.NR3D_SVLVL_M2_MAX_Y = NR3D_SVLVL_M2_MAX_Y_ou;
    rSmoothNR3D.sly_con.bits.NR3D_STLVL_M3_ADD_Y = NR3D_STLVL_M3_ADD_Y_ou;
    rSmoothNR3D.sly_con.bits.NR3D_STLVL_M3_MAX_Y = NR3D_STLVL_M3_MAX_Y_ou;

    rSmoothNR3D.slc_con.bits.NR3D_SVLVL_M1_LS1_C = NR3D_SVLVL_M1_LS1_C_ou;
    rSmoothNR3D.slc_con.bits.NR3D_SVLVL_M2_LS1_C = NR3D_SVLVL_M2_LS1_C_ou;
    rSmoothNR3D.slc_con.bits.NR3D_STLVL_M3_LS1_C = NR3D_STLVL_M3_LS1_C_ou;
    rSmoothNR3D.slc_con.bits.NR3D_SVLVL_M1_ADD_C = NR3D_SVLVL_M1_ADD_C_ou;
    rSmoothNR3D.slc_con.bits.NR3D_SVLVL_M1_MAX_C = NR3D_SVLVL_M1_MAX_C_ou;
    rSmoothNR3D.slc_con.bits.NR3D_SVLVL_M2_ADD_C = NR3D_SVLVL_M2_ADD_C_ou;
    rSmoothNR3D.slc_con.bits.NR3D_SVLVL_M2_MAX_C = NR3D_SVLVL_M2_MAX_C_ou;
    rSmoothNR3D.slc_con.bits.NR3D_STLVL_M3_ADD_C = NR3D_STLVL_M3_ADD_C_ou;
    rSmoothNR3D.slc_con.bits.NR3D_STLVL_M3_MAX_C = NR3D_STLVL_M3_MAX_C_ou;

    rSmoothNR3D.difg_con.bits.NR3D_DDIF_GAIN_Y = NR3D_DDIF_GAIN_Y_ou;
    rSmoothNR3D.difg_con.bits.NR3D_DDIF_GAIN_C = NR3D_DDIF_GAIN_C_ou;
    rSmoothNR3D.difg_con.bits.NR3D_BDIF_GAIN_Y = NR3D_BDIF_GAIN_Y_ou;
    rSmoothNR3D.difg_con.bits.NR3D_BDIF_GAIN_C = NR3D_BDIF_GAIN_C_ou;

    rSmoothNR3D.ycjc_con1.bits.NR3D_YCJC_TH1_Y = NR3D_YCJC_TH1_Y_ou;
    rSmoothNR3D.ycjc_con1.bits.NR3D_YCJC_TH3_Y = NR3D_YCJC_TH3_Y_ou;
    rSmoothNR3D.ycjc_con1.bits.NR3D_YCJC_TH1_C = NR3D_YCJC_TH1_C_ou;
    rSmoothNR3D.ycjc_con1.bits.NR3D_YCJC_TH2_C = NR3D_YCJC_TH2_C_ou;

    rSmoothNR3D.ycjc_con2.bits.NR3D_YCJC_TH3_C = NR3D_YCJC_TH3_C_ou;
    rSmoothNR3D.ycjc_con2.bits.NR3D_YCJC_GAIN_Y = NR3D_YCJC_GAIN_Y_ou;
    rSmoothNR3D.ycjc_con2.bits.NR3D_YCJC_GAIN_C = NR3D_YCJC_GAIN_C_ou;
    rSmoothNR3D.ycjc_con2.bits.NR3D_YCJC_RS_Y = NR3D_YCJC_RS_Y_ou;
    rSmoothNR3D.ycjc_con2.bits.NR3D_YCJC_RS_C = NR3D_YCJC_RS_C_ou;


    if (bSmoothNR3DDebug && (bmCount % 33) == 0)
    {
        // alpha curve y
//        MINT32 NR3D_ALPHA_Y_X1_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_ALPHA_Y_X1_lo,NR3D_ALPHA_Y_X1_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_ALPHA_Y_X1_ou    = %d, %d, %d", NR3D_ALPHA_Y_X1_lo,  NR3D_ALPHA_Y_X1_ou, NR3D_ALPHA_Y_X1_up);
//        MINT32 NR3D_ALPHA_Y_X2_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_ALPHA_Y_X2_lo,NR3D_ALPHA_Y_X2_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_ALPHA_Y_X2_ou = %d, %d, %d", NR3D_ALPHA_Y_X2_lo,  NR3D_ALPHA_Y_X2_ou, NR3D_ALPHA_Y_X2_up);
//        MINT32 NR3D_ALPHA_Y_X3_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_ALPHA_Y_X3_lo,NR3D_ALPHA_Y_X3_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_ALPHA_Y_X3_ou = %d, %d, %d", NR3D_ALPHA_Y_X3_lo,  NR3D_ALPHA_Y_X3_ou, NR3D_ALPHA_Y_X3_up);
//        MINT32 NR3D_ALPHA_Y_X4_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_ALPHA_Y_X4_lo,NR3D_ALPHA_Y_X4_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_ALPHA_Y_X4_ou = %d, %d, %d", NR3D_ALPHA_Y_X4_lo,  NR3D_ALPHA_Y_X4_ou, NR3D_ALPHA_Y_X4_up);

//        MINT32 NR3D_ALPHA_Y_Y0_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_ALPHA_Y_Y0_lo,NR3D_ALPHA_Y_Y0_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_ALPHA_Y_Y0_ou = %d, %d, %d", NR3D_ALPHA_Y_Y0_lo,  NR3D_ALPHA_Y_Y0_ou, NR3D_ALPHA_Y_Y0_up);
//        MINT32 NR3D_ALPHA_Y_Y1_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_ALPHA_Y_Y1_lo,NR3D_ALPHA_Y_Y1_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_ALPHA_Y_Y1_ou = %d, %d, %d", NR3D_ALPHA_Y_Y1_lo,  NR3D_ALPHA_Y_Y1_ou, NR3D_ALPHA_Y_Y1_up);
//        MINT32 NR3D_ALPHA_Y_Y2_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_ALPHA_Y_Y2_lo,NR3D_ALPHA_Y_Y2_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_ALPHA_Y_Y2_ou = %d, %d, %d", NR3D_ALPHA_Y_Y2_lo,  NR3D_ALPHA_Y_Y2_ou, NR3D_ALPHA_Y_Y2_up);
//        MINT32 NR3D_ALPHA_Y_Y3_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_ALPHA_Y_Y3_lo,NR3D_ALPHA_Y_Y3_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_ALPHA_Y_Y3_ou = %d, %d, %d", NR3D_ALPHA_Y_Y3_lo,  NR3D_ALPHA_Y_Y3_ou, NR3D_ALPHA_Y_Y3_up);
//        MINT32 NR3D_ALPHA_Y_Y4_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_ALPHA_Y_Y4_lo,NR3D_ALPHA_Y_Y4_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_ALPHA_Y_Y4_ou = %d, %d, %d", NR3D_ALPHA_Y_Y4_lo,  NR3D_ALPHA_Y_Y4_ou, NR3D_ALPHA_Y_Y4_up);

        // alpha curve c
//        MINT32 NR3D_ALPHA_C_X1_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_ALPHA_C_X1_lo,NR3D_ALPHA_C_X1_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_ALPHA_C_X1_ou = %d, %d, %d", NR3D_ALPHA_C_X1_lo,  NR3D_ALPHA_C_X1_ou, NR3D_ALPHA_C_X1_up);
//        MINT32 NR3D_ALPHA_C_X2_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_ALPHA_C_X2_lo,NR3D_ALPHA_C_X2_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_ALPHA_C_X2_ou = %d, %d, %d", NR3D_ALPHA_C_X2_lo,  NR3D_ALPHA_C_X2_ou, NR3D_ALPHA_C_X2_up);
//        MINT32 NR3D_ALPHA_C_X3_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_ALPHA_C_X3_lo,NR3D_ALPHA_C_X3_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_ALPHA_C_X3_ou = %d, %d, %d", NR3D_ALPHA_C_X3_lo,  NR3D_ALPHA_C_X3_ou, NR3D_ALPHA_C_X3_up);
//        MINT32 NR3D_ALPHA_C_X4_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_ALPHA_C_X4_lo,NR3D_ALPHA_C_X4_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_ALPHA_C_X4_ou = %d, %d, %d", NR3D_ALPHA_C_X4_lo,  NR3D_ALPHA_C_X4_ou, NR3D_ALPHA_C_X4_up);

//        MINT32 NR3D_ALPHA_C_Y0_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_ALPHA_C_Y0_lo,NR3D_ALPHA_C_Y0_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_ALPHA_C_Y0_ou = %d, %d, %d", NR3D_ALPHA_C_Y0_lo,  NR3D_ALPHA_C_Y0_ou, NR3D_ALPHA_C_Y0_up);
//        MINT32 NR3D_ALPHA_C_Y1_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_ALPHA_C_Y1_lo,NR3D_ALPHA_C_Y1_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_ALPHA_C_Y1_ou = %d, %d, %d", NR3D_ALPHA_C_Y1_lo,  NR3D_ALPHA_C_Y1_ou, NR3D_ALPHA_C_Y1_up);
//        MINT32 NR3D_ALPHA_C_Y2_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_ALPHA_C_Y2_lo,NR3D_ALPHA_C_Y2_lo,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_ALPHA_C_Y2_ou = %d, %d, %d", NR3D_ALPHA_C_Y2_lo,  NR3D_ALPHA_C_Y2_ou, NR3D_ALPHA_C_Y2_lo);
//        MINT32 NR3D_ALPHA_C_Y3_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_ALPHA_C_Y3_lo,NR3D_ALPHA_C_Y3_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_ALPHA_C_Y3_ou = %d, %d, %d", NR3D_ALPHA_C_Y3_lo,  NR3D_ALPHA_C_Y3_ou, NR3D_ALPHA_C_Y3_up);
//        MINT32 NR3D_ALPHA_C_Y4_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_ALPHA_C_Y4_lo,NR3D_ALPHA_C_Y4_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_ALPHA_C_Y4_ou = %d, %d, %d", NR3D_ALPHA_C_Y4_lo,  NR3D_ALPHA_C_Y4_ou, NR3D_ALPHA_C_Y4_up);

        // beta curve y
//        MINT32 NR3D_BETA1_Y_X1_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_BETA1_Y_X1_lo,NR3D_BETA1_Y_X1_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_BETA1_Y_X1_ou = %d, %d, %d", NR3D_BETA1_Y_X1_lo,  NR3D_BETA1_Y_X1_ou, NR3D_BETA1_Y_X1_up);
//        MINT32 NR3D_BETA1_Y_X2_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_BETA1_Y_X2_lo,NR3D_BETA1_Y_X2_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_BETA1_Y_X2_ou = %d, %d, %d", NR3D_BETA1_Y_X2_lo,  NR3D_BETA1_Y_X2_ou, NR3D_BETA1_Y_X2_up);
//        MINT32 NR3D_BETA1_Y_X3_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_BETA1_Y_X3_lo,NR3D_BETA1_Y_X3_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_BETA1_Y_X3_ou = %d, %d, %d", NR3D_BETA1_Y_X3_lo,  NR3D_BETA1_Y_X3_ou, NR3D_BETA1_Y_X3_up);
//        MINT32 NR3D_BETA1_Y_X4_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_BETA1_Y_X4_lo,NR3D_BETA1_Y_X4_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_BETA1_Y_X4_ou = %d, %d, %d", NR3D_BETA1_Y_X4_lo,  NR3D_BETA1_Y_X4_ou, NR3D_BETA1_Y_X4_up);

//        MINT32 NR3D_BETA1_Y_Y1_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_BETA1_Y_Y1_lo,NR3D_BETA1_Y_Y1_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_BETA1_Y_Y1_ou = %d, %d, %d", NR3D_BETA1_Y_Y1_lo,  NR3D_BETA1_Y_Y1_ou, NR3D_BETA1_Y_Y1_up);
//        MINT32 NR3D_BETA1_Y_Y2_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_BETA1_Y_Y2_lo,NR3D_BETA1_Y_Y2_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_BETA1_Y_Y2_ou = %d, %d, %d", NR3D_BETA1_Y_Y2_lo,  NR3D_BETA1_Y_Y2_ou, NR3D_BETA1_Y_Y2_up);
//        MINT32 NR3D_BETA1_Y_Y3_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_BETA1_Y_Y3_lo,NR3D_BETA1_Y_Y3_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_BETA1_Y_Y3_ou = %d, %d, %d", NR3D_BETA1_Y_Y3_lo,  NR3D_BETA1_Y_Y3_ou, NR3D_BETA1_Y_Y3_up);
//        MINT32 NR3D_BETA1_Y_Y4_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_BETA1_Y_Y4_lo,NR3D_BETA1_Y_Y4_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_BETA1_Y_Y4_ou = %d, %d, %d", NR3D_BETA1_Y_Y4_lo,  NR3D_BETA1_Y_Y4_ou, NR3D_BETA1_Y_Y4_up);

        // beta curve c
//        MINT32 NR3D_BETA1_C_X1_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_BETA1_C_X1_lo,NR3D_BETA1_C_X1_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_BETA1_C_X1_ou = %d, %d, %d", NR3D_BETA1_C_X1_lo,  NR3D_BETA1_C_X1_ou, NR3D_BETA1_C_X1_up);
//        MINT32 NR3D_BETA1_C_X2_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_BETA1_C_X2_lo,NR3D_BETA1_C_X2_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_BETA1_C_X2_ou = %d, %d, %d", NR3D_BETA1_C_X2_lo,  NR3D_BETA1_C_X2_ou, NR3D_BETA1_C_X2_up);
//        MINT32 NR3D_BETA1_C_X3_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_BETA1_C_X3_lo,NR3D_BETA1_C_X3_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_BETA1_C_X3_ou = %d, %d, %d", NR3D_BETA1_C_X3_lo,  NR3D_BETA1_C_X3_ou, NR3D_BETA1_C_X3_up);
//        MINT32 NR3D_BETA1_C_X4_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_BETA1_C_X4_lo,NR3D_BETA1_C_X4_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_BETA1_C_X4_ou = %d, %d, %d", NR3D_BETA1_C_X4_lo,  NR3D_BETA1_C_X4_ou, NR3D_BETA1_C_X4_up);

        MINT32 NR3D_BETA1_C_Y1_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_BETA1_C_Y1_lo,NR3D_BETA1_C_Y1_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_BETA1_C_Y1_ou = %d, %d, %d", NR3D_BETA1_C_Y1_lo,  NR3D_BETA1_C_Y1_ou, NR3D_BETA1_C_Y1_up);
        MINT32 NR3D_BETA1_C_Y2_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_BETA1_C_Y2_lo,NR3D_BETA1_C_Y2_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_BETA1_C_Y2_ou = %d, %d, %d", NR3D_BETA1_C_Y2_lo,  NR3D_BETA1_C_Y2_ou, NR3D_BETA1_C_Y2_up);
        MINT32 NR3D_BETA1_C_Y3_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_BETA1_C_Y3_lo,NR3D_BETA1_C_Y3_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_BETA1_C_Y3_ou = %d, %d, %d", NR3D_BETA1_C_Y3_lo,  NR3D_BETA1_C_Y3_ou, NR3D_BETA1_C_Y3_up);
        MINT32 NR3D_BETA1_C_Y4_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_BETA1_C_Y4_lo,NR3D_BETA1_C_Y4_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_BETA1_C_Y4_ou = %d, %d, %d", NR3D_BETA1_C_Y4_lo,  NR3D_BETA1_C_Y4_ou, NR3D_BETA1_C_Y4_up);

        // LSCG1 (beta2) curve y
//        MINT32 NR3D_LSCG1_Y_X1_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_LSCG1_Y_X1_lo,NR3D_LSCG1_Y_X1_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_LSCG1_Y_X1_ou = %d, %d, %d", NR3D_LSCG1_Y_X1_lo,  NR3D_LSCG1_Y_X1_ou, NR3D_LSCG1_Y_X1_up);
//        MINT32 NR3D_LSCG1_Y_X2_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_LSCG1_Y_X2_lo,NR3D_LSCG1_Y_X2_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_LSCG1_Y_X2_ou = %d, %d, %d", NR3D_LSCG1_Y_X2_lo,  NR3D_LSCG1_Y_X2_ou, NR3D_LSCG1_Y_X2_up);
//        MINT32 NR3D_LSCG1_Y_X3_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_LSCG1_Y_X3_lo,NR3D_LSCG1_Y_X3_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_LSCG1_Y_X3_ou = %d, %d, %d", NR3D_LSCG1_Y_X3_lo,  NR3D_LSCG1_Y_X3_ou, NR3D_LSCG1_Y_X3_up);
//        MINT32 NR3D_LSCG1_Y_X4_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_LSCG1_Y_X4_lo,NR3D_LSCG1_Y_X4_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_LSCG1_Y_X4_ou = %d, %d, %d", NR3D_LSCG1_Y_X4_lo,  NR3D_LSCG1_Y_X4_ou, NR3D_LSCG1_Y_X4_up);

//        MINT32 NR3D_LSCG1_Y_Y0_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_LSCG1_Y_Y0_lo,NR3D_LSCG1_Y_Y0_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_LSCG1_Y_Y0_ou = %d, %d, %d", NR3D_LSCG1_Y_Y0_lo,  NR3D_LSCG1_Y_Y0_ou, NR3D_LSCG1_Y_Y0_up);
//        MINT32 NR3D_LSCG1_Y_Y1_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_LSCG1_Y_Y1_lo,NR3D_LSCG1_Y_Y1_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_LSCG1_Y_Y1_ou = %d, %d, %d", NR3D_LSCG1_Y_Y1_lo,  NR3D_LSCG1_Y_Y1_ou, NR3D_LSCG1_Y_Y1_up);
//        MINT32 NR3D_LSCG1_Y_Y2_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_LSCG1_Y_Y2_lo,NR3D_LSCG1_Y_Y2_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_LSCG1_Y_Y2_ou = %d, %d, %d", NR3D_LSCG1_Y_Y2_lo,  NR3D_LSCG1_Y_Y2_ou, NR3D_LSCG1_Y_Y2_up);
//        MINT32 NR3D_LSCG1_Y_Y3_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_LSCG1_Y_Y3_lo,NR3D_LSCG1_Y_Y3_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_LSCG1_Y_Y3_ou = %d, %d, %d", NR3D_LSCG1_Y_Y3_lo,  NR3D_LSCG1_Y_Y3_ou, NR3D_LSCG1_Y_Y3_up);
//        MINT32 NR3D_LSCG1_Y_Y4_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_LSCG1_Y_Y4_lo,NR3D_LSCG1_Y_Y4_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_LSCG1_Y_Y4_ou = %d, %d, %d", NR3D_LSCG1_Y_Y4_lo,  NR3D_LSCG1_Y_Y4_ou, NR3D_LSCG1_Y_Y4_up);

//        MINT32 NR3D_LSCG1_DLT_GAIN_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_LSCG1_DLT_GAIN_Y_lo,NR3D_LSCG1_DLT_GAIN_Y_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_LSCG1_DLT_GAIN_Y_ou = %d, %d, %d", NR3D_LSCG1_DLT_GAIN_Y_lo,  NR3D_LSCG1_DLT_GAIN_Y_ou, NR3D_LSCG1_DLT_GAIN_Y_up);
//        MINT32 NR3D_LSCG1_DLT_GAIN_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_LSCG1_DLT_GAIN_C_lo,NR3D_LSCG1_DLT_GAIN_C_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_LSCG1_DLT_GAIN_C_ou = %d, %d, %d", NR3D_LSCG1_DLT_GAIN_C_lo,  NR3D_LSCG1_DLT_GAIN_C_ou, NR3D_LSCG1_DLT_GAIN_C_up);

//        MINT32 NR3D_MPNL_Y_Y0_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_MPNL_Y_Y0_lo,NR3D_MPNL_Y_Y0_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_MPNL_Y_Y0_ou = %d, %d, %d", NR3D_MPNL_Y_Y0_lo,  NR3D_MPNL_Y_Y0_ou, NR3D_MPNL_Y_Y0_up);
//        MINT32 NR3D_EPNL1_MAX_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_EPNL1_MAX_Y_lo,NR3D_EPNL1_MAX_Y_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_EPNL1_MAX_Y_ou = %d, %d, %d", NR3D_EPNL1_MAX_Y_lo,  NR3D_EPNL1_MAX_Y_ou, NR3D_EPNL1_MAX_Y_up);
//        MINT32 NR3D_EPNL1_COR_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_EPNL1_COR_Y_lo,NR3D_EPNL1_COR_Y_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_EPNL1_COR_Y_ou = %d, %d, %d", NR3D_EPNL1_COR_Y_lo,  NR3D_EPNL1_COR_Y_ou, NR3D_EPNL1_COR_Y_up);
//        MINT32 NR3D_EPNL2_MAX_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_EPNL2_MAX_Y_lo,NR3D_EPNL2_MAX_Y_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_EPNL2_MAX_Y_ou = %d, %d, %d", NR3D_EPNL2_MAX_Y_lo,  NR3D_EPNL2_MAX_Y_ou, NR3D_EPNL2_MAX_Y_up);
//        MINT32 NR3D_EPNL2_COR_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_EPNL2_COR_Y_lo,NR3D_EPNL2_COR_Y_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_EPNL2_COR_Y_ou = %d, %d, %d", NR3D_EPNL2_COR_Y_lo,  NR3D_EPNL2_COR_Y_ou, NR3D_EPNL2_COR_Y_up);
//        MINT32 NR3D_MPNL1_MAX_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_MPNL1_MAX_Y_lo,NR3D_MPNL1_MAX_Y_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_MPNL1_MAX_Y_ou = %d, %d, %d", NR3D_MPNL1_MAX_Y_lo,  NR3D_MPNL1_MAX_Y_ou, NR3D_MPNL1_MAX_Y_up);
//        MINT32 NR3D_MPNL1_COR_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_MPNL1_COR_Y_lo,NR3D_MPNL1_COR_Y_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_MPNL1_COR_Y_ou = %d, %d, %d", NR3D_MPNL1_COR_Y_lo,  NR3D_MPNL1_COR_Y_ou, NR3D_MPNL1_COR_Y_up);

        MINT32 NR3D_PVAI1_GAIN_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_PVAI1_GAIN_Y_lo,NR3D_PVAI1_GAIN_Y_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_PVAI1_GAIN_Y_ou = %d, %d, %d", NR3D_PVAI1_GAIN_Y_lo,  NR3D_PVAI1_GAIN_Y_ou, NR3D_PVAI1_GAIN_Y_up);
        MINT32 NR3D_MPNL2_MAX_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_MPNL2_MAX_Y_lo,NR3D_MPNL2_MAX_Y_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_MPNL2_MAX_Y_ou = %d, %d, %d", NR3D_MPNL2_MAX_Y_lo,  NR3D_MPNL2_MAX_Y_ou, NR3D_MPNL2_MAX_Y_up);
        MINT32 NR3D_MPNL2_COR_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_MPNL2_COR_Y_lo,NR3D_MPNL2_COR_Y_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_MPNL2_COR_Y_ou = %d, %d, %d", NR3D_MPNL2_COR_Y_lo,  NR3D_MPNL2_COR_Y_ou, NR3D_MPNL2_COR_Y_up);
        MINT32 NR3D_MPNL3_MAX_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_MPNL3_MAX_Y_lo,NR3D_MPNL3_MAX_Y_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_MPNL3_MAX_Y_ou = %d, %d, %d", NR3D_MPNL3_MAX_Y_lo,  NR3D_MPNL3_MAX_Y_ou, NR3D_MPNL3_MAX_Y_up);
        MINT32 NR3D_MPNL4_MAX_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_MPNL4_MAX_Y_lo,NR3D_MPNL4_MAX_Y_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_MPNL4_MAX_Y_ou = %d, %d, %d", NR3D_MPNL4_MAX_Y_lo,  NR3D_MPNL4_MAX_Y_ou, NR3D_MPNL4_MAX_Y_up);
        MINT32 NR3D_PVAR1_MIN_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_PVAR1_MIN_Y_lo,NR3D_PVAR1_MIN_Y_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_PVAR1_MIN_Y_ou = %d, %d, %d", NR3D_PVAR1_MIN_Y_lo,  NR3D_PVAR1_MIN_Y_ou, NR3D_PVAR1_MIN_Y_up);
        MINT32 NR3D_PVAR1_MAX_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_PVAR1_MAX_Y_lo,NR3D_PVAR1_MAX_Y_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_PVAR1_MAX_Y_ou = %d, %d, %d", NR3D_PVAR1_MAX_Y_lo,  NR3D_PVAR1_MAX_Y_ou, NR3D_PVAR1_MAX_Y_up);

//        MINT32 NR3D_MTLVL_COR_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_MTLVL_COR_Y_lo,NR3D_MTLVL_COR_Y_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_MTLVL_COR_Y_ou = %d, %d, %d", NR3D_MTLVL_COR_Y_lo,  NR3D_MTLVL_COR_Y_ou, NR3D_MTLVL_COR_Y_up);
//        MINT32 NR3D_MTLVL_GAIN_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_MTLVL_GAIN_Y_lo,NR3D_MTLVL_GAIN_Y_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_MTLVL_GAIN_Y_ou = %d, %d, %d", NR3D_MTLVL_GAIN_Y_lo,  NR3D_MTLVL_GAIN_Y_ou, NR3D_MTLVL_GAIN_Y_up);
//        MINT32 NR3D_MTLVL_VASEL_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_MTLVL_VASEL_Y_lo,NR3D_MTLVL_VASEL_Y_up,1);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_MTLVL_VASEL_Y_ou = %d, %d, %d", NR3D_MTLVL_VASEL_Y_lo,  NR3D_MTLVL_VASEL_Y_ou, NR3D_MTLVL_VASEL_Y_up);
//        MINT32 NR3D_MTLVL_DFSEL_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_MTLVL_DFSEL_Y_lo,NR3D_MTLVL_DFSEL_Y_up,1);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_MTLVL_DFSEL_Y_ou = %d, %d, %d", NR3D_MTLVL_DFSEL_Y_lo,  NR3D_MTLVL_DFSEL_Y_ou, NR3D_MTLVL_DFSEL_Y_up);
//        MINT32 NR3D_DDIF2_GPSEL_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_DDIF2_GPSEL_Y_lo,NR3D_DDIF2_GPSEL_Y_up,1);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_DDIF2_GPSEL_Y_ou = %d, %d, %d", NR3D_DDIF2_GPSEL_Y_lo,  NR3D_DDIF2_GPSEL_Y_ou, NR3D_DDIF2_GPSEL_Y_up);

//        MINT32 NR3D_MPNL_C_Y0_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_MPNL_C_Y0_lo,NR3D_MPNL_C_Y0_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_MPNL_C_Y0_ou = %d, %d, %d", NR3D_MPNL_C_Y0_lo,  NR3D_MPNL_C_Y0_ou, NR3D_MPNL_C_Y0_up);
//        MINT32 NR3D_EPNL1_MAX_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_EPNL1_MAX_C_lo,NR3D_EPNL1_MAX_C_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_EPNL1_MAX_C_ou = %d, %d, %d", NR3D_EPNL1_MAX_C_lo,  NR3D_EPNL1_MAX_C_ou, NR3D_EPNL1_MAX_C_up);
//        MINT32 NR3D_EPNL1_COR_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_EPNL1_COR_C_lo,NR3D_EPNL1_COR_C_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_EPNL1_COR_C_ou = %d, %d, %d", NR3D_EPNL1_COR_C_lo,  NR3D_EPNL1_COR_C_ou, NR3D_EPNL1_COR_C_up);
//        MINT32 NR3D_EPNL2_MAX_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_EPNL2_MAX_C_lo,NR3D_EPNL2_MAX_C_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_EPNL2_MAX_C_ou = %d, %d, %d", NR3D_EPNL2_MAX_C_lo,  NR3D_EPNL2_MAX_C_ou, NR3D_EPNL2_MAX_C_up);
//        MINT32 NR3D_EPNL2_COR_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_EPNL2_COR_C_lo,NR3D_EPNL2_COR_C_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_EPNL2_COR_C_ou = %d, %d, %d", NR3D_EPNL2_COR_C_lo,  NR3D_EPNL2_COR_C_ou, NR3D_EPNL2_COR_C_up);
//        MINT32 NR3D_MPNL1_MAX_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_MPNL1_MAX_C_lo,NR3D_MPNL1_MAX_C_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_MPNL1_MAX_C_ou = %d, %d, %d", NR3D_MPNL1_MAX_C_lo,  NR3D_MPNL1_MAX_C_ou, NR3D_MPNL1_MAX_C_up);
//        MINT32 NR3D_MPNL1_COR_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_MPNL1_COR_C_lo,NR3D_MPNL1_COR_C_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_MPNL1_COR_C_ou = %d, %d, %d", NR3D_MPNL1_COR_C_lo,  NR3D_MPNL1_COR_C_ou, NR3D_MPNL1_COR_C_up);

        MINT32 NR3D_PVAI1_GAIN_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_PVAI1_GAIN_C_lo,NR3D_PVAI1_GAIN_C_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_PVAI1_GAIN_C_ou = %d, %d, %d", NR3D_PVAI1_GAIN_C_lo,  NR3D_PVAI1_GAIN_C_ou, NR3D_PVAI1_GAIN_C_up);
        MINT32 NR3D_MPNL2_MAX_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_MPNL2_MAX_C_lo,NR3D_MPNL2_MAX_C_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_MPNL2_MAX_C_ou = %d, %d, %d", NR3D_MPNL2_MAX_C_lo,  NR3D_MPNL2_MAX_C_ou, NR3D_MPNL2_MAX_C_up);
        MINT32 NR3D_MPNL2_COR_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_MPNL2_COR_C_lo,NR3D_MPNL2_COR_C_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_MPNL2_COR_C_ou = %d, %d, %d", NR3D_MPNL2_COR_C_lo,  NR3D_MPNL2_COR_C_ou, NR3D_MPNL2_COR_C_up);
        MINT32 NR3D_MPNL3_MAX_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_MPNL3_MAX_C_lo,NR3D_MPNL3_MAX_C_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_MPNL3_MAX_C_ou = %d, %d, %d", NR3D_MPNL3_MAX_C_lo,  NR3D_MPNL3_MAX_C_ou, NR3D_MPNL3_MAX_C_up);
        MINT32 NR3D_MPNL4_MAX_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_MPNL4_MAX_C_lo,NR3D_MPNL4_MAX_C_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_MPNL4_MAX_C_ou = %d, %d, %d", NR3D_MPNL4_MAX_C_lo,  NR3D_MPNL4_MAX_C_ou, NR3D_MPNL4_MAX_C_up);
        MINT32 NR3D_PVAR1_MIN_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_PVAR1_MIN_C_lo,NR3D_PVAR1_MIN_C_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_PVAR1_MIN_C_ou = %d, %d, %d", NR3D_PVAR1_MIN_C_lo,  NR3D_PVAR1_MIN_C_ou, NR3D_PVAR1_MIN_C_up);
        MINT32 NR3D_PVAR1_MAX_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_PVAR1_MAX_C_lo,NR3D_PVAR1_MAX_C_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_PVAR1_MAX_C_ou = %d, %d, %d", NR3D_PVAR1_MAX_C_lo,  NR3D_PVAR1_MAX_C_ou, NR3D_PVAR1_MAX_C_up);

        MINT32 NR3D_MTLVL_COR_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_MTLVL_COR_C_lo,NR3D_MTLVL_COR_C_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_MTLVL_COR_C_ou = %d, %d, %d", NR3D_MTLVL_COR_C_lo,  NR3D_MTLVL_COR_C_ou, NR3D_MTLVL_COR_C_up);
        MINT32 NR3D_MTLVL_GAIN_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_MTLVL_GAIN_C_lo,NR3D_MTLVL_GAIN_C_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_MTLVL_GAIN_C_ou = %d, %d, %d", NR3D_MTLVL_GAIN_C_lo,  NR3D_MTLVL_GAIN_C_ou, NR3D_MTLVL_GAIN_C_up);

        MINT32 NR3D_SVLVL_M1_LS1_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_SVLVL_M1_LS1_Y_lo,NR3D_SVLVL_M1_LS1_Y_up,1);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_SVLVL_M1_LS1_Y_ou = %d, %d, %d", NR3D_SVLVL_M1_LS1_Y_lo,  NR3D_SVLVL_M1_LS1_Y_ou, NR3D_SVLVL_M1_LS1_Y_up);

//        MINT32 NR3D_SVLVL_M2_LS1_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_SVLVL_M2_LS1_Y_lo,NR3D_SVLVL_M2_LS1_Y_up,1);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_SVLVL_M2_LS1_Y_ou = %d, %d, %d", NR3D_SVLVL_M2_LS1_Y_lo,  NR3D_SVLVL_M2_LS1_Y_ou, NR3D_SVLVL_M2_LS1_Y_up);
//        MINT32 NR3D_STLVL_M3_LS1_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_STLVL_M3_LS1_Y_lo,NR3D_STLVL_M3_LS1_Y_up,1);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_STLVL_M3_LS1_Y_ou = %d, %d, %d", NR3D_STLVL_M3_LS1_Y_lo,  NR3D_STLVL_M3_LS1_Y_ou, NR3D_STLVL_M3_LS1_Y_up);
//        MINT32 NR3D_SVLVL_M1_ADD_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_SVLVL_M1_ADD_Y_lo,NR3D_SVLVL_M1_ADD_Y_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_SVLVL_M1_ADD_Y_ou = %d, %d, %d", NR3D_SVLVL_M1_ADD_Y_lo,  NR3D_SVLVL_M1_ADD_Y_ou, NR3D_SVLVL_M1_ADD_Y_up);
//        MINT32 NR3D_SVLVL_M1_MAX_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_SVLVL_M1_MAX_Y_lo,NR3D_SVLVL_M1_MAX_Y_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_SVLVL_M1_MAX_Y_ou = %d, %d, %d", NR3D_SVLVL_M1_MAX_Y_lo,  NR3D_SVLVL_M1_MAX_Y_ou, NR3D_SVLVL_M1_MAX_Y_up);
//        MINT32 NR3D_SVLVL_M2_ADD_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_SVLVL_M2_ADD_Y_lo,NR3D_SVLVL_M2_ADD_Y_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_SVLVL_M2_ADD_Y_ou = %d, %d, %d", NR3D_SVLVL_M2_ADD_Y_lo,  NR3D_SVLVL_M2_ADD_Y_ou, NR3D_SVLVL_M2_ADD_Y_up);
//        MINT32 NR3D_SVLVL_M2_MAX_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_SVLVL_M2_MAX_Y_lo,NR3D_SVLVL_M2_MAX_Y_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_SVLVL_M2_MAX_Y_ou = %d, %d, %d", NR3D_SVLVL_M2_MAX_Y_lo,  NR3D_SVLVL_M2_MAX_Y_ou, NR3D_SVLVL_M2_MAX_Y_up);
//        MINT32 NR3D_STLVL_M3_ADD_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_STLVL_M3_ADD_Y_lo,NR3D_STLVL_M3_ADD_Y_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_STLVL_M3_ADD_Y_ou = %d, %d, %d", NR3D_STLVL_M3_ADD_Y_lo,  NR3D_STLVL_M3_ADD_Y_ou, NR3D_STLVL_M3_ADD_Y_up);
//        MINT32 NR3D_STLVL_M3_MAX_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_STLVL_M3_MAX_Y_lo,NR3D_STLVL_M3_MAX_Y_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_STLVL_M3_MAX_Y_ou = %d, %d, %d", NR3D_STLVL_M3_MAX_Y_lo,  NR3D_STLVL_M3_MAX_Y_ou, NR3D_STLVL_M3_MAX_Y_up);

//        MINT32 NR3D_SVLVL_M1_LS1_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_SVLVL_M1_LS1_C_lo,NR3D_SVLVL_M1_LS1_C_up,1);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_SVLVL_M1_LS1_C_ou = %d, %d, %d", NR3D_SVLVL_M1_LS1_C_lo,  NR3D_SVLVL_M1_LS1_C_ou, NR3D_SVLVL_M1_LS1_C_up);
//        MINT32 NR3D_SVLVL_M2_LS1_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_SVLVL_M2_LS1_C_lo,NR3D_SVLVL_M2_LS1_C_up,1);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_SVLVL_M2_LS1_C_ou = %d, %d, %d", NR3D_SVLVL_M2_LS1_C_lo,  NR3D_SVLVL_M2_LS1_C_ou, NR3D_SVLVL_M2_LS1_C_up);
//        MINT32 NR3D_STLVL_M3_LS1_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_STLVL_M3_LS1_C_lo,NR3D_STLVL_M3_LS1_C_up,1);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_STLVL_M3_LS1_C_ou = %d, %d, %d", NR3D_STLVL_M3_LS1_C_lo,  NR3D_STLVL_M3_LS1_C_ou, NR3D_STLVL_M3_LS1_C_up);
//        MINT32 NR3D_SVLVL_M1_ADD_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_SVLVL_M1_ADD_C_ou,NR3D_SVLVL_M1_ADD_C_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_SVLVL_M1_ADD_C_ou = %d, %d, %d", NR3D_SVLVL_M1_ADD_C_lo,  NR3D_SVLVL_M1_ADD_C_ou, NR3D_SVLVL_M1_ADD_C_up);
//        MINT32 NR3D_SVLVL_M1_MAX_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_SVLVL_M1_MAX_C_lo,NR3D_SVLVL_M1_MAX_C_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_SVLVL_M1_MAX_C_ou = %d, %d, %d", NR3D_SVLVL_M1_MAX_C_lo,  NR3D_SVLVL_M1_MAX_C_ou, NR3D_SVLVL_M1_MAX_C_up);
//        MINT32 NR3D_SVLVL_M2_ADD_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_SVLVL_M2_ADD_C_lo,NR3D_SVLVL_M2_ADD_C_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_SVLVL_M2_ADD_C_ou = %d, %d, %d", NR3D_SVLVL_M2_ADD_C_lo,  NR3D_SVLVL_M2_ADD_C_ou, NR3D_SVLVL_M2_ADD_C_up);
//        MINT32 NR3D_SVLVL_M2_MAX_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_SVLVL_M2_MAX_C_lo,NR3D_SVLVL_M2_MAX_C_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_SVLVL_M2_MAX_C_ou = %d, %d, %d", NR3D_SVLVL_M2_MAX_C_lo,  NR3D_SVLVL_M2_MAX_C_ou, NR3D_SVLVL_M2_MAX_C_up);
//        MINT32 NR3D_STLVL_M3_ADD_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_STLVL_M3_ADD_C_lo,NR3D_STLVL_M3_ADD_C_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_STLVL_M3_ADD_C_ou = %d, %d, %d", NR3D_STLVL_M3_ADD_C_lo,  NR3D_STLVL_M3_ADD_C_ou, NR3D_STLVL_M3_ADD_C_up);
//        MINT32 NR3D_STLVL_M3_MAX_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_STLVL_M3_MAX_C_lo,NR3D_STLVL_M3_MAX_C_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_STLVL_M3_MAX_C_ou = %d, %d, %d", NR3D_STLVL_M3_MAX_C_lo,  NR3D_STLVL_M3_MAX_C_ou, NR3D_STLVL_M3_MAX_C_up);
//        MINT32 NR3D_DDIF_GAIN_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_DDIF_GAIN_Y_lo,NR3D_DDIF_GAIN_Y_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_DDIF_GAIN_Y_ou = %d, %d, %d", NR3D_DDIF_GAIN_Y_lo,  NR3D_DDIF_GAIN_Y_ou, NR3D_DDIF_GAIN_Y_up);
//        MINT32 NR3D_DDIF_GAIN_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_DDIF_GAIN_C_lo,NR3D_DDIF_GAIN_C_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_DDIF_GAIN_C_ou = %d, %d, %d", NR3D_DDIF_GAIN_C_lo,  NR3D_DDIF_GAIN_C_ou, NR3D_DDIF_GAIN_C_up);
//        MINT32 NR3D_BDIF_GAIN_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_BDIF_GAIN_Y_lo,NR3D_BDIF_GAIN_Y_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_BDIF_GAIN_Y_ou = %d, %d, %d", NR3D_BDIF_GAIN_Y_lo,  NR3D_BDIF_GAIN_Y_ou, NR3D_BDIF_GAIN_Y_up);
//        MINT32 NR3D_BDIF_GAIN_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_BDIF_GAIN_C_lo,NR3D_BDIF_GAIN_C_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_BDIF_GAIN_C_ou = %d, %d, %d", NR3D_BDIF_GAIN_C_lo,  NR3D_BDIF_GAIN_C_ou, NR3D_BDIF_GAIN_C_up);

//        MINT32 NR3D_YCJC_TH1_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_YCJC_TH1_Y_lo,NR3D_YCJC_TH1_Y_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_YCJC_TH1_Y_ou = %d, %d, %d", NR3D_YCJC_TH1_Y_lo,  NR3D_YCJC_TH1_Y_ou, NR3D_YCJC_TH1_Y_up);
//        MINT32 NR3D_YCJC_TH3_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_YCJC_TH3_Y_lo,NR3D_YCJC_TH3_Y_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_YCJC_TH3_Y_ou = %d, %d, %d", NR3D_YCJC_TH3_Y_lo,  NR3D_YCJC_TH3_Y_ou, NR3D_YCJC_TH3_Y_up);
//        MINT32 NR3D_YCJC_TH1_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_YCJC_TH1_C_lo,NR3D_YCJC_TH1_C_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_YCJC_TH1_C_ou = %d, %d, %d", NR3D_YCJC_TH1_C_lo,  NR3D_YCJC_TH1_C_ou, NR3D_YCJC_TH1_C_up);
//        MINT32 NR3D_YCJC_TH2_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_YCJC_TH2_C_lo,NR3D_YCJC_TH2_C_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_YCJC_TH2_C_ou = %d, %d, %d", NR3D_YCJC_TH2_C_lo,  NR3D_YCJC_TH2_C_ou, NR3D_YCJC_TH2_C_up);

//        MINT32 NR3D_YCJC_TH3_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_YCJC_TH3_C_lo,NR3D_YCJC_TH3_C_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_YCJC_TH3_C_ou = %d, %d, %d", NR3D_YCJC_TH3_C_lo,  NR3D_YCJC_TH3_C_ou, NR3D_YCJC_TH3_C_up);
//        MINT32 NR3D_YCJC_GAIN_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_YCJC_GAIN_Y_lo,NR3D_YCJC_GAIN_Y_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_YCJC_GAIN_Y_ou = %d, %d, %d", NR3D_YCJC_GAIN_Y_lo,  NR3D_YCJC_GAIN_Y_ou, NR3D_YCJC_GAIN_Y_up);
//        MINT32 NR3D_YCJC_GAIN_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_YCJC_GAIN_C_lo,NR3D_YCJC_GAIN_C_up,0);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_YCJC_GAIN_C_ou = %d, %d, %d", NR3D_YCJC_GAIN_C_lo,  NR3D_YCJC_GAIN_C_ou, NR3D_YCJC_GAIN_C_up);
//        MINT32 NR3D_YCJC_RS_Y_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_YCJC_RS_Y_lo,NR3D_YCJC_RS_Y_up,1);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_YCJC_RS_Y_ou = %d, %d, %d", NR3D_YCJC_RS_Y_lo,  NR3D_YCJC_RS_Y_ou, NR3D_YCJC_RS_Y_up);
//        MINT32 NR3D_YCJC_RS_C_ou = InterParam_NR3D(u4RealISO,u4LowerISO,u4UpperISO,NR3D_YCJC_RS_C_lo,NR3D_YCJC_RS_C_up,1);
        CAM_LOGW_IF(bSmoothNR3DDebug, "[L,S,U] NR3D_YCJC_RS_C_ou = %d, %d, %d", NR3D_YCJC_RS_C_lo,  NR3D_YCJC_RS_C_ou, NR3D_YCJC_RS_C_up);
    }
}
#endif

// ===========================================================================================


MVOID SmoothNR(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                NR_PARAM const& rUpper, // settings for upper ISO
                NR_PARAM const& rLower,   // settings for lower ISO
                NR_PARAM& rSmooth) // Output
{
#define Inter(item, method)                                                         \
    do{                                                                               \
        rSmooth.item = InterParam_NR(                                                 \
                u4RealISO,                                                            \
                u4LowerISO,                                                           \
                u4UpperISO,                                                           \
                rLower.item,                                                          \
                rUpper.item,                                                          \
                method);                                                              \
    } while(0)
    //
    Inter(ANR_Y_LUMA_SCALE_RANGE , 0);
    Inter(ANR_C_CHROMA_SCALE     , 0);
    Inter(ANR_Y_SCALE_CPY0       , 0);
    Inter(ANR_Y_SCALE_CPY1       , 0);
    Inter(ANR_Y_SCALE_CPY2       , 0);
    Inter(ANR_Y_SCALE_CPY3       , 0);
    Inter(ANR_Y_SCALE_CPY4       , 0);
    Inter(ANR_Y_CPX1             , 0);
    Inter(ANR_Y_CPX2             , 0);
    Inter(ANR_Y_CPX3             , 0);
    Inter(ANR_CEN_GAIN_LO_TH     , 0);
    Inter(ANR_CEN_GAIN_HI_TH     , 0);
    Inter(ANR_PTY_GAIN_TH        , 0);
    Inter(ANR_KSIZE_LO_TH        , 0);
    Inter(ANR_KSIZE_HI_TH        , 0);
    Inter(ANR_KSIZE_LO_TH_C      , 0);
    Inter(ANR_KSIZE_HI_TH_C      , 0);
    Inter(ITUNE_ANR_PTY_STD      , 0);
    Inter(ITUNE_ANR_PTU_STD      , 0);
    Inter(ITUNE_ANR_PTV_STD      , 0);
    Inter(ANR_ACT_TH_Y           , 0);
    Inter(ANR_ACT_BLD_BASE_Y     , 0);
    Inter(ANR_ACT_BLD_TH_Y       , 0);
    Inter(ANR_ACT_SLANT_Y        , 0);
    Inter(ANR_ACT_TH_C           , 0);
    Inter(ANR_ACT_BLD_BASE_C     , 0);
    Inter(ANR_ACT_BLD_TH_C       , 0);
    Inter(ANR_ACT_SLANT_C        , 0);
    Inter(RADIUS_H               , 0);
    Inter(RADIUS_V               , 0);
    Inter(RADIUS_H_C             , 0);
    Inter(RADIUS_V_C             , 0);
    Inter(ANR_PTC_HGAIN          , 0);
    Inter(ANR_PTY_HGAIN          , 0);
    Inter(ANR_LPF_HALFKERNEL     , 0);
    Inter(ANR_LPF_HALFKERNEL_C   , 0);
    Inter(ANR_ACT_MODE           , 1);
    Inter(ANR_LCE_SCALE_GAIN     , 0);
    Inter(ANR_LCE_C_GAIN         , 0);
    Inter(ANR_LCE_GAIN0          , 0);
    Inter(ANR_LCE_GAIN1          , 0);
    Inter(ANR_LCE_GAIN2          , 0);
    Inter(ANR_LCE_GAIN3          , 0);
    Inter(ANR_MEDIAN_LOCATION    , 0);
    Inter(ANR_CEN_X              , 0);
    Inter(ANR_CEN_Y              , 0);
    Inter(ANR_R1                 , 0);
    Inter(ANR_R2                 , 0);
    Inter(ANR_R3                 , 0);
    Inter(LUMA_ON_OFF            , 0);
    // debug
    MBOOL bSmoothDebug = property_get_bool("debug.smooth_swnr.enable", false);
    if( bSmoothDebug )
    {
        MY_LOG("[%s()] u4LowerISO: %d, u4RealISO = %d, u4UpperISO = %d\n",
                __FUNCTION__, u4LowerISO, u4RealISO, u4UpperISO);
#define DumpParam(item)                                                               \
        do{                                                                           \
            MY_LOG("%s = %d, %d, %d", #item, rLower.item, rSmooth.item, rUpper.item); \
        }while(0)
        DumpParam(ANR_Y_LUMA_SCALE_RANGE);
        DumpParam(ANR_C_CHROMA_SCALE);
        DumpParam(ANR_Y_SCALE_CPY0);
        DumpParam(ANR_Y_SCALE_CPY1);
        DumpParam(ANR_Y_SCALE_CPY2);
        DumpParam(ANR_Y_SCALE_CPY3);
        DumpParam(ANR_Y_SCALE_CPY4);
        DumpParam(ANR_Y_CPX1);
        DumpParam(ANR_Y_CPX2);
        DumpParam(ANR_Y_CPX3);
        DumpParam(ANR_CEN_GAIN_LO_TH);
        DumpParam(ANR_CEN_GAIN_HI_TH);
        DumpParam(ANR_PTY_GAIN_TH);
        DumpParam(ANR_KSIZE_LO_TH);
        DumpParam(ANR_KSIZE_HI_TH);
        DumpParam(ANR_KSIZE_LO_TH_C);
        DumpParam(ANR_KSIZE_HI_TH_C);
        DumpParam(ITUNE_ANR_PTY_STD);
        DumpParam(ITUNE_ANR_PTU_STD);
        DumpParam(ITUNE_ANR_PTV_STD);
        DumpParam(ANR_ACT_TH_Y);
        DumpParam(ANR_ACT_BLD_BASE_Y);
        DumpParam(ANR_ACT_BLD_TH_Y);
        DumpParam(ANR_ACT_SLANT_Y);
        DumpParam(ANR_ACT_TH_C);
        DumpParam(ANR_ACT_BLD_BASE_C);
        DumpParam(ANR_ACT_BLD_TH_C);
        DumpParam(ANR_ACT_SLANT_C);
        DumpParam(RADIUS_H);
        DumpParam(RADIUS_V);
        DumpParam(RADIUS_H_C);
        DumpParam(RADIUS_V_C);
        DumpParam(ANR_PTC_HGAIN);
        DumpParam(ANR_PTY_HGAIN);
        DumpParam(ANR_LPF_HALFKERNEL);
        DumpParam(ANR_LPF_HALFKERNEL_C);
        DumpParam(ANR_ACT_MODE);
        DumpParam(ANR_LCE_SCALE_GAIN);
        DumpParam(ANR_LCE_C_GAIN);
        DumpParam(ANR_LCE_GAIN0);
        DumpParam(ANR_LCE_GAIN1);
        DumpParam(ANR_LCE_GAIN2);
        DumpParam(ANR_LCE_GAIN3);
        DumpParam(ANR_MEDIAN_LOCATION);
        DumpParam(ANR_CEN_X);
        DumpParam(ANR_CEN_Y);
        DumpParam(ANR_R1);
        DumpParam(ANR_R2);
        DumpParam(ANR_R3);
        DumpParam(LUMA_ON_OFF);
#undef DumpParam
    }
#undef Inter
}

MVOID SmoothNR2(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                NR2_PARAM const& rUpper, // settings for upper ISO
                NR2_PARAM const& rLower,   // settings for lower ISO
                NR2_PARAM& rSmooth) // Output
{
#define Inter(item, method)                                                           \
    do{                                                                               \
        rSmooth.item = InterParam_NR(                                                 \
                u4RealISO,                                                            \
                u4LowerISO,                                                           \
                u4UpperISO,                                                           \
                rLower.item,                                                          \
                rUpper.item,                                                          \
                method);                                                              \
    } while(0)
    //
    Inter(NR_K      , 0);
    Inter(NR_S      , 0);
    Inter(NR_SD     , 0);
    Inter(NR_BLD_W  , 0);
    Inter(NR_BLD_TH , 0);
    Inter(NR_SMTH   , 0);
#undef Inter
    MBOOL bSmoothDebug = property_get_bool("debug.smooth_swnr.enable", false);
    if( bSmoothDebug )
    {
        MY_LOG("[%s()] u4LowerISO: %d, u4RealISO = %d, u4UpperISO = %d\n",
                __FUNCTION__, u4LowerISO, u4RealISO, u4UpperISO);
#define DumpParam(item)                                                               \
        do{                                                                           \
            MY_LOG("%s = %d, %d, %d", #item, rLower.item, rSmooth.item, rUpper.item); \
        }while(0)
        DumpParam(NR_K);
        DumpParam(NR_S);
        DumpParam(NR_SD);
        DumpParam(NR_BLD_W);
        DumpParam(NR_BLD_TH);
        DumpParam(NR_SMTH);
#undef DumpParam
    }
}

MVOID SmoothSWHFG(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                HFG_PARAM const& rUpper, // settings for upper ISO
                HFG_PARAM const& rLower,   // settings for lower ISO
                HFG_PARAM& rSmooth) // Output
{
#define Inter(item, method)                                                           \
    do{                                                                               \
        rSmooth.item = InterParam_NR(                                                 \
                u4RealISO,                                                            \
                u4LowerISO,                                                           \
                u4UpperISO,                                                           \
                rLower.item,                                                          \
                rUpper.item,                                                          \
                method);                                                              \
    } while(0)
    MINT32 HFG_inter_method = 0;
    if (rLower.HFG_ENABLE != rUpper.HFG_ENABLE) // HFG_ENABLE
        HFG_inter_method = 1;

    Inter(HFG_ENABLE      , HFG_inter_method);
    Inter(HFG_SD0         , HFG_inter_method);
    Inter(HFG_SD1         , HFG_inter_method);
    Inter(HFG_SD2         , HFG_inter_method);
    Inter(HFG_TX_S        , HFG_inter_method);
    Inter(HFG_LCE_LINK_EN , HFG_inter_method);
    Inter(HFG_LUMA_CPX1   , HFG_inter_method);
    Inter(HFG_LUMA_CPX2   , HFG_inter_method);
    Inter(HFG_LUMA_CPX3   , HFG_inter_method);
    Inter(HFG_LUMA_CPY0   , HFG_inter_method);
    Inter(HFG_LUMA_CPY1   , HFG_inter_method);
    Inter(HFG_LUMA_CPY2   , HFG_inter_method);
    Inter(HFG_LUMA_CPY3   , HFG_inter_method);
    Inter(HFG_LUMA_SP0    , HFG_inter_method);
    Inter(HFG_LUMA_SP1    , HFG_inter_method);
    Inter(HFG_LUMA_SP2    , HFG_inter_method);
    Inter(HFG_LUMA_SP3    , HFG_inter_method);
#undef Inter
    MBOOL bSmoothDebug = property_get_bool("debug.smooth_swnr.enable", false);
    if( bSmoothDebug )
    {
        MY_LOG("[%s()] u4LowerISO: %d, u4RealISO = %d, u4UpperISO = %d\n",
                __FUNCTION__, u4LowerISO, u4RealISO, u4UpperISO);
#define DumpParam(item)                                                               \
        do{                                                                           \
            MY_LOG("%s = %d, %d, %d", #item, rLower.item, rSmooth.item, rUpper.item); \
        }while(0)
        DumpParam(HFG_ENABLE);
        DumpParam(HFG_GSD);
        DumpParam(HFG_SD0);
        DumpParam(HFG_SD1);
        DumpParam(HFG_SD2);
        DumpParam(HFG_TX_S);
        DumpParam(HFG_LCE_LINK_EN);
        DumpParam(HFG_LUMA_CPX1);
        DumpParam(HFG_LUMA_CPX2);
        DumpParam(HFG_LUMA_CPX3);
        DumpParam(HFG_LUMA_CPY0);
        DumpParam(HFG_LUMA_CPY1);
        DumpParam(HFG_LUMA_CPY2);
        DumpParam(HFG_LUMA_CPY3);
        DumpParam(HFG_LUMA_SP0);
        DumpParam(HFG_LUMA_SP1);
        DumpParam(HFG_LUMA_SP2);
        DumpParam(HFG_LUMA_SP3);
#undef DumpParam
    }
}

MVOID SmoothSWCCR(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                CCR_PARAM const& rUpper, // settings for upper ISO
                CCR_PARAM const& rLower,   // settings for lower ISO
                CCR_PARAM& rSmooth) // Output
{
#define Inter(item, method)                                                           \
    do{                                                                               \
        rSmooth.item = InterParam_NR(                                                 \
                u4RealISO,                                                            \
                u4LowerISO,                                                           \
                u4UpperISO,                                                           \
                rLower.item,                                                          \
                rUpper.item,                                                          \
                method);                                                              \
    } while(0)
    //
    // SWCCR part
    MINT32 CCR_inter_method = 0;
    if (rLower.CCR_ENABLE != rUpper.CCR_ENABLE)
        CCR_inter_method = 1;

    Inter(CCR_ENABLE             , CCR_inter_method);
    Inter(CCR_CEN_U              , CCR_inter_method);
    Inter(CCR_CEN_V              , CCR_inter_method);
    Inter(CCR_Y_CPX1             , CCR_inter_method);
    Inter(CCR_Y_CPX2             , CCR_inter_method);
    Inter(CCR_Y_CPY1             , CCR_inter_method);
    Inter(CCR_UV_X1              , CCR_inter_method);
    Inter(CCR_UV_X2              , CCR_inter_method);
    Inter(CCR_UV_X3              , CCR_inter_method);
    Inter(CCR_UV_GAIN1           , CCR_inter_method);
    Inter(CCR_UV_GAIN2           , CCR_inter_method);
    Inter(CCR_Y_CPX3             , CCR_inter_method);
    Inter(CCR_Y_CPY0             , CCR_inter_method);
    Inter(CCR_Y_CPY2             , CCR_inter_method);
    Inter(CCR_UV_GAIN_MODE       , CCR_inter_method);
    Inter(CCR_MODE               , CCR_inter_method);
    Inter(CCR_OR_MODE            , CCR_inter_method);
    Inter(CCR_HUE_X1             , CCR_inter_method);
    Inter(CCR_HUE_X2             , CCR_inter_method);
    Inter(CCR_HUE_X3             , CCR_inter_method);
    Inter(CCR_HUE_X4             , CCR_inter_method);
    Inter(CCR_HUE_GAIN1          , CCR_inter_method);
    Inter(CCR_HUE_GAIN2          , CCR_inter_method);
    Inter(CCR_Y_SP2              , CCR_inter_method);
    Inter(CCR_UV_GAIN_SP2        , CCR_inter_method);

    //    Slope parameters update
    MINT32 CCR_Y_SP0;
    MINT32 CCR_Y_SP1;
    MINT32 CCR_UV_GAIN_SP1;
    MINT32 CCR_HUE_SP1;
    MINT32 CCR_HUE_SP2;

    //CCR_Y_SP0
    if(rSmooth.CCR_Y_CPX1 == 0)
        CCR_Y_SP0 = 0;
    else
    {
        MINT32 Y_SP0 = (MINT32)(32 * (double)(rSmooth.CCR_Y_CPY1-rSmooth.CCR_Y_CPY0)/(double)rSmooth.CCR_Y_CPX1+0.5);
        CCR_Y_SP0 = (Y_SP0>127)?127:Y_SP0;
    }

    //CCR_Y_SP1
    if(rSmooth.CCR_Y_CPX2 == 0)
        CCR_Y_SP1 = 0;
    else
    {
        MINT32 Y_SP1 = (MINT32)(32 * (double)(rSmooth.CCR_Y_CPY2-rSmooth.CCR_Y_CPY1)/(double)(rSmooth.CCR_Y_CPX2 - rSmooth.CCR_Y_CPX1)+0.5);
        CCR_Y_SP1 = (Y_SP1>127)?127:Y_SP1;
    }

    //CCR_UV_GAIN_SP1
    if(rSmooth.CCR_UV_X2 == rSmooth.CCR_UV_X1)
        CCR_UV_GAIN_SP1 = 0;
    else
    {
        MINT32 UV_SP1 = (MINT32)(32 * (double)(rSmooth.CCR_UV_GAIN1-rSmooth.CCR_UV_GAIN2)/(double)(rSmooth.CCR_UV_X2-rSmooth.CCR_UV_X1)+0.5);
        CCR_UV_GAIN_SP1 = (UV_SP1>1023)?1023:UV_SP1;
    }

    //CCR_HUE_SP1
    if(rSmooth.CCR_HUE_X1 == rSmooth.CCR_HUE_X2)
        CCR_HUE_SP1 = (rSmooth.CCR_HUE_GAIN1>rSmooth.CCR_HUE_GAIN2)?-128:127;
    else
    {
        MINT32 HUE_SP1 = (MINT32)(32 * (double)(rSmooth.CCR_HUE_GAIN2-rSmooth.CCR_HUE_GAIN1)/(double)(rSmooth.CCR_HUE_X2-rSmooth.CCR_HUE_X1));
        CCR_HUE_SP1 = (HUE_SP1>127)?127:(HUE_SP1<-128)?-128:HUE_SP1;
    }

    //CCR_HUE_SP2
    if(rSmooth.CCR_HUE_X3 == rSmooth.CCR_HUE_X4)
        CCR_HUE_SP2 = (rSmooth.CCR_HUE_GAIN1>rSmooth.CCR_HUE_GAIN2)?127:-128;
    else
    {
        MINT32 HUE_SP2 = (MINT32)(32 * (double)(rSmooth.CCR_HUE_GAIN1-rSmooth.CCR_HUE_GAIN2)/(double)(rSmooth.CCR_HUE_X4-rSmooth.CCR_HUE_X3));
        CCR_HUE_SP2 = (HUE_SP2>127)?127:(HUE_SP2<-128)?-128:HUE_SP2;
    }

    rSmooth.CCR_Y_SP0       = CCR_Y_SP0;
    rSmooth.CCR_Y_SP1       = CCR_Y_SP1;
    rSmooth.CCR_UV_GAIN_SP1 = CCR_UV_GAIN_SP1;
    rSmooth.CCR_HUE_SP1     = CCR_HUE_SP1;
    rSmooth.CCR_HUE_SP2     = CCR_HUE_SP2;
#undef Inter
    MBOOL bSmoothDebug = property_get_bool("debug.smooth_swnr.enable", false);
    if( bSmoothDebug )
    {
        MY_LOG("[%s()] u4LowerISO: %d, u4RealISO = %d, u4UpperISO = %d\n",
                __FUNCTION__, u4LowerISO, u4RealISO, u4UpperISO);
#define DumpParam(item)                                                               \
        do{                                                                           \
            MY_LOG("%s = %d, %d, %d", #item, rLower.item, rSmooth.item, rUpper.item); \
        }while(0)
        DumpParam(CCR_ENABLE);
        DumpParam(CCR_CEN_U);
        DumpParam(CCR_CEN_V);
        DumpParam(CCR_Y_CPX1);
        DumpParam(CCR_Y_CPX2);
        DumpParam(CCR_Y_CPY1);
        DumpParam(CCR_Y_SP1);
        DumpParam(CCR_UV_X1);
        DumpParam(CCR_UV_X2);
        DumpParam(CCR_UV_X3);
        DumpParam(CCR_UV_GAIN1);
        DumpParam(CCR_UV_GAIN2);
        DumpParam(CCR_UV_GAIN_SP1);
        DumpParam(CCR_UV_GAIN_SP2);
        DumpParam(CCR_Y_CPX3);
        DumpParam(CCR_Y_CPY0);
        DumpParam(CCR_Y_CPY2);
        DumpParam(CCR_Y_SP0);
        DumpParam(CCR_Y_SP2);
        DumpParam(CCR_UV_GAIN_MODE);
        DumpParam(CCR_MODE);
        DumpParam(CCR_OR_MODE);
        DumpParam(CCR_HUE_X1);
        DumpParam(CCR_HUE_X2);
        DumpParam(CCR_HUE_X3);
        DumpParam(CCR_HUE_X4);
        DumpParam(CCR_HUE_SP1);
        DumpParam(CCR_HUE_SP2);
        DumpParam(CCR_HUE_GAIN1);
        DumpParam(CCR_HUE_GAIN2);
#undef DumpParam
    }
}

MVOID SmoothSWNR(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                NVRAM_CAMERA_FEATURE_SWNR_STRUCT const& rUpper, // settings for upper ISO
                NVRAM_CAMERA_FEATURE_SWNR_STRUCT const& rLower,   // settings for lower ISO
                NVRAM_CAMERA_FEATURE_SWNR_STRUCT& rSmooth) // Output
{
    SmoothNR(u4RealISO, u4UpperISO, u4LowerISO,
            rUpper.swnr.NR, rLower.swnr.NR, rSmooth.swnr.NR);
    SmoothSWHFG(u4RealISO, u4UpperISO, u4LowerISO,
            rUpper.swnr.HFG, rLower.swnr.HFG, rSmooth.swnr.HFG);
    SmoothSWCCR(u4RealISO, u4UpperISO, u4LowerISO,
            rUpper.swnr.CCR, rLower.swnr.CCR, rSmooth.swnr.CCR);
}

MVOID SmoothFSWNR(MUINT32 u4RealISO, // Real ISO
               MUINT32 u4UpperISO, // Upper ISO
               MUINT32 u4LowerISO, // Lower ISO
               NVRAM_CAMERA_FEATURE_SWNR_STRUCT const& rUpper, // settings for upper ISO
               NVRAM_CAMERA_FEATURE_SWNR_STRUCT const& rLower,   // settings for lower ISO
               NVRAM_CAMERA_FEATURE_SWNR_STRUCT& rSmooth)  // Output
{
    SmoothNR2(u4RealISO, u4UpperISO, u4LowerISO,
            rUpper.fswnr.NR, rLower.fswnr.NR, rSmooth.fswnr.NR);
    SmoothSWHFG(u4RealISO, u4UpperISO, u4LowerISO,
            rUpper.fswnr.HFG, rLower.fswnr.HFG, rSmooth.fswnr.HFG);
    SmoothSWCCR(u4RealISO, u4UpperISO, u4LowerISO,
            rUpper.fswnr.CCR, rLower.fswnr.CCR, rSmooth.fswnr.CCR);
}
