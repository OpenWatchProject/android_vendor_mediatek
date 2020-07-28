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
#ifndef _CAMERA_AE_PLINETABLE_GC8034MIPIRAW_H
#define _CAMERA_AE_PLINETABLE_GC8034MIPIRAW_H

#include "camera_custom_AEPlinetable.h"
static strEvPline sPreviewPLineTable_60Hz =
{
{
    {30,1152,1104, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.28  BV=11.75
    {30,1280,1056, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.37  BV=11.66
    {30,1408,1040, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.48  BV=11.54
    {30,1536,1032, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.60  BV=11.43
    {30,1664,1024, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.70  BV=11.32
    {45,1152,1048, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.20  BV=11.24
    {45,1152,1120, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.30  BV=11.14
    {45,1280,1088, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.41  BV=11.03
    {45,1408,1040, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.48  BV=10.96
    {60,1152,1056, 0, 0, 0},  /* TV = 14.02(4 lines)  AV=2.00  SV=5.21  BV=10.81 */
    {60,1152,1128, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.31  BV=10.72
    {60,1280,1072, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.39  BV=10.64
    {75,1152,1032, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.18  BV=10.52
    {75,1152,1088, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.26  BV=10.45
    {75,1280,1056, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.37  BV=10.34
    {90,1152,1040, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.19  BV=10.25
    {90,1280,1024, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.32  BV=10.12
    {104,1152,1040, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.19  BV=10.04
    {104,1152,1104, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.28  BV=9.95
    {119,1152,1032, 0, 0, 0},  //TV = 13.04(8 lines)  AV=2.00  SV=5.18  BV=9.86
    {134,1136,1024, 0, 0, 0},  /* TV = 12.87(9 lines)  AV=2.00  SV=5.15  BV=9.72 */
    {134,1152,1064, 0, 0, 0},  //TV = 12.87(9 lines)  AV=2.00  SV=5.23  BV=9.64
    {149,1152,1032, 0, 0, 0},  //TV = 12.71(10 lines)  AV=2.00  SV=5.18  BV=9.53
    {164,1136,1024, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.15  BV=9.42
    {164,1152,1088, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.26  BV=9.32
    {179,1152,1040, 0, 0, 0},  /* TV = 12.45(12 lines)  AV=2.00  SV=5.19  BV=9.26 */
    {193,1152,1032, 0, 0, 0},  //TV = 12.34(13 lines)  AV=2.00  SV=5.18  BV=9.16
    {208,1152,1032, 0, 0, 0},  //TV = 12.23(14 lines)  AV=2.00  SV=5.18  BV=9.05
    {223,1152,1032, 0, 0, 0},  //TV = 12.13(15 lines)  AV=2.00  SV=5.18  BV=8.95
    {238,1152,1032, 0, 0, 0},  //TV = 12.04(16 lines)  AV=2.00  SV=5.18  BV=8.86
    {253,1152,1040, 0, 0, 0},  //TV = 11.95(17 lines)  AV=2.00  SV=5.19  BV=8.76
    {268,1152,1056, 0, 0, 0},  //TV = 11.87(18 lines)  AV=2.00  SV=5.21  BV=8.65
    {297,1152,1024, 0, 0, 0},  //TV = 11.72(20 lines)  AV=2.00  SV=5.17  BV=8.55
    {312,1152,1040, 0, 0, 0},  //TV = 11.65(21 lines)  AV=2.00  SV=5.19  BV=8.45
    {342,1152,1024, 0, 0, 0},  //TV = 11.51(23 lines)  AV=2.00  SV=5.17  BV=8.34
    {371,1136,1024, 0, 0, 0},  //TV = 11.40(25 lines)  AV=2.00  SV=5.15  BV=8.25
    {386,1152,1032, 0, 0, 0},  //TV = 11.34(26 lines)  AV=2.00  SV=5.18  BV=8.16
    {416,1152,1032, 0, 0, 0},  //TV = 11.23(28 lines)  AV=2.00  SV=5.18  BV=8.05
    {446,1152,1032, 0, 0, 0},  //TV = 11.13(30 lines)  AV=2.00  SV=5.18  BV=7.95
    {475,1152,1032, 0, 0, 0},  //TV = 11.04(32 lines)  AV=2.00  SV=5.18  BV=7.86
    {520,1136,1024, 0, 0, 0},  //TV = 10.91(35 lines)  AV=2.00  SV=5.15  BV=7.76
    {564,1136,1024, 0, 0, 0},  //TV = 10.79(38 lines)  AV=2.00  SV=5.15  BV=7.64
    {594,1152,1024, 0, 0, 0},  //TV = 10.72(40 lines)  AV=2.00  SV=5.17  BV=7.55
    {638,1152,1024, 0, 0, 0},  //TV = 10.61(43 lines)  AV=2.00  SV=5.17  BV=7.44
    {683,1152,1024, 0, 0, 0},  //TV = 10.52(46 lines)  AV=2.00  SV=5.17  BV=7.35
    {727,1152,1024, 0, 0, 0},  //TV = 10.43(49 lines)  AV=2.00  SV=5.17  BV=7.26
    {787,1152,1024, 0, 0, 0},  //TV = 10.31(53 lines)  AV=2.00  SV=5.17  BV=7.14
    {846,1136,1024, 0, 0, 0},  //TV = 10.21(57 lines)  AV=2.00  SV=5.15  BV=7.06
    {905,1136,1024, 0, 0, 0},  //TV = 10.11(61 lines)  AV=2.00  SV=5.15  BV=6.96
    {965,1152,1024, 0, 0, 0},  //TV = 10.02(65 lines)  AV=2.00  SV=5.17  BV=6.85
    {1039,1152,1024, 0, 0, 0},  //TV = 9.91(70 lines)  AV=2.00  SV=5.17  BV=6.74
    {1128,1136,1024, 0, 0, 0},  //TV = 9.79(76 lines)  AV=2.00  SV=5.15  BV=6.64
    {1202,1136,1024, 0, 0, 0},  //TV = 9.70(81 lines)  AV=2.00  SV=5.15  BV=6.55
    {1291,1136,1024, 0, 0, 0},  //TV = 9.60(87 lines)  AV=2.00  SV=5.15  BV=6.45
    {1380,1136,1024, 0, 0, 0},  //TV = 9.50(93 lines)  AV=2.00  SV=5.15  BV=6.35
    {1484,1136,1024, 0, 0, 0},  //TV = 9.40(100 lines)  AV=2.00  SV=5.15  BV=6.25
    {1588,1136,1024, 0, 0, 0},  //TV = 9.30(107 lines)  AV=2.00  SV=5.15  BV=6.15
    {1692,1136,1024, 0, 0, 0},  //TV = 9.21(114 lines)  AV=2.00  SV=5.15  BV=6.06
    {1810,1136,1024, 0, 0, 0},  //TV = 9.11(122 lines)  AV=2.00  SV=5.15  BV=5.96
    {1959,1136,1024, 0, 0, 0},  //TV = 9.00(132 lines)  AV=2.00  SV=5.15  BV=5.85
    {2107,1136,1024, 0, 0, 0},  //TV = 8.89(142 lines)  AV=2.00  SV=5.15  BV=5.74
    {2255,1136,1024, 0, 0, 0},  //TV = 8.79(152 lines)  AV=2.00  SV=5.15  BV=5.64
    {2389,1136,1024, 0, 0, 0},  //TV = 8.71(161 lines)  AV=2.00  SV=5.15  BV=5.56
    {2582,1136,1024, 0, 0, 0},  //TV = 8.60(174 lines)  AV=2.00  SV=5.15  BV=5.45
    {2774,1136,1024, 0, 0, 0},  //TV = 8.49(187 lines)  AV=2.00  SV=5.15  BV=5.34
    {2967,1136,1024, 0, 0, 0},  //TV = 8.40(200 lines)  AV=2.00  SV=5.15  BV=5.25
    {3175,1136,1024, 0, 0, 0},  //TV = 8.30(214 lines)  AV=2.00  SV=5.15  BV=5.15
    {3397,1136,1024, 0, 0, 0},  //TV = 8.20(229 lines)  AV=2.00  SV=5.15  BV=5.05
    {3635,1136,1024, 0, 0, 0},  //TV = 8.10(245 lines)  AV=2.00  SV=5.15  BV=4.95
    {3932,1136,1024, 0, 0, 0},  //TV = 7.99(265 lines)  AV=2.00  SV=5.15  BV=4.84
    {4199,1136,1024, 0, 0, 0},  //TV = 7.90(283 lines)  AV=2.00  SV=5.15  BV=4.75
    {4495,1136,1024, 0, 0, 0},  //TV = 7.80(303 lines)  AV=2.00  SV=5.15  BV=4.65
    {4822,1136,1024, 0, 0, 0},  //TV = 7.70(325 lines)  AV=2.00  SV=5.15  BV=4.55
    {5163,1136,1024, 0, 0, 0},  //TV = 7.60(348 lines)  AV=2.00  SV=5.15  BV=4.45
    {5563,1136,1024, 0, 0, 0},  //TV = 7.49(375 lines)  AV=2.00  SV=5.15  BV=4.34
    {5949,1136,1024, 0, 0, 0},  //TV = 7.39(401 lines)  AV=2.00  SV=5.15  BV=4.24
    {6379,1136,1024, 0, 0, 0},  //TV = 7.29(430 lines)  AV=2.00  SV=5.15  BV=4.14
    {6824,1136,1024, 0, 0, 0},  //TV = 7.20(460 lines)  AV=2.00  SV=5.15  BV=4.05
    {7314,1136,1024, 0, 0, 0},  //TV = 7.10(493 lines)  AV=2.00  SV=5.15  BV=3.95
    {7833,1136,1024, 0, 0, 0},  //TV = 7.00(528 lines)  AV=2.00  SV=5.15  BV=3.85
    {8337,1136,1024, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.15  BV=3.76
    {8337,1152,1088, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.26  BV=3.65
    {8337,1280,1048, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.36  BV=3.55
    {8337,1408,1024, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.46  BV=3.45
    {8337,1408,1104, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.57  BV=3.34
    {8337,1536,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.65  BV=3.26
    {8337,1664,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.77  BV=3.14
    {8337,1792,1056, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.85  BV=3.05
    {8337,1920,1064, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.96  BV=2.94
    {8337,2048,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=6.07  BV=2.84
    {16659,1136,1024, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.15  BV=2.76
    {16659,1152,1088, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.26  BV=2.65
    {16659,1280,1056, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.37  BV=2.54
    {16659,1408,1024, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.46  BV=2.45
    {16659,1408,1096, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.56  BV=2.35
    {16659,1536,1080, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.66  BV=2.25
    {24996,1152,1024, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.17  BV=2.15
    {24996,1152,1096, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.27  BV=2.05
    {24996,1280,1056, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.37  BV=1.96
    {24996,1408,1032, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.47  BV=1.85
    {33332,1136,1024, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.15  BV=1.76
    {33332,1152,1088, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.26  BV=1.65
    {33332,1280,1048, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.36  BV=1.55
    {33332,1408,1024, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.46  BV=1.45
    {33332,1408,1096, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.56  BV=1.35
    {33332,1536,1072, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.65  BV=1.26
    {33332,1664,1072, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.77  BV=1.14
    {33332,1792,1064, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.86  BV=1.04
    {33332,1920,1056, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.95  BV=0.96
    {33332,2048,1064, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=6.06  BV=0.85
    {33332,2176,1072, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=6.15  BV=0.75
    {41669,1920,1040, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=5.93  BV=0.66
    {41669,2048,1048, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=6.03  BV=0.55
    {41669,2176,1056, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=6.13  BV=0.45
    {50006,1920,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.97  BV=0.35
    {50006,2048,1080, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.08  BV=0.24
    {50006,2304,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.17  BV=0.15
    {50006,2432,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.27  BV=0.05
    {50006,2560,1064, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.38  BV=-0.06
    {50006,2816,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.47  BV=-0.15
    {50006,2944,1056, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.57  BV=-0.25
    {50006,3200,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.67  BV=-0.34
    {50006,3456,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.78  BV=-0.46
    {50006,3712,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.87  BV=-0.55
    {50006,3968,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.97  BV=-0.64
    {50006,4224,1048, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.08  BV=-0.76
    {58328,3968,1024, 0, 0, 0},  //TV = 4.10(3932 lines)  AV=2.00  SV=6.95  BV=-0.85
    {58328,4224,1032, 0, 0, 0},  //TV = 4.10(3932 lines)  AV=2.00  SV=7.06  BV=-0.96
    {66664,3968,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=6.97  BV=-1.06
    {66664,4224,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.06  BV=-1.15
    {66664,4480,1040, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.15  BV=-1.24
    {66664,4864,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.26  BV=-1.35
    {66664,5248,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.36  BV=-1.45
    {66664,5632,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.46  BV=-1.55
    {66664,6016,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.55  BV=-1.65
    {66664,6400,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.66  BV=-1.75
    {66664,6912,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.75  BV=-1.85
    {66664,7424,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.86  BV=-1.95
    {66664,7936,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.95  BV=-2.05
    {66664,8448,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=8.06  BV=-2.15
    {75001,8064,1032, 0, 0, 0},  //TV = 3.74(5056 lines)  AV=2.00  SV=7.99  BV=-2.25
    {83338,7808,1024, 0, 0, 0},  //TV = 3.58(5618 lines)  AV=2.00  SV=7.93  BV=-2.35
    {83338,8320,1032, 0, 0, 0},  //TV = 3.58(5618 lines)  AV=2.00  SV=8.03  BV=-2.45
    {91660,8192,1024, 0, 0, 0},  //TV = 3.45(6179 lines)  AV=2.00  SV=8.00  BV=-2.55
    {99996,7936,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=7.97  BV=-2.64
    {99996,8576,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.08  BV=-2.76
    {99996,9216,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.17  BV=-2.85
    {99996,9856,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.27  BV=-2.94
    {99996,10624,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.38  BV=-3.05
    {99996,11392,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.48  BV=-3.15
    {99996,12160,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.57  BV=-3.25
    {99996,13056,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.67  BV=-3.35
    {99996,13952,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.77  BV=-3.45
    {99996,14976,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.87  BV=-3.55
    {99996,16000,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.97  BV=-3.64
    {99996,16384,1024, 0, 0, 0},  /* TV = 3.32(6741 lines)  AV=2.00  SV=9.00  BV=-3.68 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
},
};

static strEvPline sPreviewPLineTable_50Hz =
{
{
    {30,1152,1104, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.28  BV=11.75
    {30,1280,1056, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.37  BV=11.66
    {30,1408,1040, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.48  BV=11.54
    {30,1536,1032, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.60  BV=11.43
    {30,1664,1024, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.70  BV=11.32
    {45,1152,1048, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.20  BV=11.24
    {45,1152,1120, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.30  BV=11.14
    {45,1280,1088, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.41  BV=11.03
    {45,1408,1040, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.48  BV=10.96
    {60,1152,1056, 0, 0, 0},  /* TV = 14.02(4 lines)  AV=2.00  SV=5.21  BV=10.81 */
    {60,1152,1128, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.31  BV=10.72
    {60,1280,1072, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.39  BV=10.64
    {75,1152,1032, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.18  BV=10.52
    {75,1152,1088, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.26  BV=10.45
    {75,1280,1056, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.37  BV=10.34
    {90,1152,1040, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.19  BV=10.25
    {90,1280,1024, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.32  BV=10.12
    {104,1152,1040, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.19  BV=10.04
    {104,1152,1104, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.28  BV=9.95
    {119,1152,1032, 0, 0, 0},  //TV = 13.04(8 lines)  AV=2.00  SV=5.18  BV=9.86
    {134,1136,1024, 0, 0, 0},  /* TV = 12.87(9 lines)  AV=2.00  SV=5.15  BV=9.72 */
    {134,1152,1064, 0, 0, 0},  //TV = 12.87(9 lines)  AV=2.00  SV=5.23  BV=9.64
    {149,1152,1032, 0, 0, 0},  //TV = 12.71(10 lines)  AV=2.00  SV=5.18  BV=9.53
    {164,1136,1024, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.15  BV=9.42
    {164,1152,1088, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.26  BV=9.32
    {179,1152,1040, 0, 0, 0},  /* TV = 12.45(12 lines)  AV=2.00  SV=5.19  BV=9.26 */
    {193,1152,1032, 0, 0, 0},  //TV = 12.34(13 lines)  AV=2.00  SV=5.18  BV=9.16
    {208,1152,1032, 0, 0, 0},  //TV = 12.23(14 lines)  AV=2.00  SV=5.18  BV=9.05
    {223,1152,1032, 0, 0, 0},  //TV = 12.13(15 lines)  AV=2.00  SV=5.18  BV=8.95
    {238,1152,1032, 0, 0, 0},  //TV = 12.04(16 lines)  AV=2.00  SV=5.18  BV=8.86
    {253,1152,1040, 0, 0, 0},  //TV = 11.95(17 lines)  AV=2.00  SV=5.19  BV=8.76
    {268,1152,1056, 0, 0, 0},  //TV = 11.87(18 lines)  AV=2.00  SV=5.21  BV=8.65
    {297,1152,1024, 0, 0, 0},  //TV = 11.72(20 lines)  AV=2.00  SV=5.17  BV=8.55
    {312,1152,1040, 0, 0, 0},  //TV = 11.65(21 lines)  AV=2.00  SV=5.19  BV=8.45
    {342,1152,1024, 0, 0, 0},  //TV = 11.51(23 lines)  AV=2.00  SV=5.17  BV=8.34
    {371,1136,1024, 0, 0, 0},  //TV = 11.40(25 lines)  AV=2.00  SV=5.15  BV=8.25
    {386,1152,1032, 0, 0, 0},  //TV = 11.34(26 lines)  AV=2.00  SV=5.18  BV=8.16
    {416,1152,1032, 0, 0, 0},  //TV = 11.23(28 lines)  AV=2.00  SV=5.18  BV=8.05
    {446,1152,1032, 0, 0, 0},  //TV = 11.13(30 lines)  AV=2.00  SV=5.18  BV=7.95
    {475,1152,1032, 0, 0, 0},  //TV = 11.04(32 lines)  AV=2.00  SV=5.18  BV=7.86
    {520,1136,1024, 0, 0, 0},  //TV = 10.91(35 lines)  AV=2.00  SV=5.15  BV=7.76
    {564,1136,1024, 0, 0, 0},  //TV = 10.79(38 lines)  AV=2.00  SV=5.15  BV=7.64
    {594,1152,1024, 0, 0, 0},  //TV = 10.72(40 lines)  AV=2.00  SV=5.17  BV=7.55
    {638,1152,1024, 0, 0, 0},  //TV = 10.61(43 lines)  AV=2.00  SV=5.17  BV=7.44
    {683,1152,1024, 0, 0, 0},  //TV = 10.52(46 lines)  AV=2.00  SV=5.17  BV=7.35
    {727,1152,1024, 0, 0, 0},  //TV = 10.43(49 lines)  AV=2.00  SV=5.17  BV=7.26
    {787,1152,1024, 0, 0, 0},  //TV = 10.31(53 lines)  AV=2.00  SV=5.17  BV=7.14
    {846,1136,1024, 0, 0, 0},  //TV = 10.21(57 lines)  AV=2.00  SV=5.15  BV=7.06
    {905,1136,1024, 0, 0, 0},  //TV = 10.11(61 lines)  AV=2.00  SV=5.15  BV=6.96
    {965,1152,1024, 0, 0, 0},  //TV = 10.02(65 lines)  AV=2.00  SV=5.17  BV=6.85
    {1039,1152,1024, 0, 0, 0},  //TV = 9.91(70 lines)  AV=2.00  SV=5.17  BV=6.74
    {1128,1136,1024, 0, 0, 0},  //TV = 9.79(76 lines)  AV=2.00  SV=5.15  BV=6.64
    {1202,1136,1024, 0, 0, 0},  //TV = 9.70(81 lines)  AV=2.00  SV=5.15  BV=6.55
    {1291,1136,1024, 0, 0, 0},  //TV = 9.60(87 lines)  AV=2.00  SV=5.15  BV=6.45
    {1380,1136,1024, 0, 0, 0},  //TV = 9.50(93 lines)  AV=2.00  SV=5.15  BV=6.35
    {1484,1136,1024, 0, 0, 0},  //TV = 9.40(100 lines)  AV=2.00  SV=5.15  BV=6.25
    {1588,1136,1024, 0, 0, 0},  //TV = 9.30(107 lines)  AV=2.00  SV=5.15  BV=6.15
    {1692,1136,1024, 0, 0, 0},  //TV = 9.21(114 lines)  AV=2.00  SV=5.15  BV=6.06
    {1810,1136,1024, 0, 0, 0},  //TV = 9.11(122 lines)  AV=2.00  SV=5.15  BV=5.96
    {1959,1136,1024, 0, 0, 0},  //TV = 9.00(132 lines)  AV=2.00  SV=5.15  BV=5.85
    {2107,1136,1024, 0, 0, 0},  //TV = 8.89(142 lines)  AV=2.00  SV=5.15  BV=5.74
    {2255,1136,1024, 0, 0, 0},  //TV = 8.79(152 lines)  AV=2.00  SV=5.15  BV=5.64
    {2389,1136,1024, 0, 0, 0},  //TV = 8.71(161 lines)  AV=2.00  SV=5.15  BV=5.56
    {2582,1136,1024, 0, 0, 0},  //TV = 8.60(174 lines)  AV=2.00  SV=5.15  BV=5.45
    {2774,1136,1024, 0, 0, 0},  //TV = 8.49(187 lines)  AV=2.00  SV=5.15  BV=5.34
    {2967,1136,1024, 0, 0, 0},  //TV = 8.40(200 lines)  AV=2.00  SV=5.15  BV=5.25
    {3175,1136,1024, 0, 0, 0},  //TV = 8.30(214 lines)  AV=2.00  SV=5.15  BV=5.15
    {3397,1136,1024, 0, 0, 0},  //TV = 8.20(229 lines)  AV=2.00  SV=5.15  BV=5.05
    {3635,1136,1024, 0, 0, 0},  //TV = 8.10(245 lines)  AV=2.00  SV=5.15  BV=4.95
    {3932,1136,1024, 0, 0, 0},  //TV = 7.99(265 lines)  AV=2.00  SV=5.15  BV=4.84
    {4199,1136,1024, 0, 0, 0},  //TV = 7.90(283 lines)  AV=2.00  SV=5.15  BV=4.75
    {4495,1136,1024, 0, 0, 0},  //TV = 7.80(303 lines)  AV=2.00  SV=5.15  BV=4.65
    {4822,1136,1024, 0, 0, 0},  //TV = 7.70(325 lines)  AV=2.00  SV=5.15  BV=4.55
    {5163,1136,1024, 0, 0, 0},  //TV = 7.60(348 lines)  AV=2.00  SV=5.15  BV=4.45
    {5563,1136,1024, 0, 0, 0},  //TV = 7.49(375 lines)  AV=2.00  SV=5.15  BV=4.34
    {5949,1136,1024, 0, 0, 0},  //TV = 7.39(401 lines)  AV=2.00  SV=5.15  BV=4.24
    {6379,1136,1024, 0, 0, 0},  //TV = 7.29(430 lines)  AV=2.00  SV=5.15  BV=4.14
    {6824,1136,1024, 0, 0, 0},  //TV = 7.20(460 lines)  AV=2.00  SV=5.15  BV=4.05
    {7314,1136,1024, 0, 0, 0},  //TV = 7.10(493 lines)  AV=2.00  SV=5.15  BV=3.95
    {7833,1136,1024, 0, 0, 0},  //TV = 7.00(528 lines)  AV=2.00  SV=5.15  BV=3.85
    {8397,1136,1024, 0, 0, 0},  //TV = 6.90(566 lines)  AV=2.00  SV=5.15  BV=3.75
    {8990,1136,1024, 0, 0, 0},  //TV = 6.80(606 lines)  AV=2.00  SV=5.15  BV=3.65
    {9628,1136,1024, 0, 0, 0},  //TV = 6.70(649 lines)  AV=2.00  SV=5.15  BV=3.55
    {9999,1152,1040, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.19  BV=3.45
    {9999,1152,1112, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.29  BV=3.36
    {9999,1280,1080, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.40  BV=3.25
    {9999,1408,1056, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.50  BV=3.14
    {9999,1536,1032, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.60  BV=3.05
    {9999,1664,1024, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.70  BV=2.94
    {9999,1664,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.79  BV=2.86
    {9999,1792,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.89  BV=2.75
    {9999,1920,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.99  BV=2.65
    {9999,2176,1032, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=6.10  BV=2.55
    {19997,1152,1040, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.19  BV=2.45
    {19997,1152,1112, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.29  BV=2.36
    {19997,1280,1080, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.40  BV=2.25
    {19997,1408,1048, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.49  BV=2.15
    {19997,1536,1032, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.60  BV=2.05
    {19997,1664,1024, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.70  BV=1.94
    {29995,1152,1048, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.20  BV=1.86
    {29995,1152,1128, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.31  BV=1.75
    {29995,1280,1088, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.41  BV=1.65
    {29995,1408,1064, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.51  BV=1.54
    {29995,1536,1040, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.61  BV=1.45
    {29995,1664,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.71  BV=1.35
    {29995,1792,1024, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.81  BV=1.25
    {29995,1920,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.92  BV=1.14
    {29995,2048,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.01  BV=1.05
    {29995,2176,1040, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.11  BV=0.95
    {29995,2304,1056, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.21  BV=0.84
    {40008,1792,1088, 0, 0, 0},  //TV = 4.64(2697 lines)  AV=2.00  SV=5.89  BV=0.75
    {40008,1920,1088, 0, 0, 0},  //TV = 4.64(2697 lines)  AV=2.00  SV=5.99  BV=0.65
    {40008,2176,1024, 0, 0, 0},  //TV = 4.64(2697 lines)  AV=2.00  SV=6.09  BV=0.56
    {50006,1792,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.87  BV=0.45
    {50006,1920,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.97  BV=0.35
    {50006,2048,1080, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.08  BV=0.24
    {50006,2304,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.17  BV=0.15
    {50006,2432,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.27  BV=0.05
    {50006,2560,1064, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.38  BV=-0.06
    {50006,2816,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.47  BV=-0.15
    {50006,2944,1056, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.57  BV=-0.25
    {50006,3200,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.67  BV=-0.34
    {50006,3456,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.78  BV=-0.46
    {50006,3712,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.87  BV=-0.55
    {50006,3968,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.97  BV=-0.64
    {50006,4224,1048, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.08  BV=-0.76
    {60004,3840,1024, 0, 0, 0},  //TV = 4.06(4045 lines)  AV=2.00  SV=6.91  BV=-0.85
    {60004,4096,1032, 0, 0, 0},  //TV = 4.06(4045 lines)  AV=2.00  SV=7.01  BV=-0.95
    {70002,3712,1048, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=6.89  BV=-1.05
    {70002,3968,1048, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=6.99  BV=-1.15
    {70002,4352,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.09  BV=-1.25
    {70002,4608,1040, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.19  BV=-1.36
    {70002,4992,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.29  BV=-1.45
    {70002,5376,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.39  BV=-1.56
    {70002,5632,1040, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.48  BV=-1.65
    {70002,6144,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.58  BV=-1.75
    {70002,6528,1032, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.68  BV=-1.85
    {70002,7040,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.78  BV=-1.94
    {70002,7552,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.88  BV=-2.05
    {70002,8064,1032, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.99  BV=-2.15
    {70002,8704,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=8.09  BV=-2.25
    {80000,8064,1032, 0, 0, 0},  //TV = 3.64(5393 lines)  AV=2.00  SV=7.99  BV=-2.34
    {89998,7680,1032, 0, 0, 0},  //TV = 3.47(6067 lines)  AV=2.00  SV=7.92  BV=-2.44
    {89998,8320,1024, 0, 0, 0},  //TV = 3.47(6067 lines)  AV=2.00  SV=8.02  BV=-2.55
    {99996,7936,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=7.97  BV=-2.64
    {99996,8576,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.08  BV=-2.76
    {99996,9216,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.17  BV=-2.85
    {99996,9856,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.27  BV=-2.94
    {99996,10624,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.38  BV=-3.05
    {99996,11392,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.48  BV=-3.15
    {99996,12160,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.57  BV=-3.25
    {99996,13056,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.67  BV=-3.35
    {99996,13952,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.77  BV=-3.45
    {99996,14976,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.87  BV=-3.55
    {99996,16000,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.97  BV=-3.64
    {99996,16384,1024, 0, 0, 0},  /* TV = 3.32(6741 lines)  AV=2.00  SV=9.00  BV=-3.68 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
},
};

static strAETable g_AE_PreviewAutoTable =
{
    AETABLE_RPEVIEW_AUTO, //eAETableID
    156, //u4TotalIndex
    -20, //i4StrobeTrigerBV
    118, //i4MaxBV
    -37, //i4MinBV
    90, //i4EffectiveMaxBV
    0, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_AUTO, //ISO
    sPreviewPLineTable_60Hz,
    sPreviewPLineTable_50Hz,
    NULL,
};
static strEvPline sCapturePLineTable_60Hz =
{
{
    {30,1152,1104, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.28  BV=11.75
    {30,1280,1056, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.37  BV=11.66
    {30,1408,1040, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.48  BV=11.54
    {30,1536,1032, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.60  BV=11.43
    {30,1664,1024, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.70  BV=11.32
    {45,1152,1048, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.20  BV=11.24
    {45,1152,1120, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.30  BV=11.14
    {45,1280,1088, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.41  BV=11.03
    {45,1408,1040, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.48  BV=10.96
    {60,1152,1056, 0, 0, 0},  /* TV = 14.02(4 lines)  AV=2.00  SV=5.21  BV=10.81 */
    {60,1152,1128, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.31  BV=10.72
    {60,1280,1072, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.39  BV=10.64
    {75,1152,1032, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.18  BV=10.52
    {75,1152,1088, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.26  BV=10.45
    {75,1280,1056, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.37  BV=10.34
    {90,1152,1040, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.19  BV=10.25
    {90,1280,1024, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.32  BV=10.12
    {104,1152,1040, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.19  BV=10.04
    {104,1152,1104, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.28  BV=9.95
    {119,1152,1032, 0, 0, 0},  //TV = 13.04(8 lines)  AV=2.00  SV=5.18  BV=9.86
    {134,1136,1024, 0, 0, 0},  /* TV = 12.87(9 lines)  AV=2.00  SV=5.15  BV=9.72 */
    {134,1152,1064, 0, 0, 0},  //TV = 12.87(9 lines)  AV=2.00  SV=5.23  BV=9.64
    {149,1152,1032, 0, 0, 0},  //TV = 12.71(10 lines)  AV=2.00  SV=5.18  BV=9.53
    {164,1136,1024, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.15  BV=9.42
    {164,1152,1088, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.26  BV=9.32
    {179,1152,1040, 0, 0, 0},  /* TV = 12.45(12 lines)  AV=2.00  SV=5.19  BV=9.26 */
    {193,1152,1032, 0, 0, 0},  //TV = 12.34(13 lines)  AV=2.00  SV=5.18  BV=9.16
    {208,1152,1032, 0, 0, 0},  //TV = 12.23(14 lines)  AV=2.00  SV=5.18  BV=9.05
    {223,1152,1032, 0, 0, 0},  //TV = 12.13(15 lines)  AV=2.00  SV=5.18  BV=8.95
    {238,1152,1032, 0, 0, 0},  //TV = 12.04(16 lines)  AV=2.00  SV=5.18  BV=8.86
    {253,1152,1040, 0, 0, 0},  //TV = 11.95(17 lines)  AV=2.00  SV=5.19  BV=8.76
    {268,1152,1056, 0, 0, 0},  //TV = 11.87(18 lines)  AV=2.00  SV=5.21  BV=8.65
    {297,1152,1024, 0, 0, 0},  //TV = 11.72(20 lines)  AV=2.00  SV=5.17  BV=8.55
    {312,1152,1040, 0, 0, 0},  //TV = 11.65(21 lines)  AV=2.00  SV=5.19  BV=8.45
    {342,1152,1024, 0, 0, 0},  //TV = 11.51(23 lines)  AV=2.00  SV=5.17  BV=8.34
    {371,1136,1024, 0, 0, 0},  //TV = 11.40(25 lines)  AV=2.00  SV=5.15  BV=8.25
    {386,1152,1032, 0, 0, 0},  //TV = 11.34(26 lines)  AV=2.00  SV=5.18  BV=8.16
    {416,1152,1032, 0, 0, 0},  //TV = 11.23(28 lines)  AV=2.00  SV=5.18  BV=8.05
    {446,1152,1032, 0, 0, 0},  //TV = 11.13(30 lines)  AV=2.00  SV=5.18  BV=7.95
    {475,1152,1032, 0, 0, 0},  //TV = 11.04(32 lines)  AV=2.00  SV=5.18  BV=7.86
    {520,1136,1024, 0, 0, 0},  //TV = 10.91(35 lines)  AV=2.00  SV=5.15  BV=7.76
    {564,1136,1024, 0, 0, 0},  //TV = 10.79(38 lines)  AV=2.00  SV=5.15  BV=7.64
    {594,1152,1024, 0, 0, 0},  //TV = 10.72(40 lines)  AV=2.00  SV=5.17  BV=7.55
    {638,1152,1024, 0, 0, 0},  //TV = 10.61(43 lines)  AV=2.00  SV=5.17  BV=7.44
    {683,1152,1024, 0, 0, 0},  //TV = 10.52(46 lines)  AV=2.00  SV=5.17  BV=7.35
    {727,1152,1024, 0, 0, 0},  //TV = 10.43(49 lines)  AV=2.00  SV=5.17  BV=7.26
    {787,1152,1024, 0, 0, 0},  //TV = 10.31(53 lines)  AV=2.00  SV=5.17  BV=7.14
    {846,1136,1024, 0, 0, 0},  //TV = 10.21(57 lines)  AV=2.00  SV=5.15  BV=7.06
    {905,1136,1024, 0, 0, 0},  //TV = 10.11(61 lines)  AV=2.00  SV=5.15  BV=6.96
    {965,1152,1024, 0, 0, 0},  //TV = 10.02(65 lines)  AV=2.00  SV=5.17  BV=6.85
    {1039,1152,1024, 0, 0, 0},  //TV = 9.91(70 lines)  AV=2.00  SV=5.17  BV=6.74
    {1128,1136,1024, 0, 0, 0},  //TV = 9.79(76 lines)  AV=2.00  SV=5.15  BV=6.64
    {1202,1136,1024, 0, 0, 0},  //TV = 9.70(81 lines)  AV=2.00  SV=5.15  BV=6.55
    {1291,1136,1024, 0, 0, 0},  //TV = 9.60(87 lines)  AV=2.00  SV=5.15  BV=6.45
    {1380,1136,1024, 0, 0, 0},  //TV = 9.50(93 lines)  AV=2.00  SV=5.15  BV=6.35
    {1484,1136,1024, 0, 0, 0},  //TV = 9.40(100 lines)  AV=2.00  SV=5.15  BV=6.25
    {1588,1136,1024, 0, 0, 0},  //TV = 9.30(107 lines)  AV=2.00  SV=5.15  BV=6.15
    {1692,1136,1024, 0, 0, 0},  //TV = 9.21(114 lines)  AV=2.00  SV=5.15  BV=6.06
    {1810,1136,1024, 0, 0, 0},  //TV = 9.11(122 lines)  AV=2.00  SV=5.15  BV=5.96
    {1959,1136,1024, 0, 0, 0},  //TV = 9.00(132 lines)  AV=2.00  SV=5.15  BV=5.85
    {2107,1136,1024, 0, 0, 0},  //TV = 8.89(142 lines)  AV=2.00  SV=5.15  BV=5.74
    {2255,1136,1024, 0, 0, 0},  //TV = 8.79(152 lines)  AV=2.00  SV=5.15  BV=5.64
    {2389,1136,1024, 0, 0, 0},  //TV = 8.71(161 lines)  AV=2.00  SV=5.15  BV=5.56
    {2582,1136,1024, 0, 0, 0},  //TV = 8.60(174 lines)  AV=2.00  SV=5.15  BV=5.45
    {2774,1136,1024, 0, 0, 0},  //TV = 8.49(187 lines)  AV=2.00  SV=5.15  BV=5.34
    {2967,1136,1024, 0, 0, 0},  //TV = 8.40(200 lines)  AV=2.00  SV=5.15  BV=5.25
    {3175,1136,1024, 0, 0, 0},  //TV = 8.30(214 lines)  AV=2.00  SV=5.15  BV=5.15
    {3397,1136,1024, 0, 0, 0},  //TV = 8.20(229 lines)  AV=2.00  SV=5.15  BV=5.05
    {3635,1136,1024, 0, 0, 0},  //TV = 8.10(245 lines)  AV=2.00  SV=5.15  BV=4.95
    {3932,1136,1024, 0, 0, 0},  //TV = 7.99(265 lines)  AV=2.00  SV=5.15  BV=4.84
    {4199,1136,1024, 0, 0, 0},  //TV = 7.90(283 lines)  AV=2.00  SV=5.15  BV=4.75
    {4495,1136,1024, 0, 0, 0},  //TV = 7.80(303 lines)  AV=2.00  SV=5.15  BV=4.65
    {4822,1136,1024, 0, 0, 0},  //TV = 7.70(325 lines)  AV=2.00  SV=5.15  BV=4.55
    {5163,1136,1024, 0, 0, 0},  //TV = 7.60(348 lines)  AV=2.00  SV=5.15  BV=4.45
    {5563,1136,1024, 0, 0, 0},  //TV = 7.49(375 lines)  AV=2.00  SV=5.15  BV=4.34
    {5949,1136,1024, 0, 0, 0},  //TV = 7.39(401 lines)  AV=2.00  SV=5.15  BV=4.24
    {6379,1136,1024, 0, 0, 0},  //TV = 7.29(430 lines)  AV=2.00  SV=5.15  BV=4.14
    {6824,1136,1024, 0, 0, 0},  //TV = 7.20(460 lines)  AV=2.00  SV=5.15  BV=4.05
    {7314,1136,1024, 0, 0, 0},  //TV = 7.10(493 lines)  AV=2.00  SV=5.15  BV=3.95
    {7833,1136,1024, 0, 0, 0},  //TV = 7.00(528 lines)  AV=2.00  SV=5.15  BV=3.85
    {8337,1136,1024, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.15  BV=3.76
    {8337,1152,1088, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.26  BV=3.65
    {8337,1280,1048, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.36  BV=3.55
    {8337,1408,1024, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.46  BV=3.45
    {8337,1408,1104, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.57  BV=3.34
    {8337,1536,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.65  BV=3.26
    {8337,1664,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.77  BV=3.14
    {8337,1792,1056, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.85  BV=3.05
    {8337,1920,1064, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.96  BV=2.94
    {8337,2048,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=6.07  BV=2.84
    {16659,1136,1024, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.15  BV=2.76
    {16659,1152,1088, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.26  BV=2.65
    {16659,1280,1056, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.37  BV=2.54
    {16659,1408,1024, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.46  BV=2.45
    {16659,1408,1096, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.56  BV=2.35
    {16659,1536,1080, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.66  BV=2.25
    {24996,1152,1024, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.17  BV=2.15
    {24996,1152,1096, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.27  BV=2.05
    {24996,1280,1056, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.37  BV=1.96
    {24996,1408,1032, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.47  BV=1.85
    {33332,1136,1024, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.15  BV=1.76
    {33332,1152,1088, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.26  BV=1.65
    {33332,1280,1048, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.36  BV=1.55
    {33332,1408,1024, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.46  BV=1.45
    {33332,1408,1096, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.56  BV=1.35
    {33332,1536,1072, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.65  BV=1.26
    {33332,1664,1072, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.77  BV=1.14
    {33332,1792,1064, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.86  BV=1.04
    {33332,1920,1056, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.95  BV=0.96
    {33332,2048,1064, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=6.06  BV=0.85
    {33332,2176,1072, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=6.15  BV=0.75
    {41669,1920,1040, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=5.93  BV=0.66
    {41669,2048,1048, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=6.03  BV=0.55
    {41669,2176,1056, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=6.13  BV=0.45
    {50006,1920,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.97  BV=0.35
    {50006,2048,1080, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.08  BV=0.24
    {50006,2304,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.17  BV=0.15
    {50006,2432,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.27  BV=0.05
    {50006,2560,1064, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.38  BV=-0.06
    {50006,2816,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.47  BV=-0.15
    {50006,2944,1056, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.57  BV=-0.25
    {50006,3200,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.67  BV=-0.34
    {50006,3456,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.78  BV=-0.46
    {50006,3712,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.87  BV=-0.55
    {50006,3968,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.97  BV=-0.64
    {50006,4224,1048, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.08  BV=-0.76
    {58328,3968,1024, 0, 0, 0},  //TV = 4.10(3932 lines)  AV=2.00  SV=6.95  BV=-0.85
    {58328,4224,1032, 0, 0, 0},  //TV = 4.10(3932 lines)  AV=2.00  SV=7.06  BV=-0.96
    {66664,3968,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=6.97  BV=-1.06
    {66664,4224,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.06  BV=-1.15
    {66664,4480,1040, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.15  BV=-1.24
    {66664,4864,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.26  BV=-1.35
    {66664,5248,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.36  BV=-1.45
    {66664,5632,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.46  BV=-1.55
    {66664,6016,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.55  BV=-1.65
    {66664,6400,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.66  BV=-1.75
    {66664,6912,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.75  BV=-1.85
    {66664,7424,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.86  BV=-1.95
    {66664,7936,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.95  BV=-2.05
    {66664,8448,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=8.06  BV=-2.15
    {75001,8064,1032, 0, 0, 0},  //TV = 3.74(5056 lines)  AV=2.00  SV=7.99  BV=-2.25
    {83338,7808,1024, 0, 0, 0},  //TV = 3.58(5618 lines)  AV=2.00  SV=7.93  BV=-2.35
    {83338,8320,1032, 0, 0, 0},  //TV = 3.58(5618 lines)  AV=2.00  SV=8.03  BV=-2.45
    {91660,8192,1024, 0, 0, 0},  //TV = 3.45(6179 lines)  AV=2.00  SV=8.00  BV=-2.55
    {99996,7936,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=7.97  BV=-2.64
    {99996,8576,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.08  BV=-2.76
    {99996,9216,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.17  BV=-2.85
    {99996,9856,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.27  BV=-2.94
    {99996,10624,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.38  BV=-3.05
    {99996,11392,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.48  BV=-3.15
    {99996,12160,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.57  BV=-3.25
    {99996,13056,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.67  BV=-3.35
    {99996,13952,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.77  BV=-3.45
    {99996,14976,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.87  BV=-3.55
    {99996,16000,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.97  BV=-3.64
    {99996,16384,1024, 0, 0, 0},  /* TV = 3.32(6741 lines)  AV=2.00  SV=9.00  BV=-3.68 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
},
};

static strEvPline sCapturePLineTable_50Hz =
{
{
    {30,1152,1104, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.28  BV=11.75
    {30,1280,1056, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.37  BV=11.66
    {30,1408,1040, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.48  BV=11.54
    {30,1536,1032, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.60  BV=11.43
    {30,1664,1024, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.70  BV=11.32
    {45,1152,1048, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.20  BV=11.24
    {45,1152,1120, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.30  BV=11.14
    {45,1280,1088, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.41  BV=11.03
    {45,1408,1040, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.48  BV=10.96
    {60,1152,1056, 0, 0, 0},  /* TV = 14.02(4 lines)  AV=2.00  SV=5.21  BV=10.81 */
    {60,1152,1128, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.31  BV=10.72
    {60,1280,1072, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.39  BV=10.64
    {75,1152,1032, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.18  BV=10.52
    {75,1152,1088, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.26  BV=10.45
    {75,1280,1056, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.37  BV=10.34
    {90,1152,1040, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.19  BV=10.25
    {90,1280,1024, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.32  BV=10.12
    {104,1152,1040, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.19  BV=10.04
    {104,1152,1104, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.28  BV=9.95
    {119,1152,1032, 0, 0, 0},  //TV = 13.04(8 lines)  AV=2.00  SV=5.18  BV=9.86
    {134,1136,1024, 0, 0, 0},  /* TV = 12.87(9 lines)  AV=2.00  SV=5.15  BV=9.72 */
    {134,1152,1064, 0, 0, 0},  //TV = 12.87(9 lines)  AV=2.00  SV=5.23  BV=9.64
    {149,1152,1032, 0, 0, 0},  //TV = 12.71(10 lines)  AV=2.00  SV=5.18  BV=9.53
    {164,1136,1024, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.15  BV=9.42
    {164,1152,1088, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.26  BV=9.32
    {179,1152,1040, 0, 0, 0},  /* TV = 12.45(12 lines)  AV=2.00  SV=5.19  BV=9.26 */
    {193,1152,1032, 0, 0, 0},  //TV = 12.34(13 lines)  AV=2.00  SV=5.18  BV=9.16
    {208,1152,1032, 0, 0, 0},  //TV = 12.23(14 lines)  AV=2.00  SV=5.18  BV=9.05
    {223,1152,1032, 0, 0, 0},  //TV = 12.13(15 lines)  AV=2.00  SV=5.18  BV=8.95
    {238,1152,1032, 0, 0, 0},  //TV = 12.04(16 lines)  AV=2.00  SV=5.18  BV=8.86
    {253,1152,1040, 0, 0, 0},  //TV = 11.95(17 lines)  AV=2.00  SV=5.19  BV=8.76
    {268,1152,1056, 0, 0, 0},  //TV = 11.87(18 lines)  AV=2.00  SV=5.21  BV=8.65
    {297,1152,1024, 0, 0, 0},  //TV = 11.72(20 lines)  AV=2.00  SV=5.17  BV=8.55
    {312,1152,1040, 0, 0, 0},  //TV = 11.65(21 lines)  AV=2.00  SV=5.19  BV=8.45
    {342,1152,1024, 0, 0, 0},  //TV = 11.51(23 lines)  AV=2.00  SV=5.17  BV=8.34
    {371,1136,1024, 0, 0, 0},  //TV = 11.40(25 lines)  AV=2.00  SV=5.15  BV=8.25
    {386,1152,1032, 0, 0, 0},  //TV = 11.34(26 lines)  AV=2.00  SV=5.18  BV=8.16
    {416,1152,1032, 0, 0, 0},  //TV = 11.23(28 lines)  AV=2.00  SV=5.18  BV=8.05
    {446,1152,1032, 0, 0, 0},  //TV = 11.13(30 lines)  AV=2.00  SV=5.18  BV=7.95
    {475,1152,1032, 0, 0, 0},  //TV = 11.04(32 lines)  AV=2.00  SV=5.18  BV=7.86
    {520,1136,1024, 0, 0, 0},  //TV = 10.91(35 lines)  AV=2.00  SV=5.15  BV=7.76
    {564,1136,1024, 0, 0, 0},  //TV = 10.79(38 lines)  AV=2.00  SV=5.15  BV=7.64
    {594,1152,1024, 0, 0, 0},  //TV = 10.72(40 lines)  AV=2.00  SV=5.17  BV=7.55
    {638,1152,1024, 0, 0, 0},  //TV = 10.61(43 lines)  AV=2.00  SV=5.17  BV=7.44
    {683,1152,1024, 0, 0, 0},  //TV = 10.52(46 lines)  AV=2.00  SV=5.17  BV=7.35
    {727,1152,1024, 0, 0, 0},  //TV = 10.43(49 lines)  AV=2.00  SV=5.17  BV=7.26
    {787,1152,1024, 0, 0, 0},  //TV = 10.31(53 lines)  AV=2.00  SV=5.17  BV=7.14
    {846,1136,1024, 0, 0, 0},  //TV = 10.21(57 lines)  AV=2.00  SV=5.15  BV=7.06
    {905,1136,1024, 0, 0, 0},  //TV = 10.11(61 lines)  AV=2.00  SV=5.15  BV=6.96
    {965,1152,1024, 0, 0, 0},  //TV = 10.02(65 lines)  AV=2.00  SV=5.17  BV=6.85
    {1039,1152,1024, 0, 0, 0},  //TV = 9.91(70 lines)  AV=2.00  SV=5.17  BV=6.74
    {1128,1136,1024, 0, 0, 0},  //TV = 9.79(76 lines)  AV=2.00  SV=5.15  BV=6.64
    {1202,1136,1024, 0, 0, 0},  //TV = 9.70(81 lines)  AV=2.00  SV=5.15  BV=6.55
    {1291,1136,1024, 0, 0, 0},  //TV = 9.60(87 lines)  AV=2.00  SV=5.15  BV=6.45
    {1380,1136,1024, 0, 0, 0},  //TV = 9.50(93 lines)  AV=2.00  SV=5.15  BV=6.35
    {1484,1136,1024, 0, 0, 0},  //TV = 9.40(100 lines)  AV=2.00  SV=5.15  BV=6.25
    {1588,1136,1024, 0, 0, 0},  //TV = 9.30(107 lines)  AV=2.00  SV=5.15  BV=6.15
    {1692,1136,1024, 0, 0, 0},  //TV = 9.21(114 lines)  AV=2.00  SV=5.15  BV=6.06
    {1810,1136,1024, 0, 0, 0},  //TV = 9.11(122 lines)  AV=2.00  SV=5.15  BV=5.96
    {1959,1136,1024, 0, 0, 0},  //TV = 9.00(132 lines)  AV=2.00  SV=5.15  BV=5.85
    {2107,1136,1024, 0, 0, 0},  //TV = 8.89(142 lines)  AV=2.00  SV=5.15  BV=5.74
    {2255,1136,1024, 0, 0, 0},  //TV = 8.79(152 lines)  AV=2.00  SV=5.15  BV=5.64
    {2389,1136,1024, 0, 0, 0},  //TV = 8.71(161 lines)  AV=2.00  SV=5.15  BV=5.56
    {2582,1136,1024, 0, 0, 0},  //TV = 8.60(174 lines)  AV=2.00  SV=5.15  BV=5.45
    {2774,1136,1024, 0, 0, 0},  //TV = 8.49(187 lines)  AV=2.00  SV=5.15  BV=5.34
    {2967,1136,1024, 0, 0, 0},  //TV = 8.40(200 lines)  AV=2.00  SV=5.15  BV=5.25
    {3175,1136,1024, 0, 0, 0},  //TV = 8.30(214 lines)  AV=2.00  SV=5.15  BV=5.15
    {3397,1136,1024, 0, 0, 0},  //TV = 8.20(229 lines)  AV=2.00  SV=5.15  BV=5.05
    {3635,1136,1024, 0, 0, 0},  //TV = 8.10(245 lines)  AV=2.00  SV=5.15  BV=4.95
    {3932,1136,1024, 0, 0, 0},  //TV = 7.99(265 lines)  AV=2.00  SV=5.15  BV=4.84
    {4199,1136,1024, 0, 0, 0},  //TV = 7.90(283 lines)  AV=2.00  SV=5.15  BV=4.75
    {4495,1136,1024, 0, 0, 0},  //TV = 7.80(303 lines)  AV=2.00  SV=5.15  BV=4.65
    {4822,1136,1024, 0, 0, 0},  //TV = 7.70(325 lines)  AV=2.00  SV=5.15  BV=4.55
    {5163,1136,1024, 0, 0, 0},  //TV = 7.60(348 lines)  AV=2.00  SV=5.15  BV=4.45
    {5563,1136,1024, 0, 0, 0},  //TV = 7.49(375 lines)  AV=2.00  SV=5.15  BV=4.34
    {5949,1136,1024, 0, 0, 0},  //TV = 7.39(401 lines)  AV=2.00  SV=5.15  BV=4.24
    {6379,1136,1024, 0, 0, 0},  //TV = 7.29(430 lines)  AV=2.00  SV=5.15  BV=4.14
    {6824,1136,1024, 0, 0, 0},  //TV = 7.20(460 lines)  AV=2.00  SV=5.15  BV=4.05
    {7314,1136,1024, 0, 0, 0},  //TV = 7.10(493 lines)  AV=2.00  SV=5.15  BV=3.95
    {7833,1136,1024, 0, 0, 0},  //TV = 7.00(528 lines)  AV=2.00  SV=5.15  BV=3.85
    {8397,1136,1024, 0, 0, 0},  //TV = 6.90(566 lines)  AV=2.00  SV=5.15  BV=3.75
    {8990,1136,1024, 0, 0, 0},  //TV = 6.80(606 lines)  AV=2.00  SV=5.15  BV=3.65
    {9628,1136,1024, 0, 0, 0},  //TV = 6.70(649 lines)  AV=2.00  SV=5.15  BV=3.55
    {9999,1152,1040, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.19  BV=3.45
    {9999,1152,1112, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.29  BV=3.36
    {9999,1280,1080, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.40  BV=3.25
    {9999,1408,1056, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.50  BV=3.14
    {9999,1536,1032, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.60  BV=3.05
    {9999,1664,1024, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.70  BV=2.94
    {9999,1664,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.79  BV=2.86
    {9999,1792,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.89  BV=2.75
    {9999,1920,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.99  BV=2.65
    {9999,2176,1032, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=6.10  BV=2.55
    {19997,1152,1040, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.19  BV=2.45
    {19997,1152,1112, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.29  BV=2.36
    {19997,1280,1080, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.40  BV=2.25
    {19997,1408,1048, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.49  BV=2.15
    {19997,1536,1032, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.60  BV=2.05
    {19997,1664,1024, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.70  BV=1.94
    {29995,1152,1048, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.20  BV=1.86
    {29995,1152,1128, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.31  BV=1.75
    {29995,1280,1088, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.41  BV=1.65
    {29995,1408,1064, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.51  BV=1.54
    {29995,1536,1040, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.61  BV=1.45
    {29995,1664,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.71  BV=1.35
    {29995,1792,1024, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.81  BV=1.25
    {29995,1920,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.92  BV=1.14
    {29995,2048,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.01  BV=1.05
    {29995,2176,1040, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.11  BV=0.95
    {29995,2304,1056, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.21  BV=0.84
    {40008,1792,1088, 0, 0, 0},  //TV = 4.64(2697 lines)  AV=2.00  SV=5.89  BV=0.75
    {40008,1920,1088, 0, 0, 0},  //TV = 4.64(2697 lines)  AV=2.00  SV=5.99  BV=0.65
    {40008,2176,1024, 0, 0, 0},  //TV = 4.64(2697 lines)  AV=2.00  SV=6.09  BV=0.56
    {50006,1792,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.87  BV=0.45
    {50006,1920,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.97  BV=0.35
    {50006,2048,1080, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.08  BV=0.24
    {50006,2304,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.17  BV=0.15
    {50006,2432,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.27  BV=0.05
    {50006,2560,1064, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.38  BV=-0.06
    {50006,2816,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.47  BV=-0.15
    {50006,2944,1056, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.57  BV=-0.25
    {50006,3200,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.67  BV=-0.34
    {50006,3456,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.78  BV=-0.46
    {50006,3712,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.87  BV=-0.55
    {50006,3968,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.97  BV=-0.64
    {50006,4224,1048, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.08  BV=-0.76
    {60004,3840,1024, 0, 0, 0},  //TV = 4.06(4045 lines)  AV=2.00  SV=6.91  BV=-0.85
    {60004,4096,1032, 0, 0, 0},  //TV = 4.06(4045 lines)  AV=2.00  SV=7.01  BV=-0.95
    {70002,3712,1048, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=6.89  BV=-1.05
    {70002,3968,1048, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=6.99  BV=-1.15
    {70002,4352,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.09  BV=-1.25
    {70002,4608,1040, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.19  BV=-1.36
    {70002,4992,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.29  BV=-1.45
    {70002,5376,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.39  BV=-1.56
    {70002,5632,1040, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.48  BV=-1.65
    {70002,6144,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.58  BV=-1.75
    {70002,6528,1032, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.68  BV=-1.85
    {70002,7040,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.78  BV=-1.94
    {70002,7552,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.88  BV=-2.05
    {70002,8064,1032, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.99  BV=-2.15
    {70002,8704,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=8.09  BV=-2.25
    {80000,8064,1032, 0, 0, 0},  //TV = 3.64(5393 lines)  AV=2.00  SV=7.99  BV=-2.34
    {89998,7680,1032, 0, 0, 0},  //TV = 3.47(6067 lines)  AV=2.00  SV=7.92  BV=-2.44
    {89998,8320,1024, 0, 0, 0},  //TV = 3.47(6067 lines)  AV=2.00  SV=8.02  BV=-2.55
    {99996,7936,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=7.97  BV=-2.64
    {99996,8576,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.08  BV=-2.76
    {99996,9216,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.17  BV=-2.85
    {99996,9856,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.27  BV=-2.94
    {99996,10624,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.38  BV=-3.05
    {99996,11392,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.48  BV=-3.15
    {99996,12160,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.57  BV=-3.25
    {99996,13056,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.67  BV=-3.35
    {99996,13952,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.77  BV=-3.45
    {99996,14976,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.87  BV=-3.55
    {99996,16000,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.97  BV=-3.64
    {99996,16384,1024, 0, 0, 0},  /* TV = 3.32(6741 lines)  AV=2.00  SV=9.00  BV=-3.68 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
},
};

static strAETable g_AE_CaptureAutoTable =
{
    AETABLE_CAPTURE_AUTO, //eAETableID
    156, //u4TotalIndex
    -20, //i4StrobeTrigerBV
    118, //i4MaxBV
    -37, //i4MinBV
    90, //i4EffectiveMaxBV
    0, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_AUTO, //ISO
    sCapturePLineTable_60Hz,
    sCapturePLineTable_50Hz,
    NULL,
};
static strEvPline sVideoPLineTable_60Hz =
{
{
    {30,1152,1104, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.28  BV=11.75
    {30,1280,1056, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.37  BV=11.66
    {30,1408,1040, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.48  BV=11.54
    {30,1536,1032, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.60  BV=11.43
    {30,1664,1024, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.70  BV=11.32
    {45,1152,1048, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.20  BV=11.24
    {45,1152,1120, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.30  BV=11.14
    {45,1280,1088, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.41  BV=11.03
    {45,1408,1040, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.48  BV=10.96
    {60,1152,1056, 0, 0, 0},  /* TV = 14.02(4 lines)  AV=2.00  SV=5.21  BV=10.81 */
    {60,1152,1128, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.31  BV=10.72
    {60,1280,1072, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.39  BV=10.64
    {75,1152,1032, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.18  BV=10.52
    {75,1152,1088, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.26  BV=10.45
    {75,1280,1056, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.37  BV=10.34
    {90,1152,1040, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.19  BV=10.25
    {90,1280,1024, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.32  BV=10.12
    {104,1152,1040, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.19  BV=10.04
    {104,1152,1104, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.28  BV=9.95
    {119,1152,1032, 0, 0, 0},  //TV = 13.04(8 lines)  AV=2.00  SV=5.18  BV=9.86
    {134,1136,1024, 0, 0, 0},  /* TV = 12.87(9 lines)  AV=2.00  SV=5.15  BV=9.72 */
    {134,1152,1064, 0, 0, 0},  //TV = 12.87(9 lines)  AV=2.00  SV=5.23  BV=9.64
    {149,1152,1032, 0, 0, 0},  //TV = 12.71(10 lines)  AV=2.00  SV=5.18  BV=9.53
    {164,1136,1024, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.15  BV=9.42
    {164,1152,1088, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.26  BV=9.32
    {179,1152,1040, 0, 0, 0},  /* TV = 12.45(12 lines)  AV=2.00  SV=5.19  BV=9.26 */
    {193,1152,1032, 0, 0, 0},  //TV = 12.34(13 lines)  AV=2.00  SV=5.18  BV=9.16
    {208,1152,1032, 0, 0, 0},  //TV = 12.23(14 lines)  AV=2.00  SV=5.18  BV=9.05
    {223,1152,1032, 0, 0, 0},  //TV = 12.13(15 lines)  AV=2.00  SV=5.18  BV=8.95
    {238,1152,1032, 0, 0, 0},  //TV = 12.04(16 lines)  AV=2.00  SV=5.18  BV=8.86
    {253,1152,1040, 0, 0, 0},  //TV = 11.95(17 lines)  AV=2.00  SV=5.19  BV=8.76
    {268,1152,1056, 0, 0, 0},  //TV = 11.87(18 lines)  AV=2.00  SV=5.21  BV=8.65
    {297,1152,1024, 0, 0, 0},  //TV = 11.72(20 lines)  AV=2.00  SV=5.17  BV=8.55
    {312,1152,1040, 0, 0, 0},  //TV = 11.65(21 lines)  AV=2.00  SV=5.19  BV=8.45
    {342,1152,1024, 0, 0, 0},  //TV = 11.51(23 lines)  AV=2.00  SV=5.17  BV=8.34
    {371,1136,1024, 0, 0, 0},  //TV = 11.40(25 lines)  AV=2.00  SV=5.15  BV=8.25
    {386,1152,1032, 0, 0, 0},  //TV = 11.34(26 lines)  AV=2.00  SV=5.18  BV=8.16
    {416,1152,1032, 0, 0, 0},  //TV = 11.23(28 lines)  AV=2.00  SV=5.18  BV=8.05
    {446,1152,1032, 0, 0, 0},  //TV = 11.13(30 lines)  AV=2.00  SV=5.18  BV=7.95
    {475,1152,1032, 0, 0, 0},  //TV = 11.04(32 lines)  AV=2.00  SV=5.18  BV=7.86
    {520,1136,1024, 0, 0, 0},  //TV = 10.91(35 lines)  AV=2.00  SV=5.15  BV=7.76
    {564,1136,1024, 0, 0, 0},  //TV = 10.79(38 lines)  AV=2.00  SV=5.15  BV=7.64
    {594,1152,1024, 0, 0, 0},  //TV = 10.72(40 lines)  AV=2.00  SV=5.17  BV=7.55
    {638,1152,1024, 0, 0, 0},  //TV = 10.61(43 lines)  AV=2.00  SV=5.17  BV=7.44
    {683,1152,1024, 0, 0, 0},  //TV = 10.52(46 lines)  AV=2.00  SV=5.17  BV=7.35
    {727,1152,1024, 0, 0, 0},  //TV = 10.43(49 lines)  AV=2.00  SV=5.17  BV=7.26
    {787,1152,1024, 0, 0, 0},  //TV = 10.31(53 lines)  AV=2.00  SV=5.17  BV=7.14
    {846,1136,1024, 0, 0, 0},  //TV = 10.21(57 lines)  AV=2.00  SV=5.15  BV=7.06
    {905,1136,1024, 0, 0, 0},  //TV = 10.11(61 lines)  AV=2.00  SV=5.15  BV=6.96
    {965,1152,1024, 0, 0, 0},  //TV = 10.02(65 lines)  AV=2.00  SV=5.17  BV=6.85
    {1039,1152,1024, 0, 0, 0},  //TV = 9.91(70 lines)  AV=2.00  SV=5.17  BV=6.74
    {1128,1136,1024, 0, 0, 0},  //TV = 9.79(76 lines)  AV=2.00  SV=5.15  BV=6.64
    {1202,1136,1024, 0, 0, 0},  //TV = 9.70(81 lines)  AV=2.00  SV=5.15  BV=6.55
    {1291,1136,1024, 0, 0, 0},  //TV = 9.60(87 lines)  AV=2.00  SV=5.15  BV=6.45
    {1380,1136,1024, 0, 0, 0},  //TV = 9.50(93 lines)  AV=2.00  SV=5.15  BV=6.35
    {1484,1136,1024, 0, 0, 0},  //TV = 9.40(100 lines)  AV=2.00  SV=5.15  BV=6.25
    {1588,1136,1024, 0, 0, 0},  //TV = 9.30(107 lines)  AV=2.00  SV=5.15  BV=6.15
    {1692,1136,1024, 0, 0, 0},  //TV = 9.21(114 lines)  AV=2.00  SV=5.15  BV=6.06
    {1810,1136,1024, 0, 0, 0},  //TV = 9.11(122 lines)  AV=2.00  SV=5.15  BV=5.96
    {1959,1136,1024, 0, 0, 0},  //TV = 9.00(132 lines)  AV=2.00  SV=5.15  BV=5.85
    {2107,1136,1024, 0, 0, 0},  //TV = 8.89(142 lines)  AV=2.00  SV=5.15  BV=5.74
    {2255,1136,1024, 0, 0, 0},  //TV = 8.79(152 lines)  AV=2.00  SV=5.15  BV=5.64
    {2389,1136,1024, 0, 0, 0},  //TV = 8.71(161 lines)  AV=2.00  SV=5.15  BV=5.56
    {2582,1136,1024, 0, 0, 0},  //TV = 8.60(174 lines)  AV=2.00  SV=5.15  BV=5.45
    {2774,1136,1024, 0, 0, 0},  //TV = 8.49(187 lines)  AV=2.00  SV=5.15  BV=5.34
    {2967,1136,1024, 0, 0, 0},  //TV = 8.40(200 lines)  AV=2.00  SV=5.15  BV=5.25
    {3175,1136,1024, 0, 0, 0},  //TV = 8.30(214 lines)  AV=2.00  SV=5.15  BV=5.15
    {3397,1136,1024, 0, 0, 0},  //TV = 8.20(229 lines)  AV=2.00  SV=5.15  BV=5.05
    {3635,1136,1024, 0, 0, 0},  //TV = 8.10(245 lines)  AV=2.00  SV=5.15  BV=4.95
    {3932,1136,1024, 0, 0, 0},  //TV = 7.99(265 lines)  AV=2.00  SV=5.15  BV=4.84
    {4199,1136,1024, 0, 0, 0},  //TV = 7.90(283 lines)  AV=2.00  SV=5.15  BV=4.75
    {4495,1136,1024, 0, 0, 0},  //TV = 7.80(303 lines)  AV=2.00  SV=5.15  BV=4.65
    {4822,1136,1024, 0, 0, 0},  //TV = 7.70(325 lines)  AV=2.00  SV=5.15  BV=4.55
    {5163,1136,1024, 0, 0, 0},  //TV = 7.60(348 lines)  AV=2.00  SV=5.15  BV=4.45
    {5563,1136,1024, 0, 0, 0},  //TV = 7.49(375 lines)  AV=2.00  SV=5.15  BV=4.34
    {5949,1136,1024, 0, 0, 0},  //TV = 7.39(401 lines)  AV=2.00  SV=5.15  BV=4.24
    {6379,1136,1024, 0, 0, 0},  //TV = 7.29(430 lines)  AV=2.00  SV=5.15  BV=4.14
    {6824,1136,1024, 0, 0, 0},  //TV = 7.20(460 lines)  AV=2.00  SV=5.15  BV=4.05
    {7314,1136,1024, 0, 0, 0},  //TV = 7.10(493 lines)  AV=2.00  SV=5.15  BV=3.95
    {7833,1136,1024, 0, 0, 0},  //TV = 7.00(528 lines)  AV=2.00  SV=5.15  BV=3.85
    {8337,1136,1024, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.15  BV=3.76
    {8337,1152,1088, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.26  BV=3.65
    {8337,1280,1048, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.36  BV=3.55
    {8337,1408,1024, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.46  BV=3.45
    {8337,1408,1104, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.57  BV=3.34
    {8337,1536,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.65  BV=3.26
    {8337,1664,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.77  BV=3.14
    {8337,1792,1056, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.85  BV=3.05
    {8337,1920,1064, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.96  BV=2.94
    {8337,2048,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=6.07  BV=2.84
    {16659,1136,1024, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.15  BV=2.76
    {16659,1152,1088, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.26  BV=2.65
    {16659,1280,1056, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.37  BV=2.54
    {16659,1408,1024, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.46  BV=2.45
    {16659,1408,1096, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.56  BV=2.35
    {16659,1536,1080, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.66  BV=2.25
    {24996,1152,1024, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.17  BV=2.15
    {24996,1152,1096, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.27  BV=2.05
    {24996,1280,1056, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.37  BV=1.96
    {24996,1408,1032, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.47  BV=1.85
    {33288,1136,1024, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=5.15  BV=1.76
    {33288,1152,1088, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=5.26  BV=1.65
    {33288,1280,1056, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=5.37  BV=1.54
    {33288,1408,1024, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=5.46  BV=1.45
    {33288,1408,1104, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=5.57  BV=1.34
    {33288,1536,1080, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=5.66  BV=1.25
    {33288,1664,1072, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=5.77  BV=1.14
    {33288,1792,1064, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=5.86  BV=1.05
    {33288,1920,1056, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=5.95  BV=0.96
    {33288,2048,1064, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=6.06  BV=0.85
    {33288,2176,1072, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=6.15  BV=0.76
    {41669,1920,1040, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=5.93  BV=0.66
    {41669,2048,1048, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=6.03  BV=0.55
    {41669,2176,1056, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=6.13  BV=0.45
    {50006,1920,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.97  BV=0.35
    {50006,2048,1080, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.08  BV=0.24
    {50006,2304,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.17  BV=0.15
    {50006,2432,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.27  BV=0.05
    {50006,2560,1064, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.38  BV=-0.06
    {50006,2816,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.47  BV=-0.15
    {50006,2944,1056, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.57  BV=-0.25
    {50006,3200,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.67  BV=-0.34
    {50006,3456,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.78  BV=-0.46
    {50006,3712,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.87  BV=-0.55
    {50006,3968,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.97  BV=-0.64
    {50006,4224,1048, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.08  BV=-0.76
    {58328,3968,1024, 0, 0, 0},  //TV = 4.10(3932 lines)  AV=2.00  SV=6.95  BV=-0.85
    {58328,4224,1032, 0, 0, 0},  //TV = 4.10(3932 lines)  AV=2.00  SV=7.06  BV=-0.96
    {66620,3968,1032, 0, 0, 0},  //TV = 3.91(4491 lines)  AV=2.00  SV=6.97  BV=-1.06
    {66620,4224,1032, 0, 0, 0},  //TV = 3.91(4491 lines)  AV=2.00  SV=7.06  BV=-1.15
    {66620,4480,1048, 0, 0, 0},  //TV = 3.91(4491 lines)  AV=2.00  SV=7.16  BV=-1.25
    {66620,4864,1032, 0, 0, 0},  //TV = 3.91(4491 lines)  AV=2.00  SV=7.26  BV=-1.35
    {66620,5248,1024, 0, 0, 0},  //TV = 3.91(4491 lines)  AV=2.00  SV=7.36  BV=-1.45
    {66620,5632,1024, 0, 0, 0},  //TV = 3.91(4491 lines)  AV=2.00  SV=7.46  BV=-1.55
    {66620,6016,1024, 0, 0, 0},  //TV = 3.91(4491 lines)  AV=2.00  SV=7.55  BV=-1.65
    {66620,6400,1032, 0, 0, 0},  //TV = 3.91(4491 lines)  AV=2.00  SV=7.66  BV=-1.75
    {66620,6912,1024, 0, 0, 0},  //TV = 3.91(4491 lines)  AV=2.00  SV=7.75  BV=-1.85
    {66620,7424,1024, 0, 0, 0},  //TV = 3.91(4491 lines)  AV=2.00  SV=7.86  BV=-1.95
    {66620,7936,1024, 0, 0, 0},  //TV = 3.91(4491 lines)  AV=2.00  SV=7.95  BV=-2.05
    {66620,8448,1032, 0, 0, 0},  //TV = 3.91(4491 lines)  AV=2.00  SV=8.06  BV=-2.15
    {75001,8064,1032, 0, 0, 0},  //TV = 3.74(5056 lines)  AV=2.00  SV=7.99  BV=-2.25
    {83338,7808,1024, 0, 0, 0},  //TV = 3.58(5618 lines)  AV=2.00  SV=7.93  BV=-2.35
    {83338,8320,1032, 0, 0, 0},  //TV = 3.58(5618 lines)  AV=2.00  SV=8.03  BV=-2.45
    {91660,8192,1024, 0, 0, 0},  //TV = 3.45(6179 lines)  AV=2.00  SV=8.00  BV=-2.55
    {99996,7936,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=7.97  BV=-2.64
    {99996,8576,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.08  BV=-2.76
    {99996,9216,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.17  BV=-2.85
    {99996,9856,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.27  BV=-2.94
    {99996,10624,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.38  BV=-3.05
    {99996,11392,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.48  BV=-3.15
    {99996,12160,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.57  BV=-3.25
    {99996,13056,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.67  BV=-3.35
    {99996,13952,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.77  BV=-3.45
    {99996,14976,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.87  BV=-3.55
    {99996,16000,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.97  BV=-3.64
    {99996,16384,1024, 0, 0, 0},  /* TV = 3.32(6741 lines)  AV=2.00  SV=9.00  BV=-3.68 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
},
};

static strEvPline sVideoPLineTable_50Hz =
{
{
    {30,1152,1104, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.28  BV=11.75
    {30,1280,1056, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.37  BV=11.66
    {30,1408,1040, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.48  BV=11.54
    {30,1536,1032, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.60  BV=11.43
    {30,1664,1024, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.70  BV=11.32
    {45,1152,1048, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.20  BV=11.24
    {45,1152,1120, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.30  BV=11.14
    {45,1280,1088, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.41  BV=11.03
    {45,1408,1040, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.48  BV=10.96
    {60,1152,1056, 0, 0, 0},  /* TV = 14.02(4 lines)  AV=2.00  SV=5.21  BV=10.81 */
    {60,1152,1128, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.31  BV=10.72
    {60,1280,1072, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.39  BV=10.64
    {75,1152,1032, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.18  BV=10.52
    {75,1152,1088, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.26  BV=10.45
    {75,1280,1056, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.37  BV=10.34
    {90,1152,1040, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.19  BV=10.25
    {90,1280,1024, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.32  BV=10.12
    {104,1152,1040, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.19  BV=10.04
    {104,1152,1104, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.28  BV=9.95
    {119,1152,1032, 0, 0, 0},  //TV = 13.04(8 lines)  AV=2.00  SV=5.18  BV=9.86
    {134,1136,1024, 0, 0, 0},  /* TV = 12.87(9 lines)  AV=2.00  SV=5.15  BV=9.72 */
    {134,1152,1064, 0, 0, 0},  //TV = 12.87(9 lines)  AV=2.00  SV=5.23  BV=9.64
    {149,1152,1032, 0, 0, 0},  //TV = 12.71(10 lines)  AV=2.00  SV=5.18  BV=9.53
    {164,1136,1024, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.15  BV=9.42
    {164,1152,1088, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.26  BV=9.32
    {179,1152,1040, 0, 0, 0},  /* TV = 12.45(12 lines)  AV=2.00  SV=5.19  BV=9.26 */
    {193,1152,1032, 0, 0, 0},  //TV = 12.34(13 lines)  AV=2.00  SV=5.18  BV=9.16
    {208,1152,1032, 0, 0, 0},  //TV = 12.23(14 lines)  AV=2.00  SV=5.18  BV=9.05
    {223,1152,1032, 0, 0, 0},  //TV = 12.13(15 lines)  AV=2.00  SV=5.18  BV=8.95
    {238,1152,1032, 0, 0, 0},  //TV = 12.04(16 lines)  AV=2.00  SV=5.18  BV=8.86
    {253,1152,1040, 0, 0, 0},  //TV = 11.95(17 lines)  AV=2.00  SV=5.19  BV=8.76
    {268,1152,1056, 0, 0, 0},  //TV = 11.87(18 lines)  AV=2.00  SV=5.21  BV=8.65
    {297,1152,1024, 0, 0, 0},  //TV = 11.72(20 lines)  AV=2.00  SV=5.17  BV=8.55
    {312,1152,1040, 0, 0, 0},  //TV = 11.65(21 lines)  AV=2.00  SV=5.19  BV=8.45
    {342,1152,1024, 0, 0, 0},  //TV = 11.51(23 lines)  AV=2.00  SV=5.17  BV=8.34
    {371,1136,1024, 0, 0, 0},  //TV = 11.40(25 lines)  AV=2.00  SV=5.15  BV=8.25
    {386,1152,1032, 0, 0, 0},  //TV = 11.34(26 lines)  AV=2.00  SV=5.18  BV=8.16
    {416,1152,1032, 0, 0, 0},  //TV = 11.23(28 lines)  AV=2.00  SV=5.18  BV=8.05
    {446,1152,1032, 0, 0, 0},  //TV = 11.13(30 lines)  AV=2.00  SV=5.18  BV=7.95
    {475,1152,1032, 0, 0, 0},  //TV = 11.04(32 lines)  AV=2.00  SV=5.18  BV=7.86
    {520,1136,1024, 0, 0, 0},  //TV = 10.91(35 lines)  AV=2.00  SV=5.15  BV=7.76
    {564,1136,1024, 0, 0, 0},  //TV = 10.79(38 lines)  AV=2.00  SV=5.15  BV=7.64
    {594,1152,1024, 0, 0, 0},  //TV = 10.72(40 lines)  AV=2.00  SV=5.17  BV=7.55
    {638,1152,1024, 0, 0, 0},  //TV = 10.61(43 lines)  AV=2.00  SV=5.17  BV=7.44
    {683,1152,1024, 0, 0, 0},  //TV = 10.52(46 lines)  AV=2.00  SV=5.17  BV=7.35
    {727,1152,1024, 0, 0, 0},  //TV = 10.43(49 lines)  AV=2.00  SV=5.17  BV=7.26
    {787,1152,1024, 0, 0, 0},  //TV = 10.31(53 lines)  AV=2.00  SV=5.17  BV=7.14
    {846,1136,1024, 0, 0, 0},  //TV = 10.21(57 lines)  AV=2.00  SV=5.15  BV=7.06
    {905,1136,1024, 0, 0, 0},  //TV = 10.11(61 lines)  AV=2.00  SV=5.15  BV=6.96
    {965,1152,1024, 0, 0, 0},  //TV = 10.02(65 lines)  AV=2.00  SV=5.17  BV=6.85
    {1039,1152,1024, 0, 0, 0},  //TV = 9.91(70 lines)  AV=2.00  SV=5.17  BV=6.74
    {1128,1136,1024, 0, 0, 0},  //TV = 9.79(76 lines)  AV=2.00  SV=5.15  BV=6.64
    {1202,1136,1024, 0, 0, 0},  //TV = 9.70(81 lines)  AV=2.00  SV=5.15  BV=6.55
    {1291,1136,1024, 0, 0, 0},  //TV = 9.60(87 lines)  AV=2.00  SV=5.15  BV=6.45
    {1380,1136,1024, 0, 0, 0},  //TV = 9.50(93 lines)  AV=2.00  SV=5.15  BV=6.35
    {1484,1136,1024, 0, 0, 0},  //TV = 9.40(100 lines)  AV=2.00  SV=5.15  BV=6.25
    {1588,1136,1024, 0, 0, 0},  //TV = 9.30(107 lines)  AV=2.00  SV=5.15  BV=6.15
    {1692,1136,1024, 0, 0, 0},  //TV = 9.21(114 lines)  AV=2.00  SV=5.15  BV=6.06
    {1810,1136,1024, 0, 0, 0},  //TV = 9.11(122 lines)  AV=2.00  SV=5.15  BV=5.96
    {1959,1136,1024, 0, 0, 0},  //TV = 9.00(132 lines)  AV=2.00  SV=5.15  BV=5.85
    {2107,1136,1024, 0, 0, 0},  //TV = 8.89(142 lines)  AV=2.00  SV=5.15  BV=5.74
    {2255,1136,1024, 0, 0, 0},  //TV = 8.79(152 lines)  AV=2.00  SV=5.15  BV=5.64
    {2389,1136,1024, 0, 0, 0},  //TV = 8.71(161 lines)  AV=2.00  SV=5.15  BV=5.56
    {2582,1136,1024, 0, 0, 0},  //TV = 8.60(174 lines)  AV=2.00  SV=5.15  BV=5.45
    {2774,1136,1024, 0, 0, 0},  //TV = 8.49(187 lines)  AV=2.00  SV=5.15  BV=5.34
    {2967,1136,1024, 0, 0, 0},  //TV = 8.40(200 lines)  AV=2.00  SV=5.15  BV=5.25
    {3175,1136,1024, 0, 0, 0},  //TV = 8.30(214 lines)  AV=2.00  SV=5.15  BV=5.15
    {3397,1136,1024, 0, 0, 0},  //TV = 8.20(229 lines)  AV=2.00  SV=5.15  BV=5.05
    {3635,1136,1024, 0, 0, 0},  //TV = 8.10(245 lines)  AV=2.00  SV=5.15  BV=4.95
    {3932,1136,1024, 0, 0, 0},  //TV = 7.99(265 lines)  AV=2.00  SV=5.15  BV=4.84
    {4199,1136,1024, 0, 0, 0},  //TV = 7.90(283 lines)  AV=2.00  SV=5.15  BV=4.75
    {4495,1136,1024, 0, 0, 0},  //TV = 7.80(303 lines)  AV=2.00  SV=5.15  BV=4.65
    {4822,1136,1024, 0, 0, 0},  //TV = 7.70(325 lines)  AV=2.00  SV=5.15  BV=4.55
    {5163,1136,1024, 0, 0, 0},  //TV = 7.60(348 lines)  AV=2.00  SV=5.15  BV=4.45
    {5563,1136,1024, 0, 0, 0},  //TV = 7.49(375 lines)  AV=2.00  SV=5.15  BV=4.34
    {5949,1136,1024, 0, 0, 0},  //TV = 7.39(401 lines)  AV=2.00  SV=5.15  BV=4.24
    {6379,1136,1024, 0, 0, 0},  //TV = 7.29(430 lines)  AV=2.00  SV=5.15  BV=4.14
    {6824,1136,1024, 0, 0, 0},  //TV = 7.20(460 lines)  AV=2.00  SV=5.15  BV=4.05
    {7314,1136,1024, 0, 0, 0},  //TV = 7.10(493 lines)  AV=2.00  SV=5.15  BV=3.95
    {7833,1136,1024, 0, 0, 0},  //TV = 7.00(528 lines)  AV=2.00  SV=5.15  BV=3.85
    {8397,1136,1024, 0, 0, 0},  //TV = 6.90(566 lines)  AV=2.00  SV=5.15  BV=3.75
    {8990,1136,1024, 0, 0, 0},  //TV = 6.80(606 lines)  AV=2.00  SV=5.15  BV=3.65
    {9628,1136,1024, 0, 0, 0},  //TV = 6.70(649 lines)  AV=2.00  SV=5.15  BV=3.55
    {9999,1152,1040, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.19  BV=3.45
    {9999,1152,1112, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.29  BV=3.36
    {9999,1280,1080, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.40  BV=3.25
    {9999,1408,1056, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.50  BV=3.14
    {9999,1536,1032, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.60  BV=3.05
    {9999,1664,1024, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.70  BV=2.94
    {9999,1664,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.79  BV=2.86
    {9999,1792,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.89  BV=2.75
    {9999,1920,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.99  BV=2.65
    {9999,2176,1032, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=6.10  BV=2.55
    {19997,1152,1040, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.19  BV=2.45
    {19997,1152,1112, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.29  BV=2.36
    {19997,1280,1080, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.40  BV=2.25
    {19997,1408,1048, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.49  BV=2.15
    {19997,1536,1032, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.60  BV=2.05
    {19997,1664,1024, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.70  BV=1.94
    {29995,1152,1048, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.20  BV=1.86
    {29995,1152,1128, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.31  BV=1.75
    {29995,1280,1088, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.41  BV=1.65
    {29995,1408,1064, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.51  BV=1.54
    {29995,1536,1040, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.61  BV=1.45
    {29995,1664,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.71  BV=1.35
    {29995,1792,1024, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.81  BV=1.25
    {29995,1920,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.92  BV=1.14
    {29995,2048,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.01  BV=1.05
    {29995,2176,1040, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.11  BV=0.95
    {29995,2304,1056, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.21  BV=0.84
    {40008,1792,1088, 0, 0, 0},  //TV = 4.64(2697 lines)  AV=2.00  SV=5.89  BV=0.75
    {40008,1920,1088, 0, 0, 0},  //TV = 4.64(2697 lines)  AV=2.00  SV=5.99  BV=0.65
    {40008,2176,1024, 0, 0, 0},  //TV = 4.64(2697 lines)  AV=2.00  SV=6.09  BV=0.56
    {50006,1792,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.87  BV=0.45
    {50006,1920,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.97  BV=0.35
    {50006,2048,1080, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.08  BV=0.24
    {50006,2304,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.17  BV=0.15
    {50006,2432,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.27  BV=0.05
    {50006,2560,1064, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.38  BV=-0.06
    {50006,2816,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.47  BV=-0.15
    {50006,2944,1056, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.57  BV=-0.25
    {50006,3200,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.67  BV=-0.34
    {50006,3456,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.78  BV=-0.46
    {50006,3712,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.87  BV=-0.55
    {50006,3968,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.97  BV=-0.64
    {50006,4224,1048, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.08  BV=-0.76
    {60004,3840,1024, 0, 0, 0},  //TV = 4.06(4045 lines)  AV=2.00  SV=6.91  BV=-0.85
    {60004,4096,1032, 0, 0, 0},  //TV = 4.06(4045 lines)  AV=2.00  SV=7.01  BV=-0.95
    {70002,3712,1048, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=6.89  BV=-1.05
    {70002,3968,1048, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=6.99  BV=-1.15
    {70002,4352,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.09  BV=-1.25
    {70002,4608,1040, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.19  BV=-1.36
    {70002,4992,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.29  BV=-1.45
    {70002,5376,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.39  BV=-1.56
    {70002,5632,1040, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.48  BV=-1.65
    {70002,6144,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.58  BV=-1.75
    {70002,6528,1032, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.68  BV=-1.85
    {70002,7040,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.78  BV=-1.94
    {70002,7552,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.88  BV=-2.05
    {70002,8064,1032, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.99  BV=-2.15
    {70002,8704,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=8.09  BV=-2.25
    {80000,8064,1032, 0, 0, 0},  //TV = 3.64(5393 lines)  AV=2.00  SV=7.99  BV=-2.34
    {89998,7680,1032, 0, 0, 0},  //TV = 3.47(6067 lines)  AV=2.00  SV=7.92  BV=-2.44
    {89998,8320,1024, 0, 0, 0},  //TV = 3.47(6067 lines)  AV=2.00  SV=8.02  BV=-2.55
    {99996,7936,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=7.97  BV=-2.64
    {99996,8576,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.08  BV=-2.76
    {99996,9216,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.17  BV=-2.85
    {99996,9856,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.27  BV=-2.94
    {99996,10624,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.38  BV=-3.05
    {99996,11392,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.48  BV=-3.15
    {99996,12160,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.57  BV=-3.25
    {99996,13056,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.67  BV=-3.35
    {99996,13952,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.77  BV=-3.45
    {99996,14976,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.87  BV=-3.55
    {99996,16000,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.97  BV=-3.64
    {99996,16384,1024, 0, 0, 0},  /* TV = 3.32(6741 lines)  AV=2.00  SV=9.00  BV=-3.68 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
},
};

static strAETable g_AE_VideoAutoTable =
{
    AETABLE_VIDEO_AUTO, //eAETableID
    156, //u4TotalIndex
    -20, //i4StrobeTrigerBV
    118, //i4MaxBV
    -37, //i4MinBV
    90, //i4EffectiveMaxBV
    0, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_AUTO, //ISO
    sVideoPLineTable_60Hz,
    sVideoPLineTable_50Hz,
    NULL,
};
static strEvPline sVideo1PLineTable_60Hz =
{
{
    {30,1152,1104, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.28  BV=11.75
    {30,1280,1056, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.37  BV=11.66
    {30,1408,1040, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.48  BV=11.54
    {30,1536,1032, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.60  BV=11.43
    {30,1664,1024, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.70  BV=11.32
    {45,1152,1048, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.20  BV=11.24
    {45,1152,1120, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.30  BV=11.14
    {45,1280,1088, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.41  BV=11.03
    {45,1408,1040, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.48  BV=10.96
    {60,1152,1056, 0, 0, 0},  /* TV = 14.02(4 lines)  AV=2.00  SV=5.21  BV=10.81 */
    {60,1152,1128, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.31  BV=10.72
    {60,1280,1072, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.39  BV=10.64
    {75,1152,1032, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.18  BV=10.52
    {75,1152,1088, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.26  BV=10.45
    {75,1280,1056, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.37  BV=10.34
    {90,1152,1040, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.19  BV=10.25
    {90,1280,1024, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.32  BV=10.12
    {104,1152,1040, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.19  BV=10.04
    {104,1152,1104, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.28  BV=9.95
    {119,1152,1032, 0, 0, 0},  //TV = 13.04(8 lines)  AV=2.00  SV=5.18  BV=9.86
    {134,1136,1024, 0, 0, 0},  /* TV = 12.87(9 lines)  AV=2.00  SV=5.15  BV=9.72 */
    {134,1152,1064, 0, 0, 0},  //TV = 12.87(9 lines)  AV=2.00  SV=5.23  BV=9.64
    {149,1152,1032, 0, 0, 0},  //TV = 12.71(10 lines)  AV=2.00  SV=5.18  BV=9.53
    {164,1136,1024, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.15  BV=9.42
    {164,1152,1088, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.26  BV=9.32
    {179,1152,1040, 0, 0, 0},  /* TV = 12.45(12 lines)  AV=2.00  SV=5.19  BV=9.26 */
    {193,1152,1032, 0, 0, 0},  //TV = 12.34(13 lines)  AV=2.00  SV=5.18  BV=9.16
    {208,1152,1032, 0, 0, 0},  //TV = 12.23(14 lines)  AV=2.00  SV=5.18  BV=9.05
    {223,1152,1032, 0, 0, 0},  //TV = 12.13(15 lines)  AV=2.00  SV=5.18  BV=8.95
    {238,1152,1032, 0, 0, 0},  //TV = 12.04(16 lines)  AV=2.00  SV=5.18  BV=8.86
    {253,1152,1040, 0, 0, 0},  //TV = 11.95(17 lines)  AV=2.00  SV=5.19  BV=8.76
    {268,1152,1056, 0, 0, 0},  //TV = 11.87(18 lines)  AV=2.00  SV=5.21  BV=8.65
    {297,1152,1024, 0, 0, 0},  //TV = 11.72(20 lines)  AV=2.00  SV=5.17  BV=8.55
    {312,1152,1040, 0, 0, 0},  //TV = 11.65(21 lines)  AV=2.00  SV=5.19  BV=8.45
    {342,1152,1024, 0, 0, 0},  //TV = 11.51(23 lines)  AV=2.00  SV=5.17  BV=8.34
    {371,1136,1024, 0, 0, 0},  //TV = 11.40(25 lines)  AV=2.00  SV=5.15  BV=8.25
    {386,1152,1032, 0, 0, 0},  //TV = 11.34(26 lines)  AV=2.00  SV=5.18  BV=8.16
    {416,1152,1032, 0, 0, 0},  //TV = 11.23(28 lines)  AV=2.00  SV=5.18  BV=8.05
    {446,1152,1032, 0, 0, 0},  //TV = 11.13(30 lines)  AV=2.00  SV=5.18  BV=7.95
    {475,1152,1032, 0, 0, 0},  //TV = 11.04(32 lines)  AV=2.00  SV=5.18  BV=7.86
    {520,1136,1024, 0, 0, 0},  //TV = 10.91(35 lines)  AV=2.00  SV=5.15  BV=7.76
    {564,1136,1024, 0, 0, 0},  //TV = 10.79(38 lines)  AV=2.00  SV=5.15  BV=7.64
    {594,1152,1024, 0, 0, 0},  //TV = 10.72(40 lines)  AV=2.00  SV=5.17  BV=7.55
    {638,1152,1024, 0, 0, 0},  //TV = 10.61(43 lines)  AV=2.00  SV=5.17  BV=7.44
    {683,1152,1024, 0, 0, 0},  //TV = 10.52(46 lines)  AV=2.00  SV=5.17  BV=7.35
    {727,1152,1024, 0, 0, 0},  //TV = 10.43(49 lines)  AV=2.00  SV=5.17  BV=7.26
    {787,1152,1024, 0, 0, 0},  //TV = 10.31(53 lines)  AV=2.00  SV=5.17  BV=7.14
    {846,1136,1024, 0, 0, 0},  //TV = 10.21(57 lines)  AV=2.00  SV=5.15  BV=7.06
    {905,1136,1024, 0, 0, 0},  //TV = 10.11(61 lines)  AV=2.00  SV=5.15  BV=6.96
    {965,1152,1024, 0, 0, 0},  //TV = 10.02(65 lines)  AV=2.00  SV=5.17  BV=6.85
    {1039,1152,1024, 0, 0, 0},  //TV = 9.91(70 lines)  AV=2.00  SV=5.17  BV=6.74
    {1128,1136,1024, 0, 0, 0},  //TV = 9.79(76 lines)  AV=2.00  SV=5.15  BV=6.64
    {1202,1136,1024, 0, 0, 0},  //TV = 9.70(81 lines)  AV=2.00  SV=5.15  BV=6.55
    {1291,1136,1024, 0, 0, 0},  //TV = 9.60(87 lines)  AV=2.00  SV=5.15  BV=6.45
    {1380,1136,1024, 0, 0, 0},  //TV = 9.50(93 lines)  AV=2.00  SV=5.15  BV=6.35
    {1484,1136,1024, 0, 0, 0},  //TV = 9.40(100 lines)  AV=2.00  SV=5.15  BV=6.25
    {1588,1136,1024, 0, 0, 0},  //TV = 9.30(107 lines)  AV=2.00  SV=5.15  BV=6.15
    {1692,1136,1024, 0, 0, 0},  //TV = 9.21(114 lines)  AV=2.00  SV=5.15  BV=6.06
    {1810,1136,1024, 0, 0, 0},  //TV = 9.11(122 lines)  AV=2.00  SV=5.15  BV=5.96
    {1959,1136,1024, 0, 0, 0},  //TV = 9.00(132 lines)  AV=2.00  SV=5.15  BV=5.85
    {2107,1136,1024, 0, 0, 0},  //TV = 8.89(142 lines)  AV=2.00  SV=5.15  BV=5.74
    {2255,1136,1024, 0, 0, 0},  //TV = 8.79(152 lines)  AV=2.00  SV=5.15  BV=5.64
    {2389,1136,1024, 0, 0, 0},  //TV = 8.71(161 lines)  AV=2.00  SV=5.15  BV=5.56
    {2582,1136,1024, 0, 0, 0},  //TV = 8.60(174 lines)  AV=2.00  SV=5.15  BV=5.45
    {2774,1136,1024, 0, 0, 0},  //TV = 8.49(187 lines)  AV=2.00  SV=5.15  BV=5.34
    {2967,1136,1024, 0, 0, 0},  //TV = 8.40(200 lines)  AV=2.00  SV=5.15  BV=5.25
    {3175,1136,1024, 0, 0, 0},  //TV = 8.30(214 lines)  AV=2.00  SV=5.15  BV=5.15
    {3397,1136,1024, 0, 0, 0},  //TV = 8.20(229 lines)  AV=2.00  SV=5.15  BV=5.05
    {3635,1136,1024, 0, 0, 0},  //TV = 8.10(245 lines)  AV=2.00  SV=5.15  BV=4.95
    {3932,1136,1024, 0, 0, 0},  //TV = 7.99(265 lines)  AV=2.00  SV=5.15  BV=4.84
    {4199,1136,1024, 0, 0, 0},  //TV = 7.90(283 lines)  AV=2.00  SV=5.15  BV=4.75
    {4495,1136,1024, 0, 0, 0},  //TV = 7.80(303 lines)  AV=2.00  SV=5.15  BV=4.65
    {4822,1136,1024, 0, 0, 0},  //TV = 7.70(325 lines)  AV=2.00  SV=5.15  BV=4.55
    {5163,1136,1024, 0, 0, 0},  //TV = 7.60(348 lines)  AV=2.00  SV=5.15  BV=4.45
    {5563,1136,1024, 0, 0, 0},  //TV = 7.49(375 lines)  AV=2.00  SV=5.15  BV=4.34
    {5949,1136,1024, 0, 0, 0},  //TV = 7.39(401 lines)  AV=2.00  SV=5.15  BV=4.24
    {6379,1136,1024, 0, 0, 0},  //TV = 7.29(430 lines)  AV=2.00  SV=5.15  BV=4.14
    {6824,1136,1024, 0, 0, 0},  //TV = 7.20(460 lines)  AV=2.00  SV=5.15  BV=4.05
    {7314,1136,1024, 0, 0, 0},  //TV = 7.10(493 lines)  AV=2.00  SV=5.15  BV=3.95
    {7833,1136,1024, 0, 0, 0},  //TV = 7.00(528 lines)  AV=2.00  SV=5.15  BV=3.85
    {8337,1136,1024, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.15  BV=3.76
    {8337,1152,1088, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.26  BV=3.65
    {8337,1280,1048, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.36  BV=3.55
    {8337,1408,1024, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.46  BV=3.45
    {8337,1408,1104, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.57  BV=3.34
    {8337,1536,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.65  BV=3.26
    {8337,1664,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.77  BV=3.14
    {8337,1792,1056, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.85  BV=3.05
    {8337,1920,1064, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.96  BV=2.94
    {8337,2048,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=6.07  BV=2.84
    {16659,1136,1024, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.15  BV=2.76
    {16659,1152,1088, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.26  BV=2.65
    {16659,1280,1056, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.37  BV=2.54
    {16659,1408,1024, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.46  BV=2.45
    {16659,1408,1096, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.56  BV=2.35
    {16659,1536,1080, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.66  BV=2.25
    {24996,1152,1024, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.17  BV=2.15
    {24996,1152,1096, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.27  BV=2.05
    {24996,1280,1056, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.37  BV=1.96
    {24996,1408,1032, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.47  BV=1.85
    {33288,1136,1024, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=5.15  BV=1.76
    {33288,1152,1088, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=5.26  BV=1.65
    {33288,1280,1056, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=5.37  BV=1.54
    {33288,1408,1024, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=5.46  BV=1.45
    {33288,1408,1104, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=5.57  BV=1.34
    {33288,1536,1080, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=5.66  BV=1.25
    {33288,1664,1072, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=5.77  BV=1.14
    {33288,1792,1064, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=5.86  BV=1.05
    {33288,1920,1056, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=5.95  BV=0.96
    {33288,2048,1072, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=6.07  BV=0.84
    {33288,2176,1080, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=6.16  BV=0.74
    {33288,2432,1040, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=6.27  BV=0.64
    {33288,2560,1056, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=6.37  BV=0.54
    {33288,2816,1024, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=6.46  BV=0.45
    {33288,2944,1048, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=6.56  BV=0.35
    {33288,3200,1040, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=6.67  BV=0.24
    {33288,3456,1032, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=6.77  BV=0.14
    {33288,3712,1024, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=6.86  BV=0.05
    {33288,3968,1024, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=6.95  BV=-0.05
    {33288,4224,1032, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=7.06  BV=-0.15
    {33288,4480,1040, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=7.15  BV=-0.24
    {33288,4864,1032, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=7.26  BV=-0.35
    {33288,5248,1024, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=7.36  BV=-0.45
    {33288,5632,1024, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=7.46  BV=-0.55
    {33288,6016,1024, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=7.55  BV=-0.65
    {33288,6400,1032, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=7.66  BV=-0.75
    {33288,6912,1024, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=7.75  BV=-0.85
    {33288,7424,1024, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=7.86  BV=-0.95
    {33288,7936,1032, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=7.97  BV=-1.06
    {33288,8576,1024, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=8.07  BV=-1.16
    {33288,9088,1032, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=8.16  BV=-1.25
    {33288,9856,1024, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=8.27  BV=-1.36
    {33288,10496,1024, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=8.36  BV=-1.45
    {33288,11264,1024, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=8.46  BV=-1.55
    {33288,12032,1024, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=8.55  BV=-1.65
    {33288,12928,1024, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=8.66  BV=-1.75
    {33288,13824,1024, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=8.75  BV=-1.85
    {33288,14848,1024, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=8.86  BV=-1.95
    {33288,15872,1024, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=8.95  BV=-2.05
    {33288,16384,1024, 0, 0, 0},  /* TV = 4.91(2244 lines)  AV=2.00  SV=9.00  BV=-2.09 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
},
};

static strEvPline sVideo1PLineTable_50Hz =
{
{
    {30,1152,1104, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.28  BV=11.75
    {30,1280,1056, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.37  BV=11.66
    {30,1408,1040, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.48  BV=11.54
    {30,1536,1032, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.60  BV=11.43
    {30,1664,1024, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.70  BV=11.32
    {45,1152,1048, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.20  BV=11.24
    {45,1152,1120, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.30  BV=11.14
    {45,1280,1088, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.41  BV=11.03
    {45,1408,1040, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.48  BV=10.96
    {60,1152,1056, 0, 0, 0},  /* TV = 14.02(4 lines)  AV=2.00  SV=5.21  BV=10.81 */
    {60,1152,1128, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.31  BV=10.72
    {60,1280,1072, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.39  BV=10.64
    {75,1152,1032, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.18  BV=10.52
    {75,1152,1088, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.26  BV=10.45
    {75,1280,1056, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.37  BV=10.34
    {90,1152,1040, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.19  BV=10.25
    {90,1280,1024, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.32  BV=10.12
    {104,1152,1040, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.19  BV=10.04
    {104,1152,1104, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.28  BV=9.95
    {119,1152,1032, 0, 0, 0},  //TV = 13.04(8 lines)  AV=2.00  SV=5.18  BV=9.86
    {134,1136,1024, 0, 0, 0},  /* TV = 12.87(9 lines)  AV=2.00  SV=5.15  BV=9.72 */
    {134,1152,1064, 0, 0, 0},  //TV = 12.87(9 lines)  AV=2.00  SV=5.23  BV=9.64
    {149,1152,1032, 0, 0, 0},  //TV = 12.71(10 lines)  AV=2.00  SV=5.18  BV=9.53
    {164,1136,1024, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.15  BV=9.42
    {164,1152,1088, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.26  BV=9.32
    {179,1152,1040, 0, 0, 0},  /* TV = 12.45(12 lines)  AV=2.00  SV=5.19  BV=9.26 */
    {193,1152,1032, 0, 0, 0},  //TV = 12.34(13 lines)  AV=2.00  SV=5.18  BV=9.16
    {208,1152,1032, 0, 0, 0},  //TV = 12.23(14 lines)  AV=2.00  SV=5.18  BV=9.05
    {223,1152,1032, 0, 0, 0},  //TV = 12.13(15 lines)  AV=2.00  SV=5.18  BV=8.95
    {238,1152,1032, 0, 0, 0},  //TV = 12.04(16 lines)  AV=2.00  SV=5.18  BV=8.86
    {253,1152,1040, 0, 0, 0},  //TV = 11.95(17 lines)  AV=2.00  SV=5.19  BV=8.76
    {268,1152,1056, 0, 0, 0},  //TV = 11.87(18 lines)  AV=2.00  SV=5.21  BV=8.65
    {297,1152,1024, 0, 0, 0},  //TV = 11.72(20 lines)  AV=2.00  SV=5.17  BV=8.55
    {312,1152,1040, 0, 0, 0},  //TV = 11.65(21 lines)  AV=2.00  SV=5.19  BV=8.45
    {342,1152,1024, 0, 0, 0},  //TV = 11.51(23 lines)  AV=2.00  SV=5.17  BV=8.34
    {371,1136,1024, 0, 0, 0},  //TV = 11.40(25 lines)  AV=2.00  SV=5.15  BV=8.25
    {386,1152,1032, 0, 0, 0},  //TV = 11.34(26 lines)  AV=2.00  SV=5.18  BV=8.16
    {416,1152,1032, 0, 0, 0},  //TV = 11.23(28 lines)  AV=2.00  SV=5.18  BV=8.05
    {446,1152,1032, 0, 0, 0},  //TV = 11.13(30 lines)  AV=2.00  SV=5.18  BV=7.95
    {475,1152,1032, 0, 0, 0},  //TV = 11.04(32 lines)  AV=2.00  SV=5.18  BV=7.86
    {520,1136,1024, 0, 0, 0},  //TV = 10.91(35 lines)  AV=2.00  SV=5.15  BV=7.76
    {564,1136,1024, 0, 0, 0},  //TV = 10.79(38 lines)  AV=2.00  SV=5.15  BV=7.64
    {594,1152,1024, 0, 0, 0},  //TV = 10.72(40 lines)  AV=2.00  SV=5.17  BV=7.55
    {638,1152,1024, 0, 0, 0},  //TV = 10.61(43 lines)  AV=2.00  SV=5.17  BV=7.44
    {683,1152,1024, 0, 0, 0},  //TV = 10.52(46 lines)  AV=2.00  SV=5.17  BV=7.35
    {727,1152,1024, 0, 0, 0},  //TV = 10.43(49 lines)  AV=2.00  SV=5.17  BV=7.26
    {787,1152,1024, 0, 0, 0},  //TV = 10.31(53 lines)  AV=2.00  SV=5.17  BV=7.14
    {846,1136,1024, 0, 0, 0},  //TV = 10.21(57 lines)  AV=2.00  SV=5.15  BV=7.06
    {905,1136,1024, 0, 0, 0},  //TV = 10.11(61 lines)  AV=2.00  SV=5.15  BV=6.96
    {965,1152,1024, 0, 0, 0},  //TV = 10.02(65 lines)  AV=2.00  SV=5.17  BV=6.85
    {1039,1152,1024, 0, 0, 0},  //TV = 9.91(70 lines)  AV=2.00  SV=5.17  BV=6.74
    {1128,1136,1024, 0, 0, 0},  //TV = 9.79(76 lines)  AV=2.00  SV=5.15  BV=6.64
    {1202,1136,1024, 0, 0, 0},  //TV = 9.70(81 lines)  AV=2.00  SV=5.15  BV=6.55
    {1291,1136,1024, 0, 0, 0},  //TV = 9.60(87 lines)  AV=2.00  SV=5.15  BV=6.45
    {1380,1136,1024, 0, 0, 0},  //TV = 9.50(93 lines)  AV=2.00  SV=5.15  BV=6.35
    {1484,1136,1024, 0, 0, 0},  //TV = 9.40(100 lines)  AV=2.00  SV=5.15  BV=6.25
    {1588,1136,1024, 0, 0, 0},  //TV = 9.30(107 lines)  AV=2.00  SV=5.15  BV=6.15
    {1692,1136,1024, 0, 0, 0},  //TV = 9.21(114 lines)  AV=2.00  SV=5.15  BV=6.06
    {1810,1136,1024, 0, 0, 0},  //TV = 9.11(122 lines)  AV=2.00  SV=5.15  BV=5.96
    {1959,1136,1024, 0, 0, 0},  //TV = 9.00(132 lines)  AV=2.00  SV=5.15  BV=5.85
    {2107,1136,1024, 0, 0, 0},  //TV = 8.89(142 lines)  AV=2.00  SV=5.15  BV=5.74
    {2255,1136,1024, 0, 0, 0},  //TV = 8.79(152 lines)  AV=2.00  SV=5.15  BV=5.64
    {2389,1136,1024, 0, 0, 0},  //TV = 8.71(161 lines)  AV=2.00  SV=5.15  BV=5.56
    {2582,1136,1024, 0, 0, 0},  //TV = 8.60(174 lines)  AV=2.00  SV=5.15  BV=5.45
    {2774,1136,1024, 0, 0, 0},  //TV = 8.49(187 lines)  AV=2.00  SV=5.15  BV=5.34
    {2967,1136,1024, 0, 0, 0},  //TV = 8.40(200 lines)  AV=2.00  SV=5.15  BV=5.25
    {3175,1136,1024, 0, 0, 0},  //TV = 8.30(214 lines)  AV=2.00  SV=5.15  BV=5.15
    {3397,1136,1024, 0, 0, 0},  //TV = 8.20(229 lines)  AV=2.00  SV=5.15  BV=5.05
    {3635,1136,1024, 0, 0, 0},  //TV = 8.10(245 lines)  AV=2.00  SV=5.15  BV=4.95
    {3932,1136,1024, 0, 0, 0},  //TV = 7.99(265 lines)  AV=2.00  SV=5.15  BV=4.84
    {4199,1136,1024, 0, 0, 0},  //TV = 7.90(283 lines)  AV=2.00  SV=5.15  BV=4.75
    {4495,1136,1024, 0, 0, 0},  //TV = 7.80(303 lines)  AV=2.00  SV=5.15  BV=4.65
    {4822,1136,1024, 0, 0, 0},  //TV = 7.70(325 lines)  AV=2.00  SV=5.15  BV=4.55
    {5163,1136,1024, 0, 0, 0},  //TV = 7.60(348 lines)  AV=2.00  SV=5.15  BV=4.45
    {5563,1136,1024, 0, 0, 0},  //TV = 7.49(375 lines)  AV=2.00  SV=5.15  BV=4.34
    {5949,1136,1024, 0, 0, 0},  //TV = 7.39(401 lines)  AV=2.00  SV=5.15  BV=4.24
    {6379,1136,1024, 0, 0, 0},  //TV = 7.29(430 lines)  AV=2.00  SV=5.15  BV=4.14
    {6824,1136,1024, 0, 0, 0},  //TV = 7.20(460 lines)  AV=2.00  SV=5.15  BV=4.05
    {7314,1136,1024, 0, 0, 0},  //TV = 7.10(493 lines)  AV=2.00  SV=5.15  BV=3.95
    {7833,1136,1024, 0, 0, 0},  //TV = 7.00(528 lines)  AV=2.00  SV=5.15  BV=3.85
    {8397,1136,1024, 0, 0, 0},  //TV = 6.90(566 lines)  AV=2.00  SV=5.15  BV=3.75
    {8990,1136,1024, 0, 0, 0},  //TV = 6.80(606 lines)  AV=2.00  SV=5.15  BV=3.65
    {9628,1136,1024, 0, 0, 0},  //TV = 6.70(649 lines)  AV=2.00  SV=5.15  BV=3.55
    {9999,1152,1040, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.19  BV=3.45
    {9999,1152,1112, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.29  BV=3.36
    {9999,1280,1080, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.40  BV=3.25
    {9999,1408,1056, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.50  BV=3.14
    {9999,1536,1032, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.60  BV=3.05
    {9999,1664,1024, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.70  BV=2.94
    {9999,1664,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.79  BV=2.86
    {9999,1792,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.89  BV=2.75
    {9999,1920,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.99  BV=2.65
    {9999,2176,1032, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=6.10  BV=2.55
    {19997,1152,1040, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.19  BV=2.45
    {19997,1152,1112, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.29  BV=2.36
    {19997,1280,1080, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.40  BV=2.25
    {19997,1408,1048, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.49  BV=2.15
    {19997,1536,1032, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.60  BV=2.05
    {19997,1664,1024, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.70  BV=1.94
    {29995,1152,1048, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.20  BV=1.86
    {29995,1152,1128, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.31  BV=1.75
    {29995,1280,1088, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.41  BV=1.65
    {29995,1408,1064, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.51  BV=1.54
    {29995,1536,1040, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.61  BV=1.45
    {29995,1664,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.71  BV=1.35
    {29995,1792,1024, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.81  BV=1.25
    {29995,1920,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.92  BV=1.14
    {29995,2048,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.01  BV=1.05
    {29995,2176,1040, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.11  BV=0.95
    {29995,2304,1056, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.21  BV=0.84
    {29995,2432,1064, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.30  BV=0.76
    {29995,2688,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.40  BV=0.66
    {29995,2816,1064, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.51  BV=0.54
    {29995,3072,1040, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.61  BV=0.45
    {29995,3328,1024, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.70  BV=0.36
    {29995,3584,1024, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.81  BV=0.25
    {29995,3840,1024, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.91  BV=0.15
    {29995,4096,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=7.01  BV=0.05
    {29995,4352,1040, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=7.11  BV=-0.05
    {29995,4736,1024, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=7.21  BV=-0.15
    {29995,4992,1040, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=7.31  BV=-0.25
    {29995,5376,1040, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=7.41  BV=-0.36
    {29995,5760,1040, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=7.51  BV=-0.46
    {29995,6272,1024, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=7.61  BV=-0.56
    {29995,6656,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=7.71  BV=-0.65
    {29995,7168,1024, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=7.81  BV=-0.75
    {29995,7680,1024, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=7.91  BV=-0.85
    {29995,8192,1024, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=8.00  BV=-0.94
    {29995,8832,1024, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=8.11  BV=-1.05
    {29995,9472,1024, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=8.21  BV=-1.15
    {29995,10112,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=8.32  BV=-1.26
    {29995,10880,1024, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=8.41  BV=-1.35
    {29995,11648,1024, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=8.51  BV=-1.45
    {29995,12544,1024, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=8.61  BV=-1.56
    {29995,13440,1024, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=8.71  BV=-1.66
    {29995,14336,1024, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=8.81  BV=-1.75
    {29995,15360,1024, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=8.91  BV=-1.85
    {29995,16384,1024, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=9.00  BV=-1.94
    {29995,16384,1024, 0, 0, 0},  /* TV = 5.06(2022 lines)  AV=2.00  SV=9.00  BV=-1.94 */
    {29995,16384,1024, 0, 0, 0},  /* TV = 5.06(2022 lines)  AV=2.00  SV=9.00  BV=-1.94 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
},
};

static strAETable g_AE_Video1AutoTable =
{
    AETABLE_VIDEO1_AUTO, //eAETableID
    140, //u4TotalIndex
    -20, //i4StrobeTrigerBV
    118, //i4MaxBV
    -21, //i4MinBV
    90, //i4EffectiveMaxBV
    0, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_AUTO, //ISO
    sVideo1PLineTable_60Hz,
    sVideo1PLineTable_50Hz,
    NULL,
};
static strEvPline sVideo2PLineTable_60Hz =
{
{
    {30,1152,1104, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.28  BV=11.75
    {30,1280,1056, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.37  BV=11.66
    {30,1408,1040, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.48  BV=11.54
    {30,1536,1032, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.60  BV=11.43
    {30,1664,1024, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.70  BV=11.32
    {45,1152,1048, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.20  BV=11.24
    {45,1152,1120, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.30  BV=11.14
    {45,1280,1088, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.41  BV=11.03
    {45,1408,1040, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.48  BV=10.96
    {60,1152,1056, 0, 0, 0},  /* TV = 14.02(4 lines)  AV=2.00  SV=5.21  BV=10.81 */
    {60,1152,1128, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.31  BV=10.72
    {60,1280,1072, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.39  BV=10.64
    {75,1152,1032, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.18  BV=10.52
    {75,1152,1088, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.26  BV=10.45
    {75,1280,1056, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.37  BV=10.34
    {90,1152,1040, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.19  BV=10.25
    {90,1280,1024, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.32  BV=10.12
    {104,1152,1040, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.19  BV=10.04
    {104,1152,1104, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.28  BV=9.95
    {119,1152,1032, 0, 0, 0},  //TV = 13.04(8 lines)  AV=2.00  SV=5.18  BV=9.86
    {134,1136,1024, 0, 0, 0},  /* TV = 12.87(9 lines)  AV=2.00  SV=5.15  BV=9.72 */
    {134,1152,1064, 0, 0, 0},  //TV = 12.87(9 lines)  AV=2.00  SV=5.23  BV=9.64
    {149,1152,1032, 0, 0, 0},  //TV = 12.71(10 lines)  AV=2.00  SV=5.18  BV=9.53
    {164,1136,1024, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.15  BV=9.42
    {164,1152,1088, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.26  BV=9.32
    {179,1152,1040, 0, 0, 0},  /* TV = 12.45(12 lines)  AV=2.00  SV=5.19  BV=9.26 */
    {193,1152,1032, 0, 0, 0},  //TV = 12.34(13 lines)  AV=2.00  SV=5.18  BV=9.16
    {208,1152,1032, 0, 0, 0},  //TV = 12.23(14 lines)  AV=2.00  SV=5.18  BV=9.05
    {223,1152,1032, 0, 0, 0},  //TV = 12.13(15 lines)  AV=2.00  SV=5.18  BV=8.95
    {238,1152,1032, 0, 0, 0},  //TV = 12.04(16 lines)  AV=2.00  SV=5.18  BV=8.86
    {253,1152,1040, 0, 0, 0},  //TV = 11.95(17 lines)  AV=2.00  SV=5.19  BV=8.76
    {268,1152,1056, 0, 0, 0},  //TV = 11.87(18 lines)  AV=2.00  SV=5.21  BV=8.65
    {297,1152,1024, 0, 0, 0},  //TV = 11.72(20 lines)  AV=2.00  SV=5.17  BV=8.55
    {312,1152,1040, 0, 0, 0},  //TV = 11.65(21 lines)  AV=2.00  SV=5.19  BV=8.45
    {342,1152,1024, 0, 0, 0},  //TV = 11.51(23 lines)  AV=2.00  SV=5.17  BV=8.34
    {371,1136,1024, 0, 0, 0},  //TV = 11.40(25 lines)  AV=2.00  SV=5.15  BV=8.25
    {386,1152,1032, 0, 0, 0},  //TV = 11.34(26 lines)  AV=2.00  SV=5.18  BV=8.16
    {416,1152,1032, 0, 0, 0},  //TV = 11.23(28 lines)  AV=2.00  SV=5.18  BV=8.05
    {446,1152,1032, 0, 0, 0},  //TV = 11.13(30 lines)  AV=2.00  SV=5.18  BV=7.95
    {475,1152,1032, 0, 0, 0},  //TV = 11.04(32 lines)  AV=2.00  SV=5.18  BV=7.86
    {520,1136,1024, 0, 0, 0},  //TV = 10.91(35 lines)  AV=2.00  SV=5.15  BV=7.76
    {564,1136,1024, 0, 0, 0},  //TV = 10.79(38 lines)  AV=2.00  SV=5.15  BV=7.64
    {594,1152,1024, 0, 0, 0},  //TV = 10.72(40 lines)  AV=2.00  SV=5.17  BV=7.55
    {638,1152,1024, 0, 0, 0},  //TV = 10.61(43 lines)  AV=2.00  SV=5.17  BV=7.44
    {683,1152,1024, 0, 0, 0},  //TV = 10.52(46 lines)  AV=2.00  SV=5.17  BV=7.35
    {727,1152,1024, 0, 0, 0},  //TV = 10.43(49 lines)  AV=2.00  SV=5.17  BV=7.26
    {787,1152,1024, 0, 0, 0},  //TV = 10.31(53 lines)  AV=2.00  SV=5.17  BV=7.14
    {846,1136,1024, 0, 0, 0},  //TV = 10.21(57 lines)  AV=2.00  SV=5.15  BV=7.06
    {905,1136,1024, 0, 0, 0},  //TV = 10.11(61 lines)  AV=2.00  SV=5.15  BV=6.96
    {965,1152,1024, 0, 0, 0},  //TV = 10.02(65 lines)  AV=2.00  SV=5.17  BV=6.85
    {1039,1152,1024, 0, 0, 0},  //TV = 9.91(70 lines)  AV=2.00  SV=5.17  BV=6.74
    {1128,1136,1024, 0, 0, 0},  //TV = 9.79(76 lines)  AV=2.00  SV=5.15  BV=6.64
    {1202,1136,1024, 0, 0, 0},  //TV = 9.70(81 lines)  AV=2.00  SV=5.15  BV=6.55
    {1291,1136,1024, 0, 0, 0},  //TV = 9.60(87 lines)  AV=2.00  SV=5.15  BV=6.45
    {1380,1136,1024, 0, 0, 0},  //TV = 9.50(93 lines)  AV=2.00  SV=5.15  BV=6.35
    {1484,1136,1024, 0, 0, 0},  //TV = 9.40(100 lines)  AV=2.00  SV=5.15  BV=6.25
    {1588,1136,1024, 0, 0, 0},  //TV = 9.30(107 lines)  AV=2.00  SV=5.15  BV=6.15
    {1692,1136,1024, 0, 0, 0},  //TV = 9.21(114 lines)  AV=2.00  SV=5.15  BV=6.06
    {1810,1136,1024, 0, 0, 0},  //TV = 9.11(122 lines)  AV=2.00  SV=5.15  BV=5.96
    {1959,1136,1024, 0, 0, 0},  //TV = 9.00(132 lines)  AV=2.00  SV=5.15  BV=5.85
    {2107,1136,1024, 0, 0, 0},  //TV = 8.89(142 lines)  AV=2.00  SV=5.15  BV=5.74
    {2255,1136,1024, 0, 0, 0},  //TV = 8.79(152 lines)  AV=2.00  SV=5.15  BV=5.64
    {2389,1136,1024, 0, 0, 0},  //TV = 8.71(161 lines)  AV=2.00  SV=5.15  BV=5.56
    {2582,1136,1024, 0, 0, 0},  //TV = 8.60(174 lines)  AV=2.00  SV=5.15  BV=5.45
    {2774,1136,1024, 0, 0, 0},  //TV = 8.49(187 lines)  AV=2.00  SV=5.15  BV=5.34
    {2967,1136,1024, 0, 0, 0},  //TV = 8.40(200 lines)  AV=2.00  SV=5.15  BV=5.25
    {3175,1136,1024, 0, 0, 0},  //TV = 8.30(214 lines)  AV=2.00  SV=5.15  BV=5.15
    {3397,1136,1024, 0, 0, 0},  //TV = 8.20(229 lines)  AV=2.00  SV=5.15  BV=5.05
    {3635,1136,1024, 0, 0, 0},  //TV = 8.10(245 lines)  AV=2.00  SV=5.15  BV=4.95
    {3932,1136,1024, 0, 0, 0},  //TV = 7.99(265 lines)  AV=2.00  SV=5.15  BV=4.84
    {4199,1136,1024, 0, 0, 0},  //TV = 7.90(283 lines)  AV=2.00  SV=5.15  BV=4.75
    {4495,1136,1024, 0, 0, 0},  //TV = 7.80(303 lines)  AV=2.00  SV=5.15  BV=4.65
    {4822,1136,1024, 0, 0, 0},  //TV = 7.70(325 lines)  AV=2.00  SV=5.15  BV=4.55
    {5163,1136,1024, 0, 0, 0},  //TV = 7.60(348 lines)  AV=2.00  SV=5.15  BV=4.45
    {5563,1136,1024, 0, 0, 0},  //TV = 7.49(375 lines)  AV=2.00  SV=5.15  BV=4.34
    {5949,1136,1024, 0, 0, 0},  //TV = 7.39(401 lines)  AV=2.00  SV=5.15  BV=4.24
    {6379,1136,1024, 0, 0, 0},  //TV = 7.29(430 lines)  AV=2.00  SV=5.15  BV=4.14
    {6824,1136,1024, 0, 0, 0},  //TV = 7.20(460 lines)  AV=2.00  SV=5.15  BV=4.05
    {7314,1136,1024, 0, 0, 0},  //TV = 7.10(493 lines)  AV=2.00  SV=5.15  BV=3.95
    {7833,1136,1024, 0, 0, 0},  //TV = 7.00(528 lines)  AV=2.00  SV=5.15  BV=3.85
    {8337,1136,1024, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.15  BV=3.76
    {8337,1152,1088, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.26  BV=3.65
    {8337,1280,1048, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.36  BV=3.55
    {8337,1408,1024, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.46  BV=3.45
    {8337,1408,1104, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.57  BV=3.34
    {8337,1536,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.65  BV=3.26
    {8337,1664,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.77  BV=3.14
    {8337,1792,1056, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.85  BV=3.05
    {8337,1920,1064, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.96  BV=2.94
    {8337,2048,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=6.07  BV=2.84
    {16659,1136,1024, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.15  BV=2.76
    {16659,1152,1088, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.26  BV=2.65
    {16659,1280,1056, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.37  BV=2.54
    {16659,1408,1024, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.46  BV=2.45
    {16659,1408,1096, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.56  BV=2.35
    {16659,1536,1080, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.66  BV=2.25
    {24996,1152,1024, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.17  BV=2.15
    {24996,1152,1096, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.27  BV=2.05
    {24996,1280,1056, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.37  BV=1.96
    {24996,1408,1032, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.47  BV=1.85
    {33288,1136,1024, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=5.15  BV=1.76
    {33288,1152,1088, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=5.26  BV=1.65
    {33288,1280,1056, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=5.37  BV=1.54
    {33288,1408,1024, 0, 0, 0},  //TV = 4.91(2244 lines)  AV=2.00  SV=5.46  BV=1.45
    {41669,1152,1072, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=5.24  BV=1.35
    {41669,1280,1032, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=5.33  BV=1.25
    {49991,1152,1024, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.17  BV=1.15
    {49991,1152,1096, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.27  BV=1.05
    {49991,1280,1056, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.37  BV=0.96
    {49991,1408,1032, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.47  BV=0.85
    {49991,1408,1104, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.57  BV=0.75
    {49991,1536,1088, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.67  BV=0.65
    {49991,1664,1072, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.77  BV=0.56
    {49991,1792,1072, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.87  BV=0.45
    {49991,1920,1072, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.97  BV=0.35
    {49991,2048,1080, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=6.08  BV=0.25
    {49991,2304,1024, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=6.17  BV=0.15
    {49991,2432,1040, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=6.27  BV=0.05
    {49991,2560,1056, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=6.37  BV=-0.04
    {49991,2816,1032, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=6.47  BV=-0.15
    {49991,2944,1056, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=6.57  BV=-0.25
    {49991,3200,1040, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=6.67  BV=-0.34
    {49991,3456,1032, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=6.77  BV=-0.44
    {49991,3712,1032, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=6.87  BV=-0.55
    {49991,3968,1040, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=6.98  BV=-0.65
    {49991,4224,1048, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=7.08  BV=-0.76
    {49991,4608,1024, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=7.17  BV=-0.85
    {49991,4864,1040, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=7.27  BV=-0.95
    {49991,5248,1032, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=7.37  BV=-1.05
    {49991,5632,1032, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=7.47  BV=-1.15
    {49991,6016,1040, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=7.58  BV=-1.25
    {49991,6528,1024, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=7.67  BV=-1.35
    {49991,6912,1040, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=7.78  BV=-1.46
    {49991,7424,1032, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=7.87  BV=-1.55
    {49991,8064,1024, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=7.98  BV=-1.66
    {49991,8576,1024, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=8.07  BV=-1.74
    {49991,9216,1024, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=8.17  BV=-1.85
    {49991,9856,1024, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=8.27  BV=-1.94
    {49991,10496,1032, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=8.37  BV=-2.05
    {49991,11264,1032, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=8.47  BV=-2.15
    {49991,12160,1024, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=8.57  BV=-2.25
    {49991,13056,1024, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=8.67  BV=-2.35
    {49991,14080,1024, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=8.78  BV=-2.46
    {49991,14976,1024, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=8.87  BV=-2.55
    {49991,16128,1024, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=8.98  BV=-2.66
    {49991,16384,1024, 0, 0, 0},  /* TV = 4.32(3370 lines)  AV=2.00  SV=9.00  BV=-2.68 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
},
};

static strEvPline sVideo2PLineTable_50Hz =
{
{
    {30,1152,1104, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.28  BV=11.75
    {30,1280,1056, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.37  BV=11.66
    {30,1408,1040, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.48  BV=11.54
    {30,1536,1032, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.60  BV=11.43
    {30,1664,1024, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.70  BV=11.32
    {45,1152,1048, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.20  BV=11.24
    {45,1152,1120, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.30  BV=11.14
    {45,1280,1088, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.41  BV=11.03
    {45,1408,1040, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.48  BV=10.96
    {60,1152,1056, 0, 0, 0},  /* TV = 14.02(4 lines)  AV=2.00  SV=5.21  BV=10.81 */
    {60,1152,1128, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.31  BV=10.72
    {60,1280,1072, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.39  BV=10.64
    {75,1152,1032, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.18  BV=10.52
    {75,1152,1088, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.26  BV=10.45
    {75,1280,1056, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.37  BV=10.34
    {90,1152,1040, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.19  BV=10.25
    {90,1280,1024, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.32  BV=10.12
    {104,1152,1040, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.19  BV=10.04
    {104,1152,1104, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.28  BV=9.95
    {119,1152,1032, 0, 0, 0},  //TV = 13.04(8 lines)  AV=2.00  SV=5.18  BV=9.86
    {134,1136,1024, 0, 0, 0},  /* TV = 12.87(9 lines)  AV=2.00  SV=5.15  BV=9.72 */
    {134,1152,1064, 0, 0, 0},  //TV = 12.87(9 lines)  AV=2.00  SV=5.23  BV=9.64
    {149,1152,1032, 0, 0, 0},  //TV = 12.71(10 lines)  AV=2.00  SV=5.18  BV=9.53
    {164,1136,1024, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.15  BV=9.42
    {164,1152,1088, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.26  BV=9.32
    {179,1152,1040, 0, 0, 0},  /* TV = 12.45(12 lines)  AV=2.00  SV=5.19  BV=9.26 */
    {193,1152,1032, 0, 0, 0},  //TV = 12.34(13 lines)  AV=2.00  SV=5.18  BV=9.16
    {208,1152,1032, 0, 0, 0},  //TV = 12.23(14 lines)  AV=2.00  SV=5.18  BV=9.05
    {223,1152,1032, 0, 0, 0},  //TV = 12.13(15 lines)  AV=2.00  SV=5.18  BV=8.95
    {238,1152,1032, 0, 0, 0},  //TV = 12.04(16 lines)  AV=2.00  SV=5.18  BV=8.86
    {253,1152,1040, 0, 0, 0},  //TV = 11.95(17 lines)  AV=2.00  SV=5.19  BV=8.76
    {268,1152,1056, 0, 0, 0},  //TV = 11.87(18 lines)  AV=2.00  SV=5.21  BV=8.65
    {297,1152,1024, 0, 0, 0},  //TV = 11.72(20 lines)  AV=2.00  SV=5.17  BV=8.55
    {312,1152,1040, 0, 0, 0},  //TV = 11.65(21 lines)  AV=2.00  SV=5.19  BV=8.45
    {342,1152,1024, 0, 0, 0},  //TV = 11.51(23 lines)  AV=2.00  SV=5.17  BV=8.34
    {371,1136,1024, 0, 0, 0},  //TV = 11.40(25 lines)  AV=2.00  SV=5.15  BV=8.25
    {386,1152,1032, 0, 0, 0},  //TV = 11.34(26 lines)  AV=2.00  SV=5.18  BV=8.16
    {416,1152,1032, 0, 0, 0},  //TV = 11.23(28 lines)  AV=2.00  SV=5.18  BV=8.05
    {446,1152,1032, 0, 0, 0},  //TV = 11.13(30 lines)  AV=2.00  SV=5.18  BV=7.95
    {475,1152,1032, 0, 0, 0},  //TV = 11.04(32 lines)  AV=2.00  SV=5.18  BV=7.86
    {520,1136,1024, 0, 0, 0},  //TV = 10.91(35 lines)  AV=2.00  SV=5.15  BV=7.76
    {564,1136,1024, 0, 0, 0},  //TV = 10.79(38 lines)  AV=2.00  SV=5.15  BV=7.64
    {594,1152,1024, 0, 0, 0},  //TV = 10.72(40 lines)  AV=2.00  SV=5.17  BV=7.55
    {638,1152,1024, 0, 0, 0},  //TV = 10.61(43 lines)  AV=2.00  SV=5.17  BV=7.44
    {683,1152,1024, 0, 0, 0},  //TV = 10.52(46 lines)  AV=2.00  SV=5.17  BV=7.35
    {727,1152,1024, 0, 0, 0},  //TV = 10.43(49 lines)  AV=2.00  SV=5.17  BV=7.26
    {787,1152,1024, 0, 0, 0},  //TV = 10.31(53 lines)  AV=2.00  SV=5.17  BV=7.14
    {846,1136,1024, 0, 0, 0},  //TV = 10.21(57 lines)  AV=2.00  SV=5.15  BV=7.06
    {905,1136,1024, 0, 0, 0},  //TV = 10.11(61 lines)  AV=2.00  SV=5.15  BV=6.96
    {965,1152,1024, 0, 0, 0},  //TV = 10.02(65 lines)  AV=2.00  SV=5.17  BV=6.85
    {1039,1152,1024, 0, 0, 0},  //TV = 9.91(70 lines)  AV=2.00  SV=5.17  BV=6.74
    {1128,1136,1024, 0, 0, 0},  //TV = 9.79(76 lines)  AV=2.00  SV=5.15  BV=6.64
    {1202,1136,1024, 0, 0, 0},  //TV = 9.70(81 lines)  AV=2.00  SV=5.15  BV=6.55
    {1291,1136,1024, 0, 0, 0},  //TV = 9.60(87 lines)  AV=2.00  SV=5.15  BV=6.45
    {1380,1136,1024, 0, 0, 0},  //TV = 9.50(93 lines)  AV=2.00  SV=5.15  BV=6.35
    {1484,1136,1024, 0, 0, 0},  //TV = 9.40(100 lines)  AV=2.00  SV=5.15  BV=6.25
    {1588,1136,1024, 0, 0, 0},  //TV = 9.30(107 lines)  AV=2.00  SV=5.15  BV=6.15
    {1692,1136,1024, 0, 0, 0},  //TV = 9.21(114 lines)  AV=2.00  SV=5.15  BV=6.06
    {1810,1136,1024, 0, 0, 0},  //TV = 9.11(122 lines)  AV=2.00  SV=5.15  BV=5.96
    {1959,1136,1024, 0, 0, 0},  //TV = 9.00(132 lines)  AV=2.00  SV=5.15  BV=5.85
    {2107,1136,1024, 0, 0, 0},  //TV = 8.89(142 lines)  AV=2.00  SV=5.15  BV=5.74
    {2255,1136,1024, 0, 0, 0},  //TV = 8.79(152 lines)  AV=2.00  SV=5.15  BV=5.64
    {2389,1136,1024, 0, 0, 0},  //TV = 8.71(161 lines)  AV=2.00  SV=5.15  BV=5.56
    {2582,1136,1024, 0, 0, 0},  //TV = 8.60(174 lines)  AV=2.00  SV=5.15  BV=5.45
    {2774,1136,1024, 0, 0, 0},  //TV = 8.49(187 lines)  AV=2.00  SV=5.15  BV=5.34
    {2967,1136,1024, 0, 0, 0},  //TV = 8.40(200 lines)  AV=2.00  SV=5.15  BV=5.25
    {3175,1136,1024, 0, 0, 0},  //TV = 8.30(214 lines)  AV=2.00  SV=5.15  BV=5.15
    {3397,1136,1024, 0, 0, 0},  //TV = 8.20(229 lines)  AV=2.00  SV=5.15  BV=5.05
    {3635,1136,1024, 0, 0, 0},  //TV = 8.10(245 lines)  AV=2.00  SV=5.15  BV=4.95
    {3932,1136,1024, 0, 0, 0},  //TV = 7.99(265 lines)  AV=2.00  SV=5.15  BV=4.84
    {4199,1136,1024, 0, 0, 0},  //TV = 7.90(283 lines)  AV=2.00  SV=5.15  BV=4.75
    {4495,1136,1024, 0, 0, 0},  //TV = 7.80(303 lines)  AV=2.00  SV=5.15  BV=4.65
    {4822,1136,1024, 0, 0, 0},  //TV = 7.70(325 lines)  AV=2.00  SV=5.15  BV=4.55
    {5163,1136,1024, 0, 0, 0},  //TV = 7.60(348 lines)  AV=2.00  SV=5.15  BV=4.45
    {5563,1136,1024, 0, 0, 0},  //TV = 7.49(375 lines)  AV=2.00  SV=5.15  BV=4.34
    {5949,1136,1024, 0, 0, 0},  //TV = 7.39(401 lines)  AV=2.00  SV=5.15  BV=4.24
    {6379,1136,1024, 0, 0, 0},  //TV = 7.29(430 lines)  AV=2.00  SV=5.15  BV=4.14
    {6824,1136,1024, 0, 0, 0},  //TV = 7.20(460 lines)  AV=2.00  SV=5.15  BV=4.05
    {7314,1136,1024, 0, 0, 0},  //TV = 7.10(493 lines)  AV=2.00  SV=5.15  BV=3.95
    {7833,1136,1024, 0, 0, 0},  //TV = 7.00(528 lines)  AV=2.00  SV=5.15  BV=3.85
    {8397,1136,1024, 0, 0, 0},  //TV = 6.90(566 lines)  AV=2.00  SV=5.15  BV=3.75
    {8990,1136,1024, 0, 0, 0},  //TV = 6.80(606 lines)  AV=2.00  SV=5.15  BV=3.65
    {9628,1136,1024, 0, 0, 0},  //TV = 6.70(649 lines)  AV=2.00  SV=5.15  BV=3.55
    {9999,1152,1040, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.19  BV=3.45
    {9999,1152,1112, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.29  BV=3.36
    {9999,1280,1080, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.40  BV=3.25
    {9999,1408,1056, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.50  BV=3.14
    {9999,1536,1032, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.60  BV=3.05
    {9999,1664,1024, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.70  BV=2.94
    {9999,1664,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.79  BV=2.86
    {9999,1792,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.89  BV=2.75
    {9999,1920,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.99  BV=2.65
    {9999,2176,1032, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=6.10  BV=2.55
    {19997,1152,1040, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.19  BV=2.45
    {19997,1152,1112, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.29  BV=2.36
    {19997,1280,1080, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.40  BV=2.25
    {19997,1408,1048, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.49  BV=2.15
    {19997,1536,1032, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.60  BV=2.05
    {19997,1664,1024, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.70  BV=1.94
    {29995,1152,1048, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.20  BV=1.86
    {29995,1152,1128, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.31  BV=1.75
    {29995,1280,1088, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.41  BV=1.65
    {29995,1408,1064, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.51  BV=1.54
    {39993,1152,1040, 0, 0, 0},  //TV = 4.64(2696 lines)  AV=2.00  SV=5.19  BV=1.45
    {39993,1152,1112, 0, 0, 0},  //TV = 4.64(2696 lines)  AV=2.00  SV=5.29  BV=1.36
    {39993,1280,1080, 0, 0, 0},  //TV = 4.64(2696 lines)  AV=2.00  SV=5.40  BV=1.25
    {50006,1152,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.17  BV=1.15
    {50006,1152,1096, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.27  BV=1.05
    {50006,1280,1056, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.37  BV=0.96
    {50006,1408,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.47  BV=0.85
    {50006,1408,1104, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.57  BV=0.75
    {50006,1536,1088, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.67  BV=0.65
    {50006,1664,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.77  BV=0.56
    {50006,1792,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.87  BV=0.45
    {50006,1920,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.97  BV=0.35
    {50006,2048,1080, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.08  BV=0.24
    {50006,2304,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.17  BV=0.15
    {50006,2432,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.27  BV=0.05
    {50006,2560,1056, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.37  BV=-0.04
    {50006,2816,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.47  BV=-0.15
    {50006,2944,1056, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.57  BV=-0.25
    {50006,3200,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.67  BV=-0.34
    {50006,3456,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.77  BV=-0.44
    {50006,3712,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.87  BV=-0.55
    {50006,3968,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.98  BV=-0.65
    {50006,4224,1048, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.08  BV=-0.76
    {50006,4608,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.17  BV=-0.85
    {50006,4864,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.27  BV=-0.95
    {50006,5248,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.37  BV=-1.05
    {50006,5632,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.48  BV=-1.16
    {50006,6016,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.58  BV=-1.26
    {50006,6528,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.67  BV=-1.35
    {50006,6912,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.78  BV=-1.46
    {50006,7424,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.87  BV=-1.55
    {50006,8064,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.98  BV=-1.66
    {50006,8576,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=8.07  BV=-1.74
    {50006,9216,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=8.17  BV=-1.85
    {50006,9856,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=8.27  BV=-1.95
    {50006,10496,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=8.37  BV=-2.05
    {50006,11264,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=8.47  BV=-2.15
    {50006,12160,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=8.57  BV=-2.25
    {50006,13056,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=8.67  BV=-2.35
    {50006,14080,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=8.78  BV=-2.46
    {50006,14976,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=8.87  BV=-2.55
    {50006,16128,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=8.98  BV=-2.66
    {50006,16384,1024, 0, 0, 0},  /* TV = 4.32(3371 lines)  AV=2.00  SV=9.00  BV=-2.68 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
},
};

static strAETable g_AE_Video2AutoTable =
{
    AETABLE_VIDEO2_AUTO, //eAETableID
    146, //u4TotalIndex
    -20, //i4StrobeTrigerBV
    118, //i4MaxBV
    -27, //i4MinBV
    90, //i4EffectiveMaxBV
    0, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_AUTO, //ISO
    sVideo2PLineTable_60Hz,
    sVideo2PLineTable_50Hz,
    NULL,
};
static strEvPline sCustom1PLineTable_60Hz =
{
{
    {30,1152,1104, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.28  BV=11.75
    {30,1280,1056, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.37  BV=11.66
    {30,1408,1040, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.48  BV=11.54
    {30,1536,1032, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.60  BV=11.43
    {30,1664,1024, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.70  BV=11.32
    {45,1152,1048, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.20  BV=11.24
    {45,1152,1120, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.30  BV=11.14
    {45,1280,1088, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.41  BV=11.03
    {45,1408,1040, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.48  BV=10.96
    {60,1152,1056, 0, 0, 0},  /* TV = 14.02(4 lines)  AV=2.00  SV=5.21  BV=10.81 */
    {60,1152,1128, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.31  BV=10.72
    {60,1280,1072, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.39  BV=10.64
    {75,1152,1032, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.18  BV=10.52
    {75,1152,1088, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.26  BV=10.45
    {75,1280,1056, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.37  BV=10.34
    {90,1152,1040, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.19  BV=10.25
    {90,1280,1024, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.32  BV=10.12
    {104,1152,1040, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.19  BV=10.04
    {104,1152,1104, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.28  BV=9.95
    {119,1152,1032, 0, 0, 0},  //TV = 13.04(8 lines)  AV=2.00  SV=5.18  BV=9.86
    {134,1136,1024, 0, 0, 0},  /* TV = 12.87(9 lines)  AV=2.00  SV=5.15  BV=9.72 */
    {134,1152,1064, 0, 0, 0},  //TV = 12.87(9 lines)  AV=2.00  SV=5.23  BV=9.64
    {149,1152,1032, 0, 0, 0},  //TV = 12.71(10 lines)  AV=2.00  SV=5.18  BV=9.53
    {164,1136,1024, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.15  BV=9.42
    {164,1152,1088, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.26  BV=9.32
    {179,1152,1040, 0, 0, 0},  /* TV = 12.45(12 lines)  AV=2.00  SV=5.19  BV=9.26 */
    {193,1152,1032, 0, 0, 0},  //TV = 12.34(13 lines)  AV=2.00  SV=5.18  BV=9.16
    {208,1152,1032, 0, 0, 0},  //TV = 12.23(14 lines)  AV=2.00  SV=5.18  BV=9.05
    {223,1152,1032, 0, 0, 0},  //TV = 12.13(15 lines)  AV=2.00  SV=5.18  BV=8.95
    {238,1152,1032, 0, 0, 0},  //TV = 12.04(16 lines)  AV=2.00  SV=5.18  BV=8.86
    {253,1152,1040, 0, 0, 0},  //TV = 11.95(17 lines)  AV=2.00  SV=5.19  BV=8.76
    {268,1152,1056, 0, 0, 0},  //TV = 11.87(18 lines)  AV=2.00  SV=5.21  BV=8.65
    {297,1152,1024, 0, 0, 0},  //TV = 11.72(20 lines)  AV=2.00  SV=5.17  BV=8.55
    {312,1152,1040, 0, 0, 0},  //TV = 11.65(21 lines)  AV=2.00  SV=5.19  BV=8.45
    {342,1152,1024, 0, 0, 0},  //TV = 11.51(23 lines)  AV=2.00  SV=5.17  BV=8.34
    {371,1136,1024, 0, 0, 0},  //TV = 11.40(25 lines)  AV=2.00  SV=5.15  BV=8.25
    {386,1152,1032, 0, 0, 0},  //TV = 11.34(26 lines)  AV=2.00  SV=5.18  BV=8.16
    {416,1152,1032, 0, 0, 0},  //TV = 11.23(28 lines)  AV=2.00  SV=5.18  BV=8.05
    {446,1152,1032, 0, 0, 0},  //TV = 11.13(30 lines)  AV=2.00  SV=5.18  BV=7.95
    {475,1152,1032, 0, 0, 0},  //TV = 11.04(32 lines)  AV=2.00  SV=5.18  BV=7.86
    {520,1136,1024, 0, 0, 0},  //TV = 10.91(35 lines)  AV=2.00  SV=5.15  BV=7.76
    {564,1136,1024, 0, 0, 0},  //TV = 10.79(38 lines)  AV=2.00  SV=5.15  BV=7.64
    {594,1152,1024, 0, 0, 0},  //TV = 10.72(40 lines)  AV=2.00  SV=5.17  BV=7.55
    {638,1152,1024, 0, 0, 0},  //TV = 10.61(43 lines)  AV=2.00  SV=5.17  BV=7.44
    {683,1152,1024, 0, 0, 0},  //TV = 10.52(46 lines)  AV=2.00  SV=5.17  BV=7.35
    {727,1152,1024, 0, 0, 0},  //TV = 10.43(49 lines)  AV=2.00  SV=5.17  BV=7.26
    {787,1152,1024, 0, 0, 0},  //TV = 10.31(53 lines)  AV=2.00  SV=5.17  BV=7.14
    {846,1136,1024, 0, 0, 0},  //TV = 10.21(57 lines)  AV=2.00  SV=5.15  BV=7.06
    {905,1136,1024, 0, 0, 0},  //TV = 10.11(61 lines)  AV=2.00  SV=5.15  BV=6.96
    {965,1152,1024, 0, 0, 0},  //TV = 10.02(65 lines)  AV=2.00  SV=5.17  BV=6.85
    {1039,1152,1024, 0, 0, 0},  //TV = 9.91(70 lines)  AV=2.00  SV=5.17  BV=6.74
    {1128,1136,1024, 0, 0, 0},  //TV = 9.79(76 lines)  AV=2.00  SV=5.15  BV=6.64
    {1202,1136,1024, 0, 0, 0},  //TV = 9.70(81 lines)  AV=2.00  SV=5.15  BV=6.55
    {1291,1136,1024, 0, 0, 0},  //TV = 9.60(87 lines)  AV=2.00  SV=5.15  BV=6.45
    {1380,1136,1024, 0, 0, 0},  //TV = 9.50(93 lines)  AV=2.00  SV=5.15  BV=6.35
    {1484,1136,1024, 0, 0, 0},  //TV = 9.40(100 lines)  AV=2.00  SV=5.15  BV=6.25
    {1588,1136,1024, 0, 0, 0},  //TV = 9.30(107 lines)  AV=2.00  SV=5.15  BV=6.15
    {1692,1136,1024, 0, 0, 0},  //TV = 9.21(114 lines)  AV=2.00  SV=5.15  BV=6.06
    {1810,1136,1024, 0, 0, 0},  //TV = 9.11(122 lines)  AV=2.00  SV=5.15  BV=5.96
    {1959,1136,1024, 0, 0, 0},  //TV = 9.00(132 lines)  AV=2.00  SV=5.15  BV=5.85
    {2107,1136,1024, 0, 0, 0},  //TV = 8.89(142 lines)  AV=2.00  SV=5.15  BV=5.74
    {2255,1136,1024, 0, 0, 0},  //TV = 8.79(152 lines)  AV=2.00  SV=5.15  BV=5.64
    {2389,1136,1024, 0, 0, 0},  //TV = 8.71(161 lines)  AV=2.00  SV=5.15  BV=5.56
    {2582,1136,1024, 0, 0, 0},  //TV = 8.60(174 lines)  AV=2.00  SV=5.15  BV=5.45
    {2774,1136,1024, 0, 0, 0},  //TV = 8.49(187 lines)  AV=2.00  SV=5.15  BV=5.34
    {2967,1136,1024, 0, 0, 0},  //TV = 8.40(200 lines)  AV=2.00  SV=5.15  BV=5.25
    {3175,1136,1024, 0, 0, 0},  //TV = 8.30(214 lines)  AV=2.00  SV=5.15  BV=5.15
    {3397,1136,1024, 0, 0, 0},  //TV = 8.20(229 lines)  AV=2.00  SV=5.15  BV=5.05
    {3635,1136,1024, 0, 0, 0},  //TV = 8.10(245 lines)  AV=2.00  SV=5.15  BV=4.95
    {3932,1136,1024, 0, 0, 0},  //TV = 7.99(265 lines)  AV=2.00  SV=5.15  BV=4.84
    {4199,1136,1024, 0, 0, 0},  //TV = 7.90(283 lines)  AV=2.00  SV=5.15  BV=4.75
    {4495,1136,1024, 0, 0, 0},  //TV = 7.80(303 lines)  AV=2.00  SV=5.15  BV=4.65
    {4822,1136,1024, 0, 0, 0},  //TV = 7.70(325 lines)  AV=2.00  SV=5.15  BV=4.55
    {5163,1136,1024, 0, 0, 0},  //TV = 7.60(348 lines)  AV=2.00  SV=5.15  BV=4.45
    {5563,1136,1024, 0, 0, 0},  //TV = 7.49(375 lines)  AV=2.00  SV=5.15  BV=4.34
    {5949,1136,1024, 0, 0, 0},  //TV = 7.39(401 lines)  AV=2.00  SV=5.15  BV=4.24
    {6379,1136,1024, 0, 0, 0},  //TV = 7.29(430 lines)  AV=2.00  SV=5.15  BV=4.14
    {6824,1136,1024, 0, 0, 0},  //TV = 7.20(460 lines)  AV=2.00  SV=5.15  BV=4.05
    {7314,1136,1024, 0, 0, 0},  //TV = 7.10(493 lines)  AV=2.00  SV=5.15  BV=3.95
    {7833,1136,1024, 0, 0, 0},  //TV = 7.00(528 lines)  AV=2.00  SV=5.15  BV=3.85
    {8337,1136,1024, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.15  BV=3.76
    {8337,1152,1088, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.26  BV=3.65
    {8337,1280,1048, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.36  BV=3.55
    {8337,1408,1024, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.46  BV=3.45
    {8337,1408,1104, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.57  BV=3.34
    {8337,1536,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.65  BV=3.26
    {8337,1664,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.77  BV=3.14
    {8337,1792,1056, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.85  BV=3.05
    {8337,1920,1064, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.96  BV=2.94
    {8337,2048,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=6.07  BV=2.84
    {16659,1136,1024, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.15  BV=2.76
    {16659,1152,1088, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.26  BV=2.65
    {16659,1280,1056, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.37  BV=2.54
    {16659,1408,1024, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.46  BV=2.45
    {16659,1408,1096, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.56  BV=2.35
    {16659,1536,1080, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.66  BV=2.25
    {24996,1152,1024, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.17  BV=2.15
    {24996,1152,1096, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.27  BV=2.05
    {24996,1280,1056, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.37  BV=1.96
    {24996,1408,1032, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.47  BV=1.85
    {33332,1136,1024, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.15  BV=1.76
    {33332,1152,1088, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.26  BV=1.65
    {33332,1280,1048, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.36  BV=1.55
    {33332,1408,1024, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.46  BV=1.45
    {33332,1408,1096, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.56  BV=1.35
    {33332,1536,1072, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.65  BV=1.26
    {33332,1664,1072, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.77  BV=1.14
    {33332,1792,1064, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.86  BV=1.04
    {33332,1920,1056, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.95  BV=0.96
    {33332,2048,1064, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=6.06  BV=0.85
    {33332,2176,1072, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=6.15  BV=0.75
    {41669,1920,1040, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=5.93  BV=0.66
    {41669,2048,1048, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=6.03  BV=0.55
    {41669,2176,1056, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=6.13  BV=0.45
    {50006,1920,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.97  BV=0.35
    {50006,2048,1080, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.08  BV=0.24
    {50006,2304,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.17  BV=0.15
    {50006,2432,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.27  BV=0.05
    {50006,2560,1064, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.38  BV=-0.06
    {50006,2816,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.47  BV=-0.15
    {50006,2944,1056, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.57  BV=-0.25
    {50006,3200,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.67  BV=-0.34
    {50006,3456,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.78  BV=-0.46
    {50006,3712,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.87  BV=-0.55
    {50006,3968,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.97  BV=-0.64
    {50006,4224,1048, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.08  BV=-0.76
    {58328,3968,1024, 0, 0, 0},  //TV = 4.10(3932 lines)  AV=2.00  SV=6.95  BV=-0.85
    {58328,4224,1032, 0, 0, 0},  //TV = 4.10(3932 lines)  AV=2.00  SV=7.06  BV=-0.96
    {66664,3968,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=6.97  BV=-1.06
    {66664,4224,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.06  BV=-1.15
    {66664,4480,1040, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.15  BV=-1.24
    {66664,4864,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.26  BV=-1.35
    {66664,5248,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.36  BV=-1.45
    {66664,5632,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.46  BV=-1.55
    {66664,6016,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.55  BV=-1.65
    {66664,6400,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.66  BV=-1.75
    {66664,6912,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.75  BV=-1.85
    {66664,7424,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.86  BV=-1.95
    {66664,7936,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.95  BV=-2.05
    {66664,8448,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=8.06  BV=-2.15
    {75001,8064,1032, 0, 0, 0},  //TV = 3.74(5056 lines)  AV=2.00  SV=7.99  BV=-2.25
    {83338,7808,1024, 0, 0, 0},  //TV = 3.58(5618 lines)  AV=2.00  SV=7.93  BV=-2.35
    {83338,8320,1032, 0, 0, 0},  //TV = 3.58(5618 lines)  AV=2.00  SV=8.03  BV=-2.45
    {91660,8192,1024, 0, 0, 0},  //TV = 3.45(6179 lines)  AV=2.00  SV=8.00  BV=-2.55
    {99996,7936,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=7.97  BV=-2.64
    {99996,8576,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.08  BV=-2.76
    {99996,9216,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.17  BV=-2.85
    {99996,9856,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.27  BV=-2.94
    {99996,10624,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.38  BV=-3.05
    {99996,11392,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.48  BV=-3.15
    {99996,12160,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.57  BV=-3.25
    {99996,13056,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.67  BV=-3.35
    {99996,13952,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.77  BV=-3.45
    {99996,14976,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.87  BV=-3.55
    {99996,16000,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.97  BV=-3.64
    {99996,16384,1024, 0, 0, 0},  /* TV = 3.32(6741 lines)  AV=2.00  SV=9.00  BV=-3.68 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
},
};

static strEvPline sCustom1PLineTable_50Hz =
{
{
    {30,1152,1104, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.28  BV=11.75
    {30,1280,1056, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.37  BV=11.66
    {30,1408,1040, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.48  BV=11.54
    {30,1536,1032, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.60  BV=11.43
    {30,1664,1024, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.70  BV=11.32
    {45,1152,1048, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.20  BV=11.24
    {45,1152,1120, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.30  BV=11.14
    {45,1280,1088, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.41  BV=11.03
    {45,1408,1040, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.48  BV=10.96
    {60,1152,1056, 0, 0, 0},  /* TV = 14.02(4 lines)  AV=2.00  SV=5.21  BV=10.81 */
    {60,1152,1128, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.31  BV=10.72
    {60,1280,1072, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.39  BV=10.64
    {75,1152,1032, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.18  BV=10.52
    {75,1152,1088, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.26  BV=10.45
    {75,1280,1056, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.37  BV=10.34
    {90,1152,1040, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.19  BV=10.25
    {90,1280,1024, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.32  BV=10.12
    {104,1152,1040, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.19  BV=10.04
    {104,1152,1104, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.28  BV=9.95
    {119,1152,1032, 0, 0, 0},  //TV = 13.04(8 lines)  AV=2.00  SV=5.18  BV=9.86
    {134,1136,1024, 0, 0, 0},  /* TV = 12.87(9 lines)  AV=2.00  SV=5.15  BV=9.72 */
    {134,1152,1064, 0, 0, 0},  //TV = 12.87(9 lines)  AV=2.00  SV=5.23  BV=9.64
    {149,1152,1032, 0, 0, 0},  //TV = 12.71(10 lines)  AV=2.00  SV=5.18  BV=9.53
    {164,1136,1024, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.15  BV=9.42
    {164,1152,1088, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.26  BV=9.32
    {179,1152,1040, 0, 0, 0},  /* TV = 12.45(12 lines)  AV=2.00  SV=5.19  BV=9.26 */
    {193,1152,1032, 0, 0, 0},  //TV = 12.34(13 lines)  AV=2.00  SV=5.18  BV=9.16
    {208,1152,1032, 0, 0, 0},  //TV = 12.23(14 lines)  AV=2.00  SV=5.18  BV=9.05
    {223,1152,1032, 0, 0, 0},  //TV = 12.13(15 lines)  AV=2.00  SV=5.18  BV=8.95
    {238,1152,1032, 0, 0, 0},  //TV = 12.04(16 lines)  AV=2.00  SV=5.18  BV=8.86
    {253,1152,1040, 0, 0, 0},  //TV = 11.95(17 lines)  AV=2.00  SV=5.19  BV=8.76
    {268,1152,1056, 0, 0, 0},  //TV = 11.87(18 lines)  AV=2.00  SV=5.21  BV=8.65
    {297,1152,1024, 0, 0, 0},  //TV = 11.72(20 lines)  AV=2.00  SV=5.17  BV=8.55
    {312,1152,1040, 0, 0, 0},  //TV = 11.65(21 lines)  AV=2.00  SV=5.19  BV=8.45
    {342,1152,1024, 0, 0, 0},  //TV = 11.51(23 lines)  AV=2.00  SV=5.17  BV=8.34
    {371,1136,1024, 0, 0, 0},  //TV = 11.40(25 lines)  AV=2.00  SV=5.15  BV=8.25
    {386,1152,1032, 0, 0, 0},  //TV = 11.34(26 lines)  AV=2.00  SV=5.18  BV=8.16
    {416,1152,1032, 0, 0, 0},  //TV = 11.23(28 lines)  AV=2.00  SV=5.18  BV=8.05
    {446,1152,1032, 0, 0, 0},  //TV = 11.13(30 lines)  AV=2.00  SV=5.18  BV=7.95
    {475,1152,1032, 0, 0, 0},  //TV = 11.04(32 lines)  AV=2.00  SV=5.18  BV=7.86
    {520,1136,1024, 0, 0, 0},  //TV = 10.91(35 lines)  AV=2.00  SV=5.15  BV=7.76
    {564,1136,1024, 0, 0, 0},  //TV = 10.79(38 lines)  AV=2.00  SV=5.15  BV=7.64
    {594,1152,1024, 0, 0, 0},  //TV = 10.72(40 lines)  AV=2.00  SV=5.17  BV=7.55
    {638,1152,1024, 0, 0, 0},  //TV = 10.61(43 lines)  AV=2.00  SV=5.17  BV=7.44
    {683,1152,1024, 0, 0, 0},  //TV = 10.52(46 lines)  AV=2.00  SV=5.17  BV=7.35
    {727,1152,1024, 0, 0, 0},  //TV = 10.43(49 lines)  AV=2.00  SV=5.17  BV=7.26
    {787,1152,1024, 0, 0, 0},  //TV = 10.31(53 lines)  AV=2.00  SV=5.17  BV=7.14
    {846,1136,1024, 0, 0, 0},  //TV = 10.21(57 lines)  AV=2.00  SV=5.15  BV=7.06
    {905,1136,1024, 0, 0, 0},  //TV = 10.11(61 lines)  AV=2.00  SV=5.15  BV=6.96
    {965,1152,1024, 0, 0, 0},  //TV = 10.02(65 lines)  AV=2.00  SV=5.17  BV=6.85
    {1039,1152,1024, 0, 0, 0},  //TV = 9.91(70 lines)  AV=2.00  SV=5.17  BV=6.74
    {1128,1136,1024, 0, 0, 0},  //TV = 9.79(76 lines)  AV=2.00  SV=5.15  BV=6.64
    {1202,1136,1024, 0, 0, 0},  //TV = 9.70(81 lines)  AV=2.00  SV=5.15  BV=6.55
    {1291,1136,1024, 0, 0, 0},  //TV = 9.60(87 lines)  AV=2.00  SV=5.15  BV=6.45
    {1380,1136,1024, 0, 0, 0},  //TV = 9.50(93 lines)  AV=2.00  SV=5.15  BV=6.35
    {1484,1136,1024, 0, 0, 0},  //TV = 9.40(100 lines)  AV=2.00  SV=5.15  BV=6.25
    {1588,1136,1024, 0, 0, 0},  //TV = 9.30(107 lines)  AV=2.00  SV=5.15  BV=6.15
    {1692,1136,1024, 0, 0, 0},  //TV = 9.21(114 lines)  AV=2.00  SV=5.15  BV=6.06
    {1810,1136,1024, 0, 0, 0},  //TV = 9.11(122 lines)  AV=2.00  SV=5.15  BV=5.96
    {1959,1136,1024, 0, 0, 0},  //TV = 9.00(132 lines)  AV=2.00  SV=5.15  BV=5.85
    {2107,1136,1024, 0, 0, 0},  //TV = 8.89(142 lines)  AV=2.00  SV=5.15  BV=5.74
    {2255,1136,1024, 0, 0, 0},  //TV = 8.79(152 lines)  AV=2.00  SV=5.15  BV=5.64
    {2389,1136,1024, 0, 0, 0},  //TV = 8.71(161 lines)  AV=2.00  SV=5.15  BV=5.56
    {2582,1136,1024, 0, 0, 0},  //TV = 8.60(174 lines)  AV=2.00  SV=5.15  BV=5.45
    {2774,1136,1024, 0, 0, 0},  //TV = 8.49(187 lines)  AV=2.00  SV=5.15  BV=5.34
    {2967,1136,1024, 0, 0, 0},  //TV = 8.40(200 lines)  AV=2.00  SV=5.15  BV=5.25
    {3175,1136,1024, 0, 0, 0},  //TV = 8.30(214 lines)  AV=2.00  SV=5.15  BV=5.15
    {3397,1136,1024, 0, 0, 0},  //TV = 8.20(229 lines)  AV=2.00  SV=5.15  BV=5.05
    {3635,1136,1024, 0, 0, 0},  //TV = 8.10(245 lines)  AV=2.00  SV=5.15  BV=4.95
    {3932,1136,1024, 0, 0, 0},  //TV = 7.99(265 lines)  AV=2.00  SV=5.15  BV=4.84
    {4199,1136,1024, 0, 0, 0},  //TV = 7.90(283 lines)  AV=2.00  SV=5.15  BV=4.75
    {4495,1136,1024, 0, 0, 0},  //TV = 7.80(303 lines)  AV=2.00  SV=5.15  BV=4.65
    {4822,1136,1024, 0, 0, 0},  //TV = 7.70(325 lines)  AV=2.00  SV=5.15  BV=4.55
    {5163,1136,1024, 0, 0, 0},  //TV = 7.60(348 lines)  AV=2.00  SV=5.15  BV=4.45
    {5563,1136,1024, 0, 0, 0},  //TV = 7.49(375 lines)  AV=2.00  SV=5.15  BV=4.34
    {5949,1136,1024, 0, 0, 0},  //TV = 7.39(401 lines)  AV=2.00  SV=5.15  BV=4.24
    {6379,1136,1024, 0, 0, 0},  //TV = 7.29(430 lines)  AV=2.00  SV=5.15  BV=4.14
    {6824,1136,1024, 0, 0, 0},  //TV = 7.20(460 lines)  AV=2.00  SV=5.15  BV=4.05
    {7314,1136,1024, 0, 0, 0},  //TV = 7.10(493 lines)  AV=2.00  SV=5.15  BV=3.95
    {7833,1136,1024, 0, 0, 0},  //TV = 7.00(528 lines)  AV=2.00  SV=5.15  BV=3.85
    {8397,1136,1024, 0, 0, 0},  //TV = 6.90(566 lines)  AV=2.00  SV=5.15  BV=3.75
    {8990,1136,1024, 0, 0, 0},  //TV = 6.80(606 lines)  AV=2.00  SV=5.15  BV=3.65
    {9628,1136,1024, 0, 0, 0},  //TV = 6.70(649 lines)  AV=2.00  SV=5.15  BV=3.55
    {9999,1152,1040, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.19  BV=3.45
    {9999,1152,1112, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.29  BV=3.36
    {9999,1280,1080, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.40  BV=3.25
    {9999,1408,1056, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.50  BV=3.14
    {9999,1536,1032, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.60  BV=3.05
    {9999,1664,1024, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.70  BV=2.94
    {9999,1664,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.79  BV=2.86
    {9999,1792,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.89  BV=2.75
    {9999,1920,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.99  BV=2.65
    {9999,2176,1032, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=6.10  BV=2.55
    {19997,1152,1040, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.19  BV=2.45
    {19997,1152,1112, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.29  BV=2.36
    {19997,1280,1080, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.40  BV=2.25
    {19997,1408,1048, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.49  BV=2.15
    {19997,1536,1032, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.60  BV=2.05
    {19997,1664,1024, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.70  BV=1.94
    {29995,1152,1048, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.20  BV=1.86
    {29995,1152,1128, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.31  BV=1.75
    {29995,1280,1088, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.41  BV=1.65
    {29995,1408,1064, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.51  BV=1.54
    {29995,1536,1040, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.61  BV=1.45
    {29995,1664,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.71  BV=1.35
    {29995,1792,1024, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.81  BV=1.25
    {29995,1920,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.92  BV=1.14
    {29995,2048,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.01  BV=1.05
    {29995,2176,1040, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.11  BV=0.95
    {29995,2304,1056, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.21  BV=0.84
    {40008,1792,1088, 0, 0, 0},  //TV = 4.64(2697 lines)  AV=2.00  SV=5.89  BV=0.75
    {40008,1920,1088, 0, 0, 0},  //TV = 4.64(2697 lines)  AV=2.00  SV=5.99  BV=0.65
    {40008,2176,1024, 0, 0, 0},  //TV = 4.64(2697 lines)  AV=2.00  SV=6.09  BV=0.56
    {50006,1792,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.87  BV=0.45
    {50006,1920,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.97  BV=0.35
    {50006,2048,1080, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.08  BV=0.24
    {50006,2304,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.17  BV=0.15
    {50006,2432,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.27  BV=0.05
    {50006,2560,1064, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.38  BV=-0.06
    {50006,2816,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.47  BV=-0.15
    {50006,2944,1056, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.57  BV=-0.25
    {50006,3200,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.67  BV=-0.34
    {50006,3456,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.78  BV=-0.46
    {50006,3712,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.87  BV=-0.55
    {50006,3968,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.97  BV=-0.64
    {50006,4224,1048, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.08  BV=-0.76
    {60004,3840,1024, 0, 0, 0},  //TV = 4.06(4045 lines)  AV=2.00  SV=6.91  BV=-0.85
    {60004,4096,1032, 0, 0, 0},  //TV = 4.06(4045 lines)  AV=2.00  SV=7.01  BV=-0.95
    {70002,3712,1048, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=6.89  BV=-1.05
    {70002,3968,1048, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=6.99  BV=-1.15
    {70002,4352,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.09  BV=-1.25
    {70002,4608,1040, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.19  BV=-1.36
    {70002,4992,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.29  BV=-1.45
    {70002,5376,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.39  BV=-1.56
    {70002,5632,1040, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.48  BV=-1.65
    {70002,6144,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.58  BV=-1.75
    {70002,6528,1032, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.68  BV=-1.85
    {70002,7040,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.78  BV=-1.94
    {70002,7552,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.88  BV=-2.05
    {70002,8064,1032, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.99  BV=-2.15
    {70002,8704,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=8.09  BV=-2.25
    {80000,8064,1032, 0, 0, 0},  //TV = 3.64(5393 lines)  AV=2.00  SV=7.99  BV=-2.34
    {89998,7680,1032, 0, 0, 0},  //TV = 3.47(6067 lines)  AV=2.00  SV=7.92  BV=-2.44
    {89998,8320,1024, 0, 0, 0},  //TV = 3.47(6067 lines)  AV=2.00  SV=8.02  BV=-2.55
    {99996,7936,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=7.97  BV=-2.64
    {99996,8576,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.08  BV=-2.76
    {99996,9216,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.17  BV=-2.85
    {99996,9856,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.27  BV=-2.94
    {99996,10624,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.38  BV=-3.05
    {99996,11392,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.48  BV=-3.15
    {99996,12160,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.57  BV=-3.25
    {99996,13056,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.67  BV=-3.35
    {99996,13952,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.77  BV=-3.45
    {99996,14976,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.87  BV=-3.55
    {99996,16000,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.97  BV=-3.64
    {99996,16384,1024, 0, 0, 0},  /* TV = 3.32(6741 lines)  AV=2.00  SV=9.00  BV=-3.68 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
},
};

static strAETable g_AE_Custom1AutoTable =
{
    AETABLE_CUSTOM1_AUTO, //eAETableID
    156, //u4TotalIndex
    -20, //i4StrobeTrigerBV
    118, //i4MaxBV
    -37, //i4MinBV
    90, //i4EffectiveMaxBV
    0, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_AUTO, //ISO
    sCustom1PLineTable_60Hz,
    sCustom1PLineTable_50Hz,
    NULL,
};
static strEvPline sCustom2PLineTable_60Hz =
{
{
    {30,1152,1104, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.28  BV=11.75
    {30,1280,1056, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.37  BV=11.66
    {30,1408,1040, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.48  BV=11.54
    {30,1536,1032, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.60  BV=11.43
    {30,1664,1024, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.70  BV=11.32
    {45,1152,1048, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.20  BV=11.24
    {45,1152,1120, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.30  BV=11.14
    {45,1280,1088, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.41  BV=11.03
    {45,1408,1040, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.48  BV=10.96
    {60,1152,1056, 0, 0, 0},  /* TV = 14.02(4 lines)  AV=2.00  SV=5.21  BV=10.81 */
    {60,1152,1128, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.31  BV=10.72
    {60,1280,1072, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.39  BV=10.64
    {75,1152,1032, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.18  BV=10.52
    {75,1152,1088, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.26  BV=10.45
    {75,1280,1056, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.37  BV=10.34
    {90,1152,1040, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.19  BV=10.25
    {90,1280,1024, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.32  BV=10.12
    {104,1152,1040, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.19  BV=10.04
    {104,1152,1104, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.28  BV=9.95
    {119,1152,1032, 0, 0, 0},  //TV = 13.04(8 lines)  AV=2.00  SV=5.18  BV=9.86
    {134,1136,1024, 0, 0, 0},  /* TV = 12.87(9 lines)  AV=2.00  SV=5.15  BV=9.72 */
    {134,1152,1064, 0, 0, 0},  //TV = 12.87(9 lines)  AV=2.00  SV=5.23  BV=9.64
    {149,1152,1032, 0, 0, 0},  //TV = 12.71(10 lines)  AV=2.00  SV=5.18  BV=9.53
    {164,1136,1024, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.15  BV=9.42
    {164,1152,1088, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.26  BV=9.32
    {179,1152,1040, 0, 0, 0},  /* TV = 12.45(12 lines)  AV=2.00  SV=5.19  BV=9.26 */
    {193,1152,1032, 0, 0, 0},  //TV = 12.34(13 lines)  AV=2.00  SV=5.18  BV=9.16
    {208,1152,1032, 0, 0, 0},  //TV = 12.23(14 lines)  AV=2.00  SV=5.18  BV=9.05
    {223,1152,1032, 0, 0, 0},  //TV = 12.13(15 lines)  AV=2.00  SV=5.18  BV=8.95
    {238,1152,1032, 0, 0, 0},  //TV = 12.04(16 lines)  AV=2.00  SV=5.18  BV=8.86
    {253,1152,1040, 0, 0, 0},  //TV = 11.95(17 lines)  AV=2.00  SV=5.19  BV=8.76
    {268,1152,1056, 0, 0, 0},  //TV = 11.87(18 lines)  AV=2.00  SV=5.21  BV=8.65
    {297,1152,1024, 0, 0, 0},  //TV = 11.72(20 lines)  AV=2.00  SV=5.17  BV=8.55
    {312,1152,1040, 0, 0, 0},  //TV = 11.65(21 lines)  AV=2.00  SV=5.19  BV=8.45
    {342,1152,1024, 0, 0, 0},  //TV = 11.51(23 lines)  AV=2.00  SV=5.17  BV=8.34
    {371,1136,1024, 0, 0, 0},  //TV = 11.40(25 lines)  AV=2.00  SV=5.15  BV=8.25
    {386,1152,1032, 0, 0, 0},  //TV = 11.34(26 lines)  AV=2.00  SV=5.18  BV=8.16
    {416,1152,1032, 0, 0, 0},  //TV = 11.23(28 lines)  AV=2.00  SV=5.18  BV=8.05
    {446,1152,1032, 0, 0, 0},  //TV = 11.13(30 lines)  AV=2.00  SV=5.18  BV=7.95
    {475,1152,1032, 0, 0, 0},  //TV = 11.04(32 lines)  AV=2.00  SV=5.18  BV=7.86
    {520,1136,1024, 0, 0, 0},  //TV = 10.91(35 lines)  AV=2.00  SV=5.15  BV=7.76
    {564,1136,1024, 0, 0, 0},  //TV = 10.79(38 lines)  AV=2.00  SV=5.15  BV=7.64
    {594,1152,1024, 0, 0, 0},  //TV = 10.72(40 lines)  AV=2.00  SV=5.17  BV=7.55
    {638,1152,1024, 0, 0, 0},  //TV = 10.61(43 lines)  AV=2.00  SV=5.17  BV=7.44
    {683,1152,1024, 0, 0, 0},  //TV = 10.52(46 lines)  AV=2.00  SV=5.17  BV=7.35
    {727,1152,1024, 0, 0, 0},  //TV = 10.43(49 lines)  AV=2.00  SV=5.17  BV=7.26
    {787,1152,1024, 0, 0, 0},  //TV = 10.31(53 lines)  AV=2.00  SV=5.17  BV=7.14
    {846,1136,1024, 0, 0, 0},  //TV = 10.21(57 lines)  AV=2.00  SV=5.15  BV=7.06
    {905,1136,1024, 0, 0, 0},  //TV = 10.11(61 lines)  AV=2.00  SV=5.15  BV=6.96
    {965,1152,1024, 0, 0, 0},  //TV = 10.02(65 lines)  AV=2.00  SV=5.17  BV=6.85
    {1039,1152,1024, 0, 0, 0},  //TV = 9.91(70 lines)  AV=2.00  SV=5.17  BV=6.74
    {1128,1136,1024, 0, 0, 0},  //TV = 9.79(76 lines)  AV=2.00  SV=5.15  BV=6.64
    {1202,1136,1024, 0, 0, 0},  //TV = 9.70(81 lines)  AV=2.00  SV=5.15  BV=6.55
    {1291,1136,1024, 0, 0, 0},  //TV = 9.60(87 lines)  AV=2.00  SV=5.15  BV=6.45
    {1380,1136,1024, 0, 0, 0},  //TV = 9.50(93 lines)  AV=2.00  SV=5.15  BV=6.35
    {1484,1136,1024, 0, 0, 0},  //TV = 9.40(100 lines)  AV=2.00  SV=5.15  BV=6.25
    {1588,1136,1024, 0, 0, 0},  //TV = 9.30(107 lines)  AV=2.00  SV=5.15  BV=6.15
    {1692,1136,1024, 0, 0, 0},  //TV = 9.21(114 lines)  AV=2.00  SV=5.15  BV=6.06
    {1810,1136,1024, 0, 0, 0},  //TV = 9.11(122 lines)  AV=2.00  SV=5.15  BV=5.96
    {1959,1136,1024, 0, 0, 0},  //TV = 9.00(132 lines)  AV=2.00  SV=5.15  BV=5.85
    {2107,1136,1024, 0, 0, 0},  //TV = 8.89(142 lines)  AV=2.00  SV=5.15  BV=5.74
    {2255,1136,1024, 0, 0, 0},  //TV = 8.79(152 lines)  AV=2.00  SV=5.15  BV=5.64
    {2389,1136,1024, 0, 0, 0},  //TV = 8.71(161 lines)  AV=2.00  SV=5.15  BV=5.56
    {2582,1136,1024, 0, 0, 0},  //TV = 8.60(174 lines)  AV=2.00  SV=5.15  BV=5.45
    {2774,1136,1024, 0, 0, 0},  //TV = 8.49(187 lines)  AV=2.00  SV=5.15  BV=5.34
    {2967,1136,1024, 0, 0, 0},  //TV = 8.40(200 lines)  AV=2.00  SV=5.15  BV=5.25
    {3175,1136,1024, 0, 0, 0},  //TV = 8.30(214 lines)  AV=2.00  SV=5.15  BV=5.15
    {3397,1136,1024, 0, 0, 0},  //TV = 8.20(229 lines)  AV=2.00  SV=5.15  BV=5.05
    {3635,1136,1024, 0, 0, 0},  //TV = 8.10(245 lines)  AV=2.00  SV=5.15  BV=4.95
    {3932,1136,1024, 0, 0, 0},  //TV = 7.99(265 lines)  AV=2.00  SV=5.15  BV=4.84
    {4199,1136,1024, 0, 0, 0},  //TV = 7.90(283 lines)  AV=2.00  SV=5.15  BV=4.75
    {4495,1136,1024, 0, 0, 0},  //TV = 7.80(303 lines)  AV=2.00  SV=5.15  BV=4.65
    {4822,1136,1024, 0, 0, 0},  //TV = 7.70(325 lines)  AV=2.00  SV=5.15  BV=4.55
    {5163,1136,1024, 0, 0, 0},  //TV = 7.60(348 lines)  AV=2.00  SV=5.15  BV=4.45
    {5563,1136,1024, 0, 0, 0},  //TV = 7.49(375 lines)  AV=2.00  SV=5.15  BV=4.34
    {5949,1136,1024, 0, 0, 0},  //TV = 7.39(401 lines)  AV=2.00  SV=5.15  BV=4.24
    {6379,1136,1024, 0, 0, 0},  //TV = 7.29(430 lines)  AV=2.00  SV=5.15  BV=4.14
    {6824,1136,1024, 0, 0, 0},  //TV = 7.20(460 lines)  AV=2.00  SV=5.15  BV=4.05
    {7314,1136,1024, 0, 0, 0},  //TV = 7.10(493 lines)  AV=2.00  SV=5.15  BV=3.95
    {7833,1136,1024, 0, 0, 0},  //TV = 7.00(528 lines)  AV=2.00  SV=5.15  BV=3.85
    {8337,1136,1024, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.15  BV=3.76
    {8337,1152,1088, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.26  BV=3.65
    {8337,1280,1048, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.36  BV=3.55
    {8337,1408,1024, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.46  BV=3.45
    {8337,1408,1104, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.57  BV=3.34
    {8337,1536,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.65  BV=3.26
    {8337,1664,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.77  BV=3.14
    {8337,1792,1056, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.85  BV=3.05
    {8337,1920,1064, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.96  BV=2.94
    {8337,2048,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=6.07  BV=2.84
    {16659,1136,1024, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.15  BV=2.76
    {16659,1152,1088, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.26  BV=2.65
    {16659,1280,1056, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.37  BV=2.54
    {16659,1408,1024, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.46  BV=2.45
    {16659,1408,1096, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.56  BV=2.35
    {16659,1536,1080, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.66  BV=2.25
    {24996,1152,1024, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.17  BV=2.15
    {24996,1152,1096, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.27  BV=2.05
    {24996,1280,1056, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.37  BV=1.96
    {24996,1408,1032, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.47  BV=1.85
    {33332,1136,1024, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.15  BV=1.76
    {33332,1152,1088, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.26  BV=1.65
    {33332,1280,1048, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.36  BV=1.55
    {33332,1408,1024, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.46  BV=1.45
    {33332,1408,1096, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.56  BV=1.35
    {33332,1536,1072, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.65  BV=1.26
    {33332,1664,1072, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.77  BV=1.14
    {33332,1792,1064, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.86  BV=1.04
    {33332,1920,1056, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.95  BV=0.96
    {33332,2048,1064, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=6.06  BV=0.85
    {33332,2176,1072, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=6.15  BV=0.75
    {41669,1920,1040, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=5.93  BV=0.66
    {41669,2048,1048, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=6.03  BV=0.55
    {41669,2176,1056, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=6.13  BV=0.45
    {50006,1920,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.97  BV=0.35
    {50006,2048,1080, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.08  BV=0.24
    {50006,2304,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.17  BV=0.15
    {50006,2432,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.27  BV=0.05
    {50006,2560,1064, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.38  BV=-0.06
    {50006,2816,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.47  BV=-0.15
    {50006,2944,1056, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.57  BV=-0.25
    {50006,3200,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.67  BV=-0.34
    {50006,3456,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.78  BV=-0.46
    {50006,3712,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.87  BV=-0.55
    {50006,3968,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.97  BV=-0.64
    {50006,4224,1048, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.08  BV=-0.76
    {58328,3968,1024, 0, 0, 0},  //TV = 4.10(3932 lines)  AV=2.00  SV=6.95  BV=-0.85
    {58328,4224,1032, 0, 0, 0},  //TV = 4.10(3932 lines)  AV=2.00  SV=7.06  BV=-0.96
    {66664,3968,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=6.97  BV=-1.06
    {66664,4224,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.06  BV=-1.15
    {66664,4480,1040, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.15  BV=-1.24
    {66664,4864,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.26  BV=-1.35
    {66664,5248,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.36  BV=-1.45
    {66664,5632,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.46  BV=-1.55
    {66664,6016,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.55  BV=-1.65
    {66664,6400,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.66  BV=-1.75
    {66664,6912,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.75  BV=-1.85
    {66664,7424,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.86  BV=-1.95
    {66664,7936,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.95  BV=-2.05
    {66664,8448,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=8.06  BV=-2.15
    {75001,8064,1032, 0, 0, 0},  //TV = 3.74(5056 lines)  AV=2.00  SV=7.99  BV=-2.25
    {83338,7808,1024, 0, 0, 0},  //TV = 3.58(5618 lines)  AV=2.00  SV=7.93  BV=-2.35
    {83338,8320,1032, 0, 0, 0},  //TV = 3.58(5618 lines)  AV=2.00  SV=8.03  BV=-2.45
    {91660,8192,1024, 0, 0, 0},  //TV = 3.45(6179 lines)  AV=2.00  SV=8.00  BV=-2.55
    {99996,7936,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=7.97  BV=-2.64
    {99996,8576,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.08  BV=-2.76
    {99996,9216,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.17  BV=-2.85
    {99996,9856,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.27  BV=-2.94
    {99996,10624,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.38  BV=-3.05
    {99996,11392,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.48  BV=-3.15
    {99996,12160,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.57  BV=-3.25
    {99996,13056,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.67  BV=-3.35
    {99996,13952,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.77  BV=-3.45
    {99996,14976,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.87  BV=-3.55
    {99996,16000,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.97  BV=-3.64
    {99996,16384,1024, 0, 0, 0},  /* TV = 3.32(6741 lines)  AV=2.00  SV=9.00  BV=-3.68 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
},
};

static strEvPline sCustom2PLineTable_50Hz =
{
{
    {30,1152,1104, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.28  BV=11.75
    {30,1280,1056, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.37  BV=11.66
    {30,1408,1040, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.48  BV=11.54
    {30,1536,1032, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.60  BV=11.43
    {30,1664,1024, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.70  BV=11.32
    {45,1152,1048, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.20  BV=11.24
    {45,1152,1120, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.30  BV=11.14
    {45,1280,1088, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.41  BV=11.03
    {45,1408,1040, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.48  BV=10.96
    {60,1152,1056, 0, 0, 0},  /* TV = 14.02(4 lines)  AV=2.00  SV=5.21  BV=10.81 */
    {60,1152,1128, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.31  BV=10.72
    {60,1280,1072, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.39  BV=10.64
    {75,1152,1032, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.18  BV=10.52
    {75,1152,1088, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.26  BV=10.45
    {75,1280,1056, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.37  BV=10.34
    {90,1152,1040, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.19  BV=10.25
    {90,1280,1024, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.32  BV=10.12
    {104,1152,1040, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.19  BV=10.04
    {104,1152,1104, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.28  BV=9.95
    {119,1152,1032, 0, 0, 0},  //TV = 13.04(8 lines)  AV=2.00  SV=5.18  BV=9.86
    {134,1136,1024, 0, 0, 0},  /* TV = 12.87(9 lines)  AV=2.00  SV=5.15  BV=9.72 */
    {134,1152,1064, 0, 0, 0},  //TV = 12.87(9 lines)  AV=2.00  SV=5.23  BV=9.64
    {149,1152,1032, 0, 0, 0},  //TV = 12.71(10 lines)  AV=2.00  SV=5.18  BV=9.53
    {164,1136,1024, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.15  BV=9.42
    {164,1152,1088, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.26  BV=9.32
    {179,1152,1040, 0, 0, 0},  /* TV = 12.45(12 lines)  AV=2.00  SV=5.19  BV=9.26 */
    {193,1152,1032, 0, 0, 0},  //TV = 12.34(13 lines)  AV=2.00  SV=5.18  BV=9.16
    {208,1152,1032, 0, 0, 0},  //TV = 12.23(14 lines)  AV=2.00  SV=5.18  BV=9.05
    {223,1152,1032, 0, 0, 0},  //TV = 12.13(15 lines)  AV=2.00  SV=5.18  BV=8.95
    {238,1152,1032, 0, 0, 0},  //TV = 12.04(16 lines)  AV=2.00  SV=5.18  BV=8.86
    {253,1152,1040, 0, 0, 0},  //TV = 11.95(17 lines)  AV=2.00  SV=5.19  BV=8.76
    {268,1152,1056, 0, 0, 0},  //TV = 11.87(18 lines)  AV=2.00  SV=5.21  BV=8.65
    {297,1152,1024, 0, 0, 0},  //TV = 11.72(20 lines)  AV=2.00  SV=5.17  BV=8.55
    {312,1152,1040, 0, 0, 0},  //TV = 11.65(21 lines)  AV=2.00  SV=5.19  BV=8.45
    {342,1152,1024, 0, 0, 0},  //TV = 11.51(23 lines)  AV=2.00  SV=5.17  BV=8.34
    {371,1136,1024, 0, 0, 0},  //TV = 11.40(25 lines)  AV=2.00  SV=5.15  BV=8.25
    {386,1152,1032, 0, 0, 0},  //TV = 11.34(26 lines)  AV=2.00  SV=5.18  BV=8.16
    {416,1152,1032, 0, 0, 0},  //TV = 11.23(28 lines)  AV=2.00  SV=5.18  BV=8.05
    {446,1152,1032, 0, 0, 0},  //TV = 11.13(30 lines)  AV=2.00  SV=5.18  BV=7.95
    {475,1152,1032, 0, 0, 0},  //TV = 11.04(32 lines)  AV=2.00  SV=5.18  BV=7.86
    {520,1136,1024, 0, 0, 0},  //TV = 10.91(35 lines)  AV=2.00  SV=5.15  BV=7.76
    {564,1136,1024, 0, 0, 0},  //TV = 10.79(38 lines)  AV=2.00  SV=5.15  BV=7.64
    {594,1152,1024, 0, 0, 0},  //TV = 10.72(40 lines)  AV=2.00  SV=5.17  BV=7.55
    {638,1152,1024, 0, 0, 0},  //TV = 10.61(43 lines)  AV=2.00  SV=5.17  BV=7.44
    {683,1152,1024, 0, 0, 0},  //TV = 10.52(46 lines)  AV=2.00  SV=5.17  BV=7.35
    {727,1152,1024, 0, 0, 0},  //TV = 10.43(49 lines)  AV=2.00  SV=5.17  BV=7.26
    {787,1152,1024, 0, 0, 0},  //TV = 10.31(53 lines)  AV=2.00  SV=5.17  BV=7.14
    {846,1136,1024, 0, 0, 0},  //TV = 10.21(57 lines)  AV=2.00  SV=5.15  BV=7.06
    {905,1136,1024, 0, 0, 0},  //TV = 10.11(61 lines)  AV=2.00  SV=5.15  BV=6.96
    {965,1152,1024, 0, 0, 0},  //TV = 10.02(65 lines)  AV=2.00  SV=5.17  BV=6.85
    {1039,1152,1024, 0, 0, 0},  //TV = 9.91(70 lines)  AV=2.00  SV=5.17  BV=6.74
    {1128,1136,1024, 0, 0, 0},  //TV = 9.79(76 lines)  AV=2.00  SV=5.15  BV=6.64
    {1202,1136,1024, 0, 0, 0},  //TV = 9.70(81 lines)  AV=2.00  SV=5.15  BV=6.55
    {1291,1136,1024, 0, 0, 0},  //TV = 9.60(87 lines)  AV=2.00  SV=5.15  BV=6.45
    {1380,1136,1024, 0, 0, 0},  //TV = 9.50(93 lines)  AV=2.00  SV=5.15  BV=6.35
    {1484,1136,1024, 0, 0, 0},  //TV = 9.40(100 lines)  AV=2.00  SV=5.15  BV=6.25
    {1588,1136,1024, 0, 0, 0},  //TV = 9.30(107 lines)  AV=2.00  SV=5.15  BV=6.15
    {1692,1136,1024, 0, 0, 0},  //TV = 9.21(114 lines)  AV=2.00  SV=5.15  BV=6.06
    {1810,1136,1024, 0, 0, 0},  //TV = 9.11(122 lines)  AV=2.00  SV=5.15  BV=5.96
    {1959,1136,1024, 0, 0, 0},  //TV = 9.00(132 lines)  AV=2.00  SV=5.15  BV=5.85
    {2107,1136,1024, 0, 0, 0},  //TV = 8.89(142 lines)  AV=2.00  SV=5.15  BV=5.74
    {2255,1136,1024, 0, 0, 0},  //TV = 8.79(152 lines)  AV=2.00  SV=5.15  BV=5.64
    {2389,1136,1024, 0, 0, 0},  //TV = 8.71(161 lines)  AV=2.00  SV=5.15  BV=5.56
    {2582,1136,1024, 0, 0, 0},  //TV = 8.60(174 lines)  AV=2.00  SV=5.15  BV=5.45
    {2774,1136,1024, 0, 0, 0},  //TV = 8.49(187 lines)  AV=2.00  SV=5.15  BV=5.34
    {2967,1136,1024, 0, 0, 0},  //TV = 8.40(200 lines)  AV=2.00  SV=5.15  BV=5.25
    {3175,1136,1024, 0, 0, 0},  //TV = 8.30(214 lines)  AV=2.00  SV=5.15  BV=5.15
    {3397,1136,1024, 0, 0, 0},  //TV = 8.20(229 lines)  AV=2.00  SV=5.15  BV=5.05
    {3635,1136,1024, 0, 0, 0},  //TV = 8.10(245 lines)  AV=2.00  SV=5.15  BV=4.95
    {3932,1136,1024, 0, 0, 0},  //TV = 7.99(265 lines)  AV=2.00  SV=5.15  BV=4.84
    {4199,1136,1024, 0, 0, 0},  //TV = 7.90(283 lines)  AV=2.00  SV=5.15  BV=4.75
    {4495,1136,1024, 0, 0, 0},  //TV = 7.80(303 lines)  AV=2.00  SV=5.15  BV=4.65
    {4822,1136,1024, 0, 0, 0},  //TV = 7.70(325 lines)  AV=2.00  SV=5.15  BV=4.55
    {5163,1136,1024, 0, 0, 0},  //TV = 7.60(348 lines)  AV=2.00  SV=5.15  BV=4.45
    {5563,1136,1024, 0, 0, 0},  //TV = 7.49(375 lines)  AV=2.00  SV=5.15  BV=4.34
    {5949,1136,1024, 0, 0, 0},  //TV = 7.39(401 lines)  AV=2.00  SV=5.15  BV=4.24
    {6379,1136,1024, 0, 0, 0},  //TV = 7.29(430 lines)  AV=2.00  SV=5.15  BV=4.14
    {6824,1136,1024, 0, 0, 0},  //TV = 7.20(460 lines)  AV=2.00  SV=5.15  BV=4.05
    {7314,1136,1024, 0, 0, 0},  //TV = 7.10(493 lines)  AV=2.00  SV=5.15  BV=3.95
    {7833,1136,1024, 0, 0, 0},  //TV = 7.00(528 lines)  AV=2.00  SV=5.15  BV=3.85
    {8397,1136,1024, 0, 0, 0},  //TV = 6.90(566 lines)  AV=2.00  SV=5.15  BV=3.75
    {8990,1136,1024, 0, 0, 0},  //TV = 6.80(606 lines)  AV=2.00  SV=5.15  BV=3.65
    {9628,1136,1024, 0, 0, 0},  //TV = 6.70(649 lines)  AV=2.00  SV=5.15  BV=3.55
    {9999,1152,1040, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.19  BV=3.45
    {9999,1152,1112, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.29  BV=3.36
    {9999,1280,1080, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.40  BV=3.25
    {9999,1408,1056, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.50  BV=3.14
    {9999,1536,1032, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.60  BV=3.05
    {9999,1664,1024, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.70  BV=2.94
    {9999,1664,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.79  BV=2.86
    {9999,1792,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.89  BV=2.75
    {9999,1920,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.99  BV=2.65
    {9999,2176,1032, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=6.10  BV=2.55
    {19997,1152,1040, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.19  BV=2.45
    {19997,1152,1112, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.29  BV=2.36
    {19997,1280,1080, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.40  BV=2.25
    {19997,1408,1048, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.49  BV=2.15
    {19997,1536,1032, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.60  BV=2.05
    {19997,1664,1024, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.70  BV=1.94
    {29995,1152,1048, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.20  BV=1.86
    {29995,1152,1128, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.31  BV=1.75
    {29995,1280,1088, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.41  BV=1.65
    {29995,1408,1064, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.51  BV=1.54
    {29995,1536,1040, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.61  BV=1.45
    {29995,1664,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.71  BV=1.35
    {29995,1792,1024, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.81  BV=1.25
    {29995,1920,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.92  BV=1.14
    {29995,2048,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.01  BV=1.05
    {29995,2176,1040, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.11  BV=0.95
    {29995,2304,1056, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.21  BV=0.84
    {40008,1792,1088, 0, 0, 0},  //TV = 4.64(2697 lines)  AV=2.00  SV=5.89  BV=0.75
    {40008,1920,1088, 0, 0, 0},  //TV = 4.64(2697 lines)  AV=2.00  SV=5.99  BV=0.65
    {40008,2176,1024, 0, 0, 0},  //TV = 4.64(2697 lines)  AV=2.00  SV=6.09  BV=0.56
    {50006,1792,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.87  BV=0.45
    {50006,1920,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.97  BV=0.35
    {50006,2048,1080, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.08  BV=0.24
    {50006,2304,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.17  BV=0.15
    {50006,2432,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.27  BV=0.05
    {50006,2560,1064, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.38  BV=-0.06
    {50006,2816,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.47  BV=-0.15
    {50006,2944,1056, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.57  BV=-0.25
    {50006,3200,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.67  BV=-0.34
    {50006,3456,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.78  BV=-0.46
    {50006,3712,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.87  BV=-0.55
    {50006,3968,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.97  BV=-0.64
    {50006,4224,1048, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.08  BV=-0.76
    {60004,3840,1024, 0, 0, 0},  //TV = 4.06(4045 lines)  AV=2.00  SV=6.91  BV=-0.85
    {60004,4096,1032, 0, 0, 0},  //TV = 4.06(4045 lines)  AV=2.00  SV=7.01  BV=-0.95
    {70002,3712,1048, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=6.89  BV=-1.05
    {70002,3968,1048, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=6.99  BV=-1.15
    {70002,4352,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.09  BV=-1.25
    {70002,4608,1040, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.19  BV=-1.36
    {70002,4992,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.29  BV=-1.45
    {70002,5376,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.39  BV=-1.56
    {70002,5632,1040, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.48  BV=-1.65
    {70002,6144,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.58  BV=-1.75
    {70002,6528,1032, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.68  BV=-1.85
    {70002,7040,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.78  BV=-1.94
    {70002,7552,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.88  BV=-2.05
    {70002,8064,1032, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.99  BV=-2.15
    {70002,8704,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=8.09  BV=-2.25
    {80000,8064,1032, 0, 0, 0},  //TV = 3.64(5393 lines)  AV=2.00  SV=7.99  BV=-2.34
    {89998,7680,1032, 0, 0, 0},  //TV = 3.47(6067 lines)  AV=2.00  SV=7.92  BV=-2.44
    {89998,8320,1024, 0, 0, 0},  //TV = 3.47(6067 lines)  AV=2.00  SV=8.02  BV=-2.55
    {99996,7936,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=7.97  BV=-2.64
    {99996,8576,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.08  BV=-2.76
    {99996,9216,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.17  BV=-2.85
    {99996,9856,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.27  BV=-2.94
    {99996,10624,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.38  BV=-3.05
    {99996,11392,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.48  BV=-3.15
    {99996,12160,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.57  BV=-3.25
    {99996,13056,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.67  BV=-3.35
    {99996,13952,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.77  BV=-3.45
    {99996,14976,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.87  BV=-3.55
    {99996,16000,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.97  BV=-3.64
    {99996,16384,1024, 0, 0, 0},  /* TV = 3.32(6741 lines)  AV=2.00  SV=9.00  BV=-3.68 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
},
};

static strAETable g_AE_Custom2AutoTable =
{
    AETABLE_CUSTOM2_AUTO, //eAETableID
    156, //u4TotalIndex
    -20, //i4StrobeTrigerBV
    118, //i4MaxBV
    -37, //i4MinBV
    90, //i4EffectiveMaxBV
    0, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_AUTO, //ISO
    sCustom2PLineTable_60Hz,
    sCustom2PLineTable_50Hz,
    NULL,
};
static strEvPline sCustom3PLineTable_60Hz =
{
{
    {30,1152,1104, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.28  BV=11.75
    {30,1280,1056, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.37  BV=11.66
    {30,1408,1040, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.48  BV=11.54
    {30,1536,1032, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.60  BV=11.43
    {30,1664,1024, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.70  BV=11.32
    {45,1152,1048, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.20  BV=11.24
    {45,1152,1120, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.30  BV=11.14
    {45,1280,1088, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.41  BV=11.03
    {45,1408,1040, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.48  BV=10.96
    {60,1152,1056, 0, 0, 0},  /* TV = 14.02(4 lines)  AV=2.00  SV=5.21  BV=10.81 */
    {60,1152,1128, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.31  BV=10.72
    {60,1280,1072, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.39  BV=10.64
    {75,1152,1032, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.18  BV=10.52
    {75,1152,1088, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.26  BV=10.45
    {75,1280,1056, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.37  BV=10.34
    {90,1152,1040, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.19  BV=10.25
    {90,1280,1024, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.32  BV=10.12
    {104,1152,1040, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.19  BV=10.04
    {104,1152,1104, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.28  BV=9.95
    {119,1152,1032, 0, 0, 0},  //TV = 13.04(8 lines)  AV=2.00  SV=5.18  BV=9.86
    {134,1136,1024, 0, 0, 0},  /* TV = 12.87(9 lines)  AV=2.00  SV=5.15  BV=9.72 */
    {134,1152,1064, 0, 0, 0},  //TV = 12.87(9 lines)  AV=2.00  SV=5.23  BV=9.64
    {149,1152,1032, 0, 0, 0},  //TV = 12.71(10 lines)  AV=2.00  SV=5.18  BV=9.53
    {164,1136,1024, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.15  BV=9.42
    {164,1152,1088, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.26  BV=9.32
    {179,1152,1040, 0, 0, 0},  /* TV = 12.45(12 lines)  AV=2.00  SV=5.19  BV=9.26 */
    {193,1152,1032, 0, 0, 0},  //TV = 12.34(13 lines)  AV=2.00  SV=5.18  BV=9.16
    {208,1152,1032, 0, 0, 0},  //TV = 12.23(14 lines)  AV=2.00  SV=5.18  BV=9.05
    {223,1152,1032, 0, 0, 0},  //TV = 12.13(15 lines)  AV=2.00  SV=5.18  BV=8.95
    {238,1152,1032, 0, 0, 0},  //TV = 12.04(16 lines)  AV=2.00  SV=5.18  BV=8.86
    {253,1152,1040, 0, 0, 0},  //TV = 11.95(17 lines)  AV=2.00  SV=5.19  BV=8.76
    {268,1152,1056, 0, 0, 0},  //TV = 11.87(18 lines)  AV=2.00  SV=5.21  BV=8.65
    {297,1152,1024, 0, 0, 0},  //TV = 11.72(20 lines)  AV=2.00  SV=5.17  BV=8.55
    {312,1152,1040, 0, 0, 0},  //TV = 11.65(21 lines)  AV=2.00  SV=5.19  BV=8.45
    {342,1152,1024, 0, 0, 0},  //TV = 11.51(23 lines)  AV=2.00  SV=5.17  BV=8.34
    {371,1136,1024, 0, 0, 0},  //TV = 11.40(25 lines)  AV=2.00  SV=5.15  BV=8.25
    {386,1152,1032, 0, 0, 0},  //TV = 11.34(26 lines)  AV=2.00  SV=5.18  BV=8.16
    {416,1152,1032, 0, 0, 0},  //TV = 11.23(28 lines)  AV=2.00  SV=5.18  BV=8.05
    {446,1152,1032, 0, 0, 0},  //TV = 11.13(30 lines)  AV=2.00  SV=5.18  BV=7.95
    {475,1152,1032, 0, 0, 0},  //TV = 11.04(32 lines)  AV=2.00  SV=5.18  BV=7.86
    {520,1136,1024, 0, 0, 0},  //TV = 10.91(35 lines)  AV=2.00  SV=5.15  BV=7.76
    {564,1136,1024, 0, 0, 0},  //TV = 10.79(38 lines)  AV=2.00  SV=5.15  BV=7.64
    {594,1152,1024, 0, 0, 0},  //TV = 10.72(40 lines)  AV=2.00  SV=5.17  BV=7.55
    {638,1152,1024, 0, 0, 0},  //TV = 10.61(43 lines)  AV=2.00  SV=5.17  BV=7.44
    {683,1152,1024, 0, 0, 0},  //TV = 10.52(46 lines)  AV=2.00  SV=5.17  BV=7.35
    {727,1152,1024, 0, 0, 0},  //TV = 10.43(49 lines)  AV=2.00  SV=5.17  BV=7.26
    {787,1152,1024, 0, 0, 0},  //TV = 10.31(53 lines)  AV=2.00  SV=5.17  BV=7.14
    {846,1136,1024, 0, 0, 0},  //TV = 10.21(57 lines)  AV=2.00  SV=5.15  BV=7.06
    {905,1136,1024, 0, 0, 0},  //TV = 10.11(61 lines)  AV=2.00  SV=5.15  BV=6.96
    {965,1152,1024, 0, 0, 0},  //TV = 10.02(65 lines)  AV=2.00  SV=5.17  BV=6.85
    {1039,1152,1024, 0, 0, 0},  //TV = 9.91(70 lines)  AV=2.00  SV=5.17  BV=6.74
    {1128,1136,1024, 0, 0, 0},  //TV = 9.79(76 lines)  AV=2.00  SV=5.15  BV=6.64
    {1202,1136,1024, 0, 0, 0},  //TV = 9.70(81 lines)  AV=2.00  SV=5.15  BV=6.55
    {1291,1136,1024, 0, 0, 0},  //TV = 9.60(87 lines)  AV=2.00  SV=5.15  BV=6.45
    {1380,1136,1024, 0, 0, 0},  //TV = 9.50(93 lines)  AV=2.00  SV=5.15  BV=6.35
    {1484,1136,1024, 0, 0, 0},  //TV = 9.40(100 lines)  AV=2.00  SV=5.15  BV=6.25
    {1588,1136,1024, 0, 0, 0},  //TV = 9.30(107 lines)  AV=2.00  SV=5.15  BV=6.15
    {1692,1136,1024, 0, 0, 0},  //TV = 9.21(114 lines)  AV=2.00  SV=5.15  BV=6.06
    {1810,1136,1024, 0, 0, 0},  //TV = 9.11(122 lines)  AV=2.00  SV=5.15  BV=5.96
    {1959,1136,1024, 0, 0, 0},  //TV = 9.00(132 lines)  AV=2.00  SV=5.15  BV=5.85
    {2107,1136,1024, 0, 0, 0},  //TV = 8.89(142 lines)  AV=2.00  SV=5.15  BV=5.74
    {2255,1136,1024, 0, 0, 0},  //TV = 8.79(152 lines)  AV=2.00  SV=5.15  BV=5.64
    {2389,1136,1024, 0, 0, 0},  //TV = 8.71(161 lines)  AV=2.00  SV=5.15  BV=5.56
    {2582,1136,1024, 0, 0, 0},  //TV = 8.60(174 lines)  AV=2.00  SV=5.15  BV=5.45
    {2774,1136,1024, 0, 0, 0},  //TV = 8.49(187 lines)  AV=2.00  SV=5.15  BV=5.34
    {2967,1136,1024, 0, 0, 0},  //TV = 8.40(200 lines)  AV=2.00  SV=5.15  BV=5.25
    {3175,1136,1024, 0, 0, 0},  //TV = 8.30(214 lines)  AV=2.00  SV=5.15  BV=5.15
    {3397,1136,1024, 0, 0, 0},  //TV = 8.20(229 lines)  AV=2.00  SV=5.15  BV=5.05
    {3635,1136,1024, 0, 0, 0},  //TV = 8.10(245 lines)  AV=2.00  SV=5.15  BV=4.95
    {3932,1136,1024, 0, 0, 0},  //TV = 7.99(265 lines)  AV=2.00  SV=5.15  BV=4.84
    {4199,1136,1024, 0, 0, 0},  //TV = 7.90(283 lines)  AV=2.00  SV=5.15  BV=4.75
    {4495,1136,1024, 0, 0, 0},  //TV = 7.80(303 lines)  AV=2.00  SV=5.15  BV=4.65
    {4822,1136,1024, 0, 0, 0},  //TV = 7.70(325 lines)  AV=2.00  SV=5.15  BV=4.55
    {5163,1136,1024, 0, 0, 0},  //TV = 7.60(348 lines)  AV=2.00  SV=5.15  BV=4.45
    {5563,1136,1024, 0, 0, 0},  //TV = 7.49(375 lines)  AV=2.00  SV=5.15  BV=4.34
    {5949,1136,1024, 0, 0, 0},  //TV = 7.39(401 lines)  AV=2.00  SV=5.15  BV=4.24
    {6379,1136,1024, 0, 0, 0},  //TV = 7.29(430 lines)  AV=2.00  SV=5.15  BV=4.14
    {6824,1136,1024, 0, 0, 0},  //TV = 7.20(460 lines)  AV=2.00  SV=5.15  BV=4.05
    {7314,1136,1024, 0, 0, 0},  //TV = 7.10(493 lines)  AV=2.00  SV=5.15  BV=3.95
    {7833,1136,1024, 0, 0, 0},  //TV = 7.00(528 lines)  AV=2.00  SV=5.15  BV=3.85
    {8337,1136,1024, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.15  BV=3.76
    {8337,1152,1088, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.26  BV=3.65
    {8337,1280,1048, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.36  BV=3.55
    {8337,1408,1024, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.46  BV=3.45
    {8337,1408,1104, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.57  BV=3.34
    {8337,1536,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.65  BV=3.26
    {8337,1664,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.77  BV=3.14
    {8337,1792,1056, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.85  BV=3.05
    {8337,1920,1064, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.96  BV=2.94
    {8337,2048,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=6.07  BV=2.84
    {16659,1136,1024, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.15  BV=2.76
    {16659,1152,1088, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.26  BV=2.65
    {16659,1280,1056, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.37  BV=2.54
    {16659,1408,1024, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.46  BV=2.45
    {16659,1408,1096, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.56  BV=2.35
    {16659,1536,1080, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.66  BV=2.25
    {24996,1152,1024, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.17  BV=2.15
    {24996,1152,1096, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.27  BV=2.05
    {24996,1280,1056, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.37  BV=1.96
    {24996,1408,1032, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.47  BV=1.85
    {33332,1136,1024, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.15  BV=1.76
    {33332,1152,1088, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.26  BV=1.65
    {33332,1280,1048, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.36  BV=1.55
    {33332,1408,1024, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.46  BV=1.45
    {33332,1408,1096, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.56  BV=1.35
    {33332,1536,1072, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.65  BV=1.26
    {33332,1664,1072, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.77  BV=1.14
    {33332,1792,1064, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.86  BV=1.04
    {33332,1920,1056, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.95  BV=0.96
    {33332,2048,1064, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=6.06  BV=0.85
    {33332,2176,1072, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=6.15  BV=0.75
    {41669,1920,1040, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=5.93  BV=0.66
    {41669,2048,1048, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=6.03  BV=0.55
    {41669,2176,1056, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=6.13  BV=0.45
    {50006,1920,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.97  BV=0.35
    {50006,2048,1080, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.08  BV=0.24
    {50006,2304,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.17  BV=0.15
    {50006,2432,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.27  BV=0.05
    {50006,2560,1064, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.38  BV=-0.06
    {50006,2816,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.47  BV=-0.15
    {50006,2944,1056, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.57  BV=-0.25
    {50006,3200,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.67  BV=-0.34
    {50006,3456,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.78  BV=-0.46
    {50006,3712,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.87  BV=-0.55
    {50006,3968,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.97  BV=-0.64
    {50006,4224,1048, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.08  BV=-0.76
    {58328,3968,1024, 0, 0, 0},  //TV = 4.10(3932 lines)  AV=2.00  SV=6.95  BV=-0.85
    {58328,4224,1032, 0, 0, 0},  //TV = 4.10(3932 lines)  AV=2.00  SV=7.06  BV=-0.96
    {66664,3968,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=6.97  BV=-1.06
    {66664,4224,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.06  BV=-1.15
    {66664,4480,1040, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.15  BV=-1.24
    {66664,4864,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.26  BV=-1.35
    {66664,5248,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.36  BV=-1.45
    {66664,5632,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.46  BV=-1.55
    {66664,6016,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.55  BV=-1.65
    {66664,6400,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.66  BV=-1.75
    {66664,6912,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.75  BV=-1.85
    {66664,7424,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.86  BV=-1.95
    {66664,7936,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.95  BV=-2.05
    {66664,8448,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=8.06  BV=-2.15
    {75001,8064,1032, 0, 0, 0},  //TV = 3.74(5056 lines)  AV=2.00  SV=7.99  BV=-2.25
    {83338,7808,1024, 0, 0, 0},  //TV = 3.58(5618 lines)  AV=2.00  SV=7.93  BV=-2.35
    {83338,8320,1032, 0, 0, 0},  //TV = 3.58(5618 lines)  AV=2.00  SV=8.03  BV=-2.45
    {91660,8192,1024, 0, 0, 0},  //TV = 3.45(6179 lines)  AV=2.00  SV=8.00  BV=-2.55
    {99996,7936,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=7.97  BV=-2.64
    {99996,8576,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.08  BV=-2.76
    {99996,9216,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.17  BV=-2.85
    {99996,9856,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.27  BV=-2.94
    {99996,10624,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.38  BV=-3.05
    {99996,11392,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.48  BV=-3.15
    {99996,12160,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.57  BV=-3.25
    {99996,13056,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.67  BV=-3.35
    {99996,13952,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.77  BV=-3.45
    {99996,14976,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.87  BV=-3.55
    {99996,16000,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.97  BV=-3.64
    {99996,16384,1024, 0, 0, 0},  /* TV = 3.32(6741 lines)  AV=2.00  SV=9.00  BV=-3.68 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
},
};

static strEvPline sCustom3PLineTable_50Hz =
{
{
    {30,1152,1104, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.28  BV=11.75
    {30,1280,1056, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.37  BV=11.66
    {30,1408,1040, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.48  BV=11.54
    {30,1536,1032, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.60  BV=11.43
    {30,1664,1024, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.70  BV=11.32
    {45,1152,1048, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.20  BV=11.24
    {45,1152,1120, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.30  BV=11.14
    {45,1280,1088, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.41  BV=11.03
    {45,1408,1040, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.48  BV=10.96
    {60,1152,1056, 0, 0, 0},  /* TV = 14.02(4 lines)  AV=2.00  SV=5.21  BV=10.81 */
    {60,1152,1128, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.31  BV=10.72
    {60,1280,1072, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.39  BV=10.64
    {75,1152,1032, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.18  BV=10.52
    {75,1152,1088, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.26  BV=10.45
    {75,1280,1056, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.37  BV=10.34
    {90,1152,1040, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.19  BV=10.25
    {90,1280,1024, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.32  BV=10.12
    {104,1152,1040, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.19  BV=10.04
    {104,1152,1104, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.28  BV=9.95
    {119,1152,1032, 0, 0, 0},  //TV = 13.04(8 lines)  AV=2.00  SV=5.18  BV=9.86
    {134,1136,1024, 0, 0, 0},  /* TV = 12.87(9 lines)  AV=2.00  SV=5.15  BV=9.72 */
    {134,1152,1064, 0, 0, 0},  //TV = 12.87(9 lines)  AV=2.00  SV=5.23  BV=9.64
    {149,1152,1032, 0, 0, 0},  //TV = 12.71(10 lines)  AV=2.00  SV=5.18  BV=9.53
    {164,1136,1024, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.15  BV=9.42
    {164,1152,1088, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.26  BV=9.32
    {179,1152,1040, 0, 0, 0},  /* TV = 12.45(12 lines)  AV=2.00  SV=5.19  BV=9.26 */
    {193,1152,1032, 0, 0, 0},  //TV = 12.34(13 lines)  AV=2.00  SV=5.18  BV=9.16
    {208,1152,1032, 0, 0, 0},  //TV = 12.23(14 lines)  AV=2.00  SV=5.18  BV=9.05
    {223,1152,1032, 0, 0, 0},  //TV = 12.13(15 lines)  AV=2.00  SV=5.18  BV=8.95
    {238,1152,1032, 0, 0, 0},  //TV = 12.04(16 lines)  AV=2.00  SV=5.18  BV=8.86
    {253,1152,1040, 0, 0, 0},  //TV = 11.95(17 lines)  AV=2.00  SV=5.19  BV=8.76
    {268,1152,1056, 0, 0, 0},  //TV = 11.87(18 lines)  AV=2.00  SV=5.21  BV=8.65
    {297,1152,1024, 0, 0, 0},  //TV = 11.72(20 lines)  AV=2.00  SV=5.17  BV=8.55
    {312,1152,1040, 0, 0, 0},  //TV = 11.65(21 lines)  AV=2.00  SV=5.19  BV=8.45
    {342,1152,1024, 0, 0, 0},  //TV = 11.51(23 lines)  AV=2.00  SV=5.17  BV=8.34
    {371,1136,1024, 0, 0, 0},  //TV = 11.40(25 lines)  AV=2.00  SV=5.15  BV=8.25
    {386,1152,1032, 0, 0, 0},  //TV = 11.34(26 lines)  AV=2.00  SV=5.18  BV=8.16
    {416,1152,1032, 0, 0, 0},  //TV = 11.23(28 lines)  AV=2.00  SV=5.18  BV=8.05
    {446,1152,1032, 0, 0, 0},  //TV = 11.13(30 lines)  AV=2.00  SV=5.18  BV=7.95
    {475,1152,1032, 0, 0, 0},  //TV = 11.04(32 lines)  AV=2.00  SV=5.18  BV=7.86
    {520,1136,1024, 0, 0, 0},  //TV = 10.91(35 lines)  AV=2.00  SV=5.15  BV=7.76
    {564,1136,1024, 0, 0, 0},  //TV = 10.79(38 lines)  AV=2.00  SV=5.15  BV=7.64
    {594,1152,1024, 0, 0, 0},  //TV = 10.72(40 lines)  AV=2.00  SV=5.17  BV=7.55
    {638,1152,1024, 0, 0, 0},  //TV = 10.61(43 lines)  AV=2.00  SV=5.17  BV=7.44
    {683,1152,1024, 0, 0, 0},  //TV = 10.52(46 lines)  AV=2.00  SV=5.17  BV=7.35
    {727,1152,1024, 0, 0, 0},  //TV = 10.43(49 lines)  AV=2.00  SV=5.17  BV=7.26
    {787,1152,1024, 0, 0, 0},  //TV = 10.31(53 lines)  AV=2.00  SV=5.17  BV=7.14
    {846,1136,1024, 0, 0, 0},  //TV = 10.21(57 lines)  AV=2.00  SV=5.15  BV=7.06
    {905,1136,1024, 0, 0, 0},  //TV = 10.11(61 lines)  AV=2.00  SV=5.15  BV=6.96
    {965,1152,1024, 0, 0, 0},  //TV = 10.02(65 lines)  AV=2.00  SV=5.17  BV=6.85
    {1039,1152,1024, 0, 0, 0},  //TV = 9.91(70 lines)  AV=2.00  SV=5.17  BV=6.74
    {1128,1136,1024, 0, 0, 0},  //TV = 9.79(76 lines)  AV=2.00  SV=5.15  BV=6.64
    {1202,1136,1024, 0, 0, 0},  //TV = 9.70(81 lines)  AV=2.00  SV=5.15  BV=6.55
    {1291,1136,1024, 0, 0, 0},  //TV = 9.60(87 lines)  AV=2.00  SV=5.15  BV=6.45
    {1380,1136,1024, 0, 0, 0},  //TV = 9.50(93 lines)  AV=2.00  SV=5.15  BV=6.35
    {1484,1136,1024, 0, 0, 0},  //TV = 9.40(100 lines)  AV=2.00  SV=5.15  BV=6.25
    {1588,1136,1024, 0, 0, 0},  //TV = 9.30(107 lines)  AV=2.00  SV=5.15  BV=6.15
    {1692,1136,1024, 0, 0, 0},  //TV = 9.21(114 lines)  AV=2.00  SV=5.15  BV=6.06
    {1810,1136,1024, 0, 0, 0},  //TV = 9.11(122 lines)  AV=2.00  SV=5.15  BV=5.96
    {1959,1136,1024, 0, 0, 0},  //TV = 9.00(132 lines)  AV=2.00  SV=5.15  BV=5.85
    {2107,1136,1024, 0, 0, 0},  //TV = 8.89(142 lines)  AV=2.00  SV=5.15  BV=5.74
    {2255,1136,1024, 0, 0, 0},  //TV = 8.79(152 lines)  AV=2.00  SV=5.15  BV=5.64
    {2389,1136,1024, 0, 0, 0},  //TV = 8.71(161 lines)  AV=2.00  SV=5.15  BV=5.56
    {2582,1136,1024, 0, 0, 0},  //TV = 8.60(174 lines)  AV=2.00  SV=5.15  BV=5.45
    {2774,1136,1024, 0, 0, 0},  //TV = 8.49(187 lines)  AV=2.00  SV=5.15  BV=5.34
    {2967,1136,1024, 0, 0, 0},  //TV = 8.40(200 lines)  AV=2.00  SV=5.15  BV=5.25
    {3175,1136,1024, 0, 0, 0},  //TV = 8.30(214 lines)  AV=2.00  SV=5.15  BV=5.15
    {3397,1136,1024, 0, 0, 0},  //TV = 8.20(229 lines)  AV=2.00  SV=5.15  BV=5.05
    {3635,1136,1024, 0, 0, 0},  //TV = 8.10(245 lines)  AV=2.00  SV=5.15  BV=4.95
    {3932,1136,1024, 0, 0, 0},  //TV = 7.99(265 lines)  AV=2.00  SV=5.15  BV=4.84
    {4199,1136,1024, 0, 0, 0},  //TV = 7.90(283 lines)  AV=2.00  SV=5.15  BV=4.75
    {4495,1136,1024, 0, 0, 0},  //TV = 7.80(303 lines)  AV=2.00  SV=5.15  BV=4.65
    {4822,1136,1024, 0, 0, 0},  //TV = 7.70(325 lines)  AV=2.00  SV=5.15  BV=4.55
    {5163,1136,1024, 0, 0, 0},  //TV = 7.60(348 lines)  AV=2.00  SV=5.15  BV=4.45
    {5563,1136,1024, 0, 0, 0},  //TV = 7.49(375 lines)  AV=2.00  SV=5.15  BV=4.34
    {5949,1136,1024, 0, 0, 0},  //TV = 7.39(401 lines)  AV=2.00  SV=5.15  BV=4.24
    {6379,1136,1024, 0, 0, 0},  //TV = 7.29(430 lines)  AV=2.00  SV=5.15  BV=4.14
    {6824,1136,1024, 0, 0, 0},  //TV = 7.20(460 lines)  AV=2.00  SV=5.15  BV=4.05
    {7314,1136,1024, 0, 0, 0},  //TV = 7.10(493 lines)  AV=2.00  SV=5.15  BV=3.95
    {7833,1136,1024, 0, 0, 0},  //TV = 7.00(528 lines)  AV=2.00  SV=5.15  BV=3.85
    {8397,1136,1024, 0, 0, 0},  //TV = 6.90(566 lines)  AV=2.00  SV=5.15  BV=3.75
    {8990,1136,1024, 0, 0, 0},  //TV = 6.80(606 lines)  AV=2.00  SV=5.15  BV=3.65
    {9628,1136,1024, 0, 0, 0},  //TV = 6.70(649 lines)  AV=2.00  SV=5.15  BV=3.55
    {9999,1152,1040, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.19  BV=3.45
    {9999,1152,1112, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.29  BV=3.36
    {9999,1280,1080, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.40  BV=3.25
    {9999,1408,1056, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.50  BV=3.14
    {9999,1536,1032, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.60  BV=3.05
    {9999,1664,1024, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.70  BV=2.94
    {9999,1664,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.79  BV=2.86
    {9999,1792,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.89  BV=2.75
    {9999,1920,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.99  BV=2.65
    {9999,2176,1032, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=6.10  BV=2.55
    {19997,1152,1040, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.19  BV=2.45
    {19997,1152,1112, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.29  BV=2.36
    {19997,1280,1080, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.40  BV=2.25
    {19997,1408,1048, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.49  BV=2.15
    {19997,1536,1032, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.60  BV=2.05
    {19997,1664,1024, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.70  BV=1.94
    {29995,1152,1048, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.20  BV=1.86
    {29995,1152,1128, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.31  BV=1.75
    {29995,1280,1088, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.41  BV=1.65
    {29995,1408,1064, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.51  BV=1.54
    {29995,1536,1040, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.61  BV=1.45
    {29995,1664,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.71  BV=1.35
    {29995,1792,1024, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.81  BV=1.25
    {29995,1920,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.92  BV=1.14
    {29995,2048,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.01  BV=1.05
    {29995,2176,1040, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.11  BV=0.95
    {29995,2304,1056, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.21  BV=0.84
    {40008,1792,1088, 0, 0, 0},  //TV = 4.64(2697 lines)  AV=2.00  SV=5.89  BV=0.75
    {40008,1920,1088, 0, 0, 0},  //TV = 4.64(2697 lines)  AV=2.00  SV=5.99  BV=0.65
    {40008,2176,1024, 0, 0, 0},  //TV = 4.64(2697 lines)  AV=2.00  SV=6.09  BV=0.56
    {50006,1792,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.87  BV=0.45
    {50006,1920,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.97  BV=0.35
    {50006,2048,1080, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.08  BV=0.24
    {50006,2304,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.17  BV=0.15
    {50006,2432,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.27  BV=0.05
    {50006,2560,1064, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.38  BV=-0.06
    {50006,2816,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.47  BV=-0.15
    {50006,2944,1056, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.57  BV=-0.25
    {50006,3200,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.67  BV=-0.34
    {50006,3456,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.78  BV=-0.46
    {50006,3712,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.87  BV=-0.55
    {50006,3968,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.97  BV=-0.64
    {50006,4224,1048, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.08  BV=-0.76
    {60004,3840,1024, 0, 0, 0},  //TV = 4.06(4045 lines)  AV=2.00  SV=6.91  BV=-0.85
    {60004,4096,1032, 0, 0, 0},  //TV = 4.06(4045 lines)  AV=2.00  SV=7.01  BV=-0.95
    {70002,3712,1048, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=6.89  BV=-1.05
    {70002,3968,1048, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=6.99  BV=-1.15
    {70002,4352,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.09  BV=-1.25
    {70002,4608,1040, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.19  BV=-1.36
    {70002,4992,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.29  BV=-1.45
    {70002,5376,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.39  BV=-1.56
    {70002,5632,1040, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.48  BV=-1.65
    {70002,6144,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.58  BV=-1.75
    {70002,6528,1032, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.68  BV=-1.85
    {70002,7040,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.78  BV=-1.94
    {70002,7552,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.88  BV=-2.05
    {70002,8064,1032, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.99  BV=-2.15
    {70002,8704,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=8.09  BV=-2.25
    {80000,8064,1032, 0, 0, 0},  //TV = 3.64(5393 lines)  AV=2.00  SV=7.99  BV=-2.34
    {89998,7680,1032, 0, 0, 0},  //TV = 3.47(6067 lines)  AV=2.00  SV=7.92  BV=-2.44
    {89998,8320,1024, 0, 0, 0},  //TV = 3.47(6067 lines)  AV=2.00  SV=8.02  BV=-2.55
    {99996,7936,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=7.97  BV=-2.64
    {99996,8576,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.08  BV=-2.76
    {99996,9216,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.17  BV=-2.85
    {99996,9856,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.27  BV=-2.94
    {99996,10624,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.38  BV=-3.05
    {99996,11392,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.48  BV=-3.15
    {99996,12160,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.57  BV=-3.25
    {99996,13056,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.67  BV=-3.35
    {99996,13952,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.77  BV=-3.45
    {99996,14976,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.87  BV=-3.55
    {99996,16000,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.97  BV=-3.64
    {99996,16384,1024, 0, 0, 0},  /* TV = 3.32(6741 lines)  AV=2.00  SV=9.00  BV=-3.68 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
},
};

static strAETable g_AE_Custom3AutoTable =
{
    AETABLE_CUSTOM3_AUTO, //eAETableID
    156, //u4TotalIndex
    -20, //i4StrobeTrigerBV
    118, //i4MaxBV
    -37, //i4MinBV
    90, //i4EffectiveMaxBV
    0, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_AUTO, //ISO
    sCustom3PLineTable_60Hz,
    sCustom3PLineTable_50Hz,
    NULL,
};
static strEvPline sCustom4PLineTable_60Hz =
{
{
    {30,1152,1104, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.28  BV=11.75
    {30,1280,1056, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.37  BV=11.66
    {30,1408,1040, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.48  BV=11.54
    {30,1536,1032, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.60  BV=11.43
    {30,1664,1024, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.70  BV=11.32
    {45,1152,1048, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.20  BV=11.24
    {45,1152,1120, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.30  BV=11.14
    {45,1280,1088, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.41  BV=11.03
    {45,1408,1040, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.48  BV=10.96
    {60,1152,1056, 0, 0, 0},  /* TV = 14.02(4 lines)  AV=2.00  SV=5.21  BV=10.81 */
    {60,1152,1128, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.31  BV=10.72
    {60,1280,1072, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.39  BV=10.64
    {75,1152,1032, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.18  BV=10.52
    {75,1152,1088, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.26  BV=10.45
    {75,1280,1056, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.37  BV=10.34
    {90,1152,1040, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.19  BV=10.25
    {90,1280,1024, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.32  BV=10.12
    {104,1152,1040, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.19  BV=10.04
    {104,1152,1104, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.28  BV=9.95
    {119,1152,1032, 0, 0, 0},  //TV = 13.04(8 lines)  AV=2.00  SV=5.18  BV=9.86
    {134,1136,1024, 0, 0, 0},  /* TV = 12.87(9 lines)  AV=2.00  SV=5.15  BV=9.72 */
    {134,1152,1064, 0, 0, 0},  //TV = 12.87(9 lines)  AV=2.00  SV=5.23  BV=9.64
    {149,1152,1032, 0, 0, 0},  //TV = 12.71(10 lines)  AV=2.00  SV=5.18  BV=9.53
    {164,1136,1024, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.15  BV=9.42
    {164,1152,1088, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.26  BV=9.32
    {179,1152,1040, 0, 0, 0},  /* TV = 12.45(12 lines)  AV=2.00  SV=5.19  BV=9.26 */
    {193,1152,1032, 0, 0, 0},  //TV = 12.34(13 lines)  AV=2.00  SV=5.18  BV=9.16
    {208,1152,1032, 0, 0, 0},  //TV = 12.23(14 lines)  AV=2.00  SV=5.18  BV=9.05
    {223,1152,1032, 0, 0, 0},  //TV = 12.13(15 lines)  AV=2.00  SV=5.18  BV=8.95
    {238,1152,1032, 0, 0, 0},  //TV = 12.04(16 lines)  AV=2.00  SV=5.18  BV=8.86
    {253,1152,1040, 0, 0, 0},  //TV = 11.95(17 lines)  AV=2.00  SV=5.19  BV=8.76
    {268,1152,1056, 0, 0, 0},  //TV = 11.87(18 lines)  AV=2.00  SV=5.21  BV=8.65
    {297,1152,1024, 0, 0, 0},  //TV = 11.72(20 lines)  AV=2.00  SV=5.17  BV=8.55
    {312,1152,1040, 0, 0, 0},  //TV = 11.65(21 lines)  AV=2.00  SV=5.19  BV=8.45
    {342,1152,1024, 0, 0, 0},  //TV = 11.51(23 lines)  AV=2.00  SV=5.17  BV=8.34
    {371,1136,1024, 0, 0, 0},  //TV = 11.40(25 lines)  AV=2.00  SV=5.15  BV=8.25
    {386,1152,1032, 0, 0, 0},  //TV = 11.34(26 lines)  AV=2.00  SV=5.18  BV=8.16
    {416,1152,1032, 0, 0, 0},  //TV = 11.23(28 lines)  AV=2.00  SV=5.18  BV=8.05
    {446,1152,1032, 0, 0, 0},  //TV = 11.13(30 lines)  AV=2.00  SV=5.18  BV=7.95
    {475,1152,1032, 0, 0, 0},  //TV = 11.04(32 lines)  AV=2.00  SV=5.18  BV=7.86
    {520,1136,1024, 0, 0, 0},  //TV = 10.91(35 lines)  AV=2.00  SV=5.15  BV=7.76
    {564,1136,1024, 0, 0, 0},  //TV = 10.79(38 lines)  AV=2.00  SV=5.15  BV=7.64
    {594,1152,1024, 0, 0, 0},  //TV = 10.72(40 lines)  AV=2.00  SV=5.17  BV=7.55
    {638,1152,1024, 0, 0, 0},  //TV = 10.61(43 lines)  AV=2.00  SV=5.17  BV=7.44
    {683,1152,1024, 0, 0, 0},  //TV = 10.52(46 lines)  AV=2.00  SV=5.17  BV=7.35
    {727,1152,1024, 0, 0, 0},  //TV = 10.43(49 lines)  AV=2.00  SV=5.17  BV=7.26
    {787,1152,1024, 0, 0, 0},  //TV = 10.31(53 lines)  AV=2.00  SV=5.17  BV=7.14
    {846,1136,1024, 0, 0, 0},  //TV = 10.21(57 lines)  AV=2.00  SV=5.15  BV=7.06
    {905,1136,1024, 0, 0, 0},  //TV = 10.11(61 lines)  AV=2.00  SV=5.15  BV=6.96
    {965,1152,1024, 0, 0, 0},  //TV = 10.02(65 lines)  AV=2.00  SV=5.17  BV=6.85
    {1039,1152,1024, 0, 0, 0},  //TV = 9.91(70 lines)  AV=2.00  SV=5.17  BV=6.74
    {1128,1136,1024, 0, 0, 0},  //TV = 9.79(76 lines)  AV=2.00  SV=5.15  BV=6.64
    {1202,1136,1024, 0, 0, 0},  //TV = 9.70(81 lines)  AV=2.00  SV=5.15  BV=6.55
    {1291,1136,1024, 0, 0, 0},  //TV = 9.60(87 lines)  AV=2.00  SV=5.15  BV=6.45
    {1380,1136,1024, 0, 0, 0},  //TV = 9.50(93 lines)  AV=2.00  SV=5.15  BV=6.35
    {1484,1136,1024, 0, 0, 0},  //TV = 9.40(100 lines)  AV=2.00  SV=5.15  BV=6.25
    {1588,1136,1024, 0, 0, 0},  //TV = 9.30(107 lines)  AV=2.00  SV=5.15  BV=6.15
    {1692,1136,1024, 0, 0, 0},  //TV = 9.21(114 lines)  AV=2.00  SV=5.15  BV=6.06
    {1810,1136,1024, 0, 0, 0},  //TV = 9.11(122 lines)  AV=2.00  SV=5.15  BV=5.96
    {1959,1136,1024, 0, 0, 0},  //TV = 9.00(132 lines)  AV=2.00  SV=5.15  BV=5.85
    {2107,1136,1024, 0, 0, 0},  //TV = 8.89(142 lines)  AV=2.00  SV=5.15  BV=5.74
    {2255,1136,1024, 0, 0, 0},  //TV = 8.79(152 lines)  AV=2.00  SV=5.15  BV=5.64
    {2389,1136,1024, 0, 0, 0},  //TV = 8.71(161 lines)  AV=2.00  SV=5.15  BV=5.56
    {2582,1136,1024, 0, 0, 0},  //TV = 8.60(174 lines)  AV=2.00  SV=5.15  BV=5.45
    {2774,1136,1024, 0, 0, 0},  //TV = 8.49(187 lines)  AV=2.00  SV=5.15  BV=5.34
    {2967,1136,1024, 0, 0, 0},  //TV = 8.40(200 lines)  AV=2.00  SV=5.15  BV=5.25
    {3175,1136,1024, 0, 0, 0},  //TV = 8.30(214 lines)  AV=2.00  SV=5.15  BV=5.15
    {3397,1136,1024, 0, 0, 0},  //TV = 8.20(229 lines)  AV=2.00  SV=5.15  BV=5.05
    {3635,1136,1024, 0, 0, 0},  //TV = 8.10(245 lines)  AV=2.00  SV=5.15  BV=4.95
    {3932,1136,1024, 0, 0, 0},  //TV = 7.99(265 lines)  AV=2.00  SV=5.15  BV=4.84
    {4199,1136,1024, 0, 0, 0},  //TV = 7.90(283 lines)  AV=2.00  SV=5.15  BV=4.75
    {4495,1136,1024, 0, 0, 0},  //TV = 7.80(303 lines)  AV=2.00  SV=5.15  BV=4.65
    {4822,1136,1024, 0, 0, 0},  //TV = 7.70(325 lines)  AV=2.00  SV=5.15  BV=4.55
    {5163,1136,1024, 0, 0, 0},  //TV = 7.60(348 lines)  AV=2.00  SV=5.15  BV=4.45
    {5563,1136,1024, 0, 0, 0},  //TV = 7.49(375 lines)  AV=2.00  SV=5.15  BV=4.34
    {5949,1136,1024, 0, 0, 0},  //TV = 7.39(401 lines)  AV=2.00  SV=5.15  BV=4.24
    {6379,1136,1024, 0, 0, 0},  //TV = 7.29(430 lines)  AV=2.00  SV=5.15  BV=4.14
    {6824,1136,1024, 0, 0, 0},  //TV = 7.20(460 lines)  AV=2.00  SV=5.15  BV=4.05
    {7314,1136,1024, 0, 0, 0},  //TV = 7.10(493 lines)  AV=2.00  SV=5.15  BV=3.95
    {7833,1136,1024, 0, 0, 0},  //TV = 7.00(528 lines)  AV=2.00  SV=5.15  BV=3.85
    {8337,1136,1024, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.15  BV=3.76
    {8337,1152,1088, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.26  BV=3.65
    {8337,1280,1048, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.36  BV=3.55
    {8337,1408,1024, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.46  BV=3.45
    {8337,1408,1104, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.57  BV=3.34
    {8337,1536,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.65  BV=3.26
    {8337,1664,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.77  BV=3.14
    {8337,1792,1056, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.85  BV=3.05
    {8337,1920,1064, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.96  BV=2.94
    {8337,2048,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=6.07  BV=2.84
    {16659,1136,1024, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.15  BV=2.76
    {16659,1152,1088, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.26  BV=2.65
    {16659,1280,1056, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.37  BV=2.54
    {16659,1408,1024, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.46  BV=2.45
    {16659,1408,1096, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.56  BV=2.35
    {16659,1536,1080, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.66  BV=2.25
    {24996,1152,1024, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.17  BV=2.15
    {24996,1152,1096, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.27  BV=2.05
    {24996,1280,1056, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.37  BV=1.96
    {24996,1408,1032, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.47  BV=1.85
    {33332,1136,1024, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.15  BV=1.76
    {33332,1152,1088, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.26  BV=1.65
    {33332,1280,1048, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.36  BV=1.55
    {33332,1408,1024, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.46  BV=1.45
    {33332,1408,1096, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.56  BV=1.35
    {33332,1536,1072, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.65  BV=1.26
    {33332,1664,1072, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.77  BV=1.14
    {33332,1792,1064, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.86  BV=1.04
    {33332,1920,1056, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.95  BV=0.96
    {33332,2048,1064, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=6.06  BV=0.85
    {33332,2176,1072, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=6.15  BV=0.75
    {41669,1920,1040, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=5.93  BV=0.66
    {41669,2048,1048, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=6.03  BV=0.55
    {41669,2176,1056, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=6.13  BV=0.45
    {50006,1920,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.97  BV=0.35
    {50006,2048,1080, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.08  BV=0.24
    {50006,2304,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.17  BV=0.15
    {50006,2432,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.27  BV=0.05
    {50006,2560,1064, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.38  BV=-0.06
    {50006,2816,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.47  BV=-0.15
    {50006,2944,1056, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.57  BV=-0.25
    {50006,3200,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.67  BV=-0.34
    {50006,3456,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.78  BV=-0.46
    {50006,3712,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.87  BV=-0.55
    {50006,3968,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.97  BV=-0.64
    {50006,4224,1048, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.08  BV=-0.76
    {58328,3968,1024, 0, 0, 0},  //TV = 4.10(3932 lines)  AV=2.00  SV=6.95  BV=-0.85
    {58328,4224,1032, 0, 0, 0},  //TV = 4.10(3932 lines)  AV=2.00  SV=7.06  BV=-0.96
    {66664,3968,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=6.97  BV=-1.06
    {66664,4224,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.06  BV=-1.15
    {66664,4480,1040, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.15  BV=-1.24
    {66664,4864,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.26  BV=-1.35
    {66664,5248,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.36  BV=-1.45
    {66664,5632,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.46  BV=-1.55
    {66664,6016,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.55  BV=-1.65
    {66664,6400,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.66  BV=-1.75
    {66664,6912,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.75  BV=-1.85
    {66664,7424,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.86  BV=-1.95
    {66664,7936,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.95  BV=-2.05
    {66664,8448,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=8.06  BV=-2.15
    {75001,8064,1032, 0, 0, 0},  //TV = 3.74(5056 lines)  AV=2.00  SV=7.99  BV=-2.25
    {83338,7808,1024, 0, 0, 0},  //TV = 3.58(5618 lines)  AV=2.00  SV=7.93  BV=-2.35
    {83338,8320,1032, 0, 0, 0},  //TV = 3.58(5618 lines)  AV=2.00  SV=8.03  BV=-2.45
    {91660,8192,1024, 0, 0, 0},  //TV = 3.45(6179 lines)  AV=2.00  SV=8.00  BV=-2.55
    {99996,7936,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=7.97  BV=-2.64
    {99996,8576,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.08  BV=-2.76
    {99996,9216,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.17  BV=-2.85
    {99996,9856,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.27  BV=-2.94
    {99996,10624,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.38  BV=-3.05
    {99996,11392,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.48  BV=-3.15
    {99996,12160,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.57  BV=-3.25
    {99996,13056,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.67  BV=-3.35
    {99996,13952,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.77  BV=-3.45
    {99996,14976,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.87  BV=-3.55
    {99996,16000,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.97  BV=-3.64
    {99996,16384,1024, 0, 0, 0},  /* TV = 3.32(6741 lines)  AV=2.00  SV=9.00  BV=-3.68 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
},
};

static strEvPline sCustom4PLineTable_50Hz =
{
{
    {30,1152,1104, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.28  BV=11.75
    {30,1280,1056, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.37  BV=11.66
    {30,1408,1040, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.48  BV=11.54
    {30,1536,1032, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.60  BV=11.43
    {30,1664,1024, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.70  BV=11.32
    {45,1152,1048, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.20  BV=11.24
    {45,1152,1120, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.30  BV=11.14
    {45,1280,1088, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.41  BV=11.03
    {45,1408,1040, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.48  BV=10.96
    {60,1152,1056, 0, 0, 0},  /* TV = 14.02(4 lines)  AV=2.00  SV=5.21  BV=10.81 */
    {60,1152,1128, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.31  BV=10.72
    {60,1280,1072, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.39  BV=10.64
    {75,1152,1032, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.18  BV=10.52
    {75,1152,1088, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.26  BV=10.45
    {75,1280,1056, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.37  BV=10.34
    {90,1152,1040, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.19  BV=10.25
    {90,1280,1024, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.32  BV=10.12
    {104,1152,1040, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.19  BV=10.04
    {104,1152,1104, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.28  BV=9.95
    {119,1152,1032, 0, 0, 0},  //TV = 13.04(8 lines)  AV=2.00  SV=5.18  BV=9.86
    {134,1136,1024, 0, 0, 0},  /* TV = 12.87(9 lines)  AV=2.00  SV=5.15  BV=9.72 */
    {134,1152,1064, 0, 0, 0},  //TV = 12.87(9 lines)  AV=2.00  SV=5.23  BV=9.64
    {149,1152,1032, 0, 0, 0},  //TV = 12.71(10 lines)  AV=2.00  SV=5.18  BV=9.53
    {164,1136,1024, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.15  BV=9.42
    {164,1152,1088, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.26  BV=9.32
    {179,1152,1040, 0, 0, 0},  /* TV = 12.45(12 lines)  AV=2.00  SV=5.19  BV=9.26 */
    {193,1152,1032, 0, 0, 0},  //TV = 12.34(13 lines)  AV=2.00  SV=5.18  BV=9.16
    {208,1152,1032, 0, 0, 0},  //TV = 12.23(14 lines)  AV=2.00  SV=5.18  BV=9.05
    {223,1152,1032, 0, 0, 0},  //TV = 12.13(15 lines)  AV=2.00  SV=5.18  BV=8.95
    {238,1152,1032, 0, 0, 0},  //TV = 12.04(16 lines)  AV=2.00  SV=5.18  BV=8.86
    {253,1152,1040, 0, 0, 0},  //TV = 11.95(17 lines)  AV=2.00  SV=5.19  BV=8.76
    {268,1152,1056, 0, 0, 0},  //TV = 11.87(18 lines)  AV=2.00  SV=5.21  BV=8.65
    {297,1152,1024, 0, 0, 0},  //TV = 11.72(20 lines)  AV=2.00  SV=5.17  BV=8.55
    {312,1152,1040, 0, 0, 0},  //TV = 11.65(21 lines)  AV=2.00  SV=5.19  BV=8.45
    {342,1152,1024, 0, 0, 0},  //TV = 11.51(23 lines)  AV=2.00  SV=5.17  BV=8.34
    {371,1136,1024, 0, 0, 0},  //TV = 11.40(25 lines)  AV=2.00  SV=5.15  BV=8.25
    {386,1152,1032, 0, 0, 0},  //TV = 11.34(26 lines)  AV=2.00  SV=5.18  BV=8.16
    {416,1152,1032, 0, 0, 0},  //TV = 11.23(28 lines)  AV=2.00  SV=5.18  BV=8.05
    {446,1152,1032, 0, 0, 0},  //TV = 11.13(30 lines)  AV=2.00  SV=5.18  BV=7.95
    {475,1152,1032, 0, 0, 0},  //TV = 11.04(32 lines)  AV=2.00  SV=5.18  BV=7.86
    {520,1136,1024, 0, 0, 0},  //TV = 10.91(35 lines)  AV=2.00  SV=5.15  BV=7.76
    {564,1136,1024, 0, 0, 0},  //TV = 10.79(38 lines)  AV=2.00  SV=5.15  BV=7.64
    {594,1152,1024, 0, 0, 0},  //TV = 10.72(40 lines)  AV=2.00  SV=5.17  BV=7.55
    {638,1152,1024, 0, 0, 0},  //TV = 10.61(43 lines)  AV=2.00  SV=5.17  BV=7.44
    {683,1152,1024, 0, 0, 0},  //TV = 10.52(46 lines)  AV=2.00  SV=5.17  BV=7.35
    {727,1152,1024, 0, 0, 0},  //TV = 10.43(49 lines)  AV=2.00  SV=5.17  BV=7.26
    {787,1152,1024, 0, 0, 0},  //TV = 10.31(53 lines)  AV=2.00  SV=5.17  BV=7.14
    {846,1136,1024, 0, 0, 0},  //TV = 10.21(57 lines)  AV=2.00  SV=5.15  BV=7.06
    {905,1136,1024, 0, 0, 0},  //TV = 10.11(61 lines)  AV=2.00  SV=5.15  BV=6.96
    {965,1152,1024, 0, 0, 0},  //TV = 10.02(65 lines)  AV=2.00  SV=5.17  BV=6.85
    {1039,1152,1024, 0, 0, 0},  //TV = 9.91(70 lines)  AV=2.00  SV=5.17  BV=6.74
    {1128,1136,1024, 0, 0, 0},  //TV = 9.79(76 lines)  AV=2.00  SV=5.15  BV=6.64
    {1202,1136,1024, 0, 0, 0},  //TV = 9.70(81 lines)  AV=2.00  SV=5.15  BV=6.55
    {1291,1136,1024, 0, 0, 0},  //TV = 9.60(87 lines)  AV=2.00  SV=5.15  BV=6.45
    {1380,1136,1024, 0, 0, 0},  //TV = 9.50(93 lines)  AV=2.00  SV=5.15  BV=6.35
    {1484,1136,1024, 0, 0, 0},  //TV = 9.40(100 lines)  AV=2.00  SV=5.15  BV=6.25
    {1588,1136,1024, 0, 0, 0},  //TV = 9.30(107 lines)  AV=2.00  SV=5.15  BV=6.15
    {1692,1136,1024, 0, 0, 0},  //TV = 9.21(114 lines)  AV=2.00  SV=5.15  BV=6.06
    {1810,1136,1024, 0, 0, 0},  //TV = 9.11(122 lines)  AV=2.00  SV=5.15  BV=5.96
    {1959,1136,1024, 0, 0, 0},  //TV = 9.00(132 lines)  AV=2.00  SV=5.15  BV=5.85
    {2107,1136,1024, 0, 0, 0},  //TV = 8.89(142 lines)  AV=2.00  SV=5.15  BV=5.74
    {2255,1136,1024, 0, 0, 0},  //TV = 8.79(152 lines)  AV=2.00  SV=5.15  BV=5.64
    {2389,1136,1024, 0, 0, 0},  //TV = 8.71(161 lines)  AV=2.00  SV=5.15  BV=5.56
    {2582,1136,1024, 0, 0, 0},  //TV = 8.60(174 lines)  AV=2.00  SV=5.15  BV=5.45
    {2774,1136,1024, 0, 0, 0},  //TV = 8.49(187 lines)  AV=2.00  SV=5.15  BV=5.34
    {2967,1136,1024, 0, 0, 0},  //TV = 8.40(200 lines)  AV=2.00  SV=5.15  BV=5.25
    {3175,1136,1024, 0, 0, 0},  //TV = 8.30(214 lines)  AV=2.00  SV=5.15  BV=5.15
    {3397,1136,1024, 0, 0, 0},  //TV = 8.20(229 lines)  AV=2.00  SV=5.15  BV=5.05
    {3635,1136,1024, 0, 0, 0},  //TV = 8.10(245 lines)  AV=2.00  SV=5.15  BV=4.95
    {3932,1136,1024, 0, 0, 0},  //TV = 7.99(265 lines)  AV=2.00  SV=5.15  BV=4.84
    {4199,1136,1024, 0, 0, 0},  //TV = 7.90(283 lines)  AV=2.00  SV=5.15  BV=4.75
    {4495,1136,1024, 0, 0, 0},  //TV = 7.80(303 lines)  AV=2.00  SV=5.15  BV=4.65
    {4822,1136,1024, 0, 0, 0},  //TV = 7.70(325 lines)  AV=2.00  SV=5.15  BV=4.55
    {5163,1136,1024, 0, 0, 0},  //TV = 7.60(348 lines)  AV=2.00  SV=5.15  BV=4.45
    {5563,1136,1024, 0, 0, 0},  //TV = 7.49(375 lines)  AV=2.00  SV=5.15  BV=4.34
    {5949,1136,1024, 0, 0, 0},  //TV = 7.39(401 lines)  AV=2.00  SV=5.15  BV=4.24
    {6379,1136,1024, 0, 0, 0},  //TV = 7.29(430 lines)  AV=2.00  SV=5.15  BV=4.14
    {6824,1136,1024, 0, 0, 0},  //TV = 7.20(460 lines)  AV=2.00  SV=5.15  BV=4.05
    {7314,1136,1024, 0, 0, 0},  //TV = 7.10(493 lines)  AV=2.00  SV=5.15  BV=3.95
    {7833,1136,1024, 0, 0, 0},  //TV = 7.00(528 lines)  AV=2.00  SV=5.15  BV=3.85
    {8397,1136,1024, 0, 0, 0},  //TV = 6.90(566 lines)  AV=2.00  SV=5.15  BV=3.75
    {8990,1136,1024, 0, 0, 0},  //TV = 6.80(606 lines)  AV=2.00  SV=5.15  BV=3.65
    {9628,1136,1024, 0, 0, 0},  //TV = 6.70(649 lines)  AV=2.00  SV=5.15  BV=3.55
    {9999,1152,1040, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.19  BV=3.45
    {9999,1152,1112, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.29  BV=3.36
    {9999,1280,1080, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.40  BV=3.25
    {9999,1408,1056, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.50  BV=3.14
    {9999,1536,1032, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.60  BV=3.05
    {9999,1664,1024, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.70  BV=2.94
    {9999,1664,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.79  BV=2.86
    {9999,1792,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.89  BV=2.75
    {9999,1920,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.99  BV=2.65
    {9999,2176,1032, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=6.10  BV=2.55
    {19997,1152,1040, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.19  BV=2.45
    {19997,1152,1112, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.29  BV=2.36
    {19997,1280,1080, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.40  BV=2.25
    {19997,1408,1048, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.49  BV=2.15
    {19997,1536,1032, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.60  BV=2.05
    {19997,1664,1024, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.70  BV=1.94
    {29995,1152,1048, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.20  BV=1.86
    {29995,1152,1128, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.31  BV=1.75
    {29995,1280,1088, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.41  BV=1.65
    {29995,1408,1064, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.51  BV=1.54
    {29995,1536,1040, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.61  BV=1.45
    {29995,1664,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.71  BV=1.35
    {29995,1792,1024, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.81  BV=1.25
    {29995,1920,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.92  BV=1.14
    {29995,2048,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.01  BV=1.05
    {29995,2176,1040, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.11  BV=0.95
    {29995,2304,1056, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.21  BV=0.84
    {40008,1792,1088, 0, 0, 0},  //TV = 4.64(2697 lines)  AV=2.00  SV=5.89  BV=0.75
    {40008,1920,1088, 0, 0, 0},  //TV = 4.64(2697 lines)  AV=2.00  SV=5.99  BV=0.65
    {40008,2176,1024, 0, 0, 0},  //TV = 4.64(2697 lines)  AV=2.00  SV=6.09  BV=0.56
    {50006,1792,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.87  BV=0.45
    {50006,1920,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.97  BV=0.35
    {50006,2048,1080, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.08  BV=0.24
    {50006,2304,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.17  BV=0.15
    {50006,2432,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.27  BV=0.05
    {50006,2560,1064, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.38  BV=-0.06
    {50006,2816,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.47  BV=-0.15
    {50006,2944,1056, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.57  BV=-0.25
    {50006,3200,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.67  BV=-0.34
    {50006,3456,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.78  BV=-0.46
    {50006,3712,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.87  BV=-0.55
    {50006,3968,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.97  BV=-0.64
    {50006,4224,1048, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.08  BV=-0.76
    {60004,3840,1024, 0, 0, 0},  //TV = 4.06(4045 lines)  AV=2.00  SV=6.91  BV=-0.85
    {60004,4096,1032, 0, 0, 0},  //TV = 4.06(4045 lines)  AV=2.00  SV=7.01  BV=-0.95
    {70002,3712,1048, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=6.89  BV=-1.05
    {70002,3968,1048, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=6.99  BV=-1.15
    {70002,4352,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.09  BV=-1.25
    {70002,4608,1040, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.19  BV=-1.36
    {70002,4992,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.29  BV=-1.45
    {70002,5376,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.39  BV=-1.56
    {70002,5632,1040, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.48  BV=-1.65
    {70002,6144,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.58  BV=-1.75
    {70002,6528,1032, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.68  BV=-1.85
    {70002,7040,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.78  BV=-1.94
    {70002,7552,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.88  BV=-2.05
    {70002,8064,1032, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.99  BV=-2.15
    {70002,8704,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=8.09  BV=-2.25
    {80000,8064,1032, 0, 0, 0},  //TV = 3.64(5393 lines)  AV=2.00  SV=7.99  BV=-2.34
    {89998,7680,1032, 0, 0, 0},  //TV = 3.47(6067 lines)  AV=2.00  SV=7.92  BV=-2.44
    {89998,8320,1024, 0, 0, 0},  //TV = 3.47(6067 lines)  AV=2.00  SV=8.02  BV=-2.55
    {99996,7936,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=7.97  BV=-2.64
    {99996,8576,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.08  BV=-2.76
    {99996,9216,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.17  BV=-2.85
    {99996,9856,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.27  BV=-2.94
    {99996,10624,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.38  BV=-3.05
    {99996,11392,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.48  BV=-3.15
    {99996,12160,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.57  BV=-3.25
    {99996,13056,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.67  BV=-3.35
    {99996,13952,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.77  BV=-3.45
    {99996,14976,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.87  BV=-3.55
    {99996,16000,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.97  BV=-3.64
    {99996,16384,1024, 0, 0, 0},  /* TV = 3.32(6741 lines)  AV=2.00  SV=9.00  BV=-3.68 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
},
};

static strAETable g_AE_Custom4AutoTable =
{
    AETABLE_CUSTOM4_AUTO, //eAETableID
    156, //u4TotalIndex
    -20, //i4StrobeTrigerBV
    118, //i4MaxBV
    -37, //i4MinBV
    90, //i4EffectiveMaxBV
    0, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_AUTO, //ISO
    sCustom4PLineTable_60Hz,
    sCustom4PLineTable_50Hz,
    NULL,
};
static strEvPline sCustom5PLineTable_60Hz =
{
{
    {30,1152,1104, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.28  BV=11.75
    {30,1280,1056, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.37  BV=11.66
    {30,1408,1040, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.48  BV=11.54
    {30,1536,1032, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.60  BV=11.43
    {30,1664,1024, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.70  BV=11.32
    {45,1152,1048, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.20  BV=11.24
    {45,1152,1120, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.30  BV=11.14
    {45,1280,1088, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.41  BV=11.03
    {45,1408,1040, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.48  BV=10.96
    {60,1152,1056, 0, 0, 0},  /* TV = 14.02(4 lines)  AV=2.00  SV=5.21  BV=10.81 */
    {60,1152,1128, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.31  BV=10.72
    {60,1280,1072, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.39  BV=10.64
    {75,1152,1032, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.18  BV=10.52
    {75,1152,1088, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.26  BV=10.45
    {75,1280,1056, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.37  BV=10.34
    {90,1152,1040, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.19  BV=10.25
    {90,1280,1024, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.32  BV=10.12
    {104,1152,1040, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.19  BV=10.04
    {104,1152,1104, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.28  BV=9.95
    {119,1152,1032, 0, 0, 0},  //TV = 13.04(8 lines)  AV=2.00  SV=5.18  BV=9.86
    {134,1136,1024, 0, 0, 0},  /* TV = 12.87(9 lines)  AV=2.00  SV=5.15  BV=9.72 */
    {134,1152,1064, 0, 0, 0},  //TV = 12.87(9 lines)  AV=2.00  SV=5.23  BV=9.64
    {149,1152,1032, 0, 0, 0},  //TV = 12.71(10 lines)  AV=2.00  SV=5.18  BV=9.53
    {164,1136,1024, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.15  BV=9.42
    {164,1152,1088, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.26  BV=9.32
    {179,1152,1040, 0, 0, 0},  /* TV = 12.45(12 lines)  AV=2.00  SV=5.19  BV=9.26 */
    {193,1152,1032, 0, 0, 0},  //TV = 12.34(13 lines)  AV=2.00  SV=5.18  BV=9.16
    {208,1152,1032, 0, 0, 0},  //TV = 12.23(14 lines)  AV=2.00  SV=5.18  BV=9.05
    {223,1152,1032, 0, 0, 0},  //TV = 12.13(15 lines)  AV=2.00  SV=5.18  BV=8.95
    {238,1152,1032, 0, 0, 0},  //TV = 12.04(16 lines)  AV=2.00  SV=5.18  BV=8.86
    {253,1152,1040, 0, 0, 0},  //TV = 11.95(17 lines)  AV=2.00  SV=5.19  BV=8.76
    {268,1152,1056, 0, 0, 0},  //TV = 11.87(18 lines)  AV=2.00  SV=5.21  BV=8.65
    {297,1152,1024, 0, 0, 0},  //TV = 11.72(20 lines)  AV=2.00  SV=5.17  BV=8.55
    {312,1152,1040, 0, 0, 0},  //TV = 11.65(21 lines)  AV=2.00  SV=5.19  BV=8.45
    {342,1152,1024, 0, 0, 0},  //TV = 11.51(23 lines)  AV=2.00  SV=5.17  BV=8.34
    {371,1136,1024, 0, 0, 0},  //TV = 11.40(25 lines)  AV=2.00  SV=5.15  BV=8.25
    {386,1152,1032, 0, 0, 0},  //TV = 11.34(26 lines)  AV=2.00  SV=5.18  BV=8.16
    {416,1152,1032, 0, 0, 0},  //TV = 11.23(28 lines)  AV=2.00  SV=5.18  BV=8.05
    {446,1152,1032, 0, 0, 0},  //TV = 11.13(30 lines)  AV=2.00  SV=5.18  BV=7.95
    {475,1152,1032, 0, 0, 0},  //TV = 11.04(32 lines)  AV=2.00  SV=5.18  BV=7.86
    {520,1136,1024, 0, 0, 0},  //TV = 10.91(35 lines)  AV=2.00  SV=5.15  BV=7.76
    {564,1136,1024, 0, 0, 0},  //TV = 10.79(38 lines)  AV=2.00  SV=5.15  BV=7.64
    {594,1152,1024, 0, 0, 0},  //TV = 10.72(40 lines)  AV=2.00  SV=5.17  BV=7.55
    {638,1152,1024, 0, 0, 0},  //TV = 10.61(43 lines)  AV=2.00  SV=5.17  BV=7.44
    {683,1152,1024, 0, 0, 0},  //TV = 10.52(46 lines)  AV=2.00  SV=5.17  BV=7.35
    {727,1152,1024, 0, 0, 0},  //TV = 10.43(49 lines)  AV=2.00  SV=5.17  BV=7.26
    {787,1152,1024, 0, 0, 0},  //TV = 10.31(53 lines)  AV=2.00  SV=5.17  BV=7.14
    {846,1136,1024, 0, 0, 0},  //TV = 10.21(57 lines)  AV=2.00  SV=5.15  BV=7.06
    {905,1136,1024, 0, 0, 0},  //TV = 10.11(61 lines)  AV=2.00  SV=5.15  BV=6.96
    {965,1152,1024, 0, 0, 0},  //TV = 10.02(65 lines)  AV=2.00  SV=5.17  BV=6.85
    {1039,1152,1024, 0, 0, 0},  //TV = 9.91(70 lines)  AV=2.00  SV=5.17  BV=6.74
    {1128,1136,1024, 0, 0, 0},  //TV = 9.79(76 lines)  AV=2.00  SV=5.15  BV=6.64
    {1202,1136,1024, 0, 0, 0},  //TV = 9.70(81 lines)  AV=2.00  SV=5.15  BV=6.55
    {1291,1136,1024, 0, 0, 0},  //TV = 9.60(87 lines)  AV=2.00  SV=5.15  BV=6.45
    {1380,1136,1024, 0, 0, 0},  //TV = 9.50(93 lines)  AV=2.00  SV=5.15  BV=6.35
    {1484,1136,1024, 0, 0, 0},  //TV = 9.40(100 lines)  AV=2.00  SV=5.15  BV=6.25
    {1588,1136,1024, 0, 0, 0},  //TV = 9.30(107 lines)  AV=2.00  SV=5.15  BV=6.15
    {1692,1136,1024, 0, 0, 0},  //TV = 9.21(114 lines)  AV=2.00  SV=5.15  BV=6.06
    {1810,1136,1024, 0, 0, 0},  //TV = 9.11(122 lines)  AV=2.00  SV=5.15  BV=5.96
    {1959,1136,1024, 0, 0, 0},  //TV = 9.00(132 lines)  AV=2.00  SV=5.15  BV=5.85
    {2107,1136,1024, 0, 0, 0},  //TV = 8.89(142 lines)  AV=2.00  SV=5.15  BV=5.74
    {2255,1136,1024, 0, 0, 0},  //TV = 8.79(152 lines)  AV=2.00  SV=5.15  BV=5.64
    {2389,1136,1024, 0, 0, 0},  //TV = 8.71(161 lines)  AV=2.00  SV=5.15  BV=5.56
    {2582,1136,1024, 0, 0, 0},  //TV = 8.60(174 lines)  AV=2.00  SV=5.15  BV=5.45
    {2774,1136,1024, 0, 0, 0},  //TV = 8.49(187 lines)  AV=2.00  SV=5.15  BV=5.34
    {2967,1136,1024, 0, 0, 0},  //TV = 8.40(200 lines)  AV=2.00  SV=5.15  BV=5.25
    {3175,1136,1024, 0, 0, 0},  //TV = 8.30(214 lines)  AV=2.00  SV=5.15  BV=5.15
    {3397,1136,1024, 0, 0, 0},  //TV = 8.20(229 lines)  AV=2.00  SV=5.15  BV=5.05
    {3635,1136,1024, 0, 0, 0},  //TV = 8.10(245 lines)  AV=2.00  SV=5.15  BV=4.95
    {3932,1136,1024, 0, 0, 0},  //TV = 7.99(265 lines)  AV=2.00  SV=5.15  BV=4.84
    {4199,1136,1024, 0, 0, 0},  //TV = 7.90(283 lines)  AV=2.00  SV=5.15  BV=4.75
    {4495,1136,1024, 0, 0, 0},  //TV = 7.80(303 lines)  AV=2.00  SV=5.15  BV=4.65
    {4822,1136,1024, 0, 0, 0},  //TV = 7.70(325 lines)  AV=2.00  SV=5.15  BV=4.55
    {5163,1136,1024, 0, 0, 0},  //TV = 7.60(348 lines)  AV=2.00  SV=5.15  BV=4.45
    {5563,1136,1024, 0, 0, 0},  //TV = 7.49(375 lines)  AV=2.00  SV=5.15  BV=4.34
    {5949,1136,1024, 0, 0, 0},  //TV = 7.39(401 lines)  AV=2.00  SV=5.15  BV=4.24
    {6379,1136,1024, 0, 0, 0},  //TV = 7.29(430 lines)  AV=2.00  SV=5.15  BV=4.14
    {6824,1136,1024, 0, 0, 0},  //TV = 7.20(460 lines)  AV=2.00  SV=5.15  BV=4.05
    {7314,1136,1024, 0, 0, 0},  //TV = 7.10(493 lines)  AV=2.00  SV=5.15  BV=3.95
    {7833,1136,1024, 0, 0, 0},  //TV = 7.00(528 lines)  AV=2.00  SV=5.15  BV=3.85
    {8337,1136,1024, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.15  BV=3.76
    {8337,1152,1088, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.26  BV=3.65
    {8337,1280,1048, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.36  BV=3.55
    {8337,1408,1024, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.46  BV=3.45
    {8337,1408,1104, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.57  BV=3.34
    {8337,1536,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.65  BV=3.26
    {8337,1664,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.77  BV=3.14
    {8337,1792,1056, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.85  BV=3.05
    {8337,1920,1064, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.96  BV=2.94
    {8337,2048,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=6.07  BV=2.84
    {16659,1136,1024, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.15  BV=2.76
    {16659,1152,1088, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.26  BV=2.65
    {16659,1280,1056, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.37  BV=2.54
    {16659,1408,1024, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.46  BV=2.45
    {16659,1408,1096, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.56  BV=2.35
    {16659,1536,1080, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.66  BV=2.25
    {24996,1152,1024, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.17  BV=2.15
    {24996,1152,1096, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.27  BV=2.05
    {24996,1280,1056, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.37  BV=1.96
    {24996,1408,1032, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.47  BV=1.85
    {33332,1136,1024, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.15  BV=1.76
    {33332,1152,1088, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.26  BV=1.65
    {33332,1280,1048, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.36  BV=1.55
    {33332,1408,1024, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.46  BV=1.45
    {33332,1408,1096, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.56  BV=1.35
    {33332,1536,1072, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.65  BV=1.26
    {33332,1664,1072, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.77  BV=1.14
    {33332,1792,1064, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.86  BV=1.04
    {33332,1920,1056, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.95  BV=0.96
    {33332,2048,1064, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=6.06  BV=0.85
    {33332,2176,1072, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=6.15  BV=0.75
    {41669,1920,1040, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=5.93  BV=0.66
    {41669,2048,1048, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=6.03  BV=0.55
    {41669,2176,1056, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=6.13  BV=0.45
    {50006,1920,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.97  BV=0.35
    {50006,2048,1080, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.08  BV=0.24
    {50006,2304,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.17  BV=0.15
    {50006,2432,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.27  BV=0.05
    {50006,2560,1064, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.38  BV=-0.06
    {50006,2816,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.47  BV=-0.15
    {50006,2944,1056, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.57  BV=-0.25
    {50006,3200,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.67  BV=-0.34
    {50006,3456,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.78  BV=-0.46
    {50006,3712,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.87  BV=-0.55
    {50006,3968,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.97  BV=-0.64
    {50006,4224,1048, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.08  BV=-0.76
    {58328,3968,1024, 0, 0, 0},  //TV = 4.10(3932 lines)  AV=2.00  SV=6.95  BV=-0.85
    {58328,4224,1032, 0, 0, 0},  //TV = 4.10(3932 lines)  AV=2.00  SV=7.06  BV=-0.96
    {66664,3968,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=6.97  BV=-1.06
    {66664,4224,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.06  BV=-1.15
    {66664,4480,1040, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.15  BV=-1.24
    {66664,4864,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.26  BV=-1.35
    {66664,5248,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.36  BV=-1.45
    {66664,5632,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.46  BV=-1.55
    {66664,6016,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.55  BV=-1.65
    {66664,6400,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.66  BV=-1.75
    {66664,6912,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.75  BV=-1.85
    {66664,7424,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.86  BV=-1.95
    {66664,7936,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.95  BV=-2.05
    {66664,8448,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=8.06  BV=-2.15
    {75001,8064,1032, 0, 0, 0},  //TV = 3.74(5056 lines)  AV=2.00  SV=7.99  BV=-2.25
    {83338,7808,1024, 0, 0, 0},  //TV = 3.58(5618 lines)  AV=2.00  SV=7.93  BV=-2.35
    {83338,8320,1032, 0, 0, 0},  //TV = 3.58(5618 lines)  AV=2.00  SV=8.03  BV=-2.45
    {91660,8192,1024, 0, 0, 0},  //TV = 3.45(6179 lines)  AV=2.00  SV=8.00  BV=-2.55
    {99996,7936,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=7.97  BV=-2.64
    {99996,8576,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.08  BV=-2.76
    {99996,9216,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.17  BV=-2.85
    {99996,9856,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.27  BV=-2.94
    {99996,10624,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.38  BV=-3.05
    {99996,11392,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.48  BV=-3.15
    {99996,12160,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.57  BV=-3.25
    {99996,13056,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.67  BV=-3.35
    {99996,13952,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.77  BV=-3.45
    {99996,14976,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.87  BV=-3.55
    {99996,16000,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.97  BV=-3.64
    {99996,16384,1024, 0, 0, 0},  /* TV = 3.32(6741 lines)  AV=2.00  SV=9.00  BV=-3.68 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
},
};

static strEvPline sCustom5PLineTable_50Hz =
{
{
    {30,1152,1104, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.28  BV=11.75
    {30,1280,1056, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.37  BV=11.66
    {30,1408,1040, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.48  BV=11.54
    {30,1536,1032, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.60  BV=11.43
    {30,1664,1024, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.70  BV=11.32
    {45,1152,1048, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.20  BV=11.24
    {45,1152,1120, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.30  BV=11.14
    {45,1280,1088, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.41  BV=11.03
    {45,1408,1040, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.48  BV=10.96
    {60,1152,1056, 0, 0, 0},  /* TV = 14.02(4 lines)  AV=2.00  SV=5.21  BV=10.81 */
    {60,1152,1128, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.31  BV=10.72
    {60,1280,1072, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.39  BV=10.64
    {75,1152,1032, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.18  BV=10.52
    {75,1152,1088, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.26  BV=10.45
    {75,1280,1056, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.37  BV=10.34
    {90,1152,1040, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.19  BV=10.25
    {90,1280,1024, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.32  BV=10.12
    {104,1152,1040, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.19  BV=10.04
    {104,1152,1104, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.28  BV=9.95
    {119,1152,1032, 0, 0, 0},  //TV = 13.04(8 lines)  AV=2.00  SV=5.18  BV=9.86
    {134,1136,1024, 0, 0, 0},  /* TV = 12.87(9 lines)  AV=2.00  SV=5.15  BV=9.72 */
    {134,1152,1064, 0, 0, 0},  //TV = 12.87(9 lines)  AV=2.00  SV=5.23  BV=9.64
    {149,1152,1032, 0, 0, 0},  //TV = 12.71(10 lines)  AV=2.00  SV=5.18  BV=9.53
    {164,1136,1024, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.15  BV=9.42
    {164,1152,1088, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.26  BV=9.32
    {179,1152,1040, 0, 0, 0},  /* TV = 12.45(12 lines)  AV=2.00  SV=5.19  BV=9.26 */
    {193,1152,1032, 0, 0, 0},  //TV = 12.34(13 lines)  AV=2.00  SV=5.18  BV=9.16
    {208,1152,1032, 0, 0, 0},  //TV = 12.23(14 lines)  AV=2.00  SV=5.18  BV=9.05
    {223,1152,1032, 0, 0, 0},  //TV = 12.13(15 lines)  AV=2.00  SV=5.18  BV=8.95
    {238,1152,1032, 0, 0, 0},  //TV = 12.04(16 lines)  AV=2.00  SV=5.18  BV=8.86
    {253,1152,1040, 0, 0, 0},  //TV = 11.95(17 lines)  AV=2.00  SV=5.19  BV=8.76
    {268,1152,1056, 0, 0, 0},  //TV = 11.87(18 lines)  AV=2.00  SV=5.21  BV=8.65
    {297,1152,1024, 0, 0, 0},  //TV = 11.72(20 lines)  AV=2.00  SV=5.17  BV=8.55
    {312,1152,1040, 0, 0, 0},  //TV = 11.65(21 lines)  AV=2.00  SV=5.19  BV=8.45
    {342,1152,1024, 0, 0, 0},  //TV = 11.51(23 lines)  AV=2.00  SV=5.17  BV=8.34
    {371,1136,1024, 0, 0, 0},  //TV = 11.40(25 lines)  AV=2.00  SV=5.15  BV=8.25
    {386,1152,1032, 0, 0, 0},  //TV = 11.34(26 lines)  AV=2.00  SV=5.18  BV=8.16
    {416,1152,1032, 0, 0, 0},  //TV = 11.23(28 lines)  AV=2.00  SV=5.18  BV=8.05
    {446,1152,1032, 0, 0, 0},  //TV = 11.13(30 lines)  AV=2.00  SV=5.18  BV=7.95
    {475,1152,1032, 0, 0, 0},  //TV = 11.04(32 lines)  AV=2.00  SV=5.18  BV=7.86
    {520,1136,1024, 0, 0, 0},  //TV = 10.91(35 lines)  AV=2.00  SV=5.15  BV=7.76
    {564,1136,1024, 0, 0, 0},  //TV = 10.79(38 lines)  AV=2.00  SV=5.15  BV=7.64
    {594,1152,1024, 0, 0, 0},  //TV = 10.72(40 lines)  AV=2.00  SV=5.17  BV=7.55
    {638,1152,1024, 0, 0, 0},  //TV = 10.61(43 lines)  AV=2.00  SV=5.17  BV=7.44
    {683,1152,1024, 0, 0, 0},  //TV = 10.52(46 lines)  AV=2.00  SV=5.17  BV=7.35
    {727,1152,1024, 0, 0, 0},  //TV = 10.43(49 lines)  AV=2.00  SV=5.17  BV=7.26
    {787,1152,1024, 0, 0, 0},  //TV = 10.31(53 lines)  AV=2.00  SV=5.17  BV=7.14
    {846,1136,1024, 0, 0, 0},  //TV = 10.21(57 lines)  AV=2.00  SV=5.15  BV=7.06
    {905,1136,1024, 0, 0, 0},  //TV = 10.11(61 lines)  AV=2.00  SV=5.15  BV=6.96
    {965,1152,1024, 0, 0, 0},  //TV = 10.02(65 lines)  AV=2.00  SV=5.17  BV=6.85
    {1039,1152,1024, 0, 0, 0},  //TV = 9.91(70 lines)  AV=2.00  SV=5.17  BV=6.74
    {1128,1136,1024, 0, 0, 0},  //TV = 9.79(76 lines)  AV=2.00  SV=5.15  BV=6.64
    {1202,1136,1024, 0, 0, 0},  //TV = 9.70(81 lines)  AV=2.00  SV=5.15  BV=6.55
    {1291,1136,1024, 0, 0, 0},  //TV = 9.60(87 lines)  AV=2.00  SV=5.15  BV=6.45
    {1380,1136,1024, 0, 0, 0},  //TV = 9.50(93 lines)  AV=2.00  SV=5.15  BV=6.35
    {1484,1136,1024, 0, 0, 0},  //TV = 9.40(100 lines)  AV=2.00  SV=5.15  BV=6.25
    {1588,1136,1024, 0, 0, 0},  //TV = 9.30(107 lines)  AV=2.00  SV=5.15  BV=6.15
    {1692,1136,1024, 0, 0, 0},  //TV = 9.21(114 lines)  AV=2.00  SV=5.15  BV=6.06
    {1810,1136,1024, 0, 0, 0},  //TV = 9.11(122 lines)  AV=2.00  SV=5.15  BV=5.96
    {1959,1136,1024, 0, 0, 0},  //TV = 9.00(132 lines)  AV=2.00  SV=5.15  BV=5.85
    {2107,1136,1024, 0, 0, 0},  //TV = 8.89(142 lines)  AV=2.00  SV=5.15  BV=5.74
    {2255,1136,1024, 0, 0, 0},  //TV = 8.79(152 lines)  AV=2.00  SV=5.15  BV=5.64
    {2389,1136,1024, 0, 0, 0},  //TV = 8.71(161 lines)  AV=2.00  SV=5.15  BV=5.56
    {2582,1136,1024, 0, 0, 0},  //TV = 8.60(174 lines)  AV=2.00  SV=5.15  BV=5.45
    {2774,1136,1024, 0, 0, 0},  //TV = 8.49(187 lines)  AV=2.00  SV=5.15  BV=5.34
    {2967,1136,1024, 0, 0, 0},  //TV = 8.40(200 lines)  AV=2.00  SV=5.15  BV=5.25
    {3175,1136,1024, 0, 0, 0},  //TV = 8.30(214 lines)  AV=2.00  SV=5.15  BV=5.15
    {3397,1136,1024, 0, 0, 0},  //TV = 8.20(229 lines)  AV=2.00  SV=5.15  BV=5.05
    {3635,1136,1024, 0, 0, 0},  //TV = 8.10(245 lines)  AV=2.00  SV=5.15  BV=4.95
    {3932,1136,1024, 0, 0, 0},  //TV = 7.99(265 lines)  AV=2.00  SV=5.15  BV=4.84
    {4199,1136,1024, 0, 0, 0},  //TV = 7.90(283 lines)  AV=2.00  SV=5.15  BV=4.75
    {4495,1136,1024, 0, 0, 0},  //TV = 7.80(303 lines)  AV=2.00  SV=5.15  BV=4.65
    {4822,1136,1024, 0, 0, 0},  //TV = 7.70(325 lines)  AV=2.00  SV=5.15  BV=4.55
    {5163,1136,1024, 0, 0, 0},  //TV = 7.60(348 lines)  AV=2.00  SV=5.15  BV=4.45
    {5563,1136,1024, 0, 0, 0},  //TV = 7.49(375 lines)  AV=2.00  SV=5.15  BV=4.34
    {5949,1136,1024, 0, 0, 0},  //TV = 7.39(401 lines)  AV=2.00  SV=5.15  BV=4.24
    {6379,1136,1024, 0, 0, 0},  //TV = 7.29(430 lines)  AV=2.00  SV=5.15  BV=4.14
    {6824,1136,1024, 0, 0, 0},  //TV = 7.20(460 lines)  AV=2.00  SV=5.15  BV=4.05
    {7314,1136,1024, 0, 0, 0},  //TV = 7.10(493 lines)  AV=2.00  SV=5.15  BV=3.95
    {7833,1136,1024, 0, 0, 0},  //TV = 7.00(528 lines)  AV=2.00  SV=5.15  BV=3.85
    {8397,1136,1024, 0, 0, 0},  //TV = 6.90(566 lines)  AV=2.00  SV=5.15  BV=3.75
    {8990,1136,1024, 0, 0, 0},  //TV = 6.80(606 lines)  AV=2.00  SV=5.15  BV=3.65
    {9628,1136,1024, 0, 0, 0},  //TV = 6.70(649 lines)  AV=2.00  SV=5.15  BV=3.55
    {9999,1152,1040, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.19  BV=3.45
    {9999,1152,1112, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.29  BV=3.36
    {9999,1280,1080, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.40  BV=3.25
    {9999,1408,1056, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.50  BV=3.14
    {9999,1536,1032, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.60  BV=3.05
    {9999,1664,1024, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.70  BV=2.94
    {9999,1664,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.79  BV=2.86
    {9999,1792,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.89  BV=2.75
    {9999,1920,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.99  BV=2.65
    {9999,2176,1032, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=6.10  BV=2.55
    {19997,1152,1040, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.19  BV=2.45
    {19997,1152,1112, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.29  BV=2.36
    {19997,1280,1080, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.40  BV=2.25
    {19997,1408,1048, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.49  BV=2.15
    {19997,1536,1032, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.60  BV=2.05
    {19997,1664,1024, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.70  BV=1.94
    {29995,1152,1048, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.20  BV=1.86
    {29995,1152,1128, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.31  BV=1.75
    {29995,1280,1088, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.41  BV=1.65
    {29995,1408,1064, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.51  BV=1.54
    {29995,1536,1040, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.61  BV=1.45
    {29995,1664,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.71  BV=1.35
    {29995,1792,1024, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.81  BV=1.25
    {29995,1920,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.92  BV=1.14
    {29995,2048,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.01  BV=1.05
    {29995,2176,1040, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.11  BV=0.95
    {29995,2304,1056, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.21  BV=0.84
    {40008,1792,1088, 0, 0, 0},  //TV = 4.64(2697 lines)  AV=2.00  SV=5.89  BV=0.75
    {40008,1920,1088, 0, 0, 0},  //TV = 4.64(2697 lines)  AV=2.00  SV=5.99  BV=0.65
    {40008,2176,1024, 0, 0, 0},  //TV = 4.64(2697 lines)  AV=2.00  SV=6.09  BV=0.56
    {50006,1792,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.87  BV=0.45
    {50006,1920,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.97  BV=0.35
    {50006,2048,1080, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.08  BV=0.24
    {50006,2304,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.17  BV=0.15
    {50006,2432,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.27  BV=0.05
    {50006,2560,1064, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.38  BV=-0.06
    {50006,2816,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.47  BV=-0.15
    {50006,2944,1056, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.57  BV=-0.25
    {50006,3200,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.67  BV=-0.34
    {50006,3456,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.78  BV=-0.46
    {50006,3712,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.87  BV=-0.55
    {50006,3968,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.97  BV=-0.64
    {50006,4224,1048, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.08  BV=-0.76
    {60004,3840,1024, 0, 0, 0},  //TV = 4.06(4045 lines)  AV=2.00  SV=6.91  BV=-0.85
    {60004,4096,1032, 0, 0, 0},  //TV = 4.06(4045 lines)  AV=2.00  SV=7.01  BV=-0.95
    {70002,3712,1048, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=6.89  BV=-1.05
    {70002,3968,1048, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=6.99  BV=-1.15
    {70002,4352,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.09  BV=-1.25
    {70002,4608,1040, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.19  BV=-1.36
    {70002,4992,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.29  BV=-1.45
    {70002,5376,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.39  BV=-1.56
    {70002,5632,1040, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.48  BV=-1.65
    {70002,6144,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.58  BV=-1.75
    {70002,6528,1032, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.68  BV=-1.85
    {70002,7040,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.78  BV=-1.94
    {70002,7552,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.88  BV=-2.05
    {70002,8064,1032, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.99  BV=-2.15
    {70002,8704,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=8.09  BV=-2.25
    {80000,8064,1032, 0, 0, 0},  //TV = 3.64(5393 lines)  AV=2.00  SV=7.99  BV=-2.34
    {89998,7680,1032, 0, 0, 0},  //TV = 3.47(6067 lines)  AV=2.00  SV=7.92  BV=-2.44
    {89998,8320,1024, 0, 0, 0},  //TV = 3.47(6067 lines)  AV=2.00  SV=8.02  BV=-2.55
    {99996,7936,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=7.97  BV=-2.64
    {99996,8576,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.08  BV=-2.76
    {99996,9216,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.17  BV=-2.85
    {99996,9856,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.27  BV=-2.94
    {99996,10624,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.38  BV=-3.05
    {99996,11392,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.48  BV=-3.15
    {99996,12160,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.57  BV=-3.25
    {99996,13056,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.67  BV=-3.35
    {99996,13952,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.77  BV=-3.45
    {99996,14976,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.87  BV=-3.55
    {99996,16000,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.97  BV=-3.64
    {99996,16384,1024, 0, 0, 0},  /* TV = 3.32(6741 lines)  AV=2.00  SV=9.00  BV=-3.68 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
},
};

static strAETable g_AE_Custom5AutoTable =
{
    AETABLE_CUSTOM5_AUTO, //eAETableID
    156, //u4TotalIndex
    -20, //i4StrobeTrigerBV
    118, //i4MaxBV
    -37, //i4MinBV
    90, //i4EffectiveMaxBV
    0, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_AUTO, //ISO
    sCustom5PLineTable_60Hz,
    sCustom5PLineTable_50Hz,
    NULL,
};
static strEvPline sVideoNightPLineTable_60Hz =
{
{
    {30,1152,1104, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.28  BV=11.75
    {30,1280,1056, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.37  BV=11.66
    {30,1408,1040, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.48  BV=11.54
    {30,1536,1032, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.60  BV=11.43
    {30,1664,1024, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.70  BV=11.32
    {45,1152,1048, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.20  BV=11.24
    {45,1152,1120, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.30  BV=11.14
    {45,1280,1088, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.41  BV=11.03
    {45,1408,1040, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.48  BV=10.96
    {60,1152,1056, 0, 0, 0},  /* TV = 14.02(4 lines)  AV=2.00  SV=5.21  BV=10.81 */
    {60,1152,1128, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.31  BV=10.72
    {60,1280,1072, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.39  BV=10.64
    {75,1152,1032, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.18  BV=10.52
    {75,1152,1088, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.26  BV=10.45
    {75,1280,1056, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.37  BV=10.34
    {90,1152,1040, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.19  BV=10.25
    {90,1280,1024, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.32  BV=10.12
    {104,1152,1040, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.19  BV=10.04
    {104,1152,1104, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.28  BV=9.95
    {119,1152,1032, 0, 0, 0},  //TV = 13.04(8 lines)  AV=2.00  SV=5.18  BV=9.86
    {134,1136,1024, 0, 0, 0},  /* TV = 12.87(9 lines)  AV=2.00  SV=5.15  BV=9.72 */
    {134,1152,1064, 0, 0, 0},  //TV = 12.87(9 lines)  AV=2.00  SV=5.23  BV=9.64
    {149,1152,1032, 0, 0, 0},  //TV = 12.71(10 lines)  AV=2.00  SV=5.18  BV=9.53
    {164,1136,1024, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.15  BV=9.42
    {164,1152,1088, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.26  BV=9.32
    {179,1152,1040, 0, 0, 0},  /* TV = 12.45(12 lines)  AV=2.00  SV=5.19  BV=9.26 */
    {193,1152,1032, 0, 0, 0},  //TV = 12.34(13 lines)  AV=2.00  SV=5.18  BV=9.16
    {208,1152,1032, 0, 0, 0},  //TV = 12.23(14 lines)  AV=2.00  SV=5.18  BV=9.05
    {223,1152,1032, 0, 0, 0},  //TV = 12.13(15 lines)  AV=2.00  SV=5.18  BV=8.95
    {238,1152,1032, 0, 0, 0},  //TV = 12.04(16 lines)  AV=2.00  SV=5.18  BV=8.86
    {253,1152,1040, 0, 0, 0},  //TV = 11.95(17 lines)  AV=2.00  SV=5.19  BV=8.76
    {268,1152,1056, 0, 0, 0},  //TV = 11.87(18 lines)  AV=2.00  SV=5.21  BV=8.65
    {297,1152,1024, 0, 0, 0},  //TV = 11.72(20 lines)  AV=2.00  SV=5.17  BV=8.55
    {312,1152,1040, 0, 0, 0},  //TV = 11.65(21 lines)  AV=2.00  SV=5.19  BV=8.45
    {342,1152,1024, 0, 0, 0},  //TV = 11.51(23 lines)  AV=2.00  SV=5.17  BV=8.34
    {371,1136,1024, 0, 0, 0},  //TV = 11.40(25 lines)  AV=2.00  SV=5.15  BV=8.25
    {386,1152,1032, 0, 0, 0},  //TV = 11.34(26 lines)  AV=2.00  SV=5.18  BV=8.16
    {416,1152,1032, 0, 0, 0},  //TV = 11.23(28 lines)  AV=2.00  SV=5.18  BV=8.05
    {446,1152,1032, 0, 0, 0},  //TV = 11.13(30 lines)  AV=2.00  SV=5.18  BV=7.95
    {475,1152,1032, 0, 0, 0},  //TV = 11.04(32 lines)  AV=2.00  SV=5.18  BV=7.86
    {520,1136,1024, 0, 0, 0},  //TV = 10.91(35 lines)  AV=2.00  SV=5.15  BV=7.76
    {564,1136,1024, 0, 0, 0},  //TV = 10.79(38 lines)  AV=2.00  SV=5.15  BV=7.64
    {594,1152,1024, 0, 0, 0},  //TV = 10.72(40 lines)  AV=2.00  SV=5.17  BV=7.55
    {638,1152,1024, 0, 0, 0},  //TV = 10.61(43 lines)  AV=2.00  SV=5.17  BV=7.44
    {683,1152,1024, 0, 0, 0},  //TV = 10.52(46 lines)  AV=2.00  SV=5.17  BV=7.35
    {727,1152,1024, 0, 0, 0},  //TV = 10.43(49 lines)  AV=2.00  SV=5.17  BV=7.26
    {787,1152,1024, 0, 0, 0},  //TV = 10.31(53 lines)  AV=2.00  SV=5.17  BV=7.14
    {846,1136,1024, 0, 0, 0},  //TV = 10.21(57 lines)  AV=2.00  SV=5.15  BV=7.06
    {905,1136,1024, 0, 0, 0},  //TV = 10.11(61 lines)  AV=2.00  SV=5.15  BV=6.96
    {965,1152,1024, 0, 0, 0},  //TV = 10.02(65 lines)  AV=2.00  SV=5.17  BV=6.85
    {1039,1152,1024, 0, 0, 0},  //TV = 9.91(70 lines)  AV=2.00  SV=5.17  BV=6.74
    {1128,1136,1024, 0, 0, 0},  //TV = 9.79(76 lines)  AV=2.00  SV=5.15  BV=6.64
    {1202,1136,1024, 0, 0, 0},  //TV = 9.70(81 lines)  AV=2.00  SV=5.15  BV=6.55
    {1291,1136,1024, 0, 0, 0},  //TV = 9.60(87 lines)  AV=2.00  SV=5.15  BV=6.45
    {1380,1136,1024, 0, 0, 0},  //TV = 9.50(93 lines)  AV=2.00  SV=5.15  BV=6.35
    {1484,1136,1024, 0, 0, 0},  //TV = 9.40(100 lines)  AV=2.00  SV=5.15  BV=6.25
    {1588,1136,1024, 0, 0, 0},  //TV = 9.30(107 lines)  AV=2.00  SV=5.15  BV=6.15
    {1692,1136,1024, 0, 0, 0},  //TV = 9.21(114 lines)  AV=2.00  SV=5.15  BV=6.06
    {1810,1136,1024, 0, 0, 0},  //TV = 9.11(122 lines)  AV=2.00  SV=5.15  BV=5.96
    {1959,1136,1024, 0, 0, 0},  //TV = 9.00(132 lines)  AV=2.00  SV=5.15  BV=5.85
    {2107,1136,1024, 0, 0, 0},  //TV = 8.89(142 lines)  AV=2.00  SV=5.15  BV=5.74
    {2255,1136,1024, 0, 0, 0},  //TV = 8.79(152 lines)  AV=2.00  SV=5.15  BV=5.64
    {2389,1136,1024, 0, 0, 0},  //TV = 8.71(161 lines)  AV=2.00  SV=5.15  BV=5.56
    {2582,1136,1024, 0, 0, 0},  //TV = 8.60(174 lines)  AV=2.00  SV=5.15  BV=5.45
    {2774,1136,1024, 0, 0, 0},  //TV = 8.49(187 lines)  AV=2.00  SV=5.15  BV=5.34
    {2967,1136,1024, 0, 0, 0},  //TV = 8.40(200 lines)  AV=2.00  SV=5.15  BV=5.25
    {3175,1136,1024, 0, 0, 0},  //TV = 8.30(214 lines)  AV=2.00  SV=5.15  BV=5.15
    {3397,1136,1024, 0, 0, 0},  //TV = 8.20(229 lines)  AV=2.00  SV=5.15  BV=5.05
    {3635,1136,1024, 0, 0, 0},  //TV = 8.10(245 lines)  AV=2.00  SV=5.15  BV=4.95
    {3932,1136,1024, 0, 0, 0},  //TV = 7.99(265 lines)  AV=2.00  SV=5.15  BV=4.84
    {4199,1136,1024, 0, 0, 0},  //TV = 7.90(283 lines)  AV=2.00  SV=5.15  BV=4.75
    {4495,1136,1024, 0, 0, 0},  //TV = 7.80(303 lines)  AV=2.00  SV=5.15  BV=4.65
    {4822,1136,1024, 0, 0, 0},  //TV = 7.70(325 lines)  AV=2.00  SV=5.15  BV=4.55
    {5163,1136,1024, 0, 0, 0},  //TV = 7.60(348 lines)  AV=2.00  SV=5.15  BV=4.45
    {5563,1136,1024, 0, 0, 0},  //TV = 7.49(375 lines)  AV=2.00  SV=5.15  BV=4.34
    {5949,1136,1024, 0, 0, 0},  //TV = 7.39(401 lines)  AV=2.00  SV=5.15  BV=4.24
    {6379,1136,1024, 0, 0, 0},  //TV = 7.29(430 lines)  AV=2.00  SV=5.15  BV=4.14
    {6824,1136,1024, 0, 0, 0},  //TV = 7.20(460 lines)  AV=2.00  SV=5.15  BV=4.05
    {7314,1136,1024, 0, 0, 0},  //TV = 7.10(493 lines)  AV=2.00  SV=5.15  BV=3.95
    {7833,1136,1024, 0, 0, 0},  //TV = 7.00(528 lines)  AV=2.00  SV=5.15  BV=3.85
    {8337,1136,1024, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.15  BV=3.76
    {8337,1152,1088, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.26  BV=3.65
    {8337,1280,1048, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.36  BV=3.55
    {8337,1408,1024, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.46  BV=3.45
    {8337,1408,1104, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.57  BV=3.34
    {8337,1536,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.65  BV=3.26
    {8337,1664,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.77  BV=3.14
    {8337,1792,1056, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.85  BV=3.05
    {8337,1920,1064, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.96  BV=2.94
    {8337,2048,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=6.07  BV=2.84
    {16659,1136,1024, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.15  BV=2.76
    {16659,1152,1088, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.26  BV=2.65
    {16659,1280,1056, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.37  BV=2.54
    {16659,1408,1024, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.46  BV=2.45
    {16659,1408,1096, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.56  BV=2.35
    {16659,1536,1080, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.66  BV=2.25
    {24996,1152,1024, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.17  BV=2.15
    {24996,1152,1096, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.27  BV=2.05
    {24996,1280,1056, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.37  BV=1.96
    {24996,1408,1032, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.47  BV=1.85
    {33332,1136,1024, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.15  BV=1.76
    {33332,1152,1088, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.26  BV=1.65
    {33332,1280,1048, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.36  BV=1.55
    {33332,1408,1024, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.46  BV=1.45
    {33332,1408,1096, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.56  BV=1.35
    {33332,1536,1072, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.65  BV=1.26
    {33332,1664,1072, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.77  BV=1.14
    {33332,1792,1064, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.86  BV=1.04
    {33332,1920,1056, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.95  BV=0.96
    {33332,2048,1064, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=6.06  BV=0.85
    {33332,2176,1072, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=6.15  BV=0.75
    {41669,1920,1040, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=5.93  BV=0.66
    {41669,2048,1048, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=6.03  BV=0.55
    {41669,2176,1056, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=6.13  BV=0.45
    {50006,1920,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.97  BV=0.35
    {50006,2048,1080, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.08  BV=0.24
    {50006,2304,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.17  BV=0.15
    {50006,2432,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.27  BV=0.05
    {50006,2560,1064, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.38  BV=-0.06
    {50006,2816,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.47  BV=-0.15
    {50006,2944,1056, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.57  BV=-0.25
    {50006,3200,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.67  BV=-0.34
    {50006,3456,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.78  BV=-0.46
    {50006,3712,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.87  BV=-0.55
    {50006,3968,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.97  BV=-0.64
    {50006,4224,1048, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.08  BV=-0.76
    {58328,3968,1024, 0, 0, 0},  //TV = 4.10(3932 lines)  AV=2.00  SV=6.95  BV=-0.85
    {58328,4224,1032, 0, 0, 0},  //TV = 4.10(3932 lines)  AV=2.00  SV=7.06  BV=-0.96
    {66664,3968,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=6.97  BV=-1.06
    {66664,4224,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.06  BV=-1.15
    {66664,4480,1040, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.15  BV=-1.24
    {66664,4864,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.26  BV=-1.35
    {66664,5248,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.36  BV=-1.45
    {66664,5632,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.46  BV=-1.55
    {66664,6016,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.55  BV=-1.65
    {66664,6400,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.66  BV=-1.75
    {66664,6912,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.75  BV=-1.85
    {66664,7424,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.86  BV=-1.95
    {66664,7936,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.95  BV=-2.05
    {66664,8448,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=8.06  BV=-2.15
    {75001,8064,1032, 0, 0, 0},  //TV = 3.74(5056 lines)  AV=2.00  SV=7.99  BV=-2.25
    {83338,7808,1024, 0, 0, 0},  //TV = 3.58(5618 lines)  AV=2.00  SV=7.93  BV=-2.35
    {83338,8320,1032, 0, 0, 0},  //TV = 3.58(5618 lines)  AV=2.00  SV=8.03  BV=-2.45
    {91660,8192,1024, 0, 0, 0},  //TV = 3.45(6179 lines)  AV=2.00  SV=8.00  BV=-2.55
    {99996,7936,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=7.97  BV=-2.64
    {99996,8576,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.08  BV=-2.76
    {99996,9216,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.17  BV=-2.85
    {99996,9856,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.27  BV=-2.94
    {99996,10624,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.38  BV=-3.05
    {99996,11392,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.48  BV=-3.15
    {99996,12160,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.57  BV=-3.25
    {99996,13056,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.67  BV=-3.35
    {99996,13952,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.77  BV=-3.45
    {99996,14976,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.87  BV=-3.55
    {99996,16000,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.97  BV=-3.64
    {99996,16384,1024, 0, 0, 0},  /* TV = 3.32(6741 lines)  AV=2.00  SV=9.00  BV=-3.68 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
},
};

static strEvPline sVideoNightPLineTable_50Hz =
{
{
    {30,1152,1104, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.28  BV=11.75
    {30,1280,1056, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.37  BV=11.66
    {30,1408,1040, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.48  BV=11.54
    {30,1536,1032, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.60  BV=11.43
    {30,1664,1024, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.70  BV=11.32
    {45,1152,1048, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.20  BV=11.24
    {45,1152,1120, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.30  BV=11.14
    {45,1280,1088, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.41  BV=11.03
    {45,1408,1040, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.48  BV=10.96
    {60,1152,1056, 0, 0, 0},  /* TV = 14.02(4 lines)  AV=2.00  SV=5.21  BV=10.81 */
    {60,1152,1128, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.31  BV=10.72
    {60,1280,1072, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.39  BV=10.64
    {75,1152,1032, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.18  BV=10.52
    {75,1152,1088, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.26  BV=10.45
    {75,1280,1056, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.37  BV=10.34
    {90,1152,1040, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.19  BV=10.25
    {90,1280,1024, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.32  BV=10.12
    {104,1152,1040, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.19  BV=10.04
    {104,1152,1104, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.28  BV=9.95
    {119,1152,1032, 0, 0, 0},  //TV = 13.04(8 lines)  AV=2.00  SV=5.18  BV=9.86
    {134,1136,1024, 0, 0, 0},  /* TV = 12.87(9 lines)  AV=2.00  SV=5.15  BV=9.72 */
    {134,1152,1064, 0, 0, 0},  //TV = 12.87(9 lines)  AV=2.00  SV=5.23  BV=9.64
    {149,1152,1032, 0, 0, 0},  //TV = 12.71(10 lines)  AV=2.00  SV=5.18  BV=9.53
    {164,1136,1024, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.15  BV=9.42
    {164,1152,1088, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.26  BV=9.32
    {179,1152,1040, 0, 0, 0},  /* TV = 12.45(12 lines)  AV=2.00  SV=5.19  BV=9.26 */
    {193,1152,1032, 0, 0, 0},  //TV = 12.34(13 lines)  AV=2.00  SV=5.18  BV=9.16
    {208,1152,1032, 0, 0, 0},  //TV = 12.23(14 lines)  AV=2.00  SV=5.18  BV=9.05
    {223,1152,1032, 0, 0, 0},  //TV = 12.13(15 lines)  AV=2.00  SV=5.18  BV=8.95
    {238,1152,1032, 0, 0, 0},  //TV = 12.04(16 lines)  AV=2.00  SV=5.18  BV=8.86
    {253,1152,1040, 0, 0, 0},  //TV = 11.95(17 lines)  AV=2.00  SV=5.19  BV=8.76
    {268,1152,1056, 0, 0, 0},  //TV = 11.87(18 lines)  AV=2.00  SV=5.21  BV=8.65
    {297,1152,1024, 0, 0, 0},  //TV = 11.72(20 lines)  AV=2.00  SV=5.17  BV=8.55
    {312,1152,1040, 0, 0, 0},  //TV = 11.65(21 lines)  AV=2.00  SV=5.19  BV=8.45
    {342,1152,1024, 0, 0, 0},  //TV = 11.51(23 lines)  AV=2.00  SV=5.17  BV=8.34
    {371,1136,1024, 0, 0, 0},  //TV = 11.40(25 lines)  AV=2.00  SV=5.15  BV=8.25
    {386,1152,1032, 0, 0, 0},  //TV = 11.34(26 lines)  AV=2.00  SV=5.18  BV=8.16
    {416,1152,1032, 0, 0, 0},  //TV = 11.23(28 lines)  AV=2.00  SV=5.18  BV=8.05
    {446,1152,1032, 0, 0, 0},  //TV = 11.13(30 lines)  AV=2.00  SV=5.18  BV=7.95
    {475,1152,1032, 0, 0, 0},  //TV = 11.04(32 lines)  AV=2.00  SV=5.18  BV=7.86
    {520,1136,1024, 0, 0, 0},  //TV = 10.91(35 lines)  AV=2.00  SV=5.15  BV=7.76
    {564,1136,1024, 0, 0, 0},  //TV = 10.79(38 lines)  AV=2.00  SV=5.15  BV=7.64
    {594,1152,1024, 0, 0, 0},  //TV = 10.72(40 lines)  AV=2.00  SV=5.17  BV=7.55
    {638,1152,1024, 0, 0, 0},  //TV = 10.61(43 lines)  AV=2.00  SV=5.17  BV=7.44
    {683,1152,1024, 0, 0, 0},  //TV = 10.52(46 lines)  AV=2.00  SV=5.17  BV=7.35
    {727,1152,1024, 0, 0, 0},  //TV = 10.43(49 lines)  AV=2.00  SV=5.17  BV=7.26
    {787,1152,1024, 0, 0, 0},  //TV = 10.31(53 lines)  AV=2.00  SV=5.17  BV=7.14
    {846,1136,1024, 0, 0, 0},  //TV = 10.21(57 lines)  AV=2.00  SV=5.15  BV=7.06
    {905,1136,1024, 0, 0, 0},  //TV = 10.11(61 lines)  AV=2.00  SV=5.15  BV=6.96
    {965,1152,1024, 0, 0, 0},  //TV = 10.02(65 lines)  AV=2.00  SV=5.17  BV=6.85
    {1039,1152,1024, 0, 0, 0},  //TV = 9.91(70 lines)  AV=2.00  SV=5.17  BV=6.74
    {1128,1136,1024, 0, 0, 0},  //TV = 9.79(76 lines)  AV=2.00  SV=5.15  BV=6.64
    {1202,1136,1024, 0, 0, 0},  //TV = 9.70(81 lines)  AV=2.00  SV=5.15  BV=6.55
    {1291,1136,1024, 0, 0, 0},  //TV = 9.60(87 lines)  AV=2.00  SV=5.15  BV=6.45
    {1380,1136,1024, 0, 0, 0},  //TV = 9.50(93 lines)  AV=2.00  SV=5.15  BV=6.35
    {1484,1136,1024, 0, 0, 0},  //TV = 9.40(100 lines)  AV=2.00  SV=5.15  BV=6.25
    {1588,1136,1024, 0, 0, 0},  //TV = 9.30(107 lines)  AV=2.00  SV=5.15  BV=6.15
    {1692,1136,1024, 0, 0, 0},  //TV = 9.21(114 lines)  AV=2.00  SV=5.15  BV=6.06
    {1810,1136,1024, 0, 0, 0},  //TV = 9.11(122 lines)  AV=2.00  SV=5.15  BV=5.96
    {1959,1136,1024, 0, 0, 0},  //TV = 9.00(132 lines)  AV=2.00  SV=5.15  BV=5.85
    {2107,1136,1024, 0, 0, 0},  //TV = 8.89(142 lines)  AV=2.00  SV=5.15  BV=5.74
    {2255,1136,1024, 0, 0, 0},  //TV = 8.79(152 lines)  AV=2.00  SV=5.15  BV=5.64
    {2389,1136,1024, 0, 0, 0},  //TV = 8.71(161 lines)  AV=2.00  SV=5.15  BV=5.56
    {2582,1136,1024, 0, 0, 0},  //TV = 8.60(174 lines)  AV=2.00  SV=5.15  BV=5.45
    {2774,1136,1024, 0, 0, 0},  //TV = 8.49(187 lines)  AV=2.00  SV=5.15  BV=5.34
    {2967,1136,1024, 0, 0, 0},  //TV = 8.40(200 lines)  AV=2.00  SV=5.15  BV=5.25
    {3175,1136,1024, 0, 0, 0},  //TV = 8.30(214 lines)  AV=2.00  SV=5.15  BV=5.15
    {3397,1136,1024, 0, 0, 0},  //TV = 8.20(229 lines)  AV=2.00  SV=5.15  BV=5.05
    {3635,1136,1024, 0, 0, 0},  //TV = 8.10(245 lines)  AV=2.00  SV=5.15  BV=4.95
    {3932,1136,1024, 0, 0, 0},  //TV = 7.99(265 lines)  AV=2.00  SV=5.15  BV=4.84
    {4199,1136,1024, 0, 0, 0},  //TV = 7.90(283 lines)  AV=2.00  SV=5.15  BV=4.75
    {4495,1136,1024, 0, 0, 0},  //TV = 7.80(303 lines)  AV=2.00  SV=5.15  BV=4.65
    {4822,1136,1024, 0, 0, 0},  //TV = 7.70(325 lines)  AV=2.00  SV=5.15  BV=4.55
    {5163,1136,1024, 0, 0, 0},  //TV = 7.60(348 lines)  AV=2.00  SV=5.15  BV=4.45
    {5563,1136,1024, 0, 0, 0},  //TV = 7.49(375 lines)  AV=2.00  SV=5.15  BV=4.34
    {5949,1136,1024, 0, 0, 0},  //TV = 7.39(401 lines)  AV=2.00  SV=5.15  BV=4.24
    {6379,1136,1024, 0, 0, 0},  //TV = 7.29(430 lines)  AV=2.00  SV=5.15  BV=4.14
    {6824,1136,1024, 0, 0, 0},  //TV = 7.20(460 lines)  AV=2.00  SV=5.15  BV=4.05
    {7314,1136,1024, 0, 0, 0},  //TV = 7.10(493 lines)  AV=2.00  SV=5.15  BV=3.95
    {7833,1136,1024, 0, 0, 0},  //TV = 7.00(528 lines)  AV=2.00  SV=5.15  BV=3.85
    {8397,1136,1024, 0, 0, 0},  //TV = 6.90(566 lines)  AV=2.00  SV=5.15  BV=3.75
    {8990,1136,1024, 0, 0, 0},  //TV = 6.80(606 lines)  AV=2.00  SV=5.15  BV=3.65
    {9628,1136,1024, 0, 0, 0},  //TV = 6.70(649 lines)  AV=2.00  SV=5.15  BV=3.55
    {9999,1152,1040, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.19  BV=3.45
    {9999,1152,1112, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.29  BV=3.36
    {9999,1280,1080, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.40  BV=3.25
    {9999,1408,1056, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.50  BV=3.14
    {9999,1536,1032, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.60  BV=3.05
    {9999,1664,1024, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.70  BV=2.94
    {9999,1664,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.79  BV=2.86
    {9999,1792,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.89  BV=2.75
    {9999,1920,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.99  BV=2.65
    {9999,2176,1032, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=6.10  BV=2.55
    {19997,1152,1040, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.19  BV=2.45
    {19997,1152,1112, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.29  BV=2.36
    {19997,1280,1080, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.40  BV=2.25
    {19997,1408,1048, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.49  BV=2.15
    {19997,1536,1032, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.60  BV=2.05
    {19997,1664,1024, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.70  BV=1.94
    {29995,1152,1048, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.20  BV=1.86
    {29995,1152,1128, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.31  BV=1.75
    {29995,1280,1088, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.41  BV=1.65
    {29995,1408,1064, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.51  BV=1.54
    {29995,1536,1040, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.61  BV=1.45
    {29995,1664,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.71  BV=1.35
    {29995,1792,1024, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.81  BV=1.25
    {29995,1920,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.92  BV=1.14
    {29995,2048,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.01  BV=1.05
    {29995,2176,1040, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.11  BV=0.95
    {29995,2304,1056, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.21  BV=0.84
    {40008,1792,1088, 0, 0, 0},  //TV = 4.64(2697 lines)  AV=2.00  SV=5.89  BV=0.75
    {40008,1920,1088, 0, 0, 0},  //TV = 4.64(2697 lines)  AV=2.00  SV=5.99  BV=0.65
    {40008,2176,1024, 0, 0, 0},  //TV = 4.64(2697 lines)  AV=2.00  SV=6.09  BV=0.56
    {50006,1792,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.87  BV=0.45
    {50006,1920,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.97  BV=0.35
    {50006,2048,1080, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.08  BV=0.24
    {50006,2304,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.17  BV=0.15
    {50006,2432,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.27  BV=0.05
    {50006,2560,1064, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.38  BV=-0.06
    {50006,2816,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.47  BV=-0.15
    {50006,2944,1056, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.57  BV=-0.25
    {50006,3200,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.67  BV=-0.34
    {50006,3456,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.78  BV=-0.46
    {50006,3712,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.87  BV=-0.55
    {50006,3968,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.97  BV=-0.64
    {50006,4224,1048, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.08  BV=-0.76
    {60004,3840,1024, 0, 0, 0},  //TV = 4.06(4045 lines)  AV=2.00  SV=6.91  BV=-0.85
    {60004,4096,1032, 0, 0, 0},  //TV = 4.06(4045 lines)  AV=2.00  SV=7.01  BV=-0.95
    {70002,3712,1048, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=6.89  BV=-1.05
    {70002,3968,1048, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=6.99  BV=-1.15
    {70002,4352,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.09  BV=-1.25
    {70002,4608,1040, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.19  BV=-1.36
    {70002,4992,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.29  BV=-1.45
    {70002,5376,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.39  BV=-1.56
    {70002,5632,1040, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.48  BV=-1.65
    {70002,6144,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.58  BV=-1.75
    {70002,6528,1032, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.68  BV=-1.85
    {70002,7040,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.78  BV=-1.94
    {70002,7552,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.88  BV=-2.05
    {70002,8064,1032, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.99  BV=-2.15
    {70002,8704,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=8.09  BV=-2.25
    {80000,8064,1032, 0, 0, 0},  //TV = 3.64(5393 lines)  AV=2.00  SV=7.99  BV=-2.34
    {89998,7680,1032, 0, 0, 0},  //TV = 3.47(6067 lines)  AV=2.00  SV=7.92  BV=-2.44
    {89998,8320,1024, 0, 0, 0},  //TV = 3.47(6067 lines)  AV=2.00  SV=8.02  BV=-2.55
    {99996,7936,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=7.97  BV=-2.64
    {99996,8576,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.08  BV=-2.76
    {99996,9216,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.17  BV=-2.85
    {99996,9856,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.27  BV=-2.94
    {99996,10624,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.38  BV=-3.05
    {99996,11392,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.48  BV=-3.15
    {99996,12160,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.57  BV=-3.25
    {99996,13056,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.67  BV=-3.35
    {99996,13952,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.77  BV=-3.45
    {99996,14976,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.87  BV=-3.55
    {99996,16000,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.97  BV=-3.64
    {99996,16384,1024, 0, 0, 0},  /* TV = 3.32(6741 lines)  AV=2.00  SV=9.00  BV=-3.68 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
},
};

static strAETable g_AE_VideoNightTable =
{
    AETABLE_VIDEO_NIGHT, //eAETableID
    156, //u4TotalIndex
    -20, //i4StrobeTrigerBV
    118, //i4MaxBV
    -37, //i4MinBV
    90, //i4EffectiveMaxBV
    -10, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_AUTO, //ISO
    sVideoNightPLineTable_60Hz,
    sVideoNightPLineTable_50Hz,
    NULL,
};
static strEvPline sCaptureISO100PLineTable_60Hz =
{
{
    {30,1152,1104, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.28  BV=11.75
    {30,1280,1056, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.37  BV=11.66
    {30,1408,1040, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.48  BV=11.54
    {30,1536,1032, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.60  BV=11.43
    {30,1664,1024, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.70  BV=11.32
    {45,1152,1048, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.20  BV=11.24
    {45,1152,1120, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.30  BV=11.14
    {45,1280,1088, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.41  BV=11.03
    {45,1408,1040, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.48  BV=10.96
    {60,1152,1056, 0, 0, 0},  /* TV = 14.02(4 lines)  AV=2.00  SV=5.21  BV=10.81 */
    {60,1152,1128, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.31  BV=10.72
    {60,1280,1072, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.39  BV=10.64
    {75,1152,1032, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.18  BV=10.52
    {75,1152,1088, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.26  BV=10.45
    {75,1280,1056, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.37  BV=10.34
    {90,1152,1040, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.19  BV=10.25
    {90,1280,1024, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.32  BV=10.12
    {104,1152,1040, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.19  BV=10.04
    {104,1152,1104, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.28  BV=9.95
    {119,1152,1032, 0, 0, 0},  //TV = 13.04(8 lines)  AV=2.00  SV=5.18  BV=9.86
    {134,1136,1024, 0, 0, 0},  /* TV = 12.87(9 lines)  AV=2.00  SV=5.15  BV=9.72 */
    {134,1152,1064, 0, 0, 0},  //TV = 12.87(9 lines)  AV=2.00  SV=5.23  BV=9.64
    {149,1152,1032, 0, 0, 0},  //TV = 12.71(10 lines)  AV=2.00  SV=5.18  BV=9.53
    {164,1136,1024, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.15  BV=9.42
    {164,1152,1088, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.26  BV=9.32
    {179,1152,1040, 0, 0, 0},  /* TV = 12.45(12 lines)  AV=2.00  SV=5.19  BV=9.26 */
    {193,1152,1032, 0, 0, 0},  //TV = 12.34(13 lines)  AV=2.00  SV=5.18  BV=9.16
    {208,1152,1032, 0, 0, 0},  //TV = 12.23(14 lines)  AV=2.00  SV=5.18  BV=9.05
    {223,1152,1032, 0, 0, 0},  //TV = 12.13(15 lines)  AV=2.00  SV=5.18  BV=8.95
    {238,1152,1032, 0, 0, 0},  //TV = 12.04(16 lines)  AV=2.00  SV=5.18  BV=8.86
    {253,1152,1040, 0, 0, 0},  //TV = 11.95(17 lines)  AV=2.00  SV=5.19  BV=8.76
    {268,1152,1056, 0, 0, 0},  //TV = 11.87(18 lines)  AV=2.00  SV=5.21  BV=8.65
    {297,1152,1024, 0, 0, 0},  //TV = 11.72(20 lines)  AV=2.00  SV=5.17  BV=8.55
    {312,1152,1040, 0, 0, 0},  //TV = 11.65(21 lines)  AV=2.00  SV=5.19  BV=8.45
    {342,1152,1024, 0, 0, 0},  //TV = 11.51(23 lines)  AV=2.00  SV=5.17  BV=8.34
    {371,1136,1024, 0, 0, 0},  //TV = 11.40(25 lines)  AV=2.00  SV=5.15  BV=8.25
    {386,1152,1032, 0, 0, 0},  //TV = 11.34(26 lines)  AV=2.00  SV=5.18  BV=8.16
    {416,1152,1032, 0, 0, 0},  //TV = 11.23(28 lines)  AV=2.00  SV=5.18  BV=8.05
    {446,1152,1032, 0, 0, 0},  //TV = 11.13(30 lines)  AV=2.00  SV=5.18  BV=7.95
    {475,1152,1032, 0, 0, 0},  //TV = 11.04(32 lines)  AV=2.00  SV=5.18  BV=7.86
    {520,1136,1024, 0, 0, 0},  //TV = 10.91(35 lines)  AV=2.00  SV=5.15  BV=7.76
    {564,1136,1024, 0, 0, 0},  //TV = 10.79(38 lines)  AV=2.00  SV=5.15  BV=7.64
    {594,1152,1024, 0, 0, 0},  //TV = 10.72(40 lines)  AV=2.00  SV=5.17  BV=7.55
    {638,1152,1024, 0, 0, 0},  //TV = 10.61(43 lines)  AV=2.00  SV=5.17  BV=7.44
    {683,1152,1024, 0, 0, 0},  //TV = 10.52(46 lines)  AV=2.00  SV=5.17  BV=7.35
    {727,1152,1024, 0, 0, 0},  //TV = 10.43(49 lines)  AV=2.00  SV=5.17  BV=7.26
    {787,1152,1024, 0, 0, 0},  //TV = 10.31(53 lines)  AV=2.00  SV=5.17  BV=7.14
    {846,1136,1024, 0, 0, 0},  //TV = 10.21(57 lines)  AV=2.00  SV=5.15  BV=7.06
    {905,1136,1024, 0, 0, 0},  //TV = 10.11(61 lines)  AV=2.00  SV=5.15  BV=6.96
    {965,1152,1024, 0, 0, 0},  //TV = 10.02(65 lines)  AV=2.00  SV=5.17  BV=6.85
    {1039,1152,1024, 0, 0, 0},  //TV = 9.91(70 lines)  AV=2.00  SV=5.17  BV=6.74
    {1128,1136,1024, 0, 0, 0},  //TV = 9.79(76 lines)  AV=2.00  SV=5.15  BV=6.64
    {1202,1136,1024, 0, 0, 0},  //TV = 9.70(81 lines)  AV=2.00  SV=5.15  BV=6.55
    {1291,1136,1024, 0, 0, 0},  //TV = 9.60(87 lines)  AV=2.00  SV=5.15  BV=6.45
    {1380,1136,1024, 0, 0, 0},  //TV = 9.50(93 lines)  AV=2.00  SV=5.15  BV=6.35
    {1484,1136,1024, 0, 0, 0},  //TV = 9.40(100 lines)  AV=2.00  SV=5.15  BV=6.25
    {1588,1136,1024, 0, 0, 0},  //TV = 9.30(107 lines)  AV=2.00  SV=5.15  BV=6.15
    {1692,1136,1024, 0, 0, 0},  //TV = 9.21(114 lines)  AV=2.00  SV=5.15  BV=6.06
    {1810,1136,1024, 0, 0, 0},  //TV = 9.11(122 lines)  AV=2.00  SV=5.15  BV=5.96
    {1959,1136,1024, 0, 0, 0},  //TV = 9.00(132 lines)  AV=2.00  SV=5.15  BV=5.85
    {2107,1136,1024, 0, 0, 0},  //TV = 8.89(142 lines)  AV=2.00  SV=5.15  BV=5.74
    {2255,1136,1024, 0, 0, 0},  //TV = 8.79(152 lines)  AV=2.00  SV=5.15  BV=5.64
    {2389,1136,1024, 0, 0, 0},  //TV = 8.71(161 lines)  AV=2.00  SV=5.15  BV=5.56
    {2582,1136,1024, 0, 0, 0},  //TV = 8.60(174 lines)  AV=2.00  SV=5.15  BV=5.45
    {2774,1136,1024, 0, 0, 0},  //TV = 8.49(187 lines)  AV=2.00  SV=5.15  BV=5.34
    {2967,1136,1024, 0, 0, 0},  //TV = 8.40(200 lines)  AV=2.00  SV=5.15  BV=5.25
    {3175,1136,1024, 0, 0, 0},  //TV = 8.30(214 lines)  AV=2.00  SV=5.15  BV=5.15
    {3397,1136,1024, 0, 0, 0},  //TV = 8.20(229 lines)  AV=2.00  SV=5.15  BV=5.05
    {3635,1136,1024, 0, 0, 0},  //TV = 8.10(245 lines)  AV=2.00  SV=5.15  BV=4.95
    {3932,1136,1024, 0, 0, 0},  //TV = 7.99(265 lines)  AV=2.00  SV=5.15  BV=4.84
    {4199,1136,1024, 0, 0, 0},  //TV = 7.90(283 lines)  AV=2.00  SV=5.15  BV=4.75
    {4495,1136,1024, 0, 0, 0},  //TV = 7.80(303 lines)  AV=2.00  SV=5.15  BV=4.65
    {4822,1136,1024, 0, 0, 0},  //TV = 7.70(325 lines)  AV=2.00  SV=5.15  BV=4.55
    {5163,1136,1024, 0, 0, 0},  //TV = 7.60(348 lines)  AV=2.00  SV=5.15  BV=4.45
    {5563,1136,1024, 0, 0, 0},  //TV = 7.49(375 lines)  AV=2.00  SV=5.15  BV=4.34
    {5949,1136,1024, 0, 0, 0},  //TV = 7.39(401 lines)  AV=2.00  SV=5.15  BV=4.24
    {6379,1136,1024, 0, 0, 0},  //TV = 7.29(430 lines)  AV=2.00  SV=5.15  BV=4.14
    {6824,1136,1024, 0, 0, 0},  //TV = 7.20(460 lines)  AV=2.00  SV=5.15  BV=4.05
    {7314,1136,1024, 0, 0, 0},  //TV = 7.10(493 lines)  AV=2.00  SV=5.15  BV=3.95
    {7833,1136,1024, 0, 0, 0},  //TV = 7.00(528 lines)  AV=2.00  SV=5.15  BV=3.85
    {8337,1136,1024, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.15  BV=3.76
    {8337,1152,1088, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.26  BV=3.65
    {8337,1280,1048, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.36  BV=3.55
    {8337,1408,1024, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.46  BV=3.45
    {8337,1408,1104, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.57  BV=3.34
    {8337,1536,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.65  BV=3.26
    {8337,1664,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.77  BV=3.14
    {8337,1792,1056, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.85  BV=3.05
    {8337,1920,1064, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.96  BV=2.94
    {8337,2048,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=6.07  BV=2.84
    {16659,1136,1024, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.15  BV=2.76
    {16659,1152,1088, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.26  BV=2.65
    {16659,1280,1056, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.37  BV=2.54
    {16659,1408,1024, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.46  BV=2.45
    {16659,1408,1096, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.56  BV=2.35
    {16659,1536,1080, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.66  BV=2.25
    {24996,1152,1024, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.17  BV=2.15
    {24996,1152,1096, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.27  BV=2.05
    {24996,1280,1056, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.37  BV=1.96
    {24996,1408,1032, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.47  BV=1.85
    {33332,1136,1024, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.15  BV=1.76
    {33332,1152,1088, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.26  BV=1.65
    {33332,1280,1048, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.36  BV=1.55
    {33332,1408,1024, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.46  BV=1.45
    {41669,1152,1072, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=5.24  BV=1.35
    {41669,1280,1032, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=5.33  BV=1.25
    {49991,1152,1024, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.17  BV=1.15
    {49991,1152,1096, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.27  BV=1.05
    {58328,1136,1024, 0, 0, 0},  //TV = 4.10(3932 lines)  AV=2.00  SV=5.15  BV=0.95
    {58328,1152,1088, 0, 0, 0},  //TV = 4.10(3932 lines)  AV=2.00  SV=5.26  BV=0.84
    {66664,1136,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=5.15  BV=0.76
    {66664,1152,1088, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=5.26  BV=0.65
    {75001,1152,1032, 0, 0, 0},  //TV = 3.74(5056 lines)  AV=2.00  SV=5.18  BV=0.56
    {75001,1152,1112, 0, 0, 0},  //TV = 3.74(5056 lines)  AV=2.00  SV=5.29  BV=0.45
    {83323,1152,1072, 0, 0, 0},  //TV = 3.59(5617 lines)  AV=2.00  SV=5.24  BV=0.35
    {91660,1152,1040, 0, 0, 0},  //TV = 3.45(6179 lines)  AV=2.00  SV=5.19  BV=0.26
    {99996,1152,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=5.17  BV=0.15
    {108333,1136,1024, 0, 0, 0},  //TV = 3.21(7303 lines)  AV=2.00  SV=5.15  BV=0.06
    {116655,1136,1024, 0, 0, 0},  //TV = 3.10(7864 lines)  AV=2.00  SV=5.15  BV=-0.05
    {116655,1136,1024, 0, 0, 0},  /* TV = 3.10(7864 lines)  AV=2.00  SV=5.15  BV=-0.05 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
},
};

static strEvPline sCaptureISO100PLineTable_50Hz =
{
{
    {30,1152,1104, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.28  BV=11.75
    {30,1280,1056, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.37  BV=11.66
    {30,1408,1040, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.48  BV=11.54
    {30,1536,1032, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.60  BV=11.43
    {30,1664,1024, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.70  BV=11.32
    {45,1152,1048, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.20  BV=11.24
    {45,1152,1120, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.30  BV=11.14
    {45,1280,1088, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.41  BV=11.03
    {45,1408,1040, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.48  BV=10.96
    {60,1152,1056, 0, 0, 0},  /* TV = 14.02(4 lines)  AV=2.00  SV=5.21  BV=10.81 */
    {60,1152,1128, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.31  BV=10.72
    {60,1280,1072, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.39  BV=10.64
    {75,1152,1032, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.18  BV=10.52
    {75,1152,1088, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.26  BV=10.45
    {75,1280,1056, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.37  BV=10.34
    {90,1152,1040, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.19  BV=10.25
    {90,1280,1024, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.32  BV=10.12
    {104,1152,1040, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.19  BV=10.04
    {104,1152,1104, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.28  BV=9.95
    {119,1152,1032, 0, 0, 0},  //TV = 13.04(8 lines)  AV=2.00  SV=5.18  BV=9.86
    {134,1136,1024, 0, 0, 0},  /* TV = 12.87(9 lines)  AV=2.00  SV=5.15  BV=9.72 */
    {134,1152,1064, 0, 0, 0},  //TV = 12.87(9 lines)  AV=2.00  SV=5.23  BV=9.64
    {149,1152,1032, 0, 0, 0},  //TV = 12.71(10 lines)  AV=2.00  SV=5.18  BV=9.53
    {164,1136,1024, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.15  BV=9.42
    {164,1152,1088, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.26  BV=9.32
    {179,1152,1040, 0, 0, 0},  /* TV = 12.45(12 lines)  AV=2.00  SV=5.19  BV=9.26 */
    {193,1152,1032, 0, 0, 0},  //TV = 12.34(13 lines)  AV=2.00  SV=5.18  BV=9.16
    {208,1152,1032, 0, 0, 0},  //TV = 12.23(14 lines)  AV=2.00  SV=5.18  BV=9.05
    {223,1152,1032, 0, 0, 0},  //TV = 12.13(15 lines)  AV=2.00  SV=5.18  BV=8.95
    {238,1152,1032, 0, 0, 0},  //TV = 12.04(16 lines)  AV=2.00  SV=5.18  BV=8.86
    {253,1152,1040, 0, 0, 0},  //TV = 11.95(17 lines)  AV=2.00  SV=5.19  BV=8.76
    {268,1152,1056, 0, 0, 0},  //TV = 11.87(18 lines)  AV=2.00  SV=5.21  BV=8.65
    {297,1152,1024, 0, 0, 0},  //TV = 11.72(20 lines)  AV=2.00  SV=5.17  BV=8.55
    {312,1152,1040, 0, 0, 0},  //TV = 11.65(21 lines)  AV=2.00  SV=5.19  BV=8.45
    {342,1152,1024, 0, 0, 0},  //TV = 11.51(23 lines)  AV=2.00  SV=5.17  BV=8.34
    {371,1136,1024, 0, 0, 0},  //TV = 11.40(25 lines)  AV=2.00  SV=5.15  BV=8.25
    {386,1152,1032, 0, 0, 0},  //TV = 11.34(26 lines)  AV=2.00  SV=5.18  BV=8.16
    {416,1152,1032, 0, 0, 0},  //TV = 11.23(28 lines)  AV=2.00  SV=5.18  BV=8.05
    {446,1152,1032, 0, 0, 0},  //TV = 11.13(30 lines)  AV=2.00  SV=5.18  BV=7.95
    {475,1152,1032, 0, 0, 0},  //TV = 11.04(32 lines)  AV=2.00  SV=5.18  BV=7.86
    {520,1136,1024, 0, 0, 0},  //TV = 10.91(35 lines)  AV=2.00  SV=5.15  BV=7.76
    {564,1136,1024, 0, 0, 0},  //TV = 10.79(38 lines)  AV=2.00  SV=5.15  BV=7.64
    {594,1152,1024, 0, 0, 0},  //TV = 10.72(40 lines)  AV=2.00  SV=5.17  BV=7.55
    {638,1152,1024, 0, 0, 0},  //TV = 10.61(43 lines)  AV=2.00  SV=5.17  BV=7.44
    {683,1152,1024, 0, 0, 0},  //TV = 10.52(46 lines)  AV=2.00  SV=5.17  BV=7.35
    {727,1152,1024, 0, 0, 0},  //TV = 10.43(49 lines)  AV=2.00  SV=5.17  BV=7.26
    {787,1152,1024, 0, 0, 0},  //TV = 10.31(53 lines)  AV=2.00  SV=5.17  BV=7.14
    {846,1136,1024, 0, 0, 0},  //TV = 10.21(57 lines)  AV=2.00  SV=5.15  BV=7.06
    {905,1136,1024, 0, 0, 0},  //TV = 10.11(61 lines)  AV=2.00  SV=5.15  BV=6.96
    {965,1152,1024, 0, 0, 0},  //TV = 10.02(65 lines)  AV=2.00  SV=5.17  BV=6.85
    {1039,1152,1024, 0, 0, 0},  //TV = 9.91(70 lines)  AV=2.00  SV=5.17  BV=6.74
    {1128,1136,1024, 0, 0, 0},  //TV = 9.79(76 lines)  AV=2.00  SV=5.15  BV=6.64
    {1202,1136,1024, 0, 0, 0},  //TV = 9.70(81 lines)  AV=2.00  SV=5.15  BV=6.55
    {1291,1136,1024, 0, 0, 0},  //TV = 9.60(87 lines)  AV=2.00  SV=5.15  BV=6.45
    {1380,1136,1024, 0, 0, 0},  //TV = 9.50(93 lines)  AV=2.00  SV=5.15  BV=6.35
    {1484,1136,1024, 0, 0, 0},  //TV = 9.40(100 lines)  AV=2.00  SV=5.15  BV=6.25
    {1588,1136,1024, 0, 0, 0},  //TV = 9.30(107 lines)  AV=2.00  SV=5.15  BV=6.15
    {1692,1136,1024, 0, 0, 0},  //TV = 9.21(114 lines)  AV=2.00  SV=5.15  BV=6.06
    {1810,1136,1024, 0, 0, 0},  //TV = 9.11(122 lines)  AV=2.00  SV=5.15  BV=5.96
    {1959,1136,1024, 0, 0, 0},  //TV = 9.00(132 lines)  AV=2.00  SV=5.15  BV=5.85
    {2107,1136,1024, 0, 0, 0},  //TV = 8.89(142 lines)  AV=2.00  SV=5.15  BV=5.74
    {2255,1136,1024, 0, 0, 0},  //TV = 8.79(152 lines)  AV=2.00  SV=5.15  BV=5.64
    {2389,1136,1024, 0, 0, 0},  //TV = 8.71(161 lines)  AV=2.00  SV=5.15  BV=5.56
    {2582,1136,1024, 0, 0, 0},  //TV = 8.60(174 lines)  AV=2.00  SV=5.15  BV=5.45
    {2774,1136,1024, 0, 0, 0},  //TV = 8.49(187 lines)  AV=2.00  SV=5.15  BV=5.34
    {2967,1136,1024, 0, 0, 0},  //TV = 8.40(200 lines)  AV=2.00  SV=5.15  BV=5.25
    {3175,1136,1024, 0, 0, 0},  //TV = 8.30(214 lines)  AV=2.00  SV=5.15  BV=5.15
    {3397,1136,1024, 0, 0, 0},  //TV = 8.20(229 lines)  AV=2.00  SV=5.15  BV=5.05
    {3635,1136,1024, 0, 0, 0},  //TV = 8.10(245 lines)  AV=2.00  SV=5.15  BV=4.95
    {3932,1136,1024, 0, 0, 0},  //TV = 7.99(265 lines)  AV=2.00  SV=5.15  BV=4.84
    {4199,1136,1024, 0, 0, 0},  //TV = 7.90(283 lines)  AV=2.00  SV=5.15  BV=4.75
    {4495,1136,1024, 0, 0, 0},  //TV = 7.80(303 lines)  AV=2.00  SV=5.15  BV=4.65
    {4822,1136,1024, 0, 0, 0},  //TV = 7.70(325 lines)  AV=2.00  SV=5.15  BV=4.55
    {5163,1136,1024, 0, 0, 0},  //TV = 7.60(348 lines)  AV=2.00  SV=5.15  BV=4.45
    {5563,1136,1024, 0, 0, 0},  //TV = 7.49(375 lines)  AV=2.00  SV=5.15  BV=4.34
    {5949,1136,1024, 0, 0, 0},  //TV = 7.39(401 lines)  AV=2.00  SV=5.15  BV=4.24
    {6379,1136,1024, 0, 0, 0},  //TV = 7.29(430 lines)  AV=2.00  SV=5.15  BV=4.14
    {6824,1136,1024, 0, 0, 0},  //TV = 7.20(460 lines)  AV=2.00  SV=5.15  BV=4.05
    {7314,1136,1024, 0, 0, 0},  //TV = 7.10(493 lines)  AV=2.00  SV=5.15  BV=3.95
    {7833,1136,1024, 0, 0, 0},  //TV = 7.00(528 lines)  AV=2.00  SV=5.15  BV=3.85
    {8397,1136,1024, 0, 0, 0},  //TV = 6.90(566 lines)  AV=2.00  SV=5.15  BV=3.75
    {8990,1136,1024, 0, 0, 0},  //TV = 6.80(606 lines)  AV=2.00  SV=5.15  BV=3.65
    {9628,1136,1024, 0, 0, 0},  //TV = 6.70(649 lines)  AV=2.00  SV=5.15  BV=3.55
    {9999,1152,1040, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.19  BV=3.45
    {9999,1152,1112, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.29  BV=3.36
    {9999,1280,1080, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.40  BV=3.25
    {9999,1408,1056, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.50  BV=3.14
    {9999,1536,1032, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.60  BV=3.05
    {9999,1664,1024, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.70  BV=2.94
    {9999,1664,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.79  BV=2.86
    {9999,1792,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.89  BV=2.75
    {9999,1920,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.99  BV=2.65
    {9999,2176,1032, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=6.10  BV=2.55
    {19997,1152,1040, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.19  BV=2.45
    {19997,1152,1112, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.29  BV=2.36
    {19997,1280,1080, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.40  BV=2.25
    {19997,1408,1048, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.49  BV=2.15
    {19997,1536,1032, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.60  BV=2.05
    {19997,1664,1024, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.70  BV=1.94
    {29995,1152,1048, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.20  BV=1.86
    {29995,1152,1128, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.31  BV=1.75
    {29995,1280,1088, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.41  BV=1.65
    {29995,1408,1064, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.51  BV=1.54
    {39993,1152,1040, 0, 0, 0},  //TV = 4.64(2696 lines)  AV=2.00  SV=5.19  BV=1.45
    {39993,1152,1112, 0, 0, 0},  //TV = 4.64(2696 lines)  AV=2.00  SV=5.29  BV=1.36
    {39993,1280,1080, 0, 0, 0},  //TV = 4.64(2696 lines)  AV=2.00  SV=5.40  BV=1.25
    {50006,1152,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.17  BV=1.15
    {50006,1152,1096, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.27  BV=1.05
    {50006,1280,1056, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.37  BV=0.96
    {60004,1152,1048, 0, 0, 0},  //TV = 4.06(4045 lines)  AV=2.00  SV=5.20  BV=0.86
    {60004,1152,1128, 0, 0, 0},  //TV = 4.06(4045 lines)  AV=2.00  SV=5.31  BV=0.75
    {70002,1152,1032, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=5.18  BV=0.66
    {70002,1152,1120, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=5.30  BV=0.54
    {80000,1152,1040, 0, 0, 0},  //TV = 3.64(5393 lines)  AV=2.00  SV=5.19  BV=0.45
    {80000,1152,1112, 0, 0, 0},  //TV = 3.64(5393 lines)  AV=2.00  SV=5.29  BV=0.35
    {89998,1152,1064, 0, 0, 0},  //TV = 3.47(6067 lines)  AV=2.00  SV=5.23  BV=0.25
    {99996,1152,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=5.17  BV=0.15
    {99996,1152,1096, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=5.27  BV=0.05
    {109995,1152,1072, 0, 0, 0},  //TV = 3.18(7415 lines)  AV=2.00  SV=5.24  BV=-0.05
    {119993,1152,1048, 0, 0, 0},  //TV = 3.06(8089 lines)  AV=2.00  SV=5.20  BV=-0.14
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
},
};

static strAETable g_AE_CaptureISO100Table =
{
    AETABLE_CAPTURE_ISO100, //eAETableID
    120, //u4TotalIndex
    -20, //i4StrobeTrigerBV
    118, //i4MaxBV
    -2, //i4MinBV
    90, //i4EffectiveMaxBV
    0, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_100, //ISO
    sCaptureISO100PLineTable_60Hz,
    sCaptureISO100PLineTable_50Hz,
    NULL,
};
static strEvPline sCaptureISO200PLineTable_60Hz =
{
{
    {30,1152,1104, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.28  BV=11.75
    {30,1280,1056, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.37  BV=11.66
    {30,1408,1040, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.48  BV=11.54
    {30,1536,1032, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.60  BV=11.43
    {30,1664,1024, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.70  BV=11.32
    {45,1152,1048, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.20  BV=11.24
    {45,1152,1120, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.30  BV=11.14
    {45,1280,1088, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.41  BV=11.03
    {45,1408,1040, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.48  BV=10.96
    {60,1152,1056, 0, 0, 0},  /* TV = 14.02(4 lines)  AV=2.00  SV=5.21  BV=10.81 */
    {60,1152,1128, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.31  BV=10.72
    {60,1280,1072, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.39  BV=10.64
    {75,1152,1032, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.18  BV=10.52
    {75,1152,1088, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.26  BV=10.45
    {75,1280,1056, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.37  BV=10.34
    {90,1152,1040, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.19  BV=10.25
    {90,1280,1024, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.32  BV=10.12
    {104,1152,1040, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.19  BV=10.04
    {104,1152,1104, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.28  BV=9.95
    {119,1152,1032, 0, 0, 0},  //TV = 13.04(8 lines)  AV=2.00  SV=5.18  BV=9.86
    {134,1136,1024, 0, 0, 0},  /* TV = 12.87(9 lines)  AV=2.00  SV=5.15  BV=9.72 */
    {134,1152,1064, 0, 0, 0},  //TV = 12.87(9 lines)  AV=2.00  SV=5.23  BV=9.64
    {149,1152,1032, 0, 0, 0},  //TV = 12.71(10 lines)  AV=2.00  SV=5.18  BV=9.53
    {164,1136,1024, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.15  BV=9.42
    {164,1152,1088, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.26  BV=9.32
    {179,1152,1040, 0, 0, 0},  /* TV = 12.45(12 lines)  AV=2.00  SV=5.19  BV=9.26 */
    {193,1152,1032, 0, 0, 0},  //TV = 12.34(13 lines)  AV=2.00  SV=5.18  BV=9.16
    {208,1152,1032, 0, 0, 0},  //TV = 12.23(14 lines)  AV=2.00  SV=5.18  BV=9.05
    {223,1152,1032, 0, 0, 0},  //TV = 12.13(15 lines)  AV=2.00  SV=5.18  BV=8.95
    {238,1152,1032, 0, 0, 0},  //TV = 12.04(16 lines)  AV=2.00  SV=5.18  BV=8.86
    {253,1152,1040, 0, 0, 0},  //TV = 11.95(17 lines)  AV=2.00  SV=5.19  BV=8.76
    {268,1152,1056, 0, 0, 0},  //TV = 11.87(18 lines)  AV=2.00  SV=5.21  BV=8.65
    {297,1152,1024, 0, 0, 0},  //TV = 11.72(20 lines)  AV=2.00  SV=5.17  BV=8.55
    {312,1152,1040, 0, 0, 0},  //TV = 11.65(21 lines)  AV=2.00  SV=5.19  BV=8.45
    {342,1152,1024, 0, 0, 0},  //TV = 11.51(23 lines)  AV=2.00  SV=5.17  BV=8.34
    {371,1136,1024, 0, 0, 0},  //TV = 11.40(25 lines)  AV=2.00  SV=5.15  BV=8.25
    {386,1152,1032, 0, 0, 0},  //TV = 11.34(26 lines)  AV=2.00  SV=5.18  BV=8.16
    {416,1152,1032, 0, 0, 0},  //TV = 11.23(28 lines)  AV=2.00  SV=5.18  BV=8.05
    {446,1152,1032, 0, 0, 0},  //TV = 11.13(30 lines)  AV=2.00  SV=5.18  BV=7.95
    {475,1152,1032, 0, 0, 0},  //TV = 11.04(32 lines)  AV=2.00  SV=5.18  BV=7.86
    {520,1136,1024, 0, 0, 0},  //TV = 10.91(35 lines)  AV=2.00  SV=5.15  BV=7.76
    {564,1136,1024, 0, 0, 0},  //TV = 10.79(38 lines)  AV=2.00  SV=5.15  BV=7.64
    {594,1152,1024, 0, 0, 0},  //TV = 10.72(40 lines)  AV=2.00  SV=5.17  BV=7.55
    {638,1152,1024, 0, 0, 0},  //TV = 10.61(43 lines)  AV=2.00  SV=5.17  BV=7.44
    {683,1152,1024, 0, 0, 0},  //TV = 10.52(46 lines)  AV=2.00  SV=5.17  BV=7.35
    {727,1152,1024, 0, 0, 0},  //TV = 10.43(49 lines)  AV=2.00  SV=5.17  BV=7.26
    {787,1152,1024, 0, 0, 0},  //TV = 10.31(53 lines)  AV=2.00  SV=5.17  BV=7.14
    {846,1136,1024, 0, 0, 0},  //TV = 10.21(57 lines)  AV=2.00  SV=5.15  BV=7.06
    {905,1136,1024, 0, 0, 0},  //TV = 10.11(61 lines)  AV=2.00  SV=5.15  BV=6.96
    {965,1152,1024, 0, 0, 0},  //TV = 10.02(65 lines)  AV=2.00  SV=5.17  BV=6.85
    {1039,1152,1024, 0, 0, 0},  //TV = 9.91(70 lines)  AV=2.00  SV=5.17  BV=6.74
    {1128,1136,1024, 0, 0, 0},  //TV = 9.79(76 lines)  AV=2.00  SV=5.15  BV=6.64
    {1202,1136,1024, 0, 0, 0},  //TV = 9.70(81 lines)  AV=2.00  SV=5.15  BV=6.55
    {1291,1136,1024, 0, 0, 0},  //TV = 9.60(87 lines)  AV=2.00  SV=5.15  BV=6.45
    {1380,1136,1024, 0, 0, 0},  //TV = 9.50(93 lines)  AV=2.00  SV=5.15  BV=6.35
    {1484,1136,1024, 0, 0, 0},  //TV = 9.40(100 lines)  AV=2.00  SV=5.15  BV=6.25
    {1588,1136,1024, 0, 0, 0},  //TV = 9.30(107 lines)  AV=2.00  SV=5.15  BV=6.15
    {1692,1136,1024, 0, 0, 0},  //TV = 9.21(114 lines)  AV=2.00  SV=5.15  BV=6.06
    {1810,1136,1024, 0, 0, 0},  //TV = 9.11(122 lines)  AV=2.00  SV=5.15  BV=5.96
    {1959,1136,1024, 0, 0, 0},  //TV = 9.00(132 lines)  AV=2.00  SV=5.15  BV=5.85
    {2107,1136,1024, 0, 0, 0},  //TV = 8.89(142 lines)  AV=2.00  SV=5.15  BV=5.74
    {2255,1136,1024, 0, 0, 0},  //TV = 8.79(152 lines)  AV=2.00  SV=5.15  BV=5.64
    {2389,1136,1024, 0, 0, 0},  //TV = 8.71(161 lines)  AV=2.00  SV=5.15  BV=5.56
    {2582,1136,1024, 0, 0, 0},  //TV = 8.60(174 lines)  AV=2.00  SV=5.15  BV=5.45
    {2774,1136,1024, 0, 0, 0},  //TV = 8.49(187 lines)  AV=2.00  SV=5.15  BV=5.34
    {2967,1136,1024, 0, 0, 0},  //TV = 8.40(200 lines)  AV=2.00  SV=5.15  BV=5.25
    {3175,1136,1024, 0, 0, 0},  //TV = 8.30(214 lines)  AV=2.00  SV=5.15  BV=5.15
    {3397,1136,1024, 0, 0, 0},  //TV = 8.20(229 lines)  AV=2.00  SV=5.15  BV=5.05
    {3635,1136,1024, 0, 0, 0},  //TV = 8.10(245 lines)  AV=2.00  SV=5.15  BV=4.95
    {3932,1136,1024, 0, 0, 0},  //TV = 7.99(265 lines)  AV=2.00  SV=5.15  BV=4.84
    {4199,1136,1024, 0, 0, 0},  //TV = 7.90(283 lines)  AV=2.00  SV=5.15  BV=4.75
    {4495,1136,1024, 0, 0, 0},  //TV = 7.80(303 lines)  AV=2.00  SV=5.15  BV=4.65
    {4822,1136,1024, 0, 0, 0},  //TV = 7.70(325 lines)  AV=2.00  SV=5.15  BV=4.55
    {5163,1136,1024, 0, 0, 0},  //TV = 7.60(348 lines)  AV=2.00  SV=5.15  BV=4.45
    {5563,1136,1024, 0, 0, 0},  //TV = 7.49(375 lines)  AV=2.00  SV=5.15  BV=4.34
    {5949,1136,1024, 0, 0, 0},  //TV = 7.39(401 lines)  AV=2.00  SV=5.15  BV=4.24
    {6379,1136,1024, 0, 0, 0},  //TV = 7.29(430 lines)  AV=2.00  SV=5.15  BV=4.14
    {6824,1136,1024, 0, 0, 0},  //TV = 7.20(460 lines)  AV=2.00  SV=5.15  BV=4.05
    {7314,1136,1024, 0, 0, 0},  //TV = 7.10(493 lines)  AV=2.00  SV=5.15  BV=3.95
    {7833,1136,1024, 0, 0, 0},  //TV = 7.00(528 lines)  AV=2.00  SV=5.15  BV=3.85
    {8337,1136,1024, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.15  BV=3.76
    {8337,1152,1088, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.26  BV=3.65
    {8337,1280,1048, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.36  BV=3.55
    {8337,1408,1024, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.46  BV=3.45
    {8337,1408,1104, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.57  BV=3.34
    {8337,1536,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.65  BV=3.26
    {8337,1664,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.77  BV=3.14
    {8337,1792,1056, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.85  BV=3.05
    {8337,1920,1064, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.96  BV=2.94
    {8337,2048,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=6.07  BV=2.84
    {16659,1136,1024, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.15  BV=2.76
    {16659,1152,1088, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.26  BV=2.65
    {16659,1280,1056, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.37  BV=2.54
    {16659,1408,1024, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.46  BV=2.45
    {16659,1408,1096, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.56  BV=2.35
    {16659,1536,1080, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.66  BV=2.25
    {24996,1152,1024, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.17  BV=2.15
    {24996,1152,1096, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.27  BV=2.05
    {24996,1280,1056, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.37  BV=1.96
    {24996,1408,1032, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.47  BV=1.85
    {33332,1136,1024, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.15  BV=1.76
    {33332,1152,1088, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.26  BV=1.65
    {33332,1280,1048, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.36  BV=1.55
    {33332,1408,1024, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.46  BV=1.45
    {41669,1152,1072, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=5.24  BV=1.35
    {41669,1280,1032, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=5.33  BV=1.25
    {49991,1152,1024, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.17  BV=1.15
    {49991,1152,1096, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.27  BV=1.05
    {49991,1280,1056, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.37  BV=0.96
    {49991,1408,1032, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.47  BV=0.85
    {49991,1408,1104, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.57  BV=0.75
    {49991,1536,1088, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.67  BV=0.65
    {49991,1664,1072, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.77  BV=0.56
    {49991,1792,1072, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.87  BV=0.45
    {49991,1920,1072, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.97  BV=0.35
    {50006,2048,1080, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.08  BV=0.24
    {58328,1920,1056, 0, 0, 0},  //TV = 4.10(3932 lines)  AV=2.00  SV=5.95  BV=0.15
    {58328,2048,1056, 0, 0, 0},  //TV = 4.10(3932 lines)  AV=2.00  SV=6.04  BV=0.06
    {66664,1920,1064, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=5.96  BV=-0.06
    {66664,2048,1072, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=6.07  BV=-0.16
    {75001,1920,1088, 0, 0, 0},  //TV = 3.74(5056 lines)  AV=2.00  SV=5.99  BV=-0.26
    {83338,1920,1048, 0, 0, 0},  //TV = 3.58(5618 lines)  AV=2.00  SV=5.94  BV=-0.36
    {83338,2048,1056, 0, 0, 0},  //TV = 3.58(5618 lines)  AV=2.00  SV=6.04  BV=-0.46
    {91660,2048,1024, 0, 0, 0},  //TV = 3.45(6179 lines)  AV=2.00  SV=6.00  BV=-0.55
    {99996,1920,1072, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=5.97  BV=-0.65
    {108333,1920,1064, 0, 0, 0},  //TV = 3.21(7303 lines)  AV=2.00  SV=5.96  BV=-0.76
    {116670,1920,1056, 0, 0, 0},  //TV = 3.10(7865 lines)  AV=2.00  SV=5.95  BV=-0.85
    {124992,1920,1056, 0, 0, 0},  //TV = 3.00(8426 lines)  AV=2.00  SV=5.95  BV=-0.95
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
},
};

static strEvPline sCaptureISO200PLineTable_50Hz =
{
{
    {30,1152,1104, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.28  BV=11.75
    {30,1280,1056, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.37  BV=11.66
    {30,1408,1040, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.48  BV=11.54
    {30,1536,1032, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.60  BV=11.43
    {30,1664,1024, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.70  BV=11.32
    {45,1152,1048, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.20  BV=11.24
    {45,1152,1120, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.30  BV=11.14
    {45,1280,1088, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.41  BV=11.03
    {45,1408,1040, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.48  BV=10.96
    {60,1152,1056, 0, 0, 0},  /* TV = 14.02(4 lines)  AV=2.00  SV=5.21  BV=10.81 */
    {60,1152,1128, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.31  BV=10.72
    {60,1280,1072, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.39  BV=10.64
    {75,1152,1032, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.18  BV=10.52
    {75,1152,1088, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.26  BV=10.45
    {75,1280,1056, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.37  BV=10.34
    {90,1152,1040, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.19  BV=10.25
    {90,1280,1024, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.32  BV=10.12
    {104,1152,1040, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.19  BV=10.04
    {104,1152,1104, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.28  BV=9.95
    {119,1152,1032, 0, 0, 0},  //TV = 13.04(8 lines)  AV=2.00  SV=5.18  BV=9.86
    {134,1136,1024, 0, 0, 0},  /* TV = 12.87(9 lines)  AV=2.00  SV=5.15  BV=9.72 */
    {134,1152,1064, 0, 0, 0},  //TV = 12.87(9 lines)  AV=2.00  SV=5.23  BV=9.64
    {149,1152,1032, 0, 0, 0},  //TV = 12.71(10 lines)  AV=2.00  SV=5.18  BV=9.53
    {164,1136,1024, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.15  BV=9.42
    {164,1152,1088, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.26  BV=9.32
    {179,1152,1040, 0, 0, 0},  /* TV = 12.45(12 lines)  AV=2.00  SV=5.19  BV=9.26 */
    {193,1152,1032, 0, 0, 0},  //TV = 12.34(13 lines)  AV=2.00  SV=5.18  BV=9.16
    {208,1152,1032, 0, 0, 0},  //TV = 12.23(14 lines)  AV=2.00  SV=5.18  BV=9.05
    {223,1152,1032, 0, 0, 0},  //TV = 12.13(15 lines)  AV=2.00  SV=5.18  BV=8.95
    {238,1152,1032, 0, 0, 0},  //TV = 12.04(16 lines)  AV=2.00  SV=5.18  BV=8.86
    {253,1152,1040, 0, 0, 0},  //TV = 11.95(17 lines)  AV=2.00  SV=5.19  BV=8.76
    {268,1152,1056, 0, 0, 0},  //TV = 11.87(18 lines)  AV=2.00  SV=5.21  BV=8.65
    {297,1152,1024, 0, 0, 0},  //TV = 11.72(20 lines)  AV=2.00  SV=5.17  BV=8.55
    {312,1152,1040, 0, 0, 0},  //TV = 11.65(21 lines)  AV=2.00  SV=5.19  BV=8.45
    {342,1152,1024, 0, 0, 0},  //TV = 11.51(23 lines)  AV=2.00  SV=5.17  BV=8.34
    {371,1136,1024, 0, 0, 0},  //TV = 11.40(25 lines)  AV=2.00  SV=5.15  BV=8.25
    {386,1152,1032, 0, 0, 0},  //TV = 11.34(26 lines)  AV=2.00  SV=5.18  BV=8.16
    {416,1152,1032, 0, 0, 0},  //TV = 11.23(28 lines)  AV=2.00  SV=5.18  BV=8.05
    {446,1152,1032, 0, 0, 0},  //TV = 11.13(30 lines)  AV=2.00  SV=5.18  BV=7.95
    {475,1152,1032, 0, 0, 0},  //TV = 11.04(32 lines)  AV=2.00  SV=5.18  BV=7.86
    {520,1136,1024, 0, 0, 0},  //TV = 10.91(35 lines)  AV=2.00  SV=5.15  BV=7.76
    {564,1136,1024, 0, 0, 0},  //TV = 10.79(38 lines)  AV=2.00  SV=5.15  BV=7.64
    {594,1152,1024, 0, 0, 0},  //TV = 10.72(40 lines)  AV=2.00  SV=5.17  BV=7.55
    {638,1152,1024, 0, 0, 0},  //TV = 10.61(43 lines)  AV=2.00  SV=5.17  BV=7.44
    {683,1152,1024, 0, 0, 0},  //TV = 10.52(46 lines)  AV=2.00  SV=5.17  BV=7.35
    {727,1152,1024, 0, 0, 0},  //TV = 10.43(49 lines)  AV=2.00  SV=5.17  BV=7.26
    {787,1152,1024, 0, 0, 0},  //TV = 10.31(53 lines)  AV=2.00  SV=5.17  BV=7.14
    {846,1136,1024, 0, 0, 0},  //TV = 10.21(57 lines)  AV=2.00  SV=5.15  BV=7.06
    {905,1136,1024, 0, 0, 0},  //TV = 10.11(61 lines)  AV=2.00  SV=5.15  BV=6.96
    {965,1152,1024, 0, 0, 0},  //TV = 10.02(65 lines)  AV=2.00  SV=5.17  BV=6.85
    {1039,1152,1024, 0, 0, 0},  //TV = 9.91(70 lines)  AV=2.00  SV=5.17  BV=6.74
    {1128,1136,1024, 0, 0, 0},  //TV = 9.79(76 lines)  AV=2.00  SV=5.15  BV=6.64
    {1202,1136,1024, 0, 0, 0},  //TV = 9.70(81 lines)  AV=2.00  SV=5.15  BV=6.55
    {1291,1136,1024, 0, 0, 0},  //TV = 9.60(87 lines)  AV=2.00  SV=5.15  BV=6.45
    {1380,1136,1024, 0, 0, 0},  //TV = 9.50(93 lines)  AV=2.00  SV=5.15  BV=6.35
    {1484,1136,1024, 0, 0, 0},  //TV = 9.40(100 lines)  AV=2.00  SV=5.15  BV=6.25
    {1588,1136,1024, 0, 0, 0},  //TV = 9.30(107 lines)  AV=2.00  SV=5.15  BV=6.15
    {1692,1136,1024, 0, 0, 0},  //TV = 9.21(114 lines)  AV=2.00  SV=5.15  BV=6.06
    {1810,1136,1024, 0, 0, 0},  //TV = 9.11(122 lines)  AV=2.00  SV=5.15  BV=5.96
    {1959,1136,1024, 0, 0, 0},  //TV = 9.00(132 lines)  AV=2.00  SV=5.15  BV=5.85
    {2107,1136,1024, 0, 0, 0},  //TV = 8.89(142 lines)  AV=2.00  SV=5.15  BV=5.74
    {2255,1136,1024, 0, 0, 0},  //TV = 8.79(152 lines)  AV=2.00  SV=5.15  BV=5.64
    {2389,1136,1024, 0, 0, 0},  //TV = 8.71(161 lines)  AV=2.00  SV=5.15  BV=5.56
    {2582,1136,1024, 0, 0, 0},  //TV = 8.60(174 lines)  AV=2.00  SV=5.15  BV=5.45
    {2774,1136,1024, 0, 0, 0},  //TV = 8.49(187 lines)  AV=2.00  SV=5.15  BV=5.34
    {2967,1136,1024, 0, 0, 0},  //TV = 8.40(200 lines)  AV=2.00  SV=5.15  BV=5.25
    {3175,1136,1024, 0, 0, 0},  //TV = 8.30(214 lines)  AV=2.00  SV=5.15  BV=5.15
    {3397,1136,1024, 0, 0, 0},  //TV = 8.20(229 lines)  AV=2.00  SV=5.15  BV=5.05
    {3635,1136,1024, 0, 0, 0},  //TV = 8.10(245 lines)  AV=2.00  SV=5.15  BV=4.95
    {3932,1136,1024, 0, 0, 0},  //TV = 7.99(265 lines)  AV=2.00  SV=5.15  BV=4.84
    {4199,1136,1024, 0, 0, 0},  //TV = 7.90(283 lines)  AV=2.00  SV=5.15  BV=4.75
    {4495,1136,1024, 0, 0, 0},  //TV = 7.80(303 lines)  AV=2.00  SV=5.15  BV=4.65
    {4822,1136,1024, 0, 0, 0},  //TV = 7.70(325 lines)  AV=2.00  SV=5.15  BV=4.55
    {5163,1136,1024, 0, 0, 0},  //TV = 7.60(348 lines)  AV=2.00  SV=5.15  BV=4.45
    {5563,1136,1024, 0, 0, 0},  //TV = 7.49(375 lines)  AV=2.00  SV=5.15  BV=4.34
    {5949,1136,1024, 0, 0, 0},  //TV = 7.39(401 lines)  AV=2.00  SV=5.15  BV=4.24
    {6379,1136,1024, 0, 0, 0},  //TV = 7.29(430 lines)  AV=2.00  SV=5.15  BV=4.14
    {6824,1136,1024, 0, 0, 0},  //TV = 7.20(460 lines)  AV=2.00  SV=5.15  BV=4.05
    {7314,1136,1024, 0, 0, 0},  //TV = 7.10(493 lines)  AV=2.00  SV=5.15  BV=3.95
    {7833,1136,1024, 0, 0, 0},  //TV = 7.00(528 lines)  AV=2.00  SV=5.15  BV=3.85
    {8397,1136,1024, 0, 0, 0},  //TV = 6.90(566 lines)  AV=2.00  SV=5.15  BV=3.75
    {8990,1136,1024, 0, 0, 0},  //TV = 6.80(606 lines)  AV=2.00  SV=5.15  BV=3.65
    {9628,1136,1024, 0, 0, 0},  //TV = 6.70(649 lines)  AV=2.00  SV=5.15  BV=3.55
    {9999,1152,1040, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.19  BV=3.45
    {9999,1152,1112, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.29  BV=3.36
    {9999,1280,1080, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.40  BV=3.25
    {9999,1408,1056, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.50  BV=3.14
    {9999,1536,1032, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.60  BV=3.05
    {9999,1664,1024, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.70  BV=2.94
    {9999,1664,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.79  BV=2.86
    {9999,1792,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.89  BV=2.75
    {9999,1920,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.99  BV=2.65
    {9999,2176,1032, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=6.10  BV=2.55
    {19997,1152,1040, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.19  BV=2.45
    {19997,1152,1112, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.29  BV=2.36
    {19997,1280,1080, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.40  BV=2.25
    {19997,1408,1048, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.49  BV=2.15
    {19997,1536,1032, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.60  BV=2.05
    {19997,1664,1024, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.70  BV=1.94
    {29995,1152,1048, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.20  BV=1.86
    {29995,1152,1128, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.31  BV=1.75
    {29995,1280,1088, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.41  BV=1.65
    {29995,1408,1064, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.51  BV=1.54
    {39993,1152,1040, 0, 0, 0},  //TV = 4.64(2696 lines)  AV=2.00  SV=5.19  BV=1.45
    {39993,1152,1112, 0, 0, 0},  //TV = 4.64(2696 lines)  AV=2.00  SV=5.29  BV=1.36
    {39993,1280,1080, 0, 0, 0},  //TV = 4.64(2696 lines)  AV=2.00  SV=5.40  BV=1.25
    {50006,1152,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.17  BV=1.15
    {50006,1152,1096, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.27  BV=1.05
    {50006,1280,1056, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.37  BV=0.96
    {50006,1408,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.47  BV=0.85
    {50006,1408,1104, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.57  BV=0.75
    {50006,1536,1088, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.67  BV=0.65
    {50006,1664,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.77  BV=0.56
    {50006,1792,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.87  BV=0.45
    {50006,1920,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.97  BV=0.35
    {50006,2048,1080, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.08  BV=0.24
    {60004,1920,1032, 0, 0, 0},  //TV = 4.06(4045 lines)  AV=2.00  SV=5.92  BV=0.14
    {60004,2048,1032, 0, 0, 0},  //TV = 4.06(4045 lines)  AV=2.00  SV=6.01  BV=0.05
    {70002,1792,1088, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=5.89  BV=-0.06
    {70002,1920,1088, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=5.99  BV=-0.16
    {70002,2176,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=6.09  BV=-0.25
    {80000,2048,1024, 0, 0, 0},  //TV = 3.64(5393 lines)  AV=2.00  SV=6.00  BV=-0.36
    {89998,1920,1040, 0, 0, 0},  //TV = 3.47(6067 lines)  AV=2.00  SV=5.93  BV=-0.46
    {89998,2048,1048, 0, 0, 0},  //TV = 3.47(6067 lines)  AV=2.00  SV=6.03  BV=-0.56
    {99996,1920,1072, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=5.97  BV=-0.65
    {109995,1920,1048, 0, 0, 0},  //TV = 3.18(7415 lines)  AV=2.00  SV=5.94  BV=-0.76
    {109995,2048,1048, 0, 0, 0},  //TV = 3.18(7415 lines)  AV=2.00  SV=6.03  BV=-0.85
    {120008,2048,1032, 0, 0, 0},  //TV = 3.06(8090 lines)  AV=2.00  SV=6.01  BV=-0.95
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
},
};

static strAETable g_AE_CaptureISO200Table =
{
    AETABLE_CAPTURE_ISO200, //eAETableID
    128, //u4TotalIndex
    -20, //i4StrobeTrigerBV
    118, //i4MaxBV
    -10, //i4MinBV
    90, //i4EffectiveMaxBV
    0, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_200, //ISO
    sCaptureISO200PLineTable_60Hz,
    sCaptureISO200PLineTable_50Hz,
    NULL,
};
static strEvPline sCaptureISO400PLineTable_60Hz =
{
{
    {30,1152,1104, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.28  BV=11.75
    {30,1280,1056, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.37  BV=11.66
    {30,1408,1040, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.48  BV=11.54
    {30,1536,1032, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.60  BV=11.43
    {30,1664,1024, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.70  BV=11.32
    {45,1152,1048, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.20  BV=11.24
    {45,1152,1120, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.30  BV=11.14
    {45,1280,1088, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.41  BV=11.03
    {45,1408,1040, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.48  BV=10.96
    {60,1152,1056, 0, 0, 0},  /* TV = 14.02(4 lines)  AV=2.00  SV=5.21  BV=10.81 */
    {60,1152,1128, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.31  BV=10.72
    {60,1280,1072, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.39  BV=10.64
    {75,1152,1032, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.18  BV=10.52
    {75,1152,1088, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.26  BV=10.45
    {75,1280,1056, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.37  BV=10.34
    {90,1152,1040, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.19  BV=10.25
    {90,1280,1024, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.32  BV=10.12
    {104,1152,1040, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.19  BV=10.04
    {104,1152,1104, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.28  BV=9.95
    {119,1152,1032, 0, 0, 0},  //TV = 13.04(8 lines)  AV=2.00  SV=5.18  BV=9.86
    {134,1136,1024, 0, 0, 0},  /* TV = 12.87(9 lines)  AV=2.00  SV=5.15  BV=9.72 */
    {134,1152,1064, 0, 0, 0},  //TV = 12.87(9 lines)  AV=2.00  SV=5.23  BV=9.64
    {149,1152,1032, 0, 0, 0},  //TV = 12.71(10 lines)  AV=2.00  SV=5.18  BV=9.53
    {164,1136,1024, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.15  BV=9.42
    {164,1152,1088, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.26  BV=9.32
    {179,1152,1040, 0, 0, 0},  /* TV = 12.45(12 lines)  AV=2.00  SV=5.19  BV=9.26 */
    {193,1152,1032, 0, 0, 0},  //TV = 12.34(13 lines)  AV=2.00  SV=5.18  BV=9.16
    {208,1152,1032, 0, 0, 0},  //TV = 12.23(14 lines)  AV=2.00  SV=5.18  BV=9.05
    {223,1152,1032, 0, 0, 0},  //TV = 12.13(15 lines)  AV=2.00  SV=5.18  BV=8.95
    {238,1152,1032, 0, 0, 0},  //TV = 12.04(16 lines)  AV=2.00  SV=5.18  BV=8.86
    {253,1152,1040, 0, 0, 0},  //TV = 11.95(17 lines)  AV=2.00  SV=5.19  BV=8.76
    {268,1152,1056, 0, 0, 0},  //TV = 11.87(18 lines)  AV=2.00  SV=5.21  BV=8.65
    {297,1152,1024, 0, 0, 0},  //TV = 11.72(20 lines)  AV=2.00  SV=5.17  BV=8.55
    {312,1152,1040, 0, 0, 0},  //TV = 11.65(21 lines)  AV=2.00  SV=5.19  BV=8.45
    {342,1152,1024, 0, 0, 0},  //TV = 11.51(23 lines)  AV=2.00  SV=5.17  BV=8.34
    {371,1136,1024, 0, 0, 0},  //TV = 11.40(25 lines)  AV=2.00  SV=5.15  BV=8.25
    {386,1152,1032, 0, 0, 0},  //TV = 11.34(26 lines)  AV=2.00  SV=5.18  BV=8.16
    {416,1152,1032, 0, 0, 0},  //TV = 11.23(28 lines)  AV=2.00  SV=5.18  BV=8.05
    {446,1152,1032, 0, 0, 0},  //TV = 11.13(30 lines)  AV=2.00  SV=5.18  BV=7.95
    {475,1152,1032, 0, 0, 0},  //TV = 11.04(32 lines)  AV=2.00  SV=5.18  BV=7.86
    {520,1136,1024, 0, 0, 0},  //TV = 10.91(35 lines)  AV=2.00  SV=5.15  BV=7.76
    {564,1136,1024, 0, 0, 0},  //TV = 10.79(38 lines)  AV=2.00  SV=5.15  BV=7.64
    {594,1152,1024, 0, 0, 0},  //TV = 10.72(40 lines)  AV=2.00  SV=5.17  BV=7.55
    {638,1152,1024, 0, 0, 0},  //TV = 10.61(43 lines)  AV=2.00  SV=5.17  BV=7.44
    {683,1152,1024, 0, 0, 0},  //TV = 10.52(46 lines)  AV=2.00  SV=5.17  BV=7.35
    {727,1152,1024, 0, 0, 0},  //TV = 10.43(49 lines)  AV=2.00  SV=5.17  BV=7.26
    {787,1152,1024, 0, 0, 0},  //TV = 10.31(53 lines)  AV=2.00  SV=5.17  BV=7.14
    {846,1136,1024, 0, 0, 0},  //TV = 10.21(57 lines)  AV=2.00  SV=5.15  BV=7.06
    {905,1136,1024, 0, 0, 0},  //TV = 10.11(61 lines)  AV=2.00  SV=5.15  BV=6.96
    {965,1152,1024, 0, 0, 0},  //TV = 10.02(65 lines)  AV=2.00  SV=5.17  BV=6.85
    {1039,1152,1024, 0, 0, 0},  //TV = 9.91(70 lines)  AV=2.00  SV=5.17  BV=6.74
    {1128,1136,1024, 0, 0, 0},  //TV = 9.79(76 lines)  AV=2.00  SV=5.15  BV=6.64
    {1202,1136,1024, 0, 0, 0},  //TV = 9.70(81 lines)  AV=2.00  SV=5.15  BV=6.55
    {1291,1136,1024, 0, 0, 0},  //TV = 9.60(87 lines)  AV=2.00  SV=5.15  BV=6.45
    {1380,1136,1024, 0, 0, 0},  //TV = 9.50(93 lines)  AV=2.00  SV=5.15  BV=6.35
    {1484,1136,1024, 0, 0, 0},  //TV = 9.40(100 lines)  AV=2.00  SV=5.15  BV=6.25
    {1588,1136,1024, 0, 0, 0},  //TV = 9.30(107 lines)  AV=2.00  SV=5.15  BV=6.15
    {1692,1136,1024, 0, 0, 0},  //TV = 9.21(114 lines)  AV=2.00  SV=5.15  BV=6.06
    {1810,1136,1024, 0, 0, 0},  //TV = 9.11(122 lines)  AV=2.00  SV=5.15  BV=5.96
    {1959,1136,1024, 0, 0, 0},  //TV = 9.00(132 lines)  AV=2.00  SV=5.15  BV=5.85
    {2107,1136,1024, 0, 0, 0},  //TV = 8.89(142 lines)  AV=2.00  SV=5.15  BV=5.74
    {2255,1136,1024, 0, 0, 0},  //TV = 8.79(152 lines)  AV=2.00  SV=5.15  BV=5.64
    {2389,1136,1024, 0, 0, 0},  //TV = 8.71(161 lines)  AV=2.00  SV=5.15  BV=5.56
    {2582,1136,1024, 0, 0, 0},  //TV = 8.60(174 lines)  AV=2.00  SV=5.15  BV=5.45
    {2774,1136,1024, 0, 0, 0},  //TV = 8.49(187 lines)  AV=2.00  SV=5.15  BV=5.34
    {2967,1136,1024, 0, 0, 0},  //TV = 8.40(200 lines)  AV=2.00  SV=5.15  BV=5.25
    {3175,1136,1024, 0, 0, 0},  //TV = 8.30(214 lines)  AV=2.00  SV=5.15  BV=5.15
    {3397,1136,1024, 0, 0, 0},  //TV = 8.20(229 lines)  AV=2.00  SV=5.15  BV=5.05
    {3635,1136,1024, 0, 0, 0},  //TV = 8.10(245 lines)  AV=2.00  SV=5.15  BV=4.95
    {3932,1136,1024, 0, 0, 0},  //TV = 7.99(265 lines)  AV=2.00  SV=5.15  BV=4.84
    {4199,1136,1024, 0, 0, 0},  //TV = 7.90(283 lines)  AV=2.00  SV=5.15  BV=4.75
    {4495,1136,1024, 0, 0, 0},  //TV = 7.80(303 lines)  AV=2.00  SV=5.15  BV=4.65
    {4822,1136,1024, 0, 0, 0},  //TV = 7.70(325 lines)  AV=2.00  SV=5.15  BV=4.55
    {5163,1136,1024, 0, 0, 0},  //TV = 7.60(348 lines)  AV=2.00  SV=5.15  BV=4.45
    {5563,1136,1024, 0, 0, 0},  //TV = 7.49(375 lines)  AV=2.00  SV=5.15  BV=4.34
    {5949,1136,1024, 0, 0, 0},  //TV = 7.39(401 lines)  AV=2.00  SV=5.15  BV=4.24
    {6379,1136,1024, 0, 0, 0},  //TV = 7.29(430 lines)  AV=2.00  SV=5.15  BV=4.14
    {6824,1136,1024, 0, 0, 0},  //TV = 7.20(460 lines)  AV=2.00  SV=5.15  BV=4.05
    {7314,1136,1024, 0, 0, 0},  //TV = 7.10(493 lines)  AV=2.00  SV=5.15  BV=3.95
    {7833,1136,1024, 0, 0, 0},  //TV = 7.00(528 lines)  AV=2.00  SV=5.15  BV=3.85
    {8337,1136,1024, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.15  BV=3.76
    {8337,1152,1088, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.26  BV=3.65
    {8337,1280,1048, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.36  BV=3.55
    {8337,1408,1024, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.46  BV=3.45
    {8337,1408,1104, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.57  BV=3.34
    {8337,1536,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.65  BV=3.26
    {8337,1664,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.77  BV=3.14
    {8337,1792,1056, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.85  BV=3.05
    {8337,1920,1064, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.96  BV=2.94
    {8337,2048,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=6.07  BV=2.84
    {16659,1136,1024, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.15  BV=2.76
    {16659,1152,1088, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.26  BV=2.65
    {16659,1280,1056, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.37  BV=2.54
    {16659,1408,1024, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.46  BV=2.45
    {16659,1408,1096, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.56  BV=2.35
    {16659,1536,1080, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.66  BV=2.25
    {24996,1152,1024, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.17  BV=2.15
    {24996,1152,1096, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.27  BV=2.05
    {24996,1280,1056, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.37  BV=1.96
    {24996,1408,1032, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.47  BV=1.85
    {33332,1136,1024, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.15  BV=1.76
    {33332,1152,1088, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.26  BV=1.65
    {33332,1280,1048, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.36  BV=1.55
    {33332,1408,1024, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.46  BV=1.45
    {41669,1152,1072, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=5.24  BV=1.35
    {41669,1280,1032, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=5.33  BV=1.25
    {49991,1152,1024, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.17  BV=1.15
    {49991,1152,1096, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.27  BV=1.05
    {49991,1280,1056, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.37  BV=0.96
    {49991,1408,1032, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.47  BV=0.85
    {49991,1408,1104, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.57  BV=0.75
    {49991,1536,1088, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.67  BV=0.65
    {49991,1664,1072, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.77  BV=0.56
    {49991,1792,1072, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.87  BV=0.45
    {49991,1920,1072, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.97  BV=0.35
    {49991,2048,1080, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=6.08  BV=0.25
    {49991,2304,1024, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=6.17  BV=0.15
    {49991,2432,1040, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=6.27  BV=0.05
    {49991,2560,1056, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=6.37  BV=-0.04
    {49991,2816,1032, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=6.47  BV=-0.15
    {49991,2944,1056, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=6.57  BV=-0.25
    {49991,3200,1040, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=6.67  BV=-0.34
    {49991,3456,1032, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=6.77  BV=-0.44
    {49991,3712,1032, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=6.87  BV=-0.55
    {49991,3968,1040, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=6.98  BV=-0.65
    {50006,4224,1048, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.08  BV=-0.76
    {58328,3968,1024, 0, 0, 0},  //TV = 4.10(3932 lines)  AV=2.00  SV=6.95  BV=-0.85
    {58328,4224,1032, 0, 0, 0},  //TV = 4.10(3932 lines)  AV=2.00  SV=7.06  BV=-0.96
    {66664,3968,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=6.97  BV=-1.06
    {66664,4224,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.06  BV=-1.15
    {75001,3968,1048, 0, 0, 0},  //TV = 3.74(5056 lines)  AV=2.00  SV=6.99  BV=-1.25
    {83338,3840,1048, 0, 0, 0},  //TV = 3.58(5618 lines)  AV=2.00  SV=6.94  BV=-1.36
    {83338,4096,1048, 0, 0, 0},  //TV = 3.58(5618 lines)  AV=2.00  SV=7.03  BV=-1.45
    {91660,4096,1024, 0, 0, 0},  //TV = 3.45(6179 lines)  AV=2.00  SV=7.00  BV=-1.55
    {99996,3968,1040, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=6.98  BV=-1.65
    {108333,3968,1024, 0, 0, 0},  //TV = 3.21(7303 lines)  AV=2.00  SV=6.95  BV=-1.75
    {116670,3968,1024, 0, 0, 0},  //TV = 3.10(7865 lines)  AV=2.00  SV=6.95  BV=-1.85
    {124992,3968,1024, 0, 0, 0},  //TV = 3.00(8426 lines)  AV=2.00  SV=6.95  BV=-1.95
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
},
};

static strEvPline sCaptureISO400PLineTable_50Hz =
{
{
    {30,1152,1104, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.28  BV=11.75
    {30,1280,1056, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.37  BV=11.66
    {30,1408,1040, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.48  BV=11.54
    {30,1536,1032, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.60  BV=11.43
    {30,1664,1024, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.70  BV=11.32
    {45,1152,1048, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.20  BV=11.24
    {45,1152,1120, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.30  BV=11.14
    {45,1280,1088, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.41  BV=11.03
    {45,1408,1040, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.48  BV=10.96
    {60,1152,1056, 0, 0, 0},  /* TV = 14.02(4 lines)  AV=2.00  SV=5.21  BV=10.81 */
    {60,1152,1128, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.31  BV=10.72
    {60,1280,1072, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.39  BV=10.64
    {75,1152,1032, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.18  BV=10.52
    {75,1152,1088, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.26  BV=10.45
    {75,1280,1056, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.37  BV=10.34
    {90,1152,1040, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.19  BV=10.25
    {90,1280,1024, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.32  BV=10.12
    {104,1152,1040, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.19  BV=10.04
    {104,1152,1104, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.28  BV=9.95
    {119,1152,1032, 0, 0, 0},  //TV = 13.04(8 lines)  AV=2.00  SV=5.18  BV=9.86
    {134,1136,1024, 0, 0, 0},  /* TV = 12.87(9 lines)  AV=2.00  SV=5.15  BV=9.72 */
    {134,1152,1064, 0, 0, 0},  //TV = 12.87(9 lines)  AV=2.00  SV=5.23  BV=9.64
    {149,1152,1032, 0, 0, 0},  //TV = 12.71(10 lines)  AV=2.00  SV=5.18  BV=9.53
    {164,1136,1024, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.15  BV=9.42
    {164,1152,1088, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.26  BV=9.32
    {179,1152,1040, 0, 0, 0},  /* TV = 12.45(12 lines)  AV=2.00  SV=5.19  BV=9.26 */
    {193,1152,1032, 0, 0, 0},  //TV = 12.34(13 lines)  AV=2.00  SV=5.18  BV=9.16
    {208,1152,1032, 0, 0, 0},  //TV = 12.23(14 lines)  AV=2.00  SV=5.18  BV=9.05
    {223,1152,1032, 0, 0, 0},  //TV = 12.13(15 lines)  AV=2.00  SV=5.18  BV=8.95
    {238,1152,1032, 0, 0, 0},  //TV = 12.04(16 lines)  AV=2.00  SV=5.18  BV=8.86
    {253,1152,1040, 0, 0, 0},  //TV = 11.95(17 lines)  AV=2.00  SV=5.19  BV=8.76
    {268,1152,1056, 0, 0, 0},  //TV = 11.87(18 lines)  AV=2.00  SV=5.21  BV=8.65
    {297,1152,1024, 0, 0, 0},  //TV = 11.72(20 lines)  AV=2.00  SV=5.17  BV=8.55
    {312,1152,1040, 0, 0, 0},  //TV = 11.65(21 lines)  AV=2.00  SV=5.19  BV=8.45
    {342,1152,1024, 0, 0, 0},  //TV = 11.51(23 lines)  AV=2.00  SV=5.17  BV=8.34
    {371,1136,1024, 0, 0, 0},  //TV = 11.40(25 lines)  AV=2.00  SV=5.15  BV=8.25
    {386,1152,1032, 0, 0, 0},  //TV = 11.34(26 lines)  AV=2.00  SV=5.18  BV=8.16
    {416,1152,1032, 0, 0, 0},  //TV = 11.23(28 lines)  AV=2.00  SV=5.18  BV=8.05
    {446,1152,1032, 0, 0, 0},  //TV = 11.13(30 lines)  AV=2.00  SV=5.18  BV=7.95
    {475,1152,1032, 0, 0, 0},  //TV = 11.04(32 lines)  AV=2.00  SV=5.18  BV=7.86
    {520,1136,1024, 0, 0, 0},  //TV = 10.91(35 lines)  AV=2.00  SV=5.15  BV=7.76
    {564,1136,1024, 0, 0, 0},  //TV = 10.79(38 lines)  AV=2.00  SV=5.15  BV=7.64
    {594,1152,1024, 0, 0, 0},  //TV = 10.72(40 lines)  AV=2.00  SV=5.17  BV=7.55
    {638,1152,1024, 0, 0, 0},  //TV = 10.61(43 lines)  AV=2.00  SV=5.17  BV=7.44
    {683,1152,1024, 0, 0, 0},  //TV = 10.52(46 lines)  AV=2.00  SV=5.17  BV=7.35
    {727,1152,1024, 0, 0, 0},  //TV = 10.43(49 lines)  AV=2.00  SV=5.17  BV=7.26
    {787,1152,1024, 0, 0, 0},  //TV = 10.31(53 lines)  AV=2.00  SV=5.17  BV=7.14
    {846,1136,1024, 0, 0, 0},  //TV = 10.21(57 lines)  AV=2.00  SV=5.15  BV=7.06
    {905,1136,1024, 0, 0, 0},  //TV = 10.11(61 lines)  AV=2.00  SV=5.15  BV=6.96
    {965,1152,1024, 0, 0, 0},  //TV = 10.02(65 lines)  AV=2.00  SV=5.17  BV=6.85
    {1039,1152,1024, 0, 0, 0},  //TV = 9.91(70 lines)  AV=2.00  SV=5.17  BV=6.74
    {1128,1136,1024, 0, 0, 0},  //TV = 9.79(76 lines)  AV=2.00  SV=5.15  BV=6.64
    {1202,1136,1024, 0, 0, 0},  //TV = 9.70(81 lines)  AV=2.00  SV=5.15  BV=6.55
    {1291,1136,1024, 0, 0, 0},  //TV = 9.60(87 lines)  AV=2.00  SV=5.15  BV=6.45
    {1380,1136,1024, 0, 0, 0},  //TV = 9.50(93 lines)  AV=2.00  SV=5.15  BV=6.35
    {1484,1136,1024, 0, 0, 0},  //TV = 9.40(100 lines)  AV=2.00  SV=5.15  BV=6.25
    {1588,1136,1024, 0, 0, 0},  //TV = 9.30(107 lines)  AV=2.00  SV=5.15  BV=6.15
    {1692,1136,1024, 0, 0, 0},  //TV = 9.21(114 lines)  AV=2.00  SV=5.15  BV=6.06
    {1810,1136,1024, 0, 0, 0},  //TV = 9.11(122 lines)  AV=2.00  SV=5.15  BV=5.96
    {1959,1136,1024, 0, 0, 0},  //TV = 9.00(132 lines)  AV=2.00  SV=5.15  BV=5.85
    {2107,1136,1024, 0, 0, 0},  //TV = 8.89(142 lines)  AV=2.00  SV=5.15  BV=5.74
    {2255,1136,1024, 0, 0, 0},  //TV = 8.79(152 lines)  AV=2.00  SV=5.15  BV=5.64
    {2389,1136,1024, 0, 0, 0},  //TV = 8.71(161 lines)  AV=2.00  SV=5.15  BV=5.56
    {2582,1136,1024, 0, 0, 0},  //TV = 8.60(174 lines)  AV=2.00  SV=5.15  BV=5.45
    {2774,1136,1024, 0, 0, 0},  //TV = 8.49(187 lines)  AV=2.00  SV=5.15  BV=5.34
    {2967,1136,1024, 0, 0, 0},  //TV = 8.40(200 lines)  AV=2.00  SV=5.15  BV=5.25
    {3175,1136,1024, 0, 0, 0},  //TV = 8.30(214 lines)  AV=2.00  SV=5.15  BV=5.15
    {3397,1136,1024, 0, 0, 0},  //TV = 8.20(229 lines)  AV=2.00  SV=5.15  BV=5.05
    {3635,1136,1024, 0, 0, 0},  //TV = 8.10(245 lines)  AV=2.00  SV=5.15  BV=4.95
    {3932,1136,1024, 0, 0, 0},  //TV = 7.99(265 lines)  AV=2.00  SV=5.15  BV=4.84
    {4199,1136,1024, 0, 0, 0},  //TV = 7.90(283 lines)  AV=2.00  SV=5.15  BV=4.75
    {4495,1136,1024, 0, 0, 0},  //TV = 7.80(303 lines)  AV=2.00  SV=5.15  BV=4.65
    {4822,1136,1024, 0, 0, 0},  //TV = 7.70(325 lines)  AV=2.00  SV=5.15  BV=4.55
    {5163,1136,1024, 0, 0, 0},  //TV = 7.60(348 lines)  AV=2.00  SV=5.15  BV=4.45
    {5563,1136,1024, 0, 0, 0},  //TV = 7.49(375 lines)  AV=2.00  SV=5.15  BV=4.34
    {5949,1136,1024, 0, 0, 0},  //TV = 7.39(401 lines)  AV=2.00  SV=5.15  BV=4.24
    {6379,1136,1024, 0, 0, 0},  //TV = 7.29(430 lines)  AV=2.00  SV=5.15  BV=4.14
    {6824,1136,1024, 0, 0, 0},  //TV = 7.20(460 lines)  AV=2.00  SV=5.15  BV=4.05
    {7314,1136,1024, 0, 0, 0},  //TV = 7.10(493 lines)  AV=2.00  SV=5.15  BV=3.95
    {7833,1136,1024, 0, 0, 0},  //TV = 7.00(528 lines)  AV=2.00  SV=5.15  BV=3.85
    {8397,1136,1024, 0, 0, 0},  //TV = 6.90(566 lines)  AV=2.00  SV=5.15  BV=3.75
    {8990,1136,1024, 0, 0, 0},  //TV = 6.80(606 lines)  AV=2.00  SV=5.15  BV=3.65
    {9628,1136,1024, 0, 0, 0},  //TV = 6.70(649 lines)  AV=2.00  SV=5.15  BV=3.55
    {9999,1152,1040, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.19  BV=3.45
    {9999,1152,1112, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.29  BV=3.36
    {9999,1280,1080, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.40  BV=3.25
    {9999,1408,1056, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.50  BV=3.14
    {9999,1536,1032, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.60  BV=3.05
    {9999,1664,1024, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.70  BV=2.94
    {9999,1664,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.79  BV=2.86
    {9999,1792,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.89  BV=2.75
    {9999,1920,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.99  BV=2.65
    {9999,2176,1032, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=6.10  BV=2.55
    {19997,1152,1040, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.19  BV=2.45
    {19997,1152,1112, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.29  BV=2.36
    {19997,1280,1080, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.40  BV=2.25
    {19997,1408,1048, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.49  BV=2.15
    {19997,1536,1032, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.60  BV=2.05
    {19997,1664,1024, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.70  BV=1.94
    {29995,1152,1048, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.20  BV=1.86
    {29995,1152,1128, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.31  BV=1.75
    {29995,1280,1088, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.41  BV=1.65
    {29995,1408,1064, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.51  BV=1.54
    {39993,1152,1040, 0, 0, 0},  //TV = 4.64(2696 lines)  AV=2.00  SV=5.19  BV=1.45
    {39993,1152,1112, 0, 0, 0},  //TV = 4.64(2696 lines)  AV=2.00  SV=5.29  BV=1.36
    {39993,1280,1080, 0, 0, 0},  //TV = 4.64(2696 lines)  AV=2.00  SV=5.40  BV=1.25
    {50006,1152,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.17  BV=1.15
    {50006,1152,1096, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.27  BV=1.05
    {50006,1280,1056, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.37  BV=0.96
    {50006,1408,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.47  BV=0.85
    {50006,1408,1104, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.57  BV=0.75
    {50006,1536,1088, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.67  BV=0.65
    {50006,1664,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.77  BV=0.56
    {50006,1792,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.87  BV=0.45
    {50006,1920,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.97  BV=0.35
    {50006,2048,1080, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.08  BV=0.24
    {50006,2304,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.17  BV=0.15
    {50006,2432,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.27  BV=0.05
    {50006,2560,1056, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.37  BV=-0.04
    {50006,2816,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.47  BV=-0.15
    {50006,2944,1056, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.57  BV=-0.25
    {50006,3200,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.67  BV=-0.34
    {50006,3456,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.77  BV=-0.44
    {50006,3712,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.87  BV=-0.55
    {50006,3968,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.98  BV=-0.65
    {50006,4224,1048, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.08  BV=-0.76
    {60004,3840,1024, 0, 0, 0},  //TV = 4.06(4045 lines)  AV=2.00  SV=6.91  BV=-0.85
    {60004,4096,1032, 0, 0, 0},  //TV = 4.06(4045 lines)  AV=2.00  SV=7.01  BV=-0.95
    {70002,3712,1048, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=6.89  BV=-1.05
    {70002,3968,1048, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=6.99  BV=-1.15
    {70002,4352,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.09  BV=-1.25
    {80000,4096,1024, 0, 0, 0},  //TV = 3.64(5393 lines)  AV=2.00  SV=7.00  BV=-1.36
    {89998,3840,1040, 0, 0, 0},  //TV = 3.47(6067 lines)  AV=2.00  SV=6.93  BV=-1.46
    {89998,4096,1040, 0, 0, 0},  //TV = 3.47(6067 lines)  AV=2.00  SV=7.02  BV=-1.55
    {99996,3968,1040, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=6.98  BV=-1.65
    {109995,3840,1048, 0, 0, 0},  //TV = 3.18(7415 lines)  AV=2.00  SV=6.94  BV=-1.76
    {109995,4096,1048, 0, 0, 0},  //TV = 3.18(7415 lines)  AV=2.00  SV=7.03  BV=-1.85
    {120008,4096,1032, 0, 0, 0},  //TV = 3.06(8090 lines)  AV=2.00  SV=7.01  BV=-1.95
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
},
};

static strAETable g_AE_CaptureISO400Table =
{
    AETABLE_CAPTURE_ISO400, //eAETableID
    138, //u4TotalIndex
    -20, //i4StrobeTrigerBV
    118, //i4MaxBV
    -20, //i4MinBV
    90, //i4EffectiveMaxBV
    0, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_400, //ISO
    sCaptureISO400PLineTable_60Hz,
    sCaptureISO400PLineTable_50Hz,
    NULL,
};
static strEvPline sCaptureISO800PLineTable_60Hz =
{
{
    {30,1152,1104, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.28  BV=11.75
    {30,1280,1056, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.37  BV=11.66
    {30,1408,1040, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.48  BV=11.54
    {30,1536,1032, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.60  BV=11.43
    {30,1664,1024, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.70  BV=11.32
    {45,1152,1048, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.20  BV=11.24
    {45,1152,1120, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.30  BV=11.14
    {45,1280,1088, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.41  BV=11.03
    {45,1408,1040, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.48  BV=10.96
    {60,1152,1056, 0, 0, 0},  /* TV = 14.02(4 lines)  AV=2.00  SV=5.21  BV=10.81 */
    {60,1152,1128, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.31  BV=10.72
    {60,1280,1072, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.39  BV=10.64
    {75,1152,1032, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.18  BV=10.52
    {75,1152,1088, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.26  BV=10.45
    {75,1280,1056, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.37  BV=10.34
    {90,1152,1040, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.19  BV=10.25
    {90,1280,1024, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.32  BV=10.12
    {104,1152,1040, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.19  BV=10.04
    {104,1152,1104, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.28  BV=9.95
    {119,1152,1032, 0, 0, 0},  //TV = 13.04(8 lines)  AV=2.00  SV=5.18  BV=9.86
    {134,1136,1024, 0, 0, 0},  /* TV = 12.87(9 lines)  AV=2.00  SV=5.15  BV=9.72 */
    {134,1152,1064, 0, 0, 0},  //TV = 12.87(9 lines)  AV=2.00  SV=5.23  BV=9.64
    {149,1152,1032, 0, 0, 0},  //TV = 12.71(10 lines)  AV=2.00  SV=5.18  BV=9.53
    {164,1136,1024, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.15  BV=9.42
    {164,1152,1088, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.26  BV=9.32
    {179,1152,1040, 0, 0, 0},  /* TV = 12.45(12 lines)  AV=2.00  SV=5.19  BV=9.26 */
    {193,1152,1032, 0, 0, 0},  //TV = 12.34(13 lines)  AV=2.00  SV=5.18  BV=9.16
    {208,1152,1032, 0, 0, 0},  //TV = 12.23(14 lines)  AV=2.00  SV=5.18  BV=9.05
    {223,1152,1032, 0, 0, 0},  //TV = 12.13(15 lines)  AV=2.00  SV=5.18  BV=8.95
    {238,1152,1032, 0, 0, 0},  //TV = 12.04(16 lines)  AV=2.00  SV=5.18  BV=8.86
    {253,1152,1040, 0, 0, 0},  //TV = 11.95(17 lines)  AV=2.00  SV=5.19  BV=8.76
    {268,1152,1056, 0, 0, 0},  //TV = 11.87(18 lines)  AV=2.00  SV=5.21  BV=8.65
    {297,1152,1024, 0, 0, 0},  //TV = 11.72(20 lines)  AV=2.00  SV=5.17  BV=8.55
    {312,1152,1040, 0, 0, 0},  //TV = 11.65(21 lines)  AV=2.00  SV=5.19  BV=8.45
    {342,1152,1024, 0, 0, 0},  //TV = 11.51(23 lines)  AV=2.00  SV=5.17  BV=8.34
    {371,1136,1024, 0, 0, 0},  //TV = 11.40(25 lines)  AV=2.00  SV=5.15  BV=8.25
    {386,1152,1032, 0, 0, 0},  //TV = 11.34(26 lines)  AV=2.00  SV=5.18  BV=8.16
    {416,1152,1032, 0, 0, 0},  //TV = 11.23(28 lines)  AV=2.00  SV=5.18  BV=8.05
    {446,1152,1032, 0, 0, 0},  //TV = 11.13(30 lines)  AV=2.00  SV=5.18  BV=7.95
    {475,1152,1032, 0, 0, 0},  //TV = 11.04(32 lines)  AV=2.00  SV=5.18  BV=7.86
    {520,1136,1024, 0, 0, 0},  //TV = 10.91(35 lines)  AV=2.00  SV=5.15  BV=7.76
    {564,1136,1024, 0, 0, 0},  //TV = 10.79(38 lines)  AV=2.00  SV=5.15  BV=7.64
    {594,1152,1024, 0, 0, 0},  //TV = 10.72(40 lines)  AV=2.00  SV=5.17  BV=7.55
    {638,1152,1024, 0, 0, 0},  //TV = 10.61(43 lines)  AV=2.00  SV=5.17  BV=7.44
    {683,1152,1024, 0, 0, 0},  //TV = 10.52(46 lines)  AV=2.00  SV=5.17  BV=7.35
    {727,1152,1024, 0, 0, 0},  //TV = 10.43(49 lines)  AV=2.00  SV=5.17  BV=7.26
    {787,1152,1024, 0, 0, 0},  //TV = 10.31(53 lines)  AV=2.00  SV=5.17  BV=7.14
    {846,1136,1024, 0, 0, 0},  //TV = 10.21(57 lines)  AV=2.00  SV=5.15  BV=7.06
    {905,1136,1024, 0, 0, 0},  //TV = 10.11(61 lines)  AV=2.00  SV=5.15  BV=6.96
    {965,1152,1024, 0, 0, 0},  //TV = 10.02(65 lines)  AV=2.00  SV=5.17  BV=6.85
    {1039,1152,1024, 0, 0, 0},  //TV = 9.91(70 lines)  AV=2.00  SV=5.17  BV=6.74
    {1128,1136,1024, 0, 0, 0},  //TV = 9.79(76 lines)  AV=2.00  SV=5.15  BV=6.64
    {1202,1136,1024, 0, 0, 0},  //TV = 9.70(81 lines)  AV=2.00  SV=5.15  BV=6.55
    {1291,1136,1024, 0, 0, 0},  //TV = 9.60(87 lines)  AV=2.00  SV=5.15  BV=6.45
    {1380,1136,1024, 0, 0, 0},  //TV = 9.50(93 lines)  AV=2.00  SV=5.15  BV=6.35
    {1484,1136,1024, 0, 0, 0},  //TV = 9.40(100 lines)  AV=2.00  SV=5.15  BV=6.25
    {1588,1136,1024, 0, 0, 0},  //TV = 9.30(107 lines)  AV=2.00  SV=5.15  BV=6.15
    {1692,1136,1024, 0, 0, 0},  //TV = 9.21(114 lines)  AV=2.00  SV=5.15  BV=6.06
    {1810,1136,1024, 0, 0, 0},  //TV = 9.11(122 lines)  AV=2.00  SV=5.15  BV=5.96
    {1959,1136,1024, 0, 0, 0},  //TV = 9.00(132 lines)  AV=2.00  SV=5.15  BV=5.85
    {2107,1136,1024, 0, 0, 0},  //TV = 8.89(142 lines)  AV=2.00  SV=5.15  BV=5.74
    {2255,1136,1024, 0, 0, 0},  //TV = 8.79(152 lines)  AV=2.00  SV=5.15  BV=5.64
    {2389,1136,1024, 0, 0, 0},  //TV = 8.71(161 lines)  AV=2.00  SV=5.15  BV=5.56
    {2582,1136,1024, 0, 0, 0},  //TV = 8.60(174 lines)  AV=2.00  SV=5.15  BV=5.45
    {2774,1136,1024, 0, 0, 0},  //TV = 8.49(187 lines)  AV=2.00  SV=5.15  BV=5.34
    {2967,1136,1024, 0, 0, 0},  //TV = 8.40(200 lines)  AV=2.00  SV=5.15  BV=5.25
    {3175,1136,1024, 0, 0, 0},  //TV = 8.30(214 lines)  AV=2.00  SV=5.15  BV=5.15
    {3397,1136,1024, 0, 0, 0},  //TV = 8.20(229 lines)  AV=2.00  SV=5.15  BV=5.05
    {3635,1136,1024, 0, 0, 0},  //TV = 8.10(245 lines)  AV=2.00  SV=5.15  BV=4.95
    {3932,1136,1024, 0, 0, 0},  //TV = 7.99(265 lines)  AV=2.00  SV=5.15  BV=4.84
    {4199,1136,1024, 0, 0, 0},  //TV = 7.90(283 lines)  AV=2.00  SV=5.15  BV=4.75
    {4495,1136,1024, 0, 0, 0},  //TV = 7.80(303 lines)  AV=2.00  SV=5.15  BV=4.65
    {4822,1136,1024, 0, 0, 0},  //TV = 7.70(325 lines)  AV=2.00  SV=5.15  BV=4.55
    {5163,1136,1024, 0, 0, 0},  //TV = 7.60(348 lines)  AV=2.00  SV=5.15  BV=4.45
    {5563,1136,1024, 0, 0, 0},  //TV = 7.49(375 lines)  AV=2.00  SV=5.15  BV=4.34
    {5949,1136,1024, 0, 0, 0},  //TV = 7.39(401 lines)  AV=2.00  SV=5.15  BV=4.24
    {6379,1136,1024, 0, 0, 0},  //TV = 7.29(430 lines)  AV=2.00  SV=5.15  BV=4.14
    {6824,1136,1024, 0, 0, 0},  //TV = 7.20(460 lines)  AV=2.00  SV=5.15  BV=4.05
    {7314,1136,1024, 0, 0, 0},  //TV = 7.10(493 lines)  AV=2.00  SV=5.15  BV=3.95
    {7833,1136,1024, 0, 0, 0},  //TV = 7.00(528 lines)  AV=2.00  SV=5.15  BV=3.85
    {8337,1136,1024, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.15  BV=3.76
    {8337,1152,1088, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.26  BV=3.65
    {8337,1280,1048, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.36  BV=3.55
    {8337,1408,1024, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.46  BV=3.45
    {8337,1408,1104, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.57  BV=3.34
    {8337,1536,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.65  BV=3.26
    {8337,1664,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.77  BV=3.14
    {8337,1792,1056, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.85  BV=3.05
    {8337,1920,1064, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.96  BV=2.94
    {8337,2048,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=6.07  BV=2.84
    {16659,1136,1024, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.15  BV=2.76
    {16659,1152,1088, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.26  BV=2.65
    {16659,1280,1056, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.37  BV=2.54
    {16659,1408,1024, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.46  BV=2.45
    {16659,1408,1096, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.56  BV=2.35
    {16659,1536,1080, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.66  BV=2.25
    {24996,1152,1024, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.17  BV=2.15
    {24996,1152,1096, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.27  BV=2.05
    {24996,1280,1056, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.37  BV=1.96
    {24996,1408,1032, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.47  BV=1.85
    {33332,1136,1024, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.15  BV=1.76
    {33332,1152,1088, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.26  BV=1.65
    {33332,1280,1048, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.36  BV=1.55
    {33332,1408,1024, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.46  BV=1.45
    {41669,1152,1072, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=5.24  BV=1.35
    {41669,1280,1032, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=5.33  BV=1.25
    {49991,1152,1024, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.17  BV=1.15
    {49991,1152,1096, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.27  BV=1.05
    {49991,1280,1056, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.37  BV=0.96
    {49991,1408,1032, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.47  BV=0.85
    {49991,1408,1104, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.57  BV=0.75
    {49991,1536,1088, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.67  BV=0.65
    {49991,1664,1072, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.77  BV=0.56
    {49991,1792,1072, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.87  BV=0.45
    {49991,1920,1072, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.97  BV=0.35
    {49991,2048,1080, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=6.08  BV=0.25
    {49991,2304,1024, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=6.17  BV=0.15
    {49991,2432,1040, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=6.27  BV=0.05
    {49991,2560,1056, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=6.37  BV=-0.04
    {49991,2816,1032, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=6.47  BV=-0.15
    {49991,2944,1056, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=6.57  BV=-0.25
    {49991,3200,1040, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=6.67  BV=-0.34
    {49991,3456,1032, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=6.77  BV=-0.44
    {49991,3712,1032, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=6.87  BV=-0.55
    {49991,3968,1040, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=6.98  BV=-0.65
    {49991,4224,1048, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=7.08  BV=-0.76
    {49991,4608,1024, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=7.17  BV=-0.85
    {49991,4864,1040, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=7.27  BV=-0.95
    {49991,5248,1032, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=7.37  BV=-1.05
    {49991,5632,1032, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=7.47  BV=-1.15
    {49991,6016,1040, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=7.58  BV=-1.25
    {49991,6528,1024, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=7.67  BV=-1.35
    {49991,6912,1040, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=7.78  BV=-1.46
    {49991,7424,1032, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=7.87  BV=-1.55
    {49991,8064,1024, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=7.98  BV=-1.66
    {50006,8576,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=8.08  BV=-1.76
    {58328,7936,1024, 0, 0, 0},  //TV = 4.10(3932 lines)  AV=2.00  SV=7.95  BV=-1.85
    {58328,8448,1032, 0, 0, 0},  //TV = 4.10(3932 lines)  AV=2.00  SV=8.06  BV=-1.96
    {66664,7936,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.95  BV=-2.05
    {66664,8448,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=8.06  BV=-2.15
    {75001,8064,1032, 0, 0, 0},  //TV = 3.74(5056 lines)  AV=2.00  SV=7.99  BV=-2.25
    {83338,7808,1032, 0, 0, 0},  //TV = 3.58(5618 lines)  AV=2.00  SV=7.94  BV=-2.36
    {83338,8320,1032, 0, 0, 0},  //TV = 3.58(5618 lines)  AV=2.00  SV=8.03  BV=-2.45
    {91660,8192,1024, 0, 0, 0},  //TV = 3.45(6179 lines)  AV=2.00  SV=8.00  BV=-2.55
    {99996,8064,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=7.98  BV=-2.66
    {108333,7936,1024, 0, 0, 0},  //TV = 3.21(7303 lines)  AV=2.00  SV=7.95  BV=-2.75
    {116670,7936,1024, 0, 0, 0},  //TV = 3.10(7865 lines)  AV=2.00  SV=7.95  BV=-2.85
    {124992,7936,1024, 0, 0, 0},  //TV = 3.00(8426 lines)  AV=2.00  SV=7.95  BV=-2.95
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
},
};

static strEvPline sCaptureISO800PLineTable_50Hz =
{
{
    {30,1152,1104, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.28  BV=11.75
    {30,1280,1056, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.37  BV=11.66
    {30,1408,1040, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.48  BV=11.54
    {30,1536,1032, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.60  BV=11.43
    {30,1664,1024, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.70  BV=11.32
    {45,1152,1048, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.20  BV=11.24
    {45,1152,1120, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.30  BV=11.14
    {45,1280,1088, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.41  BV=11.03
    {45,1408,1040, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.48  BV=10.96
    {60,1152,1056, 0, 0, 0},  /* TV = 14.02(4 lines)  AV=2.00  SV=5.21  BV=10.81 */
    {60,1152,1128, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.31  BV=10.72
    {60,1280,1072, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.39  BV=10.64
    {75,1152,1032, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.18  BV=10.52
    {75,1152,1088, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.26  BV=10.45
    {75,1280,1056, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.37  BV=10.34
    {90,1152,1040, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.19  BV=10.25
    {90,1280,1024, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.32  BV=10.12
    {104,1152,1040, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.19  BV=10.04
    {104,1152,1104, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.28  BV=9.95
    {119,1152,1032, 0, 0, 0},  //TV = 13.04(8 lines)  AV=2.00  SV=5.18  BV=9.86
    {134,1136,1024, 0, 0, 0},  /* TV = 12.87(9 lines)  AV=2.00  SV=5.15  BV=9.72 */
    {134,1152,1064, 0, 0, 0},  //TV = 12.87(9 lines)  AV=2.00  SV=5.23  BV=9.64
    {149,1152,1032, 0, 0, 0},  //TV = 12.71(10 lines)  AV=2.00  SV=5.18  BV=9.53
    {164,1136,1024, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.15  BV=9.42
    {164,1152,1088, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.26  BV=9.32
    {179,1152,1040, 0, 0, 0},  /* TV = 12.45(12 lines)  AV=2.00  SV=5.19  BV=9.26 */
    {193,1152,1032, 0, 0, 0},  //TV = 12.34(13 lines)  AV=2.00  SV=5.18  BV=9.16
    {208,1152,1032, 0, 0, 0},  //TV = 12.23(14 lines)  AV=2.00  SV=5.18  BV=9.05
    {223,1152,1032, 0, 0, 0},  //TV = 12.13(15 lines)  AV=2.00  SV=5.18  BV=8.95
    {238,1152,1032, 0, 0, 0},  //TV = 12.04(16 lines)  AV=2.00  SV=5.18  BV=8.86
    {253,1152,1040, 0, 0, 0},  //TV = 11.95(17 lines)  AV=2.00  SV=5.19  BV=8.76
    {268,1152,1056, 0, 0, 0},  //TV = 11.87(18 lines)  AV=2.00  SV=5.21  BV=8.65
    {297,1152,1024, 0, 0, 0},  //TV = 11.72(20 lines)  AV=2.00  SV=5.17  BV=8.55
    {312,1152,1040, 0, 0, 0},  //TV = 11.65(21 lines)  AV=2.00  SV=5.19  BV=8.45
    {342,1152,1024, 0, 0, 0},  //TV = 11.51(23 lines)  AV=2.00  SV=5.17  BV=8.34
    {371,1136,1024, 0, 0, 0},  //TV = 11.40(25 lines)  AV=2.00  SV=5.15  BV=8.25
    {386,1152,1032, 0, 0, 0},  //TV = 11.34(26 lines)  AV=2.00  SV=5.18  BV=8.16
    {416,1152,1032, 0, 0, 0},  //TV = 11.23(28 lines)  AV=2.00  SV=5.18  BV=8.05
    {446,1152,1032, 0, 0, 0},  //TV = 11.13(30 lines)  AV=2.00  SV=5.18  BV=7.95
    {475,1152,1032, 0, 0, 0},  //TV = 11.04(32 lines)  AV=2.00  SV=5.18  BV=7.86
    {520,1136,1024, 0, 0, 0},  //TV = 10.91(35 lines)  AV=2.00  SV=5.15  BV=7.76
    {564,1136,1024, 0, 0, 0},  //TV = 10.79(38 lines)  AV=2.00  SV=5.15  BV=7.64
    {594,1152,1024, 0, 0, 0},  //TV = 10.72(40 lines)  AV=2.00  SV=5.17  BV=7.55
    {638,1152,1024, 0, 0, 0},  //TV = 10.61(43 lines)  AV=2.00  SV=5.17  BV=7.44
    {683,1152,1024, 0, 0, 0},  //TV = 10.52(46 lines)  AV=2.00  SV=5.17  BV=7.35
    {727,1152,1024, 0, 0, 0},  //TV = 10.43(49 lines)  AV=2.00  SV=5.17  BV=7.26
    {787,1152,1024, 0, 0, 0},  //TV = 10.31(53 lines)  AV=2.00  SV=5.17  BV=7.14
    {846,1136,1024, 0, 0, 0},  //TV = 10.21(57 lines)  AV=2.00  SV=5.15  BV=7.06
    {905,1136,1024, 0, 0, 0},  //TV = 10.11(61 lines)  AV=2.00  SV=5.15  BV=6.96
    {965,1152,1024, 0, 0, 0},  //TV = 10.02(65 lines)  AV=2.00  SV=5.17  BV=6.85
    {1039,1152,1024, 0, 0, 0},  //TV = 9.91(70 lines)  AV=2.00  SV=5.17  BV=6.74
    {1128,1136,1024, 0, 0, 0},  //TV = 9.79(76 lines)  AV=2.00  SV=5.15  BV=6.64
    {1202,1136,1024, 0, 0, 0},  //TV = 9.70(81 lines)  AV=2.00  SV=5.15  BV=6.55
    {1291,1136,1024, 0, 0, 0},  //TV = 9.60(87 lines)  AV=2.00  SV=5.15  BV=6.45
    {1380,1136,1024, 0, 0, 0},  //TV = 9.50(93 lines)  AV=2.00  SV=5.15  BV=6.35
    {1484,1136,1024, 0, 0, 0},  //TV = 9.40(100 lines)  AV=2.00  SV=5.15  BV=6.25
    {1588,1136,1024, 0, 0, 0},  //TV = 9.30(107 lines)  AV=2.00  SV=5.15  BV=6.15
    {1692,1136,1024, 0, 0, 0},  //TV = 9.21(114 lines)  AV=2.00  SV=5.15  BV=6.06
    {1810,1136,1024, 0, 0, 0},  //TV = 9.11(122 lines)  AV=2.00  SV=5.15  BV=5.96
    {1959,1136,1024, 0, 0, 0},  //TV = 9.00(132 lines)  AV=2.00  SV=5.15  BV=5.85
    {2107,1136,1024, 0, 0, 0},  //TV = 8.89(142 lines)  AV=2.00  SV=5.15  BV=5.74
    {2255,1136,1024, 0, 0, 0},  //TV = 8.79(152 lines)  AV=2.00  SV=5.15  BV=5.64
    {2389,1136,1024, 0, 0, 0},  //TV = 8.71(161 lines)  AV=2.00  SV=5.15  BV=5.56
    {2582,1136,1024, 0, 0, 0},  //TV = 8.60(174 lines)  AV=2.00  SV=5.15  BV=5.45
    {2774,1136,1024, 0, 0, 0},  //TV = 8.49(187 lines)  AV=2.00  SV=5.15  BV=5.34
    {2967,1136,1024, 0, 0, 0},  //TV = 8.40(200 lines)  AV=2.00  SV=5.15  BV=5.25
    {3175,1136,1024, 0, 0, 0},  //TV = 8.30(214 lines)  AV=2.00  SV=5.15  BV=5.15
    {3397,1136,1024, 0, 0, 0},  //TV = 8.20(229 lines)  AV=2.00  SV=5.15  BV=5.05
    {3635,1136,1024, 0, 0, 0},  //TV = 8.10(245 lines)  AV=2.00  SV=5.15  BV=4.95
    {3932,1136,1024, 0, 0, 0},  //TV = 7.99(265 lines)  AV=2.00  SV=5.15  BV=4.84
    {4199,1136,1024, 0, 0, 0},  //TV = 7.90(283 lines)  AV=2.00  SV=5.15  BV=4.75
    {4495,1136,1024, 0, 0, 0},  //TV = 7.80(303 lines)  AV=2.00  SV=5.15  BV=4.65
    {4822,1136,1024, 0, 0, 0},  //TV = 7.70(325 lines)  AV=2.00  SV=5.15  BV=4.55
    {5163,1136,1024, 0, 0, 0},  //TV = 7.60(348 lines)  AV=2.00  SV=5.15  BV=4.45
    {5563,1136,1024, 0, 0, 0},  //TV = 7.49(375 lines)  AV=2.00  SV=5.15  BV=4.34
    {5949,1136,1024, 0, 0, 0},  //TV = 7.39(401 lines)  AV=2.00  SV=5.15  BV=4.24
    {6379,1136,1024, 0, 0, 0},  //TV = 7.29(430 lines)  AV=2.00  SV=5.15  BV=4.14
    {6824,1136,1024, 0, 0, 0},  //TV = 7.20(460 lines)  AV=2.00  SV=5.15  BV=4.05
    {7314,1136,1024, 0, 0, 0},  //TV = 7.10(493 lines)  AV=2.00  SV=5.15  BV=3.95
    {7833,1136,1024, 0, 0, 0},  //TV = 7.00(528 lines)  AV=2.00  SV=5.15  BV=3.85
    {8397,1136,1024, 0, 0, 0},  //TV = 6.90(566 lines)  AV=2.00  SV=5.15  BV=3.75
    {8990,1136,1024, 0, 0, 0},  //TV = 6.80(606 lines)  AV=2.00  SV=5.15  BV=3.65
    {9628,1136,1024, 0, 0, 0},  //TV = 6.70(649 lines)  AV=2.00  SV=5.15  BV=3.55
    {9999,1152,1040, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.19  BV=3.45
    {9999,1152,1112, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.29  BV=3.36
    {9999,1280,1080, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.40  BV=3.25
    {9999,1408,1056, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.50  BV=3.14
    {9999,1536,1032, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.60  BV=3.05
    {9999,1664,1024, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.70  BV=2.94
    {9999,1664,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.79  BV=2.86
    {9999,1792,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.89  BV=2.75
    {9999,1920,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.99  BV=2.65
    {9999,2176,1032, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=6.10  BV=2.55
    {19997,1152,1040, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.19  BV=2.45
    {19997,1152,1112, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.29  BV=2.36
    {19997,1280,1080, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.40  BV=2.25
    {19997,1408,1048, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.49  BV=2.15
    {19997,1536,1032, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.60  BV=2.05
    {19997,1664,1024, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.70  BV=1.94
    {29995,1152,1048, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.20  BV=1.86
    {29995,1152,1128, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.31  BV=1.75
    {29995,1280,1088, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.41  BV=1.65
    {29995,1408,1064, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.51  BV=1.54
    {39993,1152,1040, 0, 0, 0},  //TV = 4.64(2696 lines)  AV=2.00  SV=5.19  BV=1.45
    {39993,1152,1112, 0, 0, 0},  //TV = 4.64(2696 lines)  AV=2.00  SV=5.29  BV=1.36
    {39993,1280,1080, 0, 0, 0},  //TV = 4.64(2696 lines)  AV=2.00  SV=5.40  BV=1.25
    {50006,1152,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.17  BV=1.15
    {50006,1152,1096, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.27  BV=1.05
    {50006,1280,1056, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.37  BV=0.96
    {50006,1408,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.47  BV=0.85
    {50006,1408,1104, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.57  BV=0.75
    {50006,1536,1088, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.67  BV=0.65
    {50006,1664,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.77  BV=0.56
    {50006,1792,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.87  BV=0.45
    {50006,1920,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.97  BV=0.35
    {50006,2048,1080, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.08  BV=0.24
    {50006,2304,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.17  BV=0.15
    {50006,2432,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.27  BV=0.05
    {50006,2560,1056, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.37  BV=-0.04
    {50006,2816,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.47  BV=-0.15
    {50006,2944,1056, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.57  BV=-0.25
    {50006,3200,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.67  BV=-0.34
    {50006,3456,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.77  BV=-0.44
    {50006,3712,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.87  BV=-0.55
    {50006,3968,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.98  BV=-0.65
    {50006,4224,1048, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.08  BV=-0.76
    {50006,4608,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.17  BV=-0.85
    {50006,4864,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.27  BV=-0.95
    {50006,5248,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.37  BV=-1.05
    {50006,5632,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.48  BV=-1.16
    {50006,6016,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.58  BV=-1.26
    {50006,6528,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.67  BV=-1.35
    {50006,6912,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.78  BV=-1.46
    {50006,7424,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.87  BV=-1.55
    {50006,8064,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.98  BV=-1.66
    {50006,8576,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=8.08  BV=-1.76
    {60004,7680,1024, 0, 0, 0},  //TV = 4.06(4045 lines)  AV=2.00  SV=7.91  BV=-1.85
    {60004,8192,1032, 0, 0, 0},  //TV = 4.06(4045 lines)  AV=2.00  SV=8.01  BV=-1.95
    {70002,7552,1032, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.89  BV=-2.06
    {70002,8064,1032, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.99  BV=-2.15
    {70002,8704,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=8.09  BV=-2.25
    {80000,8192,1024, 0, 0, 0},  //TV = 3.64(5393 lines)  AV=2.00  SV=8.00  BV=-2.36
    {89998,7808,1024, 0, 0, 0},  //TV = 3.47(6067 lines)  AV=2.00  SV=7.93  BV=-2.46
    {89998,8320,1024, 0, 0, 0},  //TV = 3.47(6067 lines)  AV=2.00  SV=8.02  BV=-2.55
    {99996,8064,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=7.98  BV=-2.66
    {109995,7808,1024, 0, 0, 0},  //TV = 3.18(7415 lines)  AV=2.00  SV=7.93  BV=-2.75
    {109995,8320,1032, 0, 0, 0},  //TV = 3.18(7415 lines)  AV=2.00  SV=8.03  BV=-2.85
    {120008,8192,1032, 0, 0, 0},  //TV = 3.06(8090 lines)  AV=2.00  SV=8.01  BV=-2.95
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
},
};

static strAETable g_AE_CaptureISO800Table =
{
    AETABLE_CAPTURE_ISO800, //eAETableID
    148, //u4TotalIndex
    -20, //i4StrobeTrigerBV
    118, //i4MaxBV
    -30, //i4MinBV
    90, //i4EffectiveMaxBV
    0, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_800, //ISO
    sCaptureISO800PLineTable_60Hz,
    sCaptureISO800PLineTable_50Hz,
    NULL,
};
static strEvPline sCaptureISO1600PLineTable_60Hz =
{
{
    {30,1152,1104, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.28  BV=11.75
    {30,1280,1056, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.37  BV=11.66
    {30,1408,1040, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.48  BV=11.54
    {30,1536,1032, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.60  BV=11.43
    {30,1664,1024, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.70  BV=11.32
    {45,1152,1048, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.20  BV=11.24
    {45,1152,1120, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.30  BV=11.14
    {45,1280,1088, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.41  BV=11.03
    {45,1408,1040, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.48  BV=10.96
    {60,1152,1056, 0, 0, 0},  /* TV = 14.02(4 lines)  AV=2.00  SV=5.21  BV=10.81 */
    {60,1152,1128, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.31  BV=10.72
    {60,1280,1072, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.39  BV=10.64
    {75,1152,1032, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.18  BV=10.52
    {75,1152,1088, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.26  BV=10.45
    {75,1280,1056, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.37  BV=10.34
    {90,1152,1040, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.19  BV=10.25
    {90,1280,1024, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.32  BV=10.12
    {104,1152,1040, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.19  BV=10.04
    {104,1152,1104, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.28  BV=9.95
    {119,1152,1032, 0, 0, 0},  //TV = 13.04(8 lines)  AV=2.00  SV=5.18  BV=9.86
    {134,1136,1024, 0, 0, 0},  /* TV = 12.87(9 lines)  AV=2.00  SV=5.15  BV=9.72 */
    {134,1152,1064, 0, 0, 0},  //TV = 12.87(9 lines)  AV=2.00  SV=5.23  BV=9.64
    {149,1152,1032, 0, 0, 0},  //TV = 12.71(10 lines)  AV=2.00  SV=5.18  BV=9.53
    {164,1136,1024, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.15  BV=9.42
    {164,1152,1088, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.26  BV=9.32
    {179,1152,1040, 0, 0, 0},  /* TV = 12.45(12 lines)  AV=2.00  SV=5.19  BV=9.26 */
    {193,1152,1032, 0, 0, 0},  //TV = 12.34(13 lines)  AV=2.00  SV=5.18  BV=9.16
    {208,1152,1032, 0, 0, 0},  //TV = 12.23(14 lines)  AV=2.00  SV=5.18  BV=9.05
    {223,1152,1032, 0, 0, 0},  //TV = 12.13(15 lines)  AV=2.00  SV=5.18  BV=8.95
    {238,1152,1032, 0, 0, 0},  //TV = 12.04(16 lines)  AV=2.00  SV=5.18  BV=8.86
    {253,1152,1040, 0, 0, 0},  //TV = 11.95(17 lines)  AV=2.00  SV=5.19  BV=8.76
    {268,1152,1056, 0, 0, 0},  //TV = 11.87(18 lines)  AV=2.00  SV=5.21  BV=8.65
    {297,1152,1024, 0, 0, 0},  //TV = 11.72(20 lines)  AV=2.00  SV=5.17  BV=8.55
    {312,1152,1040, 0, 0, 0},  //TV = 11.65(21 lines)  AV=2.00  SV=5.19  BV=8.45
    {342,1152,1024, 0, 0, 0},  //TV = 11.51(23 lines)  AV=2.00  SV=5.17  BV=8.34
    {371,1136,1024, 0, 0, 0},  //TV = 11.40(25 lines)  AV=2.00  SV=5.15  BV=8.25
    {386,1152,1032, 0, 0, 0},  //TV = 11.34(26 lines)  AV=2.00  SV=5.18  BV=8.16
    {416,1152,1032, 0, 0, 0},  //TV = 11.23(28 lines)  AV=2.00  SV=5.18  BV=8.05
    {446,1152,1032, 0, 0, 0},  //TV = 11.13(30 lines)  AV=2.00  SV=5.18  BV=7.95
    {475,1152,1032, 0, 0, 0},  //TV = 11.04(32 lines)  AV=2.00  SV=5.18  BV=7.86
    {520,1136,1024, 0, 0, 0},  //TV = 10.91(35 lines)  AV=2.00  SV=5.15  BV=7.76
    {564,1136,1024, 0, 0, 0},  //TV = 10.79(38 lines)  AV=2.00  SV=5.15  BV=7.64
    {594,1152,1024, 0, 0, 0},  //TV = 10.72(40 lines)  AV=2.00  SV=5.17  BV=7.55
    {638,1152,1024, 0, 0, 0},  //TV = 10.61(43 lines)  AV=2.00  SV=5.17  BV=7.44
    {683,1152,1024, 0, 0, 0},  //TV = 10.52(46 lines)  AV=2.00  SV=5.17  BV=7.35
    {727,1152,1024, 0, 0, 0},  //TV = 10.43(49 lines)  AV=2.00  SV=5.17  BV=7.26
    {787,1152,1024, 0, 0, 0},  //TV = 10.31(53 lines)  AV=2.00  SV=5.17  BV=7.14
    {846,1136,1024, 0, 0, 0},  //TV = 10.21(57 lines)  AV=2.00  SV=5.15  BV=7.06
    {905,1136,1024, 0, 0, 0},  //TV = 10.11(61 lines)  AV=2.00  SV=5.15  BV=6.96
    {965,1152,1024, 0, 0, 0},  //TV = 10.02(65 lines)  AV=2.00  SV=5.17  BV=6.85
    {1039,1152,1024, 0, 0, 0},  //TV = 9.91(70 lines)  AV=2.00  SV=5.17  BV=6.74
    {1128,1136,1024, 0, 0, 0},  //TV = 9.79(76 lines)  AV=2.00  SV=5.15  BV=6.64
    {1202,1136,1024, 0, 0, 0},  //TV = 9.70(81 lines)  AV=2.00  SV=5.15  BV=6.55
    {1291,1136,1024, 0, 0, 0},  //TV = 9.60(87 lines)  AV=2.00  SV=5.15  BV=6.45
    {1380,1136,1024, 0, 0, 0},  //TV = 9.50(93 lines)  AV=2.00  SV=5.15  BV=6.35
    {1484,1136,1024, 0, 0, 0},  //TV = 9.40(100 lines)  AV=2.00  SV=5.15  BV=6.25
    {1588,1136,1024, 0, 0, 0},  //TV = 9.30(107 lines)  AV=2.00  SV=5.15  BV=6.15
    {1692,1136,1024, 0, 0, 0},  //TV = 9.21(114 lines)  AV=2.00  SV=5.15  BV=6.06
    {1810,1136,1024, 0, 0, 0},  //TV = 9.11(122 lines)  AV=2.00  SV=5.15  BV=5.96
    {1959,1136,1024, 0, 0, 0},  //TV = 9.00(132 lines)  AV=2.00  SV=5.15  BV=5.85
    {2107,1136,1024, 0, 0, 0},  //TV = 8.89(142 lines)  AV=2.00  SV=5.15  BV=5.74
    {2255,1136,1024, 0, 0, 0},  //TV = 8.79(152 lines)  AV=2.00  SV=5.15  BV=5.64
    {2389,1136,1024, 0, 0, 0},  //TV = 8.71(161 lines)  AV=2.00  SV=5.15  BV=5.56
    {2582,1136,1024, 0, 0, 0},  //TV = 8.60(174 lines)  AV=2.00  SV=5.15  BV=5.45
    {2774,1136,1024, 0, 0, 0},  //TV = 8.49(187 lines)  AV=2.00  SV=5.15  BV=5.34
    {2967,1136,1024, 0, 0, 0},  //TV = 8.40(200 lines)  AV=2.00  SV=5.15  BV=5.25
    {3175,1136,1024, 0, 0, 0},  //TV = 8.30(214 lines)  AV=2.00  SV=5.15  BV=5.15
    {3397,1136,1024, 0, 0, 0},  //TV = 8.20(229 lines)  AV=2.00  SV=5.15  BV=5.05
    {3635,1136,1024, 0, 0, 0},  //TV = 8.10(245 lines)  AV=2.00  SV=5.15  BV=4.95
    {3932,1136,1024, 0, 0, 0},  //TV = 7.99(265 lines)  AV=2.00  SV=5.15  BV=4.84
    {4199,1136,1024, 0, 0, 0},  //TV = 7.90(283 lines)  AV=2.00  SV=5.15  BV=4.75
    {4495,1136,1024, 0, 0, 0},  //TV = 7.80(303 lines)  AV=2.00  SV=5.15  BV=4.65
    {4822,1136,1024, 0, 0, 0},  //TV = 7.70(325 lines)  AV=2.00  SV=5.15  BV=4.55
    {5163,1136,1024, 0, 0, 0},  //TV = 7.60(348 lines)  AV=2.00  SV=5.15  BV=4.45
    {5563,1136,1024, 0, 0, 0},  //TV = 7.49(375 lines)  AV=2.00  SV=5.15  BV=4.34
    {5949,1136,1024, 0, 0, 0},  //TV = 7.39(401 lines)  AV=2.00  SV=5.15  BV=4.24
    {6379,1136,1024, 0, 0, 0},  //TV = 7.29(430 lines)  AV=2.00  SV=5.15  BV=4.14
    {6824,1136,1024, 0, 0, 0},  //TV = 7.20(460 lines)  AV=2.00  SV=5.15  BV=4.05
    {7314,1136,1024, 0, 0, 0},  //TV = 7.10(493 lines)  AV=2.00  SV=5.15  BV=3.95
    {7833,1136,1024, 0, 0, 0},  //TV = 7.00(528 lines)  AV=2.00  SV=5.15  BV=3.85
    {8337,1136,1024, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.15  BV=3.76
    {8337,1152,1088, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.26  BV=3.65
    {8337,1280,1048, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.36  BV=3.55
    {8337,1408,1024, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.46  BV=3.45
    {8337,1408,1104, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.57  BV=3.34
    {8337,1536,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.65  BV=3.26
    {8337,1664,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.77  BV=3.14
    {8337,1792,1056, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.85  BV=3.05
    {8337,1920,1064, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.96  BV=2.94
    {8337,2048,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=6.07  BV=2.84
    {16659,1136,1024, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.15  BV=2.76
    {16659,1152,1088, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.26  BV=2.65
    {16659,1280,1056, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.37  BV=2.54
    {16659,1408,1024, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.46  BV=2.45
    {16659,1408,1096, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.56  BV=2.35
    {16659,1536,1080, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.66  BV=2.25
    {24996,1152,1024, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.17  BV=2.15
    {24996,1152,1096, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.27  BV=2.05
    {24996,1280,1056, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.37  BV=1.96
    {24996,1408,1032, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.47  BV=1.85
    {33332,1136,1024, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.15  BV=1.76
    {33332,1152,1088, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.26  BV=1.65
    {33332,1280,1048, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.36  BV=1.55
    {33332,1408,1024, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.46  BV=1.45
    {41669,1152,1072, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=5.24  BV=1.35
    {41669,1280,1032, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=5.33  BV=1.25
    {49991,1152,1024, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.17  BV=1.15
    {49991,1152,1096, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.27  BV=1.05
    {49991,1280,1056, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.37  BV=0.96
    {49991,1408,1032, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.47  BV=0.85
    {49991,1408,1104, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.57  BV=0.75
    {49991,1536,1088, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.67  BV=0.65
    {49991,1664,1072, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.77  BV=0.56
    {49991,1792,1072, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.87  BV=0.45
    {49991,1920,1072, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=5.97  BV=0.35
    {49991,2048,1080, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=6.08  BV=0.25
    {49991,2304,1024, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=6.17  BV=0.15
    {49991,2432,1040, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=6.27  BV=0.05
    {49991,2560,1056, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=6.37  BV=-0.04
    {49991,2816,1032, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=6.47  BV=-0.15
    {49991,2944,1056, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=6.57  BV=-0.25
    {49991,3200,1040, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=6.67  BV=-0.34
    {49991,3456,1032, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=6.77  BV=-0.44
    {49991,3712,1032, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=6.87  BV=-0.55
    {49991,3968,1040, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=6.98  BV=-0.65
    {49991,4224,1048, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=7.08  BV=-0.76
    {49991,4608,1024, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=7.17  BV=-0.85
    {49991,4864,1040, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=7.27  BV=-0.95
    {49991,5248,1032, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=7.37  BV=-1.05
    {49991,5632,1032, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=7.47  BV=-1.15
    {49991,6016,1040, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=7.58  BV=-1.25
    {49991,6528,1024, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=7.67  BV=-1.35
    {49991,6912,1040, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=7.78  BV=-1.46
    {49991,7424,1032, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=7.87  BV=-1.55
    {49991,8064,1024, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=7.98  BV=-1.66
    {49991,8576,1024, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=8.07  BV=-1.74
    {49991,9216,1024, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=8.17  BV=-1.85
    {49991,9856,1024, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=8.27  BV=-1.94
    {49991,10496,1032, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=8.37  BV=-2.05
    {49991,11264,1032, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=8.47  BV=-2.15
    {49991,12160,1024, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=8.57  BV=-2.25
    {49991,13056,1024, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=8.67  BV=-2.35
    {49991,14080,1024, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=8.78  BV=-2.46
    {49991,14976,1024, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=8.87  BV=-2.55
    {49991,16128,1024, 0, 0, 0},  //TV = 4.32(3370 lines)  AV=2.00  SV=8.98  BV=-2.66
    {50006,16384,1080, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=9.08  BV=-2.76
    {58328,15872,1024, 0, 0, 0},  //TV = 4.10(3932 lines)  AV=2.00  SV=8.95  BV=-2.85
    {58328,16384,1064, 0, 0, 0},  //TV = 4.10(3932 lines)  AV=2.00  SV=9.06  BV=-2.96
    {66664,15872,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=8.95  BV=-3.05
    {66664,16384,1064, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=9.06  BV=-3.15
    {75001,16256,1024, 0, 0, 0},  //TV = 3.74(5056 lines)  AV=2.00  SV=8.99  BV=-3.25
    {83338,15744,1024, 0, 0, 0},  //TV = 3.58(5618 lines)  AV=2.00  SV=8.94  BV=-3.36
    {83338,16384,1048, 0, 0, 0},  //TV = 3.58(5618 lines)  AV=2.00  SV=9.03  BV=-3.45
    {91660,16384,1024, 0, 0, 0},  //TV = 3.45(6179 lines)  AV=2.00  SV=9.00  BV=-3.55
    {99996,16128,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.98  BV=-3.66
    {108333,15872,1024, 0, 0, 0},  //TV = 3.21(7303 lines)  AV=2.00  SV=8.95  BV=-3.75
    {116670,15872,1024, 0, 0, 0},  //TV = 3.10(7865 lines)  AV=2.00  SV=8.95  BV=-3.85
    {124992,15872,1024, 0, 0, 0},  //TV = 3.00(8426 lines)  AV=2.00  SV=8.95  BV=-3.95
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
},
};

static strEvPline sCaptureISO1600PLineTable_50Hz =
{
{
    {30,1152,1104, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.28  BV=11.75
    {30,1280,1056, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.37  BV=11.66
    {30,1408,1040, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.48  BV=11.54
    {30,1536,1032, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.60  BV=11.43
    {30,1664,1024, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.70  BV=11.32
    {45,1152,1048, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.20  BV=11.24
    {45,1152,1120, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.30  BV=11.14
    {45,1280,1088, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.41  BV=11.03
    {45,1408,1040, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.48  BV=10.96
    {60,1152,1056, 0, 0, 0},  /* TV = 14.02(4 lines)  AV=2.00  SV=5.21  BV=10.81 */
    {60,1152,1128, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.31  BV=10.72
    {60,1280,1072, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.39  BV=10.64
    {75,1152,1032, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.18  BV=10.52
    {75,1152,1088, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.26  BV=10.45
    {75,1280,1056, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.37  BV=10.34
    {90,1152,1040, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.19  BV=10.25
    {90,1280,1024, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.32  BV=10.12
    {104,1152,1040, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.19  BV=10.04
    {104,1152,1104, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.28  BV=9.95
    {119,1152,1032, 0, 0, 0},  //TV = 13.04(8 lines)  AV=2.00  SV=5.18  BV=9.86
    {134,1136,1024, 0, 0, 0},  /* TV = 12.87(9 lines)  AV=2.00  SV=5.15  BV=9.72 */
    {134,1152,1064, 0, 0, 0},  //TV = 12.87(9 lines)  AV=2.00  SV=5.23  BV=9.64
    {149,1152,1032, 0, 0, 0},  //TV = 12.71(10 lines)  AV=2.00  SV=5.18  BV=9.53
    {164,1136,1024, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.15  BV=9.42
    {164,1152,1088, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.26  BV=9.32
    {179,1152,1040, 0, 0, 0},  /* TV = 12.45(12 lines)  AV=2.00  SV=5.19  BV=9.26 */
    {193,1152,1032, 0, 0, 0},  //TV = 12.34(13 lines)  AV=2.00  SV=5.18  BV=9.16
    {208,1152,1032, 0, 0, 0},  //TV = 12.23(14 lines)  AV=2.00  SV=5.18  BV=9.05
    {223,1152,1032, 0, 0, 0},  //TV = 12.13(15 lines)  AV=2.00  SV=5.18  BV=8.95
    {238,1152,1032, 0, 0, 0},  //TV = 12.04(16 lines)  AV=2.00  SV=5.18  BV=8.86
    {253,1152,1040, 0, 0, 0},  //TV = 11.95(17 lines)  AV=2.00  SV=5.19  BV=8.76
    {268,1152,1056, 0, 0, 0},  //TV = 11.87(18 lines)  AV=2.00  SV=5.21  BV=8.65
    {297,1152,1024, 0, 0, 0},  //TV = 11.72(20 lines)  AV=2.00  SV=5.17  BV=8.55
    {312,1152,1040, 0, 0, 0},  //TV = 11.65(21 lines)  AV=2.00  SV=5.19  BV=8.45
    {342,1152,1024, 0, 0, 0},  //TV = 11.51(23 lines)  AV=2.00  SV=5.17  BV=8.34
    {371,1136,1024, 0, 0, 0},  //TV = 11.40(25 lines)  AV=2.00  SV=5.15  BV=8.25
    {386,1152,1032, 0, 0, 0},  //TV = 11.34(26 lines)  AV=2.00  SV=5.18  BV=8.16
    {416,1152,1032, 0, 0, 0},  //TV = 11.23(28 lines)  AV=2.00  SV=5.18  BV=8.05
    {446,1152,1032, 0, 0, 0},  //TV = 11.13(30 lines)  AV=2.00  SV=5.18  BV=7.95
    {475,1152,1032, 0, 0, 0},  //TV = 11.04(32 lines)  AV=2.00  SV=5.18  BV=7.86
    {520,1136,1024, 0, 0, 0},  //TV = 10.91(35 lines)  AV=2.00  SV=5.15  BV=7.76
    {564,1136,1024, 0, 0, 0},  //TV = 10.79(38 lines)  AV=2.00  SV=5.15  BV=7.64
    {594,1152,1024, 0, 0, 0},  //TV = 10.72(40 lines)  AV=2.00  SV=5.17  BV=7.55
    {638,1152,1024, 0, 0, 0},  //TV = 10.61(43 lines)  AV=2.00  SV=5.17  BV=7.44
    {683,1152,1024, 0, 0, 0},  //TV = 10.52(46 lines)  AV=2.00  SV=5.17  BV=7.35
    {727,1152,1024, 0, 0, 0},  //TV = 10.43(49 lines)  AV=2.00  SV=5.17  BV=7.26
    {787,1152,1024, 0, 0, 0},  //TV = 10.31(53 lines)  AV=2.00  SV=5.17  BV=7.14
    {846,1136,1024, 0, 0, 0},  //TV = 10.21(57 lines)  AV=2.00  SV=5.15  BV=7.06
    {905,1136,1024, 0, 0, 0},  //TV = 10.11(61 lines)  AV=2.00  SV=5.15  BV=6.96
    {965,1152,1024, 0, 0, 0},  //TV = 10.02(65 lines)  AV=2.00  SV=5.17  BV=6.85
    {1039,1152,1024, 0, 0, 0},  //TV = 9.91(70 lines)  AV=2.00  SV=5.17  BV=6.74
    {1128,1136,1024, 0, 0, 0},  //TV = 9.79(76 lines)  AV=2.00  SV=5.15  BV=6.64
    {1202,1136,1024, 0, 0, 0},  //TV = 9.70(81 lines)  AV=2.00  SV=5.15  BV=6.55
    {1291,1136,1024, 0, 0, 0},  //TV = 9.60(87 lines)  AV=2.00  SV=5.15  BV=6.45
    {1380,1136,1024, 0, 0, 0},  //TV = 9.50(93 lines)  AV=2.00  SV=5.15  BV=6.35
    {1484,1136,1024, 0, 0, 0},  //TV = 9.40(100 lines)  AV=2.00  SV=5.15  BV=6.25
    {1588,1136,1024, 0, 0, 0},  //TV = 9.30(107 lines)  AV=2.00  SV=5.15  BV=6.15
    {1692,1136,1024, 0, 0, 0},  //TV = 9.21(114 lines)  AV=2.00  SV=5.15  BV=6.06
    {1810,1136,1024, 0, 0, 0},  //TV = 9.11(122 lines)  AV=2.00  SV=5.15  BV=5.96
    {1959,1136,1024, 0, 0, 0},  //TV = 9.00(132 lines)  AV=2.00  SV=5.15  BV=5.85
    {2107,1136,1024, 0, 0, 0},  //TV = 8.89(142 lines)  AV=2.00  SV=5.15  BV=5.74
    {2255,1136,1024, 0, 0, 0},  //TV = 8.79(152 lines)  AV=2.00  SV=5.15  BV=5.64
    {2389,1136,1024, 0, 0, 0},  //TV = 8.71(161 lines)  AV=2.00  SV=5.15  BV=5.56
    {2582,1136,1024, 0, 0, 0},  //TV = 8.60(174 lines)  AV=2.00  SV=5.15  BV=5.45
    {2774,1136,1024, 0, 0, 0},  //TV = 8.49(187 lines)  AV=2.00  SV=5.15  BV=5.34
    {2967,1136,1024, 0, 0, 0},  //TV = 8.40(200 lines)  AV=2.00  SV=5.15  BV=5.25
    {3175,1136,1024, 0, 0, 0},  //TV = 8.30(214 lines)  AV=2.00  SV=5.15  BV=5.15
    {3397,1136,1024, 0, 0, 0},  //TV = 8.20(229 lines)  AV=2.00  SV=5.15  BV=5.05
    {3635,1136,1024, 0, 0, 0},  //TV = 8.10(245 lines)  AV=2.00  SV=5.15  BV=4.95
    {3932,1136,1024, 0, 0, 0},  //TV = 7.99(265 lines)  AV=2.00  SV=5.15  BV=4.84
    {4199,1136,1024, 0, 0, 0},  //TV = 7.90(283 lines)  AV=2.00  SV=5.15  BV=4.75
    {4495,1136,1024, 0, 0, 0},  //TV = 7.80(303 lines)  AV=2.00  SV=5.15  BV=4.65
    {4822,1136,1024, 0, 0, 0},  //TV = 7.70(325 lines)  AV=2.00  SV=5.15  BV=4.55
    {5163,1136,1024, 0, 0, 0},  //TV = 7.60(348 lines)  AV=2.00  SV=5.15  BV=4.45
    {5563,1136,1024, 0, 0, 0},  //TV = 7.49(375 lines)  AV=2.00  SV=5.15  BV=4.34
    {5949,1136,1024, 0, 0, 0},  //TV = 7.39(401 lines)  AV=2.00  SV=5.15  BV=4.24
    {6379,1136,1024, 0, 0, 0},  //TV = 7.29(430 lines)  AV=2.00  SV=5.15  BV=4.14
    {6824,1136,1024, 0, 0, 0},  //TV = 7.20(460 lines)  AV=2.00  SV=5.15  BV=4.05
    {7314,1136,1024, 0, 0, 0},  //TV = 7.10(493 lines)  AV=2.00  SV=5.15  BV=3.95
    {7833,1136,1024, 0, 0, 0},  //TV = 7.00(528 lines)  AV=2.00  SV=5.15  BV=3.85
    {8397,1136,1024, 0, 0, 0},  //TV = 6.90(566 lines)  AV=2.00  SV=5.15  BV=3.75
    {8990,1136,1024, 0, 0, 0},  //TV = 6.80(606 lines)  AV=2.00  SV=5.15  BV=3.65
    {9628,1136,1024, 0, 0, 0},  //TV = 6.70(649 lines)  AV=2.00  SV=5.15  BV=3.55
    {9999,1152,1040, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.19  BV=3.45
    {9999,1152,1112, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.29  BV=3.36
    {9999,1280,1080, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.40  BV=3.25
    {9999,1408,1056, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.50  BV=3.14
    {9999,1536,1032, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.60  BV=3.05
    {9999,1664,1024, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.70  BV=2.94
    {9999,1664,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.79  BV=2.86
    {9999,1792,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.89  BV=2.75
    {9999,1920,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.99  BV=2.65
    {9999,2176,1032, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=6.10  BV=2.55
    {19997,1152,1040, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.19  BV=2.45
    {19997,1152,1112, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.29  BV=2.36
    {19997,1280,1080, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.40  BV=2.25
    {19997,1408,1048, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.49  BV=2.15
    {19997,1536,1032, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.60  BV=2.05
    {19997,1664,1024, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.70  BV=1.94
    {29995,1152,1048, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.20  BV=1.86
    {29995,1152,1128, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.31  BV=1.75
    {29995,1280,1088, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.41  BV=1.65
    {29995,1408,1064, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.51  BV=1.54
    {39993,1152,1040, 0, 0, 0},  //TV = 4.64(2696 lines)  AV=2.00  SV=5.19  BV=1.45
    {39993,1152,1112, 0, 0, 0},  //TV = 4.64(2696 lines)  AV=2.00  SV=5.29  BV=1.36
    {39993,1280,1080, 0, 0, 0},  //TV = 4.64(2696 lines)  AV=2.00  SV=5.40  BV=1.25
    {50006,1152,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.17  BV=1.15
    {50006,1152,1096, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.27  BV=1.05
    {50006,1280,1056, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.37  BV=0.96
    {50006,1408,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.47  BV=0.85
    {50006,1408,1104, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.57  BV=0.75
    {50006,1536,1088, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.67  BV=0.65
    {50006,1664,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.77  BV=0.56
    {50006,1792,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.87  BV=0.45
    {50006,1920,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.97  BV=0.35
    {50006,2048,1080, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.08  BV=0.24
    {50006,2304,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.17  BV=0.15
    {50006,2432,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.27  BV=0.05
    {50006,2560,1056, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.37  BV=-0.04
    {50006,2816,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.47  BV=-0.15
    {50006,2944,1056, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.57  BV=-0.25
    {50006,3200,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.67  BV=-0.34
    {50006,3456,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.77  BV=-0.44
    {50006,3712,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.87  BV=-0.55
    {50006,3968,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.98  BV=-0.65
    {50006,4224,1048, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.08  BV=-0.76
    {50006,4608,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.17  BV=-0.85
    {50006,4864,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.27  BV=-0.95
    {50006,5248,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.37  BV=-1.05
    {50006,5632,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.48  BV=-1.16
    {50006,6016,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.58  BV=-1.26
    {50006,6528,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.67  BV=-1.35
    {50006,6912,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.78  BV=-1.46
    {50006,7424,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.87  BV=-1.55
    {50006,8064,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.98  BV=-1.66
    {50006,8576,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=8.07  BV=-1.74
    {50006,9216,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=8.17  BV=-1.85
    {50006,9856,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=8.27  BV=-1.95
    {50006,10496,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=8.37  BV=-2.05
    {50006,11264,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=8.47  BV=-2.15
    {50006,12160,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=8.57  BV=-2.25
    {50006,13056,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=8.67  BV=-2.35
    {50006,14080,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=8.78  BV=-2.46
    {50006,14976,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=8.87  BV=-2.55
    {50006,16128,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=8.98  BV=-2.66
    {50006,16384,1080, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=9.08  BV=-2.76
    {60004,15360,1024, 0, 0, 0},  //TV = 4.06(4045 lines)  AV=2.00  SV=8.91  BV=-2.85
    {60004,16384,1032, 0, 0, 0},  //TV = 4.06(4045 lines)  AV=2.00  SV=9.01  BV=-2.95
    {70002,15104,1032, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=8.89  BV=-3.06
    {70002,16256,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=8.99  BV=-3.15
    {70002,16384,1088, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=9.09  BV=-3.25
    {80000,16384,1024, 0, 0, 0},  //TV = 3.64(5393 lines)  AV=2.00  SV=9.00  BV=-3.36
    {89998,15616,1024, 0, 0, 0},  //TV = 3.47(6067 lines)  AV=2.00  SV=8.93  BV=-3.46
    {89998,16384,1040, 0, 0, 0},  //TV = 3.47(6067 lines)  AV=2.00  SV=9.02  BV=-3.55
    {99996,16128,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.98  BV=-3.66
    {109995,15616,1024, 0, 0, 0},  //TV = 3.18(7415 lines)  AV=2.00  SV=8.93  BV=-3.75
    {109995,16384,1048, 0, 0, 0},  //TV = 3.18(7415 lines)  AV=2.00  SV=9.03  BV=-3.85
    {120008,16384,1032, 0, 0, 0},  //TV = 3.06(8090 lines)  AV=2.00  SV=9.01  BV=-3.95
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
},
};

static strAETable g_AE_CaptureISO1600Table =
{
    AETABLE_CAPTURE_ISO1600, //eAETableID
    158, //u4TotalIndex
    -20, //i4StrobeTrigerBV
    118, //i4MaxBV
    -40, //i4MinBV
    90, //i4EffectiveMaxBV
    0, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_1600, //ISO
    sCaptureISO1600PLineTable_60Hz,
    sCaptureISO1600PLineTable_50Hz,
    NULL,
};
static strEvPline sCaptureStrobePLineTable_60Hz =
{
{
    {30,1152,1104, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.28  BV=11.75
    {30,1280,1056, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.37  BV=11.66
    {30,1408,1040, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.48  BV=11.54
    {30,1536,1032, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.60  BV=11.43
    {30,1664,1024, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.70  BV=11.32
    {45,1152,1048, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.20  BV=11.24
    {45,1152,1120, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.30  BV=11.14
    {45,1280,1088, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.41  BV=11.03
    {45,1408,1040, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.48  BV=10.96
    {60,1152,1056, 0, 0, 0},  /* TV = 14.02(4 lines)  AV=2.00  SV=5.21  BV=10.81 */
    {60,1152,1128, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.31  BV=10.72
    {60,1280,1072, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.39  BV=10.64
    {75,1152,1032, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.18  BV=10.52
    {75,1152,1088, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.26  BV=10.45
    {75,1280,1056, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.37  BV=10.34
    {90,1152,1040, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.19  BV=10.25
    {90,1280,1024, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.32  BV=10.12
    {104,1152,1040, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.19  BV=10.04
    {104,1152,1104, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.28  BV=9.95
    {119,1152,1032, 0, 0, 0},  //TV = 13.04(8 lines)  AV=2.00  SV=5.18  BV=9.86
    {134,1136,1024, 0, 0, 0},  /* TV = 12.87(9 lines)  AV=2.00  SV=5.15  BV=9.72 */
    {134,1152,1064, 0, 0, 0},  //TV = 12.87(9 lines)  AV=2.00  SV=5.23  BV=9.64
    {149,1152,1032, 0, 0, 0},  //TV = 12.71(10 lines)  AV=2.00  SV=5.18  BV=9.53
    {164,1136,1024, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.15  BV=9.42
    {164,1152,1088, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.26  BV=9.32
    {179,1152,1040, 0, 0, 0},  /* TV = 12.45(12 lines)  AV=2.00  SV=5.19  BV=9.26 */
    {193,1152,1032, 0, 0, 0},  //TV = 12.34(13 lines)  AV=2.00  SV=5.18  BV=9.16
    {208,1152,1032, 0, 0, 0},  //TV = 12.23(14 lines)  AV=2.00  SV=5.18  BV=9.05
    {223,1152,1032, 0, 0, 0},  //TV = 12.13(15 lines)  AV=2.00  SV=5.18  BV=8.95
    {238,1152,1032, 0, 0, 0},  //TV = 12.04(16 lines)  AV=2.00  SV=5.18  BV=8.86
    {253,1152,1040, 0, 0, 0},  //TV = 11.95(17 lines)  AV=2.00  SV=5.19  BV=8.76
    {268,1152,1056, 0, 0, 0},  //TV = 11.87(18 lines)  AV=2.00  SV=5.21  BV=8.65
    {297,1152,1024, 0, 0, 0},  //TV = 11.72(20 lines)  AV=2.00  SV=5.17  BV=8.55
    {312,1152,1040, 0, 0, 0},  //TV = 11.65(21 lines)  AV=2.00  SV=5.19  BV=8.45
    {342,1152,1024, 0, 0, 0},  //TV = 11.51(23 lines)  AV=2.00  SV=5.17  BV=8.34
    {371,1136,1024, 0, 0, 0},  //TV = 11.40(25 lines)  AV=2.00  SV=5.15  BV=8.25
    {386,1152,1032, 0, 0, 0},  //TV = 11.34(26 lines)  AV=2.00  SV=5.18  BV=8.16
    {416,1152,1032, 0, 0, 0},  //TV = 11.23(28 lines)  AV=2.00  SV=5.18  BV=8.05
    {446,1152,1032, 0, 0, 0},  //TV = 11.13(30 lines)  AV=2.00  SV=5.18  BV=7.95
    {475,1152,1032, 0, 0, 0},  //TV = 11.04(32 lines)  AV=2.00  SV=5.18  BV=7.86
    {520,1136,1024, 0, 0, 0},  //TV = 10.91(35 lines)  AV=2.00  SV=5.15  BV=7.76
    {564,1136,1024, 0, 0, 0},  //TV = 10.79(38 lines)  AV=2.00  SV=5.15  BV=7.64
    {594,1152,1024, 0, 0, 0},  //TV = 10.72(40 lines)  AV=2.00  SV=5.17  BV=7.55
    {638,1152,1024, 0, 0, 0},  //TV = 10.61(43 lines)  AV=2.00  SV=5.17  BV=7.44
    {683,1152,1024, 0, 0, 0},  //TV = 10.52(46 lines)  AV=2.00  SV=5.17  BV=7.35
    {727,1152,1024, 0, 0, 0},  //TV = 10.43(49 lines)  AV=2.00  SV=5.17  BV=7.26
    {787,1152,1024, 0, 0, 0},  //TV = 10.31(53 lines)  AV=2.00  SV=5.17  BV=7.14
    {846,1136,1024, 0, 0, 0},  //TV = 10.21(57 lines)  AV=2.00  SV=5.15  BV=7.06
    {905,1136,1024, 0, 0, 0},  //TV = 10.11(61 lines)  AV=2.00  SV=5.15  BV=6.96
    {965,1152,1024, 0, 0, 0},  //TV = 10.02(65 lines)  AV=2.00  SV=5.17  BV=6.85
    {1039,1152,1024, 0, 0, 0},  //TV = 9.91(70 lines)  AV=2.00  SV=5.17  BV=6.74
    {1128,1136,1024, 0, 0, 0},  //TV = 9.79(76 lines)  AV=2.00  SV=5.15  BV=6.64
    {1202,1136,1024, 0, 0, 0},  //TV = 9.70(81 lines)  AV=2.00  SV=5.15  BV=6.55
    {1291,1136,1024, 0, 0, 0},  //TV = 9.60(87 lines)  AV=2.00  SV=5.15  BV=6.45
    {1380,1136,1024, 0, 0, 0},  //TV = 9.50(93 lines)  AV=2.00  SV=5.15  BV=6.35
    {1484,1136,1024, 0, 0, 0},  //TV = 9.40(100 lines)  AV=2.00  SV=5.15  BV=6.25
    {1588,1136,1024, 0, 0, 0},  //TV = 9.30(107 lines)  AV=2.00  SV=5.15  BV=6.15
    {1692,1136,1024, 0, 0, 0},  //TV = 9.21(114 lines)  AV=2.00  SV=5.15  BV=6.06
    {1810,1136,1024, 0, 0, 0},  //TV = 9.11(122 lines)  AV=2.00  SV=5.15  BV=5.96
    {1959,1136,1024, 0, 0, 0},  //TV = 9.00(132 lines)  AV=2.00  SV=5.15  BV=5.85
    {2107,1136,1024, 0, 0, 0},  //TV = 8.89(142 lines)  AV=2.00  SV=5.15  BV=5.74
    {2255,1136,1024, 0, 0, 0},  //TV = 8.79(152 lines)  AV=2.00  SV=5.15  BV=5.64
    {2389,1136,1024, 0, 0, 0},  //TV = 8.71(161 lines)  AV=2.00  SV=5.15  BV=5.56
    {2582,1136,1024, 0, 0, 0},  //TV = 8.60(174 lines)  AV=2.00  SV=5.15  BV=5.45
    {2774,1136,1024, 0, 0, 0},  //TV = 8.49(187 lines)  AV=2.00  SV=5.15  BV=5.34
    {2967,1136,1024, 0, 0, 0},  //TV = 8.40(200 lines)  AV=2.00  SV=5.15  BV=5.25
    {3175,1136,1024, 0, 0, 0},  //TV = 8.30(214 lines)  AV=2.00  SV=5.15  BV=5.15
    {3397,1136,1024, 0, 0, 0},  //TV = 8.20(229 lines)  AV=2.00  SV=5.15  BV=5.05
    {3635,1136,1024, 0, 0, 0},  //TV = 8.10(245 lines)  AV=2.00  SV=5.15  BV=4.95
    {3932,1136,1024, 0, 0, 0},  //TV = 7.99(265 lines)  AV=2.00  SV=5.15  BV=4.84
    {4199,1136,1024, 0, 0, 0},  //TV = 7.90(283 lines)  AV=2.00  SV=5.15  BV=4.75
    {4495,1136,1024, 0, 0, 0},  //TV = 7.80(303 lines)  AV=2.00  SV=5.15  BV=4.65
    {4822,1136,1024, 0, 0, 0},  //TV = 7.70(325 lines)  AV=2.00  SV=5.15  BV=4.55
    {5163,1136,1024, 0, 0, 0},  //TV = 7.60(348 lines)  AV=2.00  SV=5.15  BV=4.45
    {5563,1136,1024, 0, 0, 0},  //TV = 7.49(375 lines)  AV=2.00  SV=5.15  BV=4.34
    {5949,1136,1024, 0, 0, 0},  //TV = 7.39(401 lines)  AV=2.00  SV=5.15  BV=4.24
    {6379,1136,1024, 0, 0, 0},  //TV = 7.29(430 lines)  AV=2.00  SV=5.15  BV=4.14
    {6824,1136,1024, 0, 0, 0},  //TV = 7.20(460 lines)  AV=2.00  SV=5.15  BV=4.05
    {7314,1136,1024, 0, 0, 0},  //TV = 7.10(493 lines)  AV=2.00  SV=5.15  BV=3.95
    {7833,1136,1024, 0, 0, 0},  //TV = 7.00(528 lines)  AV=2.00  SV=5.15  BV=3.85
    {8337,1136,1024, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.15  BV=3.76
    {8337,1152,1088, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.26  BV=3.65
    {8337,1280,1048, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.36  BV=3.55
    {8337,1408,1024, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.46  BV=3.45
    {8337,1408,1104, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.57  BV=3.34
    {8337,1536,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.65  BV=3.26
    {8337,1664,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.77  BV=3.14
    {8337,1792,1056, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.85  BV=3.05
    {8337,1920,1064, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.96  BV=2.94
    {8337,2048,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=6.07  BV=2.84
    {16659,1136,1024, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.15  BV=2.76
    {16659,1152,1088, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.26  BV=2.65
    {16659,1280,1056, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.37  BV=2.54
    {16659,1408,1024, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.46  BV=2.45
    {16659,1408,1096, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.56  BV=2.35
    {16659,1536,1080, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.66  BV=2.25
    {24996,1152,1024, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.17  BV=2.15
    {24996,1152,1096, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.27  BV=2.05
    {24996,1280,1056, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.37  BV=1.96
    {24996,1408,1032, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.47  BV=1.85
    {33332,1136,1024, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.15  BV=1.76
    {33332,1152,1088, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.26  BV=1.65
    {33332,1280,1048, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.36  BV=1.55
    {33332,1408,1024, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.46  BV=1.45
    {33332,1408,1096, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.56  BV=1.35
    {33332,1536,1072, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.65  BV=1.26
    {33332,1664,1072, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.77  BV=1.14
    {33332,1792,1064, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.86  BV=1.04
    {33332,1920,1056, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.95  BV=0.96
    {33332,2048,1064, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=6.06  BV=0.85
    {33332,2176,1072, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=6.15  BV=0.75
    {41669,1920,1040, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=5.93  BV=0.66
    {41669,2048,1048, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=6.03  BV=0.55
    {41669,2176,1056, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=6.13  BV=0.45
    {50006,1920,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.97  BV=0.35
    {50006,2048,1080, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.08  BV=0.24
    {50006,2304,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.17  BV=0.15
    {50006,2432,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.27  BV=0.05
    {50006,2560,1064, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.38  BV=-0.06
    {50006,2816,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.47  BV=-0.15
    {50006,2944,1056, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.57  BV=-0.25
    {50006,3200,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.67  BV=-0.34
    {50006,3456,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.78  BV=-0.46
    {50006,3712,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.87  BV=-0.55
    {50006,3968,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.97  BV=-0.64
    {50006,4224,1048, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.08  BV=-0.76
    {58328,3968,1024, 0, 0, 0},  //TV = 4.10(3932 lines)  AV=2.00  SV=6.95  BV=-0.85
    {58328,4224,1032, 0, 0, 0},  //TV = 4.10(3932 lines)  AV=2.00  SV=7.06  BV=-0.96
    {66664,3968,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=6.97  BV=-1.06
    {66664,4224,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.06  BV=-1.15
    {66664,4480,1040, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.15  BV=-1.24
    {66664,4864,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.26  BV=-1.35
    {66664,5248,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.36  BV=-1.45
    {66664,5632,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.46  BV=-1.55
    {66664,6016,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.55  BV=-1.65
    {66664,6400,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.66  BV=-1.75
    {66664,6912,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.75  BV=-1.85
    {66664,7424,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.86  BV=-1.95
    {66664,7936,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.95  BV=-2.05
    {66664,8448,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=8.06  BV=-2.15
    {75001,8064,1032, 0, 0, 0},  //TV = 3.74(5056 lines)  AV=2.00  SV=7.99  BV=-2.25
    {83338,7808,1024, 0, 0, 0},  //TV = 3.58(5618 lines)  AV=2.00  SV=7.93  BV=-2.35
    {83338,8320,1032, 0, 0, 0},  //TV = 3.58(5618 lines)  AV=2.00  SV=8.03  BV=-2.45
    {91660,8192,1024, 0, 0, 0},  //TV = 3.45(6179 lines)  AV=2.00  SV=8.00  BV=-2.55
    {99996,7936,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=7.97  BV=-2.64
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
},
};

static strEvPline sCaptureStrobePLineTable_50Hz =
{
{
    {30,1152,1104, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.28  BV=11.75
    {30,1280,1056, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.37  BV=11.66
    {30,1408,1040, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.48  BV=11.54
    {30,1536,1032, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.60  BV=11.43
    {30,1664,1024, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.70  BV=11.32
    {45,1152,1048, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.20  BV=11.24
    {45,1152,1120, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.30  BV=11.14
    {45,1280,1088, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.41  BV=11.03
    {45,1408,1040, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.48  BV=10.96
    {60,1152,1056, 0, 0, 0},  /* TV = 14.02(4 lines)  AV=2.00  SV=5.21  BV=10.81 */
    {60,1152,1128, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.31  BV=10.72
    {60,1280,1072, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.39  BV=10.64
    {75,1152,1032, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.18  BV=10.52
    {75,1152,1088, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.26  BV=10.45
    {75,1280,1056, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.37  BV=10.34
    {90,1152,1040, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.19  BV=10.25
    {90,1280,1024, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.32  BV=10.12
    {104,1152,1040, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.19  BV=10.04
    {104,1152,1104, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.28  BV=9.95
    {119,1152,1032, 0, 0, 0},  //TV = 13.04(8 lines)  AV=2.00  SV=5.18  BV=9.86
    {134,1136,1024, 0, 0, 0},  /* TV = 12.87(9 lines)  AV=2.00  SV=5.15  BV=9.72 */
    {134,1152,1064, 0, 0, 0},  //TV = 12.87(9 lines)  AV=2.00  SV=5.23  BV=9.64
    {149,1152,1032, 0, 0, 0},  //TV = 12.71(10 lines)  AV=2.00  SV=5.18  BV=9.53
    {164,1136,1024, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.15  BV=9.42
    {164,1152,1088, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.26  BV=9.32
    {179,1152,1040, 0, 0, 0},  /* TV = 12.45(12 lines)  AV=2.00  SV=5.19  BV=9.26 */
    {193,1152,1032, 0, 0, 0},  //TV = 12.34(13 lines)  AV=2.00  SV=5.18  BV=9.16
    {208,1152,1032, 0, 0, 0},  //TV = 12.23(14 lines)  AV=2.00  SV=5.18  BV=9.05
    {223,1152,1032, 0, 0, 0},  //TV = 12.13(15 lines)  AV=2.00  SV=5.18  BV=8.95
    {238,1152,1032, 0, 0, 0},  //TV = 12.04(16 lines)  AV=2.00  SV=5.18  BV=8.86
    {253,1152,1040, 0, 0, 0},  //TV = 11.95(17 lines)  AV=2.00  SV=5.19  BV=8.76
    {268,1152,1056, 0, 0, 0},  //TV = 11.87(18 lines)  AV=2.00  SV=5.21  BV=8.65
    {297,1152,1024, 0, 0, 0},  //TV = 11.72(20 lines)  AV=2.00  SV=5.17  BV=8.55
    {312,1152,1040, 0, 0, 0},  //TV = 11.65(21 lines)  AV=2.00  SV=5.19  BV=8.45
    {342,1152,1024, 0, 0, 0},  //TV = 11.51(23 lines)  AV=2.00  SV=5.17  BV=8.34
    {371,1136,1024, 0, 0, 0},  //TV = 11.40(25 lines)  AV=2.00  SV=5.15  BV=8.25
    {386,1152,1032, 0, 0, 0},  //TV = 11.34(26 lines)  AV=2.00  SV=5.18  BV=8.16
    {416,1152,1032, 0, 0, 0},  //TV = 11.23(28 lines)  AV=2.00  SV=5.18  BV=8.05
    {446,1152,1032, 0, 0, 0},  //TV = 11.13(30 lines)  AV=2.00  SV=5.18  BV=7.95
    {475,1152,1032, 0, 0, 0},  //TV = 11.04(32 lines)  AV=2.00  SV=5.18  BV=7.86
    {520,1136,1024, 0, 0, 0},  //TV = 10.91(35 lines)  AV=2.00  SV=5.15  BV=7.76
    {564,1136,1024, 0, 0, 0},  //TV = 10.79(38 lines)  AV=2.00  SV=5.15  BV=7.64
    {594,1152,1024, 0, 0, 0},  //TV = 10.72(40 lines)  AV=2.00  SV=5.17  BV=7.55
    {638,1152,1024, 0, 0, 0},  //TV = 10.61(43 lines)  AV=2.00  SV=5.17  BV=7.44
    {683,1152,1024, 0, 0, 0},  //TV = 10.52(46 lines)  AV=2.00  SV=5.17  BV=7.35
    {727,1152,1024, 0, 0, 0},  //TV = 10.43(49 lines)  AV=2.00  SV=5.17  BV=7.26
    {787,1152,1024, 0, 0, 0},  //TV = 10.31(53 lines)  AV=2.00  SV=5.17  BV=7.14
    {846,1136,1024, 0, 0, 0},  //TV = 10.21(57 lines)  AV=2.00  SV=5.15  BV=7.06
    {905,1136,1024, 0, 0, 0},  //TV = 10.11(61 lines)  AV=2.00  SV=5.15  BV=6.96
    {965,1152,1024, 0, 0, 0},  //TV = 10.02(65 lines)  AV=2.00  SV=5.17  BV=6.85
    {1039,1152,1024, 0, 0, 0},  //TV = 9.91(70 lines)  AV=2.00  SV=5.17  BV=6.74
    {1128,1136,1024, 0, 0, 0},  //TV = 9.79(76 lines)  AV=2.00  SV=5.15  BV=6.64
    {1202,1136,1024, 0, 0, 0},  //TV = 9.70(81 lines)  AV=2.00  SV=5.15  BV=6.55
    {1291,1136,1024, 0, 0, 0},  //TV = 9.60(87 lines)  AV=2.00  SV=5.15  BV=6.45
    {1380,1136,1024, 0, 0, 0},  //TV = 9.50(93 lines)  AV=2.00  SV=5.15  BV=6.35
    {1484,1136,1024, 0, 0, 0},  //TV = 9.40(100 lines)  AV=2.00  SV=5.15  BV=6.25
    {1588,1136,1024, 0, 0, 0},  //TV = 9.30(107 lines)  AV=2.00  SV=5.15  BV=6.15
    {1692,1136,1024, 0, 0, 0},  //TV = 9.21(114 lines)  AV=2.00  SV=5.15  BV=6.06
    {1810,1136,1024, 0, 0, 0},  //TV = 9.11(122 lines)  AV=2.00  SV=5.15  BV=5.96
    {1959,1136,1024, 0, 0, 0},  //TV = 9.00(132 lines)  AV=2.00  SV=5.15  BV=5.85
    {2107,1136,1024, 0, 0, 0},  //TV = 8.89(142 lines)  AV=2.00  SV=5.15  BV=5.74
    {2255,1136,1024, 0, 0, 0},  //TV = 8.79(152 lines)  AV=2.00  SV=5.15  BV=5.64
    {2389,1136,1024, 0, 0, 0},  //TV = 8.71(161 lines)  AV=2.00  SV=5.15  BV=5.56
    {2582,1136,1024, 0, 0, 0},  //TV = 8.60(174 lines)  AV=2.00  SV=5.15  BV=5.45
    {2774,1136,1024, 0, 0, 0},  //TV = 8.49(187 lines)  AV=2.00  SV=5.15  BV=5.34
    {2967,1136,1024, 0, 0, 0},  //TV = 8.40(200 lines)  AV=2.00  SV=5.15  BV=5.25
    {3175,1136,1024, 0, 0, 0},  //TV = 8.30(214 lines)  AV=2.00  SV=5.15  BV=5.15
    {3397,1136,1024, 0, 0, 0},  //TV = 8.20(229 lines)  AV=2.00  SV=5.15  BV=5.05
    {3635,1136,1024, 0, 0, 0},  //TV = 8.10(245 lines)  AV=2.00  SV=5.15  BV=4.95
    {3932,1136,1024, 0, 0, 0},  //TV = 7.99(265 lines)  AV=2.00  SV=5.15  BV=4.84
    {4199,1136,1024, 0, 0, 0},  //TV = 7.90(283 lines)  AV=2.00  SV=5.15  BV=4.75
    {4495,1136,1024, 0, 0, 0},  //TV = 7.80(303 lines)  AV=2.00  SV=5.15  BV=4.65
    {4822,1136,1024, 0, 0, 0},  //TV = 7.70(325 lines)  AV=2.00  SV=5.15  BV=4.55
    {5163,1136,1024, 0, 0, 0},  //TV = 7.60(348 lines)  AV=2.00  SV=5.15  BV=4.45
    {5563,1136,1024, 0, 0, 0},  //TV = 7.49(375 lines)  AV=2.00  SV=5.15  BV=4.34
    {5949,1136,1024, 0, 0, 0},  //TV = 7.39(401 lines)  AV=2.00  SV=5.15  BV=4.24
    {6379,1136,1024, 0, 0, 0},  //TV = 7.29(430 lines)  AV=2.00  SV=5.15  BV=4.14
    {6824,1136,1024, 0, 0, 0},  //TV = 7.20(460 lines)  AV=2.00  SV=5.15  BV=4.05
    {7314,1136,1024, 0, 0, 0},  //TV = 7.10(493 lines)  AV=2.00  SV=5.15  BV=3.95
    {7833,1136,1024, 0, 0, 0},  //TV = 7.00(528 lines)  AV=2.00  SV=5.15  BV=3.85
    {8397,1136,1024, 0, 0, 0},  //TV = 6.90(566 lines)  AV=2.00  SV=5.15  BV=3.75
    {8990,1136,1024, 0, 0, 0},  //TV = 6.80(606 lines)  AV=2.00  SV=5.15  BV=3.65
    {9628,1136,1024, 0, 0, 0},  //TV = 6.70(649 lines)  AV=2.00  SV=5.15  BV=3.55
    {9999,1152,1040, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.19  BV=3.45
    {9999,1152,1112, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.29  BV=3.36
    {9999,1280,1080, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.40  BV=3.25
    {9999,1408,1056, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.50  BV=3.14
    {9999,1536,1032, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.60  BV=3.05
    {9999,1664,1024, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.70  BV=2.94
    {9999,1664,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.79  BV=2.86
    {9999,1792,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.89  BV=2.75
    {9999,1920,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.99  BV=2.65
    {9999,2176,1032, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=6.10  BV=2.55
    {19997,1152,1040, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.19  BV=2.45
    {19997,1152,1112, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.29  BV=2.36
    {19997,1280,1080, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.40  BV=2.25
    {19997,1408,1048, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.49  BV=2.15
    {19997,1536,1032, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.60  BV=2.05
    {19997,1664,1024, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.70  BV=1.94
    {29995,1152,1048, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.20  BV=1.86
    {29995,1152,1128, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.31  BV=1.75
    {29995,1280,1088, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.41  BV=1.65
    {29995,1408,1064, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.51  BV=1.54
    {29995,1536,1040, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.61  BV=1.45
    {29995,1664,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.71  BV=1.35
    {29995,1792,1024, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.81  BV=1.25
    {29995,1920,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.92  BV=1.14
    {29995,2048,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.01  BV=1.05
    {29995,2176,1040, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.11  BV=0.95
    {29995,2304,1056, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.21  BV=0.84
    {40008,1792,1088, 0, 0, 0},  //TV = 4.64(2697 lines)  AV=2.00  SV=5.89  BV=0.75
    {40008,1920,1088, 0, 0, 0},  //TV = 4.64(2697 lines)  AV=2.00  SV=5.99  BV=0.65
    {40008,2176,1024, 0, 0, 0},  //TV = 4.64(2697 lines)  AV=2.00  SV=6.09  BV=0.56
    {50006,1792,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.87  BV=0.45
    {50006,1920,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.97  BV=0.35
    {50006,2048,1080, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.08  BV=0.24
    {50006,2304,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.17  BV=0.15
    {50006,2432,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.27  BV=0.05
    {50006,2560,1064, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.38  BV=-0.06
    {50006,2816,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.47  BV=-0.15
    {50006,2944,1056, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.57  BV=-0.25
    {50006,3200,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.67  BV=-0.34
    {50006,3456,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.78  BV=-0.46
    {50006,3712,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.87  BV=-0.55
    {50006,3968,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.97  BV=-0.64
    {50006,4224,1048, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.08  BV=-0.76
    {60004,3840,1024, 0, 0, 0},  //TV = 4.06(4045 lines)  AV=2.00  SV=6.91  BV=-0.85
    {60004,4096,1032, 0, 0, 0},  //TV = 4.06(4045 lines)  AV=2.00  SV=7.01  BV=-0.95
    {70002,3712,1048, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=6.89  BV=-1.05
    {70002,3968,1048, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=6.99  BV=-1.15
    {70002,4352,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.09  BV=-1.25
    {70002,4608,1040, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.19  BV=-1.36
    {70002,4992,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.29  BV=-1.45
    {70002,5376,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.39  BV=-1.56
    {70002,5632,1040, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.48  BV=-1.65
    {70002,6144,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.58  BV=-1.75
    {70002,6528,1032, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.68  BV=-1.85
    {70002,7040,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.78  BV=-1.94
    {70002,7552,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.88  BV=-2.05
    {70002,8064,1032, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.99  BV=-2.15
    {70002,8704,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=8.09  BV=-2.25
    {80000,8064,1032, 0, 0, 0},  //TV = 3.64(5393 lines)  AV=2.00  SV=7.99  BV=-2.34
    {89998,7680,1032, 0, 0, 0},  //TV = 3.47(6067 lines)  AV=2.00  SV=7.92  BV=-2.44
    {89998,8320,1024, 0, 0, 0},  //TV = 3.47(6067 lines)  AV=2.00  SV=8.02  BV=-2.55
    {99996,7936,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=7.97  BV=-2.64
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
},
};

static strAETable g_AE_StrobeTable =
{
    AETABLE_STROBE, //eAETableID
    145, //u4TotalIndex
    -20, //i4StrobeTrigerBV
    118, //i4MaxBV
    -27, //i4MinBV
    90, //i4EffectiveMaxBV
    0, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_AUTO, //ISO
    sCaptureStrobePLineTable_60Hz,
    sCaptureStrobePLineTable_50Hz,
    NULL,
};
static strEvPline sAEScene1PLineTable_60Hz =
{
{
    {30,1152,1104, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.28  BV=11.75
    {30,1280,1056, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.37  BV=11.66
    {30,1408,1040, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.48  BV=11.54
    {30,1536,1032, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.60  BV=11.43
    {30,1664,1024, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.70  BV=11.32
    {45,1152,1048, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.20  BV=11.24
    {45,1152,1120, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.30  BV=11.14
    {45,1280,1088, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.41  BV=11.03
    {45,1408,1040, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.48  BV=10.96
    {60,1152,1056, 0, 0, 0},  /* TV = 14.02(4 lines)  AV=2.00  SV=5.21  BV=10.81 */
    {60,1152,1128, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.31  BV=10.72
    {60,1280,1072, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.39  BV=10.64
    {75,1152,1032, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.18  BV=10.52
    {75,1152,1088, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.26  BV=10.45
    {75,1280,1056, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.37  BV=10.34
    {90,1152,1040, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.19  BV=10.25
    {90,1280,1024, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.32  BV=10.12
    {104,1152,1040, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.19  BV=10.04
    {104,1152,1104, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.28  BV=9.95
    {119,1152,1032, 0, 0, 0},  //TV = 13.04(8 lines)  AV=2.00  SV=5.18  BV=9.86
    {134,1136,1024, 0, 0, 0},  /* TV = 12.87(9 lines)  AV=2.00  SV=5.15  BV=9.72 */
    {134,1152,1064, 0, 0, 0},  //TV = 12.87(9 lines)  AV=2.00  SV=5.23  BV=9.64
    {149,1152,1032, 0, 0, 0},  //TV = 12.71(10 lines)  AV=2.00  SV=5.18  BV=9.53
    {164,1136,1024, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.15  BV=9.42
    {164,1152,1088, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.26  BV=9.32
    {179,1152,1040, 0, 0, 0},  /* TV = 12.45(12 lines)  AV=2.00  SV=5.19  BV=9.26 */
    {193,1152,1032, 0, 0, 0},  //TV = 12.34(13 lines)  AV=2.00  SV=5.18  BV=9.16
    {208,1152,1032, 0, 0, 0},  //TV = 12.23(14 lines)  AV=2.00  SV=5.18  BV=9.05
    {223,1152,1032, 0, 0, 0},  //TV = 12.13(15 lines)  AV=2.00  SV=5.18  BV=8.95
    {238,1152,1032, 0, 0, 0},  //TV = 12.04(16 lines)  AV=2.00  SV=5.18  BV=8.86
    {253,1152,1040, 0, 0, 0},  //TV = 11.95(17 lines)  AV=2.00  SV=5.19  BV=8.76
    {268,1152,1056, 0, 0, 0},  //TV = 11.87(18 lines)  AV=2.00  SV=5.21  BV=8.65
    {297,1152,1024, 0, 0, 0},  //TV = 11.72(20 lines)  AV=2.00  SV=5.17  BV=8.55
    {312,1152,1040, 0, 0, 0},  //TV = 11.65(21 lines)  AV=2.00  SV=5.19  BV=8.45
    {342,1152,1024, 0, 0, 0},  //TV = 11.51(23 lines)  AV=2.00  SV=5.17  BV=8.34
    {371,1136,1024, 0, 0, 0},  //TV = 11.40(25 lines)  AV=2.00  SV=5.15  BV=8.25
    {386,1152,1032, 0, 0, 0},  //TV = 11.34(26 lines)  AV=2.00  SV=5.18  BV=8.16
    {416,1152,1032, 0, 0, 0},  //TV = 11.23(28 lines)  AV=2.00  SV=5.18  BV=8.05
    {446,1152,1032, 0, 0, 0},  //TV = 11.13(30 lines)  AV=2.00  SV=5.18  BV=7.95
    {475,1152,1032, 0, 0, 0},  //TV = 11.04(32 lines)  AV=2.00  SV=5.18  BV=7.86
    {520,1136,1024, 0, 0, 0},  //TV = 10.91(35 lines)  AV=2.00  SV=5.15  BV=7.76
    {564,1136,1024, 0, 0, 0},  //TV = 10.79(38 lines)  AV=2.00  SV=5.15  BV=7.64
    {594,1152,1024, 0, 0, 0},  //TV = 10.72(40 lines)  AV=2.00  SV=5.17  BV=7.55
    {638,1152,1024, 0, 0, 0},  //TV = 10.61(43 lines)  AV=2.00  SV=5.17  BV=7.44
    {683,1152,1024, 0, 0, 0},  //TV = 10.52(46 lines)  AV=2.00  SV=5.17  BV=7.35
    {727,1152,1024, 0, 0, 0},  //TV = 10.43(49 lines)  AV=2.00  SV=5.17  BV=7.26
    {787,1152,1024, 0, 0, 0},  //TV = 10.31(53 lines)  AV=2.00  SV=5.17  BV=7.14
    {846,1136,1024, 0, 0, 0},  //TV = 10.21(57 lines)  AV=2.00  SV=5.15  BV=7.06
    {905,1136,1024, 0, 0, 0},  //TV = 10.11(61 lines)  AV=2.00  SV=5.15  BV=6.96
    {965,1152,1024, 0, 0, 0},  //TV = 10.02(65 lines)  AV=2.00  SV=5.17  BV=6.85
    {1039,1152,1024, 0, 0, 0},  //TV = 9.91(70 lines)  AV=2.00  SV=5.17  BV=6.74
    {1128,1136,1024, 0, 0, 0},  //TV = 9.79(76 lines)  AV=2.00  SV=5.15  BV=6.64
    {1202,1136,1024, 0, 0, 0},  //TV = 9.70(81 lines)  AV=2.00  SV=5.15  BV=6.55
    {1291,1136,1024, 0, 0, 0},  //TV = 9.60(87 lines)  AV=2.00  SV=5.15  BV=6.45
    {1380,1136,1024, 0, 0, 0},  //TV = 9.50(93 lines)  AV=2.00  SV=5.15  BV=6.35
    {1484,1136,1024, 0, 0, 0},  //TV = 9.40(100 lines)  AV=2.00  SV=5.15  BV=6.25
    {1588,1136,1024, 0, 0, 0},  //TV = 9.30(107 lines)  AV=2.00  SV=5.15  BV=6.15
    {1692,1136,1024, 0, 0, 0},  //TV = 9.21(114 lines)  AV=2.00  SV=5.15  BV=6.06
    {1810,1136,1024, 0, 0, 0},  //TV = 9.11(122 lines)  AV=2.00  SV=5.15  BV=5.96
    {1959,1136,1024, 0, 0, 0},  //TV = 9.00(132 lines)  AV=2.00  SV=5.15  BV=5.85
    {2107,1136,1024, 0, 0, 0},  //TV = 8.89(142 lines)  AV=2.00  SV=5.15  BV=5.74
    {2255,1136,1024, 0, 0, 0},  //TV = 8.79(152 lines)  AV=2.00  SV=5.15  BV=5.64
    {2389,1136,1024, 0, 0, 0},  //TV = 8.71(161 lines)  AV=2.00  SV=5.15  BV=5.56
    {2582,1136,1024, 0, 0, 0},  //TV = 8.60(174 lines)  AV=2.00  SV=5.15  BV=5.45
    {2774,1136,1024, 0, 0, 0},  //TV = 8.49(187 lines)  AV=2.00  SV=5.15  BV=5.34
    {2967,1136,1024, 0, 0, 0},  //TV = 8.40(200 lines)  AV=2.00  SV=5.15  BV=5.25
    {3175,1136,1024, 0, 0, 0},  //TV = 8.30(214 lines)  AV=2.00  SV=5.15  BV=5.15
    {3397,1136,1024, 0, 0, 0},  //TV = 8.20(229 lines)  AV=2.00  SV=5.15  BV=5.05
    {3635,1136,1024, 0, 0, 0},  //TV = 8.10(245 lines)  AV=2.00  SV=5.15  BV=4.95
    {3932,1136,1024, 0, 0, 0},  //TV = 7.99(265 lines)  AV=2.00  SV=5.15  BV=4.84
    {4199,1136,1024, 0, 0, 0},  //TV = 7.90(283 lines)  AV=2.00  SV=5.15  BV=4.75
    {4495,1136,1024, 0, 0, 0},  //TV = 7.80(303 lines)  AV=2.00  SV=5.15  BV=4.65
    {4822,1136,1024, 0, 0, 0},  //TV = 7.70(325 lines)  AV=2.00  SV=5.15  BV=4.55
    {5163,1136,1024, 0, 0, 0},  //TV = 7.60(348 lines)  AV=2.00  SV=5.15  BV=4.45
    {5563,1136,1024, 0, 0, 0},  //TV = 7.49(375 lines)  AV=2.00  SV=5.15  BV=4.34
    {5949,1136,1024, 0, 0, 0},  //TV = 7.39(401 lines)  AV=2.00  SV=5.15  BV=4.24
    {6379,1136,1024, 0, 0, 0},  //TV = 7.29(430 lines)  AV=2.00  SV=5.15  BV=4.14
    {6824,1136,1024, 0, 0, 0},  //TV = 7.20(460 lines)  AV=2.00  SV=5.15  BV=4.05
    {7314,1136,1024, 0, 0, 0},  //TV = 7.10(493 lines)  AV=2.00  SV=5.15  BV=3.95
    {7833,1136,1024, 0, 0, 0},  //TV = 7.00(528 lines)  AV=2.00  SV=5.15  BV=3.85
    {8337,1136,1024, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.15  BV=3.76
    {8337,1152,1088, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.26  BV=3.65
    {8337,1280,1048, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.36  BV=3.55
    {8337,1408,1024, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.46  BV=3.45
    {8337,1408,1104, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.57  BV=3.34
    {8337,1536,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.65  BV=3.26
    {8337,1664,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.77  BV=3.14
    {8337,1792,1056, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.85  BV=3.05
    {8337,1920,1064, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.96  BV=2.94
    {8337,2048,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=6.07  BV=2.84
    {16659,1136,1024, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.15  BV=2.76
    {16659,1152,1088, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.26  BV=2.65
    {16659,1280,1056, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.37  BV=2.54
    {16659,1408,1024, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.46  BV=2.45
    {16659,1408,1096, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.56  BV=2.35
    {16659,1536,1080, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.66  BV=2.25
    {24996,1152,1024, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.17  BV=2.15
    {24996,1152,1096, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.27  BV=2.05
    {24996,1280,1056, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.37  BV=1.96
    {24996,1408,1032, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.47  BV=1.85
    {33332,1136,1024, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.15  BV=1.76
    {33332,1152,1088, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.26  BV=1.65
    {33332,1280,1048, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.36  BV=1.55
    {33332,1408,1024, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.46  BV=1.45
    {33332,1408,1096, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.56  BV=1.35
    {33332,1536,1072, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.65  BV=1.26
    {33332,1664,1072, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.77  BV=1.14
    {33332,1792,1064, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.86  BV=1.04
    {33332,1920,1056, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.95  BV=0.96
    {33332,2048,1064, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=6.06  BV=0.85
    {33332,2176,1072, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=6.15  BV=0.75
    {41669,1920,1040, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=5.93  BV=0.66
    {41669,2048,1048, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=6.03  BV=0.55
    {41669,2176,1056, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=6.13  BV=0.45
    {50006,1920,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.97  BV=0.35
    {50006,2048,1080, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.08  BV=0.24
    {50006,2304,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.17  BV=0.15
    {50006,2432,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.27  BV=0.05
    {50006,2560,1064, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.38  BV=-0.06
    {50006,2816,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.47  BV=-0.15
    {50006,2944,1056, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.57  BV=-0.25
    {50006,3200,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.67  BV=-0.34
    {50006,3456,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.78  BV=-0.46
    {50006,3712,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.87  BV=-0.55
    {50006,3968,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.97  BV=-0.64
    {50006,4224,1048, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.08  BV=-0.76
    {58328,3968,1024, 0, 0, 0},  //TV = 4.10(3932 lines)  AV=2.00  SV=6.95  BV=-0.85
    {58328,4224,1032, 0, 0, 0},  //TV = 4.10(3932 lines)  AV=2.00  SV=7.06  BV=-0.96
    {66664,3968,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=6.97  BV=-1.06
    {66664,4224,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.06  BV=-1.15
    {66664,4480,1040, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.15  BV=-1.24
    {66664,4864,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.26  BV=-1.35
    {66664,5248,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.36  BV=-1.45
    {66664,5632,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.46  BV=-1.55
    {66664,6016,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.55  BV=-1.65
    {66664,6400,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.66  BV=-1.75
    {66664,6912,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.75  BV=-1.85
    {66664,7424,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.86  BV=-1.95
    {66664,7936,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.95  BV=-2.05
    {66664,8448,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=8.06  BV=-2.15
    {75001,8064,1032, 0, 0, 0},  //TV = 3.74(5056 lines)  AV=2.00  SV=7.99  BV=-2.25
    {83338,7808,1024, 0, 0, 0},  //TV = 3.58(5618 lines)  AV=2.00  SV=7.93  BV=-2.35
    {83338,8320,1032, 0, 0, 0},  //TV = 3.58(5618 lines)  AV=2.00  SV=8.03  BV=-2.45
    {91660,8192,1024, 0, 0, 0},  //TV = 3.45(6179 lines)  AV=2.00  SV=8.00  BV=-2.55
    {99996,7936,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=7.97  BV=-2.64
    {99996,8576,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.08  BV=-2.76
    {99996,9216,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.17  BV=-2.85
    {99996,9856,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.27  BV=-2.94
    {99996,10624,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.38  BV=-3.05
    {99996,11392,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.48  BV=-3.15
    {99996,12160,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.57  BV=-3.25
    {99996,13056,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.67  BV=-3.35
    {99996,13952,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.77  BV=-3.45
    {99996,14976,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.87  BV=-3.55
    {99996,16000,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.97  BV=-3.64
    {99996,16384,1024, 0, 0, 0},  /* TV = 3.32(6741 lines)  AV=2.00  SV=9.00  BV=-3.68 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
},
};

static strEvPline sAEScene1PLineTable_50Hz =
{
{
    {30,1152,1104, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.28  BV=11.75
    {30,1280,1056, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.37  BV=11.66
    {30,1408,1040, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.48  BV=11.54
    {30,1536,1032, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.60  BV=11.43
    {30,1664,1024, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.70  BV=11.32
    {45,1152,1048, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.20  BV=11.24
    {45,1152,1120, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.30  BV=11.14
    {45,1280,1088, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.41  BV=11.03
    {45,1408,1040, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.48  BV=10.96
    {60,1152,1056, 0, 0, 0},  /* TV = 14.02(4 lines)  AV=2.00  SV=5.21  BV=10.81 */
    {60,1152,1128, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.31  BV=10.72
    {60,1280,1072, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.39  BV=10.64
    {75,1152,1032, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.18  BV=10.52
    {75,1152,1088, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.26  BV=10.45
    {75,1280,1056, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.37  BV=10.34
    {90,1152,1040, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.19  BV=10.25
    {90,1280,1024, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.32  BV=10.12
    {104,1152,1040, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.19  BV=10.04
    {104,1152,1104, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.28  BV=9.95
    {119,1152,1032, 0, 0, 0},  //TV = 13.04(8 lines)  AV=2.00  SV=5.18  BV=9.86
    {134,1136,1024, 0, 0, 0},  /* TV = 12.87(9 lines)  AV=2.00  SV=5.15  BV=9.72 */
    {134,1152,1064, 0, 0, 0},  //TV = 12.87(9 lines)  AV=2.00  SV=5.23  BV=9.64
    {149,1152,1032, 0, 0, 0},  //TV = 12.71(10 lines)  AV=2.00  SV=5.18  BV=9.53
    {164,1136,1024, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.15  BV=9.42
    {164,1152,1088, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.26  BV=9.32
    {179,1152,1040, 0, 0, 0},  /* TV = 12.45(12 lines)  AV=2.00  SV=5.19  BV=9.26 */
    {193,1152,1032, 0, 0, 0},  //TV = 12.34(13 lines)  AV=2.00  SV=5.18  BV=9.16
    {208,1152,1032, 0, 0, 0},  //TV = 12.23(14 lines)  AV=2.00  SV=5.18  BV=9.05
    {223,1152,1032, 0, 0, 0},  //TV = 12.13(15 lines)  AV=2.00  SV=5.18  BV=8.95
    {238,1152,1032, 0, 0, 0},  //TV = 12.04(16 lines)  AV=2.00  SV=5.18  BV=8.86
    {253,1152,1040, 0, 0, 0},  //TV = 11.95(17 lines)  AV=2.00  SV=5.19  BV=8.76
    {268,1152,1056, 0, 0, 0},  //TV = 11.87(18 lines)  AV=2.00  SV=5.21  BV=8.65
    {297,1152,1024, 0, 0, 0},  //TV = 11.72(20 lines)  AV=2.00  SV=5.17  BV=8.55
    {312,1152,1040, 0, 0, 0},  //TV = 11.65(21 lines)  AV=2.00  SV=5.19  BV=8.45
    {342,1152,1024, 0, 0, 0},  //TV = 11.51(23 lines)  AV=2.00  SV=5.17  BV=8.34
    {371,1136,1024, 0, 0, 0},  //TV = 11.40(25 lines)  AV=2.00  SV=5.15  BV=8.25
    {386,1152,1032, 0, 0, 0},  //TV = 11.34(26 lines)  AV=2.00  SV=5.18  BV=8.16
    {416,1152,1032, 0, 0, 0},  //TV = 11.23(28 lines)  AV=2.00  SV=5.18  BV=8.05
    {446,1152,1032, 0, 0, 0},  //TV = 11.13(30 lines)  AV=2.00  SV=5.18  BV=7.95
    {475,1152,1032, 0, 0, 0},  //TV = 11.04(32 lines)  AV=2.00  SV=5.18  BV=7.86
    {520,1136,1024, 0, 0, 0},  //TV = 10.91(35 lines)  AV=2.00  SV=5.15  BV=7.76
    {564,1136,1024, 0, 0, 0},  //TV = 10.79(38 lines)  AV=2.00  SV=5.15  BV=7.64
    {594,1152,1024, 0, 0, 0},  //TV = 10.72(40 lines)  AV=2.00  SV=5.17  BV=7.55
    {638,1152,1024, 0, 0, 0},  //TV = 10.61(43 lines)  AV=2.00  SV=5.17  BV=7.44
    {683,1152,1024, 0, 0, 0},  //TV = 10.52(46 lines)  AV=2.00  SV=5.17  BV=7.35
    {727,1152,1024, 0, 0, 0},  //TV = 10.43(49 lines)  AV=2.00  SV=5.17  BV=7.26
    {787,1152,1024, 0, 0, 0},  //TV = 10.31(53 lines)  AV=2.00  SV=5.17  BV=7.14
    {846,1136,1024, 0, 0, 0},  //TV = 10.21(57 lines)  AV=2.00  SV=5.15  BV=7.06
    {905,1136,1024, 0, 0, 0},  //TV = 10.11(61 lines)  AV=2.00  SV=5.15  BV=6.96
    {965,1152,1024, 0, 0, 0},  //TV = 10.02(65 lines)  AV=2.00  SV=5.17  BV=6.85
    {1039,1152,1024, 0, 0, 0},  //TV = 9.91(70 lines)  AV=2.00  SV=5.17  BV=6.74
    {1128,1136,1024, 0, 0, 0},  //TV = 9.79(76 lines)  AV=2.00  SV=5.15  BV=6.64
    {1202,1136,1024, 0, 0, 0},  //TV = 9.70(81 lines)  AV=2.00  SV=5.15  BV=6.55
    {1291,1136,1024, 0, 0, 0},  //TV = 9.60(87 lines)  AV=2.00  SV=5.15  BV=6.45
    {1380,1136,1024, 0, 0, 0},  //TV = 9.50(93 lines)  AV=2.00  SV=5.15  BV=6.35
    {1484,1136,1024, 0, 0, 0},  //TV = 9.40(100 lines)  AV=2.00  SV=5.15  BV=6.25
    {1588,1136,1024, 0, 0, 0},  //TV = 9.30(107 lines)  AV=2.00  SV=5.15  BV=6.15
    {1692,1136,1024, 0, 0, 0},  //TV = 9.21(114 lines)  AV=2.00  SV=5.15  BV=6.06
    {1810,1136,1024, 0, 0, 0},  //TV = 9.11(122 lines)  AV=2.00  SV=5.15  BV=5.96
    {1959,1136,1024, 0, 0, 0},  //TV = 9.00(132 lines)  AV=2.00  SV=5.15  BV=5.85
    {2107,1136,1024, 0, 0, 0},  //TV = 8.89(142 lines)  AV=2.00  SV=5.15  BV=5.74
    {2255,1136,1024, 0, 0, 0},  //TV = 8.79(152 lines)  AV=2.00  SV=5.15  BV=5.64
    {2389,1136,1024, 0, 0, 0},  //TV = 8.71(161 lines)  AV=2.00  SV=5.15  BV=5.56
    {2582,1136,1024, 0, 0, 0},  //TV = 8.60(174 lines)  AV=2.00  SV=5.15  BV=5.45
    {2774,1136,1024, 0, 0, 0},  //TV = 8.49(187 lines)  AV=2.00  SV=5.15  BV=5.34
    {2967,1136,1024, 0, 0, 0},  //TV = 8.40(200 lines)  AV=2.00  SV=5.15  BV=5.25
    {3175,1136,1024, 0, 0, 0},  //TV = 8.30(214 lines)  AV=2.00  SV=5.15  BV=5.15
    {3397,1136,1024, 0, 0, 0},  //TV = 8.20(229 lines)  AV=2.00  SV=5.15  BV=5.05
    {3635,1136,1024, 0, 0, 0},  //TV = 8.10(245 lines)  AV=2.00  SV=5.15  BV=4.95
    {3932,1136,1024, 0, 0, 0},  //TV = 7.99(265 lines)  AV=2.00  SV=5.15  BV=4.84
    {4199,1136,1024, 0, 0, 0},  //TV = 7.90(283 lines)  AV=2.00  SV=5.15  BV=4.75
    {4495,1136,1024, 0, 0, 0},  //TV = 7.80(303 lines)  AV=2.00  SV=5.15  BV=4.65
    {4822,1136,1024, 0, 0, 0},  //TV = 7.70(325 lines)  AV=2.00  SV=5.15  BV=4.55
    {5163,1136,1024, 0, 0, 0},  //TV = 7.60(348 lines)  AV=2.00  SV=5.15  BV=4.45
    {5563,1136,1024, 0, 0, 0},  //TV = 7.49(375 lines)  AV=2.00  SV=5.15  BV=4.34
    {5949,1136,1024, 0, 0, 0},  //TV = 7.39(401 lines)  AV=2.00  SV=5.15  BV=4.24
    {6379,1136,1024, 0, 0, 0},  //TV = 7.29(430 lines)  AV=2.00  SV=5.15  BV=4.14
    {6824,1136,1024, 0, 0, 0},  //TV = 7.20(460 lines)  AV=2.00  SV=5.15  BV=4.05
    {7314,1136,1024, 0, 0, 0},  //TV = 7.10(493 lines)  AV=2.00  SV=5.15  BV=3.95
    {7833,1136,1024, 0, 0, 0},  //TV = 7.00(528 lines)  AV=2.00  SV=5.15  BV=3.85
    {8397,1136,1024, 0, 0, 0},  //TV = 6.90(566 lines)  AV=2.00  SV=5.15  BV=3.75
    {8990,1136,1024, 0, 0, 0},  //TV = 6.80(606 lines)  AV=2.00  SV=5.15  BV=3.65
    {9628,1136,1024, 0, 0, 0},  //TV = 6.70(649 lines)  AV=2.00  SV=5.15  BV=3.55
    {9999,1152,1040, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.19  BV=3.45
    {9999,1152,1112, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.29  BV=3.36
    {9999,1280,1080, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.40  BV=3.25
    {9999,1408,1056, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.50  BV=3.14
    {9999,1536,1032, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.60  BV=3.05
    {9999,1664,1024, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.70  BV=2.94
    {9999,1664,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.79  BV=2.86
    {9999,1792,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.89  BV=2.75
    {9999,1920,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.99  BV=2.65
    {9999,2176,1032, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=6.10  BV=2.55
    {19997,1152,1040, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.19  BV=2.45
    {19997,1152,1112, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.29  BV=2.36
    {19997,1280,1080, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.40  BV=2.25
    {19997,1408,1048, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.49  BV=2.15
    {19997,1536,1032, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.60  BV=2.05
    {19997,1664,1024, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.70  BV=1.94
    {29995,1152,1048, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.20  BV=1.86
    {29995,1152,1128, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.31  BV=1.75
    {29995,1280,1088, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.41  BV=1.65
    {29995,1408,1064, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.51  BV=1.54
    {29995,1536,1040, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.61  BV=1.45
    {29995,1664,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.71  BV=1.35
    {29995,1792,1024, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.81  BV=1.25
    {29995,1920,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.92  BV=1.14
    {29995,2048,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.01  BV=1.05
    {29995,2176,1040, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.11  BV=0.95
    {29995,2304,1056, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.21  BV=0.84
    {40008,1792,1088, 0, 0, 0},  //TV = 4.64(2697 lines)  AV=2.00  SV=5.89  BV=0.75
    {40008,1920,1088, 0, 0, 0},  //TV = 4.64(2697 lines)  AV=2.00  SV=5.99  BV=0.65
    {40008,2176,1024, 0, 0, 0},  //TV = 4.64(2697 lines)  AV=2.00  SV=6.09  BV=0.56
    {50006,1792,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.87  BV=0.45
    {50006,1920,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.97  BV=0.35
    {50006,2048,1080, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.08  BV=0.24
    {50006,2304,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.17  BV=0.15
    {50006,2432,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.27  BV=0.05
    {50006,2560,1064, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.38  BV=-0.06
    {50006,2816,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.47  BV=-0.15
    {50006,2944,1056, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.57  BV=-0.25
    {50006,3200,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.67  BV=-0.34
    {50006,3456,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.78  BV=-0.46
    {50006,3712,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.87  BV=-0.55
    {50006,3968,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.97  BV=-0.64
    {50006,4224,1048, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.08  BV=-0.76
    {60004,3840,1024, 0, 0, 0},  //TV = 4.06(4045 lines)  AV=2.00  SV=6.91  BV=-0.85
    {60004,4096,1032, 0, 0, 0},  //TV = 4.06(4045 lines)  AV=2.00  SV=7.01  BV=-0.95
    {70002,3712,1048, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=6.89  BV=-1.05
    {70002,3968,1048, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=6.99  BV=-1.15
    {70002,4352,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.09  BV=-1.25
    {70002,4608,1040, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.19  BV=-1.36
    {70002,4992,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.29  BV=-1.45
    {70002,5376,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.39  BV=-1.56
    {70002,5632,1040, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.48  BV=-1.65
    {70002,6144,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.58  BV=-1.75
    {70002,6528,1032, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.68  BV=-1.85
    {70002,7040,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.78  BV=-1.94
    {70002,7552,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.88  BV=-2.05
    {70002,8064,1032, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.99  BV=-2.15
    {70002,8704,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=8.09  BV=-2.25
    {80000,8064,1032, 0, 0, 0},  //TV = 3.64(5393 lines)  AV=2.00  SV=7.99  BV=-2.34
    {89998,7680,1032, 0, 0, 0},  //TV = 3.47(6067 lines)  AV=2.00  SV=7.92  BV=-2.44
    {89998,8320,1024, 0, 0, 0},  //TV = 3.47(6067 lines)  AV=2.00  SV=8.02  BV=-2.55
    {99996,7936,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=7.97  BV=-2.64
    {99996,8576,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.08  BV=-2.76
    {99996,9216,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.17  BV=-2.85
    {99996,9856,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.27  BV=-2.94
    {99996,10624,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.38  BV=-3.05
    {99996,11392,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.48  BV=-3.15
    {99996,12160,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.57  BV=-3.25
    {99996,13056,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.67  BV=-3.35
    {99996,13952,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.77  BV=-3.45
    {99996,14976,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.87  BV=-3.55
    {99996,16000,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.97  BV=-3.64
    {99996,16384,1024, 0, 0, 0},  /* TV = 3.32(6741 lines)  AV=2.00  SV=9.00  BV=-3.68 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
},
};

static strAETable g_AE_SceneTable1 =
{
    AETABLE_SCENE_INDEX1, //eAETableID
    156, //u4TotalIndex
    -20, //i4StrobeTrigerBV
    118, //i4MaxBV
    -37, //i4MinBV
    90, //i4EffectiveMaxBV
    -10, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_AUTO, //ISO
    sAEScene1PLineTable_60Hz,
    sAEScene1PLineTable_50Hz,
    NULL,
};
static strEvPline sAEScene2PLineTable_60Hz =
{
{
    {30,1152,1104, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.28  BV=11.75
    {30,1280,1056, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.37  BV=11.66
    {30,1408,1040, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.48  BV=11.54
    {30,1536,1032, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.60  BV=11.43
    {30,1664,1024, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.70  BV=11.32
    {45,1152,1048, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.20  BV=11.24
    {45,1152,1120, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.30  BV=11.14
    {45,1280,1088, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.41  BV=11.03
    {45,1408,1040, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.48  BV=10.96
    {60,1152,1056, 0, 0, 0},  /* TV = 14.02(4 lines)  AV=2.00  SV=5.21  BV=10.81 */
    {60,1152,1128, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.31  BV=10.72
    {60,1280,1072, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.39  BV=10.64
    {75,1152,1032, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.18  BV=10.52
    {75,1152,1088, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.26  BV=10.45
    {75,1280,1056, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.37  BV=10.34
    {90,1152,1040, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.19  BV=10.25
    {90,1280,1024, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.32  BV=10.12
    {104,1152,1040, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.19  BV=10.04
    {104,1152,1104, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.28  BV=9.95
    {119,1152,1032, 0, 0, 0},  //TV = 13.04(8 lines)  AV=2.00  SV=5.18  BV=9.86
    {134,1136,1024, 0, 0, 0},  /* TV = 12.87(9 lines)  AV=2.00  SV=5.15  BV=9.72 */
    {134,1152,1064, 0, 0, 0},  //TV = 12.87(9 lines)  AV=2.00  SV=5.23  BV=9.64
    {149,1152,1032, 0, 0, 0},  //TV = 12.71(10 lines)  AV=2.00  SV=5.18  BV=9.53
    {164,1136,1024, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.15  BV=9.42
    {164,1152,1088, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.26  BV=9.32
    {179,1152,1040, 0, 0, 0},  /* TV = 12.45(12 lines)  AV=2.00  SV=5.19  BV=9.26 */
    {193,1152,1032, 0, 0, 0},  //TV = 12.34(13 lines)  AV=2.00  SV=5.18  BV=9.16
    {208,1152,1032, 0, 0, 0},  //TV = 12.23(14 lines)  AV=2.00  SV=5.18  BV=9.05
    {223,1152,1032, 0, 0, 0},  //TV = 12.13(15 lines)  AV=2.00  SV=5.18  BV=8.95
    {238,1152,1032, 0, 0, 0},  //TV = 12.04(16 lines)  AV=2.00  SV=5.18  BV=8.86
    {253,1152,1040, 0, 0, 0},  //TV = 11.95(17 lines)  AV=2.00  SV=5.19  BV=8.76
    {268,1152,1056, 0, 0, 0},  //TV = 11.87(18 lines)  AV=2.00  SV=5.21  BV=8.65
    {297,1152,1024, 0, 0, 0},  //TV = 11.72(20 lines)  AV=2.00  SV=5.17  BV=8.55
    {312,1152,1040, 0, 0, 0},  //TV = 11.65(21 lines)  AV=2.00  SV=5.19  BV=8.45
    {342,1152,1024, 0, 0, 0},  //TV = 11.51(23 lines)  AV=2.00  SV=5.17  BV=8.34
    {371,1136,1024, 0, 0, 0},  //TV = 11.40(25 lines)  AV=2.00  SV=5.15  BV=8.25
    {386,1152,1032, 0, 0, 0},  //TV = 11.34(26 lines)  AV=2.00  SV=5.18  BV=8.16
    {416,1152,1032, 0, 0, 0},  //TV = 11.23(28 lines)  AV=2.00  SV=5.18  BV=8.05
    {446,1152,1032, 0, 0, 0},  //TV = 11.13(30 lines)  AV=2.00  SV=5.18  BV=7.95
    {475,1152,1032, 0, 0, 0},  //TV = 11.04(32 lines)  AV=2.00  SV=5.18  BV=7.86
    {520,1136,1024, 0, 0, 0},  //TV = 10.91(35 lines)  AV=2.00  SV=5.15  BV=7.76
    {564,1136,1024, 0, 0, 0},  //TV = 10.79(38 lines)  AV=2.00  SV=5.15  BV=7.64
    {594,1152,1024, 0, 0, 0},  //TV = 10.72(40 lines)  AV=2.00  SV=5.17  BV=7.55
    {638,1152,1024, 0, 0, 0},  //TV = 10.61(43 lines)  AV=2.00  SV=5.17  BV=7.44
    {683,1152,1024, 0, 0, 0},  //TV = 10.52(46 lines)  AV=2.00  SV=5.17  BV=7.35
    {727,1152,1024, 0, 0, 0},  //TV = 10.43(49 lines)  AV=2.00  SV=5.17  BV=7.26
    {787,1152,1024, 0, 0, 0},  //TV = 10.31(53 lines)  AV=2.00  SV=5.17  BV=7.14
    {846,1136,1024, 0, 0, 0},  //TV = 10.21(57 lines)  AV=2.00  SV=5.15  BV=7.06
    {905,1136,1024, 0, 0, 0},  //TV = 10.11(61 lines)  AV=2.00  SV=5.15  BV=6.96
    {965,1152,1024, 0, 0, 0},  //TV = 10.02(65 lines)  AV=2.00  SV=5.17  BV=6.85
    {1039,1152,1024, 0, 0, 0},  //TV = 9.91(70 lines)  AV=2.00  SV=5.17  BV=6.74
    {1128,1136,1024, 0, 0, 0},  //TV = 9.79(76 lines)  AV=2.00  SV=5.15  BV=6.64
    {1202,1136,1024, 0, 0, 0},  //TV = 9.70(81 lines)  AV=2.00  SV=5.15  BV=6.55
    {1291,1136,1024, 0, 0, 0},  //TV = 9.60(87 lines)  AV=2.00  SV=5.15  BV=6.45
    {1380,1136,1024, 0, 0, 0},  //TV = 9.50(93 lines)  AV=2.00  SV=5.15  BV=6.35
    {1484,1136,1024, 0, 0, 0},  //TV = 9.40(100 lines)  AV=2.00  SV=5.15  BV=6.25
    {1588,1136,1024, 0, 0, 0},  //TV = 9.30(107 lines)  AV=2.00  SV=5.15  BV=6.15
    {1692,1136,1024, 0, 0, 0},  //TV = 9.21(114 lines)  AV=2.00  SV=5.15  BV=6.06
    {1810,1136,1024, 0, 0, 0},  //TV = 9.11(122 lines)  AV=2.00  SV=5.15  BV=5.96
    {1959,1136,1024, 0, 0, 0},  //TV = 9.00(132 lines)  AV=2.00  SV=5.15  BV=5.85
    {2107,1136,1024, 0, 0, 0},  //TV = 8.89(142 lines)  AV=2.00  SV=5.15  BV=5.74
    {2255,1136,1024, 0, 0, 0},  //TV = 8.79(152 lines)  AV=2.00  SV=5.15  BV=5.64
    {2389,1136,1024, 0, 0, 0},  //TV = 8.71(161 lines)  AV=2.00  SV=5.15  BV=5.56
    {2582,1136,1024, 0, 0, 0},  //TV = 8.60(174 lines)  AV=2.00  SV=5.15  BV=5.45
    {2774,1136,1024, 0, 0, 0},  //TV = 8.49(187 lines)  AV=2.00  SV=5.15  BV=5.34
    {2967,1136,1024, 0, 0, 0},  //TV = 8.40(200 lines)  AV=2.00  SV=5.15  BV=5.25
    {3175,1136,1024, 0, 0, 0},  //TV = 8.30(214 lines)  AV=2.00  SV=5.15  BV=5.15
    {3397,1136,1024, 0, 0, 0},  //TV = 8.20(229 lines)  AV=2.00  SV=5.15  BV=5.05
    {3635,1136,1024, 0, 0, 0},  //TV = 8.10(245 lines)  AV=2.00  SV=5.15  BV=4.95
    {3932,1136,1024, 0, 0, 0},  //TV = 7.99(265 lines)  AV=2.00  SV=5.15  BV=4.84
    {4199,1136,1024, 0, 0, 0},  //TV = 7.90(283 lines)  AV=2.00  SV=5.15  BV=4.75
    {4495,1136,1024, 0, 0, 0},  //TV = 7.80(303 lines)  AV=2.00  SV=5.15  BV=4.65
    {4822,1136,1024, 0, 0, 0},  //TV = 7.70(325 lines)  AV=2.00  SV=5.15  BV=4.55
    {5163,1136,1024, 0, 0, 0},  //TV = 7.60(348 lines)  AV=2.00  SV=5.15  BV=4.45
    {5563,1136,1024, 0, 0, 0},  //TV = 7.49(375 lines)  AV=2.00  SV=5.15  BV=4.34
    {5949,1136,1024, 0, 0, 0},  //TV = 7.39(401 lines)  AV=2.00  SV=5.15  BV=4.24
    {6379,1136,1024, 0, 0, 0},  //TV = 7.29(430 lines)  AV=2.00  SV=5.15  BV=4.14
    {6824,1136,1024, 0, 0, 0},  //TV = 7.20(460 lines)  AV=2.00  SV=5.15  BV=4.05
    {7314,1136,1024, 0, 0, 0},  //TV = 7.10(493 lines)  AV=2.00  SV=5.15  BV=3.95
    {7833,1136,1024, 0, 0, 0},  //TV = 7.00(528 lines)  AV=2.00  SV=5.15  BV=3.85
    {8337,1136,1024, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.15  BV=3.76
    {8337,1152,1088, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.26  BV=3.65
    {8337,1280,1048, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.36  BV=3.55
    {8337,1408,1024, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.46  BV=3.45
    {8337,1408,1104, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.57  BV=3.34
    {8337,1536,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.65  BV=3.26
    {8337,1664,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.77  BV=3.14
    {8337,1792,1056, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.85  BV=3.05
    {8337,1920,1064, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.96  BV=2.94
    {8337,2048,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=6.07  BV=2.84
    {16659,1136,1024, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.15  BV=2.76
    {16659,1152,1088, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.26  BV=2.65
    {16659,1280,1056, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.37  BV=2.54
    {16659,1408,1024, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.46  BV=2.45
    {16659,1408,1096, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.56  BV=2.35
    {16659,1536,1080, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.66  BV=2.25
    {24996,1152,1024, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.17  BV=2.15
    {24996,1152,1096, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.27  BV=2.05
    {24996,1280,1056, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.37  BV=1.96
    {24996,1408,1032, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.47  BV=1.85
    {33332,1136,1024, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.15  BV=1.76
    {33332,1152,1088, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.26  BV=1.65
    {33332,1280,1048, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.36  BV=1.55
    {33332,1408,1024, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.46  BV=1.45
    {33332,1408,1096, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.56  BV=1.35
    {33332,1536,1072, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.65  BV=1.26
    {33332,1664,1072, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.77  BV=1.14
    {33332,1792,1064, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.86  BV=1.04
    {33332,1920,1056, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.95  BV=0.96
    {33332,2048,1064, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=6.06  BV=0.85
    {33332,2176,1072, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=6.15  BV=0.75
    {41669,1920,1040, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=5.93  BV=0.66
    {41669,2048,1048, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=6.03  BV=0.55
    {41669,2176,1056, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=6.13  BV=0.45
    {50006,1920,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.97  BV=0.35
    {50006,2048,1080, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.08  BV=0.24
    {50006,2304,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.17  BV=0.15
    {50006,2432,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.27  BV=0.05
    {50006,2560,1064, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.38  BV=-0.06
    {50006,2816,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.47  BV=-0.15
    {50006,2944,1056, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.57  BV=-0.25
    {50006,3200,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.67  BV=-0.34
    {50006,3456,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.78  BV=-0.46
    {50006,3712,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.87  BV=-0.55
    {50006,3968,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.97  BV=-0.64
    {50006,4224,1048, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.08  BV=-0.76
    {58328,3968,1024, 0, 0, 0},  //TV = 4.10(3932 lines)  AV=2.00  SV=6.95  BV=-0.85
    {58328,4224,1032, 0, 0, 0},  //TV = 4.10(3932 lines)  AV=2.00  SV=7.06  BV=-0.96
    {66664,3968,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=6.97  BV=-1.06
    {66664,4224,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.06  BV=-1.15
    {66664,4480,1040, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.15  BV=-1.24
    {66664,4864,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.26  BV=-1.35
    {66664,5248,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.36  BV=-1.45
    {66664,5632,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.46  BV=-1.55
    {66664,6016,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.55  BV=-1.65
    {66664,6400,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.66  BV=-1.75
    {66664,6912,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.75  BV=-1.85
    {66664,7424,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.86  BV=-1.95
    {66664,7936,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.95  BV=-2.05
    {66664,8448,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=8.06  BV=-2.15
    {75001,8064,1032, 0, 0, 0},  //TV = 3.74(5056 lines)  AV=2.00  SV=7.99  BV=-2.25
    {83338,7808,1024, 0, 0, 0},  //TV = 3.58(5618 lines)  AV=2.00  SV=7.93  BV=-2.35
    {83338,8320,1032, 0, 0, 0},  //TV = 3.58(5618 lines)  AV=2.00  SV=8.03  BV=-2.45
    {91660,8192,1024, 0, 0, 0},  //TV = 3.45(6179 lines)  AV=2.00  SV=8.00  BV=-2.55
    {99996,7936,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=7.97  BV=-2.64
    {99996,8576,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.08  BV=-2.76
    {99996,9216,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.17  BV=-2.85
    {99996,9856,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.27  BV=-2.94
    {99996,10624,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.38  BV=-3.05
    {99996,11392,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.48  BV=-3.15
    {99996,12160,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.57  BV=-3.25
    {99996,13056,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.67  BV=-3.35
    {99996,13952,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.77  BV=-3.45
    {99996,14976,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.87  BV=-3.55
    {99996,16000,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.97  BV=-3.64
    {99996,16384,1024, 0, 0, 0},  /* TV = 3.32(6741 lines)  AV=2.00  SV=9.00  BV=-3.68 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
},
};

static strEvPline sAEScene2PLineTable_50Hz =
{
{
    {30,1152,1104, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.28  BV=11.75
    {30,1280,1056, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.37  BV=11.66
    {30,1408,1040, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.48  BV=11.54
    {30,1536,1032, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.60  BV=11.43
    {30,1664,1024, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.70  BV=11.32
    {45,1152,1048, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.20  BV=11.24
    {45,1152,1120, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.30  BV=11.14
    {45,1280,1088, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.41  BV=11.03
    {45,1408,1040, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.48  BV=10.96
    {60,1152,1056, 0, 0, 0},  /* TV = 14.02(4 lines)  AV=2.00  SV=5.21  BV=10.81 */
    {60,1152,1128, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.31  BV=10.72
    {60,1280,1072, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.39  BV=10.64
    {75,1152,1032, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.18  BV=10.52
    {75,1152,1088, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.26  BV=10.45
    {75,1280,1056, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.37  BV=10.34
    {90,1152,1040, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.19  BV=10.25
    {90,1280,1024, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.32  BV=10.12
    {104,1152,1040, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.19  BV=10.04
    {104,1152,1104, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.28  BV=9.95
    {119,1152,1032, 0, 0, 0},  //TV = 13.04(8 lines)  AV=2.00  SV=5.18  BV=9.86
    {134,1136,1024, 0, 0, 0},  /* TV = 12.87(9 lines)  AV=2.00  SV=5.15  BV=9.72 */
    {134,1152,1064, 0, 0, 0},  //TV = 12.87(9 lines)  AV=2.00  SV=5.23  BV=9.64
    {149,1152,1032, 0, 0, 0},  //TV = 12.71(10 lines)  AV=2.00  SV=5.18  BV=9.53
    {164,1136,1024, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.15  BV=9.42
    {164,1152,1088, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.26  BV=9.32
    {179,1152,1040, 0, 0, 0},  /* TV = 12.45(12 lines)  AV=2.00  SV=5.19  BV=9.26 */
    {193,1152,1032, 0, 0, 0},  //TV = 12.34(13 lines)  AV=2.00  SV=5.18  BV=9.16
    {208,1152,1032, 0, 0, 0},  //TV = 12.23(14 lines)  AV=2.00  SV=5.18  BV=9.05
    {223,1152,1032, 0, 0, 0},  //TV = 12.13(15 lines)  AV=2.00  SV=5.18  BV=8.95
    {238,1152,1032, 0, 0, 0},  //TV = 12.04(16 lines)  AV=2.00  SV=5.18  BV=8.86
    {253,1152,1040, 0, 0, 0},  //TV = 11.95(17 lines)  AV=2.00  SV=5.19  BV=8.76
    {268,1152,1056, 0, 0, 0},  //TV = 11.87(18 lines)  AV=2.00  SV=5.21  BV=8.65
    {297,1152,1024, 0, 0, 0},  //TV = 11.72(20 lines)  AV=2.00  SV=5.17  BV=8.55
    {312,1152,1040, 0, 0, 0},  //TV = 11.65(21 lines)  AV=2.00  SV=5.19  BV=8.45
    {342,1152,1024, 0, 0, 0},  //TV = 11.51(23 lines)  AV=2.00  SV=5.17  BV=8.34
    {371,1136,1024, 0, 0, 0},  //TV = 11.40(25 lines)  AV=2.00  SV=5.15  BV=8.25
    {386,1152,1032, 0, 0, 0},  //TV = 11.34(26 lines)  AV=2.00  SV=5.18  BV=8.16
    {416,1152,1032, 0, 0, 0},  //TV = 11.23(28 lines)  AV=2.00  SV=5.18  BV=8.05
    {446,1152,1032, 0, 0, 0},  //TV = 11.13(30 lines)  AV=2.00  SV=5.18  BV=7.95
    {475,1152,1032, 0, 0, 0},  //TV = 11.04(32 lines)  AV=2.00  SV=5.18  BV=7.86
    {520,1136,1024, 0, 0, 0},  //TV = 10.91(35 lines)  AV=2.00  SV=5.15  BV=7.76
    {564,1136,1024, 0, 0, 0},  //TV = 10.79(38 lines)  AV=2.00  SV=5.15  BV=7.64
    {594,1152,1024, 0, 0, 0},  //TV = 10.72(40 lines)  AV=2.00  SV=5.17  BV=7.55
    {638,1152,1024, 0, 0, 0},  //TV = 10.61(43 lines)  AV=2.00  SV=5.17  BV=7.44
    {683,1152,1024, 0, 0, 0},  //TV = 10.52(46 lines)  AV=2.00  SV=5.17  BV=7.35
    {727,1152,1024, 0, 0, 0},  //TV = 10.43(49 lines)  AV=2.00  SV=5.17  BV=7.26
    {787,1152,1024, 0, 0, 0},  //TV = 10.31(53 lines)  AV=2.00  SV=5.17  BV=7.14
    {846,1136,1024, 0, 0, 0},  //TV = 10.21(57 lines)  AV=2.00  SV=5.15  BV=7.06
    {905,1136,1024, 0, 0, 0},  //TV = 10.11(61 lines)  AV=2.00  SV=5.15  BV=6.96
    {965,1152,1024, 0, 0, 0},  //TV = 10.02(65 lines)  AV=2.00  SV=5.17  BV=6.85
    {1039,1152,1024, 0, 0, 0},  //TV = 9.91(70 lines)  AV=2.00  SV=5.17  BV=6.74
    {1128,1136,1024, 0, 0, 0},  //TV = 9.79(76 lines)  AV=2.00  SV=5.15  BV=6.64
    {1202,1136,1024, 0, 0, 0},  //TV = 9.70(81 lines)  AV=2.00  SV=5.15  BV=6.55
    {1291,1136,1024, 0, 0, 0},  //TV = 9.60(87 lines)  AV=2.00  SV=5.15  BV=6.45
    {1380,1136,1024, 0, 0, 0},  //TV = 9.50(93 lines)  AV=2.00  SV=5.15  BV=6.35
    {1484,1136,1024, 0, 0, 0},  //TV = 9.40(100 lines)  AV=2.00  SV=5.15  BV=6.25
    {1588,1136,1024, 0, 0, 0},  //TV = 9.30(107 lines)  AV=2.00  SV=5.15  BV=6.15
    {1692,1136,1024, 0, 0, 0},  //TV = 9.21(114 lines)  AV=2.00  SV=5.15  BV=6.06
    {1810,1136,1024, 0, 0, 0},  //TV = 9.11(122 lines)  AV=2.00  SV=5.15  BV=5.96
    {1959,1136,1024, 0, 0, 0},  //TV = 9.00(132 lines)  AV=2.00  SV=5.15  BV=5.85
    {2107,1136,1024, 0, 0, 0},  //TV = 8.89(142 lines)  AV=2.00  SV=5.15  BV=5.74
    {2255,1136,1024, 0, 0, 0},  //TV = 8.79(152 lines)  AV=2.00  SV=5.15  BV=5.64
    {2389,1136,1024, 0, 0, 0},  //TV = 8.71(161 lines)  AV=2.00  SV=5.15  BV=5.56
    {2582,1136,1024, 0, 0, 0},  //TV = 8.60(174 lines)  AV=2.00  SV=5.15  BV=5.45
    {2774,1136,1024, 0, 0, 0},  //TV = 8.49(187 lines)  AV=2.00  SV=5.15  BV=5.34
    {2967,1136,1024, 0, 0, 0},  //TV = 8.40(200 lines)  AV=2.00  SV=5.15  BV=5.25
    {3175,1136,1024, 0, 0, 0},  //TV = 8.30(214 lines)  AV=2.00  SV=5.15  BV=5.15
    {3397,1136,1024, 0, 0, 0},  //TV = 8.20(229 lines)  AV=2.00  SV=5.15  BV=5.05
    {3635,1136,1024, 0, 0, 0},  //TV = 8.10(245 lines)  AV=2.00  SV=5.15  BV=4.95
    {3932,1136,1024, 0, 0, 0},  //TV = 7.99(265 lines)  AV=2.00  SV=5.15  BV=4.84
    {4199,1136,1024, 0, 0, 0},  //TV = 7.90(283 lines)  AV=2.00  SV=5.15  BV=4.75
    {4495,1136,1024, 0, 0, 0},  //TV = 7.80(303 lines)  AV=2.00  SV=5.15  BV=4.65
    {4822,1136,1024, 0, 0, 0},  //TV = 7.70(325 lines)  AV=2.00  SV=5.15  BV=4.55
    {5163,1136,1024, 0, 0, 0},  //TV = 7.60(348 lines)  AV=2.00  SV=5.15  BV=4.45
    {5563,1136,1024, 0, 0, 0},  //TV = 7.49(375 lines)  AV=2.00  SV=5.15  BV=4.34
    {5949,1136,1024, 0, 0, 0},  //TV = 7.39(401 lines)  AV=2.00  SV=5.15  BV=4.24
    {6379,1136,1024, 0, 0, 0},  //TV = 7.29(430 lines)  AV=2.00  SV=5.15  BV=4.14
    {6824,1136,1024, 0, 0, 0},  //TV = 7.20(460 lines)  AV=2.00  SV=5.15  BV=4.05
    {7314,1136,1024, 0, 0, 0},  //TV = 7.10(493 lines)  AV=2.00  SV=5.15  BV=3.95
    {7833,1136,1024, 0, 0, 0},  //TV = 7.00(528 lines)  AV=2.00  SV=5.15  BV=3.85
    {8397,1136,1024, 0, 0, 0},  //TV = 6.90(566 lines)  AV=2.00  SV=5.15  BV=3.75
    {8990,1136,1024, 0, 0, 0},  //TV = 6.80(606 lines)  AV=2.00  SV=5.15  BV=3.65
    {9628,1136,1024, 0, 0, 0},  //TV = 6.70(649 lines)  AV=2.00  SV=5.15  BV=3.55
    {9999,1152,1040, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.19  BV=3.45
    {9999,1152,1112, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.29  BV=3.36
    {9999,1280,1080, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.40  BV=3.25
    {9999,1408,1056, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.50  BV=3.14
    {9999,1536,1032, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.60  BV=3.05
    {9999,1664,1024, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.70  BV=2.94
    {9999,1664,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.79  BV=2.86
    {9999,1792,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.89  BV=2.75
    {9999,1920,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.99  BV=2.65
    {9999,2176,1032, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=6.10  BV=2.55
    {19997,1152,1040, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.19  BV=2.45
    {19997,1152,1112, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.29  BV=2.36
    {19997,1280,1080, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.40  BV=2.25
    {19997,1408,1048, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.49  BV=2.15
    {19997,1536,1032, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.60  BV=2.05
    {19997,1664,1024, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.70  BV=1.94
    {29995,1152,1048, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.20  BV=1.86
    {29995,1152,1128, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.31  BV=1.75
    {29995,1280,1088, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.41  BV=1.65
    {29995,1408,1064, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.51  BV=1.54
    {29995,1536,1040, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.61  BV=1.45
    {29995,1664,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.71  BV=1.35
    {29995,1792,1024, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.81  BV=1.25
    {29995,1920,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.92  BV=1.14
    {29995,2048,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.01  BV=1.05
    {29995,2176,1040, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.11  BV=0.95
    {29995,2304,1056, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.21  BV=0.84
    {40008,1792,1088, 0, 0, 0},  //TV = 4.64(2697 lines)  AV=2.00  SV=5.89  BV=0.75
    {40008,1920,1088, 0, 0, 0},  //TV = 4.64(2697 lines)  AV=2.00  SV=5.99  BV=0.65
    {40008,2176,1024, 0, 0, 0},  //TV = 4.64(2697 lines)  AV=2.00  SV=6.09  BV=0.56
    {50006,1792,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.87  BV=0.45
    {50006,1920,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.97  BV=0.35
    {50006,2048,1080, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.08  BV=0.24
    {50006,2304,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.17  BV=0.15
    {50006,2432,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.27  BV=0.05
    {50006,2560,1064, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.38  BV=-0.06
    {50006,2816,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.47  BV=-0.15
    {50006,2944,1056, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.57  BV=-0.25
    {50006,3200,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.67  BV=-0.34
    {50006,3456,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.78  BV=-0.46
    {50006,3712,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.87  BV=-0.55
    {50006,3968,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.97  BV=-0.64
    {50006,4224,1048, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.08  BV=-0.76
    {60004,3840,1024, 0, 0, 0},  //TV = 4.06(4045 lines)  AV=2.00  SV=6.91  BV=-0.85
    {60004,4096,1032, 0, 0, 0},  //TV = 4.06(4045 lines)  AV=2.00  SV=7.01  BV=-0.95
    {70002,3712,1048, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=6.89  BV=-1.05
    {70002,3968,1048, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=6.99  BV=-1.15
    {70002,4352,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.09  BV=-1.25
    {70002,4608,1040, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.19  BV=-1.36
    {70002,4992,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.29  BV=-1.45
    {70002,5376,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.39  BV=-1.56
    {70002,5632,1040, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.48  BV=-1.65
    {70002,6144,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.58  BV=-1.75
    {70002,6528,1032, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.68  BV=-1.85
    {70002,7040,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.78  BV=-1.94
    {70002,7552,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.88  BV=-2.05
    {70002,8064,1032, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.99  BV=-2.15
    {70002,8704,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=8.09  BV=-2.25
    {80000,8064,1032, 0, 0, 0},  //TV = 3.64(5393 lines)  AV=2.00  SV=7.99  BV=-2.34
    {89998,7680,1032, 0, 0, 0},  //TV = 3.47(6067 lines)  AV=2.00  SV=7.92  BV=-2.44
    {89998,8320,1024, 0, 0, 0},  //TV = 3.47(6067 lines)  AV=2.00  SV=8.02  BV=-2.55
    {99996,7936,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=7.97  BV=-2.64
    {99996,8576,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.08  BV=-2.76
    {99996,9216,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.17  BV=-2.85
    {99996,9856,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.27  BV=-2.94
    {99996,10624,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.38  BV=-3.05
    {99996,11392,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.48  BV=-3.15
    {99996,12160,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.57  BV=-3.25
    {99996,13056,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.67  BV=-3.35
    {99996,13952,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.77  BV=-3.45
    {99996,14976,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.87  BV=-3.55
    {99996,16000,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.97  BV=-3.64
    {99996,16384,1024, 0, 0, 0},  /* TV = 3.32(6741 lines)  AV=2.00  SV=9.00  BV=-3.68 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
},
};

static strAETable g_AE_SceneTable2 =
{
    AETABLE_SCENE_INDEX2, //eAETableID
    156, //u4TotalIndex
    -20, //i4StrobeTrigerBV
    118, //i4MaxBV
    -37, //i4MinBV
    90, //i4EffectiveMaxBV
    0, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_AUTO, //ISO
    sAEScene2PLineTable_60Hz,
    sAEScene2PLineTable_50Hz,
    NULL,
};
static strEvPline sAEScene3PLineTable_60Hz =
{
{
    {30,1152,1104, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.28  BV=11.75
    {30,1280,1056, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.37  BV=11.66
    {30,1408,1040, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.48  BV=11.54
    {30,1536,1032, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.60  BV=11.43
    {30,1664,1024, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.70  BV=11.32
    {45,1152,1048, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.20  BV=11.24
    {45,1152,1120, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.30  BV=11.14
    {45,1280,1088, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.41  BV=11.03
    {45,1408,1040, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.48  BV=10.96
    {60,1152,1056, 0, 0, 0},  /* TV = 14.02(4 lines)  AV=2.00  SV=5.21  BV=10.81 */
    {60,1152,1128, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.31  BV=10.72
    {60,1280,1072, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.39  BV=10.64
    {75,1152,1032, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.18  BV=10.52
    {75,1152,1088, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.26  BV=10.45
    {75,1280,1056, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.37  BV=10.34
    {90,1152,1040, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.19  BV=10.25
    {90,1280,1024, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.32  BV=10.12
    {104,1152,1040, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.19  BV=10.04
    {104,1152,1104, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.28  BV=9.95
    {119,1152,1032, 0, 0, 0},  //TV = 13.04(8 lines)  AV=2.00  SV=5.18  BV=9.86
    {134,1136,1024, 0, 0, 0},  /* TV = 12.87(9 lines)  AV=2.00  SV=5.15  BV=9.72 */
    {134,1152,1064, 0, 0, 0},  //TV = 12.87(9 lines)  AV=2.00  SV=5.23  BV=9.64
    {149,1152,1032, 0, 0, 0},  //TV = 12.71(10 lines)  AV=2.00  SV=5.18  BV=9.53
    {164,1136,1024, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.15  BV=9.42
    {164,1152,1088, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.26  BV=9.32
    {179,1152,1040, 0, 0, 0},  /* TV = 12.45(12 lines)  AV=2.00  SV=5.19  BV=9.26 */
    {193,1152,1032, 0, 0, 0},  //TV = 12.34(13 lines)  AV=2.00  SV=5.18  BV=9.16
    {208,1152,1032, 0, 0, 0},  //TV = 12.23(14 lines)  AV=2.00  SV=5.18  BV=9.05
    {223,1152,1032, 0, 0, 0},  //TV = 12.13(15 lines)  AV=2.00  SV=5.18  BV=8.95
    {238,1152,1032, 0, 0, 0},  //TV = 12.04(16 lines)  AV=2.00  SV=5.18  BV=8.86
    {253,1152,1040, 0, 0, 0},  //TV = 11.95(17 lines)  AV=2.00  SV=5.19  BV=8.76
    {268,1152,1056, 0, 0, 0},  //TV = 11.87(18 lines)  AV=2.00  SV=5.21  BV=8.65
    {297,1152,1024, 0, 0, 0},  //TV = 11.72(20 lines)  AV=2.00  SV=5.17  BV=8.55
    {312,1152,1040, 0, 0, 0},  //TV = 11.65(21 lines)  AV=2.00  SV=5.19  BV=8.45
    {342,1152,1024, 0, 0, 0},  //TV = 11.51(23 lines)  AV=2.00  SV=5.17  BV=8.34
    {371,1136,1024, 0, 0, 0},  //TV = 11.40(25 lines)  AV=2.00  SV=5.15  BV=8.25
    {386,1152,1032, 0, 0, 0},  //TV = 11.34(26 lines)  AV=2.00  SV=5.18  BV=8.16
    {416,1152,1032, 0, 0, 0},  //TV = 11.23(28 lines)  AV=2.00  SV=5.18  BV=8.05
    {446,1152,1032, 0, 0, 0},  //TV = 11.13(30 lines)  AV=2.00  SV=5.18  BV=7.95
    {475,1152,1032, 0, 0, 0},  //TV = 11.04(32 lines)  AV=2.00  SV=5.18  BV=7.86
    {520,1136,1024, 0, 0, 0},  //TV = 10.91(35 lines)  AV=2.00  SV=5.15  BV=7.76
    {564,1136,1024, 0, 0, 0},  //TV = 10.79(38 lines)  AV=2.00  SV=5.15  BV=7.64
    {594,1152,1024, 0, 0, 0},  //TV = 10.72(40 lines)  AV=2.00  SV=5.17  BV=7.55
    {638,1152,1024, 0, 0, 0},  //TV = 10.61(43 lines)  AV=2.00  SV=5.17  BV=7.44
    {683,1152,1024, 0, 0, 0},  //TV = 10.52(46 lines)  AV=2.00  SV=5.17  BV=7.35
    {727,1152,1024, 0, 0, 0},  //TV = 10.43(49 lines)  AV=2.00  SV=5.17  BV=7.26
    {787,1152,1024, 0, 0, 0},  //TV = 10.31(53 lines)  AV=2.00  SV=5.17  BV=7.14
    {846,1136,1024, 0, 0, 0},  //TV = 10.21(57 lines)  AV=2.00  SV=5.15  BV=7.06
    {905,1136,1024, 0, 0, 0},  //TV = 10.11(61 lines)  AV=2.00  SV=5.15  BV=6.96
    {965,1152,1024, 0, 0, 0},  //TV = 10.02(65 lines)  AV=2.00  SV=5.17  BV=6.85
    {1039,1152,1024, 0, 0, 0},  //TV = 9.91(70 lines)  AV=2.00  SV=5.17  BV=6.74
    {1128,1136,1024, 0, 0, 0},  //TV = 9.79(76 lines)  AV=2.00  SV=5.15  BV=6.64
    {1202,1136,1024, 0, 0, 0},  //TV = 9.70(81 lines)  AV=2.00  SV=5.15  BV=6.55
    {1291,1136,1024, 0, 0, 0},  //TV = 9.60(87 lines)  AV=2.00  SV=5.15  BV=6.45
    {1380,1136,1024, 0, 0, 0},  //TV = 9.50(93 lines)  AV=2.00  SV=5.15  BV=6.35
    {1484,1136,1024, 0, 0, 0},  //TV = 9.40(100 lines)  AV=2.00  SV=5.15  BV=6.25
    {1588,1136,1024, 0, 0, 0},  //TV = 9.30(107 lines)  AV=2.00  SV=5.15  BV=6.15
    {1692,1136,1024, 0, 0, 0},  //TV = 9.21(114 lines)  AV=2.00  SV=5.15  BV=6.06
    {1810,1136,1024, 0, 0, 0},  //TV = 9.11(122 lines)  AV=2.00  SV=5.15  BV=5.96
    {1959,1136,1024, 0, 0, 0},  //TV = 9.00(132 lines)  AV=2.00  SV=5.15  BV=5.85
    {2107,1136,1024, 0, 0, 0},  //TV = 8.89(142 lines)  AV=2.00  SV=5.15  BV=5.74
    {2255,1136,1024, 0, 0, 0},  //TV = 8.79(152 lines)  AV=2.00  SV=5.15  BV=5.64
    {2389,1136,1024, 0, 0, 0},  //TV = 8.71(161 lines)  AV=2.00  SV=5.15  BV=5.56
    {2582,1136,1024, 0, 0, 0},  //TV = 8.60(174 lines)  AV=2.00  SV=5.15  BV=5.45
    {2774,1136,1024, 0, 0, 0},  //TV = 8.49(187 lines)  AV=2.00  SV=5.15  BV=5.34
    {2967,1136,1024, 0, 0, 0},  //TV = 8.40(200 lines)  AV=2.00  SV=5.15  BV=5.25
    {3175,1136,1024, 0, 0, 0},  //TV = 8.30(214 lines)  AV=2.00  SV=5.15  BV=5.15
    {3397,1136,1024, 0, 0, 0},  //TV = 8.20(229 lines)  AV=2.00  SV=5.15  BV=5.05
    {3635,1136,1024, 0, 0, 0},  //TV = 8.10(245 lines)  AV=2.00  SV=5.15  BV=4.95
    {3932,1136,1024, 0, 0, 0},  //TV = 7.99(265 lines)  AV=2.00  SV=5.15  BV=4.84
    {4199,1136,1024, 0, 0, 0},  //TV = 7.90(283 lines)  AV=2.00  SV=5.15  BV=4.75
    {4495,1136,1024, 0, 0, 0},  //TV = 7.80(303 lines)  AV=2.00  SV=5.15  BV=4.65
    {4822,1136,1024, 0, 0, 0},  //TV = 7.70(325 lines)  AV=2.00  SV=5.15  BV=4.55
    {5163,1136,1024, 0, 0, 0},  //TV = 7.60(348 lines)  AV=2.00  SV=5.15  BV=4.45
    {5563,1136,1024, 0, 0, 0},  //TV = 7.49(375 lines)  AV=2.00  SV=5.15  BV=4.34
    {5949,1136,1024, 0, 0, 0},  //TV = 7.39(401 lines)  AV=2.00  SV=5.15  BV=4.24
    {6379,1136,1024, 0, 0, 0},  //TV = 7.29(430 lines)  AV=2.00  SV=5.15  BV=4.14
    {6824,1136,1024, 0, 0, 0},  //TV = 7.20(460 lines)  AV=2.00  SV=5.15  BV=4.05
    {7314,1136,1024, 0, 0, 0},  //TV = 7.10(493 lines)  AV=2.00  SV=5.15  BV=3.95
    {7833,1136,1024, 0, 0, 0},  //TV = 7.00(528 lines)  AV=2.00  SV=5.15  BV=3.85
    {8337,1136,1024, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.15  BV=3.76
    {8337,1152,1088, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.26  BV=3.65
    {8337,1280,1048, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.36  BV=3.55
    {8337,1408,1024, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.46  BV=3.45
    {8337,1408,1104, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.57  BV=3.34
    {8337,1536,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.65  BV=3.26
    {8337,1664,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.77  BV=3.14
    {8337,1792,1056, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.85  BV=3.05
    {8337,1920,1064, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.96  BV=2.94
    {8337,2048,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=6.07  BV=2.84
    {16659,1136,1024, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.15  BV=2.76
    {16659,1152,1088, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.26  BV=2.65
    {16659,1280,1056, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.37  BV=2.54
    {16659,1408,1024, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.46  BV=2.45
    {16659,1408,1096, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.56  BV=2.35
    {16659,1536,1080, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.66  BV=2.25
    {24996,1152,1024, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.17  BV=2.15
    {24996,1152,1096, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.27  BV=2.05
    {24996,1280,1056, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.37  BV=1.96
    {24996,1408,1032, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.47  BV=1.85
    {33332,1136,1024, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.15  BV=1.76
    {33332,1152,1088, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.26  BV=1.65
    {33332,1280,1048, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.36  BV=1.55
    {33332,1408,1024, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.46  BV=1.45
    {33332,1408,1096, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.56  BV=1.35
    {33332,1536,1072, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.65  BV=1.26
    {33332,1664,1072, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.77  BV=1.14
    {33332,1792,1064, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.86  BV=1.04
    {33332,1920,1056, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.95  BV=0.96
    {33332,2048,1064, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=6.06  BV=0.85
    {33332,2176,1072, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=6.15  BV=0.75
    {41669,1920,1040, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=5.93  BV=0.66
    {41669,2048,1048, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=6.03  BV=0.55
    {41669,2176,1056, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=6.13  BV=0.45
    {50006,1920,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.97  BV=0.35
    {50006,2048,1080, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.08  BV=0.24
    {50006,2304,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.17  BV=0.15
    {50006,2432,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.27  BV=0.05
    {50006,2560,1064, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.38  BV=-0.06
    {50006,2816,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.47  BV=-0.15
    {50006,2944,1056, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.57  BV=-0.25
    {50006,3200,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.67  BV=-0.34
    {50006,3456,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.78  BV=-0.46
    {50006,3712,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.87  BV=-0.55
    {50006,3968,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.97  BV=-0.64
    {50006,4224,1048, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.08  BV=-0.76
    {58328,3968,1024, 0, 0, 0},  //TV = 4.10(3932 lines)  AV=2.00  SV=6.95  BV=-0.85
    {58328,4224,1032, 0, 0, 0},  //TV = 4.10(3932 lines)  AV=2.00  SV=7.06  BV=-0.96
    {66664,3968,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=6.97  BV=-1.06
    {66664,4224,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.06  BV=-1.15
    {66664,4480,1040, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.15  BV=-1.24
    {66664,4864,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.26  BV=-1.35
    {66664,5248,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.36  BV=-1.45
    {66664,5632,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.46  BV=-1.55
    {66664,6016,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.55  BV=-1.65
    {66664,6400,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.66  BV=-1.75
    {66664,6912,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.75  BV=-1.85
    {66664,7424,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.86  BV=-1.95
    {66664,7936,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.95  BV=-2.05
    {66664,8448,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=8.06  BV=-2.15
    {75001,8064,1032, 0, 0, 0},  //TV = 3.74(5056 lines)  AV=2.00  SV=7.99  BV=-2.25
    {83338,7808,1024, 0, 0, 0},  //TV = 3.58(5618 lines)  AV=2.00  SV=7.93  BV=-2.35
    {83338,8320,1032, 0, 0, 0},  //TV = 3.58(5618 lines)  AV=2.00  SV=8.03  BV=-2.45
    {91660,8192,1024, 0, 0, 0},  //TV = 3.45(6179 lines)  AV=2.00  SV=8.00  BV=-2.55
    {99996,7936,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=7.97  BV=-2.64
    {99996,8576,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.08  BV=-2.76
    {99996,9216,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.17  BV=-2.85
    {99996,9856,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.27  BV=-2.94
    {99996,10624,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.38  BV=-3.05
    {99996,11392,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.48  BV=-3.15
    {99996,12160,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.57  BV=-3.25
    {99996,13056,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.67  BV=-3.35
    {99996,13952,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.77  BV=-3.45
    {99996,14976,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.87  BV=-3.55
    {99996,16000,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.97  BV=-3.64
    {99996,16384,1024, 0, 0, 0},  /* TV = 3.32(6741 lines)  AV=2.00  SV=9.00  BV=-3.68 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
},
};

static strEvPline sAEScene3PLineTable_50Hz =
{
{
    {30,1152,1104, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.28  BV=11.75
    {30,1280,1056, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.37  BV=11.66
    {30,1408,1040, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.48  BV=11.54
    {30,1536,1032, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.60  BV=11.43
    {30,1664,1024, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.70  BV=11.32
    {45,1152,1048, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.20  BV=11.24
    {45,1152,1120, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.30  BV=11.14
    {45,1280,1088, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.41  BV=11.03
    {45,1408,1040, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.48  BV=10.96
    {60,1152,1056, 0, 0, 0},  /* TV = 14.02(4 lines)  AV=2.00  SV=5.21  BV=10.81 */
    {60,1152,1128, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.31  BV=10.72
    {60,1280,1072, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.39  BV=10.64
    {75,1152,1032, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.18  BV=10.52
    {75,1152,1088, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.26  BV=10.45
    {75,1280,1056, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.37  BV=10.34
    {90,1152,1040, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.19  BV=10.25
    {90,1280,1024, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.32  BV=10.12
    {104,1152,1040, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.19  BV=10.04
    {104,1152,1104, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.28  BV=9.95
    {119,1152,1032, 0, 0, 0},  //TV = 13.04(8 lines)  AV=2.00  SV=5.18  BV=9.86
    {134,1136,1024, 0, 0, 0},  /* TV = 12.87(9 lines)  AV=2.00  SV=5.15  BV=9.72 */
    {134,1152,1064, 0, 0, 0},  //TV = 12.87(9 lines)  AV=2.00  SV=5.23  BV=9.64
    {149,1152,1032, 0, 0, 0},  //TV = 12.71(10 lines)  AV=2.00  SV=5.18  BV=9.53
    {164,1136,1024, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.15  BV=9.42
    {164,1152,1088, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.26  BV=9.32
    {179,1152,1040, 0, 0, 0},  /* TV = 12.45(12 lines)  AV=2.00  SV=5.19  BV=9.26 */
    {193,1152,1032, 0, 0, 0},  //TV = 12.34(13 lines)  AV=2.00  SV=5.18  BV=9.16
    {208,1152,1032, 0, 0, 0},  //TV = 12.23(14 lines)  AV=2.00  SV=5.18  BV=9.05
    {223,1152,1032, 0, 0, 0},  //TV = 12.13(15 lines)  AV=2.00  SV=5.18  BV=8.95
    {238,1152,1032, 0, 0, 0},  //TV = 12.04(16 lines)  AV=2.00  SV=5.18  BV=8.86
    {253,1152,1040, 0, 0, 0},  //TV = 11.95(17 lines)  AV=2.00  SV=5.19  BV=8.76
    {268,1152,1056, 0, 0, 0},  //TV = 11.87(18 lines)  AV=2.00  SV=5.21  BV=8.65
    {297,1152,1024, 0, 0, 0},  //TV = 11.72(20 lines)  AV=2.00  SV=5.17  BV=8.55
    {312,1152,1040, 0, 0, 0},  //TV = 11.65(21 lines)  AV=2.00  SV=5.19  BV=8.45
    {342,1152,1024, 0, 0, 0},  //TV = 11.51(23 lines)  AV=2.00  SV=5.17  BV=8.34
    {371,1136,1024, 0, 0, 0},  //TV = 11.40(25 lines)  AV=2.00  SV=5.15  BV=8.25
    {386,1152,1032, 0, 0, 0},  //TV = 11.34(26 lines)  AV=2.00  SV=5.18  BV=8.16
    {416,1152,1032, 0, 0, 0},  //TV = 11.23(28 lines)  AV=2.00  SV=5.18  BV=8.05
    {446,1152,1032, 0, 0, 0},  //TV = 11.13(30 lines)  AV=2.00  SV=5.18  BV=7.95
    {475,1152,1032, 0, 0, 0},  //TV = 11.04(32 lines)  AV=2.00  SV=5.18  BV=7.86
    {520,1136,1024, 0, 0, 0},  //TV = 10.91(35 lines)  AV=2.00  SV=5.15  BV=7.76
    {564,1136,1024, 0, 0, 0},  //TV = 10.79(38 lines)  AV=2.00  SV=5.15  BV=7.64
    {594,1152,1024, 0, 0, 0},  //TV = 10.72(40 lines)  AV=2.00  SV=5.17  BV=7.55
    {638,1152,1024, 0, 0, 0},  //TV = 10.61(43 lines)  AV=2.00  SV=5.17  BV=7.44
    {683,1152,1024, 0, 0, 0},  //TV = 10.52(46 lines)  AV=2.00  SV=5.17  BV=7.35
    {727,1152,1024, 0, 0, 0},  //TV = 10.43(49 lines)  AV=2.00  SV=5.17  BV=7.26
    {787,1152,1024, 0, 0, 0},  //TV = 10.31(53 lines)  AV=2.00  SV=5.17  BV=7.14
    {846,1136,1024, 0, 0, 0},  //TV = 10.21(57 lines)  AV=2.00  SV=5.15  BV=7.06
    {905,1136,1024, 0, 0, 0},  //TV = 10.11(61 lines)  AV=2.00  SV=5.15  BV=6.96
    {965,1152,1024, 0, 0, 0},  //TV = 10.02(65 lines)  AV=2.00  SV=5.17  BV=6.85
    {1039,1152,1024, 0, 0, 0},  //TV = 9.91(70 lines)  AV=2.00  SV=5.17  BV=6.74
    {1128,1136,1024, 0, 0, 0},  //TV = 9.79(76 lines)  AV=2.00  SV=5.15  BV=6.64
    {1202,1136,1024, 0, 0, 0},  //TV = 9.70(81 lines)  AV=2.00  SV=5.15  BV=6.55
    {1291,1136,1024, 0, 0, 0},  //TV = 9.60(87 lines)  AV=2.00  SV=5.15  BV=6.45
    {1380,1136,1024, 0, 0, 0},  //TV = 9.50(93 lines)  AV=2.00  SV=5.15  BV=6.35
    {1484,1136,1024, 0, 0, 0},  //TV = 9.40(100 lines)  AV=2.00  SV=5.15  BV=6.25
    {1588,1136,1024, 0, 0, 0},  //TV = 9.30(107 lines)  AV=2.00  SV=5.15  BV=6.15
    {1692,1136,1024, 0, 0, 0},  //TV = 9.21(114 lines)  AV=2.00  SV=5.15  BV=6.06
    {1810,1136,1024, 0, 0, 0},  //TV = 9.11(122 lines)  AV=2.00  SV=5.15  BV=5.96
    {1959,1136,1024, 0, 0, 0},  //TV = 9.00(132 lines)  AV=2.00  SV=5.15  BV=5.85
    {2107,1136,1024, 0, 0, 0},  //TV = 8.89(142 lines)  AV=2.00  SV=5.15  BV=5.74
    {2255,1136,1024, 0, 0, 0},  //TV = 8.79(152 lines)  AV=2.00  SV=5.15  BV=5.64
    {2389,1136,1024, 0, 0, 0},  //TV = 8.71(161 lines)  AV=2.00  SV=5.15  BV=5.56
    {2582,1136,1024, 0, 0, 0},  //TV = 8.60(174 lines)  AV=2.00  SV=5.15  BV=5.45
    {2774,1136,1024, 0, 0, 0},  //TV = 8.49(187 lines)  AV=2.00  SV=5.15  BV=5.34
    {2967,1136,1024, 0, 0, 0},  //TV = 8.40(200 lines)  AV=2.00  SV=5.15  BV=5.25
    {3175,1136,1024, 0, 0, 0},  //TV = 8.30(214 lines)  AV=2.00  SV=5.15  BV=5.15
    {3397,1136,1024, 0, 0, 0},  //TV = 8.20(229 lines)  AV=2.00  SV=5.15  BV=5.05
    {3635,1136,1024, 0, 0, 0},  //TV = 8.10(245 lines)  AV=2.00  SV=5.15  BV=4.95
    {3932,1136,1024, 0, 0, 0},  //TV = 7.99(265 lines)  AV=2.00  SV=5.15  BV=4.84
    {4199,1136,1024, 0, 0, 0},  //TV = 7.90(283 lines)  AV=2.00  SV=5.15  BV=4.75
    {4495,1136,1024, 0, 0, 0},  //TV = 7.80(303 lines)  AV=2.00  SV=5.15  BV=4.65
    {4822,1136,1024, 0, 0, 0},  //TV = 7.70(325 lines)  AV=2.00  SV=5.15  BV=4.55
    {5163,1136,1024, 0, 0, 0},  //TV = 7.60(348 lines)  AV=2.00  SV=5.15  BV=4.45
    {5563,1136,1024, 0, 0, 0},  //TV = 7.49(375 lines)  AV=2.00  SV=5.15  BV=4.34
    {5949,1136,1024, 0, 0, 0},  //TV = 7.39(401 lines)  AV=2.00  SV=5.15  BV=4.24
    {6379,1136,1024, 0, 0, 0},  //TV = 7.29(430 lines)  AV=2.00  SV=5.15  BV=4.14
    {6824,1136,1024, 0, 0, 0},  //TV = 7.20(460 lines)  AV=2.00  SV=5.15  BV=4.05
    {7314,1136,1024, 0, 0, 0},  //TV = 7.10(493 lines)  AV=2.00  SV=5.15  BV=3.95
    {7833,1136,1024, 0, 0, 0},  //TV = 7.00(528 lines)  AV=2.00  SV=5.15  BV=3.85
    {8397,1136,1024, 0, 0, 0},  //TV = 6.90(566 lines)  AV=2.00  SV=5.15  BV=3.75
    {8990,1136,1024, 0, 0, 0},  //TV = 6.80(606 lines)  AV=2.00  SV=5.15  BV=3.65
    {9628,1136,1024, 0, 0, 0},  //TV = 6.70(649 lines)  AV=2.00  SV=5.15  BV=3.55
    {9999,1152,1040, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.19  BV=3.45
    {9999,1152,1112, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.29  BV=3.36
    {9999,1280,1080, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.40  BV=3.25
    {9999,1408,1056, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.50  BV=3.14
    {9999,1536,1032, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.60  BV=3.05
    {9999,1664,1024, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.70  BV=2.94
    {9999,1664,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.79  BV=2.86
    {9999,1792,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.89  BV=2.75
    {9999,1920,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.99  BV=2.65
    {9999,2176,1032, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=6.10  BV=2.55
    {19997,1152,1040, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.19  BV=2.45
    {19997,1152,1112, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.29  BV=2.36
    {19997,1280,1080, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.40  BV=2.25
    {19997,1408,1048, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.49  BV=2.15
    {19997,1536,1032, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.60  BV=2.05
    {19997,1664,1024, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.70  BV=1.94
    {29995,1152,1048, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.20  BV=1.86
    {29995,1152,1128, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.31  BV=1.75
    {29995,1280,1088, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.41  BV=1.65
    {29995,1408,1064, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.51  BV=1.54
    {29995,1536,1040, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.61  BV=1.45
    {29995,1664,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.71  BV=1.35
    {29995,1792,1024, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.81  BV=1.25
    {29995,1920,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.92  BV=1.14
    {29995,2048,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.01  BV=1.05
    {29995,2176,1040, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.11  BV=0.95
    {29995,2304,1056, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.21  BV=0.84
    {40008,1792,1088, 0, 0, 0},  //TV = 4.64(2697 lines)  AV=2.00  SV=5.89  BV=0.75
    {40008,1920,1088, 0, 0, 0},  //TV = 4.64(2697 lines)  AV=2.00  SV=5.99  BV=0.65
    {40008,2176,1024, 0, 0, 0},  //TV = 4.64(2697 lines)  AV=2.00  SV=6.09  BV=0.56
    {50006,1792,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.87  BV=0.45
    {50006,1920,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.97  BV=0.35
    {50006,2048,1080, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.08  BV=0.24
    {50006,2304,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.17  BV=0.15
    {50006,2432,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.27  BV=0.05
    {50006,2560,1064, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.38  BV=-0.06
    {50006,2816,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.47  BV=-0.15
    {50006,2944,1056, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.57  BV=-0.25
    {50006,3200,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.67  BV=-0.34
    {50006,3456,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.78  BV=-0.46
    {50006,3712,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.87  BV=-0.55
    {50006,3968,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.97  BV=-0.64
    {50006,4224,1048, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.08  BV=-0.76
    {60004,3840,1024, 0, 0, 0},  //TV = 4.06(4045 lines)  AV=2.00  SV=6.91  BV=-0.85
    {60004,4096,1032, 0, 0, 0},  //TV = 4.06(4045 lines)  AV=2.00  SV=7.01  BV=-0.95
    {70002,3712,1048, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=6.89  BV=-1.05
    {70002,3968,1048, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=6.99  BV=-1.15
    {70002,4352,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.09  BV=-1.25
    {70002,4608,1040, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.19  BV=-1.36
    {70002,4992,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.29  BV=-1.45
    {70002,5376,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.39  BV=-1.56
    {70002,5632,1040, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.48  BV=-1.65
    {70002,6144,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.58  BV=-1.75
    {70002,6528,1032, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.68  BV=-1.85
    {70002,7040,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.78  BV=-1.94
    {70002,7552,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.88  BV=-2.05
    {70002,8064,1032, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.99  BV=-2.15
    {70002,8704,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=8.09  BV=-2.25
    {80000,8064,1032, 0, 0, 0},  //TV = 3.64(5393 lines)  AV=2.00  SV=7.99  BV=-2.34
    {89998,7680,1032, 0, 0, 0},  //TV = 3.47(6067 lines)  AV=2.00  SV=7.92  BV=-2.44
    {89998,8320,1024, 0, 0, 0},  //TV = 3.47(6067 lines)  AV=2.00  SV=8.02  BV=-2.55
    {99996,7936,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=7.97  BV=-2.64
    {99996,8576,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.08  BV=-2.76
    {99996,9216,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.17  BV=-2.85
    {99996,9856,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.27  BV=-2.94
    {99996,10624,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.38  BV=-3.05
    {99996,11392,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.48  BV=-3.15
    {99996,12160,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.57  BV=-3.25
    {99996,13056,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.67  BV=-3.35
    {99996,13952,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.77  BV=-3.45
    {99996,14976,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.87  BV=-3.55
    {99996,16000,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.97  BV=-3.64
    {99996,16384,1024, 0, 0, 0},  /* TV = 3.32(6741 lines)  AV=2.00  SV=9.00  BV=-3.68 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
},
};

static strAETable g_AE_SceneTable3 =
{
    AETABLE_SCENE_INDEX3, //eAETableID
    156, //u4TotalIndex
    -20, //i4StrobeTrigerBV
    118, //i4MaxBV
    -37, //i4MinBV
    90, //i4EffectiveMaxBV
    0, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_AUTO, //ISO
    sAEScene3PLineTable_60Hz,
    sAEScene3PLineTable_50Hz,
    NULL,
};
static strEvPline sAEScene4PLineTable_60Hz =
{
{
    {249983,1136,1024, 0, 0, 0},  //TV = 2.00(16852 lines)  AV=2.00  SV=5.15  BV=-1.15
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
},
};

static strEvPline sAEScene4PLineTable_50Hz =
{
{
    {249998,1136,1024, 0, 0, 0},  //TV = 2.00(16853 lines)  AV=2.00  SV=5.15  BV=-1.15
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
},
};

static strAETable g_AE_SceneTable4 =
{
    AETABLE_SCENE_INDEX4, //eAETableID
    1, //u4TotalIndex
    -20, //i4StrobeTrigerBV
    -11, //i4MaxBV
    -12, //i4MinBV
    90, //i4EffectiveMaxBV
    0, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_AUTO, //ISO
    sAEScene4PLineTable_60Hz,
    sAEScene4PLineTable_50Hz,
    NULL,
};
static strEvPline sAEScene5PLineTable_60Hz =
{
{
    {30,1152,1104, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.28  BV=11.75
    {30,1280,1056, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.37  BV=11.66
    {30,1408,1040, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.48  BV=11.54
    {30,1536,1032, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.60  BV=11.43
    {30,1664,1024, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.70  BV=11.32
    {45,1152,1048, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.20  BV=11.24
    {45,1152,1120, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.30  BV=11.14
    {45,1280,1088, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.41  BV=11.03
    {45,1408,1040, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.48  BV=10.96
    {60,1152,1056, 0, 0, 0},  /* TV = 14.02(4 lines)  AV=2.00  SV=5.21  BV=10.81 */
    {60,1152,1128, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.31  BV=10.72
    {60,1280,1072, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.39  BV=10.64
    {75,1152,1032, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.18  BV=10.52
    {75,1152,1088, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.26  BV=10.45
    {75,1280,1056, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.37  BV=10.34
    {90,1152,1040, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.19  BV=10.25
    {90,1280,1024, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.32  BV=10.12
    {104,1152,1040, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.19  BV=10.04
    {104,1152,1104, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.28  BV=9.95
    {119,1152,1032, 0, 0, 0},  //TV = 13.04(8 lines)  AV=2.00  SV=5.18  BV=9.86
    {134,1136,1024, 0, 0, 0},  /* TV = 12.87(9 lines)  AV=2.00  SV=5.15  BV=9.72 */
    {134,1152,1064, 0, 0, 0},  //TV = 12.87(9 lines)  AV=2.00  SV=5.23  BV=9.64
    {149,1152,1032, 0, 0, 0},  //TV = 12.71(10 lines)  AV=2.00  SV=5.18  BV=9.53
    {164,1136,1024, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.15  BV=9.42
    {164,1152,1088, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.26  BV=9.32
    {179,1152,1040, 0, 0, 0},  /* TV = 12.45(12 lines)  AV=2.00  SV=5.19  BV=9.26 */
    {193,1152,1032, 0, 0, 0},  //TV = 12.34(13 lines)  AV=2.00  SV=5.18  BV=9.16
    {208,1152,1032, 0, 0, 0},  //TV = 12.23(14 lines)  AV=2.00  SV=5.18  BV=9.05
    {223,1152,1032, 0, 0, 0},  //TV = 12.13(15 lines)  AV=2.00  SV=5.18  BV=8.95
    {238,1152,1032, 0, 0, 0},  //TV = 12.04(16 lines)  AV=2.00  SV=5.18  BV=8.86
    {253,1152,1040, 0, 0, 0},  //TV = 11.95(17 lines)  AV=2.00  SV=5.19  BV=8.76
    {268,1152,1056, 0, 0, 0},  //TV = 11.87(18 lines)  AV=2.00  SV=5.21  BV=8.65
    {297,1152,1024, 0, 0, 0},  //TV = 11.72(20 lines)  AV=2.00  SV=5.17  BV=8.55
    {312,1152,1040, 0, 0, 0},  //TV = 11.65(21 lines)  AV=2.00  SV=5.19  BV=8.45
    {342,1152,1024, 0, 0, 0},  //TV = 11.51(23 lines)  AV=2.00  SV=5.17  BV=8.34
    {371,1136,1024, 0, 0, 0},  //TV = 11.40(25 lines)  AV=2.00  SV=5.15  BV=8.25
    {386,1152,1032, 0, 0, 0},  //TV = 11.34(26 lines)  AV=2.00  SV=5.18  BV=8.16
    {416,1152,1032, 0, 0, 0},  //TV = 11.23(28 lines)  AV=2.00  SV=5.18  BV=8.05
    {446,1152,1032, 0, 0, 0},  //TV = 11.13(30 lines)  AV=2.00  SV=5.18  BV=7.95
    {475,1152,1032, 0, 0, 0},  //TV = 11.04(32 lines)  AV=2.00  SV=5.18  BV=7.86
    {520,1136,1024, 0, 0, 0},  //TV = 10.91(35 lines)  AV=2.00  SV=5.15  BV=7.76
    {564,1136,1024, 0, 0, 0},  //TV = 10.79(38 lines)  AV=2.00  SV=5.15  BV=7.64
    {594,1152,1024, 0, 0, 0},  //TV = 10.72(40 lines)  AV=2.00  SV=5.17  BV=7.55
    {638,1152,1024, 0, 0, 0},  //TV = 10.61(43 lines)  AV=2.00  SV=5.17  BV=7.44
    {683,1152,1024, 0, 0, 0},  //TV = 10.52(46 lines)  AV=2.00  SV=5.17  BV=7.35
    {727,1152,1024, 0, 0, 0},  //TV = 10.43(49 lines)  AV=2.00  SV=5.17  BV=7.26
    {787,1152,1024, 0, 0, 0},  //TV = 10.31(53 lines)  AV=2.00  SV=5.17  BV=7.14
    {846,1136,1024, 0, 0, 0},  //TV = 10.21(57 lines)  AV=2.00  SV=5.15  BV=7.06
    {905,1136,1024, 0, 0, 0},  //TV = 10.11(61 lines)  AV=2.00  SV=5.15  BV=6.96
    {965,1152,1024, 0, 0, 0},  //TV = 10.02(65 lines)  AV=2.00  SV=5.17  BV=6.85
    {1039,1152,1024, 0, 0, 0},  //TV = 9.91(70 lines)  AV=2.00  SV=5.17  BV=6.74
    {1128,1136,1024, 0, 0, 0},  //TV = 9.79(76 lines)  AV=2.00  SV=5.15  BV=6.64
    {1202,1136,1024, 0, 0, 0},  //TV = 9.70(81 lines)  AV=2.00  SV=5.15  BV=6.55
    {1291,1136,1024, 0, 0, 0},  //TV = 9.60(87 lines)  AV=2.00  SV=5.15  BV=6.45
    {1380,1136,1024, 0, 0, 0},  //TV = 9.50(93 lines)  AV=2.00  SV=5.15  BV=6.35
    {1484,1136,1024, 0, 0, 0},  //TV = 9.40(100 lines)  AV=2.00  SV=5.15  BV=6.25
    {1588,1136,1024, 0, 0, 0},  //TV = 9.30(107 lines)  AV=2.00  SV=5.15  BV=6.15
    {1692,1136,1024, 0, 0, 0},  //TV = 9.21(114 lines)  AV=2.00  SV=5.15  BV=6.06
    {1810,1136,1024, 0, 0, 0},  //TV = 9.11(122 lines)  AV=2.00  SV=5.15  BV=5.96
    {1959,1136,1024, 0, 0, 0},  //TV = 9.00(132 lines)  AV=2.00  SV=5.15  BV=5.85
    {2107,1136,1024, 0, 0, 0},  //TV = 8.89(142 lines)  AV=2.00  SV=5.15  BV=5.74
    {2255,1136,1024, 0, 0, 0},  //TV = 8.79(152 lines)  AV=2.00  SV=5.15  BV=5.64
    {2389,1136,1024, 0, 0, 0},  //TV = 8.71(161 lines)  AV=2.00  SV=5.15  BV=5.56
    {2582,1136,1024, 0, 0, 0},  //TV = 8.60(174 lines)  AV=2.00  SV=5.15  BV=5.45
    {2774,1136,1024, 0, 0, 0},  //TV = 8.49(187 lines)  AV=2.00  SV=5.15  BV=5.34
    {2967,1136,1024, 0, 0, 0},  //TV = 8.40(200 lines)  AV=2.00  SV=5.15  BV=5.25
    {3175,1136,1024, 0, 0, 0},  //TV = 8.30(214 lines)  AV=2.00  SV=5.15  BV=5.15
    {3397,1136,1024, 0, 0, 0},  //TV = 8.20(229 lines)  AV=2.00  SV=5.15  BV=5.05
    {3635,1136,1024, 0, 0, 0},  //TV = 8.10(245 lines)  AV=2.00  SV=5.15  BV=4.95
    {3932,1136,1024, 0, 0, 0},  //TV = 7.99(265 lines)  AV=2.00  SV=5.15  BV=4.84
    {4199,1136,1024, 0, 0, 0},  //TV = 7.90(283 lines)  AV=2.00  SV=5.15  BV=4.75
    {4495,1136,1024, 0, 0, 0},  //TV = 7.80(303 lines)  AV=2.00  SV=5.15  BV=4.65
    {4822,1136,1024, 0, 0, 0},  //TV = 7.70(325 lines)  AV=2.00  SV=5.15  BV=4.55
    {5163,1136,1024, 0, 0, 0},  //TV = 7.60(348 lines)  AV=2.00  SV=5.15  BV=4.45
    {5563,1136,1024, 0, 0, 0},  //TV = 7.49(375 lines)  AV=2.00  SV=5.15  BV=4.34
    {5949,1136,1024, 0, 0, 0},  //TV = 7.39(401 lines)  AV=2.00  SV=5.15  BV=4.24
    {6379,1136,1024, 0, 0, 0},  //TV = 7.29(430 lines)  AV=2.00  SV=5.15  BV=4.14
    {6824,1136,1024, 0, 0, 0},  //TV = 7.20(460 lines)  AV=2.00  SV=5.15  BV=4.05
    {7314,1136,1024, 0, 0, 0},  //TV = 7.10(493 lines)  AV=2.00  SV=5.15  BV=3.95
    {7833,1136,1024, 0, 0, 0},  //TV = 7.00(528 lines)  AV=2.00  SV=5.15  BV=3.85
    {8337,1136,1024, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.15  BV=3.76
    {8337,1152,1088, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.26  BV=3.65
    {8337,1280,1048, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.36  BV=3.55
    {8337,1408,1024, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.46  BV=3.45
    {8337,1408,1104, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.57  BV=3.34
    {8337,1536,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.65  BV=3.26
    {8337,1664,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.77  BV=3.14
    {8337,1792,1056, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.85  BV=3.05
    {8337,1920,1064, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=5.96  BV=2.94
    {8337,2048,1072, 0, 0, 0},  //TV = 6.91(562 lines)  AV=2.00  SV=6.07  BV=2.84
    {16659,1136,1024, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.15  BV=2.76
    {16659,1152,1088, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.26  BV=2.65
    {16659,1280,1056, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.37  BV=2.54
    {16659,1408,1024, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.46  BV=2.45
    {16659,1408,1096, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.56  BV=2.35
    {16659,1536,1080, 0, 0, 0},  //TV = 5.91(1123 lines)  AV=2.00  SV=5.66  BV=2.25
    {24996,1152,1024, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.17  BV=2.15
    {24996,1152,1096, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.27  BV=2.05
    {24996,1280,1056, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.37  BV=1.96
    {24996,1408,1032, 0, 0, 0},  //TV = 5.32(1685 lines)  AV=2.00  SV=5.47  BV=1.85
    {33332,1136,1024, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.15  BV=1.76
    {33332,1152,1088, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.26  BV=1.65
    {33332,1280,1048, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.36  BV=1.55
    {33332,1408,1024, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.46  BV=1.45
    {33332,1408,1096, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.56  BV=1.35
    {33332,1536,1072, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.65  BV=1.26
    {33332,1664,1072, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.77  BV=1.14
    {33332,1792,1064, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.86  BV=1.04
    {33332,1920,1056, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=5.95  BV=0.96
    {33332,2048,1064, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=6.06  BV=0.85
    {33332,2176,1072, 0, 0, 0},  //TV = 4.91(2247 lines)  AV=2.00  SV=6.15  BV=0.75
    {41669,1920,1040, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=5.93  BV=0.66
    {41669,2048,1048, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=6.03  BV=0.55
    {41669,2176,1056, 0, 0, 0},  //TV = 4.58(2809 lines)  AV=2.00  SV=6.13  BV=0.45
    {50006,1920,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.97  BV=0.35
    {50006,2048,1080, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.08  BV=0.24
    {50006,2304,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.17  BV=0.15
    {50006,2432,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.27  BV=0.05
    {50006,2560,1064, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.38  BV=-0.06
    {50006,2816,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.47  BV=-0.15
    {50006,2944,1056, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.57  BV=-0.25
    {50006,3200,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.67  BV=-0.34
    {50006,3456,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.78  BV=-0.46
    {50006,3712,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.87  BV=-0.55
    {50006,3968,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.97  BV=-0.64
    {50006,4224,1048, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.08  BV=-0.76
    {58328,3968,1024, 0, 0, 0},  //TV = 4.10(3932 lines)  AV=2.00  SV=6.95  BV=-0.85
    {58328,4224,1032, 0, 0, 0},  //TV = 4.10(3932 lines)  AV=2.00  SV=7.06  BV=-0.96
    {66664,3968,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=6.97  BV=-1.06
    {66664,4224,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.06  BV=-1.15
    {66664,4480,1040, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.15  BV=-1.24
    {66664,4864,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.26  BV=-1.35
    {66664,5248,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.36  BV=-1.45
    {66664,5632,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.46  BV=-1.55
    {66664,6016,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.55  BV=-1.65
    {66664,6400,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.66  BV=-1.75
    {66664,6912,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.75  BV=-1.85
    {66664,7424,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.86  BV=-1.95
    {66664,7936,1024, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=7.95  BV=-2.05
    {66664,8448,1032, 0, 0, 0},  //TV = 3.91(4494 lines)  AV=2.00  SV=8.06  BV=-2.15
    {75001,8064,1032, 0, 0, 0},  //TV = 3.74(5056 lines)  AV=2.00  SV=7.99  BV=-2.25
    {83338,7808,1024, 0, 0, 0},  //TV = 3.58(5618 lines)  AV=2.00  SV=7.93  BV=-2.35
    {83338,8320,1032, 0, 0, 0},  //TV = 3.58(5618 lines)  AV=2.00  SV=8.03  BV=-2.45
    {91660,8192,1024, 0, 0, 0},  //TV = 3.45(6179 lines)  AV=2.00  SV=8.00  BV=-2.55
    {99996,7936,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=7.97  BV=-2.64
    {99996,8576,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.08  BV=-2.76
    {99996,9216,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.17  BV=-2.85
    {99996,9856,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.27  BV=-2.94
    {99996,10624,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.38  BV=-3.05
    {99996,11392,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.48  BV=-3.15
    {99996,12160,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.57  BV=-3.25
    {99996,13056,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.67  BV=-3.35
    {99996,13952,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.77  BV=-3.45
    {99996,14976,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.87  BV=-3.55
    {99996,16000,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.97  BV=-3.64
    {99996,16384,1024, 0, 0, 0},  /* TV = 3.32(6741 lines)  AV=2.00  SV=9.00  BV=-3.68 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
},
};

static strEvPline sAEScene5PLineTable_50Hz =
{
{
    {30,1152,1104, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.28  BV=11.75
    {30,1280,1056, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.37  BV=11.66
    {30,1408,1040, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.48  BV=11.54
    {30,1536,1032, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.60  BV=11.43
    {30,1664,1024, 0, 0, 0},  //TV = 15.02(2 lines)  AV=2.00  SV=5.70  BV=11.32
    {45,1152,1048, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.20  BV=11.24
    {45,1152,1120, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.30  BV=11.14
    {45,1280,1088, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.41  BV=11.03
    {45,1408,1040, 0, 0, 0},  //TV = 14.44(3 lines)  AV=2.00  SV=5.48  BV=10.96
    {60,1152,1056, 0, 0, 0},  /* TV = 14.02(4 lines)  AV=2.00  SV=5.21  BV=10.81 */
    {60,1152,1128, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.31  BV=10.72
    {60,1280,1072, 0, 0, 0},  //TV = 14.02(4 lines)  AV=2.00  SV=5.39  BV=10.64
    {75,1152,1032, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.18  BV=10.52
    {75,1152,1088, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.26  BV=10.45
    {75,1280,1056, 0, 0, 0},  //TV = 13.70(5 lines)  AV=2.00  SV=5.37  BV=10.34
    {90,1152,1040, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.19  BV=10.25
    {90,1280,1024, 0, 0, 0},  //TV = 13.44(6 lines)  AV=2.00  SV=5.32  BV=10.12
    {104,1152,1040, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.19  BV=10.04
    {104,1152,1104, 0, 0, 0},  //TV = 13.23(7 lines)  AV=2.00  SV=5.28  BV=9.95
    {119,1152,1032, 0, 0, 0},  //TV = 13.04(8 lines)  AV=2.00  SV=5.18  BV=9.86
    {134,1136,1024, 0, 0, 0},  /* TV = 12.87(9 lines)  AV=2.00  SV=5.15  BV=9.72 */
    {134,1152,1064, 0, 0, 0},  //TV = 12.87(9 lines)  AV=2.00  SV=5.23  BV=9.64
    {149,1152,1032, 0, 0, 0},  //TV = 12.71(10 lines)  AV=2.00  SV=5.18  BV=9.53
    {164,1136,1024, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.15  BV=9.42
    {164,1152,1088, 0, 0, 0},  //TV = 12.57(11 lines)  AV=2.00  SV=5.26  BV=9.32
    {179,1152,1040, 0, 0, 0},  /* TV = 12.45(12 lines)  AV=2.00  SV=5.19  BV=9.26 */
    {193,1152,1032, 0, 0, 0},  //TV = 12.34(13 lines)  AV=2.00  SV=5.18  BV=9.16
    {208,1152,1032, 0, 0, 0},  //TV = 12.23(14 lines)  AV=2.00  SV=5.18  BV=9.05
    {223,1152,1032, 0, 0, 0},  //TV = 12.13(15 lines)  AV=2.00  SV=5.18  BV=8.95
    {238,1152,1032, 0, 0, 0},  //TV = 12.04(16 lines)  AV=2.00  SV=5.18  BV=8.86
    {253,1152,1040, 0, 0, 0},  //TV = 11.95(17 lines)  AV=2.00  SV=5.19  BV=8.76
    {268,1152,1056, 0, 0, 0},  //TV = 11.87(18 lines)  AV=2.00  SV=5.21  BV=8.65
    {297,1152,1024, 0, 0, 0},  //TV = 11.72(20 lines)  AV=2.00  SV=5.17  BV=8.55
    {312,1152,1040, 0, 0, 0},  //TV = 11.65(21 lines)  AV=2.00  SV=5.19  BV=8.45
    {342,1152,1024, 0, 0, 0},  //TV = 11.51(23 lines)  AV=2.00  SV=5.17  BV=8.34
    {371,1136,1024, 0, 0, 0},  //TV = 11.40(25 lines)  AV=2.00  SV=5.15  BV=8.25
    {386,1152,1032, 0, 0, 0},  //TV = 11.34(26 lines)  AV=2.00  SV=5.18  BV=8.16
    {416,1152,1032, 0, 0, 0},  //TV = 11.23(28 lines)  AV=2.00  SV=5.18  BV=8.05
    {446,1152,1032, 0, 0, 0},  //TV = 11.13(30 lines)  AV=2.00  SV=5.18  BV=7.95
    {475,1152,1032, 0, 0, 0},  //TV = 11.04(32 lines)  AV=2.00  SV=5.18  BV=7.86
    {520,1136,1024, 0, 0, 0},  //TV = 10.91(35 lines)  AV=2.00  SV=5.15  BV=7.76
    {564,1136,1024, 0, 0, 0},  //TV = 10.79(38 lines)  AV=2.00  SV=5.15  BV=7.64
    {594,1152,1024, 0, 0, 0},  //TV = 10.72(40 lines)  AV=2.00  SV=5.17  BV=7.55
    {638,1152,1024, 0, 0, 0},  //TV = 10.61(43 lines)  AV=2.00  SV=5.17  BV=7.44
    {683,1152,1024, 0, 0, 0},  //TV = 10.52(46 lines)  AV=2.00  SV=5.17  BV=7.35
    {727,1152,1024, 0, 0, 0},  //TV = 10.43(49 lines)  AV=2.00  SV=5.17  BV=7.26
    {787,1152,1024, 0, 0, 0},  //TV = 10.31(53 lines)  AV=2.00  SV=5.17  BV=7.14
    {846,1136,1024, 0, 0, 0},  //TV = 10.21(57 lines)  AV=2.00  SV=5.15  BV=7.06
    {905,1136,1024, 0, 0, 0},  //TV = 10.11(61 lines)  AV=2.00  SV=5.15  BV=6.96
    {965,1152,1024, 0, 0, 0},  //TV = 10.02(65 lines)  AV=2.00  SV=5.17  BV=6.85
    {1039,1152,1024, 0, 0, 0},  //TV = 9.91(70 lines)  AV=2.00  SV=5.17  BV=6.74
    {1128,1136,1024, 0, 0, 0},  //TV = 9.79(76 lines)  AV=2.00  SV=5.15  BV=6.64
    {1202,1136,1024, 0, 0, 0},  //TV = 9.70(81 lines)  AV=2.00  SV=5.15  BV=6.55
    {1291,1136,1024, 0, 0, 0},  //TV = 9.60(87 lines)  AV=2.00  SV=5.15  BV=6.45
    {1380,1136,1024, 0, 0, 0},  //TV = 9.50(93 lines)  AV=2.00  SV=5.15  BV=6.35
    {1484,1136,1024, 0, 0, 0},  //TV = 9.40(100 lines)  AV=2.00  SV=5.15  BV=6.25
    {1588,1136,1024, 0, 0, 0},  //TV = 9.30(107 lines)  AV=2.00  SV=5.15  BV=6.15
    {1692,1136,1024, 0, 0, 0},  //TV = 9.21(114 lines)  AV=2.00  SV=5.15  BV=6.06
    {1810,1136,1024, 0, 0, 0},  //TV = 9.11(122 lines)  AV=2.00  SV=5.15  BV=5.96
    {1959,1136,1024, 0, 0, 0},  //TV = 9.00(132 lines)  AV=2.00  SV=5.15  BV=5.85
    {2107,1136,1024, 0, 0, 0},  //TV = 8.89(142 lines)  AV=2.00  SV=5.15  BV=5.74
    {2255,1136,1024, 0, 0, 0},  //TV = 8.79(152 lines)  AV=2.00  SV=5.15  BV=5.64
    {2389,1136,1024, 0, 0, 0},  //TV = 8.71(161 lines)  AV=2.00  SV=5.15  BV=5.56
    {2582,1136,1024, 0, 0, 0},  //TV = 8.60(174 lines)  AV=2.00  SV=5.15  BV=5.45
    {2774,1136,1024, 0, 0, 0},  //TV = 8.49(187 lines)  AV=2.00  SV=5.15  BV=5.34
    {2967,1136,1024, 0, 0, 0},  //TV = 8.40(200 lines)  AV=2.00  SV=5.15  BV=5.25
    {3175,1136,1024, 0, 0, 0},  //TV = 8.30(214 lines)  AV=2.00  SV=5.15  BV=5.15
    {3397,1136,1024, 0, 0, 0},  //TV = 8.20(229 lines)  AV=2.00  SV=5.15  BV=5.05
    {3635,1136,1024, 0, 0, 0},  //TV = 8.10(245 lines)  AV=2.00  SV=5.15  BV=4.95
    {3932,1136,1024, 0, 0, 0},  //TV = 7.99(265 lines)  AV=2.00  SV=5.15  BV=4.84
    {4199,1136,1024, 0, 0, 0},  //TV = 7.90(283 lines)  AV=2.00  SV=5.15  BV=4.75
    {4495,1136,1024, 0, 0, 0},  //TV = 7.80(303 lines)  AV=2.00  SV=5.15  BV=4.65
    {4822,1136,1024, 0, 0, 0},  //TV = 7.70(325 lines)  AV=2.00  SV=5.15  BV=4.55
    {5163,1136,1024, 0, 0, 0},  //TV = 7.60(348 lines)  AV=2.00  SV=5.15  BV=4.45
    {5563,1136,1024, 0, 0, 0},  //TV = 7.49(375 lines)  AV=2.00  SV=5.15  BV=4.34
    {5949,1136,1024, 0, 0, 0},  //TV = 7.39(401 lines)  AV=2.00  SV=5.15  BV=4.24
    {6379,1136,1024, 0, 0, 0},  //TV = 7.29(430 lines)  AV=2.00  SV=5.15  BV=4.14
    {6824,1136,1024, 0, 0, 0},  //TV = 7.20(460 lines)  AV=2.00  SV=5.15  BV=4.05
    {7314,1136,1024, 0, 0, 0},  //TV = 7.10(493 lines)  AV=2.00  SV=5.15  BV=3.95
    {7833,1136,1024, 0, 0, 0},  //TV = 7.00(528 lines)  AV=2.00  SV=5.15  BV=3.85
    {8397,1136,1024, 0, 0, 0},  //TV = 6.90(566 lines)  AV=2.00  SV=5.15  BV=3.75
    {8990,1136,1024, 0, 0, 0},  //TV = 6.80(606 lines)  AV=2.00  SV=5.15  BV=3.65
    {9628,1136,1024, 0, 0, 0},  //TV = 6.70(649 lines)  AV=2.00  SV=5.15  BV=3.55
    {9999,1152,1040, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.19  BV=3.45
    {9999,1152,1112, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.29  BV=3.36
    {9999,1280,1080, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.40  BV=3.25
    {9999,1408,1056, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.50  BV=3.14
    {9999,1536,1032, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.60  BV=3.05
    {9999,1664,1024, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.70  BV=2.94
    {9999,1664,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.79  BV=2.86
    {9999,1792,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.89  BV=2.75
    {9999,1920,1088, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=5.99  BV=2.65
    {9999,2176,1032, 0, 0, 0},  //TV = 6.64(674 lines)  AV=2.00  SV=6.10  BV=2.55
    {19997,1152,1040, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.19  BV=2.45
    {19997,1152,1112, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.29  BV=2.36
    {19997,1280,1080, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.40  BV=2.25
    {19997,1408,1048, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.49  BV=2.15
    {19997,1536,1032, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.60  BV=2.05
    {19997,1664,1024, 0, 0, 0},  //TV = 5.64(1348 lines)  AV=2.00  SV=5.70  BV=1.94
    {29995,1152,1048, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.20  BV=1.86
    {29995,1152,1128, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.31  BV=1.75
    {29995,1280,1088, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.41  BV=1.65
    {29995,1408,1064, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.51  BV=1.54
    {29995,1536,1040, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.61  BV=1.45
    {29995,1664,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.71  BV=1.35
    {29995,1792,1024, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.81  BV=1.25
    {29995,1920,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=5.92  BV=1.14
    {29995,2048,1032, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.01  BV=1.05
    {29995,2176,1040, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.11  BV=0.95
    {29995,2304,1056, 0, 0, 0},  //TV = 5.06(2022 lines)  AV=2.00  SV=6.21  BV=0.84
    {40008,1792,1088, 0, 0, 0},  //TV = 4.64(2697 lines)  AV=2.00  SV=5.89  BV=0.75
    {40008,1920,1088, 0, 0, 0},  //TV = 4.64(2697 lines)  AV=2.00  SV=5.99  BV=0.65
    {40008,2176,1024, 0, 0, 0},  //TV = 4.64(2697 lines)  AV=2.00  SV=6.09  BV=0.56
    {50006,1792,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.87  BV=0.45
    {50006,1920,1072, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=5.97  BV=0.35
    {50006,2048,1080, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.08  BV=0.24
    {50006,2304,1024, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.17  BV=0.15
    {50006,2432,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.27  BV=0.05
    {50006,2560,1064, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.38  BV=-0.06
    {50006,2816,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.47  BV=-0.15
    {50006,2944,1056, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.57  BV=-0.25
    {50006,3200,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.67  BV=-0.34
    {50006,3456,1040, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.78  BV=-0.46
    {50006,3712,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.87  BV=-0.55
    {50006,3968,1032, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=6.97  BV=-0.64
    {50006,4224,1048, 0, 0, 0},  //TV = 4.32(3371 lines)  AV=2.00  SV=7.08  BV=-0.76
    {60004,3840,1024, 0, 0, 0},  //TV = 4.06(4045 lines)  AV=2.00  SV=6.91  BV=-0.85
    {60004,4096,1032, 0, 0, 0},  //TV = 4.06(4045 lines)  AV=2.00  SV=7.01  BV=-0.95
    {70002,3712,1048, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=6.89  BV=-1.05
    {70002,3968,1048, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=6.99  BV=-1.15
    {70002,4352,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.09  BV=-1.25
    {70002,4608,1040, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.19  BV=-1.36
    {70002,4992,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.29  BV=-1.45
    {70002,5376,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.39  BV=-1.56
    {70002,5632,1040, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.48  BV=-1.65
    {70002,6144,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.58  BV=-1.75
    {70002,6528,1032, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.68  BV=-1.85
    {70002,7040,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.78  BV=-1.94
    {70002,7552,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.88  BV=-2.05
    {70002,8064,1032, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=7.99  BV=-2.15
    {70002,8704,1024, 0, 0, 0},  //TV = 3.84(4719 lines)  AV=2.00  SV=8.09  BV=-2.25
    {80000,8064,1032, 0, 0, 0},  //TV = 3.64(5393 lines)  AV=2.00  SV=7.99  BV=-2.34
    {89998,7680,1032, 0, 0, 0},  //TV = 3.47(6067 lines)  AV=2.00  SV=7.92  BV=-2.44
    {89998,8320,1024, 0, 0, 0},  //TV = 3.47(6067 lines)  AV=2.00  SV=8.02  BV=-2.55
    {99996,7936,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=7.97  BV=-2.64
    {99996,8576,1032, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.08  BV=-2.76
    {99996,9216,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.17  BV=-2.85
    {99996,9856,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.27  BV=-2.94
    {99996,10624,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.38  BV=-3.05
    {99996,11392,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.48  BV=-3.15
    {99996,12160,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.57  BV=-3.25
    {99996,13056,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.67  BV=-3.35
    {99996,13952,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.77  BV=-3.45
    {99996,14976,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.87  BV=-3.55
    {99996,16000,1024, 0, 0, 0},  //TV = 3.32(6741 lines)  AV=2.00  SV=8.97  BV=-3.64
    {99996,16384,1024, 0, 0, 0},  /* TV = 3.32(6741 lines)  AV=2.00  SV=9.00  BV=-3.68 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
},
};

static strAETable g_AE_SceneTable5 =
{
    AETABLE_SCENE_INDEX5, //eAETableID
    156, //u4TotalIndex
    -20, //i4StrobeTrigerBV
    118, //i4MaxBV
    -37, //i4MinBV
    90, //i4EffectiveMaxBV
    0, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_AUTO, //ISO
    sAEScene5PLineTable_60Hz,
    sAEScene5PLineTable_50Hz,
    NULL,
};
static strEvPline sAESceneReservePLineTable =
{
{
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
},
};
static strAETable g_AE_ReserveSceneTable =
{
	AETABLE_SCENE_MAX,    //eAETableID
	0,    //u4TotalIndex
	-20,    //u4StrobeTrigerBV
	0,    //i4MaxBV
	0,    //i4MinBV
	90,    //i4EffectiveMaxBV
	0,      //i4EffectiveMinBV
	LIB3A_AE_ISO_SPEED_AUTO, //ISO SPEED
	sAESceneReservePLineTable,
	sAESceneReservePLineTable,
	NULL,
};

static strAESceneMapping g_AEScenePLineMapping =
{
{
    {LIB3A_AE_SCENE_AUTO, {AETABLE_SCENE_INDEX5, AETABLE_SCENE_INDEX5, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX5}},
    {LIB3A_AE_SCENE_NIGHT, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX1, AETABLE_VIDEO_NIGHT, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX1}},
    {LIB3A_AE_SCENE_ACTION, {AETABLE_SCENE_INDEX5, AETABLE_SCENE_INDEX5, AETABLE_SCENE_INDEX5, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX2}},
    {LIB3A_AE_SCENE_BEACH, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX3, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX3}},
    {LIB3A_AE_SCENE_CANDLELIGHT, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX1, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX1}},
    {LIB3A_AE_SCENE_FIREWORKS, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX4, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX4}},
    {LIB3A_AE_SCENE_LANDSCAPE, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX3, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX3}},
    {LIB3A_AE_SCENE_PORTRAIT, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX2, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX2}},
    {LIB3A_AE_SCENE_NIGHT_PORTRAIT, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX1, AETABLE_VIDEO_NIGHT, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX1}},
    {LIB3A_AE_SCENE_PARTY, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX1, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX1}},
    {LIB3A_AE_SCENE_SNOW, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX3, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX3}},
    {LIB3A_AE_SCENE_SPORTS, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX2, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX2}},
    {LIB3A_AE_SCENE_STEADYPHOTO, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX2, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX2}},
    {LIB3A_AE_SCENE_SUNSET, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX3, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX3}},
    {LIB3A_AE_SCENE_THEATRE, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX1, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX1}},
    {LIB3A_AE_SCENE_ISO_ANTI_SHAKE, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX2, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX2}},
    {LIB3A_AE_SCENE_ISO100, {AETABLE_RPEVIEW_AUTO, AETABLE_CAPTURE_ISO100, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_CAPTURE_ISO100}},
    {LIB3A_AE_SCENE_ISO200, {AETABLE_RPEVIEW_AUTO, AETABLE_CAPTURE_ISO200, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_CAPTURE_ISO200}},
    {LIB3A_AE_SCENE_ISO400, {AETABLE_RPEVIEW_AUTO, AETABLE_CAPTURE_ISO400, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_CAPTURE_ISO400}},
    {LIB3A_AE_SCENE_ISO800, {AETABLE_RPEVIEW_AUTO, AETABLE_CAPTURE_ISO800, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_CAPTURE_ISO800}},
    {LIB3A_AE_SCENE_ISO1600, {AETABLE_RPEVIEW_AUTO, AETABLE_CAPTURE_ISO1600, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_CAPTURE_ISO1600}},
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO, AETABLE_CAPTURE_AUTO, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_CAPTURE_AUTO}},
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO, AETABLE_CAPTURE_AUTO, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_CAPTURE_AUTO}},
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO, AETABLE_CAPTURE_AUTO, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_CAPTURE_AUTO}},
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO, AETABLE_CAPTURE_AUTO, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_CAPTURE_AUTO}},
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO, AETABLE_CAPTURE_AUTO, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_CAPTURE_AUTO}},
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO, AETABLE_CAPTURE_AUTO, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_CAPTURE_AUTO}},
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO, AETABLE_CAPTURE_AUTO, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_CAPTURE_AUTO}},
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO, AETABLE_CAPTURE_AUTO, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_CAPTURE_AUTO}},
    {LIB3A_AE_SCENE_UNSUPPORTED, {AETABLE_RPEVIEW_AUTO, AETABLE_CAPTURE_AUTO, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_CAPTURE_AUTO}},
},
};

static strAEPLineTable g_strAEPlineTable =
{
{
// PLINE Table
    g_AE_PreviewAutoTable,
    g_AE_CaptureAutoTable,
    g_AE_VideoAutoTable,
    g_AE_Video1AutoTable,
    g_AE_Video2AutoTable,
    g_AE_Custom1AutoTable,
    g_AE_Custom2AutoTable,
    g_AE_Custom3AutoTable,
    g_AE_Custom4AutoTable,
    g_AE_Custom5AutoTable,
    g_AE_VideoNightTable,
    g_AE_CaptureISO100Table,
    g_AE_CaptureISO200Table,
    g_AE_CaptureISO400Table,
    g_AE_CaptureISO800Table,
    g_AE_CaptureISO1600Table,
    g_AE_StrobeTable,
    g_AE_SceneTable1,
    g_AE_SceneTable2,
    g_AE_SceneTable3,
    g_AE_SceneTable4,
    g_AE_SceneTable5,
    g_AE_ReserveSceneTable,
    g_AE_ReserveSceneTable,
    g_AE_ReserveSceneTable,
    g_AE_ReserveSceneTable,
    g_AE_ReserveSceneTable,
    g_AE_ReserveSceneTable,
    g_AE_ReserveSceneTable,
    g_AE_ReserveSceneTable,
    g_AE_ReserveSceneTable,
    g_AE_ReserveSceneTable,
    g_AE_ReserveSceneTable,
    g_AE_ReserveSceneTable,
    g_AE_ReserveSceneTable,
    g_AE_ReserveSceneTable,
    g_AE_ReserveSceneTable,
    g_AE_ReserveSceneTable,
    g_AE_ReserveSceneTable,
    g_AE_ReserveSceneTable,
    g_AE_ReserveSceneTable,
    g_AE_ReserveSceneTable,
    g_AE_ReserveSceneTable,
    g_AE_ReserveSceneTable,
    g_AE_ReserveSceneTable,
    g_AE_ReserveSceneTable,
    g_AE_ReserveSceneTable,
    g_AE_ReserveSceneTable,
    g_AE_ReserveSceneTable,
    g_AE_ReserveSceneTable,
},
};

static strAEPLineNumInfo g_strAEPreviewAutoPLineInfo =
{
    AETABLE_RPEVIEW_AUTO,
    90,
    0,
    {
        {1,30000,30,1136,1136},
        {2,30,30,1136,2048},
        {3,30,20,2048,2048},
        {4,20,20,2048,4096},
        {5,20,15,4096,4096},
        {6,15,15,4096,8192},
        {7,15,10,8192,8192},
        {8,10,10,8192,16384},
        {9,0,0,0,0},
        {10,0,0,0,0},
        {11,0,0,0,0},
        {12,0,0,0,0},
        {13,0,0,0,0},
        {14,0,0,0,0},
        {15,0,0,0,0},
    }
};

static strAEPLineNumInfo g_strAECaptureAutoPLineInfo =
{
    AETABLE_CAPTURE_AUTO,
    90,
    0,
    {
        {1,30000,30,1136,1136},
        {2,30,30,1136,2048},
        {3,30,20,2048,2048},
        {4,20,20,2048,4096},
        {5,20,15,4096,4096},
        {6,15,15,4096,8192},
        {7,15,10,8192,8192},
        {8,10,10,8192,16384},
        {9,0,0,0,0},
        {10,0,0,0,0},
        {11,0,0,0,0},
        {12,0,0,0,0},
        {13,0,0,0,0},
        {14,0,0,0,0},
        {15,0,0,0,0},
    }
};

static strAEPLineNumInfo g_strAEVideoAutoPLineInfo =
{
    AETABLE_VIDEO_AUTO,
    90,
    0,
    {
        {1,30000,30,1136,1136},
        {2,30,30,1136,2048},
        {3,30,20,2048,2048},
        {4,20,20,2048,4096},
        {5,20,15,4096,4096},
        {6,15,15,4096,8192},
        {7,15,10,8192,8192},
        {8,10,10,8192,16384},
        {9,0,0,0,0},
        {10,0,0,0,0},
        {11,0,0,0,0},
        {12,0,0,0,0},
        {13,0,0,0,0},
        {14,0,0,0,0},
        {15,0,0,0,0},
    }
};

static strAEPLineNumInfo g_strAEVideo1AutoPLineInfo =
{
    AETABLE_VIDEO1_AUTO,
    90,
    0,
    {
        {1,30000,30,1136,1136},
        {2,30,30,1136,16384},
        {3,0,0,0,0},
        {4,0,0,0,0},
        {5,0,0,0,0},
        {6,0,0,0,0},
        {7,0,0,0,0},
        {8,0,0,0,0},
        {9,0,0,0,0},
        {10,0,0,0,0},
        {11,0,0,0,0},
        {12,0,0,0,0},
        {13,0,0,0,0},
        {14,0,0,0,0},
        {15,0,0,0,0},
    }
};

static strAEPLineNumInfo g_strAEVideo2AutoPLineInfo =
{
    AETABLE_VIDEO2_AUTO,
    90,
    0,
    {
        {1,30000,20,1136,1136},
        {2,20,20,1136,16384},
        {3,0,0,0,0},
        {4,0,0,0,0},
        {5,0,0,0,0},
        {6,0,0,0,0},
        {7,0,0,0,0},
        {8,0,0,0,0},
        {9,0,0,0,0},
        {10,0,0,0,0},
        {11,0,0,0,0},
        {12,0,0,0,0},
        {13,0,0,0,0},
        {14,0,0,0,0},
        {15,0,0,0,0},
    }
};

static strAEPLineNumInfo g_strAECustom1AutoPLineInfo =
{
    AETABLE_CUSTOM1_AUTO,
    90,
    0,
    {
        {1,30000,30,1136,1136},
        {2,30,30,1136,2048},
        {3,30,20,2048,2048},
        {4,20,20,2048,4096},
        {5,20,15,4096,4096},
        {6,15,15,4096,8192},
        {7,15,10,8192,8192},
        {8,10,10,8192,16384},
        {9,0,0,0,0},
        {10,0,0,0,0},
        {11,0,0,0,0},
        {12,0,0,0,0},
        {13,0,0,0,0},
        {14,0,0,0,0},
        {15,0,0,0,0},
    }
};

static strAEPLineNumInfo g_strAECustom2AutoPLineInfo =
{
    AETABLE_CUSTOM2_AUTO,
    90,
    0,
    {
        {1,30000,30,1136,1136},
        {2,30,30,1136,2048},
        {3,30,20,2048,2048},
        {4,20,20,2048,4096},
        {5,20,15,4096,4096},
        {6,15,15,4096,8192},
        {7,15,10,8192,8192},
        {8,10,10,8192,16384},
        {9,0,0,0,0},
        {10,0,0,0,0},
        {11,0,0,0,0},
        {12,0,0,0,0},
        {13,0,0,0,0},
        {14,0,0,0,0},
        {15,0,0,0,0},
    }
};

static strAEPLineNumInfo g_strAECustom3AutoPLineInfo =
{
    AETABLE_CUSTOM3_AUTO,
    90,
    0,
    {
        {1,30000,30,1136,1136},
        {2,30,30,1136,2048},
        {3,30,20,2048,2048},
        {4,20,20,2048,4096},
        {5,20,15,4096,4096},
        {6,15,15,4096,8192},
        {7,15,10,8192,8192},
        {8,10,10,8192,16384},
        {9,0,0,0,0},
        {10,0,0,0,0},
        {11,0,0,0,0},
        {12,0,0,0,0},
        {13,0,0,0,0},
        {14,0,0,0,0},
        {15,0,0,0,0},
    }
};

static strAEPLineNumInfo g_strAECustom4AutoPLineInfo =
{
    AETABLE_CUSTOM4_AUTO,
    90,
    0,
    {
        {1,30000,30,1136,1136},
        {2,30,30,1136,2048},
        {3,30,20,2048,2048},
        {4,20,20,2048,4096},
        {5,20,15,4096,4096},
        {6,15,15,4096,8192},
        {7,15,10,8192,8192},
        {8,10,10,8192,16384},
        {9,0,0,0,0},
        {10,0,0,0,0},
        {11,0,0,0,0},
        {12,0,0,0,0},
        {13,0,0,0,0},
        {14,0,0,0,0},
        {15,0,0,0,0},
    }
};

static strAEPLineNumInfo g_strAECustom5AutoPLineInfo =
{
    AETABLE_CUSTOM5_AUTO,
    90,
    0,
    {
        {1,30000,30,1136,1136},
        {2,30,30,1136,2048},
        {3,30,20,2048,2048},
        {4,20,20,2048,4096},
        {5,20,15,4096,4096},
        {6,15,15,4096,8192},
        {7,15,10,8192,8192},
        {8,10,10,8192,16384},
        {9,0,0,0,0},
        {10,0,0,0,0},
        {11,0,0,0,0},
        {12,0,0,0,0},
        {13,0,0,0,0},
        {14,0,0,0,0},
        {15,0,0,0,0},
    }
};

static strAEPLineNumInfo g_strAEVideoNightPLineInfo =
{
    AETABLE_VIDEO_NIGHT,
    90,
    -10,
    {
        {1,30000,30,1136,1136},
        {2,30,30,1136,2048},
        {3,30,20,2048,2048},
        {4,20,20,2048,4096},
        {5,20,15,4096,4096},
        {6,15,15,4096,8192},
        {7,15,10,8192,8192},
        {8,10,10,8192,16384},
        {9,0,0,0,0},
        {10,0,0,0,0},
        {11,0,0,0,0},
        {12,0,0,0,0},
        {13,0,0,0,0},
        {14,0,0,0,0},
        {15,0,0,0,0},
    }
};

static strAEPLineNumInfo g_strAECaptureISO100PLineInfo =
{
    AETABLE_CAPTURE_ISO100,
    90,
    0,
    {
        {1,30000,8,1136,1136},
        {2,0,0,0,0},
        {3,0,0,0,0},
        {4,0,0,0,0},
        {5,0,0,0,0},
        {6,0,0,0,0},
        {7,0,0,0,0},
        {8,0,0,0,0},
        {9,0,0,0,0},
        {10,0,0,0,0},
        {11,0,0,0,0},
        {12,0,0,0,0},
        {13,0,0,0,0},
        {14,0,0,0,0},
        {15,0,0,0,0},
    }
};

static strAEPLineNumInfo g_strAECaptureISO200PLineInfo =
{
    AETABLE_CAPTURE_ISO200,
    90,
    0,
    {
        {1,30000,20,1136,1136},
        {2,20,20,1136,2048},
        {3,20,8,2048,2048},
        {4,0,0,0,0},
        {5,0,0,0,0},
        {6,0,0,0,0},
        {7,0,0,0,0},
        {8,0,0,0,0},
        {9,0,0,0,0},
        {10,0,0,0,0},
        {11,0,0,0,0},
        {12,0,0,0,0},
        {13,0,0,0,0},
        {14,0,0,0,0},
        {15,0,0,0,0},
    }
};

static strAEPLineNumInfo g_strAECaptureISO400PLineInfo =
{
    AETABLE_CAPTURE_ISO400,
    90,
    0,
    {
        {1,30000,20,1136,1136},
        {2,20,20,1136,4096},
        {3,20,8,4096,4096},
        {4,0,0,0,0},
        {5,0,0,0,0},
        {6,0,0,0,0},
        {7,0,0,0,0},
        {8,0,0,0,0},
        {9,0,0,0,0},
        {10,0,0,0,0},
        {11,0,0,0,0},
        {12,0,0,0,0},
        {13,0,0,0,0},
        {14,0,0,0,0},
        {15,0,0,0,0},
    }
};

static strAEPLineNumInfo g_strAECaptureISO800PLineInfo =
{
    AETABLE_CAPTURE_ISO800,
    90,
    0,
    {
        {1,30000,20,1136,1136},
        {2,20,20,1136,8192},
        {3,20,8,8192,8192},
        {4,0,0,0,0},
        {5,0,0,0,0},
        {6,0,0,0,0},
        {7,0,0,0,0},
        {8,0,0,0,0},
        {9,0,0,0,0},
        {10,0,0,0,0},
        {11,0,0,0,0},
        {12,0,0,0,0},
        {13,0,0,0,0},
        {14,0,0,0,0},
        {15,0,0,0,0},
    }
};

static strAEPLineNumInfo g_strAECaptureISO1600PLineInfo =
{
    AETABLE_CAPTURE_ISO1600,
    90,
    0,
    {
        {1,30000,20,1136,1136},
        {2,20,20,1136,16384},
        {3,20,8,16384,16384},
        {4,0,0,0,0},
        {5,0,0,0,0},
        {6,0,0,0,0},
        {7,0,0,0,0},
        {8,0,0,0,0},
        {9,0,0,0,0},
        {10,0,0,0,0},
        {11,0,0,0,0},
        {12,0,0,0,0},
        {13,0,0,0,0},
        {14,0,0,0,0},
        {15,0,0,0,0},
    }
};

static strAEPLineNumInfo g_strAEStrobePLineInfo =
{
    AETABLE_STROBE,
    90,
    0,
    {
        {1,30000,30,1136,1136},
        {2,30,30,1136,2048},
        {3,30,20,2048,2048},
        {4,20,20,2048,4096},
        {5,20,15,4096,4096},
        {6,15,15,4096,8192},
        {7,15,10,8192,8192},
        {8,0,0,0,0},
        {9,0,0,0,0},
        {10,0,0,0,0},
        {11,0,0,0,0},
        {12,0,0,0,0},
        {13,0,0,0,0},
        {14,0,0,0,0},
        {15,0,0,0,0},
    }
};

static strAEPLineNumInfo g_strAEScene1PLineInfo =
{
    AETABLE_SCENE_INDEX1,
    90,
    -10,
    {
        {1,30000,30,1136,1136},
        {2,30,30,1136,2048},
        {3,30,20,2048,2048},
        {4,20,20,2048,4096},
        {5,20,15,4096,4096},
        {6,15,15,4096,8192},
        {7,15,10,8192,8192},
        {8,10,10,8192,16384},
        {9,0,0,0,0},
        {10,0,0,0,0},
        {11,0,0,0,0},
        {12,0,0,0,0},
        {13,0,0,0,0},
        {14,0,0,0,0},
        {15,0,0,0,0},
    }
};

static strAEPLineNumInfo g_strAEScene2PLineInfo =
{
    AETABLE_SCENE_INDEX2,
    90,
    0,
    {
        {1,30000,30,1136,1136},
        {2,30,30,1136,2048},
        {3,30,20,2048,2048},
        {4,20,20,2048,4096},
        {5,20,15,4096,4096},
        {6,15,15,4096,8192},
        {7,15,10,8192,8192},
        {8,10,10,8192,16384},
        {9,0,0,0,0},
        {10,0,0,0,0},
        {11,0,0,0,0},
        {12,0,0,0,0},
        {13,0,0,0,0},
        {14,0,0,0,0},
        {15,0,0,0,0},
    }
};

static strAEPLineNumInfo g_strAEScene3PLineInfo =
{
    AETABLE_SCENE_INDEX3,
    90,
    0,
    {
        {1,30000,30,1136,1136},
        {2,30,30,1136,2048},
        {3,30,20,2048,2048},
        {4,20,20,2048,4096},
        {5,20,15,4096,4096},
        {6,15,15,4096,8192},
        {7,15,10,8192,8192},
        {8,10,10,8192,16384},
        {9,0,0,0,0},
        {10,0,0,0,0},
        {11,0,0,0,0},
        {12,0,0,0,0},
        {13,0,0,0,0},
        {14,0,0,0,0},
        {15,0,0,0,0},
    }
};

static strAEPLineNumInfo g_strAEScene4PLineInfo =
{
    AETABLE_SCENE_INDEX4,
    90,
    0,
    {
        {1,4,4,1136,1136},
        {2,0,0,0,0},
        {3,0,0,0,0},
        {4,0,0,0,0},
        {5,0,0,0,0},
        {6,0,0,0,0},
        {7,0,0,0,0},
        {8,0,0,0,0},
        {9,0,0,0,0},
        {10,0,0,0,0},
        {11,0,0,0,0},
        {12,0,0,0,0},
        {13,0,0,0,0},
        {14,0,0,0,0},
        {15,0,0,0,0},
    }
};

static strAEPLineNumInfo g_strAEScene5PLineInfo =
{
    AETABLE_SCENE_INDEX5,
    90,
    0,
    {
        {1,30000,30,1136,1136},
        {2,30,30,1136,2048},
        {3,30,20,2048,2048},
        {4,20,20,2048,4096},
        {5,20,15,4096,4096},
        {6,15,15,4096,8192},
        {7,15,10,8192,8192},
        {8,10,10,8192,16384},
        {9,0,0,0,0},
        {10,0,0,0,0},
        {11,0,0,0,0},
        {12,0,0,0,0},
        {13,0,0,0,0},
        {14,0,0,0,0},
        {15,0,0,0,0},
    }
};

static strAEPLineNumInfo g_strAENoScenePLineInfo =
{
    AETABLE_SCENE_MAX,
    90,
    0,
    {
        {1,0,0,0,0},
        {2,0,0,0,0},
        {3,0,0,0,0},
        {4,0,0,0,0},
        {5,0,0,0,0},
        {6,0,0,0,0},
        {7,0,0,0,0},
        {8,0,0,0,0},
        {9,0,0,0,0},
        {10,0,0,0,0},
        {11,0,0,0,0},
        {12,0,0,0,0},
        {13,0,0,0,0},
        {14,0,0,0,0},
        {15,0,0,0,0},
    }
};
static strAEPLineInfomation g_strAEPlineInfo =
{
    MTRUE,   // FALSE mean the sampling
    {
    // PLINE Info
        g_strAEPreviewAutoPLineInfo,
        g_strAECaptureAutoPLineInfo,
        g_strAEVideoAutoPLineInfo,
        g_strAEVideo1AutoPLineInfo,
        g_strAEVideo2AutoPLineInfo,
        g_strAECustom1AutoPLineInfo,
        g_strAECustom2AutoPLineInfo,
        g_strAECustom3AutoPLineInfo,
        g_strAECustom4AutoPLineInfo,
        g_strAECustom5AutoPLineInfo,
        g_strAEVideoNightPLineInfo,
        g_strAECaptureISO100PLineInfo,
        g_strAECaptureISO200PLineInfo,
        g_strAECaptureISO400PLineInfo,
        g_strAECaptureISO800PLineInfo,
        g_strAECaptureISO1600PLineInfo,
        g_strAEStrobePLineInfo,
        g_strAEScene1PLineInfo,
        g_strAEScene2PLineInfo,
        g_strAEScene3PLineInfo,
        g_strAEScene4PLineInfo,
        g_strAEScene5PLineInfo,
        g_strAENoScenePLineInfo,
        g_strAENoScenePLineInfo,
        g_strAENoScenePLineInfo,
        g_strAENoScenePLineInfo,
        g_strAENoScenePLineInfo,
        g_strAENoScenePLineInfo,
        g_strAENoScenePLineInfo,
        g_strAENoScenePLineInfo,
        g_strAENoScenePLineInfo,
        g_strAENoScenePLineInfo,
        g_strAENoScenePLineInfo,
        g_strAENoScenePLineInfo,
        g_strAENoScenePLineInfo,
        g_strAENoScenePLineInfo,
        g_strAENoScenePLineInfo,
        g_strAENoScenePLineInfo,
        g_strAENoScenePLineInfo,
        g_strAENoScenePLineInfo,
        g_strAENoScenePLineInfo,
        g_strAENoScenePLineInfo,
        g_strAENoScenePLineInfo,
        g_strAENoScenePLineInfo,
        g_strAENoScenePLineInfo,
        g_strAENoScenePLineInfo,
        g_strAENoScenePLineInfo,
        g_strAENoScenePLineInfo,
        g_strAENoScenePLineInfo,
        g_strAENoScenePLineInfo,
},
};

static strAEPLineGainList g_strAEGainList =
{
    0,
    {
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
        {0,100},
    }
};

static AE_PLINETABLE_T g_PlineTableMapping =
{
    g_AEScenePLineMapping,
    g_strAEPlineTable,
    g_strAEPlineInfo,
    g_strAEGainList
};
#endif

#if 0 //Save the P-line info to file for debug
MinGain,1136
MaxGain,16384
MiniISOGain,100
GainStepUnitInTotalRange,128
PreviewExposureLineUnit,14834
PreviewMaxFrameRate,26
VideoExposureLineUnit,14834
VideoMaxFrameRate,27
VideoToPreviewSensitivityRatio,1024
CaptureExposureLineUnit,14834
CaptureMaxFrameRate,26
CaptureToPreviewSensitivityRatio,1024
Video1ExposureLineUnit,14834
Video1MaxFrameRate,27
Video1ToPreviewSensitivityRatio,1024
Video2ExposureLineUnit,14834
Video2MaxFrameRate,26
Video2ToPreviewSensitivityRatio,1024
Custom1ExposureLineUnit,14834
Custom1MaxFrameRate,26
Custom1ToPreviewSensitivityRatio,1024
Custom2ExposureLineUnit,14834
Custom2MaxFrameRate,26
Custom2ToPreviewSensitivityRatio,1024
Custom3ExposureLineUnit,14834
Custom3MaxFrameRate,26
Custom3ToPreviewSensitivityRatio,1024
Custom4ExposureLineUnit,14834
Custom4MaxFrameRate,26
Custom4ToPreviewSensitivityRatio,1024
Custom5ExposureLineUnit,14834
Custom5MaxFrameRate,26
Custom5ToPreviewSensitivityRatio,1024
FocusLength,350
Fno,20

// Preview table -- Use preview sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_RPEVIEW_AUTO
90,0
1,30000,30,1136,1136
2,30,30,1136,2048
3,30,20,2048,2048
4,20,20,2048,4096
5,20,15,4096,4096
6,15,15,4096,8192
7,15,10,8192,8192
8,10,10,8192,16384
AETABLE_END

// Capture table -- Use capture sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_CAPTURE_AUTO
90,0
1,30000,30,1136,1136
2,30,30,1136,2048
3,30,20,2048,2048
4,20,20,2048,4096
5,20,15,4096,4096
6,15,15,4096,8192
7,15,10,8192,8192
8,10,10,8192,16384
AETABLE_END

// Video table -- Use video sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_VIDEO_AUTO
90,0
1,30000,30,1136,1136
2,30,30,1136,2048
3,30,20,2048,2048
4,20,20,2048,4096
5,20,15,4096,4096
6,15,15,4096,8192
7,15,10,8192,8192
8,10,10,8192,16384
AETABLE_END

// Video1 table -- Use video1 sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_VIDEO1_AUTO
90,0
1,30000,30,1136,1136
2,30,30,1136,16384
AETABLE_END

// Video2 table -- Use video2 sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_VIDEO2_AUTO
90,0
1,30000,20,1136,1136
2,20,20,1136,16384
AETABLE_END

// Custom1 table -- Use custom1 sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_CUSTOM1_AUTO
90,0
1,30000,30,1136,1136
2,30,30,1136,2048
3,30,20,2048,2048
4,20,20,2048,4096
5,20,15,4096,4096
6,15,15,4096,8192
7,15,10,8192,8192
8,10,10,8192,16384
AETABLE_END

// Custom2 table -- Use custom2 sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_CUSTOM2_AUTO
90,0
1,30000,30,1136,1136
2,30,30,1136,2048
3,30,20,2048,2048
4,20,20,2048,4096
5,20,15,4096,4096
6,15,15,4096,8192
7,15,10,8192,8192
8,10,10,8192,16384
AETABLE_END

// Custom3 table -- Use custom3 sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_CUSTOM3_AUTO
90,0
1,30000,30,1136,1136
2,30,30,1136,2048
3,30,20,2048,2048
4,20,20,2048,4096
5,20,15,4096,4096
6,15,15,4096,8192
7,15,10,8192,8192
8,10,10,8192,16384
AETABLE_END

// Custom4 table -- Use custom4 sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_CUSTOM4_AUTO
90,0
1,30000,30,1136,1136
2,30,30,1136,2048
3,30,20,2048,2048
4,20,20,2048,4096
5,20,15,4096,4096
6,15,15,4096,8192
7,15,10,8192,8192
8,10,10,8192,16384
AETABLE_END

// Custom5 table -- Use custom5 sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_CUSTOM5_AUTO
90,0
1,30000,30,1136,1136
2,30,30,1136,2048
3,30,20,2048,2048
4,20,20,2048,4096
5,20,15,4096,4096
6,15,15,4096,8192
7,15,10,8192,8192
8,10,10,8192,16384
AETABLE_END

// Video Night table -- Use video sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_VIDEO_NIGHT
90,-10
1,30000,30,1136,1136
2,30,30,1136,2048
3,30,20,2048,2048
4,20,20,2048,4096
5,20,15,4096,4096
6,15,15,4096,8192
7,15,10,8192,8192
8,10,10,8192,16384
AETABLE_END

// Capture ISO100 -- Use capture sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_CAPTURE_ISO100
90,0
1,30000,8,1136,1136
AETABLE_END

// Capture ISO200 -- Use capture sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_CAPTURE_ISO200
90,0
1,30000,20,1136,1136
2,20,20,1136,2048
3,20,8,2048,2048
AETABLE_END

// Capture ISO400 -- Use capture sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_CAPTURE_ISO400
90,0
1,30000,20,1136,1136
2,20,20,1136,4096
3,20,8,4096,4096
AETABLE_END

// Capture ISO800 -- Use capture sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_CAPTURE_ISO800
90,0
1,30000,20,1136,1136
2,20,20,1136,8192
3,20,8,8192,8192
AETABLE_END

// Capture ISO1600 -- Use capture sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_CAPTURE_ISO1600
90,0
1,30000,20,1136,1136
2,20,20,1136,16384
3,20,8,16384,16384
AETABLE_END

// Strobe -- Use capture sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_STROBE
90,0
1,30000,30,1136,1136
2,30,30,1136,2048
3,30,20,2048,2048
4,20,20,2048,4096
5,20,15,4096,4096
6,15,15,4096,8192
7,15,10,8192,8192
AETABLE_END

// Table1 PLine -- Use capture sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_SCENE_INDEX1
90,-10
1,30000,30,1136,1136
2,30,30,1136,2048
3,30,20,2048,2048
4,20,20,2048,4096
5,20,15,4096,4096
6,15,15,4096,8192
7,15,10,8192,8192
8,10,10,8192,16384
AETABLE_END

// Table2 PLine -- Use capture sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_SCENE_INDEX2
90,0
1,30000,30,1136,1136
2,30,30,1136,2048
3,30,20,2048,2048
4,20,20,2048,4096
5,20,15,4096,4096
6,15,15,4096,8192
7,15,10,8192,8192
8,10,10,8192,16384
AETABLE_END

// Table3 PLine -- Use capture sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_SCENE_INDEX3
90,0
1,30000,30,1136,1136
2,30,30,1136,2048
3,30,20,2048,2048
4,20,20,2048,4096
5,20,15,4096,4096
6,15,15,4096,8192
7,15,10,8192,8192
8,10,10,8192,16384
AETABLE_END

// Table4 PLine -- Use capture sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_SCENE_INDEX4
90,0
1,4,4,1136,1136
AETABLE_END

// Table5 PLine -- Use capture sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_SCENE_INDEX5
90,0
1,30000,30,1136,1136
2,30,30,1136,2048
3,30,20,2048,2048
4,20,20,2048,4096
5,20,15,4096,4096
6,15,15,4096,8192
7,15,10,8192,8192
8,10,10,8192,16384
AETABLE_END

AE_SCENE_AUTO, AETABLE_SCENE_INDEX5, AETABLE_SCENE_INDEX5, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX5
AE_SCENE_NIGHT, AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX1, AETABLE_VIDEO_NIGHT, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX1
AE_SCENE_ACTION, AETABLE_SCENE_INDEX5, AETABLE_SCENE_INDEX5, AETABLE_SCENE_INDEX5, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX2
AE_SCENE_BEACH, AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX3, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX3
AE_SCENE_CANDLELIGHT, AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX1, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX1
AE_SCENE_FIREWORKS, AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX4, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX4
AE_SCENE_LANDSCAPE, AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX3, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX3
AE_SCENE_PORTRAIT, AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX2, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX2
AE_SCENE_NIGHT_PORTRAIT, AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX1, AETABLE_VIDEO_NIGHT, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX1
AE_SCENE_PARTY, AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX1, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX1
AE_SCENE_SNOW, AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX3, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX3
AE_SCENE_SPORTS, AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX2, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX2
AE_SCENE_STEADYPHOTO, AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX2, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX2
AE_SCENE_SUNSET, AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX3, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX3
AE_SCENE_THEATRE, AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX1, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX1
AE_SCENE_ISO_ANTI_SHAKE, AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX2, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX2
AE_SCENE_ISO100, AETABLE_RPEVIEW_AUTO, AETABLE_CAPTURE_ISO100, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_CAPTURE_ISO100
AE_SCENE_ISO200, AETABLE_RPEVIEW_AUTO, AETABLE_CAPTURE_ISO200, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_CAPTURE_ISO200
AE_SCENE_ISO400, AETABLE_RPEVIEW_AUTO, AETABLE_CAPTURE_ISO400, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_CAPTURE_ISO400
AE_SCENE_ISO800, AETABLE_RPEVIEW_AUTO, AETABLE_CAPTURE_ISO800, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_CAPTURE_ISO800
AE_SCENE_ISO1600, AETABLE_RPEVIEW_AUTO, AETABLE_CAPTURE_ISO1600, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_CAPTURE_ISO1600
#endif
