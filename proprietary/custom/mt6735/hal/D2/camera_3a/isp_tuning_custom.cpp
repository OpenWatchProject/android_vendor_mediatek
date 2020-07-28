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

#define LOG_TAG "isp_tuning_custom"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <aaa_types.h>
#include <aaa_log.h>
#include <camera_custom_nvram.h>
#include <isp_tuning.h>
#include <awb_param.h>
#include <ae_param.h>
#include <af_param.h>
#include <flash_param.h>
#include <isp_tuning_cam_info.h>
#include <isp_tuning_idx.h>
#include <isp_tuning_custom.h>
#include <isp_tuning_custom_instance.h>
#include <stdlib.h> // For atoi()
#include <stdio.h>
#include <cutils/properties.h> // For property_get().
//#include "camera_custom_3dnr.h"
#include <mtkcam/algorithm/libgma/MTKGma.h>

using namespace NSIspTuning;

static MINT32 customRatioLog=0;

static MTK_GMA_ENV_INFO_STRUCT gsGMAEnvParam_main =
{
    {
        {
            {
                0x2000, 0x2008, 0x2010, 0x2018, 0x2820, 0x282A, 0x2034, 0x203C, 0x2844, 0x284E,
                0x2058, 0x2060, 0x2068, 0x2070, 0x2078, 0x2080, 0x1888, 0x188E, 0x2094, 0x209C,
                0x18A4, 0x18AA, 0x18B0, 0x18B6, 0x20BC, 0x20C4, 0x18CC, 0x18D2, 0x18D8, 0x18DE,
                0x18E4, 0x18EA, 0x18F0, 0x18F6, 0x18FC, 0x1902, 0x1908, 0x190E, 0x1114, 0x1118,
                0x191C, 0x1922, 0x1928, 0x192E, 0x1134, 0x1138, 0x193C, 0x1942, 0x1148, 0x114C,
                0x1950, 0x1956, 0x115C, 0x1160, 0x1164, 0x1168, 0x196C, 0x1972, 0x1178, 0x117C,
                0x1180, 0x1184, 0x1188, 0x118C, 0x2190, 0x2198, 0x21A0, 0x21A8, 0x21B0, 0x21B8,
                0x21C0, 0x11C8, 0x21CC, 0x21D4, 0x21DC, 0x11E4, 0x21E8, 0x21F0, 0x11F8, 0x21FC,
                0x2204, 0x120C, 0x2210, 0x1218, 0x121C, 0x2220, 0x1228, 0x222C, 0x1234, 0x1238,
                0x223C, 0x1244, 0x1248, 0x224C, 0x1254, 0x1258, 0x225C, 0x3264, 0x2270, 0x2278,
                0x2280, 0x2288, 0x2290, 0x2298, 0x22A0, 0x22A8, 0x22B0, 0x22B8, 0x22C0, 0x12C8,
                0x22CC, 0x22D4, 0x12DC, 0x22E0, 0x22E8, 0x12F0, 0x12F4, 0x22F8, 0x1300, 0x2304,
                0x130C, 0x1310, 0x2314, 0x131C, 0x1320, 0x2324, 0x132C, 0x1330, 0x5334, 0x4348,
                0x4358, 0x4368, 0x4378, 0x3388, 0x3394, 0x33A0, 0x33AC, 0x33B8, 0x33C4, 0x23D0,
                0x33D8, 0x23E4, 0x23EC, 0x2BF4, 0xFFFF
            },
            {
                0x2000, 0x2008, 0x2010, 0x2018, 0x2820, 0x282A, 0x2034, 0x203C, 0x2844, 0x284E,
                0x2058, 0x2060, 0x2068, 0x2070, 0x2078, 0x2080, 0x1888, 0x188E, 0x2094, 0x209C,
                0x18A4, 0x18AA, 0x18B0, 0x18B6, 0x20BC, 0x20C4, 0x18CC, 0x18D2, 0x18D8, 0x18DE,
                0x18E4, 0x18EA, 0x18F0, 0x18F6, 0x18FC, 0x1902, 0x1908, 0x190E, 0x1114, 0x1118,
                0x191C, 0x1922, 0x1928, 0x192E, 0x1134, 0x1138, 0x193C, 0x1942, 0x1148, 0x114C,
                0x1950, 0x1956, 0x115C, 0x1160, 0x1164, 0x1168, 0x196C, 0x1972, 0x1178, 0x117C,
                0x1180, 0x1184, 0x1188, 0x118C, 0x2190, 0x2198, 0x21A0, 0x21A8, 0x21B0, 0x21B8,
                0x21C0, 0x11C8, 0x21CC, 0x21D4, 0x21DC, 0x11E4, 0x21E8, 0x21F0, 0x11F8, 0x21FC,
                0x2204, 0x120C, 0x2210, 0x1218, 0x121C, 0x2220, 0x1228, 0x222C, 0x1234, 0x1238,
                0x223C, 0x1244, 0x1248, 0x224C, 0x1254, 0x1258, 0x225C, 0x3264, 0x2270, 0x2278,
                0x2280, 0x2288, 0x2290, 0x2298, 0x22A0, 0x22A8, 0x22B0, 0x22B8, 0x22C0, 0x12C8,
                0x22CC, 0x22D4, 0x12DC, 0x22E0, 0x22E8, 0x12F0, 0x12F4, 0x22F8, 0x1300, 0x2304,
                0x130C, 0x1310, 0x2314, 0x131C, 0x1320, 0x2324, 0x132C, 0x1330, 0x5334, 0x4348,
                0x4358, 0x4368, 0x4378, 0x3388, 0x3394, 0x33A0, 0x33AC, 0x33B8, 0x33C4, 0x23D0,
                0x33D8, 0x23E4, 0x23EC, 0x2BF4, 0xFFFF
            },
            {
                0x2000, 0x2008, 0x2010, 0x2018, 0x2820, 0x282A, 0x2034, 0x203C, 0x2844, 0x284E,
                0x2058, 0x2060, 0x2068, 0x2070, 0x2078, 0x2080, 0x1888, 0x188E, 0x2094, 0x209C,
                0x18A4, 0x18AA, 0x18B0, 0x18B6, 0x20BC, 0x20C4, 0x18CC, 0x18D2, 0x18D8, 0x18DE,
                0x18E4, 0x18EA, 0x18F0, 0x18F6, 0x18FC, 0x1902, 0x1908, 0x190E, 0x1114, 0x1118,
                0x191C, 0x1922, 0x1928, 0x192E, 0x1134, 0x1138, 0x193C, 0x1942, 0x1148, 0x114C,
                0x1950, 0x1956, 0x115C, 0x1160, 0x1164, 0x1168, 0x196C, 0x1972, 0x1178, 0x117C,
                0x1180, 0x1184, 0x1188, 0x118C, 0x2190, 0x2198, 0x21A0, 0x21A8, 0x21B0, 0x21B8,
                0x21C0, 0x11C8, 0x21CC, 0x21D4, 0x21DC, 0x11E4, 0x21E8, 0x21F0, 0x11F8, 0x21FC,
                0x2204, 0x120C, 0x2210, 0x1218, 0x121C, 0x2220, 0x1228, 0x222C, 0x1234, 0x1238,
                0x223C, 0x1244, 0x1248, 0x224C, 0x1254, 0x1258, 0x225C, 0x3264, 0x2270, 0x2278,
                0x2280, 0x2288, 0x2290, 0x2298, 0x22A0, 0x22A8, 0x22B0, 0x22B8, 0x22C0, 0x12C8,
                0x22CC, 0x22D4, 0x12DC, 0x22E0, 0x22E8, 0x12F0, 0x12F4, 0x22F8, 0x1300, 0x2304,
                0x130C, 0x1310, 0x2314, 0x131C, 0x1320, 0x2324, 0x132C, 0x1330, 0x5334, 0x4348,
                0x4358, 0x4368, 0x4378, 0x3388, 0x3394, 0x33A0, 0x33AC, 0x33B8, 0x33C4, 0x23D0,
                0x33D8, 0x23E4, 0x23EC, 0x2BF4, 0xFFFF
            },
            {
                0x2000, 0x2008, 0x2010, 0x2018, 0x2820, 0x282A, 0x2034, 0x203C, 0x2844, 0x284E,
                0x2058, 0x2060, 0x2068, 0x2070, 0x2078, 0x2080, 0x1888, 0x188E, 0x2094, 0x209C,
                0x18A4, 0x18AA, 0x18B0, 0x18B6, 0x20BC, 0x20C4, 0x18CC, 0x18D2, 0x18D8, 0x18DE,
                0x18E4, 0x18EA, 0x18F0, 0x18F6, 0x18FC, 0x1902, 0x1908, 0x190E, 0x1114, 0x1118,
                0x191C, 0x1922, 0x1928, 0x192E, 0x1134, 0x1138, 0x193C, 0x1942, 0x1148, 0x114C,
                0x1950, 0x1956, 0x115C, 0x1160, 0x1164, 0x1168, 0x196C, 0x1972, 0x1178, 0x117C,
                0x1180, 0x1184, 0x1188, 0x118C, 0x2190, 0x2198, 0x21A0, 0x21A8, 0x21B0, 0x21B8,
                0x21C0, 0x11C8, 0x21CC, 0x21D4, 0x21DC, 0x11E4, 0x21E8, 0x21F0, 0x11F8, 0x21FC,
                0x2204, 0x120C, 0x2210, 0x1218, 0x121C, 0x2220, 0x1228, 0x222C, 0x1234, 0x1238,
                0x223C, 0x1244, 0x1248, 0x224C, 0x1254, 0x1258, 0x225C, 0x3264, 0x2270, 0x2278,
                0x2280, 0x2288, 0x2290, 0x2298, 0x22A0, 0x22A8, 0x22B0, 0x22B8, 0x22C0, 0x12C8,
                0x22CC, 0x22D4, 0x12DC, 0x22E0, 0x22E8, 0x12F0, 0x12F4, 0x22F8, 0x1300, 0x2304,
                0x130C, 0x1310, 0x2314, 0x131C, 0x1320, 0x2324, 0x132C, 0x1330, 0x5334, 0x4348,
                0x4358, 0x4368, 0x4378, 0x3388, 0x3394, 0x33A0, 0x33AC, 0x33B8, 0x33C4, 0x23D0,
                0x33D8, 0x23E4, 0x23EC, 0x2BF4, 0xFFFF
            },
            {
                0x2000, 0x2008, 0x2010, 0x2018, 0x2820, 0x282A, 0x2034, 0x203C, 0x2844, 0x284E,
                0x2058, 0x2060, 0x2068, 0x2070, 0x2078, 0x2080, 0x1888, 0x188E, 0x2094, 0x209C,
                0x18A4, 0x18AA, 0x18B0, 0x18B6, 0x20BC, 0x20C4, 0x18CC, 0x18D2, 0x18D8, 0x18DE,
                0x18E4, 0x18EA, 0x18F0, 0x18F6, 0x18FC, 0x1902, 0x1908, 0x190E, 0x1114, 0x1118,
                0x191C, 0x1922, 0x1928, 0x192E, 0x1134, 0x1138, 0x193C, 0x1942, 0x1148, 0x114C,
                0x1950, 0x1956, 0x115C, 0x1160, 0x1164, 0x1168, 0x196C, 0x1972, 0x1178, 0x117C,
                0x1180, 0x1184, 0x1188, 0x118C, 0x2190, 0x2198, 0x21A0, 0x21A8, 0x21B0, 0x21B8,
                0x21C0, 0x11C8, 0x21CC, 0x21D4, 0x21DC, 0x11E4, 0x21E8, 0x21F0, 0x11F8, 0x21FC,
                0x2204, 0x120C, 0x2210, 0x1218, 0x121C, 0x2220, 0x1228, 0x222C, 0x1234, 0x1238,
                0x223C, 0x1244, 0x1248, 0x224C, 0x1254, 0x1258, 0x225C, 0x3264, 0x2270, 0x2278,
                0x2280, 0x2288, 0x2290, 0x2298, 0x22A0, 0x22A8, 0x22B0, 0x22B8, 0x22C0, 0x12C8,
                0x22CC, 0x22D4, 0x12DC, 0x22E0, 0x22E8, 0x12F0, 0x12F4, 0x22F8, 0x1300, 0x2304,
                0x130C, 0x1310, 0x2314, 0x131C, 0x1320, 0x2324, 0x132C, 0x1330, 0x5334, 0x4348,
                0x4358, 0x4368, 0x4378, 0x3388, 0x3394, 0x33A0, 0x33AC, 0x33B8, 0x33C4, 0x23D0,
                0x33D8, 0x23E4, 0x23EC, 0x2BF4, 0xFFFF
            }
        }
    },
    {
        eDYNAMIC_GMA_MODE,  // eGMAMode
        8,                  // i4LowContrastThr
        {
            {   // i4ContrastWeightingTbl
                //  0   1   2    3    4    5    6    7    8    9    10
                    0,  0,  0,  33,  66, 100, 100,  100, 100, 100, 100
            },
            {   // i4LVWeightingTbl
                //LV0   1   2   3   4   5   6   7   8   9   10   11   12   13   14   15   16   17   18   19
                    0,  0,  0,  0,  0,  0,  0,  0,  0, 33,  66, 100, 100, 100, 100, 100, 100, 100, 100, 100
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
};

#define ISP_TUN_LOG_BIT  (1<<0)
#define ISP_TUN_EN_BIT1  (1<<1)
#define ISP_TUN_EN_BIT2  (1<<2)

static MTK_GMA_ENV_INFO_STRUCT gsGMAEnvParam_sub =
{
    {
        {
            {
                0x2000, 0x2008, 0x2010, 0x2018, 0x2820, 0x282A, 0x2034, 0x203C, 0x2844, 0x284E,
                0x2058, 0x2060, 0x2068, 0x2070, 0x2078, 0x2080, 0x1888, 0x188E, 0x2094, 0x209C,
                0x18A4, 0x18AA, 0x18B0, 0x18B6, 0x20BC, 0x20C4, 0x18CC, 0x18D2, 0x18D8, 0x18DE,
                0x18E4, 0x18EA, 0x18F0, 0x18F6, 0x18FC, 0x1902, 0x1908, 0x190E, 0x1114, 0x1118,
                0x191C, 0x1922, 0x1928, 0x192E, 0x1134, 0x1138, 0x193C, 0x1942, 0x1148, 0x114C,
                0x1950, 0x1956, 0x115C, 0x1160, 0x1164, 0x1168, 0x196C, 0x1972, 0x1178, 0x117C,
                0x1180, 0x1184, 0x1188, 0x118C, 0x2190, 0x2198, 0x21A0, 0x21A8, 0x21B0, 0x21B8,
                0x21C0, 0x11C8, 0x21CC, 0x21D4, 0x21DC, 0x11E4, 0x21E8, 0x21F0, 0x11F8, 0x21FC,
                0x2204, 0x120C, 0x2210, 0x1218, 0x121C, 0x2220, 0x1228, 0x222C, 0x1234, 0x1238,
                0x223C, 0x1244, 0x1248, 0x224C, 0x1254, 0x1258, 0x225C, 0x3264, 0x2270, 0x2278,
                0x2280, 0x2288, 0x2290, 0x2298, 0x22A0, 0x22A8, 0x22B0, 0x22B8, 0x22C0, 0x12C8,
                0x22CC, 0x22D4, 0x12DC, 0x22E0, 0x22E8, 0x12F0, 0x12F4, 0x22F8, 0x1300, 0x2304,
                0x130C, 0x1310, 0x2314, 0x131C, 0x1320, 0x2324, 0x132C, 0x1330, 0x5334, 0x4348,
                0x4358, 0x4368, 0x4378, 0x3388, 0x3394, 0x33A0, 0x33AC, 0x33B8, 0x33C4, 0x23D0,
                0x33D8, 0x23E4, 0x23EC, 0x2BF4, 0xFFFF
            },
            {
                0x2000, 0x2008, 0x2010, 0x2018, 0x2820, 0x282A, 0x2034, 0x203C, 0x2844, 0x284E,
                0x2058, 0x2060, 0x2068, 0x2070, 0x2078, 0x2080, 0x1888, 0x188E, 0x2094, 0x209C,
                0x18A4, 0x18AA, 0x18B0, 0x18B6, 0x20BC, 0x20C4, 0x18CC, 0x18D2, 0x18D8, 0x18DE,
                0x18E4, 0x18EA, 0x18F0, 0x18F6, 0x18FC, 0x1902, 0x1908, 0x190E, 0x1114, 0x1118,
                0x191C, 0x1922, 0x1928, 0x192E, 0x1134, 0x1138, 0x193C, 0x1942, 0x1148, 0x114C,
                0x1950, 0x1956, 0x115C, 0x1160, 0x1164, 0x1168, 0x196C, 0x1972, 0x1178, 0x117C,
                0x1180, 0x1184, 0x1188, 0x118C, 0x2190, 0x2198, 0x21A0, 0x21A8, 0x21B0, 0x21B8,
                0x21C0, 0x11C8, 0x21CC, 0x21D4, 0x21DC, 0x11E4, 0x21E8, 0x21F0, 0x11F8, 0x21FC,
                0x2204, 0x120C, 0x2210, 0x1218, 0x121C, 0x2220, 0x1228, 0x222C, 0x1234, 0x1238,
                0x223C, 0x1244, 0x1248, 0x224C, 0x1254, 0x1258, 0x225C, 0x3264, 0x2270, 0x2278,
                0x2280, 0x2288, 0x2290, 0x2298, 0x22A0, 0x22A8, 0x22B0, 0x22B8, 0x22C0, 0x12C8,
                0x22CC, 0x22D4, 0x12DC, 0x22E0, 0x22E8, 0x12F0, 0x12F4, 0x22F8, 0x1300, 0x2304,
                0x130C, 0x1310, 0x2314, 0x131C, 0x1320, 0x2324, 0x132C, 0x1330, 0x5334, 0x4348,
                0x4358, 0x4368, 0x4378, 0x3388, 0x3394, 0x33A0, 0x33AC, 0x33B8, 0x33C4, 0x23D0,
                0x33D8, 0x23E4, 0x23EC, 0x2BF4, 0xFFFF
            },
            {
                0x2000, 0x2008, 0x2010, 0x2018, 0x2820, 0x282A, 0x2034, 0x203C, 0x2844, 0x284E,
                0x2058, 0x2060, 0x2068, 0x2070, 0x2078, 0x2080, 0x1888, 0x188E, 0x2094, 0x209C,
                0x18A4, 0x18AA, 0x18B0, 0x18B6, 0x20BC, 0x20C4, 0x18CC, 0x18D2, 0x18D8, 0x18DE,
                0x18E4, 0x18EA, 0x18F0, 0x18F6, 0x18FC, 0x1902, 0x1908, 0x190E, 0x1114, 0x1118,
                0x191C, 0x1922, 0x1928, 0x192E, 0x1134, 0x1138, 0x193C, 0x1942, 0x1148, 0x114C,
                0x1950, 0x1956, 0x115C, 0x1160, 0x1164, 0x1168, 0x196C, 0x1972, 0x1178, 0x117C,
                0x1180, 0x1184, 0x1188, 0x118C, 0x2190, 0x2198, 0x21A0, 0x21A8, 0x21B0, 0x21B8,
                0x21C0, 0x11C8, 0x21CC, 0x21D4, 0x21DC, 0x11E4, 0x21E8, 0x21F0, 0x11F8, 0x21FC,
                0x2204, 0x120C, 0x2210, 0x1218, 0x121C, 0x2220, 0x1228, 0x222C, 0x1234, 0x1238,
                0x223C, 0x1244, 0x1248, 0x224C, 0x1254, 0x1258, 0x225C, 0x3264, 0x2270, 0x2278,
                0x2280, 0x2288, 0x2290, 0x2298, 0x22A0, 0x22A8, 0x22B0, 0x22B8, 0x22C0, 0x12C8,
                0x22CC, 0x22D4, 0x12DC, 0x22E0, 0x22E8, 0x12F0, 0x12F4, 0x22F8, 0x1300, 0x2304,
                0x130C, 0x1310, 0x2314, 0x131C, 0x1320, 0x2324, 0x132C, 0x1330, 0x5334, 0x4348,
                0x4358, 0x4368, 0x4378, 0x3388, 0x3394, 0x33A0, 0x33AC, 0x33B8, 0x33C4, 0x23D0,
                0x33D8, 0x23E4, 0x23EC, 0x2BF4, 0xFFFF
            },
            {
                0x2000, 0x2008, 0x2010, 0x2018, 0x2820, 0x282A, 0x2034, 0x203C, 0x2844, 0x284E,
                0x2058, 0x2060, 0x2068, 0x2070, 0x2078, 0x2080, 0x1888, 0x188E, 0x2094, 0x209C,
                0x18A4, 0x18AA, 0x18B0, 0x18B6, 0x20BC, 0x20C4, 0x18CC, 0x18D2, 0x18D8, 0x18DE,
                0x18E4, 0x18EA, 0x18F0, 0x18F6, 0x18FC, 0x1902, 0x1908, 0x190E, 0x1114, 0x1118,
                0x191C, 0x1922, 0x1928, 0x192E, 0x1134, 0x1138, 0x193C, 0x1942, 0x1148, 0x114C,
                0x1950, 0x1956, 0x115C, 0x1160, 0x1164, 0x1168, 0x196C, 0x1972, 0x1178, 0x117C,
                0x1180, 0x1184, 0x1188, 0x118C, 0x2190, 0x2198, 0x21A0, 0x21A8, 0x21B0, 0x21B8,
                0x21C0, 0x11C8, 0x21CC, 0x21D4, 0x21DC, 0x11E4, 0x21E8, 0x21F0, 0x11F8, 0x21FC,
                0x2204, 0x120C, 0x2210, 0x1218, 0x121C, 0x2220, 0x1228, 0x222C, 0x1234, 0x1238,
                0x223C, 0x1244, 0x1248, 0x224C, 0x1254, 0x1258, 0x225C, 0x3264, 0x2270, 0x2278,
                0x2280, 0x2288, 0x2290, 0x2298, 0x22A0, 0x22A8, 0x22B0, 0x22B8, 0x22C0, 0x12C8,
                0x22CC, 0x22D4, 0x12DC, 0x22E0, 0x22E8, 0x12F0, 0x12F4, 0x22F8, 0x1300, 0x2304,
                0x130C, 0x1310, 0x2314, 0x131C, 0x1320, 0x2324, 0x132C, 0x1330, 0x5334, 0x4348,
                0x4358, 0x4368, 0x4378, 0x3388, 0x3394, 0x33A0, 0x33AC, 0x33B8, 0x33C4, 0x23D0,
                0x33D8, 0x23E4, 0x23EC, 0x2BF4, 0xFFFF
            },
            {
                0x2000, 0x2008, 0x2010, 0x2018, 0x2820, 0x282A, 0x2034, 0x203C, 0x2844, 0x284E,
                0x2058, 0x2060, 0x2068, 0x2070, 0x2078, 0x2080, 0x1888, 0x188E, 0x2094, 0x209C,
                0x18A4, 0x18AA, 0x18B0, 0x18B6, 0x20BC, 0x20C4, 0x18CC, 0x18D2, 0x18D8, 0x18DE,
                0x18E4, 0x18EA, 0x18F0, 0x18F6, 0x18FC, 0x1902, 0x1908, 0x190E, 0x1114, 0x1118,
                0x191C, 0x1922, 0x1928, 0x192E, 0x1134, 0x1138, 0x193C, 0x1942, 0x1148, 0x114C,
                0x1950, 0x1956, 0x115C, 0x1160, 0x1164, 0x1168, 0x196C, 0x1972, 0x1178, 0x117C,
                0x1180, 0x1184, 0x1188, 0x118C, 0x2190, 0x2198, 0x21A0, 0x21A8, 0x21B0, 0x21B8,
                0x21C0, 0x11C8, 0x21CC, 0x21D4, 0x21DC, 0x11E4, 0x21E8, 0x21F0, 0x11F8, 0x21FC,
                0x2204, 0x120C, 0x2210, 0x1218, 0x121C, 0x2220, 0x1228, 0x222C, 0x1234, 0x1238,
                0x223C, 0x1244, 0x1248, 0x224C, 0x1254, 0x1258, 0x225C, 0x3264, 0x2270, 0x2278,
                0x2280, 0x2288, 0x2290, 0x2298, 0x22A0, 0x22A8, 0x22B0, 0x22B8, 0x22C0, 0x12C8,
                0x22CC, 0x22D4, 0x12DC, 0x22E0, 0x22E8, 0x12F0, 0x12F4, 0x22F8, 0x1300, 0x2304,
                0x130C, 0x1310, 0x2314, 0x131C, 0x1320, 0x2324, 0x132C, 0x1330, 0x5334, 0x4348,
                0x4358, 0x4368, 0x4378, 0x3388, 0x3394, 0x33A0, 0x33AC, 0x33B8, 0x33C4, 0x23D0,
                0x33D8, 0x23E4, 0x23EC, 0x2BF4, 0xFFFF
            }
        }
    },
    {
        eDYNAMIC_GMA_MODE,  // eGMAMode
        8,                  // i4LowContrastThr
        {
            {   // i4ContrastWeightingTbl
                //  0   1   2    3    4    5    6    7    8    9    10
                    0,  0,  0,  33,  66, 100, 100,  100, 100, 100, 100
            },
            {   // i4LVWeightingTbl
                //LV0   1   2   3   4   5   6   7   8   9   10   11   12   13   14   15   16   17   18   19
                    0,  0,  0,  0,  0,  0,  0,  0,  0, 33,  66, 100, 100, 100, 100, 100, 100, 100, 100, 100
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
};

static MTK_GMA_ENV_INFO_STRUCT gsGMAEnvParam_main2 =
{
    {
        {
            {
                0x2000, 0x2008, 0x2010, 0x2018, 0x2820, 0x282A, 0x2034, 0x203C, 0x2844, 0x284E,
                0x2058, 0x2060, 0x2068, 0x2070, 0x2078, 0x2080, 0x1888, 0x188E, 0x2094, 0x209C,
                0x18A4, 0x18AA, 0x18B0, 0x18B6, 0x20BC, 0x20C4, 0x18CC, 0x18D2, 0x18D8, 0x18DE,
                0x18E4, 0x18EA, 0x18F0, 0x18F6, 0x18FC, 0x1902, 0x1908, 0x190E, 0x1114, 0x1118,
                0x191C, 0x1922, 0x1928, 0x192E, 0x1134, 0x1138, 0x193C, 0x1942, 0x1148, 0x114C,
                0x1950, 0x1956, 0x115C, 0x1160, 0x1164, 0x1168, 0x196C, 0x1972, 0x1178, 0x117C,
                0x1180, 0x1184, 0x1188, 0x118C, 0x2190, 0x2198, 0x21A0, 0x21A8, 0x21B0, 0x21B8,
                0x21C0, 0x11C8, 0x21CC, 0x21D4, 0x21DC, 0x11E4, 0x21E8, 0x21F0, 0x11F8, 0x21FC,
                0x2204, 0x120C, 0x2210, 0x1218, 0x121C, 0x2220, 0x1228, 0x222C, 0x1234, 0x1238,
                0x223C, 0x1244, 0x1248, 0x224C, 0x1254, 0x1258, 0x225C, 0x3264, 0x2270, 0x2278,
                0x2280, 0x2288, 0x2290, 0x2298, 0x22A0, 0x22A8, 0x22B0, 0x22B8, 0x22C0, 0x12C8,
                0x22CC, 0x22D4, 0x12DC, 0x22E0, 0x22E8, 0x12F0, 0x12F4, 0x22F8, 0x1300, 0x2304,
                0x130C, 0x1310, 0x2314, 0x131C, 0x1320, 0x2324, 0x132C, 0x1330, 0x5334, 0x4348,
                0x4358, 0x4368, 0x4378, 0x3388, 0x3394, 0x33A0, 0x33AC, 0x33B8, 0x33C4, 0x23D0,
                0x33D8, 0x23E4, 0x23EC, 0x2BF4, 0xFFFF
            },
            {
                0x2000, 0x2008, 0x2010, 0x2018, 0x2820, 0x282A, 0x2034, 0x203C, 0x2844, 0x284E,
                0x2058, 0x2060, 0x2068, 0x2070, 0x2078, 0x2080, 0x1888, 0x188E, 0x2094, 0x209C,
                0x18A4, 0x18AA, 0x18B0, 0x18B6, 0x20BC, 0x20C4, 0x18CC, 0x18D2, 0x18D8, 0x18DE,
                0x18E4, 0x18EA, 0x18F0, 0x18F6, 0x18FC, 0x1902, 0x1908, 0x190E, 0x1114, 0x1118,
                0x191C, 0x1922, 0x1928, 0x192E, 0x1134, 0x1138, 0x193C, 0x1942, 0x1148, 0x114C,
                0x1950, 0x1956, 0x115C, 0x1160, 0x1164, 0x1168, 0x196C, 0x1972, 0x1178, 0x117C,
                0x1180, 0x1184, 0x1188, 0x118C, 0x2190, 0x2198, 0x21A0, 0x21A8, 0x21B0, 0x21B8,
                0x21C0, 0x11C8, 0x21CC, 0x21D4, 0x21DC, 0x11E4, 0x21E8, 0x21F0, 0x11F8, 0x21FC,
                0x2204, 0x120C, 0x2210, 0x1218, 0x121C, 0x2220, 0x1228, 0x222C, 0x1234, 0x1238,
                0x223C, 0x1244, 0x1248, 0x224C, 0x1254, 0x1258, 0x225C, 0x3264, 0x2270, 0x2278,
                0x2280, 0x2288, 0x2290, 0x2298, 0x22A0, 0x22A8, 0x22B0, 0x22B8, 0x22C0, 0x12C8,
                0x22CC, 0x22D4, 0x12DC, 0x22E0, 0x22E8, 0x12F0, 0x12F4, 0x22F8, 0x1300, 0x2304,
                0x130C, 0x1310, 0x2314, 0x131C, 0x1320, 0x2324, 0x132C, 0x1330, 0x5334, 0x4348,
                0x4358, 0x4368, 0x4378, 0x3388, 0x3394, 0x33A0, 0x33AC, 0x33B8, 0x33C4, 0x23D0,
                0x33D8, 0x23E4, 0x23EC, 0x2BF4, 0xFFFF
            },
            {
                0x2000, 0x2008, 0x2010, 0x2018, 0x2820, 0x282A, 0x2034, 0x203C, 0x2844, 0x284E,
                0x2058, 0x2060, 0x2068, 0x2070, 0x2078, 0x2080, 0x1888, 0x188E, 0x2094, 0x209C,
                0x18A4, 0x18AA, 0x18B0, 0x18B6, 0x20BC, 0x20C4, 0x18CC, 0x18D2, 0x18D8, 0x18DE,
                0x18E4, 0x18EA, 0x18F0, 0x18F6, 0x18FC, 0x1902, 0x1908, 0x190E, 0x1114, 0x1118,
                0x191C, 0x1922, 0x1928, 0x192E, 0x1134, 0x1138, 0x193C, 0x1942, 0x1148, 0x114C,
                0x1950, 0x1956, 0x115C, 0x1160, 0x1164, 0x1168, 0x196C, 0x1972, 0x1178, 0x117C,
                0x1180, 0x1184, 0x1188, 0x118C, 0x2190, 0x2198, 0x21A0, 0x21A8, 0x21B0, 0x21B8,
                0x21C0, 0x11C8, 0x21CC, 0x21D4, 0x21DC, 0x11E4, 0x21E8, 0x21F0, 0x11F8, 0x21FC,
                0x2204, 0x120C, 0x2210, 0x1218, 0x121C, 0x2220, 0x1228, 0x222C, 0x1234, 0x1238,
                0x223C, 0x1244, 0x1248, 0x224C, 0x1254, 0x1258, 0x225C, 0x3264, 0x2270, 0x2278,
                0x2280, 0x2288, 0x2290, 0x2298, 0x22A0, 0x22A8, 0x22B0, 0x22B8, 0x22C0, 0x12C8,
                0x22CC, 0x22D4, 0x12DC, 0x22E0, 0x22E8, 0x12F0, 0x12F4, 0x22F8, 0x1300, 0x2304,
                0x130C, 0x1310, 0x2314, 0x131C, 0x1320, 0x2324, 0x132C, 0x1330, 0x5334, 0x4348,
                0x4358, 0x4368, 0x4378, 0x3388, 0x3394, 0x33A0, 0x33AC, 0x33B8, 0x33C4, 0x23D0,
                0x33D8, 0x23E4, 0x23EC, 0x2BF4, 0xFFFF
            },
            {
                0x2000, 0x2008, 0x2010, 0x2018, 0x2820, 0x282A, 0x2034, 0x203C, 0x2844, 0x284E,
                0x2058, 0x2060, 0x2068, 0x2070, 0x2078, 0x2080, 0x1888, 0x188E, 0x2094, 0x209C,
                0x18A4, 0x18AA, 0x18B0, 0x18B6, 0x20BC, 0x20C4, 0x18CC, 0x18D2, 0x18D8, 0x18DE,
                0x18E4, 0x18EA, 0x18F0, 0x18F6, 0x18FC, 0x1902, 0x1908, 0x190E, 0x1114, 0x1118,
                0x191C, 0x1922, 0x1928, 0x192E, 0x1134, 0x1138, 0x193C, 0x1942, 0x1148, 0x114C,
                0x1950, 0x1956, 0x115C, 0x1160, 0x1164, 0x1168, 0x196C, 0x1972, 0x1178, 0x117C,
                0x1180, 0x1184, 0x1188, 0x118C, 0x2190, 0x2198, 0x21A0, 0x21A8, 0x21B0, 0x21B8,
                0x21C0, 0x11C8, 0x21CC, 0x21D4, 0x21DC, 0x11E4, 0x21E8, 0x21F0, 0x11F8, 0x21FC,
                0x2204, 0x120C, 0x2210, 0x1218, 0x121C, 0x2220, 0x1228, 0x222C, 0x1234, 0x1238,
                0x223C, 0x1244, 0x1248, 0x224C, 0x1254, 0x1258, 0x225C, 0x3264, 0x2270, 0x2278,
                0x2280, 0x2288, 0x2290, 0x2298, 0x22A0, 0x22A8, 0x22B0, 0x22B8, 0x22C0, 0x12C8,
                0x22CC, 0x22D4, 0x12DC, 0x22E0, 0x22E8, 0x12F0, 0x12F4, 0x22F8, 0x1300, 0x2304,
                0x130C, 0x1310, 0x2314, 0x131C, 0x1320, 0x2324, 0x132C, 0x1330, 0x5334, 0x4348,
                0x4358, 0x4368, 0x4378, 0x3388, 0x3394, 0x33A0, 0x33AC, 0x33B8, 0x33C4, 0x23D0,
                0x33D8, 0x23E4, 0x23EC, 0x2BF4, 0xFFFF
            },
            {
                0x2000, 0x2008, 0x2010, 0x2018, 0x2820, 0x282A, 0x2034, 0x203C, 0x2844, 0x284E,
                0x2058, 0x2060, 0x2068, 0x2070, 0x2078, 0x2080, 0x1888, 0x188E, 0x2094, 0x209C,
                0x18A4, 0x18AA, 0x18B0, 0x18B6, 0x20BC, 0x20C4, 0x18CC, 0x18D2, 0x18D8, 0x18DE,
                0x18E4, 0x18EA, 0x18F0, 0x18F6, 0x18FC, 0x1902, 0x1908, 0x190E, 0x1114, 0x1118,
                0x191C, 0x1922, 0x1928, 0x192E, 0x1134, 0x1138, 0x193C, 0x1942, 0x1148, 0x114C,
                0x1950, 0x1956, 0x115C, 0x1160, 0x1164, 0x1168, 0x196C, 0x1972, 0x1178, 0x117C,
                0x1180, 0x1184, 0x1188, 0x118C, 0x2190, 0x2198, 0x21A0, 0x21A8, 0x21B0, 0x21B8,
                0x21C0, 0x11C8, 0x21CC, 0x21D4, 0x21DC, 0x11E4, 0x21E8, 0x21F0, 0x11F8, 0x21FC,
                0x2204, 0x120C, 0x2210, 0x1218, 0x121C, 0x2220, 0x1228, 0x222C, 0x1234, 0x1238,
                0x223C, 0x1244, 0x1248, 0x224C, 0x1254, 0x1258, 0x225C, 0x3264, 0x2270, 0x2278,
                0x2280, 0x2288, 0x2290, 0x2298, 0x22A0, 0x22A8, 0x22B0, 0x22B8, 0x22C0, 0x12C8,
                0x22CC, 0x22D4, 0x12DC, 0x22E0, 0x22E8, 0x12F0, 0x12F4, 0x22F8, 0x1300, 0x2304,
                0x130C, 0x1310, 0x2314, 0x131C, 0x1320, 0x2324, 0x132C, 0x1330, 0x5334, 0x4348,
                0x4358, 0x4368, 0x4378, 0x3388, 0x3394, 0x33A0, 0x33AC, 0x33B8, 0x33C4, 0x23D0,
                0x33D8, 0x23E4, 0x23EC, 0x2BF4, 0xFFFF
            }
        }
    },
    {
        eDYNAMIC_GMA_MODE,  // eGMAMode
        8,                  // i4LowContrastThr
        {
            {   // i4ContrastWeightingTbl
                //  0   1   2    3    4    5    6    7    8    9    10
                    0,  0,  0,  33,  66, 100, 100,  100, 100, 100, 100
            },
            {   // i4LVWeightingTbl
                //LV0   1   2   3   4   5   6   7   8   9   10   11   12   13   14   15   16   17   18   19
                    0,  0,  0,  0,  0,  0,  0,  0,  0, 33,  66, 100, 100, 100, 100, 100, 100, 100, 100, 100
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
};
/*******************************************************************************
*
*   rCamInfo
*       [in]    ISP Camera Info for RAW sensor. Its members are as below:
*
*           eCamMode:
*               ECamMode_Video          = 0,
*               ECamMode_Online_Preview,
*               ECamMode_Online_Capture,
*               ECamMode_Online_Capture_ZSD,
*               ECamMode_Offline_Capture_Pass1,
*               ECamMode_Offline_Capture_Pass2,
*               ECamMode_HDR_Cap_Pass1_SF,  //  Pass1: Single Frame
*               ECamMode_HDR_Cap_Pass1_MF1, //  Pass1: Multi Frame Stage1
*               ECamMode_HDR_Cap_Pass1_MF2, //  Pass1: Multi Frame Stage2
*               ECamMode_HDR_Cap_Pass2,     //  Pass2
*
*           eIdx_Scene:
*               SCENE_MODE_OFF,             // Disable scene mode equal Auto mode
*               SCENE_MODE_NORMAL,          // Normal mode
*               SCENE_MODE_ACTION,          // Action mode
*               SCENE_MODE_PORTRAIT,        // Portrait mode
*               SCENE_MODE_LANDSCAPE,       // Landscape
*               SCENE_MODE_NIGHTSCENE,      // Night Scene
*               SCENE_MODE_NIGHTPORTRAIT,   // Night Portrait
*               SCENE_MODE_THEATRE,         // Theatre mode
*               SCENE_MODE_BEACH,           // Beach mode
*               SCENE_MODE_SNOW,            // Snow mode
*               SCENE_MODE_SUNSET,          // Sunset mode
*               SCENE_MODE_STEADYPHOTO,     // Steady photo mode
*               SCENE_MODE_FIREWORKS,       // Fireworks mode
*               SCENE_MODE_SPORTS,          // Sports mode
*               SCENE_MODE_PARTY,           // Party mode
*               SCENE_MODE_CANDLELIGHT,     // Candle light mode
*               SCENE_MODE_HDR,             // HDR mode
*
*           u4ISOValue:
*               ISO value to determine eISO.
*
*           eIdx_ISO:
*               eIDX_ISO_100,
*               eIDX_ISO_200,
*               eIDX_ISO_400,
*               eIDX_ISO_800,
*               eIDX_ISO_1600
*
*           i4CCT:
*               Correlated color temperature
*
*           eCCTIndex_CCM:
*               Correlated color temperature index for CCM
*                   eIDX_CCM_TL84
*                   eIDX_CCM_CWF
*                   eIDX_CCM_D65
*
*           u4ZoomRatio_x100:
*               zoom ratio (x100)
*
*           i4LightValue_x10:
*               light value (x10)
*
*   rIdxMgr:
*       [in]    The default ISP tuning index manager.
*       [out]   The ISP tuning index manager after customizing.
*
*
*******************************************************************************/

MVOID
IspTuningCustom::
refine_CamInfo(RAWIspCamInfo& /*rCamInfo*/)
{
}

MVOID
IspTuningCustom::
evaluate_nvram_index(RAWIspCamInfo const& rCamInfo, IndexMgr& /*rIdxMgr*/)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.isptun_cust.ctrl", value, "-1");
    MINT32 ctrl = atoi(value);
    MBOOL logEn = (ctrl == -1) ? MFALSE : ((ctrl & ISP_TUN_LOG_BIT) ? MTRUE : MFALSE);

//..............................................................................
    //  (1) Dump info. before customizing.
//#if ENABLE_MY_LOG
#if 0
    if (logEn) rCamInfo.dump();
#endif

#if 0
    LOGD("[+evaluate_nvram_index][before customizing]");
    rIdxMgr.dump();
#endif
//..............................................................................
    //  (2) Modify each index based on conditions.
    //
    //  setIdx_XXX() returns:
    //      MTURE: if successful
    //      MFALSE: if the input index is out of range.
    //
#if 0
    fgRet = rIdxMgr.setIdx_OBC(XXX);
    fgRet = rIdxMgr.setIdx_BPC(XXX);
    fgRet = rIdxMgr.setIdx_NR1(XXX);
    fgRet = rIdxMgr.setIdx_CFA(XXX);
    fgRet = rIdxMgr.setIdx_GGM(XXX);
    fgRet = rIdxMgr.setIdx_ANR(XXX);
    fgRet = rIdxMgr.setIdx_CCR(XXX);
    fgRet = rIdxMgr.setIdx_EE(XXX);
#endif
//..............................................................................
    //  (3) Finally, dump info. after modifying.
#if 0
    LOGD("[-evaluate_nvram_index][after customizing]");
    rIdxMgr.dump();
#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_OBC(RAWIspCamInfo const& /*rCamInfo*/, IspNvramRegMgr & /*rIspRegMgr*/, ISP_NVRAM_OBC_T& /*rOBC*/)
{
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rOBC.offst0 = 0x%8x", rOBC.offst0);
    MY_LOG("rOBC.offst1 = 0x%8x", rOBC.offst1);
    MY_LOG("rOBC.offst2 = 0x%8x", rOBC.offst2);
    MY_LOG("rOBC.offst3 = 0x%8x", rOBC.offst3);
    MY_LOG("rOBC.gain0 = 0x%8x", rOBC.gain0);
    MY_LOG("rOBC.gain1 = 0x%8x", rOBC.gain1);
    MY_LOG("rOBC.gain2 = 0x%8x", rOBC.gain2);
    MY_LOG("rOBC.gain3 = 0x%8x", rOBC.gain3);
    #endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_BPC(RAWIspCamInfo const& /*rCamInfo*/, IspNvramRegMgr & /*rIspRegMgr*/, ISP_NVRAM_BPC_T& /*rBPC*/)
{
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rBPC.con = 0x%8x", rBPC.con);
    MY_LOG("rBPC.cd1_1 = 0x%8x", rBPC.cd1_1);
    MY_LOG("rBPC.cd1_2 = 0x%8x", rBPC.cd1_2);
    MY_LOG("rBPC.cd1_3 = 0x%8x", rBPC.cd1_3);
    MY_LOG("rBPC.cd1_4 = 0x%8x", rBPC.cd1_4);
    MY_LOG("rBPC.cd1_5 = 0x%8x", rBPC.cd1_5);
    MY_LOG("rBPC.cd1_6 = 0x%8x", rBPC.cd1_6);
    MY_LOG("rBPC.cd2_1 = 0x%8x", rBPC.cd2_1);
    MY_LOG("rBPC.cd2_2 = 0x%8x", rBPC.cd2_2);
    MY_LOG("rBPC.cd2_3 = 0x%8x", rBPC.cd2_3);
    MY_LOG("rBPC.cd0 = 0x%8x", rBPC.cd0);
    MY_LOG("rBPC.cor = 0x%8x", rBPC.cor);
    #endif

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_NR1(RAWIspCamInfo const& /*rCamInfo*/, IspNvramRegMgr & /*rIspRegMgr*/, ISP_NVRAM_NR1_T& /*rNR1*/)
{
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rNR1.con = 0x%8x", rNR1.con);
    MY_LOG("rNR1.ct_con = 0x%8x", rNR1.ct_con);
    #endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_SL2(RAWIspCamInfo const& /*rCamInfo*/, IspNvramRegMgr & /*rIspRegMgr*/, ISP_NVRAM_SL2_T& /*rSL2*/)
{
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rSL2.cen = 0x%8x", rSL2.cen);
    MY_LOG("rSL2.max0_rr = 0x%8x", rSL2.max0_rr);
    MY_LOG("rSL2.max1_rr = 0x%8x", rSL2.max1_rr);
    MY_LOG("rSL2.max2_rr = 0x%8x", rSL2.max2_rr);
    #endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_PGN(RAWIspCamInfo const& /*rCamInfo*/, IspNvramRegMgr & /*rIspRegMgr*/, ISP_NVRAM_PGN_T& /*rPGN*/)
{
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rPGN.satu01 = 0x%8x", rPGN.satu01);
    MY_LOG("rPGN.satu23 = 0x%8x", rPGN.satu23);
    MY_LOG("rPGN.gain01 = 0x%8x", rPGN.gain01);
    MY_LOG("rPGN.gain23 = 0x%8x", rPGN.gain23);
    MY_LOG("rPGN.offs01 = 0x%8x", rPGN.offs01);
    MY_LOG("rPGN.offs23 = 0x%8x", rPGN.offs23);
    #endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_CFA(RAWIspCamInfo const& /*rCamInfo*/, IspNvramRegMgr & /*rIspRegMgr*/, ISP_NVRAM_CFA_T& /*rCFA*/)
{
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rCFA.bypass = 0x%8x", rCFA.bypass);
    MY_LOG("rCFA.ed_f = 0x%8x", rCFA.ed_f);
    MY_LOG("rCFA.ed_nyq = 0x%8x", rCFA.ed_nyq);
    MY_LOG("rCFA.ed_step = 0x%8x", rCFA.ed_step);
    MY_LOG("rCFA.rgb_hf = 0x%8x", rCFA.rgb_hf);
    MY_LOG("rCFA.bw = 0x%8x", rCFA.bw);
    MY_LOG("rCFA.f1_act = 0x%8x", rCFA.f1_act);
    MY_LOG("rCFA.f2_act = 0x%8x", rCFA.f2_act);
    MY_LOG("rCFA.f3_act = 0x%8x", rCFA.f3_act);
    MY_LOG("rCFA.f4_act = 0x%8x", rCFA.f4_act);
    MY_LOG("rCFA.f1_l = 0x%8x", rCFA.f1_l);
    MY_LOG("rCFA.f2_l = 0x%8x", rCFA.f2_l);
    MY_LOG("rCFA.f3_l = 0x%8x", rCFA.f3_l);
    MY_LOG("rCFA.f4_l = 0x%8x", rCFA.f4_l);
    MY_LOG("rCFA.hf_rb = 0x%8x", rCFA.hf_rb);
    MY_LOG("rCFA.hf_gain = 0x%8x", rCFA.hf_gain);
    MY_LOG("rCFA.hf_comp = 0x%8x", rCFA.hf_comp);
    MY_LOG("rCFA.hf_coring_th = 0x%8x", rCFA.hf_coring_th);
    MY_LOG("rCFA.act_lut = 0x%8x", rCFA.act_lut);
    MY_LOG("rCFA.spare = 0x%8x", rCFA.spare);
    MY_LOG("rCFA.bb = 0x%8x", rCFA.bb);
    #endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define CONV_2COMP_11(x) ( (x)<1024?(x):((int)(x)-2048) )
#define ICONV_2COMP_11(x) ( (x)<0?((x)+2048):(x) )

static MINT32 Complement2(MUINT32 value, MUINT32 digit)
{
    MINT32 Result;

    if (((value >> (digit - 1)) & 0x1) == 1)    // negative
    {
        Result = 0 - (MINT32)((~value + 1) & ((1 << digit) - 1));
    }
    else
    {
        Result = (MINT32)(value & ((1 << digit) - 1));
    }

    return Result;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_CCM(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & /*rIspRegMgr*/, ISP_NVRAM_CCM_T& rCCM)
{
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rCCM.conv0a = 0x%8x", rCCM.conv0a);
    MY_LOG("rCCM.conv0b = 0x%8x", rCCM.conv0b);
    MY_LOG("rCCM.conv1a = 0x%8x", rCCM.conv1a);
    MY_LOG("rCCM.conv1b = 0x%8x", rCCM.conv1b);
    MY_LOG("rCCM.conv2a = 0x%8x", rCCM.conv2a);
    MY_LOG("rCCM.conv2b = 0x%8x", rCCM.conv2b);
#endif

        // d65
        //0x071701FB, 0x000007EE, 0x016B07BF, 0x000007D6, 0x0755001A, 0x00000191
/*
        rCCM.conv0a.val = 0x071701FB;
    rCCM.conv0b.val = 0x000007EE;
    rCCM.conv1a.val = 0x016B07BF;
    rCCM.conv1b.val = 0x000007D6;
    rCCM.conv2a.val = 0x0755001A;
    rCCM.conv2b.val = 0x00000191;
*/

    MINT32 i4SensorID = getSensorID();

    //MINT32 CCM_22 = CONV_2COMP_11(rCCM.conv2b.bits.G2G_CNV_22);
  //if (getSensorDev() == ESensorDev_Main) //Main
  if(i4SensorID == 0x5648)
    {
            MY_LOG("sensor ID = 0x%8x", i4SensorID);
            if(rCamInfo.rAWBInfo.rCurrentAWBGain.i4R > 765)
            {
                        //0x0716022C, 0x000007BE, 0x017E07AB, 0x000007D7, 0x0767000A, 0x0000018F
                        rCCM.set[0] = 0x0716022C;
                      rCCM.set[1] = 0x000007BE;
                        rCCM.set[2] = 0x017E07AB;
                        rCCM.set[3] = 0x000007D7;
                        rCCM.set[4] = 0x0767000A;
                        rCCM.set[5] = 0x0000018F;
            }
            else if(rCamInfo.rAWBInfo.rCurrentAWBGain.i4R < 493)
            {
                    //
                    //0x07910190, 0x000007DF, 0x01320790, 0x000003E, 0x067507E1, 0x000002AA
                        rCCM.set[0] = 0x07910190;
                      rCCM.set[1] = 0x000007DF;
                        rCCM.set[2] = 0x01320790;
                        rCCM.set[3] = 0x0000003E;
                        rCCM.set[4] = 0x067507E1;
                        rCCM.set[5] = 0x000002AA;
            }

    }
    else if(i4SensorID == 0x2680)
    {
            MY_LOG("sensor ID = 0x%8x", i4SensorID);
            if(rCamInfo.rAWBInfo.rCurrentAWBGain.i4R > 780)
            {
                        // sat 108
                        // 0x06E70208, 0x0000011, 0x019C07A5, 0x000007BF, 0x06C207D4, 0x0000026A
                        rCCM.set[0] = 0x06E70208;
                      rCCM.set[1] = 0x00000011;
                        rCCM.set[2] = 0x019C07A5;
                        rCCM.set[3] = 0x000007BF;
                        rCCM.set[4] = 0x06C207D4;
                        rCCM.set[5] = 0x0000026A;
            }
            else if(rCamInfo.rAWBInfo.rCurrentAWBGain.i4R < 473)
            {
                        // sat 108
                        // 0x0BE015E, 0x000006E4, 0x02350767, 0x00000764, 0x06F2070E, 0x00000300
                        rCCM.set[0] = 0x0BE015E;
                      rCCM.set[1] = 0x000006E4;
                        rCCM.set[2] = 0x02350767;
                        rCCM.set[3] = 0x00000764;
                        rCCM.set[4] = 0x06F2070E;
                        rCCM.set[5] = 0x00000300;

            }
    }

    if(i4SensorID == 0x2680)
    {
    // jason.jan, update CCM according to ISO
    MINT32 CCM_00 = CONV_2COMP_11(rCCM.conv0a.bits.G2G_CNV_00);
    MINT32 CCM_01 = CONV_2COMP_11(rCCM.conv0a.bits.G2G_CNV_01);
    MINT32 CCM_02 = CONV_2COMP_11(rCCM.conv0b.bits.G2G_CNV_02);
    MINT32 CCM_10 = CONV_2COMP_11(rCCM.conv1a.bits.G2G_CNV_10);
    MINT32 CCM_11 = CONV_2COMP_11(rCCM.conv1a.bits.G2G_CNV_11);
    MINT32 CCM_12 = CONV_2COMP_11(rCCM.conv1b.bits.G2G_CNV_12);
    MINT32 CCM_20 = CONV_2COMP_11(rCCM.conv2a.bits.G2G_CNV_20);
    MINT32 CCM_21 = CONV_2COMP_11(rCCM.conv2a.bits.G2G_CNV_21);
    MINT32 CCM_22 = CONV_2COMP_11(rCCM.conv2b.bits.G2G_CNV_22);

    // tuning parameter
    const MINT32 iso_l = 100;
    const MINT32 iso_h = 300;
    const int gain_l = (int)(0.9*128); //(0.7*128);
    const int gain_h = (int)(0.6*128); //(0.7*128);
    /*

    --------------+---------------------------------------------------+-------------------------------
    ISO < iso_l          iso_l < ISO < iso_h                                 ISO > iso_h
    gain = gain_l        gain = linear interpolation(gain_l, gain_h)         gain = gain_h

    */
    //! tuning parameter

    MINT32 current_iso = rCamInfo.u4ISOValue;
    int current_gain = 128;


    if(current_iso<iso_l)
        current_gain = gain_l;
    else if(current_iso>iso_h)
        current_gain = gain_h;
    else
        current_gain = gain_l + (gain_h-gain_l)*(current_iso-iso_l)/(iso_h-iso_l);


    CCM_01 = (CCM_01)*current_gain/128;
    CCM_02 = (CCM_02)*current_gain/128;
    CCM_00 = 256 - (CCM_01+CCM_02);

    CCM_10 = (CCM_10)*current_gain/128;
    CCM_12 = (CCM_12)*current_gain/128;
    CCM_11 = 256 - (CCM_10+CCM_12);

    CCM_20 = (CCM_20)*current_gain/128;
    CCM_21 = (CCM_21)*current_gain/128;
    CCM_22 = 256 - (CCM_20+CCM_21);


    rCCM.conv0a.bits.G2G_CNV_00 = ICONV_2COMP_11(CCM_00);
    rCCM.conv0a.bits.G2G_CNV_01 = ICONV_2COMP_11(CCM_01);
    rCCM.conv0b.bits.G2G_CNV_02 = ICONV_2COMP_11(CCM_02);
    rCCM.conv1a.bits.G2G_CNV_10 = ICONV_2COMP_11(CCM_10);
    rCCM.conv1a.bits.G2G_CNV_11 = ICONV_2COMP_11(CCM_11);
    rCCM.conv1b.bits.G2G_CNV_12 = ICONV_2COMP_11(CCM_12);
    rCCM.conv2a.bits.G2G_CNV_20 = ICONV_2COMP_11(CCM_20);
    rCCM.conv2a.bits.G2G_CNV_21 = ICONV_2COMP_11(CCM_21);
    rCCM.conv2b.bits.G2G_CNV_22 = ICONV_2COMP_11(CCM_22);


    MY_LOG("After");
    MY_LOG("Current ISO = %d", current_iso);
    MY_LOG("Current ISO = %d", current_gain);
    MY_LOG("rCCM.conv0a.bits.G2G_CNV_00 = %d", rCCM.conv0a.bits.G2G_CNV_00);
    MY_LOG("rCCM.conv0a.bits.G2G_CNV_01 = %d", rCCM.conv0a.bits.G2G_CNV_01);
    MY_LOG("rCCM.conv0b.bits.G2G_CNV_02 = %d", rCCM.conv0b.bits.G2G_CNV_02);

    MY_LOG("rCCM.conv1a.bits.G2G_CNV_10 = %d", rCCM.conv1a.bits.G2G_CNV_10);
    MY_LOG("rCCM.conv1a.bits.G2G_CNV_11 = %d", rCCM.conv1a.bits.G2G_CNV_11);
    MY_LOG("rCCM.conv1b.bits.G2G_CNV_12 = %d", rCCM.conv1b.bits.G2G_CNV_12);

    MY_LOG("rCCM.conv2a.bits.G2G_CNV_20 = %d", rCCM.conv2a.bits.G2G_CNV_20);
    MY_LOG("rCCM.conv2a.bits.G2G_CNV_21 = %d", rCCM.conv2a.bits.G2G_CNV_21);
    MY_LOG("rCCM.conv2b.bits.G2G_CNV_22 = %d", rCCM.conv2b.bits.G2G_CNV_22);
    //! jason.jan, update CCM according to ISO
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_GGM(RAWIspCamInfo const& /*rCamInfo*/, IspNvramRegMgr & /*rIspRegMgr*/, ISP_NVRAM_GGM_T& /*rGGM*/)
{
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rGGM.lut_rb.lut[0] = 0x%8x", rGGM.lut_rb.lut[0]);
    MY_LOG("rGGM.lut_g.lut[0] = 0x%8x", rGGM.lut_g.lut[0]);
    #endif
}

MVOID*
IspTuningCustom::
get_custom_GMA_env_info(ESensorDev_T eSensorDev)
{
/*
enum
{
    SENSOR_DEV_NONE = 0x00,
    SENSOR_DEV_MAIN = 0x01,
    SENSOR_DEV_SUB  = 0x02,
    SENSOR_DEV_PIP = 0x03,
    SENSOR_DEV_MAIN_2 = 0x04,
    SENSOR_DEV_MAIN_3D = 0x05,
};

*/

    switch (eSensorDev)
    {
    case ESensorDev_Main: //main
        return &gsGMAEnvParam_main;
        break;
    case ESensorDev_Sub: //sub
        return &gsGMAEnvParam_sub;
        break;
    case ESensorDev_MainSecond: //main2
        return &gsGMAEnvParam_main2;
        break;
    default:
        return &gsGMAEnvParam_main;
    }

}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_ANR(RAWIspCamInfo const& /*rCamInfo*/, IspNvramRegMgr & /*rIspRegMgr*/, ISP_NVRAM_ANR_T& /*rANR*/)
{
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rANR.con1 = 0x%8x", rANR.con1);
    MY_LOG("rANR.con2 = 0x%8x", rANR.con2);
    MY_LOG("rANR.con3 = 0x%8x", rANR.con3);
    MY_LOG("rANR.yad1 = 0x%8x", rANR.yad1);
    MY_LOG("rANR.yad2 = 0x%8x", rANR.yad2);
    MY_LOG("rANR.lut1 = 0x%8x", rANR.lut1);
    MY_LOG("rANR.lut2 = 0x%8x", rANR.lut2);
    MY_LOG("rANR.lut3 = 0x%8x", rANR.lut3);
    MY_LOG("rANR.pty = 0x%8x", rANR.pty);
    MY_LOG("rANR.cad = 0x%8x", rANR.cad);
    MY_LOG("rANR.ptc = 0x%8x", rANR.ptc);
    MY_LOG("rANR.lce1 = 0x%8x", rANR.lce1);
    MY_LOG("rANR.lce2 = 0x%8x", rANR.lce2);
    MY_LOG("rANR.hp1 = 0x%8x", rANR.hp1);
    MY_LOG("rANR.hp2 = 0x%8x", rANR.hp2);
    MY_LOG("rANR.hp3 = 0x%8x", rANR.hp3);
    MY_LOG("rANR.acty = 0x%8x", rANR.acty);
    MY_LOG("rANR.actc = 0x%8x", rANR.actc);
    #endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_CCR(RAWIspCamInfo const& /*rCamInfo*/, IspNvramRegMgr & /*rIspRegMgr*/, ISP_NVRAM_CCR_T& /*rCCR*/)
{
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rCCR.con = 0x%8x", rCCR.con);
    MY_LOG("rCCR.ylut = 0x%8x", rCCR.ylut);
    MY_LOG("rCCR.uvlut = 0x%8x", rCCR.uvlut);
    MY_LOG("rCCR.ylut2 = 0x%8x", rCCR.ylut2);
    #endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_EE(RAWIspCamInfo const& /*rCamInfo*/, IspNvramRegMgr & /*rIspRegMgr*/, ISP_NVRAM_EE_T& /*rEE*/)
{
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rEE.srk_ctrl = 0x%8x", rEE.srk_ctrl);
    MY_LOG("rEE.clip_ctrl = 0x%8x", rEE.clip_ctrl);
    MY_LOG("rEE.hp_ctrl1 = 0x%8x", rEE.hp_ctrl1);
    MY_LOG("rEE.hp_ctrl2 = 0x%8x", rEE.hp_ctrl2);
    MY_LOG("rEE.ed_ctrl1 = 0x%8x", rEE.ed_ctrl1);
    MY_LOG("rEE.ed_ctrl2 = 0x%8x", rEE.ed_ctrl2);
    MY_LOG("rEE.ed_ctrl3 = 0x%8x", rEE.ed_ctrl3);
    MY_LOG("rEE.ed_ctrl4 = 0x%8x", rEE.ed_ctrl4);
    MY_LOG("rEE.ed_ctrl5 = 0x%8x", rEE.ed_ctrl5);
    MY_LOG("rEE.ed_ctrl6 = 0x%8x", rEE.ed_ctrl6);
    MY_LOG("rEE.ed_ctrl7 = 0x%8x", rEE.ed_ctrl7);
    MY_LOG("rEE.ee_link1 = 0x%8x", rEE.ee_link1);
    MY_LOG("rEE.ee_link2 = 0x%8x", rEE.ee_link2);
    MY_LOG("rEE.ee_link3 = 0x%8x", rEE.ee_link3);
    MY_LOG("rEE.ee_link4 = 0x%8x", rEE.ee_link4);
    MY_LOG("rEE.ee_link5 = 0x%8x", rEE.ee_link5);
#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
EIndex_CCM_T
IspTuningCustom::
evaluate_CCM_index(RAWIspCamInfo const& rCamInfo)
{
    MY_LOG("%s()\n", __FUNCTION__);

    MY_LOG(
        "[+evaluate_CCM_index]"
        "(eIdx_CCM, i4CCT, i4FluorescentIndex)=(%d, %d, %d)"
        , rCamInfo.eIdx_CCM
        , rCamInfo.rAWBInfo.i4CCT
        , rCamInfo.rAWBInfo.i4FluorescentIndex);

    EIndex_CCM_T eIdx_CCM_new = rCamInfo.eIdx_CCM;

//    -----------------|---|---|--------------|---|---|------------------
//                                THA TH1 THB              THC TH2  THD

    MINT32 const THA = 3318;
    MINT32 const TH1 = 3484;
    MINT32 const THB = 3667;
    MINT32 const THC = 4810;
    MINT32 const TH2 = 5050;
    MINT32 const THD = 5316;
    MINT32 const F_IDX_TH1 = 25;
    MINT32 const F_IDX_TH2 = -25;

    switch  (rCamInfo.eIdx_CCM)
    {
    case eIDX_CCM_TL84:
        if  ( rCamInfo.rAWBInfo.i4CCT < THB )
    {
            eIdx_CCM_new = eIDX_CCM_TL84;
        }
        else if ( rCamInfo.rAWBInfo.i4CCT < THD )
        {
            if  ( rCamInfo.rAWBInfo.i4FluorescentIndex < F_IDX_TH2 )
                eIdx_CCM_new = eIDX_CCM_CWF;
            else
                eIdx_CCM_new = eIDX_CCM_TL84;
        }
        else
        {
            eIdx_CCM_new = eIDX_CCM_D65;
        }
        break;
    case eIDX_CCM_CWF:
        if  ( rCamInfo.rAWBInfo.i4CCT < THA )
        {
            eIdx_CCM_new = eIDX_CCM_TL84;
    }
        else if ( rCamInfo.rAWBInfo.i4CCT < THD )
    {
            if  ( rCamInfo.rAWBInfo.i4FluorescentIndex > F_IDX_TH1 )
                eIdx_CCM_new = eIDX_CCM_TL84;
            else
                eIdx_CCM_new = eIDX_CCM_CWF;
        }
        else
        {
            eIdx_CCM_new = eIDX_CCM_D65;
        }
        break;
    case eIDX_CCM_D65:
        if  ( rCamInfo.rAWBInfo.i4CCT > THC )
        {
            eIdx_CCM_new = eIDX_CCM_D65;
    }
        else if ( rCamInfo.rAWBInfo.i4CCT > TH1 )
    {
            if(rCamInfo.rAWBInfo.i4FluorescentIndex > F_IDX_TH2)
                eIdx_CCM_new = eIDX_CCM_TL84;
            else
                eIdx_CCM_new = eIDX_CCM_CWF;
        }
        else
        {
            eIdx_CCM_new = eIDX_CCM_TL84;
        }
        break;
    default:
        break;

    }

    if  ( rCamInfo.eIdx_CCM != eIdx_CCM_new )
    {
        MY_LOG(
            "[-evaluate_CCM_index] CCM Idx(old,new)=(%d,%d)"
            , rCamInfo.eIdx_CCM, eIdx_CCM_new
        );
    }

    return  eIdx_CCM_new;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IspTuningCustom::
is_to_invoke_smooth_ccm_with_preference_gain(RAWIspCamInfo const& /*rCamInfo*/)
{
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IspTuningCustom::
is_to_invoke_isp_interpolation(RAWIspCamInfo const& /*rCamInfo*/)
{
/*
    if(TUNING_FOR_AIS) {
        if(IS_AIS) {
            if(rCamInfo.eIspProfile == EIspProfile_MFB_Capture_EE_Off
                || rCamInfo.eIspProfile == EIspProfile_MFB_Capture_EE_Off_SWNR
                || rCamInfo.eIspProfile == EIspProfile_MFB_PostProc_EE_Off
                || rCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off
                || rCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR
                || rCamInfo.eIspProfile == EIspProfile_MFB_PostProc_ANR_EE
                || rCamInfo.eIspProfile == EIspProfile_MFB_PostProc_ANR_EE_SWNR
                || rCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing
                || rCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR
            )
			{
	            return MFALSE;
			}
       }
    }
*/
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
IspTuningCustom::
get_CCM_smooth_method(RAWIspCamInfo const& /*rCamInfo*/)
{
    // 0: CCM (without flash info)
    // 1: enable flash CCM
    return 0;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
EIndex_PCA_LUT_T
IspTuningCustom::
evaluate_PCA_LUT_index(RAWIspCamInfo const& rCamInfo)
{
    //MY_LOG("%s()\n", __FUNCTION__);

    // TODO: Add your code below...

/*
    MY_LOG(
        "[+evaluate_PCA_LUT_index]"
        "(rCamInfo.eIdx_PCA_LUT, rCamInfo.rAWBInfo.i4CCT, rCamInfo.rAWBInfo.i4FluorescentIndex)=(%d, %d, %d)"
        , rCamInfo.eIdx_PCA_LUT, rCamInfo.rAWBInfo.i4CCT, rCamInfo.rAWBInfo.i4FluorescentIndex
    );
*/
    EIndex_PCA_LUT_T eIdx_PCA_LUT_new = rCamInfo.eIdx_PCA_LUT;

//    -----------------|-------|--------------|-------|------------------
//                    THA     THB            THC     THD

    MINT32 const THA = 3318;
    MINT32 const THB = 3667;
    MINT32 const THC = 4810;
    MINT32 const THD = 5316;

    switch  (rCamInfo.eIdx_PCA_LUT)
    {
    case eIDX_PCA_HIGH:
        if  ( rCamInfo.rAWBInfo.i4CCT < THA )
        {
            eIdx_PCA_LUT_new = eIDX_PCA_LOW;
        }
        else if ( rCamInfo.rAWBInfo.i4CCT < THC )
        {
            eIdx_PCA_LUT_new = eIDX_PCA_MIDDLE;
        }
        else
        {
            eIdx_PCA_LUT_new = eIDX_PCA_HIGH;
        }
        break;
    case eIDX_PCA_MIDDLE:
        if  ( rCamInfo.rAWBInfo.i4CCT > THD )
        {
            eIdx_PCA_LUT_new = eIDX_PCA_HIGH;
        }
        else if ( rCamInfo.rAWBInfo.i4CCT < THA )
        {
            eIdx_PCA_LUT_new = eIDX_PCA_LOW;
        }
        else
        {
            eIdx_PCA_LUT_new = eIDX_PCA_MIDDLE;
        }
        break;
    case eIDX_PCA_LOW:
        if  ( rCamInfo.rAWBInfo.i4CCT > THD )
        {
	        eIdx_PCA_LUT_new = eIDX_PCA_HIGH;
        }
        else if ( rCamInfo.rAWBInfo.i4CCT > THB )
        {
            eIdx_PCA_LUT_new = eIDX_PCA_MIDDLE;
        }
        else
        {
            eIdx_PCA_LUT_new = eIDX_PCA_LOW;
        }
        break;
    default:
        break;

    }

    if  ( rCamInfo.eIdx_PCA_LUT != eIdx_PCA_LUT_new )
    {
        MY_LOG(
            "[-evaluate_PCA_LUT_index] PCA_LUT_index(old,new)=(%d,%d)"
            , rCamInfo.eIdx_PCA_LUT, eIdx_PCA_LUT_new
        );
    }

    return eIdx_PCA_LUT_new;
}

/*******************************************************************************
*
* eIdx_Shading_CCT_old:
*   [in] the previous color temperature index
*           eIDX_Shading_CCT_ALight
*           eIDX_Shading_CCT_CWF
*           eIDX_Shading_CCT_D65
*
* i4CCT:
*   [in] the current color temperature from 3A.
*
*
* return:
*   [out] the current color temperature index
*           eIDX_Shading_CCT_ALight
*           eIDX_Shading_CCT_CWF
*           eIDX_Shading_CCT_D65
*
*******************************************************************************/
EIndex_Shading_CCT_T
IspTuningCustom::
evaluate_Shading_CCT_index  (
        RAWIspCamInfo const& rCamInfo
)   const
{
    UINT32 i4CCT = rCamInfo.rAWBInfo.i4CCT;

    EIndex_Shading_CCT_T eIdx_Shading_CCT_new = rCamInfo.eIdx_Shading_CCT;

//    -----------------|----|----|--------------|----|----|------------------
//                   THH2  TH2  THL2                   THH1  TH1  THL1

    MINT32 const THL1 = 2500;//3257;
    MINT32 const THH1 = 2800;//3484;
    MINT32 const TH1 = (THL1+THH1)/2; //(THL1 +THH1)/2
    MINT32 const THL2 = 4673;
    MINT32 const THH2 = 5155;
    MINT32 const TH2 = (THL2+THH2)/2;//(THL2 +THH2)/2

    switch  (rCamInfo.eIdx_Shading_CCT)
    {
    case eIDX_Shading_CCT_ALight:
        if  ( i4CCT < THH1 )
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_ALight;
        }
        else if ( i4CCT <  TH2)
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_CWF;
        }
        else
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_D65;
        }
        break;
    case eIDX_Shading_CCT_CWF:
        if  ( i4CCT < THL1 )
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_ALight;
        }
        else if ( i4CCT < THH2 )
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_CWF;
        }
        else
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_D65;
        }
        break;
    case eIDX_Shading_CCT_D65:
        if  ( i4CCT < TH1 )
        {
         eIdx_Shading_CCT_new = eIDX_Shading_CCT_ALight;
        }
        else if ( i4CCT < THL2 )
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_CWF;
        }
        else
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_D65;
        }
        break;
    default:
        break;
    }
//#if ENABLE_MY_LOG
    if  ( rCamInfo.eIdx_Shading_CCT != eIdx_Shading_CCT_new )
    {
        MY_LOG(
            "[-evaluate_Shading_CCT_index] Shading CCT Idx(old,new)=(%d,%d), i4CCT = %d\n"
            , rCamInfo.eIdx_Shading_CCT, eIdx_Shading_CCT_new,i4CCT
        );
    }
//#endif
    return  eIdx_Shading_CCT_new;
}

MVOID
IspTuningCustom::
reset_ISO_SmoothBuffer()
{
    total_RA_num_frames_= 0;
    MY_LOG("reset_ISO total_RA_num_frames_=0");
    memset(ISO_Buffer_, 6, sizeof(ISO_Buffer_));
    MY_LOG("[%s] total_RA_num_frames_(%d)", __FUNCTION__, total_RA_num_frames_ );
    MY_LOG("[%s] ISO_Buffer_[] = {%d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n}", __FUNCTION__,
        ISO_Buffer_[0], ISO_Buffer_[1], ISO_Buffer_[2], ISO_Buffer_[3], ISO_Buffer_[4],
        ISO_Buffer_[5], ISO_Buffer_[6], ISO_Buffer_[7], ISO_Buffer_[8], ISO_Buffer_[9] );
}

static MINT32 ratioMapping(MINT32 i4Iso)
{
#define LERP(x, lo_x, lo_y, hi_x, hi_y)\
    (((hi_x) - (x))*(lo_y) + ((x) - (lo_x))*(hi_y)) / ((hi_x) - (lo_x))

    static const MINT32 iso[10] =
    {100, 200, 400, 800, 1200, 1600, 2000, 2400, 2800, 3200};

    static const MINT32 rto[10] =
    //{24, 22, 20, 18, 16, 14, 12, 10, 8, 6}; //Tower modify for iso1600 Noise 2014-12-26
    {30, 28, 26, 24, 22, 20, 18, 16, 14, 12};

    MINT32 i = 0;
    MINT32 i4Rto = 32;

    if (i4Iso < iso[0])
    {
        i4Rto = rto[0];
    }
    else if (i4Iso >= iso[9])
    {
        i4Rto = rto[9];
    }
    else
    {
        for (i = 1; i < 10; i++)
        {
            if (i4Iso < iso[i])
                break;
        }
        i4Rto = LERP(i4Iso, iso[i-1], rto[i-1], iso[i], rto[i]);
    }
    return i4Rto;
}

MINT32
IspTuningCustom::
evaluate_Shading_Ratio  (
        RAWIspCamInfo const& rCamInfo
)
{
    /*
        Sample code for evaluate shading ratio.
        The shading ratio is an integer ranging from 0(0%) to 32(100%).
        All informations can be obtained via rCamInfo.
        The following sample code shows a shading ratio evaluated by ISO value with temporal smoothness.
    */
    MINT32 Avg_Frm_Cnt = 5;
    MINT32 i = 0;
    MINT32 i4Rto = 8; //32;
    MINT32 i4Iso = rCamInfo.rAEInfo.u4RealISOValue;

    customRatioLog = property_get_int32("vendor.debug.shading.custom.ratio.log", 0);

    int idx = total_RA_num_frames_ % Avg_Frm_Cnt;
    int *p_global_Ra = ISO_Buffer_;
    int n_frames, avgISO;

    ISO_Buffer_[idx] = i4Iso;

    // to prevent total frames overflow
    if (total_RA_num_frames_ >= 65535){
        total_RA_num_frames_ = 0;
    }
    total_RA_num_frames_++;
    if (total_RA_num_frames_ < 20){
        avgISO = 8;
        CAM_LOGD_IF(customRatioLog,"[%s] first avgISO = %d\n", __FUNCTION__, avgISO);
    } else {
        // smooth
        n_frames = ( total_RA_num_frames_ <  Avg_Frm_Cnt) ? (total_RA_num_frames_) : (Avg_Frm_Cnt);
        avgISO = 0;
        for (int k = 0; k < n_frames; k++) {
            avgISO += ISO_Buffer_[k];
        }
        avgISO /= n_frames;
        CAM_LOGD_IF(customRatioLog,"[%s] ISO_Buffer_[] = {%d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n}", __FUNCTION__,
        ISO_Buffer_[0], ISO_Buffer_[1], ISO_Buffer_[2], ISO_Buffer_[3], ISO_Buffer_[4],
        ISO_Buffer_[5], ISO_Buffer_[6], ISO_Buffer_[7], ISO_Buffer_[8], ISO_Buffer_[9] );
        CAM_LOGD_IF(customRatioLog,"[%s] avgISO = %d", __FUNCTION__, avgISO);
        if (rCamInfo.rFlashInfo.isFlash == 2)
        {
            i4Rto = ratioMapping(i4Iso);
            MY_LOG("[%s] Main flash iso(%d), ratio(%d)", __FUNCTION__, i4Iso, i4Rto);
        }
        else
        {
            i4Rto = ratioMapping(avgISO);
        }
    }
    return i4Rto;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
EIndex_ISO_T
IspTuningCustom::
map_ISO_value_to_index(MUINT32 const u4Iso) const
{
    //MY_LOG("%s()\n", __FUNCTION__);

    if ( u4Iso < 150 )
    {
        return  eIDX_ISO_100;
    }
    else if ( u4Iso < 300 )
    {
        return  eIDX_ISO_200;
    }
    else if ( u4Iso < 600 )
    {
        return  eIDX_ISO_400;
    }
    else if ( u4Iso < 1000 )
    {
        return  eIDX_ISO_800;
    }
    else if ( u4Iso < 1400 )
    {
        return  eIDX_ISO_1200;
    }
    else if ( u4Iso < 1800 )
    {
        return  eIDX_ISO_1600;
    }
    else if ( u4Iso < 2200 )
    {
        return  eIDX_ISO_2000;
    }
    else if ( u4Iso < 2600 )
    {
        return  eIDX_ISO_2400;
    }
    else if ( u4Iso < 3000 )
    {
        return  eIDX_ISO_2800;
    }

    return  eIDX_ISO_3200;
}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32
IspTuningCustom::
map_ISO_index_to_value(EIndex_ISO_T const u4IsoIdx) const
{
    //MY_LOG("%s()\n", __FUNCTION__);

    if ( u4IsoIdx == eIDX_ISO_100 )
    {
        return  100;
    }
    else if ( u4IsoIdx == eIDX_ISO_200 )
    {
        return  200;
    }
    else if ( u4IsoIdx == eIDX_ISO_400 )
    {
        return  400;
    }
    else if ( u4IsoIdx == eIDX_ISO_800 )
    {
        return  800;
    }
    else if ( u4IsoIdx == eIDX_ISO_1200 )
    {
        return  1200;
    }
    else if ( u4IsoIdx == eIDX_ISO_1600 )
    {
        return  1600;
    }
    else if ( u4IsoIdx == eIDX_ISO_2000 )
    {
        return  2000;
    }
    else if ( u4IsoIdx == eIDX_ISO_2400 )
    {
        return  2400;
    }
    else if ( u4IsoIdx == eIDX_ISO_2800 )
    {
        return  2800;
    }
    else if ( u4IsoIdx == eIDX_ISO_3200 )
    {
        return  3200;
    }

    return  0;  // If no ISO Index matched, return 0.

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32
IspTuningCustom::
remap_ISO_value(MUINT32 const u4Iso) const
{
    MUINT32 remapIso = u4Iso;

    //add your remap ISO code here

    //MY_LOG("[%s] ISO: in(%d), out(%d)", __FUNCTION__, u4Iso, remapIso);
    return remapIso;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
EIndex_ISO_T
IspTuningCustom::
map_ISO_value_to_upper_index(MUINT32 const u4Iso) const
{
    //MY_LOG("%s()\n", __FUNCTION__);

    if ( u4Iso <= 100 )
    {
        return  eIDX_ISO_100;
    }
    else if ( u4Iso <= 200 )
    {
        return  eIDX_ISO_200;
    }
    else if ( u4Iso <= 400 )
    {
        return  eIDX_ISO_400;
    }
    else if ( u4Iso <= 800 )
    {
        return  eIDX_ISO_800;
    }
    else if ( u4Iso <= 1200 )
    {
        return  eIDX_ISO_1200;
    }
    else if ( u4Iso <= 1600 )
    {
        return  eIDX_ISO_1600;
    }
    else if ( u4Iso <= 2000 )
    {
        return  eIDX_ISO_2000;
    }
    else if ( u4Iso <= 2400 )
    {
        return  eIDX_ISO_2400;
    }
    else if ( u4Iso <= 2800 )
    {
        return  eIDX_ISO_2800;
    }

    return  eIDX_ISO_3200;

}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
EIndex_ISO_T
IspTuningCustom::
map_ISO_value_to_lower_index(MUINT32 const u4Iso) const
{
    //MY_LOG("%s()\n", __FUNCTION__);

    if ( u4Iso < 200 )
    {
        return  eIDX_ISO_100;
    }
    else if ( u4Iso < 400 )
    {
        return  eIDX_ISO_200;
    }
    else if ( u4Iso < 800 )
    {
        return  eIDX_ISO_400;
    }
    else if ( u4Iso < 1200 )
    {
        return  eIDX_ISO_800;
    }
    else if ( u4Iso < 1600 )
    {
        return  eIDX_ISO_1200;
    }
    else if ( u4Iso < 2000 )
    {
        return  eIDX_ISO_1600;
    }
    else if ( u4Iso < 2400 )
    {
        return  eIDX_ISO_2000;
    }
    else if ( u4Iso < 2800 )
    {
        return  eIDX_ISO_2400;
    }
    else if ( u4Iso < 3200 )
    {
        return  eIDX_ISO_2800;
    }

    return  eIDX_ISO_3200;

}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refineLightSourceAWBGainforMultiCCM(AWB_GAIN_T& rD65, AWB_GAIN_T& rTL84, AWB_GAIN_T& rCWF, AWB_GAIN_T& rA)
{
    MY_LOG("%s()\n", __FUNCTION__);

    MY_LOG("D65 AWB Gain = (%d, %d, %d)\n", rD65.i4R, rD65.i4G, rD65.i4B);
    MY_LOG("TL84 AWB Gain = (%d, %d, %d)\n", rTL84.i4R, rTL84.i4G, rTL84.i4B);
    MY_LOG("CWF AWB Gain = (%d, %d, %d)\n", rCWF.i4R, rCWF.i4G, rCWF.i4B);
    MY_LOG("A AWB Gain = (%d, %d, %d)\n", rA.i4R, rA.i4G, rA.i4B);
}

