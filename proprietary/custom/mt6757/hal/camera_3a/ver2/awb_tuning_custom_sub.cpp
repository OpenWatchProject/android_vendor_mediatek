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

#include "camera_custom_types.h"
#include "camera_custom_nvram.h"
#include "awb_feature.h"
#include "awb_param.h"
#include "awb_tuning_custom.h"

using namespace NSIspTuning;

#define AWB_PARAM_TUNING_NUM	6511001

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
template <>
MBOOL
isAWBEnabled<ESensorDev_Sub>()
{
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
template <>
AWB_PARAM_T const&
getAWBParam<ESensorDev_Sub>()
{
    static AWB_PARAM_T rAWBParam =
    {
/******************************************************************************/
//Feature name:
//AWB chip dependent parameter
//variable name: i4AWBGainOutputScaleUnit
//It is used for AWB output gain normailization, please do not modify it.
//default value: 512
//unit: 512 = 1x
//variable name: i4AWBGainOutputUpperLimit
//Maximum AWB gain is ~16x, it is hardware limitation. Please do not modify it.
//default value: 8191
//unit: 512 = 1x
//variable name: i4RotationMatrixUnit
//AWB rotation matrix unit. Please do not modify it.
//default value: 256
//unit: 256 = 1x
/******************************************************************************/
    	// Chip dependent parameter
    	{
    	    512, // i4AWBGainOutputScaleUnit: 1.0x = 512
    	   8191, // i4AWBGainOutputUpperLimit: format 4.9 (11 bit)
    	    256  // i4RotationMatrixUnit: 1.0x = 256
    	},
/******************************************************************************/
//Feature name:
//Weighting factor to apply on white point R,G,B summation.
//Weight = 1 + ((SumR + SumG + SumB)/ChildBlkNum) / 2^(i4ScalingFactor)
//variable name: bEnable
//default value: 1
//data range: 0 and 1
//0 means disable weighting factor apply to R,G,B summation
//1 means enable weighting factor apply to R,G,B summation
//varaiable name: i4ScalingFactor
//default value: 6
//data range: 6~10
//6 means Wieght will be 1~12
//7 means Weight will be 1~6
//8 means Weight will be 1~3
//9 means Weight will be 1~2
//9 means Weight will be 1~2
//10 means Weight will be 1
/******************************************************************************/
    	// Parent block weight parameter
    	{
    	    1, // bEnable
    	    6 // i4ScalingFactor: [6] 1~12, [7] 1~6, [8] 1~3, [9] 1~2, [>=10]: 1
    		},
/******************************************************************************/
//Feature name:
//AWB behavior control for one-shot capture for dark environment
//variable name: bSmoothEnable
//default value: 0
//data range: 0 and 1, 0 means false and 1 means true
//MFALSE means disable smooth one-shot AWB for dark environment: use preview AWB gain directly
//MTRUE means enable smooth one-shot AWB for dark environment: take weighted average of one-shot AWB gain and
//preview AWB gain based on scene LV
//variable name: i4LVThrL
//take preview AWB gain only when scene LV <= i4LVThrL
//default value: N/A
//data range: 0 ~ 180
//variable name: i4LVThrH
//take one-shot AWB gain only when scene LV >= i4LVThrH
//default value: N/A
//data range: 0 ~ 180
//It perform interpolation when i4LVThrH > LV > i4LVThrL
/******************************************************************************/
    	// One-shot AWB parameter
    	{
          0, // 0: flase 1:ture
    	    10, // LV 1.0
    	    50  // LV 5.0
        },
/******************************************************************************/
//Feature name:
//MWB window size in Xr Yr domain to control the white point window size
//variable name: i4WindowSizeX
//default value: 32
//variable name: i4WindowSizeY
//default value: 32
/******************************************************************************/        
      //MWB parameter
      {
          32, //i4WindowSizeX
          32  //i4WindowSizeY
      }         
    };
    return (rAWBParam);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
template <>
AWB_STAT_PARAM_T const&
getAWBStatParam<ESensorDev_Sub>()
{
    // AWB Statistics Parameter
    static AWB_STAT_PARAM_T rAWBStatParam =
    {
        // Number of AWB windows
	    120, // Number of horizontal AWB windows
	    90, // Number of vertical AWB windows
        // Thresholds
    	  1, // Low threshold of R
    	  1, // Low threshold of G
    	  1, // Low threshold of B
    	254, // High threshold of R
    	254, // High threshold of G
    	254, // High threshold of B

        // Pre-gain maximum limit clipping
       	0xFFF, // Maximum limit clipping for R color
       	0xFFF, // Maximum limit clipping for G color
       	0xFFF, // Maximum limit clipping for B color

        // AWB error threshold
       	20, // Programmable threshold for the allowed total over-exposed and under-exposed pixels in one main stat window

        // AWB error count shift bits
        0, // Programmable error count shift bits: 0 ~ 7; note: AWB statistics provide 4-bits error count output only

        // AWB error pixel ratio
       	16, // Programmable error pixel count by AWB window size (base : 256)

        // AWB motion error pixel ratio
        16 // Programmable motion error pixel count by AWB window size (base : 256)
    };

    return (rAWBStatParam);
}
