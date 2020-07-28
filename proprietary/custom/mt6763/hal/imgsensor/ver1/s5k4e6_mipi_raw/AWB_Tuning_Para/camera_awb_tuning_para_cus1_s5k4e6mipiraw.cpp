#include "camera_custom_nvram.h"
#include "camera_awb_tuning_para_s5k4e6mipiraw.h"

#define NVRAM_TUNING_PARAM_NUM  6511001

template <>
AWB_NVRAM_T const&
getAWBNVRAM_s5k4e6mipiraw<CAM_SCENARIO_CUSTOM1>() // HDR
{
    static AWB_NVRAM_T  g_rAWBNVRAM =
    AWB_NVRAM_START
{
    {
        // AWB calibration data
        {
/******************************************************************************/
//Feature name:
//AWB unit and golden module calibration gain for DNP(D50)
//variable name: rUnitGain
//default value: {512, 512, 512}
//unit: 512 = 1x
//data range : 0 ~ 4095
//note: rGain = {0, 0 ,0} means this unit not doing calibration for DNP
/******************************************************************************/
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
/******************************************************************************/
//Feature name:
//AWB unit and golden module calibration gain for middle color temperature TL84
//variable name: rUnitGainM
//default value: {512, 512, 512}
//unit: 512 = 1x
//data range : 0 ~ 4095
//note: rGain = {0, 0 ,0} means this unit not doing calibration for TL84
/******************************************************************************/
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
/******************************************************************************/
//Feature name:
//AWB and golden  module calibration gain for low color temperature A light (tungsten)
//variable name: rUnitGainL
//default value: {512, 512, 512}
//unit: 512 = 1x
//data range : 0 ~ 4095
//note: rGain = {0, 0 ,0} means this unit not doing calibration for A light
/******************************************************************************/
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
/******************************************************************************/
//Feature name:
//AWB unit gain for tuning sample only for debug use
//variable name: rTuningUnitGain
//default value: {512, 512, 512}
//unit: 512 = 1x
//data range : 0 ~ 4095
//note: rGain = {0, 0 ,0} means no tuning sample unit gain
/******************************************************************************/
            // rTuningUnitGain (Tuning sample unit gain: 1.0 = 512)
            {
                0, // TuningUnitGain_R
                0, // TuningUnitGain_G
                0 // TuningUnitGain_B
            },
/******************************************************************************/
//Feature name:
//AWB golden sample calibration gain for D65
//variable name: rD65Gain
//default value: {512, 512, 512}
//unit: 512 = 1x
//data range : 0 ~ 4095
/******************************************************************************/
            // rD65Gain (D65 WB gain: 1.0 = 512)
            {
                1204, // D65Gain_R
                512, // D65Gain_G
                661 // D65Gain_B
            },
/******************************************************************************/
//Feature name:
//AWB unit and golden module Bayer value for DNP(D50)
//variable name: rUnitValue, rGoldenValue
//default value: {128, 128, 128, 128}
//data range : 1 ~ 255
//note: rValue = {0, 0 ,0, 0} means this unit not doing calibration for DNP
/******************************************************************************/
            // rUnitValue for DNP
            {
                  0, // UnitValue_R
                  0, // UnitValue_Gr
                  0, // UnitValue_Gb
                  0  // UnitValue_B
            },
            // rGoldenValue for DNP
            {
                  0, // GoldenValue_R
                  0, // GoldenValue_Gr
                  0, // GoldenValue_Gb
                  0  // GoldenValue_B
            },
/******************************************************************************/
//Feature name:
//AWB unit and golden module Bayer value for middle color temperature TL84
//variable name: rUnitValueM, rGoldenValueM
//default value: {128, 128, 128, 128}
//data range : 1 ~ 255
//note: rValue = {0, 0 ,0, 0} means this unit not doing calibration for TL84
/******************************************************************************/
            // rUnitValueM for TL84
            {
                  0, // UnitValue_R
                  0, // UnitValue_Gr
                  0, // UnitValue_Gb
                  0  // UnitValue_B
            },
            // rGoldenValueM for TL84
            {
                  0, // UnitValue_R
                  0, // UnitValue_Gr
                  0, // UnitValue_Gb
                  0  // UnitValue_B
            },
/******************************************************************************/
//Feature name:
//AWB unit and golden module Bayer value for low color temperature A light (tungsten)
//variable name: rUnitValueL, rGoldenValueL
//default value: {128, 128, 128, 128}
//data range : 1 ~ 255
//note: rValue = {0, 0 ,0, 0} means this unit not doing calibration for Alight
/******************************************************************************/
            // rUnitValueL for Alight
            {
                  0, // UnitValue_R
                  0, // UnitValue_Gr
                  0, // UnitValue_Gb
                  0  // UnitValue_B
            },
            // rGoldenValueL for Alight
            {
                  0, // GoldenValue_R
                  0, // GoldenValue_Gr
                  0, // GoldenValue_Gb
                  0  // GoldenValue_B
            }
        },
/******************************************************************************/
//Feature name:
//AWB average white point distribution in X-Y domin under different light sources as
//strobe, horizon, A (tungsten), TL84, CWF, DNP, D65 and DF (daylight fluorescnet)
//variable name: OriX_Strobe, OriY_Strobe, OriX_Hor, OriY_Hor, OriX_A, OriY_A, OriX_TL84,
//OriY_TL84, OriX_CWF, OriY_CWF, OriX_DNP, OriY_DNP, OriX_D65, OriY_D65, OriX_DF, OriY_DF
//default value: N/A
//note: These settings are calibrated result, please do not need to modify them
/******************************************************************************/
        // Original XY coordinate of AWB light source
        {
            // Strobe
            {
                0, // OriX_Strobe
                0 // OriY_Strobe
            },
            // Horizon
            {
                -365, // OriX_Hor
                -433 // OriY_Hor
            },
            // A
            {
                -237, // OriX_A
                -451 // OriY_A
            },
            // TL84
            {
                5, // OriX_TL84
                -436 // OriY_TL84
            },
            // CWF
            {
                7, // OriX_CWF
                -502 // OriY_CWF
            },
            // DNP
            {
                97, // OriX_DNP
                -448 // OriY_DNP
            },
            // D65
            {
                222, // OriX_D65
                -410 // OriY_D65
            },
            // DF
            {
                0, // OriX_DF
                0 // OriY_DF
            }
        },
/******************************************************************************/
//Feature name:
//AWB average white point distribution in rotated X-Y domain from original X-Y domain
//in order to let the light source distribution in x-axis parallel direction
//variable name: RotX_Strobe, RotY_Strobe, RotX_Hor, RotY_Hor, RotX_A, RotY_A, RotX_TL84,
//RotY_TL84, RotX_CWF, RotY_CWF, RotX_DNP, RotY_DNP, RotX_D65, RotY_D65, RotX_DF, RotY_DF
//default value: N/A
//note: These settings are calibrated result, please do not need to modify them
/******************************************************************************/
        // Rotated XY coordinate of AWB light source		
        {
            // Strobe
            {
                0, // RotX_Strobe
                0 // RotY_Strobe
            },
            // Horizon
            {
                -394, // RotX_Hor
                -406 // RotY_Hor
            },
            // A
            {
                -268, // RotX_A
                -433 // RotY_A
            },
            // TL84
            {
                -26, // RotX_TL84
                -435 // RotY_TL84
            },
            // CWF
            {
                -28, // RotX_CWF
                -501 // RotY_CWF
            },
            // DNP
            {
                65, // RotX_DNP
                -453 // RotY_DNP
            },
            // D65
            {
                192, // RotX_D65
                -424 // RotY_D65
            },
            // DF
            {
                182, // RotX_DF
                -492 // RotY_DF
            }
        },
/******************************************************************************/
//Feature name:
//AWB pre-calibrated gain based on the calibrated average white point distribution
//for different light sources. It is treated as a default gain for corresponding light source
//variable name: Gain_Strobe, Gain_Hor, Gain_A, Gain_TL84, Gain_CWF, Gain_DNP, Gain_D65, Gain_DF
//default value: N/A
//note: These settings are calibrated result, please do not need to modify them
/******************************************************************************/
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
                561, // AWBGAIN_HOR_R
                512, // AWBGAIN_HOR_G
                1507 // AWBGAIN_HOR_B
            },
            // A
            {
                684, // AWBGAIN_A_R
                512, // AWBGAIN_A_G
                1299 // AWBGAIN_A_B
            },
            // TL84
            {
                931, // AWBGAIN_TL84_R
                512, // AWBGAIN_TL84_G
                917 // AWBGAIN_TL84_B
            },
            // CWF
            {
                1019, // AWBGAIN_CWF_R
                512, // AWBGAIN_CWF_G
                1001 // AWBGAIN_CWF_B
            },
            // DNP
            {
                1071, // AWBGAIN_DNP_R
                512, // AWBGAIN_DNP_G
                824 // AWBGAIN_DNP_B
            },
            // D65
            {
                1204, // AWBGAIN_D65_R
                512, // AWBGAIN_D65_G
                661 // AWBGAIN_D65_B
            },
            // DF
            {
                512, // AWBGAIN_DF_R
                512, // AWBGAIN_DF_G
                512 // AWBGAIN_DF_B
            }
        },
/******************************************************************************/
//Feature name:
//Rotation angle information for the rotated X-Y domain from original X-Y domain
//variable name: RotationAngle
//degree of rotation from original X-Y domain to rotated X-Y domain
//data range: -180 ~ 180 (degree)
//default value: N/A
//variable name: Cos
//value of cosine (RotationAngle) normalized by 256
//data range: -256 ~ 256
//default value: N/A
//variable name: Sin
//value of sine (RotationAngle) normalized by 255
//data range: -256 ~ 256
//defatul value: N/A
//note: These settings are calibrated result, please do not need to modify them
/******************************************************************************/
        // Rotation matrix parameter
        {
              4, // RotationAngle
            255, // Cos
             18  // Sin
        },
/******************************************************************************/
//Feature name:
//Linear regression of distrubution of D65, TL84 and A(after compensated with D65 white balance gain)
//on log(R/G)-log(B/G) domain into a linear line pass through the origin.
//The mathematical equation can be represented by y = (SlopeNumerator) / (SlopeDenominator) * x,
//where x is log(R/G) and y is log(B/G)
//variable name: SlopeNumerator
//numerator value of the slope of above equation
//data range: N/A
//default value: N/A
//variable name: SlopeDenominator
//denominator value for the slope of above equation
//data range: N/A
//default value: N/A
//note: These settings are based on calibration, please do not need to modify them
/******************************************************************************/
        // Daylight locus parameter	
        {
            -145, // SlopeNumerator
             128  // SlopeDenominator
        },
        // Predictor gain	
        {
/******************************************************************************/
//Feature name:
//White balance gain of daylight locus at low color temperature bound, used for
//current daylight locus gain prediction which is interpolated with WB gain of
//daylight locus at high color temperature bound by current LV value 
//variable name: i4R, i4G, i4B
//default value: {512, 512, 512}
/******************************************************************************/
            // rSpatial_L
            {
                1100, // i4R
                 512, // i4G
                 676  // i4B
            },
/******************************************************************************/
//Feature name:
//White balance gain of daylight locus at high color temperature bound, used for
//current daylight locus gain prediction which is interpolated with WB gain of daylight
// locus at low color temperature bound by current LV value 
//variable name: i4R, i4G, i4B
//default value: {512, 512, 512}
/******************************************************************************/
            // rSpatial_H
            {
                 971, // i4R
                 512, // i4G
                 850  // i4B
            },
/******************************************************************************/
//Feature name:
//White balance gain used when no light source information in the beginning and as
//a daylight locus refeneced when current LV is under low threshold when used temeroal
//information in no light source condition 
//variable name: i4R, i4G, i4B
//default value: {512, 512, 512}
/******************************************************************************/
            // rTemporal_General
            {
                1204, // i4R
                 512, // i4G
                 661  // i4B
            }
        },
/******************************************************************************/
//Feature name:
//A quadrangular window used to define the white point distrubution under strobe light,
//tungsten, warm fluorescent, fluorescent, CWF, daylight, shade and daylight fluorescent
//in rotated X-Y domain for auto white balance white point collection 
//variable name: i4RightBound, i4LeftBound, i4UpperBound, i4LowerBound
//defines the right boundary in x-axis, left boundary in x-axis, upper boundary in
//y-axis and lower boundary in y-axis
//default value: N/A
//note: The value is calibrated by the strobe light in the lab, it can be fine tuned
//to cover all the possible white point distrubution
/******************************************************************************/
        // AWB light area
        {
            // Strobe
            {
                   0, // StrobeRightBound
                   0, // StrobeLeftBound
                   0, // StrobeUpperBound
                   0  // StrobeLowerBound
            },
            // Tungsten
            {
                -123, // TungRightBound
                -794, // TungLeftBound
                -331, // TungUpperBound
                -396  // TungLowerBound
            },
            // Warm fluorescent
            {
                -123, // WFluoRightBound
                -794, // WFluoLeftBound
                -396, // WFluoUpperBound
                -591  // WFluoLowerBound
            },
            // Fluorescent
            {
                  35, // FluoRightBound
                -123, // FluoLeftBound
                -364, // FluoUpperBound
                -468  // FluoLowerBound
            },
            // CWF
            {
                  64, // CWFRightBound
                -123, // CWFLeftBound
                -468, // CWFUpperBound
                -556  // CWFLowerBound
            },
            // Daylight
            {
                 222, // DayRightBound
                  35, // DayLeftBound
                -364, // DayUpperBound
                -468  // DayLowerBound
            },
            // Shade
            {
                 552, // ShadeRightBound
                 222, // ShadeLeftBound
                -364, // ShadeUpperBound
                -455  // ShadeLowerBound
            },
            // Daylight Fluorescent	
            {
                 222, // DFRightBound
                  64, // DFLeftBound
                -468, // DFUpperBound
                -556  // DFLowerBound
            }
        },
/******************************************************************************/
//Feature name:
//A quadrangular window used to define the point collection reference area used in
//preset white blance for reference area, daylight, cloudy daylight, shade, twilight,
//fluorescent, warm fluorescent, incandescent and gray world. The white point inside will
//be calculated as white point.
//variable name: i4RightBound, i4LeftBound, i4UpperBound, i4LowerBound
//defines the right boundary in x-axis, left boundary in x-axis, upper boundary in
//y-axis and lower boundary in y-axis
//default value: N/A
/******************************************************************************/
        // PWB light area
        {
            // Reference area
            {
                 552, // PRefRightBound
                -794, // PRefLeftBound
                -306, // PRefUpperBound
                -591  // PRefLowerBound
            },
            // Daylight
            {
                 247, // PDayRightBound
                  35, // PDayLeftBound
                -364, // PDayUpperBound
                -468  // PDayLowerBound
            },
            // Cloudy daylight
            {
                 347, // PCloudyRightBound
                 172, // PCloudyLeftBound
                -364, // PCloudyUpperBound
                -468  // PCloudyLowerBound
            },
            // Shade
            {
                 447, // PShadeRightBound
                 172, // PShadeLeftBound
                -364, // PShadeUpperBound
                -468  // PShadeLowerBound
            },
            // Twilight
            {
                  35, // PTwiRightBound
                -125, // PTwiLeftBound
                -364, // PTwiUpperBound
                -468  // PTwiLowerBound
            },
            // Fluorescent
            {
                 242, // PFluoRightBound
                -128, // PFluoLeftBound
                -374, // PFluoUpperBound
                -551  // PFluoLowerBound
            },
            // Warm fluorescent
            {
                -168, // PWFluoRightBound
                -368, // PWFluoLeftBound
                -374, // PWFluoUpperBound
                -551  // PWFluoLowerBound
            },
            // Incandescent
            {
                -168, // PIncaRightBound
                -368, // PIncaLeftBound
                -364, // PIncaUpperBound
                -468  // PIncaLowerBound
            },
            // Gray World
            {
                5000, // PGWRightBound
                -5000, // PGWLeftBound
                5000, // PGWUpperBound
                -5000  // PGWLowerBound
            }
        },
/******************************************************************************/
//Feature name:
//Defaulte preset white balance gain for daylight, cloudy daylight, shade, twilight,
//fluorescent, warm fluorescent, incandescent and grey world.
//variable name: i4R, i4G, i4B for R gain, G gain and B gain
//default value: N/A
//data range : 0 ~ 4095
//unit: 512 = 1x
/******************************************************************************/
        // PWB default gain	
        {
            // Daylight	
            {
                1116, // PWB_Day_R
                 512, // PWB_Day_G
                 704  // PWB_Day_B
            },
            // Cloudy daylight
            {
                1296, // PWB_Cloudy_R
                 512, // PWB_Cloudy_G
                 593  // PWB_Cloudy_B
            },
            // Shade
            {
                1380, // PWB_Shade_R
                 512, // PWB_Shade_G
                 552  // PWB_Shade_B
            },
            // Twilight
            {
                 884, // PWB_Twi_R
                 512, // PWB_Twi_G
                 922  // PWB_Twi_B
            },
            // Fluorescent
            {
                1074, // PWB_Fluo_R
                 512, // PWB_Fluo_G
                 843  // PWB_Fluo_B
            },
            // Warm fluorescent
            {
                 714, // PWB_WFluo_R
                 512, // PWB_WFluo_G
                1350  // PWB_WFluo_B
            },
            // Incandescent
            {
                 668, // PWB_Inca_R
                 512, // PWB_Inca_G
                1273  // PWB_Inca_B
            },
            // Gray World	
            {
                 512, // PWB_GW_R
                 512, // PWB_GW_G
                 512  // PWB_GW_B
            }
        },
        // CCT estimation  
        {
/******************************************************************************/
//Feature name:
//Color temperature estimation for the light source
//variable name: CCT0, CCT1, CCT2, CCT3 and CCT4
//color temperature for Horizon, A, TL84, DNP and D65 correspondingly
//default value: N/A
/******************************************************************************/
            // CCT
            {
                2300,  // CCT0
                2850,  // CCT1
                3750,  // CCT2
                5100,  // CCT3
                6500   // CCT4
            },
/******************************************************************************/
//Feature name:
//The Xr (rotated X) of the light source in color temeperature estimation
//variable name: RotatedXCoordinate0, RotatedXCoordinate1, RotatedXCoordinate2,
//RotatedXCoordinate3 and RotatedXCoordinate4 
//color temperature for Horizon, A, TL84, DNP and D65 correspondingly
//default value: N/A
/******************************************************************************/
            // Rotated X coordinate	
            {
                -586,  // RotatedXCoordinate0
                -460,  // RotatedXCoordinate1
                -218,  // RotatedXCoordinate2
                -127,  // RotatedXCoordinate3
                   0   // RotatedXCoordinate4
            },
        },
/******************************************************************************/
//Feature name:
//Porperty of color transform matrix
//Uint color tranform  matrix = {1024, 0, 0, 0, 1024, 0, 0, 0, 1024} 
//default value: 0
//data range: 0 and 1
//0 means disable color transform and 1 means enable color transform
//variable name: i4RWBCof
//default value: N/A
//transform 3x3 color matrix
//variable name: i4RWBCofInv
//default value: N/A
//inverse transform 3x3 color matrix
/******************************************************************************/
        // None Bayer Sensor  
        {
            0,  // enable
            { 1024,    0,    0,    0, 1024,    0,    0,    0, 1024}, // color transfer matrix
            { 1024,    0,    0,    0, 1024,    0,    0,    0, 1024}, // inverse color transfer matrix
        },
    },
    // Algorithm Tuning Paramter
    {
/******************************************************************************/
//Feature name:
//A set of gain applied after wb algorithm calculation output gain based on light source
//and LV. It is used to optimize and fine tune for preference
//variable name: {i4R, i4G, i4B} gain table from LV0 to LV18 for each light source
//(Strobe, Tungsten, Warm Fluorescent, Fluorescent, CWF, Daylight, Daylight Fluorescent, Shade) 
//default value: {512, 512, 512}
//unit: 512 = 1x
/******************************************************************************/
        // Preference gain for each light source
        {
            { // STROBE
                //    LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
                {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                //    LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18      
                {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
            },
            { // TUNGSTEN
                //    LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
                {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                //    LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18      
                {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
            },
            { // WF
                //    LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
                {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                //    LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18      
                {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
            },
            { // F
                //    LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
                {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                //    LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18      
                {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}
            },
            { // CWF
                //    LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
                {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                //    LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18      
                {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
            },
            { // DAYLIGHT
                //    LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
                {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                //    LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18      
                {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}
            },
            { // SHADE
                //    LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
                {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                //    LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18      
                {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
            },
            { // DF
                //    LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
                {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                //    LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18      
                {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
            },
        },
/******************************************************************************/
//Feature name:
//A set of LV threshold for spatial gain determination for each illuminant light
//source including no neutral light
//variable name: rNonReliable
//default value: N/A
//data range: 0 ~ 180
//LV low threshold and high threshold for spatial gain determination in no reliable
//light source environment
//variable name: rLightSrc
//default value: N/A
//data range: 0 ~ 180
//LV low threshold and high threshold for spatial gain determination for each light source,
//including Strobe, Tungsten, Warm Fluorescent, Fluorescent, CWF, Daylight, Shade and Daylight Fluorescent
//	 y = interploate(x, x1, x2, y1, y2), where x1 and x2 are the low LV and high LV threshold.
// y1 and y2 are the spatial predictor gain DaylightLocus_L and DaylightLocus_H correspondingly.
//  y = y2, if x >= x2
//	 y = y1, if x < x1
//  y = (x - x1) / (x2 - x1) * (y2 - y1), if x1 <= x < x2
/******************************************************************************/
        // AWB LV threshold for predictor
        {
            {  90, 130}, // NonReliable
            {
                { 105, 135}, // Strobe
                {  20, 120}, // Tungsten
                {  20, 120}, // WF
                {  20,  75}, // Fluorescent
                {  20,  75}, // CWF
                {  20, 150}, // Daylight
                {  20, 150}, // Shade
                {  20, 150}, // DF
             },
        },
/******************************************************************************/
//Feature name:
//Neutral parent block number threshold (ratio)for CWF/DF temporal enqueue
//variable name: i4Neutral_ParentBlk_Thr
//default value: N/A
//data range: 0 ~ 432
//One condition for current frame be enqueued to temporal
//when neutral parent block number > i4Neutral_ParentBlk_Thr
//varaiable name: i4CWFDF_LUTThr
//default value: N/A
//Look-up table of neutral block threahold ratio based on LV for CWF and DF light source.
//The other conditions for current frame be enqueued is
//(1) neutral parent block[CWF] < neutral parent block number * i4CWFDF_LUTThr(LV)
//(2) neutral parent block[DF] < neutral parent block number * i4CWFDF_LUTThr(LV)
/******************************************************************************/
        // AWB number threshold for temporal predictor
        {
            65,  // i4Neutral_ParentBlk_Thr
            //LV 0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18
            {  100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  50,  25,   2,   2,   2,   2,   2,   2,   2}  // i4CWFDF_LUTThr
        },
/******************************************************************************/
//Feature name:
//Neutral parent block threshold for noise reduction
//variable name: i4NonNeutral and  i4NRThr[AWB_LIGHTSOURCE_NUM]
//default value: N/A
//data range: 0 ~ 100 for each element in the table
//These look-up table defines threahold ratio based on LV for no neutral light and all
//light source. When the neutral block of these light source is below threshold,
//it is not accumulated for statistics
//Ex. Neutral block number[CWF] >= Parent block number * i4NRThr[CWF][LV]
/******************************************************************************/
        // AWB light neutral noise reduction for outdoor
        {
            //LV 0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18
            {    6,   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,  10,  10,  10,  10,  10,  10,  10,  10},  //Nonreliable 
            {
                //LV 0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18
                {    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},  //Strobe 
                {    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},  //Tungsten 
                {    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},  //WF 
                {    0,   0,   0,   0,   0,   3,   5,   5,   5,   5,   5,  10,  10,  10,  10,  10,  10,  10,  10},  //Fluorescent 
                {    0,   0,   0,   0,   0,   3,   5,   5,   5,   5,   5,  10,  10,  10,  10,  10,  10,  10,  10},  //CWF 
                {    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   2,   2,   2,   2,   2,   2,   2,   2},  //Daylight 
                {    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},  //Shade 
                {    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   5,  10,  10,  10,  10,  10,  10,  10,  10},  //DF 
            },
        },
        // AWB feature detection
        {
/******************************************************************************/
//Feature name:
//Property description for sunset feature and daylight locus estimation for daylight
//and also parameters used to define possible shade area (sunset area) inside daylight window
//variable name: i4Enable
//default value: 1
//data range: 0 and 1
//0 means disable sunset feature detection
//1 means enable sunset feature detection
//variable name: i4LVThr_L
//default value: N/A
//data range: 0 ~ 180
//variable name: i4LVThr_H
//default value: N/A
//data range: 0 ~ 180
//variable name: i4SunsetCountThr
//default value: N/A
//data range: 0 ~ 432
//variable name: i4SunsetCountRatio_L
//default value: N/A
//data range: 0~256 (unit: 256, 256 means 100 precent)
//variable name: i4SunsetCountRatio_H
//default value: N/A
//data range: 0~256 (unit: 256, 256 means 100 precent)
//variable name: i4Sunset_BoundXr_Thr
//shade area right bound in Xr direction. left bound is the same as daylight window left bound
//default value: N/A
//variable name: i4Sunset_BoundYr_Thr
//shade area upper bound in Yr direction. lower bound is the same ad daylight winodw lower bound
//default value: N/A
//(1)sunset feature detection condition is ((a) && (b) && (c))
//   (a) light source is daylight
//   (b) i4Enable = 1
//   (c) Current LV > i4LVThr_L
//(2)sunset compensation for new daylight locus gain prediction in daylight when sunset feature exists
//   Condition for new daylight locus gain prediction is ((a) && (b) && (c))
//   (a) Current LV > i4LVThr_L
//   (b) parent block number > i4SunsetCountThr
//   (c) Sunset count > parent block number * i4SunsetCountRatio_L
//(3)new daylight locus gain calculation
//   i4SunsetCount_L_Thr = parent block number * i4SunsetCountRatio_L / 256
//   i4SunsetCount_H_Thr = parent block number * i4SunsetCountRatio_H / 256
//   Temp gain = interpolate(sunset count, i4SunsetCount_L_Thr, i4SunsetCount_H_Thr,
//    daylight locus gain of current LV, sunset target gain)
//   Sunset compensated daylight locus gain  = interpolate( current LV, i4LVThr_L,
//    i4LVThr_H, daylight locus gain of current LV, Temp gain)
//	 y = interploate(x, x1, x2, y1, y2), where 
//  y = y2, if x >= x2
//	 y = y1, if x < x1
//  y = (x - x1) / (x2 - x1) * (y2 - y1), if x1 <= x < x2
/******************************************************************************/
             // Sunset Prop
             {
                   1, // i4Enable
                 120, // i4LVThr_L
                 130, // i4LVThr_H
                  10, // i4SunsetCountThr
                   0, // i4SunsetCountRatio_L
                 171, // i4SunsetCountRatio_H
                 {
                       88, // i4Sunset_BoundXr_Thr
                     -453, // i4Sunset_BoundYr_Thr
                 },
             },
/******************************************************************************/
//Feature name:
//Property description for CWF / F feature and the daylight locus probability estimation
//for CWF / F parameters used to define possible shade area (soil area) inside CWF / F window
//variable name: i4Enable
//default value: 1
//data range: 0 and 1
//0 means disable shade CWF / F feature detection
//1 means enable shade CWF / F feature detection
//variable name: i4Mode
//default value: 1
//data range: 0 and 1
//0 means to calculate the probability by distance
//1 means to calculate the probability by vertex
//variable name: i4LVThr_L
//default value: N/A
//data range: 0 ~ 180
//variable name: i4LVThr_H
//default value: N/A
//data range: 0 ~ 180
//variable name: i4DaylightProb
//default value: N/A
//data range: 0~256 (unit: 256, 256 means 100 precent)
//variable name: Area X
//shade area left bound in Xr direction. right bound is the same as window left bound
//default value: N/A
//variable name: Area Y
//shade area left bound in Yr direction. right bound is the same as window left bound
//default value: N/A
//variable name: Vertex X
//soil area vertex Xr position
//default value: N/A
//variable name: Vertex Y
//soil area vertex Yr position
//default value: N/A
//(1)shade CWF feature detection condition is ((a) && (b) && (c))
//   (a) light source is daylight
//   (b) i4Enable = 1
//   (c) Current LV > i4LVThr_L
//(2)new daylight locus probability calculatio
//   Temp probability = interpolate(shade F count, 0, parenet block number of F,
//   daylight locus probability of CWF / F, shade CWF / F probability)
//   New daylight locus probability  = interpolate( current LV, i4LVThr_L, i4LVThr_H,
//   daylight locus probability of fluorescent, Temp probability)
//	 y = interploate(x, x1, x2, y1, y2), where 
//  y = y2, if x >= x2
//	 y = y1, if x < x1
//  y = (x - x1) / (x2 - x1) * (y2 - y1), if x1 <= x < x2
/******************************************************************************/
             // SubWindow F Detection
             {
                   1, // i4Enable
                   1, // i4Mode
                  50, // i4LVThr_L
                  90, // i4LVThr_H
                 128, // i4DaylightProb
                 {
                     -123, // Area X
                     -439, // Area Y
                 },
                 {
                      -44, // Vertex X
                     -454, // Vertex Y
                 },
             },
             // SubWindow CWF Detection
             {
                   1, // i4Enable
                   1, // i4Mode
                  50, // i4LVThr_L
                  90, // i4LVThr_H
                 192, // i4DaylightProb
                 {
                     -123, // Area X
                     -505, // Area Y
                 },
                 {
                      -30, // Vertex X
                     -531, // Vertex Y
                 },
             },
/******************************************************************************/
//Feature name:
//Property description for extra color and blending with a specified gain
//variable name: i4Enable
//default value: 1
//data range: 0 and 1
//0 means disable extra color feature detection
//1 means enable extra color feature detection
//variable name: rGain
//default value: N/A , unit: 512 = 1x
//The specified gain for extra color detected
//variable name: i4RightBound, i4LeftBound, i4UpperBound, i4LowerBound
//The XrYr window for extra color detection
//default value: N/A
//variable name: rLv
//default value: N/A
//data range: 0 ~ 180
//variable name: rCount
//default value: N/A
//data range: 0 ~ 432
//variable name: rWeighting
//default value: N/A
//data range: 0 ~ 100
//Blending weighting for specified gain
/******************************************************************************/
             // Extra color Detection
             {
                 0, // i4Enable
                 // Extra Color AWB gain
                 {
                        0, // GainR
                        0, // GainG
                        0, // GainB
                 },
                 // Extra Color area
                 {
                        0, // i4RightBound
                        0, // i4LeftBound
                        0, // i4UpperBound
                        0, // i4LowerBound
                 },
                 {   0,   0}, // rLv
                 {   0,   0}, // rCount
                 {   0,   0}, // rWeighting
             },
         },
/******************************************************************************/
//Feature name:
//non-neutral probability for spatial and temporal weighting look-up table
//variable name: i4LUT
//default value: N/A
//data range: 0 ~ 100
//The look-up table defines the weighting to use spatial temporal gain
/******************************************************************************/
        // AWB non-neutral probability for spatial and temporal weighting look-up table (Max: 100; Min: 0)
        {
            //LV 0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18
            {    0,  33,  66, 100, 100, 100, 100, 100, 100, 100, 100,  70,  30,  20,  10,   0,   0,   0,   0}
        },
/******************************************************************************/
//Feature name:
//Look-up table according to current LV for each light source to determine the percentage of statistic gain usage
//variable name: i4LUT
//default value: N/A
//data range: 0 ~ 100
//Ex. EQ gain of A = (i4LUT[A][LV] * statistics gain of A + (100 - i4LUT[A][LV]) * spatial gain) / 100
/******************************************************************************/
        // AWB daylight locus probability (Statistic Weighting) look-up table (Max: 100; Min: 0)
        {
            //LV 0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18
            {  100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  50,  25,   0,   0,   0,   0}, // Strobe
            {   95,  95,  95,  95,  95,  95, 100, 100, 100, 100, 100, 100, 100,  75,  50,  25,   0,   0,   0}, // Tungsten
            {   95,  95,  95,  95,  95,  95, 100, 100, 100, 100, 100,  75,  50,  25,  25,  25,   0,   0,   0}, // Warm fluorescent
            {  100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  95,  75,  50,  25,  25,  25,   0,   0,   0}, // Fluorescent
            {   90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  80,  55,  30,  30,  30,  30,   0,   0,   0}, // CWF
            {  100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  75,  50,  40,  30,  20}, // Daylight
            {  100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  75,  50,  25,   0,   0,   0,   0}, // Shade
            {   90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  80,  55,  30,  30,  30,  30,   0,   0,   0}  // Daylight fluorescent
        },
/******************************************************************************/
//Feature name:
//Light source probability (P1) look-up table according to current LV for each light source
//variable name: i4LUT
//default value: N/A
//data range: 0 ~ 100
//Final probability P = P0 * P1 * P2, where P0 is paraent block number percentage,
//P2 value is look-up by GMOffset in tungsten, WF, Shade
/******************************************************************************/
        // AWB Light source probability (P1) look-up table (Max: 100; Min: 0)
        {
            //LV 0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18
            {  100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100}, // Strobe
            {  100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  66,  33,   1,   1,   1,   1,   1}, // Tungsten
            {  100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  66,  33,   1,   1,   1,   1,   1}, // Warm fluorescent
            {  100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  66,  66,  66,  66,  66,  66,  66}, // Fluorescent
            {  100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  66,  33,   1,   1,   1,   1,   1}, // CWF
            {  100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100}, // Daylight
            {  100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  66,  33,   1,   1,   1}, // Shade
            {  100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  66,  33,   1,   1,   1,   1,   1}  // Daylight fluorescent
        },
/******************************************************************************/
//Feature name:
//AWB convergence behavior control
//variable name: i4Speed
//the higher the value, the faster the convergence speed
//default value: 50
//data range: 0 ~ 100
//variable name: i4StableThr
//A stable threshold to control AWB to converge or not.When
//((currentRgain - targetRgain)^2 + (currentBgain - targetBgain)^2) is larger than the i4StableThr,
//AWB will converge and not converge if smaller than i4StableThr.
//default value: 225
//variable name: i4ToTargetEnable
//default value: 1
//data range: 0 and 1, 0 means might not coverge to target gain and 1 means always converge to target gain in preview
/******************************************************************************/
        // AWB convergence parameter 
        {
             10, // i4Speed: Convergence speed: (0 ~ 100)
            225, // i4StableThr: Stable threshold ((currentRgain - targetRgain)^2 + (currentBgain - targetBgain)^2), WB gain format: 4.9
              1  // i4ToTargetEnable: Preview converge to target enable.
        },
/******************************************************************************/
//Feature name:
//Preference color is used control WB convergence degree for Tungsten, WF and Shade
/******************************************************************************/
        // Preference color 
        {
/******************************************************************************/
//Feature name:
//Preference color is used control WB convergence degree for Tungsten, WF and Shade
//variable name: Offset threshold
//do full compensation when current distance from the origin to the white point(log(R/G),
//log(B/G)) is under Offset threshold , and do partial compensation when
//the distance is above Offset threshold
//default value: N/A
//data range: 0 ~ 10000
/******************************************************************************/
		     //Offset threshold
		     {
                4500, // Tungsten
                4500, // WF
                 910  // Shade
		     },
/******************************************************************************/
//Feature name:
//LV thrshold for daylight locus partial comensation new offset calculation
//variable name: rOffsetRatioLv
//default value: N/A
//Daylight locus target offset = interpolate(current LV, i4ThrL, i4ThrH, Daylight
//locus temp offset*(ratio look-up in i4LUT for daylight locus offset partial compensation),
//Daylight locus temp offset)
//	 y = interploate(x, x1, x2, y1, y2), where 
//  y = y2, if x >= x2
//	 y = y1, if x < x1
//  y = (x - x1) / (x2 - x1) * (y2 - y1), if x1 <= x < x2
//data range: 0 ~ 10000
/******************************************************************************/
		     //Offset ratio LV
		     {
                {  50, 100}, // Tungsten
                {  50, 100}, // WF
                {  50, 100}  // Shade
		     },
/******************************************************************************/
//Feature name:
//Use daylight locus new offset (0~10000) as index to get daylight locus target offset ratio (0~100)
//variable name: i4LUT
//default value: N/A
//data range: 0 ~ 100
//New offset = Current offset (distance of current white point - threshold defined
//for corresponding illuminant light source) * interpolation in i4LUT by current offset.
/******************************************************************************/
		     //AWB daylight locus target Offset ratio LUT
		     {
		         //Tungsten
		         {
		             { //  0  500 1000 1500 2000 2500 3000 3500 4000 4500 5000 5500 6000 6500 7000 7500 8000 8500 9000 9500 10000
		                  50,  50,  50,  50,  50,  50,  50,  50,  50,  50,  50,  55,  60,  65,  70,  75,  80,  85,  90,  95, 100
		             },
		         },
		         //Warm Flurorescent
		         {
		             { //  0  500 1000 1500 2000 2500 3000 3500 4000 4500 5000 5500 6000 6500 7000 7500 8000 8500 9000 9500 10000
		                  50,  50,  50,  50,  50,  50,  50,  50,  50,  50,  50,  55,  60,  65,  70,  75,  80,  85,  90,  95, 100
		             },
		         },
		         //Shade
		         {
		             { //  0  500 1000 1500 2000 2500 3000 3500 4000 4500 5000 5500 6000 6500 7000 7500 8000 8500 9000 9500 10000
		                  50,  50,  50,  50,  50,  50,  50,  50,  50,  50,  50,  50,  50,  50,  50,  50,  50,  50,  50,  50,  50
		             },
		         },
		     },
/******************************************************************************/
//Feature name:
//New target offset remapping LUT table. It is used to adjust white balance partial
//compensation degree when the white point distance in log(R/G)-log(B/G) domain is larger than
//threshold (i4OffsetThr) for tungsten, warm fluorescent and shade
//variable name: i4LUTSize, LUT table dimension size
//variable name: i4LUTIn: Look-up table input of daylight locus offset
//variable name: iLUTOut: Loop-up table output of daylight locus offset
/******************************************************************************/
		     //Daylight locus offset LUTs
		     {
		         //Tungsten
		         {
		             {   0,  500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000},
		             {   0,  350,  800, 1222, 1444, 1667, 1889, 2111, 2333, 2556, 2778, 3000, 3222, 3444, 3667, 3889, 4111, 4333, 4556, 4778, 5000}
		         },
		         //WF
		         {
		             {   0,  500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000},
		             {   0,  350,  700, 1000, 1444, 1667, 1889, 2111, 2333, 2556, 2778, 3000, 3222, 3444, 3667, 3889, 4111, 4333, 4556, 4778, 5000}
		         },
		         //Shade
		         {
		             {   0,  500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000},
		             {   0,  500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000}
		         },
		     },
/******************************************************************************/
//Feature name:
//Green and Magenta offset remapping look-up table for partial compensation offset
//for tungsten, warm fluorescent and shade
//variable name: i4OffsetMode
//default value: N/A
//data range: 0 and 1
//0 means not to disable offset remapping
//1 means to enable offset remapping
//variable name: i4LutM
//default value: N/A
//data range: 0 ~ 10000
//Magenta offset remaping look-up table
//variable name: i4LUTG
//default value: N/A
//data range: 0 ~ 10000
//Green offset remaping look-up table
/******************************************************************************/
		     //AWB Green/Magenta offset threshold
		     {
		         //Tungsten
		         {
		             0, //0: disabe 1: enable offset when GM offset is large than threshold
		             { // LUT: use daylight locus offset (0~10000) as index to get magenta offset threshold
		               //  0   500  1000  1500  2000  2500  3000  3500  4000  4500  5000  5500  6000  6500  7000  7500  8000  8500  9000  9500  10000
		                   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0
		             },
		             { // LUT: use daylight locus offset (0~10000) as index to get green offset threshold
		               //  0   500  1000  1500  2000  2500  3000  3500  4000  4500  5000  5500  6000  6500  7000  7500  8000  8500  9000  9500  10000
		                   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0
		             },
		         },
		         //Warm Fluorescent
		         {
		             1, //0: disabe 1: enable offset when GM offset is large than threshold
		             { // LUT: use daylight locus offset (0~10000) as index to get magenta offset threshold
		               //  0   500  1000  1500  2000  2500  3000  3500  4000  4500  5000  5500  6000  6500  7000  7500  8000  8500  9000  9500  10000
		                1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500
		             },
		             { // LUT: use daylight locus offset (0~10000) as index to get green offset threshold
		               //  0   500  1000  1500  2000  2500  3000  3500  4000  4500  5000  5500  6000  6500  7000  7500  8000  8500  9000  9500  10000
		                1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500
		             },
		         },
		         //Shade
		         {
		             0, //0: disabe 1: enable offset when GM offset is large than threshold
		             { // LUT: use daylight locus offset (0~10000) as index to get magenta offset threshold
		               //  0   500  1000  1500  2000  2500  3000  3500  4000  4500  5000  5500  6000  6500  7000  7500  8000  8500  9000  9500  10000
		                   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0
		             },
		             { // LUT: use daylight locus offset (0~10000) as index to get green offset threshold
		               //  0   500  1000  1500  2000  2500  3000  3500  4000  4500  5000  5500  6000  6500  7000  7500  8000  8500  9000  9500  10000
		                   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0
		             },
		         },
		     },
/******************************************************************************/
//Feature name:
//Light source probability (P2) look-up table according to green or magenta offset
//of corresponding light source white point for tungsten, warm fluorescent and shade
//variable name: i4LutM
//default value: N/A
//data range: 0 ~ 256 (256 is 100 percent)
//variable name: i4LutG
//default value: N/A
//data range: 0 ~ 256 (256 is 100 percent)
//When the corresponding white point in log(R/G)-log(B/G) domain is above the
//daylight locus line, use i4LUTM else use i4LUTG for look-up table.
//Final probability P = P0 * P1 * P2, where P0 is parent block number percentage,
//P1 is look-up table accordint to LV for each light source
/******************************************************************************/
		     // AWB light source weight LUT
		     {
		         //Tungsten
		         {
		             { // LUT: use magenta offset (0~1000) as index to get tungsten weight (x/256)
		               // 0   100  200  300  400  500  600  700  800  900 1000 
		                 256, 256, 256, 256, 256, 256, 256, 128,  64,  32,  16
		             },
		             { // LUT: use green offset (0~1000) as index to get tungsten weight (x/256)
		               //  0  100  200  300  400  500  600  700  800  900 1000 
		                 256, 256, 256, 256, 256, 256, 256, 128,  64,  32,  16
		             },
		         },
		         //Warm Fluorescent
		         {
		             { // LUT: use magenta offset (0~1000) as index to get tungsten weight (x/256)
		               //  0  200  400  600  800 1000 1200 1400 1600 1800 2000 
		                 256, 256, 256, 256, 256, 256, 256, 192,  96,  40,  16
		             },
		             { // LUT: use green offset (0~1000) as index to get tungsten weight (x/256)
		               //  0  200  400  600  800 1000 1200 1400 1600 1800 2000 
		                 256, 256, 256, 256, 256, 256, 256, 192,  96,  40,  16
		             },
		         },
		         //Shade
		         {
		             { // LUT: use magenta offset (0~1000) as index to get tungsten weight (x/256)
		               //  0  100  200  300  400  500  600  700  800  900 1000 
		                 256, 256, 256, 256, 192, 128,  64,  32,  16,  16,  16
		             },
		             { // LUT: use green offset (0~1000) as index to get tungsten weight (x/256)
		               //  0  100  200  300  400  500  600  700  800  900 1000 
		                 256, 256, 256, 256, 192, 128,  64,  32,  16,  16,  16
		             },
		         },
            },
        },
        // AWB advance function 
        {
            //AWB Refinement
            {
/******************************************************************************/
//Feature name:
//Property for low light stability
//variable name: i4CbMinThr
//default value: N/A
//data range: 0 ~ 25
//The minimum value for valid statistic block
/******************************************************************************/
                0,    //i4CbMinThr
/******************************************************************************/
//Feature name:
//Modes of spatial gain for each light source, non-reliable and temporal initial mode
//variable name: i4GeneralMode
//default value: N/A
//data range: 0 ~ 2 
//Each light source 
//variable name: i4NonReliableMode
//default value: N/A
//data range: 0 ~ 2 
//variable name: i4TempInitMode
//default value: N/A
//data range: 0 ~ 2 
//Mode 0 : LV > ThrH use rSpatial_H gain
//         ThrL < LV < ThrH use interpolated gain from rSpatial_L and rSpatial_H.
//         LV < ThrL use rSpatial_L gain
//Mode 0 : LV > ThrH use rSpatial_H gain 
//         ThrL < LV < ThrH use interpolated gain from rSpatial_L and rSpatial_H.
//         LV < ThrL use rTemporal_General gain
//Mode 2 : LV > ThrH use spatial gain H
//         ThrL < LV < ThrH use interpolated gain from rSpatial_L and rSpatial_H.
//         TempLv < LV < ThrL use rSpatial_L gain
//         LV < TempLv use interpolated gain from rSpatial_L and rTemporal_General.
//variable name: i4ThrTemp
//default value: N/A
//data range: 0 ~ 180 
//This value is used when non-reliable and temporal initial mode using mode 2
//variable name: i4TempLv
//default value: N/A
//data range: 0 ~ 180 
//These value are used normal mode using mode 2
/****************************************************************************/
                 // Spatial gain 
                 {
                      0, //i4GeneralMode        // 0:Spatial L/H; 1:Spatial L/H and Temproal low LV; 2:Spatial L/H and Temproal
                      0, //i4NonReliableMode    // 0:Spatial L/H; 1:Spatial L/H and Temproal low LV; 2:Spatial L/H and Temproal
                      1, //i4TempInitMode       // 0:Spatial L/H; 1:Spatial L/H and Temproal low LV; 2:Spatial L/H and Temproal
                      0, //i4ThrTemp       
                    {
                          0, //Strobe
                          0, //T     
                          0, //WF    
                          0, //F     
                          0, //CWF   
                          0, //D     
                          0, //Shade 
                          0, //DF    
                    }
                 },
/******************************************************************************/
//Feature name:
//Property of P0 stability
//variable name: i4Enable
//default value: N/A
//data range: 0 ~ 1 
//0 means disable P0 stabilized function
//1 means enable P0 stabilized function
//variable name: i4PbRatio
//default value: N/A
//data range: 0 ~ 100 
//variable name: i4Range
//default value: N/A
//data range: 0 ~ 32 
//variable name: i4Clip
//default value: N/A
//data range: 0 ~ 10 
/****************************************************************************/
                 // P0 Stability 
                 {
                      0, //i4Enable
                     20, //i4PbRatio
                     32, //i4Range
                      8  //i4Clip
                 },
/******************************************************************************/
//Feature name:
//P2 is effective only for high LV, enough neutral parent block and 
//P0 is in resonable range
//variable name: i4Enable
//default value: N/A
//data range: 0 ~ 1 
//0 means disable P2 stabilized function
//1 means enable P2 stabilized function
//variable name: i4PbRatio
//default value: N/A
//data range: 0 ~ 100 
//The P2 is effective when valid neutral block larger than this value
//variable name: i4LvThr
//default value: N/A
//The P2 is effective when LV is larger than this value
//data range: 0 ~ 32 
//variable name: i4P0Thr1
//default value: N/A
//data range: 0 ~ 10 
//variable name: i4P0Thr2
//default value: N/A
//data range: 0 ~ 10 
//The P2 is fully effective when P0 > i4P0Thr1. The P2 is not effective if P0 > i4P0Thr2
//The P2 is partial effective when i4P0Thr2 > P0 > i4P0Thr1.
/****************************************************************************/
                 // P2 Stability 
                 {
                      0, //i4Enable
                     30, //i4PbRatio
                     20, //i4LvThr
                     80, //i4P0Thr1
                     95  //i4P0Thr2
                 },
/******************************************************************************/
//Feature name:
//Property of Preference Color stability
//variable name: i4Enable
//default value: N/A
//data range: 0 ~ 1 
//0 means disable preference color stability function
//1 means enable preference color stability function
//variable name: rOffsetSmooth
//default value: N/A
//data range: 0 ~ 1024
//These value are used for smooth transient region to preserver preference color.
/****************************************************************************/
                 // Preference Color Stability 
                 {
                      0,    //i4Enable
                     {
                        512,
                        512,
                        512 
                     }
                 },
            },
/******************************************************************************/
//Feature name:
//Property of statistic gain constraint for each light source
//variable name: i4Enable
//default value: N/A
//data range: 0 ~ 1 
//0 means disable statistic gain constraint
//1 means enable statistic gain constraint
//variable name: i4LimitY
//default value: N/A
//These value define the reasonable distance from daylight locus.The values of Tusteng, Fluorescent, Daylight
//and shade are the upper boundary. The values of Warm Fluorescent, CWF and DF are the lower boundary
//variable name: i4WeightReduce
//default value: N/A
//data range: 0 ~ 16
//If the statistic location is far away from daylight locus. It will reduce the statistic gain weighting
/******************************************************************************/
            // rStatLimit
            {
                0,    //i4Enable
                // i4LimitY[AWB_LIGHT_NUM]
                {
                       0,  //Strobe
                    -386,  //T
                    -533,  //WF
                    -415,  //F
                    -521,  //CWF
                    -404,  //Daylight
                    -404,  //Shade
                    -521,  //DF
                },
                // i4WeightReduce[AWB_LIGHT_NUM]
                {
                     0,  //Strobe
                     4,  //Tungsten
                     4,  //WF
                     4,  //F
                     4,  //CWF
                     4,  //Daylight
                     4,  //Shade
                     4,  //DF
                }
            },
/******************************************************************************/
//Feature name:
//Face AWB refer face information for AWB modification, and at the same time,
//AWB also can help skin tone to approach the preference a little
/******************************************************************************/
            // Face AWB
            {
/******************************************************************************/
//Feature name:
//Face AWB basic control & basic statistic tuning
//variable name: i4Enable
//default value: 0
//unit: N/A
//data range: 0 and 1
//0 means disable face AWB
//1 means enable face AWB
//variable name: i4StatAvoidFaceArea
//default value: 0
//unit: N/A
//data range: 0 and 1
//0 means normal AWB statistic
//1 means normal AWB statistic will exclude face area
//variable name: i4FaceWinRatio
//default value: 100
//unit: percentage
//data range: 50 ~ 400
//This parameter can change the original FD window size
//variable name: u4FaceCentralWeight
//default value: 1
//unit: N/A
//data range: 0 ~
//This parameter can enhance FD window central weighting, the formula is
//(Original FD info + Central FD info * weighting) / (weighting + 1)
//variable name: rFD_RGB_Bound
//default value: {1, 254}
//unit: N/A
//data range: 0 ~ 255
//If face R or G or B is lower than Low Bound, or higher than High bound,
//this face will be ignored for using
/******************************************************************************/
                  0,    //i4Enable
                  0,    //i4StatAvoidFaceArea
                100,    //i4FaceWinRatio  50~400
                  1,    //u4FaceCentralWeight  0=>0, 1=>1/2, 2=>2/3, 3=>3/4 ...
                // rFD_RGB_Bound
                {
                      1,    //u4LowBound
                    254,    //u4HiBound
                },
/******************************************************************************/
//Feature name:
//Face AWB preference skin tone target
//variable name: rRefTarget
//default value: calibrated from calibration library
//unit: 1000 = 1x
//data range: 0 ~
//This parameter is the preference skin tone of face {R/G B/G}, which is after
//applying AWB gain, and face AWB algo will transfer to hue for reference
//There are 3 sets of reference target for hi, mid, low color temperature
//variable name: rDynamicTarget
//	(a)i4LV_LowThr & i4LV_HiThr are LV threshold used for mid & Hi CT ref target
//		 interpolation
//      (when LV is higher, the hi CT probabilirt is higher)
//      unit: 10 = 1x
//      data range: 0 ~ 180
//	(b)i4FaceRB_LowThr & i4FaceRB_HiThr are face processed raw R/B threshold,
//     which are used for mid & low CT ref target interpolation
//	    (when face R/B is higher, the low CT probability is higher)
//      unit: 1000 = 1x
//      data range: 0 ~
//	(c)i4TempoWeight is used for temporal smooth switch ref target, the formula is
//      (Current Ref target + Temporal Ref target * Tempoweight) / (Tempoweight + 1)
//      unit: N/A
//      data range: N/A
/******************************************************************************/
                // rRefTarget
                {
                     {1312,  854},   //rHiCT Unit=1000 for outdoor
                     {1203,  922},   //rMidCT Unit=1000 for indoor office
                     {1233,  854}    //rLowCT Unit=1000 for low color temperature
                },
                // rDynamicTarget
                {
                      90,    //i4LV_LowThr for Indoor & Outdoor dynamic
                     130,    //i4LV_HiThr for Indoor & Outdoor dynamic
                    2300,    //i4FaceRB_LowThr for Indoor Mid CT & Low CT dynamic, Unit = 1000
                    3000,    //i4FaceRB_HiThr for Indoor Mid CT & Low CT dynamic, Unit = 1000
                       7,    //i4TempoWeight for Temporal Target weighting: 0=>0, 1=>1/2, 2=>2/3, 3=>3/4 ...
                },
/******************************************************************************/
//Feature name:
//Face AWB algo will generate R/G & B/G ratio to modify original AWB gain
//variable name: i4SameHueKeepOriGain_En
//default value: 0
//unit: N/A
//data range: 0 and 1
//0 means although current face hue is same with ref target, algo will still
//force generating new awb gain, and this can make AWB more smoothly
//1 means if current face hue is same with ref target hue, the ratio will be
//forced to 1x, but might cause AWB not smooth
//variable name: rOutdoorProb
//	(a)rLV use LV to interpolate from i4LV_Low to i4LV_Hi to get Probability
//	   from i4Prob_Low to i4Prob_Hi
//	   unit: LV 10 = 1x, Prob = percentage
//	   data range: LV = 0 ~ 180, Prob = 0 ~ 100
//	(b)rDaylight use Daylight P0 to interpolate from i4P0_Low to i4P0_Hi to
//	   get Probability from i4Prob_Low to i4Prob_Hi
//	   unit:  percentage
//	   data range: D_P0 = 0 ~ 100, Prob = 0 ~ 100
//	(c)rShade use Shade P0 to interpolate from i4P0_Low to i4P0_Hi to get
//	   Probability from i4Prob_Low to i4Prob_Hi
//	   unit: P0 & Prob is percentage, newoffset = N/A
//	   data range: D_P0 = 0 ~ 100, Prob = 0 ~ 100, newoffset = 0 ~
//Above parameters will choose the maximum for outdoor probability. When
//outdoor probability is higher, the face AWB gain will be more bluish,
//otherwise it will be more yellowish.
//variable name: i4GainRatioRestrictLUT
//Unit: percentage
//data range: 0 ~ 100
//This LV LUT will restrict face awb gain can only change the normal awb gain
//in specific percentage range. If this restrict percentage is higher, the
//face awb effect will be more aggressive, but cause normal awb seriously
//color bias and more non-smooth. If this restrict percentage is lower,
//the face awb effect will be reduced, but more smoothly.
//Face AWB gain = Original AWB gain * Restrict Gain Ratio
/******************************************************************************/
                0,    //i4SameHueKeepOriGain_En  If ori face hue is same with target, keep ori AWB gain
                // rOutdoorProb
                {
                    // rLV
                    {
                         90,    //i4LV_Low
                        120,    //i4LV_Hi
                          0,    //i4Prob_Low
                        100     //i4Prob_Hi
                    },
                    // rDaylight
                    {
                         40,    //i4P0_Low
                         90,    //i4P0_Hi
                          0,    //i4Prob_Low
                         50     //i4Prob_Hi
                    },
                    // rShade
                    {
                          40,    //i4P0_Low
                          90,    //i4P0_Hi
                           0,    //i4Prob_Low
                          75,    //i4Prob_Hi
                        1500     //i4NewOffset_Hi
                    }
                },
                // i4GainRatioRestrictLUT
                //LV0  1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18
                {  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  4,  3,  2,  2},
/******************************************************************************/
//Feature name:
//A set of gain applied after face awb algorithm calculation output gain based
//on LV, just like normal awb preference gain LUT. It is used to optimize and
//fine tune, but if it is set too aggressive might cause face AWB not smooth.
//variable name: {i4R, i4G, i4B} gain table from LV0 to LV18.
//default value: {512, 512, 512}
//unit: 512 = 1x
//data range: 0 ~ 4096
/******************************************************************************/
                // rPrefGain
                {
                  //    LV0             LV1             LV2             LV3             LV4             LV5             LV6             LV7             LV8             LV9 
                  {512, 512, 512},{512, 512, 512},{512, 512, 512},{512, 512, 512},{512, 512, 512},{512, 512, 512},{512, 512, 512},{512, 512, 512},{512, 512, 512},{512, 512, 512},
                  //	  LV10 	          LV11            LV12            LV13            LV14            LV15            LV16            LV17            LV18 
                  {512, 512, 512},{512, 512, 512},{512, 512, 512},{512, 512, 512},{512, 512, 512},{512, 512, 512},{512, 512, 512},{512, 512, 512},{512, 512, 512},
                },
/******************************************************************************/
//Feature name:
//Get face probability
//	(a)rFaceAWBProb0 is refering face size compare to 120x90. If face size ratio
//	   is higher, the face prob0 must be higher. It uses face size ratio to
//	   interpolate from i4FaceSizeRatio_Low to i4FaceSizeRatio_Hi to get
//	   Probability0 from i4Prob0_Min to i4Prob0_Max.
//	   unit: size ratio = percentage*1000, Prob0 = percentage
//	   data range: size ratio 0 ~ 100000, Prob0 0 ~ 100
//	(b)rFaceAWBProb1LUT is a LV LUT for face probability1.
//	   unit: percentage
//	   data range: 0 ~ 100
//	(c)rFaceAWBProb2 is refering face processed raw R/G information. If the R/G
//    is out of range, the face prob2 must be reduced. It uses face processed 
//	   raw to interpolate from i4FaceAreaRG_LowTh to (i4FaceAreaRG_LowTh -
//	   i4FaceAreaRG_Tolerance) or interpolate from i4FaceAreaRG_HiTh to 
//	   (i4FaceAreaRG_HiTh + i4FaceAreaRG_Tolerance) to get probability2 from
//	   i4Prob2_Max to i4Prob2_Min.
//	   unit: R/G 1000 = 1x, Prob = percentage
//	   data range: R/G N/A, Prob 0 ~ 100
//	(d)rFaceAWBProb3 is totally same with rFaceAWBProb2, just change to refer
//	   face processed raw B/G information.
//Face probability = face Prob0 * face Prob1 * face Prob2 * face Prob3
//Face AWB target gain = ((original AWB gain * (1 - face Probability)) +
//                       (face AWB gain * face Probability)) / 100
/******************************************************************************/
                // rFaceAWBProb0
                {
                    3900,    //i4FaceSizeRatio_Low
                    4900,    //i4FaceSizeRatio_Hi
                       0,    //i4Prob0_Min
                     100     //i4Prob0_Max
                },
                // rFaceAWBProb1LUT
                //LV0   1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18
                {  50,  75, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  80,  60,  40},
                // rFaceAWBProb2
                {
                     650,    //i4FaceAreaRG_LowTh Unit=1000
                    1200,    //i4FaceAreaRG_HiTh Unit=1000
                     500,    //i4FaceAreaRG_Tolerance Unit=1000
                       0,    //i4Prob2_Min
                     100     //i4Prob2_Max
                },
                // rFaceAWBProb3
                {
                     250,    //i4FaceAreaBG_LowTh Unit=1000
                     800,    //i4FaceAreaBG_HiTh Unit=1000
                     500,    //i4FaceAreaBG_Tolerance Unit=1000
                       0,    //i4Prob3_Min
                     100     //i4Prob3_Max
                },
/******************************************************************************/
//Feature name:
//Face spatial compensation when face size is small and neutral parent block
//number is very few.
//	(a)rSpatial_P0 is refering face size compare to 120x90. If face size ratio
//	   is higher, the spatial prob0 will be lower. It uses face size ratio to
//	   interpolate from i4FaceSizeRatio_Hi to i4FaceSizeRatio_Low to get
//	   Probability0 from i4Prob0_Min to i4Prob0_Max.
//	   unit: size ratio = percentage*1000, Prob0 = percentage
//	   data range: size ratio 0 ~ 100000, Prob0 0 ~ 100
//	(b)rSpatial_P1 is refering whole image neutral parent block number, except
//		 face area. If neutral parent block num is higher, the spatial prob1 will
//     be lower. It uses neutral parent block number to interpolate from
//     i4NeutralParentBlkNum_Hi to i4NeutralParentBlkNum_Low to get Probability1
//     from i4Prob1_Min to i4Prob1_Max.
//	   unit: neutral parent block num = N/A, Prob1 = percentage
//	   data range: neutral parent block num 0 ~ 120*90, Prob1 0 ~ 100
//	(c)i4LV_Low & i4LV_LHi are used for awb spatial gain caculation. It is
//	   interpolated from i4LV_Low to i4LV_LHi to get face awb spatial gain from
//     DaylightLocus_L to DaylightLocus_H.
//	   unit: 10 = 1x
//	   data range: 0 ~ 180
//Face spatial probability = spatial Prob0 * spatial Prob1
//Face AWB target gain = ((face awb target gain * (1 - face spatial probability)) +
//                       (face AWB spatial gain * face spatial Probability)) / 100
/******************************************************************************/
                // rSpatial
                {
                	 // rSpatial_P0
                	 {
                    	 3900,    //i4FaceSizeRatio_Hi  Unit=1000
                    	  500,    //i4FaceSizeRatio_Low Unit=1000
                    	    0,    //i4Prob0_Min
                    	   50     //i4Prob0_Max
                	 },
                	 // rSpatial_P1
                	 {
                    	   75,    //i4NeutralParentBlkNum_Hi
                    	   15,    //i4NeutralParentBlkNum_Low
                    	    0,    //i4Prob1_Min
                    	   50,    //i4Prob1_Max
                	 },
                     70,    //i4LV_Low
                    110,    //i4LV_LHi
              	  },
/******************************************************************************/
//Feature name:
//Face awb temporal smooth mechanism
//variable name: rTempoSmooth
//	(a)i4Speed is control the temporal IIR step. If it sets to 10, the temporal
//	   smooth will separate the target distance into 10 steps and forward 1 step
//     unit: N/A
//	   data range: 0 ~ 100
//	(b)i4MinStep is minimum AWB gain step. If one step, which is calculated by
//	   i4Speed, is smaller than i4MinStep, then AWB gain will choose to forward
//	   1 i4MinStep
//	   unit: percentage
//	   data range: 0 ~
//	(c)i4ProbReduceStep is used for face gone. If face gone or disappear, the
//     face probability will start to reduce by i4ProbReduceStep
//		unit: percentage
//		data range: 0 ~
//variable name: rSceneChange used when face gone.
//	(a)i4LVChange_Th is used when face gone or disappear, and the LV changes
//	   more than this threshold in 1 frame, then directly make face awb
//	   probability to 0
//	   unit: 10 = 1x
//	   data range: 0 ~ 180
//	(b)i4AWBGainChange_Th is also used when face gone or disappear, and the AWB
//	   gain changes more than this threshold in 1 frame, then directly make face
//	   awb probability to 0
//	   unit: 10 = N/A
//	   data range: 0 ~
//	   AWB changes distance = (previous normal AWB R gain - current AWB R gain)^2 + 
//                              (previous normal AWB B gain - current AWB B gain)^2
/******************************************************************************/
                // rTempoSmooth
                {
                    10,    //i4Speed: Converge Speed 1~100
                     2,    //i4MinStep: Minimum converge step
                     1     //i4ProbReduceStep: If face gone, the face prob reduce step 1~100
                },
                // rSceneChange
                {
                       30,   //i4LVChange_Th
                    20000    //i4AWBGainChange_Th
                }
            },
            // Smooth Statistic
            {
                0,    //i4Enable
                {0, 23, 23, 23, 23, 23, 23, 23},  //i4StatWinShrinkOffset
                0,    //Reserved1
                0,    //Reserved2
                0,    //Reserved3
                0,    //Reserved4
                0,    //Reserved5
            },
            // Light Sensor Assist
            {
                0, // i4Enable
                // i4WeightingLUT
                {	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0},
      				  // Preference gain for each light source
      				  {
      				      {
      				          //    LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
      				          {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
      				          //    LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18      
      				          {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}
      				      }, // STROBE
      				      {
      				          //    LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
      				          {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
      				          //    LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18      
      				          {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}
      				      }, // TUNGSTEN
      				      {
      				          //    LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
      				          {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
      				          //    LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18      
      				          {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}
      				      }, // WARM F
      				      {
      				          //    LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
      				          {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
      				          //    LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18      
      				          {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}
      				      }, // F
      				      {
      				          //    LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
      				          {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
      				          //    LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18      
      				          {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}
      				      }, // CWF
      				      {
      				          //    LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
      				          {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
      				          //    LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18      
      				          {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}
      				      }, // DAYLIGHT
      				      {
      				          //    LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
      				          {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
      				          //    LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18      
      				          {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}
      				      }, // SHADE
      				      {
      				          //    LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
      				          {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
      				          //    LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18      
      				          {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}
      				      } // DAYLIGHT F
      				  },
                0, // Reserved1
                0, // Reserved2
                0, // Reserved3
                0, // Reserved4
                0, // Reserved5
            },
        },
/******************************************************************************/
//Feature name:
//AWB tuning basic information
//variable name: i4ProjCode
//default value: N/A
//representation of chip code
//variable name: mode1
//default value: N/A
//variable name: i4Data
//default value: N/A
/******************************************************************************/
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
