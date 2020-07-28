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

#define INTER_LOGD(fmt, arg...) ALOGD(fmt, ##arg)
#define INTER_LOGW(fmt, arg...) ALOGW(fmt, ##arg)
#define INTER_LOGE(fmt, arg...) ALOGE(fmt, ##arg)

#define INTER_LOG_IF(cond, ...)      do { if ( (cond) ) { INTER_LOGD(__VA_ARGS__); } }while(0)
#define INTER_LOGW_IF(cond, ...)      do { if ( (cond) ) { INTER_LOGW(__VA_ARGS__); } }while(0)
#define INTER_LOGE_IF(cond, ...)      do { if ( (cond) ) { INTER_LOGE(__VA_ARGS__); } }while(0)


//  "return value": Interpolated register value
//  "ISO":          ISO value of current image
//  "ISOpre":       A pre-defined ISO (100, 200, 400, ...) who is the cloeset but smaller than or equal to "ISO"
//  "ISOpos":       A pre-defined ISO (100, 200, 400, ...) who is the cloeset but larger than or equal to "ISO"
//  "PARAMpre":     Corresponding register value from "ISOpre"
//  "PARAMpos":     Corresponding register value from "ISOpos"
//  "Method:        0 for linear interpolation, 1 for closest one

static MBOOL s_bPerFramePropEnable = MFALSE;

MVOID interpSetPerFramePropEnable(MBOOL bEnable)
{
    s_bPerFramePropEnable = bEnable;
}

static MBOOL interpPropertyGet(const char * pProperty, MBOOL bDefault)
{
    return (s_bPerFramePropEnable) ? (::property_get_bool(pProperty, bDefault)) : (bDefault);
}

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
    MBOOL bSmoothDBSDebug = interpPropertyGet("debug.smooth_dbs.enable", MFALSE);

    INTER_LOG_IF(bSmoothDBSDebug,"[%s()] u4RealISO: %d, u4UpperISO = %d, u4LowerISO = %d\n", __FUNCTION__, u4RealISO, u4UpperISO, u4LowerISO);

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
	MINT32 pre_DBS_EDGE       = rLowerDBS.ctl.bits.DBS_EDGE;
    MINT32 pre_DBS_MUL_B      = rLowerDBS.sigma_2.bits.DBS_MUL_B;
    MINT32 pre_DBS_MUL_GB     = rLowerDBS.sigma_2.bits.DBS_MUL_GB;
    MINT32 pre_DBS_MUL_GR     = rLowerDBS.sigma_2.bits.DBS_MUL_GR;
    MINT32 pre_DBS_MUL_R      = rLowerDBS.sigma_2.bits.DBS_MUL_R;
	MINT32 pre_DBS_YGN_B      = rLowerDBS.ygn.bits.DBS_YGN_B;
	MINT32 pre_DBS_YGN_GB      = rLowerDBS.ygn.bits.DBS_YGN_GB;
	MINT32 pre_DBS_YGN_GR      = rLowerDBS.ygn.bits.DBS_YGN_GR;
	MINT32 pre_DBS_YGN_R      = rLowerDBS.ygn.bits.DBS_YGN_R;
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
	MINT32 pos_DBS_EDGE       = rUpperDBS.ctl.bits.DBS_EDGE;
    MINT32 pos_DBS_MUL_B      = rUpperDBS.sigma_2.bits.DBS_MUL_B;
    MINT32 pos_DBS_MUL_GB     = rUpperDBS.sigma_2.bits.DBS_MUL_GB;
    MINT32 pos_DBS_MUL_GR     = rUpperDBS.sigma_2.bits.DBS_MUL_GR;
    MINT32 pos_DBS_MUL_R      = rUpperDBS.sigma_2.bits.DBS_MUL_R;
	MINT32 pos_DBS_YGN_B      = rUpperDBS.ygn.bits.DBS_YGN_B;
	MINT32 pos_DBS_YGN_GB      = rUpperDBS.ygn.bits.DBS_YGN_GB;
	MINT32 pos_DBS_YGN_GR      = rUpperDBS.ygn.bits.DBS_YGN_GR;
	MINT32 pos_DBS_YGN_R      = rUpperDBS.ygn.bits.DBS_YGN_R;
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
	MINT32 DBS_EDGE       = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_EDGE, pos_DBS_EDGE, 0);
    MINT32 DBS_MUL_B      = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_MUL_B, pos_DBS_MUL_B, 0);
    MINT32 DBS_MUL_GB     = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_MUL_GB, pos_DBS_MUL_GB, 0);
    MINT32 DBS_MUL_GR     = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_MUL_GR, pos_DBS_MUL_GR, 0);
    MINT32 DBS_MUL_R      = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_MUL_R, pos_DBS_MUL_R, 0);
	MINT32 DBS_YGN_B      = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_YGN_B, pos_DBS_YGN_B, 0);
	MINT32 DBS_YGN_GB      = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_YGN_GB, pos_DBS_YGN_GB, 0);
	MINT32 DBS_YGN_GR      = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_YGN_GR, pos_DBS_YGN_GR, 0);
	MINT32 DBS_YGN_R      = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_YGN_R, pos_DBS_YGN_R, 0);
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
	rSmoothDBS.ctl.bits.DBS_EDGE        = DBS_EDGE;
    rSmoothDBS.sigma_2.bits.DBS_MUL_B     = DBS_MUL_B;
    rSmoothDBS.sigma_2.bits.DBS_MUL_GB    = DBS_MUL_GB;
    rSmoothDBS.sigma_2.bits.DBS_MUL_GR    = DBS_MUL_GR;
    rSmoothDBS.sigma_2.bits.DBS_MUL_R     = DBS_MUL_R;
	rSmoothDBS.ygn.bits.DBS_YGN_B     = DBS_YGN_B;
	rSmoothDBS.ygn.bits.DBS_YGN_GB     = DBS_YGN_GB;
	rSmoothDBS.ygn.bits.DBS_YGN_GR     = DBS_YGN_GR;
	rSmoothDBS.ygn.bits.DBS_YGN_R     = DBS_YGN_R;
    rSmoothDBS.sl_y12.bits.DBS_SL_Y1     = DBS_SL_Y1;
    rSmoothDBS.sl_y12.bits.DBS_SL_Y2     = DBS_SL_Y2;
    rSmoothDBS.sl_y34.bits.DBS_SL_Y3     = DBS_SL_Y3;
    rSmoothDBS.sl_y34.bits.DBS_SL_Y4     = DBS_SL_Y4;
    rSmoothDBS.sl_g12.bits.DBS_SL_G1     = DBS_SL_G1;
    rSmoothDBS.sl_g12.bits.DBS_SL_G2     = DBS_SL_G2;
    rSmoothDBS.sl_g34.bits.DBS_SL_G3     = DBS_SL_G3;
    rSmoothDBS.sl_g34.bits.DBS_SL_G4     = DBS_SL_G4;

    INTER_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_OFST       = %d, %d, %d", rLowerDBS.sigma.bits.DBS_OFST, rSmoothDBS.sigma.bits.DBS_OFST, rUpperDBS.sigma.bits.DBS_OFST);
    INTER_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_SL         = %d, %d, %d", rLowerDBS.sigma.bits.DBS_SL, rSmoothDBS.sigma.bits.DBS_SL, rUpperDBS.sigma.bits.DBS_SL);
    INTER_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_BIAS_Y0    = %d, %d, %d", rLowerDBS.bstbl_0.bits.DBS_BIAS_Y0, rSmoothDBS.bstbl_0.bits.DBS_BIAS_Y0, rUpperDBS.bstbl_0.bits.DBS_BIAS_Y0);
    INTER_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_BIAS_Y1    = %d, %d, %d", rLowerDBS.bstbl_0.bits.DBS_BIAS_Y1, rSmoothDBS.bstbl_0.bits.DBS_BIAS_Y1, rUpperDBS.bstbl_0.bits.DBS_BIAS_Y1);
    INTER_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_BIAS_Y2    = %d, %d, %d", rLowerDBS.bstbl_0.bits.DBS_BIAS_Y2, rSmoothDBS.bstbl_0.bits.DBS_BIAS_Y2, rUpperDBS.bstbl_0.bits.DBS_BIAS_Y2);
    INTER_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_BIAS_Y3    = %d, %d, %d", rLowerDBS.bstbl_0.bits.DBS_BIAS_Y3, rSmoothDBS.bstbl_0.bits.DBS_BIAS_Y3, rUpperDBS.bstbl_0.bits.DBS_BIAS_Y3);
    INTER_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_BIAS_Y4    = %d, %d, %d", rLowerDBS.bstbl_1.bits.DBS_BIAS_Y4, rSmoothDBS.bstbl_1.bits.DBS_BIAS_Y4, rUpperDBS.bstbl_1.bits.DBS_BIAS_Y4);
    INTER_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_BIAS_Y5    = %d, %d, %d", rLowerDBS.bstbl_1.bits.DBS_BIAS_Y5, rSmoothDBS.bstbl_1.bits.DBS_BIAS_Y5, rUpperDBS.bstbl_1.bits.DBS_BIAS_Y5);
    INTER_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_BIAS_Y6    = %d, %d, %d", rLowerDBS.bstbl_1.bits.DBS_BIAS_Y6, rSmoothDBS.bstbl_1.bits.DBS_BIAS_Y6, rUpperDBS.bstbl_1.bits.DBS_BIAS_Y6);
    INTER_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_BIAS_Y7    = %d, %d, %d", rLowerDBS.bstbl_1.bits.DBS_BIAS_Y7, rSmoothDBS.bstbl_1.bits.DBS_BIAS_Y7, rUpperDBS.bstbl_1.bits.DBS_BIAS_Y7);
    INTER_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_BIAS_Y8    = %d, %d, %d", rLowerDBS.bstbl_2.bits.DBS_BIAS_Y8, rSmoothDBS.bstbl_2.bits.DBS_BIAS_Y8, rUpperDBS.bstbl_2.bits.DBS_BIAS_Y8);
    INTER_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_BIAS_Y9    = %d, %d, %d", rLowerDBS.bstbl_2.bits.DBS_BIAS_Y9, rSmoothDBS.bstbl_2.bits.DBS_BIAS_Y9, rUpperDBS.bstbl_2.bits.DBS_BIAS_Y9);
    INTER_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_BIAS_Y10   = %d, %d, %d", rLowerDBS.bstbl_2.bits.DBS_BIAS_Y10, rSmoothDBS.bstbl_2.bits.DBS_BIAS_Y10, rUpperDBS.bstbl_2.bits.DBS_BIAS_Y10);
    INTER_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_BIAS_Y11   = %d, %d, %d", rLowerDBS.bstbl_2.bits.DBS_BIAS_Y11, rSmoothDBS.bstbl_2.bits.DBS_BIAS_Y11, rUpperDBS.bstbl_2.bits.DBS_BIAS_Y11);
    INTER_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_BIAS_Y12   = %d, %d, %d", rLowerDBS.bstbl_3.bits.DBS_BIAS_Y12, rSmoothDBS.bstbl_3.bits.DBS_BIAS_Y12, rUpperDBS.bstbl_3.bits.DBS_BIAS_Y12);
    INTER_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_BIAS_Y13   = %d, %d, %d", rLowerDBS.bstbl_3.bits.DBS_BIAS_Y13, rSmoothDBS.bstbl_3.bits.DBS_BIAS_Y13, rUpperDBS.bstbl_3.bits.DBS_BIAS_Y13);
    INTER_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_BIAS_Y14   = %d, %d, %d", rLowerDBS.bstbl_3.bits.DBS_BIAS_Y14, rSmoothDBS.bstbl_3.bits.DBS_BIAS_Y14, rUpperDBS.bstbl_3.bits.DBS_BIAS_Y14);
    INTER_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_HDR_GAIN   = %d, %d, %d", rLowerDBS.ctl.bits.DBS_HDR_GAIN, rSmoothDBS.ctl.bits.DBS_HDR_GAIN, rUpperDBS.ctl.bits.DBS_HDR_GAIN);
    INTER_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_HDR_GAIN2  = %d, %d, %d", rLowerDBS.ctl.bits.DBS_HDR_GAIN2, rSmoothDBS.ctl.bits.DBS_HDR_GAIN2, rUpperDBS.ctl.bits.DBS_HDR_GAIN2);
    INTER_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_HDR_GNP    = %d, %d, %d", rLowerDBS.ctl.bits.DBS_HDR_GNP, rSmoothDBS.ctl.bits.DBS_HDR_GNP, rUpperDBS.ctl.bits.DBS_HDR_GNP);
    INTER_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_LE_INV_CTL = %d, %d, %d", rLowerDBS.ctl.bits.DBS_LE_INV_CTL, rSmoothDBS.ctl.bits.DBS_LE_INV_CTL, rUpperDBS.ctl.bits.DBS_LE_INV_CTL);
    INTER_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_SL_EN      = %d, %d, %d", rLowerDBS.ctl.bits.DBS_SL_EN, rSmoothDBS.ctl.bits.DBS_SL_EN, rUpperDBS.ctl.bits.DBS_SL_EN);
	INTER_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_EDGE       = %d, %d, %d", rLowerDBS.ctl.bits.DBS_EDGE, rSmoothDBS.ctl.bits.DBS_EDGE, rUpperDBS.ctl.bits.DBS_EDGE);
    INTER_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_MUL_B      = %d, %d, %d", rLowerDBS.sigma_2.bits.DBS_MUL_B, rSmoothDBS.sigma_2.bits.DBS_MUL_B, rUpperDBS.sigma_2.bits.DBS_MUL_B);
    INTER_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_MUL_GB     = %d, %d, %d", rLowerDBS.sigma_2.bits.DBS_MUL_GB, rSmoothDBS.sigma_2.bits.DBS_MUL_GB, rUpperDBS.sigma_2.bits.DBS_MUL_GB);
    INTER_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_MUL_GR     = %d, %d, %d", rLowerDBS.sigma_2.bits.DBS_MUL_GR, rSmoothDBS.sigma_2.bits.DBS_MUL_GR, rUpperDBS.sigma_2.bits.DBS_MUL_GR);
    INTER_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_MUL_R      = %d, %d, %d", rLowerDBS.sigma_2.bits.DBS_MUL_R, rSmoothDBS.sigma_2.bits.DBS_MUL_R, rUpperDBS.sigma_2.bits.DBS_MUL_R);
	INTER_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_YGN_B      = %d, %d, %d", rLowerDBS.ygn.bits.DBS_YGN_B, rSmoothDBS.ygn.bits.DBS_YGN_B, rUpperDBS.ygn.bits.DBS_YGN_B);
	INTER_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_YGN_GB      = %d, %d, %d", rLowerDBS.ygn.bits.DBS_YGN_GB, rSmoothDBS.ygn.bits.DBS_YGN_GB, rUpperDBS.ygn.bits.DBS_YGN_GB);
	INTER_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_YGN_GR      = %d, %d, %d", rLowerDBS.ygn.bits.DBS_YGN_GR, rSmoothDBS.ygn.bits.DBS_YGN_GR, rUpperDBS.ygn.bits.DBS_YGN_GR);
	INTER_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_YGN_R      = %d, %d, %d", rLowerDBS.ygn.bits.DBS_YGN_R, rSmoothDBS.ygn.bits.DBS_YGN_R, rUpperDBS.ygn.bits.DBS_YGN_R);
    INTER_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_SL_Y1      = %d, %d, %d", rLowerDBS.sl_y12.bits.DBS_SL_Y1, rSmoothDBS.sl_y12.bits.DBS_SL_Y1, rUpperDBS.sl_y12.bits.DBS_SL_Y1);
    INTER_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_SL_Y2      = %d, %d, %d", rLowerDBS.sl_y12.bits.DBS_SL_Y2, rSmoothDBS.sl_y12.bits.DBS_SL_Y2, rUpperDBS.sl_y12.bits.DBS_SL_Y2);
    INTER_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_SL_Y3      = %d, %d, %d", rLowerDBS.sl_y34.bits.DBS_SL_Y3, rSmoothDBS.sl_y34.bits.DBS_SL_Y3, rUpperDBS.sl_y34.bits.DBS_SL_Y3);
    INTER_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_SL_Y4      = %d, %d, %d", rLowerDBS.sl_y34.bits.DBS_SL_Y4, rSmoothDBS.sl_y34.bits.DBS_SL_Y4, rUpperDBS.sl_y34.bits.DBS_SL_Y4);
    INTER_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_SL_G1      = %d, %d, %d", rLowerDBS.sl_g12.bits.DBS_SL_G1, rSmoothDBS.sl_g12.bits.DBS_SL_G1, rUpperDBS.sl_g12.bits.DBS_SL_G1);
    INTER_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_SL_G2      = %d, %d, %d", rLowerDBS.sl_g12.bits.DBS_SL_G2, rSmoothDBS.sl_g12.bits.DBS_SL_G2, rUpperDBS.sl_g12.bits.DBS_SL_G2);
    INTER_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_SL_G3      = %d, %d, %d", rLowerDBS.sl_g34.bits.DBS_SL_G3, rSmoothDBS.sl_g34.bits.DBS_SL_G3, rUpperDBS.sl_g34.bits.DBS_SL_G3);
    INTER_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_SL_G4      = %d, %d, %d", rLowerDBS.sl_g34.bits.DBS_SL_G4, rSmoothDBS.sl_g34.bits.DBS_SL_G4, rUpperDBS.sl_g34.bits.DBS_SL_G4);
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
    MBOOL bSmoothOBCDebug = interpPropertyGet("debug.smooth_obc.enable", MFALSE);

    INTER_LOG_IF(bSmoothOBCDebug,"[%s()] u4RealISO: %d, u4UpperISO = %d, u4LowerISO = %d\n", __FUNCTION__, u4RealISO, u4UpperISO, u4LowerISO);

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

    INTER_LOG_IF(bSmoothOBCDebug, "[L, S, U]OBC_OFST_B  = %d, %d, %d", rLowerOBC.offst0.bits.OBC_OFST_B, rLowerOBC.offst0.bits.OBC_OFST_B, rUpperOBC.offst0.bits.OBC_OFST_B);
    INTER_LOG_IF(bSmoothOBCDebug, "[L, S, U]OBC_OFST_GR = %d, %d, %d", rLowerOBC.offst1.bits.OBC_OFST_GR, rLowerOBC.offst1.bits.OBC_OFST_GR, rUpperOBC.offst1.bits.OBC_OFST_GR);
    INTER_LOG_IF(bSmoothOBCDebug, "[L, S, U]OBC_OFST_GB = %d, %d, %d", rLowerOBC.offst2.bits.OBC_OFST_GB, rLowerOBC.offst2.bits.OBC_OFST_GB, rUpperOBC.offst2.bits.OBC_OFST_GB);
    INTER_LOG_IF(bSmoothOBCDebug, "[L, S, U]OBC_OFST_R  = %d, %d, %d", rLowerOBC.offst3.bits.OBC_OFST_R, rLowerOBC.offst3.bits.OBC_OFST_R, rUpperOBC.offst3.bits.OBC_OFST_R);
    INTER_LOG_IF(bSmoothOBCDebug, "[L, S, U]OBC_GAIN_B  = %d, %d, %d", rLowerOBC.gain0.bits.OBC_GAIN_B, rLowerOBC.gain0.bits.OBC_GAIN_B, rUpperOBC.gain0.bits.OBC_GAIN_B);
    INTER_LOG_IF(bSmoothOBCDebug, "[L, S, U]OBC_GAIN_GR = %d, %d, %d", rLowerOBC.gain1.bits.OBC_GAIN_GR, rLowerOBC.gain1.bits.OBC_GAIN_GR, rUpperOBC.gain1.bits.OBC_GAIN_GR);
    INTER_LOG_IF(bSmoothOBCDebug, "[L, S, U]OBC_GAIN_GB = %d, %d, %d", rLowerOBC.gain2.bits.OBC_GAIN_GB, rLowerOBC.gain2.bits.OBC_GAIN_GB, rUpperOBC.gain2.bits.OBC_GAIN_GB);
    INTER_LOG_IF(bSmoothOBCDebug, "[L, S, U]OBC_GAIN_R  = %d, %d, %d", rLowerOBC.gain3.bits.OBC_GAIN_R, rLowerOBC.gain3.bits.OBC_GAIN_R, rUpperOBC.gain3.bits.OBC_GAIN_R);
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
    MBOOL bSmoothBPCDebug = interpPropertyGet("debug.smooth_bpc.enable", MFALSE);
    INTER_LOG_IF(bSmoothBPCDebug,"[%s()] u4RealISO: %d, u4UpperISO = %d, u4LowerISO = %d\n", __FUNCTION__, u4RealISO, u4UpperISO, u4LowerISO);

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
//    MINT32 BPC_XSIZE           = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_XSIZEpre, BPC_XSIZEpos, 0);
//    MINT32 BPC_YSIZE           = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_YSIZEpre, BPC_YSIZEpos, 0);
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
    rSmoothBPC.con.bits.BPC_EXC	            = BPC_EXC;
    rSmoothBPC.cor.bits.BPC_RANK_IDXR	    = BPC_RANK_IDXR;
    rSmoothBPC.cor.bits.BPC_RANK_IDXG	    = BPC_RANK_IDXG;
    rSmoothBPC.th2_c.bits.BPC_RANK_IDXB	    = BPC_RANK_IDXB;
    rSmoothBPC.con.bits.BPC_BLD_MODE	    = BPC_BLD_MODE;
    rSmoothBPC.th2_c.bits.BPC_BLD_LWB 	    = BPC_BLD_LWB;
    rSmoothBPC.th2.bits.BPC_BLD0    	    = BPC_BLD0;
    rSmoothBPC.th2.bits.BPC_BLD1    	    = BPC_BLD1;
    rSmoothBPC.th1.bits.BPC_BLD_SLP0  	    = BPC_BLD_SLP0;

    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_EN              = %d, %d, %d", rLowerBPC.con.bits.BPC_EN, rSmoothBPC.con.bits.BPC_EN, rUpperBPC.con.bits.BPC_EN);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_LUT_EN          = %d, %d, %d", rLowerBPC.con.bits.BPC_LUT_EN, rSmoothBPC.con.bits.BPC_LUT_EN, rUpperBPC.con.bits.BPC_LUT_EN);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_DTC_MODE        = %d, %d, %d", rLowerBPC.con.bits.BPC_DTC_MODE, rSmoothBPC.con.bits.BPC_DTC_MODE, rUpperBPC.con.bits.BPC_DTC_MODE);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_CRC_MODE        = %d, %d, %d", rLowerBPC.con.bits.BPC_CRC_MODE, rSmoothBPC.con.bits.BPC_CRC_MODE, rUpperBPC.con.bits.BPC_CRC_MODE);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_RNG             = %d, %d, %d", rLowerBPC.dtc.bits.BPC_RNG, rSmoothBPC.dtc.bits.BPC_RNG, rUpperBPC.dtc.bits.BPC_RNG);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_DIR_TH          = %d, %d, %d", rLowerBPC.cor.bits.BPC_DIR_TH, rSmoothBPC.cor.bits.BPC_DIR_TH, rUpperBPC.cor.bits.BPC_DIR_TH);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_DIR_TH2         = %d, %d, %d", rLowerBPC.cor.bits.BPC_DIR_TH2, rSmoothBPC.cor.bits.BPC_DIR_TH2, rUpperBPC.cor.bits.BPC_DIR_TH2);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_DIR_MAX         = %d, %d, %d", rLowerBPC.cor.bits.BPC_DIR_MAX, rSmoothBPC.cor.bits.BPC_DIR_MAX, rUpperBPC.cor.bits.BPC_DIR_MAX);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_TH_LWB          = %d, %d, %d", rLowerBPC.th1.bits.BPC_TH_LWB, rSmoothBPC.th1.bits.BPC_TH_LWB, rUpperBPC.th1.bits.BPC_TH_LWB);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_TH_SLA          = %d, %d, %d", rLowerBPC.th3.bits.BPC_TH_SLA, rSmoothBPC.th3.bits.BPC_TH_SLA, rUpperBPC.th3.bits.BPC_TH_SLA);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_DK_TH_SLA       = %d, %d, %d", rLowerBPC.th4.bits.BPC_DK_TH_SLA, rSmoothBPC.th4.bits.BPC_DK_TH_SLA, rUpperBPC.th4.bits.BPC_DK_TH_SLA);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_TH_SLB          = %d, %d, %d", rLowerBPC.th3.bits.BPC_TH_SLB, rSmoothBPC.th3.bits.BPC_TH_SLB, rUpperBPC.th3.bits.BPC_TH_SLB);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_DK_TH_SLB       = %d, %d, %d", rLowerBPC.th4.bits.BPC_DK_TH_SLB, rSmoothBPC.th4.bits.BPC_DK_TH_SLB, rUpperBPC.th4.bits.BPC_DK_TH_SLB);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_TH_XA           = %d, %d, %d", rLowerBPC.th3.bits.BPC_TH_XA, rSmoothBPC.th3.bits.BPC_TH_XA, rUpperBPC.th3.bits.BPC_TH_XA);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_DK_TH_XA        = %d, %d, %d", rLowerBPC.th4.bits.BPC_DK_TH_XA, rSmoothBPC.th4.bits.BPC_DK_TH_XA, rUpperBPC.th4.bits.BPC_DK_TH_XA);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_TH_XB           = %d, %d, %d", rLowerBPC.th3.bits.BPC_TH_XB, rSmoothBPC.th3.bits.BPC_TH_XB, rUpperBPC.th3.bits.BPC_TH_XB);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_DK_TH_XB        = %d, %d, %d", rLowerBPC.th4.bits.BPC_DK_TH_XB, rSmoothBPC.th4.bits.BPC_DK_TH_XB, rUpperBPC.th4.bits.BPC_DK_TH_XB);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_TH_Y            = %d, %d, %d", rLowerBPC.th1.bits.BPC_TH_Y, rSmoothBPC.th1.bits.BPC_TH_Y, rUpperBPC.th1.bits.BPC_TH_Y);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_TH_UPB          = %d, %d, %d", rLowerBPC.th2.bits.BPC_TH_UPB, rSmoothBPC.th2.bits.BPC_TH_UPB, rUpperBPC.th2.bits.BPC_TH_UPB);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_C_TH_LWB        = %d, %d, %d", rLowerBPC.th1_c.bits.BPC_C_TH_LWB, rSmoothBPC.th1_c.bits.BPC_C_TH_LWB, rUpperBPC.th1_c.bits.BPC_C_TH_LWB);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_C_TH_SLA        = %d, %d, %d", rLowerBPC.th3_c.bits.BPC_C_TH_SLA, rSmoothBPC.th3_c.bits.BPC_C_TH_SLA, rUpperBPC.th3_c.bits.BPC_C_TH_SLA);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_C_TH_SLB        = %d, %d, %d", rLowerBPC.th3_c.bits.BPC_C_TH_SLB, rSmoothBPC.th3_c.bits.BPC_C_TH_SLB, rUpperBPC.th3_c.bits.BPC_C_TH_SLB);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_C_TH_XA         = %d, %d, %d", rLowerBPC.th3_c.bits.BPC_C_TH_XA, rSmoothBPC.th3_c.bits.BPC_C_TH_XA, rUpperBPC.th3_c.bits.BPC_C_TH_XA);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_C_TH_XB         = %d, %d, %d", rLowerBPC.th3_c.bits.BPC_C_TH_XB, rSmoothBPC.th3_c.bits.BPC_C_TH_XB, rUpperBPC.th3_c.bits.BPC_C_TH_XB);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_C_TH_Y          = %d, %d, %d", rLowerBPC.th1_c.bits.BPC_C_TH_Y, rSmoothBPC.th1_c.bits.BPC_C_TH_Y, rUpperBPC.th1_c.bits.BPC_C_TH_Y);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_C_TH_UPB        = %d, %d, %d", rLowerBPC.th2_c.bits.BPC_C_TH_UPB, rSmoothBPC.th2_c.bits.BPC_C_TH_UPB, rUpperBPC.th2_c.bits.BPC_C_TH_UPB);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_NO_LV           = %d, %d, %d", rLowerBPC.dtc.bits.BPC_NO_LV, rSmoothBPC.dtc.bits.BPC_NO_LV, rUpperBPC.dtc.bits.BPC_NO_LV);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_CT_LV           = %d, %d, %d", rLowerBPC.dtc.bits.BPC_CT_LV, rSmoothBPC.dtc.bits.BPC_CT_LV, rUpperBPC.dtc.bits.BPC_CT_LV);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_TH_MUL          = %d, %d, %d", rLowerBPC.dtc.bits.BPC_TH_MUL, rSmoothBPC.dtc.bits.BPC_TH_MUL, rUpperBPC.dtc.bits.BPC_TH_MUL);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_CS_MODE         = %d, %d, %d", rLowerBPC.con.bits.BPC_CS_MODE, rSmoothBPC.con.bits.BPC_CS_MODE, rUpperBPC.con.bits.BPC_CS_MODE);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_CS_RNG          = %d, %d, %d", rLowerBPC.dtc.bits.BPC_CS_RNG, rSmoothBPC.dtc.bits.BPC_CS_RNG, rUpperBPC.dtc.bits.BPC_CS_RNG);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_TABLE_END_MODE  = %d, %d, %d", rLowerBPC.con.bits.BPC_TABLE_END_MODE, rSmoothBPC.con.bits.BPC_TABLE_END_MODE, rUpperBPC.con.bits.BPC_TABLE_END_MODE);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_AVG_MODE        = %d, %d, %d", rLowerBPC.con.bits.BPC_AVG_MODE, rSmoothBPC.con.bits.BPC_AVG_MODE, rUpperBPC.con.bits.BPC_AVG_MODE);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BNR_LE_INV_CTL      = %d, %d, %d", rLowerBPC.con.bits.BNR_LE_INV_CTL, rSmoothBPC.con.bits.BNR_LE_INV_CTL, rUpperBPC.con.bits.BNR_LE_INV_CTL);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BNR_OSC_COUNT       = %d, %d, %d", rLowerBPC.con.bits.BNR_OSC_COUNT, rSmoothBPC.con.bits.BNR_OSC_COUNT, rUpperBPC.con.bits.BNR_OSC_COUNT);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BNR_EDGE            = %d, %d, %d", rLowerBPC.con.bits.BNR_EDGE, rSmoothBPC.con.bits.BNR_EDGE, rUpperBPC.con.bits.BNR_EDGE);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_XOFFSET         = %d, %d, %d", rLowerBPC.tbli1.bits.BPC_XOFFSET, rSmoothBPC.tbli1.bits.BPC_XOFFSET, rUpperBPC.tbli1.bits.BPC_XOFFSET);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_YOFFSET         = %d, %d, %d", rLowerBPC.tbli1.bits.BPC_YOFFSET, rSmoothBPC.tbli1.bits.BPC_YOFFSET, rUpperBPC.tbli1.bits.BPC_YOFFSET);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_EXC             = %d, %d, %d", rLowerBPC.con.bits.BPC_EXC, rSmoothBPC.con.bits.BPC_EXC, rUpperBPC.con.bits.BPC_EXC);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_RANK_IDXR       = %d, %d, %d", rLowerBPC.cor.bits.BPC_RANK_IDXR, rSmoothBPC.cor.bits.BPC_RANK_IDXR, rUpperBPC.cor.bits.BPC_RANK_IDXR);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_RANK_IDXG       = %d, %d, %d", rLowerBPC.cor.bits.BPC_RANK_IDXG, rSmoothBPC.cor.bits.BPC_RANK_IDXG, rUpperBPC.cor.bits.BPC_RANK_IDXG);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_RANK_IDXB       = %d, %d, %d", rLowerBPC.th2_c.bits.BPC_RANK_IDXB, rSmoothBPC.th2_c.bits.BPC_RANK_IDXB, rUpperBPC.th2_c.bits.BPC_RANK_IDXB);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_BLD_MODE        = %d, %d, %d", rLowerBPC.con.bits.BPC_BLD_MODE, rSmoothBPC.con.bits.BPC_BLD_MODE, rUpperBPC.con.bits.BPC_BLD_MODE);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_BLD_LWB         = %d, %d, %d", rLowerBPC.th2_c.bits.BPC_BLD_LWB, rSmoothBPC.th2_c.bits.BPC_BLD_LWB, rUpperBPC.th2_c.bits.BPC_BLD_LWB);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_BLD0            = %d, %d, %d", rLowerBPC.th2.bits.BPC_BLD0, rSmoothBPC.th2.bits.BPC_BLD0, rUpperBPC.th2.bits.BPC_BLD0);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_BLD1            = %d, %d, %d", rLowerBPC.th2.bits.BPC_BLD1, rSmoothBPC.th2.bits.BPC_BLD1, rUpperBPC.th2.bits.BPC_BLD1);
    INTER_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_BLD_SLP0        = %d, %d, %d", rLowerBPC.th1.bits.BPC_BLD_SLP0, rSmoothBPC.th1.bits.BPC_BLD_SLP0, rUpperBPC.th1.bits.BPC_BLD_SLP0);
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
    MBOOL bSmoothNR1Debug = interpPropertyGet("debug.smooth_nr1.enable", MFALSE);
    INTER_LOG_IF(bSmoothNR1Debug,"[%s()] u4RealISO: %d, u4UpperISO = %d, u4LowerISO = %d\n", __FUNCTION__, u4RealISO, u4UpperISO, u4LowerISO);

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

    INTER_LOG_IF(bSmoothNR1Debug,"[L,S,U]CT_EN    = %d, %d, %d", rLowerNR1.con.bits.NR1_CT_EN, rSmoothNR1.con.bits.NR1_CT_EN, rUpperNR1.con.bits.NR1_CT_EN);
    INTER_LOG_IF(bSmoothNR1Debug,"[L,S,U]CT_MBND  = %d, %d, %d", rLowerNR1.ct_con.bits.NR1_MBND, rSmoothNR1.ct_con.bits.NR1_MBND, rUpperNR1.ct_con.bits.NR1_MBND);
    INTER_LOG_IF(bSmoothNR1Debug,"[L,S,U]CT_MD    = %d, %d, %d", rLowerNR1.ct_con.bits.NR1_CT_MD, rSmoothNR1.ct_con.bits.NR1_CT_MD, rUpperNR1.ct_con.bits.NR1_CT_MD);
    INTER_LOG_IF(bSmoothNR1Debug,"[L,S,U]CT_MD2   = %d, %d, %d", rLowerNR1.ct_con.bits.NR1_CT_MD2, rSmoothNR1.ct_con.bits.NR1_CT_MD2, rUpperNR1.ct_con.bits.NR1_CT_MD2);
    INTER_LOG_IF(bSmoothNR1Debug,"[L,S,U]CT_THRD  = %d, %d, %d", rLowerNR1.ct_con.bits.NR1_CT_THRD, rSmoothNR1.ct_con.bits.NR1_CT_THRD, 	rUpperNR1.ct_con.bits.NR1_CT_THRD);
    INTER_LOG_IF(bSmoothNR1Debug,"[L,S,U]CT_DIV   = %d, %d, %d", rLowerNR1.ct_con.bits.NR1_CT_DIV, rSmoothNR1.ct_con.bits.NR1_CT_DIV, rUpperNR1.ct_con.bits.NR1_CT_DIV);
    INTER_LOG_IF(bSmoothNR1Debug,"[L,S,U]CT_SLOPE = %d, %d, %d", rLowerNR1.ct_con.bits.NR1_CT_SLOPE, rSmoothNR1.ct_con.bits.NR1_CT_SLOPE, rUpperNR1.ct_con.bits.NR1_CT_SLOPE);
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
    MBOOL bSmoothBNR_PDCDebug = interpPropertyGet("debug.smooth_bnr_pdc.enable", MFALSE);
    INTER_LOG_IF(bSmoothBNR_PDCDebug,"[%s()] u4RealISO: %d, u4UpperISO = %d, u4LowerISO = %d\n", __FUNCTION__, u4RealISO, u4UpperISO, u4LowerISO);

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


    INTER_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_EN       = %d, %d, %d", rLowerBNR_PDC.con.bits.PDC_EN, rSmoothBNR_PDC.con.bits.PDC_EN, rUpperBNR_PDC.con.bits.PDC_EN);
	INTER_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_CT       = %d, %d, %d", rLowerBNR_PDC.con.bits.PDC_CT, rSmoothBNR_PDC.con.bits.PDC_CT, rUpperBNR_PDC.con.bits.PDC_CT);
	INTER_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_L00  = %d, %d, %d", rLowerBNR_PDC.gain_l0.bits.PDC_GCF_L00, rSmoothBNR_PDC.gain_l0.bits.PDC_GCF_L00, rUpperBNR_PDC.gain_l0.bits.PDC_GCF_L00);
	INTER_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_L10  = %d, %d, %d", rLowerBNR_PDC.gain_l0.bits.PDC_GCF_L10, rSmoothBNR_PDC.gain_l0.bits.PDC_GCF_L10, rUpperBNR_PDC.gain_l0.bits.PDC_GCF_L10);
	INTER_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_L01  = %d, %d, %d", rLowerBNR_PDC.gain_l1.bits.PDC_GCF_L01, rSmoothBNR_PDC.gain_l1.bits.PDC_GCF_L01, rUpperBNR_PDC.gain_l1.bits.PDC_GCF_L01);
	INTER_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_L20  = %d, %d, %d", rLowerBNR_PDC.gain_l1.bits.PDC_GCF_L20, rSmoothBNR_PDC.gain_l1.bits.PDC_GCF_L20, rUpperBNR_PDC.gain_l1.bits.PDC_GCF_L20);
	INTER_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_L11  = %d, %d, %d", rLowerBNR_PDC.gain_l2.bits.PDC_GCF_L11, rSmoothBNR_PDC.gain_l2.bits.PDC_GCF_L11, rUpperBNR_PDC.gain_l2.bits.PDC_GCF_L11);
	INTER_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_L02  = %d, %d, %d", rLowerBNR_PDC.gain_l2.bits.PDC_GCF_L02, rSmoothBNR_PDC.gain_l2.bits.PDC_GCF_L02, rUpperBNR_PDC.gain_l2.bits.PDC_GCF_L02);
	INTER_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_L30  = %d, %d, %d", rLowerBNR_PDC.gain_l3.bits.PDC_GCF_L30, rSmoothBNR_PDC.gain_l3.bits.PDC_GCF_L30, rUpperBNR_PDC.gain_l3.bits.PDC_GCF_L30);
	INTER_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_L21  = %d, %d, %d", rLowerBNR_PDC.gain_l3.bits.PDC_GCF_L21, rSmoothBNR_PDC.gain_l3.bits.PDC_GCF_L21, rUpperBNR_PDC.gain_l3.bits.PDC_GCF_L21);
	INTER_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_L12  = %d, %d, %d", rLowerBNR_PDC.gain_l4.bits.PDC_GCF_L12, rSmoothBNR_PDC.gain_l4.bits.PDC_GCF_L12, rUpperBNR_PDC.gain_l4.bits.PDC_GCF_L12);
	INTER_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_L03  = %d, %d, %d", rLowerBNR_PDC.gain_l4.bits.PDC_GCF_L03, rSmoothBNR_PDC.gain_l4.bits.PDC_GCF_L03, rUpperBNR_PDC.gain_l4.bits.PDC_GCF_L03);
	INTER_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_R00  = %d, %d, %d", rLowerBNR_PDC.gain_r0.bits.PDC_GCF_R00, rSmoothBNR_PDC.gain_r0.bits.PDC_GCF_R00, rUpperBNR_PDC.gain_r0.bits.PDC_GCF_R00);
    INTER_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_R10  = %d, %d, %d", rLowerBNR_PDC.gain_r0.bits.PDC_GCF_R10, rSmoothBNR_PDC.gain_r0.bits.PDC_GCF_R10, rUpperBNR_PDC.gain_r0.bits.PDC_GCF_R10);
	INTER_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_R01  = %d, %d, %d", rLowerBNR_PDC.gain_r1.bits.PDC_GCF_R01, rSmoothBNR_PDC.gain_r1.bits.PDC_GCF_R01, rUpperBNR_PDC.gain_r1.bits.PDC_GCF_R01);
	INTER_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_R20  = %d, %d, %d", rLowerBNR_PDC.gain_r1.bits.PDC_GCF_R20, rSmoothBNR_PDC.gain_r1.bits.PDC_GCF_R20, rUpperBNR_PDC.gain_r1.bits.PDC_GCF_R20);
	INTER_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_R11  = %d, %d, %d", rLowerBNR_PDC.gain_r2.bits.PDC_GCF_R11, rSmoothBNR_PDC.gain_r2.bits.PDC_GCF_R11, rUpperBNR_PDC.gain_r2.bits.PDC_GCF_R11);
	INTER_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_R02  = %d, %d, %d", rLowerBNR_PDC.gain_r2.bits.PDC_GCF_R02, rSmoothBNR_PDC.gain_r2.bits.PDC_GCF_R02, rUpperBNR_PDC.gain_r2.bits.PDC_GCF_R02);
	INTER_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_R30  = %d, %d, %d", rLowerBNR_PDC.gain_r3.bits.PDC_GCF_R30, rSmoothBNR_PDC.gain_r3.bits.PDC_GCF_R30, rUpperBNR_PDC.gain_r3.bits.PDC_GCF_R30);
    INTER_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_R21  = %d, %d, %d", rLowerBNR_PDC.gain_r3.bits.PDC_GCF_R21, rSmoothBNR_PDC.gain_r3.bits.PDC_GCF_R21, rUpperBNR_PDC.gain_r3.bits.PDC_GCF_R21);
	INTER_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_R12  = %d, %d, %d", rLowerBNR_PDC.gain_r4.bits.PDC_GCF_R12, rSmoothBNR_PDC.gain_r4.bits.PDC_GCF_R12, rUpperBNR_PDC.gain_r4.bits.PDC_GCF_R12);
	INTER_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_R03  = %d, %d, %d", rLowerBNR_PDC.gain_r4.bits.PDC_GCF_R03, rSmoothBNR_PDC.gain_r4.bits.PDC_GCF_R03, rUpperBNR_PDC.gain_r4.bits.PDC_GCF_R03);
	INTER_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GTH      = %d, %d, %d", rLowerBNR_PDC.th_gb.bits.PDC_GTH, rSmoothBNR_PDC.th_gb.bits.PDC_GTH, rUpperBNR_PDC.th_gb.bits.PDC_GTH);
	INTER_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_ITH      = %d, %d, %d", rLowerBNR_PDC.th_ia.bits.PDC_ITH, rSmoothBNR_PDC.th_ia.bits.PDC_ITH, rUpperBNR_PDC.th_ia.bits.PDC_ITH);
    INTER_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_ATH      = %d, %d, %d", rLowerBNR_PDC.th_ia.bits.PDC_ATH, rSmoothBNR_PDC.th_ia.bits.PDC_ATH, rUpperBNR_PDC.th_ia.bits.PDC_ATH);
	INTER_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_NTH      = %d, %d, %d", rLowerBNR_PDC.th_hd.bits.PDC_NTH, rSmoothBNR_PDC.th_hd.bits.PDC_NTH, rUpperBNR_PDC.th_hd.bits.PDC_NTH);
	INTER_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_DTH      = %d, %d, %d", rLowerBNR_PDC.th_hd.bits.PDC_DTH, rSmoothBNR_PDC.th_hd.bits.PDC_DTH, rUpperBNR_PDC.th_hd.bits.PDC_DTH);
	INTER_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GSL      = %d, %d, %d", rLowerBNR_PDC.sl.bits.PDC_GSL, rSmoothBNR_PDC.sl.bits.PDC_GSL, rUpperBNR_PDC.sl.bits.PDC_GSL);
	INTER_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_BSL      = %d, %d, %d", rLowerBNR_PDC.sl.bits.PDC_BSL, rSmoothBNR_PDC.sl.bits.PDC_BSL, rUpperBNR_PDC.sl.bits.PDC_BSL);
	INTER_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_ISL      = %d, %d, %d", rLowerBNR_PDC.sl.bits.PDC_ISL, rSmoothBNR_PDC.sl.bits.PDC_ISL, rUpperBNR_PDC.sl.bits.PDC_ISL);
	INTER_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_ASL      = %d, %d, %d", rLowerBNR_PDC.sl.bits.PDC_ASL, rSmoothBNR_PDC.sl.bits.PDC_ASL, rUpperBNR_PDC.sl.bits.PDC_ASL);
	INTER_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_NORM = %d, %d, %d", rLowerBNR_PDC.sl.bits.PDC_GCF_NORM, rSmoothBNR_PDC.sl.bits.PDC_GCF_NORM, rUpperBNR_PDC.sl.bits.PDC_GCF_NORM);
	INTER_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_XCENTER  = %d, %d, %d", rLowerBNR_PDC.pos.bits.PDC_XCENTER, rSmoothBNR_PDC.pos.bits.PDC_XCENTER, rUpperBNR_PDC.pos.bits.PDC_XCENTER);
    INTER_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_YCENTER  = %d, %d, %d", rLowerBNR_PDC.pos.bits.PDC_YCENTER, rSmoothBNR_PDC.pos.bits.PDC_YCENTER, rUpperBNR_PDC.pos.bits.PDC_YCENTER);
}

MINT32 InterParam_RMM(MINT32 ISO, MINT32 ISOpre, MINT32 ISOpos, MINT32 PARAMpre, MINT32 PARAMpos, MINT32 Method){
	MINT32 InterPARAM = 0;

	if(ISOpre == ISOpos){
		//	To prevent someone sets different PARAMpre and PARAMpos
		return (MINT32)((PARAMpre + PARAMpos + 1) / 2);
	}

	switch(Method){
	case 1:
		if(abs(ISO - ISOpre) > abs(ISOpos - ISO)) {
			InterPARAM = PARAMpos;
		}
		else{
			InterPARAM = PARAMpre;
		}
		break;
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
    MBOOL bSmoothRMMDebug = interpPropertyGet("debug.smooth_rmm.enable", MFALSE);

    INTER_LOG_IF(bSmoothRMMDebug,"[%s()] u4RealISO: %d, u4UpperISO = %d, u4LowerISO = %d\n", __FUNCTION__, u4RealISO, u4UpperISO, u4LowerISO);

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
            INTER_LOG_IF(bSmoothRMMDebug, "[L,S,U]%22s = %d, %d, %d", #item2, rLowerRMM.item1.bits.item2, rSmoothRMM.item1.bits.item2, rUpperRMM.item1.bits.item2); \
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
    MBOOL bSmoothRNRDebug = interpPropertyGet("debug.smooth_rnr.enable", MFALSE);

    INTER_LOG_IF(bSmoothRNRDebug,"[%s()] u4RealISO: %d, u4UpperISO = %d, u4LowerISO = %d\n", __FUNCTION__, u4RealISO, u4UpperISO, u4LowerISO);

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

    INTER_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_EPS      = %d, %d, %d", rLowerRNR.con1.bits.RNR_EPS, rSmoothRNR.con1.bits.RNR_EPS, rUpperRNR.con1.bits.RNR_EPS);
    INTER_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_RAT      = %d, %d, %d", rLowerRNR.con1.bits.RNR_RAT, rSmoothRNR.con1.bits.RNR_RAT, rUpperRNR.con1.bits.RNR_RAT);
    INTER_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_VC0_B    = %d, %d, %d", rLowerRNR.vc0.bits.RNR_VC0_B, rSmoothRNR.vc0.bits.RNR_VC0_B, rUpperRNR.vc0.bits.RNR_VC0_B);
    INTER_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_VC0_G    = %d, %d, %d", rLowerRNR.vc0.bits.RNR_VC0_G, rSmoothRNR.vc0.bits.RNR_VC0_G, rUpperRNR.vc0.bits.RNR_VC0_G);
    INTER_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_VC0_R    = %d, %d, %d", rLowerRNR.vc0.bits.RNR_VC0_R, rSmoothRNR.vc0.bits.RNR_VC0_R, rUpperRNR.vc0.bits.RNR_VC0_R);
    INTER_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_VC1_B    = %d, %d, %d", rLowerRNR.vc1.bits.RNR_VC1_B, rSmoothRNR.vc1.bits.RNR_VC1_B, rUpperRNR.vc1.bits.RNR_VC1_B);
    INTER_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_VC1_G    = %d, %d, %d", rLowerRNR.vc1.bits.RNR_VC1_G, rSmoothRNR.vc1.bits.RNR_VC1_G, rUpperRNR.vc1.bits.RNR_VC1_G);
    INTER_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_VC1_R    = %d, %d, %d", rLowerRNR.vc1.bits.RNR_VC1_R, rSmoothRNR.vc1.bits.RNR_VC1_R, rUpperRNR.vc1.bits.RNR_VC1_R);
    INTER_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_RNG1     = %d, %d, %d", rLowerRNR.rng.bits.RNR_RNG1, rSmoothRNR.rng.bits.RNR_RNG1, rUpperRNR.rng.bits.RNR_RNG1);
    INTER_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_RNG2     = %d, %d, %d", rLowerRNR.rng.bits.RNR_RNG2, rSmoothRNR.rng.bits.RNR_RNG2, rUpperRNR.rng.bits.RNR_RNG2);
    INTER_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_RNG3     = %d, %d, %d", rLowerRNR.rng.bits.RNR_RNG3, rSmoothRNR.rng.bits.RNR_RNG3, rUpperRNR.rng.bits.RNR_RNG3);
    INTER_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_RNG4     = %d, %d, %d", rLowerRNR.rng.bits.RNR_RNG4, rSmoothRNR.rng.bits.RNR_RNG4, rUpperRNR.rng.bits.RNR_RNG4);
    INTER_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_CTHL     = %d, %d, %d", rLowerRNR.con2.bits.RNR_CTHL, rSmoothRNR.con2.bits.RNR_CTHL, rUpperRNR.con2.bits.RNR_CTHL);
    INTER_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_CTHR     = %d, %d, %d", rLowerRNR.con2.bits.RNR_CTHR, rSmoothRNR.con2.bits.RNR_CTHR, rUpperRNR.con2.bits.RNR_CTHR);
    INTER_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_RAD      = %d, %d, %d", rLowerRNR.con2.bits.RNR_RAD, rSmoothRNR.con2.bits.RNR_RAD, rUpperRNR.con2.bits.RNR_RAD);
    INTER_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_PRF_RAT  = %d, %d, %d", rLowerRNR.con2.bits.RNR_PRF_RAT, rSmoothRNR.con2.bits.RNR_PRF_RAT, rUpperRNR.con2.bits.RNR_PRF_RAT);
    INTER_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_MED_RAT  = %d, %d, %d", rLowerRNR.con2.bits.RNR_MED_RAT, rSmoothRNR.con2.bits.RNR_MED_RAT, rUpperRNR.con2.bits.RNR_MED_RAT);
    INTER_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_INT_OFST = %d, %d, %d", rLowerRNR.con3.bits.RNR_INT_OFST, rSmoothRNR.con3.bits.RNR_INT_OFST, rUpperRNR.con3.bits.RNR_INT_OFST);
    INTER_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_LLP_RAT  = %d, %d, %d", rLowerRNR.con3.bits.RNR_LLP_RAT, rSmoothRNR.con3.bits.RNR_LLP_RAT, rUpperRNR.con3.bits.RNR_LLP_RAT);
    INTER_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_GD_RAT   = %d, %d, %d", rLowerRNR.con3.bits.RNR_GD_RAT, rSmoothRNR.con3.bits.RNR_GD_RAT, rUpperRNR.con3.bits.RNR_GD_RAT);
    INTER_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_FL_MODE  = %d, %d, %d", rLowerRNR.con3.bits.RNR_FL_MODE, rSmoothRNR.con3.bits.RNR_FL_MODE, rUpperRNR.con3.bits.RNR_FL_MODE);
    INTER_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_SL_Y1    = %d, %d, %d", rLowerRNR.sl.bits.RNR_SL_Y1, rSmoothRNR.sl.bits.RNR_SL_Y1, rUpperRNR.sl.bits.RNR_SL_Y1);
    INTER_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_SL_Y2    = %d, %d, %d", rLowerRNR.sl.bits.RNR_SL_Y2, rSmoothRNR.sl.bits.RNR_SL_Y2, rUpperRNR.sl.bits.RNR_SL_Y2);
    INTER_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_SL_EN    = %d, %d, %d", rLowerRNR.sl.bits.RNR_SL_EN, rSmoothRNR.sl.bits.RNR_SL_EN, rUpperRNR.sl.bits.RNR_SL_EN);
    INTER_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_STH_C1   = %d, %d, %d", rLowerRNR.ssl_sth.bits.RNR_STH_C1, rSmoothRNR.ssl_sth.bits.RNR_STH_C1, rUpperRNR.ssl_sth.bits.RNR_STH_C1);
    INTER_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_SSL_C1   = %d, %d, %d", rLowerRNR.ssl_sth.bits.RNR_SSL_C1, rSmoothRNR.ssl_sth.bits.RNR_SSL_C1, rUpperRNR.ssl_sth.bits.RNR_SSL_C1);
    INTER_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_STH_C2   = %d, %d, %d", rLowerRNR.ssl_sth.bits.RNR_STH_C2, rSmoothRNR.ssl_sth.bits.RNR_STH_C2, rUpperRNR.ssl_sth.bits.RNR_STH_C2);
    INTER_LOG_IF(bSmoothRNRDebug, "[L,S,U]RNR_SSL_C2   = %d, %d, %d", rLowerRNR.ssl_sth.bits.RNR_SSL_C2, rSmoothRNR.ssl_sth.bits.RNR_SSL_C2, rUpperRNR.ssl_sth.bits.RNR_SSL_C2);
}


MINT32 InterParam_UDM(double Ratio_L_L, double Ratio_L_U, double Ratio_U_L, double Ratio_U_U, MINT32 PARAM_L_L, MINT32 PARAM_L_U, MINT32 PARAM_U_L, MINT32 PARAM_U_U, MINT32 Method) {
    MINT32 InterPARAM = 0;


    switch(Method) {
    case 1:
        if( Ratio_L_L > Ratio_L_U)
		{
            if( Ratio_L_L > Ratio_U_L)
			{
                InterPARAM = PARAM_L_L;
        	}
        	else
			{
                InterPARAM = PARAM_U_L;
        	}
        }
        else
		{
            if( Ratio_L_L > Ratio_U_L)
			{
                InterPARAM = PARAM_L_U;
        	}
        	else
			{
                InterPARAM = PARAM_U_U;
        	}
        }
        break;
    case 0:
    default:
        InterPARAM = (MINT32)(Ratio_L_L * PARAM_L_L + Ratio_L_U * PARAM_L_U + Ratio_U_L * PARAM_U_L + Ratio_U_U * PARAM_U_U + 0.5);
        break;
    }

    return InterPARAM;
}

MVOID SmoothUDM(ISP_NVRAM_UDM_INT_T const& rParam,   // UDM settings
                ISP_NVRAM_UDM_T& rSmoothUDM)   // Output
{
    MBOOL bSmoothUDMDebug = interpPropertyGet("debug.smooth_udm.enable", MFALSE);

	MINT32 u4RealISO = rParam.u4RealISO;
	MINT32 u4UpperISO = rParam.u4UpperISO;
	MINT32 u4LowerISO = rParam.u4LowerISO;
	MINT32 fRealZoom = rParam.i4ZoomRatio_x100;
	MINT32 fUpperZoom = rParam.u4UpperZoom;
	MINT32 fLowerZoom = rParam.u4LowerZoom;


	INTER_LOG_IF(bSmoothUDMDebug,"[%s()] u4RealISO: %d, u4UpperISO = %d, u4LowerISO = %d\n", __FUNCTION__, u4RealISO, u4UpperISO, u4LowerISO);
	INTER_LOG_IF(bSmoothUDMDebug,"[%s()] u4RealZoom: %d, u4UpperZoom = %d, u4LowerZoom = %d\n", __FUNCTION__, fRealZoom, fUpperZoom, fLowerZoom);

	// _L_L = Low ISO Low Zoom, _L_U = Low ISO Up Zoom, _U_L = Up ISO Low Zoom, _U_U = Up ISO Up Zoom

	double RATIO_ISO_lo = 0;
	double RATIO_ISO_up = 0;
	double RATIO_Zoom_lo = 0;
	double RATIO_Zoom_up = 0;

    //protection for out-of-bound


    if (u4RealISO <= u4LowerISO)
    {
        RATIO_ISO_lo = 1;
    }
	else if(u4RealISO >= u4UpperISO)
	{
		RATIO_ISO_lo = 0;
	}
	else
	{
		RATIO_ISO_lo = (double)(u4UpperISO - u4RealISO) / (double)(u4UpperISO - u4LowerISO);
	}

	RATIO_ISO_up = 1.0 - RATIO_ISO_lo;

	if (fRealZoom <= fLowerZoom)
    {
        RATIO_Zoom_lo = 1;
    }
	else if(fRealZoom >= fUpperZoom)
	{
		RATIO_Zoom_lo = 0;
	}
	else
	{
		RATIO_Zoom_lo = (double)(fUpperZoom - fRealZoom) / (double)(fUpperZoom - fLowerZoom);
	}

	RATIO_Zoom_up = 1.0 - RATIO_Zoom_lo;





	double RATIO_L_L = (double)(RATIO_ISO_lo * RATIO_Zoom_lo);
	double RATIO_L_U = (double)(RATIO_ISO_lo * RATIO_Zoom_up);
	double RATIO_U_L = (double)(RATIO_ISO_up * RATIO_Zoom_lo);
	double RATIO_U_U = (double)(RATIO_ISO_up * RATIO_Zoom_up);


	//Low ISO Low Zoom
	MINT32 UDM_BYP_L_L		= rParam.rLowerIso_LowerZoom.intp_crs.bits.UDM_BYP;
	MINT32 UDM_GRAD_WT_L_L	= rParam.rLowerIso_LowerZoom.intp_crs.bits.UDM_GRAD_WT;
	MINT32 UDM_ETH_L_L		= rParam.rLowerIso_LowerZoom.intp_crs.bits.UDM_ETH;
	MINT32 UDM_FTH_L_L		= rParam.rLowerIso_LowerZoom.intp_crs.bits.UDM_FTH;
	MINT32 UDM_DTH_L_L		= rParam.rLowerIso_LowerZoom.intp_crs.bits.UDM_DTH;
	MINT32 UDM_CD_SLC_L_L	= rParam.rLowerIso_LowerZoom.intp_nat.bits.UDM_CD_SLC;
	MINT32 UDM_CD_SLL_L_L	= rParam.rLowerIso_LowerZoom.intp_nat.bits.UDM_CD_SLL;
	MINT32 UDM_L0_OFST_L_L	= rParam.rLowerIso_LowerZoom.intp_nat.bits.UDM_L0_OFST;
	MINT32 UDM_L0_SL_L_L	= rParam.rLowerIso_LowerZoom.intp_nat.bits.UDM_L0_SL;
	MINT32 UDM_L1_OFST_L_L	= rParam.rLowerIso_LowerZoom.intp_aug.bits.UDM_L1_OFST;
	MINT32 UDM_L1_SL_L_L	= rParam.rLowerIso_LowerZoom.intp_aug.bits.UDM_L1_SL;
	MINT32 UDM_L2_OFST_L_L	= rParam.rLowerIso_LowerZoom.intp_aug.bits.UDM_L2_OFST;
	MINT32 UDM_L2_SL_L_L	= rParam.rLowerIso_LowerZoom.intp_aug.bits.UDM_L2_SL;
	MINT32 UDM_DN_OFST_L_L	= rParam.rLowerIso_LowerZoom.intp_aug.bits.UDM_DN_OFST;
	MINT32 UDM_HL_OFST_L_L	= rParam.rLowerIso_LowerZoom.intp_nat.bits.UDM_HL_OFST;
	MINT32 UDM_LM_Y0_L_L	= rParam.rLowerIso_LowerZoom.luma_lut1.bits.UDM_LM_Y0;
	MINT32 UDM_LM_Y1_L_L	= rParam.rLowerIso_LowerZoom.luma_lut1.bits.UDM_LM_Y1;
	MINT32 UDM_LM_Y2_L_L	= rParam.rLowerIso_LowerZoom.luma_lut1.bits.UDM_LM_Y2;
	MINT32 UDM_LM_Y3_L_L	= rParam.rLowerIso_LowerZoom.luma_lut2.bits.UDM_LM_Y3;
	MINT32 UDM_LM_Y4_L_L	= rParam.rLowerIso_LowerZoom.luma_lut2.bits.UDM_LM_Y4;
	MINT32 UDM_LM_Y5_L_L	= rParam.rLowerIso_LowerZoom.luma_lut2.bits.UDM_LM_Y5;
	MINT32 UDM_SL_Y1_L_L	= rParam.rLowerIso_LowerZoom.sl_ctl.bits.UDM_SL_Y1;
	MINT32 UDM_SL_Y2_L_L	= rParam.rLowerIso_LowerZoom.sl_ctl.bits.UDM_SL_Y2;
	MINT32 UDM_SL_HR_L_L	= rParam.rLowerIso_LowerZoom.sl_ctl.bits.UDM_SL_HR;
	MINT32 UDM_SL_EN_L_L	= rParam.rLowerIso_LowerZoom.sl_ctl.bits.UDM_SL_EN;
	MINT32 UDM_HT_GN1_L_L	= rParam.rLowerIso_LowerZoom.hftd_ctl.bits.UDM_HT_GN1;
	MINT32 UDM_HT_GN2_L_L	= rParam.rLowerIso_LowerZoom.hftd_ctl.bits.UDM_HT_GN2;
	MINT32 UDM_HD_GN1_L_L	= rParam.rLowerIso_LowerZoom.hftd_ctl.bits.UDM_HD_GN1;
	MINT32 UDM_HD_GN2_L_L	= rParam.rLowerIso_LowerZoom.hftd_ctl.bits.UDM_HD_GN2;
	MINT32 UDM_XTK_RAT_L_L	= rParam.rLowerIso_LowerZoom.nr_str.bits.UDM_XTK_RAT;
	MINT32 UDM_XTK_OFST_L_L = rParam.rLowerIso_LowerZoom.nr_str.bits.UDM_XTK_OFST;
	MINT32 UDM_XTK_SL_L_L	= rParam.rLowerIso_LowerZoom.nr_str.bits.UDM_XTK_SL;
	MINT32 UDM_N0_STR_L_L	= rParam.rLowerIso_LowerZoom.nr_str.bits.UDM_N0_STR;
	MINT32 UDM_N1_STR_L_L	= rParam.rLowerIso_LowerZoom.nr_str.bits.UDM_N1_STR;
	MINT32 UDM_N2_STR_L_L	= rParam.rLowerIso_LowerZoom.nr_str.bits.UDM_N2_STR;
	MINT32 UDM_N0_OFST_L_L	= rParam.rLowerIso_LowerZoom.nr_act.bits.UDM_N0_OFST;
	MINT32 UDM_N1_OFST_L_L	= rParam.rLowerIso_LowerZoom.nr_act.bits.UDM_N1_OFST;
	MINT32 UDM_N2_OFST_L_L	= rParam.rLowerIso_LowerZoom.nr_act.bits.UDM_N2_OFST;
	MINT32 UDM_NSL_L_L		= rParam.rLowerIso_LowerZoom.nr_act.bits.UDM_NSL;
	MINT32 UDM_NGR_L_L		= rParam.rLowerIso_LowerZoom.nr_act.bits.UDM_NGR;
	MINT32 UDM_HA_STR_L_L	= rParam.rLowerIso_LowerZoom.hf_str.bits.UDM_HA_STR;
	MINT32 UDM_H1_GN_L_L	= rParam.rLowerIso_LowerZoom.hf_str.bits.UDM_H1_GN;
	MINT32 UDM_H2_GN_L_L	= rParam.rLowerIso_LowerZoom.hf_str.bits.UDM_H2_GN;
	MINT32 UDM_H3_GN_L_L	= rParam.rLowerIso_LowerZoom.hf_str.bits.UDM_H3_GN;
	MINT32 UDM_HI_RAT_L_L	= rParam.rLowerIso_LowerZoom.hf_str.bits.UDM_HI_RAT;
	MINT32 UDM_H1_LWB_L_L	= rParam.rLowerIso_LowerZoom.hf_act1.bits.UDM_H1_LWB;
	MINT32 UDM_H1_UPB_L_L	= rParam.rLowerIso_LowerZoom.hf_act1.bits.UDM_H1_UPB;
	MINT32 UDM_H2_LWB_L_L	= rParam.rLowerIso_LowerZoom.hf_act1.bits.UDM_H2_LWB;
	MINT32 UDM_H2_UPB_L_L	= rParam.rLowerIso_LowerZoom.hf_act1.bits.UDM_H2_UPB;
	MINT32 UDM_H3_LWB_L_L	= rParam.rLowerIso_LowerZoom.hf_act2.bits.UDM_H3_LWB;
	MINT32 UDM_H3_UPB_L_L	= rParam.rLowerIso_LowerZoom.hf_act2.bits.UDM_H3_UPB;
	MINT32 UDM_HSLL_L_L 	= rParam.rLowerIso_LowerZoom.hf_act2.bits.UDM_HSLL;
	MINT32 UDM_HSLR_L_L 	= rParam.rLowerIso_LowerZoom.hf_act2.bits.UDM_HSLR;
	MINT32 UDM_CORE_TH1_L_L = rParam.rLowerIso_LowerZoom.hftd_ctl.bits.UDM_CORE_TH1;
	MINT32 UDM_CORE_TH2_L_L = rParam.rLowerIso_LowerZoom.hf_str.bits.UDM_CORE_TH2;
	MINT32 UDM_OV_TH_L_L	= rParam.rLowerIso_LowerZoom.clip.bits.UDM_OV_TH;
	MINT32 UDM_UN_TH_L_L	= rParam.rLowerIso_LowerZoom.clip.bits.UDM_UN_TH;
	MINT32 UDM_CLIP_TH_L_L	= rParam.rLowerIso_LowerZoom.clip.bits.UDM_CLIP_TH;
	MINT32 UDM_DS_THB_L_L	= rParam.rLowerIso_LowerZoom.dsb.bits.UDM_DS_THB;
	MINT32 UDM_DS_SLB_L_L	= rParam.rLowerIso_LowerZoom.dsb.bits.UDM_DS_SLB;
	MINT32 UDM_FL_MODE_L_L	= rParam.rLowerIso_LowerZoom.dsb.bits.UDM_FL_MODE;
	MINT32 UDM_SL_RAT_L_L	= rParam.rLowerIso_LowerZoom.dsb.bits.UDM_SL_RAT;
	MINT32 UDM_SC_RAT_L_L	= rParam.rLowerIso_LowerZoom.dsb.bits.UDM_SC_RAT;
	MINT32 UDM_LR_RAT_L_L	= rParam.rLowerIso_LowerZoom.lr_rat.bits.UDM_LR_RAT;

	//Low ISO Up Zoom
	MINT32 UDM_BYP_L_U		= rParam.rLowerIso_UpperZoom.intp_crs.bits.UDM_BYP;
	MINT32 UDM_GRAD_WT_L_U	= rParam.rLowerIso_UpperZoom.intp_crs.bits.UDM_GRAD_WT;
	MINT32 UDM_ETH_L_U		= rParam.rLowerIso_UpperZoom.intp_crs.bits.UDM_ETH;
	MINT32 UDM_FTH_L_U		= rParam.rLowerIso_UpperZoom.intp_crs.bits.UDM_FTH;
	MINT32 UDM_DTH_L_U		= rParam.rLowerIso_UpperZoom.intp_crs.bits.UDM_DTH;
	MINT32 UDM_CD_SLC_L_U	= rParam.rLowerIso_UpperZoom.intp_nat.bits.UDM_CD_SLC;
	MINT32 UDM_CD_SLL_L_U	= rParam.rLowerIso_UpperZoom.intp_nat.bits.UDM_CD_SLL;
	MINT32 UDM_L0_OFST_L_U	= rParam.rLowerIso_UpperZoom.intp_nat.bits.UDM_L0_OFST;
	MINT32 UDM_L0_SL_L_U	= rParam.rLowerIso_UpperZoom.intp_nat.bits.UDM_L0_SL;
	MINT32 UDM_L1_OFST_L_U	= rParam.rLowerIso_UpperZoom.intp_aug.bits.UDM_L1_OFST;
	MINT32 UDM_L1_SL_L_U	= rParam.rLowerIso_UpperZoom.intp_aug.bits.UDM_L1_SL;
	MINT32 UDM_L2_OFST_L_U	= rParam.rLowerIso_UpperZoom.intp_aug.bits.UDM_L2_OFST;
	MINT32 UDM_L2_SL_L_U	= rParam.rLowerIso_UpperZoom.intp_aug.bits.UDM_L2_SL;
	MINT32 UDM_DN_OFST_L_U	= rParam.rLowerIso_UpperZoom.intp_aug.bits.UDM_DN_OFST;
	MINT32 UDM_HL_OFST_L_U	= rParam.rLowerIso_UpperZoom.intp_nat.bits.UDM_HL_OFST;
	MINT32 UDM_LM_Y0_L_U	= rParam.rLowerIso_UpperZoom.luma_lut1.bits.UDM_LM_Y0;
	MINT32 UDM_LM_Y1_L_U	= rParam.rLowerIso_UpperZoom.luma_lut1.bits.UDM_LM_Y1;
	MINT32 UDM_LM_Y2_L_U	= rParam.rLowerIso_UpperZoom.luma_lut1.bits.UDM_LM_Y2;
	MINT32 UDM_LM_Y3_L_U	= rParam.rLowerIso_UpperZoom.luma_lut2.bits.UDM_LM_Y3;
	MINT32 UDM_LM_Y4_L_U	= rParam.rLowerIso_UpperZoom.luma_lut2.bits.UDM_LM_Y4;
	MINT32 UDM_LM_Y5_L_U	= rParam.rLowerIso_UpperZoom.luma_lut2.bits.UDM_LM_Y5;
	MINT32 UDM_SL_Y1_L_U	= rParam.rLowerIso_UpperZoom.sl_ctl.bits.UDM_SL_Y1;
	MINT32 UDM_SL_Y2_L_U	= rParam.rLowerIso_UpperZoom.sl_ctl.bits.UDM_SL_Y2;
	MINT32 UDM_SL_HR_L_U	= rParam.rLowerIso_UpperZoom.sl_ctl.bits.UDM_SL_HR;
	MINT32 UDM_SL_EN_L_U	= rParam.rLowerIso_UpperZoom.sl_ctl.bits.UDM_SL_EN;
	MINT32 UDM_HT_GN1_L_U	= rParam.rLowerIso_UpperZoom.hftd_ctl.bits.UDM_HT_GN1;
	MINT32 UDM_HT_GN2_L_U	= rParam.rLowerIso_UpperZoom.hftd_ctl.bits.UDM_HT_GN2;
	MINT32 UDM_HD_GN1_L_U	= rParam.rLowerIso_UpperZoom.hftd_ctl.bits.UDM_HD_GN1;
	MINT32 UDM_HD_GN2_L_U	= rParam.rLowerIso_UpperZoom.hftd_ctl.bits.UDM_HD_GN2;
	MINT32 UDM_XTK_RAT_L_U	= rParam.rLowerIso_UpperZoom.nr_str.bits.UDM_XTK_RAT;
	MINT32 UDM_XTK_OFST_L_U = rParam.rLowerIso_UpperZoom.nr_str.bits.UDM_XTK_OFST;
	MINT32 UDM_XTK_SL_L_U	= rParam.rLowerIso_UpperZoom.nr_str.bits.UDM_XTK_SL;
	MINT32 UDM_N0_STR_L_U	= rParam.rLowerIso_UpperZoom.nr_str.bits.UDM_N0_STR;
	MINT32 UDM_N1_STR_L_U	= rParam.rLowerIso_UpperZoom.nr_str.bits.UDM_N1_STR;
	MINT32 UDM_N2_STR_L_U	= rParam.rLowerIso_UpperZoom.nr_str.bits.UDM_N2_STR;
	MINT32 UDM_N0_OFST_L_U	= rParam.rLowerIso_UpperZoom.nr_act.bits.UDM_N0_OFST;
	MINT32 UDM_N1_OFST_L_U	= rParam.rLowerIso_UpperZoom.nr_act.bits.UDM_N1_OFST;
	MINT32 UDM_N2_OFST_L_U	= rParam.rLowerIso_UpperZoom.nr_act.bits.UDM_N2_OFST;
	MINT32 UDM_NSL_L_U		= rParam.rLowerIso_UpperZoom.nr_act.bits.UDM_NSL;
	MINT32 UDM_NGR_L_U		= rParam.rLowerIso_UpperZoom.nr_act.bits.UDM_NGR;
	MINT32 UDM_HA_STR_L_U	= rParam.rLowerIso_UpperZoom.hf_str.bits.UDM_HA_STR;
	MINT32 UDM_H1_GN_L_U	= rParam.rLowerIso_UpperZoom.hf_str.bits.UDM_H1_GN;
	MINT32 UDM_H2_GN_L_U	= rParam.rLowerIso_UpperZoom.hf_str.bits.UDM_H2_GN;
	MINT32 UDM_H3_GN_L_U	= rParam.rLowerIso_UpperZoom.hf_str.bits.UDM_H3_GN;
	MINT32 UDM_HI_RAT_L_U	= rParam.rLowerIso_UpperZoom.hf_str.bits.UDM_HI_RAT;
	MINT32 UDM_H1_LWB_L_U	= rParam.rLowerIso_UpperZoom.hf_act1.bits.UDM_H1_LWB;
	MINT32 UDM_H1_UPB_L_U	= rParam.rLowerIso_UpperZoom.hf_act1.bits.UDM_H1_UPB;
	MINT32 UDM_H2_LWB_L_U	= rParam.rLowerIso_UpperZoom.hf_act1.bits.UDM_H2_LWB;
	MINT32 UDM_H2_UPB_L_U	= rParam.rLowerIso_UpperZoom.hf_act1.bits.UDM_H2_UPB;
	MINT32 UDM_H3_LWB_L_U	= rParam.rLowerIso_UpperZoom.hf_act2.bits.UDM_H3_LWB;
	MINT32 UDM_H3_UPB_L_U	= rParam.rLowerIso_UpperZoom.hf_act2.bits.UDM_H3_UPB;
	MINT32 UDM_HSLL_L_U 	= rParam.rLowerIso_UpperZoom.hf_act2.bits.UDM_HSLL;
	MINT32 UDM_HSLR_L_U 	= rParam.rLowerIso_UpperZoom.hf_act2.bits.UDM_HSLR;
	MINT32 UDM_CORE_TH1_L_U = rParam.rLowerIso_UpperZoom.hftd_ctl.bits.UDM_CORE_TH1;
	MINT32 UDM_CORE_TH2_L_U = rParam.rLowerIso_UpperZoom.hf_str.bits.UDM_CORE_TH2;
	MINT32 UDM_OV_TH_L_U	= rParam.rLowerIso_UpperZoom.clip.bits.UDM_OV_TH;
	MINT32 UDM_UN_TH_L_U	= rParam.rLowerIso_UpperZoom.clip.bits.UDM_UN_TH;
	MINT32 UDM_CLIP_TH_L_U	= rParam.rLowerIso_UpperZoom.clip.bits.UDM_CLIP_TH;
	MINT32 UDM_DS_THB_L_U	= rParam.rLowerIso_UpperZoom.dsb.bits.UDM_DS_THB;
	MINT32 UDM_DS_SLB_L_U	= rParam.rLowerIso_UpperZoom.dsb.bits.UDM_DS_SLB;
	MINT32 UDM_FL_MODE_L_U	= rParam.rLowerIso_UpperZoom.dsb.bits.UDM_FL_MODE;
	MINT32 UDM_SL_RAT_L_U	= rParam.rLowerIso_UpperZoom.dsb.bits.UDM_SL_RAT;
	MINT32 UDM_SC_RAT_L_U	= rParam.rLowerIso_UpperZoom.dsb.bits.UDM_SC_RAT;
	MINT32 UDM_LR_RAT_L_U	= rParam.rLowerIso_UpperZoom.lr_rat.bits.UDM_LR_RAT;

	//Up ISO Low Zoom
	MINT32 UDM_BYP_U_L		= rParam.rUpperIso_LowerZoom.intp_crs.bits.UDM_BYP;
	MINT32 UDM_GRAD_WT_U_L	= rParam.rUpperIso_LowerZoom.intp_crs.bits.UDM_GRAD_WT;
	MINT32 UDM_ETH_U_L		= rParam.rUpperIso_LowerZoom.intp_crs.bits.UDM_ETH;
	MINT32 UDM_FTH_U_L		= rParam.rUpperIso_LowerZoom.intp_crs.bits.UDM_FTH;
	MINT32 UDM_DTH_U_L		= rParam.rUpperIso_LowerZoom.intp_crs.bits.UDM_DTH;
	MINT32 UDM_CD_SLC_U_L	= rParam.rUpperIso_LowerZoom.intp_nat.bits.UDM_CD_SLC;
	MINT32 UDM_CD_SLL_U_L	= rParam.rUpperIso_LowerZoom.intp_nat.bits.UDM_CD_SLL;
	MINT32 UDM_L0_OFST_U_L	= rParam.rUpperIso_LowerZoom.intp_nat.bits.UDM_L0_OFST;
	MINT32 UDM_L0_SL_U_L	= rParam.rUpperIso_LowerZoom.intp_nat.bits.UDM_L0_SL;
	MINT32 UDM_L1_OFST_U_L	= rParam.rUpperIso_LowerZoom.intp_aug.bits.UDM_L1_OFST;
	MINT32 UDM_L1_SL_U_L	= rParam.rUpperIso_LowerZoom.intp_aug.bits.UDM_L1_SL;
	MINT32 UDM_L2_OFST_U_L	= rParam.rUpperIso_LowerZoom.intp_aug.bits.UDM_L2_OFST;
	MINT32 UDM_L2_SL_U_L	= rParam.rUpperIso_LowerZoom.intp_aug.bits.UDM_L2_SL;
	MINT32 UDM_DN_OFST_U_L	= rParam.rUpperIso_LowerZoom.intp_aug.bits.UDM_DN_OFST;
	MINT32 UDM_HL_OFST_U_L	= rParam.rUpperIso_LowerZoom.intp_nat.bits.UDM_HL_OFST;
	MINT32 UDM_LM_Y0_U_L	= rParam.rUpperIso_LowerZoom.luma_lut1.bits.UDM_LM_Y0;
	MINT32 UDM_LM_Y1_U_L	= rParam.rUpperIso_LowerZoom.luma_lut1.bits.UDM_LM_Y1;
	MINT32 UDM_LM_Y2_U_L	= rParam.rUpperIso_LowerZoom.luma_lut1.bits.UDM_LM_Y2;
	MINT32 UDM_LM_Y3_U_L	= rParam.rUpperIso_LowerZoom.luma_lut2.bits.UDM_LM_Y3;
	MINT32 UDM_LM_Y4_U_L	= rParam.rUpperIso_LowerZoom.luma_lut2.bits.UDM_LM_Y4;
	MINT32 UDM_LM_Y5_U_L	= rParam.rUpperIso_LowerZoom.luma_lut2.bits.UDM_LM_Y5;
	MINT32 UDM_SL_Y1_U_L	= rParam.rUpperIso_LowerZoom.sl_ctl.bits.UDM_SL_Y1;
	MINT32 UDM_SL_Y2_U_L	= rParam.rUpperIso_LowerZoom.sl_ctl.bits.UDM_SL_Y2;
	MINT32 UDM_SL_HR_U_L	= rParam.rUpperIso_LowerZoom.sl_ctl.bits.UDM_SL_HR;
	MINT32 UDM_SL_EN_U_L	= rParam.rUpperIso_LowerZoom.sl_ctl.bits.UDM_SL_EN;
	MINT32 UDM_HT_GN1_U_L	= rParam.rUpperIso_LowerZoom.hftd_ctl.bits.UDM_HT_GN1;
	MINT32 UDM_HT_GN2_U_L	= rParam.rUpperIso_LowerZoom.hftd_ctl.bits.UDM_HT_GN2;
	MINT32 UDM_HD_GN1_U_L	= rParam.rUpperIso_LowerZoom.hftd_ctl.bits.UDM_HD_GN1;
	MINT32 UDM_HD_GN2_U_L	= rParam.rUpperIso_LowerZoom.hftd_ctl.bits.UDM_HD_GN2;
	MINT32 UDM_XTK_RAT_U_L	= rParam.rUpperIso_LowerZoom.nr_str.bits.UDM_XTK_RAT;
	MINT32 UDM_XTK_OFST_U_L = rParam.rUpperIso_LowerZoom.nr_str.bits.UDM_XTK_OFST;
	MINT32 UDM_XTK_SL_U_L	= rParam.rUpperIso_LowerZoom.nr_str.bits.UDM_XTK_SL;
	MINT32 UDM_N0_STR_U_L	= rParam.rUpperIso_LowerZoom.nr_str.bits.UDM_N0_STR;
	MINT32 UDM_N1_STR_U_L	= rParam.rUpperIso_LowerZoom.nr_str.bits.UDM_N1_STR;
	MINT32 UDM_N2_STR_U_L	= rParam.rUpperIso_LowerZoom.nr_str.bits.UDM_N2_STR;
	MINT32 UDM_N0_OFST_U_L	= rParam.rUpperIso_LowerZoom.nr_act.bits.UDM_N0_OFST;
	MINT32 UDM_N1_OFST_U_L	= rParam.rUpperIso_LowerZoom.nr_act.bits.UDM_N1_OFST;
	MINT32 UDM_N2_OFST_U_L	= rParam.rUpperIso_LowerZoom.nr_act.bits.UDM_N2_OFST;
	MINT32 UDM_NSL_U_L		= rParam.rUpperIso_LowerZoom.nr_act.bits.UDM_NSL;
	MINT32 UDM_NGR_U_L		= rParam.rUpperIso_LowerZoom.nr_act.bits.UDM_NGR;
	MINT32 UDM_HA_STR_U_L	= rParam.rUpperIso_LowerZoom.hf_str.bits.UDM_HA_STR;
	MINT32 UDM_H1_GN_U_L	= rParam.rUpperIso_LowerZoom.hf_str.bits.UDM_H1_GN;
	MINT32 UDM_H2_GN_U_L	= rParam.rUpperIso_LowerZoom.hf_str.bits.UDM_H2_GN;
	MINT32 UDM_H3_GN_U_L	= rParam.rUpperIso_LowerZoom.hf_str.bits.UDM_H3_GN;
	MINT32 UDM_HI_RAT_U_L	= rParam.rUpperIso_LowerZoom.hf_str.bits.UDM_HI_RAT;
	MINT32 UDM_H1_LWB_U_L	= rParam.rUpperIso_LowerZoom.hf_act1.bits.UDM_H1_LWB;
	MINT32 UDM_H1_UPB_U_L	= rParam.rUpperIso_LowerZoom.hf_act1.bits.UDM_H1_UPB;
	MINT32 UDM_H2_LWB_U_L	= rParam.rUpperIso_LowerZoom.hf_act1.bits.UDM_H2_LWB;
	MINT32 UDM_H2_UPB_U_L	= rParam.rUpperIso_LowerZoom.hf_act1.bits.UDM_H2_UPB;
	MINT32 UDM_H3_LWB_U_L	= rParam.rUpperIso_LowerZoom.hf_act2.bits.UDM_H3_LWB;
	MINT32 UDM_H3_UPB_U_L	= rParam.rUpperIso_LowerZoom.hf_act2.bits.UDM_H3_UPB;
	MINT32 UDM_HSLL_U_L 	= rParam.rUpperIso_LowerZoom.hf_act2.bits.UDM_HSLL;
	MINT32 UDM_HSLR_U_L 	= rParam.rUpperIso_LowerZoom.hf_act2.bits.UDM_HSLR;
	MINT32 UDM_CORE_TH1_U_L = rParam.rUpperIso_LowerZoom.hftd_ctl.bits.UDM_CORE_TH1;
	MINT32 UDM_CORE_TH2_U_L = rParam.rUpperIso_LowerZoom.hf_str.bits.UDM_CORE_TH2;
	MINT32 UDM_OV_TH_U_L	= rParam.rUpperIso_LowerZoom.clip.bits.UDM_OV_TH;
	MINT32 UDM_UN_TH_U_L	= rParam.rUpperIso_LowerZoom.clip.bits.UDM_UN_TH;
	MINT32 UDM_CLIP_TH_U_L	= rParam.rUpperIso_LowerZoom.clip.bits.UDM_CLIP_TH;
	MINT32 UDM_DS_THB_U_L	= rParam.rUpperIso_LowerZoom.dsb.bits.UDM_DS_THB;
	MINT32 UDM_DS_SLB_U_L	= rParam.rUpperIso_LowerZoom.dsb.bits.UDM_DS_SLB;
	MINT32 UDM_FL_MODE_U_L	= rParam.rUpperIso_LowerZoom.dsb.bits.UDM_FL_MODE;
	MINT32 UDM_SL_RAT_U_L	= rParam.rUpperIso_LowerZoom.dsb.bits.UDM_SL_RAT;
	MINT32 UDM_SC_RAT_U_L	= rParam.rUpperIso_LowerZoom.dsb.bits.UDM_SC_RAT;
	MINT32 UDM_LR_RAT_U_L	= rParam.rUpperIso_LowerZoom.lr_rat.bits.UDM_LR_RAT;

	//Up ISO Up Zoom
	MINT32 UDM_BYP_U_U		= rParam.rUpperIso_UpperZoom.intp_crs.bits.UDM_BYP;
	MINT32 UDM_GRAD_WT_U_U	= rParam.rUpperIso_UpperZoom.intp_crs.bits.UDM_GRAD_WT;
	MINT32 UDM_ETH_U_U		= rParam.rUpperIso_UpperZoom.intp_crs.bits.UDM_ETH;
	MINT32 UDM_FTH_U_U		= rParam.rUpperIso_UpperZoom.intp_crs.bits.UDM_FTH;
	MINT32 UDM_DTH_U_U		= rParam.rUpperIso_UpperZoom.intp_crs.bits.UDM_DTH;
	MINT32 UDM_CD_SLC_U_U	= rParam.rUpperIso_UpperZoom.intp_nat.bits.UDM_CD_SLC;
	MINT32 UDM_CD_SLL_U_U	= rParam.rUpperIso_UpperZoom.intp_nat.bits.UDM_CD_SLL;
	MINT32 UDM_L0_OFST_U_U	= rParam.rUpperIso_UpperZoom.intp_nat.bits.UDM_L0_OFST;
	MINT32 UDM_L0_SL_U_U	= rParam.rUpperIso_UpperZoom.intp_nat.bits.UDM_L0_SL;
	MINT32 UDM_L1_OFST_U_U	= rParam.rUpperIso_UpperZoom.intp_aug.bits.UDM_L1_OFST;
	MINT32 UDM_L1_SL_U_U	= rParam.rUpperIso_UpperZoom.intp_aug.bits.UDM_L1_SL;
	MINT32 UDM_L2_OFST_U_U	= rParam.rUpperIso_UpperZoom.intp_aug.bits.UDM_L2_OFST;
	MINT32 UDM_L2_SL_U_U	= rParam.rUpperIso_UpperZoom.intp_aug.bits.UDM_L2_SL;
	MINT32 UDM_DN_OFST_U_U	= rParam.rUpperIso_UpperZoom.intp_aug.bits.UDM_DN_OFST;
	MINT32 UDM_HL_OFST_U_U	= rParam.rUpperIso_UpperZoom.intp_nat.bits.UDM_HL_OFST;
	MINT32 UDM_LM_Y0_U_U	= rParam.rUpperIso_UpperZoom.luma_lut1.bits.UDM_LM_Y0;
	MINT32 UDM_LM_Y1_U_U	= rParam.rUpperIso_UpperZoom.luma_lut1.bits.UDM_LM_Y1;
	MINT32 UDM_LM_Y2_U_U	= rParam.rUpperIso_UpperZoom.luma_lut1.bits.UDM_LM_Y2;
	MINT32 UDM_LM_Y3_U_U	= rParam.rUpperIso_UpperZoom.luma_lut2.bits.UDM_LM_Y3;
	MINT32 UDM_LM_Y4_U_U	= rParam.rUpperIso_UpperZoom.luma_lut2.bits.UDM_LM_Y4;
	MINT32 UDM_LM_Y5_U_U	= rParam.rUpperIso_UpperZoom.luma_lut2.bits.UDM_LM_Y5;
	MINT32 UDM_SL_Y1_U_U	= rParam.rUpperIso_UpperZoom.sl_ctl.bits.UDM_SL_Y1;
	MINT32 UDM_SL_Y2_U_U	= rParam.rUpperIso_UpperZoom.sl_ctl.bits.UDM_SL_Y2;
	MINT32 UDM_SL_HR_U_U	= rParam.rUpperIso_UpperZoom.sl_ctl.bits.UDM_SL_HR;
	MINT32 UDM_SL_EN_U_U	= rParam.rUpperIso_UpperZoom.sl_ctl.bits.UDM_SL_EN;
	MINT32 UDM_HT_GN1_U_U	= rParam.rUpperIso_UpperZoom.hftd_ctl.bits.UDM_HT_GN1;
	MINT32 UDM_HT_GN2_U_U	= rParam.rUpperIso_UpperZoom.hftd_ctl.bits.UDM_HT_GN2;
	MINT32 UDM_HD_GN1_U_U	= rParam.rUpperIso_UpperZoom.hftd_ctl.bits.UDM_HD_GN1;
	MINT32 UDM_HD_GN2_U_U	= rParam.rUpperIso_UpperZoom.hftd_ctl.bits.UDM_HD_GN2;
	MINT32 UDM_XTK_RAT_U_U	= rParam.rUpperIso_UpperZoom.nr_str.bits.UDM_XTK_RAT;
	MINT32 UDM_XTK_OFST_U_U = rParam.rUpperIso_UpperZoom.nr_str.bits.UDM_XTK_OFST;
	MINT32 UDM_XTK_SL_U_U	= rParam.rUpperIso_UpperZoom.nr_str.bits.UDM_XTK_SL;
	MINT32 UDM_N0_STR_U_U	= rParam.rUpperIso_UpperZoom.nr_str.bits.UDM_N0_STR;
	MINT32 UDM_N1_STR_U_U	= rParam.rUpperIso_UpperZoom.nr_str.bits.UDM_N1_STR;
	MINT32 UDM_N2_STR_U_U	= rParam.rUpperIso_UpperZoom.nr_str.bits.UDM_N2_STR;
	MINT32 UDM_N0_OFST_U_U	= rParam.rUpperIso_UpperZoom.nr_act.bits.UDM_N0_OFST;
	MINT32 UDM_N1_OFST_U_U	= rParam.rUpperIso_UpperZoom.nr_act.bits.UDM_N1_OFST;
	MINT32 UDM_N2_OFST_U_U	= rParam.rUpperIso_UpperZoom.nr_act.bits.UDM_N2_OFST;
	MINT32 UDM_NSL_U_U		= rParam.rUpperIso_UpperZoom.nr_act.bits.UDM_NSL;
	MINT32 UDM_NGR_U_U		= rParam.rUpperIso_UpperZoom.nr_act.bits.UDM_NGR;
	MINT32 UDM_HA_STR_U_U	= rParam.rUpperIso_UpperZoom.hf_str.bits.UDM_HA_STR;
	MINT32 UDM_H1_GN_U_U	= rParam.rUpperIso_UpperZoom.hf_str.bits.UDM_H1_GN;
	MINT32 UDM_H2_GN_U_U	= rParam.rUpperIso_UpperZoom.hf_str.bits.UDM_H2_GN;
	MINT32 UDM_H3_GN_U_U	= rParam.rUpperIso_UpperZoom.hf_str.bits.UDM_H3_GN;
	MINT32 UDM_HI_RAT_U_U	= rParam.rUpperIso_UpperZoom.hf_str.bits.UDM_HI_RAT;
	MINT32 UDM_H1_LWB_U_U	= rParam.rUpperIso_UpperZoom.hf_act1.bits.UDM_H1_LWB;
	MINT32 UDM_H1_UPB_U_U	= rParam.rUpperIso_UpperZoom.hf_act1.bits.UDM_H1_UPB;
	MINT32 UDM_H2_LWB_U_U	= rParam.rUpperIso_UpperZoom.hf_act1.bits.UDM_H2_LWB;
	MINT32 UDM_H2_UPB_U_U	= rParam.rUpperIso_UpperZoom.hf_act1.bits.UDM_H2_UPB;
	MINT32 UDM_H3_LWB_U_U	= rParam.rUpperIso_UpperZoom.hf_act2.bits.UDM_H3_LWB;
	MINT32 UDM_H3_UPB_U_U	= rParam.rUpperIso_UpperZoom.hf_act2.bits.UDM_H3_UPB;
	MINT32 UDM_HSLL_U_U 	= rParam.rUpperIso_UpperZoom.hf_act2.bits.UDM_HSLL;
	MINT32 UDM_HSLR_U_U 	= rParam.rUpperIso_UpperZoom.hf_act2.bits.UDM_HSLR;
	MINT32 UDM_CORE_TH1_U_U = rParam.rUpperIso_UpperZoom.hftd_ctl.bits.UDM_CORE_TH1;
	MINT32 UDM_CORE_TH2_U_U = rParam.rUpperIso_UpperZoom.hf_str.bits.UDM_CORE_TH2;
	MINT32 UDM_OV_TH_U_U	= rParam.rUpperIso_UpperZoom.clip.bits.UDM_OV_TH;
	MINT32 UDM_UN_TH_U_U	= rParam.rUpperIso_UpperZoom.clip.bits.UDM_UN_TH;
	MINT32 UDM_CLIP_TH_U_U	= rParam.rUpperIso_UpperZoom.clip.bits.UDM_CLIP_TH;
	MINT32 UDM_DS_THB_U_U	= rParam.rUpperIso_UpperZoom.dsb.bits.UDM_DS_THB;
	MINT32 UDM_DS_SLB_U_U	= rParam.rUpperIso_UpperZoom.dsb.bits.UDM_DS_SLB;
	MINT32 UDM_FL_MODE_U_U	= rParam.rUpperIso_UpperZoom.dsb.bits.UDM_FL_MODE;
	MINT32 UDM_SL_RAT_U_U	= rParam.rUpperIso_UpperZoom.dsb.bits.UDM_SL_RAT;
	MINT32 UDM_SC_RAT_U_U	= rParam.rUpperIso_UpperZoom.dsb.bits.UDM_SC_RAT;
	MINT32 UDM_LR_RAT_U_U	= rParam.rUpperIso_UpperZoom.lr_rat.bits.UDM_LR_RAT;



	MINT32 UDM_BYP		= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_BYP_L_L, UDM_BYP_L_U, UDM_BYP_U_L, UDM_BYP_U_U,0);
	MINT32 UDM_GRAD_WT	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_GRAD_WT_L_L, UDM_GRAD_WT_L_U, UDM_GRAD_WT_U_L, UDM_GRAD_WT_U_U,0);
	MINT32 UDM_ETH		= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_ETH_L_L, UDM_ETH_L_U, UDM_ETH_U_L, UDM_ETH_U_U,0);
	MINT32 UDM_FTH		= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_FTH_L_L, UDM_FTH_L_U, UDM_FTH_U_L, UDM_FTH_U_U,0);
	MINT32 UDM_DTH		= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_DTH_L_L, UDM_DTH_L_U, UDM_DTH_U_L, UDM_DTH_U_U,0);
	MINT32 UDM_CD_SLC	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_CD_SLC_L_L, UDM_CD_SLC_L_U, UDM_CD_SLC_U_L, UDM_CD_SLC_U_U,0);
	MINT32 UDM_CD_SLL	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_CD_SLL_L_L, UDM_CD_SLL_L_U, UDM_CD_SLL_U_L, UDM_CD_SLL_U_U,0);
	MINT32 UDM_L0_OFST	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_L0_OFST_L_L, UDM_L0_OFST_L_U, UDM_L0_OFST_U_L, UDM_L0_OFST_U_U,0);
	MINT32 UDM_L0_SL	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_L0_SL_L_L, UDM_L0_SL_L_U, UDM_L0_SL_U_L, UDM_L0_SL_U_U,0);
	MINT32 UDM_L1_OFST	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_L1_OFST_L_L, UDM_L1_OFST_L_U, UDM_L1_OFST_U_L, UDM_L1_OFST_U_U,0);
	MINT32 UDM_L1_SL	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_L1_SL_L_L, UDM_L1_SL_L_U, UDM_L1_SL_U_L, UDM_L1_SL_U_U,0);
	MINT32 UDM_L2_OFST	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_L2_OFST_L_L, UDM_L2_OFST_L_U, UDM_L2_OFST_U_L, UDM_L2_OFST_U_U,0);
	MINT32 UDM_L2_SL	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_L2_SL_L_L, UDM_L2_SL_L_U, UDM_L2_SL_U_L, UDM_L2_SL_U_U,0);
	MINT32 UDM_DN_OFST	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_DN_OFST_L_L, UDM_DN_OFST_L_U, UDM_DN_OFST_U_L, UDM_DN_OFST_U_U,0);
	MINT32 UDM_HL_OFST	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_HL_OFST_L_L, UDM_HL_OFST_L_U, UDM_HL_OFST_U_L, UDM_HL_OFST_U_U,0);
	MINT32 UDM_LM_Y0	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_LM_Y0_L_L, UDM_LM_Y0_L_U, UDM_LM_Y0_U_L, UDM_LM_Y0_U_U,0);
	MINT32 UDM_LM_Y1	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_LM_Y1_L_L, UDM_LM_Y1_L_U, UDM_LM_Y1_U_L, UDM_LM_Y1_U_U,0);
	MINT32 UDM_LM_Y2	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_LM_Y2_L_L, UDM_LM_Y2_L_U, UDM_LM_Y2_U_L, UDM_LM_Y2_U_U,0);
	MINT32 UDM_LM_Y3	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_LM_Y3_L_L, UDM_LM_Y3_L_U, UDM_LM_Y3_U_L, UDM_LM_Y3_U_U,0);
	MINT32 UDM_LM_Y4	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_LM_Y4_L_L, UDM_LM_Y4_L_U, UDM_LM_Y4_U_L, UDM_LM_Y4_U_U,0);
	MINT32 UDM_LM_Y5	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_LM_Y5_L_L, UDM_LM_Y5_L_U, UDM_LM_Y5_U_L, UDM_LM_Y5_U_U,0);
	MINT32 UDM_SL_Y1	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_SL_Y1_L_L, UDM_SL_Y1_L_U, UDM_SL_Y1_U_L, UDM_SL_Y1_U_U,0);
	MINT32 UDM_SL_Y2	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_SL_Y2_L_L, UDM_SL_Y2_L_U, UDM_SL_Y2_U_L, UDM_SL_Y2_U_U,0);
	MINT32 UDM_SL_HR	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_SL_HR_L_L, UDM_SL_HR_L_U, UDM_SL_HR_U_L, UDM_SL_HR_U_U,0);
	MINT32 UDM_SL_EN	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_SL_EN_L_L, UDM_SL_EN_L_U, UDM_SL_EN_U_L, UDM_SL_EN_U_U,0);
	MINT32 UDM_HT_GN1	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_HT_GN1_L_L, UDM_HT_GN1_L_U, UDM_HT_GN1_U_L, UDM_HT_GN1_U_U,0);
	MINT32 UDM_HT_GN2	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_HT_GN2_L_L, UDM_HT_GN2_L_U, UDM_HT_GN2_U_L, UDM_HT_GN2_U_U,0);
	MINT32 UDM_HD_GN1	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_HD_GN1_L_L, UDM_HD_GN1_L_U, UDM_HD_GN1_U_L, UDM_HD_GN1_U_U,0);
	MINT32 UDM_HD_GN2	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_HD_GN2_L_L, UDM_HD_GN2_L_U, UDM_HD_GN2_U_L, UDM_HD_GN2_U_U,0);
	MINT32 UDM_XTK_RAT	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_XTK_RAT_L_L, UDM_XTK_RAT_L_U, UDM_XTK_RAT_U_L, UDM_XTK_RAT_U_U,0);
	MINT32 UDM_XTK_OFST = InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_XTK_OFST_L_L, UDM_XTK_OFST_L_U, UDM_XTK_OFST_U_L, UDM_XTK_OFST_U_U,0);
	MINT32 UDM_XTK_SL	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_XTK_SL_L_L, UDM_XTK_SL_L_U, UDM_XTK_SL_U_L, UDM_XTK_SL_U_U,0);
	MINT32 UDM_N0_STR	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_N0_STR_L_L, UDM_N0_STR_L_U, UDM_N0_STR_U_L, UDM_N0_STR_U_U,0);
	MINT32 UDM_N1_STR	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_N1_STR_L_L, UDM_N1_STR_L_U, UDM_N1_STR_U_L, UDM_N1_STR_U_U,0);
	MINT32 UDM_N2_STR	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_N2_STR_L_L, UDM_N2_STR_L_U, UDM_N2_STR_U_L, UDM_N2_STR_U_U,0);
	MINT32 UDM_N0_OFST	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_N0_OFST_L_L, UDM_N0_OFST_L_U, UDM_N0_OFST_U_L, UDM_N0_OFST_U_U,0);
	MINT32 UDM_N1_OFST	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_N1_OFST_L_L, UDM_N1_OFST_L_U, UDM_N1_OFST_U_L, UDM_N1_OFST_U_U,0);
	MINT32 UDM_N2_OFST	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_N2_OFST_L_L, UDM_N2_OFST_L_U, UDM_N2_OFST_U_L, UDM_N2_OFST_U_U,0);
	MINT32 UDM_NSL		= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_NSL_L_L, UDM_NSL_L_U, UDM_NSL_U_L, UDM_NSL_U_U,0);
	MINT32 UDM_NGR		= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_NGR_L_L, UDM_NGR_L_U, UDM_NGR_U_L, UDM_NGR_U_U,0);
	MINT32 UDM_HA_STR	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_HA_STR_L_L, UDM_HA_STR_L_U, UDM_HA_STR_U_L, UDM_HA_STR_U_U,0);
	MINT32 UDM_H1_GN	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_H1_GN_L_L, UDM_H1_GN_L_U, UDM_H1_GN_U_L, UDM_H1_GN_U_U,0);
	MINT32 UDM_H2_GN	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_H2_GN_L_L, UDM_H2_GN_L_U, UDM_H2_GN_U_L, UDM_H2_GN_U_U,0);
	MINT32 UDM_H3_GN	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_H3_GN_L_L, UDM_H3_GN_L_U, UDM_H3_GN_U_L, UDM_H3_GN_U_U,0);
	MINT32 UDM_HI_RAT	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_HI_RAT_L_L, UDM_HI_RAT_L_U, UDM_HI_RAT_U_L, UDM_HI_RAT_U_U,0);
	MINT32 UDM_H1_LWB	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_H1_LWB_L_L, UDM_H1_LWB_L_U, UDM_H1_LWB_U_L, UDM_H1_LWB_U_U,0);
	MINT32 UDM_H1_UPB	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_H1_UPB_L_L, UDM_H1_UPB_L_U, UDM_H1_UPB_U_L, UDM_H1_UPB_U_U,0);
	MINT32 UDM_H2_LWB	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_H2_LWB_L_L, UDM_H2_LWB_L_U, UDM_H2_LWB_U_L, UDM_H2_LWB_U_U,0);
	MINT32 UDM_H2_UPB	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_H2_UPB_L_L, UDM_H2_UPB_L_U, UDM_H2_UPB_U_L, UDM_H2_UPB_U_U,0);
	MINT32 UDM_H3_LWB	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_H3_LWB_L_L, UDM_H3_LWB_L_U, UDM_H3_LWB_U_L, UDM_H3_LWB_U_U,0);
	MINT32 UDM_H3_UPB	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_H3_UPB_L_L, UDM_H3_UPB_L_U, UDM_H3_UPB_U_L, UDM_H3_UPB_U_U,0);
	MINT32 UDM_HSLL 	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_HSLL_L_L, UDM_HSLL_L_U, UDM_HSLL_U_L, UDM_HSLL_U_U,0);
	MINT32 UDM_HSLR 	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_HSLR_L_L, UDM_HSLR_L_U, UDM_HSLR_U_L, UDM_HSLR_U_U,0);
	MINT32 UDM_CORE_TH1 = InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_CORE_TH1_L_L, UDM_CORE_TH1_L_U, UDM_CORE_TH1_U_L, UDM_CORE_TH1_U_U,0);
	MINT32 UDM_CORE_TH2 = InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_CORE_TH2_L_L, UDM_CORE_TH2_L_U, UDM_CORE_TH2_U_L, UDM_CORE_TH2_U_U,0);
	MINT32 UDM_OV_TH	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_OV_TH_L_L, UDM_OV_TH_L_U, UDM_OV_TH_U_L, UDM_OV_TH_U_U,0);
	MINT32 UDM_UN_TH	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_UN_TH_L_L, UDM_UN_TH_L_U, UDM_UN_TH_U_L, UDM_UN_TH_U_U,0);
	MINT32 UDM_CLIP_TH	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_CLIP_TH_L_L, UDM_CLIP_TH_L_U, UDM_CLIP_TH_U_L, UDM_CLIP_TH_U_U,0);
	MINT32 UDM_DS_THB	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_DS_THB_L_L, UDM_DS_THB_L_U, UDM_DS_THB_U_L, UDM_DS_THB_U_U,0);
	MINT32 UDM_DS_SLB	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_DS_SLB_L_L, UDM_DS_SLB_L_U, UDM_DS_SLB_U_L, UDM_DS_SLB_U_U,0);
	MINT32 UDM_FL_MODE	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_FL_MODE_L_L, UDM_FL_MODE_L_U, UDM_FL_MODE_U_L, UDM_FL_MODE_U_U,0);
	MINT32 UDM_SL_RAT	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_SL_RAT_L_L, UDM_SL_RAT_L_U, UDM_SL_RAT_U_L, UDM_SL_RAT_U_U,0);
	MINT32 UDM_SC_RAT	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_SC_RAT_L_L, UDM_SC_RAT_L_U, UDM_SC_RAT_U_L, UDM_SC_RAT_U_U,0);
	MINT32 UDM_LR_RAT	= InterParam_UDM(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, UDM_LR_RAT_L_L, UDM_LR_RAT_L_U, UDM_LR_RAT_U_L, UDM_LR_RAT_U_U,0);

	rSmoothUDM.intp_crs.bits.UDM_BYP	   = UDM_BYP;
	rSmoothUDM.intp_crs.bits.UDM_GRAD_WT   = UDM_GRAD_WT;
	rSmoothUDM.intp_crs.bits.UDM_ETH	   = UDM_ETH;
	rSmoothUDM.intp_crs.bits.UDM_FTH	   = UDM_FTH;
	rSmoothUDM.intp_crs.bits.UDM_DTH	   = UDM_DTH;
	rSmoothUDM.intp_nat.bits.UDM_CD_SLC    = UDM_CD_SLC;
	rSmoothUDM.intp_nat.bits.UDM_CD_SLL    = UDM_CD_SLL;
	rSmoothUDM.intp_nat.bits.UDM_L0_OFST   = UDM_L0_OFST;
	rSmoothUDM.intp_nat.bits.UDM_L0_SL	   = UDM_L0_SL;
	rSmoothUDM.intp_aug.bits.UDM_L1_OFST   = UDM_L1_OFST;
	rSmoothUDM.intp_aug.bits.UDM_L1_SL	   = UDM_L1_SL;
	rSmoothUDM.intp_aug.bits.UDM_L2_OFST   = UDM_L2_OFST;
	rSmoothUDM.intp_aug.bits.UDM_L2_SL	   = UDM_L2_SL;
	rSmoothUDM.intp_aug.bits.UDM_DN_OFST   = UDM_DN_OFST;
	rSmoothUDM.intp_nat.bits.UDM_HL_OFST   = UDM_HL_OFST;
	rSmoothUDM.luma_lut1.bits.UDM_LM_Y0    = UDM_LM_Y0;
	rSmoothUDM.luma_lut1.bits.UDM_LM_Y1    = UDM_LM_Y1;
	rSmoothUDM.luma_lut1.bits.UDM_LM_Y2    = UDM_LM_Y2;
	rSmoothUDM.luma_lut2.bits.UDM_LM_Y3    = UDM_LM_Y3;
	rSmoothUDM.luma_lut2.bits.UDM_LM_Y4    = UDM_LM_Y4;
	rSmoothUDM.luma_lut2.bits.UDM_LM_Y5    = UDM_LM_Y5;
	rSmoothUDM.sl_ctl.bits.UDM_SL_Y1	   = UDM_SL_Y1;
	rSmoothUDM.sl_ctl.bits.UDM_SL_Y2	   = UDM_SL_Y2;
	rSmoothUDM.sl_ctl.bits.UDM_SL_HR	   = UDM_SL_HR;
	rSmoothUDM.sl_ctl.bits.UDM_SL_EN	   = UDM_SL_EN;
	rSmoothUDM.hftd_ctl.bits.UDM_HT_GN1    = UDM_HT_GN1;
	rSmoothUDM.hftd_ctl.bits.UDM_HT_GN2    = UDM_HT_GN2;
	rSmoothUDM.hftd_ctl.bits.UDM_HD_GN1    = UDM_HD_GN1;
	rSmoothUDM.hftd_ctl.bits.UDM_HD_GN2    = UDM_HD_GN2;
	rSmoothUDM.nr_str.bits.UDM_XTK_RAT	   = UDM_XTK_RAT;
	rSmoothUDM.nr_str.bits.UDM_XTK_OFST    = UDM_XTK_OFST;
	rSmoothUDM.nr_str.bits.UDM_XTK_SL	   = UDM_XTK_SL;
	rSmoothUDM.nr_str.bits.UDM_N0_STR	   = UDM_N0_STR;
	rSmoothUDM.nr_str.bits.UDM_N1_STR	   = UDM_N1_STR;
	rSmoothUDM.nr_str.bits.UDM_N2_STR	   = UDM_N2_STR;
	rSmoothUDM.nr_act.bits.UDM_N0_OFST	   = UDM_N0_OFST;
	rSmoothUDM.nr_act.bits.UDM_N1_OFST	   = UDM_N1_OFST;
	rSmoothUDM.nr_act.bits.UDM_N2_OFST	   = UDM_N2_OFST;
	rSmoothUDM.nr_act.bits.UDM_NSL		   = UDM_NSL;
	rSmoothUDM.nr_act.bits.UDM_NGR		   = UDM_NGR;
	rSmoothUDM.hf_str.bits.UDM_HA_STR	   = UDM_HA_STR;
	rSmoothUDM.hf_str.bits.UDM_H1_GN	   = UDM_H1_GN;
	rSmoothUDM.hf_str.bits.UDM_H2_GN	   = UDM_H2_GN;
	rSmoothUDM.hf_str.bits.UDM_H3_GN	   = UDM_H3_GN;
	rSmoothUDM.hf_str.bits.UDM_HI_RAT	   = UDM_HI_RAT;
	rSmoothUDM.hf_act1.bits.UDM_H1_LWB	   = UDM_H1_LWB;
	rSmoothUDM.hf_act1.bits.UDM_H1_UPB	   = UDM_H1_UPB;
	rSmoothUDM.hf_act1.bits.UDM_H2_LWB	   = UDM_H2_LWB;
	rSmoothUDM.hf_act1.bits.UDM_H2_UPB	   = UDM_H2_UPB;
	rSmoothUDM.hf_act2.bits.UDM_H3_LWB	   = UDM_H3_LWB;
	rSmoothUDM.hf_act2.bits.UDM_H3_UPB	   = UDM_H3_UPB;
	rSmoothUDM.hf_act2.bits.UDM_HSLL	   = UDM_HSLL;
	rSmoothUDM.hf_act2.bits.UDM_HSLR	   = UDM_HSLR;
	rSmoothUDM.hftd_ctl.bits.UDM_CORE_TH1  = UDM_CORE_TH1;
	rSmoothUDM.hf_str.bits.UDM_CORE_TH2    = UDM_CORE_TH2;
	rSmoothUDM.clip.bits.UDM_OV_TH		   = UDM_OV_TH;
	rSmoothUDM.clip.bits.UDM_UN_TH		   = UDM_UN_TH;
	rSmoothUDM.clip.bits.UDM_CLIP_TH	   = UDM_CLIP_TH;
	rSmoothUDM.dsb.bits.UDM_DS_THB		   = UDM_DS_THB;
	rSmoothUDM.dsb.bits.UDM_DS_SLB		   = UDM_DS_SLB;
	rSmoothUDM.dsb.bits.UDM_FL_MODE 	   = UDM_FL_MODE;
	rSmoothUDM.dsb.bits.UDM_SL_RAT		   = UDM_SL_RAT;
	rSmoothUDM.dsb.bits.UDM_SC_RAT		   = UDM_SC_RAT;
	rSmoothUDM.lr_rat.bits.UDM_LR_RAT	   = UDM_LR_RAT;

#define UDM_DUMP(item1, item2)   \
				do{ 					 \
					INTER_LOG_IF(bSmoothUDMDebug, "[S, L_L, L_U, U_L, U_U]%22s = %d, %d, %d, %d, %d", #item2, rSmoothUDM.item1.bits.item2, rParam.rLowerIso_LowerZoom.item1.bits.item2, rParam.rLowerIso_UpperZoom.item1.bits.item2, rParam.rUpperIso_LowerZoom.item1.bits.item2, rParam.rUpperIso_UpperZoom.item1.bits.item2); \
				}while(0)

			UDM_DUMP(intp_crs, UDM_BYP);
			UDM_DUMP(intp_crs, UDM_GRAD_WT);
			UDM_DUMP(intp_crs, UDM_ETH);
			UDM_DUMP(intp_crs, UDM_FTH);
			UDM_DUMP(intp_crs, UDM_DTH);
			UDM_DUMP(intp_nat, UDM_CD_SLC);
			UDM_DUMP(intp_nat, UDM_CD_SLL);
			UDM_DUMP(intp_nat, UDM_L0_OFST);
			UDM_DUMP(intp_nat, UDM_L0_SL);
			UDM_DUMP(intp_nat, UDM_HL_OFST);
			UDM_DUMP(intp_aug, UDM_L1_OFST);
			UDM_DUMP(intp_aug, UDM_L1_SL);
			UDM_DUMP(intp_aug, UDM_L2_OFST);
			UDM_DUMP(intp_aug, UDM_L2_SL);
			UDM_DUMP(intp_aug, UDM_DN_OFST);
			UDM_DUMP(luma_lut1, UDM_LM_Y0);
			UDM_DUMP(luma_lut1, UDM_LM_Y1);
			UDM_DUMP(luma_lut1, UDM_LM_Y2);
			UDM_DUMP(luma_lut2, UDM_LM_Y3);
			UDM_DUMP(luma_lut2, UDM_LM_Y4);
			UDM_DUMP(luma_lut2, UDM_LM_Y5);
			UDM_DUMP(sl_ctl, UDM_SL_Y1);
			UDM_DUMP(sl_ctl, UDM_SL_Y2);
			UDM_DUMP(sl_ctl, UDM_SL_HR);
			UDM_DUMP(sl_ctl, UDM_SL_EN);
			UDM_DUMP(hftd_ctl, UDM_HT_GN1);
			UDM_DUMP(hftd_ctl, UDM_HT_GN2);
			UDM_DUMP(hftd_ctl, UDM_HD_GN1);
			UDM_DUMP(hftd_ctl, UDM_HD_GN2);
			UDM_DUMP(nr_str, UDM_XTK_RAT);
			UDM_DUMP(nr_str, UDM_XTK_OFST);
			UDM_DUMP(nr_str, UDM_XTK_SL);
			UDM_DUMP(nr_str, UDM_N0_STR);
			UDM_DUMP(nr_str, UDM_N1_STR);
			UDM_DUMP(nr_str, UDM_N2_STR);
			UDM_DUMP(nr_act, UDM_N0_OFST);
			UDM_DUMP(nr_act, UDM_N1_OFST);
			UDM_DUMP(nr_act, UDM_N2_OFST);
			UDM_DUMP(nr_act, UDM_NSL);
			UDM_DUMP(nr_act, UDM_NGR);
			UDM_DUMP(hf_str, UDM_HA_STR);
			UDM_DUMP(hf_str, UDM_H1_GN);
			UDM_DUMP(hf_str, UDM_H2_GN);
			UDM_DUMP(hf_str, UDM_H3_GN);
			UDM_DUMP(hf_str, UDM_HI_RAT);
			UDM_DUMP(hf_act1, UDM_H1_LWB);
			UDM_DUMP(hf_act1, UDM_H1_UPB);
			UDM_DUMP(hf_act1, UDM_H2_LWB);
			UDM_DUMP(hf_act1, UDM_H2_UPB);
			UDM_DUMP(hf_act2, UDM_H3_LWB);
			UDM_DUMP(hf_act2, UDM_H3_UPB);
			UDM_DUMP(hf_act2, UDM_HSLL);
			UDM_DUMP(hf_act2, UDM_HSLR);
			UDM_DUMP(hftd_ctl, UDM_CORE_TH1);
			UDM_DUMP(hf_str, UDM_CORE_TH2);
			UDM_DUMP(clip, UDM_OV_TH);
			UDM_DUMP(clip, UDM_UN_TH);
			UDM_DUMP(clip, UDM_CLIP_TH);
			UDM_DUMP(dsb, UDM_DS_THB);
			UDM_DUMP(dsb, UDM_DS_SLB);
			UDM_DUMP(dsb, UDM_FL_MODE);
			UDM_DUMP(dsb, UDM_SL_RAT);
			UDM_DUMP(dsb, UDM_SC_RAT);
			UDM_DUMP(lr_rat, UDM_LR_RAT);

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
    property_get("debug.smooth_LCE.enable", value, "0");
    MBOOL bSmoothLCEDebug = atoi(value);

    INTER_LOG_IF(bSmoothLCEDebug,"[%s()] u4RealISO: %d, u4UpperISO = %d, u4LowerISO = %d\n", __FUNCTION__, u4RealISO, u4UpperISO, u4LowerISO);

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
            INTER_LOG_IF(bSmoothLCEDebug, "[L,S,U]%22s = %d, %d, %d", #item2, rLowerLCE.item1.bits.item2, rSmoothLCE.item1.bits.item2, rUpperLCE.item1.bits.item2); \
        }while(0)

    LCE_DUMP(qua, LCE_PA);
    LCE_DUMP(qua, LCE_PB);
    LCE_DUMP(qua, LCE_BA);

#undef LCE_DUMP
}
#endif

MINT32 InterParam_NR(double Ratio_L_L, double Ratio_L_U, double Ratio_U_L, double Ratio_U_U, MINT32 PARAM_L_L, MINT32 PARAM_L_U, MINT32 PARAM_U_L, MINT32 PARAM_U_U, MINT32 Method) {
    MINT32 InterPARAM = 0;


    switch(Method) {
    case 1:
        if( Ratio_L_L > Ratio_L_U)
		{
            if( Ratio_L_L > Ratio_U_L)
			{
                InterPARAM = PARAM_L_L;
        	}
        	else
			{
                InterPARAM = PARAM_U_L;
        	}
        }
        else
		{
            if( Ratio_L_L > Ratio_U_L)
			{
                InterPARAM = PARAM_L_U;
        	}
        	else
			{
                InterPARAM = PARAM_U_U;
        	}
        }
        break;
    case 0:
    default:
        InterPARAM = (MINT32)(Ratio_L_L * PARAM_L_L + Ratio_L_U * PARAM_L_U + Ratio_U_L * PARAM_U_L + Ratio_U_U * PARAM_U_U + 0.5);
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

MVOID SmoothANR(ISP_NVRAM_ANR_INT_T const& rParam,   // ANR settings
               ISP_NVRAM_ANR_T& rSmoothANR)
{
    MBOOL bSmoothANRDebug = interpPropertyGet("debug.smooth_anr.enable", MFALSE);
	MINT32 u4RealISO = rParam.u4RealISO;
	MINT32 u4UpperISO = rParam.u4UpperISO;
	MINT32 u4LowerISO = rParam.u4LowerISO;
	MINT32 fRealZoom = rParam.i4ZoomRatio_x100;
	MINT32 fUpperZoom = rParam.u4UpperZoom;
	MINT32 fLowerZoom = rParam.u4LowerZoom;


	INTER_LOG_IF(bSmoothANRDebug,"[%s()] u4RealISO: %d, u4UpperISO = %d, u4LowerISO = %d\n", __FUNCTION__, u4RealISO, u4UpperISO, u4LowerISO);
	INTER_LOG_IF(bSmoothANRDebug,"[%s()] u4RealZoom: %d, u4UpperZoom = %d, u4LowerZoom = %d\n", __FUNCTION__, fRealZoom, fUpperZoom, fLowerZoom);

	// _L_L = Low ISO Low Zoom, _L_U = Low ISO Up Zoom, _U_L = Up ISO Low Zoom, _U_U = Up ISO Up Zoom

	double RATIO_ISO_lo = 0;
	double RATIO_ISO_up = 0;
	double RATIO_Zoom_lo = 0;
	double RATIO_Zoom_up = 0;

    //protection for out-of-bound


    if (u4RealISO <= u4LowerISO)
    {
        RATIO_ISO_lo = 1;
    }
	else if(u4RealISO >= u4UpperISO)
	{
		RATIO_ISO_lo = 0;
	}
	else
	{
		RATIO_ISO_lo = (double)(u4UpperISO - u4RealISO) / (double)(u4UpperISO - u4LowerISO);
	}

	RATIO_ISO_up = 1.0 - RATIO_ISO_lo;

	if (fRealZoom <= fLowerZoom)
    {
        RATIO_Zoom_lo = 1;
    }
	else if(fRealZoom >= fUpperZoom)
	{
		RATIO_Zoom_lo = 0;
	}
	else
	{
		RATIO_Zoom_lo = (double)(fUpperZoom - fRealZoom) / (double)(fUpperZoom - fLowerZoom);
	}

	RATIO_Zoom_up = 1.0 - RATIO_Zoom_lo;

	double RATIO_L_L = (double)(RATIO_ISO_lo * RATIO_Zoom_lo);
	double RATIO_L_U = (double)(RATIO_ISO_lo * RATIO_Zoom_up);
	double RATIO_U_L = (double)(RATIO_ISO_up * RATIO_Zoom_lo);
	double RATIO_U_U = (double)(RATIO_ISO_up * RATIO_Zoom_up);

	//	  ====================================================================================================================================
	//	  Get Low ISO Low Zoom
	//	  ====================================================================================================================================
	// CAM_ANR_CON1 CAM+A20H
	MINT32 ANR_ENC_L_L			 = rParam.rLowerIso_LowerZoom.con1.bits.ANR_ENC;
	MINT32 ANR_ENY_L_L			 = rParam.rLowerIso_LowerZoom.con1.bits.ANR_ENY;
	MINT32 ANR_SCALE_MODE_L_L	 = rParam.rLowerIso_LowerZoom.con1.bits.ANR_SCALE_MODE;
	MINT32 ANR_LCE_LINK_L_L 	 = rParam.rLowerIso_LowerZoom.con1.bits.ANR_LCE_LINK;
	MINT32 ANR_TABLE_EN_L_L 	 = rParam.rLowerIso_LowerZoom.con1.bits.ANR_TABLE_EN;
	MINT32 ANR_TBL_PRC_L_L		 = rParam.rLowerIso_LowerZoom.con1.bits.ANR_TBL_PRC;

	// CAM_ANR_CON2 CAM+A24H
	MINT32 ANR_IMPL_MODE_L_L	 = rParam.rLowerIso_LowerZoom.con2.bits.ANR_IMPL_MODE;
	MINT32 ANR_C_SM_EDGE_L_L	 = rParam.rLowerIso_LowerZoom.con2.bits.ANR_C_SM_EDGE;
	MINT32 ANR_FLT_C_L_L		 = rParam.rLowerIso_LowerZoom.con2.bits.ANR_FLT_C;
	MINT32 ANR_LR_L_L			 = rParam.rLowerIso_LowerZoom.con2.bits.ANR_LR;
	MINT32 ANR_ALPHA_L_L		 = rParam.rLowerIso_LowerZoom.con2.bits.ANR_ALPHA;

	// CAM_ANR_YAD1 CAM+A2CH
	MINT32 ANR_CEN_GAIN_LO_TH_L_L= rParam.rLowerIso_LowerZoom.yad1.bits.ANR_CEN_GAIN_LO_TH;
	MINT32 ANR_CEN_GAIN_HI_TH_L_L= rParam.rLowerIso_LowerZoom.yad1.bits.ANR_CEN_GAIN_HI_TH;
	MINT32 ANR_K_LO_TH_L_L		 = rParam.rLowerIso_LowerZoom.yad1.bits.ANR_K_LO_TH;
	MINT32 ANR_K_HI_TH_L_L		 = rParam.rLowerIso_LowerZoom.yad1.bits.ANR_K_HI_TH;
	MINT32 ANR_K_TH_C_L_L		 = rParam.rLowerIso_LowerZoom.yad1.bits.ANR_K_TH_C;

	// CAM_ANR_YAD2 CAM+A30H
	MINT32 ANR_PTY_VGAIN_L_L	 = rParam.rLowerIso_LowerZoom.yad2.bits.ANR_PTY_VGAIN;
	MINT32 ANR_PTY_GAIN_TH_L_L	 = rParam.rLowerIso_LowerZoom.yad2.bits.ANR_PTY_GAIN_TH;
	MINT32 ANR_K_RAT_L_L	     = rParam.rLowerIso_LowerZoom.yad2.bits.ANR_K_RAT;

	// CAM_ANR_Y4LUT1 CAM+A34H
	MINT32 ANR_Y_CPX1_L_L		 = rParam.rLowerIso_LowerZoom.y4lut1.bits.ANR_Y_CPX1;
	MINT32 ANR_Y_CPX2_L_L		 = rParam.rLowerIso_LowerZoom.y4lut1.bits.ANR_Y_CPX2;
	MINT32 ANR_Y_CPX3_L_L		 = rParam.rLowerIso_LowerZoom.y4lut1.bits.ANR_Y_CPX3;

	// CAM_ANR_Y4LUT2 CAM+A38H
	MINT32 ANR_Y_SCALE_CPY0_L_L  = rParam.rLowerIso_LowerZoom.y4lut2.bits.ANR_Y_SCALE_CPY0;
	MINT32 ANR_Y_SCALE_CPY1_L_L  = rParam.rLowerIso_LowerZoom.y4lut2.bits.ANR_Y_SCALE_CPY1;
	MINT32 ANR_Y_SCALE_CPY2_L_L  = rParam.rLowerIso_LowerZoom.y4lut2.bits.ANR_Y_SCALE_CPY2;
	MINT32 ANR_Y_SCALE_CPY3_L_L  = rParam.rLowerIso_LowerZoom.y4lut2.bits.ANR_Y_SCALE_CPY3;

	// CAM_ANR_Y4LUT3 CAM+A3CH
	MINT32 ANR_Y_SCALE_SP0_L_L	 = rParam.rLowerIso_LowerZoom.y4lut3.bits.ANR_Y_SCALE_SP0;
	MINT32 ANR_Y_SCALE_SP1_L_L	 = rParam.rLowerIso_LowerZoom.y4lut3.bits.ANR_Y_SCALE_SP1;
	MINT32 ANR_Y_SCALE_SP2_L_L	 = rParam.rLowerIso_LowerZoom.y4lut3.bits.ANR_Y_SCALE_SP2;
	MINT32 ANR_Y_SCALE_SP3_L_L	 = rParam.rLowerIso_LowerZoom.y4lut3.bits.ANR_Y_SCALE_SP3;

	// CAM_ANR_C4LUT1
	MINT32 ANR_C_CPX1_L_L		 = rParam.rLowerIso_LowerZoom.c4lut1.bits.ANR_C_CPX1;
	MINT32 ANR_C_CPX2_L_L		 = rParam.rLowerIso_LowerZoom.c4lut1.bits.ANR_C_CPX2;
	MINT32 ANR_C_CPX3_L_L		 = rParam.rLowerIso_LowerZoom.c4lut1.bits.ANR_C_CPX3;

	// CAM_ANR_C4LUT2
	MINT32 ANR_C_SCALE_CPY0_L_L  = rParam.rLowerIso_LowerZoom.c4lut2.bits.ANR_C_SCALE_CPY0;
	MINT32 ANR_C_SCALE_CPY1_L_L  = rParam.rLowerIso_LowerZoom.c4lut2.bits.ANR_C_SCALE_CPY1;
	MINT32 ANR_C_SCALE_CPY2_L_L  = rParam.rLowerIso_LowerZoom.c4lut2.bits.ANR_C_SCALE_CPY2;
	MINT32 ANR_C_SCALE_CPY3_L_L  = rParam.rLowerIso_LowerZoom.c4lut2.bits.ANR_C_SCALE_CPY3;

	// CAM_ANR_C4LUT3
	MINT32 ANR_C_SCALE_SP0_L_L	 = rParam.rLowerIso_LowerZoom.c4lut3.bits.ANR_C_SCALE_SP0;
	MINT32 ANR_C_SCALE_SP1_L_L	 = rParam.rLowerIso_LowerZoom.c4lut3.bits.ANR_C_SCALE_SP1;
	MINT32 ANR_C_SCALE_SP2_L_L	 = rParam.rLowerIso_LowerZoom.c4lut3.bits.ANR_C_SCALE_SP2;
	MINT32 ANR_C_SCALE_SP3_L_L	 = rParam.rLowerIso_LowerZoom.c4lut3.bits.ANR_C_SCALE_SP3;

	// CAM_ANR_A4LUT2
	MINT32 ANR_Y_ACT_CPY0_L_L	 = rParam.rLowerIso_LowerZoom.a4lut2.bits.ANR_Y_ACT_CPY0;
	MINT32 ANR_Y_ACT_CPY1_L_L	 = rParam.rLowerIso_LowerZoom.a4lut2.bits.ANR_Y_ACT_CPY1;
	MINT32 ANR_Y_ACT_CPY2_L_L	 = rParam.rLowerIso_LowerZoom.a4lut2.bits.ANR_Y_ACT_CPY2;
	MINT32 ANR_Y_ACT_CPY3_L_L	 = rParam.rLowerIso_LowerZoom.a4lut2.bits.ANR_Y_ACT_CPY3;

	// CAM_ANR_A4LUT3
	MINT32 ANR_Y_ACT_SP0_L_L	 = rParam.rLowerIso_LowerZoom.a4lut3.bits.ANR_Y_ACT_SP0;
	MINT32 ANR_Y_ACT_SP1_L_L	 = rParam.rLowerIso_LowerZoom.a4lut3.bits.ANR_Y_ACT_SP1;
	MINT32 ANR_Y_ACT_SP2_L_L	 = rParam.rLowerIso_LowerZoom.a4lut3.bits.ANR_Y_ACT_SP2;
	MINT32 ANR_Y_ACT_SP3_L_L	 = rParam.rLowerIso_LowerZoom.a4lut3.bits.ANR_Y_ACT_SP3;

	// CAM_ANR_L4LUT1
	MINT32 ANR_LCE_X1_L_L		 = rParam.rLowerIso_LowerZoom.l4lut1.bits.ANR_LCE_X1;
	MINT32 ANR_LCE_X2_L_L		 = rParam.rLowerIso_LowerZoom.l4lut1.bits.ANR_LCE_X2;
	MINT32 ANR_LCE_X3_L_L		 = rParam.rLowerIso_LowerZoom.l4lut1.bits.ANR_LCE_X3;

	// CAM_ANR_L4LUT2
	MINT32 ANR_LCE_GAIN0_L_L	 = rParam.rLowerIso_LowerZoom.l4lut2.bits.ANR_LCE_GAIN0;
	MINT32 ANR_LCE_GAIN1_L_L	 = rParam.rLowerIso_LowerZoom.l4lut2.bits.ANR_LCE_GAIN1;
	MINT32 ANR_LCE_GAIN2_L_L	 = rParam.rLowerIso_LowerZoom.l4lut2.bits.ANR_LCE_GAIN2;
	MINT32 ANR_LCE_GAIN3_L_L	 = rParam.rLowerIso_LowerZoom.l4lut2.bits.ANR_LCE_GAIN3;

	// CAM_ANR_L4LUT3
	MINT32 ANR_LCE_SP0_L_L		 = rParam.rLowerIso_LowerZoom.l4lut3.bits.ANR_LCE_SP0;
	MINT32 ANR_LCE_SP1_L_L		 = rParam.rLowerIso_LowerZoom.l4lut3.bits.ANR_LCE_SP1;
	MINT32 ANR_LCE_SP2_L_L		 = rParam.rLowerIso_LowerZoom.l4lut3.bits.ANR_LCE_SP2;
	MINT32 ANR_LCE_SP3_L_L		 = rParam.rLowerIso_LowerZoom.l4lut3.bits.ANR_LCE_SP3;

	// CAM_ANR_PTY CAM+A40H
	MINT32 ANR_PTY1_L_L 		 = rParam.rLowerIso_LowerZoom.pty.bits.ANR_PTY1;
	MINT32 ANR_PTY2_L_L 		 = rParam.rLowerIso_LowerZoom.pty.bits.ANR_PTY2;
	MINT32 ANR_PTY3_L_L 		 = rParam.rLowerIso_LowerZoom.pty.bits.ANR_PTY3;
	MINT32 ANR_PTY4_L_L 		 = rParam.rLowerIso_LowerZoom.pty.bits.ANR_PTY4;

	// CAM_ANR_CAD CAM+A44H
	MINT32 ANR_PTC_VGAIN_L_L	 = rParam.rLowerIso_LowerZoom.cad.bits.ANR_PTC_VGAIN;
	MINT32 ANR_PTC_GAIN_TH_L_L	 = rParam.rLowerIso_LowerZoom.cad.bits.ANR_PTC_GAIN_TH;
	MINT32 ANR_C_L_DIFF_TH_L_L	 = rParam.rLowerIso_LowerZoom.cad.bits.ANR_C_L_DIFF_TH;

	// CAM_ANR_PTC CAM+A48H
	MINT32 ANR_PTC1_L_L 		 = rParam.rLowerIso_LowerZoom.ptc.bits.ANR_PTC1;
	MINT32 ANR_PTC2_L_L 		 = rParam.rLowerIso_LowerZoom.ptc.bits.ANR_PTC2;
	MINT32 ANR_PTC3_L_L 		 = rParam.rLowerIso_LowerZoom.ptc.bits.ANR_PTC3;
	MINT32 ANR_PTC4_L_L 		 = rParam.rLowerIso_LowerZoom.ptc.bits.ANR_PTC4;

	// CAM_ANR_LCE CAM+A4CH
	MINT32 ANR_LCE_C_GAIN_L_L	 = rParam.rLowerIso_LowerZoom.lce.bits.ANR_LCE_C_GAIN;
	MINT32 ANR_LCE_SCALE_GAIN_L_L= rParam.rLowerIso_LowerZoom.lce.bits.ANR_LCE_SCALE_GAIN;
	MINT32 ANR_LM_WT_L_L		 = rParam.rLowerIso_LowerZoom.lce.bits.ANR_LM_WT;



	// CAM_ANR_ACT1 CAM+A64H
	MINT32 ANR_ACT_LCE_GAIN_L_L  = rParam.rLowerIso_LowerZoom.act1.bits.ANR_ACT_LCE_GAIN;
	MINT32 ANR_ACT_DIF_GAIN_L_L  = rParam.rLowerIso_LowerZoom.act1.bits.ANR_ACT_DIF_GAIN;
	MINT32 ANR_ACT_DIF_LO_TH_L_L = rParam.rLowerIso_LowerZoom.act1.bits.ANR_ACT_DIF_LO_TH;

	// CAM_ANR_ACT2 CAM+A64H
	MINT32 ANR_ACT_SIZE_GAIN_L_L = rParam.rLowerIso_LowerZoom.act2.bits.ANR_ACT_SIZE_GAIN;
	MINT32 ANR_ACT_SIZE_LO_TH_L_L= rParam.rLowerIso_LowerZoom.act2.bits.ANR_ACT_SIZE_LO_TH;

	// CAM_ANR_ACTC CAM+A64H
	MINT32 ANR_ACT_BLD_BASE_C_L_L= rParam.rLowerIso_LowerZoom.actc.bits.ANR_ACT_BLD_BASE_C;
	MINT32 ANR_C_DITH_U_L_L= rParam.rLowerIso_LowerZoom.actc.bits.ANR_C_DITH_U;
	MINT32 ANR_C_DITH_V_L_L= rParam.rLowerIso_LowerZoom.actc.bits.ANR_C_DITH_V;

	// DIP_X_ANR_ACTYL CAM+A60H
	MINT32 ANR_ACT_TH_Y_L_L_L = rParam.rLowerIso_LowerZoom.actyl.bits.ANR_ACT_TH_Y_L;
	MINT32 ANR_ACT_BLD_BASE_Y_L_L_L = rParam.rLowerIso_LowerZoom.actyl.bits.ANR_ACT_BLD_BASE_Y_L;
	MINT32 ANR_ACT_SLANT_Y_L_L_L = rParam.rLowerIso_LowerZoom.actyl.bits.ANR_ACT_SLANT_Y_L;
	MINT32 ANR_ACT_BLD_TH_Y_L_L_L = rParam.rLowerIso_LowerZoom.actyl.bits.ANR_ACT_BLD_TH_Y_L;

	// DIP_X_ANR_YLAD CAM+A60H
	MINT32 ANR_CEN_GAIN_LO_TH_LPF_L_L = rParam.rLowerIso_LowerZoom.ylad.bits.ANR_CEN_GAIN_LO_TH_LPF;
	MINT32 ANR_CEN_GAIN_HI_TH_LPF_L_L = rParam.rLowerIso_LowerZoom.ylad.bits.ANR_CEN_GAIN_HI_TH_LPF;
	MINT32 ANR_K_LMT_L_L = rParam.rLowerIso_LowerZoom.ylad.bits.ANR_K_LMT;
	MINT32 ANR_K_LPF_L_L = rParam.rLowerIso_LowerZoom.ylad.bits.ANR_K_LPF;

	// DIP_X_ANR_PTYL CAM+A60H
	MINT32 ANR_PTY1_LPF_L_L = rParam.rLowerIso_LowerZoom.ptyl.bits.ANR_PTY1_LPF;
	MINT32 ANR_PTY2_LPF_L_L = rParam.rLowerIso_LowerZoom.ptyl.bits.ANR_PTY2_LPF;
	MINT32 ANR_PTY3_LPF_L_L = rParam.rLowerIso_LowerZoom.ptyl.bits.ANR_PTY3_LPF;
	MINT32 ANR_PTY4_LPF_L_L = rParam.rLowerIso_LowerZoom.ptyl.bits.ANR_PTY4_LPF;

	// DIP_X_ANR_LCOEF CAM+A60H
	MINT32 ANR_COEF_A_L_L = rParam.rLowerIso_LowerZoom.lcoef.bits.ANR_COEF_A;
	MINT32 ANR_COEF_B_L_L = rParam.rLowerIso_LowerZoom.lcoef.bits.ANR_COEF_B;
	MINT32 ANR_COEF_C_L_L = rParam.rLowerIso_LowerZoom.lcoef.bits.ANR_COEF_C;
	MINT32 ANR_COEF_D_L_L = rParam.rLowerIso_LowerZoom.lcoef.bits.ANR_COEF_D;

	// Olympu
	MINT32 ANR_C_SM_EDGE_TH_L_L 	  = rParam.rLowerIso_LowerZoom.con2.bits.ANR_C_SM_EDGE_TH;
	MINT32 ANR_K_HPF_L_L			  = rParam.rLowerIso_LowerZoom.ylad.bits.ANR_K_HPF;
	MINT32 ANR_ACT_DIF_HI_TH_L_L	  = rParam.rLowerIso_LowerZoom.act2.bits.ANR_ACT_DIF_HI_TH;
	MINT32 ANR_ACT_TH_Y_HL_L_L		  = rParam.rLowerIso_LowerZoom.actyhl.bits.ANR_ACT_TH_Y_HL;
	MINT32 ANR_ACT_BLD_BASE_Y_HL_L_L  = rParam.rLowerIso_LowerZoom.actyhl.bits.ANR_ACT_BLD_BASE_Y_HL;
	MINT32 ANR_ACT_SLANT_Y_HL_L_L	  = rParam.rLowerIso_LowerZoom.actyhl.bits.ANR_ACT_SLANT_Y_HL;
	MINT32 ANR_ACT_BLD_TH_Y_HL_L_L	  = rParam.rLowerIso_LowerZoom.actyhl.bits.ANR_ACT_BLD_TH_Y_HL;
	MINT32 ANR_ACT_TH_Y_HH_L_L		  = rParam.rLowerIso_LowerZoom.actyhh.bits.ANR_ACT_TH_Y_HH;
	MINT32 ANR_ACT_BLD_BASE_Y_HH_L_L  = rParam.rLowerIso_LowerZoom.actyhh.bits.ANR_ACT_BLD_BASE_Y_HH;
	MINT32 ANR_ACT_SLANT_Y_HH_L_L	  = rParam.rLowerIso_LowerZoom.actyhh.bits.ANR_ACT_SLANT_Y_HH;
	MINT32 ANR_ACT_BLD_TH_Y_HH_L_L	  = rParam.rLowerIso_LowerZoom.actyhh.bits.ANR_ACT_BLD_TH_Y_HH;
	MINT32 ANR_LUMA_TH_Y_L_L_L		  = rParam.rLowerIso_LowerZoom.actyl2.bits.ANR_LUMA_TH_Y_L;
	MINT32 ANR_LUMA_BLD_BASE_Y_L_L_L  = rParam.rLowerIso_LowerZoom.actyl2.bits.ANR_LUMA_BLD_BASE_Y_L;
	MINT32 ANR_LUMA_SLANT_Y_L_L_L	  = rParam.rLowerIso_LowerZoom.actyl2.bits.ANR_LUMA_SLANT_Y_L;
	MINT32 ANR_LUMA_BLD_TH_Y_L_L_L	  = rParam.rLowerIso_LowerZoom.actyl2.bits.ANR_LUMA_BLD_TH_Y_L;
	MINT32 ANR_LUMA_TH_Y_HL_L_L 	  = rParam.rLowerIso_LowerZoom.actyhl2.bits.ANR_LUMA_TH_Y_HL;
	MINT32 ANR_LUMA_BLD_BASE_Y_HL_L_L = rParam.rLowerIso_LowerZoom.actyhl2.bits.ANR_LUMA_BLD_BASE_Y_HL;
	MINT32 ANR_LUMA_SLANT_Y_HL_L_L	  = rParam.rLowerIso_LowerZoom.actyhl2.bits.ANR_LUMA_SLANT_Y_HL;
	MINT32 ANR_LUMA_BLD_TH_Y_HL_L_L   = rParam.rLowerIso_LowerZoom.actyhl2.bits.ANR_LUMA_BLD_TH_Y_HL;
	MINT32 ANR_LUMA_TH_Y_HH_L_L 	  = rParam.rLowerIso_LowerZoom.actyhh2.bits.ANR_LUMA_TH_Y_HH;
	MINT32 ANR_LUMA_BLD_BASE_Y_HH_L_L = rParam.rLowerIso_LowerZoom.actyhh2.bits.ANR_LUMA_BLD_BASE_Y_HH;
	MINT32 ANR_LUMA_SLANT_Y_HH_L_L	  = rParam.rLowerIso_LowerZoom.actyhh2.bits.ANR_LUMA_SLANT_Y_HH;
	MINT32 ANR_LUMA_BLD_TH_Y_HH_L_L   = rParam.rLowerIso_LowerZoom.actyhh2.bits.ANR_LUMA_BLD_TH_Y_HH;
	MINT32 ANR_PTY_L_GAIN_L_L		  = rParam.rLowerIso_LowerZoom.ylad2.bits.ANR_PTY_L_GAIN;
	MINT32 ANR_PTY_H_GAIN_L_L		  = rParam.rLowerIso_LowerZoom.ylad2.bits.ANR_PTY_H_GAIN;
	MINT32 ANR_BLEND_TH_L_L_L		  = rParam.rLowerIso_LowerZoom.ylad2.bits.ANR_BLEND_TH_L;
	MINT32 ANR_BLEND_TH_H_L_L		  = rParam.rLowerIso_LowerZoom.ylad2.bits.ANR_BLEND_TH_H;
	MINT32 ANR_HI_LMT1_L_L			  = rParam.rLowerIso_LowerZoom.ylad3.bits.ANR_HI_LMT1;
	MINT32 ANR_HI_LMT2_L_L			  = rParam.rLowerIso_LowerZoom.ylad3.bits.ANR_HI_LMT2;
	MINT32 ANR_LO_LMT1_L_L			  = rParam.rLowerIso_LowerZoom.ylad3.bits.ANR_LO_LMT1;
	MINT32 ANR_LO_LMT2_L_L			  = rParam.rLowerIso_LowerZoom.ylad3.bits.ANR_LO_LMT2;
	MINT32 ANR_Y_ACT_CEN_OFT_L_L	  = rParam.rLowerIso_LowerZoom.act4.bits.ANR_Y_ACT_CEN_OFT;
	MINT32 ANR_Y_ACT_CEN_GAIN_L_L	  = rParam.rLowerIso_LowerZoom.act4.bits.ANR_Y_ACT_CEN_GAIN;
	MINT32 ANR_Y_ACT_CEN_TH_L_L 	  = rParam.rLowerIso_LowerZoom.act4.bits.ANR_Y_ACT_CEN_TH;
	MINT32 ANR_Y_DIR_L_L			  = rParam.rLowerIso_LowerZoom.con1.bits.ANR_Y_DIR;
	MINT32 ANR_Y_DIR_GAIN_L_L		  = rParam.rLowerIso_LowerZoom.ydir.bits.ANR_Y_DIR_GAIN;
	MINT32 ANR_Y_DIR_VER_W_TH_L_L	  = rParam.rLowerIso_LowerZoom.ydir.bits.ANR_Y_DIR_VER_W_TH;
	MINT32 ANR_Y_DIR_VER_W_SL_L_L	  = rParam.rLowerIso_LowerZoom.ydir.bits.ANR_Y_DIR_VER_W_SL;
	MINT32 ANR_Y_DIR_DIAG_W_TH_L_L	  = rParam.rLowerIso_LowerZoom.ydir.bits.ANR_Y_DIR_DIAG_W_TH;
	MINT32 ANR_Y_DIR_DIAG_W_SL_L_L	  = rParam.rLowerIso_LowerZoom.ydir.bits.ANR_Y_DIR_DIAG_W_SL;

	// Whitney E2

	MINT32 ANR_TBL_CPX1_L_L		 = rParam.rLowerIso_LowerZoom.t4lut1.bits.ANR_TBL_CPX1;
	MINT32 ANR_TBL_CPX2_L_L		 = rParam.rLowerIso_LowerZoom.t4lut1.bits.ANR_TBL_CPX2;
	MINT32 ANR_TBL_CPX3_L_L		 = rParam.rLowerIso_LowerZoom.t4lut1.bits.ANR_TBL_CPX3;
	MINT32 ANR_TBL_GAIN_CPY0_L_L		 = rParam.rLowerIso_LowerZoom.t4lut2.bits.ANR_TBL_GAIN_CPY0;
	MINT32 ANR_TBL_GAIN_CPY1_L_L		 = rParam.rLowerIso_LowerZoom.t4lut2.bits.ANR_TBL_GAIN_CPY1;
	MINT32 ANR_TBL_GAIN_CPY2_L_L		 = rParam.rLowerIso_LowerZoom.t4lut2.bits.ANR_TBL_GAIN_CPY2;
	MINT32 ANR_TBL_GAIN_CPY3_L_L		 = rParam.rLowerIso_LowerZoom.t4lut2.bits.ANR_TBL_GAIN_CPY3;
	MINT32 ANR_TBL_GAIN_SP0_L_L		 = rParam.rLowerIso_LowerZoom.t4lut3.bits.ANR_TBL_GAIN_SP0;
	MINT32 ANR_TBL_GAIN_SP1_L_L		 = rParam.rLowerIso_LowerZoom.t4lut3.bits.ANR_TBL_GAIN_SP1;
	MINT32 ANR_TBL_GAIN_SP2_L_L		 = rParam.rLowerIso_LowerZoom.t4lut3.bits.ANR_TBL_GAIN_SP2;
	MINT32 ANR_TBL_GAIN_SP3_L_L		 = rParam.rLowerIso_LowerZoom.t4lut3.bits.ANR_TBL_GAIN_SP3;


	//	  ====================================================================================================================================
	//	  Get Low ISO Up Zoom
	//	  ====================================================================================================================================
	// CAM_ANR_CON1 CAM+A20H
	MINT32 ANR_ENC_L_U			 = rParam.rLowerIso_UpperZoom.con1.bits.ANR_ENC;
	MINT32 ANR_ENY_L_U			 = rParam.rLowerIso_UpperZoom.con1.bits.ANR_ENY;
	MINT32 ANR_SCALE_MODE_L_U	 = rParam.rLowerIso_UpperZoom.con1.bits.ANR_SCALE_MODE;
	MINT32 ANR_LCE_LINK_L_U 	 = rParam.rLowerIso_UpperZoom.con1.bits.ANR_LCE_LINK;
	MINT32 ANR_TABLE_EN_L_U 	 = rParam.rLowerIso_UpperZoom.con1.bits.ANR_TABLE_EN;
	MINT32 ANR_TBL_PRC_L_U		 = rParam.rLowerIso_UpperZoom.con1.bits.ANR_TBL_PRC;

	// CAM_ANR_CON2 CAM+A24H
	MINT32 ANR_IMPL_MODE_L_U	 = rParam.rLowerIso_UpperZoom.con2.bits.ANR_IMPL_MODE;
	MINT32 ANR_C_SM_EDGE_L_U	 = rParam.rLowerIso_UpperZoom.con2.bits.ANR_C_SM_EDGE;
	MINT32 ANR_FLT_C_L_U		 = rParam.rLowerIso_UpperZoom.con2.bits.ANR_FLT_C;
	MINT32 ANR_LR_L_U			 = rParam.rLowerIso_UpperZoom.con2.bits.ANR_LR;
	MINT32 ANR_ALPHA_L_U		 = rParam.rLowerIso_UpperZoom.con2.bits.ANR_ALPHA;

	// CAM_ANR_YAD1 CAM+A2CH
	MINT32 ANR_CEN_GAIN_LO_TH_L_U= rParam.rLowerIso_UpperZoom.yad1.bits.ANR_CEN_GAIN_LO_TH;
	MINT32 ANR_CEN_GAIN_HI_TH_L_U= rParam.rLowerIso_UpperZoom.yad1.bits.ANR_CEN_GAIN_HI_TH;
	MINT32 ANR_K_LO_TH_L_U		 = rParam.rLowerIso_UpperZoom.yad1.bits.ANR_K_LO_TH;
	MINT32 ANR_K_HI_TH_L_U		 = rParam.rLowerIso_UpperZoom.yad1.bits.ANR_K_HI_TH;
	MINT32 ANR_K_TH_C_L_U		 = rParam.rLowerIso_UpperZoom.yad1.bits.ANR_K_TH_C;

	// CAM_ANR_YAD2 CAM+A30H
	MINT32 ANR_PTY_VGAIN_L_U	 = rParam.rLowerIso_UpperZoom.yad2.bits.ANR_PTY_VGAIN;
	MINT32 ANR_PTY_GAIN_TH_L_U	 = rParam.rLowerIso_UpperZoom.yad2.bits.ANR_PTY_GAIN_TH;
	MINT32 ANR_K_RAT_L_U	     = rParam.rLowerIso_UpperZoom.yad2.bits.ANR_K_RAT;

	// CAM_ANR_Y4LUT1 CAM+A34H
	MINT32 ANR_Y_CPX1_L_U		 = rParam.rLowerIso_UpperZoom.y4lut1.bits.ANR_Y_CPX1;
	MINT32 ANR_Y_CPX2_L_U		 = rParam.rLowerIso_UpperZoom.y4lut1.bits.ANR_Y_CPX2;
	MINT32 ANR_Y_CPX3_L_U		 = rParam.rLowerIso_UpperZoom.y4lut1.bits.ANR_Y_CPX3;

	// CAM_ANR_Y4LUT2 CAM+A38H
	MINT32 ANR_Y_SCALE_CPY0_L_U  = rParam.rLowerIso_UpperZoom.y4lut2.bits.ANR_Y_SCALE_CPY0;
	MINT32 ANR_Y_SCALE_CPY1_L_U  = rParam.rLowerIso_UpperZoom.y4lut2.bits.ANR_Y_SCALE_CPY1;
	MINT32 ANR_Y_SCALE_CPY2_L_U  = rParam.rLowerIso_UpperZoom.y4lut2.bits.ANR_Y_SCALE_CPY2;
	MINT32 ANR_Y_SCALE_CPY3_L_U  = rParam.rLowerIso_UpperZoom.y4lut2.bits.ANR_Y_SCALE_CPY3;

	// CAM_ANR_Y4LUT3 CAM+A3CH
	MINT32 ANR_Y_SCALE_SP0_L_U	 = rParam.rLowerIso_UpperZoom.y4lut3.bits.ANR_Y_SCALE_SP0;
	MINT32 ANR_Y_SCALE_SP1_L_U	 = rParam.rLowerIso_UpperZoom.y4lut3.bits.ANR_Y_SCALE_SP1;
	MINT32 ANR_Y_SCALE_SP2_L_U	 = rParam.rLowerIso_UpperZoom.y4lut3.bits.ANR_Y_SCALE_SP2;
	MINT32 ANR_Y_SCALE_SP3_L_U	 = rParam.rLowerIso_UpperZoom.y4lut3.bits.ANR_Y_SCALE_SP3;

	// CAM_ANR_C4LUT1
	MINT32 ANR_C_CPX1_L_U		 = rParam.rLowerIso_UpperZoom.c4lut1.bits.ANR_C_CPX1;
	MINT32 ANR_C_CPX2_L_U		 = rParam.rLowerIso_UpperZoom.c4lut1.bits.ANR_C_CPX2;
	MINT32 ANR_C_CPX3_L_U		 = rParam.rLowerIso_UpperZoom.c4lut1.bits.ANR_C_CPX3;

	// CAM_ANR_C4LUT2
	MINT32 ANR_C_SCALE_CPY0_L_U  = rParam.rLowerIso_UpperZoom.c4lut2.bits.ANR_C_SCALE_CPY0;
	MINT32 ANR_C_SCALE_CPY1_L_U  = rParam.rLowerIso_UpperZoom.c4lut2.bits.ANR_C_SCALE_CPY1;
	MINT32 ANR_C_SCALE_CPY2_L_U  = rParam.rLowerIso_UpperZoom.c4lut2.bits.ANR_C_SCALE_CPY2;
	MINT32 ANR_C_SCALE_CPY3_L_U  = rParam.rLowerIso_UpperZoom.c4lut2.bits.ANR_C_SCALE_CPY3;

	// CAM_ANR_C4LUT3
	MINT32 ANR_C_SCALE_SP0_L_U	 = rParam.rLowerIso_UpperZoom.c4lut3.bits.ANR_C_SCALE_SP0;
	MINT32 ANR_C_SCALE_SP1_L_U	 = rParam.rLowerIso_UpperZoom.c4lut3.bits.ANR_C_SCALE_SP1;
	MINT32 ANR_C_SCALE_SP2_L_U	 = rParam.rLowerIso_UpperZoom.c4lut3.bits.ANR_C_SCALE_SP2;
	MINT32 ANR_C_SCALE_SP3_L_U	 = rParam.rLowerIso_UpperZoom.c4lut3.bits.ANR_C_SCALE_SP3;

	// CAM_ANR_A4LUT2
	MINT32 ANR_Y_ACT_CPY0_L_U	 = rParam.rLowerIso_UpperZoom.a4lut2.bits.ANR_Y_ACT_CPY0;
	MINT32 ANR_Y_ACT_CPY1_L_U	 = rParam.rLowerIso_UpperZoom.a4lut2.bits.ANR_Y_ACT_CPY1;
	MINT32 ANR_Y_ACT_CPY2_L_U	 = rParam.rLowerIso_UpperZoom.a4lut2.bits.ANR_Y_ACT_CPY2;
	MINT32 ANR_Y_ACT_CPY3_L_U	 = rParam.rLowerIso_UpperZoom.a4lut2.bits.ANR_Y_ACT_CPY3;

	// CAM_ANR_A4LUT3
	MINT32 ANR_Y_ACT_SP0_L_U	 = rParam.rLowerIso_UpperZoom.a4lut3.bits.ANR_Y_ACT_SP0;
	MINT32 ANR_Y_ACT_SP1_L_U	 = rParam.rLowerIso_UpperZoom.a4lut3.bits.ANR_Y_ACT_SP1;
	MINT32 ANR_Y_ACT_SP2_L_U	 = rParam.rLowerIso_UpperZoom.a4lut3.bits.ANR_Y_ACT_SP2;
	MINT32 ANR_Y_ACT_SP3_L_U	 = rParam.rLowerIso_UpperZoom.a4lut3.bits.ANR_Y_ACT_SP3;

	// CAM_ANR_L4LUT1
	MINT32 ANR_LCE_X1_L_U		 = rParam.rLowerIso_UpperZoom.l4lut1.bits.ANR_LCE_X1;
	MINT32 ANR_LCE_X2_L_U		 = rParam.rLowerIso_UpperZoom.l4lut1.bits.ANR_LCE_X2;
	MINT32 ANR_LCE_X3_L_U		 = rParam.rLowerIso_UpperZoom.l4lut1.bits.ANR_LCE_X3;

	// CAM_ANR_L4LUT2
	MINT32 ANR_LCE_GAIN0_L_U	 = rParam.rLowerIso_UpperZoom.l4lut2.bits.ANR_LCE_GAIN0;
	MINT32 ANR_LCE_GAIN1_L_U	 = rParam.rLowerIso_UpperZoom.l4lut2.bits.ANR_LCE_GAIN1;
	MINT32 ANR_LCE_GAIN2_L_U	 = rParam.rLowerIso_UpperZoom.l4lut2.bits.ANR_LCE_GAIN2;
	MINT32 ANR_LCE_GAIN3_L_U	 = rParam.rLowerIso_UpperZoom.l4lut2.bits.ANR_LCE_GAIN3;

	// CAM_ANR_L4LUT3
	MINT32 ANR_LCE_SP0_L_U		 = rParam.rLowerIso_UpperZoom.l4lut3.bits.ANR_LCE_SP0;
	MINT32 ANR_LCE_SP1_L_U		 = rParam.rLowerIso_UpperZoom.l4lut3.bits.ANR_LCE_SP1;
	MINT32 ANR_LCE_SP2_L_U		 = rParam.rLowerIso_UpperZoom.l4lut3.bits.ANR_LCE_SP2;
	MINT32 ANR_LCE_SP3_L_U		 = rParam.rLowerIso_UpperZoom.l4lut3.bits.ANR_LCE_SP3;

	// CAM_ANR_PTY CAM+A40H
	MINT32 ANR_PTY1_L_U 		 = rParam.rLowerIso_UpperZoom.pty.bits.ANR_PTY1;
	MINT32 ANR_PTY2_L_U 		 = rParam.rLowerIso_UpperZoom.pty.bits.ANR_PTY2;
	MINT32 ANR_PTY3_L_U 		 = rParam.rLowerIso_UpperZoom.pty.bits.ANR_PTY3;
	MINT32 ANR_PTY4_L_U 		 = rParam.rLowerIso_UpperZoom.pty.bits.ANR_PTY4;

	// CAM_ANR_CAD CAM+A44H
	MINT32 ANR_PTC_VGAIN_L_U	 = rParam.rLowerIso_UpperZoom.cad.bits.ANR_PTC_VGAIN;
	MINT32 ANR_PTC_GAIN_TH_L_U	 = rParam.rLowerIso_UpperZoom.cad.bits.ANR_PTC_GAIN_TH;
	MINT32 ANR_C_L_DIFF_TH_L_U	 = rParam.rLowerIso_UpperZoom.cad.bits.ANR_C_L_DIFF_TH;

	// CAM_ANR_PTC CAM+A48H
	MINT32 ANR_PTC1_L_U 		 = rParam.rLowerIso_UpperZoom.ptc.bits.ANR_PTC1;
	MINT32 ANR_PTC2_L_U 		 = rParam.rLowerIso_UpperZoom.ptc.bits.ANR_PTC2;
	MINT32 ANR_PTC3_L_U 		 = rParam.rLowerIso_UpperZoom.ptc.bits.ANR_PTC3;
	MINT32 ANR_PTC4_L_U 		 = rParam.rLowerIso_UpperZoom.ptc.bits.ANR_PTC4;

	// CAM_ANR_LCE CAM+A4CH
	MINT32 ANR_LCE_C_GAIN_L_U	 = rParam.rLowerIso_UpperZoom.lce.bits.ANR_LCE_C_GAIN;
	MINT32 ANR_LCE_SCALE_GAIN_L_U= rParam.rLowerIso_UpperZoom.lce.bits.ANR_LCE_SCALE_GAIN;
	MINT32 ANR_LM_WT_L_U		 = rParam.rLowerIso_UpperZoom.lce.bits.ANR_LM_WT;


	// CAM_ANR_ACT1 CAM+A64H
	MINT32 ANR_ACT_LCE_GAIN_L_U  = rParam.rLowerIso_UpperZoom.act1.bits.ANR_ACT_LCE_GAIN;
	MINT32 ANR_ACT_DIF_GAIN_L_U  = rParam.rLowerIso_UpperZoom.act1.bits.ANR_ACT_DIF_GAIN;
	MINT32 ANR_ACT_DIF_LO_TH_L_U = rParam.rLowerIso_UpperZoom.act1.bits.ANR_ACT_DIF_LO_TH;

	// CAM_ANR_ACT2 CAM+A64H
	MINT32 ANR_ACT_SIZE_GAIN_L_U = rParam.rLowerIso_UpperZoom.act2.bits.ANR_ACT_SIZE_GAIN;
	MINT32 ANR_ACT_SIZE_LO_TH_L_U= rParam.rLowerIso_UpperZoom.act2.bits.ANR_ACT_SIZE_LO_TH;

	// CAM_ANR_ACTC CAM+A64H
	MINT32 ANR_ACT_BLD_BASE_C_L_U= rParam.rLowerIso_UpperZoom.actc.bits.ANR_ACT_BLD_BASE_C;
	MINT32 ANR_C_DITH_U_L_U= rParam.rLowerIso_UpperZoom.actc.bits.ANR_C_DITH_U;
	MINT32 ANR_C_DITH_V_L_U= rParam.rLowerIso_UpperZoom.actc.bits.ANR_C_DITH_V;

	// DIP_X_ANR_ACTYL CAM+A60H
	MINT32 ANR_ACT_TH_Y_L_L_U = rParam.rLowerIso_UpperZoom.actyl.bits.ANR_ACT_TH_Y_L;
	MINT32 ANR_ACT_BLD_BASE_Y_L_L_U = rParam.rLowerIso_UpperZoom.actyl.bits.ANR_ACT_BLD_BASE_Y_L;
	MINT32 ANR_ACT_SLANT_Y_L_L_U = rParam.rLowerIso_UpperZoom.actyl.bits.ANR_ACT_SLANT_Y_L;
	MINT32 ANR_ACT_BLD_TH_Y_L_L_U = rParam.rLowerIso_UpperZoom.actyl.bits.ANR_ACT_BLD_TH_Y_L;

	// DIP_X_ANR_YLAD CAM+A60H
	MINT32 ANR_CEN_GAIN_LO_TH_LPF_L_U = rParam.rLowerIso_UpperZoom.ylad.bits.ANR_CEN_GAIN_LO_TH_LPF;
	MINT32 ANR_CEN_GAIN_HI_TH_LPF_L_U = rParam.rLowerIso_UpperZoom.ylad.bits.ANR_CEN_GAIN_HI_TH_LPF;
	MINT32 ANR_K_LMT_L_U = rParam.rLowerIso_UpperZoom.ylad.bits.ANR_K_LMT;
	MINT32 ANR_K_LPF_L_U = rParam.rLowerIso_UpperZoom.ylad.bits.ANR_K_LPF;

	// DIP_X_ANR_PTYL CAM+A60H
	MINT32 ANR_PTY1_LPF_L_U = rParam.rLowerIso_UpperZoom.ptyl.bits.ANR_PTY1_LPF;
	MINT32 ANR_PTY2_LPF_L_U = rParam.rLowerIso_UpperZoom.ptyl.bits.ANR_PTY2_LPF;
	MINT32 ANR_PTY3_LPF_L_U = rParam.rLowerIso_UpperZoom.ptyl.bits.ANR_PTY3_LPF;
	MINT32 ANR_PTY4_LPF_L_U = rParam.rLowerIso_UpperZoom.ptyl.bits.ANR_PTY4_LPF;

	// DIP_X_ANR_LCOEF CAM+A60H
	MINT32 ANR_COEF_A_L_U = rParam.rLowerIso_UpperZoom.lcoef.bits.ANR_COEF_A;
	MINT32 ANR_COEF_B_L_U = rParam.rLowerIso_UpperZoom.lcoef.bits.ANR_COEF_B;
	MINT32 ANR_COEF_C_L_U = rParam.rLowerIso_UpperZoom.lcoef.bits.ANR_COEF_C;
	MINT32 ANR_COEF_D_L_U = rParam.rLowerIso_UpperZoom.lcoef.bits.ANR_COEF_D;

	// Olympu
	MINT32 ANR_C_SM_EDGE_TH_L_U 	  = rParam.rLowerIso_UpperZoom.con2.bits.ANR_C_SM_EDGE_TH;
	MINT32 ANR_K_HPF_L_U			  = rParam.rLowerIso_UpperZoom.ylad.bits.ANR_K_HPF;
	MINT32 ANR_ACT_DIF_HI_TH_L_U	  = rParam.rLowerIso_UpperZoom.act2.bits.ANR_ACT_DIF_HI_TH;
	MINT32 ANR_ACT_TH_Y_HL_L_U		  = rParam.rLowerIso_UpperZoom.actyhl.bits.ANR_ACT_TH_Y_HL;
	MINT32 ANR_ACT_BLD_BASE_Y_HL_L_U  = rParam.rLowerIso_UpperZoom.actyhl.bits.ANR_ACT_BLD_BASE_Y_HL;
	MINT32 ANR_ACT_SLANT_Y_HL_L_U	  = rParam.rLowerIso_UpperZoom.actyhl.bits.ANR_ACT_SLANT_Y_HL;
	MINT32 ANR_ACT_BLD_TH_Y_HL_L_U	  = rParam.rLowerIso_UpperZoom.actyhl.bits.ANR_ACT_BLD_TH_Y_HL;
	MINT32 ANR_ACT_TH_Y_HH_L_U		  = rParam.rLowerIso_UpperZoom.actyhh.bits.ANR_ACT_TH_Y_HH;
	MINT32 ANR_ACT_BLD_BASE_Y_HH_L_U  = rParam.rLowerIso_UpperZoom.actyhh.bits.ANR_ACT_BLD_BASE_Y_HH;
	MINT32 ANR_ACT_SLANT_Y_HH_L_U	  = rParam.rLowerIso_UpperZoom.actyhh.bits.ANR_ACT_SLANT_Y_HH;
	MINT32 ANR_ACT_BLD_TH_Y_HH_L_U	  = rParam.rLowerIso_UpperZoom.actyhh.bits.ANR_ACT_BLD_TH_Y_HH;
	MINT32 ANR_LUMA_TH_Y_L_L_U		  = rParam.rLowerIso_UpperZoom.actyl2.bits.ANR_LUMA_TH_Y_L;
	MINT32 ANR_LUMA_BLD_BASE_Y_L_L_U  = rParam.rLowerIso_UpperZoom.actyl2.bits.ANR_LUMA_BLD_BASE_Y_L;
	MINT32 ANR_LUMA_SLANT_Y_L_L_U	  = rParam.rLowerIso_UpperZoom.actyl2.bits.ANR_LUMA_SLANT_Y_L;
	MINT32 ANR_LUMA_BLD_TH_Y_L_L_U	  = rParam.rLowerIso_UpperZoom.actyl2.bits.ANR_LUMA_BLD_TH_Y_L;
	MINT32 ANR_LUMA_TH_Y_HL_L_U 	  = rParam.rLowerIso_UpperZoom.actyhl2.bits.ANR_LUMA_TH_Y_HL;
	MINT32 ANR_LUMA_BLD_BASE_Y_HL_L_U = rParam.rLowerIso_UpperZoom.actyhl2.bits.ANR_LUMA_BLD_BASE_Y_HL;
	MINT32 ANR_LUMA_SLANT_Y_HL_L_U	  = rParam.rLowerIso_UpperZoom.actyhl2.bits.ANR_LUMA_SLANT_Y_HL;
	MINT32 ANR_LUMA_BLD_TH_Y_HL_L_U   = rParam.rLowerIso_UpperZoom.actyhl2.bits.ANR_LUMA_BLD_TH_Y_HL;
	MINT32 ANR_LUMA_TH_Y_HH_L_U 	  = rParam.rLowerIso_UpperZoom.actyhh2.bits.ANR_LUMA_TH_Y_HH;
	MINT32 ANR_LUMA_BLD_BASE_Y_HH_L_U = rParam.rLowerIso_UpperZoom.actyhh2.bits.ANR_LUMA_BLD_BASE_Y_HH;
	MINT32 ANR_LUMA_SLANT_Y_HH_L_U	  = rParam.rLowerIso_UpperZoom.actyhh2.bits.ANR_LUMA_SLANT_Y_HH;
	MINT32 ANR_LUMA_BLD_TH_Y_HH_L_U   = rParam.rLowerIso_UpperZoom.actyhh2.bits.ANR_LUMA_BLD_TH_Y_HH;
	MINT32 ANR_PTY_L_GAIN_L_U		  = rParam.rLowerIso_UpperZoom.ylad2.bits.ANR_PTY_L_GAIN;
	MINT32 ANR_PTY_H_GAIN_L_U		  = rParam.rLowerIso_UpperZoom.ylad2.bits.ANR_PTY_H_GAIN;
	MINT32 ANR_BLEND_TH_L_L_U		  = rParam.rLowerIso_UpperZoom.ylad2.bits.ANR_BLEND_TH_L;
	MINT32 ANR_BLEND_TH_H_L_U		  = rParam.rLowerIso_UpperZoom.ylad2.bits.ANR_BLEND_TH_H;
	MINT32 ANR_HI_LMT1_L_U			  = rParam.rLowerIso_UpperZoom.ylad3.bits.ANR_HI_LMT1;
	MINT32 ANR_HI_LMT2_L_U			  = rParam.rLowerIso_UpperZoom.ylad3.bits.ANR_HI_LMT2;
	MINT32 ANR_LO_LMT1_L_U			  = rParam.rLowerIso_UpperZoom.ylad3.bits.ANR_LO_LMT1;
	MINT32 ANR_LO_LMT2_L_U			  = rParam.rLowerIso_UpperZoom.ylad3.bits.ANR_LO_LMT2;
	MINT32 ANR_Y_ACT_CEN_OFT_L_U	  = rParam.rLowerIso_UpperZoom.act4.bits.ANR_Y_ACT_CEN_OFT;
	MINT32 ANR_Y_ACT_CEN_GAIN_L_U	  = rParam.rLowerIso_UpperZoom.act4.bits.ANR_Y_ACT_CEN_GAIN;
	MINT32 ANR_Y_ACT_CEN_TH_L_U 	  = rParam.rLowerIso_UpperZoom.act4.bits.ANR_Y_ACT_CEN_TH;
	MINT32 ANR_Y_DIR_L_U			  = rParam.rLowerIso_UpperZoom.con1.bits.ANR_Y_DIR;
	MINT32 ANR_Y_DIR_GAIN_L_U		  = rParam.rLowerIso_UpperZoom.ydir.bits.ANR_Y_DIR_GAIN;
	MINT32 ANR_Y_DIR_VER_W_TH_L_U	  = rParam.rLowerIso_UpperZoom.ydir.bits.ANR_Y_DIR_VER_W_TH;
	MINT32 ANR_Y_DIR_VER_W_SL_L_U	  = rParam.rLowerIso_UpperZoom.ydir.bits.ANR_Y_DIR_VER_W_SL;
	MINT32 ANR_Y_DIR_DIAG_W_TH_L_U	  = rParam.rLowerIso_UpperZoom.ydir.bits.ANR_Y_DIR_DIAG_W_TH;
	MINT32 ANR_Y_DIR_DIAG_W_SL_L_U	  = rParam.rLowerIso_UpperZoom.ydir.bits.ANR_Y_DIR_DIAG_W_SL;

	// Whitney E2

	MINT32 ANR_TBL_CPX1_L_U		 = rParam.rLowerIso_UpperZoom.t4lut1.bits.ANR_TBL_CPX1;
	MINT32 ANR_TBL_CPX2_L_U		 = rParam.rLowerIso_UpperZoom.t4lut1.bits.ANR_TBL_CPX2;
	MINT32 ANR_TBL_CPX3_L_U		 = rParam.rLowerIso_UpperZoom.t4lut1.bits.ANR_TBL_CPX3;
	MINT32 ANR_TBL_GAIN_CPY0_L_U		 = rParam.rLowerIso_UpperZoom.t4lut2.bits.ANR_TBL_GAIN_CPY0;
	MINT32 ANR_TBL_GAIN_CPY1_L_U		 = rParam.rLowerIso_UpperZoom.t4lut2.bits.ANR_TBL_GAIN_CPY1;
	MINT32 ANR_TBL_GAIN_CPY2_L_U		 = rParam.rLowerIso_UpperZoom.t4lut2.bits.ANR_TBL_GAIN_CPY2;
	MINT32 ANR_TBL_GAIN_CPY3_L_U		 = rParam.rLowerIso_UpperZoom.t4lut2.bits.ANR_TBL_GAIN_CPY3;
	MINT32 ANR_TBL_GAIN_SP0_L_U		 = rParam.rLowerIso_UpperZoom.t4lut3.bits.ANR_TBL_GAIN_SP0;
	MINT32 ANR_TBL_GAIN_SP1_L_U		 = rParam.rLowerIso_UpperZoom.t4lut3.bits.ANR_TBL_GAIN_SP1;
	MINT32 ANR_TBL_GAIN_SP2_L_U		 = rParam.rLowerIso_UpperZoom.t4lut3.bits.ANR_TBL_GAIN_SP2;
	MINT32 ANR_TBL_GAIN_SP3_L_U		 = rParam.rLowerIso_UpperZoom.t4lut3.bits.ANR_TBL_GAIN_SP3;


	//	  ====================================================================================================================================
	//	  Get Up ISO Lo Zoom
	//	  ====================================================================================================================================
	// CAM_ANR_CON1 CAM+A20H
	MINT32 ANR_ENC_U_L			 = rParam.rUpperIso_LowerZoom.con1.bits.ANR_ENC;
	MINT32 ANR_ENY_U_L			 = rParam.rUpperIso_LowerZoom.con1.bits.ANR_ENY;
	MINT32 ANR_SCALE_MODE_U_L	 = rParam.rUpperIso_LowerZoom.con1.bits.ANR_SCALE_MODE;
	MINT32 ANR_LCE_LINK_U_L 	 = rParam.rUpperIso_LowerZoom.con1.bits.ANR_LCE_LINK;
	MINT32 ANR_TABLE_EN_U_L 	 = rParam.rUpperIso_LowerZoom.con1.bits.ANR_TABLE_EN;
	MINT32 ANR_TBL_PRC_U_L		 = rParam.rUpperIso_LowerZoom.con1.bits.ANR_TBL_PRC;

	// CAM_ANR_CON2 CAM+A24H
	MINT32 ANR_IMPL_MODE_U_L	 = rParam.rUpperIso_LowerZoom.con2.bits.ANR_IMPL_MODE;
	MINT32 ANR_C_SM_EDGE_U_L	 = rParam.rUpperIso_LowerZoom.con2.bits.ANR_C_SM_EDGE;
	MINT32 ANR_FLT_C_U_L		 = rParam.rUpperIso_LowerZoom.con2.bits.ANR_FLT_C;
	MINT32 ANR_LR_U_L			 = rParam.rUpperIso_LowerZoom.con2.bits.ANR_LR;
	MINT32 ANR_ALPHA_U_L		 = rParam.rUpperIso_LowerZoom.con2.bits.ANR_ALPHA;

	// CAM_ANR_YAD1 CAM+A2CH
	MINT32 ANR_CEN_GAIN_LO_TH_U_L= rParam.rUpperIso_LowerZoom.yad1.bits.ANR_CEN_GAIN_LO_TH;
	MINT32 ANR_CEN_GAIN_HI_TH_U_L= rParam.rUpperIso_LowerZoom.yad1.bits.ANR_CEN_GAIN_HI_TH;
	MINT32 ANR_K_LO_TH_U_L		 = rParam.rUpperIso_LowerZoom.yad1.bits.ANR_K_LO_TH;
	MINT32 ANR_K_HI_TH_U_L		 = rParam.rUpperIso_LowerZoom.yad1.bits.ANR_K_HI_TH;
	MINT32 ANR_K_TH_C_U_L		 = rParam.rUpperIso_LowerZoom.yad1.bits.ANR_K_TH_C;

	// CAM_ANR_YAD2 CAM+A30H
	MINT32 ANR_PTY_VGAIN_U_L	 = rParam.rUpperIso_LowerZoom.yad2.bits.ANR_PTY_VGAIN;
	MINT32 ANR_PTY_GAIN_TH_U_L	 = rParam.rUpperIso_LowerZoom.yad2.bits.ANR_PTY_GAIN_TH;
	MINT32 ANR_K_RAT_U_L	     = rParam.rUpperIso_LowerZoom.yad2.bits.ANR_K_RAT;

	// CAM_ANR_Y4LUT1 CAM+A34H
	MINT32 ANR_Y_CPX1_U_L		 = rParam.rUpperIso_LowerZoom.y4lut1.bits.ANR_Y_CPX1;
	MINT32 ANR_Y_CPX2_U_L		 = rParam.rUpperIso_LowerZoom.y4lut1.bits.ANR_Y_CPX2;
	MINT32 ANR_Y_CPX3_U_L		 = rParam.rUpperIso_LowerZoom.y4lut1.bits.ANR_Y_CPX3;

	// CAM_ANR_Y4LUT2 CAM+A38H
	MINT32 ANR_Y_SCALE_CPY0_U_L  = rParam.rUpperIso_LowerZoom.y4lut2.bits.ANR_Y_SCALE_CPY0;
	MINT32 ANR_Y_SCALE_CPY1_U_L  = rParam.rUpperIso_LowerZoom.y4lut2.bits.ANR_Y_SCALE_CPY1;
	MINT32 ANR_Y_SCALE_CPY2_U_L  = rParam.rUpperIso_LowerZoom.y4lut2.bits.ANR_Y_SCALE_CPY2;
	MINT32 ANR_Y_SCALE_CPY3_U_L  = rParam.rUpperIso_LowerZoom.y4lut2.bits.ANR_Y_SCALE_CPY3;

	// CAM_ANR_Y4LUT3 CAM+A3CH
	MINT32 ANR_Y_SCALE_SP0_U_L	 = rParam.rUpperIso_LowerZoom.y4lut3.bits.ANR_Y_SCALE_SP0;
	MINT32 ANR_Y_SCALE_SP1_U_L	 = rParam.rUpperIso_LowerZoom.y4lut3.bits.ANR_Y_SCALE_SP1;
	MINT32 ANR_Y_SCALE_SP2_U_L	 = rParam.rUpperIso_LowerZoom.y4lut3.bits.ANR_Y_SCALE_SP2;
	MINT32 ANR_Y_SCALE_SP3_U_L	 = rParam.rUpperIso_LowerZoom.y4lut3.bits.ANR_Y_SCALE_SP3;

	// CAM_ANR_C4LUT1
	MINT32 ANR_C_CPX1_U_L		 = rParam.rUpperIso_LowerZoom.c4lut1.bits.ANR_C_CPX1;
	MINT32 ANR_C_CPX2_U_L		 = rParam.rUpperIso_LowerZoom.c4lut1.bits.ANR_C_CPX2;
	MINT32 ANR_C_CPX3_U_L		 = rParam.rUpperIso_LowerZoom.c4lut1.bits.ANR_C_CPX3;

	// CAM_ANR_C4LUT2
	MINT32 ANR_C_SCALE_CPY0_U_L  = rParam.rUpperIso_LowerZoom.c4lut2.bits.ANR_C_SCALE_CPY0;
	MINT32 ANR_C_SCALE_CPY1_U_L  = rParam.rUpperIso_LowerZoom.c4lut2.bits.ANR_C_SCALE_CPY1;
	MINT32 ANR_C_SCALE_CPY2_U_L  = rParam.rUpperIso_LowerZoom.c4lut2.bits.ANR_C_SCALE_CPY2;
	MINT32 ANR_C_SCALE_CPY3_U_L  = rParam.rUpperIso_LowerZoom.c4lut2.bits.ANR_C_SCALE_CPY3;

	// CAM_ANR_C4LUT3
	MINT32 ANR_C_SCALE_SP0_U_L	 = rParam.rUpperIso_LowerZoom.c4lut3.bits.ANR_C_SCALE_SP0;
	MINT32 ANR_C_SCALE_SP1_U_L	 = rParam.rUpperIso_LowerZoom.c4lut3.bits.ANR_C_SCALE_SP1;
	MINT32 ANR_C_SCALE_SP2_U_L	 = rParam.rUpperIso_LowerZoom.c4lut3.bits.ANR_C_SCALE_SP2;
	MINT32 ANR_C_SCALE_SP3_U_L	 = rParam.rUpperIso_LowerZoom.c4lut3.bits.ANR_C_SCALE_SP3;

	// CAM_ANR_A4LUT2
	MINT32 ANR_Y_ACT_CPY0_U_L	 = rParam.rUpperIso_LowerZoom.a4lut2.bits.ANR_Y_ACT_CPY0;
	MINT32 ANR_Y_ACT_CPY1_U_L	 = rParam.rUpperIso_LowerZoom.a4lut2.bits.ANR_Y_ACT_CPY1;
	MINT32 ANR_Y_ACT_CPY2_U_L	 = rParam.rUpperIso_LowerZoom.a4lut2.bits.ANR_Y_ACT_CPY2;
	MINT32 ANR_Y_ACT_CPY3_U_L	 = rParam.rUpperIso_LowerZoom.a4lut2.bits.ANR_Y_ACT_CPY3;

	// CAM_ANR_A4LUT3
	MINT32 ANR_Y_ACT_SP0_U_L	 = rParam.rUpperIso_LowerZoom.a4lut3.bits.ANR_Y_ACT_SP0;
	MINT32 ANR_Y_ACT_SP1_U_L	 = rParam.rUpperIso_LowerZoom.a4lut3.bits.ANR_Y_ACT_SP1;
	MINT32 ANR_Y_ACT_SP2_U_L	 = rParam.rUpperIso_LowerZoom.a4lut3.bits.ANR_Y_ACT_SP2;
	MINT32 ANR_Y_ACT_SP3_U_L	 = rParam.rUpperIso_LowerZoom.a4lut3.bits.ANR_Y_ACT_SP3;

	// CAM_ANR_L4LUT1
	MINT32 ANR_LCE_X1_U_L		 = rParam.rUpperIso_LowerZoom.l4lut1.bits.ANR_LCE_X1;
	MINT32 ANR_LCE_X2_U_L		 = rParam.rUpperIso_LowerZoom.l4lut1.bits.ANR_LCE_X2;
	MINT32 ANR_LCE_X3_U_L		 = rParam.rUpperIso_LowerZoom.l4lut1.bits.ANR_LCE_X3;

	// CAM_ANR_L4LUT2
	MINT32 ANR_LCE_GAIN0_U_L	 = rParam.rUpperIso_LowerZoom.l4lut2.bits.ANR_LCE_GAIN0;
	MINT32 ANR_LCE_GAIN1_U_L	 = rParam.rUpperIso_LowerZoom.l4lut2.bits.ANR_LCE_GAIN1;
	MINT32 ANR_LCE_GAIN2_U_L	 = rParam.rUpperIso_LowerZoom.l4lut2.bits.ANR_LCE_GAIN2;
	MINT32 ANR_LCE_GAIN3_U_L	 = rParam.rUpperIso_LowerZoom.l4lut2.bits.ANR_LCE_GAIN3;

	// CAM_ANR_L4LUT3
	MINT32 ANR_LCE_SP0_U_L		 = rParam.rUpperIso_LowerZoom.l4lut3.bits.ANR_LCE_SP0;
	MINT32 ANR_LCE_SP1_U_L		 = rParam.rUpperIso_LowerZoom.l4lut3.bits.ANR_LCE_SP1;
	MINT32 ANR_LCE_SP2_U_L		 = rParam.rUpperIso_LowerZoom.l4lut3.bits.ANR_LCE_SP2;
	MINT32 ANR_LCE_SP3_U_L		 = rParam.rUpperIso_LowerZoom.l4lut3.bits.ANR_LCE_SP3;

	// CAM_ANR_PTY CAM+A40H
	MINT32 ANR_PTY1_U_L 		 = rParam.rUpperIso_LowerZoom.pty.bits.ANR_PTY1;
	MINT32 ANR_PTY2_U_L 		 = rParam.rUpperIso_LowerZoom.pty.bits.ANR_PTY2;
	MINT32 ANR_PTY3_U_L 		 = rParam.rUpperIso_LowerZoom.pty.bits.ANR_PTY3;
	MINT32 ANR_PTY4_U_L 		 = rParam.rUpperIso_LowerZoom.pty.bits.ANR_PTY4;

	// CAM_ANR_CAD CAM+A44H
	MINT32 ANR_PTC_VGAIN_U_L	 = rParam.rUpperIso_LowerZoom.cad.bits.ANR_PTC_VGAIN;
	MINT32 ANR_PTC_GAIN_TH_U_L	 = rParam.rUpperIso_LowerZoom.cad.bits.ANR_PTC_GAIN_TH;
	MINT32 ANR_C_L_DIFF_TH_U_L	 = rParam.rUpperIso_LowerZoom.cad.bits.ANR_C_L_DIFF_TH;

	// CAM_ANR_PTC CAM+A48H
	MINT32 ANR_PTC1_U_L 		 = rParam.rUpperIso_LowerZoom.ptc.bits.ANR_PTC1;
	MINT32 ANR_PTC2_U_L 		 = rParam.rUpperIso_LowerZoom.ptc.bits.ANR_PTC2;
	MINT32 ANR_PTC3_U_L 		 = rParam.rUpperIso_LowerZoom.ptc.bits.ANR_PTC3;
	MINT32 ANR_PTC4_U_L 		 = rParam.rUpperIso_LowerZoom.ptc.bits.ANR_PTC4;

	// CAM_ANR_LCE CAM+A4CH
	MINT32 ANR_LCE_C_GAIN_U_L	 = rParam.rUpperIso_LowerZoom.lce.bits.ANR_LCE_C_GAIN;
	MINT32 ANR_LCE_SCALE_GAIN_U_L= rParam.rUpperIso_LowerZoom.lce.bits.ANR_LCE_SCALE_GAIN;
	MINT32 ANR_LM_WT_U_L		 = rParam.rUpperIso_LowerZoom.lce.bits.ANR_LM_WT;



	// CAM_ANR_ACT1 CAM+A64H
	MINT32 ANR_ACT_LCE_GAIN_U_L  = rParam.rUpperIso_LowerZoom.act1.bits.ANR_ACT_LCE_GAIN;
	MINT32 ANR_ACT_DIF_GAIN_U_L  = rParam.rUpperIso_LowerZoom.act1.bits.ANR_ACT_DIF_GAIN;
	MINT32 ANR_ACT_DIF_LO_TH_U_L = rParam.rUpperIso_LowerZoom.act1.bits.ANR_ACT_DIF_LO_TH;

	// CAM_ANR_ACT2 CAM+A64H
	MINT32 ANR_ACT_SIZE_GAIN_U_L = rParam.rUpperIso_LowerZoom.act2.bits.ANR_ACT_SIZE_GAIN;
	MINT32 ANR_ACT_SIZE_LO_TH_U_L= rParam.rUpperIso_LowerZoom.act2.bits.ANR_ACT_SIZE_LO_TH;

	// CAM_ANR_ACTC CAM+A64H
	MINT32 ANR_ACT_BLD_BASE_C_U_L= rParam.rUpperIso_LowerZoom.actc.bits.ANR_ACT_BLD_BASE_C;
	MINT32 ANR_C_DITH_U_U_L= rParam.rUpperIso_LowerZoom.actc.bits.ANR_C_DITH_U;
	MINT32 ANR_C_DITH_V_U_L= rParam.rUpperIso_LowerZoom.actc.bits.ANR_C_DITH_V;

	// DIP_X_ANR_ACTYL CAM+A60H
	MINT32 ANR_ACT_TH_Y_L_U_L = rParam.rUpperIso_LowerZoom.actyl.bits.ANR_ACT_TH_Y_L;
	MINT32 ANR_ACT_BLD_BASE_Y_L_U_L = rParam.rUpperIso_LowerZoom.actyl.bits.ANR_ACT_BLD_BASE_Y_L;
	MINT32 ANR_ACT_SLANT_Y_L_U_L = rParam.rUpperIso_LowerZoom.actyl.bits.ANR_ACT_SLANT_Y_L;
	MINT32 ANR_ACT_BLD_TH_Y_L_U_L = rParam.rUpperIso_LowerZoom.actyl.bits.ANR_ACT_BLD_TH_Y_L;

	// DIP_X_ANR_YLAD CAM+A60H
	MINT32 ANR_CEN_GAIN_LO_TH_LPF_U_L = rParam.rUpperIso_LowerZoom.ylad.bits.ANR_CEN_GAIN_LO_TH_LPF;
	MINT32 ANR_CEN_GAIN_HI_TH_LPF_U_L = rParam.rUpperIso_LowerZoom.ylad.bits.ANR_CEN_GAIN_HI_TH_LPF;
	MINT32 ANR_K_LMT_U_L = rParam.rUpperIso_LowerZoom.ylad.bits.ANR_K_LMT;
	MINT32 ANR_K_LPF_U_L = rParam.rUpperIso_LowerZoom.ylad.bits.ANR_K_LPF;

	// DIP_X_ANR_PTYL CAM+A60H
	MINT32 ANR_PTY1_LPF_U_L = rParam.rUpperIso_LowerZoom.ptyl.bits.ANR_PTY1_LPF;
	MINT32 ANR_PTY2_LPF_U_L = rParam.rUpperIso_LowerZoom.ptyl.bits.ANR_PTY2_LPF;
	MINT32 ANR_PTY3_LPF_U_L = rParam.rUpperIso_LowerZoom.ptyl.bits.ANR_PTY3_LPF;
	MINT32 ANR_PTY4_LPF_U_L = rParam.rUpperIso_LowerZoom.ptyl.bits.ANR_PTY4_LPF;

	// DIP_X_ANR_LCOEF CAM+A60H
	MINT32 ANR_COEF_A_U_L = rParam.rUpperIso_LowerZoom.lcoef.bits.ANR_COEF_A;
	MINT32 ANR_COEF_B_U_L = rParam.rUpperIso_LowerZoom.lcoef.bits.ANR_COEF_B;
	MINT32 ANR_COEF_C_U_L = rParam.rUpperIso_LowerZoom.lcoef.bits.ANR_COEF_C;
	MINT32 ANR_COEF_D_U_L = rParam.rUpperIso_LowerZoom.lcoef.bits.ANR_COEF_D;

	// Olympu
	MINT32 ANR_C_SM_EDGE_TH_U_L 	  = rParam.rUpperIso_LowerZoom.con2.bits.ANR_C_SM_EDGE_TH;
	MINT32 ANR_K_HPF_U_L			  = rParam.rUpperIso_LowerZoom.ylad.bits.ANR_K_HPF;
	MINT32 ANR_ACT_DIF_HI_TH_U_L	  = rParam.rUpperIso_LowerZoom.act2.bits.ANR_ACT_DIF_HI_TH;
	MINT32 ANR_ACT_TH_Y_HL_U_L		  = rParam.rUpperIso_LowerZoom.actyhl.bits.ANR_ACT_TH_Y_HL;
	MINT32 ANR_ACT_BLD_BASE_Y_HL_U_L  = rParam.rUpperIso_LowerZoom.actyhl.bits.ANR_ACT_BLD_BASE_Y_HL;
	MINT32 ANR_ACT_SLANT_Y_HL_U_L	  = rParam.rUpperIso_LowerZoom.actyhl.bits.ANR_ACT_SLANT_Y_HL;
	MINT32 ANR_ACT_BLD_TH_Y_HL_U_L	  = rParam.rUpperIso_LowerZoom.actyhl.bits.ANR_ACT_BLD_TH_Y_HL;
	MINT32 ANR_ACT_TH_Y_HH_U_L		  = rParam.rUpperIso_LowerZoom.actyhh.bits.ANR_ACT_TH_Y_HH;
	MINT32 ANR_ACT_BLD_BASE_Y_HH_U_L  = rParam.rUpperIso_LowerZoom.actyhh.bits.ANR_ACT_BLD_BASE_Y_HH;
	MINT32 ANR_ACT_SLANT_Y_HH_U_L	  = rParam.rUpperIso_LowerZoom.actyhh.bits.ANR_ACT_SLANT_Y_HH;
	MINT32 ANR_ACT_BLD_TH_Y_HH_U_L	  = rParam.rUpperIso_LowerZoom.actyhh.bits.ANR_ACT_BLD_TH_Y_HH;
	MINT32 ANR_LUMA_TH_Y_L_U_L		  = rParam.rUpperIso_LowerZoom.actyl2.bits.ANR_LUMA_TH_Y_L;
	MINT32 ANR_LUMA_BLD_BASE_Y_L_U_L  = rParam.rUpperIso_LowerZoom.actyl2.bits.ANR_LUMA_BLD_BASE_Y_L;
	MINT32 ANR_LUMA_SLANT_Y_L_U_L	  = rParam.rUpperIso_LowerZoom.actyl2.bits.ANR_LUMA_SLANT_Y_L;
	MINT32 ANR_LUMA_BLD_TH_Y_L_U_L	  = rParam.rUpperIso_LowerZoom.actyl2.bits.ANR_LUMA_BLD_TH_Y_L;
	MINT32 ANR_LUMA_TH_Y_HL_U_L 	  = rParam.rUpperIso_LowerZoom.actyhl2.bits.ANR_LUMA_TH_Y_HL;
	MINT32 ANR_LUMA_BLD_BASE_Y_HL_U_L = rParam.rUpperIso_LowerZoom.actyhl2.bits.ANR_LUMA_BLD_BASE_Y_HL;
	MINT32 ANR_LUMA_SLANT_Y_HL_U_L	  = rParam.rUpperIso_LowerZoom.actyhl2.bits.ANR_LUMA_SLANT_Y_HL;
	MINT32 ANR_LUMA_BLD_TH_Y_HL_U_L   = rParam.rUpperIso_LowerZoom.actyhl2.bits.ANR_LUMA_BLD_TH_Y_HL;
	MINT32 ANR_LUMA_TH_Y_HH_U_L 	  = rParam.rUpperIso_LowerZoom.actyhh2.bits.ANR_LUMA_TH_Y_HH;
	MINT32 ANR_LUMA_BLD_BASE_Y_HH_U_L = rParam.rUpperIso_LowerZoom.actyhh2.bits.ANR_LUMA_BLD_BASE_Y_HH;
	MINT32 ANR_LUMA_SLANT_Y_HH_U_L	  = rParam.rUpperIso_LowerZoom.actyhh2.bits.ANR_LUMA_SLANT_Y_HH;
	MINT32 ANR_LUMA_BLD_TH_Y_HH_U_L   = rParam.rUpperIso_LowerZoom.actyhh2.bits.ANR_LUMA_BLD_TH_Y_HH;
	MINT32 ANR_PTY_L_GAIN_U_L		  = rParam.rUpperIso_LowerZoom.ylad2.bits.ANR_PTY_L_GAIN;
	MINT32 ANR_PTY_H_GAIN_U_L		  = rParam.rUpperIso_LowerZoom.ylad2.bits.ANR_PTY_H_GAIN;
	MINT32 ANR_BLEND_TH_L_U_L		  = rParam.rUpperIso_LowerZoom.ylad2.bits.ANR_BLEND_TH_L;
	MINT32 ANR_BLEND_TH_H_U_L		  = rParam.rUpperIso_LowerZoom.ylad2.bits.ANR_BLEND_TH_H;
	MINT32 ANR_HI_LMT1_U_L			  = rParam.rUpperIso_LowerZoom.ylad3.bits.ANR_HI_LMT1;
	MINT32 ANR_HI_LMT2_U_L			  = rParam.rUpperIso_LowerZoom.ylad3.bits.ANR_HI_LMT2;
	MINT32 ANR_LO_LMT1_U_L			  = rParam.rUpperIso_LowerZoom.ylad3.bits.ANR_LO_LMT1;
	MINT32 ANR_LO_LMT2_U_L			  = rParam.rUpperIso_LowerZoom.ylad3.bits.ANR_LO_LMT2;
	MINT32 ANR_Y_ACT_CEN_OFT_U_L	  = rParam.rUpperIso_LowerZoom.act4.bits.ANR_Y_ACT_CEN_OFT;
	MINT32 ANR_Y_ACT_CEN_GAIN_U_L	  = rParam.rUpperIso_LowerZoom.act4.bits.ANR_Y_ACT_CEN_GAIN;
	MINT32 ANR_Y_ACT_CEN_TH_U_L 	  = rParam.rUpperIso_LowerZoom.act4.bits.ANR_Y_ACT_CEN_TH;
	MINT32 ANR_Y_DIR_U_L			  = rParam.rUpperIso_LowerZoom.con1.bits.ANR_Y_DIR;
	MINT32 ANR_Y_DIR_GAIN_U_L		  = rParam.rUpperIso_LowerZoom.ydir.bits.ANR_Y_DIR_GAIN;
	MINT32 ANR_Y_DIR_VER_W_TH_U_L	  = rParam.rUpperIso_LowerZoom.ydir.bits.ANR_Y_DIR_VER_W_TH;
	MINT32 ANR_Y_DIR_VER_W_SL_U_L	  = rParam.rUpperIso_LowerZoom.ydir.bits.ANR_Y_DIR_VER_W_SL;
	MINT32 ANR_Y_DIR_DIAG_W_TH_U_L	  = rParam.rUpperIso_LowerZoom.ydir.bits.ANR_Y_DIR_DIAG_W_TH;
	MINT32 ANR_Y_DIR_DIAG_W_SL_U_L	  = rParam.rUpperIso_LowerZoom.ydir.bits.ANR_Y_DIR_DIAG_W_SL;

	// Whitney E2

	MINT32 ANR_TBL_CPX1_U_L		 = rParam.rUpperIso_LowerZoom.t4lut1.bits.ANR_TBL_CPX1;
	MINT32 ANR_TBL_CPX2_U_L		 = rParam.rUpperIso_LowerZoom.t4lut1.bits.ANR_TBL_CPX2;
	MINT32 ANR_TBL_CPX3_U_L		 = rParam.rUpperIso_LowerZoom.t4lut1.bits.ANR_TBL_CPX3;
	MINT32 ANR_TBL_GAIN_CPY0_U_L		 = rParam.rUpperIso_LowerZoom.t4lut2.bits.ANR_TBL_GAIN_CPY0;
	MINT32 ANR_TBL_GAIN_CPY1_U_L		 = rParam.rUpperIso_LowerZoom.t4lut2.bits.ANR_TBL_GAIN_CPY1;
	MINT32 ANR_TBL_GAIN_CPY2_U_L		 = rParam.rUpperIso_LowerZoom.t4lut2.bits.ANR_TBL_GAIN_CPY2;
	MINT32 ANR_TBL_GAIN_CPY3_U_L		 = rParam.rUpperIso_LowerZoom.t4lut2.bits.ANR_TBL_GAIN_CPY3;
	MINT32 ANR_TBL_GAIN_SP0_U_L		 = rParam.rUpperIso_LowerZoom.t4lut3.bits.ANR_TBL_GAIN_SP0;
	MINT32 ANR_TBL_GAIN_SP1_U_L		 = rParam.rUpperIso_LowerZoom.t4lut3.bits.ANR_TBL_GAIN_SP1;
	MINT32 ANR_TBL_GAIN_SP2_U_L		 = rParam.rUpperIso_LowerZoom.t4lut3.bits.ANR_TBL_GAIN_SP2;
	MINT32 ANR_TBL_GAIN_SP3_U_L		 = rParam.rUpperIso_LowerZoom.t4lut3.bits.ANR_TBL_GAIN_SP3;

	//	  ====================================================================================================================================
	//	  Get Up ISO Up Zoom
	//	  ====================================================================================================================================
	// CAM_ANR_CON1 CAM+A20H
	MINT32 ANR_ENC_U_U			 = rParam.rUpperIso_UpperZoom.con1.bits.ANR_ENC;
	MINT32 ANR_ENY_U_U			 = rParam.rUpperIso_UpperZoom.con1.bits.ANR_ENY;
	MINT32 ANR_SCALE_MODE_U_U	 = rParam.rUpperIso_UpperZoom.con1.bits.ANR_SCALE_MODE;
	MINT32 ANR_LCE_LINK_U_U 	 = rParam.rUpperIso_UpperZoom.con1.bits.ANR_LCE_LINK;
	MINT32 ANR_TABLE_EN_U_U 	 = rParam.rUpperIso_UpperZoom.con1.bits.ANR_TABLE_EN;
	MINT32 ANR_TBL_PRC_U_U		 = rParam.rUpperIso_UpperZoom.con1.bits.ANR_TBL_PRC;

	// CAM_ANR_CON2 CAM+A24H
	MINT32 ANR_IMPL_MODE_U_U	 = rParam.rUpperIso_UpperZoom.con2.bits.ANR_IMPL_MODE;
	MINT32 ANR_C_SM_EDGE_U_U	 = rParam.rUpperIso_UpperZoom.con2.bits.ANR_C_SM_EDGE;
	MINT32 ANR_FLT_C_U_U		 = rParam.rUpperIso_UpperZoom.con2.bits.ANR_FLT_C;
	MINT32 ANR_LR_U_U			 = rParam.rUpperIso_UpperZoom.con2.bits.ANR_LR;
	MINT32 ANR_ALPHA_U_U		 = rParam.rUpperIso_UpperZoom.con2.bits.ANR_ALPHA;

	// CAM_ANR_YAD1 CAM+A2CH
	MINT32 ANR_CEN_GAIN_LO_TH_U_U= rParam.rUpperIso_UpperZoom.yad1.bits.ANR_CEN_GAIN_LO_TH;
	MINT32 ANR_CEN_GAIN_HI_TH_U_U= rParam.rUpperIso_UpperZoom.yad1.bits.ANR_CEN_GAIN_HI_TH;
	MINT32 ANR_K_LO_TH_U_U		 = rParam.rUpperIso_UpperZoom.yad1.bits.ANR_K_LO_TH;
	MINT32 ANR_K_HI_TH_U_U		 = rParam.rUpperIso_UpperZoom.yad1.bits.ANR_K_HI_TH;
	MINT32 ANR_K_TH_C_U_U		 = rParam.rUpperIso_UpperZoom.yad1.bits.ANR_K_TH_C;

	// CAM_ANR_YAD2 CAM+A30H
	MINT32 ANR_PTY_VGAIN_U_U	 = rParam.rUpperIso_UpperZoom.yad2.bits.ANR_PTY_VGAIN;
	MINT32 ANR_PTY_GAIN_TH_U_U	 = rParam.rUpperIso_UpperZoom.yad2.bits.ANR_PTY_GAIN_TH;
	MINT32 ANR_K_RAT_U_U	     = rParam.rUpperIso_UpperZoom.yad2.bits.ANR_K_RAT;

	// CAM_ANR_Y4LUT1 CAM+A34H
	MINT32 ANR_Y_CPX1_U_U		 = rParam.rUpperIso_UpperZoom.y4lut1.bits.ANR_Y_CPX1;
	MINT32 ANR_Y_CPX2_U_U		 = rParam.rUpperIso_UpperZoom.y4lut1.bits.ANR_Y_CPX2;
	MINT32 ANR_Y_CPX3_U_U		 = rParam.rUpperIso_UpperZoom.y4lut1.bits.ANR_Y_CPX3;

	// CAM_ANR_Y4LUT2 CAM+A38H
	MINT32 ANR_Y_SCALE_CPY0_U_U  = rParam.rUpperIso_UpperZoom.y4lut2.bits.ANR_Y_SCALE_CPY0;
	MINT32 ANR_Y_SCALE_CPY1_U_U  = rParam.rUpperIso_UpperZoom.y4lut2.bits.ANR_Y_SCALE_CPY1;
	MINT32 ANR_Y_SCALE_CPY2_U_U  = rParam.rUpperIso_UpperZoom.y4lut2.bits.ANR_Y_SCALE_CPY2;
	MINT32 ANR_Y_SCALE_CPY3_U_U  = rParam.rUpperIso_UpperZoom.y4lut2.bits.ANR_Y_SCALE_CPY3;

	// CAM_ANR_Y4LUT3 CAM+A3CH
	MINT32 ANR_Y_SCALE_SP0_U_U	 = rParam.rUpperIso_UpperZoom.y4lut3.bits.ANR_Y_SCALE_SP0;
	MINT32 ANR_Y_SCALE_SP1_U_U	 = rParam.rUpperIso_UpperZoom.y4lut3.bits.ANR_Y_SCALE_SP1;
	MINT32 ANR_Y_SCALE_SP2_U_U	 = rParam.rUpperIso_UpperZoom.y4lut3.bits.ANR_Y_SCALE_SP2;
	MINT32 ANR_Y_SCALE_SP3_U_U	 = rParam.rUpperIso_UpperZoom.y4lut3.bits.ANR_Y_SCALE_SP3;

	// CAM_ANR_C4LUT1
	MINT32 ANR_C_CPX1_U_U		 = rParam.rUpperIso_UpperZoom.c4lut1.bits.ANR_C_CPX1;
	MINT32 ANR_C_CPX2_U_U		 = rParam.rUpperIso_UpperZoom.c4lut1.bits.ANR_C_CPX2;
	MINT32 ANR_C_CPX3_U_U		 = rParam.rUpperIso_UpperZoom.c4lut1.bits.ANR_C_CPX3;

	// CAM_ANR_C4LUT2
	MINT32 ANR_C_SCALE_CPY0_U_U  = rParam.rUpperIso_UpperZoom.c4lut2.bits.ANR_C_SCALE_CPY0;
	MINT32 ANR_C_SCALE_CPY1_U_U  = rParam.rUpperIso_UpperZoom.c4lut2.bits.ANR_C_SCALE_CPY1;
	MINT32 ANR_C_SCALE_CPY2_U_U  = rParam.rUpperIso_UpperZoom.c4lut2.bits.ANR_C_SCALE_CPY2;
	MINT32 ANR_C_SCALE_CPY3_U_U  = rParam.rUpperIso_UpperZoom.c4lut2.bits.ANR_C_SCALE_CPY3;

	// CAM_ANR_C4LUT3
	MINT32 ANR_C_SCALE_SP0_U_U	 = rParam.rUpperIso_UpperZoom.c4lut3.bits.ANR_C_SCALE_SP0;
	MINT32 ANR_C_SCALE_SP1_U_U	 = rParam.rUpperIso_UpperZoom.c4lut3.bits.ANR_C_SCALE_SP1;
	MINT32 ANR_C_SCALE_SP2_U_U	 = rParam.rUpperIso_UpperZoom.c4lut3.bits.ANR_C_SCALE_SP2;
	MINT32 ANR_C_SCALE_SP3_U_U	 = rParam.rUpperIso_UpperZoom.c4lut3.bits.ANR_C_SCALE_SP3;

	// CAM_ANR_A4LUT2
	MINT32 ANR_Y_ACT_CPY0_U_U	 = rParam.rUpperIso_UpperZoom.a4lut2.bits.ANR_Y_ACT_CPY0;
	MINT32 ANR_Y_ACT_CPY1_U_U	 = rParam.rUpperIso_UpperZoom.a4lut2.bits.ANR_Y_ACT_CPY1;
	MINT32 ANR_Y_ACT_CPY2_U_U	 = rParam.rUpperIso_UpperZoom.a4lut2.bits.ANR_Y_ACT_CPY2;
	MINT32 ANR_Y_ACT_CPY3_U_U	 = rParam.rUpperIso_UpperZoom.a4lut2.bits.ANR_Y_ACT_CPY3;

	// CAM_ANR_A4LUT3
	MINT32 ANR_Y_ACT_SP0_U_U	 = rParam.rUpperIso_UpperZoom.a4lut3.bits.ANR_Y_ACT_SP0;
	MINT32 ANR_Y_ACT_SP1_U_U	 = rParam.rUpperIso_UpperZoom.a4lut3.bits.ANR_Y_ACT_SP1;
	MINT32 ANR_Y_ACT_SP2_U_U	 = rParam.rUpperIso_UpperZoom.a4lut3.bits.ANR_Y_ACT_SP2;
	MINT32 ANR_Y_ACT_SP3_U_U	 = rParam.rUpperIso_UpperZoom.a4lut3.bits.ANR_Y_ACT_SP3;

	// CAM_ANR_L4LUT1
	MINT32 ANR_LCE_X1_U_U		 = rParam.rUpperIso_UpperZoom.l4lut1.bits.ANR_LCE_X1;
	MINT32 ANR_LCE_X2_U_U		 = rParam.rUpperIso_UpperZoom.l4lut1.bits.ANR_LCE_X2;
	MINT32 ANR_LCE_X3_U_U		 = rParam.rUpperIso_UpperZoom.l4lut1.bits.ANR_LCE_X3;

	// CAM_ANR_L4LUT2
	MINT32 ANR_LCE_GAIN0_U_U	 = rParam.rUpperIso_UpperZoom.l4lut2.bits.ANR_LCE_GAIN0;
	MINT32 ANR_LCE_GAIN1_U_U	 = rParam.rUpperIso_UpperZoom.l4lut2.bits.ANR_LCE_GAIN1;
	MINT32 ANR_LCE_GAIN2_U_U	 = rParam.rUpperIso_UpperZoom.l4lut2.bits.ANR_LCE_GAIN2;
	MINT32 ANR_LCE_GAIN3_U_U	 = rParam.rUpperIso_UpperZoom.l4lut2.bits.ANR_LCE_GAIN3;

	// CAM_ANR_L4LUT3
	MINT32 ANR_LCE_SP0_U_U		 = rParam.rUpperIso_UpperZoom.l4lut3.bits.ANR_LCE_SP0;
	MINT32 ANR_LCE_SP1_U_U		 = rParam.rUpperIso_UpperZoom.l4lut3.bits.ANR_LCE_SP1;
	MINT32 ANR_LCE_SP2_U_U		 = rParam.rUpperIso_UpperZoom.l4lut3.bits.ANR_LCE_SP2;
	MINT32 ANR_LCE_SP3_U_U		 = rParam.rUpperIso_UpperZoom.l4lut3.bits.ANR_LCE_SP3;

	// CAM_ANR_PTY CAM+A40H
	MINT32 ANR_PTY1_U_U 		 = rParam.rUpperIso_UpperZoom.pty.bits.ANR_PTY1;
	MINT32 ANR_PTY2_U_U 		 = rParam.rUpperIso_UpperZoom.pty.bits.ANR_PTY2;
	MINT32 ANR_PTY3_U_U 		 = rParam.rUpperIso_UpperZoom.pty.bits.ANR_PTY3;
	MINT32 ANR_PTY4_U_U 		 = rParam.rUpperIso_UpperZoom.pty.bits.ANR_PTY4;

	// CAM_ANR_CAD CAM+A44H
	MINT32 ANR_PTC_VGAIN_U_U	 = rParam.rUpperIso_UpperZoom.cad.bits.ANR_PTC_VGAIN;
	MINT32 ANR_PTC_GAIN_TH_U_U	 = rParam.rUpperIso_UpperZoom.cad.bits.ANR_PTC_GAIN_TH;
	MINT32 ANR_C_L_DIFF_TH_U_U	 = rParam.rUpperIso_UpperZoom.cad.bits.ANR_C_L_DIFF_TH;

	// CAM_ANR_PTC CAM+A48H
	MINT32 ANR_PTC1_U_U 		 = rParam.rUpperIso_UpperZoom.ptc.bits.ANR_PTC1;
	MINT32 ANR_PTC2_U_U 		 = rParam.rUpperIso_UpperZoom.ptc.bits.ANR_PTC2;
	MINT32 ANR_PTC3_U_U 		 = rParam.rUpperIso_UpperZoom.ptc.bits.ANR_PTC3;
	MINT32 ANR_PTC4_U_U 		 = rParam.rUpperIso_UpperZoom.ptc.bits.ANR_PTC4;

	// CAM_ANR_LCE CAM+A4CH
	MINT32 ANR_LCE_C_GAIN_U_U	 = rParam.rUpperIso_UpperZoom.lce.bits.ANR_LCE_C_GAIN;
	MINT32 ANR_LCE_SCALE_GAIN_U_U= rParam.rUpperIso_UpperZoom.lce.bits.ANR_LCE_SCALE_GAIN;
	MINT32 ANR_LM_WT_U_U		 = rParam.rUpperIso_UpperZoom.lce.bits.ANR_LM_WT;



	// CAM_ANR_ACT1 CAM+A64H
	MINT32 ANR_ACT_LCE_GAIN_U_U  = rParam.rUpperIso_UpperZoom.act1.bits.ANR_ACT_LCE_GAIN;
	MINT32 ANR_ACT_DIF_GAIN_U_U  = rParam.rUpperIso_UpperZoom.act1.bits.ANR_ACT_DIF_GAIN;
	MINT32 ANR_ACT_DIF_LO_TH_U_U = rParam.rUpperIso_UpperZoom.act1.bits.ANR_ACT_DIF_LO_TH;

	// CAM_ANR_ACT2 CAM+A64H
	MINT32 ANR_ACT_SIZE_GAIN_U_U = rParam.rUpperIso_UpperZoom.act2.bits.ANR_ACT_SIZE_GAIN;
	MINT32 ANR_ACT_SIZE_LO_TH_U_U= rParam.rUpperIso_UpperZoom.act2.bits.ANR_ACT_SIZE_LO_TH;

	// CAM_ANR_ACTC CAM+A64H
	MINT32 ANR_ACT_BLD_BASE_C_U_U= rParam.rUpperIso_UpperZoom.actc.bits.ANR_ACT_BLD_BASE_C;
	MINT32 ANR_C_DITH_U_U_U= rParam.rUpperIso_UpperZoom.actc.bits.ANR_C_DITH_U;
	MINT32 ANR_C_DITH_V_U_U= rParam.rUpperIso_UpperZoom.actc.bits.ANR_C_DITH_V;

	// DIP_X_ANR_ACTYL CAM+A60H
	MINT32 ANR_ACT_TH_Y_L_U_U = rParam.rUpperIso_UpperZoom.actyl.bits.ANR_ACT_TH_Y_L;
	MINT32 ANR_ACT_BLD_BASE_Y_L_U_U = rParam.rUpperIso_UpperZoom.actyl.bits.ANR_ACT_BLD_BASE_Y_L;
	MINT32 ANR_ACT_SLANT_Y_L_U_U = rParam.rUpperIso_UpperZoom.actyl.bits.ANR_ACT_SLANT_Y_L;
	MINT32 ANR_ACT_BLD_TH_Y_L_U_U = rParam.rUpperIso_UpperZoom.actyl.bits.ANR_ACT_BLD_TH_Y_L;

	// DIP_X_ANR_YLAD CAM+A60H
	MINT32 ANR_CEN_GAIN_LO_TH_LPF_U_U = rParam.rUpperIso_UpperZoom.ylad.bits.ANR_CEN_GAIN_LO_TH_LPF;
	MINT32 ANR_CEN_GAIN_HI_TH_LPF_U_U = rParam.rUpperIso_UpperZoom.ylad.bits.ANR_CEN_GAIN_HI_TH_LPF;
	MINT32 ANR_K_LMT_U_U = rParam.rUpperIso_UpperZoom.ylad.bits.ANR_K_LMT;
	MINT32 ANR_K_LPF_U_U = rParam.rUpperIso_UpperZoom.ylad.bits.ANR_K_LPF;

	// DIP_X_ANR_PTYL CAM+A60H
	MINT32 ANR_PTY1_LPF_U_U = rParam.rUpperIso_UpperZoom.ptyl.bits.ANR_PTY1_LPF;
	MINT32 ANR_PTY2_LPF_U_U = rParam.rUpperIso_UpperZoom.ptyl.bits.ANR_PTY2_LPF;
	MINT32 ANR_PTY3_LPF_U_U = rParam.rUpperIso_UpperZoom.ptyl.bits.ANR_PTY3_LPF;
	MINT32 ANR_PTY4_LPF_U_U = rParam.rUpperIso_UpperZoom.ptyl.bits.ANR_PTY4_LPF;

	// DIP_X_ANR_LCOEF CAM+A60H
	MINT32 ANR_COEF_A_U_U = rParam.rUpperIso_UpperZoom.lcoef.bits.ANR_COEF_A;
	MINT32 ANR_COEF_B_U_U = rParam.rUpperIso_UpperZoom.lcoef.bits.ANR_COEF_B;
	MINT32 ANR_COEF_C_U_U = rParam.rUpperIso_UpperZoom.lcoef.bits.ANR_COEF_C;
	MINT32 ANR_COEF_D_U_U = rParam.rUpperIso_UpperZoom.lcoef.bits.ANR_COEF_D;

	// Olympu
	MINT32 ANR_C_SM_EDGE_TH_U_U 	  = rParam.rUpperIso_UpperZoom.con2.bits.ANR_C_SM_EDGE_TH;
	MINT32 ANR_K_HPF_U_U			  = rParam.rUpperIso_UpperZoom.ylad.bits.ANR_K_HPF;
	MINT32 ANR_ACT_DIF_HI_TH_U_U	  = rParam.rUpperIso_UpperZoom.act2.bits.ANR_ACT_DIF_HI_TH;
	MINT32 ANR_ACT_TH_Y_HL_U_U		  = rParam.rUpperIso_UpperZoom.actyhl.bits.ANR_ACT_TH_Y_HL;
	MINT32 ANR_ACT_BLD_BASE_Y_HL_U_U  = rParam.rUpperIso_UpperZoom.actyhl.bits.ANR_ACT_BLD_BASE_Y_HL;
	MINT32 ANR_ACT_SLANT_Y_HL_U_U	  = rParam.rUpperIso_UpperZoom.actyhl.bits.ANR_ACT_SLANT_Y_HL;
	MINT32 ANR_ACT_BLD_TH_Y_HL_U_U	  = rParam.rUpperIso_UpperZoom.actyhl.bits.ANR_ACT_BLD_TH_Y_HL;
	MINT32 ANR_ACT_TH_Y_HH_U_U		  = rParam.rUpperIso_UpperZoom.actyhh.bits.ANR_ACT_TH_Y_HH;
	MINT32 ANR_ACT_BLD_BASE_Y_HH_U_U  = rParam.rUpperIso_UpperZoom.actyhh.bits.ANR_ACT_BLD_BASE_Y_HH;
	MINT32 ANR_ACT_SLANT_Y_HH_U_U	  = rParam.rUpperIso_UpperZoom.actyhh.bits.ANR_ACT_SLANT_Y_HH;
	MINT32 ANR_ACT_BLD_TH_Y_HH_U_U	  = rParam.rUpperIso_UpperZoom.actyhh.bits.ANR_ACT_BLD_TH_Y_HH;
	MINT32 ANR_LUMA_TH_Y_L_U_U		  = rParam.rUpperIso_UpperZoom.actyl2.bits.ANR_LUMA_TH_Y_L;
	MINT32 ANR_LUMA_BLD_BASE_Y_L_U_U  = rParam.rUpperIso_UpperZoom.actyl2.bits.ANR_LUMA_BLD_BASE_Y_L;
	MINT32 ANR_LUMA_SLANT_Y_L_U_U	  = rParam.rUpperIso_UpperZoom.actyl2.bits.ANR_LUMA_SLANT_Y_L;
	MINT32 ANR_LUMA_BLD_TH_Y_L_U_U	  = rParam.rUpperIso_UpperZoom.actyl2.bits.ANR_LUMA_BLD_TH_Y_L;
	MINT32 ANR_LUMA_TH_Y_HL_U_U 	  = rParam.rUpperIso_UpperZoom.actyhl2.bits.ANR_LUMA_TH_Y_HL;
	MINT32 ANR_LUMA_BLD_BASE_Y_HL_U_U = rParam.rUpperIso_UpperZoom.actyhl2.bits.ANR_LUMA_BLD_BASE_Y_HL;
	MINT32 ANR_LUMA_SLANT_Y_HL_U_U	  = rParam.rUpperIso_UpperZoom.actyhl2.bits.ANR_LUMA_SLANT_Y_HL;
	MINT32 ANR_LUMA_BLD_TH_Y_HL_U_U   = rParam.rUpperIso_UpperZoom.actyhl2.bits.ANR_LUMA_BLD_TH_Y_HL;
	MINT32 ANR_LUMA_TH_Y_HH_U_U 	  = rParam.rUpperIso_UpperZoom.actyhh2.bits.ANR_LUMA_TH_Y_HH;
	MINT32 ANR_LUMA_BLD_BASE_Y_HH_U_U = rParam.rUpperIso_UpperZoom.actyhh2.bits.ANR_LUMA_BLD_BASE_Y_HH;
	MINT32 ANR_LUMA_SLANT_Y_HH_U_U	  = rParam.rUpperIso_UpperZoom.actyhh2.bits.ANR_LUMA_SLANT_Y_HH;
	MINT32 ANR_LUMA_BLD_TH_Y_HH_U_U   = rParam.rUpperIso_UpperZoom.actyhh2.bits.ANR_LUMA_BLD_TH_Y_HH;
	MINT32 ANR_PTY_L_GAIN_U_U		  = rParam.rUpperIso_UpperZoom.ylad2.bits.ANR_PTY_L_GAIN;
	MINT32 ANR_PTY_H_GAIN_U_U		  = rParam.rUpperIso_UpperZoom.ylad2.bits.ANR_PTY_H_GAIN;
	MINT32 ANR_BLEND_TH_L_U_U		  = rParam.rUpperIso_UpperZoom.ylad2.bits.ANR_BLEND_TH_L;
	MINT32 ANR_BLEND_TH_H_U_U		  = rParam.rUpperIso_UpperZoom.ylad2.bits.ANR_BLEND_TH_H;
	MINT32 ANR_HI_LMT1_U_U			  = rParam.rUpperIso_UpperZoom.ylad3.bits.ANR_HI_LMT1;
	MINT32 ANR_HI_LMT2_U_U			  = rParam.rUpperIso_UpperZoom.ylad3.bits.ANR_HI_LMT2;
	MINT32 ANR_LO_LMT1_U_U			  = rParam.rUpperIso_UpperZoom.ylad3.bits.ANR_LO_LMT1;
	MINT32 ANR_LO_LMT2_U_U			  = rParam.rUpperIso_UpperZoom.ylad3.bits.ANR_LO_LMT2;
	MINT32 ANR_Y_ACT_CEN_OFT_U_U	  = rParam.rUpperIso_UpperZoom.act4.bits.ANR_Y_ACT_CEN_OFT;
	MINT32 ANR_Y_ACT_CEN_GAIN_U_U	  = rParam.rUpperIso_UpperZoom.act4.bits.ANR_Y_ACT_CEN_GAIN;
	MINT32 ANR_Y_ACT_CEN_TH_U_U 	  = rParam.rUpperIso_UpperZoom.act4.bits.ANR_Y_ACT_CEN_TH;
	MINT32 ANR_Y_DIR_U_U			  = rParam.rUpperIso_UpperZoom.con1.bits.ANR_Y_DIR;
	MINT32 ANR_Y_DIR_GAIN_U_U		  = rParam.rUpperIso_UpperZoom.ydir.bits.ANR_Y_DIR_GAIN;
	MINT32 ANR_Y_DIR_VER_W_TH_U_U	  = rParam.rUpperIso_UpperZoom.ydir.bits.ANR_Y_DIR_VER_W_TH;
	MINT32 ANR_Y_DIR_VER_W_SL_U_U	  = rParam.rUpperIso_UpperZoom.ydir.bits.ANR_Y_DIR_VER_W_SL;
	MINT32 ANR_Y_DIR_DIAG_W_TH_U_U	  = rParam.rUpperIso_UpperZoom.ydir.bits.ANR_Y_DIR_DIAG_W_TH;
	MINT32 ANR_Y_DIR_DIAG_W_SL_U_U	  = rParam.rUpperIso_UpperZoom.ydir.bits.ANR_Y_DIR_DIAG_W_SL;

	// Whitney E2

	MINT32 ANR_TBL_CPX1_U_U		 = rParam.rUpperIso_UpperZoom.t4lut1.bits.ANR_TBL_CPX1;
	MINT32 ANR_TBL_CPX2_U_U		 = rParam.rUpperIso_UpperZoom.t4lut1.bits.ANR_TBL_CPX2;
	MINT32 ANR_TBL_CPX3_U_U		 = rParam.rUpperIso_UpperZoom.t4lut1.bits.ANR_TBL_CPX3;
	MINT32 ANR_TBL_GAIN_CPY0_U_U		 = rParam.rUpperIso_UpperZoom.t4lut2.bits.ANR_TBL_GAIN_CPY0;
	MINT32 ANR_TBL_GAIN_CPY1_U_U		 = rParam.rUpperIso_UpperZoom.t4lut2.bits.ANR_TBL_GAIN_CPY1;
	MINT32 ANR_TBL_GAIN_CPY2_U_U		 = rParam.rUpperIso_UpperZoom.t4lut2.bits.ANR_TBL_GAIN_CPY2;
	MINT32 ANR_TBL_GAIN_CPY3_U_U		 = rParam.rUpperIso_UpperZoom.t4lut2.bits.ANR_TBL_GAIN_CPY3;
	MINT32 ANR_TBL_GAIN_SP0_U_U		 = rParam.rUpperIso_UpperZoom.t4lut3.bits.ANR_TBL_GAIN_SP0;
	MINT32 ANR_TBL_GAIN_SP1_U_U		 = rParam.rUpperIso_UpperZoom.t4lut3.bits.ANR_TBL_GAIN_SP1;
	MINT32 ANR_TBL_GAIN_SP2_U_U		 = rParam.rUpperIso_UpperZoom.t4lut3.bits.ANR_TBL_GAIN_SP2;
	MINT32 ANR_TBL_GAIN_SP3_U_U		 = rParam.rUpperIso_UpperZoom.t4lut3.bits.ANR_TBL_GAIN_SP3;

	//	  ====================================================================================================================================
	//	  Interpolating reg declaration
	//	  ====================================================================================================================================
	// CAM_ANR_CON1 CAM+A20H
	MINT32 ANR_ENC;
	MINT32 ANR_ENY;
	MINT32 ANR_SCALE_MODE;
	MINT32 ANR_LCE_LINK;
	MINT32 ANR_TABLE_EN;
	MINT32 ANR_TBL_PRC;

	// CAM_ANR_CON2 CAM+A24H
	MINT32 ANR_IMPL_MODE;
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
	MINT32 ANR_K_RAT;

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


	// CAM_ANR_ACT1 CAM+A64H
	MINT32 ANR_ACT_LCE_GAIN;
	MINT32 ANR_ACT_DIF_GAIN;
	MINT32 ANR_ACT_DIF_LO_TH;

	// CAM_ANR_ACT2 CAM+A64H
	MINT32 ANR_ACT_SIZE_GAIN;
	MINT32 ANR_ACT_SIZE_LO_TH;

	// CAM_ANR_ACTC CAM+A64H
	MINT32 ANR_ACT_BLD_BASE_C;
	MINT32 ANR_C_DITH_U;
	MINT32 ANR_C_DITH_V;

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

	//Olympu
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

	// Whitney E2

	MINT32 ANR_TBL_CPX1;
	MINT32 ANR_TBL_CPX2;
	MINT32 ANR_TBL_CPX3;
	MINT32 ANR_TBL_GAIN_CPY0;
	MINT32 ANR_TBL_GAIN_CPY1;
	MINT32 ANR_TBL_GAIN_CPY2;
	MINT32 ANR_TBL_GAIN_CPY3;
	MINT32 ANR_TBL_GAIN_SP0;
	MINT32 ANR_TBL_GAIN_SP1;
	MINT32 ANR_TBL_GAIN_SP2;
	MINT32 ANR_TBL_GAIN_SP3;

	//	  ====================================================================================================================================
	//	  Negative Conversion
	//	  ====================================================================================================================================


	ANR_Y_SCALE_SP3_L_L = (ANR_Y_SCALE_SP3_L_L > 15) ? (ANR_Y_SCALE_SP3_L_L - 32) : ANR_Y_SCALE_SP3_L_L;
	ANR_C_SCALE_SP3_L_L = (ANR_C_SCALE_SP3_L_L > 15) ? (ANR_C_SCALE_SP3_L_L - 32) : ANR_C_SCALE_SP3_L_L;
	ANR_Y_ACT_SP3_L_L	= (ANR_Y_ACT_SP3_L_L > 31) ? (ANR_Y_ACT_SP3_L_L - 64) : ANR_Y_ACT_SP3_L_L;
	ANR_LCE_SP3_L_L 	= (ANR_LCE_SP3_L_L > 31) ? (ANR_LCE_SP3_L_L - 64) : ANR_LCE_SP3_L_L;
	ANR_TBL_GAIN_SP3_L_L = (ANR_TBL_GAIN_SP3_L_L > 15) ? (ANR_TBL_GAIN_SP3_L_L - 32) : ANR_TBL_GAIN_SP3_L_L;


	ANR_Y_SCALE_SP3_L_U = (ANR_Y_SCALE_SP3_L_U > 15) ? (ANR_Y_SCALE_SP3_L_U - 32) : ANR_Y_SCALE_SP3_L_U;
	ANR_C_SCALE_SP3_L_U = (ANR_C_SCALE_SP3_L_U > 15) ? (ANR_C_SCALE_SP3_L_U - 32) : ANR_C_SCALE_SP3_L_U;
	ANR_Y_ACT_SP3_L_U	= (ANR_Y_ACT_SP3_L_U > 31) ? (ANR_Y_ACT_SP3_L_U - 64) : ANR_Y_ACT_SP3_L_U;
	ANR_LCE_SP3_L_U 	= (ANR_LCE_SP3_L_U > 31) ? (ANR_LCE_SP3_L_U - 64) : ANR_LCE_SP3_L_U;
	ANR_TBL_GAIN_SP3_L_U = (ANR_TBL_GAIN_SP3_L_U > 15) ? (ANR_TBL_GAIN_SP3_L_U - 32) : ANR_TBL_GAIN_SP3_L_U;

	ANR_Y_SCALE_SP3_U_L = (ANR_Y_SCALE_SP3_U_L > 15) ? (ANR_Y_SCALE_SP3_U_L - 32) : ANR_Y_SCALE_SP3_U_L;
	ANR_C_SCALE_SP3_U_L = (ANR_C_SCALE_SP3_U_L > 15) ? (ANR_C_SCALE_SP3_U_L - 32) : ANR_C_SCALE_SP3_U_L;
	ANR_Y_ACT_SP3_U_L	= (ANR_Y_ACT_SP3_U_L > 31) ? (ANR_Y_ACT_SP3_U_L - 64) : ANR_Y_ACT_SP3_U_L;
	ANR_LCE_SP3_U_L 	= (ANR_LCE_SP3_U_L > 31) ? (ANR_LCE_SP3_U_L - 64) : ANR_LCE_SP3_U_L;
	ANR_TBL_GAIN_SP3_U_L = (ANR_TBL_GAIN_SP3_U_L > 15) ? (ANR_TBL_GAIN_SP3_U_L - 32) : ANR_TBL_GAIN_SP3_U_L;

	ANR_Y_SCALE_SP3_U_U = (ANR_Y_SCALE_SP3_U_U > 15) ? (ANR_Y_SCALE_SP3_U_U - 32) : ANR_Y_SCALE_SP3_U_U;
	ANR_C_SCALE_SP3_U_U = (ANR_C_SCALE_SP3_U_U > 15) ? (ANR_C_SCALE_SP3_U_U - 32) : ANR_C_SCALE_SP3_U_U;
	ANR_Y_ACT_SP3_U_U	= (ANR_Y_ACT_SP3_U_U > 31) ? (ANR_Y_ACT_SP3_U_U - 64) : ANR_Y_ACT_SP3_U_U;
	ANR_LCE_SP3_U_U 	= (ANR_LCE_SP3_U_U > 31) ? (ANR_LCE_SP3_U_U - 64) : ANR_LCE_SP3_U_U;
	ANR_TBL_GAIN_SP3_U_U = (ANR_TBL_GAIN_SP3_U_U > 15) ? (ANR_TBL_GAIN_SP3_U_U - 32) : ANR_TBL_GAIN_SP3_U_U;



	//	  ====================================================================================================================================
	//	  Start Parameter Interpolation
	//	  ====================================================================================================================================

		//	  Registers that can not be interpolated
		ANR_ENC 		   = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_ENC_L_L, ANR_ENC_L_U, ANR_ENC_U_L, ANR_ENC_U_U,1);
		ANR_ENY 		   = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_ENY_L_L, ANR_ENY_L_U, ANR_ENY_U_L, ANR_ENY_U_U,1);
		ANR_TABLE_EN	   = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_TABLE_EN_L_L, ANR_TABLE_EN_L_U, ANR_TABLE_EN_U_L, ANR_TABLE_EN_U_U,1);
		ANR_TBL_PRC 	   = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_TBL_PRC_L_L, ANR_TBL_PRC_L_U, ANR_TBL_PRC_U_L, ANR_TBL_PRC_U_U,1);
		ANR_IMPL_MODE	   = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_IMPL_MODE_L_L, ANR_IMPL_MODE_L_U, ANR_IMPL_MODE_U_L, ANR_IMPL_MODE_U_U,1);
		ANR_C_SM_EDGE	   = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_C_SM_EDGE_L_L, ANR_C_SM_EDGE_L_U, ANR_C_SM_EDGE_U_L, ANR_C_SM_EDGE_U_U,1);
		ANR_FLT_C		   = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_FLT_C_L_L, ANR_FLT_C_L_U, ANR_FLT_C_U_L, ANR_FLT_C_U_U,1);
		ANR_LR			   = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_LR_L_L, ANR_LR_L_U, ANR_LR_U_L, ANR_LR_U_U,1);

		//	  Registers that can be interpolated
		ANR_ALPHA			 = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_ALPHA_L_L, ANR_ALPHA_L_U, ANR_ALPHA_U_L, ANR_ALPHA_U_U,0);
		ANR_CEN_GAIN_LO_TH			  = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_CEN_GAIN_LO_TH_L_L, ANR_CEN_GAIN_LO_TH_L_U, ANR_CEN_GAIN_LO_TH_U_L, ANR_CEN_GAIN_LO_TH_U_U,0);
		ANR_CEN_GAIN_HI_TH			  = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_CEN_GAIN_HI_TH_L_L, ANR_CEN_GAIN_HI_TH_L_U, ANR_CEN_GAIN_HI_TH_U_L, ANR_CEN_GAIN_HI_TH_U_U,0);
		ANR_PTY_VGAIN			 = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_PTY_VGAIN_L_L, ANR_PTY_VGAIN_L_U, ANR_PTY_VGAIN_U_L, ANR_PTY_VGAIN_U_U,0);
		ANR_PTY_GAIN_TH 		   = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_PTY_GAIN_TH_L_L, ANR_PTY_GAIN_TH_L_U, ANR_PTY_GAIN_TH_U_L, ANR_PTY_GAIN_TH_U_U,0);
		ANR_K_RAT 		   = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_K_RAT_L_L, ANR_K_RAT_L_U, ANR_K_RAT_U_L, ANR_K_RAT_U_U,0);
		ANR_Y_CPX1			  = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_Y_CPX1_L_L, ANR_Y_CPX1_L_U, ANR_Y_CPX1_U_L, ANR_Y_CPX1_U_U,0);
		ANR_Y_CPX2			  = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_Y_CPX2_L_L, ANR_Y_CPX2_L_U, ANR_Y_CPX2_U_L, ANR_Y_CPX2_U_U,0);
		ANR_Y_CPX3			  = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_Y_CPX3_L_L, ANR_Y_CPX3_L_U, ANR_Y_CPX3_U_L, ANR_Y_CPX3_U_U,0);
		ANR_Y_SCALE_CPY0			= InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_Y_SCALE_CPY0_L_L, ANR_Y_SCALE_CPY0_L_U, ANR_Y_SCALE_CPY0_U_L, ANR_Y_SCALE_CPY0_U_U,0);
		ANR_Y_SCALE_CPY1			= InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_Y_SCALE_CPY1_L_L, ANR_Y_SCALE_CPY1_L_U, ANR_Y_SCALE_CPY1_U_L, ANR_Y_SCALE_CPY1_U_U,0);
		ANR_Y_SCALE_CPY2			= InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_Y_SCALE_CPY2_L_L, ANR_Y_SCALE_CPY2_L_U, ANR_Y_SCALE_CPY2_U_L, ANR_Y_SCALE_CPY2_U_U,0);
		ANR_Y_SCALE_CPY3			= InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_Y_SCALE_CPY3_L_L, ANR_Y_SCALE_CPY3_L_U, ANR_Y_SCALE_CPY3_U_L, ANR_Y_SCALE_CPY3_U_U,0);
		ANR_C_CPX1			  = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_C_CPX1_L_L, ANR_C_CPX1_L_U, ANR_C_CPX1_U_L, ANR_C_CPX1_U_U,0);
		ANR_C_CPX2			  = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_C_CPX2_L_L, ANR_C_CPX2_L_U, ANR_C_CPX2_U_L, ANR_C_CPX2_U_U,0);
		ANR_C_CPX3			  = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_C_CPX3_L_L, ANR_C_CPX3_L_U, ANR_C_CPX3_U_L, ANR_C_CPX3_U_U,0);
		ANR_C_SCALE_CPY0			= InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_C_SCALE_CPY0_L_L, ANR_C_SCALE_CPY0_L_U, ANR_C_SCALE_CPY0_U_L, ANR_C_SCALE_CPY0_U_U,0);
		ANR_C_SCALE_CPY1			= InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_C_SCALE_CPY1_L_L, ANR_C_SCALE_CPY1_L_U, ANR_C_SCALE_CPY1_U_L, ANR_C_SCALE_CPY1_U_U,0);
		ANR_C_SCALE_CPY2			= InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_C_SCALE_CPY2_L_L, ANR_C_SCALE_CPY2_L_U, ANR_C_SCALE_CPY2_U_L, ANR_C_SCALE_CPY2_U_U,0);
		ANR_C_SCALE_CPY3			= InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_C_SCALE_CPY3_L_L, ANR_C_SCALE_CPY3_L_U, ANR_C_SCALE_CPY3_U_L, ANR_C_SCALE_CPY3_U_U,0);
		ANR_Y_ACT_CPY0			  = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_Y_ACT_CPY0_L_L, ANR_Y_ACT_CPY0_L_U, ANR_Y_ACT_CPY0_U_L, ANR_Y_ACT_CPY0_U_U,0);
		ANR_Y_ACT_CPY1			  = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_Y_ACT_CPY1_L_L, ANR_Y_ACT_CPY1_L_U, ANR_Y_ACT_CPY1_U_L, ANR_Y_ACT_CPY1_U_U,0);
		ANR_Y_ACT_CPY2			  = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_Y_ACT_CPY2_L_L, ANR_Y_ACT_CPY2_L_U, ANR_Y_ACT_CPY2_U_L, ANR_Y_ACT_CPY2_U_U,0);
		ANR_Y_ACT_CPY3			  = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_Y_ACT_CPY3_L_L, ANR_Y_ACT_CPY3_L_U, ANR_Y_ACT_CPY3_U_L, ANR_Y_ACT_CPY3_U_U,0);
		ANR_PTY1			= InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_PTY1_L_L, ANR_PTY1_L_U, ANR_PTY1_U_L, ANR_PTY1_U_U,0);
		ANR_PTY2			= InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_PTY2_L_L, ANR_PTY2_L_U, ANR_PTY2_U_L, ANR_PTY2_U_U,0);
		ANR_PTY3			= InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_PTY3_L_L, ANR_PTY3_L_U, ANR_PTY3_U_L, ANR_PTY3_U_U,0);
		ANR_PTY4			= InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_PTY4_L_L, ANR_PTY4_L_U, ANR_PTY4_U_L, ANR_PTY4_U_U,0);
		ANR_PTC_VGAIN			 = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_PTC_VGAIN_L_L, ANR_PTC_VGAIN_L_U, ANR_PTC_VGAIN_U_L, ANR_PTC_VGAIN_U_U,0);
		ANR_PTC_GAIN_TH 		   = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_PTC_GAIN_TH_L_L, ANR_PTC_GAIN_TH_L_U, ANR_PTC_GAIN_TH_U_L, ANR_PTC_GAIN_TH_U_U,0);
		ANR_C_L_DIFF_TH 		   = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_C_L_DIFF_TH_L_L, ANR_C_L_DIFF_TH_L_U, ANR_C_L_DIFF_TH_U_L, ANR_C_L_DIFF_TH_U_U,0);
		ANR_PTC1			= InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_PTC1_L_L, ANR_PTC1_L_U, ANR_PTC1_U_L, ANR_PTC1_U_U,0);
		ANR_PTC2			= InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_PTC2_L_L, ANR_PTC2_L_U, ANR_PTC2_U_L, ANR_PTC2_U_U,0);
		ANR_PTC3			= InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_PTC3_L_L, ANR_PTC3_L_U, ANR_PTC3_U_L, ANR_PTC3_U_U,0);
		ANR_PTC4			= InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_PTC4_L_L, ANR_PTC4_L_U, ANR_PTC4_U_L, ANR_PTC4_U_U,0);
		ANR_LM_WT			 = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_LM_WT_L_L, ANR_LM_WT_L_U, ANR_LM_WT_U_L, ANR_LM_WT_U_U,0);
		ANR_ACT_LCE_GAIN			= InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_ACT_LCE_GAIN_L_L, ANR_ACT_LCE_GAIN_L_U, ANR_ACT_LCE_GAIN_U_L, ANR_ACT_LCE_GAIN_U_U,0);
		ANR_ACT_DIF_GAIN			= InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_ACT_DIF_GAIN_L_L, ANR_ACT_DIF_GAIN_L_U, ANR_ACT_DIF_GAIN_U_L, ANR_ACT_DIF_GAIN_U_U,0);
		ANR_ACT_DIF_LO_TH			 = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_ACT_DIF_LO_TH_L_L, ANR_ACT_DIF_LO_TH_L_U, ANR_ACT_DIF_LO_TH_U_L, ANR_ACT_DIF_LO_TH_U_U,0);
		ANR_ACT_SIZE_GAIN			 = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_ACT_SIZE_GAIN_L_L, ANR_ACT_SIZE_GAIN_L_U, ANR_ACT_SIZE_GAIN_U_L, ANR_ACT_SIZE_GAIN_U_U,0);
		ANR_ACT_SIZE_LO_TH			  = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_ACT_SIZE_LO_TH_L_L, ANR_ACT_SIZE_LO_TH_L_U, ANR_ACT_SIZE_LO_TH_U_L, ANR_ACT_SIZE_LO_TH_U_U,0);
		ANR_ACT_BLD_BASE_C			  = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_ACT_BLD_BASE_C_L_L, ANR_ACT_BLD_BASE_C_L_U, ANR_ACT_BLD_BASE_C_U_L, ANR_ACT_BLD_BASE_C_U_U,0);
		ANR_C_DITH_U			  = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_C_DITH_U_L_L, ANR_C_DITH_U_L_U, ANR_C_DITH_U_U_L, ANR_C_DITH_U_U_U,0);
		ANR_C_DITH_V			  = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_C_DITH_V_L_L, ANR_C_DITH_V_L_U, ANR_C_DITH_V_U_L, ANR_C_DITH_V_U_U,0);
		ANR_ACT_TH_Y_L			  = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_ACT_TH_Y_L_L_L, ANR_ACT_TH_Y_L_L_U, ANR_ACT_TH_Y_L_U_L, ANR_ACT_TH_Y_L_U_U,0);
		ANR_ACT_BLD_BASE_Y_L			= InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_ACT_BLD_BASE_Y_L_L_L, ANR_ACT_BLD_BASE_Y_L_L_U, ANR_ACT_BLD_BASE_Y_L_U_L, ANR_ACT_BLD_BASE_Y_L_U_U,0);
		ANR_ACT_SLANT_Y_L			 = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_ACT_SLANT_Y_L_L_L, ANR_ACT_SLANT_Y_L_L_U, ANR_ACT_SLANT_Y_L_U_L, ANR_ACT_SLANT_Y_L_U_U,0);
		ANR_ACT_BLD_TH_Y_L			  = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_ACT_BLD_TH_Y_L_L_L, ANR_ACT_BLD_TH_Y_L_L_U, ANR_ACT_BLD_TH_Y_L_U_L, ANR_ACT_BLD_TH_Y_L_U_U,0);
		ANR_CEN_GAIN_LO_TH_LPF			  = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_CEN_GAIN_LO_TH_LPF_L_L, ANR_CEN_GAIN_LO_TH_LPF_L_U, ANR_CEN_GAIN_LO_TH_LPF_U_L, ANR_CEN_GAIN_LO_TH_LPF_U_U,0);
		ANR_CEN_GAIN_HI_TH_LPF			  = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_CEN_GAIN_HI_TH_LPF_L_L, ANR_CEN_GAIN_HI_TH_LPF_L_U, ANR_CEN_GAIN_HI_TH_LPF_U_L, ANR_CEN_GAIN_HI_TH_LPF_U_U,0);
		ANR_K_LMT			 = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_K_LMT_L_L, ANR_K_LMT_L_U, ANR_K_LMT_U_L, ANR_K_LMT_U_U,0);
		ANR_K_LPF			 = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_K_LPF_L_L, ANR_K_LPF_L_U, ANR_K_LPF_U_L, ANR_K_LPF_U_U,0);
		ANR_PTY1_LPF			= InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_PTY1_LPF_L_L, ANR_PTY1_LPF_L_U, ANR_PTY1_LPF_U_L, ANR_PTY1_LPF_U_U,0);
		ANR_PTY2_LPF			= InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_PTY2_LPF_L_L, ANR_PTY2_LPF_L_U, ANR_PTY2_LPF_U_L, ANR_PTY2_LPF_U_U,0);
		ANR_PTY3_LPF			= InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_PTY3_LPF_L_L, ANR_PTY3_LPF_L_U, ANR_PTY3_LPF_U_L, ANR_PTY3_LPF_U_U,0);
		ANR_PTY4_LPF			= InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_PTY4_LPF_L_L, ANR_PTY4_LPF_L_U, ANR_PTY4_LPF_U_L, ANR_PTY4_LPF_U_U,0);
		ANR_COEF_A			  = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_COEF_A_L_L, ANR_COEF_A_L_U, ANR_COEF_A_U_L, ANR_COEF_A_U_U,0);
		ANR_COEF_B			  = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_COEF_B_L_L, ANR_COEF_B_L_U, ANR_COEF_B_U_L, ANR_COEF_B_U_U,0);
		ANR_COEF_C			  = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_COEF_C_L_L, ANR_COEF_C_L_U, ANR_COEF_C_U_L, ANR_COEF_C_U_U,0);
		ANR_COEF_D			  = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_COEF_D_L_L, ANR_COEF_D_L_U, ANR_COEF_D_U_L, ANR_COEF_D_U_U,0);

		//Olympu
		ANR_C_SM_EDGE_TH			= InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_C_SM_EDGE_TH_L_L, ANR_C_SM_EDGE_TH_L_U, ANR_C_SM_EDGE_TH_U_L, ANR_C_SM_EDGE_TH_U_U,0);
		ANR_K_HPF			 = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_K_HPF_L_L, ANR_K_HPF_L_U, ANR_K_HPF_U_L, ANR_K_HPF_U_U,0);
		ANR_ACT_DIF_HI_TH			 = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_ACT_DIF_HI_TH_L_L, ANR_ACT_DIF_HI_TH_L_U, ANR_ACT_DIF_HI_TH_U_L, ANR_ACT_DIF_HI_TH_U_U,0);
		ANR_ACT_TH_Y_HL 		   = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_ACT_TH_Y_HL_L_L, ANR_ACT_TH_Y_HL_L_U, ANR_ACT_TH_Y_HL_U_L, ANR_ACT_TH_Y_HL_U_U,0);
		ANR_ACT_BLD_BASE_Y_HL			 = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_ACT_BLD_BASE_Y_HL_L_L, ANR_ACT_BLD_BASE_Y_HL_L_U, ANR_ACT_BLD_BASE_Y_HL_U_L, ANR_ACT_BLD_BASE_Y_HL_U_U,0);
		ANR_ACT_SLANT_Y_HL			  = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_ACT_SLANT_Y_HL_L_L, ANR_ACT_SLANT_Y_HL_L_U, ANR_ACT_SLANT_Y_HL_U_L, ANR_ACT_SLANT_Y_HL_U_U,0);
		ANR_ACT_BLD_TH_Y_HL 		   = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_ACT_BLD_TH_Y_HL_L_L, ANR_ACT_BLD_TH_Y_HL_L_U, ANR_ACT_BLD_TH_Y_HL_U_L, ANR_ACT_BLD_TH_Y_HL_U_U,0);
		ANR_ACT_TH_Y_HH 		   = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_ACT_TH_Y_HH_L_L, ANR_ACT_TH_Y_HH_L_U, ANR_ACT_TH_Y_HH_U_L, ANR_ACT_TH_Y_HH_U_U,0);
		ANR_ACT_BLD_BASE_Y_HH			 = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_ACT_BLD_BASE_Y_HH_L_L, ANR_ACT_BLD_BASE_Y_HH_L_U, ANR_ACT_BLD_BASE_Y_HH_U_L, ANR_ACT_BLD_BASE_Y_HH_U_U,0);
		ANR_ACT_SLANT_Y_HH			  = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_ACT_SLANT_Y_HH_L_L, ANR_ACT_SLANT_Y_HH_L_U, ANR_ACT_SLANT_Y_HH_U_L, ANR_ACT_SLANT_Y_HH_U_U,0);
		ANR_ACT_BLD_TH_Y_HH 		   = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_ACT_BLD_TH_Y_HH_L_L, ANR_ACT_BLD_TH_Y_HH_L_U, ANR_ACT_BLD_TH_Y_HH_U_L, ANR_ACT_BLD_TH_Y_HH_U_U,0);
		ANR_LUMA_TH_Y_L 		   = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_LUMA_TH_Y_L_L_L, ANR_LUMA_TH_Y_L_L_U, ANR_LUMA_TH_Y_L_U_L, ANR_LUMA_TH_Y_L_U_U,0);
		ANR_LUMA_BLD_BASE_Y_L			 = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_LUMA_BLD_BASE_Y_L_L_L, ANR_LUMA_BLD_BASE_Y_L_L_U, ANR_LUMA_BLD_BASE_Y_L_U_L, ANR_LUMA_BLD_BASE_Y_L_U_U,0);
		ANR_LUMA_SLANT_Y_L			  = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_LUMA_SLANT_Y_L_L_L, ANR_LUMA_SLANT_Y_L_L_U, ANR_LUMA_SLANT_Y_L_U_L, ANR_LUMA_SLANT_Y_L_U_U,0);
		ANR_LUMA_BLD_TH_Y_L 		   = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_LUMA_BLD_TH_Y_L_L_L, ANR_LUMA_BLD_TH_Y_L_L_U, ANR_LUMA_BLD_TH_Y_L_U_L, ANR_LUMA_BLD_TH_Y_L_U_U,0);
		ANR_LUMA_TH_Y_HL			= InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_LUMA_TH_Y_HL_L_L, ANR_LUMA_TH_Y_HL_L_U, ANR_LUMA_TH_Y_HL_U_L, ANR_LUMA_TH_Y_HL_U_U,0);
		ANR_LUMA_BLD_BASE_Y_HL			  = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_LUMA_BLD_BASE_Y_HL_L_L, ANR_LUMA_BLD_BASE_Y_HL_L_U, ANR_LUMA_BLD_BASE_Y_HL_U_L, ANR_LUMA_BLD_BASE_Y_HL_U_U,0);
		ANR_LUMA_SLANT_Y_HL 		   = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_LUMA_SLANT_Y_HL_L_L, ANR_LUMA_SLANT_Y_HL_L_U, ANR_LUMA_SLANT_Y_HL_U_L, ANR_LUMA_SLANT_Y_HL_U_U,0);
		ANR_LUMA_BLD_TH_Y_HL			= InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_LUMA_BLD_TH_Y_HL_L_L, ANR_LUMA_BLD_TH_Y_HL_L_U, ANR_LUMA_BLD_TH_Y_HL_U_L, ANR_LUMA_BLD_TH_Y_HL_U_U,0);
		ANR_LUMA_TH_Y_HH			= InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_LUMA_TH_Y_HH_L_L, ANR_LUMA_TH_Y_HH_L_U, ANR_LUMA_TH_Y_HH_U_L, ANR_LUMA_TH_Y_HH_U_U,0);
		ANR_LUMA_BLD_BASE_Y_HH			  = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_LUMA_BLD_BASE_Y_HH_L_L, ANR_LUMA_BLD_BASE_Y_HH_L_U, ANR_LUMA_BLD_BASE_Y_HH_U_L, ANR_LUMA_BLD_BASE_Y_HH_U_U,0);
		ANR_LUMA_SLANT_Y_HH 		   = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_LUMA_SLANT_Y_HH_L_L, ANR_LUMA_SLANT_Y_HH_L_U, ANR_LUMA_SLANT_Y_HH_U_L, ANR_LUMA_SLANT_Y_HH_U_U,0);
		ANR_LUMA_BLD_TH_Y_HH			= InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_LUMA_BLD_TH_Y_HH_L_L, ANR_LUMA_BLD_TH_Y_HH_L_U, ANR_LUMA_BLD_TH_Y_HH_U_L, ANR_LUMA_BLD_TH_Y_HH_U_U,0);
		ANR_PTY_L_GAIN			  = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_PTY_L_GAIN_L_L, ANR_PTY_L_GAIN_L_U, ANR_PTY_L_GAIN_U_L, ANR_PTY_L_GAIN_U_U,0);
		ANR_PTY_H_GAIN			  = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_PTY_H_GAIN_L_L, ANR_PTY_H_GAIN_L_U, ANR_PTY_H_GAIN_U_L, ANR_PTY_H_GAIN_U_U,0);
		ANR_BLEND_TH_L			  = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_BLEND_TH_L_L_L, ANR_BLEND_TH_L_L_U, ANR_BLEND_TH_L_U_L, ANR_BLEND_TH_L_U_U,0);
		ANR_BLEND_TH_H			  = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_BLEND_TH_H_L_L, ANR_BLEND_TH_H_L_U, ANR_BLEND_TH_H_U_L, ANR_BLEND_TH_H_U_U,0);
		ANR_HI_LMT1 		   = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_HI_LMT1_L_L, ANR_HI_LMT1_L_U, ANR_HI_LMT1_U_L, ANR_HI_LMT1_U_U,0);
		ANR_HI_LMT2 		   = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_HI_LMT2_L_L, ANR_HI_LMT2_L_U, ANR_HI_LMT2_U_L, ANR_HI_LMT2_U_U,0);
		ANR_LO_LMT1 		   = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_LO_LMT1_L_L, ANR_LO_LMT1_L_U, ANR_LO_LMT1_U_L, ANR_LO_LMT1_U_U,0);
		ANR_LO_LMT2 		   = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_LO_LMT2_L_L, ANR_LO_LMT2_L_U, ANR_LO_LMT2_U_L, ANR_LO_LMT2_U_U,0);
		ANR_Y_ACT_CEN_OFT			 = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_Y_ACT_CEN_OFT_L_L, ANR_Y_ACT_CEN_OFT_L_U, ANR_Y_ACT_CEN_OFT_U_L, ANR_Y_ACT_CEN_OFT_U_U,0);
		ANR_Y_ACT_CEN_GAIN			  = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_Y_ACT_CEN_GAIN_L_L, ANR_Y_ACT_CEN_GAIN_L_U, ANR_Y_ACT_CEN_GAIN_U_L, ANR_Y_ACT_CEN_GAIN_U_U,0);
		ANR_Y_ACT_CEN_TH			= InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_Y_ACT_CEN_TH_L_L, ANR_Y_ACT_CEN_TH_L_U, ANR_Y_ACT_CEN_TH_U_L, ANR_Y_ACT_CEN_TH_U_U,0);

		//Whitney E2
		ANR_TBL_CPX1			  = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_TBL_CPX1_L_L, ANR_TBL_CPX1_L_U, ANR_TBL_CPX1_U_L, ANR_TBL_CPX1_U_U,0);
		ANR_TBL_CPX2			  = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_TBL_CPX2_L_L, ANR_TBL_CPX2_L_U, ANR_TBL_CPX2_U_L, ANR_TBL_CPX2_U_U,0);
		ANR_TBL_CPX3			  = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_TBL_CPX3_L_L, ANR_TBL_CPX3_L_U, ANR_TBL_CPX3_U_L, ANR_TBL_CPX3_U_U,0);
		ANR_TBL_GAIN_CPY0			= InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_TBL_GAIN_CPY0_L_L, ANR_TBL_GAIN_CPY0_L_U, ANR_TBL_GAIN_CPY0_U_L, ANR_TBL_GAIN_CPY0_U_U,0);
		ANR_TBL_GAIN_CPY1			= InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_TBL_GAIN_CPY1_L_L, ANR_TBL_GAIN_CPY1_L_U, ANR_TBL_GAIN_CPY1_U_L, ANR_TBL_GAIN_CPY1_U_U,0);
		ANR_TBL_GAIN_CPY2			= InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_TBL_GAIN_CPY2_L_L, ANR_TBL_GAIN_CPY2_L_U, ANR_TBL_GAIN_CPY2_U_L, ANR_TBL_GAIN_CPY2_U_U,0);
		ANR_TBL_GAIN_CPY3			= InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_TBL_GAIN_CPY3_L_L, ANR_TBL_GAIN_CPY3_L_U, ANR_TBL_GAIN_CPY3_U_L, ANR_TBL_GAIN_CPY3_U_U,0);



		// ANR_SCALE_MODE - bit 1 for y, bit 0 for c
		MINT32 scale_mode_y_L_L  = (ANR_SCALE_MODE_L_L	>> 1);
		MINT32 scale_mode_c_L_L  = (ANR_SCALE_MODE_L_L	& 1);
		MINT32 scale_mode_y_L_U  = (ANR_SCALE_MODE_L_U	>> 1);
		MINT32 scale_mode_c_L_U  = (ANR_SCALE_MODE_L_U	& 1);
		MINT32 scale_mode_y_U_L  = (ANR_SCALE_MODE_U_L	>> 1);
		MINT32 scale_mode_c_U_L  = (ANR_SCALE_MODE_U_L	& 1);
		MINT32 scale_mode_y_U_U  = (ANR_SCALE_MODE_U_U	>> 1);
		MINT32 scale_mode_c_U_U  = (ANR_SCALE_MODE_U_U	& 1);



		MINT32 scale_mode_y, scale_mode_c;

		scale_mode_y			= InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, scale_mode_y_L_L, scale_mode_y_L_U, scale_mode_y_U_L, scale_mode_y_U_U,1);
		scale_mode_c			= InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, scale_mode_c_L_L, scale_mode_c_L_U, scale_mode_c_U_L, scale_mode_c_U_U,1);

		ANR_SCALE_MODE = (scale_mode_y << 1) + scale_mode_c;

		ANR_K_LO_TH_L_L = ANR_K_LO_TH_L_L * (scale_mode_y_L_L + 1);
		ANR_K_LO_TH_L_U = ANR_K_LO_TH_L_U * (scale_mode_y_L_U + 1);
		ANR_K_LO_TH_U_L = ANR_K_LO_TH_U_L * (scale_mode_y_U_L + 1);
		ANR_K_LO_TH_U_U = ANR_K_LO_TH_U_U * (scale_mode_y_U_U + 1);

		ANR_K_HI_TH_L_L = ANR_K_HI_TH_L_L * (scale_mode_y_L_L + 1);
		ANR_K_HI_TH_L_U = ANR_K_HI_TH_L_U * (scale_mode_y_L_U + 1);
		ANR_K_HI_TH_U_L = ANR_K_HI_TH_U_L * (scale_mode_y_U_L + 1);
		ANR_K_HI_TH_U_U = ANR_K_HI_TH_U_U * (scale_mode_y_U_U + 1);


		ANR_K_LO_TH = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_K_LO_TH_L_L, ANR_K_LO_TH_L_U, ANR_K_LO_TH_U_L, ANR_K_LO_TH_U_U,0);
		ANR_K_HI_TH = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_K_HI_TH_L_L, ANR_K_HI_TH_L_U, ANR_K_HI_TH_U_L, ANR_K_HI_TH_U_U,0);

		ANR_K_LO_TH =  ANR_K_LO_TH / (scale_mode_y + 1);
		ANR_K_HI_TH =  ANR_K_HI_TH / (scale_mode_y + 1);

		ANR_K_LO_TH = (ANR_K_LO_TH > 9) ? 9 : ANR_K_LO_TH;
		ANR_K_HI_TH = (ANR_K_HI_TH > 9) ? 9 : ANR_K_HI_TH;

		ANR_K_TH_C_L_L = ANR_K_TH_C_L_L * (scale_mode_c_L_L + 1);
		ANR_K_TH_C_L_U = ANR_K_TH_C_L_U * (scale_mode_c_L_U + 1);
		ANR_K_TH_C_U_L = ANR_K_TH_C_U_L * (scale_mode_c_U_L + 1);
		ANR_K_TH_C_U_U = ANR_K_TH_C_U_U * (scale_mode_c_U_U + 1);

		ANR_K_TH_C = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_K_TH_C_L_L, ANR_K_TH_C_L_U, ANR_K_TH_C_U_L, ANR_K_TH_C_U_U,0);
		ANR_K_TH_C =  ANR_K_TH_C / (scale_mode_c + 1);
		ANR_K_TH_C = (ANR_K_TH_C > 8) ? 8 : ANR_K_TH_C;




		// ANR_LCE_LINK

		ANR_LCE_LINK			= InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_LCE_LINK_L_L, ANR_LCE_LINK_L_U, ANR_LCE_LINK_U_L, ANR_LCE_LINK_U_U,0);
		ANR_LCE_X1			  = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_LCE_X1_L_L, ANR_LCE_X1_L_U, ANR_LCE_X1_U_L, ANR_LCE_X1_U_U,0);
		ANR_LCE_X2			  = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_LCE_X2_L_L, ANR_LCE_X2_L_U, ANR_LCE_X2_U_L, ANR_LCE_X2_U_U,0);
		ANR_LCE_X3			  = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_LCE_X3_L_L, ANR_LCE_X3_L_U, ANR_LCE_X3_U_L, ANR_LCE_X3_U_U,0);
		ANR_LCE_GAIN0			 = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_LCE_GAIN0_L_L, ANR_LCE_GAIN0_L_U, ANR_LCE_GAIN0_U_L, ANR_LCE_GAIN0_U_U,0);
		ANR_LCE_GAIN1			 = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_LCE_GAIN1_L_L, ANR_LCE_GAIN1_L_U, ANR_LCE_GAIN1_U_L, ANR_LCE_GAIN1_U_U,0);
		ANR_LCE_GAIN2			 = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_LCE_GAIN2_L_L, ANR_LCE_GAIN2_L_U, ANR_LCE_GAIN2_U_L, ANR_LCE_GAIN2_U_U,0);
		ANR_LCE_GAIN3			 = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_LCE_GAIN3_L_L, ANR_LCE_GAIN3_L_U, ANR_LCE_GAIN3_U_L, ANR_LCE_GAIN3_U_U,0);
		ANR_LCE_C_GAIN			  = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_LCE_C_GAIN_L_L, ANR_LCE_C_GAIN_L_U, ANR_LCE_C_GAIN_U_L, ANR_LCE_C_GAIN_U_U,0);
		ANR_LCE_SCALE_GAIN			  = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_LCE_SCALE_GAIN_L_L, ANR_LCE_SCALE_GAIN_L_U, ANR_LCE_SCALE_GAIN_U_L, ANR_LCE_SCALE_GAIN_U_U,0);

		// SP Calculation
		ANR_Y_SCALE_SP0    = CalculateLUTSP(		 0, ANR_Y_CPX1, ANR_Y_SCALE_CPY0, ANR_Y_SCALE_CPY1, 128, 15, -15);
		ANR_Y_SCALE_SP1    = CalculateLUTSP(ANR_Y_CPX1, ANR_Y_CPX2, ANR_Y_SCALE_CPY1, ANR_Y_SCALE_CPY2, 128, 15, -15);
		ANR_Y_SCALE_SP2    = CalculateLUTSP(ANR_Y_CPX2, ANR_Y_CPX3, ANR_Y_SCALE_CPY2, ANR_Y_SCALE_CPY3, 128, 15, -15);
		ANR_Y_SCALE_SP3 		   = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_Y_SCALE_SP3_L_L, ANR_Y_SCALE_SP3_L_U, ANR_Y_SCALE_SP3_U_L, ANR_Y_SCALE_SP3_U_U,0);
		ANR_C_SCALE_SP0    = CalculateLUTSP(		 0, ANR_C_CPX1, ANR_C_SCALE_CPY0, ANR_C_SCALE_CPY1, 128, 15, -15);
		ANR_C_SCALE_SP1    = CalculateLUTSP(ANR_C_CPX1, ANR_C_CPX2, ANR_C_SCALE_CPY1, ANR_C_SCALE_CPY2, 128, 15, -15);
		ANR_C_SCALE_SP2    = CalculateLUTSP(ANR_C_CPX2, ANR_C_CPX3, ANR_C_SCALE_CPY2, ANR_C_SCALE_CPY3, 128, 15, -15);
		ANR_C_SCALE_SP3 		   = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_C_SCALE_SP3_L_L, ANR_C_SCALE_SP3_L_U, ANR_C_SCALE_SP3_U_L, ANR_C_SCALE_SP3_U_U,0);
		ANR_Y_ACT_SP0	   = CalculateLUTSP(		 0, ANR_Y_CPX1, ANR_Y_ACT_CPY0	, ANR_Y_ACT_CPY1  , 32, 31, -31);
		ANR_Y_ACT_SP1	   = CalculateLUTSP(ANR_Y_CPX1, ANR_Y_CPX2, ANR_Y_ACT_CPY1	, ANR_Y_ACT_CPY2  , 32, 31, -31);
		ANR_Y_ACT_SP2	   = CalculateLUTSP(ANR_Y_CPX2, ANR_Y_CPX3, ANR_Y_ACT_CPY2	, ANR_Y_ACT_CPY3  , 32, 31, -31);
		ANR_Y_ACT_SP3			 = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_Y_ACT_SP3_L_L, ANR_Y_ACT_SP3_L_U, ANR_Y_ACT_SP3_U_L, ANR_Y_ACT_SP3_U_U,0);
		ANR_LCE_SP0 	   = CalculateLUTSP(		 0, ANR_LCE_X1, ANR_LCE_GAIN0	, ANR_LCE_GAIN1   , 128, 31, -31);
		ANR_LCE_SP1 	   = CalculateLUTSP(ANR_LCE_X1, ANR_LCE_X2, ANR_LCE_GAIN1	, ANR_LCE_GAIN2   , 128, 31, -31);
		ANR_LCE_SP2 	   = CalculateLUTSP(ANR_LCE_X2, ANR_LCE_X3, ANR_LCE_GAIN2	, ANR_LCE_GAIN3   , 128, 31, -31);
		ANR_LCE_SP3 		   = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_LCE_SP3_L_L, ANR_LCE_SP3_L_U, ANR_LCE_SP3_U_L, ANR_LCE_SP3_U_U,0);
		//Whitney E2
		ANR_TBL_GAIN_SP0    = CalculateLUTSP(		 0, ANR_TBL_CPX1, ANR_TBL_GAIN_CPY0, ANR_TBL_GAIN_CPY1, 128, 15, -15);
		ANR_TBL_GAIN_SP1    = CalculateLUTSP(ANR_TBL_CPX1, ANR_TBL_CPX2, ANR_TBL_GAIN_CPY1, ANR_TBL_GAIN_CPY2, 128, 15, -15);
		ANR_TBL_GAIN_SP2    = CalculateLUTSP(ANR_TBL_CPX2, ANR_TBL_CPX3, ANR_TBL_GAIN_CPY2, ANR_TBL_GAIN_CPY3, 128, 15, -15);
		ANR_TBL_GAIN_SP3 		   = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_TBL_GAIN_SP3_L_L, ANR_TBL_GAIN_SP3_L_U, ANR_TBL_GAIN_SP3_U_L, ANR_TBL_GAIN_SP3_U_U,0);



		// Olympu	ANR_Y_DIR

		ANR_Y_DIR			 = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_Y_DIR_L_L, ANR_Y_DIR_L_U, ANR_Y_DIR_U_L, ANR_Y_DIR_U_U,1);
		ANR_Y_DIR_GAIN			  = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_Y_DIR_GAIN_L_L, ANR_Y_DIR_GAIN_L_U, ANR_Y_DIR_GAIN_U_L, ANR_Y_DIR_GAIN_U_U,0);
		ANR_Y_DIR_VER_W_TH			  = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_Y_DIR_VER_W_TH_L_L, ANR_Y_DIR_VER_W_TH_L_U, ANR_Y_DIR_VER_W_TH_U_L, ANR_Y_DIR_VER_W_TH_U_U,0);
		ANR_Y_DIR_VER_W_SL			  = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_Y_DIR_VER_W_SL_L_L, ANR_Y_DIR_VER_W_SL_L_U, ANR_Y_DIR_VER_W_SL_U_L, ANR_Y_DIR_VER_W_SL_U_U,0);
		ANR_Y_DIR_DIAG_W_TH 		   = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_Y_DIR_DIAG_W_TH_L_L, ANR_Y_DIR_DIAG_W_TH_L_U, ANR_Y_DIR_DIAG_W_TH_U_L, ANR_Y_DIR_DIAG_W_TH_U_U,0);
		ANR_Y_DIR_DIAG_W_SL 		   = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR_Y_DIR_DIAG_W_SL_L_L, ANR_Y_DIR_DIAG_W_SL_L_U, ANR_Y_DIR_DIAG_W_SL_U_L, ANR_Y_DIR_DIAG_W_SL_U_U,0);




	//	  ====================================================================================================================================
	//	  Negative Conversion
	//	  ====================================================================================================================================
	ANR_Y_SCALE_SP0 = (ANR_Y_SCALE_SP0 < 0) ? (ANR_Y_SCALE_SP0 + 32) : ANR_Y_SCALE_SP0;
	ANR_Y_SCALE_SP1 = (ANR_Y_SCALE_SP1 < 0) ? (ANR_Y_SCALE_SP1 + 32) : ANR_Y_SCALE_SP1;
	ANR_Y_SCALE_SP2 = (ANR_Y_SCALE_SP2 < 0) ? (ANR_Y_SCALE_SP2 + 32) : ANR_Y_SCALE_SP2;
	ANR_Y_SCALE_SP3 = (ANR_Y_SCALE_SP3 < 0) ? (ANR_Y_SCALE_SP3 + 32) : ANR_Y_SCALE_SP3;
	ANR_C_SCALE_SP0 = (ANR_C_SCALE_SP0 < 0) ? (ANR_C_SCALE_SP0 + 32) : ANR_C_SCALE_SP0;
	ANR_C_SCALE_SP1 = (ANR_C_SCALE_SP1 < 0) ? (ANR_C_SCALE_SP1 + 32) : ANR_C_SCALE_SP1;
	ANR_C_SCALE_SP2 = (ANR_C_SCALE_SP2 < 0) ? (ANR_C_SCALE_SP2 + 32) : ANR_C_SCALE_SP2;
	ANR_C_SCALE_SP3 = (ANR_C_SCALE_SP3 < 0) ? (ANR_C_SCALE_SP3 + 32) : ANR_C_SCALE_SP3;
	ANR_Y_ACT_SP0	= (ANR_Y_ACT_SP0 < 0) ? (ANR_Y_ACT_SP0 + 64) : ANR_Y_ACT_SP0;
	ANR_Y_ACT_SP1	= (ANR_Y_ACT_SP1 < 0) ? (ANR_Y_ACT_SP1 + 64) : ANR_Y_ACT_SP1;
	ANR_Y_ACT_SP2	= (ANR_Y_ACT_SP2 < 0) ? (ANR_Y_ACT_SP2 + 64) : ANR_Y_ACT_SP2;
	ANR_Y_ACT_SP3	= (ANR_Y_ACT_SP3 < 0) ? (ANR_Y_ACT_SP3 + 64) : ANR_Y_ACT_SP3;
	ANR_LCE_SP0 	= (ANR_LCE_SP0 < 0) ? (ANR_LCE_SP0 + 64) : ANR_LCE_SP0;
	ANR_LCE_SP1 	= (ANR_LCE_SP1 < 0) ? (ANR_LCE_SP1 + 64) : ANR_LCE_SP1;
	ANR_LCE_SP2 	= (ANR_LCE_SP2 < 0) ? (ANR_LCE_SP2 + 64) : ANR_LCE_SP2;
	ANR_LCE_SP3 	= (ANR_LCE_SP3 < 0) ? (ANR_LCE_SP3 + 64) : ANR_LCE_SP3;
	//Whitney E2
	ANR_TBL_GAIN_SP0 = (ANR_TBL_GAIN_SP0 < 0) ? (ANR_TBL_GAIN_SP0 + 32) : ANR_TBL_GAIN_SP0;
	ANR_TBL_GAIN_SP1 = (ANR_TBL_GAIN_SP1 < 0) ? (ANR_TBL_GAIN_SP1 + 32) : ANR_TBL_GAIN_SP1;
	ANR_TBL_GAIN_SP2 = (ANR_TBL_GAIN_SP2 < 0) ? (ANR_TBL_GAIN_SP2 + 32) : ANR_TBL_GAIN_SP2;
	ANR_TBL_GAIN_SP3 = (ANR_TBL_GAIN_SP3 < 0) ? (ANR_TBL_GAIN_SP3 + 32) : ANR_TBL_GAIN_SP3;


	//	  ====================================================================================================================================
	//	  Set Smooth ISO setting
	//	  ====================================================================================================================================
	// CAM_ANR_CON1 CAM+A20H
	rSmoothANR.con1.bits.ANR_ENC = ANR_ENC;
	rSmoothANR.con1.bits.ANR_ENY = ANR_ENY;
	rSmoothANR.con1.bits.ANR_SCALE_MODE = ANR_SCALE_MODE;
	rSmoothANR.con1.bits.ANR_LCE_LINK = ANR_LCE_LINK;
	rSmoothANR.con1.bits.ANR_TABLE_EN = ANR_TABLE_EN;
	rSmoothANR.con1.bits.ANR_TBL_PRC = ANR_TBL_PRC;

	// CAM_ANR_CON2 CAM+A24H
	rSmoothANR.con2.bits.ANR_IMPL_MODE = ANR_IMPL_MODE;
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
	rSmoothANR.yad2.bits.ANR_K_RAT = ANR_K_RAT;


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


	// CAM_ANR_ACT1 CAM+A64H
	rSmoothANR.act1.bits.ANR_ACT_LCE_GAIN = ANR_ACT_LCE_GAIN;
	rSmoothANR.act1.bits.ANR_ACT_DIF_GAIN = ANR_ACT_DIF_GAIN;
	rSmoothANR.act1.bits.ANR_ACT_DIF_LO_TH = ANR_ACT_DIF_LO_TH;

	// CAM_ANR_ACT2 CAM+A64H
	rSmoothANR.act2.bits.ANR_ACT_SIZE_GAIN = ANR_ACT_SIZE_GAIN;
	rSmoothANR.act2.bits.ANR_ACT_SIZE_LO_TH = ANR_ACT_SIZE_LO_TH;

	// CAM_ANR_ACTC CAM+A64H
	rSmoothANR.actc.bits.ANR_ACT_BLD_BASE_C = ANR_ACT_BLD_BASE_C;
	rSmoothANR.actc.bits.ANR_C_DITH_U = ANR_C_DITH_U;
	rSmoothANR.actc.bits.ANR_C_DITH_V = ANR_C_DITH_V;

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

	// Olympu
    rSmoothANR.con2.bits.ANR_C_SM_EDGE_TH           =  ANR_C_SM_EDGE_TH;
    rSmoothANR.ylad.bits.ANR_K_HPF                  =  ANR_K_HPF ;
    rSmoothANR.act2.bits.ANR_ACT_DIF_HI_TH          =  ANR_ACT_DIF_HI_TH;
    rSmoothANR.actyhl.bits.ANR_ACT_TH_Y_HL          =  ANR_ACT_TH_Y_HL;
    rSmoothANR.actyhl.bits.ANR_ACT_BLD_BASE_Y_HL    =  ANR_ACT_BLD_BASE_Y_HL;
    rSmoothANR.actyhl.bits.ANR_ACT_SLANT_Y_HL       =  ANR_ACT_SLANT_Y_HL;
    rSmoothANR.actyhl.bits.ANR_ACT_BLD_TH_Y_HL      =  ANR_ACT_BLD_TH_Y_HL;
    rSmoothANR.actyhh.bits.ANR_ACT_TH_Y_HH          =  ANR_ACT_TH_Y_HH;
    rSmoothANR.actyhh.bits.ANR_ACT_BLD_BASE_Y_HH    =  ANR_ACT_BLD_BASE_Y_HH;
    rSmoothANR.actyhh.bits.ANR_ACT_SLANT_Y_HH       =  ANR_ACT_SLANT_Y_HH;
    rSmoothANR.actyhh.bits.ANR_ACT_BLD_TH_Y_HH      =  ANR_ACT_BLD_TH_Y_HH;
    rSmoothANR.actyl2.bits.ANR_LUMA_TH_Y_L          =  ANR_LUMA_TH_Y_L;
    rSmoothANR.actyl2.bits.ANR_LUMA_BLD_BASE_Y_L    =  ANR_LUMA_BLD_BASE_Y_L;
    rSmoothANR.actyl2.bits.ANR_LUMA_SLANT_Y_L       =  ANR_LUMA_SLANT_Y_L;
    rSmoothANR.actyl2.bits.ANR_LUMA_BLD_TH_Y_L      =  ANR_LUMA_BLD_TH_Y_L;
    rSmoothANR.actyhl2.bits.ANR_LUMA_TH_Y_HL        =  ANR_LUMA_TH_Y_HL;
    rSmoothANR.actyhl2.bits.ANR_LUMA_BLD_BASE_Y_HL  =  ANR_LUMA_BLD_BASE_Y_HL;
    rSmoothANR.actyhl2.bits.ANR_LUMA_SLANT_Y_HL     =  ANR_LUMA_SLANT_Y_HL;
    rSmoothANR.actyhl2.bits.ANR_LUMA_BLD_TH_Y_HL    =  ANR_LUMA_BLD_TH_Y_HL;
    rSmoothANR.actyhh2.bits.ANR_LUMA_TH_Y_HH        =  ANR_LUMA_TH_Y_HH;
    rSmoothANR.actyhh2.bits.ANR_LUMA_BLD_BASE_Y_HH  =  ANR_LUMA_BLD_BASE_Y_HH;
    rSmoothANR.actyhh2.bits.ANR_LUMA_SLANT_Y_HH     =  ANR_LUMA_SLANT_Y_HH;
    rSmoothANR.actyhh2.bits.ANR_LUMA_BLD_TH_Y_HH    =  ANR_LUMA_BLD_TH_Y_HH;
    rSmoothANR.ylad2.bits.ANR_PTY_L_GAIN            =  ANR_PTY_L_GAIN;
    rSmoothANR.ylad2.bits.ANR_PTY_H_GAIN            =  ANR_PTY_H_GAIN;
    rSmoothANR.ylad2.bits.ANR_BLEND_TH_L            =  ANR_BLEND_TH_L;
    rSmoothANR.ylad2.bits.ANR_BLEND_TH_H            =  ANR_BLEND_TH_H;
    rSmoothANR.ylad3.bits.ANR_HI_LMT1               =  ANR_HI_LMT1;
    rSmoothANR.ylad3.bits.ANR_HI_LMT2               =  ANR_HI_LMT2;
    rSmoothANR.ylad3.bits.ANR_LO_LMT1               =  ANR_LO_LMT1;
    rSmoothANR.ylad3.bits.ANR_LO_LMT2               =  ANR_LO_LMT2;
    rSmoothANR.act4.bits.ANR_Y_ACT_CEN_OFT          =  ANR_Y_ACT_CEN_OFT;
    rSmoothANR.act4.bits.ANR_Y_ACT_CEN_GAIN         =  ANR_Y_ACT_CEN_GAIN;
    rSmoothANR.act4.bits.ANR_Y_ACT_CEN_TH           =  ANR_Y_ACT_CEN_TH;
    rSmoothANR.con1.bits.ANR_Y_DIR                  =  ANR_Y_DIR;
    rSmoothANR.ydir.bits.ANR_Y_DIR_GAIN             =  ANR_Y_DIR_GAIN;
    rSmoothANR.ydir.bits.ANR_Y_DIR_VER_W_TH         =  ANR_Y_DIR_VER_W_TH;
    rSmoothANR.ydir.bits.ANR_Y_DIR_VER_W_SL         =  ANR_Y_DIR_VER_W_SL;
    rSmoothANR.ydir.bits.ANR_Y_DIR_DIAG_W_TH        =  ANR_Y_DIR_DIAG_W_TH;
    rSmoothANR.ydir.bits.ANR_Y_DIR_DIAG_W_SL        =  ANR_Y_DIR_DIAG_W_SL;

	//Whitney E2
	rSmoothANR.t4lut1.bits.ANR_TBL_CPX1        =  ANR_TBL_CPX1;
	rSmoothANR.t4lut1.bits.ANR_TBL_CPX2        =  ANR_TBL_CPX2;
	rSmoothANR.t4lut1.bits.ANR_TBL_CPX3        =  ANR_TBL_CPX3;
	rSmoothANR.t4lut2.bits.ANR_TBL_GAIN_CPY0        =  ANR_TBL_GAIN_CPY0;
	rSmoothANR.t4lut2.bits.ANR_TBL_GAIN_CPY1        =  ANR_TBL_GAIN_CPY1;
	rSmoothANR.t4lut2.bits.ANR_TBL_GAIN_CPY2        =  ANR_TBL_GAIN_CPY2;
	rSmoothANR.t4lut2.bits.ANR_TBL_GAIN_CPY3        =  ANR_TBL_GAIN_CPY3;
	rSmoothANR.t4lut3.bits.ANR_TBL_GAIN_SP0        =  ANR_TBL_GAIN_SP0;
	rSmoothANR.t4lut3.bits.ANR_TBL_GAIN_SP1        =  ANR_TBL_GAIN_SP1;
	rSmoothANR.t4lut3.bits.ANR_TBL_GAIN_SP2        =  ANR_TBL_GAIN_SP2;
	rSmoothANR.t4lut3.bits.ANR_TBL_GAIN_SP3        =  ANR_TBL_GAIN_SP3;


#define ANR_DUMP(item1, item2)   \
		do{ 					 \
			INTER_LOG_IF(bSmoothANRDebug, "[S, L_L, L_U, U_L, U_U]%22s = %d, %d, %d, %d, %d", #item2, rSmoothANR.item1.bits.item2, rParam.rLowerIso_LowerZoom.item1.bits.item2, rParam.rLowerIso_UpperZoom.item1.bits.item2, rParam.rUpperIso_LowerZoom.item1.bits.item2, rParam.rUpperIso_UpperZoom.item1.bits.item2); \
		}while(0)

	ANR_DUMP(con1, ANR_ENC);
	ANR_DUMP(con1, ANR_ENY);
	ANR_DUMP(con1, ANR_SCALE_MODE);
	ANR_DUMP(con1, ANR_LCE_LINK);
	ANR_DUMP(con1, ANR_TABLE_EN);
	ANR_DUMP(con1, ANR_TBL_PRC);

	ANR_DUMP(con2, ANR_IMPL_MODE);
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
	ANR_DUMP(yad2, ANR_K_RAT);

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


	ANR_DUMP(act1, ANR_ACT_LCE_GAIN);
	ANR_DUMP(act1, ANR_ACT_DIF_GAIN);
	ANR_DUMP(act1, ANR_ACT_DIF_LO_TH);

	ANR_DUMP(act2, ANR_ACT_SIZE_GAIN);
	ANR_DUMP(act2, ANR_ACT_SIZE_LO_TH);

	ANR_DUMP(actc, ANR_ACT_BLD_BASE_C);
	ANR_DUMP(actc, ANR_C_DITH_U);
	ANR_DUMP(actc, ANR_C_DITH_V);

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

	// Olympu
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

	//Whitney E2
	ANR_DUMP(t4lut1, ANR_TBL_CPX1);
	ANR_DUMP(t4lut1, ANR_TBL_CPX2);
	ANR_DUMP(t4lut1, ANR_TBL_CPX3);

	ANR_DUMP(t4lut2, ANR_TBL_GAIN_CPY0);
	ANR_DUMP(t4lut2, ANR_TBL_GAIN_CPY1);
	ANR_DUMP(t4lut2, ANR_TBL_GAIN_CPY2);
	ANR_DUMP(t4lut2, ANR_TBL_GAIN_CPY3);

	ANR_DUMP(t4lut3, ANR_TBL_GAIN_SP0);
	ANR_DUMP(t4lut3, ANR_TBL_GAIN_SP1);
	ANR_DUMP(t4lut3, ANR_TBL_GAIN_SP2);
	ANR_DUMP(t4lut3, ANR_TBL_GAIN_SP3);


#undef ANR_DUMP

}

MVOID SmoothANR2(ISP_NVRAM_ANR2_INT_T const& rParam,   // ANR2 settings
                ISP_NVRAM_ANR2_T& rSmoothANR2)
{
    MBOOL bSmoothANRDebug = interpPropertyGet("debug.smooth_anr.enable", MFALSE);

	MINT32 u4RealISO = rParam.u4RealISO;
	MINT32 u4UpperISO = rParam.u4UpperISO;
	MINT32 u4LowerISO = rParam.u4LowerISO;
	MINT32 fRealZoom = rParam.i4ZoomRatio_x100;
	MINT32 fUpperZoom = rParam.u4UpperZoom;
	MINT32 fLowerZoom = rParam.u4LowerZoom;


	INTER_LOG_IF(bSmoothANRDebug,"[%s()] u4RealISO: %d, u4UpperISO = %d, u4LowerISO = %d\n", __FUNCTION__, u4RealISO, u4UpperISO, u4LowerISO);
	INTER_LOG_IF(bSmoothANRDebug,"[%s()] u4RealZoom: %d, u4UpperZoom = %d, u4LowerZoom = %d\n", __FUNCTION__, fRealZoom, fUpperZoom, fLowerZoom);

	// _L_L = Low ISO Low Zoom, _L_U = Low ISO Up Zoom, _U_L = Up ISO Low Zoom, _U_U = Up ISO Up Zoom

	double RATIO_ISO_lo = 0;
	double RATIO_ISO_up = 0;
	double RATIO_Zoom_lo = 0;
	double RATIO_Zoom_up = 0;

    //protection for out-of-bound


    if (u4RealISO <= u4LowerISO)
    {
        RATIO_ISO_lo = 1;
    }
	else if(u4RealISO >= u4UpperISO)
	{
		RATIO_ISO_lo = 0;
	}
	else
	{
		RATIO_ISO_lo = (double)(u4UpperISO - u4RealISO) / (double)(u4UpperISO - u4LowerISO);
	}

	RATIO_ISO_up = 1.0 - RATIO_ISO_lo;

	if (fRealZoom <= fLowerZoom)
    {
        RATIO_Zoom_lo = 1;
    }
	else if(fRealZoom >= fUpperZoom)
	{
		RATIO_Zoom_lo = 0;
	}
	else
	{
		RATIO_Zoom_lo = (double)(fUpperZoom - fRealZoom) / (double)(fUpperZoom - fLowerZoom);
	}

	RATIO_Zoom_up = 1.0 - RATIO_Zoom_lo;

	double RATIO_L_L = (double)(RATIO_ISO_lo * RATIO_Zoom_lo);
	double RATIO_L_U = (double)(RATIO_ISO_lo * RATIO_Zoom_up);
	double RATIO_U_L = (double)(RATIO_ISO_up * RATIO_Zoom_lo);
	double RATIO_U_U = (double)(RATIO_ISO_up * RATIO_Zoom_up);

    //    ====================================================================================================================================
    //    Get Low ISO Low Zoom
    //    ====================================================================================================================================
    // CAM_ANR2_CON1 CAM+A20H
    MINT32 ANR2_ENC_L_L           = rParam.rLowerIso_LowerZoom.con1.bits.ANR2_ENC;
    MINT32 ANR2_ENY_L_L           = rParam.rLowerIso_LowerZoom.con1.bits.ANR2_ENY;
    MINT32 ANR2_SCALE_MODE_L_L    = rParam.rLowerIso_LowerZoom.con1.bits.ANR2_SCALE_MODE;
    MINT32 ANR2_MODE_L_L          = rParam.rLowerIso_LowerZoom.con1.bits.ANR2_MODE;
    MINT32 ANR2_LCE_LINK_L_L      = rParam.rLowerIso_LowerZoom.con1.bits.ANR2_LCE_LINK;

    // CAM_ANR2_CON2 CAM+A24H
    MINT32 ANR2_C_SM_EDGE_L_L     = rParam.rLowerIso_LowerZoom.con2.bits.ANR2_C_SM_EDGE;
    MINT32 ANR2_FLT_C_L_L         = rParam.rLowerIso_LowerZoom.con2.bits.ANR2_FLT_C;

    // CAM_ANR2_YAD1 CAM+A2CH
    MINT32 ANR2_K_TH_C_L_L        = rParam.rLowerIso_LowerZoom.yad1.bits.ANR2_K_TH_C;

    // CAM_ANR2_Y4LUT1 CAM+A34H
    MINT32 ANR2_Y_CPX1_L_L        = rParam.rLowerIso_LowerZoom.y4lut1.bits.ANR2_Y_CPX1;
    MINT32 ANR2_Y_CPX2_L_L        = rParam.rLowerIso_LowerZoom.y4lut1.bits.ANR2_Y_CPX2;
    MINT32 ANR2_Y_CPX3_L_L        = rParam.rLowerIso_LowerZoom.y4lut1.bits.ANR2_Y_CPX3;

    // CAM_ANR2_Y4LUT2 CAM+A38H
    MINT32 ANR2_Y_SCALE_CPY0_L_L  = rParam.rLowerIso_LowerZoom.y4lut2.bits.ANR2_Y_SCALE_CPY0;
    MINT32 ANR2_Y_SCALE_CPY1_L_L  = rParam.rLowerIso_LowerZoom.y4lut2.bits.ANR2_Y_SCALE_CPY1;
    MINT32 ANR2_Y_SCALE_CPY2_L_L  = rParam.rLowerIso_LowerZoom.y4lut2.bits.ANR2_Y_SCALE_CPY2;
    MINT32 ANR2_Y_SCALE_CPY3_L_L  = rParam.rLowerIso_LowerZoom.y4lut2.bits.ANR2_Y_SCALE_CPY3;

    // CAM_ANR2_Y4LUT3 CAM+A3CH
    MINT32 ANR2_Y_SCALE_SP0_L_L   = rParam.rLowerIso_LowerZoom.y4lut3.bits.ANR2_Y_SCALE_SP0;
    MINT32 ANR2_Y_SCALE_SP1_L_L   = rParam.rLowerIso_LowerZoom.y4lut3.bits.ANR2_Y_SCALE_SP1;
    MINT32 ANR2_Y_SCALE_SP2_L_L   = rParam.rLowerIso_LowerZoom.y4lut3.bits.ANR2_Y_SCALE_SP2;
    MINT32 ANR2_Y_SCALE_SP3_L_L   = rParam.rLowerIso_LowerZoom.y4lut3.bits.ANR2_Y_SCALE_SP3;

    // CAM_ANR2_L4LUT1
    MINT32 ANR2_LCE_X1_L_L        = rParam.rLowerIso_LowerZoom.l4lut1.bits.ANR2_LCE_X1;
    MINT32 ANR2_LCE_X2_L_L        = rParam.rLowerIso_LowerZoom.l4lut1.bits.ANR2_LCE_X2;
    MINT32 ANR2_LCE_X3_L_L        = rParam.rLowerIso_LowerZoom.l4lut1.bits.ANR2_LCE_X3;

    // CAM_ANR2_L4LUT2
    MINT32 ANR2_LCE_GAIN0_L_L     = rParam.rLowerIso_LowerZoom.l4lut2.bits.ANR2_LCE_GAIN0;
    MINT32 ANR2_LCE_GAIN1_L_L     = rParam.rLowerIso_LowerZoom.l4lut2.bits.ANR2_LCE_GAIN1;
    MINT32 ANR2_LCE_GAIN2_L_L     = rParam.rLowerIso_LowerZoom.l4lut2.bits.ANR2_LCE_GAIN2;
    MINT32 ANR2_LCE_GAIN3_L_L     = rParam.rLowerIso_LowerZoom.l4lut2.bits.ANR2_LCE_GAIN3;

    // CAM_ANR2_L4LUT3
    MINT32 ANR2_LCE_SP0_L_L       = rParam.rLowerIso_LowerZoom.l4lut3.bits.ANR2_LCE_SP0;
    MINT32 ANR2_LCE_SP1_L_L       = rParam.rLowerIso_LowerZoom.l4lut3.bits.ANR2_LCE_SP1;
    MINT32 ANR2_LCE_SP2_L_L       = rParam.rLowerIso_LowerZoom.l4lut3.bits.ANR2_LCE_SP2;
    MINT32 ANR2_LCE_SP3_L_L       = rParam.rLowerIso_LowerZoom.l4lut3.bits.ANR2_LCE_SP3;

    // CAM_ANR2_CAD CAM+A44H
    MINT32 ANR2_PTC_VGAIN_L_L     = rParam.rLowerIso_LowerZoom.cad.bits.ANR2_PTC_VGAIN;
    MINT32 ANR2_C_GAIN_L_L        = rParam.rLowerIso_LowerZoom.cad.bits.ANR2_C_GAIN;
    MINT32 ANR2_PTC_GAIN_TH_L_L   = rParam.rLowerIso_LowerZoom.cad.bits.ANR2_PTC_GAIN_TH;
    MINT32 ANR2_C_L_DIFF_TH_L_L   = rParam.rLowerIso_LowerZoom.cad.bits.ANR2_C_L_DIFF_TH;
    MINT32 ANR2_C_MODE_L_L        = rParam.rLowerIso_LowerZoom.cad.bits.ANR2_C_MODE;

    // CAM_ANR2_PTC CAM+A48H
    MINT32 ANR2_PTC1_L_L          = rParam.rLowerIso_LowerZoom.ptc.bits.ANR2_PTC1;
    MINT32 ANR2_PTC2_L_L          = rParam.rLowerIso_LowerZoom.ptc.bits.ANR2_PTC2;
    MINT32 ANR2_PTC3_L_L          = rParam.rLowerIso_LowerZoom.ptc.bits.ANR2_PTC3;
    MINT32 ANR2_PTC4_L_L          = rParam.rLowerIso_LowerZoom.ptc.bits.ANR2_PTC4;

    // CAM_ANR2_LCE CAM+A4CH
    MINT32 ANR2_LCE_C_GAIN_L_L    = rParam.rLowerIso_LowerZoom.lce.bits.ANR2_LCE_C_GAIN;
    MINT32 ANR2_LM_WT_L_L         = rParam.rLowerIso_LowerZoom.lce.bits.ANR2_LM_WT;

    // CAM_ANR2_MED1
    MINT32 ANR2_COR_TH_L_L        = rParam.rLowerIso_LowerZoom.med1.bits.ANR2_COR_TH;
    MINT32 ANR2_COR_SL_L_L        = rParam.rLowerIso_LowerZoom.med1.bits.ANR2_COR_SL;
    MINT32 ANR2_MCD_TH_L_L        = rParam.rLowerIso_LowerZoom.med1.bits.ANR2_MCD_TH;
    MINT32 ANR2_MCD_SL_L_L        = rParam.rLowerIso_LowerZoom.med1.bits.ANR2_MCD_SL;
    MINT32 ANR2_LCL_TH_L_L        = rParam.rLowerIso_LowerZoom.med1.bits.ANR2_LCL_TH;

    // CAM_ANR2_MED2
    MINT32 ANR2_LCL_SL_L_L        = rParam.rLowerIso_LowerZoom.med2.bits.ANR2_LCL_SL;
    MINT32 ANR2_LCL_LV_L_L        = rParam.rLowerIso_LowerZoom.med2.bits.ANR2_LCL_LV;
    MINT32 ANR2_SCL_TH_L_L        = rParam.rLowerIso_LowerZoom.med2.bits.ANR2_SCL_TH;
    MINT32 ANR2_SCL_SL_L_L        = rParam.rLowerIso_LowerZoom.med2.bits.ANR2_SCL_SL;
    MINT32 ANR2_SCL_LV_L_L        = rParam.rLowerIso_LowerZoom.med2.bits.ANR2_SCL_LV;

    // CAM_ANR2_MED3
    MINT32 ANR2_NCL_TH_L_L        = rParam.rLowerIso_LowerZoom.med3.bits.ANR2_NCL_TH;
    MINT32 ANR2_NCL_SL_L_L        = rParam.rLowerIso_LowerZoom.med3.bits.ANR2_NCL_SL;
    MINT32 ANR2_NCL_LV_L_L        = rParam.rLowerIso_LowerZoom.med3.bits.ANR2_NCL_LV;
    MINT32 ANR2_VAR_L_L           = rParam.rLowerIso_LowerZoom.med3.bits.ANR2_VAR;
    MINT32 ANR2_Y0_L_L            = rParam.rLowerIso_LowerZoom.med3.bits.ANR2_Y0;

    // CAM_ANR2_MED4
    MINT32 ANR2_Y1_L_L            = rParam.rLowerIso_LowerZoom.med4.bits.ANR2_Y1;
    MINT32 ANR2_Y2_L_L            = rParam.rLowerIso_LowerZoom.med4.bits.ANR2_Y2;
    MINT32 ANR2_Y3_L_L            = rParam.rLowerIso_LowerZoom.med4.bits.ANR2_Y3;
    MINT32 ANR2_Y4_L_L            = rParam.rLowerIso_LowerZoom.med4.bits.ANR2_Y4;

	// CAM_ANR_MED5
	MINT32 ANR2_LCL_OFT_L_L			 = rParam.rLowerIso_LowerZoom.med5.bits.ANR2_LCL_OFT;
	MINT32 ANR2_SCL_OFT_L_L			 = rParam.rLowerIso_LowerZoom.med5.bits.ANR2_SCL_OFT;
	MINT32 ANR2_NCL_OFT_L_L			 = rParam.rLowerIso_LowerZoom.med5.bits.ANR2_NCL_OFT;

    // CAM_ANR2_ACTC CAM+A64H
    MINT32 ANR2_ACT_BLD_BASE_C_L_L= rParam.rLowerIso_LowerZoom.actc.bits.ANR2_ACT_BLD_BASE_C;
	MINT32 ANR2_C_DITH_U_L_L= rParam.rLowerIso_LowerZoom.actc.bits.ANR2_C_DITH_U;
	MINT32 ANR2_C_DITH_V_L_L= rParam.rLowerIso_LowerZoom.actc.bits.ANR2_C_DITH_V;

	//    ====================================================================================================================================
    //    Get Low ISO Up Zoom
    //    ====================================================================================================================================
    // CAM_ANR2_CON1 CAM+A20H
    MINT32 ANR2_ENC_L_U           = rParam.rLowerIso_UpperZoom.con1.bits.ANR2_ENC;
    MINT32 ANR2_ENY_L_U           = rParam.rLowerIso_UpperZoom.con1.bits.ANR2_ENY;
    MINT32 ANR2_SCALE_MODE_L_U    = rParam.rLowerIso_UpperZoom.con1.bits.ANR2_SCALE_MODE;
    MINT32 ANR2_MODE_L_U          = rParam.rLowerIso_UpperZoom.con1.bits.ANR2_MODE;
    MINT32 ANR2_LCE_LINK_L_U      = rParam.rLowerIso_UpperZoom.con1.bits.ANR2_LCE_LINK;

    // CAM_ANR2_CON2 CAM+A24H
    MINT32 ANR2_C_SM_EDGE_L_U     = rParam.rLowerIso_UpperZoom.con2.bits.ANR2_C_SM_EDGE;
    MINT32 ANR2_FLT_C_L_U         = rParam.rLowerIso_UpperZoom.con2.bits.ANR2_FLT_C;

    // CAM_ANR2_YAD1 CAM+A2CH
    MINT32 ANR2_K_TH_C_L_U        = rParam.rLowerIso_UpperZoom.yad1.bits.ANR2_K_TH_C;

    // CAM_ANR2_Y4LUT1 CAM+A34H
    MINT32 ANR2_Y_CPX1_L_U        = rParam.rLowerIso_UpperZoom.y4lut1.bits.ANR2_Y_CPX1;
    MINT32 ANR2_Y_CPX2_L_U        = rParam.rLowerIso_UpperZoom.y4lut1.bits.ANR2_Y_CPX2;
    MINT32 ANR2_Y_CPX3_L_U        = rParam.rLowerIso_UpperZoom.y4lut1.bits.ANR2_Y_CPX3;

    // CAM_ANR2_Y4LUT2 CAM+A38H
    MINT32 ANR2_Y_SCALE_CPY0_L_U  = rParam.rLowerIso_UpperZoom.y4lut2.bits.ANR2_Y_SCALE_CPY0;
    MINT32 ANR2_Y_SCALE_CPY1_L_U  = rParam.rLowerIso_UpperZoom.y4lut2.bits.ANR2_Y_SCALE_CPY1;
    MINT32 ANR2_Y_SCALE_CPY2_L_U  = rParam.rLowerIso_UpperZoom.y4lut2.bits.ANR2_Y_SCALE_CPY2;
    MINT32 ANR2_Y_SCALE_CPY3_L_U  = rParam.rLowerIso_UpperZoom.y4lut2.bits.ANR2_Y_SCALE_CPY3;

    // CAM_ANR2_Y4LUT3 CAM+A3CH
    MINT32 ANR2_Y_SCALE_SP0_L_U   = rParam.rLowerIso_UpperZoom.y4lut3.bits.ANR2_Y_SCALE_SP0;
    MINT32 ANR2_Y_SCALE_SP1_L_U   = rParam.rLowerIso_UpperZoom.y4lut3.bits.ANR2_Y_SCALE_SP1;
    MINT32 ANR2_Y_SCALE_SP2_L_U   = rParam.rLowerIso_UpperZoom.y4lut3.bits.ANR2_Y_SCALE_SP2;
    MINT32 ANR2_Y_SCALE_SP3_L_U   = rParam.rLowerIso_UpperZoom.y4lut3.bits.ANR2_Y_SCALE_SP3;

    // CAM_ANR2_L4LUT1
    MINT32 ANR2_LCE_X1_L_U        = rParam.rLowerIso_UpperZoom.l4lut1.bits.ANR2_LCE_X1;
    MINT32 ANR2_LCE_X2_L_U        = rParam.rLowerIso_UpperZoom.l4lut1.bits.ANR2_LCE_X2;
    MINT32 ANR2_LCE_X3_L_U        = rParam.rLowerIso_UpperZoom.l4lut1.bits.ANR2_LCE_X3;

    // CAM_ANR2_L4LUT2
    MINT32 ANR2_LCE_GAIN0_L_U     = rParam.rLowerIso_UpperZoom.l4lut2.bits.ANR2_LCE_GAIN0;
    MINT32 ANR2_LCE_GAIN1_L_U     = rParam.rLowerIso_UpperZoom.l4lut2.bits.ANR2_LCE_GAIN1;
    MINT32 ANR2_LCE_GAIN2_L_U     = rParam.rLowerIso_UpperZoom.l4lut2.bits.ANR2_LCE_GAIN2;
    MINT32 ANR2_LCE_GAIN3_L_U     = rParam.rLowerIso_UpperZoom.l4lut2.bits.ANR2_LCE_GAIN3;

    // CAM_ANR2_L4LUT3
    MINT32 ANR2_LCE_SP0_L_U       = rParam.rLowerIso_UpperZoom.l4lut3.bits.ANR2_LCE_SP0;
    MINT32 ANR2_LCE_SP1_L_U       = rParam.rLowerIso_UpperZoom.l4lut3.bits.ANR2_LCE_SP1;
    MINT32 ANR2_LCE_SP2_L_U       = rParam.rLowerIso_UpperZoom.l4lut3.bits.ANR2_LCE_SP2;
    MINT32 ANR2_LCE_SP3_L_U       = rParam.rLowerIso_UpperZoom.l4lut3.bits.ANR2_LCE_SP3;

    // CAM_ANR2_CAD CAM+A44H
    MINT32 ANR2_PTC_VGAIN_L_U     = rParam.rLowerIso_UpperZoom.cad.bits.ANR2_PTC_VGAIN;
    MINT32 ANR2_C_GAIN_L_U        = rParam.rLowerIso_UpperZoom.cad.bits.ANR2_C_GAIN;
    MINT32 ANR2_PTC_GAIN_TH_L_U   = rParam.rLowerIso_UpperZoom.cad.bits.ANR2_PTC_GAIN_TH;
    MINT32 ANR2_C_L_DIFF_TH_L_U   = rParam.rLowerIso_UpperZoom.cad.bits.ANR2_C_L_DIFF_TH;
    MINT32 ANR2_C_MODE_L_U        = rParam.rLowerIso_UpperZoom.cad.bits.ANR2_C_MODE;

    // CAM_ANR2_PTC CAM+A48H
    MINT32 ANR2_PTC1_L_U          = rParam.rLowerIso_UpperZoom.ptc.bits.ANR2_PTC1;
    MINT32 ANR2_PTC2_L_U          = rParam.rLowerIso_UpperZoom.ptc.bits.ANR2_PTC2;
    MINT32 ANR2_PTC3_L_U          = rParam.rLowerIso_UpperZoom.ptc.bits.ANR2_PTC3;
    MINT32 ANR2_PTC4_L_U          = rParam.rLowerIso_UpperZoom.ptc.bits.ANR2_PTC4;

    // CAM_ANR2_LCE CAM+A4CH
    MINT32 ANR2_LCE_C_GAIN_L_U    = rParam.rLowerIso_UpperZoom.lce.bits.ANR2_LCE_C_GAIN;
    MINT32 ANR2_LM_WT_L_U         = rParam.rLowerIso_UpperZoom.lce.bits.ANR2_LM_WT;

    // CAM_ANR2_MED1
    MINT32 ANR2_COR_TH_L_U        = rParam.rLowerIso_UpperZoom.med1.bits.ANR2_COR_TH;
    MINT32 ANR2_COR_SL_L_U        = rParam.rLowerIso_UpperZoom.med1.bits.ANR2_COR_SL;
    MINT32 ANR2_MCD_TH_L_U        = rParam.rLowerIso_UpperZoom.med1.bits.ANR2_MCD_TH;
    MINT32 ANR2_MCD_SL_L_U        = rParam.rLowerIso_UpperZoom.med1.bits.ANR2_MCD_SL;
    MINT32 ANR2_LCL_TH_L_U        = rParam.rLowerIso_UpperZoom.med1.bits.ANR2_LCL_TH;

    // CAM_ANR2_MED2
    MINT32 ANR2_LCL_SL_L_U        = rParam.rLowerIso_UpperZoom.med2.bits.ANR2_LCL_SL;
    MINT32 ANR2_LCL_LV_L_U        = rParam.rLowerIso_UpperZoom.med2.bits.ANR2_LCL_LV;
    MINT32 ANR2_SCL_TH_L_U        = rParam.rLowerIso_UpperZoom.med2.bits.ANR2_SCL_TH;
    MINT32 ANR2_SCL_SL_L_U        = rParam.rLowerIso_UpperZoom.med2.bits.ANR2_SCL_SL;
    MINT32 ANR2_SCL_LV_L_U        = rParam.rLowerIso_UpperZoom.med2.bits.ANR2_SCL_LV;

    // CAM_ANR2_MED3
    MINT32 ANR2_NCL_TH_L_U        = rParam.rLowerIso_UpperZoom.med3.bits.ANR2_NCL_TH;
    MINT32 ANR2_NCL_SL_L_U        = rParam.rLowerIso_UpperZoom.med3.bits.ANR2_NCL_SL;
    MINT32 ANR2_NCL_LV_L_U        = rParam.rLowerIso_UpperZoom.med3.bits.ANR2_NCL_LV;
    MINT32 ANR2_VAR_L_U           = rParam.rLowerIso_UpperZoom.med3.bits.ANR2_VAR;
    MINT32 ANR2_Y0_L_U            = rParam.rLowerIso_UpperZoom.med3.bits.ANR2_Y0;

    // CAM_ANR2_MED4
    MINT32 ANR2_Y1_L_U            = rParam.rLowerIso_UpperZoom.med4.bits.ANR2_Y1;
    MINT32 ANR2_Y2_L_U            = rParam.rLowerIso_UpperZoom.med4.bits.ANR2_Y2;
    MINT32 ANR2_Y3_L_U            = rParam.rLowerIso_UpperZoom.med4.bits.ANR2_Y3;
    MINT32 ANR2_Y4_L_U            = rParam.rLowerIso_UpperZoom.med4.bits.ANR2_Y4;

	// CAM_ANR_MED5
	MINT32 ANR2_LCL_OFT_L_U			 = rParam.rLowerIso_UpperZoom.med5.bits.ANR2_LCL_OFT;
	MINT32 ANR2_SCL_OFT_L_U			 = rParam.rLowerIso_UpperZoom.med5.bits.ANR2_SCL_OFT;
	MINT32 ANR2_NCL_OFT_L_U			 = rParam.rLowerIso_UpperZoom.med5.bits.ANR2_NCL_OFT;

    // CAM_ANR2_ACTC CAM+A64H
    MINT32 ANR2_ACT_BLD_BASE_C_L_U= rParam.rLowerIso_UpperZoom.actc.bits.ANR2_ACT_BLD_BASE_C;
	MINT32 ANR2_C_DITH_U_L_U= rParam.rLowerIso_UpperZoom.actc.bits.ANR2_C_DITH_U;
	MINT32 ANR2_C_DITH_V_L_U= rParam.rLowerIso_UpperZoom.actc.bits.ANR2_C_DITH_V;

	//    ====================================================================================================================================
    //    Get Up ISO Low Zoom
    //    ====================================================================================================================================
    // CAM_ANR2_CON1 CAM+A20H
    MINT32 ANR2_ENC_U_L           = rParam.rUpperIso_LowerZoom.con1.bits.ANR2_ENC;
    MINT32 ANR2_ENY_U_L           = rParam.rUpperIso_LowerZoom.con1.bits.ANR2_ENY;
    MINT32 ANR2_SCALE_MODE_U_L    = rParam.rUpperIso_LowerZoom.con1.bits.ANR2_SCALE_MODE;
    MINT32 ANR2_MODE_U_L          = rParam.rUpperIso_LowerZoom.con1.bits.ANR2_MODE;
    MINT32 ANR2_LCE_LINK_U_L      = rParam.rUpperIso_LowerZoom.con1.bits.ANR2_LCE_LINK;

    // CAM_ANR2_CON2 CAM+A24H
    MINT32 ANR2_C_SM_EDGE_U_L     = rParam.rUpperIso_LowerZoom.con2.bits.ANR2_C_SM_EDGE;
    MINT32 ANR2_FLT_C_U_L         = rParam.rUpperIso_LowerZoom.con2.bits.ANR2_FLT_C;

    // CAM_ANR2_YAD1 CAM+A2CH
    MINT32 ANR2_K_TH_C_U_L        = rParam.rUpperIso_LowerZoom.yad1.bits.ANR2_K_TH_C;

    // CAM_ANR2_Y4LUT1 CAM+A34H
    MINT32 ANR2_Y_CPX1_U_L        = rParam.rUpperIso_LowerZoom.y4lut1.bits.ANR2_Y_CPX1;
    MINT32 ANR2_Y_CPX2_U_L        = rParam.rUpperIso_LowerZoom.y4lut1.bits.ANR2_Y_CPX2;
    MINT32 ANR2_Y_CPX3_U_L        = rParam.rUpperIso_LowerZoom.y4lut1.bits.ANR2_Y_CPX3;

    // CAM_ANR2_Y4LUT2 CAM+A38H
    MINT32 ANR2_Y_SCALE_CPY0_U_L  = rParam.rUpperIso_LowerZoom.y4lut2.bits.ANR2_Y_SCALE_CPY0;
    MINT32 ANR2_Y_SCALE_CPY1_U_L  = rParam.rUpperIso_LowerZoom.y4lut2.bits.ANR2_Y_SCALE_CPY1;
    MINT32 ANR2_Y_SCALE_CPY2_U_L  = rParam.rUpperIso_LowerZoom.y4lut2.bits.ANR2_Y_SCALE_CPY2;
    MINT32 ANR2_Y_SCALE_CPY3_U_L  = rParam.rUpperIso_LowerZoom.y4lut2.bits.ANR2_Y_SCALE_CPY3;

    // CAM_ANR2_Y4LUT3 CAM+A3CH
    MINT32 ANR2_Y_SCALE_SP0_U_L   = rParam.rUpperIso_LowerZoom.y4lut3.bits.ANR2_Y_SCALE_SP0;
    MINT32 ANR2_Y_SCALE_SP1_U_L   = rParam.rUpperIso_LowerZoom.y4lut3.bits.ANR2_Y_SCALE_SP1;
    MINT32 ANR2_Y_SCALE_SP2_U_L   = rParam.rUpperIso_LowerZoom.y4lut3.bits.ANR2_Y_SCALE_SP2;
    MINT32 ANR2_Y_SCALE_SP3_U_L   = rParam.rUpperIso_LowerZoom.y4lut3.bits.ANR2_Y_SCALE_SP3;

    // CAM_ANR2_L4LUT1
    MINT32 ANR2_LCE_X1_U_L        = rParam.rUpperIso_LowerZoom.l4lut1.bits.ANR2_LCE_X1;
    MINT32 ANR2_LCE_X2_U_L        = rParam.rUpperIso_LowerZoom.l4lut1.bits.ANR2_LCE_X2;
    MINT32 ANR2_LCE_X3_U_L        = rParam.rUpperIso_LowerZoom.l4lut1.bits.ANR2_LCE_X3;

    // CAM_ANR2_L4LUT2
    MINT32 ANR2_LCE_GAIN0_U_L     = rParam.rUpperIso_LowerZoom.l4lut2.bits.ANR2_LCE_GAIN0;
    MINT32 ANR2_LCE_GAIN1_U_L     = rParam.rUpperIso_LowerZoom.l4lut2.bits.ANR2_LCE_GAIN1;
    MINT32 ANR2_LCE_GAIN2_U_L     = rParam.rUpperIso_LowerZoom.l4lut2.bits.ANR2_LCE_GAIN2;
    MINT32 ANR2_LCE_GAIN3_U_L     = rParam.rUpperIso_LowerZoom.l4lut2.bits.ANR2_LCE_GAIN3;

    // CAM_ANR2_L4LUT3
    MINT32 ANR2_LCE_SP0_U_L       = rParam.rUpperIso_LowerZoom.l4lut3.bits.ANR2_LCE_SP0;
    MINT32 ANR2_LCE_SP1_U_L       = rParam.rUpperIso_LowerZoom.l4lut3.bits.ANR2_LCE_SP1;
    MINT32 ANR2_LCE_SP2_U_L       = rParam.rUpperIso_LowerZoom.l4lut3.bits.ANR2_LCE_SP2;
    MINT32 ANR2_LCE_SP3_U_L       = rParam.rUpperIso_LowerZoom.l4lut3.bits.ANR2_LCE_SP3;

    // CAM_ANR2_CAD CAM+A44H
    MINT32 ANR2_PTC_VGAIN_U_L     = rParam.rUpperIso_LowerZoom.cad.bits.ANR2_PTC_VGAIN;
    MINT32 ANR2_C_GAIN_U_L        = rParam.rUpperIso_LowerZoom.cad.bits.ANR2_C_GAIN;
    MINT32 ANR2_PTC_GAIN_TH_U_L   = rParam.rUpperIso_LowerZoom.cad.bits.ANR2_PTC_GAIN_TH;
    MINT32 ANR2_C_L_DIFF_TH_U_L   = rParam.rUpperIso_LowerZoom.cad.bits.ANR2_C_L_DIFF_TH;
    MINT32 ANR2_C_MODE_U_L        = rParam.rUpperIso_LowerZoom.cad.bits.ANR2_C_MODE;

    // CAM_ANR2_PTC CAM+A48H
    MINT32 ANR2_PTC1_U_L          = rParam.rUpperIso_LowerZoom.ptc.bits.ANR2_PTC1;
    MINT32 ANR2_PTC2_U_L          = rParam.rUpperIso_LowerZoom.ptc.bits.ANR2_PTC2;
    MINT32 ANR2_PTC3_U_L          = rParam.rUpperIso_LowerZoom.ptc.bits.ANR2_PTC3;
    MINT32 ANR2_PTC4_U_L          = rParam.rUpperIso_LowerZoom.ptc.bits.ANR2_PTC4;

    // CAM_ANR2_LCE CAM+A4CH
    MINT32 ANR2_LCE_C_GAIN_U_L    = rParam.rUpperIso_LowerZoom.lce.bits.ANR2_LCE_C_GAIN;
    MINT32 ANR2_LM_WT_U_L         = rParam.rUpperIso_LowerZoom.lce.bits.ANR2_LM_WT;

    // CAM_ANR2_MED1
    MINT32 ANR2_COR_TH_U_L        = rParam.rUpperIso_LowerZoom.med1.bits.ANR2_COR_TH;
    MINT32 ANR2_COR_SL_U_L        = rParam.rUpperIso_LowerZoom.med1.bits.ANR2_COR_SL;
    MINT32 ANR2_MCD_TH_U_L        = rParam.rUpperIso_LowerZoom.med1.bits.ANR2_MCD_TH;
    MINT32 ANR2_MCD_SL_U_L        = rParam.rUpperIso_LowerZoom.med1.bits.ANR2_MCD_SL;
    MINT32 ANR2_LCL_TH_U_L        = rParam.rUpperIso_LowerZoom.med1.bits.ANR2_LCL_TH;

    // CAM_ANR2_MED2
    MINT32 ANR2_LCL_SL_U_L        = rParam.rUpperIso_LowerZoom.med2.bits.ANR2_LCL_SL;
    MINT32 ANR2_LCL_LV_U_L        = rParam.rUpperIso_LowerZoom.med2.bits.ANR2_LCL_LV;
    MINT32 ANR2_SCL_TH_U_L        = rParam.rUpperIso_LowerZoom.med2.bits.ANR2_SCL_TH;
    MINT32 ANR2_SCL_SL_U_L        = rParam.rUpperIso_LowerZoom.med2.bits.ANR2_SCL_SL;
    MINT32 ANR2_SCL_LV_U_L        = rParam.rUpperIso_LowerZoom.med2.bits.ANR2_SCL_LV;

    // CAM_ANR2_MED3
    MINT32 ANR2_NCL_TH_U_L        = rParam.rUpperIso_LowerZoom.med3.bits.ANR2_NCL_TH;
    MINT32 ANR2_NCL_SL_U_L        = rParam.rUpperIso_LowerZoom.med3.bits.ANR2_NCL_SL;
    MINT32 ANR2_NCL_LV_U_L        = rParam.rUpperIso_LowerZoom.med3.bits.ANR2_NCL_LV;
    MINT32 ANR2_VAR_U_L           = rParam.rUpperIso_LowerZoom.med3.bits.ANR2_VAR;
    MINT32 ANR2_Y0_U_L            = rParam.rUpperIso_LowerZoom.med3.bits.ANR2_Y0;

    // CAM_ANR2_MED4
    MINT32 ANR2_Y1_U_L            = rParam.rUpperIso_LowerZoom.med4.bits.ANR2_Y1;
    MINT32 ANR2_Y2_U_L            = rParam.rUpperIso_LowerZoom.med4.bits.ANR2_Y2;
    MINT32 ANR2_Y3_U_L            = rParam.rUpperIso_LowerZoom.med4.bits.ANR2_Y3;
    MINT32 ANR2_Y4_U_L            = rParam.rUpperIso_LowerZoom.med4.bits.ANR2_Y4;

	// CAM_ANR_MED5
	MINT32 ANR2_LCL_OFT_U_L			 = rParam.rUpperIso_LowerZoom.med5.bits.ANR2_LCL_OFT;
	MINT32 ANR2_SCL_OFT_U_L			 = rParam.rUpperIso_LowerZoom.med5.bits.ANR2_SCL_OFT;
	MINT32 ANR2_NCL_OFT_U_L			 = rParam.rUpperIso_LowerZoom.med5.bits.ANR2_NCL_OFT;

    // CAM_ANR2_ACTC CAM+A64H
    MINT32 ANR2_ACT_BLD_BASE_C_U_L= rParam.rUpperIso_LowerZoom.actc.bits.ANR2_ACT_BLD_BASE_C;
	MINT32 ANR2_C_DITH_U_U_L= rParam.rUpperIso_LowerZoom.actc.bits.ANR2_C_DITH_U;
	MINT32 ANR2_C_DITH_V_U_L= rParam.rUpperIso_LowerZoom.actc.bits.ANR2_C_DITH_V;

	//    ====================================================================================================================================
    //    Get Up ISO Up Zoom
    //    ====================================================================================================================================
    // CAM_ANR2_CON1 CAM+A20H
    MINT32 ANR2_ENC_U_U           = rParam.rUpperIso_UpperZoom.con1.bits.ANR2_ENC;
    MINT32 ANR2_ENY_U_U           = rParam.rUpperIso_UpperZoom.con1.bits.ANR2_ENY;
    MINT32 ANR2_SCALE_MODE_U_U    = rParam.rUpperIso_UpperZoom.con1.bits.ANR2_SCALE_MODE;
    MINT32 ANR2_MODE_U_U          = rParam.rUpperIso_UpperZoom.con1.bits.ANR2_MODE;
    MINT32 ANR2_LCE_LINK_U_U      = rParam.rUpperIso_UpperZoom.con1.bits.ANR2_LCE_LINK;

    // CAM_ANR2_CON2 CAM+A24H
    MINT32 ANR2_C_SM_EDGE_U_U     = rParam.rUpperIso_UpperZoom.con2.bits.ANR2_C_SM_EDGE;
    MINT32 ANR2_FLT_C_U_U         = rParam.rUpperIso_UpperZoom.con2.bits.ANR2_FLT_C;

    // CAM_ANR2_YAD1 CAM+A2CH
    MINT32 ANR2_K_TH_C_U_U        = rParam.rUpperIso_UpperZoom.yad1.bits.ANR2_K_TH_C;

    // CAM_ANR2_Y4LUT1 CAM+A34H
    MINT32 ANR2_Y_CPX1_U_U        = rParam.rUpperIso_UpperZoom.y4lut1.bits.ANR2_Y_CPX1;
    MINT32 ANR2_Y_CPX2_U_U        = rParam.rUpperIso_UpperZoom.y4lut1.bits.ANR2_Y_CPX2;
    MINT32 ANR2_Y_CPX3_U_U        = rParam.rUpperIso_UpperZoom.y4lut1.bits.ANR2_Y_CPX3;

    // CAM_ANR2_Y4LUT2 CAM+A38H
    MINT32 ANR2_Y_SCALE_CPY0_U_U  = rParam.rUpperIso_UpperZoom.y4lut2.bits.ANR2_Y_SCALE_CPY0;
    MINT32 ANR2_Y_SCALE_CPY1_U_U  = rParam.rUpperIso_UpperZoom.y4lut2.bits.ANR2_Y_SCALE_CPY1;
    MINT32 ANR2_Y_SCALE_CPY2_U_U  = rParam.rUpperIso_UpperZoom.y4lut2.bits.ANR2_Y_SCALE_CPY2;
    MINT32 ANR2_Y_SCALE_CPY3_U_U  = rParam.rUpperIso_UpperZoom.y4lut2.bits.ANR2_Y_SCALE_CPY3;

    // CAM_ANR2_Y4LUT3 CAM+A3CH
    MINT32 ANR2_Y_SCALE_SP0_U_U   = rParam.rUpperIso_UpperZoom.y4lut3.bits.ANR2_Y_SCALE_SP0;
    MINT32 ANR2_Y_SCALE_SP1_U_U   = rParam.rUpperIso_UpperZoom.y4lut3.bits.ANR2_Y_SCALE_SP1;
    MINT32 ANR2_Y_SCALE_SP2_U_U   = rParam.rUpperIso_UpperZoom.y4lut3.bits.ANR2_Y_SCALE_SP2;
    MINT32 ANR2_Y_SCALE_SP3_U_U   = rParam.rUpperIso_UpperZoom.y4lut3.bits.ANR2_Y_SCALE_SP3;

    // CAM_ANR2_L4LUT1
    MINT32 ANR2_LCE_X1_U_U        = rParam.rUpperIso_UpperZoom.l4lut1.bits.ANR2_LCE_X1;
    MINT32 ANR2_LCE_X2_U_U        = rParam.rUpperIso_UpperZoom.l4lut1.bits.ANR2_LCE_X2;
    MINT32 ANR2_LCE_X3_U_U        = rParam.rUpperIso_UpperZoom.l4lut1.bits.ANR2_LCE_X3;

    // CAM_ANR2_L4LUT2
    MINT32 ANR2_LCE_GAIN0_U_U     = rParam.rUpperIso_UpperZoom.l4lut2.bits.ANR2_LCE_GAIN0;
    MINT32 ANR2_LCE_GAIN1_U_U     = rParam.rUpperIso_UpperZoom.l4lut2.bits.ANR2_LCE_GAIN1;
    MINT32 ANR2_LCE_GAIN2_U_U     = rParam.rUpperIso_UpperZoom.l4lut2.bits.ANR2_LCE_GAIN2;
    MINT32 ANR2_LCE_GAIN3_U_U     = rParam.rUpperIso_UpperZoom.l4lut2.bits.ANR2_LCE_GAIN3;

    // CAM_ANR2_L4LUT3
    MINT32 ANR2_LCE_SP0_U_U       = rParam.rUpperIso_UpperZoom.l4lut3.bits.ANR2_LCE_SP0;
    MINT32 ANR2_LCE_SP1_U_U       = rParam.rUpperIso_UpperZoom.l4lut3.bits.ANR2_LCE_SP1;
    MINT32 ANR2_LCE_SP2_U_U       = rParam.rUpperIso_UpperZoom.l4lut3.bits.ANR2_LCE_SP2;
    MINT32 ANR2_LCE_SP3_U_U       = rParam.rUpperIso_UpperZoom.l4lut3.bits.ANR2_LCE_SP3;

    // CAM_ANR2_CAD CAM+A44H
    MINT32 ANR2_PTC_VGAIN_U_U     = rParam.rUpperIso_UpperZoom.cad.bits.ANR2_PTC_VGAIN;
    MINT32 ANR2_C_GAIN_U_U        = rParam.rUpperIso_UpperZoom.cad.bits.ANR2_C_GAIN;
    MINT32 ANR2_PTC_GAIN_TH_U_U   = rParam.rUpperIso_UpperZoom.cad.bits.ANR2_PTC_GAIN_TH;
    MINT32 ANR2_C_L_DIFF_TH_U_U   = rParam.rUpperIso_UpperZoom.cad.bits.ANR2_C_L_DIFF_TH;
    MINT32 ANR2_C_MODE_U_U        = rParam.rUpperIso_UpperZoom.cad.bits.ANR2_C_MODE;

    // CAM_ANR2_PTC CAM+A48H
    MINT32 ANR2_PTC1_U_U          = rParam.rUpperIso_UpperZoom.ptc.bits.ANR2_PTC1;
    MINT32 ANR2_PTC2_U_U          = rParam.rUpperIso_UpperZoom.ptc.bits.ANR2_PTC2;
    MINT32 ANR2_PTC3_U_U          = rParam.rUpperIso_UpperZoom.ptc.bits.ANR2_PTC3;
    MINT32 ANR2_PTC4_U_U          = rParam.rUpperIso_UpperZoom.ptc.bits.ANR2_PTC4;

    // CAM_ANR2_LCE CAM+A4CH
    MINT32 ANR2_LCE_C_GAIN_U_U    = rParam.rUpperIso_UpperZoom.lce.bits.ANR2_LCE_C_GAIN;
    MINT32 ANR2_LM_WT_U_U         = rParam.rUpperIso_UpperZoom.lce.bits.ANR2_LM_WT;

    // CAM_ANR2_MED1
    MINT32 ANR2_COR_TH_U_U        = rParam.rUpperIso_UpperZoom.med1.bits.ANR2_COR_TH;
    MINT32 ANR2_COR_SL_U_U        = rParam.rUpperIso_UpperZoom.med1.bits.ANR2_COR_SL;
    MINT32 ANR2_MCD_TH_U_U        = rParam.rUpperIso_UpperZoom.med1.bits.ANR2_MCD_TH;
    MINT32 ANR2_MCD_SL_U_U        = rParam.rUpperIso_UpperZoom.med1.bits.ANR2_MCD_SL;
    MINT32 ANR2_LCL_TH_U_U        = rParam.rUpperIso_UpperZoom.med1.bits.ANR2_LCL_TH;

    // CAM_ANR2_MED2
    MINT32 ANR2_LCL_SL_U_U        = rParam.rUpperIso_UpperZoom.med2.bits.ANR2_LCL_SL;
    MINT32 ANR2_LCL_LV_U_U        = rParam.rUpperIso_UpperZoom.med2.bits.ANR2_LCL_LV;
    MINT32 ANR2_SCL_TH_U_U        = rParam.rUpperIso_UpperZoom.med2.bits.ANR2_SCL_TH;
    MINT32 ANR2_SCL_SL_U_U        = rParam.rUpperIso_UpperZoom.med2.bits.ANR2_SCL_SL;
    MINT32 ANR2_SCL_LV_U_U        = rParam.rUpperIso_UpperZoom.med2.bits.ANR2_SCL_LV;

    // CAM_ANR2_MED3
    MINT32 ANR2_NCL_TH_U_U        = rParam.rUpperIso_UpperZoom.med3.bits.ANR2_NCL_TH;
    MINT32 ANR2_NCL_SL_U_U        = rParam.rUpperIso_UpperZoom.med3.bits.ANR2_NCL_SL;
    MINT32 ANR2_NCL_LV_U_U        = rParam.rUpperIso_UpperZoom.med3.bits.ANR2_NCL_LV;
    MINT32 ANR2_VAR_U_U           = rParam.rUpperIso_UpperZoom.med3.bits.ANR2_VAR;
    MINT32 ANR2_Y0_U_U            = rParam.rUpperIso_UpperZoom.med3.bits.ANR2_Y0;

    // CAM_ANR2_MED4
    MINT32 ANR2_Y1_U_U            = rParam.rUpperIso_UpperZoom.med4.bits.ANR2_Y1;
    MINT32 ANR2_Y2_U_U            = rParam.rUpperIso_UpperZoom.med4.bits.ANR2_Y2;
    MINT32 ANR2_Y3_U_U            = rParam.rUpperIso_UpperZoom.med4.bits.ANR2_Y3;
    MINT32 ANR2_Y4_U_U            = rParam.rUpperIso_UpperZoom.med4.bits.ANR2_Y4;

	// CAM_ANR_MED5
	MINT32 ANR2_LCL_OFT_U_U			 = rParam.rUpperIso_UpperZoom.med5.bits.ANR2_LCL_OFT;
	MINT32 ANR2_SCL_OFT_U_U			 = rParam.rUpperIso_UpperZoom.med5.bits.ANR2_SCL_OFT;
	MINT32 ANR2_NCL_OFT_U_U			 = rParam.rUpperIso_UpperZoom.med5.bits.ANR2_NCL_OFT;

    // CAM_ANR2_ACTC CAM+A64H
    MINT32 ANR2_ACT_BLD_BASE_C_U_U= rParam.rUpperIso_UpperZoom.actc.bits.ANR2_ACT_BLD_BASE_C;
	MINT32 ANR2_C_DITH_U_U_U= rParam.rUpperIso_UpperZoom.actc.bits.ANR2_C_DITH_U;
	MINT32 ANR2_C_DITH_V_U_U= rParam.rUpperIso_UpperZoom.actc.bits.ANR2_C_DITH_V;



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

	// CAM_ANR_MED5
	MINT32 ANR2_LCL_OFT;
	MINT32 ANR2_SCL_OFT;
	MINT32 ANR2_NCL_OFT;

    // CAM_ANR2_ACTC CAM+A64H
    MINT32 ANR2_ACT_BLD_BASE_C;
	MINT32 ANR2_C_DITH_U;
	MINT32 ANR2_C_DITH_V;

    //    ====================================================================================================================================
    //    Negative Conversion
    //    ====================================================================================================================================
    ANR2_Y_SCALE_SP3_L_L = (ANR2_Y_SCALE_SP3_L_L > 15) ? (ANR2_Y_SCALE_SP3_L_L - 32) : ANR2_Y_SCALE_SP3_L_L;
    ANR2_LCE_SP3_L_L     = (ANR2_LCE_SP3_L_L > 31) ? (ANR2_LCE_SP3_L_L - 64) : ANR2_LCE_SP3_L_L;

	ANR2_Y_SCALE_SP3_L_U = (ANR2_Y_SCALE_SP3_L_U > 15) ? (ANR2_Y_SCALE_SP3_L_U - 32) : ANR2_Y_SCALE_SP3_L_U;
    ANR2_LCE_SP3_L_U     = (ANR2_LCE_SP3_L_U > 31) ? (ANR2_LCE_SP3_L_U - 64) : ANR2_LCE_SP3_L_U;

	ANR2_Y_SCALE_SP3_U_L = (ANR2_Y_SCALE_SP3_U_L > 15) ? (ANR2_Y_SCALE_SP3_U_L - 32) : ANR2_Y_SCALE_SP3_U_L;
    ANR2_LCE_SP3_U_L     = (ANR2_LCE_SP3_U_L > 31) ? (ANR2_LCE_SP3_U_L - 64) : ANR2_LCE_SP3_U_L;

	ANR2_Y_SCALE_SP3_U_U = (ANR2_Y_SCALE_SP3_U_U > 15) ? (ANR2_Y_SCALE_SP3_U_U - 32) : ANR2_Y_SCALE_SP3_U_U;
    ANR2_LCE_SP3_U_U     = (ANR2_LCE_SP3_U_U > 31) ? (ANR2_LCE_SP3_U_U - 64) : ANR2_LCE_SP3_U_U;



    //    ====================================================================================================================================
    //    Start Parameter Interpolation
    //    ====================================================================================================================================
    MINT32 ISO = u4RealISO;
    MINT32 ISO_low = u4LowerISO;
    MINT32 ISO_high = u4UpperISO;


    //    Registers that can not be interpolated
    ANR2_ENC            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_ENC_L_L, ANR2_ENC_L_U, ANR2_ENC_U_L, ANR2_ENC_U_U,1);
    ANR2_ENY            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_ENY_L_L, ANR2_ENY_L_U, ANR2_ENY_U_L, ANR2_ENY_U_U,1);
    ANR2_MODE            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_MODE_L_L, ANR2_MODE_L_U, ANR2_MODE_U_L, ANR2_MODE_U_U,1);
    ANR2_C_SM_EDGE            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_C_SM_EDGE_L_L, ANR2_C_SM_EDGE_L_U, ANR2_C_SM_EDGE_U_L, ANR2_C_SM_EDGE_U_U,1);
    ANR2_FLT_C            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_FLT_C_L_L, ANR2_FLT_C_L_U, ANR2_FLT_C_U_L, ANR2_FLT_C_U_U,1);

    //    Registers that can be interpolated
    ANR2_Y_CPX1            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_Y_CPX1_L_L, ANR2_Y_CPX1_L_U, ANR2_Y_CPX1_U_L, ANR2_Y_CPX1_U_U,0);
    ANR2_Y_CPX2            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_Y_CPX2_L_L, ANR2_Y_CPX2_L_U, ANR2_Y_CPX2_U_L, ANR2_Y_CPX2_U_U,0);
    ANR2_Y_CPX3            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_Y_CPX3_L_L, ANR2_Y_CPX3_L_U, ANR2_Y_CPX3_U_L, ANR2_Y_CPX3_U_U,0);
    ANR2_Y_SCALE_CPY0            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_Y_SCALE_CPY0_L_L, ANR2_Y_SCALE_CPY0_L_U, ANR2_Y_SCALE_CPY0_U_L, ANR2_Y_SCALE_CPY0_U_U,0);
    ANR2_Y_SCALE_CPY1            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_Y_SCALE_CPY1_L_L, ANR2_Y_SCALE_CPY1_L_U, ANR2_Y_SCALE_CPY1_U_L, ANR2_Y_SCALE_CPY1_U_U,0);
    ANR2_Y_SCALE_CPY2            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_Y_SCALE_CPY2_L_L, ANR2_Y_SCALE_CPY2_L_U, ANR2_Y_SCALE_CPY2_U_L, ANR2_Y_SCALE_CPY2_U_U,0);
    ANR2_Y_SCALE_CPY3            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_Y_SCALE_CPY3_L_L, ANR2_Y_SCALE_CPY3_L_U, ANR2_Y_SCALE_CPY3_U_L, ANR2_Y_SCALE_CPY3_U_U,0);
    ANR2_PTC_VGAIN            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_PTC_VGAIN_L_L, ANR2_PTC_VGAIN_L_U, ANR2_PTC_VGAIN_U_L, ANR2_PTC_VGAIN_U_U,0);
    ANR2_C_GAIN            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_C_GAIN_L_L, ANR2_C_GAIN_L_U, ANR2_C_GAIN_U_L, ANR2_C_GAIN_U_U,0);
    ANR2_PTC_GAIN_TH            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_PTC_GAIN_TH_L_L, ANR2_PTC_GAIN_TH_L_U, ANR2_PTC_GAIN_TH_U_L, ANR2_PTC_GAIN_TH_U_U,0);
    ANR2_C_L_DIFF_TH            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_C_L_DIFF_TH_L_L, ANR2_C_L_DIFF_TH_L_U, ANR2_C_L_DIFF_TH_U_L, ANR2_C_L_DIFF_TH_U_U,0);
    ANR2_C_MODE            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_C_MODE_L_L, ANR2_C_MODE_L_U, ANR2_C_MODE_U_L, ANR2_C_MODE_U_U,0);
    ANR2_PTC1            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_PTC1_L_L, ANR2_PTC1_L_U, ANR2_PTC1_U_L, ANR2_PTC1_U_U,0);
    ANR2_PTC2            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_PTC2_L_L, ANR2_PTC2_L_U, ANR2_PTC2_U_L, ANR2_PTC2_U_U,0);
    ANR2_PTC3            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_PTC3_L_L, ANR2_PTC3_L_U, ANR2_PTC3_U_L, ANR2_PTC3_U_U,0);
    ANR2_PTC4            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_PTC4_L_L, ANR2_PTC4_L_U, ANR2_PTC4_U_L, ANR2_PTC4_U_U,0);
    ANR2_LM_WT            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_LM_WT_L_L, ANR2_LM_WT_L_U, ANR2_LM_WT_U_L, ANR2_LM_WT_U_U,0);
    ANR2_ACT_BLD_BASE_C            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_ACT_BLD_BASE_C_L_L, ANR2_ACT_BLD_BASE_C_L_U, ANR2_ACT_BLD_BASE_C_U_L, ANR2_ACT_BLD_BASE_C_U_U,0);
	ANR2_C_DITH_U            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_C_DITH_U_L_L, ANR2_C_DITH_U_L_U, ANR2_C_DITH_U_U_L, ANR2_C_DITH_U_U_U,0);
	ANR2_C_DITH_V            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_C_DITH_V_L_L, ANR2_C_DITH_V_L_U, ANR2_C_DITH_V_U_L, ANR2_C_DITH_V_U_U,0);

    // ANR2_SCALE_MODE
    ANR2_SCALE_MODE            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_SCALE_MODE_L_L, ANR2_SCALE_MODE_L_U, ANR2_SCALE_MODE_U_L, ANR2_SCALE_MODE_U_U,1);
    ANR2_K_TH_C            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_K_TH_C_L_L, ANR2_K_TH_C_L_U, ANR2_K_TH_C_U_L, ANR2_K_TH_C_U_U,1);

    // ANR2_LCE_LINK

    ANR2_LCE_LINK            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_LCE_LINK_L_L, ANR2_LCE_LINK_L_U, ANR2_LCE_LINK_U_L, ANR2_LCE_LINK_U_U,1);
    ANR2_LCE_X1            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_LCE_X1_L_L, ANR2_LCE_X1_L_U, ANR2_LCE_X1_U_L, ANR2_LCE_X1_U_U,0);
    ANR2_LCE_X2            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_LCE_X2_L_L, ANR2_LCE_X2_L_U, ANR2_LCE_X2_U_L, ANR2_LCE_X2_U_U,0);
    ANR2_LCE_X3            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_LCE_X3_L_L, ANR2_LCE_X3_L_U, ANR2_LCE_X3_U_L, ANR2_LCE_X3_U_U,0);
    ANR2_LCE_GAIN0            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_LCE_GAIN0_L_L, ANR2_LCE_GAIN0_L_U, ANR2_LCE_GAIN0_U_L, ANR2_LCE_GAIN0_U_U,0);
    ANR2_LCE_GAIN1            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_LCE_GAIN1_L_L, ANR2_LCE_GAIN1_L_U, ANR2_LCE_GAIN1_U_L, ANR2_LCE_GAIN1_U_U,0);
    ANR2_LCE_GAIN2            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_LCE_GAIN2_L_L, ANR2_LCE_GAIN2_L_U, ANR2_LCE_GAIN2_U_L, ANR2_LCE_GAIN2_U_U,0);
    ANR2_LCE_GAIN3            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_LCE_GAIN3_L_L, ANR2_LCE_GAIN3_L_U, ANR2_LCE_GAIN3_U_L, ANR2_LCE_GAIN3_U_U,0);
    ANR2_LCE_C_GAIN            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_LCE_C_GAIN_L_L, ANR2_LCE_C_GAIN_L_U, ANR2_LCE_C_GAIN_U_L, ANR2_LCE_C_GAIN_U_U,0);

    // SP Calculation
    ANR2_Y_SCALE_SP0    = CalculateLUTSP(         0, ANR2_Y_CPX1, ANR2_Y_SCALE_CPY0, ANR2_Y_SCALE_CPY1, 128, 15, -15);
    ANR2_Y_SCALE_SP1    = CalculateLUTSP(ANR2_Y_CPX1, ANR2_Y_CPX2, ANR2_Y_SCALE_CPY1, ANR2_Y_SCALE_CPY2, 128, 15, -15);
    ANR2_Y_SCALE_SP2    = CalculateLUTSP(ANR2_Y_CPX2, ANR2_Y_CPX3, ANR2_Y_SCALE_CPY2, ANR2_Y_SCALE_CPY3, 128, 15, -15);
    ANR2_Y_SCALE_SP3            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_Y_SCALE_SP3_L_L, ANR2_Y_SCALE_SP3_L_U, ANR2_Y_SCALE_SP3_U_L, ANR2_Y_SCALE_SP3_U_U,0);
    ANR2_LCE_SP0        = CalculateLUTSP(         0, ANR2_LCE_X1, ANR2_LCE_GAIN0   , ANR2_LCE_GAIN1   , 128, 31, -31);
    ANR2_LCE_SP1        = CalculateLUTSP(ANR2_LCE_X1, ANR2_LCE_X2, ANR2_LCE_GAIN1   , ANR2_LCE_GAIN2   , 128, 31, -31);
    ANR2_LCE_SP2        = CalculateLUTSP(ANR2_LCE_X2, ANR2_LCE_X3, ANR2_LCE_GAIN2   , ANR2_LCE_GAIN3   , 128, 31, -31);
    ANR2_LCE_SP3            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_LCE_SP3_L_L, ANR2_LCE_SP3_L_U, ANR2_LCE_SP3_U_L, ANR2_LCE_SP3_U_U,0);

    // ANR2_MEDIAN_EN
    ANR2_COR_TH            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_COR_TH_L_L, ANR2_COR_TH_L_U, ANR2_COR_TH_U_L, ANR2_COR_TH_U_U,0);
    ANR2_COR_SL            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_COR_SL_L_L, ANR2_COR_SL_L_U, ANR2_COR_SL_U_L, ANR2_COR_SL_U_U,0);
    ANR2_MCD_TH            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_MCD_TH_L_L, ANR2_MCD_TH_L_U, ANR2_MCD_TH_U_L, ANR2_MCD_TH_U_U,0);
    ANR2_MCD_SL            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_MCD_SL_L_L, ANR2_MCD_SL_L_U, ANR2_MCD_SL_U_L, ANR2_MCD_SL_U_U,0);
    ANR2_LCL_TH            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_LCL_TH_L_L, ANR2_LCL_TH_L_U, ANR2_LCL_TH_U_L, ANR2_LCL_TH_U_U,0);
    ANR2_LCL_SL            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_LCL_SL_L_L, ANR2_LCL_SL_L_U, ANR2_LCL_SL_U_L, ANR2_LCL_SL_U_U,0);
    ANR2_LCL_LV            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_LCL_LV_L_L, ANR2_LCL_LV_L_U, ANR2_LCL_LV_U_L, ANR2_LCL_LV_U_U,0);
    ANR2_SCL_TH            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_SCL_TH_L_L, ANR2_SCL_TH_L_U, ANR2_SCL_TH_U_L, ANR2_SCL_TH_U_U,0);
    ANR2_SCL_SL            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_SCL_SL_L_L, ANR2_SCL_SL_L_U, ANR2_SCL_SL_U_L, ANR2_SCL_SL_U_U,0);
    ANR2_SCL_LV            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_SCL_LV_L_L, ANR2_SCL_LV_L_U, ANR2_SCL_LV_U_L, ANR2_SCL_LV_U_U,0);
    ANR2_NCL_TH            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_NCL_TH_L_L, ANR2_NCL_TH_L_U, ANR2_NCL_TH_U_L, ANR2_NCL_TH_U_U,0);
    ANR2_NCL_SL            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_NCL_SL_L_L, ANR2_NCL_SL_L_U, ANR2_NCL_SL_U_L, ANR2_NCL_SL_U_U,0);
    ANR2_NCL_LV            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_NCL_LV_L_L, ANR2_NCL_LV_L_U, ANR2_NCL_LV_U_L, ANR2_NCL_LV_U_U,0);
    ANR2_VAR            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_VAR_L_L, ANR2_VAR_L_U, ANR2_VAR_U_L, ANR2_VAR_U_U,0);
    ANR2_Y0            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_Y0_L_L, ANR2_Y0_L_U, ANR2_Y0_U_L, ANR2_Y0_U_U,0);
    ANR2_Y1            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_Y1_L_L, ANR2_Y1_L_U, ANR2_Y1_U_L, ANR2_Y1_U_U,0);
    ANR2_Y2            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_Y2_L_L, ANR2_Y2_L_U, ANR2_Y2_U_L, ANR2_Y2_U_U,0);
    ANR2_Y3            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_Y3_L_L, ANR2_Y3_L_U, ANR2_Y3_U_L, ANR2_Y3_U_U,0);
    ANR2_Y4            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_Y4_L_L, ANR2_Y4_L_U, ANR2_Y4_U_L, ANR2_Y4_U_U,0);
	ANR2_LCL_OFT            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_LCL_OFT_L_L, ANR2_LCL_OFT_L_U, ANR2_LCL_OFT_U_L, ANR2_LCL_OFT_U_U,0);
	ANR2_SCL_OFT            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_SCL_OFT_L_L, ANR2_SCL_OFT_L_U, ANR2_SCL_OFT_U_L, ANR2_SCL_OFT_U_U,0);
	ANR2_NCL_OFT            = InterParam_NR(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, ANR2_NCL_OFT_L_L, ANR2_NCL_OFT_L_U, ANR2_NCL_OFT_U_L, ANR2_NCL_OFT_U_U,0);



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
    rSmoothANR2.con1.bits.ANR2_ENC = ANR2_ENC;
    rSmoothANR2.con1.bits.ANR2_ENY = ANR2_ENY;
    rSmoothANR2.con1.bits.ANR2_SCALE_MODE = ANR2_SCALE_MODE;
    rSmoothANR2.con1.bits.ANR2_MODE = ANR2_MODE;
    rSmoothANR2.con1.bits.ANR2_LCE_LINK = ANR2_LCE_LINK;

    // CAM_ANR2_CON2 CAM+A24H
    rSmoothANR2.con2.bits.ANR2_C_SM_EDGE = ANR2_C_SM_EDGE;
    rSmoothANR2.con2.bits.ANR2_FLT_C = ANR2_FLT_C;

    // CAM_ANR2_YAD1 CAM+A2CH
    rSmoothANR2.yad1.bits.ANR2_K_TH_C = ANR2_K_TH_C;

    // CAM_ANR2_Y4LUT1 CAM+A34H
    rSmoothANR2.y4lut1.bits.ANR2_Y_CPX1 = ANR2_Y_CPX1;
    rSmoothANR2.y4lut1.bits.ANR2_Y_CPX2 = ANR2_Y_CPX2;
    rSmoothANR2.y4lut1.bits.ANR2_Y_CPX3 = ANR2_Y_CPX3;

    // CAM_ANR2_Y4LUT2 CAM+A38H
    rSmoothANR2.y4lut2.bits.ANR2_Y_SCALE_CPY0 = ANR2_Y_SCALE_CPY0;
    rSmoothANR2.y4lut2.bits.ANR2_Y_SCALE_CPY1 = ANR2_Y_SCALE_CPY1;
    rSmoothANR2.y4lut2.bits.ANR2_Y_SCALE_CPY2 = ANR2_Y_SCALE_CPY2;
    rSmoothANR2.y4lut2.bits.ANR2_Y_SCALE_CPY3 = ANR2_Y_SCALE_CPY3;

    // CAM_ANR2_Y4LUT3 CAM+A3CH
    rSmoothANR2.y4lut3.bits.ANR2_Y_SCALE_SP0 = ANR2_Y_SCALE_SP0;
    rSmoothANR2.y4lut3.bits.ANR2_Y_SCALE_SP1 = ANR2_Y_SCALE_SP1;
    rSmoothANR2.y4lut3.bits.ANR2_Y_SCALE_SP2 = ANR2_Y_SCALE_SP2;
    rSmoothANR2.y4lut3.bits.ANR2_Y_SCALE_SP3 = ANR2_Y_SCALE_SP3;

    // CAM_ANR2_L4LUT1
    rSmoothANR2.l4lut1.bits.ANR2_LCE_X1 = ANR2_LCE_X1;
    rSmoothANR2.l4lut1.bits.ANR2_LCE_X2 = ANR2_LCE_X2;
    rSmoothANR2.l4lut1.bits.ANR2_LCE_X3 = ANR2_LCE_X3;

    // CAM_ANR2_L4LUT2
    rSmoothANR2.l4lut2.bits.ANR2_LCE_GAIN0 = ANR2_LCE_GAIN0;
    rSmoothANR2.l4lut2.bits.ANR2_LCE_GAIN1 = ANR2_LCE_GAIN1;
    rSmoothANR2.l4lut2.bits.ANR2_LCE_GAIN2 = ANR2_LCE_GAIN2;
    rSmoothANR2.l4lut2.bits.ANR2_LCE_GAIN3 = ANR2_LCE_GAIN3;

    // CAM_ANR2_L4LUT3
    rSmoothANR2.l4lut3.bits.ANR2_LCE_SP0 = ANR2_LCE_SP0;
    rSmoothANR2.l4lut3.bits.ANR2_LCE_SP1 = ANR2_LCE_SP1;
    rSmoothANR2.l4lut3.bits.ANR2_LCE_SP2 = ANR2_LCE_SP2;
    rSmoothANR2.l4lut3.bits.ANR2_LCE_SP3 = ANR2_LCE_SP3;

    // CAM_ANR2_CAD CAM+A44H
    rSmoothANR2.cad.bits.ANR2_PTC_VGAIN   = ANR2_PTC_VGAIN;
    rSmoothANR2.cad.bits.ANR2_C_GAIN      = ANR2_C_GAIN;
    rSmoothANR2.cad.bits.ANR2_PTC_GAIN_TH = ANR2_PTC_GAIN_TH;
    rSmoothANR2.cad.bits.ANR2_C_L_DIFF_TH = ANR2_C_L_DIFF_TH;
    rSmoothANR2.cad.bits.ANR2_C_MODE      = ANR2_C_MODE;

    // CAM_ANR2_PTC CAM+A48H
    rSmoothANR2.ptc.bits.ANR2_PTC1 = ANR2_PTC1;
    rSmoothANR2.ptc.bits.ANR2_PTC2 = ANR2_PTC2;
    rSmoothANR2.ptc.bits.ANR2_PTC3 = ANR2_PTC3;
    rSmoothANR2.ptc.bits.ANR2_PTC4 = ANR2_PTC4;

    // CAM_ANR2_LCE CAM+A4CH
    rSmoothANR2.lce.bits.ANR2_LCE_C_GAIN = ANR2_LCE_C_GAIN;
    rSmoothANR2.lce.bits.ANR2_LM_WT = ANR2_LM_WT;

    // CAM_ANR2_MED1
    rSmoothANR2.med1.bits.ANR2_COR_TH = ANR2_COR_TH;
    rSmoothANR2.med1.bits.ANR2_COR_SL = ANR2_COR_SL;
    rSmoothANR2.med1.bits.ANR2_MCD_TH = ANR2_MCD_TH;
    rSmoothANR2.med1.bits.ANR2_MCD_SL = ANR2_MCD_SL;
    rSmoothANR2.med1.bits.ANR2_LCL_TH = ANR2_LCL_TH;

    // CAM_ANR2_MED2
    rSmoothANR2.med2.bits.ANR2_LCL_SL = ANR2_LCL_SL;
    rSmoothANR2.med2.bits.ANR2_LCL_LV = ANR2_LCL_LV;
    rSmoothANR2.med2.bits.ANR2_SCL_TH = ANR2_SCL_TH;
    rSmoothANR2.med2.bits.ANR2_SCL_SL = ANR2_SCL_SL;
    rSmoothANR2.med2.bits.ANR2_SCL_LV = ANR2_SCL_LV;

    // CAM_ANR2_MED3
    rSmoothANR2.med3.bits.ANR2_NCL_TH = ANR2_NCL_TH;
    rSmoothANR2.med3.bits.ANR2_NCL_SL = ANR2_NCL_SL;
    rSmoothANR2.med3.bits.ANR2_NCL_LV = ANR2_NCL_LV;
    rSmoothANR2.med3.bits.ANR2_VAR = ANR2_VAR;
    rSmoothANR2.med3.bits.ANR2_Y0 = ANR2_Y0;

    // CAM_ANR2_MED4
    rSmoothANR2.med4.bits.ANR2_Y1 = ANR2_Y1;
    rSmoothANR2.med4.bits.ANR2_Y2 = ANR2_Y2;
    rSmoothANR2.med4.bits.ANR2_Y3 = ANR2_Y3;
    rSmoothANR2.med4.bits.ANR2_Y4 = ANR2_Y4;

	// CAM_ANR2_MED5
    rSmoothANR2.med5.bits.ANR2_LCL_OFT = ANR2_LCL_OFT;
	rSmoothANR2.med5.bits.ANR2_SCL_OFT = ANR2_SCL_OFT;
	rSmoothANR2.med5.bits.ANR2_NCL_OFT = ANR2_NCL_OFT;

    // CAM_ANR2_ACTC CAM+A64H
    rSmoothANR2.actc.bits.ANR2_ACT_BLD_BASE_C = ANR2_ACT_BLD_BASE_C;
	rSmoothANR2.actc.bits.ANR2_C_DITH_U = ANR2_C_DITH_U;
	rSmoothANR2.actc.bits.ANR2_C_DITH_V = ANR2_C_DITH_V;

#define ANR2_DUMP(item1, item2)   \
        do{                      \
            INTER_LOG_IF(bSmoothANRDebug, "[S, L_L, L_U, U_L, U_U]%22s = %d, %d, %d, %d, %d", #item2, rSmoothANR2.item1.bits.item2, rParam.rLowerIso_LowerZoom.item1.bits.item2, rParam.rLowerIso_UpperZoom.item1.bits.item2, rParam.rUpperIso_LowerZoom.item1.bits.item2, rParam.rUpperIso_UpperZoom.item1.bits.item2); \
        }while(0)

    ANR2_DUMP(con1, ANR2_ENC);
    ANR2_DUMP(con1, ANR2_ENY);
    ANR2_DUMP(con1, ANR2_SCALE_MODE);
    ANR2_DUMP(con1, ANR2_MODE);
    ANR2_DUMP(con1, ANR2_LCE_LINK);

    ANR2_DUMP(con2, ANR2_C_SM_EDGE);
    ANR2_DUMP(con2, ANR2_FLT_C);

    ANR2_DUMP(yad1, ANR2_K_TH_C);

    ANR2_DUMP(y4lut1, ANR2_Y_CPX1);
    ANR2_DUMP(y4lut1, ANR2_Y_CPX2);
    ANR2_DUMP(y4lut1, ANR2_Y_CPX3);

    ANR2_DUMP(y4lut2, ANR2_Y_SCALE_CPY0);
    ANR2_DUMP(y4lut2, ANR2_Y_SCALE_CPY1);
    ANR2_DUMP(y4lut2, ANR2_Y_SCALE_CPY2);
    ANR2_DUMP(y4lut2, ANR2_Y_SCALE_CPY3);

    ANR2_DUMP(y4lut3, ANR2_Y_SCALE_SP0);
    ANR2_DUMP(y4lut3, ANR2_Y_SCALE_SP1);
    ANR2_DUMP(y4lut3, ANR2_Y_SCALE_SP2);
    ANR2_DUMP(y4lut3, ANR2_Y_SCALE_SP3);

    ANR2_DUMP(l4lut1, ANR2_LCE_X1);
    ANR2_DUMP(l4lut1, ANR2_LCE_X2);
    ANR2_DUMP(l4lut1, ANR2_LCE_X3);

    ANR2_DUMP(l4lut2, ANR2_LCE_GAIN0);
    ANR2_DUMP(l4lut2, ANR2_LCE_GAIN1);
    ANR2_DUMP(l4lut2, ANR2_LCE_GAIN2);
    ANR2_DUMP(l4lut2, ANR2_LCE_GAIN3);

    ANR2_DUMP(l4lut3, ANR2_LCE_SP0);
    ANR2_DUMP(l4lut3, ANR2_LCE_SP1);
    ANR2_DUMP(l4lut3, ANR2_LCE_SP2);
    ANR2_DUMP(l4lut3, ANR2_LCE_SP3);

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

	ANR2_DUMP(med5, ANR2_LCL_OFT);
	ANR2_DUMP(med5, ANR2_SCL_OFT);
	ANR2_DUMP(med5, ANR2_NCL_OFT);

    ANR2_DUMP(actc, ANR2_ACT_BLD_BASE_C);
	ANR2_DUMP(actc, ANR2_C_DITH_U);
	ANR2_DUMP(actc, ANR2_C_DITH_V);

#undef ANR2_DUMP
}

MVOID SmoothANR_TBL(RAWIspCamInfo const& rCamInfo,
                          ISP_NVRAM_ANR_LUT_T const(& rNVRAM_ANR_TBL)[ISP_NVRAM_ANR_TBL_CT_NUM][ISP_NVRAM_ANR_TBL_LV_NUM],
                          ISP_NVRAM_ANR_LUT_T& rSmoothANR_TBL)
{
    MUINT32 u4RealLV = rCamInfo.rAEInfo.i4LightValue_x10;
	u4RealLV = 100;

	for(int i = 0; i < ANR_HW_TBL_NUM; ++i){

		rSmoothANR_TBL.lut[i].bits.ANR_TBL_Y0 =  rNVRAM_ANR_TBL[0][0].lut[i].bits.ANR_TBL_Y0;
		rSmoothANR_TBL.lut[i].bits.ANR_TBL_U0 =  rNVRAM_ANR_TBL[0][0].lut[i].bits.ANR_TBL_U0;
		rSmoothANR_TBL.lut[i].bits.ANR_TBL_V0 =  rNVRAM_ANR_TBL[0][0].lut[i].bits.ANR_TBL_V0;
		rSmoothANR_TBL.lut[i].bits.ANR_TBL_Y1 =  rNVRAM_ANR_TBL[0][0].lut[i].bits.ANR_TBL_Y1;
		rSmoothANR_TBL.lut[i].bits.ANR_TBL_U1 =  rNVRAM_ANR_TBL[0][0].lut[i].bits.ANR_TBL_U1;
		rSmoothANR_TBL.lut[i].bits.ANR_TBL_V1 =  rNVRAM_ANR_TBL[0][0].lut[i].bits.ANR_TBL_V1;
		rSmoothANR_TBL.lut[i].bits.rsv_12 = 0;
		rSmoothANR_TBL.lut[i].bits.rsv_28 = 0;

	}
}

#if 0
MVOID SmoothANR_TBL(MUINT32 u4RealISO,  // Real ISO
               MUINT32 u4UpperISO, // Upper ISO
               MUINT32 u4LowerISO, // Lower ISO
               ISP_NVRAM_ANR_LUT_T const& rUpperANR_TBL, // ANR TBL for upper ISO
               ISP_NVRAM_ANR_LUT_T const& rLowerANR_TBL,   // ANR TBL for lower ISO
               ISP_NVRAM_ANR_LUT_T& rSmoothANR_TBL)  // Output
{

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.smooth_anr_tbl.enable", value, "0");
    MBOOL bSmoothANR_TBL_Debug = atoi(value);

    INTER_LOG_IF(bSmoothANR_TBL_Debug,"[%s()] u4RealISO: %d, u4UpperISO = %d, u4LowerISO = %d\n", __FUNCTION__, u4RealISO, u4UpperISO, u4LowerISO);

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
#endif


#define CLIP_HFG(a,b,c)   ( (a<b)?  b : ((a>c)? c: a) )
MINT32 InterParam_HFG(double Ratio_L_L, double Ratio_L_U, double Ratio_U_L, double Ratio_U_U, MINT32 PARAM_L_L, MINT32 PARAM_L_U, MINT32 PARAM_U_L, MINT32 PARAM_U_U, MINT32 Method) {
    MINT32 InterPARAM = 0;


    switch(Method) {
    case 1:
        if( Ratio_L_L > Ratio_L_U)
		{
            if( Ratio_L_L > Ratio_U_L)
			{
                InterPARAM = PARAM_L_L;
        	}
        	else
			{
                InterPARAM = PARAM_U_L;
        	}
        }
        else
		{
            if( Ratio_L_L > Ratio_U_L)
			{
                InterPARAM = PARAM_L_U;
        	}
        	else
			{
                InterPARAM = PARAM_U_U;
        	}
        }
        break;
    case 0:
    default:
        InterPARAM = (MINT32)(Ratio_L_L * PARAM_L_L + Ratio_L_U * PARAM_L_U + Ratio_U_L * PARAM_U_L + Ratio_U_U * PARAM_U_U + 0.5);
        break;
    }

    return InterPARAM;
}

MVOID SmoothHFG(ISP_NVRAM_HFG_INT_T const& rParam,   // HFG settings
               ISP_NVRAM_HFG_T& rSmoothHFG)   // Output
{
    MBOOL bSmoothHFGDebug = interpPropertyGet("debug.smooth_hfg.enable", MFALSE);


    MINT32 u4RealISO = rParam.u4RealISO;
	MINT32 u4UpperISO = rParam.u4UpperISO;
	MINT32 u4LowerISO = rParam.u4LowerISO;
	MINT32 fRealZoom = rParam.i4ZoomRatio_x100;
	MINT32 fUpperZoom = rParam.u4UpperZoom;
	MINT32 fLowerZoom = rParam.u4LowerZoom;


	INTER_LOG_IF(bSmoothHFGDebug,"[%s()] u4RealISO: %d, u4UpperISO = %d, u4LowerISO = %d\n", __FUNCTION__, u4RealISO, u4UpperISO, u4LowerISO);
	INTER_LOG_IF(bSmoothHFGDebug,"[%s()] u4RealZoom: %d, u4UpperZoom = %d, u4LowerZoom = %d\n", __FUNCTION__, fRealZoom, fUpperZoom, fLowerZoom);

	// _L_L = Low ISO Low Zoom, _L_U = Low ISO Up Zoom, _U_L = Up ISO Low Zoom, _U_U = Up ISO Up Zoom

	double RATIO_ISO_lo = 0;
	double RATIO_ISO_up = 0;
	double RATIO_Zoom_lo = 0;
	double RATIO_Zoom_up = 0;

    //protection for out-of-bound


    if (u4RealISO <= u4LowerISO)
    {
        RATIO_ISO_lo = 1;
    }
	else if(u4RealISO >= u4UpperISO)
	{
		RATIO_ISO_lo = 0;
	}
	else
	{
		RATIO_ISO_lo = (double)(u4UpperISO - u4RealISO) / (double)(u4UpperISO - u4LowerISO);
	}

	RATIO_ISO_up = 1.0 - RATIO_ISO_lo;

	if (fRealZoom <= fLowerZoom)
    {
        RATIO_Zoom_lo = 1;
    }
	else if(fRealZoom >= fUpperZoom)
	{
		RATIO_Zoom_lo = 0;
	}
	else
	{
		RATIO_Zoom_lo = (double)(fUpperZoom - fRealZoom) / (double)(fUpperZoom - fLowerZoom);
	}

	RATIO_Zoom_up = 1.0 - RATIO_Zoom_lo;

	double RATIO_L_L = (double)(RATIO_ISO_lo * RATIO_Zoom_lo);
	double RATIO_L_U = (double)(RATIO_ISO_lo * RATIO_Zoom_up);
	double RATIO_U_L = (double)(RATIO_ISO_up * RATIO_Zoom_lo);
	double RATIO_U_U = (double)(RATIO_ISO_up * RATIO_Zoom_up);

    //Low ISO Low Zoom
    MINT32 DIP_HFC_STD_L_L   = rParam.rLowerIso_LowerZoom.con_0.bits.DIP_HFC_STD;
    MINT32 HFC_LUMA_CPX1_L_L = rParam.rLowerIso_LowerZoom.luma_0.bits.HFC_LUMA_CPX1;
    MINT32 HFC_LUMA_CPX2_L_L = rParam.rLowerIso_LowerZoom.luma_0.bits.HFC_LUMA_CPX2;
    MINT32 HFC_LUMA_CPX3_L_L = rParam.rLowerIso_LowerZoom.luma_0.bits.HFC_LUMA_CPX3;
    MINT32 HFC_LUMA_CPY0_L_L = rParam.rLowerIso_LowerZoom.luma_1.bits.HFC_LUMA_CPY0;
    MINT32 HFC_LUMA_CPY1_L_L = rParam.rLowerIso_LowerZoom.luma_1.bits.HFC_LUMA_CPY1;
    MINT32 HFC_LUMA_CPY2_L_L = rParam.rLowerIso_LowerZoom.luma_1.bits.HFC_LUMA_CPY2;
    MINT32 HFC_LUMA_CPY3_L_L = rParam.rLowerIso_LowerZoom.luma_1.bits.HFC_LUMA_CPY3;
    MINT32 HFC_LUMA_SP0_L_L  = rParam.rLowerIso_LowerZoom.luma_2.bits.HFC_LUMA_SP0;
    MINT32 HFC_LUMA_SP1_L_L  = rParam.rLowerIso_LowerZoom.luma_2.bits.HFC_LUMA_SP1;
    MINT32 HFC_LUMA_SP2_L_L  = rParam.rLowerIso_LowerZoom.luma_2.bits.HFC_LUMA_SP2;
    MINT32 HFC_LUMA_SP3_L_L  = rParam.rLowerIso_LowerZoom.luma_2.bits.HFC_LUMA_SP3;
    MINT32 HFC_LCE_CPX1_L_L  = rParam.rLowerIso_LowerZoom.lce_0.bits.HFC_LCE_CPX1;
    MINT32 HFC_LCE_CPX2_L_L  = rParam.rLowerIso_LowerZoom.lce_0.bits.HFC_LCE_CPX2;
    MINT32 HFC_LCE_CPX3_L_L  = rParam.rLowerIso_LowerZoom.lce_0.bits.HFC_LCE_CPX3;
    MINT32 HFC_LCE_CPY0_L_L  = rParam.rLowerIso_LowerZoom.lce_1.bits.HFC_LCE_CPY0;
    MINT32 HFC_LCE_CPY1_L_L  = rParam.rLowerIso_LowerZoom.lce_1.bits.HFC_LCE_CPY1;
    MINT32 HFC_LCE_CPY2_L_L  = rParam.rLowerIso_LowerZoom.lce_1.bits.HFC_LCE_CPY2;
    MINT32 HFC_LCE_CPY3_L_L  = rParam.rLowerIso_LowerZoom.lce_1.bits.HFC_LCE_CPY3;
    MINT32 HFC_LCE_SP0_L_L   = rParam.rLowerIso_LowerZoom.lce_2.bits.HFC_LCE_SP0;
    MINT32 HFC_LCE_SP1_L_L   = rParam.rLowerIso_LowerZoom.lce_2.bits.HFC_LCE_SP1;
    MINT32 HFC_LCE_SP2_L_L   = rParam.rLowerIso_LowerZoom.lce_2.bits.HFC_LCE_SP2;
    MINT32 HFC_LCE_SP3_L_L   = rParam.rLowerIso_LowerZoom.lce_2.bits.HFC_LCE_SP3;

	//Low ISO Up Zoom
    MINT32 DIP_HFC_STD_L_U   = rParam.rLowerIso_UpperZoom.con_0.bits.DIP_HFC_STD;
    MINT32 HFC_LUMA_CPX1_L_U = rParam.rLowerIso_UpperZoom.luma_0.bits.HFC_LUMA_CPX1;
    MINT32 HFC_LUMA_CPX2_L_U = rParam.rLowerIso_UpperZoom.luma_0.bits.HFC_LUMA_CPX2;
    MINT32 HFC_LUMA_CPX3_L_U = rParam.rLowerIso_UpperZoom.luma_0.bits.HFC_LUMA_CPX3;
    MINT32 HFC_LUMA_CPY0_L_U = rParam.rLowerIso_UpperZoom.luma_1.bits.HFC_LUMA_CPY0;
    MINT32 HFC_LUMA_CPY1_L_U = rParam.rLowerIso_UpperZoom.luma_1.bits.HFC_LUMA_CPY1;
    MINT32 HFC_LUMA_CPY2_L_U = rParam.rLowerIso_UpperZoom.luma_1.bits.HFC_LUMA_CPY2;
    MINT32 HFC_LUMA_CPY3_L_U = rParam.rLowerIso_UpperZoom.luma_1.bits.HFC_LUMA_CPY3;
    MINT32 HFC_LUMA_SP0_L_U  = rParam.rLowerIso_UpperZoom.luma_2.bits.HFC_LUMA_SP0;
    MINT32 HFC_LUMA_SP1_L_U  = rParam.rLowerIso_UpperZoom.luma_2.bits.HFC_LUMA_SP1;
    MINT32 HFC_LUMA_SP2_L_U  = rParam.rLowerIso_UpperZoom.luma_2.bits.HFC_LUMA_SP2;
    MINT32 HFC_LUMA_SP3_L_U  = rParam.rLowerIso_UpperZoom.luma_2.bits.HFC_LUMA_SP3;
    MINT32 HFC_LCE_CPX1_L_U  = rParam.rLowerIso_UpperZoom.lce_0.bits.HFC_LCE_CPX1;
    MINT32 HFC_LCE_CPX2_L_U  = rParam.rLowerIso_UpperZoom.lce_0.bits.HFC_LCE_CPX2;
    MINT32 HFC_LCE_CPX3_L_U  = rParam.rLowerIso_UpperZoom.lce_0.bits.HFC_LCE_CPX3;
    MINT32 HFC_LCE_CPY0_L_U  = rParam.rLowerIso_UpperZoom.lce_1.bits.HFC_LCE_CPY0;
    MINT32 HFC_LCE_CPY1_L_U  = rParam.rLowerIso_UpperZoom.lce_1.bits.HFC_LCE_CPY1;
    MINT32 HFC_LCE_CPY2_L_U  = rParam.rLowerIso_UpperZoom.lce_1.bits.HFC_LCE_CPY2;
    MINT32 HFC_LCE_CPY3_L_U  = rParam.rLowerIso_UpperZoom.lce_1.bits.HFC_LCE_CPY3;
    MINT32 HFC_LCE_SP0_L_U   = rParam.rLowerIso_UpperZoom.lce_2.bits.HFC_LCE_SP0;
    MINT32 HFC_LCE_SP1_L_U   = rParam.rLowerIso_UpperZoom.lce_2.bits.HFC_LCE_SP1;
    MINT32 HFC_LCE_SP2_L_U   = rParam.rLowerIso_UpperZoom.lce_2.bits.HFC_LCE_SP2;
    MINT32 HFC_LCE_SP3_L_U   = rParam.rLowerIso_UpperZoom.lce_2.bits.HFC_LCE_SP3;

	//Up ISO Low Zoom
    MINT32 DIP_HFC_STD_U_L   = rParam.rUpperIso_LowerZoom.con_0.bits.DIP_HFC_STD;
    MINT32 HFC_LUMA_CPX1_U_L = rParam.rUpperIso_LowerZoom.luma_0.bits.HFC_LUMA_CPX1;
    MINT32 HFC_LUMA_CPX2_U_L = rParam.rUpperIso_LowerZoom.luma_0.bits.HFC_LUMA_CPX2;
    MINT32 HFC_LUMA_CPX3_U_L = rParam.rUpperIso_LowerZoom.luma_0.bits.HFC_LUMA_CPX3;
    MINT32 HFC_LUMA_CPY0_U_L = rParam.rUpperIso_LowerZoom.luma_1.bits.HFC_LUMA_CPY0;
    MINT32 HFC_LUMA_CPY1_U_L = rParam.rUpperIso_LowerZoom.luma_1.bits.HFC_LUMA_CPY1;
    MINT32 HFC_LUMA_CPY2_U_L = rParam.rUpperIso_LowerZoom.luma_1.bits.HFC_LUMA_CPY2;
    MINT32 HFC_LUMA_CPY3_U_L = rParam.rUpperIso_LowerZoom.luma_1.bits.HFC_LUMA_CPY3;
    MINT32 HFC_LUMA_SP0_U_L  = rParam.rUpperIso_LowerZoom.luma_2.bits.HFC_LUMA_SP0;
    MINT32 HFC_LUMA_SP1_U_L  = rParam.rUpperIso_LowerZoom.luma_2.bits.HFC_LUMA_SP1;
    MINT32 HFC_LUMA_SP2_U_L  = rParam.rUpperIso_LowerZoom.luma_2.bits.HFC_LUMA_SP2;
    MINT32 HFC_LUMA_SP3_U_L  = rParam.rUpperIso_LowerZoom.luma_2.bits.HFC_LUMA_SP3;
    MINT32 HFC_LCE_CPX1_U_L  = rParam.rUpperIso_LowerZoom.lce_0.bits.HFC_LCE_CPX1;
    MINT32 HFC_LCE_CPX2_U_L  = rParam.rUpperIso_LowerZoom.lce_0.bits.HFC_LCE_CPX2;
    MINT32 HFC_LCE_CPX3_U_L  = rParam.rUpperIso_LowerZoom.lce_0.bits.HFC_LCE_CPX3;
    MINT32 HFC_LCE_CPY0_U_L  = rParam.rUpperIso_LowerZoom.lce_1.bits.HFC_LCE_CPY0;
    MINT32 HFC_LCE_CPY1_U_L  = rParam.rUpperIso_LowerZoom.lce_1.bits.HFC_LCE_CPY1;
    MINT32 HFC_LCE_CPY2_U_L  = rParam.rUpperIso_LowerZoom.lce_1.bits.HFC_LCE_CPY2;
    MINT32 HFC_LCE_CPY3_U_L  = rParam.rUpperIso_LowerZoom.lce_1.bits.HFC_LCE_CPY3;
    MINT32 HFC_LCE_SP0_U_L   = rParam.rUpperIso_LowerZoom.lce_2.bits.HFC_LCE_SP0;
    MINT32 HFC_LCE_SP1_U_L   = rParam.rUpperIso_LowerZoom.lce_2.bits.HFC_LCE_SP1;
    MINT32 HFC_LCE_SP2_U_L   = rParam.rUpperIso_LowerZoom.lce_2.bits.HFC_LCE_SP2;
    MINT32 HFC_LCE_SP3_U_L   = rParam.rUpperIso_LowerZoom.lce_2.bits.HFC_LCE_SP3;

	//Up ISO Up Zoom
    MINT32 DIP_HFC_STD_U_U   = rParam.rUpperIso_UpperZoom.con_0.bits.DIP_HFC_STD;
    MINT32 HFC_LUMA_CPX1_U_U = rParam.rUpperIso_UpperZoom.luma_0.bits.HFC_LUMA_CPX1;
    MINT32 HFC_LUMA_CPX2_U_U = rParam.rUpperIso_UpperZoom.luma_0.bits.HFC_LUMA_CPX2;
    MINT32 HFC_LUMA_CPX3_U_U = rParam.rUpperIso_UpperZoom.luma_0.bits.HFC_LUMA_CPX3;
    MINT32 HFC_LUMA_CPY0_U_U = rParam.rUpperIso_UpperZoom.luma_1.bits.HFC_LUMA_CPY0;
    MINT32 HFC_LUMA_CPY1_U_U = rParam.rUpperIso_UpperZoom.luma_1.bits.HFC_LUMA_CPY1;
    MINT32 HFC_LUMA_CPY2_U_U = rParam.rUpperIso_UpperZoom.luma_1.bits.HFC_LUMA_CPY2;
    MINT32 HFC_LUMA_CPY3_U_U = rParam.rUpperIso_UpperZoom.luma_1.bits.HFC_LUMA_CPY3;
    MINT32 HFC_LUMA_SP0_U_U  = rParam.rUpperIso_UpperZoom.luma_2.bits.HFC_LUMA_SP0;
    MINT32 HFC_LUMA_SP1_U_U  = rParam.rUpperIso_UpperZoom.luma_2.bits.HFC_LUMA_SP1;
    MINT32 HFC_LUMA_SP2_U_U  = rParam.rUpperIso_UpperZoom.luma_2.bits.HFC_LUMA_SP2;
    MINT32 HFC_LUMA_SP3_U_U  = rParam.rUpperIso_UpperZoom.luma_2.bits.HFC_LUMA_SP3;
    MINT32 HFC_LCE_CPX1_U_U  = rParam.rUpperIso_UpperZoom.lce_0.bits.HFC_LCE_CPX1;
    MINT32 HFC_LCE_CPX2_U_U  = rParam.rUpperIso_UpperZoom.lce_0.bits.HFC_LCE_CPX2;
    MINT32 HFC_LCE_CPX3_U_U  = rParam.rUpperIso_UpperZoom.lce_0.bits.HFC_LCE_CPX3;
    MINT32 HFC_LCE_CPY0_U_U  = rParam.rUpperIso_UpperZoom.lce_1.bits.HFC_LCE_CPY0;
    MINT32 HFC_LCE_CPY1_U_U  = rParam.rUpperIso_UpperZoom.lce_1.bits.HFC_LCE_CPY1;
    MINT32 HFC_LCE_CPY2_U_U  = rParam.rUpperIso_UpperZoom.lce_1.bits.HFC_LCE_CPY2;
    MINT32 HFC_LCE_CPY3_U_U  = rParam.rUpperIso_UpperZoom.lce_1.bits.HFC_LCE_CPY3;
    MINT32 HFC_LCE_SP0_U_U   = rParam.rUpperIso_UpperZoom.lce_2.bits.HFC_LCE_SP0;
    MINT32 HFC_LCE_SP1_U_U   = rParam.rUpperIso_UpperZoom.lce_2.bits.HFC_LCE_SP1;
    MINT32 HFC_LCE_SP2_U_U   = rParam.rUpperIso_UpperZoom.lce_2.bits.HFC_LCE_SP2;
    MINT32 HFC_LCE_SP3_U_U   = rParam.rUpperIso_UpperZoom.lce_2.bits.HFC_LCE_SP3;




    MINT32 DIP_HFC_STD   = InterParam_HFG(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, DIP_HFC_STD_L_L, DIP_HFC_STD_L_U, DIP_HFC_STD_U_L, DIP_HFC_STD_U_U,0);
    MINT32 HFC_LUMA_CPX1 = InterParam_HFG(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, HFC_LUMA_CPX1_L_L, HFC_LUMA_CPX1_L_U, HFC_LUMA_CPX1_U_L, HFC_LUMA_CPX1_U_U,0);
    MINT32 HFC_LUMA_CPX2 = InterParam_HFG(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, HFC_LUMA_CPX2_L_L, HFC_LUMA_CPX2_L_U, HFC_LUMA_CPX2_U_L, HFC_LUMA_CPX2_U_U,0);
    MINT32 HFC_LUMA_CPX3 = InterParam_HFG(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, HFC_LUMA_CPX3_L_L, HFC_LUMA_CPX3_L_U, HFC_LUMA_CPX3_U_L, HFC_LUMA_CPX3_U_U,0);
    MINT32 HFC_LUMA_CPY0 = InterParam_HFG(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, HFC_LUMA_CPY0_L_L, HFC_LUMA_CPY0_L_U, HFC_LUMA_CPY0_U_L, HFC_LUMA_CPY0_U_U,0);
    MINT32 HFC_LUMA_CPY1 = InterParam_HFG(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, HFC_LUMA_CPY1_L_L, HFC_LUMA_CPY1_L_U, HFC_LUMA_CPY1_U_L, HFC_LUMA_CPY1_U_U,0);
    MINT32 HFC_LUMA_CPY2 = InterParam_HFG(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, HFC_LUMA_CPY2_L_L, HFC_LUMA_CPY2_L_U, HFC_LUMA_CPY2_U_L, HFC_LUMA_CPY2_U_U,0);
    MINT32 HFC_LUMA_CPY3 = InterParam_HFG(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, HFC_LUMA_CPY3_L_L, HFC_LUMA_CPY3_L_U, HFC_LUMA_CPY3_U_L, HFC_LUMA_CPY3_U_U,0);
    MINT32 HFC_LCE_CPX1  = InterParam_HFG(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, HFC_LCE_CPX1_L_L, HFC_LCE_CPX1_L_U, HFC_LCE_CPX1_U_L, HFC_LCE_CPX1_U_U,0);
    MINT32 HFC_LCE_CPX2  = InterParam_HFG(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, HFC_LCE_CPX2_L_L, HFC_LCE_CPX2_L_U, HFC_LCE_CPX2_U_L, HFC_LCE_CPX2_U_U,0);
    MINT32 HFC_LCE_CPX3  = InterParam_HFG(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, HFC_LCE_CPX3_L_L, HFC_LCE_CPX3_L_U, HFC_LCE_CPX3_U_L, HFC_LCE_CPX3_U_U,0);
    MINT32 HFC_LCE_CPY0  = InterParam_HFG(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, HFC_LCE_CPY0_L_L, HFC_LCE_CPY0_L_U, HFC_LCE_CPY0_U_L, HFC_LCE_CPY0_U_U,0);
    MINT32 HFC_LCE_CPY1  = InterParam_HFG(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, HFC_LCE_CPY1_L_L, HFC_LCE_CPY1_L_U, HFC_LCE_CPY1_U_L, HFC_LCE_CPY1_U_U,0);
    MINT32 HFC_LCE_CPY2  = InterParam_HFG(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, HFC_LCE_CPY2_L_L, HFC_LCE_CPY2_L_U, HFC_LCE_CPY2_U_L, HFC_LCE_CPY2_U_U,0);
    MINT32 HFC_LCE_CPY3  = InterParam_HFG(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, HFC_LCE_CPY3_L_L, HFC_LCE_CPY3_L_U, HFC_LCE_CPY3_U_L, HFC_LCE_CPY3_U_U,0);

	MINT32 HFC_LUMA_SP0;
	MINT32 HFC_LUMA_SP1;
	MINT32 HFC_LUMA_SP2;
	MINT32 HFC_LUMA_SP3;

	MINT32 HFC_LCE_SP0;
	MINT32 HFC_LCE_SP1;
	MINT32 HFC_LCE_SP2;
	MINT32 HFC_LCE_SP3;

	//Slope Calculation

	HFC_LUMA_CPX1 = (HFC_LUMA_CPX1 < 1) ? 1 : HFC_LUMA_CPX1;
	HFC_LUMA_CPX2 = (HFC_LUMA_CPX2 < HFC_LUMA_CPX1 + 1) ? HFC_LUMA_CPX1 + 1 : HFC_LUMA_CPX2;
	HFC_LUMA_CPX3 = (HFC_LUMA_CPX3 < HFC_LUMA_CPX2 + 1) ? HFC_LUMA_CPX2 + 1 : HFC_LUMA_CPX3;

	HFC_LUMA_SP0 = ((HFC_LUMA_CPY1 - HFC_LUMA_CPY0) * 64) / HFC_LUMA_CPX1 ;
	HFC_LUMA_SP1 = ((HFC_LUMA_CPY2 - HFC_LUMA_CPY1) * 64) / (HFC_LUMA_CPX2 - HFC_LUMA_CPX1) ;
	HFC_LUMA_SP2 = ((HFC_LUMA_CPY3 - HFC_LUMA_CPY2) * 64) / (HFC_LUMA_CPX3 - HFC_LUMA_CPX2) ;
	HFC_LUMA_SP3 = HFC_LUMA_SP2 ;

	HFC_LCE_CPX1 = (HFC_LCE_CPX1 < 1) ? 1 : HFC_LCE_CPX1;
	HFC_LCE_CPX2 = (HFC_LCE_CPX2 < HFC_LCE_CPX1 + 1) ? HFC_LCE_CPX1 + 1 : HFC_LCE_CPX2;
	HFC_LCE_CPX3 = (HFC_LCE_CPX3 < HFC_LCE_CPX2 + 1) ? HFC_LCE_CPX2 + 1 : HFC_LCE_CPX3;

	HFC_LCE_SP0 = ((HFC_LCE_CPY1 - HFC_LCE_CPY0) * 64) / HFC_LCE_CPX1 ;
	HFC_LCE_SP1 = ((HFC_LCE_CPY2 - HFC_LCE_CPY1) * 64) / (HFC_LCE_CPX2 - HFC_LCE_CPX1) ;
	HFC_LCE_SP2 = ((HFC_LCE_CPY3 - HFC_LCE_CPY2) * 64) / (HFC_LCE_CPX3 - HFC_LCE_CPX2) ;
	HFC_LCE_SP3 = HFC_LCE_SP2 ;

	HFC_LUMA_SP0 = CLIP_HFG(HFC_LUMA_SP0,-15,15);
	HFC_LUMA_SP1 = CLIP_HFG(HFC_LUMA_SP1,-15,15);
	HFC_LUMA_SP2 = CLIP_HFG(HFC_LUMA_SP2,-15,15);
	HFC_LUMA_SP3 = CLIP_HFG(HFC_LUMA_SP3,-15,15);
	HFC_LCE_SP0 = CLIP_HFG(HFC_LCE_SP0,-15,15);
	HFC_LCE_SP1 = CLIP_HFG(HFC_LCE_SP1,-15,15);
	HFC_LCE_SP2 = CLIP_HFG(HFC_LCE_SP2,-15,15);
	HFC_LCE_SP3 = CLIP_HFG(HFC_LCE_SP3,-15,15);

	HFC_LUMA_SP0 = (HFC_LUMA_SP0 < 0) ? HFC_LUMA_SP0 + 32 : HFC_LUMA_SP0;
	HFC_LUMA_SP1 = (HFC_LUMA_SP1 < 0) ? HFC_LUMA_SP1 + 32 : HFC_LUMA_SP1;
	HFC_LUMA_SP2 = (HFC_LUMA_SP2 < 0) ? HFC_LUMA_SP2 + 32 : HFC_LUMA_SP2;
	HFC_LUMA_SP3 = (HFC_LUMA_SP3 < 0) ? HFC_LUMA_SP3 + 32 : HFC_LUMA_SP3;
	HFC_LCE_SP0 = (HFC_LCE_SP0 < 0) ? HFC_LCE_SP0 + 32 : HFC_LUMA_SP0;
	HFC_LCE_SP1 = (HFC_LCE_SP1 < 0) ? HFC_LCE_SP1 + 32 : HFC_LUMA_SP1;
	HFC_LCE_SP2 = (HFC_LCE_SP2 < 0) ? HFC_LCE_SP2 + 32 : HFC_LUMA_SP2;
	HFC_LCE_SP3 = (HFC_LCE_SP3 < 0) ? HFC_LCE_SP3 + 32 : HFC_LUMA_SP3;


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
            INTER_LOG_IF(bSmoothHFGDebug, "[S, L_L, L_U, U_L, U_U]%22s = %d, %d, %d, %d, %d", #item2, rSmoothHFG.item1.bits.item2, rParam.rLowerIso_LowerZoom.item1.bits.item2, rParam.rLowerIso_UpperZoom.item1.bits.item2, rParam.rUpperIso_LowerZoom.item1.bits.item2, rParam.rUpperIso_UpperZoom.item1.bits.item2); \
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
    MBOOL bSmoothCCRDebug = interpPropertyGet("debug.smooth_ccr.enable", MFALSE);

    INTER_LOG_IF(bSmoothCCRDebug,"[%s()] u4RealISO: %d, u4UpperISO = %d, u4LowerISO = %d\n", __FUNCTION__, u4RealISO, u4UpperISO, u4LowerISO);

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

    INTER_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_EN           = %d, %d, %d", rLowerCCR.con.bits.CCR_EN            , rSmoothCCR.con.bits.CCR_EN           , rUpperCCR.con.bits.CCR_EN   );
    INTER_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_OR_MODE      = %d, %d, %d", rLowerCCR.con.bits.CCR_OR_MODE       , rSmoothCCR.con.bits.CCR_OR_MODE      , rUpperCCR.con.bits.CCR_OR_MODE   );
    INTER_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_UV_GAIN_MODE = %d, %d, %d", rLowerCCR.con.bits.CCR_UV_GAIN_MODE  , rSmoothCCR.con.bits.CCR_UV_GAIN_MODE , rUpperCCR.con.bits.CCR_UV_GAIN_MODE   );
    INTER_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_UV_GAIN2     = %d, %d, %d", rLowerCCR.con.bits.CCR_UV_GAIN2      , rSmoothCCR.con.bits.CCR_UV_GAIN2     , rUpperCCR.con.bits.CCR_UV_GAIN2   );
    INTER_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_Y_CPX3       = %d, %d, %d", rLowerCCR.con.bits.CCR_Y_CPX3        , rSmoothCCR.con.bits.CCR_Y_CPX3       , rUpperCCR.con.bits.CCR_Y_CPX3   );
    INTER_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_Y_CPX1       = %d, %d, %d", rLowerCCR.ylut.bits.CCR_Y_CPX1        , rSmoothCCR.ylut.bits.CCR_Y_CPX1       , rUpperCCR.ylut.bits.CCR_Y_CPX1   );
    INTER_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_Y_CPX2       = %d, %d, %d", rLowerCCR.ylut.bits.CCR_Y_CPX2        , rSmoothCCR.ylut.bits.CCR_Y_CPX2       , rUpperCCR.ylut.bits.CCR_Y_CPX2   );
    INTER_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_Y_SP1        = %d, %d, %d", rLowerCCR.ylut.bits.CCR_Y_SP1         , rSmoothCCR.ylut.bits.CCR_Y_SP1        , rUpperCCR.ylut.bits.CCR_Y_SP1   );
    INTER_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_Y_CPY1       = %d, %d, %d", rLowerCCR.ylut.bits.CCR_Y_CPY1        , rSmoothCCR.ylut.bits.CCR_Y_CPY1       , rUpperCCR.ylut.bits.CCR_Y_CPY1   );
    INTER_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_UV_X1        = %d, %d, %d", rLowerCCR.uvlut.bits.CCR_UV_X1         , rSmoothCCR.uvlut.bits.CCR_UV_X1        , rUpperCCR.uvlut.bits.CCR_UV_X1   );
    INTER_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_UV_X2        = %d, %d, %d", rLowerCCR.uvlut.bits.CCR_UV_X2         , rSmoothCCR.uvlut.bits.CCR_UV_X2        , rUpperCCR.uvlut.bits.CCR_UV_X2   );
    INTER_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_UV_X3        = %d, %d, %d", rLowerCCR.uvlut.bits.CCR_UV_X3         , rSmoothCCR.uvlut.bits.CCR_UV_X3        , rUpperCCR.uvlut.bits.CCR_UV_X3   );
    INTER_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_UV_GAIN1     = %d, %d, %d", rLowerCCR.uvlut.bits.CCR_UV_GAIN1      , rSmoothCCR.uvlut.bits.CCR_UV_GAIN1     , rUpperCCR.uvlut.bits.CCR_UV_GAIN1   );
    INTER_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_Y_SP0        = %d, %d, %d", rLowerCCR.ylut2.bits.CCR_Y_SP0         , rSmoothCCR.ylut2.bits.CCR_Y_SP0        , rUpperCCR.ylut2.bits.CCR_Y_SP0   );
    INTER_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_Y_SP2        = %d, %d, %d", rLowerCCR.ylut2.bits.CCR_Y_SP2         , rSmoothCCR.ylut2.bits.CCR_Y_SP2        , rUpperCCR.ylut2.bits.CCR_Y_SP2   );
    INTER_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_Y_CPY0       = %d, %d, %d", rLowerCCR.ylut2.bits.CCR_Y_CPY0        , rSmoothCCR.ylut2.bits.CCR_Y_CPY0       , rUpperCCR.ylut2.bits.CCR_Y_CPY0   );
    INTER_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_Y_CPY2       = %d, %d, %d", rLowerCCR.ylut2.bits.CCR_Y_CPY2        , rSmoothCCR.ylut2.bits.CCR_Y_CPY2       , rUpperCCR.ylut2.bits.CCR_Y_CPY2   );
    INTER_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_MODE         = %d, %d, %d", rLowerCCR.sat_ctrl.bits.CCR_MODE          , rSmoothCCR.sat_ctrl.bits.CCR_MODE         , rUpperCCR.sat_ctrl.bits.CCR_MODE   );
    INTER_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_CEN_U        = %d, %d, %d", rLowerCCR.sat_ctrl.bits.CCR_CEN_U         , rSmoothCCR.sat_ctrl.bits.CCR_CEN_U        , rUpperCCR.sat_ctrl.bits.CCR_CEN_U   );
    INTER_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_CEN_V        = %d, %d, %d", rLowerCCR.sat_ctrl.bits.CCR_CEN_V         , rSmoothCCR.sat_ctrl.bits.CCR_CEN_V        , rUpperCCR.sat_ctrl.bits.CCR_CEN_V   );
    INTER_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_UV_GAIN_SP1  = %d, %d, %d", rLowerCCR.uvlut_sp.bits.CCR_UV_GAIN_SP1   , rSmoothCCR.uvlut_sp.bits.CCR_UV_GAIN_SP1  , rUpperCCR.uvlut_sp.bits.CCR_UV_GAIN_SP1   );
    INTER_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_UV_GAIN_SP2  = %d, %d, %d", rLowerCCR.uvlut_sp.bits.CCR_UV_GAIN_SP2   , rSmoothCCR.uvlut_sp.bits.CCR_UV_GAIN_SP2  , rUpperCCR.uvlut_sp.bits.CCR_UV_GAIN_SP2   );
    INTER_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_HUE_X1       = %d, %d, %d", rLowerCCR.hue1.bits.CCR_HUE_X1        , rSmoothCCR.hue1.bits.CCR_HUE_X1       , rUpperCCR.hue1.bits.CCR_HUE_X1   );
    INTER_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_HUE_X2       = %d, %d, %d", rLowerCCR.hue1.bits.CCR_HUE_X2        , rSmoothCCR.hue1.bits.CCR_HUE_X2       , rUpperCCR.hue1.bits.CCR_HUE_X2   );
    INTER_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_HUE_X3       = %d, %d, %d", rLowerCCR.hue2.bits.CCR_HUE_X3        , rSmoothCCR.hue2.bits.CCR_HUE_X3       , rUpperCCR.hue2.bits.CCR_HUE_X3   );
    INTER_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_HUE_X4       = %d, %d, %d", rLowerCCR.hue2.bits.CCR_HUE_X4        , rSmoothCCR.hue2.bits.CCR_HUE_X4       , rUpperCCR.hue2.bits.CCR_HUE_X4   );
    INTER_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_HUE_GAIN1    = %d, %d, %d", rLowerCCR.hue3.bits.CCR_HUE_GAIN1     , rSmoothCCR.hue3.bits.CCR_HUE_GAIN1    , rUpperCCR.hue3.bits.CCR_HUE_GAIN1   );
    INTER_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_HUE_GAIN2    = %d, %d, %d", rLowerCCR.hue3.bits.CCR_HUE_GAIN2     , rSmoothCCR.hue3.bits.CCR_HUE_GAIN2    , rUpperCCR.hue3.bits.CCR_HUE_GAIN2   );
}

MINT32 InterParam_EE(double Ratio_L_L, double Ratio_L_U, double Ratio_U_L, double Ratio_U_U, MINT32 PARAM_L_L, MINT32 PARAM_L_U, MINT32 PARAM_U_L, MINT32 PARAM_U_U, MINT32 Method) {
    MINT32 InterPARAM = 0;


    switch(Method) {
    case 1:
        if( Ratio_L_L > Ratio_L_U)
		{
            if( Ratio_L_L > Ratio_U_L)
			{
                InterPARAM = PARAM_L_L;
        	}
        	else
			{
                InterPARAM = PARAM_U_L;
        	}
        }
        else
		{
            if( Ratio_L_L > Ratio_U_L)
			{
                InterPARAM = PARAM_L_U;
        	}
        	else
			{
                InterPARAM = PARAM_U_U;
        	}
        }
        break;
    case 0:
    default:
        InterPARAM = (MINT32)(Ratio_L_L * PARAM_L_L + Ratio_L_U * PARAM_L_U + Ratio_U_L * PARAM_U_L + Ratio_U_U * PARAM_U_U + 0.5);
        break;
    }

    return InterPARAM;
}


MVOID SmoothEE(ISP_NVRAM_EE_INT_T const& rParam,   // EE settings
               ISP_NVRAM_EE_T& rSmoothEE)
{       // Output
    MBOOL bSmoothEEDebug = interpPropertyGet("debug.smooth_ee.enable", MFALSE);

    MINT32 u4RealISO = rParam.u4RealISO;
	MINT32 u4UpperISO = rParam.u4UpperISO;
	MINT32 u4LowerISO = rParam.u4LowerISO;
	MINT32 fRealZoom = rParam.i4ZoomRatio_x100;
	MINT32 fUpperZoom = rParam.u4UpperZoom;
	MINT32 fLowerZoom = rParam.u4LowerZoom;


	INTER_LOG_IF(bSmoothEEDebug,"[%s()] u4RealISO: %d, u4UpperISO = %d, u4LowerISO = %d\n", __FUNCTION__, u4RealISO, u4UpperISO, u4LowerISO);
	INTER_LOG_IF(bSmoothEEDebug,"[%s()] u4RealZoom: %d, u4UpperZoom = %d, u4LowerZoom = %d\n", __FUNCTION__, fRealZoom, fUpperZoom, fLowerZoom);

	// _L_L = Low ISO Low Zoom, _L_U = Low ISO Up Zoom, _U_L = Up ISO Low Zoom, _U_U = Up ISO Up Zoom

	double RATIO_ISO_lo = 0;
	double RATIO_ISO_up = 0;
	double RATIO_Zoom_lo = 0;
	double RATIO_Zoom_up = 0;

    //protection for out-of-bound


    if (u4RealISO <= u4LowerISO)
    {
        RATIO_ISO_lo = 1;
    }
	else if(u4RealISO >= u4UpperISO)
	{
		RATIO_ISO_lo = 0;
	}
	else
	{
		RATIO_ISO_lo = (double)(u4UpperISO - u4RealISO) / (double)(u4UpperISO - u4LowerISO);
	}

	RATIO_ISO_up = 1.0 - RATIO_ISO_lo;

	if (fRealZoom <= fLowerZoom)
    {
        RATIO_Zoom_lo = 1;
    }
	else if(fRealZoom >= fUpperZoom)
	{
		RATIO_Zoom_lo = 0;
	}
	else
	{
		RATIO_Zoom_lo = (double)(fUpperZoom - fRealZoom) / (double)(fUpperZoom - fLowerZoom);
	}

	RATIO_Zoom_up = 1.0 - RATIO_Zoom_lo;

	double RATIO_L_L = (double)(RATIO_ISO_lo * RATIO_Zoom_lo);
	double RATIO_L_U = (double)(RATIO_ISO_lo * RATIO_Zoom_up);
	double RATIO_U_L = (double)(RATIO_ISO_up * RATIO_Zoom_lo);
	double RATIO_U_U = (double)(RATIO_ISO_up * RATIO_Zoom_up);

    //  ====================================================================================================================================
    //  Get Lower ISO Low Zoom
    //  ====================================================================================================================================
    MINT32 SEEE_H1_DI_BLND_OFST_L_L          = rParam.rLowerIso_LowerZoom.blnd_ctrl_1.bits.SEEE_H1_DI_BLND_OFST;
    MINT32 SEEE_H2_DI_BLND_OFST_L_L          = rParam.rLowerIso_LowerZoom.blnd_ctrl_1.bits.SEEE_H2_DI_BLND_OFST;
    MINT32 SEEE_H3_DI_BLND_OFST_L_L          = rParam.rLowerIso_LowerZoom.blnd_ctrl_1.bits.SEEE_H3_DI_BLND_OFST;
    MINT32 SEEE_H1_DI_BLND_SL_L_L            = rParam.rLowerIso_LowerZoom.blnd_ctrl_2.bits.SEEE_H1_DI_BLND_SL;
    MINT32 SEEE_H2_DI_BLND_SL_L_L            = rParam.rLowerIso_LowerZoom.blnd_ctrl_2.bits.SEEE_H2_DI_BLND_SL;
    MINT32 SEEE_H3_DI_BLND_SL_L_L            = rParam.rLowerIso_LowerZoom.blnd_ctrl_2.bits.SEEE_H3_DI_BLND_SL;
    MINT32 SEEE_HX_ISO_BLND_RAT_L_L          = rParam.rLowerIso_LowerZoom.blnd_ctrl_2.bits.SEEE_HX_ISO_BLND_RAT;
    MINT32 SEEE_H1_GN_L_L                    = rParam.rLowerIso_LowerZoom.gn_ctrl.bits.SEEE_H1_GN;
    MINT32 SEEE_H2_GN_L_L                    = rParam.rLowerIso_LowerZoom.gn_ctrl.bits.SEEE_H2_GN;
    MINT32 SEEE_H3_GN_L_L                    = rParam.rLowerIso_LowerZoom.gn_ctrl.bits.SEEE_H3_GN;
    MINT32 SEEE_H4_GN_L_L                    = rParam.rLowerIso_LowerZoom.gn_ctrl.bits.SEEE_H4_GN;
    MINT32 SEEE_FLT_CORE_TH_L_L              = rParam.rLowerIso_LowerZoom.clip_ctrl_3.bits.SEEE_FLT_CORE_TH;
    MINT32 SEEE_LUMA_MOD_Y0_L_L              = rParam.rLowerIso_LowerZoom.luma_ctrl_1.bits.SEEE_LUMA_MOD_Y0;
    MINT32 SEEE_LUMA_MOD_Y1_L_L              = rParam.rLowerIso_LowerZoom.luma_ctrl_1.bits.SEEE_LUMA_MOD_Y1;
    MINT32 SEEE_LUMA_MOD_Y2_L_L              = rParam.rLowerIso_LowerZoom.luma_ctrl_2.bits.SEEE_LUMA_MOD_Y2;
    MINT32 SEEE_LUMA_MOD_Y3_L_L              = rParam.rLowerIso_LowerZoom.luma_ctrl_2.bits.SEEE_LUMA_MOD_Y3;
    MINT32 SEEE_LUMA_MOD_Y4_L_L              = rParam.rLowerIso_LowerZoom.luma_ctrl_3.bits.SEEE_LUMA_MOD_Y4;
    MINT32 SEEE_LUMA_MOD_Y5_L_L              = rParam.rLowerIso_LowerZoom.luma_ctrl_3.bits.SEEE_LUMA_MOD_Y5;
    MINT32 SEEE_LUMA_MOD_Y6_L_L              = rParam.rLowerIso_LowerZoom.luma_ctrl_4.bits.SEEE_LUMA_MOD_Y6;
    MINT32 SEEE_SLNK_GN_Y1_L_L               = rParam.rLowerIso_LowerZoom.slnk_ctrl_1.bits.SEEE_SLNK_GN_Y1;
    MINT32 SEEE_SLNK_GN_Y2_L_L               = rParam.rLowerIso_LowerZoom.slnk_ctrl_1.bits.SEEE_SLNK_GN_Y2;
    MINT32 SEEE_RESP_SLNK_GN_RAT_L_L         = rParam.rLowerIso_LowerZoom.slnk_ctrl_2.bits.SEEE_RESP_SLNK_GN_RAT;
    MINT32 SEEE_GLUT_X1_L_L                  = rParam.rLowerIso_LowerZoom.glut_ctrl_1.bits.SEEE_GLUT_X1;
    MINT32 SEEE_GLUT_S1_L_L                  = rParam.rLowerIso_LowerZoom.glut_ctrl_1.bits.SEEE_GLUT_S1;
    MINT32 SEEE_GLUT_Y1_L_L                  = rParam.rLowerIso_LowerZoom.glut_ctrl_1.bits.SEEE_GLUT_Y1;
    MINT32 SEEE_GLUT_X2_L_L                  = rParam.rLowerIso_LowerZoom.glut_ctrl_2.bits.SEEE_GLUT_X2;
    MINT32 SEEE_GLUT_S2_L_L                  = rParam.rLowerIso_LowerZoom.glut_ctrl_2.bits.SEEE_GLUT_S2;
    MINT32 SEEE_GLUT_Y2_L_L                  = rParam.rLowerIso_LowerZoom.glut_ctrl_2.bits.SEEE_GLUT_Y2;
    MINT32 SEEE_GLUT_X3_L_L                  = rParam.rLowerIso_LowerZoom.glut_ctrl_3.bits.SEEE_GLUT_X3;
    MINT32 SEEE_GLUT_S3_L_L                  = rParam.rLowerIso_LowerZoom.glut_ctrl_3.bits.SEEE_GLUT_S3;
    MINT32 SEEE_GLUT_Y3_L_L                  = rParam.rLowerIso_LowerZoom.glut_ctrl_3.bits.SEEE_GLUT_Y3;
    MINT32 SEEE_GLUT_X4_L_L                  = rParam.rLowerIso_LowerZoom.glut_ctrl_4.bits.SEEE_GLUT_X4;
    MINT32 SEEE_GLUT_S4_L_L                  = rParam.rLowerIso_LowerZoom.glut_ctrl_4.bits.SEEE_GLUT_S4;
    MINT32 SEEE_GLUT_Y4_L_L                  = rParam.rLowerIso_LowerZoom.glut_ctrl_4.bits.SEEE_GLUT_Y4;
    MINT32 SEEE_GLUT_S5_L_L                  = rParam.rLowerIso_LowerZoom.glut_ctrl_5.bits.SEEE_GLUT_S5;
    MINT32 SEEE_GLUT_TH_OVR_L_L              = rParam.rLowerIso_LowerZoom.glut_ctrl_6.bits.SEEE_GLUT_TH_OVR;
    MINT32 SEEE_GLUT_TH_UND_L_L              = rParam.rLowerIso_LowerZoom.glut_ctrl_6.bits.SEEE_GLUT_TH_UND;
    MINT32 SEEE_GLUT_TH_MIN_L_L              = rParam.rLowerIso_LowerZoom.glut_ctrl_6.bits.SEEE_GLUT_TH_MIN;
    MINT32 SEEE_GLUT_LINK_EN_L_L             = rParam.rLowerIso_LowerZoom.slnk_ctrl_2.bits.SEEE_GLUT_LINK_EN;
    MINT32 SEEE_GLUT_SL_DEC_Y_L_L            = rParam.rLowerIso_LowerZoom.slnk_ctrl_2.bits.SEEE_GLUT_SL_DEC_Y;
    MINT32 SEEE_OVRSH_CLIP_STR_L_L           = rParam.rLowerIso_LowerZoom.clip_ctrl_3.bits.SEEE_OVRSH_CLIP_STR;
    MINT32 SEEE_DOT_TH_L_L                   = rParam.rLowerIso_LowerZoom.clip_ctrl_1.bits.SEEE_DOT_TH;
    MINT32 SEEE_DOT_REDUC_AMNT_L_L           = rParam.rLowerIso_LowerZoom.clip_ctrl_1.bits.SEEE_DOT_REDUC_AMNT;
    MINT32 SEEE_RESP_CLIP_L_L                = rParam.rLowerIso_LowerZoom.clip_ctrl_2.bits.SEEE_RESP_CLIP;
    MINT32 SEEE_RESP_CLIP_LUMA_SPC_TH_L_L    = rParam.rLowerIso_LowerZoom.clip_ctrl_2.bits.SEEE_RESP_CLIP_LUMA_SPC_TH;
    MINT32 SEEE_RESP_CLIP_LUMA_LWB_L_L       = rParam.rLowerIso_LowerZoom.clip_ctrl_2.bits.SEEE_RESP_CLIP_LUMA_LWB;
    MINT32 SEEE_RESP_CLIP_LUMA_UPB_L_L       = rParam.rLowerIso_LowerZoom.clip_ctrl_2.bits.SEEE_RESP_CLIP_LUMA_UPB;

	//  ====================================================================================================================================
    //  Get Lower ISO Up Zoom
    //  ====================================================================================================================================
    MINT32 SEEE_H1_DI_BLND_OFST_L_U          = rParam.rLowerIso_UpperZoom.blnd_ctrl_1.bits.SEEE_H1_DI_BLND_OFST;
    MINT32 SEEE_H2_DI_BLND_OFST_L_U          = rParam.rLowerIso_UpperZoom.blnd_ctrl_1.bits.SEEE_H2_DI_BLND_OFST;
    MINT32 SEEE_H3_DI_BLND_OFST_L_U          = rParam.rLowerIso_UpperZoom.blnd_ctrl_1.bits.SEEE_H3_DI_BLND_OFST;
    MINT32 SEEE_H1_DI_BLND_SL_L_U            = rParam.rLowerIso_UpperZoom.blnd_ctrl_2.bits.SEEE_H1_DI_BLND_SL;
    MINT32 SEEE_H2_DI_BLND_SL_L_U            = rParam.rLowerIso_UpperZoom.blnd_ctrl_2.bits.SEEE_H2_DI_BLND_SL;
    MINT32 SEEE_H3_DI_BLND_SL_L_U            = rParam.rLowerIso_UpperZoom.blnd_ctrl_2.bits.SEEE_H3_DI_BLND_SL;
    MINT32 SEEE_HX_ISO_BLND_RAT_L_U          = rParam.rLowerIso_UpperZoom.blnd_ctrl_2.bits.SEEE_HX_ISO_BLND_RAT;
    MINT32 SEEE_H1_GN_L_U                    = rParam.rLowerIso_UpperZoom.gn_ctrl.bits.SEEE_H1_GN;
    MINT32 SEEE_H2_GN_L_U                    = rParam.rLowerIso_UpperZoom.gn_ctrl.bits.SEEE_H2_GN;
    MINT32 SEEE_H3_GN_L_U                    = rParam.rLowerIso_UpperZoom.gn_ctrl.bits.SEEE_H3_GN;
    MINT32 SEEE_H4_GN_L_U                    = rParam.rLowerIso_UpperZoom.gn_ctrl.bits.SEEE_H4_GN;
    MINT32 SEEE_FLT_CORE_TH_L_U              = rParam.rLowerIso_UpperZoom.clip_ctrl_3.bits.SEEE_FLT_CORE_TH;
    MINT32 SEEE_LUMA_MOD_Y0_L_U              = rParam.rLowerIso_UpperZoom.luma_ctrl_1.bits.SEEE_LUMA_MOD_Y0;
    MINT32 SEEE_LUMA_MOD_Y1_L_U              = rParam.rLowerIso_UpperZoom.luma_ctrl_1.bits.SEEE_LUMA_MOD_Y1;
    MINT32 SEEE_LUMA_MOD_Y2_L_U              = rParam.rLowerIso_UpperZoom.luma_ctrl_2.bits.SEEE_LUMA_MOD_Y2;
    MINT32 SEEE_LUMA_MOD_Y3_L_U              = rParam.rLowerIso_UpperZoom.luma_ctrl_2.bits.SEEE_LUMA_MOD_Y3;
    MINT32 SEEE_LUMA_MOD_Y4_L_U              = rParam.rLowerIso_UpperZoom.luma_ctrl_3.bits.SEEE_LUMA_MOD_Y4;
    MINT32 SEEE_LUMA_MOD_Y5_L_U              = rParam.rLowerIso_UpperZoom.luma_ctrl_3.bits.SEEE_LUMA_MOD_Y5;
    MINT32 SEEE_LUMA_MOD_Y6_L_U              = rParam.rLowerIso_UpperZoom.luma_ctrl_4.bits.SEEE_LUMA_MOD_Y6;
    MINT32 SEEE_SLNK_GN_Y1_L_U               = rParam.rLowerIso_UpperZoom.slnk_ctrl_1.bits.SEEE_SLNK_GN_Y1;
    MINT32 SEEE_SLNK_GN_Y2_L_U               = rParam.rLowerIso_UpperZoom.slnk_ctrl_1.bits.SEEE_SLNK_GN_Y2;
    MINT32 SEEE_RESP_SLNK_GN_RAT_L_U         = rParam.rLowerIso_UpperZoom.slnk_ctrl_2.bits.SEEE_RESP_SLNK_GN_RAT;
    MINT32 SEEE_GLUT_X1_L_U                  = rParam.rLowerIso_UpperZoom.glut_ctrl_1.bits.SEEE_GLUT_X1;
    MINT32 SEEE_GLUT_S1_L_U                  = rParam.rLowerIso_UpperZoom.glut_ctrl_1.bits.SEEE_GLUT_S1;
    MINT32 SEEE_GLUT_Y1_L_U                  = rParam.rLowerIso_UpperZoom.glut_ctrl_1.bits.SEEE_GLUT_Y1;
    MINT32 SEEE_GLUT_X2_L_U                  = rParam.rLowerIso_UpperZoom.glut_ctrl_2.bits.SEEE_GLUT_X2;
    MINT32 SEEE_GLUT_S2_L_U                  = rParam.rLowerIso_UpperZoom.glut_ctrl_2.bits.SEEE_GLUT_S2;
    MINT32 SEEE_GLUT_Y2_L_U                  = rParam.rLowerIso_UpperZoom.glut_ctrl_2.bits.SEEE_GLUT_Y2;
    MINT32 SEEE_GLUT_X3_L_U                  = rParam.rLowerIso_UpperZoom.glut_ctrl_3.bits.SEEE_GLUT_X3;
    MINT32 SEEE_GLUT_S3_L_U                  = rParam.rLowerIso_UpperZoom.glut_ctrl_3.bits.SEEE_GLUT_S3;
    MINT32 SEEE_GLUT_Y3_L_U                  = rParam.rLowerIso_UpperZoom.glut_ctrl_3.bits.SEEE_GLUT_Y3;
    MINT32 SEEE_GLUT_X4_L_U                  = rParam.rLowerIso_UpperZoom.glut_ctrl_4.bits.SEEE_GLUT_X4;
    MINT32 SEEE_GLUT_S4_L_U                  = rParam.rLowerIso_UpperZoom.glut_ctrl_4.bits.SEEE_GLUT_S4;
    MINT32 SEEE_GLUT_Y4_L_U                  = rParam.rLowerIso_UpperZoom.glut_ctrl_4.bits.SEEE_GLUT_Y4;
    MINT32 SEEE_GLUT_S5_L_U                  = rParam.rLowerIso_UpperZoom.glut_ctrl_5.bits.SEEE_GLUT_S5;
    MINT32 SEEE_GLUT_TH_OVR_L_U              = rParam.rLowerIso_UpperZoom.glut_ctrl_6.bits.SEEE_GLUT_TH_OVR;
    MINT32 SEEE_GLUT_TH_UND_L_U              = rParam.rLowerIso_UpperZoom.glut_ctrl_6.bits.SEEE_GLUT_TH_UND;
    MINT32 SEEE_GLUT_TH_MIN_L_U              = rParam.rLowerIso_UpperZoom.glut_ctrl_6.bits.SEEE_GLUT_TH_MIN;
    MINT32 SEEE_GLUT_LINK_EN_L_U             = rParam.rLowerIso_UpperZoom.slnk_ctrl_2.bits.SEEE_GLUT_LINK_EN;
    MINT32 SEEE_GLUT_SL_DEC_Y_L_U            = rParam.rLowerIso_UpperZoom.slnk_ctrl_2.bits.SEEE_GLUT_SL_DEC_Y;
    MINT32 SEEE_OVRSH_CLIP_STR_L_U           = rParam.rLowerIso_UpperZoom.clip_ctrl_3.bits.SEEE_OVRSH_CLIP_STR;
    MINT32 SEEE_DOT_TH_L_U                   = rParam.rLowerIso_UpperZoom.clip_ctrl_1.bits.SEEE_DOT_TH;
    MINT32 SEEE_DOT_REDUC_AMNT_L_U           = rParam.rLowerIso_UpperZoom.clip_ctrl_1.bits.SEEE_DOT_REDUC_AMNT;
    MINT32 SEEE_RESP_CLIP_L_U                = rParam.rLowerIso_UpperZoom.clip_ctrl_2.bits.SEEE_RESP_CLIP;
    MINT32 SEEE_RESP_CLIP_LUMA_SPC_TH_L_U    = rParam.rLowerIso_UpperZoom.clip_ctrl_2.bits.SEEE_RESP_CLIP_LUMA_SPC_TH;
    MINT32 SEEE_RESP_CLIP_LUMA_LWB_L_U       = rParam.rLowerIso_UpperZoom.clip_ctrl_2.bits.SEEE_RESP_CLIP_LUMA_LWB;
    MINT32 SEEE_RESP_CLIP_LUMA_UPB_L_U       = rParam.rLowerIso_UpperZoom.clip_ctrl_2.bits.SEEE_RESP_CLIP_LUMA_UPB;

	//  ====================================================================================================================================
    //  Get Up ISO Low Zoom
    //  ====================================================================================================================================
    MINT32 SEEE_H1_DI_BLND_OFST_U_L          = rParam.rUpperIso_LowerZoom.blnd_ctrl_1.bits.SEEE_H1_DI_BLND_OFST;
    MINT32 SEEE_H2_DI_BLND_OFST_U_L          = rParam.rUpperIso_LowerZoom.blnd_ctrl_1.bits.SEEE_H2_DI_BLND_OFST;
    MINT32 SEEE_H3_DI_BLND_OFST_U_L          = rParam.rUpperIso_LowerZoom.blnd_ctrl_1.bits.SEEE_H3_DI_BLND_OFST;
    MINT32 SEEE_H1_DI_BLND_SL_U_L            = rParam.rUpperIso_LowerZoom.blnd_ctrl_2.bits.SEEE_H1_DI_BLND_SL;
    MINT32 SEEE_H2_DI_BLND_SL_U_L            = rParam.rUpperIso_LowerZoom.blnd_ctrl_2.bits.SEEE_H2_DI_BLND_SL;
    MINT32 SEEE_H3_DI_BLND_SL_U_L            = rParam.rUpperIso_LowerZoom.blnd_ctrl_2.bits.SEEE_H3_DI_BLND_SL;
    MINT32 SEEE_HX_ISO_BLND_RAT_U_L          = rParam.rUpperIso_LowerZoom.blnd_ctrl_2.bits.SEEE_HX_ISO_BLND_RAT;
    MINT32 SEEE_H1_GN_U_L                    = rParam.rUpperIso_LowerZoom.gn_ctrl.bits.SEEE_H1_GN;
    MINT32 SEEE_H2_GN_U_L                    = rParam.rUpperIso_LowerZoom.gn_ctrl.bits.SEEE_H2_GN;
    MINT32 SEEE_H3_GN_U_L                    = rParam.rUpperIso_LowerZoom.gn_ctrl.bits.SEEE_H3_GN;
    MINT32 SEEE_H4_GN_U_L                    = rParam.rUpperIso_LowerZoom.gn_ctrl.bits.SEEE_H4_GN;
    MINT32 SEEE_FLT_CORE_TH_U_L              = rParam.rUpperIso_LowerZoom.clip_ctrl_3.bits.SEEE_FLT_CORE_TH;
    MINT32 SEEE_LUMA_MOD_Y0_U_L              = rParam.rUpperIso_LowerZoom.luma_ctrl_1.bits.SEEE_LUMA_MOD_Y0;
    MINT32 SEEE_LUMA_MOD_Y1_U_L              = rParam.rUpperIso_LowerZoom.luma_ctrl_1.bits.SEEE_LUMA_MOD_Y1;
    MINT32 SEEE_LUMA_MOD_Y2_U_L              = rParam.rUpperIso_LowerZoom.luma_ctrl_2.bits.SEEE_LUMA_MOD_Y2;
    MINT32 SEEE_LUMA_MOD_Y3_U_L              = rParam.rUpperIso_LowerZoom.luma_ctrl_2.bits.SEEE_LUMA_MOD_Y3;
    MINT32 SEEE_LUMA_MOD_Y4_U_L              = rParam.rUpperIso_LowerZoom.luma_ctrl_3.bits.SEEE_LUMA_MOD_Y4;
    MINT32 SEEE_LUMA_MOD_Y5_U_L              = rParam.rUpperIso_LowerZoom.luma_ctrl_3.bits.SEEE_LUMA_MOD_Y5;
    MINT32 SEEE_LUMA_MOD_Y6_U_L              = rParam.rUpperIso_LowerZoom.luma_ctrl_4.bits.SEEE_LUMA_MOD_Y6;
    MINT32 SEEE_SLNK_GN_Y1_U_L               = rParam.rUpperIso_LowerZoom.slnk_ctrl_1.bits.SEEE_SLNK_GN_Y1;
    MINT32 SEEE_SLNK_GN_Y2_U_L               = rParam.rUpperIso_LowerZoom.slnk_ctrl_1.bits.SEEE_SLNK_GN_Y2;
    MINT32 SEEE_RESP_SLNK_GN_RAT_U_L         = rParam.rUpperIso_LowerZoom.slnk_ctrl_2.bits.SEEE_RESP_SLNK_GN_RAT;
    MINT32 SEEE_GLUT_X1_U_L                  = rParam.rUpperIso_LowerZoom.glut_ctrl_1.bits.SEEE_GLUT_X1;
    MINT32 SEEE_GLUT_S1_U_L                  = rParam.rUpperIso_LowerZoom.glut_ctrl_1.bits.SEEE_GLUT_S1;
    MINT32 SEEE_GLUT_Y1_U_L                  = rParam.rUpperIso_LowerZoom.glut_ctrl_1.bits.SEEE_GLUT_Y1;
    MINT32 SEEE_GLUT_X2_U_L                  = rParam.rUpperIso_LowerZoom.glut_ctrl_2.bits.SEEE_GLUT_X2;
    MINT32 SEEE_GLUT_S2_U_L                  = rParam.rUpperIso_LowerZoom.glut_ctrl_2.bits.SEEE_GLUT_S2;
    MINT32 SEEE_GLUT_Y2_U_L                  = rParam.rUpperIso_LowerZoom.glut_ctrl_2.bits.SEEE_GLUT_Y2;
    MINT32 SEEE_GLUT_X3_U_L                  = rParam.rUpperIso_LowerZoom.glut_ctrl_3.bits.SEEE_GLUT_X3;
    MINT32 SEEE_GLUT_S3_U_L                  = rParam.rUpperIso_LowerZoom.glut_ctrl_3.bits.SEEE_GLUT_S3;
    MINT32 SEEE_GLUT_Y3_U_L                  = rParam.rUpperIso_LowerZoom.glut_ctrl_3.bits.SEEE_GLUT_Y3;
    MINT32 SEEE_GLUT_X4_U_L                  = rParam.rUpperIso_LowerZoom.glut_ctrl_4.bits.SEEE_GLUT_X4;
    MINT32 SEEE_GLUT_S4_U_L                  = rParam.rUpperIso_LowerZoom.glut_ctrl_4.bits.SEEE_GLUT_S4;
    MINT32 SEEE_GLUT_Y4_U_L                  = rParam.rUpperIso_LowerZoom.glut_ctrl_4.bits.SEEE_GLUT_Y4;
    MINT32 SEEE_GLUT_S5_U_L                  = rParam.rUpperIso_LowerZoom.glut_ctrl_5.bits.SEEE_GLUT_S5;
    MINT32 SEEE_GLUT_TH_OVR_U_L              = rParam.rUpperIso_LowerZoom.glut_ctrl_6.bits.SEEE_GLUT_TH_OVR;
    MINT32 SEEE_GLUT_TH_UND_U_L              = rParam.rUpperIso_LowerZoom.glut_ctrl_6.bits.SEEE_GLUT_TH_UND;
    MINT32 SEEE_GLUT_TH_MIN_U_L              = rParam.rUpperIso_LowerZoom.glut_ctrl_6.bits.SEEE_GLUT_TH_MIN;
    MINT32 SEEE_GLUT_LINK_EN_U_L             = rParam.rUpperIso_LowerZoom.slnk_ctrl_2.bits.SEEE_GLUT_LINK_EN;
    MINT32 SEEE_GLUT_SL_DEC_Y_U_L            = rParam.rUpperIso_LowerZoom.slnk_ctrl_2.bits.SEEE_GLUT_SL_DEC_Y;
    MINT32 SEEE_OVRSH_CLIP_STR_U_L           = rParam.rUpperIso_LowerZoom.clip_ctrl_3.bits.SEEE_OVRSH_CLIP_STR;
    MINT32 SEEE_DOT_TH_U_L                   = rParam.rUpperIso_LowerZoom.clip_ctrl_1.bits.SEEE_DOT_TH;
    MINT32 SEEE_DOT_REDUC_AMNT_U_L           = rParam.rUpperIso_LowerZoom.clip_ctrl_1.bits.SEEE_DOT_REDUC_AMNT;
    MINT32 SEEE_RESP_CLIP_U_L                = rParam.rUpperIso_LowerZoom.clip_ctrl_2.bits.SEEE_RESP_CLIP;
    MINT32 SEEE_RESP_CLIP_LUMA_SPC_TH_U_L    = rParam.rUpperIso_LowerZoom.clip_ctrl_2.bits.SEEE_RESP_CLIP_LUMA_SPC_TH;
    MINT32 SEEE_RESP_CLIP_LUMA_LWB_U_L       = rParam.rUpperIso_LowerZoom.clip_ctrl_2.bits.SEEE_RESP_CLIP_LUMA_LWB;
    MINT32 SEEE_RESP_CLIP_LUMA_UPB_U_L       = rParam.rUpperIso_LowerZoom.clip_ctrl_2.bits.SEEE_RESP_CLIP_LUMA_UPB;

	//  ====================================================================================================================================
    //  Get Up ISO Up Zoom
    //  ====================================================================================================================================
    MINT32 SEEE_H1_DI_BLND_OFST_U_U          = rParam.rUpperIso_UpperZoom.blnd_ctrl_1.bits.SEEE_H1_DI_BLND_OFST;
    MINT32 SEEE_H2_DI_BLND_OFST_U_U          = rParam.rUpperIso_UpperZoom.blnd_ctrl_1.bits.SEEE_H2_DI_BLND_OFST;
    MINT32 SEEE_H3_DI_BLND_OFST_U_U          = rParam.rUpperIso_UpperZoom.blnd_ctrl_1.bits.SEEE_H3_DI_BLND_OFST;
    MINT32 SEEE_H1_DI_BLND_SL_U_U            = rParam.rUpperIso_UpperZoom.blnd_ctrl_2.bits.SEEE_H1_DI_BLND_SL;
    MINT32 SEEE_H2_DI_BLND_SL_U_U            = rParam.rUpperIso_UpperZoom.blnd_ctrl_2.bits.SEEE_H2_DI_BLND_SL;
    MINT32 SEEE_H3_DI_BLND_SL_U_U            = rParam.rUpperIso_UpperZoom.blnd_ctrl_2.bits.SEEE_H3_DI_BLND_SL;
    MINT32 SEEE_HX_ISO_BLND_RAT_U_U          = rParam.rUpperIso_UpperZoom.blnd_ctrl_2.bits.SEEE_HX_ISO_BLND_RAT;
    MINT32 SEEE_H1_GN_U_U                    = rParam.rUpperIso_UpperZoom.gn_ctrl.bits.SEEE_H1_GN;
    MINT32 SEEE_H2_GN_U_U                    = rParam.rUpperIso_UpperZoom.gn_ctrl.bits.SEEE_H2_GN;
    MINT32 SEEE_H3_GN_U_U                    = rParam.rUpperIso_UpperZoom.gn_ctrl.bits.SEEE_H3_GN;
    MINT32 SEEE_H4_GN_U_U                    = rParam.rUpperIso_UpperZoom.gn_ctrl.bits.SEEE_H4_GN;
    MINT32 SEEE_FLT_CORE_TH_U_U              = rParam.rUpperIso_UpperZoom.clip_ctrl_3.bits.SEEE_FLT_CORE_TH;
    MINT32 SEEE_LUMA_MOD_Y0_U_U              = rParam.rUpperIso_UpperZoom.luma_ctrl_1.bits.SEEE_LUMA_MOD_Y0;
    MINT32 SEEE_LUMA_MOD_Y1_U_U              = rParam.rUpperIso_UpperZoom.luma_ctrl_1.bits.SEEE_LUMA_MOD_Y1;
    MINT32 SEEE_LUMA_MOD_Y2_U_U              = rParam.rUpperIso_UpperZoom.luma_ctrl_2.bits.SEEE_LUMA_MOD_Y2;
    MINT32 SEEE_LUMA_MOD_Y3_U_U              = rParam.rUpperIso_UpperZoom.luma_ctrl_2.bits.SEEE_LUMA_MOD_Y3;
    MINT32 SEEE_LUMA_MOD_Y4_U_U              = rParam.rUpperIso_UpperZoom.luma_ctrl_3.bits.SEEE_LUMA_MOD_Y4;
    MINT32 SEEE_LUMA_MOD_Y5_U_U              = rParam.rUpperIso_UpperZoom.luma_ctrl_3.bits.SEEE_LUMA_MOD_Y5;
    MINT32 SEEE_LUMA_MOD_Y6_U_U              = rParam.rUpperIso_UpperZoom.luma_ctrl_4.bits.SEEE_LUMA_MOD_Y6;
    MINT32 SEEE_SLNK_GN_Y1_U_U               = rParam.rUpperIso_UpperZoom.slnk_ctrl_1.bits.SEEE_SLNK_GN_Y1;
    MINT32 SEEE_SLNK_GN_Y2_U_U               = rParam.rUpperIso_UpperZoom.slnk_ctrl_1.bits.SEEE_SLNK_GN_Y2;
    MINT32 SEEE_RESP_SLNK_GN_RAT_U_U         = rParam.rUpperIso_UpperZoom.slnk_ctrl_2.bits.SEEE_RESP_SLNK_GN_RAT;
    MINT32 SEEE_GLUT_X1_U_U                  = rParam.rUpperIso_UpperZoom.glut_ctrl_1.bits.SEEE_GLUT_X1;
    MINT32 SEEE_GLUT_S1_U_U                  = rParam.rUpperIso_UpperZoom.glut_ctrl_1.bits.SEEE_GLUT_S1;
    MINT32 SEEE_GLUT_Y1_U_U                  = rParam.rUpperIso_UpperZoom.glut_ctrl_1.bits.SEEE_GLUT_Y1;
    MINT32 SEEE_GLUT_X2_U_U                  = rParam.rUpperIso_UpperZoom.glut_ctrl_2.bits.SEEE_GLUT_X2;
    MINT32 SEEE_GLUT_S2_U_U                  = rParam.rUpperIso_UpperZoom.glut_ctrl_2.bits.SEEE_GLUT_S2;
    MINT32 SEEE_GLUT_Y2_U_U                  = rParam.rUpperIso_UpperZoom.glut_ctrl_2.bits.SEEE_GLUT_Y2;
    MINT32 SEEE_GLUT_X3_U_U                  = rParam.rUpperIso_UpperZoom.glut_ctrl_3.bits.SEEE_GLUT_X3;
    MINT32 SEEE_GLUT_S3_U_U                  = rParam.rUpperIso_UpperZoom.glut_ctrl_3.bits.SEEE_GLUT_S3;
    MINT32 SEEE_GLUT_Y3_U_U                  = rParam.rUpperIso_UpperZoom.glut_ctrl_3.bits.SEEE_GLUT_Y3;
    MINT32 SEEE_GLUT_X4_U_U                  = rParam.rUpperIso_UpperZoom.glut_ctrl_4.bits.SEEE_GLUT_X4;
    MINT32 SEEE_GLUT_S4_U_U                  = rParam.rUpperIso_UpperZoom.glut_ctrl_4.bits.SEEE_GLUT_S4;
    MINT32 SEEE_GLUT_Y4_U_U                  = rParam.rUpperIso_UpperZoom.glut_ctrl_4.bits.SEEE_GLUT_Y4;
    MINT32 SEEE_GLUT_S5_U_U                  = rParam.rUpperIso_UpperZoom.glut_ctrl_5.bits.SEEE_GLUT_S5;
    MINT32 SEEE_GLUT_TH_OVR_U_U              = rParam.rUpperIso_UpperZoom.glut_ctrl_6.bits.SEEE_GLUT_TH_OVR;
    MINT32 SEEE_GLUT_TH_UND_U_U              = rParam.rUpperIso_UpperZoom.glut_ctrl_6.bits.SEEE_GLUT_TH_UND;
    MINT32 SEEE_GLUT_TH_MIN_U_U              = rParam.rUpperIso_UpperZoom.glut_ctrl_6.bits.SEEE_GLUT_TH_MIN;
    MINT32 SEEE_GLUT_LINK_EN_U_U             = rParam.rUpperIso_UpperZoom.slnk_ctrl_2.bits.SEEE_GLUT_LINK_EN;
    MINT32 SEEE_GLUT_SL_DEC_Y_U_U            = rParam.rUpperIso_UpperZoom.slnk_ctrl_2.bits.SEEE_GLUT_SL_DEC_Y;
    MINT32 SEEE_OVRSH_CLIP_STR_U_U           = rParam.rUpperIso_UpperZoom.clip_ctrl_3.bits.SEEE_OVRSH_CLIP_STR;
    MINT32 SEEE_DOT_TH_U_U                   = rParam.rUpperIso_UpperZoom.clip_ctrl_1.bits.SEEE_DOT_TH;
    MINT32 SEEE_DOT_REDUC_AMNT_U_U           = rParam.rUpperIso_UpperZoom.clip_ctrl_1.bits.SEEE_DOT_REDUC_AMNT;
    MINT32 SEEE_RESP_CLIP_U_U                = rParam.rUpperIso_UpperZoom.clip_ctrl_2.bits.SEEE_RESP_CLIP;
    MINT32 SEEE_RESP_CLIP_LUMA_SPC_TH_U_U    = rParam.rUpperIso_UpperZoom.clip_ctrl_2.bits.SEEE_RESP_CLIP_LUMA_SPC_TH;
    MINT32 SEEE_RESP_CLIP_LUMA_LWB_U_U       = rParam.rUpperIso_UpperZoom.clip_ctrl_2.bits.SEEE_RESP_CLIP_LUMA_LWB;
    MINT32 SEEE_RESP_CLIP_LUMA_UPB_U_U       = rParam.rUpperIso_UpperZoom.clip_ctrl_2.bits.SEEE_RESP_CLIP_LUMA_UPB;



    //  Negative value for slope
    SEEE_GLUT_S1_L_L = (SEEE_GLUT_S1_L_L > 127) ? (SEEE_GLUT_S1_L_L - 256) : SEEE_GLUT_S1_L_L;
    SEEE_GLUT_S2_L_L = (SEEE_GLUT_S2_L_L > 127) ? (SEEE_GLUT_S2_L_L - 256) : SEEE_GLUT_S2_L_L;
    SEEE_GLUT_S3_L_L = (SEEE_GLUT_S3_L_L > 127) ? (SEEE_GLUT_S3_L_L - 256) : SEEE_GLUT_S3_L_L;
    SEEE_GLUT_S4_L_L = (SEEE_GLUT_S4_L_L > 127) ? (SEEE_GLUT_S4_L_L - 256) : SEEE_GLUT_S4_L_L;
    SEEE_GLUT_S5_L_L = (SEEE_GLUT_S5_L_L > 127) ? (SEEE_GLUT_S5_L_L - 256) : SEEE_GLUT_S5_L_L;

	SEEE_GLUT_S1_L_U = (SEEE_GLUT_S1_L_U > 127) ? (SEEE_GLUT_S1_L_U - 256) : SEEE_GLUT_S1_L_U;
    SEEE_GLUT_S2_L_U = (SEEE_GLUT_S2_L_U > 127) ? (SEEE_GLUT_S2_L_U - 256) : SEEE_GLUT_S2_L_U;
    SEEE_GLUT_S3_L_U = (SEEE_GLUT_S3_L_U > 127) ? (SEEE_GLUT_S3_L_U - 256) : SEEE_GLUT_S3_L_U;
    SEEE_GLUT_S4_L_U = (SEEE_GLUT_S4_L_U > 127) ? (SEEE_GLUT_S4_L_U - 256) : SEEE_GLUT_S4_L_U;
    SEEE_GLUT_S5_L_U = (SEEE_GLUT_S5_L_U > 127) ? (SEEE_GLUT_S5_L_U - 256) : SEEE_GLUT_S5_L_U;

	SEEE_GLUT_S1_U_L = (SEEE_GLUT_S1_U_L > 127) ? (SEEE_GLUT_S1_U_L - 256) : SEEE_GLUT_S1_U_L;
    SEEE_GLUT_S2_U_L = (SEEE_GLUT_S2_U_L > 127) ? (SEEE_GLUT_S2_U_L - 256) : SEEE_GLUT_S2_U_L;
    SEEE_GLUT_S3_U_L = (SEEE_GLUT_S3_U_L > 127) ? (SEEE_GLUT_S3_U_L - 256) : SEEE_GLUT_S3_U_L;
    SEEE_GLUT_S4_U_L = (SEEE_GLUT_S4_U_L > 127) ? (SEEE_GLUT_S4_U_L - 256) : SEEE_GLUT_S4_U_L;
    SEEE_GLUT_S5_U_L = (SEEE_GLUT_S5_U_L > 127) ? (SEEE_GLUT_S5_U_L - 256) : SEEE_GLUT_S5_U_L;

	SEEE_GLUT_S1_U_U = (SEEE_GLUT_S1_U_U > 127) ? (SEEE_GLUT_S1_U_U - 256) : SEEE_GLUT_S1_U_U;
    SEEE_GLUT_S2_U_U = (SEEE_GLUT_S2_U_U > 127) ? (SEEE_GLUT_S2_U_U - 256) : SEEE_GLUT_S2_U_U;
    SEEE_GLUT_S3_U_U = (SEEE_GLUT_S3_U_U > 127) ? (SEEE_GLUT_S3_U_U - 256) : SEEE_GLUT_S3_U_U;
    SEEE_GLUT_S4_U_U = (SEEE_GLUT_S4_U_U > 127) ? (SEEE_GLUT_S4_U_U - 256) : SEEE_GLUT_S4_U_U;
    SEEE_GLUT_S5_U_U = (SEEE_GLUT_S5_U_U > 127) ? (SEEE_GLUT_S5_U_U - 256) : SEEE_GLUT_S5_U_U;



    //  ====================================================================================================================================
    //  Start Parameter Interpolation
    //  ====================================================================================================================================


    //  Registers that can be interpolated
    MINT32 SEEE_H1_DI_BLND_OFST            = InterParam_EE(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, SEEE_H1_DI_BLND_OFST_L_L, SEEE_H1_DI_BLND_OFST_L_U, SEEE_H1_DI_BLND_OFST_U_L, SEEE_H1_DI_BLND_OFST_U_U,0);
    MINT32 SEEE_H2_DI_BLND_OFST            = InterParam_EE(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, SEEE_H2_DI_BLND_OFST_L_L, SEEE_H2_DI_BLND_OFST_L_U, SEEE_H2_DI_BLND_OFST_U_L, SEEE_H2_DI_BLND_OFST_U_U,0);
    MINT32 SEEE_H3_DI_BLND_OFST            = InterParam_EE(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, SEEE_H3_DI_BLND_OFST_L_L, SEEE_H3_DI_BLND_OFST_L_U, SEEE_H3_DI_BLND_OFST_U_L, SEEE_H3_DI_BLND_OFST_U_U,0);
    MINT32 SEEE_H1_DI_BLND_SL            = InterParam_EE(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, SEEE_H1_DI_BLND_SL_L_L, SEEE_H1_DI_BLND_SL_L_U, SEEE_H1_DI_BLND_SL_U_L, SEEE_H1_DI_BLND_SL_U_U,0);
    MINT32 SEEE_H2_DI_BLND_SL            = InterParam_EE(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, SEEE_H2_DI_BLND_SL_L_L, SEEE_H2_DI_BLND_SL_L_U, SEEE_H2_DI_BLND_SL_U_L, SEEE_H2_DI_BLND_SL_U_U,0);
    MINT32 SEEE_H3_DI_BLND_SL            = InterParam_EE(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, SEEE_H3_DI_BLND_SL_L_L, SEEE_H3_DI_BLND_SL_L_U, SEEE_H3_DI_BLND_SL_U_L, SEEE_H3_DI_BLND_SL_U_U,0);
    MINT32 SEEE_HX_ISO_BLND_RAT            = InterParam_EE(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, SEEE_HX_ISO_BLND_RAT_L_L, SEEE_HX_ISO_BLND_RAT_L_U, SEEE_HX_ISO_BLND_RAT_U_L, SEEE_HX_ISO_BLND_RAT_U_U,0);
    MINT32 SEEE_H1_GN            = InterParam_EE(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, SEEE_H1_GN_L_L, SEEE_H1_GN_L_U, SEEE_H1_GN_U_L, SEEE_H1_GN_U_U,0);
    MINT32 SEEE_H2_GN            = InterParam_EE(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, SEEE_H2_GN_L_L, SEEE_H2_GN_L_U, SEEE_H2_GN_U_L, SEEE_H2_GN_U_U,0);
    MINT32 SEEE_H3_GN            = InterParam_EE(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, SEEE_H3_GN_L_L, SEEE_H3_GN_L_U, SEEE_H3_GN_U_L, SEEE_H3_GN_U_U,0);
    MINT32 SEEE_H4_GN            = InterParam_EE(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, SEEE_H4_GN_L_L, SEEE_H4_GN_L_U, SEEE_H4_GN_U_L, SEEE_H4_GN_U_U,0);
    MINT32 SEEE_FLT_CORE_TH            = InterParam_EE(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, SEEE_FLT_CORE_TH_L_L, SEEE_FLT_CORE_TH_L_U, SEEE_FLT_CORE_TH_U_L, SEEE_FLT_CORE_TH_U_U,0);
    MINT32 SEEE_LUMA_MOD_Y0            = InterParam_EE(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, SEEE_LUMA_MOD_Y0_L_L, SEEE_LUMA_MOD_Y0_L_U, SEEE_LUMA_MOD_Y0_U_L, SEEE_LUMA_MOD_Y0_U_U,0);
    MINT32 SEEE_LUMA_MOD_Y1            = InterParam_EE(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, SEEE_LUMA_MOD_Y1_L_L, SEEE_LUMA_MOD_Y1_L_U, SEEE_LUMA_MOD_Y1_U_L, SEEE_LUMA_MOD_Y1_U_U,0);
    MINT32 SEEE_LUMA_MOD_Y2            = InterParam_EE(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, SEEE_LUMA_MOD_Y2_L_L, SEEE_LUMA_MOD_Y2_L_U, SEEE_LUMA_MOD_Y2_U_L, SEEE_LUMA_MOD_Y2_U_U,0);
    MINT32 SEEE_LUMA_MOD_Y3            = InterParam_EE(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, SEEE_LUMA_MOD_Y3_L_L, SEEE_LUMA_MOD_Y3_L_U, SEEE_LUMA_MOD_Y3_U_L, SEEE_LUMA_MOD_Y3_U_U,0);
    MINT32 SEEE_LUMA_MOD_Y4            = InterParam_EE(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, SEEE_LUMA_MOD_Y4_L_L, SEEE_LUMA_MOD_Y4_L_U, SEEE_LUMA_MOD_Y4_U_L, SEEE_LUMA_MOD_Y4_U_U,0);
    MINT32 SEEE_LUMA_MOD_Y5            = InterParam_EE(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, SEEE_LUMA_MOD_Y5_L_L, SEEE_LUMA_MOD_Y5_L_U, SEEE_LUMA_MOD_Y5_U_L, SEEE_LUMA_MOD_Y5_U_U,0);
    MINT32 SEEE_LUMA_MOD_Y6            = InterParam_EE(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, SEEE_LUMA_MOD_Y6_L_L, SEEE_LUMA_MOD_Y6_L_U, SEEE_LUMA_MOD_Y6_U_L, SEEE_LUMA_MOD_Y6_U_U,0);
    MINT32 SEEE_SLNK_GN_Y1            = InterParam_EE(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, SEEE_SLNK_GN_Y1_L_L, SEEE_SLNK_GN_Y1_L_U, SEEE_SLNK_GN_Y1_U_L, SEEE_SLNK_GN_Y1_U_U,0);
    MINT32 SEEE_SLNK_GN_Y2            = InterParam_EE(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, SEEE_SLNK_GN_Y2_L_L, SEEE_SLNK_GN_Y2_L_U, SEEE_SLNK_GN_Y2_U_L, SEEE_SLNK_GN_Y2_U_U,0);
    MINT32 SEEE_RESP_SLNK_GN_RAT            = InterParam_EE(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, SEEE_RESP_SLNK_GN_RAT_L_L, SEEE_RESP_SLNK_GN_RAT_L_U, SEEE_RESP_SLNK_GN_RAT_U_L, SEEE_RESP_SLNK_GN_RAT_U_U,0);
    MINT32 SEEE_GLUT_X1            = InterParam_EE(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, SEEE_GLUT_X1_L_L, SEEE_GLUT_X1_L_U, SEEE_GLUT_X1_U_L, SEEE_GLUT_X1_U_U,0);
    MINT32 SEEE_GLUT_X2            = InterParam_EE(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, SEEE_GLUT_X2_L_L, SEEE_GLUT_X2_L_U, SEEE_GLUT_X2_U_L, SEEE_GLUT_X2_U_U,0);
    MINT32 SEEE_GLUT_X3            = InterParam_EE(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, SEEE_GLUT_X3_L_L, SEEE_GLUT_X3_L_U, SEEE_GLUT_X3_U_L, SEEE_GLUT_X3_U_U,0);
    MINT32 SEEE_GLUT_X4            = InterParam_EE(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, SEEE_GLUT_X4_L_L, SEEE_GLUT_X4_L_U, SEEE_GLUT_X4_U_L, SEEE_GLUT_X4_U_U,0);
    MINT32 SEEE_GLUT_Y1            = InterParam_EE(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, SEEE_GLUT_Y1_L_L, SEEE_GLUT_Y1_L_U, SEEE_GLUT_Y1_U_L, SEEE_GLUT_Y1_U_U,0);
    MINT32 SEEE_GLUT_Y2            = InterParam_EE(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, SEEE_GLUT_Y2_L_L, SEEE_GLUT_Y2_L_U, SEEE_GLUT_Y2_U_L, SEEE_GLUT_Y2_U_U,0);
    MINT32 SEEE_GLUT_Y3            = InterParam_EE(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, SEEE_GLUT_Y3_L_L, SEEE_GLUT_Y3_L_U, SEEE_GLUT_Y3_U_L, SEEE_GLUT_Y3_U_U,0);
    MINT32 SEEE_GLUT_Y4            = InterParam_EE(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, SEEE_GLUT_Y4_L_L, SEEE_GLUT_Y4_L_U, SEEE_GLUT_Y4_U_L, SEEE_GLUT_Y4_U_U,0);
    MINT32 SEEE_GLUT_TH_OVR            = InterParam_EE(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, SEEE_GLUT_TH_OVR_L_L, SEEE_GLUT_TH_OVR_L_U, SEEE_GLUT_TH_OVR_U_L, SEEE_GLUT_TH_OVR_U_U,0);
    MINT32 SEEE_GLUT_TH_UND            = InterParam_EE(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, SEEE_GLUT_TH_UND_L_L, SEEE_GLUT_TH_UND_L_U, SEEE_GLUT_TH_UND_U_L, SEEE_GLUT_TH_UND_U_U,0);
    MINT32 SEEE_GLUT_TH_MIN            = InterParam_EE(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, SEEE_GLUT_TH_MIN_L_L, SEEE_GLUT_TH_MIN_L_U, SEEE_GLUT_TH_MIN_U_L, SEEE_GLUT_TH_MIN_U_U,0);
    MINT32 SEEE_GLUT_LINK_EN            = InterParam_EE(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, SEEE_GLUT_LINK_EN_L_L, SEEE_GLUT_LINK_EN_L_U, SEEE_GLUT_LINK_EN_U_L, SEEE_GLUT_LINK_EN_U_U,1);
    MINT32 SEEE_GLUT_SL_DEC_Y            = InterParam_EE(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, SEEE_GLUT_SL_DEC_Y_L_L, SEEE_GLUT_SL_DEC_Y_L_U, SEEE_GLUT_SL_DEC_Y_U_L, SEEE_GLUT_SL_DEC_Y_U_U,0);
    MINT32 SEEE_OVRSH_CLIP_STR            = InterParam_EE(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, SEEE_OVRSH_CLIP_STR_L_L, SEEE_OVRSH_CLIP_STR_L_U, SEEE_OVRSH_CLIP_STR_U_L, SEEE_OVRSH_CLIP_STR_U_U,0);
    MINT32 SEEE_DOT_TH            = InterParam_EE(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, SEEE_DOT_TH_L_L, SEEE_DOT_TH_L_U, SEEE_DOT_TH_U_L, SEEE_DOT_TH_U_U,0);
    MINT32 SEEE_DOT_REDUC_AMNT            = InterParam_EE(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, SEEE_DOT_REDUC_AMNT_L_L, SEEE_DOT_REDUC_AMNT_L_U, SEEE_DOT_REDUC_AMNT_U_L, SEEE_DOT_REDUC_AMNT_U_U,0);
    MINT32 SEEE_RESP_CLIP            = InterParam_EE(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, SEEE_RESP_CLIP_L_L, SEEE_RESP_CLIP_L_U, SEEE_RESP_CLIP_U_L, SEEE_RESP_CLIP_U_U,0);
    MINT32 SEEE_RESP_CLIP_LUMA_SPC_TH            = InterParam_EE(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, SEEE_RESP_CLIP_LUMA_SPC_TH_L_L, SEEE_RESP_CLIP_LUMA_SPC_TH_L_U, SEEE_RESP_CLIP_LUMA_SPC_TH_U_L, SEEE_RESP_CLIP_LUMA_SPC_TH_U_U,0);
    MINT32 SEEE_RESP_CLIP_LUMA_LWB            = InterParam_EE(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, SEEE_RESP_CLIP_LUMA_LWB_L_L, SEEE_RESP_CLIP_LUMA_LWB_L_U, SEEE_RESP_CLIP_LUMA_LWB_U_L, SEEE_RESP_CLIP_LUMA_LWB_U_U,0);
    MINT32 SEEE_RESP_CLIP_LUMA_UPB            = InterParam_EE(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, SEEE_RESP_CLIP_LUMA_UPB_L_L, SEEE_RESP_CLIP_LUMA_UPB_L_U, SEEE_RESP_CLIP_LUMA_UPB_U_L, SEEE_RESP_CLIP_LUMA_UPB_U_U,0);

    //  To calculate S5, we need to find Y5 first.
    //  Y5 is not a real register. So we use (X4, Y4) and S5 to evaluate (255, Y5) for pre-ISO and post-ISO setting.
    //  Then we get the interpolated Y5.
    MINT32 S5_L_L_sign       = (SEEE_GLUT_S5_L_L >= 0) ? 1 : -1;
    MINT32 SEEE_GLUT_Y5_L_L  = SEEE_GLUT_Y4_L_L + (SEEE_GLUT_S5_L_L * (255 - SEEE_GLUT_X4_L_L) + 8 * S5_L_L_sign) / 16;

	MINT32 S5_L_U_sign       = (SEEE_GLUT_S5_L_U >= 0) ? 1 : -1;
    MINT32 SEEE_GLUT_Y5_L_U  = SEEE_GLUT_Y4_L_U + (SEEE_GLUT_S5_L_U * (255 - SEEE_GLUT_X4_L_U) + 8 * S5_L_U_sign) / 16;

	MINT32 S5_U_L_sign       = (SEEE_GLUT_S5_U_L >= 0) ? 1 : -1;
    MINT32 SEEE_GLUT_Y5_U_L  = SEEE_GLUT_Y4_U_L + (SEEE_GLUT_S5_U_L * (255 - SEEE_GLUT_X4_U_L) + 8 * S5_U_L_sign) / 16;

	MINT32 S5_U_U_sign       = (SEEE_GLUT_S5_U_U >= 0) ? 1 : -1;
    MINT32 SEEE_GLUT_Y5_U_U  = SEEE_GLUT_Y4_U_U + (SEEE_GLUT_S5_U_U * (255 - SEEE_GLUT_X4_U_U) + 8 * S5_U_U_sign) / 16;

    MINT32 SEEE_GLUT_Y5            = InterParam_EE(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, SEEE_GLUT_Y5_L_L, SEEE_GLUT_Y5_L_U, SEEE_GLUT_Y5_U_L, SEEE_GLUT_Y5_U_U,0);

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

	// negative conversion

	SEEE_GLUT_S1 = (SEEE_GLUT_S1 < 0) ? (SEEE_GLUT_S1 + 256) : SEEE_GLUT_S1;
	SEEE_GLUT_S2 = (SEEE_GLUT_S2 < 0) ? (SEEE_GLUT_S2 + 256) : SEEE_GLUT_S2;
	SEEE_GLUT_S3 = (SEEE_GLUT_S3 < 0) ? (SEEE_GLUT_S3 + 256) : SEEE_GLUT_S3;
	SEEE_GLUT_S4 = (SEEE_GLUT_S4 < 0) ? (SEEE_GLUT_S4 + 256) : SEEE_GLUT_S4;
	SEEE_GLUT_S5 = (SEEE_GLUT_S5 < 0) ? (SEEE_GLUT_S5 + 256) : SEEE_GLUT_S5;

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

	#define EE_DUMP(item1, item2)   \
        do{                      \
            INTER_LOG_IF(bSmoothEEDebug, "[S, L_L, L_U, U_L, U_U]%22s = %d, %d, %d, %d, %d", #item2, rSmoothEE.item1.bits.item2, rParam.rLowerIso_LowerZoom.item1.bits.item2, rParam.rLowerIso_UpperZoom.item1.bits.item2, rParam.rUpperIso_LowerZoom.item1.bits.item2, rParam.rUpperIso_UpperZoom.item1.bits.item2); \
        }while(0)

    EE_DUMP(blnd_ctrl_1, SEEE_H1_DI_BLND_OFST);
	EE_DUMP(blnd_ctrl_1, SEEE_H2_DI_BLND_OFST);
	EE_DUMP(blnd_ctrl_1, SEEE_H3_DI_BLND_OFST);
	EE_DUMP(blnd_ctrl_2, SEEE_H1_DI_BLND_SL);
	EE_DUMP(blnd_ctrl_2, SEEE_H2_DI_BLND_SL);
	EE_DUMP(blnd_ctrl_2, SEEE_H3_DI_BLND_SL);
	EE_DUMP(blnd_ctrl_2, SEEE_HX_ISO_BLND_RAT);
	EE_DUMP(gn_ctrl, SEEE_H1_GN);
	EE_DUMP(gn_ctrl, SEEE_H2_GN);
	EE_DUMP(gn_ctrl, SEEE_H3_GN);
	EE_DUMP(gn_ctrl, SEEE_H4_GN);
	EE_DUMP(clip_ctrl_3, SEEE_FLT_CORE_TH);
	EE_DUMP(luma_ctrl_1, SEEE_LUMA_MOD_Y0);
	EE_DUMP(luma_ctrl_1, SEEE_LUMA_MOD_Y1);
	EE_DUMP(luma_ctrl_2, SEEE_LUMA_MOD_Y2);
	EE_DUMP(luma_ctrl_2, SEEE_LUMA_MOD_Y3);
	EE_DUMP(luma_ctrl_3, SEEE_LUMA_MOD_Y4);
	EE_DUMP(luma_ctrl_3, SEEE_LUMA_MOD_Y5);
	EE_DUMP(luma_ctrl_4, SEEE_LUMA_MOD_Y6);
	EE_DUMP(slnk_ctrl_1, SEEE_SLNK_GN_Y1);
	EE_DUMP(slnk_ctrl_1, SEEE_SLNK_GN_Y2);
	EE_DUMP(slnk_ctrl_2, SEEE_RESP_SLNK_GN_RAT);
	EE_DUMP(glut_ctrl_1, SEEE_GLUT_X1);
	EE_DUMP(glut_ctrl_1, SEEE_GLUT_S1);
	EE_DUMP(glut_ctrl_1, SEEE_GLUT_Y1);
	EE_DUMP(glut_ctrl_2, SEEE_GLUT_X2);
	EE_DUMP(glut_ctrl_2, SEEE_GLUT_S2);
	EE_DUMP(glut_ctrl_2, SEEE_GLUT_Y2);
	EE_DUMP(glut_ctrl_3, SEEE_GLUT_X3);
	EE_DUMP(glut_ctrl_3, SEEE_GLUT_S3);
	EE_DUMP(glut_ctrl_3, SEEE_GLUT_Y3);
	EE_DUMP(glut_ctrl_4, SEEE_GLUT_X4);
	EE_DUMP(glut_ctrl_4, SEEE_GLUT_S4);
	EE_DUMP(glut_ctrl_4, SEEE_GLUT_Y4);
	EE_DUMP(glut_ctrl_5, SEEE_GLUT_S5);
	EE_DUMP(glut_ctrl_6, SEEE_GLUT_TH_OVR);
	EE_DUMP(glut_ctrl_6, SEEE_GLUT_TH_UND);
	EE_DUMP(glut_ctrl_6, SEEE_GLUT_TH_MIN);
	EE_DUMP(slnk_ctrl_2, SEEE_GLUT_LINK_EN);
	EE_DUMP(slnk_ctrl_2, SEEE_GLUT_SL_DEC_Y);
	EE_DUMP(clip_ctrl_3, SEEE_OVRSH_CLIP_STR);
	EE_DUMP(clip_ctrl_1, SEEE_DOT_TH);
	EE_DUMP(clip_ctrl_1, SEEE_DOT_REDUC_AMNT);
	EE_DUMP(clip_ctrl_2, SEEE_RESP_CLIP);
	EE_DUMP(clip_ctrl_2, SEEE_RESP_CLIP_LUMA_SPC_TH);
	EE_DUMP(clip_ctrl_2, SEEE_RESP_CLIP_LUMA_LWB);
	EE_DUMP(clip_ctrl_2, SEEE_RESP_CLIP_LUMA_UPB);
}

//===========================================================================

#define CLIP_NR3D(a,b,c)   ( (a<b)?  b : ((a>c)? c: a) )

#define CLIP_MIXER3(a,b,c)   ( (a<b)?  b : ((a>c)? c: a) )


// ===========================================================================================

MINT32 InterParam_MIXER3(double Ratio_L_L, double Ratio_L_U, double Ratio_U_L, double Ratio_U_U, MINT32 PARAM_L_L, MINT32 PARAM_L_U, MINT32 PARAM_U_L, MINT32 PARAM_U_U, MINT32 Method) {
    MINT32 InterPARAM = 0;


    switch(Method) {
    case 1:
        if( Ratio_L_L > Ratio_L_U)
		{
            if( Ratio_L_L > Ratio_U_L)
			{
                InterPARAM = PARAM_L_L;
        	}
        	else
			{
                InterPARAM = PARAM_U_L;
        	}
        }
        else
		{
            if( Ratio_L_L > Ratio_U_L)
			{
                InterPARAM = PARAM_L_U;
        	}
        	else
			{
                InterPARAM = PARAM_U_U;
        	}
        }
        break;
    case 0:
    default:
        InterPARAM = (MINT32)(Ratio_L_L * PARAM_L_L + Ratio_L_U * PARAM_L_U + Ratio_U_L * PARAM_U_L + Ratio_U_U * PARAM_U_U + 0.5);
        break;
    }

    return InterPARAM;
}




MVOID SmoothMIXER3(ISP_NVRAM_MIXER3_INT_T const& rParam,   //MIXER3 settings
                ISP_NVRAM_MIXER3_T& rSmoothMIXER3)   // Output
{
    MBOOL bSmoothMIXER3Debug = interpPropertyGet("debug.smooth_mixer3.enable", MFALSE);

    MINT32 u4RealISO = rParam.u4RealISO;
	MINT32 u4UpperISO = rParam.u4UpperISO;
	MINT32 u4LowerISO = rParam.u4LowerISO;
	MINT32 fRealZoom = rParam.i4ZoomRatio_x100;
	MINT32 fUpperZoom = rParam.u4UpperZoom;
	MINT32 fLowerZoom = rParam.u4LowerZoom;


	INTER_LOG_IF(bSmoothMIXER3Debug,"[%s()] u4RealISO: %d, u4UpperISO = %d, u4LowerISO = %d\n", __FUNCTION__, u4RealISO, u4UpperISO, u4LowerISO);
	INTER_LOG_IF(bSmoothMIXER3Debug,"[%s()] u4RealZoom: %d, u4UpperZoom = %d, u4LowerZoom = %d\n", __FUNCTION__, fRealZoom, fUpperZoom, fLowerZoom);

	// _L_L = Low ISO Low Zoom, _L_U = Low ISO Up Zoom, _U_L = Up ISO Low Zoom, _U_U = Up ISO Up Zoom

	double RATIO_ISO_lo = 0;
	double RATIO_ISO_up = 0;
	double RATIO_Zoom_lo = 0;
	double RATIO_Zoom_up = 0;

    //protection for out-of-bound


    if (u4RealISO <= u4LowerISO)
    {
        RATIO_ISO_lo = 1;
    }
	else if(u4RealISO >= u4UpperISO)
	{
		RATIO_ISO_lo = 0;
	}
	else
	{
		RATIO_ISO_lo = (double)(u4UpperISO - u4RealISO) / (double)(u4UpperISO - u4LowerISO);
	}

	RATIO_ISO_up = 1.0 - RATIO_ISO_lo;

	if (fRealZoom <= fLowerZoom)
    {
        RATIO_Zoom_lo = 1;
    }
	else if(fRealZoom >= fUpperZoom)
	{
		RATIO_Zoom_lo = 0;
	}
	else
	{
		RATIO_Zoom_lo = (double)(fUpperZoom - fRealZoom) / (double)(fUpperZoom - fLowerZoom);
	}

	RATIO_Zoom_up = 1.0 - RATIO_Zoom_lo;

	double RATIO_L_L = (double)(RATIO_ISO_lo * RATIO_Zoom_lo);
	double RATIO_L_U = (double)(RATIO_ISO_lo * RATIO_Zoom_up);
	double RATIO_U_L = (double)(RATIO_ISO_up * RATIO_Zoom_lo);
	double RATIO_U_U = (double)(RATIO_ISO_up * RATIO_Zoom_up);

	//  ====================================================================================================================================
    //  Get Lower ISO Low Zoom
    //  ====================================================================================================================================
    MINT32 MIX3_DT_L_L          = rParam.rLowerIso_LowerZoom.ctrl_0.bits.MIX3_DT;
	MINT32 MIX3_B1_L_L          = rParam.rLowerIso_LowerZoom.ctrl_0.bits.MIX3_B1;
	MINT32 MIX3_B0_L_L          = rParam.rLowerIso_LowerZoom.ctrl_0.bits.MIX3_B0;
	MINT32 MIX3_UV_DEFAULT_L_L          = rParam.rLowerIso_LowerZoom.ctrl_0.bits.MIX3_UV_DEFAULT;
	MINT32 MIX3_UV_EN_L_L          = rParam.rLowerIso_LowerZoom.ctrl_0.bits.MIX3_UV_EN;
	MINT32 MIX3_Y_DEFAULT_L_L          = rParam.rLowerIso_LowerZoom.ctrl_0.bits.MIX3_Y_DEFAULT;
	MINT32 MIX3_Y_EN_L_L          = rParam.rLowerIso_LowerZoom.ctrl_0.bits.MIX3_Y_EN;
	MINT32 MIX3_WT_SEL_L_L          = rParam.rLowerIso_LowerZoom.ctrl_0.bits.MIX3_WT_SEL;
	MINT32 MIX3_M1_L_L          = rParam.rLowerIso_LowerZoom.ctrl_1.bits.MIX3_M1;
	MINT32 MIX3_M0_L_L          = rParam.rLowerIso_LowerZoom.ctrl_1.bits.MIX3_M0;

	//  ====================================================================================================================================
    //  Get Lower ISO Up Zoom
    //  ====================================================================================================================================
    MINT32 MIX3_DT_L_U          = rParam.rLowerIso_UpperZoom.ctrl_0.bits.MIX3_DT;
	MINT32 MIX3_B1_L_U          = rParam.rLowerIso_UpperZoom.ctrl_0.bits.MIX3_B1;
	MINT32 MIX3_B0_L_U          = rParam.rLowerIso_UpperZoom.ctrl_0.bits.MIX3_B0;
	MINT32 MIX3_UV_DEFAULT_L_U          = rParam.rLowerIso_UpperZoom.ctrl_0.bits.MIX3_UV_DEFAULT;
	MINT32 MIX3_UV_EN_L_U          = rParam.rLowerIso_UpperZoom.ctrl_0.bits.MIX3_UV_EN;
	MINT32 MIX3_Y_DEFAULT_L_U          = rParam.rLowerIso_UpperZoom.ctrl_0.bits.MIX3_Y_DEFAULT;
	MINT32 MIX3_Y_EN_L_U          = rParam.rLowerIso_UpperZoom.ctrl_0.bits.MIX3_Y_EN;
	MINT32 MIX3_WT_SEL_L_U          = rParam.rLowerIso_UpperZoom.ctrl_0.bits.MIX3_WT_SEL;
	MINT32 MIX3_M1_L_U          = rParam.rLowerIso_UpperZoom.ctrl_1.bits.MIX3_M1;
	MINT32 MIX3_M0_L_U          = rParam.rLowerIso_UpperZoom.ctrl_1.bits.MIX3_M0;

	//  ====================================================================================================================================
    //  Get Up ISO Low Zoom
    //  ====================================================================================================================================
    MINT32 MIX3_DT_U_L          = rParam.rUpperIso_LowerZoom.ctrl_0.bits.MIX3_DT;
	MINT32 MIX3_B1_U_L          = rParam.rUpperIso_LowerZoom.ctrl_0.bits.MIX3_B1;
	MINT32 MIX3_B0_U_L          = rParam.rUpperIso_LowerZoom.ctrl_0.bits.MIX3_B0;
	MINT32 MIX3_UV_DEFAULT_U_L          = rParam.rUpperIso_LowerZoom.ctrl_0.bits.MIX3_UV_DEFAULT;
	MINT32 MIX3_UV_EN_U_L          = rParam.rUpperIso_LowerZoom.ctrl_0.bits.MIX3_UV_EN;
	MINT32 MIX3_Y_DEFAULT_U_L          = rParam.rUpperIso_LowerZoom.ctrl_0.bits.MIX3_Y_DEFAULT;
	MINT32 MIX3_Y_EN_U_L          = rParam.rUpperIso_LowerZoom.ctrl_0.bits.MIX3_Y_EN;
	MINT32 MIX3_WT_SEL_U_L          = rParam.rUpperIso_LowerZoom.ctrl_0.bits.MIX3_WT_SEL;
	MINT32 MIX3_M1_U_L          = rParam.rUpperIso_LowerZoom.ctrl_1.bits.MIX3_M1;
	MINT32 MIX3_M0_U_L          = rParam.rUpperIso_LowerZoom.ctrl_1.bits.MIX3_M0;

	//  ====================================================================================================================================
    //  Get Up ISO Up Zoom
    //  ====================================================================================================================================
    MINT32 MIX3_DT_U_U          = rParam.rUpperIso_UpperZoom.ctrl_0.bits.MIX3_DT;
	MINT32 MIX3_B1_U_U          = rParam.rUpperIso_UpperZoom.ctrl_0.bits.MIX3_B1;
	MINT32 MIX3_B0_U_U          = rParam.rUpperIso_UpperZoom.ctrl_0.bits.MIX3_B0;
	MINT32 MIX3_UV_DEFAULT_U_U          = rParam.rUpperIso_UpperZoom.ctrl_0.bits.MIX3_UV_DEFAULT;
	MINT32 MIX3_UV_EN_U_U          = rParam.rUpperIso_UpperZoom.ctrl_0.bits.MIX3_UV_EN;
	MINT32 MIX3_Y_DEFAULT_U_U          = rParam.rUpperIso_UpperZoom.ctrl_0.bits.MIX3_Y_DEFAULT;
	MINT32 MIX3_Y_EN_U_U          = rParam.rUpperIso_UpperZoom.ctrl_0.bits.MIX3_Y_EN;
	MINT32 MIX3_WT_SEL_U_U          = rParam.rUpperIso_UpperZoom.ctrl_0.bits.MIX3_WT_SEL;
	MINT32 MIX3_M1_U_U          = rParam.rUpperIso_UpperZoom.ctrl_1.bits.MIX3_M1;
	MINT32 MIX3_M0_U_U          = rParam.rUpperIso_UpperZoom.ctrl_1.bits.MIX3_M0;

	//  ====================================================================================================================================
    //  Start Parameter Interpolation
    //  ====================================================================================================================================


    //  Registers that can be interpolated
    MINT32 MIX3_DT            = MIX3_DT_L_L; //use low ISO Low Zoom
	MINT32 MIX3_B1            = InterParam_MIXER3(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, MIX3_B1_L_L, MIX3_B1_L_U, MIX3_B1_U_L, MIX3_B1_U_U,0);
	MINT32 MIX3_B0            = InterParam_MIXER3(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, MIX3_B0_L_L, MIX3_B0_L_U, MIX3_B0_U_L, MIX3_B0_U_U,0);
	MINT32 MIX3_UV_DEFAULT            = InterParam_MIXER3(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, MIX3_UV_DEFAULT_L_L, MIX3_UV_DEFAULT_L_U, MIX3_UV_DEFAULT_U_L, MIX3_UV_DEFAULT_U_U,1);
	MINT32 MIX3_UV_EN            = InterParam_MIXER3(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, MIX3_UV_EN_L_L, MIX3_UV_EN_L_U, MIX3_UV_EN_U_L, MIX3_UV_EN_U_U,1);
	MINT32 MIX3_Y_DEFAULT            = InterParam_MIXER3(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, MIX3_Y_DEFAULT_L_L, MIX3_Y_DEFAULT_L_U, MIX3_Y_DEFAULT_U_L, MIX3_Y_DEFAULT_U_U,1);
	MINT32 MIX3_Y_EN            = InterParam_MIXER3(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, MIX3_Y_EN_L_L, MIX3_Y_EN_L_U, MIX3_Y_EN_U_L, MIX3_Y_EN_U_U,1);
	MINT32 MIX3_WT_SEL            = InterParam_MIXER3(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, MIX3_WT_SEL_L_L, MIX3_WT_SEL_L_U, MIX3_WT_SEL_U_L, MIX3_WT_SEL_U_U,0);
	MINT32 MIX3_M1            = MIX3_M1_L_L; //use low ISO Low Zoom
	MINT32 MIX3_M0            = InterParam_MIXER3(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, MIX3_M0_L_L, MIX3_M0_L_U, MIX3_M0_U_L, MIX3_M0_U_U,0);

	// Calculation for M0

	MINT32 MIX3_M0_cal;
	if(MIX3_DT == 0)
	{
		MIX3_M0_cal = 0;
	}
	else
	{
		MIX3_M0_cal = MIX3_M1 - (256 / MIX3_DT) - 1;
	}

	MIX3_M0_cal = CLIP_MIXER3(MIX3_M0_cal,32,255);

	MIX3_M0 = (MIX3_M0 > MIX3_M0_cal) ? MIX3_M0 : MIX3_M0_cal;

	//  ====================================================================================================================================
    //  Set Smooth ISO setting
    //  ====================================================================================================================================
    rSmoothMIXER3.ctrl_0.bits.MIX3_DT         = MIX3_DT;
	rSmoothMIXER3.ctrl_0.bits.MIX3_B1         = MIX3_B1;
	rSmoothMIXER3.ctrl_0.bits.MIX3_B0         = MIX3_B0;
	rSmoothMIXER3.ctrl_0.bits.MIX3_UV_DEFAULT         = MIX3_UV_DEFAULT;
	rSmoothMIXER3.ctrl_0.bits.MIX3_UV_EN         = MIX3_UV_EN;
	rSmoothMIXER3.ctrl_0.bits.MIX3_Y_DEFAULT         = MIX3_Y_DEFAULT;
	rSmoothMIXER3.ctrl_0.bits.MIX3_Y_EN         = MIX3_Y_EN;
	rSmoothMIXER3.ctrl_0.bits.MIX3_WT_SEL         = MIX3_WT_SEL;
	rSmoothMIXER3.ctrl_1.bits.MIX3_M1         = MIX3_M1;
	rSmoothMIXER3.ctrl_1.bits.MIX3_M0         = MIX3_M0;


	#define MIXER3_DUMP(item1, item2)   \
        do{                      \
            INTER_LOG_IF(bSmoothMIXER3Debug, "[S, L_L, L_U, U_L, U_U]%22s = %d, %d, %d, %d, %d", #item2, rSmoothMIXER3.item1.bits.item2, rParam.rLowerIso_LowerZoom.item1.bits.item2, rParam.rLowerIso_UpperZoom.item1.bits.item2, rParam.rUpperIso_LowerZoom.item1.bits.item2, rParam.rUpperIso_UpperZoom.item1.bits.item2); \
        }while(0)

    MIXER3_DUMP(ctrl_0, MIX3_DT);
	MIXER3_DUMP(ctrl_0, MIX3_B1);
	MIXER3_DUMP(ctrl_0, MIX3_B0);
	MIXER3_DUMP(ctrl_0, MIX3_UV_DEFAULT);
	MIXER3_DUMP(ctrl_0, MIX3_UV_EN);
	MIXER3_DUMP(ctrl_0, MIX3_Y_DEFAULT);
	MIXER3_DUMP(ctrl_0, MIX3_Y_EN);
	MIXER3_DUMP(ctrl_0, MIX3_WT_SEL);
	MIXER3_DUMP(ctrl_1, MIX3_M1);
	MIXER3_DUMP(ctrl_1, MIX3_M0);



}

MINT32 InterParam_MFB(double Ratio_L_L, double Ratio_L_U, double Ratio_U_L, double Ratio_U_U, MINT32 PARAM_L_L, MINT32 PARAM_L_U, MINT32 PARAM_U_L, MINT32 PARAM_U_U, MINT32 Method) {
    MINT32 InterPARAM = 0;


    switch(Method) {
    case 1:
        if( Ratio_L_L > Ratio_L_U)
		{
            if( Ratio_L_L > Ratio_U_L)
			{
                InterPARAM = PARAM_L_L;
        	}
        	else
			{
                InterPARAM = PARAM_U_L;
        	}
        }
        else
		{
            if( Ratio_L_L > Ratio_U_L)
			{
                InterPARAM = PARAM_L_U;
        	}
        	else
			{
                InterPARAM = PARAM_U_U;
        	}
        }
        break;
    case 0:
    default:
        InterPARAM = (MINT32)(Ratio_L_L * PARAM_L_L + Ratio_L_U * PARAM_L_U + Ratio_U_L * PARAM_U_L + Ratio_U_U * PARAM_U_U + 0.5);
        break;
    }

    return InterPARAM;
}




MVOID SmoothMFB(ISP_NVRAM_MFB_INT_T const& rParam,   //MFB settings
                ISP_NVRAM_MFB_T& rSmoothMFB)  // Output

{       // Output
    MBOOL bSmoothMFBDebug = interpPropertyGet("debug.smooth_mfb.enable", MFALSE);

    MINT32 u4RealISO = rParam.u4RealISO;
	MINT32 u4UpperISO = rParam.u4UpperISO;
	MINT32 u4LowerISO = rParam.u4LowerISO;
	MINT32 fRealZoom = rParam.i4ZoomRatio_x100;
	MINT32 fUpperZoom = rParam.u4UpperZoom;
	MINT32 fLowerZoom = rParam.u4LowerZoom;


	INTER_LOG_IF(bSmoothMFBDebug,"[%s()] u4RealISO: %d, u4UpperISO = %d, u4LowerISO = %d\n", __FUNCTION__, u4RealISO, u4UpperISO, u4LowerISO);
	INTER_LOG_IF(bSmoothMFBDebug,"[%s()] u4RealZoom: %d, u4UpperZoom = %d, u4LowerZoom = %d\n", __FUNCTION__, fRealZoom, fUpperZoom, fLowerZoom);

	// _L_L = Low ISO Low Zoom, _L_U = Low ISO Up Zoom, _U_L = Up ISO Low Zoom, _U_U = Up ISO Up Zoom

	double RATIO_ISO_lo = 0;
	double RATIO_ISO_up = 0;
	double RATIO_Zoom_lo = 0;
	double RATIO_Zoom_up = 0;

    //protection for out-of-bound


    if (u4RealISO <= u4LowerISO)
    {
        RATIO_ISO_lo = 1;
    }
	else if(u4RealISO >= u4UpperISO)
	{
		RATIO_ISO_lo = 0;
	}
	else
	{
		RATIO_ISO_lo = (double)(u4UpperISO - u4RealISO) / (double)(u4UpperISO - u4LowerISO);
	}

	RATIO_ISO_up = 1.0 - RATIO_ISO_lo;

	if (fRealZoom <= fLowerZoom)
    {
        RATIO_Zoom_lo = 1;
    }
	else if(fRealZoom >= fUpperZoom)
	{
		RATIO_Zoom_lo = 0;
	}
	else
	{
		RATIO_Zoom_lo = (double)(fUpperZoom - fRealZoom) / (double)(fUpperZoom - fLowerZoom);
	}

	RATIO_Zoom_up = 1.0 - RATIO_Zoom_lo;

	double RATIO_L_L = (double)(RATIO_ISO_lo * RATIO_Zoom_lo);
	double RATIO_L_U = (double)(RATIO_ISO_lo * RATIO_Zoom_up);
	double RATIO_U_L = (double)(RATIO_ISO_up * RATIO_Zoom_lo);
	double RATIO_U_U = (double)(RATIO_ISO_up * RATIO_Zoom_up);

	//  ====================================================================================================================================
    //  Get Lower ISO Low Zoom
    //  ====================================================================================================================================
    MINT32 BLD_LL_CLIP_TH2_L_L          = rParam.rLowerIso_LowerZoom.ll_con1.bits.BLD_LL_CLIP_TH2;
	MINT32 BLD_LL_CLIP_TH1_L_L          = rParam.rLowerIso_LowerZoom.ll_con1.bits.BLD_LL_CLIP_TH1;
	MINT32 BLD_LL_FLT_WT_MODE2_L_L          = rParam.rLowerIso_LowerZoom.ll_con1.bits.BLD_LL_FLT_WT_MODE2;
	MINT32 BLD_LL_FLT_WT_MODE1_L_L          = rParam.rLowerIso_LowerZoom.ll_con1.bits.BLD_LL_FLT_WT_MODE1;
	MINT32 BLD_LL_FLT_MODE_L_L          = rParam.rLowerIso_LowerZoom.ll_con1.bits.BLD_LL_FLT_MODE;
	MINT32 BLD_LL_TH2_L_L          = rParam.rLowerIso_LowerZoom.ll_con2.bits.BLD_LL_TH2;
	MINT32 BLD_LL_TH1_L_L          = rParam.rLowerIso_LowerZoom.ll_con2.bits.BLD_LL_TH1;
	MINT32 BLD_LL_DT1_L_L          = rParam.rLowerIso_LowerZoom.ll_con2.bits.BLD_LL_DT1;
	MINT32 BLD_LL_MAX_WT_L_L          = rParam.rLowerIso_LowerZoom.ll_con2.bits.BLD_LL_MAX_WT;
	MINT32 BLD_MBD_MAX_WT_L_L          = rParam.rLowerIso_LowerZoom.mbd_con0.bits.BLD_MBD_MAX_WT;
	MINT32 BLD_MBD_YL1_THL_0_L_L          = rParam.rLowerIso_LowerZoom.mbd_con1.bits.BLD_MBD_YL1_THL_0;
	MINT32 BLD_MBD_YL1_THL_1_L_L          = rParam.rLowerIso_LowerZoom.mbd_con1.bits.BLD_MBD_YL1_THL_1;
	MINT32 BLD_MBD_YL1_THL_2_L_L          = rParam.rLowerIso_LowerZoom.mbd_con1.bits.BLD_MBD_YL1_THL_2;
	MINT32 BLD_MBD_YL1_THL_3_L_L          = rParam.rLowerIso_LowerZoom.mbd_con1.bits.BLD_MBD_YL1_THL_3;
	MINT32 BLD_MBD_YL1_THL_4_L_L          = rParam.rLowerIso_LowerZoom.mbd_con2.bits.BLD_MBD_YL1_THL_4;
	MINT32 BLD_MBD_YL1_THH_0_L_L          = rParam.rLowerIso_LowerZoom.mbd_con2.bits.BLD_MBD_YL1_THH_0;
	MINT32 BLD_MBD_YL1_THH_1_L_L          = rParam.rLowerIso_LowerZoom.mbd_con2.bits.BLD_MBD_YL1_THH_1;
	MINT32 BLD_MBD_YL1_THH_2_L_L          = rParam.rLowerIso_LowerZoom.mbd_con2.bits.BLD_MBD_YL1_THH_2;
	MINT32 BLD_MBD_YL1_THH_3_L_L          = rParam.rLowerIso_LowerZoom.mbd_con3.bits.BLD_MBD_YL1_THH_3;
	MINT32 BLD_MBD_YL1_THH_4_L_L          = rParam.rLowerIso_LowerZoom.mbd_con3.bits.BLD_MBD_YL1_THH_4;
	MINT32 BLD_MBD_YL2_THL_0_L_L          = rParam.rLowerIso_LowerZoom.mbd_con3.bits.BLD_MBD_YL2_THL_0;
	MINT32 BLD_MBD_YL2_THL_1_L_L          = rParam.rLowerIso_LowerZoom.mbd_con3.bits.BLD_MBD_YL2_THL_1;
	MINT32 BLD_MBD_YL2_THL_2_L_L          = rParam.rLowerIso_LowerZoom.mbd_con4.bits.BLD_MBD_YL2_THL_2;
	MINT32 BLD_MBD_YL2_THL_3_L_L          = rParam.rLowerIso_LowerZoom.mbd_con4.bits.BLD_MBD_YL2_THL_3;
	MINT32 BLD_MBD_YL2_THL_4_L_L          = rParam.rLowerIso_LowerZoom.mbd_con4.bits.BLD_MBD_YL2_THL_4;
	MINT32 BLD_MBD_YL2_THH_0_L_L          = rParam.rLowerIso_LowerZoom.mbd_con4.bits.BLD_MBD_YL2_THH_0;
	MINT32 BLD_MBD_YL2_THH_1_L_L          = rParam.rLowerIso_LowerZoom.mbd_con5.bits.BLD_MBD_YL2_THH_1;
	MINT32 BLD_MBD_YL2_THH_2_L_L          = rParam.rLowerIso_LowerZoom.mbd_con5.bits.BLD_MBD_YL2_THH_2;
	MINT32 BLD_MBD_YL2_THH_3_L_L          = rParam.rLowerIso_LowerZoom.mbd_con5.bits.BLD_MBD_YL2_THH_3;
	MINT32 BLD_MBD_YL2_THH_4_L_L          = rParam.rLowerIso_LowerZoom.mbd_con5.bits.BLD_MBD_YL2_THH_4;
	MINT32 BLD_MBD_YL3_THL_0_L_L          = rParam.rLowerIso_LowerZoom.mbd_con6.bits.BLD_MBD_YL3_THL_0;
	MINT32 BLD_MBD_YL3_THL_1_L_L          = rParam.rLowerIso_LowerZoom.mbd_con6.bits.BLD_MBD_YL3_THL_1;
	MINT32 BLD_MBD_YL3_THL_2_L_L          = rParam.rLowerIso_LowerZoom.mbd_con6.bits.BLD_MBD_YL3_THL_2;
	MINT32 BLD_MBD_YL3_THL_3_L_L          = rParam.rLowerIso_LowerZoom.mbd_con6.bits.BLD_MBD_YL3_THL_3;
	MINT32 BLD_MBD_YL3_THL_4_L_L          = rParam.rLowerIso_LowerZoom.mbd_con7.bits.BLD_MBD_YL3_THL_4;
	MINT32 BLD_MBD_YL3_THH_0_L_L          = rParam.rLowerIso_LowerZoom.mbd_con7.bits.BLD_MBD_YL3_THH_0;
	MINT32 BLD_MBD_YL3_THH_1_L_L          = rParam.rLowerIso_LowerZoom.mbd_con7.bits.BLD_MBD_YL3_THH_1;
	MINT32 BLD_MBD_YL3_THH_2_L_L          = rParam.rLowerIso_LowerZoom.mbd_con7.bits.BLD_MBD_YL3_THH_2;
	MINT32 BLD_MBD_YL3_THH_3_L_L          = rParam.rLowerIso_LowerZoom.mbd_con8.bits.BLD_MBD_YL3_THH_3;
	MINT32 BLD_MBD_YL3_THH_4_L_L          = rParam.rLowerIso_LowerZoom.mbd_con8.bits.BLD_MBD_YL3_THH_4;
	MINT32 BLD_MBD_CL1_THL_0_L_L          = rParam.rLowerIso_LowerZoom.mbd_con8.bits.BLD_MBD_CL1_THL_0;
	MINT32 BLD_MBD_CL1_THL_1_L_L          = rParam.rLowerIso_LowerZoom.mbd_con8.bits.BLD_MBD_CL1_THL_1;
	MINT32 BLD_MBD_CL1_THL_2_L_L          = rParam.rLowerIso_LowerZoom.mbd_con9.bits.BLD_MBD_CL1_THL_2;
	MINT32 BLD_MBD_CL1_THL_3_L_L          = rParam.rLowerIso_LowerZoom.mbd_con9.bits.BLD_MBD_CL1_THL_3;
	MINT32 BLD_MBD_CL1_THL_4_L_L          = rParam.rLowerIso_LowerZoom.mbd_con9.bits.BLD_MBD_CL1_THL_4;
	MINT32 BLD_MBD_CL1_THH_0_L_L          = rParam.rLowerIso_LowerZoom.mbd_con9.bits.BLD_MBD_CL1_THH_0;
	MINT32 BLD_MBD_CL1_THH_1_L_L          = rParam.rLowerIso_LowerZoom.mbd_con10.bits.BLD_MBD_CL1_THH_1;
	MINT32 BLD_MBD_CL1_THH_2_L_L          = rParam.rLowerIso_LowerZoom.mbd_con10.bits.BLD_MBD_CL1_THH_2;
	MINT32 BLD_MBD_CL1_THH_3_L_L          = rParam.rLowerIso_LowerZoom.mbd_con10.bits.BLD_MBD_CL1_THH_3;
	MINT32 BLD_MBD_CL1_THH_4_L_L          = rParam.rLowerIso_LowerZoom.mbd_con10.bits.BLD_MBD_CL1_THH_4;

	// use dynamic
	MINT32 BLD_MODE_L_L                   = rParam.rLowerIso_LowerZoom.con.bits.BLD_MODE;
	MINT32 BLD_LL_BRZ_EN_L_L                   = rParam.rLowerIso_LowerZoom.con.bits.BLD_LL_BRZ_EN;
	MINT32 BLD_LL_DB_EN_L_L                   = rParam.rLowerIso_LowerZoom.con.bits.BLD_LL_DB_EN;
	MINT32 BLD_MBD_WT_EN_L_L                   = rParam.rLowerIso_LowerZoom.con.bits.BLD_MBD_WT_EN;
	MINT32 BLD_SR_WT_EN_L_L                   = rParam.rLowerIso_LowerZoom.con.bits.BLD_SR_WT_EN;
	MINT32 BLD_LL_TH_E_L_L                   = rParam.rLowerIso_LowerZoom.con.bits.BLD_LL_TH_E;
	MINT32 BLD_LL_DB_XDIST_L_L          = rParam.rLowerIso_LowerZoom.ll_con4.bits.BLD_LL_DB_XDIST;
	MINT32 BLD_LL_DB_YDIST_L_L          = rParam.rLowerIso_LowerZoom.ll_con4.bits.BLD_LL_DB_YDIST;
	MINT32 BLD_LL_GRAD_R1_L_L          = rParam.rLowerIso_LowerZoom.ll_con5.bits.BLD_LL_GRAD_R1;
	MINT32 BLD_LL_GRAD_R2_L_L          = rParam.rLowerIso_LowerZoom.ll_con5.bits.BLD_LL_GRAD_R2;
	MINT32 BLD_LL_TH3_L_L          = rParam.rLowerIso_LowerZoom.ll_con5.bits.BLD_LL_TH3;
	MINT32 BLD_LL_TH4_L_L          = rParam.rLowerIso_LowerZoom.ll_con5.bits.BLD_LL_TH4;
	MINT32 BLD_LL_TH5_L_L          = rParam.rLowerIso_LowerZoom.ll_con6.bits.BLD_LL_TH5;
	MINT32 BLD_LL_TH6_L_L         = rParam.rLowerIso_LowerZoom.ll_con6.bits.BLD_LL_TH6;
	MINT32 BLD_LL_TH7_L_L          = rParam.rLowerIso_LowerZoom.ll_con6.bits.BLD_LL_TH7;
	MINT32 BLD_LL_TH8_L_L          = rParam.rLowerIso_LowerZoom.ll_con6.bits.BLD_LL_TH8;
	MINT32 BLD_LL_C1_L_L          = rParam.rLowerIso_LowerZoom.ll_con7.bits.BLD_LL_C1;
	MINT32 BLD_LL_C2_L_L          = rParam.rLowerIso_LowerZoom.ll_con7.bits.BLD_LL_C2;
	MINT32 BLD_LL_C3_L_L          = rParam.rLowerIso_LowerZoom.ll_con7.bits.BLD_LL_C3;
	MINT32 BLD_LL_SU1_L_L          = rParam.rLowerIso_LowerZoom.ll_con8.bits.BLD_LL_SU1;
	MINT32 BLD_LL_SU2_L_L          = rParam.rLowerIso_LowerZoom.ll_con8.bits.BLD_LL_SU2;
	MINT32 BLD_LL_SU3_L_L          = rParam.rLowerIso_LowerZoom.ll_con8.bits.BLD_LL_SU3;
	MINT32 BLD_LL_SL1_L_L          = rParam.rLowerIso_LowerZoom.ll_con9.bits.BLD_LL_SL1;
	MINT32 BLD_LL_SL2_L_L          = rParam.rLowerIso_LowerZoom.ll_con9.bits.BLD_LL_SL2;
	MINT32 BLD_LL_SL3_L_L          = rParam.rLowerIso_LowerZoom.ll_con9.bits.BLD_LL_SL3;
	MINT32 BLD_CONF_MAP_EN_L_L          = rParam.rLowerIso_LowerZoom.ll_con10.bits.BLD_CONF_MAP_EN;
	MINT32 BLD_LL_GRAD_EN_L_L          = rParam.rLowerIso_LowerZoom.ll_con10.bits.BLD_LL_GRAD_EN;
	MINT32 BLD_LL_GRAD_ENTH_L_L          = rParam.rLowerIso_LowerZoom.ll_con10.bits.BLD_LL_GRAD_ENTH;

	//  ====================================================================================================================================
    //  Get Lower ISO Up Zoom
    //  ====================================================================================================================================
    MINT32 BLD_LL_CLIP_TH2_L_U          = rParam.rLowerIso_UpperZoom.ll_con1.bits.BLD_LL_CLIP_TH2;
	MINT32 BLD_LL_CLIP_TH1_L_U          = rParam.rLowerIso_UpperZoom.ll_con1.bits.BLD_LL_CLIP_TH1;
	MINT32 BLD_LL_FLT_WT_MODE2_L_U          = rParam.rLowerIso_UpperZoom.ll_con1.bits.BLD_LL_FLT_WT_MODE2;
	MINT32 BLD_LL_FLT_WT_MODE1_L_U          = rParam.rLowerIso_UpperZoom.ll_con1.bits.BLD_LL_FLT_WT_MODE1;
	MINT32 BLD_LL_FLT_MODE_L_U          = rParam.rLowerIso_UpperZoom.ll_con1.bits.BLD_LL_FLT_MODE;
	MINT32 BLD_LL_TH2_L_U          = rParam.rLowerIso_UpperZoom.ll_con2.bits.BLD_LL_TH2;
	MINT32 BLD_LL_TH1_L_U          = rParam.rLowerIso_UpperZoom.ll_con2.bits.BLD_LL_TH1;
	MINT32 BLD_LL_DT1_L_U          = rParam.rLowerIso_UpperZoom.ll_con2.bits.BLD_LL_DT1;
	MINT32 BLD_LL_MAX_WT_L_U          = rParam.rLowerIso_UpperZoom.ll_con2.bits.BLD_LL_MAX_WT;
	MINT32 BLD_MBD_MAX_WT_L_U          = rParam.rLowerIso_UpperZoom.mbd_con0.bits.BLD_MBD_MAX_WT;
	MINT32 BLD_MBD_YL1_THL_0_L_U          = rParam.rLowerIso_UpperZoom.mbd_con1.bits.BLD_MBD_YL1_THL_0;
	MINT32 BLD_MBD_YL1_THL_1_L_U          = rParam.rLowerIso_UpperZoom.mbd_con1.bits.BLD_MBD_YL1_THL_1;
	MINT32 BLD_MBD_YL1_THL_2_L_U          = rParam.rLowerIso_UpperZoom.mbd_con1.bits.BLD_MBD_YL1_THL_2;
	MINT32 BLD_MBD_YL1_THL_3_L_U          = rParam.rLowerIso_UpperZoom.mbd_con1.bits.BLD_MBD_YL1_THL_3;
	MINT32 BLD_MBD_YL1_THL_4_L_U          = rParam.rLowerIso_UpperZoom.mbd_con2.bits.BLD_MBD_YL1_THL_4;
	MINT32 BLD_MBD_YL1_THH_0_L_U          = rParam.rLowerIso_UpperZoom.mbd_con2.bits.BLD_MBD_YL1_THH_0;
	MINT32 BLD_MBD_YL1_THH_1_L_U          = rParam.rLowerIso_UpperZoom.mbd_con2.bits.BLD_MBD_YL1_THH_1;
	MINT32 BLD_MBD_YL1_THH_2_L_U          = rParam.rLowerIso_UpperZoom.mbd_con2.bits.BLD_MBD_YL1_THH_2;
	MINT32 BLD_MBD_YL1_THH_3_L_U          = rParam.rLowerIso_UpperZoom.mbd_con3.bits.BLD_MBD_YL1_THH_3;
	MINT32 BLD_MBD_YL1_THH_4_L_U          = rParam.rLowerIso_UpperZoom.mbd_con3.bits.BLD_MBD_YL1_THH_4;
	MINT32 BLD_MBD_YL2_THL_0_L_U          = rParam.rLowerIso_UpperZoom.mbd_con3.bits.BLD_MBD_YL2_THL_0;
	MINT32 BLD_MBD_YL2_THL_1_L_U          = rParam.rLowerIso_UpperZoom.mbd_con3.bits.BLD_MBD_YL2_THL_1;
	MINT32 BLD_MBD_YL2_THL_2_L_U          = rParam.rLowerIso_UpperZoom.mbd_con4.bits.BLD_MBD_YL2_THL_2;
	MINT32 BLD_MBD_YL2_THL_3_L_U          = rParam.rLowerIso_UpperZoom.mbd_con4.bits.BLD_MBD_YL2_THL_3;
	MINT32 BLD_MBD_YL2_THL_4_L_U          = rParam.rLowerIso_UpperZoom.mbd_con4.bits.BLD_MBD_YL2_THL_4;
	MINT32 BLD_MBD_YL2_THH_0_L_U          = rParam.rLowerIso_UpperZoom.mbd_con4.bits.BLD_MBD_YL2_THH_0;
	MINT32 BLD_MBD_YL2_THH_1_L_U          = rParam.rLowerIso_UpperZoom.mbd_con5.bits.BLD_MBD_YL2_THH_1;
	MINT32 BLD_MBD_YL2_THH_2_L_U          = rParam.rLowerIso_UpperZoom.mbd_con5.bits.BLD_MBD_YL2_THH_2;
	MINT32 BLD_MBD_YL2_THH_3_L_U          = rParam.rLowerIso_UpperZoom.mbd_con5.bits.BLD_MBD_YL2_THH_3;
	MINT32 BLD_MBD_YL2_THH_4_L_U          = rParam.rLowerIso_UpperZoom.mbd_con5.bits.BLD_MBD_YL2_THH_4;
	MINT32 BLD_MBD_YL3_THL_0_L_U          = rParam.rLowerIso_UpperZoom.mbd_con6.bits.BLD_MBD_YL3_THL_0;
	MINT32 BLD_MBD_YL3_THL_1_L_U          = rParam.rLowerIso_UpperZoom.mbd_con6.bits.BLD_MBD_YL3_THL_1;
	MINT32 BLD_MBD_YL3_THL_2_L_U          = rParam.rLowerIso_UpperZoom.mbd_con6.bits.BLD_MBD_YL3_THL_2;
	MINT32 BLD_MBD_YL3_THL_3_L_U          = rParam.rLowerIso_UpperZoom.mbd_con6.bits.BLD_MBD_YL3_THL_3;
	MINT32 BLD_MBD_YL3_THL_4_L_U          = rParam.rLowerIso_UpperZoom.mbd_con7.bits.BLD_MBD_YL3_THL_4;
	MINT32 BLD_MBD_YL3_THH_0_L_U          = rParam.rLowerIso_UpperZoom.mbd_con7.bits.BLD_MBD_YL3_THH_0;
	MINT32 BLD_MBD_YL3_THH_1_L_U          = rParam.rLowerIso_UpperZoom.mbd_con7.bits.BLD_MBD_YL3_THH_1;
	MINT32 BLD_MBD_YL3_THH_2_L_U          = rParam.rLowerIso_UpperZoom.mbd_con7.bits.BLD_MBD_YL3_THH_2;
	MINT32 BLD_MBD_YL3_THH_3_L_U          = rParam.rLowerIso_UpperZoom.mbd_con8.bits.BLD_MBD_YL3_THH_3;
	MINT32 BLD_MBD_YL3_THH_4_L_U          = rParam.rLowerIso_UpperZoom.mbd_con8.bits.BLD_MBD_YL3_THH_4;
	MINT32 BLD_MBD_CL1_THL_0_L_U          = rParam.rLowerIso_UpperZoom.mbd_con8.bits.BLD_MBD_CL1_THL_0;
	MINT32 BLD_MBD_CL1_THL_1_L_U          = rParam.rLowerIso_UpperZoom.mbd_con8.bits.BLD_MBD_CL1_THL_1;
	MINT32 BLD_MBD_CL1_THL_2_L_U          = rParam.rLowerIso_UpperZoom.mbd_con9.bits.BLD_MBD_CL1_THL_2;
	MINT32 BLD_MBD_CL1_THL_3_L_U          = rParam.rLowerIso_UpperZoom.mbd_con9.bits.BLD_MBD_CL1_THL_3;
	MINT32 BLD_MBD_CL1_THL_4_L_U          = rParam.rLowerIso_UpperZoom.mbd_con9.bits.BLD_MBD_CL1_THL_4;
	MINT32 BLD_MBD_CL1_THH_0_L_U          = rParam.rLowerIso_UpperZoom.mbd_con9.bits.BLD_MBD_CL1_THH_0;
	MINT32 BLD_MBD_CL1_THH_1_L_U          = rParam.rLowerIso_UpperZoom.mbd_con10.bits.BLD_MBD_CL1_THH_1;
	MINT32 BLD_MBD_CL1_THH_2_L_U          = rParam.rLowerIso_UpperZoom.mbd_con10.bits.BLD_MBD_CL1_THH_2;
	MINT32 BLD_MBD_CL1_THH_3_L_U          = rParam.rLowerIso_UpperZoom.mbd_con10.bits.BLD_MBD_CL1_THH_3;
	MINT32 BLD_MBD_CL1_THH_4_L_U          = rParam.rLowerIso_UpperZoom.mbd_con10.bits.BLD_MBD_CL1_THH_4;

	// use dynamic
	MINT32 BLD_MODE_L_U                   = rParam.rLowerIso_UpperZoom.con.bits.BLD_MODE;
	MINT32 BLD_LL_BRZ_EN_L_U                   = rParam.rLowerIso_UpperZoom.con.bits.BLD_LL_BRZ_EN;
	MINT32 BLD_LL_DB_EN_L_U                   = rParam.rLowerIso_UpperZoom.con.bits.BLD_LL_DB_EN;
	MINT32 BLD_MBD_WT_EN_L_U                   = rParam.rLowerIso_UpperZoom.con.bits.BLD_MBD_WT_EN;
	MINT32 BLD_SR_WT_EN_L_U                   = rParam.rLowerIso_UpperZoom.con.bits.BLD_SR_WT_EN;
	MINT32 BLD_LL_TH_E_L_U                   = rParam.rLowerIso_UpperZoom.con.bits.BLD_LL_TH_E;
	MINT32 BLD_LL_DB_XDIST_L_U          = rParam.rLowerIso_UpperZoom.ll_con4.bits.BLD_LL_DB_XDIST;
	MINT32 BLD_LL_DB_YDIST_L_U          = rParam.rLowerIso_UpperZoom.ll_con4.bits.BLD_LL_DB_YDIST;
	MINT32 BLD_LL_GRAD_R1_L_U          = rParam.rLowerIso_UpperZoom.ll_con5.bits.BLD_LL_GRAD_R1;
	MINT32 BLD_LL_GRAD_R2_L_U          = rParam.rLowerIso_UpperZoom.ll_con5.bits.BLD_LL_GRAD_R2;
	MINT32 BLD_LL_TH3_L_U          = rParam.rLowerIso_UpperZoom.ll_con5.bits.BLD_LL_TH3;
	MINT32 BLD_LL_TH4_L_U          = rParam.rLowerIso_UpperZoom.ll_con5.bits.BLD_LL_TH4;
	MINT32 BLD_LL_TH5_L_U          = rParam.rLowerIso_UpperZoom.ll_con6.bits.BLD_LL_TH5;
	MINT32 BLD_LL_TH6_L_U          = rParam.rLowerIso_UpperZoom.ll_con6.bits.BLD_LL_TH6;
	MINT32 BLD_LL_TH7_L_U          = rParam.rLowerIso_UpperZoom.ll_con6.bits.BLD_LL_TH7;
	MINT32 BLD_LL_TH8_L_U          = rParam.rLowerIso_UpperZoom.ll_con6.bits.BLD_LL_TH8;
	MINT32 BLD_LL_C1_L_U          = rParam.rLowerIso_UpperZoom.ll_con7.bits.BLD_LL_C1;
	MINT32 BLD_LL_C2_L_U          = rParam.rLowerIso_UpperZoom.ll_con7.bits.BLD_LL_C2;
	MINT32 BLD_LL_C3_L_U          = rParam.rLowerIso_UpperZoom.ll_con7.bits.BLD_LL_C3;
	MINT32 BLD_LL_SU1_L_U          = rParam.rLowerIso_UpperZoom.ll_con8.bits.BLD_LL_SU1;
	MINT32 BLD_LL_SU2_L_U          = rParam.rLowerIso_UpperZoom.ll_con8.bits.BLD_LL_SU2;
	MINT32 BLD_LL_SU3_L_U          = rParam.rLowerIso_UpperZoom.ll_con8.bits.BLD_LL_SU3;
	MINT32 BLD_LL_SL1_L_U          = rParam.rLowerIso_UpperZoom.ll_con9.bits.BLD_LL_SL1;
	MINT32 BLD_LL_SL2_L_U          = rParam.rLowerIso_UpperZoom.ll_con9.bits.BLD_LL_SL2;
	MINT32 BLD_LL_SL3_L_U          = rParam.rLowerIso_UpperZoom.ll_con9.bits.BLD_LL_SL3;
	MINT32 BLD_CONF_MAP_EN_L_U          = rParam.rLowerIso_UpperZoom.ll_con10.bits.BLD_CONF_MAP_EN;
	MINT32 BLD_LL_GRAD_EN_L_U          = rParam.rLowerIso_UpperZoom.ll_con10.bits.BLD_LL_GRAD_EN;
	MINT32 BLD_LL_GRAD_ENTH_L_U          = rParam.rLowerIso_UpperZoom.ll_con10.bits.BLD_LL_GRAD_ENTH;

	//  ====================================================================================================================================
    //  Get Up ISO Low Zoom
    //  ====================================================================================================================================
    MINT32 BLD_LL_CLIP_TH2_U_L          = rParam.rUpperIso_LowerZoom.ll_con1.bits.BLD_LL_CLIP_TH2;
	MINT32 BLD_LL_CLIP_TH1_U_L          = rParam.rUpperIso_LowerZoom.ll_con1.bits.BLD_LL_CLIP_TH1;
	MINT32 BLD_LL_FLT_WT_MODE2_U_L          = rParam.rUpperIso_LowerZoom.ll_con1.bits.BLD_LL_FLT_WT_MODE2;
	MINT32 BLD_LL_FLT_WT_MODE1_U_L          = rParam.rUpperIso_LowerZoom.ll_con1.bits.BLD_LL_FLT_WT_MODE1;
	MINT32 BLD_LL_FLT_MODE_U_L          = rParam.rUpperIso_LowerZoom.ll_con1.bits.BLD_LL_FLT_MODE;
	MINT32 BLD_LL_TH2_U_L          = rParam.rUpperIso_LowerZoom.ll_con2.bits.BLD_LL_TH2;
	MINT32 BLD_LL_TH1_U_L          = rParam.rUpperIso_LowerZoom.ll_con2.bits.BLD_LL_TH1;
	MINT32 BLD_LL_DT1_U_L          = rParam.rUpperIso_LowerZoom.ll_con2.bits.BLD_LL_DT1;
	MINT32 BLD_LL_MAX_WT_U_L          = rParam.rUpperIso_LowerZoom.ll_con2.bits.BLD_LL_MAX_WT;
	MINT32 BLD_MBD_MAX_WT_U_L          = rParam.rUpperIso_LowerZoom.mbd_con0.bits.BLD_MBD_MAX_WT;
	MINT32 BLD_MBD_YL1_THL_0_U_L          = rParam.rUpperIso_LowerZoom.mbd_con1.bits.BLD_MBD_YL1_THL_0;
	MINT32 BLD_MBD_YL1_THL_1_U_L          = rParam.rUpperIso_LowerZoom.mbd_con1.bits.BLD_MBD_YL1_THL_1;
	MINT32 BLD_MBD_YL1_THL_2_U_L          = rParam.rUpperIso_LowerZoom.mbd_con1.bits.BLD_MBD_YL1_THL_2;
	MINT32 BLD_MBD_YL1_THL_3_U_L          = rParam.rUpperIso_LowerZoom.mbd_con1.bits.BLD_MBD_YL1_THL_3;
	MINT32 BLD_MBD_YL1_THL_4_U_L          = rParam.rUpperIso_LowerZoom.mbd_con2.bits.BLD_MBD_YL1_THL_4;
	MINT32 BLD_MBD_YL1_THH_0_U_L          = rParam.rUpperIso_LowerZoom.mbd_con2.bits.BLD_MBD_YL1_THH_0;
	MINT32 BLD_MBD_YL1_THH_1_U_L          = rParam.rUpperIso_LowerZoom.mbd_con2.bits.BLD_MBD_YL1_THH_1;
	MINT32 BLD_MBD_YL1_THH_2_U_L          = rParam.rUpperIso_LowerZoom.mbd_con2.bits.BLD_MBD_YL1_THH_2;
	MINT32 BLD_MBD_YL1_THH_3_U_L          = rParam.rUpperIso_LowerZoom.mbd_con3.bits.BLD_MBD_YL1_THH_3;
	MINT32 BLD_MBD_YL1_THH_4_U_L          = rParam.rUpperIso_LowerZoom.mbd_con3.bits.BLD_MBD_YL1_THH_4;
	MINT32 BLD_MBD_YL2_THL_0_U_L          = rParam.rUpperIso_LowerZoom.mbd_con3.bits.BLD_MBD_YL2_THL_0;
	MINT32 BLD_MBD_YL2_THL_1_U_L          = rParam.rUpperIso_LowerZoom.mbd_con3.bits.BLD_MBD_YL2_THL_1;
	MINT32 BLD_MBD_YL2_THL_2_U_L          = rParam.rUpperIso_LowerZoom.mbd_con4.bits.BLD_MBD_YL2_THL_2;
	MINT32 BLD_MBD_YL2_THL_3_U_L          = rParam.rUpperIso_LowerZoom.mbd_con4.bits.BLD_MBD_YL2_THL_3;
	MINT32 BLD_MBD_YL2_THL_4_U_L          = rParam.rUpperIso_LowerZoom.mbd_con4.bits.BLD_MBD_YL2_THL_4;
	MINT32 BLD_MBD_YL2_THH_0_U_L          = rParam.rUpperIso_LowerZoom.mbd_con4.bits.BLD_MBD_YL2_THH_0;
	MINT32 BLD_MBD_YL2_THH_1_U_L          = rParam.rUpperIso_LowerZoom.mbd_con5.bits.BLD_MBD_YL2_THH_1;
	MINT32 BLD_MBD_YL2_THH_2_U_L          = rParam.rUpperIso_LowerZoom.mbd_con5.bits.BLD_MBD_YL2_THH_2;
	MINT32 BLD_MBD_YL2_THH_3_U_L          = rParam.rUpperIso_LowerZoom.mbd_con5.bits.BLD_MBD_YL2_THH_3;
	MINT32 BLD_MBD_YL2_THH_4_U_L          = rParam.rUpperIso_LowerZoom.mbd_con5.bits.BLD_MBD_YL2_THH_4;
	MINT32 BLD_MBD_YL3_THL_0_U_L          = rParam.rUpperIso_LowerZoom.mbd_con6.bits.BLD_MBD_YL3_THL_0;
	MINT32 BLD_MBD_YL3_THL_1_U_L          = rParam.rUpperIso_LowerZoom.mbd_con6.bits.BLD_MBD_YL3_THL_1;
	MINT32 BLD_MBD_YL3_THL_2_U_L          = rParam.rUpperIso_LowerZoom.mbd_con6.bits.BLD_MBD_YL3_THL_2;
	MINT32 BLD_MBD_YL3_THL_3_U_L          = rParam.rUpperIso_LowerZoom.mbd_con6.bits.BLD_MBD_YL3_THL_3;
	MINT32 BLD_MBD_YL3_THL_4_U_L          = rParam.rUpperIso_LowerZoom.mbd_con7.bits.BLD_MBD_YL3_THL_4;
	MINT32 BLD_MBD_YL3_THH_0_U_L          = rParam.rUpperIso_LowerZoom.mbd_con7.bits.BLD_MBD_YL3_THH_0;
	MINT32 BLD_MBD_YL3_THH_1_U_L          = rParam.rUpperIso_LowerZoom.mbd_con7.bits.BLD_MBD_YL3_THH_1;
	MINT32 BLD_MBD_YL3_THH_2_U_L          = rParam.rUpperIso_LowerZoom.mbd_con7.bits.BLD_MBD_YL3_THH_2;
	MINT32 BLD_MBD_YL3_THH_3_U_L          = rParam.rUpperIso_LowerZoom.mbd_con8.bits.BLD_MBD_YL3_THH_3;
	MINT32 BLD_MBD_YL3_THH_4_U_L          = rParam.rUpperIso_LowerZoom.mbd_con8.bits.BLD_MBD_YL3_THH_4;
	MINT32 BLD_MBD_CL1_THL_0_U_L          = rParam.rUpperIso_LowerZoom.mbd_con8.bits.BLD_MBD_CL1_THL_0;
	MINT32 BLD_MBD_CL1_THL_1_U_L          = rParam.rUpperIso_LowerZoom.mbd_con8.bits.BLD_MBD_CL1_THL_1;
	MINT32 BLD_MBD_CL1_THL_2_U_L          = rParam.rUpperIso_LowerZoom.mbd_con9.bits.BLD_MBD_CL1_THL_2;
	MINT32 BLD_MBD_CL1_THL_3_U_L          = rParam.rUpperIso_LowerZoom.mbd_con9.bits.BLD_MBD_CL1_THL_3;
	MINT32 BLD_MBD_CL1_THL_4_U_L          = rParam.rUpperIso_LowerZoom.mbd_con9.bits.BLD_MBD_CL1_THL_4;
	MINT32 BLD_MBD_CL1_THH_0_U_L          = rParam.rUpperIso_LowerZoom.mbd_con9.bits.BLD_MBD_CL1_THH_0;
	MINT32 BLD_MBD_CL1_THH_1_U_L          = rParam.rUpperIso_LowerZoom.mbd_con10.bits.BLD_MBD_CL1_THH_1;
	MINT32 BLD_MBD_CL1_THH_2_U_L          = rParam.rUpperIso_LowerZoom.mbd_con10.bits.BLD_MBD_CL1_THH_2;
	MINT32 BLD_MBD_CL1_THH_3_U_L          = rParam.rUpperIso_LowerZoom.mbd_con10.bits.BLD_MBD_CL1_THH_3;
	MINT32 BLD_MBD_CL1_THH_4_U_L          = rParam.rUpperIso_LowerZoom.mbd_con10.bits.BLD_MBD_CL1_THH_4;

	// use dynamic
	MINT32 BLD_MODE_U_L                   = rParam.rUpperIso_LowerZoom.con.bits.BLD_MODE;
	MINT32 BLD_LL_BRZ_EN_U_L                   = rParam.rUpperIso_LowerZoom.con.bits.BLD_LL_BRZ_EN;
	MINT32 BLD_LL_DB_EN_U_L                   = rParam.rUpperIso_LowerZoom.con.bits.BLD_LL_DB_EN;
	MINT32 BLD_MBD_WT_EN_U_L                   = rParam.rUpperIso_LowerZoom.con.bits.BLD_MBD_WT_EN;
	MINT32 BLD_SR_WT_EN_U_L                   = rParam.rUpperIso_LowerZoom.con.bits.BLD_SR_WT_EN;
	MINT32 BLD_LL_TH_E_U_L                   = rParam.rUpperIso_LowerZoom.con.bits.BLD_LL_TH_E;
	MINT32 BLD_LL_DB_XDIST_U_L          = rParam.rUpperIso_LowerZoom.ll_con4.bits.BLD_LL_DB_XDIST;
	MINT32 BLD_LL_DB_YDIST_U_L          = rParam.rUpperIso_LowerZoom.ll_con4.bits.BLD_LL_DB_YDIST;
	MINT32 BLD_LL_GRAD_R1_U_L          = rParam.rUpperIso_LowerZoom.ll_con5.bits.BLD_LL_GRAD_R1;
	MINT32 BLD_LL_GRAD_R2_U_L          = rParam.rUpperIso_LowerZoom.ll_con5.bits.BLD_LL_GRAD_R2;
	MINT32 BLD_LL_TH3_U_L          = rParam.rUpperIso_LowerZoom.ll_con5.bits.BLD_LL_TH3;
	MINT32 BLD_LL_TH4_U_L          = rParam.rUpperIso_LowerZoom.ll_con5.bits.BLD_LL_TH4;
	MINT32 BLD_LL_TH5_U_L          = rParam.rUpperIso_LowerZoom.ll_con6.bits.BLD_LL_TH5;
	MINT32 BLD_LL_TH6_U_L          = rParam.rUpperIso_LowerZoom.ll_con6.bits.BLD_LL_TH6;
	MINT32 BLD_LL_TH7_U_L          = rParam.rUpperIso_LowerZoom.ll_con6.bits.BLD_LL_TH7;
	MINT32 BLD_LL_TH8_U_L          = rParam.rUpperIso_LowerZoom.ll_con6.bits.BLD_LL_TH8;
	MINT32 BLD_LL_C1_U_L          = rParam.rUpperIso_LowerZoom.ll_con7.bits.BLD_LL_C1;
	MINT32 BLD_LL_C2_U_L          = rParam.rUpperIso_LowerZoom.ll_con7.bits.BLD_LL_C2;
	MINT32 BLD_LL_C3_U_L          = rParam.rUpperIso_LowerZoom.ll_con7.bits.BLD_LL_C3;
	MINT32 BLD_LL_SU1_U_L          = rParam.rUpperIso_LowerZoom.ll_con8.bits.BLD_LL_SU1;
	MINT32 BLD_LL_SU2_U_L          = rParam.rUpperIso_LowerZoom.ll_con8.bits.BLD_LL_SU2;
	MINT32 BLD_LL_SU3_U_L          = rParam.rUpperIso_LowerZoom.ll_con8.bits.BLD_LL_SU3;
	MINT32 BLD_LL_SL1_U_L          = rParam.rUpperIso_LowerZoom.ll_con9.bits.BLD_LL_SL1;
	MINT32 BLD_LL_SL2_U_L          = rParam.rUpperIso_LowerZoom.ll_con9.bits.BLD_LL_SL2;
	MINT32 BLD_LL_SL3_U_L          = rParam.rUpperIso_LowerZoom.ll_con9.bits.BLD_LL_SL3;
	MINT32 BLD_CONF_MAP_EN_U_L          = rParam.rUpperIso_LowerZoom.ll_con10.bits.BLD_CONF_MAP_EN;
	MINT32 BLD_LL_GRAD_EN_U_L          = rParam.rUpperIso_LowerZoom.ll_con10.bits.BLD_LL_GRAD_EN;
	MINT32 BLD_LL_GRAD_ENTH_U_L          = rParam.rUpperIso_LowerZoom.ll_con10.bits.BLD_LL_GRAD_ENTH;

	//  ====================================================================================================================================
    //  Get Up ISO Up Zoom
    //  ====================================================================================================================================
    MINT32 BLD_LL_CLIP_TH2_U_U          = rParam.rUpperIso_UpperZoom.ll_con1.bits.BLD_LL_CLIP_TH2;
	MINT32 BLD_LL_CLIP_TH1_U_U          = rParam.rUpperIso_UpperZoom.ll_con1.bits.BLD_LL_CLIP_TH1;
	MINT32 BLD_LL_FLT_WT_MODE2_U_U          = rParam.rUpperIso_UpperZoom.ll_con1.bits.BLD_LL_FLT_WT_MODE2;
	MINT32 BLD_LL_FLT_WT_MODE1_U_U          = rParam.rUpperIso_UpperZoom.ll_con1.bits.BLD_LL_FLT_WT_MODE1;
	MINT32 BLD_LL_FLT_MODE_U_U          = rParam.rUpperIso_UpperZoom.ll_con1.bits.BLD_LL_FLT_MODE;
	MINT32 BLD_LL_TH2_U_U          = rParam.rUpperIso_UpperZoom.ll_con2.bits.BLD_LL_TH2;
	MINT32 BLD_LL_TH1_U_U          = rParam.rUpperIso_UpperZoom.ll_con2.bits.BLD_LL_TH1;
	MINT32 BLD_LL_DT1_U_U          = rParam.rUpperIso_UpperZoom.ll_con2.bits.BLD_LL_DT1;
	MINT32 BLD_LL_MAX_WT_U_U          = rParam.rUpperIso_UpperZoom.ll_con2.bits.BLD_LL_MAX_WT;
	MINT32 BLD_MBD_MAX_WT_U_U          = rParam.rUpperIso_UpperZoom.mbd_con0.bits.BLD_MBD_MAX_WT;
	MINT32 BLD_MBD_YL1_THL_0_U_U          = rParam.rUpperIso_UpperZoom.mbd_con1.bits.BLD_MBD_YL1_THL_0;
	MINT32 BLD_MBD_YL1_THL_1_U_U          = rParam.rUpperIso_UpperZoom.mbd_con1.bits.BLD_MBD_YL1_THL_1;
	MINT32 BLD_MBD_YL1_THL_2_U_U          = rParam.rUpperIso_UpperZoom.mbd_con1.bits.BLD_MBD_YL1_THL_2;
	MINT32 BLD_MBD_YL1_THL_3_U_U          = rParam.rUpperIso_UpperZoom.mbd_con1.bits.BLD_MBD_YL1_THL_3;
	MINT32 BLD_MBD_YL1_THL_4_U_U          = rParam.rUpperIso_UpperZoom.mbd_con2.bits.BLD_MBD_YL1_THL_4;
	MINT32 BLD_MBD_YL1_THH_0_U_U          = rParam.rUpperIso_UpperZoom.mbd_con2.bits.BLD_MBD_YL1_THH_0;
	MINT32 BLD_MBD_YL1_THH_1_U_U          = rParam.rUpperIso_UpperZoom.mbd_con2.bits.BLD_MBD_YL1_THH_1;
	MINT32 BLD_MBD_YL1_THH_2_U_U          = rParam.rUpperIso_UpperZoom.mbd_con2.bits.BLD_MBD_YL1_THH_2;
	MINT32 BLD_MBD_YL1_THH_3_U_U          = rParam.rUpperIso_UpperZoom.mbd_con3.bits.BLD_MBD_YL1_THH_3;
	MINT32 BLD_MBD_YL1_THH_4_U_U          = rParam.rUpperIso_UpperZoom.mbd_con3.bits.BLD_MBD_YL1_THH_4;
	MINT32 BLD_MBD_YL2_THL_0_U_U          = rParam.rUpperIso_UpperZoom.mbd_con3.bits.BLD_MBD_YL2_THL_0;
	MINT32 BLD_MBD_YL2_THL_1_U_U          = rParam.rUpperIso_UpperZoom.mbd_con3.bits.BLD_MBD_YL2_THL_1;
	MINT32 BLD_MBD_YL2_THL_2_U_U          = rParam.rUpperIso_UpperZoom.mbd_con4.bits.BLD_MBD_YL2_THL_2;
	MINT32 BLD_MBD_YL2_THL_3_U_U          = rParam.rUpperIso_UpperZoom.mbd_con4.bits.BLD_MBD_YL2_THL_3;
	MINT32 BLD_MBD_YL2_THL_4_U_U          = rParam.rUpperIso_UpperZoom.mbd_con4.bits.BLD_MBD_YL2_THL_4;
	MINT32 BLD_MBD_YL2_THH_0_U_U          = rParam.rUpperIso_UpperZoom.mbd_con4.bits.BLD_MBD_YL2_THH_0;
	MINT32 BLD_MBD_YL2_THH_1_U_U          = rParam.rUpperIso_UpperZoom.mbd_con5.bits.BLD_MBD_YL2_THH_1;
	MINT32 BLD_MBD_YL2_THH_2_U_U          = rParam.rUpperIso_UpperZoom.mbd_con5.bits.BLD_MBD_YL2_THH_2;
	MINT32 BLD_MBD_YL2_THH_3_U_U          = rParam.rUpperIso_UpperZoom.mbd_con5.bits.BLD_MBD_YL2_THH_3;
	MINT32 BLD_MBD_YL2_THH_4_U_U          = rParam.rUpperIso_UpperZoom.mbd_con5.bits.BLD_MBD_YL2_THH_4;
	MINT32 BLD_MBD_YL3_THL_0_U_U          = rParam.rUpperIso_UpperZoom.mbd_con6.bits.BLD_MBD_YL3_THL_0;
	MINT32 BLD_MBD_YL3_THL_1_U_U          = rParam.rUpperIso_UpperZoom.mbd_con6.bits.BLD_MBD_YL3_THL_1;
	MINT32 BLD_MBD_YL3_THL_2_U_U          = rParam.rUpperIso_UpperZoom.mbd_con6.bits.BLD_MBD_YL3_THL_2;
	MINT32 BLD_MBD_YL3_THL_3_U_U          = rParam.rUpperIso_UpperZoom.mbd_con6.bits.BLD_MBD_YL3_THL_3;
	MINT32 BLD_MBD_YL3_THL_4_U_U          = rParam.rUpperIso_UpperZoom.mbd_con7.bits.BLD_MBD_YL3_THL_4;
	MINT32 BLD_MBD_YL3_THH_0_U_U          = rParam.rUpperIso_UpperZoom.mbd_con7.bits.BLD_MBD_YL3_THH_0;
	MINT32 BLD_MBD_YL3_THH_1_U_U          = rParam.rUpperIso_UpperZoom.mbd_con7.bits.BLD_MBD_YL3_THH_1;
	MINT32 BLD_MBD_YL3_THH_2_U_U          = rParam.rUpperIso_UpperZoom.mbd_con7.bits.BLD_MBD_YL3_THH_2;
	MINT32 BLD_MBD_YL3_THH_3_U_U          = rParam.rUpperIso_UpperZoom.mbd_con8.bits.BLD_MBD_YL3_THH_3;
	MINT32 BLD_MBD_YL3_THH_4_U_U          = rParam.rUpperIso_UpperZoom.mbd_con8.bits.BLD_MBD_YL3_THH_4;
	MINT32 BLD_MBD_CL1_THL_0_U_U          = rParam.rUpperIso_UpperZoom.mbd_con8.bits.BLD_MBD_CL1_THL_0;
	MINT32 BLD_MBD_CL1_THL_1_U_U          = rParam.rUpperIso_UpperZoom.mbd_con8.bits.BLD_MBD_CL1_THL_1;
	MINT32 BLD_MBD_CL1_THL_2_U_U          = rParam.rUpperIso_UpperZoom.mbd_con9.bits.BLD_MBD_CL1_THL_2;
	MINT32 BLD_MBD_CL1_THL_3_U_U          = rParam.rUpperIso_UpperZoom.mbd_con9.bits.BLD_MBD_CL1_THL_3;
	MINT32 BLD_MBD_CL1_THL_4_U_U          = rParam.rUpperIso_UpperZoom.mbd_con9.bits.BLD_MBD_CL1_THL_4;
	MINT32 BLD_MBD_CL1_THH_0_U_U          = rParam.rUpperIso_UpperZoom.mbd_con9.bits.BLD_MBD_CL1_THH_0;
	MINT32 BLD_MBD_CL1_THH_1_U_U          = rParam.rUpperIso_UpperZoom.mbd_con10.bits.BLD_MBD_CL1_THH_1;
	MINT32 BLD_MBD_CL1_THH_2_U_U          = rParam.rUpperIso_UpperZoom.mbd_con10.bits.BLD_MBD_CL1_THH_2;
	MINT32 BLD_MBD_CL1_THH_3_U_U          = rParam.rUpperIso_UpperZoom.mbd_con10.bits.BLD_MBD_CL1_THH_3;
	MINT32 BLD_MBD_CL1_THH_4_U_U          = rParam.rUpperIso_UpperZoom.mbd_con10.bits.BLD_MBD_CL1_THH_4;

	// use dynamic
	MINT32 BLD_MODE_U_U                   = rParam.rUpperIso_UpperZoom.con.bits.BLD_MODE;
	MINT32 BLD_LL_BRZ_EN_U_U                   = rParam.rUpperIso_UpperZoom.con.bits.BLD_LL_BRZ_EN;
	MINT32 BLD_LL_DB_EN_U_U                   = rParam.rUpperIso_UpperZoom.con.bits.BLD_LL_DB_EN;
	MINT32 BLD_MBD_WT_EN_U_U                   = rParam.rUpperIso_UpperZoom.con.bits.BLD_MBD_WT_EN;
	MINT32 BLD_SR_WT_EN_U_U                   = rParam.rUpperIso_UpperZoom.con.bits.BLD_SR_WT_EN;
	MINT32 BLD_LL_TH_E_U_U                   = rParam.rUpperIso_UpperZoom.con.bits.BLD_LL_TH_E;
	MINT32 BLD_LL_DB_XDIST_U_U          = rParam.rUpperIso_UpperZoom.ll_con4.bits.BLD_LL_DB_XDIST;
	MINT32 BLD_LL_DB_YDIST_U_U          = rParam.rUpperIso_UpperZoom.ll_con4.bits.BLD_LL_DB_YDIST;
	MINT32 BLD_LL_GRAD_R1_U_U          = rParam.rUpperIso_UpperZoom.ll_con5.bits.BLD_LL_GRAD_R1;
	MINT32 BLD_LL_GRAD_R2_U_U          = rParam.rUpperIso_UpperZoom.ll_con5.bits.BLD_LL_GRAD_R2;
	MINT32 BLD_LL_TH3_U_U          = rParam.rUpperIso_UpperZoom.ll_con5.bits.BLD_LL_TH3;
	MINT32 BLD_LL_TH4_U_U          = rParam.rUpperIso_UpperZoom.ll_con5.bits.BLD_LL_TH4;
	MINT32 BLD_LL_TH5_U_U          = rParam.rUpperIso_UpperZoom.ll_con6.bits.BLD_LL_TH5;
	MINT32 BLD_LL_TH6_U_U          = rParam.rUpperIso_UpperZoom.ll_con6.bits.BLD_LL_TH6;
	MINT32 BLD_LL_TH7_U_U          = rParam.rUpperIso_UpperZoom.ll_con6.bits.BLD_LL_TH7;
	MINT32 BLD_LL_TH8_U_U          = rParam.rUpperIso_UpperZoom.ll_con6.bits.BLD_LL_TH8;
	MINT32 BLD_LL_C1_U_U          = rParam.rUpperIso_UpperZoom.ll_con7.bits.BLD_LL_C1;
	MINT32 BLD_LL_C2_U_U          = rParam.rUpperIso_UpperZoom.ll_con7.bits.BLD_LL_C2;
	MINT32 BLD_LL_C3_U_U          = rParam.rUpperIso_UpperZoom.ll_con7.bits.BLD_LL_C3;
	MINT32 BLD_LL_SU1_U_U          = rParam.rUpperIso_UpperZoom.ll_con8.bits.BLD_LL_SU1;
	MINT32 BLD_LL_SU2_U_U          = rParam.rUpperIso_UpperZoom.ll_con8.bits.BLD_LL_SU2;
	MINT32 BLD_LL_SU3_U_U          = rParam.rUpperIso_UpperZoom.ll_con8.bits.BLD_LL_SU3;
	MINT32 BLD_LL_SL1_U_U          = rParam.rUpperIso_UpperZoom.ll_con9.bits.BLD_LL_SL1;
	MINT32 BLD_LL_SL2_U_U          = rParam.rUpperIso_UpperZoom.ll_con9.bits.BLD_LL_SL2;
	MINT32 BLD_LL_SL3_U_U          = rParam.rUpperIso_UpperZoom.ll_con9.bits.BLD_LL_SL3;
	MINT32 BLD_CONF_MAP_EN_U_U          = rParam.rUpperIso_UpperZoom.ll_con10.bits.BLD_CONF_MAP_EN;
	MINT32 BLD_LL_GRAD_EN_U_U          = rParam.rUpperIso_UpperZoom.ll_con10.bits.BLD_LL_GRAD_EN;
	MINT32 BLD_LL_GRAD_ENTH_U_U          = rParam.rUpperIso_UpperZoom.ll_con10.bits.BLD_LL_GRAD_ENTH;








    //  Registers that can be interpolated
    MINT32 BLD_LL_CLIP_TH2            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_LL_CLIP_TH2_L_L, BLD_LL_CLIP_TH2_L_U, BLD_LL_CLIP_TH2_U_L, BLD_LL_CLIP_TH2_U_U,0);
	MINT32 BLD_LL_CLIP_TH1            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_LL_CLIP_TH1_L_L, BLD_LL_CLIP_TH1_L_U, BLD_LL_CLIP_TH1_U_L, BLD_LL_CLIP_TH1_U_U,0);
	MINT32 BLD_LL_FLT_WT_MODE2            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_LL_FLT_WT_MODE2_L_L, BLD_LL_FLT_WT_MODE2_L_U, BLD_LL_FLT_WT_MODE2_U_L, BLD_LL_FLT_WT_MODE2_U_U,1);
	MINT32 BLD_LL_FLT_WT_MODE1            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_LL_FLT_WT_MODE1_L_L, BLD_LL_FLT_WT_MODE1_L_U, BLD_LL_FLT_WT_MODE1_U_L, BLD_LL_FLT_WT_MODE1_U_U,1);
	MINT32 BLD_LL_FLT_MODE            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_LL_FLT_MODE_L_L, BLD_LL_FLT_MODE_L_U, BLD_LL_FLT_MODE_U_L, BLD_LL_FLT_MODE_U_U,1);
	MINT32 BLD_LL_TH2            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_LL_TH2_L_L, BLD_LL_TH2_L_U, BLD_LL_TH2_U_L, BLD_LL_TH2_U_U,0);
	MINT32 BLD_LL_TH1            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_LL_TH1_L_L, BLD_LL_TH1_L_U, BLD_LL_TH1_U_L, BLD_LL_TH1_U_U,0);
	MINT32 BLD_LL_DT1            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_LL_DT1_L_L, BLD_LL_DT1_L_U, BLD_LL_DT1_U_L, BLD_LL_DT1_U_U,0);
	MINT32 BLD_LL_MAX_WT            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_LL_MAX_WT_L_L, BLD_LL_MAX_WT_L_U, BLD_LL_MAX_WT_U_L, BLD_LL_MAX_WT_U_U,0);
	MINT32 BLD_MBD_MAX_WT            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_MBD_MAX_WT_L_L, BLD_MBD_MAX_WT_L_U, BLD_MBD_MAX_WT_U_L, BLD_MBD_MAX_WT_U_U,0);
	MINT32 BLD_MBD_YL1_THL_0            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_MBD_YL1_THL_0_L_L, BLD_MBD_YL1_THL_0_L_U, BLD_MBD_YL1_THL_0_U_L, BLD_MBD_YL1_THL_0_U_U,0);
	MINT32 BLD_MBD_YL1_THL_1            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_MBD_YL1_THL_1_L_L, BLD_MBD_YL1_THL_1_L_U, BLD_MBD_YL1_THL_1_U_L, BLD_MBD_YL1_THL_1_U_U,0);
	MINT32 BLD_MBD_YL1_THL_2            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_MBD_YL1_THL_2_L_L, BLD_MBD_YL1_THL_2_L_U, BLD_MBD_YL1_THL_2_U_L, BLD_MBD_YL1_THL_2_U_U,0);
	MINT32 BLD_MBD_YL1_THL_3            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_MBD_YL1_THL_3_L_L, BLD_MBD_YL1_THL_3_L_U, BLD_MBD_YL1_THL_3_U_L, BLD_MBD_YL1_THL_3_U_U,0);
	MINT32 BLD_MBD_YL1_THL_4            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_MBD_YL1_THL_4_L_L, BLD_MBD_YL1_THL_4_L_U, BLD_MBD_YL1_THL_4_U_L, BLD_MBD_YL1_THL_4_U_U,0);
	MINT32 BLD_MBD_YL1_THH_0            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_MBD_YL1_THH_0_L_L, BLD_MBD_YL1_THH_0_L_U, BLD_MBD_YL1_THH_0_U_L, BLD_MBD_YL1_THH_0_U_U,0);
	MINT32 BLD_MBD_YL1_THH_1            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_MBD_YL1_THH_1_L_L, BLD_MBD_YL1_THH_1_L_U, BLD_MBD_YL1_THH_1_U_L, BLD_MBD_YL1_THH_1_U_U,0);
	MINT32 BLD_MBD_YL1_THH_2            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_MBD_YL1_THH_2_L_L, BLD_MBD_YL1_THH_2_L_U, BLD_MBD_YL1_THH_2_U_L, BLD_MBD_YL1_THH_2_U_U,0);
	MINT32 BLD_MBD_YL1_THH_3            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_MBD_YL1_THH_3_L_L, BLD_MBD_YL1_THH_3_L_U, BLD_MBD_YL1_THH_3_U_L, BLD_MBD_YL1_THH_3_U_U,0);
	MINT32 BLD_MBD_YL1_THH_4            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_MBD_YL1_THH_4_L_L, BLD_MBD_YL1_THH_4_L_U, BLD_MBD_YL1_THH_4_U_L, BLD_MBD_YL1_THH_4_U_U,0);
	MINT32 BLD_MBD_YL2_THL_0            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_MBD_YL2_THL_0_L_L, BLD_MBD_YL2_THL_0_L_U, BLD_MBD_YL2_THL_0_U_L, BLD_MBD_YL2_THL_0_U_U,0);
	MINT32 BLD_MBD_YL2_THL_1            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_MBD_YL2_THL_1_L_L, BLD_MBD_YL2_THL_1_L_U, BLD_MBD_YL2_THL_1_U_L, BLD_MBD_YL2_THL_1_U_U,0);
	MINT32 BLD_MBD_YL2_THL_2            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_MBD_YL2_THL_2_L_L, BLD_MBD_YL2_THL_2_L_U, BLD_MBD_YL2_THL_2_U_L, BLD_MBD_YL2_THL_2_U_U,0);
	MINT32 BLD_MBD_YL2_THL_3            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_MBD_YL2_THL_3_L_L, BLD_MBD_YL2_THL_3_L_U, BLD_MBD_YL2_THL_3_U_L, BLD_MBD_YL2_THL_3_U_U,0);
	MINT32 BLD_MBD_YL2_THL_4            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_MBD_YL2_THL_4_L_L, BLD_MBD_YL2_THL_4_L_U, BLD_MBD_YL2_THL_4_U_L, BLD_MBD_YL2_THL_4_U_U,0);
	MINT32 BLD_MBD_YL2_THH_0            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_MBD_YL2_THH_0_L_L, BLD_MBD_YL2_THH_0_L_U, BLD_MBD_YL2_THH_0_U_L, BLD_MBD_YL2_THH_0_U_U,0);
	MINT32 BLD_MBD_YL2_THH_1            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_MBD_YL2_THH_1_L_L, BLD_MBD_YL2_THH_1_L_U, BLD_MBD_YL2_THH_1_U_L, BLD_MBD_YL2_THH_1_U_U,0);
	MINT32 BLD_MBD_YL2_THH_2            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_MBD_YL2_THH_2_L_L, BLD_MBD_YL2_THH_2_L_U, BLD_MBD_YL2_THH_2_U_L, BLD_MBD_YL2_THH_2_U_U,0);
	MINT32 BLD_MBD_YL2_THH_3            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_MBD_YL2_THH_3_L_L, BLD_MBD_YL2_THH_3_L_U, BLD_MBD_YL2_THH_3_U_L, BLD_MBD_YL2_THH_3_U_U,0);
	MINT32 BLD_MBD_YL2_THH_4            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_MBD_YL2_THH_4_L_L, BLD_MBD_YL2_THH_4_L_U, BLD_MBD_YL2_THH_4_U_L, BLD_MBD_YL2_THH_4_U_U,0);
	MINT32 BLD_MBD_YL3_THL_0            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_MBD_YL3_THL_0_L_L, BLD_MBD_YL3_THL_0_L_U, BLD_MBD_YL3_THL_0_U_L, BLD_MBD_YL3_THL_0_U_U,0);
	MINT32 BLD_MBD_YL3_THL_1            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_MBD_YL3_THL_1_L_L, BLD_MBD_YL3_THL_1_L_U, BLD_MBD_YL3_THL_1_U_L, BLD_MBD_YL3_THL_1_U_U,0);
	MINT32 BLD_MBD_YL3_THL_2            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_MBD_YL3_THL_2_L_L, BLD_MBD_YL3_THL_2_L_U, BLD_MBD_YL3_THL_2_U_L, BLD_MBD_YL3_THL_2_U_U,0);
	MINT32 BLD_MBD_YL3_THL_3            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_MBD_YL3_THL_3_L_L, BLD_MBD_YL3_THL_3_L_U, BLD_MBD_YL3_THL_3_U_L, BLD_MBD_YL3_THL_3_U_U,0);
	MINT32 BLD_MBD_YL3_THL_4            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_MBD_YL3_THL_4_L_L, BLD_MBD_YL3_THL_4_L_U, BLD_MBD_YL3_THL_4_U_L, BLD_MBD_YL3_THL_4_U_U,0);
	MINT32 BLD_MBD_YL3_THH_0            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_MBD_YL3_THH_0_L_L, BLD_MBD_YL3_THH_0_L_U, BLD_MBD_YL3_THH_0_U_L, BLD_MBD_YL3_THH_0_U_U,0);
	MINT32 BLD_MBD_YL3_THH_1            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_MBD_YL3_THH_1_L_L, BLD_MBD_YL3_THH_1_L_U, BLD_MBD_YL3_THH_1_U_L, BLD_MBD_YL3_THH_1_U_U,0);
	MINT32 BLD_MBD_YL3_THH_2            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_MBD_YL3_THH_2_L_L, BLD_MBD_YL3_THH_2_L_U, BLD_MBD_YL3_THH_2_U_L, BLD_MBD_YL3_THH_2_U_U,0);
	MINT32 BLD_MBD_YL3_THH_3            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_MBD_YL3_THH_3_L_L, BLD_MBD_YL3_THH_3_L_U, BLD_MBD_YL3_THH_3_U_L, BLD_MBD_YL3_THH_3_U_U,0);
	MINT32 BLD_MBD_YL3_THH_4            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_MBD_YL3_THH_4_L_L, BLD_MBD_YL3_THH_4_L_U, BLD_MBD_YL3_THH_4_U_L, BLD_MBD_YL3_THH_4_U_U,0);
	MINT32 BLD_MBD_CL1_THL_0            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_MBD_CL1_THL_0_L_L, BLD_MBD_CL1_THL_0_L_U, BLD_MBD_CL1_THL_0_U_L, BLD_MBD_CL1_THL_0_U_U,0);
	MINT32 BLD_MBD_CL1_THL_1            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_MBD_CL1_THL_1_L_L, BLD_MBD_CL1_THL_1_L_U, BLD_MBD_CL1_THL_1_U_L, BLD_MBD_CL1_THL_1_U_U,0);
	MINT32 BLD_MBD_CL1_THL_2            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_MBD_CL1_THL_2_L_L, BLD_MBD_CL1_THL_2_L_U, BLD_MBD_CL1_THL_2_U_L, BLD_MBD_CL1_THL_2_U_U,0);
	MINT32 BLD_MBD_CL1_THL_3            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_MBD_CL1_THL_3_L_L, BLD_MBD_CL1_THL_3_L_U, BLD_MBD_CL1_THL_3_U_L, BLD_MBD_CL1_THL_3_U_U,0);
	MINT32 BLD_MBD_CL1_THL_4            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_MBD_CL1_THL_4_L_L, BLD_MBD_CL1_THL_4_L_U, BLD_MBD_CL1_THL_4_U_L, BLD_MBD_CL1_THL_4_U_U,0);
	MINT32 BLD_MBD_CL1_THH_0            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_MBD_CL1_THH_0_L_L, BLD_MBD_CL1_THH_0_L_U, BLD_MBD_CL1_THH_0_U_L, BLD_MBD_CL1_THH_0_U_U,0);
	MINT32 BLD_MBD_CL1_THH_1            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_MBD_CL1_THH_1_L_L, BLD_MBD_CL1_THH_1_L_U, BLD_MBD_CL1_THH_1_U_L, BLD_MBD_CL1_THH_1_U_U,0);
	MINT32 BLD_MBD_CL1_THH_2            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_MBD_CL1_THH_2_L_L, BLD_MBD_CL1_THH_2_L_U, BLD_MBD_CL1_THH_2_U_L, BLD_MBD_CL1_THH_2_U_U,0);
	MINT32 BLD_MBD_CL1_THH_3            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_MBD_CL1_THH_3_L_L, BLD_MBD_CL1_THH_3_L_U, BLD_MBD_CL1_THH_3_U_L, BLD_MBD_CL1_THH_3_U_U,0);
	MINT32 BLD_MBD_CL1_THH_4            = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_MBD_CL1_THH_4_L_L, BLD_MBD_CL1_THH_4_L_U, BLD_MBD_CL1_THH_4_U_L, BLD_MBD_CL1_THH_4_U_U,0);

	// use dynamic
	MINT32 BLD_MODE                   = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_MODE_L_L, BLD_MODE_L_U, BLD_MODE_U_L, BLD_MODE_U_U,1);
	MINT32 BLD_LL_BRZ_EN                   = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_LL_BRZ_EN_L_L, BLD_LL_BRZ_EN_L_U, BLD_LL_BRZ_EN_U_L, BLD_LL_BRZ_EN_U_U,1);
	MINT32 BLD_LL_DB_EN                   = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_LL_DB_EN_L_L, BLD_LL_DB_EN_L_U, BLD_LL_DB_EN_U_L, BLD_LL_DB_EN_U_U,1);
	MINT32 BLD_MBD_WT_EN                   = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_MBD_WT_EN_L_L, BLD_MBD_WT_EN_L_U, BLD_MBD_WT_EN_U_L, BLD_MBD_WT_EN_U_U,1);
	MINT32 BLD_SR_WT_EN                   = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_SR_WT_EN_L_L, BLD_SR_WT_EN_L_U, BLD_SR_WT_EN_U_L, BLD_SR_WT_EN_U_U,1);
	MINT32 BLD_LL_TH_E                   = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_LL_TH_E_L_L, BLD_LL_TH_E_L_U, BLD_LL_TH_E_U_L, BLD_LL_TH_E_U_U,1);
	MINT32 BLD_LL_DB_XDIST          = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_LL_DB_XDIST_L_L, BLD_LL_DB_XDIST_L_U, BLD_LL_DB_XDIST_U_L, BLD_LL_DB_XDIST_U_U,1);
	MINT32 BLD_LL_DB_YDIST          = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_LL_DB_YDIST_L_L, BLD_LL_DB_YDIST_L_U, BLD_LL_DB_YDIST_U_L, BLD_LL_DB_YDIST_U_U,1);
	MINT32 BLD_LL_GRAD_R1          = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_LL_GRAD_R1_L_L, BLD_LL_GRAD_R1_L_U, BLD_LL_GRAD_R1_U_L, BLD_LL_GRAD_R1_U_U,1);
	MINT32 BLD_LL_GRAD_R2          = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_LL_GRAD_R2_L_L, BLD_LL_GRAD_R2_L_U, BLD_LL_GRAD_R2_U_L, BLD_LL_GRAD_R2_U_U,1);
	MINT32 BLD_LL_TH3          = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_LL_TH3_L_L, BLD_LL_TH3_L_U, BLD_LL_TH3_U_L, BLD_LL_TH3_U_U,1);
	MINT32 BLD_LL_TH4          = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_LL_TH4_L_L, BLD_LL_TH4_L_U, BLD_LL_TH4_U_L, BLD_LL_TH4_U_U,1);
	MINT32 BLD_LL_TH5          = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_LL_TH5_L_L, BLD_LL_TH5_L_U, BLD_LL_TH5_U_L, BLD_LL_TH5_U_U,1);
	MINT32 BLD_LL_TH6          = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_LL_TH6_L_L, BLD_LL_TH6_L_U, BLD_LL_TH6_U_L, BLD_LL_TH6_U_U,1);
	MINT32 BLD_LL_TH7          = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_LL_TH7_L_L, BLD_LL_TH7_L_U, BLD_LL_TH7_U_L, BLD_LL_TH7_U_U,1);
	MINT32 BLD_LL_TH8          = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_LL_TH8_L_L, BLD_LL_TH8_L_U, BLD_LL_TH8_U_L, BLD_LL_TH8_U_U,1);
	MINT32 BLD_LL_C1          = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_LL_C1_L_L, BLD_LL_C1_L_U, BLD_LL_C1_U_L, BLD_LL_C1_U_U,1);
	MINT32 BLD_LL_C2          = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_LL_C2_L_L, BLD_LL_C2_L_U, BLD_LL_C2_U_L, BLD_LL_C2_U_U,1);
	MINT32 BLD_LL_C3          = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_LL_C3_L_L, BLD_LL_C3_L_U, BLD_LL_C3_U_L, BLD_LL_C3_U_U,1);
	MINT32 BLD_LL_SU1          = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_LL_SU1_L_L, BLD_LL_SU1_L_U, BLD_LL_SU1_U_L, BLD_LL_SU1_U_U,1);
	MINT32 BLD_LL_SU2          = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_LL_SU2_L_L, BLD_LL_SU2_L_U, BLD_LL_SU2_U_L, BLD_LL_SU2_U_U,1);
	MINT32 BLD_LL_SU3          = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_LL_SU3_L_L, BLD_LL_SU3_L_U, BLD_LL_SU3_U_L, BLD_LL_SU3_U_U,1);
	MINT32 BLD_LL_SL1          = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_LL_SL1_L_L, BLD_LL_SL1_L_U, BLD_LL_SL1_U_L, BLD_LL_SL1_U_U,1);
	MINT32 BLD_LL_SL2          = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_LL_SL2_L_L, BLD_LL_SL2_L_U, BLD_LL_SL2_U_L, BLD_LL_SL2_U_U,1);
	MINT32 BLD_LL_SL3          = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_LL_SL3_L_L, BLD_LL_SL3_L_U, BLD_LL_SL3_U_L, BLD_LL_SL3_U_U,1);
	MINT32 BLD_CONF_MAP_EN          = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_CONF_MAP_EN_L_L, BLD_CONF_MAP_EN_L_U, BLD_CONF_MAP_EN_U_L, BLD_CONF_MAP_EN_U_U,1);
	MINT32 BLD_LL_GRAD_EN          = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_LL_GRAD_EN_L_L, BLD_LL_GRAD_EN_L_U, BLD_LL_GRAD_EN_U_L, BLD_LL_GRAD_EN_U_U,1);
	MINT32 BLD_LL_GRAD_ENTH          = InterParam_MFB(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, BLD_LL_GRAD_ENTH_L_L, BLD_LL_GRAD_ENTH_L_U, BLD_LL_GRAD_ENTH_U_L, BLD_LL_GRAD_ENTH_U_U,1);




	//  ====================================================================================================================================
    //  Set Smooth ISO setting
    //  ====================================================================================================================================
    rSmoothMFB.ll_con1.bits.BLD_LL_CLIP_TH2       =          BLD_LL_CLIP_TH2;
	rSmoothMFB.ll_con1.bits.BLD_LL_CLIP_TH1       =          BLD_LL_CLIP_TH1;
	rSmoothMFB.ll_con1.bits.BLD_LL_FLT_WT_MODE2       =      BLD_LL_FLT_WT_MODE2;
	rSmoothMFB.ll_con1.bits.BLD_LL_FLT_WT_MODE1       =      BLD_LL_FLT_WT_MODE1;
	rSmoothMFB.ll_con1.bits.BLD_LL_FLT_MODE       =          BLD_LL_FLT_MODE;
	rSmoothMFB.ll_con2.bits.BLD_LL_TH2       =               BLD_LL_TH2;
	rSmoothMFB.ll_con2.bits.BLD_LL_TH1       =               BLD_LL_TH1;
	rSmoothMFB.ll_con2.bits.BLD_LL_DT1       =               BLD_LL_DT1;
	rSmoothMFB.ll_con2.bits.BLD_LL_MAX_WT       =            BLD_LL_MAX_WT;
	rSmoothMFB.mbd_con0.bits.BLD_MBD_MAX_WT       =          BLD_MBD_MAX_WT;
	rSmoothMFB.mbd_con1.bits.BLD_MBD_YL1_THL_0       =       BLD_MBD_YL1_THL_0;
	rSmoothMFB.mbd_con1.bits.BLD_MBD_YL1_THL_1       =       BLD_MBD_YL1_THL_1;
	rSmoothMFB.mbd_con1.bits.BLD_MBD_YL1_THL_2       =       BLD_MBD_YL1_THL_2;
	rSmoothMFB.mbd_con1.bits.BLD_MBD_YL1_THL_3       =       BLD_MBD_YL1_THL_3;
	rSmoothMFB.mbd_con2.bits.BLD_MBD_YL1_THL_4       =       BLD_MBD_YL1_THL_4;
	rSmoothMFB.mbd_con2.bits.BLD_MBD_YL1_THH_0       =       BLD_MBD_YL1_THH_0;
	rSmoothMFB.mbd_con2.bits.BLD_MBD_YL1_THH_1       =       BLD_MBD_YL1_THH_1;
	rSmoothMFB.mbd_con2.bits.BLD_MBD_YL1_THH_2       =       BLD_MBD_YL1_THH_2;
	rSmoothMFB.mbd_con3.bits.BLD_MBD_YL1_THH_3       =       BLD_MBD_YL1_THH_3;
	rSmoothMFB.mbd_con3.bits.BLD_MBD_YL1_THH_4       =       BLD_MBD_YL1_THH_4;
	rSmoothMFB.mbd_con3.bits.BLD_MBD_YL2_THL_0       =       BLD_MBD_YL2_THL_0;
	rSmoothMFB.mbd_con3.bits.BLD_MBD_YL2_THL_1       =       BLD_MBD_YL2_THL_1;
	rSmoothMFB.mbd_con4.bits.BLD_MBD_YL2_THL_2       =       BLD_MBD_YL2_THL_2;
	rSmoothMFB.mbd_con4.bits.BLD_MBD_YL2_THL_3       =       BLD_MBD_YL2_THL_3;
	rSmoothMFB.mbd_con4.bits.BLD_MBD_YL2_THL_4       =       BLD_MBD_YL2_THL_4;
	rSmoothMFB.mbd_con4.bits.BLD_MBD_YL2_THH_0       =       BLD_MBD_YL2_THH_0;
	rSmoothMFB.mbd_con5.bits.BLD_MBD_YL2_THH_1       =       BLD_MBD_YL2_THH_1;
	rSmoothMFB.mbd_con5.bits.BLD_MBD_YL2_THH_2       =       BLD_MBD_YL2_THH_2;
	rSmoothMFB.mbd_con5.bits.BLD_MBD_YL2_THH_3       =       BLD_MBD_YL2_THH_3;
	rSmoothMFB.mbd_con5.bits.BLD_MBD_YL2_THH_4       =       BLD_MBD_YL2_THH_4;
	rSmoothMFB.mbd_con6.bits.BLD_MBD_YL3_THL_0       =       BLD_MBD_YL3_THL_0;
	rSmoothMFB.mbd_con6.bits.BLD_MBD_YL3_THL_1       =       BLD_MBD_YL3_THL_1;
	rSmoothMFB.mbd_con6.bits.BLD_MBD_YL3_THL_2       =       BLD_MBD_YL3_THL_2;
	rSmoothMFB.mbd_con6.bits.BLD_MBD_YL3_THL_3       =       BLD_MBD_YL3_THL_3;
	rSmoothMFB.mbd_con7.bits.BLD_MBD_YL3_THL_4       =       BLD_MBD_YL3_THL_4;
	rSmoothMFB.mbd_con7.bits.BLD_MBD_YL3_THH_0       =       BLD_MBD_YL3_THH_0;
	rSmoothMFB.mbd_con7.bits.BLD_MBD_YL3_THH_1       =       BLD_MBD_YL3_THH_1;
	rSmoothMFB.mbd_con7.bits.BLD_MBD_YL3_THH_2       =       BLD_MBD_YL3_THH_2;
	rSmoothMFB.mbd_con8.bits.BLD_MBD_YL3_THH_3       =       BLD_MBD_YL3_THH_3;
	rSmoothMFB.mbd_con8.bits.BLD_MBD_YL3_THH_4       =       BLD_MBD_YL3_THH_4;
	rSmoothMFB.mbd_con8.bits.BLD_MBD_CL1_THL_0       =       BLD_MBD_CL1_THL_0;
	rSmoothMFB.mbd_con8.bits.BLD_MBD_CL1_THL_1       =       BLD_MBD_CL1_THL_1;
	rSmoothMFB.mbd_con9.bits.BLD_MBD_CL1_THL_2       =       BLD_MBD_CL1_THL_2;
	rSmoothMFB.mbd_con9.bits.BLD_MBD_CL1_THL_3       =       BLD_MBD_CL1_THL_3;
	rSmoothMFB.mbd_con9.bits.BLD_MBD_CL1_THL_4       =       BLD_MBD_CL1_THL_4;
	rSmoothMFB.mbd_con9.bits.BLD_MBD_CL1_THH_0       =       BLD_MBD_CL1_THH_0;
	rSmoothMFB.mbd_con10.bits.BLD_MBD_CL1_THH_1       =      BLD_MBD_CL1_THH_1;
	rSmoothMFB.mbd_con10.bits.BLD_MBD_CL1_THH_2       =      BLD_MBD_CL1_THH_2;
	rSmoothMFB.mbd_con10.bits.BLD_MBD_CL1_THH_3       =      BLD_MBD_CL1_THH_3;
	rSmoothMFB.mbd_con10.bits.BLD_MBD_CL1_THH_4       =      BLD_MBD_CL1_THH_4;

	// use dynamic
	rSmoothMFB.con.bits.BLD_MODE    =      BLD_MODE;
	rSmoothMFB.con.bits.BLD_LL_BRZ_EN    =      BLD_LL_BRZ_EN;
	rSmoothMFB.con.bits.BLD_LL_DB_EN   =      BLD_LL_DB_EN;
	rSmoothMFB.con.bits.BLD_MBD_WT_EN   =      BLD_MBD_WT_EN;
	rSmoothMFB.con.bits.BLD_SR_WT_EN   =      BLD_SR_WT_EN;
	rSmoothMFB.con.bits.BLD_LL_TH_E    =      BLD_LL_TH_E;
	rSmoothMFB.ll_con4.bits.BLD_LL_DB_XDIST  =      BLD_LL_DB_XDIST;
	rSmoothMFB.ll_con4.bits.BLD_LL_DB_YDIST  =      BLD_LL_DB_YDIST;
	rSmoothMFB.ll_con5.bits.BLD_LL_GRAD_R1   =      BLD_LL_GRAD_R1;
	rSmoothMFB.ll_con5.bits.BLD_LL_GRAD_R2   =      BLD_LL_GRAD_R2;
	rSmoothMFB.ll_con5.bits.BLD_LL_TH3   =      BLD_LL_TH3;
	rSmoothMFB.ll_con5.bits.BLD_LL_TH4   =      BLD_LL_TH4;
	rSmoothMFB.ll_con6.bits.BLD_LL_TH5   =      BLD_LL_TH5;
	rSmoothMFB.ll_con6.bits.BLD_LL_TH6   =      BLD_LL_TH6;
	rSmoothMFB.ll_con6.bits.BLD_LL_TH7   =      BLD_LL_TH7;
	rSmoothMFB.ll_con6.bits.BLD_LL_TH8   =      BLD_LL_TH8;
	rSmoothMFB.ll_con7.bits.BLD_LL_C1   =      BLD_LL_C1;
	rSmoothMFB.ll_con7.bits.BLD_LL_C2   =      BLD_LL_C2;
	rSmoothMFB.ll_con7.bits.BLD_LL_C3   =      BLD_LL_C3;
	rSmoothMFB.ll_con8.bits.BLD_LL_SU1  =      BLD_LL_SU1;
	rSmoothMFB.ll_con8.bits.BLD_LL_SU2  =      BLD_LL_SU2;
	rSmoothMFB.ll_con8.bits.BLD_LL_SU3  =      BLD_LL_SU3;
	rSmoothMFB.ll_con9.bits.BLD_LL_SL1  =      BLD_LL_SL1;
	rSmoothMFB.ll_con9.bits.BLD_LL_SL2  =      BLD_LL_SL2;
	rSmoothMFB.ll_con9.bits.BLD_LL_SL3  =      BLD_LL_SL3;
	rSmoothMFB.ll_con10.bits.BLD_CONF_MAP_EN  =      BLD_CONF_MAP_EN;
	rSmoothMFB.ll_con10.bits.BLD_LL_GRAD_EN   =      BLD_LL_GRAD_EN;
	rSmoothMFB.ll_con10.bits.BLD_LL_GRAD_ENTH  =      BLD_LL_GRAD_ENTH;


	#define MFB_DUMP(item1, item2)   \
        do{                      \
            INTER_LOG_IF(bSmoothMFBDebug, "[S, L_L, L_U, U_L, U_U]%22s = %d, %d, %d, %d, %d", #item2, rSmoothMFB.item1.bits.item2, rSmoothMFB.item1.bits.item2, rSmoothMFB.item1.bits.item2, rSmoothMFB.item1.bits.item2, rSmoothMFB.item1.bits.item2); \
        }while(0)

    MFB_DUMP(ll_con1, BLD_LL_CLIP_TH2);
	MFB_DUMP(ll_con1, BLD_LL_CLIP_TH1);
	MFB_DUMP(ll_con1, BLD_LL_FLT_WT_MODE2);
	MFB_DUMP(ll_con1, BLD_LL_FLT_WT_MODE1);
	MFB_DUMP(ll_con1, BLD_LL_FLT_MODE);
	MFB_DUMP(ll_con2, BLD_LL_TH2);
	MFB_DUMP(ll_con2, BLD_LL_TH1);
	MFB_DUMP(ll_con2, BLD_LL_DT1);
	MFB_DUMP(ll_con2, BLD_LL_MAX_WT);
	MFB_DUMP(mbd_con0, BLD_MBD_MAX_WT);
	MFB_DUMP(mbd_con1, BLD_MBD_YL1_THL_0);
	MFB_DUMP(mbd_con1, BLD_MBD_YL1_THL_1);
	MFB_DUMP(mbd_con1, BLD_MBD_YL1_THL_2);
	MFB_DUMP(mbd_con1, BLD_MBD_YL1_THL_3);
	MFB_DUMP(mbd_con2, BLD_MBD_YL1_THL_4);
	MFB_DUMP(mbd_con2, BLD_MBD_YL1_THH_0);
	MFB_DUMP(mbd_con2, BLD_MBD_YL1_THH_1);
	MFB_DUMP(mbd_con2, BLD_MBD_YL1_THH_2);
	MFB_DUMP(mbd_con3, BLD_MBD_YL1_THH_3);
	MFB_DUMP(mbd_con3, BLD_MBD_YL1_THH_4);
	MFB_DUMP(mbd_con3, BLD_MBD_YL2_THL_0);
	MFB_DUMP(mbd_con3, BLD_MBD_YL2_THL_1);
	MFB_DUMP(mbd_con4, BLD_MBD_YL2_THL_2);
	MFB_DUMP(mbd_con4, BLD_MBD_YL2_THL_3);
	MFB_DUMP(mbd_con4, BLD_MBD_YL2_THL_4);
	MFB_DUMP(mbd_con4, BLD_MBD_YL2_THH_0);
	MFB_DUMP(mbd_con5, BLD_MBD_YL2_THH_1);
	MFB_DUMP(mbd_con5, BLD_MBD_YL2_THH_2);
	MFB_DUMP(mbd_con5, BLD_MBD_YL2_THH_3);
	MFB_DUMP(mbd_con5, BLD_MBD_YL2_THH_4);
	MFB_DUMP(mbd_con6, BLD_MBD_YL3_THL_0);
	MFB_DUMP(mbd_con6, BLD_MBD_YL3_THL_1);
	MFB_DUMP(mbd_con6, BLD_MBD_YL3_THL_2);
	MFB_DUMP(mbd_con6, BLD_MBD_YL3_THL_3);
	MFB_DUMP(mbd_con7, BLD_MBD_YL3_THL_4);
	MFB_DUMP(mbd_con7, BLD_MBD_YL3_THH_0);
	MFB_DUMP(mbd_con7, BLD_MBD_YL3_THH_1);
	MFB_DUMP(mbd_con7, BLD_MBD_YL3_THH_2);
	MFB_DUMP(mbd_con8, BLD_MBD_YL3_THH_3);
	MFB_DUMP(mbd_con8, BLD_MBD_YL3_THH_4);
	MFB_DUMP(mbd_con8, BLD_MBD_CL1_THL_0);
	MFB_DUMP(mbd_con8, BLD_MBD_CL1_THL_1);
	MFB_DUMP(mbd_con9, BLD_MBD_CL1_THL_2);
	MFB_DUMP(mbd_con9, BLD_MBD_CL1_THL_3);
	MFB_DUMP(mbd_con9, BLD_MBD_CL1_THL_4);
	MFB_DUMP(mbd_con9, BLD_MBD_CL1_THH_0);
	MFB_DUMP(mbd_con10, BLD_MBD_CL1_THH_1);
	MFB_DUMP(mbd_con10, BLD_MBD_CL1_THH_2);
	MFB_DUMP(mbd_con10, BLD_MBD_CL1_THH_3);
	MFB_DUMP(mbd_con10, BLD_MBD_CL1_THH_4);

	MFB_DUMP(con, BLD_MODE);
	MFB_DUMP(con, BLD_LL_BRZ_EN);
	MFB_DUMP(con, BLD_LL_DB_EN);
	MFB_DUMP(con, BLD_MBD_WT_EN);
	MFB_DUMP(con, BLD_SR_WT_EN);
	MFB_DUMP(con, BLD_LL_TH_E);
	MFB_DUMP(ll_con4, BLD_LL_DB_XDIST);
	MFB_DUMP(ll_con4, BLD_LL_DB_YDIST);
	MFB_DUMP(ll_con5, BLD_LL_GRAD_R1);
	MFB_DUMP(ll_con5, BLD_LL_GRAD_R2);
	MFB_DUMP(ll_con5, BLD_LL_TH3);
	MFB_DUMP(ll_con5, BLD_LL_TH4);
	MFB_DUMP(ll_con6, BLD_LL_TH5);
	MFB_DUMP(ll_con6, BLD_LL_TH6);
	MFB_DUMP(ll_con6, BLD_LL_TH7);
	MFB_DUMP(ll_con6, BLD_LL_TH8);
	MFB_DUMP(ll_con7, BLD_LL_C1);
	MFB_DUMP(ll_con7, BLD_LL_C2);
	MFB_DUMP(ll_con7, BLD_LL_C3);
	MFB_DUMP(ll_con8, BLD_LL_SU1);
	MFB_DUMP(ll_con8, BLD_LL_SU2);
	MFB_DUMP(ll_con8, BLD_LL_SU3);
	MFB_DUMP(ll_con9, BLD_LL_SL1);
	MFB_DUMP(ll_con9, BLD_LL_SL2);
	MFB_DUMP(ll_con9, BLD_LL_SL3);
	MFB_DUMP(ll_con10, BLD_CONF_MAP_EN);
	MFB_DUMP(ll_con10, BLD_LL_GRAD_EN);
	MFB_DUMP(ll_con10, BLD_LL_GRAD_ENTH);



}


MINT32 InterParam_NR3D(double Ratio_L_L, double Ratio_L_U, double Ratio_U_L, double Ratio_U_U, MINT32 PARAM_L_L, MINT32 PARAM_L_U, MINT32 PARAM_U_L, MINT32 PARAM_U_U, MINT32 Method) {
    MINT32 InterPARAM = 0;


    switch(Method) {
    case 1:
        if( Ratio_L_L > Ratio_L_U)
		{
            if( Ratio_L_L > Ratio_U_L)
			{
                InterPARAM = PARAM_L_L;
        	}
        	else
			{
                InterPARAM = PARAM_U_L;
        	}
        }
        else
		{
            if( Ratio_L_L > Ratio_U_L)
			{
                InterPARAM = PARAM_L_U;
        	}
        	else
			{
                InterPARAM = PARAM_U_U;
        	}
        }
        break;
    case 0:
    default:
        InterPARAM = (MINT32)(Ratio_L_L * PARAM_L_L + Ratio_L_U * PARAM_L_U + Ratio_U_L * PARAM_U_L + Ratio_U_U * PARAM_U_U + 0.5);
        break;
    }

    return InterPARAM;
}

MVOID SmoothNR3D(ISP_NVRAM_NR3D_INT_T const& rParam,   // EE settings
                ISP_NVRAM_NR3D_T& rSmoothNR3D,
               ISP_NR3D_SMOOTH_INFO_T const& rNR3D_SW_Params)  // Output
{
	MBOOL bSmoothNR3DDebug = interpPropertyGet("debug.nr3d.bm.enable", MFALSE);

	static MINT32 bmCount = 0;
	bmCount++;

	MINT32 u4MAX_GMV = rNR3D_SW_Params.M_Info.MAX_GMV;
	u4MAX_GMV = 32;


	MINT32 u4RealISO = rParam.u4RealISO;
	MINT32 u4UpperISO = rParam.u4UpperISO;
	MINT32 u4LowerISO = rParam.u4LowerISO;
	MINT32 fRealZoom = rParam.i4ZoomRatio_x100;
	MINT32 fUpperZoom = rParam.u4UpperZoom;
	MINT32 fLowerZoom = rParam.u4LowerZoom;




	if (bSmoothNR3DDebug>=2 && bmCount % 33 == 0)
	{
		INTER_LOG_IF(bSmoothNR3DDebug>=2,"[%s()] u4RealISO: %d, u4UpperISO = %d, u4LowerISO = %d\n", __FUNCTION__, u4RealISO, u4UpperISO, u4LowerISO);
		INTER_LOG_IF(bSmoothNR3DDebug>=2,"[%s()] u4RealZoom: %d, u4UpperZoom = %d, u4LowerZoom = %d\n", __FUNCTION__, fRealZoom, fUpperZoom, fLowerZoom);
	}

	// _L_L = Low ISO Low Zoom, _L_U = Low ISO Up Zoom, _U_L = Up ISO Low Zoom, _U_U = Up ISO Up Zoom

	double RATIO_ISO_lo = 0;
	double RATIO_ISO_up = 0;
	double RATIO_Zoom_lo = 0;
	double RATIO_Zoom_up = 0;

    //protection for out-of-bound


    if (u4RealISO <= u4LowerISO)
    {
        RATIO_ISO_lo = 1;
    }
	else if(u4RealISO >= u4UpperISO)
	{
		RATIO_ISO_lo = 0;
	}
	else
	{
		RATIO_ISO_lo = (double)(u4UpperISO - u4RealISO) / (double)(u4UpperISO - u4LowerISO);
	}

	RATIO_ISO_up = 1.0 - RATIO_ISO_lo;

	if (fRealZoom <= fLowerZoom)
    {
        RATIO_Zoom_lo = 1;
    }
	else if(fRealZoom >= fUpperZoom)
	{
		RATIO_Zoom_lo = 0;
	}
	else
	{
		RATIO_Zoom_lo = (double)(fUpperZoom - fRealZoom) / (double)(fUpperZoom - fLowerZoom);
	}

	RATIO_Zoom_up = 1.0 - RATIO_Zoom_lo;

	double RATIO_L_L = (double)(RATIO_ISO_lo * RATIO_Zoom_lo);
	double RATIO_L_U = (double)(RATIO_ISO_lo * RATIO_Zoom_up);
	double RATIO_U_L = (double)(RATIO_ISO_up * RATIO_Zoom_lo);
	double RATIO_U_U = (double)(RATIO_ISO_up * RATIO_Zoom_up);


	// ======================
		// Low Iso Low Zoom
	// ======================

	MINT32 NR3D_SL2_OFF_L_L = rParam.rLowerIso_LowerZoom.on_con.bits.NR3D_SL2_OFF;
	MINT32 NR3D_TNR_Y_EN_L_L = rParam.rLowerIso_LowerZoom.tnr_tnr_enable.bits.NR3D_TNR_Y_EN;
	MINT32 NR3D_TNR_C_EN_L_L = rParam.rLowerIso_LowerZoom.tnr_tnr_enable.bits.NR3D_TNR_C_EN;
	MINT32 NR3D_BLEND_RATIO_BLKY_L_L  =  rParam.rLowerIso_LowerZoom.tnr_flt_config.bits.NR3D_BLEND_RATIO_BLKY;
	MINT32 NR3D_BLEND_RATIO_DE_L_L	=  rParam.rLowerIso_LowerZoom.tnr_flt_config.bits.NR3D_BLEND_RATIO_DE;
	MINT32 NR3D_BLEND_RATIO_TXTR_L_L  =  rParam.rLowerIso_LowerZoom.tnr_flt_config.bits.NR3D_BLEND_RATIO_TXTR;
	MINT32 NR3D_BLEND_RATIO_MV_L_L	=  rParam.rLowerIso_LowerZoom.tnr_flt_config.bits.NR3D_BLEND_RATIO_MV;
	MINT32 NR3D_FLT_STR_MAX_L_L  =	rParam.rLowerIso_LowerZoom.tnr_flt_config.bits.NR3D_FLT_STR_MAX;
	MINT32 NR3D_Q_NL_L_L  =  rParam.rLowerIso_LowerZoom.tnr_fb_info1.bits.NR3D_Q_NL;
	MINT32 NR3D_Q_SP_L_L  =  rParam.rLowerIso_LowerZoom.tnr_fb_info1.bits.NR3D_Q_SP;
	MINT32 NR3D_BDI_THR_L_L  =	rParam.rLowerIso_LowerZoom.tnr_thr_1.bits.NR3D_BDI_THR;
	MINT32 NR3D_MV_PEN_W_L_L  =  rParam.rLowerIso_LowerZoom.tnr_thr_1.bits.NR3D_MV_PEN_W;
	MINT32 NR3D_MV_PEN_THR_L_L	=  rParam.rLowerIso_LowerZoom.tnr_thr_1.bits.NR3D_MV_PEN_THR;
	MINT32 NR3D_SMALL_SAD_THR_L_L  =  rParam.rLowerIso_LowerZoom.tnr_thr_1.bits.NR3D_SMALL_SAD_THR;
	MINT32 NR3D_Q_BLKY_Y0_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_1.bits.NR3D_Q_BLKY_Y0;
	MINT32 NR3D_Q_BLKY_Y1_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_1.bits.NR3D_Q_BLKY_Y1;
	MINT32 NR3D_Q_BLKY_Y2_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_1.bits.NR3D_Q_BLKY_Y2;
	MINT32 NR3D_Q_BLKY_Y3_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_1.bits.NR3D_Q_BLKY_Y3;
	MINT32 NR3D_Q_BLKY_Y4_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_1.bits.NR3D_Q_BLKY_Y4;
	MINT32 NR3D_Q_BLKY_Y5_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_2.bits.NR3D_Q_BLKY_Y5;
	MINT32 NR3D_Q_BLKY_Y6_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_2.bits.NR3D_Q_BLKY_Y6;
	MINT32 NR3D_Q_BLKY_Y7_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_2.bits.NR3D_Q_BLKY_Y7;
	MINT32 NR3D_Q_BLKC_Y0_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_2.bits.NR3D_Q_BLKC_Y0;
	MINT32 NR3D_Q_BLKC_Y1_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_2.bits.NR3D_Q_BLKC_Y1;
	MINT32 NR3D_Q_BLKC_Y2_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_3.bits.NR3D_Q_BLKC_Y2;
	MINT32 NR3D_Q_BLKC_Y3_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_3.bits.NR3D_Q_BLKC_Y3;
	MINT32 NR3D_Q_BLKC_Y4_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_3.bits.NR3D_Q_BLKC_Y4;
	MINT32 NR3D_Q_BLKC_Y5_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_3.bits.NR3D_Q_BLKC_Y5;
	MINT32 NR3D_Q_BLKC_Y6_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_3.bits.NR3D_Q_BLKC_Y6;
	MINT32 NR3D_Q_BLKC_Y7_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_4.bits.NR3D_Q_BLKC_Y7;
	MINT32 NR3D_Q_DETXTR_LVL_Y0_L_L  =	rParam.rLowerIso_LowerZoom.tnr_curve_4.bits.NR3D_Q_DETXTR_LVL_Y0;
	MINT32 NR3D_Q_DETXTR_LVL_Y1_L_L  =	rParam.rLowerIso_LowerZoom.tnr_curve_4.bits.NR3D_Q_DETXTR_LVL_Y1;
	MINT32 NR3D_Q_DETXTR_LVL_Y2_L_L  =	rParam.rLowerIso_LowerZoom.tnr_curve_4.bits.NR3D_Q_DETXTR_LVL_Y2;
	MINT32 NR3D_Q_DETXTR_LVL_Y3_L_L  =	rParam.rLowerIso_LowerZoom.tnr_curve_4.bits.NR3D_Q_DETXTR_LVL_Y3;
	MINT32 NR3D_Q_DETXTR_LVL_Y4_L_L  =	rParam.rLowerIso_LowerZoom.tnr_curve_5.bits.NR3D_Q_DETXTR_LVL_Y4;
	MINT32 NR3D_Q_DETXTR_LVL_Y5_L_L  =	rParam.rLowerIso_LowerZoom.tnr_curve_5.bits.NR3D_Q_DETXTR_LVL_Y5;
	MINT32 NR3D_Q_DETXTR_LVL_Y6_L_L  =	rParam.rLowerIso_LowerZoom.tnr_curve_5.bits.NR3D_Q_DETXTR_LVL_Y6;
	MINT32 NR3D_Q_DETXTR_LVL_Y7_L_L  =	rParam.rLowerIso_LowerZoom.tnr_curve_5.bits.NR3D_Q_DETXTR_LVL_Y7;
	MINT32 NR3D_Q_DE1_BASE_Y0_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_5.bits.NR3D_Q_DE1_BASE_Y0;
	MINT32 NR3D_Q_DE1_BASE_Y1_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_6.bits.NR3D_Q_DE1_BASE_Y1;
	MINT32 NR3D_Q_DE1_BASE_Y2_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_6.bits.NR3D_Q_DE1_BASE_Y2;
	MINT32 NR3D_Q_DE1_BASE_Y3_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_6.bits.NR3D_Q_DE1_BASE_Y3;
	MINT32 NR3D_Q_DE1_BASE_Y4_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_6.bits.NR3D_Q_DE1_BASE_Y4;
	MINT32 NR3D_Q_DE1_BASE_Y5_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_6.bits.NR3D_Q_DE1_BASE_Y5;
	MINT32 NR3D_Q_DE1_BASE_Y6_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_7.bits.NR3D_Q_DE1_BASE_Y6;
	MINT32 NR3D_Q_DE1_BASE_Y7_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_7.bits.NR3D_Q_DE1_BASE_Y7;
	MINT32 NR3D_Q_DE2TXTR_BASE_Y0_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_7.bits.NR3D_Q_DE2TXTR_BASE_Y0;
	MINT32 NR3D_Q_DE2TXTR_BASE_Y1_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_7.bits.NR3D_Q_DE2TXTR_BASE_Y1;
	MINT32 NR3D_Q_DE2TXTR_BASE_Y2_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_7.bits.NR3D_Q_DE2TXTR_BASE_Y2;
	MINT32 NR3D_Q_DE2TXTR_BASE_Y3_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_8.bits.NR3D_Q_DE2TXTR_BASE_Y3;
	MINT32 NR3D_Q_DE2TXTR_BASE_Y4_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_8.bits.NR3D_Q_DE2TXTR_BASE_Y4;
	MINT32 NR3D_Q_DE2TXTR_BASE_Y5_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_8.bits.NR3D_Q_DE2TXTR_BASE_Y5;
	MINT32 NR3D_Q_DE2TXTR_BASE_Y6_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_8.bits.NR3D_Q_DE2TXTR_BASE_Y6;
	MINT32 NR3D_Q_DE2TXTR_BASE_Y7_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_8.bits.NR3D_Q_DE2TXTR_BASE_Y7;
	MINT32 NR3D_Q_MV_Y0_L_L  =	rParam.rLowerIso_LowerZoom.tnr_curve_9.bits.NR3D_Q_MV_Y0;
	MINT32 NR3D_Q_MV_Y1_L_L  =	rParam.rLowerIso_LowerZoom.tnr_curve_9.bits.NR3D_Q_MV_Y1;
	MINT32 NR3D_Q_MV_Y2_L_L  =	rParam.rLowerIso_LowerZoom.tnr_curve_9.bits.NR3D_Q_MV_Y2;
	MINT32 NR3D_Q_MV_Y3_L_L  =	rParam.rLowerIso_LowerZoom.tnr_curve_9.bits.NR3D_Q_MV_Y3;
	MINT32 NR3D_Q_MV_Y4_L_L  =	rParam.rLowerIso_LowerZoom.tnr_curve_9.bits.NR3D_Q_MV_Y4;
	MINT32 NR3D_Q_MV_Y5_L_L  =	rParam.rLowerIso_LowerZoom.tnr_curve_10.bits.NR3D_Q_MV_Y5;
	MINT32 NR3D_Q_MV_Y6_L_L  =	rParam.rLowerIso_LowerZoom.tnr_curve_10.bits.NR3D_Q_MV_Y6;
	MINT32 NR3D_Q_MV_Y7_L_L  =	rParam.rLowerIso_LowerZoom.tnr_curve_10.bits.NR3D_Q_MV_Y7;
	MINT32 NR3D_Q_WVAR_Y0_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_10.bits.NR3D_Q_WVAR_Y0;
	MINT32 NR3D_Q_WVAR_Y1_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_10.bits.NR3D_Q_WVAR_Y1;
	MINT32 NR3D_Q_WVAR_Y2_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_11.bits.NR3D_Q_WVAR_Y2;
	MINT32 NR3D_Q_WVAR_Y3_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_11.bits.NR3D_Q_WVAR_Y3;
	MINT32 NR3D_Q_WVAR_Y4_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_11.bits.NR3D_Q_WVAR_Y4;
	MINT32 NR3D_Q_WVAR_Y5_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_11.bits.NR3D_Q_WVAR_Y5;
	MINT32 NR3D_Q_WVAR_Y6_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_11.bits.NR3D_Q_WVAR_Y6;
	MINT32 NR3D_Q_WVAR_Y7_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_12.bits.NR3D_Q_WVAR_Y7;
	MINT32 NR3D_Q_WSM_Y0_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_12.bits.NR3D_Q_WSM_Y0;
	MINT32 NR3D_Q_WSM_Y1_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_12.bits.NR3D_Q_WSM_Y1;
	MINT32 NR3D_Q_WSM_Y2_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_12.bits.NR3D_Q_WSM_Y2;
	MINT32 NR3D_Q_WSM_Y3_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_12.bits.NR3D_Q_WSM_Y3;
	MINT32 NR3D_Q_WSM_Y4_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_13.bits.NR3D_Q_WSM_Y4;
	MINT32 NR3D_Q_WSM_Y5_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_13.bits.NR3D_Q_WSM_Y5;
	MINT32 NR3D_Q_WSM_Y6_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_13.bits.NR3D_Q_WSM_Y6;
	MINT32 NR3D_Q_WSM_Y7_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_13.bits.NR3D_Q_WSM_Y7;
	MINT32 NR3D_Q_SDL_Y0_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_14.bits.NR3D_Q_SDL_Y0;
	MINT32 NR3D_Q_SDL_Y1_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_14.bits.NR3D_Q_SDL_Y1;
	MINT32 NR3D_Q_SDL_Y2_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_14.bits.NR3D_Q_SDL_Y2;
	MINT32 NR3D_Q_SDL_Y3_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_14.bits.NR3D_Q_SDL_Y3;
	MINT32 NR3D_Q_SDL_Y4_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_14.bits.NR3D_Q_SDL_Y4;
	MINT32 NR3D_Q_SDL_Y5_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_15.bits.NR3D_Q_SDL_Y5;
	MINT32 NR3D_Q_SDL_Y6_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_15.bits.NR3D_Q_SDL_Y6;
	MINT32 NR3D_Q_SDL_Y7_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_15.bits.NR3D_Q_SDL_Y7;
	MINT32 NR3D_Q_SDL_Y8_L_L  =  rParam.rLowerIso_LowerZoom.tnr_curve_15.bits.NR3D_Q_SDL_Y8;

    //R2C Kib+ & Whitne E2
	MINT32 NR3D_R2CENC_L_L  =  rParam.rLowerIso_LowerZoom.tnr_r2c_1.bits.NR3D_R2CENC;
	MINT32 NR3D_R2C_VAL4_L_L  =  rParam.rLowerIso_LowerZoom.tnr_r2c_1.bits.NR3D_R2C_VAL4;
	MINT32 NR3D_R2C_VAL3_L_L  =  rParam.rLowerIso_LowerZoom.tnr_r2c_1.bits.NR3D_R2C_VAL3;
	MINT32 NR3D_R2C_VAL2_L_L  =  rParam.rLowerIso_LowerZoom.tnr_r2c_1.bits.NR3D_R2C_VAL2;
	MINT32 NR3D_R2C_VAL1_L_L  =  rParam.rLowerIso_LowerZoom.tnr_r2c_1.bits.NR3D_R2C_VAL1;
	MINT32 NR3D_R2C_TXTR_THROFF_L_L  =  rParam.rLowerIso_LowerZoom.tnr_r2c_2.bits.NR3D_R2C_TXTR_THROFF;
	MINT32 NR3D_R2C_TXTR_THR4_L_L  =  rParam.rLowerIso_LowerZoom.tnr_r2c_2.bits.NR3D_R2C_TXTR_THR4;
	MINT32 NR3D_R2C_TXTR_THR3_L_L  =  rParam.rLowerIso_LowerZoom.tnr_r2c_2.bits.NR3D_R2C_TXTR_THR3;
	MINT32 NR3D_R2C_TXTR_THR2_L_L  =  rParam.rLowerIso_LowerZoom.tnr_r2c_2.bits.NR3D_R2C_TXTR_THR2;
	MINT32 NR3D_R2C_TXTR_THR1_L_L  =  rParam.rLowerIso_LowerZoom.tnr_r2c_2.bits.NR3D_R2C_TXTR_THR1;

	// ======================
		// Low Iso Up Zoom
	// ======================

	MINT32 NR3D_SL2_OFF_L_U = rParam.rLowerIso_UpperZoom.on_con.bits.NR3D_SL2_OFF;
	MINT32 NR3D_TNR_Y_EN_L_U = rParam.rLowerIso_UpperZoom.tnr_tnr_enable.bits.NR3D_TNR_Y_EN;
	MINT32 NR3D_TNR_C_EN_L_U = rParam.rLowerIso_UpperZoom.tnr_tnr_enable.bits.NR3D_TNR_C_EN;
	MINT32 NR3D_BLEND_RATIO_BLKY_L_U  =  rParam.rLowerIso_UpperZoom.tnr_flt_config.bits.NR3D_BLEND_RATIO_BLKY;
	MINT32 NR3D_BLEND_RATIO_DE_L_U	=  rParam.rLowerIso_UpperZoom.tnr_flt_config.bits.NR3D_BLEND_RATIO_DE;
	MINT32 NR3D_BLEND_RATIO_TXTR_L_U  =  rParam.rLowerIso_UpperZoom.tnr_flt_config.bits.NR3D_BLEND_RATIO_TXTR;
	MINT32 NR3D_BLEND_RATIO_MV_L_U	=  rParam.rLowerIso_UpperZoom.tnr_flt_config.bits.NR3D_BLEND_RATIO_MV;
	MINT32 NR3D_FLT_STR_MAX_L_U  =	rParam.rLowerIso_UpperZoom.tnr_flt_config.bits.NR3D_FLT_STR_MAX;
	MINT32 NR3D_Q_NL_L_U  =  rParam.rLowerIso_UpperZoom.tnr_fb_info1.bits.NR3D_Q_NL;
	MINT32 NR3D_Q_SP_L_U  =  rParam.rLowerIso_UpperZoom.tnr_fb_info1.bits.NR3D_Q_SP;
	MINT32 NR3D_BDI_THR_L_U  =	rParam.rLowerIso_UpperZoom.tnr_thr_1.bits.NR3D_BDI_THR;
	MINT32 NR3D_MV_PEN_W_L_U  =  rParam.rLowerIso_UpperZoom.tnr_thr_1.bits.NR3D_MV_PEN_W;
	MINT32 NR3D_MV_PEN_THR_L_U	=  rParam.rLowerIso_UpperZoom.tnr_thr_1.bits.NR3D_MV_PEN_THR;
	MINT32 NR3D_SMALL_SAD_THR_L_U  =  rParam.rLowerIso_UpperZoom.tnr_thr_1.bits.NR3D_SMALL_SAD_THR;
	MINT32 NR3D_Q_BLKY_Y0_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_1.bits.NR3D_Q_BLKY_Y0;
	MINT32 NR3D_Q_BLKY_Y1_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_1.bits.NR3D_Q_BLKY_Y1;
	MINT32 NR3D_Q_BLKY_Y2_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_1.bits.NR3D_Q_BLKY_Y2;
	MINT32 NR3D_Q_BLKY_Y3_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_1.bits.NR3D_Q_BLKY_Y3;
	MINT32 NR3D_Q_BLKY_Y4_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_1.bits.NR3D_Q_BLKY_Y4;
	MINT32 NR3D_Q_BLKY_Y5_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_2.bits.NR3D_Q_BLKY_Y5;
	MINT32 NR3D_Q_BLKY_Y6_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_2.bits.NR3D_Q_BLKY_Y6;
	MINT32 NR3D_Q_BLKY_Y7_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_2.bits.NR3D_Q_BLKY_Y7;
	MINT32 NR3D_Q_BLKC_Y0_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_2.bits.NR3D_Q_BLKC_Y0;
	MINT32 NR3D_Q_BLKC_Y1_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_2.bits.NR3D_Q_BLKC_Y1;
	MINT32 NR3D_Q_BLKC_Y2_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_3.bits.NR3D_Q_BLKC_Y2;
	MINT32 NR3D_Q_BLKC_Y3_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_3.bits.NR3D_Q_BLKC_Y3;
	MINT32 NR3D_Q_BLKC_Y4_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_3.bits.NR3D_Q_BLKC_Y4;
	MINT32 NR3D_Q_BLKC_Y5_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_3.bits.NR3D_Q_BLKC_Y5;
	MINT32 NR3D_Q_BLKC_Y6_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_3.bits.NR3D_Q_BLKC_Y6;
	MINT32 NR3D_Q_BLKC_Y7_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_4.bits.NR3D_Q_BLKC_Y7;
	MINT32 NR3D_Q_DETXTR_LVL_Y0_L_U  =	rParam.rLowerIso_UpperZoom.tnr_curve_4.bits.NR3D_Q_DETXTR_LVL_Y0;
	MINT32 NR3D_Q_DETXTR_LVL_Y1_L_U  =	rParam.rLowerIso_UpperZoom.tnr_curve_4.bits.NR3D_Q_DETXTR_LVL_Y1;
	MINT32 NR3D_Q_DETXTR_LVL_Y2_L_U  =	rParam.rLowerIso_UpperZoom.tnr_curve_4.bits.NR3D_Q_DETXTR_LVL_Y2;
	MINT32 NR3D_Q_DETXTR_LVL_Y3_L_U  =	rParam.rLowerIso_UpperZoom.tnr_curve_4.bits.NR3D_Q_DETXTR_LVL_Y3;
	MINT32 NR3D_Q_DETXTR_LVL_Y4_L_U  =	rParam.rLowerIso_UpperZoom.tnr_curve_5.bits.NR3D_Q_DETXTR_LVL_Y4;
	MINT32 NR3D_Q_DETXTR_LVL_Y5_L_U  =	rParam.rLowerIso_UpperZoom.tnr_curve_5.bits.NR3D_Q_DETXTR_LVL_Y5;
	MINT32 NR3D_Q_DETXTR_LVL_Y6_L_U  =	rParam.rLowerIso_UpperZoom.tnr_curve_5.bits.NR3D_Q_DETXTR_LVL_Y6;
	MINT32 NR3D_Q_DETXTR_LVL_Y7_L_U  =	rParam.rLowerIso_UpperZoom.tnr_curve_5.bits.NR3D_Q_DETXTR_LVL_Y7;
	MINT32 NR3D_Q_DE1_BASE_Y0_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_5.bits.NR3D_Q_DE1_BASE_Y0;
	MINT32 NR3D_Q_DE1_BASE_Y1_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_6.bits.NR3D_Q_DE1_BASE_Y1;
	MINT32 NR3D_Q_DE1_BASE_Y2_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_6.bits.NR3D_Q_DE1_BASE_Y2;
	MINT32 NR3D_Q_DE1_BASE_Y3_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_6.bits.NR3D_Q_DE1_BASE_Y3;
	MINT32 NR3D_Q_DE1_BASE_Y4_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_6.bits.NR3D_Q_DE1_BASE_Y4;
	MINT32 NR3D_Q_DE1_BASE_Y5_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_6.bits.NR3D_Q_DE1_BASE_Y5;
	MINT32 NR3D_Q_DE1_BASE_Y6_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_7.bits.NR3D_Q_DE1_BASE_Y6;
	MINT32 NR3D_Q_DE1_BASE_Y7_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_7.bits.NR3D_Q_DE1_BASE_Y7;
	MINT32 NR3D_Q_DE2TXTR_BASE_Y0_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_7.bits.NR3D_Q_DE2TXTR_BASE_Y0;
	MINT32 NR3D_Q_DE2TXTR_BASE_Y1_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_7.bits.NR3D_Q_DE2TXTR_BASE_Y1;
	MINT32 NR3D_Q_DE2TXTR_BASE_Y2_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_7.bits.NR3D_Q_DE2TXTR_BASE_Y2;
	MINT32 NR3D_Q_DE2TXTR_BASE_Y3_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_8.bits.NR3D_Q_DE2TXTR_BASE_Y3;
	MINT32 NR3D_Q_DE2TXTR_BASE_Y4_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_8.bits.NR3D_Q_DE2TXTR_BASE_Y4;
	MINT32 NR3D_Q_DE2TXTR_BASE_Y5_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_8.bits.NR3D_Q_DE2TXTR_BASE_Y5;
	MINT32 NR3D_Q_DE2TXTR_BASE_Y6_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_8.bits.NR3D_Q_DE2TXTR_BASE_Y6;
	MINT32 NR3D_Q_DE2TXTR_BASE_Y7_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_8.bits.NR3D_Q_DE2TXTR_BASE_Y7;
	MINT32 NR3D_Q_MV_Y0_L_U  =	rParam.rLowerIso_UpperZoom.tnr_curve_9.bits.NR3D_Q_MV_Y0;
	MINT32 NR3D_Q_MV_Y1_L_U  =	rParam.rLowerIso_UpperZoom.tnr_curve_9.bits.NR3D_Q_MV_Y1;
	MINT32 NR3D_Q_MV_Y2_L_U  =	rParam.rLowerIso_UpperZoom.tnr_curve_9.bits.NR3D_Q_MV_Y2;
	MINT32 NR3D_Q_MV_Y3_L_U  =	rParam.rLowerIso_UpperZoom.tnr_curve_9.bits.NR3D_Q_MV_Y3;
	MINT32 NR3D_Q_MV_Y4_L_U  =	rParam.rLowerIso_UpperZoom.tnr_curve_9.bits.NR3D_Q_MV_Y4;
	MINT32 NR3D_Q_MV_Y5_L_U  =	rParam.rLowerIso_UpperZoom.tnr_curve_10.bits.NR3D_Q_MV_Y5;
	MINT32 NR3D_Q_MV_Y6_L_U  =	rParam.rLowerIso_UpperZoom.tnr_curve_10.bits.NR3D_Q_MV_Y6;
	MINT32 NR3D_Q_MV_Y7_L_U  =	rParam.rLowerIso_UpperZoom.tnr_curve_10.bits.NR3D_Q_MV_Y7;
	MINT32 NR3D_Q_WVAR_Y0_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_10.bits.NR3D_Q_WVAR_Y0;
	MINT32 NR3D_Q_WVAR_Y1_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_10.bits.NR3D_Q_WVAR_Y1;
	MINT32 NR3D_Q_WVAR_Y2_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_11.bits.NR3D_Q_WVAR_Y2;
	MINT32 NR3D_Q_WVAR_Y3_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_11.bits.NR3D_Q_WVAR_Y3;
	MINT32 NR3D_Q_WVAR_Y4_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_11.bits.NR3D_Q_WVAR_Y4;
	MINT32 NR3D_Q_WVAR_Y5_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_11.bits.NR3D_Q_WVAR_Y5;
	MINT32 NR3D_Q_WVAR_Y6_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_11.bits.NR3D_Q_WVAR_Y6;
	MINT32 NR3D_Q_WVAR_Y7_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_12.bits.NR3D_Q_WVAR_Y7;
	MINT32 NR3D_Q_WSM_Y0_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_12.bits.NR3D_Q_WSM_Y0;
	MINT32 NR3D_Q_WSM_Y1_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_12.bits.NR3D_Q_WSM_Y1;
	MINT32 NR3D_Q_WSM_Y2_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_12.bits.NR3D_Q_WSM_Y2;
	MINT32 NR3D_Q_WSM_Y3_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_12.bits.NR3D_Q_WSM_Y3;
	MINT32 NR3D_Q_WSM_Y4_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_13.bits.NR3D_Q_WSM_Y4;
	MINT32 NR3D_Q_WSM_Y5_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_13.bits.NR3D_Q_WSM_Y5;
	MINT32 NR3D_Q_WSM_Y6_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_13.bits.NR3D_Q_WSM_Y6;
	MINT32 NR3D_Q_WSM_Y7_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_13.bits.NR3D_Q_WSM_Y7;
	MINT32 NR3D_Q_SDL_Y0_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_14.bits.NR3D_Q_SDL_Y0;
	MINT32 NR3D_Q_SDL_Y1_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_14.bits.NR3D_Q_SDL_Y1;
	MINT32 NR3D_Q_SDL_Y2_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_14.bits.NR3D_Q_SDL_Y2;
	MINT32 NR3D_Q_SDL_Y3_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_14.bits.NR3D_Q_SDL_Y3;
	MINT32 NR3D_Q_SDL_Y4_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_14.bits.NR3D_Q_SDL_Y4;
	MINT32 NR3D_Q_SDL_Y5_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_15.bits.NR3D_Q_SDL_Y5;
	MINT32 NR3D_Q_SDL_Y6_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_15.bits.NR3D_Q_SDL_Y6;
	MINT32 NR3D_Q_SDL_Y7_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_15.bits.NR3D_Q_SDL_Y7;
	MINT32 NR3D_Q_SDL_Y8_L_U  =  rParam.rLowerIso_UpperZoom.tnr_curve_15.bits.NR3D_Q_SDL_Y8;

	//R2C Kib+ & Whitne E2
	MINT32 NR3D_R2CENC_L_U  =  rParam.rLowerIso_UpperZoom.tnr_r2c_1.bits.NR3D_R2CENC;
	MINT32 NR3D_R2C_VAL4_L_U  =  rParam.rLowerIso_UpperZoom.tnr_r2c_1.bits.NR3D_R2C_VAL4;
	MINT32 NR3D_R2C_VAL3_L_U  =  rParam.rLowerIso_UpperZoom.tnr_r2c_1.bits.NR3D_R2C_VAL3;
	MINT32 NR3D_R2C_VAL2_L_U  =  rParam.rLowerIso_UpperZoom.tnr_r2c_1.bits.NR3D_R2C_VAL2;
	MINT32 NR3D_R2C_VAL1_L_U  =  rParam.rLowerIso_UpperZoom.tnr_r2c_1.bits.NR3D_R2C_VAL1;
	MINT32 NR3D_R2C_TXTR_THROFF_L_U  =  rParam.rLowerIso_UpperZoom.tnr_r2c_2.bits.NR3D_R2C_TXTR_THROFF;
	MINT32 NR3D_R2C_TXTR_THR4_L_U  =  rParam.rLowerIso_UpperZoom.tnr_r2c_2.bits.NR3D_R2C_TXTR_THR4;
	MINT32 NR3D_R2C_TXTR_THR3_L_U  =  rParam.rLowerIso_UpperZoom.tnr_r2c_2.bits.NR3D_R2C_TXTR_THR3;
	MINT32 NR3D_R2C_TXTR_THR2_L_U  =  rParam.rLowerIso_UpperZoom.tnr_r2c_2.bits.NR3D_R2C_TXTR_THR2;
	MINT32 NR3D_R2C_TXTR_THR1_L_U  =  rParam.rLowerIso_UpperZoom.tnr_r2c_2.bits.NR3D_R2C_TXTR_THR1;

	// ======================
		// Up Iso Low Zoom
	// ======================

	MINT32 NR3D_SL2_OFF_U_L = rParam.rUpperIso_LowerZoom.on_con.bits.NR3D_SL2_OFF;
	MINT32 NR3D_TNR_Y_EN_U_L = rParam.rUpperIso_LowerZoom.tnr_tnr_enable.bits.NR3D_TNR_Y_EN;
	MINT32 NR3D_TNR_C_EN_U_L = rParam.rUpperIso_LowerZoom.tnr_tnr_enable.bits.NR3D_TNR_C_EN;
	MINT32 NR3D_BLEND_RATIO_BLKY_U_L  =  rParam.rUpperIso_LowerZoom.tnr_flt_config.bits.NR3D_BLEND_RATIO_BLKY;
	MINT32 NR3D_BLEND_RATIO_DE_U_L	=  rParam.rUpperIso_LowerZoom.tnr_flt_config.bits.NR3D_BLEND_RATIO_DE;
	MINT32 NR3D_BLEND_RATIO_TXTR_U_L  =  rParam.rUpperIso_LowerZoom.tnr_flt_config.bits.NR3D_BLEND_RATIO_TXTR;
	MINT32 NR3D_BLEND_RATIO_MV_U_L	=  rParam.rUpperIso_LowerZoom.tnr_flt_config.bits.NR3D_BLEND_RATIO_MV;
	MINT32 NR3D_FLT_STR_MAX_U_L  =	rParam.rUpperIso_LowerZoom.tnr_flt_config.bits.NR3D_FLT_STR_MAX;
	MINT32 NR3D_Q_NL_U_L  =  rParam.rUpperIso_LowerZoom.tnr_fb_info1.bits.NR3D_Q_NL;
	MINT32 NR3D_Q_SP_U_L  =  rParam.rUpperIso_LowerZoom.tnr_fb_info1.bits.NR3D_Q_SP;
	MINT32 NR3D_BDI_THR_U_L  =	rParam.rUpperIso_LowerZoom.tnr_thr_1.bits.NR3D_BDI_THR;
	MINT32 NR3D_MV_PEN_W_U_L  =  rParam.rUpperIso_LowerZoom.tnr_thr_1.bits.NR3D_MV_PEN_W;
	MINT32 NR3D_MV_PEN_THR_U_L	=  rParam.rUpperIso_LowerZoom.tnr_thr_1.bits.NR3D_MV_PEN_THR;
	MINT32 NR3D_SMALL_SAD_THR_U_L  =  rParam.rUpperIso_LowerZoom.tnr_thr_1.bits.NR3D_SMALL_SAD_THR;
	MINT32 NR3D_Q_BLKY_Y0_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_1.bits.NR3D_Q_BLKY_Y0;
	MINT32 NR3D_Q_BLKY_Y1_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_1.bits.NR3D_Q_BLKY_Y1;
	MINT32 NR3D_Q_BLKY_Y2_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_1.bits.NR3D_Q_BLKY_Y2;
	MINT32 NR3D_Q_BLKY_Y3_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_1.bits.NR3D_Q_BLKY_Y3;
	MINT32 NR3D_Q_BLKY_Y4_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_1.bits.NR3D_Q_BLKY_Y4;
	MINT32 NR3D_Q_BLKY_Y5_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_2.bits.NR3D_Q_BLKY_Y5;
	MINT32 NR3D_Q_BLKY_Y6_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_2.bits.NR3D_Q_BLKY_Y6;
	MINT32 NR3D_Q_BLKY_Y7_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_2.bits.NR3D_Q_BLKY_Y7;
	MINT32 NR3D_Q_BLKC_Y0_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_2.bits.NR3D_Q_BLKC_Y0;
	MINT32 NR3D_Q_BLKC_Y1_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_2.bits.NR3D_Q_BLKC_Y1;
	MINT32 NR3D_Q_BLKC_Y2_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_3.bits.NR3D_Q_BLKC_Y2;
	MINT32 NR3D_Q_BLKC_Y3_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_3.bits.NR3D_Q_BLKC_Y3;
	MINT32 NR3D_Q_BLKC_Y4_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_3.bits.NR3D_Q_BLKC_Y4;
	MINT32 NR3D_Q_BLKC_Y5_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_3.bits.NR3D_Q_BLKC_Y5;
	MINT32 NR3D_Q_BLKC_Y6_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_3.bits.NR3D_Q_BLKC_Y6;
	MINT32 NR3D_Q_BLKC_Y7_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_4.bits.NR3D_Q_BLKC_Y7;
	MINT32 NR3D_Q_DETXTR_LVL_Y0_U_L  =	rParam.rUpperIso_LowerZoom.tnr_curve_4.bits.NR3D_Q_DETXTR_LVL_Y0;
	MINT32 NR3D_Q_DETXTR_LVL_Y1_U_L  =	rParam.rUpperIso_LowerZoom.tnr_curve_4.bits.NR3D_Q_DETXTR_LVL_Y1;
	MINT32 NR3D_Q_DETXTR_LVL_Y2_U_L  =	rParam.rUpperIso_LowerZoom.tnr_curve_4.bits.NR3D_Q_DETXTR_LVL_Y2;
	MINT32 NR3D_Q_DETXTR_LVL_Y3_U_L  =	rParam.rUpperIso_LowerZoom.tnr_curve_4.bits.NR3D_Q_DETXTR_LVL_Y3;
	MINT32 NR3D_Q_DETXTR_LVL_Y4_U_L  =	rParam.rUpperIso_LowerZoom.tnr_curve_5.bits.NR3D_Q_DETXTR_LVL_Y4;
	MINT32 NR3D_Q_DETXTR_LVL_Y5_U_L  =	rParam.rUpperIso_LowerZoom.tnr_curve_5.bits.NR3D_Q_DETXTR_LVL_Y5;
	MINT32 NR3D_Q_DETXTR_LVL_Y6_U_L  =	rParam.rUpperIso_LowerZoom.tnr_curve_5.bits.NR3D_Q_DETXTR_LVL_Y6;
	MINT32 NR3D_Q_DETXTR_LVL_Y7_U_L  =	rParam.rUpperIso_LowerZoom.tnr_curve_5.bits.NR3D_Q_DETXTR_LVL_Y7;
	MINT32 NR3D_Q_DE1_BASE_Y0_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_5.bits.NR3D_Q_DE1_BASE_Y0;
	MINT32 NR3D_Q_DE1_BASE_Y1_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_6.bits.NR3D_Q_DE1_BASE_Y1;
	MINT32 NR3D_Q_DE1_BASE_Y2_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_6.bits.NR3D_Q_DE1_BASE_Y2;
	MINT32 NR3D_Q_DE1_BASE_Y3_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_6.bits.NR3D_Q_DE1_BASE_Y3;
	MINT32 NR3D_Q_DE1_BASE_Y4_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_6.bits.NR3D_Q_DE1_BASE_Y4;
	MINT32 NR3D_Q_DE1_BASE_Y5_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_6.bits.NR3D_Q_DE1_BASE_Y5;
	MINT32 NR3D_Q_DE1_BASE_Y6_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_7.bits.NR3D_Q_DE1_BASE_Y6;
	MINT32 NR3D_Q_DE1_BASE_Y7_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_7.bits.NR3D_Q_DE1_BASE_Y7;
	MINT32 NR3D_Q_DE2TXTR_BASE_Y0_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_7.bits.NR3D_Q_DE2TXTR_BASE_Y0;
	MINT32 NR3D_Q_DE2TXTR_BASE_Y1_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_7.bits.NR3D_Q_DE2TXTR_BASE_Y1;
	MINT32 NR3D_Q_DE2TXTR_BASE_Y2_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_7.bits.NR3D_Q_DE2TXTR_BASE_Y2;
	MINT32 NR3D_Q_DE2TXTR_BASE_Y3_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_8.bits.NR3D_Q_DE2TXTR_BASE_Y3;
	MINT32 NR3D_Q_DE2TXTR_BASE_Y4_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_8.bits.NR3D_Q_DE2TXTR_BASE_Y4;
	MINT32 NR3D_Q_DE2TXTR_BASE_Y5_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_8.bits.NR3D_Q_DE2TXTR_BASE_Y5;
	MINT32 NR3D_Q_DE2TXTR_BASE_Y6_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_8.bits.NR3D_Q_DE2TXTR_BASE_Y6;
	MINT32 NR3D_Q_DE2TXTR_BASE_Y7_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_8.bits.NR3D_Q_DE2TXTR_BASE_Y7;
	MINT32 NR3D_Q_MV_Y0_U_L  =	rParam.rUpperIso_LowerZoom.tnr_curve_9.bits.NR3D_Q_MV_Y0;
	MINT32 NR3D_Q_MV_Y1_U_L  =	rParam.rUpperIso_LowerZoom.tnr_curve_9.bits.NR3D_Q_MV_Y1;
	MINT32 NR3D_Q_MV_Y2_U_L  =	rParam.rUpperIso_LowerZoom.tnr_curve_9.bits.NR3D_Q_MV_Y2;
	MINT32 NR3D_Q_MV_Y3_U_L  =	rParam.rUpperIso_LowerZoom.tnr_curve_9.bits.NR3D_Q_MV_Y3;
	MINT32 NR3D_Q_MV_Y4_U_L  =	rParam.rUpperIso_LowerZoom.tnr_curve_9.bits.NR3D_Q_MV_Y4;
	MINT32 NR3D_Q_MV_Y5_U_L  =	rParam.rUpperIso_LowerZoom.tnr_curve_10.bits.NR3D_Q_MV_Y5;
	MINT32 NR3D_Q_MV_Y6_U_L  =	rParam.rUpperIso_LowerZoom.tnr_curve_10.bits.NR3D_Q_MV_Y6;
	MINT32 NR3D_Q_MV_Y7_U_L  =	rParam.rUpperIso_LowerZoom.tnr_curve_10.bits.NR3D_Q_MV_Y7;
	MINT32 NR3D_Q_WVAR_Y0_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_10.bits.NR3D_Q_WVAR_Y0;
	MINT32 NR3D_Q_WVAR_Y1_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_10.bits.NR3D_Q_WVAR_Y1;
	MINT32 NR3D_Q_WVAR_Y2_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_11.bits.NR3D_Q_WVAR_Y2;
	MINT32 NR3D_Q_WVAR_Y3_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_11.bits.NR3D_Q_WVAR_Y3;
	MINT32 NR3D_Q_WVAR_Y4_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_11.bits.NR3D_Q_WVAR_Y4;
	MINT32 NR3D_Q_WVAR_Y5_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_11.bits.NR3D_Q_WVAR_Y5;
	MINT32 NR3D_Q_WVAR_Y6_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_11.bits.NR3D_Q_WVAR_Y6;
	MINT32 NR3D_Q_WVAR_Y7_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_12.bits.NR3D_Q_WVAR_Y7;
	MINT32 NR3D_Q_WSM_Y0_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_12.bits.NR3D_Q_WSM_Y0;
	MINT32 NR3D_Q_WSM_Y1_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_12.bits.NR3D_Q_WSM_Y1;
	MINT32 NR3D_Q_WSM_Y2_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_12.bits.NR3D_Q_WSM_Y2;
	MINT32 NR3D_Q_WSM_Y3_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_12.bits.NR3D_Q_WSM_Y3;
	MINT32 NR3D_Q_WSM_Y4_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_13.bits.NR3D_Q_WSM_Y4;
	MINT32 NR3D_Q_WSM_Y5_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_13.bits.NR3D_Q_WSM_Y5;
	MINT32 NR3D_Q_WSM_Y6_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_13.bits.NR3D_Q_WSM_Y6;
	MINT32 NR3D_Q_WSM_Y7_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_13.bits.NR3D_Q_WSM_Y7;
	MINT32 NR3D_Q_SDL_Y0_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_14.bits.NR3D_Q_SDL_Y0;
	MINT32 NR3D_Q_SDL_Y1_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_14.bits.NR3D_Q_SDL_Y1;
	MINT32 NR3D_Q_SDL_Y2_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_14.bits.NR3D_Q_SDL_Y2;
	MINT32 NR3D_Q_SDL_Y3_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_14.bits.NR3D_Q_SDL_Y3;
	MINT32 NR3D_Q_SDL_Y4_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_14.bits.NR3D_Q_SDL_Y4;
	MINT32 NR3D_Q_SDL_Y5_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_15.bits.NR3D_Q_SDL_Y5;
	MINT32 NR3D_Q_SDL_Y6_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_15.bits.NR3D_Q_SDL_Y6;
	MINT32 NR3D_Q_SDL_Y7_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_15.bits.NR3D_Q_SDL_Y7;
	MINT32 NR3D_Q_SDL_Y8_U_L  =  rParam.rUpperIso_LowerZoom.tnr_curve_15.bits.NR3D_Q_SDL_Y8;

	//R2C Kib+ & Whitne E2
	MINT32 NR3D_R2CENC_U_L  =  rParam.rUpperIso_LowerZoom.tnr_r2c_1.bits.NR3D_R2CENC;
	MINT32 NR3D_R2C_VAL4_U_L  =  rParam.rUpperIso_LowerZoom.tnr_r2c_1.bits.NR3D_R2C_VAL4;
	MINT32 NR3D_R2C_VAL3_U_L  =  rParam.rUpperIso_LowerZoom.tnr_r2c_1.bits.NR3D_R2C_VAL3;
	MINT32 NR3D_R2C_VAL2_U_L  =  rParam.rUpperIso_LowerZoom.tnr_r2c_1.bits.NR3D_R2C_VAL2;
	MINT32 NR3D_R2C_VAL1_U_L  =  rParam.rUpperIso_LowerZoom.tnr_r2c_1.bits.NR3D_R2C_VAL1;
	MINT32 NR3D_R2C_TXTR_THROFF_U_L  =  rParam.rUpperIso_LowerZoom.tnr_r2c_2.bits.NR3D_R2C_TXTR_THROFF;
	MINT32 NR3D_R2C_TXTR_THR4_U_L  =  rParam.rUpperIso_LowerZoom.tnr_r2c_2.bits.NR3D_R2C_TXTR_THR4;
	MINT32 NR3D_R2C_TXTR_THR3_U_L  =  rParam.rUpperIso_LowerZoom.tnr_r2c_2.bits.NR3D_R2C_TXTR_THR3;
	MINT32 NR3D_R2C_TXTR_THR2_U_L  =  rParam.rUpperIso_LowerZoom.tnr_r2c_2.bits.NR3D_R2C_TXTR_THR2;
	MINT32 NR3D_R2C_TXTR_THR1_U_L  =  rParam.rUpperIso_LowerZoom.tnr_r2c_2.bits.NR3D_R2C_TXTR_THR1;

	// ======================
		// Up Iso Up Zoom
	// ======================

	MINT32 NR3D_SL2_OFF_U_U = rParam.rUpperIso_UpperZoom.on_con.bits.NR3D_SL2_OFF;
	MINT32 NR3D_TNR_Y_EN_U_U = rParam.rUpperIso_UpperZoom.tnr_tnr_enable.bits.NR3D_TNR_Y_EN;
	MINT32 NR3D_TNR_C_EN_U_U = rParam.rUpperIso_UpperZoom.tnr_tnr_enable.bits.NR3D_TNR_C_EN;
	MINT32 NR3D_BLEND_RATIO_BLKY_U_U  =  rParam.rUpperIso_UpperZoom.tnr_flt_config.bits.NR3D_BLEND_RATIO_BLKY;
	MINT32 NR3D_BLEND_RATIO_DE_U_U	=  rParam.rUpperIso_UpperZoom.tnr_flt_config.bits.NR3D_BLEND_RATIO_DE;
	MINT32 NR3D_BLEND_RATIO_TXTR_U_U  =  rParam.rUpperIso_UpperZoom.tnr_flt_config.bits.NR3D_BLEND_RATIO_TXTR;
	MINT32 NR3D_BLEND_RATIO_MV_U_U	=  rParam.rUpperIso_UpperZoom.tnr_flt_config.bits.NR3D_BLEND_RATIO_MV;
	MINT32 NR3D_FLT_STR_MAX_U_U  =	rParam.rUpperIso_UpperZoom.tnr_flt_config.bits.NR3D_FLT_STR_MAX;
	MINT32 NR3D_Q_NL_U_U  =  rParam.rUpperIso_UpperZoom.tnr_fb_info1.bits.NR3D_Q_NL;
	MINT32 NR3D_Q_SP_U_U  =  rParam.rUpperIso_UpperZoom.tnr_fb_info1.bits.NR3D_Q_SP;
	MINT32 NR3D_BDI_THR_U_U  =	rParam.rUpperIso_UpperZoom.tnr_thr_1.bits.NR3D_BDI_THR;
	MINT32 NR3D_MV_PEN_W_U_U  =  rParam.rUpperIso_UpperZoom.tnr_thr_1.bits.NR3D_MV_PEN_W;
	MINT32 NR3D_MV_PEN_THR_U_U	=  rParam.rUpperIso_UpperZoom.tnr_thr_1.bits.NR3D_MV_PEN_THR;
	MINT32 NR3D_SMALL_SAD_THR_U_U  =  rParam.rUpperIso_UpperZoom.tnr_thr_1.bits.NR3D_SMALL_SAD_THR;
	MINT32 NR3D_Q_BLKY_Y0_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_1.bits.NR3D_Q_BLKY_Y0;
	MINT32 NR3D_Q_BLKY_Y1_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_1.bits.NR3D_Q_BLKY_Y1;
	MINT32 NR3D_Q_BLKY_Y2_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_1.bits.NR3D_Q_BLKY_Y2;
	MINT32 NR3D_Q_BLKY_Y3_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_1.bits.NR3D_Q_BLKY_Y3;
	MINT32 NR3D_Q_BLKY_Y4_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_1.bits.NR3D_Q_BLKY_Y4;
	MINT32 NR3D_Q_BLKY_Y5_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_2.bits.NR3D_Q_BLKY_Y5;
	MINT32 NR3D_Q_BLKY_Y6_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_2.bits.NR3D_Q_BLKY_Y6;
	MINT32 NR3D_Q_BLKY_Y7_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_2.bits.NR3D_Q_BLKY_Y7;
	MINT32 NR3D_Q_BLKC_Y0_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_2.bits.NR3D_Q_BLKC_Y0;
	MINT32 NR3D_Q_BLKC_Y1_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_2.bits.NR3D_Q_BLKC_Y1;
	MINT32 NR3D_Q_BLKC_Y2_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_3.bits.NR3D_Q_BLKC_Y2;
	MINT32 NR3D_Q_BLKC_Y3_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_3.bits.NR3D_Q_BLKC_Y3;
	MINT32 NR3D_Q_BLKC_Y4_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_3.bits.NR3D_Q_BLKC_Y4;
	MINT32 NR3D_Q_BLKC_Y5_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_3.bits.NR3D_Q_BLKC_Y5;
	MINT32 NR3D_Q_BLKC_Y6_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_3.bits.NR3D_Q_BLKC_Y6;
	MINT32 NR3D_Q_BLKC_Y7_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_4.bits.NR3D_Q_BLKC_Y7;
	MINT32 NR3D_Q_DETXTR_LVL_Y0_U_U  =	rParam.rUpperIso_UpperZoom.tnr_curve_4.bits.NR3D_Q_DETXTR_LVL_Y0;
	MINT32 NR3D_Q_DETXTR_LVL_Y1_U_U  =	rParam.rUpperIso_UpperZoom.tnr_curve_4.bits.NR3D_Q_DETXTR_LVL_Y1;
	MINT32 NR3D_Q_DETXTR_LVL_Y2_U_U  =	rParam.rUpperIso_UpperZoom.tnr_curve_4.bits.NR3D_Q_DETXTR_LVL_Y2;
	MINT32 NR3D_Q_DETXTR_LVL_Y3_U_U  =	rParam.rUpperIso_UpperZoom.tnr_curve_4.bits.NR3D_Q_DETXTR_LVL_Y3;
	MINT32 NR3D_Q_DETXTR_LVL_Y4_U_U  =	rParam.rUpperIso_UpperZoom.tnr_curve_5.bits.NR3D_Q_DETXTR_LVL_Y4;
	MINT32 NR3D_Q_DETXTR_LVL_Y5_U_U  =	rParam.rUpperIso_UpperZoom.tnr_curve_5.bits.NR3D_Q_DETXTR_LVL_Y5;
	MINT32 NR3D_Q_DETXTR_LVL_Y6_U_U  =	rParam.rUpperIso_UpperZoom.tnr_curve_5.bits.NR3D_Q_DETXTR_LVL_Y6;
	MINT32 NR3D_Q_DETXTR_LVL_Y7_U_U  =	rParam.rUpperIso_UpperZoom.tnr_curve_5.bits.NR3D_Q_DETXTR_LVL_Y7;
	MINT32 NR3D_Q_DE1_BASE_Y0_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_5.bits.NR3D_Q_DE1_BASE_Y0;
	MINT32 NR3D_Q_DE1_BASE_Y1_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_6.bits.NR3D_Q_DE1_BASE_Y1;
	MINT32 NR3D_Q_DE1_BASE_Y2_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_6.bits.NR3D_Q_DE1_BASE_Y2;
	MINT32 NR3D_Q_DE1_BASE_Y3_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_6.bits.NR3D_Q_DE1_BASE_Y3;
	MINT32 NR3D_Q_DE1_BASE_Y4_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_6.bits.NR3D_Q_DE1_BASE_Y4;
	MINT32 NR3D_Q_DE1_BASE_Y5_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_6.bits.NR3D_Q_DE1_BASE_Y5;
	MINT32 NR3D_Q_DE1_BASE_Y6_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_7.bits.NR3D_Q_DE1_BASE_Y6;
	MINT32 NR3D_Q_DE1_BASE_Y7_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_7.bits.NR3D_Q_DE1_BASE_Y7;
	MINT32 NR3D_Q_DE2TXTR_BASE_Y0_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_7.bits.NR3D_Q_DE2TXTR_BASE_Y0;
	MINT32 NR3D_Q_DE2TXTR_BASE_Y1_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_7.bits.NR3D_Q_DE2TXTR_BASE_Y1;
	MINT32 NR3D_Q_DE2TXTR_BASE_Y2_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_7.bits.NR3D_Q_DE2TXTR_BASE_Y2;
	MINT32 NR3D_Q_DE2TXTR_BASE_Y3_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_8.bits.NR3D_Q_DE2TXTR_BASE_Y3;
	MINT32 NR3D_Q_DE2TXTR_BASE_Y4_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_8.bits.NR3D_Q_DE2TXTR_BASE_Y4;
	MINT32 NR3D_Q_DE2TXTR_BASE_Y5_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_8.bits.NR3D_Q_DE2TXTR_BASE_Y5;
	MINT32 NR3D_Q_DE2TXTR_BASE_Y6_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_8.bits.NR3D_Q_DE2TXTR_BASE_Y6;
	MINT32 NR3D_Q_DE2TXTR_BASE_Y7_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_8.bits.NR3D_Q_DE2TXTR_BASE_Y7;
	MINT32 NR3D_Q_MV_Y0_U_U  =	rParam.rUpperIso_UpperZoom.tnr_curve_9.bits.NR3D_Q_MV_Y0;
	MINT32 NR3D_Q_MV_Y1_U_U  =	rParam.rUpperIso_UpperZoom.tnr_curve_9.bits.NR3D_Q_MV_Y1;
	MINT32 NR3D_Q_MV_Y2_U_U  =	rParam.rUpperIso_UpperZoom.tnr_curve_9.bits.NR3D_Q_MV_Y2;
	MINT32 NR3D_Q_MV_Y3_U_U  =	rParam.rUpperIso_UpperZoom.tnr_curve_9.bits.NR3D_Q_MV_Y3;
	MINT32 NR3D_Q_MV_Y4_U_U  =	rParam.rUpperIso_UpperZoom.tnr_curve_9.bits.NR3D_Q_MV_Y4;
	MINT32 NR3D_Q_MV_Y5_U_U  =	rParam.rUpperIso_UpperZoom.tnr_curve_10.bits.NR3D_Q_MV_Y5;
	MINT32 NR3D_Q_MV_Y6_U_U  =	rParam.rUpperIso_UpperZoom.tnr_curve_10.bits.NR3D_Q_MV_Y6;
	MINT32 NR3D_Q_MV_Y7_U_U  =	rParam.rUpperIso_UpperZoom.tnr_curve_10.bits.NR3D_Q_MV_Y7;
	MINT32 NR3D_Q_WVAR_Y0_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_10.bits.NR3D_Q_WVAR_Y0;
	MINT32 NR3D_Q_WVAR_Y1_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_10.bits.NR3D_Q_WVAR_Y1;
	MINT32 NR3D_Q_WVAR_Y2_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_11.bits.NR3D_Q_WVAR_Y2;
	MINT32 NR3D_Q_WVAR_Y3_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_11.bits.NR3D_Q_WVAR_Y3;
	MINT32 NR3D_Q_WVAR_Y4_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_11.bits.NR3D_Q_WVAR_Y4;
	MINT32 NR3D_Q_WVAR_Y5_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_11.bits.NR3D_Q_WVAR_Y5;
	MINT32 NR3D_Q_WVAR_Y6_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_11.bits.NR3D_Q_WVAR_Y6;
	MINT32 NR3D_Q_WVAR_Y7_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_12.bits.NR3D_Q_WVAR_Y7;
	MINT32 NR3D_Q_WSM_Y0_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_12.bits.NR3D_Q_WSM_Y0;
	MINT32 NR3D_Q_WSM_Y1_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_12.bits.NR3D_Q_WSM_Y1;
	MINT32 NR3D_Q_WSM_Y2_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_12.bits.NR3D_Q_WSM_Y2;
	MINT32 NR3D_Q_WSM_Y3_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_12.bits.NR3D_Q_WSM_Y3;
	MINT32 NR3D_Q_WSM_Y4_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_13.bits.NR3D_Q_WSM_Y4;
	MINT32 NR3D_Q_WSM_Y5_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_13.bits.NR3D_Q_WSM_Y5;
	MINT32 NR3D_Q_WSM_Y6_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_13.bits.NR3D_Q_WSM_Y6;
	MINT32 NR3D_Q_WSM_Y7_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_13.bits.NR3D_Q_WSM_Y7;
	MINT32 NR3D_Q_SDL_Y0_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_14.bits.NR3D_Q_SDL_Y0;
	MINT32 NR3D_Q_SDL_Y1_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_14.bits.NR3D_Q_SDL_Y1;
	MINT32 NR3D_Q_SDL_Y2_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_14.bits.NR3D_Q_SDL_Y2;
	MINT32 NR3D_Q_SDL_Y3_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_14.bits.NR3D_Q_SDL_Y3;
	MINT32 NR3D_Q_SDL_Y4_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_14.bits.NR3D_Q_SDL_Y4;
	MINT32 NR3D_Q_SDL_Y5_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_15.bits.NR3D_Q_SDL_Y5;
	MINT32 NR3D_Q_SDL_Y6_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_15.bits.NR3D_Q_SDL_Y6;
	MINT32 NR3D_Q_SDL_Y7_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_15.bits.NR3D_Q_SDL_Y7;
	MINT32 NR3D_Q_SDL_Y8_U_U  =  rParam.rUpperIso_UpperZoom.tnr_curve_15.bits.NR3D_Q_SDL_Y8;

	//R2C Kib+ & Whitne E2
	MINT32 NR3D_R2CENC_U_U  =  rParam.rUpperIso_UpperZoom.tnr_r2c_1.bits.NR3D_R2CENC;
	MINT32 NR3D_R2C_VAL4_U_U  =  rParam.rUpperIso_UpperZoom.tnr_r2c_1.bits.NR3D_R2C_VAL4;
	MINT32 NR3D_R2C_VAL3_U_U  =  rParam.rUpperIso_UpperZoom.tnr_r2c_1.bits.NR3D_R2C_VAL3;
	MINT32 NR3D_R2C_VAL2_U_U  =  rParam.rUpperIso_UpperZoom.tnr_r2c_1.bits.NR3D_R2C_VAL2;
	MINT32 NR3D_R2C_VAL1_U_U  =  rParam.rUpperIso_UpperZoom.tnr_r2c_1.bits.NR3D_R2C_VAL1;
	MINT32 NR3D_R2C_TXTR_THROFF_U_U  =  rParam.rUpperIso_UpperZoom.tnr_r2c_2.bits.NR3D_R2C_TXTR_THROFF;
	MINT32 NR3D_R2C_TXTR_THR4_U_U  =  rParam.rUpperIso_UpperZoom.tnr_r2c_2.bits.NR3D_R2C_TXTR_THR4;
	MINT32 NR3D_R2C_TXTR_THR3_U_U  =  rParam.rUpperIso_UpperZoom.tnr_r2c_2.bits.NR3D_R2C_TXTR_THR3;
	MINT32 NR3D_R2C_TXTR_THR2_U_U  =  rParam.rUpperIso_UpperZoom.tnr_r2c_2.bits.NR3D_R2C_TXTR_THR2;
	MINT32 NR3D_R2C_TXTR_THR1_U_U  =  rParam.rUpperIso_UpperZoom.tnr_r2c_2.bits.NR3D_R2C_TXTR_THR1;




	// ======================
	// interpolation section

	 MINT32 NR3D_SL2_OFF			= InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_SL2_OFF_L_L, NR3D_SL2_OFF_L_U, NR3D_SL2_OFF_U_L, NR3D_SL2_OFF_U_U,1);
	 MINT32 NR3D_TNR_Y_EN			= InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_TNR_Y_EN_L_L, NR3D_TNR_Y_EN_L_U, NR3D_TNR_Y_EN_U_L, NR3D_TNR_Y_EN_U_U,1);
	 MINT32 NR3D_TNR_C_EN			= InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_TNR_C_EN_L_L, NR3D_TNR_C_EN_L_U, NR3D_TNR_C_EN_U_L, NR3D_TNR_C_EN_U_U,1);
	 MINT32 NR3D_BLEND_RATIO_BLKY			 = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_BLEND_RATIO_BLKY_L_L, NR3D_BLEND_RATIO_BLKY_L_U, NR3D_BLEND_RATIO_BLKY_U_L, NR3D_BLEND_RATIO_BLKY_U_U,0);
	 MINT32 NR3D_BLEND_RATIO_DE 		   = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_BLEND_RATIO_DE_L_L, NR3D_BLEND_RATIO_DE_L_U, NR3D_BLEND_RATIO_DE_U_L, NR3D_BLEND_RATIO_DE_U_U,0);
	 MINT32 NR3D_BLEND_RATIO_TXTR			 = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_BLEND_RATIO_TXTR_L_L, NR3D_BLEND_RATIO_TXTR_L_U, NR3D_BLEND_RATIO_TXTR_U_L, NR3D_BLEND_RATIO_TXTR_U_U,0);
	 MINT32 NR3D_BLEND_RATIO_MV 		   = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_BLEND_RATIO_MV_L_L, NR3D_BLEND_RATIO_MV_L_U, NR3D_BLEND_RATIO_MV_U_L, NR3D_BLEND_RATIO_MV_U_U,0);
	 MINT32 NR3D_FLT_STR_MAX			= InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_FLT_STR_MAX_L_L, NR3D_FLT_STR_MAX_L_U, NR3D_FLT_STR_MAX_U_L, NR3D_FLT_STR_MAX_U_U,0);
	 MINT32 NR3D_Q_NL			 = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_NL_L_L, NR3D_Q_NL_L_U, NR3D_Q_NL_U_L, NR3D_Q_NL_U_U,0);
	 MINT32 NR3D_Q_SP			 = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_SP_L_L, NR3D_Q_SP_L_U, NR3D_Q_SP_U_L, NR3D_Q_SP_U_U,0);
	 MINT32 NR3D_BDI_THR			= InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_BDI_THR_L_L, NR3D_BDI_THR_L_U, NR3D_BDI_THR_U_L, NR3D_BDI_THR_U_U,0);
	 MINT32 NR3D_MV_PEN_W			 = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_MV_PEN_W_L_L, NR3D_MV_PEN_W_L_U, NR3D_MV_PEN_W_U_L, NR3D_MV_PEN_W_U_U,0);
	 MINT32 NR3D_MV_PEN_THR 		   = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_MV_PEN_THR_L_L, NR3D_MV_PEN_THR_L_U, NR3D_MV_PEN_THR_U_L, NR3D_MV_PEN_THR_U_U,0);
	 MINT32 NR3D_SMALL_SAD_THR			  = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_SMALL_SAD_THR_L_L, NR3D_SMALL_SAD_THR_L_U, NR3D_SMALL_SAD_THR_U_L, NR3D_SMALL_SAD_THR_U_U,0);
	 MINT32 NR3D_Q_BLKY_Y0			  = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_BLKY_Y0_L_L, NR3D_Q_BLKY_Y0_L_U, NR3D_Q_BLKY_Y0_U_L, NR3D_Q_BLKY_Y0_U_U,0);
	 MINT32 NR3D_Q_BLKY_Y1			  = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_BLKY_Y1_L_L, NR3D_Q_BLKY_Y1_L_U, NR3D_Q_BLKY_Y1_U_L, NR3D_Q_BLKY_Y1_U_U,0);
	 MINT32 NR3D_Q_BLKY_Y2			  = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_BLKY_Y2_L_L, NR3D_Q_BLKY_Y2_L_U, NR3D_Q_BLKY_Y2_U_L, NR3D_Q_BLKY_Y2_U_U,0);
	 MINT32 NR3D_Q_BLKY_Y3			  = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_BLKY_Y3_L_L, NR3D_Q_BLKY_Y3_L_U, NR3D_Q_BLKY_Y3_U_L, NR3D_Q_BLKY_Y3_U_U,0);
	 MINT32 NR3D_Q_BLKY_Y4			  = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_BLKY_Y4_L_L, NR3D_Q_BLKY_Y4_L_U, NR3D_Q_BLKY_Y4_U_L, NR3D_Q_BLKY_Y4_U_U,0);
	 MINT32 NR3D_Q_BLKY_Y5			  = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_BLKY_Y5_L_L, NR3D_Q_BLKY_Y5_L_U, NR3D_Q_BLKY_Y5_U_L, NR3D_Q_BLKY_Y5_U_U,0);
	 MINT32 NR3D_Q_BLKY_Y6			  = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_BLKY_Y6_L_L, NR3D_Q_BLKY_Y6_L_U, NR3D_Q_BLKY_Y6_U_L, NR3D_Q_BLKY_Y6_U_U,0);
	 MINT32 NR3D_Q_BLKY_Y7			  = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_BLKY_Y7_L_L, NR3D_Q_BLKY_Y7_L_U, NR3D_Q_BLKY_Y7_U_L, NR3D_Q_BLKY_Y7_U_U,0);
	 MINT32 NR3D_Q_BLKC_Y0			  = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_BLKC_Y0_L_L, NR3D_Q_BLKC_Y0_L_U, NR3D_Q_BLKC_Y0_U_L, NR3D_Q_BLKC_Y0_U_U,0);
	 MINT32 NR3D_Q_BLKC_Y1			  = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_BLKC_Y1_L_L, NR3D_Q_BLKC_Y1_L_U, NR3D_Q_BLKC_Y1_U_L, NR3D_Q_BLKC_Y1_U_U,0);
	 MINT32 NR3D_Q_BLKC_Y2			  = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_BLKC_Y2_L_L, NR3D_Q_BLKC_Y2_L_U, NR3D_Q_BLKC_Y2_U_L, NR3D_Q_BLKC_Y2_U_U,0);
	 MINT32 NR3D_Q_BLKC_Y3			  = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_BLKC_Y3_L_L, NR3D_Q_BLKC_Y3_L_U, NR3D_Q_BLKC_Y3_U_L, NR3D_Q_BLKC_Y3_U_U,0);
	 MINT32 NR3D_Q_BLKC_Y4			  = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_BLKC_Y4_L_L, NR3D_Q_BLKC_Y4_L_U, NR3D_Q_BLKC_Y4_U_L, NR3D_Q_BLKC_Y4_U_U,0);
	 MINT32 NR3D_Q_BLKC_Y5			  = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_BLKC_Y5_L_L, NR3D_Q_BLKC_Y5_L_U, NR3D_Q_BLKC_Y5_U_L, NR3D_Q_BLKC_Y5_U_U,0);
	 MINT32 NR3D_Q_BLKC_Y6			  = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_BLKC_Y6_L_L, NR3D_Q_BLKC_Y6_L_U, NR3D_Q_BLKC_Y6_U_L, NR3D_Q_BLKC_Y6_U_U,0);
	 MINT32 NR3D_Q_BLKC_Y7			  = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_BLKC_Y7_L_L, NR3D_Q_BLKC_Y7_L_U, NR3D_Q_BLKC_Y7_U_L, NR3D_Q_BLKC_Y7_U_U,0);
	 MINT32 NR3D_Q_DETXTR_LVL_Y0			= InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_DETXTR_LVL_Y0_L_L, NR3D_Q_DETXTR_LVL_Y0_L_U, NR3D_Q_DETXTR_LVL_Y0_U_L, NR3D_Q_DETXTR_LVL_Y0_U_U,0);
	 MINT32 NR3D_Q_DETXTR_LVL_Y1			= InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_DETXTR_LVL_Y1_L_L, NR3D_Q_DETXTR_LVL_Y1_L_U, NR3D_Q_DETXTR_LVL_Y1_U_L, NR3D_Q_DETXTR_LVL_Y1_U_U,0);
	 MINT32 NR3D_Q_DETXTR_LVL_Y2			= InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_DETXTR_LVL_Y2_L_L, NR3D_Q_DETXTR_LVL_Y2_L_U, NR3D_Q_DETXTR_LVL_Y2_U_L, NR3D_Q_DETXTR_LVL_Y2_U_U,0);
	 MINT32 NR3D_Q_DETXTR_LVL_Y3			= InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_DETXTR_LVL_Y3_L_L, NR3D_Q_DETXTR_LVL_Y3_L_U, NR3D_Q_DETXTR_LVL_Y3_U_L, NR3D_Q_DETXTR_LVL_Y3_U_U,0);
	 MINT32 NR3D_Q_DETXTR_LVL_Y4			= InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_DETXTR_LVL_Y4_L_L, NR3D_Q_DETXTR_LVL_Y4_L_U, NR3D_Q_DETXTR_LVL_Y4_U_L, NR3D_Q_DETXTR_LVL_Y4_U_U,0);
	 MINT32 NR3D_Q_DETXTR_LVL_Y5			= InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_DETXTR_LVL_Y5_L_L, NR3D_Q_DETXTR_LVL_Y5_L_U, NR3D_Q_DETXTR_LVL_Y5_U_L, NR3D_Q_DETXTR_LVL_Y5_U_U,0);
	 MINT32 NR3D_Q_DETXTR_LVL_Y6			= InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_DETXTR_LVL_Y6_L_L, NR3D_Q_DETXTR_LVL_Y6_L_U, NR3D_Q_DETXTR_LVL_Y6_U_L, NR3D_Q_DETXTR_LVL_Y6_U_U,0);
	 MINT32 NR3D_Q_DETXTR_LVL_Y7			= InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_DETXTR_LVL_Y7_L_L, NR3D_Q_DETXTR_LVL_Y7_L_U, NR3D_Q_DETXTR_LVL_Y7_U_L, NR3D_Q_DETXTR_LVL_Y7_U_U,0);
	 MINT32 NR3D_Q_DE1_BASE_Y0			  = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_DE1_BASE_Y0_L_L, NR3D_Q_DE1_BASE_Y0_L_U, NR3D_Q_DE1_BASE_Y0_U_L, NR3D_Q_DE1_BASE_Y0_U_U,0);
	 MINT32 NR3D_Q_DE1_BASE_Y1			  = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_DE1_BASE_Y1_L_L, NR3D_Q_DE1_BASE_Y1_L_U, NR3D_Q_DE1_BASE_Y1_U_L, NR3D_Q_DE1_BASE_Y1_U_U,0);
	 MINT32 NR3D_Q_DE1_BASE_Y2			  = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_DE1_BASE_Y2_L_L, NR3D_Q_DE1_BASE_Y2_L_U, NR3D_Q_DE1_BASE_Y2_U_L, NR3D_Q_DE1_BASE_Y2_U_U,0);
	 MINT32 NR3D_Q_DE1_BASE_Y3			  = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_DE1_BASE_Y3_L_L, NR3D_Q_DE1_BASE_Y3_L_U, NR3D_Q_DE1_BASE_Y3_U_L, NR3D_Q_DE1_BASE_Y3_U_U,0);
	 MINT32 NR3D_Q_DE1_BASE_Y4			  = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_DE1_BASE_Y4_L_L, NR3D_Q_DE1_BASE_Y4_L_U, NR3D_Q_DE1_BASE_Y4_U_L, NR3D_Q_DE1_BASE_Y4_U_U,0);
	 MINT32 NR3D_Q_DE1_BASE_Y5			  = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_DE1_BASE_Y5_L_L, NR3D_Q_DE1_BASE_Y5_L_U, NR3D_Q_DE1_BASE_Y5_U_L, NR3D_Q_DE1_BASE_Y5_U_U,0);
	 MINT32 NR3D_Q_DE1_BASE_Y6			  = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_DE1_BASE_Y6_L_L, NR3D_Q_DE1_BASE_Y6_L_U, NR3D_Q_DE1_BASE_Y6_U_L, NR3D_Q_DE1_BASE_Y6_U_U,0);
	 MINT32 NR3D_Q_DE1_BASE_Y7			  = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_DE1_BASE_Y7_L_L, NR3D_Q_DE1_BASE_Y7_L_U, NR3D_Q_DE1_BASE_Y7_U_L, NR3D_Q_DE1_BASE_Y7_U_U,0);
	 MINT32 NR3D_Q_DE2TXTR_BASE_Y0			  = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_DE2TXTR_BASE_Y0_L_L, NR3D_Q_DE2TXTR_BASE_Y0_L_U, NR3D_Q_DE2TXTR_BASE_Y0_U_L, NR3D_Q_DE2TXTR_BASE_Y0_U_U,0);
	 MINT32 NR3D_Q_DE2TXTR_BASE_Y1			  = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_DE2TXTR_BASE_Y1_L_L, NR3D_Q_DE2TXTR_BASE_Y1_L_U, NR3D_Q_DE2TXTR_BASE_Y1_U_L, NR3D_Q_DE2TXTR_BASE_Y1_U_U,0);
	 MINT32 NR3D_Q_DE2TXTR_BASE_Y2			  = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_DE2TXTR_BASE_Y2_L_L, NR3D_Q_DE2TXTR_BASE_Y2_L_U, NR3D_Q_DE2TXTR_BASE_Y2_U_L, NR3D_Q_DE2TXTR_BASE_Y2_U_U,0);
	 MINT32 NR3D_Q_DE2TXTR_BASE_Y3			  = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_DE2TXTR_BASE_Y3_L_L, NR3D_Q_DE2TXTR_BASE_Y3_L_U, NR3D_Q_DE2TXTR_BASE_Y3_U_L, NR3D_Q_DE2TXTR_BASE_Y3_U_U,0);
	 MINT32 NR3D_Q_DE2TXTR_BASE_Y4			  = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_DE2TXTR_BASE_Y4_L_L, NR3D_Q_DE2TXTR_BASE_Y4_L_U, NR3D_Q_DE2TXTR_BASE_Y4_U_L, NR3D_Q_DE2TXTR_BASE_Y4_U_U,0);
	 MINT32 NR3D_Q_DE2TXTR_BASE_Y5			  = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_DE2TXTR_BASE_Y5_L_L, NR3D_Q_DE2TXTR_BASE_Y5_L_U, NR3D_Q_DE2TXTR_BASE_Y5_U_L, NR3D_Q_DE2TXTR_BASE_Y5_U_U,0);
	 MINT32 NR3D_Q_DE2TXTR_BASE_Y6			  = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_DE2TXTR_BASE_Y6_L_L, NR3D_Q_DE2TXTR_BASE_Y6_L_U, NR3D_Q_DE2TXTR_BASE_Y6_U_L, NR3D_Q_DE2TXTR_BASE_Y6_U_U,0);
	 MINT32 NR3D_Q_DE2TXTR_BASE_Y7			  = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_DE2TXTR_BASE_Y7_L_L, NR3D_Q_DE2TXTR_BASE_Y7_L_U, NR3D_Q_DE2TXTR_BASE_Y7_U_L, NR3D_Q_DE2TXTR_BASE_Y7_U_U,0);
	 MINT32 NR3D_Q_MV_Y0			= InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_MV_Y0_L_L, NR3D_Q_MV_Y0_L_U, NR3D_Q_MV_Y0_U_L, NR3D_Q_MV_Y0_U_U,0);
	 MINT32 NR3D_Q_MV_Y1			= InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_MV_Y1_L_L, NR3D_Q_MV_Y1_L_U, NR3D_Q_MV_Y1_U_L, NR3D_Q_MV_Y1_U_U,0);
	 MINT32 NR3D_Q_MV_Y2			= InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_MV_Y2_L_L, NR3D_Q_MV_Y2_L_U, NR3D_Q_MV_Y2_U_L, NR3D_Q_MV_Y2_U_U,0);
	 MINT32 NR3D_Q_MV_Y3			= InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_MV_Y3_L_L, NR3D_Q_MV_Y3_L_U, NR3D_Q_MV_Y3_U_L, NR3D_Q_MV_Y3_U_U,0);
	 MINT32 NR3D_Q_MV_Y4			= InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_MV_Y4_L_L, NR3D_Q_MV_Y4_L_U, NR3D_Q_MV_Y4_U_L, NR3D_Q_MV_Y4_U_U,0);
	 MINT32 NR3D_Q_MV_Y5			= InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_MV_Y5_L_L, NR3D_Q_MV_Y5_L_U, NR3D_Q_MV_Y5_U_L, NR3D_Q_MV_Y5_U_U,0);
	 MINT32 NR3D_Q_MV_Y6			= InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_MV_Y6_L_L, NR3D_Q_MV_Y6_L_U, NR3D_Q_MV_Y6_U_L, NR3D_Q_MV_Y6_U_U,0);
	 MINT32 NR3D_Q_MV_Y7			= InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_MV_Y7_L_L, NR3D_Q_MV_Y7_L_U, NR3D_Q_MV_Y7_U_L, NR3D_Q_MV_Y7_U_U,0);
	 MINT32 NR3D_Q_WVAR_Y0			  = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_WVAR_Y0_L_L, NR3D_Q_WVAR_Y0_L_U, NR3D_Q_WVAR_Y0_U_L, NR3D_Q_WVAR_Y0_U_U,0);
	 MINT32 NR3D_Q_WVAR_Y1			  = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_WVAR_Y1_L_L, NR3D_Q_WVAR_Y1_L_U, NR3D_Q_WVAR_Y1_U_L, NR3D_Q_WVAR_Y1_U_U,0);
	 MINT32 NR3D_Q_WVAR_Y2			  = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_WVAR_Y2_L_L, NR3D_Q_WVAR_Y2_L_U, NR3D_Q_WVAR_Y2_U_L, NR3D_Q_WVAR_Y2_U_U,0);
	 MINT32 NR3D_Q_WVAR_Y3			  = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_WVAR_Y3_L_L, NR3D_Q_WVAR_Y3_L_U, NR3D_Q_WVAR_Y3_U_L, NR3D_Q_WVAR_Y3_U_U,0);
	 MINT32 NR3D_Q_WVAR_Y4			  = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_WVAR_Y4_L_L, NR3D_Q_WVAR_Y4_L_U, NR3D_Q_WVAR_Y4_U_L, NR3D_Q_WVAR_Y4_U_U,0);
	 MINT32 NR3D_Q_WVAR_Y5			  = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_WVAR_Y5_L_L, NR3D_Q_WVAR_Y5_L_U, NR3D_Q_WVAR_Y5_U_L, NR3D_Q_WVAR_Y5_U_U,0);
	 MINT32 NR3D_Q_WVAR_Y6			  = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_WVAR_Y6_L_L, NR3D_Q_WVAR_Y6_L_U, NR3D_Q_WVAR_Y6_U_L, NR3D_Q_WVAR_Y6_U_U,0);
	 MINT32 NR3D_Q_WVAR_Y7			  = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_WVAR_Y7_L_L, NR3D_Q_WVAR_Y7_L_U, NR3D_Q_WVAR_Y7_U_L, NR3D_Q_WVAR_Y7_U_U,0);
	 MINT32 NR3D_Q_WSM_Y0			 = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_WSM_Y0_L_L, NR3D_Q_WSM_Y0_L_U, NR3D_Q_WSM_Y0_U_L, NR3D_Q_WSM_Y0_U_U,0);
	 MINT32 NR3D_Q_WSM_Y1			 = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_WSM_Y1_L_L, NR3D_Q_WSM_Y1_L_U, NR3D_Q_WSM_Y1_U_L, NR3D_Q_WSM_Y1_U_U,0);
	 MINT32 NR3D_Q_WSM_Y2			 = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_WSM_Y2_L_L, NR3D_Q_WSM_Y2_L_U, NR3D_Q_WSM_Y2_U_L, NR3D_Q_WSM_Y2_U_U,0);
	 MINT32 NR3D_Q_WSM_Y3			 = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_WSM_Y3_L_L, NR3D_Q_WSM_Y3_L_U, NR3D_Q_WSM_Y3_U_L, NR3D_Q_WSM_Y3_U_U,0);
	 MINT32 NR3D_Q_WSM_Y4			 = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_WSM_Y4_L_L, NR3D_Q_WSM_Y4_L_U, NR3D_Q_WSM_Y4_U_L, NR3D_Q_WSM_Y4_U_U,0);
	 MINT32 NR3D_Q_WSM_Y5			 = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_WSM_Y5_L_L, NR3D_Q_WSM_Y5_L_U, NR3D_Q_WSM_Y5_U_L, NR3D_Q_WSM_Y5_U_U,0);
	 MINT32 NR3D_Q_WSM_Y6			 = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_WSM_Y6_L_L, NR3D_Q_WSM_Y6_L_U, NR3D_Q_WSM_Y6_U_L, NR3D_Q_WSM_Y6_U_U,0);
	 MINT32 NR3D_Q_WSM_Y7			 = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_WSM_Y7_L_L, NR3D_Q_WSM_Y7_L_U, NR3D_Q_WSM_Y7_U_L, NR3D_Q_WSM_Y7_U_U,0);
	 MINT32 NR3D_Q_SDL_Y0			 = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_SDL_Y0_L_L, NR3D_Q_SDL_Y0_L_U, NR3D_Q_SDL_Y0_U_L, NR3D_Q_SDL_Y0_U_U,0);
	 MINT32 NR3D_Q_SDL_Y1			 = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_SDL_Y1_L_L, NR3D_Q_SDL_Y1_L_U, NR3D_Q_SDL_Y1_U_L, NR3D_Q_SDL_Y1_U_U,0);
	 MINT32 NR3D_Q_SDL_Y2			 = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_SDL_Y2_L_L, NR3D_Q_SDL_Y2_L_U, NR3D_Q_SDL_Y2_U_L, NR3D_Q_SDL_Y2_U_U,0);
	 MINT32 NR3D_Q_SDL_Y3			 = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_SDL_Y3_L_L, NR3D_Q_SDL_Y3_L_U, NR3D_Q_SDL_Y3_U_L, NR3D_Q_SDL_Y3_U_U,0);
	 MINT32 NR3D_Q_SDL_Y4			 = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_SDL_Y4_L_L, NR3D_Q_SDL_Y4_L_U, NR3D_Q_SDL_Y4_U_L, NR3D_Q_SDL_Y4_U_U,0);
	 MINT32 NR3D_Q_SDL_Y5			 = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_SDL_Y5_L_L, NR3D_Q_SDL_Y5_L_U, NR3D_Q_SDL_Y5_U_L, NR3D_Q_SDL_Y5_U_U,0);
	 MINT32 NR3D_Q_SDL_Y6			 = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_SDL_Y6_L_L, NR3D_Q_SDL_Y6_L_U, NR3D_Q_SDL_Y6_U_L, NR3D_Q_SDL_Y6_U_U,0);
	 MINT32 NR3D_Q_SDL_Y7			 = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_SDL_Y7_L_L, NR3D_Q_SDL_Y7_L_U, NR3D_Q_SDL_Y7_U_L, NR3D_Q_SDL_Y7_U_U,0);
	 MINT32 NR3D_Q_SDL_Y8			 = InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_Q_SDL_Y8_L_L, NR3D_Q_SDL_Y8_L_U, NR3D_Q_SDL_Y8_U_L, NR3D_Q_SDL_Y8_U_U,0);

    //R2C Kib+ & Whitne E2
	MINT32 NR3D_R2CENC  =  InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_R2CENC_L_L, NR3D_R2CENC_L_U, NR3D_R2CENC_U_L, NR3D_R2CENC_U_U,0);
	MINT32 NR3D_R2C_VAL4  =  InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_R2C_VAL4_L_L, NR3D_R2C_VAL4_L_U, NR3D_R2C_VAL4_U_L, NR3D_R2C_VAL4_U_U,0);
	MINT32 NR3D_R2C_VAL3  =  InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_R2C_VAL3_L_L, NR3D_R2C_VAL3_L_U, NR3D_R2C_VAL3_U_L, NR3D_R2C_VAL3_U_U,0);
	MINT32 NR3D_R2C_VAL2  =  InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_R2C_VAL2_L_L, NR3D_R2C_VAL2_L_U, NR3D_R2C_VAL2_U_L, NR3D_R2C_VAL2_U_U,0);
	MINT32 NR3D_R2C_VAL1  =  InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_R2C_VAL1_L_L, NR3D_R2C_VAL1_L_U, NR3D_R2C_VAL1_U_L, NR3D_R2C_VAL1_U_U,0);
	MINT32 NR3D_R2C_TXTR_THROFF  =  InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_R2C_TXTR_THROFF_L_L, NR3D_R2C_TXTR_THROFF_L_U, NR3D_R2C_TXTR_THROFF_U_L, NR3D_R2C_TXTR_THROFF_U_U,0);
	MINT32 NR3D_R2C_TXTR_THR4  =  InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_R2C_TXTR_THR4_L_L, NR3D_R2C_TXTR_THR4_L_U, NR3D_R2C_TXTR_THR4_U_L, NR3D_R2C_TXTR_THR4_U_U,0);
	MINT32 NR3D_R2C_TXTR_THR3  =  InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_R2C_TXTR_THR3_L_L, NR3D_R2C_TXTR_THR3_L_U, NR3D_R2C_TXTR_THR3_U_L, NR3D_R2C_TXTR_THR3_U_U,0);
	MINT32 NR3D_R2C_TXTR_THR2  =  InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_R2C_TXTR_THR2_L_L, NR3D_R2C_TXTR_THR2_L_U, NR3D_R2C_TXTR_THR2_U_L, NR3D_R2C_TXTR_THR2_U_U,0);
	MINT32 NR3D_R2C_TXTR_THR1  =  InterParam_NR3D(RATIO_L_L, RATIO_L_U, RATIO_U_L, RATIO_U_U, NR3D_R2C_TXTR_THR1_L_L, NR3D_R2C_TXTR_THR1_L_U, NR3D_R2C_TXTR_THR1_U_L, NR3D_R2C_TXTR_THR1_U_U,0);


	// =============================
	// refine NR3D strength

	// =============================


    rSmoothNR3D.on_con.bits.NR3D_SL2_OFF = NR3D_SL2_OFF;
	rSmoothNR3D.tnr_tnr_enable.bits.NR3D_TNR_Y_EN = NR3D_TNR_Y_EN;
	rSmoothNR3D.tnr_tnr_enable.bits.NR3D_TNR_C_EN = NR3D_TNR_C_EN;
	rSmoothNR3D.tnr_flt_config.bits.NR3D_BLEND_RATIO_BLKY = NR3D_BLEND_RATIO_BLKY;
	rSmoothNR3D.tnr_flt_config.bits.NR3D_BLEND_RATIO_DE = NR3D_BLEND_RATIO_DE;
	rSmoothNR3D.tnr_flt_config.bits.NR3D_BLEND_RATIO_TXTR = NR3D_BLEND_RATIO_TXTR;
	rSmoothNR3D.tnr_flt_config.bits.NR3D_BLEND_RATIO_MV = NR3D_BLEND_RATIO_MV;
	rSmoothNR3D.tnr_flt_config.bits.NR3D_FLT_STR_MAX = NR3D_FLT_STR_MAX;
	rSmoothNR3D.tnr_fb_info1.bits.NR3D_Q_NL = NR3D_Q_NL;
	rSmoothNR3D.tnr_fb_info1.bits.NR3D_Q_SP = NR3D_Q_SP;
	rSmoothNR3D.tnr_thr_1.bits.NR3D_BDI_THR = NR3D_BDI_THR;
	rSmoothNR3D.tnr_thr_1.bits.NR3D_MV_PEN_W = NR3D_MV_PEN_W;
	rSmoothNR3D.tnr_thr_1.bits.NR3D_MV_PEN_THR = NR3D_MV_PEN_THR;
	rSmoothNR3D.tnr_thr_1.bits.NR3D_SMALL_SAD_THR = NR3D_SMALL_SAD_THR;
	rSmoothNR3D.tnr_curve_1.bits.NR3D_Q_BLKY_Y0 = NR3D_Q_BLKY_Y0;
	rSmoothNR3D.tnr_curve_1.bits.NR3D_Q_BLKY_Y1 = NR3D_Q_BLKY_Y1;
	rSmoothNR3D.tnr_curve_1.bits.NR3D_Q_BLKY_Y2 = NR3D_Q_BLKY_Y2;
	rSmoothNR3D.tnr_curve_1.bits.NR3D_Q_BLKY_Y3 = NR3D_Q_BLKY_Y3;
	rSmoothNR3D.tnr_curve_1.bits.NR3D_Q_BLKY_Y4 = NR3D_Q_BLKY_Y4;
	rSmoothNR3D.tnr_curve_2.bits.NR3D_Q_BLKY_Y5 = NR3D_Q_BLKY_Y5;
	rSmoothNR3D.tnr_curve_2.bits.NR3D_Q_BLKY_Y6 = NR3D_Q_BLKY_Y6;
	rSmoothNR3D.tnr_curve_2.bits.NR3D_Q_BLKY_Y7 = NR3D_Q_BLKY_Y7;
	rSmoothNR3D.tnr_curve_2.bits.NR3D_Q_BLKC_Y0 = NR3D_Q_BLKC_Y0;
	rSmoothNR3D.tnr_curve_2.bits.NR3D_Q_BLKC_Y1 = NR3D_Q_BLKC_Y1;
	rSmoothNR3D.tnr_curve_3.bits.NR3D_Q_BLKC_Y2 = NR3D_Q_BLKC_Y2;
	rSmoothNR3D.tnr_curve_3.bits.NR3D_Q_BLKC_Y3 = NR3D_Q_BLKC_Y3;
	rSmoothNR3D.tnr_curve_3.bits.NR3D_Q_BLKC_Y4 = NR3D_Q_BLKC_Y4;
	rSmoothNR3D.tnr_curve_3.bits.NR3D_Q_BLKC_Y5 = NR3D_Q_BLKC_Y5;
	rSmoothNR3D.tnr_curve_3.bits.NR3D_Q_BLKC_Y6 = NR3D_Q_BLKC_Y6;
	rSmoothNR3D.tnr_curve_4.bits.NR3D_Q_BLKC_Y7 = NR3D_Q_BLKC_Y7;
	rSmoothNR3D.tnr_curve_4.bits.NR3D_Q_DETXTR_LVL_Y0 = NR3D_Q_DETXTR_LVL_Y0;
	rSmoothNR3D.tnr_curve_4.bits.NR3D_Q_DETXTR_LVL_Y1 = NR3D_Q_DETXTR_LVL_Y1;
	rSmoothNR3D.tnr_curve_4.bits.NR3D_Q_DETXTR_LVL_Y2 = NR3D_Q_DETXTR_LVL_Y2;
	rSmoothNR3D.tnr_curve_4.bits.NR3D_Q_DETXTR_LVL_Y3 = NR3D_Q_DETXTR_LVL_Y3;
	rSmoothNR3D.tnr_curve_5.bits.NR3D_Q_DETXTR_LVL_Y4 = NR3D_Q_DETXTR_LVL_Y4;
	rSmoothNR3D.tnr_curve_5.bits.NR3D_Q_DETXTR_LVL_Y5 = NR3D_Q_DETXTR_LVL_Y5;
	rSmoothNR3D.tnr_curve_5.bits.NR3D_Q_DETXTR_LVL_Y6 = NR3D_Q_DETXTR_LVL_Y6;
	rSmoothNR3D.tnr_curve_5.bits.NR3D_Q_DETXTR_LVL_Y7 = NR3D_Q_DETXTR_LVL_Y7;
	rSmoothNR3D.tnr_curve_5.bits.NR3D_Q_DE1_BASE_Y0 = NR3D_Q_DE1_BASE_Y0;
	rSmoothNR3D.tnr_curve_6.bits.NR3D_Q_DE1_BASE_Y1 = NR3D_Q_DE1_BASE_Y1;
	rSmoothNR3D.tnr_curve_6.bits.NR3D_Q_DE1_BASE_Y2 = NR3D_Q_DE1_BASE_Y2;
	rSmoothNR3D.tnr_curve_6.bits.NR3D_Q_DE1_BASE_Y3 = NR3D_Q_DE1_BASE_Y3;
	rSmoothNR3D.tnr_curve_6.bits.NR3D_Q_DE1_BASE_Y4 = NR3D_Q_DE1_BASE_Y4;
	rSmoothNR3D.tnr_curve_6.bits.NR3D_Q_DE1_BASE_Y5 = NR3D_Q_DE1_BASE_Y5;
	rSmoothNR3D.tnr_curve_7.bits.NR3D_Q_DE1_BASE_Y6 = NR3D_Q_DE1_BASE_Y6;
	rSmoothNR3D.tnr_curve_7.bits.NR3D_Q_DE1_BASE_Y7 = NR3D_Q_DE1_BASE_Y7;
	rSmoothNR3D.tnr_curve_7.bits.NR3D_Q_DE2TXTR_BASE_Y0 = NR3D_Q_DE2TXTR_BASE_Y0;
	rSmoothNR3D.tnr_curve_7.bits.NR3D_Q_DE2TXTR_BASE_Y1 = NR3D_Q_DE2TXTR_BASE_Y1;
	rSmoothNR3D.tnr_curve_7.bits.NR3D_Q_DE2TXTR_BASE_Y2 = NR3D_Q_DE2TXTR_BASE_Y2;
	rSmoothNR3D.tnr_curve_8.bits.NR3D_Q_DE2TXTR_BASE_Y3 = NR3D_Q_DE2TXTR_BASE_Y3;
	rSmoothNR3D.tnr_curve_8.bits.NR3D_Q_DE2TXTR_BASE_Y4 = NR3D_Q_DE2TXTR_BASE_Y4;
	rSmoothNR3D.tnr_curve_8.bits.NR3D_Q_DE2TXTR_BASE_Y5 = NR3D_Q_DE2TXTR_BASE_Y5;
	rSmoothNR3D.tnr_curve_8.bits.NR3D_Q_DE2TXTR_BASE_Y6 = NR3D_Q_DE2TXTR_BASE_Y6;
	rSmoothNR3D.tnr_curve_8.bits.NR3D_Q_DE2TXTR_BASE_Y7 = NR3D_Q_DE2TXTR_BASE_Y7;
	rSmoothNR3D.tnr_curve_9.bits.NR3D_Q_MV_Y0 = NR3D_Q_MV_Y0;
	rSmoothNR3D.tnr_curve_9.bits.NR3D_Q_MV_Y1 = NR3D_Q_MV_Y1;
	rSmoothNR3D.tnr_curve_9.bits.NR3D_Q_MV_Y2 = NR3D_Q_MV_Y2;
	rSmoothNR3D.tnr_curve_9.bits.NR3D_Q_MV_Y3 = NR3D_Q_MV_Y3;
	rSmoothNR3D.tnr_curve_9.bits.NR3D_Q_MV_Y4 = NR3D_Q_MV_Y4;
	rSmoothNR3D.tnr_curve_10.bits.NR3D_Q_MV_Y5 = NR3D_Q_MV_Y5;
	rSmoothNR3D.tnr_curve_10.bits.NR3D_Q_MV_Y6 = NR3D_Q_MV_Y6;
	rSmoothNR3D.tnr_curve_10.bits.NR3D_Q_MV_Y7 = NR3D_Q_MV_Y7;
	rSmoothNR3D.tnr_curve_10.bits.NR3D_Q_WVAR_Y0 = NR3D_Q_WVAR_Y0;
	rSmoothNR3D.tnr_curve_10.bits.NR3D_Q_WVAR_Y1 = NR3D_Q_WVAR_Y1;
	rSmoothNR3D.tnr_curve_11.bits.NR3D_Q_WVAR_Y2 = NR3D_Q_WVAR_Y2;
	rSmoothNR3D.tnr_curve_11.bits.NR3D_Q_WVAR_Y3 = NR3D_Q_WVAR_Y3;
	rSmoothNR3D.tnr_curve_11.bits.NR3D_Q_WVAR_Y4 = NR3D_Q_WVAR_Y4;
	rSmoothNR3D.tnr_curve_11.bits.NR3D_Q_WVAR_Y5 = NR3D_Q_WVAR_Y5;
	rSmoothNR3D.tnr_curve_11.bits.NR3D_Q_WVAR_Y6 = NR3D_Q_WVAR_Y6;
	rSmoothNR3D.tnr_curve_12.bits.NR3D_Q_WVAR_Y7 = NR3D_Q_WVAR_Y7;
	rSmoothNR3D.tnr_curve_12.bits.NR3D_Q_WSM_Y0 = NR3D_Q_WSM_Y0;
	rSmoothNR3D.tnr_curve_12.bits.NR3D_Q_WSM_Y1 = NR3D_Q_WSM_Y1;
	rSmoothNR3D.tnr_curve_12.bits.NR3D_Q_WSM_Y2 = NR3D_Q_WSM_Y2;
	rSmoothNR3D.tnr_curve_12.bits.NR3D_Q_WSM_Y3 = NR3D_Q_WSM_Y3;
	rSmoothNR3D.tnr_curve_13.bits.NR3D_Q_WSM_Y4 = NR3D_Q_WSM_Y4;
	rSmoothNR3D.tnr_curve_13.bits.NR3D_Q_WSM_Y5 = NR3D_Q_WSM_Y5;
	rSmoothNR3D.tnr_curve_13.bits.NR3D_Q_WSM_Y6 = NR3D_Q_WSM_Y6;
	rSmoothNR3D.tnr_curve_13.bits.NR3D_Q_WSM_Y7 = NR3D_Q_WSM_Y7;
	rSmoothNR3D.tnr_curve_14.bits.NR3D_Q_SDL_Y0 = NR3D_Q_SDL_Y0;
	rSmoothNR3D.tnr_curve_14.bits.NR3D_Q_SDL_Y1 = NR3D_Q_SDL_Y1;
	rSmoothNR3D.tnr_curve_14.bits.NR3D_Q_SDL_Y2 = NR3D_Q_SDL_Y2;
	rSmoothNR3D.tnr_curve_14.bits.NR3D_Q_SDL_Y3 = NR3D_Q_SDL_Y3;
	rSmoothNR3D.tnr_curve_14.bits.NR3D_Q_SDL_Y4 = NR3D_Q_SDL_Y4;
	rSmoothNR3D.tnr_curve_15.bits.NR3D_Q_SDL_Y5 = NR3D_Q_SDL_Y5;
	rSmoothNR3D.tnr_curve_15.bits.NR3D_Q_SDL_Y6 = NR3D_Q_SDL_Y6;
	rSmoothNR3D.tnr_curve_15.bits.NR3D_Q_SDL_Y7 = NR3D_Q_SDL_Y7;
	rSmoothNR3D.tnr_curve_15.bits.NR3D_Q_SDL_Y8 = NR3D_Q_SDL_Y8;

	//R2C Kib+ & Whitne E2
	rSmoothNR3D.tnr_r2c_1.bits.NR3D_R2CENC = NR3D_R2CENC;
	rSmoothNR3D.tnr_r2c_1.bits.NR3D_R2C_VAL4 = NR3D_R2C_VAL4;
	rSmoothNR3D.tnr_r2c_1.bits.NR3D_R2C_VAL3 = NR3D_R2C_VAL3;
	rSmoothNR3D.tnr_r2c_1.bits.NR3D_R2C_VAL2 = NR3D_R2C_VAL2;
	rSmoothNR3D.tnr_r2c_1.bits.NR3D_R2C_VAL1 = NR3D_R2C_VAL1;
	rSmoothNR3D.tnr_r2c_2.bits.NR3D_R2C_TXTR_THROFF = NR3D_R2C_TXTR_THROFF;
	rSmoothNR3D.tnr_r2c_2.bits.NR3D_R2C_TXTR_THR4 = NR3D_R2C_TXTR_THR4;
	rSmoothNR3D.tnr_r2c_2.bits.NR3D_R2C_TXTR_THR3 = NR3D_R2C_TXTR_THR3;
	rSmoothNR3D.tnr_r2c_2.bits.NR3D_R2C_TXTR_THR2 = NR3D_R2C_TXTR_THR2;
	rSmoothNR3D.tnr_r2c_2.bits.NR3D_R2C_TXTR_THR1 = NR3D_R2C_TXTR_THR1;


#define NR3D_DUMP(item1, item2)   \
		do{ 					 \
			INTER_LOG_IF(bSmoothNR3DDebug, "[S, L_L, L_U, U_L, U_U]%22s = %d, %d, %d, %d, %d", #item2, rSmoothNR3D.item1.bits.item2, rParam.rLowerIso_LowerZoom.item1.bits.item2, rParam.rLowerIso_UpperZoom.item1.bits.item2, rParam.rUpperIso_LowerZoom.item1.bits.item2, rParam.rUpperIso_UpperZoom.item1.bits.item2); \
		}while(0)

	NR3D_DUMP(on_con, NR3D_SL2_OFF);
	NR3D_DUMP(tnr_tnr_enable, NR3D_TNR_Y_EN);
	NR3D_DUMP(tnr_tnr_enable, NR3D_TNR_C_EN);
	NR3D_DUMP(tnr_flt_config, NR3D_BLEND_RATIO_BLKY);
	NR3D_DUMP(tnr_flt_config, NR3D_BLEND_RATIO_DE);
	NR3D_DUMP(tnr_flt_config, NR3D_BLEND_RATIO_TXTR);
	NR3D_DUMP(tnr_flt_config, NR3D_BLEND_RATIO_MV);
	NR3D_DUMP(tnr_flt_config, NR3D_FLT_STR_MAX);
	NR3D_DUMP(tnr_fb_info1, NR3D_Q_NL);
	NR3D_DUMP(tnr_fb_info1, NR3D_Q_SP);
	NR3D_DUMP(tnr_thr_1, NR3D_BDI_THR);
	NR3D_DUMP(tnr_thr_1, NR3D_MV_PEN_W);
	NR3D_DUMP(tnr_thr_1, NR3D_MV_PEN_THR);
	NR3D_DUMP(tnr_thr_1, NR3D_SMALL_SAD_THR);
	NR3D_DUMP(tnr_curve_1, NR3D_Q_BLKY_Y0);
	NR3D_DUMP(tnr_curve_1, NR3D_Q_BLKY_Y1);
	NR3D_DUMP(tnr_curve_1, NR3D_Q_BLKY_Y2);
	NR3D_DUMP(tnr_curve_1, NR3D_Q_BLKY_Y3);
	NR3D_DUMP(tnr_curve_1, NR3D_Q_BLKY_Y4);
	NR3D_DUMP(tnr_curve_2, NR3D_Q_BLKY_Y5);
	NR3D_DUMP(tnr_curve_2, NR3D_Q_BLKY_Y6);
	NR3D_DUMP(tnr_curve_2, NR3D_Q_BLKY_Y7);
	NR3D_DUMP(tnr_curve_2, NR3D_Q_BLKC_Y0);
	NR3D_DUMP(tnr_curve_2, NR3D_Q_BLKC_Y1);
	NR3D_DUMP(tnr_curve_3, NR3D_Q_BLKC_Y2);
	NR3D_DUMP(tnr_curve_3, NR3D_Q_BLKC_Y3);
	NR3D_DUMP(tnr_curve_3, NR3D_Q_BLKC_Y4);
	NR3D_DUMP(tnr_curve_3, NR3D_Q_BLKC_Y5);
	NR3D_DUMP(tnr_curve_3, NR3D_Q_BLKC_Y6);
	NR3D_DUMP(tnr_curve_4, NR3D_Q_BLKC_Y7);
	NR3D_DUMP(tnr_curve_4, NR3D_Q_DETXTR_LVL_Y0);
	NR3D_DUMP(tnr_curve_4, NR3D_Q_DETXTR_LVL_Y1);
	NR3D_DUMP(tnr_curve_4, NR3D_Q_DETXTR_LVL_Y2);
	NR3D_DUMP(tnr_curve_4, NR3D_Q_DETXTR_LVL_Y3);
	NR3D_DUMP(tnr_curve_5, NR3D_Q_DETXTR_LVL_Y4);
	NR3D_DUMP(tnr_curve_5, NR3D_Q_DETXTR_LVL_Y5);
	NR3D_DUMP(tnr_curve_5, NR3D_Q_DETXTR_LVL_Y6);
	NR3D_DUMP(tnr_curve_5, NR3D_Q_DETXTR_LVL_Y7);
	NR3D_DUMP(tnr_curve_5, NR3D_Q_DE1_BASE_Y0);
	NR3D_DUMP(tnr_curve_6, NR3D_Q_DE1_BASE_Y1);
	NR3D_DUMP(tnr_curve_6, NR3D_Q_DE1_BASE_Y2);
	NR3D_DUMP(tnr_curve_6, NR3D_Q_DE1_BASE_Y3);
	NR3D_DUMP(tnr_curve_6, NR3D_Q_DE1_BASE_Y4);
	NR3D_DUMP(tnr_curve_6, NR3D_Q_DE1_BASE_Y5);
	NR3D_DUMP(tnr_curve_7, NR3D_Q_DE1_BASE_Y6);
	NR3D_DUMP(tnr_curve_7, NR3D_Q_DE1_BASE_Y7);
	NR3D_DUMP(tnr_curve_7, NR3D_Q_DE2TXTR_BASE_Y0);
	NR3D_DUMP(tnr_curve_7, NR3D_Q_DE2TXTR_BASE_Y1);
	NR3D_DUMP(tnr_curve_7, NR3D_Q_DE2TXTR_BASE_Y2);
	NR3D_DUMP(tnr_curve_8, NR3D_Q_DE2TXTR_BASE_Y3);
	NR3D_DUMP(tnr_curve_8, NR3D_Q_DE2TXTR_BASE_Y4);
	NR3D_DUMP(tnr_curve_8, NR3D_Q_DE2TXTR_BASE_Y5);
	NR3D_DUMP(tnr_curve_8, NR3D_Q_DE2TXTR_BASE_Y6);
	NR3D_DUMP(tnr_curve_8, NR3D_Q_DE2TXTR_BASE_Y7);
	NR3D_DUMP(tnr_curve_9, NR3D_Q_MV_Y0);
	NR3D_DUMP(tnr_curve_9, NR3D_Q_MV_Y1);
	NR3D_DUMP(tnr_curve_9, NR3D_Q_MV_Y2);
	NR3D_DUMP(tnr_curve_9, NR3D_Q_MV_Y3);
	NR3D_DUMP(tnr_curve_9, NR3D_Q_MV_Y4);
	NR3D_DUMP(tnr_curve_10, NR3D_Q_MV_Y5);
	NR3D_DUMP(tnr_curve_10, NR3D_Q_MV_Y6);
	NR3D_DUMP(tnr_curve_10, NR3D_Q_MV_Y7);
	NR3D_DUMP(tnr_curve_10, NR3D_Q_WVAR_Y0);
	NR3D_DUMP(tnr_curve_10, NR3D_Q_WVAR_Y1);
	NR3D_DUMP(tnr_curve_11, NR3D_Q_WVAR_Y2);
	NR3D_DUMP(tnr_curve_11, NR3D_Q_WVAR_Y3);
	NR3D_DUMP(tnr_curve_11, NR3D_Q_WVAR_Y4);
	NR3D_DUMP(tnr_curve_11, NR3D_Q_WVAR_Y5);
	NR3D_DUMP(tnr_curve_11, NR3D_Q_WVAR_Y6);
	NR3D_DUMP(tnr_curve_12, NR3D_Q_WVAR_Y7);
	NR3D_DUMP(tnr_curve_12, NR3D_Q_WSM_Y0);
	NR3D_DUMP(tnr_curve_12, NR3D_Q_WSM_Y1);
	NR3D_DUMP(tnr_curve_12, NR3D_Q_WSM_Y2);
	NR3D_DUMP(tnr_curve_12, NR3D_Q_WSM_Y3);
	NR3D_DUMP(tnr_curve_13, NR3D_Q_WSM_Y4);
	NR3D_DUMP(tnr_curve_13, NR3D_Q_WSM_Y5);
	NR3D_DUMP(tnr_curve_13, NR3D_Q_WSM_Y6);
	NR3D_DUMP(tnr_curve_13, NR3D_Q_WSM_Y7);
	NR3D_DUMP(tnr_curve_14, NR3D_Q_SDL_Y0);
	NR3D_DUMP(tnr_curve_14, NR3D_Q_SDL_Y1);
	NR3D_DUMP(tnr_curve_14, NR3D_Q_SDL_Y2);
	NR3D_DUMP(tnr_curve_14, NR3D_Q_SDL_Y3);
	NR3D_DUMP(tnr_curve_14, NR3D_Q_SDL_Y4);
	NR3D_DUMP(tnr_curve_15, NR3D_Q_SDL_Y5);
	NR3D_DUMP(tnr_curve_15, NR3D_Q_SDL_Y6);
	NR3D_DUMP(tnr_curve_15, NR3D_Q_SDL_Y7);
	NR3D_DUMP(tnr_curve_15, NR3D_Q_SDL_Y8);

	//R2C Kib+ & Whitne E2
	NR3D_DUMP(tnr_r2c_1, NR3D_R2CENC);
	NR3D_DUMP(tnr_r2c_1, NR3D_R2C_VAL4);
	NR3D_DUMP(tnr_r2c_1, NR3D_R2C_VAL3);
	NR3D_DUMP(tnr_r2c_1, NR3D_R2C_VAL2);
	NR3D_DUMP(tnr_r2c_1, NR3D_R2C_VAL1);
	NR3D_DUMP(tnr_r2c_2, NR3D_R2C_TXTR_THROFF);
	NR3D_DUMP(tnr_r2c_2, NR3D_R2C_TXTR_THR4);
	NR3D_DUMP(tnr_r2c_2, NR3D_R2C_TXTR_THR3);
	NR3D_DUMP(tnr_r2c_2, NR3D_R2C_TXTR_THR2);
	NR3D_DUMP(tnr_r2c_2, NR3D_R2C_TXTR_THR1);

}

// ===========================================================================================

MINT32 InterParam_SWNR(MINT32 ISO, MINT32 ISOpre, MINT32 ISOpos, MINT32 PARAMpre, MINT32 PARAMpos, MINT32 Method) {
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


MVOID SmoothNR(MUINT32 u4RealISO,  // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                NR_PARAM const& rUpper, // settings for upper ISO
                NR_PARAM const& rLower,   // settings for lower ISO
                NR_PARAM& rSmooth) // Output
{
#define Inter(item, method)                                                         \
    do{                                                                               \
        rSmooth.item = InterParam_SWNR(                                                 \
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
    Inter(ANR_ACT_BLD_BASE_C     , 0);
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
    MBOOL bSmoothDebug = interpPropertyGet("debug.smooth_swnr.enable", MFALSE);
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
        DumpParam(ANR_ACT_BLD_BASE_C);
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
        rSmooth.item = InterParam_SWNR(                                                 \
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
	Inter(NR_NTRL_TH_1_Y   , 0);
	Inter(NR_NTRL_TH_2_Y   , 0);
	Inter(NR_NTRL_TH_1_UV   , 0);
	Inter(NR_NTRL_TH_2_UV   , 0);
#undef Inter
    MBOOL bSmoothDebug = interpPropertyGet("debug.smooth_swnr.enable", MFALSE);
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
		DumpParam(NR_NTRL_TH_1_Y);
		DumpParam(NR_NTRL_TH_2_Y);
		DumpParam(NR_NTRL_TH_1_UV);
		DumpParam(NR_NTRL_TH_2_UV);
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
        rSmooth.item = InterParam_SWNR(                                                 \
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
    MBOOL bSmoothDebug = interpPropertyGet("debug.smooth_swnr.enable", MFALSE);
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
        rSmooth.item = InterParam_SWNR(                                                 \
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
    MBOOL bSmoothDebug = interpPropertyGet("debug.smooth_swnr.enable", MFALSE);
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
