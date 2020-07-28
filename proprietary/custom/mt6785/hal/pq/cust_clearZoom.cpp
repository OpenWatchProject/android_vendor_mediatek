#include "PQDSImpl.h"
#include "PQRszImpl.h"
#include <string.h>
extern "C" {

uint32_t RszLevel[ISP_SCENARIOS] =
{
    0,0,0,0,
    0,0,0,0,
    0,0,0,0,
    0
};

uint32_t iTdshpLevel[ISP_SCENARIOS] =
{
    0,0,0,0,
    0,0,0,0,
    0,0,0,0,
    0
};

RszReg RszEntrySWReg[UR_MAX_LEVEL] =
{
    {// RszEntrySWReg_0
    .tableMode = 0,
    .autoTableSelection = 1, // default table selection
    .autoAlignment = 1,
    .autoAlgSelection = 1,
    .IBSEEnable = 1,
    .ultraResEnable = 1,
    .autoCoefTrunc = 1,

    .defaultUpTable = 9,
    .defaultDownTable = 17,


    .switchRatio6Tap6nTap = 1,

    .switchRatio6nTapAcc = 24,  // <1/24x: Source Accumulation, >=1/24x: 6nTap

    // Ultra Resolution
    // for IBSE
    .dynIBSE_gain_ratio_thr0 = 1024,  // 1x
    .dynIBSE_gain_ratio_thr1 = 2048, // 2x
    .dynIBSE_gain_ratio_thr2 = 4096, // 4x
    .dynIBSE_gain_clip1 = 0,
    .dynIBSE_gain_clip2 = 0,
    .dynIBSE_gain_min = 0,
    .dynIBSE_gain_max = 31,

    .IBSE_clip_ratio = 0,
    .IBSE_clip_ratio_ratio_thr0 = 1024, // 1x
    .IBSE_clip_ratio_ratio_thr1 = 2048, // 2x
    .IBSE_clip_ratio_ratio_thr2 = 4096, // 4x
    .IBSE_clip_ratio_clip1 = 0, //0
    .IBSE_clip_ratio_clip2 = 0,
    .IBSE_clip_ratio_min = 0,
    .IBSE_clip_ratio_max = 31,

    .IBSE_clip_thr = 0, //3
    .IBSE_clip_thr_ratio_thr0 = 1024, // 1x
    .IBSE_clip_thr_ratio_thr1 = 2048, // 2x
    .IBSE_clip_thr_ratio_thr2 = 4096, // 4x
    .IBSE_clip_thr_clip1 = 0,
    .IBSE_clip_thr_clip2 = 0,
    .IBSE_clip_thr_min = 0,
    .IBSE_clip_thr_max = 255,

    .IBSE_gaincontrol_gain = 0,
    .IBSE_gaincontrol_gain_ratio_thr0 = 1024,
    .IBSE_gaincontrol_gain_ratio_thr1 = 2048,
    .IBSE_gaincontrol_gain_ratio_thr2 = 4096,
    .IBSE_gaincontrol_gain_clip1 = 0,
    .IBSE_gaincontrol_gain_clip2 = 0,
    .IBSE_gaincontrol_gain_min = 0,
    .IBSE_gaincontrol_gain_max = 255,

    // for tapAdaptive
    .tapAdapt_slope_ratio_thr0 = 1024,  // 1x
    .tapAdapt_slope_ratio_thr1 = 1536, // 1.5x
    .tapAdapt_slope_ratio_thr2 = 2048, // 2x
    .tapAdapt_slope_clip1 = 0,
    .tapAdapt_slope_clip2 = -7,
    .tapAdapt_slope_min = 0,
    .tapAdapt_slope_max = 15,
    },

    {// RszEntrySWReg_1
    .tableMode = 0,
    .autoTableSelection = 1, // default table selection
    .autoAlignment = 1,
    .autoAlgSelection = 1,
    .IBSEEnable = 1,
    .ultraResEnable = 1,
    .autoCoefTrunc = 1,

    .defaultUpTable = 9,
    .defaultDownTable = 17,


    .switchRatio6Tap6nTap = 1,

    .switchRatio6nTapAcc = 24,  // <1/24x: Source Accumulation, >=1/24x: 6nTap

    // Ultra Resolution
    // for IBSE
    .dynIBSE_gain_ratio_thr0 = 1024,  // 1x
    .dynIBSE_gain_ratio_thr1 = 2048, // 2x
    .dynIBSE_gain_ratio_thr2 = 4096, // 4x
    .dynIBSE_gain_clip1 = 3,
    .dynIBSE_gain_clip2 = 7,
    .dynIBSE_gain_min = 0,
    .dynIBSE_gain_max = 31,

    .IBSE_clip_ratio = 8,
    .IBSE_clip_ratio_ratio_thr0 = 1024, // 1x
    .IBSE_clip_ratio_ratio_thr1 = 2048, // 2x
    .IBSE_clip_ratio_ratio_thr2 = 4096, // 4x
    .IBSE_clip_ratio_clip1 = 5, //0
    .IBSE_clip_ratio_clip2 = -7,
    .IBSE_clip_ratio_min = 0,
    .IBSE_clip_ratio_max = 31,

    .IBSE_clip_thr = 3, //3
    .IBSE_clip_thr_ratio_thr0 = 1024, // 1x
    .IBSE_clip_thr_ratio_thr1 = 2048, // 2x
    .IBSE_clip_thr_ratio_thr2 = 4096, // 4x
    .IBSE_clip_thr_clip1 = 0,
    .IBSE_clip_thr_clip2 = -1,
    .IBSE_clip_thr_min = 0,
    .IBSE_clip_thr_max = 255,

    .IBSE_gaincontrol_gain = 90,
    .IBSE_gaincontrol_gain_ratio_thr0 = 1024,
    .IBSE_gaincontrol_gain_ratio_thr1 = 2048,
    .IBSE_gaincontrol_gain_ratio_thr2 = 4096,
    .IBSE_gaincontrol_gain_clip1 = 8,
    .IBSE_gaincontrol_gain_clip2 = 8,
    .IBSE_gaincontrol_gain_min = 0,
    .IBSE_gaincontrol_gain_max = 255,

    // for tapAdaptive
    .tapAdapt_slope_ratio_thr0 = 1024,  // 1x
    .tapAdapt_slope_ratio_thr1 = 1536, // 1.5x
    .tapAdapt_slope_ratio_thr2 = 2048, // 2x
    .tapAdapt_slope_clip1 = 0,
    .tapAdapt_slope_clip2 = -7,
    .tapAdapt_slope_min = 0,
    .tapAdapt_slope_max = 15,
    },

    {// RszEntrySWReg_2
    .tableMode = 0,
    .autoTableSelection = 1, // default table selection
    .autoAlignment = 1,
    .autoAlgSelection = 1,
    .IBSEEnable = 1,
    .ultraResEnable = 1,
    .autoCoefTrunc = 1,

    .defaultUpTable = 9,
    .defaultDownTable = 17,


    .switchRatio6Tap6nTap = 1,

    .switchRatio6nTapAcc = 24,  // <1/24x: Source Accumulation, >=1/24x: 6nTap

    // Ultra Resolution
    // for IBSE
    .dynIBSE_gain_ratio_thr0 = 1024,  // 1x
    .dynIBSE_gain_ratio_thr1 = 2048, // 2x
    .dynIBSE_gain_ratio_thr2 = 4096, // 4x
    .dynIBSE_gain_clip1 = 3,
    .dynIBSE_gain_clip2 = 7,
    .dynIBSE_gain_min = 0,
    .dynIBSE_gain_max = 31,

    .IBSE_clip_ratio = 8,
    .IBSE_clip_ratio_ratio_thr0 = 1024, // 1x
    .IBSE_clip_ratio_ratio_thr1 = 2048, // 2x
    .IBSE_clip_ratio_ratio_thr2 = 4096, // 4x
    .IBSE_clip_ratio_clip1 = 5, //0
    .IBSE_clip_ratio_clip2 = -7,
    .IBSE_clip_ratio_min = 0,
    .IBSE_clip_ratio_max = 31,

    .IBSE_clip_thr = 3, //3
    .IBSE_clip_thr_ratio_thr0 = 1024, // 1x
    .IBSE_clip_thr_ratio_thr1 = 2048, // 2x
    .IBSE_clip_thr_ratio_thr2 = 4096, // 4x
    .IBSE_clip_thr_clip1 = 0,
    .IBSE_clip_thr_clip2 = -1,
    .IBSE_clip_thr_min = 0,
    .IBSE_clip_thr_max = 255,

    .IBSE_gaincontrol_gain = 90,
    .IBSE_gaincontrol_gain_ratio_thr0 = 1024,
    .IBSE_gaincontrol_gain_ratio_thr1 = 2048,
    .IBSE_gaincontrol_gain_ratio_thr2 = 4096,
    .IBSE_gaincontrol_gain_clip1 = 8,
    .IBSE_gaincontrol_gain_clip2 = 8,
    .IBSE_gaincontrol_gain_min = 0,
    .IBSE_gaincontrol_gain_max = 255,

    // for tapAdaptive
    .tapAdapt_slope_ratio_thr0 = 1024,  // 1x
    .tapAdapt_slope_ratio_thr1 = 1536, // 1.5x
    .tapAdapt_slope_ratio_thr2 = 2048, // 2x
    .tapAdapt_slope_clip1 = 0,
    .tapAdapt_slope_clip2 = -7,
    .tapAdapt_slope_min = 0,
    .tapAdapt_slope_max = 15,
    },

    {// RszEntrySWReg_3
    .tableMode = 0,
    .autoTableSelection = 1, // default table selection
    .autoAlignment = 1,
    .autoAlgSelection = 1,
    .IBSEEnable = 1,
    .ultraResEnable = 1,
    .autoCoefTrunc = 1,

    .defaultUpTable = 9,
    .defaultDownTable = 17,


    .switchRatio6Tap6nTap = 1,

    .switchRatio6nTapAcc = 24,  // <1/24x: Source Accumulation, >=1/24x: 6nTap

    // Ultra Resolution
    // for IBSE
    .dynIBSE_gain_ratio_thr0 = 1024,  // 1x
    .dynIBSE_gain_ratio_thr1 = 2048, // 2x
    .dynIBSE_gain_ratio_thr2 = 4096, // 4x
    .dynIBSE_gain_clip1 = 3,
    .dynIBSE_gain_clip2 = 7,
    .dynIBSE_gain_min = 0,
    .dynIBSE_gain_max = 31,

    .IBSE_clip_ratio = 8,
    .IBSE_clip_ratio_ratio_thr0 = 1024, // 1x
    .IBSE_clip_ratio_ratio_thr1 = 2048, // 2x
    .IBSE_clip_ratio_ratio_thr2 = 4096, // 4x
    .IBSE_clip_ratio_clip1 = 5, //0
    .IBSE_clip_ratio_clip2 = -7,
    .IBSE_clip_ratio_min = 0,
    .IBSE_clip_ratio_max = 31,

    .IBSE_clip_thr = 3, //3
    .IBSE_clip_thr_ratio_thr0 = 1024, // 1x
    .IBSE_clip_thr_ratio_thr1 = 2048, // 2x
    .IBSE_clip_thr_ratio_thr2 = 4096, // 4x
    .IBSE_clip_thr_clip1 = 0,
    .IBSE_clip_thr_clip2 = -1,
    .IBSE_clip_thr_min = 0,
    .IBSE_clip_thr_max = 255,

    .IBSE_gaincontrol_gain = 90,
    .IBSE_gaincontrol_gain_ratio_thr0 = 1024,
    .IBSE_gaincontrol_gain_ratio_thr1 = 2048,
    .IBSE_gaincontrol_gain_ratio_thr2 = 4096,
    .IBSE_gaincontrol_gain_clip1 = 8,
    .IBSE_gaincontrol_gain_clip2 = 8,
    .IBSE_gaincontrol_gain_min = 0,
    .IBSE_gaincontrol_gain_max = 255,

    // for tapAdaptive
    .tapAdapt_slope_ratio_thr0 = 1024,  // 1x
    .tapAdapt_slope_ratio_thr1 = 1536, // 1.5x
    .tapAdapt_slope_ratio_thr2 = 2048, // 2x
    .tapAdapt_slope_clip1 = 0,
    .tapAdapt_slope_clip2 = -7,
    .tapAdapt_slope_min = 0,
    .tapAdapt_slope_max = 15,
    },

    {// RszEntrySWReg_4
    .tableMode = 0,
    .autoTableSelection = 1, // default table selection
    .autoAlignment = 1,
    .autoAlgSelection = 1,
    .IBSEEnable = 1,
    .ultraResEnable = 1,
    .autoCoefTrunc = 1,

    .defaultUpTable = 9,
    .defaultDownTable = 17,


    .switchRatio6Tap6nTap = 1,

    .switchRatio6nTapAcc = 24,  // <1/24x: Source Accumulation, >=1/24x: 6nTap

    // Ultra Resolution
    // for IBSE
    .dynIBSE_gain_ratio_thr0 = 1024,  // 1x
    .dynIBSE_gain_ratio_thr1 = 2048, // 2x
    .dynIBSE_gain_ratio_thr2 = 4096, // 4x
    .dynIBSE_gain_clip1 = 3,
    .dynIBSE_gain_clip2 = 7,
    .dynIBSE_gain_min = 0,
    .dynIBSE_gain_max = 31,

    .IBSE_clip_ratio = 8,
    .IBSE_clip_ratio_ratio_thr0 = 1024, // 1x
    .IBSE_clip_ratio_ratio_thr1 = 2048, // 2x
    .IBSE_clip_ratio_ratio_thr2 = 4096, // 4x
    .IBSE_clip_ratio_clip1 = 5, //0
    .IBSE_clip_ratio_clip2 = -7,
    .IBSE_clip_ratio_min = 0,
    .IBSE_clip_ratio_max = 31,

    .IBSE_clip_thr = 3, //3
    .IBSE_clip_thr_ratio_thr0 = 1024, // 1x
    .IBSE_clip_thr_ratio_thr1 = 2048, // 2x
    .IBSE_clip_thr_ratio_thr2 = 4096, // 4x
    .IBSE_clip_thr_clip1 = 0,
    .IBSE_clip_thr_clip2 = -1,
    .IBSE_clip_thr_min = 0,
    .IBSE_clip_thr_max = 255,

    .IBSE_gaincontrol_gain = 40,
    .IBSE_gaincontrol_gain_ratio_thr0 = 1024,
    .IBSE_gaincontrol_gain_ratio_thr1 = 2048,
    .IBSE_gaincontrol_gain_ratio_thr2 = 4096,
    .IBSE_gaincontrol_gain_clip1 = 8,
    .IBSE_gaincontrol_gain_clip2 = 8,
    .IBSE_gaincontrol_gain_min = 0,
    .IBSE_gaincontrol_gain_max = 255,

    // for tapAdaptive
    .tapAdapt_slope_ratio_thr0 = 1024,  // 1x
    .tapAdapt_slope_ratio_thr1 = 1536, // 1.5x
    .tapAdapt_slope_ratio_thr2 = 2048, // 2x
    .tapAdapt_slope_clip1 = 0,
    .tapAdapt_slope_clip2 = -7,
    .tapAdapt_slope_min = 0,
    .tapAdapt_slope_max = 15,
    },

    {// RszEntrySWReg_5
    .tableMode = 0,
    .autoTableSelection = 1, // default table selection
    .autoAlignment = 1,
    .autoAlgSelection = 1,
    .IBSEEnable = 1,
    .ultraResEnable = 1,
    .autoCoefTrunc = 1,

    .defaultUpTable = 9,
    .defaultDownTable = 17,


    .switchRatio6Tap6nTap = 1,

    .switchRatio6nTapAcc = 24,  // <1/24x: Source Accumulation, >=1/24x: 6nTap

    // Ultra Resolution
    // for IBSE
    .dynIBSE_gain_ratio_thr0 = 1024,  // 1x
    .dynIBSE_gain_ratio_thr1 = 2048, // 2x
    .dynIBSE_gain_ratio_thr2 = 4096, // 4x
    .dynIBSE_gain_clip1 = 3,
    .dynIBSE_gain_clip2 = 7,
    .dynIBSE_gain_min = 0,
    .dynIBSE_gain_max = 31,

    .IBSE_clip_ratio = 8,
    .IBSE_clip_ratio_ratio_thr0 = 1024, // 1x
    .IBSE_clip_ratio_ratio_thr1 = 2048, // 2x
    .IBSE_clip_ratio_ratio_thr2 = 4096, // 4x
    .IBSE_clip_ratio_clip1 = 5, //0
    .IBSE_clip_ratio_clip2 = -7,
    .IBSE_clip_ratio_min = 0,
    .IBSE_clip_ratio_max = 31,

    .IBSE_clip_thr = 3, //3
    .IBSE_clip_thr_ratio_thr0 = 1024, // 1x
    .IBSE_clip_thr_ratio_thr1 = 2048, // 2x
    .IBSE_clip_thr_ratio_thr2 = 4096, // 4x
    .IBSE_clip_thr_clip1 = 0,
    .IBSE_clip_thr_clip2 = -1,
    .IBSE_clip_thr_min = 0,
    .IBSE_clip_thr_max = 255,

    .IBSE_gaincontrol_gain = 90,
    .IBSE_gaincontrol_gain_ratio_thr0 = 1024,
    .IBSE_gaincontrol_gain_ratio_thr1 = 2048,
    .IBSE_gaincontrol_gain_ratio_thr2 = 4096,
    .IBSE_gaincontrol_gain_clip1 = 8,
    .IBSE_gaincontrol_gain_clip2 = 8,
    .IBSE_gaincontrol_gain_min = 0,
    .IBSE_gaincontrol_gain_max = 255,

    // for tapAdaptive
    .tapAdapt_slope_ratio_thr0 = 1024,  // 1x
    .tapAdapt_slope_ratio_thr1 = 1536, // 1.5x
    .tapAdapt_slope_ratio_thr2 = 2048, // 2x
    .tapAdapt_slope_clip1 = 0,
    .tapAdapt_slope_clip2 = -7,
    .tapAdapt_slope_min = 0,
    .tapAdapt_slope_max = 15,
    },

    {// RszEntrySWReg_6
    .tableMode = 0,
    .autoTableSelection = 1, // default table selection
    .autoAlignment = 1,
    .autoAlgSelection = 1,
    .IBSEEnable = 1,
    .ultraResEnable = 1,
    .autoCoefTrunc = 1,

    .defaultUpTable = 9,
    .defaultDownTable = 17,


    .switchRatio6Tap6nTap = 1,

    .switchRatio6nTapAcc = 24,  // <1/24x: Source Accumulation, >=1/24x: 6nTap

    // Ultra Resolution
    // for IBSE
    .dynIBSE_gain_ratio_thr0 = 1024,  // 1x
    .dynIBSE_gain_ratio_thr1 = 2048, // 2x
    .dynIBSE_gain_ratio_thr2 = 4096, // 4x
    .dynIBSE_gain_clip1 = 16,
    .dynIBSE_gain_clip2 = 16,
    .dynIBSE_gain_min = 0,
    .dynIBSE_gain_max = 31,

    .IBSE_clip_ratio = 8,
    .IBSE_clip_ratio_ratio_thr0 = 1024, // 1x
    .IBSE_clip_ratio_ratio_thr1 = 2048, // 2x
    .IBSE_clip_ratio_ratio_thr2 = 4096, // 4x
    .IBSE_clip_ratio_clip1 = 16, //0
    .IBSE_clip_ratio_clip2 = 16,
    .IBSE_clip_ratio_min = 0,
    .IBSE_clip_ratio_max = 31,

    .IBSE_clip_thr = 3, //3
    .IBSE_clip_thr_ratio_thr0 = 1024, // 1x
    .IBSE_clip_thr_ratio_thr1 = 2048, // 2x
    .IBSE_clip_thr_ratio_thr2 = 4096, // 4x
    .IBSE_clip_thr_clip1 = 16,
    .IBSE_clip_thr_clip2 = 16,
    .IBSE_clip_thr_min = 0,
    .IBSE_clip_thr_max = 255,

    .IBSE_gaincontrol_gain = 0,
    .IBSE_gaincontrol_gain_ratio_thr0 = 1024,
    .IBSE_gaincontrol_gain_ratio_thr1 = 2048,
    .IBSE_gaincontrol_gain_ratio_thr2 = 4096,
    .IBSE_gaincontrol_gain_clip1 = 8,
    .IBSE_gaincontrol_gain_clip2 = 6,
    .IBSE_gaincontrol_gain_min = 0,
    .IBSE_gaincontrol_gain_max = 255,

    // for tapAdaptive
    .tapAdapt_slope_ratio_thr0 = 1024,  // 1x
    .tapAdapt_slope_ratio_thr1 = 2048, // 1.5x
    .tapAdapt_slope_ratio_thr2 = 4096, // 2x
    .tapAdapt_slope_clip1 = 16,
    .tapAdapt_slope_clip2 = 16,
    .tapAdapt_slope_min = 0,
    .tapAdapt_slope_max = 31,
    },

    {// RszEntrySWReg_7
    .tableMode = 0,
    .autoTableSelection = 1, // default table selection
    .autoAlignment = 1,
    .autoAlgSelection = 1,
    .IBSEEnable = 1,
    .ultraResEnable = 1,
    .autoCoefTrunc = 1,

    .defaultUpTable = 9,
    .defaultDownTable = 17,


    .switchRatio6Tap6nTap = 1,

    .switchRatio6nTapAcc = 24,  // <1/24x: Source Accumulation, >=1/24x: 6nTap

    // Ultra Resolution
    // for IBSE
    .dynIBSE_gain_ratio_thr0 = 1024,  // 1x
    .dynIBSE_gain_ratio_thr1 = 2048, // 2x
    .dynIBSE_gain_ratio_thr2 = 4096, // 4x
    .dynIBSE_gain_clip1 = 3,
    .dynIBSE_gain_clip2 = 7,
    .dynIBSE_gain_min = 0,
    .dynIBSE_gain_max = 31,

    .IBSE_clip_ratio = 8,
    .IBSE_clip_ratio_ratio_thr0 = 1024, // 1x
    .IBSE_clip_ratio_ratio_thr1 = 2048, // 2x
    .IBSE_clip_ratio_ratio_thr2 = 4096, // 4x
    .IBSE_clip_ratio_clip1 = 5, //0
    .IBSE_clip_ratio_clip2 = -7,
    .IBSE_clip_ratio_min = 0,
    .IBSE_clip_ratio_max = 31,

    .IBSE_clip_thr = 3, //3
    .IBSE_clip_thr_ratio_thr0 = 1024, // 1x
    .IBSE_clip_thr_ratio_thr1 = 2048, // 2x
    .IBSE_clip_thr_ratio_thr2 = 4096, // 4x
    .IBSE_clip_thr_clip1 = 0,
    .IBSE_clip_thr_clip2 = -1,
    .IBSE_clip_thr_min = 0,
    .IBSE_clip_thr_max = 255,

    .IBSE_gaincontrol_gain = 90,
    .IBSE_gaincontrol_gain_ratio_thr0 = 1024,
    .IBSE_gaincontrol_gain_ratio_thr1 = 2048,
    .IBSE_gaincontrol_gain_ratio_thr2 = 4096,
    .IBSE_gaincontrol_gain_clip1 = 8,
    .IBSE_gaincontrol_gain_clip2 = 8,
    .IBSE_gaincontrol_gain_min = 0,
    .IBSE_gaincontrol_gain_max = 255,

    // for tapAdaptive
    .tapAdapt_slope_ratio_thr0 = 1024,  // 1x
    .tapAdapt_slope_ratio_thr1 = 1536, // 1.5x
    .tapAdapt_slope_ratio_thr2 = 2048, // 2x
    .tapAdapt_slope_clip1 = 0,
    .tapAdapt_slope_clip2 = -7,
    .tapAdapt_slope_min = 0,
    .tapAdapt_slope_max = 15,
    },

    {// RszEntrySWReg_8
    .tableMode = 0,
    .autoTableSelection = 1, // default table selection
    .autoAlignment = 1,
    .autoAlgSelection = 1,
    .IBSEEnable = 1,
    .ultraResEnable = 1,
    .autoCoefTrunc = 1,

    .defaultUpTable = 9,
    .defaultDownTable = 17,


    .switchRatio6Tap6nTap = 1,

    .switchRatio6nTapAcc = 24,  // <1/24x: Source Accumulation, >=1/24x: 6nTap

    // Ultra Resolution
    // for IBSE
    .dynIBSE_gain_ratio_thr0 = 1024,  // 1x
    .dynIBSE_gain_ratio_thr1 = 2048, // 2x
    .dynIBSE_gain_ratio_thr2 = 4096, // 4x
    .dynIBSE_gain_clip1 = 3,
    .dynIBSE_gain_clip2 = 7,
    .dynIBSE_gain_min = 0,
    .dynIBSE_gain_max = 31,

    .IBSE_clip_ratio = 8,
    .IBSE_clip_ratio_ratio_thr0 = 1024, // 1x
    .IBSE_clip_ratio_ratio_thr1 = 2048, // 2x
    .IBSE_clip_ratio_ratio_thr2 = 4096, // 4x
    .IBSE_clip_ratio_clip1 = 5, //0
    .IBSE_clip_ratio_clip2 = -7,
    .IBSE_clip_ratio_min = 0,
    .IBSE_clip_ratio_max = 31,

    .IBSE_clip_thr = 3, //3
    .IBSE_clip_thr_ratio_thr0 = 1024, // 1x
    .IBSE_clip_thr_ratio_thr1 = 2048, // 2x
    .IBSE_clip_thr_ratio_thr2 = 4096, // 4x
    .IBSE_clip_thr_clip1 = 0,
    .IBSE_clip_thr_clip2 = -1,
    .IBSE_clip_thr_min = 0,
    .IBSE_clip_thr_max = 255,

    .IBSE_gaincontrol_gain = 90,
    .IBSE_gaincontrol_gain_ratio_thr0 = 1024,
    .IBSE_gaincontrol_gain_ratio_thr1 = 2048,
    .IBSE_gaincontrol_gain_ratio_thr2 = 4096,
    .IBSE_gaincontrol_gain_clip1 = 8,
    .IBSE_gaincontrol_gain_clip2 = 8,
    .IBSE_gaincontrol_gain_min = 0,
    .IBSE_gaincontrol_gain_max = 255,

    // for tapAdaptive
    .tapAdapt_slope_ratio_thr0 = 1024,  // 1x
    .tapAdapt_slope_ratio_thr1 = 1536, // 1.5x
    .tapAdapt_slope_ratio_thr2 = 2048, // 2x
    .tapAdapt_slope_clip1 = 0,
    .tapAdapt_slope_clip2 = -7,
    .tapAdapt_slope_min = 0,
    .tapAdapt_slope_max = 15,
    },

    {// RszEntrySWReg_9
    .tableMode = 0,
    .autoTableSelection = 1, // default table selection
    .autoAlignment = 1,
    .autoAlgSelection = 1,
    .IBSEEnable = 1,
    .ultraResEnable = 1,
    .autoCoefTrunc = 1,

    .defaultUpTable = 9,
    .defaultDownTable = 17,


    .switchRatio6Tap6nTap = 1,

    .switchRatio6nTapAcc = 24,  // <1/24x: Source Accumulation, >=1/24x: 6nTap

    // Ultra Resolution
    // for IBSE
    .dynIBSE_gain_ratio_thr0 = 1024,  // 1x
    .dynIBSE_gain_ratio_thr1 = 2048, // 2x
    .dynIBSE_gain_ratio_thr2 = 4096, // 4x
    .dynIBSE_gain_clip1 = 3,
    .dynIBSE_gain_clip2 = 7,
    .dynIBSE_gain_min = 0,
    .dynIBSE_gain_max = 31,

    .IBSE_clip_ratio = 8,
    .IBSE_clip_ratio_ratio_thr0 = 1024, // 1x
    .IBSE_clip_ratio_ratio_thr1 = 2048, // 2x
    .IBSE_clip_ratio_ratio_thr2 = 4096, // 4x
    .IBSE_clip_ratio_clip1 = 5, //0
    .IBSE_clip_ratio_clip2 = -7,
    .IBSE_clip_ratio_min = 0,
    .IBSE_clip_ratio_max = 31,

    .IBSE_clip_thr = 3, //3
    .IBSE_clip_thr_ratio_thr0 = 1024, // 1x
    .IBSE_clip_thr_ratio_thr1 = 2048, // 2x
    .IBSE_clip_thr_ratio_thr2 = 4096, // 4x
    .IBSE_clip_thr_clip1 = 0,
    .IBSE_clip_thr_clip2 = -1,
    .IBSE_clip_thr_min = 0,
    .IBSE_clip_thr_max = 255,

    .IBSE_gaincontrol_gain = 90,
    .IBSE_gaincontrol_gain_ratio_thr0 = 1024,
    .IBSE_gaincontrol_gain_ratio_thr1 = 2048,
    .IBSE_gaincontrol_gain_ratio_thr2 = 4096,
    .IBSE_gaincontrol_gain_clip1 = 8,
    .IBSE_gaincontrol_gain_clip2 = 8,
    .IBSE_gaincontrol_gain_min = 0,
    .IBSE_gaincontrol_gain_max = 255,

    // for tapAdaptive
    .tapAdapt_slope_ratio_thr0 = 1024,  // 1x
    .tapAdapt_slope_ratio_thr1 = 1536, // 1.5x
    .tapAdapt_slope_ratio_thr2 = 2048, // 2x
    .tapAdapt_slope_clip1 = 0,
    .tapAdapt_slope_clip2 = -7,
    .tapAdapt_slope_min = 0,
    .tapAdapt_slope_max = 15,
    },

    {// RszEntrySWReg_10
    .tableMode = 0,
    .autoTableSelection = 1, // default table selection
    .autoAlignment = 1,
    .autoAlgSelection = 1,
    .IBSEEnable = 1,
    .ultraResEnable = 1,
    .autoCoefTrunc = 1,

    .defaultUpTable = 9,
    .defaultDownTable = 17,


    .switchRatio6Tap6nTap = 1,

    .switchRatio6nTapAcc = 24,  // <1/24x: Source Accumulation, >=1/24x: 6nTap

    // Ultra Resolution
    // for IBSE
    .dynIBSE_gain_ratio_thr0 = 1024,  // 1x
    .dynIBSE_gain_ratio_thr1 = 2048, // 2x
    .dynIBSE_gain_ratio_thr2 = 4096, // 4x
    .dynIBSE_gain_clip1 = 3,
    .dynIBSE_gain_clip2 = 7,
    .dynIBSE_gain_min = 0,
    .dynIBSE_gain_max = 31,

    .IBSE_clip_ratio = 8,
    .IBSE_clip_ratio_ratio_thr0 = 1024, // 1x
    .IBSE_clip_ratio_ratio_thr1 = 2048, // 2x
    .IBSE_clip_ratio_ratio_thr2 = 4096, // 4x
    .IBSE_clip_ratio_clip1 = 5, //0
    .IBSE_clip_ratio_clip2 = -7,
    .IBSE_clip_ratio_min = 0,
    .IBSE_clip_ratio_max = 31,

    .IBSE_clip_thr = 3, //3
    .IBSE_clip_thr_ratio_thr0 = 1024, // 1x
    .IBSE_clip_thr_ratio_thr1 = 2048, // 2x
    .IBSE_clip_thr_ratio_thr2 = 4096, // 4x
    .IBSE_clip_thr_clip1 = 0,
    .IBSE_clip_thr_clip2 = -1,
    .IBSE_clip_thr_min = 0,
    .IBSE_clip_thr_max = 255,

    .IBSE_gaincontrol_gain = 90,
    .IBSE_gaincontrol_gain_ratio_thr0 = 1024,
    .IBSE_gaincontrol_gain_ratio_thr1 = 2048,
    .IBSE_gaincontrol_gain_ratio_thr2 = 4096,
    .IBSE_gaincontrol_gain_clip1 = 8,
    .IBSE_gaincontrol_gain_clip2 = 8,
    .IBSE_gaincontrol_gain_min = 0,
    .IBSE_gaincontrol_gain_max = 255,

    // for tapAdaptive
    .tapAdapt_slope_ratio_thr0 = 1024,  // 1x
    .tapAdapt_slope_ratio_thr1 = 1536, // 1.5x
    .tapAdapt_slope_ratio_thr2 = 2048, // 2x
    .tapAdapt_slope_clip1 = 0,
    .tapAdapt_slope_clip2 = -7,
    .tapAdapt_slope_min = 0,
    .tapAdapt_slope_max = 15,
    }
};

RszRegClearZoom RszEntryHWReg[UR_MAX_LEVEL] =
{
    {// RszEntryHWReg_0
    .IBSE_gaincontrol_coring_value = 0,
    .IBSE_gaincontrol_coring_thr = 0,
    .IBSE_gaincontrol_coring_zero = 0,
    .IBSE_gaincontrol_softlimit_ratio = 0,
    .IBSE_gaincontrol_bound = 0,
    .IBSE_gaincontrol_limit = 0,
    .IBSE_gaincontrol_softcoring_gain = 0,
    // ylevel_gain
    .IBSE_ylevel_p48 = 72,
    .IBSE_ylevel_p32 = 48,
    .IBSE_ylevel_p16 = 24,
    .IBSE_ylevel_p0 = 0,
    .IBSE_ylevel_p112 = 120,
    .IBSE_ylevel_p96 = 112,
    .IBSE_ylevel_p80 = 104,
    .IBSE_ylevel_p64 = 96,
    .IBSE_ylevel_p176 = 112,
    .IBSE_ylevel_p160 = 118,
    .IBSE_ylevel_p144 = 123,
    .IBSE_ylevel_p128 = 128,
    .IBSE_ylevel_p240 = 91,
    .IBSE_ylevel_p224 = 96,
    .IBSE_ylevel_p208 = 102,
    .IBSE_ylevel_p192 = 107,
    .IBSE_ylevel_alpha = 16,
    .IBSE_ylevel_p256 = 86
    },

    {// RszEntryHWReg_1
    .IBSE_gaincontrol_coring_value = 3,
    .IBSE_gaincontrol_coring_thr = 4,
    .IBSE_gaincontrol_coring_zero = 2,
    .IBSE_gaincontrol_softlimit_ratio = 8,
    .IBSE_gaincontrol_bound = 48,
    .IBSE_gaincontrol_limit = 24,
    .IBSE_gaincontrol_softcoring_gain = 8,
    // ylevel_gain
    .IBSE_ylevel_p48 = 72,
    .IBSE_ylevel_p32 = 48,
    .IBSE_ylevel_p16 = 24,
    .IBSE_ylevel_p0 = 0,
    .IBSE_ylevel_p112 = 120,
    .IBSE_ylevel_p96 = 112,
    .IBSE_ylevel_p80 = 104,
    .IBSE_ylevel_p64 = 96,
    .IBSE_ylevel_p176 = 112,
    .IBSE_ylevel_p160 = 118,
    .IBSE_ylevel_p144 = 123,
    .IBSE_ylevel_p128 = 128,
    .IBSE_ylevel_p240 = 91,
    .IBSE_ylevel_p224 = 96,
    .IBSE_ylevel_p208 = 102,
    .IBSE_ylevel_p192 = 107,
    .IBSE_ylevel_alpha = 16,
    .IBSE_ylevel_p256 = 86
    },

    {// RszEntryHWReg_2
    .IBSE_gaincontrol_coring_value = 3,
    .IBSE_gaincontrol_coring_thr = 4,
    .IBSE_gaincontrol_coring_zero = 2,
    .IBSE_gaincontrol_softlimit_ratio = 8,
    .IBSE_gaincontrol_bound = 48,
    .IBSE_gaincontrol_limit = 24,
    .IBSE_gaincontrol_softcoring_gain = 8,
    // ylevel_gain
    .IBSE_ylevel_p48 = 72,
    .IBSE_ylevel_p32 = 48,
    .IBSE_ylevel_p16 = 24,
    .IBSE_ylevel_p0 = 0,
    .IBSE_ylevel_p112 = 120,
    .IBSE_ylevel_p96 = 112,
    .IBSE_ylevel_p80 = 104,
    .IBSE_ylevel_p64 = 96,
    .IBSE_ylevel_p176 = 112,
    .IBSE_ylevel_p160 = 118,
    .IBSE_ylevel_p144 = 123,
    .IBSE_ylevel_p128 = 128,
    .IBSE_ylevel_p240 = 91,
    .IBSE_ylevel_p224 = 96,
    .IBSE_ylevel_p208 = 102,
    .IBSE_ylevel_p192 = 107,
    .IBSE_ylevel_alpha = 16,
    .IBSE_ylevel_p256 = 86
    },

    {// RszEntryHWReg_3
    .IBSE_gaincontrol_coring_value = 3,
    .IBSE_gaincontrol_coring_thr = 4,
    .IBSE_gaincontrol_coring_zero = 2,
    .IBSE_gaincontrol_softlimit_ratio = 8,
    .IBSE_gaincontrol_bound = 48,
    .IBSE_gaincontrol_limit = 24,
    .IBSE_gaincontrol_softcoring_gain = 8,
    // ylevel_gain
    .IBSE_ylevel_p48 = 72,
    .IBSE_ylevel_p32 = 48,
    .IBSE_ylevel_p16 = 24,
    .IBSE_ylevel_p0 = 0,
    .IBSE_ylevel_p112 = 120,
    .IBSE_ylevel_p96 = 112,
    .IBSE_ylevel_p80 = 104,
    .IBSE_ylevel_p64 = 96,
    .IBSE_ylevel_p176 = 112,
    .IBSE_ylevel_p160 = 118,
    .IBSE_ylevel_p144 = 123,
    .IBSE_ylevel_p128 = 128,
    .IBSE_ylevel_p240 = 91,
    .IBSE_ylevel_p224 = 96,
    .IBSE_ylevel_p208 = 102,
    .IBSE_ylevel_p192 = 107,
    .IBSE_ylevel_alpha = 16,
    .IBSE_ylevel_p256 = 86
    },

    {// RszEntryHWReg_4
    .IBSE_gaincontrol_coring_value = 3,
    .IBSE_gaincontrol_coring_thr = 4,
    .IBSE_gaincontrol_coring_zero = 2,
    .IBSE_gaincontrol_softlimit_ratio = 8,
    .IBSE_gaincontrol_bound = 48,
    .IBSE_gaincontrol_limit = 24,
    .IBSE_gaincontrol_softcoring_gain = 8,
    // ylevel_gain
    .IBSE_ylevel_p48 = 72,
    .IBSE_ylevel_p32 = 48,
    .IBSE_ylevel_p16 = 24,
    .IBSE_ylevel_p0 = 0,
    .IBSE_ylevel_p112 = 120,
    .IBSE_ylevel_p96 = 112,
    .IBSE_ylevel_p80 = 104,
    .IBSE_ylevel_p64 = 96,
    .IBSE_ylevel_p176 = 112,
    .IBSE_ylevel_p160 = 118,
    .IBSE_ylevel_p144 = 123,
    .IBSE_ylevel_p128 = 128,
    .IBSE_ylevel_p240 = 91,
    .IBSE_ylevel_p224 = 96,
    .IBSE_ylevel_p208 = 102,
    .IBSE_ylevel_p192 = 107,
    .IBSE_ylevel_alpha = 16,
    .IBSE_ylevel_p256 = 86
    },

    {// RszEntryHWReg_5
    .IBSE_gaincontrol_coring_value = 3,
    .IBSE_gaincontrol_coring_thr = 4,
    .IBSE_gaincontrol_coring_zero = 2,
    .IBSE_gaincontrol_softlimit_ratio = 8,
    .IBSE_gaincontrol_bound = 48,
    .IBSE_gaincontrol_limit = 24,
    .IBSE_gaincontrol_softcoring_gain = 8,
    // ylevel_gain
    .IBSE_ylevel_p48 = 72,
    .IBSE_ylevel_p32 = 48,
    .IBSE_ylevel_p16 = 24,
    .IBSE_ylevel_p0 = 0,
    .IBSE_ylevel_p112 = 120,
    .IBSE_ylevel_p96 = 112,
    .IBSE_ylevel_p80 = 104,
    .IBSE_ylevel_p64 = 96,
    .IBSE_ylevel_p176 = 112,
    .IBSE_ylevel_p160 = 118,
    .IBSE_ylevel_p144 = 123,
    .IBSE_ylevel_p128 = 128,
    .IBSE_ylevel_p240 = 91,
    .IBSE_ylevel_p224 = 96,
    .IBSE_ylevel_p208 = 102,
    .IBSE_ylevel_p192 = 107,
    .IBSE_ylevel_alpha = 16,
    .IBSE_ylevel_p256 = 86
    },

    {// RszEntryHWReg_6
    .IBSE_gaincontrol_coring_value = 3,
    .IBSE_gaincontrol_coring_thr = 4,
    .IBSE_gaincontrol_coring_zero = 2,
    .IBSE_gaincontrol_softlimit_ratio = 8,
    .IBSE_gaincontrol_bound = 48,
    .IBSE_gaincontrol_limit = 24,
    .IBSE_gaincontrol_softcoring_gain = 8,
    // ylevel_gain
    .IBSE_ylevel_p48 = 72,
    .IBSE_ylevel_p32 = 48,
    .IBSE_ylevel_p16 = 24,
    .IBSE_ylevel_p0 = 0,
    .IBSE_ylevel_p112 = 120,
    .IBSE_ylevel_p96 = 112,
    .IBSE_ylevel_p80 = 104,
    .IBSE_ylevel_p64 = 96,
    .IBSE_ylevel_p176 = 112,
    .IBSE_ylevel_p160 = 118,
    .IBSE_ylevel_p144 = 123,
    .IBSE_ylevel_p128 = 128,
    .IBSE_ylevel_p240 = 91,
    .IBSE_ylevel_p224 = 96,
    .IBSE_ylevel_p208 = 102,
    .IBSE_ylevel_p192 = 107,
    .IBSE_ylevel_alpha = 16,
    .IBSE_ylevel_p256 = 86
    },

    {// RszEntryHWReg_7
    .IBSE_gaincontrol_coring_value = 3,
    .IBSE_gaincontrol_coring_thr = 4,
    .IBSE_gaincontrol_coring_zero = 2,
    .IBSE_gaincontrol_softlimit_ratio = 8,
    .IBSE_gaincontrol_bound = 48,
    .IBSE_gaincontrol_limit = 24,
    .IBSE_gaincontrol_softcoring_gain = 8,
    // ylevel_gain
    .IBSE_ylevel_p48 = 72,
    .IBSE_ylevel_p32 = 48,
    .IBSE_ylevel_p16 = 24,
    .IBSE_ylevel_p0 = 0,
    .IBSE_ylevel_p112 = 120,
    .IBSE_ylevel_p96 = 112,
    .IBSE_ylevel_p80 = 104,
    .IBSE_ylevel_p64 = 96,
    .IBSE_ylevel_p176 = 112,
    .IBSE_ylevel_p160 = 118,
    .IBSE_ylevel_p144 = 123,
    .IBSE_ylevel_p128 = 128,
    .IBSE_ylevel_p240 = 91,
    .IBSE_ylevel_p224 = 96,
    .IBSE_ylevel_p208 = 102,
    .IBSE_ylevel_p192 = 107,
    .IBSE_ylevel_alpha = 16,
    .IBSE_ylevel_p256 = 86
    },

    {// RszEntryHWReg_8
    .IBSE_gaincontrol_coring_value = 3,
    .IBSE_gaincontrol_coring_thr = 4,
    .IBSE_gaincontrol_coring_zero = 2,
    .IBSE_gaincontrol_softlimit_ratio = 8,
    .IBSE_gaincontrol_bound = 48,
    .IBSE_gaincontrol_limit = 24,
    .IBSE_gaincontrol_softcoring_gain = 8,
    // ylevel_gain
    .IBSE_ylevel_p48 = 72,
    .IBSE_ylevel_p32 = 48,
    .IBSE_ylevel_p16 = 24,
    .IBSE_ylevel_p0 = 0,
    .IBSE_ylevel_p112 = 120,
    .IBSE_ylevel_p96 = 112,
    .IBSE_ylevel_p80 = 104,
    .IBSE_ylevel_p64 = 96,
    .IBSE_ylevel_p176 = 112,
    .IBSE_ylevel_p160 = 118,
    .IBSE_ylevel_p144 = 123,
    .IBSE_ylevel_p128 = 128,
    .IBSE_ylevel_p240 = 91,
    .IBSE_ylevel_p224 = 96,
    .IBSE_ylevel_p208 = 102,
    .IBSE_ylevel_p192 = 107,
    .IBSE_ylevel_alpha = 16,
    .IBSE_ylevel_p256 = 86
    },

    {// RszEntryHWReg_9
    .IBSE_gaincontrol_coring_value = 3,
    .IBSE_gaincontrol_coring_thr = 4,
    .IBSE_gaincontrol_coring_zero = 2,
    .IBSE_gaincontrol_softlimit_ratio = 8,
    .IBSE_gaincontrol_bound = 48,
    .IBSE_gaincontrol_limit = 24,
    .IBSE_gaincontrol_softcoring_gain = 8,
    // ylevel_gain
    .IBSE_ylevel_p48 = 72,
    .IBSE_ylevel_p32 = 48,
    .IBSE_ylevel_p16 = 24,
    .IBSE_ylevel_p0 = 0,
    .IBSE_ylevel_p112 = 120,
    .IBSE_ylevel_p96 = 112,
    .IBSE_ylevel_p80 = 104,
    .IBSE_ylevel_p64 = 96,
    .IBSE_ylevel_p176 = 112,
    .IBSE_ylevel_p160 = 118,
    .IBSE_ylevel_p144 = 123,
    .IBSE_ylevel_p128 = 128,
    .IBSE_ylevel_p240 = 91,
    .IBSE_ylevel_p224 = 96,
    .IBSE_ylevel_p208 = 102,
    .IBSE_ylevel_p192 = 107,
    .IBSE_ylevel_alpha = 16,
    .IBSE_ylevel_p256 = 86
    },

    {// RszEntryHWReg_10
    .IBSE_gaincontrol_coring_value = 3,
    .IBSE_gaincontrol_coring_thr = 4,
    .IBSE_gaincontrol_coring_zero = 2,
    .IBSE_gaincontrol_softlimit_ratio = 8,
    .IBSE_gaincontrol_bound = 48,
    .IBSE_gaincontrol_limit = 24,
    .IBSE_gaincontrol_softcoring_gain = 8,
    // ylevel_gain
    .IBSE_ylevel_p48 = 72,
    .IBSE_ylevel_p32 = 48,
    .IBSE_ylevel_p16 = 24,
    .IBSE_ylevel_p0 = 0,
    .IBSE_ylevel_p112 = 120,
    .IBSE_ylevel_p96 = 112,
    .IBSE_ylevel_p80 = 104,
    .IBSE_ylevel_p64 = 96,
    .IBSE_ylevel_p176 = 112,
    .IBSE_ylevel_p160 = 118,
    .IBSE_ylevel_p144 = 123,
    .IBSE_ylevel_p128 = 128,
    .IBSE_ylevel_p240 = 91,
    .IBSE_ylevel_p224 = 96,
    .IBSE_ylevel_p208 = 102,
    .IBSE_ylevel_p192 = 107,
    .IBSE_ylevel_alpha = 16,
    .IBSE_ylevel_p256 = 86
    }
};

DSReg iDSRegEntry[UR_MAX_LEVEL] =
{
    {// iDSRegEntry_0
    .DS_en = 1,

    .iUpSlope = 0,          // Range from -64 to 63
    .iUpThreshold = 1024,    // Range from 0 to 4095
    .iDownSlope = 0,        // Range from -64 to 63
    .iDownThreshold = 1024,  // Range from 0 to 4095

    //ISO adaptive SW registers
    .iISO_en   = 1,
    .iISO_thr0 = 200,
    .iISO_thr1 = 400,
    .iISO_thr2 = 800,
    .iISO_thr3 = 1600,
    .iISO_IIR_alpha = 128,

    .iCorZero_clip2 =   0,
    .iCorZero_clip1 =   0,
    .iCorZero_clip0 =   0,
    .iCorThr_clip2  =   0,
    .iCorThr_clip1  =   0,
    .iCorThr_clip0  =   0,
    .iCorGain_clip2 =   0,
    .iCorGain_clip1 =   0,
    .iCorGain_clip0 =   0,
    .iGain_clip2    =   0,
    .iGain_clip1    =   0,
    .iGain_clip0    =   0,

#ifndef DS_BYPASS_2DGC
    .iHighCorZero_clip2 =   0,
    .iHighCorZero_clip1 =   0,
    .iHighCorZero_clip0 =   0,
    .iHighCorThr_clip2  =   0,
    .iHighCorThr_clip1  =   0,
    .iHighCorThr_clip0  =   0,
    .iHighCorGain_clip2 =   0,
    .iHighCorGain_clip1 =   0,
    .iHighCorGain_clip0 =   0,

    .iMidCorZero_clip2 =  0,
    .iMidCorZero_clip1 =  0,
    .iMidCorZero_clip0 =  0,
    .iMidCorThr_clip2  =  0,
    .iMidCorThr_clip1  =  0,
    .iMidCorThr_clip0  =  0,
    .iMidCorGain_clip2 =  0,
    .iMidCorGain_clip1 =  0,
    .iMidCorGain_clip0 =  0,
#endif //DS_BYPASS_2DGC

    //UR SW registers
    .iUltraRes_en = 1,
    .iUltraRes_ratio_thr0 = 1024,
    .iUltraRes_ratio_thr1 = 2048,
    .iUltraRes_ratio_thr2 = 4096,
    .iUltraRes_gain_mid_clip0 = 0,
    .iUltraRes_gain_mid_clip1 = 0,
    .iUltraRes_gain_high_clip0 = 0,
    .iUltraRes_gain_high_clip1 = 0,
#ifndef DS_BYPASS_EDS
    .iUltraRes_edf_edge_gain_clip0 = 0,
    .iUltraRes_edf_edge_gain_clip1 = 0,
    .iUltraRes_edf_detail_gain_clip0 = 0,
    .iUltraRes_edf_detail_gain_clip1 = 0,
    .iUltraRes_edf_flat_gain_clip0 = 0,
    .iUltraRes_edf_flat_gain_clip1 = 0,
#endif //DS_BYPASS_EDS
    .iUltraRes_clip_thr_clip0 = 0,
    .iUltraRes_clip_thr_clip1 = 0,
    .iUltraRes_clip_ratio_clip0 = 0,
    .iUltraRes_clip_ratio_clip1 = 0,

#ifndef DS_BYPASS_EDS
    .i_edf_flat_th_clip2 =  0,
    .i_edf_flat_th_clip1 =  0,
    .i_edf_flat_th_clip0 =  0,
    .i_edf_detail_rise_th_clip2 =  0,
    .i_edf_detail_rise_th_clip1 =  0,
    .i_edf_detail_rise_th_clip0 =  0
#endif //DS_BYPASS_EDS
    },

    {// iDSRegEntry_1
    .DS_en = 1,

    .iUpSlope = -4,          // Range from -64 to 63
    .iUpThreshold = 1024,    // Range from 0 to 4095
    .iDownSlope = -4,        // Range from -64 to 63
    .iDownThreshold = 1024,  // Range from 0 to 4095

    //ISO adaptive SW registers
    .iISO_en   = 1,
    .iISO_thr0 = 200,
    .iISO_thr1 = 400,
    .iISO_thr2 = 800,
    .iISO_thr3 = 1600,
    .iISO_IIR_alpha = 128,

    .iCorZero_clip2 =   4,
    .iCorZero_clip1 =   4,
    .iCorZero_clip0 =  10,
    .iCorThr_clip2  =   6,
    .iCorThr_clip1  =   6,
    .iCorThr_clip0  =  24,
    .iCorGain_clip2 =   0,
    .iCorGain_clip1 =   0,
    .iCorGain_clip0 =  16,
    .iGain_clip2    =   0,
    .iGain_clip1    =   0,
    .iGain_clip0    =   0,

#ifndef DS_BYPASS_2DGC
    .iHighCorZero_clip2 =   3,
    .iHighCorZero_clip1 =   3,
    .iHighCorZero_clip0 =   3,
    .iHighCorThr_clip2  =   4,
    .iHighCorThr_clip1  =   4,
    .iHighCorThr_clip0  =   4,
    .iHighCorGain_clip2 =   0,
    .iHighCorGain_clip1 =   0,
    .iHighCorGain_clip0 =  16,

    .iMidCorZero_clip2 =  3,
    .iMidCorZero_clip1 =  3,
    .iMidCorZero_clip0 =  3,
    .iMidCorThr_clip2  =  4,
    .iMidCorThr_clip1  =  4,
    .iMidCorThr_clip0  =  4,
    .iMidCorGain_clip2 =  0,
    .iMidCorGain_clip1 =  0,
    .iMidCorGain_clip0 = 16,
#endif //DS_BYPASS_2DGC

    //UR SW registers
    .iUltraRes_en = 1,
    .iUltraRes_ratio_thr0 = 1024,
    .iUltraRes_ratio_thr1 = 2048,
    .iUltraRes_ratio_thr2 = 4096,
    .iUltraRes_gain_mid_clip0 = 35,
    .iUltraRes_gain_mid_clip1 = 34,
    .iUltraRes_gain_high_clip0 = 50,
    .iUltraRes_gain_high_clip1 = 51,
#ifndef DS_BYPASS_EDS
    .iUltraRes_edf_edge_gain_clip0 = -2,
    .iUltraRes_edf_edge_gain_clip1 = -1,
    .iUltraRes_edf_detail_gain_clip0 = -2,
    .iUltraRes_edf_detail_gain_clip1 = -1,
    .iUltraRes_edf_flat_gain_clip0 = -2,
    .iUltraRes_edf_flat_gain_clip1 = -1,
#endif //DS_BYPASS_EDS
    .iUltraRes_clip_thr_clip0 = 0,
    .iUltraRes_clip_thr_clip1 = -4,
    .iUltraRes_clip_ratio_clip0 = 0,
    .iUltraRes_clip_ratio_clip1 = -4,

#ifndef DS_BYPASS_EDS
    .i_edf_flat_th_clip2 =  8,
    .i_edf_flat_th_clip1 =  4,
    .i_edf_flat_th_clip0 = 12,
    .i_edf_detail_rise_th_clip2 =  8,
    .i_edf_detail_rise_th_clip1 =  4,
    .i_edf_detail_rise_th_clip0 = 12
#endif //DS_BYPASS_EDS
    },

    {// iDSRegEntry_2
    .DS_en = 1,

    .iUpSlope = -4,          // Range from -64 to 63
    .iUpThreshold = 1024,    // Range from 0 to 4095
    .iDownSlope = -4,        // Range from -64 to 63
    .iDownThreshold = 1024,  // Range from 0 to 4095

    //ISO adaptive SW registers
    .iISO_en   = 1,
    .iISO_thr0 = 200,
    .iISO_thr1 = 400,
    .iISO_thr2 = 800,
    .iISO_thr3 = 1600,
    .iISO_IIR_alpha = 128,

    .iCorZero_clip2 =   4,
    .iCorZero_clip1 =   4,
    .iCorZero_clip0 =  10,
    .iCorThr_clip2  =   6,
    .iCorThr_clip1  =   6,
    .iCorThr_clip0  =  24,
    .iCorGain_clip2 =   0,
    .iCorGain_clip1 =   0,
    .iCorGain_clip0 =  16,
    .iGain_clip2    =   0,
    .iGain_clip1    =   0,
    .iGain_clip0    =   0,

#ifndef DS_BYPASS_2DGC
    .iHighCorZero_clip2 =   3,
    .iHighCorZero_clip1 =   3,
    .iHighCorZero_clip0 =   3,
    .iHighCorThr_clip2  =   4,
    .iHighCorThr_clip1  =   4,
    .iHighCorThr_clip0  =   4,
    .iHighCorGain_clip2 =   0,
    .iHighCorGain_clip1 =   0,
    .iHighCorGain_clip0 =  16,

    .iMidCorZero_clip2 =  3,
    .iMidCorZero_clip1 =  3,
    .iMidCorZero_clip0 =  3,
    .iMidCorThr_clip2  =  4,
    .iMidCorThr_clip1  =  4,
    .iMidCorThr_clip0  =  4,
    .iMidCorGain_clip2 =  0,
    .iMidCorGain_clip1 =  0,
    .iMidCorGain_clip0 = 16,
#endif //DS_BYPASS_2DGC

    //UR SW registers
    .iUltraRes_en = 1,
    .iUltraRes_ratio_thr0 = 1024,
    .iUltraRes_ratio_thr1 = 2048,
    .iUltraRes_ratio_thr2 = 4096,
    .iUltraRes_gain_mid_clip0 = 35,
    .iUltraRes_gain_mid_clip1 = 34,
    .iUltraRes_gain_high_clip0 = 50,
    .iUltraRes_gain_high_clip1 = 51,
#ifndef DS_BYPASS_EDS
    .iUltraRes_edf_edge_gain_clip0 = -2,
    .iUltraRes_edf_edge_gain_clip1 = -1,
    .iUltraRes_edf_detail_gain_clip0 = -2,
    .iUltraRes_edf_detail_gain_clip1 = -1,
    .iUltraRes_edf_flat_gain_clip0 = -2,
    .iUltraRes_edf_flat_gain_clip1 = -1,
#endif //DS_BYPASS_EDS
    .iUltraRes_clip_thr_clip0 = 0,
    .iUltraRes_clip_thr_clip1 = -4,
    .iUltraRes_clip_ratio_clip0 = 0,
    .iUltraRes_clip_ratio_clip1 = -4,

#ifndef DS_BYPASS_EDS
    .i_edf_flat_th_clip2 =  8,
    .i_edf_flat_th_clip1 =  4,
    .i_edf_flat_th_clip0 = 12,
    .i_edf_detail_rise_th_clip2 =  8,
    .i_edf_detail_rise_th_clip1 =  4,
    .i_edf_detail_rise_th_clip0 = 12
#endif //DS_BYPASS_EDS
    },

    {// iDSRegEntry_3
    .DS_en = 1,

    .iUpSlope = -4,          // Range from -64 to 63
    .iUpThreshold = 1024,    // Range from 0 to 4095
    .iDownSlope = -4,        // Range from -64 to 63
    .iDownThreshold = 1024,  // Range from 0 to 4095

    //ISO adaptive SW registers
    .iISO_en   = 1,
    .iISO_thr0 = 200,
    .iISO_thr1 = 400,
    .iISO_thr2 = 800,
    .iISO_thr3 = 1600,
    .iISO_IIR_alpha = 128,

    .iCorZero_clip2 =   4,
    .iCorZero_clip1 =   4,
    .iCorZero_clip0 =  10,
    .iCorThr_clip2  =   6,
    .iCorThr_clip1  =   6,
    .iCorThr_clip0  =  24,
    .iCorGain_clip2 =   0,
    .iCorGain_clip1 =   0,
    .iCorGain_clip0 =  16,
    .iGain_clip2    =   0,
    .iGain_clip1    =   0,
    .iGain_clip0    =   0,

#ifndef DS_BYPASS_2DGC
    .iHighCorZero_clip2 =   3,
    .iHighCorZero_clip1 =   3,
    .iHighCorZero_clip0 =   3,
    .iHighCorThr_clip2  =   4,
    .iHighCorThr_clip1  =   4,
    .iHighCorThr_clip0  =   4,
    .iHighCorGain_clip2 =   0,
    .iHighCorGain_clip1 =   0,
    .iHighCorGain_clip0 =  16,

    .iMidCorZero_clip2 =  3,
    .iMidCorZero_clip1 =  3,
    .iMidCorZero_clip0 =  3,
    .iMidCorThr_clip2  =  4,
    .iMidCorThr_clip1  =  4,
    .iMidCorThr_clip0  =  4,
    .iMidCorGain_clip2 =  0,
    .iMidCorGain_clip1 =  0,
    .iMidCorGain_clip0 = 16,
#endif //DS_BYPASS_2DGC

    //UR SW registers
    .iUltraRes_en = 1,
    .iUltraRes_ratio_thr0 = 1024,
    .iUltraRes_ratio_thr1 = 2048,
    .iUltraRes_ratio_thr2 = 4096,
    .iUltraRes_gain_mid_clip0 = 35,
    .iUltraRes_gain_mid_clip1 = 34,
    .iUltraRes_gain_high_clip0 = 50,
    .iUltraRes_gain_high_clip1 = 51,
#ifndef DS_BYPASS_EDS
    .iUltraRes_edf_edge_gain_clip0 = -2,
    .iUltraRes_edf_edge_gain_clip1 = -1,
    .iUltraRes_edf_detail_gain_clip0 = -2,
    .iUltraRes_edf_detail_gain_clip1 = -1,
    .iUltraRes_edf_flat_gain_clip0 = -2,
    .iUltraRes_edf_flat_gain_clip1 = -1,
#endif //DS_BYPASS_EDS
    .iUltraRes_clip_thr_clip0 = 0,
    .iUltraRes_clip_thr_clip1 = -4,
    .iUltraRes_clip_ratio_clip0 = 0,
    .iUltraRes_clip_ratio_clip1 = -4,

#ifndef DS_BYPASS_EDS
    .i_edf_flat_th_clip2 =  8,
    .i_edf_flat_th_clip1 =  4,
    .i_edf_flat_th_clip0 = 12,
    .i_edf_detail_rise_th_clip2 =  8,
    .i_edf_detail_rise_th_clip1 =  4,
    .i_edf_detail_rise_th_clip0 = 12
#endif //DS_BYPASS_EDS
    },

    {// iDSRegEntry_4
    .DS_en = 1,

    .iUpSlope = -4,          // Range from -64 to 63
    .iUpThreshold = 1024,    // Range from 0 to 4095
    .iDownSlope = -4,        // Range from -64 to 63
    .iDownThreshold = 1024,  // Range from 0 to 4095

    //ISO adaptive SW registers
    .iISO_en   = 1,
    .iISO_thr0 = 200,
    .iISO_thr1 = 400,
    .iISO_thr2 = 800,
    .iISO_thr3 = 1600,
    .iISO_IIR_alpha = 128,

    .iCorZero_clip2 =   4,
    .iCorZero_clip1 =   4,
    .iCorZero_clip0 =  10,
    .iCorThr_clip2  =   6,
    .iCorThr_clip1  =   6,
    .iCorThr_clip0  =  24,
    .iCorGain_clip2 =   0,
    .iCorGain_clip1 =   0,
    .iCorGain_clip0 =  16,
    .iGain_clip2    =   0,
    .iGain_clip1    =   0,
    .iGain_clip0    =   0,

#ifndef DS_BYPASS_2DGC
    .iHighCorZero_clip2 =   3,
    .iHighCorZero_clip1 =   3,
    .iHighCorZero_clip0 =   3,
    .iHighCorThr_clip2  =   4,
    .iHighCorThr_clip1  =   4,
    .iHighCorThr_clip0  =   4,
    .iHighCorGain_clip2 =   0,
    .iHighCorGain_clip1 =   0,
    .iHighCorGain_clip0 =  16,

    .iMidCorZero_clip2 =  3,
    .iMidCorZero_clip1 =  3,
    .iMidCorZero_clip0 =  3,
    .iMidCorThr_clip2  =  4,
    .iMidCorThr_clip1  =  4,
    .iMidCorThr_clip0  =  4,
    .iMidCorGain_clip2 =  0,
    .iMidCorGain_clip1 =  0,
    .iMidCorGain_clip0 = 16,
#endif //DS_BYPASS_2DGC

    //UR SW registers
    .iUltraRes_en = 1,
    .iUltraRes_ratio_thr0 = 1024,
    .iUltraRes_ratio_thr1 = 2048,
    .iUltraRes_ratio_thr2 = 4096,
    .iUltraRes_gain_mid_clip0 = 40,
    .iUltraRes_gain_mid_clip1 = 34,
    .iUltraRes_gain_high_clip0 = 50,
    .iUltraRes_gain_high_clip1 = 51,
#ifndef DS_BYPASS_EDS
    .iUltraRes_edf_edge_gain_clip0 = -2,
    .iUltraRes_edf_edge_gain_clip1 = -1,
    .iUltraRes_edf_detail_gain_clip0 = -2,
    .iUltraRes_edf_detail_gain_clip1 = -1,
    .iUltraRes_edf_flat_gain_clip0 = -2,
    .iUltraRes_edf_flat_gain_clip1 = -1,
#endif //DS_BYPASS_EDS
    .iUltraRes_clip_thr_clip0 = 0,
    .iUltraRes_clip_thr_clip1 = -4,
    .iUltraRes_clip_ratio_clip0 = 0,
    .iUltraRes_clip_ratio_clip1 = -4,

#ifndef DS_BYPASS_EDS
    .i_edf_flat_th_clip2 =  8,
    .i_edf_flat_th_clip1 =  4,
    .i_edf_flat_th_clip0 = 12,
    .i_edf_detail_rise_th_clip2 =  8,
    .i_edf_detail_rise_th_clip1 =  4,
    .i_edf_detail_rise_th_clip0 = 12
#endif //DS_BYPASS_EDS
    },

    {// iDSRegEntry_5
    .DS_en = 1,

    .iUpSlope = -4,          // Range from -64 to 63
    .iUpThreshold = 1024,    // Range from 0 to 4095
    .iDownSlope = -4,        // Range from -64 to 63
    .iDownThreshold = 1024,  // Range from 0 to 4095

    //ISO adaptive SW registers
    .iISO_en   = 1,
    .iISO_thr0 = 200,
    .iISO_thr1 = 400,
    .iISO_thr2 = 800,
    .iISO_thr3 = 1600,
    .iISO_IIR_alpha = 128,

    .iCorZero_clip2 =   4,
    .iCorZero_clip1 =   4,
    .iCorZero_clip0 =  10,
    .iCorThr_clip2  =   6,
    .iCorThr_clip1  =   6,
    .iCorThr_clip0  =  24,
    .iCorGain_clip2 =   0,
    .iCorGain_clip1 =   0,
    .iCorGain_clip0 =  16,
    .iGain_clip2    =   0,
    .iGain_clip1    =   0,
    .iGain_clip0    =   0,

#ifndef DS_BYPASS_2DGC
    .iHighCorZero_clip2 =   3,
    .iHighCorZero_clip1 =   3,
    .iHighCorZero_clip0 =   3,
    .iHighCorThr_clip2  =   4,
    .iHighCorThr_clip1  =   4,
    .iHighCorThr_clip0  =   4,
    .iHighCorGain_clip2 =   0,
    .iHighCorGain_clip1 =   0,
    .iHighCorGain_clip0 =  16,

    .iMidCorZero_clip2 =  3,
    .iMidCorZero_clip1 =  3,
    .iMidCorZero_clip0 =  3,
    .iMidCorThr_clip2  =  4,
    .iMidCorThr_clip1  =  4,
    .iMidCorThr_clip0  =  4,
    .iMidCorGain_clip2 =  0,
    .iMidCorGain_clip1 =  0,
    .iMidCorGain_clip0 = 16,
#endif //DS_BYPASS_2DGC

    //UR SW registers
    .iUltraRes_en = 1,
    .iUltraRes_ratio_thr0 = 1024,
    .iUltraRes_ratio_thr1 = 2048,
    .iUltraRes_ratio_thr2 = 4096,
    .iUltraRes_gain_mid_clip0 = 35,
    .iUltraRes_gain_mid_clip1 = 34,
    .iUltraRes_gain_high_clip0 = 50,
    .iUltraRes_gain_high_clip1 = 51,
#ifndef DS_BYPASS_EDS
    .iUltraRes_edf_edge_gain_clip0 = -2,
    .iUltraRes_edf_edge_gain_clip1 = -1,
    .iUltraRes_edf_detail_gain_clip0 = -2,
    .iUltraRes_edf_detail_gain_clip1 = -1,
    .iUltraRes_edf_flat_gain_clip0 = -2,
    .iUltraRes_edf_flat_gain_clip1 = -1,
#endif //DS_BYPASS_EDS
    .iUltraRes_clip_thr_clip0 = 0,
    .iUltraRes_clip_thr_clip1 = -4,
    .iUltraRes_clip_ratio_clip0 = 0,
    .iUltraRes_clip_ratio_clip1 = -4,

#ifndef DS_BYPASS_EDS
    .i_edf_flat_th_clip2 =  8,
    .i_edf_flat_th_clip1 =  4,
    .i_edf_flat_th_clip0 = 12,
    .i_edf_detail_rise_th_clip2 =  8,
    .i_edf_detail_rise_th_clip1 =  4,
    .i_edf_detail_rise_th_clip0 = 12
#endif //DS_BYPASS_EDS
    },

    {// iDSRegEntry_6
    .DS_en = 1,

    .iUpSlope = -4,          // Range from -64 to 63
    .iUpThreshold = 1024,    // Range from 0 to 4095
    .iDownSlope = -4,        // Range from -64 to 63
    .iDownThreshold = 1024,  // Range from 0 to 4095

    //ISO adaptive SW registers
    .iISO_en   = 1,
    .iISO_thr0 = 200,
    .iISO_thr1 = 400,
    .iISO_thr2 = 800,
    .iISO_thr3 = 1600,
    .iISO_IIR_alpha = 128,

    .iCorZero_clip2 =   4,
    .iCorZero_clip1 =   4,
    .iCorZero_clip0 =  10,
    .iCorThr_clip2  =   6,
    .iCorThr_clip1  =   6,
    .iCorThr_clip0  =  24,
    .iCorGain_clip2 =   0,
    .iCorGain_clip1 =   0,
    .iCorGain_clip0 =  16,
    .iGain_clip2    =   0,
    .iGain_clip1    =   0,
    .iGain_clip0    =   0,

#ifndef DS_BYPASS_2DGC
    .iHighCorZero_clip2 =   3,
    .iHighCorZero_clip1 =   3,
    .iHighCorZero_clip0 =   3,
    .iHighCorThr_clip2  =   4,
    .iHighCorThr_clip1  =   4,
    .iHighCorThr_clip0  =   4,
    .iHighCorGain_clip2 =   0,
    .iHighCorGain_clip1 =   0,
    .iHighCorGain_clip0 =  16,

    .iMidCorZero_clip2 =  3,
    .iMidCorZero_clip1 =  3,
    .iMidCorZero_clip0 =  3,
    .iMidCorThr_clip2  =  4,
    .iMidCorThr_clip1  =  4,
    .iMidCorThr_clip0  =  4,
    .iMidCorGain_clip2 =  0,
    .iMidCorGain_clip1 =  0,
    .iMidCorGain_clip0 = 16,
#endif //DS_BYPASS_2DGC

    //UR SW registers
    .iUltraRes_en = 1,
    .iUltraRes_ratio_thr0 = 1024,
    .iUltraRes_ratio_thr1 = 2048,
    .iUltraRes_ratio_thr2 = 4096,
    .iUltraRes_gain_mid_clip0 = 10,
    .iUltraRes_gain_mid_clip1 = 10,
    .iUltraRes_gain_high_clip0 = 5,
    .iUltraRes_gain_high_clip1 = 5,
#ifndef DS_BYPASS_EDS
    .iUltraRes_edf_edge_gain_clip0 = -8,
    .iUltraRes_edf_edge_gain_clip1 = -8,
    .iUltraRes_edf_detail_gain_clip0 = -2,
    .iUltraRes_edf_detail_gain_clip1 = -1,
    .iUltraRes_edf_flat_gain_clip0 = 0,
    .iUltraRes_edf_flat_gain_clip1 = 0,
#endif //DS_BYPASS_EDS
    .iUltraRes_clip_thr_clip0 = 0,
    .iUltraRes_clip_thr_clip1 = -4,
    .iUltraRes_clip_ratio_clip0 = 0,
    .iUltraRes_clip_ratio_clip1 = -4,

#ifndef DS_BYPASS_EDS
    .i_edf_flat_th_clip2 =  8,
    .i_edf_flat_th_clip1 =  4,
    .i_edf_flat_th_clip0 = 12,
    .i_edf_detail_rise_th_clip2 =  8,
    .i_edf_detail_rise_th_clip1 =  4,
    .i_edf_detail_rise_th_clip0 = 12
#endif //DS_BYPASS_EDS
    },

    {// iDSRegEntry_7
    .DS_en = 1,

    .iUpSlope = -4,          // Range from -64 to 63
    .iUpThreshold = 1024,    // Range from 0 to 4095
    .iDownSlope = -4,        // Range from -64 to 63
    .iDownThreshold = 1024,  // Range from 0 to 4095

    //ISO adaptive SW registers
    .iISO_en   = 1,
    .iISO_thr0 = 200,
    .iISO_thr1 = 400,
    .iISO_thr2 = 800,
    .iISO_thr3 = 1600,
    .iISO_IIR_alpha = 128,

    .iCorZero_clip2 =   4,
    .iCorZero_clip1 =   4,
    .iCorZero_clip0 =  10,
    .iCorThr_clip2  =   6,
    .iCorThr_clip1  =   6,
    .iCorThr_clip0  =  24,
    .iCorGain_clip2 =   0,
    .iCorGain_clip1 =   0,
    .iCorGain_clip0 =  16,
    .iGain_clip2    =   0,
    .iGain_clip1    =   0,
    .iGain_clip0    =   0,

#ifndef DS_BYPASS_2DGC
    .iHighCorZero_clip2 =   3,
    .iHighCorZero_clip1 =   3,
    .iHighCorZero_clip0 =   3,
    .iHighCorThr_clip2  =   4,
    .iHighCorThr_clip1  =   4,
    .iHighCorThr_clip0  =   4,
    .iHighCorGain_clip2 =   0,
    .iHighCorGain_clip1 =   0,
    .iHighCorGain_clip0 =  16,

    .iMidCorZero_clip2 =  3,
    .iMidCorZero_clip1 =  3,
    .iMidCorZero_clip0 =  3,
    .iMidCorThr_clip2  =  4,
    .iMidCorThr_clip1  =  4,
    .iMidCorThr_clip0  =  4,
    .iMidCorGain_clip2 =  0,
    .iMidCorGain_clip1 =  0,
    .iMidCorGain_clip0 = 16,
#endif //DS_BYPASS_2DGC

    //UR SW registers
    .iUltraRes_en = 1,
    .iUltraRes_ratio_thr0 = 1024,
    .iUltraRes_ratio_thr1 = 2048,
    .iUltraRes_ratio_thr2 = 4096,
    .iUltraRes_gain_mid_clip0 = 70,
    .iUltraRes_gain_mid_clip1 = 34,
    .iUltraRes_gain_high_clip0 = 50,
    .iUltraRes_gain_high_clip1 = 51,
#ifndef DS_BYPASS_EDS
    .iUltraRes_edf_edge_gain_clip0 = -2,
    .iUltraRes_edf_edge_gain_clip1 = -1,
    .iUltraRes_edf_detail_gain_clip0 = -2,
    .iUltraRes_edf_detail_gain_clip1 = -1,
    .iUltraRes_edf_flat_gain_clip0 = -2,
    .iUltraRes_edf_flat_gain_clip1 = -1,
#endif //DS_BYPASS_EDS
    .iUltraRes_clip_thr_clip0 = 0,
    .iUltraRes_clip_thr_clip1 = -4,
    .iUltraRes_clip_ratio_clip0 = 0,
    .iUltraRes_clip_ratio_clip1 = -4,

#ifndef DS_BYPASS_EDS
    .i_edf_flat_th_clip2 =  8,
    .i_edf_flat_th_clip1 =  4,
    .i_edf_flat_th_clip0 = 12,
    .i_edf_detail_rise_th_clip2 =  8,
    .i_edf_detail_rise_th_clip1 =  4,
    .i_edf_detail_rise_th_clip0 = 12
#endif //DS_BYPASS_EDS
    },

    {// iDSRegEntry_8
    .DS_en = 1,

    .iUpSlope = -4,          // Range from -64 to 63
    .iUpThreshold = 1024,    // Range from 0 to 4095
    .iDownSlope = -4,        // Range from -64 to 63
    .iDownThreshold = 1024,  // Range from 0 to 4095

    //ISO adaptive SW registers
    .iISO_en   = 1,
    .iISO_thr0 = 200,
    .iISO_thr1 = 400,
    .iISO_thr2 = 800,
    .iISO_thr3 = 1600,
    .iISO_IIR_alpha = 128,

    .iCorZero_clip2 =   4,
    .iCorZero_clip1 =   4,
    .iCorZero_clip0 =  10,
    .iCorThr_clip2  =   6,
    .iCorThr_clip1  =   6,
    .iCorThr_clip0  =  24,
    .iCorGain_clip2 =   0,
    .iCorGain_clip1 =   0,
    .iCorGain_clip0 =  16,
    .iGain_clip2    =   0,
    .iGain_clip1    =   0,
    .iGain_clip0    =   0,

#ifndef DS_BYPASS_2DGC
    .iHighCorZero_clip2 =   3,
    .iHighCorZero_clip1 =   3,
    .iHighCorZero_clip0 =   3,
    .iHighCorThr_clip2  =   4,
    .iHighCorThr_clip1  =   4,
    .iHighCorThr_clip0  =   4,
    .iHighCorGain_clip2 =   0,
    .iHighCorGain_clip1 =   0,
    .iHighCorGain_clip0 =  16,

    .iMidCorZero_clip2 =  3,
    .iMidCorZero_clip1 =  3,
    .iMidCorZero_clip0 =  3,
    .iMidCorThr_clip2  =  4,
    .iMidCorThr_clip1  =  4,
    .iMidCorThr_clip0  =  4,
    .iMidCorGain_clip2 =  0,
    .iMidCorGain_clip1 =  0,
    .iMidCorGain_clip0 = 16,
#endif //DS_BYPASS_2DGC

    //UR SW registers
    .iUltraRes_en = 1,
    .iUltraRes_ratio_thr0 = 1024,
    .iUltraRes_ratio_thr1 = 2048,
    .iUltraRes_ratio_thr2 = 4096,
    .iUltraRes_gain_mid_clip0 = 80,
    .iUltraRes_gain_mid_clip1 = 34,
    .iUltraRes_gain_high_clip0 = 50,
    .iUltraRes_gain_high_clip1 = 51,
#ifndef DS_BYPASS_EDS
    .iUltraRes_edf_edge_gain_clip0 = -2,
    .iUltraRes_edf_edge_gain_clip1 = -1,
    .iUltraRes_edf_detail_gain_clip0 = -2,
    .iUltraRes_edf_detail_gain_clip1 = -1,
    .iUltraRes_edf_flat_gain_clip0 = -2,
    .iUltraRes_edf_flat_gain_clip1 = -1,
#endif //DS_BYPASS_EDS
    .iUltraRes_clip_thr_clip0 = 0,
    .iUltraRes_clip_thr_clip1 = -4,
    .iUltraRes_clip_ratio_clip0 = 0,
    .iUltraRes_clip_ratio_clip1 = -4,

#ifndef DS_BYPASS_EDS
    .i_edf_flat_th_clip2 =  8,
    .i_edf_flat_th_clip1 =  4,
    .i_edf_flat_th_clip0 = 12,
    .i_edf_detail_rise_th_clip2 =  8,
    .i_edf_detail_rise_th_clip1 =  4,
    .i_edf_detail_rise_th_clip0 = 12
#endif //DS_BYPASS_EDS
    },

    {// iDSRegEntry_9
    .DS_en = 1,

    .iUpSlope = -4,          // Range from -64 to 63
    .iUpThreshold = 1024,    // Range from 0 to 4095
    .iDownSlope = -4,        // Range from -64 to 63
    .iDownThreshold = 1024,  // Range from 0 to 4095

    //ISO adaptive SW registers
    .iISO_en   = 1,
    .iISO_thr0 = 200,
    .iISO_thr1 = 400,
    .iISO_thr2 = 800,
    .iISO_thr3 = 1600,
    .iISO_IIR_alpha = 128,

    .iCorZero_clip2 =   4,
    .iCorZero_clip1 =   4,
    .iCorZero_clip0 =  10,
    .iCorThr_clip2  =   6,
    .iCorThr_clip1  =   6,
    .iCorThr_clip0  =  24,
    .iCorGain_clip2 =   0,
    .iCorGain_clip1 =   0,
    .iCorGain_clip0 =  16,
    .iGain_clip2    =   0,
    .iGain_clip1    =   0,
    .iGain_clip0    =   0,

#ifndef DS_BYPASS_2DGC
    .iHighCorZero_clip2 =   3,
    .iHighCorZero_clip1 =   3,
    .iHighCorZero_clip0 =   3,
    .iHighCorThr_clip2  =   4,
    .iHighCorThr_clip1  =   4,
    .iHighCorThr_clip0  =   4,
    .iHighCorGain_clip2 =   0,
    .iHighCorGain_clip1 =   0,
    .iHighCorGain_clip0 =  16,

    .iMidCorZero_clip2 =  3,
    .iMidCorZero_clip1 =  3,
    .iMidCorZero_clip0 =  3,
    .iMidCorThr_clip2  =  4,
    .iMidCorThr_clip1  =  4,
    .iMidCorThr_clip0  =  4,
    .iMidCorGain_clip2 =  0,
    .iMidCorGain_clip1 =  0,
    .iMidCorGain_clip0 = 16,
#endif //DS_BYPASS_2DGC

    //UR SW registers
    .iUltraRes_en = 1,
    .iUltraRes_ratio_thr0 = 1024,
    .iUltraRes_ratio_thr1 = 2048,
    .iUltraRes_ratio_thr2 = 4096,
    .iUltraRes_gain_mid_clip0 = 90,
    .iUltraRes_gain_mid_clip1 = 34,
    .iUltraRes_gain_high_clip0 = 50,
    .iUltraRes_gain_high_clip1 = 51,
#ifndef DS_BYPASS_EDS
    .iUltraRes_edf_edge_gain_clip0 = -2,
    .iUltraRes_edf_edge_gain_clip1 = -1,
    .iUltraRes_edf_detail_gain_clip0 = -2,
    .iUltraRes_edf_detail_gain_clip1 = -1,
    .iUltraRes_edf_flat_gain_clip0 = -2,
    .iUltraRes_edf_flat_gain_clip1 = -1,
#endif //DS_BYPASS_EDS
    .iUltraRes_clip_thr_clip0 = 0,
    .iUltraRes_clip_thr_clip1 = -4,
    .iUltraRes_clip_ratio_clip0 = 0,
    .iUltraRes_clip_ratio_clip1 = -4,

#ifndef DS_BYPASS_EDS
    .i_edf_flat_th_clip2 =  8,
    .i_edf_flat_th_clip1 =  4,
    .i_edf_flat_th_clip0 = 12,
    .i_edf_detail_rise_th_clip2 =  8,
    .i_edf_detail_rise_th_clip1 =  4,
    .i_edf_detail_rise_th_clip0 = 12
#endif //DS_BYPASS_EDS
    },
    {// iDSRegEntry_10
    .DS_en = 1,

    .iUpSlope = -4,          // Range from -64 to 63
    .iUpThreshold = 1024,    // Range from 0 to 4095
    .iDownSlope = -4,        // Range from -64 to 63
    .iDownThreshold = 1024,  // Range from 0 to 4095

    //ISO adaptive SW registers
    .iISO_en   = 1,
    .iISO_thr0 = 200,
    .iISO_thr1 = 400,
    .iISO_thr2 = 800,
    .iISO_thr3 = 1600,
    .iISO_IIR_alpha = 128,

    .iCorZero_clip2 =   4,
    .iCorZero_clip1 =   4,
    .iCorZero_clip0 =  10,
    .iCorThr_clip2  =   6,
    .iCorThr_clip1  =   6,
    .iCorThr_clip0  =  24,
    .iCorGain_clip2 =   0,
    .iCorGain_clip1 =   0,
    .iCorGain_clip0 =  16,
    .iGain_clip2    =   0,
    .iGain_clip1    =   0,
    .iGain_clip0    =   0,

#ifndef DS_BYPASS_2DGC
    .iHighCorZero_clip2 =   3,
    .iHighCorZero_clip1 =   3,
    .iHighCorZero_clip0 =   3,
    .iHighCorThr_clip2  =   4,
    .iHighCorThr_clip1  =   4,
    .iHighCorThr_clip0  =   4,
    .iHighCorGain_clip2 =   0,
    .iHighCorGain_clip1 =   0,
    .iHighCorGain_clip0 =  16,

    .iMidCorZero_clip2 =  3,
    .iMidCorZero_clip1 =  3,
    .iMidCorZero_clip0 =  3,
    .iMidCorThr_clip2  =  4,
    .iMidCorThr_clip1  =  4,
    .iMidCorThr_clip0  =  4,
    .iMidCorGain_clip2 =  0,
    .iMidCorGain_clip1 =  0,
    .iMidCorGain_clip0 = 16,
#endif //DS_BYPASS_2DGC

    //UR SW registers
    .iUltraRes_en = 1,
    .iUltraRes_ratio_thr0 = 1024,
    .iUltraRes_ratio_thr1 = 2048,
    .iUltraRes_ratio_thr2 = 4096,
    .iUltraRes_gain_mid_clip0 = 35,
    .iUltraRes_gain_mid_clip1 = 34,
    .iUltraRes_gain_high_clip0 = 50,
    .iUltraRes_gain_high_clip1 = 51,
#ifndef DS_BYPASS_EDS
    .iUltraRes_edf_edge_gain_clip0 = -2,
    .iUltraRes_edf_edge_gain_clip1 = -1,
    .iUltraRes_edf_detail_gain_clip0 = -2,
    .iUltraRes_edf_detail_gain_clip1 = -1,
    .iUltraRes_edf_flat_gain_clip0 = -2,
    .iUltraRes_edf_flat_gain_clip1 = -1,
#endif //DS_BYPASS_EDS
    .iUltraRes_clip_thr_clip0 = 0,
    .iUltraRes_clip_thr_clip1 = -4,
    .iUltraRes_clip_ratio_clip0 = 0,
    .iUltraRes_clip_ratio_clip1 = -4,

#ifndef DS_BYPASS_EDS
    .i_edf_flat_th_clip2 =  8,
    .i_edf_flat_th_clip1 =  4,
    .i_edf_flat_th_clip0 = 12,
    .i_edf_detail_rise_th_clip2 =  8,
    .i_edf_detail_rise_th_clip1 =  4,
    .i_edf_detail_rise_th_clip0 = 12
#endif //DS_BYPASS_EDS
    }
};

DSHWReg iDSHWRegEntry[UR_MAX_LEVEL] =
{
    {// iDSHWRegEntry_0
    .tdshp_en = 0,
    .tdshp_bypass_mid = 0,
    .tdshp_bypass_high = 0,
    .tdshp_ink_sel = 0,
    .tdshp_gain_mid = 0,
    .tdshp_gain_high = 0,
    .tdshp_softcoring_gain = 0,

    .tdshp_coring_thr = 0,
    .tdshp_coring_zero = 0,
    .tdshp_gain = 0,
    .tdshp_limit_ratio = 0,
    .tdshp_limit = 0,
    .tdshp_bound = 0,
    .tdshp_coring_value = 0,

    .tdshp_clip_en = 1,
    .tdshp_clip_ratio = 0,
    .tdshp_clip_thr = 0,
    .tdshp_ac_lpf_coe = 8,
    .tdshp_sat_proc = 10,

    .tdshp_ylev_p000 = 0,
    .tdshp_ylev_p016 = 24,
    .tdshp_ylev_p032 = 48,
    .tdshp_ylev_p048 = 72,
    .tdshp_ylev_p064 = 96,
    .tdshp_ylev_p080 = 104,
    .tdshp_ylev_p096 = 112,
    .tdshp_ylev_p112 = 120,
    .tdshp_ylev_p128 = 128,
    .tdshp_ylev_p144 = 123,
    .tdshp_ylev_p160 = 118,
    .tdshp_ylev_p176 = 112,
    .tdshp_ylev_p192 = 107,
    .tdshp_ylev_p208 = 102,
    .tdshp_ylev_p224 = 96,
    .tdshp_ylev_p240 = 91,
    .tdshp_ylev_256  = 86,
    .tdshp_ylev_alpha = 16,
    .tdshp_ylev_en = 1,


    .pbc1_en = 0,
    .pbc1_lpf_en = 0,
    .pbc1_gain = 0,
    .pbc1_rslope_1 = 39,
    .pbc1_theta_r = 12,
    .pbc1_radius_r = 12,
    .pbc1_theta_c = 127,
    .pbc1_radius_c = 26,
    .pbc1_tslope = 85,
    .pbc1_lpf_gain = 0,
    .pbc1_rslope = 85,
    .pbc1_conf_gain = 1,
    .pbc1_edge_en = 0,
    .pbc1_edge_thr = 12,
    .pbc1_edge_slope = 32,


    .pbc2_en = 0,
    .pbc2_lpf_en = 0,
    .pbc2_gain = 0,
    .pbc2_rslope_1 = 32,
    .pbc2_theta_r = 24,
    .pbc2_radius_r = 24,
    .pbc2_theta_c = 96,
    .pbc2_radius_c = 32,
    .pbc2_tslope = 43,
    .pbc2_lpf_gain = 0,
    .pbc2_rslope = 43,
    .pbc2_conf_gain = 5,
    .pbc2_edge_en = 1,
    .pbc2_edge_thr = 12,
    .pbc2_edge_slope = 32,

    .pbc3_en = 0,
    .pbc3_lpf_en = 0,
    .pbc3_gain = 36,
    .pbc3_rslope_1 = 17,
    .pbc3_theta_r = 16,
    .pbc3_radius_r = 32,
    .pbc3_theta_c = 224,
    .pbc3_radius_c = 60,
    .pbc3_tslope = 64,
    .pbc3_lpf_gain = 0,
    .pbc3_rslope = 32,
    .pbc3_conf_gain = 1,
    .pbc3_edge_en = 0,
    .pbc3_edge_thr = 12,
    .pbc3_edge_slope = 32,

#ifndef DS_BYPASS_2DGC
    .tdshp_mid_coring_thr = 4,
    .tdshp_mid_coring_zero = 2,
    .tdshp_mid_softlimit_ratio = 8,
    .tdshp_mid_limit = 24,
    .tdshp_mid_bound = 48,
    .tdshp_mid_coring_value = 3,
    .tdshp_mid_softcoring_gain = 16,

    .tdshp_high_coring_thr = 4,
    .tdshp_high_coring_zero = 2,
    .tdshp_high_softlimit_ratio = 8,
    .tdshp_high_limit = 24,
    .tdshp_high_bound = 48,
    .tdshp_high_coring_value = 3,
    .tdshp_high_softcoring_gain = 16,
#endif //DS_BYPASS_2DGC

#ifndef DS_BYPASS_EDS
    .edf_gain_en = 1,
    .edf_flat_gain = 0,
    .edf_detail_gain = 0,
    .edf_edge_gain = 0,
    .edf_clip_ratio_inc = 2,

    .edf_flat_th = 16,
    .edf_detail_rise_th = 16,
    .edf_detail_fall_th = 240,
    .edf_edge_th = 240,

    .edf_flat_slope = 4,
    .edf_detail_rise_slope = 5,
    .edf_detail_fall_slope = 10,
    .edf_edge_slope = 10,

    .edf_edge_mag_th = 0,
    .edf_edge_mag_slope = 8,
    .edf_edge_mono_th = 48,
    .edf_edge_mono_slope = 7,

    .edf_edge_trend_th = 32,
    .edf_edge_trend_slope = 16,
    .edf_edge_trend_flat_mag = 8,

    .edf_bld_wgt_trend = 32,
    .edf_bld_wgt_mono = 32,
    .edf_bld_wgt_mag = 32,
#endif //DS_BYPASS_EDS

#ifndef DS_BYPASS_CB
    .tdshp_cboost_lmt_u = 255,
    .tdshp_cboost_lmt_l = 128,
    .tdshp_cboost_en = 0,
    .tdshp_cboost_gain = 128,
    .tdshp_cboost_yconst = 16,
    .tdshp_cboost_yoffset_sel = 0,
    .tdshp_cboost_yoffset = 0,
#endif

#ifndef DS_BYPASS_POSTYLEV
    .tdshp_post_ylev_p000 = 0,
    .tdshp_post_ylev_p016 = 24,
    .tdshp_post_ylev_p032 = 48,
    .tdshp_post_ylev_p048 = 72,
    .tdshp_post_ylev_p064 = 96,
    .tdshp_post_ylev_p080 = 104,
    .tdshp_post_ylev_p096 = 112,
    .tdshp_post_ylev_p112 = 120,
    .tdshp_post_ylev_p128 = 128,
    .tdshp_post_ylev_p144 = 123,
    .tdshp_post_ylev_p160 = 118,
    .tdshp_post_ylev_p176 = 112,
    .tdshp_post_ylev_p192 = 107,
    .tdshp_post_ylev_p208 = 102,
    .tdshp_post_ylev_p224 = 96,
    .tdshp_post_ylev_p240 = 91,
    .tdshp_post_ylev_256  = 86,
    .tdshp_post_ylev_alpha = 16,
    .tdshp_post_ylev_en = 0,
#endif //DS_BYPASS_POSTYLEV
    },

    {// iDSHWRegEntry_1
    .tdshp_en = 1,
    .tdshp_bypass_mid = 0,
    .tdshp_bypass_high = 0,
    .tdshp_ink_sel = 0,
    .tdshp_gain_mid = 16,
    .tdshp_gain_high = 32,
    .tdshp_softcoring_gain = 16,

    .tdshp_coring_thr = 4,
    .tdshp_coring_zero = 2,
    .tdshp_gain = 32,
    .tdshp_limit_ratio = 8,
    .tdshp_limit = 24,
    .tdshp_bound = 48,
    .tdshp_coring_value = 3,

    .tdshp_clip_en = 1,
    .tdshp_clip_ratio = 8,
    .tdshp_clip_thr = 2,
    .tdshp_ac_lpf_coe = 8,
    .tdshp_sat_proc = 10,

    .tdshp_ylev_p000 = 0,
    .tdshp_ylev_p016 = 24,
    .tdshp_ylev_p032 = 48,
    .tdshp_ylev_p048 = 72,
    .tdshp_ylev_p064 = 96,
    .tdshp_ylev_p080 = 104,
    .tdshp_ylev_p096 = 112,
    .tdshp_ylev_p112 = 120,
    .tdshp_ylev_p128 = 128,
    .tdshp_ylev_p144 = 123,
    .tdshp_ylev_p160 = 118,
    .tdshp_ylev_p176 = 112,
    .tdshp_ylev_p192 = 107,
    .tdshp_ylev_p208 = 102,
    .tdshp_ylev_p224 = 96,
    .tdshp_ylev_p240 = 91,
    .tdshp_ylev_256  = 86,
    .tdshp_ylev_alpha = 16,
    .tdshp_ylev_en = 1,


    .pbc1_en = 1,
    .pbc1_lpf_en = 0,
    .pbc1_gain = 48,
    .pbc1_rslope_1 = 39,
    .pbc1_theta_r = 12,
    .pbc1_radius_r = 12,
    .pbc1_theta_c = 127,
    .pbc1_radius_c = 26,
    .pbc1_tslope = 85,
    .pbc1_lpf_gain = 16,
    .pbc1_rslope = 85,
    .pbc1_conf_gain = 1,
    .pbc1_edge_en = 0,
    .pbc1_edge_thr = 12,
    .pbc1_edge_slope = 32,


    .pbc2_en = 1,
    .pbc2_lpf_en = 1,
    .pbc2_gain = 16,
    .pbc2_rslope_1 = 32,
    .pbc2_theta_r = 24,
    .pbc2_radius_r = 24,
    .pbc2_theta_c = 96,
    .pbc2_radius_c = 32,
    .pbc2_tslope = 43,
    .pbc2_lpf_gain = 16,
    .pbc2_rslope = 43,
    .pbc2_conf_gain = 5,
    .pbc2_edge_en = 1,
    .pbc2_edge_thr = 12,
    .pbc2_edge_slope = 32,

    .pbc3_en = 1,
    .pbc3_lpf_en = 0,
    .pbc3_gain = 36,
    .pbc3_rslope_1 = 17,
    .pbc3_theta_r = 16,
    .pbc3_radius_r = 32,
    .pbc3_theta_c = 224,
    .pbc3_radius_c = 60,
    .pbc3_tslope = 64,
    .pbc3_lpf_gain = 16,
    .pbc3_rslope = 32,
    .pbc3_conf_gain = 1,
    .pbc3_edge_en = 0,
    .pbc3_edge_thr = 12,
    .pbc3_edge_slope = 32,

#ifndef DS_BYPASS_2DGC
    .tdshp_mid_coring_thr = 4,
    .tdshp_mid_coring_zero = 2,
    .tdshp_mid_softlimit_ratio = 8,
    .tdshp_mid_limit = 24,
    .tdshp_mid_bound = 48,
    .tdshp_mid_coring_value = 3,
    .tdshp_mid_softcoring_gain = 16,

    .tdshp_high_coring_thr = 4,
    .tdshp_high_coring_zero = 2,
    .tdshp_high_softlimit_ratio = 8,
    .tdshp_high_limit = 24,
    .tdshp_high_bound = 48,
    .tdshp_high_coring_value = 3,
    .tdshp_high_softcoring_gain = 16,
#endif //DS_BYPASS_2DGC

#ifndef DS_BYPASS_EDS
    .edf_gain_en = 1,
    .edf_flat_gain = 0,
    .edf_detail_gain = 64,
    .edf_edge_gain = 16,
    .edf_clip_ratio_inc = 2,

    .edf_flat_th = 16,
    .edf_detail_rise_th = 16,
    .edf_detail_fall_th = 240,
    .edf_edge_th = 240,

    .edf_flat_slope = 4,
    .edf_detail_rise_slope = 5,
    .edf_detail_fall_slope = 10,
    .edf_edge_slope = 10,

    .edf_edge_mag_th = 0,
    .edf_edge_mag_slope = 8,
    .edf_edge_mono_th = 48,
    .edf_edge_mono_slope = 7,

    .edf_edge_trend_th = 32,
    .edf_edge_trend_slope = 16,
    .edf_edge_trend_flat_mag = 8,

    .edf_bld_wgt_trend = 32,
    .edf_bld_wgt_mono = 32,
    .edf_bld_wgt_mag = 32,
#endif //DS_BYPASS_EDS

#ifndef DS_BYPASS_CB
    .tdshp_cboost_lmt_u = 255,
    .tdshp_cboost_lmt_l = 128,
    .tdshp_cboost_en = 1,
    .tdshp_cboost_gain = 128,
    .tdshp_cboost_yconst = 16,
    .tdshp_cboost_yoffset_sel = 0,
    .tdshp_cboost_yoffset = 0,
#endif

#ifndef DS_BYPASS_POSTYLEV
    .tdshp_post_ylev_p000 = 0,
    .tdshp_post_ylev_p016 = 24,
    .tdshp_post_ylev_p032 = 48,
    .tdshp_post_ylev_p048 = 72,
    .tdshp_post_ylev_p064 = 96,
    .tdshp_post_ylev_p080 = 104,
    .tdshp_post_ylev_p096 = 112,
    .tdshp_post_ylev_p112 = 120,
    .tdshp_post_ylev_p128 = 128,
    .tdshp_post_ylev_p144 = 123,
    .tdshp_post_ylev_p160 = 118,
    .tdshp_post_ylev_p176 = 112,
    .tdshp_post_ylev_p192 = 107,
    .tdshp_post_ylev_p208 = 102,
    .tdshp_post_ylev_p224 = 96,
    .tdshp_post_ylev_p240 = 91,
    .tdshp_post_ylev_256  = 86,
    .tdshp_post_ylev_alpha = 16,
    .tdshp_post_ylev_en = 0,
#endif //DS_BYPASS_POSTYLEV
    },


    {// iDSHWRegEntry_2
    .tdshp_en = 1,
    .tdshp_bypass_mid = 0,
    .tdshp_bypass_high = 0,
    .tdshp_ink_sel = 0,
    .tdshp_gain_mid = 16,
    .tdshp_gain_high = 32,
    .tdshp_softcoring_gain = 16,

    .tdshp_coring_thr = 12,
    .tdshp_coring_zero = 4,
    .tdshp_gain = 3,
    .tdshp_limit_ratio = 8,
    .tdshp_limit = 24,
    .tdshp_bound = 32,
    .tdshp_coring_value = 3,

    .tdshp_clip_en = 1,
    .tdshp_clip_ratio = 8,
    .tdshp_clip_thr = 2,
    .tdshp_ac_lpf_coe = 8,
    .tdshp_sat_proc = 10,

    .tdshp_ylev_p000 = 0,
    .tdshp_ylev_p016 = 24,
    .tdshp_ylev_p032 = 48,
    .tdshp_ylev_p048 = 72,
    .tdshp_ylev_p064 = 96,
    .tdshp_ylev_p080 = 104,
    .tdshp_ylev_p096 = 112,
    .tdshp_ylev_p112 = 120,
    .tdshp_ylev_p128 = 128,
    .tdshp_ylev_p144 = 123,
    .tdshp_ylev_p160 = 118,
    .tdshp_ylev_p176 = 112,
    .tdshp_ylev_p192 = 107,
    .tdshp_ylev_p208 = 102,
    .tdshp_ylev_p224 = 96,
    .tdshp_ylev_p240 = 91,
    .tdshp_ylev_256  = 86,
    .tdshp_ylev_alpha = 16,
    .tdshp_ylev_en = 1,


    .pbc1_en = 1,
    .pbc1_lpf_en = 0,
    .pbc1_gain = 48,
    .pbc1_rslope_1 = 39,
    .pbc1_theta_r = 12,
    .pbc1_radius_r = 12,
    .pbc1_theta_c = 127,
    .pbc1_radius_c = 26,
    .pbc1_tslope = 85,
    .pbc1_lpf_gain = 16,
    .pbc1_rslope = 85,
    .pbc1_conf_gain = 1,
    .pbc1_edge_en = 0,
    .pbc1_edge_thr = 12,
    .pbc1_edge_slope = 32,


    .pbc2_en = 1,
    .pbc2_lpf_en = 1,
    .pbc2_gain = 16,
    .pbc2_rslope_1 = 32,
    .pbc2_theta_r = 24,
    .pbc2_radius_r = 24,
    .pbc2_theta_c = 96,
    .pbc2_radius_c = 32,
    .pbc2_tslope = 43,
    .pbc2_lpf_gain = 16,
    .pbc2_rslope = 43,
    .pbc2_conf_gain = 5,
    .pbc2_edge_en = 1,
    .pbc2_edge_thr = 12,
    .pbc2_edge_slope = 32,

    .pbc3_en = 1,
    .pbc3_lpf_en = 0,
    .pbc3_gain = 36,
    .pbc3_rslope_1 = 17,
    .pbc3_theta_r = 16,
    .pbc3_radius_r = 32,
    .pbc3_theta_c = 224,
    .pbc3_radius_c = 60,
    .pbc3_tslope = 64,
    .pbc3_lpf_gain = 16,
    .pbc3_rslope = 32,
    .pbc3_conf_gain = 1,
    .pbc3_edge_en = 0,
    .pbc3_edge_thr = 12,
    .pbc3_edge_slope = 32,

#ifndef DS_BYPASS_2DGC
    .tdshp_mid_coring_thr = 4,
    .tdshp_mid_coring_zero = 2,
    .tdshp_mid_softlimit_ratio = 8,
    .tdshp_mid_limit = 24,
    .tdshp_mid_bound = 48,
    .tdshp_mid_coring_value = 3,
    .tdshp_mid_softcoring_gain = 16,

    .tdshp_high_coring_thr = 4,
    .tdshp_high_coring_zero = 2,
    .tdshp_high_softlimit_ratio = 8,
    .tdshp_high_limit = 24,
    .tdshp_high_bound = 48,
    .tdshp_high_coring_value = 3,
    .tdshp_high_softcoring_gain = 16,
#endif //DS_BYPASS_2DGC

#ifndef DS_BYPASS_EDS
    .edf_gain_en = 1,
    .edf_flat_gain = 0,
    .edf_detail_gain = 32,
    .edf_edge_gain = 8,
    .edf_clip_ratio_inc = 2,

    .edf_flat_th = 16,
    .edf_detail_rise_th = 16,
    .edf_detail_fall_th = 240,
    .edf_edge_th = 240,

    .edf_flat_slope = 4,
    .edf_detail_rise_slope = 5,
    .edf_detail_fall_slope = 10,
    .edf_edge_slope = 10,

    .edf_edge_mag_th = 0,
    .edf_edge_mag_slope = 8,
    .edf_edge_mono_th = 48,
    .edf_edge_mono_slope = 7,

    .edf_edge_trend_th = 32,
    .edf_edge_trend_slope = 16,
    .edf_edge_trend_flat_mag = 8,

    .edf_bld_wgt_trend = 32,
    .edf_bld_wgt_mono = 32,
    .edf_bld_wgt_mag = 32,
#endif //DS_BYPASS_EDS

#ifndef DS_BYPASS_CB
    .tdshp_cboost_lmt_u = 255,
    .tdshp_cboost_lmt_l = 128,
    .tdshp_cboost_en = 1,
    .tdshp_cboost_gain = 128,
    .tdshp_cboost_yconst = 16,
    .tdshp_cboost_yoffset_sel = 0,
    .tdshp_cboost_yoffset = 0,
#endif

#ifndef DS_BYPASS_POSTYLEV
    .tdshp_post_ylev_p000 = 0,
    .tdshp_post_ylev_p016 = 24,
    .tdshp_post_ylev_p032 = 48,
    .tdshp_post_ylev_p048 = 72,
    .tdshp_post_ylev_p064 = 96,
    .tdshp_post_ylev_p080 = 104,
    .tdshp_post_ylev_p096 = 112,
    .tdshp_post_ylev_p112 = 120,
    .tdshp_post_ylev_p128 = 128,
    .tdshp_post_ylev_p144 = 123,
    .tdshp_post_ylev_p160 = 118,
    .tdshp_post_ylev_p176 = 112,
    .tdshp_post_ylev_p192 = 107,
    .tdshp_post_ylev_p208 = 102,
    .tdshp_post_ylev_p224 = 96,
    .tdshp_post_ylev_p240 = 91,
    .tdshp_post_ylev_256  = 86,
    .tdshp_post_ylev_alpha = 16,
    .tdshp_post_ylev_en = 0,
#endif //DS_BYPASS_POSTYLEV
    },

    {// iDSHWRegEntry_3
    .tdshp_en = 1,
    .tdshp_bypass_mid = 0,
    .tdshp_bypass_high = 0,
    .tdshp_ink_sel = 0,
    .tdshp_gain_mid = 16,
    .tdshp_gain_high = 32,
    .tdshp_softcoring_gain = 16,

    .tdshp_coring_thr = 4,
    .tdshp_coring_zero = 2,
    .tdshp_gain = 32,
    .tdshp_limit_ratio = 8,
    .tdshp_limit = 24,
    .tdshp_bound = 48,
    .tdshp_coring_value = 3,

    .tdshp_clip_en = 1,
    .tdshp_clip_ratio = 8,
    .tdshp_clip_thr = 2,
    .tdshp_ac_lpf_coe = 8,
    .tdshp_sat_proc = 10,

    .tdshp_ylev_p000 = 0,
    .tdshp_ylev_p016 = 24,
    .tdshp_ylev_p032 = 48,
    .tdshp_ylev_p048 = 72,
    .tdshp_ylev_p064 = 96,
    .tdshp_ylev_p080 = 104,
    .tdshp_ylev_p096 = 112,
    .tdshp_ylev_p112 = 120,
    .tdshp_ylev_p128 = 128,
    .tdshp_ylev_p144 = 123,
    .tdshp_ylev_p160 = 118,
    .tdshp_ylev_p176 = 112,
    .tdshp_ylev_p192 = 107,
    .tdshp_ylev_p208 = 102,
    .tdshp_ylev_p224 = 96,
    .tdshp_ylev_p240 = 91,
    .tdshp_ylev_256  = 86,
    .tdshp_ylev_alpha = 16,
    .tdshp_ylev_en = 1,


    .pbc1_en = 1,
    .pbc1_lpf_en = 0,
    .pbc1_gain = 48,
    .pbc1_rslope_1 = 39,
    .pbc1_theta_r = 12,
    .pbc1_radius_r = 12,
    .pbc1_theta_c = 127,
    .pbc1_radius_c = 26,
    .pbc1_tslope = 85,
    .pbc1_lpf_gain = 16,
    .pbc1_rslope = 85,
    .pbc1_conf_gain = 1,
    .pbc1_edge_en = 0,
    .pbc1_edge_thr = 12,
    .pbc1_edge_slope = 32,


    .pbc2_en = 1,
    .pbc2_lpf_en = 1,
    .pbc2_gain = 16,
    .pbc2_rslope_1 = 32,
    .pbc2_theta_r = 24,
    .pbc2_radius_r = 24,
    .pbc2_theta_c = 96,
    .pbc2_radius_c = 32,
    .pbc2_tslope = 43,
    .pbc2_lpf_gain = 16,
    .pbc2_rslope = 43,
    .pbc2_conf_gain = 5,
    .pbc2_edge_en = 1,
    .pbc2_edge_thr = 12,
    .pbc2_edge_slope = 32,

    .pbc3_en = 1,
    .pbc3_lpf_en = 0,
    .pbc3_gain = 36,
    .pbc3_rslope_1 = 17,
    .pbc3_theta_r = 16,
    .pbc3_radius_r = 32,
    .pbc3_theta_c = 224,
    .pbc3_radius_c = 60,
    .pbc3_tslope = 64,
    .pbc3_lpf_gain = 16,
    .pbc3_rslope = 32,
    .pbc3_conf_gain = 1,
    .pbc3_edge_en = 0,
    .pbc3_edge_thr = 12,
    .pbc3_edge_slope = 32,

#ifndef DS_BYPASS_2DGC
    .tdshp_mid_coring_thr = 4,
    .tdshp_mid_coring_zero = 2,
    .tdshp_mid_softlimit_ratio = 8,
    .tdshp_mid_limit = 24,
    .tdshp_mid_bound = 48,
    .tdshp_mid_coring_value = 3,
    .tdshp_mid_softcoring_gain = 16,

    .tdshp_high_coring_thr = 4,
    .tdshp_high_coring_zero = 2,
    .tdshp_high_softlimit_ratio = 8,
    .tdshp_high_limit = 24,
    .tdshp_high_bound = 48,
    .tdshp_high_coring_value = 3,
    .tdshp_high_softcoring_gain = 16,
#endif //DS_BYPASS_2DGC

#ifndef DS_BYPASS_EDS
    .edf_gain_en = 1,
    .edf_flat_gain = 0,
    .edf_detail_gain = 64,
    .edf_edge_gain = 16,
    .edf_clip_ratio_inc = 2,

    .edf_flat_th = 16,
    .edf_detail_rise_th = 16,
    .edf_detail_fall_th = 240,
    .edf_edge_th = 240,

    .edf_flat_slope = 4,
    .edf_detail_rise_slope = 5,
    .edf_detail_fall_slope = 10,
    .edf_edge_slope = 10,

    .edf_edge_mag_th = 0,
    .edf_edge_mag_slope = 8,
    .edf_edge_mono_th = 48,
    .edf_edge_mono_slope = 7,

    .edf_edge_trend_th = 32,
    .edf_edge_trend_slope = 16,
    .edf_edge_trend_flat_mag = 8,

    .edf_bld_wgt_trend = 32,
    .edf_bld_wgt_mono = 32,
    .edf_bld_wgt_mag = 32,
#endif //DS_BYPASS_EDS

#ifndef DS_BYPASS_CB
    .tdshp_cboost_lmt_u = 255,
    .tdshp_cboost_lmt_l = 128,
    .tdshp_cboost_en = 1,
    .tdshp_cboost_gain = 128,
    .tdshp_cboost_yconst = 16,
    .tdshp_cboost_yoffset_sel = 0,
    .tdshp_cboost_yoffset = 0,
#endif

#ifndef DS_BYPASS_POSTYLEV
    .tdshp_post_ylev_p000 = 0,
    .tdshp_post_ylev_p016 = 24,
    .tdshp_post_ylev_p032 = 48,
    .tdshp_post_ylev_p048 = 72,
    .tdshp_post_ylev_p064 = 96,
    .tdshp_post_ylev_p080 = 104,
    .tdshp_post_ylev_p096 = 112,
    .tdshp_post_ylev_p112 = 120,
    .tdshp_post_ylev_p128 = 128,
    .tdshp_post_ylev_p144 = 123,
    .tdshp_post_ylev_p160 = 118,
    .tdshp_post_ylev_p176 = 112,
    .tdshp_post_ylev_p192 = 107,
    .tdshp_post_ylev_p208 = 102,
    .tdshp_post_ylev_p224 = 96,
    .tdshp_post_ylev_p240 = 91,
    .tdshp_post_ylev_256  = 86,
    .tdshp_post_ylev_alpha = 16,
    .tdshp_post_ylev_en = 0,
#endif //DS_BYPASS_POSTYLEV
    },

    {// iDSHWRegEntry_4
    .tdshp_en = 1,
    .tdshp_bypass_mid = 0,
    .tdshp_bypass_high = 0,
    .tdshp_ink_sel = 0,
    .tdshp_gain_mid = 16,
    .tdshp_gain_high = 32,
    .tdshp_softcoring_gain = 16,

    .tdshp_coring_thr = 4,
    .tdshp_coring_zero = 2,
    .tdshp_gain = 32,
    .tdshp_limit_ratio = 8,
    .tdshp_limit = 24,
    .tdshp_bound = 48,
    .tdshp_coring_value = 3,

    .tdshp_clip_en = 1,
    .tdshp_clip_ratio = 8,
    .tdshp_clip_thr = 2,
    .tdshp_ac_lpf_coe = 8,
    .tdshp_sat_proc = 10,

    .tdshp_ylev_p000 = 0,
    .tdshp_ylev_p016 = 24,
    .tdshp_ylev_p032 = 48,
    .tdshp_ylev_p048 = 72,
    .tdshp_ylev_p064 = 96,
    .tdshp_ylev_p080 = 104,
    .tdshp_ylev_p096 = 112,
    .tdshp_ylev_p112 = 120,
    .tdshp_ylev_p128 = 128,
    .tdshp_ylev_p144 = 123,
    .tdshp_ylev_p160 = 118,
    .tdshp_ylev_p176 = 112,
    .tdshp_ylev_p192 = 107,
    .tdshp_ylev_p208 = 102,
    .tdshp_ylev_p224 = 96,
    .tdshp_ylev_p240 = 91,
    .tdshp_ylev_256  = 86,
    .tdshp_ylev_alpha = 16,
    .tdshp_ylev_en = 1,


    .pbc1_en = 1,
    .pbc1_lpf_en = 0,
    .pbc1_gain = 48,
    .pbc1_rslope_1 = 39,
    .pbc1_theta_r = 12,
    .pbc1_radius_r = 12,
    .pbc1_theta_c = 127,
    .pbc1_radius_c = 26,
    .pbc1_tslope = 85,
    .pbc1_lpf_gain = 16,
    .pbc1_rslope = 85,
    .pbc1_conf_gain = 1,
    .pbc1_edge_en = 0,
    .pbc1_edge_thr = 12,
    .pbc1_edge_slope = 32,


    .pbc2_en = 1,
    .pbc2_lpf_en = 1,
    .pbc2_gain = 16,
    .pbc2_rslope_1 = 32,
    .pbc2_theta_r = 24,
    .pbc2_radius_r = 24,
    .pbc2_theta_c = 96,
    .pbc2_radius_c = 32,
    .pbc2_tslope = 43,
    .pbc2_lpf_gain = 16,
    .pbc2_rslope = 43,
    .pbc2_conf_gain = 5,
    .pbc2_edge_en = 1,
    .pbc2_edge_thr = 12,
    .pbc2_edge_slope = 32,

    .pbc3_en = 1,
    .pbc3_lpf_en = 0,
    .pbc3_gain = 36,
    .pbc3_rslope_1 = 17,
    .pbc3_theta_r = 16,
    .pbc3_radius_r = 32,
    .pbc3_theta_c = 224,
    .pbc3_radius_c = 60,
    .pbc3_tslope = 64,
    .pbc3_lpf_gain = 16,
    .pbc3_rslope = 32,
    .pbc3_conf_gain = 1,
    .pbc3_edge_en = 0,
    .pbc3_edge_thr = 12,
    .pbc3_edge_slope = 32,

#ifndef DS_BYPASS_2DGC
    .tdshp_mid_coring_thr = 4,
    .tdshp_mid_coring_zero = 2,
    .tdshp_mid_softlimit_ratio = 8,
    .tdshp_mid_limit = 24,
    .tdshp_mid_bound = 48,
    .tdshp_mid_coring_value = 3,
    .tdshp_mid_softcoring_gain = 16,

    .tdshp_high_coring_thr = 4,
    .tdshp_high_coring_zero = 2,
    .tdshp_high_softlimit_ratio = 8,
    .tdshp_high_limit = 24,
    .tdshp_high_bound = 48,
    .tdshp_high_coring_value = 3,
    .tdshp_high_softcoring_gain = 16,
#endif //DS_BYPASS_2DGC

#ifndef DS_BYPASS_EDS
    .edf_gain_en = 1,
    .edf_flat_gain = 0,
    .edf_detail_gain = 64,
    .edf_edge_gain = 16,
    .edf_clip_ratio_inc = 2,

    .edf_flat_th = 16,
    .edf_detail_rise_th = 16,
    .edf_detail_fall_th = 240,
    .edf_edge_th = 240,

    .edf_flat_slope = 4,
    .edf_detail_rise_slope = 5,
    .edf_detail_fall_slope = 10,
    .edf_edge_slope = 10,

    .edf_edge_mag_th = 0,
    .edf_edge_mag_slope = 8,
    .edf_edge_mono_th = 48,
    .edf_edge_mono_slope = 7,

    .edf_edge_trend_th = 32,
    .edf_edge_trend_slope = 16,
    .edf_edge_trend_flat_mag = 8,

    .edf_bld_wgt_trend = 32,
    .edf_bld_wgt_mono = 32,
    .edf_bld_wgt_mag = 32,
#endif //DS_BYPASS_EDS

#ifndef DS_BYPASS_CB
    .tdshp_cboost_lmt_u = 255,
    .tdshp_cboost_lmt_l = 128,
    .tdshp_cboost_en = 1,
    .tdshp_cboost_gain = 128,
    .tdshp_cboost_yconst = 16,
    .tdshp_cboost_yoffset_sel = 0,
    .tdshp_cboost_yoffset = 0,
#endif

#ifndef DS_BYPASS_POSTYLEV
    .tdshp_post_ylev_p000 = 0,
    .tdshp_post_ylev_p016 = 24,
    .tdshp_post_ylev_p032 = 48,
    .tdshp_post_ylev_p048 = 72,
    .tdshp_post_ylev_p064 = 96,
    .tdshp_post_ylev_p080 = 104,
    .tdshp_post_ylev_p096 = 112,
    .tdshp_post_ylev_p112 = 120,
    .tdshp_post_ylev_p128 = 128,
    .tdshp_post_ylev_p144 = 123,
    .tdshp_post_ylev_p160 = 118,
    .tdshp_post_ylev_p176 = 112,
    .tdshp_post_ylev_p192 = 107,
    .tdshp_post_ylev_p208 = 102,
    .tdshp_post_ylev_p224 = 96,
    .tdshp_post_ylev_p240 = 91,
    .tdshp_post_ylev_256  = 86,
    .tdshp_post_ylev_alpha = 16,
    .tdshp_post_ylev_en = 0,
#endif //DS_BYPASS_POSTYLEV
    },

    {// iDSHWRegEntry_5
    .tdshp_en = 1,
    .tdshp_bypass_mid = 0,
    .tdshp_bypass_high = 0,
    .tdshp_ink_sel = 0,
    .tdshp_gain_mid = 16,
    .tdshp_gain_high = 32,
    .tdshp_softcoring_gain = 16,

    .tdshp_coring_thr = 4,
    .tdshp_coring_zero = 2,
    .tdshp_gain = 32,
    .tdshp_limit_ratio = 8,
    .tdshp_limit = 24,
    .tdshp_bound = 48,
    .tdshp_coring_value = 3,

    .tdshp_clip_en = 1,
    .tdshp_clip_ratio = 8,
    .tdshp_clip_thr = 2,
    .tdshp_ac_lpf_coe = 8,
    .tdshp_sat_proc = 10,

    .tdshp_ylev_p000 = 0,
    .tdshp_ylev_p016 = 24,
    .tdshp_ylev_p032 = 48,
    .tdshp_ylev_p048 = 72,
    .tdshp_ylev_p064 = 96,
    .tdshp_ylev_p080 = 104,
    .tdshp_ylev_p096 = 112,
    .tdshp_ylev_p112 = 120,
    .tdshp_ylev_p128 = 128,
    .tdshp_ylev_p144 = 123,
    .tdshp_ylev_p160 = 118,
    .tdshp_ylev_p176 = 112,
    .tdshp_ylev_p192 = 107,
    .tdshp_ylev_p208 = 102,
    .tdshp_ylev_p224 = 96,
    .tdshp_ylev_p240 = 91,
    .tdshp_ylev_256  = 86,
    .tdshp_ylev_alpha = 16,
    .tdshp_ylev_en = 1,


    .pbc1_en = 1,
    .pbc1_lpf_en = 0,
    .pbc1_gain = 48,
    .pbc1_rslope_1 = 39,
    .pbc1_theta_r = 12,
    .pbc1_radius_r = 12,
    .pbc1_theta_c = 127,
    .pbc1_radius_c = 26,
    .pbc1_tslope = 85,
    .pbc1_lpf_gain = 16,
    .pbc1_rslope = 85,
    .pbc1_conf_gain = 1,
    .pbc1_edge_en = 0,
    .pbc1_edge_thr = 12,
    .pbc1_edge_slope = 32,


    .pbc2_en = 1,
    .pbc2_lpf_en = 1,
    .pbc2_gain = 16,
    .pbc2_rslope_1 = 32,
    .pbc2_theta_r = 24,
    .pbc2_radius_r = 24,
    .pbc2_theta_c = 96,
    .pbc2_radius_c = 32,
    .pbc2_tslope = 43,
    .pbc2_lpf_gain = 16,
    .pbc2_rslope = 43,
    .pbc2_conf_gain = 5,
    .pbc2_edge_en = 1,
    .pbc2_edge_thr = 12,
    .pbc2_edge_slope = 32,

    .pbc3_en = 1,
    .pbc3_lpf_en = 0,
    .pbc3_gain = 36,
    .pbc3_rslope_1 = 17,
    .pbc3_theta_r = 16,
    .pbc3_radius_r = 32,
    .pbc3_theta_c = 224,
    .pbc3_radius_c = 60,
    .pbc3_tslope = 64,
    .pbc3_lpf_gain = 16,
    .pbc3_rslope = 32,
    .pbc3_conf_gain = 1,
    .pbc3_edge_en = 0,
    .pbc3_edge_thr = 12,
    .pbc3_edge_slope = 32,

#ifndef DS_BYPASS_2DGC
    .tdshp_mid_coring_thr = 4,
    .tdshp_mid_coring_zero = 2,
    .tdshp_mid_softlimit_ratio = 8,
    .tdshp_mid_limit = 24,
    .tdshp_mid_bound = 48,
    .tdshp_mid_coring_value = 3,
    .tdshp_mid_softcoring_gain = 16,

    .tdshp_high_coring_thr = 4,
    .tdshp_high_coring_zero = 2,
    .tdshp_high_softlimit_ratio = 8,
    .tdshp_high_limit = 24,
    .tdshp_high_bound = 48,
    .tdshp_high_coring_value = 3,
    .tdshp_high_softcoring_gain = 16,
#endif //DS_BYPASS_2DGC

#ifndef DS_BYPASS_EDS
    .edf_gain_en = 1,
    .edf_flat_gain = 0,
    .edf_detail_gain = 64,
    .edf_edge_gain = 16,
    .edf_clip_ratio_inc = 2,

    .edf_flat_th = 16,
    .edf_detail_rise_th = 16,
    .edf_detail_fall_th = 240,
    .edf_edge_th = 240,

    .edf_flat_slope = 4,
    .edf_detail_rise_slope = 5,
    .edf_detail_fall_slope = 10,
    .edf_edge_slope = 10,

    .edf_edge_mag_th = 0,
    .edf_edge_mag_slope = 8,
    .edf_edge_mono_th = 48,
    .edf_edge_mono_slope = 7,

    .edf_edge_trend_th = 32,
    .edf_edge_trend_slope = 16,
    .edf_edge_trend_flat_mag = 8,

    .edf_bld_wgt_trend = 32,
    .edf_bld_wgt_mono = 32,
    .edf_bld_wgt_mag = 32,
#endif //DS_BYPASS_EDS

#ifndef DS_BYPASS_CB
    .tdshp_cboost_lmt_u = 255,
    .tdshp_cboost_lmt_l = 128,
    .tdshp_cboost_en = 1,
    .tdshp_cboost_gain = 128,
    .tdshp_cboost_yconst = 16,
    .tdshp_cboost_yoffset_sel = 0,
    .tdshp_cboost_yoffset = 0,
#endif

#ifndef DS_BYPASS_POSTYLEV
    .tdshp_post_ylev_p000 = 0,
    .tdshp_post_ylev_p016 = 24,
    .tdshp_post_ylev_p032 = 48,
    .tdshp_post_ylev_p048 = 72,
    .tdshp_post_ylev_p064 = 96,
    .tdshp_post_ylev_p080 = 104,
    .tdshp_post_ylev_p096 = 112,
    .tdshp_post_ylev_p112 = 120,
    .tdshp_post_ylev_p128 = 128,
    .tdshp_post_ylev_p144 = 123,
    .tdshp_post_ylev_p160 = 118,
    .tdshp_post_ylev_p176 = 112,
    .tdshp_post_ylev_p192 = 107,
    .tdshp_post_ylev_p208 = 102,
    .tdshp_post_ylev_p224 = 96,
    .tdshp_post_ylev_p240 = 91,
    .tdshp_post_ylev_256  = 86,
    .tdshp_post_ylev_alpha = 16,
    .tdshp_post_ylev_en = 0,
#endif //DS_BYPASS_POSTYLEV
    },

    {// iDSHWRegEntry_6
    .tdshp_en = 1,
    .tdshp_bypass_mid = 0,
    .tdshp_bypass_high = 0,
    .tdshp_ink_sel = 0,
    .tdshp_gain_mid = 0,
    .tdshp_gain_high = 0,
    .tdshp_softcoring_gain = 8,

    .tdshp_coring_thr = 10,
    .tdshp_coring_zero = 4,
    .tdshp_gain = 3,
    .tdshp_limit_ratio = 8,
    .tdshp_limit = 24,
    .tdshp_bound = 48,
    .tdshp_coring_value = 3,

    .tdshp_clip_en = 1,
    .tdshp_clip_ratio = 8,
    .tdshp_clip_thr = 2,
    .tdshp_ac_lpf_coe = 8,
    .tdshp_sat_proc = 10,

    .tdshp_ylev_p000 = 0,
    .tdshp_ylev_p016 = 24,
    .tdshp_ylev_p032 = 48,
    .tdshp_ylev_p048 = 72,
    .tdshp_ylev_p064 = 96,
    .tdshp_ylev_p080 = 104,
    .tdshp_ylev_p096 = 112,
    .tdshp_ylev_p112 = 120,
    .tdshp_ylev_p128 = 128,
    .tdshp_ylev_p144 = 123,
    .tdshp_ylev_p160 = 118,
    .tdshp_ylev_p176 = 112,
    .tdshp_ylev_p192 = 107,
    .tdshp_ylev_p208 = 102,
    .tdshp_ylev_p224 = 96,
    .tdshp_ylev_p240 = 91,
    .tdshp_ylev_256  = 86,
    .tdshp_ylev_alpha = 16,
    .tdshp_ylev_en = 1,


    .pbc1_en = 1,
    .pbc1_lpf_en = 0,
    .pbc1_gain = 48,
    .pbc1_rslope_1 = 39,
    .pbc1_theta_r = 12,
    .pbc1_radius_r = 12,
    .pbc1_theta_c = 127,
    .pbc1_radius_c = 26,
    .pbc1_tslope = 85,
    .pbc1_lpf_gain = 16,
    .pbc1_rslope = 85,
    .pbc1_conf_gain = 1,
    .pbc1_edge_en = 0,
    .pbc1_edge_thr = 12,
    .pbc1_edge_slope = 32,


    .pbc2_en = 1,
    .pbc2_lpf_en = 1,
    .pbc2_gain = 16,
    .pbc2_rslope_1 = 32,
    .pbc2_theta_r = 24,
    .pbc2_radius_r = 24,
    .pbc2_theta_c = 96,
    .pbc2_radius_c = 32,
    .pbc2_tslope = 43,
    .pbc2_lpf_gain = 16,
    .pbc2_rslope = 43,
    .pbc2_conf_gain = 5,
    .pbc2_edge_en = 1,
    .pbc2_edge_thr = 12,
    .pbc2_edge_slope = 32,

    .pbc3_en = 1,
    .pbc3_lpf_en = 0,
    .pbc3_gain = 36,
    .pbc3_rslope_1 = 17,
    .pbc3_theta_r = 16,
    .pbc3_radius_r = 32,
    .pbc3_theta_c = 224,
    .pbc3_radius_c = 60,
    .pbc3_tslope = 64,
    .pbc3_lpf_gain = 16,
    .pbc3_rslope = 32,
    .pbc3_conf_gain = 1,
    .pbc3_edge_en = 0,
    .pbc3_edge_thr = 12,
    .pbc3_edge_slope = 32,

#ifndef DS_BYPASS_2DGC
    .tdshp_mid_coring_thr = 4,
    .tdshp_mid_coring_zero = 2,
    .tdshp_mid_softlimit_ratio = 8,
    .tdshp_mid_limit = 24,
    .tdshp_mid_bound = 48,
    .tdshp_mid_coring_value = 3,
    .tdshp_mid_softcoring_gain = 16,

    .tdshp_high_coring_thr = 4,
    .tdshp_high_coring_zero = 2,
    .tdshp_high_softlimit_ratio = 8,
    .tdshp_high_limit = 24,
    .tdshp_high_bound = 48,
    .tdshp_high_coring_value = 3,
    .tdshp_high_softcoring_gain = 16,
#endif //DS_BYPASS_2DGC

#ifndef DS_BYPASS_EDS
    .edf_gain_en = 1,
    .edf_flat_gain = 0,
    .edf_detail_gain = 32,
    .edf_edge_gain = 8,
    .edf_clip_ratio_inc = 2,

    .edf_flat_th = 16,
    .edf_detail_rise_th = 16,
    .edf_detail_fall_th = 240,
    .edf_edge_th = 240,

    .edf_flat_slope = 4,
    .edf_detail_rise_slope = 5,
    .edf_detail_fall_slope = 10,
    .edf_edge_slope = 10,

    .edf_edge_mag_th = 0,
    .edf_edge_mag_slope = 8,
    .edf_edge_mono_th = 48,
    .edf_edge_mono_slope = 7,

    .edf_edge_trend_th = 32,
    .edf_edge_trend_slope = 16,
    .edf_edge_trend_flat_mag = 8,

    .edf_bld_wgt_trend = 32,
    .edf_bld_wgt_mono = 32,
    .edf_bld_wgt_mag = 32,
#endif //DS_BYPASS_EDS

#ifndef DS_BYPASS_CB
    .tdshp_cboost_lmt_u = 255,
    .tdshp_cboost_lmt_l = 128,
    .tdshp_cboost_en = 1,
    .tdshp_cboost_gain = 128,
    .tdshp_cboost_yconst = 16,
    .tdshp_cboost_yoffset_sel = 0,
    .tdshp_cboost_yoffset = 0,
#endif

#ifndef DS_BYPASS_POSTYLEV
    .tdshp_post_ylev_p000 = 0,
    .tdshp_post_ylev_p016 = 24,
    .tdshp_post_ylev_p032 = 48,
    .tdshp_post_ylev_p048 = 72,
    .tdshp_post_ylev_p064 = 96,
    .tdshp_post_ylev_p080 = 104,
    .tdshp_post_ylev_p096 = 112,
    .tdshp_post_ylev_p112 = 120,
    .tdshp_post_ylev_p128 = 128,
    .tdshp_post_ylev_p144 = 123,
    .tdshp_post_ylev_p160 = 118,
    .tdshp_post_ylev_p176 = 112,
    .tdshp_post_ylev_p192 = 107,
    .tdshp_post_ylev_p208 = 102,
    .tdshp_post_ylev_p224 = 96,
    .tdshp_post_ylev_p240 = 91,
    .tdshp_post_ylev_256  = 86,
    .tdshp_post_ylev_alpha = 16,
    .tdshp_post_ylev_en = 0,
#endif //DS_BYPASS_POSTYLEV
    },

    {// iDSHWRegEntry_7
    .tdshp_en = 1,
    .tdshp_bypass_mid = 0,
    .tdshp_bypass_high = 0,
    .tdshp_ink_sel = 0,
    .tdshp_gain_mid = 16,
    .tdshp_gain_high = 32,
    .tdshp_softcoring_gain = 16,

    .tdshp_coring_thr = 4,
    .tdshp_coring_zero = 2,
    .tdshp_gain = 32,
    .tdshp_limit_ratio = 8,
    .tdshp_limit = 24,
    .tdshp_bound = 48,
    .tdshp_coring_value = 3,

    .tdshp_clip_en = 1,
    .tdshp_clip_ratio = 8,
    .tdshp_clip_thr = 2,
    .tdshp_ac_lpf_coe = 8,
    .tdshp_sat_proc = 10,

    .tdshp_ylev_p000 = 0,
    .tdshp_ylev_p016 = 24,
    .tdshp_ylev_p032 = 48,
    .tdshp_ylev_p048 = 72,
    .tdshp_ylev_p064 = 96,
    .tdshp_ylev_p080 = 104,
    .tdshp_ylev_p096 = 112,
    .tdshp_ylev_p112 = 120,
    .tdshp_ylev_p128 = 128,
    .tdshp_ylev_p144 = 123,
    .tdshp_ylev_p160 = 118,
    .tdshp_ylev_p176 = 112,
    .tdshp_ylev_p192 = 107,
    .tdshp_ylev_p208 = 102,
    .tdshp_ylev_p224 = 96,
    .tdshp_ylev_p240 = 91,
    .tdshp_ylev_256  = 86,
    .tdshp_ylev_alpha = 16,
    .tdshp_ylev_en = 1,


    .pbc1_en = 1,
    .pbc1_lpf_en = 0,
    .pbc1_gain = 48,
    .pbc1_rslope_1 = 39,
    .pbc1_theta_r = 12,
    .pbc1_radius_r = 12,
    .pbc1_theta_c = 127,
    .pbc1_radius_c = 26,
    .pbc1_tslope = 85,
    .pbc1_lpf_gain = 16,
    .pbc1_rslope = 85,
    .pbc1_conf_gain = 1,
    .pbc1_edge_en = 0,
    .pbc1_edge_thr = 12,
    .pbc1_edge_slope = 32,


    .pbc2_en = 1,
    .pbc2_lpf_en = 1,
    .pbc2_gain = 16,
    .pbc2_rslope_1 = 32,
    .pbc2_theta_r = 24,
    .pbc2_radius_r = 24,
    .pbc2_theta_c = 96,
    .pbc2_radius_c = 32,
    .pbc2_tslope = 43,
    .pbc2_lpf_gain = 16,
    .pbc2_rslope = 43,
    .pbc2_conf_gain = 5,
    .pbc2_edge_en = 1,
    .pbc2_edge_thr = 12,
    .pbc2_edge_slope = 32,

    .pbc3_en = 1,
    .pbc3_lpf_en = 0,
    .pbc3_gain = 36,
    .pbc3_rslope_1 = 17,
    .pbc3_theta_r = 16,
    .pbc3_radius_r = 32,
    .pbc3_theta_c = 224,
    .pbc3_radius_c = 60,
    .pbc3_tslope = 64,
    .pbc3_lpf_gain = 16,
    .pbc3_rslope = 32,
    .pbc3_conf_gain = 1,
    .pbc3_edge_en = 0,
    .pbc3_edge_thr = 12,
    .pbc3_edge_slope = 32,

#ifndef DS_BYPASS_2DGC
    .tdshp_mid_coring_thr = 4,
    .tdshp_mid_coring_zero = 2,
    .tdshp_mid_softlimit_ratio = 8,
    .tdshp_mid_limit = 24,
    .tdshp_mid_bound = 48,
    .tdshp_mid_coring_value = 3,
    .tdshp_mid_softcoring_gain = 16,

    .tdshp_high_coring_thr = 4,
    .tdshp_high_coring_zero = 2,
    .tdshp_high_softlimit_ratio = 8,
    .tdshp_high_limit = 24,
    .tdshp_high_bound = 48,
    .tdshp_high_coring_value = 3,
    .tdshp_high_softcoring_gain = 16,
#endif //DS_BYPASS_2DGC

#ifndef DS_BYPASS_EDS
    .edf_gain_en = 1,
    .edf_flat_gain = 0,
    .edf_detail_gain = 64,
    .edf_edge_gain = 16,
    .edf_clip_ratio_inc = 2,

    .edf_flat_th = 16,
    .edf_detail_rise_th = 16,
    .edf_detail_fall_th = 240,
    .edf_edge_th = 240,

    .edf_flat_slope = 4,
    .edf_detail_rise_slope = 5,
    .edf_detail_fall_slope = 10,
    .edf_edge_slope = 10,

    .edf_edge_mag_th = 0,
    .edf_edge_mag_slope = 8,
    .edf_edge_mono_th = 48,
    .edf_edge_mono_slope = 7,

    .edf_edge_trend_th = 32,
    .edf_edge_trend_slope = 16,
    .edf_edge_trend_flat_mag = 8,

    .edf_bld_wgt_trend = 32,
    .edf_bld_wgt_mono = 32,
    .edf_bld_wgt_mag = 32,
#endif //DS_BYPASS_EDS

#ifndef DS_BYPASS_CB
    .tdshp_cboost_lmt_u = 255,
    .tdshp_cboost_lmt_l = 128,
    .tdshp_cboost_en = 1,
    .tdshp_cboost_gain = 128,
    .tdshp_cboost_yconst = 16,
    .tdshp_cboost_yoffset_sel = 0,
    .tdshp_cboost_yoffset = 0,
#endif

#ifndef DS_BYPASS_POSTYLEV
    .tdshp_post_ylev_p000 = 0,
    .tdshp_post_ylev_p016 = 24,
    .tdshp_post_ylev_p032 = 48,
    .tdshp_post_ylev_p048 = 72,
    .tdshp_post_ylev_p064 = 96,
    .tdshp_post_ylev_p080 = 104,
    .tdshp_post_ylev_p096 = 112,
    .tdshp_post_ylev_p112 = 120,
    .tdshp_post_ylev_p128 = 128,
    .tdshp_post_ylev_p144 = 123,
    .tdshp_post_ylev_p160 = 118,
    .tdshp_post_ylev_p176 = 112,
    .tdshp_post_ylev_p192 = 107,
    .tdshp_post_ylev_p208 = 102,
    .tdshp_post_ylev_p224 = 96,
    .tdshp_post_ylev_p240 = 91,
    .tdshp_post_ylev_256  = 86,
    .tdshp_post_ylev_alpha = 16,
    .tdshp_post_ylev_en = 0,
#endif //DS_BYPASS_POSTYLEV
    },

    {// iDSHWRegEntry_8
    .tdshp_en = 1,
    .tdshp_bypass_mid = 0,
    .tdshp_bypass_high = 0,
    .tdshp_ink_sel = 0,
    .tdshp_gain_mid = 16,
    .tdshp_gain_high = 32,
    .tdshp_softcoring_gain = 16,

    .tdshp_coring_thr = 4,
    .tdshp_coring_zero = 2,
    .tdshp_gain = 32,
    .tdshp_limit_ratio = 8,
    .tdshp_limit = 24,
    .tdshp_bound = 48,
    .tdshp_coring_value = 3,

    .tdshp_clip_en = 1,
    .tdshp_clip_ratio = 8,
    .tdshp_clip_thr = 2,
    .tdshp_ac_lpf_coe = 8,
    .tdshp_sat_proc = 10,

    .tdshp_ylev_p000 = 0,
    .tdshp_ylev_p016 = 24,
    .tdshp_ylev_p032 = 48,
    .tdshp_ylev_p048 = 72,
    .tdshp_ylev_p064 = 96,
    .tdshp_ylev_p080 = 104,
    .tdshp_ylev_p096 = 112,
    .tdshp_ylev_p112 = 120,
    .tdshp_ylev_p128 = 128,
    .tdshp_ylev_p144 = 123,
    .tdshp_ylev_p160 = 118,
    .tdshp_ylev_p176 = 112,
    .tdshp_ylev_p192 = 107,
    .tdshp_ylev_p208 = 102,
    .tdshp_ylev_p224 = 96,
    .tdshp_ylev_p240 = 91,
    .tdshp_ylev_256  = 86,
    .tdshp_ylev_alpha = 16,
    .tdshp_ylev_en = 1,


    .pbc1_en = 1,
    .pbc1_lpf_en = 0,
    .pbc1_gain = 48,
    .pbc1_rslope_1 = 39,
    .pbc1_theta_r = 12,
    .pbc1_radius_r = 12,
    .pbc1_theta_c = 127,
    .pbc1_radius_c = 26,
    .pbc1_tslope = 85,
    .pbc1_lpf_gain = 16,
    .pbc1_rslope = 85,
    .pbc1_conf_gain = 1,
    .pbc1_edge_en = 0,
    .pbc1_edge_thr = 12,
    .pbc1_edge_slope = 32,


    .pbc2_en = 1,
    .pbc2_lpf_en = 1,
    .pbc2_gain = 16,
    .pbc2_rslope_1 = 32,
    .pbc2_theta_r = 24,
    .pbc2_radius_r = 24,
    .pbc2_theta_c = 96,
    .pbc2_radius_c = 32,
    .pbc2_tslope = 43,
    .pbc2_lpf_gain = 16,
    .pbc2_rslope = 43,
    .pbc2_conf_gain = 5,
    .pbc2_edge_en = 1,
    .pbc2_edge_thr = 12,
    .pbc2_edge_slope = 32,

    .pbc3_en = 1,
    .pbc3_lpf_en = 0,
    .pbc3_gain = 36,
    .pbc3_rslope_1 = 17,
    .pbc3_theta_r = 16,
    .pbc3_radius_r = 32,
    .pbc3_theta_c = 224,
    .pbc3_radius_c = 60,
    .pbc3_tslope = 64,
    .pbc3_lpf_gain = 16,
    .pbc3_rslope = 32,
    .pbc3_conf_gain = 1,
    .pbc3_edge_en = 0,
    .pbc3_edge_thr = 12,
    .pbc3_edge_slope = 32,

#ifndef DS_BYPASS_2DGC
    .tdshp_mid_coring_thr = 4,
    .tdshp_mid_coring_zero = 2,
    .tdshp_mid_softlimit_ratio = 8,
    .tdshp_mid_limit = 24,
    .tdshp_mid_bound = 48,
    .tdshp_mid_coring_value = 3,
    .tdshp_mid_softcoring_gain = 16,

    .tdshp_high_coring_thr = 4,
    .tdshp_high_coring_zero = 2,
    .tdshp_high_softlimit_ratio = 8,
    .tdshp_high_limit = 24,
    .tdshp_high_bound = 48,
    .tdshp_high_coring_value = 3,
    .tdshp_high_softcoring_gain = 16,
#endif //DS_BYPASS_2DGC

#ifndef DS_BYPASS_EDS
    .edf_gain_en = 1,
    .edf_flat_gain = 0,
    .edf_detail_gain = 64,
    .edf_edge_gain = 16,
    .edf_clip_ratio_inc = 2,

    .edf_flat_th = 16,
    .edf_detail_rise_th = 16,
    .edf_detail_fall_th = 240,
    .edf_edge_th = 240,

    .edf_flat_slope = 4,
    .edf_detail_rise_slope = 5,
    .edf_detail_fall_slope = 10,
    .edf_edge_slope = 10,

    .edf_edge_mag_th = 0,
    .edf_edge_mag_slope = 8,
    .edf_edge_mono_th = 48,
    .edf_edge_mono_slope = 7,

    .edf_edge_trend_th = 32,
    .edf_edge_trend_slope = 16,
    .edf_edge_trend_flat_mag = 8,

    .edf_bld_wgt_trend = 32,
    .edf_bld_wgt_mono = 32,
    .edf_bld_wgt_mag = 32,
#endif //DS_BYPASS_EDS

#ifndef DS_BYPASS_CB
    .tdshp_cboost_lmt_u = 255,
    .tdshp_cboost_lmt_l = 128,
    .tdshp_cboost_en = 1,
    .tdshp_cboost_gain = 128,
    .tdshp_cboost_yconst = 16,
    .tdshp_cboost_yoffset_sel = 0,
    .tdshp_cboost_yoffset = 0,
#endif

#ifndef DS_BYPASS_POSTYLEV
    .tdshp_post_ylev_p000 = 0,
    .tdshp_post_ylev_p016 = 24,
    .tdshp_post_ylev_p032 = 48,
    .tdshp_post_ylev_p048 = 72,
    .tdshp_post_ylev_p064 = 96,
    .tdshp_post_ylev_p080 = 104,
    .tdshp_post_ylev_p096 = 112,
    .tdshp_post_ylev_p112 = 120,
    .tdshp_post_ylev_p128 = 128,
    .tdshp_post_ylev_p144 = 123,
    .tdshp_post_ylev_p160 = 118,
    .tdshp_post_ylev_p176 = 112,
    .tdshp_post_ylev_p192 = 107,
    .tdshp_post_ylev_p208 = 102,
    .tdshp_post_ylev_p224 = 96,
    .tdshp_post_ylev_p240 = 91,
    .tdshp_post_ylev_256  = 86,
    .tdshp_post_ylev_alpha = 16,
    .tdshp_post_ylev_en = 0,
#endif //DS_BYPASS_POSTYLEV
    },

    {// iDSHWRegEntry_9
    .tdshp_en = 1,
    .tdshp_bypass_mid = 0,
    .tdshp_bypass_high = 0,
    .tdshp_ink_sel = 0,
    .tdshp_gain_mid = 16,
    .tdshp_gain_high = 32,
    .tdshp_softcoring_gain = 16,

    .tdshp_coring_thr = 4,
    .tdshp_coring_zero = 2,
    .tdshp_gain = 32,
    .tdshp_limit_ratio = 8,
    .tdshp_limit = 24,
    .tdshp_bound = 48,
    .tdshp_coring_value = 3,

    .tdshp_clip_en = 1,
    .tdshp_clip_ratio = 8,
    .tdshp_clip_thr = 2,
    .tdshp_ac_lpf_coe = 8,
    .tdshp_sat_proc = 10,

    .tdshp_ylev_p000 = 0,
    .tdshp_ylev_p016 = 24,
    .tdshp_ylev_p032 = 48,
    .tdshp_ylev_p048 = 72,
    .tdshp_ylev_p064 = 96,
    .tdshp_ylev_p080 = 104,
    .tdshp_ylev_p096 = 112,
    .tdshp_ylev_p112 = 120,
    .tdshp_ylev_p128 = 128,
    .tdshp_ylev_p144 = 123,
    .tdshp_ylev_p160 = 118,
    .tdshp_ylev_p176 = 112,
    .tdshp_ylev_p192 = 107,
    .tdshp_ylev_p208 = 102,
    .tdshp_ylev_p224 = 96,
    .tdshp_ylev_p240 = 91,
    .tdshp_ylev_256  = 86,
    .tdshp_ylev_alpha = 16,
    .tdshp_ylev_en = 1,


    .pbc1_en = 1,
    .pbc1_lpf_en = 0,
    .pbc1_gain = 48,
    .pbc1_rslope_1 = 39,
    .pbc1_theta_r = 12,
    .pbc1_radius_r = 12,
    .pbc1_theta_c = 127,
    .pbc1_radius_c = 26,
    .pbc1_tslope = 85,
    .pbc1_lpf_gain = 16,
    .pbc1_rslope = 85,
    .pbc1_conf_gain = 1,
    .pbc1_edge_en = 0,
    .pbc1_edge_thr = 12,
    .pbc1_edge_slope = 32,


    .pbc2_en = 1,
    .pbc2_lpf_en = 1,
    .pbc2_gain = 16,
    .pbc2_rslope_1 = 32,
    .pbc2_theta_r = 24,
    .pbc2_radius_r = 24,
    .pbc2_theta_c = 96,
    .pbc2_radius_c = 32,
    .pbc2_tslope = 43,
    .pbc2_lpf_gain = 16,
    .pbc2_rslope = 43,
    .pbc2_conf_gain = 5,
    .pbc2_edge_en = 1,
    .pbc2_edge_thr = 12,
    .pbc2_edge_slope = 32,

    .pbc3_en = 1,
    .pbc3_lpf_en = 0,
    .pbc3_gain = 36,
    .pbc3_rslope_1 = 17,
    .pbc3_theta_r = 16,
    .pbc3_radius_r = 32,
    .pbc3_theta_c = 224,
    .pbc3_radius_c = 60,
    .pbc3_tslope = 64,
    .pbc3_lpf_gain = 16,
    .pbc3_rslope = 32,
    .pbc3_conf_gain = 1,
    .pbc3_edge_en = 0,
    .pbc3_edge_thr = 12,
    .pbc3_edge_slope = 32,

#ifndef DS_BYPASS_2DGC
    .tdshp_mid_coring_thr = 4,
    .tdshp_mid_coring_zero = 2,
    .tdshp_mid_softlimit_ratio = 8,
    .tdshp_mid_limit = 24,
    .tdshp_mid_bound = 48,
    .tdshp_mid_coring_value = 3,
    .tdshp_mid_softcoring_gain = 16,

    .tdshp_high_coring_thr = 4,
    .tdshp_high_coring_zero = 2,
    .tdshp_high_softlimit_ratio = 8,
    .tdshp_high_limit = 24,
    .tdshp_high_bound = 48,
    .tdshp_high_coring_value = 3,
    .tdshp_high_softcoring_gain = 16,
#endif //DS_BYPASS_2DGC

#ifndef DS_BYPASS_EDS
    .edf_gain_en = 1,
    .edf_flat_gain = 0,
    .edf_detail_gain = 64,
    .edf_edge_gain = 16,
    .edf_clip_ratio_inc = 2,

    .edf_flat_th = 16,
    .edf_detail_rise_th = 16,
    .edf_detail_fall_th = 240,
    .edf_edge_th = 240,

    .edf_flat_slope = 4,
    .edf_detail_rise_slope = 5,
    .edf_detail_fall_slope = 10,
    .edf_edge_slope = 10,

    .edf_edge_mag_th = 0,
    .edf_edge_mag_slope = 8,
    .edf_edge_mono_th = 48,
    .edf_edge_mono_slope = 7,

    .edf_edge_trend_th = 32,
    .edf_edge_trend_slope = 16,
    .edf_edge_trend_flat_mag = 8,

    .edf_bld_wgt_trend = 32,
    .edf_bld_wgt_mono = 32,
    .edf_bld_wgt_mag = 32,
#endif //DS_BYPASS_EDS

#ifndef DS_BYPASS_CB
    .tdshp_cboost_lmt_u = 255,
    .tdshp_cboost_lmt_l = 128,
    .tdshp_cboost_en = 1,
    .tdshp_cboost_gain = 128,
    .tdshp_cboost_yconst = 16,
    .tdshp_cboost_yoffset_sel = 0,
    .tdshp_cboost_yoffset = 0,
#endif

#ifndef DS_BYPASS_POSTYLEV
    .tdshp_post_ylev_p000 = 0,
    .tdshp_post_ylev_p016 = 24,
    .tdshp_post_ylev_p032 = 48,
    .tdshp_post_ylev_p048 = 72,
    .tdshp_post_ylev_p064 = 96,
    .tdshp_post_ylev_p080 = 104,
    .tdshp_post_ylev_p096 = 112,
    .tdshp_post_ylev_p112 = 120,
    .tdshp_post_ylev_p128 = 128,
    .tdshp_post_ylev_p144 = 123,
    .tdshp_post_ylev_p160 = 118,
    .tdshp_post_ylev_p176 = 112,
    .tdshp_post_ylev_p192 = 107,
    .tdshp_post_ylev_p208 = 102,
    .tdshp_post_ylev_p224 = 96,
    .tdshp_post_ylev_p240 = 91,
    .tdshp_post_ylev_256  = 86,
    .tdshp_post_ylev_alpha = 16,
    .tdshp_post_ylev_en = 0,
#endif //DS_BYPASS_POSTYLEV
    },

    {// iDSHWRegEntry_10
    .tdshp_en = 1,
    .tdshp_bypass_mid = 0,
    .tdshp_bypass_high = 0,
    .tdshp_ink_sel = 0,
    .tdshp_gain_mid = 16,
    .tdshp_gain_high = 32,
    .tdshp_softcoring_gain = 16,

    .tdshp_coring_thr = 4,
    .tdshp_coring_zero = 2,
    .tdshp_gain = 32,
    .tdshp_limit_ratio = 8,
    .tdshp_limit = 24,
    .tdshp_bound = 48,
    .tdshp_coring_value = 3,

    .tdshp_clip_en = 1,
    .tdshp_clip_ratio = 8,
    .tdshp_clip_thr = 2,
    .tdshp_ac_lpf_coe = 8,
    .tdshp_sat_proc = 10,

    .tdshp_ylev_p000 = 0,
    .tdshp_ylev_p016 = 24,
    .tdshp_ylev_p032 = 48,
    .tdshp_ylev_p048 = 72,
    .tdshp_ylev_p064 = 96,
    .tdshp_ylev_p080 = 104,
    .tdshp_ylev_p096 = 112,
    .tdshp_ylev_p112 = 120,
    .tdshp_ylev_p128 = 128,
    .tdshp_ylev_p144 = 123,
    .tdshp_ylev_p160 = 118,
    .tdshp_ylev_p176 = 112,
    .tdshp_ylev_p192 = 107,
    .tdshp_ylev_p208 = 102,
    .tdshp_ylev_p224 = 96,
    .tdshp_ylev_p240 = 91,
    .tdshp_ylev_256  = 86,
    .tdshp_ylev_alpha = 16,
    .tdshp_ylev_en = 1,


    .pbc1_en = 1,
    .pbc1_lpf_en = 0,
    .pbc1_gain = 48,
    .pbc1_rslope_1 = 39,
    .pbc1_theta_r = 12,
    .pbc1_radius_r = 12,
    .pbc1_theta_c = 127,
    .pbc1_radius_c = 26,
    .pbc1_tslope = 85,
    .pbc1_lpf_gain = 16,
    .pbc1_rslope = 85,
    .pbc1_conf_gain = 1,
    .pbc1_edge_en = 0,
    .pbc1_edge_thr = 12,
    .pbc1_edge_slope = 32,


    .pbc2_en = 1,
    .pbc2_lpf_en = 1,
    .pbc2_gain = 16,
    .pbc2_rslope_1 = 32,
    .pbc2_theta_r = 24,
    .pbc2_radius_r = 24,
    .pbc2_theta_c = 96,
    .pbc2_radius_c = 32,
    .pbc2_tslope = 43,
    .pbc2_lpf_gain = 16,
    .pbc2_rslope = 43,
    .pbc2_conf_gain = 5,
    .pbc2_edge_en = 1,
    .pbc2_edge_thr = 12,
    .pbc2_edge_slope = 32,

    .pbc3_en = 1,
    .pbc3_lpf_en = 0,
    .pbc3_gain = 36,
    .pbc3_rslope_1 = 17,
    .pbc3_theta_r = 16,
    .pbc3_radius_r = 32,
    .pbc3_theta_c = 224,
    .pbc3_radius_c = 60,
    .pbc3_tslope = 64,
    .pbc3_lpf_gain = 16,
    .pbc3_rslope = 32,
    .pbc3_conf_gain = 1,
    .pbc3_edge_en = 0,
    .pbc3_edge_thr = 12,
    .pbc3_edge_slope = 32,

#ifndef DS_BYPASS_2DGC
    .tdshp_mid_coring_thr = 4,
    .tdshp_mid_coring_zero = 2,
    .tdshp_mid_softlimit_ratio = 8,
    .tdshp_mid_limit = 24,
    .tdshp_mid_bound = 48,
    .tdshp_mid_coring_value = 3,
    .tdshp_mid_softcoring_gain = 16,

    .tdshp_high_coring_thr = 4,
    .tdshp_high_coring_zero = 2,
    .tdshp_high_softlimit_ratio = 8,
    .tdshp_high_limit = 24,
    .tdshp_high_bound = 48,
    .tdshp_high_coring_value = 3,
    .tdshp_high_softcoring_gain = 16,
#endif //DS_BYPASS_2DGC

#ifndef DS_BYPASS_EDS
    .edf_gain_en = 1,
    .edf_flat_gain = 0,
    .edf_detail_gain = 64,
    .edf_edge_gain = 16,
    .edf_clip_ratio_inc = 2,

    .edf_flat_th = 16,
    .edf_detail_rise_th = 16,
    .edf_detail_fall_th = 240,
    .edf_edge_th = 240,

    .edf_flat_slope = 4,
    .edf_detail_rise_slope = 5,
    .edf_detail_fall_slope = 10,
    .edf_edge_slope = 10,

    .edf_edge_mag_th = 0,
    .edf_edge_mag_slope = 8,
    .edf_edge_mono_th = 48,
    .edf_edge_mono_slope = 7,

    .edf_edge_trend_th = 32,
    .edf_edge_trend_slope = 16,
    .edf_edge_trend_flat_mag = 8,

    .edf_bld_wgt_trend = 32,
    .edf_bld_wgt_mono = 32,
    .edf_bld_wgt_mag = 32,
#endif //DS_BYPASS_EDS

#ifndef DS_BYPASS_CB
    .tdshp_cboost_lmt_u = 255,
    .tdshp_cboost_lmt_l = 128,
    .tdshp_cboost_en = 1,
    .tdshp_cboost_gain = 128,
    .tdshp_cboost_yconst = 16,
    .tdshp_cboost_yoffset_sel = 0,
    .tdshp_cboost_yoffset = 0,
#endif

#ifndef DS_BYPASS_POSTYLEV
    .tdshp_post_ylev_p000 = 0,
    .tdshp_post_ylev_p016 = 24,
    .tdshp_post_ylev_p032 = 48,
    .tdshp_post_ylev_p048 = 72,
    .tdshp_post_ylev_p064 = 96,
    .tdshp_post_ylev_p080 = 104,
    .tdshp_post_ylev_p096 = 112,
    .tdshp_post_ylev_p112 = 120,
    .tdshp_post_ylev_p128 = 128,
    .tdshp_post_ylev_p144 = 123,
    .tdshp_post_ylev_p160 = 118,
    .tdshp_post_ylev_p176 = 112,
    .tdshp_post_ylev_p192 = 107,
    .tdshp_post_ylev_p208 = 102,
    .tdshp_post_ylev_p224 = 96,
    .tdshp_post_ylev_p240 = 91,
    .tdshp_post_ylev_256  = 86,
    .tdshp_post_ylev_alpha = 16,
    .tdshp_post_ylev_en = 0,
#endif //DS_BYPASS_POSTYLEV
    }
};

}
