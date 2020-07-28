#include "camera_custom_nvram.h"
#include "camera_awb_tuning_para_s5k3l6mipiraw.h"

#define NVRAM_TUNING_PARAM_NUM  6511001

template <>
AWB_NVRAM_T const&
getAWBNVRAM_s5k3l6mipiraw<CAM_SCENARIO_CUSTOM3>() // VT
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
                    1329,    // D65Gain_R
                    512,    // D65Gain_G
                    658    // D65Gain_B
                    }
                },
                // Original XY coordinate of AWB light source
                {
                    // Strobe
                        {
                    152,    // i4X
                    -374    // i4Y
                        },
                        // Horizon
                        {
                    -327,    // OriX_Hor
                    -446    // OriY_Hor
                        },
                        // A
                        {
                    -182,    // OriX_A
                    -451    // OriY_A
                        },
                        // TL84
                        {
                    -15,    // OriX_TL84
                    -468    // OriY_TL84
                        },
                        // CWF
                        {
                    21,    // OriX_CWF
                    -537    // OriY_CWF
                        },
                        // DNP
                        {
                    96,    // OriX_DNP
                    -451    // OriY_DNP
                        },
                        // D65
                        {
                    260,    // OriX_D65
                    -445    // OriY_D65
                        },
                        // DF
                        {
                    210,    // OriX_DF
                    -497    // OriY_DF
                        }
                },
                // Rotated XY coordinate of AWB light source
                {
                    // Strobe
                        {
                    146,    // i4X
                    -376    // i4Y
                },
                // Horizon
                {
                    -334,    // RotX_Hor
                    -441    // RotY_Hor
                },
                // A
                {
                    -189,    // RotX_A
                    -448    // RotY_A
                },
                // TL84
                {
                    -22,    // RotX_TL84
                    -468    // RotY_TL84
                },
                // CWF
                {
                    13,    // RotX_CWF
                    -537    // RotY_CWF
                },
                // DNP
                {
                    89,    // RotX_DNP
                    -452    // RotY_DNP
                },
                // D65
                {
                    253,    // RotX_D65
                    -449    // RotY_D65
                },
                // DF
                {
                    202,    // RotX_DF
                    -500    // RotY_DF
                }
            },
            // AWB gain of AWB light source
            {
                // Strobe
                {
                    1043,    // i4R
                    512,    // i4G
                    691    // i4B
                },
                // Horizon
                {
                    602,    // AWBGAIN_HOR_R
                    512,    // AWBGAIN_HOR_G
                    1457    // AWBGAIN_HOR_B
                },
                // A
                {
                    737,    // AWBGAIN_A_R
                    512,    // AWBGAIN_A_G
                    1205    // AWBGAIN_A_B
                },
                // TL84
                {
                    945,    // AWBGAIN_TL84_R
                    512,    // AWBGAIN_TL84_G
                    985    // AWBGAIN_TL84_B
                },
                // CWF
                {
                    1090,    // AWBGAIN_CWF_R
                    512,    // AWBGAIN_CWF_G
                    1029    // AWBGAIN_CWF_B
                },
                // DNP
                {
                    1074,    // AWBGAIN_DNP_R
                    512,    // AWBGAIN_DNP_G
                    828    // AWBGAIN_DNP_B
                },
                // D65
                {
                    1329,    // AWBGAIN_D65_R
                    512,    // AWBGAIN_D65_G
                    658    // AWBGAIN_D65_B
                },
                // DF
                {
                    1333,    // AWBGAIN_DF_R
                    512,    // AWBGAIN_DF_G
                    755    // AWBGAIN_DF_B
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
                -132,    // i4SlopeNumerator
                128    // i4SlopeDenominator
                },
              // Predictor gain
                {
                101, // i4PrefRatio100
                // DaylightLocus_L
                {
                    1299,    // i4R
                    530,    // i4G
                    678    // i4B
                },
                // DaylightLocus_H
                {
                    1017,    // i4R
                    512,    // i4G
                    867    // i4B
                },
                // Temporal General
                {
                    1329,    // i4R
                    512,    // i4G
                    808    // i4B 658
                },
                // AWB LSC Gain
                {
                    1036,        // i4R
                    512,        // i4G
                    851        // i4B
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
                    -89,    // TungRightBound
                    -734,    // TungLeftBound
                    -300,    // TungUpperBound
                    -300    // TungLowerBound
                },
                // Warm fluorescent
                {
                    -89,    // WFluoRightBound
                    -734,    // WFluoLeftBound
                    -300,    // WFluoUpperBound
                    -627    // WFluoLowerBound
                },
                // Fluorescent
                {
                    53,    // FluoRightBound
                    -89,    // FluoLeftBound
                    -389,    // FluoUpperBound
                    -503    // FluoLowerBound
                },
                // CWF
                {
                53,    // CWFRightBound
                -89,    // CWFLeftBound
                -503,    // CWFUpperBound
                -592    // CWFLowerBound
                },
                // Daylight
                {
                    323,    // DayRightBound
                    53,    // DayLeftBound
                    -300,    // DayUpperBound
                    -592    // DayLowerBound
                },
                // Shade
                {
                    613,    // ShadeRightBound
                    323,    // ShadeLeftBound
                    -300,    // ShadeUpperBound
                    -484    // ShadeLowerBound
                },
                // Daylight Fluorescent
                {
                    283,    // DFRightBound
                    96,    // DFLeftBound
                    -592,    // DFUpperBound
                    -592    // DFLowerBound
                }
                },
            // PWB light area
            {
                // Reference area
                {
                    613,    // PRefRightBound
                    -734,    // PRefLeftBound
                    -361,    // PRefUpperBound
                    -627    // PRefLowerBound
                },
                // Daylight
                {
                    308,    // PDayRightBound
                    53,    // PDayLeftBound
                    -389,    // PDayUpperBound
                    -503    // PDayLowerBound
                },
                // Cloudy daylight
                {
                    408,    // PCloudyRightBound
                    233,    // PCloudyLeftBound
                    -389,    // PCloudyUpperBound
                    -503    // PCloudyLowerBound
                },
                // Shade
                {
                    508,    // PShadeRightBound
                    233,    // PShadeLeftBound
                    -389,    // PShadeUpperBound
                    -503    // PShadeLowerBound
                },
                // Twilight
                {
                    53,    // PTwiRightBound
                    -107,    // PTwiLeftBound
                    -389,    // PTwiUpperBound
                    -503    // PTwiLowerBound
                },
                // Fluorescent
                {
                    303,    // PFluoRightBound
                    -122,    // PFluoLeftBound
                    -399,    // PFluoUpperBound
                    -587    // PFluoLowerBound
                },
                // Warm fluorescent
                {
                    -89,    // PWFluoRightBound
                    -289,    // PWFluoLeftBound
                    -399,    // PWFluoUpperBound
                    -587    // PWFluoLowerBound
                },
                // Incandescent
                {
                    -89,    // PIncaRightBound
                    -289,    // PIncaLeftBound
                    -389,    // PIncaUpperBound
                    -503    // PIncaLowerBound
                },
                // Gray World
                {
                    5000,    // PGWRightBound
                    -5000,    // PGWLeftBound
                    5000,    // PGWUpperBound
                    -5000    // PGWLowerBound
                }
            },
            // PWB default gain
            {
                // Daylight
                {
                    1202,    // PWB_Day_R
                    512,    // PWB_Day_G
                    724    // PWB_Day_B
                },
                // Cloudy daylight
                {
                    1449,    // PWB_Cloudy_R
                    512,    // PWB_Cloudy_G
                    597    // PWB_Cloudy_B
                },
                // Shade
                {
                    1548,    // PWB_Shade_R
                    512,    // PWB_Shade_G
                    557    // PWB_Shade_B
                },
                // Twilight
                {
                    912,    // PWB_Twi_R
                    512,    // PWB_Twi_G
                    963    // PWB_Twi_B
                },
                // Fluorescent
                {
                    1137,    // PWB_Fluo_R
                    512,    // PWB_Fluo_G
                    872    // PWB_Fluo_B
                },
                // Warm fluorescent
                {
                    784,    // PWB_WFluo_R
                    512,    // PWB_WFluo_G
                    1280    // PWB_WFluo_B
                },
                // Incandescent
                {
                    735,    // PWB_Inca_R
                    512,    // PWB_Inca_G
                    1203    // PWB_Inca_B
                },
                // Gray World
                {
                    512,    // PWB_GW_R
                    512,    // PWB_GW_G
                    512    // PWB_GW_B
                }
            },
            // AWB preference color
            {
                // Tungsten
                {
                    40,    // TUNG_SLIDER
                    4860    // TUNG_OFFS
                },
                // Warm fluorescent
                {
                    40,    // WFluo_SLIDER
                    4860    // WFluo_OFFS
                },
                // Shade
                {
                    50,    // Shade_SLIDER
                    909    // Shade_OFFS
                },
                // Sunset Area
                {
                    117,   // i4Sunset_BoundXr_Thr
                    -452    // i4Sunset_BoundYr_Thr
                },
                // Shade F Area
                {
                    -89,   // i4BoundXrThr
                    -472    // i4BoundYrThr
                },
                // Shade F Vertex
                {
                    -18,   // i4BoundXrThr
                    -488    // i4BoundYrThr
                },
                // Shade CWF Area
                {
                    -89,   // i4BoundXrThr
                    -541    // i4BoundYrThr
                },
                // Shade CWF Vertex
                {
                    4,   // i4BoundXrThr
                    -567    // i4BoundYrThr
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
                -587,    // i4RotatedXCoordinate[0]
                -442,    // i4RotatedXCoordinate[1]
                -275,    // i4RotatedXCoordinate[2]
                -164,    // i4RotatedXCoordinate[3]
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
                    {0, 500, 1000, 1500, 2500, 3500, 3500, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000} // i4LUTOut
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
                    {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {508, 512, 512}, {500, 512, 512}, {508, 512, 512}, {500, 512, 512}, {500, 512, 512}, {500, 512, 512},
                    {505, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                  }, // WARM F
                  {
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {505, 512, 512}, {500, 512, 512},
                        {500, 512, 512}, {500, 512, 512}, {505, 512, 512}, {505, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}
                    }, // F
                    {
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                    }, // CWF
                    {
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 500}, {512, 512, 500}, {506, 512, 500}, {500, 512, 500}, {500, 512, 506}, {500, 512, 512}, {500, 512, 512}, {500, 512, 512},
                        {500, 512, 512}, {500, 512, 512}, {505, 512, 512}, {505, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}
                    }, // DAYLIGHT
                    {
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {500, 512, 512}, {500, 512, 512}, {500, 512, 512}, {500, 512, 516}, {506, 512, 520}, {500, 512, 520}, {500, 512, 516},
                        {512, 512, 500}, {512, 512, 500}, {512, 512, 500}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
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
                        90,        // i4LVThr_H
                        128         // i4DaylightProb
                    },

                    // Shade CWF Detection
                    {
                        1,          // i4Enable
                        50,        // i4LVThr_L
                        90,        // i4LVThr_H
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
                    {100, 98, 98, 95, 90, 85, 85, 85, 85, 85,  75,   75,  50,  40,  30,  20,  10,   0,   0}, // Warm fluorescent
                    { 100, 100, 100, 100, 100, 100, 90, 85, 85, 85,  80,  70,  60,  50,  40,  30,   20,   10,   0}, // Fluorescent
                    { 90,  90,  90,  90,  90,  90,  90,  90,  90,  90,   80,   55,  30,  30,  30,  30,  0,   0,   0}, // CWF
                    {100, 100, 100, 90, 80, 80, 90, 90, 90, 90,  80/*d*/,  80, 80, 70,  60,  50, 40,  30,  20}, // Daylight
                    {100, 100, 100, 100, 90, 80, 70, 60, 70, 80,  80,  80,  75,  50,  25,   0,  0,   0,   0}, // Shade
                    { 90,  90,  90,  90,  90,  90,  90,  90,  90,  90,   80,   55,  30,  30,  30,  30,  0,   0,   0}  // Daylight fluorescent
                },

                // AWB tuning information
                {
                    6761,       // project code
                    5588,       // model
                    20181227,   // date //wrd
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
