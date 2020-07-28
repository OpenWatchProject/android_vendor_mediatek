#include "camera_custom_nvram.h"
#include "camera_awb_tuning_para_ov8856mipiraw.h"

#define NVRAM_TUNING_PARAM_NUM  6511001

template <>
AWB_NVRAM_T const&
getAWBNVRAM_ov8856mipiraw<CAM_SCENARIO_CUSTOM2>() // AUTO HDR
{
    static AWB_NVRAM_T  g_rAWBNVRAM =
    AWB_NVRAM_START
    {
   {
  // AWB calibration data
  {
    // rUnitGain (unit gain: 1.0 = 512)
    {
      0,  // i4R
      0,  // i4G
      0 // i4B
    },
    // rGoldenGain (golden sample gain: 1.0 = 512)
    {
      0,  // i4R
      0,  // i4G
      0 // i4B
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
      0,  // i4R
      0,  // i4G
      0 // i4B
    },
    // rD65Gain (D65 WB gain: 1.0 = 512)
    {
                            989,    // D65Gain_R
                            512,    // D65Gain_G
                            668    // D65Gain_B
    }
  },
  // Original XY coordinate of AWB light source
  {
    // Strobe
    {
      0,  // i4X
      0 // i4Y
    },
    // Horizon
    {
                        -364,    // OriX_Hor
                        -325    // OriY_Hor
    },
    // A
    {
                        -253,    // OriX_A
                        -351    // OriY_A
    },
    // TL84
    {
                        -84,    // OriX_TL84
                        -383    // OriY_TL84
    },
    // CWF
    {
                        -70,    // OriX_CWF
                        -430    // OriY_CWF
    },
    // DNP
    {
                        47,    // OriX_DNP
                        -430    // OriY_DNP
    },
    // D65
    {
                        145,    // OriX_D65
                        -342    // OriY_D65
    },
    // DF
    {
                        105,    // OriX_DF
                        -429    // OriY_DF
    }
  },
  // Rotated XY coordinate of AWB light source
  {
    // Strobe
    {
      0,     // i4X
      0     // i4Y
    },
    // Horizon
    {
                        -369,    // RotX_Hor
                        -319    // RotY_Hor
    },
    // A
    {
                        -258,    // RotX_A
                        -347    // RotY_A
    },
    // TL84
    {
                        -90,    // RotX_TL84
                        -382    // RotY_TL84
    },
    // CWF
    {
                        -77,    // RotX_CWF
                        -429    // RotY_CWF
    },
    // DNP
    {
                        40,    // RotX_DNP
                        -431    // RotY_DNP
    },
    // D65
    {
                        140,    // RotX_D65
                        -344    // RotY_D65
    },
    // DF
    {
                        98,    // RotX_DF
                        -431    // RotY_DF
    }
  },
  // AWB gain of AWB light source
  {
    // Strobe
    {
      512,  // i4R
      512,  // i4G
      512 // i4B
    },
    // Horizon
    {
                    512,    // AWBGAIN_HOR_R
                    539,    // AWBGAIN_HOR_G
                    1370    // AWBGAIN_HOR_B
    },
    // A
    {
                    584,    // AWBGAIN_A_R
                    512,    // AWBGAIN_A_G
                    1160    // AWBGAIN_A_B
    },
    // TL84
    {
                    767,    // AWBGAIN_TL84_R
                    512,    // AWBGAIN_TL84_G
                    963    // AWBGAIN_TL84_B
    },
    // CWF
    {
                    834,    // AWBGAIN_CWF_R
                    512,    // AWBGAIN_CWF_G
                    1008    // AWBGAIN_CWF_B
    },
    // DNP
    {
                    977,    // AWBGAIN_DNP_R
                    512,    // AWBGAIN_DNP_G
                    860    // AWBGAIN_DNP_B
    },
    // D65
    {
                    989,    // AWBGAIN_D65_R
                    512,    // AWBGAIN_D65_G
                    668    // AWBGAIN_D65_B
    },
    // DF
    {
                    1055,    // AWBGAIN_DF_R
                    512,    // AWBGAIN_DF_G
                    794    // AWBGAIN_DF_B
    }
  },
  // Rotation matrix parameter
  {
    1,  // i4RotationAngle
    256,  // i4Cos
    4 // i4Sin
  },
  // Daylight locus parameter
  {
    -130, // i4SlopeNumerator
    128 // i4SlopeDenominator
  },
 // Predictor gain
 {
                    101, // i4PrefRatio100
                        // DaylightLocus_L
                        {
                        959,    // i4R
                        530,    // i4G
                        688    // i4B
                        },
                        // DaylightLocus_H
                        {
                        850,    // i4R
                            512,    // i4G
                        782    // i4B
                        },
                        // Temporal General
                        {
                        989,    // i4R
                            512,    // i4G
                        668    // i4B
                        },
                        // AWB LSC Gain
                        {
                        850,        // i4R
                            512,    // i4G
                        782        // i4B
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
                        -157,    // TungRightBound
                        -769,    // TungLeftBound
                        -264,    // TungUpperBound
                        -365    // TungLowerBound
    },
    // Warm fluorescent
    {
                        -157,    // WFluoRightBound
                        -769,    // WFluoLeftBound
                        -365,    // WFluoUpperBound
                        -519    // WFluoLowerBound
    },
    // Fluorescent
    {
                        -2,    // FluoRightBound
                        -157,    // FluoLeftBound
                        -284,    // FluoUpperBound
                        -406    // FluoLowerBound
    },
    // CWF
    {
                        0,    // CWFRightBound
                        -157,    // CWFLeftBound
                        -406,    // CWFUpperBound
                        -484    // CWFLowerBound
    },
    // Daylight
    {
                        170,    // DayRightBound
                        -2,    // DayLeftBound
                        -284,    // DayUpperBound
                        -406    // DayLowerBound
    },
    // Shade
    {
                        500,    // ShadeRightBound
                        170,    // ShadeLeftBound
                        -284,    // ShadeUpperBound
                        -378    // ShadeLowerBound
    },
    // Daylight Fluorescent
    {
                        170,    // DFRightBound
                        0,    // DFLeftBound
                        -406,    // DFUpperBound
                        -484    // DFLowerBound
    }
  },
  // PWB light area
  {
    // Reference area
    {
                        500,    // PRefRightBound
                        -769,    // PRefLeftBound
                        -239,    // PRefUpperBound
                        -519    // PRefLowerBound
    },
    // Daylight
    {
                        195,    // PDayRightBound
                        -2,    // PDayLeftBound
                        -284,    // PDayUpperBound
                        -406    // PDayLowerBound
    },
    // Cloudy daylight
    {
                        295,    // PCloudyRightBound
                        120,    // PCloudyLeftBound
                        -284,    // PCloudyUpperBound
                        -406    // PCloudyLowerBound
    },
    // Shade
    {
                        395,    // PShadeRightBound
                        120,    // PShadeLeftBound
                        -284,    // PShadeUpperBound
                        -406    // PShadeLowerBound
    },
    // Twilight
    {
                        -2,    // PTwiRightBound
                        -162,    // PTwiLeftBound
                        -284,    // PTwiUpperBound
                        -406    // PTwiLowerBound
    },
    // Fluorescent
    {
                        190,    // PFluoRightBound
                        -190,    // PFluoLeftBound
                        -294,    // PFluoUpperBound
                        -479    // PFluoLowerBound
    },
    // Warm fluorescent
    {
                        -158,    // PWFluoRightBound
                        -358,    // PWFluoLeftBound
                        -294,    // PWFluoUpperBound
                        -479    // PWFluoLowerBound
    },
    // Incandescent
    {
                        -158,    // PIncaRightBound
                        -358,    // PIncaLeftBound
                        -284,    // PIncaUpperBound
                        -406    // PIncaLowerBound
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
                    936,    // PWB_Day_R
      512,  // PWB_Day_G
                    710    // PWB_Day_B
    },
    // Cloudy daylight
    {
                    1085,    // PWB_Cloudy_R
      512,  // PWB_Cloudy_G
                    610    // PWB_Cloudy_B
    },
    // Shade
    {
                    1159,    // PWB_Shade_R
      512,  // PWB_Shade_G
                    569    // PWB_Shade_B
    },
    // Twilight
    {
                    738,    // PWB_Twi_R
      512,  // PWB_Twi_G
                    908    // PWB_Twi_B
    },
    // Fluorescent
    {
                    871,    // PWB_Fluo_R
      512,  // PWB_Fluo_G
                    857    // PWB_Fluo_B
    },
    // Warm fluorescent
    {
                    618,    // PWB_WFluo_R
      512,  // PWB_WFluo_G
                    1222    // PWB_WFluo_B
    },
    // Incandescent
    {
                    583,    // PWB_Inca_R
      512,  // PWB_Inca_G
                    1156    // PWB_Inca_B
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
                    4046    // TUNG_OFFS
    },
    // Warm fluorescent
    {
      50, // WFluo_SLIDER
                    4046    // WFluo_OFFS
    },
    // Shade
    {
      50, // Shade_SLIDER
      909 // Shade_OFFS
    },
            // Sunset Area
            {
                    72,   // i4Sunset_BoundXr_Thr
                    -431     // i4Sunset_BoundYr_Thr
                           },
            // Shade F Area
            {
                    -157,   // i4BoundXrThr
                    -386    // i4BoundYrThr
                            },
            // Shade F Vertex
           {
                    -80,   // i4BoundXrThr
                    -396    // i4BoundYrThr
                            },
            // Shade CWF Area
            {
                    -157,   // i4BoundXrThr
                    -433    // i4BoundYrThr
                            },
            // Shade CWF Vertex
            {
                    -79,   // i4BoundXrThr
                    -459    // i4BoundYrThr
                            }
  },
  // CCT estimation
  {
    // CCT
    {
      2300, // CCT0
      2850, // CCT1
      3750, // CCT2
      5100, // CCT3
      6500     // CCT4
    },
    // Rotated X coordinate
    {
                        -509,    // i4RotatedXCoordinate[0]
                        -398,    // i4RotatedXCoordinate[1]
                        -230,    // i4RotatedXCoordinate[2]
                        -100,    // i4RotatedXCoordinate[3]
                        0    // i4RotatedXCoordinate[4]
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
                        //LV0  1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18
                { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  50,  25,   2,   2,   2,   2,   2,   2,   2}  // (%) i4CWFDF_LUTThr
                    },
                    // AWB light neutral noise reduction for outdoor
                    {
                        //LV0  1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18
                        // Non neutral
                    { 3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
                    // Flurescent
                {   0,   0,   0,   0,   0,   3,   5,   5,   5,   5,   5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
                    // CWF
                {   0,   0,   0,   0,   0,   3,   5,   5,   5,   5,   5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
                    // Daylight
                    { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  2,  2,  2,  2,  2,  2,  2},  // (%)
                    // DF
                    { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
                    },
                    // AWB feature detection
                    {
                        // Sunset Prop
                        {
                            1,      // i4Enable
                            120,        // i4LVThr_L
                          130,        // i4LVThr_H
                            10,         // i4SunsetCountThr
                            0,          // i4SunsetCountRatio_L
                            171         // i4SunsetCountRatio_H
                        },

                        // Shade F Detection
                        {
                            1,      // i4Enable
                        50,        // i4LVThr_L
                        70,        // i4LVThr_H
                            128         // i4DaylightProb
                        },

                        // Shade CWF Detection
                        {
                            1,        // i4Enable
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
            //LV0    1     2     3      4     5     6     7     8      9      10     11    12   13     14    15   16    17    18
                    { 100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  50,  25,  0,  0,  0,  0}, // Strobe
                { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  75,  50,  25,   0,   0,   0}, // Tungsten
                { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  75,  50,  25,  25,  25,   0,   0,   0}, // Warm fluorescent
                { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  95,  75,  50,  25,  25,  25,   0,   0,   0}, // Fluorescent
                {  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  80,  55,  30,  30,  30,  30,   0,   0,   0}, // CWF
                { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  75,  50,  40,  30,  20}, // Daylight
                { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  75,  50,  25,   0,   0,   0,   0}, // Shade
                {  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  80,  55,  30,  30,  30,  30,   0,   0,   0} // Daylight fluorescent
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
