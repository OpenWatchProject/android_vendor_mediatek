/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

#include "camera_custom_types.h"
#include "camera_custom_nvram.h"
#include "ae_feature.h"
#include <aaa_types.h>
#include <custom/aaa/AEPlinetable.h>
//#include "awb_param.h"
//#include "ae_param.h"
#include "ae_tuning_custom.h"

using namespace NSIspTuning;
/*******************************************************************************
*
********************************************************************************/
template <>
AE_PARAM_TEMP_T const&
getAEParam<ESensorDev_Main>()
{
    static strHdrTargetControlCFG g_strHdrTargetControlCFG =
    {
        {
            5,                            // u4TblLength
            { -1500, -500, 1000, 3500, 8000 },  // i4BV4HDRTarget
            {    45,   58,   55,   63,   67 }   // u4HDRTarget
        },
        {
            2,                            // u4TblLength
            {   150,   400},              // i4HDRRatio4targetFusion
            {  1024,  1024}               // u4HDRTargetFusionRate
        }
    };
    static strHdrRatioCFG g_strHdrRatioCFG =
    {
        {                                 // HdrMaxRatioInfo
            1600,                         // i4MaxHDRRatio
            1600,                         // i4MaxHDRRatio_Face
            4,
            { -1000, 1000, 3500, 8000 },  // i4BV4MaxHDRRatio
            {  6400, 6400, 6400, 6400 }   // u4MaxHDRRatio_BV
        },
        {                                 // HdrRatioConvergeInfo
            20,                           // i4HDRRatioInTolerance
            30,                           // i4HDRRatioOutTolerance
            3,                            // i4HDRStableCountTHD
        },
        {                                 // HdrRatioTargetInfo
            FALSE,                        // bEnableHLAvgRatioControl;
            200,                          // u4HDRHLRatio
            120,                          // u4HDRHLTarget
            TRUE,                         // bEnableHistOERatioControl
            5,                            // u4TblLength
            {    0,   3, 10,  20,   25 },  // i4OEPct
            {    0,   0,100, 150,  180 },  // i4RatioMovOE1
            { -100, -30,  0,   0,    0 },  // i4RatioMovOE2
            {    1,  129}                 // u4OEBin
        },
        {                                 // HdrRatioDecreaseInfo
            TRUE,                         // bEnableBVDecrease
            4,                            // u4BVTblLength
            { -1000, 1000, 3500, 8000 },  // i4BV4HDRRatioDecrease
            {   100,  100,  100,  100 },  // u4RatioDecresaeRate4BV

            TRUE,                         // bEnableOEDecrease
            10,                           // u4OEBinN
            4,                            // u4OETblLength
            {    10,    20,   30,   40},  // u4OERate4HDRDecrease
            {   100,   100,  100,  100}   // u4RatioDecreaseRate4OE
        }
    };
    static str3ExpomVHdrCFG g_str3ExpomVHdrCFG =
    {
        {   //str3ExpomVHdrRatioSmoothTbl
            TRUE,                                       // bEnable
            4,                                          // u4TblLength
            {     30,   100,   400,   800 },            // u4RatioPcent_X
            {    512,   300,   256,   128 },            // u4RatioPcent_Y
        },
        {   //str3ExpomVHdrRatioLimitTbl
            TRUE,                                       // bEnable
            4,                                          // u4TblLength
            {     100,   300,  1000, 2000},             // u4RatioLimit_X
            {    1024,   128,     0,    0},             // u4RatioLimit_Y
        },
    };
    static strHdrSmoothCFG g_strHdrSmoothCFG =
    {
        950,            //u4B2DLinearThd
        -45,            // i4B2DBVACCExtreamBound
        -15,            //i4B2DBVACCNonlinearBound
        -10,           //i4B2DNonlinearEVDBound;
        -10,           //i4B2DLinearEVDBound;
    
        750,          //u4D2BLinearThd;
        50,            //i4D2BNonlinearEVDBound;
        50,            //i4D2BLinearEVDBound;
    };
    static AE_PARAM_TEMP_T strAEInitPara =
    {
        &g_strHdrTargetControlCFG,
        &g_strHdrRatioCFG,
        &g_str3ExpomVHdrCFG,
        &g_strHdrSmoothCFG,
    };

    return strAEInitPara;
}

template <>
AE_PARAM_TEMP_T const&
getHDRAEParam<ESensorDev_Main>()
{
    static strHdrTargetControlCFG g_strHdrTargetControlCFG =
    {
        {
            4,                            // u4TblLength
            { -1000, 1000, 3500, 8000 },  // i4BV4HDRTarget
            {    45,   55,   63,   67 }   // u4HDRTarget
        },
        {
            2,                            // u4TblLength
            {   150,  400},               // i4HDRRatio4targetFusion
            {     0,  0}               // u4HDRTargetFusionRate
        }
    };
    static strHdrRatioCFG g_strHdrRatioCFG =
    {
        {                                 // HdrMaxRatioInfo
            400,                         // i4MaxHDRRatio
            200,                          // i4MaxHDRRatio_Face
            4,
            { -1000, 1000, 3500, 8000 },  // i4BV4MaxHDRRatio
            {   100,  200,  400,  800 }   // u4MaxHDRRatio_BV
        },
        {                                 // HdrRatioConvergeInfo
            15,                           // i4HDRRatioInTolerance
            30,                           // i4HDRRatioOutTolerance
            3,                            // i4HDRStableCountTHD
        },
        {                                 // HdrRatioTargetInfo
            FALSE,                        // bEnableHLAvgRatioControl;
            200,                          // u4HDRHLRatio
            120,                          // u4HDRHLTarget
            TRUE,                         // bEnableHistOERatioControl
            5,                            // u4TblLength
            {    0,  5, 15,  20,   30 },  // i4OEPct
            {    0,  0,100, 150,  180 },  // i4RatioMovOE1
            { -100,  0,  0,   0,    0 },  // i4RatioMovOE2
            {    1,  129}                 // u4OEBin
        },
        {                                 // HdrRatioDecreaseInfo
            TRUE,                         // bEnableBVDecrease
            4,                            // u4BVTblLength
            { -1000, 1000, 3500, 8000 },  // i4BV4HDRRatioDecrease
            {    50,   80,  100,  100 },  // u4RatioDecresaeRate4BV
    
            TRUE,                         // bEnableOEDecrease
            10,                           // u4OEBinN
            4,                            // u4OETblLength
            {    10,   20,   30,   40 },  // u4OERate4HDRDecrease
            {    80,   80,  100,  100 }   // u4RatioDecreaseRate4OE
        }
    };
    static str3ExpomVHdrCFG g_str3ExpomVHdrCFG =
    {
        {   //str3ExpomVHdrRatioSmoothTbl
            TRUE,                                                        // bEnable
            4,                                                             // u4TblLength
            {     30,   100,   400,   800 },            // u4RatioPcent_X
            {   1024,   360,   256,   128 },            // u4RatioPcent_Y
        },
        {   //str3ExpomVHdrRatioLimitTbl
            TRUE,                                                        // bEnable
            4,                                                             // u4TblLength
            {     100,   500,  1000, 2000},             // u4RatioLimit_X
            {    1024,   768,   512,    0},             // u4RatioLimit_Y
        },
    };
    static strHdrSmoothCFG g_strHdrSmoothCFG =
    {
        950,            //u4B2DLinearThd
        -45,            // i4B2DBVACCExtreamBound
        -15,            //i4B2DBVACCNonlinearBound
        -10,           //i4B2DNonlinearEVDBound;
        -10,           //i4B2DLinearEVDBound;

        750,          //u4D2BLinearThd;
        50,            //i4D2BNonlinearEVDBound;
        50,            //i4D2BLinearEVDBound;
    };
    static AE_PARAM_TEMP_T strAEInitPara =
    {
        &g_strHdrTargetControlCFG,
        &g_strHdrRatioCFG,
        &g_str3ExpomVHdrCFG,
        &g_strHdrSmoothCFG,
    };

    return strAEInitPara;
}
template <>
AE_PARAM_TEMP_T const&
getAUTOHDRAEParam<ESensorDev_Main>()
{
    static strHdrTargetControlCFG g_strHdrTargetControlCFG =
    {
        {
            4,                            // u4TblLength
            { -1000, 1000, 3500, 8000 },  // i4BV4HDRTarget
            {    42,   42,   47,   50 }   // u4HDRTarget
        },
        {
            2,                            // u4TblLength
            {   150,  400},               // i4HDRRatio4targetFusion
            {     0,  0}               // u4HDRTargetFusionRate
        }
    };
    static strHdrRatioCFG g_strHdrRatioCFG =
    {
        {                                 // HdrMaxRatioInfo
            400,                         // i4MaxHDRRatio
            200,                          // i4MaxHDRRatio_Face
            4,
            { -1000, 1000, 3500, 8000 },  // i4BV4MaxHDRRatio
            {   100,  200,  400,  800 }   // u4MaxHDRRatio_BV
        },
        {                                 // HdrRatioConvergeInfo
            15,                           // i4HDRRatioInTolerance
            30,                           // i4HDRRatioOutTolerance
            3,                            // i4HDRStableCountTHD
        },
        {                                 // HdrRatioTargetInfo
            FALSE,                        // bEnableHLAvgRatioControl;
            200,                          // u4HDRHLRatio
            120,                          // u4HDRHLTarget
            TRUE,                         // bEnableHistOERatioControl
            5,                            // u4TblLength
            {    0,  5, 15,  20,   30 },  // i4OEPct
            {    0,  0,100, 150,  180 },  // i4RatioMovOE1
            { -100,  0,  0,   0,    0 },  // i4RatioMovOE2
            {    1,  129}                 // u4OEBin
        },
        {                                 // HdrRatioDecreaseInfo
            TRUE,                         // bEnableBVDecrease
            4,                            // u4BVTblLength
            { -1000, 1000, 3500, 8000 },  // i4BV4HDRRatioDecrease
            {    50,   80,  100,  100 },  // u4RatioDecresaeRate4BV

            TRUE,                         // bEnableOEDecrease
            10,                           // u4OEBinN
            4,                            // u4OETblLength
            {    10,   20,   30,   40 },  // u4OERate4HDRDecrease
            {    80,   80,  100,  100 }   // u4RatioDecreaseRate4OE
        }
    };
    static str3ExpomVHdrCFG g_str3ExpomVHdrCFG =
    {
        {   //str3ExpomVHdrRatioSmoothTbl
            TRUE,                                                        // bEnable
            4,                                                             // u4TblLength
            {     50,   200,   400,   800 },            // u4RatioPcent_X
            {   1024,   512,   256,   128 },            // u4RatioPcent_Y
        },
        {   //str3ExpomVHdrRatioLimitTbl
            TRUE,                                                        // bEnable
            4,                                                             // u4TblLength
            {     100,   500,  1000,   2000 },           // u4RatioLimit_X
            {    1024,   768,   512,   0 },           // u4RatioLimit_Y
        },
    };
    static strHdrSmoothCFG g_strHdrSmoothCFG =
    {
        950,            //u4B2DLinearThd
        -45,            // i4B2DBVACCExtreamBound
        -15,            //i4B2DBVACCNonlinearBound
        -10,           //i4B2DNonlinearEVDBound;
        -10,           //i4B2DLinearEVDBound;

        750,          //u4D2BLinearThd;
        50,            //i4D2BNonlinearEVDBound;
        50,            //i4D2BLinearEVDBound;
    };
    static AE_PARAM_TEMP_T strAEInitPara =
    {
        &g_strHdrTargetControlCFG,
        &g_strHdrRatioCFG,
        &g_str3ExpomVHdrCFG,
        &g_strHdrSmoothCFG,
    };

    return strAEInitPara;
}

template <>
AE_PARAM_TEMP_T const&
getVTAEParam<ESensorDev_Main>()
{
    static strHdrTargetControlCFG g_strHdrTargetControlCFG =
    {
        {
            4,                            // u4TblLength
            { -1000, 1000, 3500, 8000 },  // i4BV4HDRTarget
            {    42,   42,   47,   50 }   // u4HDRTarget
        },
        {
            2,                            // u4TblLength
            {   150,  400},               // i4HDRRatio4targetFusion
            {     0,  1024}               // u4HDRTargetFusionRate
        }
    };
    static strHdrRatioCFG g_strHdrRatioCFG =
    {
        {                                 // HdrMaxRatioInfo
            400,                         // i4MaxHDRRatio
            200,                          // i4MaxHDRRatio_Face
            4,
            { -1000, 1000, 3500, 8000 },  // i4BV4MaxHDRRatio
            {   100,  200,  400,  800 }   // u4MaxHDRRatio_BV
        },
        {                                 // HdrRatioConvergeInfo
            15,                           // i4HDRRatioInTolerance
            30,                           // i4HDRRatioOutTolerance
            3,                            // i4HDRStableCountTHD
        },
       {                                 // HdrRatioTargetInfo
           FALSE,                        // bEnableHLAvgRatioControl;
           200,                          // u4HDRHLRatio
           120,                          // u4HDRHLTarget
           TRUE,                         // bEnableHistOERatioControl
           5,                            // u4TblLength
           {    0,  5, 15,  20,   30 },  // i4OEPct
           {    0,  0,100, 150,  180 },  // i4RatioMovOE1
           { -100,  0,  0,   0,    0 },  // i4RatioMovOE2
           {    1,  129}                 // u4OEBin
       },
        {                                 // HdrRatioDecreaseInfo
            TRUE,                         // bEnableBVDecrease
            4,                            // u4BVTblLength
            { -1000, 1000, 3500, 8000 },  // i4BV4HDRRatioDecrease
            {    50,   80,  100,  100 },  // u4RatioDecresaeRate4BV

            TRUE,                         // bEnableOEDecrease
            10,                           // u4OEBinN
            4,                            // u4OETblLength
            {    10,   20,   30,   40 },  // u4OERate4HDRDecrease
            {    80,   80,  100,  100 }   // u4RatioDecreaseRate4OE
        }
    };
    static str3ExpomVHdrCFG g_str3ExpomVHdrCFG =
    {
        {   //str3ExpomVHdrRatioSmoothTbl
            TRUE,                                                        // bEnable
            4,                                                             // u4TblLength
            {     50,   200,   400,   800 },            // u4RatioPcent_X
            {   1024,   512,   256,   128 },            // u4RatioPcent_Y
        },
        {   //str3ExpomVHdrRatioLimitTbl
            TRUE,                                                        // bEnable
            4,                                                             // u4TblLength
            {     100,   500,  1000,   2000 },           // u4RatioLimit_X
            {    1024,   768,   512,   0 },           // u4RatioLimit_Y
        },
    };
    static strHdrSmoothCFG g_strHdrSmoothCFG =
    {
        950,            //u4B2DLinearThd
        -45,            // i4B2DBVACCExtreamBound
        -15,            //i4B2DBVACCNonlinearBound
        -10,           //i4B2DNonlinearEVDBound;
        -10,           //i4B2DLinearEVDBound;

        750,          //u4D2BLinearThd;
        50,            //i4D2BNonlinearEVDBound;
        50,            //i4D2BLinearEVDBound;
    };
    static AE_PARAM_TEMP_T strAEInitPara =
    {
        &g_strHdrTargetControlCFG,
        &g_strHdrRatioCFG,
        &g_str3ExpomVHdrCFG,
        &g_strHdrSmoothCFG,
    };

    return strAEInitPara;
}



/*******************************************************************************
*
********************************************************************************/
template <>
MBOOL
isAEEnabled<ESensorDev_Main>()
{
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define AE_CYCLE_NUM (3)

template <>
const MINT32*
getAEActiveCycle<ESensorDev_Main>()
{
    // Default AE cycle
    static MINT32 i4AEActiveCycle[AE_CYCLE_NUM] =
    {
        MTRUE,
        MFALSE,
        MFALSE,
    };

    return (&i4AEActiveCycle[0]);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
template <>
MINT32
getAECycleNum<ESensorDev_Main>()
{
    return AE_CYCLE_NUM;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AE Param Mapping Table by Scenario
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
template <> AE_PARAM_TEMP_T const& getAEParamData<ESensorDev_Main, CAM_SCENARIO_PREVIEW>() {
    return getAEParam<ESensorDev_Main>();
}
template <> AE_PARAM_TEMP_T const& getAEParamData<ESensorDev_Main, CAM_SCENARIO_VIDEO>() {
    return getAEParam<ESensorDev_Main>();
}
template <> AE_PARAM_TEMP_T const& getAEParamData<ESensorDev_Main, CAM_SCENARIO_CAPTURE>() {
    return getAEParam<ESensorDev_Main>();
}
template <> AE_PARAM_TEMP_T const& getAEParamData<ESensorDev_Main, CAM_SCENARIO_CUSTOM1>() {
    return getHDRAEParam<ESensorDev_Main>();
}
template <> AE_PARAM_TEMP_T const& getAEParamData<ESensorDev_Main, CAM_SCENARIO_CUSTOM2>() {
    return getAUTOHDRAEParam<ESensorDev_Main>();
}
template <> AE_PARAM_TEMP_T const& getAEParamData<ESensorDev_Main, CAM_SCENARIO_CUSTOM3>() {
    return getVTAEParam<ESensorDev_Main>();
}
template <> AE_PARAM_TEMP_T const& getAEParamData<ESensorDev_Main, CAM_SCENARIO_CUSTOM4>() {
    return getAEParam<ESensorDev_Main>();
}
