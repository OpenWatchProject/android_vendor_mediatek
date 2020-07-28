#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>
#include <cutils/log.h>
#include <string.h>
#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"
#include "camera_AE_PLineTable_imx300mipiraw.h"
#include "camera_info_imx300mipiraw.h"
#include "camera_custom_AEPlinetable.h"
#include "camera_custom_tsf_tbl.h"
#include "AWB_Tuning_Para/camera_awb_tuning_para_imx300mipiraw.h"

#define NVRAM_TUNING_PARAM_NUM  6181001

const NVRAM_CAMERA_ISP_PARAM_STRUCT CAMERA_ISP_DEFAULT_VALUE =
{{
    //Version
    .Version= NVRAM_CAMERA_PARA_FILE_VERSION,

    //SensorId
    .SensorId= SENSOR_ID,
    .ISPComm={
      {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      }
    },
    .ISPRegs={
#include INCLUDE_FILENAME_ISP_REGS_PARAM
    },
    .ISPColorTbl= {
#include INCLUDE_FILENAME_ISP_COLORTBL_PARAM
    },
    .ISPToneMap={
#include INCLUDE_FILENAME_ISP_TONEMAP_PARAM
    },
    .DngMetadata=
    {
        0,  //i4RefereceIlluminant1
        3,  //i4RefereceIlluminant2
        {
            // rNoiseProfile[4]
            {
                {3.530980e-6, 3.357493e-4},
                {8.098436e-8, -2.336529e-4},
            },
            {
                {3.530980e-6, 3.357493e-4},
                {8.098436e-8, -2.336529e-4},
            },
            {
                {3.530980e-6, 3.357493e-4},
                {8.098436e-8, -2.336529e-4},
            },
            {
                {3.530980e-6, 3.357493e-4},
                {8.098436e-8, -2.336529e-4},
            },
        },
    }
}};

const NVRAM_CAMERA_3A_STRUCT CAMERA_3A_NVRAM_DEFAULT_VALUE =
{
    NVRAM_CAMERA_3A_FILE_VERSION, // u4Version
    SENSOR_ID, // SensorId

    // AE NVRAM
    {
        {
            // rDevicesInfo
            {
                1136,   // u4MinGain, 1024 base =  1x
                8192,  // u4MaxGain, 16x
                100,     // u4MiniISOGain, ISOxx
                16,    // u4GainStepUnit, 1x/8
                11345,     // u4PreExpUnit
                30,     // u4PreMaxFrameRate
                10040,     // u4VideoExpUnit
                30,     // u4VideoMaxFrameRate
                1024,   // u4Video2PreRatio, 1024 base = 1x
                10040,     // u4CapExpUnit
                24,     // u4CapMaxFrameRate
                1024,   // u4Cap2PreRatio, 1024 base = 1x
                10040,     // u4Video1ExpUnit
                120,     // u4Video1MaxFrameRate
                1024,   // u4Video12PreRatio, 1024 base = 1x
                10040,     // u4Video2ExpUnit
                60,     // u4Video2MaxFrameRate
                1024,   // u4Video22PreRatio, 1024 base = 1x
                11345,     // u4Custom1ExpUnit
                30,     // u4Custom1MaxFrameRate
                1024,   // u4Custom12PreRatio, 1024 base = 1x
                11345,     // u4Custom2ExpUnit
                30,     // u4Custom2MaxFrameRate
                1024,   // u4Custom22PreRatio, 1024 base = 1x
                11345,     // u4Custom3ExpUnit
                30,     // u4Custom3MaxFrameRate
                1024,   // u4Custom32PreRatio, 1024 base = 1x
                11345,     // u4Custom4ExpUnit
                30,     // u4Custom4MaxFrameRate
                1024,   // u4Custom42PreRatio, 1024 base = 1x
                11345,     // u4Custom5ExpUnit
                30,     // u4Custom5MaxFrameRate
                1024,   // u4Custom52PreRatio, 1024 base = 1x
                20,      // u4LensFno, Fno = 2.0
                350     // u4FocusLength_100x
            },
            // rHistConfig
            {
                4, // 2,   // u4HistHighThres
                40,  // u4HistLowThres
                2,   // u4MostBrightRatio
                1,   // u4MostDarkRatio
                160, // u4CentralHighBound
                20,  // u4CentralLowBound
                {240, 230, 220, 210, 200}, // u4OverExpThres[AE_CCT_STRENGTH_NUM]
                {62, 70, 82, 108, 141},  // u4HistStretchThres[AE_CCT_STRENGTH_NUM]
                {18, 22, 26, 30, 34}       // u4BlackLightThres[AE_CCT_STRENGTH_NUM]
            },
            // rCCTConfig
            {
                TRUE,            // bEnableBlackLight
                TRUE,            // bEnableHistStretch
                TRUE,           // bEnableAntiOverExposure
                TRUE,            // bEnableTimeLPF
                TRUE,            // bEnableCaptureThres
                TRUE,            // bEnableVideoThres
                TRUE,            // bEnableVideo1Thres
                TRUE,            // bEnableVideo2Thres
                TRUE,            // bEnableCustom1Thres
                TRUE,            // bEnableCustom2Thres
                TRUE,            // bEnableCustom3Thres
                TRUE,            // bEnableCustom4Thres
                TRUE,            // bEnableCustom5Thres
                TRUE,            // bEnableStrobeThres
                47,                // u4AETarget
                47,                // u4StrobeAETarget

                70,                // u4InitIndex
                4,                 // u4BackLightWeight
                32,                // u4HistStretchWeight
                4,                 // u4AntiOverExpWeight
                2,                 // u4BlackLightStrengthIndex
                2,                 // u4HistStretchStrengthIndex
                2,                 // u4AntiOverExpStrengthIndex
                2,                 // u4TimeLPFStrengthIndex
                {1, 3, 5, 7, 8}, // u4LPFConvergeTable[AE_CCT_STRENGTH_NUM]
                90,                // u4InDoorEV = 9.0, 10 base
                -20,    // i4BVOffset delta BV = value/10
                64,                 // u4PreviewFlareOffset
                64,                 // u4CaptureFlareOffset
                10,                 // u4CaptureFlareThres
                64,                 // u4VideoFlareOffset
                10,                 // u4VideoFlareThres
                64,               // u4CustomFlareOffset
                10,                 //  u4CustomFlareThres
                64,                 // u4StrobeFlareOffset //12 bits
                3,                 // u4StrobeFlareThres // 0.5%
                98,                 // u4PrvMaxFlareThres //12 bit
                0,                 // u4PrvMinFlareThres
                160,                 // u4VideoMaxFlareThres // 12 bit
                0,                 // u4VideoMinFlareThres
                18,                // u4FlatnessThres              // 10 base for flatness condition.
                75,    // u4FlatnessStrength
                //rMeteringSpec
                {
                    //rHS_Spec
                    {
                        TRUE,//bEnableHistStretch           // enable histogram stretch
                        1024,//u4HistStretchWeight          // Histogram weighting value
                        60, //50, //20,//u4Pcent                      // 1%=10, 0~1000
                        160, //166,//176,//u4Thd                        // 0~255
                        75, //54, //74,//u4FlatThd                    // 0~255

                        120,//u4FlatBrightPcent
                        120,//u4FlatDarkPcent
                        //sFlatRatio
                        {
                            1000,  //i4X1
                            1024,  //i4Y1
                            2400, //i4X2
                            0     //i4Y2
                        },
                        FALSE, //bEnableGreyTextEnhance
                        1800, //u4GreyTextFlatStart, > sFlatRatio.i4X1, < sFlatRatio.i4X2
                        {
                            10,     //i4X1
                            1024,   //i4Y1
                            80,     //i4X2
                            0       //i4Y2
                        }
                    },
                    //rAOE_Spec
                    {
                        TRUE, //bEnableAntiOverExposure
                        1024, //u4AntiOverExpWeight
                        10,    //u4Pcent
                      220,//  200,  //u4Thd

                        TRUE, //bEnableCOEP
                        1,    //u4COEPcent
                        106,  //u4COEThd
                        0,  // u4BVCompRatio
                        //sCOEYRatio;     // the outer y ratio
                        {
                            23,   //i4X1
                            1024,  //i4Y1
                            47,   //i4X2
                            0     //i4Y2
                        },
                        //sCOEDiffRatio;  // inner/outer y difference ratio
                        {
                            1500, //i4X1
                            0,    //i4Y1
                            2100, //i4X2
                            1024   //i4Y2
                        }
                    },
                    //rABL_Spec
                    {
                        TRUE,//bEnableBlackLigh
                        1024,//u4BackLightWeigh
                        500,//u4Pcent
                        22,//u4Thd,
                        255, // center luminance
                        180, //256, // final target limitation, 256/128 = 2x
                        //sFgBgEVRatio
                        {
                            2100,//2200, //i4X1
                            0,    //i4Y1
                            4000, //i4X2
                            1024   //i4Y2
                        },
                        //sBVRatio
                        {
                            3800,//i4X1
                            0,   //i4Y1
                            5000,//i4X2
                            1024  //i4Y2
                        }
                    },
                    //rNS_Spec
                    {
                        TRUE, // bEnableNightScene
                        10, //5,    //u4Pcent
                       150, // 170,  //u4Thd
                        55, //52,   //u4FlatThd

    					          180, //  200,  //u4BrightTonePcent
    				            55, //85,//	82, //  95, //u4BrightToneThd

                        500,  //u4LowBndPcent
                        5,    //u4LowBndThd
                        26,    //u4LowBndThdLimit

                        50,  //u4FlatBrightPcent;
                        300,   //u4FlatDarkPcent;
                        //sFlatRatio
                        {
                            1200, //i4X1
                            1024, //i4Y1
                          2800,//  2400, //i4X2
                            0    //i4Y2
                        },
                        //sBVRatio
                        {
                            -500, //i4X1
                            1024,  //i4Y1
                            3000, //i4X2
                            0     //i4Y2
                        },
                        TRUE, // bEnableNightSkySuppresion
                        //sSkyBVRatio
                        {
                            -4000, //i4X1
                            1024, //i4X2
                            -2000,  //i4Y1
                            0     //i4Y2
                        }
                    },
                    // rTOUCHFD_Spec
                    {
                        40, //uMeteringYLowBound;
                        50, //uMeteringYHighBound;
                        40, //uFaceYLowBound;
                        50, //uFaceYHighBound;
                        3,  //uFaceCentralWeight;
                        120,//u4MeteringStableMax;
                        80, //u4MeteringStableMin;
                    }
                }, //End rMeteringSpec
                // rFlareSpec
                {
                    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, //uPrvFlareWeightArr[16];
                    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, //uVideoFlareWeightArr[16];
                    96,                                               //u4FlareStdThrHigh;
                    48,                                               //u4FlareStdThrLow;
                    0,                                                //u4PrvCapFlareDiff;
                    2,                                                //u4FlareMaxStepGap_Fast;
                    0,//0,                                                //u4FlareMaxStepGap_Slow;
                    1800,                                             //u4FlarMaxStepGapLimitBV;
                    2,//0,                                                //u4FlareAEStableCount;
                },
                //rAEMoveRatio =
                {
                    100,//100, //u4SpeedUpRatio 500
                    100, //100, //u4GlobalRatio
                    190, //u4Bright2TargetEnd  190 150
                    10,//20,   //u4Dark2TargetStart
                    90, //u4B2TEnd
                    85,//70,  //u4B2TStart 70
                    70,//60,  //u4D2TEnd 60
                    85,  //u4D2TStart
                },

                //rAEVideoMoveRatio =
                {
                    100, //u4SpeedUpRatio
                    100, //u4GlobalRatio
                    150,  //u4Bright2TargetEnd
                    20,    //u4Dark2TargetStart
                    90, //u4B2TEnd
                    40,  //u4B2TStart
                    30,  //u4D2TEnd
                    90,  //u4D2TStart
                },

                //rAEVideo1MoveRatio =
                {
                    100, //u4SpeedUpRatio
                    100, //u4GlobalRatio
                    150,  //u4Bright2TargetEnd
                    20,    //u4Dark2TargetStart
                    90, //u4B2TEnd
                    40,  //u4B2TStart
                    30,  //u4D2TEnd
                    90,  //u4D2TStart
                },

                //rAEVideo2MoveRatio =
                {
                    100, //u4SpeedUpRatio
                    100, //u4GlobalRatio
                    150,  //u4Bright2TargetEnd
                    20,    //u4Dark2TargetStart
                    90, //u4B2TEnd
                    40,  //u4B2TStart
                    30,  //u4D2TEnd
                    90,  //u4D2TStart
                },

                //rAECustom1MoveRatio =
                {
                    100, //u4SpeedUpRatio
                    100, //u4GlobalRatio
                    150,  //u4Bright2TargetEnd
                    20,    //u4Dark2TargetStart
                    90, //u4B2TEnd
                    40,  //u4B2TStart
                    30,  //u4D2TEnd
                    90,  //u4D2TStart
                },

                //rAECustom2MoveRatio =
                {
                    100, //u4SpeedUpRatio
                    100, //u4GlobalRatio
                    150,  //u4Bright2TargetEnd
                    20,    //u4Dark2TargetStart
                    90, //u4B2TEnd
                    40,  //u4B2TStart
                    30,  //u4D2TEnd
                    90,  //u4D2TStart
                },

                //rAECustom3MoveRatio =
                {
                    100, //u4SpeedUpRatio
                    100, //u4GlobalRatio
                    150,  //u4Bright2TargetEnd
                    20,    //u4Dark2TargetStart
                    90, //u4B2TEnd
                    40,  //u4B2TStart
                    30,  //u4D2TEnd
                    90,  //u4D2TStart
                },

                //rAECustom4MoveRatio =
                {
                    100, //u4SpeedUpRatio
                    100, //u4GlobalRatio
                    150,  //u4Bright2TargetEnd
                    20,    //u4Dark2TargetStart
                    90, //u4B2TEnd
                    40,  //u4B2TStart
                    30,  //u4D2TEnd
                    90,  //u4D2TStart
                },

                //rAECustom5MoveRatio =
                {
                    100, //u4SpeedUpRatio
                    100, //u4GlobalRatio
                    150,  //u4Bright2TargetEnd
                    20,    //u4Dark2TargetStart
                    90, //u4B2TEnd
                    40,  //u4B2TStart
                    30,  //u4D2TEnd
                    90,  //u4D2TStart
                },

                //rAEFaceMoveRatio =
                {
                    100, //u4SpeedUpRatio
                    100, //u4GlobalRatio
                    190,  //u4Bright2TargetEnd
                    10,    //u4Dark2TargetStart
                    80, //u4B2TEnd
                    30,  //u4B2TStart
                    20,  //u4D2TEnd
                    60,  //u4D2TStart
                },

                //rAETrackingMoveRatio =
                {
                    100, //u4SpeedUpRatio
                    100, //u4GlobalRatio
                    190,  //u4Bright2TargetEnd
                    10,    //u4Dark2TargetStart
                    80, //u4B2TEnd
                    30,  //u4B2TStart
                    20,  //u4D2TEnd
                    60,  //u4D2TStart
                },
                //rAEAOENVRAMParam =
                {
                    1,      // i4AOEStrengthIdx: 0 / 1 / 2
                    130,    // u4BVCompRatio
                    {
                        {
                            28,  //u4Y_Target
                            25,  //u4AOE_OE_percent
                            210,  //u4AOE_OEBound
                            10,    //u4AOE_DarkBound
                            950,    //u4AOE_LowlightPrecent
                            1,    //u4AOE_LowlightBound
                            145,    //u4AOESceneLV_L
                            180,    //u4AOESceneLV_H
                            40,    //u4AOE_SWHdrLE_Bound
                        },
                        {
                            28,  //u4Y_Target
                            25,  //u4AOE_OE_percent
                            210,  //u4AOE_OEBound
                            15, //20,    //u4AOE_DarkBound
                            950,    //u4AOE_LowlightPrecent
                            3, //10,    //u4AOE_LowlightBound
                            145,    //u4AOESceneLV_L
                            180,    //u4AOESceneLV_H
                            40,    //u4AOE_SWHdrLE_Bound
                        },
                        {
                            28,  //u4Y_Target
                            25,  //u4AOE_OE_percent
                            210,  //u4AOE_OEBound
                            25,    //u4AOE_DarkBound
                            950,    //u4AOE_LowlightPrecent
                            8,    //u4AOE_LowlightBound
                            145,    //u4AOESceneLV_L
                            180,    //u4AOESceneLV_H
                            40,    //u4AOE_SWHdrLE_Bound
                        }
                    }
                }
            },
            // rAEHDRConfig
            {
                3072,   // i4RMGSeg
                768,    // i4RMGTurn
                35,     // i4HDRTarget_L;
                40,     // i4HDRTarget_H;
                100,    // i4HDRTargetLV_L;
                120,    // i4HDRTargetLV_H;
                20,     // i4OverExpoRatio;
                212,    // i4OverExpoTarget;
                120,    // i4OverExpoLV_L;
                180,    // i4OverExpoLV_H;
                4,      // i4UnderExpoContrastThr;
                {
                 // Contrast:
                 //  0   1   2   3   4   5   6   7   8   9  10
                     3,  3,  3,  3,  3,  2,  1,  1,  1,  1,  1    // i4UnderExpoTargetTbl[AE_HDR_UNDEREXPO_CONTRAST_TARGET_TBL_NUM];
                },
                950,        // i4UnderExpoRatio;
                1000,       // i4AvgExpoRatio;
                8, //10,       // i4AvgExpoTarget;
                768,  // i4HDRAESpeed
                2,          // i4HDRAEConvergeThr;
                40,         // i4SWHdrLEThr
                20,         // i4SWHdrSERatio
                180,        // i4SWHdrSETarget
                1024        // i4SWHdrBaseGain
            }
        },
        {   /****  HDR ON MODE  ****/
            // rDevicesInfo
            {
                1136,   // u4MinGain, 1024 base =  1x
                8192,  // u4MaxGain, 16x
                100,     // u4MiniISOGain, ISOxx
                16,    // u4GainStepUnit, 1x/8
                11345,     // u4PreExpUnit
                30,     // u4PreMaxFrameRate
                10040,     // u4VideoExpUnit
                30,     // u4VideoMaxFrameRate
                1024,   // u4Video2PreRatio, 1024 base = 1x
                10040,     // u4CapExpUnit
                24,     // u4CapMaxFrameRate
                1024,   // u4Cap2PreRatio, 1024 base = 1x
                10040,     // u4Video1ExpUnit
                120,     // u4Video1MaxFrameRate
                1024,   // u4Video12PreRatio, 1024 base = 1x
                10040,     // u4Video2ExpUnit
                60,     // u4Video2MaxFrameRate
                1024,   // u4Video22PreRatio, 1024 base = 1x
                11345,     // u4Custom1ExpUnit
                30,     // u4Custom1MaxFrameRate
                1024,   // u4Custom12PreRatio, 1024 base = 1x
                11345,     // u4Custom2ExpUnit
                30,     // u4Custom2MaxFrameRate
                1024,   // u4Custom22PreRatio, 1024 base = 1x
                11345,     // u4Custom3ExpUnit
                30,     // u4Custom3MaxFrameRate
                1024,   // u4Custom32PreRatio, 1024 base = 1x
                11345,     // u4Custom4ExpUnit
                30,     // u4Custom4MaxFrameRate
                1024,   // u4Custom42PreRatio, 1024 base = 1x
                11345,     // u4Custom5ExpUnit
                30,     // u4Custom5MaxFrameRate
                1024,   // u4Custom52PreRatio, 1024 base = 1x
                20,      // u4LensFno, Fno = 2.0
                350     // u4FocusLength_100x
            },
            // rHistConfig
            {
                4, // 2,   // u4HistHighThres
                40,  // u4HistLowThres
                2,   // u4MostBrightRatio
                1,   // u4MostDarkRatio
                160, // u4CentralHighBound
                20,  // u4CentralLowBound
                {240, 230, 220, 210, 200}, // u4OverExpThres[AE_CCT_STRENGTH_NUM]
                {62, 70, 82, 108, 141},  // u4HistStretchThres[AE_CCT_STRENGTH_NUM]
                {18, 22, 26, 30, 34}       // u4BlackLightThres[AE_CCT_STRENGTH_NUM]
            },
            // rCCTConfig
            {
                TRUE,            // bEnableBlackLight
                TRUE,            // bEnableHistStretch
                TRUE,           // bEnableAntiOverExposure
                TRUE,            // bEnableTimeLPF
                TRUE,            // bEnableCaptureThres
                TRUE,            // bEnableVideoThres
                TRUE,            // bEnableVideo1Thres
                TRUE,            // bEnableVideo2Thres
                TRUE,            // bEnableCustom1Thres
                TRUE,            // bEnableCustom2Thres
                TRUE,            // bEnableCustom3Thres
                TRUE,            // bEnableCustom4Thres
                TRUE,            // bEnableCustom5Thres
                TRUE,            // bEnableStrobeThres
                52,                // u4AETarget
                52,                // u4StrobeAETarget

                70,                // u4InitIndex
                4,                 // u4BackLightWeight
                32,                // u4HistStretchWeight
                4,                 // u4AntiOverExpWeight
                2,                 // u4BlackLightStrengthIndex
                2,                 // u4HistStretchStrengthIndex
                2,                 // u4AntiOverExpStrengthIndex
                2,                 // u4TimeLPFStrengthIndex
                {1, 3, 5, 7, 8}, // u4LPFConvergeTable[AE_CCT_STRENGTH_NUM]
                90,                // u4InDoorEV = 9.0, 10 base
                -20,    // i4BVOffset delta BV = value/10
                64,                 // u4PreviewFlareOffset
                64,                 // u4CaptureFlareOffset
                3,                 // u4CaptureFlareThres
                64,                 // u4VideoFlareOffset
                3,                 // u4VideoFlareThres
                64,               // u4CustomFlareOffset
                3,                 //  u4CustomFlareThres
                64,                 // u4StrobeFlareOffset //12 bits
                3,                 // u4StrobeFlareThres // 0.5%
                160,                 // u4PrvMaxFlareThres //12 bit
                0,                 // u4PrvMinFlareThres
                160,                 // u4VideoMaxFlareThres // 12 bit
                0,                 // u4VideoMinFlareThres
                18,                // u4FlatnessThres              // 10 base for flatness condition.
                75,    // u4FlatnessStrength
                //rMeteringSpec
                {
                    //rHS_Spec
                    {
                        TRUE,//bEnableHistStretch           // enable histogram stretch
                        1024,//u4HistStretchWeight          // Histogram weighting value
                        40, //50, //20,//u4Pcent                      // 1%=10, 0~1000
                        160, //166,//176,//u4Thd                        // 0~255
                        75, //54, //74,//u4FlatThd                    // 0~255

                        120,//u4FlatBrightPcent
                        120,//u4FlatDarkPcent
                        //sFlatRatio
                        {
                            1000,  //i4X1
                            1024,  //i4Y1
                            2400, //i4X2
                            0     //i4Y2
                        },
                        TRUE, //bEnableGreyTextEnhance
                        1800, //u4GreyTextFlatStart, > sFlatRatio.i4X1, < sFlatRatio.i4X2
                        {
                            10,     //i4X1
                            1024,   //i4Y1
                            80,     //i4X2
                            0       //i4Y2
                        }
                    },
                    //rAOE_Spec
                    {
                        TRUE, //bEnableAntiOverExposure
                        1024, //u4AntiOverExpWeight
                        10,    //u4Pcent
                      220,//  200,  //u4Thd

                        TRUE, //bEnableCOEP
                        1,    //u4COEPcent
                        106,  //u4COEThd
                        0,  // u4BVCompRatio
                        //sCOEYRatio;     // the outer y ratio
                        {
                            23,   //i4X1
                            1024,  //i4Y1
                            47,   //i4X2
                            0     //i4Y2
                        },
                        //sCOEDiffRatio;  // inner/outer y difference ratio
                        {
                            1500, //i4X1
                            0,    //i4Y1
                            2100, //i4X2
                            1024   //i4Y2
                        }
                    },
                    //rABL_Spec
                    {
                        TRUE,//bEnableBlackLigh
                        1024,//u4BackLightWeigh
                        400,//u4Pcent
                        22,//u4Thd,
                        255, // center luminance
                        246, //256, // final target limitation, 256/128 = 2x
                        //sFgBgEVRatio
                        {
                            2100,//2200, //i4X1
                            0,    //i4Y1
                            4000, //i4X2
                            1024   //i4Y2
                        },
                        //sBVRatio
                        {
                            3800,//i4X1
                            0,   //i4Y1
                            5000,//i4X2
                            1024  //i4Y2
                        }
                    },
                    //rNS_Spec
                    {
                        TRUE, // bEnableNightScene
                        10, //5,    //u4Pcent
                       150, // 170,  //u4Thd
                        72, //52,   //u4FlatThd

    					          180, //  200,  //u4BrightTonePcent
    				            80, //85,//	82, //  95, //u4BrightToneThd

                        500,  //u4LowBndPcent
                        5,    //u4LowBndThd
                        26,    //u4LowBndThdLimit

                        50,  //u4FlatBrightPcent;
                        300,   //u4FlatDarkPcent;
                        //sFlatRatio
                        {
                            1200, //i4X1
                            1024, //i4Y1
                          2800,//  2400, //i4X2
                            0    //i4Y2
                        },
                        //sBVRatio
                        {
                            -500, //i4X1
                            1024,  //i4Y1
                            3000, //i4X2
                            0     //i4Y2
                        },
                        TRUE, // bEnableNightSkySuppresion
                        //sSkyBVRatio
                        {
                            -4000, //i4X1
                            1024, //i4X2
                            -2000,  //i4Y1
                            0     //i4Y2
                        }
                    },
                    // rTOUCHFD_Spec
                    {
                        40, //uMeteringYLowBound;
                        50, //uMeteringYHighBound;
                        40, //uFaceYLowBound;
                        50, //uFaceYHighBound;
                        3,  //uFaceCentralWeight;
                        120,//u4MeteringStableMax;
                        80, //u4MeteringStableMin;
                    }
                }, //End rMeteringSpec
                // rFlareSpec
                {
                    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, //uPrvFlareWeightArr[16];
                    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, //uVideoFlareWeightArr[16];
                    96,                                               //u4FlareStdThrHigh;
                    48,                                               //u4FlareStdThrLow;
                    0,                                                //u4PrvCapFlareDiff;
                    2,                                                //u4FlareMaxStepGap_Fast;
                    0,//0,                                                //u4FlareMaxStepGap_Slow;
                    1800,                                             //u4FlarMaxStepGapLimitBV;
                    2,//0,                                                //u4FlareAEStableCount;
                },
                //rAEMoveRatio =
                {
                    100,//100, //u4SpeedUpRatio 500
                    100, //100, //u4GlobalRatio
                    190, //u4Bright2TargetEnd  190 150
                    10,//20,   //u4Dark2TargetStart
                    90, //u4B2TEnd
                    85,//70,  //u4B2TStart 70
                    70,//60,  //u4D2TEnd 60
                    85,  //u4D2TStart
                },

                //rAEVideoMoveRatio =
                {
                    100, //u4SpeedUpRatio
                    100, //u4GlobalRatio
                    150,  //u4Bright2TargetEnd
                    20,    //u4Dark2TargetStart
                    90, //u4B2TEnd
                    40,  //u4B2TStart
                    30,  //u4D2TEnd
                    90,  //u4D2TStart
                },

                //rAEVideo1MoveRatio =
                {
                    100, //u4SpeedUpRatio
                    100, //u4GlobalRatio
                    150,  //u4Bright2TargetEnd
                    20,    //u4Dark2TargetStart
                    90, //u4B2TEnd
                    40,  //u4B2TStart
                    30,  //u4D2TEnd
                    90,  //u4D2TStart
                },

                //rAEVideo2MoveRatio =
                {
                    100, //u4SpeedUpRatio
                    100, //u4GlobalRatio
                    150,  //u4Bright2TargetEnd
                    20,    //u4Dark2TargetStart
                    90, //u4B2TEnd
                    40,  //u4B2TStart
                    30,  //u4D2TEnd
                    90,  //u4D2TStart
                },

                //rAECustom1MoveRatio =
                {
                    100, //u4SpeedUpRatio
                    100, //u4GlobalRatio
                    150,  //u4Bright2TargetEnd
                    20,    //u4Dark2TargetStart
                    90, //u4B2TEnd
                    40,  //u4B2TStart
                    30,  //u4D2TEnd
                    90,  //u4D2TStart
                },

                //rAECustom2MoveRatio =
                {
                    100, //u4SpeedUpRatio
                    100, //u4GlobalRatio
                    150,  //u4Bright2TargetEnd
                    20,    //u4Dark2TargetStart
                    90, //u4B2TEnd
                    40,  //u4B2TStart
                    30,  //u4D2TEnd
                    90,  //u4D2TStart
                },

                //rAECustom3MoveRatio =
                {
                    100, //u4SpeedUpRatio
                    100, //u4GlobalRatio
                    150,  //u4Bright2TargetEnd
                    20,    //u4Dark2TargetStart
                    90, //u4B2TEnd
                    40,  //u4B2TStart
                    30,  //u4D2TEnd
                    90,  //u4D2TStart
                },

                //rAECustom4MoveRatio =
                {
                    100, //u4SpeedUpRatio
                    100, //u4GlobalRatio
                    150,  //u4Bright2TargetEnd
                    20,    //u4Dark2TargetStart
                    90, //u4B2TEnd
                    40,  //u4B2TStart
                    30,  //u4D2TEnd
                    90,  //u4D2TStart
                },

                //rAECustom5MoveRatio =
                {
                    100, //u4SpeedUpRatio
                    100, //u4GlobalRatio
                    150,  //u4Bright2TargetEnd
                    20,    //u4Dark2TargetStart
                    90, //u4B2TEnd
                    40,  //u4B2TStart
                    30,  //u4D2TEnd
                    90,  //u4D2TStart
                },

                //rAEFaceMoveRatio =
                {
                    100, //u4SpeedUpRatio
                    100, //u4GlobalRatio
                    190,  //u4Bright2TargetEnd
                    10,    //u4Dark2TargetStart
                    80, //u4B2TEnd
                    30,  //u4B2TStart
                    20,  //u4D2TEnd
                    60,  //u4D2TStart
                },

                //rAETrackingMoveRatio =
                {
                    100, //u4SpeedUpRatio
                    100, //u4GlobalRatio
                    190,  //u4Bright2TargetEnd
                    10,    //u4Dark2TargetStart
                    80, //u4B2TEnd
                    30,  //u4B2TStart
                    20,  //u4D2TEnd
                    60,  //u4D2TStart
                },
                //rAEAOENVRAMParam =
                {
                    1,      // i4AOEStrengthIdx: 0 / 1 / 2
                    130,    // u4BVCompRatio
                    {
                        {
                            28,  //u4Y_Target
                            25,  //u4AOE_OE_percent
                            210,  //u4AOE_OEBound
                            10,    //u4AOE_DarkBound
                            950,    //u4AOE_LowlightPrecent
                            1,    //u4AOE_LowlightBound
                            145,    //u4AOESceneLV_L
                            180,    //u4AOESceneLV_H
                            40,    //u4AOE_SWHdrLE_Bound
                        },
                        {
                            28,  //u4Y_Target
                            25,  //u4AOE_OE_percent
                            210,  //u4AOE_OEBound
                            15, //20,    //u4AOE_DarkBound
                            950,    //u4AOE_LowlightPrecent
                            3, //10,    //u4AOE_LowlightBound
                            145,    //u4AOESceneLV_L
                            180,    //u4AOESceneLV_H
                            40,    //u4AOE_SWHdrLE_Bound
                        },
                        {
                            28,  //u4Y_Target
                            25,  //u4AOE_OE_percent
                            210,  //u4AOE_OEBound
                            25,    //u4AOE_DarkBound
                            950,    //u4AOE_LowlightPrecent
                            8,    //u4AOE_LowlightBound
                            145,    //u4AOESceneLV_L
                            180,    //u4AOESceneLV_H
                            40,    //u4AOE_SWHdrLE_Bound
                        }
                    }
                }
            },
            // rAEHDRConfig
            {
                3072,   // i4RMGSeg
                768,    // i4RMGTurn
                35,     // i4HDRTarget_L;
                40,     // i4HDRTarget_H;
                100,    // i4HDRTargetLV_L;
                120,    // i4HDRTargetLV_H;
                20,     // i4OverExpoRatio;
                212,    // i4OverExpoTarget;
                120,    // i4OverExpoLV_L;
                180,    // i4OverExpoLV_H;
                4,      // i4UnderExpoContrastThr;
                {
                 // Contrast:
                 //  0   1   2   3   4   5   6   7   8   9  10
                     3,  3,  3,  3,  3,  2,  1,  1,  1,  1,  1    // i4UnderExpoTargetTbl[AE_HDR_UNDEREXPO_CONTRAST_TARGET_TBL_NUM];
                },
                950,        // i4UnderExpoRatio;
                1000,       // i4AvgExpoRatio;
                8, //10,       // i4AvgExpoTarget;
                768,  // i4HDRAESpeed
                2,          // i4HDRAEConvergeThr;
                40,         // i4SWHdrLEThr
                20,         // i4SWHdrSERatio
                180,        // i4SWHdrSETarget
                1024        // i4SWHdrBaseGain
            }
        },
        {
            // rDevicesInfo
            {
                1136,   // u4MinGain, 1024 base =  1x
                8192,  // u4MaxGain, 16x
                100,     // u4MiniISOGain, ISOxx
                16,    // u4GainStepUnit, 1x/8
                11345,     // u4PreExpUnit
                30,     // u4PreMaxFrameRate
                10040,     // u4VideoExpUnit
                30,     // u4VideoMaxFrameRate
                1024,   // u4Video2PreRatio, 1024 base = 1x
                10040,     // u4CapExpUnit
                24,     // u4CapMaxFrameRate
                1024,   // u4Cap2PreRatio, 1024 base = 1x
                10040,     // u4Video1ExpUnit
                120,     // u4Video1MaxFrameRate
                1024,   // u4Video12PreRatio, 1024 base = 1x
                10040,     // u4Video2ExpUnit
                60,     // u4Video2MaxFrameRate
                1024,   // u4Video22PreRatio, 1024 base = 1x
                11345,     // u4Custom1ExpUnit
                30,     // u4Custom1MaxFrameRate
                1024,   // u4Custom12PreRatio, 1024 base = 1x
                11345,     // u4Custom2ExpUnit
                30,     // u4Custom2MaxFrameRate
                1024,   // u4Custom22PreRatio, 1024 base = 1x
                11345,     // u4Custom3ExpUnit
                30,     // u4Custom3MaxFrameRate
                1024,   // u4Custom32PreRatio, 1024 base = 1x
                11345,     // u4Custom4ExpUnit
                30,     // u4Custom4MaxFrameRate
                1024,   // u4Custom42PreRatio, 1024 base = 1x
                11345,     // u4Custom5ExpUnit
                30,     // u4Custom5MaxFrameRate
                1024,   // u4Custom52PreRatio, 1024 base = 1x
                20,      // u4LensFno, Fno = 2.0
                350     // u4FocusLength_100x
            },
            // rHistConfig
            {
                4, // 2,   // u4HistHighThres
                40,  // u4HistLowThres
                2,   // u4MostBrightRatio
                1,   // u4MostDarkRatio
                160, // u4CentralHighBound
                20,  // u4CentralLowBound
                {240, 230, 220, 210, 200}, // u4OverExpThres[AE_CCT_STRENGTH_NUM]
                {62, 70, 82, 108, 141},  // u4HistStretchThres[AE_CCT_STRENGTH_NUM]
                {18, 22, 26, 30, 34}       // u4BlackLightThres[AE_CCT_STRENGTH_NUM]
            },
            // rCCTConfig
            {
                TRUE,            // bEnableBlackLight
                TRUE,            // bEnableHistStretch
                TRUE,           // bEnableAntiOverExposure
                TRUE,            // bEnableTimeLPF
                TRUE,            // bEnableCaptureThres
                TRUE,            // bEnableVideoThres
                TRUE,            // bEnableVideo1Thres
                TRUE,            // bEnableVideo2Thres
                TRUE,            // bEnableCustom1Thres
                TRUE,            // bEnableCustom2Thres
                TRUE,            // bEnableCustom3Thres
                TRUE,            // bEnableCustom4Thres
                TRUE,            // bEnableCustom5Thres
                TRUE,            // bEnableStrobeThres
                52,                // u4AETarget
                52,                // u4StrobeAETarget

                70,                // u4InitIndex
                4,                 // u4BackLightWeight
                32,                // u4HistStretchWeight
                4,                 // u4AntiOverExpWeight
                2,                 // u4BlackLightStrengthIndex
                2,                 // u4HistStretchStrengthIndex
                2,                 // u4AntiOverExpStrengthIndex
                2,                 // u4TimeLPFStrengthIndex
                {1, 3, 5, 7, 8}, // u4LPFConvergeTable[AE_CCT_STRENGTH_NUM]
                90,                // u4InDoorEV = 9.0, 10 base
                -20,    // i4BVOffset delta BV = value/10
                64,                 // u4PreviewFlareOffset
                64,                 // u4CaptureFlareOffset
                3,                 // u4CaptureFlareThres
                64,                 // u4VideoFlareOffset
                3,                 // u4VideoFlareThres
                64,               // u4CustomFlareOffset
                3,                 //  u4CustomFlareThres
                64,                 // u4StrobeFlareOffset //12 bits
                3,                 // u4StrobeFlareThres // 0.5%
                160,                 // u4PrvMaxFlareThres //12 bit
                0,                 // u4PrvMinFlareThres
                160,                 // u4VideoMaxFlareThres // 12 bit
                0,                 // u4VideoMinFlareThres
                18,                // u4FlatnessThres              // 10 base for flatness condition.
                75,    // u4FlatnessStrength
                //rMeteringSpec
                {
                    //rHS_Spec
                    {
                        TRUE,//bEnableHistStretch           // enable histogram stretch
                        1024,//u4HistStretchWeight          // Histogram weighting value
                        40, //50, //20,//u4Pcent                      // 1%=10, 0~1000
                        160, //166,//176,//u4Thd                        // 0~255
                        75, //54, //74,//u4FlatThd                    // 0~255

                        120,//u4FlatBrightPcent
                        120,//u4FlatDarkPcent
                        //sFlatRatio
                        {
                            1000,  //i4X1
                            1024,  //i4Y1
                            2400, //i4X2
                            0     //i4Y2
                        },
                        TRUE, //bEnableGreyTextEnhance
                        1800, //u4GreyTextFlatStart, > sFlatRatio.i4X1, < sFlatRatio.i4X2
                        {
                            10,     //i4X1
                            1024,   //i4Y1
                            80,     //i4X2
                            0       //i4Y2
                        }
                    },
                    //rAOE_Spec
                    {
                        TRUE, //bEnableAntiOverExposure
                        1024, //u4AntiOverExpWeight
                        10,    //u4Pcent
                      220,//  200,  //u4Thd

                        TRUE, //bEnableCOEP
                        1,    //u4COEPcent
                        106,  //u4COEThd
                        0,  // u4BVCompRatio
                        //sCOEYRatio;     // the outer y ratio
                        {
                            23,   //i4X1
                            1024,  //i4Y1
                            47,   //i4X2
                            0     //i4Y2
                        },
                        //sCOEDiffRatio;  // inner/outer y difference ratio
                        {
                            1500, //i4X1
                            0,    //i4Y1
                            2100, //i4X2
                            1024   //i4Y2
                        }
                    },
                    //rABL_Spec
                    {
                        TRUE,//bEnableBlackLigh
                        1024,//u4BackLightWeigh
                        400,//u4Pcent
                        22,//u4Thd,
                        255, // center luminance
                        246, //256, // final target limitation, 256/128 = 2x
                        //sFgBgEVRatio
                        {
                            2100,//2200, //i4X1
                            0,    //i4Y1
                            4000, //i4X2
                            1024   //i4Y2
                        },
                        //sBVRatio
                        {
                            3800,//i4X1
                            0,   //i4Y1
                            5000,//i4X2
                            1024  //i4Y2
                        }
                    },
                    //rNS_Spec
                    {
                        TRUE, // bEnableNightScene
                        10, //5,    //u4Pcent
                       150, // 170,  //u4Thd
                        72, //52,   //u4FlatThd

    					          180, //  200,  //u4BrightTonePcent
    				            80, //85,//	82, //  95, //u4BrightToneThd

                        500,  //u4LowBndPcent
                        5,    //u4LowBndThd
                        26,    //u4LowBndThdLimit

                        50,  //u4FlatBrightPcent;
                        300,   //u4FlatDarkPcent;
                        //sFlatRatio
                        {
                            1200, //i4X1
                            1024, //i4Y1
                          2800,//  2400, //i4X2
                            0    //i4Y2
                        },
                        //sBVRatio
                        {
                            -500, //i4X1
                            1024,  //i4Y1
                            3000, //i4X2
                            0     //i4Y2
                        },
                        TRUE, // bEnableNightSkySuppresion
                        //sSkyBVRatio
                        {
                            -4000, //i4X1
                            1024, //i4X2
                            -2000,  //i4Y1
                            0     //i4Y2
                        }
                    },
                    // rTOUCHFD_Spec
                    {
                        40, //uMeteringYLowBound;
                        50, //uMeteringYHighBound;
                        40, //uFaceYLowBound;
                        50, //uFaceYHighBound;
                        3,  //uFaceCentralWeight;
                        120,//u4MeteringStableMax;
                        80, //u4MeteringStableMin;
                    }
                }, //End rMeteringSpec
                // rFlareSpec
                {
                    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, //uPrvFlareWeightArr[16];
                    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, //uVideoFlareWeightArr[16];
                    96,                                               //u4FlareStdThrHigh;
                    48,                                               //u4FlareStdThrLow;
                    0,                                                //u4PrvCapFlareDiff;
                    2,                                                //u4FlareMaxStepGap_Fast;
                    0,//0,                                                //u4FlareMaxStepGap_Slow;
                    1800,                                             //u4FlarMaxStepGapLimitBV;
                    2,//0,                                                //u4FlareAEStableCount;
                },
                //rAEMoveRatio =
                {
                    100,//100, //u4SpeedUpRatio 500
                    100, //100, //u4GlobalRatio
                    190, //u4Bright2TargetEnd  190 150
                    10,//20,   //u4Dark2TargetStart
                    90, //u4B2TEnd
                    85,//70,  //u4B2TStart 70
                    70,//60,  //u4D2TEnd 60
                    85,  //u4D2TStart
                },

                //rAEVideoMoveRatio =
                {
                    100, //u4SpeedUpRatio
                    100, //u4GlobalRatio
                    150,  //u4Bright2TargetEnd
                    20,    //u4Dark2TargetStart
                    90, //u4B2TEnd
                    40,  //u4B2TStart
                    30,  //u4D2TEnd
                    90,  //u4D2TStart
                },

                //rAEVideo1MoveRatio =
                {
                    100, //u4SpeedUpRatio
                    100, //u4GlobalRatio
                    150,  //u4Bright2TargetEnd
                    20,    //u4Dark2TargetStart
                    90, //u4B2TEnd
                    40,  //u4B2TStart
                    30,  //u4D2TEnd
                    90,  //u4D2TStart
                },

                //rAEVideo2MoveRatio =
                {
                    100, //u4SpeedUpRatio
                    100, //u4GlobalRatio
                    150,  //u4Bright2TargetEnd
                    20,    //u4Dark2TargetStart
                    90, //u4B2TEnd
                    40,  //u4B2TStart
                    30,  //u4D2TEnd
                    90,  //u4D2TStart
                },

                //rAECustom1MoveRatio =
                {
                    100, //u4SpeedUpRatio
                    100, //u4GlobalRatio
                    150,  //u4Bright2TargetEnd
                    20,    //u4Dark2TargetStart
                    90, //u4B2TEnd
                    40,  //u4B2TStart
                    30,  //u4D2TEnd
                    90,  //u4D2TStart
                },

                //rAECustom2MoveRatio =
                {
                    100, //u4SpeedUpRatio
                    100, //u4GlobalRatio
                    150,  //u4Bright2TargetEnd
                    20,    //u4Dark2TargetStart
                    90, //u4B2TEnd
                    40,  //u4B2TStart
                    30,  //u4D2TEnd
                    90,  //u4D2TStart
                },

                //rAECustom3MoveRatio =
                {
                    100, //u4SpeedUpRatio
                    100, //u4GlobalRatio
                    150,  //u4Bright2TargetEnd
                    20,    //u4Dark2TargetStart
                    90, //u4B2TEnd
                    40,  //u4B2TStart
                    30,  //u4D2TEnd
                    90,  //u4D2TStart
                },

                //rAECustom4MoveRatio =
                {
                    100, //u4SpeedUpRatio
                    100, //u4GlobalRatio
                    150,  //u4Bright2TargetEnd
                    20,    //u4Dark2TargetStart
                    90, //u4B2TEnd
                    40,  //u4B2TStart
                    30,  //u4D2TEnd
                    90,  //u4D2TStart
                },

                //rAECustom5MoveRatio =
                {
                    100, //u4SpeedUpRatio
                    100, //u4GlobalRatio
                    150,  //u4Bright2TargetEnd
                    20,    //u4Dark2TargetStart
                    90, //u4B2TEnd
                    40,  //u4B2TStart
                    30,  //u4D2TEnd
                    90,  //u4D2TStart
                },

                //rAEFaceMoveRatio =
                {
                    100, //u4SpeedUpRatio
                    100, //u4GlobalRatio
                    190,  //u4Bright2TargetEnd
                    10,    //u4Dark2TargetStart
                    80, //u4B2TEnd
                    30,  //u4B2TStart
                    20,  //u4D2TEnd
                    60,  //u4D2TStart
                },

                //rAETrackingMoveRatio =
                {
                    100, //u4SpeedUpRatio
                    100, //u4GlobalRatio
                    190,  //u4Bright2TargetEnd
                    10,    //u4Dark2TargetStart
                    80, //u4B2TEnd
                    30,  //u4B2TStart
                    20,  //u4D2TEnd
                    60,  //u4D2TStart
                },
                //rAEAOENVRAMParam =
                {
                    1,      // i4AOEStrengthIdx: 0 / 1 / 2
                    130,    // u4BVCompRatio
                    {
                        {
                            28,  //u4Y_Target
                            25,  //u4AOE_OE_percent
                            210,  //u4AOE_OEBound
                            10,    //u4AOE_DarkBound
                            950,    //u4AOE_LowlightPrecent
                            1,    //u4AOE_LowlightBound
                            145,    //u4AOESceneLV_L
                            180,    //u4AOESceneLV_H
                            40,    //u4AOE_SWHdrLE_Bound
                        },
                        {
                            28,  //u4Y_Target
                            25,  //u4AOE_OE_percent
                            210,  //u4AOE_OEBound
                            15, //20,    //u4AOE_DarkBound
                            950,    //u4AOE_LowlightPrecent
                            3, //10,    //u4AOE_LowlightBound
                            145,    //u4AOESceneLV_L
                            180,    //u4AOESceneLV_H
                            40,    //u4AOE_SWHdrLE_Bound
                        },
                        {
                            28,  //u4Y_Target
                            25,  //u4AOE_OE_percent
                            210,  //u4AOE_OEBound
                            25,    //u4AOE_DarkBound
                            950,    //u4AOE_LowlightPrecent
                            8,    //u4AOE_LowlightBound
                            145,    //u4AOESceneLV_L
                            180,    //u4AOESceneLV_H
                            40,    //u4AOE_SWHdrLE_Bound
                        }
                    }
                }
            },
            // rAEHDRConfig
            {
                3072,   // i4RMGSeg
                768,    // i4RMGTurn
                35,     // i4HDRTarget_L;
                40,     // i4HDRTarget_H;
                100,    // i4HDRTargetLV_L;
                120,    // i4HDRTargetLV_H;
                20,     // i4OverExpoRatio;
                212,    // i4OverExpoTarget;
                120,    // i4OverExpoLV_L;
                180,    // i4OverExpoLV_H;
                4,      // i4UnderExpoContrastThr;
                {
                 // Contrast:
                 //  0   1   2   3   4   5   6   7   8   9  10
                     3,  3,  3,  3,  3,  2,  1,  1,  1,  1,  1    // i4UnderExpoTargetTbl[AE_HDR_UNDEREXPO_CONTRAST_TARGET_TBL_NUM];
                },
                950,        // i4UnderExpoRatio;
                1000,       // i4AvgExpoRatio;
                8, //10,       // i4AvgExpoTarget;
                768,  // i4HDRAESpeed
                2,          // i4HDRAEConvergeThr;
                40,         // i4SWHdrLEThr
                20,         // i4SWHdrSERatio
                180,        // i4SWHdrSETarget
                1024        // i4SWHdrBaseGain
            }
        },
        {
            // rDevicesInfo
            {
                1136,   // u4MinGain, 1024 base =  1x
                8192,  // u4MaxGain, 16x
                100,     // u4MiniISOGain, ISOxx
                16,    // u4GainStepUnit, 1x/8
                11345,     // u4PreExpUnit
                30,     // u4PreMaxFrameRate
                10040,     // u4VideoExpUnit
                30,     // u4VideoMaxFrameRate
                1024,   // u4Video2PreRatio, 1024 base = 1x
                10040,     // u4CapExpUnit
                24,     // u4CapMaxFrameRate
                1024,   // u4Cap2PreRatio, 1024 base = 1x
                10040,     // u4Video1ExpUnit
                120,     // u4Video1MaxFrameRate
                1024,   // u4Video12PreRatio, 1024 base = 1x
                10040,     // u4Video2ExpUnit
                60,     // u4Video2MaxFrameRate
                1024,   // u4Video22PreRatio, 1024 base = 1x
                11345,     // u4Custom1ExpUnit
                30,     // u4Custom1MaxFrameRate
                1024,   // u4Custom12PreRatio, 1024 base = 1x
                11345,     // u4Custom2ExpUnit
                30,     // u4Custom2MaxFrameRate
                1024,   // u4Custom22PreRatio, 1024 base = 1x
                11345,     // u4Custom3ExpUnit
                30,     // u4Custom3MaxFrameRate
                1024,   // u4Custom32PreRatio, 1024 base = 1x
                11345,     // u4Custom4ExpUnit
                30,     // u4Custom4MaxFrameRate
                1024,   // u4Custom42PreRatio, 1024 base = 1x
                11345,     // u4Custom5ExpUnit
                30,     // u4Custom5MaxFrameRate
                1024,   // u4Custom52PreRatio, 1024 base = 1x
                20,      // u4LensFno, Fno = 2.0
                350     // u4FocusLength_100x
            },
            // rHistConfig
            {
                4, // 2,   // u4HistHighThres
                40,  // u4HistLowThres
                2,   // u4MostBrightRatio
                1,   // u4MostDarkRatio
                160, // u4CentralHighBound
                20,  // u4CentralLowBound
                {240, 230, 220, 210, 200}, // u4OverExpThres[AE_CCT_STRENGTH_NUM]
                {62, 70, 82, 108, 141},  // u4HistStretchThres[AE_CCT_STRENGTH_NUM]
                {18, 22, 26, 30, 34}       // u4BlackLightThres[AE_CCT_STRENGTH_NUM]
            },
            // rCCTConfig
            {
                TRUE,            // bEnableBlackLight
                TRUE,            // bEnableHistStretch
                TRUE,           // bEnableAntiOverExposure
                TRUE,            // bEnableTimeLPF
                TRUE,            // bEnableCaptureThres
                TRUE,            // bEnableVideoThres
                TRUE,            // bEnableVideo1Thres
                TRUE,            // bEnableVideo2Thres
                TRUE,            // bEnableCustom1Thres
                TRUE,            // bEnableCustom2Thres
                TRUE,            // bEnableCustom3Thres
                TRUE,            // bEnableCustom4Thres
                TRUE,            // bEnableCustom5Thres
                TRUE,            // bEnableStrobeThres
                47,                // u4AETarget
                47,                // u4StrobeAETarget

                70,                // u4InitIndex
                4,                 // u4BackLightWeight
                32,                // u4HistStretchWeight
                4,                 // u4AntiOverExpWeight
                2,                 // u4BlackLightStrengthIndex
                2,                 // u4HistStretchStrengthIndex
                2,                 // u4AntiOverExpStrengthIndex
                2,                 // u4TimeLPFStrengthIndex
                {1, 3, 5, 7, 8}, // u4LPFConvergeTable[AE_CCT_STRENGTH_NUM]
                90,                // u4InDoorEV = 9.0, 10 base
                -20,    // i4BVOffset delta BV = value/10
                64,                 // u4PreviewFlareOffset
                64,                 // u4CaptureFlareOffset
                3,                 // u4CaptureFlareThres
                64,                 // u4VideoFlareOffset
                3,                 // u4VideoFlareThres
                64,               // u4CustomFlareOffset
                3,                 //  u4CustomFlareThres
                64,                 // u4StrobeFlareOffset //12 bits
                3,                 // u4StrobeFlareThres // 0.5%
                160,                 // u4PrvMaxFlareThres //12 bit
                0,                 // u4PrvMinFlareThres
                160,                 // u4VideoMaxFlareThres // 12 bit
                0,                 // u4VideoMinFlareThres
                18,                // u4FlatnessThres              // 10 base for flatness condition.
                75,    // u4FlatnessStrength
                //rMeteringSpec
                {
                    //rHS_Spec
                    {
                        TRUE,//bEnableHistStretch           // enable histogram stretch
                        1024,//u4HistStretchWeight          // Histogram weighting value
                        40, //50, //20,//u4Pcent                      // 1%=10, 0~1000
                        160, //166,//176,//u4Thd                        // 0~255
                        75, //54, //74,//u4FlatThd                    // 0~255

                        120,//u4FlatBrightPcent
                        120,//u4FlatDarkPcent
                        //sFlatRatio
                        {
                            1000,  //i4X1
                            1024,  //i4Y1
                            2400, //i4X2
                            0     //i4Y2
                        },
                        TRUE, //bEnableGreyTextEnhance
                        1800, //u4GreyTextFlatStart, > sFlatRatio.i4X1, < sFlatRatio.i4X2
                        {
                            10,     //i4X1
                            1024,   //i4Y1
                            80,     //i4X2
                            0       //i4Y2
                        }
                    },
                    //rAOE_Spec
                    {
                        TRUE, //bEnableAntiOverExposure
                        1024, //u4AntiOverExpWeight
                        10,    //u4Pcent
                      220,//  200,  //u4Thd

                        TRUE, //bEnableCOEP
                        1,    //u4COEPcent
                        106,  //u4COEThd
                        0,  // u4BVCompRatio
                        //sCOEYRatio;     // the outer y ratio
                        {
                            23,   //i4X1
                            1024,  //i4Y1
                            47,   //i4X2
                            0     //i4Y2
                        },
                        //sCOEDiffRatio;  // inner/outer y difference ratio
                        {
                            1500, //i4X1
                            0,    //i4Y1
                            2100, //i4X2
                            1024   //i4Y2
                        }
                    },
                    //rABL_Spec
                    {
                        TRUE,//bEnableBlackLigh
                        1024,//u4BackLightWeigh
                        400,//u4Pcent
                        22,//u4Thd,
                        255, // center luminance
                        246, //256, // final target limitation, 256/128 = 2x
                        //sFgBgEVRatio
                        {
                            2100,//2200, //i4X1
                            0,    //i4Y1
                            4000, //i4X2
                            1024   //i4Y2
                        },
                        //sBVRatio
                        {
                            3800,//i4X1
                            0,   //i4Y1
                            5000,//i4X2
                            1024  //i4Y2
                        }
                    },
                    //rNS_Spec
                    {
                        TRUE, // bEnableNightScene
                        10, //5,    //u4Pcent
                       150, // 170,  //u4Thd
                        72, //52,   //u4FlatThd

    					          180, //  200,  //u4BrightTonePcent
    				            80, //85,//	82, //  95, //u4BrightToneThd

                        500,  //u4LowBndPcent
                        5,    //u4LowBndThd
                        26,    //u4LowBndThdLimit

                        50,  //u4FlatBrightPcent;
                        300,   //u4FlatDarkPcent;
                        //sFlatRatio
                        {
                            1200, //i4X1
                            1024, //i4Y1
                          2800,//  2400, //i4X2
                            0    //i4Y2
                        },
                        //sBVRatio
                        {
                            -500, //i4X1
                            1024,  //i4Y1
                            3000, //i4X2
                            0     //i4Y2
                        },
                        TRUE, // bEnableNightSkySuppresion
                        //sSkyBVRatio
                        {
                            -4000, //i4X1
                            1024, //i4X2
                            -2000,  //i4Y1
                            0     //i4Y2
                        }
                    },
                    // rTOUCHFD_Spec
                    {
                        40, //uMeteringYLowBound;
                        50, //uMeteringYHighBound;
                        40, //uFaceYLowBound;
                        50, //uFaceYHighBound;
                        3,  //uFaceCentralWeight;
                        120,//u4MeteringStableMax;
                        80, //u4MeteringStableMin;
                    }
                }, //End rMeteringSpec
                // rFlareSpec
                {
                    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, //uPrvFlareWeightArr[16];
                    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, //uVideoFlareWeightArr[16];
                    96,                                               //u4FlareStdThrHigh;
                    48,                                               //u4FlareStdThrLow;
                    0,                                                //u4PrvCapFlareDiff;
                    2,                                                //u4FlareMaxStepGap_Fast;
                    0,//0,                                                //u4FlareMaxStepGap_Slow;
                    1800,                                             //u4FlarMaxStepGapLimitBV;
                    2,//0,                                                //u4FlareAEStableCount;
                },
                //rAEMoveRatio =
                {
                    100,//100, //u4SpeedUpRatio 500
                    100, //100, //u4GlobalRatio
                    190, //u4Bright2TargetEnd  190 150
                    10,//20,   //u4Dark2TargetStart
                    90, //u4B2TEnd
                    85,//70,  //u4B2TStart 70
                    70,//60,  //u4D2TEnd 60
                    85,  //u4D2TStart
                },

                //rAEVideoMoveRatio =
                {
                    100, //u4SpeedUpRatio
                    100, //u4GlobalRatio
                    150,  //u4Bright2TargetEnd
                    20,    //u4Dark2TargetStart
                    90, //u4B2TEnd
                    40,  //u4B2TStart
                    30,  //u4D2TEnd
                    90,  //u4D2TStart
                },

                //rAEVideo1MoveRatio =
                {
                    100, //u4SpeedUpRatio
                    100, //u4GlobalRatio
                    150,  //u4Bright2TargetEnd
                    20,    //u4Dark2TargetStart
                    90, //u4B2TEnd
                    40,  //u4B2TStart
                    30,  //u4D2TEnd
                    90,  //u4D2TStart
                },

                //rAEVideo2MoveRatio =
                {
                    100, //u4SpeedUpRatio
                    100, //u4GlobalRatio
                    150,  //u4Bright2TargetEnd
                    20,    //u4Dark2TargetStart
                    90, //u4B2TEnd
                    40,  //u4B2TStart
                    30,  //u4D2TEnd
                    90,  //u4D2TStart
                },

                //rAECustom1MoveRatio =
                {
                    100, //u4SpeedUpRatio
                    100, //u4GlobalRatio
                    150,  //u4Bright2TargetEnd
                    20,    //u4Dark2TargetStart
                    90, //u4B2TEnd
                    40,  //u4B2TStart
                    30,  //u4D2TEnd
                    90,  //u4D2TStart
                },

                //rAECustom2MoveRatio =
                {
                    100, //u4SpeedUpRatio
                    100, //u4GlobalRatio
                    150,  //u4Bright2TargetEnd
                    20,    //u4Dark2TargetStart
                    90, //u4B2TEnd
                    40,  //u4B2TStart
                    30,  //u4D2TEnd
                    90,  //u4D2TStart
                },

                //rAECustom3MoveRatio =
                {
                    100, //u4SpeedUpRatio
                    100, //u4GlobalRatio
                    150,  //u4Bright2TargetEnd
                    20,    //u4Dark2TargetStart
                    90, //u4B2TEnd
                    40,  //u4B2TStart
                    30,  //u4D2TEnd
                    90,  //u4D2TStart
                },

                //rAECustom4MoveRatio =
                {
                    100, //u4SpeedUpRatio
                    100, //u4GlobalRatio
                    150,  //u4Bright2TargetEnd
                    20,    //u4Dark2TargetStart
                    90, //u4B2TEnd
                    40,  //u4B2TStart
                    30,  //u4D2TEnd
                    90,  //u4D2TStart
                },

                //rAECustom5MoveRatio =
                {
                    100, //u4SpeedUpRatio
                    100, //u4GlobalRatio
                    150,  //u4Bright2TargetEnd
                    20,    //u4Dark2TargetStart
                    90, //u4B2TEnd
                    40,  //u4B2TStart
                    30,  //u4D2TEnd
                    90,  //u4D2TStart
                },

                //rAEFaceMoveRatio =
                {
                    100, //u4SpeedUpRatio
                    100, //u4GlobalRatio
                    190,  //u4Bright2TargetEnd
                    10,    //u4Dark2TargetStart
                    80, //u4B2TEnd
                    30,  //u4B2TStart
                    20,  //u4D2TEnd
                    60,  //u4D2TStart
                },

                //rAETrackingMoveRatio =
                {
                    100, //u4SpeedUpRatio
                    100, //u4GlobalRatio
                    190,  //u4Bright2TargetEnd
                    10,    //u4Dark2TargetStart
                    80, //u4B2TEnd
                    30,  //u4B2TStart
                    20,  //u4D2TEnd
                    60,  //u4D2TStart
                },
                //rAEAOENVRAMParam =
                {
                    1,      // i4AOEStrengthIdx: 0 / 1 / 2
                    130,    // u4BVCompRatio
                    {
                        {
                            28,  //u4Y_Target
                            25,  //u4AOE_OE_percent
                            210,  //u4AOE_OEBound
                            10,    //u4AOE_DarkBound
                            950,    //u4AOE_LowlightPrecent
                            1,    //u4AOE_LowlightBound
                            145,    //u4AOESceneLV_L
                            180,    //u4AOESceneLV_H
                            40,    //u4AOE_SWHdrLE_Bound
                        },
                        {
                            28,  //u4Y_Target
                            25,  //u4AOE_OE_percent
                            210,  //u4AOE_OEBound
                            15, //20,    //u4AOE_DarkBound
                            950,    //u4AOE_LowlightPrecent
                            3, //10,    //u4AOE_LowlightBound
                            145,    //u4AOESceneLV_L
                            180,    //u4AOESceneLV_H
                            40,    //u4AOE_SWHdrLE_Bound
                        },
                        {
                            28,  //u4Y_Target
                            25,  //u4AOE_OE_percent
                            210,  //u4AOE_OEBound
                            25,    //u4AOE_DarkBound
                            950,    //u4AOE_LowlightPrecent
                            8,    //u4AOE_LowlightBound
                            145,    //u4AOESceneLV_L
                            180,    //u4AOESceneLV_H
                            40,    //u4AOE_SWHdrLE_Bound
                        }
                    }
                }
            },
            // rAEHDRConfig
            {
                3072,   // i4RMGSeg
                768,    // i4RMGTurn
                35,     // i4HDRTarget_L;
                40,     // i4HDRTarget_H;
                100,    // i4HDRTargetLV_L;
                120,    // i4HDRTargetLV_H;
                20,     // i4OverExpoRatio;
                212,    // i4OverExpoTarget;
                120,    // i4OverExpoLV_L;
                180,    // i4OverExpoLV_H;
                4,      // i4UnderExpoContrastThr;
                {
                 // Contrast:
                 //  0   1   2   3   4   5   6   7   8   9  10
                     3,  3,  3,  3,  3,  2,  1,  1,  1,  1,  1    // i4UnderExpoTargetTbl[AE_HDR_UNDEREXPO_CONTRAST_TARGET_TBL_NUM];
                },
                950,        // i4UnderExpoRatio;
                1000,       // i4AvgExpoRatio;
                8, //10,       // i4AvgExpoTarget;
                768,  // i4HDRAESpeed
                2,          // i4HDRAEConvergeThr;
                40,         // i4SWHdrLEThr
                20,         // i4SWHdrSERatio
                180,        // i4SWHdrSETarget
                1024        // i4SWHdrBaseGain
            }
        }
    },

    // AWB NVRAM
    AWB_NVRAM_START
    {
        getAWBNVRAM_imx300mipiraw<CAM_SCENARIO_PREVIEW>(),
        getAWBNVRAM_imx300mipiraw<CAM_SCENARIO_VIDEO>(),
        getAWBNVRAM_imx300mipiraw<CAM_SCENARIO_CAPTURE>(),
        getAWBNVRAM_imx300mipiraw<CAM_SCENARIO_CUSTOM1>(),
        getAWBNVRAM_imx300mipiraw<CAM_SCENARIO_CUSTOM2>(),
        getAWBNVRAM_imx300mipiraw<CAM_SCENARIO_CUSTOM3>(),
        getAWBNVRAM_imx300mipiraw<CAM_SCENARIO_CUSTOM4>()
    },

    // Flash AWB NVRAM
    {
#include INCLUDE_FILENAME_FLASH_AWB_PARA
    },

    {0}
};

#include INCLUDE_FILENAME_ISP_LSC_PARAM
//};  //  namespace

const CAMERA_TSF_TBL_STRUCT CAMERA_TSF_DEFAULT_VALUE =
{
    {
        1,  // isTsfEn
        2,  // tsfCtIdx
        {20, 2000, -110, -110, 512, 512, 512, 0}    // rAWBInput[8]
    },

#include INCLUDE_FILENAME_TSF_PARA
#include INCLUDE_FILENAME_TSF_DATA
};

const NVRAM_CAMERA_FEATURE_STRUCT CAMERA_FEATURE_DEFAULT_VALUE =
{
#include INCLUDE_FILENAME_FEATURE_PARA
};

typedef NSFeature::RAWSensorInfo<SENSOR_ID> SensorInfoSingleton_T;


namespace NSFeature {
  template <>
  UINT32
  SensorInfoSingleton_T::
  impGetDefaultData(CAMERA_DATA_TYPE_ENUM const CameraDataType, VOID*const pDataBuf, UINT32 const size) const
  {
    UINT32 dataSize[CAMERA_DATA_TYPE_NUM] = {sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT),
        sizeof(NVRAM_CAMERA_3A_STRUCT),
        sizeof(NVRAM_CAMERA_SHADING_STRUCT),
        sizeof(NVRAM_LENS_PARA_STRUCT),
        sizeof(AE_PLINETABLE_T),
        0,
        sizeof(CAMERA_TSF_TBL_STRUCT),
        0,
        0,
        sizeof(NVRAM_CAMERA_FEATURE_STRUCT)
    };

    if (CameraDataType > CAMERA_NVRAM_DATA_FEATURE || NULL == pDataBuf || (size < dataSize[CameraDataType]))
    {
      return 1;
    }

    switch(CameraDataType)
    {
      case CAMERA_NVRAM_DATA_ISP:
        memcpy(pDataBuf,&CAMERA_ISP_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT));
        break;
      case CAMERA_NVRAM_DATA_3A:
        memcpy(pDataBuf,&CAMERA_3A_NVRAM_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_3A_STRUCT));
        break;
      case CAMERA_NVRAM_DATA_SHADING:
        memcpy(pDataBuf,&CAMERA_SHADING_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_SHADING_STRUCT));
        break;
      case CAMERA_DATA_AE_PLINETABLE:
        memcpy(pDataBuf,&g_PlineTableMapping,sizeof(AE_PLINETABLE_T));
        break;
      case CAMERA_DATA_TSF_TABLE:
        memcpy(pDataBuf,&CAMERA_TSF_DEFAULT_VALUE,sizeof(CAMERA_TSF_TBL_STRUCT));
        break;
      case CAMERA_NVRAM_DATA_FEATURE:
        memcpy(pDataBuf,&CAMERA_FEATURE_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_FEATURE_STRUCT));
        break;
      default:
        break;
    }
    return 0;
  }};  //  NSFeature


