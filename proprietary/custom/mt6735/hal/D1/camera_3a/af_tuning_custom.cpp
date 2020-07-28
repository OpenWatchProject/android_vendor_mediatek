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

/*******************************************************************************
*
********************************************************************************/
AF_PARAM_T const&
getAFParam()
{
    static AF_PARAM_T g_AFparam = 
    {
        1,   // i4AFS_MODE  0 : singleAF, 1:smoothAF
        1,   // i4AFC_MODE  0 : singleAF, 1:smoothAF
        2,   // i4VAFC_MODE  0 : singleAF, 1:smoothAF     2:smoothAF + 3 points curve fitting.   
        
        3,      //i4DafTuningMaxW
        3,      //i4DafTuningMaxH
        1500,   //i4PDafTuningLevel0
        3500,   //i4PDafTuningLevel1
        2500,   //i4PDafTuningLevel2
        1500,   //i4PDafTuningLevel3

        9, //i4TBLL
        {50, 100,150,200,400,800,1200,2000,3000,0,0,0,0,0,0}, //i4Dist
        {530,480,420,380,350,320, 300, 200, 195,0,0,0,0,0,0}, //i4Dacv
        { 10, 20, 40, 80,120,200, 250, 300, 350,0,0,0,0,0,0}, //i4Dacv

        // PD+LASER
        3000,  //i4LDafTuningLevel0
        3000,  //i4LDafTuningLevel1
        3000,  //i4LDafTuningLevel2
        3000,  //i4LDafTuningLevel3
        3000,  //i4SDafTuningLevel0
        3000,  //i4SDafTuningLevel1
        3000,  //i4SDafTuningLevel2
        3000,  //i4SDafTuningLevel3

        {3,2,1},  //High
        {3,2,1},  //Mid
        {1,2,3},  //Low
    };

    return g_AFparam;
}

AF_CONFIG_T const&
getAFConfig()
{
    static AF_CONFIG_T g_AFconfig;

    g_AFconfig.i4SGG_GAIN = 16;
    g_AFconfig.i4SGG_GMR1 = 31;
    g_AFconfig.i4SGG_GMR2 = 63;
    g_AFconfig.i4SGG_GMR3 = 127;    
    g_AFconfig.AF_DECI_1  = 0;
    g_AFconfig.AF_ZIGZAG  = 0;
    g_AFconfig.AF_ODD     = 0;
    g_AFconfig.AF_FILT1[0] = 15;
    g_AFconfig.AF_FILT1[1] = 36;
    g_AFconfig.AF_FILT1[2] = 43;
    g_AFconfig.AF_FILT1[3] = 36;
    g_AFconfig.AF_FILT1[4] = 22;
    g_AFconfig.AF_FILT1[5] = 10;
    g_AFconfig.AF_FILT1[6] = 3;
    g_AFconfig.AF_FILT1[7] = 0;
    g_AFconfig.AF_FILT1[8] = 0;
    g_AFconfig.AF_FILT1[9] = 0;    
    g_AFconfig.AF_FILT1[10] = 0;
    g_AFconfig.AF_FILT1[11] = 0;

    g_AFconfig.AF_FILT2[0] =0x100;
    g_AFconfig.AF_FILT2[1] =0x100;
    g_AFconfig.AF_FILT2[2] =0x700;
    g_AFconfig.AF_FILT2[3] =0x700;

    g_AFconfig.AF_TH[0] = 5;
    g_AFconfig.AF_TH[1] = 5;

    g_AFconfig.AF_THEX = 5;

    return g_AFconfig;
}

//////////////////////////////////////////////////
///
//   For multi sensor mode, extend lens_para_xxxx.cpp coef. 
//   AF threshold setting for different sensor resolution
//
//   If current sensor output size match to one of this table, 
//   AF will use the coef of the entry to do AF. 
//   or, AF will use coef. of lens_para_xxxxx.cpp 
//
//   input param. :  
//     w: sensor mode output image width
//     h: sensor mode output image height
//     SensorID:  current sensor ID, 
//     
//     The mapping table for sensor ID can be found in kd_imgsensor.h (custom\common\kernel\imgsensor\inc)
//     for example, 
//         #define OV8825_SENSOR_ID  0x8825
//         and the SensorID = 0x8825
//   
/////////////////////////////////////////////////////////
#ifdef AFEXTENDCOEF
CustAF_THRES_T const& getAFExtendCoef(MINT32 w, MINT32 h, MINT32 SensorId)
{
    static AF_Extend_Coef_T AFTH_ExtCoef[]=
    {
        {//set1   //example
            0,0, 0, //320,240, 0x5648//resolution, SensorId
            {
                8, // i4ISONum;
                {100,150,200,300,400,600,800,1600}, // i4ISO[ISO_MAX_NUM];
                // SGG1~7
                {20, 19, 19, 18, 18, 17, 16, 15,
                 29, 29, 28, 28, 27, 26, 26, 24,
                 43, 42, 42, 41, 41, 40, 39, 37,
                 62, 61, 61, 60, 60, 59, 58, 56,
                 88, 88, 88, 87, 87, 86, 85, 84,
                 126, 126, 126, 125, 125, 125, 124, 123,
                 180, 180, 180, 179, 179, 179, 179, 178},
                {0,0,0,0,0,0,0,0}, // i4FV_DC[ISO_MAX_NUM];
                {50000,50000,50000,50000,50000,50000,50000,50000}, // i4MIN_TH[ISO_MAX_NUM];
                {5,5,5,5,5,5,5,5}, // i4HW_TH[ISO_MAX_NUM];
                {0,0,0,0,0,0,0,0}, // i4FV_DC2[ISO_MAX_NUM];
                {0,0,0,0,0,0,0,0}, // i4MIN_TH2[ISO_MAX_NUM];
                {5,5,5,5,5,5,5,5}  // i4HW_TH2[ISO_MAX_NUM];
            },
        },
    
        {//set2   //example
            0,0, 0,//800,600, 0x8825//resolution, SensorId
            {
                8, // i4ISONum;
                {101,151,201,300,400,600,800,1600}, // i4ISO[ISO_MAX_NUM];
                // SGG1~7
                {20, 19, 19, 18, 18, 17, 16, 15,
                 29, 29, 28, 28, 27, 26, 26, 24,
                 43, 42, 42, 41, 41, 40, 39, 37,
                 62, 61, 61, 60, 60, 59, 58, 56,
                 88, 88, 88, 87, 87, 86, 85, 84,
                 126, 126, 126, 125, 125, 125, 124, 123,
                 180, 180, 180, 179, 179, 179, 179, 178},
                {0,0,0,0,0,0,0,0}, // i4FV_DC[ISO_MAX_NUM];
                {998888,998888,998888,998888,998888,998888,998888,998888}, // i4MIN_TH[ISO_MAX_NUM];
                {5,5,5,5,5,5,5,5}, // i4HW_TH[ISO_MAX_NUM];
                {0,0,0,0,0,0,0,0}, // i4FV_DC2[ISO_MAX_NUM];
                {0,0,0,0,0,0,0,0}, // i4MIN_TH2[ISO_MAX_NUM];
                {5,5,5,5,5,5,5,5}  // i4HW_TH2[ISO_MAX_NUM];
            },
        },
        //..... increase set below if need.
    };

    MUINT32 coefidx=0;
    for(coefidx=0;coefidx< sizeof(AFTH_ExtCoef)/sizeof(AF_Extend_Coef_T);coefidx++)
    {
        if((AFTH_ExtCoef[coefidx].i4ImageWidth==w)
         &&(AFTH_ExtCoef[coefidx].i4ImageHeight==h)
         &&(AFTH_ExtCoef[coefidx].i4SensorID==SensorId)
         && (w>0) && (h>0))
            return AFTH_ExtCoef[coefidx].Coef;
    }
    AFTH_ExtCoef[0].Coef.i4ISONum=0;
    return AFTH_ExtCoef[0].Coef;
    
}
#endif

