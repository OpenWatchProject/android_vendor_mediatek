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

#include "camera_custom_types.h"
#include "af_param.h"
#include "af_tuning_custom.h"
#include "af_tuning_custom_cfg.h"

/*******************************************************************************
*
********************************************************************************/
template <>
MBOOL
_getAFParam<ESensorDev_MainSecond>( AF_PARAM_T const **outAFParam)
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

    AAA_LOGD( "%s : dev( ESensorDev_MainSecond ) get specialization setting, 0x%x, tbl_size(%d)",
              __FUNCTION__,
              (*outAFParam),
              (*outAFParam)->i4TBLL);

    return MTRUE;
}

template <>
MBOOL
_getAFConfig<ESensorDev_MainSecond>( AF_CONFIG_T const **outAFCfg)
{
    static AF_CONFIG_T g_AFconfig;

    /* should be modified before set to HW*/
    g_AFconfig.sTG_SZ.i4H  = 0;
    g_AFconfig.sTG_SZ.i4W  = 0;
    g_AFconfig.sBIN_SZ.i4H = 0;
    g_AFconfig.sBIN_SZ.i4W = 0;
    g_AFconfig.sRoi.i4X    = 0;
    g_AFconfig.sRoi.i4Y    = 0;
    g_AFconfig.sRoi.i4W    = 0;
    g_AFconfig.sRoi.i4H    = 0;
    g_AFconfig.sRoi.i4Info = 0;
    /* HW default setting */
    g_AFconfig.AF_BLK_XNUM   = 32;
    g_AFconfig.AF_BLK_YNUM   = 32;
    g_AFconfig.i4SGG_GAIN    = 16;
    g_AFconfig.i4SGG_GMR1    = 20;
    g_AFconfig.i4SGG_GMR2    = 29;
    g_AFconfig.i4SGG_GMR3    = 43;
    g_AFconfig.i4SGG_GMR4    = 62;
    g_AFconfig.i4SGG_GMR5    = 88;
    g_AFconfig.i4SGG_GMR6    = 126;
    g_AFconfig.i4SGG_GMR7    = 180;
    g_AFconfig.AF_H_GONLY    = 0;
    g_AFconfig.AF_V_GONLY    = 0;
    g_AFconfig.AF_V_AVG_LVL  = 3;
    g_AFconfig.AF_BLF[0]     = 1;
    g_AFconfig.AF_BLF[1]     = 3;
    g_AFconfig.AF_BLF[2]     = 7;
    g_AFconfig.AF_BLF[3]     = 1;
    g_AFconfig.AF_TH_H[0]    = 1;
    g_AFconfig.AF_TH_H[1]    = 1;
    g_AFconfig.AF_TH_V       = 1;
    g_AFconfig.AF_TH_G_SAT   = 240;
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
    g_AFconfig.AF_FIL_H1[0]  = 15;
    g_AFconfig.AF_FIL_H1[1]  = 36;
    g_AFconfig.AF_FIL_H1[2]  = 43;
    g_AFconfig.AF_FIL_H1[3]  = 36;
    g_AFconfig.AF_FIL_H1[4]  = 22;
    g_AFconfig.AF_FIL_H1[5]  = 10;
    g_AFconfig.AF_FIL_H1[6]  = 2;
    g_AFconfig.AF_FIL_H1[7]  = 0;
    g_AFconfig.AF_FIL_H1[8]  = 0;
    g_AFconfig.AF_FIL_H1[9]  = 0;
    g_AFconfig.AF_FIL_H1[10] = 0;
    g_AFconfig.AF_FIL_H1[11] = 0;
    g_AFconfig.AF_FIL_V[0]   = 256;
    g_AFconfig.AF_FIL_V[1]   = 256;
    g_AFconfig.AF_FIL_V[2]   = 1792;
    g_AFconfig.AF_FIL_V[3]   = 1792;
    /* EXT mode 3.5*/
    g_AFconfig.AF_EXT_ENABLE = 1;
    g_AFconfig.AF_TH_R_SAT   = 240;
    g_AFconfig.AF_TH_B_SAT   = 240;
    g_AFconfig.AF_TH_H2[0]   = 255;
    g_AFconfig.AF_TH_H2[1]   = 255;
    g_AFconfig.AF_TH_H2[2]   = 255;
    g_AFconfig.AF_TH_H2[3]   = 255;
    g_AFconfig.i4SGG5_GAIN   = 16;
    g_AFconfig.i4SGG5_GMR[0] = 2;
    g_AFconfig.i4SGG5_GMR[1] = 4;
    g_AFconfig.i4SGG5_GMR[2] = 8;
    g_AFconfig.i4SGG5_GMR[3] = 16;
    g_AFconfig.i4SGG5_GMR[4] = 32;
    g_AFconfig.i4SGG5_GMR[5] = 64;
    g_AFconfig.i4SGG5_GMR[6] = 128;


    /***********
     *  Output
     ***********/
    (*outAFCfg) = &g_AFconfig;

    AAA_LOGD( "%s : dev( ESensorDev_MainSecond ) get specialization setting, 0x%x, AF_EXT_ENABLE(%d)",
              __FUNCTION__,
              (*outAFCfg),
              (*outAFCfg)->AF_EXT_ENABLE);

    return MTRUE;


}


