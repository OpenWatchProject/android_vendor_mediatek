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

// "return value":    Interpolated register value
// "ISO":            ISO value of current image
// "ISOpre":        A pre-defined ISO (100, 200, 400, ...) who is the cloeset but smaller than or equal to "ISO"
// "ISOpos":        A pre-defined ISO (100, 200, 400, ...) who is the cloeset but larger than or equal to "ISO"
// "PARAMpre":        Corresponding register value from "ISOpre"
// "PARAMpos":        Corresponding register value from "ISOpos"
// "Method:        0 for linear interpolation, 1 for closest one

static MBOOL s_bPerFramePropEnable = MFALSE;

MVOID interpSetPerFramePropEnable(MBOOL bEnable)
{
    s_bPerFramePropEnable = bEnable;
}

static MBOOL interpPropertyGet(const char * pProperty, MBOOL bDefault)
{
    return (s_bPerFramePropEnable) ? (::property_get_bool(pProperty, bDefault)) : (bDefault);
}
MINT32 InterParam_NR1(MINT32 ISO, MINT32 ISOpre, MINT32 ISOpos, MINT32 PARAMpre, MINT32 PARAMpos, MINT32 Method) {
    MINT32 InterPARAM = 0;

    if(ISOpre == ISOpos) {
        // To prevent someone sets different PARAMpre and PARAMpos
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


MVOID SmoothNR1(MUINT32 u4RealISO, // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_BNR_NR1_T const& rUpperNR1, // NR1 settings for upper ISO
                ISP_NVRAM_BNR_NR1_T const& rLowerNR1, // NR1 settings for lower ISO
                ISP_NVRAM_BNR_NR1_T& rSmoothNR1)   // Output
{

	MBOOL bSmoothNR1Debug = interpPropertyGet("vendor.debug.smooth_nr1.enable", MFALSE);
    MY_LOG_IF(bSmoothNR1Debug,"[%s()] u4RealISO: %d, u4UpperISO = %d, u4LowerISO = %d\n", __FUNCTION__, u4RealISO, u4UpperISO, u4LowerISO);


        MINT32 CT_ENpre = rLowerNR1.con.bits.NR1_CT_EN;
        MINT32 CT_MBNDpre = rLowerNR1.ct_con.bits.NR1_MBND;
        MINT32 CT_MDpre = rLowerNR1.ct_con.bits.NR1_CT_MD;
        MINT32 CT_MD2pre = rLowerNR1.ct_con.bits.NR1_CT_MD2;
        MINT32 CT_THRDpre = rLowerNR1.ct_con.bits.NR1_CT_THRD;
        MINT32 CT_DIVpre = rLowerNR1.ct_con.bits.NR1_CT_DIV;
        MINT32 CT_SLOPEpre = rLowerNR1.ct_con.bits.NR1_CT_SLOPE;

        MINT32 CT_ENpos = rUpperNR1.con.bits.NR1_CT_EN;
        MINT32 CT_MBNDpos = rUpperNR1.ct_con.bits.NR1_MBND;
        MINT32 CT_MDpos = rUpperNR1.ct_con.bits.NR1_CT_MD;
        MINT32 CT_MD2pos = rUpperNR1.ct_con.bits.NR1_CT_MD2;
        MINT32 CT_THRDpos = rUpperNR1.ct_con.bits.NR1_CT_THRD;
        MINT32 CT_DIVpos = rUpperNR1.ct_con.bits.NR1_CT_DIV;
        MINT32 CT_SLOPEpos = rUpperNR1.ct_con.bits.NR1_CT_SLOPE;


    // interpolation //
    MINT32 ISO = u4RealISO;
    MINT32 ISOpre = u4LowerISO;
    MINT32 ISOpos = u4UpperISO;

     MINT32 CT_EN = InterParam_NR1(ISO, ISOpre, ISOpos, CT_ENpre, CT_ENpos, 0);
    MINT32 CT_MBND = InterParam_NR1(ISO, ISOpre, ISOpos, CT_MBNDpre, CT_MBNDpos, 0);
    MINT32 CT_MD = InterParam_NR1(ISO, ISOpre, ISOpos, CT_MDpre, CT_MDpos, 0);
    MINT32 CT_MD2 = InterParam_NR1(ISO, ISOpre, ISOpos, CT_MD2pre, CT_MD2pos, 0);
    MINT32 CT_THRD = InterParam_NR1(ISO, ISOpre, ISOpos, CT_THRDpre, CT_THRDpos, 0);
    MINT32 CT_DIV = InterParam_NR1(ISO, ISOpre, ISOpos, CT_DIVpre, CT_DIVpos, 0);
    MINT32 CT_SLOPE = InterParam_NR1(ISO, ISOpre, ISOpos, CT_SLOPEpre,CT_SLOPEpos,0);

    rSmoothNR1.con.bits.NR1_CT_EN = CT_EN;
    rSmoothNR1.ct_con.bits.NR1_MBND = CT_MBND;
    rSmoothNR1.ct_con.bits.NR1_CT_MD = CT_MD;
    rSmoothNR1.ct_con.bits.NR1_CT_MD2 = CT_MD2;
    rSmoothNR1.ct_con.bits.NR1_CT_THRD = CT_THRD;
    rSmoothNR1.ct_con.bits.NR1_CT_DIV = CT_DIV;
    rSmoothNR1.ct_con.bits.NR1_CT_SLOPE = CT_SLOPE;

    MY_LOG_IF(bSmoothNR1Debug,"[L,S,U]CT_EN = %d, %d, %d", rLowerNR1.con.bits.NR1_CT_EN, rSmoothNR1.con.bits.NR1_CT_EN, rUpperNR1.con.bits.NR1_CT_EN  );
    MY_LOG_IF(bSmoothNR1Debug,"[L,S,U]CT_MBND = %d, %d, %d", rLowerNR1.ct_con.bits.NR1_MBND, rSmoothNR1.ct_con.bits.NR1_MBND, rUpperNR1.ct_con.bits.NR1_MBND  );
    MY_LOG_IF(bSmoothNR1Debug,"[L,S,U]CT_MD = %d, %d, %d", rLowerNR1.ct_con.bits.NR1_CT_MD, rSmoothNR1.ct_con.bits.NR1_CT_MD, rUpperNR1.ct_con.bits.NR1_CT_MD  );
    MY_LOG_IF(bSmoothNR1Debug,"[L,S,U]CT_MD2 = %d, %d, %d", rLowerNR1.ct_con.bits.NR1_CT_MD2, rSmoothNR1.ct_con.bits.NR1_CT_MD2, rUpperNR1.ct_con.bits.NR1_CT_MD2  );
    MY_LOG_IF(bSmoothNR1Debug,"[L,S,U]CT_THRD = %d, %d, %d", rLowerNR1.ct_con.bits.NR1_CT_THRD, rSmoothNR1.ct_con.bits.NR1_CT_THRD, rUpperNR1.ct_con.bits.NR1_CT_THRD  );
    MY_LOG_IF(bSmoothNR1Debug,"[L,S,U]CT_DIV = %d, %d, %d", rLowerNR1.ct_con.bits.NR1_CT_DIV, rSmoothNR1.ct_con.bits.NR1_CT_DIV, rUpperNR1.ct_con.bits.NR1_CT_DIV  );
    MY_LOG_IF(bSmoothNR1Debug,"[L,S,U]CT_SLOPE = %d, %d, %d", rLowerNR1.ct_con.bits.NR1_CT_SLOPE, rSmoothNR1.ct_con.bits.NR1_CT_SLOPE, rUpperNR1.ct_con.bits.NR1_CT_SLOPE  );
}

MINT32 InterParam_BPC(MINT32 ISO, MINT32 ISOpre, MINT32 ISOpos, MINT32 PARAMpre, MINT32 PARAMpos, MINT32 Method) {
    MINT32 InterPARAM = 0;

    if(ISOpre == ISOpos) {
        // To prevent someone sets different PARAMpre and PARAMpos
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

MVOID SmoothBPC(MUINT32 u4RealISO, // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_BNR_BPC_T const& rUpperBPC, // NR1 settings for upper ISO
                ISP_NVRAM_BNR_BPC_T const& rLowerBPC, // NR1 settings for lower ISO
                ISP_NVRAM_BNR_BPC_T& rSmoothBPC)   // Output
{
	MBOOL bSmoothBPCDebug = interpPropertyGet("vendor.debug.smooth_bpc.enable", MFALSE);
    MY_LOG_IF(bSmoothBPCDebug,"[%s()] u4RealISO: %d, u4UpperISO = %d, u4LowerISO = %d\n", __FUNCTION__, u4RealISO, u4UpperISO, u4LowerISO);

    MINT32 BPC_ENpre = rLowerBPC.con.bits.BPC_EN;
    MINT32 BPC_LUT_ENpre = rLowerBPC.con.bits.BPC_LUT_EN;
    MINT32 BPC_DTC_MODEpre = rLowerBPC.con.bits.BPC_DTC_MODE;
    MINT32 BPC_CRC_MODEpre = rLowerBPC.con.bits.BPC_CRC_MODE;
    MINT32 BPC_RNGpre = rLowerBPC.dtc.bits.BPC_RNG;
    MINT32 BPC_DIR_THpre = rLowerBPC.cor.bits.BPC_DIR_TH;
    MINT32 BPC_DIR_TH2pre = rLowerBPC.cor.bits.BPC_DIR_TH2;
    MINT32 BPC_DIR_MAXpre = rLowerBPC.cor.bits.BPC_DIR_MAX;
    MINT32 BPC_TH_LWBpre = rLowerBPC.th1.bits.BPC_TH_LWB;
    MINT32 BPC_TH_SLApre = rLowerBPC.th3.bits.BPC_TH_SLA;
    MINT32 BPC_DK_TH_SLApre = rLowerBPC.th4.bits.BPC_DK_TH_SLA;
    MINT32 BPC_TH_SLBpre = rLowerBPC.th3.bits.BPC_TH_SLB;
    MINT32 BPC_DK_TH_SLBpre = rLowerBPC.th4.bits.BPC_DK_TH_SLB;
    MINT32 BPC_TH_XApre = rLowerBPC.th3.bits.BPC_TH_XA;
    MINT32 BPC_DK_TH_XApre = rLowerBPC.th4.bits.BPC_DK_TH_XA;
    MINT32 BPC_TH_XBpre = rLowerBPC.th3.bits.BPC_TH_XB;
    MINT32 BPC_DK_TH_XBpre = rLowerBPC.th4.bits.BPC_DK_TH_XB;
    MINT32 BPC_TH_Ypre = rLowerBPC.th1.bits.BPC_TH_Y;
    MINT32 BPC_TH_UPBpre = rLowerBPC.th2.bits. BPC_TH_UPB;
    MINT32 BPC_C_TH_LWBpre = rLowerBPC.th1_c.bits.BPC_C_TH_LWB;
    MINT32 BPC_C_TH_SLApre = rLowerBPC.th3_c.bits.BPC_C_TH_SLA;
    MINT32 BPC_C_TH_SLBpre = rLowerBPC.th3_c.bits.BPC_C_TH_SLB;
    MINT32 BPC_C_TH_XApre = rLowerBPC.th3_c.bits.BPC_C_TH_XA;
    MINT32 BPC_C_TH_XBpre = rLowerBPC.th3_c.bits.BPC_C_TH_XB;
    MINT32 BPC_C_TH_Ypre = rLowerBPC.th1_c.bits.BPC_C_TH_Y;
    MINT32 BPC_C_TH_UPBpre = rLowerBPC.th2_c.bits.BPC_C_TH_UPB;
    MINT32 BPC_NO_LVpre = rLowerBPC.dtc.bits.BPC_NO_LV;
    MINT32 BPC_CT_LVpre = rLowerBPC.dtc.bits.BPC_CT_LV;
    MINT32 BPC_TH_MULpre = rLowerBPC.dtc.bits.BPC_TH_MUL;
    MINT32 BPC_CS_MODEpre = rLowerBPC.con.bits.BPC_CS_MODE;
    MINT32 BPC_CS_RNGpre = rLowerBPC.dtc.bits.BPC_CS_RNG;
    MINT32 BPC_TABLE_END_MODEpre = rLowerBPC.con.bits.BPC_TABLE_END_MODE;
    MINT32 BPC_AVG_MODEpre = rLowerBPC.con.bits.BPC_AVG_MODE;
    MINT32 BNR_LE_INV_CTLpre = rLowerBPC.con.bits.BNR_LE_INV_CTL;
    MINT32 BNR_OSC_COUNTpre = rLowerBPC.con.bits.BNR_OSC_COUNT;
    MINT32 BNR_EDGEpre = rLowerBPC.con.bits.BNR_EDGE;
    MINT32 BPC_EXCpre = rLowerBPC.con.bits.BPC_EXC;
    MINT32 BPC_RANK_IDXRpre = rLowerBPC.cor.bits.BPC_RANK_IDXR;
    MINT32 BPC_RANK_IDXGpre = rLowerBPC.cor.bits.BPC_RANK_IDXG;
    MINT32 BPC_RANK_IDXBpre = rLowerBPC.th2_c.bits.BPC_RANK_IDXB;
    MINT32 BPC_BLD_MODEpre = rLowerBPC.con.bits.BPC_BLD_MODE;
    MINT32 BPC_BLD_LWBpre = rLowerBPC.th2_c.bits.BPC_BLD_LWB;
    MINT32 BPC_BLD0pre = rLowerBPC.th2.bits.BPC_BLD0;
    MINT32 BPC_BLD1pre = rLowerBPC.th2.bits.BPC_BLD1;
    MINT32 BPC_BLD_SLP0pre = rLowerBPC.th1.bits.BPC_BLD_SLP0;

    MINT32 BPC_ENpos = rUpperBPC.con.bits.BPC_EN;
    MINT32 BPC_LUT_ENpos = rUpperBPC.con.bits.BPC_LUT_EN;
    MINT32 BPC_DTC_MODEpos = rUpperBPC.con.bits.BPC_DTC_MODE;
    MINT32 BPC_CRC_MODEpos = rUpperBPC.con.bits.BPC_CRC_MODE;
    MINT32 BPC_RNGpos = rUpperBPC.dtc.bits.BPC_RNG;
    MINT32 BPC_DIR_THpos = rUpperBPC.cor.bits.BPC_DIR_TH;
    MINT32 BPC_DIR_TH2pos = rUpperBPC.cor.bits.BPC_DIR_TH2;
    MINT32 BPC_DIR_MAXpos = rUpperBPC.cor.bits.BPC_DIR_MAX;
    MINT32 BPC_TH_LWBpos = rUpperBPC.th1.bits.BPC_TH_LWB;
    MINT32 BPC_TH_SLApos = rUpperBPC.th3.bits.BPC_TH_SLA;
    MINT32 BPC_DK_TH_SLApos = rUpperBPC.th4.bits.BPC_DK_TH_SLA;
    MINT32 BPC_TH_SLBpos = rUpperBPC.th3.bits.BPC_TH_SLB;
    MINT32 BPC_DK_TH_SLBpos = rUpperBPC.th4.bits.BPC_DK_TH_SLB;
    MINT32 BPC_TH_XApos = rUpperBPC.th3.bits.BPC_TH_XA;
    MINT32 BPC_DK_TH_XApos = rUpperBPC.th4.bits.BPC_DK_TH_XA;
    MINT32 BPC_TH_XBpos = rUpperBPC.th3.bits.BPC_TH_XB;
    MINT32 BPC_DK_TH_XBpos = rUpperBPC.th4.bits.BPC_DK_TH_XB;
    MINT32 BPC_TH_Ypos = rUpperBPC.th1.bits.BPC_TH_Y;
    MINT32 BPC_TH_UPBpos = rUpperBPC.th2.bits. BPC_TH_UPB;
    MINT32 BPC_C_TH_LWBpos = rUpperBPC.th1_c.bits.BPC_C_TH_LWB;
    MINT32 BPC_C_TH_SLApos = rUpperBPC.th3_c.bits.BPC_C_TH_SLA;
    MINT32 BPC_C_TH_SLBpos = rUpperBPC.th3_c.bits.BPC_C_TH_SLB;
    MINT32 BPC_C_TH_XApos = rUpperBPC.th3_c.bits.BPC_C_TH_XA;
    MINT32 BPC_C_TH_XBpos = rUpperBPC.th3_c.bits.BPC_C_TH_XB;
    MINT32 BPC_C_TH_Ypos = rUpperBPC.th1_c.bits.BPC_C_TH_Y;
    MINT32 BPC_C_TH_UPBpos = rUpperBPC.th2_c.bits.BPC_C_TH_UPB;
    MINT32 BPC_NO_LVpos = rUpperBPC.dtc.bits.BPC_NO_LV;
    MINT32 BPC_CT_LVpos = rUpperBPC.dtc.bits.BPC_CT_LV;
    MINT32 BPC_TH_MULpos = rUpperBPC.dtc.bits.BPC_TH_MUL;
    MINT32 BPC_CS_MODEpos = rUpperBPC.con.bits.BPC_CS_MODE;
    MINT32 BPC_CS_RNGpos = rUpperBPC.dtc.bits.BPC_CS_RNG;
    MINT32 BPC_TABLE_END_MODEpos = rUpperBPC.con.bits.BPC_TABLE_END_MODE;
    MINT32 BPC_AVG_MODEpos = rUpperBPC.con.bits.BPC_AVG_MODE;
    MINT32 BNR_LE_INV_CTLpos = rUpperBPC.con.bits.BNR_LE_INV_CTL;
    MINT32 BNR_OSC_COUNTpos = rUpperBPC.con.bits.BNR_OSC_COUNT;
    MINT32 BNR_EDGEpos = rUpperBPC.con.bits.BNR_EDGE;
    MINT32 BPC_EXCpos = rUpperBPC.con.bits.BPC_EXC;
    MINT32 BPC_RANK_IDXRpos = rUpperBPC.cor.bits.BPC_RANK_IDXR;
    MINT32 BPC_RANK_IDXGpos = rUpperBPC.cor.bits.BPC_RANK_IDXG;
    MINT32 BPC_RANK_IDXBpos = rUpperBPC.th2_c.bits.BPC_RANK_IDXB;
    MINT32 BPC_BLD_MODEpos = rUpperBPC.con.bits.BPC_BLD_MODE;
    MINT32 BPC_BLD_LWBpos = rUpperBPC.th2_c.bits.BPC_BLD_LWB;
    MINT32 BPC_BLD0pos = rUpperBPC.th2.bits.BPC_BLD0;
    MINT32 BPC_BLD1pos = rUpperBPC.th2.bits.BPC_BLD1;
    MINT32 BPC_BLD_SLP0pos = rUpperBPC.th1.bits.BPC_BLD_SLP0;

// interpolation //
    MINT32 ISO = u4RealISO;
    MINT32 ISOpre = u4LowerISO;
    MINT32 ISOpos = u4UpperISO;

    MINT32 BPC_EN = InterParam_BPC(ISO,ISOpre,ISOpos, BPC_ENpre, BPC_ENpos, 0);
    MINT32 BPC_LUT_EN = InterParam_BPC(ISO,ISOpre,ISOpos, BPC_LUT_ENpre, BPC_LUT_ENpos, 0);
    MINT32 BPC_DTC_MODE = InterParam_BPC(ISO,ISOpre,ISOpos, BPC_DTC_MODEpre,BPC_DTC_MODEpos,0);
    MINT32 BPC_CRC_MODE = InterParam_BPC(ISO,ISOpre,ISOpos, BPC_CRC_MODEpre,BPC_CRC_MODEpos,0);
    MINT32 BPC_RNG = InterParam_BPC(ISO,ISOpre,ISOpos, BPC_RNGpre, BPC_RNGpos, 0);
    MINT32 BPC_DIR_TH = InterParam_BPC(ISO,ISOpre,ISOpos, BPC_DIR_THpre, BPC_DIR_THpos, 0);
    MINT32 BPC_DIR_TH2 = InterParam_BPC(ISO,ISOpre,ISOpos, BPC_DIR_TH2pre,BPC_DIR_TH2pos,0);
    MINT32 BPC_DIR_MAX = InterParam_BPC(ISO,ISOpre,ISOpos, BPC_DIR_MAXpre,BPC_DIR_MAXpos,0);
    MINT32 BPC_TH_LWB = InterParam_BPC(ISO,ISOpre,ISOpos, BPC_TH_LWBpre, BPC_TH_LWBpos, 0);
    MINT32 BPC_TH_SLA = InterParam_BPC(ISO,ISOpre,ISOpos, BPC_TH_SLApre, BPC_TH_SLApos, 0);
    MINT32 BPC_DK_TH_SLA = InterParam_BPC(ISO,ISOpre,ISOpos, BPC_DK_TH_SLApre,BPC_DK_TH_SLApos,0);
    MINT32 BPC_TH_SLB = InterParam_BPC(ISO,ISOpre,ISOpos, BPC_TH_SLBpre, BPC_TH_SLBpos, 0);
    MINT32 BPC_DK_TH_SLB = InterParam_BPC(ISO,ISOpre,ISOpos, BPC_DK_TH_SLBpre,BPC_DK_TH_SLBpos,0);
    MINT32 BPC_TH_XA = InterParam_BPC(ISO,ISOpre,ISOpos, BPC_TH_XApre, BPC_TH_XApos, 0);
    MINT32 BPC_DK_TH_XA = InterParam_BPC(ISO,ISOpre,ISOpos, BPC_DK_TH_XApre,BPC_DK_TH_XApos,0);
    MINT32 BPC_TH_XB = InterParam_BPC(ISO,ISOpre,ISOpos, BPC_TH_XBpre, BPC_TH_XBpos, 0);
    MINT32 BPC_DK_TH_XB = InterParam_BPC(ISO,ISOpre,ISOpos, BPC_DK_TH_XBpre,BPC_DK_TH_XBpos,0);
    MINT32 BPC_TH_Y = InterParam_BPC(ISO,ISOpre,ISOpos, BPC_TH_Ypre, BPC_TH_Ypos, 0);
    MINT32 BPC_TH_UPB = InterParam_BPC(ISO,ISOpre,ISOpos, BPC_TH_UPBpre, BPC_TH_UPBpos, 0);
    MINT32 BPC_C_TH_LWB = InterParam_BPC(ISO,ISOpre,ISOpos, BPC_C_TH_LWBpre,BPC_C_TH_LWBpos,0);
    MINT32 BPC_C_TH_SLA = InterParam_BPC(ISO,ISOpre,ISOpos, BPC_C_TH_SLApre,BPC_C_TH_SLApos,0);
    MINT32 BPC_C_TH_SLB = InterParam_BPC(ISO,ISOpre,ISOpos, BPC_C_TH_SLBpre,BPC_C_TH_SLBpos,0);
    MINT32 BPC_C_TH_XA = InterParam_BPC(ISO,ISOpre,ISOpos, BPC_C_TH_XApre,BPC_C_TH_XApos,0);
    MINT32 BPC_C_TH_XB = InterParam_BPC(ISO,ISOpre,ISOpos, BPC_C_TH_XBpre,BPC_C_TH_XBpos,0);
    MINT32 BPC_C_TH_Y = InterParam_BPC(ISO,ISOpre,ISOpos, BPC_C_TH_Ypre, BPC_C_TH_Ypos, 0);
    MINT32 BPC_C_TH_UPB = InterParam_BPC(ISO,ISOpre,ISOpos, BPC_C_TH_UPBpre,BPC_C_TH_UPBpos,0);
    MINT32 BPC_NO_LV = InterParam_BPC(ISO,ISOpre,ISOpos, BPC_NO_LVpre, BPC_NO_LVpos, 0);
    MINT32 BPC_CT_LV = InterParam_BPC(ISO,ISOpre,ISOpos, BPC_CT_LVpre, BPC_CT_LVpos, 0);
    MINT32 BPC_TH_MUL = InterParam_BPC(ISO,ISOpre,ISOpos, BPC_TH_MULpre, BPC_TH_MULpos, 0);
    MINT32 BPC_CS_MODE = InterParam_BPC(ISO,ISOpre,ISOpos, BPC_CS_MODEpre,BPC_CS_MODEpos,0);
    MINT32 BPC_CS_RNG = InterParam_BPC(ISO,ISOpre,ISOpos, BPC_CS_RNGpre, BPC_CS_RNGpos, 0);
    MINT32 BPC_TABLE_END_MODE = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_TABLE_END_MODEpre, BPC_TABLE_END_MODEpos, 1);
    MINT32 BPC_AVG_MODE = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_AVG_MODEpre, BPC_AVG_MODEpos, 1);
    MINT32 BNR_LE_INV_CTL = InterParam_BPC(ISO, ISOpre, ISOpos, BNR_LE_INV_CTLpre, BNR_LE_INV_CTLpos, 1);
    MINT32 BNR_OSC_COUNT = InterParam_BPC(ISO, ISOpre, ISOpos, BNR_OSC_COUNTpre, BNR_OSC_COUNTpos, 0);
    MINT32 BNR_EDGE = InterParam_BPC(ISO, ISOpre, ISOpos, BNR_EDGEpre, BNR_EDGEpos, 1);
    MINT32 BPC_EXC = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_EXCpre, BPC_EXCpos  , 0);
    MINT32 BPC_RANK_IDXR = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_RANK_IDXRpre, BPC_RANK_IDXRpos, 0);
    MINT32 BPC_RANK_IDXG = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_RANK_IDXGpre, BPC_RANK_IDXGpos, 0);
    MINT32 BPC_RANK_IDXB = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_RANK_IDXBpre, BPC_RANK_IDXBpos, 0);
    MINT32 BPC_BLD_MODE = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_BLD_MODEpre, BPC_BLD_MODEpos, 1);
    MINT32 BPC_BLD_LWB = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_BLD_LWBpre, BPC_BLD_LWBpos, 0);
    MINT32 BPC_BLD0 = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_BLD0pre, BPC_BLD0pos, 0);
    MINT32 BPC_BLD1 = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_BLD1pre, BPC_BLD1pos, 0);
    MINT32 BPC_BLD_SLP0 = InterParam_BPC(ISO, ISOpre, ISOpos, BPC_BLD_SLP0pre, BPC_BLD_SLP0pos, 0);

    rSmoothBPC.con.bits.BPC_EN = BPC_EN;
    rSmoothBPC.con.bits.BPC_LUT_EN = BPC_LUT_EN;
    rSmoothBPC.con.bits.BPC_DTC_MODE = BPC_DTC_MODE;
    rSmoothBPC.con.bits.BPC_CRC_MODE = BPC_CRC_MODE;
    rSmoothBPC.dtc.bits.BPC_RNG = BPC_RNG;
    rSmoothBPC.cor.bits.BPC_DIR_TH = BPC_DIR_TH;
    rSmoothBPC.cor.bits.BPC_DIR_TH2 = BPC_DIR_TH2;
    rSmoothBPC.cor.bits.BPC_DIR_MAX = BPC_DIR_MAX;
    rSmoothBPC.th1.bits.BPC_TH_LWB = BPC_TH_LWB;
    rSmoothBPC.th3.bits.BPC_TH_SLA = BPC_TH_SLA;
    rSmoothBPC.th4.bits.BPC_DK_TH_SLA = BPC_DK_TH_SLA;
    rSmoothBPC.th3.bits.BPC_TH_SLB = BPC_TH_SLB;
    rSmoothBPC.th4.bits.BPC_DK_TH_SLB = BPC_DK_TH_SLB;
    rSmoothBPC.th3.bits.BPC_TH_XA = BPC_TH_XA;
    rSmoothBPC.th4.bits.BPC_DK_TH_XA = BPC_DK_TH_XA;
    rSmoothBPC.th3.bits.BPC_TH_XB = BPC_TH_XB;
    rSmoothBPC.th4.bits.BPC_DK_TH_XB = BPC_DK_TH_XB;
    rSmoothBPC.th1.bits.BPC_TH_Y = BPC_TH_Y;
    rSmoothBPC.th2.bits.BPC_TH_UPB = BPC_TH_UPB;
    rSmoothBPC.th1_c.bits.BPC_C_TH_LWB = BPC_C_TH_LWB;
    rSmoothBPC.th3_c.bits.BPC_C_TH_SLA = BPC_C_TH_SLA;
    rSmoothBPC.th3_c.bits.BPC_C_TH_SLB = BPC_C_TH_SLB;
    rSmoothBPC.th3_c.bits.BPC_C_TH_XA = BPC_C_TH_XA;
    rSmoothBPC.th3_c.bits.BPC_C_TH_XB = BPC_C_TH_XB;
    rSmoothBPC.th1_c.bits.BPC_C_TH_Y = BPC_C_TH_Y;
    rSmoothBPC.th2_c.bits.BPC_C_TH_UPB = BPC_C_TH_UPB;
    rSmoothBPC.dtc.bits.BPC_NO_LV = BPC_NO_LV;
    rSmoothBPC.dtc.bits.BPC_CT_LV = BPC_CT_LV;
    rSmoothBPC.dtc.bits.BPC_TH_MUL = BPC_TH_MUL;
    rSmoothBPC.con.bits.BPC_CS_MODE = BPC_CS_MODE;
    rSmoothBPC.dtc.bits.BPC_CS_RNG = BPC_CS_RNG;
    rSmoothBPC.con.bits.BPC_TABLE_END_MODE = BPC_TABLE_END_MODE;
    rSmoothBPC.con.bits.BPC_AVG_MODE = BPC_AVG_MODE;
    rSmoothBPC.con.bits.BNR_LE_INV_CTL = BNR_LE_INV_CTL;
    rSmoothBPC.con.bits.BNR_OSC_COUNT = BNR_OSC_COUNT;
    rSmoothBPC.con.bits.BNR_EDGE = BNR_EDGE;
    rSmoothBPC.con.bits.BPC_EXC = BPC_EXC;
    rSmoothBPC.cor.bits.BPC_RANK_IDXR = BPC_RANK_IDXR;
    rSmoothBPC.cor.bits.BPC_RANK_IDXG = BPC_RANK_IDXG;
    rSmoothBPC.th2_c.bits.BPC_RANK_IDXB = BPC_RANK_IDXB;
    rSmoothBPC.con.bits.BPC_BLD_MODE = BPC_BLD_MODE;
    rSmoothBPC.th2_c.bits.BPC_BLD_LWB = BPC_BLD_LWB;
    rSmoothBPC.th2.bits.BPC_BLD0 = BPC_BLD0;
    rSmoothBPC.th2.bits.BPC_BLD1 = BPC_BLD1;
    rSmoothBPC.th1.bits.BPC_BLD_SLP0 = BPC_BLD_SLP0;

    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_EN = %d, %d, %d", rLowerBPC.con.bits.BPC_EN, rSmoothBPC.con.bits.BPC_EN, rUpperBPC.con.bits.BPC_EN);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_LUT_EN = %d, %d, %d", rLowerBPC.con.bits.BPC_LUT_EN, rSmoothBPC.con.bits.BPC_LUT_EN, rUpperBPC.con.bits.BPC_LUT_EN);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_DTC_MODE = %d, %d, %d", rLowerBPC.con.bits.BPC_DTC_MODE, rSmoothBPC.con.bits.BPC_DTC_MODE, rUpperBPC.con.bits.BPC_DTC_MODE);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_CRC_MODE = %d, %d, %d", rLowerBPC.con.bits.BPC_CRC_MODE, rSmoothBPC.con.bits.BPC_CRC_MODE, rUpperBPC.con.bits.BPC_CRC_MODE);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_RNG = %d, %d, %d", rLowerBPC.dtc.bits.BPC_RNG, rSmoothBPC.dtc.bits.BPC_RNG, rUpperBPC.dtc.bits.BPC_RNG);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_DIR_TH = %d, %d, %d", rLowerBPC.cor.bits.BPC_DIR_TH, rSmoothBPC.cor.bits.BPC_DIR_TH, rUpperBPC.cor.bits.BPC_DIR_TH);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_DIR_TH2 = %d, %d, %d", rLowerBPC.cor.bits.BPC_DIR_TH2, rSmoothBPC.cor.bits.BPC_DIR_TH2, rUpperBPC.cor.bits.BPC_DIR_TH2);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_DIR_MAX = %d, %d, %d", rLowerBPC.cor.bits.BPC_DIR_MAX, rSmoothBPC.cor.bits.BPC_DIR_MAX, rUpperBPC.cor.bits.BPC_DIR_MAX);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_TH_LWB = %d, %d, %d", rLowerBPC.th1.bits.BPC_TH_LWB, rSmoothBPC.th1.bits.BPC_TH_LWB, rUpperBPC.th1.bits.BPC_TH_LWB);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_TH_SLA = %d, %d, %d", rLowerBPC.th3.bits.BPC_TH_SLA, rSmoothBPC.th3.bits.BPC_TH_SLA, rUpperBPC.th3.bits.BPC_TH_SLA);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_DK_TH_SLA = %d, %d, %d", rLowerBPC.th4.bits.BPC_DK_TH_SLA, rSmoothBPC.th4.bits.BPC_DK_TH_SLA, rUpperBPC.th4.bits.BPC_DK_TH_SLA);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_TH_SLB = %d, %d, %d", rLowerBPC.th3.bits.BPC_TH_SLB, rSmoothBPC.th3.bits.BPC_TH_SLB, rUpperBPC.th3.bits.BPC_TH_SLB);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_DK_TH_SLB = %d, %d, %d", rLowerBPC.th4.bits.BPC_DK_TH_SLB, rSmoothBPC.th4.bits.BPC_DK_TH_SLB, rUpperBPC.th4.bits.BPC_DK_TH_SLB);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_TH_XA = %d, %d, %d", rLowerBPC.th3.bits.BPC_TH_XA, rSmoothBPC.th3.bits.BPC_TH_XA, rUpperBPC.th3.bits.BPC_TH_XA);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_DK_TH_XA = %d, %d, %d", rLowerBPC.th4.bits.BPC_DK_TH_XA, rSmoothBPC.th4.bits.BPC_DK_TH_XA, rUpperBPC.th4.bits.BPC_DK_TH_XA);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_TH_XB = %d, %d, %d", rLowerBPC.th3.bits.BPC_TH_XB, rSmoothBPC.th3.bits.BPC_TH_XB, rUpperBPC.th3.bits.BPC_TH_XB);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_DK_TH_XB = %d, %d, %d", rLowerBPC.th4.bits.BPC_DK_TH_XB, rSmoothBPC.th4.bits.BPC_DK_TH_XB, rUpperBPC.th4.bits.BPC_DK_TH_XB);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_TH_Y = %d, %d, %d", rLowerBPC.th1.bits.BPC_TH_Y, rSmoothBPC.th1.bits.BPC_TH_Y, rUpperBPC.th1.bits.BPC_TH_Y);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_TH_UPB = %d, %d, %d", rLowerBPC.th2.bits.BPC_TH_UPB, rSmoothBPC.th2.bits.BPC_TH_UPB, rUpperBPC.th2.bits.BPC_TH_UPB);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_C_TH_LWB = %d, %d, %d", rLowerBPC.th1_c.bits.BPC_C_TH_LWB, rSmoothBPC.th1_c.bits.BPC_C_TH_LWB, rUpperBPC.th1_c.bits.BPC_C_TH_LWB);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_C_TH_SLA = %d, %d, %d", rLowerBPC.th3_c.bits.BPC_C_TH_SLA, rSmoothBPC.th3_c.bits.BPC_C_TH_SLA, rUpperBPC.th3_c.bits.BPC_C_TH_SLA);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_C_TH_SLB = %d, %d, %d", rLowerBPC.th3_c.bits.BPC_C_TH_SLB, rSmoothBPC.th3_c.bits.BPC_C_TH_SLB, rUpperBPC.th3_c.bits.BPC_C_TH_SLB);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_C_TH_XA = %d, %d, %d", rLowerBPC.th3_c.bits.BPC_C_TH_XA, rSmoothBPC.th3_c.bits.BPC_C_TH_XA, rUpperBPC.th3_c.bits.BPC_C_TH_XA);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_C_TH_XB = %d, %d, %d", rLowerBPC.th3_c.bits.BPC_C_TH_XB, rSmoothBPC.th3_c.bits.BPC_C_TH_XB, rUpperBPC.th3_c.bits.BPC_C_TH_XB);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_C_TH_Y = %d, %d, %d", rLowerBPC.th1_c.bits.BPC_C_TH_Y, rSmoothBPC.th1_c.bits.BPC_C_TH_Y, rUpperBPC.th1_c.bits.BPC_C_TH_Y);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_C_TH_UPB = %d, %d, %d", rLowerBPC.th2_c.bits.BPC_C_TH_UPB, rSmoothBPC.th2_c.bits.BPC_C_TH_UPB, rUpperBPC.th2_c.bits.BPC_C_TH_UPB);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_NO_LV = %d, %d, %d", rLowerBPC.dtc.bits.BPC_NO_LV, rSmoothBPC.dtc.bits.BPC_NO_LV, rUpperBPC.dtc.bits.BPC_NO_LV);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_CT_LV = %d, %d, %d", rLowerBPC.dtc.bits.BPC_CT_LV, rSmoothBPC.dtc.bits.BPC_CT_LV, rUpperBPC.dtc.bits.BPC_CT_LV);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_TH_MUL = %d, %d, %d", rLowerBPC.dtc.bits.BPC_TH_MUL, rSmoothBPC.dtc.bits.BPC_TH_MUL, rUpperBPC.dtc.bits.BPC_TH_MUL);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_CS_MODE = %d, %d, %d", rLowerBPC.con.bits.BPC_CS_MODE, rSmoothBPC.con.bits.BPC_CS_MODE, rUpperBPC.con.bits.BPC_CS_MODE);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_CS_RNG = %d, %d, %d", rLowerBPC.dtc.bits.BPC_CS_RNG, rSmoothBPC.dtc.bits.BPC_CS_RNG, rUpperBPC.dtc.bits.BPC_CS_RNG);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_TABLE_END_MODE = %d, %d, %d", rLowerBPC.con.bits.BPC_TABLE_END_MODE, rSmoothBPC.con.bits.BPC_TABLE_END_MODE, rUpperBPC.con.bits.BPC_TABLE_END_MODE);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_AVG_MODE = %d, %d, %d", rLowerBPC.con.bits.BPC_AVG_MODE, rSmoothBPC.con.bits.BPC_AVG_MODE, rUpperBPC.con.bits.BPC_AVG_MODE);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BNR_LE_INV_CTL = %d, %d, %d", rLowerBPC.con.bits.BNR_LE_INV_CTL, rSmoothBPC.con.bits.BNR_LE_INV_CTL, rUpperBPC.con.bits.BNR_LE_INV_CTL);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BNR_OSC_COUNT = %d, %d, %d", rLowerBPC.con.bits.BNR_OSC_COUNT, rSmoothBPC.con.bits.BNR_OSC_COUNT, rUpperBPC.con.bits.BNR_OSC_COUNT);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BNR_EDGE = %d, %d, %d", rLowerBPC.con.bits.BNR_EDGE, rSmoothBPC.con.bits.BNR_EDGE, rUpperBPC.con.bits.BNR_EDGE);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_EXC = %d, %d, %d", rLowerBPC.con.bits.BPC_EXC, rSmoothBPC.con.bits.BPC_EXC, rUpperBPC.con.bits.BPC_EXC);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_RANK_IDXR = %d, %d, %d", rLowerBPC.cor.bits.BPC_RANK_IDXR, rSmoothBPC.cor.bits.BPC_RANK_IDXR, rUpperBPC.cor.bits.BPC_RANK_IDXR);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_RANK_IDXG = %d, %d, %d", rLowerBPC.cor.bits.BPC_RANK_IDXG, rSmoothBPC.cor.bits.BPC_RANK_IDXG, rUpperBPC.cor.bits.BPC_RANK_IDXG);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_RANK_IDXB = %d, %d, %d", rLowerBPC.th2_c.bits.BPC_RANK_IDXB, rSmoothBPC.th2_c.bits.BPC_RANK_IDXB, rUpperBPC.th2_c.bits.BPC_RANK_IDXB);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_BLD_MODE = %d, %d, %d", rLowerBPC.con.bits.BPC_BLD_MODE, rSmoothBPC.con.bits.BPC_BLD_MODE, rUpperBPC.con.bits.BPC_BLD_MODE);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_BLD_LWB = %d, %d, %d", rLowerBPC.th2_c.bits.BPC_BLD_LWB, rSmoothBPC.th2_c.bits.BPC_BLD_LWB, rUpperBPC.th2_c.bits.BPC_BLD_LWB);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_BLD0 = %d, %d, %d", rLowerBPC.th2.bits.BPC_BLD0, rSmoothBPC.th2.bits.BPC_BLD0, rUpperBPC.th2.bits.BPC_BLD0);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_BLD1 = %d, %d, %d", rLowerBPC.th2.bits.BPC_BLD1, rSmoothBPC.th2.bits.BPC_BLD1, rUpperBPC.th2.bits.BPC_BLD1);
    MY_LOG_IF(bSmoothBPCDebug, "[L,S,U]BPC_BLD_SLP0 = %d, %d, %d", rLowerBPC.th1.bits.BPC_BLD_SLP0, rSmoothBPC.th1.bits.BPC_BLD_SLP0, rUpperBPC.th1.bits.BPC_BLD_SLP0);
}

MINT32 InterParam_DBS(MINT32 ISO, MINT32 ISOpre, MINT32 ISOpos, MINT32 PARAMpre, MINT32 PARAMpos, MINT32 Method) {
    MINT32 InterPARAM = 0;

    if(ISOpre == ISOpos) {
        // To prevent someone sets different PARAMpre and PARAMpos
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

MVOID SmoothDBS(MUINT32 u4RealISO, // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_DBS_T const& rUpperDBS, // DBS settings for upper ISO
                ISP_NVRAM_DBS_T const& rLowerDBS, // DBS settings for lower ISO
                ISP_NVRAM_DBS_T& rSmoothDBS)   // Output
{

	MBOOL bSmoothDBSDebug = interpPropertyGet("vendor.debug.smooth_dbs.enable", MFALSE);

    MY_LOG_IF(bSmoothDBSDebug,"[%s()] u4RealISO: %d, u4UpperISO = %d, u4LowerISO = %d\n", __FUNCTION__, u4RealISO, u4UpperISO, u4LowerISO);

        MINT32 pre_DBS_OFST = rLowerDBS.sigma.bits.DBS_OFST;
        MINT32 pre_DBS_SL = rLowerDBS.sigma.bits.DBS_SL;
        MINT32 pre_DBS_MUL = rLowerDBS.sigma.bits.DBS_MUL;
        MINT32 pre_DBS_BIAS_Y0 = rLowerDBS.bstbl_0.bits.DBS_BIAS_Y0;
        MINT32 pre_DBS_BIAS_Y1 = rLowerDBS.bstbl_0.bits.DBS_BIAS_Y1;
        MINT32 pre_DBS_BIAS_Y2 = rLowerDBS.bstbl_0.bits.DBS_BIAS_Y2;
        MINT32 pre_DBS_BIAS_Y3 = rLowerDBS.bstbl_0.bits.DBS_BIAS_Y3;
        MINT32 pre_DBS_BIAS_Y4 = rLowerDBS.bstbl_1.bits.DBS_BIAS_Y4;
        MINT32 pre_DBS_BIAS_Y5 = rLowerDBS.bstbl_1.bits.DBS_BIAS_Y5;
        MINT32 pre_DBS_BIAS_Y6 = rLowerDBS.bstbl_1.bits.DBS_BIAS_Y6;
        MINT32 pre_DBS_BIAS_Y7 = rLowerDBS.bstbl_1.bits.DBS_BIAS_Y7;
        MINT32 pre_DBS_BIAS_Y8 = rLowerDBS.bstbl_2.bits.DBS_BIAS_Y8;
        MINT32 pre_DBS_BIAS_Y9 = rLowerDBS.bstbl_2.bits.DBS_BIAS_Y9;
        MINT32 pre_DBS_BIAS_Y10 = rLowerDBS.bstbl_2.bits.DBS_BIAS_Y10;
        MINT32 pre_DBS_BIAS_Y11 = rLowerDBS.bstbl_2.bits.DBS_BIAS_Y11;
        MINT32 pre_DBS_BIAS_Y12 = rLowerDBS.bstbl_3.bits.DBS_BIAS_Y12;
        MINT32 pre_DBS_HDR_GAIN = rLowerDBS.ctl.bits.DBS_HDR_GAIN;
        MINT32 pre_DBS_HDR_GAIN2 = rLowerDBS.ctl.bits.DBS_HDR_GAIN2;
        MINT32 pre_DBS_HDR_GNP = rLowerDBS.ctl.bits.DBS_HDR_GNP;
        MINT32 pre_DBS_LE_INV_CTL = rLowerDBS.ctl.bits.DBS_LE_INV_CTL;

        MINT32 pos_DBS_OFST = rUpperDBS.sigma.bits.DBS_OFST;
        MINT32 pos_DBS_SL = rUpperDBS.sigma.bits.DBS_SL;
        MINT32 pos_DBS_MUL = rUpperDBS.sigma.bits.DBS_MUL;
        MINT32 pos_DBS_BIAS_Y0 = rUpperDBS.bstbl_0.bits.DBS_BIAS_Y0;
        MINT32 pos_DBS_BIAS_Y1 = rUpperDBS.bstbl_0.bits.DBS_BIAS_Y1;
        MINT32 pos_DBS_BIAS_Y2 = rUpperDBS.bstbl_0.bits.DBS_BIAS_Y2;
        MINT32 pos_DBS_BIAS_Y3 = rUpperDBS.bstbl_0.bits.DBS_BIAS_Y3;
        MINT32 pos_DBS_BIAS_Y4 = rUpperDBS.bstbl_1.bits.DBS_BIAS_Y4;
        MINT32 pos_DBS_BIAS_Y5 = rUpperDBS.bstbl_1.bits.DBS_BIAS_Y5;
        MINT32 pos_DBS_BIAS_Y6 = rUpperDBS.bstbl_1.bits.DBS_BIAS_Y6;
        MINT32 pos_DBS_BIAS_Y7 = rUpperDBS.bstbl_1.bits.DBS_BIAS_Y7;
        MINT32 pos_DBS_BIAS_Y8 = rUpperDBS.bstbl_2.bits.DBS_BIAS_Y8;
        MINT32 pos_DBS_BIAS_Y9 = rUpperDBS.bstbl_2.bits.DBS_BIAS_Y9;
        MINT32 pos_DBS_BIAS_Y10 = rUpperDBS.bstbl_2.bits.DBS_BIAS_Y10;
        MINT32 pos_DBS_BIAS_Y11 = rUpperDBS.bstbl_2.bits.DBS_BIAS_Y11;
        MINT32 pos_DBS_BIAS_Y12 = rUpperDBS.bstbl_3.bits.DBS_BIAS_Y12;
        MINT32 pos_DBS_HDR_GAIN = rUpperDBS.ctl.bits.DBS_HDR_GAIN;
        MINT32 pos_DBS_HDR_GAIN2 = rUpperDBS.ctl.bits.DBS_HDR_GAIN2;
        MINT32 pos_DBS_HDR_GNP = rUpperDBS.ctl.bits.DBS_HDR_GNP;
        MINT32 pos_DBS_LE_INV_CTL = rUpperDBS.ctl.bits.DBS_LE_INV_CTL;

    // interpolation //
    MINT32 ISO = u4RealISO;
    MINT32 ISOpre = u4LowerISO;
    MINT32 ISOpos = u4UpperISO;

        MINT32 DBS_OFST = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_OFST, pos_DBS_OFST, 0);
        MINT32 DBS_SL = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_SL, pos_DBS_SL, 0);
        MINT32 DBS_MUL = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_MUL, pos_DBS_MUL, 0);
        MINT32 DBS_BIAS_Y0 = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_BIAS_Y0, pos_DBS_BIAS_Y0, 0);
        MINT32 DBS_BIAS_Y1 = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_BIAS_Y1, pos_DBS_BIAS_Y1, 0);
        MINT32 DBS_BIAS_Y2 = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_BIAS_Y2, pos_DBS_BIAS_Y2, 0);
        MINT32 DBS_BIAS_Y3 = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_BIAS_Y3, pos_DBS_BIAS_Y3, 0);
        MINT32 DBS_BIAS_Y4 = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_BIAS_Y4, pos_DBS_BIAS_Y4, 0);
        MINT32 DBS_BIAS_Y5 = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_BIAS_Y5, pos_DBS_BIAS_Y5, 0);
        MINT32 DBS_BIAS_Y6 = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_BIAS_Y6, pos_DBS_BIAS_Y6, 0);
        MINT32 DBS_BIAS_Y7 = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_BIAS_Y7, pos_DBS_BIAS_Y7, 0);
        MINT32 DBS_BIAS_Y8 = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_BIAS_Y8, pos_DBS_BIAS_Y8, 0);
        MINT32 DBS_BIAS_Y9 = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_BIAS_Y9, pos_DBS_BIAS_Y9, 0);
        MINT32 DBS_BIAS_Y10 = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_BIAS_Y10, pos_DBS_BIAS_Y10, 0);
        MINT32 DBS_BIAS_Y11 = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_BIAS_Y11, pos_DBS_BIAS_Y11, 0);
        MINT32 DBS_BIAS_Y12 = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_BIAS_Y12, pos_DBS_BIAS_Y12, 0);
        MINT32 DBS_HDR_GAIN = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_HDR_GAIN, pos_DBS_HDR_GAIN, 0);
        MINT32 DBS_HDR_GAIN2 = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_HDR_GAIN2, pos_DBS_HDR_GAIN2, 0);
        MINT32 DBS_HDR_GNP = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_HDR_GNP, pos_DBS_HDR_GNP, 0);
        MINT32 DBS_LE_INV_CTL = InterParam_DBS(ISO, ISOpre, ISOpos, pre_DBS_LE_INV_CTL, pos_DBS_LE_INV_CTL, 0);

    rSmoothDBS.sigma.bits.DBS_OFST = DBS_OFST;
    rSmoothDBS.sigma.bits.DBS_SL = DBS_SL;
    rSmoothDBS.sigma.bits.DBS_MUL = DBS_MUL;
    rSmoothDBS.bstbl_0.bits.DBS_BIAS_Y0 = DBS_BIAS_Y0;
    rSmoothDBS.bstbl_0.bits.DBS_BIAS_Y1 = DBS_BIAS_Y1;
    rSmoothDBS.bstbl_0.bits.DBS_BIAS_Y2 = DBS_BIAS_Y2;
    rSmoothDBS.bstbl_0.bits.DBS_BIAS_Y3 = DBS_BIAS_Y3;
    rSmoothDBS.bstbl_1.bits.DBS_BIAS_Y4 = DBS_BIAS_Y4;
    rSmoothDBS.bstbl_1.bits.DBS_BIAS_Y5 = DBS_BIAS_Y5;
    rSmoothDBS.bstbl_1.bits.DBS_BIAS_Y6 = DBS_BIAS_Y6;
    rSmoothDBS.bstbl_1.bits.DBS_BIAS_Y7 = DBS_BIAS_Y7;
    rSmoothDBS.bstbl_2.bits.DBS_BIAS_Y8 = DBS_BIAS_Y8;
    rSmoothDBS.bstbl_2.bits.DBS_BIAS_Y9 = DBS_BIAS_Y9;
    rSmoothDBS.bstbl_2.bits.DBS_BIAS_Y10 = DBS_BIAS_Y10;
    rSmoothDBS.bstbl_2.bits.DBS_BIAS_Y11 = DBS_BIAS_Y11;
    rSmoothDBS.bstbl_3.bits.DBS_BIAS_Y12 = DBS_BIAS_Y12;
    rSmoothDBS.ctl.bits.DBS_HDR_GAIN = DBS_HDR_GAIN;
    rSmoothDBS.ctl.bits.DBS_HDR_GAIN2 = DBS_HDR_GAIN2;
    rSmoothDBS.ctl.bits.DBS_HDR_GNP = DBS_HDR_GNP;
    rSmoothDBS.ctl.bits.DBS_LE_INV_CTL = DBS_LE_INV_CTL;

    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_OFST = %d, %d, %d", rLowerDBS.sigma.bits.DBS_OFST, rSmoothDBS.sigma.bits.DBS_OFST, rUpperDBS.sigma.bits.DBS_OFST       );
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_SL = %d, %d, %d", rLowerDBS.sigma.bits.DBS_SL, rSmoothDBS.sigma.bits.DBS_SL, rUpperDBS.sigma.bits.DBS_SL         );
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_MUL = %d, %d, %d", rLowerDBS.sigma.bits.DBS_MUL, rSmoothDBS.sigma.bits.DBS_MUL, rUpperDBS.sigma.bits.DBS_MUL        );
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_BIAS_Y0 = %d, %d, %d", rLowerDBS.bstbl_0.bits.DBS_BIAS_Y0, rSmoothDBS.bstbl_0.bits.DBS_BIAS_Y0, rUpperDBS.bstbl_0.bits.DBS_BIAS_Y0  );
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_BIAS_Y1 = %d, %d, %d", rLowerDBS.bstbl_0.bits.DBS_BIAS_Y1, rSmoothDBS.bstbl_0.bits.DBS_BIAS_Y1, rUpperDBS.bstbl_0.bits.DBS_BIAS_Y1  );
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_BIAS_Y2 = %d, %d, %d", rLowerDBS.bstbl_0.bits.DBS_BIAS_Y2, rSmoothDBS.bstbl_0.bits.DBS_BIAS_Y2, rUpperDBS.bstbl_0.bits.DBS_BIAS_Y2  );
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_BIAS_Y3 = %d, %d, %d", rLowerDBS.bstbl_0.bits.DBS_BIAS_Y3, rSmoothDBS.bstbl_0.bits.DBS_BIAS_Y3, rUpperDBS.bstbl_0.bits.DBS_BIAS_Y3  );
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_BIAS_Y4 = %d, %d, %d", rLowerDBS.bstbl_1.bits.DBS_BIAS_Y4, rSmoothDBS.bstbl_1.bits.DBS_BIAS_Y4, rUpperDBS.bstbl_1.bits.DBS_BIAS_Y4  );
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_BIAS_Y5 = %d, %d, %d", rLowerDBS.bstbl_1.bits.DBS_BIAS_Y5, rSmoothDBS.bstbl_1.bits.DBS_BIAS_Y5, rUpperDBS.bstbl_1.bits.DBS_BIAS_Y5  );
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_BIAS_Y6 = %d, %d, %d", rLowerDBS.bstbl_1.bits.DBS_BIAS_Y6, rSmoothDBS.bstbl_1.bits.DBS_BIAS_Y6, rUpperDBS.bstbl_1.bits.DBS_BIAS_Y6  );
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_BIAS_Y7 = %d, %d, %d", rLowerDBS.bstbl_1.bits.DBS_BIAS_Y7, rSmoothDBS.bstbl_1.bits.DBS_BIAS_Y7, rUpperDBS.bstbl_1.bits.DBS_BIAS_Y7  );
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_BIAS_Y8 = %d, %d, %d", rLowerDBS.bstbl_2.bits.DBS_BIAS_Y8, rSmoothDBS.bstbl_2.bits.DBS_BIAS_Y8, rUpperDBS.bstbl_2.bits.DBS_BIAS_Y8  );
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_BIAS_Y9 = %d, %d, %d", rLowerDBS.bstbl_2.bits.DBS_BIAS_Y9, rSmoothDBS.bstbl_2.bits.DBS_BIAS_Y9, rUpperDBS.bstbl_2.bits.DBS_BIAS_Y9  );
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_BIAS_Y10 = %d, %d, %d", rLowerDBS.bstbl_2.bits.DBS_BIAS_Y10, rSmoothDBS.bstbl_2.bits.DBS_BIAS_Y10, rUpperDBS.bstbl_2.bits.DBS_BIAS_Y10 );
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_BIAS_Y11 = %d, %d, %d", rLowerDBS.bstbl_2.bits.DBS_BIAS_Y11, rSmoothDBS.bstbl_2.bits.DBS_BIAS_Y11, rUpperDBS.bstbl_2.bits.DBS_BIAS_Y11 );
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_BIAS_Y12 = %d, %d, %d", rLowerDBS.bstbl_3.bits.DBS_BIAS_Y12, rSmoothDBS.bstbl_3.bits.DBS_BIAS_Y12, rUpperDBS.bstbl_3.bits.DBS_BIAS_Y12 );
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_HDR_GAIN = %d, %d, %d", rLowerDBS.ctl.bits.DBS_HDR_GAIN, rSmoothDBS.ctl.bits.DBS_HDR_GAIN, rUpperDBS.ctl.bits.DBS_HDR_GAIN     );
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_HDR_GAIN2 = %d, %d, %d", rLowerDBS.ctl.bits.DBS_HDR_GAIN2, rSmoothDBS.ctl.bits.DBS_HDR_GAIN2, rUpperDBS.ctl.bits.DBS_HDR_GAIN2    );
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_HDR_GNP = %d, %d, %d", rLowerDBS.ctl.bits.DBS_HDR_GNP, rSmoothDBS.ctl.bits.DBS_HDR_GNP, rUpperDBS.ctl.bits.DBS_HDR_GNP      );
    MY_LOG_IF(bSmoothDBSDebug, "[L,S,U]DBS_LE_INV_CTL = %d, %d, %d", rLowerDBS.ctl.bits.DBS_LE_INV_CTL, rSmoothDBS.ctl.bits.DBS_LE_INV_CTL, rUpperDBS.ctl.bits.DBS_LE_INV_CTL   );

}

MINT32 InterParam_UDM(MINT32 ISO, MINT32 ISOpre, MINT32 ISOpos, MINT32 PARAMpre, MINT32 PARAMpos, MINT32 Method) {
    MINT32 InterPARAM = 0;

    if(ISOpre == ISOpos) {
        // To prevent someone sets different PARAMpre and PARAMpos
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

MVOID SmoothUDM(MUINT32 u4RealISO, // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_UDM_T const& rUpperUDM, // UDM settings for upper ISO
                ISP_NVRAM_UDM_T const& rLowerUDM, // UDM settings for lower ISO
                ISP_NVRAM_UDM_T& rSmoothUDM)   // Output
{
	MBOOL bSmoothUDMDebug = interpPropertyGet("vendor.debug.smooth_udm.enable", MFALSE);

    MY_LOG_IF(bSmoothUDMDebug,"[%s()] u4RealISO: %d, u4UpperISO = %d, u4LowerISO = %d\n", __FUNCTION__, u4RealISO, u4UpperISO, u4LowerISO);
    MINT32 pre_UDM_BYP = rLowerUDM.intp_crs.bits.UDM_BYP;
    MINT32 pre_UDM_GRAD_WT = rLowerUDM.intp_crs.bits.UDM_GRAD_WT;
    MINT32 pre_UDM_ETH = rLowerUDM.intp_crs.bits.UDM_ETH;
    MINT32 pre_UDM_FTH = rLowerUDM.intp_crs.bits.UDM_FTH;
    MINT32 pre_UDM_DTH = rLowerUDM.intp_crs.bits.UDM_DTH;
    MINT32 pre_UDM_CD_SLC = rLowerUDM.intp_nat.bits.UDM_CD_SLC;
    MINT32 pre_UDM_CD_SLL = rLowerUDM.intp_nat.bits.UDM_CD_SLL;
    MINT32 pre_UDM_L0_OFST = rLowerUDM.intp_nat.bits.UDM_L0_OFST;
    MINT32 pre_UDM_L0_SL = rLowerUDM.intp_nat.bits.UDM_L0_SL;
    MINT32 pre_UDM_L1_OFST = rLowerUDM.intp_aug.bits.UDM_L1_OFST;
    MINT32 pre_UDM_L1_SL = rLowerUDM.intp_aug.bits.UDM_L1_SL;
    MINT32 pre_UDM_L2_OFST = rLowerUDM.intp_aug.bits.UDM_L2_OFST;
    MINT32 pre_UDM_L2_SL = rLowerUDM.intp_aug.bits.UDM_L2_SL;
    MINT32 pre_UDM_DN_OFST = rLowerUDM.intp_aug.bits.UDM_DN_OFST;
    MINT32 pre_UDM_HL_OFST = rLowerUDM.intp_nat.bits.UDM_HL_OFST;
    MINT32 pre_UDM_LM_Y0 = rLowerUDM.luma_lut1.bits.UDM_LM_Y0;
    MINT32 pre_UDM_LM_Y1 = rLowerUDM.luma_lut1.bits.UDM_LM_Y1;
    MINT32 pre_UDM_LM_Y2 = rLowerUDM.luma_lut1.bits.UDM_LM_Y2;
    MINT32 pre_UDM_LM_Y3 = rLowerUDM.luma_lut2.bits.UDM_LM_Y3;
    MINT32 pre_UDM_LM_Y4 = rLowerUDM.luma_lut2.bits.UDM_LM_Y4;
    MINT32 pre_UDM_LM_Y5 = rLowerUDM.luma_lut2.bits.UDM_LM_Y5;
    MINT32 pre_UDM_SL_Y1 = rLowerUDM.sl_ctl.bits.UDM_SL_Y1;
    MINT32 pre_UDM_SL_Y2 = rLowerUDM.sl_ctl.bits.UDM_SL_Y2;
    MINT32 pre_UDM_SL_HR = rLowerUDM.sl_ctl.bits.UDM_SL_HR;
    MINT32 pre_UDM_HT_GN1 = rLowerUDM.hftd_ctl.bits.UDM_HT_GN1;
    MINT32 pre_UDM_HT_GN2 = rLowerUDM.hftd_ctl.bits.UDM_HT_GN2;
    MINT32 pre_UDM_HD_GN1 = rLowerUDM.hftd_ctl.bits.UDM_HD_GN1;
    MINT32 pre_UDM_HD_GN2 = rLowerUDM.hftd_ctl.bits.UDM_HD_GN2;
    MINT32 pre_UDM_XTK_RAT = rLowerUDM.nr_str.bits.UDM_XTK_RAT;
    MINT32 pre_UDM_XTK_OFST = rLowerUDM.nr_str.bits.UDM_XTK_OFST;
    MINT32 pre_UDM_XTK_SL = rLowerUDM.nr_str.bits.UDM_XTK_SL;
    MINT32 pre_UDM_N0_STR = rLowerUDM.nr_str.bits.UDM_N0_STR;
    MINT32 pre_UDM_N1_STR = rLowerUDM.nr_str.bits.UDM_N1_STR;
    MINT32 pre_UDM_N2_STR = rLowerUDM.nr_str.bits.UDM_N2_STR;
    MINT32 pre_UDM_N0_OFST = rLowerUDM.nr_act.bits.UDM_N0_OFST;
    MINT32 pre_UDM_N1_OFST = rLowerUDM.nr_act.bits.UDM_N1_OFST;
    MINT32 pre_UDM_N2_OFST = rLowerUDM.nr_act.bits.UDM_N2_OFST;
    MINT32 pre_UDM_NSL = rLowerUDM.nr_act.bits.UDM_NSL;
    MINT32 pre_UDM_NGR = rLowerUDM.nr_act.bits.UDM_NGR;
    MINT32 pre_UDM_HA_STR = rLowerUDM.hf_str.bits.UDM_HA_STR;
    MINT32 pre_UDM_H1_GN = rLowerUDM.hf_str.bits.UDM_H1_GN;
    MINT32 pre_UDM_H2_GN = rLowerUDM.hf_str.bits.UDM_H2_GN;
    MINT32 pre_UDM_H3_GN = rLowerUDM.hf_str.bits.UDM_H3_GN;
    MINT32 pre_UDM_HI_RAT = rLowerUDM.hf_str.bits.UDM_HI_RAT;
    MINT32 pre_UDM_H1_LWB = rLowerUDM.hf_act1.bits.UDM_H1_LWB;
    MINT32 pre_UDM_H1_UPB = rLowerUDM.hf_act1.bits.UDM_H1_UPB;
    MINT32 pre_UDM_H2_LWB = rLowerUDM.hf_act1.bits.UDM_H2_LWB;
    MINT32 pre_UDM_H2_UPB = rLowerUDM.hf_act1.bits.UDM_H2_UPB;
    MINT32 pre_UDM_H3_LWB = rLowerUDM.hf_act2.bits.UDM_H3_LWB;
    MINT32 pre_UDM_H3_UPB = rLowerUDM.hf_act2.bits.UDM_H3_UPB;
    MINT32 pre_UDM_HSLL = rLowerUDM.hf_act2.bits.UDM_HSLL;
    MINT32 pre_UDM_HSLR = rLowerUDM.hf_act2.bits.UDM_HSLR;
    MINT32 pre_UDM_CORE_TH1 = rLowerUDM.hftd_ctl.bits.UDM_CORE_TH1;
    MINT32 pre_UDM_CORE_TH2 = rLowerUDM.hf_str.bits.UDM_CORE_TH2;
    MINT32 pre_UDM_OV_TH = rLowerUDM.clip.bits.UDM_OV_TH;
    MINT32 pre_UDM_UN_TH = rLowerUDM.clip.bits.UDM_UN_TH;
    MINT32 pre_UDM_CLIP_TH = rLowerUDM.clip.bits.UDM_CLIP_TH;
    MINT32 pre_UDM_DS_THB = rLowerUDM.dsb.bits.UDM_DS_THB;
    MINT32 pre_UDM_DS_SLB = rLowerUDM.dsb.bits.UDM_DS_SLB;
    MINT32 pre_UDM_FL_MODE = rLowerUDM.dsb.bits.UDM_FL_MODE;
    MINT32 pre_UDM_SL_RAT = rLowerUDM.dsb.bits.UDM_SL_RAT;
    MINT32 pre_UDM_SC_RAT = rLowerUDM.dsb.bits.UDM_SC_RAT;

    MINT32 pos_UDM_BYP = rUpperUDM.intp_crs.bits.UDM_BYP;
    MINT32 pos_UDM_GRAD_WT = rUpperUDM.intp_crs.bits.UDM_GRAD_WT;
    MINT32 pos_UDM_ETH = rUpperUDM.intp_crs.bits.UDM_ETH;
    MINT32 pos_UDM_FTH = rUpperUDM.intp_crs.bits.UDM_FTH;
    MINT32 pos_UDM_DTH = rUpperUDM.intp_crs.bits.UDM_DTH;
    MINT32 pos_UDM_CD_SLC = rUpperUDM.intp_nat.bits.UDM_CD_SLC;
    MINT32 pos_UDM_CD_SLL = rUpperUDM.intp_nat.bits.UDM_CD_SLL;
    MINT32 pos_UDM_L0_OFST = rUpperUDM.intp_nat.bits.UDM_L0_OFST;
    MINT32 pos_UDM_L0_SL = rUpperUDM.intp_nat.bits.UDM_L0_SL;
    MINT32 pos_UDM_L1_OFST = rUpperUDM.intp_aug.bits.UDM_L1_OFST;
    MINT32 pos_UDM_L1_SL = rUpperUDM.intp_aug.bits.UDM_L1_SL;
    MINT32 pos_UDM_L2_OFST = rUpperUDM.intp_aug.bits.UDM_L2_OFST;
    MINT32 pos_UDM_L2_SL = rUpperUDM.intp_aug.bits.UDM_L2_SL;
    MINT32 pos_UDM_DN_OFST = rUpperUDM.intp_aug.bits.UDM_DN_OFST;
    MINT32 pos_UDM_HL_OFST = rUpperUDM.intp_nat.bits.UDM_HL_OFST;
    MINT32 pos_UDM_LM_Y0 = rUpperUDM.luma_lut1.bits.UDM_LM_Y0;
    MINT32 pos_UDM_LM_Y1 = rUpperUDM.luma_lut1.bits.UDM_LM_Y1;
    MINT32 pos_UDM_LM_Y2 = rUpperUDM.luma_lut1.bits.UDM_LM_Y2;
    MINT32 pos_UDM_LM_Y3 = rUpperUDM.luma_lut2.bits.UDM_LM_Y3;
    MINT32 pos_UDM_LM_Y4 = rUpperUDM.luma_lut2.bits.UDM_LM_Y4;
    MINT32 pos_UDM_LM_Y5 = rUpperUDM.luma_lut2.bits.UDM_LM_Y5;
    MINT32 pos_UDM_SL_Y1 = rUpperUDM.sl_ctl.bits.UDM_SL_Y1;
    MINT32 pos_UDM_SL_Y2 = rUpperUDM.sl_ctl.bits.UDM_SL_Y2;
    MINT32 pos_UDM_SL_HR = rUpperUDM.sl_ctl.bits.UDM_SL_HR;
    MINT32 pos_UDM_HT_GN1 = rUpperUDM.hftd_ctl.bits.UDM_HT_GN1;
    MINT32 pos_UDM_HT_GN2 = rUpperUDM.hftd_ctl.bits.UDM_HT_GN2;
    MINT32 pos_UDM_HD_GN1 = rUpperUDM.hftd_ctl.bits.UDM_HD_GN1;
    MINT32 pos_UDM_HD_GN2 = rUpperUDM.hftd_ctl.bits.UDM_HD_GN2;
    MINT32 pos_UDM_XTK_RAT = rUpperUDM.nr_str.bits.UDM_XTK_RAT;
    MINT32 pos_UDM_XTK_OFST = rUpperUDM.nr_str.bits.UDM_XTK_OFST;
    MINT32 pos_UDM_XTK_SL = rUpperUDM.nr_str.bits.UDM_XTK_SL;
    MINT32 pos_UDM_N0_STR = rUpperUDM.nr_str.bits.UDM_N0_STR;
    MINT32 pos_UDM_N1_STR = rUpperUDM.nr_str.bits.UDM_N1_STR;
    MINT32 pos_UDM_N2_STR = rUpperUDM.nr_str.bits.UDM_N2_STR;
    MINT32 pos_UDM_N0_OFST = rUpperUDM.nr_act.bits.UDM_N0_OFST;
    MINT32 pos_UDM_N1_OFST = rUpperUDM.nr_act.bits.UDM_N1_OFST;
    MINT32 pos_UDM_N2_OFST = rUpperUDM.nr_act.bits.UDM_N2_OFST;
    MINT32 pos_UDM_NSL = rUpperUDM.nr_act.bits.UDM_NSL;
    MINT32 pos_UDM_NGR = rUpperUDM.nr_act.bits.UDM_NGR;
    MINT32 pos_UDM_HA_STR = rUpperUDM.hf_str.bits.UDM_HA_STR;
    MINT32 pos_UDM_H1_GN = rUpperUDM.hf_str.bits.UDM_H1_GN;
    MINT32 pos_UDM_H2_GN = rUpperUDM.hf_str.bits.UDM_H2_GN;
    MINT32 pos_UDM_H3_GN = rUpperUDM.hf_str.bits.UDM_H3_GN;
    MINT32 pos_UDM_HI_RAT = rUpperUDM.hf_str.bits.UDM_HI_RAT;
    MINT32 pos_UDM_H1_LWB = rUpperUDM.hf_act1.bits.UDM_H1_LWB;
    MINT32 pos_UDM_H1_UPB = rUpperUDM.hf_act1.bits.UDM_H1_UPB;
    MINT32 pos_UDM_H2_LWB = rUpperUDM.hf_act1.bits.UDM_H2_LWB;
    MINT32 pos_UDM_H2_UPB = rUpperUDM.hf_act1.bits.UDM_H2_UPB;
    MINT32 pos_UDM_H3_LWB = rUpperUDM.hf_act2.bits.UDM_H3_LWB;
    MINT32 pos_UDM_H3_UPB = rUpperUDM.hf_act2.bits.UDM_H3_UPB;
    MINT32 pos_UDM_HSLL = rUpperUDM.hf_act2.bits.UDM_HSLL;
    MINT32 pos_UDM_HSLR = rUpperUDM.hf_act2.bits.UDM_HSLR;
    MINT32 pos_UDM_CORE_TH1 = rUpperUDM.hftd_ctl.bits.UDM_CORE_TH1;
    MINT32 pos_UDM_CORE_TH2 = rUpperUDM.hf_str.bits.UDM_CORE_TH2;
    MINT32 pos_UDM_OV_TH = rUpperUDM.clip.bits.UDM_OV_TH;
    MINT32 pos_UDM_UN_TH = rUpperUDM.clip.bits.UDM_UN_TH;
    MINT32 pos_UDM_CLIP_TH = rUpperUDM.clip.bits.UDM_CLIP_TH;
    MINT32 pos_UDM_DS_THB = rUpperUDM.dsb.bits.UDM_DS_THB;
    MINT32 pos_UDM_DS_SLB = rUpperUDM.dsb.bits.UDM_DS_SLB;
    MINT32 pos_UDM_FL_MODE = rUpperUDM.dsb.bits.UDM_FL_MODE;
    MINT32 pos_UDM_SL_RAT = rUpperUDM.dsb.bits.UDM_SL_RAT;
    MINT32 pos_UDM_SC_RAT = rUpperUDM.dsb.bits.UDM_SC_RAT;

    // interpolation //
    MINT32 ISO = u4RealISO;
    MINT32 ISOpre = u4LowerISO;
    MINT32 ISOpos = u4UpperISO;

    MINT32 UDM_BYP = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_BYP, pos_UDM_BYP, 0);
    MINT32 UDM_GRAD_WT = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_GRAD_WT, pos_UDM_GRAD_WT, 0);
    MINT32 UDM_ETH = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_ETH, pos_UDM_ETH, 0);
    MINT32 UDM_FTH = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_FTH, pos_UDM_FTH, 0);
    MINT32 UDM_DTH = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_DTH, pos_UDM_DTH, 0);
    MINT32 UDM_CD_SLC = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_CD_SLC, pos_UDM_CD_SLC, 0);
    MINT32 UDM_CD_SLL = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_CD_SLL, pos_UDM_CD_SLL, 0);
    MINT32 UDM_L0_OFST = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_L0_OFST, pos_UDM_L0_OFST, 0);
    MINT32 UDM_L0_SL = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_L0_SL, pos_UDM_L0_SL, 0);
    MINT32 UDM_L1_OFST = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_L1_OFST, pos_UDM_L1_OFST, 0);
    MINT32 UDM_L1_SL = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_L1_SL, pos_UDM_L1_SL, 0);
    MINT32 UDM_L2_OFST = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_L2_OFST, pos_UDM_L2_OFST, 0);
    MINT32 UDM_L2_SL = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_L2_SL, pos_UDM_L2_SL, 0);
    MINT32 UDM_DN_OFST = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_DN_OFST, pos_UDM_DN_OFST, 0);
    MINT32 UDM_HL_OFST = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_HL_OFST, pos_UDM_HL_OFST, 0);
    MINT32 UDM_LM_Y0 = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_LM_Y0, pos_UDM_LM_Y0, 0);
    MINT32 UDM_LM_Y1 = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_LM_Y1, pos_UDM_LM_Y1, 0);
    MINT32 UDM_LM_Y2 = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_LM_Y2, pos_UDM_LM_Y2, 0);
    MINT32 UDM_LM_Y3 = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_LM_Y3, pos_UDM_LM_Y3, 0);
    MINT32 UDM_LM_Y4 = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_LM_Y4, pos_UDM_LM_Y4, 0);
    MINT32 UDM_LM_Y5 = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_LM_Y5, pos_UDM_LM_Y5, 0);
    MINT32 UDM_SL_Y1 = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_SL_Y1, pos_UDM_SL_Y1, 0);
    MINT32 UDM_SL_Y2 = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_SL_Y2, pos_UDM_SL_Y2, 0);
    MINT32 UDM_SL_HR = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_SL_HR, pos_UDM_SL_HR, 0);
    MINT32 UDM_HT_GN1 = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_HT_GN1, pos_UDM_HT_GN1, 0);
    MINT32 UDM_HT_GN2 = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_HT_GN2, pos_UDM_HT_GN2, 0);
    MINT32 UDM_HD_GN1 = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_HD_GN1, pos_UDM_HD_GN1, 0);
    MINT32 UDM_HD_GN2 = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_HD_GN2, pos_UDM_HD_GN2, 0);
    MINT32 UDM_XTK_RAT = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_XTK_RAT, pos_UDM_XTK_RAT, 0);
    MINT32 UDM_XTK_OFST = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_XTK_OFST, pos_UDM_XTK_OFST, 0);
    MINT32 UDM_XTK_SL = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_XTK_SL, pos_UDM_XTK_SL, 0);
    MINT32 UDM_N0_STR = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_N0_STR, pos_UDM_N0_STR, 0);
    MINT32 UDM_N1_STR = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_N1_STR, pos_UDM_N1_STR, 0);
    MINT32 UDM_N2_STR = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_N2_STR, pos_UDM_N2_STR, 0);
    MINT32 UDM_N0_OFST = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_N0_OFST, pos_UDM_N0_OFST, 0);
    MINT32 UDM_N1_OFST = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_N1_OFST, pos_UDM_N1_OFST, 0);
    MINT32 UDM_N2_OFST = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_N2_OFST, pos_UDM_N2_OFST, 0);
    MINT32 UDM_NSL = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_NSL, pos_UDM_NSL, 0);
    MINT32 UDM_NGR = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_NGR, pos_UDM_NGR, 0);
    MINT32 UDM_HA_STR = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_HA_STR, pos_UDM_HA_STR, 0);
    MINT32 UDM_H1_GN = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_H1_GN, pos_UDM_H1_GN, 0);
    MINT32 UDM_H2_GN = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_H2_GN, pos_UDM_H2_GN, 0);
    MINT32 UDM_H3_GN = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_H3_GN, pos_UDM_H3_GN, 0);
    MINT32 UDM_HI_RAT = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_HI_RAT, pos_UDM_HI_RAT, 0);
    MINT32 UDM_H1_LWB = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_H1_LWB, pos_UDM_H1_LWB, 0);
    MINT32 UDM_H1_UPB = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_H1_UPB, pos_UDM_H1_UPB, 0);
    MINT32 UDM_H2_LWB = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_H2_LWB, pos_UDM_H2_LWB, 0);
    MINT32 UDM_H2_UPB = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_H2_UPB, pos_UDM_H2_UPB, 0);
    MINT32 UDM_H3_LWB = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_H3_LWB, pos_UDM_H3_LWB, 0);
    MINT32 UDM_H3_UPB = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_H3_UPB, pos_UDM_H3_UPB, 0);
    MINT32 UDM_HSLL = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_HSLL, pos_UDM_HSLL, 0);
    MINT32 UDM_HSLR = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_HSLR, pos_UDM_HSLR, 0);
    MINT32 UDM_CORE_TH1 = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_CORE_TH1, pos_UDM_CORE_TH1, 0);
    MINT32 UDM_CORE_TH2 = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_CORE_TH2, pos_UDM_CORE_TH2, 0);
    MINT32 UDM_OV_TH = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_OV_TH, pos_UDM_OV_TH, 0);
    MINT32 UDM_UN_TH = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_UN_TH, pos_UDM_UN_TH, 0);
    MINT32 UDM_CLIP_TH = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_CLIP_TH, pos_UDM_CLIP_TH, 0);
    MINT32 UDM_DS_THB = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_DS_THB, pos_UDM_DS_THB, 0);
    MINT32 UDM_DS_SLB = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_DS_SLB, pos_UDM_DS_SLB, 0);
    MINT32 UDM_FL_MODE = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_FL_MODE, pos_UDM_FL_MODE, 0);
    MINT32 UDM_SL_RAT = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_SL_RAT, pos_UDM_SL_RAT, 0);
    MINT32 UDM_SC_RAT = InterParam_UDM(ISO, ISOpre, ISOpos, pre_UDM_SC_RAT, pos_UDM_SC_RAT, 0);

    rSmoothUDM.intp_crs.bits.UDM_BYP = UDM_BYP;
    rSmoothUDM.intp_crs.bits.UDM_GRAD_WT = UDM_GRAD_WT;
    rSmoothUDM.intp_crs.bits.UDM_ETH = UDM_ETH;
    rSmoothUDM.intp_crs.bits.UDM_FTH = UDM_FTH;
    rSmoothUDM.intp_crs.bits.UDM_DTH = UDM_DTH;
    rSmoothUDM.intp_nat.bits.UDM_CD_SLC = UDM_CD_SLC;
    rSmoothUDM.intp_nat.bits.UDM_CD_SLL = UDM_CD_SLL;
    rSmoothUDM.intp_nat.bits.UDM_L0_OFST = UDM_L0_OFST;
    rSmoothUDM.intp_nat.bits.UDM_L0_SL = UDM_L0_SL;
    rSmoothUDM.intp_aug.bits.UDM_L1_OFST = UDM_L1_OFST;
    rSmoothUDM.intp_aug.bits.UDM_L1_SL = UDM_L1_SL;
    rSmoothUDM.intp_aug.bits.UDM_L2_OFST = UDM_L2_OFST;
    rSmoothUDM.intp_aug.bits.UDM_L2_SL = UDM_L2_SL;
    rSmoothUDM.intp_aug.bits.UDM_DN_OFST = UDM_DN_OFST;
    rSmoothUDM.intp_nat.bits.UDM_HL_OFST = UDM_HL_OFST;
    rSmoothUDM.luma_lut1.bits.UDM_LM_Y0 = UDM_LM_Y0;
    rSmoothUDM.luma_lut1.bits.UDM_LM_Y1 = UDM_LM_Y1;
    rSmoothUDM.luma_lut1.bits.UDM_LM_Y2 = UDM_LM_Y2;
    rSmoothUDM.luma_lut2.bits.UDM_LM_Y3 = UDM_LM_Y3;
    rSmoothUDM.luma_lut2.bits.UDM_LM_Y4 = UDM_LM_Y4;
    rSmoothUDM.luma_lut2.bits.UDM_LM_Y5 = UDM_LM_Y5;
    rSmoothUDM.sl_ctl.bits.UDM_SL_Y1 = UDM_SL_Y1;
    rSmoothUDM.sl_ctl.bits.UDM_SL_Y2 = UDM_SL_Y2;
    rSmoothUDM.sl_ctl.bits.UDM_SL_HR = UDM_SL_HR;
    rSmoothUDM.hftd_ctl.bits.UDM_HT_GN1 = UDM_HT_GN1;
    rSmoothUDM.hftd_ctl.bits.UDM_HT_GN2 = UDM_HT_GN2;
    rSmoothUDM.hftd_ctl.bits.UDM_HD_GN1 = UDM_HD_GN1;
    rSmoothUDM.hftd_ctl.bits.UDM_HD_GN2 = UDM_HD_GN2;
    rSmoothUDM.nr_str.bits.UDM_XTK_RAT = UDM_XTK_RAT;
    rSmoothUDM.nr_str.bits.UDM_XTK_OFST = UDM_XTK_OFST;
    rSmoothUDM.nr_str.bits.UDM_XTK_SL = UDM_XTK_SL;
    rSmoothUDM.nr_str.bits.UDM_N0_STR = UDM_N0_STR;
    rSmoothUDM.nr_str.bits.UDM_N1_STR = UDM_N1_STR;
    rSmoothUDM.nr_str.bits.UDM_N2_STR = UDM_N2_STR;
    rSmoothUDM.nr_act.bits.UDM_N0_OFST = UDM_N0_OFST;
    rSmoothUDM.nr_act.bits.UDM_N1_OFST = UDM_N1_OFST;
    rSmoothUDM.nr_act.bits.UDM_N2_OFST = UDM_N2_OFST;
    rSmoothUDM.nr_act.bits.UDM_NSL = UDM_NSL;
    rSmoothUDM.nr_act.bits.UDM_NGR = UDM_NGR;
    rSmoothUDM.hf_str.bits.UDM_HA_STR = UDM_HA_STR;
    rSmoothUDM.hf_str.bits.UDM_H1_GN = UDM_H1_GN;
    rSmoothUDM.hf_str.bits.UDM_H2_GN = UDM_H2_GN;
    rSmoothUDM.hf_str.bits.UDM_H3_GN = UDM_H3_GN;
    rSmoothUDM.hf_str.bits.UDM_HI_RAT = UDM_HI_RAT;
    rSmoothUDM.hf_act1.bits.UDM_H1_LWB = UDM_H1_LWB;
    rSmoothUDM.hf_act1.bits.UDM_H1_UPB = UDM_H1_UPB;
    rSmoothUDM.hf_act1.bits.UDM_H2_LWB = UDM_H2_LWB;
    rSmoothUDM.hf_act1.bits.UDM_H2_UPB = UDM_H2_UPB;
    rSmoothUDM.hf_act2.bits.UDM_H3_LWB = UDM_H3_LWB;
    rSmoothUDM.hf_act2.bits.UDM_H3_UPB = UDM_H3_UPB;
    rSmoothUDM.hf_act2.bits.UDM_HSLL = UDM_HSLL;
    rSmoothUDM.hf_act2.bits.UDM_HSLR = UDM_HSLR;
    rSmoothUDM.hftd_ctl.bits.UDM_CORE_TH1 = UDM_CORE_TH1;
    rSmoothUDM.hf_str.bits.UDM_CORE_TH2 = UDM_CORE_TH2;
    rSmoothUDM.clip.bits.UDM_OV_TH = UDM_OV_TH;
    rSmoothUDM.clip.bits.UDM_UN_TH = UDM_UN_TH;
    rSmoothUDM.clip.bits.UDM_CLIP_TH = UDM_CLIP_TH;
    rSmoothUDM.dsb.bits.UDM_DS_THB = UDM_DS_THB;
    rSmoothUDM.dsb.bits.UDM_DS_SLB = UDM_DS_SLB;
    rSmoothUDM.dsb.bits.UDM_FL_MODE = UDM_FL_MODE;
    rSmoothUDM.dsb.bits.UDM_SL_RAT = UDM_SL_RAT;
    rSmoothUDM.dsb.bits.UDM_SC_RAT = UDM_SC_RAT;

    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_BYP = %d, %d, %d", rLowerUDM.intp_crs.bits.UDM_BYP, rSmoothUDM.intp_crs.bits.UDM_BYP, rUpperUDM.intp_crs.bits.UDM_BYP     );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_GRAD_WT = %d, %d, %d", rLowerUDM.intp_crs.bits.UDM_GRAD_WT, rSmoothUDM.intp_crs.bits.UDM_GRAD_WT, rUpperUDM.intp_crs.bits.UDM_GRAD_WT );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_ETH = %d, %d, %d", rLowerUDM.intp_crs.bits.UDM_ETH, rSmoothUDM.intp_crs.bits.UDM_ETH, rUpperUDM.intp_crs.bits.UDM_ETH     );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_FTH = %d, %d, %d", rLowerUDM.intp_crs.bits.UDM_FTH, rSmoothUDM.intp_crs.bits.UDM_FTH, rUpperUDM.intp_crs.bits.UDM_FTH     );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_DTH = %d, %d, %d", rLowerUDM.intp_crs.bits.UDM_DTH, rSmoothUDM.intp_crs.bits.UDM_DTH, rUpperUDM.intp_crs.bits.UDM_DTH     );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_CD_SLC = %d, %d, %d", rLowerUDM.intp_nat.bits.UDM_CD_SLC, rSmoothUDM.intp_nat.bits.UDM_CD_SLC, rUpperUDM.intp_nat.bits.UDM_CD_SLC  );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_CD_SLL = %d, %d, %d", rLowerUDM.intp_nat.bits.UDM_CD_SLL, rSmoothUDM.intp_nat.bits.UDM_CD_SLL, rUpperUDM.intp_nat.bits.UDM_CD_SLL  );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_L0_OFST = %d, %d, %d", rLowerUDM.intp_nat.bits.UDM_L0_OFST, rSmoothUDM.intp_nat.bits.UDM_L0_OFST, rUpperUDM.intp_nat.bits.UDM_L0_OFST );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_L0_SL = %d, %d, %d", rLowerUDM.intp_nat.bits.UDM_L0_SL, rSmoothUDM.intp_nat.bits.UDM_L0_SL, rUpperUDM.intp_nat.bits.UDM_L0_SL   );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_L1_OFST = %d, %d, %d", rLowerUDM.intp_aug.bits.UDM_L1_OFST, rSmoothUDM.intp_aug.bits.UDM_L1_OFST, rUpperUDM.intp_aug.bits.UDM_L1_OFST );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_L1_SL = %d, %d, %d", rLowerUDM.intp_aug.bits.UDM_L1_SL, rSmoothUDM.intp_aug.bits.UDM_L1_SL, rUpperUDM.intp_aug.bits.UDM_L1_SL   );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_L2_OFST = %d, %d, %d", rLowerUDM.intp_aug.bits.UDM_L2_OFST, rSmoothUDM.intp_aug.bits.UDM_L2_OFST, rUpperUDM.intp_aug.bits.UDM_L2_OFST );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_L2_SL = %d, %d, %d", rLowerUDM.intp_aug.bits.UDM_L2_SL, rSmoothUDM.intp_aug.bits.UDM_L2_SL, rUpperUDM.intp_aug.bits.UDM_L2_SL   );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_DN_OFST = %d, %d, %d", rLowerUDM.intp_aug.bits.UDM_DN_OFST, rSmoothUDM.intp_aug.bits.UDM_DN_OFST, rUpperUDM.intp_aug.bits.UDM_DN_OFST );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_HL_OFST = %d, %d, %d", rLowerUDM.intp_nat.bits.UDM_HL_OFST, rSmoothUDM.intp_nat.bits.UDM_HL_OFST, rUpperUDM.intp_nat.bits.UDM_HL_OFST );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_LM_Y0 = %d, %d, %d", rLowerUDM.luma_lut1.bits.UDM_LM_Y0, rSmoothUDM.luma_lut1.bits.UDM_LM_Y0, rUpperUDM.luma_lut1.bits.UDM_LM_Y0  );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_LM_Y1 = %d, %d, %d", rLowerUDM.luma_lut1.bits.UDM_LM_Y1, rSmoothUDM.luma_lut1.bits.UDM_LM_Y1, rUpperUDM.luma_lut1.bits.UDM_LM_Y1  );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_LM_Y2 = %d, %d, %d", rLowerUDM.luma_lut1.bits.UDM_LM_Y2, rSmoothUDM.luma_lut1.bits.UDM_LM_Y2, rUpperUDM.luma_lut1.bits.UDM_LM_Y2  );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_LM_Y3 = %d, %d, %d", rLowerUDM.luma_lut2.bits.UDM_LM_Y3, rSmoothUDM.luma_lut2.bits.UDM_LM_Y3, rUpperUDM.luma_lut2.bits.UDM_LM_Y3  );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_LM_Y4 = %d, %d, %d", rLowerUDM.luma_lut2.bits.UDM_LM_Y4, rSmoothUDM.luma_lut2.bits.UDM_LM_Y4, rUpperUDM.luma_lut2.bits.UDM_LM_Y4  );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_LM_Y5 = %d, %d, %d", rLowerUDM.luma_lut2.bits.UDM_LM_Y5, rSmoothUDM.luma_lut2.bits.UDM_LM_Y5, rUpperUDM.luma_lut2.bits.UDM_LM_Y5  );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_SL_Y1 = %d, %d, %d", rLowerUDM.sl_ctl.bits.UDM_SL_Y1, rSmoothUDM.sl_ctl.bits.UDM_SL_Y1, rUpperUDM.sl_ctl.bits.UDM_SL_Y1     );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_SL_Y2 = %d, %d, %d", rLowerUDM.sl_ctl.bits.UDM_SL_Y2, rSmoothUDM.sl_ctl.bits.UDM_SL_Y2, rUpperUDM.sl_ctl.bits.UDM_SL_Y2     );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_SL_HR = %d, %d, %d", rLowerUDM.sl_ctl.bits.UDM_SL_HR, rSmoothUDM.sl_ctl.bits.UDM_SL_HR, rUpperUDM.sl_ctl.bits.UDM_SL_HR     );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_HT_GN1 = %d, %d, %d", rLowerUDM.hftd_ctl.bits.UDM_HT_GN1, rSmoothUDM.hftd_ctl.bits.UDM_HT_GN1, rUpperUDM.hftd_ctl.bits.UDM_HT_GN1  );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_HT_GN2 = %d, %d, %d", rLowerUDM.hftd_ctl.bits.UDM_HT_GN2, rSmoothUDM.hftd_ctl.bits.UDM_HT_GN2, rUpperUDM.hftd_ctl.bits.UDM_HT_GN2  );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_HD_GN1 = %d, %d, %d", rLowerUDM.hftd_ctl.bits.UDM_HD_GN1, rSmoothUDM.hftd_ctl.bits.UDM_HD_GN1, rUpperUDM.hftd_ctl.bits.UDM_HD_GN1  );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_HD_GN2 = %d, %d, %d", rLowerUDM.hftd_ctl.bits.UDM_HD_GN2, rSmoothUDM.hftd_ctl.bits.UDM_HD_GN2, rUpperUDM.hftd_ctl.bits.UDM_HD_GN2  );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_XTK_RAT = %d, %d, %d", rLowerUDM.nr_str.bits.UDM_XTK_RAT, rSmoothUDM.nr_str.bits.UDM_XTK_RAT, rUpperUDM.nr_str.bits.UDM_XTK_RAT   );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_XTK_OFST = %d, %d, %d", rLowerUDM.nr_str.bits.UDM_XTK_OFST, rSmoothUDM.nr_str.bits.UDM_XTK_OFST, rUpperUDM.nr_str.bits.UDM_XTK_OFST   );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_XTK_SL = %d, %d, %d", rLowerUDM.nr_str.bits.UDM_XTK_SL, rSmoothUDM.nr_str.bits.UDM_XTK_SL, rUpperUDM.nr_str.bits.UDM_XTK_SL    );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_N0_STR = %d, %d, %d", rLowerUDM.nr_str.bits.UDM_N0_STR, rSmoothUDM.nr_str.bits.UDM_N0_STR, rUpperUDM.nr_str.bits.UDM_N0_STR    );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_N1_STR = %d, %d, %d", rLowerUDM.nr_str.bits.UDM_N1_STR, rSmoothUDM.nr_str.bits.UDM_N1_STR, rUpperUDM.nr_str.bits.UDM_N1_STR    );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_N2_STR = %d, %d, %d", rLowerUDM.nr_str.bits.UDM_N2_STR, rSmoothUDM.nr_str.bits.UDM_N2_STR, rUpperUDM.nr_str.bits.UDM_N2_STR    );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_N0_OFST = %d, %d, %d", rLowerUDM.nr_act.bits.UDM_N0_OFST, rSmoothUDM.nr_act.bits.UDM_N0_OFST, rUpperUDM.nr_act.bits.UDM_N0_OFST   );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_N1_OFST = %d, %d, %d", rLowerUDM.nr_act.bits.UDM_N1_OFST, rSmoothUDM.nr_act.bits.UDM_N1_OFST, rUpperUDM.nr_act.bits.UDM_N1_OFST   );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_N2_OFST = %d, %d, %d", rLowerUDM.nr_act.bits.UDM_N2_OFST, rSmoothUDM.nr_act.bits.UDM_N2_OFST, rUpperUDM.nr_act.bits.UDM_N2_OFST   );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_NSL = %d, %d, %d", rLowerUDM.nr_act.bits.UDM_NSL, rSmoothUDM.nr_act.bits.UDM_NSL, rUpperUDM.nr_act.bits.UDM_NSL       );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_NGR = %d, %d, %d", rLowerUDM.nr_act.bits.UDM_NGR, rSmoothUDM.nr_act.bits.UDM_NGR, rUpperUDM.nr_act.bits.UDM_NGR       );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_HA_STR = %d, %d, %d", rLowerUDM.hf_str.bits.UDM_HA_STR, rSmoothUDM.hf_str.bits.UDM_HA_STR, rUpperUDM.hf_str.bits.UDM_HA_STR    );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_H1_GN = %d, %d, %d", rLowerUDM.hf_str.bits.UDM_H1_GN, rSmoothUDM.hf_str.bits.UDM_H1_GN, rUpperUDM.hf_str.bits.UDM_H1_GN     );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_H2_GN = %d, %d, %d", rLowerUDM.hf_str.bits.UDM_H2_GN, rSmoothUDM.hf_str.bits.UDM_H2_GN, rUpperUDM.hf_str.bits.UDM_H2_GN     );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_H3_GN = %d, %d, %d", rLowerUDM.hf_str.bits.UDM_H3_GN, rSmoothUDM.hf_str.bits.UDM_H3_GN, rUpperUDM.hf_str.bits.UDM_H3_GN     );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_HI_RAT = %d, %d, %d", rLowerUDM.hf_str.bits.UDM_HI_RAT, rSmoothUDM.hf_str.bits.UDM_HI_RAT, rUpperUDM.hf_str.bits.UDM_HI_RAT    );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_H1_LWB = %d, %d, %d", rLowerUDM.hf_act1.bits.UDM_H1_LWB, rSmoothUDM.hf_act1.bits.UDM_H1_LWB, rUpperUDM.hf_act1.bits.UDM_H1_LWB   );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_H1_UPB = %d, %d, %d", rLowerUDM.hf_act1.bits.UDM_H1_UPB, rSmoothUDM.hf_act1.bits.UDM_H1_UPB, rUpperUDM.hf_act1.bits.UDM_H1_UPB   );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_H2_LWB = %d, %d, %d", rLowerUDM.hf_act1.bits.UDM_H2_LWB, rSmoothUDM.hf_act1.bits.UDM_H2_LWB, rUpperUDM.hf_act1.bits.UDM_H2_LWB   );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_H2_UPB = %d, %d, %d", rLowerUDM.hf_act1.bits.UDM_H2_UPB, rSmoothUDM.hf_act1.bits.UDM_H2_UPB, rUpperUDM.hf_act1.bits.UDM_H2_UPB   );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_H3_LWB = %d, %d, %d", rLowerUDM.hf_act2.bits.UDM_H3_LWB, rSmoothUDM.hf_act2.bits.UDM_H3_LWB, rUpperUDM.hf_act2.bits.UDM_H3_LWB   );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_H3_UPB = %d, %d, %d", rLowerUDM.hf_act2.bits.UDM_H3_UPB, rSmoothUDM.hf_act2.bits.UDM_H3_UPB, rUpperUDM.hf_act2.bits.UDM_H3_UPB   );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_HSLL = %d, %d, %d", rLowerUDM.hf_act2.bits.UDM_HSLL, rSmoothUDM.hf_act2.bits.UDM_HSLL, rUpperUDM.hf_act2.bits.UDM_HSLL     );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_HSLR = %d, %d, %d", rLowerUDM.hf_act2.bits.UDM_HSLR, rSmoothUDM.hf_act2.bits.UDM_HSLR, rUpperUDM.hf_act2.bits.UDM_HSLR     );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_CORE_TH1 = %d, %d, %d", rLowerUDM.hftd_ctl.bits.UDM_CORE_TH1, rSmoothUDM.hftd_ctl.bits.UDM_CORE_TH1, rUpperUDM.hftd_ctl.bits.UDM_CORE_TH1);
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_CORE_TH2 = %d, %d, %d", rLowerUDM.hf_str.bits.UDM_CORE_TH2, rSmoothUDM.hf_str.bits.UDM_CORE_TH2, rUpperUDM.hf_str.bits.UDM_CORE_TH2  );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_OV_TH = %d, %d, %d", rLowerUDM.clip.bits.UDM_OV_TH, rSmoothUDM.clip.bits.UDM_OV_TH, rUpperUDM.clip.bits.UDM_OV_TH       );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_UN_TH = %d, %d, %d", rLowerUDM.clip.bits.UDM_UN_TH, rSmoothUDM.clip.bits.UDM_UN_TH, rUpperUDM.clip.bits.UDM_UN_TH       );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_CLIP_TH = %d, %d, %d", rLowerUDM.clip.bits.UDM_CLIP_TH, rSmoothUDM.clip.bits.UDM_CLIP_TH, rUpperUDM.clip.bits.UDM_CLIP_TH     );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_DS_THB = %d, %d, %d", rLowerUDM.dsb.bits.UDM_DS_THB, rSmoothUDM.dsb.bits.UDM_DS_THB, rUpperUDM.dsb.bits.UDM_DS_THB       );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_DS_SLB = %d, %d, %d", rLowerUDM.dsb.bits.UDM_DS_SLB, rSmoothUDM.dsb.bits.UDM_DS_SLB, rUpperUDM.dsb.bits.UDM_DS_SLB       );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_FL_MODE = %d, %d, %d", rLowerUDM.dsb.bits.UDM_FL_MODE, rSmoothUDM.dsb.bits.UDM_FL_MODE, rUpperUDM.dsb.bits.UDM_FL_MODE      );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_SL_RAT = %d, %d, %d", rLowerUDM.dsb.bits.UDM_SL_RAT, rSmoothUDM.dsb.bits.UDM_SL_RAT, rUpperUDM.dsb.bits.UDM_SL_RAT       );
    MY_LOG_IF(bSmoothUDMDebug, "[L,S,U]UDM_SC_RAT = %d, %d, %d", rLowerUDM.dsb.bits.UDM_SC_RAT, rSmoothUDM.dsb.bits.UDM_SC_RAT, rUpperUDM.dsb.bits.UDM_SC_RAT       );

}


// "return value":    Interpolated register value
// "ISO":            ISO value of current image
// "ISOpre":        A pre-defined ISO (100, 200, 400, ...) which is the closest but smaller than or equal to "ISO"
// "ISOpos":        A pre-defined ISO (100, 200, 400, ...) which is the closest but larger than or equal to "ISO"
// "PARAMpre":        Corresponding register value from "ISOpre"
// "PARAMpos":        Corresponding register value from "ISOpos"
// "Method":        0 for linear interpolation, 1 for closest one
MINT32 InterParam_EE(MINT32 ISO, MINT32 ISOpre, MINT32 ISOpos, MINT32 PARAMpre, MINT32 PARAMpos, MINT32 Method){
    MINT32 InterPARAM = 0;

    if(ISOpre == ISOpos){
        // To prevent someone sets different PARAMpre and PARAMpos
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


MVOID SmoothEE(MUINT32 u4RealISO, // Real ISO
               MUINT32 u4UpperISO, // Upper ISO
               MUINT32 u4LowerISO, // Lower ISO
               ISP_NVRAM_EE_T const& rUpperEE, // EE settings for upper ISO
               ISP_NVRAM_EE_T const& rLowerEE, // EE settings for lower ISO
               ISP_NVRAM_EE_T& rSmoothEE)   // Output
{
	MBOOL bSmoothEEDebug = interpPropertyGet("vendor.debug.smooth_ee.enable", MFALSE);

    MY_LOG_IF(bSmoothEEDebug,"[%s()] u4RealISO: %d, u4UpperISO = %d, u4LowerISO = %d\n", __FUNCTION__, u4RealISO, u4UpperISO, u4LowerISO);

    // ====================================================================================================================================
    // Get Lower ISO setting
    // ====================================================================================================================================
    MINT32 SEEE_OVRSH_CLIP_ENpre = rLowerEE.clip_ctrl.bits.SEEE_OVRSH_CLIP_EN;
    MINT32 SEEE_OVRSH_CLIP_STRpre = rLowerEE.clip_ctrl.bits.SEEE_OVRSH_CLIP_STR;
    MINT32 SEEE_DOT_REDUC_ENpre = rLowerEE.clip_ctrl.bits.SEEE_DOT_REDUC_EN;
    MINT32 SEEE_DOT_WFpre = rLowerEE.clip_ctrl.bits.SEEE_DOT_WF;
    MINT32 SEEE_DOT_THpre = rLowerEE.clip_ctrl.bits.SEEE_DOT_TH;
    MINT32 SEEE_RESP_CLIPpre = rLowerEE.clip_ctrl.bits.SEEE_RESP_CLIP;
    MINT32 SEEE_FLT_G1pre = rLowerEE.flt_ctrl_2.bits.SEEE_FLT_G1;
    MINT32 SEEE_FLT_G2pre = rLowerEE.flt_ctrl_2.bits.SEEE_FLT_G2;
    MINT32 SEEE_FLT_G3pre = rLowerEE.flt_ctrl_2.bits.SEEE_FLT_G3;
    MINT32 SEEE_FLT_G4pre = rLowerEE.flt_ctrl_2.bits.SEEE_FLT_G4;
    MINT32 SEEE_FLT_G5pre = rLowerEE.flt_ctrl_2.bits.SEEE_FLT_G5;
    MINT32 SEEE_FLT_G6pre = rLowerEE.flt_ctrl_2.bits.SEEE_FLT_G6;
    MINT32 SEEE_FLT_THpre = rLowerEE.flt_ctrl_1.bits.SEEE_FLT_TH;
    MINT32 SEEE_FLT_AMPpre = rLowerEE.flt_ctrl_1.bits.SEEE_FLT_AMP;
    MINT32 SEEE_GLUT_X1pre = rLowerEE.glut_ctrl_01.bits.SEEE_GLUT_X1;
    MINT32 SEEE_GLUT_S1pre = rLowerEE.glut_ctrl_01.bits.SEEE_GLUT_S1;
    MINT32 SEEE_GLUT_Y1pre = rLowerEE.glut_ctrl_01.bits.SEEE_GLUT_Y1;
    MINT32 SEEE_GLUT_X2pre = rLowerEE.glut_ctrl_02.bits.SEEE_GLUT_X2;
    MINT32 SEEE_GLUT_S2pre = rLowerEE.glut_ctrl_02.bits.SEEE_GLUT_S2;
    MINT32 SEEE_GLUT_Y2pre = rLowerEE.glut_ctrl_02.bits.SEEE_GLUT_Y2;
    MINT32 SEEE_GLUT_X3pre = rLowerEE.glut_ctrl_03.bits.SEEE_GLUT_X3;
    MINT32 SEEE_GLUT_S3pre = rLowerEE.glut_ctrl_03.bits.SEEE_GLUT_S3;
    MINT32 SEEE_GLUT_Y3pre = rLowerEE.glut_ctrl_03.bits.SEEE_GLUT_Y3;
    MINT32 SEEE_GLUT_X4pre = rLowerEE.glut_ctrl_04.bits.SEEE_GLUT_X4;
    MINT32 SEEE_GLUT_S4pre = rLowerEE.glut_ctrl_04.bits.SEEE_GLUT_S4;
    MINT32 SEEE_GLUT_Y4pre = rLowerEE.glut_ctrl_04.bits.SEEE_GLUT_Y4;
    MINT32 SEEE_GLUT_S5pre = rLowerEE.glut_ctrl_05.bits.SEEE_GLUT_S5;
    MINT32 SEEE_GLUT_TH_OVRpre = rLowerEE.glut_ctrl_06.bits.SEEE_GLUT_TH_OVR;
    MINT32 SEEE_GLUT_TH_UNDpre = rLowerEE.glut_ctrl_06.bits.SEEE_GLUT_TH_UND;
    MINT32 SEEE_GLUT_TH_MINpre = rLowerEE.glut_ctrl_06.bits.SEEE_GLUT_TH_MIN;
    MINT32 SEEE_EDTR_DIAG_AMPpre = rLowerEE.edtr_ctrl.bits.SEEE_EDTR_DIAG_AMP;
    MINT32 SEEE_EDTR_AMPpre = rLowerEE.edtr_ctrl.bits.SEEE_EDTR_AMP;
    MINT32 SEEE_EDTR_LVpre = rLowerEE.edtr_ctrl.bits.SEEE_EDTR_LV;
    MINT32 SEEE_EDTR_FLT_MODEpre = rLowerEE.edtr_ctrl.bits.SEEE_EDTR_FLT_MODE;
    MINT32 SEEE_EDTR_FLT_2_ENpre = rLowerEE.edtr_ctrl.bits.SEEE_EDTR_FLT_2_EN;
    MINT32 SEEE_GLUT_X1_1pre = rLowerEE.glut_ctrl_07.bits.SEEE_GLUT_X1_1;
    MINT32 SEEE_GLUT_S1_1pre = rLowerEE.glut_ctrl_07.bits.SEEE_GLUT_S1_1;
    MINT32 SEEE_GLUT_S2_1pre = rLowerEE.glut_ctrl_07.bits.SEEE_GLUT_S2_1;
    MINT32 SEEE_GLUT_LLINK_ENpre = rLowerEE.glut_ctrl_07.bits.SEEE_GLUT_LLINK_EN;
    MINT32 SEEE_GLUT_X1_2pre = rLowerEE.glut_ctrl_08.bits.SEEE_GLUT_X1_2;
    MINT32 SEEE_GLUT_S1_2pre = rLowerEE.glut_ctrl_08.bits.SEEE_GLUT_S1_2;
    MINT32 SEEE_GLUT_S2_2pre = rLowerEE.glut_ctrl_08.bits.SEEE_GLUT_S2_2;
    MINT32 SEEE_GLUT_X1_3pre = rLowerEE.glut_ctrl_09.bits.SEEE_GLUT_X1_3;
    MINT32 SEEE_GLUT_S1_3pre = rLowerEE.glut_ctrl_09.bits.SEEE_GLUT_S1_3;
    MINT32 SEEE_GLUT_S2_3pre = rLowerEE.glut_ctrl_09.bits.SEEE_GLUT_S2_3;
    MINT32 SEEE_GLUT_TH_OVR_1pre = rLowerEE.glut_ctrl_10.bits.SEEE_GLUT_TH_OVR_1;
    MINT32 SEEE_GLUT_TH_UND_1pre = rLowerEE.glut_ctrl_10.bits.SEEE_GLUT_TH_UND_1;
    MINT32 SEEE_GLUT_TH_OVR_2pre = rLowerEE.glut_ctrl_10.bits.SEEE_GLUT_TH_OVR_2;
    MINT32 SEEE_GLUT_TH_UND_2pre = rLowerEE.glut_ctrl_10.bits.SEEE_GLUT_TH_UND_2;
    MINT32 SEEE_GLUT_TH_OVR_3pre = rLowerEE.glut_ctrl_11.bits.SEEE_GLUT_TH_OVR_3;
    MINT32 SEEE_GLUT_TH_UND_3pre = rLowerEE.glut_ctrl_11.bits.SEEE_GLUT_TH_UND_3;

    // ====================================================================================================================================
    // Get Upper ISO setting
    // ====================================================================================================================================
    MINT32 SEEE_OVRSH_CLIP_ENpos = rUpperEE.clip_ctrl.bits.SEEE_OVRSH_CLIP_EN;
    MINT32 SEEE_OVRSH_CLIP_STRpos = rUpperEE.clip_ctrl.bits.SEEE_OVRSH_CLIP_STR;
    MINT32 SEEE_DOT_REDUC_ENpos = rUpperEE.clip_ctrl.bits.SEEE_DOT_REDUC_EN;
    MINT32 SEEE_DOT_WFpos = rUpperEE.clip_ctrl.bits.SEEE_DOT_WF;
    MINT32 SEEE_DOT_THpos = rUpperEE.clip_ctrl.bits.SEEE_DOT_TH;
    MINT32 SEEE_RESP_CLIPpos = rUpperEE.clip_ctrl.bits.SEEE_RESP_CLIP;
    MINT32 SEEE_FLT_G1pos = rUpperEE.flt_ctrl_2.bits.SEEE_FLT_G1;
    MINT32 SEEE_FLT_G2pos = rUpperEE.flt_ctrl_2.bits.SEEE_FLT_G2;
    MINT32 SEEE_FLT_G3pos = rUpperEE.flt_ctrl_2.bits.SEEE_FLT_G3;
    MINT32 SEEE_FLT_G4pos = rUpperEE.flt_ctrl_2.bits.SEEE_FLT_G4;
    MINT32 SEEE_FLT_G5pos = rUpperEE.flt_ctrl_2.bits.SEEE_FLT_G5;
    MINT32 SEEE_FLT_G6pos = rUpperEE.flt_ctrl_2.bits.SEEE_FLT_G6;
    MINT32 SEEE_FLT_THpos = rUpperEE.flt_ctrl_1.bits.SEEE_FLT_TH;
    MINT32 SEEE_FLT_AMPpos = rUpperEE.flt_ctrl_1.bits.SEEE_FLT_AMP;
    MINT32 SEEE_GLUT_X1pos = rUpperEE.glut_ctrl_01.bits.SEEE_GLUT_X1;
    MINT32 SEEE_GLUT_S1pos = rUpperEE.glut_ctrl_01.bits.SEEE_GLUT_S1;
    MINT32 SEEE_GLUT_Y1pos = rUpperEE.glut_ctrl_01.bits.SEEE_GLUT_Y1;
    MINT32 SEEE_GLUT_X2pos = rUpperEE.glut_ctrl_02.bits.SEEE_GLUT_X2;
    MINT32 SEEE_GLUT_S2pos = rUpperEE.glut_ctrl_02.bits.SEEE_GLUT_S2;
    MINT32 SEEE_GLUT_Y2pos = rUpperEE.glut_ctrl_02.bits.SEEE_GLUT_Y2;
    MINT32 SEEE_GLUT_X3pos = rUpperEE.glut_ctrl_03.bits.SEEE_GLUT_X3;
    MINT32 SEEE_GLUT_S3pos = rUpperEE.glut_ctrl_03.bits.SEEE_GLUT_S3;
    MINT32 SEEE_GLUT_Y3pos = rUpperEE.glut_ctrl_03.bits.SEEE_GLUT_Y3;
    MINT32 SEEE_GLUT_X4pos = rUpperEE.glut_ctrl_04.bits.SEEE_GLUT_X4;
    MINT32 SEEE_GLUT_S4pos = rUpperEE.glut_ctrl_04.bits.SEEE_GLUT_S4;
    MINT32 SEEE_GLUT_Y4pos = rUpperEE.glut_ctrl_04.bits.SEEE_GLUT_Y4;
    MINT32 SEEE_GLUT_S5pos = rUpperEE.glut_ctrl_05.bits.SEEE_GLUT_S5;
    MINT32 SEEE_GLUT_TH_OVRpos = rUpperEE.glut_ctrl_06.bits.SEEE_GLUT_TH_OVR;
    MINT32 SEEE_GLUT_TH_UNDpos = rUpperEE.glut_ctrl_06.bits.SEEE_GLUT_TH_UND;
    MINT32 SEEE_GLUT_TH_MINpos = rUpperEE.glut_ctrl_06.bits.SEEE_GLUT_TH_MIN;
    MINT32 SEEE_EDTR_DIAG_AMPpos = rUpperEE.edtr_ctrl.bits.SEEE_EDTR_DIAG_AMP;
    MINT32 SEEE_EDTR_AMPpos = rUpperEE.edtr_ctrl.bits.SEEE_EDTR_AMP;
    MINT32 SEEE_EDTR_LVpos = rUpperEE.edtr_ctrl.bits.SEEE_EDTR_LV;
    MINT32 SEEE_EDTR_FLT_MODEpos = rUpperEE.edtr_ctrl.bits.SEEE_EDTR_FLT_MODE;
    MINT32 SEEE_EDTR_FLT_2_ENpos = rUpperEE.edtr_ctrl.bits.SEEE_EDTR_FLT_2_EN;
    MINT32 SEEE_GLUT_X1_1pos = rUpperEE.glut_ctrl_07.bits.SEEE_GLUT_X1_1;
    MINT32 SEEE_GLUT_S1_1pos = rUpperEE.glut_ctrl_07.bits.SEEE_GLUT_S1_1;
    MINT32 SEEE_GLUT_S2_1pos = rUpperEE.glut_ctrl_07.bits.SEEE_GLUT_S2_1;
    MINT32 SEEE_GLUT_LLINK_ENpos = rUpperEE.glut_ctrl_07.bits.SEEE_GLUT_LLINK_EN;
    MINT32 SEEE_GLUT_X1_2pos = rUpperEE.glut_ctrl_08.bits.SEEE_GLUT_X1_2;
    MINT32 SEEE_GLUT_S1_2pos = rUpperEE.glut_ctrl_08.bits.SEEE_GLUT_S1_2;
    MINT32 SEEE_GLUT_S2_2pos = rUpperEE.glut_ctrl_08.bits.SEEE_GLUT_S2_2;
    MINT32 SEEE_GLUT_X1_3pos = rUpperEE.glut_ctrl_09.bits.SEEE_GLUT_X1_3;
    MINT32 SEEE_GLUT_S1_3pos = rUpperEE.glut_ctrl_09.bits.SEEE_GLUT_S1_3;
    MINT32 SEEE_GLUT_S2_3pos = rUpperEE.glut_ctrl_09.bits.SEEE_GLUT_S2_3;
    MINT32 SEEE_GLUT_TH_OVR_1pos = rUpperEE.glut_ctrl_10.bits.SEEE_GLUT_TH_OVR_1;
    MINT32 SEEE_GLUT_TH_UND_1pos = rUpperEE.glut_ctrl_10.bits.SEEE_GLUT_TH_UND_1;
    MINT32 SEEE_GLUT_TH_OVR_2pos = rUpperEE.glut_ctrl_10.bits.SEEE_GLUT_TH_OVR_2;
    MINT32 SEEE_GLUT_TH_UND_2pos = rUpperEE.glut_ctrl_10.bits.SEEE_GLUT_TH_UND_2;
    MINT32 SEEE_GLUT_TH_OVR_3pos = rUpperEE.glut_ctrl_11.bits.SEEE_GLUT_TH_OVR_3;
    MINT32 SEEE_GLUT_TH_UND_3pos = rUpperEE.glut_ctrl_11.bits.SEEE_GLUT_TH_UND_3;

    // Negative value for slope
    SEEE_GLUT_S1pre = (SEEE_GLUT_S1pre > 127) ? (SEEE_GLUT_S1pre - 256) : SEEE_GLUT_S1pre;
    SEEE_GLUT_S2pre = (SEEE_GLUT_S2pre > 127) ? (SEEE_GLUT_S2pre - 256) : SEEE_GLUT_S2pre;
    SEEE_GLUT_S3pre = (SEEE_GLUT_S3pre > 127) ? (SEEE_GLUT_S3pre - 256) : SEEE_GLUT_S3pre;
    SEEE_GLUT_S4pre = (SEEE_GLUT_S4pre > 127) ? (SEEE_GLUT_S4pre - 256) : SEEE_GLUT_S4pre;
    SEEE_GLUT_S5pre = (SEEE_GLUT_S5pre > 127) ? (SEEE_GLUT_S5pre - 256) : SEEE_GLUT_S5pre;

    SEEE_GLUT_S1_1pre = (SEEE_GLUT_S1_1pre > 127) ? (SEEE_GLUT_S1_1pre - 256) : SEEE_GLUT_S1_1pre;
    SEEE_GLUT_S1_2pre = (SEEE_GLUT_S1_2pre > 127) ? (SEEE_GLUT_S1_2pre - 256) : SEEE_GLUT_S1_2pre;
    SEEE_GLUT_S1_3pre = (SEEE_GLUT_S1_3pre > 127) ? (SEEE_GLUT_S1_3pre - 256) : SEEE_GLUT_S1_3pre;
    SEEE_GLUT_S2_1pre = (SEEE_GLUT_S2_1pre > 127) ? (SEEE_GLUT_S2_1pre - 256) : SEEE_GLUT_S2_1pre;
    SEEE_GLUT_S2_2pre = (SEEE_GLUT_S2_2pre > 127) ? (SEEE_GLUT_S2_2pre - 256) : SEEE_GLUT_S2_2pre;
    SEEE_GLUT_S2_3pre = (SEEE_GLUT_S2_3pre > 127) ? (SEEE_GLUT_S2_3pre - 256) : SEEE_GLUT_S2_3pre;

    SEEE_GLUT_S1pos = (SEEE_GLUT_S1pos > 127) ? (SEEE_GLUT_S1pos - 256) : SEEE_GLUT_S1pos;
    SEEE_GLUT_S2pos = (SEEE_GLUT_S2pos > 127) ? (SEEE_GLUT_S2pos - 256) : SEEE_GLUT_S2pos;
    SEEE_GLUT_S3pos = (SEEE_GLUT_S3pos > 127) ? (SEEE_GLUT_S3pos - 256) : SEEE_GLUT_S3pos;
    SEEE_GLUT_S4pos = (SEEE_GLUT_S4pos > 127) ? (SEEE_GLUT_S4pos - 256) : SEEE_GLUT_S4pos;
    SEEE_GLUT_S5pos = (SEEE_GLUT_S5pos > 127) ? (SEEE_GLUT_S5pos - 256) : SEEE_GLUT_S5pos;

    SEEE_GLUT_S1_1pos = (SEEE_GLUT_S1_1pos > 127) ? (SEEE_GLUT_S1_1pos - 256) : SEEE_GLUT_S1_1pos;
    SEEE_GLUT_S1_2pos = (SEEE_GLUT_S1_2pos > 127) ? (SEEE_GLUT_S1_2pos - 256) : SEEE_GLUT_S1_2pos;
    SEEE_GLUT_S1_3pos = (SEEE_GLUT_S1_3pos > 127) ? (SEEE_GLUT_S1_3pos - 256) : SEEE_GLUT_S1_3pos;
    SEEE_GLUT_S2_1pos = (SEEE_GLUT_S2_1pos > 127) ? (SEEE_GLUT_S2_1pos - 256) : SEEE_GLUT_S2_1pos;
    SEEE_GLUT_S2_2pos = (SEEE_GLUT_S2_2pos > 127) ? (SEEE_GLUT_S2_2pos - 256) : SEEE_GLUT_S2_2pos;
    SEEE_GLUT_S2_3pos = (SEEE_GLUT_S2_3pos > 127) ? (SEEE_GLUT_S2_3pos - 256) : SEEE_GLUT_S2_3pos;
    // ====================================================================================================================================
    // Start Parameter Interpolation
    // ====================================================================================================================================
    MINT32 ISO = u4RealISO;
    MINT32 ISOpre = u4LowerISO;
    MINT32 ISOpos = u4UpperISO;

    // Registers that can not be interpolated
    MINT32 SEEE_EDTR_FLT_MODE = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_EDTR_FLT_MODEpre, SEEE_EDTR_FLT_MODEpos, 1);
    MINT32 SEEE_FLT_G1 = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_FLT_G1pre, SEEE_FLT_G1pos, 1);
    MINT32 SEEE_FLT_G2 = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_FLT_G2pre, SEEE_FLT_G2pos, 1);
    MINT32 SEEE_FLT_G3 = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_FLT_G3pre, SEEE_FLT_G3pos, 1);
    MINT32 SEEE_FLT_G4 = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_FLT_G4pre, SEEE_FLT_G4pos, 1);
    MINT32 SEEE_FLT_G5 = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_FLT_G5pre, SEEE_FLT_G5pos, 1);
    MINT32 SEEE_FLT_G6 = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_FLT_G6pre, SEEE_FLT_G6pos, 1);
    MINT32 SEEE_FLT_TH = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_FLT_THpre, SEEE_FLT_THpos, 1);
    MINT32 SEEE_FLT_AMP = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_FLT_AMPpre, SEEE_FLT_AMPpos, 1);

    // Registers that can be interpolated
    MINT32 SEEE_RESP_CLIP = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_RESP_CLIPpre, SEEE_RESP_CLIPpos, 0);
    MINT32 SEEE_EDTR_LV = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_EDTR_LVpre, SEEE_EDTR_LVpos, 0);
    MINT32 SEEE_EDTR_DIAG_AMP = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_EDTR_DIAG_AMPpre, SEEE_EDTR_DIAG_AMPpos, 0);
    MINT32 SEEE_EDTR_AMP = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_EDTR_AMPpre, SEEE_EDTR_AMPpos, 0);
    MINT32 SEEE_EDTR_FLT_2_EN = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_EDTR_FLT_2_ENpre, SEEE_EDTR_FLT_2_ENpos, 0);
    MINT32 SEEE_OVRSH_CLIP_EN = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_OVRSH_CLIP_ENpre, SEEE_OVRSH_CLIP_ENpos, 0);
    MINT32 SEEE_OVRSH_CLIP_STR = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_OVRSH_CLIP_STRpre, SEEE_OVRSH_CLIP_STRpos, 0);
    MINT32 SEEE_DOT_REDUC_EN = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_DOT_REDUC_ENpre, SEEE_DOT_REDUC_ENpos, 0);
    MINT32 SEEE_DOT_TH = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_DOT_THpre, SEEE_DOT_THpos, 0);
    MINT32 SEEE_DOT_WF = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_DOT_WFpre, SEEE_DOT_WFpos, 0);
    MINT32 SEEE_GLUT_X1 = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_GLUT_X1pre, SEEE_GLUT_X1pos, 0);
    MINT32 SEEE_GLUT_X2 = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_GLUT_X2pre, SEEE_GLUT_X2pos, 0);
    MINT32 SEEE_GLUT_X3 = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_GLUT_X3pre, SEEE_GLUT_X3pos, 0);
    MINT32 SEEE_GLUT_X4 = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_GLUT_X4pre, SEEE_GLUT_X4pos, 0);
    MINT32 SEEE_GLUT_Y1 = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_GLUT_Y1pre, SEEE_GLUT_Y1pos, 0);
    MINT32 SEEE_GLUT_Y2 = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_GLUT_Y2pre, SEEE_GLUT_Y2pos, 0);
    MINT32 SEEE_GLUT_Y3 = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_GLUT_Y3pre, SEEE_GLUT_Y3pos, 0);
    MINT32 SEEE_GLUT_Y4 = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_GLUT_Y4pre, SEEE_GLUT_Y4pos, 0);
    MINT32 SEEE_GLUT_TH_OVR = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_GLUT_TH_OVRpre, SEEE_GLUT_TH_OVRpos, 0);
    MINT32 SEEE_GLUT_TH_UND = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_GLUT_TH_UNDpre, SEEE_GLUT_TH_UNDpos, 0);
    MINT32 SEEE_GLUT_TH_MIN = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_GLUT_TH_MINpre, SEEE_GLUT_TH_MINpos, 0);
    MINT32 SEEE_GLUT_LLINK_EN = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_GLUT_LLINK_ENpre, SEEE_GLUT_LLINK_ENpos, 0);
    MINT32 SEEE_GLUT_X1_1 = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_GLUT_X1_1pre, SEEE_GLUT_X1_1pos, 0);
    MINT32 SEEE_GLUT_X1_2 = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_GLUT_X1_2pre, SEEE_GLUT_X1_2pos, 0);
    MINT32 SEEE_GLUT_X1_3 = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_GLUT_X1_3pre, SEEE_GLUT_X1_3pos, 0);
    MINT32 SEEE_GLUT_TH_OVR_1 = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_GLUT_TH_OVR_1pre, SEEE_GLUT_TH_OVR_1pos, 0);
    MINT32 SEEE_GLUT_TH_OVR_2 = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_GLUT_TH_OVR_2pre, SEEE_GLUT_TH_OVR_2pos, 0);
    MINT32 SEEE_GLUT_TH_OVR_3 = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_GLUT_TH_OVR_3pre, SEEE_GLUT_TH_OVR_3pos, 0);
    MINT32 SEEE_GLUT_TH_UND_1 = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_GLUT_TH_UND_1pre, SEEE_GLUT_TH_UND_1pos, 0);
    MINT32 SEEE_GLUT_TH_UND_2 = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_GLUT_TH_UND_2pre, SEEE_GLUT_TH_UND_2pos, 0);
    MINT32 SEEE_GLUT_TH_UND_3 = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_GLUT_TH_UND_3pre, SEEE_GLUT_TH_UND_3pos, 0);

    // To calculate S5, we need to find Y5 first.
    // Y5 is not a real register. So we use (X4, Y4) and S5 to evaluate (255, Y5) for pre-ISO and post-ISO setting.
    // Then we get the interpolated Y5.
    MINT32 SEEE_GLUT_Y5pre = SEEE_GLUT_Y4pre + SEEE_GLUT_S5pre * (255 - SEEE_GLUT_X4pre);
    MINT32 SEEE_GLUT_Y5pos = SEEE_GLUT_Y4pos + SEEE_GLUT_S5pos * (255 - SEEE_GLUT_X4pos);
    MINT32 SEEE_GLUT_Y5 = InterParam_EE(ISO, ISOpre, ISOpos, SEEE_GLUT_Y5pre, SEEE_GLUT_Y5pos, 0);

    MINT32 SEEE_GLUT_S1;
    MINT32 SEEE_GLUT_S1_1;
    MINT32 SEEE_GLUT_S1_2;
    MINT32 SEEE_GLUT_S1_3;
    MINT32 SEEE_GLUT_S2;
    MINT32 SEEE_GLUT_S2_1;
    MINT32 SEEE_GLUT_S2_2;
    MINT32 SEEE_GLUT_S2_3;
    MINT32 SEEE_GLUT_S3;
    MINT32 SEEE_GLUT_S4;
    MINT32 SEEE_GLUT_S5;
    // Due to slope can not be interpolated for most cases, they need to be re-calculated.
    if(SEEE_GLUT_X1 == 0){
        SEEE_GLUT_S1 = 0;
    }
    else{
        MINT32 S1 = (MINT32)((double)(SEEE_GLUT_Y1) / (double)SEEE_GLUT_X1 + 0.5);
        SEEE_GLUT_S1 = (S1 > 127) ? 127 : S1;
    }

    // EE_LCE_Link
    if(SEEE_GLUT_X1_1 == 0){
        SEEE_GLUT_S1_1 = 0;
    }
    else{
        MINT32 S1_1 = (MINT32)((double)(SEEE_GLUT_Y1) / (double)SEEE_GLUT_X1_1 + 0.5);
        SEEE_GLUT_S1_1 = (S1_1 > 127) ? 127 : S1_1;
    }
    if(SEEE_GLUT_X1_2 == 0){
        SEEE_GLUT_S1_2 = 0;
    }
    else{
        MINT32 S1_2 = (MINT32)((double)(SEEE_GLUT_Y1) / (double)SEEE_GLUT_X1_2 + 0.5);
        SEEE_GLUT_S1_2 = (S1_2 > 127) ? 127 : S1_2;
    }
    if(SEEE_GLUT_X1_3 == 0){
        SEEE_GLUT_S1_3 = 0;
    }
    else{
        MINT32 S1_3 = (MINT32)((double)(SEEE_GLUT_Y1) / (double)SEEE_GLUT_X1_3 + 0.5);
        SEEE_GLUT_S1_3 = (S1_3 > 127) ? 127 : S1_3;
    }

    if((SEEE_GLUT_Y2 - SEEE_GLUT_Y1) > 0){
        MINT32 S2 = (MINT32)((double)(SEEE_GLUT_Y2 - SEEE_GLUT_Y1) / (double)(SEEE_GLUT_X2 - SEEE_GLUT_X1) + 0.5);
        SEEE_GLUT_S2 = (S2 > 127) ? 127 : S2;
        // EE_LCE_Link
        MINT32 S2_1 = (MINT32)((double)(SEEE_GLUT_Y2 - SEEE_GLUT_Y1) / (double)(SEEE_GLUT_X2 - SEEE_GLUT_X1_1) + 0.5);
        SEEE_GLUT_S2_1 = (S2_1 > 127) ? 127 : S2_1;
        MINT32 S2_2 = (MINT32)((double)(SEEE_GLUT_Y2 - SEEE_GLUT_Y1) / (double)(SEEE_GLUT_X2 - SEEE_GLUT_X1_2) + 0.5);
        SEEE_GLUT_S2_2 = (S2_2 > 127) ? 127 : S2_2;
        MINT32 S2_3 = (MINT32)((double)(SEEE_GLUT_Y2 - SEEE_GLUT_Y1) / (double)(SEEE_GLUT_X2 - SEEE_GLUT_X1_3) + 0.5);
        SEEE_GLUT_S2_3 = (S2_3 > 127) ? 127 : S2_3;
    }
    else{
        MINT32 S2 = (MINT32)((double)(SEEE_GLUT_Y2 - SEEE_GLUT_Y1) / (double)(SEEE_GLUT_X2 - SEEE_GLUT_X1) - 0.5);
        SEEE_GLUT_S2 = (S2 < -128) ? -128 : S2;
        // EE_LCE_Link
        MINT32 S2_1 = (MINT32)((double)(SEEE_GLUT_Y2 - SEEE_GLUT_Y1) / (double)(SEEE_GLUT_X2 - SEEE_GLUT_X1_1) - 0.5);
        SEEE_GLUT_S2_1 = (S2_1 < -128) ? -128 : S2_1;
        MINT32 S2_2 = (MINT32)((double)(SEEE_GLUT_Y2 - SEEE_GLUT_Y1) / (double)(SEEE_GLUT_X2 - SEEE_GLUT_X1_2) - 0.5);
        SEEE_GLUT_S2_2 = (S2_2 < -128) ? -128 : S2_2;
        MINT32 S2_3 = (MINT32)((double)(SEEE_GLUT_Y2 - SEEE_GLUT_Y1) / (double)(SEEE_GLUT_X2 - SEEE_GLUT_X1_3) - 0.5);
        SEEE_GLUT_S2_3 = (S2_3 < -128) ? -128 : S2_3;
    }

    if((SEEE_GLUT_Y3 - SEEE_GLUT_Y2) > 0){
        MINT32 S3 = (MINT32)((double)(SEEE_GLUT_Y3 - SEEE_GLUT_Y2) / (double)(SEEE_GLUT_X3 - SEEE_GLUT_X2) + 0.5);
        SEEE_GLUT_S3 = (S3 > 127) ? 127 : S3;
    }
    else{
        MINT32 S3 = (MINT32)((double)(SEEE_GLUT_Y3 - SEEE_GLUT_Y2) / (double)(SEEE_GLUT_X3 - SEEE_GLUT_X2) - 0.5);
        SEEE_GLUT_S3 = (S3 < -128) ? -128 : S3;
    }

    if((SEEE_GLUT_Y4 - SEEE_GLUT_Y3) > 0){
        MINT32 S4 = (MINT32)((double)(SEEE_GLUT_Y4 - SEEE_GLUT_Y3) / (double)(SEEE_GLUT_X4 - SEEE_GLUT_X3) + 0.5);
        SEEE_GLUT_S4 = (S4 > 127) ? 127 : S4;
    }
    else{
        MINT32 S4 = (MINT32)((double)(SEEE_GLUT_Y4 - SEEE_GLUT_Y3) / (double)(SEEE_GLUT_X4 - SEEE_GLUT_X3) - 0.5);
        SEEE_GLUT_S4 = (S4 < -128) ? -128 : S4;
    }

    if((SEEE_GLUT_Y5 - SEEE_GLUT_Y4) > 0){
        MINT32 S5 = (MINT32)((double)(SEEE_GLUT_Y5 - SEEE_GLUT_Y4) / (double)(255 - SEEE_GLUT_X4) + 0.5);
        SEEE_GLUT_S5 = (S5 > 127) ? 127 : S5;
    }
    else{
        MINT32 S5 = (MINT32)((double)(SEEE_GLUT_Y5 - SEEE_GLUT_Y4) / (double)(255 - SEEE_GLUT_X4) - 0.5);
        SEEE_GLUT_S5 = (S5 < -128) ? -128 : S5;
    }

    // ====================================================================================================================================
    // Set Smooth ISO setting
    // ====================================================================================================================================
    rSmoothEE.clip_ctrl.bits.SEEE_OVRSH_CLIP_EN = SEEE_OVRSH_CLIP_EN;
    rSmoothEE.clip_ctrl.bits.SEEE_OVRSH_CLIP_STR = SEEE_OVRSH_CLIP_STR;
    rSmoothEE.clip_ctrl.bits.SEEE_DOT_REDUC_EN = SEEE_DOT_REDUC_EN;
    rSmoothEE.clip_ctrl.bits.SEEE_DOT_WF = SEEE_DOT_WF;
    rSmoothEE.clip_ctrl.bits.SEEE_DOT_TH = SEEE_DOT_TH;
    rSmoothEE.clip_ctrl.bits.SEEE_RESP_CLIP = SEEE_RESP_CLIP;
    rSmoothEE.flt_ctrl_1.bits.SEEE_FLT_TH = SEEE_FLT_TH;
    rSmoothEE.flt_ctrl_1.bits.SEEE_FLT_AMP = SEEE_FLT_AMP;
    rSmoothEE.flt_ctrl_2.bits.SEEE_FLT_G1 = SEEE_FLT_G1;
    rSmoothEE.flt_ctrl_2.bits.SEEE_FLT_G2 = SEEE_FLT_G2;
    rSmoothEE.flt_ctrl_2.bits.SEEE_FLT_G3 = SEEE_FLT_G3;
    rSmoothEE.flt_ctrl_2.bits.SEEE_FLT_G4 = SEEE_FLT_G4;
    rSmoothEE.flt_ctrl_2.bits.SEEE_FLT_G5 = SEEE_FLT_G5;
    rSmoothEE.flt_ctrl_2.bits.SEEE_FLT_G6 = SEEE_FLT_G6;
    rSmoothEE.glut_ctrl_01.bits.SEEE_GLUT_X1 = SEEE_GLUT_X1;
    rSmoothEE.glut_ctrl_01.bits.SEEE_GLUT_S1 = SEEE_GLUT_S1;
    rSmoothEE.glut_ctrl_01.bits.SEEE_GLUT_Y1 = SEEE_GLUT_Y1;
    rSmoothEE.glut_ctrl_02.bits.SEEE_GLUT_X2 = SEEE_GLUT_X2;
    rSmoothEE.glut_ctrl_02.bits.SEEE_GLUT_S2 = SEEE_GLUT_S2;
    rSmoothEE.glut_ctrl_02.bits.SEEE_GLUT_Y2 = SEEE_GLUT_Y2;
    rSmoothEE.glut_ctrl_03.bits.SEEE_GLUT_X3 = SEEE_GLUT_X3;
    rSmoothEE.glut_ctrl_03.bits.SEEE_GLUT_S3 = SEEE_GLUT_S3;
    rSmoothEE.glut_ctrl_03.bits.SEEE_GLUT_Y3 = SEEE_GLUT_Y3;
    rSmoothEE.glut_ctrl_04.bits.SEEE_GLUT_X4 = SEEE_GLUT_X4;
    rSmoothEE.glut_ctrl_04.bits.SEEE_GLUT_S4 = SEEE_GLUT_S4;
    rSmoothEE.glut_ctrl_04.bits.SEEE_GLUT_Y4 = SEEE_GLUT_Y4;
    rSmoothEE.glut_ctrl_05.bits.SEEE_GLUT_S5 = SEEE_GLUT_S5;
    rSmoothEE.glut_ctrl_06.bits.SEEE_GLUT_TH_OVR = SEEE_GLUT_TH_OVR;
    rSmoothEE.glut_ctrl_06.bits.SEEE_GLUT_TH_UND = SEEE_GLUT_TH_UND;
    rSmoothEE.glut_ctrl_06.bits.SEEE_GLUT_TH_MIN = SEEE_GLUT_TH_MIN;
    rSmoothEE.edtr_ctrl.bits.SEEE_EDTR_DIAG_AMP = SEEE_EDTR_DIAG_AMP;
    rSmoothEE.edtr_ctrl.bits.SEEE_EDTR_AMP = SEEE_EDTR_AMP;
    rSmoothEE.edtr_ctrl.bits.SEEE_EDTR_LV = SEEE_EDTR_LV;
    rSmoothEE.edtr_ctrl.bits.SEEE_EDTR_FLT_MODE = SEEE_EDTR_FLT_MODE;
    rSmoothEE.edtr_ctrl.bits.SEEE_EDTR_FLT_2_EN = SEEE_EDTR_FLT_2_EN;
    rSmoothEE.glut_ctrl_07.bits.SEEE_GLUT_X1_1 = SEEE_GLUT_X1_1;
    rSmoothEE.glut_ctrl_07.bits.SEEE_GLUT_S1_1 = SEEE_GLUT_S1_1;
    rSmoothEE.glut_ctrl_07.bits.SEEE_GLUT_S2_1 = SEEE_GLUT_S2_1;
    rSmoothEE.glut_ctrl_07.bits.SEEE_GLUT_LLINK_EN = SEEE_GLUT_LLINK_EN;
    rSmoothEE.glut_ctrl_08.bits.SEEE_GLUT_X1_2 = SEEE_GLUT_X1_2;
    rSmoothEE.glut_ctrl_08.bits.SEEE_GLUT_S1_2 = SEEE_GLUT_S1_2;
    rSmoothEE.glut_ctrl_08.bits.SEEE_GLUT_S2_2 = SEEE_GLUT_S2_2;
    rSmoothEE.glut_ctrl_09.bits.SEEE_GLUT_X1_3 = SEEE_GLUT_X1_3;
    rSmoothEE.glut_ctrl_09.bits.SEEE_GLUT_S1_3 = SEEE_GLUT_S1_3;
    rSmoothEE.glut_ctrl_09.bits.SEEE_GLUT_S2_3 = SEEE_GLUT_S2_3;
    rSmoothEE.glut_ctrl_10.bits.SEEE_GLUT_TH_OVR_1 = SEEE_GLUT_TH_OVR_1;
    rSmoothEE.glut_ctrl_10.bits.SEEE_GLUT_TH_UND_1 = SEEE_GLUT_TH_UND_1;
    rSmoothEE.glut_ctrl_10.bits.SEEE_GLUT_TH_OVR_2 = SEEE_GLUT_TH_OVR_2;
    rSmoothEE.glut_ctrl_10.bits.SEEE_GLUT_TH_UND_2 = SEEE_GLUT_TH_UND_2;
    rSmoothEE.glut_ctrl_11.bits.SEEE_GLUT_TH_OVR_3 = SEEE_GLUT_TH_OVR_3;
    rSmoothEE.glut_ctrl_11.bits.SEEE_GLUT_TH_UND_3 = SEEE_GLUT_TH_UND_3;

    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_OVRSH_CLIP_EN = %d, %d, %d", rLowerEE.clip_ctrl.bits.SEEE_OVRSH_CLIP_EN, rSmoothEE.clip_ctrl.bits.SEEE_OVRSH_CLIP_EN, rUpperEE.clip_ctrl.bits.SEEE_OVRSH_CLIP_EN   );
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_OVRSH_CLIP_STR = %d, %d, %d", rLowerEE.clip_ctrl.bits.SEEE_OVRSH_CLIP_STR, rSmoothEE.clip_ctrl.bits.SEEE_OVRSH_CLIP_STR, rUpperEE.clip_ctrl.bits.SEEE_OVRSH_CLIP_STR  );
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_DOT_REDUC_EN = %d, %d, %d", rLowerEE.clip_ctrl.bits.SEEE_DOT_REDUC_EN, rSmoothEE.clip_ctrl.bits.SEEE_DOT_REDUC_EN, rUpperEE.clip_ctrl.bits.SEEE_DOT_REDUC_EN    );
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_DOT_WF = %d, %d, %d", rLowerEE.clip_ctrl.bits.SEEE_DOT_WF, rSmoothEE.clip_ctrl.bits.SEEE_DOT_WF, rUpperEE.clip_ctrl.bits.SEEE_DOT_WF          );
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_DOT_TH = %d, %d, %d", rLowerEE.clip_ctrl.bits.SEEE_DOT_TH, rSmoothEE.clip_ctrl.bits.SEEE_DOT_TH, rUpperEE.clip_ctrl.bits.SEEE_DOT_TH          );
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_RESP_CLIP = %d, %d, %d", rLowerEE.clip_ctrl.bits.SEEE_RESP_CLIP, rSmoothEE.clip_ctrl.bits.SEEE_RESP_CLIP, rUpperEE.clip_ctrl.bits.SEEE_RESP_CLIP       );
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_FLT_TH = %d, %d, %d", rLowerEE.flt_ctrl_1.bits.SEEE_FLT_TH, rSmoothEE.flt_ctrl_1.bits.SEEE_FLT_TH, rUpperEE.flt_ctrl_1.bits.SEEE_FLT_TH         );
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_FLT_AMP = %d, %d, %d", rLowerEE.flt_ctrl_1.bits.SEEE_FLT_AMP, rSmoothEE.flt_ctrl_1.bits.SEEE_FLT_AMP, rUpperEE.flt_ctrl_1.bits.SEEE_FLT_AMP        );
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_FLT_G1 = %d, %d, %d", rLowerEE.flt_ctrl_2.bits.SEEE_FLT_G1, rSmoothEE.flt_ctrl_2.bits.SEEE_FLT_G1, rUpperEE.flt_ctrl_2.bits.SEEE_FLT_G1         );
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_FLT_G2 = %d, %d, %d", rLowerEE.flt_ctrl_2.bits.SEEE_FLT_G2, rSmoothEE.flt_ctrl_2.bits.SEEE_FLT_G2, rUpperEE.flt_ctrl_2.bits.SEEE_FLT_G2         );
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_FLT_G3 = %d, %d, %d", rLowerEE.flt_ctrl_2.bits.SEEE_FLT_G3, rSmoothEE.flt_ctrl_2.bits.SEEE_FLT_G3, rUpperEE.flt_ctrl_2.bits.SEEE_FLT_G3         );
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_FLT_G4 = %d, %d, %d", rLowerEE.flt_ctrl_2.bits.SEEE_FLT_G4, rSmoothEE.flt_ctrl_2.bits.SEEE_FLT_G4, rUpperEE.flt_ctrl_2.bits.SEEE_FLT_G4         );
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_FLT_G5 = %d, %d, %d", rLowerEE.flt_ctrl_2.bits.SEEE_FLT_G5, rSmoothEE.flt_ctrl_2.bits.SEEE_FLT_G5, rUpperEE.flt_ctrl_2.bits.SEEE_FLT_G5         );
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_FLT_G6 = %d, %d, %d", rLowerEE.flt_ctrl_2.bits.SEEE_FLT_G6, rSmoothEE.flt_ctrl_2.bits.SEEE_FLT_G6, rUpperEE.flt_ctrl_2.bits.SEEE_FLT_G6         );
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_GLUT_X1 = %d, %d, %d", rLowerEE.glut_ctrl_01.bits.SEEE_GLUT_X1, rSmoothEE.glut_ctrl_01.bits.SEEE_GLUT_X1, rUpperEE.glut_ctrl_01.bits.SEEE_GLUT_X1      );
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_GLUT_S1 = %d, %d, %d", SEEE_GLUT_S1pre, SEEE_GLUT_S1, SEEE_GLUT_S1pos                              );
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_GLUT_Y1 = %d, %d, %d", rLowerEE.glut_ctrl_01.bits.SEEE_GLUT_Y1, rSmoothEE.glut_ctrl_01.bits.SEEE_GLUT_Y1, rUpperEE.glut_ctrl_01.bits.SEEE_GLUT_Y1      );
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_GLUT_X2 = %d, %d, %d", rLowerEE.glut_ctrl_02.bits.SEEE_GLUT_X2, rSmoothEE.glut_ctrl_02.bits.SEEE_GLUT_X2, rUpperEE.glut_ctrl_02.bits.SEEE_GLUT_X2      );
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_GLUT_S2 = %d, %d, %d", SEEE_GLUT_S2pre, SEEE_GLUT_S2, SEEE_GLUT_S2pos                              );
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_GLUT_Y2 = %d, %d, %d", rLowerEE.glut_ctrl_02.bits.SEEE_GLUT_Y2, rSmoothEE.glut_ctrl_02.bits.SEEE_GLUT_Y2, rUpperEE.glut_ctrl_02.bits.SEEE_GLUT_Y2      );
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_GLUT_X3 = %d, %d, %d", rLowerEE.glut_ctrl_03.bits.SEEE_GLUT_X3, rSmoothEE.glut_ctrl_03.bits.SEEE_GLUT_X3, rUpperEE.glut_ctrl_03.bits.SEEE_GLUT_X3      );
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_GLUT_S3 = %d, %d, %d", SEEE_GLUT_S3pre, SEEE_GLUT_S3, SEEE_GLUT_S3pos                              );
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_GLUT_Y3 = %d, %d, %d", rLowerEE.glut_ctrl_03.bits.SEEE_GLUT_Y3, rSmoothEE.glut_ctrl_03.bits.SEEE_GLUT_Y3, rUpperEE.glut_ctrl_03.bits.SEEE_GLUT_Y3      );
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_GLUT_X4 = %d, %d, %d", rLowerEE.glut_ctrl_04.bits.SEEE_GLUT_X4, rSmoothEE.glut_ctrl_04.bits.SEEE_GLUT_X4, rUpperEE.glut_ctrl_04.bits.SEEE_GLUT_X4      );
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_GLUT_S4 = %d, %d, %d", SEEE_GLUT_S4pre, SEEE_GLUT_S4, SEEE_GLUT_S4pos                              );
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_GLUT_Y4 = %d, %d, %d", rLowerEE.glut_ctrl_04.bits.SEEE_GLUT_Y4, rSmoothEE.glut_ctrl_04.bits.SEEE_GLUT_Y4, rUpperEE.glut_ctrl_04.bits.SEEE_GLUT_Y4      );
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_GLUT_S5 = %d, %d, %d", SEEE_GLUT_S5pre, SEEE_GLUT_S5, SEEE_GLUT_S5pos                              );
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_GLUT_TH_OVR = %d, %d, %d", rLowerEE.glut_ctrl_06.bits.SEEE_GLUT_TH_OVR, rSmoothEE.glut_ctrl_06.bits.SEEE_GLUT_TH_OVR, rUpperEE.glut_ctrl_06.bits.SEEE_GLUT_TH_OVR  );
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_GLUT_TH_UND = %d, %d, %d", rLowerEE.glut_ctrl_06.bits.SEEE_GLUT_TH_UND, rSmoothEE.glut_ctrl_06.bits.SEEE_GLUT_TH_UND, rUpperEE.glut_ctrl_06.bits.SEEE_GLUT_TH_UND  );
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_GLUT_TH_MIN = %d, %d, %d", rLowerEE.glut_ctrl_06.bits.SEEE_GLUT_TH_MIN, rSmoothEE.glut_ctrl_06.bits.SEEE_GLUT_TH_MIN, rUpperEE.glut_ctrl_06.bits.SEEE_GLUT_TH_MIN  );
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_EDTR_DIAG_AMP = %d, %d, %d", rLowerEE.edtr_ctrl.bits.SEEE_EDTR_DIAG_AMP, rSmoothEE.edtr_ctrl.bits.SEEE_EDTR_DIAG_AMP, rUpperEE.edtr_ctrl.bits.SEEE_EDTR_DIAG_AMP   );
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_EDTR_AMP = %d, %d, %d", rLowerEE.edtr_ctrl.bits.SEEE_EDTR_AMP, rSmoothEE.edtr_ctrl.bits.SEEE_EDTR_AMP, rUpperEE.edtr_ctrl.bits.SEEE_EDTR_AMP        );
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_EDTR_LV = %d, %d, %d", rLowerEE.edtr_ctrl.bits.SEEE_EDTR_LV, rSmoothEE.edtr_ctrl.bits.SEEE_EDTR_LV, rUpperEE.edtr_ctrl.bits.SEEE_EDTR_LV         );
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_EDTR_FLT_MODE = %d, %d, %d", rLowerEE.edtr_ctrl.bits.SEEE_EDTR_FLT_MODE, rSmoothEE.edtr_ctrl.bits.SEEE_EDTR_FLT_MODE, rUpperEE.edtr_ctrl.bits.SEEE_EDTR_FLT_MODE   );
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_EDTR_FLT_2_EN = %d, %d, %d", rLowerEE.edtr_ctrl.bits.SEEE_EDTR_FLT_2_EN, rSmoothEE.edtr_ctrl.bits.SEEE_EDTR_FLT_2_EN, rUpperEE.edtr_ctrl.bits.SEEE_EDTR_FLT_2_EN   );
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_GLUT_X1_1 = %d, %d, %d", rLowerEE.glut_ctrl_07.bits.SEEE_GLUT_X1_1, rSmoothEE.glut_ctrl_07.bits.SEEE_GLUT_X1_1, rUpperEE.glut_ctrl_07.bits.SEEE_GLUT_X1_1    );
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_GLUT_S1_1 = %d, %d, %d", SEEE_GLUT_S1_1pre, SEEE_GLUT_S1_1, SEEE_GLUT_S1_1pos                            );
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_GLUT_S2_1 = %d, %d, %d", SEEE_GLUT_S1_2pre, SEEE_GLUT_S2_1, SEEE_GLUT_S1_2pos                            );
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_GLUT_LLINK_EN = %d, %d, %d", rLowerEE.glut_ctrl_07.bits.SEEE_GLUT_LLINK_EN, rSmoothEE.glut_ctrl_07.bits.SEEE_GLUT_LLINK_EN, rUpperEE.glut_ctrl_07.bits.SEEE_GLUT_LLINK_EN);
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_GLUT_X1_2 = %d, %d, %d", rLowerEE.glut_ctrl_08.bits.SEEE_GLUT_X1_2, rSmoothEE.glut_ctrl_08.bits.SEEE_GLUT_X1_2, rUpperEE.glut_ctrl_08.bits.SEEE_GLUT_X1_2    );
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_GLUT_S1_2 = %d, %d, %d", SEEE_GLUT_S1_2pre, SEEE_GLUT_S1_2, SEEE_GLUT_S1_2pos                            );
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_GLUT_S2_2 = %d, %d, %d", SEEE_GLUT_S2_2pre, SEEE_GLUT_S2_2, SEEE_GLUT_S2_2pos                            );
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_GLUT_X1_3 = %d, %d, %d", rLowerEE.glut_ctrl_09.bits.SEEE_GLUT_X1_3, rSmoothEE.glut_ctrl_09.bits.SEEE_GLUT_X1_3, rUpperEE.glut_ctrl_09.bits.SEEE_GLUT_X1_3    );
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_GLUT_S1_3 = %d, %d, %d", SEEE_GLUT_S1_3pre, SEEE_GLUT_S1_3, SEEE_GLUT_S1_3pos                            );
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_GLUT_S2_3 = %d, %d, %d", SEEE_GLUT_S2_3pre, SEEE_GLUT_S2_3, SEEE_GLUT_S2_3pos                            );
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_GLUT_TH_OVR_1 = %d, %d, %d", rLowerEE.glut_ctrl_10.bits.SEEE_GLUT_TH_OVR_1, rSmoothEE.glut_ctrl_10.bits.SEEE_GLUT_TH_OVR_1, rUpperEE.glut_ctrl_10.bits.SEEE_GLUT_TH_OVR_1);
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_GLUT_TH_UND_1 = %d, %d, %d", rLowerEE.glut_ctrl_10.bits.SEEE_GLUT_TH_UND_1, rSmoothEE.glut_ctrl_10.bits.SEEE_GLUT_TH_UND_1, rUpperEE.glut_ctrl_10.bits.SEEE_GLUT_TH_UND_1);
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_GLUT_TH_OVR_2 = %d, %d, %d", rLowerEE.glut_ctrl_10.bits.SEEE_GLUT_TH_OVR_2, rSmoothEE.glut_ctrl_10.bits.SEEE_GLUT_TH_OVR_2, rUpperEE.glut_ctrl_10.bits.SEEE_GLUT_TH_OVR_2);
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_GLUT_TH_UND_2 = %d, %d, %d", rLowerEE.glut_ctrl_10.bits.SEEE_GLUT_TH_UND_2, rSmoothEE.glut_ctrl_10.bits.SEEE_GLUT_TH_UND_2, rUpperEE.glut_ctrl_10.bits.SEEE_GLUT_TH_UND_2);
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_GLUT_TH_OVR_3 = %d, %d, %d", rLowerEE.glut_ctrl_11.bits.SEEE_GLUT_TH_OVR_3, rSmoothEE.glut_ctrl_11.bits.SEEE_GLUT_TH_OVR_3, rUpperEE.glut_ctrl_11.bits.SEEE_GLUT_TH_OVR_3);
    MY_LOG_IF(bSmoothEEDebug,"[L,S,U]EE.SEEE_GLUT_TH_UND_3 = %d, %d, %d", rLowerEE.glut_ctrl_11.bits.SEEE_GLUT_TH_UND_3, rSmoothEE.glut_ctrl_11.bits.SEEE_GLUT_TH_UND_3, rUpperEE.glut_ctrl_11.bits.SEEE_GLUT_TH_UND_3);

}

// "return value":    Interpolated register value
// "ISO":            ISO value of current image
// "ISOpre":        A pre-defined ISO (100, 200, 400, ...) which is the closest but smaller than or equal to "ISO"
// "ISOpos":        A pre-defined ISO (100, 200, 400, ...) which is the closest but larger than or equal to "ISO"
// "PARAMpre":        Corresponding register value from "ISOpre"
// "PARAMpos":        Corresponding register value from "ISOpos"
// "Method":        0 for linear interpolation, 1 for closest one
MINT32 InterParam_NR(MINT32 ISO, MINT32 ISOpre, MINT32 ISOpos, MINT32 PARAMpre, MINT32 PARAMpos, MINT32 Method){
    MINT32 InterPARAM = 0;

    if(ISOpre == ISOpos){
        // To prevent someone sets different PARAMpre and PARAMpos
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
        double temp = RATIOpre * PARAMpre + RATIOpos * PARAMpos;
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

MVOID SmoothANR(MUINT32 u4RealISO, // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                ISP_NVRAM_ANR_T const& rUpperANR, // ANR settings for upper ISO
                ISP_NVRAM_ANR_T const& rLowerANR, // ANR settings for lower ISO
                ISP_NVRAM_ANR_T& rSmoothANR)   // Output
{
	MBOOL bSmoothANRDebug = interpPropertyGet("vendor.debug.smooth_anr.enable", MFALSE);
	

    MY_LOG_IF(bSmoothANRDebug,"[%s()] u4RealISO: %d, u4UpperISO = %d, u4LowerISO = %d\n", __FUNCTION__, u4RealISO, u4UpperISO, u4LowerISO);

    // ====================================================================================================================================
    // Get Lower ISO setting
    // ====================================================================================================================================
    // CAM_ANR_CON1 CAM+A20H
    MINT32 ANR_ENC_low = rLowerANR.con1.bits.ANR1_ENC;
    MINT32 ANR_ENY_low = rLowerANR.con1.bits.ANR1_ENY;
    MINT32 ANR_SCALE_MODE_low = rLowerANR.con1.bits.ANR_SCALE_MODE;
    MINT32 ANR_MEDIAN_EN_low = rLowerANR.con1.bits.ANR1_MEDIAN_EN;
    MINT32 ANR_LCE_LINK_low = rLowerANR.con1.bits.ANR1_LCE_LINK;
    MINT32 ANR_TABLE_EN_low = rLowerANR.con1.bits.ANR1_TABLE_EN;
    MINT32 ANR_TBL_PRC_low = rLowerANR.con1.bits.ANR1_TBL_PRC;

    // CAM_ANR_CON2 CAM+A24H
    MINT32 ANR_IMPL_MODE_low = rLowerANR.con2.bits.ANR1_IMPL_MODE;
    MINT32 ANR_C_MED_EN_low = rLowerANR.con2.bits.ANR1_C_MED_EN;
    MINT32 ANR_C_SM_EDGE_low = rLowerANR.con2.bits.ANR1_C_SM_EDGE;
    MINT32 ANR_FLT_C_low = rLowerANR.con2.bits.ANR1_FLT_C;
    MINT32 ANR_LR_low = rLowerANR.con2.bits.ANR1_LR;
    MINT32 ANR_ALPHA_low = rLowerANR.con2.bits.ANR1_ALPHA;

    // CAM_ANR_YAD1 CAM+A2CH
    MINT32 ANR_CEN_GAIN_LO_TH_low= rLowerANR.yad1.bits.ANR1_CEN_GAIN_LO_TH;
    MINT32 ANR_CEN_GAIN_HI_TH_low= rLowerANR.yad1.bits.ANR1_CEN_GAIN_HI_TH;
    MINT32 ANR_K_LO_TH_low = rLowerANR.yad1.bits.ANR1_K_LO_TH;
    MINT32 ANR_K_HI_TH_low = rLowerANR.yad1.bits.ANR1_K_HI_TH;
    MINT32 ANR_K_TH_C_low = rLowerANR.yad1.bits.ANR1_K_TH_C;

    // CAM_ANR_YAD2 CAM+A30H
    MINT32 ANR_PTY_VGAIN_low = rLowerANR.yad2.bits.ANR1_PTY_VGAIN;
    MINT32 ANR_PTY_GAIN_TH_low = rLowerANR.yad2.bits.ANR1_PTY_GAIN_TH;

    // CAM_ANR_Y4LUT1 CAM+A34H
    MINT32 ANR_Y_CPX1_low = rLowerANR.lut1.bits.ANR1_Y_CPX1;
    MINT32 ANR_Y_CPX2_low = rLowerANR.lut1.bits.ANR1_Y_CPX2;
    MINT32 ANR_Y_CPX3_low = rLowerANR.lut1.bits.ANR1_Y_CPX3;

    // CAM_ANR_Y4LUT2 CAM+A38H
    MINT32 ANR_Y_SCALE_CPY0_low = rLowerANR.lut2.bits.ANR1_Y_SCALE_CPY0;
    MINT32 ANR_Y_SCALE_CPY1_low = rLowerANR.lut2.bits.ANR1_Y_SCALE_CPY1;
    MINT32 ANR_Y_SCALE_CPY2_low = rLowerANR.lut2.bits.ANR1_Y_SCALE_CPY2;
    MINT32 ANR_Y_SCALE_CPY3_low = rLowerANR.lut2.bits.ANR1_Y_SCALE_CPY3;

    // CAM_ANR_Y4LUT3 CAM+A3CH
    MINT32 ANR_Y_SCALE_SP0_low = rLowerANR.lut3.bits.ANR1_Y_SCALE_SP0;
    MINT32 ANR_Y_SCALE_SP1_low = rLowerANR.lut3.bits.ANR1_Y_SCALE_SP1;
    MINT32 ANR_Y_SCALE_SP2_low = rLowerANR.lut3.bits.ANR1_Y_SCALE_SP2;
    MINT32 ANR_Y_SCALE_SP3_low = rLowerANR.lut3.bits.ANR1_Y_SCALE_SP3;

    // CAM_ANR_C4LUT1
    MINT32 ANR_C_CPX1_low = rLowerANR.clut1.bits.ANR1_C_CPX1;
    MINT32 ANR_C_CPX2_low = rLowerANR.clut1.bits.ANR1_C_CPX2;
    MINT32 ANR_C_CPX3_low = rLowerANR.clut1.bits.ANR1_C_CPX3;

    // CAM_ANR_C4LUT2
    MINT32 ANR_C_SCALE_CPY0_low = rLowerANR.clut2.bits.ANR1_C_SCALE_CPY0;
    MINT32 ANR_C_SCALE_CPY1_low = rLowerANR.clut2.bits.ANR1_C_SCALE_CPY1;
    MINT32 ANR_C_SCALE_CPY2_low = rLowerANR.clut2.bits.ANR1_C_SCALE_CPY2;
    MINT32 ANR_C_SCALE_CPY3_low = rLowerANR.clut2.bits.ANR1_C_SCALE_CPY3;

    // CAM_ANR_C4LUT3
    MINT32 ANR_C_SCALE_SP0_low = rLowerANR.clut3.bits.ANR1_C_SCALE_SP0;
    MINT32 ANR_C_SCALE_SP1_low = rLowerANR.clut3.bits.ANR1_C_SCALE_SP1;
    MINT32 ANR_C_SCALE_SP2_low = rLowerANR.clut3.bits.ANR1_C_SCALE_SP2;
    MINT32 ANR_C_SCALE_SP3_low = rLowerANR.clut3.bits.ANR1_C_SCALE_SP3;

    // CAM_ANR_A4LUT2
    MINT32 ANR_Y_ACT_CPY0_low = rLowerANR.alut2.bits.ANR1_Y_ACT_CPY0;
    MINT32 ANR_Y_ACT_CPY1_low = rLowerANR.alut2.bits.ANR1_Y_ACT_CPY1;
    MINT32 ANR_Y_ACT_CPY2_low = rLowerANR.alut2.bits.ANR1_Y_ACT_CPY2;
    MINT32 ANR_Y_ACT_CPY3_low = rLowerANR.alut2.bits.ANR1_Y_ACT_CPY3;

    // CAM_ANR_A4LUT3
    MINT32 ANR_Y_ACT_SP0_low = rLowerANR.alut3.bits.ANR1_Y_ACT_SP0;
    MINT32 ANR_Y_ACT_SP1_low = rLowerANR.alut3.bits.ANR1_Y_ACT_SP1;
    MINT32 ANR_Y_ACT_SP2_low = rLowerANR.alut3.bits.ANR1_Y_ACT_SP2;
    MINT32 ANR_Y_ACT_SP3_low = rLowerANR.alut3.bits.ANR1_Y_ACT_SP3;

    // CAM_ANR_L4LUT1
    MINT32 ANR_LCE_X1_low = rLowerANR.llut1.bits.ANR1_LCE_X1;
    MINT32 ANR_LCE_X2_low = rLowerANR.llut1.bits.ANR1_LCE_X2;
    MINT32 ANR_LCE_X3_low = rLowerANR.llut1.bits.ANR1_LCE_X3;

    // CAM_ANR_L4LUT2
    MINT32 ANR_LCE_GAIN0_low = rLowerANR.llut2.bits.ANR1_LCE_GAIN0;
    MINT32 ANR_LCE_GAIN1_low = rLowerANR.llut2.bits.ANR1_LCE_GAIN1;
    MINT32 ANR_LCE_GAIN2_low = rLowerANR.llut2.bits.ANR1_LCE_GAIN2;
    MINT32 ANR_LCE_GAIN3_low = rLowerANR.llut2.bits.ANR1_LCE_GAIN3;

    // CAM_ANR_L4LUT3
    MINT32 ANR_LCE_SP0_low = rLowerANR.llut3.bits.ANR1_LCE_SP0;
    MINT32 ANR_LCE_SP1_low = rLowerANR.llut3.bits.ANR1_LCE_SP1;
    MINT32 ANR_LCE_SP2_low = rLowerANR.llut3.bits.ANR1_LCE_SP2;
    MINT32 ANR_LCE_SP3_low = rLowerANR.llut3.bits.ANR1_LCE_SP3;

    // CAM_ANR_PTY CAM+A40H
    MINT32 ANR_PTY1_low = rLowerANR.pty.bits.ANR1_PTY1;
    MINT32 ANR_PTY2_low = rLowerANR.pty.bits.ANR1_PTY2;
    MINT32 ANR_PTY3_low = rLowerANR.pty.bits.ANR1_PTY3;
    MINT32 ANR_PTY4_low = rLowerANR.pty.bits.ANR1_PTY4;

    // CAM_ANR_CAD CAM+A44H
    MINT32 ANR_PTC_VGAIN_low = rLowerANR.cad.bits.ANR1_PTC_VGAIN;
    MINT32 ANR_PTC_GAIN_TH_low = rLowerANR.cad.bits.ANR1_PTC_GAIN_TH;
    MINT32 ANR_C_L_DIFF_TH_low = rLowerANR.cad.bits.ANR1_C_L_DIFF_TH;

    // CAM_ANR_PTC CAM+A48H
    MINT32 ANR_PTC1_low = rLowerANR.ptc.bits.ANR1_PTC1;
    MINT32 ANR_PTC2_low = rLowerANR.ptc.bits.ANR1_PTC2;
    MINT32 ANR_PTC3_low = rLowerANR.ptc.bits.ANR1_PTC3;
    MINT32 ANR_PTC4_low = rLowerANR.ptc.bits.ANR1_PTC4;

    // CAM_ANR_LCE CAM+A4CH
    MINT32 ANR_LCE_C_GAIN_low = rLowerANR.lce.bits.ANR1_LCE_C_GAIN;
    MINT32 ANR_LCE_SCALE_GAIN_low= rLowerANR.lce.bits.ANR1_LCE_SCALE_GAIN;

    // CAM_ANR_MED1
    MINT32 ANR_COR_TH_low = rLowerANR.med1.bits.ANR1_COR_TH;
    MINT32 ANR_COR_SL_low = rLowerANR.med1.bits.ANR1_COR_SL;
    MINT32 ANR_MCD_TH_low = rLowerANR.med1.bits.ANR1_MCD_TH;
    MINT32 ANR_MCD_SL_low = rLowerANR.med1.bits.ANR1_MCD_SL;
    MINT32 ANR_LCL_TH_low = rLowerANR.med1.bits.ANR1_LCL_TH;

    // CAM_ANR_MED2
    MINT32 ANR_LCL_SL_low = rLowerANR.med2.bits.ANR1_LCL_SL;
    MINT32 ANR_LCL_LV_low = rLowerANR.med2.bits.ANR1_LCL_LV;
    MINT32 ANR_SCL_TH_low = rLowerANR.med2.bits.ANR1_SCL_TH;
    MINT32 ANR_SCL_SL_low = rLowerANR.med2.bits.ANR1_SCL_SL;
    MINT32 ANR_SCL_LV_low = rLowerANR.med2.bits.ANR1_SCL_LV;

    // CAM_ANR_MED3
    MINT32 ANR_NCL_TH_low = rLowerANR.med3.bits.ANR1_NCL_TH;
    MINT32 ANR_NCL_SL_low = rLowerANR.med3.bits.ANR1_NCL_SL;
    MINT32 ANR_NCL_LV_low = rLowerANR.med3.bits.ANR1_NCL_LV;
    MINT32 ANR_VAR_low = rLowerANR.med3.bits.ANR1_VAR;
    MINT32 ANR_Y0_low = rLowerANR.med3.bits.ANR1_Y0;

    // CAM_ANR_MED4
    MINT32 ANR_Y1_low = rLowerANR.med4.bits.ANR1_Y1;
    MINT32 ANR_Y2_low = rLowerANR.med4.bits.ANR1_Y2;
    MINT32 ANR_Y3_low = rLowerANR.med4.bits.ANR1_Y3;
    MINT32 ANR_Y4_low = rLowerANR.med4.bits.ANR1_Y4;

    // CAM_ANR_HP1 CAM+A54H
    MINT32 ANR_HP_A_low = rLowerANR.hp1.bits.ANR1_HP_A;
    MINT32 ANR_HP_B_low = rLowerANR.hp1.bits.ANR1_HP_B;
    MINT32 ANR_HP_C_low = rLowerANR.hp1.bits.ANR1_HP_C;
    MINT32 ANR_HP_D_low = rLowerANR.hp1.bits.ANR1_HP_D;
    MINT32 ANR_HP_E_low = rLowerANR.hp1.bits.ANR1_HP_E;

    // CAM_ANR_HP2 CAM+A58H
    MINT32 ANR_HP_S1_low = rLowerANR.hp2.bits.ANR1_HP_S1;
    MINT32 ANR_HP_S2_low = rLowerANR.hp2.bits.ANR1_HP_S2;
    MINT32 ANR_HP_X1_low = rLowerANR.hp2.bits.ANR1_HP_X1;
    MINT32 ANR_HP_F_low = rLowerANR.hp2.bits.ANR1_HP_F;

    // CAM_ANR_HP3 CAM+A5CH
    MINT32 ANR_HP_Y_GAIN_CLIP_low= rLowerANR.hp3.bits.ANR1_HP_Y_GAIN_CLIP;
    MINT32 ANR_HP_Y_SP_low = rLowerANR.hp3.bits.ANR1_HP_Y_SP;
    MINT32 ANR_HP_Y_LO_low = rLowerANR.hp3.bits.ANR1_HP_Y_LO;
    MINT32 ANR_HP_CLIP_low = rLowerANR.hp3.bits.ANR1_HP_CLIP;

    // CAM_ANR_ACTY CAM+A60H
    MINT32 ANR_ACT_TH_Y_low = rLowerANR.acty.bits.ANR1_ACT_TH_Y;
    MINT32 ANR_ACT_BLD_BASE_Y_low= rLowerANR.acty.bits.ANR1_ACT_BLD_BASE_Y;
    MINT32 ANR_ACT_SLANT_Y_low = rLowerANR.acty.bits.ANR1_ACT_SLANT_Y;
    MINT32 ANR_ACT_BLD_TH_Y_low = rLowerANR.acty.bits.ANR1_ACT_BLD_TH_Y;

    // CAM_ANR_ACT1 CAM+A64H
    MINT32 ANR_ACT_LCE_GAIN_low = rLowerANR.act1.bits.ANR1_ACT_LCE_GAIN;
    MINT32 ANR_ACT_SCALE_OFT_low = rLowerANR.act1.bits.ANR1_ACT_SCALE_OFT;
    MINT32 ANR_ACT_SCALE_GAIN_low= rLowerANR.act1.bits.ANR1_ACT_SCALE_GAIN;
    MINT32 ANR_ACT_DIF_GAIN_low = rLowerANR.act1.bits.ANR1_ACT_DIF_GAIN;
    MINT32 ANR_ACT_DIF_LO_TH_low = rLowerANR.act1.bits.ANR1_ACT_DIF_LO_TH;

    // CAM_ANR_ACT2 CAM+A64H
    MINT32 ANR_ACT_SIZE_GAIN_low = rLowerANR.act2.bits.ANR1_ACT_SIZE_GAIN;
    MINT32 ANR_ACT_SIZE_LO_TH_low= rLowerANR.act2.bits.ANR1_ACT_SIZE_LO_TH;
    MINT32 ANR_COR_TH1_low = rLowerANR.act2.bits.ANR1_COR_TH1;
    MINT32 ANR_COR_SL1_low = rLowerANR.act2.bits.ANR1_COR_SL1;

    // CAM_ANR_ACT3 CAM+A64H
    MINT32 ANR_COR_ACT_TH_low = rLowerANR.act3.bits.ANR1_COR_ACT_TH;
    MINT32 ANR_COR_ACT_SL1_low = rLowerANR.act3.bits.ANR1_COR_ACT_SL1;
    MINT32 ANR_COR_ACT_SL2_low = rLowerANR.act3.bits.ANR1_COR_ACT_SL2;

    // CAM_ANR_ACTC CAM+A64H
    MINT32 ANR_ACT_TH_C_low = rLowerANR.actc.bits.ANR1_ACT_TH_C;
    MINT32 ANR_ACT_BLD_BASE_C_low= rLowerANR.actc.bits.ANR1_ACT_BLD_BASE_C;
    MINT32 ANR_ACT_SLANT_C_low = rLowerANR.actc.bits.ANR1_ACT_SLANT_C;
    MINT32 ANR_ACT_BLD_TH_C_low = rLowerANR.actc.bits.ANR1_ACT_BLD_TH_C;

    // ====================================================================================================================================
    // Get Upper ISO setting
    // ====================================================================================================================================
    // CAM_ANR_CON1 CAM+A20H
    MINT32 ANR_ENC_high = rUpperANR.con1.bits.ANR1_ENC;
    MINT32 ANR_ENY_high = rUpperANR.con1.bits.ANR1_ENY;
    MINT32 ANR_SCALE_MODE_high = rUpperANR.con1.bits.ANR_SCALE_MODE;
    MINT32 ANR_MEDIAN_EN_high = rUpperANR.con1.bits.ANR1_MEDIAN_EN;
    MINT32 ANR_LCE_LINK_high = rUpperANR.con1.bits.ANR1_LCE_LINK;
    MINT32 ANR_TABLE_EN_high = rUpperANR.con1.bits.ANR1_TABLE_EN;
    MINT32 ANR_TBL_PRC_high = rUpperANR.con1.bits.ANR1_TBL_PRC;

    // CAM_ANR_CON2 CAM+A24H
    MINT32 ANR_IMPL_MODE_high = rUpperANR.con2.bits.ANR1_IMPL_MODE;
    MINT32 ANR_C_MED_EN_high = rUpperANR.con2.bits.ANR1_C_MED_EN;
    MINT32 ANR_C_SM_EDGE_high = rUpperANR.con2.bits.ANR1_C_SM_EDGE;
    MINT32 ANR_FLT_C_high = rUpperANR.con2.bits.ANR1_FLT_C;
    MINT32 ANR_LR_high = rUpperANR.con2.bits.ANR1_LR;
    MINT32 ANR_ALPHA_high = rUpperANR.con2.bits.ANR1_ALPHA;

    // CAM_ANR_YAD1 CAM+A2CH
    MINT32 ANR_CEN_GAIN_LO_TH_high= rUpperANR.yad1.bits.ANR1_CEN_GAIN_LO_TH;
    MINT32 ANR_CEN_GAIN_HI_TH_high= rUpperANR.yad1.bits.ANR1_CEN_GAIN_HI_TH;
    MINT32 ANR_K_LO_TH_high = rUpperANR.yad1.bits.ANR1_K_LO_TH;
    MINT32 ANR_K_HI_TH_high = rUpperANR.yad1.bits.ANR1_K_HI_TH;
    MINT32 ANR_K_TH_C_high = rUpperANR.yad1.bits.ANR1_K_TH_C;

    // CAM_ANR_YAD2 CAM+A30H
    MINT32 ANR_PTY_VGAIN_high = rUpperANR.yad2.bits.ANR1_PTY_VGAIN;
    MINT32 ANR_PTY_GAIN_TH_high = rUpperANR.yad2.bits.ANR1_PTY_GAIN_TH;

    // CAM_ANR_Y4LUT1 CAM+A34H
    MINT32 ANR_Y_CPX1_high = rUpperANR.lut1.bits.ANR1_Y_CPX1;
    MINT32 ANR_Y_CPX2_high = rUpperANR.lut1.bits.ANR1_Y_CPX2;
    MINT32 ANR_Y_CPX3_high = rUpperANR.lut1.bits.ANR1_Y_CPX3;

    // CAM_ANR_Y4LUT2 CAM+A38H
    MINT32 ANR_Y_SCALE_CPY0_high = rUpperANR.lut2.bits.ANR1_Y_SCALE_CPY0;
    MINT32 ANR_Y_SCALE_CPY1_high = rUpperANR.lut2.bits.ANR1_Y_SCALE_CPY1;
    MINT32 ANR_Y_SCALE_CPY2_high = rUpperANR.lut2.bits.ANR1_Y_SCALE_CPY2;
    MINT32 ANR_Y_SCALE_CPY3_high = rUpperANR.lut2.bits.ANR1_Y_SCALE_CPY3;

    // CAM_ANR_Y4LUT3 CAM+A3CH
    MINT32 ANR_Y_SCALE_SP0_high = rUpperANR.lut3.bits.ANR1_Y_SCALE_SP0;
    MINT32 ANR_Y_SCALE_SP1_high = rUpperANR.lut3.bits.ANR1_Y_SCALE_SP1;
    MINT32 ANR_Y_SCALE_SP2_high = rUpperANR.lut3.bits.ANR1_Y_SCALE_SP2;
    MINT32 ANR_Y_SCALE_SP3_high = rUpperANR.lut3.bits.ANR1_Y_SCALE_SP3;

    // CAM_ANR_C4LUT1
    MINT32 ANR_C_CPX1_high = rUpperANR.clut1.bits.ANR1_C_CPX1;
    MINT32 ANR_C_CPX2_high = rUpperANR.clut1.bits.ANR1_C_CPX2;
    MINT32 ANR_C_CPX3_high = rUpperANR.clut1.bits.ANR1_C_CPX3;

    // CAM_ANR_C4LUT2
    MINT32 ANR_C_SCALE_CPY0_high = rUpperANR.clut2.bits.ANR1_C_SCALE_CPY0;
    MINT32 ANR_C_SCALE_CPY1_high = rUpperANR.clut2.bits.ANR1_C_SCALE_CPY1;
    MINT32 ANR_C_SCALE_CPY2_high = rUpperANR.clut2.bits.ANR1_C_SCALE_CPY2;
    MINT32 ANR_C_SCALE_CPY3_high = rUpperANR.clut2.bits.ANR1_C_SCALE_CPY3;

    // CAM_ANR_C4LUT3
    MINT32 ANR_C_SCALE_SP0_high = rUpperANR.clut3.bits.ANR1_C_SCALE_SP0;
    MINT32 ANR_C_SCALE_SP1_high = rUpperANR.clut3.bits.ANR1_C_SCALE_SP1;
    MINT32 ANR_C_SCALE_SP2_high = rUpperANR.clut3.bits.ANR1_C_SCALE_SP2;
    MINT32 ANR_C_SCALE_SP3_high = rUpperANR.clut3.bits.ANR1_C_SCALE_SP3;

    // CAM_ANR_A4LUT2
    MINT32 ANR_Y_ACT_CPY0_high = rUpperANR.alut2.bits.ANR1_Y_ACT_CPY0;
    MINT32 ANR_Y_ACT_CPY1_high = rUpperANR.alut2.bits.ANR1_Y_ACT_CPY1;
    MINT32 ANR_Y_ACT_CPY2_high = rUpperANR.alut2.bits.ANR1_Y_ACT_CPY2;
    MINT32 ANR_Y_ACT_CPY3_high = rUpperANR.alut2.bits.ANR1_Y_ACT_CPY3;

    // CAM_ANR_A4LUT3
    MINT32 ANR_Y_ACT_SP0_high = rLowerANR.alut3.bits.ANR1_Y_ACT_SP0;
    MINT32 ANR_Y_ACT_SP1_high = rLowerANR.alut3.bits.ANR1_Y_ACT_SP1;
    MINT32 ANR_Y_ACT_SP2_high = rLowerANR.alut3.bits.ANR1_Y_ACT_SP2;
    MINT32 ANR_Y_ACT_SP3_high = rLowerANR.alut3.bits.ANR1_Y_ACT_SP3;

    // CAM_ANR_L4LUT1
    MINT32 ANR_LCE_X1_high = rUpperANR.llut1.bits.ANR1_LCE_X1;
    MINT32 ANR_LCE_X2_high = rUpperANR.llut1.bits.ANR1_LCE_X2;
    MINT32 ANR_LCE_X3_high = rUpperANR.llut1.bits.ANR1_LCE_X3;

    // CAM_ANR_L4LUT2
    MINT32 ANR_LCE_GAIN0_high = rUpperANR.llut2.bits.ANR1_LCE_GAIN0;
    MINT32 ANR_LCE_GAIN1_high = rUpperANR.llut2.bits.ANR1_LCE_GAIN1;
    MINT32 ANR_LCE_GAIN2_high = rUpperANR.llut2.bits.ANR1_LCE_GAIN2;
    MINT32 ANR_LCE_GAIN3_high = rUpperANR.llut2.bits.ANR1_LCE_GAIN3;

    // CAM_ANR_L4LUT3
    MINT32 ANR_LCE_SP0_high = rUpperANR.llut3.bits.ANR1_LCE_SP0;
    MINT32 ANR_LCE_SP1_high = rUpperANR.llut3.bits.ANR1_LCE_SP1;
    MINT32 ANR_LCE_SP2_high = rUpperANR.llut3.bits.ANR1_LCE_SP2;
    MINT32 ANR_LCE_SP3_high = rUpperANR.llut3.bits.ANR1_LCE_SP3;

    // CAM_ANR_PTY CAM+A40H
    MINT32 ANR_PTY1_high = rUpperANR.pty.bits.ANR1_PTY1;
    MINT32 ANR_PTY2_high = rUpperANR.pty.bits.ANR1_PTY2;
    MINT32 ANR_PTY3_high = rUpperANR.pty.bits.ANR1_PTY3;
    MINT32 ANR_PTY4_high = rUpperANR.pty.bits.ANR1_PTY4;

    // CAM_ANR_CAD CAM+A44H
    MINT32 ANR_PTC_VGAIN_high = rUpperANR.cad.bits.ANR1_PTC_VGAIN;
    MINT32 ANR_PTC_GAIN_TH_high = rUpperANR.cad.bits.ANR1_PTC_GAIN_TH;
    MINT32 ANR_C_L_DIFF_TH_high = rUpperANR.cad.bits.ANR1_C_L_DIFF_TH;

    // CAM_ANR_PTC CAM+A48H
    MINT32 ANR_PTC1_high = rUpperANR.ptc.bits.ANR1_PTC1;
    MINT32 ANR_PTC2_high = rUpperANR.ptc.bits.ANR1_PTC2;
    MINT32 ANR_PTC3_high = rUpperANR.ptc.bits.ANR1_PTC3;
    MINT32 ANR_PTC4_high = rUpperANR.ptc.bits.ANR1_PTC4;

    // CAM_ANR_LCE CAM+A4CH
    MINT32 ANR_LCE_C_GAIN_high = rUpperANR.lce.bits.ANR1_LCE_C_GAIN;
    MINT32 ANR_LCE_SCALE_GAIN_high= rUpperANR.lce.bits.ANR1_LCE_SCALE_GAIN;

    // CAM_ANR_MED1
    MINT32 ANR_COR_TH_high = rUpperANR.med1.bits.ANR1_COR_TH;
    MINT32 ANR_COR_SL_high = rUpperANR.med1.bits.ANR1_COR_SL;
    MINT32 ANR_MCD_TH_high = rUpperANR.med1.bits.ANR1_MCD_TH;
    MINT32 ANR_MCD_SL_high = rUpperANR.med1.bits.ANR1_MCD_SL;
    MINT32 ANR_LCL_TH_high = rUpperANR.med1.bits.ANR1_LCL_TH;

    // CAM_ANR_MED2
    MINT32 ANR_LCL_SL_high = rUpperANR.med2.bits.ANR1_LCL_SL;
    MINT32 ANR_LCL_LV_high = rUpperANR.med2.bits.ANR1_LCL_LV;
    MINT32 ANR_SCL_TH_high = rUpperANR.med2.bits.ANR1_SCL_TH;
    MINT32 ANR_SCL_SL_high = rUpperANR.med2.bits.ANR1_SCL_SL;
    MINT32 ANR_SCL_LV_high = rUpperANR.med2.bits.ANR1_SCL_LV;

    // CAM_ANR_MED3
    MINT32 ANR_NCL_TH_high = rUpperANR.med3.bits.ANR1_NCL_TH;
    MINT32 ANR_NCL_SL_high = rUpperANR.med3.bits.ANR1_NCL_SL;
    MINT32 ANR_NCL_LV_high = rUpperANR.med3.bits.ANR1_NCL_LV;
    MINT32 ANR_VAR_high = rUpperANR.med3.bits.ANR1_VAR;
    MINT32 ANR_Y0_high = rUpperANR.med3.bits.ANR1_Y0;

    // CAM_ANR_MED4
    MINT32 ANR_Y1_high = rUpperANR.med4.bits.ANR1_Y1;
    MINT32 ANR_Y2_high = rUpperANR.med4.bits.ANR1_Y2;
    MINT32 ANR_Y3_high = rUpperANR.med4.bits.ANR1_Y3;
    MINT32 ANR_Y4_high = rUpperANR.med4.bits.ANR1_Y4;

    // CAM_ANR_HP1 CAM+A54H
    MINT32 ANR_HP_A_high = rUpperANR.hp1.bits.ANR1_HP_A;
    MINT32 ANR_HP_B_high = rUpperANR.hp1.bits.ANR1_HP_B;
    MINT32 ANR_HP_C_high = rUpperANR.hp1.bits.ANR1_HP_C;
    MINT32 ANR_HP_D_high = rUpperANR.hp1.bits.ANR1_HP_D;
    MINT32 ANR_HP_E_high = rUpperANR.hp1.bits.ANR1_HP_E;

    // CAM_ANR_HP2 CAM+A58H
    MINT32 ANR_HP_S1_high = rUpperANR.hp2.bits.ANR1_HP_S1;
    MINT32 ANR_HP_S2_high = rUpperANR.hp2.bits.ANR1_HP_S2;
    MINT32 ANR_HP_X1_high = rUpperANR.hp2.bits.ANR1_HP_X1;
    MINT32 ANR_HP_F_high = rUpperANR.hp2.bits.ANR1_HP_F;

    // CAM_ANR_HP3 CAM+A5CH
    MINT32 ANR_HP_Y_GAIN_CLIP_high= rUpperANR.hp3.bits.ANR1_HP_Y_GAIN_CLIP;
    MINT32 ANR_HP_Y_SP_high = rUpperANR.hp3.bits.ANR1_HP_Y_SP;
    MINT32 ANR_HP_Y_LO_high = rUpperANR.hp3.bits.ANR1_HP_Y_LO;
    MINT32 ANR_HP_CLIP_high = rUpperANR.hp3.bits.ANR1_HP_CLIP;

    // CAM_ANR_ACTY CAM+A60H
    MINT32 ANR_ACT_TH_Y_high = rUpperANR.acty.bits.ANR1_ACT_TH_Y;
    MINT32 ANR_ACT_BLD_BASE_Y_high= rUpperANR.acty.bits.ANR1_ACT_BLD_BASE_Y;
    MINT32 ANR_ACT_SLANT_Y_high = rUpperANR.acty.bits.ANR1_ACT_SLANT_Y;
    MINT32 ANR_ACT_BLD_TH_Y_high = rUpperANR.acty.bits.ANR1_ACT_BLD_TH_Y;

    // CAM_ANR_ACT1 CAM+A64H
    MINT32 ANR_ACT_LCE_GAIN_high = rUpperANR.act1.bits.ANR1_ACT_LCE_GAIN;
    MINT32 ANR_ACT_SCALE_OFT_high = rUpperANR.act1.bits.ANR1_ACT_SCALE_OFT;
    MINT32 ANR_ACT_SCALE_GAIN_high= rUpperANR.act1.bits.ANR1_ACT_SCALE_GAIN;
    MINT32 ANR_ACT_DIF_GAIN_high = rUpperANR.act1.bits.ANR1_ACT_DIF_GAIN;
    MINT32 ANR_ACT_DIF_LO_TH_high = rUpperANR.act1.bits.ANR1_ACT_DIF_LO_TH;

    // CAM_ANR_ACT2 CAM+A64H
    MINT32 ANR_ACT_SIZE_GAIN_high = rUpperANR.act2.bits.ANR1_ACT_SIZE_GAIN;
    MINT32 ANR_ACT_SIZE_LO_TH_high= rUpperANR.act2.bits.ANR1_ACT_SIZE_LO_TH;
    MINT32 ANR_COR_TH1_high = rUpperANR.act2.bits.ANR1_COR_TH1;
    MINT32 ANR_COR_SL1_high = rUpperANR.act2.bits.ANR1_COR_SL1;

    // CAM_ANR_ACT3 CAM+A64H
    MINT32 ANR_COR_ACT_TH_high = rUpperANR.act3.bits.ANR1_COR_ACT_TH;
    MINT32 ANR_COR_ACT_SL1_high = rUpperANR.act3.bits.ANR1_COR_ACT_SL1;
    MINT32 ANR_COR_ACT_SL2_high = rUpperANR.act3.bits.ANR1_COR_ACT_SL2;

    // CAM_ANR_ACTC CAM+A64H
    MINT32 ANR_ACT_TH_C_high = rUpperANR.actc.bits.ANR1_ACT_TH_C;
    MINT32 ANR_ACT_BLD_BASE_C_high= rUpperANR.actc.bits.ANR1_ACT_BLD_BASE_C;
    MINT32 ANR_ACT_SLANT_C_high = rUpperANR.actc.bits.ANR1_ACT_SLANT_C;
    MINT32 ANR_ACT_BLD_TH_C_high = rUpperANR.actc.bits.ANR1_ACT_BLD_TH_C;

    // ====================================================================================================================================
    // Interpolating reg declaration
    // ====================================================================================================================================
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

    // CAM_ANR_ACTY CAM+A60H
    MINT32 ANR_ACT_TH_Y;
    MINT32 ANR_ACT_BLD_BASE_Y;
    MINT32 ANR_ACT_SLANT_Y;
    MINT32 ANR_ACT_BLD_TH_Y;

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

    // ====================================================================================================================================
    // Negative Conversion
    // ====================================================================================================================================
    ANR_Y_SCALE_SP0_low = (ANR_Y_SCALE_SP0_low > 15) ? (ANR_Y_SCALE_SP0_low - 32) : ANR_Y_SCALE_SP0_low;
    ANR_Y_SCALE_SP1_low = (ANR_Y_SCALE_SP1_low > 15) ? (ANR_Y_SCALE_SP1_low - 32) : ANR_Y_SCALE_SP1_low;
    ANR_Y_SCALE_SP2_low = (ANR_Y_SCALE_SP2_low > 15) ? (ANR_Y_SCALE_SP2_low - 32) : ANR_Y_SCALE_SP2_low;
    ANR_Y_SCALE_SP3_low = (ANR_Y_SCALE_SP3_low > 15) ? (ANR_Y_SCALE_SP3_low - 32) : ANR_Y_SCALE_SP3_low;
    ANR_C_SCALE_SP0_low = (ANR_C_SCALE_SP0_low > 15) ? (ANR_C_SCALE_SP0_low - 32) : ANR_C_SCALE_SP0_low;
    ANR_C_SCALE_SP1_low = (ANR_C_SCALE_SP1_low > 15) ? (ANR_C_SCALE_SP1_low - 32) : ANR_C_SCALE_SP1_low;
    ANR_C_SCALE_SP2_low = (ANR_C_SCALE_SP2_low > 15) ? (ANR_C_SCALE_SP2_low - 32) : ANR_C_SCALE_SP2_low;
    ANR_C_SCALE_SP3_low = (ANR_C_SCALE_SP3_low > 15) ? (ANR_C_SCALE_SP3_low - 32) : ANR_C_SCALE_SP3_low;
    ANR_Y_ACT_SP0_low = (ANR_Y_ACT_SP0_low > 31) ? (ANR_Y_ACT_SP0_low - 64) : ANR_Y_ACT_SP0_low;
    ANR_Y_ACT_SP1_low = (ANR_Y_ACT_SP1_low > 31) ? (ANR_Y_ACT_SP1_low - 64) : ANR_Y_ACT_SP1_low;
    ANR_Y_ACT_SP2_low = (ANR_Y_ACT_SP2_low > 31) ? (ANR_Y_ACT_SP2_low - 64) : ANR_Y_ACT_SP2_low;
    ANR_Y_ACT_SP3_low = (ANR_Y_ACT_SP3_low > 31) ? (ANR_Y_ACT_SP3_low - 64) : ANR_Y_ACT_SP3_low;
    ANR_LCE_SP0_low = (ANR_LCE_SP0_low > 15) ? (ANR_LCE_SP0_low - 32) : ANR_LCE_SP0_low;
    ANR_LCE_SP1_low = (ANR_LCE_SP1_low > 15) ? (ANR_LCE_SP1_low - 32) : ANR_LCE_SP1_low;
    ANR_LCE_SP2_low = (ANR_LCE_SP2_low > 15) ? (ANR_LCE_SP2_low - 32) : ANR_LCE_SP2_low;
    ANR_LCE_SP3_low = (ANR_LCE_SP3_low > 15) ? (ANR_LCE_SP3_low - 32) : ANR_LCE_SP3_low;
    ANR_HP_B_low = (ANR_HP_B_low > 31) ? (ANR_HP_B_low - 64) : ANR_HP_B_low;
    ANR_HP_C_low = (ANR_HP_C_low > 15) ? (ANR_HP_C_low - 32) : ANR_HP_C_low;
    ANR_HP_D_low = (ANR_HP_D_low > 7)  ? (ANR_HP_D_low - 16) : ANR_HP_D_low;
    ANR_HP_E_low = (ANR_HP_E_low > 7)  ? (ANR_HP_E_low - 16) : ANR_HP_E_low;
    ANR_HP_F_low = (ANR_HP_F_low > 3)  ? (ANR_HP_F_low - 8)  : ANR_HP_F_low;

    ANR_Y_SCALE_SP0_high = (ANR_Y_SCALE_SP0_high > 15) ? (ANR_Y_SCALE_SP0_high - 32) : ANR_Y_SCALE_SP0_high;
    ANR_Y_SCALE_SP1_high = (ANR_Y_SCALE_SP1_high > 15) ? (ANR_Y_SCALE_SP1_high - 32) : ANR_Y_SCALE_SP1_high;
    ANR_Y_SCALE_SP2_high = (ANR_Y_SCALE_SP2_high > 15) ? (ANR_Y_SCALE_SP2_high - 32) : ANR_Y_SCALE_SP2_high;
    ANR_Y_SCALE_SP3_high = (ANR_Y_SCALE_SP3_high > 15) ? (ANR_Y_SCALE_SP3_high - 32) : ANR_Y_SCALE_SP3_high;
    ANR_C_SCALE_SP0_high = (ANR_C_SCALE_SP0_high > 15) ? (ANR_C_SCALE_SP0_high - 32) : ANR_C_SCALE_SP0_high;
    ANR_C_SCALE_SP1_high = (ANR_C_SCALE_SP1_high > 15) ? (ANR_C_SCALE_SP1_high - 32) : ANR_C_SCALE_SP1_high;
    ANR_C_SCALE_SP2_high = (ANR_C_SCALE_SP2_high > 15) ? (ANR_C_SCALE_SP2_high - 32) : ANR_C_SCALE_SP2_high;
    ANR_C_SCALE_SP3_high = (ANR_C_SCALE_SP3_high > 15) ? (ANR_C_SCALE_SP3_high - 32) : ANR_C_SCALE_SP3_high;
    ANR_Y_ACT_SP0_high = (ANR_Y_ACT_SP0_high > 31) ? (ANR_Y_ACT_SP0_high - 64) : ANR_Y_ACT_SP0_high;
    ANR_Y_ACT_SP1_high = (ANR_Y_ACT_SP1_high > 31) ? (ANR_Y_ACT_SP1_high - 64) : ANR_Y_ACT_SP1_high;
    ANR_Y_ACT_SP2_high = (ANR_Y_ACT_SP2_high > 31) ? (ANR_Y_ACT_SP2_high - 64) : ANR_Y_ACT_SP2_high;
    ANR_Y_ACT_SP3_high = (ANR_Y_ACT_SP3_high > 31) ? (ANR_Y_ACT_SP3_high - 64) : ANR_Y_ACT_SP3_high;
    ANR_LCE_SP0_high = (ANR_LCE_SP0_high > 15) ? (ANR_LCE_SP0_high - 32) : ANR_LCE_SP0_high;
    ANR_LCE_SP1_high = (ANR_LCE_SP1_high > 15) ? (ANR_LCE_SP1_high - 32) : ANR_LCE_SP1_high;
    ANR_LCE_SP2_high = (ANR_LCE_SP2_high > 15) ? (ANR_LCE_SP2_high - 32) : ANR_LCE_SP2_high;
    ANR_LCE_SP3_high = (ANR_LCE_SP3_high > 15) ? (ANR_LCE_SP3_high - 32) : ANR_LCE_SP3_high;
    ANR_HP_B_high = (ANR_HP_B_high > 31) ? (ANR_HP_B_high - 64) : ANR_HP_B_high;
    ANR_HP_C_high = (ANR_HP_C_high > 15) ? (ANR_HP_C_high - 32) : ANR_HP_C_high;
    ANR_HP_D_high = (ANR_HP_D_high > 7)  ? (ANR_HP_D_high - 16) : ANR_HP_D_high;
    ANR_HP_E_high = (ANR_HP_E_high > 7)  ? (ANR_HP_E_high - 16) : ANR_HP_E_high;
    ANR_HP_F_high = (ANR_HP_F_high > 3)  ? (ANR_HP_F_high - 8)  : ANR_HP_F_high;

    // ====================================================================================================================================
    // Start Parameter Interpolation
    // ====================================================================================================================================

    MINT32 ISO = u4RealISO;
    MINT32 ISO_low = u4LowerISO;
    MINT32 ISO_high = u4UpperISO;

    if(ANR_ENC_low==ANR_ENC_high && ANR_ENY_low==ANR_ENY_high)
    {
        // Registers that can not be interpolated
        ANR_ENC = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ENC_low, ANR_ENC_high, 1);
        ANR_ENY = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ENY_low, ANR_ENY_high, 1);
        ANR_TABLE_EN = InterParam_NR(ISO, ISO_low, ISO_high, ANR_TABLE_EN_low, ANR_TABLE_EN_high, 1);
        ANR_TBL_PRC = InterParam_NR(ISO, ISO_low, ISO_high, ANR_TBL_PRC_low, ANR_TBL_PRC_high, 1);
        ANR_IMPL_MODE = InterParam_NR(ISO, ISO_low, ISO_high, ANR_IMPL_MODE_low, ANR_IMPL_MODE_high, 1);
        ANR_C_MED_EN = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_MED_EN_low, ANR_C_MED_EN_high, 1);
        ANR_C_SM_EDGE = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_SM_EDGE_low, ANR_C_SM_EDGE_high, 1);
        ANR_FLT_C = InterParam_NR(ISO, ISO_low, ISO_high, ANR_FLT_C_low, ANR_FLT_C_high, 1);
        ANR_LR = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LR_low, ANR_LR_high, 1);
        ANR_HP_A = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_A_low, ANR_HP_A_high, 1);
        ANR_HP_B = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_B_low, ANR_HP_B_high, 1);
        ANR_HP_C = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_C_low, ANR_HP_C_high, 1);
        ANR_HP_D = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_D_low, ANR_HP_D_high, 1);
        ANR_HP_E = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_E_low, ANR_HP_E_high, 1);
        ANR_HP_F = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_F_low, ANR_HP_F_high, 1);
        ANR_HP_S1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_S1_low, ANR_HP_S1_high, 1);
        ANR_HP_S2 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_S2_low, ANR_HP_S2_high, 1);
        ANR_HP_X1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_X1_low, ANR_HP_X1_high, 1);
        ANR_HP_Y_GAIN_CLIP = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_Y_GAIN_CLIP_low, ANR_HP_Y_GAIN_CLIP_high, 1);
        ANR_HP_Y_SP = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_Y_SP_low, ANR_HP_Y_SP_high, 1);
        ANR_HP_Y_LO = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_Y_LO_low, ANR_HP_Y_LO_high, 1);
        ANR_HP_CLIP = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_CLIP_low, ANR_HP_CLIP_high, 1);

        // Registers that can be interpolated
        ANR_ALPHA = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ALPHA_low, ANR_ALPHA_high, 0);
        ANR_CEN_GAIN_LO_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_CEN_GAIN_LO_TH_low, ANR_CEN_GAIN_LO_TH_high, 0);
        ANR_CEN_GAIN_HI_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_CEN_GAIN_HI_TH_low, ANR_CEN_GAIN_HI_TH_high, 0);
        ANR_PTY_VGAIN = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTY_VGAIN_low, ANR_PTY_VGAIN_high, 0);
        ANR_PTY_GAIN_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTY_GAIN_TH_low, ANR_PTY_GAIN_TH_high, 0);
        ANR_Y_CPX1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_CPX1_low, ANR_Y_CPX1_high, 0);
        ANR_Y_CPX2 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_CPX2_low, ANR_Y_CPX2_high, 0);
        ANR_Y_CPX3 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_CPX3_low, ANR_Y_CPX3_high, 0);
        ANR_Y_SCALE_CPY0 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_SCALE_CPY0_low, ANR_Y_SCALE_CPY0_high, 0);
        ANR_Y_SCALE_CPY1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_SCALE_CPY1_low, ANR_Y_SCALE_CPY1_high, 0);
        ANR_Y_SCALE_CPY2 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_SCALE_CPY2_low, ANR_Y_SCALE_CPY2_high, 0);
        ANR_Y_SCALE_CPY3 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_SCALE_CPY3_low, ANR_Y_SCALE_CPY3_high, 0);
        ANR_C_CPX1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_CPX1_low, ANR_C_CPX1_high, 0);
        ANR_C_CPX2 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_CPX2_low, ANR_C_CPX2_high, 0);
        ANR_C_CPX3 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_CPX3_low, ANR_C_CPX3_high, 0);
        ANR_C_SCALE_CPY0 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_SCALE_CPY0_low, ANR_C_SCALE_CPY0_high, 0);
        ANR_C_SCALE_CPY1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_SCALE_CPY1_low, ANR_C_SCALE_CPY1_high, 0);
        ANR_C_SCALE_CPY2 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_SCALE_CPY2_low, ANR_C_SCALE_CPY2_high, 0);
        ANR_C_SCALE_CPY3 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_SCALE_CPY3_low, ANR_C_SCALE_CPY3_high, 0);
        ANR_Y_ACT_CPY0 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_ACT_CPY0_low, ANR_Y_ACT_CPY0_high, 0);
        ANR_Y_ACT_CPY1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_ACT_CPY1_low, ANR_Y_ACT_CPY1_high, 0);
        ANR_Y_ACT_CPY2 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_ACT_CPY2_low, ANR_Y_ACT_CPY2_high, 0);
        ANR_Y_ACT_CPY3 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_ACT_CPY3_low, ANR_Y_ACT_CPY3_high, 0);
        ANR_PTY1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTY1_low, ANR_PTY1_high, 0);
        ANR_PTY2 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTY2_low, ANR_PTY2_high, 0);
        ANR_PTY3 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTY3_low, ANR_PTY3_high, 0);
        ANR_PTY4 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTY4_low, ANR_PTY4_high, 0);
        ANR_PTC_VGAIN = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTC_VGAIN_low, ANR_PTC_VGAIN_high, 0);
        ANR_PTC_GAIN_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTC_GAIN_TH_low, ANR_PTC_GAIN_TH_high, 0);
        ANR_C_L_DIFF_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_L_DIFF_TH_low, ANR_C_L_DIFF_TH_high, 0);
        ANR_PTC1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTC1_low, ANR_PTC1_high, 0);
        ANR_PTC2 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTC2_low, ANR_PTC2_high, 0);
        ANR_PTC3 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTC3_low, ANR_PTC3_high, 0);
        ANR_PTC4 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTC4_low, ANR_PTC4_high, 0);
        ANR_ACT_TH_Y = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_TH_Y_low, ANR_ACT_TH_Y_high, 0);
        ANR_ACT_BLD_BASE_Y = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_BLD_BASE_Y_low, ANR_ACT_BLD_BASE_Y_high, 0);
        ANR_ACT_SLANT_Y = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_SLANT_Y_low, ANR_ACT_SLANT_Y_high, 0);
        ANR_ACT_BLD_TH_Y = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_BLD_TH_Y_low, ANR_ACT_BLD_TH_Y_high, 0);
        ANR_ACT_LCE_GAIN = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_LCE_GAIN_low, ANR_ACT_LCE_GAIN_high, 0);
        ANR_ACT_SCALE_OFT = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_SCALE_OFT_low, ANR_ACT_SCALE_OFT_high, 0);
        ANR_ACT_SCALE_GAIN = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_SCALE_GAIN_low, ANR_ACT_SCALE_GAIN_high, 0);
        ANR_ACT_DIF_GAIN = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_DIF_GAIN_low, ANR_ACT_DIF_GAIN_high, 0);
        ANR_ACT_DIF_LO_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_DIF_LO_TH_low, ANR_ACT_DIF_LO_TH_high, 0);
        ANR_ACT_SIZE_GAIN = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_SIZE_GAIN_low, ANR_ACT_SIZE_GAIN_high, 0);
        ANR_ACT_SIZE_LO_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_SIZE_LO_TH_low, ANR_ACT_SIZE_LO_TH_high, 0);
        ANR_COR_TH1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_COR_TH1_low, ANR_COR_TH1_high, 0);
        ANR_COR_SL1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_COR_SL1_low, ANR_COR_SL1_high, 0);
        ANR_COR_ACT_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_COR_ACT_TH_low, ANR_COR_ACT_TH_high, 0);
        ANR_COR_ACT_SL1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_COR_ACT_SL1_low, ANR_COR_ACT_SL1_high, 0);
        ANR_COR_ACT_SL2 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_COR_ACT_SL2_low, ANR_COR_ACT_SL2_high, 0);
        ANR_ACT_TH_C = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_TH_C_low, ANR_ACT_TH_C_high, 0);
        ANR_ACT_BLD_BASE_C = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_BLD_BASE_C_low, ANR_ACT_BLD_BASE_C_high, 0);
        ANR_ACT_SLANT_C = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_SLANT_C_low, ANR_ACT_SLANT_C_high, 0);
        ANR_ACT_BLD_TH_C = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_BLD_TH_C_low, ANR_ACT_BLD_TH_C_high, 0);

        // ANR_SCALE_MODE - bit 1 for y, bit 0 for c
            MINT32 scale_mode_y_low = (ANR_SCALE_MODE_low  >> 1);
        MINT32 scale_mode_c_low = (ANR_SCALE_MODE_low  & 1);
            MINT32 scale_mode_y_high = (ANR_SCALE_MODE_high >> 1);
        MINT32 scale_mode_c_high = (ANR_SCALE_MODE_high & 1);
        MINT32 scale_mode_y, scale_mode_c;

        if(scale_mode_y_low == scale_mode_y_high)
        {
            scale_mode_y = scale_mode_y_low;
            ANR_K_LO_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_K_LO_TH_low, ANR_K_LO_TH_high, 0);
            ANR_K_HI_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_K_HI_TH_low, ANR_K_HI_TH_high, 0);
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
            ANR_K_TH_C = InterParam_NR(ISO, ISO_low, ISO_high, ANR_K_TH_C_low, ANR_K_TH_C_high, 0);
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

            ANR_K_TH_C = InterParam_NR(ISO, ISO_low, ISO_high, ANR_K_TH_C_low, ANR_K_TH_C_high, 0);

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
            ANR_LCE_LINK_low = ANR_LCE_LINK_high;
            ANR_LCE_X1_low = ANR_LCE_X1_high;
            ANR_LCE_X2_low = ANR_LCE_X2_high;
            ANR_LCE_X3_low = ANR_LCE_X3_high;
            ANR_LCE_GAIN0_low = 8;
            ANR_LCE_GAIN1_low = 8;
            ANR_LCE_GAIN2_low = 8;
            ANR_LCE_GAIN3_low = 8;
            ANR_LCE_SP0_low = 0;
            ANR_LCE_SP1_low = 0;
            ANR_LCE_SP2_low = 0;
            ANR_LCE_SP3_low = 0;
            ANR_LCE_C_GAIN_low = 4;
            ANR_LCE_SCALE_GAIN_low = 0;
        }
        if(0 == ANR_LCE_LINK_high && 1 == ANR_LCE_LINK_low)
        {
            ANR_LCE_LINK_high = ANR_LCE_LINK_low;
            ANR_LCE_X1_high = ANR_LCE_X1_low;
            ANR_LCE_X2_high = ANR_LCE_X2_low;
            ANR_LCE_X3_high = ANR_LCE_X3_low;
            ANR_LCE_GAIN0_high = 8;
            ANR_LCE_GAIN1_high = 8;
            ANR_LCE_GAIN2_high = 8;
            ANR_LCE_GAIN3_high = 8;
            ANR_LCE_SP0_high = 0;
            ANR_LCE_SP1_high = 0;
            ANR_LCE_SP2_high = 0;
            ANR_LCE_SP3_high = 0;
            ANR_LCE_C_GAIN_high = 4;
            ANR_LCE_SCALE_GAIN_high = 0;
        }

        ANR_LCE_LINK = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_LINK_low, ANR_LCE_LINK_high, 0);
        ANR_LCE_X1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_X1_low, ANR_LCE_X1_high, 0);
        ANR_LCE_X2 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_X2_low, ANR_LCE_X2_high, 0);
        ANR_LCE_X3 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_X3_low, ANR_LCE_X3_high, 0);
        ANR_LCE_GAIN0 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_GAIN0_low, ANR_LCE_GAIN0_high, 0);
        ANR_LCE_GAIN1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_GAIN1_low, ANR_LCE_GAIN1_high, 0);
        ANR_LCE_GAIN2 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_GAIN2_low, ANR_LCE_GAIN2_high, 0);
        ANR_LCE_GAIN3 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_GAIN3_low, ANR_LCE_GAIN3_high, 0);
        ANR_LCE_C_GAIN = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_C_GAIN_low, ANR_LCE_C_GAIN_high, 0);
        ANR_LCE_SCALE_GAIN = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_SCALE_GAIN_low, ANR_LCE_SCALE_GAIN_high, 0);

        // SP Calculation
                ANR_Y_SCALE_SP0 = CalculateLUTSP(         0, ANR_Y_CPX1, ANR_Y_SCALE_CPY0, ANR_Y_SCALE_CPY1, 32, 15, -15);
                ANR_Y_SCALE_SP1 = CalculateLUTSP(ANR_Y_CPX1, ANR_Y_CPX2, ANR_Y_SCALE_CPY1, ANR_Y_SCALE_CPY2, 32, 15, -15);
                ANR_Y_SCALE_SP2 = CalculateLUTSP(ANR_Y_CPX2, ANR_Y_CPX3, ANR_Y_SCALE_CPY2, ANR_Y_SCALE_CPY3, 32, 15, -15);
                ANR_Y_SCALE_SP3 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_SCALE_SP3_low, ANR_Y_SCALE_SP3_high, 0);
                ANR_C_SCALE_SP0 = CalculateLUTSP(         0, ANR_C_CPX1, ANR_C_SCALE_CPY0, ANR_C_SCALE_CPY1, 32, 15, -15);
                ANR_C_SCALE_SP1 = CalculateLUTSP(ANR_C_CPX1, ANR_C_CPX2, ANR_C_SCALE_CPY1, ANR_C_SCALE_CPY2, 32, 15, -15);
                ANR_C_SCALE_SP2 = CalculateLUTSP(ANR_C_CPX2, ANR_C_CPX3, ANR_C_SCALE_CPY2, ANR_C_SCALE_CPY3, 32, 15, -15);
                ANR_C_SCALE_SP3 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_SCALE_SP3_low, ANR_C_SCALE_SP3_high, 0);
                ANR_Y_ACT_SP0 = CalculateLUTSP(         0, ANR_Y_CPX1, ANR_Y_ACT_CPY0, ANR_Y_ACT_CPY1, 32, 31, -31);
                ANR_Y_ACT_SP1 = CalculateLUTSP(ANR_Y_CPX1, ANR_Y_CPX2, ANR_Y_ACT_CPY1, ANR_Y_ACT_CPY2, 32, 31, -31);
                ANR_Y_ACT_SP2 = CalculateLUTSP(ANR_Y_CPX2, ANR_Y_CPX3, ANR_Y_ACT_CPY2, ANR_Y_ACT_CPY3, 32, 31, -31);
        ANR_Y_ACT_SP3 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_ACT_SP3_low, ANR_Y_ACT_SP3_high, 0);
                ANR_LCE_SP0 = CalculateLUTSP(         0, ANR_LCE_X1, ANR_LCE_GAIN0, ANR_LCE_GAIN1, 32, 15, -15);
                ANR_LCE_SP1 = CalculateLUTSP(ANR_LCE_X1, ANR_LCE_X2, ANR_LCE_GAIN1, ANR_LCE_GAIN2, 32, 15, -15);
                ANR_LCE_SP2 = CalculateLUTSP(ANR_LCE_X2, ANR_LCE_X3, ANR_LCE_GAIN2, ANR_LCE_GAIN3, 32, 15, -15);
                ANR_LCE_SP3 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_SP3_low, ANR_LCE_SP3_high, 0);

        // ANR_MEDIAN_EN
        if (ANR_MEDIAN_EN_low == ANR_MEDIAN_EN_high)
        {
            ANR_MEDIAN_EN = InterParam_NR(ISO, ISO_low, ISO_high, ANR_MEDIAN_EN_low, ANR_MEDIAN_EN_high, 0);
            ANR_COR_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_COR_TH_low, ANR_COR_TH_high, 0);
            ANR_COR_SL = InterParam_NR(ISO, ISO_low, ISO_high, ANR_COR_SL_low, ANR_COR_SL_high, 0);
            ANR_MCD_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_MCD_TH_low, ANR_MCD_TH_high, 0);
            ANR_MCD_SL = InterParam_NR(ISO, ISO_low, ISO_high, ANR_MCD_SL_low, ANR_MCD_SL_high, 0);
            ANR_LCL_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCL_TH_low, ANR_LCL_TH_high, 0);
            ANR_LCL_SL = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCL_SL_low, ANR_LCL_SL_high, 0);
            ANR_LCL_LV = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCL_LV_low, ANR_LCL_LV_high, 0);
            ANR_SCL_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_SCL_TH_low, ANR_SCL_TH_high, 0);
            ANR_SCL_SL = InterParam_NR(ISO, ISO_low, ISO_high, ANR_SCL_SL_low, ANR_SCL_SL_high, 0);
            ANR_SCL_LV = InterParam_NR(ISO, ISO_low, ISO_high, ANR_SCL_LV_low, ANR_SCL_LV_high, 0);
            ANR_NCL_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_NCL_TH_low, ANR_NCL_TH_high, 0);
            ANR_NCL_SL = InterParam_NR(ISO, ISO_low, ISO_high, ANR_NCL_SL_low, ANR_NCL_SL_high, 0);
            ANR_NCL_LV = InterParam_NR(ISO, ISO_low, ISO_high, ANR_NCL_LV_low, ANR_NCL_LV_high, 0);
            ANR_VAR = InterParam_NR(ISO, ISO_low, ISO_high, ANR_VAR_low, ANR_VAR_high, 0);
            ANR_Y0 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y0_low, ANR_Y0_high, 0);
            ANR_Y1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y1_low, ANR_Y1_high, 0);
            ANR_Y2 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y2_low, ANR_Y2_high, 0);
            ANR_Y3 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y3_low, ANR_Y3_high, 0);
            ANR_Y4 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y4_low, ANR_Y4_high, 0);
        }
        else
        {
            ANR_MEDIAN_EN = InterParam_NR(ISO, ISO_low, ISO_high, ANR_MEDIAN_EN_low, ANR_MEDIAN_EN_high, 1);
            ANR_COR_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_COR_TH_low, ANR_COR_TH_high, 1);
            ANR_COR_SL = InterParam_NR(ISO, ISO_low, ISO_high, ANR_COR_SL_low, ANR_COR_SL_high, 1);
            ANR_MCD_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_MCD_TH_low, ANR_MCD_TH_high, 1);
            ANR_MCD_SL = InterParam_NR(ISO, ISO_low, ISO_high, ANR_MCD_SL_low, ANR_MCD_SL_high, 1);
            ANR_LCL_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCL_TH_low, ANR_LCL_TH_high, 1);
            ANR_LCL_SL = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCL_SL_low, ANR_LCL_SL_high, 1);
            ANR_LCL_LV = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCL_LV_low, ANR_LCL_LV_high, 1);
            ANR_SCL_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_SCL_TH_low, ANR_SCL_TH_high, 1);
            ANR_SCL_SL = InterParam_NR(ISO, ISO_low, ISO_high, ANR_SCL_SL_low, ANR_SCL_SL_high, 1);
            ANR_SCL_LV = InterParam_NR(ISO, ISO_low, ISO_high, ANR_SCL_LV_low, ANR_SCL_LV_high, 1);
            ANR_NCL_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_NCL_TH_low, ANR_NCL_TH_high, 1);
            ANR_NCL_SL = InterParam_NR(ISO, ISO_low, ISO_high, ANR_NCL_SL_low, ANR_NCL_SL_high, 1);
            ANR_NCL_LV = InterParam_NR(ISO, ISO_low, ISO_high, ANR_NCL_LV_low, ANR_NCL_LV_high, 1);
            ANR_VAR = InterParam_NR(ISO, ISO_low, ISO_high, ANR_VAR_low, ANR_VAR_high, 1);
            ANR_Y0 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y0_low, ANR_Y0_high, 1);
            ANR_Y1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y1_low, ANR_Y1_high, 1);
            ANR_Y2 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y2_low, ANR_Y2_high, 1);
            ANR_Y3 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y3_low, ANR_Y3_high, 1);
            ANR_Y4 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y4_low, ANR_Y4_high, 1);
        }
    }
    else
    {
        // Registers that can not be interpolated
        ANR_ENC = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ENC_low, ANR_ENC_high, 1);
        ANR_ENY = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ENY_low, ANR_ENY_high, 1);
        ANR_TABLE_EN = InterParam_NR(ISO, ISO_low, ISO_high, ANR_TABLE_EN_low, ANR_TABLE_EN_high, 1);
        ANR_TBL_PRC = InterParam_NR(ISO, ISO_low, ISO_high, ANR_TBL_PRC_low, ANR_TBL_PRC_high, 1);
        ANR_IMPL_MODE = InterParam_NR(ISO, ISO_low, ISO_high, ANR_IMPL_MODE_low, ANR_IMPL_MODE_high, 1);
        ANR_C_MED_EN = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_MED_EN_low, ANR_C_MED_EN_high, 1);
        ANR_C_SM_EDGE = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_SM_EDGE_low, ANR_C_SM_EDGE_high, 1);
        ANR_FLT_C = InterParam_NR(ISO, ISO_low, ISO_high, ANR_FLT_C_low, ANR_FLT_C_high, 1);
        ANR_LR = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LR_low, ANR_LR_high, 1);
        ANR_ALPHA = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ALPHA_low, ANR_ALPHA_high, 1);
        ANR_CEN_GAIN_LO_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_CEN_GAIN_LO_TH_low, ANR_CEN_GAIN_LO_TH_high, 1);
        ANR_CEN_GAIN_HI_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_CEN_GAIN_HI_TH_low, ANR_CEN_GAIN_HI_TH_high, 1);
        ANR_PTY_VGAIN = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTY_VGAIN_low, ANR_PTY_VGAIN_high, 1);
        ANR_PTY_GAIN_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTY_GAIN_TH_low, ANR_PTY_GAIN_TH_high, 1);
        ANR_Y_CPX1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_CPX1_low, ANR_Y_CPX1_high, 1);
        ANR_Y_CPX2 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_CPX2_low, ANR_Y_CPX2_high, 1);
        ANR_Y_CPX3 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_CPX3_low, ANR_Y_CPX3_high, 1);
        ANR_Y_SCALE_CPY0 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_SCALE_CPY0_low, ANR_Y_SCALE_CPY0_high, 1);
        ANR_Y_SCALE_CPY1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_SCALE_CPY1_low, ANR_Y_SCALE_CPY1_high, 1);
        ANR_Y_SCALE_CPY2 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_SCALE_CPY2_low, ANR_Y_SCALE_CPY2_high, 1);
        ANR_Y_SCALE_CPY3 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_SCALE_CPY3_low, ANR_Y_SCALE_CPY3_high, 1);
        ANR_Y_SCALE_SP0 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_SCALE_SP0_low, ANR_Y_SCALE_SP0_high, 1);
        ANR_Y_SCALE_SP1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_SCALE_SP1_low, ANR_Y_SCALE_SP1_high, 1);
        ANR_Y_SCALE_SP2 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_SCALE_SP2_low, ANR_Y_SCALE_SP2_high, 1);
        ANR_Y_SCALE_SP3 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_SCALE_SP3_low, ANR_Y_SCALE_SP3_high, 1);
        ANR_C_CPX1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_CPX1_low, ANR_C_CPX1_high, 1);
        ANR_C_CPX2 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_CPX2_low, ANR_C_CPX2_high, 1);
        ANR_C_CPX3 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_CPX3_low, ANR_C_CPX3_high, 1);
        ANR_C_SCALE_CPY0 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_SCALE_CPY0_low, ANR_C_SCALE_CPY0_high, 1);
        ANR_C_SCALE_CPY1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_SCALE_CPY1_low, ANR_C_SCALE_CPY1_high, 1);
        ANR_C_SCALE_CPY2 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_SCALE_CPY2_low, ANR_C_SCALE_CPY2_high, 1);
        ANR_C_SCALE_CPY3 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_SCALE_CPY3_low, ANR_C_SCALE_CPY3_high, 1);
        ANR_C_SCALE_SP0 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_SCALE_SP0_low, ANR_C_SCALE_SP0_high, 1);
        ANR_C_SCALE_SP1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_SCALE_SP1_low, ANR_C_SCALE_SP1_high, 1);
        ANR_C_SCALE_SP2 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_SCALE_SP2_low, ANR_C_SCALE_SP2_high, 1);
        ANR_C_SCALE_SP3 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_SCALE_SP3_low, ANR_C_SCALE_SP3_high, 1);
        ANR_Y_ACT_CPY0 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_ACT_CPY0_low, ANR_Y_ACT_CPY0_high, 1);
        ANR_Y_ACT_CPY1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_ACT_CPY1_low, ANR_Y_ACT_CPY1_high, 1);
        ANR_Y_ACT_CPY2 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_ACT_CPY2_low, ANR_Y_ACT_CPY2_high, 1);
        ANR_Y_ACT_CPY3 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_ACT_CPY3_low, ANR_Y_ACT_CPY3_high, 1);
        ANR_Y_ACT_SP0 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_ACT_SP0_low, ANR_Y_ACT_SP0_high, 1);
        ANR_Y_ACT_SP1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_ACT_SP1_low, ANR_Y_ACT_SP1_high, 1);
        ANR_Y_ACT_SP2 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_ACT_SP2_low, ANR_Y_ACT_SP2_high, 1);
        ANR_Y_ACT_SP3 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_ACT_SP3_low, ANR_Y_ACT_SP3_high, 1);
        ANR_PTY1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTY1_low, ANR_PTY1_high, 1);
        ANR_PTY2 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTY2_low, ANR_PTY2_high, 1);
        ANR_PTY3 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTY3_low, ANR_PTY3_high, 1);
        ANR_PTY4 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTY4_low, ANR_PTY4_high, 1);
        ANR_PTC_VGAIN = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTC_VGAIN_low, ANR_PTC_VGAIN_high, 1);
        ANR_PTC_GAIN_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTC_GAIN_TH_low, ANR_PTC_GAIN_TH_high, 1);
        ANR_C_L_DIFF_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_L_DIFF_TH_low, ANR_C_L_DIFF_TH_high, 1);
        ANR_PTC1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTC1_low, ANR_PTC1_high, 1);
        ANR_PTC2 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTC2_low, ANR_PTC2_high, 1);
        ANR_PTC3 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTC3_low, ANR_PTC3_high, 1);
        ANR_PTC4 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTC4_low, ANR_PTC4_high, 1);
        ANR_HP_A = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_A_low, ANR_HP_A_high, 1);
        ANR_HP_B = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_B_low, ANR_HP_B_high, 1);
        ANR_HP_C = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_C_low, ANR_HP_C_high, 1);
        ANR_HP_D = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_D_low, ANR_HP_D_high, 1);
        ANR_HP_E = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_E_low, ANR_HP_E_high, 1);
        ANR_HP_F = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_F_low, ANR_HP_F_high, 1);
        ANR_HP_S1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_S1_low, ANR_HP_S1_high, 1);
        ANR_HP_S2 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_S2_low, ANR_HP_S2_high, 1);
        ANR_HP_X1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_X1_low, ANR_HP_X1_high, 1);
        ANR_HP_Y_GAIN_CLIP = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_Y_GAIN_CLIP_low, ANR_HP_Y_GAIN_CLIP_high, 1);
        ANR_HP_Y_SP = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_Y_SP_low, ANR_HP_Y_SP_high, 1);
        ANR_HP_Y_LO = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_Y_LO_low, ANR_HP_Y_LO_high, 1);
        ANR_HP_CLIP = InterParam_NR(ISO, ISO_low, ISO_high, ANR_HP_CLIP_low, ANR_HP_CLIP_high, 1);
        ANR_ACT_TH_Y = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_TH_Y_low, ANR_ACT_TH_Y_high, 1);
        ANR_ACT_BLD_BASE_Y = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_BLD_BASE_Y_low, ANR_ACT_BLD_BASE_Y_high, 1);
        ANR_ACT_SLANT_Y = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_SLANT_Y_low, ANR_ACT_SLANT_Y_high, 1);
        ANR_ACT_BLD_TH_Y = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_BLD_TH_Y_low, ANR_ACT_BLD_TH_Y_high, 1);
        ANR_ACT_LCE_GAIN = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_LCE_GAIN_low, ANR_ACT_LCE_GAIN_high, 1);
        ANR_ACT_SCALE_OFT = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_SCALE_OFT_low, ANR_ACT_SCALE_OFT_high, 1);
        ANR_ACT_SCALE_GAIN = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_SCALE_GAIN_low, ANR_ACT_SCALE_GAIN_high, 1);
        ANR_ACT_DIF_GAIN = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_DIF_GAIN_low, ANR_ACT_DIF_GAIN_high, 1);
        ANR_ACT_DIF_LO_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_DIF_LO_TH_low, ANR_ACT_DIF_LO_TH_high, 1);
        ANR_ACT_SIZE_GAIN = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_SIZE_GAIN_low, ANR_ACT_SIZE_GAIN_high, 1);
        ANR_ACT_SIZE_LO_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_SIZE_LO_TH_low, ANR_ACT_SIZE_LO_TH_high, 1);
        ANR_COR_TH1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_COR_TH1_low, ANR_COR_TH1_high, 1);
        ANR_COR_SL1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_COR_SL1_low, ANR_COR_SL1_high, 1);
        ANR_COR_ACT_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_COR_ACT_TH_low, ANR_COR_ACT_TH_high, 1);
        ANR_COR_ACT_SL1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_COR_ACT_SL1_low, ANR_COR_ACT_SL1_high, 1);
        ANR_COR_ACT_SL2 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_COR_ACT_SL2_low, ANR_COR_ACT_SL2_high, 1);
        ANR_ACT_TH_C = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_TH_C_low, ANR_ACT_TH_C_high, 1);
        ANR_ACT_BLD_BASE_C = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_BLD_BASE_C_low, ANR_ACT_BLD_BASE_C_high, 1);
        ANR_ACT_SLANT_C = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_SLANT_C_low, ANR_ACT_SLANT_C_high, 1);
        ANR_ACT_BLD_TH_C = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_BLD_TH_C_low, ANR_ACT_BLD_TH_C_high, 1);

        // ANR_SCALE_MODE
        ANR_SCALE_MODE = InterParam_NR(ISO, ISO_low, ISO_high, ANR_SCALE_MODE_low, ANR_SCALE_MODE_high, 1);
        ANR_K_LO_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_K_LO_TH_low, ANR_K_LO_TH_high, 1);
        ANR_K_HI_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_K_HI_TH_low, ANR_K_HI_TH_high, 1);
        ANR_K_TH_C = InterParam_NR(ISO, ISO_low, ISO_high, ANR_K_TH_C_low, ANR_K_TH_C_high, 1);

        // ANR_LCE_LINK
        ANR_LCE_LINK = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_LINK_low, ANR_LCE_LINK_high, 1);
        ANR_LCE_X1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_X1_low, ANR_LCE_X1_high, 1);
        ANR_LCE_X2 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_X2_low, ANR_LCE_X2_high, 1);
        ANR_LCE_X3 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_X3_low, ANR_LCE_X3_high, 1);
        ANR_LCE_GAIN0 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_GAIN0_low, ANR_LCE_GAIN0_high, 1);
        ANR_LCE_GAIN1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_GAIN1_low, ANR_LCE_GAIN1_high, 1);
        ANR_LCE_GAIN2 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_GAIN2_low, ANR_LCE_GAIN2_high, 1);
        ANR_LCE_GAIN3 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_GAIN3_low, ANR_LCE_GAIN3_high, 1);
        ANR_LCE_SP0 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_SP0_low, ANR_LCE_SP0_high, 1);
        ANR_LCE_SP1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_SP1_low, ANR_LCE_SP1_high, 1);
        ANR_LCE_SP2 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_SP2_low, ANR_LCE_SP2_high, 1);
        ANR_LCE_SP3 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_SP3_low, ANR_LCE_SP3_high, 1);
        ANR_LCE_C_GAIN = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_C_GAIN_low, ANR_LCE_C_GAIN_high, 1);
        ANR_LCE_SCALE_GAIN = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_SCALE_GAIN_low, ANR_LCE_SCALE_GAIN_high, 1);

        // ANR_MEDIAN_EN
        ANR_MEDIAN_EN = InterParam_NR(ISO, ISO_low, ISO_high, ANR_MEDIAN_EN_low, ANR_MEDIAN_EN_high, 1);
        ANR_COR_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_COR_TH_low, ANR_COR_TH_high, 1);
        ANR_COR_SL = InterParam_NR(ISO, ISO_low, ISO_high, ANR_COR_SL_low, ANR_COR_SL_high, 1);
        ANR_MCD_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_MCD_TH_low, ANR_MCD_TH_high, 1);
        ANR_MCD_SL = InterParam_NR(ISO, ISO_low, ISO_high, ANR_MCD_SL_low, ANR_MCD_SL_high, 1);
        ANR_LCL_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCL_TH_low, ANR_LCL_TH_high, 1);
        ANR_LCL_SL = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCL_SL_low, ANR_LCL_SL_high, 1);
        ANR_LCL_LV = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCL_LV_low, ANR_LCL_LV_high, 1);
        ANR_SCL_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_SCL_TH_low, ANR_SCL_TH_high, 1);
        ANR_SCL_SL = InterParam_NR(ISO, ISO_low, ISO_high, ANR_SCL_SL_low, ANR_SCL_SL_high, 1);
        ANR_SCL_LV = InterParam_NR(ISO, ISO_low, ISO_high, ANR_SCL_LV_low, ANR_SCL_LV_high, 1);
        ANR_NCL_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_NCL_TH_low, ANR_NCL_TH_high, 1);
        ANR_NCL_SL = InterParam_NR(ISO, ISO_low, ISO_high, ANR_NCL_SL_low, ANR_NCL_SL_high, 1);
        ANR_NCL_LV = InterParam_NR(ISO, ISO_low, ISO_high, ANR_NCL_LV_low, ANR_NCL_LV_high, 1);
        ANR_VAR = InterParam_NR(ISO, ISO_low, ISO_high, ANR_VAR_low, ANR_VAR_high, 1);
        ANR_Y0 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y0_low, ANR_Y0_high, 1);
        ANR_Y1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y1_low, ANR_Y1_high, 1);
        ANR_Y2 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y2_low, ANR_Y2_high, 1);
        ANR_Y3 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y3_low, ANR_Y3_high, 1);
        ANR_Y4 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y4_low, ANR_Y4_high, 1);
    }

    // ====================================================================================================================================
    // Negative Conversion
    // ====================================================================================================================================
    ANR_Y_SCALE_SP0 = (ANR_Y_SCALE_SP0 < 0) ? (ANR_Y_SCALE_SP0 + 32) : ANR_Y_SCALE_SP0;
    ANR_Y_SCALE_SP1 = (ANR_Y_SCALE_SP1 < 0) ? (ANR_Y_SCALE_SP1 + 32) : ANR_Y_SCALE_SP1;
    ANR_Y_SCALE_SP2 = (ANR_Y_SCALE_SP2 < 0) ? (ANR_Y_SCALE_SP2 + 32) : ANR_Y_SCALE_SP2;
    ANR_Y_SCALE_SP3 = (ANR_Y_SCALE_SP3 < 0) ? (ANR_Y_SCALE_SP3 + 32) : ANR_Y_SCALE_SP3;
    ANR_C_SCALE_SP0 = (ANR_C_SCALE_SP0 < 0) ? (ANR_C_SCALE_SP0 + 32) : ANR_C_SCALE_SP0;
    ANR_C_SCALE_SP1 = (ANR_C_SCALE_SP1 < 0) ? (ANR_C_SCALE_SP1 + 32) : ANR_C_SCALE_SP1;
    ANR_C_SCALE_SP2 = (ANR_C_SCALE_SP2 < 0) ? (ANR_C_SCALE_SP2 + 32) : ANR_C_SCALE_SP2;
    ANR_C_SCALE_SP3 = (ANR_C_SCALE_SP3 < 0) ? (ANR_C_SCALE_SP3 + 32) : ANR_C_SCALE_SP3;
    ANR_Y_ACT_SP0 = (ANR_Y_ACT_SP0 < 0) ? (ANR_Y_ACT_SP0 + 64) : ANR_Y_ACT_SP0;
    ANR_Y_ACT_SP1 = (ANR_Y_ACT_SP1 < 0) ? (ANR_Y_ACT_SP1 + 64) : ANR_Y_ACT_SP1;
    ANR_Y_ACT_SP2 = (ANR_Y_ACT_SP2 < 0) ? (ANR_Y_ACT_SP2 + 64) : ANR_Y_ACT_SP2;
    ANR_Y_ACT_SP3 = (ANR_Y_ACT_SP3 < 0) ? (ANR_Y_ACT_SP3 + 64) : ANR_Y_ACT_SP3;
    ANR_LCE_SP0 = (ANR_LCE_SP0 < 0) ? (ANR_LCE_SP0 + 32) : ANR_LCE_SP0;
    ANR_LCE_SP1 = (ANR_LCE_SP1 < 0) ? (ANR_LCE_SP1 + 32) : ANR_LCE_SP1;
    ANR_LCE_SP2 = (ANR_LCE_SP2 < 0) ? (ANR_LCE_SP2 + 32) : ANR_LCE_SP2;
    ANR_LCE_SP3 = (ANR_LCE_SP3 < 0) ? (ANR_LCE_SP3 + 32) : ANR_LCE_SP3;
    ANR_HP_B = (ANR_HP_B < 0) ? (ANR_HP_B + 64) : ANR_HP_B;
    ANR_HP_C = (ANR_HP_C < 0) ? (ANR_HP_C + 32) : ANR_HP_C;
    ANR_HP_D = (ANR_HP_D < 0) ? (ANR_HP_D + 16) : ANR_HP_D;
    ANR_HP_E = (ANR_HP_E < 0) ? (ANR_HP_E + 16) : ANR_HP_E;
    ANR_HP_F = (ANR_HP_F < 0) ? (ANR_HP_F + 8)  : ANR_HP_F;

    // ====================================================================================================================================
    // Set Smooth ISO setting
    // ====================================================================================================================================
    // CAM_ANR_CON1 CAM+A20H
    rSmoothANR.con1.bits.ANR1_ENC = ANR_ENC;
    rSmoothANR.con1.bits.ANR1_ENY = ANR_ENY;
    rSmoothANR.con1.bits.ANR_SCALE_MODE = ANR_SCALE_MODE;
    rSmoothANR.con1.bits.ANR1_MEDIAN_EN = ANR_MEDIAN_EN;
    rSmoothANR.con1.bits.ANR1_LCE_LINK = ANR_LCE_LINK;
    rSmoothANR.con1.bits.ANR1_TABLE_EN = ANR_TABLE_EN;
    rSmoothANR.con1.bits.ANR1_TBL_PRC = ANR_TBL_PRC;

    // CAM_ANR_CON2 CAM+A24H
    rSmoothANR.con2.bits.ANR1_IMPL_MODE = ANR_IMPL_MODE;
    rSmoothANR.con2.bits.ANR1_C_MED_EN = ANR_C_MED_EN;
    rSmoothANR.con2.bits.ANR1_C_SM_EDGE = ANR_C_SM_EDGE;
    rSmoothANR.con2.bits.ANR1_FLT_C = ANR_FLT_C;
    rSmoothANR.con2.bits.ANR1_LR = ANR_LR;
    rSmoothANR.con2.bits.ANR1_ALPHA = ANR_ALPHA;

    // CAM_ANR_YAD1 CAM+A2CH
    rSmoothANR.yad1.bits.ANR1_CEN_GAIN_LO_TH = ANR_CEN_GAIN_LO_TH;
    rSmoothANR.yad1.bits.ANR1_CEN_GAIN_HI_TH = ANR_CEN_GAIN_HI_TH;
    rSmoothANR.yad1.bits.ANR1_K_LO_TH = ANR_K_LO_TH;
    rSmoothANR.yad1.bits.ANR1_K_HI_TH = ANR_K_HI_TH;
    rSmoothANR.yad1.bits.ANR1_K_TH_C = ANR_K_TH_C;

    // CAM_ANR_YAD2 CAM+A30H
    rSmoothANR.yad2.bits.ANR1_PTY_VGAIN = ANR_PTY_VGAIN;
    rSmoothANR.yad2.bits.ANR1_PTY_GAIN_TH = ANR_PTY_GAIN_TH;

    // CAM_ANR_Y4LUT1 CAM+A34H
    rSmoothANR.lut1.bits.ANR1_Y_CPX1 = ANR_Y_CPX1;
    rSmoothANR.lut1.bits.ANR1_Y_CPX2 = ANR_Y_CPX2;
    rSmoothANR.lut1.bits.ANR1_Y_CPX3 = ANR_Y_CPX3;

    // CAM_ANR_Y4LUT2 CAM+A38H
    rSmoothANR.lut2.bits.ANR1_Y_SCALE_CPY0 = ANR_Y_SCALE_CPY0;
    rSmoothANR.lut2.bits.ANR1_Y_SCALE_CPY1 = ANR_Y_SCALE_CPY1;
    rSmoothANR.lut2.bits.ANR1_Y_SCALE_CPY2 = ANR_Y_SCALE_CPY2;
    rSmoothANR.lut2.bits.ANR1_Y_SCALE_CPY3 = ANR_Y_SCALE_CPY3;

    // CAM_ANR_Y4LUT3 CAM+A3CH
    rSmoothANR.lut3.bits.ANR1_Y_SCALE_SP0 = ANR_Y_SCALE_SP0;
    rSmoothANR.lut3.bits.ANR1_Y_SCALE_SP1 = ANR_Y_SCALE_SP1;
    rSmoothANR.lut3.bits.ANR1_Y_SCALE_SP2 = ANR_Y_SCALE_SP2;
    rSmoothANR.lut3.bits.ANR1_Y_SCALE_SP3 = ANR_Y_SCALE_SP3;

    // CAM_ANR_C4LUT1
    rSmoothANR.clut1.bits.ANR1_C_CPX1 = ANR_C_CPX1;
    rSmoothANR.clut1.bits.ANR1_C_CPX2 = ANR_C_CPX2;
    rSmoothANR.clut1.bits.ANR1_C_CPX3 = ANR_C_CPX3;

    // CAM_ANR_C4LUT2
    rSmoothANR.clut2.bits.ANR1_C_SCALE_CPY0 = ANR_C_SCALE_CPY0;
    rSmoothANR.clut2.bits.ANR1_C_SCALE_CPY1 = ANR_C_SCALE_CPY1;
    rSmoothANR.clut2.bits.ANR1_C_SCALE_CPY2 = ANR_C_SCALE_CPY2;
    rSmoothANR.clut2.bits.ANR1_C_SCALE_CPY3 = ANR_C_SCALE_CPY3;

    // CAM_ANR_C4LUT3
    rSmoothANR.clut3.bits.ANR1_C_SCALE_SP0 = ANR_C_SCALE_SP0;
    rSmoothANR.clut3.bits.ANR1_C_SCALE_SP1 = ANR_C_SCALE_SP1;
    rSmoothANR.clut3.bits.ANR1_C_SCALE_SP2 = ANR_C_SCALE_SP2;
    rSmoothANR.clut3.bits.ANR1_C_SCALE_SP3 = ANR_C_SCALE_SP3;

    // CAM_ANR_A4LUT2
    rSmoothANR.alut2.bits.ANR1_Y_ACT_CPY0 = ANR_Y_ACT_CPY0;
    rSmoothANR.alut2.bits.ANR1_Y_ACT_CPY1 = ANR_Y_ACT_CPY1;
    rSmoothANR.alut2.bits.ANR1_Y_ACT_CPY2 = ANR_Y_ACT_CPY2;
    rSmoothANR.alut2.bits.ANR1_Y_ACT_CPY3 = ANR_Y_ACT_CPY3;

    // CAM_ANR_A4LUT3
    rSmoothANR.alut3.bits.ANR1_Y_ACT_SP0 = ANR_Y_ACT_SP0;
    rSmoothANR.alut3.bits.ANR1_Y_ACT_SP1 = ANR_Y_ACT_SP1;
    rSmoothANR.alut3.bits.ANR1_Y_ACT_SP2 = ANR_Y_ACT_SP2;
    rSmoothANR.alut3.bits.ANR1_Y_ACT_SP3 = ANR_Y_ACT_SP3;

    // CAM_ANR_L4LUT1
    rSmoothANR.llut1.bits.ANR1_LCE_X1 = ANR_LCE_X1;
    rSmoothANR.llut1.bits.ANR1_LCE_X2 = ANR_LCE_X2;
    rSmoothANR.llut1.bits.ANR1_LCE_X3 = ANR_LCE_X3;

    // CAM_ANR_L4LUT2
    rSmoothANR.llut2.bits.ANR1_LCE_GAIN0 = ANR_LCE_GAIN0;
    rSmoothANR.llut2.bits.ANR1_LCE_GAIN1 = ANR_LCE_GAIN1;
    rSmoothANR.llut2.bits.ANR1_LCE_GAIN2 = ANR_LCE_GAIN2;
    rSmoothANR.llut2.bits.ANR1_LCE_GAIN3 = ANR_LCE_GAIN3;

    // CAM_ANR_L4LUT3
    rSmoothANR.llut3.bits.ANR1_LCE_SP0 = ANR_LCE_SP0;
    rSmoothANR.llut3.bits.ANR1_LCE_SP1 = ANR_LCE_SP1;
    rSmoothANR.llut3.bits.ANR1_LCE_SP2 = ANR_LCE_SP2;
    rSmoothANR.llut3.bits.ANR1_LCE_SP3 = ANR_LCE_SP3;

    // CAM_ANR_PTY CAM+A40H
    rSmoothANR.pty.bits.ANR1_PTY1 = ANR_PTY1;
    rSmoothANR.pty.bits.ANR1_PTY2 = ANR_PTY2;
    rSmoothANR.pty.bits.ANR1_PTY3 = ANR_PTY3;
    rSmoothANR.pty.bits.ANR1_PTY4 = ANR_PTY4;

    // CAM_ANR_CAD CAM+A44H
    rSmoothANR.cad.bits.ANR1_PTC_VGAIN = ANR_PTC_VGAIN;
    rSmoothANR.cad.bits.ANR1_PTC_GAIN_TH = ANR_PTC_GAIN_TH;
    rSmoothANR.cad.bits.ANR1_C_L_DIFF_TH = ANR_C_L_DIFF_TH;

    // CAM_ANR_PTC CAM+A48H
    rSmoothANR.ptc.bits.ANR1_PTC1 = ANR_PTC1;
    rSmoothANR.ptc.bits.ANR1_PTC2 = ANR_PTC2;
    rSmoothANR.ptc.bits.ANR1_PTC3 = ANR_PTC3;
    rSmoothANR.ptc.bits.ANR1_PTC4 = ANR_PTC4;

    // CAM_ANR_LCE CAM+A4CH
    rSmoothANR.lce.bits.ANR1_LCE_C_GAIN = ANR_LCE_C_GAIN;
    rSmoothANR.lce.bits.ANR1_LCE_SCALE_GAIN = ANR_LCE_SCALE_GAIN;

    // CAM_ANR_MED1
    rSmoothANR.med1.bits.ANR1_COR_TH = ANR_COR_TH;
    rSmoothANR.med1.bits.ANR1_COR_SL = ANR_COR_SL;
    rSmoothANR.med1.bits.ANR1_MCD_TH = ANR_MCD_TH;
    rSmoothANR.med1.bits.ANR1_MCD_SL = ANR_MCD_SL;
    rSmoothANR.med1.bits.ANR1_LCL_TH = ANR_LCL_TH;

    // CAM_ANR_MED2
    rSmoothANR.med2.bits.ANR1_LCL_SL = ANR_LCL_SL;
    rSmoothANR.med2.bits.ANR1_LCL_LV = ANR_LCL_LV;
    rSmoothANR.med2.bits.ANR1_SCL_TH = ANR_SCL_TH;
    rSmoothANR.med2.bits.ANR1_SCL_SL = ANR_SCL_SL;
    rSmoothANR.med2.bits.ANR1_SCL_LV = ANR_SCL_LV;

    // CAM_ANR_MED3
    rSmoothANR.med3.bits.ANR1_NCL_TH = ANR_NCL_TH;
    rSmoothANR.med3.bits.ANR1_NCL_SL = ANR_NCL_SL;
    rSmoothANR.med3.bits.ANR1_NCL_LV = ANR_NCL_LV;
    rSmoothANR.med3.bits.ANR1_VAR = ANR_VAR;
    rSmoothANR.med3.bits.ANR1_Y0 = ANR_Y0;

    // CAM_ANR_MED4
    rSmoothANR.med4.bits.ANR1_Y1 = ANR_Y1;
    rSmoothANR.med4.bits.ANR1_Y2 = ANR_Y2;
    rSmoothANR.med4.bits.ANR1_Y3 = ANR_Y3;
    rSmoothANR.med4.bits.ANR1_Y4 = ANR_Y4;

    // CAM_ANR_HP1 CAM+A54H
    rSmoothANR.hp1.bits.ANR1_HP_A = ANR_HP_A;
    rSmoothANR.hp1.bits.ANR1_HP_B = ANR_HP_B;
    rSmoothANR.hp1.bits.ANR1_HP_C = ANR_HP_C;
    rSmoothANR.hp1.bits.ANR1_HP_D = ANR_HP_D;
    rSmoothANR.hp1.bits.ANR1_HP_E = ANR_HP_E;

    // CAM_ANR_HP2 CAM+A58H
    rSmoothANR.hp2.bits.ANR1_HP_S1 = ANR_HP_S1;
    rSmoothANR.hp2.bits.ANR1_HP_S2 = ANR_HP_S2;
    rSmoothANR.hp2.bits.ANR1_HP_X1 = ANR_HP_X1;
    rSmoothANR.hp2.bits.ANR1_HP_F = ANR_HP_F;

    // CAM_ANR_HP3 CAM+A5CH
    rSmoothANR.hp3.bits.ANR1_HP_Y_GAIN_CLIP = ANR_HP_Y_GAIN_CLIP;
    rSmoothANR.hp3.bits.ANR1_HP_Y_SP = ANR_HP_Y_SP;
    rSmoothANR.hp3.bits.ANR1_HP_Y_LO = ANR_HP_Y_LO;
    rSmoothANR.hp3.bits.ANR1_HP_CLIP = ANR_HP_CLIP;

    // CAM_ANR_ACTY CAM+A60H
    rSmoothANR.acty.bits.ANR1_ACT_TH_Y = ANR_ACT_TH_Y;
    rSmoothANR.acty.bits.ANR1_ACT_BLD_BASE_Y = ANR_ACT_BLD_BASE_Y;
    rSmoothANR.acty.bits.ANR1_ACT_SLANT_Y = ANR_ACT_SLANT_Y;
    rSmoothANR.acty.bits.ANR1_ACT_BLD_TH_Y = ANR_ACT_BLD_TH_Y;

    // CAM_ANR_ACT1 CAM+A64H
    rSmoothANR.act1.bits.ANR1_ACT_LCE_GAIN = ANR_ACT_LCE_GAIN;
    rSmoothANR.act1.bits.ANR1_ACT_SCALE_OFT = ANR_ACT_SCALE_OFT;
    rSmoothANR.act1.bits.ANR1_ACT_SCALE_GAIN = ANR_ACT_SCALE_GAIN;
    rSmoothANR.act1.bits.ANR1_ACT_DIF_GAIN = ANR_ACT_DIF_GAIN;
    rSmoothANR.act1.bits.ANR1_ACT_DIF_LO_TH = ANR_ACT_DIF_LO_TH;

    // CAM_ANR_ACT2 CAM+A64H
    rSmoothANR.act2.bits.ANR1_ACT_SIZE_GAIN = ANR_ACT_SIZE_GAIN;
    rSmoothANR.act2.bits.ANR1_ACT_SIZE_LO_TH = ANR_ACT_SIZE_LO_TH;
    rSmoothANR.act2.bits.ANR1_COR_TH1 = ANR_COR_TH1;
    rSmoothANR.act2.bits.ANR1_COR_SL1 = ANR_COR_SL1;

    // CAM_ANR_ACT3 CAM+A64H
    rSmoothANR.act3.bits.ANR1_COR_ACT_TH = ANR_COR_ACT_TH;
    rSmoothANR.act3.bits.ANR1_COR_ACT_SL1 = ANR_COR_ACT_SL1;
    rSmoothANR.act3.bits.ANR1_COR_ACT_SL2 = ANR_COR_ACT_SL2;

    // CAM_ANR_ACTC CAM+A64H
    rSmoothANR.actc.bits.ANR1_ACT_TH_C = ANR_ACT_TH_C;
    rSmoothANR.actc.bits.ANR1_ACT_BLD_BASE_C = ANR_ACT_BLD_BASE_C;
    rSmoothANR.actc.bits.ANR1_ACT_SLANT_C = ANR_ACT_SLANT_C;
    rSmoothANR.actc.bits.ANR1_ACT_BLD_TH_C = ANR_ACT_BLD_TH_C;

#define ANR_DUMP(item1, item2)   \
        do{                      \
            MY_LOG_IF(bSmoothANRDebug, "[L,S,U]%22s = %d, %d, %d", #item2, rLowerANR.item1.bits.item2, rSmoothANR.item1.bits.item2, rUpperANR.item1.bits.item2); \
        }while(0)

    //ANR_DUMP(tbl, ANR1_TBL_Y0);
    //ANR_DUMP(tbl, ANR1_TBL_U0);
    //ANR_DUMP(tbl, ANR1_TBL_V0);
    //ANR_DUMP(tbl, ANR1_TBL_Y1);
    //ANR_DUMP(tbl, ANR1_TBL_U1);
    //ANR_DUMP(tbl, ANR1_TBL_V1);

    ANR_DUMP(con1, ANR1_ENC);
    ANR_DUMP(con1, ANR1_ENY);
    ANR_DUMP(con1, ANR_SCALE_MODE);
    ANR_DUMP(con1, ANR1_MEDIAN_EN);
    ANR_DUMP(con1, ANR1_LCE_LINK);
    ANR_DUMP(con1, ANR1_TABLE_EN);
    ANR_DUMP(con1, ANR1_TBL_PRC);

    ANR_DUMP(con2, ANR1_IMPL_MODE);
    ANR_DUMP(con2, ANR1_C_MED_EN);
    ANR_DUMP(con2, ANR1_C_SM_EDGE);
    ANR_DUMP(con2, ANR1_FLT_C);
    ANR_DUMP(con2, ANR1_LR);
    ANR_DUMP(con2, ANR1_ALPHA);

    ANR_DUMP(yad1, ANR1_CEN_GAIN_LO_TH);
    ANR_DUMP(yad1, ANR1_CEN_GAIN_HI_TH);
    ANR_DUMP(yad1, ANR1_K_LO_TH);
    ANR_DUMP(yad1, ANR1_K_HI_TH);
    ANR_DUMP(yad1, ANR1_K_TH_C);

    ANR_DUMP(yad2, ANR1_PTY_VGAIN);
    ANR_DUMP(yad2, ANR1_PTY_GAIN_TH);

    ANR_DUMP(lut1, ANR1_Y_CPX1);
    ANR_DUMP(lut1, ANR1_Y_CPX2);
    ANR_DUMP(lut1, ANR1_Y_CPX3);

    ANR_DUMP(lut2, ANR1_Y_SCALE_CPY0);
    ANR_DUMP(lut2, ANR1_Y_SCALE_CPY1);
    ANR_DUMP(lut2, ANR1_Y_SCALE_CPY2);
    ANR_DUMP(lut2, ANR1_Y_SCALE_CPY3);

    ANR_DUMP(lut3, ANR1_Y_SCALE_SP0);
    ANR_DUMP(lut3, ANR1_Y_SCALE_SP1);
    ANR_DUMP(lut3, ANR1_Y_SCALE_SP2);
    ANR_DUMP(lut3, ANR1_Y_SCALE_SP3);

    ANR_DUMP(clut1, ANR1_C_CPX1);
    ANR_DUMP(clut1, ANR1_C_CPX2);
    ANR_DUMP(clut1, ANR1_C_CPX3);

    ANR_DUMP(clut2, ANR1_C_SCALE_CPY0);
    ANR_DUMP(clut2, ANR1_C_SCALE_CPY1);
    ANR_DUMP(clut2, ANR1_C_SCALE_CPY2);
    ANR_DUMP(clut2, ANR1_C_SCALE_CPY3);

    ANR_DUMP(clut3, ANR1_C_SCALE_SP0);
    ANR_DUMP(clut3, ANR1_C_SCALE_SP1);
    ANR_DUMP(clut3, ANR1_C_SCALE_SP2);
    ANR_DUMP(clut3, ANR1_C_SCALE_SP3);

    ANR_DUMP(alut2, ANR1_Y_ACT_CPY0);
    ANR_DUMP(alut2, ANR1_Y_ACT_CPY1);
    ANR_DUMP(alut2, ANR1_Y_ACT_CPY2);
    ANR_DUMP(alut2, ANR1_Y_ACT_CPY3);

    ANR_DUMP(alut3, ANR1_Y_ACT_SP0);
    ANR_DUMP(alut3, ANR1_Y_ACT_SP1);
    ANR_DUMP(alut3, ANR1_Y_ACT_SP2);
    ANR_DUMP(alut3, ANR1_Y_ACT_SP3);

    ANR_DUMP(llut1, ANR1_LCE_X1);
    ANR_DUMP(llut1, ANR1_LCE_X2);
    ANR_DUMP(llut1, ANR1_LCE_X3);

    ANR_DUMP(llut2, ANR1_LCE_GAIN0);
    ANR_DUMP(llut2, ANR1_LCE_GAIN1);
    ANR_DUMP(llut2, ANR1_LCE_GAIN2);
    ANR_DUMP(llut2, ANR1_LCE_GAIN3);

    ANR_DUMP(llut3, ANR1_LCE_SP0);
    ANR_DUMP(llut3, ANR1_LCE_SP1);
    ANR_DUMP(llut3, ANR1_LCE_SP2);
    ANR_DUMP(llut3, ANR1_LCE_SP3);

    ANR_DUMP(pty, ANR1_PTY1);
    ANR_DUMP(pty, ANR1_PTY2);
    ANR_DUMP(pty, ANR1_PTY3);
    ANR_DUMP(pty, ANR1_PTY4);

    ANR_DUMP(cad, ANR1_PTC_VGAIN);
    ANR_DUMP(cad, ANR1_PTC_GAIN_TH);
    ANR_DUMP(cad, ANR1_C_L_DIFF_TH);

    ANR_DUMP(ptc, ANR1_PTC1);
    ANR_DUMP(ptc, ANR1_PTC2);
    ANR_DUMP(ptc, ANR1_PTC3);
    ANR_DUMP(ptc, ANR1_PTC4);

    ANR_DUMP(lce, ANR1_LCE_C_GAIN);
    ANR_DUMP(lce, ANR1_LCE_SCALE_GAIN);

    ANR_DUMP(med1, ANR1_COR_TH);
    ANR_DUMP(med1, ANR1_COR_SL);
    ANR_DUMP(med1, ANR1_MCD_TH);
    ANR_DUMP(med1, ANR1_MCD_SL);
    ANR_DUMP(med1, ANR1_LCL_TH);

    ANR_DUMP(med2, ANR1_LCL_SL);
    ANR_DUMP(med2, ANR1_LCL_LV);
    ANR_DUMP(med2, ANR1_SCL_TH);
    ANR_DUMP(med2, ANR1_SCL_SL);
    ANR_DUMP(med2, ANR1_SCL_LV);

    ANR_DUMP(med3, ANR1_NCL_TH);
    ANR_DUMP(med3, ANR1_NCL_SL);
    ANR_DUMP(med3, ANR1_NCL_LV);
    ANR_DUMP(med3, ANR1_VAR);
    ANR_DUMP(med3, ANR1_Y0);

    ANR_DUMP(med4, ANR1_Y1);
    ANR_DUMP(med4, ANR1_Y2);
    ANR_DUMP(med4, ANR1_Y3);
    ANR_DUMP(med4, ANR1_Y4);

    ANR_DUMP(hp1, ANR1_HP_A);
    ANR_DUMP(hp1, ANR1_HP_B);
    ANR_DUMP(hp1, ANR1_HP_C);
    ANR_DUMP(hp1, ANR1_HP_D);
    ANR_DUMP(hp1, ANR1_HP_E);

    ANR_DUMP(hp2, ANR1_HP_S1);
    ANR_DUMP(hp2, ANR1_HP_S2);
    ANR_DUMP(hp2, ANR1_HP_X1);
    ANR_DUMP(hp2, ANR1_HP_F);

    ANR_DUMP(hp3, ANR1_HP_Y_GAIN_CLIP);
    ANR_DUMP(hp3, ANR1_HP_Y_SP);
    ANR_DUMP(hp3, ANR1_HP_Y_LO);
    ANR_DUMP(hp3, ANR1_HP_CLIP);

    ANR_DUMP(acty, ANR1_ACT_TH_Y);
    ANR_DUMP(acty, ANR1_ACT_BLD_BASE_Y);
    ANR_DUMP(acty, ANR1_ACT_SLANT_Y);
    ANR_DUMP(acty, ANR1_ACT_BLD_TH_Y);

    ANR_DUMP(act1, ANR1_ACT_LCE_GAIN);
    ANR_DUMP(act1, ANR1_ACT_SCALE_OFT);
    ANR_DUMP(act1, ANR1_ACT_SCALE_GAIN);
    ANR_DUMP(act1, ANR1_ACT_DIF_GAIN);
    ANR_DUMP(act1, ANR1_ACT_DIF_LO_TH);

    ANR_DUMP(act2, ANR1_ACT_SIZE_GAIN);
    ANR_DUMP(act2, ANR1_ACT_SIZE_LO_TH);
    ANR_DUMP(act2, ANR1_COR_TH1);
    ANR_DUMP(act2, ANR1_COR_SL1);

    ANR_DUMP(act3, ANR1_COR_ACT_TH);
    ANR_DUMP(act3, ANR1_COR_ACT_SL1);
    ANR_DUMP(act3, ANR1_COR_ACT_SL2);

    ANR_DUMP(actc, ANR1_ACT_TH_C);
    ANR_DUMP(actc, ANR1_ACT_BLD_BASE_C);
    ANR_DUMP(actc, ANR1_ACT_SLANT_C);
    ANR_DUMP(actc, ANR1_ACT_BLD_TH_C);

#undef ANR_DUMP
}

MVOID SmoothANR_TBL(MUINT32 u4RealISO, // Real ISO
               MUINT32 u4UpperISO, // Upper ISO
               MUINT32 u4LowerISO, // Lower ISO
               ISP_NVRAM_ANR_LUT_T const& rUpperANR_TBL, // ANR TBL for upper ISO
               ISP_NVRAM_ANR_LUT_T const& rLowerANR_TBL, // ANR TBL for lower ISO
               ISP_NVRAM_ANR_LUT_T& rSmoothANR_TBL)  // Output
{

	MBOOL bSmoothANR_TBL_Debug = interpPropertyGet("vendor.debug.smooth_anr_tbl.enable", MFALSE);

    MY_LOG_IF(bSmoothANR_TBL_Debug,"[%s()] u4RealISO: %d, u4UpperISO = %d, u4LowerISO = %d\n", __FUNCTION__, u4RealISO, u4UpperISO, u4LowerISO);

	MINT32 ISO = u4RealISO;
    MINT32 ISO_low = u4LowerISO;
    MINT32 ISO_high = u4UpperISO;

	for(int i = 0; i < ANR_HW_TBL_NUM; ++i){

		// Get Lower ISO setting
		MINT32 ANR_TBL_Y0_low = rLowerANR_TBL.lut[i].bits.ANR1_TBL_Y0;
		MINT32 ANR_TBL_U0_low = rLowerANR_TBL.lut[i].bits.ANR1_TBL_U0;
		MINT32 ANR_TBL_V0_low = rLowerANR_TBL.lut[i].bits.ANR1_TBL_V0;
		MINT32 ANR_TBL_Y1_low = rLowerANR_TBL.lut[i].bits.ANR1_TBL_Y1;
		MINT32 ANR_TBL_U1_low = rLowerANR_TBL.lut[i].bits.ANR1_TBL_U1;
		MINT32 ANR_TBL_V1_low = rLowerANR_TBL.lut[i].bits.ANR1_TBL_V1;

		// Get Higher ISO setting
		MINT32 ANR_TBL_Y0_high = rUpperANR_TBL.lut[i].bits.ANR1_TBL_Y0;
		MINT32 ANR_TBL_U0_high = rUpperANR_TBL.lut[i].bits.ANR1_TBL_U0;
		MINT32 ANR_TBL_V0_high = rUpperANR_TBL.lut[i].bits.ANR1_TBL_V0;
		MINT32 ANR_TBL_Y1_high = rUpperANR_TBL.lut[i].bits.ANR1_TBL_Y1;
		MINT32 ANR_TBL_U1_high = rUpperANR_TBL.lut[i].bits.ANR1_TBL_U1;
		MINT32 ANR_TBL_V1_high = rUpperANR_TBL.lut[i].bits.ANR1_TBL_V1;

		// Start Parameter Interpolation
		MINT32 ANR_TBL_Y0;
		MINT32 ANR_TBL_U0;
		MINT32 ANR_TBL_V0;
		MINT32 ANR_TBL_Y1;
		MINT32 ANR_TBL_U1;
		MINT32 ANR_TBL_V1;

		ANR_TBL_Y0 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_TBL_Y0_low, ANR_TBL_Y0_high, 0);
		ANR_TBL_U0 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_TBL_U0_low, ANR_TBL_U0_high, 0);
		ANR_TBL_V0 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_TBL_V0_low, ANR_TBL_V0_high, 0);
		ANR_TBL_Y1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_TBL_Y1_low, ANR_TBL_Y1_high, 0);
		ANR_TBL_U1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_TBL_U1_low, ANR_TBL_U1_high, 0);
		ANR_TBL_V1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_TBL_V1_low, ANR_TBL_V1_high, 0);

		rSmoothANR_TBL.lut[i].bits.ANR1_TBL_Y0 = ANR_TBL_Y0;
		rSmoothANR_TBL.lut[i].bits.ANR1_TBL_U0 = ANR_TBL_U0;
		rSmoothANR_TBL.lut[i].bits.ANR1_TBL_V0 = ANR_TBL_V0;
		rSmoothANR_TBL.lut[i].bits.ANR1_TBL_Y1 = ANR_TBL_Y1;
		rSmoothANR_TBL.lut[i].bits.ANR1_TBL_U1 = ANR_TBL_U1;
		rSmoothANR_TBL.lut[i].bits.ANR1_TBL_V1 = ANR_TBL_V1;

	}

}

MVOID SmoothANR2(MUINT32 u4RealISO, // Real ISO
                 MUINT32 u4UpperISO, // Upper ISO
                 MUINT32 u4LowerISO, // Lower ISO
                 ISP_NVRAM_ANR2_T const& rUpperANR, // ANR settings for upper ISO
                 ISP_NVRAM_ANR2_T const& rLowerANR, // ANR settings for lower ISO
                 ISP_NVRAM_ANR2_T& rSmoothANR)   // Output
{
	MBOOL bSmoothANRDebug = interpPropertyGet("vendor.debug.smooth_anr.enable", MFALSE);

    MY_LOG_IF(bSmoothANRDebug,"[%s()] u4RealISO: %d, u4UpperISO = %d, u4LowerISO = %d\n", __FUNCTION__, u4RealISO, u4UpperISO, u4LowerISO);

    // ====================================================================================================================================
    // Get Lower ISO setting
    // ====================================================================================================================================
    // CAM_ANR2_CON1 CAM+A20H
    MINT32 ANR_ENC_low = rLowerANR.con1.bits.ANR2_ENC;
    MINT32 ANR_ENY_low = rLowerANR.con1.bits.ANR2_ENY;
    MINT32 ANR_SCALE_MODE_low = rLowerANR.con1.bits.ANR2_SCALE_MODE;
    MINT32 ANR_MODE_low = rLowerANR.con1.bits.ANR2_MODE;
    MINT32 ANR_LCE_LINK_low = rLowerANR.con1.bits.ANR2_LCE_LINK;

    // CAM_ANR2_CON2 CAM+A24H
    MINT32 ANR_C_MED_EN_low = rLowerANR.con2.bits.ANR2_C_MED_EN;
    MINT32 ANR_C_SM_EDGE_low = rLowerANR.con2.bits.ANR2_C_SM_EDGE;
    MINT32 ANR_FLT_C_low = rLowerANR.con2.bits.ANR2_FLT_C;

    // CAM_ANR_YAD1 CAM+A2CH
    MINT32 ANR_K_TH_C_low = rLowerANR.yad1.bits.ANR2_K_TH_C;

    // CAM_ANR_Y4LUT1 CAM+A34H
    MINT32 ANR_Y_CPX1_low = rLowerANR.lut1.bits.ANR2_Y_CPX1;
    MINT32 ANR_Y_CPX2_low = rLowerANR.lut1.bits.ANR2_Y_CPX2;
    MINT32 ANR_Y_CPX3_low = rLowerANR.lut1.bits.ANR2_Y_CPX3;

    // CAM_ANR_Y4LUT2 CAM+A38H
    MINT32 ANR_Y_SCALE_CPY0_low = rLowerANR.lut2.bits.ANR2_Y_SCALE_CPY0;
    MINT32 ANR_Y_SCALE_CPY1_low = rLowerANR.lut2.bits.ANR2_Y_SCALE_CPY1;
    MINT32 ANR_Y_SCALE_CPY2_low = rLowerANR.lut2.bits.ANR2_Y_SCALE_CPY2;
    MINT32 ANR_Y_SCALE_CPY3_low = rLowerANR.lut2.bits.ANR2_Y_SCALE_CPY3;

    // CAM_ANR_Y4LUT3 CAM+A3CH
    MINT32 ANR_Y_SCALE_SP0_low = rLowerANR.lut3.bits.ANR2_Y_SCALE_SP0;
    MINT32 ANR_Y_SCALE_SP1_low = rLowerANR.lut3.bits.ANR2_Y_SCALE_SP1;
    MINT32 ANR_Y_SCALE_SP2_low = rLowerANR.lut3.bits.ANR2_Y_SCALE_SP2;
    MINT32 ANR_Y_SCALE_SP3_low = rLowerANR.lut3.bits.ANR2_Y_SCALE_SP3;

    // CAM_ANR_L4LUT1
    MINT32 ANR_LCE_X1_low = rLowerANR.llut1.bits.ANR2_LCE_X1;
    MINT32 ANR_LCE_X2_low = rLowerANR.llut1.bits.ANR2_LCE_X2;
    MINT32 ANR_LCE_X3_low = rLowerANR.llut1.bits.ANR2_LCE_X3;

    // CAM_ANR_L4LUT2
    MINT32 ANR_LCE_GAIN0_low = rLowerANR.llut2.bits.ANR2_LCE_GAIN0;
    MINT32 ANR_LCE_GAIN1_low = rLowerANR.llut2.bits.ANR2_LCE_GAIN1;
    MINT32 ANR_LCE_GAIN2_low = rLowerANR.llut2.bits.ANR2_LCE_GAIN2;
    MINT32 ANR_LCE_GAIN3_low = rLowerANR.llut2.bits.ANR2_LCE_GAIN3;

    // CAM_ANR_L4LUT3
    MINT32 ANR_LCE_SP0_low = rLowerANR.llut3.bits.ANR2_LCE_SP0;
    MINT32 ANR_LCE_SP1_low = rLowerANR.llut3.bits.ANR2_LCE_SP1;
    MINT32 ANR_LCE_SP2_low = rLowerANR.llut3.bits.ANR2_LCE_SP2;
    MINT32 ANR_LCE_SP3_low = rLowerANR.llut3.bits.ANR2_LCE_SP3;

    // CAM_ANR_CAD CAM+A44H
    MINT32 ANR_PTC_VGAIN_low = rLowerANR.cad.bits.ANR2_PTC_VGAIN;
    MINT32 ANR_C_GAIN_low = rLowerANR.cad.bits.ANR2_C_GAIN;
    MINT32 ANR_PTC_GAIN_TH_low = rLowerANR.cad.bits.ANR2_PTC_GAIN_TH;
    MINT32 ANR_C_L_DIFF_TH_low = rLowerANR.cad.bits.ANR2_C_L_DIFF_TH;

    // CAM_ANR_PTC CAM+A48H
    MINT32 ANR_PTC1_low = rLowerANR.ptc.bits.ANR2_PTC1;
    MINT32 ANR_PTC2_low = rLowerANR.ptc.bits.ANR2_PTC2;
    MINT32 ANR_PTC3_low = rLowerANR.ptc.bits.ANR2_PTC3;
    MINT32 ANR_PTC4_low = rLowerANR.ptc.bits.ANR2_PTC4;

    // CAM_ANR_LCE CAM+A4CH
    MINT32 ANR_LCE_C_GAIN_low = rLowerANR.lce.bits.ANR2_LCE_C_GAIN;

    // CAM_ANR_MED1
    MINT32 ANR_COR_TH_low = rLowerANR.med1.bits.ANR2_COR_TH;
    MINT32 ANR_COR_SL_low = rLowerANR.med1.bits.ANR2_COR_SL;
    MINT32 ANR_MCD_TH_low = rLowerANR.med1.bits.ANR2_MCD_TH;
    MINT32 ANR_MCD_SL_low = rLowerANR.med1.bits.ANR2_MCD_SL;
    MINT32 ANR_LCL_TH_low = rLowerANR.med1.bits.ANR2_LCL_TH;

    // CAM_ANR_MED2
    MINT32 ANR_LCL_SL_low = rLowerANR.med2.bits.ANR2_LCL_SL;
    MINT32 ANR_LCL_LV_low = rLowerANR.med2.bits.ANR2_LCL_LV;
    MINT32 ANR_SCL_TH_low = rLowerANR.med2.bits.ANR2_SCL_TH;
    MINT32 ANR_SCL_SL_low = rLowerANR.med2.bits.ANR2_SCL_SL;
    MINT32 ANR_SCL_LV_low = rLowerANR.med2.bits.ANR2_SCL_LV;

    // CAM_ANR_MED3
    MINT32 ANR_NCL_TH_low = rLowerANR.med3.bits.ANR2_NCL_TH;
    MINT32 ANR_NCL_SL_low = rLowerANR.med3.bits.ANR2_NCL_SL;
    MINT32 ANR_NCL_LV_low = rLowerANR.med3.bits.ANR2_NCL_LV;
    MINT32 ANR_VAR_low = rLowerANR.med3.bits.ANR2_VAR;
    MINT32 ANR_Y0_low = rLowerANR.med3.bits.ANR2_Y0;

    // CAM_ANR_MED4
    MINT32 ANR_Y1_low = rLowerANR.med4.bits.ANR2_Y1;
    MINT32 ANR_Y2_low = rLowerANR.med4.bits.ANR2_Y2;
    MINT32 ANR_Y3_low = rLowerANR.med4.bits.ANR2_Y3;
    MINT32 ANR_Y4_low = rLowerANR.med4.bits.ANR2_Y4;

    // CAM_ANR_ACTY CAM+A60H
    MINT32 ANR_ACT_TH_Y_low = rLowerANR.acty.bits.ANR2_ACT_TH_Y;
    MINT32 ANR_ACT_BLD_BASE_Y_low= rLowerANR.acty.bits.ANR2_ACT_BLD_BASE_Y;
    MINT32 ANR_ACT_SLANT_Y_low = rLowerANR.acty.bits.ANR2_ACT_SLANT_Y;
    MINT32 ANR_ACT_BLD_TH_Y_low = rLowerANR.acty.bits.ANR2_ACT_BLD_TH_Y;

    // CAM_ANR_ACTC CAM+A64H
    MINT32 ANR_ACT_TH_C_low = rLowerANR.actc.bits.ANR2_ACT_TH_C;
    MINT32 ANR_ACT_BLD_BASE_C_low= rLowerANR.actc.bits.ANR2_ACT_BLD_BASE_C;
    MINT32 ANR_ACT_SLANT_C_low = rLowerANR.actc.bits.ANR2_ACT_SLANT_C;
    MINT32 ANR_ACT_BLD_TH_C_low = rLowerANR.actc.bits.ANR2_ACT_BLD_TH_C;

    // ====================================================================================================================================
    // Get Upper ISO setting
    // ====================================================================================================================================
    // CAM_ANR_CON1 CAM+A20H
    MINT32 ANR_ENC_high = rUpperANR.con1.bits.ANR2_ENC;
    MINT32 ANR_ENY_high = rUpperANR.con1.bits.ANR2_ENY;
    MINT32 ANR_SCALE_MODE_high = rUpperANR.con1.bits.ANR2_SCALE_MODE;
    MINT32 ANR_MODE_high = rUpperANR.con1.bits.ANR2_MODE;
    MINT32 ANR_LCE_LINK_high = rUpperANR.con1.bits.ANR2_LCE_LINK;

    // CAM_ANR_CON2 CAM+A24H
    MINT32 ANR_C_MED_EN_high = rUpperANR.con2.bits.ANR2_C_MED_EN;
    MINT32 ANR_C_SM_EDGE_high = rUpperANR.con2.bits.ANR2_C_SM_EDGE;
    MINT32 ANR_FLT_C_high = rUpperANR.con2.bits.ANR2_FLT_C;

    // CAM_ANR_YAD1 CAM+A2CH
    MINT32 ANR_K_TH_C_high = rUpperANR.yad1.bits.ANR2_K_TH_C;

    // CAM_ANR_Y4LUT1 CAM+A34H
    MINT32 ANR_Y_CPX1_high = rUpperANR.lut1.bits.ANR2_Y_CPX1;
    MINT32 ANR_Y_CPX2_high = rUpperANR.lut1.bits.ANR2_Y_CPX2;
    MINT32 ANR_Y_CPX3_high = rUpperANR.lut1.bits.ANR2_Y_CPX3;

    // CAM_ANR_Y4LUT2 CAM+A38H
    MINT32 ANR_Y_SCALE_CPY0_high = rUpperANR.lut2.bits.ANR2_Y_SCALE_CPY0;
    MINT32 ANR_Y_SCALE_CPY1_high = rUpperANR.lut2.bits.ANR2_Y_SCALE_CPY1;
    MINT32 ANR_Y_SCALE_CPY2_high = rUpperANR.lut2.bits.ANR2_Y_SCALE_CPY2;
    MINT32 ANR_Y_SCALE_CPY3_high = rUpperANR.lut2.bits.ANR2_Y_SCALE_CPY3;

    // CAM_ANR_Y4LUT3 CAM+A3CH
    MINT32 ANR_Y_SCALE_SP0_high = rUpperANR.lut3.bits.ANR2_Y_SCALE_SP0;
    MINT32 ANR_Y_SCALE_SP1_high = rUpperANR.lut3.bits.ANR2_Y_SCALE_SP1;
    MINT32 ANR_Y_SCALE_SP2_high = rUpperANR.lut3.bits.ANR2_Y_SCALE_SP2;
    MINT32 ANR_Y_SCALE_SP3_high = rUpperANR.lut3.bits.ANR2_Y_SCALE_SP3;

    // CAM_ANR_L4LUT1
    MINT32 ANR_LCE_X1_high = rUpperANR.llut1.bits.ANR2_LCE_X1;
    MINT32 ANR_LCE_X2_high = rUpperANR.llut1.bits.ANR2_LCE_X2;
    MINT32 ANR_LCE_X3_high = rUpperANR.llut1.bits.ANR2_LCE_X3;

    // CAM_ANR_L4LUT2
    MINT32 ANR_LCE_GAIN0_high = rUpperANR.llut2.bits.ANR2_LCE_GAIN0;
    MINT32 ANR_LCE_GAIN1_high = rUpperANR.llut2.bits.ANR2_LCE_GAIN1;
    MINT32 ANR_LCE_GAIN2_high = rUpperANR.llut2.bits.ANR2_LCE_GAIN2;
    MINT32 ANR_LCE_GAIN3_high = rUpperANR.llut2.bits.ANR2_LCE_GAIN3;

    // CAM_ANR_L4LUT3
    MINT32 ANR_LCE_SP0_high = rUpperANR.llut3.bits.ANR2_LCE_SP0;
    MINT32 ANR_LCE_SP1_high = rUpperANR.llut3.bits.ANR2_LCE_SP1;
    MINT32 ANR_LCE_SP2_high = rUpperANR.llut3.bits.ANR2_LCE_SP2;
    MINT32 ANR_LCE_SP3_high = rUpperANR.llut3.bits.ANR2_LCE_SP3;

    // CAM_ANR_CAD CAM+A44H
    MINT32 ANR_PTC_VGAIN_high = rUpperANR.cad.bits.ANR2_PTC_VGAIN;
    MINT32 ANR_C_GAIN_high = rLowerANR.cad.bits.ANR2_C_GAIN;
    MINT32 ANR_PTC_GAIN_TH_high = rUpperANR.cad.bits.ANR2_PTC_GAIN_TH;
    MINT32 ANR_C_L_DIFF_TH_high = rUpperANR.cad.bits.ANR2_C_L_DIFF_TH;

    // CAM_ANR_PTC CAM+A48H
    MINT32 ANR_PTC1_high = rUpperANR.ptc.bits.ANR2_PTC1;
    MINT32 ANR_PTC2_high = rUpperANR.ptc.bits.ANR2_PTC2;
    MINT32 ANR_PTC3_high = rUpperANR.ptc.bits.ANR2_PTC3;
    MINT32 ANR_PTC4_high = rUpperANR.ptc.bits.ANR2_PTC4;

    // CAM_ANR_LCE CAM+A4CH
    MINT32 ANR_LCE_C_GAIN_high = rUpperANR.lce.bits.ANR2_LCE_C_GAIN;

    // CAM_ANR_MED1
    MINT32 ANR_COR_TH_high = rUpperANR.med1.bits.ANR2_COR_TH;
    MINT32 ANR_COR_SL_high = rUpperANR.med1.bits.ANR2_COR_SL;
    MINT32 ANR_MCD_TH_high = rUpperANR.med1.bits.ANR2_MCD_TH;
    MINT32 ANR_MCD_SL_high = rUpperANR.med1.bits.ANR2_MCD_SL;
    MINT32 ANR_LCL_TH_high = rUpperANR.med1.bits.ANR2_LCL_TH;

    // CAM_ANR_MED2
    MINT32 ANR_LCL_SL_high = rUpperANR.med2.bits.ANR2_LCL_SL;
    MINT32 ANR_LCL_LV_high = rUpperANR.med2.bits.ANR2_LCL_LV;
    MINT32 ANR_SCL_TH_high = rUpperANR.med2.bits.ANR2_SCL_TH;
    MINT32 ANR_SCL_SL_high = rUpperANR.med2.bits.ANR2_SCL_SL;
    MINT32 ANR_SCL_LV_high = rUpperANR.med2.bits.ANR2_SCL_LV;

    // CAM_ANR_MED3
    MINT32 ANR_NCL_TH_high = rUpperANR.med3.bits.ANR2_NCL_TH;
    MINT32 ANR_NCL_SL_high = rUpperANR.med3.bits.ANR2_NCL_SL;
    MINT32 ANR_NCL_LV_high = rUpperANR.med3.bits.ANR2_NCL_LV;
    MINT32 ANR_VAR_high = rUpperANR.med3.bits.ANR2_VAR;
    MINT32 ANR_Y0_high = rUpperANR.med3.bits.ANR2_Y0;

    // CAM_ANR_MED4
    MINT32 ANR_Y1_high = rUpperANR.med4.bits.ANR2_Y1;
    MINT32 ANR_Y2_high = rUpperANR.med4.bits.ANR2_Y2;
    MINT32 ANR_Y3_high = rUpperANR.med4.bits.ANR2_Y3;
    MINT32 ANR_Y4_high = rUpperANR.med4.bits.ANR2_Y4;

    // CAM_ANR_ACTY CAM+A60H
    MINT32 ANR_ACT_TH_Y_high = rUpperANR.acty.bits.ANR2_ACT_TH_Y;
    MINT32 ANR_ACT_BLD_BASE_Y_high= rUpperANR.acty.bits.ANR2_ACT_BLD_BASE_Y;
    MINT32 ANR_ACT_SLANT_Y_high = rUpperANR.acty.bits.ANR2_ACT_SLANT_Y;
    MINT32 ANR_ACT_BLD_TH_Y_high = rUpperANR.acty.bits.ANR2_ACT_BLD_TH_Y;

    // CAM_ANR_ACTC CAM+A64H
    MINT32 ANR_ACT_TH_C_high = rUpperANR.actc.bits.ANR2_ACT_TH_C;
    MINT32 ANR_ACT_BLD_BASE_C_high= rUpperANR.actc.bits.ANR2_ACT_BLD_BASE_C;
    MINT32 ANR_ACT_SLANT_C_high = rUpperANR.actc.bits.ANR2_ACT_SLANT_C;
    MINT32 ANR_ACT_BLD_TH_C_high = rUpperANR.actc.bits.ANR2_ACT_BLD_TH_C;

    // ====================================================================================================================================
    // Interpolating reg declaration
    // ====================================================================================================================================

    // CAM_ANR_CON1 CAM+A20H
    MINT32 ANR_ENC;
    MINT32 ANR_ENY;
    MINT32 ANR_SCALE_MODE;
    MINT32 ANR_MODE;
    MINT32 ANR_LCE_LINK;

    // CAM_ANR_CON2 CAM+A24H
    MINT32 ANR_C_MED_EN;
    MINT32 ANR_C_SM_EDGE;
    MINT32 ANR_FLT_C;

    // CAM_ANR_YAD1 CAM+A2CH
    MINT32 ANR_K_TH_C;

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

    // CAM_ANR_CAD CAM+A44H
    MINT32 ANR_PTC_VGAIN;
    MINT32 ANR_C_GAIN;
    MINT32 ANR_PTC_GAIN_TH;
    MINT32 ANR_C_L_DIFF_TH;

    // CAM_ANR_PTC CAM+A48H
    MINT32 ANR_PTC1;
    MINT32 ANR_PTC2;
    MINT32 ANR_PTC3;
    MINT32 ANR_PTC4;

    // CAM_ANR_LCE CAM+A4CH
    MINT32 ANR_LCE_C_GAIN;

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

    // CAM_ANR_ACTY CAM+A60H
    MINT32 ANR_ACT_TH_Y;
    MINT32 ANR_ACT_BLD_BASE_Y;
    MINT32 ANR_ACT_SLANT_Y;
    MINT32 ANR_ACT_BLD_TH_Y;

    // CAM_ANR_ACTC CAM+A64H
    MINT32 ANR_ACT_TH_C;
    MINT32 ANR_ACT_BLD_BASE_C;
    MINT32 ANR_ACT_SLANT_C;
    MINT32 ANR_ACT_BLD_TH_C;

    // ====================================================================================================================================
    // Negative Conversion
    // ====================================================================================================================================
    ANR_Y_SCALE_SP0_low = (ANR_Y_SCALE_SP0_low > 15) ? (ANR_Y_SCALE_SP0_low - 32) : ANR_Y_SCALE_SP0_low;
    ANR_Y_SCALE_SP1_low = (ANR_Y_SCALE_SP1_low > 15) ? (ANR_Y_SCALE_SP1_low - 32) : ANR_Y_SCALE_SP1_low;
    ANR_Y_SCALE_SP2_low = (ANR_Y_SCALE_SP2_low > 15) ? (ANR_Y_SCALE_SP2_low - 32) : ANR_Y_SCALE_SP2_low;
    ANR_Y_SCALE_SP3_low = (ANR_Y_SCALE_SP3_low > 15) ? (ANR_Y_SCALE_SP3_low - 32) : ANR_Y_SCALE_SP3_low;
    ANR_LCE_SP0_low = (ANR_LCE_SP0_low > 15) ? (ANR_LCE_SP0_low - 32) : ANR_LCE_SP0_low;
    ANR_LCE_SP1_low = (ANR_LCE_SP1_low > 15) ? (ANR_LCE_SP1_low - 32) : ANR_LCE_SP1_low;
    ANR_LCE_SP2_low = (ANR_LCE_SP2_low > 15) ? (ANR_LCE_SP2_low - 32) : ANR_LCE_SP2_low;
    ANR_LCE_SP3_low = (ANR_LCE_SP3_low > 15) ? (ANR_LCE_SP3_low - 32) : ANR_LCE_SP3_low;

    ANR_Y_SCALE_SP0_high = (ANR_Y_SCALE_SP0_high > 15) ? (ANR_Y_SCALE_SP0_high - 32) : ANR_Y_SCALE_SP0_high;
    ANR_Y_SCALE_SP1_high = (ANR_Y_SCALE_SP1_high > 15) ? (ANR_Y_SCALE_SP1_high - 32) : ANR_Y_SCALE_SP1_high;
    ANR_Y_SCALE_SP2_high = (ANR_Y_SCALE_SP2_high > 15) ? (ANR_Y_SCALE_SP2_high - 32) : ANR_Y_SCALE_SP2_high;
    ANR_Y_SCALE_SP3_high = (ANR_Y_SCALE_SP3_high > 15) ? (ANR_Y_SCALE_SP3_high - 32) : ANR_Y_SCALE_SP3_high;
    ANR_LCE_SP0_high = (ANR_LCE_SP0_high > 15) ? (ANR_LCE_SP0_high - 32) : ANR_LCE_SP0_high;
    ANR_LCE_SP1_high = (ANR_LCE_SP1_high > 15) ? (ANR_LCE_SP1_high - 32) : ANR_LCE_SP1_high;
    ANR_LCE_SP2_high = (ANR_LCE_SP2_high > 15) ? (ANR_LCE_SP2_high - 32) : ANR_LCE_SP2_high;
    ANR_LCE_SP3_high = (ANR_LCE_SP3_high > 15) ? (ANR_LCE_SP3_high - 32) : ANR_LCE_SP3_high;

    // ====================================================================================================================================
    // Start Parameter Interpolation
    // ====================================================================================================================================

    MINT32 ISO = u4RealISO;
    MINT32 ISO_low = u4LowerISO;
    MINT32 ISO_high = u4UpperISO;

    if(ANR_ENC_low==ANR_ENC_high && ANR_ENY_low==ANR_ENY_high)
    {
        // Registers that can not be interpolated
        ANR_ENC = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ENC_low, ANR_ENC_high, 1);
        ANR_ENY = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ENY_low, ANR_ENY_high, 1);
        ANR_MODE = InterParam_NR(ISO, ISO_low, ISO_high, ANR_MODE_low, ANR_MODE_high, 1);
        ANR_C_MED_EN = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_MED_EN_low, ANR_C_MED_EN_high, 1);
        ANR_C_SM_EDGE = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_SM_EDGE_low, ANR_C_SM_EDGE_high, 1);
        ANR_FLT_C = InterParam_NR(ISO, ISO_low, ISO_high, ANR_FLT_C_low, ANR_FLT_C_high, 1);

        // Registers that can be interpolated
        ANR_Y_CPX1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_CPX1_low, ANR_Y_CPX1_high, 0);
        ANR_Y_CPX2 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_CPX2_low, ANR_Y_CPX2_high, 0);
        ANR_Y_CPX3 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_CPX3_low, ANR_Y_CPX3_high, 0);
        ANR_Y_SCALE_CPY0 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_SCALE_CPY0_low, ANR_Y_SCALE_CPY0_high, 0);
        ANR_Y_SCALE_CPY1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_SCALE_CPY1_low, ANR_Y_SCALE_CPY1_high, 0);
        ANR_Y_SCALE_CPY2 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_SCALE_CPY2_low, ANR_Y_SCALE_CPY2_high, 0);
        ANR_Y_SCALE_CPY3 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_SCALE_CPY3_low, ANR_Y_SCALE_CPY3_high, 0);
        ANR_PTC_VGAIN = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTC_VGAIN_low, ANR_PTC_VGAIN_high, 0);
        ANR_C_GAIN = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_GAIN_low, ANR_C_GAIN_high, 0);
        ANR_PTC_GAIN_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTC_GAIN_TH_low, ANR_PTC_GAIN_TH_high, 0);
        ANR_C_L_DIFF_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_L_DIFF_TH_low, ANR_C_L_DIFF_TH_high, 0);
        ANR_PTC1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTC1_low, ANR_PTC1_high, 0);
        ANR_PTC2 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTC2_low, ANR_PTC2_high, 0);
        ANR_PTC3 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTC3_low, ANR_PTC3_high, 0);
        ANR_PTC4 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTC4_low, ANR_PTC4_high, 0);
        ANR_ACT_TH_Y = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_TH_Y_low, ANR_ACT_TH_Y_high, 0);
        ANR_ACT_BLD_BASE_Y = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_BLD_BASE_Y_low, ANR_ACT_BLD_BASE_Y_high, 0);
        ANR_ACT_SLANT_Y = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_SLANT_Y_low, ANR_ACT_SLANT_Y_high, 0);
        ANR_ACT_BLD_TH_Y = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_BLD_TH_Y_low, ANR_ACT_BLD_TH_Y_high, 0);
        ANR_ACT_TH_C = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_TH_C_low, ANR_ACT_TH_C_high, 0);
        ANR_ACT_BLD_BASE_C = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_BLD_BASE_C_low, ANR_ACT_BLD_BASE_C_high, 0);
        ANR_ACT_SLANT_C = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_SLANT_C_low, ANR_ACT_SLANT_C_high, 0);
        ANR_ACT_BLD_TH_C = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_BLD_TH_C_low, ANR_ACT_BLD_TH_C_high, 0);

        // ANR_SCALE_MODE
        ANR_SCALE_MODE = InterParam_NR(ISO, ISO_low, ISO_high, ANR_SCALE_MODE_low, ANR_SCALE_MODE_high, 1);
        ANR_K_TH_C = InterParam_NR(ISO, ISO_low, ISO_high, ANR_K_TH_C_low, ANR_K_TH_C_high, 1);

        // ANR_LCE_LINK
        if(0 == ANR_LCE_LINK_low && 1 == ANR_LCE_LINK_high)
        {
            ANR_LCE_LINK_low = ANR_LCE_LINK_high;
            ANR_LCE_X1_low = ANR_LCE_X1_high;
            ANR_LCE_X2_low = ANR_LCE_X2_high;
            ANR_LCE_X3_low = ANR_LCE_X3_high;
            ANR_LCE_GAIN0_low = 8;
            ANR_LCE_GAIN1_low = 8;
            ANR_LCE_GAIN2_low = 8;
            ANR_LCE_GAIN3_low = 8;
            ANR_LCE_SP0_low = 0;
            ANR_LCE_SP1_low = 0;
            ANR_LCE_SP2_low = 0;
            ANR_LCE_SP3_low = 0;
            ANR_LCE_C_GAIN_low = 4;
        }
        if(0 == ANR_LCE_LINK_high && 1 == ANR_LCE_LINK_low)
        {
            ANR_LCE_LINK_high = ANR_LCE_LINK_low;
            ANR_LCE_X1_high = ANR_LCE_X1_low;
            ANR_LCE_X2_high = ANR_LCE_X2_low;
            ANR_LCE_X3_high = ANR_LCE_X3_low;
            ANR_LCE_GAIN0_high = 8;
            ANR_LCE_GAIN1_high = 8;
            ANR_LCE_GAIN2_high = 8;
            ANR_LCE_GAIN3_high = 8;
            ANR_LCE_SP0_high = 0;
            ANR_LCE_SP1_high = 0;
            ANR_LCE_SP2_high = 0;
            ANR_LCE_SP3_high = 0;
            ANR_LCE_C_GAIN_high = 4;
        }

        ANR_LCE_LINK = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_LINK_low, ANR_LCE_LINK_high, 0);
        ANR_LCE_X1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_X1_low, ANR_LCE_X1_high, 0);
        ANR_LCE_X2 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_X2_low, ANR_LCE_X2_high, 0);
        ANR_LCE_X3 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_X3_low, ANR_LCE_X3_high, 0);
        ANR_LCE_GAIN0 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_GAIN0_low, ANR_LCE_GAIN0_high, 0);
        ANR_LCE_GAIN1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_GAIN1_low, ANR_LCE_GAIN1_high, 0);
        ANR_LCE_GAIN2 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_GAIN2_low, ANR_LCE_GAIN2_high, 0);
        ANR_LCE_GAIN3 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_GAIN3_low, ANR_LCE_GAIN3_high, 0);
        ANR_LCE_C_GAIN = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_C_GAIN_low, ANR_LCE_C_GAIN_high, 0);

        // SP Calculation
        ANR_Y_SCALE_SP0 = CalculateLUTSP(         0, ANR_Y_CPX1, ANR_Y_SCALE_CPY0, ANR_Y_SCALE_CPY1, 32, 15, -15);
        ANR_Y_SCALE_SP1 = CalculateLUTSP(ANR_Y_CPX1, ANR_Y_CPX2, ANR_Y_SCALE_CPY1, ANR_Y_SCALE_CPY2, 32, 15, -15);
        ANR_Y_SCALE_SP2 = CalculateLUTSP(ANR_Y_CPX2, ANR_Y_CPX3, ANR_Y_SCALE_CPY2, ANR_Y_SCALE_CPY3, 32, 15, -15);
        ANR_Y_SCALE_SP3 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_SCALE_SP3_low, ANR_Y_SCALE_SP3_high, 0);
        ANR_LCE_SP0 = CalculateLUTSP(         0, ANR_LCE_X1, ANR_LCE_GAIN0, ANR_LCE_GAIN1, 32, 15, -15);
        ANR_LCE_SP1 = CalculateLUTSP(ANR_LCE_X1, ANR_LCE_X2, ANR_LCE_GAIN1, ANR_LCE_GAIN2, 32, 15, -15);
        ANR_LCE_SP2 = CalculateLUTSP(ANR_LCE_X2, ANR_LCE_X3, ANR_LCE_GAIN2, ANR_LCE_GAIN3, 32, 15, -15);
        ANR_LCE_SP3 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_SP3_low, ANR_LCE_SP3_high, 0);

        // ANR_MEDIAN_EN
        ANR_COR_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_COR_TH_low, ANR_COR_TH_high, 0);
        ANR_COR_SL = InterParam_NR(ISO, ISO_low, ISO_high, ANR_COR_SL_low, ANR_COR_SL_high, 0);
        ANR_MCD_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_MCD_TH_low, ANR_MCD_TH_high, 0);
        ANR_MCD_SL = InterParam_NR(ISO, ISO_low, ISO_high, ANR_MCD_SL_low, ANR_MCD_SL_high, 0);
        ANR_LCL_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCL_TH_low, ANR_LCL_TH_high, 0);
        ANR_LCL_SL = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCL_SL_low, ANR_LCL_SL_high, 0);
        ANR_LCL_LV = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCL_LV_low, ANR_LCL_LV_high, 0);
        ANR_SCL_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_SCL_TH_low, ANR_SCL_TH_high, 0);
        ANR_SCL_SL = InterParam_NR(ISO, ISO_low, ISO_high, ANR_SCL_SL_low, ANR_SCL_SL_high, 0);
        ANR_SCL_LV = InterParam_NR(ISO, ISO_low, ISO_high, ANR_SCL_LV_low, ANR_SCL_LV_high, 0);
        ANR_NCL_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_NCL_TH_low, ANR_NCL_TH_high, 0);
        ANR_NCL_SL = InterParam_NR(ISO, ISO_low, ISO_high, ANR_NCL_SL_low, ANR_NCL_SL_high, 0);
        ANR_NCL_LV = InterParam_NR(ISO, ISO_low, ISO_high, ANR_NCL_LV_low, ANR_NCL_LV_high, 0);
        ANR_VAR = InterParam_NR(ISO, ISO_low, ISO_high, ANR_VAR_low, ANR_VAR_high, 0);
        ANR_Y0 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y0_low, ANR_Y0_high, 0);
        ANR_Y1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y1_low, ANR_Y1_high, 0);
        ANR_Y2 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y2_low, ANR_Y2_high, 0);
        ANR_Y3 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y3_low, ANR_Y3_high, 0);
        ANR_Y4 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y4_low, ANR_Y4_high, 0);
    }
    else
    {
        // Registers that can not be interpolated
        ANR_ENC = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ENC_low, ANR_ENC_high, 1);
        ANR_ENY = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ENY_low, ANR_ENY_high, 1);
        ANR_MODE = InterParam_NR(ISO, ISO_low, ISO_high, ANR_MODE_low, ANR_MODE_high, 1);
        ANR_C_MED_EN = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_MED_EN_low, ANR_C_MED_EN_high, 1);
        ANR_C_SM_EDGE = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_SM_EDGE_low, ANR_C_SM_EDGE_high, 1);
        ANR_FLT_C = InterParam_NR(ISO, ISO_low, ISO_high, ANR_FLT_C_low, ANR_FLT_C_high, 1);
        ANR_Y_CPX1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_CPX1_low, ANR_Y_CPX1_high, 1);
        ANR_Y_CPX2 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_CPX2_low, ANR_Y_CPX2_high, 1);
        ANR_Y_CPX3 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_CPX3_low, ANR_Y_CPX3_high, 1);
        ANR_Y_SCALE_CPY0 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_SCALE_CPY0_low, ANR_Y_SCALE_CPY0_high, 1);
        ANR_Y_SCALE_CPY1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_SCALE_CPY1_low, ANR_Y_SCALE_CPY1_high, 1);
        ANR_Y_SCALE_CPY2 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_SCALE_CPY2_low, ANR_Y_SCALE_CPY2_high, 1);
        ANR_Y_SCALE_CPY3 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_SCALE_CPY3_low, ANR_Y_SCALE_CPY3_high, 1);
        ANR_Y_SCALE_SP0 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_SCALE_SP0_low, ANR_Y_SCALE_SP0_high, 1);
        ANR_Y_SCALE_SP1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_SCALE_SP1_low, ANR_Y_SCALE_SP1_high, 1);
        ANR_Y_SCALE_SP2 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_SCALE_SP2_low, ANR_Y_SCALE_SP2_high, 1);
        ANR_Y_SCALE_SP3 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y_SCALE_SP3_low, ANR_Y_SCALE_SP3_high, 1);
        ANR_PTC_VGAIN = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTC_VGAIN_low, ANR_PTC_VGAIN_high, 1);
        ANR_C_GAIN = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_GAIN_low, ANR_C_GAIN_high, 1);
        ANR_PTC_GAIN_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTC_GAIN_TH_low, ANR_PTC_GAIN_TH_high, 1);
        ANR_C_L_DIFF_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_C_L_DIFF_TH_low, ANR_C_L_DIFF_TH_high, 1);
        ANR_PTC1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTC1_low, ANR_PTC1_high, 1);
        ANR_PTC2 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTC2_low, ANR_PTC2_high, 1);
        ANR_PTC3 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTC3_low, ANR_PTC3_high, 1);
        ANR_PTC4 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_PTC4_low, ANR_PTC4_high, 1);
        ANR_ACT_TH_Y = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_TH_Y_low, ANR_ACT_TH_Y_high, 1);
        ANR_ACT_BLD_BASE_Y = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_BLD_BASE_Y_low, ANR_ACT_BLD_BASE_Y_high, 1);
        ANR_ACT_SLANT_Y = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_SLANT_Y_low, ANR_ACT_SLANT_Y_high, 1);
        ANR_ACT_BLD_TH_Y = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_BLD_TH_Y_low, ANR_ACT_BLD_TH_Y_high, 1);
        ANR_ACT_TH_C = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_TH_C_low, ANR_ACT_TH_C_high, 1);
        ANR_ACT_BLD_BASE_C = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_BLD_BASE_C_low, ANR_ACT_BLD_BASE_C_high, 1);
        ANR_ACT_SLANT_C = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_SLANT_C_low, ANR_ACT_SLANT_C_high, 1);
        ANR_ACT_BLD_TH_C = InterParam_NR(ISO, ISO_low, ISO_high, ANR_ACT_BLD_TH_C_low, ANR_ACT_BLD_TH_C_high, 1);

        // ANR_SCALE_MODE
        ANR_SCALE_MODE = InterParam_NR(ISO, ISO_low, ISO_high, ANR_SCALE_MODE_low, ANR_SCALE_MODE_high, 1);
        ANR_K_TH_C = InterParam_NR(ISO, ISO_low, ISO_high, ANR_K_TH_C_low, ANR_K_TH_C_high, 1);

        // ANR_LCE_LINK
        ANR_LCE_LINK = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_LINK_low, ANR_LCE_LINK_high, 1);
        ANR_LCE_X1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_X1_low, ANR_LCE_X1_high, 1);
        ANR_LCE_X2 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_X2_low, ANR_LCE_X2_high, 1);
        ANR_LCE_X3 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_X3_low, ANR_LCE_X3_high, 1);
        ANR_LCE_GAIN0 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_GAIN0_low, ANR_LCE_GAIN0_high, 1);
        ANR_LCE_GAIN1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_GAIN1_low, ANR_LCE_GAIN1_high, 1);
        ANR_LCE_GAIN2 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_GAIN2_low, ANR_LCE_GAIN2_high, 1);
        ANR_LCE_GAIN3 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_GAIN3_low, ANR_LCE_GAIN3_high, 1);
        ANR_LCE_SP0 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_SP0_low, ANR_LCE_SP0_high, 1);
        ANR_LCE_SP1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_SP1_low, ANR_LCE_SP1_high, 1);
        ANR_LCE_SP2 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_SP2_low, ANR_LCE_SP2_high, 1);
        ANR_LCE_SP3 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_SP3_low, ANR_LCE_SP3_high, 1);
        ANR_LCE_C_GAIN = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCE_C_GAIN_low, ANR_LCE_C_GAIN_high, 1);

        // ANR_MEDIAN_EN
        ANR_COR_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_COR_TH_low, ANR_COR_TH_high, 1);
        ANR_COR_SL = InterParam_NR(ISO, ISO_low, ISO_high, ANR_COR_SL_low, ANR_COR_SL_high, 1);
        ANR_MCD_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_MCD_TH_low, ANR_MCD_TH_high, 1);
        ANR_MCD_SL = InterParam_NR(ISO, ISO_low, ISO_high, ANR_MCD_SL_low, ANR_MCD_SL_high, 1);
        ANR_LCL_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCL_TH_low, ANR_LCL_TH_high, 1);
        ANR_LCL_SL = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCL_SL_low, ANR_LCL_SL_high, 1);
        ANR_LCL_LV = InterParam_NR(ISO, ISO_low, ISO_high, ANR_LCL_LV_low, ANR_LCL_LV_high, 1);
        ANR_SCL_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_SCL_TH_low, ANR_SCL_TH_high, 1);
        ANR_SCL_SL = InterParam_NR(ISO, ISO_low, ISO_high, ANR_SCL_SL_low, ANR_SCL_SL_high, 1);
        ANR_SCL_LV = InterParam_NR(ISO, ISO_low, ISO_high, ANR_SCL_LV_low, ANR_SCL_LV_high, 1);
        ANR_NCL_TH = InterParam_NR(ISO, ISO_low, ISO_high, ANR_NCL_TH_low, ANR_NCL_TH_high, 1);
        ANR_NCL_SL = InterParam_NR(ISO, ISO_low, ISO_high, ANR_NCL_SL_low, ANR_NCL_SL_high, 1);
        ANR_NCL_LV = InterParam_NR(ISO, ISO_low, ISO_high, ANR_NCL_LV_low, ANR_NCL_LV_high, 1);
        ANR_VAR = InterParam_NR(ISO, ISO_low, ISO_high, ANR_VAR_low, ANR_VAR_high, 1);
        ANR_Y0 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y0_low, ANR_Y0_high, 1);
        ANR_Y1 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y1_low, ANR_Y1_high, 1);
        ANR_Y2 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y2_low, ANR_Y2_high, 1);
        ANR_Y3 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y3_low, ANR_Y3_high, 1);
        ANR_Y4 = InterParam_NR(ISO, ISO_low, ISO_high, ANR_Y4_low, ANR_Y4_high, 1);
    }

    // ====================================================================================================================================
    // Negative Conversion
    // ====================================================================================================================================
    ANR_Y_SCALE_SP0 = (ANR_Y_SCALE_SP0 < 0) ? (ANR_Y_SCALE_SP0 + 32) : ANR_Y_SCALE_SP0;
    ANR_Y_SCALE_SP1 = (ANR_Y_SCALE_SP1 < 0) ? (ANR_Y_SCALE_SP1 + 32) : ANR_Y_SCALE_SP1;
    ANR_Y_SCALE_SP2 = (ANR_Y_SCALE_SP2 < 0) ? (ANR_Y_SCALE_SP2 + 32) : ANR_Y_SCALE_SP2;
    ANR_Y_SCALE_SP3 = (ANR_Y_SCALE_SP3 < 0) ? (ANR_Y_SCALE_SP3 + 32) : ANR_Y_SCALE_SP3;
    ANR_LCE_SP0 = (ANR_LCE_SP0 < 0) ? (ANR_LCE_SP0 + 32) : ANR_LCE_SP0;
    ANR_LCE_SP1 = (ANR_LCE_SP1 < 0) ? (ANR_LCE_SP1 + 32) : ANR_LCE_SP1;
    ANR_LCE_SP2 = (ANR_LCE_SP2 < 0) ? (ANR_LCE_SP2 + 32) : ANR_LCE_SP2;
    ANR_LCE_SP3 = (ANR_LCE_SP3 < 0) ? (ANR_LCE_SP3 + 32) : ANR_LCE_SP3;

    // ====================================================================================================================================
    // Set Smooth ISO setting
    // ====================================================================================================================================
    // CAM_ANR_CON1 CAM+A20H
    rSmoothANR.con1.bits.ANR2_ENC = ANR_ENC;
    rSmoothANR.con1.bits.ANR2_ENY = ANR_ENY;
    rSmoothANR.con1.bits.ANR2_SCALE_MODE = ANR_SCALE_MODE;
    rSmoothANR.con1.bits.ANR2_MODE = ANR_MODE;
    rSmoothANR.con1.bits.ANR2_LCE_LINK = ANR_LCE_LINK;

    // CAM_ANR_CON2 CAM+A24H
    rSmoothANR.con2.bits.ANR2_C_MED_EN = ANR_C_MED_EN;
    rSmoothANR.con2.bits.ANR2_C_SM_EDGE = ANR_C_SM_EDGE;
    rSmoothANR.con2.bits.ANR2_FLT_C = ANR_FLT_C;

    // CAM_ANR_YAD1 CAM+A2CH
    rSmoothANR.yad1.bits.ANR2_K_TH_C = ANR_K_TH_C;

    // CAM_ANR_Y4LUT1 CAM+A34H
    rSmoothANR.lut1.bits.ANR2_Y_CPX1 = ANR_Y_CPX1;
    rSmoothANR.lut1.bits.ANR2_Y_CPX2 = ANR_Y_CPX2;
    rSmoothANR.lut1.bits.ANR2_Y_CPX3 = ANR_Y_CPX3;

    // CAM_ANR_Y4LUT2 CAM+A38H
    rSmoothANR.lut2.bits.ANR2_Y_SCALE_CPY0 = ANR_Y_SCALE_CPY0;
    rSmoothANR.lut2.bits.ANR2_Y_SCALE_CPY1 = ANR_Y_SCALE_CPY1;
    rSmoothANR.lut2.bits.ANR2_Y_SCALE_CPY2 = ANR_Y_SCALE_CPY2;
    rSmoothANR.lut2.bits.ANR2_Y_SCALE_CPY3 = ANR_Y_SCALE_CPY3;

    // CAM_ANR_Y4LUT3 CAM+A3CH
    rSmoothANR.lut3.bits.ANR2_Y_SCALE_SP0 = ANR_Y_SCALE_SP0;
    rSmoothANR.lut3.bits.ANR2_Y_SCALE_SP1 = ANR_Y_SCALE_SP1;
    rSmoothANR.lut3.bits.ANR2_Y_SCALE_SP2 = ANR_Y_SCALE_SP2;
    rSmoothANR.lut3.bits.ANR2_Y_SCALE_SP3 = ANR_Y_SCALE_SP3;

    // CAM_ANR_L4LUT1
    rSmoothANR.llut1.bits.ANR2_LCE_X1 = ANR_LCE_X1;
    rSmoothANR.llut1.bits.ANR2_LCE_X2 = ANR_LCE_X2;
    rSmoothANR.llut1.bits.ANR2_LCE_X3 = ANR_LCE_X3;

    // CAM_ANR_L4LUT2
    rSmoothANR.llut2.bits.ANR2_LCE_GAIN0 = ANR_LCE_GAIN0;
    rSmoothANR.llut2.bits.ANR2_LCE_GAIN1 = ANR_LCE_GAIN1;
    rSmoothANR.llut2.bits.ANR2_LCE_GAIN2 = ANR_LCE_GAIN2;
    rSmoothANR.llut2.bits.ANR2_LCE_GAIN3 = ANR_LCE_GAIN3;

    // CAM_ANR_L4LUT3
    rSmoothANR.llut3.bits.ANR2_LCE_SP0 = ANR_LCE_SP0;
    rSmoothANR.llut3.bits.ANR2_LCE_SP1 = ANR_LCE_SP1;
    rSmoothANR.llut3.bits.ANR2_LCE_SP2 = ANR_LCE_SP2;
    rSmoothANR.llut3.bits.ANR2_LCE_SP3 = ANR_LCE_SP3;

    // CAM_ANR_CAD CAM+A44H
    rSmoothANR.cad.bits.ANR2_PTC_VGAIN = ANR_PTC_VGAIN;
    rSmoothANR.cad.bits.ANR2_C_GAIN = ANR_C_GAIN;
    rSmoothANR.cad.bits.ANR2_PTC_GAIN_TH = ANR_PTC_GAIN_TH;
    rSmoothANR.cad.bits.ANR2_C_L_DIFF_TH = ANR_C_L_DIFF_TH;

    // CAM_ANR_PTC CAM+A48H
    rSmoothANR.ptc.bits.ANR2_PTC1 = ANR_PTC1;
    rSmoothANR.ptc.bits.ANR2_PTC2 = ANR_PTC2;
    rSmoothANR.ptc.bits.ANR2_PTC3 = ANR_PTC3;
    rSmoothANR.ptc.bits.ANR2_PTC4 = ANR_PTC4;

    // CAM_ANR_LCE CAM+A4CH
    rSmoothANR.lce.bits.ANR2_LCE_C_GAIN = ANR_LCE_C_GAIN;

    // CAM_ANR_MED1
    rSmoothANR.med1.bits.ANR2_COR_TH = ANR_COR_TH;
    rSmoothANR.med1.bits.ANR2_COR_SL = ANR_COR_SL;
    rSmoothANR.med1.bits.ANR2_MCD_TH = ANR_MCD_TH;
    rSmoothANR.med1.bits.ANR2_MCD_SL = ANR_MCD_SL;
    rSmoothANR.med1.bits.ANR2_LCL_TH = ANR_LCL_TH;

    // CAM_ANR_MED2
    rSmoothANR.med2.bits.ANR2_LCL_SL = ANR_LCL_SL;
    rSmoothANR.med2.bits.ANR2_LCL_LV = ANR_LCL_LV;
    rSmoothANR.med2.bits.ANR2_SCL_TH = ANR_SCL_TH;
    rSmoothANR.med2.bits.ANR2_SCL_SL = ANR_SCL_SL;
    rSmoothANR.med2.bits.ANR2_SCL_LV = ANR_SCL_LV;

    // CAM_ANR_MED3
    rSmoothANR.med3.bits.ANR2_NCL_TH = ANR_NCL_TH;
    rSmoothANR.med3.bits.ANR2_NCL_SL = ANR_NCL_SL;
    rSmoothANR.med3.bits.ANR2_NCL_LV = ANR_NCL_LV;
    rSmoothANR.med3.bits.ANR2_VAR = ANR_VAR;
    rSmoothANR.med3.bits.ANR2_Y0 = ANR_Y0;

    // CAM_ANR_MED4
    rSmoothANR.med4.bits.ANR2_Y1 = ANR_Y1;
    rSmoothANR.med4.bits.ANR2_Y2 = ANR_Y2;
    rSmoothANR.med4.bits.ANR2_Y3 = ANR_Y3;
    rSmoothANR.med4.bits.ANR2_Y4 = ANR_Y4;

    // CAM_ANR_ACTY CAM+A60H
    rSmoothANR.acty.bits.ANR2_ACT_TH_Y = ANR_ACT_TH_Y;
    rSmoothANR.acty.bits.ANR2_ACT_BLD_BASE_Y = ANR_ACT_BLD_BASE_Y;
    rSmoothANR.acty.bits.ANR2_ACT_SLANT_Y = ANR_ACT_SLANT_Y;
    rSmoothANR.acty.bits.ANR2_ACT_BLD_TH_Y = ANR_ACT_BLD_TH_Y;

    // CAM_ANR_ACTC CAM+A64H
    rSmoothANR.actc.bits.ANR2_ACT_TH_C = ANR_ACT_TH_C;
    rSmoothANR.actc.bits.ANR2_ACT_BLD_BASE_C = ANR_ACT_BLD_BASE_C;
    rSmoothANR.actc.bits.ANR2_ACT_SLANT_C = ANR_ACT_SLANT_C;
    rSmoothANR.actc.bits.ANR2_ACT_BLD_TH_C = ANR_ACT_BLD_TH_C;

#define ANR_DUMP(item1, item2)   \
        do{                      \
            MY_LOG_IF(bSmoothANRDebug, "[L,S,U]%22s = %d, %d, %d", #item2, rLowerANR.item1.bits.item2, rSmoothANR.item1.bits.item2, rUpperANR.item1.bits.item2); \
        }while(0)

    ANR_DUMP(con1, ANR2_ENC);
    ANR_DUMP(con1, ANR2_ENY);
    ANR_DUMP(con1, ANR2_SCALE_MODE);
    ANR_DUMP(con1, ANR2_MODE);
    ANR_DUMP(con1, ANR2_LCE_LINK);

    ANR_DUMP(con2, ANR2_C_MED_EN);
    ANR_DUMP(con2, ANR2_C_SM_EDGE);
    ANR_DUMP(con2, ANR2_FLT_C);

    ANR_DUMP(yad1, ANR2_K_TH_C);

    ANR_DUMP(lut1, ANR2_Y_CPX1);
    ANR_DUMP(lut1, ANR2_Y_CPX2);
    ANR_DUMP(lut1, ANR2_Y_CPX3);

    ANR_DUMP(lut2, ANR2_Y_SCALE_CPY0);
    ANR_DUMP(lut2, ANR2_Y_SCALE_CPY1);
    ANR_DUMP(lut2, ANR2_Y_SCALE_CPY2);
    ANR_DUMP(lut2, ANR2_Y_SCALE_CPY3);

    ANR_DUMP(lut3, ANR2_Y_SCALE_SP0);
    ANR_DUMP(lut3, ANR2_Y_SCALE_SP1);
    ANR_DUMP(lut3, ANR2_Y_SCALE_SP2);
    ANR_DUMP(lut3, ANR2_Y_SCALE_SP3);

    ANR_DUMP(llut1, ANR2_LCE_X1);
    ANR_DUMP(llut1, ANR2_LCE_X2);
    ANR_DUMP(llut1, ANR2_LCE_X3);

    ANR_DUMP(llut2, ANR2_LCE_GAIN0);
    ANR_DUMP(llut2, ANR2_LCE_GAIN1);
    ANR_DUMP(llut2, ANR2_LCE_GAIN2);
    ANR_DUMP(llut2, ANR2_LCE_GAIN3);

    ANR_DUMP(llut3, ANR2_LCE_SP0);
    ANR_DUMP(llut3, ANR2_LCE_SP1);
    ANR_DUMP(llut3, ANR2_LCE_SP2);
    ANR_DUMP(llut3, ANR2_LCE_SP3);

    ANR_DUMP(cad, ANR2_PTC_VGAIN);
    ANR_DUMP(cad, ANR2_C_GAIN);
    ANR_DUMP(cad, ANR2_PTC_GAIN_TH);
    ANR_DUMP(cad, ANR2_C_L_DIFF_TH);

    ANR_DUMP(ptc, ANR2_PTC1);
    ANR_DUMP(ptc, ANR2_PTC2);
    ANR_DUMP(ptc, ANR2_PTC3);
    ANR_DUMP(ptc, ANR2_PTC4);

    ANR_DUMP(lce, ANR2_LCE_C_GAIN);

    ANR_DUMP(med1, ANR2_COR_TH);
    ANR_DUMP(med1, ANR2_COR_SL);
    ANR_DUMP(med1, ANR2_MCD_TH);
    ANR_DUMP(med1, ANR2_MCD_SL);
    ANR_DUMP(med1, ANR2_LCL_TH);

    ANR_DUMP(med2, ANR2_LCL_SL);
    ANR_DUMP(med2, ANR2_LCL_LV);
    ANR_DUMP(med2, ANR2_SCL_TH);
    ANR_DUMP(med2, ANR2_SCL_SL);
    ANR_DUMP(med2, ANR2_SCL_LV);

    ANR_DUMP(med3, ANR2_NCL_TH);
    ANR_DUMP(med3, ANR2_NCL_SL);
    ANR_DUMP(med3, ANR2_NCL_LV);
    ANR_DUMP(med3, ANR2_VAR);
    ANR_DUMP(med3, ANR2_Y0);

    ANR_DUMP(med4, ANR2_Y1);
    ANR_DUMP(med4, ANR2_Y2);
    ANR_DUMP(med4, ANR2_Y3);
    ANR_DUMP(med4, ANR2_Y4);

    ANR_DUMP(acty, ANR2_ACT_TH_Y);
    ANR_DUMP(acty, ANR2_ACT_BLD_BASE_Y);
    ANR_DUMP(acty, ANR2_ACT_SLANT_Y);
    ANR_DUMP(acty, ANR2_ACT_BLD_TH_Y);

    ANR_DUMP(actc, ANR2_ACT_TH_C);
    ANR_DUMP(actc, ANR2_ACT_BLD_BASE_C);
    ANR_DUMP(actc, ANR2_ACT_SLANT_C);
    ANR_DUMP(actc, ANR2_ACT_BLD_TH_C);

#undef ANR_DUMP
}

MINT32 InterParam_PDC(MINT32 ISO, MINT32 ISOpre, MINT32 ISOpos, MINT32 PARAMpre, MINT32 PARAMpos, MINT32 Method) {
    MINT32 InterPARAM = 0;

    if(ISOpre == ISOpos) {
        // To prevent someone sets different PARAMpre and PARAMpos
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

MVOID SmoothPDC(MUINT32 u4RealISO, // Real ISO
                    MUINT32 u4UpperISO, // Upper ISO
                    MUINT32 u4LowerISO, // Lower ISO
                    ISP_NVRAM_BNR_PDC_T const& rUpperBNR_PDC, // NR1 settings for upper ISO
                    ISP_NVRAM_BNR_PDC_T const& rLowerBNR_PDC, // NR1 settings for lower ISO
                    ISP_NVRAM_BNR_PDC_T& rSmoothBNR_PDC)   // Output
{

	MBOOL bSmoothBNR_PDCDebug = interpPropertyGet("vendor.debug.smooth_bnr_pdc.enable", MFALSE);
	
    MY_LOG_IF(bSmoothBNR_PDCDebug,"[%s()] u4RealISO: %d, u4UpperISO = %d, u4LowerISO = %d\n", __FUNCTION__, u4RealISO, u4UpperISO, u4LowerISO);

    MINT32 PDC_ENpre = rLowerBNR_PDC.con.bits.PDC_EN;
    MINT32 PDC_CTpre = rLowerBNR_PDC.con.bits.PDC_CT;
    MINT32 PDC_GCF_L00pre = rLowerBNR_PDC.gain_l0.bits.PDC_GCF_L00;
    MINT32 PDC_GCF_L10pre = rLowerBNR_PDC.gain_l0.bits.PDC_GCF_L10;
    MINT32 PDC_GCF_L01pre = rLowerBNR_PDC.gain_l1.bits.PDC_GCF_L01;
    MINT32 PDC_GCF_L20pre = rLowerBNR_PDC.gain_l1.bits.PDC_GCF_L20;
    MINT32 PDC_GCF_L11pre = rLowerBNR_PDC.gain_l2.bits.PDC_GCF_L11;
    MINT32 PDC_GCF_L02pre = rLowerBNR_PDC.gain_l2.bits.PDC_GCF_L02;
    MINT32 PDC_GCF_L30pre = rLowerBNR_PDC.gain_l3.bits.PDC_GCF_L30;
    MINT32 PDC_GCF_L21pre = rLowerBNR_PDC.gain_l3.bits.PDC_GCF_L21;
    MINT32 PDC_GCF_L12pre = rLowerBNR_PDC.gain_l4.bits.PDC_GCF_L12;
    MINT32 PDC_GCF_L03pre = rLowerBNR_PDC.gain_l4.bits.PDC_GCF_L03;
    MINT32 PDC_GCF_R00pre = rLowerBNR_PDC.gain_r0.bits.PDC_GCF_R00;
    MINT32 PDC_GCF_R10pre = rLowerBNR_PDC.gain_r0.bits.PDC_GCF_R10;
    MINT32 PDC_GCF_R01pre = rLowerBNR_PDC.gain_r1.bits.PDC_GCF_R01;
    MINT32 PDC_GCF_R20pre = rLowerBNR_PDC.gain_r1.bits.PDC_GCF_R20;
    MINT32 PDC_GCF_R11pre = rLowerBNR_PDC.gain_r2.bits.PDC_GCF_R11;
    MINT32 PDC_GCF_R02pre = rLowerBNR_PDC.gain_r2.bits.PDC_GCF_R02;
    MINT32 PDC_GCF_R30pre = rLowerBNR_PDC.gain_r3.bits.PDC_GCF_R30;
    MINT32 PDC_GCF_R21pre = rLowerBNR_PDC.gain_r3.bits.PDC_GCF_R21;
    MINT32 PDC_GCF_R12pre = rLowerBNR_PDC.gain_r4.bits.PDC_GCF_R12;
    MINT32 PDC_GCF_R03pre = rLowerBNR_PDC.gain_r4.bits.PDC_GCF_R03;
    MINT32 PDC_GTHpre = rLowerBNR_PDC.th_gb.bits.PDC_GTH;
    MINT32 PDC_BTHpre = rLowerBNR_PDC.th_gb.bits.PDC_BTH;
    MINT32 PDC_ITHpre = rLowerBNR_PDC.th_ia.bits.PDC_ITH;
    MINT32 PDC_ATHpre = rLowerBNR_PDC.th_ia.bits.PDC_ATH;
    MINT32 PDC_NTHpre = rLowerBNR_PDC.th_hd.bits.PDC_NTH;
    MINT32 PDC_DTHpre = rLowerBNR_PDC.th_hd.bits.PDC_DTH;
    MINT32 PDC_GSLpre = rLowerBNR_PDC.sl.bits.PDC_GSL;
    MINT32 PDC_BSLpre = rLowerBNR_PDC.sl.bits.PDC_BSL;
    MINT32 PDC_ISLpre = rLowerBNR_PDC.sl.bits.PDC_ISL;
    MINT32 PDC_ASLpre = rLowerBNR_PDC.sl.bits.PDC_ASL;
    MINT32 PDC_GCF_NORMpre = rLowerBNR_PDC.sl.bits.PDC_GCF_NORM;
    MINT32 PDC_XCENTERpre = rLowerBNR_PDC.pos.bits.PDC_XCENTER;
    MINT32 PDC_YCENTERpre = rLowerBNR_PDC.pos.bits.PDC_YCENTER;

    MINT32 PDC_ENpos = rUpperBNR_PDC.con.bits.PDC_EN;
    MINT32 PDC_CTpos = rUpperBNR_PDC.con.bits.PDC_CT;
    MINT32 PDC_GCF_L00pos = rUpperBNR_PDC.gain_l0.bits.PDC_GCF_L00;
    MINT32 PDC_GCF_L10pos = rUpperBNR_PDC.gain_l0.bits.PDC_GCF_L10;
    MINT32 PDC_GCF_L01pos = rUpperBNR_PDC.gain_l1.bits.PDC_GCF_L01;
    MINT32 PDC_GCF_L20pos = rUpperBNR_PDC.gain_l1.bits.PDC_GCF_L20;
    MINT32 PDC_GCF_L11pos = rUpperBNR_PDC.gain_l2.bits.PDC_GCF_L11;
    MINT32 PDC_GCF_L02pos = rUpperBNR_PDC.gain_l2.bits.PDC_GCF_L02;
    MINT32 PDC_GCF_L30pos = rUpperBNR_PDC.gain_l3.bits.PDC_GCF_L30;
    MINT32 PDC_GCF_L21pos = rUpperBNR_PDC.gain_l3.bits.PDC_GCF_L21;
    MINT32 PDC_GCF_L12pos = rUpperBNR_PDC.gain_l4.bits.PDC_GCF_L12;
    MINT32 PDC_GCF_L03pos = rUpperBNR_PDC.gain_l4.bits.PDC_GCF_L03;
    MINT32 PDC_GCF_R00pos = rUpperBNR_PDC.gain_r0.bits.PDC_GCF_R00;
    MINT32 PDC_GCF_R10pos = rUpperBNR_PDC.gain_r0.bits.PDC_GCF_R10;
    MINT32 PDC_GCF_R01pos = rUpperBNR_PDC.gain_r1.bits.PDC_GCF_R01;
    MINT32 PDC_GCF_R20pos = rUpperBNR_PDC.gain_r1.bits.PDC_GCF_R20;
    MINT32 PDC_GCF_R11pos = rUpperBNR_PDC.gain_r2.bits.PDC_GCF_R11;
    MINT32 PDC_GCF_R02pos = rUpperBNR_PDC.gain_r2.bits.PDC_GCF_R02;
    MINT32 PDC_GCF_R30pos = rUpperBNR_PDC.gain_r3.bits.PDC_GCF_R30;
    MINT32 PDC_GCF_R21pos = rUpperBNR_PDC.gain_r3.bits.PDC_GCF_R21;
    MINT32 PDC_GCF_R12pos = rUpperBNR_PDC.gain_r4.bits.PDC_GCF_R12;
    MINT32 PDC_GCF_R03pos = rUpperBNR_PDC.gain_r4.bits.PDC_GCF_R03;
    MINT32 PDC_GTHpos = rUpperBNR_PDC.th_gb.bits.PDC_GTH;
    MINT32 PDC_BTHpos = rUpperBNR_PDC.th_gb.bits.PDC_BTH;
    MINT32 PDC_ITHpos = rUpperBNR_PDC.th_ia.bits.PDC_ITH;
    MINT32 PDC_ATHpos = rUpperBNR_PDC.th_ia.bits.PDC_ATH;
    MINT32 PDC_NTHpos = rUpperBNR_PDC.th_hd.bits.PDC_NTH;
    MINT32 PDC_DTHpos = rUpperBNR_PDC.th_hd.bits.PDC_DTH;
    MINT32 PDC_GSLpos = rUpperBNR_PDC.sl.bits.PDC_GSL;
    MINT32 PDC_BSLpos = rUpperBNR_PDC.sl.bits.PDC_BSL;
    MINT32 PDC_ISLpos = rUpperBNR_PDC.sl.bits.PDC_ISL;
    MINT32 PDC_ASLpos = rUpperBNR_PDC.sl.bits.PDC_ASL;
    MINT32 PDC_GCF_NORMpos = rUpperBNR_PDC.sl.bits.PDC_GCF_NORM;
    MINT32 PDC_XCENTERpos = rUpperBNR_PDC.pos.bits.PDC_XCENTER;
    MINT32 PDC_YCENTERpos = rUpperBNR_PDC.pos.bits.PDC_YCENTER;

    // interpolation //
    MINT32 ISO = u4RealISO;
    MINT32 ISOpre = u4LowerISO;
    MINT32 ISOpos = u4UpperISO;

    MINT32 PDC_EN = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_ENpre, PDC_ENpos, 1);
    MINT32 PDC_CT = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_CTpre, PDC_CTpos, 1);
    MINT32 PDC_GCF_L00 = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GCF_L00pre, PDC_GCF_L00pos, 0);
    MINT32 PDC_GCF_L10 = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GCF_L10pre, PDC_GCF_L10pos, 0);
    MINT32 PDC_GCF_L01 = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GCF_L01pre, PDC_GCF_L01pos, 0);
    MINT32 PDC_GCF_L20 = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GCF_L20pre, PDC_GCF_L20pos, 0);
    MINT32 PDC_GCF_L11 = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GCF_L11pre, PDC_GCF_L11pos, 0);
    MINT32 PDC_GCF_L02 = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GCF_L02pre, PDC_GCF_L02pos, 0);
    MINT32 PDC_GCF_L30 = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GCF_L30pre, PDC_GCF_L30pos, 0);
    MINT32 PDC_GCF_L21 = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GCF_L21pre, PDC_GCF_L21pos, 0);
    MINT32 PDC_GCF_L12 = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GCF_L12pre, PDC_GCF_L12pos, 0);
    MINT32 PDC_GCF_L03 = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GCF_L03pre, PDC_GCF_L03pos, 0);
    MINT32 PDC_GCF_R00 = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GCF_R00pre, PDC_GCF_R00pos, 0);
    MINT32 PDC_GCF_R10 = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GCF_R10pre, PDC_GCF_R10pos, 0);
    MINT32 PDC_GCF_R01 = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GCF_R01pre, PDC_GCF_R01pos, 0);
    MINT32 PDC_GCF_R20 = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GCF_R20pre, PDC_GCF_R20pos, 0);
    MINT32 PDC_GCF_R11 = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GCF_R11pre, PDC_GCF_R11pos, 0);
    MINT32 PDC_GCF_R02 = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GCF_R02pre, PDC_GCF_R02pos, 0);
    MINT32 PDC_GCF_R30 = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GCF_R30pre, PDC_GCF_R30pos, 0);
    MINT32 PDC_GCF_R21 = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GCF_R21pre, PDC_GCF_R21pos, 0);
    MINT32 PDC_GCF_R12 = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GCF_R12pre, PDC_GCF_R12pos, 0);
    MINT32 PDC_GCF_R03 = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GCF_R03pre, PDC_GCF_R03pos, 0);
    MINT32 PDC_GTH = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GTHpre, PDC_GTHpos, 0);
    MINT32 PDC_BTH = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_BTHpre, PDC_BTHpos, 0);
    MINT32 PDC_ITH = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_ITHpre, PDC_ITHpos, 0);
    MINT32 PDC_ATH = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_ATHpre, PDC_ATHpos, 0);
    MINT32 PDC_NTH = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_NTHpre, PDC_NTHpos, 0);
    MINT32 PDC_DTH = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_DTHpre, PDC_DTHpos, 0);
    MINT32 PDC_GSL = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GSLpre, PDC_GSLpos, 0);
    MINT32 PDC_BSL = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_BSLpre, PDC_BSLpos, 0);
    MINT32 PDC_ISL = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_ISLpre, PDC_ISLpos, 0);
    MINT32 PDC_ASL = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_ASLpre, PDC_ASLpos, 0);
    MINT32 PDC_GCF_NORM = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_GCF_NORMpre, PDC_GCF_NORMpos, 0);
    MINT32 PDC_XCENTER = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_XCENTERpre, PDC_XCENTERpos, 0);
    MINT32 PDC_YCENTER = InterParam_PDC(ISO, ISOpre, ISOpos, PDC_YCENTERpre, PDC_YCENTERpos, 0);

    rSmoothBNR_PDC.con.bits.PDC_EN = PDC_EN;
    rSmoothBNR_PDC.con.bits.PDC_CT = PDC_CT;
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
    rSmoothBNR_PDC.th_gb.bits.PDC_GTH = PDC_GTH;
    rSmoothBNR_PDC.th_gb.bits.PDC_BTH = PDC_BTH;
    rSmoothBNR_PDC.th_ia.bits.PDC_ITH = PDC_ITH;
    rSmoothBNR_PDC.th_ia.bits.PDC_ATH = PDC_ATH;
    rSmoothBNR_PDC.th_hd.bits.PDC_NTH = PDC_NTH;
    rSmoothBNR_PDC.th_hd.bits.PDC_DTH = PDC_DTH;
    rSmoothBNR_PDC.sl.bits.PDC_GSL = PDC_GSL;
    rSmoothBNR_PDC.sl.bits.PDC_BSL = PDC_BSL;
    rSmoothBNR_PDC.sl.bits.PDC_ISL = PDC_ISL;
    rSmoothBNR_PDC.sl.bits.PDC_ASL = PDC_ASL;
    rSmoothBNR_PDC.sl.bits.PDC_GCF_NORM = PDC_GCF_NORM;
    rSmoothBNR_PDC.pos.bits.PDC_XCENTER = PDC_XCENTER;
    rSmoothBNR_PDC.pos.bits.PDC_YCENTER = PDC_YCENTER;


    MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_EN = %d, %d, %d", rLowerBNR_PDC.con.bits.PDC_EN, rSmoothBNR_PDC.con.bits.PDC_EN, rUpperBNR_PDC.con.bits.PDC_EN);
    MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_CT = %d, %d, %d", rLowerBNR_PDC.con.bits.PDC_CT, rSmoothBNR_PDC.con.bits.PDC_CT, rUpperBNR_PDC.con.bits.PDC_CT);
    MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_L00 = %d, %d, %d", rLowerBNR_PDC.gain_l0.bits.PDC_GCF_L00, rSmoothBNR_PDC.gain_l0.bits.PDC_GCF_L00, rUpperBNR_PDC.gain_l0.bits.PDC_GCF_L00);
    MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_L10 = %d, %d, %d", rLowerBNR_PDC.gain_l0.bits.PDC_GCF_L10, rSmoothBNR_PDC.gain_l0.bits.PDC_GCF_L10, rUpperBNR_PDC.gain_l0.bits.PDC_GCF_L10);
    MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_L01 = %d, %d, %d", rLowerBNR_PDC.gain_l1.bits.PDC_GCF_L01, rSmoothBNR_PDC.gain_l1.bits.PDC_GCF_L01, rUpperBNR_PDC.gain_l1.bits.PDC_GCF_L01);
    MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_L20 = %d, %d, %d", rLowerBNR_PDC.gain_l1.bits.PDC_GCF_L20, rSmoothBNR_PDC.gain_l1.bits.PDC_GCF_L20, rUpperBNR_PDC.gain_l1.bits.PDC_GCF_L20);
    MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_L11 = %d, %d, %d", rLowerBNR_PDC.gain_l2.bits.PDC_GCF_L11, rSmoothBNR_PDC.gain_l2.bits.PDC_GCF_L11, rUpperBNR_PDC.gain_l2.bits.PDC_GCF_L11);
    MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_L02 = %d, %d, %d", rLowerBNR_PDC.gain_l2.bits.PDC_GCF_L02, rSmoothBNR_PDC.gain_l2.bits.PDC_GCF_L02, rUpperBNR_PDC.gain_l2.bits.PDC_GCF_L02);
    MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_L30 = %d, %d, %d", rLowerBNR_PDC.gain_l3.bits.PDC_GCF_L30, rSmoothBNR_PDC.gain_l3.bits.PDC_GCF_L30, rUpperBNR_PDC.gain_l3.bits.PDC_GCF_L30);
    MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_L21 = %d, %d, %d", rLowerBNR_PDC.gain_l3.bits.PDC_GCF_L21, rSmoothBNR_PDC.gain_l3.bits.PDC_GCF_L21, rUpperBNR_PDC.gain_l3.bits.PDC_GCF_L21);
    MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_L12 = %d, %d, %d", rLowerBNR_PDC.gain_l4.bits.PDC_GCF_L12, rSmoothBNR_PDC.gain_l4.bits.PDC_GCF_L12, rUpperBNR_PDC.gain_l4.bits.PDC_GCF_L12);
    MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_L03 = %d, %d, %d", rLowerBNR_PDC.gain_l4.bits.PDC_GCF_L03, rSmoothBNR_PDC.gain_l4.bits.PDC_GCF_L03, rUpperBNR_PDC.gain_l4.bits.PDC_GCF_L03);
    MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_R00 = %d, %d, %d", rLowerBNR_PDC.gain_r0.bits.PDC_GCF_R00, rSmoothBNR_PDC.gain_r0.bits.PDC_GCF_R00, rUpperBNR_PDC.gain_r0.bits.PDC_GCF_R00);
    MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_R10 = %d, %d, %d", rLowerBNR_PDC.gain_r0.bits.PDC_GCF_R10, rSmoothBNR_PDC.gain_r0.bits.PDC_GCF_R10, rUpperBNR_PDC.gain_r0.bits.PDC_GCF_R10);
    MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_R01 = %d, %d, %d", rLowerBNR_PDC.gain_r1.bits.PDC_GCF_R01, rSmoothBNR_PDC.gain_r1.bits.PDC_GCF_R01, rUpperBNR_PDC.gain_r1.bits.PDC_GCF_R01);
    MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_R20 = %d, %d, %d", rLowerBNR_PDC.gain_r1.bits.PDC_GCF_R20, rSmoothBNR_PDC.gain_r1.bits.PDC_GCF_R20, rUpperBNR_PDC.gain_r1.bits.PDC_GCF_R20);
    MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_R11 = %d, %d, %d", rLowerBNR_PDC.gain_r2.bits.PDC_GCF_R11, rSmoothBNR_PDC.gain_r2.bits.PDC_GCF_R11, rUpperBNR_PDC.gain_r2.bits.PDC_GCF_R11);
    MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_R02 = %d, %d, %d", rLowerBNR_PDC.gain_r2.bits.PDC_GCF_R02, rSmoothBNR_PDC.gain_r2.bits.PDC_GCF_R02, rUpperBNR_PDC.gain_r2.bits.PDC_GCF_R02);
    MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_R30 = %d, %d, %d", rLowerBNR_PDC.gain_r3.bits.PDC_GCF_R30, rSmoothBNR_PDC.gain_r3.bits.PDC_GCF_R30, rUpperBNR_PDC.gain_r3.bits.PDC_GCF_R30);
    MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_R21 = %d, %d, %d", rLowerBNR_PDC.gain_r3.bits.PDC_GCF_R21, rSmoothBNR_PDC.gain_r3.bits.PDC_GCF_R21, rUpperBNR_PDC.gain_r3.bits.PDC_GCF_R21);
    MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_R12 = %d, %d, %d", rLowerBNR_PDC.gain_r4.bits.PDC_GCF_R12, rSmoothBNR_PDC.gain_r4.bits.PDC_GCF_R12, rUpperBNR_PDC.gain_r4.bits.PDC_GCF_R12);
    MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_R03 = %d, %d, %d", rLowerBNR_PDC.gain_r4.bits.PDC_GCF_R03, rSmoothBNR_PDC.gain_r4.bits.PDC_GCF_R03, rUpperBNR_PDC.gain_r4.bits.PDC_GCF_R03);
    MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GTH = %d, %d, %d", rLowerBNR_PDC.th_gb.bits.PDC_GTH, rSmoothBNR_PDC.th_gb.bits.PDC_GTH, rUpperBNR_PDC.th_gb.bits.PDC_GTH);
    MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_ITH = %d, %d, %d", rLowerBNR_PDC.th_ia.bits.PDC_ITH, rSmoothBNR_PDC.th_ia.bits.PDC_ITH, rUpperBNR_PDC.th_ia.bits.PDC_ITH);
    MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_ATH = %d, %d, %d", rLowerBNR_PDC.th_ia.bits.PDC_ATH, rSmoothBNR_PDC.th_ia.bits.PDC_ATH, rUpperBNR_PDC.th_ia.bits.PDC_ATH);
    MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_NTH = %d, %d, %d", rLowerBNR_PDC.th_hd.bits.PDC_NTH, rSmoothBNR_PDC.th_hd.bits.PDC_NTH, rUpperBNR_PDC.th_hd.bits.PDC_NTH);
    MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_DTH = %d, %d, %d", rLowerBNR_PDC.th_hd.bits.PDC_DTH, rSmoothBNR_PDC.th_hd.bits.PDC_DTH, rUpperBNR_PDC.th_hd.bits.PDC_DTH);
    MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GSL = %d, %d, %d", rLowerBNR_PDC.sl.bits.PDC_GSL, rSmoothBNR_PDC.sl.bits.PDC_GSL, rUpperBNR_PDC.sl.bits.PDC_GSL);
    MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_BSL = %d, %d, %d", rLowerBNR_PDC.sl.bits.PDC_BSL, rSmoothBNR_PDC.sl.bits.PDC_BSL, rUpperBNR_PDC.sl.bits.PDC_BSL);
    MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_ISL = %d, %d, %d", rLowerBNR_PDC.sl.bits.PDC_ISL, rSmoothBNR_PDC.sl.bits.PDC_ISL, rUpperBNR_PDC.sl.bits.PDC_ISL);
    MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_ASL = %d, %d, %d", rLowerBNR_PDC.sl.bits.PDC_ASL, rSmoothBNR_PDC.sl.bits.PDC_ASL, rUpperBNR_PDC.sl.bits.PDC_ASL);
    MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_GCF_NORM = %d, %d, %d", rLowerBNR_PDC.sl.bits.PDC_GCF_NORM, rSmoothBNR_PDC.sl.bits.PDC_GCF_NORM, rUpperBNR_PDC.sl.bits.PDC_GCF_NORM);
    MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_XCENTER = %d, %d, %d", rLowerBNR_PDC.pos.bits.PDC_XCENTER, rSmoothBNR_PDC.pos.bits.PDC_XCENTER, rUpperBNR_PDC.pos.bits.PDC_XCENTER);
    MY_LOG_IF(bSmoothBNR_PDCDebug, "[L,S,U]PDC_YCENTER = %d, %d, %d", rLowerBNR_PDC.pos.bits.PDC_YCENTER, rSmoothBNR_PDC.pos.bits.PDC_YCENTER, rUpperBNR_PDC.pos.bits.PDC_YCENTER);
}

// "return value":    Interpolated register value
// "ISO":            ISO value of current image
// "ISOpre":        A pre-defined ISO (100, 200, 400, ...) which is the closest but smaller than or equal to "ISO"
// "ISOpos":        A pre-defined ISO (100, 200, 400, ...) which is the closest but larger than or equal to "ISO"
// "PARAMpre":        Corresponding register value from "ISOpre"
// "PARAMpos":        Corresponding register value from "ISOpos"
// "Method":        0 for linear interpolation, 1 for closest one
MINT32 InterParam_CCR(MINT32 ISO, MINT32 ISOpre, MINT32 ISOpos, MINT32 PARAMpre, MINT32 PARAMpos, MINT32 Method){
    MINT32 InterPARAM = 0;

    if(ISOpre == ISOpos){
        // To prevent someone sets different PARAMpre and PARAMpos
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
        double temp = RATIOpre * PARAMpre + RATIOpos * PARAMpos;
        if(temp>0)
            InterPARAM = (MINT32)(RATIOpre * PARAMpre + RATIOpos * PARAMpos + 0.5);
        else
            InterPARAM = (MINT32)(RATIOpre * PARAMpre + RATIOpos * PARAMpos - 0.5);
        break;
    }

    return InterPARAM;
}


MVOID SmoothCCR(MUINT32 u4RealISO, // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
               ISP_NVRAM_CCR_T const& rUpperCCR, // EE settings for upper ISO
               ISP_NVRAM_CCR_T const& rLowerCCR, // EE settings for lower ISO
               ISP_NVRAM_CCR_T& rSmoothCCR)   // Output
{
	MBOOL bSmoothCCRDebug = interpPropertyGet("vendor.debug.smooth_ccr.enable", MFALSE);

    MY_LOG_IF(bSmoothCCRDebug,"[%s()] u4RealISO: %d, u4UpperISO = %d, u4LowerISO = %d\n", __FUNCTION__, u4RealISO, u4UpperISO, u4LowerISO);

    // ====================================================================================================================================
    // Get Lower ISO setting
    // ====================================================================================================================================
  MINT32 CCR_EN_lower = rLowerCCR.con.bits.CCR_EN;
  MINT32 CCR_OR_MODE_lower = rLowerCCR.con.bits.CCR_OR_MODE;
  MINT32 CCR_UV_GAIN_MODE_lower = rLowerCCR.con.bits.CCR_UV_GAIN_MODE;
  MINT32 CCR_UV_GAIN2_lower = rLowerCCR.con.bits.CCR_UV_GAIN2;
  MINT32 CCR_Y_CPX3_lower = rLowerCCR.con.bits.CCR_Y_CPX3;
  MINT32 CCR_Y_CPX1_lower = rLowerCCR.ylut.bits.CCR_Y_CPX1;
  MINT32 CCR_Y_CPX2_lower = rLowerCCR.ylut.bits.CCR_Y_CPX2;
  MINT32 CCR_Y_SP1_lower = rLowerCCR.ylut.bits.CCR_Y_SP1;
  MINT32 CCR_Y_CPY1_lower = rLowerCCR.ylut.bits.CCR_Y_CPY1;
  MINT32 CCR_UV_X1_lower = rLowerCCR.uvlut.bits.CCR_UV_X1;
  MINT32 CCR_UV_X2_lower = rLowerCCR.uvlut.bits.CCR_UV_X2;
  MINT32 CCR_UV_X3_lower = rLowerCCR.uvlut.bits.CCR_UV_X3;
  MINT32 CCR_UV_GAIN1_lower = rLowerCCR.uvlut.bits.CCR_UV_GAIN1;
  MINT32 CCR_Y_SP0_lower = rLowerCCR.ylut2.bits.CCR_Y_SP0;
  MINT32 CCR_Y_SP2_lower = rLowerCCR.ylut2.bits.CCR_Y_SP2;
  MINT32 CCR_Y_CPY0_lower = rLowerCCR.ylut2.bits.CCR_Y_CPY0;
  MINT32 CCR_Y_CPY2_lower = rLowerCCR.ylut2.bits.CCR_Y_CPY2;
  MINT32 CCR_MODE_lower = rLowerCCR.sat_ctrl.bits.CCR_MODE;
  MINT32 CCR_CEN_U_lower = rLowerCCR.sat_ctrl.bits.CCR_CEN_U;
  MINT32 CCR_CEN_V_lower = rLowerCCR.sat_ctrl.bits.CCR_CEN_V;
  MINT32 CCR_UV_GAIN_SP1_lower = rLowerCCR.uvlut_sp.bits.CCR_UV_GAIN_SP1;
  MINT32 CCR_UV_GAIN_SP2_lower = rLowerCCR.uvlut_sp.bits.CCR_UV_GAIN_SP2;
  MINT32 CCR_HUE_X1_lower = rLowerCCR.hue1.bits.CCR_HUE_X1;
  MINT32 CCR_HUE_X2_lower = rLowerCCR.hue1.bits.CCR_HUE_X2;
  MINT32 CCR_HUE_X3_lower = rLowerCCR.hue2.bits.CCR_HUE_X3;
  MINT32 CCR_HUE_X4_lower = rLowerCCR.hue2.bits.CCR_HUE_X4;
  MINT32 CCR_HUE_SP1_lower = rLowerCCR.hue3.bits.CCR_HUE_SP1;
  MINT32 CCR_HUE_SP2_lower = rLowerCCR.hue3.bits.CCR_HUE_SP2;
  MINT32 CCR_HUE_GAIN1_lower = rLowerCCR.hue3.bits.CCR_HUE_GAIN1;
  MINT32 CCR_HUE_GAIN2_lower = rLowerCCR.hue3.bits.CCR_HUE_GAIN2;

    // ====================================================================================================================================
    // Get Upper ISO setting
    // ====================================================================================================================================
  MINT32 CCR_EN_upper = rUpperCCR.con.bits.CCR_EN;
  MINT32 CCR_OR_MODE_upper = rUpperCCR.con.bits.CCR_OR_MODE;
  MINT32 CCR_UV_GAIN_MODE_upper = rUpperCCR.con.bits.CCR_UV_GAIN_MODE;
  MINT32 CCR_UV_GAIN2_upper = rUpperCCR.con.bits.CCR_UV_GAIN2;
  MINT32 CCR_Y_CPX3_upper = rUpperCCR.con.bits.CCR_Y_CPX3;
  MINT32 CCR_Y_CPX1_upper = rUpperCCR.ylut.bits.CCR_Y_CPX1;
  MINT32 CCR_Y_CPX2_upper = rUpperCCR.ylut.bits.CCR_Y_CPX2;
  MINT32 CCR_Y_SP1_upper = rUpperCCR.ylut.bits.CCR_Y_SP1;
  MINT32 CCR_Y_CPY1_upper = rUpperCCR.ylut.bits.CCR_Y_CPY1;
  MINT32 CCR_UV_X1_upper = rUpperCCR.uvlut.bits.CCR_UV_X1;
  MINT32 CCR_UV_X2_upper = rUpperCCR.uvlut.bits.CCR_UV_X2;
  MINT32 CCR_UV_X3_upper = rUpperCCR.uvlut.bits.CCR_UV_X3;
  MINT32 CCR_UV_GAIN1_upper = rUpperCCR.uvlut.bits.CCR_UV_GAIN1;
  MINT32 CCR_Y_SP0_upper = rUpperCCR.ylut2.bits.CCR_Y_SP0;
  MINT32 CCR_Y_SP2_upper = rUpperCCR.ylut2.bits.CCR_Y_SP2;
  MINT32 CCR_Y_CPY0_upper = rUpperCCR.ylut2.bits.CCR_Y_CPY0;
  MINT32 CCR_Y_CPY2_upper = rUpperCCR.ylut2.bits.CCR_Y_CPY2;
  MINT32 CCR_MODE_upper = rUpperCCR.sat_ctrl.bits.CCR_MODE;
  MINT32 CCR_CEN_U_upper = rUpperCCR.sat_ctrl.bits.CCR_CEN_U;
  MINT32 CCR_CEN_V_upper = rUpperCCR.sat_ctrl.bits.CCR_CEN_V;
  MINT32 CCR_UV_GAIN_SP1_upper = rUpperCCR.uvlut_sp.bits.CCR_UV_GAIN_SP1;
  MINT32 CCR_UV_GAIN_SP2_upper = rUpperCCR.uvlut_sp.bits.CCR_UV_GAIN_SP2;
  MINT32 CCR_HUE_X1_upper = rUpperCCR.hue1.bits.CCR_HUE_X1;
  MINT32 CCR_HUE_X2_upper = rUpperCCR.hue1.bits.CCR_HUE_X2;
  MINT32 CCR_HUE_X3_upper = rUpperCCR.hue2.bits.CCR_HUE_X3;
  MINT32 CCR_HUE_X4_upper = rUpperCCR.hue2.bits.CCR_HUE_X4;
  MINT32 CCR_HUE_SP1_upper = rUpperCCR.hue3.bits.CCR_HUE_SP1;
  MINT32 CCR_HUE_SP2_upper = rUpperCCR.hue3.bits.CCR_HUE_SP2;
  MINT32 CCR_HUE_GAIN1_upper = rUpperCCR.hue3.bits.CCR_HUE_GAIN1;
  MINT32 CCR_HUE_GAIN2_upper = rUpperCCR.hue3.bits.CCR_HUE_GAIN2;

  // Negative value for slope
  CCR_CEN_U_lower = (CCR_CEN_U_lower > 31) ? (CCR_CEN_U_lower - 64) : CCR_CEN_U_lower;
  CCR_CEN_V_lower = (CCR_CEN_V_lower > 31) ? (CCR_CEN_V_lower - 64) : CCR_CEN_V_lower;
  CCR_HUE_SP1_lower = (CCR_HUE_SP1_lower > 127) ? (CCR_HUE_SP1_lower - 256) : CCR_HUE_SP1_lower;
  CCR_HUE_SP2_lower = (CCR_HUE_SP2_lower > 127) ? (CCR_HUE_SP2_lower - 256) : CCR_HUE_SP2_lower;

  CCR_CEN_U_upper = (CCR_CEN_U_upper > 31) ? (CCR_CEN_U_upper - 64) : CCR_CEN_U_upper;
  CCR_CEN_V_upper = (CCR_CEN_V_upper > 31) ? (CCR_CEN_V_upper - 64) : CCR_CEN_V_upper;
  CCR_HUE_SP1_upper = (CCR_HUE_SP1_upper > 127) ? (CCR_HUE_SP1_upper - 256) : CCR_HUE_SP1_upper;
  CCR_HUE_SP2_upper = (CCR_HUE_SP2_upper > 127) ? (CCR_HUE_SP2_upper - 256) : CCR_HUE_SP2_upper;

  // Start Parameter Interpolation
    // ====================================================================================================================================
  MINT32 ISO = u4RealISO;
  MINT32 ISOpre = u4LowerISO;
  MINT32 ISOpos = u4UpperISO;

  // Registers that can not be interpolated
    MINT32 CCR_EN = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_EN_lower, CCR_EN_upper, 1);
    MINT32 CCR_OR_MODE = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_OR_MODE_lower, CCR_OR_MODE_upper, 1);
    MINT32 CCR_UV_GAIN_MODE = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_UV_GAIN_MODE_lower, CCR_UV_GAIN_MODE_upper, 1);
    MINT32 CCR_MODE = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_MODE_lower, CCR_MODE_upper, 1);

    // Registers that can be interpolated
    MINT32 CCR_UV_GAIN2 = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_UV_GAIN2_lower, CCR_UV_GAIN2_upper, 0);
    MINT32 CCR_Y_CPX3 = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_Y_CPX3_lower, CCR_Y_CPX3_upper, 0);
    MINT32 CCR_Y_CPX1 = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_Y_CPX1_lower, CCR_Y_CPX1_upper, 0);
    MINT32 CCR_Y_CPX2 = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_Y_CPX2_lower, CCR_Y_CPX2_upper, 0);
    MINT32 CCR_Y_CPY1 = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_Y_CPY1_lower, CCR_Y_CPY1_upper, 0);
    MINT32 CCR_UV_X1 = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_UV_X1_lower, CCR_UV_X1_upper, 0);
    MINT32 CCR_UV_X2 = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_UV_X2_lower, CCR_UV_X2_upper, 0);
    MINT32 CCR_UV_X3 = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_UV_X3_lower, CCR_UV_X3_upper, 0);
    MINT32 CCR_UV_GAIN1 = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_UV_GAIN1_lower, CCR_UV_GAIN1_upper, 0);
    MINT32 CCR_Y_CPY0 = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_Y_CPY0_lower, CCR_Y_CPY0_upper, 0);
    MINT32 CCR_Y_CPY2 = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_Y_CPY2_lower, CCR_Y_CPY2_upper, 0);
    MINT32 CCR_CEN_U = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_CEN_U_lower, CCR_CEN_U_upper, 0);
    MINT32 CCR_CEN_V = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_CEN_V_lower, CCR_CEN_V_upper, 0);
    MINT32 CCR_HUE_X1 = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_HUE_X1_lower, CCR_HUE_X1_upper, 0);
    MINT32 CCR_HUE_X2 = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_HUE_X2_lower, CCR_HUE_X2_upper, 0);
    MINT32 CCR_HUE_X3 = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_HUE_X3_lower, CCR_HUE_X3_upper, 0);
    MINT32 CCR_HUE_X4 = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_HUE_X4_lower, CCR_HUE_X4_upper, 0);
    MINT32 CCR_HUE_GAIN1 = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_HUE_GAIN1_lower, CCR_HUE_GAIN1_upper, 0);
    MINT32 CCR_HUE_GAIN2 = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_HUE_GAIN2_lower, CCR_HUE_GAIN2_upper, 0);
    MINT32 CCR_Y_SP2 = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_Y_SP2_lower, CCR_Y_SP2_upper, 0);
    MINT32 CCR_UV_GAIN_SP2 = InterParam_CCR(ISO, ISOpre, ISOpos, CCR_UV_GAIN_SP2_lower, CCR_UV_GAIN_SP2_upper, 0);
    // Slope parameters update

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

    // ====================================================================================================================================
    // Set Smooth ISO setting
    // ====================================================================================================================================
    rSmoothCCR.con.bits.CCR_EN = CCR_EN;
    rSmoothCCR.con.bits.CCR_OR_MODE = CCR_OR_MODE;
    rSmoothCCR.con.bits.CCR_UV_GAIN_MODE = CCR_UV_GAIN_MODE;
    rSmoothCCR.con.bits.CCR_UV_GAIN2 = CCR_UV_GAIN2;
    rSmoothCCR.con.bits.CCR_Y_CPX3 = CCR_Y_CPX3;
    rSmoothCCR.ylut.bits.CCR_Y_CPX1 = CCR_Y_CPX1;
    rSmoothCCR.ylut.bits.CCR_Y_CPX2 = CCR_Y_CPX2;
    rSmoothCCR.ylut.bits.CCR_Y_SP1 = CCR_Y_SP1;
    rSmoothCCR.ylut.bits.CCR_Y_CPY1 = CCR_Y_CPY1;
    rSmoothCCR.uvlut.bits.CCR_UV_X1 = CCR_UV_X1;
    rSmoothCCR.uvlut.bits.CCR_UV_X2 = CCR_UV_X2;
    rSmoothCCR.uvlut.bits.CCR_UV_X3 = CCR_UV_X3;
    rSmoothCCR.uvlut.bits.CCR_UV_GAIN1 = CCR_UV_GAIN1;
    rSmoothCCR.ylut2.bits.CCR_Y_SP0 = CCR_Y_SP0;
    rSmoothCCR.ylut2.bits.CCR_Y_SP2 = CCR_Y_SP2;
    rSmoothCCR.ylut2.bits.CCR_Y_CPY0 = CCR_Y_CPY0;
    rSmoothCCR.ylut2.bits.CCR_Y_CPY2 = CCR_Y_CPY2;
    rSmoothCCR.sat_ctrl.bits.CCR_MODE = CCR_MODE;
    rSmoothCCR.sat_ctrl.bits.CCR_CEN_U = CCR_CEN_U;
    rSmoothCCR.sat_ctrl.bits.CCR_CEN_V = CCR_CEN_V;
    rSmoothCCR.uvlut_sp.bits.CCR_UV_GAIN_SP1 = CCR_UV_GAIN_SP1;
    rSmoothCCR.uvlut_sp.bits.CCR_UV_GAIN_SP2 = CCR_UV_GAIN_SP2;
    rSmoothCCR.hue1.bits.CCR_HUE_X1 = CCR_HUE_X1;
    rSmoothCCR.hue1.bits.CCR_HUE_X2 = CCR_HUE_X2;
    rSmoothCCR.hue2.bits.CCR_HUE_X3 = CCR_HUE_X3;
    rSmoothCCR.hue2.bits.CCR_HUE_X4 = CCR_HUE_X4;
    rSmoothCCR.hue3.bits.CCR_HUE_GAIN1 = CCR_HUE_GAIN1;
    rSmoothCCR.hue3.bits.CCR_HUE_GAIN2 = CCR_HUE_GAIN2;


    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_EN = %d, %d, %d", rLowerCCR.con.bits.CCR_EN, rSmoothCCR.con.bits.CCR_EN, rUpperCCR.con.bits.CCR_EN   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_OR_MODE = %d, %d, %d", rLowerCCR.con.bits.CCR_OR_MODE, rSmoothCCR.con.bits.CCR_OR_MODE, rUpperCCR.con.bits.CCR_OR_MODE   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_UV_GAIN_MODE = %d, %d, %d", rLowerCCR.con.bits.CCR_UV_GAIN_MODE, rSmoothCCR.con.bits.CCR_UV_GAIN_MODE, rUpperCCR.con.bits.CCR_UV_GAIN_MODE   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_UV_GAIN2 = %d, %d, %d", rLowerCCR.con.bits.CCR_UV_GAIN2, rSmoothCCR.con.bits.CCR_UV_GAIN2, rUpperCCR.con.bits.CCR_UV_GAIN2   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_Y_CPX3 = %d, %d, %d", rLowerCCR.con.bits.CCR_Y_CPX3, rSmoothCCR.con.bits.CCR_Y_CPX3, rUpperCCR.con.bits.CCR_Y_CPX3   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_Y_CPX1 = %d, %d, %d", rLowerCCR.ylut.bits.CCR_Y_CPX1, rSmoothCCR.ylut.bits.CCR_Y_CPX1, rUpperCCR.ylut.bits.CCR_Y_CPX1   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_Y_CPX2 = %d, %d, %d", rLowerCCR.ylut.bits.CCR_Y_CPX2, rSmoothCCR.ylut.bits.CCR_Y_CPX2, rUpperCCR.ylut.bits.CCR_Y_CPX2   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_Y_SP1 = %d, %d, %d", rLowerCCR.ylut.bits.CCR_Y_SP1, rSmoothCCR.ylut.bits.CCR_Y_SP1, rUpperCCR.ylut.bits.CCR_Y_SP1   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_Y_CPY1 = %d, %d, %d", rLowerCCR.ylut.bits.CCR_Y_CPY1, rSmoothCCR.ylut.bits.CCR_Y_CPY1, rUpperCCR.ylut.bits.CCR_Y_CPY1   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_UV_X1 = %d, %d, %d", rLowerCCR.uvlut.bits.CCR_UV_X1, rSmoothCCR.uvlut.bits.CCR_UV_X1, rUpperCCR.uvlut.bits.CCR_UV_X1   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_UV_X2 = %d, %d, %d", rLowerCCR.uvlut.bits.CCR_UV_X2, rSmoothCCR.uvlut.bits.CCR_UV_X2, rUpperCCR.uvlut.bits.CCR_UV_X2   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_UV_X3 = %d, %d, %d", rLowerCCR.uvlut.bits.CCR_UV_X3, rSmoothCCR.uvlut.bits.CCR_UV_X3, rUpperCCR.uvlut.bits.CCR_UV_X3   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_UV_GAIN1 = %d, %d, %d", rLowerCCR.uvlut.bits.CCR_UV_GAIN1, rSmoothCCR.uvlut.bits.CCR_UV_GAIN1, rUpperCCR.uvlut.bits.CCR_UV_GAIN1   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_Y_SP0 = %d, %d, %d", rLowerCCR.ylut2.bits.CCR_Y_SP0, rSmoothCCR.ylut2.bits.CCR_Y_SP0, rUpperCCR.ylut2.bits.CCR_Y_SP0   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_Y_SP2 = %d, %d, %d", rLowerCCR.ylut2.bits.CCR_Y_SP2, rSmoothCCR.ylut2.bits.CCR_Y_SP2, rUpperCCR.ylut2.bits.CCR_Y_SP2   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_Y_CPY0 = %d, %d, %d", rLowerCCR.ylut2.bits.CCR_Y_CPY0, rSmoothCCR.ylut2.bits.CCR_Y_CPY0, rUpperCCR.ylut2.bits.CCR_Y_CPY0   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_Y_CPY2 = %d, %d, %d", rLowerCCR.ylut2.bits.CCR_Y_CPY2, rSmoothCCR.ylut2.bits.CCR_Y_CPY2, rUpperCCR.ylut2.bits.CCR_Y_CPY2   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_MODE = %d, %d, %d", rLowerCCR.sat_ctrl.bits.CCR_MODE, rSmoothCCR.sat_ctrl.bits.CCR_MODE, rUpperCCR.sat_ctrl.bits.CCR_MODE   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_CEN_U = %d, %d, %d", rLowerCCR.sat_ctrl.bits.CCR_CEN_U, rSmoothCCR.sat_ctrl.bits.CCR_CEN_U, rUpperCCR.sat_ctrl.bits.CCR_CEN_U   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_CEN_V = %d, %d, %d", rLowerCCR.sat_ctrl.bits.CCR_CEN_V, rSmoothCCR.sat_ctrl.bits.CCR_CEN_V, rUpperCCR.sat_ctrl.bits.CCR_CEN_V   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_UV_GAIN_SP1 = %d, %d, %d", rLowerCCR.uvlut_sp.bits.CCR_UV_GAIN_SP1, rSmoothCCR.uvlut_sp.bits.CCR_UV_GAIN_SP1, rUpperCCR.uvlut_sp.bits.CCR_UV_GAIN_SP1   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_UV_GAIN_SP2 = %d, %d, %d", rLowerCCR.uvlut_sp.bits.CCR_UV_GAIN_SP2, rSmoothCCR.uvlut_sp.bits.CCR_UV_GAIN_SP2, rUpperCCR.uvlut_sp.bits.CCR_UV_GAIN_SP2   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_HUE_X1 = %d, %d, %d", rLowerCCR.hue1.bits.CCR_HUE_X1, rSmoothCCR.hue1.bits.CCR_HUE_X1, rUpperCCR.hue1.bits.CCR_HUE_X1   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_HUE_X2 = %d, %d, %d", rLowerCCR.hue1.bits.CCR_HUE_X2, rSmoothCCR.hue1.bits.CCR_HUE_X2, rUpperCCR.hue1.bits.CCR_HUE_X2   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_HUE_X3 = %d, %d, %d", rLowerCCR.hue2.bits.CCR_HUE_X3, rSmoothCCR.hue2.bits.CCR_HUE_X3, rUpperCCR.hue2.bits.CCR_HUE_X3   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_HUE_X4 = %d, %d, %d", rLowerCCR.hue2.bits.CCR_HUE_X4, rSmoothCCR.hue2.bits.CCR_HUE_X4, rUpperCCR.hue2.bits.CCR_HUE_X4   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_HUE_GAIN1 = %d, %d, %d", rLowerCCR.hue3.bits.CCR_HUE_GAIN1, rSmoothCCR.hue3.bits.CCR_HUE_GAIN1, rUpperCCR.hue3.bits.CCR_HUE_GAIN1   );
    MY_LOG_IF(bSmoothCCRDebug,"[L,S,U]CCR.CCR_HUE_GAIN2 = %d, %d, %d", rLowerCCR.hue3.bits.CCR_HUE_GAIN2, rSmoothCCR.hue3.bits.CCR_HUE_GAIN2, rUpperCCR.hue3.bits.CCR_HUE_GAIN2   );
}

MINT32 InterParam_PCA(MINT32 u4RealCT, MINT32 u4UpperCT, MINT32 u4LowerCT, MINT32 PARAMupper, MINT32 PARAMlower){
                MINT32 InterPARAM = 0;

                if(u4UpperCT == u4LowerCT){
                   // To prevent someone sets different PARAMpre and PARAMpos
                    return (MINT32)((PARAMupper + PARAMlower + 1) / 2);
                }
        double RATIOpre = (double)(u4UpperCT - u4RealCT) / (double)(u4UpperCT - u4LowerCT);
        double RATIOpos = 1.0 - (double)(RATIOpre);
        double temp = RATIOpre * PARAMlower + RATIOpos * PARAMupper;
        if(temp>0)
          InterPARAM = (MINT32)(RATIOpre * PARAMlower + RATIOpos * PARAMupper + 0.5);
        else
          InterPARAM = (MINT32)(RATIOpre * PARAMlower + RATIOpos * PARAMupper - 0.5);

                return InterPARAM;
}

MVOID SmoothPCA_TBL(MUINT32 u4RealCT, // Real CT
               MUINT32 u4UpperCT, // Upper CT
               MUINT32 u4LowerCT, // Lower CT
               ISP_NVRAM_PCA_LUT_T const& rUpperPCA_TBL,
               ISP_NVRAM_PCA_LUT_T const& rLowerPCA_TBL,
               ISP_NVRAM_PCA_LUT_T& rSmoothPCA_TBL,
               MUINT32 u4RealLV,
               ISP_CUSTOM_PARA_PCA_T* Para_PCA)
{

        //return upper PCA
        if(u4RealCT == 0 || u4UpperCT == 0 || u4LowerCT == 0)
        {
           for(int i = 0; i < PCA_BIN_NUM; ++i) {

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



        if (u4RealLV < Para_PCA->u4PCA_LV_H) //else lv_ratio = 1.0
        {
          if (u4RealLV > Para_PCA->u4PCA_LV_L) //else R = Para_PCA.PCA_R_MIN
          {
            lv_ratio = ( double(u4RealLV -Para_PCA->u4PCA_LV_L )/ (Para_PCA->u4PCA_LV_H - Para_PCA->u4PCA_LV_L) * (Rmax - Para_PCA->uPCA_R_MIN) + Para_PCA->uPCA_R_MIN);
            lv_ratio /= Rmax;
          }
          else
            lv_ratio = Para_PCA->uPCA_R_MIN/Rmax;
        }


        //CT interpolation
         for(int i = 0; i < PCA_BIN_NUM; ++i) {

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

          //LV tuning //There is no LV tuning for PCA_TBL_LUMA, only CT interpolation
          if(lv_ratio != 1.0) {

            PCA_TBL_SAT = PCA_TBL_SAT * lv_ratio;
            PCA_TBL_HUE = PCA_TBL_HUE * lv_ratio;

            double sat_flat_strength = Para_PCA->uPCA_SAT_FLAT_STR * (1.0-lv_ratio);
            double hue_flat_strength = Para_PCA->uPCA_HUE_FLAT_STR * (1.0-lv_ratio);

            //smaller lv_ratio is, stronger flat effect
            PCA_TBL_SAT = ((16 - sat_flat_strength )* PCA_TBL_SAT + sat_flat_strength * Para_PCA->uPCA_SAT_FLAT )/16;
            PCA_TBL_HUE = ((16 - hue_flat_strength )* PCA_TBL_HUE + hue_flat_strength * Para_PCA->uPCA_HUE_FLAT)/16;
          }


                    PCA_TBL_LUMA = (PCA_TBL_LUMA < 0 )? PCA_TBL_LUMA+256:PCA_TBL_LUMA;
                    PCA_TBL_SAT = (PCA_TBL_SAT < 0 )? PCA_TBL_SAT +256:PCA_TBL_SAT;
                    PCA_TBL_HUE = (PCA_TBL_HUE < 0 )? PCA_TBL_HUE +256:PCA_TBL_HUE;
          rSmoothPCA_TBL.lut[i].bits.PCA_LUMA_GAIN = PCA_TBL_LUMA;
          rSmoothPCA_TBL.lut[i].bits.PCA_SAT_GAIN = PCA_TBL_SAT;
          rSmoothPCA_TBL.lut[i].bits.PCA_HUE_SHIFT = PCA_TBL_HUE;


         }
}


MVOID SmoothSWNR(MUINT32 u4RealISO, // Real ISO
                MUINT32 u4UpperISO, // Upper ISO
                MUINT32 u4LowerISO, // Lower ISO
                NVRAM_CAMERA_FEATURE_SWNR_STRUCT const& rUpperSWNR, // SWNR settings for upper ISO
                NVRAM_CAMERA_FEATURE_SWNR_STRUCT const& rLowerSWNR, // SWNR settings for lower ISO
                NVRAM_CAMERA_FEATURE_SWNR_STRUCT& rSmoothSWNR) // Output
{
	MBOOL bSmoothSWNRDebug = interpPropertyGet("vendor.debug.smooth_swnr.enable", MFALSE);

    MY_LOG_IF(bSmoothSWNRDebug,"[%s()] u4RealISO: %d, u4UpperISO = %d, u4LowerISO = %d\n", __FUNCTION__, u4RealISO, u4UpperISO, u4LowerISO);

#define InterSWNR(item, method)           \
    do{                                   \
        rSmoothSWNR.item = InterParam_NR( \
                u4RealISO, \
                u4LowerISO, \
                u4UpperISO, \
                rLowerSWNR.item, \
                rUpperSWNR.item, \
                method);                  \
    } while(0)

    // SWNR & FastSWNR part
    InterSWNR(ANR_Y_LUMA_SCALE_RANGE, 0);
    InterSWNR(ANR_C_CHROMA_SCALE, 0);
    InterSWNR(ANR_Y_SCALE_CPY4, 0);
    InterSWNR(ANR_CEN_GAIN_LO_TH, 0);
    InterSWNR(ANR_CEN_GAIN_HI_TH, 0);
    InterSWNR(ANR_KSIZE_LO_TH, 0);
    InterSWNR(ANR_KSIZE_HI_TH, 0);
    InterSWNR(ANR_KSIZE_LO_TH_C, 0);
    InterSWNR(ANR_KSIZE_HI_TH_C, 0);
    InterSWNR(ITUNE_ANR_PTY_STD, 0);
    InterSWNR(ITUNE_ANR_PTU_STD, 0);
    InterSWNR(ITUNE_ANR_PTV_STD, 0);
    InterSWNR(ANR_ACT_TH_Y, 0);
    InterSWNR(ANR_ACT_TH_C, 0);
    InterSWNR(ANR_ACT_BLD_BASE_C, 0);
    InterSWNR(ANR_ACT_BLD_TH_C, 0);
    InterSWNR(ANR_ACT_SLANT_C, 0);
    InterSWNR(RADIUS_H, 0);
    InterSWNR(RADIUS_V, 0);
    InterSWNR(RADIUS_H_C, 0);
    InterSWNR(RADIUS_V_C, 0);
    InterSWNR(ANR_PTC_HGAIN, 0);
    InterSWNR(ANR_LPF_HALFKERNEL, 0);
    InterSWNR(ANR_LPF_HALFKERNEL_C, 0);
    InterSWNR(ANR_ACT_MODE, 1);
    InterSWNR(ANR_LCE_SCALE_GAIN, 0);
    InterSWNR(ANR_LCE_C_GAIN, 0);
    InterSWNR(ANR_MEDIAN_LOCATION, 0);
    InterSWNR(ANR_CEN_X, 0);
    InterSWNR(ANR_CEN_Y, 0);
    InterSWNR(ANR_R1, 0);
    InterSWNR(ANR_R2, 0);
    InterSWNR(ANR_R3, 0);
    InterSWNR(LUMA_ON_OFF, 0);

    // HFG part
    MINT32 HFG_inter_method = 0;
    if (rLowerSWNR.ANR_PTY_HGAIN != rUpperSWNR.ANR_PTY_HGAIN) // HFG_ENABLE
        HFG_inter_method = 1;

    InterSWNR(ANR_PTY_HGAIN, HFG_inter_method);
    InterSWNR(ANR_ACT_BLD_BASE_Y, HFG_inter_method);
    InterSWNR(ANR_ACT_BLD_TH_Y, HFG_inter_method);
    InterSWNR(ANR_ACT_SLANT_Y, HFG_inter_method);
    InterSWNR(ANR_PTY_GAIN_TH, HFG_inter_method);
    InterSWNR(ANR_Y_CPX1, HFG_inter_method);
    InterSWNR(ANR_Y_CPX2, HFG_inter_method);
    InterSWNR(ANR_Y_CPX3, HFG_inter_method);
    InterSWNR(ANR_Y_SCALE_CPY0, HFG_inter_method);
    InterSWNR(ANR_Y_SCALE_CPY1, HFG_inter_method);
    InterSWNR(ANR_Y_SCALE_CPY2, HFG_inter_method);
    InterSWNR(ANR_Y_SCALE_CPY3, HFG_inter_method);
    InterSWNR(ANR_LCE_GAIN0, HFG_inter_method);
    InterSWNR(ANR_LCE_GAIN1, HFG_inter_method);
    InterSWNR(ANR_LCE_GAIN2, HFG_inter_method);
    InterSWNR(ANR_LCE_GAIN3, HFG_inter_method);

    // SWCCR part
    MINT32 CCR_inter_method = 0;
    if (rLowerSWNR.CCR_EN != rUpperSWNR.CCR_EN)
        CCR_inter_method = 1;

    InterSWNR(CCR_EN, CCR_inter_method);
    InterSWNR(CCR_CEN_U, CCR_inter_method);
    InterSWNR(CCR_CEN_V, CCR_inter_method);
    InterSWNR(CCR_Y_CPX1, CCR_inter_method);
    InterSWNR(CCR_Y_CPX2, CCR_inter_method);
    InterSWNR(CCR_Y_CPY1, CCR_inter_method);
    InterSWNR(CCR_UV_X1, CCR_inter_method);
    InterSWNR(CCR_UV_X2, CCR_inter_method);
    InterSWNR(CCR_UV_X3, CCR_inter_method);
    InterSWNR(CCR_UV_GAIN1, CCR_inter_method);
    InterSWNR(CCR_UV_GAIN2, CCR_inter_method);
    InterSWNR(CCR_Y_CPX3, CCR_inter_method);
    InterSWNR(CCR_Y_CPY0, CCR_inter_method);
    InterSWNR(CCR_Y_CPY2, CCR_inter_method);
    InterSWNR(CCR_UV_GAIN_MODE, CCR_inter_method);
    InterSWNR(CCR_MODE, CCR_inter_method);
    InterSWNR(CCR_OR_MODE, CCR_inter_method);
    InterSWNR(CCR_HUE_X1, CCR_inter_method);
    InterSWNR(CCR_HUE_X2, CCR_inter_method);
    InterSWNR(CCR_HUE_X3, CCR_inter_method);
    InterSWNR(CCR_HUE_X4, CCR_inter_method);
    InterSWNR(CCR_HUE_GAIN1, CCR_inter_method);
    InterSWNR(CCR_HUE_GAIN2, CCR_inter_method);
    InterSWNR(CCR_Y_SP2, CCR_inter_method);
    InterSWNR(CCR_UV_GAIN_SP2, CCR_inter_method);

    // Slope parameters update
    MINT32 CCR_Y_SP0;
    MINT32 CCR_Y_SP1;
    MINT32 CCR_UV_GAIN_SP1;
    MINT32 CCR_HUE_SP1;
    MINT32 CCR_HUE_SP2;

    //CCR_Y_SP0
    if(rSmoothSWNR.CCR_Y_CPX1 == 0)
        CCR_Y_SP0 = 0;
    else
    {
        MINT32 Y_SP0 = (MINT32)(32 * (double)(rSmoothSWNR.CCR_Y_CPY1-rSmoothSWNR.CCR_Y_CPY0)/(double)rSmoothSWNR.CCR_Y_CPX1+0.5);
        CCR_Y_SP0 = (Y_SP0>127)?127:Y_SP0;
    }

    //CCR_Y_SP1
    if(rSmoothSWNR.CCR_Y_CPX2 == 0)
        CCR_Y_SP1 = 0;
    else
    {
        MINT32 Y_SP1 = (MINT32)(32 * (double)(rSmoothSWNR.CCR_Y_CPY2-rSmoothSWNR.CCR_Y_CPY1)/(double)(rSmoothSWNR.CCR_Y_CPX2 - rSmoothSWNR.CCR_Y_CPX1)+0.5);
        CCR_Y_SP1 = (Y_SP1>127)?127:Y_SP1;
    }

    //CCR_UV_GAIN_SP1
    if(rSmoothSWNR.CCR_UV_X2 == rSmoothSWNR.CCR_UV_X1)
        CCR_UV_GAIN_SP1 = 0;
    else
    {
        MINT32 UV_SP1 = (MINT32)(32 * (double)(rSmoothSWNR.CCR_UV_GAIN1-rSmoothSWNR.CCR_UV_GAIN2)/(double)(rSmoothSWNR.CCR_UV_X2-rSmoothSWNR.CCR_UV_X1)+0.5);
        CCR_UV_GAIN_SP1 = (UV_SP1>1023)?1023:UV_SP1;
    }

    //CCR_HUE_SP1
    if(rSmoothSWNR.CCR_HUE_X1 == rSmoothSWNR.CCR_HUE_X2)
        CCR_HUE_SP1 = (rSmoothSWNR.CCR_HUE_GAIN1>rSmoothSWNR.CCR_HUE_GAIN2)?-128:127;
    else
    {
        MINT32 HUE_SP1 = (MINT32)(32 * (double)(rSmoothSWNR.CCR_HUE_GAIN2-rSmoothSWNR.CCR_HUE_GAIN1)/(double)(rSmoothSWNR.CCR_HUE_X2-rSmoothSWNR.CCR_HUE_X1));
        CCR_HUE_SP1 = (HUE_SP1>127)?127:(HUE_SP1<-128)?-128:HUE_SP1;
    }

    //CCR_HUE_SP2
    if(rSmoothSWNR.CCR_HUE_X3 == rSmoothSWNR.CCR_HUE_X4)
        CCR_HUE_SP2 = (rSmoothSWNR.CCR_HUE_GAIN1>rSmoothSWNR.CCR_HUE_GAIN2)?127:-128;
    else
    {
        MINT32 HUE_SP2 = (MINT32)(32 * (double)(rSmoothSWNR.CCR_HUE_GAIN1-rSmoothSWNR.CCR_HUE_GAIN2)/(double)(rSmoothSWNR.CCR_HUE_X4-rSmoothSWNR.CCR_HUE_X3));
        CCR_HUE_SP2 = (HUE_SP2>127)?127:(HUE_SP2<-128)?-128:HUE_SP2;
    }

    rSmoothSWNR.CCR_Y_SP0 = CCR_Y_SP0;
    rSmoothSWNR.CCR_Y_SP1 = CCR_Y_SP1;
    rSmoothSWNR.CCR_UV_GAIN_SP1 = CCR_UV_GAIN_SP1;
    rSmoothSWNR.CCR_HUE_SP1 = CCR_HUE_SP1;
    rSmoothSWNR.CCR_HUE_SP2 = CCR_HUE_SP2;

#undef InterSWNR

    if( bSmoothSWNRDebug )
    {
#define SWNR_DUMP(item)                                                                           \
        do{                                                                                       \
            MY_LOG("%s = %d, %d, %d", #item, rLowerSWNR.item, rSmoothSWNR.item, rUpperSWNR.item); \
        }while(0)

        SWNR_DUMP(ANR_Y_LUMA_SCALE_RANGE);
        SWNR_DUMP(ANR_C_CHROMA_SCALE);
        SWNR_DUMP(ANR_Y_SCALE_CPY0);
        SWNR_DUMP(ANR_Y_SCALE_CPY1);
        SWNR_DUMP(ANR_Y_SCALE_CPY2);
        SWNR_DUMP(ANR_Y_SCALE_CPY3);
        SWNR_DUMP(ANR_Y_SCALE_CPY4);
        SWNR_DUMP(ANR_Y_CPX1);
        SWNR_DUMP(ANR_Y_CPX2);
        SWNR_DUMP(ANR_Y_CPX3);
        SWNR_DUMP(ANR_CEN_GAIN_LO_TH);
        SWNR_DUMP(ANR_CEN_GAIN_HI_TH);
        SWNR_DUMP(ANR_PTY_GAIN_TH);
        SWNR_DUMP(ANR_KSIZE_LO_TH);
        SWNR_DUMP(ANR_KSIZE_HI_TH);
        SWNR_DUMP(ANR_KSIZE_LO_TH_C);
        SWNR_DUMP(ANR_KSIZE_HI_TH_C);
        SWNR_DUMP(ITUNE_ANR_PTY_STD);
        SWNR_DUMP(ITUNE_ANR_PTU_STD);
        SWNR_DUMP(ITUNE_ANR_PTV_STD);
        SWNR_DUMP(ANR_ACT_TH_Y);
        SWNR_DUMP(ANR_ACT_BLD_BASE_Y);
        SWNR_DUMP(ANR_ACT_BLD_TH_Y);
        SWNR_DUMP(ANR_ACT_SLANT_Y);
        SWNR_DUMP(ANR_ACT_TH_C);
        SWNR_DUMP(ANR_ACT_BLD_BASE_C);
        SWNR_DUMP(ANR_ACT_BLD_TH_C);
        SWNR_DUMP(ANR_ACT_SLANT_C);
        SWNR_DUMP(RADIUS_H);
        SWNR_DUMP(RADIUS_V);
        SWNR_DUMP(RADIUS_H_C);
        SWNR_DUMP(RADIUS_V_C);
        SWNR_DUMP(ANR_PTC_HGAIN);
        SWNR_DUMP(ANR_PTY_HGAIN);
        SWNR_DUMP(ANR_LPF_HALFKERNEL);
        SWNR_DUMP(ANR_LPF_HALFKERNEL_C);
        SWNR_DUMP(ANR_ACT_MODE);
        SWNR_DUMP(ANR_LCE_SCALE_GAIN);
        SWNR_DUMP(ANR_LCE_C_GAIN);
        SWNR_DUMP(ANR_LCE_GAIN0);
        SWNR_DUMP(ANR_LCE_GAIN1);
        SWNR_DUMP(ANR_LCE_GAIN2);
        SWNR_DUMP(ANR_LCE_GAIN3);
        SWNR_DUMP(ANR_MEDIAN_LOCATION);
        SWNR_DUMP(ANR_CEN_X);
        SWNR_DUMP(ANR_CEN_Y);
        SWNR_DUMP(ANR_R1);
        SWNR_DUMP(ANR_R2);
        SWNR_DUMP(ANR_R3);
        SWNR_DUMP(LUMA_ON_OFF);
        SWNR_DUMP(CCR_EN);
        SWNR_DUMP(CCR_CEN_U);
        SWNR_DUMP(CCR_CEN_V);
        SWNR_DUMP(CCR_Y_CPX1);
        SWNR_DUMP(CCR_Y_CPX2);
        SWNR_DUMP(CCR_Y_CPY1);
        SWNR_DUMP(CCR_Y_SP1);
        SWNR_DUMP(CCR_UV_X1);
        SWNR_DUMP(CCR_UV_X2);
        SWNR_DUMP(CCR_UV_X3);
        SWNR_DUMP(CCR_UV_GAIN1);
        SWNR_DUMP(CCR_UV_GAIN2);
        SWNR_DUMP(CCR_UV_GAIN_SP1);
        SWNR_DUMP(CCR_UV_GAIN_SP2);
        SWNR_DUMP(CCR_Y_CPX3);
        SWNR_DUMP(CCR_Y_CPY0);
        SWNR_DUMP(CCR_Y_CPY2);
        SWNR_DUMP(CCR_Y_SP0);
        SWNR_DUMP(CCR_Y_SP2);
        SWNR_DUMP(CCR_UV_GAIN_MODE);
        SWNR_DUMP(CCR_MODE);
        SWNR_DUMP(CCR_OR_MODE);
        SWNR_DUMP(CCR_HUE_X1);
        SWNR_DUMP(CCR_HUE_X2);
        SWNR_DUMP(CCR_HUE_X3);
        SWNR_DUMP(CCR_HUE_X4);
        SWNR_DUMP(CCR_HUE_SP1);
        SWNR_DUMP(CCR_HUE_SP2);
        SWNR_DUMP(CCR_HUE_GAIN1);
        SWNR_DUMP(CCR_HUE_GAIN2);

#undef SWNR_DUMP
    }
}



