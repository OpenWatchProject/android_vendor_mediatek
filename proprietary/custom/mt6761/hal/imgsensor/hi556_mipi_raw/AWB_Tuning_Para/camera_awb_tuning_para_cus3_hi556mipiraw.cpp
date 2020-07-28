#include "camera_custom_nvram.h"
#include "camera_awb_tuning_para_hi556mipiraw.h"

#define NVRAM_TUNING_PARAM_NUM  6511001

template <>
AWB_NVRAM_T const&
getAWBNVRAM_hi556mipiraw<CAM_SCENARIO_CUSTOM3>() // VT
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
                        1158, // D65Gain_R
                        512, // D65Gain_G
                        756 // D65Gain_B
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
                        -516, // OriX_Hor
                        -455 // OriY_Hor
                    },
                    // A
                    {
                        -330, // OriX_A
                        -453 // OriY_A
                    },
                    // TL84
                    {
                        -146, // OriX_TL84
                        -542 // OriY_TL84
                    },
                    // CWF
                    {
                        -88, // OriX_CWF
                        -588 // OriY_CWF
                    },
                    // DNP
                    {
                        15, // OriX_DNP
                        -457 // OriY_DNP
                    },
                    // D65
                    {
                        158, // OriX_D65
                        -445 // OriY_D65
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
                        -539, // RotX_Hor
                        -429 // RotY_Hor
                    },
                    // A
                    {
                        -353, // RotX_A
                        -436 // RotY_A
                    },
                    // TL84
                    {
                        -174, // RotX_TL84
                        -535 // RotY_TL84
                    },
                    // CWF
                    {
                        -118, // RotX_CWF
                        -584 // RotY_CWF
                    },
                    // DNP
                    {
                        -8, // RotX_DNP
                        -458 // RotY_DNP
                    },
                    // D65
                    {
                        135, // RotX_D65
                        -453 // RotY_D65
                    },
                    // DF
                    {
                        116, // RotX_DF
                        -568 // RotY_DF
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
                        556, // AWBGAIN_HOR_G
                        2070 // AWBGAIN_HOR_B
                    },
                    // A
                    {
                        604, // AWBGAIN_A_R
                        512, // AWBGAIN_A_G
                        1478 // AWBGAIN_A_B
                    },
                    // TL84
                    {
                        875, // AWBGAIN_TL84_R
                        512, // AWBGAIN_TL84_G
                        1299 // AWBGAIN_TL84_B
                    },
                    // CWF
                    {
                        1007, // AWBGAIN_CWF_R
                        512, // AWBGAIN_CWF_G
                        1277 // AWBGAIN_CWF_B
                    },
                    // DNP
                    {
                        971, // AWBGAIN_DNP_R
                        512, // AWBGAIN_DNP_G
                        932 // AWBGAIN_DNP_B
                    },
                    // D65
                    {
                        1158, // AWBGAIN_D65_R
                        512, // AWBGAIN_D65_G
                        756 // AWBGAIN_D65_B
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
                    3, // RotationAngle
                    256, // Cos
                    13 // Sin
                },
                // Daylight locus parameter	
                {
                    -139, // SlopeNumerator
                    128 // SlopeDenominator
                },
                // Predictor gain	
                {
                    101, // i4PrefRatio100
                    // DaylightLocus_L
                    {
                        1128, // i4R
                        530, // i4G
                        776 // i4B
                    },
                    // DaylightLocus_H
                    {
                        939, // i4R
                        512, // i4G
                        953 // i4B
                    },
                    // Temporal General
                    {
                        1158, // i4R
                        512, // i4G
                        756 // i4B
                    },
                    // AWB LSC
                    {
                        939, // i4R
                        512, // i4G
                        953 // i4B
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
                        -246, // TungRightBound
                        -939, // TungLeftBound
                        -374, // TungUpperBound
                        -486 // TungLowerBound
                    },
                    // Warm fluorescent
                    {
                        -246, // WFluoRightBound
                        -939, // WFluoLeftBound
                        -486, // WFluoUpperBound
                        -674 // WFluoLowerBound
                    },
                    // Fluorescent
                    {
                        -62, // FluoRightBound
                        -246, // FluoLeftBound
                        -393, // FluoUpperBound
                        -560 // FluoLowerBound
                    },
                    // CWF
                    {
                        -15, // CWFRightBound
                        -246, // CWFLeftBound
                        -560, // CWFUpperBound
                        -639 // CWFLowerBound
                    },
                    // Daylight
                    {
                        165, // DayRightBound
                        -62, // DayLeftBound
                        -393, // DayUpperBound
                        -560 // DayLowerBound
                    },
                    // Shade
                    {
                        495, // ShadeRightBound
                        165, // ShadeLeftBound
                        -393, // ShadeUpperBound
                        -505 // ShadeLowerBound
                    },
                    // Daylight Fluorescent	
                    {
                        165, // DFRightBound
                        -15, // DFLeftBound
                        -560, // DFUpperBound
                        -639 // DFLowerBound
                    }
                },
                // PWB light area
                {
                    // Reference area
                    {
                        495, // PRefRightBound
                        -939, // PRefLeftBound
                        -349, // PRefUpperBound
                        -674 // PRefLowerBound
                    },
                    // Daylight
                    {
                        190, // PDayRightBound
                        -62, // PDayLeftBound
                        -393, // PDayUpperBound
                        -560 // PDayLowerBound
                    },
                    // Cloudy daylight
                    {
                        290, // PCloudyRightBound
                        115, // PCloudyLeftBound
                        -393, // PCloudyUpperBound
                        -560 // PCloudyLowerBound
                    },
                    // Shade
                    {
                        390, // PShadeRightBound
                        115, // PShadeLeftBound
                        -393, // PShadeUpperBound
                        -560 // PShadeLowerBound
                    },
                    // Twilight
                    {
                        -62, // PTwiRightBound
                        -222, // PTwiLeftBound
                        -393, // PTwiUpperBound
                        -560 // PTwiLowerBound
                    },
                    // Fluorescent
                    {
                        185, // PFluoRightBound
                        -274, // PFluoLeftBound
                        -403, // PFluoUpperBound
                        -634 // PFluoLowerBound
                    },
                    // Warm fluorescent
                    {
                        -253, // PWFluoRightBound
                        -453, // PWFluoLeftBound
                        -403, // PWFluoUpperBound
                        -634 // PWFluoLowerBound
                    },
                    // Incandescent
                    {
                        -253, // PIncaRightBound
                        -453, // PIncaLeftBound
                        -393, // PIncaUpperBound
                        -560 // PIncaLowerBound
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
                        1093, // PWB_Day_R
                        512, // PWB_Day_G
                        861 // PWB_Day_B
                    },
                    // Cloudy daylight
                    {
                        1305, // PWB_Cloudy_R
                        512, // PWB_Cloudy_G
                        708 // PWB_Cloudy_B
                    },
                    // Shade
                    {
                        1391, // PWB_Shade_R
                        512, // PWB_Shade_G
                        659 // PWB_Shade_B
                    },
                    // Twilight
                    {
                        839, // PWB_Twi_R
                        512, // PWB_Twi_G
                        1154 // PWB_Twi_B
                    },
                    // Fluorescent
                    {
                        1009, // PWB_Fluo_R
                        512, // PWB_Fluo_G
                        1060 // PWB_Fluo_B
                    },
                    // Warm fluorescent
                    {
                        680, // PWB_WFluo_R
                        512, // PWB_WFluo_G
                        1642 // PWB_WFluo_B
                    },
                    // Incandescent
                    {
                        640, // PWB_Inca_R
                        512, // PWB_Inca_G
                        1556 // PWB_Inca_B
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
                        5397 // TUNG_OFFS
                    },
                    // Warm fluorescent
                    {
                        40, // WFluo_SLIDER
                        5397 // WFluo_OFFS
                    },
                    // Shade
                    {
                        50, // Shade_SLIDER
                        909 // Shade_OFFS
                    },
                    // Sunset Area
                    {
                        34, // i4Sunset_BoundXr_Thr
                        -458 // i4Sunset_BoundXr_Thr
                    },
                    // Shade F Area
                    {
                        -246, // i4Sunset_BoundXr_Thr
                        -539 // i4Sunset_BoundXr_Thr
                    },
                    // Shade F Vertex
                    {
                        -154, // i4Sunset_VertexXr_Thr
                        -550 // i4Sunset_VertexXr_Thr
                    },
                    // Shade CWF Area
                    {
                        -246, // i4Sunset_BoundXr_Thr
                        -588 // i4Sunset_BoundXr_Thr
                    },
                    // Shade CWF Vertex
                    {
                        -131, // i4Sunset_VertexXr_Thr
                        -614 // i4Sunset_VertexXr_Thr
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
                        -674,  // RotatedXCoordinate0
                        -488,  // RotatedXCoordinate1
                        -309,  // RotatedXCoordinate2
                        -143,  // RotatedXCoordinate3
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
                    115,  // i4InitLVThr_L
                    155,  // i4InitLVThr_H
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
                    {  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  5,  10,  10,  10,  10,  10,  10,  10,  10},  // 
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
                    { 100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,   75,   50,  25,   0,   0,   0}, // Tungsten
                    { 100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,   75,   50,   25,   25,  25,   0,   0,   0}, // Warm fluorescent
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
