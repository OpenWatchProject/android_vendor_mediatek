#include "camera_custom_nvram.h"
#include "camera_awb_tuning_para_gc8c34mipiraw.h"

#define NVRAM_TUNING_PARAM_NUM  6511001

template <>
AWB_NVRAM_T const&
getAWBNVRAM_gc8c34mipiraw<CAM_SCENARIO_CAPTURE>() // CAPTURE
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
                            1185,    // D65Gain_R
                            512,    // D65Gain_G
                            737    // D65Gain_B
                    }
			},
			// rOriginalXY
			{
				// rStrobe
				{
                    175,    // i4X
                    -444    // i4Y
				},
				// rHorizon
				{
                    -419,    // OriX_Hor
                    -477    // OriY_Hor
				},
				// rA
				{
                    -274,    // OriX_A
                    -476    // OriY_A
				},
				// rTL84
				{
                    -57,    // OriX_TL84
                    -480    // OriY_TL84
				},
				// rCWF
				{
                    -51,    // OriX_CWF
                    -538    // OriY_CWF
				},
				// rDNP
				{
                    45,    // OriX_DNP
                    -471    // OriY_DNP
				},
				// rD65
				{
                    175,    // OriX_D65
                    -444    // OriY_D65
				},
				// rDF
				{
                    175,    // OriX_DF
                    -444    // OriY_DF
				}
			},
			// rRotatedXY
			{
				// rStrobe
				{
                    143,    // i4X
                    -455    // i4Y
				},
				// rHorizon
				{
                    -451,    // RotX_Hor
                    -446    // RotY_Hor
				},
				// rA
				{
                    -306,    // RotX_A
                    -455    // RotY_A
				},
				// rTL84
				{
                    -91,    // RotX_TL84
                    -474    // RotY_TL84
				},
				// rCWF
				{
                    -89,    // RotX_CWF
                    -532    // RotY_CWF
				},
				// rDNP
				{
                    12,    // RotX_DNP
                    -472    // RotY_DNP
				},
				// rD65
				{
                    143,    // RotX_D65
                    -455    // RotY_D65
				},
				// rDF
				{
                    143,    // RotX_DF
                    -455    // RotY_DF
				}
			},
			// rLightAWBGain
			{
				// rStrobe
				{
                    1185,    // i4R
                    512,    // i4G
                    737    // i4B
				},
				// rHorizon
				{
                    554,    // AWBGAIN_HOR_R
                    512,    // AWBGAIN_HOR_G
                    1722    // AWBGAIN_HOR_B
				},
				// rA
				{
                    674,    // AWBGAIN_A_R
                    512,    // AWBGAIN_A_G
                    1414    // AWBGAIN_A_B
				},
				// rTL84
				{
                    908,    // AWBGAIN_TL84_R
                    512,    // AWBGAIN_TL84_G
                    1059    // AWBGAIN_TL84_B
				},
				// rCWF
				{
                    990,    // AWBGAIN_CWF_R
                    512,    // AWBGAIN_CWF_G
                    1138    // AWBGAIN_CWF_B
				},
				// rDNP
				{
                    1030,    // AWBGAIN_DNP_R
                    512,    // AWBGAIN_DNP_G
                    910    // AWBGAIN_DNP_B
				},
				// rD65
				{
                    1185,    // AWBGAIN_D65_R
                    512,    // AWBGAIN_D65_G
                    737    // AWBGAIN_D65_B
				},
				// rDF
				{
                    1185,    // AWBGAIN_DF_R
                    512,    // AWBGAIN_DF_G
                    737    // AWBGAIN_DF_B
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
                -147,    // i4SlopeNumerator
                128    // i4SlopeDenominator
			},
			// rPredictorGain
			{
				101,  // i4PrefRatio100
				// rSpatial_DaylightLocus_L
				{
                    1155,    // i4R
                    530,    // i4G
                    757    // i4B
				},
				// rSpatial_DaylightLocus_H
				{
                    905,    // i4R
                    512,    // i4G
                    1003    // i4B
				},
				// rTemporal_General
				{
                    791,    // i4R
                    512,    // i4G
                    434    // i4B
				},
				// rAWBGain_LSC
				{
                    981,        // i4R
                    512,        // i4G
                    916        // i4B
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
					-177,  // i4RightBound
					-851,  // i4LeftBound
					-200,  // i4UpperBound
					-622  // i4LowerBound
				},
				// rFluorescent
				{
					-21,  // i4RightBound
					-177,  // i4LeftBound
					-200,  // i4UpperBound
					-503  // i4LowerBound
				},
				// rCWF
				{
					-21,  // i4RightBound
					-177,  // i4LeftBound
					-503,  // i4UpperBound
					-622  // i4LowerBound
				},
				// rDaylight
				{
					300,  // i4RightBound
					-21,  // i4LeftBound
					-200,  // i4UpperBound
					-622  // i4LowerBound
				},
				// rShade
				{
					503,  // i4RightBound
					300,  // i4LeftBound
					-200,  // i4UpperBound
					-486  // i4LowerBound
				},
				// rDaylightFluorescent
				{
					173,  // i4RightBound
					-21,  // i4LeftBound
					-587,  // i4UpperBound
					-587  // i4LowerBound
				}
			},
			// rPWBLightArea
			{
				// rReferenceArea
				{
					503,  // i4RightBound
					-851,  // i4LeftBound
					0,  // i4UpperBound
					-622  // i4LowerBound
				},
				// rDaylight
				{
					325,  // i4RightBound
					-21,  // i4LeftBound
					-200,  // i4UpperBound
					-622  // i4LowerBound
				},
				// rCloudyDaylight
				{
					425,  // i4RightBound
					250,  // i4LeftBound
					-200,  // i4UpperBound
					-622  // i4LowerBound
				},
				// rShade
				{
					525,  // i4RightBound
					250,  // i4LeftBound
					-200,  // i4UpperBound
					-622  // i4LowerBound
				},
				// rTwilight
				{
					-21,  // i4RightBound
					-181,  // i4LeftBound
					-200,  // i4UpperBound
					-622  // i4LowerBound
				},
				// rFluorescent
				{
					193,  // i4RightBound
					-191,  // i4LeftBound
					-405,  // i4UpperBound
					-582  // i4LowerBound
				},
				// rWarmFluorescent
				{
					-206,  // i4RightBound
					-406,  // i4LeftBound
					-405,  // i4UpperBound
					-582  // i4LowerBound
				},
				// rIncandescent
				{
					-206,  // i4RightBound
					-406,  // i4LeftBound
					-200,  // i4UpperBound
					-622  // i4LowerBound
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
					1124,  // i4R
					512,  // i4G
					689  // i4B
				},
				// rCloudyDaylight
				{
					1419,  // i4R
					512,  // i4G
					527  // i4B
				},
				// rShade
				{
					1511,  // i4R
					512,  // i4G
					490  // i4B
				},
				// rTwilight
				{
					818,  // i4R
					512,  // i4G
					993  // i4B
				},
				// rFluorescent
				{
					1047,  // i4R
					512,  // i4G
					951  // i4B
				},
				// rWarmFluorescent
				{
					712,  // i4R
					512,  // i4G
					1483  // i4B
				},
				// rIncandescent
				{
					632,  // i4R
					512,  // i4G
					1337  // i4B
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
					40,  // i4SliderValue
					0  // i4OffsetThr
				},
				// rWarmFluorescent
				{
					60,  // i4SliderValue
					4850  // i4OffsetThr
				},
				// rShade
				{
					50,  // i4SliderValue
					210  // i4OffsetThr
				},
				// rSunsetArea
				{
					38,  // i4BoundXrThr
					-472  // i4BoundYrThr
				},
				// rShadeFArea
				{
					-177,  // i4BoundXrThr
					-478  // i4BoundYrThr
				},
				// rShadeFVerTex
				{
					-99,  // i4BoundXrThr
					-491  // i4BoundYrThr
				},
				// rShadeCWFArea
				{
					-177,  // i4BoundXrThr
					-536  // i4BoundYrThr
				},
				// rShadeCWFVerTex
				{
					-82,  // i4BoundXrThr
					-562  // i4BoundYrThr
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
                    -594,    // i4RotatedXCoordinate[0]
                    -449,    // i4RotatedXCoordinate[1]
                    -234,    // i4RotatedXCoordinate[2]
                    -131,    // i4RotatedXCoordinate[3]
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
						{517, 512, 495}, {517, 512, 495}, {517, 512, 500}, {513, 512, 500}, {513, 512, 500}, {513, 512, 506}, {518, 512, 506}, {515, 512, 506}, {515, 512, 510}, {512, 512, 512},
						{515, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                    },
                    {// F - TL84
						{535, 512, 505}, {532, 512, 505}, {527, 512, 505}, {527, 512, 505}, {527, 512, 505}, {527, 512, 505}, {525, 512, 505}, {525, 512, 505}, {525, 512, 508}, {518, 512, 508},
						{518, 512, 508}, {518, 512, 510}, {515, 512, 510}, {512, 512, 510}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                    },
                    {// CWF
						{512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
						{512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                    },
                    {// DAYLIGHT
						{515, 512, 510}, {515, 512, 510}, {515, 512, 512}, {518, 512, 512}, {520, 512, 512}, {518, 512, 512}, {520, 512, 512}, {521, 512, 512}, {523, 512, 515}, {521, 512, 513},
						{515, 512, 512}, {510, 512, 512}, {508, 512, 512}, {505, 512, 512}, {505, 512, 512}, {505, 512, 512}, {505, 512, 512}, {505, 512, 512}, {505, 512, 512}
                    },
                    {// SHADE
						{512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
						{512, 512, 510}, {512, 512, 510}, {512, 512, 510}, {512, 512, 510}, {512, 512, 510}, {512, 512, 510}, {512, 512, 510}, {512, 512, 510}, {512, 512, 510}
                    },
                    {// DAYLIGHT F
						{512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {510, 512, 506}, {512, 512, 512},
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
                    { 90,  90,  90,  85,  80,  80,  80,  80,  80,  80,  70,  60,  50,  40,  30,  25,   0,   0,   0}, // Warm fluorescent
                    { 90,  90,  90,  85,  80,  80,  80,  80,  80,  80,  80,  75,  50,  25,  25,  25,   0,   0,   0}, // Fluorescent
                    { 90,  90,  90,  80,  60,  30,  30,  60,  80,  80,  80,  55,  30,  30,  30,  30,   0,   0,   0}, // CWF
                    {100, 100, 100, 100,  90,  90,  90,  90,  90,  80,  80,  80,  80,  80,  75,  50,  40,  30,  20}, // Daylight
                    {100, 100, 100, 100, 100, 100,  80,  90,  80,  80,  80,  80,  75,  50,  25,   0,   0,   0,   0}, // Shade
                    { 90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  80,  55,  30,  30,  30,  30,   0,   0,   0} // Daylight fluorescent
                },

                // AWB tuning information
                {
                    6761,       // project code
                    8134,       // model
                    20181107,   // date
                    0,          // reserved 0
                    1,          // reserved 1
                    2,          // reserved 2
                    3,          // reserved 3
                    4,          // reserved 4
        },
    }
};
    return g_rAWBNVRAM;
};
