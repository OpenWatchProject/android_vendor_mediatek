#include "camera_custom_nvram.h"
#include "camera_awb_tuning_para_s5k5e8yxmipiraw.h"

#define NVRAM_TUNING_PARAM_NUM  6511001

template <>
AWB_NVRAM_T const&
getAWBNVRAM_s5k5e8yxmipiraw<CAM_SCENARIO_PREVIEW>() // PREVIEW
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
                        1223, // D65Gain_R
                        512, // D65Gain_G
                        660 // D65Gain_B
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
                        -366, // OriX_Hor
                        -359 // OriY_Hor
                    },
                    // A
                    {
                        -223, // OriX_A
                        -387 // OriY_A
                    },
                    // TL84
                    {
                        -11, // OriX_TL84
                        -416 // OriY_TL84
                    },
                    // CWF
                    {
                        -5, // OriX_CWF
                        -514 // OriY_CWF
                    },
                    // DNP
                    {
                        107, // OriX_DNP
                        -433 // OriY_DNP
                    },
                    // D65
                    {
                        228, // OriX_D65
                        -416 // OriY_D65
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
                        -339, // RotX_Hor
                        -383 // RotY_Hor
                    },
                    // A
                    {
                        -195, // RotX_A
                        -401 // RotY_A
                    },
                    // TL84
                    {
                        18, // RotX_TL84
                        -415 // RotY_TL84
                    },
                    // CWF
                    {
                        31, // RotX_CWF
                        -512 // RotY_CWF
                    },
                    // DNP
                    {
                        137, // RotX_DNP
                        -424 // RotY_DNP
                    },
                    // D65
                    {
                        256, // RotX_D65
                        -398 // RotY_D65
                    },
                    // DF
                    {
                        254, // RotX_DF
                        -498 // RotY_DF
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
                        517, // AWBGAIN_HOR_G
                        1378 // AWBGAIN_HOR_B
                    },
                    // A
                    {
                        639, // AWBGAIN_A_R
                        512, // AWBGAIN_A_G
                        1170 // AWBGAIN_A_B
                    },
                    // TL84
                    {
                        886, // AWBGAIN_TL84_R
                        512, // AWBGAIN_TL84_G
                        912 // AWBGAIN_TL84_B
                    },
                    // CWF
                    {
                        1020, // AWBGAIN_CWF_R
                        512, // AWBGAIN_CWF_G
                        1034 // AWBGAIN_CWF_B
                    },
                    // DNP
                    {
                        1065, // AWBGAIN_DNP_R
                        512, // AWBGAIN_DNP_G
                        796 // AWBGAIN_DNP_B
                    },
                    // D65
                    {
                        1223, // AWBGAIN_D65_R
                        512, // AWBGAIN_D65_G
                        660 // AWBGAIN_D65_B
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
                    -4, // RotationAngle
                    255, // Cos
                    -18 // Sin
                },
                // Daylight locus parameter
                {
                    -110, // SlopeNumerator
                    128 // SlopeDenominator
                },
                // Predictor gain
                {
                    0, // i4PrefRatio100
                    // DaylightLocus_L
                    {
                        1152, // i4R
                        512, // i4G
                        661 // i4B
                    },
                    // DaylightLocus_H
                    {
                        962, // i4R
                        512, // i4G
                        787 // i4B
                    },
                    // Temporal General
                    {
                        953, // i4R
                        512, // i4G
                        835 // i4B
                    },
                    // AWB LSC
                    {
                        962, // i4R
                        512, // i4G
                        787 // i4B
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
                        -67, // TungRightBound
                        -739, // TungLeftBound
                        -308, // TungUpperBound
                        -373 // TungLowerBound
                    },
                    // Warm fluorescent
                    {
                        -67, // WFluoRightBound
                        -739, // WFluoLeftBound
                        -373, // WFluoUpperBound
                        -602 // WFluoLowerBound
                    },
                    // Fluorescent
                    {
                        98, // FluoRightBound
                        -67, // FluoLeftBound
                        -338, // FluoUpperBound
                        -464 // FluoLowerBound
                    },
                    // CWF
                    {
                        129, // CWFRightBound
                        -67, // CWFLeftBound
                        -464, // CWFUpperBound
                        -567 // CWFLowerBound
                    },
                    // Daylight
                    {
                        286, // DayRightBound
                        98, // DayLeftBound
                        -338, // DayUpperBound
                        -464 // DayLowerBound
                    },
                    // Shade
                    {
                        616, // ShadeRightBound
                        286, // ShadeLeftBound
                        -338, // ShadeUpperBound
                        -444 // ShadeLowerBound
                    },
                    // Daylight Fluorescent
                    {
                        286, // DFRightBound
                        129, // DFLeftBound
                        -464, // DFUpperBound
                        -567 // DFLowerBound
                    }
                },
                // PWB light area
                {
                    // Reference area
                    {
                        616, // PRefRightBound
                        -739, // PRefLeftBound
                        -283, // PRefUpperBound
                        -602 // PRefLowerBound
                    },
                    // Daylight
                    {
                        311, // PDayRightBound
                        98, // PDayLeftBound
                        -338, // PDayUpperBound
                        -464 // PDayLowerBound
                    },
                    // Cloudy daylight
                    {
                        411, // PCloudyRightBound
                        236, // PCloudyLeftBound
                        -338, // PCloudyUpperBound
                        -464 // PCloudyLowerBound
                    },
                    // Shade
                    {
                        511, // PShadeRightBound
                        236, // PShadeLeftBound
                        -338, // PShadeUpperBound
                        -464 // PShadeLowerBound
                    },
                    // Twilight
                    {
                        98, // PTwiRightBound
                        -62, // PTwiLeftBound
                        -338, // PTwiUpperBound
                        -464 // PTwiLowerBound
                    },
                    // Fluorescent
                    {
                        306, // PFluoRightBound
                        -82, // PFluoLeftBound
                        -348, // PFluoUpperBound
                        -562 // PFluoLowerBound
                    },
                    // Warm fluorescent
                    {
                        -95, // PWFluoRightBound
                        -295, // PWFluoLeftBound
                        -348, // PWFluoUpperBound
                        -562 // PWFluoLowerBound
                    },
                    // Incandescent
                    {
                        -95, // PIncaRightBound
                        -295, // PIncaLeftBound
                        -338, // PIncaUpperBound
                        -464 // PIncaLowerBound
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
                        1140, // PWB_Day_R
                        512, // PWB_Day_G
                        708 // PWB_Day_B
                    },
                    // Cloudy daylight
                    {
                        1354, // PWB_Cloudy_R
                        512, // PWB_Cloudy_G
                        609 // PWB_Cloudy_B
                    },
                    // Shade
                    {
                        1456, // PWB_Shade_R
                        512, // PWB_Shade_G
                        572 // PWB_Shade_B
                    },
                    // Twilight
                    {
                        870, // PWB_Twi_R
                        512, // PWB_Twi_G
                        895 // PWB_Twi_B
                    },
                    // Fluorescent
                    {
                        1067, // PWB_Fluo_R
                        512, // PWB_Fluo_G
                        859 // PWB_Fluo_B
                    },
                    // Warm fluorescent
                    {
                        684, // PWB_WFluo_R
                        512, // PWB_WFluo_G
                        1264 // PWB_WFluo_B
                    },
                    // Incandescent
                    {
                        639, // PWB_Inca_R
                        512, // PWB_Inca_G
                        1169 // PWB_Inca_B
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
                        4613 // TUNG_OFFS
                    },
                    // Warm fluorescent
                    {
                        40, // WFluo_SLIDER
                        4613 // WFluo_OFFS
                    },
                    // Shade
                    {
                        50, // Shade_SLIDER
                        910 // Shade_OFFS
                    },
                    // Sunset Area
                    {
                        167, // i4Sunset_BoundXr_Thr
                        -424 // i4Sunset_BoundYr_Thr
                    },
                    // Shade F Area
                    {
                        -67, // i4ShadeF_BoundXr_Thr
                        -419 // i4ShadeF_BoundYr_Thr
                    },
                    // Shade F Vertex
                    {
                        16, // i4ShadeF_VertexXr_Thr
                        -442 // i4ShadeF_VertexYr_Thr
                    },
                    // Shade CWF Area
                    {
                        -67, // i4ShadeCWF_BoundXr_Thr
                        -516 // i4ShadeCWF_BoundYr_Thr
                    },
                    // Shade CWF Vertex
                    {
                        31, // i4ShadeCWF_VertexXr_Thr
                        -542 // i4ShadeCWF_VertexYr_Thr
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
                        -595,  // RotatedXCoordinate0
                        -451,  // RotatedXCoordinate1
                        -238,  // RotatedXCoordinate2
                        -119,  // RotatedXCoordinate3
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
