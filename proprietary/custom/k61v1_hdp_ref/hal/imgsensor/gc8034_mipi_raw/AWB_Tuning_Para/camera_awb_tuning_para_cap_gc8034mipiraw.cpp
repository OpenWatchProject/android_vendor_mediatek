/* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein
* is confidential and proprietary to MediaTek Inc. and/or its licensors.
* Without the prior written permission of MediaTek inc. and/or its licensors,
* any reproduction, modification, use or disclosure of MediaTek Software,
* and information contained herein, in whole or in part, shall be strictly prohibited.
*/
/* MediaTek Inc. (C) 2016. All rights reserved.
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
#include "camera_custom_nvram.h"

#include "camera_awb_tuning_para_gc8034mipiraw.h"

#define AWB_PARAM_TUNING_NUM	5341001

template <>
AWB_NVRAM_T const&
getAWBNVRAM_gc8034mipiraw<CAM_SCENARIO_CAPTURE>() // CAPTURE
{
	static AWB_NVRAM_T  g_rAWBNVRAM =
    AWB_NVRAM_START
    {
        {
                // AWB calibration data
                {
                    // rUnitGain (unit gain: 1.0 = 512)
                    {
                            0,    // i4R
                            0,    // i4G
                            0    // i4B
                    },
                    // rGoldenGain (golden sample gain: 1.0 = 512)
                    {
                            0,    // i4R
                            0,    // i4G
                            0    // i4B
                    },
                    // rUnitGain TL84 (unit gain: 1.0 = 512)
                    {
                            0,    // i4R
                            0,    // i4G
                            0     // i4B
                    },
                    // rGoldenGain TL84 (golden sample gain: 1.0 = 512)
                    {
                            0,    // i4R
                            0,    // i4G
                            0     // i4B
                    },
                     // rUnitGain Alight (unit gain: 1.0 = 512)
                    {
                            0,    // i4R
                            0,    // i4G
                            0     // i4B
                    },
                    // rGoldenGain Alight (golden sample gain: 1.0 = 512)
                    {
                            0,    // i4R
                            0,    // i4G
                            0     // i4B
                    },
                    // rTuningUnitGain (Tuning sample unit gain: 1.0 = 512)
                    {
                            0,    // i4R
                            0,    // i4G
                            0    // i4B
                    },
                    // rD65Gain (D65 WB gain: 1.0 = 512)
                    {
                            1097,    // D65Gain_R
                            512,    // D65Gain_G
                            655    // D65Gain_B
                    }
			},
			// rOriginalXY
			{
				// rStrobe
				{
					0,  // i4X
					0  // i4Y
				},
				// rHorizon
				{
					-376,  // i4X
					-394  // i4Y
				},
				// rA
				{
					-236,  // i4X
					-397  // i4Y
				},
				// rTL84
				{
					-66,  // i4X
					-426  // i4Y
				},
				// rCWF
				{
					-50,  // i4X
					-452  // i4Y
				},
				// rDNP
				{
					47,  // i4X
					-407  // i4Y
				},
				// rD65
				{
					190,  // i4X
					-372  // i4Y
				},
				// rDF
				{
					0,  // i4X
					0  // i4Y
				}
			},
			// rRotatedXY
			{
				// rStrobe
				{
					0,  // i4X
					0  // i4Y
				},
				// rHorizon
				{
					-402,  // i4X
					-366  // i4Y
				},
				// rA
				{
					-263,  // i4X
					-379  // i4Y
				},
				// rTL84
				{
					-96,  // i4X
					-420  // i4Y
				},
				// rCWF
				{
					-82,  // i4X
					-447  // i4Y
				},
				// rDNP
				{
					18,  // i4X
					-409  // i4Y
				},
				// rD65
				{
					163,  // i4X
					-384  // i4Y
				},
				// rDF
				{
					146,  // i4X
					-439  // i4Y
				}
			},
			// rLightAWBGain
			{
				// rStrobe
				{
					512,  // i4R
					512,  // i4G
					512  // i4B
				},
				// rHorizon
				{
					524,  // i4R
					512,  // i4G
					1453  // i4B
				},
				// rA
				{
					637,  // i4R
					512,  // i4G
					1207  // i4B
				},
				// rTL84
				{
					834,  // i4R
					512,  // i4G
					996  // i4B
				},
				// rCWF
				{
					882,  // i4R
					512,  // i4G
					1011  // i4B
				},
				// rDNP
				{
					947,  // i4R
					512,  // i4G
					833  // i4B
				},
				// rD65
				{
					1097,  // i4R
					512,  // i4G
					655  // i4B
				},
				// rDF
				{
					512,  // i4R
					512,  // i4G
					512  // i4B
				}
			},
			// rRotationMatrix
			{
				4,  // i4RotationAngle
				255,  // i4Cos
				18  // i4Sin
			},
			// rDaylightLocus
			{
				-145,  // i4SlopeNumerator
				128  // i4SlopeDenominator
			},
			// rPredictorGain
			{
				101,  // i4PrefRatio100
				// rSpatial_DaylightLocus_L
				{
					1067,  // i4R
					530,  // i4G
					675  // i4B
				},
				// rSpatial_DaylightLocus_H
				{
					831,  // i4R
					512,  // i4G
					899  // i4B
				},
				// rTemporal_General
				{
					807,  // i4R
					512,  // i4G
					916  // i4B
				},
				// rAWBGain_LSC
				{
					890,  // i4R
					512,  // i4G
					833  // i4B
				}
			},
			// rAWBLightArea
			{
				// rStrobe
				{
					0,  // i4RightBound
					0,  // i4LeftBound
					0,  // i4UpperBound
					0  // i4LowerBound
				},
				// rTungsten
				{
					0,  // i4RightBound
					0,  // i4LeftBound
					0,  // i4UpperBound
					0  // i4LowerBound
				},
				// rWarmFluorescent
				{
					-163,  // i4RightBound
					-802,  // i4LeftBound
					-200,  // i4UpperBound
					-537  // i4LowerBound
				},
				// rFluorescent
				{
					-19,  // i4RightBound
					-163,  // i4LeftBound
					-200,  // i4UpperBound
					-434  // i4LowerBound
				},
				// rCWF
				{
					-19,  // i4RightBound
					-163,  // i4LeftBound
					-434,  // i4UpperBound
					-537  // i4LowerBound
				},
				// rDaylight
				{
					300,  // i4RightBound
					-19,  // i4LeftBound
					-200,  // i4UpperBound
					-434  // i4LowerBound
				},
				// rShade
				{
					523,  // i4RightBound
					300,  // i4LeftBound
					-200,  // i4UpperBound
					-409  // i4LowerBound
				},
				// rDaylightFluorescent
				{
					300,  // i4RightBound
					-19,  // i4LeftBound
					-434,  // i4UpperBound
					-537  // i4LowerBound
				}
			},
			// rPWBLightArea
			{
				// rReferenceArea
				{
					523,  // i4RightBound
					-802,  // i4LeftBound
					0,  // i4UpperBound
					-537  // i4LowerBound
				},
				// rDaylight
				{
					325,  // i4RightBound
					-19,  // i4LeftBound
					-200,  // i4UpperBound
					-434  // i4LowerBound
				},
				// rCloudyDaylight
				{
					425,  // i4RightBound
					250,  // i4LeftBound
					-200,  // i4UpperBound
					-434  // i4LowerBound
				},
				// rShade
				{
					525,  // i4RightBound
					250,  // i4LeftBound
					-200,  // i4UpperBound
					-434  // i4LowerBound
				},
				// rTwilight
				{
					-19,  // i4RightBound
					-179,  // i4LeftBound
					-200,  // i4UpperBound
					-434  // i4LowerBound
				},
				// rFluorescent
				{
					213,  // i4RightBound
					-196,  // i4LeftBound
					-334,  // i4UpperBound
					-497  // i4LowerBound
				},
				// rWarmFluorescent
				{
					-163,  // i4RightBound
					-363,  // i4LeftBound
					-334,  // i4UpperBound
					-497  // i4LowerBound
				},
				// rIncandescent
				{
					-163,  // i4RightBound
					-363,  // i4LeftBound
					-200,  // i4UpperBound
					-434  // i4LowerBound
				},
				// rGrayWorld
				{
					5000,  // i4RightBound
					-5000,  // i4LeftBound
					5000,  // i4UpperBound
					-5000  // i4LowerBound
				}
			},
			// rPWBDefaultGain
			{
				// rDaylight
				{
					985,  // i4R
					512,  // i4G
					608  // i4B
				},
				// rCloudyDaylight
				{
					1238,  // i4R
					512,  // i4G
					468  // i4B
				},
				// rShade
				{
					1319,  // i4R
					512,  // i4G
					435  // i4B
				},
				// rTwilight
				{
					715,  // i4R
					512,  // i4G
					880  // i4B
				},
				// rFluorescent
				{
					944,  // i4R
					512,  // i4G
					853  // i4B
				},
				// rWarmFluorescent
				{
					671,  // i4R
					512,  // i4G
					1263  // i4B
				},
				// rIncandescent
				{
					582,  // i4R
					512,  // i4G
					1116  // i4B
				},
				// rGrayWorld
				{
					512,  // i4R
					512,  // i4G
					512  // i4B
				}
			},
			// rPreferenceColor
			{
				// rTungsten
				{
					0,  // i4SliderValue
					0  // i4OffsetThr
				},
				// rWarmFluorescent
				{
					60,  // i4SliderValue
					7500  // i4OffsetThr
				},
				// rShade
				{
					50,  // i4SliderValue
					210  // i4OffsetThr
				},
				// rSunsetArea
				{
					47,  // i4BoundXrThr
					-409  // i4BoundYrThr
				},
				// rShadeFArea
				{
					-163,  // i4BoundXrThr
					-424  // i4BoundYrThr
				},
				// rShadeFVerTex
				{
					-91,  // i4BoundXrThr
					-429  // i4BoundYrThr
				},
				// rShadeCWFArea
				{
					-163,  // i4BoundXrThr
					-451  // i4BoundYrThr
				},
				// rShadeCWFVerTex
				{
					-73,  // i4BoundXrThr
					-477  // i4BoundYrThr
				}
                },
                // CCT estimation
                {
                  // CCT
                  {
                    2300, // i4CCT[0]
                    2850, // i4CCT[1]
                    3750, // i4CCT[2]
                    5100, // i4CCT[3]
                    6500  // i4CCT[4]
                  },
                  // Rotated X coordinate
                  {
                    -565,    // i4RotatedXCoordinate[0]
                    -426,    // i4RotatedXCoordinate[1]
                    -359,    // i4RotatedXCoordinate[2]
                    -145,    // i4RotatedXCoordinate[3]
                    0        // i4RotatedXCoordinate[4]
                  }
                }
            },

            // Algorithm Tuning Paramter
            {
                // AWB Backup Enable
                0,

                // Daylight locus offset LUTs for tungsten
                {
                    21, // i4Size: LUT dimension
                	{0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000}, // i4LUTIn
                	{0, 700, 1000, 1333, 1667, 2313, 2625, 2938, 3250, 3563, 3875, 4188, 4500, 4813, 5125, 5438, 5750, 6063, 6375, 6688, 7000} // i4LUTOut
                },

                // Daylight locus offset LUTs for WF
                {
                    21, // i4Size: LUT dimension
                	{0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000}, // i4LUTIn
                	{0, 250, 500, 1250, 1500, 1767, 1950, 2111, 2333, 2556, 2778, 3000, 3222, 3444, 3667, 3889, 4111, 4333, 4556, 4778, 5000} // i4LUTOut
                },

                // Daylight locus offset LUTs for shade
                {
                    21, // i4Size: LUT dimension
                	{0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000}, // i4LUTIn
                	{0, 300, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000} // i4LUTOut
                },
                // Preference gain for each light source
                {
                    //        LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
                    //        LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18
                    {// STROBE
						{512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
						{512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                    },
                    {// TUNGSTEN
						{512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
						{512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                    },
                    {// WARM F
						{528, 512, 502}, {526, 512, 504}, {524, 512, 506}, {524, 512, 508}, {524, 512, 510}, {524, 512, 512}, {524, 512, 510}, {524, 512, 508}, {523, 512, 506}, {522, 512, 508},
						{519, 512, 510}, {517, 512, 512}, {514, 512, 512}, {513, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                    },
                    {// F - TL84
						{503, 512, 528}, {503, 512, 528}, {503, 512, 528}, {503, 512, 528}, {508, 512, 526}, {508, 512, 526}, {508, 512, 526}, {508, 512, 526}, {508, 512, 526}, {503, 512, 526},
						{508, 512, 528}, {505, 512, 530}, {502, 512, 532}, {502, 512, 532}, {502, 512, 532}, {502, 512, 532}, {502, 512, 532}, {502, 512, 532}, {502, 512, 532}
                    },
                    {// CWF
						{503, 512, 528}, {503, 512, 528}, {503, 512, 528}, {503, 512, 528}, {508, 512, 526}, {508, 512, 526}, {508, 512, 526}, {508, 512, 526}, {508, 512, 526}, {503, 512, 526},
						{508, 512, 528}, {505, 512, 530}, {502, 512, 532}, {502, 512, 532}, {502, 512, 532}, {502, 512, 532}, {502, 512, 532}, {502, 512, 532}, {502, 512, 532}
                    },
                    {// DAYLIGHT
						{525, 512, 501}, {525, 512, 501}, {525, 512, 503}, {526, 512, 503}, {526, 512, 502}, {526, 512, 502}, {526, 512, 502}, {526, 512, 504}, {526, 512, 504}, {524, 512, 504},
						{524, 512, 507}, {520, 512, 509}, {520, 512, 512}, {518, 512, 512}, {515, 512, 512}, {510, 512, 512}, {508, 512, 512}, {508, 512, 512}, {508, 512, 512}
                    },
                    {// SHADE
						{516, 512, 506}, {516, 512, 506}, {513, 512, 508}, {512, 512, 510}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
						{512, 512, 510}, {512, 512, 510}, {512, 512, 510}, {512, 512, 510}, {512, 512, 510}, {512, 512, 510}, {512, 512, 510}, {512, 512, 510}, {512, 512, 510}
                    },
                    {// DAYLIGHT F
						{512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {515, 512, 510}, {517, 512, 509}, {517, 512, 509}, {515, 512, 510}, {510, 512, 506}, {512, 512, 512},
						{512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                    }
                    //        LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
                    //        LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18

                },
                // Parent block weight parameter
                {
                    1,      // bEnable
                    6       // i4ScalingFactor: [6] 1~12, [7] 1~6, [8] 1~3, [9] 1~2, [>=10]: 1
                },
                // AWB LV threshold for predictor
                {
                    115,    // i4InitLVThr_L
                    155,    // i4InitLVThr_H
                    100      // i4EnqueueLVThr
                },
                // AWB number threshold for temporal predictor
                {
                    65,     // i4Neutral_ParentBlk_Thr
                    //LV0   1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18
                    { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  50,  25,   2,   2,   2,   2,   2,   2,   2}  // (%) i4CWFDF_LUTThr
                },
                // AWB light neutral noise reduction for outdoor
                {
                    //LV0  1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18
                  // Non neutral
                    { 3,   3,   3,   3,   3,   3,   3,   3,    3,   3,   5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
                  // Flurescent
                    { 0,   0,   0,   0,   0,   3,   5,   5,    5,   5,   5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
                  // CWF
                    { 0,   0,   0,   0,   0,   3,   5,   5,    5,   5,   5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
                  // Daylight
                    { 0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   2,   2,   2,   2,   2,   2,   2,   2},  // (%)
                  // DF
                    { 0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
                },
                // AWB feature detection
                {
                    // Sunset Prop
                    {
                        1,          // i4Enable
                        120,        // i4LVThr_L
                        130,        // i4LVThr_H
                        10,         // i4SunsetCountThr
                        0,          // i4SunsetCountRatio_L
                        171         // i4SunsetCountRatio_H
                    },

                    // Shade F Detection
                    {
                        1,          // i4Enable
                        50,         // i4LVThr_L
                        90,         // i4LVThr_H
                        128         // i4DaylightProb
                    },

                    // Shade CWF Detection
                    {
                        1,          // i4Enable
                        50,         // i4LVThr_L
                        90,         // i4LVThr_H
                        192         // i4DaylightProb
                    }

                },

                // AWB non-neutral probability for spatial and temporal weighting look-up table (Max: 100; Min: 0)
                {
                    //LV0   1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18
                    {   0,  33,  66, 100, 100, 100, 100, 100, 100, 100, 100,  70,  30,  20,  10,   0,   0,   0,   0}
                },

                // AWB daylight locus probability look-up table (Max: 100; Min: 0)
                {
                    //LV0  1    2    3    4    5    6    7    8    9   10    11   12   13   14   15   16   17   18
                    {100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  50,  25,   0,   0,   0,   0}, // Strobe
                    {100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  75,  50,  25,   0,   0,   0}, // Tungsten
                    { 85,  85,  85,  85,  80,  75,  75,  80,  80,  80,  70,  60,  50,  40,  30,  25,   0,   0,   0}, // Warm fluorescent
				  //{ 95,  90,  90,  85,  80,  80,  80,  80,  80,  80,  70,  60,  50,  40,  30,  25,   0,   0,   0},
                    { 90,  90,  90,  85,  85,  85,  80,  80,  80,  75,  75,  75,  50,  25,  25,  25,   0,   0,   0}, // Fluorescent
                    { 90,  90,  90,  80,  60,  30,  30,  60,  80,  80,  80,  55,  30,  30,  30,  30,   0,   0,   0}, // CWF
                    {100, 100, 100,  90,  90,  90,  90,  90,  90,  80,  80,  80,  80,  80,  75,  50,  40,  30,  20}, // Daylight
                    {100, 100, 100, 100, 100,  90,  80,  90,  80,  80,  80,  80,  75,  50,  25,   0,   0,   0,   0}, // Shade
                    { 90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  80,  55,  30,  30,  30,  30,   0,   0,   0} // Daylight fluorescent
                },
                // AWB tuning information
                {
                     6761,       // project code
                     8034,       // model
                     20190212, // i4Date
                     0, // i4Res0
                     0, // i4Res1
                     0, // i4Res2
                     0, // i4Res3
                     0, // i4Res4
                },
            }
};
    return g_rAWBNVRAM;
};
