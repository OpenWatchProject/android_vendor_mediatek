/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#include "camera_custom_nvram.h"
#include "ov8858mipiraw_N3D_Preview.h"

#define NVRAM_AWB_TUNING_PARAM_NUM 7522001

const AWB_NVRAM_T ov8858mipiraw_AWB_0005 =
AWB_NVRAM_START
{
    // AWB calibration data
    {
        {
/******************************************************************************/
//Feature name:
//AWB unit and golden module calibration gain for high(DNP), middle(TL84) and color
//low(ALight) color temperature
//variable name: rUnitGain
//default value: {512, 512, 512}
//unit: 512 = 1x
//data range : 0 ~ 4095
//note: rGain = {0, 0 ,0} means this unit not doing calibration for DNP
/******************************************************************************/
            // rUnitGain for High color temperature (DNP)
            {
                   0, // UnitGain_R
                   0, // UnitGain_G
                   0  // UnitGain_B
            },
            // rGoldenGain for High color temperature (DNP)
            {
                   0, // GoldenGain_R
                   0, // GoldenGain_G
                   0  // GoldenGain_B
            },
            // rUnitGain for middle color temperature (TL84)
            {
                   0, // UnitGain_R
                   0, // UnitGain_G
                   0  // UnitGain_B
            },
            // rGoldenGain for middle color temperature (TL84)
            {
                   0, // GoldenGain_R
                   0, // GoldenGain_G
                   0  // GoldenGain_B
            },
            // rUnitGain for low color temperature (ALight)
            {
                   0, // UnitGain_R
                   0, // UnitGain_G
                   0  // UnitGain_B
            },
            // rGoldenGain for low color temperature (ALight)
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
                   0  // TuningUnitGain_B
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
                 842, // D65Gain_R
                 512, // D65Gain_G
                 726  // D65Gain_B
            },
/******************************************************************************/
//Feature name:
//AWB unit and golden module Bayer value for high(DNP), middle(TL84) and 
//low(ALight) color temperature
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
//variable name: rOriginalXY
//default value: N/A
//note: These settings are calibrated result, please do not need to modify them
/******************************************************************************/
        // Original XY coordinate of AWB light source
        {
            // STROBE
            {
                   0, // OriX_STROBE
                   0  // OriY_STROBE
            },
            // HORIZON
            {
                -423, // OriX_HORIZON
                -289  // OriY_HORIZON
            },
            // A
            {
                -319, // OriX_A
                -311  // OriY_A
            },
            // TL84
            {
                -166, // OriX_TL84
                -346  // OriY_TL84
            },
            // CWF
            {
                -141, // OriX_CWF
                -401  // OriY_CWF
            },
            // DNP
            {
                 -19, // OriX_DNP
                -338  // OriY_DNP
            },
            // D65
            {
                  55, // OriX_D65
                -313  // OriY_D65
            },
            // DF
            {
                  41, // OriX_DF
                -389  // OriY_DF
            },
        },
/******************************************************************************/
//Feature name:
//AWB average white point distribution in rotated X-Y domain from original X-Y domain
//in order to let the light source distribution in x-axis parallel direction
//variable name: rRotatedXY
//default value: N/A
//note: These settings are calibrated result, please do not need to modify them
/******************************************************************************/
        // Rotated XY coordinate of AWB light source
        {
            // STROBE
            {
                   0, // RotX_STROBE
                   0  // RotY_STROBE
            },
            // HORIZON
            {
                -418, // RotX_HORIZON
                -296  // RotY_HORIZON
            },
            // A
            {
                -314, // RotX_A
                -316  // RotY_A
            },
            // TL84
            {
                -161, // RotX_TL84
                -349  // RotY_TL84
            },
            // CWF
            {
                -135, // RotX_CWF
                -403  // RotY_CWF
            },
            // DNP
            {
                 -14, // RotX_DNP
                -338  // RotY_DNP
            },
            // D65
            {
                  60, // RotX_D65
                -312  // RotY_D65
            },
            // DF
            {
                  47, // RotX_DF
                -388  // RotY_DF
            },
        },
/******************************************************************************/
//Feature name:
//AWB pre-calibrated gain based on the calibrated average white point distribution
//for different light sources. It is treated as a default gain for corresponding light source
//variable name: rLightAWBGain
//default value: N/A
//note: These settings are calibrated result, please do not need to modify them
/******************************************************************************/
        // AWB gain of AWB light source
        {
            // STROBE
            {
                 512, // AWBGAIN_STROBE_R
                 512, // AWBGAIN_STROBE_G
                 512  // AWBGAIN_STROBE_B
            },
            // HORIZON
            {
                 512, // AWBGAIN_HORIZON_R
                 614, // AWBGAIN_HORIZON_G
                1609  // AWBGAIN_HORIZON_B
            },
            // A
            {
                 512, // AWBGAIN_A_R
                 517, // AWBGAIN_A_G
                1213  // AWBGAIN_A_B
            },
            // TL84
            {
                 654, // AWBGAIN_TL84_R
                 512, // AWBGAIN_TL84_G
                1024  // AWBGAIN_TL84_B
            },
            // CWF
            {
                 728, // AWBGAIN_CWF_R
                 512, // AWBGAIN_CWF_G
                1066  // AWBGAIN_CWF_B
            },
            // DNP
            {
                 788, // AWBGAIN_DNP_R
                 512, // AWBGAIN_DNP_G
                 831  // AWBGAIN_DNP_B
            },
            // D65
            {
                 842, // AWBGAIN_D65_R
                 512, // AWBGAIN_D65_G
                 726  // AWBGAIN_D65_B
            },
            // DF
            {
                 916, // AWBGAIN_DF_R
                 512, // AWBGAIN_DF_G
                 820  // AWBGAIN_DF_B
            },
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
             -1, // RotationAngle
            256, // Cos
             -4  // Sin
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
            -124, // SlopeNumerator
             128  // SlopeDenominator
        },
/******************************************************************************/
//Feature name:
//Predefined white balance gain for different LV condition.
//rSpatial_L gain is used for low LV condition.
//rSpatial_H gain is used for high LV condition.
//rTemporal gain is used for extra low LV or nonreliable condition.
//variable name: i4R, i4G, i4B
//default value: {512, 512, 512}
/******************************************************************************/
        // Predictor gain
        {
            // Spatial_L
            {
                 812, // i4R
                 530, // i4G
                 746  // i4B
            },
            // Spatial_H
            {
                 750, // i4R
                 512, // i4G
                 812  // i4B
            },
            // Temporal
            {
                 842, // i4R
                 512, // i4G
                 726  // i4B
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
            // STROBE
            {
                   0, // STROBE RightBound
                   0, // STROBE LeftBound
                   0, // STROBE UpperBound
                   0  // STROBE LowerBound
            },
            // TUNGSTEN
            {
                -222, // TUNGSTEN RightBound
                -818, // TUNGSTEN LeftBound
                -241, // TUNGSTEN UpperBound
                -333  // TUNGSTEN LowerBound
            },
            // WARM FLUORESCENT
            {
                -222, // WARM FLUORESCENT RightBound
                -818, // WARM FLUORESCENT LeftBound
                -333, // WARM FLUORESCENT UpperBound
                -493  // WARM FLUORESCENT LowerBound
            },
            // FLUORESCENT
            {
                 -62, // FLUORESCENT RightBound
                -222, // FLUORESCENT LeftBound
                -252, // FLUORESCENT UpperBound
                -376  // FLUORESCENT LowerBound
            },
            // CWF
            {
                 -55, // CWF RightBound
                -222, // CWF LeftBound
                -376, // CWF UpperBound
                -458  // CWF LowerBound
            },
            // DAYLIGHT
            {
                  90, // DAYLIGHT RightBound
                 -62, // DAYLIGHT LeftBound
                -252, // DAYLIGHT UpperBound
                -376  // DAYLIGHT LowerBound
            },
            // SHADE
            {
                 420, // SHADE RightBound
                  90, // SHADE LeftBound
                -252, // SHADE UpperBound
                -348  // SHADE LowerBound
            },
            // DF
            {
                  90, // DF RightBound
                 -55, // DF LeftBound
                -376, // DF UpperBound
                -458  // DF LowerBound
            },
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
        // PWB Reference area
        {
                 420, // PRefRightBound
                -818, // PRefLeftBound
                -216, // PRefUpperBound
                -493  // PRefLowerBound
        },
        // PWB light area
        {
            // DAYLIGHT
            {
                 115, // DAYLIGHT RightBound
                 -62, // DAYLIGHT LeftBound
                -252, // DAYLIGHT UpperBound
                -376  // DAYLIGHT LowerBound
            },
            // CLOUDY_D
            {
                 215, // CLOUDY_D RightBound
                  40, // CLOUDY_D LeftBound
                -252, // CLOUDY_D UpperBound
                -376  // CLOUDY_D LowerBound
            },
            // SHADE
            {
                 315, // SHADE RightBound
                  40, // SHADE LeftBound
                -252, // SHADE UpperBound
                -376  // SHADE LowerBound
            },
            // TWILIGHT
            {
                 -62, // TWILIGHT RightBound
                -222, // TWILIGHT LeftBound
                -252, // TWILIGHT UpperBound
                -376  // TWILIGHT LowerBound
            },
            // FLUORESCENT
            {
                 110, // FLUORESCENT RightBound
                -261, // FLUORESCENT LeftBound
                -262, // FLUORESCENT UpperBound
                -453  // FLUORESCENT LowerBound
            },
            // WARM FLUORESCENT
            {
                -214, // WARM FLUORESCENT RightBound
                -414, // WARM FLUORESCENT LeftBound
                -262, // WARM FLUORESCENT UpperBound
                -453  // WARM FLUORESCENT LowerBound
            },
            // INCANDESCENT
            {
                -214, // INCANDESCENT RightBound
                -414, // INCANDESCENT LeftBound
                -252, // INCANDESCENT UpperBound
                -376  // INCANDESCENT LowerBound
            },
            // GRAY_WORLD
            {
                5000, // GRAY_WORLD RightBound
                -5000, // GRAY_WORLD LeftBound
                5000, // GRAY_WORLD UpperBound
                -5000  // GRAY_WORLD LowerBound
            },
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
            // DAYLIGHT
            {
                 807, // PWB_DAYLIGHT_R
                 512, // PWB_DAYLIGHT_G
                 761  // PWB_DAYLIGHT_B
            },
            // CLOUDY_D
            {
                 927, // PWB_CLOUDY_D_R
                 512, // PWB_CLOUDY_D_G
                 665  // PWB_CLOUDY_D_B
            },
            // SHADE
            {
                 993, // PWB_SHADE_R
                 512, // PWB_SHADE_G
                 622  // PWB_SHADE_B
            },
            // TWILIGHT
            {
                 640, // PWB_TWILIGHT_R
                 512, // PWB_TWILIGHT_G
                 953  // PWB_TWILIGHT_B
            },
            // FLUORESCENT
            {
                 743, // PWB_FLUORESCENT_R
                 512, // PWB_FLUORESCENT_G
                 926  // PWB_FLUORESCENT_B
            },
            // WARM FLUORESCENT
            {
                 535, // PWB_WARM FLUORESCENT_R
                 512, // PWB_WARM FLUORESCENT_G
                1272  // PWB_WARM FLUORESCENT_B
            },
            // INCANDESCENT
            {
                 505, // PWB_INCANDESCENT_R
                 512, // PWB_INCANDESCENT_G
                1198  // PWB_INCANDESCENT_B
            },
            // GRAY_WORLD
            {
                 512, // PWB_GRAY_WORLD_R
                 512, // PWB_GRAY_WORLD_G
                 512  // PWB_GRAY_WORLD_B
            },
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
                -478,  // RotatedXCoordinate0
                -374,  // RotatedXCoordinate1
                -221,  // RotatedXCoordinate2
                 -74,  // RotatedXCoordinate3
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
/******************************************************************************/
//Feature name:
//AWB Linear AAO statistics to replace block with error count
//variable name: i4Enable
//default value: 1
//variable name: i4ErrCntThr
//default value: 0
/******************************************************************************/
        // Linear AAO parameter
        {
            0, //i4Enable
            0  //i4ErrCntThr
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
            { // WARM FLUORESCENT
                //    LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
                {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                //    LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18      
                {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
            },
            { // FLUORESCENT
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
        // AWB LV threshold for spatial predictor
        {
            { 115, 155}, // NonReliable
            { // Reliable
                { 105, 135}, // STROBE
                {  20, 120}, // TUNGSTEN
                {  20, 120}, // WARM FLUORESCENT
                {  20,  75}, // FLUORESCENT
                {  20,  75}, // CWF
                {  20, 150}, // DAYLIGHT
                {  20, 150}, // SHADE
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
//Parent block threshold for reliable mode and noise reduction
//variable name: i4NonNeutral and  i4NRThr[AWB_LIGHT_NUM]
//default value: N/A
//data range: 0 ~ 100 for each element in the table
//These look-up table defines threahold ratio based on LV for no neutral light and all
//light source. When the neutral block of these light source is below threshold,
//it is not accumulated for statistics
//Ex. Neutral block number[CWF] >= Parent block number * i4NRThr[CWF][LV]
/******************************************************************************/
        // Reliable parent block threshold 
        {
            //LV 0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18
            {    3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   5,  10,  10,  10,  10,  10,  10,  10,  10},  
            // Noise Reduction
            {
                //LV 0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18
                {    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},  // STROBE 
                {    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},  // TUNGSTEN 
                {    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},  // WARM FLUORESCENT 
                {    0,   0,   0,   0,   0,   3,   5,   5,   5,   5,   5,  10,  10,  10,  10,  10,  10,  10,  10},  // FLUORESCENT 
                {    0,   0,   0,   0,   0,   3,   5,   5,   5,   5,   5,  10,  10,  10,  10,  10,  10,  10,  10},  // CWF 
                {    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   2,   2,   2,   2,   2,   2,   2,   2},  // DAYLIGHT 
                {    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},  // SHADE 
                {    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   5,  10,  10,  10,  10,  10,  10,  10,  10},  // DF 
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
                       23, // i4Sunset_BoundXr_Thr
                     -338, // i4Sunset_BoundYr_Thr
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
//variable name: i4Method
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
                   1, // i4Method
                  50, // i4LVThr_L
                  70, // i4LVThr_H
                 128, // i4DaylightProb
                 {
                     -222, // Area X
                     -353, // Area Y
                 },
                 {
                     -142, // Vertex X
                     -365, // Vertex Y
                 },
             },
             // SubWindow CWF Detection
             {
                   1, // i4Enable
                   1, // i4Method
                  50, // i4LVThr_L
                  70, // i4LVThr_H
                 192, // i4DaylightProb
                 {
                     -222, // Area X
                     -407, // Area Y
                 },
                 {
                     -139, // Vertex X
                     -433, // Vertex Y
                 },
             },
/******************************************************************************/
//Feature name:
//Property description for extra color detection.
//variable name: i4Enable
//default value: 1
//data range: 0 and 1
//0 means disable extra color feature detection
//1 means enable extra color feature detection
//variable name: i4ModeWeight
//Weighting for [0] blending specified gain, [1] P2 reduction and
//[2] daylight locus probability reduction.
//data range: 0 ~ 256, unit: 256 = 1x
//variable name: i4ConfThr
//The extra color confidence level frame number threshold
//default value: 16
//variable name: i4SelLightSrc
//The extra color compensated light source
//default value: 0
//variable name: i4LvRange
//The LV range for blending weight for difference LV to keep in weighting high and reverse
//If the LV range is 0, means weighting keeps high and not reverse
//default value: 0
//data range: 0 ~ 255
//variable name: rGain
//default value: N/A , unit: 512 = 1x
//The blending gain for extra color detected
//variable name: i4RightBound, i4LeftBound, i4UpperBound, i4LowerBound
//The XrYr window for extra color detection
//default value: N/A
//variable name: rGlevel
//The average G level for extra color detection
//default value: N/A
//variable name: rLv
//default value: N/A
//data range: 0 ~ 180
//Blending weight for different LV
//variable name: rCount
//default value: N/A
//data range: 0 ~ 432
//Start to compensate when detected count > rCount.i4ThrL
//Fully compensated when detected count > rCount.i4ThrH
//variable name: rWeighting 
//default value: N/A
//data range: 0 ~ 100
//Blending weight for different LV
/******************************************************************************/
             // Extra color Compensation
             {
                 // Extra Color 0
                 {
                     0, // i4Enable
                     { 256, 0, 0}, // i4ModeWeight 
                     16, // i4ConfThr
                     {0, 0, 0, 0, 0, 0, 0, 0}, // i4SelLightSrc : Strobe T WF F CWF D Shade DF
                     20, // i4LvRange
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
                     {   0, 255}, // rGlevel
                     {   0,   0}, // rLv
                     {   0,   0}, // rCount
                     {   0,   0}, // rWeighting
                 },
                 // Extra Color 1
                 {
                     0, // i4Enable
                     { 256, 0, 0}, // i4ModeWeight 
                     16, // i4ConfThr
                     {0, 0, 0, 0, 0, 0, 0, 0}, // i4SelLightSrc : Strobe T WF F CWF D Shade DF
                     20, // i4LvRange
                     // Extra Color AWB gain
                     {
                          842, // GainR
                          512, // GainG
                          726, // GainB
                     },
                     // Extra Color area
                     {
                            0, // i4RightBound
                            0, // i4LeftBound
                            0, // i4UpperBound
                            0, // i4LowerBound
                     },
                     {   0, 255}, // rGlevel
                     {  60, 100}, // rLv
                     {  30, 200}, // rCount
                     {   0,  20}, // rWeighting
                 },
                 // Extra Color 2
                 {
                     0, // i4Enable
                     { 256, 0, 0}, // i4ModeWeight 
                     16, // i4ConfThr
                     {0, 0, 0, 0, 0, 0, 0, 0}, // i4SelLightSrc : Strobe T WF F CWF D Shade DF
                     20, // i4LvRange
                     // Extra Color AWB gain
                     {
                          842, // GainR
                          512, // GainG
                          726, // GainB
                     },
                     // Extra Color area
                     {
                            0, // i4RightBound
                            0, // i4LeftBound
                            0, // i4UpperBound
                            0, // i4LowerBound
                     },
                     {   0, 255}, // rGlevel
                     {  60, 100}, // rLv
                     {  30, 200}, // rCount
                     {   0,  20}, // rWeighting
                 },
                 // Extra Color 3
                 {
                     0, // i4Enable
                     { 256, 0, 0}, // i4ModeWeight 
                     16, // i4ConfThr
                     {0, 0, 0, 0, 0, 0, 0, 0}, // i4SelLightSrc : Strobe T WF F CWF D Shade DF
                     20, // i4LvRange
                     // Extra Color AWB gain
                     {
                          842, // GainR
                          512, // GainG
                          726, // GainB
                     },
                     // Extra Color area
                     {
                            0, // i4RightBound
                            0, // i4LeftBound
                            0, // i4UpperBound
                            0, // i4LowerBound
                     },
                     {   0, 255}, // rGlevel
                     {  60, 100}, // rLv
                     {  30, 200}, // rCount
                     {   0,  20}, // rWeighting
                 },
                 // Extra Color 4
                 {
                     0, // i4Enable
                     { 256, 0, 0}, // i4ModeWeight 
                     16, // i4ConfThr
                     {0, 0, 0, 0, 0, 0, 0, 0}, // i4SelLightSrc : Strobe T WF F CWF D Shade DF
                     20, // i4LvRange
                     // Extra Color AWB gain
                     {
                          842, // GainR
                          512, // GainG
                          726, // GainB
                     },
                     // Extra Color area
                     {
                            0, // i4RightBound
                            0, // i4LeftBound
                            0, // i4UpperBound
                            0, // i4LowerBound
                     },
                     {   0, 255}, // rGlevel
                     {  60, 100}, // rLv
                     {  30, 200}, // rCount
                     {   0,  20}, // rWeighting
                 },
                 // Extra Color 5
                 {
                     0, // i4Enable
                     { 256, 0, 0}, // i4ModeWeight 
                     16, // i4ConfThr
                     {0, 0, 0, 0, 0, 0, 0, 0}, // i4SelLightSrc : Strobe T WF F CWF D Shade DF
                     20, // i4LvRange
                     // Extra Color AWB gain
                     {
                          842, // GainR
                          512, // GainG
                          726, // GainB
                     },
                     // Extra Color area
                     {
                            0, // i4RightBound
                            0, // i4LeftBound
                            0, // i4UpperBound
                            0, // i4LowerBound
                     },
                     {   0, 255}, // rGlevel
                     {  60, 100}, // rLv
                     {  30, 200}, // rCount
                     {   0,  20}, // rWeighting
                 },
                 // Extra Color 6
                 {
                     0, // i4Enable
                     { 256, 0, 0}, // i4ModeWeight 
                     16, // i4ConfThr
                     {0, 0, 0, 0, 0, 0, 0, 0}, // i4SelLightSrc : Strobe T WF F CWF D Shade DF
                     20, // i4LvRange
                     // Extra Color AWB gain
                     {
                          842, // GainR
                          512, // GainG
                          726, // GainB
                     },
                     // Extra Color area
                     {
                            0, // i4RightBound
                            0, // i4LeftBound
                            0, // i4UpperBound
                            0, // i4LowerBound
                     },
                     {   0, 255}, // rGlevel
                     {  60, 100}, // rLv
                     {  30, 200}, // rCount
                     {   0,  20}, // rWeighting
                 },
                 // Extra Color 7
                 {
                     0, // i4Enable
                     { 256, 0, 0}, // i4ModeWeight 
                     16, // i4ConfThr
                     {0, 0, 0, 0, 0, 0, 0, 0}, // i4SelLightSrc : Strobe T WF F CWF D Shade DF
                     20, // i4LvRange
                     // Extra Color AWB gain
                     {
                          842, // GainR
                          512, // GainG
                          726, // GainB
                     },
                     // Extra Color area
                     {
                            0, // i4RightBound
                            0, // i4LeftBound
                            0, // i4UpperBound
                            0, // i4LowerBound
                     },
                     {   0, 255}, // rGlevel
                     {  60, 100}, // rLv
                     {  30, 200}, // rCount
                     {   0,  20}, // rWeighting
                 },
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
        // Non-neutral probability for spatial and temporal weighting look-up table
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
            {{ 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  50,  25,   0,   0,   0,   0}}, // STROBE
            {{ 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  75,  50,  25,   0,   0,   0}}, // TUNGSTEN
            {{ 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  75,  50,  25,  25,  25,   0,   0,   0}}, // WARM FLUORESCENT
            {{ 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  95,  75,  50,  25,  25,  25,   0,   0,   0}}, // FLUORESCENT
            {{  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  80,  55,  30,  30,  30,  30,   0,   0,   0}}, // CWF
            {{ 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  75,  50,  40,  30,  20}}, // DAYLIGHT
            {{ 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  75,  50,  25,   0,   0,   0,   0}}, // SHADE
            {{  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  80,  55,  30,  30,  30,  30,   0,   0,   0}}, // DF
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
            {{ 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100}}, // STROBE
            {{ 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  66,  33,   1,   1,   1,   1,   1}}, // TUNGSTEN
            {{ 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  66,  33,   1,   1,   1,   1,   1}}, // WARM FLUORESCENT
            {{ 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  66,  66,  66,  66,  66,  66,  66}}, // FLUORESCENT
            {{ 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  66,  33,   1,   1,   1,   1,   1}}, // CWF
            {{ 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100}}, // DAYLIGHT
            {{ 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  66,  33,   1,   1,   1}}, // SHADE
            {{ 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  66,  33,   1,   1,   1,   1,   1}}, // DF
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
                5120, // T
                5120, // WF
                 909  // Shade
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
                {  50, 100}, // TUNGSTEN
                {  50, 100}, // WARM FLUORESCENT
                {  50, 100}, // SHADE
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
		         { // TUNGSTEN
		             { //  0  500 1000 1500 2000 2500 3000 3500 4000 4500 5000 5500 6000 6500 7000 7500 8000 8500 9000 9500 10000
		                  50,  50,  50,  50,  50,  50,  50,  50,  50,  50,  50,  55,  60,  65,  70,  75,  80,  85,  90,  95, 100
		             },
		         },
		         { // WARM FLUORESCENT
		             { //  0  500 1000 1500 2000 2500 3000 3500 4000 4500 5000 5500 6000 6500 7000 7500 8000 8500 9000 9500 10000
		                  50,  50,  50,  50,  50,  50,  50,  50,  50,  50,  50,  55,  60,  65,  70,  75,  80,  85,  90,  95, 100
		             },
		         },
		         { // SHADE
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
		         { // TUNGSTEN
		             {   0,  500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000},
		             {   0,  350,  800, 1222, 1444, 1667, 1889, 2111, 2333, 2556, 2778, 3000, 3222, 3444, 3667, 3889, 4111, 4333, 4556, 4778, 5000}
		         },
		         { // WARM FLUORESCENT
		             {   0,  500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000},
		             {   0,  350,  700, 1000, 1444, 1667, 1889, 2111, 2333, 2556, 2778, 3000, 3222, 3444, 3667, 3889, 4111, 4333, 4556, 4778, 5000}
		         },
		         { // SHADE
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
		         { // TUNGSTEN
		             0, //i4OffsetMode : 0 disabe 1 enable. Increase offset when GM offset is large than threshold
		             { // LUT: use daylight locus offset (0~10000) as index to get magenta offset threshold
		               //  0   500  1000  1500  2000  2500  3000  3500  4000  4500  5000  5500  6000  6500  7000  7500  8000  8500  9000  9500  10000
		                   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0
		             },
		             { // LUT: use daylight locus offset (0~10000) as index to get green offset threshold
		               //  0   500  1000  1500  2000  2500  3000  3500  4000  4500  5000  5500  6000  6500  7000  7500  8000  8500  9000  9500  10000
		                   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0
		             },
		         },
		         { // WARM FLUORESCENT
		             0, //i4OffsetMode : 0 disabe 1 enable. Increase offset when GM offset is large than threshold
		             { // LUT: use daylight locus offset (0~10000) as index to get magenta offset threshold
		               //  0   500  1000  1500  2000  2500  3000  3500  4000  4500  5000  5500  6000  6500  7000  7500  8000  8500  9000  9500  10000
		                1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500
		             },
		             { // LUT: use daylight locus offset (0~10000) as index to get green offset threshold
		               //  0   500  1000  1500  2000  2500  3000  3500  4000  4500  5000  5500  6000  6500  7000  7500  8000  8500  9000  9500  10000
		                1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500
		             },
		         },
		         { // SHADE
		             0, //i4OffsetMode : 0 disabe 1 enable. Increase offset when GM offset is large than threshold
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
		         { // Tungsten
		             { // LUT: use magenta offset (0~1000) as index to get tungsten weight (x/256)
		               // 0   100  200  300  400  500  600  700  800  900 1000 
		                 256, 256, 256, 256, 256, 256, 256, 128,  64,  32,  16
		             },
		             { // LUT: use green offset (0~1000) as index to get tungsten weight (x/256)
		               //  0  100  200  300  400  500  600  700  800  900 1000 
		                 256, 256, 256, 256, 256, 256, 256, 128,  64,  32,  16
		             },
		         },
		         { // Warm Fluorescent
		             { // LUT: use magenta offset (0~1000) as index to get tungsten weight (x/256)
		               //  0  200  400  600  800 1000 1200 1400 1600 1800 2000 
		                 256, 256, 256, 256, 256, 256, 256, 192,  96,  40,  16
		             },
		             { // LUT: use green offset (0~1000) as index to get tungsten weight (x/256)
		               //  0  200  400  600  800 1000 1200 1400 1600 1800 2000 
		                 256, 256, 256, 256, 256, 256, 256, 192,  96,  40,  16
		             },
		         },
		         { // Shade
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
/******************************************************************************/
//Feature name:
//Light source preference gain according to LUT of daylight locust offset and green or 
//magenta offset of corresponding light source white point for tungsten, warm fluorescent and shade
//variable name: rDlOffsetGain
//default value: N/A
//variable name: rMOffsetGain
//default value: N/A
//variable name: rGOffsetGain
//default value: N/A
/******************************************************************************/
		     // AWB light source preference gain LUT
		     {
		         { // TUNGSTEN
		             { // LUT: use daylight locus offset as index
		                 {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512},
		                 {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512},
                    },
		             { // LUT: use (magenta offset - thld) as index 
		                 {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}
		             },
		             { // LUT: use (green offset - thld) as index 
		                 {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}
		             },
		         },
		         { // WARM FLUORESCENT
		             { // LUT: use daylight locus offset as index
		                 {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512},
		                 {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512},
                    },
		             { // LUT: use (magenta offset - thld) as index 
		                 {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}
		             },
		             { // LUT: use (green offset - thld) as index 
		                 {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}
		             },
		         },
		         { // SHADE
		             { // LUT: use daylight locus offset as index
		                 {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512},
		                 {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512},
                    },
		             { // LUT: use (magenta offset - thld) as index 
		                 {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}
		             },
		             { // LUT: use (green offset - thld) as index 
		                 {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}, {512, 512}
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
                      0, // General Mode
                      0, // Non-Reliable Mode
                      1, // Temporal Init Mode
                    // Non-reliable LV threshold
                      0, //i4ThrTemp       
                    // Reliable LV threshold
                    {
                          0, // STROBE
                          0, // TUNGSTEN
                          0, // WARM FLUORESCENT
                          0, // FLUORESCENT
                          0, // CWF
                          0, // DAYLIGHT
                          0, // SHADE
                          0, // DF
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
//data range: 0 ~ 180 
//variable name: i4P0Thr1
//default value: N/A
//data range: 0 ~ 150 
//variable name: i4P0Thr2
//default value: N/A
//data range: 0 ~ 150 
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
                        512, //T
                        512, //WF
                        512  //Shade
                     }
                 },
            },
/******************************************************************************/
//Feature name:
//Property of statistic gain constraint for each light source
//variable name: i4Mode
//default value: N/A
//data range: 0 ~ 1 
//0 means disable statistic gain constraint
//1 means enable statistic gain constraint on avergage
//2 means enable statistic gain constraint on each parent block
//variable name: rLv
//default value: N/A
//data range: 0 ~ 180
//The statistic limit effect will be reduced from i4ThrH and disable when Lv lower than i4ThrL
//variable name: i4LimitY
//default value: N/A
//These value define the reasonable distance from daylight locus.The values of Tusteng, Fluorescent, Daylight
//and shade are the upper boundary. The values of Warm Fluorescent, CWF and DF are the lower boundary
//variable name: i4WeightReduce
//default value: N/A
//data range: 0 ~ 16
//If the statistic location is far away from daylight locus. It will reduce the statistic gain weighting
//variable name: i4ProjWeight
//default value: N/A
//data range: 0 ~ 16
//The statistic will project to the statistic limit boundary according to projection weighting
/******************************************************************************/
            // rStatLimit
            {
                0,    // i4Mode
                {   0,   0}, // rLv
                { // i4LimitY[AWB_LIGHT_NUM]
                     0,  // STROBE
                  -276,  // TUNGSTEN
                  -416,  // WARM FLUORESCENT
                  -318,  // FLUORESCENT
                  -423,  // CWF
                  -292,  // DAYLIGHT
                  -292,  // SHADE
                  -423,  // DF
                },
                { // i4WeightReduce[AWB_LIGHT_NUM]
                    0,  // STROBE
                    4,  // TUNGSTEN
                    4,  // WARM FLUORESCENT
                    4,  // FLUORESCENT
                    4,  // CWF
                    4,  // DAYLIGHT
                    4,  // SHADE
                    4,  // DF
                },
                { // i4ProjWeight[AWB_LIGHT_NUM]
                    0,  // STROBE
                   16,  // TUNGSTEN
                   16,  // WARM FLUORESCENT
                   16,  // FLUORESCENT
                   16,  // CWF
                   16,  // DAYLIGHT
                   16,  // SHADE
                   16,  // DF
                },
            },
/******************************************************************************/
//Feature name:
//Common setting for face compensation and face assisted awb algorithm
/******************************************************************************/
            // Face Common
            {
/******************************************************************************/
//Feature name:
//Face AWB basic control & basic statistic tuning
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
//variable name: rStatisticNR
//	(a)i4Enable is used for enable NR process
//    unit: N/A
//    data range: 0 and 1
//	(b)i4DiffThr is used to judge the child block belongs to face or not
//	   e.g One child block {R/G, B/G}, and the central face area is {R'/G', B'/G'}
//	   If |R/G - R'/G'| + |B/G - B'/G'| > i4DiffThr, this child block doesn't belong to face
//	   And face statistic will skip this child block
//    unit: 1000
//    data range: 0 ~
//Do NR process when statistic face area data
//variable name: rFD_RGB_Bound
//default value: {1, 254}
//unit: N/A
//data range: 0 ~ 255
//If face R or G or B is lower than Low Bound, or higher than High bound,
//this face will be ignored for using
/******************************************************************************/
                  0,    //i4StatAvoidFaceArea
                100,    //i4FaceWinRatio  50~400
                  1,    //u4FaceCentralWeight  0=>1:1, 1=>2:1, 2=>3:1 ... (center : others)
                // rStatisticNR
                {
                      0,    //i4Enable
                      0     //i4DiffThr
                },
                // rFD_RGB_Bound
                {
                      1,    //u4LowBound
                    254     //u4HiBound
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
//	(a)i4LVChangeTh is used when face gone or disappear, and the LV changes
//	   more than this threshold in 1 frame, then directly make face awb
//	   probability to 0
//	   unit: 10 = 1x
//	   data range: 0 ~ 180
//	(b)i4AWBGainChangeTh is also used when face gone or disappear, and the AWB
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
                       30,   //i4LVChangeTh
                    20000    //i4AWBGainChangeTh
                },
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
//Face AWB Compensation Enable Flag
//variable name: i4Enable
//default value: 0
//unit: N/A
//data range: 0 and 1
//0 means disable face AWB
//1 means enable face AWB
//Feature name:
//Face AWB preference skin tone target
//variable name: i4SatTargetEn
//unit: N/A
//data range: 0 and 1
//Enable Saturation Target for face AWB saturation control
//variable name: rRefTarget
//default value: calibrated from calibration library
//unit: 1000 = 1x
//data range: 0 ~ 60000
//These parameter is the preference skin tone of face hue and saturation, which is after
//applying AWB gain
//There are 3 sets of reference target for hi, mid, low color temperature
//i4TempoWeight is used for temporal smooth switch ref target, the formula is
//  (Current Ref target + Temporal Ref target * Tempoweight) / (Tempoweight + 1)
//  unit: N/A
//  data range: 1 ~
//variable name: rSceneJudge
//	(a)i4LVLow & i4LVHi are LV threshold used for mid & Hi CT ref target
//		 interpolation
//      (when LV is higher, the hi CT probabilirt is higher)
//      unit: 10 = 1x
//      data range: 0 ~ 180
//	(b)i4FaceRB_Low & i4FaceRB_Hi are face processed raw R/B threshold,
//     which are used for mid & low CT ref target interpolation
//	    (when face R/B is higher, the low CT probability is higher)
//      unit: 1000 = 1x
//      data range: 0 ~
/******************************************************************************/
                0,    //i4Enable
                0,    //i4SatTargetEn
                // rRefTarget
                {
                    // rHue
                    {
                        19156,    //i4HiCT
                        16684,    //i4MidCT
                        23143     //i4LowCT
                    },
                	 // rSat
                    {
                        34909,    //i4HiCT
                        23358,    //i4MidCT
                        30738     //i4LowCT
                    },
                    7    //i4TempoWeight
                },
                // rSceneJudge
                {
                      90,    //i4LVLow for Indoor & Outdoor dynamic
                     130,    //i4LVHi for Indoor & Outdoor dynamic
                    2300,    //i4FaceRB_Low for Indoor Mid CT & Low CT dynamic, Unit = 1000
                    3000     //i4FaceRB_Hi for Indoor Mid CT & Low CT dynamic, Unit = 1000
                },
/******************************************************************************/
//Feature name:
//When face hue & sat is close to target, face AWB strength will be reduced
//variable name: rConvergeCtrl
//	(a)i4Enable use to enable target converge control mechanism
//    default value: 1
//unit: N/A
//data range: 0 and 1
//    0 means although current face hue or sat is same with ref target, algo will still
//    force generating new awb gain
//    1 means if current face hue is close to ref target hue, the ratio will be reduced
//	(b)i4HueTOL target hue converge tolerance
//    default value: 1000
//	   unit: 1000
//	   data range: 0 ~ 60000
//	(c)i4SatTOL target sat converge tolerance
//    default value: 5000
//	   unit: 1000
//	   data range: 0 ~ 100000
//	(d)i4RestrictRatioTOL restrcit ratio converge tolerance
//    default value: 500
//	   unit: 1000
//	   data range: 0 ~ 60000
//variable name: rOversat
//	(a)rLV use LV to interpolate from i4LVLow to i4LVHi to get Probability
//	   from i4OverSatProb_Low to i4OverSatProb_Hi
//	   unit: LV 10 = 1x, Prob = percentage
//	   data range: LV = 0 ~ 180, Prob = 0 ~ 100
//	(b)rHiCT use Hi CT Prob to interpolate from i4CT_P_Low to i4CT_P_Hi to
//	   get Probability from i4OverSatProb_Low to i4OverSatProb_Hi
//	   unit:  percentage
//	   data range: CT_P = 0 ~ 100, Prob = 0 ~ 100
//	(c)rMidCT use Mid CT Prob to interpolate from i4CT_P_Low to i4CT_P_Hi to
//	   get Probability from i4OverSatProb_Low to i4OverSatProb_Hi
//	   unit:  percentage
//	   data range: CT_P = 0 ~ 100, Prob = 0 ~ 100
//	(d)rLowCT use Hi CT Prob to interpolate from i4CT_P_Low to i4CT_P_Hi to
//	   get Probability from i4OverSatProb_Low to i4OverSatProb_Hi
//	   unit:  percentage
//	   data range: CT_P = 0 ~ 100, Prob = 0 ~ 100
//Above parameters will choose the maximum for oversat probability. When
//oversat probability is higher, the face AWB gain will be more bluish,
//otherwise it will be more yellowish.
//variable name: rUndersat
//	(a)i4SatHi when face sat is lower than this threshold, undersat mechanism will start work
//	   and reduce oversat prob
//	   unit: N/A
//	   data range: 0 ~ 100000
//	(b)i4SatLow when face sat is lower than this threshold, undersat mechanism will be strongest
//	   unit: N/A
//	   data range: 0 ~ 100000
//	(c)i4NewRestrict when face enter undersat condition, it also force increasing restrict ratio
//	   to this NewRestrict
//	   unit:  0.1%
//	   data range: 0 ~ 1000
//	(d)i4LVLow only when LV > i4LVLow - i4LVTOL and LV < i4LVHi + i4LVTOL, the undersat mechanism will work
//	   unit:  LV 10 = 1x,
//	   data range: 0 ~ 180
//	(e)i4LVHi as descibed in (d)
//	   unit:  LV 10 = 1x,
//	   data range: 0 ~ 180
//	(f)i4LVTOL oas descibed in (d)
//	   unit:  LV 10 = 1x,
//	   data range: 0 ~ 180
//variable name: i4GainRatioRestrictLUT
//Unit: 0.1%
//data range: 0 ~ 1000
//This LV LUT will restrict face awb gain can only change the normal awb gain
//in specific percentage range. If this restrict percentage is higher, the
//face awb effect will be more aggressive, but cause normal awb seriously
//color bias and more non-smooth. If this restrict percentage is lower,
//the face awb effect will be reduced, but more smoothly.
//Face AWB gain = Original AWB gain * Restrict Gain Ratio
/******************************************************************************/
                // rConvergeCtrl
                {
                       0,    //i4Enable
                    1000,    //i4HueTOL e.g If target hue = 19, so the range will be 18~20 (19-1 ~ 19+1)
                    5000,    //i4SatTOL e.g If target sat = 40, so the range will be 35~45 (40-5 ~ 40+5)
                     500     //i4RestrictRatioTOL e.g If restric ratio = 50 means 5%, so the restrict ratio tolerance will be 5*500 = 2500
                },
                // rOversat
                {
                    // rLV
                    {
                         90,    //i4LVLow
                        120,    //i4LVHi
                          0,    //i4OverSatProb_Low
                        100     //i4OverSatProb_Hi
                    },
                    // rHiCT
                    {
                         40,    //i4CT_P_Low
                         90,    //i4CT_P_Hi
                          0,    //i4OverSatProb_Low
                         50     //i4OverSatProb_Hi
                    },
                    // rMidCT
                    {
                          0,    //i4CT_P_Low
                          0,    //i4CT_P_Hi
                          0,    //i4OverSatProb_Low
                          0,    //i4OverSatProb_Hi
                    },
                    // rLowCT
                    {
                          0,    //i4CT_P_Low
                          0,    //i4CT_P_Hi
                          0,    //i4OverSatProb_Low
                          0     //i4OverSatProb_Hi
                    }
                },
                // rUndersat
                {
                        0,    //i4SatHi
                        0,    //i4SatLow
                       75,    //i4NewRestrict
                       25,    //i4LVLow
                       95,    //i4LVHi
                       10     //i4LVTOL
                },
                // i4GainRatioRestrictLUT
                //LV0  1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18
                { 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 40, 30, 20, 20},
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
//	(a)rProb0 is refering face size compare to 120x90. If face size ratio
//	   is higher, the face prob0 must be higher. It uses face size ratio to
//	   interpolate from i4FaceSizeRatioLow to i4FaceSizeRatioHi to get
//	   Probability0 from i4Prob0_Min to i4Prob0_Max.
//	   unit: size ratio = percentage*1000, Prob0 = percentage
//	   data range: size ratio 0 ~ 100000, Prob0 0 ~ 100
//	(b)rProb1 is a LV LUT in 3 kinds of CT for face probability1.
//	   unit: percentage
//	   data range: 0 ~ 100
//	(c)rProb2 is refering face processed raw R/G information. If the R/G
//    is out of range, the face prob2 must be reduced. It uses face processed 
//	   raw to interpolate from i4FaceRG_Low to (i4FaceRG_Low - i4FaceRG_TOL)
//	   or interpolate from i4FaceRG_Hi to (i4FaceRG_Hi + i4FaceRG_TOL)
//	   to get probability2 from i4Prob2_Max to i4Prob2_Min.
//	   unit: R/G 1000 = 1x, Prob = percentage
//	   data range: R/G N/A, Prob 0 ~ 100
//	(d)rProb3 is totally same with rProb2, just change to refer
//	   face processed raw B/G information.
//	(e)rProb4 is totally same with rProb2, just change to refer
//	   face processed raw G information.
//Face probability = face Prob0 * face Prob1 * face Prob2 * face Prob3 * face Prob4
//Face AWB target gain = ((original AWB gain * (1 - face Probability)) +
//                       (face AWB gain * face Probability)) / 100
/******************************************************************************/
                // rProb0
                {
                    3900,    //i4FaceSizeRatioLow
                    4900,    //i4FaceSizeRatioHi
                       0,    //i4Prob0_Min
                     100     //i4Prob0_Max
                },
                // rProb1
                {
                    //LV0   1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18
                	 {  50,  75, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  80,  60,  40},    //i4HiCT_LUT
                	 {  50,  75, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  80,  60,  40},    //i4MidCT_LUT
                	 {  50,  75, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  80,  60,  40}     //i4LowCT_LUT
                },
                // rProb2
                {
                     650,    //i4FaceRG_Low Unit=1000
                    1200,    //i4FaceRG_Hi Unit=1000
                     500,    //i4FaceRG_TOL Unit=1000
                       0,    //i4Prob2_Min
                     100     //i4Prob2_Max
                },
                // rProb3
                {
                     250,    //i4FaceBG_Low Unit=1000
                     800,    //i4FaceBG_Hi Unit=1000
                     500,    //i4FaceBG_TOL Unit=1000
                       0,    //i4Prob3_Min
                     100     //i4Prob3_Max
                },
                // rProb4
                {
                       0,    //i4FaceG_Low Unit=1000
                       0,    //i4FaceG_Hi Unit=1000
                       0,    //i4FaceG_TOL Unit=1000
                     100,    //i4Prob4_Min
                     100     //i4Prob4_Max
                },
/******************************************************************************/
//Feature name: rSpatial
//Face spatial compensation when face size is small and neutral parent block
//number is very few.
//	(a)rP0 is refering face size compare to 120x90. If face size ratio
//	   is higher, the spatial prob0 will be lower. It uses face size ratio to
//	   interpolate from i4FaceSizeRatioHi to i4FaceSizeRatioLow to get
//	   Probability0 from i4Prob0_Min to i4Prob0_Max.
//	   unit: size ratio = percentage*1000, Prob0 = percentage
//	   data range: size ratio 0 ~ 100000, Prob0 0 ~ 100
//	(b)rP1 is refering whole image neutral parent block number, except
//		 face area. If neutral parent block num is higher, the spatial prob1 will
//     be lower. It uses neutral parent block number to interpolate from
//     i4NeutralParentBlkNum_Hi to i4NeutralParentBlkNum_Low to get Probability1
//     from i4Prob1_Min to i4Prob1_Max.
//	   unit: neutral parent block num = N/A, Prob1 = percentage
//	   data range: neutral parent block num 0 ~ 120*90, Prob1 0 ~ 100
//	(c)i4LVLow & i4LVHi are used for awb spatial gain caculation. It is
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
                	 // rP0
                	 {
                    	 3900,    //i4FaceSizeRatioHi  Unit=1000
                    	  500,    //i4FaceSizeRatioLow Unit=1000
                    	    0,    //i4Prob0_Min
                    	   50     //i4Prob0_Max
                	 },
                	 // rP1
                	 {
                    	   75,    //i4NeutralParentBlkNum_Hi
                    	   15,    //i4NeutralParentBlkNum_Low
                    	    0,    //i4Prob1_Min
                    	   50     //i4Prob1_Max
                	 },
                     70,    //i4LVLow
                    110     //i4LVHi
              	  },
/******************************************************************************/
//Feature name:
//variable name: rPortrait used when face gone.
//	(a)i4Enable is used to enable portrait mechanism if the DL is also enalbe
//	   unit: N/A
//	   data range: 0 and 1
//	(b)i4NoPortraitTh is used for judge portrait is exist or not
//	   If portrait already exists, the portrait value only need to bigger than this thr,
//	   then portrait will be considerd still exist
//	   unit: N/A
//	   data range: N/A
//	(c)i4IsPortraitTh is used for judge portrait is exist or not
//	   If portrait appear in this frame, the portrait value need to bigger than this thr,
//	   then portrait will be considerd exist
//	   unit: N/A
//	   data range: N/A
//	(d)i4FrameDelay is used for delay reduceing face prob
//	   If FrameDelay is 3, which means that every 3 frame will reduce face prob one time
//	   unit: N/A
//	   data range: 0 ~
//variable name: rGender used to change ref target when there is female in image.
//	(a)i4Enable is used to enable gender mechanism if the DL is also enalbe
//	   unit: N/A
//	   data range: 0 and 1
//	(b)i4TargetHueOFS is the target hue offset for female exist in image
//	   unit: N/A
//	   data range: -60000 ~ 60000
//	(c)i4TargetSatOFS is the target sat offset for female exist in image
//	   unit: N/A
//	   data range: -100000 ~ 100000
//	(d)i4OversatProbRatio forcing change oversat prob when female exist in image
//	   unit: N/A
//	   data range: 0 ~ 100000
/******************************************************************************/
                // rPortrait
                {
                        0,   //i4Enable
                    -3000,   //i4NoPortraitTh
                        0,   //i4IsPortraitTh
                        3    //i4FrameDelay
                },
                // rGender
                {
                        0,   //i4Enable
                    -2000,   //i4TargetHueOFS
                    -2000,   //i4TargetSatOFS
                     1200    //i4OversatProbRatio
                }
            },
/******************************************************************************/
//Feature name:
//Face Assisted AWB refer face information to assist normal AWB prediction.
//Especially for confusing scene with face
/******************************************************************************/
            // Face Assisted AWB
            {
/******************************************************************************/
//Feature name:
//Face Assisted AWB Algorithm Basic Control
//variable name: i4Enable
//  default value: 0
//  unit: N/A
//  data range: 0 and 1
//  0 means disable face assisted AWB
//  1 means enable face assisted AWB
//variable name: i4TOL
//  description: tolerance range for every light source
//  default value: 0
//  unit: XrYr domain unit
//  data range: 0 ~
//variable name: rRefTarget
//  description: user predefined setting of reference target
//               support three difference value of ref. target position for more flexible use
//  default value: default setting could calibration by color patch of color chart
//  unit: XrYr domain unit
//  data range: 0 ~
//variable name: rPrefGain
//  description: preference gain(Rgain, Bgain) for face predictor gain by light source
//  default value: {512, 512}
//  unit: 1X = 512
//variable name: rProb0
//  description: face assisted probability by face-size ratio
//	 unit: size ratio = percentage*1000, Prob0 = percentage
//	 data range: size ratio 0 ~ 100000, Prob0 0 ~ 100
//variable name: rProb1 
//  description: face assisted probability by environment brightness LV for each light source
//	 unit: percentage
//	 data range: 0 ~ 100
//variable name: rProb2
//  description: face assisted probability by Y-bias offset from face reference target
//	 unit: reduce retio
//	 data range: 1X = 256
//variable name: rProb3
//  description: face assisted probability by temporal information
//               if face is detected and stable, probability will increase with time
//	   i4StableConfLow/Hi
//	     unit: frame number
//	     data range: 0 ~ 
//	   i4ProbMin/i4ProbMax
//	     unit: percentage
//	     data range: 0 ~ 100
//variable name: rProb4 (TODO)
//  description: face assisted probability by valid face-color ratio if statistic-NR is enable
//	   i4FaceColorRationLow/Hi
//	     unit: ratio by 1X = 1000
//	     data range: 0 ~ 1000
//	   i4ProbMin/i4ProbMax
//	     unit: percentage
//	     data range: 0 ~ 100
//variable name: rCompSetting
//  description: compensation ratio by face assisted algorithm
//variable name: rStableSetting
//  description: Stable setting for temporal smooth of face-assisted algo.
/******************************************************************************/
                0,    //i4Enable
                { //i4TOL
                     32,    //STB
                     32,    //T
                     32,    //WF
                     32,    //F
                     32,    //CWF
                     32,    //D
                     32,    //SHADE
                     32,    //DF
                },
                //rRefTarget - Reference Target Setting
                {
                      1,    //i4Mode
                    100,    //i4WeightCoef_a
                    100,    //i4WeightCoef_b
                    // i4RefTargetThr
                    {
                         32,    //ThrLow
                         64,    //ThrMid
                         96,    //ThrHigh
                    },
                    // i4RefTargetThr
                    {
                        // Ref_Low       Ref_Mid       Ref_High
                        {{4095, 4095}, {4095, 4095}, {4095, 4095}},  // STROBE
                        {{-610, -224}, {-610, -224}, {-610, -224}},  // TUNGSTEN
                        {{-506, -244}, {-506, -244}, {-506, -244}},  // WARM FLUORESCENT
                        {{-365, -281}, {-365, -281}, {-365, -281}},  // FLUORESCENT
                        {{-325, -358}, {-325, -358}, {-325, -358}},  // CWF
                        {{-205, -273}, {-205, -273}, {-205, -273}},  // DAYLIGHT
                        {{-131, -237}, {-131, -237}, {-131, -237}},  // SHADE
                        {{-144, -321}, {-144, -321}, {-144, -321}},  // DF
                    },
                },
                //rPrefGain for Predictor Gain
                {
                    { 512,  512}, // STROBE
                    { 512,  512}, // TUNGSTEN
                    { 512,  512}, // WARM FLUORESCENT
                    { 512,  512}, // FLUORESCENT
                    { 512,  512}, // CWF
                    { 512,  512}, // DAYLIGHT
                    { 512,  512}, // SHADE
                    { 512,  512}, // DF
                },
                //rProb0 - Face Size
                {
                    3900,  // i4FaceSizeRatioLow
                    4900,  // i4FaceSizeRatioHi
                       0,  // i4ProbMin
                     100,  // i4ProbMax
                },
                //rProb1 - LV
                {
                    {
                        //LV 0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18
                    {   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1}, // STROBE
                    {  50,  75, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  80,  60,  40}, // TUNGSTEN
                    {  50,  75, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  80,  60,  40}, // WARM FLUORESCENT
                    {  50,  75, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  80,  60,  40}, // FLUORESCENT
                    {  50,  75, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  80,  60,  40}, // CWF
                    {  50,  75, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  80,  60,  40}, // DAYLIGHT
                    {  50,  75, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  80,  60,  40}, // SHADE
                    {  50,  75, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  80,  60,  40}, // DF
                    }
                },
                //rProb2
                {
                    { //High-CT
                      //   0    25   50   75  100  125  150  175  200  225  250
                        { 256, 256, 256, 256, 256, 256, 256, 256, 240, 160, 128 }, // Magenta
                        { 256, 256, 256, 256, 256, 256, 240, 212, 180, 128,  80 }, // Green
                    },
                    { //Mid-CT
                      //   0    25   50   75  100  125  150  175  200  225  250
                        { 256, 256, 256, 256, 256, 256, 256, 256, 240, 160, 128 }, // Magenta
                        { 256, 256, 256, 256, 256, 256, 240, 212, 180, 128,  80 }, // Green
                    },
                    { //Low-CT
                      //   0    25   50   75  100  125  150  175  200  225  250
                        { 256, 256, 256, 256, 240, 212, 180, 128,  96,  64,  40 }, // Magenta
                        { 256, 256, 256, 256, 256, 256, 240, 212, 180, 128,  80 }, // Green
                    }
                },
                //rProb3
                {
                       5,  // i4StableConfLow
                      60,  // i4StableConfHi
                       0,  // i4ProbMin
                     100,  // i4ProbMax
                },
                //rProb4
                {
                     200,  // i4FaceColorRatioLow
                     800,  // i4FaceColorRatioHi
                       0,  // i4ProbMin
                     100,  // i4ProbMax
                },
                //rCompSetting
                {
                    //rConfThr
                    {
                          0,  // i4ThrL
                        100,  // i4ThrH
                    },
                    //rDistanceThr
                    {
                         50,  // i4ThrL
                        200,  // i4ThrH
                    },
                    //rCompRatio
                    {
                          0,  // i4ThrL
                         20,  // i4ThrH
                    },
                },
                // rStableSetting
                {
                      7,  // i4TempoWeight
                      8,  // i4DelayFrm
                },
                // rReserved
                {
                      0,  // i4Resv01
                      0,  // i4Resv02
                      0,  // i4Resv03
                      0,  // i4Resv04
                      0,  // i4Resv05
                      0,  // i4Resv06
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
            // Panel Damping Prevention
            {
                1,    //i4Enable
                // rP0
                {
                    50,	 //i4PBRatioH
                		25,  //i4PBRatioL
                		70,  //i4ErrCntRatioL
                		80,  //i4ErrCntRatioH
                		75,  //i4RefineP1L
                		90,  //i4RefineP1H
                },
                // rScene
                {
             		    70,  //i4LV
             			  // rTrigger
             			  {
             				    30,  //i4YDiffTh
             				    20,	 //i4RgainDiffRatioTh
             			  },
             			  20,  //i4TrackingYDiffTh
                },
                // rStr
                {
                    500,  //i4XYDiffLow
                		750,  //i4XYDiffHi
                		// rGain
                	 	{
                		     50,  //i4Max1
                		    100,  //i4Max2
                		},
                		// rSpeed
                	 	{
                		     50,  //i4Max1
                		    100,  //i4Max2
                		},
                },
                // rRsv
                {
                    0,  //i4Resv01
                		0,  //i4Resv02
                		0,  //i4Resv03
                		0,  //i4Resv04
                		0,  //i4Resv05
                		0,  //i4Resv06
                		0,  //i4Resv07
                },
            },
        },
/******************************************************************************/
//Feature name:
//AWB chip dependent parameter
//variable name: i4AWBGainOutputScaleUnit
//It is used for AWB output gain normailization, please do not modify it.
//default value: 512
//unit: 512 = 1x
//variable name: i4AWBGainOutputUpperLimit
//Maximum AWB gain is ~16x, it is hardware limitation. Please do not modify it.
//default value: 8191
//unit: 512 = 1x
//variable name: i4RotationMatrixUnit
//AWB rotation matrix unit. Please do not modify it.
//default value: 256
//unit: 256 = 1x
/******************************************************************************/
        // Chip dependent parameter
        {
    	      512, // i4AWBGainOutputScaleUnit: 1.0x = 512
    	      8191, // i4AWBGainOutputUpperLimit: format 4.9 (11 bit)
    	      256  // i4RotationMatrixUnit: 1.0x = 256
        },
/******************************************************************************/
//Feature name:
//Statistic data weighting function.
//
//variable name: i4Mode
//default value: 1
//data range: 0 to 3
//0 means disable weighting function
//1 means enable : Weight = 1+((SumR+SumG+SumB)/ChildBlkNum)/2^(i4ScalingFactor)
//2 means enable : Weighting LUT. Weight = i4WeightLut(i4GammaLut((SumR+SumG+SumB)/ChildBlkNum))
//                 For different color temperature : Low:T,WF; Middle:F,CWF; High:D65,DF,Shade
//3 means enable : Weighting LUT. Weight = i4WeightLut(i4GammaLut((SumR+SumG+SumB)/ChildBlkNum))
//                 For different LV : Low; Middle; High
//varaiable name: i4ScalingFactor
//default value: 6
//data range: 6~10
//6 means Wieght will be 1~12
//7 means Weight will be 1~6
//8 means Weight will be 1~3
//9 means Weight will be 1~2
//9 means Weight will be 1~2
//10 means Weight will be 1
//varaiable name: i4GammaLut[17]
//data range: 0 to 256
//varaiable name: i4WeightLut[3][17]
//data range: 1 to 15
/******************************************************************************/
        // Parent block weight parameter
        {
    	     1, // i4Mode : 0:Disable 1:Linear weighting 2:Weighting LUT
    	     6, // i4ScalingFactor: [6] 1~12, [7] 1~6, [8] 1~3, [9] 1~2, [>=10]: 1
    	     { 20,  70, 120}, // i4LvThld[3]
    	     // i4GammaLut[17]
    	     {0, 64, 90, 111, 128, 143, 156, 169, 181, 192, 202, 212, 221, 230, 239, 247, 256},
    	     // i4WeightLut[3][17] : Low, Middle, High
    	     {
    	         {4, 12, 14, 13, 10, 7, 6, 5, 5, 4, 4, 4, 3, 3, 3, 3, 2},
    	         {4, 12, 14, 13, 10, 7, 6, 5, 5, 4, 4, 4, 3, 3, 3, 3, 2},
    	         {4, 12, 14, 13, 10, 7, 6, 5, 5, 4, 4, 4, 3, 3, 3, 3, 2},
             },
        },
/******************************************************************************/
//Feature name:
//AWB behavior control for one-shot capture for dark environment
//variable name: bSmoothEnable
//default value: 0
//data range: 0 and 1, 0 means false and 1 means true
//MFALSE means disable smooth one-shot AWB for dark environment: use preview AWB gain directly
//MTRUE means enable smooth one-shot AWB for dark environment: take weighted average of one-shot AWB gain and
//preview AWB gain based on scene LV
//variable name: i4LVThrL
//take preview AWB gain only when scene LV <= i4LVThrL
//default value: N/A
//data range: 0 ~ 180
//variable name: i4LVThrH
//take one-shot AWB gain only when scene LV >= i4LVThrH
//default value: N/A
//data range: 0 ~ 180
//It perform interpolation when i4LVThrH > LV > i4LVThrL
/******************************************************************************/
        // One-shot AWB parameter
        {
            0, // bSmoothEnable
            10, // i4LVThrL
            50  // i4LVThrH
        },
/******************************************************************************/
//Feature name:
//MWB window size in Xr Yr domain to control the white point window size
//variable name: i4WindowSizeX
//default value: 32
//variable name: i4WindowSizeY
//default value: 32
/******************************************************************************/
        // MWB parameter
        {
            32, //i4WindowSizeX
            32  //i4WindowSizeY
        },
/******************************************************************************/
//Feature name:
//Misc function 
//variable name: i4CctGainSel
//Estimate CCT according to AWB gain. 0: AWB gain 1: AWB full gain 2: AWB gain without preference
//default value: 0
//variable name: i4StrobePbReduceX
//Reduce parent block window X size during strobe
//default value: 0
//variable name: i4StrobePbReduceY
//Reduce parent block window Y size during strobe
//default value: 0
/******************************************************************************/
        // Misc parameter
        {
            0, //i4CctGainSel
            0, //i4StrobePbReduceX
            0  //i4StrobePbReduceY
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
             6735, // i4ProjCode
             5588, // i4Model
             20150624, // i4Date
             0, // i4Res0
             1, // i4Res1
             2, // i4Res2
             3, // i4Res3
             4, // i4Res4
        }
    },
    // AWB Statistics Parameter
    {
        // Number of AWB windows
        120, // Number of horizontal AWB windows
         90, // Number of vertical AWB windows

        // Thresholds
           1, // Low threshold of R
           1, // Low threshold of G
           1, // Low threshold of B
         254, // High threshold of R
         254, // High threshold of G
         254, // High threshold of B

        // Pre-gain maximum limit clipping
        0xFFF, // Maximum limit clipping for R color
        0xFFF, // Maximum limit clipping for G color
        0xFFF, // Maximum limit clipping for B color

        // AWB error threshold
          0, // Programmable threshold for the allowed total over-exposed and under-exposed pixels in one main stat window

        // AWB error count shift bits
          0, // Programmable error count shift bits: 0 ~ 7; note: AWB statistics provide 4-bits error count output only

        // AWB error pixel ratio
         20, // Programmable error pixel count by AWB window size (base : 256)

        // AWB motion error pixel ratio
         20  // Programmable motion error pixel count by AWB window size (base : 256)
    }
};

