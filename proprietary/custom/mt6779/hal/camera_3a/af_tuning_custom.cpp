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

/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#define LOG_TAG "af_tuning_custom"

#include "camera_custom_types.h"
#include "af_param.h"
#include "af_tuning_custom.h"
#include "aaa_log.h"
#include <mutex>

static std::mutex g_AFLock;

/*******************************************************************************
*
********************************************************************************/
MBOOL _getAFParam( AF_PARAM_T const **outAFParam)
{
    static AF_PARAM_T g_AFparam =
    {
        1,   // i4AFS_MODE   0 : singleAF, 1:smoothAF
        1,   // i4AFC_MODE   0 : singleAF, 1:smoothAF
        2,   // i4VAFC_MODE  0 : singleAF, 1:smoothAF     2:smoothAF + 3 points curve fitting.
        9,   // i4TBLL
        {50, 100,150,200,400,800,1200,2000,3000,0,0,0,0,0,0}, //i4Dist
        {530,480,420,380,350,320, 300, 200, 195,0,0,0,0,0,0}, //i4Dacv
        { 10, 20, 40, 80,120,200, 250, 300, 350,0,0,0,0,0,0}, //i4FocusRange

    };

    // Boundary check
    g_AFparam.i4TBLL =    g_AFparam.i4TBLL<=DistTBLLength ? g_AFparam.i4TBLL : DistTBLLength;
    g_AFparam.i4TBLL = 0<=g_AFparam.i4TBLL                ? g_AFparam.i4TBLL : 0;


    /***********
     *  Output
     ***********/
    (*outAFParam) = &g_AFparam;

    AAA_LOGD( "%s : use default setting, 0x%x, tbl_size(%d)",
              __FUNCTION__,
              (*outAFParam),
              (*outAFParam)->i4TBLL);

    return MTRUE;
}

MBOOL _getAFConfig( AF_CONFIG_T const **outAFCfg)
{
    static AF_CONFIG_T g_AFconfig;

    /* should be modified before set to HW*/
    g_AFconfig.sTG_SZ.i4W  = 4000;
    g_AFconfig.sTG_SZ.i4H  = 3000;
    g_AFconfig.sBIN_SZ.i4W = 4000;
    g_AFconfig.sBIN_SZ.i4H = 3000;
    g_AFconfig.sRoi.i4X    = 1600;
    g_AFconfig.sRoi.i4Y    = 1100;
    g_AFconfig.sRoi.i4W    = 800;
    g_AFconfig.sRoi.i4H    = 800;
    g_AFconfig.sRoi.i4Info = 0;

    /* HW default setting */
    g_AFconfig.AF_BLF[0]     = 1;
    g_AFconfig.AF_BLF[1]     = 3;
    g_AFconfig.AF_BLF[2]     = 7;
    g_AFconfig.AF_BLF[3]     = 1;
    g_AFconfig.AF_H_GONLY    = 0;
    g_AFconfig.AF_V_GONLY    = 0;
    g_AFconfig.AF_V_AVG_LVL  = 3;
    g_AFconfig.AF_V_FLT_MODE = 0;

    g_AFconfig.AF_DS_En         = 0;
    g_AFconfig.AF_FV_H_ABS[0]   = 0;
    g_AFconfig.AF_FV_H_ABS[1]   = 0;
    g_AFconfig.AF_FV_H_ABS[2]   = 0;
    g_AFconfig.AF_FV_V_ABS      = 0;
    g_AFconfig.AF_FV_PL_H_ABS   = 0;
    g_AFconfig.AF_FV_PL_V_ABS   = 0;
    g_AFconfig.AF_FV_H_En[0]    = 1;
    g_AFconfig.AF_FV_H_En[1]    = 1;
    g_AFconfig.AF_FV_H_En[2]    = 1;
    g_AFconfig.AF_FV_V_En       = 1;
    g_AFconfig.AF_FV_PL_En      = 1;
    g_AFconfig.AF_8BIT_LOWPOWER_En = 1;
    g_AFconfig.AF_PL_BITSEL     = 0;

    g_AFconfig.AF_BLK_XNUM   = 50;
    g_AFconfig.AF_BLK_YNUM   = 50;
    g_AFconfig.AF_BLK_XSIZE  = 16;
    g_AFconfig.AF_BLK_YSIZE  = 16;

    g_AFconfig.AF_FIL_H0[0]  = 15;
    g_AFconfig.AF_FIL_H0[1]  = 36;
    g_AFconfig.AF_FIL_H0[2]  = 43;
    g_AFconfig.AF_FIL_H0[3]  = 36;
    g_AFconfig.AF_FIL_H0[4]  = 22;
    g_AFconfig.AF_FIL_H0[5]  = 10;
    g_AFconfig.AF_FIL_H0[6]  = 2;
    g_AFconfig.AF_FIL_H0[7]  = 0;
    g_AFconfig.AF_FIL_H0[8]  = 0;
    g_AFconfig.AF_FIL_H0[9]  = 0;
    g_AFconfig.AF_FIL_H0[10] = 0;
    g_AFconfig.AF_FIL_H0[11] = 0;

    g_AFconfig.AF_FIL_H1[0]  = 10;
    g_AFconfig.AF_FIL_H1[1]  = 29;
    g_AFconfig.AF_FIL_H1[2]  = 37;
    g_AFconfig.AF_FIL_H1[3]  = 34;
    g_AFconfig.AF_FIL_H1[4]  = 23;
    g_AFconfig.AF_FIL_H1[5]  = 8;
    g_AFconfig.AF_FIL_H1[6]  = -5;
    g_AFconfig.AF_FIL_H1[7]  = -12;
    g_AFconfig.AF_FIL_H1[8]  = -13;
    g_AFconfig.AF_FIL_H1[9]  = -10;
    g_AFconfig.AF_FIL_H1[10] = -6;
    g_AFconfig.AF_FIL_H1[11] = -2;

    g_AFconfig.AF_FIL_H2[0]  = 4;
    g_AFconfig.AF_FIL_H2[1]  = 13;
    g_AFconfig.AF_FIL_H2[2]  = 21;
    g_AFconfig.AF_FIL_H2[3]  = 24;
    g_AFconfig.AF_FIL_H2[4]  = 27;
    g_AFconfig.AF_FIL_H2[5]  = 27;
    g_AFconfig.AF_FIL_H2[6]  = 23;
    g_AFconfig.AF_FIL_H2[7]  = 22;
    g_AFconfig.AF_FIL_H2[8]  = 16;
    g_AFconfig.AF_FIL_H2[9]  = 9;
    g_AFconfig.AF_FIL_H2[10] = 8;
    g_AFconfig.AF_FIL_H2[11] = 4;

    g_AFconfig.AF_FIL_V[0]  = -10;
    g_AFconfig.AF_FIL_V[1]  = -22;
    g_AFconfig.AF_FIL_V[2]  = -36;
    g_AFconfig.AF_FIL_V[3]  = -43;
    g_AFconfig.AF_FIL_V[4]  = -36;
    g_AFconfig.AF_FIL_V[5]  = -15;
    g_AFconfig.AF_FIL_V[6]  = 15;
    g_AFconfig.AF_FIL_V[7]  = 36;
    g_AFconfig.AF_FIL_V[8]  = 43;
    g_AFconfig.AF_FIL_V[9]  = 36;
    g_AFconfig.AF_FIL_V[10] = 22;
    g_AFconfig.AF_FIL_V[11] = 10;

    g_AFconfig.AF_FIL_PL_H[0]  = 10;
    g_AFconfig.AF_FIL_PL_H[1]  = 29;
    g_AFconfig.AF_FIL_PL_H[2]  = 37;
    g_AFconfig.AF_FIL_PL_H[3]  = 34;
    g_AFconfig.AF_FIL_PL_H[4]  = 23;
    g_AFconfig.AF_FIL_PL_H[5]  = 8;
    g_AFconfig.AF_FIL_PL_H[6]  = -5;
    g_AFconfig.AF_FIL_PL_H[7]  = -12;
    g_AFconfig.AF_FIL_PL_H[8]  = -13;
    g_AFconfig.AF_FIL_PL_H[9]  = -10;
    g_AFconfig.AF_FIL_PL_H[10] = -6;
    g_AFconfig.AF_FIL_PL_H[11] = -2;

    g_AFconfig.AF_FIL_PL_V[0]  = 255;
    g_AFconfig.AF_FIL_PL_V[1]  = 255;
    g_AFconfig.AF_FIL_PL_V[2]  = -255;
    g_AFconfig.AF_FIL_PL_V[3]  = -255;
    g_AFconfig.AF_FIL_PL_V[4]  = 0;
    g_AFconfig.AF_FIL_PL_V[5]  = 0;
    g_AFconfig.AF_FIL_PL_V[6]  = 0;
    g_AFconfig.AF_FIL_PL_V[7]  = 0;
    g_AFconfig.AF_FIL_PL_V[8]  = 0;
    g_AFconfig.AF_FIL_PL_V[9]  = 0;
    g_AFconfig.AF_FIL_PL_V[10] = 0;
    g_AFconfig.AF_FIL_PL_V[11] = 0;

    g_AFconfig.AF_TH_H[0]    = 0;
    g_AFconfig.AF_TH_H[1]    = 0;
    g_AFconfig.AF_TH_H[2]    = 0;
    g_AFconfig.AF_TH_V       = 0;
    g_AFconfig.AF_TH_H_PL    = 0;
    g_AFconfig.AF_TH_G_SAT   = 240;
    g_AFconfig.AF_TH_B_SAT   = 250;
    g_AFconfig.AF_TH_R_SAT   = 250;

    g_AFconfig.AF_LUT_H_MODE[0] = 0;
    g_AFconfig.AF_LUT_H_GAIN[0] = 256;
    g_AFconfig.AF_LUT_H_TH0_Dn[0]=32;
    g_AFconfig.AF_LUT_H_TH0_Dn[1]=64;
    g_AFconfig.AF_LUT_H_TH0_Dn[2]=96;
    g_AFconfig.AF_LUT_H_TH0_Dn[3]=128;
    g_AFconfig.AF_LUT_H_TH0_Dn[4]=160;
    g_AFconfig.AF_LUT_H_TH0_Dn[5]=192;
    g_AFconfig.AF_LUT_H_TH0_Dn[6]=224;
    g_AFconfig.AF_LUT_H_TH0_Dn[7]=255;
    g_AFconfig.AF_LUT_H_TH0_Dn[8]=0;
    g_AFconfig.AF_LUT_H_TH0_Dn[9]=0;
    g_AFconfig.AF_LUT_H_TH0_Dn[10]=0;
    g_AFconfig.AF_LUT_H_TH0_Dn[11]=0;
    g_AFconfig.AF_LUT_H_TH0_Dn[12]=0;
    g_AFconfig.AF_LUT_H_TH0_Dn[13]=0;
    g_AFconfig.AF_LUT_H_TH0_Dn[14]=0;
    g_AFconfig.AF_LUT_H_TH0_Dn[15]=0;

    g_AFconfig.AF_LUT_H_MODE[1] = 0;
    g_AFconfig.AF_LUT_H_GAIN[1] = 256;
    g_AFconfig.AF_LUT_H_TH1_Dn[0]=32;
    g_AFconfig.AF_LUT_H_TH1_Dn[1]=64;
    g_AFconfig.AF_LUT_H_TH1_Dn[2]=96;
    g_AFconfig.AF_LUT_H_TH1_Dn[3]=128;
    g_AFconfig.AF_LUT_H_TH1_Dn[4]=160;
    g_AFconfig.AF_LUT_H_TH1_Dn[5]=192;
    g_AFconfig.AF_LUT_H_TH1_Dn[6]=224;
    g_AFconfig.AF_LUT_H_TH1_Dn[7]=255;
    g_AFconfig.AF_LUT_H_TH1_Dn[8]=0;
    g_AFconfig.AF_LUT_H_TH1_Dn[9]=0;
    g_AFconfig.AF_LUT_H_TH1_Dn[10]=0;
    g_AFconfig.AF_LUT_H_TH1_Dn[11]=0;
    g_AFconfig.AF_LUT_H_TH1_Dn[12]=0;
    g_AFconfig.AF_LUT_H_TH1_Dn[13]=0;
    g_AFconfig.AF_LUT_H_TH1_Dn[14]=0;
    g_AFconfig.AF_LUT_H_TH1_Dn[15]=0;

    g_AFconfig.AF_LUT_H_MODE[2] = 0;
    g_AFconfig.AF_LUT_H_GAIN[2] = 256;
    g_AFconfig.AF_LUT_H_TH2_Dn[0]=32;
    g_AFconfig.AF_LUT_H_TH2_Dn[1]=64;
    g_AFconfig.AF_LUT_H_TH2_Dn[2]=96;
    g_AFconfig.AF_LUT_H_TH2_Dn[3]=128;
    g_AFconfig.AF_LUT_H_TH2_Dn[4]=160;
    g_AFconfig.AF_LUT_H_TH2_Dn[5]=192;
    g_AFconfig.AF_LUT_H_TH2_Dn[6]=224;
    g_AFconfig.AF_LUT_H_TH2_Dn[7]=255;
    g_AFconfig.AF_LUT_H_TH2_Dn[8]=0;
    g_AFconfig.AF_LUT_H_TH2_Dn[9]=0;
    g_AFconfig.AF_LUT_H_TH2_Dn[10]=0;
    g_AFconfig.AF_LUT_H_TH2_Dn[11]=0;
    g_AFconfig.AF_LUT_H_TH2_Dn[12]=0;
    g_AFconfig.AF_LUT_H_TH2_Dn[13]=0;
    g_AFconfig.AF_LUT_H_TH2_Dn[14]=0;
    g_AFconfig.AF_LUT_H_TH2_Dn[15]=0;

    g_AFconfig.AF_LUT_V_MODE    = 0;
    g_AFconfig.AF_LUT_V_GAIN    = 256;
    g_AFconfig.AF_LUT_V_TH_Dn[0]=32;
    g_AFconfig.AF_LUT_V_TH_Dn[1]=64;
    g_AFconfig.AF_LUT_V_TH_Dn[2]=96;
    g_AFconfig.AF_LUT_V_TH_Dn[3]=128;
    g_AFconfig.AF_LUT_V_TH_Dn[4]=160;
    g_AFconfig.AF_LUT_V_TH_Dn[5]=192;
    g_AFconfig.AF_LUT_V_TH_Dn[6]=224;
    g_AFconfig.AF_LUT_V_TH_Dn[7]=255;
    g_AFconfig.AF_LUT_V_TH_Dn[8]=0;
    g_AFconfig.AF_LUT_V_TH_Dn[9]=0;
    g_AFconfig.AF_LUT_V_TH_Dn[10]=0;
    g_AFconfig.AF_LUT_V_TH_Dn[11]=0;
    g_AFconfig.AF_LUT_V_TH_Dn[12]=0;
    g_AFconfig.AF_LUT_V_TH_Dn[13]=0;
    g_AFconfig.AF_LUT_V_TH_Dn[14]=0;
    g_AFconfig.AF_LUT_V_TH_Dn[15]=0;

    g_AFconfig.AF_SGG1_GAIN     = 16;
    g_AFconfig.AF_SGG1_GMR[0]   = 20;
    g_AFconfig.AF_SGG1_GMR[1]   = 29;
    g_AFconfig.AF_SGG1_GMR[2]   = 43;
    g_AFconfig.AF_SGG1_GMR[3]   = 62;
    g_AFconfig.AF_SGG1_GMR[4]   = 88;
    g_AFconfig.AF_SGG1_GMR[5]   = 126;
    g_AFconfig.AF_SGG1_GMR[6]   = 180;
    g_AFconfig.AF_SGG1_GMR[7]   = 255;
    g_AFconfig.AF_SGG1_GMR[8]   = 512;
    g_AFconfig.AF_SGG1_GMR[9]   = 1024;
    g_AFconfig.AF_SGG1_GMR[10]  = 2047;

    g_AFconfig.AF_SGG5_GAIN     = 16;
    g_AFconfig.AF_SGG5_GMR[0]   = 2;
    g_AFconfig.AF_SGG5_GMR[1]   = 4;
    g_AFconfig.AF_SGG5_GMR[2]   = 8;
    g_AFconfig.AF_SGG5_GMR[3]   = 16;
    g_AFconfig.AF_SGG5_GMR[4]   = 32;
    g_AFconfig.AF_SGG5_GMR[5]   = 64;
    g_AFconfig.AF_SGG5_GMR[6]   = 128;
    g_AFconfig.AF_SGG5_GMR[7]   = 256;
    g_AFconfig.AF_SGG5_GMR[8]   = 512;
    g_AFconfig.AF_SGG5_GMR[9]   = 1024;
    g_AFconfig.AF_SGG5_GMR[10]  = 2047;

    /***********
     *  Output
     ***********/
    (*outAFCfg) = &g_AFconfig;

    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL getAFParam( MINT32 i4SensorDev, AF_PARAM_T const **outAFParam)
{
    MBOOL ret=MFALSE;

    g_AFLock.lock();
    ret = _getAFParam(outAFParam);
    g_AFLock.unlock();

    return ret;
}


MBOOL getAFConfig( MINT32 i4SensorDev, AF_CONFIG_T const **outAFCfg)
{
    MBOOL ret=MFALSE;

    g_AFLock.lock();
    ret = _getAFConfig(outAFCfg);
    g_AFLock.unlock();

    return ret;
}

