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
                        0, // UnitGain_R
                        0, // UnitGain_G
                        0  // UnitGain_B
                    },
                    // rGoldenGain (golden sample gain: 1.0 = 512)
                    {
                        0, // GoldenGain_R
                        0, // GoldenGain_G
                        0  // GoldenGain_B
                    },
                    // rUnitGain for TL84(unit gain: 1.0 = 512)
                    {
                        0, // UnitGain_R
                        0, // UnitGain_G
                        0  // UnitGain_B
                    },
                    // rGoldenGain for TL84(golden sample gain: 1.0 = 512)
                    {
                        0, // GoldenGain_R
                        0, // GoldenGain_G
                        0  // GoldenGain_B
                    },
                    // rUnitGain for ALight(unit gain: 1.0 = 512)
                    {
                        0, // UnitGain_R
                        0, // UnitGain_G
                        0  // UnitGain_B
                    },
                    // rGoldenGain for ALight(golden sample gain: 1.0 = 512)
                    {
                        0, // GoldenGain_R
                        0, // GoldenGain_G
                        0  // GoldenGain_B
                    },
                    // rTuningUnitGain (Tuning sample unit gain: 1.0 = 512)
                    {
                        0, // TuningUnitGain_R
                        0, // TuningUnitGain_G
                        0 // TuningUnitGain_B
                    },
                    // rD65Gain (D65 WB gain: 1.0 = 512)
                    {
                        894, // D65Gain_R
                        512, // D65Gain_G
                        664 // D65Gain_B
                    }
                },
                // Original XY coordinate of AWB light source
                {
                    // Strobe
                    {
                        0, // OriX_Strobe
                        0 // OriY_Strobe
                    },
                    // Horizon
                    {
                        -363, // OriX_Hor
                        -276 // OriY_Hor
                    },
                    // A
                    {
                        -257, // OriX_A
                        -297 // OriY_A
                    },
                    // TL84
                    {
                        -98, // OriX_TL84
                        -325 // OriY_TL84
                    },
                    // CWF
                    {
                        -94, // OriX_CWF
                        -400 // OriY_CWF
                    },
                    // DNP
                    {
                        -1, // OriX_DNP
                        -330 // OriY_DNP
                    },
                    // D65
                    {
                        110, // OriX_D65
                        -302 // OriY_D65
                    },
                    // DF
                    {
                        0, // OriX_DF
                        0 // OriY_DF
                    }
                },
                // Rotated XY coordinate of AWB light source
                {
                    // Strobe
                    {
                        0, // RotX_Strobe
                        0 // RotY_Strobe
                    },
                    // Horizon
                    {
                        -359, // RotX_Hor
                        -282 // RotY_Hor
                    },
                    // A
                    {
                        -252, // RotX_A
                        -301 // RotY_A
                    },
                    // TL84
                    {
                        -93, // RotX_TL84
                        -327 // RotY_TL84
                    },
                    // CWF
                    {
                        -88, // RotX_CWF
                        -401 // RotY_CWF
                    },
                    // DNP
                    {
                        4, // RotX_DNP
                        -330 // RotY_DNP
                    },
                    // D65
                    {
                        115, // RotX_D65
                        -300 // RotY_D65
                    },
                    // DF
                    {
                        101, // RotX_DF
                        -389 // RotY_DF
                    }
                },
                // AWB gain of AWB light source
                {
                    // Strobe
                    {
                        512, // AWBGAIN_STROBE_R
                        512, // AWBGAIN_STROBE_G
                        512 // AWBGAIN_STROBE_B
                    },
                    // Horizon
                    {
                        512, // AWBGAIN_HOR_R
                        577, // AWBGAIN_HOR_G
                        1370 // AWBGAIN_HOR_B
                    },
                    // A
                    {
                        540, // AWBGAIN_A_R
                        512, // AWBGAIN_A_G
                        1085 // AWBGAIN_A_B
                    },
                    // TL84
                    {
                        697, // AWBGAIN_TL84_R
                        512, // AWBGAIN_TL84_G
                        908 // AWBGAIN_TL84_B
                    },
                    // CWF
                    {
                        775, // AWBGAIN_CWF_R
                        512, // AWBGAIN_CWF_G
                        1000 // AWBGAIN_CWF_B
                    },
                    // DNP
                    {
                        800, // AWBGAIN_DNP_R
                        512, // AWBGAIN_DNP_G
                        801 // AWBGAIN_DNP_B
                    },
                    // D65
                    {
                        894, // AWBGAIN_D65_R
                        512, // AWBGAIN_D65_G
                        664 // AWBGAIN_D65_B
                    },
                    // DF
                    {
                        512, // AWBGAIN_DF_R
                        512, // AWBGAIN_DF_G
                        512 // AWBGAIN_DF_B
                    }
                },
                // Rotation matrix parameter
                {
                    -1, // RotationAngle
                    256, // Cos
                    -4 // Sin
                },
                // Daylight locus parameter
                {
                    -122, // SlopeNumerator
                    128 // SlopeDenominator
                },
                // Predictor gain
                {
                    0, // i4PrefRatio100
                    // DaylightLocus_L
                    {
                        845, // i4R
                        512, // i4G
                        665 // i4B
                    },
                    // DaylightLocus_H
                    {
                        733, // i4R
                        512, // i4G
                        796 // i4B
                    },
                    // Temporal General
                    {
                        733, // i4R
                        512, // i4G
                        839 // i4B
                    },
                    // AWB LSC
                    {
                        733, // i4R
                        512, // i4G
                        796 // i4B
                    }
                },
                // AWB light area
                {
                    // Strobe
                    {
                        0, // StrobeRightBound
                        0, // StrobeLeftBound
                        0, // StrobeUpperBound
                        0 // StrobeLowerBound
                    },
                    // Tungsten
                    {
                        -157, // TungRightBound
                        -759, // TungLeftBound
                        -207, // TungUpperBound
                        -272 // TungLowerBound
                    },
                    // Warm fluorescent
                    {
                        -157, // WFluoRightBound
                        -759, // WFluoLeftBound
                        -272, // WFluoUpperBound
                        -491 // WFluoLowerBound
                    },
                    // Fluorescent
                    {
                        -28, // FluoRightBound
                        -157, // FluoLeftBound
                        -240, // FluoUpperBound
                        -364 // FluoLowerBound
                    },
                    // CWF
                    {
                        -5, // CWFRightBound
                        -157, // CWFLeftBound
                        -364, // CWFUpperBound
                        -456 // CWFLowerBound
                    },
                    // Daylight
                    {
                        145, // DayRightBound
                        -28, // DayLeftBound
                        -240, // DayUpperBound
                        -364 // DayLowerBound
                    },
                    // Shade
                    {
                        475, // ShadeRightBound
                        145, // ShadeLeftBound
                        -240, // ShadeUpperBound
                        -340 // ShadeLowerBound
                    },
                    // Daylight Fluorescent
                    {
                        145, // DFRightBound
                        -5, // DFLeftBound
                        -364, // DFUpperBound
                        -456 // DFLowerBound
                    }
                },
                // PWB light area
                {
                    // Reference area
                    {
                        475, // PRefRightBound
                        -759, // PRefLeftBound
                        -182, // PRefUpperBound
                        -491 // PRefLowerBound
                    },
                    // Daylight
                    {
                        170, // PDayRightBound
                        -28, // PDayLeftBound
                        -240, // PDayUpperBound
                        -364 // PDayLowerBound
                    },
                    // Cloudy daylight
                    {
                        270, // PCloudyRightBound
                        95, // PCloudyLeftBound
                        -240, // PCloudyUpperBound
                        -364 // PCloudyLowerBound
                    },
                    // Shade
                    {
                        370, // PShadeRightBound
                        95, // PShadeLeftBound
                        -240, // PShadeUpperBound
                        -364 // PShadeLowerBound
                    },
                    // Twilight
                    {
                        -28, // PTwiRightBound
                        -188, // PTwiLeftBound
                        -240, // PTwiUpperBound
                        -364 // PTwiLowerBound
                    },
                    // Fluorescent
                    {
                        165, // PFluoRightBound
                        -193, // PFluoLeftBound
                        -250, // PFluoUpperBound
                        -451 // PFluoLowerBound
                    },
                    // Warm fluorescent
                    {
                        -152, // PWFluoRightBound
                        -352, // PWFluoLeftBound
                        -250, // PWFluoUpperBound
                        -451 // PWFluoLowerBound
                    },
                    // Incandescent
                    {
                        -152, // PIncaRightBound
                        -352, // PIncaLeftBound
                        -240, // PIncaUpperBound
                        -364 // PIncaLowerBound
                    },
                    // Gray World
                    {
                        5000, // PGWRightBound
                        -5000, // PGWLeftBound
                        5000, // PGWUpperBound
                        -5000 // PGWLowerBound
                    }
                },
                // PWB default gain
                {
                    // Daylight
                    {
                        844, // PWB_Day_R
                        512, // PWB_Day_G
                        705 // PWB_Day_B
                    },
                    // Cloudy daylight
                    {
                        984, // PWB_Cloudy_R
                        512, // PWB_Cloudy_G
                        608 // PWB_Cloudy_B
                    },
                    // Shade
                    {
                        1054, // PWB_Shade_R
                        512, // PWB_Shade_G
                        569 // PWB_Shade_B
                    },
                    // Twilight
                    {
                        660, // PWB_Twi_R
                        512, // PWB_Twi_G
                        895 // PWB_Twi_B
                    },
                    // Fluorescent
                    {
                        801, // PWB_Fluo_R
                        512, // PWB_Fluo_G
                        845 // PWB_Fluo_B
                    },
                    // Warm fluorescent
                    {
                        578, // PWB_WFluo_R
                        512, // PWB_WFluo_G
                        1160 // PWB_WFluo_B
                    },
                    // Incandescent
                    {
                        541, // PWB_Inca_R
                        512, // PWB_Inca_G
                        1085 // PWB_Inca_B
                    },
                    // Gray World
                    {
                        512, // PWB_GW_R
                        512, // PWB_GW_G
                        512 // PWB_GW_B
                    }
                },
                // AWB preference color
                {
                    // Tungsten
                    {
                        40, // TUNG_SLIDER
                        3906 // TUNG_OFFS
                    },
                    // Warm fluorescent
                    {
                        40, // WFluo_SLIDER
                        3906 // WFluo_OFFS
                    },
                    // Shade
                    {
                        50, // Shade_SLIDER
                        909 // Shade_OFFS
                    },
                    // Sunset Area
                    {
                        28, // i4Sunset_BoundXr_Thr
                        -330 // i4Sunset_BoundYr_Thr
                    },
                    // Shade F Area
                    {
                        -157, // i4ShadeF_BoundXr_Thr
                        -331 // i4ShadeF_BoundYr_Thr
                    },
                    // Shade F Vertex
                    {
                        -93, // i4ShadeF_VertexXr_Thr
                        -348 // i4ShadeF_VertexYr_Thr
                    },
                    // Shade CWF Area
                    {
                        -157, // i4ShadeCWF_BoundXr_Thr
                        -405 // i4ShadeCWF_BoundYr_Thr
                    },
                    // Shade CWF Vertex
                    {
                        -81, // i4ShadeCWF_VertexXr_Thr
                        -431 // i4ShadeCWF_VertexYr_Thr
                    }
                },
                // CCT estimation
                {
                    // CCT
                    {
                        2300,  // CCT0
                        2850,  // CCT1
                        3750,  // CCT2
                        5100,  // CCT3
                        6500  // CCT4
                    },
                    // Rotated X coordinate
                    {
                        -474,  // RotatedXCoordinate0
                        -367,  // RotatedXCoordinate1
                        -208,  // RotatedXCoordinate2
                        -111,  // RotatedXCoordinate3
                        0  // RotatedXCoordinate4
                    }
                }
            },
            // Algorithm Tuning Paramter
            {
                // AWB Backup Enable
                0,
                // Daylight locus offset LUTs for tungsten
                {
                    21,// i4Size: LUT dimension
                    {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000}, // i4LUTIn
                    {0, 350, 800, 1222, 1444, 1667, 1889, 2111, 2333, 2556, 2778, 3000, 3222, 3444, 3667, 3889, 4111, 4333, 4556, 4778, 5000}   // i4LUTOut
                },
                // Daylight locus offset LUTs for WF
                {
                    21,// i4Size: LUT dimension
                    {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000}, // i4LUTIn
                    {0, 350, 700, 1000, 1444, 1667, 1889, 2111, 2333, 2556, 2778, 3000, 3222, 3444, 3667, 3889, 4111, 4333, 4556, 4778, 5000}   // i4LUTOut
                },
                // Daylight locus offset LUTs for Shade
                {
                    21,// i4Size: LUT dimension
                    {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000}, // i4LUTIn
                    {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000}   // i4LUTOut
                },
                // Preference gain for each light source
                {
                    {
                        //    LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        //    LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                    }, // STROBE
                    {
                        //    LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        //    LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                    }, // TUNGSTEN
                    {
                        //    LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        //    LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                    }, // WARM F
                    {
                        //    LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        //    LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}
                    }, // F
                    {
                        //    LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        //    LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                    }, // CWF
                    {
                        //    LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        //    LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}
                    }, // DAYLIGHT
                    {
                        //    LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        //    LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                    }, // SHADE
                    {
                        //    LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        //    LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                    } // DAYLIGHT F
                },
                // Parent block weight parameter
                {
                    1,  // bEnable
                    6  // i4ScalingFactor: [6] 1~12, [7] 1~6, [8] 1~3, [9] 1~2, [>=10]: 1
                },
                // AWB LV threshold for predictor
                {
                    90,  // i4InitLVThr_L
                    130,  // i4InitLVThr_H
                    100  // i4EnqueueLVThr
                },
                // AWB number threshold for temporal predictor
                {
                    65,  // i4Neutral_ParentBlk_Thr
                    //LV0    1     2     3     4     5     6     7     8     9     10    11    12    13    14    15    16    17    18
                    { 100,  100,  100,  100,  100,  100,  100,  100,  100,  100,   50,   25,    2,    2,    2,    2,    2,    2,    2}  // i4CWFDF_LUTThr
                },
                // AWB light neutral noise reduction for outdoor
                {
                    //LV0  1   2   3   4   5   6   7   8   9   10   11   12   13   14   15   16   17   18
                    // Non neutral
                    {  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  10,  10,  10,  10,  10,  10,  10,  10},  //
                    // Flurescent
                    {  0,  0,  0,  0,  0,  3,  5,  5,  5,  5,  5,  10,  10,  10,  10,  10,  10,  10,  10},  //
                    // CWF
                    {  0,  0,  0,  0,  0,  3,  5,  5,  5,  5,  5,  10,  10,  10,  10,  10,  10,  10,  10},  //
                    // Daylight
                    {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   2,   2,   2,   2,   2,   2,   2,   2},  //
                    // DF
                    {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  5,  10,  10,  10,  10,  10,  10,  10,  10}  //
                },
                // AWB feature detection
                {
                     // Sunset Prop
                     {
                         1, // i4Enable
                         120, // i4LVThr_L
                         130, // i4LVThr_H
                         10, // i4SunsetCountThr
                         0, // i4SunsetCountRatio_L
                         171 // i4SunsetCountRatio_H
                     },
                     // Shade F Detection
                     {
                         1, // i4Enable
                         50, // i4LVThr_L
                         90, // i4LVThr_H
                         128 // i4DaylightProb
                     },
                     // Shade CWF Detection
                     {
                         1, // i4Enable
                         50, // i4LVThr_L
                         90, // i4LVThr_H
                         192 // i4DaylightProb
                     }
                 },
                 // AWB non-neutral probability for spatial and temporal weighting look-up table (Max: 100; Min: 0)
                 {
                     //LV0   1    2    3     4     5     6     7     8     9     10    11    12    13    14    15    16    17    18
                     {  0,  33,  66,  100,  100,  100,  100,  100,  100,  100,  100,   70,   30,   20,   10,    0,    0,    0,    0}
                },
                // AWB daylight locus probability look-up table (Max: 100; Min: 0)
                {
                    //LV0    1     2     3     4     5     6     7     8     9     10    11    12    13    14   15   16   17   18
                    { 100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,   50,   25,   0,   0,   0,   0}, // Strobe
                    { 95,  95,  95,  95,  95,  95,  100,  100,  100,  100,  100,  100,  100,   75,   50,  25,   0,   0,   0}, // Tungsten
                    { 95,  95,  95,  95,  95,  95,  100,  100,  100,  100,  100,   75,   50,   25,   25,  25,   0,   0,   0}, // Warm fluorescent
                    { 100,  100,  100,  100,  100,  100,  100,  100,  100,  100,   95,   75,   50,   25,   25,  25,   0,   0,   0}, // Fluorescent
                    {  90,   90,   90,   90,   90,   90,   90,   90,   90,   90,   80,   55,   30,   30,   30,  30,   0,   0,   0}, // CWF
                    { 100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,   75,  50,  40,  30,  20}, // Daylight
                    { 100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,   75,   50,   25,   0,   0,   0,   0}, // Shade
                    {  90,   90,   90,   90,   90,   90,   90,   90,   90,   90,   80,   55,   30,   30,   30,  30,   0,   0,   0}  // Daylight fluorescent
                },
                // AWB tuning information
                {
                     0, // i4ProjCode
                     0, // i4Model
                     0, // i4Date
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
