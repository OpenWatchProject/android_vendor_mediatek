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
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>
#include <cutils/log.h>
#include <string.h>
#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"
#include "camera_AE_PLineTable_ov8858mipiraw.h"
#include "camera_info_ov8858mipiraw.h"
#include <custom/aaa/AEPlinetable.h>
#include "camera_custom_tsf_tbl.h"

#define NVRAM_TUNING_PARAM_NUM  5341001

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
    .ISO_Env={
        20,
        {100, 200, 400, 800, 1200, 1600, 2000, 2400, 2800, 3200,
         4000, 4800, 5600, 6400, 7200, 8000, 8800, 9600, 11200, 12800},
        {150, 300, 600, 1000, 1400, 1800, 2200, 2600, 3000, 3600,
         4400, 5200, 6000, 6800, 7600, 8400, 9200, 10400, 12000, 12800}
    },
    .ISPPca= {
#include INCLUDE_FILENAME_ISP_PCA_PARAM
    },
    .ISPRegs={
#include INCLUDE_FILENAME_ISP_REGS_PARAM
    },
    .ISPMulitCCM={
      .Poly22={
            90525,      // i4R_AVG
            21197,      // i4R_STD
            108825,      // i4B_AVG
            26748,      // i4B_STD
                  0, // i4R_MAX
                   0, // i4R_MIN
                  0, // i4G_MAX
                   0, // i4G_MIN
                  0, // i4B_MAX
                   0, // i4B_MIN
                { // i4P00[9]
            10505000, -4790000, -585000, -1642500, 8872500, -2102500, -595000, -2750000, 8470000
                },
                { // i4P10[9]
            -1181342, 1020476, 162318, 201210, -453508, 255138, 364933, 105045, -457441
                },
                { // i4P01[9]
            -1210273, 891074, 316660, -804030, 361524, 445636, 536044, -962288, 430694
                },
                { // i4P20[9]
                0, 0, 0, 0, 0, 0, 0, 0, 0
                },
                { // i4P11[9]
                0, 0, 0, 0, 0, 0, 0, 0, 0
                },
                { // i4P02[9]
                0, 0, 0, 0, 0, 0, 0, 0, 0
                }

      },
      .AWBGain={
        // Strobe
        {
          512,    // i4R
          512,    // i4G
          512    // i4B
        },
        // A
        {
          637,    // i4R
            512,    // i4G
          1373    // i4B
        },
        // TL84
        {
          878,    // i4R
            512,    // i4G
          1062    // i4B
        },
        // CWF
        {
          957,    // i4R
            512,    // i4G
          1182    // i4B
        },
        // D65
        {
          1149,    // i4R
            512,    // i4G
          736    // i4B
        },
        // Reserved 1
        {
            512,    // i4R
            512,    // i4G
            512    // i4B
        },
        // Reserved 2
        {
            512,    // i4R
            512,    // i4G
            512    // i4B
        },
        // Reserved 3
        {
            512,    // i4R
            512,    // i4G
            512    // i4B
        }
      },
      .Weight={
        1, // Strobe
        1, // A
        1, // TL84
        1, // CWF
        1, // D65
        1, // Reserved 1
        1, // Reserved 2
        1  // Reserved 3
      }
    },
    .isp_ccm_ratio= 1.0,
    //bInvokeSmoothCCM
    .bInvokeSmoothCCM= MTRUE,
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
    },
    .rGmaParam=
    {
        {   // Normal Preview
            eISP_DYNAMIC_GMA_MODE,  // eGMAMode
            8,                  // i4LowContrastThr
            80,                  // i4LowContrastRatio
            3,                  // i4LowContrastSeg
            {
                {   // i4ContrastWeightingTbl
                    //  0   1   2    3    4    5    6    7    8    9    10
                        0,  0,  0,  10,  20,  40,  60,  80,  80,  80,  80
                },
                {   // i4LVWeightingTbl
                    //LV0   1   2   3   4   5   6   7   8   9   10   11   12   13   14   15   16   17   18   19
                        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  33,  66, 100, 100, 100, 100, 100, 100, 100, 100
                },
                {   // i4NightContrastWtTbl
                    //  0   1   2    3    4    5    6    7    8    9    10
                       60, 50, 40,  30,  20,  10,   0,   0,   0,   0,   0
                },
                {   // i4NightLVWtTbl
                    //LV0   1    2    3   4   5   6   7   8   9   10   11   12   13   14   15   16   17   18   19
                       50,  50,  50,  40, 40, 40, 30, 30,  0,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0
                }
            },
            {
                1,      // i4Enable
                1,      // i4WaitAEStable
                4       // i4Speed
            },
            {
                0,      // i4Enable
                2047,   // i4CenterPt
                50,     // i4LowPercent
                100000, // i4LowCurve100
                100000, // i4HighCurve100
                50,     // i4HighPercent
                100,    // i4SlopeH100
                100     // i4SlopeL100
            },
            {
                0       // rGMAFlare.i4Enable
            }
        },
        {   // Normal Video
            eISP_DYNAMIC_GMA_MODE,  // eGMAMode
            8,                  // i4LowContrastThr
            80,                  // i4LowContrastRatio
            3,                  // i4LowContrastSeg
            {
                {   // i4ContrastWeightingTbl
                    //  0   1   2    3    4    5    6    7    8    9    10
                        0,  0,  0,  10,  20,  40,  60,  80,  80,  80,  80
                },
                {   // i4LVWeightingTbl
                    //LV0   1   2   3   4   5   6   7   8   9   10   11   12   13   14   15   16   17   18   19
                        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  33,  66, 100, 100, 100, 100, 100, 100, 100, 100
                },
                {   // i4NightContrastWtTbl
                    //  0   1   2    3    4    5    6    7    8    9    10
                       60, 50, 40,  30,  20,  10,   0,   0,   0,   0,   0
                },
                {   // i4NightLVWtTbl
                    //LV0   1    2    3   4   5   6   7   8   9   10   11   12   13   14   15   16   17   18   19
                       50,  50,  50,  40, 40, 40, 30, 30,  0,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0
                }
            },
            {
                1,      // i4Enable
                1,      // i4WaitAEStable
                4       // i4Speed
            },
            {
                0,      // i4Enable
                2047,   // i4CenterPt
                50,     // i4LowPercent
                100000, // i4LowCurve100
                100000, // i4HighCurve100
                50,     // i4HighPercent
                100,    // i4SlopeH100
                100     // i4SlopeL100
            },
            {
                0       // rGMAFlare.i4Enable
            }
        },
        {   // Normal Capture
            eISP_DYNAMIC_GMA_MODE,  // eGMAMode
            8,                  // i4LowContrastThr
            80,                  // i4LowContrastRatio
            3,                  // i4LowContrastSeg
            {
                {   // i4ContrastWeightingTbl
                    //  0   1   2    3    4    5    6    7    8    9    10
                        0,  0,  0,  10,  20,  40,  60,  80,  80,  80,  80
                },
                {   // i4LVWeightingTbl
                    //LV0   1   2   3   4   5   6   7   8   9   10   11   12   13   14   15   16   17   18   19
                        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  33,  66, 100, 100, 100, 100, 100, 100, 100, 100
                },
                {   // i4NightContrastWtTbl
                    //  0   1   2    3    4    5    6    7    8    9    10
                       60, 50, 40,  30,  20,  10,   0,   0,   0,   0,   0
                },
                {   // i4NightLVWtTbl
                    //LV0   1    2    3   4   5   6   7   8   9   10   11   12   13   14   15   16   17   18   19
                       50,  50,  50,  40, 40, 40, 30, 30,  0,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0
                }
            },
            {
                1,      // i4Enable
                1,      // i4WaitAEStable
                4       // i4Speed
            },
            {
                0,      // i4Enable
                2047,   // i4CenterPt
                50,     // i4LowPercent
                100000, // i4LowCurve100
                100000, // i4HighCurve100
                50,     // i4HighPercent
                100,    // i4SlopeH100
                100     // i4SlopeL100
            },
            {
                0       // rGMAFlare.i4Enable
            }
        },
        {   // HDRon Preview
            eISP_DYNAMIC_GMA_MODE,  // eGMAMode
            8,                  // i4LowContrastThr
            80,                  // i4LowContrastRatio
            3,                  // i4LowContrastSeg
            {
                {   // i4ContrastWeightingTbl
                    //  0   1   2    3    4    5    6    7    8    9    10
                        0,  0,  0,  10,  20,  40,  60,  80,  80,  80,  80
                },
                {   // i4LVWeightingTbl
                    //LV0   1   2   3   4   5   6   7   8   9   10   11   12   13   14   15   16   17   18   19
                        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  33,  66, 100, 100, 100, 100, 100, 100, 100, 100
                },
                {   // i4NightContrastWtTbl
                    //  0   1   2    3    4    5    6    7    8    9    10
                       60, 50, 40,  30,  20,  10,   0,   0,   0,   0,   0
                },
                {   // i4NightLVWtTbl
                    //LV0   1    2    3   4   5   6   7   8   9   10   11   12   13   14   15   16   17   18   19
                       50,  50,  50,  40, 40, 40, 30, 30,  0,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0
                }
            },
            {
                1,      // i4Enable
                1,      // i4WaitAEStable
                4       // i4Speed
            },
            {
                0,      // i4Enable
                2047,   // i4CenterPt
                50,     // i4LowPercent
                100000, // i4LowCurve100
                100000, // i4HighCurve100
                50,     // i4HighPercent
                100,    // i4SlopeH100
                100     // i4SlopeL100
            },
            {
                0       // rGMAFlare.i4Enable
            }
        },
        {   // HDRon Video
            eISP_DYNAMIC_GMA_MODE,  // eGMAMode
            8,                  // i4LowContrastThr
            80,                  // i4LowContrastRatio
            3,                  // i4LowContrastSeg
            {
                {   // i4ContrastWeightingTbl
                    //  0   1   2    3    4    5    6    7    8    9    10
                        0,  0,  0,  10,  20,  40,  60,  80,  80,  80,  80
                },
                {   // i4LVWeightingTbl
                    //LV0   1   2   3   4   5   6   7   8   9   10   11   12   13   14   15   16   17   18   19
                        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  33,  66, 100, 100, 100, 100, 100, 100, 100, 100
                },
                {   // i4NightContrastWtTbl
                    //  0   1   2    3    4    5    6    7    8    9    10
                       60, 50, 40,  30,  20,  10,   0,   0,   0,   0,   0
                },
                {   // i4NightLVWtTbl
                    //LV0   1    2    3   4   5   6   7   8   9   10   11   12   13   14   15   16   17   18   19
                       50,  50,  50,  40, 40, 40, 30, 30,  0,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0
                }
            },
            {
                1,      // i4Enable
                1,      // i4WaitAEStable
                4       // i4Speed
            },
            {
                0,      // i4Enable
                2047,   // i4CenterPt
                50,     // i4LowPercent
                100000, // i4LowCurve100
                100000, // i4HighCurve100
                50,     // i4HighPercent
                100,    // i4SlopeH100
                100     // i4SlopeL100
            },
            {
                0       // rGMAFlare.i4Enable
            }
        },
        {   // HDRon Capture
            eISP_DYNAMIC_GMA_MODE,  // eGMAMode
            8,                  // i4LowContrastThr
            80,                  // i4LowContrastRatio
            3,                  // i4LowContrastSeg
            {
                {   // i4ContrastWeightingTbl
                    //  0   1   2    3    4    5    6    7    8    9    10
                        0,  0,  0,  10,  20,  40,  60,  80,  80,  80,  80
                },
                {   // i4LVWeightingTbl
                    //LV0   1   2   3   4   5   6   7   8   9   10   11   12   13   14   15   16   17   18   19
                        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  33,  66, 100, 100, 100, 100, 100, 100, 100, 100
                },
                {   // i4NightContrastWtTbl
                    //  0   1   2    3    4    5    6    7    8    9    10
                       60, 50, 40,  30,  20,  10,   0,   0,   0,   0,   0
                },
                {   // i4NightLVWtTbl
                    //LV0   1    2    3   4   5   6   7   8   9   10   11   12   13   14   15   16   17   18   19
                       50,  50,  50,  40, 40, 40, 30, 30,  0,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0
                }
            },
            {
                1,      // i4Enable
                1,      // i4WaitAEStable
                4       // i4Speed
            },
            {
                0,      // i4Enable
                2047,   // i4CenterPt
                50,     // i4LowPercent
                100000, // i4LowCurve100
                100000, // i4HighCurve100
                50,     // i4HighPercent
                100,    // i4SlopeH100
                100     // i4SlopeL100
            },
            {
                0       // rGMAFlare.i4Enable
            }
        },
        {   // AutoHDR Preview
            eISP_DYNAMIC_GMA_MODE,  // eGMAMode
            8,                  // i4LowContrastThr
            80,                  // i4LowContrastRatio
            3,                  // i4LowContrastSeg
            {
                {   // i4ContrastWeightingTbl
                    //  0   1   2    3    4    5    6    7    8    9    10
                        0,  0,  0,  10,  20,  40,  60,  80,  80,  80,  80
                },
                {   // i4LVWeightingTbl
                    //LV0   1   2   3   4   5   6   7   8   9   10   11   12   13   14   15   16   17   18   19
                        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  33,  66, 100, 100, 100, 100, 100, 100, 100, 100
                },
                {   // i4NightContrastWtTbl
                    //  0   1   2    3    4    5    6    7    8    9    10
                       60, 50, 40,  30,  20,  10,   0,   0,   0,   0,   0
                },
                {   // i4NightLVWtTbl
                    //LV0   1    2    3   4   5   6   7   8   9   10   11   12   13   14   15   16   17   18   19
                       50,  50,  50,  40, 40, 40, 30, 30,  0,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0
                }
            },
            {
                1,      // i4Enable
                1,      // i4WaitAEStable
                4       // i4Speed
            },
            {
                0,      // i4Enable
                2047,   // i4CenterPt
                50,     // i4LowPercent
                100000, // i4LowCurve100
                100000, // i4HighCurve100
                50,     // i4HighPercent
                100,    // i4SlopeH100
                100     // i4SlopeL100
            },
            {
                0       // rGMAFlare.i4Enable
            }
        },
        {   // AutoHDR Video
            eISP_DYNAMIC_GMA_MODE,  // eGMAMode
            8,                  // i4LowContrastThr
            80,                  // i4LowContrastRatio
            3,                  // i4LowContrastSeg
            {
                {   // i4ContrastWeightingTbl
                    //  0   1   2    3    4    5    6    7    8    9    10
                        0,  0,  0,  10,  20,  40,  60,  80,  80,  80,  80
                },
                {   // i4LVWeightingTbl
                    //LV0   1   2   3   4   5   6   7   8   9   10   11   12   13   14   15   16   17   18   19
                        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  33,  66, 100, 100, 100, 100, 100, 100, 100, 100
                },
                {   // i4NightContrastWtTbl
                    //  0   1   2    3    4    5    6    7    8    9    10
                       60, 50, 40,  30,  20,  10,   0,   0,   0,   0,   0
                },
                {   // i4NightLVWtTbl
                    //LV0   1    2    3   4   5   6   7   8   9   10   11   12   13   14   15   16   17   18   19
                       50,  50,  50,  40, 40, 40, 30, 30,  0,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0
                }
            },
            {
                1,      // i4Enable
                1,      // i4WaitAEStable
                4       // i4Speed
            },
            {
                0,      // i4Enable
                2047,   // i4CenterPt
                50,     // i4LowPercent
                100000, // i4LowCurve100
                100000, // i4HighCurve100
                50,     // i4HighPercent
                100,    // i4SlopeH100
                100     // i4SlopeL100
            },
            {
                0       // rGMAFlare.i4Enable
            }
        },
        {   // AutoHDR Capture
            eISP_DYNAMIC_GMA_MODE,  // eGMAMode
            8,                  // i4LowContrastThr
            80,                  // i4LowContrastRatio
            3,                  // i4LowContrastSeg
            {
                {   // i4ContrastWeightingTbl
                    //  0   1   2    3    4    5    6    7    8    9    10
                        0,  0,  0,  10,  20,  40,  60,  80,  80,  80,  80
                },
                {   // i4LVWeightingTbl
                    //LV0   1   2   3   4   5   6   7   8   9   10   11   12   13   14   15   16   17   18   19
                        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  33,  66, 100, 100, 100, 100, 100, 100, 100, 100
                },
                {   // i4NightContrastWtTbl
                    //  0   1   2    3    4    5    6    7    8    9    10
                       60, 50, 40,  30,  20,  10,   0,   0,   0,   0,   0
                },
                {   // i4NightLVWtTbl
                    //LV0   1    2    3   4   5   6   7   8   9   10   11   12   13   14   15   16   17   18   19
                       50,  50,  50,  40, 40, 40, 30, 30,  0,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0
                }
            },
            {
                1,      // i4Enable
                1,      // i4WaitAEStable
                4       // i4Speed
            },
            {
                0,      // i4Enable
                2047,   // i4CenterPt
                50,     // i4LowPercent
                100000, // i4LowCurve100
                100000, // i4HighCurve100
                50,     // i4HighPercent
                100,    // i4SlopeH100
                100     // i4SlopeL100
            },
            {
                0       // rGMAFlare.i4Enable
            }
        },
        {   // Reserved 0
            eISP_DYNAMIC_GMA_MODE,  // eGMAMode
            8,                  // i4LowContrastThr
            80,                  // i4LowContrastRatio
            3,                  // i4LowContrastSeg
            {
                {   // i4ContrastWeightingTbl
                    //  0   1   2    3    4    5    6    7    8    9    10
                        0,  0,  0,  10,  20,  40,  60,  80,  80,  80,  80
                },
                {   // i4LVWeightingTbl
                    //LV0   1   2   3   4   5   6   7   8   9   10   11   12   13   14   15   16   17   18   19
                        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  33,  66, 100, 100, 100, 100, 100, 100, 100, 100
                },
                {   // i4NightContrastWtTbl
                    //  0   1   2    3    4    5    6    7    8    9    10
                       60, 50, 40,  30,  20,  10,   0,   0,   0,   0,   0
                },
                {   // i4NightLVWtTbl
                    //LV0   1    2    3   4   5   6   7   8   9   10   11   12   13   14   15   16   17   18   19
                       50,  50,  50,  40, 40, 40, 30, 30,  0,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0
                }
            },
            {
                1,      // i4Enable
                1,      // i4WaitAEStable
                4       // i4Speed
            },
            {
                0,      // i4Enable
                2047,   // i4CenterPt
                50,     // i4LowPercent
                100000, // i4LowCurve100
                100000, // i4HighCurve100
                50,     // i4HighPercent
                100,    // i4SlopeH100
                100     // i4SlopeL100
            },
            {
                0       // rGMAFlare.i4Enable
            }
        },
        {   // Reserved 1
            eISP_DYNAMIC_GMA_MODE,  // eGMAMode
            8,                  // i4LowContrastThr
            80,                  // i4LowContrastRatio
            3,                  // i4LowContrastSeg
            {
                {   // i4ContrastWeightingTbl
                    //  0   1   2    3    4    5    6    7    8    9    10
                        0,  0,  0,  10,  20,  40,  60,  80,  80,  80,  80
                },
                {   // i4LVWeightingTbl
                    //LV0   1   2   3   4   5   6   7   8   9   10   11   12   13   14   15   16   17   18   19
                        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  33,  66, 100, 100, 100, 100, 100, 100, 100, 100
                },
                {   // i4NightContrastWtTbl
                    //  0   1   2    3    4    5    6    7    8    9    10
                       60, 50, 40,  30,  20,  10,   0,   0,   0,   0,   0
                },
                {   // i4NightLVWtTbl
                    //LV0   1    2    3   4   5   6   7   8   9   10   11   12   13   14   15   16   17   18   19
                       50,  50,  50,  40, 40, 40, 30, 30,  0,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0
                }
            },
            {
                1,      // i4Enable
                1,      // i4WaitAEStable
                4       // i4Speed
            },
            {
                0,      // i4Enable
                2047,   // i4CenterPt
                50,     // i4LowPercent
                100000, // i4LowCurve100
                100000, // i4HighCurve100
                50,     // i4HighPercent
                100,    // i4SlopeH100
                100     // i4SlopeL100
            },
            {
                0       // rGMAFlare.i4Enable
            }
        },
        {   // Reserved 2
            eISP_DYNAMIC_GMA_MODE,  // eGMAMode
            8,                  // i4LowContrastThr
            80,                  // i4LowContrastRatio
            3,                  // i4LowContrastSeg
            {
                {   // i4ContrastWeightingTbl
                    //  0   1   2    3    4    5    6    7    8    9    10
                        0,  0,  0,  10,  20,  40,  60,  80,  80,  80,  80
                },
                {   // i4LVWeightingTbl
                    //LV0   1   2   3   4   5   6   7   8   9   10   11   12   13   14   15   16   17   18   19
                        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  33,  66, 100, 100, 100, 100, 100, 100, 100, 100
                },
                {   // i4NightContrastWtTbl
                    //  0   1   2    3    4    5    6    7    8    9    10
                       60, 50, 40,  30,  20,  10,   0,   0,   0,   0,   0
                },
                {   // i4NightLVWtTbl
                    //LV0   1    2    3   4   5   6   7   8   9   10   11   12   13   14   15   16   17   18   19
                       50,  50,  50,  40, 40, 40, 30, 30,  0,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0
                }
            },
            {
                1,      // i4Enable
                1,      // i4WaitAEStable
                4       // i4Speed
            },
            {
                0,      // i4Enable
                2047,   // i4CenterPt
                50,     // i4LowPercent
                100000, // i4LowCurve100
                100000, // i4HighCurve100
                50,     // i4HighPercent
                100,    // i4SlopeH100
                100     // i4SlopeL100
            },
            {
                0       // rGMAFlare.i4Enable
            }
        }
    },
    .rLceParam=
    {
        0,  // i4AutoHDREnable
        {// rAutoLCEParam
            { //Normal Preview
                0,  //i4HistCCLB
                1600, //i4HistCCUB
                3,   //i4LCESeg
                600, //i4LCEContrastRatio
                25,  //i4LCEHistYThr
                51,  //SlopeLow --> 0.2*256
                307, //SlopeHigh --> 1.2*256
                384, //SlopeTop --> 1.5*256
                20,  //i4DetailRangeRatio --> base 1000
                60,  //i4CenSlopeMin

                0,    //i4vBlacKeep
                1700, //i4vFixedPt1
                2200, //i4vFixedPt2
                2800, //i4vFixedPt3
                3450, //i4vFixedPt4
                1000, //i4TCPLB
                20000,//I4TCPUB

                0,    //i4Reserved0
                0,    //i4Reserved1
                0,    //i4Reserved2
                0,    //i4Reserved3
                0,    //i4Reserved4

                { //rLCELUTs
                    {    //i4vTbl1 (for i4vFixedPt1)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {2150,  2150,  2200,  2200,  2200,  2200,  2200,  2250,  2250,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  0 * N
                         {2150,  2150,  2150,  2200,  2200,  2200,  2200,  2250,  2250,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  1
                         {2150,  2150,  2150,  2150,  2150,  2150,  2150,  2200,  2200,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  2
                         {2150,  2150,  2150,  2150,  2150,  2150,  2150,  2200,  2200,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  3
                         {2100,  2100,  2100,  2150,  2150,  2150,  2150,  2150,  2150,  2200,  2200,   2150,   2100,   2100,   2100,   2100,   2100,   2100,   2100},   //  4
                         {2100,  2100,  2100,  2100,  2100,  2100,  2100,  2100,  2100,  2150,  2150,   2100,   2100,   2100,   2100,   2100,   2100,   2100,   2100},   //  5
                         {2050,  2050,  2050,  2050,  2050,  2050,  2050,  2050,  2050,  2100,  2100,   2100,   2100,   2100,   2100,   2100,   2100,   2100,   2100},   //  6
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  2050,  2050,   2050,   2050,   2050,   2100,   2100,   2050,   2050,   2050},   //  7
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  2100,  2100,   1950,   1950,   1950,   2050,   2050,   1950,   1950,   1950},   //  8
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950},   //  9
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950}    // 10

                    },

                    {    //i4vTbl2 (for i4vFixedPt2)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  10 * N
                         {2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  1
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  2
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  3
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2500,   2600,   2600,   2600,   2600,   2600,   2600},   //  4
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2500,   2500,   2500,   2500,   2500,   2500,   2500},   //  5
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,   2500,   2500,   2500,   2500,   2500,   2500,   2500,   2500},   //  6
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,   2400,   2500,   2500,   2500,   2500,   2500,   2500,   2500},   //  7
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,   2400,   2400,   2400,   2400,   2400,   2400,   2400,   2400},   //  8
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300},   //  9
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300}    // 10

                    },

                    {    //i4vTbl3 (for i4vFixedPt3)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100},   //  10 * N
                         {3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100},   //  1
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  2
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  3
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  4
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  5
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  6
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,   2900,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  7
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,   2900,   2900,   2900,   2900,   2900,   2900,   2900,   2900},   //  8
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850},   //  9
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850}    // 10

                    },

                    {    //i4vTbl4 (fori4vFixedPt4)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  10 * N
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  1
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  2
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  3
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  4
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  5
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  6
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  7
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  8
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  9
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450}    // 10


                    },

                    {    //i4DetailWtTbl
                        //  0   1   2   3   4   5   6   7   8    9   10
                        //   50, 50, 60, 70, 80, 90, 90, 90, 90, 100, 100 //original
                           100,100,100,100,100,100,100,100,100,100,100 //fixed P test
                    },

                    {    //i4CenSlopeEnhance
                        //  0   1   2   3   4   5   6   7   8   9   10   11
                            0,  4,  8, 12, 16, 20, 16, 12,  8,  4,   0,   0
                    }

                },
                {  // rLCESmooth
                    1,      // i4Enable
                    0,      // i4WaitAEStable
                    4       // i4Speed
                },
                {
                    // rLCEFlare
                    0   // i4Enable
                }
            },
            { //Normal Video
                0,  //i4HistCCLB
                1600, //i4HistCCUB
                3,   //i4LCESeg
                600, //i4LCEContrastRatio
                25,  //i4LCEHistYThr
                51,  //SlopeLow --> 0.2*256
                307, //SlopeHigh --> 1.2*256
                384, //SlopeTop --> 1.5*256
                20,  //i4DetailRangeRatio --> base 1000
                60,  //i4CenSlopeMin

                0,    //i4vBlacKeep
                1700, //i4vFixedPt1
                2200, //i4vFixedPt2
                2800, //i4vFixedPt3
                3450, //i4vFixedPt4
                1000, //i4TCPLB
                20000,//I4TCPUB

                0,    //i4Reserved0
                0,    //i4Reserved1
                0,    //i4Reserved2
                0,    //i4Reserved3
                0,    //i4Reserved4

                { //rLCELUTs
                    {    //i4vTbl1 (for i4vFixedPt1)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {2150,  2150,  2200,  2200,  2200,  2200,  2200,  2250,  2250,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  0 * N
                         {2150,  2150,  2150,  2200,  2200,  2200,  2200,  2250,  2250,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  1
                         {2150,  2150,  2150,  2150,  2150,  2150,  2150,  2200,  2200,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  2
                         {2150,  2150,  2150,  2150,  2150,  2150,  2150,  2200,  2200,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  3
                         {2100,  2100,  2100,  2150,  2150,  2150,  2150,  2150,  2150,  2200,  2200,   2150,   2100,   2100,   2100,   2100,   2100,   2100,   2100},   //  4
                         {2100,  2100,  2100,  2100,  2100,  2100,  2100,  2100,  2100,  2150,  2150,   2100,   2100,   2100,   2100,   2100,   2100,   2100,   2100},   //  5
                         {2050,  2050,  2050,  2050,  2050,  2050,  2050,  2050,  2050,  2100,  2100,   2100,   2100,   2100,   2100,   2100,   2100,   2100,   2100},   //  6
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  2050,  2050,   2050,   2050,   2050,   2100,   2100,   2050,   2050,   2050},   //  7
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  2100,  2100,   1950,   1950,   1950,   2050,   2050,   1950,   1950,   1950},   //  8
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950},   //  9
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950}    // 10

                    },

                    {    //i4vTbl2 (for i4vFixedPt2)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  10 * N
                         {2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  1
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  2
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  3
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2500,   2600,   2600,   2600,   2600,   2600,   2600},   //  4
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2500,   2500,   2500,   2500,   2500,   2500,   2500},   //  5
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,   2500,   2500,   2500,   2500,   2500,   2500,   2500,   2500},   //  6
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,   2400,   2500,   2500,   2500,   2500,   2500,   2500,   2500},   //  7
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,   2400,   2400,   2400,   2400,   2400,   2400,   2400,   2400},   //  8
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300},   //  9
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300}    // 10

                    },

                    {    //i4vTbl3 (for i4vFixedPt3)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100},   //  10 * N
                         {3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100},   //  1
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  2
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  3
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  4
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  5
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  6
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,   2900,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  7
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,   2900,   2900,   2900,   2900,   2900,   2900,   2900,   2900},   //  8
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850},   //  9
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850}    // 10

                    },

                    {    //i4vTbl4 (fori4vFixedPt4)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  10 * N
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  1
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  2
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  3
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  4
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  5
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  6
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  7
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  8
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  9
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450}    // 10


                    },

                    {    //i4DetailWtTbl
                        //  0   1   2   3   4   5   6   7   8    9   10
                        //   50, 50, 60, 70, 80, 90, 90, 90, 90, 100, 100 //original
                           100,100,100,100,100,100,100,100,100,100,100 //fixed P test
                    },

                    {    //i4CenSlopeEnhance
                        //  0   1   2   3   4   5   6   7   8   9   10   11
                            0,  4,  8, 12, 16, 20, 16, 12,  8,  4,   0,   0
                    }

                },
                {  // rLCESmooth
                    1,      // i4Enable
                    0,      // i4WaitAEStable
                    4       // i4Speed
                },
                {
                    // rLCEFlare
                    0   // i4Enable
                }
            },
            { //Normal Capture
                0,  //i4HistCCLB
                1600, //i4HistCCUB
                3,   //i4LCESeg
                600, //i4LCEContrastRatio
                25,  //i4LCEHistYThr
                51,  //SlopeLow --> 0.2*256
                307, //SlopeHigh --> 1.2*256
                384, //SlopeTop --> 1.5*256
                20,  //i4DetailRangeRatio --> base 1000
                60,  //i4CenSlopeMin

                0,    //i4vBlacKeep
                1700, //i4vFixedPt1
                2200, //i4vFixedPt2
                2800, //i4vFixedPt3
                3450, //i4vFixedPt4
                1000, //i4TCPLB
                20000,//I4TCPUB

                0,    //i4Reserved0
                0,    //i4Reserved1
                0,    //i4Reserved2
                0,    //i4Reserved3
                0,    //i4Reserved4

                { //rLCELUTs
                    {    //i4vTbl1 (for i4vFixedPt1)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {2150,  2150,  2200,  2200,  2200,  2200,  2200,  2250,  2250,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  0 * N
                         {2150,  2150,  2150,  2200,  2200,  2200,  2200,  2250,  2250,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  1
                         {2150,  2150,  2150,  2150,  2150,  2150,  2150,  2200,  2200,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  2
                         {2150,  2150,  2150,  2150,  2150,  2150,  2150,  2200,  2200,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  3
                         {2100,  2100,  2100,  2150,  2150,  2150,  2150,  2150,  2150,  2200,  2200,   2150,   2100,   2100,   2100,   2100,   2100,   2100,   2100},   //  4
                         {2100,  2100,  2100,  2100,  2100,  2100,  2100,  2100,  2100,  2150,  2150,   2100,   2100,   2100,   2100,   2100,   2100,   2100,   2100},   //  5
                         {2050,  2050,  2050,  2050,  2050,  2050,  2050,  2050,  2050,  2100,  2100,   2100,   2100,   2100,   2100,   2100,   2100,   2100,   2100},   //  6
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  2050,  2050,   2050,   2050,   2050,   2100,   2100,   2050,   2050,   2050},   //  7
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  2100,  2100,   1950,   1950,   1950,   2050,   2050,   1950,   1950,   1950},   //  8
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950},   //  9
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950}    // 10

                    },

                    {    //i4vTbl2 (for i4vFixedPt2)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  10 * N
                         {2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  1
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  2
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  3
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2500,   2600,   2600,   2600,   2600,   2600,   2600},   //  4
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2500,   2500,   2500,   2500,   2500,   2500,   2500},   //  5
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,   2500,   2500,   2500,   2500,   2500,   2500,   2500,   2500},   //  6
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,   2400,   2500,   2500,   2500,   2500,   2500,   2500,   2500},   //  7
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,   2400,   2400,   2400,   2400,   2400,   2400,   2400,   2400},   //  8
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300},   //  9
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300}    // 10

                    },

                    {    //i4vTbl3 (for i4vFixedPt3)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100},   //  10 * N
                         {3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100},   //  1
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  2
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  3
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  4
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  5
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  6
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,   2900,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  7
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,   2900,   2900,   2900,   2900,   2900,   2900,   2900,   2900},   //  8
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850},   //  9
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850}    // 10

                    },

                    {    //i4vTbl4 (fori4vFixedPt4)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  10 * N
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  1
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  2
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  3
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  4
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  5
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  6
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  7
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  8
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  9
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450}    // 10


                    },

                    {    //i4DetailWtTbl
                        //  0   1   2   3   4   5   6   7   8    9   10
                        //   50, 50, 60, 70, 80, 90, 90, 90, 90, 100, 100 //original
                           100,100,100,100,100,100,100,100,100,100,100 //fixed P test
                    },

                    {    //i4CenSlopeEnhance
                        //  0   1   2   3   4   5   6   7   8   9   10   11
                            0,  4,  8, 12, 16, 20, 16, 12,  8,  4,   0,   0
                    }

                },
                {  // rLCESmooth
                    1,      // i4Enable
                    0,      // i4WaitAEStable
                    4       // i4Speed
                },
                {
                    // rLCEFlare
                    0   // i4Enable
                }
            },
            { //HDROn Preview
                0,  //i4HistCCLB
                1600, //i4HistCCUB
                3,   //i4LCESeg
                600, //i4LCEContrastRatio
                25,  //i4LCEHistYThr
                51,  //SlopeLow --> 0.2*256
                307, //SlopeHigh --> 1.2*256
                384, //SlopeTop --> 1.5*256
                20,  //i4DetailRangeRatio --> base 1000
                60,  //i4CenSlopeMin

                0,    //i4vBlacKeep
                1700, //i4vFixedPt1
                2200, //i4vFixedPt2
                2800, //i4vFixedPt3
                3450, //i4vFixedPt4
                1000, //i4TCPLB
                20000,//I4TCPUB

                0,    //i4Reserved0
                0,    //i4Reserved1
                0,    //i4Reserved2
                0,    //i4Reserved3
                0,    //i4Reserved4

                { //rLCELUTs
                    {    //i4vTbl1 (for i4vFixedPt1)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {2150,  2150,  2200,  2200,  2200,  2200,  2200,  2250,  2250,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  0 * N
                         {2150,  2150,  2150,  2200,  2200,  2200,  2200,  2250,  2250,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  1
                         {2150,  2150,  2150,  2150,  2150,  2150,  2150,  2200,  2200,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  2
                         {2150,  2150,  2150,  2150,  2150,  2150,  2150,  2200,  2200,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  3
                         {2100,  2100,  2100,  2150,  2150,  2150,  2150,  2150,  2150,  2200,  2200,   2150,   2100,   2100,   2100,   2100,   2100,   2100,   2100},   //  4
                         {2100,  2100,  2100,  2100,  2100,  2100,  2100,  2100,  2100,  2150,  2150,   2100,   2100,   2100,   2100,   2100,   2100,   2100,   2100},   //  5
                         {2050,  2050,  2050,  2050,  2050,  2050,  2050,  2050,  2050,  2100,  2100,   2100,   2100,   2100,   2100,   2100,   2100,   2100,   2100},   //  6
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  2050,  2050,   2050,   2050,   2050,   2100,   2100,   2050,   2050,   2050},   //  7
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  2100,  2100,   1950,   1950,   1950,   2050,   2050,   1950,   1950,   1950},   //  8
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950},   //  9
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950}    // 10

                    },

                    {    //i4vTbl2 (for i4vFixedPt2)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  10 * N
                         {2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  1
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  2
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  3
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2500,   2600,   2600,   2600,   2600,   2600,   2600},   //  4
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2500,   2500,   2500,   2500,   2500,   2500,   2500},   //  5
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,   2500,   2500,   2500,   2500,   2500,   2500,   2500,   2500},   //  6
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,   2400,   2500,   2500,   2500,   2500,   2500,   2500,   2500},   //  7
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,   2400,   2400,   2400,   2400,   2400,   2400,   2400,   2400},   //  8
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300},   //  9
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300}    // 10

                    },

                    {    //i4vTbl3 (for i4vFixedPt3)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100},   //  10 * N
                         {3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100},   //  1
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  2
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  3
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  4
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  5
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  6
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,   2900,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  7
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,   2900,   2900,   2900,   2900,   2900,   2900,   2900,   2900},   //  8
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850},   //  9
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850}    // 10

                    },

                    {    //i4vTbl4 (fori4vFixedPt4)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  10 * N
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  1
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  2
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  3
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  4
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  5
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  6
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  7
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  8
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  9
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450}    // 10


                    },

                    {    //i4DetailWtTbl
                        //  0   1   2   3   4   5   6   7   8    9   10
                        //   50, 50, 60, 70, 80, 90, 90, 90, 90, 100, 100 //original
                           100,100,100,100,100,100,100,100,100,100,100 //fixed P test
                    },

                    {    //i4CenSlopeEnhance
                        //  0   1   2   3   4   5   6   7   8   9   10   11
                            0,  4,  8, 12, 16, 20, 16, 12,  8,  4,   0,   0
                    }

                },
                {  // rLCESmooth
                    1,      // i4Enable
                    0,      // i4WaitAEStable
                    4       // i4Speed
                },
                {
                    // rLCEFlare
                    0   // i4Enable
                }
            },
            { //HDROn Video
                0,  //i4HistCCLB
                1600, //i4HistCCUB
                3,   //i4LCESeg
                600, //i4LCEContrastRatio
                25,  //i4LCEHistYThr
                51,  //SlopeLow --> 0.2*256
                307, //SlopeHigh --> 1.2*256
                384, //SlopeTop --> 1.5*256
                20,  //i4DetailRangeRatio --> base 1000
                60,  //i4CenSlopeMin

                0,    //i4vBlacKeep
                1700, //i4vFixedPt1
                2200, //i4vFixedPt2
                2800, //i4vFixedPt3
                3450, //i4vFixedPt4
                1000, //i4TCPLB
                20000,//I4TCPUB

                0,    //i4Reserved0
                0,    //i4Reserved1
                0,    //i4Reserved2
                0,    //i4Reserved3
                0,    //i4Reserved4

                { //rLCELUTs
                    {    //i4vTbl1 (for i4vFixedPt1)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {2150,  2150,  2200,  2200,  2200,  2200,  2200,  2250,  2250,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  0 * N
                         {2150,  2150,  2150,  2200,  2200,  2200,  2200,  2250,  2250,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  1
                         {2150,  2150,  2150,  2150,  2150,  2150,  2150,  2200,  2200,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  2
                         {2150,  2150,  2150,  2150,  2150,  2150,  2150,  2200,  2200,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  3
                         {2100,  2100,  2100,  2150,  2150,  2150,  2150,  2150,  2150,  2200,  2200,   2150,   2100,   2100,   2100,   2100,   2100,   2100,   2100},   //  4
                         {2100,  2100,  2100,  2100,  2100,  2100,  2100,  2100,  2100,  2150,  2150,   2100,   2100,   2100,   2100,   2100,   2100,   2100,   2100},   //  5
                         {2050,  2050,  2050,  2050,  2050,  2050,  2050,  2050,  2050,  2100,  2100,   2100,   2100,   2100,   2100,   2100,   2100,   2100,   2100},   //  6
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  2050,  2050,   2050,   2050,   2050,   2100,   2100,   2050,   2050,   2050},   //  7
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  2100,  2100,   1950,   1950,   1950,   2050,   2050,   1950,   1950,   1950},   //  8
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950},   //  9
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950}    // 10

                    },

                    {    //i4vTbl2 (for i4vFixedPt2)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  10 * N
                         {2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  1
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  2
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  3
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2500,   2600,   2600,   2600,   2600,   2600,   2600},   //  4
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2500,   2500,   2500,   2500,   2500,   2500,   2500},   //  5
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,   2500,   2500,   2500,   2500,   2500,   2500,   2500,   2500},   //  6
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,   2400,   2500,   2500,   2500,   2500,   2500,   2500,   2500},   //  7
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,   2400,   2400,   2400,   2400,   2400,   2400,   2400,   2400},   //  8
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300},   //  9
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300}    // 10

                    },

                    {    //i4vTbl3 (for i4vFixedPt3)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100},   //  10 * N
                         {3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100},   //  1
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  2
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  3
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  4
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  5
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  6
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,   2900,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  7
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,   2900,   2900,   2900,   2900,   2900,   2900,   2900,   2900},   //  8
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850},   //  9
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850}    // 10

                    },

                    {    //i4vTbl4 (fori4vFixedPt4)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  10 * N
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  1
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  2
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  3
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  4
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  5
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  6
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  7
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  8
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  9
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450}    // 10


                    },

                    {    //i4DetailWtTbl
                        //  0   1   2   3   4   5   6   7   8    9   10
                        //   50, 50, 60, 70, 80, 90, 90, 90, 90, 100, 100 //original
                           100,100,100,100,100,100,100,100,100,100,100 //fixed P test
                    },

                    {    //i4CenSlopeEnhance
                        //  0   1   2   3   4   5   6   7   8   9   10   11
                            0,  4,  8, 12, 16, 20, 16, 12,  8,  4,   0,   0
                    }

                },
                {  // rLCESmooth
                    1,      // i4Enable
                    0,      // i4WaitAEStable
                    4       // i4Speed
                },
                {
                    // rLCEFlare
                    0   // i4Enable
                }
            },
            { //HDROn Capture
                0,  //i4HistCCLB
                1600, //i4HistCCUB
                3,   //i4LCESeg
                600, //i4LCEContrastRatio
                25,  //i4LCEHistYThr
                51,  //SlopeLow --> 0.2*256
                307, //SlopeHigh --> 1.2*256
                384, //SlopeTop --> 1.5*256
                20,  //i4DetailRangeRatio --> base 1000
                60,  //i4CenSlopeMin

                0,    //i4vBlacKeep
                1700, //i4vFixedPt1
                2200, //i4vFixedPt2
                2800, //i4vFixedPt3
                3450, //i4vFixedPt4
                1000, //i4TCPLB
                20000,//I4TCPUB

                0,    //i4Reserved0
                0,    //i4Reserved1
                0,    //i4Reserved2
                0,    //i4Reserved3
                0,    //i4Reserved4

                { //rLCELUTs
                    {    //i4vTbl1 (for i4vFixedPt1)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {2150,  2150,  2200,  2200,  2200,  2200,  2200,  2250,  2250,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  0 * N
                         {2150,  2150,  2150,  2200,  2200,  2200,  2200,  2250,  2250,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  1
                         {2150,  2150,  2150,  2150,  2150,  2150,  2150,  2200,  2200,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  2
                         {2150,  2150,  2150,  2150,  2150,  2150,  2150,  2200,  2200,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  3
                         {2100,  2100,  2100,  2150,  2150,  2150,  2150,  2150,  2150,  2200,  2200,   2150,   2100,   2100,   2100,   2100,   2100,   2100,   2100},   //  4
                         {2100,  2100,  2100,  2100,  2100,  2100,  2100,  2100,  2100,  2150,  2150,   2100,   2100,   2100,   2100,   2100,   2100,   2100,   2100},   //  5
                         {2050,  2050,  2050,  2050,  2050,  2050,  2050,  2050,  2050,  2100,  2100,   2100,   2100,   2100,   2100,   2100,   2100,   2100,   2100},   //  6
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  2050,  2050,   2050,   2050,   2050,   2100,   2100,   2050,   2050,   2050},   //  7
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  2100,  2100,   1950,   1950,   1950,   2050,   2050,   1950,   1950,   1950},   //  8
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950},   //  9
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950}    // 10

                    },

                    {    //i4vTbl2 (for i4vFixedPt2)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  10 * N
                         {2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  1
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  2
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  3
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2500,   2600,   2600,   2600,   2600,   2600,   2600},   //  4
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2500,   2500,   2500,   2500,   2500,   2500,   2500},   //  5
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,   2500,   2500,   2500,   2500,   2500,   2500,   2500,   2500},   //  6
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,   2400,   2500,   2500,   2500,   2500,   2500,   2500,   2500},   //  7
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,   2400,   2400,   2400,   2400,   2400,   2400,   2400,   2400},   //  8
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300},   //  9
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300}    // 10

                    },

                    {    //i4vTbl3 (for i4vFixedPt3)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100},   //  10 * N
                         {3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100},   //  1
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  2
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  3
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  4
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  5
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  6
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,   2900,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  7
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,   2900,   2900,   2900,   2900,   2900,   2900,   2900,   2900},   //  8
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850},   //  9
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850}    // 10

                    },

                    {    //i4vTbl4 (fori4vFixedPt4)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  10 * N
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  1
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  2
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  3
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  4
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  5
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  6
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  7
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  8
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  9
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450}    // 10


                    },

                    {    //i4DetailWtTbl
                        //  0   1   2   3   4   5   6   7   8    9   10
                        //   50, 50, 60, 70, 80, 90, 90, 90, 90, 100, 100 //original
                           100,100,100,100,100,100,100,100,100,100,100 //fixed P test
                    },

                    {    //i4CenSlopeEnhance
                        //  0   1   2   3   4   5   6   7   8   9   10   11
                            0,  4,  8, 12, 16, 20, 16, 12,  8,  4,   0,   0
                    }

                },
                {  // rLCESmooth
                    1,      // i4Enable
                    0,      // i4WaitAEStable
                    4       // i4Speed
                },
                {
                    // rLCEFlare
                    0   // i4Enable
                }
            },
            { //AutoHDR Preview
                0,  //i4HistCCLB
                1600, //i4HistCCUB
                3,   //i4LCESeg
                600, //i4LCEContrastRatio
                25,  //i4LCEHistYThr
                51,  //SlopeLow --> 0.2*256
                307, //SlopeHigh --> 1.2*256
                384, //SlopeTop --> 1.5*256
                20,  //i4DetailRangeRatio --> base 1000
                60,  //i4CenSlopeMin

                0,    //i4vBlacKeep
                1700, //i4vFixedPt1
                2200, //i4vFixedPt2
                2800, //i4vFixedPt3
                3450, //i4vFixedPt4
                1000, //i4TCPLB
                20000,//I4TCPUB

                0,    //i4Reserved0
                0,    //i4Reserved1
                0,    //i4Reserved2
                0,    //i4Reserved3
                0,    //i4Reserved4

                { //rLCELUTs
                    {    //i4vTbl1 (for i4vFixedPt1)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {2150,  2150,  2200,  2200,  2200,  2200,  2200,  2250,  2250,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  0 * N
                         {2150,  2150,  2150,  2200,  2200,  2200,  2200,  2250,  2250,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  1
                         {2150,  2150,  2150,  2150,  2150,  2150,  2150,  2200,  2200,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  2
                         {2150,  2150,  2150,  2150,  2150,  2150,  2150,  2200,  2200,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  3
                         {2100,  2100,  2100,  2150,  2150,  2150,  2150,  2150,  2150,  2200,  2200,   2150,   2100,   2100,   2100,   2100,   2100,   2100,   2100},   //  4
                         {2100,  2100,  2100,  2100,  2100,  2100,  2100,  2100,  2100,  2150,  2150,   2100,   2100,   2100,   2100,   2100,   2100,   2100,   2100},   //  5
                         {2050,  2050,  2050,  2050,  2050,  2050,  2050,  2050,  2050,  2100,  2100,   2100,   2100,   2100,   2100,   2100,   2100,   2100,   2100},   //  6
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  2050,  2050,   2050,   2050,   2050,   2100,   2100,   2050,   2050,   2050},   //  7
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  2100,  2100,   1950,   1950,   1950,   2050,   2050,   1950,   1950,   1950},   //  8
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950},   //  9
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950}    // 10

                    },

                    {    //i4vTbl2 (for i4vFixedPt2)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  10 * N
                         {2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  1
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  2
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  3
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2500,   2600,   2600,   2600,   2600,   2600,   2600},   //  4
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2500,   2500,   2500,   2500,   2500,   2500,   2500},   //  5
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,   2500,   2500,   2500,   2500,   2500,   2500,   2500,   2500},   //  6
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,   2400,   2500,   2500,   2500,   2500,   2500,   2500,   2500},   //  7
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,   2400,   2400,   2400,   2400,   2400,   2400,   2400,   2400},   //  8
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300},   //  9
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300}    // 10

                    },

                    {    //i4vTbl3 (for i4vFixedPt3)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100},   //  10 * N
                         {3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100},   //  1
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  2
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  3
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  4
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  5
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  6
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,   2900,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  7
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,   2900,   2900,   2900,   2900,   2900,   2900,   2900,   2900},   //  8
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850},   //  9
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850}    // 10

                    },

                    {    //i4vTbl4 (fori4vFixedPt4)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  10 * N
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  1
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  2
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  3
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  4
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  5
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  6
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  7
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  8
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  9
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450}    // 10


                    },

                    {    //i4DetailWtTbl
                        //  0   1   2   3   4   5   6   7   8    9   10
                        //   50, 50, 60, 70, 80, 90, 90, 90, 90, 100, 100 //original
                           100,100,100,100,100,100,100,100,100,100,100 //fixed P test
                    },

                    {    //i4CenSlopeEnhance
                        //  0   1   2   3   4   5   6   7   8   9   10   11
                            0,  4,  8, 12, 16, 20, 16, 12,  8,  4,   0,   0
                    }

                },
                {  // rLCESmooth
                    1,      // i4Enable
                    0,      // i4WaitAEStable
                    4       // i4Speed
                },
                {
                    // rLCEFlare
                    0   // i4Enable
                }
            },
            { //AutoHDR Video
                0,  //i4HistCCLB
                1600, //i4HistCCUB
                3,   //i4LCESeg
                600, //i4LCEContrastRatio
                25,  //i4LCEHistYThr
                51,  //SlopeLow --> 0.2*256
                307, //SlopeHigh --> 1.2*256
                384, //SlopeTop --> 1.5*256
                20,  //i4DetailRangeRatio --> base 1000
                60,  //i4CenSlopeMin

                0,    //i4vBlacKeep
                1700, //i4vFixedPt1
                2200, //i4vFixedPt2
                2800, //i4vFixedPt3
                3450, //i4vFixedPt4
                1000, //i4TCPLB
                20000,//I4TCPUB

                0,    //i4Reserved0
                0,    //i4Reserved1
                0,    //i4Reserved2
                0,    //i4Reserved3
                0,    //i4Reserved4

                { //rLCELUTs
                    {    //i4vTbl1 (for i4vFixedPt1)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {2150,  2150,  2200,  2200,  2200,  2200,  2200,  2250,  2250,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  0 * N
                         {2150,  2150,  2150,  2200,  2200,  2200,  2200,  2250,  2250,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  1
                         {2150,  2150,  2150,  2150,  2150,  2150,  2150,  2200,  2200,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  2
                         {2150,  2150,  2150,  2150,  2150,  2150,  2150,  2200,  2200,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  3
                         {2100,  2100,  2100,  2150,  2150,  2150,  2150,  2150,  2150,  2200,  2200,   2150,   2100,   2100,   2100,   2100,   2100,   2100,   2100},   //  4
                         {2100,  2100,  2100,  2100,  2100,  2100,  2100,  2100,  2100,  2150,  2150,   2100,   2100,   2100,   2100,   2100,   2100,   2100,   2100},   //  5
                         {2050,  2050,  2050,  2050,  2050,  2050,  2050,  2050,  2050,  2100,  2100,   2100,   2100,   2100,   2100,   2100,   2100,   2100,   2100},   //  6
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  2050,  2050,   2050,   2050,   2050,   2100,   2100,   2050,   2050,   2050},   //  7
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  2100,  2100,   1950,   1950,   1950,   2050,   2050,   1950,   1950,   1950},   //  8
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950},   //  9
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950}    // 10

                    },

                    {    //i4vTbl2 (for i4vFixedPt2)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  10 * N
                         {2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  1
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  2
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  3
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2500,   2600,   2600,   2600,   2600,   2600,   2600},   //  4
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2500,   2500,   2500,   2500,   2500,   2500,   2500},   //  5
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,   2500,   2500,   2500,   2500,   2500,   2500,   2500,   2500},   //  6
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,   2400,   2500,   2500,   2500,   2500,   2500,   2500,   2500},   //  7
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,   2400,   2400,   2400,   2400,   2400,   2400,   2400,   2400},   //  8
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300},   //  9
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300}    // 10

                    },

                    {    //i4vTbl3 (for i4vFixedPt3)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100},   //  10 * N
                         {3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100},   //  1
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  2
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  3
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  4
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  5
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  6
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,   2900,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  7
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,   2900,   2900,   2900,   2900,   2900,   2900,   2900,   2900},   //  8
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850},   //  9
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850}    // 10

                    },

                    {    //i4vTbl4 (fori4vFixedPt4)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  10 * N
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  1
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  2
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  3
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  4
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  5
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  6
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  7
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  8
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  9
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450}    // 10


                    },

                    {    //i4DetailWtTbl
                        //  0   1   2   3   4   5   6   7   8    9   10
                        //   50, 50, 60, 70, 80, 90, 90, 90, 90, 100, 100 //original
                           100,100,100,100,100,100,100,100,100,100,100 //fixed P test
                    },

                    {    //i4CenSlopeEnhance
                        //  0   1   2   3   4   5   6   7   8   9   10   11
                            0,  4,  8, 12, 16, 20, 16, 12,  8,  4,   0,   0
                    }

                },
                {  // rLCESmooth
                    1,      // i4Enable
                    0,      // i4WaitAEStable
                    4       // i4Speed
                },
                {
                    // rLCEFlare
                    0   // i4Enable
                }
            },
            { //AutoHDR Capture
                0,  //i4HistCCLB
                1600, //i4HistCCUB
                3,   //i4LCESeg
                600, //i4LCEContrastRatio
                25,  //i4LCEHistYThr
                51,  //SlopeLow --> 0.2*256
                307, //SlopeHigh --> 1.2*256
                384, //SlopeTop --> 1.5*256
                20,  //i4DetailRangeRatio --> base 1000
                60,  //i4CenSlopeMin

                0,    //i4vBlacKeep
                1700, //i4vFixedPt1
                2200, //i4vFixedPt2
                2800, //i4vFixedPt3
                3450, //i4vFixedPt4
                1000, //i4TCPLB
                20000,//I4TCPUB

                0,    //i4Reserved0
                0,    //i4Reserved1
                0,    //i4Reserved2
                0,    //i4Reserved3
                0,    //i4Reserved4

                { //rLCELUTs
                    {    //i4vTbl1 (for i4vFixedPt1)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {2150,  2150,  2200,  2200,  2200,  2200,  2200,  2250,  2250,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  0 * N
                         {2150,  2150,  2150,  2200,  2200,  2200,  2200,  2250,  2250,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  1
                         {2150,  2150,  2150,  2150,  2150,  2150,  2150,  2200,  2200,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  2
                         {2150,  2150,  2150,  2150,  2150,  2150,  2150,  2200,  2200,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  3
                         {2100,  2100,  2100,  2150,  2150,  2150,  2150,  2150,  2150,  2200,  2200,   2150,   2100,   2100,   2100,   2100,   2100,   2100,   2100},   //  4
                         {2100,  2100,  2100,  2100,  2100,  2100,  2100,  2100,  2100,  2150,  2150,   2100,   2100,   2100,   2100,   2100,   2100,   2100,   2100},   //  5
                         {2050,  2050,  2050,  2050,  2050,  2050,  2050,  2050,  2050,  2100,  2100,   2100,   2100,   2100,   2100,   2100,   2100,   2100,   2100},   //  6
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  2050,  2050,   2050,   2050,   2050,   2100,   2100,   2050,   2050,   2050},   //  7
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  2100,  2100,   1950,   1950,   1950,   2050,   2050,   1950,   1950,   1950},   //  8
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950},   //  9
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950}    // 10

                    },

                    {    //i4vTbl2 (for i4vFixedPt2)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  10 * N
                         {2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  1
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  2
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  3
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2500,   2600,   2600,   2600,   2600,   2600,   2600},   //  4
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2500,   2500,   2500,   2500,   2500,   2500,   2500},   //  5
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,   2500,   2500,   2500,   2500,   2500,   2500,   2500,   2500},   //  6
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,   2400,   2500,   2500,   2500,   2500,   2500,   2500,   2500},   //  7
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,   2400,   2400,   2400,   2400,   2400,   2400,   2400,   2400},   //  8
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300},   //  9
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300}    // 10

                    },

                    {    //i4vTbl3 (for i4vFixedPt3)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100},   //  10 * N
                         {3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100},   //  1
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  2
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  3
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  4
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  5
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  6
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,   2900,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  7
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,   2900,   2900,   2900,   2900,   2900,   2900,   2900,   2900},   //  8
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850},   //  9
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850}    // 10

                    },

                    {    //i4vTbl4 (fori4vFixedPt4)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  10 * N
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  1
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  2
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  3
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  4
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  5
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  6
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  7
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  8
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  9
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450}    // 10


                    },

                    {    //i4DetailWtTbl
                        //  0   1   2   3   4   5   6   7   8    9   10
                        //   50, 50, 60, 70, 80, 90, 90, 90, 90, 100, 100 //original
                           100,100,100,100,100,100,100,100,100,100,100 //fixed P test
                    },

                    {    //i4CenSlopeEnhance
                        //  0   1   2   3   4   5   6   7   8   9   10   11
                            0,  4,  8, 12, 16, 20, 16, 12,  8,  4,   0,   0
                    }

                },
                {  // rLCESmooth
                    1,      // i4Enable
                    0,      // i4WaitAEStable
                    4       // i4Speed
                },
                {
                    // rLCEFlare
                    0   // i4Enable
                }
            },
            { //Reserved 0
                0,  //i4HistCCLB
                1600, //i4HistCCUB
                3,   //i4LCESeg
                600, //i4LCEContrastRatio
                25,  //i4LCEHistYThr
                51,  //SlopeLow --> 0.2*256
                307, //SlopeHigh --> 1.2*256
                384, //SlopeTop --> 1.5*256
                20,  //i4DetailRangeRatio --> base 1000
                60,  //i4CenSlopeMin

                0,    //i4vBlacKeep
                1700, //i4vFixedPt1
                2200, //i4vFixedPt2
                2800, //i4vFixedPt3
                3450, //i4vFixedPt4
                1000, //i4TCPLB
                20000,//I4TCPUB

                0,    //i4Reserved0
                0,    //i4Reserved1
                0,    //i4Reserved2
                0,    //i4Reserved3
                0,    //i4Reserved4

                { //rLCELUTs
                    {    //i4vTbl1 (for i4vFixedPt1)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {2150,  2150,  2200,  2200,  2200,  2200,  2200,  2250,  2250,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  0 * N
                         {2150,  2150,  2150,  2200,  2200,  2200,  2200,  2250,  2250,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  1
                         {2150,  2150,  2150,  2150,  2150,  2150,  2150,  2200,  2200,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  2
                         {2150,  2150,  2150,  2150,  2150,  2150,  2150,  2200,  2200,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  3
                         {2100,  2100,  2100,  2150,  2150,  2150,  2150,  2150,  2150,  2200,  2200,   2150,   2100,   2100,   2100,   2100,   2100,   2100,   2100},   //  4
                         {2100,  2100,  2100,  2100,  2100,  2100,  2100,  2100,  2100,  2150,  2150,   2100,   2100,   2100,   2100,   2100,   2100,   2100,   2100},   //  5
                         {2050,  2050,  2050,  2050,  2050,  2050,  2050,  2050,  2050,  2100,  2100,   2100,   2100,   2100,   2100,   2100,   2100,   2100,   2100},   //  6
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  2050,  2050,   2050,   2050,   2050,   2100,   2100,   2050,   2050,   2050},   //  7
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  2100,  2100,   1950,   1950,   1950,   2050,   2050,   1950,   1950,   1950},   //  8
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950},   //  9
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950}    // 10

                    },

                    {    //i4vTbl2 (for i4vFixedPt2)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  10 * N
                         {2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  1
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  2
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  3
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2500,   2600,   2600,   2600,   2600,   2600,   2600},   //  4
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2500,   2500,   2500,   2500,   2500,   2500,   2500},   //  5
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,   2500,   2500,   2500,   2500,   2500,   2500,   2500,   2500},   //  6
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,   2400,   2500,   2500,   2500,   2500,   2500,   2500,   2500},   //  7
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,   2400,   2400,   2400,   2400,   2400,   2400,   2400,   2400},   //  8
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300},   //  9
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300}    // 10

                    },

                    {    //i4vTbl3 (for i4vFixedPt3)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100},   //  10 * N
                         {3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100},   //  1
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  2
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  3
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  4
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  5
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  6
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,   2900,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  7
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,   2900,   2900,   2900,   2900,   2900,   2900,   2900,   2900},   //  8
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850},   //  9
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850}    // 10

                    },

                    {    //i4vTbl4 (fori4vFixedPt4)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  10 * N
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  1
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  2
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  3
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  4
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  5
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  6
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  7
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  8
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  9
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450}    // 10


                    },

                    {    //i4DetailWtTbl
                        //  0   1   2   3   4   5   6   7   8    9   10
                        //   50, 50, 60, 70, 80, 90, 90, 90, 90, 100, 100 //original
                           100,100,100,100,100,100,100,100,100,100,100 //fixed P test
                    },

                    {    //i4CenSlopeEnhance
                        //  0   1   2   3   4   5   6   7   8   9   10   11
                            0,  4,  8, 12, 16, 20, 16, 12,  8,  4,   0,   0
                    }

                },
                {  // rLCESmooth
                    1,      // i4Enable
                    0,      // i4WaitAEStable
                    4       // i4Speed
                },
                {
                    // rLCEFlare
                    0   // i4Enable
                }
            },
            { //Reserved 1
                0,  //i4HistCCLB
                1600, //i4HistCCUB
                3,   //i4LCESeg
                600, //i4LCEContrastRatio
                25,  //i4LCEHistYThr
                51,  //SlopeLow --> 0.2*256
                307, //SlopeHigh --> 1.2*256
                384, //SlopeTop --> 1.5*256
                20,  //i4DetailRangeRatio --> base 1000
                60,  //i4CenSlopeMin

                0,    //i4vBlacKeep
                1700, //i4vFixedPt1
                2200, //i4vFixedPt2
                2800, //i4vFixedPt3
                3450, //i4vFixedPt4
                1000, //i4TCPLB
                20000,//I4TCPUB

                0,    //i4Reserved0
                0,    //i4Reserved1
                0,    //i4Reserved2
                0,    //i4Reserved3
                0,    //i4Reserved4

                { //rLCELUTs
                    {    //i4vTbl1 (for i4vFixedPt1)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {2150,  2150,  2200,  2200,  2200,  2200,  2200,  2250,  2250,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  0 * N
                         {2150,  2150,  2150,  2200,  2200,  2200,  2200,  2250,  2250,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  1
                         {2150,  2150,  2150,  2150,  2150,  2150,  2150,  2200,  2200,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  2
                         {2150,  2150,  2150,  2150,  2150,  2150,  2150,  2200,  2200,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  3
                         {2100,  2100,  2100,  2150,  2150,  2150,  2150,  2150,  2150,  2200,  2200,   2150,   2100,   2100,   2100,   2100,   2100,   2100,   2100},   //  4
                         {2100,  2100,  2100,  2100,  2100,  2100,  2100,  2100,  2100,  2150,  2150,   2100,   2100,   2100,   2100,   2100,   2100,   2100,   2100},   //  5
                         {2050,  2050,  2050,  2050,  2050,  2050,  2050,  2050,  2050,  2100,  2100,   2100,   2100,   2100,   2100,   2100,   2100,   2100,   2100},   //  6
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  2050,  2050,   2050,   2050,   2050,   2100,   2100,   2050,   2050,   2050},   //  7
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  2100,  2100,   1950,   1950,   1950,   2050,   2050,   1950,   1950,   1950},   //  8
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950},   //  9
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950}    // 10

                    },

                    {    //i4vTbl2 (for i4vFixedPt2)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  10 * N
                         {2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  1
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  2
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  3
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2500,   2600,   2600,   2600,   2600,   2600,   2600},   //  4
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2500,   2500,   2500,   2500,   2500,   2500,   2500},   //  5
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,   2500,   2500,   2500,   2500,   2500,   2500,   2500,   2500},   //  6
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,   2400,   2500,   2500,   2500,   2500,   2500,   2500,   2500},   //  7
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,   2400,   2400,   2400,   2400,   2400,   2400,   2400,   2400},   //  8
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300},   //  9
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300}    // 10

                    },

                    {    //i4vTbl3 (for i4vFixedPt3)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100},   //  10 * N
                         {3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100},   //  1
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  2
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  3
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  4
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  5
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  6
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,   2900,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  7
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,   2900,   2900,   2900,   2900,   2900,   2900,   2900,   2900},   //  8
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850},   //  9
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850}    // 10

                    },

                    {    //i4vTbl4 (fori4vFixedPt4)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  10 * N
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  1
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  2
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  3
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  4
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  5
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  6
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  7
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  8
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  9
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450}    // 10


                    },

                    {    //i4DetailWtTbl
                        //  0   1   2   3   4   5   6   7   8    9   10
                        //   50, 50, 60, 70, 80, 90, 90, 90, 90, 100, 100 //original
                           100,100,100,100,100,100,100,100,100,100,100 //fixed P test
                    },

                    {    //i4CenSlopeEnhance
                        //  0   1   2   3   4   5   6   7   8   9   10   11
                            0,  4,  8, 12, 16, 20, 16, 12,  8,  4,   0,   0
                    }

                },
                {  // rLCESmooth
                    1,      // i4Enable
                    0,      // i4WaitAEStable
                    4       // i4Speed
                },
                {
                    // rLCEFlare
                    0   // i4Enable
                }
            },
            { //Reserved 2
                0,  //i4HistCCLB
                1600, //i4HistCCUB
                3,   //i4LCESeg
                600, //i4LCEContrastRatio
                25,  //i4LCEHistYThr
                51,  //SlopeLow --> 0.2*256
                307, //SlopeHigh --> 1.2*256
                384, //SlopeTop --> 1.5*256
                20,  //i4DetailRangeRatio --> base 1000
                60,  //i4CenSlopeMin

                0,    //i4vBlacKeep
                1700, //i4vFixedPt1
                2200, //i4vFixedPt2
                2800, //i4vFixedPt3
                3450, //i4vFixedPt4
                1000, //i4TCPLB
                20000,//I4TCPUB

                0,    //i4Reserved0
                0,    //i4Reserved1
                0,    //i4Reserved2
                0,    //i4Reserved3
                0,    //i4Reserved4

                { //rLCELUTs
                    {    //i4vTbl1 (for i4vFixedPt1)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {2150,  2150,  2200,  2200,  2200,  2200,  2200,  2250,  2250,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  0 * N
                         {2150,  2150,  2150,  2200,  2200,  2200,  2200,  2250,  2250,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  1
                         {2150,  2150,  2150,  2150,  2150,  2150,  2150,  2200,  2200,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  2
                         {2150,  2150,  2150,  2150,  2150,  2150,  2150,  2200,  2200,  2250,  2250,   2200,   2150,   2150,   2150,   2150,   2150,   2150,   2150},   //  3
                         {2100,  2100,  2100,  2150,  2150,  2150,  2150,  2150,  2150,  2200,  2200,   2150,   2100,   2100,   2100,   2100,   2100,   2100,   2100},   //  4
                         {2100,  2100,  2100,  2100,  2100,  2100,  2100,  2100,  2100,  2150,  2150,   2100,   2100,   2100,   2100,   2100,   2100,   2100,   2100},   //  5
                         {2050,  2050,  2050,  2050,  2050,  2050,  2050,  2050,  2050,  2100,  2100,   2100,   2100,   2100,   2100,   2100,   2100,   2100,   2100},   //  6
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  2050,  2050,   2050,   2050,   2050,   2100,   2100,   2050,   2050,   2050},   //  7
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  2100,  2100,   1950,   1950,   1950,   2050,   2050,   1950,   1950,   1950},   //  8
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950},   //  9
                         {1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,  1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950,   1950}    // 10

                    },

                    {    //i4vTbl2 (for i4vFixedPt2)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  10 * N
                         {2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  1
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  2
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2600,   2600,   2600,   2600,   2600,   2600,   2600},   //  3
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2500,   2600,   2600,   2600,   2600,   2600,   2600},   //  4
                         {2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,   2500,   2500,   2500,   2500,   2500,   2500,   2500,   2500},   //  5
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,   2500,   2500,   2500,   2500,   2500,   2500,   2500,   2500},   //  6
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,   2400,   2500,   2500,   2500,   2500,   2500,   2500,   2500},   //  7
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,   2400,   2400,   2400,   2400,   2400,   2400,   2400,   2400},   //  8
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300},   //  9
                         {2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2400,  2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300,   2300}    // 10

                    },

                    {    //i4vTbl3 (for i4vFixedPt3)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100},   //  10 * N
                         {3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100,   3100},   //  1
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  2
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  3
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  4
                         {3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,  3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  5
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,   3000,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  6
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,   2900,   3000,   3000,   3000,   3000,   3000,   3000,   3000},   //  7
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,   2900,   2900,   2900,   2900,   2900,   2900,   2900,   2900},   //  8
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850},   //  9
                         {2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850,   2850}    // 10

                    },

                    {    //i4vTbl4 (fori4vFixedPt4)
                     //    LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9    LV10    LV11    LV12    LV13    LV14    LV15    LV16    LV17    LV18
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  10 * N
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  1
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  2
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  3
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  4
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  5
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  6
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  7
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  8
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450},   //  9
                         {3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,  3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450,   3450}    // 10


                    },

                    {    //i4DetailWtTbl
                        //  0   1   2   3   4   5   6   7   8    9   10
                        //   50, 50, 60, 70, 80, 90, 90, 90, 90, 100, 100 //original
                           100,100,100,100,100,100,100,100,100,100,100 //fixed P test
                    },

                    {    //i4CenSlopeEnhance
                        //  0   1   2   3   4   5   6   7   8   9   10   11
                            0,  4,  8, 12, 16, 20, 16, 12,  8,  4,   0,   0
                    }

                },
                {  // rLCESmooth
                    1,      // i4Enable
                    0,      // i4WaitAEStable
                    4       // i4Speed
                },
                {
                    // rLCEFlare
                    0   // i4Enable
                }
            }
        }
    },
    .ANR_TBL=
    {

    //CT_0
    {

        //ISO_0
        {set:{
            0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04490447, 0x04440445, 0x04440444, 0x04440444,
            0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445,
            0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444,
            0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444,
            0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
            0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444,
            0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444,
            0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446,
            0x04440444, 0x04440444, 0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444,
            0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
            0x04440444, 0x04440444, 0x044c0447, 0x04440445, 0x04440444, 0x04440444, 0x04470445, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04430443, 0x04430443, 0x04440443, 0x04430443, 0x04430443, 0x04430443,
            0x04470445, 0x04430444, 0x04430443, 0x04430443, 0x04490446, 0x04430444, 0x04430443, 0x04430443, 0x04460444, 0x04430444,
            0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443,
            0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443
        }},

        //ISO_1
        {set:{
            0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04490447, 0x04440445, 0x04440444, 0x04440444,
            0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445,
            0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444,
            0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444,
            0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
            0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444,
            0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444,
            0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446,
            0x04440444, 0x04440444, 0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444,
            0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
            0x04440444, 0x04440444, 0x044c0447, 0x04440445, 0x04440444, 0x04440444, 0x04470445, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04430443, 0x04430443, 0x04440443, 0x04430443, 0x04430443, 0x04430443,
            0x04470445, 0x04430444, 0x04430443, 0x04430443, 0x04490446, 0x04430444, 0x04430443, 0x04430443, 0x04460444, 0x04430444,
            0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443,
            0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443
        }},

        //ISO_2
        {set:{
            0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04490447, 0x04440445, 0x04440444, 0x04440444,
            0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445,
            0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444,
            0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444,
            0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
            0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444,
            0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444,
            0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446,
            0x04440444, 0x04440444, 0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444,
            0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
            0x04440444, 0x04440444, 0x044c0447, 0x04440445, 0x04440444, 0x04440444, 0x04470445, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04430443, 0x04430443, 0x04440443, 0x04430443, 0x04430443, 0x04430443,
            0x04470445, 0x04430444, 0x04430443, 0x04430443, 0x04490446, 0x04430444, 0x04430443, 0x04430443, 0x04460444, 0x04430444,
            0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443,
            0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443
        }},

        //ISO_3
        {set:{
            0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04490447, 0x04440445, 0x04440444, 0x04440444,
            0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445,
            0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444,
            0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444,
            0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
            0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444,
            0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444,
            0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446,
            0x04440444, 0x04440444, 0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444,
            0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
            0x04440444, 0x04440444, 0x044c0447, 0x04440445, 0x04440444, 0x04440444, 0x04470445, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04430443, 0x04430443, 0x04440443, 0x04430443, 0x04430443, 0x04430443,
            0x04470445, 0x04430444, 0x04430443, 0x04430443, 0x04490446, 0x04430444, 0x04430443, 0x04430443, 0x04460444, 0x04430444,
            0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443,
            0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443
        }},

        //ISO_4
        {set:{
            0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04490447, 0x04440445, 0x04440444, 0x04440444,
            0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445,
            0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444,
            0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444,
            0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
            0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444,
            0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444,
            0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446,
            0x04440444, 0x04440444, 0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444,
            0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
            0x04440444, 0x04440444, 0x044c0447, 0x04440445, 0x04440444, 0x04440444, 0x04470445, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04430443, 0x04430443, 0x04440443, 0x04430443, 0x04430443, 0x04430443,
            0x04470445, 0x04430444, 0x04430443, 0x04430443, 0x04490446, 0x04430444, 0x04430443, 0x04430443, 0x04460444, 0x04430444,
            0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443,
            0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443
        }},

        //ISO_5
        {set:{
            0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04490447, 0x04440445, 0x04440444, 0x04440444,
            0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445,
            0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444,
            0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444,
            0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
            0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444,
            0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444,
            0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446,
            0x04440444, 0x04440444, 0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444,
            0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
            0x04440444, 0x04440444, 0x044c0447, 0x04440445, 0x04440444, 0x04440444, 0x04470445, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04430443, 0x04430443, 0x04440443, 0x04430443, 0x04430443, 0x04430443,
            0x04470445, 0x04430444, 0x04430443, 0x04430443, 0x04490446, 0x04430444, 0x04430443, 0x04430443, 0x04460444, 0x04430444,
            0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443,
            0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443
        }}

    },

    //CT_1
    {

        //ISO_0
        {set:{
            0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04490447, 0x04440445, 0x04440444, 0x04440444,
            0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445,
            0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444,
            0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444,
            0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
            0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444,
            0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444,
            0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446,
            0x04440444, 0x04440444, 0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444,
            0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
            0x04440444, 0x04440444, 0x044c0447, 0x04440445, 0x04440444, 0x04440444, 0x04470445, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04430443, 0x04430443, 0x04440443, 0x04430443, 0x04430443, 0x04430443,
            0x04470445, 0x04430444, 0x04430443, 0x04430443, 0x04490446, 0x04430444, 0x04430443, 0x04430443, 0x04460444, 0x04430444,
            0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443,
            0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443
        }},

        //ISO_1
        {set:{
            0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04490447, 0x04440445, 0x04440444, 0x04440444,
            0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445,
            0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444,
            0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444,
            0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
            0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444,
            0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444,
            0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446,
            0x04440444, 0x04440444, 0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444,
            0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
            0x04440444, 0x04440444, 0x044c0447, 0x04440445, 0x04440444, 0x04440444, 0x04470445, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04430443, 0x04430443, 0x04440443, 0x04430443, 0x04430443, 0x04430443,
            0x04470445, 0x04430444, 0x04430443, 0x04430443, 0x04490446, 0x04430444, 0x04430443, 0x04430443, 0x04460444, 0x04430444,
            0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443,
            0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443
        }},

        //ISO_2
        {set:{
            0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04490447, 0x04440445, 0x04440444, 0x04440444,
            0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445,
            0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444,
            0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444,
            0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
            0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444,
            0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444,
            0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446,
            0x04440444, 0x04440444, 0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444,
            0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
            0x04440444, 0x04440444, 0x044c0447, 0x04440445, 0x04440444, 0x04440444, 0x04470445, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04430443, 0x04430443, 0x04440443, 0x04430443, 0x04430443, 0x04430443,
            0x04470445, 0x04430444, 0x04430443, 0x04430443, 0x04490446, 0x04430444, 0x04430443, 0x04430443, 0x04460444, 0x04430444,
            0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443,
            0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443
        }},

        //ISO_3
        {set:{
            0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04490447, 0x04440445, 0x04440444, 0x04440444,
            0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445,
            0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444,
            0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444,
            0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
            0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444,
            0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444,
            0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446,
            0x04440444, 0x04440444, 0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444,
            0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
            0x04440444, 0x04440444, 0x044c0447, 0x04440445, 0x04440444, 0x04440444, 0x04470445, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04430443, 0x04430443, 0x04440443, 0x04430443, 0x04430443, 0x04430443,
            0x04470445, 0x04430444, 0x04430443, 0x04430443, 0x04490446, 0x04430444, 0x04430443, 0x04430443, 0x04460444, 0x04430444,
            0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443,
            0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443
        }},

        //ISO_4
        {set:{
            0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04490447, 0x04440445, 0x04440444, 0x04440444,
            0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445,
            0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444,
            0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444,
            0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
            0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444,
            0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444,
            0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446,
            0x04440444, 0x04440444, 0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444,
            0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
            0x04440444, 0x04440444, 0x044c0447, 0x04440445, 0x04440444, 0x04440444, 0x04470445, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04430443, 0x04430443, 0x04440443, 0x04430443, 0x04430443, 0x04430443,
            0x04470445, 0x04430444, 0x04430443, 0x04430443, 0x04490446, 0x04430444, 0x04430443, 0x04430443, 0x04460444, 0x04430444,
            0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443,
            0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443
        }},

        //ISO_5
        {set:{
            0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04490447, 0x04440445, 0x04440444, 0x04440444,
            0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445,
            0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444,
            0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444,
            0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
            0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444,
            0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444,
            0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446,
            0x04440444, 0x04440444, 0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444,
            0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
            0x04440444, 0x04440444, 0x044c0447, 0x04440445, 0x04440444, 0x04440444, 0x04470445, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
            0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04430443, 0x04430443, 0x04440443, 0x04430443, 0x04430443, 0x04430443,
            0x04470445, 0x04430444, 0x04430443, 0x04430443, 0x04490446, 0x04430444, 0x04430443, 0x04430443, 0x04460444, 0x04430444,
            0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443,
            0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443
        }}

    }
},
    // PCA Param
    {0, 0, 0, 0, 64, 0, 0}
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
                1280,    // u4MinGain, 1024 base = 1x
                10512,    // u4MaxGain, 16x
                100,    // u4MiniISOGain, ISOxx
                128,    // u4GainStepUnit, 1x/8
                25915,    // u4PreExpUnit
                30,    // u4PreMaxFrameRate
                13039,    // u4VideoExpUnit
                30,    // u4VideoMaxFrameRate
                1024,    // u4Video2PreRatio, 1024 base = 1x
                13039,    // u4CapExpUnit
                30,    // u4CapMaxFrameRate
                1024,    // u4Cap2PreRatio, 1024 base = 1x
                15256,     // u4Video1ExpUnit
                120,     // u4Video1MaxFrameRate
                1024,   // u4Video12PreRatio, 1024 base = 1x
                25915,     // u4Video2ExpUnit
                30,     // u4Video2MaxFrameRate
                1024,   // u4Video22PreRatio, 1024 base = 1x
                19770,     // u4Custom1ExpUnit
                30,     // u4Custom1MaxFrameRate
                1024,   // u4Custom12PreRatio, 1024 base = 1x
                19770,     // u4Custom2ExpUnit
                30,     // u4Custom2MaxFrameRate
                1024,   // u4Custom22PreRatio, 1024 base = 1x
                19770,     // u4Custom3ExpUnit
                30,     // u4Custom3MaxFrameRate
                1024,   // u4Custom32PreRatio, 1024 base = 1x
                19770,     // u4Custom4ExpUnit
                30,     // u4Custom4MaxFrameRate
                1024,   // u4Custom42PreRatio, 1024 base = 1x
                19770,     // u4Custom5ExpUnit
                30,     // u4Custom5MaxFrameRate
                1024,   // u4Custom52PreRatio, 1024 base = 1x
                20,    // u4LensFno, Fno = 2.8
                285    // u4FocusLength_100x
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

                50,                // u4InitIndex
                4,                 // u4BackLightWeight
                32,                // u4HistStretchWeight
                4,                 // u4AntiOverExpWeight
                2,                 // u4BlackLightStrengthIndex
                2,                 // u4HistStretchStrengthIndex
                2,                 // u4AntiOverExpStrengthIndex
                2,                 // u4TimeLPFStrengthIndex
                {1, 3, 5, 7, 8}, // u4LPFConvergeTable[AE_CCT_STRENGTH_NUM]
                90,                // u4InDoorEV = 9.0, 10 base
                -6,    // i4BVOffset delta BV = value/10
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
                                80, //85,// 82, //  95, //u4BrightToneThd

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
                            47,  //u4Y_Target
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
                            47,  //u4Y_Target
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
                            47,  //u4Y_Target
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
                1280,    // u4MinGain, 1024 base = 1x
                10512,    // u4MaxGain, 16x
                100,    // u4MiniISOGain, ISOxx
                128,    // u4GainStepUnit, 1x/8
                25915,    // u4PreExpUnit
                30,    // u4PreMaxFrameRate
                13039,    // u4VideoExpUnit
                30,    // u4VideoMaxFrameRate
                1024,    // u4Video2PreRatio, 1024 base = 1x
                13039,    // u4CapExpUnit
                30,    // u4CapMaxFrameRate
                1024,    // u4Cap2PreRatio, 1024 base = 1x
                15256,     // u4Video1ExpUnit
                120,     // u4Video1MaxFrameRate
                1024,   // u4Video12PreRatio, 1024 base = 1x
                25915,     // u4Video2ExpUnit
                30,     // u4Video2MaxFrameRate
                1024,   // u4Video22PreRatio, 1024 base = 1x
                19770,     // u4Custom1ExpUnit
                30,     // u4Custom1MaxFrameRate
                1024,   // u4Custom12PreRatio, 1024 base = 1x
                19770,     // u4Custom2ExpUnit
                30,     // u4Custom2MaxFrameRate
                1024,   // u4Custom22PreRatio, 1024 base = 1x
                19770,     // u4Custom3ExpUnit
                30,     // u4Custom3MaxFrameRate
                1024,   // u4Custom32PreRatio, 1024 base = 1x
                19770,     // u4Custom4ExpUnit
                30,     // u4Custom4MaxFrameRate
                1024,   // u4Custom42PreRatio, 1024 base = 1x
                19770,     // u4Custom5ExpUnit
                30,     // u4Custom5MaxFrameRate
                1024,   // u4Custom52PreRatio, 1024 base = 1x
                20,    // u4LensFno, Fno = 2.8
                285    // u4FocusLength_100x
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

                50,                // u4InitIndex
                4,                 // u4BackLightWeight
                32,                // u4HistStretchWeight
                4,                 // u4AntiOverExpWeight
                2,                 // u4BlackLightStrengthIndex
                2,                 // u4HistStretchStrengthIndex
                2,                 // u4AntiOverExpStrengthIndex
                2,                 // u4TimeLPFStrengthIndex
                {1, 3, 5, 7, 8}, // u4LPFConvergeTable[AE_CCT_STRENGTH_NUM]
                90,                // u4InDoorEV = 9.0, 10 base
                -6,    // i4BVOffset delta BV = value/10
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
                                80, //85,// 82, //  95, //u4BrightToneThd

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
                            47,  //u4Y_Target
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
                            47,  //u4Y_Target
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
                            47,  //u4Y_Target
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
                1280,    // u4MinGain, 1024 base = 1x
                10512,    // u4MaxGain, 16x
                100,    // u4MiniISOGain, ISOxx
                128,    // u4GainStepUnit, 1x/8
                25915,    // u4PreExpUnit
                30,    // u4PreMaxFrameRate
                13039,    // u4VideoExpUnit
                30,    // u4VideoMaxFrameRate
                1024,    // u4Video2PreRatio, 1024 base = 1x
                13039,    // u4CapExpUnit
                30,    // u4CapMaxFrameRate
                1024,    // u4Cap2PreRatio, 1024 base = 1x
                15256,     // u4Video1ExpUnit
                120,     // u4Video1MaxFrameRate
                1024,   // u4Video12PreRatio, 1024 base = 1x
                25915,     // u4Video2ExpUnit
                30,     // u4Video2MaxFrameRate
                1024,   // u4Video22PreRatio, 1024 base = 1x
                19770,     // u4Custom1ExpUnit
                30,     // u4Custom1MaxFrameRate
                1024,   // u4Custom12PreRatio, 1024 base = 1x
                19770,     // u4Custom2ExpUnit
                30,     // u4Custom2MaxFrameRate
                1024,   // u4Custom22PreRatio, 1024 base = 1x
                19770,     // u4Custom3ExpUnit
                30,     // u4Custom3MaxFrameRate
                1024,   // u4Custom32PreRatio, 1024 base = 1x
                19770,     // u4Custom4ExpUnit
                30,     // u4Custom4MaxFrameRate
                1024,   // u4Custom42PreRatio, 1024 base = 1x
                19770,     // u4Custom5ExpUnit
                30,     // u4Custom5MaxFrameRate
                1024,   // u4Custom52PreRatio, 1024 base = 1x
                20,    // u4LensFno, Fno = 2.8
                285    // u4FocusLength_100x
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

                50,                // u4InitIndex
                4,                 // u4BackLightWeight
                32,                // u4HistStretchWeight
                4,                 // u4AntiOverExpWeight
                2,                 // u4BlackLightStrengthIndex
                2,                 // u4HistStretchStrengthIndex
                2,                 // u4AntiOverExpStrengthIndex
                2,                 // u4TimeLPFStrengthIndex
                {1, 3, 5, 7, 8}, // u4LPFConvergeTable[AE_CCT_STRENGTH_NUM]
                90,                // u4InDoorEV = 9.0, 10 base
                -6,    // i4BVOffset delta BV = value/10
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
                                80, //85,// 82, //  95, //u4BrightToneThd

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
                            47,  //u4Y_Target
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
                            47,  //u4Y_Target
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
                            47,  //u4Y_Target
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
                1280,    // u4MinGain, 1024 base = 1x
                10512,    // u4MaxGain, 16x
                100,    // u4MiniISOGain, ISOxx
                128,    // u4GainStepUnit, 1x/8
                25915,    // u4PreExpUnit
                30,    // u4PreMaxFrameRate
                13039,    // u4VideoExpUnit
                30,    // u4VideoMaxFrameRate
                1024,    // u4Video2PreRatio, 1024 base = 1x
                13039,    // u4CapExpUnit
                30,    // u4CapMaxFrameRate
                1024,    // u4Cap2PreRatio, 1024 base = 1x
                15256,     // u4Video1ExpUnit
                120,     // u4Video1MaxFrameRate
                1024,   // u4Video12PreRatio, 1024 base = 1x
                25915,     // u4Video2ExpUnit
                30,     // u4Video2MaxFrameRate
                1024,   // u4Video22PreRatio, 1024 base = 1x
                19770,     // u4Custom1ExpUnit
                30,     // u4Custom1MaxFrameRate
                1024,   // u4Custom12PreRatio, 1024 base = 1x
                19770,     // u4Custom2ExpUnit
                30,     // u4Custom2MaxFrameRate
                1024,   // u4Custom22PreRatio, 1024 base = 1x
                19770,     // u4Custom3ExpUnit
                30,     // u4Custom3MaxFrameRate
                1024,   // u4Custom32PreRatio, 1024 base = 1x
                19770,     // u4Custom4ExpUnit
                30,     // u4Custom4MaxFrameRate
                1024,   // u4Custom42PreRatio, 1024 base = 1x
                19770,     // u4Custom5ExpUnit
                30,     // u4Custom5MaxFrameRate
                1024,   // u4Custom52PreRatio, 1024 base = 1x
                20,    // u4LensFno, Fno = 2.8
                285    // u4FocusLength_100x
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

                50,                // u4InitIndex
                4,                 // u4BackLightWeight
                32,                // u4HistStretchWeight
                4,                 // u4AntiOverExpWeight
                2,                 // u4BlackLightStrengthIndex
                2,                 // u4HistStretchStrengthIndex
                2,                 // u4AntiOverExpStrengthIndex
                2,                 // u4TimeLPFStrengthIndex
                {1, 3, 5, 7, 8}, // u4LPFConvergeTable[AE_CCT_STRENGTH_NUM]
                90,                // u4InDoorEV = 9.0, 10 base
                -6,    // i4BVOffset delta BV = value/10
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
                                80, //85,// 82, //  95, //u4BrightToneThd

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
                            47,  //u4Y_Target
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
                            47,  //u4Y_Target
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
                            47,  //u4Y_Target
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
                                842,    // D65Gain_R
                                512,    // D65Gain_G
                                726    // D65Gain_B
                    }
                },
                // Original XY coordinate of AWB light source
                {
                // Strobe
                {
                    0,    // i4X
                    0    // i4Y
                },
                // Horizon
                {
                    -423,    // OriX_Hor
                    -289    // OriY_Hor
                },
                // A
                {
                    -319,    // OriX_A
                    -311    // OriY_A
                },
                // TL84
                {
                    -166,    // OriX_TL84
                    -346    // OriY_TL84
                },
                // CWF
                {
                    -141,    // OriX_CWF
                    -401    // OriY_CWF
                },
                // DNP
                {
                    -19,    // OriX_DNP
                    -338    // OriY_DNP
                },
                // D65
                {
                    55,    // OriX_D65
                    -313    // OriY_D65
                },
                // DF
                {
                    41,    // OriX_DF
                    -389    // OriY_DF
                }
            },
                // Rotated XY coordinate of AWB light source
                {
                // Strobe
                {
                    0,    // i4X
                    0    // i4Y
                },
                // Horizon
                {
                    -418,    // RotX_Hor
                    -296    // RotY_Hor
                },
                // A
                {
                    -314,    // RotX_A
                    -316    // RotY_A
                },
                // TL84
                {
                    -161,    // RotX_TL84
                    -349    // RotY_TL84
                },
                // CWF
                {
                    -135,    // RotX_CWF
                    -403    // RotY_CWF
                },
                // DNP
                {
                    -14,    // RotX_DNP
                    -338    // RotY_DNP
                },
                // D65
                {
                    60,    // RotX_D65
                    -312    // RotY_D65
                },
                // DF
                {
                    47,    // RotX_DF
                    -388    // RotY_DF
                }
            },
                // AWB gain of AWB light source
                {
                // Strobe
                {
                    512,    // i4R
                    512,    // i4G
                    512    // i4B
                },
                // Horizon
                {
                    512,    // AWBGAIN_HOR_R
                    614,    // AWBGAIN_HOR_G
                    1609    // AWBGAIN_HOR_B
                },
                // A
                {
                    512,    // AWBGAIN_A_R
                    517,    // AWBGAIN_A_G
                    1213    // AWBGAIN_A_B
                },
                // TL84
                {
                    654,    // AWBGAIN_TL84_R
                    512,    // AWBGAIN_TL84_G
                    1024    // AWBGAIN_TL84_B
                },
                // CWF
                {
                    728,    // AWBGAIN_CWF_R
                    512,    // AWBGAIN_CWF_G
                    1066    // AWBGAIN_CWF_B
                },
                // DNP
                {
                    788,    // AWBGAIN_DNP_R
                    512,    // AWBGAIN_DNP_G
                    831    // AWBGAIN_DNP_B
                },
                // D65
                {
                    842,    // AWBGAIN_D65_R
                    512,    // AWBGAIN_D65_G
                    726    // AWBGAIN_D65_B
                },
                // DF
                {
                    916,    // AWBGAIN_DF_R
                    512,    // AWBGAIN_DF_G
                    820    // AWBGAIN_DF_B
                }
            },
                // Rotation matrix parameter
    {
                -1,    // RotationAngle
                256,    // Cos
                -4    // Sin
            },
    // Daylight locus parameter
    {
                    -124,    // i4SlopeNumerator
                    128    // i4SlopeDenominator
            },
            // Predictor gain
            {
                // DaylightLocus_L
                {
                    812,    // i4R
                    530,    // i4G
                    746    // i4B
                },
                // DaylightLocus_H
                {
                    750,    // i4R
                    512,    // i4G
                    812    // i4B
                },
                // Temporal General
                {
                    842,    // i4R
                    512,    // i4G
                    726    // i4B
                },
            },
                // AWB light area
                {
                // Strobe:FIXME
                {
                    0,    // i4RightBound
                    0,    // i4LeftBound
                    0,    // i4UpperBound
                    0    // i4LowerBound
                },
                // Tungsten
                {
                    -222,    // TungRightBound
                    -818,    // TungLeftBound
                    -241,    // TungUpperBound
                    -333    // TungLowerBound
                },
                // Warm fluorescent
                {
                    -222,    // WFluoRightBound
                    -818,    // WFluoLeftBound
                    -333,    // WFluoUpperBound
                    -493    // WFluoLowerBound
                },
                // Fluorescent
                {
                    -62,    // FluoRightBound
                    -222,    // FluoLeftBound
                    -252,    // FluoUpperBound
                    -376    // FluoLowerBound
                },
                // CWF
                {
                -55,    // CWFRightBound
                -222,    // CWFLeftBound
                -376,    // CWFUpperBound
                -458    // CWFLowerBound
                },
                // Daylight
                {
                    90,    // DayRightBound
                    -62,    // DayLeftBound
                    -252,    // DayUpperBound
                    -376    // DayLowerBound
                },
                // Shade
                {
                    420,    // ShadeRightBound
                    90,    // ShadeLeftBound
                    -252,    // ShadeUpperBound
                    -348    // ShadeLowerBound
                },
                // Daylight Fluorescent
                {
                    90,    // DFRightBound
                    -55,    // DFLeftBound
                    -376,    // DFUpperBound
                    -458    // DFLowerBound
                }
            },
    // PWB light area
    {
                // Reference area
                {
                    420,    // PRefRightBound
                    -818,    // PRefLeftBound
                    -216,    // PRefUpperBound
                    -493    // PRefLowerBound
                },
                // Daylight
                {
                    115,    // PDayRightBound
                    -62,    // PDayLeftBound
                    -252,    // PDayUpperBound
                    -376    // PDayLowerBound
                },
                // Cloudy daylight
                {
                    215,    // PCloudyRightBound
                    40,    // PCloudyLeftBound
                    -252,    // PCloudyUpperBound
                    -376    // PCloudyLowerBound
                },
                // Shade
                {
                    315,    // PShadeRightBound
                    40,    // PShadeLeftBound
                    -252,    // PShadeUpperBound
                    -376    // PShadeLowerBound
                },
                // Twilight
                {
                    -62,    // PTwiRightBound
                    -222,    // PTwiLeftBound
                    -252,    // PTwiUpperBound
                    -376    // PTwiLowerBound
                },
                // Fluorescent
                {
                    110,    // PFluoRightBound
                    -261,    // PFluoLeftBound
                    -262,    // PFluoUpperBound
                    -453    // PFluoLowerBound
                },
                // Warm fluorescent
                {
                    -214,    // PWFluoRightBound
                    -414,    // PWFluoLeftBound
                    -262,    // PWFluoUpperBound
                    -453    // PWFluoLowerBound
                },
                // Incandescent
                {
                    -214,    // PIncaRightBound
                    -414,    // PIncaLeftBound
                    -252,    // PIncaUpperBound
                    -376    // PIncaLowerBound
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
                    807,    // PWB_Day_R
                    512,    // PWB_Day_G
                    761    // PWB_Day_B
                },
                // Cloudy daylight
                {
                    927,    // PWB_Cloudy_R
                    512,    // PWB_Cloudy_G
                    665    // PWB_Cloudy_B
                },
                // Shade
                {
                    993,    // PWB_Shade_R
                    512,    // PWB_Shade_G
                    622    // PWB_Shade_B
                },
                // Twilight
                {
                    640,    // PWB_Twi_R
                    512,    // PWB_Twi_G
                    953    // PWB_Twi_B
                },
                // Fluorescent
                {
                    743,    // PWB_Fluo_R
                    512,    // PWB_Fluo_G
                    926    // PWB_Fluo_B
                },
                // Warm fluorescent
                {
                    535,    // PWB_WFluo_R
                    512,    // PWB_WFluo_G
                    1272    // PWB_WFluo_B
                },
                // Incandescent
                {
                    505,    // PWB_Inca_R
                    512,    // PWB_Inca_G
                    1198    // PWB_Inca_B
                },
                // Gray World
                {
                    512,    // PWB_GW_R
                    512,    // PWB_GW_G
                    512    // PWB_GW_B
                }
            },
                // CCT estimation
                {
                    // CCT
                    {
                        2300,   // i4CCT[0]
                        2850,   // i4CCT[1]
                        3750,   // i4CCT[2]
                        5100,   // i4CCT[3]
                        6500    // i4CCT[4]
                    },
                    // Rotated X coordinate
                    {
                -478,    // i4RotatedXCoordinate[0]
                -374,    // i4RotatedXCoordinate[1]
                -221,    // i4RotatedXCoordinate[2]
                -74,    // i4RotatedXCoordinate[3]
                0    // i4RotatedXCoordinate[4]
                }
                }
            },

            // Algorithm Tuning Paramter
            {
                // Daylight locus offset LUTs for tungsten
                {
                    {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000}, // i4LUTIn
                    {0, 350,  800, 1222, 1444, 1667, 1889, 2111, 2333, 2556, 2778, 3000, 3222, 3444, 3667, 3889, 4111, 4333, 4556, 4778,  5000} // i4LUTOut
                },

                // Daylight locus offset LUTs for WF
                {
                    {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000}, // i4LUTIn
                {0, 350, 700, 1000, 1444, 1667, 1889, 2111, 2333, 2556, 2778, 3000, 3222, 3444, 3667, 3889, 4111, 4333, 4556, 4778, 5000} // i4LUTOut
                },

                // Daylight locus offset LUTs for shade
                {
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
                        70,        // i4LVThr_H
                        128         // i4DaylightProb
                    },

                    // Shade CWF Detection
                    {
                        1,          // i4Enable
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
                    //LV0  1    2    3    4    5    6    7    8    9     10    11   12   13   14   15  16   17   18
                    {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 100,  50,  25,   0,  0,   0,   0}, // Strobe
                    { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  75,  50,  25,   0,   0,   0}, // Tungsten
                    {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,   75,  50,  25,  25,  25,  0,   0,   0}, // Warm fluorescent
                    { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  95,  75,  50,  25,  25,  25,   0,   0,   0}, // Fluorescent
                    { 90,  90,  90,  90,  90,  90,  90,  90,  90,  90,   80,   55,  30,  30,  30,  30,  0,   0,   0}, // CWF
                    {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 100, 100,  75,  50, 40,  30,  20}, // Daylight
                    {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100,  75,  50,  25,   0,  0,   0,   0}, // Shade
                    { 90,  90,  90,  90,  90,  90,  90,  90,  90,  90,   80,   55,  30,  30,  30,  30,  0,   0,   0}  // Daylight fluorescent
                },
        // AWB advance function
        {
            // Confusion Color Component
            {
                0, // i4Enable
                0, // i4SingleColorWeight
                0, // i4ColorCheckerWeight
                0, // i4SubWindowProcWeight
                0, // Reserved1
                0, // Reserved2
                0, // Reserved3
                0, // Reserved4
                0 // Reserved5
            },
            // Face Component
            {
                0, // i4Enable
                0, // i4CompWeighting
                0, // i4CompLimit
                0, // Reserved1
                0, // Reserved2
                0, // Reserved3
                0, // Reserved4
                0, // Reserved5
            },
            // Confusion Color Component
            {
                0, // i4Enable
                // i4StatWinScaling
                {   0,  0,  0,  0,  0,  0,  0,  0},
                0, // Reserved1
                0, // Reserved2
                0, // Reserved3
                0, // Reserved4
                0 // Reserved5
            },
            // Light Sensor Assist
            {
                0, // i4Enable
                // i4WeightingLUT
                {   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
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
                // AWB preference color
                {
                    // Tungsten
                        {
                            5120    // TUNG_OFFS
                        },
                        // Warm fluorescent
                        {
                            5120    // WFluo_OFFS
                        },
                        // Shade
                        {
            909 // Shade_OFFS
                        },
                    // Sunset Area
                    {
                    23,   // i4Sunset_BoundXr_Thr
                    -338    // i4Sunset_BoundYr_Thr
                },
                    // Shade F Area
                    {
                    -222,   // i4BoundXrThr
                    -353    // i4BoundYrThr
                },
                    // Shade F Vertex
                    {
                    -142,   // i4BoundXrThr
                    -365    // i4BoundYrThr
                },
                    // Shade CWF Area
                    {
                    -222,   // i4BoundXrThr
                    -407    // i4BoundYrThr
                },
                    // Shade CWF Vertex
                    {
                    -139,   // i4BoundXrThr
                    -433    // i4BoundYrThr
                }
                },

            }
        },
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
                                842,    // D65Gain_R
                                512,    // D65Gain_G
                                726    // D65Gain_B
                    }
                },
                // Original XY coordinate of AWB light source
                {
                // Strobe
                {
                    0,    // i4X
                    0    // i4Y
                },
                // Horizon
                {
                    -423,    // OriX_Hor
                    -289    // OriY_Hor
                },
                // A
                {
                    -319,    // OriX_A
                    -311    // OriY_A
                },
                // TL84
                {
                    -166,    // OriX_TL84
                    -346    // OriY_TL84
                },
                // CWF
                {
                    -141,    // OriX_CWF
                    -401    // OriY_CWF
                },
                // DNP
                {
                    -19,    // OriX_DNP
                    -338    // OriY_DNP
                },
                // D65
                {
                    55,    // OriX_D65
                    -313    // OriY_D65
                },
                // DF
                {
                    41,    // OriX_DF
                    -389    // OriY_DF
                }
            },
                // Rotated XY coordinate of AWB light source
                {
                // Strobe
                {
                    0,    // i4X
                    0    // i4Y
                },
                // Horizon
                {
                    -418,    // RotX_Hor
                    -296    // RotY_Hor
                },
                // A
                {
                    -314,    // RotX_A
                    -316    // RotY_A
                },
                // TL84
                {
                    -161,    // RotX_TL84
                    -349    // RotY_TL84
                },
                // CWF
                {
                    -135,    // RotX_CWF
                    -403    // RotY_CWF
                },
                // DNP
                {
                    -14,    // RotX_DNP
                    -338    // RotY_DNP
                },
                // D65
                {
                    60,    // RotX_D65
                    -312    // RotY_D65
                },
                // DF
                {
                    47,    // RotX_DF
                    -388    // RotY_DF
                }
            },
                // AWB gain of AWB light source
                {
                // Strobe
                {
                    512,    // i4R
                    512,    // i4G
                    512    // i4B
                },
                // Horizon
                {
                    512,    // AWBGAIN_HOR_R
                    614,    // AWBGAIN_HOR_G
                    1609    // AWBGAIN_HOR_B
                },
                // A
                {
                    512,    // AWBGAIN_A_R
                    517,    // AWBGAIN_A_G
                    1213    // AWBGAIN_A_B
                },
                // TL84
                {
                    654,    // AWBGAIN_TL84_R
                    512,    // AWBGAIN_TL84_G
                    1024    // AWBGAIN_TL84_B
                },
                // CWF
                {
                    728,    // AWBGAIN_CWF_R
                    512,    // AWBGAIN_CWF_G
                    1066    // AWBGAIN_CWF_B
                },
                // DNP
                {
                    788,    // AWBGAIN_DNP_R
                    512,    // AWBGAIN_DNP_G
                    831    // AWBGAIN_DNP_B
                },
                // D65
                {
                    842,    // AWBGAIN_D65_R
                    512,    // AWBGAIN_D65_G
                    726    // AWBGAIN_D65_B
                },
                // DF
                {
                    916,    // AWBGAIN_DF_R
                    512,    // AWBGAIN_DF_G
                    820    // AWBGAIN_DF_B
                }
            },
                // Rotation matrix parameter
    {
                -1,    // RotationAngle
                256,    // Cos
                -4    // Sin
            },
    // Daylight locus parameter
    {
                    -124,    // i4SlopeNumerator
                    128    // i4SlopeDenominator
            },
            // Predictor gain
            {
                // DaylightLocus_L
                {
                    812,    // i4R
                    530,    // i4G
                    746    // i4B
                },
                // DaylightLocus_H
                {
                    750,    // i4R
                    512,    // i4G
                    812    // i4B
                },
                // Temporal General
                {
                    842,    // i4R
                    512,    // i4G
                    726    // i4B
                },
            },
                // AWB light area
                {
                // Strobe:FIXME
                {
                    0,    // i4RightBound
                    0,    // i4LeftBound
                    0,    // i4UpperBound
                    0    // i4LowerBound
                },
                // Tungsten
                {
                    -222,    // TungRightBound
                    -818,    // TungLeftBound
                    -241,    // TungUpperBound
                    -333    // TungLowerBound
                },
                // Warm fluorescent
                {
                    -222,    // WFluoRightBound
                    -818,    // WFluoLeftBound
                    -333,    // WFluoUpperBound
                    -493    // WFluoLowerBound
                },
                // Fluorescent
                {
                    -62,    // FluoRightBound
                    -222,    // FluoLeftBound
                    -252,    // FluoUpperBound
                    -376    // FluoLowerBound
                },
                // CWF
                {
                -55,    // CWFRightBound
                -222,    // CWFLeftBound
                -376,    // CWFUpperBound
                -458    // CWFLowerBound
                },
                // Daylight
                {
                    90,    // DayRightBound
                    -62,    // DayLeftBound
                    -252,    // DayUpperBound
                    -376    // DayLowerBound
                },
                // Shade
                {
                    420,    // ShadeRightBound
                    90,    // ShadeLeftBound
                    -252,    // ShadeUpperBound
                    -348    // ShadeLowerBound
                },
                // Daylight Fluorescent
                {
                    90,    // DFRightBound
                    -55,    // DFLeftBound
                    -376,    // DFUpperBound
                    -458    // DFLowerBound
                }
            },
    // PWB light area
    {
                // Reference area
                {
                    420,    // PRefRightBound
                    -818,    // PRefLeftBound
                    -216,    // PRefUpperBound
                    -493    // PRefLowerBound
                },
                // Daylight
                {
                    115,    // PDayRightBound
                    -62,    // PDayLeftBound
                    -252,    // PDayUpperBound
                    -376    // PDayLowerBound
                },
                // Cloudy daylight
                {
                    215,    // PCloudyRightBound
                    40,    // PCloudyLeftBound
                    -252,    // PCloudyUpperBound
                    -376    // PCloudyLowerBound
                },
                // Shade
                {
                    315,    // PShadeRightBound
                    40,    // PShadeLeftBound
                    -252,    // PShadeUpperBound
                    -376    // PShadeLowerBound
                },
                // Twilight
                {
                    -62,    // PTwiRightBound
                    -222,    // PTwiLeftBound
                    -252,    // PTwiUpperBound
                    -376    // PTwiLowerBound
                },
                // Fluorescent
                {
                    110,    // PFluoRightBound
                    -261,    // PFluoLeftBound
                    -262,    // PFluoUpperBound
                    -453    // PFluoLowerBound
                },
                // Warm fluorescent
                {
                    -214,    // PWFluoRightBound
                    -414,    // PWFluoLeftBound
                    -262,    // PWFluoUpperBound
                    -453    // PWFluoLowerBound
                },
                // Incandescent
                {
                    -214,    // PIncaRightBound
                    -414,    // PIncaLeftBound
                    -252,    // PIncaUpperBound
                    -376    // PIncaLowerBound
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
                    807,    // PWB_Day_R
                    512,    // PWB_Day_G
                    761    // PWB_Day_B
                },
                // Cloudy daylight
                {
                    927,    // PWB_Cloudy_R
                    512,    // PWB_Cloudy_G
                    665    // PWB_Cloudy_B
                },
                // Shade
                {
                    993,    // PWB_Shade_R
                    512,    // PWB_Shade_G
                    622    // PWB_Shade_B
                },
                // Twilight
                {
                    640,    // PWB_Twi_R
                    512,    // PWB_Twi_G
                    953    // PWB_Twi_B
                },
                // Fluorescent
                {
                    743,    // PWB_Fluo_R
                    512,    // PWB_Fluo_G
                    926    // PWB_Fluo_B
                },
                // Warm fluorescent
                {
                    535,    // PWB_WFluo_R
                    512,    // PWB_WFluo_G
                    1272    // PWB_WFluo_B
                },
                // Incandescent
                {
                    505,    // PWB_Inca_R
                    512,    // PWB_Inca_G
                    1198    // PWB_Inca_B
                },
                // Gray World
                {
                    512,    // PWB_GW_R
                    512,    // PWB_GW_G
                    512    // PWB_GW_B
                }
            },
                // CCT estimation
                {
                    // CCT
                    {
                        2300,   // i4CCT[0]
                        2850,   // i4CCT[1]
                        3750,   // i4CCT[2]
                        5100,   // i4CCT[3]
                        6500    // i4CCT[4]
                    },
                    // Rotated X coordinate
                    {
                -478,    // i4RotatedXCoordinate[0]
                -374,    // i4RotatedXCoordinate[1]
                -221,    // i4RotatedXCoordinate[2]
                -74,    // i4RotatedXCoordinate[3]
                0    // i4RotatedXCoordinate[4]
                }
                }
            },

            // Algorithm Tuning Paramter
            {
                // Daylight locus offset LUTs for tungsten
                {
                    {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000}, // i4LUTIn
                    {0, 350,  800, 1222, 1444, 1667, 1889, 2111, 2333, 2556, 2778, 3000, 3222, 3444, 3667, 3889, 4111, 4333, 4556, 4778,  5000} // i4LUTOut
                },

                // Daylight locus offset LUTs for WF
                {
                    {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000}, // i4LUTIn
                {0, 350, 700, 1000, 1444, 1667, 1889, 2111, 2333, 2556, 2778, 3000, 3222, 3444, 3667, 3889, 4111, 4333, 4556, 4778, 5000} // i4LUTOut
                },

                // Daylight locus offset LUTs for shade
                {
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
                        70,        // i4LVThr_H
                        128         // i4DaylightProb
                    },

                    // Shade CWF Detection
                    {
                        1,          // i4Enable
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
                    //LV0  1    2    3    4    5    6    7    8    9     10    11   12   13   14   15  16   17   18
                    {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 100,  50,  25,   0,  0,   0,   0}, // Strobe
                    { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  75,  50,  25,   0,   0,   0}, // Tungsten
                    {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,   75,  50,  25,  25,  25,  0,   0,   0}, // Warm fluorescent
                    { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  95,  75,  50,  25,  25,  25,   0,   0,   0}, // Fluorescent
                    { 90,  90,  90,  90,  90,  90,  90,  90,  90,  90,   80,   55,  30,  30,  30,  30,  0,   0,   0}, // CWF
                    {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 100, 100,  75,  50, 40,  30,  20}, // Daylight
                    {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100,  75,  50,  25,   0,  0,   0,   0}, // Shade
                    { 90,  90,  90,  90,  90,  90,  90,  90,  90,  90,   80,   55,  30,  30,  30,  30,  0,   0,   0}  // Daylight fluorescent
                },
        // AWB advance function
        {
            // Confusion Color Component
            {
                0, // i4Enable
                0, // i4SingleColorWeight
                0, // i4ColorCheckerWeight
                0, // i4SubWindowProcWeight
                0, // Reserved1
                0, // Reserved2
                0, // Reserved3
                0, // Reserved4
                0 // Reserved5
            },
            // Face Component
            {
                0, // i4Enable
                0, // i4CompWeighting
                0, // i4CompLimit
                0, // Reserved1
                0, // Reserved2
                0, // Reserved3
                0, // Reserved4
                0, // Reserved5
            },
            // Confusion Color Component
            {
                0, // i4Enable
                // i4StatWinScaling
                {   0,  0,  0,  0,  0,  0,  0,  0},
                0, // Reserved1
                0, // Reserved2
                0, // Reserved3
                0, // Reserved4
                0 // Reserved5
            },
            // Light Sensor Assist
            {
                0, // i4Enable
                // i4WeightingLUT
                {   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
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
                // AWB preference color
                {
                    // Tungsten
                        {
                            5120    // TUNG_OFFS
                        },
                        // Warm fluorescent
                        {
                            5120    // WFluo_OFFS
                        },
                        // Shade
                        {
            909 // Shade_OFFS
                        },
                    // Sunset Area
                    {
                    23,   // i4Sunset_BoundXr_Thr
                    -338    // i4Sunset_BoundYr_Thr
                },
                    // Shade F Area
                    {
                    -222,   // i4BoundXrThr
                    -353    // i4BoundYrThr
                },
                    // Shade F Vertex
                    {
                    -142,   // i4BoundXrThr
                    -365    // i4BoundYrThr
                },
                    // Shade CWF Area
                    {
                    -222,   // i4BoundXrThr
                    -407    // i4BoundYrThr
                },
                    // Shade CWF Vertex
                    {
                    -139,   // i4BoundXrThr
                    -433    // i4BoundYrThr
                }
                },

            }
        },

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
                                842,    // D65Gain_R
                                512,    // D65Gain_G
                                726    // D65Gain_B
                    }
                },
                // Original XY coordinate of AWB light source
                {
                // Strobe
                {
                    0,    // i4X
                    0    // i4Y
                },
                // Horizon
                {
                    -423,    // OriX_Hor
                    -289    // OriY_Hor
                },
                // A
                {
                    -319,    // OriX_A
                    -311    // OriY_A
                },
                // TL84
                {
                    -166,    // OriX_TL84
                    -346    // OriY_TL84
                },
                // CWF
                {
                    -141,    // OriX_CWF
                    -401    // OriY_CWF
                },
                // DNP
                {
                    -19,    // OriX_DNP
                    -338    // OriY_DNP
                },
                // D65
                {
                    55,    // OriX_D65
                    -313    // OriY_D65
                },
                // DF
                {
                    41,    // OriX_DF
                    -389    // OriY_DF
                }
            },
                // Rotated XY coordinate of AWB light source
                {
                // Strobe
                {
                    0,    // i4X
                    0    // i4Y
                },
                // Horizon
                {
                    -418,    // RotX_Hor
                    -296    // RotY_Hor
                },
                // A
                {
                    -314,    // RotX_A
                    -316    // RotY_A
                },
                // TL84
                {
                    -161,    // RotX_TL84
                    -349    // RotY_TL84
                },
                // CWF
                {
                    -135,    // RotX_CWF
                    -403    // RotY_CWF
                },
                // DNP
                {
                    -14,    // RotX_DNP
                    -338    // RotY_DNP
                },
                // D65
                {
                    60,    // RotX_D65
                    -312    // RotY_D65
                },
                // DF
                {
                    47,    // RotX_DF
                    -388    // RotY_DF
                }
            },
                // AWB gain of AWB light source
                {
                // Strobe
                {
                    512,    // i4R
                    512,    // i4G
                    512    // i4B
                },
                // Horizon
                {
                    512,    // AWBGAIN_HOR_R
                    614,    // AWBGAIN_HOR_G
                    1609    // AWBGAIN_HOR_B
                },
                // A
                {
                    512,    // AWBGAIN_A_R
                    517,    // AWBGAIN_A_G
                    1213    // AWBGAIN_A_B
                },
                // TL84
                {
                    654,    // AWBGAIN_TL84_R
                    512,    // AWBGAIN_TL84_G
                    1024    // AWBGAIN_TL84_B
                },
                // CWF
                {
                    728,    // AWBGAIN_CWF_R
                    512,    // AWBGAIN_CWF_G
                    1066    // AWBGAIN_CWF_B
                },
                // DNP
                {
                    788,    // AWBGAIN_DNP_R
                    512,    // AWBGAIN_DNP_G
                    831    // AWBGAIN_DNP_B
                },
                // D65
                {
                    842,    // AWBGAIN_D65_R
                    512,    // AWBGAIN_D65_G
                    726    // AWBGAIN_D65_B
                },
                // DF
                {
                    916,    // AWBGAIN_DF_R
                    512,    // AWBGAIN_DF_G
                    820    // AWBGAIN_DF_B
                }
            },
                // Rotation matrix parameter
    {
                -1,    // RotationAngle
                256,    // Cos
                -4    // Sin
            },
    // Daylight locus parameter
    {
                    -124,    // i4SlopeNumerator
                    128    // i4SlopeDenominator
            },
            // Predictor gain
            {
                // DaylightLocus_L
                {
                    812,    // i4R
                    530,    // i4G
                    746    // i4B
                },
                // DaylightLocus_H
                {
                    750,    // i4R
                    512,    // i4G
                    812    // i4B
                },
                // Temporal General
                {
                    842,    // i4R
                    512,    // i4G
                    726    // i4B
                },
            },
                // AWB light area
                {
                // Strobe:FIXME
                {
                    0,    // i4RightBound
                    0,    // i4LeftBound
                    0,    // i4UpperBound
                    0    // i4LowerBound
                },
                // Tungsten
                {
                    -222,    // TungRightBound
                    -818,    // TungLeftBound
                    -241,    // TungUpperBound
                    -333    // TungLowerBound
                },
                // Warm fluorescent
                {
                    -222,    // WFluoRightBound
                    -818,    // WFluoLeftBound
                    -333,    // WFluoUpperBound
                    -493    // WFluoLowerBound
                },
                // Fluorescent
                {
                    -62,    // FluoRightBound
                    -222,    // FluoLeftBound
                    -252,    // FluoUpperBound
                    -376    // FluoLowerBound
                },
                // CWF
                {
                -55,    // CWFRightBound
                -222,    // CWFLeftBound
                -376,    // CWFUpperBound
                -458    // CWFLowerBound
                },
                // Daylight
                {
                    90,    // DayRightBound
                    -62,    // DayLeftBound
                    -252,    // DayUpperBound
                    -376    // DayLowerBound
                },
                // Shade
                {
                    420,    // ShadeRightBound
                    90,    // ShadeLeftBound
                    -252,    // ShadeUpperBound
                    -348    // ShadeLowerBound
                },
                // Daylight Fluorescent
                {
                    90,    // DFRightBound
                    -55,    // DFLeftBound
                    -376,    // DFUpperBound
                    -458    // DFLowerBound
                }
            },
    // PWB light area
    {
                // Reference area
                {
                    420,    // PRefRightBound
                    -818,    // PRefLeftBound
                    -216,    // PRefUpperBound
                    -493    // PRefLowerBound
                },
                // Daylight
                {
                    115,    // PDayRightBound
                    -62,    // PDayLeftBound
                    -252,    // PDayUpperBound
                    -376    // PDayLowerBound
                },
                // Cloudy daylight
                {
                    215,    // PCloudyRightBound
                    40,    // PCloudyLeftBound
                    -252,    // PCloudyUpperBound
                    -376    // PCloudyLowerBound
                },
                // Shade
                {
                    315,    // PShadeRightBound
                    40,    // PShadeLeftBound
                    -252,    // PShadeUpperBound
                    -376    // PShadeLowerBound
                },
                // Twilight
                {
                    -62,    // PTwiRightBound
                    -222,    // PTwiLeftBound
                    -252,    // PTwiUpperBound
                    -376    // PTwiLowerBound
                },
                // Fluorescent
                {
                    110,    // PFluoRightBound
                    -261,    // PFluoLeftBound
                    -262,    // PFluoUpperBound
                    -453    // PFluoLowerBound
                },
                // Warm fluorescent
                {
                    -214,    // PWFluoRightBound
                    -414,    // PWFluoLeftBound
                    -262,    // PWFluoUpperBound
                    -453    // PWFluoLowerBound
                },
                // Incandescent
                {
                    -214,    // PIncaRightBound
                    -414,    // PIncaLeftBound
                    -252,    // PIncaUpperBound
                    -376    // PIncaLowerBound
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
                    807,    // PWB_Day_R
                    512,    // PWB_Day_G
                    761    // PWB_Day_B
                },
                // Cloudy daylight
                {
                    927,    // PWB_Cloudy_R
                    512,    // PWB_Cloudy_G
                    665    // PWB_Cloudy_B
                },
                // Shade
                {
                    993,    // PWB_Shade_R
                    512,    // PWB_Shade_G
                    622    // PWB_Shade_B
                },
                // Twilight
                {
                    640,    // PWB_Twi_R
                    512,    // PWB_Twi_G
                    953    // PWB_Twi_B
                },
                // Fluorescent
                {
                    743,    // PWB_Fluo_R
                    512,    // PWB_Fluo_G
                    926    // PWB_Fluo_B
                },
                // Warm fluorescent
                {
                    535,    // PWB_WFluo_R
                    512,    // PWB_WFluo_G
                    1272    // PWB_WFluo_B
                },
                // Incandescent
                {
                    505,    // PWB_Inca_R
                    512,    // PWB_Inca_G
                    1198    // PWB_Inca_B
                },
                // Gray World
                {
                    512,    // PWB_GW_R
                    512,    // PWB_GW_G
                    512    // PWB_GW_B
                }
            },
                // CCT estimation
                {
                    // CCT
                    {
                        2300,   // i4CCT[0]
                        2850,   // i4CCT[1]
                        3750,   // i4CCT[2]
                        5100,   // i4CCT[3]
                        6500    // i4CCT[4]
                    },
                    // Rotated X coordinate
                    {
                -478,    // i4RotatedXCoordinate[0]
                -374,    // i4RotatedXCoordinate[1]
                -221,    // i4RotatedXCoordinate[2]
                -74,    // i4RotatedXCoordinate[3]
                0    // i4RotatedXCoordinate[4]
                }
                }
            },

            // Algorithm Tuning Paramter
            {
                // Daylight locus offset LUTs for tungsten
                {
                    {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000}, // i4LUTIn
                    {0, 350,  800, 1222, 1444, 1667, 1889, 2111, 2333, 2556, 2778, 3000, 3222, 3444, 3667, 3889, 4111, 4333, 4556, 4778,  5000} // i4LUTOut
                },

                // Daylight locus offset LUTs for WF
                {
                    {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000}, // i4LUTIn
                {0, 350, 700, 1000, 1444, 1667, 1889, 2111, 2333, 2556, 2778, 3000, 3222, 3444, 3667, 3889, 4111, 4333, 4556, 4778, 5000} // i4LUTOut
                },

                // Daylight locus offset LUTs for shade
                {
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
                        70,        // i4LVThr_H
                        128         // i4DaylightProb
                    },

                    // Shade CWF Detection
                    {
                        1,          // i4Enable
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
                    //LV0  1    2    3    4    5    6    7    8    9     10    11   12   13   14   15  16   17   18
                    {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 100,  50,  25,   0,  0,   0,   0}, // Strobe
                    { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  75,  50,  25,   0,   0,   0}, // Tungsten
                    {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,   75,  50,  25,  25,  25,  0,   0,   0}, // Warm fluorescent
                    { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  95,  75,  50,  25,  25,  25,   0,   0,   0}, // Fluorescent
                    { 90,  90,  90,  90,  90,  90,  90,  90,  90,  90,   80,   55,  30,  30,  30,  30,  0,   0,   0}, // CWF
                    {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 100, 100,  75,  50, 40,  30,  20}, // Daylight
                    {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100,  75,  50,  25,   0,  0,   0,   0}, // Shade
                    { 90,  90,  90,  90,  90,  90,  90,  90,  90,  90,   80,   55,  30,  30,  30,  30,  0,   0,   0}  // Daylight fluorescent
                },
        // AWB advance function
        {
            // Confusion Color Component
            {
                0, // i4Enable
                0, // i4SingleColorWeight
                0, // i4ColorCheckerWeight
                0, // i4SubWindowProcWeight
                0, // Reserved1
                0, // Reserved2
                0, // Reserved3
                0, // Reserved4
                0 // Reserved5
            },
            // Face Component
            {
                0, // i4Enable
                0, // i4CompWeighting
                0, // i4CompLimit
                0, // Reserved1
                0, // Reserved2
                0, // Reserved3
                0, // Reserved4
                0, // Reserved5
            },
            // Confusion Color Component
            {
                0, // i4Enable
                // i4StatWinScaling
                {   0,  0,  0,  0,  0,  0,  0,  0},
                0, // Reserved1
                0, // Reserved2
                0, // Reserved3
                0, // Reserved4
                0 // Reserved5
            },
            // Light Sensor Assist
            {
                0, // i4Enable
                // i4WeightingLUT
                {   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
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
                // AWB preference color
                {
                    // Tungsten
                        {
                            5120    // TUNG_OFFS
                        },
                        // Warm fluorescent
                        {
                            5120    // WFluo_OFFS
                        },
                        // Shade
                        {
            909 // Shade_OFFS
                        },
                    // Sunset Area
                    {
                    23,   // i4Sunset_BoundXr_Thr
                    -338    // i4Sunset_BoundYr_Thr
                },
                    // Shade F Area
                    {
                    -222,   // i4BoundXrThr
                    -353    // i4BoundYrThr
                },
                    // Shade F Vertex
                    {
                    -142,   // i4BoundXrThr
                    -365    // i4BoundYrThr
                },
                    // Shade CWF Area
                    {
                    -222,   // i4BoundXrThr
                    -407    // i4BoundYrThr
                },
                    // Shade CWF Vertex
                    {
                    -139,   // i4BoundXrThr
                    -433    // i4BoundYrThr
                }
                },

            }
        }
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

    if (CameraDataType > CAMERA_NVRAM_DATA_FEATURE || NULL == pDataBuf || (size != dataSize[CameraDataType]))
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

