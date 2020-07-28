#include "camera_custom_nvram.h"
#include "camera_awb_tuning_para_s5k5e8yxmipiraw.h"

#define NVRAM_TUNING_PARAM_NUM  6511001

template <>
AWB_NVRAM_T const&
getAWBNVRAM_s5k5e8yxmipiraw<CAM_SCENARIO_CUSTOM3>() // VT
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
                    1122,    // D65Gain_R
                    512,    // D65Gain_G
                    743    // D65Gain_B
                    }
                },
                // Original XY coordinate of AWB light source
                {
                    // Strobe
                        {
                                0,    // i4X
                                0    // i4Y
                        },
                        // Horizon
                        {
                    -447,    // OriX_Hor
                    -429    // OriY_Hor
                        },
                        // A
                        {
                    -312,    // OriX_A
                    -437    // OriY_A
                        },
                        // TL84
                        {
                    -88,    // OriX_TL84
                    -469    // OriY_TL84
                        },
                        // CWF
                        {
                    -90,    // OriX_CWF
                    -546    // OriY_CWF
                        },
                        // DNP
                        {
                    26,    // OriX_DNP
                    -488    // OriY_DNP
                        },
                        // D65
                        {
                    152,    // OriX_D65
                    -427    // OriY_D65
                        },
                        // DF
                        {
                    131,    // OriX_DF
                    -527    // OriY_DF
                        }
                },
                // Rotated XY coordinate of AWB light source
                {
                    // Strobe
                        {
                                0,    // i4X
                                0    // i4Y
                        },
                        // Horizon
                        {
                    -454,    // RotX_Hor
                    -422    // RotY_Hor
                        },
                        // A
                        {
                    -319,    // RotX_A
                    -432    // RotY_A
                        },
                        // TL84
                        {
                    -95,    // RotX_TL84
                    -468    // RotY_TL84
                        },
                        // CWF
                        {
                    -99,    // RotX_CWF
                    -545    // RotY_CWF
                        },
                        // DNP
                        {
                    18,    // RotX_DNP
                    -488    // RotY_DNP
                        },
                        // D65
                        {
                    145,    // RotX_D65
                    -429    // RotY_D65
                        },
                        // DF
                        {
                    123,    // RotX_DF
                    -529    // RotY_DF
                        }
                },
                // AWB gain of AWB light source
                {
                    // Strobe
                        {
                                512,    // i4R
                                512,    // i4G
                                512    // i4B
                        },
                        // Horizon
                        {
                    512,    // AWBGAIN_HOR_R
                    524,    // AWBGAIN_HOR_G
                    1715    // AWBGAIN_HOR_B
                        },
                        // A
                        {
                    606,    // AWBGAIN_A_R
                    512,    // AWBGAIN_A_G
                    1412    // AWBGAIN_A_B
                        },
                        // TL84
                        {
                    858,    // AWBGAIN_TL84_R
                    512,    // AWBGAIN_TL84_G
                    1089    // AWBGAIN_TL84_B
                        },
                        // CWF
                        {
                    949,    // AWBGAIN_CWF_R
                    512,    // AWBGAIN_CWF_G
                    1211    // AWBGAIN_CWF_B
                        },
                        // DNP
                        {
                    1027,    // AWBGAIN_DNP_R
                    512,    // AWBGAIN_DNP_G
                    956    // AWBGAIN_DNP_B
                        },
                        // D65
                        {
                    1122,    // AWBGAIN_D65_R
                    512,    // AWBGAIN_D65_G
                    743    // AWBGAIN_D65_B
                        },
                        // DF
                        {
                    1248,    // AWBGAIN_DF_R
                    512,    // AWBGAIN_DF_G
                    874    // AWBGAIN_DF_B
                        }
                },
                // Rotation matrix parameter
                {
                1,    // RotationAngle
                256,    // Cos
                4    // Sin
                },
                // Daylight locus parameter
                {
                -134,    // i4SlopeNumerator
                        128    // i4SlopeDenominator
                },
              // Predictor gain
                {
                101, // i4PrefRatio100
                        // DaylightLocus_L
                        {
                    1092,    // i4R
                    512,    // i4G
                    763    // i4B
                        },
                        // DaylightLocus_H
                        {
                    923,    // i4R
                            512, // i4G
                    907    // i4B
                        },
                        // Temporal General
                        {
                    1122,    // i4R
                            512, // i4G
                    743    // i4B
                        },
                    // AWB LSC Gain
                    {
                    923,        // i4R
                            512, // i4G
                    907        // i4B
                    }
                },
                // AWB light area
                {
                    // Strobe
                        {
      0,  // StrobeRightBound
      0,  // StrobeLeftBound
      0,  // StrobeUpperBound
      0 // StrobeLowerBound
                        },
                        // Tungsten
                        {
                    -185,    // TungRightBound
                    -854,    // TungLeftBound
                    -367,    // TungUpperBound
                    -450    // TungLowerBound
                        },
                        // Warm fluorescent
                        {
                    -185,    // WFluoRightBound
                    -854,    // WFluoLeftBound
                    -450,    // WFluoUpperBound
                    -635    // WFluoLowerBound
                        },
                        // Fluorescent
                        {
      -19,  // FluoRightBound
                    -185,    // FluoLeftBound
                    -369,    // FluoUpperBound
                    -507    // FluoLowerBound
                        },
                        // CWF
                        {
                -1,    // CWFRightBound
                -185,    // CWFLeftBound
                -507,    // CWFUpperBound
                -635    // CWFLowerBound
                        },
                        // Daylight
                        {
                    175,    // DayRightBound
      -19,  // DayLeftBound
                    -369,    // DayUpperBound
                    -507    // DayLowerBound
                        },
                        // Shade
                        {
                    505,    // ShadeRightBound
                    175,    // ShadeLeftBound
                    -369,    // ShadeUpperBound
                    -475    // ShadeLowerBound
                        },
                        // Daylight Fluorescent
                        {
                    175,    // DFRightBound
                    -1,    // DFLeftBound
                    -507,    // DFUpperBound
                    -600    // DFLowerBound
                        }
                },
                // PWB light area
                {
                    // Reference area
                        {
                    505,    // PRefRightBound
                    -854,    // PRefLeftBound
                    -342,    // PRefUpperBound
                    -635    // PRefLowerBound
                        },
                        // Daylight
                        {
                    200,    // PDayRightBound
      -19,  // PDayLeftBound
                    -369,    // PDayUpperBound
                    -507    // PDayLowerBound
                        },
                        // Cloudy daylight
                        {
                    300,    // PCloudyRightBound
                    125,    // PCloudyLeftBound
                    -369,    // PCloudyUpperBound
                    -507    // PCloudyLowerBound
                        },
                        // Shade
                        {
                    400,    // PShadeRightBound
                    125,    // PShadeLeftBound
                    -369,    // PShadeUpperBound
                    -507    // PShadeLowerBound
                        },
                        // Twilight
                        {
      -19,  // PTwiRightBound
      -179, // PTwiLeftBound
                    -369,    // PTwiUpperBound
                    -507    // PTwiLowerBound
                        },
                        // Fluorescent
                        {
                    195,    // PFluoRightBound
                    -199,    // PFluoLeftBound
                    -379,    // PFluoUpperBound
                    -595    // PFluoLowerBound
                        },
                        // Warm fluorescent
                        {
                    -219,    // PWFluoRightBound
                    -419,    // PWFluoLeftBound
                    -379,    // PWFluoUpperBound
                    -595    // PWFluoLowerBound
                        },
                        // Incandescent
                        {
                    -219,    // PIncaRightBound
                    -419,    // PIncaLeftBound
                    -369,    // PIncaUpperBound
                    -507    // PIncaLowerBound
                        },
                        // Gray World
                        {
      5000, // PGWRightBound
      -5000,  // PGWLeftBound
      5000, // PGWUpperBound
      -5000 // PGWLowerBound
                        }
                },
                // PWB default gain
                {
                    // Daylight
                        {
                    1055,    // PWB_Day_R
      512,  // PWB_Day_G
                    810    // PWB_Day_B
                        },
                        // Cloudy daylight
                        {
                    1241,    // PWB_Cloudy_R
      512,  // PWB_Cloudy_G
                    685    // PWB_Cloudy_B
                        },
                        // Shade
                        {
                    1326,    // PWB_Shade_R
      512,  // PWB_Shade_G
                    640    // PWB_Shade_B
                        },
                        // Twilight
                        {
                    819,    // PWB_Twi_R
      512,  // PWB_Twi_G
                    1051    // PWB_Twi_B
                        },
                        // Fluorescent
                        {
                    997,    // PWB_Fluo_R
      512,  // PWB_Fluo_G
                    982    // PWB_Fluo_B
                        },
                        // Warm fluorescent
                        {
                    654,    // PWB_WFluo_R
      512,  // PWB_WFluo_G
                    1519    // PWB_WFluo_B
                        },
                        // Incandescent
                        {
                    611,    // PWB_Inca_R
      512,  // PWB_Inca_G
                    1423    // PWB_Inca_B
                        },
                        // Gray World
                        {
      512,  // PWB_GW_R
      512,  // PWB_GW_G
      512 // PWB_GW_B
                        }
                },
                // AWB preference color
                {
                    // Tungsten
                        {
                    50,    // TUNG_SLIDER
                            5000    // TUNG_OFFS
                        },
                        // Warm fluorescent
                        {
                    50,    // WFluo_SLIDER
                            3000    // WFluo_OFFS
                        },
                        // Shade
                        {
      50, // Shade_SLIDER
      909 // Shade_OFFS
                        },
                    // Sunset Area
                    {
                    46,   // i4Sunset_BoundXr_Thr
                    -488    // i4Sunset_BoundYr_Thr
                    },
                    // Shade F Area
                    {

                    -185,   // i4BoundXrThr
                    -400    // i4BoundYrThr
                    },
                    // Shade F Vertex
                    {
                    -102,   // i4BoundXrThr
                    -490    // i4BoundYrThr
                    },
                    // Shade CWF Area
                    {
                    -185,   // i4BoundXrThr
                    -529    // i4BoundYrThr
                    },
                    // Shade CWF Vertex
                    {
                    -93,   // i4BoundXrThr
                    -575    // i4BoundYrThr
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
                -599,    // i4RotatedXCoordinate[0]
                -464,    // i4RotatedXCoordinate[1]
                -240,    // i4RotatedXCoordinate[2]
                -127,    // i4RotatedXCoordinate[3]
                      0       // i4RotatedXCoordinate[4]
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
                    {0, 350,  800, 1222, 1444, 1667, 1889, 2111, 2333, 2556, 2778, 3000, 3222, 3444, 3667, 3889, 4111, 4333, 4556, 4778,  5000} // i4LUTOut
                },

                // Daylight locus offset LUTs for WF
                {
                    21, // i4Size: LUT dimension
                    {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000}, // i4LUTIn
                {0, 350, 700, 1000, 1444, 1667, 1889, 2111, 2333, 2556, 2778, 3000, 3222, 3444, 3667, 3889, 4111, 4333, 4556, 4778, 5000} // i4LUTOut
                },

                // Daylight locus offset LUTs for shade
                {
                    21, // i4Size: LUT dimension
                    {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000}, // i4LUTIn
                {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000} // i4LUTOut
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
                        70,        // i4LVThr_H
                        128         // i4DaylightProb
                    },

                    // Shade CWF Detection
                    {
                        1,          // i4Enable
                        50,        // i4LVThr_L
                        70,        // i4LVThr_H
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
                    6735,       // project code
                    5588,       // model
                    20150624,   // date
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
