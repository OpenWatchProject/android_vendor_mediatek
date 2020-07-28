#include "camera_custom_nvram.h"
#include "camera_awb_tuning_para_gc8c34mipiraw.h"

#define NVRAM_TUNING_PARAM_NUM  6511001

template <>
AWB_NVRAM_T const&
getAWBNVRAM_gc8c34mipiraw<CAM_SCENARIO_CUSTOM3>() // VT
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
					1177,  // i4R
					512,  // i4G
					751  // i4B
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
					-429,  // i4X
					-479  // i4Y
				},
				// rA
				{
					-266,  // i4X
					-476  // i4Y
				},
				// rTL84
				{
					-80,  // i4X
					-495  // i4Y
				},
				// rCWF
				{
					-57,  // i4X
					-544  // i4Y
				},
				// rDNP
				{
					33,  // i4X
					-473  // i4Y
				},
				// rD65
				{
					166,  // i4X
					-449  // i4Y
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
					-461,  // i4X
					-447  // i4Y
				},
				// rA
				{
					-298,  // i4X
					-455  // i4Y
				},
				// rTL84
				{
					-114,  // i4X
					-487  // i4Y
				},
				// rCWF
				{
					-95,  // i4X
					-538  // i4Y
				},
				// rDNP
				{
					0,  // i4X
					-473  // i4Y
				},
				// rD65
				{
					134,  // i4X
					-459  // i4Y
				},
				// rDF
				{
					117,  // i4X
					-529  // i4Y
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
					547,  // i4R
					512,  // i4G
					1750  // i4B
				},
				// rA
				{
					680,  // i4R
					512,  // i4G
					1398  // i4B
				},
				// rTL84
				{
					899,  // i4R
					512,  // i4G
					1115  // i4B
				},
				// rCWF
				{
					989,  // i4R
					512,  // i4G
					1155  // i4B
				},
				// rDNP
				{
					1016,  // i4R
					512,  // i4G
					929  // i4B
				},
				// rD65
				{
					1177,  // i4R
					512,  // i4G
					751  // i4B
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
				-146,  // i4SlopeNumerator
				128  // i4SlopeDenominator
			},
			// rPredictorGain
			{
				101,  // i4PrefRatio100
				// rSpatial_DaylightLocus_L
				{
					1147,  // i4R
					530,  // i4G
					771  // i4B
				},
				// rSpatial_DaylightLocus_H
				{
					897,  // i4R
					512,  // i4G
					1027  // i4B
				},
				// rTemporal_General
				{
					1177,  // i4R
					512,  // i4G
					751  // i4B
				},
				// rAWBGain_LSC
				{
					972,  // i4R
					512,  // i4G
					937  // i4B
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
					-188,  // i4RightBound
					-861,  // i4LeftBound
					-255,  // i4UpperBound
					-628  // i4LowerBound
				},
				// rFluorescent
				{
					-37,  // i4RightBound
					-188,  // i4LeftBound
					-255,  // i4UpperBound
					-513  // i4LowerBound
				},
				// rCWF
				{
					-2,  // i4RightBound
					-188,  // i4LeftBound
					-513,  // i4UpperBound
					-628  // i4LowerBound
				},
				// rDaylight
				{
					300,  // i4RightBound
					-37,  // i4LeftBound
					-255,  // i4UpperBound
					-513  // i4LowerBound
				},
				// rShade
				{
					494,  // i4RightBound
					300,  // i4LeftBound
					-255,  // i4UpperBound
					-513  // i4LowerBound
				},
				// rDaylightFluorescent
				{
					164,  // i4RightBound
					-2,  // i4LeftBound
					-513,  // i4UpperBound
					-628  // i4LowerBound
				}
			},
			// rPWBLightArea
			{
				// rReferenceArea
				{
					494,  // i4RightBound
					-861,  // i4LeftBound
					0,  // i4UpperBound
					-628  // i4LowerBound
				},
				// rDaylight
				{
					325,  // i4RightBound
					-37,  // i4LeftBound
					-255,  // i4UpperBound
					-513  // i4LowerBound
				},
				// rCloudyDaylight
				{
					425,  // i4RightBound
					250,  // i4LeftBound
					-255,  // i4UpperBound
					-513  // i4LowerBound
				},
				// rShade
				{
					525,  // i4RightBound
					250,  // i4LeftBound
					-255,  // i4UpperBound
					-513  // i4LowerBound
				},
				// rTwilight
				{
					-37,  // i4RightBound
					-197,  // i4LeftBound
					-255,  // i4UpperBound
					-513  // i4LowerBound
				},
				// rFluorescent
				{
					184,  // i4RightBound
					-214,  // i4LeftBound
					-409,  // i4UpperBound
					-588  // i4LowerBound
				},
				// rWarmFluorescent
				{
					-198,  // i4RightBound
					-398,  // i4LeftBound
					-409,  // i4UpperBound
					-588  // i4LowerBound
				},
				// rIncandescent
				{
					-198,  // i4RightBound
					-398,  // i4LeftBound
					-255,  // i4UpperBound
					-513  // i4LowerBound
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
					1070,  // i4R
					512,  // i4G
					674  // i4B
				},
				// rCloudyDaylight
				{
					1364,  // i4R
					512,  // i4G
					509  // i4B
				},
				// rShade
				{
					1453,  // i4R
					512,  // i4G
					473  // i4B
				},
				// rTwilight
				{
					771,  // i4R
					512,  // i4G
					983  // i4B
				},
				// rFluorescent
				{
					1034,  // i4R
					512,  // i4G
					979  // i4B
				},
				// rWarmFluorescent
				{
					725,  // i4R
					512,  // i4G
					1475  // i4B
				},
				// rIncandescent
				{
					614,  // i4R
					512,  // i4G
					1277  // i4B
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
					40,  // i4SliderValue
					4597  // i4OffsetThr
				},
				// rShade
				{
					50,  // i4SliderValue
					410  // i4OffsetThr
				},
				// rSunsetArea
				{
					29,  // i4BoundXrThr
					-473  // i4BoundYrThr
				},
				// rShadeFArea
				{
					-188,  // i4BoundXrThr
					-491  // i4BoundYrThr
				},
				// rShadeFVerTex
				{
					-113,  // i4BoundXrThr
					-502  // i4BoundYrThr
				},
				// rShadeCWFArea
				{
					-188,  // i4BoundXrThr
					-542  // i4BoundYrThr
				},
				// rShadeCWFVerTex
				{
					-95,  // i4BoundXrThr
					-568  // i4BoundYrThr
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
                -595,    // i4RotatedXCoordinate[0]
                -432,    // i4RotatedXCoordinate[1]
                -248,    // i4RotatedXCoordinate[2]
                -134,    // i4RotatedXCoordinate[3]
                 0       // i4RotatedXCoordinate[4]
                  }
                }
            },

            // Algorithm Tuning Paramter
            {
                // AWB Backup Enable
                0,

			// rDaylightLocusOffsetLUTs_Tungsten
			{
				21,  // i4LUTSize
				// i4LUTIn
				{
					0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500,
					5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500,
					10000
				},
				// i4LUTOut
				{
					0, 350, 800, 1222, 1444, 1667, 1889, 2111, 2333, 2556,
					2778, 3000, 3222, 3444, 3667, 3889, 4111, 4333, 4556, 4778,
					5000
				}
			},
			// rDaylightLocusOffsetLUTs_WF
			{
				21,  // i4LUTSize
				// i4LUTIn
				{
					0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500,
					5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500,
					10000
				},
				// i4LUTOut
				{
					0, 250, 500, 1250, 1500, 1767, 1950, 2111, 2333, 2556,
					2778, 3000, 3222, 3444, 3667, 3889, 4111, 4333, 4556, 4778,
					5000
				}
			},
			// rDaylightLocusOffsetLUTs_Shade
			{
				21,  // i4LUTSize
				// i4LUTIn
				{
					0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500,
					5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500,
					10000
				},
				// i4LUTOut
				{
					0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500,
					5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500,
					10000
				}
			},
                // Preference gain for each light source
                {
                    //        LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
                    {
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                    //        LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                  }, // STROBE
                  {
                    {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                  }, // TUNGSTEN
                  {
                    {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                    {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                  }, // WARM F
                  {
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}
                    }, // F
                    {
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                    }, // CWF
                    {
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}
                    }, // DAYLIGHT
                    {
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                    }, // SHADE
                    {
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                    } // DAYLIGHT F
                },
                // Parent block weight parameter
                {
                    1,      // bEnable
                    6           // i4ScalingFactor: [6] 1~12, [7] 1~6, [8] 1~3, [9] 1~2, [>=10]: 1
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
                    {   0,   0,   0,   0,   0,   3,   5,   5,   5,   5,   5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
                  // CWF
                    {   0,   0,   0,   0,   0,   3,   5,   5,   5,   5,   5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
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
                        50,        // i4LVThr_L
					90,  // i4LVThr_H
                        128         // i4DaylightProb
                    },

                    // Shade CWF Detection
                    {
                        1,          // i4Enable
                        50,        // i4LVThr_L
					90,  // i4LVThr_H
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
                    //LV0  1    2    3    4    5    6    7    8    9     10    11   12   13   14   15  16   17   18
                    {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 100,  50,  25,   0,  0,   0,   0}, // Strobe
                    { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  75,  50,  25,   0,   0,   0}, // Tungsten
                    {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,   75,  50,  25,  25,  25,  0,   0,   0}, // Warm fluorescent
                    { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  95,  75,  50,  25,  25,  25,   0,   0,   0}, // Fluorescent
                    { 90,  90,  90,  90,  90,  90,  90,  90,  90,  90,   80,   55,  30,  30,  30,  30,  0,   0,   0}, // CWF
                    {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 100, 100,  75,  50, 40,  30,  20}, // Daylight
                    {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100,  75,  50,  25,   0,  0,   0,   0}, // Shade
                    { 90,  90,  90,  90,  90,  90,  90,  90,  90,  90,   80,   55,  30,  30,  30,  30,  0,   0,   0}  // Daylight fluorescent
                },

                // AWB tuning information
                {
                    6761,       // project code
                    8134,       // model
                    20180930,   // date
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
