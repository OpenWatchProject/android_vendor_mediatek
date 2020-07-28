/* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein
* is confidential and proprietary to MediaTek Inc. and/or its licensors.
* Without the prior written permission of MediaTek inc. and/or its licensors,
* any reproduction, modification, use or disclosure of MediaTek Software,
* and information contained herein, in whole or in part, shall be strictly prohibited.
*/
/* MediaTek Inc. (C) 2016. All rights reserved.
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
#ifndef _CAMERA_AE_PLINETABLE_OV13B10MIPIRAW_H
#define _CAMERA_AE_PLINETABLE_OV13B10MIPIRAW_H

#include <custom/aaa/AEPlinetable.h>
static strEvPline sPreviewPLineTable_60Hz =
{
{
    {95,1072,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.07  BV=10.57
    {95,1104,1024, 0, 0, 0},  /* TV = 13.36(9 lines)  AV=2.28  SV=5.11  BV=10.53 */
    {105,1056,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.04  BV=10.45
    {116,1040,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.02  BV=10.33
    {126,1024,1032, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.01  BV=10.22
    {126,1088,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.09  BV=10.14
    {137,1072,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.07  BV=10.04
    {147,1072,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.07  BV=9.94
    {158,1072,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.07  BV=9.84
    {179,1024,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.00  BV=9.72
    {189,1040,1024, 0, 0, 0},  //TV = 12.37(18 lines)  AV=2.28  SV=5.02  BV=9.62
    {200,1040,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.02  BV=9.54
    {210,1056,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.04  BV=9.45
    {231,1024,1032, 0, 0, 0},  //TV = 12.08(22 lines)  AV=2.28  SV=5.01  BV=9.34
    {242,1056,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.04  BV=9.24
    {263,1040,1024, 0, 0, 0},  //TV = 11.89(25 lines)  AV=2.28  SV=5.02  BV=9.15
    {284,1024,1032, 0, 0, 0},  //TV = 11.78(27 lines)  AV=2.28  SV=5.01  BV=9.05
    {305,1024,1032, 0, 0, 0},  //TV = 11.68(29 lines)  AV=2.28  SV=5.01  BV=8.94
    {326,1024,1032, 0, 0, 0},  //TV = 11.58(31 lines)  AV=2.28  SV=5.01  BV=8.85
    {347,1040,1024, 0, 0, 0},  //TV = 11.49(33 lines)  AV=2.28  SV=5.02  BV=8.75
    {368,1040,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.02  BV=8.66
    {399,1040,1024, 0, 0, 0},  //TV = 11.29(38 lines)  AV=2.28  SV=5.02  BV=8.54
    {431,1024,1032, 0, 0, 0},  //TV = 11.18(41 lines)  AV=2.28  SV=5.01  BV=8.44
    {462,1024,1032, 0, 0, 0},  //TV = 11.08(44 lines)  AV=2.28  SV=5.01  BV=8.34
    {494,1024,1032, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.01  BV=8.25
    {525,1040,1024, 0, 0, 0},  //TV = 10.90(50 lines)  AV=2.28  SV=5.02  BV=8.15
    {567,1024,1032, 0, 0, 0},  //TV = 10.78(54 lines)  AV=2.28  SV=5.01  BV=8.05
    {609,1024,1032, 0, 0, 0},  //TV = 10.68(58 lines)  AV=2.28  SV=5.01  BV=7.95
    {651,1024,1032, 0, 0, 0},  //TV = 10.59(62 lines)  AV=2.28  SV=5.01  BV=7.85
    {704,1024,1024, 0, 0, 0},  //TV = 10.47(67 lines)  AV=2.28  SV=5.00  BV=7.75
    {746,1040,1024, 0, 0, 0},  //TV = 10.39(71 lines)  AV=2.28  SV=5.02  BV=7.64
    {809,1024,1024, 0, 0, 0},  //TV = 10.27(77 lines)  AV=2.28  SV=5.00  BV=7.55
    {861,1024,1032, 0, 0, 0},  //TV = 10.18(82 lines)  AV=2.28  SV=5.01  BV=7.45
    {924,1024,1032, 0, 0, 0},  //TV = 10.08(88 lines)  AV=2.28  SV=5.01  BV=7.34
    {987,1024,1032, 0, 0, 0},  //TV = 9.98(94 lines)  AV=2.28  SV=5.01  BV=7.25
    {1071,1024,1024, 0, 0, 0},  //TV = 9.87(102 lines)  AV=2.28  SV=5.00  BV=7.14
    {1134,1024,1032, 0, 0, 0},  //TV = 9.78(108 lines)  AV=2.28  SV=5.01  BV=7.05
    {1218,1024,1032, 0, 0, 0},  //TV = 9.68(116 lines)  AV=2.28  SV=5.01  BV=6.95
    {1302,1024,1032, 0, 0, 0},  //TV = 9.59(124 lines)  AV=2.28  SV=5.01  BV=6.85
    {1407,1024,1024, 0, 0, 0},  //TV = 9.47(134 lines)  AV=2.28  SV=5.00  BV=6.75
    {1512,1024,1024, 0, 0, 0},  //TV = 9.37(144 lines)  AV=2.28  SV=5.00  BV=6.64
    {1617,1024,1024, 0, 0, 0},  //TV = 9.27(154 lines)  AV=2.28  SV=5.00  BV=6.55
    {1733,1024,1024, 0, 0, 0},  //TV = 9.17(165 lines)  AV=2.28  SV=5.00  BV=6.45
    {1859,1024,1024, 0, 0, 0},  //TV = 9.07(177 lines)  AV=2.28  SV=5.00  BV=6.35
    {1995,1024,1024, 0, 0, 0},  //TV = 8.97(190 lines)  AV=2.28  SV=5.00  BV=6.24
    {2132,1024,1024, 0, 0, 0},  //TV = 8.87(203 lines)  AV=2.28  SV=5.00  BV=6.15
    {2279,1024,1032, 0, 0, 0},  //TV = 8.78(217 lines)  AV=2.28  SV=5.01  BV=6.04
    {2447,1024,1024, 0, 0, 0},  //TV = 8.67(233 lines)  AV=2.28  SV=5.00  BV=5.95
    {2636,1024,1024, 0, 0, 0},  //TV = 8.57(251 lines)  AV=2.28  SV=5.00  BV=5.84
    {2814,1024,1024, 0, 0, 0},  //TV = 8.47(268 lines)  AV=2.28  SV=5.00  BV=5.75
    {3014,1024,1024, 0, 0, 0},  //TV = 8.37(287 lines)  AV=2.28  SV=5.00  BV=5.65
    {3245,1024,1024, 0, 0, 0},  //TV = 8.27(309 lines)  AV=2.28  SV=5.00  BV=5.54
    {3476,1024,1024, 0, 0, 0},  //TV = 8.17(331 lines)  AV=2.28  SV=5.00  BV=5.44
    {3717,1024,1024, 0, 0, 0},  //TV = 8.07(354 lines)  AV=2.28  SV=5.00  BV=5.35
    {3980,1024,1024, 0, 0, 0},  //TV = 7.97(379 lines)  AV=2.28  SV=5.00  BV=5.25
    {4263,1024,1024, 0, 0, 0},  //TV = 7.87(406 lines)  AV=2.28  SV=5.00  BV=5.15
    {4568,1024,1024, 0, 0, 0},  //TV = 7.77(435 lines)  AV=2.28  SV=5.00  BV=5.05
    {4914,1024,1024, 0, 0, 0},  //TV = 7.67(468 lines)  AV=2.28  SV=5.00  BV=4.94
    {5271,1024,1024, 0, 0, 0},  //TV = 7.57(502 lines)  AV=2.28  SV=5.00  BV=4.84
    {5639,1024,1024, 0, 0, 0},  //TV = 7.47(537 lines)  AV=2.28  SV=5.00  BV=4.75
    {6038,1024,1024, 0, 0, 0},  //TV = 7.37(575 lines)  AV=2.28  SV=5.00  BV=4.65
    {6468,1024,1024, 0, 0, 0},  //TV = 7.27(616 lines)  AV=2.28  SV=5.00  BV=4.55
    {6930,1024,1024, 0, 0, 0},  //TV = 7.17(660 lines)  AV=2.28  SV=5.00  BV=4.45
    {7424,1024,1024, 0, 0, 0},  //TV = 7.07(707 lines)  AV=2.28  SV=5.00  BV=4.35
    {8001,1024,1024, 0, 0, 0},  //TV = 6.97(762 lines)  AV=2.28  SV=5.00  BV=4.24
    {8338,1040,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.02  BV=4.16
    {8338,1120,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.13  BV=4.05
    {8338,1200,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.23  BV=3.95
    {8338,1296,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.34  BV=3.84
    {8338,1376,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.43  BV=3.75
    {8338,1488,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.54  BV=3.64
    {8338,1584,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.63  BV=3.55
    {8338,1696,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.73  BV=3.45
    {8338,1824,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.83  BV=3.35
    {8338,1952,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.93  BV=3.25
    {16664,1040,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.02  BV=3.16
    {16664,1120,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.13  BV=3.05
    {16664,1200,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.23  BV=2.95
    {16664,1296,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.34  BV=2.84
    {16664,1376,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.43  BV=2.76
    {16664,1488,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.54  BV=2.64
    {25001,1056,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.04  BV=2.55
    {25001,1136,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.15  BV=2.45
    {25001,1216,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.25  BV=2.35
    {25001,1296,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.34  BV=2.26
    {33327,1040,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.02  BV=2.16
    {33327,1120,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.13  BV=2.05
    {33327,1200,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.23  BV=1.95
    {33327,1296,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.34  BV=1.84
    {33327,1376,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.43  BV=1.76
    {33327,1488,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.54  BV=1.64
    {33327,1600,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.64  BV=1.54
    {33327,1696,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.73  BV=1.45
    {33327,1824,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.83  BV=1.35
    {33327,1952,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.93  BV=1.25
    {33327,2096,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.03  BV=1.15
    {33327,2240,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.13  BV=1.05
    {33327,2416,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.24  BV=0.94
    {33327,2576,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.33  BV=0.85
    {33327,2768,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.43  BV=0.75
    {33327,2976,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.54  BV=0.64
    {33327,3184,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.64  BV=0.55
    {33327,3408,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.73  BV=0.45
    {33327,3648,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.83  BV=0.35
    {33327,3936,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.94  BV=0.24
    {33327,4208,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.04  BV=0.14
    {33327,4512,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.14  BV=0.04
    {41664,3872,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=6.92  BV=-0.06
    {41664,4144,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=7.02  BV=-0.16
    {41664,4432,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=7.11  BV=-0.25
    {50001,3968,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=6.95  BV=-0.36
    {50001,4240,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=7.05  BV=-0.45
    {50001,4528,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=7.14  BV=-0.55
    {50001,4880,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=7.25  BV=-0.66
    {50001,5232,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=7.35  BV=-0.76
    {50001,5600,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=7.45  BV=-0.85
    {50001,6000,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=7.55  BV=-0.95
    {50001,6432,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=7.65  BV=-1.05
    {50001,6880,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=7.75  BV=-1.15
    {50001,7376,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=7.85  BV=-1.25
    {50001,7952,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=7.96  BV=-1.36
    {50001,8480,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=8.05  BV=-1.45
    {58328,7792,1024, 0, 0, 0},  //TV = 4.10(5555 lines)  AV=2.28  SV=7.93  BV=-1.55
    {58328,8352,1024, 0, 0, 0},  //TV = 4.10(5555 lines)  AV=2.28  SV=8.03  BV=-1.65
    {66665,7824,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=7.93  BV=-1.75
    {66665,8416,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=8.04  BV=-1.86
    {66665,9008,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=8.14  BV=-1.96
    {66665,9664,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=8.24  BV=-2.06
    {66665,10352,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=8.34  BV=-2.16
    {66665,11088,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=8.44  BV=-2.25
    {66665,11872,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=8.54  BV=-2.35
    {66665,12720,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=8.63  BV=-2.45
    {66665,13632,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=8.73  BV=-2.55
    {66665,14608,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=8.83  BV=-2.65
    {66665,15648,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=8.93  BV=-2.75
    {66665,15872,1024, 0, 0, 0},  /* TV = 3.91(6349 lines)  AV=2.28  SV=8.95  BV=-2.77 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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
    {95,1072,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.07  BV=10.57
    {95,1104,1024, 0, 0, 0},  /* TV = 13.36(9 lines)  AV=2.28  SV=5.11  BV=10.53 */
    {105,1056,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.04  BV=10.45
    {116,1040,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.02  BV=10.33
    {126,1024,1032, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.01  BV=10.22
    {126,1088,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.09  BV=10.14
    {137,1072,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.07  BV=10.04
    {147,1072,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.07  BV=9.94
    {158,1072,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.07  BV=9.84
    {179,1024,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.00  BV=9.72
    {189,1040,1024, 0, 0, 0},  //TV = 12.37(18 lines)  AV=2.28  SV=5.02  BV=9.62
    {200,1040,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.02  BV=9.54
    {210,1056,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.04  BV=9.45
    {231,1024,1032, 0, 0, 0},  //TV = 12.08(22 lines)  AV=2.28  SV=5.01  BV=9.34
    {242,1056,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.04  BV=9.24
    {263,1040,1024, 0, 0, 0},  //TV = 11.89(25 lines)  AV=2.28  SV=5.02  BV=9.15
    {284,1024,1032, 0, 0, 0},  //TV = 11.78(27 lines)  AV=2.28  SV=5.01  BV=9.05
    {305,1024,1032, 0, 0, 0},  //TV = 11.68(29 lines)  AV=2.28  SV=5.01  BV=8.94
    {326,1024,1032, 0, 0, 0},  //TV = 11.58(31 lines)  AV=2.28  SV=5.01  BV=8.85
    {347,1040,1024, 0, 0, 0},  //TV = 11.49(33 lines)  AV=2.28  SV=5.02  BV=8.75
    {368,1040,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.02  BV=8.66
    {399,1040,1024, 0, 0, 0},  //TV = 11.29(38 lines)  AV=2.28  SV=5.02  BV=8.54
    {431,1024,1032, 0, 0, 0},  //TV = 11.18(41 lines)  AV=2.28  SV=5.01  BV=8.44
    {462,1024,1032, 0, 0, 0},  //TV = 11.08(44 lines)  AV=2.28  SV=5.01  BV=8.34
    {494,1024,1032, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.01  BV=8.25
    {525,1040,1024, 0, 0, 0},  //TV = 10.90(50 lines)  AV=2.28  SV=5.02  BV=8.15
    {567,1024,1032, 0, 0, 0},  //TV = 10.78(54 lines)  AV=2.28  SV=5.01  BV=8.05
    {609,1024,1032, 0, 0, 0},  //TV = 10.68(58 lines)  AV=2.28  SV=5.01  BV=7.95
    {651,1024,1032, 0, 0, 0},  //TV = 10.59(62 lines)  AV=2.28  SV=5.01  BV=7.85
    {704,1024,1024, 0, 0, 0},  //TV = 10.47(67 lines)  AV=2.28  SV=5.00  BV=7.75
    {746,1040,1024, 0, 0, 0},  //TV = 10.39(71 lines)  AV=2.28  SV=5.02  BV=7.64
    {809,1024,1024, 0, 0, 0},  //TV = 10.27(77 lines)  AV=2.28  SV=5.00  BV=7.55
    {861,1024,1032, 0, 0, 0},  //TV = 10.18(82 lines)  AV=2.28  SV=5.01  BV=7.45
    {924,1024,1032, 0, 0, 0},  //TV = 10.08(88 lines)  AV=2.28  SV=5.01  BV=7.34
    {987,1024,1032, 0, 0, 0},  //TV = 9.98(94 lines)  AV=2.28  SV=5.01  BV=7.25
    {1071,1024,1024, 0, 0, 0},  //TV = 9.87(102 lines)  AV=2.28  SV=5.00  BV=7.14
    {1134,1024,1032, 0, 0, 0},  //TV = 9.78(108 lines)  AV=2.28  SV=5.01  BV=7.05
    {1218,1024,1032, 0, 0, 0},  //TV = 9.68(116 lines)  AV=2.28  SV=5.01  BV=6.95
    {1302,1024,1032, 0, 0, 0},  //TV = 9.59(124 lines)  AV=2.28  SV=5.01  BV=6.85
    {1407,1024,1024, 0, 0, 0},  //TV = 9.47(134 lines)  AV=2.28  SV=5.00  BV=6.75
    {1512,1024,1024, 0, 0, 0},  //TV = 9.37(144 lines)  AV=2.28  SV=5.00  BV=6.64
    {1617,1024,1024, 0, 0, 0},  //TV = 9.27(154 lines)  AV=2.28  SV=5.00  BV=6.55
    {1733,1024,1024, 0, 0, 0},  //TV = 9.17(165 lines)  AV=2.28  SV=5.00  BV=6.45
    {1859,1024,1024, 0, 0, 0},  //TV = 9.07(177 lines)  AV=2.28  SV=5.00  BV=6.35
    {1995,1024,1024, 0, 0, 0},  //TV = 8.97(190 lines)  AV=2.28  SV=5.00  BV=6.24
    {2132,1024,1024, 0, 0, 0},  //TV = 8.87(203 lines)  AV=2.28  SV=5.00  BV=6.15
    {2279,1024,1032, 0, 0, 0},  //TV = 8.78(217 lines)  AV=2.28  SV=5.01  BV=6.04
    {2447,1024,1024, 0, 0, 0},  //TV = 8.67(233 lines)  AV=2.28  SV=5.00  BV=5.95
    {2636,1024,1024, 0, 0, 0},  //TV = 8.57(251 lines)  AV=2.28  SV=5.00  BV=5.84
    {2814,1024,1024, 0, 0, 0},  //TV = 8.47(268 lines)  AV=2.28  SV=5.00  BV=5.75
    {3014,1024,1024, 0, 0, 0},  //TV = 8.37(287 lines)  AV=2.28  SV=5.00  BV=5.65
    {3245,1024,1024, 0, 0, 0},  //TV = 8.27(309 lines)  AV=2.28  SV=5.00  BV=5.54
    {3476,1024,1024, 0, 0, 0},  //TV = 8.17(331 lines)  AV=2.28  SV=5.00  BV=5.44
    {3717,1024,1024, 0, 0, 0},  //TV = 8.07(354 lines)  AV=2.28  SV=5.00  BV=5.35
    {3980,1024,1024, 0, 0, 0},  //TV = 7.97(379 lines)  AV=2.28  SV=5.00  BV=5.25
    {4263,1024,1024, 0, 0, 0},  //TV = 7.87(406 lines)  AV=2.28  SV=5.00  BV=5.15
    {4568,1024,1024, 0, 0, 0},  //TV = 7.77(435 lines)  AV=2.28  SV=5.00  BV=5.05
    {4914,1024,1024, 0, 0, 0},  //TV = 7.67(468 lines)  AV=2.28  SV=5.00  BV=4.94
    {5271,1024,1024, 0, 0, 0},  //TV = 7.57(502 lines)  AV=2.28  SV=5.00  BV=4.84
    {5639,1024,1024, 0, 0, 0},  //TV = 7.47(537 lines)  AV=2.28  SV=5.00  BV=4.75
    {6038,1024,1024, 0, 0, 0},  //TV = 7.37(575 lines)  AV=2.28  SV=5.00  BV=4.65
    {6468,1024,1024, 0, 0, 0},  //TV = 7.27(616 lines)  AV=2.28  SV=5.00  BV=4.55
    {6930,1024,1024, 0, 0, 0},  //TV = 7.17(660 lines)  AV=2.28  SV=5.00  BV=4.45
    {7424,1024,1024, 0, 0, 0},  //TV = 7.07(707 lines)  AV=2.28  SV=5.00  BV=4.35
    {8001,1024,1024, 0, 0, 0},  //TV = 6.97(762 lines)  AV=2.28  SV=5.00  BV=4.24
    {8568,1024,1024, 0, 0, 0},  //TV = 6.87(816 lines)  AV=2.28  SV=5.00  BV=4.14
    {9177,1024,1024, 0, 0, 0},  //TV = 6.77(874 lines)  AV=2.28  SV=5.00  BV=4.04
    {9839,1024,1024, 0, 0, 0},  //TV = 6.67(937 lines)  AV=2.28  SV=5.00  BV=3.94
    {9996,1072,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.07  BV=3.85
    {9996,1152,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.17  BV=3.75
    {9996,1232,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.27  BV=3.65
    {9996,1328,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.38  BV=3.54
    {9996,1424,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.48  BV=3.44
    {9996,1520,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.57  BV=3.35
    {9996,1632,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.67  BV=3.25
    {9996,1744,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.77  BV=3.15
    {9996,1872,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.87  BV=3.05
    {9996,2000,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.97  BV=2.95
    {20003,1072,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.07  BV=2.85
    {20003,1152,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.17  BV=2.75
    {20003,1232,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.27  BV=2.65
    {20003,1328,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.38  BV=2.54
    {20003,1424,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.48  BV=2.44
    {20003,1520,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.57  BV=2.35
    {29999,1088,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.09  BV=2.25
    {29999,1168,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.19  BV=2.14
    {29999,1248,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.29  BV=2.05
    {29999,1344,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.39  BV=1.94
    {29999,1440,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.49  BV=1.84
    {29999,1536,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.58  BV=1.75
    {29999,1648,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.69  BV=1.65
    {29999,1776,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.79  BV=1.54
    {29999,1888,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.88  BV=1.45
    {29999,2032,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.99  BV=1.35
    {29999,2176,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.09  BV=1.25
    {29999,2336,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.19  BV=1.14
    {29999,2496,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.29  BV=1.05
    {29999,2688,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.39  BV=0.94
    {29999,2864,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.48  BV=0.85
    {29999,3072,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.58  BV=0.75
    {29999,3312,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.69  BV=0.64
    {29999,3536,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.79  BV=0.55
    {29999,3792,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.89  BV=0.45
    {29999,4048,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.98  BV=0.35
    {29999,4336,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.08  BV=0.25
    {29999,4672,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.19  BV=0.14
    {40005,3760,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=6.88  BV=0.04
    {40005,4032,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=6.98  BV=-0.06
    {40005,4320,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=7.08  BV=-0.16
    {50001,3696,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=6.85  BV=-0.25
    {50001,3968,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=6.95  BV=-0.36
    {50001,4240,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=7.05  BV=-0.45
    {50001,4528,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=7.14  BV=-0.55
    {50001,4880,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=7.25  BV=-0.66
    {50001,5232,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=7.35  BV=-0.76
    {50001,5600,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=7.45  BV=-0.85
    {50001,6000,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=7.55  BV=-0.95
    {50001,6432,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=7.65  BV=-1.05
    {50001,6880,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=7.75  BV=-1.15
    {50001,7376,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=7.85  BV=-1.25
    {50001,7952,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=7.96  BV=-1.36
    {50001,8480,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=8.05  BV=-1.45
    {59997,7568,1024, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=7.89  BV=-1.55
    {59997,8112,1024, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=7.99  BV=-1.65
    {59997,8688,1024, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=8.08  BV=-1.75
    {70004,8016,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=7.97  BV=-1.86
    {70004,8592,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=8.07  BV=-1.96
    {70004,9200,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=8.17  BV=-2.06
    {70004,9856,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=8.27  BV=-2.16
    {70004,10560,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=8.37  BV=-2.25
    {70004,11312,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=8.47  BV=-2.35
    {70004,12112,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=8.56  BV=-2.45
    {70004,12976,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=8.66  BV=-2.55
    {70004,13904,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=8.76  BV=-2.65
    {70004,14896,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=8.86  BV=-2.75
    {70004,15872,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=8.95  BV=-2.84
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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
    136, //u4TotalIndex
    20, //i4StrobeTrigerBV
    106, //i4MaxBV
    -29, //i4MinBV
    90, //i4EffectiveMaxBV
    -50, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_AUTO, //ISO
    sPreviewPLineTable_60Hz,
    sPreviewPLineTable_50Hz,
    NULL,
};
static strEvPline sCapturePLineTable_60Hz =
{
{
    {95,1072,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.07  BV=10.57
    {95,1104,1024, 0, 0, 0},  /* TV = 13.36(9 lines)  AV=2.28  SV=5.11  BV=10.53 */
    {105,1056,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.04  BV=10.45
    {116,1040,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.02  BV=10.33
    {126,1024,1032, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.01  BV=10.22
    {126,1088,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.09  BV=10.14
    {137,1072,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.07  BV=10.04
    {147,1072,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.07  BV=9.94
    {158,1072,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.07  BV=9.84
    {179,1024,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.00  BV=9.72
    {189,1040,1024, 0, 0, 0},  //TV = 12.37(18 lines)  AV=2.28  SV=5.02  BV=9.62
    {200,1040,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.02  BV=9.54
    {210,1056,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.04  BV=9.45
    {231,1024,1032, 0, 0, 0},  //TV = 12.08(22 lines)  AV=2.28  SV=5.01  BV=9.34
    {242,1056,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.04  BV=9.24
    {263,1040,1024, 0, 0, 0},  //TV = 11.89(25 lines)  AV=2.28  SV=5.02  BV=9.15
    {284,1024,1032, 0, 0, 0},  //TV = 11.78(27 lines)  AV=2.28  SV=5.01  BV=9.05
    {305,1024,1032, 0, 0, 0},  //TV = 11.68(29 lines)  AV=2.28  SV=5.01  BV=8.94
    {326,1024,1032, 0, 0, 0},  //TV = 11.58(31 lines)  AV=2.28  SV=5.01  BV=8.85
    {347,1040,1024, 0, 0, 0},  //TV = 11.49(33 lines)  AV=2.28  SV=5.02  BV=8.75
    {368,1040,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.02  BV=8.66
    {399,1040,1024, 0, 0, 0},  //TV = 11.29(38 lines)  AV=2.28  SV=5.02  BV=8.54
    {431,1024,1032, 0, 0, 0},  //TV = 11.18(41 lines)  AV=2.28  SV=5.01  BV=8.44
    {462,1024,1032, 0, 0, 0},  //TV = 11.08(44 lines)  AV=2.28  SV=5.01  BV=8.34
    {494,1024,1032, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.01  BV=8.25
    {525,1040,1024, 0, 0, 0},  //TV = 10.90(50 lines)  AV=2.28  SV=5.02  BV=8.15
    {567,1024,1032, 0, 0, 0},  //TV = 10.78(54 lines)  AV=2.28  SV=5.01  BV=8.05
    {609,1024,1032, 0, 0, 0},  //TV = 10.68(58 lines)  AV=2.28  SV=5.01  BV=7.95
    {651,1024,1032, 0, 0, 0},  //TV = 10.59(62 lines)  AV=2.28  SV=5.01  BV=7.85
    {704,1024,1024, 0, 0, 0},  //TV = 10.47(67 lines)  AV=2.28  SV=5.00  BV=7.75
    {746,1040,1024, 0, 0, 0},  //TV = 10.39(71 lines)  AV=2.28  SV=5.02  BV=7.64
    {809,1024,1024, 0, 0, 0},  //TV = 10.27(77 lines)  AV=2.28  SV=5.00  BV=7.55
    {861,1024,1032, 0, 0, 0},  //TV = 10.18(82 lines)  AV=2.28  SV=5.01  BV=7.45
    {924,1024,1032, 0, 0, 0},  //TV = 10.08(88 lines)  AV=2.28  SV=5.01  BV=7.34
    {987,1024,1032, 0, 0, 0},  //TV = 9.98(94 lines)  AV=2.28  SV=5.01  BV=7.25
    {1071,1024,1024, 0, 0, 0},  //TV = 9.87(102 lines)  AV=2.28  SV=5.00  BV=7.14
    {1134,1024,1032, 0, 0, 0},  //TV = 9.78(108 lines)  AV=2.28  SV=5.01  BV=7.05
    {1218,1024,1032, 0, 0, 0},  //TV = 9.68(116 lines)  AV=2.28  SV=5.01  BV=6.95
    {1302,1024,1032, 0, 0, 0},  //TV = 9.59(124 lines)  AV=2.28  SV=5.01  BV=6.85
    {1407,1024,1024, 0, 0, 0},  //TV = 9.47(134 lines)  AV=2.28  SV=5.00  BV=6.75
    {1512,1024,1024, 0, 0, 0},  //TV = 9.37(144 lines)  AV=2.28  SV=5.00  BV=6.64
    {1617,1024,1024, 0, 0, 0},  //TV = 9.27(154 lines)  AV=2.28  SV=5.00  BV=6.55
    {1733,1024,1024, 0, 0, 0},  //TV = 9.17(165 lines)  AV=2.28  SV=5.00  BV=6.45
    {1859,1024,1024, 0, 0, 0},  //TV = 9.07(177 lines)  AV=2.28  SV=5.00  BV=6.35
    {1995,1024,1024, 0, 0, 0},  //TV = 8.97(190 lines)  AV=2.28  SV=5.00  BV=6.24
    {2132,1024,1024, 0, 0, 0},  //TV = 8.87(203 lines)  AV=2.28  SV=5.00  BV=6.15
    {2279,1024,1032, 0, 0, 0},  //TV = 8.78(217 lines)  AV=2.28  SV=5.01  BV=6.04
    {2447,1024,1024, 0, 0, 0},  //TV = 8.67(233 lines)  AV=2.28  SV=5.00  BV=5.95
    {2636,1024,1024, 0, 0, 0},  //TV = 8.57(251 lines)  AV=2.28  SV=5.00  BV=5.84
    {2814,1024,1024, 0, 0, 0},  //TV = 8.47(268 lines)  AV=2.28  SV=5.00  BV=5.75
    {3014,1024,1024, 0, 0, 0},  //TV = 8.37(287 lines)  AV=2.28  SV=5.00  BV=5.65
    {3245,1024,1024, 0, 0, 0},  //TV = 8.27(309 lines)  AV=2.28  SV=5.00  BV=5.54
    {3476,1024,1024, 0, 0, 0},  //TV = 8.17(331 lines)  AV=2.28  SV=5.00  BV=5.44
    {3717,1024,1024, 0, 0, 0},  //TV = 8.07(354 lines)  AV=2.28  SV=5.00  BV=5.35
    {3980,1024,1024, 0, 0, 0},  //TV = 7.97(379 lines)  AV=2.28  SV=5.00  BV=5.25
    {4263,1024,1024, 0, 0, 0},  //TV = 7.87(406 lines)  AV=2.28  SV=5.00  BV=5.15
    {4568,1024,1024, 0, 0, 0},  //TV = 7.77(435 lines)  AV=2.28  SV=5.00  BV=5.05
    {4914,1024,1024, 0, 0, 0},  //TV = 7.67(468 lines)  AV=2.28  SV=5.00  BV=4.94
    {5271,1024,1024, 0, 0, 0},  //TV = 7.57(502 lines)  AV=2.28  SV=5.00  BV=4.84
    {5639,1024,1024, 0, 0, 0},  //TV = 7.47(537 lines)  AV=2.28  SV=5.00  BV=4.75
    {6038,1024,1024, 0, 0, 0},  //TV = 7.37(575 lines)  AV=2.28  SV=5.00  BV=4.65
    {6468,1024,1024, 0, 0, 0},  //TV = 7.27(616 lines)  AV=2.28  SV=5.00  BV=4.55
    {6930,1024,1024, 0, 0, 0},  //TV = 7.17(660 lines)  AV=2.28  SV=5.00  BV=4.45
    {7424,1024,1024, 0, 0, 0},  //TV = 7.07(707 lines)  AV=2.28  SV=5.00  BV=4.35
    {8001,1024,1024, 0, 0, 0},  //TV = 6.97(762 lines)  AV=2.28  SV=5.00  BV=4.24
    {8338,1040,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.02  BV=4.16
    {8338,1120,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.13  BV=4.05
    {8338,1200,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.23  BV=3.95
    {8338,1296,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.34  BV=3.84
    {8338,1376,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.43  BV=3.75
    {8338,1488,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.54  BV=3.64
    {8338,1584,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.63  BV=3.55
    {8338,1696,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.73  BV=3.45
    {8338,1824,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.83  BV=3.35
    {8338,1952,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.93  BV=3.25
    {16664,1040,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.02  BV=3.16
    {16664,1120,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.13  BV=3.05
    {16664,1200,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.23  BV=2.95
    {16664,1296,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.34  BV=2.84
    {16664,1376,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.43  BV=2.76
    {16664,1488,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.54  BV=2.64
    {25001,1056,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.04  BV=2.55
    {25001,1136,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.15  BV=2.45
    {25001,1216,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.25  BV=2.35
    {25001,1296,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.34  BV=2.26
    {33327,1040,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.02  BV=2.16
    {33327,1120,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.13  BV=2.05
    {33327,1200,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.23  BV=1.95
    {33327,1296,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.34  BV=1.84
    {33327,1376,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.43  BV=1.76
    {33327,1488,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.54  BV=1.64
    {33327,1600,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.64  BV=1.54
    {33327,1696,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.73  BV=1.45
    {33327,1824,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.83  BV=1.35
    {33327,1952,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.93  BV=1.25
    {33327,2096,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.03  BV=1.15
    {33327,2240,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.13  BV=1.05
    {33327,2416,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.24  BV=0.94
    {33327,2576,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.33  BV=0.85
    {33327,2768,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.43  BV=0.75
    {33327,2976,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.54  BV=0.64
    {33327,3184,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.64  BV=0.55
    {33327,3408,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.73  BV=0.45
    {33327,3648,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.83  BV=0.35
    {33327,3936,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.94  BV=0.24
    {33327,4208,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.04  BV=0.14
    {33327,4512,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.14  BV=0.04
    {41664,3872,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=6.92  BV=-0.06
    {41664,4128,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=7.01  BV=-0.15
    {41664,4448,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=7.12  BV=-0.26
    {41664,4752,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=7.21  BV=-0.35
    {41664,5088,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=7.31  BV=-0.45
    {41664,5456,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=7.41  BV=-0.55
    {41664,5840,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=7.51  BV=-0.65
    {41664,6256,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=7.61  BV=-0.75
    {41664,6736,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=7.72  BV=-0.86
    {41664,7216,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=7.82  BV=-0.96
    {41664,7728,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=7.92  BV=-1.06
    {41664,8304,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=8.02  BV=-1.16
    {41664,8896,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=8.12  BV=-1.26
    {50001,7952,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=7.96  BV=-1.36
    {50001,8512,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=8.06  BV=-1.46
    {58328,7824,1024, 0, 0, 0},  //TV = 4.10(5555 lines)  AV=2.28  SV=7.93  BV=-1.56
    {58328,8384,1024, 0, 0, 0},  //TV = 4.10(5555 lines)  AV=2.28  SV=8.03  BV=-1.66
    {66665,7856,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=7.94  BV=-1.76
    {66665,8416,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=8.04  BV=-1.86
    {66665,9008,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=8.14  BV=-1.96
    {66665,9664,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=8.24  BV=-2.06
    {66665,10304,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=8.33  BV=-2.15
    {75002,9824,1024, 0, 0, 0},  //TV = 3.74(7143 lines)  AV=2.28  SV=8.26  BV=-2.25
    {75002,10528,1024, 0, 0, 0},  //TV = 3.74(7143 lines)  AV=2.28  SV=8.36  BV=-2.35
    {83328,10224,1024, 0, 0, 0},  //TV = 3.59(7936 lines)  AV=2.28  SV=8.32  BV=-2.46
    {91665,9888,1024, 0, 0, 0},  //TV = 3.45(8730 lines)  AV=2.28  SV=8.27  BV=-2.55
    {91665,10672,1024, 0, 0, 0},  //TV = 3.45(8730 lines)  AV=2.28  SV=8.38  BV=-2.66
    {99992,10448,1024, 0, 0, 0},  //TV = 3.32(9523 lines)  AV=2.28  SV=8.35  BV=-2.75
    {99992,11200,1024, 0, 0, 0},  //TV = 3.32(9523 lines)  AV=2.28  SV=8.45  BV=-2.85
    {99992,12000,1024, 0, 0, 0},  //TV = 3.32(9523 lines)  AV=2.28  SV=8.55  BV=-2.95
    {99992,12848,1024, 0, 0, 0},  //TV = 3.32(9523 lines)  AV=2.28  SV=8.65  BV=-3.05
    {99992,13776,1024, 0, 0, 0},  //TV = 3.32(9523 lines)  AV=2.28  SV=8.75  BV=-3.15
    {99992,14752,1024, 0, 0, 0},  //TV = 3.32(9523 lines)  AV=2.28  SV=8.85  BV=-3.25
    {99992,15808,1024, 0, 0, 0},  //TV = 3.32(9523 lines)  AV=2.28  SV=8.95  BV=-3.35
    {99992,15872,1024, 0, 0, 0},  /* TV = 3.32(9523 lines)  AV=2.28  SV=8.95  BV=-3.36 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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
    {95,1072,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.07  BV=10.57
    {95,1104,1024, 0, 0, 0},  /* TV = 13.36(9 lines)  AV=2.28  SV=5.11  BV=10.53 */
    {105,1056,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.04  BV=10.45
    {116,1040,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.02  BV=10.33
    {126,1024,1032, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.01  BV=10.22
    {126,1088,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.09  BV=10.14
    {137,1072,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.07  BV=10.04
    {147,1072,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.07  BV=9.94
    {158,1072,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.07  BV=9.84
    {179,1024,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.00  BV=9.72
    {189,1040,1024, 0, 0, 0},  //TV = 12.37(18 lines)  AV=2.28  SV=5.02  BV=9.62
    {200,1040,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.02  BV=9.54
    {210,1056,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.04  BV=9.45
    {231,1024,1032, 0, 0, 0},  //TV = 12.08(22 lines)  AV=2.28  SV=5.01  BV=9.34
    {242,1056,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.04  BV=9.24
    {263,1040,1024, 0, 0, 0},  //TV = 11.89(25 lines)  AV=2.28  SV=5.02  BV=9.15
    {284,1024,1032, 0, 0, 0},  //TV = 11.78(27 lines)  AV=2.28  SV=5.01  BV=9.05
    {305,1024,1032, 0, 0, 0},  //TV = 11.68(29 lines)  AV=2.28  SV=5.01  BV=8.94
    {326,1024,1032, 0, 0, 0},  //TV = 11.58(31 lines)  AV=2.28  SV=5.01  BV=8.85
    {347,1040,1024, 0, 0, 0},  //TV = 11.49(33 lines)  AV=2.28  SV=5.02  BV=8.75
    {368,1040,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.02  BV=8.66
    {399,1040,1024, 0, 0, 0},  //TV = 11.29(38 lines)  AV=2.28  SV=5.02  BV=8.54
    {431,1024,1032, 0, 0, 0},  //TV = 11.18(41 lines)  AV=2.28  SV=5.01  BV=8.44
    {462,1024,1032, 0, 0, 0},  //TV = 11.08(44 lines)  AV=2.28  SV=5.01  BV=8.34
    {494,1024,1032, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.01  BV=8.25
    {525,1040,1024, 0, 0, 0},  //TV = 10.90(50 lines)  AV=2.28  SV=5.02  BV=8.15
    {567,1024,1032, 0, 0, 0},  //TV = 10.78(54 lines)  AV=2.28  SV=5.01  BV=8.05
    {609,1024,1032, 0, 0, 0},  //TV = 10.68(58 lines)  AV=2.28  SV=5.01  BV=7.95
    {651,1024,1032, 0, 0, 0},  //TV = 10.59(62 lines)  AV=2.28  SV=5.01  BV=7.85
    {704,1024,1024, 0, 0, 0},  //TV = 10.47(67 lines)  AV=2.28  SV=5.00  BV=7.75
    {746,1040,1024, 0, 0, 0},  //TV = 10.39(71 lines)  AV=2.28  SV=5.02  BV=7.64
    {809,1024,1024, 0, 0, 0},  //TV = 10.27(77 lines)  AV=2.28  SV=5.00  BV=7.55
    {861,1024,1032, 0, 0, 0},  //TV = 10.18(82 lines)  AV=2.28  SV=5.01  BV=7.45
    {924,1024,1032, 0, 0, 0},  //TV = 10.08(88 lines)  AV=2.28  SV=5.01  BV=7.34
    {987,1024,1032, 0, 0, 0},  //TV = 9.98(94 lines)  AV=2.28  SV=5.01  BV=7.25
    {1071,1024,1024, 0, 0, 0},  //TV = 9.87(102 lines)  AV=2.28  SV=5.00  BV=7.14
    {1134,1024,1032, 0, 0, 0},  //TV = 9.78(108 lines)  AV=2.28  SV=5.01  BV=7.05
    {1218,1024,1032, 0, 0, 0},  //TV = 9.68(116 lines)  AV=2.28  SV=5.01  BV=6.95
    {1302,1024,1032, 0, 0, 0},  //TV = 9.59(124 lines)  AV=2.28  SV=5.01  BV=6.85
    {1407,1024,1024, 0, 0, 0},  //TV = 9.47(134 lines)  AV=2.28  SV=5.00  BV=6.75
    {1512,1024,1024, 0, 0, 0},  //TV = 9.37(144 lines)  AV=2.28  SV=5.00  BV=6.64
    {1617,1024,1024, 0, 0, 0},  //TV = 9.27(154 lines)  AV=2.28  SV=5.00  BV=6.55
    {1733,1024,1024, 0, 0, 0},  //TV = 9.17(165 lines)  AV=2.28  SV=5.00  BV=6.45
    {1859,1024,1024, 0, 0, 0},  //TV = 9.07(177 lines)  AV=2.28  SV=5.00  BV=6.35
    {1995,1024,1024, 0, 0, 0},  //TV = 8.97(190 lines)  AV=2.28  SV=5.00  BV=6.24
    {2132,1024,1024, 0, 0, 0},  //TV = 8.87(203 lines)  AV=2.28  SV=5.00  BV=6.15
    {2279,1024,1032, 0, 0, 0},  //TV = 8.78(217 lines)  AV=2.28  SV=5.01  BV=6.04
    {2447,1024,1024, 0, 0, 0},  //TV = 8.67(233 lines)  AV=2.28  SV=5.00  BV=5.95
    {2636,1024,1024, 0, 0, 0},  //TV = 8.57(251 lines)  AV=2.28  SV=5.00  BV=5.84
    {2814,1024,1024, 0, 0, 0},  //TV = 8.47(268 lines)  AV=2.28  SV=5.00  BV=5.75
    {3014,1024,1024, 0, 0, 0},  //TV = 8.37(287 lines)  AV=2.28  SV=5.00  BV=5.65
    {3245,1024,1024, 0, 0, 0},  //TV = 8.27(309 lines)  AV=2.28  SV=5.00  BV=5.54
    {3476,1024,1024, 0, 0, 0},  //TV = 8.17(331 lines)  AV=2.28  SV=5.00  BV=5.44
    {3717,1024,1024, 0, 0, 0},  //TV = 8.07(354 lines)  AV=2.28  SV=5.00  BV=5.35
    {3980,1024,1024, 0, 0, 0},  //TV = 7.97(379 lines)  AV=2.28  SV=5.00  BV=5.25
    {4263,1024,1024, 0, 0, 0},  //TV = 7.87(406 lines)  AV=2.28  SV=5.00  BV=5.15
    {4568,1024,1024, 0, 0, 0},  //TV = 7.77(435 lines)  AV=2.28  SV=5.00  BV=5.05
    {4914,1024,1024, 0, 0, 0},  //TV = 7.67(468 lines)  AV=2.28  SV=5.00  BV=4.94
    {5271,1024,1024, 0, 0, 0},  //TV = 7.57(502 lines)  AV=2.28  SV=5.00  BV=4.84
    {5639,1024,1024, 0, 0, 0},  //TV = 7.47(537 lines)  AV=2.28  SV=5.00  BV=4.75
    {6038,1024,1024, 0, 0, 0},  //TV = 7.37(575 lines)  AV=2.28  SV=5.00  BV=4.65
    {6468,1024,1024, 0, 0, 0},  //TV = 7.27(616 lines)  AV=2.28  SV=5.00  BV=4.55
    {6930,1024,1024, 0, 0, 0},  //TV = 7.17(660 lines)  AV=2.28  SV=5.00  BV=4.45
    {7424,1024,1024, 0, 0, 0},  //TV = 7.07(707 lines)  AV=2.28  SV=5.00  BV=4.35
    {8001,1024,1024, 0, 0, 0},  //TV = 6.97(762 lines)  AV=2.28  SV=5.00  BV=4.24
    {8568,1024,1024, 0, 0, 0},  //TV = 6.87(816 lines)  AV=2.28  SV=5.00  BV=4.14
    {9177,1024,1024, 0, 0, 0},  //TV = 6.77(874 lines)  AV=2.28  SV=5.00  BV=4.04
    {9839,1024,1024, 0, 0, 0},  //TV = 6.67(937 lines)  AV=2.28  SV=5.00  BV=3.94
    {9996,1072,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.07  BV=3.85
    {9996,1152,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.17  BV=3.75
    {9996,1232,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.27  BV=3.65
    {9996,1328,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.38  BV=3.54
    {9996,1424,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.48  BV=3.44
    {9996,1520,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.57  BV=3.35
    {9996,1632,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.67  BV=3.25
    {9996,1744,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.77  BV=3.15
    {9996,1872,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.87  BV=3.05
    {9996,2000,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.97  BV=2.95
    {20003,1072,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.07  BV=2.85
    {20003,1152,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.17  BV=2.75
    {20003,1232,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.27  BV=2.65
    {20003,1328,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.38  BV=2.54
    {20003,1424,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.48  BV=2.44
    {20003,1520,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.57  BV=2.35
    {29999,1088,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.09  BV=2.25
    {29999,1168,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.19  BV=2.14
    {29999,1248,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.29  BV=2.05
    {29999,1344,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.39  BV=1.94
    {29999,1440,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.49  BV=1.84
    {29999,1536,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.58  BV=1.75
    {29999,1648,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.69  BV=1.65
    {29999,1776,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.79  BV=1.54
    {29999,1888,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.88  BV=1.45
    {29999,2032,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.99  BV=1.35
    {29999,2176,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.09  BV=1.25
    {29999,2336,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.19  BV=1.14
    {29999,2496,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.29  BV=1.05
    {29999,2688,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.39  BV=0.94
    {29999,2864,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.48  BV=0.85
    {29999,3072,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.58  BV=0.75
    {29999,3312,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.69  BV=0.64
    {29999,3536,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.79  BV=0.55
    {29999,3792,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.89  BV=0.45
    {29999,4048,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.98  BV=0.35
    {29999,4336,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.08  BV=0.25
    {29999,4672,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.19  BV=0.14
    {40005,3760,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=6.88  BV=0.04
    {40005,4032,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=6.98  BV=-0.06
    {40005,4288,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=7.07  BV=-0.15
    {40005,4624,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=7.17  BV=-0.26
    {40005,4944,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=7.27  BV=-0.35
    {40005,5296,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=7.37  BV=-0.45
    {40005,5680,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=7.47  BV=-0.55
    {40005,6080,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=7.57  BV=-0.65
    {40005,6512,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=7.67  BV=-0.75
    {40005,7024,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=7.78  BV=-0.86
    {40005,7520,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=7.88  BV=-0.96
    {40005,8048,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=7.97  BV=-1.06
    {40005,8592,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=8.07  BV=-1.15
    {50001,7408,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=7.85  BV=-1.26
    {50001,7952,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=7.96  BV=-1.36
    {50001,8512,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=8.06  BV=-1.46
    {59997,7600,1024, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=7.89  BV=-1.56
    {59997,8144,1024, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=7.99  BV=-1.66
    {59997,8736,1024, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=8.09  BV=-1.76
    {70004,8016,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=7.97  BV=-1.86
    {70004,8592,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=8.07  BV=-1.96
    {70004,9200,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=8.17  BV=-2.06
    {70004,9888,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=8.27  BV=-2.16
    {70004,10592,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=8.37  BV=-2.26
    {80000,9872,1024, 0, 0, 0},  //TV = 3.64(7619 lines)  AV=2.28  SV=8.27  BV=-2.35
    {80000,10640,1024, 0, 0, 0},  //TV = 3.64(7619 lines)  AV=2.28  SV=8.38  BV=-2.46
    {89996,10144,1024, 0, 0, 0},  //TV = 3.47(8571 lines)  AV=2.28  SV=8.31  BV=-2.56
    {100002,9776,1024, 0, 0, 0},  //TV = 3.32(9524 lines)  AV=2.28  SV=8.26  BV=-2.66
    {100002,10448,1024, 0, 0, 0},  //TV = 3.32(9524 lines)  AV=2.28  SV=8.35  BV=-2.75
    {100002,11200,1024, 0, 0, 0},  //TV = 3.32(9524 lines)  AV=2.28  SV=8.45  BV=-2.85
    {100002,12000,1024, 0, 0, 0},  //TV = 3.32(9524 lines)  AV=2.28  SV=8.55  BV=-2.95
    {100002,12848,1024, 0, 0, 0},  //TV = 3.32(9524 lines)  AV=2.28  SV=8.65  BV=-3.05
    {100002,13760,1024, 0, 0, 0},  //TV = 3.32(9524 lines)  AV=2.28  SV=8.75  BV=-3.15
    {100002,14752,1024, 0, 0, 0},  //TV = 3.32(9524 lines)  AV=2.28  SV=8.85  BV=-3.25
    {100002,15808,1024, 0, 0, 0},  //TV = 3.32(9524 lines)  AV=2.28  SV=8.95  BV=-3.35
    {100002,15872,1024, 0, 0, 0},  /* TV = 3.32(9524 lines)  AV=2.28  SV=8.95  BV=-3.36 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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
    142, //u4TotalIndex
    20, //i4StrobeTrigerBV
    106, //i4MaxBV
    -34, //i4MinBV
    90, //i4EffectiveMaxBV
    -30, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_AUTO, //ISO
    sCapturePLineTable_60Hz,
    sCapturePLineTable_50Hz,
    NULL,
};
static strEvPline sVideoPLineTable_60Hz =
{
{
    {94,1088,1024, 0, 0, 0},  //TV = 13.38(9 lines)  AV=2.28  SV=5.09  BV=10.56
    {94,1120,1024, 0, 0, 0},  /* TV = 13.38(9 lines)  AV=2.28  SV=5.13  BV=10.52 */
    {104,1072,1024, 0, 0, 0},  //TV = 13.23(10 lines)  AV=2.28  SV=5.07  BV=10.44
    {115,1056,1024, 0, 0, 0},  //TV = 13.09(11 lines)  AV=2.28  SV=5.04  BV=10.32
    {125,1040,1024, 0, 0, 0},  //TV = 12.97(12 lines)  AV=2.28  SV=5.02  BV=10.22
    {135,1024,1024, 0, 0, 0},  //TV = 12.85(13 lines)  AV=2.28  SV=5.00  BV=10.13
    {135,1088,1024, 0, 0, 0},  //TV = 12.85(13 lines)  AV=2.28  SV=5.09  BV=10.04
    {146,1072,1024, 0, 0, 0},  //TV = 12.74(14 lines)  AV=2.28  SV=5.07  BV=9.95
    {166,1024,1024, 0, 0, 0},  //TV = 12.56(16 lines)  AV=2.28  SV=5.00  BV=9.83
    {177,1024,1032, 0, 0, 0},  //TV = 12.46(17 lines)  AV=2.28  SV=5.01  BV=9.73
    {187,1056,1024, 0, 0, 0},  //TV = 12.38(18 lines)  AV=2.28  SV=5.04  BV=9.62
    {198,1040,1024, 0, 0, 0},  /* TV = 12.30(19 lines)  AV=2.28  SV=5.02  BV=9.55 */
    {208,1072,1024, 0, 0, 0},  //TV = 12.23(20 lines)  AV=2.28  SV=5.07  BV=9.44
    {229,1040,1024, 0, 0, 0},  //TV = 12.09(22 lines)  AV=2.28  SV=5.02  BV=9.35
    {249,1024,1024, 0, 0, 0},  //TV = 11.97(24 lines)  AV=2.28  SV=5.00  BV=9.25
    {260,1056,1024, 0, 0, 0},  //TV = 11.91(25 lines)  AV=2.28  SV=5.04  BV=9.14
    {281,1040,1024, 0, 0, 0},  //TV = 11.80(27 lines)  AV=2.28  SV=5.02  BV=9.05
    {301,1040,1024, 0, 0, 0},  //TV = 11.70(29 lines)  AV=2.28  SV=5.02  BV=8.95
    {322,1040,1024, 0, 0, 0},  //TV = 11.60(31 lines)  AV=2.28  SV=5.02  BV=8.85
    {353,1024,1024, 0, 0, 0},  //TV = 11.47(34 lines)  AV=2.28  SV=5.00  BV=8.74
    {374,1024,1032, 0, 0, 0},  //TV = 11.38(36 lines)  AV=2.28  SV=5.01  BV=8.65
    {395,1040,1024, 0, 0, 0},  //TV = 11.31(38 lines)  AV=2.28  SV=5.02  BV=8.56
    {426,1040,1024, 0, 0, 0},  //TV = 11.20(41 lines)  AV=2.28  SV=5.02  BV=8.45
    {457,1040,1024, 0, 0, 0},  //TV = 11.10(44 lines)  AV=2.28  SV=5.02  BV=8.35
    {498,1024,1024, 0, 0, 0},  //TV = 10.97(48 lines)  AV=2.28  SV=5.00  BV=8.25
    {529,1024,1032, 0, 0, 0},  //TV = 10.88(51 lines)  AV=2.28  SV=5.01  BV=8.15
    {571,1024,1024, 0, 0, 0},  //TV = 10.77(55 lines)  AV=2.28  SV=5.00  BV=8.05
    {612,1024,1024, 0, 0, 0},  //TV = 10.67(59 lines)  AV=2.28  SV=5.00  BV=7.95
    {654,1024,1032, 0, 0, 0},  //TV = 10.58(63 lines)  AV=2.28  SV=5.01  BV=7.84
    {695,1040,1024, 0, 0, 0},  //TV = 10.49(67 lines)  AV=2.28  SV=5.02  BV=7.74
    {758,1024,1024, 0, 0, 0},  //TV = 10.37(73 lines)  AV=2.28  SV=5.00  BV=7.64
    {809,1024,1024, 0, 0, 0},  //TV = 10.27(78 lines)  AV=2.28  SV=5.00  BV=7.55
    {861,1024,1032, 0, 0, 0},  //TV = 10.18(83 lines)  AV=2.28  SV=5.01  BV=7.45
    {924,1024,1032, 0, 0, 0},  //TV = 10.08(89 lines)  AV=2.28  SV=5.01  BV=7.34
    {986,1024,1032, 0, 0, 0},  //TV = 9.99(95 lines)  AV=2.28  SV=5.01  BV=7.25
    {1058,1024,1032, 0, 0, 0},  //TV = 9.88(102 lines)  AV=2.28  SV=5.01  BV=7.15
    {1141,1024,1032, 0, 0, 0},  //TV = 9.78(110 lines)  AV=2.28  SV=5.01  BV=7.04
    {1214,1024,1032, 0, 0, 0},  //TV = 9.69(117 lines)  AV=2.28  SV=5.01  BV=6.95
    {1318,1024,1024, 0, 0, 0},  //TV = 9.57(127 lines)  AV=2.28  SV=5.00  BV=6.84
    {1401,1024,1032, 0, 0, 0},  //TV = 9.48(135 lines)  AV=2.28  SV=5.01  BV=6.74
    {1504,1024,1032, 0, 0, 0},  //TV = 9.38(145 lines)  AV=2.28  SV=5.01  BV=6.64
    {1608,1024,1032, 0, 0, 0},  //TV = 9.28(155 lines)  AV=2.28  SV=5.01  BV=6.54
    {1732,1024,1032, 0, 0, 0},  //TV = 9.17(167 lines)  AV=2.28  SV=5.01  BV=6.44
    {1857,1024,1024, 0, 0, 0},  //TV = 9.07(179 lines)  AV=2.28  SV=5.00  BV=6.35
    {1992,1024,1032, 0, 0, 0},  //TV = 8.97(192 lines)  AV=2.28  SV=5.01  BV=6.24
    {2116,1024,1032, 0, 0, 0},  //TV = 8.88(204 lines)  AV=2.28  SV=5.01  BV=6.15
    {2303,1024,1024, 0, 0, 0},  //TV = 8.76(222 lines)  AV=2.28  SV=5.00  BV=6.04
    {2458,1024,1024, 0, 0, 0},  //TV = 8.67(237 lines)  AV=2.28  SV=5.00  BV=5.94
    {2635,1024,1024, 0, 0, 0},  //TV = 8.57(254 lines)  AV=2.28  SV=5.00  BV=5.84
    {2811,1024,1024, 0, 0, 0},  //TV = 8.47(271 lines)  AV=2.28  SV=5.00  BV=5.75
    {3029,1024,1024, 0, 0, 0},  //TV = 8.37(292 lines)  AV=2.28  SV=5.00  BV=5.64
    {3247,1024,1024, 0, 0, 0},  //TV = 8.27(313 lines)  AV=2.28  SV=5.00  BV=5.54
    {3475,1024,1024, 0, 0, 0},  //TV = 8.17(335 lines)  AV=2.28  SV=5.00  BV=5.44
    {3724,1024,1024, 0, 0, 0},  //TV = 8.07(359 lines)  AV=2.28  SV=5.00  BV=5.34
    {3983,1024,1024, 0, 0, 0},  //TV = 7.97(384 lines)  AV=2.28  SV=5.00  BV=5.25
    {4263,1024,1024, 0, 0, 0},  //TV = 7.87(411 lines)  AV=2.28  SV=5.00  BV=5.15
    {4595,1024,1024, 0, 0, 0},  //TV = 7.77(443 lines)  AV=2.28  SV=5.00  BV=5.04
    {4916,1024,1024, 0, 0, 0},  //TV = 7.67(474 lines)  AV=2.28  SV=5.00  BV=4.94
    {5269,1024,1024, 0, 0, 0},  //TV = 7.57(508 lines)  AV=2.28  SV=5.00  BV=4.84
    {5642,1024,1024, 0, 0, 0},  //TV = 7.47(544 lines)  AV=2.28  SV=5.00  BV=4.74
    {6036,1024,1024, 0, 0, 0},  //TV = 7.37(582 lines)  AV=2.28  SV=5.00  BV=4.65
    {6472,1024,1024, 0, 0, 0},  //TV = 7.27(624 lines)  AV=2.28  SV=5.00  BV=4.55
    {6928,1024,1024, 0, 0, 0},  //TV = 7.17(668 lines)  AV=2.28  SV=5.00  BV=4.45
    {7468,1024,1024, 0, 0, 0},  //TV = 7.07(720 lines)  AV=2.28  SV=5.00  BV=4.34
    {7997,1024,1024, 0, 0, 0},  //TV = 6.97(771 lines)  AV=2.28  SV=5.00  BV=4.24
    {8328,1040,1024, 0, 0, 0},  //TV = 6.91(803 lines)  AV=2.28  SV=5.02  BV=4.16
    {8328,1120,1024, 0, 0, 0},  //TV = 6.91(803 lines)  AV=2.28  SV=5.13  BV=4.05
    {8328,1200,1024, 0, 0, 0},  //TV = 6.91(803 lines)  AV=2.28  SV=5.23  BV=3.95
    {8328,1296,1024, 0, 0, 0},  //TV = 6.91(803 lines)  AV=2.28  SV=5.34  BV=3.84
    {8328,1392,1024, 0, 0, 0},  //TV = 6.91(803 lines)  AV=2.28  SV=5.44  BV=3.74
    {8328,1488,1024, 0, 0, 0},  //TV = 6.91(803 lines)  AV=2.28  SV=5.54  BV=3.64
    {8328,1584,1024, 0, 0, 0},  //TV = 6.91(803 lines)  AV=2.28  SV=5.63  BV=3.55
    {8328,1696,1024, 0, 0, 0},  //TV = 6.91(803 lines)  AV=2.28  SV=5.73  BV=3.45
    {8328,1824,1024, 0, 0, 0},  //TV = 6.91(803 lines)  AV=2.28  SV=5.83  BV=3.35
    {8328,1952,1024, 0, 0, 0},  //TV = 6.91(803 lines)  AV=2.28  SV=5.93  BV=3.25
    {16667,1040,1024, 0, 0, 0},  //TV = 5.91(1607 lines)  AV=2.28  SV=5.02  BV=3.16
    {16667,1120,1024, 0, 0, 0},  //TV = 5.91(1607 lines)  AV=2.28  SV=5.13  BV=3.05
    {16667,1200,1024, 0, 0, 0},  //TV = 5.91(1607 lines)  AV=2.28  SV=5.23  BV=2.95
    {16667,1296,1024, 0, 0, 0},  //TV = 5.91(1607 lines)  AV=2.28  SV=5.34  BV=2.84
    {16667,1376,1024, 0, 0, 0},  //TV = 5.91(1607 lines)  AV=2.28  SV=5.43  BV=2.76
    {16667,1488,1024, 0, 0, 0},  //TV = 5.91(1607 lines)  AV=2.28  SV=5.54  BV=2.64
    {24995,1056,1024, 0, 0, 0},  //TV = 5.32(2410 lines)  AV=2.28  SV=5.04  BV=2.55
    {24995,1136,1024, 0, 0, 0},  //TV = 5.32(2410 lines)  AV=2.28  SV=5.15  BV=2.45
    {24995,1216,1024, 0, 0, 0},  //TV = 5.32(2410 lines)  AV=2.28  SV=5.25  BV=2.35
    {24995,1296,1024, 0, 0, 0},  //TV = 5.32(2410 lines)  AV=2.28  SV=5.34  BV=2.26
    {33292,1056,1024, 0, 0, 0},  //TV = 4.91(3210 lines)  AV=2.28  SV=5.04  BV=2.14
    {33292,1120,1024, 0, 0, 0},  //TV = 4.91(3210 lines)  AV=2.28  SV=5.13  BV=2.05
    {33292,1200,1024, 0, 0, 0},  //TV = 4.91(3210 lines)  AV=2.28  SV=5.23  BV=1.95
    {33292,1296,1024, 0, 0, 0},  //TV = 4.91(3210 lines)  AV=2.28  SV=5.34  BV=1.84
    {33292,1376,1024, 0, 0, 0},  //TV = 4.91(3210 lines)  AV=2.28  SV=5.43  BV=1.76
    {33292,1488,1024, 0, 0, 0},  //TV = 4.91(3210 lines)  AV=2.28  SV=5.54  BV=1.64
    {33292,1600,1024, 0, 0, 0},  //TV = 4.91(3210 lines)  AV=2.28  SV=5.64  BV=1.54
    {33292,1712,1024, 0, 0, 0},  //TV = 4.91(3210 lines)  AV=2.28  SV=5.74  BV=1.44
    {33292,1840,1024, 0, 0, 0},  //TV = 4.91(3210 lines)  AV=2.28  SV=5.85  BV=1.34
    {33292,1968,1024, 0, 0, 0},  //TV = 4.91(3210 lines)  AV=2.28  SV=5.94  BV=1.24
    {33292,2096,1024, 0, 0, 0},  //TV = 4.91(3210 lines)  AV=2.28  SV=6.03  BV=1.15
    {33292,2240,1024, 0, 0, 0},  //TV = 4.91(3210 lines)  AV=2.28  SV=6.13  BV=1.05
    {33292,2416,1024, 0, 0, 0},  //TV = 4.91(3210 lines)  AV=2.28  SV=6.24  BV=0.95
    {33292,2576,1024, 0, 0, 0},  //TV = 4.91(3210 lines)  AV=2.28  SV=6.33  BV=0.85
    {33292,2768,1024, 0, 0, 0},  //TV = 4.91(3210 lines)  AV=2.28  SV=6.43  BV=0.75
    {33292,2976,1024, 0, 0, 0},  //TV = 4.91(3210 lines)  AV=2.28  SV=6.54  BV=0.64
    {33302,3184,1024, 0, 0, 0},  //TV = 4.91(3211 lines)  AV=2.28  SV=6.64  BV=0.55
    {33302,3408,1024, 0, 0, 0},  //TV = 4.91(3211 lines)  AV=2.28  SV=6.73  BV=0.45
    {41661,2912,1024, 0, 0, 0},  //TV = 4.59(4017 lines)  AV=2.28  SV=6.51  BV=0.35
    {41661,3136,1024, 0, 0, 0},  //TV = 4.59(4017 lines)  AV=2.28  SV=6.61  BV=0.25
    {41661,3344,1024, 0, 0, 0},  //TV = 4.59(4017 lines)  AV=2.28  SV=6.71  BV=0.15
    {41661,3584,1024, 0, 0, 0},  //TV = 4.59(4017 lines)  AV=2.28  SV=6.81  BV=0.05
    {41661,3872,1024, 0, 0, 0},  //TV = 4.59(4017 lines)  AV=2.28  SV=6.92  BV=-0.06
    {41661,4144,1024, 0, 0, 0},  //TV = 4.59(4017 lines)  AV=2.28  SV=7.02  BV=-0.16
    {41661,4432,1024, 0, 0, 0},  //TV = 4.59(4017 lines)  AV=2.28  SV=7.11  BV=-0.25
    {41661,4752,1024, 0, 0, 0},  //TV = 4.59(4017 lines)  AV=2.28  SV=7.21  BV=-0.35
    {41661,5088,1024, 0, 0, 0},  //TV = 4.59(4017 lines)  AV=2.28  SV=7.31  BV=-0.45
    {41661,5440,1024, 0, 0, 0},  //TV = 4.59(4017 lines)  AV=2.28  SV=7.41  BV=-0.55
    {41661,5872,1024, 0, 0, 0},  //TV = 4.59(4017 lines)  AV=2.28  SV=7.52  BV=-0.66
    {41661,6288,1024, 0, 0, 0},  //TV = 4.59(4017 lines)  AV=2.28  SV=7.62  BV=-0.76
    {41661,6720,1024, 0, 0, 0},  //TV = 4.59(4017 lines)  AV=2.28  SV=7.71  BV=-0.85
    {41661,7200,1024, 0, 0, 0},  //TV = 4.59(4017 lines)  AV=2.28  SV=7.81  BV=-0.95
    {41661,7712,1024, 0, 0, 0},  //TV = 4.59(4017 lines)  AV=2.28  SV=7.91  BV=-1.05
    {41661,8304,1024, 0, 0, 0},  //TV = 4.59(4017 lines)  AV=2.28  SV=8.02  BV=-1.16
    {41661,8896,1024, 0, 0, 0},  //TV = 4.59(4017 lines)  AV=2.28  SV=8.12  BV=-1.26
    {49999,7952,1024, 0, 0, 0},  //TV = 4.32(4821 lines)  AV=2.28  SV=7.96  BV=-1.36
    {49999,8512,1024, 0, 0, 0},  //TV = 4.32(4821 lines)  AV=2.28  SV=8.06  BV=-1.46
    {58327,7824,1024, 0, 0, 0},  //TV = 4.10(5624 lines)  AV=2.28  SV=7.93  BV=-1.56
    {58327,8384,1024, 0, 0, 0},  //TV = 4.10(5624 lines)  AV=2.28  SV=8.03  BV=-1.66
    {66634,7856,1024, 0, 0, 0},  //TV = 3.91(6425 lines)  AV=2.28  SV=7.94  BV=-1.76
    {66634,8416,1024, 0, 0, 0},  //TV = 3.91(6425 lines)  AV=2.28  SV=8.04  BV=-1.86
    {66634,9024,1024, 0, 0, 0},  //TV = 3.91(6425 lines)  AV=2.28  SV=8.14  BV=-1.96
    {66634,9664,1024, 0, 0, 0},  //TV = 3.91(6425 lines)  AV=2.28  SV=8.24  BV=-2.06
    {66634,10352,1024, 0, 0, 0},  //TV = 3.91(6425 lines)  AV=2.28  SV=8.34  BV=-2.16
    {66634,11088,1024, 0, 0, 0},  //TV = 3.91(6425 lines)  AV=2.28  SV=8.44  BV=-2.25
    {66634,11888,1024, 0, 0, 0},  //TV = 3.91(6425 lines)  AV=2.28  SV=8.54  BV=-2.35
    {66634,12736,1024, 0, 0, 0},  //TV = 3.91(6425 lines)  AV=2.28  SV=8.64  BV=-2.45
    {66634,13632,1024, 0, 0, 0},  //TV = 3.91(6425 lines)  AV=2.28  SV=8.73  BV=-2.55
    {66634,14608,1024, 0, 0, 0},  //TV = 3.91(6425 lines)  AV=2.28  SV=8.83  BV=-2.65
    {66634,15648,1024, 0, 0, 0},  //TV = 3.91(6425 lines)  AV=2.28  SV=8.93  BV=-2.75
    {66634,15872,1024, 0, 0, 0},  /* TV = 3.91(6425 lines)  AV=2.28  SV=8.95  BV=-2.77 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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
    {94,1088,1024, 0, 0, 0},  //TV = 13.38(9 lines)  AV=2.28  SV=5.09  BV=10.56
    {94,1120,1024, 0, 0, 0},  /* TV = 13.38(9 lines)  AV=2.28  SV=5.13  BV=10.52 */
    {104,1072,1024, 0, 0, 0},  //TV = 13.23(10 lines)  AV=2.28  SV=5.07  BV=10.44
    {115,1056,1024, 0, 0, 0},  //TV = 13.09(11 lines)  AV=2.28  SV=5.04  BV=10.32
    {125,1040,1024, 0, 0, 0},  //TV = 12.97(12 lines)  AV=2.28  SV=5.02  BV=10.22
    {135,1024,1024, 0, 0, 0},  //TV = 12.85(13 lines)  AV=2.28  SV=5.00  BV=10.13
    {135,1088,1024, 0, 0, 0},  //TV = 12.85(13 lines)  AV=2.28  SV=5.09  BV=10.04
    {146,1072,1024, 0, 0, 0},  //TV = 12.74(14 lines)  AV=2.28  SV=5.07  BV=9.95
    {166,1024,1024, 0, 0, 0},  //TV = 12.56(16 lines)  AV=2.28  SV=5.00  BV=9.83
    {177,1024,1032, 0, 0, 0},  //TV = 12.46(17 lines)  AV=2.28  SV=5.01  BV=9.73
    {187,1056,1024, 0, 0, 0},  //TV = 12.38(18 lines)  AV=2.28  SV=5.04  BV=9.62
    {198,1040,1024, 0, 0, 0},  /* TV = 12.30(19 lines)  AV=2.28  SV=5.02  BV=9.55 */
    {208,1072,1024, 0, 0, 0},  //TV = 12.23(20 lines)  AV=2.28  SV=5.07  BV=9.44
    {229,1040,1024, 0, 0, 0},  //TV = 12.09(22 lines)  AV=2.28  SV=5.02  BV=9.35
    {249,1024,1024, 0, 0, 0},  //TV = 11.97(24 lines)  AV=2.28  SV=5.00  BV=9.25
    {260,1056,1024, 0, 0, 0},  //TV = 11.91(25 lines)  AV=2.28  SV=5.04  BV=9.14
    {281,1040,1024, 0, 0, 0},  //TV = 11.80(27 lines)  AV=2.28  SV=5.02  BV=9.05
    {301,1040,1024, 0, 0, 0},  //TV = 11.70(29 lines)  AV=2.28  SV=5.02  BV=8.95
    {322,1040,1024, 0, 0, 0},  //TV = 11.60(31 lines)  AV=2.28  SV=5.02  BV=8.85
    {353,1024,1024, 0, 0, 0},  //TV = 11.47(34 lines)  AV=2.28  SV=5.00  BV=8.74
    {374,1024,1032, 0, 0, 0},  //TV = 11.38(36 lines)  AV=2.28  SV=5.01  BV=8.65
    {395,1040,1024, 0, 0, 0},  //TV = 11.31(38 lines)  AV=2.28  SV=5.02  BV=8.56
    {426,1040,1024, 0, 0, 0},  //TV = 11.20(41 lines)  AV=2.28  SV=5.02  BV=8.45
    {457,1040,1024, 0, 0, 0},  //TV = 11.10(44 lines)  AV=2.28  SV=5.02  BV=8.35
    {498,1024,1024, 0, 0, 0},  //TV = 10.97(48 lines)  AV=2.28  SV=5.00  BV=8.25
    {529,1024,1032, 0, 0, 0},  //TV = 10.88(51 lines)  AV=2.28  SV=5.01  BV=8.15
    {571,1024,1024, 0, 0, 0},  //TV = 10.77(55 lines)  AV=2.28  SV=5.00  BV=8.05
    {612,1024,1024, 0, 0, 0},  //TV = 10.67(59 lines)  AV=2.28  SV=5.00  BV=7.95
    {654,1024,1032, 0, 0, 0},  //TV = 10.58(63 lines)  AV=2.28  SV=5.01  BV=7.84
    {695,1040,1024, 0, 0, 0},  //TV = 10.49(67 lines)  AV=2.28  SV=5.02  BV=7.74
    {758,1024,1024, 0, 0, 0},  //TV = 10.37(73 lines)  AV=2.28  SV=5.00  BV=7.64
    {809,1024,1024, 0, 0, 0},  //TV = 10.27(78 lines)  AV=2.28  SV=5.00  BV=7.55
    {861,1024,1032, 0, 0, 0},  //TV = 10.18(83 lines)  AV=2.28  SV=5.01  BV=7.45
    {924,1024,1032, 0, 0, 0},  //TV = 10.08(89 lines)  AV=2.28  SV=5.01  BV=7.34
    {986,1024,1032, 0, 0, 0},  //TV = 9.99(95 lines)  AV=2.28  SV=5.01  BV=7.25
    {1058,1024,1032, 0, 0, 0},  //TV = 9.88(102 lines)  AV=2.28  SV=5.01  BV=7.15
    {1141,1024,1032, 0, 0, 0},  //TV = 9.78(110 lines)  AV=2.28  SV=5.01  BV=7.04
    {1214,1024,1032, 0, 0, 0},  //TV = 9.69(117 lines)  AV=2.28  SV=5.01  BV=6.95
    {1318,1024,1024, 0, 0, 0},  //TV = 9.57(127 lines)  AV=2.28  SV=5.00  BV=6.84
    {1401,1024,1032, 0, 0, 0},  //TV = 9.48(135 lines)  AV=2.28  SV=5.01  BV=6.74
    {1504,1024,1032, 0, 0, 0},  //TV = 9.38(145 lines)  AV=2.28  SV=5.01  BV=6.64
    {1608,1024,1032, 0, 0, 0},  //TV = 9.28(155 lines)  AV=2.28  SV=5.01  BV=6.54
    {1732,1024,1032, 0, 0, 0},  //TV = 9.17(167 lines)  AV=2.28  SV=5.01  BV=6.44
    {1857,1024,1024, 0, 0, 0},  //TV = 9.07(179 lines)  AV=2.28  SV=5.00  BV=6.35
    {1992,1024,1032, 0, 0, 0},  //TV = 8.97(192 lines)  AV=2.28  SV=5.01  BV=6.24
    {2116,1024,1032, 0, 0, 0},  //TV = 8.88(204 lines)  AV=2.28  SV=5.01  BV=6.15
    {2303,1024,1024, 0, 0, 0},  //TV = 8.76(222 lines)  AV=2.28  SV=5.00  BV=6.04
    {2458,1024,1024, 0, 0, 0},  //TV = 8.67(237 lines)  AV=2.28  SV=5.00  BV=5.94
    {2635,1024,1024, 0, 0, 0},  //TV = 8.57(254 lines)  AV=2.28  SV=5.00  BV=5.84
    {2811,1024,1024, 0, 0, 0},  //TV = 8.47(271 lines)  AV=2.28  SV=5.00  BV=5.75
    {3029,1024,1024, 0, 0, 0},  //TV = 8.37(292 lines)  AV=2.28  SV=5.00  BV=5.64
    {3247,1024,1024, 0, 0, 0},  //TV = 8.27(313 lines)  AV=2.28  SV=5.00  BV=5.54
    {3475,1024,1024, 0, 0, 0},  //TV = 8.17(335 lines)  AV=2.28  SV=5.00  BV=5.44
    {3724,1024,1024, 0, 0, 0},  //TV = 8.07(359 lines)  AV=2.28  SV=5.00  BV=5.34
    {3983,1024,1024, 0, 0, 0},  //TV = 7.97(384 lines)  AV=2.28  SV=5.00  BV=5.25
    {4263,1024,1024, 0, 0, 0},  //TV = 7.87(411 lines)  AV=2.28  SV=5.00  BV=5.15
    {4595,1024,1024, 0, 0, 0},  //TV = 7.77(443 lines)  AV=2.28  SV=5.00  BV=5.04
    {4916,1024,1024, 0, 0, 0},  //TV = 7.67(474 lines)  AV=2.28  SV=5.00  BV=4.94
    {5269,1024,1024, 0, 0, 0},  //TV = 7.57(508 lines)  AV=2.28  SV=5.00  BV=4.84
    {5642,1024,1024, 0, 0, 0},  //TV = 7.47(544 lines)  AV=2.28  SV=5.00  BV=4.74
    {6036,1024,1024, 0, 0, 0},  //TV = 7.37(582 lines)  AV=2.28  SV=5.00  BV=4.65
    {6472,1024,1024, 0, 0, 0},  //TV = 7.27(624 lines)  AV=2.28  SV=5.00  BV=4.55
    {6928,1024,1024, 0, 0, 0},  //TV = 7.17(668 lines)  AV=2.28  SV=5.00  BV=4.45
    {7468,1024,1024, 0, 0, 0},  //TV = 7.07(720 lines)  AV=2.28  SV=5.00  BV=4.34
    {7997,1024,1024, 0, 0, 0},  //TV = 6.97(771 lines)  AV=2.28  SV=5.00  BV=4.24
    {8567,1024,1024, 0, 0, 0},  //TV = 6.87(826 lines)  AV=2.28  SV=5.00  BV=4.14
    {9179,1024,1024, 0, 0, 0},  //TV = 6.77(885 lines)  AV=2.28  SV=5.00  BV=4.04
    {9832,1024,1024, 0, 0, 0},  //TV = 6.67(948 lines)  AV=2.28  SV=5.00  BV=3.94
    {9998,1072,1024, 0, 0, 0},  //TV = 6.64(964 lines)  AV=2.28  SV=5.07  BV=3.85
    {9998,1152,1024, 0, 0, 0},  //TV = 6.64(964 lines)  AV=2.28  SV=5.17  BV=3.75
    {9998,1232,1024, 0, 0, 0},  //TV = 6.64(964 lines)  AV=2.28  SV=5.27  BV=3.65
    {9998,1328,1024, 0, 0, 0},  //TV = 6.64(964 lines)  AV=2.28  SV=5.38  BV=3.54
    {9998,1424,1024, 0, 0, 0},  //TV = 6.64(964 lines)  AV=2.28  SV=5.48  BV=3.44
    {9998,1520,1024, 0, 0, 0},  //TV = 6.64(964 lines)  AV=2.28  SV=5.57  BV=3.35
    {9998,1632,1024, 0, 0, 0},  //TV = 6.64(964 lines)  AV=2.28  SV=5.67  BV=3.25
    {9998,1744,1024, 0, 0, 0},  //TV = 6.64(964 lines)  AV=2.28  SV=5.77  BV=3.15
    {9998,1872,1024, 0, 0, 0},  //TV = 6.64(964 lines)  AV=2.28  SV=5.87  BV=3.05
    {9998,2000,1024, 0, 0, 0},  //TV = 6.64(964 lines)  AV=2.28  SV=5.97  BV=2.95
    {19996,1072,1024, 0, 0, 0},  //TV = 5.64(1928 lines)  AV=2.28  SV=5.07  BV=2.85
    {19996,1152,1024, 0, 0, 0},  //TV = 5.64(1928 lines)  AV=2.28  SV=5.17  BV=2.75
    {19996,1232,1024, 0, 0, 0},  //TV = 5.64(1928 lines)  AV=2.28  SV=5.27  BV=2.65
    {19996,1328,1024, 0, 0, 0},  //TV = 5.64(1928 lines)  AV=2.28  SV=5.38  BV=2.54
    {19996,1424,1024, 0, 0, 0},  //TV = 5.64(1928 lines)  AV=2.28  SV=5.48  BV=2.44
    {19996,1520,1024, 0, 0, 0},  //TV = 5.64(1928 lines)  AV=2.28  SV=5.57  BV=2.35
    {30004,1088,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=5.09  BV=2.25
    {30004,1168,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=5.19  BV=2.14
    {30004,1248,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=5.29  BV=2.05
    {30004,1344,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=5.39  BV=1.94
    {30004,1440,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=5.49  BV=1.84
    {30004,1536,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=5.58  BV=1.75
    {30004,1648,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=5.69  BV=1.65
    {30004,1776,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=5.79  BV=1.54
    {30004,1888,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=5.88  BV=1.45
    {30004,2032,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=5.99  BV=1.35
    {30004,2176,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=6.09  BV=1.25
    {30004,2320,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=6.18  BV=1.15
    {30004,2496,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=6.29  BV=1.05
    {30004,2688,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=6.39  BV=0.94
    {30004,2864,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=6.48  BV=0.85
    {30004,3072,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=6.58  BV=0.75
    {30004,3312,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=6.69  BV=0.64
    {30004,3536,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=6.79  BV=0.55
    {40001,2832,1024, 0, 0, 0},  //TV = 4.64(3857 lines)  AV=2.28  SV=6.47  BV=0.45
    {40001,3040,1024, 0, 0, 0},  //TV = 4.64(3857 lines)  AV=2.28  SV=6.57  BV=0.35
    {40001,3264,1024, 0, 0, 0},  //TV = 4.64(3857 lines)  AV=2.28  SV=6.67  BV=0.25
    {40001,3488,1024, 0, 0, 0},  //TV = 4.64(3857 lines)  AV=2.28  SV=6.77  BV=0.15
    {40001,3760,1024, 0, 0, 0},  //TV = 4.64(3857 lines)  AV=2.28  SV=6.88  BV=0.04
    {40001,4032,1024, 0, 0, 0},  //TV = 4.64(3857 lines)  AV=2.28  SV=6.98  BV=-0.06
    {40001,4304,1024, 0, 0, 0},  //TV = 4.64(3857 lines)  AV=2.28  SV=7.07  BV=-0.15
    {40001,4608,1024, 0, 0, 0},  //TV = 4.64(3857 lines)  AV=2.28  SV=7.17  BV=-0.25
    {40001,4944,1024, 0, 0, 0},  //TV = 4.64(3857 lines)  AV=2.28  SV=7.27  BV=-0.35
    {40001,5296,1024, 0, 0, 0},  //TV = 4.64(3857 lines)  AV=2.28  SV=7.37  BV=-0.45
    {40001,5664,1024, 0, 0, 0},  //TV = 4.64(3857 lines)  AV=2.28  SV=7.47  BV=-0.55
    {40001,6112,1024, 0, 0, 0},  //TV = 4.64(3857 lines)  AV=2.28  SV=7.58  BV=-0.66
    {40001,6544,1024, 0, 0, 0},  //TV = 4.64(3857 lines)  AV=2.28  SV=7.68  BV=-0.76
    {40001,7008,1024, 0, 0, 0},  //TV = 4.64(3857 lines)  AV=2.28  SV=7.77  BV=-0.86
    {40001,7504,1024, 0, 0, 0},  //TV = 4.64(3857 lines)  AV=2.28  SV=7.87  BV=-0.95
    {40001,8032,1024, 0, 0, 0},  //TV = 4.64(3857 lines)  AV=2.28  SV=7.97  BV=-1.05
    {40001,8640,1024, 0, 0, 0},  //TV = 4.64(3857 lines)  AV=2.28  SV=8.08  BV=-1.16
    {49999,7408,1024, 0, 0, 0},  //TV = 4.32(4821 lines)  AV=2.28  SV=7.85  BV=-1.26
    {49999,7952,1024, 0, 0, 0},  //TV = 4.32(4821 lines)  AV=2.28  SV=7.96  BV=-1.36
    {49999,8512,1024, 0, 0, 0},  //TV = 4.32(4821 lines)  AV=2.28  SV=8.06  BV=-1.46
    {59997,7600,1024, 0, 0, 0},  //TV = 4.06(5785 lines)  AV=2.28  SV=7.89  BV=-1.56
    {59997,8144,1024, 0, 0, 0},  //TV = 4.06(5785 lines)  AV=2.28  SV=7.99  BV=-1.66
    {59997,8736,1024, 0, 0, 0},  //TV = 4.06(5785 lines)  AV=2.28  SV=8.09  BV=-1.76
    {70005,8016,1024, 0, 0, 0},  //TV = 3.84(6750 lines)  AV=2.28  SV=7.97  BV=-1.86
    {70005,8592,1024, 0, 0, 0},  //TV = 3.84(6750 lines)  AV=2.28  SV=8.07  BV=-1.96
    {70005,9200,1024, 0, 0, 0},  //TV = 3.84(6750 lines)  AV=2.28  SV=8.17  BV=-2.06
    {70005,9856,1024, 0, 0, 0},  //TV = 3.84(6750 lines)  AV=2.28  SV=8.27  BV=-2.16
    {70005,10560,1024, 0, 0, 0},  //TV = 3.84(6750 lines)  AV=2.28  SV=8.37  BV=-2.25
    {70005,11312,1024, 0, 0, 0},  //TV = 3.84(6750 lines)  AV=2.28  SV=8.47  BV=-2.35
    {70005,12112,1024, 0, 0, 0},  //TV = 3.84(6750 lines)  AV=2.28  SV=8.56  BV=-2.45
    {70005,12976,1024, 0, 0, 0},  //TV = 3.84(6750 lines)  AV=2.28  SV=8.66  BV=-2.55
    {70005,13904,1024, 0, 0, 0},  //TV = 3.84(6750 lines)  AV=2.28  SV=8.76  BV=-2.65
    {70005,14896,1024, 0, 0, 0},  //TV = 3.84(6750 lines)  AV=2.28  SV=8.86  BV=-2.75
    {70005,15872,1024, 0, 0, 0},  //TV = 3.84(6750 lines)  AV=2.28  SV=8.95  BV=-2.84
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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
    136, //u4TotalIndex
    20, //i4StrobeTrigerBV
    106, //i4MaxBV
    -29, //i4MinBV
    90, //i4EffectiveMaxBV
    -30, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_AUTO, //ISO
    sVideoPLineTable_60Hz,
    sVideoPLineTable_50Hz,
    NULL,
};
static strEvPline sVideo1PLineTable_60Hz =
{
{
    {94,1280,1024, 0, 0, 0},  //TV = 13.38(9 lines)  AV=2.28  SV=5.32  BV=10.33
    {104,1216,1024, 0, 0, 0},  //TV = 13.23(10 lines)  AV=2.28  SV=5.25  BV=10.26
    {115,1200,1024, 0, 0, 0},  //TV = 13.09(11 lines)  AV=2.28  SV=5.23  BV=10.13
    {115,1280,1024, 0, 0, 0},  //TV = 13.09(11 lines)  AV=2.28  SV=5.32  BV=10.04
    {125,1248,1024, 0, 0, 0},  //TV = 12.97(12 lines)  AV=2.28  SV=5.29  BV=9.96
    {135,1264,1024, 0, 0, 0},  //TV = 12.85(13 lines)  AV=2.28  SV=5.30  BV=9.83
    {146,1264,1024, 0, 0, 0},  //TV = 12.74(14 lines)  AV=2.28  SV=5.30  BV=9.71
    {166,1200,1024, 0, 0, 0},  //TV = 12.56(16 lines)  AV=2.28  SV=5.23  BV=9.60
    {177,1216,1024, 0, 0, 0},  //TV = 12.46(17 lines)  AV=2.28  SV=5.25  BV=9.49
    {177,1264,1024, 0, 0, 0},  /* TV = 12.46(17 lines)  AV=2.28  SV=5.30  BV=9.44 */
    {197,1216,1024, 0, 0, 0},  //TV = 12.31(19 lines)  AV=2.28  SV=5.25  BV=9.34
    {208,1216,1024, 0, 0, 0},  //TV = 12.23(20 lines)  AV=2.28  SV=5.25  BV=9.26
    {218,1248,1024, 0, 0, 0},  //TV = 12.16(21 lines)  AV=2.28  SV=5.29  BV=9.15
    {239,1216,1024, 0, 0, 0},  //TV = 12.03(23 lines)  AV=2.28  SV=5.25  BV=9.06
    {260,1200,1024, 0, 0, 0},  //TV = 11.91(25 lines)  AV=2.28  SV=5.23  BV=8.96
    {280,1200,1024, 0, 0, 0},  //TV = 11.80(27 lines)  AV=2.28  SV=5.23  BV=8.85
    {291,1232,1024, 0, 0, 0},  //TV = 11.75(28 lines)  AV=2.28  SV=5.27  BV=8.75
    {322,1200,1024, 0, 0, 0},  //TV = 11.60(31 lines)  AV=2.28  SV=5.23  BV=8.65
    {343,1200,1024, 0, 0, 0},  //TV = 11.51(33 lines)  AV=2.28  SV=5.23  BV=8.56
    {363,1216,1024, 0, 0, 0},  //TV = 11.43(35 lines)  AV=2.28  SV=5.25  BV=8.45
    {394,1200,1024, 0, 0, 0},  //TV = 11.31(38 lines)  AV=2.28  SV=5.23  BV=8.36
    {425,1200,1024, 0, 0, 0},  //TV = 11.20(41 lines)  AV=2.28  SV=5.23  BV=8.25
    {446,1216,1024, 0, 0, 0},  //TV = 11.13(43 lines)  AV=2.28  SV=5.25  BV=8.16
    {488,1200,1024, 0, 0, 0},  //TV = 11.00(47 lines)  AV=2.28  SV=5.23  BV=8.05
    {519,1200,1024, 0, 0, 0},  //TV = 10.91(50 lines)  AV=2.28  SV=5.23  BV=7.96
    {560,1200,1024, 0, 0, 0},  //TV = 10.80(54 lines)  AV=2.28  SV=5.23  BV=7.85
    {602,1200,1024, 0, 0, 0},  //TV = 10.70(58 lines)  AV=2.28  SV=5.23  BV=7.74
    {643,1200,1024, 0, 0, 0},  //TV = 10.60(62 lines)  AV=2.28  SV=5.23  BV=7.65
    {685,1200,1024, 0, 0, 0},  //TV = 10.51(66 lines)  AV=2.28  SV=5.23  BV=7.56
    {736,1200,1024, 0, 0, 0},  //TV = 10.41(71 lines)  AV=2.28  SV=5.23  BV=7.45
    {788,1200,1024, 0, 0, 0},  //TV = 10.31(76 lines)  AV=2.28  SV=5.23  BV=7.36
    {850,1200,1024, 0, 0, 0},  //TV = 10.20(82 lines)  AV=2.28  SV=5.23  BV=7.25
    {913,1200,1024, 0, 0, 0},  //TV = 10.10(88 lines)  AV=2.28  SV=5.23  BV=7.14
    {975,1200,1024, 0, 0, 0},  //TV = 10.00(94 lines)  AV=2.28  SV=5.23  BV=7.05
    {1037,1200,1024, 0, 0, 0},  //TV = 9.91(100 lines)  AV=2.28  SV=5.23  BV=6.96
    {1120,1200,1024, 0, 0, 0},  //TV = 9.80(108 lines)  AV=2.28  SV=5.23  BV=6.85
    {1192,1200,1024, 0, 0, 0},  //TV = 9.71(115 lines)  AV=2.28  SV=5.23  BV=6.76
    {1286,1200,1024, 0, 0, 0},  //TV = 9.60(124 lines)  AV=2.28  SV=5.23  BV=6.65
    {1379,1200,1024, 0, 0, 0},  //TV = 9.50(133 lines)  AV=2.28  SV=5.23  BV=6.55
    {1483,1200,1024, 0, 0, 0},  //TV = 9.40(143 lines)  AV=2.28  SV=5.23  BV=6.44
    {1576,1200,1024, 0, 0, 0},  //TV = 9.31(152 lines)  AV=2.28  SV=5.23  BV=6.36
    {1700,1200,1024, 0, 0, 0},  //TV = 9.20(164 lines)  AV=2.28  SV=5.23  BV=6.25
    {1814,1200,1024, 0, 0, 0},  //TV = 9.11(175 lines)  AV=2.28  SV=5.23  BV=6.15
    {1949,1200,1024, 0, 0, 0},  //TV = 9.00(188 lines)  AV=2.28  SV=5.23  BV=6.05
    {2094,1200,1024, 0, 0, 0},  //TV = 8.90(202 lines)  AV=2.28  SV=5.23  BV=5.95
    {2239,1200,1024, 0, 0, 0},  //TV = 8.80(216 lines)  AV=2.28  SV=5.23  BV=5.85
    {2415,1200,1024, 0, 0, 0},  //TV = 8.69(233 lines)  AV=2.28  SV=5.23  BV=5.74
    {2581,1200,1024, 0, 0, 0},  //TV = 8.60(249 lines)  AV=2.28  SV=5.23  BV=5.64
    {2757,1200,1024, 0, 0, 0},  //TV = 8.50(266 lines)  AV=2.28  SV=5.23  BV=5.55
    {2954,1200,1024, 0, 0, 0},  //TV = 8.40(285 lines)  AV=2.28  SV=5.23  BV=5.45
    {3182,1200,1024, 0, 0, 0},  //TV = 8.30(307 lines)  AV=2.28  SV=5.23  BV=5.34
    {3400,1200,1024, 0, 0, 0},  //TV = 8.20(328 lines)  AV=2.28  SV=5.23  BV=5.25
    {3649,1200,1024, 0, 0, 0},  //TV = 8.10(352 lines)  AV=2.28  SV=5.23  BV=5.14
    {3897,1200,1024, 0, 0, 0},  //TV = 8.00(376 lines)  AV=2.28  SV=5.23  BV=5.05
    {4177,1200,1024, 0, 0, 0},  //TV = 7.90(403 lines)  AV=2.28  SV=5.23  BV=4.95
    {4498,1200,1024, 0, 0, 0},  //TV = 7.80(434 lines)  AV=2.28  SV=5.23  BV=4.84
    {4820,1200,1024, 0, 0, 0},  //TV = 7.70(465 lines)  AV=2.28  SV=5.23  BV=4.74
    {5162,1200,1024, 0, 0, 0},  //TV = 7.60(498 lines)  AV=2.28  SV=5.23  BV=4.64
    {5525,1200,1024, 0, 0, 0},  //TV = 7.50(533 lines)  AV=2.28  SV=5.23  BV=4.55
    {5918,1200,1024, 0, 0, 0},  //TV = 7.40(571 lines)  AV=2.28  SV=5.23  BV=4.45
    {6333,1200,1024, 0, 0, 0},  //TV = 7.30(611 lines)  AV=2.28  SV=5.23  BV=4.35
    {6789,1200,1024, 0, 0, 0},  //TV = 7.20(655 lines)  AV=2.28  SV=5.23  BV=4.25
    {7317,1200,1024, 0, 0, 0},  //TV = 7.09(706 lines)  AV=2.28  SV=5.23  BV=4.14
    {7836,1200,1024, 0, 0, 0},  //TV = 7.00(756 lines)  AV=2.28  SV=5.23  BV=4.04
    {8261,1216,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=5.25  BV=3.95
    {8261,1312,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=5.36  BV=3.84
    {8261,1392,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=5.44  BV=3.75
    {8261,1488,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=5.54  BV=3.66
    {8261,1600,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=5.64  BV=3.55
    {8261,1728,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=5.75  BV=3.44
    {8261,1840,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=5.85  BV=3.35
    {8261,1968,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=5.94  BV=3.25
    {8261,2128,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=6.06  BV=3.14
    {8261,2272,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=6.15  BV=3.04
    {8261,2432,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=6.25  BV=2.95
    {8261,2608,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=6.35  BV=2.85
    {8261,2800,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=6.45  BV=2.74
    {8261,2992,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=6.55  BV=2.65
    {8261,3200,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=6.64  BV=2.55
    {8261,3424,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=6.74  BV=2.45
    {8261,3696,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=6.85  BV=2.34
    {8261,3952,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=6.95  BV=2.25
    {8261,4240,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=7.05  BV=2.14
    {8261,4528,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=7.14  BV=2.05
    {8261,4848,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=7.24  BV=1.95
    {8261,5232,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=7.35  BV=1.84
    {8261,5600,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=7.45  BV=1.74
    {8261,6000,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=7.55  BV=1.64
    {8261,6432,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=7.65  BV=1.54
    {8261,6880,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=7.75  BV=1.45
    {8261,7376,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=7.85  BV=1.35
    {8261,7888,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=7.95  BV=1.25
    {8261,8448,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=8.04  BV=1.15
    {8261,9056,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=8.14  BV=1.05
    {8261,9776,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=8.26  BV=0.94
    {8261,10464,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=8.35  BV=0.84
    {8261,11216,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=8.45  BV=0.74
    {8261,12016,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=8.55  BV=0.64
    {8261,12864,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=8.65  BV=0.54
    {8261,13792,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=8.75  BV=0.44
    {8261,14768,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=8.85  BV=0.34
    {8261,15824,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=8.95  BV=0.24
    {8261,15872,1088, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=9.04  BV=0.15
    {8261,15872,1168, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=9.14  BV=0.05
    {8261,15872,1256, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=9.25  BV=-0.05
    {8261,15872,1320, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=9.32  BV=-0.13
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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
    {94,1280,1024, 0, 0, 0},  //TV = 13.38(9 lines)  AV=2.28  SV=5.32  BV=10.33
    {104,1216,1024, 0, 0, 0},  //TV = 13.23(10 lines)  AV=2.28  SV=5.25  BV=10.26
    {115,1200,1024, 0, 0, 0},  //TV = 13.09(11 lines)  AV=2.28  SV=5.23  BV=10.13
    {115,1280,1024, 0, 0, 0},  //TV = 13.09(11 lines)  AV=2.28  SV=5.32  BV=10.04
    {125,1248,1024, 0, 0, 0},  //TV = 12.97(12 lines)  AV=2.28  SV=5.29  BV=9.96
    {135,1264,1024, 0, 0, 0},  //TV = 12.85(13 lines)  AV=2.28  SV=5.30  BV=9.83
    {146,1264,1024, 0, 0, 0},  //TV = 12.74(14 lines)  AV=2.28  SV=5.30  BV=9.71
    {166,1200,1024, 0, 0, 0},  //TV = 12.56(16 lines)  AV=2.28  SV=5.23  BV=9.60
    {177,1216,1024, 0, 0, 0},  //TV = 12.46(17 lines)  AV=2.28  SV=5.25  BV=9.49
    {177,1264,1024, 0, 0, 0},  /* TV = 12.46(17 lines)  AV=2.28  SV=5.30  BV=9.44 */
    {197,1216,1024, 0, 0, 0},  //TV = 12.31(19 lines)  AV=2.28  SV=5.25  BV=9.34
    {208,1216,1024, 0, 0, 0},  //TV = 12.23(20 lines)  AV=2.28  SV=5.25  BV=9.26
    {218,1248,1024, 0, 0, 0},  //TV = 12.16(21 lines)  AV=2.28  SV=5.29  BV=9.15
    {239,1216,1024, 0, 0, 0},  //TV = 12.03(23 lines)  AV=2.28  SV=5.25  BV=9.06
    {260,1200,1024, 0, 0, 0},  //TV = 11.91(25 lines)  AV=2.28  SV=5.23  BV=8.96
    {280,1200,1024, 0, 0, 0},  //TV = 11.80(27 lines)  AV=2.28  SV=5.23  BV=8.85
    {291,1232,1024, 0, 0, 0},  //TV = 11.75(28 lines)  AV=2.28  SV=5.27  BV=8.75
    {322,1200,1024, 0, 0, 0},  //TV = 11.60(31 lines)  AV=2.28  SV=5.23  BV=8.65
    {343,1200,1024, 0, 0, 0},  //TV = 11.51(33 lines)  AV=2.28  SV=5.23  BV=8.56
    {363,1216,1024, 0, 0, 0},  //TV = 11.43(35 lines)  AV=2.28  SV=5.25  BV=8.45
    {394,1200,1024, 0, 0, 0},  //TV = 11.31(38 lines)  AV=2.28  SV=5.23  BV=8.36
    {425,1200,1024, 0, 0, 0},  //TV = 11.20(41 lines)  AV=2.28  SV=5.23  BV=8.25
    {446,1216,1024, 0, 0, 0},  //TV = 11.13(43 lines)  AV=2.28  SV=5.25  BV=8.16
    {488,1200,1024, 0, 0, 0},  //TV = 11.00(47 lines)  AV=2.28  SV=5.23  BV=8.05
    {519,1200,1024, 0, 0, 0},  //TV = 10.91(50 lines)  AV=2.28  SV=5.23  BV=7.96
    {560,1200,1024, 0, 0, 0},  //TV = 10.80(54 lines)  AV=2.28  SV=5.23  BV=7.85
    {602,1200,1024, 0, 0, 0},  //TV = 10.70(58 lines)  AV=2.28  SV=5.23  BV=7.74
    {643,1200,1024, 0, 0, 0},  //TV = 10.60(62 lines)  AV=2.28  SV=5.23  BV=7.65
    {685,1200,1024, 0, 0, 0},  //TV = 10.51(66 lines)  AV=2.28  SV=5.23  BV=7.56
    {736,1200,1024, 0, 0, 0},  //TV = 10.41(71 lines)  AV=2.28  SV=5.23  BV=7.45
    {788,1200,1024, 0, 0, 0},  //TV = 10.31(76 lines)  AV=2.28  SV=5.23  BV=7.36
    {850,1200,1024, 0, 0, 0},  //TV = 10.20(82 lines)  AV=2.28  SV=5.23  BV=7.25
    {913,1200,1024, 0, 0, 0},  //TV = 10.10(88 lines)  AV=2.28  SV=5.23  BV=7.14
    {975,1200,1024, 0, 0, 0},  //TV = 10.00(94 lines)  AV=2.28  SV=5.23  BV=7.05
    {1037,1200,1024, 0, 0, 0},  //TV = 9.91(100 lines)  AV=2.28  SV=5.23  BV=6.96
    {1120,1200,1024, 0, 0, 0},  //TV = 9.80(108 lines)  AV=2.28  SV=5.23  BV=6.85
    {1192,1200,1024, 0, 0, 0},  //TV = 9.71(115 lines)  AV=2.28  SV=5.23  BV=6.76
    {1286,1200,1024, 0, 0, 0},  //TV = 9.60(124 lines)  AV=2.28  SV=5.23  BV=6.65
    {1379,1200,1024, 0, 0, 0},  //TV = 9.50(133 lines)  AV=2.28  SV=5.23  BV=6.55
    {1483,1200,1024, 0, 0, 0},  //TV = 9.40(143 lines)  AV=2.28  SV=5.23  BV=6.44
    {1576,1200,1024, 0, 0, 0},  //TV = 9.31(152 lines)  AV=2.28  SV=5.23  BV=6.36
    {1700,1200,1024, 0, 0, 0},  //TV = 9.20(164 lines)  AV=2.28  SV=5.23  BV=6.25
    {1814,1200,1024, 0, 0, 0},  //TV = 9.11(175 lines)  AV=2.28  SV=5.23  BV=6.15
    {1949,1200,1024, 0, 0, 0},  //TV = 9.00(188 lines)  AV=2.28  SV=5.23  BV=6.05
    {2094,1200,1024, 0, 0, 0},  //TV = 8.90(202 lines)  AV=2.28  SV=5.23  BV=5.95
    {2239,1200,1024, 0, 0, 0},  //TV = 8.80(216 lines)  AV=2.28  SV=5.23  BV=5.85
    {2415,1200,1024, 0, 0, 0},  //TV = 8.69(233 lines)  AV=2.28  SV=5.23  BV=5.74
    {2581,1200,1024, 0, 0, 0},  //TV = 8.60(249 lines)  AV=2.28  SV=5.23  BV=5.64
    {2757,1200,1024, 0, 0, 0},  //TV = 8.50(266 lines)  AV=2.28  SV=5.23  BV=5.55
    {2954,1200,1024, 0, 0, 0},  //TV = 8.40(285 lines)  AV=2.28  SV=5.23  BV=5.45
    {3182,1200,1024, 0, 0, 0},  //TV = 8.30(307 lines)  AV=2.28  SV=5.23  BV=5.34
    {3400,1200,1024, 0, 0, 0},  //TV = 8.20(328 lines)  AV=2.28  SV=5.23  BV=5.25
    {3649,1200,1024, 0, 0, 0},  //TV = 8.10(352 lines)  AV=2.28  SV=5.23  BV=5.14
    {3897,1200,1024, 0, 0, 0},  //TV = 8.00(376 lines)  AV=2.28  SV=5.23  BV=5.05
    {4177,1200,1024, 0, 0, 0},  //TV = 7.90(403 lines)  AV=2.28  SV=5.23  BV=4.95
    {4498,1200,1024, 0, 0, 0},  //TV = 7.80(434 lines)  AV=2.28  SV=5.23  BV=4.84
    {4820,1200,1024, 0, 0, 0},  //TV = 7.70(465 lines)  AV=2.28  SV=5.23  BV=4.74
    {5162,1200,1024, 0, 0, 0},  //TV = 7.60(498 lines)  AV=2.28  SV=5.23  BV=4.64
    {5525,1200,1024, 0, 0, 0},  //TV = 7.50(533 lines)  AV=2.28  SV=5.23  BV=4.55
    {5918,1200,1024, 0, 0, 0},  //TV = 7.40(571 lines)  AV=2.28  SV=5.23  BV=4.45
    {6333,1200,1024, 0, 0, 0},  //TV = 7.30(611 lines)  AV=2.28  SV=5.23  BV=4.35
    {6789,1200,1024, 0, 0, 0},  //TV = 7.20(655 lines)  AV=2.28  SV=5.23  BV=4.25
    {7317,1200,1024, 0, 0, 0},  //TV = 7.09(706 lines)  AV=2.28  SV=5.23  BV=4.14
    {7836,1200,1024, 0, 0, 0},  //TV = 7.00(756 lines)  AV=2.28  SV=5.23  BV=4.04
    {8261,1216,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=5.25  BV=3.95
    {8261,1312,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=5.36  BV=3.84
    {8261,1392,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=5.44  BV=3.75
    {8261,1488,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=5.54  BV=3.66
    {8261,1600,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=5.64  BV=3.55
    {8261,1728,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=5.75  BV=3.44
    {8261,1840,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=5.85  BV=3.35
    {8261,1968,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=5.94  BV=3.25
    {8261,2128,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=6.06  BV=3.14
    {8261,2272,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=6.15  BV=3.04
    {8261,2432,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=6.25  BV=2.95
    {8261,2608,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=6.35  BV=2.85
    {8261,2800,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=6.45  BV=2.74
    {8261,2992,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=6.55  BV=2.65
    {8261,3200,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=6.64  BV=2.55
    {8261,3424,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=6.74  BV=2.45
    {8261,3696,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=6.85  BV=2.34
    {8261,3952,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=6.95  BV=2.25
    {8261,4240,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=7.05  BV=2.14
    {8261,4528,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=7.14  BV=2.05
    {8261,4848,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=7.24  BV=1.95
    {8261,5232,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=7.35  BV=1.84
    {8261,5600,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=7.45  BV=1.74
    {8261,6000,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=7.55  BV=1.64
    {8261,6432,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=7.65  BV=1.54
    {8261,6880,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=7.75  BV=1.45
    {8261,7376,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=7.85  BV=1.35
    {8261,7888,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=7.95  BV=1.25
    {8261,8448,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=8.04  BV=1.15
    {8261,9056,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=8.14  BV=1.05
    {8261,9776,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=8.26  BV=0.94
    {8261,10464,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=8.35  BV=0.84
    {8261,11216,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=8.45  BV=0.74
    {8261,12016,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=8.55  BV=0.64
    {8261,12864,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=8.65  BV=0.54
    {8261,13792,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=8.75  BV=0.44
    {8261,14768,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=8.85  BV=0.34
    {8261,15824,1024, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=8.95  BV=0.24
    {8261,15872,1088, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=9.04  BV=0.15
    {8261,15872,1168, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=9.14  BV=0.05
    {8261,15872,1256, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=9.25  BV=-0.05
    {8261,15872,1320, 0, 0, 0},  //TV = 6.92(797 lines)  AV=2.28  SV=9.32  BV=-0.13
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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
    106, //u4TotalIndex
    20, //i4StrobeTrigerBV
    104, //i4MaxBV
    -2, //i4MinBV
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
    {94,1280,1024, 0, 0, 0},  //TV = 13.38(9 lines)  AV=2.28  SV=5.32  BV=10.33
    {104,1216,1024, 0, 0, 0},  //TV = 13.23(10 lines)  AV=2.28  SV=5.25  BV=10.26
    {115,1200,1024, 0, 0, 0},  //TV = 13.09(11 lines)  AV=2.28  SV=5.23  BV=10.13
    {115,1280,1024, 0, 0, 0},  //TV = 13.09(11 lines)  AV=2.28  SV=5.32  BV=10.04
    {125,1248,1024, 0, 0, 0},  //TV = 12.97(12 lines)  AV=2.28  SV=5.29  BV=9.96
    {135,1264,1024, 0, 0, 0},  //TV = 12.85(13 lines)  AV=2.28  SV=5.30  BV=9.83
    {146,1264,1024, 0, 0, 0},  //TV = 12.74(14 lines)  AV=2.28  SV=5.30  BV=9.71
    {166,1200,1024, 0, 0, 0},  //TV = 12.56(16 lines)  AV=2.28  SV=5.23  BV=9.60
    {177,1216,1024, 0, 0, 0},  //TV = 12.46(17 lines)  AV=2.28  SV=5.25  BV=9.49
    {177,1264,1024, 0, 0, 0},  /* TV = 12.46(17 lines)  AV=2.28  SV=5.30  BV=9.44 */
    {197,1216,1024, 0, 0, 0},  //TV = 12.31(19 lines)  AV=2.28  SV=5.25  BV=9.34
    {208,1216,1024, 0, 0, 0},  //TV = 12.23(20 lines)  AV=2.28  SV=5.25  BV=9.26
    {218,1248,1024, 0, 0, 0},  //TV = 12.16(21 lines)  AV=2.28  SV=5.29  BV=9.15
    {239,1216,1024, 0, 0, 0},  //TV = 12.03(23 lines)  AV=2.28  SV=5.25  BV=9.06
    {260,1200,1024, 0, 0, 0},  //TV = 11.91(25 lines)  AV=2.28  SV=5.23  BV=8.96
    {280,1200,1024, 0, 0, 0},  //TV = 11.80(27 lines)  AV=2.28  SV=5.23  BV=8.85
    {291,1232,1024, 0, 0, 0},  //TV = 11.75(28 lines)  AV=2.28  SV=5.27  BV=8.75
    {322,1200,1024, 0, 0, 0},  //TV = 11.60(31 lines)  AV=2.28  SV=5.23  BV=8.65
    {343,1200,1024, 0, 0, 0},  //TV = 11.51(33 lines)  AV=2.28  SV=5.23  BV=8.56
    {363,1216,1024, 0, 0, 0},  //TV = 11.43(35 lines)  AV=2.28  SV=5.25  BV=8.45
    {394,1200,1024, 0, 0, 0},  //TV = 11.31(38 lines)  AV=2.28  SV=5.23  BV=8.36
    {425,1200,1024, 0, 0, 0},  //TV = 11.20(41 lines)  AV=2.28  SV=5.23  BV=8.25
    {446,1216,1024, 0, 0, 0},  //TV = 11.13(43 lines)  AV=2.28  SV=5.25  BV=8.16
    {488,1200,1024, 0, 0, 0},  //TV = 11.00(47 lines)  AV=2.28  SV=5.23  BV=8.05
    {519,1200,1024, 0, 0, 0},  //TV = 10.91(50 lines)  AV=2.28  SV=5.23  BV=7.96
    {560,1200,1024, 0, 0, 0},  //TV = 10.80(54 lines)  AV=2.28  SV=5.23  BV=7.85
    {602,1200,1024, 0, 0, 0},  //TV = 10.70(58 lines)  AV=2.28  SV=5.23  BV=7.74
    {643,1200,1024, 0, 0, 0},  //TV = 10.60(62 lines)  AV=2.28  SV=5.23  BV=7.65
    {685,1200,1024, 0, 0, 0},  //TV = 10.51(66 lines)  AV=2.28  SV=5.23  BV=7.56
    {736,1200,1024, 0, 0, 0},  //TV = 10.41(71 lines)  AV=2.28  SV=5.23  BV=7.45
    {788,1200,1024, 0, 0, 0},  //TV = 10.31(76 lines)  AV=2.28  SV=5.23  BV=7.36
    {850,1200,1024, 0, 0, 0},  //TV = 10.20(82 lines)  AV=2.28  SV=5.23  BV=7.25
    {913,1200,1024, 0, 0, 0},  //TV = 10.10(88 lines)  AV=2.28  SV=5.23  BV=7.14
    {975,1200,1024, 0, 0, 0},  //TV = 10.00(94 lines)  AV=2.28  SV=5.23  BV=7.05
    {1037,1200,1024, 0, 0, 0},  //TV = 9.91(100 lines)  AV=2.28  SV=5.23  BV=6.96
    {1120,1200,1024, 0, 0, 0},  //TV = 9.80(108 lines)  AV=2.28  SV=5.23  BV=6.85
    {1192,1200,1024, 0, 0, 0},  //TV = 9.71(115 lines)  AV=2.28  SV=5.23  BV=6.76
    {1286,1200,1024, 0, 0, 0},  //TV = 9.60(124 lines)  AV=2.28  SV=5.23  BV=6.65
    {1379,1200,1024, 0, 0, 0},  //TV = 9.50(133 lines)  AV=2.28  SV=5.23  BV=6.55
    {1483,1200,1024, 0, 0, 0},  //TV = 9.40(143 lines)  AV=2.28  SV=5.23  BV=6.44
    {1576,1200,1024, 0, 0, 0},  //TV = 9.31(152 lines)  AV=2.28  SV=5.23  BV=6.36
    {1700,1200,1024, 0, 0, 0},  //TV = 9.20(164 lines)  AV=2.28  SV=5.23  BV=6.25
    {1814,1200,1024, 0, 0, 0},  //TV = 9.11(175 lines)  AV=2.28  SV=5.23  BV=6.15
    {1949,1200,1024, 0, 0, 0},  //TV = 9.00(188 lines)  AV=2.28  SV=5.23  BV=6.05
    {2094,1200,1024, 0, 0, 0},  //TV = 8.90(202 lines)  AV=2.28  SV=5.23  BV=5.95
    {2239,1200,1024, 0, 0, 0},  //TV = 8.80(216 lines)  AV=2.28  SV=5.23  BV=5.85
    {2415,1200,1024, 0, 0, 0},  //TV = 8.69(233 lines)  AV=2.28  SV=5.23  BV=5.74
    {2581,1200,1024, 0, 0, 0},  //TV = 8.60(249 lines)  AV=2.28  SV=5.23  BV=5.64
    {2757,1200,1024, 0, 0, 0},  //TV = 8.50(266 lines)  AV=2.28  SV=5.23  BV=5.55
    {2954,1200,1024, 0, 0, 0},  //TV = 8.40(285 lines)  AV=2.28  SV=5.23  BV=5.45
    {3182,1200,1024, 0, 0, 0},  //TV = 8.30(307 lines)  AV=2.28  SV=5.23  BV=5.34
    {3400,1200,1024, 0, 0, 0},  //TV = 8.20(328 lines)  AV=2.28  SV=5.23  BV=5.25
    {3649,1200,1024, 0, 0, 0},  //TV = 8.10(352 lines)  AV=2.28  SV=5.23  BV=5.14
    {3897,1200,1024, 0, 0, 0},  //TV = 8.00(376 lines)  AV=2.28  SV=5.23  BV=5.05
    {4177,1200,1024, 0, 0, 0},  //TV = 7.90(403 lines)  AV=2.28  SV=5.23  BV=4.95
    {4498,1200,1024, 0, 0, 0},  //TV = 7.80(434 lines)  AV=2.28  SV=5.23  BV=4.84
    {4820,1200,1024, 0, 0, 0},  //TV = 7.70(465 lines)  AV=2.28  SV=5.23  BV=4.74
    {5162,1200,1024, 0, 0, 0},  //TV = 7.60(498 lines)  AV=2.28  SV=5.23  BV=4.64
    {5525,1200,1024, 0, 0, 0},  //TV = 7.50(533 lines)  AV=2.28  SV=5.23  BV=4.55
    {5918,1200,1024, 0, 0, 0},  //TV = 7.40(571 lines)  AV=2.28  SV=5.23  BV=4.45
    {6333,1200,1024, 0, 0, 0},  //TV = 7.30(611 lines)  AV=2.28  SV=5.23  BV=4.35
    {6789,1200,1024, 0, 0, 0},  //TV = 7.20(655 lines)  AV=2.28  SV=5.23  BV=4.25
    {7317,1200,1024, 0, 0, 0},  //TV = 7.09(706 lines)  AV=2.28  SV=5.23  BV=4.14
    {7836,1200,1024, 0, 0, 0},  //TV = 7.00(756 lines)  AV=2.28  SV=5.23  BV=4.04
    {8333,1200,1024, 0, 0, 0},  //TV = 6.91(804 lines)  AV=2.28  SV=5.23  BV=3.95
    {8333,1296,1024, 0, 0, 0},  //TV = 6.91(804 lines)  AV=2.28  SV=5.34  BV=3.84
    {8333,1392,1024, 0, 0, 0},  //TV = 6.91(804 lines)  AV=2.28  SV=5.44  BV=3.74
    {8333,1488,1024, 0, 0, 0},  //TV = 6.91(804 lines)  AV=2.28  SV=5.54  BV=3.64
    {8333,1584,1024, 0, 0, 0},  //TV = 6.91(804 lines)  AV=2.28  SV=5.63  BV=3.55
    {8333,1696,1024, 0, 0, 0},  //TV = 6.91(804 lines)  AV=2.28  SV=5.73  BV=3.45
    {8333,1824,1024, 0, 0, 0},  //TV = 6.91(804 lines)  AV=2.28  SV=5.83  BV=3.35
    {8333,1952,1024, 0, 0, 0},  //TV = 6.91(804 lines)  AV=2.28  SV=5.93  BV=3.25
    {8333,2096,1024, 0, 0, 0},  //TV = 6.91(804 lines)  AV=2.28  SV=6.03  BV=3.15
    {8333,2240,1024, 0, 0, 0},  //TV = 6.91(804 lines)  AV=2.28  SV=6.13  BV=3.05
    {16666,1200,1024, 0, 0, 0},  //TV = 5.91(1608 lines)  AV=2.28  SV=5.23  BV=2.95
    {16666,1296,1024, 0, 0, 0},  //TV = 5.91(1608 lines)  AV=2.28  SV=5.34  BV=2.84
    {16666,1376,1024, 0, 0, 0},  //TV = 5.91(1608 lines)  AV=2.28  SV=5.43  BV=2.76
    {16666,1488,1024, 0, 0, 0},  //TV = 5.91(1608 lines)  AV=2.28  SV=5.54  BV=2.64
    {16666,1584,1024, 0, 0, 0},  //TV = 5.91(1608 lines)  AV=2.28  SV=5.63  BV=2.55
    {16666,1696,1024, 0, 0, 0},  //TV = 5.91(1608 lines)  AV=2.28  SV=5.73  BV=2.45
    {16666,1824,1024, 0, 0, 0},  //TV = 5.91(1608 lines)  AV=2.28  SV=5.83  BV=2.35
    {16666,1952,1024, 0, 0, 0},  //TV = 5.91(1608 lines)  AV=2.28  SV=5.93  BV=2.25
    {16666,2096,1024, 0, 0, 0},  //TV = 5.91(1608 lines)  AV=2.28  SV=6.03  BV=2.15
    {16666,2256,1024, 0, 0, 0},  //TV = 5.91(1608 lines)  AV=2.28  SV=6.14  BV=2.04
    {16666,2416,1024, 0, 0, 0},  //TV = 5.91(1608 lines)  AV=2.28  SV=6.24  BV=1.94
    {16666,2576,1024, 0, 0, 0},  //TV = 5.91(1608 lines)  AV=2.28  SV=6.33  BV=1.85
    {16666,2784,1024, 0, 0, 0},  //TV = 5.91(1608 lines)  AV=2.28  SV=6.44  BV=1.74
    {16666,2976,1024, 0, 0, 0},  //TV = 5.91(1608 lines)  AV=2.28  SV=6.54  BV=1.64
    {16666,3168,1024, 0, 0, 0},  //TV = 5.91(1608 lines)  AV=2.28  SV=6.63  BV=1.55
    {16666,3408,1024, 0, 0, 0},  //TV = 5.91(1608 lines)  AV=2.28  SV=6.73  BV=1.45
    {16666,3648,1024, 0, 0, 0},  //TV = 5.91(1608 lines)  AV=2.28  SV=6.83  BV=1.35
    {24998,2608,1024, 0, 0, 0},  //TV = 5.32(2412 lines)  AV=2.28  SV=6.35  BV=1.25
    {24998,2784,1024, 0, 0, 0},  //TV = 5.32(2412 lines)  AV=2.28  SV=6.44  BV=1.15
    {24998,3008,1024, 0, 0, 0},  //TV = 5.32(2412 lines)  AV=2.28  SV=6.55  BV=1.04
    {24998,3200,1024, 0, 0, 0},  //TV = 5.32(2412 lines)  AV=2.28  SV=6.64  BV=0.95
    {24998,3440,1024, 0, 0, 0},  //TV = 5.32(2412 lines)  AV=2.28  SV=6.75  BV=0.85
    {33300,2768,1024, 0, 0, 0},  //TV = 4.91(3213 lines)  AV=2.28  SV=6.43  BV=0.75
    {33300,2976,1024, 0, 0, 0},  //TV = 4.91(3213 lines)  AV=2.28  SV=6.54  BV=0.64
    {33300,3184,1024, 0, 0, 0},  //TV = 4.91(3213 lines)  AV=2.28  SV=6.64  BV=0.55
    {33300,3408,1024, 0, 0, 0},  //TV = 4.91(3213 lines)  AV=2.28  SV=6.73  BV=0.45
    {33300,3664,1024, 0, 0, 0},  //TV = 4.91(3213 lines)  AV=2.28  SV=6.84  BV=0.34
    {33300,3920,1024, 0, 0, 0},  //TV = 4.91(3213 lines)  AV=2.28  SV=6.94  BV=0.25
    {33300,4192,1024, 0, 0, 0},  //TV = 4.91(3213 lines)  AV=2.28  SV=7.03  BV=0.15
    {33300,4496,1024, 0, 0, 0},  //TV = 4.91(3213 lines)  AV=2.28  SV=7.13  BV=0.05
    {33300,4816,1024, 0, 0, 0},  //TV = 4.91(3213 lines)  AV=2.28  SV=7.23  BV=-0.05
    {33300,5184,1024, 0, 0, 0},  //TV = 4.91(3213 lines)  AV=2.28  SV=7.34  BV=-0.16
    {33300,5552,1024, 0, 0, 0},  //TV = 4.91(3213 lines)  AV=2.28  SV=7.44  BV=-0.26
    {33300,5952,1024, 0, 0, 0},  //TV = 4.91(3213 lines)  AV=2.28  SV=7.54  BV=-0.36
    {33300,6368,1024, 0, 0, 0},  //TV = 4.91(3213 lines)  AV=2.28  SV=7.64  BV=-0.45
    {33300,6832,1024, 0, 0, 0},  //TV = 4.91(3213 lines)  AV=2.28  SV=7.74  BV=-0.55
    {33300,7312,1024, 0, 0, 0},  //TV = 4.91(3213 lines)  AV=2.28  SV=7.84  BV=-0.65
    {33300,7824,1024, 0, 0, 0},  //TV = 4.91(3213 lines)  AV=2.28  SV=7.93  BV=-0.75
    {33300,8384,1024, 0, 0, 0},  //TV = 4.91(3213 lines)  AV=2.28  SV=8.03  BV=-0.85
    {33300,8992,1024, 0, 0, 0},  //TV = 4.91(3213 lines)  AV=2.28  SV=8.13  BV=-0.95
    {33300,9632,1024, 0, 0, 0},  //TV = 4.91(3213 lines)  AV=2.28  SV=8.23  BV=-1.05
    {33300,10240,1024, 0, 0, 0},  //TV = 4.91(3213 lines)  AV=2.28  SV=8.32  BV=-1.14
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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
    {94,1280,1024, 0, 0, 0},  //TV = 13.38(9 lines)  AV=2.28  SV=5.32  BV=10.33
    {104,1216,1024, 0, 0, 0},  //TV = 13.23(10 lines)  AV=2.28  SV=5.25  BV=10.26
    {115,1200,1024, 0, 0, 0},  //TV = 13.09(11 lines)  AV=2.28  SV=5.23  BV=10.13
    {115,1280,1024, 0, 0, 0},  //TV = 13.09(11 lines)  AV=2.28  SV=5.32  BV=10.04
    {125,1248,1024, 0, 0, 0},  //TV = 12.97(12 lines)  AV=2.28  SV=5.29  BV=9.96
    {135,1264,1024, 0, 0, 0},  //TV = 12.85(13 lines)  AV=2.28  SV=5.30  BV=9.83
    {146,1264,1024, 0, 0, 0},  //TV = 12.74(14 lines)  AV=2.28  SV=5.30  BV=9.71
    {166,1200,1024, 0, 0, 0},  //TV = 12.56(16 lines)  AV=2.28  SV=5.23  BV=9.60
    {177,1216,1024, 0, 0, 0},  //TV = 12.46(17 lines)  AV=2.28  SV=5.25  BV=9.49
    {177,1264,1024, 0, 0, 0},  /* TV = 12.46(17 lines)  AV=2.28  SV=5.30  BV=9.44 */
    {197,1216,1024, 0, 0, 0},  //TV = 12.31(19 lines)  AV=2.28  SV=5.25  BV=9.34
    {208,1216,1024, 0, 0, 0},  //TV = 12.23(20 lines)  AV=2.28  SV=5.25  BV=9.26
    {218,1248,1024, 0, 0, 0},  //TV = 12.16(21 lines)  AV=2.28  SV=5.29  BV=9.15
    {239,1216,1024, 0, 0, 0},  //TV = 12.03(23 lines)  AV=2.28  SV=5.25  BV=9.06
    {260,1200,1024, 0, 0, 0},  //TV = 11.91(25 lines)  AV=2.28  SV=5.23  BV=8.96
    {280,1200,1024, 0, 0, 0},  //TV = 11.80(27 lines)  AV=2.28  SV=5.23  BV=8.85
    {291,1232,1024, 0, 0, 0},  //TV = 11.75(28 lines)  AV=2.28  SV=5.27  BV=8.75
    {322,1200,1024, 0, 0, 0},  //TV = 11.60(31 lines)  AV=2.28  SV=5.23  BV=8.65
    {343,1200,1024, 0, 0, 0},  //TV = 11.51(33 lines)  AV=2.28  SV=5.23  BV=8.56
    {363,1216,1024, 0, 0, 0},  //TV = 11.43(35 lines)  AV=2.28  SV=5.25  BV=8.45
    {394,1200,1024, 0, 0, 0},  //TV = 11.31(38 lines)  AV=2.28  SV=5.23  BV=8.36
    {425,1200,1024, 0, 0, 0},  //TV = 11.20(41 lines)  AV=2.28  SV=5.23  BV=8.25
    {446,1216,1024, 0, 0, 0},  //TV = 11.13(43 lines)  AV=2.28  SV=5.25  BV=8.16
    {488,1200,1024, 0, 0, 0},  //TV = 11.00(47 lines)  AV=2.28  SV=5.23  BV=8.05
    {519,1200,1024, 0, 0, 0},  //TV = 10.91(50 lines)  AV=2.28  SV=5.23  BV=7.96
    {560,1200,1024, 0, 0, 0},  //TV = 10.80(54 lines)  AV=2.28  SV=5.23  BV=7.85
    {602,1200,1024, 0, 0, 0},  //TV = 10.70(58 lines)  AV=2.28  SV=5.23  BV=7.74
    {643,1200,1024, 0, 0, 0},  //TV = 10.60(62 lines)  AV=2.28  SV=5.23  BV=7.65
    {685,1200,1024, 0, 0, 0},  //TV = 10.51(66 lines)  AV=2.28  SV=5.23  BV=7.56
    {736,1200,1024, 0, 0, 0},  //TV = 10.41(71 lines)  AV=2.28  SV=5.23  BV=7.45
    {788,1200,1024, 0, 0, 0},  //TV = 10.31(76 lines)  AV=2.28  SV=5.23  BV=7.36
    {850,1200,1024, 0, 0, 0},  //TV = 10.20(82 lines)  AV=2.28  SV=5.23  BV=7.25
    {913,1200,1024, 0, 0, 0},  //TV = 10.10(88 lines)  AV=2.28  SV=5.23  BV=7.14
    {975,1200,1024, 0, 0, 0},  //TV = 10.00(94 lines)  AV=2.28  SV=5.23  BV=7.05
    {1037,1200,1024, 0, 0, 0},  //TV = 9.91(100 lines)  AV=2.28  SV=5.23  BV=6.96
    {1120,1200,1024, 0, 0, 0},  //TV = 9.80(108 lines)  AV=2.28  SV=5.23  BV=6.85
    {1192,1200,1024, 0, 0, 0},  //TV = 9.71(115 lines)  AV=2.28  SV=5.23  BV=6.76
    {1286,1200,1024, 0, 0, 0},  //TV = 9.60(124 lines)  AV=2.28  SV=5.23  BV=6.65
    {1379,1200,1024, 0, 0, 0},  //TV = 9.50(133 lines)  AV=2.28  SV=5.23  BV=6.55
    {1483,1200,1024, 0, 0, 0},  //TV = 9.40(143 lines)  AV=2.28  SV=5.23  BV=6.44
    {1576,1200,1024, 0, 0, 0},  //TV = 9.31(152 lines)  AV=2.28  SV=5.23  BV=6.36
    {1700,1200,1024, 0, 0, 0},  //TV = 9.20(164 lines)  AV=2.28  SV=5.23  BV=6.25
    {1814,1200,1024, 0, 0, 0},  //TV = 9.11(175 lines)  AV=2.28  SV=5.23  BV=6.15
    {1949,1200,1024, 0, 0, 0},  //TV = 9.00(188 lines)  AV=2.28  SV=5.23  BV=6.05
    {2094,1200,1024, 0, 0, 0},  //TV = 8.90(202 lines)  AV=2.28  SV=5.23  BV=5.95
    {2239,1200,1024, 0, 0, 0},  //TV = 8.80(216 lines)  AV=2.28  SV=5.23  BV=5.85
    {2415,1200,1024, 0, 0, 0},  //TV = 8.69(233 lines)  AV=2.28  SV=5.23  BV=5.74
    {2581,1200,1024, 0, 0, 0},  //TV = 8.60(249 lines)  AV=2.28  SV=5.23  BV=5.64
    {2757,1200,1024, 0, 0, 0},  //TV = 8.50(266 lines)  AV=2.28  SV=5.23  BV=5.55
    {2954,1200,1024, 0, 0, 0},  //TV = 8.40(285 lines)  AV=2.28  SV=5.23  BV=5.45
    {3182,1200,1024, 0, 0, 0},  //TV = 8.30(307 lines)  AV=2.28  SV=5.23  BV=5.34
    {3400,1200,1024, 0, 0, 0},  //TV = 8.20(328 lines)  AV=2.28  SV=5.23  BV=5.25
    {3649,1200,1024, 0, 0, 0},  //TV = 8.10(352 lines)  AV=2.28  SV=5.23  BV=5.14
    {3897,1200,1024, 0, 0, 0},  //TV = 8.00(376 lines)  AV=2.28  SV=5.23  BV=5.05
    {4177,1200,1024, 0, 0, 0},  //TV = 7.90(403 lines)  AV=2.28  SV=5.23  BV=4.95
    {4498,1200,1024, 0, 0, 0},  //TV = 7.80(434 lines)  AV=2.28  SV=5.23  BV=4.84
    {4820,1200,1024, 0, 0, 0},  //TV = 7.70(465 lines)  AV=2.28  SV=5.23  BV=4.74
    {5162,1200,1024, 0, 0, 0},  //TV = 7.60(498 lines)  AV=2.28  SV=5.23  BV=4.64
    {5525,1200,1024, 0, 0, 0},  //TV = 7.50(533 lines)  AV=2.28  SV=5.23  BV=4.55
    {5918,1200,1024, 0, 0, 0},  //TV = 7.40(571 lines)  AV=2.28  SV=5.23  BV=4.45
    {6333,1200,1024, 0, 0, 0},  //TV = 7.30(611 lines)  AV=2.28  SV=5.23  BV=4.35
    {6789,1200,1024, 0, 0, 0},  //TV = 7.20(655 lines)  AV=2.28  SV=5.23  BV=4.25
    {7317,1200,1024, 0, 0, 0},  //TV = 7.09(706 lines)  AV=2.28  SV=5.23  BV=4.14
    {7836,1200,1024, 0, 0, 0},  //TV = 7.00(756 lines)  AV=2.28  SV=5.23  BV=4.04
    {8395,1200,1024, 0, 0, 0},  //TV = 6.90(810 lines)  AV=2.28  SV=5.23  BV=3.94
    {8996,1200,1024, 0, 0, 0},  //TV = 6.80(868 lines)  AV=2.28  SV=5.23  BV=3.84
    {9639,1200,1024, 0, 0, 0},  //TV = 6.70(930 lines)  AV=2.28  SV=5.23  BV=3.74
    {10002,1232,1024, 0, 0, 0},  //TV = 6.64(965 lines)  AV=2.28  SV=5.27  BV=3.65
    {10002,1328,1024, 0, 0, 0},  //TV = 6.64(965 lines)  AV=2.28  SV=5.38  BV=3.54
    {10002,1424,1024, 0, 0, 0},  //TV = 6.64(965 lines)  AV=2.28  SV=5.48  BV=3.44
    {10002,1520,1024, 0, 0, 0},  //TV = 6.64(965 lines)  AV=2.28  SV=5.57  BV=3.35
    {10002,1632,1024, 0, 0, 0},  //TV = 6.64(965 lines)  AV=2.28  SV=5.67  BV=3.25
    {10002,1744,1024, 0, 0, 0},  //TV = 6.64(965 lines)  AV=2.28  SV=5.77  BV=3.15
    {10002,1872,1024, 0, 0, 0},  //TV = 6.64(965 lines)  AV=2.28  SV=5.87  BV=3.05
    {10002,2000,1024, 0, 0, 0},  //TV = 6.64(965 lines)  AV=2.28  SV=5.97  BV=2.95
    {10002,2144,1024, 0, 0, 0},  //TV = 6.64(965 lines)  AV=2.28  SV=6.07  BV=2.85
    {10002,2304,1024, 0, 0, 0},  //TV = 6.64(965 lines)  AV=2.28  SV=6.17  BV=2.75
    {10002,2464,1024, 0, 0, 0},  //TV = 6.64(965 lines)  AV=2.28  SV=6.27  BV=2.65
    {10002,2656,1024, 0, 0, 0},  //TV = 6.64(965 lines)  AV=2.28  SV=6.38  BV=2.54
    {10002,2832,1024, 0, 0, 0},  //TV = 6.64(965 lines)  AV=2.28  SV=6.47  BV=2.45
    {10002,3056,1024, 0, 0, 0},  //TV = 6.64(965 lines)  AV=2.28  SV=6.58  BV=2.34
    {10002,3264,1024, 0, 0, 0},  //TV = 6.64(965 lines)  AV=2.28  SV=6.67  BV=2.25
    {10002,3488,1024, 0, 0, 0},  //TV = 6.64(965 lines)  AV=2.28  SV=6.77  BV=2.15
    {10002,3760,1024, 0, 0, 0},  //TV = 6.64(965 lines)  AV=2.28  SV=6.88  BV=2.04
    {10002,4016,1024, 0, 0, 0},  //TV = 6.64(965 lines)  AV=2.28  SV=6.97  BV=1.95
    {10002,4304,1024, 0, 0, 0},  //TV = 6.64(965 lines)  AV=2.28  SV=7.07  BV=1.85
    {10002,4624,1024, 0, 0, 0},  //TV = 6.64(965 lines)  AV=2.28  SV=7.17  BV=1.74
    {10002,4960,1024, 0, 0, 0},  //TV = 6.64(965 lines)  AV=2.28  SV=7.28  BV=1.64
    {20003,2640,1024, 0, 0, 0},  //TV = 5.64(1930 lines)  AV=2.28  SV=6.37  BV=1.55
    {20003,2832,1024, 0, 0, 0},  //TV = 5.64(1930 lines)  AV=2.28  SV=6.47  BV=1.45
    {20003,3040,1024, 0, 0, 0},  //TV = 5.64(1930 lines)  AV=2.28  SV=6.57  BV=1.35
    {20003,3248,1024, 0, 0, 0},  //TV = 5.64(1930 lines)  AV=2.28  SV=6.67  BV=1.25
    {20003,3504,1024, 0, 0, 0},  //TV = 5.64(1930 lines)  AV=2.28  SV=6.77  BV=1.14
    {20003,3744,1024, 0, 0, 0},  //TV = 5.64(1930 lines)  AV=2.28  SV=6.87  BV=1.05
    {30004,2688,1024, 0, 0, 0},  //TV = 5.06(2895 lines)  AV=2.28  SV=6.39  BV=0.94
    {30004,2864,1024, 0, 0, 0},  //TV = 5.06(2895 lines)  AV=2.28  SV=6.48  BV=0.85
    {30004,3072,1024, 0, 0, 0},  //TV = 5.06(2895 lines)  AV=2.28  SV=6.58  BV=0.75
    {30004,3312,1024, 0, 0, 0},  //TV = 5.06(2895 lines)  AV=2.28  SV=6.69  BV=0.64
    {30004,3520,1024, 0, 0, 0},  //TV = 5.06(2895 lines)  AV=2.28  SV=6.78  BV=0.55
    {30004,3792,1024, 0, 0, 0},  //TV = 5.06(2895 lines)  AV=2.28  SV=6.89  BV=0.44
    {30004,4064,1024, 0, 0, 0},  //TV = 5.06(2895 lines)  AV=2.28  SV=6.99  BV=0.35
    {30004,4352,1024, 0, 0, 0},  //TV = 5.06(2895 lines)  AV=2.28  SV=7.09  BV=0.25
    {30004,4656,1024, 0, 0, 0},  //TV = 5.06(2895 lines)  AV=2.28  SV=7.18  BV=0.15
    {30004,4992,1024, 0, 0, 0},  //TV = 5.06(2895 lines)  AV=2.28  SV=7.29  BV=0.05
    {30004,5344,1024, 0, 0, 0},  //TV = 5.06(2895 lines)  AV=2.28  SV=7.38  BV=-0.05
    {30004,5728,1024, 0, 0, 0},  //TV = 5.06(2895 lines)  AV=2.28  SV=7.48  BV=-0.15
    {30004,6176,1024, 0, 0, 0},  //TV = 5.06(2895 lines)  AV=2.28  SV=7.59  BV=-0.26
    {30004,6608,1024, 0, 0, 0},  //TV = 5.06(2895 lines)  AV=2.28  SV=7.69  BV=-0.36
    {30004,7072,1024, 0, 0, 0},  //TV = 5.06(2895 lines)  AV=2.28  SV=7.79  BV=-0.45
    {30004,7584,1024, 0, 0, 0},  //TV = 5.06(2895 lines)  AV=2.28  SV=7.89  BV=-0.56
    {30004,8112,1024, 0, 0, 0},  //TV = 5.06(2895 lines)  AV=2.28  SV=7.99  BV=-0.65
    {30004,8688,1024, 0, 0, 0},  //TV = 5.06(2895 lines)  AV=2.28  SV=8.08  BV=-0.75
    {30004,9312,1024, 0, 0, 0},  //TV = 5.06(2895 lines)  AV=2.28  SV=8.18  BV=-0.85
    {30004,9968,1024, 0, 0, 0},  //TV = 5.06(2895 lines)  AV=2.28  SV=8.28  BV=-0.95
    {30004,10240,1024, 0, 0, 0},  /* TV = 5.06(2895 lines)  AV=2.28  SV=8.32  BV=-0.99 */
    {30004,10240,1024, 0, 0, 0},  /* TV = 5.06(2895 lines)  AV=2.28  SV=8.32  BV=-0.99 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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
    116, //u4TotalIndex
    20, //i4StrobeTrigerBV
    104, //i4MaxBV
    -12, //i4MinBV
    90, //i4EffectiveMaxBV
    -30, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_AUTO, //ISO
    sVideo2PLineTable_60Hz,
    sVideo2PLineTable_50Hz,
    NULL,
};
static strEvPline sCustom1PLineTable_60Hz =
{
{
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {525,1200,1024, 0, 0, 0},  //TV = 10.90(50 lines)  AV=2.28  SV=5.23  BV=7.94
    {557,1200,1024, 0, 0, 0},  //TV = 10.81(53 lines)  AV=2.28  SV=5.23  BV=7.86
    {599,1200,1024, 0, 0, 0},  //TV = 10.71(57 lines)  AV=2.28  SV=5.23  BV=7.75
    {641,1200,1024, 0, 0, 0},  //TV = 10.61(61 lines)  AV=2.28  SV=5.23  BV=7.65
    {683,1216,1024, 0, 0, 0},  //TV = 10.52(65 lines)  AV=2.28  SV=5.25  BV=7.54
    {735,1200,1024, 0, 0, 0},  //TV = 10.41(70 lines)  AV=2.28  SV=5.23  BV=7.46
    {788,1200,1024, 0, 0, 0},  //TV = 10.31(75 lines)  AV=2.28  SV=5.23  BV=7.36
    {840,1216,1024, 0, 0, 0},  //TV = 10.22(80 lines)  AV=2.28  SV=5.25  BV=7.24
    {903,1200,1024, 0, 0, 0},  //TV = 10.11(86 lines)  AV=2.28  SV=5.23  BV=7.16
    {977,1200,1024, 0, 0, 0},  //TV = 10.00(93 lines)  AV=2.28  SV=5.23  BV=7.05
    {1040,1200,1024, 0, 0, 0},  //TV = 9.91(99 lines)  AV=2.28  SV=5.23  BV=6.96
    {1113,1200,1024, 0, 0, 0},  //TV = 9.81(106 lines)  AV=2.28  SV=5.23  BV=6.86
    {1197,1200,1024, 0, 0, 0},  //TV = 9.71(114 lines)  AV=2.28  SV=5.23  BV=6.75
    {1292,1200,1024, 0, 0, 0},  //TV = 9.60(123 lines)  AV=2.28  SV=5.23  BV=6.64
    {1376,1200,1024, 0, 0, 0},  //TV = 9.51(131 lines)  AV=2.28  SV=5.23  BV=6.55
    {1481,1200,1024, 0, 0, 0},  //TV = 9.40(141 lines)  AV=2.28  SV=5.23  BV=6.45
    {1575,1200,1024, 0, 0, 0},  //TV = 9.31(150 lines)  AV=2.28  SV=5.23  BV=6.36
    {1701,1200,1024, 0, 0, 0},  //TV = 9.20(162 lines)  AV=2.28  SV=5.23  BV=6.25
    {1827,1200,1024, 0, 0, 0},  //TV = 9.10(174 lines)  AV=2.28  SV=5.23  BV=6.14
    {1953,1200,1024, 0, 0, 0},  //TV = 9.00(186 lines)  AV=2.28  SV=5.23  BV=6.05
    {2090,1200,1024, 0, 0, 0},  //TV = 8.90(199 lines)  AV=2.28  SV=5.23  BV=5.95
    {2258,1200,1024, 0, 0, 0},  //TV = 8.79(215 lines)  AV=2.28  SV=5.23  BV=5.84
    {2415,1200,1024, 0, 0, 0},  //TV = 8.69(230 lines)  AV=2.28  SV=5.23  BV=5.74
    {2583,1200,1024, 0, 0, 0},  //TV = 8.60(246 lines)  AV=2.28  SV=5.23  BV=5.64
    {2762,1200,1024, 0, 0, 0},  //TV = 8.50(263 lines)  AV=2.28  SV=5.23  BV=5.55
    {2972,1200,1024, 0, 0, 0},  //TV = 8.39(283 lines)  AV=2.28  SV=5.23  BV=5.44
    {3182,1200,1024, 0, 0, 0},  //TV = 8.30(303 lines)  AV=2.28  SV=5.23  BV=5.34
    {3402,1200,1024, 0, 0, 0},  //TV = 8.20(324 lines)  AV=2.28  SV=5.23  BV=5.25
    {3644,1200,1024, 0, 0, 0},  //TV = 8.10(347 lines)  AV=2.28  SV=5.23  BV=5.15
    {3896,1200,1024, 0, 0, 0},  //TV = 8.00(371 lines)  AV=2.28  SV=5.23  BV=5.05
    {4179,1200,1024, 0, 0, 0},  //TV = 7.90(398 lines)  AV=2.28  SV=5.23  BV=4.95
    {4505,1200,1024, 0, 0, 0},  //TV = 7.79(429 lines)  AV=2.28  SV=5.23  BV=4.84
    {4820,1200,1024, 0, 0, 0},  //TV = 7.70(459 lines)  AV=2.28  SV=5.23  BV=4.74
    {5156,1200,1024, 0, 0, 0},  //TV = 7.60(491 lines)  AV=2.28  SV=5.23  BV=4.65
    {5523,1200,1024, 0, 0, 0},  //TV = 7.50(526 lines)  AV=2.28  SV=5.23  BV=4.55
    {5912,1200,1024, 0, 0, 0},  //TV = 7.40(563 lines)  AV=2.28  SV=5.23  BV=4.45
    {6332,1200,1024, 0, 0, 0},  //TV = 7.30(603 lines)  AV=2.28  SV=5.23  BV=4.35
    {6836,1200,1024, 0, 0, 0},  //TV = 7.19(651 lines)  AV=2.28  SV=5.23  BV=4.24
    {7319,1200,1024, 0, 0, 0},  //TV = 7.09(697 lines)  AV=2.28  SV=5.23  BV=4.14
    {7833,1200,1024, 0, 0, 0},  //TV = 7.00(746 lines)  AV=2.28  SV=5.23  BV=4.04
    {8338,1200,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.23  BV=3.95
    {8338,1296,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.34  BV=3.84
    {8338,1376,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.43  BV=3.75
    {8338,1488,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.54  BV=3.64
    {8338,1584,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.63  BV=3.55
    {8338,1696,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.73  BV=3.45
    {8338,1824,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.83  BV=3.35
    {8338,1952,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.93  BV=3.25
    {8338,2096,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.03  BV=3.15
    {8338,2240,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.13  BV=3.05
    {16664,1200,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.23  BV=2.95
    {16664,1296,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.34  BV=2.84
    {16664,1376,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.43  BV=2.76
    {16664,1488,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.54  BV=2.64
    {16664,1584,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.63  BV=2.55
    {16664,1696,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.73  BV=2.45
    {16664,1824,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.83  BV=2.35
    {16664,1952,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.93  BV=2.25
    {16664,2096,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.03  BV=2.15
    {16664,2256,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.14  BV=2.04
    {16664,2416,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.24  BV=1.94
    {16664,2576,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.33  BV=1.85
    {16664,2784,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.44  BV=1.74
    {16664,2976,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.54  BV=1.64
    {16664,3184,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.64  BV=1.55
    {16664,3408,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.73  BV=1.45
    {16664,3648,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.83  BV=1.35
    {25001,2608,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=6.35  BV=1.25
    {25001,2784,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=6.44  BV=1.15
    {25001,2992,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=6.55  BV=1.05
    {25001,3200,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=6.64  BV=0.95
    {25001,3440,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=6.75  BV=0.85
    {33327,2784,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.44  BV=0.74
    {33327,2976,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.54  BV=0.64
    {33327,3168,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.63  BV=0.55
    {33327,3424,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.74  BV=0.44
    {33327,3664,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.84  BV=0.34
    {33327,3920,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.94  BV=0.25
    {33327,4192,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.03  BV=0.15
    {33327,4496,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.13  BV=0.05
    {33327,4816,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.23  BV=-0.05
    {33327,5184,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.34  BV=-0.16
    {33327,5552,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.44  BV=-0.26
    {33327,5952,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.54  BV=-0.36
    {33327,6368,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.64  BV=-0.45
    {33327,6816,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.73  BV=-0.55
    {33327,7312,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.84  BV=-0.65
    {33327,7824,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.93  BV=-0.75
    {33327,8384,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.03  BV=-0.85
    {33327,8976,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.13  BV=-0.95
    {33327,9616,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.23  BV=-1.05
    {33327,10240,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.32  BV=-1.14
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {525,1200,1024, 0, 0, 0},  //TV = 10.90(50 lines)  AV=2.28  SV=5.23  BV=7.94
    {557,1200,1024, 0, 0, 0},  //TV = 10.81(53 lines)  AV=2.28  SV=5.23  BV=7.86
    {599,1200,1024, 0, 0, 0},  //TV = 10.71(57 lines)  AV=2.28  SV=5.23  BV=7.75
    {641,1200,1024, 0, 0, 0},  //TV = 10.61(61 lines)  AV=2.28  SV=5.23  BV=7.65
    {683,1216,1024, 0, 0, 0},  //TV = 10.52(65 lines)  AV=2.28  SV=5.25  BV=7.54
    {735,1200,1024, 0, 0, 0},  //TV = 10.41(70 lines)  AV=2.28  SV=5.23  BV=7.46
    {788,1200,1024, 0, 0, 0},  //TV = 10.31(75 lines)  AV=2.28  SV=5.23  BV=7.36
    {840,1216,1024, 0, 0, 0},  //TV = 10.22(80 lines)  AV=2.28  SV=5.25  BV=7.24
    {903,1200,1024, 0, 0, 0},  //TV = 10.11(86 lines)  AV=2.28  SV=5.23  BV=7.16
    {977,1200,1024, 0, 0, 0},  //TV = 10.00(93 lines)  AV=2.28  SV=5.23  BV=7.05
    {1040,1200,1024, 0, 0, 0},  //TV = 9.91(99 lines)  AV=2.28  SV=5.23  BV=6.96
    {1113,1200,1024, 0, 0, 0},  //TV = 9.81(106 lines)  AV=2.28  SV=5.23  BV=6.86
    {1197,1200,1024, 0, 0, 0},  //TV = 9.71(114 lines)  AV=2.28  SV=5.23  BV=6.75
    {1292,1200,1024, 0, 0, 0},  //TV = 9.60(123 lines)  AV=2.28  SV=5.23  BV=6.64
    {1376,1200,1024, 0, 0, 0},  //TV = 9.51(131 lines)  AV=2.28  SV=5.23  BV=6.55
    {1481,1200,1024, 0, 0, 0},  //TV = 9.40(141 lines)  AV=2.28  SV=5.23  BV=6.45
    {1575,1200,1024, 0, 0, 0},  //TV = 9.31(150 lines)  AV=2.28  SV=5.23  BV=6.36
    {1701,1200,1024, 0, 0, 0},  //TV = 9.20(162 lines)  AV=2.28  SV=5.23  BV=6.25
    {1827,1200,1024, 0, 0, 0},  //TV = 9.10(174 lines)  AV=2.28  SV=5.23  BV=6.14
    {1953,1200,1024, 0, 0, 0},  //TV = 9.00(186 lines)  AV=2.28  SV=5.23  BV=6.05
    {2090,1200,1024, 0, 0, 0},  //TV = 8.90(199 lines)  AV=2.28  SV=5.23  BV=5.95
    {2258,1200,1024, 0, 0, 0},  //TV = 8.79(215 lines)  AV=2.28  SV=5.23  BV=5.84
    {2415,1200,1024, 0, 0, 0},  //TV = 8.69(230 lines)  AV=2.28  SV=5.23  BV=5.74
    {2583,1200,1024, 0, 0, 0},  //TV = 8.60(246 lines)  AV=2.28  SV=5.23  BV=5.64
    {2762,1200,1024, 0, 0, 0},  //TV = 8.50(263 lines)  AV=2.28  SV=5.23  BV=5.55
    {2972,1200,1024, 0, 0, 0},  //TV = 8.39(283 lines)  AV=2.28  SV=5.23  BV=5.44
    {3182,1200,1024, 0, 0, 0},  //TV = 8.30(303 lines)  AV=2.28  SV=5.23  BV=5.34
    {3402,1200,1024, 0, 0, 0},  //TV = 8.20(324 lines)  AV=2.28  SV=5.23  BV=5.25
    {3644,1200,1024, 0, 0, 0},  //TV = 8.10(347 lines)  AV=2.28  SV=5.23  BV=5.15
    {3896,1200,1024, 0, 0, 0},  //TV = 8.00(371 lines)  AV=2.28  SV=5.23  BV=5.05
    {4179,1200,1024, 0, 0, 0},  //TV = 7.90(398 lines)  AV=2.28  SV=5.23  BV=4.95
    {4505,1200,1024, 0, 0, 0},  //TV = 7.79(429 lines)  AV=2.28  SV=5.23  BV=4.84
    {4820,1200,1024, 0, 0, 0},  //TV = 7.70(459 lines)  AV=2.28  SV=5.23  BV=4.74
    {5156,1200,1024, 0, 0, 0},  //TV = 7.60(491 lines)  AV=2.28  SV=5.23  BV=4.65
    {5523,1200,1024, 0, 0, 0},  //TV = 7.50(526 lines)  AV=2.28  SV=5.23  BV=4.55
    {5912,1200,1024, 0, 0, 0},  //TV = 7.40(563 lines)  AV=2.28  SV=5.23  BV=4.45
    {6332,1200,1024, 0, 0, 0},  //TV = 7.30(603 lines)  AV=2.28  SV=5.23  BV=4.35
    {6836,1200,1024, 0, 0, 0},  //TV = 7.19(651 lines)  AV=2.28  SV=5.23  BV=4.24
    {7319,1200,1024, 0, 0, 0},  //TV = 7.09(697 lines)  AV=2.28  SV=5.23  BV=4.14
    {7833,1200,1024, 0, 0, 0},  //TV = 7.00(746 lines)  AV=2.28  SV=5.23  BV=4.04
    {8400,1200,1024, 0, 0, 0},  //TV = 6.90(800 lines)  AV=2.28  SV=5.23  BV=3.94
    {8999,1200,1024, 0, 0, 0},  //TV = 6.80(857 lines)  AV=2.28  SV=5.23  BV=3.84
    {9639,1200,1024, 0, 0, 0},  //TV = 6.70(918 lines)  AV=2.28  SV=5.23  BV=3.74
    {9996,1232,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.27  BV=3.65
    {9996,1328,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.38  BV=3.54
    {9996,1424,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.48  BV=3.44
    {9996,1520,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.57  BV=3.35
    {9996,1632,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.67  BV=3.25
    {9996,1744,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.77  BV=3.15
    {9996,1872,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.87  BV=3.05
    {9996,2000,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.97  BV=2.95
    {9996,2144,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.07  BV=2.85
    {9996,2304,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.17  BV=2.75
    {9996,2464,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.27  BV=2.65
    {9996,2656,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.38  BV=2.54
    {9996,2832,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.47  BV=2.45
    {9996,3040,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.57  BV=2.35
    {9996,3264,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.67  BV=2.25
    {9996,3488,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.77  BV=2.15
    {9996,3760,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.88  BV=2.04
    {9996,4016,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.97  BV=1.95
    {9996,4304,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=7.07  BV=1.85
    {9996,4608,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=7.17  BV=1.75
    {9996,4960,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=7.28  BV=1.64
    {20003,2640,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.37  BV=1.55
    {20003,2832,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.47  BV=1.45
    {20003,3040,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.57  BV=1.35
    {20003,3248,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.67  BV=1.25
    {20003,3504,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.77  BV=1.14
    {20003,3744,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.87  BV=1.05
    {29999,2688,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.39  BV=0.94
    {29999,2880,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.49  BV=0.84
    {29999,3072,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.58  BV=0.75
    {29999,3312,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.69  BV=0.64
    {29999,3520,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.78  BV=0.55
    {29999,3808,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.89  BV=0.44
    {29999,4064,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.99  BV=0.35
    {29999,4352,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.09  BV=0.25
    {29999,4656,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.18  BV=0.15
    {29999,4992,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.29  BV=0.05
    {29999,5344,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.38  BV=-0.05
    {29999,5760,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.49  BV=-0.16
    {29999,6176,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.59  BV=-0.26
    {29999,6608,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.69  BV=-0.36
    {29999,7072,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.79  BV=-0.45
    {29999,7584,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.89  BV=-0.55
    {29999,8112,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.99  BV=-0.65
    {29999,8688,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.08  BV=-0.75
    {29999,9312,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.18  BV=-0.85
    {29999,9968,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.28  BV=-0.95
    {29999,10240,1024, 0, 0, 0},  /* TV = 5.06(2857 lines)  AV=2.28  SV=8.32  BV=-0.99 */
    {29999,10240,1024, 0, 0, 0},  /* TV = 5.06(2857 lines)  AV=2.28  SV=8.32  BV=-0.99 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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
    116, //u4TotalIndex
    20, //i4StrobeTrigerBV
    104, //i4MaxBV
    -12, //i4MinBV
    90, //i4EffectiveMaxBV
    -30, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_AUTO, //ISO
    sCustom1PLineTable_60Hz,
    sCustom1PLineTable_50Hz,
    NULL,
};
static strEvPline sCustom2PLineTable_60Hz =
{
{
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {525,1200,1024, 0, 0, 0},  //TV = 10.90(50 lines)  AV=2.28  SV=5.23  BV=7.94
    {557,1200,1024, 0, 0, 0},  //TV = 10.81(53 lines)  AV=2.28  SV=5.23  BV=7.86
    {599,1200,1024, 0, 0, 0},  //TV = 10.71(57 lines)  AV=2.28  SV=5.23  BV=7.75
    {641,1200,1024, 0, 0, 0},  //TV = 10.61(61 lines)  AV=2.28  SV=5.23  BV=7.65
    {683,1216,1024, 0, 0, 0},  //TV = 10.52(65 lines)  AV=2.28  SV=5.25  BV=7.54
    {735,1200,1024, 0, 0, 0},  //TV = 10.41(70 lines)  AV=2.28  SV=5.23  BV=7.46
    {788,1200,1024, 0, 0, 0},  //TV = 10.31(75 lines)  AV=2.28  SV=5.23  BV=7.36
    {840,1216,1024, 0, 0, 0},  //TV = 10.22(80 lines)  AV=2.28  SV=5.25  BV=7.24
    {903,1200,1024, 0, 0, 0},  //TV = 10.11(86 lines)  AV=2.28  SV=5.23  BV=7.16
    {977,1200,1024, 0, 0, 0},  //TV = 10.00(93 lines)  AV=2.28  SV=5.23  BV=7.05
    {1040,1200,1024, 0, 0, 0},  //TV = 9.91(99 lines)  AV=2.28  SV=5.23  BV=6.96
    {1113,1200,1024, 0, 0, 0},  //TV = 9.81(106 lines)  AV=2.28  SV=5.23  BV=6.86
    {1197,1200,1024, 0, 0, 0},  //TV = 9.71(114 lines)  AV=2.28  SV=5.23  BV=6.75
    {1292,1200,1024, 0, 0, 0},  //TV = 9.60(123 lines)  AV=2.28  SV=5.23  BV=6.64
    {1376,1200,1024, 0, 0, 0},  //TV = 9.51(131 lines)  AV=2.28  SV=5.23  BV=6.55
    {1481,1200,1024, 0, 0, 0},  //TV = 9.40(141 lines)  AV=2.28  SV=5.23  BV=6.45
    {1575,1200,1024, 0, 0, 0},  //TV = 9.31(150 lines)  AV=2.28  SV=5.23  BV=6.36
    {1701,1200,1024, 0, 0, 0},  //TV = 9.20(162 lines)  AV=2.28  SV=5.23  BV=6.25
    {1827,1200,1024, 0, 0, 0},  //TV = 9.10(174 lines)  AV=2.28  SV=5.23  BV=6.14
    {1953,1200,1024, 0, 0, 0},  //TV = 9.00(186 lines)  AV=2.28  SV=5.23  BV=6.05
    {2090,1200,1024, 0, 0, 0},  //TV = 8.90(199 lines)  AV=2.28  SV=5.23  BV=5.95
    {2258,1200,1024, 0, 0, 0},  //TV = 8.79(215 lines)  AV=2.28  SV=5.23  BV=5.84
    {2415,1200,1024, 0, 0, 0},  //TV = 8.69(230 lines)  AV=2.28  SV=5.23  BV=5.74
    {2583,1200,1024, 0, 0, 0},  //TV = 8.60(246 lines)  AV=2.28  SV=5.23  BV=5.64
    {2762,1200,1024, 0, 0, 0},  //TV = 8.50(263 lines)  AV=2.28  SV=5.23  BV=5.55
    {2972,1200,1024, 0, 0, 0},  //TV = 8.39(283 lines)  AV=2.28  SV=5.23  BV=5.44
    {3182,1200,1024, 0, 0, 0},  //TV = 8.30(303 lines)  AV=2.28  SV=5.23  BV=5.34
    {3402,1200,1024, 0, 0, 0},  //TV = 8.20(324 lines)  AV=2.28  SV=5.23  BV=5.25
    {3644,1200,1024, 0, 0, 0},  //TV = 8.10(347 lines)  AV=2.28  SV=5.23  BV=5.15
    {3896,1200,1024, 0, 0, 0},  //TV = 8.00(371 lines)  AV=2.28  SV=5.23  BV=5.05
    {4179,1200,1024, 0, 0, 0},  //TV = 7.90(398 lines)  AV=2.28  SV=5.23  BV=4.95
    {4505,1200,1024, 0, 0, 0},  //TV = 7.79(429 lines)  AV=2.28  SV=5.23  BV=4.84
    {4820,1200,1024, 0, 0, 0},  //TV = 7.70(459 lines)  AV=2.28  SV=5.23  BV=4.74
    {5156,1200,1024, 0, 0, 0},  //TV = 7.60(491 lines)  AV=2.28  SV=5.23  BV=4.65
    {5523,1200,1024, 0, 0, 0},  //TV = 7.50(526 lines)  AV=2.28  SV=5.23  BV=4.55
    {5912,1200,1024, 0, 0, 0},  //TV = 7.40(563 lines)  AV=2.28  SV=5.23  BV=4.45
    {6332,1200,1024, 0, 0, 0},  //TV = 7.30(603 lines)  AV=2.28  SV=5.23  BV=4.35
    {6836,1200,1024, 0, 0, 0},  //TV = 7.19(651 lines)  AV=2.28  SV=5.23  BV=4.24
    {7319,1200,1024, 0, 0, 0},  //TV = 7.09(697 lines)  AV=2.28  SV=5.23  BV=4.14
    {7833,1200,1024, 0, 0, 0},  //TV = 7.00(746 lines)  AV=2.28  SV=5.23  BV=4.04
    {8338,1200,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.23  BV=3.95
    {8338,1296,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.34  BV=3.84
    {8338,1376,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.43  BV=3.75
    {8338,1488,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.54  BV=3.64
    {8338,1584,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.63  BV=3.55
    {8338,1696,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.73  BV=3.45
    {8338,1824,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.83  BV=3.35
    {8338,1952,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.93  BV=3.25
    {8338,2096,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.03  BV=3.15
    {8338,2240,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.13  BV=3.05
    {16664,1200,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.23  BV=2.95
    {16664,1296,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.34  BV=2.84
    {16664,1376,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.43  BV=2.76
    {16664,1488,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.54  BV=2.64
    {16664,1584,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.63  BV=2.55
    {16664,1696,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.73  BV=2.45
    {16664,1824,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.83  BV=2.35
    {16664,1952,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.93  BV=2.25
    {16664,2096,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.03  BV=2.15
    {16664,2256,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.14  BV=2.04
    {16664,2416,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.24  BV=1.94
    {16664,2576,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.33  BV=1.85
    {16664,2784,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.44  BV=1.74
    {16664,2976,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.54  BV=1.64
    {16664,3184,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.64  BV=1.55
    {16664,3408,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.73  BV=1.45
    {16664,3648,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.83  BV=1.35
    {25001,2608,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=6.35  BV=1.25
    {25001,2784,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=6.44  BV=1.15
    {25001,2992,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=6.55  BV=1.05
    {25001,3200,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=6.64  BV=0.95
    {25001,3440,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=6.75  BV=0.85
    {33327,2784,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.44  BV=0.74
    {33327,2976,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.54  BV=0.64
    {33327,3168,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.63  BV=0.55
    {33327,3424,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.74  BV=0.44
    {33327,3664,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.84  BV=0.34
    {33327,3920,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.94  BV=0.25
    {33327,4192,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.03  BV=0.15
    {33327,4496,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.13  BV=0.05
    {33327,4816,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.23  BV=-0.05
    {33327,5184,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.34  BV=-0.16
    {33327,5552,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.44  BV=-0.26
    {33327,5952,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.54  BV=-0.36
    {33327,6368,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.64  BV=-0.45
    {33327,6816,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.73  BV=-0.55
    {33327,7312,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.84  BV=-0.65
    {33327,7824,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.93  BV=-0.75
    {33327,8384,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.03  BV=-0.85
    {33327,8976,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.13  BV=-0.95
    {33327,9616,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.23  BV=-1.05
    {33327,10240,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.32  BV=-1.14
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {525,1200,1024, 0, 0, 0},  //TV = 10.90(50 lines)  AV=2.28  SV=5.23  BV=7.94
    {557,1200,1024, 0, 0, 0},  //TV = 10.81(53 lines)  AV=2.28  SV=5.23  BV=7.86
    {599,1200,1024, 0, 0, 0},  //TV = 10.71(57 lines)  AV=2.28  SV=5.23  BV=7.75
    {641,1200,1024, 0, 0, 0},  //TV = 10.61(61 lines)  AV=2.28  SV=5.23  BV=7.65
    {683,1216,1024, 0, 0, 0},  //TV = 10.52(65 lines)  AV=2.28  SV=5.25  BV=7.54
    {735,1200,1024, 0, 0, 0},  //TV = 10.41(70 lines)  AV=2.28  SV=5.23  BV=7.46
    {788,1200,1024, 0, 0, 0},  //TV = 10.31(75 lines)  AV=2.28  SV=5.23  BV=7.36
    {840,1216,1024, 0, 0, 0},  //TV = 10.22(80 lines)  AV=2.28  SV=5.25  BV=7.24
    {903,1200,1024, 0, 0, 0},  //TV = 10.11(86 lines)  AV=2.28  SV=5.23  BV=7.16
    {977,1200,1024, 0, 0, 0},  //TV = 10.00(93 lines)  AV=2.28  SV=5.23  BV=7.05
    {1040,1200,1024, 0, 0, 0},  //TV = 9.91(99 lines)  AV=2.28  SV=5.23  BV=6.96
    {1113,1200,1024, 0, 0, 0},  //TV = 9.81(106 lines)  AV=2.28  SV=5.23  BV=6.86
    {1197,1200,1024, 0, 0, 0},  //TV = 9.71(114 lines)  AV=2.28  SV=5.23  BV=6.75
    {1292,1200,1024, 0, 0, 0},  //TV = 9.60(123 lines)  AV=2.28  SV=5.23  BV=6.64
    {1376,1200,1024, 0, 0, 0},  //TV = 9.51(131 lines)  AV=2.28  SV=5.23  BV=6.55
    {1481,1200,1024, 0, 0, 0},  //TV = 9.40(141 lines)  AV=2.28  SV=5.23  BV=6.45
    {1575,1200,1024, 0, 0, 0},  //TV = 9.31(150 lines)  AV=2.28  SV=5.23  BV=6.36
    {1701,1200,1024, 0, 0, 0},  //TV = 9.20(162 lines)  AV=2.28  SV=5.23  BV=6.25
    {1827,1200,1024, 0, 0, 0},  //TV = 9.10(174 lines)  AV=2.28  SV=5.23  BV=6.14
    {1953,1200,1024, 0, 0, 0},  //TV = 9.00(186 lines)  AV=2.28  SV=5.23  BV=6.05
    {2090,1200,1024, 0, 0, 0},  //TV = 8.90(199 lines)  AV=2.28  SV=5.23  BV=5.95
    {2258,1200,1024, 0, 0, 0},  //TV = 8.79(215 lines)  AV=2.28  SV=5.23  BV=5.84
    {2415,1200,1024, 0, 0, 0},  //TV = 8.69(230 lines)  AV=2.28  SV=5.23  BV=5.74
    {2583,1200,1024, 0, 0, 0},  //TV = 8.60(246 lines)  AV=2.28  SV=5.23  BV=5.64
    {2762,1200,1024, 0, 0, 0},  //TV = 8.50(263 lines)  AV=2.28  SV=5.23  BV=5.55
    {2972,1200,1024, 0, 0, 0},  //TV = 8.39(283 lines)  AV=2.28  SV=5.23  BV=5.44
    {3182,1200,1024, 0, 0, 0},  //TV = 8.30(303 lines)  AV=2.28  SV=5.23  BV=5.34
    {3402,1200,1024, 0, 0, 0},  //TV = 8.20(324 lines)  AV=2.28  SV=5.23  BV=5.25
    {3644,1200,1024, 0, 0, 0},  //TV = 8.10(347 lines)  AV=2.28  SV=5.23  BV=5.15
    {3896,1200,1024, 0, 0, 0},  //TV = 8.00(371 lines)  AV=2.28  SV=5.23  BV=5.05
    {4179,1200,1024, 0, 0, 0},  //TV = 7.90(398 lines)  AV=2.28  SV=5.23  BV=4.95
    {4505,1200,1024, 0, 0, 0},  //TV = 7.79(429 lines)  AV=2.28  SV=5.23  BV=4.84
    {4820,1200,1024, 0, 0, 0},  //TV = 7.70(459 lines)  AV=2.28  SV=5.23  BV=4.74
    {5156,1200,1024, 0, 0, 0},  //TV = 7.60(491 lines)  AV=2.28  SV=5.23  BV=4.65
    {5523,1200,1024, 0, 0, 0},  //TV = 7.50(526 lines)  AV=2.28  SV=5.23  BV=4.55
    {5912,1200,1024, 0, 0, 0},  //TV = 7.40(563 lines)  AV=2.28  SV=5.23  BV=4.45
    {6332,1200,1024, 0, 0, 0},  //TV = 7.30(603 lines)  AV=2.28  SV=5.23  BV=4.35
    {6836,1200,1024, 0, 0, 0},  //TV = 7.19(651 lines)  AV=2.28  SV=5.23  BV=4.24
    {7319,1200,1024, 0, 0, 0},  //TV = 7.09(697 lines)  AV=2.28  SV=5.23  BV=4.14
    {7833,1200,1024, 0, 0, 0},  //TV = 7.00(746 lines)  AV=2.28  SV=5.23  BV=4.04
    {8400,1200,1024, 0, 0, 0},  //TV = 6.90(800 lines)  AV=2.28  SV=5.23  BV=3.94
    {8999,1200,1024, 0, 0, 0},  //TV = 6.80(857 lines)  AV=2.28  SV=5.23  BV=3.84
    {9639,1200,1024, 0, 0, 0},  //TV = 6.70(918 lines)  AV=2.28  SV=5.23  BV=3.74
    {9996,1232,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.27  BV=3.65
    {9996,1328,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.38  BV=3.54
    {9996,1424,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.48  BV=3.44
    {9996,1520,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.57  BV=3.35
    {9996,1632,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.67  BV=3.25
    {9996,1744,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.77  BV=3.15
    {9996,1872,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.87  BV=3.05
    {9996,2000,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.97  BV=2.95
    {9996,2144,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.07  BV=2.85
    {9996,2304,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.17  BV=2.75
    {9996,2464,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.27  BV=2.65
    {9996,2656,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.38  BV=2.54
    {9996,2832,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.47  BV=2.45
    {9996,3040,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.57  BV=2.35
    {9996,3264,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.67  BV=2.25
    {9996,3488,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.77  BV=2.15
    {9996,3760,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.88  BV=2.04
    {9996,4016,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.97  BV=1.95
    {9996,4304,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=7.07  BV=1.85
    {9996,4608,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=7.17  BV=1.75
    {9996,4960,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=7.28  BV=1.64
    {20003,2640,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.37  BV=1.55
    {20003,2832,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.47  BV=1.45
    {20003,3040,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.57  BV=1.35
    {20003,3248,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.67  BV=1.25
    {20003,3504,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.77  BV=1.14
    {20003,3744,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.87  BV=1.05
    {29999,2688,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.39  BV=0.94
    {29999,2880,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.49  BV=0.84
    {29999,3072,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.58  BV=0.75
    {29999,3312,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.69  BV=0.64
    {29999,3520,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.78  BV=0.55
    {29999,3808,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.89  BV=0.44
    {29999,4064,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.99  BV=0.35
    {29999,4352,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.09  BV=0.25
    {29999,4656,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.18  BV=0.15
    {29999,4992,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.29  BV=0.05
    {29999,5344,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.38  BV=-0.05
    {29999,5760,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.49  BV=-0.16
    {29999,6176,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.59  BV=-0.26
    {29999,6608,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.69  BV=-0.36
    {29999,7072,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.79  BV=-0.45
    {29999,7584,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.89  BV=-0.55
    {29999,8112,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.99  BV=-0.65
    {29999,8688,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.08  BV=-0.75
    {29999,9312,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.18  BV=-0.85
    {29999,9968,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.28  BV=-0.95
    {29999,10240,1024, 0, 0, 0},  /* TV = 5.06(2857 lines)  AV=2.28  SV=8.32  BV=-0.99 */
    {29999,10240,1024, 0, 0, 0},  /* TV = 5.06(2857 lines)  AV=2.28  SV=8.32  BV=-0.99 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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
    116, //u4TotalIndex
    20, //i4StrobeTrigerBV
    104, //i4MaxBV
    -12, //i4MinBV
    90, //i4EffectiveMaxBV
    -30, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_AUTO, //ISO
    sCustom2PLineTable_60Hz,
    sCustom2PLineTable_50Hz,
    NULL,
};
static strEvPline sCustom3PLineTable_60Hz =
{
{
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {525,1200,1024, 0, 0, 0},  //TV = 10.90(50 lines)  AV=2.28  SV=5.23  BV=7.94
    {557,1200,1024, 0, 0, 0},  //TV = 10.81(53 lines)  AV=2.28  SV=5.23  BV=7.86
    {599,1200,1024, 0, 0, 0},  //TV = 10.71(57 lines)  AV=2.28  SV=5.23  BV=7.75
    {641,1200,1024, 0, 0, 0},  //TV = 10.61(61 lines)  AV=2.28  SV=5.23  BV=7.65
    {683,1216,1024, 0, 0, 0},  //TV = 10.52(65 lines)  AV=2.28  SV=5.25  BV=7.54
    {735,1200,1024, 0, 0, 0},  //TV = 10.41(70 lines)  AV=2.28  SV=5.23  BV=7.46
    {788,1200,1024, 0, 0, 0},  //TV = 10.31(75 lines)  AV=2.28  SV=5.23  BV=7.36
    {840,1216,1024, 0, 0, 0},  //TV = 10.22(80 lines)  AV=2.28  SV=5.25  BV=7.24
    {903,1200,1024, 0, 0, 0},  //TV = 10.11(86 lines)  AV=2.28  SV=5.23  BV=7.16
    {977,1200,1024, 0, 0, 0},  //TV = 10.00(93 lines)  AV=2.28  SV=5.23  BV=7.05
    {1040,1200,1024, 0, 0, 0},  //TV = 9.91(99 lines)  AV=2.28  SV=5.23  BV=6.96
    {1113,1200,1024, 0, 0, 0},  //TV = 9.81(106 lines)  AV=2.28  SV=5.23  BV=6.86
    {1197,1200,1024, 0, 0, 0},  //TV = 9.71(114 lines)  AV=2.28  SV=5.23  BV=6.75
    {1292,1200,1024, 0, 0, 0},  //TV = 9.60(123 lines)  AV=2.28  SV=5.23  BV=6.64
    {1376,1200,1024, 0, 0, 0},  //TV = 9.51(131 lines)  AV=2.28  SV=5.23  BV=6.55
    {1481,1200,1024, 0, 0, 0},  //TV = 9.40(141 lines)  AV=2.28  SV=5.23  BV=6.45
    {1575,1200,1024, 0, 0, 0},  //TV = 9.31(150 lines)  AV=2.28  SV=5.23  BV=6.36
    {1701,1200,1024, 0, 0, 0},  //TV = 9.20(162 lines)  AV=2.28  SV=5.23  BV=6.25
    {1827,1200,1024, 0, 0, 0},  //TV = 9.10(174 lines)  AV=2.28  SV=5.23  BV=6.14
    {1953,1200,1024, 0, 0, 0},  //TV = 9.00(186 lines)  AV=2.28  SV=5.23  BV=6.05
    {2090,1200,1024, 0, 0, 0},  //TV = 8.90(199 lines)  AV=2.28  SV=5.23  BV=5.95
    {2258,1200,1024, 0, 0, 0},  //TV = 8.79(215 lines)  AV=2.28  SV=5.23  BV=5.84
    {2415,1200,1024, 0, 0, 0},  //TV = 8.69(230 lines)  AV=2.28  SV=5.23  BV=5.74
    {2583,1200,1024, 0, 0, 0},  //TV = 8.60(246 lines)  AV=2.28  SV=5.23  BV=5.64
    {2762,1200,1024, 0, 0, 0},  //TV = 8.50(263 lines)  AV=2.28  SV=5.23  BV=5.55
    {2972,1200,1024, 0, 0, 0},  //TV = 8.39(283 lines)  AV=2.28  SV=5.23  BV=5.44
    {3182,1200,1024, 0, 0, 0},  //TV = 8.30(303 lines)  AV=2.28  SV=5.23  BV=5.34
    {3402,1200,1024, 0, 0, 0},  //TV = 8.20(324 lines)  AV=2.28  SV=5.23  BV=5.25
    {3644,1200,1024, 0, 0, 0},  //TV = 8.10(347 lines)  AV=2.28  SV=5.23  BV=5.15
    {3896,1200,1024, 0, 0, 0},  //TV = 8.00(371 lines)  AV=2.28  SV=5.23  BV=5.05
    {4179,1200,1024, 0, 0, 0},  //TV = 7.90(398 lines)  AV=2.28  SV=5.23  BV=4.95
    {4505,1200,1024, 0, 0, 0},  //TV = 7.79(429 lines)  AV=2.28  SV=5.23  BV=4.84
    {4820,1200,1024, 0, 0, 0},  //TV = 7.70(459 lines)  AV=2.28  SV=5.23  BV=4.74
    {5156,1200,1024, 0, 0, 0},  //TV = 7.60(491 lines)  AV=2.28  SV=5.23  BV=4.65
    {5523,1200,1024, 0, 0, 0},  //TV = 7.50(526 lines)  AV=2.28  SV=5.23  BV=4.55
    {5912,1200,1024, 0, 0, 0},  //TV = 7.40(563 lines)  AV=2.28  SV=5.23  BV=4.45
    {6332,1200,1024, 0, 0, 0},  //TV = 7.30(603 lines)  AV=2.28  SV=5.23  BV=4.35
    {6836,1200,1024, 0, 0, 0},  //TV = 7.19(651 lines)  AV=2.28  SV=5.23  BV=4.24
    {7319,1200,1024, 0, 0, 0},  //TV = 7.09(697 lines)  AV=2.28  SV=5.23  BV=4.14
    {7833,1200,1024, 0, 0, 0},  //TV = 7.00(746 lines)  AV=2.28  SV=5.23  BV=4.04
    {8338,1200,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.23  BV=3.95
    {8338,1296,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.34  BV=3.84
    {8338,1376,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.43  BV=3.75
    {8338,1488,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.54  BV=3.64
    {8338,1584,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.63  BV=3.55
    {8338,1696,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.73  BV=3.45
    {8338,1824,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.83  BV=3.35
    {8338,1952,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.93  BV=3.25
    {8338,2096,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.03  BV=3.15
    {8338,2240,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.13  BV=3.05
    {16664,1200,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.23  BV=2.95
    {16664,1296,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.34  BV=2.84
    {16664,1376,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.43  BV=2.76
    {16664,1488,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.54  BV=2.64
    {16664,1584,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.63  BV=2.55
    {16664,1696,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.73  BV=2.45
    {16664,1824,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.83  BV=2.35
    {16664,1952,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.93  BV=2.25
    {16664,2096,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.03  BV=2.15
    {16664,2256,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.14  BV=2.04
    {16664,2416,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.24  BV=1.94
    {16664,2576,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.33  BV=1.85
    {16664,2784,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.44  BV=1.74
    {16664,2976,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.54  BV=1.64
    {16664,3184,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.64  BV=1.55
    {16664,3408,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.73  BV=1.45
    {16664,3648,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.83  BV=1.35
    {25001,2608,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=6.35  BV=1.25
    {25001,2784,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=6.44  BV=1.15
    {25001,2992,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=6.55  BV=1.05
    {25001,3200,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=6.64  BV=0.95
    {25001,3440,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=6.75  BV=0.85
    {33327,2784,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.44  BV=0.74
    {33327,2976,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.54  BV=0.64
    {33327,3168,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.63  BV=0.55
    {33327,3424,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.74  BV=0.44
    {33327,3664,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.84  BV=0.34
    {33327,3920,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.94  BV=0.25
    {33327,4192,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.03  BV=0.15
    {33327,4496,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.13  BV=0.05
    {33327,4816,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.23  BV=-0.05
    {33327,5184,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.34  BV=-0.16
    {33327,5552,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.44  BV=-0.26
    {33327,5952,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.54  BV=-0.36
    {33327,6368,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.64  BV=-0.45
    {33327,6816,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.73  BV=-0.55
    {33327,7312,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.84  BV=-0.65
    {33327,7824,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.93  BV=-0.75
    {33327,8384,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.03  BV=-0.85
    {33327,8976,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.13  BV=-0.95
    {33327,9616,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.23  BV=-1.05
    {33327,10240,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.32  BV=-1.14
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {525,1200,1024, 0, 0, 0},  //TV = 10.90(50 lines)  AV=2.28  SV=5.23  BV=7.94
    {557,1200,1024, 0, 0, 0},  //TV = 10.81(53 lines)  AV=2.28  SV=5.23  BV=7.86
    {599,1200,1024, 0, 0, 0},  //TV = 10.71(57 lines)  AV=2.28  SV=5.23  BV=7.75
    {641,1200,1024, 0, 0, 0},  //TV = 10.61(61 lines)  AV=2.28  SV=5.23  BV=7.65
    {683,1216,1024, 0, 0, 0},  //TV = 10.52(65 lines)  AV=2.28  SV=5.25  BV=7.54
    {735,1200,1024, 0, 0, 0},  //TV = 10.41(70 lines)  AV=2.28  SV=5.23  BV=7.46
    {788,1200,1024, 0, 0, 0},  //TV = 10.31(75 lines)  AV=2.28  SV=5.23  BV=7.36
    {840,1216,1024, 0, 0, 0},  //TV = 10.22(80 lines)  AV=2.28  SV=5.25  BV=7.24
    {903,1200,1024, 0, 0, 0},  //TV = 10.11(86 lines)  AV=2.28  SV=5.23  BV=7.16
    {977,1200,1024, 0, 0, 0},  //TV = 10.00(93 lines)  AV=2.28  SV=5.23  BV=7.05
    {1040,1200,1024, 0, 0, 0},  //TV = 9.91(99 lines)  AV=2.28  SV=5.23  BV=6.96
    {1113,1200,1024, 0, 0, 0},  //TV = 9.81(106 lines)  AV=2.28  SV=5.23  BV=6.86
    {1197,1200,1024, 0, 0, 0},  //TV = 9.71(114 lines)  AV=2.28  SV=5.23  BV=6.75
    {1292,1200,1024, 0, 0, 0},  //TV = 9.60(123 lines)  AV=2.28  SV=5.23  BV=6.64
    {1376,1200,1024, 0, 0, 0},  //TV = 9.51(131 lines)  AV=2.28  SV=5.23  BV=6.55
    {1481,1200,1024, 0, 0, 0},  //TV = 9.40(141 lines)  AV=2.28  SV=5.23  BV=6.45
    {1575,1200,1024, 0, 0, 0},  //TV = 9.31(150 lines)  AV=2.28  SV=5.23  BV=6.36
    {1701,1200,1024, 0, 0, 0},  //TV = 9.20(162 lines)  AV=2.28  SV=5.23  BV=6.25
    {1827,1200,1024, 0, 0, 0},  //TV = 9.10(174 lines)  AV=2.28  SV=5.23  BV=6.14
    {1953,1200,1024, 0, 0, 0},  //TV = 9.00(186 lines)  AV=2.28  SV=5.23  BV=6.05
    {2090,1200,1024, 0, 0, 0},  //TV = 8.90(199 lines)  AV=2.28  SV=5.23  BV=5.95
    {2258,1200,1024, 0, 0, 0},  //TV = 8.79(215 lines)  AV=2.28  SV=5.23  BV=5.84
    {2415,1200,1024, 0, 0, 0},  //TV = 8.69(230 lines)  AV=2.28  SV=5.23  BV=5.74
    {2583,1200,1024, 0, 0, 0},  //TV = 8.60(246 lines)  AV=2.28  SV=5.23  BV=5.64
    {2762,1200,1024, 0, 0, 0},  //TV = 8.50(263 lines)  AV=2.28  SV=5.23  BV=5.55
    {2972,1200,1024, 0, 0, 0},  //TV = 8.39(283 lines)  AV=2.28  SV=5.23  BV=5.44
    {3182,1200,1024, 0, 0, 0},  //TV = 8.30(303 lines)  AV=2.28  SV=5.23  BV=5.34
    {3402,1200,1024, 0, 0, 0},  //TV = 8.20(324 lines)  AV=2.28  SV=5.23  BV=5.25
    {3644,1200,1024, 0, 0, 0},  //TV = 8.10(347 lines)  AV=2.28  SV=5.23  BV=5.15
    {3896,1200,1024, 0, 0, 0},  //TV = 8.00(371 lines)  AV=2.28  SV=5.23  BV=5.05
    {4179,1200,1024, 0, 0, 0},  //TV = 7.90(398 lines)  AV=2.28  SV=5.23  BV=4.95
    {4505,1200,1024, 0, 0, 0},  //TV = 7.79(429 lines)  AV=2.28  SV=5.23  BV=4.84
    {4820,1200,1024, 0, 0, 0},  //TV = 7.70(459 lines)  AV=2.28  SV=5.23  BV=4.74
    {5156,1200,1024, 0, 0, 0},  //TV = 7.60(491 lines)  AV=2.28  SV=5.23  BV=4.65
    {5523,1200,1024, 0, 0, 0},  //TV = 7.50(526 lines)  AV=2.28  SV=5.23  BV=4.55
    {5912,1200,1024, 0, 0, 0},  //TV = 7.40(563 lines)  AV=2.28  SV=5.23  BV=4.45
    {6332,1200,1024, 0, 0, 0},  //TV = 7.30(603 lines)  AV=2.28  SV=5.23  BV=4.35
    {6836,1200,1024, 0, 0, 0},  //TV = 7.19(651 lines)  AV=2.28  SV=5.23  BV=4.24
    {7319,1200,1024, 0, 0, 0},  //TV = 7.09(697 lines)  AV=2.28  SV=5.23  BV=4.14
    {7833,1200,1024, 0, 0, 0},  //TV = 7.00(746 lines)  AV=2.28  SV=5.23  BV=4.04
    {8400,1200,1024, 0, 0, 0},  //TV = 6.90(800 lines)  AV=2.28  SV=5.23  BV=3.94
    {8999,1200,1024, 0, 0, 0},  //TV = 6.80(857 lines)  AV=2.28  SV=5.23  BV=3.84
    {9639,1200,1024, 0, 0, 0},  //TV = 6.70(918 lines)  AV=2.28  SV=5.23  BV=3.74
    {9996,1232,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.27  BV=3.65
    {9996,1328,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.38  BV=3.54
    {9996,1424,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.48  BV=3.44
    {9996,1520,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.57  BV=3.35
    {9996,1632,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.67  BV=3.25
    {9996,1744,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.77  BV=3.15
    {9996,1872,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.87  BV=3.05
    {9996,2000,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.97  BV=2.95
    {9996,2144,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.07  BV=2.85
    {9996,2304,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.17  BV=2.75
    {9996,2464,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.27  BV=2.65
    {9996,2656,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.38  BV=2.54
    {9996,2832,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.47  BV=2.45
    {9996,3040,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.57  BV=2.35
    {9996,3264,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.67  BV=2.25
    {9996,3488,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.77  BV=2.15
    {9996,3760,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.88  BV=2.04
    {9996,4016,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.97  BV=1.95
    {9996,4304,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=7.07  BV=1.85
    {9996,4608,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=7.17  BV=1.75
    {9996,4960,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=7.28  BV=1.64
    {20003,2640,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.37  BV=1.55
    {20003,2832,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.47  BV=1.45
    {20003,3040,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.57  BV=1.35
    {20003,3248,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.67  BV=1.25
    {20003,3504,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.77  BV=1.14
    {20003,3744,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.87  BV=1.05
    {29999,2688,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.39  BV=0.94
    {29999,2880,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.49  BV=0.84
    {29999,3072,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.58  BV=0.75
    {29999,3312,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.69  BV=0.64
    {29999,3520,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.78  BV=0.55
    {29999,3808,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.89  BV=0.44
    {29999,4064,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.99  BV=0.35
    {29999,4352,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.09  BV=0.25
    {29999,4656,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.18  BV=0.15
    {29999,4992,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.29  BV=0.05
    {29999,5344,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.38  BV=-0.05
    {29999,5760,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.49  BV=-0.16
    {29999,6176,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.59  BV=-0.26
    {29999,6608,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.69  BV=-0.36
    {29999,7072,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.79  BV=-0.45
    {29999,7584,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.89  BV=-0.55
    {29999,8112,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.99  BV=-0.65
    {29999,8688,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.08  BV=-0.75
    {29999,9312,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.18  BV=-0.85
    {29999,9968,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.28  BV=-0.95
    {29999,10240,1024, 0, 0, 0},  /* TV = 5.06(2857 lines)  AV=2.28  SV=8.32  BV=-0.99 */
    {29999,10240,1024, 0, 0, 0},  /* TV = 5.06(2857 lines)  AV=2.28  SV=8.32  BV=-0.99 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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
    116, //u4TotalIndex
    20, //i4StrobeTrigerBV
    104, //i4MaxBV
    -12, //i4MinBV
    90, //i4EffectiveMaxBV
    -30, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_AUTO, //ISO
    sCustom3PLineTable_60Hz,
    sCustom3PLineTable_50Hz,
    NULL,
};
static strEvPline sCustom4PLineTable_60Hz =
{
{
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {525,1200,1024, 0, 0, 0},  //TV = 10.90(50 lines)  AV=2.28  SV=5.23  BV=7.94
    {557,1200,1024, 0, 0, 0},  //TV = 10.81(53 lines)  AV=2.28  SV=5.23  BV=7.86
    {599,1200,1024, 0, 0, 0},  //TV = 10.71(57 lines)  AV=2.28  SV=5.23  BV=7.75
    {641,1200,1024, 0, 0, 0},  //TV = 10.61(61 lines)  AV=2.28  SV=5.23  BV=7.65
    {683,1216,1024, 0, 0, 0},  //TV = 10.52(65 lines)  AV=2.28  SV=5.25  BV=7.54
    {735,1200,1024, 0, 0, 0},  //TV = 10.41(70 lines)  AV=2.28  SV=5.23  BV=7.46
    {788,1200,1024, 0, 0, 0},  //TV = 10.31(75 lines)  AV=2.28  SV=5.23  BV=7.36
    {840,1216,1024, 0, 0, 0},  //TV = 10.22(80 lines)  AV=2.28  SV=5.25  BV=7.24
    {903,1200,1024, 0, 0, 0},  //TV = 10.11(86 lines)  AV=2.28  SV=5.23  BV=7.16
    {977,1200,1024, 0, 0, 0},  //TV = 10.00(93 lines)  AV=2.28  SV=5.23  BV=7.05
    {1040,1200,1024, 0, 0, 0},  //TV = 9.91(99 lines)  AV=2.28  SV=5.23  BV=6.96
    {1113,1200,1024, 0, 0, 0},  //TV = 9.81(106 lines)  AV=2.28  SV=5.23  BV=6.86
    {1197,1200,1024, 0, 0, 0},  //TV = 9.71(114 lines)  AV=2.28  SV=5.23  BV=6.75
    {1292,1200,1024, 0, 0, 0},  //TV = 9.60(123 lines)  AV=2.28  SV=5.23  BV=6.64
    {1376,1200,1024, 0, 0, 0},  //TV = 9.51(131 lines)  AV=2.28  SV=5.23  BV=6.55
    {1481,1200,1024, 0, 0, 0},  //TV = 9.40(141 lines)  AV=2.28  SV=5.23  BV=6.45
    {1575,1200,1024, 0, 0, 0},  //TV = 9.31(150 lines)  AV=2.28  SV=5.23  BV=6.36
    {1701,1200,1024, 0, 0, 0},  //TV = 9.20(162 lines)  AV=2.28  SV=5.23  BV=6.25
    {1827,1200,1024, 0, 0, 0},  //TV = 9.10(174 lines)  AV=2.28  SV=5.23  BV=6.14
    {1953,1200,1024, 0, 0, 0},  //TV = 9.00(186 lines)  AV=2.28  SV=5.23  BV=6.05
    {2090,1200,1024, 0, 0, 0},  //TV = 8.90(199 lines)  AV=2.28  SV=5.23  BV=5.95
    {2258,1200,1024, 0, 0, 0},  //TV = 8.79(215 lines)  AV=2.28  SV=5.23  BV=5.84
    {2415,1200,1024, 0, 0, 0},  //TV = 8.69(230 lines)  AV=2.28  SV=5.23  BV=5.74
    {2583,1200,1024, 0, 0, 0},  //TV = 8.60(246 lines)  AV=2.28  SV=5.23  BV=5.64
    {2762,1200,1024, 0, 0, 0},  //TV = 8.50(263 lines)  AV=2.28  SV=5.23  BV=5.55
    {2972,1200,1024, 0, 0, 0},  //TV = 8.39(283 lines)  AV=2.28  SV=5.23  BV=5.44
    {3182,1200,1024, 0, 0, 0},  //TV = 8.30(303 lines)  AV=2.28  SV=5.23  BV=5.34
    {3402,1200,1024, 0, 0, 0},  //TV = 8.20(324 lines)  AV=2.28  SV=5.23  BV=5.25
    {3644,1200,1024, 0, 0, 0},  //TV = 8.10(347 lines)  AV=2.28  SV=5.23  BV=5.15
    {3896,1200,1024, 0, 0, 0},  //TV = 8.00(371 lines)  AV=2.28  SV=5.23  BV=5.05
    {4179,1200,1024, 0, 0, 0},  //TV = 7.90(398 lines)  AV=2.28  SV=5.23  BV=4.95
    {4505,1200,1024, 0, 0, 0},  //TV = 7.79(429 lines)  AV=2.28  SV=5.23  BV=4.84
    {4820,1200,1024, 0, 0, 0},  //TV = 7.70(459 lines)  AV=2.28  SV=5.23  BV=4.74
    {5156,1200,1024, 0, 0, 0},  //TV = 7.60(491 lines)  AV=2.28  SV=5.23  BV=4.65
    {5523,1200,1024, 0, 0, 0},  //TV = 7.50(526 lines)  AV=2.28  SV=5.23  BV=4.55
    {5912,1200,1024, 0, 0, 0},  //TV = 7.40(563 lines)  AV=2.28  SV=5.23  BV=4.45
    {6332,1200,1024, 0, 0, 0},  //TV = 7.30(603 lines)  AV=2.28  SV=5.23  BV=4.35
    {6836,1200,1024, 0, 0, 0},  //TV = 7.19(651 lines)  AV=2.28  SV=5.23  BV=4.24
    {7319,1200,1024, 0, 0, 0},  //TV = 7.09(697 lines)  AV=2.28  SV=5.23  BV=4.14
    {7833,1200,1024, 0, 0, 0},  //TV = 7.00(746 lines)  AV=2.28  SV=5.23  BV=4.04
    {8338,1200,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.23  BV=3.95
    {8338,1296,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.34  BV=3.84
    {8338,1376,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.43  BV=3.75
    {8338,1488,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.54  BV=3.64
    {8338,1584,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.63  BV=3.55
    {8338,1696,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.73  BV=3.45
    {8338,1824,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.83  BV=3.35
    {8338,1952,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.93  BV=3.25
    {8338,2096,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.03  BV=3.15
    {8338,2240,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.13  BV=3.05
    {16664,1200,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.23  BV=2.95
    {16664,1296,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.34  BV=2.84
    {16664,1376,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.43  BV=2.76
    {16664,1488,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.54  BV=2.64
    {16664,1584,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.63  BV=2.55
    {16664,1696,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.73  BV=2.45
    {16664,1824,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.83  BV=2.35
    {16664,1952,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.93  BV=2.25
    {16664,2096,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.03  BV=2.15
    {16664,2256,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.14  BV=2.04
    {16664,2416,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.24  BV=1.94
    {16664,2576,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.33  BV=1.85
    {16664,2784,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.44  BV=1.74
    {16664,2976,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.54  BV=1.64
    {16664,3184,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.64  BV=1.55
    {16664,3408,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.73  BV=1.45
    {16664,3648,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.83  BV=1.35
    {25001,2608,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=6.35  BV=1.25
    {25001,2784,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=6.44  BV=1.15
    {25001,2992,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=6.55  BV=1.05
    {25001,3200,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=6.64  BV=0.95
    {25001,3440,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=6.75  BV=0.85
    {33327,2784,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.44  BV=0.74
    {33327,2976,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.54  BV=0.64
    {33327,3168,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.63  BV=0.55
    {33327,3424,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.74  BV=0.44
    {33327,3664,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.84  BV=0.34
    {33327,3920,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.94  BV=0.25
    {33327,4192,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.03  BV=0.15
    {33327,4496,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.13  BV=0.05
    {33327,4816,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.23  BV=-0.05
    {33327,5184,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.34  BV=-0.16
    {33327,5552,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.44  BV=-0.26
    {33327,5952,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.54  BV=-0.36
    {33327,6368,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.64  BV=-0.45
    {33327,6816,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.73  BV=-0.55
    {33327,7312,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.84  BV=-0.65
    {33327,7824,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.93  BV=-0.75
    {33327,8384,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.03  BV=-0.85
    {33327,8976,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.13  BV=-0.95
    {33327,9616,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.23  BV=-1.05
    {33327,10240,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.32  BV=-1.14
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {525,1200,1024, 0, 0, 0},  //TV = 10.90(50 lines)  AV=2.28  SV=5.23  BV=7.94
    {557,1200,1024, 0, 0, 0},  //TV = 10.81(53 lines)  AV=2.28  SV=5.23  BV=7.86
    {599,1200,1024, 0, 0, 0},  //TV = 10.71(57 lines)  AV=2.28  SV=5.23  BV=7.75
    {641,1200,1024, 0, 0, 0},  //TV = 10.61(61 lines)  AV=2.28  SV=5.23  BV=7.65
    {683,1216,1024, 0, 0, 0},  //TV = 10.52(65 lines)  AV=2.28  SV=5.25  BV=7.54
    {735,1200,1024, 0, 0, 0},  //TV = 10.41(70 lines)  AV=2.28  SV=5.23  BV=7.46
    {788,1200,1024, 0, 0, 0},  //TV = 10.31(75 lines)  AV=2.28  SV=5.23  BV=7.36
    {840,1216,1024, 0, 0, 0},  //TV = 10.22(80 lines)  AV=2.28  SV=5.25  BV=7.24
    {903,1200,1024, 0, 0, 0},  //TV = 10.11(86 lines)  AV=2.28  SV=5.23  BV=7.16
    {977,1200,1024, 0, 0, 0},  //TV = 10.00(93 lines)  AV=2.28  SV=5.23  BV=7.05
    {1040,1200,1024, 0, 0, 0},  //TV = 9.91(99 lines)  AV=2.28  SV=5.23  BV=6.96
    {1113,1200,1024, 0, 0, 0},  //TV = 9.81(106 lines)  AV=2.28  SV=5.23  BV=6.86
    {1197,1200,1024, 0, 0, 0},  //TV = 9.71(114 lines)  AV=2.28  SV=5.23  BV=6.75
    {1292,1200,1024, 0, 0, 0},  //TV = 9.60(123 lines)  AV=2.28  SV=5.23  BV=6.64
    {1376,1200,1024, 0, 0, 0},  //TV = 9.51(131 lines)  AV=2.28  SV=5.23  BV=6.55
    {1481,1200,1024, 0, 0, 0},  //TV = 9.40(141 lines)  AV=2.28  SV=5.23  BV=6.45
    {1575,1200,1024, 0, 0, 0},  //TV = 9.31(150 lines)  AV=2.28  SV=5.23  BV=6.36
    {1701,1200,1024, 0, 0, 0},  //TV = 9.20(162 lines)  AV=2.28  SV=5.23  BV=6.25
    {1827,1200,1024, 0, 0, 0},  //TV = 9.10(174 lines)  AV=2.28  SV=5.23  BV=6.14
    {1953,1200,1024, 0, 0, 0},  //TV = 9.00(186 lines)  AV=2.28  SV=5.23  BV=6.05
    {2090,1200,1024, 0, 0, 0},  //TV = 8.90(199 lines)  AV=2.28  SV=5.23  BV=5.95
    {2258,1200,1024, 0, 0, 0},  //TV = 8.79(215 lines)  AV=2.28  SV=5.23  BV=5.84
    {2415,1200,1024, 0, 0, 0},  //TV = 8.69(230 lines)  AV=2.28  SV=5.23  BV=5.74
    {2583,1200,1024, 0, 0, 0},  //TV = 8.60(246 lines)  AV=2.28  SV=5.23  BV=5.64
    {2762,1200,1024, 0, 0, 0},  //TV = 8.50(263 lines)  AV=2.28  SV=5.23  BV=5.55
    {2972,1200,1024, 0, 0, 0},  //TV = 8.39(283 lines)  AV=2.28  SV=5.23  BV=5.44
    {3182,1200,1024, 0, 0, 0},  //TV = 8.30(303 lines)  AV=2.28  SV=5.23  BV=5.34
    {3402,1200,1024, 0, 0, 0},  //TV = 8.20(324 lines)  AV=2.28  SV=5.23  BV=5.25
    {3644,1200,1024, 0, 0, 0},  //TV = 8.10(347 lines)  AV=2.28  SV=5.23  BV=5.15
    {3896,1200,1024, 0, 0, 0},  //TV = 8.00(371 lines)  AV=2.28  SV=5.23  BV=5.05
    {4179,1200,1024, 0, 0, 0},  //TV = 7.90(398 lines)  AV=2.28  SV=5.23  BV=4.95
    {4505,1200,1024, 0, 0, 0},  //TV = 7.79(429 lines)  AV=2.28  SV=5.23  BV=4.84
    {4820,1200,1024, 0, 0, 0},  //TV = 7.70(459 lines)  AV=2.28  SV=5.23  BV=4.74
    {5156,1200,1024, 0, 0, 0},  //TV = 7.60(491 lines)  AV=2.28  SV=5.23  BV=4.65
    {5523,1200,1024, 0, 0, 0},  //TV = 7.50(526 lines)  AV=2.28  SV=5.23  BV=4.55
    {5912,1200,1024, 0, 0, 0},  //TV = 7.40(563 lines)  AV=2.28  SV=5.23  BV=4.45
    {6332,1200,1024, 0, 0, 0},  //TV = 7.30(603 lines)  AV=2.28  SV=5.23  BV=4.35
    {6836,1200,1024, 0, 0, 0},  //TV = 7.19(651 lines)  AV=2.28  SV=5.23  BV=4.24
    {7319,1200,1024, 0, 0, 0},  //TV = 7.09(697 lines)  AV=2.28  SV=5.23  BV=4.14
    {7833,1200,1024, 0, 0, 0},  //TV = 7.00(746 lines)  AV=2.28  SV=5.23  BV=4.04
    {8400,1200,1024, 0, 0, 0},  //TV = 6.90(800 lines)  AV=2.28  SV=5.23  BV=3.94
    {8999,1200,1024, 0, 0, 0},  //TV = 6.80(857 lines)  AV=2.28  SV=5.23  BV=3.84
    {9639,1200,1024, 0, 0, 0},  //TV = 6.70(918 lines)  AV=2.28  SV=5.23  BV=3.74
    {9996,1232,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.27  BV=3.65
    {9996,1328,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.38  BV=3.54
    {9996,1424,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.48  BV=3.44
    {9996,1520,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.57  BV=3.35
    {9996,1632,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.67  BV=3.25
    {9996,1744,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.77  BV=3.15
    {9996,1872,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.87  BV=3.05
    {9996,2000,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.97  BV=2.95
    {9996,2144,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.07  BV=2.85
    {9996,2304,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.17  BV=2.75
    {9996,2464,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.27  BV=2.65
    {9996,2656,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.38  BV=2.54
    {9996,2832,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.47  BV=2.45
    {9996,3040,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.57  BV=2.35
    {9996,3264,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.67  BV=2.25
    {9996,3488,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.77  BV=2.15
    {9996,3760,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.88  BV=2.04
    {9996,4016,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.97  BV=1.95
    {9996,4304,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=7.07  BV=1.85
    {9996,4608,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=7.17  BV=1.75
    {9996,4960,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=7.28  BV=1.64
    {20003,2640,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.37  BV=1.55
    {20003,2832,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.47  BV=1.45
    {20003,3040,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.57  BV=1.35
    {20003,3248,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.67  BV=1.25
    {20003,3504,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.77  BV=1.14
    {20003,3744,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.87  BV=1.05
    {29999,2688,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.39  BV=0.94
    {29999,2880,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.49  BV=0.84
    {29999,3072,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.58  BV=0.75
    {29999,3312,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.69  BV=0.64
    {29999,3520,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.78  BV=0.55
    {29999,3808,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.89  BV=0.44
    {29999,4064,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.99  BV=0.35
    {29999,4352,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.09  BV=0.25
    {29999,4656,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.18  BV=0.15
    {29999,4992,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.29  BV=0.05
    {29999,5344,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.38  BV=-0.05
    {29999,5760,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.49  BV=-0.16
    {29999,6176,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.59  BV=-0.26
    {29999,6608,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.69  BV=-0.36
    {29999,7072,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.79  BV=-0.45
    {29999,7584,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.89  BV=-0.55
    {29999,8112,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.99  BV=-0.65
    {29999,8688,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.08  BV=-0.75
    {29999,9312,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.18  BV=-0.85
    {29999,9968,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.28  BV=-0.95
    {29999,10240,1024, 0, 0, 0},  /* TV = 5.06(2857 lines)  AV=2.28  SV=8.32  BV=-0.99 */
    {29999,10240,1024, 0, 0, 0},  /* TV = 5.06(2857 lines)  AV=2.28  SV=8.32  BV=-0.99 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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
    116, //u4TotalIndex
    20, //i4StrobeTrigerBV
    104, //i4MaxBV
    -12, //i4MinBV
    90, //i4EffectiveMaxBV
    -30, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_AUTO, //ISO
    sCustom4PLineTable_60Hz,
    sCustom4PLineTable_50Hz,
    NULL,
};
static strEvPline sCustom5PLineTable_60Hz =
{
{
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {525,1200,1024, 0, 0, 0},  //TV = 10.90(50 lines)  AV=2.28  SV=5.23  BV=7.94
    {557,1200,1024, 0, 0, 0},  //TV = 10.81(53 lines)  AV=2.28  SV=5.23  BV=7.86
    {599,1200,1024, 0, 0, 0},  //TV = 10.71(57 lines)  AV=2.28  SV=5.23  BV=7.75
    {641,1200,1024, 0, 0, 0},  //TV = 10.61(61 lines)  AV=2.28  SV=5.23  BV=7.65
    {683,1216,1024, 0, 0, 0},  //TV = 10.52(65 lines)  AV=2.28  SV=5.25  BV=7.54
    {735,1200,1024, 0, 0, 0},  //TV = 10.41(70 lines)  AV=2.28  SV=5.23  BV=7.46
    {788,1200,1024, 0, 0, 0},  //TV = 10.31(75 lines)  AV=2.28  SV=5.23  BV=7.36
    {840,1216,1024, 0, 0, 0},  //TV = 10.22(80 lines)  AV=2.28  SV=5.25  BV=7.24
    {903,1200,1024, 0, 0, 0},  //TV = 10.11(86 lines)  AV=2.28  SV=5.23  BV=7.16
    {977,1200,1024, 0, 0, 0},  //TV = 10.00(93 lines)  AV=2.28  SV=5.23  BV=7.05
    {1040,1200,1024, 0, 0, 0},  //TV = 9.91(99 lines)  AV=2.28  SV=5.23  BV=6.96
    {1113,1200,1024, 0, 0, 0},  //TV = 9.81(106 lines)  AV=2.28  SV=5.23  BV=6.86
    {1197,1200,1024, 0, 0, 0},  //TV = 9.71(114 lines)  AV=2.28  SV=5.23  BV=6.75
    {1292,1200,1024, 0, 0, 0},  //TV = 9.60(123 lines)  AV=2.28  SV=5.23  BV=6.64
    {1376,1200,1024, 0, 0, 0},  //TV = 9.51(131 lines)  AV=2.28  SV=5.23  BV=6.55
    {1481,1200,1024, 0, 0, 0},  //TV = 9.40(141 lines)  AV=2.28  SV=5.23  BV=6.45
    {1575,1200,1024, 0, 0, 0},  //TV = 9.31(150 lines)  AV=2.28  SV=5.23  BV=6.36
    {1701,1200,1024, 0, 0, 0},  //TV = 9.20(162 lines)  AV=2.28  SV=5.23  BV=6.25
    {1827,1200,1024, 0, 0, 0},  //TV = 9.10(174 lines)  AV=2.28  SV=5.23  BV=6.14
    {1953,1200,1024, 0, 0, 0},  //TV = 9.00(186 lines)  AV=2.28  SV=5.23  BV=6.05
    {2090,1200,1024, 0, 0, 0},  //TV = 8.90(199 lines)  AV=2.28  SV=5.23  BV=5.95
    {2258,1200,1024, 0, 0, 0},  //TV = 8.79(215 lines)  AV=2.28  SV=5.23  BV=5.84
    {2415,1200,1024, 0, 0, 0},  //TV = 8.69(230 lines)  AV=2.28  SV=5.23  BV=5.74
    {2583,1200,1024, 0, 0, 0},  //TV = 8.60(246 lines)  AV=2.28  SV=5.23  BV=5.64
    {2762,1200,1024, 0, 0, 0},  //TV = 8.50(263 lines)  AV=2.28  SV=5.23  BV=5.55
    {2972,1200,1024, 0, 0, 0},  //TV = 8.39(283 lines)  AV=2.28  SV=5.23  BV=5.44
    {3182,1200,1024, 0, 0, 0},  //TV = 8.30(303 lines)  AV=2.28  SV=5.23  BV=5.34
    {3402,1200,1024, 0, 0, 0},  //TV = 8.20(324 lines)  AV=2.28  SV=5.23  BV=5.25
    {3644,1200,1024, 0, 0, 0},  //TV = 8.10(347 lines)  AV=2.28  SV=5.23  BV=5.15
    {3896,1200,1024, 0, 0, 0},  //TV = 8.00(371 lines)  AV=2.28  SV=5.23  BV=5.05
    {4179,1200,1024, 0, 0, 0},  //TV = 7.90(398 lines)  AV=2.28  SV=5.23  BV=4.95
    {4505,1200,1024, 0, 0, 0},  //TV = 7.79(429 lines)  AV=2.28  SV=5.23  BV=4.84
    {4820,1200,1024, 0, 0, 0},  //TV = 7.70(459 lines)  AV=2.28  SV=5.23  BV=4.74
    {5156,1200,1024, 0, 0, 0},  //TV = 7.60(491 lines)  AV=2.28  SV=5.23  BV=4.65
    {5523,1200,1024, 0, 0, 0},  //TV = 7.50(526 lines)  AV=2.28  SV=5.23  BV=4.55
    {5912,1200,1024, 0, 0, 0},  //TV = 7.40(563 lines)  AV=2.28  SV=5.23  BV=4.45
    {6332,1200,1024, 0, 0, 0},  //TV = 7.30(603 lines)  AV=2.28  SV=5.23  BV=4.35
    {6836,1200,1024, 0, 0, 0},  //TV = 7.19(651 lines)  AV=2.28  SV=5.23  BV=4.24
    {7319,1200,1024, 0, 0, 0},  //TV = 7.09(697 lines)  AV=2.28  SV=5.23  BV=4.14
    {7833,1200,1024, 0, 0, 0},  //TV = 7.00(746 lines)  AV=2.28  SV=5.23  BV=4.04
    {8338,1200,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.23  BV=3.95
    {8338,1296,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.34  BV=3.84
    {8338,1376,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.43  BV=3.75
    {8338,1488,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.54  BV=3.64
    {8338,1584,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.63  BV=3.55
    {8338,1696,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.73  BV=3.45
    {8338,1824,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.83  BV=3.35
    {8338,1952,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.93  BV=3.25
    {8338,2096,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.03  BV=3.15
    {8338,2240,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.13  BV=3.05
    {16664,1200,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.23  BV=2.95
    {16664,1296,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.34  BV=2.84
    {16664,1376,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.43  BV=2.76
    {16664,1488,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.54  BV=2.64
    {16664,1584,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.63  BV=2.55
    {16664,1696,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.73  BV=2.45
    {16664,1824,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.83  BV=2.35
    {16664,1952,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.93  BV=2.25
    {16664,2096,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.03  BV=2.15
    {16664,2256,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.14  BV=2.04
    {16664,2416,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.24  BV=1.94
    {16664,2576,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.33  BV=1.85
    {16664,2784,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.44  BV=1.74
    {16664,2976,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.54  BV=1.64
    {16664,3184,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.64  BV=1.55
    {16664,3408,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.73  BV=1.45
    {16664,3648,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.83  BV=1.35
    {25001,2608,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=6.35  BV=1.25
    {25001,2784,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=6.44  BV=1.15
    {25001,2992,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=6.55  BV=1.05
    {25001,3200,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=6.64  BV=0.95
    {25001,3440,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=6.75  BV=0.85
    {33327,2784,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.44  BV=0.74
    {33327,2976,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.54  BV=0.64
    {33327,3168,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.63  BV=0.55
    {33327,3424,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.74  BV=0.44
    {33327,3664,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.84  BV=0.34
    {33327,3920,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.94  BV=0.25
    {33327,4192,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.03  BV=0.15
    {33327,4496,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.13  BV=0.05
    {33327,4816,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.23  BV=-0.05
    {33327,5184,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.34  BV=-0.16
    {33327,5552,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.44  BV=-0.26
    {33327,5952,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.54  BV=-0.36
    {33327,6368,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.64  BV=-0.45
    {33327,6816,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.73  BV=-0.55
    {33327,7312,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.84  BV=-0.65
    {33327,7824,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.93  BV=-0.75
    {33327,8384,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.03  BV=-0.85
    {33327,8976,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.13  BV=-0.95
    {33327,9616,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.23  BV=-1.05
    {33327,10240,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.32  BV=-1.14
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {525,1200,1024, 0, 0, 0},  //TV = 10.90(50 lines)  AV=2.28  SV=5.23  BV=7.94
    {557,1200,1024, 0, 0, 0},  //TV = 10.81(53 lines)  AV=2.28  SV=5.23  BV=7.86
    {599,1200,1024, 0, 0, 0},  //TV = 10.71(57 lines)  AV=2.28  SV=5.23  BV=7.75
    {641,1200,1024, 0, 0, 0},  //TV = 10.61(61 lines)  AV=2.28  SV=5.23  BV=7.65
    {683,1216,1024, 0, 0, 0},  //TV = 10.52(65 lines)  AV=2.28  SV=5.25  BV=7.54
    {735,1200,1024, 0, 0, 0},  //TV = 10.41(70 lines)  AV=2.28  SV=5.23  BV=7.46
    {788,1200,1024, 0, 0, 0},  //TV = 10.31(75 lines)  AV=2.28  SV=5.23  BV=7.36
    {840,1216,1024, 0, 0, 0},  //TV = 10.22(80 lines)  AV=2.28  SV=5.25  BV=7.24
    {903,1200,1024, 0, 0, 0},  //TV = 10.11(86 lines)  AV=2.28  SV=5.23  BV=7.16
    {977,1200,1024, 0, 0, 0},  //TV = 10.00(93 lines)  AV=2.28  SV=5.23  BV=7.05
    {1040,1200,1024, 0, 0, 0},  //TV = 9.91(99 lines)  AV=2.28  SV=5.23  BV=6.96
    {1113,1200,1024, 0, 0, 0},  //TV = 9.81(106 lines)  AV=2.28  SV=5.23  BV=6.86
    {1197,1200,1024, 0, 0, 0},  //TV = 9.71(114 lines)  AV=2.28  SV=5.23  BV=6.75
    {1292,1200,1024, 0, 0, 0},  //TV = 9.60(123 lines)  AV=2.28  SV=5.23  BV=6.64
    {1376,1200,1024, 0, 0, 0},  //TV = 9.51(131 lines)  AV=2.28  SV=5.23  BV=6.55
    {1481,1200,1024, 0, 0, 0},  //TV = 9.40(141 lines)  AV=2.28  SV=5.23  BV=6.45
    {1575,1200,1024, 0, 0, 0},  //TV = 9.31(150 lines)  AV=2.28  SV=5.23  BV=6.36
    {1701,1200,1024, 0, 0, 0},  //TV = 9.20(162 lines)  AV=2.28  SV=5.23  BV=6.25
    {1827,1200,1024, 0, 0, 0},  //TV = 9.10(174 lines)  AV=2.28  SV=5.23  BV=6.14
    {1953,1200,1024, 0, 0, 0},  //TV = 9.00(186 lines)  AV=2.28  SV=5.23  BV=6.05
    {2090,1200,1024, 0, 0, 0},  //TV = 8.90(199 lines)  AV=2.28  SV=5.23  BV=5.95
    {2258,1200,1024, 0, 0, 0},  //TV = 8.79(215 lines)  AV=2.28  SV=5.23  BV=5.84
    {2415,1200,1024, 0, 0, 0},  //TV = 8.69(230 lines)  AV=2.28  SV=5.23  BV=5.74
    {2583,1200,1024, 0, 0, 0},  //TV = 8.60(246 lines)  AV=2.28  SV=5.23  BV=5.64
    {2762,1200,1024, 0, 0, 0},  //TV = 8.50(263 lines)  AV=2.28  SV=5.23  BV=5.55
    {2972,1200,1024, 0, 0, 0},  //TV = 8.39(283 lines)  AV=2.28  SV=5.23  BV=5.44
    {3182,1200,1024, 0, 0, 0},  //TV = 8.30(303 lines)  AV=2.28  SV=5.23  BV=5.34
    {3402,1200,1024, 0, 0, 0},  //TV = 8.20(324 lines)  AV=2.28  SV=5.23  BV=5.25
    {3644,1200,1024, 0, 0, 0},  //TV = 8.10(347 lines)  AV=2.28  SV=5.23  BV=5.15
    {3896,1200,1024, 0, 0, 0},  //TV = 8.00(371 lines)  AV=2.28  SV=5.23  BV=5.05
    {4179,1200,1024, 0, 0, 0},  //TV = 7.90(398 lines)  AV=2.28  SV=5.23  BV=4.95
    {4505,1200,1024, 0, 0, 0},  //TV = 7.79(429 lines)  AV=2.28  SV=5.23  BV=4.84
    {4820,1200,1024, 0, 0, 0},  //TV = 7.70(459 lines)  AV=2.28  SV=5.23  BV=4.74
    {5156,1200,1024, 0, 0, 0},  //TV = 7.60(491 lines)  AV=2.28  SV=5.23  BV=4.65
    {5523,1200,1024, 0, 0, 0},  //TV = 7.50(526 lines)  AV=2.28  SV=5.23  BV=4.55
    {5912,1200,1024, 0, 0, 0},  //TV = 7.40(563 lines)  AV=2.28  SV=5.23  BV=4.45
    {6332,1200,1024, 0, 0, 0},  //TV = 7.30(603 lines)  AV=2.28  SV=5.23  BV=4.35
    {6836,1200,1024, 0, 0, 0},  //TV = 7.19(651 lines)  AV=2.28  SV=5.23  BV=4.24
    {7319,1200,1024, 0, 0, 0},  //TV = 7.09(697 lines)  AV=2.28  SV=5.23  BV=4.14
    {7833,1200,1024, 0, 0, 0},  //TV = 7.00(746 lines)  AV=2.28  SV=5.23  BV=4.04
    {8400,1200,1024, 0, 0, 0},  //TV = 6.90(800 lines)  AV=2.28  SV=5.23  BV=3.94
    {8999,1200,1024, 0, 0, 0},  //TV = 6.80(857 lines)  AV=2.28  SV=5.23  BV=3.84
    {9639,1200,1024, 0, 0, 0},  //TV = 6.70(918 lines)  AV=2.28  SV=5.23  BV=3.74
    {9996,1232,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.27  BV=3.65
    {9996,1328,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.38  BV=3.54
    {9996,1424,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.48  BV=3.44
    {9996,1520,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.57  BV=3.35
    {9996,1632,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.67  BV=3.25
    {9996,1744,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.77  BV=3.15
    {9996,1872,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.87  BV=3.05
    {9996,2000,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.97  BV=2.95
    {9996,2144,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.07  BV=2.85
    {9996,2304,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.17  BV=2.75
    {9996,2464,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.27  BV=2.65
    {9996,2656,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.38  BV=2.54
    {9996,2832,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.47  BV=2.45
    {9996,3040,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.57  BV=2.35
    {9996,3264,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.67  BV=2.25
    {9996,3488,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.77  BV=2.15
    {9996,3760,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.88  BV=2.04
    {9996,4016,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.97  BV=1.95
    {9996,4304,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=7.07  BV=1.85
    {9996,4608,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=7.17  BV=1.75
    {9996,4960,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=7.28  BV=1.64
    {20003,2640,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.37  BV=1.55
    {20003,2832,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.47  BV=1.45
    {20003,3040,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.57  BV=1.35
    {20003,3248,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.67  BV=1.25
    {20003,3504,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.77  BV=1.14
    {20003,3744,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.87  BV=1.05
    {29999,2688,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.39  BV=0.94
    {29999,2880,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.49  BV=0.84
    {29999,3072,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.58  BV=0.75
    {29999,3312,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.69  BV=0.64
    {29999,3520,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.78  BV=0.55
    {29999,3808,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.89  BV=0.44
    {29999,4064,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.99  BV=0.35
    {29999,4352,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.09  BV=0.25
    {29999,4656,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.18  BV=0.15
    {29999,4992,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.29  BV=0.05
    {29999,5344,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.38  BV=-0.05
    {29999,5760,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.49  BV=-0.16
    {29999,6176,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.59  BV=-0.26
    {29999,6608,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.69  BV=-0.36
    {29999,7072,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.79  BV=-0.45
    {29999,7584,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.89  BV=-0.55
    {29999,8112,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.99  BV=-0.65
    {29999,8688,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.08  BV=-0.75
    {29999,9312,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.18  BV=-0.85
    {29999,9968,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.28  BV=-0.95
    {29999,10240,1024, 0, 0, 0},  /* TV = 5.06(2857 lines)  AV=2.28  SV=8.32  BV=-0.99 */
    {29999,10240,1024, 0, 0, 0},  /* TV = 5.06(2857 lines)  AV=2.28  SV=8.32  BV=-0.99 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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
    116, //u4TotalIndex
    20, //i4StrobeTrigerBV
    104, //i4MaxBV
    -12, //i4MinBV
    90, //i4EffectiveMaxBV
    -30, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_AUTO, //ISO
    sCustom5PLineTable_60Hz,
    sCustom5PLineTable_50Hz,
    NULL,
};
static strEvPline sVideoNightPLineTable_60Hz =
{
{
    {94,1280,1024, 0, 0, 0},  //TV = 13.38(9 lines)  AV=2.28  SV=5.32  BV=10.33
    {104,1216,1024, 0, 0, 0},  //TV = 13.23(10 lines)  AV=2.28  SV=5.25  BV=10.26
    {115,1200,1024, 0, 0, 0},  //TV = 13.09(11 lines)  AV=2.28  SV=5.23  BV=10.13
    {115,1280,1024, 0, 0, 0},  //TV = 13.09(11 lines)  AV=2.28  SV=5.32  BV=10.04
    {125,1248,1024, 0, 0, 0},  //TV = 12.97(12 lines)  AV=2.28  SV=5.29  BV=9.96
    {135,1264,1024, 0, 0, 0},  //TV = 12.85(13 lines)  AV=2.28  SV=5.30  BV=9.83
    {146,1264,1024, 0, 0, 0},  //TV = 12.74(14 lines)  AV=2.28  SV=5.30  BV=9.71
    {166,1200,1024, 0, 0, 0},  //TV = 12.56(16 lines)  AV=2.28  SV=5.23  BV=9.60
    {177,1216,1024, 0, 0, 0},  //TV = 12.46(17 lines)  AV=2.28  SV=5.25  BV=9.49
    {177,1264,1024, 0, 0, 0},  /* TV = 12.46(17 lines)  AV=2.28  SV=5.30  BV=9.44 */
    {198,1216,1024, 0, 0, 0},  //TV = 12.30(19 lines)  AV=2.28  SV=5.25  BV=9.33
    {208,1216,1024, 0, 0, 0},  //TV = 12.23(20 lines)  AV=2.28  SV=5.25  BV=9.26
    {218,1248,1024, 0, 0, 0},  //TV = 12.16(21 lines)  AV=2.28  SV=5.29  BV=9.15
    {239,1216,1024, 0, 0, 0},  //TV = 12.03(23 lines)  AV=2.28  SV=5.25  BV=9.06
    {260,1200,1024, 0, 0, 0},  //TV = 11.91(25 lines)  AV=2.28  SV=5.23  BV=8.96
    {270,1248,1024, 0, 0, 0},  //TV = 11.85(26 lines)  AV=2.28  SV=5.29  BV=8.84
    {291,1232,1024, 0, 0, 0},  //TV = 11.75(28 lines)  AV=2.28  SV=5.27  BV=8.75
    {322,1200,1024, 0, 0, 0},  //TV = 11.60(31 lines)  AV=2.28  SV=5.23  BV=8.65
    {343,1200,1024, 0, 0, 0},  //TV = 11.51(33 lines)  AV=2.28  SV=5.23  BV=8.56
    {363,1216,1024, 0, 0, 0},  //TV = 11.43(35 lines)  AV=2.28  SV=5.25  BV=8.45
    {395,1200,1024, 0, 0, 0},  //TV = 11.31(38 lines)  AV=2.28  SV=5.23  BV=8.35
    {426,1200,1024, 0, 0, 0},  //TV = 11.20(41 lines)  AV=2.28  SV=5.23  BV=8.24
    {446,1216,1024, 0, 0, 0},  //TV = 11.13(43 lines)  AV=2.28  SV=5.25  BV=8.16
    {478,1216,1024, 0, 0, 0},  //TV = 11.03(46 lines)  AV=2.28  SV=5.25  BV=8.06
    {519,1200,1024, 0, 0, 0},  //TV = 10.91(50 lines)  AV=2.28  SV=5.23  BV=7.96
    {561,1200,1024, 0, 0, 0},  //TV = 10.80(54 lines)  AV=2.28  SV=5.23  BV=7.85
    {592,1216,1024, 0, 0, 0},  //TV = 10.72(57 lines)  AV=2.28  SV=5.25  BV=7.75
    {644,1200,1024, 0, 0, 0},  //TV = 10.60(62 lines)  AV=2.28  SV=5.23  BV=7.65
    {685,1200,1024, 0, 0, 0},  //TV = 10.51(66 lines)  AV=2.28  SV=5.23  BV=7.56
    {737,1200,1024, 0, 0, 0},  //TV = 10.41(71 lines)  AV=2.28  SV=5.23  BV=7.45
    {789,1200,1024, 0, 0, 0},  //TV = 10.31(76 lines)  AV=2.28  SV=5.23  BV=7.35
    {841,1216,1024, 0, 0, 0},  //TV = 10.22(81 lines)  AV=2.28  SV=5.25  BV=7.24
    {903,1200,1024, 0, 0, 0},  //TV = 10.11(87 lines)  AV=2.28  SV=5.23  BV=7.16
    {975,1200,1024, 0, 0, 0},  //TV = 10.00(94 lines)  AV=2.28  SV=5.23  BV=7.05
    {1038,1200,1024, 0, 0, 0},  //TV = 9.91(100 lines)  AV=2.28  SV=5.23  BV=6.96
    {1121,1200,1024, 0, 0, 0},  //TV = 9.80(108 lines)  AV=2.28  SV=5.23  BV=6.85
    {1204,1200,1024, 0, 0, 0},  //TV = 9.70(116 lines)  AV=2.28  SV=5.23  BV=6.74
    {1287,1200,1024, 0, 0, 0},  //TV = 9.60(124 lines)  AV=2.28  SV=5.23  BV=6.65
    {1380,1200,1024, 0, 0, 0},  //TV = 9.50(133 lines)  AV=2.28  SV=5.23  BV=6.55
    {1484,1200,1024, 0, 0, 0},  //TV = 9.40(143 lines)  AV=2.28  SV=5.23  BV=6.44
    {1577,1200,1024, 0, 0, 0},  //TV = 9.31(152 lines)  AV=2.28  SV=5.23  BV=6.35
    {1701,1200,1024, 0, 0, 0},  //TV = 9.20(164 lines)  AV=2.28  SV=5.23  BV=6.25
    {1815,1200,1024, 0, 0, 0},  //TV = 9.11(175 lines)  AV=2.28  SV=5.23  BV=6.15
    {1940,1200,1024, 0, 0, 0},  //TV = 9.01(187 lines)  AV=2.28  SV=5.23  BV=6.06
    {2075,1200,1024, 0, 0, 0},  //TV = 8.91(200 lines)  AV=2.28  SV=5.23  BV=5.96
    {2241,1200,1024, 0, 0, 0},  //TV = 8.80(216 lines)  AV=2.28  SV=5.23  BV=5.85
    {2407,1200,1024, 0, 0, 0},  //TV = 8.70(232 lines)  AV=2.28  SV=5.23  BV=5.74
    {2583,1200,1024, 0, 0, 0},  //TV = 8.60(249 lines)  AV=2.28  SV=5.23  BV=5.64
    {2759,1200,1024, 0, 0, 0},  //TV = 8.50(266 lines)  AV=2.28  SV=5.23  BV=5.55
    {2956,1200,1024, 0, 0, 0},  //TV = 8.40(285 lines)  AV=2.28  SV=5.23  BV=5.45
    {3184,1200,1024, 0, 0, 0},  //TV = 8.29(307 lines)  AV=2.28  SV=5.23  BV=5.34
    {3402,1200,1024, 0, 0, 0},  //TV = 8.20(328 lines)  AV=2.28  SV=5.23  BV=5.25
    {3641,1200,1024, 0, 0, 0},  //TV = 8.10(351 lines)  AV=2.28  SV=5.23  BV=5.15
    {3900,1200,1024, 0, 0, 0},  //TV = 8.00(376 lines)  AV=2.28  SV=5.23  BV=5.05
    {4180,1200,1024, 0, 0, 0},  //TV = 7.90(403 lines)  AV=2.28  SV=5.23  BV=4.95
    {4502,1200,1024, 0, 0, 0},  //TV = 7.80(434 lines)  AV=2.28  SV=5.23  BV=4.84
    {4823,1200,1024, 0, 0, 0},  //TV = 7.70(465 lines)  AV=2.28  SV=5.23  BV=4.74
    {5165,1200,1024, 0, 0, 0},  //TV = 7.60(498 lines)  AV=2.28  SV=5.23  BV=4.64
    {5528,1200,1024, 0, 0, 0},  //TV = 7.50(533 lines)  AV=2.28  SV=5.23  BV=4.55
    {5912,1200,1024, 0, 0, 0},  //TV = 7.40(570 lines)  AV=2.28  SV=5.23  BV=4.45
    {6337,1200,1024, 0, 0, 0},  //TV = 7.30(611 lines)  AV=2.28  SV=5.23  BV=4.35
    {6835,1200,1024, 0, 0, 0},  //TV = 7.19(659 lines)  AV=2.28  SV=5.23  BV=4.24
    {7271,1200,1024, 0, 0, 0},  //TV = 7.10(701 lines)  AV=2.28  SV=5.23  BV=4.15
    {7841,1200,1024, 0, 0, 0},  //TV = 6.99(756 lines)  AV=2.28  SV=5.23  BV=4.04
    {8328,1200,1024, 0, 0, 0},  //TV = 6.91(803 lines)  AV=2.28  SV=5.23  BV=3.95
    {8328,1296,1024, 0, 0, 0},  //TV = 6.91(803 lines)  AV=2.28  SV=5.34  BV=3.84
    {8328,1392,1024, 0, 0, 0},  //TV = 6.91(803 lines)  AV=2.28  SV=5.44  BV=3.74
    {8328,1488,1024, 0, 0, 0},  //TV = 6.91(803 lines)  AV=2.28  SV=5.54  BV=3.64
    {8328,1584,1024, 0, 0, 0},  //TV = 6.91(803 lines)  AV=2.28  SV=5.63  BV=3.55
    {8328,1696,1024, 0, 0, 0},  //TV = 6.91(803 lines)  AV=2.28  SV=5.73  BV=3.45
    {8328,1824,1024, 0, 0, 0},  //TV = 6.91(803 lines)  AV=2.28  SV=5.83  BV=3.35
    {8328,1952,1024, 0, 0, 0},  //TV = 6.91(803 lines)  AV=2.28  SV=5.93  BV=3.25
    {8328,2096,1024, 0, 0, 0},  //TV = 6.91(803 lines)  AV=2.28  SV=6.03  BV=3.15
    {8328,2240,1024, 0, 0, 0},  //TV = 6.91(803 lines)  AV=2.28  SV=6.13  BV=3.05
    {16667,1200,1024, 0, 0, 0},  //TV = 5.91(1607 lines)  AV=2.28  SV=5.23  BV=2.95
    {16667,1296,1024, 0, 0, 0},  //TV = 5.91(1607 lines)  AV=2.28  SV=5.34  BV=2.84
    {16667,1376,1024, 0, 0, 0},  //TV = 5.91(1607 lines)  AV=2.28  SV=5.43  BV=2.76
    {16667,1488,1024, 0, 0, 0},  //TV = 5.91(1607 lines)  AV=2.28  SV=5.54  BV=2.64
    {16667,1600,1024, 0, 0, 0},  //TV = 5.91(1607 lines)  AV=2.28  SV=5.64  BV=2.54
    {16667,1696,1024, 0, 0, 0},  //TV = 5.91(1607 lines)  AV=2.28  SV=5.73  BV=2.45
    {24995,1216,1024, 0, 0, 0},  //TV = 5.32(2410 lines)  AV=2.28  SV=5.25  BV=2.35
    {24995,1296,1024, 0, 0, 0},  //TV = 5.32(2410 lines)  AV=2.28  SV=5.34  BV=2.26
    {24995,1408,1024, 0, 0, 0},  //TV = 5.32(2410 lines)  AV=2.28  SV=5.46  BV=2.14
    {24995,1504,1024, 0, 0, 0},  //TV = 5.32(2410 lines)  AV=2.28  SV=5.55  BV=2.04
    {33334,1200,1024, 0, 0, 0},  //TV = 4.91(3214 lines)  AV=2.28  SV=5.23  BV=1.95
    {33334,1296,1024, 0, 0, 0},  //TV = 4.91(3214 lines)  AV=2.28  SV=5.34  BV=1.84
    {33334,1392,1024, 0, 0, 0},  //TV = 4.91(3214 lines)  AV=2.28  SV=5.44  BV=1.74
    {33334,1488,1024, 0, 0, 0},  //TV = 4.91(3214 lines)  AV=2.28  SV=5.54  BV=1.64
    {33334,1584,1024, 0, 0, 0},  //TV = 4.91(3214 lines)  AV=2.28  SV=5.63  BV=1.55
    {33334,1696,1024, 0, 0, 0},  //TV = 4.91(3214 lines)  AV=2.28  SV=5.73  BV=1.45
    {33334,1824,1024, 0, 0, 0},  //TV = 4.91(3214 lines)  AV=2.28  SV=5.83  BV=1.35
    {33334,1952,1024, 0, 0, 0},  //TV = 4.91(3214 lines)  AV=2.28  SV=5.93  BV=1.25
    {33334,2096,1024, 0, 0, 0},  //TV = 4.91(3214 lines)  AV=2.28  SV=6.03  BV=1.15
    {33334,2256,1024, 0, 0, 0},  //TV = 4.91(3214 lines)  AV=2.28  SV=6.14  BV=1.04
    {33334,2416,1024, 0, 0, 0},  //TV = 4.91(3214 lines)  AV=2.28  SV=6.24  BV=0.94
    {33334,2576,1024, 0, 0, 0},  //TV = 4.91(3214 lines)  AV=2.28  SV=6.33  BV=0.85
    {33334,2784,1024, 0, 0, 0},  //TV = 4.91(3214 lines)  AV=2.28  SV=6.44  BV=0.74
    {33334,2976,1024, 0, 0, 0},  //TV = 4.91(3214 lines)  AV=2.28  SV=6.54  BV=0.64
    {33334,3184,1024, 0, 0, 0},  //TV = 4.91(3214 lines)  AV=2.28  SV=6.64  BV=0.55
    {33334,3408,1024, 0, 0, 0},  //TV = 4.91(3214 lines)  AV=2.28  SV=6.73  BV=0.45
    {33334,3648,1024, 0, 0, 0},  //TV = 4.91(3214 lines)  AV=2.28  SV=6.83  BV=0.35
    {33334,3920,1024, 0, 0, 0},  //TV = 4.91(3214 lines)  AV=2.28  SV=6.94  BV=0.25
    {33334,4208,1024, 0, 0, 0},  //TV = 4.91(3214 lines)  AV=2.28  SV=7.04  BV=0.14
    {33334,4496,1024, 0, 0, 0},  //TV = 4.91(3214 lines)  AV=2.28  SV=7.13  BV=0.05
    {33334,4816,1024, 0, 0, 0},  //TV = 4.91(3214 lines)  AV=2.28  SV=7.23  BV=-0.05
    {33334,5152,1024, 0, 0, 0},  //TV = 4.91(3214 lines)  AV=2.28  SV=7.33  BV=-0.15
    {33334,5520,1024, 0, 0, 0},  //TV = 4.91(3214 lines)  AV=2.28  SV=7.43  BV=-0.25
    {33334,5952,1024, 0, 0, 0},  //TV = 4.91(3214 lines)  AV=2.28  SV=7.54  BV=-0.36
    {33334,6384,1024, 0, 0, 0},  //TV = 4.91(3214 lines)  AV=2.28  SV=7.64  BV=-0.46
    {33334,6832,1024, 0, 0, 0},  //TV = 4.91(3214 lines)  AV=2.28  SV=7.74  BV=-0.56
    {33334,7312,1024, 0, 0, 0},  //TV = 4.91(3214 lines)  AV=2.28  SV=7.84  BV=-0.65
    {33334,7840,1024, 0, 0, 0},  //TV = 4.91(3214 lines)  AV=2.28  SV=7.94  BV=-0.75
    {33334,8400,1024, 0, 0, 0},  //TV = 4.91(3214 lines)  AV=2.28  SV=8.04  BV=-0.85
    {33334,8992,1024, 0, 0, 0},  //TV = 4.91(3214 lines)  AV=2.28  SV=8.13  BV=-0.95
    {33334,9632,1024, 0, 0, 0},  //TV = 4.91(3214 lines)  AV=2.28  SV=8.23  BV=-1.05
    {33334,10320,1024, 0, 0, 0},  //TV = 4.91(3214 lines)  AV=2.28  SV=8.33  BV=-1.15
    {33334,11056,1024, 0, 0, 0},  //TV = 4.91(3214 lines)  AV=2.28  SV=8.43  BV=-1.25
    {33334,11840,1024, 0, 0, 0},  //TV = 4.91(3214 lines)  AV=2.28  SV=8.53  BV=-1.35
    {33333,12720,1024, 0, 0, 0},  //TV = 4.91(3214 lines)  AV=2.28  SV=8.63  BV=-1.45
    {33333,13632,1024, 0, 0, 0},  //TV = 4.91(3214 lines)  AV=2.28  SV=8.73  BV=-1.55
    {41661,11680,1024, 0, 0, 0},  //TV = 4.59(4017 lines)  AV=2.28  SV=8.51  BV=-1.65
    {41661,12528,1024, 0, 0, 0},  //TV = 4.59(4017 lines)  AV=2.28  SV=8.61  BV=-1.75
    {41661,13424,1024, 0, 0, 0},  //TV = 4.59(4017 lines)  AV=2.28  SV=8.71  BV=-1.85
    {49999,11984,1024, 0, 0, 0},  //TV = 4.32(4821 lines)  AV=2.28  SV=8.55  BV=-1.95
    {49999,12848,1024, 0, 0, 0},  //TV = 4.32(4821 lines)  AV=2.28  SV=8.65  BV=-2.05
    {58327,11792,1024, 0, 0, 0},  //TV = 4.10(5624 lines)  AV=2.28  SV=8.53  BV=-2.15
    {58327,12640,1024, 0, 0, 0},  //TV = 4.10(5624 lines)  AV=2.28  SV=8.63  BV=-2.25
    {66665,11856,1024, 0, 0, 0},  //TV = 3.91(6428 lines)  AV=2.28  SV=8.53  BV=-2.35
    {66665,12720,1024, 0, 0, 0},  //TV = 3.91(6428 lines)  AV=2.28  SV=8.63  BV=-2.45
    {66665,13616,1024, 0, 0, 0},  //TV = 3.91(6428 lines)  AV=2.28  SV=8.73  BV=-2.55
    {66665,14592,1024, 0, 0, 0},  //TV = 3.91(6428 lines)  AV=2.28  SV=8.83  BV=-2.65
    {66665,15632,1024, 0, 0, 0},  //TV = 3.91(6428 lines)  AV=2.28  SV=8.93  BV=-2.75
    {66665,15872,1080, 0, 0, 0},  //TV = 3.91(6428 lines)  AV=2.28  SV=9.03  BV=-2.85
    {66665,15872,1152, 0, 0, 0},  //TV = 3.91(6428 lines)  AV=2.28  SV=9.12  BV=-2.94
    {66665,15872,1248, 0, 0, 0},  //TV = 3.91(6428 lines)  AV=2.28  SV=9.24  BV=-3.06
    {66665,15872,1336, 0, 0, 0},  //TV = 3.91(6428 lines)  AV=2.28  SV=9.34  BV=-3.16
    {66665,15872,1432, 0, 0, 0},  //TV = 3.91(6428 lines)  AV=2.28  SV=9.44  BV=-3.26
    {66665,15872,1536, 0, 0, 0},  //TV = 3.91(6428 lines)  AV=2.28  SV=9.54  BV=-3.36
    {66665,15872,1648, 0, 0, 0},  //TV = 3.91(6428 lines)  AV=2.28  SV=9.64  BV=-3.46
    {66665,15872,1760, 0, 0, 0},  //TV = 3.91(6428 lines)  AV=2.28  SV=9.74  BV=-3.55
    {66665,15872,1888, 0, 0, 0},  //TV = 3.91(6428 lines)  AV=2.28  SV=9.84  BV=-3.65
    {66665,15872,2024, 0, 0, 0},  //TV = 3.91(6428 lines)  AV=2.28  SV=9.94  BV=-3.76
    {66665,15872,2168, 0, 0, 0},  //TV = 3.91(6428 lines)  AV=2.28  SV=10.04  BV=-3.85
    {66665,15872,2312, 0, 0, 0},  //TV = 3.91(6428 lines)  AV=2.28  SV=10.13  BV=-3.95
    {74993,15872,2208, 0, 0, 0},  //TV = 3.74(7231 lines)  AV=2.28  SV=10.06  BV=-4.05
    {74993,15872,2360, 0, 0, 0},  //TV = 3.74(7231 lines)  AV=2.28  SV=10.16  BV=-4.15
    {83331,15872,2280, 0, 0, 0},  //TV = 3.59(8035 lines)  AV=2.28  SV=10.11  BV=-4.25
    {91659,15872,2224, 0, 0, 0},  //TV = 3.45(8838 lines)  AV=2.28  SV=10.07  BV=-4.35
    {99998,15872,2184, 0, 0, 0},  //TV = 3.32(9642 lines)  AV=2.28  SV=10.05  BV=-4.45
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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
    {94,1280,1024, 0, 0, 0},  //TV = 13.38(9 lines)  AV=2.28  SV=5.32  BV=10.33
    {104,1216,1024, 0, 0, 0},  //TV = 13.23(10 lines)  AV=2.28  SV=5.25  BV=10.26
    {115,1200,1024, 0, 0, 0},  //TV = 13.09(11 lines)  AV=2.28  SV=5.23  BV=10.13
    {115,1280,1024, 0, 0, 0},  //TV = 13.09(11 lines)  AV=2.28  SV=5.32  BV=10.04
    {125,1248,1024, 0, 0, 0},  //TV = 12.97(12 lines)  AV=2.28  SV=5.29  BV=9.96
    {135,1264,1024, 0, 0, 0},  //TV = 12.85(13 lines)  AV=2.28  SV=5.30  BV=9.83
    {146,1264,1024, 0, 0, 0},  //TV = 12.74(14 lines)  AV=2.28  SV=5.30  BV=9.71
    {166,1200,1024, 0, 0, 0},  //TV = 12.56(16 lines)  AV=2.28  SV=5.23  BV=9.60
    {177,1216,1024, 0, 0, 0},  //TV = 12.46(17 lines)  AV=2.28  SV=5.25  BV=9.49
    {177,1264,1024, 0, 0, 0},  /* TV = 12.46(17 lines)  AV=2.28  SV=5.30  BV=9.44 */
    {198,1216,1024, 0, 0, 0},  //TV = 12.30(19 lines)  AV=2.28  SV=5.25  BV=9.33
    {208,1216,1024, 0, 0, 0},  //TV = 12.23(20 lines)  AV=2.28  SV=5.25  BV=9.26
    {218,1248,1024, 0, 0, 0},  //TV = 12.16(21 lines)  AV=2.28  SV=5.29  BV=9.15
    {239,1216,1024, 0, 0, 0},  //TV = 12.03(23 lines)  AV=2.28  SV=5.25  BV=9.06
    {260,1200,1024, 0, 0, 0},  //TV = 11.91(25 lines)  AV=2.28  SV=5.23  BV=8.96
    {270,1248,1024, 0, 0, 0},  //TV = 11.85(26 lines)  AV=2.28  SV=5.29  BV=8.84
    {291,1232,1024, 0, 0, 0},  //TV = 11.75(28 lines)  AV=2.28  SV=5.27  BV=8.75
    {322,1200,1024, 0, 0, 0},  //TV = 11.60(31 lines)  AV=2.28  SV=5.23  BV=8.65
    {343,1200,1024, 0, 0, 0},  //TV = 11.51(33 lines)  AV=2.28  SV=5.23  BV=8.56
    {363,1216,1024, 0, 0, 0},  //TV = 11.43(35 lines)  AV=2.28  SV=5.25  BV=8.45
    {395,1200,1024, 0, 0, 0},  //TV = 11.31(38 lines)  AV=2.28  SV=5.23  BV=8.35
    {426,1200,1024, 0, 0, 0},  //TV = 11.20(41 lines)  AV=2.28  SV=5.23  BV=8.24
    {446,1216,1024, 0, 0, 0},  //TV = 11.13(43 lines)  AV=2.28  SV=5.25  BV=8.16
    {478,1216,1024, 0, 0, 0},  //TV = 11.03(46 lines)  AV=2.28  SV=5.25  BV=8.06
    {519,1200,1024, 0, 0, 0},  //TV = 10.91(50 lines)  AV=2.28  SV=5.23  BV=7.96
    {561,1200,1024, 0, 0, 0},  //TV = 10.80(54 lines)  AV=2.28  SV=5.23  BV=7.85
    {592,1216,1024, 0, 0, 0},  //TV = 10.72(57 lines)  AV=2.28  SV=5.25  BV=7.75
    {644,1200,1024, 0, 0, 0},  //TV = 10.60(62 lines)  AV=2.28  SV=5.23  BV=7.65
    {685,1200,1024, 0, 0, 0},  //TV = 10.51(66 lines)  AV=2.28  SV=5.23  BV=7.56
    {737,1200,1024, 0, 0, 0},  //TV = 10.41(71 lines)  AV=2.28  SV=5.23  BV=7.45
    {789,1200,1024, 0, 0, 0},  //TV = 10.31(76 lines)  AV=2.28  SV=5.23  BV=7.35
    {841,1216,1024, 0, 0, 0},  //TV = 10.22(81 lines)  AV=2.28  SV=5.25  BV=7.24
    {903,1200,1024, 0, 0, 0},  //TV = 10.11(87 lines)  AV=2.28  SV=5.23  BV=7.16
    {975,1200,1024, 0, 0, 0},  //TV = 10.00(94 lines)  AV=2.28  SV=5.23  BV=7.05
    {1038,1200,1024, 0, 0, 0},  //TV = 9.91(100 lines)  AV=2.28  SV=5.23  BV=6.96
    {1121,1200,1024, 0, 0, 0},  //TV = 9.80(108 lines)  AV=2.28  SV=5.23  BV=6.85
    {1204,1200,1024, 0, 0, 0},  //TV = 9.70(116 lines)  AV=2.28  SV=5.23  BV=6.74
    {1287,1200,1024, 0, 0, 0},  //TV = 9.60(124 lines)  AV=2.28  SV=5.23  BV=6.65
    {1380,1200,1024, 0, 0, 0},  //TV = 9.50(133 lines)  AV=2.28  SV=5.23  BV=6.55
    {1484,1200,1024, 0, 0, 0},  //TV = 9.40(143 lines)  AV=2.28  SV=5.23  BV=6.44
    {1577,1200,1024, 0, 0, 0},  //TV = 9.31(152 lines)  AV=2.28  SV=5.23  BV=6.35
    {1701,1200,1024, 0, 0, 0},  //TV = 9.20(164 lines)  AV=2.28  SV=5.23  BV=6.25
    {1815,1200,1024, 0, 0, 0},  //TV = 9.11(175 lines)  AV=2.28  SV=5.23  BV=6.15
    {1940,1200,1024, 0, 0, 0},  //TV = 9.01(187 lines)  AV=2.28  SV=5.23  BV=6.06
    {2075,1200,1024, 0, 0, 0},  //TV = 8.91(200 lines)  AV=2.28  SV=5.23  BV=5.96
    {2241,1200,1024, 0, 0, 0},  //TV = 8.80(216 lines)  AV=2.28  SV=5.23  BV=5.85
    {2407,1200,1024, 0, 0, 0},  //TV = 8.70(232 lines)  AV=2.28  SV=5.23  BV=5.74
    {2583,1200,1024, 0, 0, 0},  //TV = 8.60(249 lines)  AV=2.28  SV=5.23  BV=5.64
    {2759,1200,1024, 0, 0, 0},  //TV = 8.50(266 lines)  AV=2.28  SV=5.23  BV=5.55
    {2956,1200,1024, 0, 0, 0},  //TV = 8.40(285 lines)  AV=2.28  SV=5.23  BV=5.45
    {3184,1200,1024, 0, 0, 0},  //TV = 8.29(307 lines)  AV=2.28  SV=5.23  BV=5.34
    {3402,1200,1024, 0, 0, 0},  //TV = 8.20(328 lines)  AV=2.28  SV=5.23  BV=5.25
    {3641,1200,1024, 0, 0, 0},  //TV = 8.10(351 lines)  AV=2.28  SV=5.23  BV=5.15
    {3900,1200,1024, 0, 0, 0},  //TV = 8.00(376 lines)  AV=2.28  SV=5.23  BV=5.05
    {4180,1200,1024, 0, 0, 0},  //TV = 7.90(403 lines)  AV=2.28  SV=5.23  BV=4.95
    {4502,1200,1024, 0, 0, 0},  //TV = 7.80(434 lines)  AV=2.28  SV=5.23  BV=4.84
    {4823,1200,1024, 0, 0, 0},  //TV = 7.70(465 lines)  AV=2.28  SV=5.23  BV=4.74
    {5165,1200,1024, 0, 0, 0},  //TV = 7.60(498 lines)  AV=2.28  SV=5.23  BV=4.64
    {5528,1200,1024, 0, 0, 0},  //TV = 7.50(533 lines)  AV=2.28  SV=5.23  BV=4.55
    {5912,1200,1024, 0, 0, 0},  //TV = 7.40(570 lines)  AV=2.28  SV=5.23  BV=4.45
    {6337,1200,1024, 0, 0, 0},  //TV = 7.30(611 lines)  AV=2.28  SV=5.23  BV=4.35
    {6835,1200,1024, 0, 0, 0},  //TV = 7.19(659 lines)  AV=2.28  SV=5.23  BV=4.24
    {7271,1200,1024, 0, 0, 0},  //TV = 7.10(701 lines)  AV=2.28  SV=5.23  BV=4.15
    {7841,1200,1024, 0, 0, 0},  //TV = 6.99(756 lines)  AV=2.28  SV=5.23  BV=4.04
    {8401,1200,1024, 0, 0, 0},  //TV = 6.90(810 lines)  AV=2.28  SV=5.23  BV=3.94
    {8992,1200,1024, 0, 0, 0},  //TV = 6.80(867 lines)  AV=2.28  SV=5.23  BV=3.84
    {9635,1200,1024, 0, 0, 0},  //TV = 6.70(929 lines)  AV=2.28  SV=5.23  BV=3.74
    {9998,1232,1024, 0, 0, 0},  //TV = 6.64(964 lines)  AV=2.28  SV=5.27  BV=3.65
    {9998,1328,1024, 0, 0, 0},  //TV = 6.64(964 lines)  AV=2.28  SV=5.38  BV=3.54
    {9998,1424,1024, 0, 0, 0},  //TV = 6.64(964 lines)  AV=2.28  SV=5.48  BV=3.44
    {9998,1520,1024, 0, 0, 0},  //TV = 6.64(964 lines)  AV=2.28  SV=5.57  BV=3.35
    {9998,1632,1024, 0, 0, 0},  //TV = 6.64(964 lines)  AV=2.28  SV=5.67  BV=3.25
    {9998,1744,1024, 0, 0, 0},  //TV = 6.64(964 lines)  AV=2.28  SV=5.77  BV=3.15
    {9998,1872,1024, 0, 0, 0},  //TV = 6.64(964 lines)  AV=2.28  SV=5.87  BV=3.05
    {9998,2000,1024, 0, 0, 0},  //TV = 6.64(964 lines)  AV=2.28  SV=5.97  BV=2.95
    {9998,2144,1024, 0, 0, 0},  //TV = 6.64(964 lines)  AV=2.28  SV=6.07  BV=2.85
    {9998,2304,1024, 0, 0, 0},  //TV = 6.64(964 lines)  AV=2.28  SV=6.17  BV=2.75
    {19996,1232,1024, 0, 0, 0},  //TV = 5.64(1928 lines)  AV=2.28  SV=5.27  BV=2.65
    {19996,1328,1024, 0, 0, 0},  //TV = 5.64(1928 lines)  AV=2.28  SV=5.38  BV=2.54
    {19996,1424,1024, 0, 0, 0},  //TV = 5.64(1928 lines)  AV=2.28  SV=5.48  BV=2.44
    {19996,1520,1024, 0, 0, 0},  //TV = 5.64(1928 lines)  AV=2.28  SV=5.57  BV=2.35
    {19996,1632,1024, 0, 0, 0},  //TV = 5.64(1928 lines)  AV=2.28  SV=5.67  BV=2.25
    {19996,1760,1024, 0, 0, 0},  //TV = 5.64(1928 lines)  AV=2.28  SV=5.78  BV=2.14
    {30004,1248,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=5.29  BV=2.05
    {30004,1344,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=5.39  BV=1.94
    {30004,1424,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=5.48  BV=1.86
    {30004,1536,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=5.58  BV=1.75
    {30004,1648,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=5.69  BV=1.65
    {30004,1760,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=5.78  BV=1.55
    {30004,1888,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=5.88  BV=1.45
    {30004,2032,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=5.99  BV=1.35
    {30004,2176,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=6.09  BV=1.25
    {30004,2320,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=6.18  BV=1.15
    {30004,2496,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=6.29  BV=1.05
    {30004,2672,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=6.38  BV=0.95
    {30004,2864,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=6.48  BV=0.85
    {30004,3072,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=6.58  BV=0.75
    {30004,3296,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=6.69  BV=0.65
    {30004,3536,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=6.79  BV=0.55
    {30004,3776,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=6.88  BV=0.45
    {30004,4080,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=6.99  BV=0.34
    {30004,4368,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=7.09  BV=0.24
    {30004,4672,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=7.19  BV=0.14
    {30004,4992,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=7.29  BV=0.05
    {30004,5360,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=7.39  BV=-0.05
    {30004,5728,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=7.48  BV=-0.15
    {30004,6144,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=7.58  BV=-0.25
    {30004,6624,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=7.69  BV=-0.36
    {30004,7088,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=7.79  BV=-0.46
    {30004,7584,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=7.89  BV=-0.56
    {30004,8128,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=7.99  BV=-0.65
    {30004,8704,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=8.09  BV=-0.75
    {30004,9328,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=8.19  BV=-0.85
    {30004,9984,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=8.29  BV=-0.95
    {30004,10704,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=8.39  BV=-1.05
    {30004,11472,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=8.49  BV=-1.15
    {30004,12288,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=8.58  BV=-1.25
    {30004,13152,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=8.68  BV=-1.35
    {30004,14128,1024, 0, 0, 0},  //TV = 5.06(2893 lines)  AV=2.28  SV=8.79  BV=-1.45
    {40001,11360,1024, 0, 0, 0},  //TV = 4.64(3857 lines)  AV=2.28  SV=8.47  BV=-1.55
    {40001,12176,1024, 0, 0, 0},  //TV = 4.64(3857 lines)  AV=2.28  SV=8.57  BV=-1.65
    {40001,13040,1024, 0, 0, 0},  //TV = 4.64(3857 lines)  AV=2.28  SV=8.67  BV=-1.75
    {49999,11184,1024, 0, 0, 0},  //TV = 4.32(4821 lines)  AV=2.28  SV=8.45  BV=-1.85
    {49999,11984,1024, 0, 0, 0},  //TV = 4.32(4821 lines)  AV=2.28  SV=8.55  BV=-1.95
    {49999,12848,1024, 0, 0, 0},  //TV = 4.32(4821 lines)  AV=2.28  SV=8.65  BV=-2.05
    {59997,11472,1024, 0, 0, 0},  //TV = 4.06(5785 lines)  AV=2.28  SV=8.49  BV=-2.15
    {59997,12288,1024, 0, 0, 0},  //TV = 4.06(5785 lines)  AV=2.28  SV=8.58  BV=-2.25
    {59997,13168,1024, 0, 0, 0},  //TV = 4.06(5785 lines)  AV=2.28  SV=8.68  BV=-2.35
    {70005,12112,1024, 0, 0, 0},  //TV = 3.84(6750 lines)  AV=2.28  SV=8.56  BV=-2.45
    {70005,12976,1024, 0, 0, 0},  //TV = 3.84(6750 lines)  AV=2.28  SV=8.66  BV=-2.55
    {70005,13904,1024, 0, 0, 0},  //TV = 3.84(6750 lines)  AV=2.28  SV=8.76  BV=-2.65
    {70005,14896,1024, 0, 0, 0},  //TV = 3.84(6750 lines)  AV=2.28  SV=8.86  BV=-2.75
    {70005,15872,1024, 0, 0, 0},  //TV = 3.84(6750 lines)  AV=2.28  SV=8.95  BV=-2.84
    {70005,15872,1096, 0, 0, 0},  //TV = 3.84(6750 lines)  AV=2.28  SV=9.05  BV=-2.94
    {70005,15872,1176, 0, 0, 0},  //TV = 3.84(6750 lines)  AV=2.28  SV=9.15  BV=-3.04
    {70005,15872,1272, 0, 0, 0},  //TV = 3.84(6750 lines)  AV=2.28  SV=9.27  BV=-3.16
    {70005,15872,1360, 0, 0, 0},  //TV = 3.84(6750 lines)  AV=2.28  SV=9.36  BV=-3.25
    {70005,15872,1464, 0, 0, 0},  //TV = 3.84(6750 lines)  AV=2.28  SV=9.47  BV=-3.36
    {70005,15872,1568, 0, 0, 0},  //TV = 3.84(6750 lines)  AV=2.28  SV=9.57  BV=-3.46
    {70005,15872,1680, 0, 0, 0},  //TV = 3.84(6750 lines)  AV=2.28  SV=9.67  BV=-3.56
    {70005,15872,1800, 0, 0, 0},  //TV = 3.84(6750 lines)  AV=2.28  SV=9.77  BV=-3.66
    {70005,15872,1928, 0, 0, 0},  //TV = 3.84(6750 lines)  AV=2.28  SV=9.87  BV=-3.76
    {70005,15872,2064, 0, 0, 0},  //TV = 3.84(6750 lines)  AV=2.28  SV=9.97  BV=-3.85
    {70005,15872,2208, 0, 0, 0},  //TV = 3.84(6750 lines)  AV=2.28  SV=10.06  BV=-3.95
    {70005,15872,2360, 0, 0, 0},  //TV = 3.84(6750 lines)  AV=2.28  SV=10.16  BV=-4.05
    {80002,15872,2216, 0, 0, 0},  //TV = 3.64(7714 lines)  AV=2.28  SV=10.07  BV=-4.15
    {80002,15872,2376, 0, 0, 0},  //TV = 3.64(7714 lines)  AV=2.28  SV=10.17  BV=-4.25
    {90000,15872,2264, 0, 0, 0},  //TV = 3.47(8678 lines)  AV=2.28  SV=10.10  BV=-4.35
    {99998,15872,2184, 0, 0, 0},  //TV = 3.32(9642 lines)  AV=2.28  SV=10.05  BV=-4.45
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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
    149, //u4TotalIndex
    20, //i4StrobeTrigerBV
    104, //i4MaxBV
    -45, //i4MinBV
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
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {525,1200,1024, 0, 0, 0},  //TV = 10.90(50 lines)  AV=2.28  SV=5.23  BV=7.94
    {557,1200,1024, 0, 0, 0},  //TV = 10.81(53 lines)  AV=2.28  SV=5.23  BV=7.86
    {599,1200,1024, 0, 0, 0},  //TV = 10.71(57 lines)  AV=2.28  SV=5.23  BV=7.75
    {641,1200,1024, 0, 0, 0},  //TV = 10.61(61 lines)  AV=2.28  SV=5.23  BV=7.65
    {683,1216,1024, 0, 0, 0},  //TV = 10.52(65 lines)  AV=2.28  SV=5.25  BV=7.54
    {735,1200,1024, 0, 0, 0},  //TV = 10.41(70 lines)  AV=2.28  SV=5.23  BV=7.46
    {788,1200,1024, 0, 0, 0},  //TV = 10.31(75 lines)  AV=2.28  SV=5.23  BV=7.36
    {840,1216,1024, 0, 0, 0},  //TV = 10.22(80 lines)  AV=2.28  SV=5.25  BV=7.24
    {903,1200,1024, 0, 0, 0},  //TV = 10.11(86 lines)  AV=2.28  SV=5.23  BV=7.16
    {977,1200,1024, 0, 0, 0},  //TV = 10.00(93 lines)  AV=2.28  SV=5.23  BV=7.05
    {1040,1200,1024, 0, 0, 0},  //TV = 9.91(99 lines)  AV=2.28  SV=5.23  BV=6.96
    {1113,1200,1024, 0, 0, 0},  //TV = 9.81(106 lines)  AV=2.28  SV=5.23  BV=6.86
    {1197,1200,1024, 0, 0, 0},  //TV = 9.71(114 lines)  AV=2.28  SV=5.23  BV=6.75
    {1292,1200,1024, 0, 0, 0},  //TV = 9.60(123 lines)  AV=2.28  SV=5.23  BV=6.64
    {1376,1200,1024, 0, 0, 0},  //TV = 9.51(131 lines)  AV=2.28  SV=5.23  BV=6.55
    {1481,1200,1024, 0, 0, 0},  //TV = 9.40(141 lines)  AV=2.28  SV=5.23  BV=6.45
    {1575,1200,1024, 0, 0, 0},  //TV = 9.31(150 lines)  AV=2.28  SV=5.23  BV=6.36
    {1701,1200,1024, 0, 0, 0},  //TV = 9.20(162 lines)  AV=2.28  SV=5.23  BV=6.25
    {1827,1200,1024, 0, 0, 0},  //TV = 9.10(174 lines)  AV=2.28  SV=5.23  BV=6.14
    {1953,1200,1024, 0, 0, 0},  //TV = 9.00(186 lines)  AV=2.28  SV=5.23  BV=6.05
    {2090,1200,1024, 0, 0, 0},  //TV = 8.90(199 lines)  AV=2.28  SV=5.23  BV=5.95
    {2258,1200,1024, 0, 0, 0},  //TV = 8.79(215 lines)  AV=2.28  SV=5.23  BV=5.84
    {2415,1200,1024, 0, 0, 0},  //TV = 8.69(230 lines)  AV=2.28  SV=5.23  BV=5.74
    {2583,1200,1024, 0, 0, 0},  //TV = 8.60(246 lines)  AV=2.28  SV=5.23  BV=5.64
    {2762,1200,1024, 0, 0, 0},  //TV = 8.50(263 lines)  AV=2.28  SV=5.23  BV=5.55
    {2972,1200,1024, 0, 0, 0},  //TV = 8.39(283 lines)  AV=2.28  SV=5.23  BV=5.44
    {3182,1200,1024, 0, 0, 0},  //TV = 8.30(303 lines)  AV=2.28  SV=5.23  BV=5.34
    {3402,1200,1024, 0, 0, 0},  //TV = 8.20(324 lines)  AV=2.28  SV=5.23  BV=5.25
    {3644,1200,1024, 0, 0, 0},  //TV = 8.10(347 lines)  AV=2.28  SV=5.23  BV=5.15
    {3896,1200,1024, 0, 0, 0},  //TV = 8.00(371 lines)  AV=2.28  SV=5.23  BV=5.05
    {4179,1200,1024, 0, 0, 0},  //TV = 7.90(398 lines)  AV=2.28  SV=5.23  BV=4.95
    {4505,1200,1024, 0, 0, 0},  //TV = 7.79(429 lines)  AV=2.28  SV=5.23  BV=4.84
    {4820,1200,1024, 0, 0, 0},  //TV = 7.70(459 lines)  AV=2.28  SV=5.23  BV=4.74
    {5156,1200,1024, 0, 0, 0},  //TV = 7.60(491 lines)  AV=2.28  SV=5.23  BV=4.65
    {5523,1200,1024, 0, 0, 0},  //TV = 7.50(526 lines)  AV=2.28  SV=5.23  BV=4.55
    {5912,1200,1024, 0, 0, 0},  //TV = 7.40(563 lines)  AV=2.28  SV=5.23  BV=4.45
    {6332,1200,1024, 0, 0, 0},  //TV = 7.30(603 lines)  AV=2.28  SV=5.23  BV=4.35
    {6836,1200,1024, 0, 0, 0},  //TV = 7.19(651 lines)  AV=2.28  SV=5.23  BV=4.24
    {7319,1200,1024, 0, 0, 0},  //TV = 7.09(697 lines)  AV=2.28  SV=5.23  BV=4.14
    {7833,1200,1024, 0, 0, 0},  //TV = 7.00(746 lines)  AV=2.28  SV=5.23  BV=4.04
    {8338,1200,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.23  BV=3.95
    {8338,1296,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.34  BV=3.84
    {8338,1376,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.43  BV=3.75
    {8338,1488,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.54  BV=3.64
    {8338,1584,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.63  BV=3.55
    {8338,1696,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.73  BV=3.45
    {8338,1824,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.83  BV=3.35
    {8338,1952,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.93  BV=3.25
    {8338,2096,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.03  BV=3.15
    {8338,2240,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.13  BV=3.05
    {16664,1200,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.23  BV=2.95
    {16664,1296,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.34  BV=2.84
    {16664,1376,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.43  BV=2.76
    {16664,1488,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.54  BV=2.64
    {16664,1600,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.64  BV=2.54
    {16664,1696,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.73  BV=2.45
    {25001,1216,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.25  BV=2.35
    {25001,1296,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.34  BV=2.26
    {25001,1408,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.46  BV=2.14
    {25001,1488,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.54  BV=2.06
    {33327,1200,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.23  BV=1.95
    {33327,1296,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.34  BV=1.84
    {33327,1376,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.43  BV=1.76
    {33327,1488,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.54  BV=1.64
    {41664,1264,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=5.30  BV=1.56
    {41664,1360,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=5.41  BV=1.45
    {50002,1216,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=5.25  BV=1.35
    {50002,1296,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=5.34  BV=1.26
    {58328,1200,1024, 0, 0, 0},  //TV = 4.10(5555 lines)  AV=2.28  SV=5.23  BV=1.15
    {58328,1280,1024, 0, 0, 0},  //TV = 4.10(5555 lines)  AV=2.28  SV=5.32  BV=1.05
    {66665,1200,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=5.23  BV=0.95
    {66665,1296,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=5.34  BV=0.84
    {75002,1232,1024, 0, 0, 0},  //TV = 3.74(7143 lines)  AV=2.28  SV=5.27  BV=0.75
    {75002,1312,1024, 0, 0, 0},  //TV = 3.74(7143 lines)  AV=2.28  SV=5.36  BV=0.65
    {83328,1264,1024, 0, 0, 0},  //TV = 3.59(7936 lines)  AV=2.28  SV=5.30  BV=0.56
    {91666,1232,1024, 0, 0, 0},  //TV = 3.45(8730 lines)  AV=2.28  SV=5.27  BV=0.46
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {525,1200,1024, 0, 0, 0},  //TV = 10.90(50 lines)  AV=2.28  SV=5.23  BV=7.94
    {557,1200,1024, 0, 0, 0},  //TV = 10.81(53 lines)  AV=2.28  SV=5.23  BV=7.86
    {599,1200,1024, 0, 0, 0},  //TV = 10.71(57 lines)  AV=2.28  SV=5.23  BV=7.75
    {641,1200,1024, 0, 0, 0},  //TV = 10.61(61 lines)  AV=2.28  SV=5.23  BV=7.65
    {683,1216,1024, 0, 0, 0},  //TV = 10.52(65 lines)  AV=2.28  SV=5.25  BV=7.54
    {735,1200,1024, 0, 0, 0},  //TV = 10.41(70 lines)  AV=2.28  SV=5.23  BV=7.46
    {788,1200,1024, 0, 0, 0},  //TV = 10.31(75 lines)  AV=2.28  SV=5.23  BV=7.36
    {840,1216,1024, 0, 0, 0},  //TV = 10.22(80 lines)  AV=2.28  SV=5.25  BV=7.24
    {903,1200,1024, 0, 0, 0},  //TV = 10.11(86 lines)  AV=2.28  SV=5.23  BV=7.16
    {977,1200,1024, 0, 0, 0},  //TV = 10.00(93 lines)  AV=2.28  SV=5.23  BV=7.05
    {1040,1200,1024, 0, 0, 0},  //TV = 9.91(99 lines)  AV=2.28  SV=5.23  BV=6.96
    {1113,1200,1024, 0, 0, 0},  //TV = 9.81(106 lines)  AV=2.28  SV=5.23  BV=6.86
    {1197,1200,1024, 0, 0, 0},  //TV = 9.71(114 lines)  AV=2.28  SV=5.23  BV=6.75
    {1292,1200,1024, 0, 0, 0},  //TV = 9.60(123 lines)  AV=2.28  SV=5.23  BV=6.64
    {1376,1200,1024, 0, 0, 0},  //TV = 9.51(131 lines)  AV=2.28  SV=5.23  BV=6.55
    {1481,1200,1024, 0, 0, 0},  //TV = 9.40(141 lines)  AV=2.28  SV=5.23  BV=6.45
    {1575,1200,1024, 0, 0, 0},  //TV = 9.31(150 lines)  AV=2.28  SV=5.23  BV=6.36
    {1701,1200,1024, 0, 0, 0},  //TV = 9.20(162 lines)  AV=2.28  SV=5.23  BV=6.25
    {1827,1200,1024, 0, 0, 0},  //TV = 9.10(174 lines)  AV=2.28  SV=5.23  BV=6.14
    {1953,1200,1024, 0, 0, 0},  //TV = 9.00(186 lines)  AV=2.28  SV=5.23  BV=6.05
    {2090,1200,1024, 0, 0, 0},  //TV = 8.90(199 lines)  AV=2.28  SV=5.23  BV=5.95
    {2258,1200,1024, 0, 0, 0},  //TV = 8.79(215 lines)  AV=2.28  SV=5.23  BV=5.84
    {2415,1200,1024, 0, 0, 0},  //TV = 8.69(230 lines)  AV=2.28  SV=5.23  BV=5.74
    {2583,1200,1024, 0, 0, 0},  //TV = 8.60(246 lines)  AV=2.28  SV=5.23  BV=5.64
    {2762,1200,1024, 0, 0, 0},  //TV = 8.50(263 lines)  AV=2.28  SV=5.23  BV=5.55
    {2972,1200,1024, 0, 0, 0},  //TV = 8.39(283 lines)  AV=2.28  SV=5.23  BV=5.44
    {3182,1200,1024, 0, 0, 0},  //TV = 8.30(303 lines)  AV=2.28  SV=5.23  BV=5.34
    {3402,1200,1024, 0, 0, 0},  //TV = 8.20(324 lines)  AV=2.28  SV=5.23  BV=5.25
    {3644,1200,1024, 0, 0, 0},  //TV = 8.10(347 lines)  AV=2.28  SV=5.23  BV=5.15
    {3896,1200,1024, 0, 0, 0},  //TV = 8.00(371 lines)  AV=2.28  SV=5.23  BV=5.05
    {4179,1200,1024, 0, 0, 0},  //TV = 7.90(398 lines)  AV=2.28  SV=5.23  BV=4.95
    {4505,1200,1024, 0, 0, 0},  //TV = 7.79(429 lines)  AV=2.28  SV=5.23  BV=4.84
    {4820,1200,1024, 0, 0, 0},  //TV = 7.70(459 lines)  AV=2.28  SV=5.23  BV=4.74
    {5156,1200,1024, 0, 0, 0},  //TV = 7.60(491 lines)  AV=2.28  SV=5.23  BV=4.65
    {5523,1200,1024, 0, 0, 0},  //TV = 7.50(526 lines)  AV=2.28  SV=5.23  BV=4.55
    {5912,1200,1024, 0, 0, 0},  //TV = 7.40(563 lines)  AV=2.28  SV=5.23  BV=4.45
    {6332,1200,1024, 0, 0, 0},  //TV = 7.30(603 lines)  AV=2.28  SV=5.23  BV=4.35
    {6836,1200,1024, 0, 0, 0},  //TV = 7.19(651 lines)  AV=2.28  SV=5.23  BV=4.24
    {7319,1200,1024, 0, 0, 0},  //TV = 7.09(697 lines)  AV=2.28  SV=5.23  BV=4.14
    {7833,1200,1024, 0, 0, 0},  //TV = 7.00(746 lines)  AV=2.28  SV=5.23  BV=4.04
    {8400,1200,1024, 0, 0, 0},  //TV = 6.90(800 lines)  AV=2.28  SV=5.23  BV=3.94
    {8999,1200,1024, 0, 0, 0},  //TV = 6.80(857 lines)  AV=2.28  SV=5.23  BV=3.84
    {9639,1200,1024, 0, 0, 0},  //TV = 6.70(918 lines)  AV=2.28  SV=5.23  BV=3.74
    {9996,1232,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.27  BV=3.65
    {9996,1328,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.38  BV=3.54
    {9996,1424,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.48  BV=3.44
    {9996,1520,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.57  BV=3.35
    {9996,1632,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.67  BV=3.25
    {9996,1744,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.77  BV=3.15
    {9996,1872,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.87  BV=3.05
    {9996,2000,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.97  BV=2.95
    {9996,2144,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.07  BV=2.85
    {9996,2304,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.17  BV=2.75
    {20003,1232,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.27  BV=2.65
    {20003,1328,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.38  BV=2.54
    {20003,1424,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.48  BV=2.44
    {20003,1520,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.57  BV=2.35
    {20003,1632,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.67  BV=2.25
    {20003,1760,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.78  BV=2.14
    {29999,1248,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.29  BV=2.05
    {29999,1344,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.39  BV=1.94
    {29999,1440,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.49  BV=1.84
    {29999,1536,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.58  BV=1.75
    {39995,1232,1024, 0, 0, 0},  //TV = 4.64(3809 lines)  AV=2.28  SV=5.27  BV=1.65
    {39995,1328,1024, 0, 0, 0},  //TV = 4.64(3809 lines)  AV=2.28  SV=5.38  BV=1.54
    {39995,1424,1024, 0, 0, 0},  //TV = 4.64(3809 lines)  AV=2.28  SV=5.48  BV=1.44
    {50002,1216,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=5.25  BV=1.35
    {50002,1296,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=5.34  BV=1.26
    {50002,1392,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=5.44  BV=1.15
    {59997,1248,1024, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=5.29  BV=1.05
    {59997,1344,1024, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=5.39  BV=0.94
    {70004,1232,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=5.27  BV=0.84
    {70004,1312,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=5.36  BV=0.75
    {80000,1232,1024, 0, 0, 0},  //TV = 3.64(7619 lines)  AV=2.28  SV=5.27  BV=0.65
    {80000,1328,1024, 0, 0, 0},  //TV = 3.64(7619 lines)  AV=2.28  SV=5.38  BV=0.54
    {89996,1264,1024, 0, 0, 0},  //TV = 3.47(8571 lines)  AV=2.28  SV=5.30  BV=0.45
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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
    100, //u4TotalIndex
    20, //i4StrobeTrigerBV
    104, //i4MaxBV
    4, //i4MinBV
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
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {494,1264,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.30  BV=7.95
    {494,1360,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.41  BV=7.85
    {494,1456,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.51  BV=7.75
    {494,1568,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.61  BV=7.64
    {494,1680,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.71  BV=7.54
    {494,1808,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.82  BV=7.44
    {494,1936,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.92  BV=7.34
    {494,2064,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.01  BV=7.25
    {536,2032,1024, 0, 0, 0},  //TV = 10.87(51 lines)  AV=2.28  SV=5.99  BV=7.15
    {567,2064,1024, 0, 0, 0},  //TV = 10.78(54 lines)  AV=2.28  SV=6.01  BV=7.05
    {609,2064,1024, 0, 0, 0},  //TV = 10.68(58 lines)  AV=2.28  SV=6.01  BV=6.95
    {662,2032,1024, 0, 0, 0},  //TV = 10.56(63 lines)  AV=2.28  SV=5.99  BV=6.85
    {704,2048,1024, 0, 0, 0},  //TV = 10.47(67 lines)  AV=2.28  SV=6.00  BV=6.75
    {756,2032,1024, 0, 0, 0},  //TV = 10.37(72 lines)  AV=2.28  SV=5.99  BV=6.66
    {809,2048,1024, 0, 0, 0},  //TV = 10.27(77 lines)  AV=2.28  SV=6.00  BV=6.55
    {872,2032,1024, 0, 0, 0},  //TV = 10.16(83 lines)  AV=2.28  SV=5.99  BV=6.45
    {924,2048,1024, 0, 0, 0},  //TV = 10.08(88 lines)  AV=2.28  SV=6.00  BV=6.35
    {998,2032,1024, 0, 0, 0},  //TV = 9.97(95 lines)  AV=2.28  SV=5.99  BV=6.25
    {1071,2032,1024, 0, 0, 0},  //TV = 9.87(102 lines)  AV=2.28  SV=5.99  BV=6.15
    {1145,2048,1024, 0, 0, 0},  //TV = 9.77(109 lines)  AV=2.28  SV=6.00  BV=6.05
    {1229,2048,1024, 0, 0, 0},  //TV = 9.67(117 lines)  AV=2.28  SV=6.00  BV=5.94
    {1313,2048,1024, 0, 0, 0},  //TV = 9.57(125 lines)  AV=2.28  SV=6.00  BV=5.85
    {1407,2048,1024, 0, 0, 0},  //TV = 9.47(134 lines)  AV=2.28  SV=6.00  BV=5.75
    {1512,2032,1024, 0, 0, 0},  //TV = 9.37(144 lines)  AV=2.28  SV=5.99  BV=5.66
    {1617,2048,1024, 0, 0, 0},  //TV = 9.27(154 lines)  AV=2.28  SV=6.00  BV=5.55
    {1733,2048,1024, 0, 0, 0},  //TV = 9.17(165 lines)  AV=2.28  SV=6.00  BV=5.45
    {1848,2048,1024, 0, 0, 0},  //TV = 9.08(176 lines)  AV=2.28  SV=6.00  BV=5.35
    {1995,2048,1024, 0, 0, 0},  //TV = 8.97(190 lines)  AV=2.28  SV=6.00  BV=5.24
    {2132,2048,1024, 0, 0, 0},  //TV = 8.87(203 lines)  AV=2.28  SV=6.00  BV=5.15
    {2289,2048,1024, 0, 0, 0},  //TV = 8.77(218 lines)  AV=2.28  SV=6.00  BV=5.05
    {2457,2048,1024, 0, 0, 0},  //TV = 8.67(234 lines)  AV=2.28  SV=6.00  BV=4.94
    {2625,2048,1024, 0, 0, 0},  //TV = 8.57(250 lines)  AV=2.28  SV=6.00  BV=4.85
    {2814,2048,1024, 0, 0, 0},  //TV = 8.47(268 lines)  AV=2.28  SV=6.00  BV=4.75
    {3024,2048,1024, 0, 0, 0},  //TV = 8.37(288 lines)  AV=2.28  SV=6.00  BV=4.64
    {3234,2048,1024, 0, 0, 0},  //TV = 8.27(308 lines)  AV=2.28  SV=6.00  BV=4.55
    {3465,2048,1024, 0, 0, 0},  //TV = 8.17(330 lines)  AV=2.28  SV=6.00  BV=4.45
    {3738,2048,1024, 0, 0, 0},  //TV = 8.06(356 lines)  AV=2.28  SV=6.00  BV=4.34
    {4001,2048,1024, 0, 0, 0},  //TV = 7.97(381 lines)  AV=2.28  SV=6.00  BV=4.24
    {4274,2048,1024, 0, 0, 0},  //TV = 7.87(407 lines)  AV=2.28  SV=6.00  BV=4.15
    {4578,2048,1024, 0, 0, 0},  //TV = 7.77(436 lines)  AV=2.28  SV=6.00  BV=4.05
    {4904,2048,1024, 0, 0, 0},  //TV = 7.67(467 lines)  AV=2.28  SV=6.00  BV=3.95
    {5250,2048,1024, 0, 0, 0},  //TV = 7.57(500 lines)  AV=2.28  SV=6.00  BV=3.85
    {5660,2048,1024, 0, 0, 0},  //TV = 7.46(539 lines)  AV=2.28  SV=6.00  BV=3.74
    {6059,2048,1024, 0, 0, 0},  //TV = 7.37(577 lines)  AV=2.28  SV=6.00  BV=3.64
    {6489,2048,1024, 0, 0, 0},  //TV = 7.27(618 lines)  AV=2.28  SV=6.00  BV=3.54
    {6951,2048,1024, 0, 0, 0},  //TV = 7.17(662 lines)  AV=2.28  SV=6.00  BV=3.44
    {7445,2048,1024, 0, 0, 0},  //TV = 7.07(709 lines)  AV=2.28  SV=6.00  BV=3.34
    {7980,2048,1024, 0, 0, 0},  //TV = 6.97(760 lines)  AV=2.28  SV=6.00  BV=3.24
    {8337,2096,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.03  BV=3.15
    {8337,2240,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.13  BV=3.05
    {8337,2400,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.23  BV=2.95
    {8337,2576,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.33  BV=2.85
    {8337,2768,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.43  BV=2.75
    {8337,2960,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.53  BV=2.65
    {16664,1600,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.64  BV=2.54
    {16664,1696,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.73  BV=2.45
    {16664,1824,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.83  BV=2.35
    {16664,1968,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.94  BV=2.24
    {16664,2096,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.03  BV=2.15
    {16664,2256,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.14  BV=2.04
    {16664,2416,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.24  BV=1.94
    {25001,1728,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.75  BV=1.84
    {25001,1840,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.85  BV=1.75
    {25001,1984,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.95  BV=1.64
    {25001,2128,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=6.06  BV=1.54
    {25001,2272,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=6.15  BV=1.45
    {33327,1824,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.83  BV=1.35
    {33327,1952,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.93  BV=1.25
    {33327,2096,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.03  BV=1.15
    {33327,2256,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.14  BV=1.04
    {41664,1920,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=5.91  BV=0.95
    {41664,2064,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=6.01  BV=0.85
    {41664,2208,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=6.11  BV=0.75
    {50001,1968,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=5.94  BV=0.65
    {50001,2112,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=6.04  BV=0.55
    {58328,1952,1024, 0, 0, 0},  //TV = 4.10(5555 lines)  AV=2.28  SV=5.93  BV=0.44
    {58328,2080,1024, 0, 0, 0},  //TV = 4.10(5555 lines)  AV=2.28  SV=6.02  BV=0.35
    {66665,1952,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=5.93  BV=0.25
    {66665,2096,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=6.03  BV=0.15
    {75002,2000,1024, 0, 0, 0},  //TV = 3.74(7143 lines)  AV=2.28  SV=5.97  BV=0.05
    {75002,2144,1024, 0, 0, 0},  //TV = 3.74(7143 lines)  AV=2.28  SV=6.07  BV=-0.05
    {83328,2064,1024, 0, 0, 0},  //TV = 3.59(7936 lines)  AV=2.28  SV=6.01  BV=-0.15
    {91665,2016,1024, 0, 0, 0},  //TV = 3.45(8730 lines)  AV=2.28  SV=5.98  BV=-0.25
    {99992,1968,1024, 0, 0, 0},  //TV = 3.32(9523 lines)  AV=2.28  SV=5.94  BV=-0.35
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {494,1264,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.30  BV=7.95
    {494,1360,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.41  BV=7.85
    {494,1456,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.51  BV=7.75
    {494,1568,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.61  BV=7.64
    {494,1680,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.71  BV=7.54
    {494,1808,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.82  BV=7.44
    {494,1936,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.92  BV=7.34
    {494,2064,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.01  BV=7.25
    {536,2032,1024, 0, 0, 0},  //TV = 10.87(51 lines)  AV=2.28  SV=5.99  BV=7.15
    {567,2064,1024, 0, 0, 0},  //TV = 10.78(54 lines)  AV=2.28  SV=6.01  BV=7.05
    {609,2064,1024, 0, 0, 0},  //TV = 10.68(58 lines)  AV=2.28  SV=6.01  BV=6.95
    {662,2032,1024, 0, 0, 0},  //TV = 10.56(63 lines)  AV=2.28  SV=5.99  BV=6.85
    {704,2048,1024, 0, 0, 0},  //TV = 10.47(67 lines)  AV=2.28  SV=6.00  BV=6.75
    {756,2032,1024, 0, 0, 0},  //TV = 10.37(72 lines)  AV=2.28  SV=5.99  BV=6.66
    {809,2048,1024, 0, 0, 0},  //TV = 10.27(77 lines)  AV=2.28  SV=6.00  BV=6.55
    {872,2032,1024, 0, 0, 0},  //TV = 10.16(83 lines)  AV=2.28  SV=5.99  BV=6.45
    {924,2048,1024, 0, 0, 0},  //TV = 10.08(88 lines)  AV=2.28  SV=6.00  BV=6.35
    {998,2032,1024, 0, 0, 0},  //TV = 9.97(95 lines)  AV=2.28  SV=5.99  BV=6.25
    {1071,2032,1024, 0, 0, 0},  //TV = 9.87(102 lines)  AV=2.28  SV=5.99  BV=6.15
    {1145,2048,1024, 0, 0, 0},  //TV = 9.77(109 lines)  AV=2.28  SV=6.00  BV=6.05
    {1229,2048,1024, 0, 0, 0},  //TV = 9.67(117 lines)  AV=2.28  SV=6.00  BV=5.94
    {1313,2048,1024, 0, 0, 0},  //TV = 9.57(125 lines)  AV=2.28  SV=6.00  BV=5.85
    {1407,2048,1024, 0, 0, 0},  //TV = 9.47(134 lines)  AV=2.28  SV=6.00  BV=5.75
    {1512,2032,1024, 0, 0, 0},  //TV = 9.37(144 lines)  AV=2.28  SV=5.99  BV=5.66
    {1617,2048,1024, 0, 0, 0},  //TV = 9.27(154 lines)  AV=2.28  SV=6.00  BV=5.55
    {1733,2048,1024, 0, 0, 0},  //TV = 9.17(165 lines)  AV=2.28  SV=6.00  BV=5.45
    {1848,2048,1024, 0, 0, 0},  //TV = 9.08(176 lines)  AV=2.28  SV=6.00  BV=5.35
    {1995,2048,1024, 0, 0, 0},  //TV = 8.97(190 lines)  AV=2.28  SV=6.00  BV=5.24
    {2132,2048,1024, 0, 0, 0},  //TV = 8.87(203 lines)  AV=2.28  SV=6.00  BV=5.15
    {2289,2048,1024, 0, 0, 0},  //TV = 8.77(218 lines)  AV=2.28  SV=6.00  BV=5.05
    {2457,2048,1024, 0, 0, 0},  //TV = 8.67(234 lines)  AV=2.28  SV=6.00  BV=4.94
    {2625,2048,1024, 0, 0, 0},  //TV = 8.57(250 lines)  AV=2.28  SV=6.00  BV=4.85
    {2814,2048,1024, 0, 0, 0},  //TV = 8.47(268 lines)  AV=2.28  SV=6.00  BV=4.75
    {3024,2048,1024, 0, 0, 0},  //TV = 8.37(288 lines)  AV=2.28  SV=6.00  BV=4.64
    {3234,2048,1024, 0, 0, 0},  //TV = 8.27(308 lines)  AV=2.28  SV=6.00  BV=4.55
    {3465,2048,1024, 0, 0, 0},  //TV = 8.17(330 lines)  AV=2.28  SV=6.00  BV=4.45
    {3738,2048,1024, 0, 0, 0},  //TV = 8.06(356 lines)  AV=2.28  SV=6.00  BV=4.34
    {4001,2048,1024, 0, 0, 0},  //TV = 7.97(381 lines)  AV=2.28  SV=6.00  BV=4.24
    {4274,2048,1024, 0, 0, 0},  //TV = 7.87(407 lines)  AV=2.28  SV=6.00  BV=4.15
    {4578,2048,1024, 0, 0, 0},  //TV = 7.77(436 lines)  AV=2.28  SV=6.00  BV=4.05
    {4904,2048,1024, 0, 0, 0},  //TV = 7.67(467 lines)  AV=2.28  SV=6.00  BV=3.95
    {5250,2048,1024, 0, 0, 0},  //TV = 7.57(500 lines)  AV=2.28  SV=6.00  BV=3.85
    {5660,2048,1024, 0, 0, 0},  //TV = 7.46(539 lines)  AV=2.28  SV=6.00  BV=3.74
    {6059,2048,1024, 0, 0, 0},  //TV = 7.37(577 lines)  AV=2.28  SV=6.00  BV=3.64
    {6489,2048,1024, 0, 0, 0},  //TV = 7.27(618 lines)  AV=2.28  SV=6.00  BV=3.54
    {6951,2048,1024, 0, 0, 0},  //TV = 7.17(662 lines)  AV=2.28  SV=6.00  BV=3.44
    {7445,2048,1024, 0, 0, 0},  //TV = 7.07(709 lines)  AV=2.28  SV=6.00  BV=3.34
    {7980,2048,1024, 0, 0, 0},  //TV = 6.97(760 lines)  AV=2.28  SV=6.00  BV=3.24
    {8547,2048,1024, 0, 0, 0},  //TV = 6.87(814 lines)  AV=2.28  SV=6.00  BV=3.15
    {9156,2048,1024, 0, 0, 0},  //TV = 6.77(872 lines)  AV=2.28  SV=6.00  BV=3.05
    {9807,2048,1024, 0, 0, 0},  //TV = 6.67(934 lines)  AV=2.28  SV=6.00  BV=2.95
    {9996,2144,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.07  BV=2.85
    {9996,2304,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.17  BV=2.75
    {9996,2464,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.27  BV=2.65
    {9996,2640,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.37  BV=2.55
    {9996,2848,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.48  BV=2.44
    {9996,3040,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.57  BV=2.35
    {20003,1632,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.67  BV=2.25
    {20003,1744,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.77  BV=2.15
    {20003,1872,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.87  BV=2.05
    {20003,2016,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.98  BV=1.94
    {20003,2160,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.08  BV=1.84
    {20003,2304,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.17  BV=1.75
    {20003,2480,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.28  BV=1.64
    {29999,1760,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.78  BV=1.55
    {29999,1888,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.88  BV=1.45
    {29999,2032,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.99  BV=1.35
    {29999,2176,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.09  BV=1.25
    {29999,2336,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.19  BV=1.14
    {40005,1872,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=5.87  BV=1.05
    {40005,2000,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=5.97  BV=0.95
    {40005,2144,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=6.07  BV=0.85
    {50001,1840,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=5.85  BV=0.75
    {50001,1968,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=5.94  BV=0.65
    {50001,2112,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=6.04  BV=0.55
    {59997,1888,1024, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=5.88  BV=0.45
    {59997,2032,1024, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=5.99  BV=0.35
    {59997,2176,1024, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=6.09  BV=0.25
    {70004,2000,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=5.97  BV=0.15
    {70004,2144,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=6.07  BV=0.05
    {80000,2000,1024, 0, 0, 0},  //TV = 3.64(7619 lines)  AV=2.28  SV=5.97  BV=-0.05
    {80000,2144,1024, 0, 0, 0},  //TV = 3.64(7619 lines)  AV=2.28  SV=6.07  BV=-0.15
    {89996,2048,1024, 0, 0, 0},  //TV = 3.47(8571 lines)  AV=2.28  SV=6.00  BV=-0.25
    {100002,1968,1024, 0, 0, 0},  //TV = 3.32(9524 lines)  AV=2.28  SV=5.94  BV=-0.35
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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
    108, //u4TotalIndex
    20, //i4StrobeTrigerBV
    104, //i4MaxBV
    -4, //i4MinBV
    90, //i4EffectiveMaxBV
    -30, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_200, //ISO
    sCaptureISO200PLineTable_60Hz,
    sCaptureISO200PLineTable_50Hz,
    NULL,
};
static strEvPline sCaptureISO400PLineTable_60Hz =
{
{
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {494,1264,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.30  BV=7.95
    {494,1360,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.41  BV=7.85
    {494,1456,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.51  BV=7.75
    {494,1568,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.61  BV=7.64
    {494,1680,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.71  BV=7.54
    {494,1808,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.82  BV=7.44
    {494,1936,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.92  BV=7.34
    {494,2064,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.01  BV=7.25
    {494,2208,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.11  BV=7.15
    {494,2368,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.21  BV=7.05
    {494,2544,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.31  BV=6.95
    {494,2720,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.41  BV=6.85
    {494,2928,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.52  BV=6.74
    {494,3136,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.61  BV=6.64
    {494,3344,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.71  BV=6.55
    {494,3584,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.81  BV=6.45
    {494,3840,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.91  BV=6.35
    {494,4144,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.02  BV=6.24
    {536,4064,1024, 0, 0, 0},  //TV = 10.87(51 lines)  AV=2.28  SV=6.99  BV=6.15
    {567,4128,1024, 0, 0, 0},  //TV = 10.78(54 lines)  AV=2.28  SV=7.01  BV=6.05
    {609,4128,1024, 0, 0, 0},  //TV = 10.68(58 lines)  AV=2.28  SV=7.01  BV=5.95
    {662,4064,1024, 0, 0, 0},  //TV = 10.56(63 lines)  AV=2.28  SV=6.99  BV=5.85
    {704,4080,1024, 0, 0, 0},  //TV = 10.47(67 lines)  AV=2.28  SV=6.99  BV=5.75
    {756,4096,1024, 0, 0, 0},  //TV = 10.37(72 lines)  AV=2.28  SV=7.00  BV=5.64
    {809,4080,1024, 0, 0, 0},  //TV = 10.27(77 lines)  AV=2.28  SV=6.99  BV=5.55
    {872,4064,1024, 0, 0, 0},  //TV = 10.16(83 lines)  AV=2.28  SV=6.99  BV=5.45
    {924,4112,1024, 0, 0, 0},  //TV = 10.08(88 lines)  AV=2.28  SV=7.01  BV=5.35
    {998,4096,1024, 0, 0, 0},  //TV = 9.97(95 lines)  AV=2.28  SV=7.00  BV=5.24
    {1071,4080,1024, 0, 0, 0},  //TV = 9.87(102 lines)  AV=2.28  SV=6.99  BV=5.15
    {1145,4096,1024, 0, 0, 0},  //TV = 9.77(109 lines)  AV=2.28  SV=7.00  BV=5.05
    {1229,4096,1024, 0, 0, 0},  //TV = 9.67(117 lines)  AV=2.28  SV=7.00  BV=4.94
    {1313,4096,1024, 0, 0, 0},  //TV = 9.57(125 lines)  AV=2.28  SV=7.00  BV=4.85
    {1407,4112,1024, 0, 0, 0},  //TV = 9.47(134 lines)  AV=2.28  SV=7.01  BV=4.74
    {1512,4080,1024, 0, 0, 0},  //TV = 9.37(144 lines)  AV=2.28  SV=6.99  BV=4.65
    {1617,4112,1024, 0, 0, 0},  //TV = 9.27(154 lines)  AV=2.28  SV=7.01  BV=4.54
    {1733,4096,1024, 0, 0, 0},  //TV = 9.17(165 lines)  AV=2.28  SV=7.00  BV=4.45
    {1869,4080,1024, 0, 0, 0},  //TV = 9.06(178 lines)  AV=2.28  SV=6.99  BV=4.34
    {1995,4080,1024, 0, 0, 0},  //TV = 8.97(190 lines)  AV=2.28  SV=6.99  BV=4.25
    {2132,4096,1024, 0, 0, 0},  //TV = 8.87(203 lines)  AV=2.28  SV=7.00  BV=4.15
    {2289,4096,1024, 0, 0, 0},  //TV = 8.77(218 lines)  AV=2.28  SV=7.00  BV=4.05
    {2457,4080,1024, 0, 0, 0},  //TV = 8.67(234 lines)  AV=2.28  SV=6.99  BV=3.95
    {2625,4096,1024, 0, 0, 0},  //TV = 8.57(250 lines)  AV=2.28  SV=7.00  BV=3.85
    {2825,4096,1024, 0, 0, 0},  //TV = 8.47(269 lines)  AV=2.28  SV=7.00  BV=3.74
    {3024,4096,1024, 0, 0, 0},  //TV = 8.37(288 lines)  AV=2.28  SV=7.00  BV=3.64
    {3234,4096,1024, 0, 0, 0},  //TV = 8.27(308 lines)  AV=2.28  SV=7.00  BV=3.55
    {3465,4096,1024, 0, 0, 0},  //TV = 8.17(330 lines)  AV=2.28  SV=7.00  BV=3.45
    {3738,4080,1024, 0, 0, 0},  //TV = 8.06(356 lines)  AV=2.28  SV=6.99  BV=3.34
    {4001,4080,1024, 0, 0, 0},  //TV = 7.97(381 lines)  AV=2.28  SV=6.99  BV=3.25
    {4274,4096,1024, 0, 0, 0},  //TV = 7.87(407 lines)  AV=2.28  SV=7.00  BV=3.15
    {4578,4096,1024, 0, 0, 0},  //TV = 7.77(436 lines)  AV=2.28  SV=7.00  BV=3.05
    {4904,4096,1024, 0, 0, 0},  //TV = 7.67(467 lines)  AV=2.28  SV=7.00  BV=2.95
    {5250,4096,1024, 0, 0, 0},  //TV = 7.57(500 lines)  AV=2.28  SV=7.00  BV=2.85
    {5660,4096,1024, 0, 0, 0},  //TV = 7.46(539 lines)  AV=2.28  SV=7.00  BV=2.74
    {6059,4096,1024, 0, 0, 0},  //TV = 7.37(577 lines)  AV=2.28  SV=7.00  BV=2.64
    {6489,4096,1024, 0, 0, 0},  //TV = 7.27(618 lines)  AV=2.28  SV=7.00  BV=2.54
    {6951,4096,1024, 0, 0, 0},  //TV = 7.17(662 lines)  AV=2.28  SV=7.00  BV=2.44
    {7445,4096,1024, 0, 0, 0},  //TV = 7.07(709 lines)  AV=2.28  SV=7.00  BV=2.34
    {7980,4096,1024, 0, 0, 0},  //TV = 6.97(760 lines)  AV=2.28  SV=7.00  BV=2.24
    {8337,4192,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=7.03  BV=2.15
    {8337,4496,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=7.13  BV=2.05
    {8337,4816,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=7.23  BV=1.95
    {8337,5152,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=7.33  BV=1.85
    {8337,5520,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=7.43  BV=1.75
    {8337,5920,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=7.53  BV=1.65
    {16664,3168,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.63  BV=1.55
    {16664,3424,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.74  BV=1.44
    {16664,3648,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.83  BV=1.35
    {16664,3936,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.94  BV=1.24
    {16664,4208,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=7.04  BV=1.14
    {16664,4512,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=7.14  BV=1.04
    {16664,4832,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=7.24  BV=0.94
    {25001,3456,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=6.75  BV=0.84
    {25001,3696,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=6.85  BV=0.75
    {25001,3968,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=6.95  BV=0.64
    {25001,4240,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=7.05  BV=0.55
    {25001,4544,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=7.15  BV=0.45
    {33327,3664,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.84  BV=0.34
    {33327,3920,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.94  BV=0.25
    {33327,4192,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.03  BV=0.15
    {33327,4496,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.13  BV=0.05
    {41664,3856,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=6.91  BV=-0.05
    {41664,4128,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=7.01  BV=-0.15
    {41664,4432,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=7.11  BV=-0.25
    {50001,3952,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=6.95  BV=-0.35
    {50001,4240,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=7.05  BV=-0.45
    {58328,3888,1024, 0, 0, 0},  //TV = 4.10(5555 lines)  AV=2.28  SV=6.92  BV=-0.55
    {58328,4176,1024, 0, 0, 0},  //TV = 4.10(5555 lines)  AV=2.28  SV=7.03  BV=-0.65
    {66665,3920,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=6.94  BV=-0.75
    {66665,4192,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=7.03  BV=-0.85
    {75002,4000,1024, 0, 0, 0},  //TV = 3.74(7143 lines)  AV=2.28  SV=6.97  BV=-0.95
    {75002,4288,1024, 0, 0, 0},  //TV = 3.74(7143 lines)  AV=2.28  SV=7.07  BV=-1.05
    {83328,4128,1024, 0, 0, 0},  //TV = 3.59(7936 lines)  AV=2.28  SV=7.01  BV=-1.15
    {91665,4016,1024, 0, 0, 0},  //TV = 3.45(8730 lines)  AV=2.28  SV=6.97  BV=-1.25
    {99992,3952,1024, 0, 0, 0},  //TV = 3.32(9523 lines)  AV=2.28  SV=6.95  BV=-1.35
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {494,1264,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.30  BV=7.95
    {494,1360,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.41  BV=7.85
    {494,1456,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.51  BV=7.75
    {494,1568,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.61  BV=7.64
    {494,1680,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.71  BV=7.54
    {494,1808,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.82  BV=7.44
    {494,1936,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.92  BV=7.34
    {494,2064,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.01  BV=7.25
    {494,2208,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.11  BV=7.15
    {494,2368,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.21  BV=7.05
    {494,2544,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.31  BV=6.95
    {494,2720,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.41  BV=6.85
    {494,2928,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.52  BV=6.74
    {494,3136,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.61  BV=6.64
    {494,3344,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.71  BV=6.55
    {494,3584,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.81  BV=6.45
    {494,3840,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.91  BV=6.35
    {494,4144,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.02  BV=6.24
    {536,4064,1024, 0, 0, 0},  //TV = 10.87(51 lines)  AV=2.28  SV=6.99  BV=6.15
    {567,4128,1024, 0, 0, 0},  //TV = 10.78(54 lines)  AV=2.28  SV=7.01  BV=6.05
    {609,4128,1024, 0, 0, 0},  //TV = 10.68(58 lines)  AV=2.28  SV=7.01  BV=5.95
    {662,4064,1024, 0, 0, 0},  //TV = 10.56(63 lines)  AV=2.28  SV=6.99  BV=5.85
    {704,4080,1024, 0, 0, 0},  //TV = 10.47(67 lines)  AV=2.28  SV=6.99  BV=5.75
    {756,4096,1024, 0, 0, 0},  //TV = 10.37(72 lines)  AV=2.28  SV=7.00  BV=5.64
    {809,4080,1024, 0, 0, 0},  //TV = 10.27(77 lines)  AV=2.28  SV=6.99  BV=5.55
    {872,4064,1024, 0, 0, 0},  //TV = 10.16(83 lines)  AV=2.28  SV=6.99  BV=5.45
    {924,4112,1024, 0, 0, 0},  //TV = 10.08(88 lines)  AV=2.28  SV=7.01  BV=5.35
    {998,4096,1024, 0, 0, 0},  //TV = 9.97(95 lines)  AV=2.28  SV=7.00  BV=5.24
    {1071,4080,1024, 0, 0, 0},  //TV = 9.87(102 lines)  AV=2.28  SV=6.99  BV=5.15
    {1145,4096,1024, 0, 0, 0},  //TV = 9.77(109 lines)  AV=2.28  SV=7.00  BV=5.05
    {1229,4096,1024, 0, 0, 0},  //TV = 9.67(117 lines)  AV=2.28  SV=7.00  BV=4.94
    {1313,4096,1024, 0, 0, 0},  //TV = 9.57(125 lines)  AV=2.28  SV=7.00  BV=4.85
    {1407,4112,1024, 0, 0, 0},  //TV = 9.47(134 lines)  AV=2.28  SV=7.01  BV=4.74
    {1512,4080,1024, 0, 0, 0},  //TV = 9.37(144 lines)  AV=2.28  SV=6.99  BV=4.65
    {1617,4112,1024, 0, 0, 0},  //TV = 9.27(154 lines)  AV=2.28  SV=7.01  BV=4.54
    {1733,4096,1024, 0, 0, 0},  //TV = 9.17(165 lines)  AV=2.28  SV=7.00  BV=4.45
    {1869,4080,1024, 0, 0, 0},  //TV = 9.06(178 lines)  AV=2.28  SV=6.99  BV=4.34
    {1995,4080,1024, 0, 0, 0},  //TV = 8.97(190 lines)  AV=2.28  SV=6.99  BV=4.25
    {2132,4096,1024, 0, 0, 0},  //TV = 8.87(203 lines)  AV=2.28  SV=7.00  BV=4.15
    {2289,4096,1024, 0, 0, 0},  //TV = 8.77(218 lines)  AV=2.28  SV=7.00  BV=4.05
    {2457,4080,1024, 0, 0, 0},  //TV = 8.67(234 lines)  AV=2.28  SV=6.99  BV=3.95
    {2625,4096,1024, 0, 0, 0},  //TV = 8.57(250 lines)  AV=2.28  SV=7.00  BV=3.85
    {2825,4096,1024, 0, 0, 0},  //TV = 8.47(269 lines)  AV=2.28  SV=7.00  BV=3.74
    {3024,4096,1024, 0, 0, 0},  //TV = 8.37(288 lines)  AV=2.28  SV=7.00  BV=3.64
    {3234,4096,1024, 0, 0, 0},  //TV = 8.27(308 lines)  AV=2.28  SV=7.00  BV=3.55
    {3465,4096,1024, 0, 0, 0},  //TV = 8.17(330 lines)  AV=2.28  SV=7.00  BV=3.45
    {3738,4080,1024, 0, 0, 0},  //TV = 8.06(356 lines)  AV=2.28  SV=6.99  BV=3.34
    {4001,4080,1024, 0, 0, 0},  //TV = 7.97(381 lines)  AV=2.28  SV=6.99  BV=3.25
    {4274,4096,1024, 0, 0, 0},  //TV = 7.87(407 lines)  AV=2.28  SV=7.00  BV=3.15
    {4578,4096,1024, 0, 0, 0},  //TV = 7.77(436 lines)  AV=2.28  SV=7.00  BV=3.05
    {4904,4096,1024, 0, 0, 0},  //TV = 7.67(467 lines)  AV=2.28  SV=7.00  BV=2.95
    {5250,4096,1024, 0, 0, 0},  //TV = 7.57(500 lines)  AV=2.28  SV=7.00  BV=2.85
    {5660,4096,1024, 0, 0, 0},  //TV = 7.46(539 lines)  AV=2.28  SV=7.00  BV=2.74
    {6059,4096,1024, 0, 0, 0},  //TV = 7.37(577 lines)  AV=2.28  SV=7.00  BV=2.64
    {6489,4096,1024, 0, 0, 0},  //TV = 7.27(618 lines)  AV=2.28  SV=7.00  BV=2.54
    {6951,4096,1024, 0, 0, 0},  //TV = 7.17(662 lines)  AV=2.28  SV=7.00  BV=2.44
    {7445,4096,1024, 0, 0, 0},  //TV = 7.07(709 lines)  AV=2.28  SV=7.00  BV=2.34
    {7980,4096,1024, 0, 0, 0},  //TV = 6.97(760 lines)  AV=2.28  SV=7.00  BV=2.24
    {8547,4096,1024, 0, 0, 0},  //TV = 6.87(814 lines)  AV=2.28  SV=7.00  BV=2.15
    {9156,4096,1024, 0, 0, 0},  //TV = 6.77(872 lines)  AV=2.28  SV=7.00  BV=2.05
    {9807,4096,1024, 0, 0, 0},  //TV = 6.67(934 lines)  AV=2.28  SV=7.00  BV=1.95
    {9996,4304,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=7.07  BV=1.85
    {9996,4608,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=7.17  BV=1.75
    {9996,4944,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=7.27  BV=1.65
    {9996,5296,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=7.37  BV=1.55
    {9996,5664,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=7.47  BV=1.45
    {9996,6112,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=7.58  BV=1.34
    {20003,3248,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.67  BV=1.25
    {20003,3504,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.77  BV=1.14
    {20003,3760,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.88  BV=1.04
    {20003,4032,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.98  BV=0.94
    {20003,4320,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=7.08  BV=0.84
    {20003,4624,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=7.17  BV=0.74
    {20003,4960,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=7.28  BV=0.64
    {29999,3536,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.79  BV=0.55
    {29999,3792,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.89  BV=0.45
    {29999,4064,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.99  BV=0.35
    {29999,4352,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.09  BV=0.25
    {29999,4672,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.19  BV=0.14
    {40005,3744,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=6.87  BV=0.05
    {40005,4016,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=6.97  BV=-0.05
    {40005,4304,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=7.07  BV=-0.15
    {50001,3696,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=6.85  BV=-0.25
    {50001,3952,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=6.95  BV=-0.35
    {50001,4240,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=7.05  BV=-0.45
    {59997,3792,1024, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=6.89  BV=-0.55
    {59997,4064,1024, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=6.99  BV=-0.65
    {59997,4352,1024, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=7.09  BV=-0.75
    {70004,4000,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=6.97  BV=-0.85
    {70004,4288,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=7.07  BV=-0.95
    {80000,4016,1024, 0, 0, 0},  //TV = 3.64(7619 lines)  AV=2.28  SV=6.97  BV=-1.05
    {80000,4304,1024, 0, 0, 0},  //TV = 3.64(7619 lines)  AV=2.28  SV=7.07  BV=-1.15
    {89996,4096,1024, 0, 0, 0},  //TV = 3.47(8571 lines)  AV=2.28  SV=7.00  BV=-1.25
    {100002,3952,1024, 0, 0, 0},  //TV = 3.32(9524 lines)  AV=2.28  SV=6.95  BV=-1.35
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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
    118, //u4TotalIndex
    20, //i4StrobeTrigerBV
    104, //i4MaxBV
    -14, //i4MinBV
    90, //i4EffectiveMaxBV
    -30, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_400, //ISO
    sCaptureISO400PLineTable_60Hz,
    sCaptureISO400PLineTable_50Hz,
    NULL,
};
static strEvPline sCaptureISO800PLineTable_60Hz =
{
{
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {494,1264,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.30  BV=7.95
    {494,1360,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.41  BV=7.85
    {494,1456,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.51  BV=7.75
    {494,1568,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.61  BV=7.64
    {494,1680,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.71  BV=7.54
    {494,1808,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.82  BV=7.44
    {494,1936,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.92  BV=7.34
    {494,2064,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.01  BV=7.25
    {494,2208,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.11  BV=7.15
    {494,2368,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.21  BV=7.05
    {494,2544,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.31  BV=6.95
    {494,2720,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.41  BV=6.85
    {494,2928,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.52  BV=6.74
    {494,3136,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.61  BV=6.64
    {494,3344,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.71  BV=6.55
    {494,3584,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.81  BV=6.45
    {494,3840,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.91  BV=6.35
    {494,4144,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.02  BV=6.24
    {494,4432,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.11  BV=6.14
    {494,4752,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.21  BV=6.04
    {494,5088,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.31  BV=5.95
    {494,5440,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.41  BV=5.85
    {494,5872,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.52  BV=5.74
    {494,6288,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.62  BV=5.64
    {494,6720,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.71  BV=5.54
    {494,7200,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.81  BV=5.44
    {494,7712,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.91  BV=5.35
    {494,8272,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.01  BV=5.24
    {536,8144,1024, 0, 0, 0},  //TV = 10.87(51 lines)  AV=2.28  SV=7.99  BV=5.15
    {567,8256,1024, 0, 0, 0},  //TV = 10.78(54 lines)  AV=2.28  SV=8.01  BV=5.05
    {609,8256,1024, 0, 0, 0},  //TV = 10.68(58 lines)  AV=2.28  SV=8.01  BV=4.95
    {662,8144,1024, 0, 0, 0},  //TV = 10.56(63 lines)  AV=2.28  SV=7.99  BV=4.84
    {704,8208,1024, 0, 0, 0},  //TV = 10.47(67 lines)  AV=2.28  SV=8.00  BV=4.74
    {756,8192,1024, 0, 0, 0},  //TV = 10.37(72 lines)  AV=2.28  SV=8.00  BV=4.64
    {809,8224,1024, 0, 0, 0},  //TV = 10.27(77 lines)  AV=2.28  SV=8.01  BV=4.54
    {872,8144,1024, 0, 0, 0},  //TV = 10.16(83 lines)  AV=2.28  SV=7.99  BV=4.45
    {935,8176,1024, 0, 0, 0},  //TV = 10.06(89 lines)  AV=2.28  SV=8.00  BV=4.34
    {998,8192,1024, 0, 0, 0},  //TV = 9.97(95 lines)  AV=2.28  SV=8.00  BV=4.24
    {1071,8160,1024, 0, 0, 0},  //TV = 9.87(102 lines)  AV=2.28  SV=7.99  BV=4.15
    {1145,8176,1024, 0, 0, 0},  //TV = 9.77(109 lines)  AV=2.28  SV=8.00  BV=4.05
    {1229,8208,1024, 0, 0, 0},  //TV = 9.67(117 lines)  AV=2.28  SV=8.00  BV=3.94
    {1313,8192,1024, 0, 0, 0},  //TV = 9.57(125 lines)  AV=2.28  SV=8.00  BV=3.85
    {1407,8224,1024, 0, 0, 0},  //TV = 9.47(134 lines)  AV=2.28  SV=8.01  BV=3.74
    {1512,8176,1024, 0, 0, 0},  //TV = 9.37(144 lines)  AV=2.28  SV=8.00  BV=3.65
    {1617,8208,1024, 0, 0, 0},  //TV = 9.27(154 lines)  AV=2.28  SV=8.00  BV=3.54
    {1733,8176,1024, 0, 0, 0},  //TV = 9.17(165 lines)  AV=2.28  SV=8.00  BV=3.45
    {1869,8160,1024, 0, 0, 0},  //TV = 9.06(178 lines)  AV=2.28  SV=7.99  BV=3.34
    {1995,8176,1024, 0, 0, 0},  //TV = 8.97(190 lines)  AV=2.28  SV=8.00  BV=3.25
    {2132,8192,1024, 0, 0, 0},  //TV = 8.87(203 lines)  AV=2.28  SV=8.00  BV=3.15
    {2289,8208,1024, 0, 0, 0},  //TV = 8.77(218 lines)  AV=2.28  SV=8.00  BV=3.04
    {2457,8176,1024, 0, 0, 0},  //TV = 8.67(234 lines)  AV=2.28  SV=8.00  BV=2.95
    {2625,8192,1024, 0, 0, 0},  //TV = 8.57(250 lines)  AV=2.28  SV=8.00  BV=2.85
    {2825,8192,1024, 0, 0, 0},  //TV = 8.47(269 lines)  AV=2.28  SV=8.00  BV=2.74
    {3024,8192,1024, 0, 0, 0},  //TV = 8.37(288 lines)  AV=2.28  SV=8.00  BV=2.64
    {3234,8192,1024, 0, 0, 0},  //TV = 8.27(308 lines)  AV=2.28  SV=8.00  BV=2.55
    {3465,8192,1024, 0, 0, 0},  //TV = 8.17(330 lines)  AV=2.28  SV=8.00  BV=2.45
    {3738,8176,1024, 0, 0, 0},  //TV = 8.06(356 lines)  AV=2.28  SV=8.00  BV=2.34
    {4001,8176,1024, 0, 0, 0},  //TV = 7.97(381 lines)  AV=2.28  SV=8.00  BV=2.24
    {4274,8192,1024, 0, 0, 0},  //TV = 7.87(407 lines)  AV=2.28  SV=8.00  BV=2.15
    {4578,8192,1024, 0, 0, 0},  //TV = 7.77(436 lines)  AV=2.28  SV=8.00  BV=2.05
    {4904,8176,1024, 0, 0, 0},  //TV = 7.67(467 lines)  AV=2.28  SV=8.00  BV=1.95
    {5250,8192,1024, 0, 0, 0},  //TV = 7.57(500 lines)  AV=2.28  SV=8.00  BV=1.85
    {5618,8192,1024, 0, 0, 0},  //TV = 7.48(535 lines)  AV=2.28  SV=8.00  BV=1.75
    {6059,8192,1024, 0, 0, 0},  //TV = 7.37(577 lines)  AV=2.28  SV=8.00  BV=1.64
    {6489,8192,1024, 0, 0, 0},  //TV = 7.27(618 lines)  AV=2.28  SV=8.00  BV=1.54
    {6951,8192,1024, 0, 0, 0},  //TV = 7.17(662 lines)  AV=2.28  SV=8.00  BV=1.44
    {7445,8192,1024, 0, 0, 0},  //TV = 7.07(709 lines)  AV=2.28  SV=8.00  BV=1.34
    {7980,8192,1024, 0, 0, 0},  //TV = 6.97(760 lines)  AV=2.28  SV=8.00  BV=1.24
    {8337,8400,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=8.04  BV=1.15
    {8337,8992,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=8.13  BV=1.05
    {8337,9632,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=8.23  BV=0.95
    {8337,10320,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=8.33  BV=0.85
    {8337,11056,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=8.43  BV=0.75
    {8337,11840,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=8.53  BV=0.65
    {16664,6352,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=7.63  BV=0.55
    {16664,6848,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=7.74  BV=0.44
    {16664,7344,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=7.84  BV=0.34
    {16664,7856,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=7.94  BV=0.24
    {16664,8416,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=8.04  BV=0.14
    {16664,9024,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=8.14  BV=0.04
    {16664,9664,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=8.24  BV=-0.06
    {25001,6912,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=7.75  BV=-0.16
    {25001,7392,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=7.85  BV=-0.25
    {25001,7920,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=7.95  BV=-0.35
    {25001,8496,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=8.05  BV=-0.46
    {25001,9104,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=8.15  BV=-0.56
    {33327,7312,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.84  BV=-0.65
    {33327,7840,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.94  BV=-0.75
    {33327,8400,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.04  BV=-0.85
    {33327,9008,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.14  BV=-0.95
    {41664,7712,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=7.91  BV=-1.05
    {41664,8272,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=8.01  BV=-1.15
    {41664,8864,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=8.11  BV=-1.25
    {50001,7920,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=7.95  BV=-1.35
    {50001,8480,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=8.05  BV=-1.45
    {58328,7792,1024, 0, 0, 0},  //TV = 4.10(5555 lines)  AV=2.28  SV=7.93  BV=-1.55
    {58328,8352,1024, 0, 0, 0},  //TV = 4.10(5555 lines)  AV=2.28  SV=8.03  BV=-1.65
    {66665,7824,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=7.93  BV=-1.75
    {66665,8400,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=8.04  BV=-1.85
    {75002,8000,1024, 0, 0, 0},  //TV = 3.74(7143 lines)  AV=2.28  SV=7.97  BV=-1.95
    {75002,8560,1024, 0, 0, 0},  //TV = 3.74(7143 lines)  AV=2.28  SV=8.06  BV=-2.05
    {83328,8272,1024, 0, 0, 0},  //TV = 3.59(7936 lines)  AV=2.28  SV=8.01  BV=-2.15
    {91665,8048,1024, 0, 0, 0},  //TV = 3.45(8730 lines)  AV=2.28  SV=7.97  BV=-2.25
    {99992,7904,1024, 0, 0, 0},  //TV = 3.32(9523 lines)  AV=2.28  SV=7.95  BV=-2.35
    {99992,8480,1024, 0, 0, 0},  //TV = 3.32(9523 lines)  AV=2.28  SV=8.05  BV=-2.45
    {108329,8384,1024, 0, 0, 0},  //TV = 3.21(10317 lines)  AV=2.28  SV=8.03  BV=-2.55
    {116666,8352,1024, 0, 0, 0},  //TV = 3.10(11111 lines)  AV=2.28  SV=8.03  BV=-2.65
    {124992,8352,1024, 0, 0, 0},  //TV = 3.00(11904 lines)  AV=2.28  SV=8.03  BV=-2.75
    {133329,8384,1024, 0, 0, 0},  //TV = 2.91(12698 lines)  AV=2.28  SV=8.03  BV=-2.85
    {149993,7984,1024, 0, 0, 0},  //TV = 2.74(14285 lines)  AV=2.28  SV=7.96  BV=-2.95
    {158330,8112,1024, 0, 0, 0},  //TV = 2.66(15079 lines)  AV=2.28  SV=7.99  BV=-3.05
    {166656,8256,1024, 0, 0, 0},  //TV = 2.59(15872 lines)  AV=2.28  SV=8.01  BV=-3.15
    {183330,8048,1024, 0, 0, 0},  //TV = 2.45(17460 lines)  AV=2.28  SV=7.97  BV=-3.25
    {191657,8256,1024, 0, 0, 0},  //TV = 2.38(18253 lines)  AV=2.28  SV=8.01  BV=-3.35
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {494,1264,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.30  BV=7.95
    {494,1360,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.41  BV=7.85
    {494,1456,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.51  BV=7.75
    {494,1568,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.61  BV=7.64
    {494,1680,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.71  BV=7.54
    {494,1808,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.82  BV=7.44
    {494,1936,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.92  BV=7.34
    {494,2064,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.01  BV=7.25
    {494,2208,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.11  BV=7.15
    {494,2368,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.21  BV=7.05
    {494,2544,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.31  BV=6.95
    {494,2720,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.41  BV=6.85
    {494,2928,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.52  BV=6.74
    {494,3136,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.61  BV=6.64
    {494,3344,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.71  BV=6.55
    {494,3584,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.81  BV=6.45
    {494,3840,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.91  BV=6.35
    {494,4144,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.02  BV=6.24
    {494,4432,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.11  BV=6.14
    {494,4752,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.21  BV=6.04
    {494,5088,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.31  BV=5.95
    {494,5440,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.41  BV=5.85
    {494,5872,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.52  BV=5.74
    {494,6288,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.62  BV=5.64
    {494,6720,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.71  BV=5.54
    {494,7200,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.81  BV=5.44
    {494,7712,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.91  BV=5.35
    {494,8272,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.01  BV=5.24
    {536,8144,1024, 0, 0, 0},  //TV = 10.87(51 lines)  AV=2.28  SV=7.99  BV=5.15
    {567,8256,1024, 0, 0, 0},  //TV = 10.78(54 lines)  AV=2.28  SV=8.01  BV=5.05
    {609,8256,1024, 0, 0, 0},  //TV = 10.68(58 lines)  AV=2.28  SV=8.01  BV=4.95
    {662,8144,1024, 0, 0, 0},  //TV = 10.56(63 lines)  AV=2.28  SV=7.99  BV=4.84
    {704,8208,1024, 0, 0, 0},  //TV = 10.47(67 lines)  AV=2.28  SV=8.00  BV=4.74
    {756,8192,1024, 0, 0, 0},  //TV = 10.37(72 lines)  AV=2.28  SV=8.00  BV=4.64
    {809,8224,1024, 0, 0, 0},  //TV = 10.27(77 lines)  AV=2.28  SV=8.01  BV=4.54
    {872,8144,1024, 0, 0, 0},  //TV = 10.16(83 lines)  AV=2.28  SV=7.99  BV=4.45
    {935,8176,1024, 0, 0, 0},  //TV = 10.06(89 lines)  AV=2.28  SV=8.00  BV=4.34
    {998,8192,1024, 0, 0, 0},  //TV = 9.97(95 lines)  AV=2.28  SV=8.00  BV=4.24
    {1071,8160,1024, 0, 0, 0},  //TV = 9.87(102 lines)  AV=2.28  SV=7.99  BV=4.15
    {1145,8176,1024, 0, 0, 0},  //TV = 9.77(109 lines)  AV=2.28  SV=8.00  BV=4.05
    {1229,8208,1024, 0, 0, 0},  //TV = 9.67(117 lines)  AV=2.28  SV=8.00  BV=3.94
    {1313,8192,1024, 0, 0, 0},  //TV = 9.57(125 lines)  AV=2.28  SV=8.00  BV=3.85
    {1407,8224,1024, 0, 0, 0},  //TV = 9.47(134 lines)  AV=2.28  SV=8.01  BV=3.74
    {1512,8176,1024, 0, 0, 0},  //TV = 9.37(144 lines)  AV=2.28  SV=8.00  BV=3.65
    {1617,8208,1024, 0, 0, 0},  //TV = 9.27(154 lines)  AV=2.28  SV=8.00  BV=3.54
    {1733,8176,1024, 0, 0, 0},  //TV = 9.17(165 lines)  AV=2.28  SV=8.00  BV=3.45
    {1869,8160,1024, 0, 0, 0},  //TV = 9.06(178 lines)  AV=2.28  SV=7.99  BV=3.34
    {1995,8176,1024, 0, 0, 0},  //TV = 8.97(190 lines)  AV=2.28  SV=8.00  BV=3.25
    {2132,8192,1024, 0, 0, 0},  //TV = 8.87(203 lines)  AV=2.28  SV=8.00  BV=3.15
    {2289,8208,1024, 0, 0, 0},  //TV = 8.77(218 lines)  AV=2.28  SV=8.00  BV=3.04
    {2457,8176,1024, 0, 0, 0},  //TV = 8.67(234 lines)  AV=2.28  SV=8.00  BV=2.95
    {2625,8192,1024, 0, 0, 0},  //TV = 8.57(250 lines)  AV=2.28  SV=8.00  BV=2.85
    {2825,8192,1024, 0, 0, 0},  //TV = 8.47(269 lines)  AV=2.28  SV=8.00  BV=2.74
    {3024,8192,1024, 0, 0, 0},  //TV = 8.37(288 lines)  AV=2.28  SV=8.00  BV=2.64
    {3234,8192,1024, 0, 0, 0},  //TV = 8.27(308 lines)  AV=2.28  SV=8.00  BV=2.55
    {3465,8192,1024, 0, 0, 0},  //TV = 8.17(330 lines)  AV=2.28  SV=8.00  BV=2.45
    {3738,8176,1024, 0, 0, 0},  //TV = 8.06(356 lines)  AV=2.28  SV=8.00  BV=2.34
    {4001,8176,1024, 0, 0, 0},  //TV = 7.97(381 lines)  AV=2.28  SV=8.00  BV=2.24
    {4274,8192,1024, 0, 0, 0},  //TV = 7.87(407 lines)  AV=2.28  SV=8.00  BV=2.15
    {4578,8192,1024, 0, 0, 0},  //TV = 7.77(436 lines)  AV=2.28  SV=8.00  BV=2.05
    {4904,8176,1024, 0, 0, 0},  //TV = 7.67(467 lines)  AV=2.28  SV=8.00  BV=1.95
    {5250,8192,1024, 0, 0, 0},  //TV = 7.57(500 lines)  AV=2.28  SV=8.00  BV=1.85
    {5618,8192,1024, 0, 0, 0},  //TV = 7.48(535 lines)  AV=2.28  SV=8.00  BV=1.75
    {6059,8192,1024, 0, 0, 0},  //TV = 7.37(577 lines)  AV=2.28  SV=8.00  BV=1.64
    {6489,8192,1024, 0, 0, 0},  //TV = 7.27(618 lines)  AV=2.28  SV=8.00  BV=1.54
    {6951,8192,1024, 0, 0, 0},  //TV = 7.17(662 lines)  AV=2.28  SV=8.00  BV=1.44
    {7445,8192,1024, 0, 0, 0},  //TV = 7.07(709 lines)  AV=2.28  SV=8.00  BV=1.34
    {7980,8192,1024, 0, 0, 0},  //TV = 6.97(760 lines)  AV=2.28  SV=8.00  BV=1.24
    {8547,8192,1024, 0, 0, 0},  //TV = 6.87(814 lines)  AV=2.28  SV=8.00  BV=1.15
    {9156,8192,1024, 0, 0, 0},  //TV = 6.77(872 lines)  AV=2.28  SV=8.00  BV=1.05
    {9807,8192,1024, 0, 0, 0},  //TV = 6.67(934 lines)  AV=2.28  SV=8.00  BV=0.95
    {9996,8608,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=8.07  BV=0.85
    {9996,9216,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=8.17  BV=0.75
    {9996,9872,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=8.27  BV=0.65
    {9996,10576,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=8.37  BV=0.55
    {9996,11344,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=8.47  BV=0.45
    {9996,12240,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=8.58  BV=0.34
    {20003,6544,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=7.68  BV=0.24
    {20003,7008,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=7.77  BV=0.14
    {20003,7520,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=7.88  BV=0.04
    {20003,8048,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=7.97  BV=-0.06
    {20003,8624,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=8.07  BV=-0.16
    {20003,9248,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=8.17  BV=-0.26
    {20003,9904,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=8.27  BV=-0.36
    {29999,7072,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.79  BV=-0.45
    {29999,7584,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.89  BV=-0.55
    {29999,8128,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.99  BV=-0.65
    {29999,8720,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.09  BV=-0.76
    {29999,9344,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.19  BV=-0.86
    {40005,7504,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=7.87  BV=-0.95
    {40005,8032,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=7.97  BV=-1.05
    {40005,8608,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=8.07  BV=-1.15
    {50001,7392,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=7.85  BV=-1.25
    {50001,7920,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=7.95  BV=-1.35
    {50001,8480,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=8.05  BV=-1.45
    {59997,7584,1024, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=7.89  BV=-1.55
    {59997,8112,1024, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=7.99  BV=-1.65
    {59997,8704,1024, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=8.09  BV=-1.75
    {70004,8000,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=7.97  BV=-1.85
    {70004,8560,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=8.06  BV=-1.95
    {80000,8032,1024, 0, 0, 0},  //TV = 3.64(7619 lines)  AV=2.28  SV=7.97  BV=-2.05
    {80000,8608,1024, 0, 0, 0},  //TV = 3.64(7619 lines)  AV=2.28  SV=8.07  BV=-2.15
    {89996,8208,1024, 0, 0, 0},  //TV = 3.47(8571 lines)  AV=2.28  SV=8.00  BV=-2.25
    {100002,7904,1024, 0, 0, 0},  //TV = 3.32(9524 lines)  AV=2.28  SV=7.95  BV=-2.35
    {100002,8480,1024, 0, 0, 0},  //TV = 3.32(9524 lines)  AV=2.28  SV=8.05  BV=-2.45
    {109998,8256,1024, 0, 0, 0},  //TV = 3.18(10476 lines)  AV=2.28  SV=8.01  BV=-2.55
    {120005,8112,1024, 0, 0, 0},  //TV = 3.06(11429 lines)  AV=2.28  SV=7.99  BV=-2.65
    {130001,8032,1024, 0, 0, 0},  //TV = 2.94(12381 lines)  AV=2.28  SV=7.97  BV=-2.75
    {139997,7984,1024, 0, 0, 0},  //TV = 2.84(13333 lines)  AV=2.28  SV=7.96  BV=-2.85
    {150004,7984,1024, 0, 0, 0},  //TV = 2.74(14286 lines)  AV=2.28  SV=7.96  BV=-2.95
    {160000,8032,1024, 0, 0, 0},  //TV = 2.64(15238 lines)  AV=2.28  SV=7.97  BV=-3.05
    {169996,8096,1024, 0, 0, 0},  //TV = 2.56(16190 lines)  AV=2.28  SV=7.98  BV=-3.15
    {180002,8192,1024, 0, 0, 0},  //TV = 2.47(17143 lines)  AV=2.28  SV=8.00  BV=-3.25
    {189998,8320,1024, 0, 0, 0},  //TV = 2.40(18095 lines)  AV=2.28  SV=8.02  BV=-3.35
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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
    138, //u4TotalIndex
    20, //i4StrobeTrigerBV
    104, //i4MaxBV
    -34, //i4MinBV
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
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {494,1264,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.30  BV=7.95
    {494,1360,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.41  BV=7.85
    {494,1456,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.51  BV=7.75
    {494,1568,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.61  BV=7.64
    {494,1680,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.71  BV=7.54
    {494,1808,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.82  BV=7.44
    {494,1936,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.92  BV=7.34
    {494,2064,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.01  BV=7.25
    {494,2208,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.11  BV=7.15
    {494,2368,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.21  BV=7.05
    {494,2544,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.31  BV=6.95
    {494,2720,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.41  BV=6.85
    {494,2928,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.52  BV=6.74
    {494,3136,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.61  BV=6.64
    {494,3344,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.71  BV=6.55
    {494,3584,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.81  BV=6.45
    {494,3840,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.91  BV=6.35
    {494,4144,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.02  BV=6.24
    {494,4432,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.11  BV=6.14
    {494,4752,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.21  BV=6.04
    {494,5088,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.31  BV=5.95
    {494,5440,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.41  BV=5.85
    {494,5872,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.52  BV=5.74
    {494,6288,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.62  BV=5.64
    {494,6720,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.71  BV=5.54
    {494,7200,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.81  BV=5.44
    {494,7712,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.91  BV=5.35
    {494,8272,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.01  BV=5.24
    {494,8848,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.11  BV=5.15
    {494,9488,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.21  BV=5.05
    {494,10160,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.31  BV=4.95
    {494,10880,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.41  BV=4.85
    {494,11664,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.51  BV=4.75
    {494,12496,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.61  BV=4.65
    {494,13376,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.71  BV=4.55
    {494,14432,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.82  BV=4.44
    {494,15472,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.92  BV=4.34
    {494,15872,1064, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.01  BV=4.25
    {536,15872,1048, 0, 0, 0},  //TV = 10.87(51 lines)  AV=2.28  SV=8.99  BV=4.15
    {567,15872,1064, 0, 0, 0},  //TV = 10.78(54 lines)  AV=2.28  SV=9.01  BV=4.05
    {609,15872,1064, 0, 0, 0},  //TV = 10.68(58 lines)  AV=2.28  SV=9.01  BV=3.95
    {662,15872,1048, 0, 0, 0},  //TV = 10.56(63 lines)  AV=2.28  SV=8.99  BV=3.85
    {704,15872,1056, 0, 0, 0},  //TV = 10.47(67 lines)  AV=2.28  SV=9.00  BV=3.75
    {756,15872,1056, 0, 0, 0},  //TV = 10.37(72 lines)  AV=2.28  SV=9.00  BV=3.65
    {809,15872,1056, 0, 0, 0},  //TV = 10.27(77 lines)  AV=2.28  SV=9.00  BV=3.55
    {872,15872,1048, 0, 0, 0},  //TV = 10.16(83 lines)  AV=2.28  SV=8.99  BV=3.45
    {924,15872,1056, 0, 0, 0},  //TV = 10.08(88 lines)  AV=2.28  SV=9.00  BV=3.36
    {998,15872,1056, 0, 0, 0},  //TV = 9.97(95 lines)  AV=2.28  SV=9.00  BV=3.25
    {1071,15872,1048, 0, 0, 0},  //TV = 9.87(102 lines)  AV=2.28  SV=8.99  BV=3.15
    {1145,15872,1056, 0, 0, 0},  //TV = 9.77(109 lines)  AV=2.28  SV=9.00  BV=3.05
    {1229,15872,1056, 0, 0, 0},  //TV = 9.67(117 lines)  AV=2.28  SV=9.00  BV=2.94
    {1313,15872,1056, 0, 0, 0},  //TV = 9.57(125 lines)  AV=2.28  SV=9.00  BV=2.85
    {1407,15872,1056, 0, 0, 0},  //TV = 9.47(134 lines)  AV=2.28  SV=9.00  BV=2.75
    {1512,15872,1048, 0, 0, 0},  //TV = 9.37(144 lines)  AV=2.28  SV=8.99  BV=2.66
    {1617,15872,1056, 0, 0, 0},  //TV = 9.27(154 lines)  AV=2.28  SV=9.00  BV=2.55
    {1733,15872,1056, 0, 0, 0},  //TV = 9.17(165 lines)  AV=2.28  SV=9.00  BV=2.45
    {1869,15872,1048, 0, 0, 0},  //TV = 9.06(178 lines)  AV=2.28  SV=8.99  BV=2.35
    {1995,15872,1056, 0, 0, 0},  //TV = 8.97(190 lines)  AV=2.28  SV=9.00  BV=2.25
    {2132,15872,1056, 0, 0, 0},  //TV = 8.87(203 lines)  AV=2.28  SV=9.00  BV=2.15
    {2289,15872,1056, 0, 0, 0},  //TV = 8.77(218 lines)  AV=2.28  SV=9.00  BV=2.05
    {2457,15872,1048, 0, 0, 0},  //TV = 8.67(234 lines)  AV=2.28  SV=8.99  BV=1.96
    {2625,15872,1056, 0, 0, 0},  //TV = 8.57(250 lines)  AV=2.28  SV=9.00  BV=1.85
    {2825,15872,1056, 0, 0, 0},  //TV = 8.47(269 lines)  AV=2.28  SV=9.00  BV=1.74
    {3024,15872,1056, 0, 0, 0},  //TV = 8.37(288 lines)  AV=2.28  SV=9.00  BV=1.65
    {3234,15872,1056, 0, 0, 0},  //TV = 8.27(308 lines)  AV=2.28  SV=9.00  BV=1.55
    {3465,15872,1056, 0, 0, 0},  //TV = 8.17(330 lines)  AV=2.28  SV=9.00  BV=1.45
    {3707,15872,1056, 0, 0, 0},  //TV = 8.08(353 lines)  AV=2.28  SV=9.00  BV=1.35
    {4001,15872,1048, 0, 0, 0},  //TV = 7.97(381 lines)  AV=2.28  SV=8.99  BV=1.25
    {4274,15872,1056, 0, 0, 0},  //TV = 7.87(407 lines)  AV=2.28  SV=9.00  BV=1.15
    {4578,15872,1056, 0, 0, 0},  //TV = 7.77(436 lines)  AV=2.28  SV=9.00  BV=1.05
    {4904,15872,1056, 0, 0, 0},  //TV = 7.67(467 lines)  AV=2.28  SV=9.00  BV=0.95
    {5250,15872,1056, 0, 0, 0},  //TV = 7.57(500 lines)  AV=2.28  SV=9.00  BV=0.85
    {5660,15872,1056, 0, 0, 0},  //TV = 7.46(539 lines)  AV=2.28  SV=9.00  BV=0.74
    {6059,15872,1056, 0, 0, 0},  //TV = 7.37(577 lines)  AV=2.28  SV=9.00  BV=0.64
    {6489,15872,1056, 0, 0, 0},  //TV = 7.27(618 lines)  AV=2.28  SV=9.00  BV=0.54
    {6951,15872,1056, 0, 0, 0},  //TV = 7.17(662 lines)  AV=2.28  SV=9.00  BV=0.44
    {7445,15872,1056, 0, 0, 0},  //TV = 7.07(709 lines)  AV=2.28  SV=9.00  BV=0.35
    {7980,15872,1056, 0, 0, 0},  //TV = 6.97(760 lines)  AV=2.28  SV=9.00  BV=0.25
    {8337,15872,1080, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=9.03  BV=0.15
    {8337,15872,1160, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=9.13  BV=0.05
    {8337,15872,1240, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=9.23  BV=-0.05
    {8337,15872,1328, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=9.33  BV=-0.15
    {8337,15872,1424, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=9.43  BV=-0.25
    {8337,15872,1528, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=9.53  BV=-0.35
    {16664,12704,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=8.63  BV=-0.45
    {16664,13600,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=8.73  BV=-0.55
    {16664,14672,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=8.84  BV=-0.66
    {16664,15728,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=8.94  BV=-0.76
    {16664,15872,1080, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=9.03  BV=-0.85
    {16664,15872,1160, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=9.13  BV=-0.95
    {16664,15872,1248, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=9.24  BV=-1.06
    {25001,13808,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=8.75  BV=-1.16
    {25001,14800,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=8.85  BV=-1.26
    {25001,15856,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=8.95  BV=-1.36
    {25001,15872,1096, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=9.05  BV=-1.46
    {25001,15872,1168, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=9.14  BV=-1.55
    {33327,14640,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.84  BV=-1.66
    {33327,15680,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.94  BV=-1.75
    {33327,15872,1080, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=9.03  BV=-1.85
    {33327,15872,1160, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=9.13  BV=-1.95
    {41664,15440,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=8.91  BV=-2.05
    {41664,15872,1064, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.01  BV=-2.15
    {41664,15872,1144, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.11  BV=-2.25
    {50001,15840,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=8.95  BV=-2.35
    {50001,15872,1088, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=9.04  BV=-2.44
    {58328,15584,1024, 0, 0, 0},  //TV = 4.10(5555 lines)  AV=2.28  SV=8.93  BV=-2.55
    {58328,15872,1072, 0, 0, 0},  //TV = 4.10(5555 lines)  AV=2.28  SV=9.02  BV=-2.65
    {66665,15664,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=8.94  BV=-2.75
    {66665,15872,1080, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=9.03  BV=-2.85
    {75002,15872,1032, 0, 0, 0},  //TV = 3.74(7143 lines)  AV=2.28  SV=8.97  BV=-2.95
    {75002,15872,1104, 0, 0, 0},  //TV = 3.74(7143 lines)  AV=2.28  SV=9.06  BV=-3.05
    {83328,15872,1064, 0, 0, 0},  //TV = 3.59(7936 lines)  AV=2.28  SV=9.01  BV=-3.15
    {91665,15872,1032, 0, 0, 0},  //TV = 3.45(8730 lines)  AV=2.28  SV=8.97  BV=-3.24
    {99992,15824,1024, 0, 0, 0},  //TV = 3.32(9523 lines)  AV=2.28  SV=8.95  BV=-3.35
    {99992,15872,1088, 0, 0, 0},  //TV = 3.32(9523 lines)  AV=2.28  SV=9.04  BV=-3.44
    {108329,15872,1080, 0, 0, 0},  //TV = 3.21(10317 lines)  AV=2.28  SV=9.03  BV=-3.55
    {116666,15872,1072, 0, 0, 0},  //TV = 3.10(11111 lines)  AV=2.28  SV=9.02  BV=-3.65
    {124992,15872,1072, 0, 0, 0},  //TV = 3.00(11904 lines)  AV=2.28  SV=9.02  BV=-3.75
    {133329,15872,1080, 0, 0, 0},  //TV = 2.91(12698 lines)  AV=2.28  SV=9.03  BV=-3.85
    {149993,15872,1024, 0, 0, 0},  //TV = 2.74(14285 lines)  AV=2.28  SV=8.95  BV=-3.94
    {158330,15872,1040, 0, 0, 0},  //TV = 2.66(15079 lines)  AV=2.28  SV=8.98  BV=-4.04
    {166656,15872,1064, 0, 0, 0},  //TV = 2.59(15872 lines)  AV=2.28  SV=9.01  BV=-4.15
    {183330,15872,1032, 0, 0, 0},  //TV = 2.45(17460 lines)  AV=2.28  SV=8.97  BV=-4.24
    {191657,15872,1064, 0, 0, 0},  //TV = 2.38(18253 lines)  AV=2.28  SV=9.01  BV=-4.35
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {494,1264,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.30  BV=7.95
    {494,1360,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.41  BV=7.85
    {494,1456,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.51  BV=7.75
    {494,1568,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.61  BV=7.64
    {494,1680,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.71  BV=7.54
    {494,1808,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.82  BV=7.44
    {494,1936,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.92  BV=7.34
    {494,2064,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.01  BV=7.25
    {494,2208,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.11  BV=7.15
    {494,2368,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.21  BV=7.05
    {494,2544,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.31  BV=6.95
    {494,2720,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.41  BV=6.85
    {494,2928,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.52  BV=6.74
    {494,3136,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.61  BV=6.64
    {494,3344,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.71  BV=6.55
    {494,3584,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.81  BV=6.45
    {494,3840,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.91  BV=6.35
    {494,4144,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.02  BV=6.24
    {494,4432,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.11  BV=6.14
    {494,4752,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.21  BV=6.04
    {494,5088,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.31  BV=5.95
    {494,5440,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.41  BV=5.85
    {494,5872,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.52  BV=5.74
    {494,6288,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.62  BV=5.64
    {494,6720,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.71  BV=5.54
    {494,7200,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.81  BV=5.44
    {494,7712,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.91  BV=5.35
    {494,8272,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.01  BV=5.24
    {494,8848,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.11  BV=5.15
    {494,9488,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.21  BV=5.05
    {494,10160,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.31  BV=4.95
    {494,10880,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.41  BV=4.85
    {494,11664,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.51  BV=4.75
    {494,12496,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.61  BV=4.65
    {494,13376,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.71  BV=4.55
    {494,14432,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.82  BV=4.44
    {494,15472,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.92  BV=4.34
    {494,15872,1064, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.01  BV=4.25
    {536,15872,1048, 0, 0, 0},  //TV = 10.87(51 lines)  AV=2.28  SV=8.99  BV=4.15
    {567,15872,1064, 0, 0, 0},  //TV = 10.78(54 lines)  AV=2.28  SV=9.01  BV=4.05
    {609,15872,1064, 0, 0, 0},  //TV = 10.68(58 lines)  AV=2.28  SV=9.01  BV=3.95
    {662,15872,1048, 0, 0, 0},  //TV = 10.56(63 lines)  AV=2.28  SV=8.99  BV=3.85
    {704,15872,1056, 0, 0, 0},  //TV = 10.47(67 lines)  AV=2.28  SV=9.00  BV=3.75
    {756,15872,1056, 0, 0, 0},  //TV = 10.37(72 lines)  AV=2.28  SV=9.00  BV=3.65
    {809,15872,1056, 0, 0, 0},  //TV = 10.27(77 lines)  AV=2.28  SV=9.00  BV=3.55
    {872,15872,1048, 0, 0, 0},  //TV = 10.16(83 lines)  AV=2.28  SV=8.99  BV=3.45
    {924,15872,1056, 0, 0, 0},  //TV = 10.08(88 lines)  AV=2.28  SV=9.00  BV=3.36
    {998,15872,1056, 0, 0, 0},  //TV = 9.97(95 lines)  AV=2.28  SV=9.00  BV=3.25
    {1071,15872,1048, 0, 0, 0},  //TV = 9.87(102 lines)  AV=2.28  SV=8.99  BV=3.15
    {1145,15872,1056, 0, 0, 0},  //TV = 9.77(109 lines)  AV=2.28  SV=9.00  BV=3.05
    {1229,15872,1056, 0, 0, 0},  //TV = 9.67(117 lines)  AV=2.28  SV=9.00  BV=2.94
    {1313,15872,1056, 0, 0, 0},  //TV = 9.57(125 lines)  AV=2.28  SV=9.00  BV=2.85
    {1407,15872,1056, 0, 0, 0},  //TV = 9.47(134 lines)  AV=2.28  SV=9.00  BV=2.75
    {1512,15872,1048, 0, 0, 0},  //TV = 9.37(144 lines)  AV=2.28  SV=8.99  BV=2.66
    {1617,15872,1056, 0, 0, 0},  //TV = 9.27(154 lines)  AV=2.28  SV=9.00  BV=2.55
    {1733,15872,1056, 0, 0, 0},  //TV = 9.17(165 lines)  AV=2.28  SV=9.00  BV=2.45
    {1869,15872,1048, 0, 0, 0},  //TV = 9.06(178 lines)  AV=2.28  SV=8.99  BV=2.35
    {1995,15872,1056, 0, 0, 0},  //TV = 8.97(190 lines)  AV=2.28  SV=9.00  BV=2.25
    {2132,15872,1056, 0, 0, 0},  //TV = 8.87(203 lines)  AV=2.28  SV=9.00  BV=2.15
    {2289,15872,1056, 0, 0, 0},  //TV = 8.77(218 lines)  AV=2.28  SV=9.00  BV=2.05
    {2457,15872,1048, 0, 0, 0},  //TV = 8.67(234 lines)  AV=2.28  SV=8.99  BV=1.96
    {2625,15872,1056, 0, 0, 0},  //TV = 8.57(250 lines)  AV=2.28  SV=9.00  BV=1.85
    {2825,15872,1056, 0, 0, 0},  //TV = 8.47(269 lines)  AV=2.28  SV=9.00  BV=1.74
    {3024,15872,1056, 0, 0, 0},  //TV = 8.37(288 lines)  AV=2.28  SV=9.00  BV=1.65
    {3234,15872,1056, 0, 0, 0},  //TV = 8.27(308 lines)  AV=2.28  SV=9.00  BV=1.55
    {3465,15872,1056, 0, 0, 0},  //TV = 8.17(330 lines)  AV=2.28  SV=9.00  BV=1.45
    {3707,15872,1056, 0, 0, 0},  //TV = 8.08(353 lines)  AV=2.28  SV=9.00  BV=1.35
    {4001,15872,1048, 0, 0, 0},  //TV = 7.97(381 lines)  AV=2.28  SV=8.99  BV=1.25
    {4274,15872,1056, 0, 0, 0},  //TV = 7.87(407 lines)  AV=2.28  SV=9.00  BV=1.15
    {4578,15872,1056, 0, 0, 0},  //TV = 7.77(436 lines)  AV=2.28  SV=9.00  BV=1.05
    {4904,15872,1056, 0, 0, 0},  //TV = 7.67(467 lines)  AV=2.28  SV=9.00  BV=0.95
    {5250,15872,1056, 0, 0, 0},  //TV = 7.57(500 lines)  AV=2.28  SV=9.00  BV=0.85
    {5660,15872,1056, 0, 0, 0},  //TV = 7.46(539 lines)  AV=2.28  SV=9.00  BV=0.74
    {6059,15872,1056, 0, 0, 0},  //TV = 7.37(577 lines)  AV=2.28  SV=9.00  BV=0.64
    {6489,15872,1056, 0, 0, 0},  //TV = 7.27(618 lines)  AV=2.28  SV=9.00  BV=0.54
    {6951,15872,1056, 0, 0, 0},  //TV = 7.17(662 lines)  AV=2.28  SV=9.00  BV=0.44
    {7445,15872,1056, 0, 0, 0},  //TV = 7.07(709 lines)  AV=2.28  SV=9.00  BV=0.35
    {7980,15872,1056, 0, 0, 0},  //TV = 6.97(760 lines)  AV=2.28  SV=9.00  BV=0.25
    {8547,15872,1056, 0, 0, 0},  //TV = 6.87(814 lines)  AV=2.28  SV=9.00  BV=0.15
    {9156,15872,1056, 0, 0, 0},  //TV = 6.77(872 lines)  AV=2.28  SV=9.00  BV=0.05
    {9807,15872,1056, 0, 0, 0},  //TV = 6.67(934 lines)  AV=2.28  SV=9.00  BV=-0.05
    {9996,15872,1104, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=9.06  BV=-0.14
    {9996,15872,1184, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=9.16  BV=-0.24
    {9996,15872,1272, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=9.27  BV=-0.35
    {9996,15872,1360, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=9.36  BV=-0.44
    {9996,15872,1456, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=9.46  BV=-0.54
    {9996,15872,1576, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=9.58  BV=-0.66
    {20003,13104,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=8.68  BV=-0.76
    {20003,14032,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=8.78  BV=-0.86
    {20003,15040,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=8.88  BV=-0.96
    {20003,15872,1040, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=8.98  BV=-1.06
    {20003,15872,1112, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=9.07  BV=-1.15
    {20003,15872,1192, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=9.17  BV=-1.25
    {20003,15872,1272, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=9.27  BV=-1.35
    {29999,14160,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.79  BV=-1.46
    {29999,15184,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.89  BV=-1.56
    {29999,15872,1048, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.99  BV=-1.65
    {29999,15872,1120, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=9.08  BV=-1.75
    {29999,15872,1200, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=9.18  BV=-1.85
    {40005,15008,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=8.87  BV=-1.95
    {40005,15872,1032, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=8.97  BV=-2.05
    {40005,15872,1112, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.07  BV=-2.15
    {50001,14784,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=8.85  BV=-2.25
    {50001,15840,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=8.95  BV=-2.35
    {50001,15872,1088, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=9.04  BV=-2.44
    {59997,15152,1024, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=8.89  BV=-2.55
    {59997,15872,1048, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=8.99  BV=-2.65
    {59997,15872,1120, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=9.08  BV=-2.75
    {70004,15872,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=8.95  BV=-2.84
    {70004,15872,1104, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=9.06  BV=-2.95
    {80000,15872,1032, 0, 0, 0},  //TV = 3.64(7619 lines)  AV=2.28  SV=8.97  BV=-3.05
    {80000,15872,1104, 0, 0, 0},  //TV = 3.64(7619 lines)  AV=2.28  SV=9.06  BV=-3.14
    {89996,15872,1056, 0, 0, 0},  //TV = 3.47(8571 lines)  AV=2.28  SV=9.00  BV=-3.25
    {100002,15824,1024, 0, 0, 0},  //TV = 3.32(9524 lines)  AV=2.28  SV=8.95  BV=-3.35
    {100002,15872,1088, 0, 0, 0},  //TV = 3.32(9524 lines)  AV=2.28  SV=9.04  BV=-3.44
    {109998,15872,1064, 0, 0, 0},  //TV = 3.18(10476 lines)  AV=2.28  SV=9.01  BV=-3.55
    {120005,15872,1040, 0, 0, 0},  //TV = 3.06(11429 lines)  AV=2.28  SV=8.98  BV=-3.64
    {130001,15872,1032, 0, 0, 0},  //TV = 2.94(12381 lines)  AV=2.28  SV=8.97  BV=-3.75
    {139997,15872,1024, 0, 0, 0},  //TV = 2.84(13333 lines)  AV=2.28  SV=8.95  BV=-3.84
    {150004,15872,1024, 0, 0, 0},  //TV = 2.74(14286 lines)  AV=2.28  SV=8.95  BV=-3.94
    {160000,15872,1032, 0, 0, 0},  //TV = 2.64(15238 lines)  AV=2.28  SV=8.97  BV=-4.05
    {169996,15872,1040, 0, 0, 0},  //TV = 2.56(16190 lines)  AV=2.28  SV=8.98  BV=-4.15
    {180002,15872,1056, 0, 0, 0},  //TV = 2.47(17143 lines)  AV=2.28  SV=9.00  BV=-4.25
    {189998,15872,1072, 0, 0, 0},  //TV = 2.40(18095 lines)  AV=2.28  SV=9.02  BV=-4.35
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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
    148, //u4TotalIndex
    20, //i4StrobeTrigerBV
    104, //i4MaxBV
    -44, //i4MinBV
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
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {525,1200,1024, 0, 0, 0},  //TV = 10.90(50 lines)  AV=2.28  SV=5.23  BV=7.94
    {557,1200,1024, 0, 0, 0},  //TV = 10.81(53 lines)  AV=2.28  SV=5.23  BV=7.86
    {599,1200,1024, 0, 0, 0},  //TV = 10.71(57 lines)  AV=2.28  SV=5.23  BV=7.75
    {641,1200,1024, 0, 0, 0},  //TV = 10.61(61 lines)  AV=2.28  SV=5.23  BV=7.65
    {683,1216,1024, 0, 0, 0},  //TV = 10.52(65 lines)  AV=2.28  SV=5.25  BV=7.54
    {735,1200,1024, 0, 0, 0},  //TV = 10.41(70 lines)  AV=2.28  SV=5.23  BV=7.46
    {788,1200,1024, 0, 0, 0},  //TV = 10.31(75 lines)  AV=2.28  SV=5.23  BV=7.36
    {840,1216,1024, 0, 0, 0},  //TV = 10.22(80 lines)  AV=2.28  SV=5.25  BV=7.24
    {903,1200,1024, 0, 0, 0},  //TV = 10.11(86 lines)  AV=2.28  SV=5.23  BV=7.16
    {977,1200,1024, 0, 0, 0},  //TV = 10.00(93 lines)  AV=2.28  SV=5.23  BV=7.05
    {1040,1200,1024, 0, 0, 0},  //TV = 9.91(99 lines)  AV=2.28  SV=5.23  BV=6.96
    {1113,1200,1024, 0, 0, 0},  //TV = 9.81(106 lines)  AV=2.28  SV=5.23  BV=6.86
    {1197,1200,1024, 0, 0, 0},  //TV = 9.71(114 lines)  AV=2.28  SV=5.23  BV=6.75
    {1292,1200,1024, 0, 0, 0},  //TV = 9.60(123 lines)  AV=2.28  SV=5.23  BV=6.64
    {1376,1200,1024, 0, 0, 0},  //TV = 9.51(131 lines)  AV=2.28  SV=5.23  BV=6.55
    {1481,1200,1024, 0, 0, 0},  //TV = 9.40(141 lines)  AV=2.28  SV=5.23  BV=6.45
    {1575,1200,1024, 0, 0, 0},  //TV = 9.31(150 lines)  AV=2.28  SV=5.23  BV=6.36
    {1701,1200,1024, 0, 0, 0},  //TV = 9.20(162 lines)  AV=2.28  SV=5.23  BV=6.25
    {1827,1200,1024, 0, 0, 0},  //TV = 9.10(174 lines)  AV=2.28  SV=5.23  BV=6.14
    {1953,1200,1024, 0, 0, 0},  //TV = 9.00(186 lines)  AV=2.28  SV=5.23  BV=6.05
    {2090,1200,1024, 0, 0, 0},  //TV = 8.90(199 lines)  AV=2.28  SV=5.23  BV=5.95
    {2258,1200,1024, 0, 0, 0},  //TV = 8.79(215 lines)  AV=2.28  SV=5.23  BV=5.84
    {2415,1200,1024, 0, 0, 0},  //TV = 8.69(230 lines)  AV=2.28  SV=5.23  BV=5.74
    {2583,1200,1024, 0, 0, 0},  //TV = 8.60(246 lines)  AV=2.28  SV=5.23  BV=5.64
    {2762,1200,1024, 0, 0, 0},  //TV = 8.50(263 lines)  AV=2.28  SV=5.23  BV=5.55
    {2972,1200,1024, 0, 0, 0},  //TV = 8.39(283 lines)  AV=2.28  SV=5.23  BV=5.44
    {3182,1200,1024, 0, 0, 0},  //TV = 8.30(303 lines)  AV=2.28  SV=5.23  BV=5.34
    {3402,1200,1024, 0, 0, 0},  //TV = 8.20(324 lines)  AV=2.28  SV=5.23  BV=5.25
    {3644,1200,1024, 0, 0, 0},  //TV = 8.10(347 lines)  AV=2.28  SV=5.23  BV=5.15
    {3896,1200,1024, 0, 0, 0},  //TV = 8.00(371 lines)  AV=2.28  SV=5.23  BV=5.05
    {4179,1200,1024, 0, 0, 0},  //TV = 7.90(398 lines)  AV=2.28  SV=5.23  BV=4.95
    {4505,1200,1024, 0, 0, 0},  //TV = 7.79(429 lines)  AV=2.28  SV=5.23  BV=4.84
    {4820,1200,1024, 0, 0, 0},  //TV = 7.70(459 lines)  AV=2.28  SV=5.23  BV=4.74
    {5156,1200,1024, 0, 0, 0},  //TV = 7.60(491 lines)  AV=2.28  SV=5.23  BV=4.65
    {5523,1200,1024, 0, 0, 0},  //TV = 7.50(526 lines)  AV=2.28  SV=5.23  BV=4.55
    {5912,1200,1024, 0, 0, 0},  //TV = 7.40(563 lines)  AV=2.28  SV=5.23  BV=4.45
    {6332,1200,1024, 0, 0, 0},  //TV = 7.30(603 lines)  AV=2.28  SV=5.23  BV=4.35
    {6836,1200,1024, 0, 0, 0},  //TV = 7.19(651 lines)  AV=2.28  SV=5.23  BV=4.24
    {7319,1200,1024, 0, 0, 0},  //TV = 7.09(697 lines)  AV=2.28  SV=5.23  BV=4.14
    {7833,1200,1024, 0, 0, 0},  //TV = 7.00(746 lines)  AV=2.28  SV=5.23  BV=4.04
    {8338,1200,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.23  BV=3.95
    {8338,1296,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.34  BV=3.84
    {8338,1376,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.43  BV=3.75
    {8338,1488,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.54  BV=3.64
    {8338,1584,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.63  BV=3.55
    {8338,1696,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.73  BV=3.45
    {8338,1824,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.83  BV=3.35
    {8338,1952,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.93  BV=3.25
    {8338,2096,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.03  BV=3.15
    {8338,2240,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.13  BV=3.05
    {16664,1200,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.23  BV=2.95
    {16664,1296,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.34  BV=2.84
    {16664,1376,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.43  BV=2.76
    {16664,1488,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.54  BV=2.64
    {16664,1600,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.64  BV=2.54
    {16664,1696,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.73  BV=2.45
    {25001,1216,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.25  BV=2.35
    {25001,1296,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.34  BV=2.26
    {25001,1408,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.46  BV=2.14
    {25001,1488,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.54  BV=2.06
    {33327,1200,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.23  BV=1.95
    {33327,1296,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.34  BV=1.84
    {33327,1376,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.43  BV=1.76
    {33327,1488,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.54  BV=1.64
    {33327,1584,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.63  BV=1.55
    {33327,1696,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.73  BV=1.45
    {33327,1824,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.83  BV=1.35
    {33327,1952,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.93  BV=1.25
    {33327,2096,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.03  BV=1.15
    {33327,2256,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.14  BV=1.04
    {33327,2416,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.24  BV=0.94
    {33327,2576,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.33  BV=0.85
    {33327,2784,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.44  BV=0.74
    {33327,2976,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.54  BV=0.64
    {33327,3184,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.64  BV=0.55
    {33327,3408,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.73  BV=0.45
    {33327,3648,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.83  BV=0.35
    {33327,3920,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.94  BV=0.25
    {33327,4208,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.04  BV=0.14
    {33327,4496,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.13  BV=0.05
    {33327,4816,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.23  BV=-0.05
    {33327,5152,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.33  BV=-0.15
    {33327,5520,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.43  BV=-0.25
    {33327,5952,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.54  BV=-0.36
    {33327,6384,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.64  BV=-0.46
    {33327,6832,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.74  BV=-0.56
    {33327,7312,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.84  BV=-0.65
    {33327,7840,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.94  BV=-0.75
    {33327,8400,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.04  BV=-0.85
    {33327,8992,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.13  BV=-0.95
    {33327,9632,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.23  BV=-1.05
    {33327,10320,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.33  BV=-1.15
    {33327,11056,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.43  BV=-1.25
    {33327,11840,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.53  BV=-1.35
    {33327,12720,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.63  BV=-1.45
    {33327,13632,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.73  BV=-1.55
    {41664,11680,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=8.51  BV=-1.65
    {41664,12528,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=8.61  BV=-1.75
    {41664,13424,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=8.71  BV=-1.85
    {50001,11984,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=8.55  BV=-1.95
    {50001,12848,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=8.65  BV=-2.05
    {58328,11792,1024, 0, 0, 0},  //TV = 4.10(5555 lines)  AV=2.28  SV=8.53  BV=-2.15
    {58328,12640,1024, 0, 0, 0},  //TV = 4.10(5555 lines)  AV=2.28  SV=8.63  BV=-2.25
    {66665,11856,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=8.53  BV=-2.35
    {66665,12720,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=8.63  BV=-2.45
    {66665,13616,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=8.73  BV=-2.55
    {66665,14592,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=8.83  BV=-2.65
    {66665,15632,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=8.93  BV=-2.75
    {66665,15872,1080, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=9.03  BV=-2.85
    {66665,15872,1152, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=9.12  BV=-2.94
    {66665,15872,1248, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=9.24  BV=-3.06
    {66665,15872,1336, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=9.34  BV=-3.16
    {66665,15872,1432, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=9.44  BV=-3.26
    {66665,15872,1536, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=9.54  BV=-3.36
    {66665,15872,1648, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=9.64  BV=-3.46
    {66665,15872,1760, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=9.74  BV=-3.55
    {66665,15872,1888, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=9.84  BV=-3.65
    {66665,15872,2024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=9.94  BV=-3.76
    {66665,15872,2168, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=10.04  BV=-3.85
    {66665,15872,2328, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=10.14  BV=-3.96
    {66665,15872,2488, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=10.23  BV=-4.05
    {66665,15872,2672, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=10.34  BV=-4.16
    {66665,15872,2864, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=10.44  BV=-4.26
    {66665,15872,2968, 0, 0, 0},  /* TV = 3.91(6349 lines)  AV=2.28  SV=10.49  BV=-4.31 */
    {66665,15872,2968, 0, 0, 0},  /* TV = 3.91(6349 lines)  AV=2.28  SV=10.49  BV=-4.31 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {525,1200,1024, 0, 0, 0},  //TV = 10.90(50 lines)  AV=2.28  SV=5.23  BV=7.94
    {557,1200,1024, 0, 0, 0},  //TV = 10.81(53 lines)  AV=2.28  SV=5.23  BV=7.86
    {599,1200,1024, 0, 0, 0},  //TV = 10.71(57 lines)  AV=2.28  SV=5.23  BV=7.75
    {641,1200,1024, 0, 0, 0},  //TV = 10.61(61 lines)  AV=2.28  SV=5.23  BV=7.65
    {683,1216,1024, 0, 0, 0},  //TV = 10.52(65 lines)  AV=2.28  SV=5.25  BV=7.54
    {735,1200,1024, 0, 0, 0},  //TV = 10.41(70 lines)  AV=2.28  SV=5.23  BV=7.46
    {788,1200,1024, 0, 0, 0},  //TV = 10.31(75 lines)  AV=2.28  SV=5.23  BV=7.36
    {840,1216,1024, 0, 0, 0},  //TV = 10.22(80 lines)  AV=2.28  SV=5.25  BV=7.24
    {903,1200,1024, 0, 0, 0},  //TV = 10.11(86 lines)  AV=2.28  SV=5.23  BV=7.16
    {977,1200,1024, 0, 0, 0},  //TV = 10.00(93 lines)  AV=2.28  SV=5.23  BV=7.05
    {1040,1200,1024, 0, 0, 0},  //TV = 9.91(99 lines)  AV=2.28  SV=5.23  BV=6.96
    {1113,1200,1024, 0, 0, 0},  //TV = 9.81(106 lines)  AV=2.28  SV=5.23  BV=6.86
    {1197,1200,1024, 0, 0, 0},  //TV = 9.71(114 lines)  AV=2.28  SV=5.23  BV=6.75
    {1292,1200,1024, 0, 0, 0},  //TV = 9.60(123 lines)  AV=2.28  SV=5.23  BV=6.64
    {1376,1200,1024, 0, 0, 0},  //TV = 9.51(131 lines)  AV=2.28  SV=5.23  BV=6.55
    {1481,1200,1024, 0, 0, 0},  //TV = 9.40(141 lines)  AV=2.28  SV=5.23  BV=6.45
    {1575,1200,1024, 0, 0, 0},  //TV = 9.31(150 lines)  AV=2.28  SV=5.23  BV=6.36
    {1701,1200,1024, 0, 0, 0},  //TV = 9.20(162 lines)  AV=2.28  SV=5.23  BV=6.25
    {1827,1200,1024, 0, 0, 0},  //TV = 9.10(174 lines)  AV=2.28  SV=5.23  BV=6.14
    {1953,1200,1024, 0, 0, 0},  //TV = 9.00(186 lines)  AV=2.28  SV=5.23  BV=6.05
    {2090,1200,1024, 0, 0, 0},  //TV = 8.90(199 lines)  AV=2.28  SV=5.23  BV=5.95
    {2258,1200,1024, 0, 0, 0},  //TV = 8.79(215 lines)  AV=2.28  SV=5.23  BV=5.84
    {2415,1200,1024, 0, 0, 0},  //TV = 8.69(230 lines)  AV=2.28  SV=5.23  BV=5.74
    {2583,1200,1024, 0, 0, 0},  //TV = 8.60(246 lines)  AV=2.28  SV=5.23  BV=5.64
    {2762,1200,1024, 0, 0, 0},  //TV = 8.50(263 lines)  AV=2.28  SV=5.23  BV=5.55
    {2972,1200,1024, 0, 0, 0},  //TV = 8.39(283 lines)  AV=2.28  SV=5.23  BV=5.44
    {3182,1200,1024, 0, 0, 0},  //TV = 8.30(303 lines)  AV=2.28  SV=5.23  BV=5.34
    {3402,1200,1024, 0, 0, 0},  //TV = 8.20(324 lines)  AV=2.28  SV=5.23  BV=5.25
    {3644,1200,1024, 0, 0, 0},  //TV = 8.10(347 lines)  AV=2.28  SV=5.23  BV=5.15
    {3896,1200,1024, 0, 0, 0},  //TV = 8.00(371 lines)  AV=2.28  SV=5.23  BV=5.05
    {4179,1200,1024, 0, 0, 0},  //TV = 7.90(398 lines)  AV=2.28  SV=5.23  BV=4.95
    {4505,1200,1024, 0, 0, 0},  //TV = 7.79(429 lines)  AV=2.28  SV=5.23  BV=4.84
    {4820,1200,1024, 0, 0, 0},  //TV = 7.70(459 lines)  AV=2.28  SV=5.23  BV=4.74
    {5156,1200,1024, 0, 0, 0},  //TV = 7.60(491 lines)  AV=2.28  SV=5.23  BV=4.65
    {5523,1200,1024, 0, 0, 0},  //TV = 7.50(526 lines)  AV=2.28  SV=5.23  BV=4.55
    {5912,1200,1024, 0, 0, 0},  //TV = 7.40(563 lines)  AV=2.28  SV=5.23  BV=4.45
    {6332,1200,1024, 0, 0, 0},  //TV = 7.30(603 lines)  AV=2.28  SV=5.23  BV=4.35
    {6836,1200,1024, 0, 0, 0},  //TV = 7.19(651 lines)  AV=2.28  SV=5.23  BV=4.24
    {7319,1200,1024, 0, 0, 0},  //TV = 7.09(697 lines)  AV=2.28  SV=5.23  BV=4.14
    {7833,1200,1024, 0, 0, 0},  //TV = 7.00(746 lines)  AV=2.28  SV=5.23  BV=4.04
    {8400,1200,1024, 0, 0, 0},  //TV = 6.90(800 lines)  AV=2.28  SV=5.23  BV=3.94
    {8999,1200,1024, 0, 0, 0},  //TV = 6.80(857 lines)  AV=2.28  SV=5.23  BV=3.84
    {9639,1200,1024, 0, 0, 0},  //TV = 6.70(918 lines)  AV=2.28  SV=5.23  BV=3.74
    {9996,1232,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.27  BV=3.65
    {9996,1328,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.38  BV=3.54
    {9996,1424,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.48  BV=3.44
    {9996,1520,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.57  BV=3.35
    {9996,1632,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.67  BV=3.25
    {9996,1744,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.77  BV=3.15
    {9996,1872,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.87  BV=3.05
    {9996,2000,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.97  BV=2.95
    {9996,2144,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.07  BV=2.85
    {9996,2304,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.17  BV=2.75
    {20003,1232,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.27  BV=2.65
    {20003,1328,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.38  BV=2.54
    {20003,1424,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.48  BV=2.44
    {20003,1520,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.57  BV=2.35
    {20003,1632,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.67  BV=2.25
    {20003,1760,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.78  BV=2.14
    {29999,1248,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.29  BV=2.05
    {29999,1344,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.39  BV=1.94
    {29999,1440,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.49  BV=1.84
    {29999,1536,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.58  BV=1.75
    {29999,1648,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.69  BV=1.65
    {29999,1760,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.78  BV=1.55
    {29999,1888,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.88  BV=1.45
    {29999,2032,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.99  BV=1.35
    {29999,2176,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.09  BV=1.25
    {29999,2320,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.18  BV=1.15
    {29999,2496,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.29  BV=1.05
    {29999,2672,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.38  BV=0.95
    {29999,2864,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.48  BV=0.85
    {29999,3072,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.58  BV=0.75
    {29999,3296,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.69  BV=0.65
    {29999,3536,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.79  BV=0.55
    {29999,3776,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.88  BV=0.45
    {29999,4048,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.98  BV=0.35
    {29999,4368,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.09  BV=0.24
    {29999,4672,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.19  BV=0.14
    {29999,5008,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.29  BV=0.04
    {29999,5360,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.39  BV=-0.05
    {29999,5728,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.48  BV=-0.15
    {29999,6144,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.58  BV=-0.25
    {29999,6624,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.69  BV=-0.36
    {29999,7088,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.79  BV=-0.46
    {29999,7584,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.89  BV=-0.55
    {29999,8128,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.99  BV=-0.65
    {29999,8704,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.09  BV=-0.75
    {29999,9328,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.19  BV=-0.85
    {29999,9984,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.29  BV=-0.95
    {29999,10704,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.39  BV=-1.05
    {29999,11472,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.49  BV=-1.15
    {29999,12288,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.58  BV=-1.25
    {29999,13248,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.69  BV=-1.36
    {29999,14128,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.79  BV=-1.45
    {40005,11360,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=8.47  BV=-1.55
    {40005,12160,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=8.57  BV=-1.65
    {40005,13040,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=8.67  BV=-1.75
    {50001,11184,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=8.45  BV=-1.85
    {50001,11984,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=8.55  BV=-1.95
    {50001,12848,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=8.65  BV=-2.05
    {59997,11472,1024, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=8.49  BV=-2.15
    {59997,12288,1024, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=8.58  BV=-2.25
    {59997,13168,1024, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=8.68  BV=-2.35
    {70004,12112,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=8.56  BV=-2.45
    {70004,12976,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=8.66  BV=-2.55
    {70004,13904,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=8.76  BV=-2.65
    {70004,14896,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=8.86  BV=-2.75
    {70004,15872,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=8.95  BV=-2.84
    {70004,15872,1096, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=9.05  BV=-2.94
    {70004,15872,1176, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=9.15  BV=-3.04
    {70004,15872,1272, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=9.27  BV=-3.16
    {70004,15872,1360, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=9.36  BV=-3.25
    {70004,15872,1464, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=9.47  BV=-3.36
    {70004,15872,1568, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=9.57  BV=-3.46
    {70004,15872,1680, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=9.67  BV=-3.56
    {70004,15872,1800, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=9.77  BV=-3.66
    {70004,15872,1928, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=9.87  BV=-3.76
    {70004,15872,2064, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=9.97  BV=-3.85
    {70004,15872,2216, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=10.07  BV=-3.96
    {70004,15872,2376, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=10.17  BV=-4.06
    {70004,15872,2544, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=10.27  BV=-4.16
    {70004,15872,2728, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=10.37  BV=-4.26
    {70004,15872,2920, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=10.47  BV=-4.35
    {70004,15872,2968, 0, 0, 0},  /* TV = 3.84(6667 lines)  AV=2.28  SV=10.49  BV=-4.38 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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
    149, //u4TotalIndex
    20, //i4StrobeTrigerBV
    104, //i4MaxBV
    -44, //i4MinBV
    90, //i4EffectiveMaxBV
    -30, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_AUTO, //ISO
    sCaptureStrobePLineTable_60Hz,
    sCaptureStrobePLineTable_50Hz,
    NULL,
};
static strEvPline sAEScene1PLineTable_60Hz =
{
{
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {525,1200,1024, 0, 0, 0},  //TV = 10.90(50 lines)  AV=2.28  SV=5.23  BV=7.94
    {557,1200,1024, 0, 0, 0},  //TV = 10.81(53 lines)  AV=2.28  SV=5.23  BV=7.86
    {599,1200,1024, 0, 0, 0},  //TV = 10.71(57 lines)  AV=2.28  SV=5.23  BV=7.75
    {641,1200,1024, 0, 0, 0},  //TV = 10.61(61 lines)  AV=2.28  SV=5.23  BV=7.65
    {683,1216,1024, 0, 0, 0},  //TV = 10.52(65 lines)  AV=2.28  SV=5.25  BV=7.54
    {735,1200,1024, 0, 0, 0},  //TV = 10.41(70 lines)  AV=2.28  SV=5.23  BV=7.46
    {788,1200,1024, 0, 0, 0},  //TV = 10.31(75 lines)  AV=2.28  SV=5.23  BV=7.36
    {840,1216,1024, 0, 0, 0},  //TV = 10.22(80 lines)  AV=2.28  SV=5.25  BV=7.24
    {903,1200,1024, 0, 0, 0},  //TV = 10.11(86 lines)  AV=2.28  SV=5.23  BV=7.16
    {977,1200,1024, 0, 0, 0},  //TV = 10.00(93 lines)  AV=2.28  SV=5.23  BV=7.05
    {1040,1200,1024, 0, 0, 0},  //TV = 9.91(99 lines)  AV=2.28  SV=5.23  BV=6.96
    {1113,1200,1024, 0, 0, 0},  //TV = 9.81(106 lines)  AV=2.28  SV=5.23  BV=6.86
    {1197,1200,1024, 0, 0, 0},  //TV = 9.71(114 lines)  AV=2.28  SV=5.23  BV=6.75
    {1292,1200,1024, 0, 0, 0},  //TV = 9.60(123 lines)  AV=2.28  SV=5.23  BV=6.64
    {1376,1200,1024, 0, 0, 0},  //TV = 9.51(131 lines)  AV=2.28  SV=5.23  BV=6.55
    {1481,1200,1024, 0, 0, 0},  //TV = 9.40(141 lines)  AV=2.28  SV=5.23  BV=6.45
    {1575,1200,1024, 0, 0, 0},  //TV = 9.31(150 lines)  AV=2.28  SV=5.23  BV=6.36
    {1701,1200,1024, 0, 0, 0},  //TV = 9.20(162 lines)  AV=2.28  SV=5.23  BV=6.25
    {1827,1200,1024, 0, 0, 0},  //TV = 9.10(174 lines)  AV=2.28  SV=5.23  BV=6.14
    {1953,1200,1024, 0, 0, 0},  //TV = 9.00(186 lines)  AV=2.28  SV=5.23  BV=6.05
    {2090,1200,1024, 0, 0, 0},  //TV = 8.90(199 lines)  AV=2.28  SV=5.23  BV=5.95
    {2258,1200,1024, 0, 0, 0},  //TV = 8.79(215 lines)  AV=2.28  SV=5.23  BV=5.84
    {2415,1200,1024, 0, 0, 0},  //TV = 8.69(230 lines)  AV=2.28  SV=5.23  BV=5.74
    {2583,1200,1024, 0, 0, 0},  //TV = 8.60(246 lines)  AV=2.28  SV=5.23  BV=5.64
    {2762,1200,1024, 0, 0, 0},  //TV = 8.50(263 lines)  AV=2.28  SV=5.23  BV=5.55
    {2972,1200,1024, 0, 0, 0},  //TV = 8.39(283 lines)  AV=2.28  SV=5.23  BV=5.44
    {3182,1200,1024, 0, 0, 0},  //TV = 8.30(303 lines)  AV=2.28  SV=5.23  BV=5.34
    {3402,1200,1024, 0, 0, 0},  //TV = 8.20(324 lines)  AV=2.28  SV=5.23  BV=5.25
    {3644,1200,1024, 0, 0, 0},  //TV = 8.10(347 lines)  AV=2.28  SV=5.23  BV=5.15
    {3896,1200,1024, 0, 0, 0},  //TV = 8.00(371 lines)  AV=2.28  SV=5.23  BV=5.05
    {4179,1200,1024, 0, 0, 0},  //TV = 7.90(398 lines)  AV=2.28  SV=5.23  BV=4.95
    {4505,1200,1024, 0, 0, 0},  //TV = 7.79(429 lines)  AV=2.28  SV=5.23  BV=4.84
    {4820,1200,1024, 0, 0, 0},  //TV = 7.70(459 lines)  AV=2.28  SV=5.23  BV=4.74
    {5156,1200,1024, 0, 0, 0},  //TV = 7.60(491 lines)  AV=2.28  SV=5.23  BV=4.65
    {5523,1200,1024, 0, 0, 0},  //TV = 7.50(526 lines)  AV=2.28  SV=5.23  BV=4.55
    {5912,1200,1024, 0, 0, 0},  //TV = 7.40(563 lines)  AV=2.28  SV=5.23  BV=4.45
    {6332,1200,1024, 0, 0, 0},  //TV = 7.30(603 lines)  AV=2.28  SV=5.23  BV=4.35
    {6836,1200,1024, 0, 0, 0},  //TV = 7.19(651 lines)  AV=2.28  SV=5.23  BV=4.24
    {7319,1200,1024, 0, 0, 0},  //TV = 7.09(697 lines)  AV=2.28  SV=5.23  BV=4.14
    {7833,1200,1024, 0, 0, 0},  //TV = 7.00(746 lines)  AV=2.28  SV=5.23  BV=4.04
    {8338,1200,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.23  BV=3.95
    {8338,1296,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.34  BV=3.84
    {8338,1376,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.43  BV=3.75
    {8338,1488,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.54  BV=3.64
    {8338,1584,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.63  BV=3.55
    {8338,1696,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.73  BV=3.45
    {8338,1824,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.83  BV=3.35
    {8338,1952,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.93  BV=3.25
    {8338,2096,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.03  BV=3.15
    {8338,2240,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.13  BV=3.05
    {16664,1200,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.23  BV=2.95
    {16664,1296,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.34  BV=2.84
    {16664,1376,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.43  BV=2.76
    {16664,1488,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.54  BV=2.64
    {16664,1600,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.64  BV=2.54
    {16664,1696,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.73  BV=2.45
    {25001,1216,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.25  BV=2.35
    {25001,1296,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.34  BV=2.26
    {25001,1408,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.46  BV=2.14
    {25001,1488,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.54  BV=2.06
    {33327,1200,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.23  BV=1.95
    {33327,1296,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.34  BV=1.84
    {33327,1376,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.43  BV=1.76
    {33327,1488,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.54  BV=1.64
    {33327,1584,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.63  BV=1.55
    {33327,1696,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.73  BV=1.45
    {33327,1824,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.83  BV=1.35
    {33327,1952,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.93  BV=1.25
    {33327,2096,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.03  BV=1.15
    {33327,2256,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.14  BV=1.04
    {33327,2416,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.24  BV=0.94
    {33327,2576,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.33  BV=0.85
    {33327,2784,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.44  BV=0.74
    {33327,2976,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.54  BV=0.64
    {33327,3184,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.64  BV=0.55
    {33327,3408,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.73  BV=0.45
    {33327,3648,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.83  BV=0.35
    {33327,3920,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.94  BV=0.25
    {33327,4208,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.04  BV=0.14
    {33327,4496,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.13  BV=0.05
    {33327,4816,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.23  BV=-0.05
    {33327,5152,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.33  BV=-0.15
    {33327,5520,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.43  BV=-0.25
    {33327,5952,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.54  BV=-0.36
    {33327,6384,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.64  BV=-0.46
    {33327,6832,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.74  BV=-0.56
    {33327,7312,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.84  BV=-0.65
    {33327,7840,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.94  BV=-0.75
    {33327,8400,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.04  BV=-0.85
    {33327,8992,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.13  BV=-0.95
    {33327,9632,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.23  BV=-1.05
    {33327,10320,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.33  BV=-1.15
    {33327,11056,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.43  BV=-1.25
    {33327,11840,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.53  BV=-1.35
    {33327,12720,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.63  BV=-1.45
    {33327,13632,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.73  BV=-1.55
    {41664,11680,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=8.51  BV=-1.65
    {41664,12528,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=8.61  BV=-1.75
    {41664,13424,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=8.71  BV=-1.85
    {50001,11984,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=8.55  BV=-1.95
    {50001,12848,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=8.65  BV=-2.05
    {58328,11792,1024, 0, 0, 0},  //TV = 4.10(5555 lines)  AV=2.28  SV=8.53  BV=-2.15
    {58328,12640,1024, 0, 0, 0},  //TV = 4.10(5555 lines)  AV=2.28  SV=8.63  BV=-2.25
    {66665,11856,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=8.53  BV=-2.35
    {66665,12720,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=8.63  BV=-2.45
    {66665,13616,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=8.73  BV=-2.55
    {66665,14592,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=8.83  BV=-2.65
    {66665,15632,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=8.93  BV=-2.75
    {66665,15872,1080, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=9.03  BV=-2.85
    {66665,15872,1152, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=9.12  BV=-2.94
    {66665,15872,1248, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=9.24  BV=-3.06
    {66665,15872,1336, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=9.34  BV=-3.16
    {66665,15872,1432, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=9.44  BV=-3.26
    {66665,15872,1536, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=9.54  BV=-3.36
    {66665,15872,1648, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=9.64  BV=-3.46
    {66665,15872,1760, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=9.74  BV=-3.55
    {66665,15872,1888, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=9.84  BV=-3.65
    {66665,15872,2024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=9.94  BV=-3.76
    {66665,15872,2168, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=10.04  BV=-3.85
    {66665,15872,2328, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=10.14  BV=-3.96
    {66665,15872,2488, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=10.23  BV=-4.05
    {66665,15872,2672, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=10.34  BV=-4.16
    {66665,15872,2864, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=10.44  BV=-4.26
    {66665,15872,3072, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=10.54  BV=-4.36
    {75002,15872,2928, 0, 0, 0},  //TV = 3.74(7143 lines)  AV=2.28  SV=10.47  BV=-4.46
    {83328,15872,2824, 0, 0, 0},  //TV = 3.59(7936 lines)  AV=2.28  SV=10.42  BV=-4.56
    {83328,15872,3024, 0, 0, 0},  //TV = 3.59(7936 lines)  AV=2.28  SV=10.52  BV=-4.66
    {91665,15872,2952, 0, 0, 0},  //TV = 3.45(8730 lines)  AV=2.28  SV=10.48  BV=-4.76
    {99992,15872,2896, 0, 0, 0},  //TV = 3.32(9523 lines)  AV=2.28  SV=10.45  BV=-4.86
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {525,1200,1024, 0, 0, 0},  //TV = 10.90(50 lines)  AV=2.28  SV=5.23  BV=7.94
    {557,1200,1024, 0, 0, 0},  //TV = 10.81(53 lines)  AV=2.28  SV=5.23  BV=7.86
    {599,1200,1024, 0, 0, 0},  //TV = 10.71(57 lines)  AV=2.28  SV=5.23  BV=7.75
    {641,1200,1024, 0, 0, 0},  //TV = 10.61(61 lines)  AV=2.28  SV=5.23  BV=7.65
    {683,1216,1024, 0, 0, 0},  //TV = 10.52(65 lines)  AV=2.28  SV=5.25  BV=7.54
    {735,1200,1024, 0, 0, 0},  //TV = 10.41(70 lines)  AV=2.28  SV=5.23  BV=7.46
    {788,1200,1024, 0, 0, 0},  //TV = 10.31(75 lines)  AV=2.28  SV=5.23  BV=7.36
    {840,1216,1024, 0, 0, 0},  //TV = 10.22(80 lines)  AV=2.28  SV=5.25  BV=7.24
    {903,1200,1024, 0, 0, 0},  //TV = 10.11(86 lines)  AV=2.28  SV=5.23  BV=7.16
    {977,1200,1024, 0, 0, 0},  //TV = 10.00(93 lines)  AV=2.28  SV=5.23  BV=7.05
    {1040,1200,1024, 0, 0, 0},  //TV = 9.91(99 lines)  AV=2.28  SV=5.23  BV=6.96
    {1113,1200,1024, 0, 0, 0},  //TV = 9.81(106 lines)  AV=2.28  SV=5.23  BV=6.86
    {1197,1200,1024, 0, 0, 0},  //TV = 9.71(114 lines)  AV=2.28  SV=5.23  BV=6.75
    {1292,1200,1024, 0, 0, 0},  //TV = 9.60(123 lines)  AV=2.28  SV=5.23  BV=6.64
    {1376,1200,1024, 0, 0, 0},  //TV = 9.51(131 lines)  AV=2.28  SV=5.23  BV=6.55
    {1481,1200,1024, 0, 0, 0},  //TV = 9.40(141 lines)  AV=2.28  SV=5.23  BV=6.45
    {1575,1200,1024, 0, 0, 0},  //TV = 9.31(150 lines)  AV=2.28  SV=5.23  BV=6.36
    {1701,1200,1024, 0, 0, 0},  //TV = 9.20(162 lines)  AV=2.28  SV=5.23  BV=6.25
    {1827,1200,1024, 0, 0, 0},  //TV = 9.10(174 lines)  AV=2.28  SV=5.23  BV=6.14
    {1953,1200,1024, 0, 0, 0},  //TV = 9.00(186 lines)  AV=2.28  SV=5.23  BV=6.05
    {2090,1200,1024, 0, 0, 0},  //TV = 8.90(199 lines)  AV=2.28  SV=5.23  BV=5.95
    {2258,1200,1024, 0, 0, 0},  //TV = 8.79(215 lines)  AV=2.28  SV=5.23  BV=5.84
    {2415,1200,1024, 0, 0, 0},  //TV = 8.69(230 lines)  AV=2.28  SV=5.23  BV=5.74
    {2583,1200,1024, 0, 0, 0},  //TV = 8.60(246 lines)  AV=2.28  SV=5.23  BV=5.64
    {2762,1200,1024, 0, 0, 0},  //TV = 8.50(263 lines)  AV=2.28  SV=5.23  BV=5.55
    {2972,1200,1024, 0, 0, 0},  //TV = 8.39(283 lines)  AV=2.28  SV=5.23  BV=5.44
    {3182,1200,1024, 0, 0, 0},  //TV = 8.30(303 lines)  AV=2.28  SV=5.23  BV=5.34
    {3402,1200,1024, 0, 0, 0},  //TV = 8.20(324 lines)  AV=2.28  SV=5.23  BV=5.25
    {3644,1200,1024, 0, 0, 0},  //TV = 8.10(347 lines)  AV=2.28  SV=5.23  BV=5.15
    {3896,1200,1024, 0, 0, 0},  //TV = 8.00(371 lines)  AV=2.28  SV=5.23  BV=5.05
    {4179,1200,1024, 0, 0, 0},  //TV = 7.90(398 lines)  AV=2.28  SV=5.23  BV=4.95
    {4505,1200,1024, 0, 0, 0},  //TV = 7.79(429 lines)  AV=2.28  SV=5.23  BV=4.84
    {4820,1200,1024, 0, 0, 0},  //TV = 7.70(459 lines)  AV=2.28  SV=5.23  BV=4.74
    {5156,1200,1024, 0, 0, 0},  //TV = 7.60(491 lines)  AV=2.28  SV=5.23  BV=4.65
    {5523,1200,1024, 0, 0, 0},  //TV = 7.50(526 lines)  AV=2.28  SV=5.23  BV=4.55
    {5912,1200,1024, 0, 0, 0},  //TV = 7.40(563 lines)  AV=2.28  SV=5.23  BV=4.45
    {6332,1200,1024, 0, 0, 0},  //TV = 7.30(603 lines)  AV=2.28  SV=5.23  BV=4.35
    {6836,1200,1024, 0, 0, 0},  //TV = 7.19(651 lines)  AV=2.28  SV=5.23  BV=4.24
    {7319,1200,1024, 0, 0, 0},  //TV = 7.09(697 lines)  AV=2.28  SV=5.23  BV=4.14
    {7833,1200,1024, 0, 0, 0},  //TV = 7.00(746 lines)  AV=2.28  SV=5.23  BV=4.04
    {8400,1200,1024, 0, 0, 0},  //TV = 6.90(800 lines)  AV=2.28  SV=5.23  BV=3.94
    {8999,1200,1024, 0, 0, 0},  //TV = 6.80(857 lines)  AV=2.28  SV=5.23  BV=3.84
    {9639,1200,1024, 0, 0, 0},  //TV = 6.70(918 lines)  AV=2.28  SV=5.23  BV=3.74
    {9996,1232,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.27  BV=3.65
    {9996,1328,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.38  BV=3.54
    {9996,1424,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.48  BV=3.44
    {9996,1520,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.57  BV=3.35
    {9996,1632,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.67  BV=3.25
    {9996,1744,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.77  BV=3.15
    {9996,1872,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.87  BV=3.05
    {9996,2000,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.97  BV=2.95
    {9996,2144,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.07  BV=2.85
    {9996,2304,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.17  BV=2.75
    {20003,1232,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.27  BV=2.65
    {20003,1328,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.38  BV=2.54
    {20003,1424,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.48  BV=2.44
    {20003,1520,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.57  BV=2.35
    {20003,1632,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.67  BV=2.25
    {20003,1760,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.78  BV=2.14
    {29999,1248,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.29  BV=2.05
    {29999,1344,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.39  BV=1.94
    {29999,1440,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.49  BV=1.84
    {29999,1536,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.58  BV=1.75
    {29999,1648,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.69  BV=1.65
    {29999,1760,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.78  BV=1.55
    {29999,1888,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.88  BV=1.45
    {29999,2032,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.99  BV=1.35
    {29999,2176,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.09  BV=1.25
    {29999,2320,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.18  BV=1.15
    {29999,2496,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.29  BV=1.05
    {29999,2672,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.38  BV=0.95
    {29999,2864,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.48  BV=0.85
    {29999,3072,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.58  BV=0.75
    {29999,3296,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.69  BV=0.65
    {29999,3536,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.79  BV=0.55
    {29999,3776,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.88  BV=0.45
    {29999,4048,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.98  BV=0.35
    {29999,4368,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.09  BV=0.24
    {29999,4672,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.19  BV=0.14
    {29999,5008,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.29  BV=0.04
    {29999,5360,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.39  BV=-0.05
    {29999,5728,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.48  BV=-0.15
    {29999,6144,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.58  BV=-0.25
    {29999,6624,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.69  BV=-0.36
    {29999,7088,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.79  BV=-0.46
    {29999,7584,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.89  BV=-0.55
    {29999,8128,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.99  BV=-0.65
    {29999,8704,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.09  BV=-0.75
    {29999,9328,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.19  BV=-0.85
    {29999,9984,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.29  BV=-0.95
    {29999,10704,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.39  BV=-1.05
    {29999,11472,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.49  BV=-1.15
    {29999,12288,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.58  BV=-1.25
    {29999,13248,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.69  BV=-1.36
    {29999,14128,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.79  BV=-1.45
    {40005,11360,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=8.47  BV=-1.55
    {40005,12160,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=8.57  BV=-1.65
    {40005,13040,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=8.67  BV=-1.75
    {50001,11184,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=8.45  BV=-1.85
    {50001,11984,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=8.55  BV=-1.95
    {50001,12848,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=8.65  BV=-2.05
    {59997,11472,1024, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=8.49  BV=-2.15
    {59997,12288,1024, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=8.58  BV=-2.25
    {59997,13168,1024, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=8.68  BV=-2.35
    {70004,12112,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=8.56  BV=-2.45
    {70004,12976,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=8.66  BV=-2.55
    {70004,13904,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=8.76  BV=-2.65
    {70004,14896,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=8.86  BV=-2.75
    {70004,15872,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=8.95  BV=-2.84
    {70004,15872,1096, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=9.05  BV=-2.94
    {70004,15872,1176, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=9.15  BV=-3.04
    {70004,15872,1272, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=9.27  BV=-3.16
    {70004,15872,1360, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=9.36  BV=-3.25
    {70004,15872,1464, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=9.47  BV=-3.36
    {70004,15872,1568, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=9.57  BV=-3.46
    {70004,15872,1680, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=9.67  BV=-3.56
    {70004,15872,1800, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=9.77  BV=-3.66
    {70004,15872,1928, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=9.87  BV=-3.76
    {70004,15872,2064, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=9.97  BV=-3.85
    {70004,15872,2216, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=10.07  BV=-3.96
    {70004,15872,2376, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=10.17  BV=-4.06
    {70004,15872,2544, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=10.27  BV=-4.16
    {70004,15872,2728, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=10.37  BV=-4.26
    {70004,15872,2928, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=10.47  BV=-4.36
    {70004,15872,3136, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=10.57  BV=-4.46
    {80000,15872,2944, 0, 0, 0},  //TV = 3.64(7619 lines)  AV=2.28  SV=10.48  BV=-4.56
    {80000,15872,3152, 0, 0, 0},  //TV = 3.64(7619 lines)  AV=2.28  SV=10.58  BV=-4.66
    {89996,15872,3000, 0, 0, 0},  //TV = 3.47(8571 lines)  AV=2.28  SV=10.50  BV=-4.76
    {100002,15872,2896, 0, 0, 0},  //TV = 3.32(9524 lines)  AV=2.28  SV=10.45  BV=-4.86
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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
    153, //u4TotalIndex
    20, //i4StrobeTrigerBV
    104, //i4MaxBV
    -49, //i4MinBV
    90, //i4EffectiveMaxBV
    -50, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_AUTO, //ISO
    sAEScene1PLineTable_60Hz,
    sAEScene1PLineTable_50Hz,
    NULL,
};
static strEvPline sAEScene2PLineTable_60Hz =
{
{
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {525,1200,1024, 0, 0, 0},  //TV = 10.90(50 lines)  AV=2.28  SV=5.23  BV=7.94
    {557,1200,1024, 0, 0, 0},  //TV = 10.81(53 lines)  AV=2.28  SV=5.23  BV=7.86
    {599,1200,1024, 0, 0, 0},  //TV = 10.71(57 lines)  AV=2.28  SV=5.23  BV=7.75
    {641,1200,1024, 0, 0, 0},  //TV = 10.61(61 lines)  AV=2.28  SV=5.23  BV=7.65
    {683,1216,1024, 0, 0, 0},  //TV = 10.52(65 lines)  AV=2.28  SV=5.25  BV=7.54
    {735,1200,1024, 0, 0, 0},  //TV = 10.41(70 lines)  AV=2.28  SV=5.23  BV=7.46
    {788,1200,1024, 0, 0, 0},  //TV = 10.31(75 lines)  AV=2.28  SV=5.23  BV=7.36
    {840,1216,1024, 0, 0, 0},  //TV = 10.22(80 lines)  AV=2.28  SV=5.25  BV=7.24
    {903,1200,1024, 0, 0, 0},  //TV = 10.11(86 lines)  AV=2.28  SV=5.23  BV=7.16
    {977,1200,1024, 0, 0, 0},  //TV = 10.00(93 lines)  AV=2.28  SV=5.23  BV=7.05
    {1040,1200,1024, 0, 0, 0},  //TV = 9.91(99 lines)  AV=2.28  SV=5.23  BV=6.96
    {1113,1200,1024, 0, 0, 0},  //TV = 9.81(106 lines)  AV=2.28  SV=5.23  BV=6.86
    {1197,1200,1024, 0, 0, 0},  //TV = 9.71(114 lines)  AV=2.28  SV=5.23  BV=6.75
    {1292,1200,1024, 0, 0, 0},  //TV = 9.60(123 lines)  AV=2.28  SV=5.23  BV=6.64
    {1376,1200,1024, 0, 0, 0},  //TV = 9.51(131 lines)  AV=2.28  SV=5.23  BV=6.55
    {1481,1200,1024, 0, 0, 0},  //TV = 9.40(141 lines)  AV=2.28  SV=5.23  BV=6.45
    {1575,1200,1024, 0, 0, 0},  //TV = 9.31(150 lines)  AV=2.28  SV=5.23  BV=6.36
    {1701,1200,1024, 0, 0, 0},  //TV = 9.20(162 lines)  AV=2.28  SV=5.23  BV=6.25
    {1827,1200,1024, 0, 0, 0},  //TV = 9.10(174 lines)  AV=2.28  SV=5.23  BV=6.14
    {1953,1200,1024, 0, 0, 0},  //TV = 9.00(186 lines)  AV=2.28  SV=5.23  BV=6.05
    {2090,1200,1024, 0, 0, 0},  //TV = 8.90(199 lines)  AV=2.28  SV=5.23  BV=5.95
    {2258,1200,1024, 0, 0, 0},  //TV = 8.79(215 lines)  AV=2.28  SV=5.23  BV=5.84
    {2415,1200,1024, 0, 0, 0},  //TV = 8.69(230 lines)  AV=2.28  SV=5.23  BV=5.74
    {2583,1200,1024, 0, 0, 0},  //TV = 8.60(246 lines)  AV=2.28  SV=5.23  BV=5.64
    {2762,1200,1024, 0, 0, 0},  //TV = 8.50(263 lines)  AV=2.28  SV=5.23  BV=5.55
    {2972,1200,1024, 0, 0, 0},  //TV = 8.39(283 lines)  AV=2.28  SV=5.23  BV=5.44
    {3182,1200,1024, 0, 0, 0},  //TV = 8.30(303 lines)  AV=2.28  SV=5.23  BV=5.34
    {3402,1200,1024, 0, 0, 0},  //TV = 8.20(324 lines)  AV=2.28  SV=5.23  BV=5.25
    {3644,1200,1024, 0, 0, 0},  //TV = 8.10(347 lines)  AV=2.28  SV=5.23  BV=5.15
    {3896,1200,1024, 0, 0, 0},  //TV = 8.00(371 lines)  AV=2.28  SV=5.23  BV=5.05
    {4179,1200,1024, 0, 0, 0},  //TV = 7.90(398 lines)  AV=2.28  SV=5.23  BV=4.95
    {4505,1200,1024, 0, 0, 0},  //TV = 7.79(429 lines)  AV=2.28  SV=5.23  BV=4.84
    {4820,1200,1024, 0, 0, 0},  //TV = 7.70(459 lines)  AV=2.28  SV=5.23  BV=4.74
    {5156,1200,1024, 0, 0, 0},  //TV = 7.60(491 lines)  AV=2.28  SV=5.23  BV=4.65
    {5523,1200,1024, 0, 0, 0},  //TV = 7.50(526 lines)  AV=2.28  SV=5.23  BV=4.55
    {5912,1200,1024, 0, 0, 0},  //TV = 7.40(563 lines)  AV=2.28  SV=5.23  BV=4.45
    {6332,1200,1024, 0, 0, 0},  //TV = 7.30(603 lines)  AV=2.28  SV=5.23  BV=4.35
    {6836,1200,1024, 0, 0, 0},  //TV = 7.19(651 lines)  AV=2.28  SV=5.23  BV=4.24
    {7319,1200,1024, 0, 0, 0},  //TV = 7.09(697 lines)  AV=2.28  SV=5.23  BV=4.14
    {7833,1200,1024, 0, 0, 0},  //TV = 7.00(746 lines)  AV=2.28  SV=5.23  BV=4.04
    {8338,1200,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.23  BV=3.95
    {8338,1296,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.34  BV=3.84
    {8338,1376,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.43  BV=3.75
    {8338,1472,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.52  BV=3.66
    {8338,1584,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.63  BV=3.55
    {8338,1712,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.74  BV=3.44
    {8338,1824,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.83  BV=3.35
    {8338,1968,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.94  BV=3.24
    {8338,2096,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.03  BV=3.15
    {8338,2256,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.14  BV=3.04
    {8338,2416,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.24  BV=2.94
    {8338,2576,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.33  BV=2.85
    {8338,2784,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.44  BV=2.74
    {8338,2976,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.54  BV=2.64
    {8338,3184,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.64  BV=2.54
    {8338,3392,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.73  BV=2.45
    {8338,3664,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.84  BV=2.34
    {8338,3920,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.94  BV=2.24
    {8338,4192,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=7.03  BV=2.15
    {8338,4496,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=7.13  BV=2.05
    {8338,4800,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=7.23  BV=1.95
    {8337,5184,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=7.34  BV=1.84
    {8337,5552,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=7.44  BV=1.74
    {8337,5920,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=7.53  BV=1.65
    {16664,3184,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.64  BV=1.55
    {16664,3424,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.74  BV=1.44
    {16664,3664,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.84  BV=1.34
    {16664,3920,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.94  BV=1.25
    {16664,4208,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=7.04  BV=1.14
    {16664,4512,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=7.14  BV=1.04
    {16664,4832,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=7.24  BV=0.94
    {25001,3440,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=6.75  BV=0.85
    {25001,3696,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=6.85  BV=0.75
    {25001,3952,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=6.95  BV=0.65
    {25001,4240,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=7.05  BV=0.55
    {25001,4544,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=7.15  BV=0.45
    {33327,3648,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.83  BV=0.35
    {33327,3920,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.94  BV=0.25
    {33327,4208,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.04  BV=0.14
    {33327,4496,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.13  BV=0.05
    {33327,4816,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.23  BV=-0.05
    {33327,5168,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.34  BV=-0.15
    {33327,5520,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.43  BV=-0.25
    {33327,5952,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.54  BV=-0.36
    {33327,6384,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.64  BV=-0.46
    {33327,6832,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.74  BV=-0.56
    {33327,7328,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.84  BV=-0.66
    {33327,7840,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.94  BV=-0.75
    {33327,8400,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.04  BV=-0.85
    {33327,8992,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.13  BV=-0.95
    {33327,9632,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.23  BV=-1.05
    {33327,10320,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.33  BV=-1.15
    {33327,11056,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.43  BV=-1.25
    {33327,11840,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.53  BV=-1.35
    {33327,12784,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.64  BV=-1.46
    {33327,13696,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.74  BV=-1.56
    {33327,14672,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.84  BV=-1.66
    {33327,15712,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.94  BV=-1.76
    {33327,15872,1056, 0, 0, 0},  /* TV = 4.91(3174 lines)  AV=2.28  SV=9.00  BV=-1.82 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {525,1200,1024, 0, 0, 0},  //TV = 10.90(50 lines)  AV=2.28  SV=5.23  BV=7.94
    {557,1200,1024, 0, 0, 0},  //TV = 10.81(53 lines)  AV=2.28  SV=5.23  BV=7.86
    {599,1200,1024, 0, 0, 0},  //TV = 10.71(57 lines)  AV=2.28  SV=5.23  BV=7.75
    {641,1200,1024, 0, 0, 0},  //TV = 10.61(61 lines)  AV=2.28  SV=5.23  BV=7.65
    {683,1216,1024, 0, 0, 0},  //TV = 10.52(65 lines)  AV=2.28  SV=5.25  BV=7.54
    {735,1200,1024, 0, 0, 0},  //TV = 10.41(70 lines)  AV=2.28  SV=5.23  BV=7.46
    {788,1200,1024, 0, 0, 0},  //TV = 10.31(75 lines)  AV=2.28  SV=5.23  BV=7.36
    {840,1216,1024, 0, 0, 0},  //TV = 10.22(80 lines)  AV=2.28  SV=5.25  BV=7.24
    {903,1200,1024, 0, 0, 0},  //TV = 10.11(86 lines)  AV=2.28  SV=5.23  BV=7.16
    {977,1200,1024, 0, 0, 0},  //TV = 10.00(93 lines)  AV=2.28  SV=5.23  BV=7.05
    {1040,1200,1024, 0, 0, 0},  //TV = 9.91(99 lines)  AV=2.28  SV=5.23  BV=6.96
    {1113,1200,1024, 0, 0, 0},  //TV = 9.81(106 lines)  AV=2.28  SV=5.23  BV=6.86
    {1197,1200,1024, 0, 0, 0},  //TV = 9.71(114 lines)  AV=2.28  SV=5.23  BV=6.75
    {1292,1200,1024, 0, 0, 0},  //TV = 9.60(123 lines)  AV=2.28  SV=5.23  BV=6.64
    {1376,1200,1024, 0, 0, 0},  //TV = 9.51(131 lines)  AV=2.28  SV=5.23  BV=6.55
    {1481,1200,1024, 0, 0, 0},  //TV = 9.40(141 lines)  AV=2.28  SV=5.23  BV=6.45
    {1575,1200,1024, 0, 0, 0},  //TV = 9.31(150 lines)  AV=2.28  SV=5.23  BV=6.36
    {1701,1200,1024, 0, 0, 0},  //TV = 9.20(162 lines)  AV=2.28  SV=5.23  BV=6.25
    {1827,1200,1024, 0, 0, 0},  //TV = 9.10(174 lines)  AV=2.28  SV=5.23  BV=6.14
    {1953,1200,1024, 0, 0, 0},  //TV = 9.00(186 lines)  AV=2.28  SV=5.23  BV=6.05
    {2090,1200,1024, 0, 0, 0},  //TV = 8.90(199 lines)  AV=2.28  SV=5.23  BV=5.95
    {2258,1200,1024, 0, 0, 0},  //TV = 8.79(215 lines)  AV=2.28  SV=5.23  BV=5.84
    {2415,1200,1024, 0, 0, 0},  //TV = 8.69(230 lines)  AV=2.28  SV=5.23  BV=5.74
    {2583,1200,1024, 0, 0, 0},  //TV = 8.60(246 lines)  AV=2.28  SV=5.23  BV=5.64
    {2762,1200,1024, 0, 0, 0},  //TV = 8.50(263 lines)  AV=2.28  SV=5.23  BV=5.55
    {2972,1200,1024, 0, 0, 0},  //TV = 8.39(283 lines)  AV=2.28  SV=5.23  BV=5.44
    {3182,1200,1024, 0, 0, 0},  //TV = 8.30(303 lines)  AV=2.28  SV=5.23  BV=5.34
    {3402,1200,1024, 0, 0, 0},  //TV = 8.20(324 lines)  AV=2.28  SV=5.23  BV=5.25
    {3644,1200,1024, 0, 0, 0},  //TV = 8.10(347 lines)  AV=2.28  SV=5.23  BV=5.15
    {3896,1200,1024, 0, 0, 0},  //TV = 8.00(371 lines)  AV=2.28  SV=5.23  BV=5.05
    {4179,1200,1024, 0, 0, 0},  //TV = 7.90(398 lines)  AV=2.28  SV=5.23  BV=4.95
    {4505,1200,1024, 0, 0, 0},  //TV = 7.79(429 lines)  AV=2.28  SV=5.23  BV=4.84
    {4820,1200,1024, 0, 0, 0},  //TV = 7.70(459 lines)  AV=2.28  SV=5.23  BV=4.74
    {5156,1200,1024, 0, 0, 0},  //TV = 7.60(491 lines)  AV=2.28  SV=5.23  BV=4.65
    {5523,1200,1024, 0, 0, 0},  //TV = 7.50(526 lines)  AV=2.28  SV=5.23  BV=4.55
    {5912,1200,1024, 0, 0, 0},  //TV = 7.40(563 lines)  AV=2.28  SV=5.23  BV=4.45
    {6332,1200,1024, 0, 0, 0},  //TV = 7.30(603 lines)  AV=2.28  SV=5.23  BV=4.35
    {6836,1200,1024, 0, 0, 0},  //TV = 7.19(651 lines)  AV=2.28  SV=5.23  BV=4.24
    {7319,1200,1024, 0, 0, 0},  //TV = 7.09(697 lines)  AV=2.28  SV=5.23  BV=4.14
    {7833,1200,1024, 0, 0, 0},  //TV = 7.00(746 lines)  AV=2.28  SV=5.23  BV=4.04
    {8400,1200,1024, 0, 0, 0},  //TV = 6.90(800 lines)  AV=2.28  SV=5.23  BV=3.94
    {8999,1200,1024, 0, 0, 0},  //TV = 6.80(857 lines)  AV=2.28  SV=5.23  BV=3.84
    {9639,1200,1024, 0, 0, 0},  //TV = 6.70(918 lines)  AV=2.28  SV=5.23  BV=3.74
    {9996,1232,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.27  BV=3.65
    {9996,1328,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.38  BV=3.54
    {9996,1424,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.48  BV=3.44
    {9996,1520,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.57  BV=3.35
    {9996,1632,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.67  BV=3.25
    {9996,1744,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.77  BV=3.15
    {9996,1872,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.87  BV=3.05
    {9996,2016,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.98  BV=2.94
    {9996,2144,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.07  BV=2.85
    {9996,2304,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.17  BV=2.75
    {9996,2480,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.28  BV=2.64
    {9996,2656,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.38  BV=2.54
    {9996,2832,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.47  BV=2.45
    {9996,3040,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.57  BV=2.35
    {9996,3264,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.67  BV=2.25
    {9996,3504,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.77  BV=2.14
    {9996,3744,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.87  BV=2.05
    {9996,4016,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.97  BV=1.95
    {9996,4304,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=7.07  BV=1.85
    {9996,4608,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=7.17  BV=1.75
    {9996,4960,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=7.28  BV=1.64
    {9996,5328,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=7.38  BV=1.54
    {9996,5696,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=7.48  BV=1.44
    {9996,6112,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=7.58  BV=1.34
    {20003,3264,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.67  BV=1.25
    {20003,3504,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.77  BV=1.14
    {20003,3760,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.88  BV=1.04
    {20003,4016,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.97  BV=0.95
    {20003,4304,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=7.07  BV=0.85
    {20003,4624,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=7.17  BV=0.74
    {20003,4944,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=7.27  BV=0.65
    {29999,3536,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.79  BV=0.55
    {29999,3792,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.89  BV=0.45
    {29999,4064,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.99  BV=0.35
    {29999,4352,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.09  BV=0.25
    {29999,4672,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.19  BV=0.14
    {29999,5008,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.29  BV=0.04
    {29999,5360,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.39  BV=-0.05
    {29999,5728,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.48  BV=-0.15
    {29999,6144,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.58  BV=-0.25
    {29999,6624,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.69  BV=-0.36
    {29999,7088,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.79  BV=-0.46
    {29999,7600,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.89  BV=-0.56
    {29999,8128,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.99  BV=-0.65
    {29999,8704,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.09  BV=-0.75
    {29999,9328,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.19  BV=-0.85
    {29999,10000,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.29  BV=-0.95
    {29999,10704,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.39  BV=-1.05
    {29999,11472,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.49  BV=-1.15
    {29999,12288,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.58  BV=-1.25
    {29999,13168,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.68  BV=-1.35
    {29999,14192,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.79  BV=-1.46
    {29999,15216,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.89  BV=-1.56
    {29999,15872,1048, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.99  BV=-1.65
    {29999,15872,1056, 0, 0, 0},  /* TV = 5.06(2857 lines)  AV=2.28  SV=9.00  BV=-1.66 */
    {29999,15872,1056, 0, 0, 0},  /* TV = 5.06(2857 lines)  AV=2.28  SV=9.00  BV=-1.66 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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
    123, //u4TotalIndex
    20, //i4StrobeTrigerBV
    104, //i4MaxBV
    -19, //i4MinBV
    90, //i4EffectiveMaxBV
    -30, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_AUTO, //ISO
    sAEScene2PLineTable_60Hz,
    sAEScene2PLineTable_50Hz,
    NULL,
};
static strEvPline sAEScene3PLineTable_60Hz =
{
{
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {525,1200,1024, 0, 0, 0},  //TV = 10.90(50 lines)  AV=2.28  SV=5.23  BV=7.94
    {557,1200,1024, 0, 0, 0},  //TV = 10.81(53 lines)  AV=2.28  SV=5.23  BV=7.86
    {599,1200,1024, 0, 0, 0},  //TV = 10.71(57 lines)  AV=2.28  SV=5.23  BV=7.75
    {641,1200,1024, 0, 0, 0},  //TV = 10.61(61 lines)  AV=2.28  SV=5.23  BV=7.65
    {683,1216,1024, 0, 0, 0},  //TV = 10.52(65 lines)  AV=2.28  SV=5.25  BV=7.54
    {735,1200,1024, 0, 0, 0},  //TV = 10.41(70 lines)  AV=2.28  SV=5.23  BV=7.46
    {788,1200,1024, 0, 0, 0},  //TV = 10.31(75 lines)  AV=2.28  SV=5.23  BV=7.36
    {840,1216,1024, 0, 0, 0},  //TV = 10.22(80 lines)  AV=2.28  SV=5.25  BV=7.24
    {903,1200,1024, 0, 0, 0},  //TV = 10.11(86 lines)  AV=2.28  SV=5.23  BV=7.16
    {977,1200,1024, 0, 0, 0},  //TV = 10.00(93 lines)  AV=2.28  SV=5.23  BV=7.05
    {1040,1200,1024, 0, 0, 0},  //TV = 9.91(99 lines)  AV=2.28  SV=5.23  BV=6.96
    {1113,1200,1024, 0, 0, 0},  //TV = 9.81(106 lines)  AV=2.28  SV=5.23  BV=6.86
    {1197,1200,1024, 0, 0, 0},  //TV = 9.71(114 lines)  AV=2.28  SV=5.23  BV=6.75
    {1292,1200,1024, 0, 0, 0},  //TV = 9.60(123 lines)  AV=2.28  SV=5.23  BV=6.64
    {1376,1200,1024, 0, 0, 0},  //TV = 9.51(131 lines)  AV=2.28  SV=5.23  BV=6.55
    {1481,1200,1024, 0, 0, 0},  //TV = 9.40(141 lines)  AV=2.28  SV=5.23  BV=6.45
    {1575,1200,1024, 0, 0, 0},  //TV = 9.31(150 lines)  AV=2.28  SV=5.23  BV=6.36
    {1701,1200,1024, 0, 0, 0},  //TV = 9.20(162 lines)  AV=2.28  SV=5.23  BV=6.25
    {1827,1200,1024, 0, 0, 0},  //TV = 9.10(174 lines)  AV=2.28  SV=5.23  BV=6.14
    {1953,1200,1024, 0, 0, 0},  //TV = 9.00(186 lines)  AV=2.28  SV=5.23  BV=6.05
    {2090,1200,1024, 0, 0, 0},  //TV = 8.90(199 lines)  AV=2.28  SV=5.23  BV=5.95
    {2258,1200,1024, 0, 0, 0},  //TV = 8.79(215 lines)  AV=2.28  SV=5.23  BV=5.84
    {2415,1200,1024, 0, 0, 0},  //TV = 8.69(230 lines)  AV=2.28  SV=5.23  BV=5.74
    {2583,1200,1024, 0, 0, 0},  //TV = 8.60(246 lines)  AV=2.28  SV=5.23  BV=5.64
    {2762,1200,1024, 0, 0, 0},  //TV = 8.50(263 lines)  AV=2.28  SV=5.23  BV=5.55
    {2972,1200,1024, 0, 0, 0},  //TV = 8.39(283 lines)  AV=2.28  SV=5.23  BV=5.44
    {3182,1200,1024, 0, 0, 0},  //TV = 8.30(303 lines)  AV=2.28  SV=5.23  BV=5.34
    {3402,1200,1024, 0, 0, 0},  //TV = 8.20(324 lines)  AV=2.28  SV=5.23  BV=5.25
    {3644,1200,1024, 0, 0, 0},  //TV = 8.10(347 lines)  AV=2.28  SV=5.23  BV=5.15
    {3896,1200,1024, 0, 0, 0},  //TV = 8.00(371 lines)  AV=2.28  SV=5.23  BV=5.05
    {4179,1200,1024, 0, 0, 0},  //TV = 7.90(398 lines)  AV=2.28  SV=5.23  BV=4.95
    {4505,1200,1024, 0, 0, 0},  //TV = 7.79(429 lines)  AV=2.28  SV=5.23  BV=4.84
    {4820,1200,1024, 0, 0, 0},  //TV = 7.70(459 lines)  AV=2.28  SV=5.23  BV=4.74
    {5156,1200,1024, 0, 0, 0},  //TV = 7.60(491 lines)  AV=2.28  SV=5.23  BV=4.65
    {5523,1200,1024, 0, 0, 0},  //TV = 7.50(526 lines)  AV=2.28  SV=5.23  BV=4.55
    {5912,1200,1024, 0, 0, 0},  //TV = 7.40(563 lines)  AV=2.28  SV=5.23  BV=4.45
    {6332,1200,1024, 0, 0, 0},  //TV = 7.30(603 lines)  AV=2.28  SV=5.23  BV=4.35
    {6836,1200,1024, 0, 0, 0},  //TV = 7.19(651 lines)  AV=2.28  SV=5.23  BV=4.24
    {7319,1200,1024, 0, 0, 0},  //TV = 7.09(697 lines)  AV=2.28  SV=5.23  BV=4.14
    {7833,1200,1024, 0, 0, 0},  //TV = 7.00(746 lines)  AV=2.28  SV=5.23  BV=4.04
    {8338,1200,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.23  BV=3.95
    {8338,1296,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.34  BV=3.84
    {8338,1376,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.43  BV=3.75
    {8338,1488,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.54  BV=3.64
    {8338,1584,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.63  BV=3.55
    {8338,1696,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.73  BV=3.45
    {8338,1824,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.83  BV=3.35
    {8338,1952,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.93  BV=3.25
    {8338,2096,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.03  BV=3.15
    {8338,2240,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.13  BV=3.05
    {16664,1200,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.23  BV=2.95
    {16664,1296,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.34  BV=2.84
    {16664,1376,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.43  BV=2.76
    {16664,1488,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.54  BV=2.64
    {16664,1600,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.64  BV=2.54
    {16664,1696,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.73  BV=2.45
    {25001,1216,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.25  BV=2.35
    {25001,1296,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.34  BV=2.26
    {25001,1408,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.46  BV=2.14
    {25001,1488,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.54  BV=2.06
    {33327,1200,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.23  BV=1.95
    {33327,1296,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.34  BV=1.84
    {33327,1376,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.43  BV=1.76
    {33327,1488,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.54  BV=1.64
    {33327,1584,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.63  BV=1.55
    {33327,1696,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.73  BV=1.45
    {33327,1824,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.83  BV=1.35
    {33327,1952,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.93  BV=1.25
    {33327,2096,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.03  BV=1.15
    {33327,2256,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.14  BV=1.04
    {41664,1936,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=5.92  BV=0.94
    {41664,2064,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=6.01  BV=0.85
    {41664,2224,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=6.12  BV=0.74
    {50001,1984,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=5.95  BV=0.64
    {50001,2128,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=6.06  BV=0.54
    {58328,1952,1024, 0, 0, 0},  //TV = 4.10(5555 lines)  AV=2.28  SV=5.93  BV=0.44
    {58328,2096,1024, 0, 0, 0},  //TV = 4.10(5555 lines)  AV=2.28  SV=6.03  BV=0.34
    {66665,1952,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=5.93  BV=0.25
    {66665,2096,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=6.03  BV=0.15
    {66665,2240,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=6.13  BV=0.05
    {66665,2400,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=6.23  BV=-0.05
    {66665,2592,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=6.34  BV=-0.16
    {66665,2768,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=6.43  BV=-0.25
    {66665,2960,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=6.53  BV=-0.35
    {66665,3168,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=6.63  BV=-0.45
    {66665,3408,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=6.73  BV=-0.55
    {66665,3648,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=6.83  BV=-0.65
    {66665,3920,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=6.94  BV=-0.75
    {66665,4208,1024, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=7.04  BV=-0.86
    {75002,4000,1024, 0, 0, 0},  //TV = 3.74(7143 lines)  AV=2.28  SV=6.97  BV=-0.95
    {75002,4288,1024, 0, 0, 0},  //TV = 3.74(7143 lines)  AV=2.28  SV=7.07  BV=-1.05
    {83328,4144,1024, 0, 0, 0},  //TV = 3.59(7936 lines)  AV=2.28  SV=7.02  BV=-1.16
    {91665,4032,1024, 0, 0, 0},  //TV = 3.45(8730 lines)  AV=2.28  SV=6.98  BV=-1.25
    {99992,3968,1024, 0, 0, 0},  //TV = 3.32(9523 lines)  AV=2.28  SV=6.95  BV=-1.36
    {99992,4240,1024, 0, 0, 0},  //TV = 3.32(9523 lines)  AV=2.28  SV=7.05  BV=-1.45
    {99992,4528,1024, 0, 0, 0},  //TV = 3.32(9523 lines)  AV=2.28  SV=7.14  BV=-1.55
    {99992,4880,1024, 0, 0, 0},  //TV = 3.32(9523 lines)  AV=2.28  SV=7.25  BV=-1.66
    {99992,5232,1024, 0, 0, 0},  //TV = 3.32(9523 lines)  AV=2.28  SV=7.35  BV=-1.76
    {99992,5600,1024, 0, 0, 0},  //TV = 3.32(9523 lines)  AV=2.28  SV=7.45  BV=-1.85
    {99992,6000,1024, 0, 0, 0},  //TV = 3.32(9523 lines)  AV=2.28  SV=7.55  BV=-1.95
    {99992,6432,1024, 0, 0, 0},  //TV = 3.32(9523 lines)  AV=2.28  SV=7.65  BV=-2.05
    {99992,6880,1024, 0, 0, 0},  //TV = 3.32(9523 lines)  AV=2.28  SV=7.75  BV=-2.15
    {99992,7376,1024, 0, 0, 0},  //TV = 3.32(9523 lines)  AV=2.28  SV=7.85  BV=-2.25
    {99992,7952,1024, 0, 0, 0},  //TV = 3.32(9523 lines)  AV=2.28  SV=7.96  BV=-2.36
    {99992,8512,1024, 0, 0, 0},  //TV = 3.32(9523 lines)  AV=2.28  SV=8.06  BV=-2.46
    {99992,9120,1024, 0, 0, 0},  //TV = 3.32(9523 lines)  AV=2.28  SV=8.15  BV=-2.56
    {99992,9776,1024, 0, 0, 0},  //TV = 3.32(9523 lines)  AV=2.28  SV=8.26  BV=-2.66
    {99992,10464,1024, 0, 0, 0},  //TV = 3.32(9523 lines)  AV=2.28  SV=8.35  BV=-2.76
    {99992,11216,1024, 0, 0, 0},  //TV = 3.32(9523 lines)  AV=2.28  SV=8.45  BV=-2.86
    {99992,12016,1024, 0, 0, 0},  //TV = 3.32(9523 lines)  AV=2.28  SV=8.55  BV=-2.96
    {99992,12288,1024, 0, 0, 0},  /* TV = 3.32(9523 lines)  AV=2.28  SV=8.58  BV=-2.99 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {525,1200,1024, 0, 0, 0},  //TV = 10.90(50 lines)  AV=2.28  SV=5.23  BV=7.94
    {557,1200,1024, 0, 0, 0},  //TV = 10.81(53 lines)  AV=2.28  SV=5.23  BV=7.86
    {599,1200,1024, 0, 0, 0},  //TV = 10.71(57 lines)  AV=2.28  SV=5.23  BV=7.75
    {641,1200,1024, 0, 0, 0},  //TV = 10.61(61 lines)  AV=2.28  SV=5.23  BV=7.65
    {683,1216,1024, 0, 0, 0},  //TV = 10.52(65 lines)  AV=2.28  SV=5.25  BV=7.54
    {735,1200,1024, 0, 0, 0},  //TV = 10.41(70 lines)  AV=2.28  SV=5.23  BV=7.46
    {788,1200,1024, 0, 0, 0},  //TV = 10.31(75 lines)  AV=2.28  SV=5.23  BV=7.36
    {840,1216,1024, 0, 0, 0},  //TV = 10.22(80 lines)  AV=2.28  SV=5.25  BV=7.24
    {903,1200,1024, 0, 0, 0},  //TV = 10.11(86 lines)  AV=2.28  SV=5.23  BV=7.16
    {977,1200,1024, 0, 0, 0},  //TV = 10.00(93 lines)  AV=2.28  SV=5.23  BV=7.05
    {1040,1200,1024, 0, 0, 0},  //TV = 9.91(99 lines)  AV=2.28  SV=5.23  BV=6.96
    {1113,1200,1024, 0, 0, 0},  //TV = 9.81(106 lines)  AV=2.28  SV=5.23  BV=6.86
    {1197,1200,1024, 0, 0, 0},  //TV = 9.71(114 lines)  AV=2.28  SV=5.23  BV=6.75
    {1292,1200,1024, 0, 0, 0},  //TV = 9.60(123 lines)  AV=2.28  SV=5.23  BV=6.64
    {1376,1200,1024, 0, 0, 0},  //TV = 9.51(131 lines)  AV=2.28  SV=5.23  BV=6.55
    {1481,1200,1024, 0, 0, 0},  //TV = 9.40(141 lines)  AV=2.28  SV=5.23  BV=6.45
    {1575,1200,1024, 0, 0, 0},  //TV = 9.31(150 lines)  AV=2.28  SV=5.23  BV=6.36
    {1701,1200,1024, 0, 0, 0},  //TV = 9.20(162 lines)  AV=2.28  SV=5.23  BV=6.25
    {1827,1200,1024, 0, 0, 0},  //TV = 9.10(174 lines)  AV=2.28  SV=5.23  BV=6.14
    {1953,1200,1024, 0, 0, 0},  //TV = 9.00(186 lines)  AV=2.28  SV=5.23  BV=6.05
    {2090,1200,1024, 0, 0, 0},  //TV = 8.90(199 lines)  AV=2.28  SV=5.23  BV=5.95
    {2258,1200,1024, 0, 0, 0},  //TV = 8.79(215 lines)  AV=2.28  SV=5.23  BV=5.84
    {2415,1200,1024, 0, 0, 0},  //TV = 8.69(230 lines)  AV=2.28  SV=5.23  BV=5.74
    {2583,1200,1024, 0, 0, 0},  //TV = 8.60(246 lines)  AV=2.28  SV=5.23  BV=5.64
    {2762,1200,1024, 0, 0, 0},  //TV = 8.50(263 lines)  AV=2.28  SV=5.23  BV=5.55
    {2972,1200,1024, 0, 0, 0},  //TV = 8.39(283 lines)  AV=2.28  SV=5.23  BV=5.44
    {3182,1200,1024, 0, 0, 0},  //TV = 8.30(303 lines)  AV=2.28  SV=5.23  BV=5.34
    {3402,1200,1024, 0, 0, 0},  //TV = 8.20(324 lines)  AV=2.28  SV=5.23  BV=5.25
    {3644,1200,1024, 0, 0, 0},  //TV = 8.10(347 lines)  AV=2.28  SV=5.23  BV=5.15
    {3896,1200,1024, 0, 0, 0},  //TV = 8.00(371 lines)  AV=2.28  SV=5.23  BV=5.05
    {4179,1200,1024, 0, 0, 0},  //TV = 7.90(398 lines)  AV=2.28  SV=5.23  BV=4.95
    {4505,1200,1024, 0, 0, 0},  //TV = 7.79(429 lines)  AV=2.28  SV=5.23  BV=4.84
    {4820,1200,1024, 0, 0, 0},  //TV = 7.70(459 lines)  AV=2.28  SV=5.23  BV=4.74
    {5156,1200,1024, 0, 0, 0},  //TV = 7.60(491 lines)  AV=2.28  SV=5.23  BV=4.65
    {5523,1200,1024, 0, 0, 0},  //TV = 7.50(526 lines)  AV=2.28  SV=5.23  BV=4.55
    {5912,1200,1024, 0, 0, 0},  //TV = 7.40(563 lines)  AV=2.28  SV=5.23  BV=4.45
    {6332,1200,1024, 0, 0, 0},  //TV = 7.30(603 lines)  AV=2.28  SV=5.23  BV=4.35
    {6836,1200,1024, 0, 0, 0},  //TV = 7.19(651 lines)  AV=2.28  SV=5.23  BV=4.24
    {7319,1200,1024, 0, 0, 0},  //TV = 7.09(697 lines)  AV=2.28  SV=5.23  BV=4.14
    {7833,1200,1024, 0, 0, 0},  //TV = 7.00(746 lines)  AV=2.28  SV=5.23  BV=4.04
    {8400,1200,1024, 0, 0, 0},  //TV = 6.90(800 lines)  AV=2.28  SV=5.23  BV=3.94
    {8999,1200,1024, 0, 0, 0},  //TV = 6.80(857 lines)  AV=2.28  SV=5.23  BV=3.84
    {9639,1200,1024, 0, 0, 0},  //TV = 6.70(918 lines)  AV=2.28  SV=5.23  BV=3.74
    {9996,1232,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.27  BV=3.65
    {9996,1328,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.38  BV=3.54
    {9996,1424,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.48  BV=3.44
    {9996,1520,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.57  BV=3.35
    {9996,1632,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.67  BV=3.25
    {9996,1744,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.77  BV=3.15
    {9996,1872,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.87  BV=3.05
    {9996,2000,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.97  BV=2.95
    {9996,2144,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.07  BV=2.85
    {9996,2304,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.17  BV=2.75
    {20003,1232,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.27  BV=2.65
    {20003,1328,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.38  BV=2.54
    {20003,1424,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.48  BV=2.44
    {20003,1520,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.57  BV=2.35
    {20003,1632,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.67  BV=2.25
    {20003,1760,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.78  BV=2.14
    {29999,1248,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.29  BV=2.05
    {29999,1344,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.39  BV=1.94
    {29999,1440,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.49  BV=1.84
    {29999,1536,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.58  BV=1.75
    {29999,1648,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.69  BV=1.65
    {29999,1760,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.78  BV=1.55
    {29999,1888,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.88  BV=1.45
    {29999,2032,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.99  BV=1.35
    {29999,2176,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.09  BV=1.25
    {29999,2336,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.19  BV=1.14
    {40005,1872,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=5.87  BV=1.05
    {40005,2016,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=5.98  BV=0.94
    {40005,2160,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=6.08  BV=0.84
    {50001,1840,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=5.85  BV=0.75
    {50001,1984,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=5.95  BV=0.64
    {50001,2128,1024, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=6.06  BV=0.54
    {59997,1888,1024, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=5.88  BV=0.45
    {59997,2032,1024, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=5.99  BV=0.35
    {59997,2176,1024, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=6.09  BV=0.25
    {70004,2000,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=5.97  BV=0.15
    {70004,2144,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=6.07  BV=0.05
    {70004,2304,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=6.17  BV=-0.06
    {70004,2464,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=6.27  BV=-0.16
    {70004,2640,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=6.37  BV=-0.25
    {70004,2816,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=6.46  BV=-0.35
    {70004,3024,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=6.56  BV=-0.45
    {70004,3248,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=6.67  BV=-0.55
    {70004,3488,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=6.77  BV=-0.66
    {70004,3728,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=6.86  BV=-0.75
    {70004,4000,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=6.97  BV=-0.85
    {70004,4288,1024, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=7.07  BV=-0.95
    {80000,4032,1024, 0, 0, 0},  //TV = 3.64(7619 lines)  AV=2.28  SV=6.98  BV=-1.06
    {80000,4320,1024, 0, 0, 0},  //TV = 3.64(7619 lines)  AV=2.28  SV=7.08  BV=-1.16
    {89996,4112,1024, 0, 0, 0},  //TV = 3.47(8571 lines)  AV=2.28  SV=7.01  BV=-1.26
    {100002,3968,1024, 0, 0, 0},  //TV = 3.32(9524 lines)  AV=2.28  SV=6.95  BV=-1.36
    {100002,4240,1024, 0, 0, 0},  //TV = 3.32(9524 lines)  AV=2.28  SV=7.05  BV=-1.45
    {100002,4528,1024, 0, 0, 0},  //TV = 3.32(9524 lines)  AV=2.28  SV=7.14  BV=-1.55
    {100002,4880,1024, 0, 0, 0},  //TV = 3.32(9524 lines)  AV=2.28  SV=7.25  BV=-1.66
    {100002,5232,1024, 0, 0, 0},  //TV = 3.32(9524 lines)  AV=2.28  SV=7.35  BV=-1.76
    {100002,5600,1024, 0, 0, 0},  //TV = 3.32(9524 lines)  AV=2.28  SV=7.45  BV=-1.85
    {100002,6000,1024, 0, 0, 0},  //TV = 3.32(9524 lines)  AV=2.28  SV=7.55  BV=-1.95
    {100002,6432,1024, 0, 0, 0},  //TV = 3.32(9524 lines)  AV=2.28  SV=7.65  BV=-2.05
    {100002,6880,1024, 0, 0, 0},  //TV = 3.32(9524 lines)  AV=2.28  SV=7.75  BV=-2.15
    {100002,7376,1024, 0, 0, 0},  //TV = 3.32(9524 lines)  AV=2.28  SV=7.85  BV=-2.25
    {100002,7952,1024, 0, 0, 0},  //TV = 3.32(9524 lines)  AV=2.28  SV=7.96  BV=-2.36
    {100002,8512,1024, 0, 0, 0},  //TV = 3.32(9524 lines)  AV=2.28  SV=8.06  BV=-2.46
    {100002,9120,1024, 0, 0, 0},  //TV = 3.32(9524 lines)  AV=2.28  SV=8.15  BV=-2.56
    {100002,9776,1024, 0, 0, 0},  //TV = 3.32(9524 lines)  AV=2.28  SV=8.26  BV=-2.66
    {100002,10464,1024, 0, 0, 0},  //TV = 3.32(9524 lines)  AV=2.28  SV=8.35  BV=-2.76
    {100002,11216,1024, 0, 0, 0},  //TV = 3.32(9524 lines)  AV=2.28  SV=8.45  BV=-2.86
    {100002,12016,1024, 0, 0, 0},  //TV = 3.32(9524 lines)  AV=2.28  SV=8.55  BV=-2.96
    {100002,12288,1024, 0, 0, 0},  /* TV = 3.32(9524 lines)  AV=2.28  SV=8.58  BV=-2.99 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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
    135, //u4TotalIndex
    20, //i4StrobeTrigerBV
    104, //i4MaxBV
    -30, //i4MinBV
    90, //i4EffectiveMaxBV
    -30, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_AUTO, //ISO
    sAEScene3PLineTable_60Hz,
    sAEScene3PLineTable_50Hz,
    NULL,
};
static strEvPline sAEScene4PLineTable_60Hz =
{
{
    {199995,1712,1024, 0, 0, 0},  //TV = 2.32(19047 lines)  AV=2.28  SV=5.74  BV=-1.14
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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
    {200005,1712,1024, 0, 0, 0},  //TV = 2.32(19048 lines)  AV=2.28  SV=5.74  BV=-1.14
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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
    20, //i4StrobeTrigerBV
    -11, //i4MaxBV
    -12, //i4MinBV
    90, //i4EffectiveMaxBV
    -30, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_AUTO, //ISO
    sAEScene4PLineTable_60Hz,
    sAEScene4PLineTable_50Hz,
    NULL,
};
static strEvPline sAEScene5PLineTable_60Hz =
{
{
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {525,1200,1024, 0, 0, 0},  //TV = 10.90(50 lines)  AV=2.28  SV=5.23  BV=7.94
    {557,1200,1024, 0, 0, 0},  //TV = 10.81(53 lines)  AV=2.28  SV=5.23  BV=7.86
    {599,1200,1024, 0, 0, 0},  //TV = 10.71(57 lines)  AV=2.28  SV=5.23  BV=7.75
    {641,1200,1024, 0, 0, 0},  //TV = 10.61(61 lines)  AV=2.28  SV=5.23  BV=7.65
    {683,1216,1024, 0, 0, 0},  //TV = 10.52(65 lines)  AV=2.28  SV=5.25  BV=7.54
    {735,1200,1024, 0, 0, 0},  //TV = 10.41(70 lines)  AV=2.28  SV=5.23  BV=7.46
    {788,1200,1024, 0, 0, 0},  //TV = 10.31(75 lines)  AV=2.28  SV=5.23  BV=7.36
    {840,1216,1024, 0, 0, 0},  //TV = 10.22(80 lines)  AV=2.28  SV=5.25  BV=7.24
    {903,1200,1024, 0, 0, 0},  //TV = 10.11(86 lines)  AV=2.28  SV=5.23  BV=7.16
    {977,1200,1024, 0, 0, 0},  //TV = 10.00(93 lines)  AV=2.28  SV=5.23  BV=7.05
    {1040,1200,1024, 0, 0, 0},  //TV = 9.91(99 lines)  AV=2.28  SV=5.23  BV=6.96
    {1113,1200,1024, 0, 0, 0},  //TV = 9.81(106 lines)  AV=2.28  SV=5.23  BV=6.86
    {1197,1200,1024, 0, 0, 0},  //TV = 9.71(114 lines)  AV=2.28  SV=5.23  BV=6.75
    {1292,1200,1024, 0, 0, 0},  //TV = 9.60(123 lines)  AV=2.28  SV=5.23  BV=6.64
    {1376,1200,1024, 0, 0, 0},  //TV = 9.51(131 lines)  AV=2.28  SV=5.23  BV=6.55
    {1481,1200,1024, 0, 0, 0},  //TV = 9.40(141 lines)  AV=2.28  SV=5.23  BV=6.45
    {1575,1200,1024, 0, 0, 0},  //TV = 9.31(150 lines)  AV=2.28  SV=5.23  BV=6.36
    {1701,1200,1024, 0, 0, 0},  //TV = 9.20(162 lines)  AV=2.28  SV=5.23  BV=6.25
    {1827,1200,1024, 0, 0, 0},  //TV = 9.10(174 lines)  AV=2.28  SV=5.23  BV=6.14
    {1953,1200,1024, 0, 0, 0},  //TV = 9.00(186 lines)  AV=2.28  SV=5.23  BV=6.05
    {2090,1200,1024, 0, 0, 0},  //TV = 8.90(199 lines)  AV=2.28  SV=5.23  BV=5.95
    {2258,1200,1024, 0, 0, 0},  //TV = 8.79(215 lines)  AV=2.28  SV=5.23  BV=5.84
    {2415,1200,1024, 0, 0, 0},  //TV = 8.69(230 lines)  AV=2.28  SV=5.23  BV=5.74
    {2583,1200,1024, 0, 0, 0},  //TV = 8.60(246 lines)  AV=2.28  SV=5.23  BV=5.64
    {2762,1200,1024, 0, 0, 0},  //TV = 8.50(263 lines)  AV=2.28  SV=5.23  BV=5.55
    {2972,1200,1024, 0, 0, 0},  //TV = 8.39(283 lines)  AV=2.28  SV=5.23  BV=5.44
    {3182,1200,1024, 0, 0, 0},  //TV = 8.30(303 lines)  AV=2.28  SV=5.23  BV=5.34
    {3402,1200,1024, 0, 0, 0},  //TV = 8.20(324 lines)  AV=2.28  SV=5.23  BV=5.25
    {3644,1200,1024, 0, 0, 0},  //TV = 8.10(347 lines)  AV=2.28  SV=5.23  BV=5.15
    {3896,1200,1024, 0, 0, 0},  //TV = 8.00(371 lines)  AV=2.28  SV=5.23  BV=5.05
    {4179,1200,1024, 0, 0, 0},  //TV = 7.90(398 lines)  AV=2.28  SV=5.23  BV=4.95
    {4505,1200,1024, 0, 0, 0},  //TV = 7.79(429 lines)  AV=2.28  SV=5.23  BV=4.84
    {4820,1200,1024, 0, 0, 0},  //TV = 7.70(459 lines)  AV=2.28  SV=5.23  BV=4.74
    {5156,1200,1024, 0, 0, 0},  //TV = 7.60(491 lines)  AV=2.28  SV=5.23  BV=4.65
    {5523,1200,1024, 0, 0, 0},  //TV = 7.50(526 lines)  AV=2.28  SV=5.23  BV=4.55
    {5912,1200,1024, 0, 0, 0},  //TV = 7.40(563 lines)  AV=2.28  SV=5.23  BV=4.45
    {6332,1200,1024, 0, 0, 0},  //TV = 7.30(603 lines)  AV=2.28  SV=5.23  BV=4.35
    {6836,1200,1024, 0, 0, 0},  //TV = 7.19(651 lines)  AV=2.28  SV=5.23  BV=4.24
    {7319,1200,1024, 0, 0, 0},  //TV = 7.09(697 lines)  AV=2.28  SV=5.23  BV=4.14
    {7833,1200,1024, 0, 0, 0},  //TV = 7.00(746 lines)  AV=2.28  SV=5.23  BV=4.04
    {8338,1200,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.23  BV=3.95
    {8338,1296,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.34  BV=3.84
    {8338,1376,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.43  BV=3.75
    {8338,1488,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.54  BV=3.64
    {8338,1584,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.63  BV=3.55
    {8338,1696,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.73  BV=3.45
    {8338,1824,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.83  BV=3.35
    {8338,1952,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.93  BV=3.25
    {8338,2096,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.03  BV=3.15
    {8338,2240,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.13  BV=3.05
    {16664,1200,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.23  BV=2.95
    {16664,1296,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.34  BV=2.84
    {16664,1376,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.43  BV=2.76
    {16664,1488,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.54  BV=2.64
    {16664,1600,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.64  BV=2.54
    {16664,1696,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.73  BV=2.45
    {25001,1216,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.25  BV=2.35
    {25001,1296,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.34  BV=2.26
    {25001,1408,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.46  BV=2.14
    {25001,1488,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.54  BV=2.06
    {33327,1200,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.23  BV=1.95
    {33327,1296,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.34  BV=1.84
    {33327,1376,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.43  BV=1.76
    {33327,1488,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.54  BV=1.64
    {33327,1584,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.63  BV=1.55
    {33327,1696,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.73  BV=1.45
    {33327,1824,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.83  BV=1.35
    {33327,1952,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.93  BV=1.25
    {33327,2096,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.03  BV=1.15
    {33327,2256,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.14  BV=1.04
    {33327,2416,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.24  BV=0.94
    {33327,2576,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.33  BV=0.85
    {33327,2784,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.44  BV=0.74
    {33327,2976,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.54  BV=0.64
    {33327,3184,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.64  BV=0.55
    {33327,3408,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.73  BV=0.45
    {33327,3648,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.83  BV=0.35
    {33327,3920,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.94  BV=0.25
    {33327,4208,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.04  BV=0.14
    {33327,4496,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.13  BV=0.05
    {33327,4816,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.23  BV=-0.05
    {33327,5152,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.33  BV=-0.15
    {33327,5520,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.43  BV=-0.25
    {33327,5952,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.54  BV=-0.36
    {33327,6384,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.64  BV=-0.46
    {33327,6832,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.74  BV=-0.56
    {33327,7312,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.84  BV=-0.65
    {33327,7840,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.94  BV=-0.75
    {33327,8400,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.04  BV=-0.85
    {33327,8992,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.13  BV=-0.95
    {33327,9632,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.23  BV=-1.05
    {33327,10320,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.33  BV=-1.15
    {33327,11056,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.43  BV=-1.25
    {33327,11840,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.53  BV=-1.35
    {33327,12720,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.63  BV=-1.45
    {33327,13632,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.73  BV=-1.55
    {41664,11680,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=8.51  BV=-1.65
    {41664,12544,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=8.61  BV=-1.75
    {41664,13440,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=8.71  BV=-1.85
    {41664,14400,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=8.81  BV=-1.95
    {41664,15424,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=8.91  BV=-2.05
    {41664,15872,1064, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.01  BV=-2.15
    {41664,15872,1136, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.10  BV=-2.24
    {41664,15872,1224, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.21  BV=-2.35
    {41664,15872,1312, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.31  BV=-2.45
    {41664,15872,1400, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.41  BV=-2.55
    {41664,15872,1504, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.51  BV=-2.65
    {41664,15872,1608, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.61  BV=-2.75
    {41664,15872,1728, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.71  BV=-2.85
    {41664,15872,1848, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.81  BV=-2.95
    {41664,15872,2000, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.92  BV=-3.06
    {41664,15872,2136, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=10.01  BV=-3.15
    {41664,15872,2296, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=10.12  BV=-3.26
    {41664,15872,2456, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=10.22  BV=-3.36
    {41664,15872,2632, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=10.32  BV=-3.46
    {41664,15872,2824, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=10.42  BV=-3.56
    {41664,15872,3008, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=10.51  BV=-3.65
    {41664,15872,3224, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=10.61  BV=-3.75
    {50001,15872,2880, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=10.45  BV=-3.85
    {50001,15872,3088, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=10.55  BV=-3.95
    {58328,15872,2832, 0, 0, 0},  //TV = 4.10(5555 lines)  AV=2.28  SV=10.42  BV=-4.05
    {58328,15872,3040, 0, 0, 0},  //TV = 4.10(5555 lines)  AV=2.28  SV=10.52  BV=-4.15
    {66665,15872,2848, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=10.43  BV=-4.25
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {525,1200,1024, 0, 0, 0},  //TV = 10.90(50 lines)  AV=2.28  SV=5.23  BV=7.94
    {557,1200,1024, 0, 0, 0},  //TV = 10.81(53 lines)  AV=2.28  SV=5.23  BV=7.86
    {599,1200,1024, 0, 0, 0},  //TV = 10.71(57 lines)  AV=2.28  SV=5.23  BV=7.75
    {641,1200,1024, 0, 0, 0},  //TV = 10.61(61 lines)  AV=2.28  SV=5.23  BV=7.65
    {683,1216,1024, 0, 0, 0},  //TV = 10.52(65 lines)  AV=2.28  SV=5.25  BV=7.54
    {735,1200,1024, 0, 0, 0},  //TV = 10.41(70 lines)  AV=2.28  SV=5.23  BV=7.46
    {788,1200,1024, 0, 0, 0},  //TV = 10.31(75 lines)  AV=2.28  SV=5.23  BV=7.36
    {840,1216,1024, 0, 0, 0},  //TV = 10.22(80 lines)  AV=2.28  SV=5.25  BV=7.24
    {903,1200,1024, 0, 0, 0},  //TV = 10.11(86 lines)  AV=2.28  SV=5.23  BV=7.16
    {977,1200,1024, 0, 0, 0},  //TV = 10.00(93 lines)  AV=2.28  SV=5.23  BV=7.05
    {1040,1200,1024, 0, 0, 0},  //TV = 9.91(99 lines)  AV=2.28  SV=5.23  BV=6.96
    {1113,1200,1024, 0, 0, 0},  //TV = 9.81(106 lines)  AV=2.28  SV=5.23  BV=6.86
    {1197,1200,1024, 0, 0, 0},  //TV = 9.71(114 lines)  AV=2.28  SV=5.23  BV=6.75
    {1292,1200,1024, 0, 0, 0},  //TV = 9.60(123 lines)  AV=2.28  SV=5.23  BV=6.64
    {1376,1200,1024, 0, 0, 0},  //TV = 9.51(131 lines)  AV=2.28  SV=5.23  BV=6.55
    {1481,1200,1024, 0, 0, 0},  //TV = 9.40(141 lines)  AV=2.28  SV=5.23  BV=6.45
    {1575,1200,1024, 0, 0, 0},  //TV = 9.31(150 lines)  AV=2.28  SV=5.23  BV=6.36
    {1701,1200,1024, 0, 0, 0},  //TV = 9.20(162 lines)  AV=2.28  SV=5.23  BV=6.25
    {1827,1200,1024, 0, 0, 0},  //TV = 9.10(174 lines)  AV=2.28  SV=5.23  BV=6.14
    {1953,1200,1024, 0, 0, 0},  //TV = 9.00(186 lines)  AV=2.28  SV=5.23  BV=6.05
    {2090,1200,1024, 0, 0, 0},  //TV = 8.90(199 lines)  AV=2.28  SV=5.23  BV=5.95
    {2258,1200,1024, 0, 0, 0},  //TV = 8.79(215 lines)  AV=2.28  SV=5.23  BV=5.84
    {2415,1200,1024, 0, 0, 0},  //TV = 8.69(230 lines)  AV=2.28  SV=5.23  BV=5.74
    {2583,1200,1024, 0, 0, 0},  //TV = 8.60(246 lines)  AV=2.28  SV=5.23  BV=5.64
    {2762,1200,1024, 0, 0, 0},  //TV = 8.50(263 lines)  AV=2.28  SV=5.23  BV=5.55
    {2972,1200,1024, 0, 0, 0},  //TV = 8.39(283 lines)  AV=2.28  SV=5.23  BV=5.44
    {3182,1200,1024, 0, 0, 0},  //TV = 8.30(303 lines)  AV=2.28  SV=5.23  BV=5.34
    {3402,1200,1024, 0, 0, 0},  //TV = 8.20(324 lines)  AV=2.28  SV=5.23  BV=5.25
    {3644,1200,1024, 0, 0, 0},  //TV = 8.10(347 lines)  AV=2.28  SV=5.23  BV=5.15
    {3896,1200,1024, 0, 0, 0},  //TV = 8.00(371 lines)  AV=2.28  SV=5.23  BV=5.05
    {4179,1200,1024, 0, 0, 0},  //TV = 7.90(398 lines)  AV=2.28  SV=5.23  BV=4.95
    {4505,1200,1024, 0, 0, 0},  //TV = 7.79(429 lines)  AV=2.28  SV=5.23  BV=4.84
    {4820,1200,1024, 0, 0, 0},  //TV = 7.70(459 lines)  AV=2.28  SV=5.23  BV=4.74
    {5156,1200,1024, 0, 0, 0},  //TV = 7.60(491 lines)  AV=2.28  SV=5.23  BV=4.65
    {5523,1200,1024, 0, 0, 0},  //TV = 7.50(526 lines)  AV=2.28  SV=5.23  BV=4.55
    {5912,1200,1024, 0, 0, 0},  //TV = 7.40(563 lines)  AV=2.28  SV=5.23  BV=4.45
    {6332,1200,1024, 0, 0, 0},  //TV = 7.30(603 lines)  AV=2.28  SV=5.23  BV=4.35
    {6836,1200,1024, 0, 0, 0},  //TV = 7.19(651 lines)  AV=2.28  SV=5.23  BV=4.24
    {7319,1200,1024, 0, 0, 0},  //TV = 7.09(697 lines)  AV=2.28  SV=5.23  BV=4.14
    {7833,1200,1024, 0, 0, 0},  //TV = 7.00(746 lines)  AV=2.28  SV=5.23  BV=4.04
    {8400,1200,1024, 0, 0, 0},  //TV = 6.90(800 lines)  AV=2.28  SV=5.23  BV=3.94
    {8999,1200,1024, 0, 0, 0},  //TV = 6.80(857 lines)  AV=2.28  SV=5.23  BV=3.84
    {9639,1200,1024, 0, 0, 0},  //TV = 6.70(918 lines)  AV=2.28  SV=5.23  BV=3.74
    {9996,1232,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.27  BV=3.65
    {9996,1328,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.38  BV=3.54
    {9996,1424,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.48  BV=3.44
    {9996,1520,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.57  BV=3.35
    {9996,1632,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.67  BV=3.25
    {9996,1744,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.77  BV=3.15
    {9996,1872,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.87  BV=3.05
    {9996,2000,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.97  BV=2.95
    {9996,2144,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.07  BV=2.85
    {9996,2304,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.17  BV=2.75
    {20003,1232,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.27  BV=2.65
    {20003,1328,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.38  BV=2.54
    {20003,1424,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.48  BV=2.44
    {20003,1520,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.57  BV=2.35
    {20003,1632,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.67  BV=2.25
    {20003,1760,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.78  BV=2.14
    {29999,1248,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.29  BV=2.05
    {29999,1344,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.39  BV=1.94
    {29999,1440,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.49  BV=1.84
    {29999,1536,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.58  BV=1.75
    {29999,1648,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.69  BV=1.65
    {29999,1760,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.78  BV=1.55
    {29999,1888,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.88  BV=1.45
    {29999,2032,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.99  BV=1.35
    {29999,2176,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.09  BV=1.25
    {29999,2320,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.18  BV=1.15
    {29999,2496,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.29  BV=1.05
    {29999,2672,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.38  BV=0.95
    {29999,2864,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.48  BV=0.85
    {29999,3072,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.58  BV=0.75
    {29999,3296,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.69  BV=0.65
    {29999,3536,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.79  BV=0.55
    {29999,3776,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.88  BV=0.45
    {29999,4048,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.98  BV=0.35
    {29999,4368,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.09  BV=0.24
    {29999,4672,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.19  BV=0.14
    {29999,5008,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.29  BV=0.04
    {29999,5360,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.39  BV=-0.05
    {29999,5728,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.48  BV=-0.15
    {29999,6144,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.58  BV=-0.25
    {29999,6624,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.69  BV=-0.36
    {29999,7088,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.79  BV=-0.46
    {29999,7584,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.89  BV=-0.55
    {29999,8128,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.99  BV=-0.65
    {29999,8704,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.09  BV=-0.75
    {29999,9328,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.19  BV=-0.85
    {29999,9984,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.29  BV=-0.95
    {29999,10704,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.39  BV=-1.05
    {29999,11472,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.49  BV=-1.15
    {29999,12288,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.58  BV=-1.25
    {29999,13248,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.69  BV=-1.36
    {29999,14128,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.79  BV=-1.45
    {40005,11360,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=8.47  BV=-1.55
    {40005,12160,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=8.57  BV=-1.65
    {40005,13056,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=8.67  BV=-1.75
    {40005,14000,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=8.77  BV=-1.85
    {40005,14992,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=8.87  BV=-1.95
    {40005,15872,1032, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=8.97  BV=-2.05
    {40005,15872,1104, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.06  BV=-2.14
    {40005,15872,1184, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.16  BV=-2.24
    {40005,15872,1272, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.27  BV=-2.35
    {40005,15872,1360, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.36  BV=-2.44
    {40005,15872,1464, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.47  BV=-2.55
    {40005,15872,1568, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.57  BV=-2.65
    {40005,15872,1680, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.67  BV=-2.75
    {40005,15872,1800, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.77  BV=-2.85
    {40005,15872,1928, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.87  BV=-2.95
    {40005,15872,2080, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.98  BV=-3.06
    {40005,15872,2232, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=10.08  BV=-3.16
    {40005,15872,2392, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=10.18  BV=-3.26
    {40005,15872,2560, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=10.28  BV=-3.36
    {40005,15872,2744, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=10.38  BV=-3.46
    {40005,15872,2944, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=10.48  BV=-3.56
    {40005,15872,3136, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=10.57  BV=-3.65
    {50001,15872,2688, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=10.35  BV=-3.75
    {50001,15872,2880, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=10.45  BV=-3.85
    {50001,15872,3088, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=10.55  BV=-3.95
    {59997,15872,2760, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=10.38  BV=-4.05
    {59997,15872,2952, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=10.48  BV=-4.15
    {59997,15872,3168, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=10.58  BV=-4.25
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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
    147, //u4TotalIndex
    20, //i4StrobeTrigerBV
    104, //i4MaxBV
    -43, //i4MinBV
    90, //i4EffectiveMaxBV
    -30, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_AUTO, //ISO
    sAEScene5PLineTable_60Hz,
    sAEScene5PLineTable_50Hz,
    NULL,
};
static strEvPline sAEScene6PLineTable_60Hz =
{
{
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {494,1264,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.30  BV=7.95
    {494,1360,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.41  BV=7.85
    {494,1456,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.51  BV=7.75
    {494,1568,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.61  BV=7.64
    {494,1680,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.71  BV=7.54
    {494,1808,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.82  BV=7.44
    {494,1936,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.92  BV=7.34
    {494,2064,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.01  BV=7.25
    {494,2208,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.11  BV=7.15
    {494,2368,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.21  BV=7.05
    {494,2544,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.31  BV=6.95
    {494,2720,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.41  BV=6.85
    {494,2928,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.52  BV=6.74
    {494,3136,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.61  BV=6.64
    {494,3344,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.71  BV=6.55
    {494,3584,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.81  BV=6.45
    {494,3840,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.91  BV=6.35
    {494,4144,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.02  BV=6.24
    {494,4432,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.11  BV=6.14
    {494,4752,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.21  BV=6.04
    {494,5088,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.31  BV=5.95
    {494,5440,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.41  BV=5.85
    {494,5872,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.52  BV=5.74
    {494,6288,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.62  BV=5.64
    {494,6720,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.71  BV=5.54
    {494,7200,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.81  BV=5.44
    {494,7712,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.91  BV=5.35
    {494,8272,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.01  BV=5.24
    {494,8848,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.11  BV=5.15
    {494,9488,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.21  BV=5.05
    {494,10160,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.31  BV=4.95
    {494,10880,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.41  BV=4.85
    {494,11664,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.51  BV=4.75
    {494,12496,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.61  BV=4.65
    {494,13376,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.71  BV=4.55
    {494,14432,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.82  BV=4.44
    {494,15472,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.92  BV=4.34
    {494,15872,1064, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.01  BV=4.25
    {494,15872,1144, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.11  BV=4.14
    {494,15872,1224, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.21  BV=4.05
    {494,15872,1312, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.31  BV=3.95
    {494,15872,1408, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.41  BV=3.84
    {494,15872,1504, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.51  BV=3.75
    {494,15872,1616, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.61  BV=3.65
    {494,15872,1728, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.71  BV=3.55
    {494,15872,1856, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.81  BV=3.45
    {494,15872,1992, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.91  BV=3.34
    {494,15872,2128, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=10.01  BV=3.25
    {536,15872,2096, 0, 0, 0},  //TV = 10.87(51 lines)  AV=2.28  SV=9.99  BV=3.15
    {567,15872,2128, 0, 0, 0},  //TV = 10.78(54 lines)  AV=2.28  SV=10.01  BV=3.05
    {609,15872,2128, 0, 0, 0},  //TV = 10.68(58 lines)  AV=2.28  SV=10.01  BV=2.95
    {662,15872,2096, 0, 0, 0},  //TV = 10.56(63 lines)  AV=2.28  SV=9.99  BV=2.85
    {704,15872,2120, 0, 0, 0},  //TV = 10.47(67 lines)  AV=2.28  SV=10.00  BV=2.74
    {756,15872,2112, 0, 0, 0},  //TV = 10.37(72 lines)  AV=2.28  SV=10.00  BV=2.65
    {809,15872,2120, 0, 0, 0},  //TV = 10.27(77 lines)  AV=2.28  SV=10.00  BV=2.54
    {872,15872,2096, 0, 0, 0},  //TV = 10.16(83 lines)  AV=2.28  SV=9.99  BV=2.45
    {935,15872,2104, 0, 0, 0},  //TV = 10.06(89 lines)  AV=2.28  SV=9.99  BV=2.34
    {998,15872,2112, 0, 0, 0},  //TV = 9.97(95 lines)  AV=2.28  SV=10.00  BV=2.25
    {1071,15872,2104, 0, 0, 0},  //TV = 9.87(102 lines)  AV=2.28  SV=9.99  BV=2.15
    {1145,15872,2112, 0, 0, 0},  //TV = 9.77(109 lines)  AV=2.28  SV=10.00  BV=2.05
    {1229,15872,2112, 0, 0, 0},  //TV = 9.67(117 lines)  AV=2.28  SV=10.00  BV=1.94
    {1313,15872,2112, 0, 0, 0},  //TV = 9.57(125 lines)  AV=2.28  SV=10.00  BV=1.85
    {1407,15872,2120, 0, 0, 0},  //TV = 9.47(134 lines)  AV=2.28  SV=10.00  BV=1.74
    {1512,15872,2104, 0, 0, 0},  //TV = 9.37(144 lines)  AV=2.28  SV=9.99  BV=1.65
    {1617,15872,2120, 0, 0, 0},  //TV = 9.27(154 lines)  AV=2.28  SV=10.00  BV=1.54
    {1733,15872,2112, 0, 0, 0},  //TV = 9.17(165 lines)  AV=2.28  SV=10.00  BV=1.45
    {1869,15872,2104, 0, 0, 0},  //TV = 9.06(178 lines)  AV=2.28  SV=9.99  BV=1.35
    {1995,15872,2112, 0, 0, 0},  //TV = 8.97(190 lines)  AV=2.28  SV=10.00  BV=1.25
    {2132,15872,2112, 0, 0, 0},  //TV = 8.87(203 lines)  AV=2.28  SV=10.00  BV=1.15
    {2289,15872,2112, 0, 0, 0},  //TV = 8.77(218 lines)  AV=2.28  SV=10.00  BV=1.05
    {2457,15872,2104, 0, 0, 0},  //TV = 8.67(234 lines)  AV=2.28  SV=9.99  BV=0.95
    {2625,15872,2112, 0, 0, 0},  //TV = 8.57(250 lines)  AV=2.28  SV=10.00  BV=0.85
    {2825,15872,2112, 0, 0, 0},  //TV = 8.47(269 lines)  AV=2.28  SV=10.00  BV=0.74
    {3024,15872,2112, 0, 0, 0},  //TV = 8.37(288 lines)  AV=2.28  SV=10.00  BV=0.65
    {3234,15872,2112, 0, 0, 0},  //TV = 8.27(308 lines)  AV=2.28  SV=10.00  BV=0.55
    {3465,15872,2112, 0, 0, 0},  //TV = 8.17(330 lines)  AV=2.28  SV=10.00  BV=0.45
    {3738,15872,2104, 0, 0, 0},  //TV = 8.06(356 lines)  AV=2.28  SV=9.99  BV=0.35
    {4001,15872,2104, 0, 0, 0},  //TV = 7.97(381 lines)  AV=2.28  SV=9.99  BV=0.25
    {4274,15872,2112, 0, 0, 0},  //TV = 7.87(407 lines)  AV=2.28  SV=10.00  BV=0.15
    {4578,15872,2112, 0, 0, 0},  //TV = 7.77(436 lines)  AV=2.28  SV=10.00  BV=0.05
    {4904,15872,2112, 0, 0, 0},  //TV = 7.67(467 lines)  AV=2.28  SV=10.00  BV=-0.05
    {5250,15872,2112, 0, 0, 0},  //TV = 7.57(500 lines)  AV=2.28  SV=10.00  BV=-0.15
    {5660,15872,2112, 0, 0, 0},  //TV = 7.46(539 lines)  AV=2.28  SV=10.00  BV=-0.26
    {6059,15872,2112, 0, 0, 0},  //TV = 7.37(577 lines)  AV=2.28  SV=10.00  BV=-0.36
    {6489,15872,2112, 0, 0, 0},  //TV = 7.27(618 lines)  AV=2.28  SV=10.00  BV=-0.46
    {6951,15872,2112, 0, 0, 0},  //TV = 7.17(662 lines)  AV=2.28  SV=10.00  BV=-0.56
    {7445,15872,2112, 0, 0, 0},  //TV = 7.07(709 lines)  AV=2.28  SV=10.00  BV=-0.65
    {7980,15872,2112, 0, 0, 0},  //TV = 6.97(760 lines)  AV=2.28  SV=10.00  BV=-0.75
    {8337,15872,2160, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=10.03  BV=-0.85
    {8337,15872,2320, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=10.13  BV=-0.95
    {8337,15872,2480, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=10.23  BV=-1.05
    {8337,15872,2656, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=10.33  BV=-1.15
    {8337,15872,2848, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=10.43  BV=-1.25
    {8337,15872,3056, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=10.53  BV=-1.35
    {16664,15872,1632, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=9.63  BV=-1.44
    {16664,15872,1752, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=9.73  BV=-1.55
    {16664,15872,1888, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=9.84  BV=-1.65
    {16664,15872,2024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=9.94  BV=-1.76
    {16664,15872,2168, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=10.04  BV=-1.85
    {16664,15872,2328, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=10.14  BV=-1.96
    {16664,15872,2496, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=10.24  BV=-2.06
    {25001,15872,1776, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=9.75  BV=-2.15
    {25001,15872,1904, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=9.85  BV=-2.25
    {25001,15872,2040, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=9.95  BV=-2.35
    {25001,15872,2192, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=10.05  BV=-2.46
    {25001,15872,2344, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=10.15  BV=-2.55
    {33327,15872,1888, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=9.84  BV=-2.65
    {33327,15872,2024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=9.94  BV=-2.76
    {33327,15872,2168, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=10.04  BV=-2.85
    {33327,15872,2320, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=10.13  BV=-2.95
    {41664,15872,1992, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.91  BV=-3.05
    {41664,15872,2128, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=10.01  BV=-3.15
    {41664,15872,2288, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=10.11  BV=-3.25
    {50001,15872,2040, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=9.95  BV=-3.35
    {50001,15872,2184, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=10.05  BV=-3.45
    {58328,15872,2008, 0, 0, 0},  //TV = 4.10(5555 lines)  AV=2.28  SV=9.93  BV=-3.55
    {58328,15872,2152, 0, 0, 0},  //TV = 4.10(5555 lines)  AV=2.28  SV=10.03  BV=-3.65
    {66665,15872,2016, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=9.93  BV=-3.75
    {66665,15872,2160, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=10.03  BV=-3.85
    {75002,15872,2064, 0, 0, 0},  //TV = 3.74(7143 lines)  AV=2.28  SV=9.97  BV=-3.95
    {75002,15872,2208, 0, 0, 0},  //TV = 3.74(7143 lines)  AV=2.28  SV=10.06  BV=-4.05
    {83328,15872,2128, 0, 0, 0},  //TV = 3.59(7936 lines)  AV=2.28  SV=10.01  BV=-4.15
    {91665,15872,2072, 0, 0, 0},  //TV = 3.45(8730 lines)  AV=2.28  SV=9.97  BV=-4.25
    {99992,15872,2040, 0, 0, 0},  //TV = 3.32(9523 lines)  AV=2.28  SV=9.95  BV=-4.35
    {99992,15872,2184, 0, 0, 0},  //TV = 3.32(9523 lines)  AV=2.28  SV=10.05  BV=-4.45
    {108329,15872,2160, 0, 0, 0},  //TV = 3.21(10317 lines)  AV=2.28  SV=10.03  BV=-4.55
    {116666,15872,2152, 0, 0, 0},  //TV = 3.10(11111 lines)  AV=2.28  SV=10.03  BV=-4.65
    {124992,15872,2152, 0, 0, 0},  //TV = 3.00(11904 lines)  AV=2.28  SV=10.03  BV=-4.75
    {133329,15872,2160, 0, 0, 0},  //TV = 2.91(12698 lines)  AV=2.28  SV=10.03  BV=-4.85
    {149993,15872,2056, 0, 0, 0},  //TV = 2.74(14285 lines)  AV=2.28  SV=9.96  BV=-4.95
    {158330,15872,2088, 0, 0, 0},  //TV = 2.66(15079 lines)  AV=2.28  SV=9.98  BV=-5.05
    {166656,15872,2128, 0, 0, 0},  //TV = 2.59(15872 lines)  AV=2.28  SV=10.01  BV=-5.15
    {183330,15872,2072, 0, 0, 0},  //TV = 2.45(17460 lines)  AV=2.28  SV=9.97  BV=-5.25
    {191657,15872,2128, 0, 0, 0},  //TV = 2.38(18253 lines)  AV=2.28  SV=10.01  BV=-5.35
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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

static strEvPline sAEScene6PLineTable_50Hz =
{
{
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {494,1264,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.30  BV=7.95
    {494,1360,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.41  BV=7.85
    {494,1456,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.51  BV=7.75
    {494,1568,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.61  BV=7.64
    {494,1680,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.71  BV=7.54
    {494,1808,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.82  BV=7.44
    {494,1936,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.92  BV=7.34
    {494,2064,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.01  BV=7.25
    {494,2208,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.11  BV=7.15
    {494,2368,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.21  BV=7.05
    {494,2544,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.31  BV=6.95
    {494,2720,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.41  BV=6.85
    {494,2928,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.52  BV=6.74
    {494,3136,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.61  BV=6.64
    {494,3344,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.71  BV=6.55
    {494,3584,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.81  BV=6.45
    {494,3840,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.91  BV=6.35
    {494,4144,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.02  BV=6.24
    {494,4432,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.11  BV=6.14
    {494,4752,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.21  BV=6.04
    {494,5088,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.31  BV=5.95
    {494,5440,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.41  BV=5.85
    {494,5872,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.52  BV=5.74
    {494,6288,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.62  BV=5.64
    {494,6720,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.71  BV=5.54
    {494,7200,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.81  BV=5.44
    {494,7712,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.91  BV=5.35
    {494,8272,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.01  BV=5.24
    {494,8848,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.11  BV=5.15
    {494,9488,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.21  BV=5.05
    {494,10160,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.31  BV=4.95
    {494,10880,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.41  BV=4.85
    {494,11664,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.51  BV=4.75
    {494,12496,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.61  BV=4.65
    {494,13376,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.71  BV=4.55
    {494,14432,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.82  BV=4.44
    {494,15472,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.92  BV=4.34
    {494,15872,1064, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.01  BV=4.25
    {494,15872,1144, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.11  BV=4.14
    {494,15872,1224, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.21  BV=4.05
    {494,15872,1312, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.31  BV=3.95
    {494,15872,1408, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.41  BV=3.84
    {494,15872,1504, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.51  BV=3.75
    {494,15872,1616, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.61  BV=3.65
    {494,15872,1728, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.71  BV=3.55
    {494,15872,1856, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.81  BV=3.45
    {494,15872,1992, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.91  BV=3.34
    {494,15872,2128, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=10.01  BV=3.25
    {536,15872,2096, 0, 0, 0},  //TV = 10.87(51 lines)  AV=2.28  SV=9.99  BV=3.15
    {567,15872,2128, 0, 0, 0},  //TV = 10.78(54 lines)  AV=2.28  SV=10.01  BV=3.05
    {609,15872,2128, 0, 0, 0},  //TV = 10.68(58 lines)  AV=2.28  SV=10.01  BV=2.95
    {662,15872,2096, 0, 0, 0},  //TV = 10.56(63 lines)  AV=2.28  SV=9.99  BV=2.85
    {704,15872,2120, 0, 0, 0},  //TV = 10.47(67 lines)  AV=2.28  SV=10.00  BV=2.74
    {756,15872,2112, 0, 0, 0},  //TV = 10.37(72 lines)  AV=2.28  SV=10.00  BV=2.65
    {809,15872,2120, 0, 0, 0},  //TV = 10.27(77 lines)  AV=2.28  SV=10.00  BV=2.54
    {872,15872,2096, 0, 0, 0},  //TV = 10.16(83 lines)  AV=2.28  SV=9.99  BV=2.45
    {935,15872,2104, 0, 0, 0},  //TV = 10.06(89 lines)  AV=2.28  SV=9.99  BV=2.34
    {998,15872,2112, 0, 0, 0},  //TV = 9.97(95 lines)  AV=2.28  SV=10.00  BV=2.25
    {1071,15872,2104, 0, 0, 0},  //TV = 9.87(102 lines)  AV=2.28  SV=9.99  BV=2.15
    {1145,15872,2112, 0, 0, 0},  //TV = 9.77(109 lines)  AV=2.28  SV=10.00  BV=2.05
    {1229,15872,2112, 0, 0, 0},  //TV = 9.67(117 lines)  AV=2.28  SV=10.00  BV=1.94
    {1313,15872,2112, 0, 0, 0},  //TV = 9.57(125 lines)  AV=2.28  SV=10.00  BV=1.85
    {1407,15872,2120, 0, 0, 0},  //TV = 9.47(134 lines)  AV=2.28  SV=10.00  BV=1.74
    {1512,15872,2104, 0, 0, 0},  //TV = 9.37(144 lines)  AV=2.28  SV=9.99  BV=1.65
    {1617,15872,2120, 0, 0, 0},  //TV = 9.27(154 lines)  AV=2.28  SV=10.00  BV=1.54
    {1733,15872,2112, 0, 0, 0},  //TV = 9.17(165 lines)  AV=2.28  SV=10.00  BV=1.45
    {1869,15872,2104, 0, 0, 0},  //TV = 9.06(178 lines)  AV=2.28  SV=9.99  BV=1.35
    {1995,15872,2112, 0, 0, 0},  //TV = 8.97(190 lines)  AV=2.28  SV=10.00  BV=1.25
    {2132,15872,2112, 0, 0, 0},  //TV = 8.87(203 lines)  AV=2.28  SV=10.00  BV=1.15
    {2289,15872,2112, 0, 0, 0},  //TV = 8.77(218 lines)  AV=2.28  SV=10.00  BV=1.05
    {2457,15872,2104, 0, 0, 0},  //TV = 8.67(234 lines)  AV=2.28  SV=9.99  BV=0.95
    {2625,15872,2112, 0, 0, 0},  //TV = 8.57(250 lines)  AV=2.28  SV=10.00  BV=0.85
    {2825,15872,2112, 0, 0, 0},  //TV = 8.47(269 lines)  AV=2.28  SV=10.00  BV=0.74
    {3024,15872,2112, 0, 0, 0},  //TV = 8.37(288 lines)  AV=2.28  SV=10.00  BV=0.65
    {3234,15872,2112, 0, 0, 0},  //TV = 8.27(308 lines)  AV=2.28  SV=10.00  BV=0.55
    {3465,15872,2112, 0, 0, 0},  //TV = 8.17(330 lines)  AV=2.28  SV=10.00  BV=0.45
    {3738,15872,2104, 0, 0, 0},  //TV = 8.06(356 lines)  AV=2.28  SV=9.99  BV=0.35
    {4001,15872,2104, 0, 0, 0},  //TV = 7.97(381 lines)  AV=2.28  SV=9.99  BV=0.25
    {4274,15872,2112, 0, 0, 0},  //TV = 7.87(407 lines)  AV=2.28  SV=10.00  BV=0.15
    {4578,15872,2112, 0, 0, 0},  //TV = 7.77(436 lines)  AV=2.28  SV=10.00  BV=0.05
    {4904,15872,2112, 0, 0, 0},  //TV = 7.67(467 lines)  AV=2.28  SV=10.00  BV=-0.05
    {5250,15872,2112, 0, 0, 0},  //TV = 7.57(500 lines)  AV=2.28  SV=10.00  BV=-0.15
    {5660,15872,2112, 0, 0, 0},  //TV = 7.46(539 lines)  AV=2.28  SV=10.00  BV=-0.26
    {6059,15872,2112, 0, 0, 0},  //TV = 7.37(577 lines)  AV=2.28  SV=10.00  BV=-0.36
    {6489,15872,2112, 0, 0, 0},  //TV = 7.27(618 lines)  AV=2.28  SV=10.00  BV=-0.46
    {6951,15872,2112, 0, 0, 0},  //TV = 7.17(662 lines)  AV=2.28  SV=10.00  BV=-0.56
    {7445,15872,2112, 0, 0, 0},  //TV = 7.07(709 lines)  AV=2.28  SV=10.00  BV=-0.65
    {7980,15872,2112, 0, 0, 0},  //TV = 6.97(760 lines)  AV=2.28  SV=10.00  BV=-0.75
    {8547,15872,2112, 0, 0, 0},  //TV = 6.87(814 lines)  AV=2.28  SV=10.00  BV=-0.85
    {9156,15872,2112, 0, 0, 0},  //TV = 6.77(872 lines)  AV=2.28  SV=10.00  BV=-0.95
    {9807,15872,2112, 0, 0, 0},  //TV = 6.67(934 lines)  AV=2.28  SV=10.00  BV=-1.05
    {9996,15872,2216, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=10.07  BV=-1.15
    {9996,15872,2376, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=10.17  BV=-1.25
    {9996,15872,2544, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=10.27  BV=-1.35
    {9996,15872,2728, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=10.37  BV=-1.45
    {9996,15872,2944, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=10.48  BV=-1.56
    {9996,15872,3152, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=10.58  BV=-1.66
    {20003,15872,1688, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=9.68  BV=-1.76
    {20003,15872,1808, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=9.77  BV=-1.86
    {20003,15872,1936, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=9.87  BV=-1.95
    {20003,15872,2080, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=9.98  BV=-2.06
    {20003,15872,2224, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=10.07  BV=-2.15
    {20003,15872,2384, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=10.17  BV=-2.25
    {20003,15872,2552, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=10.27  BV=-2.35
    {29999,15872,1824, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=9.79  BV=-2.45
    {29999,15872,1952, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=9.88  BV=-2.55
    {29999,15872,2096, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=9.99  BV=-2.65
    {29999,15872,2248, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=10.09  BV=-2.75
    {29999,15872,2408, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=10.19  BV=-2.85
    {40005,15872,1936, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.87  BV=-2.95
    {40005,15872,2072, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.97  BV=-3.05
    {40005,15872,2224, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=10.07  BV=-3.15
    {50001,15872,1904, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=9.85  BV=-3.25
    {50001,15872,2040, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=9.95  BV=-3.35
    {50001,15872,2184, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=10.05  BV=-3.45
    {59997,15872,1952, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=9.88  BV=-3.55
    {59997,15872,2096, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=9.99  BV=-3.65
    {59997,15872,2240, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=10.08  BV=-3.75
    {70004,15872,2056, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=9.96  BV=-3.85
    {70004,15872,2208, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=10.06  BV=-3.95
    {80000,15872,2072, 0, 0, 0},  //TV = 3.64(7619 lines)  AV=2.28  SV=9.97  BV=-4.05
    {80000,15872,2216, 0, 0, 0},  //TV = 3.64(7619 lines)  AV=2.28  SV=10.07  BV=-4.15
    {89996,15872,2112, 0, 0, 0},  //TV = 3.47(8571 lines)  AV=2.28  SV=10.00  BV=-4.25
    {100002,15872,2040, 0, 0, 0},  //TV = 3.32(9524 lines)  AV=2.28  SV=9.95  BV=-4.35
    {100002,15872,2184, 0, 0, 0},  //TV = 3.32(9524 lines)  AV=2.28  SV=10.05  BV=-4.45
    {109998,15872,2128, 0, 0, 0},  //TV = 3.18(10476 lines)  AV=2.28  SV=10.01  BV=-4.55
    {120005,15872,2088, 0, 0, 0},  //TV = 3.06(11429 lines)  AV=2.28  SV=9.98  BV=-4.65
    {130001,15872,2072, 0, 0, 0},  //TV = 2.94(12381 lines)  AV=2.28  SV=9.97  BV=-4.75
    {139997,15872,2056, 0, 0, 0},  //TV = 2.84(13333 lines)  AV=2.28  SV=9.96  BV=-4.85
    {150004,15872,2056, 0, 0, 0},  //TV = 2.74(14286 lines)  AV=2.28  SV=9.96  BV=-4.95
    {160000,15872,2072, 0, 0, 0},  //TV = 2.64(15238 lines)  AV=2.28  SV=9.97  BV=-5.05
    {169996,15872,2088, 0, 0, 0},  //TV = 2.56(16190 lines)  AV=2.28  SV=9.98  BV=-5.15
    {180002,15872,2112, 0, 0, 0},  //TV = 2.47(17143 lines)  AV=2.28  SV=10.00  BV=-5.25
    {189998,15872,2144, 0, 0, 0},  //TV = 2.40(18095 lines)  AV=2.28  SV=10.02  BV=-5.35
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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

static strAETable g_AE_SceneTable6 =
{
    AETABLE_SCENE_INDEX6, //eAETableID
    158, //u4TotalIndex
    20, //i4StrobeTrigerBV
    104, //i4MaxBV
    -54, //i4MinBV
    90, //i4EffectiveMaxBV
    0, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_AUTO, //ISO
    sAEScene6PLineTable_60Hz,
    sAEScene6PLineTable_50Hz,
    NULL,
};
static strEvPline sAEScene7PLineTable_60Hz =
{
{
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {494,1264,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.30  BV=7.95
    {494,1360,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.41  BV=7.85
    {494,1456,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.51  BV=7.75
    {494,1568,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.61  BV=7.64
    {494,1680,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.71  BV=7.54
    {494,1808,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.82  BV=7.44
    {494,1936,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.92  BV=7.34
    {494,2064,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.01  BV=7.25
    {494,2208,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.11  BV=7.15
    {494,2368,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.21  BV=7.05
    {494,2544,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.31  BV=6.95
    {494,2720,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.41  BV=6.85
    {494,2928,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.52  BV=6.74
    {494,3136,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.61  BV=6.64
    {494,3344,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.71  BV=6.55
    {494,3584,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.81  BV=6.45
    {494,3840,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.91  BV=6.35
    {494,4144,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.02  BV=6.24
    {494,4432,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.11  BV=6.14
    {494,4752,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.21  BV=6.04
    {494,5088,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.31  BV=5.95
    {494,5440,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.41  BV=5.85
    {494,5872,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.52  BV=5.74
    {494,6288,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.62  BV=5.64
    {494,6720,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.71  BV=5.54
    {494,7200,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.81  BV=5.44
    {494,7712,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.91  BV=5.35
    {494,8272,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.01  BV=5.24
    {494,8848,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.11  BV=5.15
    {494,9488,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.21  BV=5.05
    {494,10160,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.31  BV=4.95
    {494,10880,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.41  BV=4.85
    {494,11664,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.51  BV=4.75
    {494,12496,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.61  BV=4.65
    {494,13376,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.71  BV=4.55
    {494,14432,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.82  BV=4.44
    {494,15472,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.92  BV=4.34
    {494,15872,1064, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.01  BV=4.25
    {494,15872,1144, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.11  BV=4.14
    {494,15872,1224, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.21  BV=4.05
    {494,15872,1312, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.31  BV=3.95
    {494,15872,1408, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.41  BV=3.84
    {494,15872,1504, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.51  BV=3.75
    {494,15872,1616, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.61  BV=3.65
    {494,15872,1728, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.71  BV=3.55
    {494,15872,1856, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.81  BV=3.45
    {494,15872,1992, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.91  BV=3.34
    {494,15872,2128, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=10.01  BV=3.25
    {494,15872,2288, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=10.11  BV=3.14
    {494,15872,2448, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=10.21  BV=3.05
    {494,15872,2624, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=10.31  BV=2.95
    {494,15872,2816, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=10.41  BV=2.84
    {494,15872,3016, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=10.51  BV=2.75
    {494,15872,3232, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=10.61  BV=2.65
    {494,15872,3464, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=10.71  BV=2.55
    {494,15872,3712, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=10.81  BV=2.45
    {494,15872,3976, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=10.91  BV=2.35
    {494,15872,4264, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=11.01  BV=2.25
    {536,15872,4200, 0, 0, 0},  //TV = 10.87(51 lines)  AV=2.28  SV=10.99  BV=2.15
    {567,15872,4264, 0, 0, 0},  //TV = 10.78(54 lines)  AV=2.28  SV=11.01  BV=2.05
    {609,15872,4256, 0, 0, 0},  //TV = 10.68(58 lines)  AV=2.28  SV=11.01  BV=1.95
    {662,15872,4200, 0, 0, 0},  //TV = 10.56(63 lines)  AV=2.28  SV=10.99  BV=1.85
    {704,15872,4240, 0, 0, 0},  //TV = 10.47(67 lines)  AV=2.28  SV=11.00  BV=1.74
    {756,15872,4232, 0, 0, 0},  //TV = 10.37(72 lines)  AV=2.28  SV=11.00  BV=1.64
    {809,15872,4240, 0, 0, 0},  //TV = 10.27(77 lines)  AV=2.28  SV=11.00  BV=1.54
    {872,15872,4200, 0, 0, 0},  //TV = 10.16(83 lines)  AV=2.28  SV=10.99  BV=1.45
    {935,15872,4216, 0, 0, 0},  //TV = 10.06(89 lines)  AV=2.28  SV=11.00  BV=1.34
    {998,15872,4232, 0, 0, 0},  //TV = 9.97(95 lines)  AV=2.28  SV=11.00  BV=1.24
    {1071,15872,4208, 0, 0, 0},  //TV = 9.87(102 lines)  AV=2.28  SV=10.99  BV=1.15
    {1145,15872,4224, 0, 0, 0},  //TV = 9.77(109 lines)  AV=2.28  SV=11.00  BV=1.05
    {1229,15872,4232, 0, 0, 0},  //TV = 9.67(117 lines)  AV=2.28  SV=11.00  BV=0.94
    {1313,15872,4224, 0, 0, 0},  //TV = 9.57(125 lines)  AV=2.28  SV=11.00  BV=0.85
    {1407,15872,4240, 0, 0, 0},  //TV = 9.47(134 lines)  AV=2.28  SV=11.00  BV=0.74
    {1512,15872,4216, 0, 0, 0},  //TV = 9.37(144 lines)  AV=2.28  SV=11.00  BV=0.65
    {1617,15872,4240, 0, 0, 0},  //TV = 9.27(154 lines)  AV=2.28  SV=11.00  BV=0.54
    {1733,15872,4224, 0, 0, 0},  //TV = 9.17(165 lines)  AV=2.28  SV=11.00  BV=0.45
    {1869,15872,4216, 0, 0, 0},  //TV = 9.06(178 lines)  AV=2.28  SV=11.00  BV=0.34
    {1995,15872,4224, 0, 0, 0},  //TV = 8.97(190 lines)  AV=2.28  SV=11.00  BV=0.25
    {2132,15872,4224, 0, 0, 0},  //TV = 8.87(203 lines)  AV=2.28  SV=11.00  BV=0.15
    {2289,15872,4232, 0, 0, 0},  //TV = 8.77(218 lines)  AV=2.28  SV=11.00  BV=0.04
    {2457,15872,4216, 0, 0, 0},  //TV = 8.67(234 lines)  AV=2.28  SV=11.00  BV=-0.05
    {2625,15872,4224, 0, 0, 0},  //TV = 8.57(250 lines)  AV=2.28  SV=11.00  BV=-0.15
    {2825,15872,4232, 0, 0, 0},  //TV = 8.47(269 lines)  AV=2.28  SV=11.00  BV=-0.26
    {3024,15872,4224, 0, 0, 0},  //TV = 8.37(288 lines)  AV=2.28  SV=11.00  BV=-0.35
    {3234,15872,4232, 0, 0, 0},  //TV = 8.27(308 lines)  AV=2.28  SV=11.00  BV=-0.45
    {3465,15872,4224, 0, 0, 0},  //TV = 8.17(330 lines)  AV=2.28  SV=11.00  BV=-0.55
    {3738,15872,4216, 0, 0, 0},  //TV = 8.06(356 lines)  AV=2.28  SV=11.00  BV=-0.66
    {4001,15872,4216, 0, 0, 0},  //TV = 7.97(381 lines)  AV=2.28  SV=11.00  BV=-0.76
    {4274,15872,4224, 0, 0, 0},  //TV = 7.87(407 lines)  AV=2.28  SV=11.00  BV=-0.85
    {4578,15872,4224, 0, 0, 0},  //TV = 7.77(436 lines)  AV=2.28  SV=11.00  BV=-0.95
    {4904,15872,4224, 0, 0, 0},  //TV = 7.67(467 lines)  AV=2.28  SV=11.00  BV=-1.05
    {5250,15872,4224, 0, 0, 0},  //TV = 7.57(500 lines)  AV=2.28  SV=11.00  BV=-1.15
    {5660,15872,4224, 0, 0, 0},  //TV = 7.46(539 lines)  AV=2.28  SV=11.00  BV=-1.26
    {6059,15872,4224, 0, 0, 0},  //TV = 7.37(577 lines)  AV=2.28  SV=11.00  BV=-1.36
    {6489,15872,4224, 0, 0, 0},  //TV = 7.27(618 lines)  AV=2.28  SV=11.00  BV=-1.46
    {6951,15872,4224, 0, 0, 0},  //TV = 7.17(662 lines)  AV=2.28  SV=11.00  BV=-1.56
    {7445,15872,4224, 0, 0, 0},  //TV = 7.07(709 lines)  AV=2.28  SV=11.00  BV=-1.65
    {7980,15872,4224, 0, 0, 0},  //TV = 6.97(760 lines)  AV=2.28  SV=11.00  BV=-1.75
    {8337,15872,4328, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=11.03  BV=-1.85
    {8337,15872,4640, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=11.13  BV=-1.95
    {8337,15872,4968, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=11.23  BV=-2.05
    {8337,15872,5320, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=11.33  BV=-2.15
    {8337,15872,5704, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=11.43  BV=-2.25
    {8337,15872,6112, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=11.53  BV=-2.35
    {16664,15872,3272, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=10.63  BV=-2.45
    {16664,15872,3504, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=10.73  BV=-2.55
    {16664,15872,3784, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=10.84  BV=-2.66
    {16664,15872,4056, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=10.94  BV=-2.76
    {16664,15872,4344, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=11.04  BV=-2.86
    {16664,15872,4656, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=11.14  BV=-2.96
    {16664,15872,4992, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=11.24  BV=-3.06
    {25001,15872,3560, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=10.75  BV=-3.15
    {25001,15872,3816, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=10.85  BV=-3.26
    {25001,15872,4088, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=10.95  BV=-3.35
    {25001,15872,4384, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=11.05  BV=-3.46
    {25001,15872,4696, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=11.15  BV=-3.55
    {33327,15872,3776, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=10.84  BV=-3.65
    {33327,15872,4048, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=10.94  BV=-3.76
    {33327,15872,4336, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=11.04  BV=-3.85
    {33327,15872,4648, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=11.14  BV=-3.95
    {41664,15872,3984, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=10.91  BV=-4.05
    {41664,15872,4264, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=11.01  BV=-4.15
    {41664,15872,4576, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=11.11  BV=-4.25
    {50001,15872,4080, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=10.95  BV=-4.35
    {50001,15872,4376, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=11.05  BV=-4.45
    {58328,15872,4024, 0, 0, 0},  //TV = 4.10(5555 lines)  AV=2.28  SV=10.93  BV=-4.55
    {58328,15872,4312, 0, 0, 0},  //TV = 4.10(5555 lines)  AV=2.28  SV=11.03  BV=-4.65
    {66665,15872,4040, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=10.93  BV=-4.75
    {66665,15872,4328, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=11.03  BV=-4.85
    {75002,15872,4128, 0, 0, 0},  //TV = 3.74(7143 lines)  AV=2.28  SV=10.97  BV=-4.95
    {75002,15872,4424, 0, 0, 0},  //TV = 3.74(7143 lines)  AV=2.28  SV=11.07  BV=-5.05
    {83328,15872,4264, 0, 0, 0},  //TV = 3.59(7936 lines)  AV=2.28  SV=11.01  BV=-5.15
    {91665,15872,4152, 0, 0, 0},  //TV = 3.45(8730 lines)  AV=2.28  SV=10.97  BV=-5.25
    {99992,15872,4080, 0, 0, 0},  //TV = 3.32(9523 lines)  AV=2.28  SV=10.95  BV=-5.35
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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

static strEvPline sAEScene7PLineTable_50Hz =
{
{
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {494,1264,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.30  BV=7.95
    {494,1360,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.41  BV=7.85
    {494,1456,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.51  BV=7.75
    {494,1568,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.61  BV=7.64
    {494,1680,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.71  BV=7.54
    {494,1808,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.82  BV=7.44
    {494,1936,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.92  BV=7.34
    {494,2064,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.01  BV=7.25
    {494,2208,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.11  BV=7.15
    {494,2368,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.21  BV=7.05
    {494,2544,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.31  BV=6.95
    {494,2720,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.41  BV=6.85
    {494,2928,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.52  BV=6.74
    {494,3136,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.61  BV=6.64
    {494,3344,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.71  BV=6.55
    {494,3584,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.81  BV=6.45
    {494,3840,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.91  BV=6.35
    {494,4144,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.02  BV=6.24
    {494,4432,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.11  BV=6.14
    {494,4752,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.21  BV=6.04
    {494,5088,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.31  BV=5.95
    {494,5440,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.41  BV=5.85
    {494,5872,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.52  BV=5.74
    {494,6288,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.62  BV=5.64
    {494,6720,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.71  BV=5.54
    {494,7200,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.81  BV=5.44
    {494,7712,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.91  BV=5.35
    {494,8272,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.01  BV=5.24
    {494,8848,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.11  BV=5.15
    {494,9488,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.21  BV=5.05
    {494,10160,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.31  BV=4.95
    {494,10880,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.41  BV=4.85
    {494,11664,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.51  BV=4.75
    {494,12496,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.61  BV=4.65
    {494,13376,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.71  BV=4.55
    {494,14432,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.82  BV=4.44
    {494,15472,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.92  BV=4.34
    {494,15872,1064, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.01  BV=4.25
    {494,15872,1144, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.11  BV=4.14
    {494,15872,1224, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.21  BV=4.05
    {494,15872,1312, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.31  BV=3.95
    {494,15872,1408, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.41  BV=3.84
    {494,15872,1504, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.51  BV=3.75
    {494,15872,1616, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.61  BV=3.65
    {494,15872,1728, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.71  BV=3.55
    {494,15872,1856, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.81  BV=3.45
    {494,15872,1992, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.91  BV=3.34
    {494,15872,2128, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=10.01  BV=3.25
    {494,15872,2288, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=10.11  BV=3.14
    {494,15872,2448, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=10.21  BV=3.05
    {494,15872,2624, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=10.31  BV=2.95
    {494,15872,2816, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=10.41  BV=2.84
    {494,15872,3016, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=10.51  BV=2.75
    {494,15872,3232, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=10.61  BV=2.65
    {494,15872,3464, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=10.71  BV=2.55
    {494,15872,3712, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=10.81  BV=2.45
    {494,15872,3976, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=10.91  BV=2.35
    {494,15872,4264, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=11.01  BV=2.25
    {536,15872,4200, 0, 0, 0},  //TV = 10.87(51 lines)  AV=2.28  SV=10.99  BV=2.15
    {567,15872,4264, 0, 0, 0},  //TV = 10.78(54 lines)  AV=2.28  SV=11.01  BV=2.05
    {609,15872,4256, 0, 0, 0},  //TV = 10.68(58 lines)  AV=2.28  SV=11.01  BV=1.95
    {662,15872,4200, 0, 0, 0},  //TV = 10.56(63 lines)  AV=2.28  SV=10.99  BV=1.85
    {704,15872,4240, 0, 0, 0},  //TV = 10.47(67 lines)  AV=2.28  SV=11.00  BV=1.74
    {756,15872,4232, 0, 0, 0},  //TV = 10.37(72 lines)  AV=2.28  SV=11.00  BV=1.64
    {809,15872,4240, 0, 0, 0},  //TV = 10.27(77 lines)  AV=2.28  SV=11.00  BV=1.54
    {872,15872,4200, 0, 0, 0},  //TV = 10.16(83 lines)  AV=2.28  SV=10.99  BV=1.45
    {935,15872,4216, 0, 0, 0},  //TV = 10.06(89 lines)  AV=2.28  SV=11.00  BV=1.34
    {998,15872,4232, 0, 0, 0},  //TV = 9.97(95 lines)  AV=2.28  SV=11.00  BV=1.24
    {1071,15872,4208, 0, 0, 0},  //TV = 9.87(102 lines)  AV=2.28  SV=10.99  BV=1.15
    {1145,15872,4224, 0, 0, 0},  //TV = 9.77(109 lines)  AV=2.28  SV=11.00  BV=1.05
    {1229,15872,4232, 0, 0, 0},  //TV = 9.67(117 lines)  AV=2.28  SV=11.00  BV=0.94
    {1313,15872,4224, 0, 0, 0},  //TV = 9.57(125 lines)  AV=2.28  SV=11.00  BV=0.85
    {1407,15872,4240, 0, 0, 0},  //TV = 9.47(134 lines)  AV=2.28  SV=11.00  BV=0.74
    {1512,15872,4216, 0, 0, 0},  //TV = 9.37(144 lines)  AV=2.28  SV=11.00  BV=0.65
    {1617,15872,4240, 0, 0, 0},  //TV = 9.27(154 lines)  AV=2.28  SV=11.00  BV=0.54
    {1733,15872,4224, 0, 0, 0},  //TV = 9.17(165 lines)  AV=2.28  SV=11.00  BV=0.45
    {1869,15872,4216, 0, 0, 0},  //TV = 9.06(178 lines)  AV=2.28  SV=11.00  BV=0.34
    {1995,15872,4224, 0, 0, 0},  //TV = 8.97(190 lines)  AV=2.28  SV=11.00  BV=0.25
    {2132,15872,4224, 0, 0, 0},  //TV = 8.87(203 lines)  AV=2.28  SV=11.00  BV=0.15
    {2289,15872,4232, 0, 0, 0},  //TV = 8.77(218 lines)  AV=2.28  SV=11.00  BV=0.04
    {2457,15872,4216, 0, 0, 0},  //TV = 8.67(234 lines)  AV=2.28  SV=11.00  BV=-0.05
    {2625,15872,4224, 0, 0, 0},  //TV = 8.57(250 lines)  AV=2.28  SV=11.00  BV=-0.15
    {2825,15872,4232, 0, 0, 0},  //TV = 8.47(269 lines)  AV=2.28  SV=11.00  BV=-0.26
    {3024,15872,4224, 0, 0, 0},  //TV = 8.37(288 lines)  AV=2.28  SV=11.00  BV=-0.35
    {3234,15872,4232, 0, 0, 0},  //TV = 8.27(308 lines)  AV=2.28  SV=11.00  BV=-0.45
    {3465,15872,4224, 0, 0, 0},  //TV = 8.17(330 lines)  AV=2.28  SV=11.00  BV=-0.55
    {3738,15872,4216, 0, 0, 0},  //TV = 8.06(356 lines)  AV=2.28  SV=11.00  BV=-0.66
    {4001,15872,4216, 0, 0, 0},  //TV = 7.97(381 lines)  AV=2.28  SV=11.00  BV=-0.76
    {4274,15872,4224, 0, 0, 0},  //TV = 7.87(407 lines)  AV=2.28  SV=11.00  BV=-0.85
    {4578,15872,4224, 0, 0, 0},  //TV = 7.77(436 lines)  AV=2.28  SV=11.00  BV=-0.95
    {4904,15872,4224, 0, 0, 0},  //TV = 7.67(467 lines)  AV=2.28  SV=11.00  BV=-1.05
    {5250,15872,4224, 0, 0, 0},  //TV = 7.57(500 lines)  AV=2.28  SV=11.00  BV=-1.15
    {5660,15872,4224, 0, 0, 0},  //TV = 7.46(539 lines)  AV=2.28  SV=11.00  BV=-1.26
    {6059,15872,4224, 0, 0, 0},  //TV = 7.37(577 lines)  AV=2.28  SV=11.00  BV=-1.36
    {6489,15872,4224, 0, 0, 0},  //TV = 7.27(618 lines)  AV=2.28  SV=11.00  BV=-1.46
    {6951,15872,4224, 0, 0, 0},  //TV = 7.17(662 lines)  AV=2.28  SV=11.00  BV=-1.56
    {7445,15872,4224, 0, 0, 0},  //TV = 7.07(709 lines)  AV=2.28  SV=11.00  BV=-1.65
    {7980,15872,4224, 0, 0, 0},  //TV = 6.97(760 lines)  AV=2.28  SV=11.00  BV=-1.75
    {8547,15872,4224, 0, 0, 0},  //TV = 6.87(814 lines)  AV=2.28  SV=11.00  BV=-1.85
    {9156,15872,4224, 0, 0, 0},  //TV = 6.77(872 lines)  AV=2.28  SV=11.00  BV=-1.95
    {9807,15872,4224, 0, 0, 0},  //TV = 6.67(934 lines)  AV=2.28  SV=11.00  BV=-2.05
    {9996,15872,4440, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=11.07  BV=-2.15
    {9996,15872,4760, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=11.17  BV=-2.25
    {9996,15872,5096, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=11.27  BV=-2.35
    {9996,15872,5464, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=11.37  BV=-2.45
    {9996,15872,5888, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=11.48  BV=-2.56
    {9996,15872,6312, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=11.58  BV=-2.66
    {20003,15872,3376, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=10.68  BV=-2.76
    {20003,15872,3616, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=10.77  BV=-2.86
    {20003,15872,3880, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=10.88  BV=-2.96
    {20003,15872,4152, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=10.97  BV=-3.06
    {20003,15872,4456, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=11.08  BV=-3.16
    {20003,15872,4768, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=11.17  BV=-3.25
    {20003,15872,5112, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=11.27  BV=-3.36
    {29999,15872,3648, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=10.79  BV=-3.45
    {29999,15872,3912, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=10.89  BV=-3.55
    {29999,15872,4192, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=10.99  BV=-3.65
    {29999,15872,4496, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=11.09  BV=-3.75
    {29999,15872,4816, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=11.19  BV=-3.85
    {40005,15872,3872, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=10.87  BV=-3.95
    {40005,15872,4144, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=10.97  BV=-4.05
    {40005,15872,4448, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=11.07  BV=-4.15
    {50001,15872,3808, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=10.85  BV=-4.25
    {50001,15872,4080, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=10.95  BV=-4.35
    {50001,15872,4376, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=11.05  BV=-4.45
    {59997,15872,3912, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=10.89  BV=-4.55
    {59997,15872,4192, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=10.99  BV=-4.65
    {59997,15872,4488, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=11.09  BV=-4.75
    {70004,15872,4120, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=10.96  BV=-4.85
    {70004,15872,4416, 0, 0, 0},  //TV = 3.84(6667 lines)  AV=2.28  SV=11.06  BV=-4.95
    {80000,15872,4144, 0, 0, 0},  //TV = 3.64(7619 lines)  AV=2.28  SV=10.97  BV=-5.05
    {80000,15872,4440, 0, 0, 0},  //TV = 3.64(7619 lines)  AV=2.28  SV=11.07  BV=-5.15
    {89996,15872,4232, 0, 0, 0},  //TV = 3.47(8571 lines)  AV=2.28  SV=11.00  BV=-5.25
    {100002,15872,4080, 0, 0, 0},  //TV = 3.32(9524 lines)  AV=2.28  SV=10.95  BV=-5.35
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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

static strAETable g_AE_SceneTable7 =
{
    AETABLE_SCENE_INDEX7, //eAETableID
    158, //u4TotalIndex
    20, //i4StrobeTrigerBV
    104, //i4MaxBV
    -54, //i4MinBV
    90, //i4EffectiveMaxBV
    0, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_AUTO, //ISO
    sAEScene7PLineTable_60Hz,
    sAEScene7PLineTable_50Hz,
    NULL,
};
static strEvPline sAEScene8PLineTable_60Hz =
{
{
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {494,1264,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.30  BV=7.95
    {494,1360,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.41  BV=7.85
    {494,1456,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.51  BV=7.75
    {494,1568,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.61  BV=7.64
    {494,1680,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.71  BV=7.54
    {494,1808,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.82  BV=7.44
    {494,1936,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.92  BV=7.34
    {494,2064,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.01  BV=7.25
    {494,2208,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.11  BV=7.15
    {494,2368,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.21  BV=7.05
    {494,2544,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.31  BV=6.95
    {494,2720,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.41  BV=6.85
    {494,2928,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.52  BV=6.74
    {494,3136,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.61  BV=6.64
    {494,3344,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.71  BV=6.55
    {494,3584,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.81  BV=6.45
    {494,3840,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.91  BV=6.35
    {494,4144,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.02  BV=6.24
    {494,4432,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.11  BV=6.14
    {494,4752,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.21  BV=6.04
    {494,5088,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.31  BV=5.95
    {494,5440,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.41  BV=5.85
    {494,5872,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.52  BV=5.74
    {494,6288,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.62  BV=5.64
    {494,6720,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.71  BV=5.54
    {494,7200,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.81  BV=5.44
    {494,7712,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.91  BV=5.35
    {494,8272,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.01  BV=5.24
    {494,8848,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.11  BV=5.15
    {494,9488,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.21  BV=5.05
    {494,10160,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.31  BV=4.95
    {494,10880,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.41  BV=4.85
    {494,11664,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.51  BV=4.75
    {494,12496,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.61  BV=4.65
    {494,13376,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.71  BV=4.55
    {494,14432,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.82  BV=4.44
    {494,15472,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.92  BV=4.34
    {494,15872,1064, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.01  BV=4.25
    {494,15872,1144, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.11  BV=4.14
    {494,15872,1224, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.21  BV=4.05
    {494,15872,1312, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.31  BV=3.95
    {494,15872,1408, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.41  BV=3.84
    {494,15872,1504, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.51  BV=3.75
    {494,15872,1616, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.61  BV=3.65
    {494,15872,1728, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.71  BV=3.55
    {494,15872,1856, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.81  BV=3.45
    {494,15872,1992, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.91  BV=3.34
    {494,15872,2128, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=10.01  BV=3.25
    {494,15872,2288, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=10.11  BV=3.14
    {494,15872,2448, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=10.21  BV=3.05
    {494,15872,2624, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=10.31  BV=2.95
    {494,15872,2816, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=10.41  BV=2.84
    {494,15872,3016, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=10.51  BV=2.75
    {494,15872,3232, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=10.61  BV=2.65
    {494,15872,3464, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=10.71  BV=2.55
    {494,15872,3712, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=10.81  BV=2.45
    {494,15872,3976, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=10.91  BV=2.35
    {494,15872,4264, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=11.01  BV=2.25
    {494,15872,4568, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=11.11  BV=2.15
    {494,15872,4896, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=11.21  BV=2.05
    {494,15872,5248, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=11.31  BV=1.95
    {494,15872,5624, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=11.41  BV=1.85
    {494,15872,6024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=11.51  BV=1.75
    {494,15872,6456, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=11.61  BV=1.65
    {494,15872,6920, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=11.71  BV=1.55
    {494,15872,7416, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=11.81  BV=1.45
    {494,15872,7952, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=11.91  BV=1.35
    {494,15872,8520, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=12.01  BV=1.25
    {536,15872,8408, 0, 0, 0},  //TV = 10.87(51 lines)  AV=2.28  SV=11.99  BV=1.15
    {567,15872,8528, 0, 0, 0},  //TV = 10.78(54 lines)  AV=2.28  SV=12.01  BV=1.05
    {609,15872,8520, 0, 0, 0},  //TV = 10.68(58 lines)  AV=2.28  SV=12.01  BV=0.95
    {662,15872,8400, 0, 0, 0},  //TV = 10.56(63 lines)  AV=2.28  SV=11.99  BV=0.85
    {704,15872,8480, 0, 0, 0},  //TV = 10.47(67 lines)  AV=2.28  SV=12.00  BV=0.74
    {756,15872,8464, 0, 0, 0},  //TV = 10.37(72 lines)  AV=2.28  SV=12.00  BV=0.64
    {809,15872,8480, 0, 0, 0},  //TV = 10.27(77 lines)  AV=2.28  SV=12.00  BV=0.54
    {872,15872,8400, 0, 0, 0},  //TV = 10.16(83 lines)  AV=2.28  SV=11.99  BV=0.45
    {935,15872,8432, 0, 0, 0},  //TV = 10.06(89 lines)  AV=2.28  SV=12.00  BV=0.34
    {998,15872,8464, 0, 0, 0},  //TV = 9.97(95 lines)  AV=2.28  SV=12.00  BV=0.24
    {1071,15872,8424, 0, 0, 0},  //TV = 9.87(102 lines)  AV=2.28  SV=11.99  BV=0.15
    {1145,15872,8448, 0, 0, 0},  //TV = 9.77(109 lines)  AV=2.28  SV=12.00  BV=0.05
    {1229,15872,8464, 0, 0, 0},  //TV = 9.67(117 lines)  AV=2.28  SV=12.00  BV=-0.06
    {1313,15872,8456, 0, 0, 0},  //TV = 9.57(125 lines)  AV=2.28  SV=12.00  BV=-0.15
    {1407,15872,8480, 0, 0, 0},  //TV = 9.47(134 lines)  AV=2.28  SV=12.00  BV=-0.26
    {1512,15872,8432, 0, 0, 0},  //TV = 9.37(144 lines)  AV=2.28  SV=12.00  BV=-0.35
    {1617,15872,8480, 0, 0, 0},  //TV = 9.27(154 lines)  AV=2.28  SV=12.00  BV=-0.46
    {1733,15872,8448, 0, 0, 0},  //TV = 9.17(165 lines)  AV=2.28  SV=12.00  BV=-0.55
    {1869,15872,8432, 0, 0, 0},  //TV = 9.06(178 lines)  AV=2.28  SV=12.00  BV=-0.66
    {1995,15872,8448, 0, 0, 0},  //TV = 8.97(190 lines)  AV=2.28  SV=12.00  BV=-0.75
    {2132,15872,8448, 0, 0, 0},  //TV = 8.87(203 lines)  AV=2.28  SV=12.00  BV=-0.85
    {2289,15872,8472, 0, 0, 0},  //TV = 8.77(218 lines)  AV=2.28  SV=12.00  BV=-0.96
    {2457,15872,8440, 0, 0, 0},  //TV = 8.67(234 lines)  AV=2.28  SV=12.00  BV=-1.05
    {2625,15872,8456, 0, 0, 0},  //TV = 8.57(250 lines)  AV=2.28  SV=12.00  BV=-1.15
    {2825,15872,8464, 0, 0, 0},  //TV = 8.47(269 lines)  AV=2.28  SV=12.00  BV=-1.26
    {3024,15872,8456, 0, 0, 0},  //TV = 8.37(288 lines)  AV=2.28  SV=12.00  BV=-1.36
    {3234,15872,8464, 0, 0, 0},  //TV = 8.27(308 lines)  AV=2.28  SV=12.00  BV=-1.45
    {3465,15872,8448, 0, 0, 0},  //TV = 8.17(330 lines)  AV=2.28  SV=12.00  BV=-1.55
    {3738,15872,8440, 0, 0, 0},  //TV = 8.06(356 lines)  AV=2.28  SV=12.00  BV=-1.66
    {4001,15872,8440, 0, 0, 0},  //TV = 7.97(381 lines)  AV=2.28  SV=12.00  BV=-1.76
    {4274,15872,8456, 0, 0, 0},  //TV = 7.87(407 lines)  AV=2.28  SV=12.00  BV=-1.85
    {4578,15872,8456, 0, 0, 0},  //TV = 7.77(436 lines)  AV=2.28  SV=12.00  BV=-1.95
    {4904,15872,8448, 0, 0, 0},  //TV = 7.67(467 lines)  AV=2.28  SV=12.00  BV=-2.05
    {5250,15872,8448, 0, 0, 0},  //TV = 7.57(500 lines)  AV=2.28  SV=12.00  BV=-2.15
    {5618,15872,8456, 0, 0, 0},  //TV = 7.48(535 lines)  AV=2.28  SV=12.00  BV=-2.25
    {6059,15872,8456, 0, 0, 0},  //TV = 7.37(577 lines)  AV=2.28  SV=12.00  BV=-2.36
    {6489,15872,8456, 0, 0, 0},  //TV = 7.27(618 lines)  AV=2.28  SV=12.00  BV=-2.46
    {6951,15872,8456, 0, 0, 0},  //TV = 7.17(662 lines)  AV=2.28  SV=12.00  BV=-2.56
    {7445,15872,8456, 0, 0, 0},  //TV = 7.07(709 lines)  AV=2.28  SV=12.00  BV=-2.66
    {7980,15872,8448, 0, 0, 0},  //TV = 6.97(760 lines)  AV=2.28  SV=12.00  BV=-2.75
    {8337,15872,8664, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=12.04  BV=-2.85
    {8337,15872,9280, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=12.13  BV=-2.95
    {8337,15872,9944, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=12.23  BV=-3.05
    {8337,15872,10648, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=12.33  BV=-3.15
    {8337,15872,11408, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=12.43  BV=-3.25
    {8337,15872,12224, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=12.53  BV=-3.35
    {16664,15872,6552, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=11.63  BV=-3.45
    {16664,15872,7072, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=11.74  BV=-3.56
    {16664,15872,7576, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=11.84  BV=-3.66
    {16664,15872,8112, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=11.94  BV=-3.76
    {16664,15872,8696, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=12.04  BV=-3.86
    {16664,15872,9312, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=12.14  BV=-3.96
    {16664,15872,9984, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=12.24  BV=-4.06
    {25001,15872,7128, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=11.75  BV=-4.16
    {25001,15872,7640, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=11.85  BV=-4.26
    {25001,15872,8184, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=11.95  BV=-4.36
    {25001,15872,8768, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=12.05  BV=-4.46
    {25001,15872,9400, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=12.15  BV=-4.56
    {33327,15872,7552, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=11.84  BV=-4.65
    {33327,15872,8096, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=11.94  BV=-4.76
    {33327,15872,8672, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=12.04  BV=-4.85
    {33327,15872,9296, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=12.14  BV=-4.95
    {41664,15872,7968, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=11.91  BV=-5.05
    {41664,15872,8536, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=12.01  BV=-5.15
    {41664,15872,9152, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=12.11  BV=-5.25
    {50001,15872,8168, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=11.95  BV=-5.35
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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

static strEvPline sAEScene8PLineTable_50Hz =
{
{
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {494,1264,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.30  BV=7.95
    {494,1360,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.41  BV=7.85
    {494,1456,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.51  BV=7.75
    {494,1568,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.61  BV=7.64
    {494,1680,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.71  BV=7.54
    {494,1808,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.82  BV=7.44
    {494,1936,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=5.92  BV=7.34
    {494,2064,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.01  BV=7.25
    {494,2208,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.11  BV=7.15
    {494,2368,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.21  BV=7.05
    {494,2544,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.31  BV=6.95
    {494,2720,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.41  BV=6.85
    {494,2928,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.52  BV=6.74
    {494,3136,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.61  BV=6.64
    {494,3344,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.71  BV=6.55
    {494,3584,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.81  BV=6.45
    {494,3840,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=6.91  BV=6.35
    {494,4144,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.02  BV=6.24
    {494,4432,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.11  BV=6.14
    {494,4752,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.21  BV=6.04
    {494,5088,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.31  BV=5.95
    {494,5440,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.41  BV=5.85
    {494,5872,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.52  BV=5.74
    {494,6288,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.62  BV=5.64
    {494,6720,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.71  BV=5.54
    {494,7200,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.81  BV=5.44
    {494,7712,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=7.91  BV=5.35
    {494,8272,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.01  BV=5.24
    {494,8848,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.11  BV=5.15
    {494,9488,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.21  BV=5.05
    {494,10160,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.31  BV=4.95
    {494,10880,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.41  BV=4.85
    {494,11664,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.51  BV=4.75
    {494,12496,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.61  BV=4.65
    {494,13376,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.71  BV=4.55
    {494,14432,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.82  BV=4.44
    {494,15472,1024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=8.92  BV=4.34
    {494,15872,1064, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.01  BV=4.25
    {494,15872,1144, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.11  BV=4.14
    {494,15872,1224, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.21  BV=4.05
    {494,15872,1312, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.31  BV=3.95
    {494,15872,1408, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.41  BV=3.84
    {494,15872,1504, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.51  BV=3.75
    {494,15872,1616, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.61  BV=3.65
    {494,15872,1728, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.71  BV=3.55
    {494,15872,1856, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.81  BV=3.45
    {494,15872,1992, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=9.91  BV=3.34
    {494,15872,2128, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=10.01  BV=3.25
    {494,15872,2288, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=10.11  BV=3.14
    {494,15872,2448, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=10.21  BV=3.05
    {494,15872,2624, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=10.31  BV=2.95
    {494,15872,2816, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=10.41  BV=2.84
    {494,15872,3016, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=10.51  BV=2.75
    {494,15872,3232, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=10.61  BV=2.65
    {494,15872,3464, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=10.71  BV=2.55
    {494,15872,3712, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=10.81  BV=2.45
    {494,15872,3976, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=10.91  BV=2.35
    {494,15872,4264, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=11.01  BV=2.25
    {494,15872,4568, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=11.11  BV=2.15
    {494,15872,4896, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=11.21  BV=2.05
    {494,15872,5248, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=11.31  BV=1.95
    {494,15872,5624, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=11.41  BV=1.85
    {494,15872,6024, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=11.51  BV=1.75
    {494,15872,6456, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=11.61  BV=1.65
    {494,15872,6920, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=11.71  BV=1.55
    {494,15872,7416, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=11.81  BV=1.45
    {494,15872,7952, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=11.91  BV=1.35
    {494,15872,8520, 0, 0, 0},  //TV = 10.98(47 lines)  AV=2.28  SV=12.01  BV=1.25
    {536,15872,8408, 0, 0, 0},  //TV = 10.87(51 lines)  AV=2.28  SV=11.99  BV=1.15
    {567,15872,8528, 0, 0, 0},  //TV = 10.78(54 lines)  AV=2.28  SV=12.01  BV=1.05
    {609,15872,8520, 0, 0, 0},  //TV = 10.68(58 lines)  AV=2.28  SV=12.01  BV=0.95
    {662,15872,8400, 0, 0, 0},  //TV = 10.56(63 lines)  AV=2.28  SV=11.99  BV=0.85
    {704,15872,8480, 0, 0, 0},  //TV = 10.47(67 lines)  AV=2.28  SV=12.00  BV=0.74
    {756,15872,8464, 0, 0, 0},  //TV = 10.37(72 lines)  AV=2.28  SV=12.00  BV=0.64
    {809,15872,8480, 0, 0, 0},  //TV = 10.27(77 lines)  AV=2.28  SV=12.00  BV=0.54
    {872,15872,8400, 0, 0, 0},  //TV = 10.16(83 lines)  AV=2.28  SV=11.99  BV=0.45
    {935,15872,8432, 0, 0, 0},  //TV = 10.06(89 lines)  AV=2.28  SV=12.00  BV=0.34
    {998,15872,8464, 0, 0, 0},  //TV = 9.97(95 lines)  AV=2.28  SV=12.00  BV=0.24
    {1071,15872,8424, 0, 0, 0},  //TV = 9.87(102 lines)  AV=2.28  SV=11.99  BV=0.15
    {1145,15872,8448, 0, 0, 0},  //TV = 9.77(109 lines)  AV=2.28  SV=12.00  BV=0.05
    {1229,15872,8464, 0, 0, 0},  //TV = 9.67(117 lines)  AV=2.28  SV=12.00  BV=-0.06
    {1313,15872,8456, 0, 0, 0},  //TV = 9.57(125 lines)  AV=2.28  SV=12.00  BV=-0.15
    {1407,15872,8480, 0, 0, 0},  //TV = 9.47(134 lines)  AV=2.28  SV=12.00  BV=-0.26
    {1512,15872,8432, 0, 0, 0},  //TV = 9.37(144 lines)  AV=2.28  SV=12.00  BV=-0.35
    {1617,15872,8480, 0, 0, 0},  //TV = 9.27(154 lines)  AV=2.28  SV=12.00  BV=-0.46
    {1733,15872,8448, 0, 0, 0},  //TV = 9.17(165 lines)  AV=2.28  SV=12.00  BV=-0.55
    {1869,15872,8432, 0, 0, 0},  //TV = 9.06(178 lines)  AV=2.28  SV=12.00  BV=-0.66
    {1995,15872,8448, 0, 0, 0},  //TV = 8.97(190 lines)  AV=2.28  SV=12.00  BV=-0.75
    {2132,15872,8448, 0, 0, 0},  //TV = 8.87(203 lines)  AV=2.28  SV=12.00  BV=-0.85
    {2289,15872,8472, 0, 0, 0},  //TV = 8.77(218 lines)  AV=2.28  SV=12.00  BV=-0.96
    {2457,15872,8440, 0, 0, 0},  //TV = 8.67(234 lines)  AV=2.28  SV=12.00  BV=-1.05
    {2625,15872,8456, 0, 0, 0},  //TV = 8.57(250 lines)  AV=2.28  SV=12.00  BV=-1.15
    {2825,15872,8464, 0, 0, 0},  //TV = 8.47(269 lines)  AV=2.28  SV=12.00  BV=-1.26
    {3024,15872,8456, 0, 0, 0},  //TV = 8.37(288 lines)  AV=2.28  SV=12.00  BV=-1.36
    {3234,15872,8464, 0, 0, 0},  //TV = 8.27(308 lines)  AV=2.28  SV=12.00  BV=-1.45
    {3465,15872,8448, 0, 0, 0},  //TV = 8.17(330 lines)  AV=2.28  SV=12.00  BV=-1.55
    {3738,15872,8440, 0, 0, 0},  //TV = 8.06(356 lines)  AV=2.28  SV=12.00  BV=-1.66
    {4001,15872,8440, 0, 0, 0},  //TV = 7.97(381 lines)  AV=2.28  SV=12.00  BV=-1.76
    {4274,15872,8456, 0, 0, 0},  //TV = 7.87(407 lines)  AV=2.28  SV=12.00  BV=-1.85
    {4578,15872,8456, 0, 0, 0},  //TV = 7.77(436 lines)  AV=2.28  SV=12.00  BV=-1.95
    {4904,15872,8448, 0, 0, 0},  //TV = 7.67(467 lines)  AV=2.28  SV=12.00  BV=-2.05
    {5250,15872,8448, 0, 0, 0},  //TV = 7.57(500 lines)  AV=2.28  SV=12.00  BV=-2.15
    {5618,15872,8456, 0, 0, 0},  //TV = 7.48(535 lines)  AV=2.28  SV=12.00  BV=-2.25
    {6059,15872,8456, 0, 0, 0},  //TV = 7.37(577 lines)  AV=2.28  SV=12.00  BV=-2.36
    {6489,15872,8456, 0, 0, 0},  //TV = 7.27(618 lines)  AV=2.28  SV=12.00  BV=-2.46
    {6951,15872,8456, 0, 0, 0},  //TV = 7.17(662 lines)  AV=2.28  SV=12.00  BV=-2.56
    {7445,15872,8456, 0, 0, 0},  //TV = 7.07(709 lines)  AV=2.28  SV=12.00  BV=-2.66
    {7980,15872,8448, 0, 0, 0},  //TV = 6.97(760 lines)  AV=2.28  SV=12.00  BV=-2.75
    {8547,15872,8448, 0, 0, 0},  //TV = 6.87(814 lines)  AV=2.28  SV=12.00  BV=-2.85
    {9156,15872,8448, 0, 0, 0},  //TV = 6.77(872 lines)  AV=2.28  SV=12.00  BV=-2.95
    {9807,15872,8448, 0, 0, 0},  //TV = 6.67(934 lines)  AV=2.28  SV=12.00  BV=-3.05
    {9996,15872,8880, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=12.07  BV=-3.15
    {9996,15872,9520, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=12.17  BV=-3.25
    {9996,15872,10192, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=12.27  BV=-3.35
    {9996,15872,10928, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=12.37  BV=-3.45
    {9996,15872,11704, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=12.47  BV=-3.55
    {9996,15872,12624, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=12.58  BV=-3.66
    {20003,15872,6760, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=11.68  BV=-3.76
    {20003,15872,7240, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=11.78  BV=-3.86
    {20003,15872,7760, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=11.88  BV=-3.96
    {20003,15872,8312, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=11.98  BV=-4.06
    {20003,15872,8912, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=12.08  BV=-4.16
    {20003,15872,9544, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=12.17  BV=-4.26
    {20003,15872,10232, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=12.27  BV=-4.36
    {29999,15872,7312, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=11.79  BV=-4.46
    {29999,15872,7832, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=11.89  BV=-4.56
    {29999,15872,8392, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=11.99  BV=-4.66
    {29999,15872,8992, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=12.09  BV=-4.75
    {29999,15872,9640, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=12.19  BV=-4.86
    {40005,15872,7744, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=11.87  BV=-4.95
    {40005,15872,8296, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=11.97  BV=-5.05
    {40005,15872,8896, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=12.07  BV=-5.15
    {50001,15872,7624, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=11.85  BV=-5.25
    {50001,15872,8168, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=11.95  BV=-5.35
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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

static strAETable g_AE_SceneTable8 =
{
    AETABLE_SCENE_INDEX8, //eAETableID
    158, //u4TotalIndex
    20, //i4StrobeTrigerBV
    104, //i4MaxBV
    -54, //i4MinBV
    90, //i4EffectiveMaxBV
    0, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_AUTO, //ISO
    sAEScene8PLineTable_60Hz,
    sAEScene8PLineTable_50Hz,
    NULL,
};
static strEvPline sAEScene9PLineTable_60Hz =
{
{
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {525,1200,1024, 0, 0, 0},  //TV = 10.90(50 lines)  AV=2.28  SV=5.23  BV=7.94
    {557,1200,1024, 0, 0, 0},  //TV = 10.81(53 lines)  AV=2.28  SV=5.23  BV=7.86
    {599,1200,1024, 0, 0, 0},  //TV = 10.71(57 lines)  AV=2.28  SV=5.23  BV=7.75
    {641,1200,1024, 0, 0, 0},  //TV = 10.61(61 lines)  AV=2.28  SV=5.23  BV=7.65
    {683,1216,1024, 0, 0, 0},  //TV = 10.52(65 lines)  AV=2.28  SV=5.25  BV=7.54
    {735,1200,1024, 0, 0, 0},  //TV = 10.41(70 lines)  AV=2.28  SV=5.23  BV=7.46
    {788,1200,1024, 0, 0, 0},  //TV = 10.31(75 lines)  AV=2.28  SV=5.23  BV=7.36
    {840,1216,1024, 0, 0, 0},  //TV = 10.22(80 lines)  AV=2.28  SV=5.25  BV=7.24
    {903,1200,1024, 0, 0, 0},  //TV = 10.11(86 lines)  AV=2.28  SV=5.23  BV=7.16
    {977,1200,1024, 0, 0, 0},  //TV = 10.00(93 lines)  AV=2.28  SV=5.23  BV=7.05
    {1040,1200,1024, 0, 0, 0},  //TV = 9.91(99 lines)  AV=2.28  SV=5.23  BV=6.96
    {1113,1200,1024, 0, 0, 0},  //TV = 9.81(106 lines)  AV=2.28  SV=5.23  BV=6.86
    {1197,1200,1024, 0, 0, 0},  //TV = 9.71(114 lines)  AV=2.28  SV=5.23  BV=6.75
    {1292,1200,1024, 0, 0, 0},  //TV = 9.60(123 lines)  AV=2.28  SV=5.23  BV=6.64
    {1376,1200,1024, 0, 0, 0},  //TV = 9.51(131 lines)  AV=2.28  SV=5.23  BV=6.55
    {1481,1200,1024, 0, 0, 0},  //TV = 9.40(141 lines)  AV=2.28  SV=5.23  BV=6.45
    {1575,1200,1024, 0, 0, 0},  //TV = 9.31(150 lines)  AV=2.28  SV=5.23  BV=6.36
    {1701,1200,1024, 0, 0, 0},  //TV = 9.20(162 lines)  AV=2.28  SV=5.23  BV=6.25
    {1827,1200,1024, 0, 0, 0},  //TV = 9.10(174 lines)  AV=2.28  SV=5.23  BV=6.14
    {1953,1200,1024, 0, 0, 0},  //TV = 9.00(186 lines)  AV=2.28  SV=5.23  BV=6.05
    {2090,1200,1024, 0, 0, 0},  //TV = 8.90(199 lines)  AV=2.28  SV=5.23  BV=5.95
    {2258,1200,1024, 0, 0, 0},  //TV = 8.79(215 lines)  AV=2.28  SV=5.23  BV=5.84
    {2415,1200,1024, 0, 0, 0},  //TV = 8.69(230 lines)  AV=2.28  SV=5.23  BV=5.74
    {2583,1200,1024, 0, 0, 0},  //TV = 8.60(246 lines)  AV=2.28  SV=5.23  BV=5.64
    {2762,1200,1024, 0, 0, 0},  //TV = 8.50(263 lines)  AV=2.28  SV=5.23  BV=5.55
    {2972,1200,1024, 0, 0, 0},  //TV = 8.39(283 lines)  AV=2.28  SV=5.23  BV=5.44
    {3182,1200,1024, 0, 0, 0},  //TV = 8.30(303 lines)  AV=2.28  SV=5.23  BV=5.34
    {3402,1200,1024, 0, 0, 0},  //TV = 8.20(324 lines)  AV=2.28  SV=5.23  BV=5.25
    {3644,1200,1024, 0, 0, 0},  //TV = 8.10(347 lines)  AV=2.28  SV=5.23  BV=5.15
    {3896,1200,1024, 0, 0, 0},  //TV = 8.00(371 lines)  AV=2.28  SV=5.23  BV=5.05
    {4179,1200,1024, 0, 0, 0},  //TV = 7.90(398 lines)  AV=2.28  SV=5.23  BV=4.95
    {4505,1200,1024, 0, 0, 0},  //TV = 7.79(429 lines)  AV=2.28  SV=5.23  BV=4.84
    {4820,1200,1024, 0, 0, 0},  //TV = 7.70(459 lines)  AV=2.28  SV=5.23  BV=4.74
    {5156,1200,1024, 0, 0, 0},  //TV = 7.60(491 lines)  AV=2.28  SV=5.23  BV=4.65
    {5523,1200,1024, 0, 0, 0},  //TV = 7.50(526 lines)  AV=2.28  SV=5.23  BV=4.55
    {5912,1200,1024, 0, 0, 0},  //TV = 7.40(563 lines)  AV=2.28  SV=5.23  BV=4.45
    {6332,1200,1024, 0, 0, 0},  //TV = 7.30(603 lines)  AV=2.28  SV=5.23  BV=4.35
    {6836,1200,1024, 0, 0, 0},  //TV = 7.19(651 lines)  AV=2.28  SV=5.23  BV=4.24
    {7319,1200,1024, 0, 0, 0},  //TV = 7.09(697 lines)  AV=2.28  SV=5.23  BV=4.14
    {7833,1200,1024, 0, 0, 0},  //TV = 7.00(746 lines)  AV=2.28  SV=5.23  BV=4.04
    {8338,1200,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.23  BV=3.95
    {8338,1296,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.34  BV=3.84
    {8338,1376,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.43  BV=3.75
    {8338,1488,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.54  BV=3.64
    {8338,1584,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.63  BV=3.55
    {8338,1696,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.73  BV=3.45
    {8338,1824,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.83  BV=3.35
    {8338,1952,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.93  BV=3.25
    {8338,2096,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.03  BV=3.15
    {8338,2240,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.13  BV=3.05
    {16664,1200,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.23  BV=2.95
    {16664,1296,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.34  BV=2.84
    {16664,1376,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.43  BV=2.76
    {16664,1488,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.54  BV=2.64
    {16664,1584,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.63  BV=2.55
    {16664,1696,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.73  BV=2.45
    {16664,1824,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.83  BV=2.35
    {16664,1952,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.93  BV=2.25
    {16664,2096,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.03  BV=2.15
    {16664,2256,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.14  BV=2.04
    {16664,2416,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.24  BV=1.94
    {25001,1728,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.75  BV=1.84
    {25001,1840,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.85  BV=1.75
    {25001,1984,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.95  BV=1.64
    {25001,2112,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=6.04  BV=1.55
    {25001,2272,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=6.15  BV=1.45
    {25001,2432,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=6.25  BV=1.35
    {25001,2608,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=6.35  BV=1.25
    {25001,2784,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=6.44  BV=1.15
    {25001,3008,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=6.55  BV=1.04
    {25001,3216,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=6.65  BV=0.95
    {25001,3440,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=6.75  BV=0.85
    {25001,3712,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=6.86  BV=0.74
    {25001,3968,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=6.95  BV=0.64
    {25001,4240,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=7.05  BV=0.55
    {25001,4544,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=7.15  BV=0.45
    {25001,4864,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=7.25  BV=0.35
    {25001,5216,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=7.35  BV=0.25
    {25001,5584,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=7.45  BV=0.15
    {25001,6016,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=7.55  BV=0.04
    {25001,6448,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=7.65  BV=-0.06
    {25001,6896,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=7.75  BV=-0.15
    {25001,7392,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=7.85  BV=-0.25
    {25001,7920,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=7.95  BV=-0.35
    {25001,8480,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=8.05  BV=-0.45
    {25001,9088,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=8.15  BV=-0.55
    {25001,9728,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=8.25  BV=-0.65
    {25001,10432,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=8.35  BV=-0.75
    {25001,11168,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=8.45  BV=-0.85
    {25001,11968,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=8.55  BV=-0.95
    {25001,12832,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=8.65  BV=-1.05
    {25001,13840,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=8.76  BV=-1.16
    {25001,14832,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=8.86  BV=-1.26
    {25001,15872,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=8.95  BV=-1.36
    {25001,15872,1096, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=9.05  BV=-1.46
    {25001,15872,1176, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=9.15  BV=-1.56
    {25001,15872,1256, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=9.25  BV=-1.65
    {25001,15872,1344, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=9.35  BV=-1.75
    {25001,15872,1440, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=9.45  BV=-1.85
    {25001,15872,1544, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=9.55  BV=-1.95
    {25001,15872,1656, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=9.65  BV=-2.05
    {25001,15872,1776, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=9.75  BV=-2.15
    {25001,15872,1904, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=9.85  BV=-2.25
    {25001,15872,2040, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=9.95  BV=-2.35
    {25001,15872,2184, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=10.05  BV=-2.45
    {25001,15872,2344, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=10.15  BV=-2.55
    {25001,15872,2512, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=10.25  BV=-2.65
    {25001,15872,2696, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=10.35  BV=-2.75
    {25001,15872,2888, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=10.45  BV=-2.85
    {25001,15872,2968, 0, 0, 0},  /* TV = 5.32(2381 lines)  AV=2.28  SV=10.49  BV=-2.89 */
    {25001,15872,2968, 0, 0, 0},  /* TV = 5.32(2381 lines)  AV=2.28  SV=10.49  BV=-2.89 */
    {25001,15872,2968, 0, 0, 0},  /* TV = 5.32(2381 lines)  AV=2.28  SV=10.49  BV=-2.89 */
    {25001,15872,2968, 0, 0, 0},  /* TV = 5.32(2381 lines)  AV=2.28  SV=10.49  BV=-2.89 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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

static strEvPline sAEScene9PLineTable_50Hz =
{
{
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {525,1200,1024, 0, 0, 0},  //TV = 10.90(50 lines)  AV=2.28  SV=5.23  BV=7.94
    {557,1200,1024, 0, 0, 0},  //TV = 10.81(53 lines)  AV=2.28  SV=5.23  BV=7.86
    {599,1200,1024, 0, 0, 0},  //TV = 10.71(57 lines)  AV=2.28  SV=5.23  BV=7.75
    {641,1200,1024, 0, 0, 0},  //TV = 10.61(61 lines)  AV=2.28  SV=5.23  BV=7.65
    {683,1216,1024, 0, 0, 0},  //TV = 10.52(65 lines)  AV=2.28  SV=5.25  BV=7.54
    {735,1200,1024, 0, 0, 0},  //TV = 10.41(70 lines)  AV=2.28  SV=5.23  BV=7.46
    {788,1200,1024, 0, 0, 0},  //TV = 10.31(75 lines)  AV=2.28  SV=5.23  BV=7.36
    {840,1216,1024, 0, 0, 0},  //TV = 10.22(80 lines)  AV=2.28  SV=5.25  BV=7.24
    {903,1200,1024, 0, 0, 0},  //TV = 10.11(86 lines)  AV=2.28  SV=5.23  BV=7.16
    {977,1200,1024, 0, 0, 0},  //TV = 10.00(93 lines)  AV=2.28  SV=5.23  BV=7.05
    {1040,1200,1024, 0, 0, 0},  //TV = 9.91(99 lines)  AV=2.28  SV=5.23  BV=6.96
    {1113,1200,1024, 0, 0, 0},  //TV = 9.81(106 lines)  AV=2.28  SV=5.23  BV=6.86
    {1197,1200,1024, 0, 0, 0},  //TV = 9.71(114 lines)  AV=2.28  SV=5.23  BV=6.75
    {1292,1200,1024, 0, 0, 0},  //TV = 9.60(123 lines)  AV=2.28  SV=5.23  BV=6.64
    {1376,1200,1024, 0, 0, 0},  //TV = 9.51(131 lines)  AV=2.28  SV=5.23  BV=6.55
    {1481,1200,1024, 0, 0, 0},  //TV = 9.40(141 lines)  AV=2.28  SV=5.23  BV=6.45
    {1575,1200,1024, 0, 0, 0},  //TV = 9.31(150 lines)  AV=2.28  SV=5.23  BV=6.36
    {1701,1200,1024, 0, 0, 0},  //TV = 9.20(162 lines)  AV=2.28  SV=5.23  BV=6.25
    {1827,1200,1024, 0, 0, 0},  //TV = 9.10(174 lines)  AV=2.28  SV=5.23  BV=6.14
    {1953,1200,1024, 0, 0, 0},  //TV = 9.00(186 lines)  AV=2.28  SV=5.23  BV=6.05
    {2090,1200,1024, 0, 0, 0},  //TV = 8.90(199 lines)  AV=2.28  SV=5.23  BV=5.95
    {2258,1200,1024, 0, 0, 0},  //TV = 8.79(215 lines)  AV=2.28  SV=5.23  BV=5.84
    {2415,1200,1024, 0, 0, 0},  //TV = 8.69(230 lines)  AV=2.28  SV=5.23  BV=5.74
    {2583,1200,1024, 0, 0, 0},  //TV = 8.60(246 lines)  AV=2.28  SV=5.23  BV=5.64
    {2762,1200,1024, 0, 0, 0},  //TV = 8.50(263 lines)  AV=2.28  SV=5.23  BV=5.55
    {2972,1200,1024, 0, 0, 0},  //TV = 8.39(283 lines)  AV=2.28  SV=5.23  BV=5.44
    {3182,1200,1024, 0, 0, 0},  //TV = 8.30(303 lines)  AV=2.28  SV=5.23  BV=5.34
    {3402,1200,1024, 0, 0, 0},  //TV = 8.20(324 lines)  AV=2.28  SV=5.23  BV=5.25
    {3644,1200,1024, 0, 0, 0},  //TV = 8.10(347 lines)  AV=2.28  SV=5.23  BV=5.15
    {3896,1200,1024, 0, 0, 0},  //TV = 8.00(371 lines)  AV=2.28  SV=5.23  BV=5.05
    {4179,1200,1024, 0, 0, 0},  //TV = 7.90(398 lines)  AV=2.28  SV=5.23  BV=4.95
    {4505,1200,1024, 0, 0, 0},  //TV = 7.79(429 lines)  AV=2.28  SV=5.23  BV=4.84
    {4820,1200,1024, 0, 0, 0},  //TV = 7.70(459 lines)  AV=2.28  SV=5.23  BV=4.74
    {5156,1200,1024, 0, 0, 0},  //TV = 7.60(491 lines)  AV=2.28  SV=5.23  BV=4.65
    {5523,1200,1024, 0, 0, 0},  //TV = 7.50(526 lines)  AV=2.28  SV=5.23  BV=4.55
    {5912,1200,1024, 0, 0, 0},  //TV = 7.40(563 lines)  AV=2.28  SV=5.23  BV=4.45
    {6332,1200,1024, 0, 0, 0},  //TV = 7.30(603 lines)  AV=2.28  SV=5.23  BV=4.35
    {6836,1200,1024, 0, 0, 0},  //TV = 7.19(651 lines)  AV=2.28  SV=5.23  BV=4.24
    {7319,1200,1024, 0, 0, 0},  //TV = 7.09(697 lines)  AV=2.28  SV=5.23  BV=4.14
    {7833,1200,1024, 0, 0, 0},  //TV = 7.00(746 lines)  AV=2.28  SV=5.23  BV=4.04
    {8400,1200,1024, 0, 0, 0},  //TV = 6.90(800 lines)  AV=2.28  SV=5.23  BV=3.94
    {8999,1200,1024, 0, 0, 0},  //TV = 6.80(857 lines)  AV=2.28  SV=5.23  BV=3.84
    {9639,1200,1024, 0, 0, 0},  //TV = 6.70(918 lines)  AV=2.28  SV=5.23  BV=3.74
    {9996,1232,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.27  BV=3.65
    {9996,1328,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.38  BV=3.54
    {9996,1424,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.48  BV=3.44
    {9996,1520,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.57  BV=3.35
    {9996,1632,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.67  BV=3.25
    {9996,1744,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.77  BV=3.15
    {9996,1872,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.87  BV=3.05
    {9996,2000,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.97  BV=2.95
    {9996,2144,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.07  BV=2.85
    {9996,2304,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.17  BV=2.75
    {9996,2464,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.27  BV=2.65
    {9996,2656,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.38  BV=2.54
    {9996,2832,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.47  BV=2.45
    {9996,3040,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.57  BV=2.35
    {9996,3264,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.67  BV=2.25
    {20003,1744,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.77  BV=2.15
    {20003,1872,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.87  BV=2.05
    {20003,2016,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.98  BV=1.94
    {20003,2160,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.08  BV=1.84
    {20003,2304,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.17  BV=1.75
    {20003,2480,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.28  BV=1.64
    {29999,1760,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.78  BV=1.55
    {29999,1904,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.89  BV=1.44
    {29999,2032,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.99  BV=1.35
    {29999,2176,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.09  BV=1.25
    {29999,2320,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.18  BV=1.15
    {29999,2496,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.29  BV=1.05
    {29999,2672,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.38  BV=0.95
    {29999,2864,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.48  BV=0.85
    {29999,3072,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.58  BV=0.75
    {29999,3312,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.69  BV=0.64
    {29999,3536,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.79  BV=0.55
    {29999,3792,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.89  BV=0.45
    {29999,4048,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.98  BV=0.35
    {29999,4336,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.08  BV=0.25
    {29999,4656,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.18  BV=0.15
    {29999,5008,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.29  BV=0.04
    {29999,5376,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.39  BV=-0.06
    {29999,5744,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.49  BV=-0.15
    {29999,6160,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.59  BV=-0.25
    {29999,6592,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.69  BV=-0.35
    {29999,7072,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.79  BV=-0.45
    {29999,7568,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.89  BV=-0.55
    {29999,8112,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.99  BV=-0.65
    {29999,8688,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.08  BV=-0.75
    {29999,9312,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.18  BV=-0.85
    {29999,9968,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.28  BV=-0.95
    {29999,10688,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.38  BV=-1.05
    {29999,11536,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.49  BV=-1.16
    {29999,12352,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.59  BV=-1.26
    {29999,13232,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.69  BV=-1.36
    {29999,14176,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.79  BV=-1.46
    {29999,15200,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.89  BV=-1.56
    {29999,15872,1048, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.99  BV=-1.65
    {29999,15872,1120, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=9.08  BV=-1.75
    {29999,15872,1200, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=9.18  BV=-1.85
    {29999,15872,1288, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=9.29  BV=-1.95
    {29999,15872,1384, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=9.39  BV=-2.05
    {29999,15872,1480, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=9.49  BV=-2.15
    {29999,15872,1584, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=9.58  BV=-2.25
    {29999,15872,1704, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=9.69  BV=-2.35
    {29999,15872,1824, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=9.79  BV=-2.45
    {29999,15872,1952, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=9.88  BV=-2.55
    {29999,15872,2096, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=9.99  BV=-2.65
    {29999,15872,2240, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=10.08  BV=-2.75
    {29999,15872,2408, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=10.19  BV=-2.85
    {29999,15872,2576, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=10.29  BV=-2.95
    {29999,15872,2760, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=10.38  BV=-3.05
    {29999,15872,2960, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=10.49  BV=-3.15
    {29999,15872,2968, 0, 0, 0},  /* TV = 5.06(2857 lines)  AV=2.28  SV=10.49  BV=-3.16 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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

static strAETable g_AE_SceneTable9 =
{
    AETABLE_SCENE_INDEX9, //eAETableID
    137, //u4TotalIndex
    20, //i4StrobeTrigerBV
    104, //i4MaxBV
    -32, //i4MinBV
    90, //i4EffectiveMaxBV
    0, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_AUTO, //ISO
    sAEScene9PLineTable_60Hz,
    sAEScene9PLineTable_50Hz,
    NULL,
};
static strEvPline sAEScene14PLineTable_60Hz =
{
{
    {95,1184,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.21  BV=10.43
    {105,1136,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.15  BV=10.34
    {105,1248,1024, 0, 0, 0},  /* TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21 */
    {116,1200,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.23  BV=10.12
    {126,1184,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.21  BV=10.02
    {137,1152,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.17  BV=9.94
    {147,1136,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.15  BV=9.86
    {158,1152,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.17  BV=9.73
    {168,1152,1024, 0, 0, 0},  //TV = 12.54(16 lines)  AV=2.28  SV=5.17  BV=9.64
    {179,1184,1024, 0, 0, 0},  /* TV = 12.45(17 lines)  AV=2.28  SV=5.21  BV=9.51 */
    {200,1136,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.15  BV=9.41
    {210,1136,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.15  BV=9.34
    {221,1152,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.17  BV=9.25
    {231,1184,1024, 0, 0, 0},  //TV = 12.08(22 lines)  AV=2.28  SV=5.21  BV=9.15
    {252,1152,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.17  BV=9.06
    {273,1152,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.17  BV=8.94
    {294,1136,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.15  BV=8.86
    {315,1136,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.15  BV=8.76
    {336,1152,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.17  BV=8.64
    {357,1152,1024, 0, 0, 0},  //TV = 11.45(34 lines)  AV=2.28  SV=5.17  BV=8.56
    {389,1136,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.15  BV=8.45
    {410,1152,1024, 0, 0, 0},  //TV = 11.25(39 lines)  AV=2.28  SV=5.17  BV=8.36
    {441,1152,1024, 0, 0, 0},  //TV = 11.15(42 lines)  AV=2.28  SV=5.17  BV=8.25
    {483,1136,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.15  BV=8.14
    {504,1152,1024, 0, 0, 0},  //TV = 10.95(48 lines)  AV=2.28  SV=5.17  BV=8.06
    {546,1152,1024, 0, 0, 0},  //TV = 10.84(52 lines)  AV=2.28  SV=5.17  BV=7.94
    {588,1136,1024, 0, 0, 0},  //TV = 10.73(56 lines)  AV=2.28  SV=5.15  BV=7.86
    {630,1136,1024, 0, 0, 0},  //TV = 10.63(60 lines)  AV=2.28  SV=5.15  BV=7.76
    {672,1152,1024, 0, 0, 0},  //TV = 10.54(64 lines)  AV=2.28  SV=5.17  BV=7.64
    {725,1136,1024, 0, 0, 0},  //TV = 10.43(69 lines)  AV=2.28  SV=5.15  BV=7.55
    {777,1136,1024, 0, 0, 0},  //TV = 10.33(74 lines)  AV=2.28  SV=5.15  BV=7.46
    {830,1136,1024, 0, 0, 0},  //TV = 10.23(79 lines)  AV=2.28  SV=5.15  BV=7.36
    {893,1136,1024, 0, 0, 0},  //TV = 10.13(85 lines)  AV=2.28  SV=5.15  BV=7.25
    {956,1136,1024, 0, 0, 0},  //TV = 10.03(91 lines)  AV=2.28  SV=5.15  BV=7.16
    {1029,1136,1024, 0, 0, 0},  //TV = 9.92(98 lines)  AV=2.28  SV=5.15  BV=7.05
    {1103,1136,1024, 0, 0, 0},  //TV = 9.82(105 lines)  AV=2.28  SV=5.15  BV=6.95
    {1187,1136,1024, 0, 0, 0},  //TV = 9.72(113 lines)  AV=2.28  SV=5.15  BV=6.84
    {1260,1136,1024, 0, 0, 0},  //TV = 9.63(120 lines)  AV=2.28  SV=5.15  BV=6.76
    {1355,1136,1024, 0, 0, 0},  //TV = 9.53(129 lines)  AV=2.28  SV=5.15  BV=6.65
    {1460,1136,1024, 0, 0, 0},  //TV = 9.42(139 lines)  AV=2.28  SV=5.15  BV=6.55
    {1554,1136,1024, 0, 0, 0},  //TV = 9.33(148 lines)  AV=2.28  SV=5.15  BV=6.46
    {1680,1136,1024, 0, 0, 0},  //TV = 9.22(160 lines)  AV=2.28  SV=5.15  BV=6.34
    {1796,1136,1024, 0, 0, 0},  //TV = 9.12(171 lines)  AV=2.28  SV=5.15  BV=6.25
    {1922,1136,1024, 0, 0, 0},  //TV = 9.02(183 lines)  AV=2.28  SV=5.15  BV=6.15
    {2048,1136,1024, 0, 0, 0},  //TV = 8.93(195 lines)  AV=2.28  SV=5.15  BV=6.06
    {2205,1136,1024, 0, 0, 0},  //TV = 8.83(210 lines)  AV=2.28  SV=5.15  BV=5.95
    {2373,1136,1024, 0, 0, 0},  //TV = 8.72(226 lines)  AV=2.28  SV=5.15  BV=5.84
    {2541,1136,1024, 0, 0, 0},  //TV = 8.62(242 lines)  AV=2.28  SV=5.15  BV=5.75
    {2720,1136,1024, 0, 0, 0},  //TV = 8.52(259 lines)  AV=2.28  SV=5.15  BV=5.65
    {2930,1136,1024, 0, 0, 0},  //TV = 8.41(279 lines)  AV=2.28  SV=5.15  BV=5.54
    {3119,1136,1024, 0, 0, 0},  //TV = 8.32(297 lines)  AV=2.28  SV=5.15  BV=5.45
    {3360,1136,1024, 0, 0, 0},  //TV = 8.22(320 lines)  AV=2.28  SV=5.15  BV=5.34
    {3591,1136,1024, 0, 0, 0},  //TV = 8.12(342 lines)  AV=2.28  SV=5.15  BV=5.25
    {3843,1136,1024, 0, 0, 0},  //TV = 8.02(366 lines)  AV=2.28  SV=5.15  BV=5.15
    {4116,1136,1024, 0, 0, 0},  //TV = 7.92(392 lines)  AV=2.28  SV=5.15  BV=5.05
    {4442,1136,1024, 0, 0, 0},  //TV = 7.81(423 lines)  AV=2.28  SV=5.15  BV=4.94
    {4757,1136,1024, 0, 0, 0},  //TV = 7.72(453 lines)  AV=2.28  SV=5.15  BV=4.84
    {5093,1136,1024, 0, 0, 0},  //TV = 7.62(485 lines)  AV=2.28  SV=5.15  BV=4.74
    {5450,1136,1024, 0, 0, 0},  //TV = 7.52(519 lines)  AV=2.28  SV=5.15  BV=4.64
    {5838,1136,1024, 0, 0, 0},  //TV = 7.42(556 lines)  AV=2.28  SV=5.15  BV=4.55
    {6248,1136,1024, 0, 0, 0},  //TV = 7.32(595 lines)  AV=2.28  SV=5.15  BV=4.45
    {6699,1136,1024, 0, 0, 0},  //TV = 7.22(638 lines)  AV=2.28  SV=5.15  BV=4.35
    {7172,1136,1024, 0, 0, 0},  //TV = 7.12(683 lines)  AV=2.28  SV=5.15  BV=4.25
    {7728,1136,1024, 0, 0, 0},  //TV = 7.02(736 lines)  AV=2.28  SV=5.15  BV=4.14
    {8285,1136,1024, 0, 0, 0},  //TV = 6.92(789 lines)  AV=2.28  SV=5.15  BV=4.04
    {8338,1200,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.23  BV=3.95
    {8338,1296,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.34  BV=3.84
    {8338,1376,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.43  BV=3.75
    {8338,1472,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.52  BV=3.66
    {8338,1584,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.63  BV=3.55
    {8338,1696,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.73  BV=3.45
    {8338,1824,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.83  BV=3.35
    {8338,1952,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.93  BV=3.25
    {8338,2096,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.03  BV=3.15
    {8338,2240,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.13  BV=3.05
    {16664,1200,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.23  BV=2.95
    {16664,1296,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.34  BV=2.84
    {16664,1376,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.43  BV=2.76
    {16664,1488,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.54  BV=2.64
    {16664,1600,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.64  BV=2.54
    {16664,1696,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.73  BV=2.45
    {16664,1824,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.83  BV=2.35
    {16664,1952,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.93  BV=2.25
    {16664,2096,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.03  BV=2.15
    {16664,2256,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.14  BV=2.04
    {16664,2416,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.24  BV=1.94
    {16664,2592,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.34  BV=1.84
    {16664,2768,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.43  BV=1.75
    {16664,2960,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.53  BV=1.65
    {16664,3168,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.63  BV=1.55
    {16664,3408,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.73  BV=1.45
    {16664,3648,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.83  BV=1.35
    {16664,3904,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=6.93  BV=1.25
    {16664,4208,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=7.04  BV=1.14
    {16664,4512,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=7.14  BV=1.04
    {16664,4832,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=7.24  BV=0.94
    {16664,5168,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=7.34  BV=0.85
    {16664,5536,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=7.43  BV=0.75
    {16664,5920,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=7.53  BV=0.65
    {16664,6352,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=7.63  BV=0.55
    {25001,4544,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=7.15  BV=0.45
    {25001,4864,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=7.25  BV=0.35
    {25001,5216,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=7.35  BV=0.25
    {25001,5584,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=7.45  BV=0.15
    {33327,4496,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.13  BV=0.05
    {33327,4816,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.23  BV=-0.05
    {33327,5152,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.33  BV=-0.15
    {33327,5552,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.44  BV=-0.26
    {33327,5952,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.54  BV=-0.36
    {33327,6368,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.64  BV=-0.45
    {33327,6832,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.74  BV=-0.56
    {33327,7312,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.84  BV=-0.65
    {33327,7840,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.94  BV=-0.75
    {33327,8384,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.03  BV=-0.85
    {33327,8992,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.13  BV=-0.95
    {33327,9632,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.23  BV=-1.05
    {33327,10320,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.33  BV=-1.15
    {33327,11056,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.43  BV=-1.25
    {33327,11840,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.53  BV=-1.35
    {33327,12768,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.64  BV=-1.46
    {33327,13696,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.74  BV=-1.56
    {33327,14672,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.84  BV=-1.66
    {33327,15632,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.93  BV=-1.75
    {33327,15872,1080, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=9.03  BV=-1.85
    {33327,15872,1152, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=9.12  BV=-1.94
    {41664,15488,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=8.92  BV=-2.06
    {41664,15872,1064, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.01  BV=-2.15
    {41664,15872,1136, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.10  BV=-2.24
    {41664,15872,1216, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.20  BV=-2.34
    {41664,15872,1312, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.31  BV=-2.45
    {41664,15872,1400, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.41  BV=-2.55
    {41664,15872,1504, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.51  BV=-2.65
    {50001,15872,1344, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=9.35  BV=-2.75
    {50001,15872,1440, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=9.45  BV=-2.85
    {50001,15872,1552, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=9.55  BV=-2.96
    {58328,15872,1416, 0, 0, 0},  //TV = 4.10(5555 lines)  AV=2.28  SV=9.42  BV=-3.05
    {58328,15872,1520, 0, 0, 0},  //TV = 4.10(5555 lines)  AV=2.28  SV=9.52  BV=-3.15
    {58328,15872,1632, 0, 0, 0},  //TV = 4.10(5555 lines)  AV=2.28  SV=9.63  BV=-3.25
    {58328,15872,1752, 0, 0, 0},  //TV = 4.10(5555 lines)  AV=2.28  SV=9.73  BV=-3.35
    {58328,15872,1872, 0, 0, 0},  //TV = 4.10(5555 lines)  AV=2.28  SV=9.82  BV=-3.45
    {58328,15872,2008, 0, 0, 0},  //TV = 4.10(5555 lines)  AV=2.28  SV=9.93  BV=-3.55
    {58328,15872,2112, 0, 0, 0},  //TV = 4.10(5555 lines)  AV=2.28  SV=10.00  BV=-3.62
    {58328,15872,2112, 0, 0, 0},  /* TV = 4.10(5555 lines)  AV=2.28  SV=10.00  BV=-3.62 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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

static strEvPline sAEScene14PLineTable_50Hz =
{
{
    {95,1184,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.21  BV=10.43
    {105,1136,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.15  BV=10.34
    {105,1248,1024, 0, 0, 0},  /* TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21 */
    {116,1200,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.23  BV=10.12
    {126,1184,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.21  BV=10.02
    {137,1152,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.17  BV=9.94
    {147,1136,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.15  BV=9.86
    {158,1152,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.17  BV=9.73
    {168,1152,1024, 0, 0, 0},  //TV = 12.54(16 lines)  AV=2.28  SV=5.17  BV=9.64
    {179,1184,1024, 0, 0, 0},  /* TV = 12.45(17 lines)  AV=2.28  SV=5.21  BV=9.51 */
    {200,1136,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.15  BV=9.41
    {210,1136,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.15  BV=9.34
    {221,1152,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.17  BV=9.25
    {231,1184,1024, 0, 0, 0},  //TV = 12.08(22 lines)  AV=2.28  SV=5.21  BV=9.15
    {252,1152,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.17  BV=9.06
    {273,1152,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.17  BV=8.94
    {294,1136,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.15  BV=8.86
    {315,1136,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.15  BV=8.76
    {336,1152,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.17  BV=8.64
    {357,1152,1024, 0, 0, 0},  //TV = 11.45(34 lines)  AV=2.28  SV=5.17  BV=8.56
    {389,1136,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.15  BV=8.45
    {410,1152,1024, 0, 0, 0},  //TV = 11.25(39 lines)  AV=2.28  SV=5.17  BV=8.36
    {441,1152,1024, 0, 0, 0},  //TV = 11.15(42 lines)  AV=2.28  SV=5.17  BV=8.25
    {483,1136,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.15  BV=8.14
    {504,1152,1024, 0, 0, 0},  //TV = 10.95(48 lines)  AV=2.28  SV=5.17  BV=8.06
    {546,1152,1024, 0, 0, 0},  //TV = 10.84(52 lines)  AV=2.28  SV=5.17  BV=7.94
    {588,1136,1024, 0, 0, 0},  //TV = 10.73(56 lines)  AV=2.28  SV=5.15  BV=7.86
    {630,1136,1024, 0, 0, 0},  //TV = 10.63(60 lines)  AV=2.28  SV=5.15  BV=7.76
    {672,1152,1024, 0, 0, 0},  //TV = 10.54(64 lines)  AV=2.28  SV=5.17  BV=7.64
    {725,1136,1024, 0, 0, 0},  //TV = 10.43(69 lines)  AV=2.28  SV=5.15  BV=7.55
    {777,1136,1024, 0, 0, 0},  //TV = 10.33(74 lines)  AV=2.28  SV=5.15  BV=7.46
    {830,1136,1024, 0, 0, 0},  //TV = 10.23(79 lines)  AV=2.28  SV=5.15  BV=7.36
    {893,1136,1024, 0, 0, 0},  //TV = 10.13(85 lines)  AV=2.28  SV=5.15  BV=7.25
    {956,1136,1024, 0, 0, 0},  //TV = 10.03(91 lines)  AV=2.28  SV=5.15  BV=7.16
    {1029,1136,1024, 0, 0, 0},  //TV = 9.92(98 lines)  AV=2.28  SV=5.15  BV=7.05
    {1103,1136,1024, 0, 0, 0},  //TV = 9.82(105 lines)  AV=2.28  SV=5.15  BV=6.95
    {1187,1136,1024, 0, 0, 0},  //TV = 9.72(113 lines)  AV=2.28  SV=5.15  BV=6.84
    {1260,1136,1024, 0, 0, 0},  //TV = 9.63(120 lines)  AV=2.28  SV=5.15  BV=6.76
    {1355,1136,1024, 0, 0, 0},  //TV = 9.53(129 lines)  AV=2.28  SV=5.15  BV=6.65
    {1460,1136,1024, 0, 0, 0},  //TV = 9.42(139 lines)  AV=2.28  SV=5.15  BV=6.55
    {1554,1136,1024, 0, 0, 0},  //TV = 9.33(148 lines)  AV=2.28  SV=5.15  BV=6.46
    {1680,1136,1024, 0, 0, 0},  //TV = 9.22(160 lines)  AV=2.28  SV=5.15  BV=6.34
    {1796,1136,1024, 0, 0, 0},  //TV = 9.12(171 lines)  AV=2.28  SV=5.15  BV=6.25
    {1922,1136,1024, 0, 0, 0},  //TV = 9.02(183 lines)  AV=2.28  SV=5.15  BV=6.15
    {2048,1136,1024, 0, 0, 0},  //TV = 8.93(195 lines)  AV=2.28  SV=5.15  BV=6.06
    {2205,1136,1024, 0, 0, 0},  //TV = 8.83(210 lines)  AV=2.28  SV=5.15  BV=5.95
    {2373,1136,1024, 0, 0, 0},  //TV = 8.72(226 lines)  AV=2.28  SV=5.15  BV=5.84
    {2541,1136,1024, 0, 0, 0},  //TV = 8.62(242 lines)  AV=2.28  SV=5.15  BV=5.75
    {2720,1136,1024, 0, 0, 0},  //TV = 8.52(259 lines)  AV=2.28  SV=5.15  BV=5.65
    {2930,1136,1024, 0, 0, 0},  //TV = 8.41(279 lines)  AV=2.28  SV=5.15  BV=5.54
    {3119,1136,1024, 0, 0, 0},  //TV = 8.32(297 lines)  AV=2.28  SV=5.15  BV=5.45
    {3360,1136,1024, 0, 0, 0},  //TV = 8.22(320 lines)  AV=2.28  SV=5.15  BV=5.34
    {3591,1136,1024, 0, 0, 0},  //TV = 8.12(342 lines)  AV=2.28  SV=5.15  BV=5.25
    {3843,1136,1024, 0, 0, 0},  //TV = 8.02(366 lines)  AV=2.28  SV=5.15  BV=5.15
    {4116,1136,1024, 0, 0, 0},  //TV = 7.92(392 lines)  AV=2.28  SV=5.15  BV=5.05
    {4442,1136,1024, 0, 0, 0},  //TV = 7.81(423 lines)  AV=2.28  SV=5.15  BV=4.94
    {4757,1136,1024, 0, 0, 0},  //TV = 7.72(453 lines)  AV=2.28  SV=5.15  BV=4.84
    {5093,1136,1024, 0, 0, 0},  //TV = 7.62(485 lines)  AV=2.28  SV=5.15  BV=4.74
    {5450,1136,1024, 0, 0, 0},  //TV = 7.52(519 lines)  AV=2.28  SV=5.15  BV=4.64
    {5838,1136,1024, 0, 0, 0},  //TV = 7.42(556 lines)  AV=2.28  SV=5.15  BV=4.55
    {6248,1136,1024, 0, 0, 0},  //TV = 7.32(595 lines)  AV=2.28  SV=5.15  BV=4.45
    {6699,1136,1024, 0, 0, 0},  //TV = 7.22(638 lines)  AV=2.28  SV=5.15  BV=4.35
    {7172,1136,1024, 0, 0, 0},  //TV = 7.12(683 lines)  AV=2.28  SV=5.15  BV=4.25
    {7728,1136,1024, 0, 0, 0},  //TV = 7.02(736 lines)  AV=2.28  SV=5.15  BV=4.14
    {8285,1136,1024, 0, 0, 0},  //TV = 6.92(789 lines)  AV=2.28  SV=5.15  BV=4.04
    {8873,1136,1024, 0, 0, 0},  //TV = 6.82(845 lines)  AV=2.28  SV=5.15  BV=3.94
    {9503,1136,1024, 0, 0, 0},  //TV = 6.72(905 lines)  AV=2.28  SV=5.15  BV=3.84
    {9996,1152,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.17  BV=3.75
    {9996,1232,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.27  BV=3.65
    {9996,1328,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.38  BV=3.54
    {9996,1424,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.48  BV=3.44
    {9996,1520,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.57  BV=3.35
    {9996,1632,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.67  BV=3.25
    {9996,1744,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.77  BV=3.15
    {9996,1872,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.87  BV=3.05
    {9996,2000,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.97  BV=2.95
    {9996,2144,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.07  BV=2.85
    {9996,2304,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.17  BV=2.75
    {9996,2464,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.27  BV=2.65
    {9996,2656,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.38  BV=2.54
    {9996,2848,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.48  BV=2.44
    {9996,3040,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.57  BV=2.35
    {9996,3264,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.67  BV=2.25
    {20003,1744,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.77  BV=2.15
    {20003,1872,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.87  BV=2.05
    {20003,2016,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.98  BV=1.94
    {20003,2160,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.08  BV=1.84
    {20003,2304,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.17  BV=1.75
    {20003,2464,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.27  BV=1.65
    {20003,2656,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.38  BV=1.54
    {20003,2848,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.48  BV=1.44
    {20003,3040,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.57  BV=1.35
    {20003,3264,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.67  BV=1.25
    {20003,3504,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.77  BV=1.14
    {20003,3760,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.88  BV=1.04
    {20003,4016,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=6.97  BV=0.95
    {20003,4304,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=7.07  BV=0.85
    {20003,4608,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=7.17  BV=0.75
    {20003,4928,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=7.27  BV=0.65
    {20003,5296,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=7.37  BV=0.55
    {20003,5680,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=7.47  BV=0.45
    {20003,6080,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=7.57  BV=0.35
    {29999,4352,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.09  BV=0.25
    {29999,4656,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.18  BV=0.15
    {29999,4992,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.29  BV=0.05
    {29999,5344,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.38  BV=-0.05
    {29999,5760,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.49  BV=-0.16
    {29999,6176,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.59  BV=-0.26
    {29999,6608,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.69  BV=-0.36
    {29999,7088,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.79  BV=-0.46
    {29999,7584,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.89  BV=-0.55
    {29999,8128,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.99  BV=-0.65
    {29999,8704,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.09  BV=-0.75
    {29999,9328,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.19  BV=-0.85
    {29999,9984,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.29  BV=-0.95
    {29999,10704,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.39  BV=-1.05
    {29999,11456,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.48  BV=-1.15
    {29999,12288,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.58  BV=-1.25
    {29999,13248,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.69  BV=-1.36
    {29999,14192,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.79  BV=-1.46
    {29999,15200,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.89  BV=-1.56
    {29999,15872,1048, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.99  BV=-1.65
    {29999,15872,1120, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=9.08  BV=-1.75
    {29999,15872,1200, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=9.18  BV=-1.85
    {40005,14960,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=8.87  BV=-1.95
    {40005,15872,1040, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=8.98  BV=-2.06
    {40005,15872,1104, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.06  BV=-2.14
    {40005,15872,1184, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.16  BV=-2.24
    {40005,15872,1280, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.28  BV=-2.36
    {40005,15872,1368, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.37  BV=-2.45
    {40005,15872,1456, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.46  BV=-2.54
    {40005,15872,1568, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.57  BV=-2.65
    {50001,15872,1344, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=9.35  BV=-2.75
    {50001,15872,1440, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=9.45  BV=-2.85
    {50001,15872,1552, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=9.55  BV=-2.96
    {59997,15872,1384, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=9.39  BV=-3.05
    {59997,15872,1480, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=9.49  BV=-3.15
    {59997,15872,1584, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=9.58  BV=-3.25
    {59997,15872,1704, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=9.69  BV=-3.35
    {59997,15872,1824, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=9.79  BV=-3.45
    {59997,15872,1952, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=9.88  BV=-3.55
    {59997,15872,2096, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=9.99  BV=-3.65
    {59997,15872,2112, 0, 0, 0},  /* TV = 4.06(5714 lines)  AV=2.28  SV=10.00  BV=-3.66 */
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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

static strAETable g_AE_SceneTable14 =
{
    AETABLE_SCENE_INDEX14, //eAETableID
    143, //u4TotalIndex
    20, //i4StrobeTrigerBV
    105, //i4MaxBV
    -37, //i4MinBV
    0, //i4EffectiveMaxBV
    0, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_AUTO, //ISO
    sAEScene14PLineTable_60Hz,
    sAEScene14PLineTable_50Hz,
    NULL,
};
static strEvPline sAEScene15PLineTable_60Hz =
{
{
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {525,1200,1024, 0, 0, 0},  //TV = 10.90(50 lines)  AV=2.28  SV=5.23  BV=7.94
    {557,1200,1024, 0, 0, 0},  //TV = 10.81(53 lines)  AV=2.28  SV=5.23  BV=7.86
    {599,1200,1024, 0, 0, 0},  //TV = 10.71(57 lines)  AV=2.28  SV=5.23  BV=7.75
    {641,1200,1024, 0, 0, 0},  //TV = 10.61(61 lines)  AV=2.28  SV=5.23  BV=7.65
    {683,1216,1024, 0, 0, 0},  //TV = 10.52(65 lines)  AV=2.28  SV=5.25  BV=7.54
    {735,1200,1024, 0, 0, 0},  //TV = 10.41(70 lines)  AV=2.28  SV=5.23  BV=7.46
    {788,1200,1024, 0, 0, 0},  //TV = 10.31(75 lines)  AV=2.28  SV=5.23  BV=7.36
    {840,1216,1024, 0, 0, 0},  //TV = 10.22(80 lines)  AV=2.28  SV=5.25  BV=7.24
    {903,1200,1024, 0, 0, 0},  //TV = 10.11(86 lines)  AV=2.28  SV=5.23  BV=7.16
    {977,1200,1024, 0, 0, 0},  //TV = 10.00(93 lines)  AV=2.28  SV=5.23  BV=7.05
    {1040,1200,1024, 0, 0, 0},  //TV = 9.91(99 lines)  AV=2.28  SV=5.23  BV=6.96
    {1113,1200,1024, 0, 0, 0},  //TV = 9.81(106 lines)  AV=2.28  SV=5.23  BV=6.86
    {1197,1200,1024, 0, 0, 0},  //TV = 9.71(114 lines)  AV=2.28  SV=5.23  BV=6.75
    {1292,1200,1024, 0, 0, 0},  //TV = 9.60(123 lines)  AV=2.28  SV=5.23  BV=6.64
    {1376,1200,1024, 0, 0, 0},  //TV = 9.51(131 lines)  AV=2.28  SV=5.23  BV=6.55
    {1481,1200,1024, 0, 0, 0},  //TV = 9.40(141 lines)  AV=2.28  SV=5.23  BV=6.45
    {1575,1200,1024, 0, 0, 0},  //TV = 9.31(150 lines)  AV=2.28  SV=5.23  BV=6.36
    {1701,1200,1024, 0, 0, 0},  //TV = 9.20(162 lines)  AV=2.28  SV=5.23  BV=6.25
    {1827,1200,1024, 0, 0, 0},  //TV = 9.10(174 lines)  AV=2.28  SV=5.23  BV=6.14
    {1953,1200,1024, 0, 0, 0},  //TV = 9.00(186 lines)  AV=2.28  SV=5.23  BV=6.05
    {2090,1200,1024, 0, 0, 0},  //TV = 8.90(199 lines)  AV=2.28  SV=5.23  BV=5.95
    {2258,1200,1024, 0, 0, 0},  //TV = 8.79(215 lines)  AV=2.28  SV=5.23  BV=5.84
    {2415,1200,1024, 0, 0, 0},  //TV = 8.69(230 lines)  AV=2.28  SV=5.23  BV=5.74
    {2583,1200,1024, 0, 0, 0},  //TV = 8.60(246 lines)  AV=2.28  SV=5.23  BV=5.64
    {2762,1200,1024, 0, 0, 0},  //TV = 8.50(263 lines)  AV=2.28  SV=5.23  BV=5.55
    {2972,1200,1024, 0, 0, 0},  //TV = 8.39(283 lines)  AV=2.28  SV=5.23  BV=5.44
    {3182,1200,1024, 0, 0, 0},  //TV = 8.30(303 lines)  AV=2.28  SV=5.23  BV=5.34
    {3402,1200,1024, 0, 0, 0},  //TV = 8.20(324 lines)  AV=2.28  SV=5.23  BV=5.25
    {3644,1200,1024, 0, 0, 0},  //TV = 8.10(347 lines)  AV=2.28  SV=5.23  BV=5.15
    {3896,1200,1024, 0, 0, 0},  //TV = 8.00(371 lines)  AV=2.28  SV=5.23  BV=5.05
    {4179,1200,1024, 0, 0, 0},  //TV = 7.90(398 lines)  AV=2.28  SV=5.23  BV=4.95
    {4505,1200,1024, 0, 0, 0},  //TV = 7.79(429 lines)  AV=2.28  SV=5.23  BV=4.84
    {4820,1200,1024, 0, 0, 0},  //TV = 7.70(459 lines)  AV=2.28  SV=5.23  BV=4.74
    {5156,1200,1024, 0, 0, 0},  //TV = 7.60(491 lines)  AV=2.28  SV=5.23  BV=4.65
    {5523,1200,1024, 0, 0, 0},  //TV = 7.50(526 lines)  AV=2.28  SV=5.23  BV=4.55
    {5912,1200,1024, 0, 0, 0},  //TV = 7.40(563 lines)  AV=2.28  SV=5.23  BV=4.45
    {6332,1200,1024, 0, 0, 0},  //TV = 7.30(603 lines)  AV=2.28  SV=5.23  BV=4.35
    {6836,1200,1024, 0, 0, 0},  //TV = 7.19(651 lines)  AV=2.28  SV=5.23  BV=4.24
    {7319,1200,1024, 0, 0, 0},  //TV = 7.09(697 lines)  AV=2.28  SV=5.23  BV=4.14
    {7833,1200,1024, 0, 0, 0},  //TV = 7.00(746 lines)  AV=2.28  SV=5.23  BV=4.04
    {8338,1200,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.23  BV=3.95
    {8338,1296,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.34  BV=3.84
    {8338,1376,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.43  BV=3.75
    {8338,1488,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.54  BV=3.64
    {8338,1584,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.63  BV=3.55
    {8338,1696,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.73  BV=3.45
    {8338,1824,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.83  BV=3.35
    {8338,1952,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.93  BV=3.25
    {8338,2096,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.03  BV=3.15
    {8338,2240,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.13  BV=3.05
    {16664,1200,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.23  BV=2.95
    {16664,1296,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.34  BV=2.84
    {16664,1376,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.43  BV=2.76
    {16664,1488,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.54  BV=2.64
    {16664,1600,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.64  BV=2.54
    {16664,1696,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.73  BV=2.45
    {25001,1216,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.25  BV=2.35
    {25001,1296,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.34  BV=2.26
    {25001,1408,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.46  BV=2.14
    {25001,1488,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.54  BV=2.06
    {33327,1200,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.23  BV=1.95
    {33327,1296,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.34  BV=1.84
    {33327,1376,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.43  BV=1.76
    {33327,1488,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.54  BV=1.64
    {33327,1584,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.63  BV=1.55
    {33327,1696,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.73  BV=1.45
    {33327,1824,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.83  BV=1.35
    {33327,1952,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.93  BV=1.25
    {33327,2096,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.03  BV=1.15
    {33327,2256,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.14  BV=1.04
    {33327,2416,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.24  BV=0.94
    {33327,2576,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.33  BV=0.85
    {33327,2784,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.44  BV=0.74
    {33327,2976,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.54  BV=0.64
    {33327,3184,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.64  BV=0.55
    {33327,3408,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.73  BV=0.45
    {33327,3648,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.83  BV=0.35
    {33327,3920,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.94  BV=0.25
    {33327,4208,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.04  BV=0.14
    {33327,4496,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.13  BV=0.05
    {33327,4816,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.23  BV=-0.05
    {33327,5152,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.33  BV=-0.15
    {33327,5520,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.43  BV=-0.25
    {33327,5952,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.54  BV=-0.36
    {33327,6384,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.64  BV=-0.46
    {33327,6832,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.74  BV=-0.56
    {33327,7312,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.84  BV=-0.65
    {33327,7840,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.94  BV=-0.75
    {33327,8400,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.04  BV=-0.85
    {33327,8992,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.13  BV=-0.95
    {33327,9632,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.23  BV=-1.05
    {33327,10320,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.33  BV=-1.15
    {33327,11056,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.43  BV=-1.25
    {33327,11840,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.53  BV=-1.35
    {33327,12720,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.63  BV=-1.45
    {33327,13632,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.73  BV=-1.55
    {41664,11680,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=8.51  BV=-1.65
    {41664,12544,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=8.61  BV=-1.75
    {41664,13440,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=8.71  BV=-1.85
    {41664,14400,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=8.81  BV=-1.95
    {41664,15424,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=8.91  BV=-2.05
    {41664,15872,1064, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.01  BV=-2.15
    {41664,15872,1136, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.10  BV=-2.24
    {41664,15872,1224, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.21  BV=-2.35
    {41664,15872,1312, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.31  BV=-2.45
    {41664,15872,1400, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.41  BV=-2.55
    {41664,15872,1504, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.51  BV=-2.65
    {41664,15872,1608, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.61  BV=-2.75
    {41664,15872,1728, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.71  BV=-2.85
    {41664,15872,1848, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.81  BV=-2.95
    {41664,15872,2000, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.92  BV=-3.06
    {41664,15872,2136, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=10.01  BV=-3.15
    {41664,15872,2296, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=10.12  BV=-3.26
    {41664,15872,2456, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=10.22  BV=-3.36
    {41664,15872,2632, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=10.32  BV=-3.46
    {41664,15872,2824, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=10.42  BV=-3.56
    {41664,15872,3008, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=10.51  BV=-3.65
    {41664,15872,3224, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=10.61  BV=-3.75
    {50001,15872,2880, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=10.45  BV=-3.85
    {50001,15872,3088, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=10.55  BV=-3.95
    {58328,15872,2832, 0, 0, 0},  //TV = 4.10(5555 lines)  AV=2.28  SV=10.42  BV=-4.05
    {58328,15872,3040, 0, 0, 0},  //TV = 4.10(5555 lines)  AV=2.28  SV=10.52  BV=-4.15
    {66665,15872,2848, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=10.43  BV=-4.25
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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

static strEvPline sAEScene15PLineTable_50Hz =
{
{
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {525,1200,1024, 0, 0, 0},  //TV = 10.90(50 lines)  AV=2.28  SV=5.23  BV=7.94
    {557,1200,1024, 0, 0, 0},  //TV = 10.81(53 lines)  AV=2.28  SV=5.23  BV=7.86
    {599,1200,1024, 0, 0, 0},  //TV = 10.71(57 lines)  AV=2.28  SV=5.23  BV=7.75
    {641,1200,1024, 0, 0, 0},  //TV = 10.61(61 lines)  AV=2.28  SV=5.23  BV=7.65
    {683,1216,1024, 0, 0, 0},  //TV = 10.52(65 lines)  AV=2.28  SV=5.25  BV=7.54
    {735,1200,1024, 0, 0, 0},  //TV = 10.41(70 lines)  AV=2.28  SV=5.23  BV=7.46
    {788,1200,1024, 0, 0, 0},  //TV = 10.31(75 lines)  AV=2.28  SV=5.23  BV=7.36
    {840,1216,1024, 0, 0, 0},  //TV = 10.22(80 lines)  AV=2.28  SV=5.25  BV=7.24
    {903,1200,1024, 0, 0, 0},  //TV = 10.11(86 lines)  AV=2.28  SV=5.23  BV=7.16
    {977,1200,1024, 0, 0, 0},  //TV = 10.00(93 lines)  AV=2.28  SV=5.23  BV=7.05
    {1040,1200,1024, 0, 0, 0},  //TV = 9.91(99 lines)  AV=2.28  SV=5.23  BV=6.96
    {1113,1200,1024, 0, 0, 0},  //TV = 9.81(106 lines)  AV=2.28  SV=5.23  BV=6.86
    {1197,1200,1024, 0, 0, 0},  //TV = 9.71(114 lines)  AV=2.28  SV=5.23  BV=6.75
    {1292,1200,1024, 0, 0, 0},  //TV = 9.60(123 lines)  AV=2.28  SV=5.23  BV=6.64
    {1376,1200,1024, 0, 0, 0},  //TV = 9.51(131 lines)  AV=2.28  SV=5.23  BV=6.55
    {1481,1200,1024, 0, 0, 0},  //TV = 9.40(141 lines)  AV=2.28  SV=5.23  BV=6.45
    {1575,1200,1024, 0, 0, 0},  //TV = 9.31(150 lines)  AV=2.28  SV=5.23  BV=6.36
    {1701,1200,1024, 0, 0, 0},  //TV = 9.20(162 lines)  AV=2.28  SV=5.23  BV=6.25
    {1827,1200,1024, 0, 0, 0},  //TV = 9.10(174 lines)  AV=2.28  SV=5.23  BV=6.14
    {1953,1200,1024, 0, 0, 0},  //TV = 9.00(186 lines)  AV=2.28  SV=5.23  BV=6.05
    {2090,1200,1024, 0, 0, 0},  //TV = 8.90(199 lines)  AV=2.28  SV=5.23  BV=5.95
    {2258,1200,1024, 0, 0, 0},  //TV = 8.79(215 lines)  AV=2.28  SV=5.23  BV=5.84
    {2415,1200,1024, 0, 0, 0},  //TV = 8.69(230 lines)  AV=2.28  SV=5.23  BV=5.74
    {2583,1200,1024, 0, 0, 0},  //TV = 8.60(246 lines)  AV=2.28  SV=5.23  BV=5.64
    {2762,1200,1024, 0, 0, 0},  //TV = 8.50(263 lines)  AV=2.28  SV=5.23  BV=5.55
    {2972,1200,1024, 0, 0, 0},  //TV = 8.39(283 lines)  AV=2.28  SV=5.23  BV=5.44
    {3182,1200,1024, 0, 0, 0},  //TV = 8.30(303 lines)  AV=2.28  SV=5.23  BV=5.34
    {3402,1200,1024, 0, 0, 0},  //TV = 8.20(324 lines)  AV=2.28  SV=5.23  BV=5.25
    {3644,1200,1024, 0, 0, 0},  //TV = 8.10(347 lines)  AV=2.28  SV=5.23  BV=5.15
    {3896,1200,1024, 0, 0, 0},  //TV = 8.00(371 lines)  AV=2.28  SV=5.23  BV=5.05
    {4179,1200,1024, 0, 0, 0},  //TV = 7.90(398 lines)  AV=2.28  SV=5.23  BV=4.95
    {4505,1200,1024, 0, 0, 0},  //TV = 7.79(429 lines)  AV=2.28  SV=5.23  BV=4.84
    {4820,1200,1024, 0, 0, 0},  //TV = 7.70(459 lines)  AV=2.28  SV=5.23  BV=4.74
    {5156,1200,1024, 0, 0, 0},  //TV = 7.60(491 lines)  AV=2.28  SV=5.23  BV=4.65
    {5523,1200,1024, 0, 0, 0},  //TV = 7.50(526 lines)  AV=2.28  SV=5.23  BV=4.55
    {5912,1200,1024, 0, 0, 0},  //TV = 7.40(563 lines)  AV=2.28  SV=5.23  BV=4.45
    {6332,1200,1024, 0, 0, 0},  //TV = 7.30(603 lines)  AV=2.28  SV=5.23  BV=4.35
    {6836,1200,1024, 0, 0, 0},  //TV = 7.19(651 lines)  AV=2.28  SV=5.23  BV=4.24
    {7319,1200,1024, 0, 0, 0},  //TV = 7.09(697 lines)  AV=2.28  SV=5.23  BV=4.14
    {7833,1200,1024, 0, 0, 0},  //TV = 7.00(746 lines)  AV=2.28  SV=5.23  BV=4.04
    {8400,1200,1024, 0, 0, 0},  //TV = 6.90(800 lines)  AV=2.28  SV=5.23  BV=3.94
    {8999,1200,1024, 0, 0, 0},  //TV = 6.80(857 lines)  AV=2.28  SV=5.23  BV=3.84
    {9639,1200,1024, 0, 0, 0},  //TV = 6.70(918 lines)  AV=2.28  SV=5.23  BV=3.74
    {9996,1232,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.27  BV=3.65
    {9996,1328,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.38  BV=3.54
    {9996,1424,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.48  BV=3.44
    {9996,1520,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.57  BV=3.35
    {9996,1632,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.67  BV=3.25
    {9996,1744,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.77  BV=3.15
    {9996,1872,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.87  BV=3.05
    {9996,2000,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.97  BV=2.95
    {9996,2144,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.07  BV=2.85
    {9996,2304,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.17  BV=2.75
    {20003,1232,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.27  BV=2.65
    {20003,1328,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.38  BV=2.54
    {20003,1424,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.48  BV=2.44
    {20003,1520,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.57  BV=2.35
    {20003,1632,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.67  BV=2.25
    {20003,1760,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.78  BV=2.14
    {29999,1248,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.29  BV=2.05
    {29999,1344,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.39  BV=1.94
    {29999,1440,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.49  BV=1.84
    {29999,1536,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.58  BV=1.75
    {29999,1648,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.69  BV=1.65
    {29999,1760,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.78  BV=1.55
    {29999,1888,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.88  BV=1.45
    {29999,2032,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.99  BV=1.35
    {29999,2176,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.09  BV=1.25
    {29999,2320,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.18  BV=1.15
    {29999,2496,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.29  BV=1.05
    {29999,2672,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.38  BV=0.95
    {29999,2864,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.48  BV=0.85
    {29999,3072,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.58  BV=0.75
    {29999,3296,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.69  BV=0.65
    {29999,3536,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.79  BV=0.55
    {29999,3776,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.88  BV=0.45
    {29999,4048,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.98  BV=0.35
    {29999,4368,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.09  BV=0.24
    {29999,4672,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.19  BV=0.14
    {29999,5008,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.29  BV=0.04
    {29999,5360,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.39  BV=-0.05
    {29999,5728,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.48  BV=-0.15
    {29999,6144,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.58  BV=-0.25
    {29999,6624,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.69  BV=-0.36
    {29999,7088,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.79  BV=-0.46
    {29999,7584,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.89  BV=-0.55
    {29999,8128,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.99  BV=-0.65
    {29999,8704,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.09  BV=-0.75
    {29999,9328,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.19  BV=-0.85
    {29999,9984,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.29  BV=-0.95
    {29999,10704,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.39  BV=-1.05
    {29999,11472,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.49  BV=-1.15
    {29999,12288,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.58  BV=-1.25
    {29999,13248,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.69  BV=-1.36
    {29999,14128,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.79  BV=-1.45
    {40005,11360,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=8.47  BV=-1.55
    {40005,12160,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=8.57  BV=-1.65
    {40005,13056,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=8.67  BV=-1.75
    {40005,14000,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=8.77  BV=-1.85
    {40005,14992,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=8.87  BV=-1.95
    {40005,15872,1032, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=8.97  BV=-2.05
    {40005,15872,1104, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.06  BV=-2.14
    {40005,15872,1184, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.16  BV=-2.24
    {40005,15872,1272, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.27  BV=-2.35
    {40005,15872,1360, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.36  BV=-2.44
    {40005,15872,1464, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.47  BV=-2.55
    {40005,15872,1568, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.57  BV=-2.65
    {40005,15872,1680, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.67  BV=-2.75
    {40005,15872,1800, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.77  BV=-2.85
    {40005,15872,1928, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.87  BV=-2.95
    {40005,15872,2080, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.98  BV=-3.06
    {40005,15872,2232, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=10.08  BV=-3.16
    {40005,15872,2392, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=10.18  BV=-3.26
    {40005,15872,2560, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=10.28  BV=-3.36
    {40005,15872,2744, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=10.38  BV=-3.46
    {40005,15872,2944, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=10.48  BV=-3.56
    {40005,15872,3136, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=10.57  BV=-3.65
    {50001,15872,2688, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=10.35  BV=-3.75
    {50001,15872,2880, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=10.45  BV=-3.85
    {50001,15872,3088, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=10.55  BV=-3.95
    {59997,15872,2760, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=10.38  BV=-4.05
    {59997,15872,2952, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=10.48  BV=-4.15
    {59997,15872,3168, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=10.58  BV=-4.25
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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

static strAETable g_AE_SceneTable15 =
{
    AETABLE_SCENE_INDEX15, //eAETableID
    147, //u4TotalIndex
    20, //i4StrobeTrigerBV
    104, //i4MaxBV
    -43, //i4MinBV
    90, //i4EffectiveMaxBV
    -50, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_AUTO, //ISO
    sAEScene15PLineTable_60Hz,
    sAEScene15PLineTable_50Hz,
    NULL,
};
static strEvPline sAEScene16PLineTable_60Hz =
{
{
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {525,1200,1024, 0, 0, 0},  //TV = 10.90(50 lines)  AV=2.28  SV=5.23  BV=7.94
    {557,1200,1024, 0, 0, 0},  //TV = 10.81(53 lines)  AV=2.28  SV=5.23  BV=7.86
    {599,1200,1024, 0, 0, 0},  //TV = 10.71(57 lines)  AV=2.28  SV=5.23  BV=7.75
    {641,1200,1024, 0, 0, 0},  //TV = 10.61(61 lines)  AV=2.28  SV=5.23  BV=7.65
    {683,1216,1024, 0, 0, 0},  //TV = 10.52(65 lines)  AV=2.28  SV=5.25  BV=7.54
    {735,1200,1024, 0, 0, 0},  //TV = 10.41(70 lines)  AV=2.28  SV=5.23  BV=7.46
    {788,1200,1024, 0, 0, 0},  //TV = 10.31(75 lines)  AV=2.28  SV=5.23  BV=7.36
    {840,1216,1024, 0, 0, 0},  //TV = 10.22(80 lines)  AV=2.28  SV=5.25  BV=7.24
    {903,1200,1024, 0, 0, 0},  //TV = 10.11(86 lines)  AV=2.28  SV=5.23  BV=7.16
    {977,1200,1024, 0, 0, 0},  //TV = 10.00(93 lines)  AV=2.28  SV=5.23  BV=7.05
    {1040,1200,1024, 0, 0, 0},  //TV = 9.91(99 lines)  AV=2.28  SV=5.23  BV=6.96
    {1113,1200,1024, 0, 0, 0},  //TV = 9.81(106 lines)  AV=2.28  SV=5.23  BV=6.86
    {1197,1200,1024, 0, 0, 0},  //TV = 9.71(114 lines)  AV=2.28  SV=5.23  BV=6.75
    {1292,1200,1024, 0, 0, 0},  //TV = 9.60(123 lines)  AV=2.28  SV=5.23  BV=6.64
    {1376,1200,1024, 0, 0, 0},  //TV = 9.51(131 lines)  AV=2.28  SV=5.23  BV=6.55
    {1481,1200,1024, 0, 0, 0},  //TV = 9.40(141 lines)  AV=2.28  SV=5.23  BV=6.45
    {1575,1200,1024, 0, 0, 0},  //TV = 9.31(150 lines)  AV=2.28  SV=5.23  BV=6.36
    {1701,1200,1024, 0, 0, 0},  //TV = 9.20(162 lines)  AV=2.28  SV=5.23  BV=6.25
    {1827,1200,1024, 0, 0, 0},  //TV = 9.10(174 lines)  AV=2.28  SV=5.23  BV=6.14
    {1953,1200,1024, 0, 0, 0},  //TV = 9.00(186 lines)  AV=2.28  SV=5.23  BV=6.05
    {2090,1200,1024, 0, 0, 0},  //TV = 8.90(199 lines)  AV=2.28  SV=5.23  BV=5.95
    {2258,1200,1024, 0, 0, 0},  //TV = 8.79(215 lines)  AV=2.28  SV=5.23  BV=5.84
    {2415,1200,1024, 0, 0, 0},  //TV = 8.69(230 lines)  AV=2.28  SV=5.23  BV=5.74
    {2583,1200,1024, 0, 0, 0},  //TV = 8.60(246 lines)  AV=2.28  SV=5.23  BV=5.64
    {2762,1200,1024, 0, 0, 0},  //TV = 8.50(263 lines)  AV=2.28  SV=5.23  BV=5.55
    {2972,1200,1024, 0, 0, 0},  //TV = 8.39(283 lines)  AV=2.28  SV=5.23  BV=5.44
    {3182,1200,1024, 0, 0, 0},  //TV = 8.30(303 lines)  AV=2.28  SV=5.23  BV=5.34
    {3402,1200,1024, 0, 0, 0},  //TV = 8.20(324 lines)  AV=2.28  SV=5.23  BV=5.25
    {3644,1200,1024, 0, 0, 0},  //TV = 8.10(347 lines)  AV=2.28  SV=5.23  BV=5.15
    {3896,1200,1024, 0, 0, 0},  //TV = 8.00(371 lines)  AV=2.28  SV=5.23  BV=5.05
    {4179,1200,1024, 0, 0, 0},  //TV = 7.90(398 lines)  AV=2.28  SV=5.23  BV=4.95
    {4505,1200,1024, 0, 0, 0},  //TV = 7.79(429 lines)  AV=2.28  SV=5.23  BV=4.84
    {4820,1200,1024, 0, 0, 0},  //TV = 7.70(459 lines)  AV=2.28  SV=5.23  BV=4.74
    {5156,1200,1024, 0, 0, 0},  //TV = 7.60(491 lines)  AV=2.28  SV=5.23  BV=4.65
    {5523,1200,1024, 0, 0, 0},  //TV = 7.50(526 lines)  AV=2.28  SV=5.23  BV=4.55
    {5912,1200,1024, 0, 0, 0},  //TV = 7.40(563 lines)  AV=2.28  SV=5.23  BV=4.45
    {6332,1200,1024, 0, 0, 0},  //TV = 7.30(603 lines)  AV=2.28  SV=5.23  BV=4.35
    {6836,1200,1024, 0, 0, 0},  //TV = 7.19(651 lines)  AV=2.28  SV=5.23  BV=4.24
    {7319,1200,1024, 0, 0, 0},  //TV = 7.09(697 lines)  AV=2.28  SV=5.23  BV=4.14
    {7833,1200,1024, 0, 0, 0},  //TV = 7.00(746 lines)  AV=2.28  SV=5.23  BV=4.04
    {8338,1200,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.23  BV=3.95
    {8338,1296,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.34  BV=3.84
    {8338,1376,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.43  BV=3.75
    {8338,1488,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.54  BV=3.64
    {8338,1584,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.63  BV=3.55
    {8338,1696,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.73  BV=3.45
    {8338,1824,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.83  BV=3.35
    {8338,1952,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.93  BV=3.25
    {8338,2096,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.03  BV=3.15
    {8338,2240,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.13  BV=3.05
    {16664,1200,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.23  BV=2.95
    {16664,1296,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.34  BV=2.84
    {16664,1376,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.43  BV=2.76
    {16664,1488,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.54  BV=2.64
    {16664,1600,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.64  BV=2.54
    {16664,1696,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.73  BV=2.45
    {25001,1216,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.25  BV=2.35
    {25001,1296,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.34  BV=2.26
    {25001,1408,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.46  BV=2.14
    {25001,1488,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.54  BV=2.06
    {33327,1200,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.23  BV=1.95
    {33327,1296,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.34  BV=1.84
    {33327,1376,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.43  BV=1.76
    {33327,1488,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.54  BV=1.64
    {33327,1584,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.63  BV=1.55
    {33327,1696,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.73  BV=1.45
    {33327,1824,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.83  BV=1.35
    {33327,1952,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.93  BV=1.25
    {33327,2096,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.03  BV=1.15
    {33327,2256,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.14  BV=1.04
    {33327,2416,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.24  BV=0.94
    {33327,2576,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.33  BV=0.85
    {33327,2784,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.44  BV=0.74
    {33327,2976,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.54  BV=0.64
    {33327,3184,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.64  BV=0.55
    {33327,3408,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.73  BV=0.45
    {33327,3648,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.83  BV=0.35
    {33327,3920,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.94  BV=0.25
    {33327,4208,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.04  BV=0.14
    {33327,4496,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.13  BV=0.05
    {33327,4816,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.23  BV=-0.05
    {33327,5152,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.33  BV=-0.15
    {33327,5520,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.43  BV=-0.25
    {33327,5952,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.54  BV=-0.36
    {33327,6384,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.64  BV=-0.46
    {33327,6832,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.74  BV=-0.56
    {33327,7312,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.84  BV=-0.65
    {33327,7840,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.94  BV=-0.75
    {33327,8400,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.04  BV=-0.85
    {33327,8992,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.13  BV=-0.95
    {33327,9632,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.23  BV=-1.05
    {33327,10320,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.33  BV=-1.15
    {33327,11056,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.43  BV=-1.25
    {33327,11840,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.53  BV=-1.35
    {33327,12720,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.63  BV=-1.45
    {33327,13632,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.73  BV=-1.55
    {41664,11680,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=8.51  BV=-1.65
    {41664,12544,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=8.61  BV=-1.75
    {41664,13440,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=8.71  BV=-1.85
    {41664,14400,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=8.81  BV=-1.95
    {41664,15424,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=8.91  BV=-2.05
    {41664,15872,1064, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.01  BV=-2.15
    {41664,15872,1136, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.10  BV=-2.24
    {41664,15872,1224, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.21  BV=-2.35
    {41664,15872,1312, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.31  BV=-2.45
    {41664,15872,1400, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.41  BV=-2.55
    {41664,15872,1504, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.51  BV=-2.65
    {41664,15872,1608, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.61  BV=-2.75
    {41664,15872,1728, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.71  BV=-2.85
    {41664,15872,1848, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.81  BV=-2.95
    {41664,15872,2000, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.92  BV=-3.06
    {41664,15872,2136, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=10.01  BV=-3.15
    {41664,15872,2296, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=10.12  BV=-3.26
    {41664,15872,2456, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=10.22  BV=-3.36
    {41664,15872,2632, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=10.32  BV=-3.46
    {41664,15872,2824, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=10.42  BV=-3.56
    {41664,15872,3008, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=10.51  BV=-3.65
    {41664,15872,3224, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=10.61  BV=-3.75
    {50001,15872,2880, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=10.45  BV=-3.85
    {50001,15872,3088, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=10.55  BV=-3.95
    {58328,15872,2832, 0, 0, 0},  //TV = 4.10(5555 lines)  AV=2.28  SV=10.42  BV=-4.05
    {58328,15872,3040, 0, 0, 0},  //TV = 4.10(5555 lines)  AV=2.28  SV=10.52  BV=-4.15
    {66665,15872,2848, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=10.43  BV=-4.25
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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

static strEvPline sAEScene16PLineTable_50Hz =
{
{
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {525,1200,1024, 0, 0, 0},  //TV = 10.90(50 lines)  AV=2.28  SV=5.23  BV=7.94
    {557,1200,1024, 0, 0, 0},  //TV = 10.81(53 lines)  AV=2.28  SV=5.23  BV=7.86
    {599,1200,1024, 0, 0, 0},  //TV = 10.71(57 lines)  AV=2.28  SV=5.23  BV=7.75
    {641,1200,1024, 0, 0, 0},  //TV = 10.61(61 lines)  AV=2.28  SV=5.23  BV=7.65
    {683,1216,1024, 0, 0, 0},  //TV = 10.52(65 lines)  AV=2.28  SV=5.25  BV=7.54
    {735,1200,1024, 0, 0, 0},  //TV = 10.41(70 lines)  AV=2.28  SV=5.23  BV=7.46
    {788,1200,1024, 0, 0, 0},  //TV = 10.31(75 lines)  AV=2.28  SV=5.23  BV=7.36
    {840,1216,1024, 0, 0, 0},  //TV = 10.22(80 lines)  AV=2.28  SV=5.25  BV=7.24
    {903,1200,1024, 0, 0, 0},  //TV = 10.11(86 lines)  AV=2.28  SV=5.23  BV=7.16
    {977,1200,1024, 0, 0, 0},  //TV = 10.00(93 lines)  AV=2.28  SV=5.23  BV=7.05
    {1040,1200,1024, 0, 0, 0},  //TV = 9.91(99 lines)  AV=2.28  SV=5.23  BV=6.96
    {1113,1200,1024, 0, 0, 0},  //TV = 9.81(106 lines)  AV=2.28  SV=5.23  BV=6.86
    {1197,1200,1024, 0, 0, 0},  //TV = 9.71(114 lines)  AV=2.28  SV=5.23  BV=6.75
    {1292,1200,1024, 0, 0, 0},  //TV = 9.60(123 lines)  AV=2.28  SV=5.23  BV=6.64
    {1376,1200,1024, 0, 0, 0},  //TV = 9.51(131 lines)  AV=2.28  SV=5.23  BV=6.55
    {1481,1200,1024, 0, 0, 0},  //TV = 9.40(141 lines)  AV=2.28  SV=5.23  BV=6.45
    {1575,1200,1024, 0, 0, 0},  //TV = 9.31(150 lines)  AV=2.28  SV=5.23  BV=6.36
    {1701,1200,1024, 0, 0, 0},  //TV = 9.20(162 lines)  AV=2.28  SV=5.23  BV=6.25
    {1827,1200,1024, 0, 0, 0},  //TV = 9.10(174 lines)  AV=2.28  SV=5.23  BV=6.14
    {1953,1200,1024, 0, 0, 0},  //TV = 9.00(186 lines)  AV=2.28  SV=5.23  BV=6.05
    {2090,1200,1024, 0, 0, 0},  //TV = 8.90(199 lines)  AV=2.28  SV=5.23  BV=5.95
    {2258,1200,1024, 0, 0, 0},  //TV = 8.79(215 lines)  AV=2.28  SV=5.23  BV=5.84
    {2415,1200,1024, 0, 0, 0},  //TV = 8.69(230 lines)  AV=2.28  SV=5.23  BV=5.74
    {2583,1200,1024, 0, 0, 0},  //TV = 8.60(246 lines)  AV=2.28  SV=5.23  BV=5.64
    {2762,1200,1024, 0, 0, 0},  //TV = 8.50(263 lines)  AV=2.28  SV=5.23  BV=5.55
    {2972,1200,1024, 0, 0, 0},  //TV = 8.39(283 lines)  AV=2.28  SV=5.23  BV=5.44
    {3182,1200,1024, 0, 0, 0},  //TV = 8.30(303 lines)  AV=2.28  SV=5.23  BV=5.34
    {3402,1200,1024, 0, 0, 0},  //TV = 8.20(324 lines)  AV=2.28  SV=5.23  BV=5.25
    {3644,1200,1024, 0, 0, 0},  //TV = 8.10(347 lines)  AV=2.28  SV=5.23  BV=5.15
    {3896,1200,1024, 0, 0, 0},  //TV = 8.00(371 lines)  AV=2.28  SV=5.23  BV=5.05
    {4179,1200,1024, 0, 0, 0},  //TV = 7.90(398 lines)  AV=2.28  SV=5.23  BV=4.95
    {4505,1200,1024, 0, 0, 0},  //TV = 7.79(429 lines)  AV=2.28  SV=5.23  BV=4.84
    {4820,1200,1024, 0, 0, 0},  //TV = 7.70(459 lines)  AV=2.28  SV=5.23  BV=4.74
    {5156,1200,1024, 0, 0, 0},  //TV = 7.60(491 lines)  AV=2.28  SV=5.23  BV=4.65
    {5523,1200,1024, 0, 0, 0},  //TV = 7.50(526 lines)  AV=2.28  SV=5.23  BV=4.55
    {5912,1200,1024, 0, 0, 0},  //TV = 7.40(563 lines)  AV=2.28  SV=5.23  BV=4.45
    {6332,1200,1024, 0, 0, 0},  //TV = 7.30(603 lines)  AV=2.28  SV=5.23  BV=4.35
    {6836,1200,1024, 0, 0, 0},  //TV = 7.19(651 lines)  AV=2.28  SV=5.23  BV=4.24
    {7319,1200,1024, 0, 0, 0},  //TV = 7.09(697 lines)  AV=2.28  SV=5.23  BV=4.14
    {7833,1200,1024, 0, 0, 0},  //TV = 7.00(746 lines)  AV=2.28  SV=5.23  BV=4.04
    {8400,1200,1024, 0, 0, 0},  //TV = 6.90(800 lines)  AV=2.28  SV=5.23  BV=3.94
    {8999,1200,1024, 0, 0, 0},  //TV = 6.80(857 lines)  AV=2.28  SV=5.23  BV=3.84
    {9639,1200,1024, 0, 0, 0},  //TV = 6.70(918 lines)  AV=2.28  SV=5.23  BV=3.74
    {9996,1232,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.27  BV=3.65
    {9996,1328,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.38  BV=3.54
    {9996,1424,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.48  BV=3.44
    {9996,1520,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.57  BV=3.35
    {9996,1632,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.67  BV=3.25
    {9996,1744,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.77  BV=3.15
    {9996,1872,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.87  BV=3.05
    {9996,2000,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.97  BV=2.95
    {9996,2144,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.07  BV=2.85
    {9996,2304,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.17  BV=2.75
    {20003,1232,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.27  BV=2.65
    {20003,1328,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.38  BV=2.54
    {20003,1424,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.48  BV=2.44
    {20003,1520,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.57  BV=2.35
    {20003,1632,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.67  BV=2.25
    {20003,1760,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.78  BV=2.14
    {29999,1248,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.29  BV=2.05
    {29999,1344,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.39  BV=1.94
    {29999,1440,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.49  BV=1.84
    {29999,1536,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.58  BV=1.75
    {29999,1648,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.69  BV=1.65
    {29999,1760,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.78  BV=1.55
    {29999,1888,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.88  BV=1.45
    {29999,2032,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.99  BV=1.35
    {29999,2176,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.09  BV=1.25
    {29999,2320,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.18  BV=1.15
    {29999,2496,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.29  BV=1.05
    {29999,2672,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.38  BV=0.95
    {29999,2864,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.48  BV=0.85
    {29999,3072,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.58  BV=0.75
    {29999,3296,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.69  BV=0.65
    {29999,3536,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.79  BV=0.55
    {29999,3776,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.88  BV=0.45
    {29999,4048,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.98  BV=0.35
    {29999,4368,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.09  BV=0.24
    {29999,4672,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.19  BV=0.14
    {29999,5008,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.29  BV=0.04
    {29999,5360,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.39  BV=-0.05
    {29999,5728,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.48  BV=-0.15
    {29999,6144,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.58  BV=-0.25
    {29999,6624,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.69  BV=-0.36
    {29999,7088,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.79  BV=-0.46
    {29999,7584,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.89  BV=-0.55
    {29999,8128,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.99  BV=-0.65
    {29999,8704,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.09  BV=-0.75
    {29999,9328,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.19  BV=-0.85
    {29999,9984,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.29  BV=-0.95
    {29999,10704,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.39  BV=-1.05
    {29999,11472,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.49  BV=-1.15
    {29999,12288,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.58  BV=-1.25
    {29999,13248,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.69  BV=-1.36
    {29999,14128,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.79  BV=-1.45
    {40005,11360,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=8.47  BV=-1.55
    {40005,12160,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=8.57  BV=-1.65
    {40005,13056,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=8.67  BV=-1.75
    {40005,14000,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=8.77  BV=-1.85
    {40005,14992,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=8.87  BV=-1.95
    {40005,15872,1032, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=8.97  BV=-2.05
    {40005,15872,1104, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.06  BV=-2.14
    {40005,15872,1184, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.16  BV=-2.24
    {40005,15872,1272, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.27  BV=-2.35
    {40005,15872,1360, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.36  BV=-2.44
    {40005,15872,1464, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.47  BV=-2.55
    {40005,15872,1568, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.57  BV=-2.65
    {40005,15872,1680, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.67  BV=-2.75
    {40005,15872,1800, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.77  BV=-2.85
    {40005,15872,1928, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.87  BV=-2.95
    {40005,15872,2080, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.98  BV=-3.06
    {40005,15872,2232, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=10.08  BV=-3.16
    {40005,15872,2392, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=10.18  BV=-3.26
    {40005,15872,2560, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=10.28  BV=-3.36
    {40005,15872,2744, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=10.38  BV=-3.46
    {40005,15872,2944, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=10.48  BV=-3.56
    {40005,15872,3136, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=10.57  BV=-3.65
    {50001,15872,2688, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=10.35  BV=-3.75
    {50001,15872,2880, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=10.45  BV=-3.85
    {50001,15872,3088, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=10.55  BV=-3.95
    {59997,15872,2760, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=10.38  BV=-4.05
    {59997,15872,2952, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=10.48  BV=-4.15
    {59997,15872,3168, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=10.58  BV=-4.25
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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

static strAETable g_AE_SceneTable16 =
{
    AETABLE_SCENE_INDEX16, //eAETableID
    147, //u4TotalIndex
    20, //i4StrobeTrigerBV
    104, //i4MaxBV
    -43, //i4MinBV
    90, //i4EffectiveMaxBV
    -50, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_AUTO, //ISO
    sAEScene16PLineTable_60Hz,
    sAEScene16PLineTable_50Hz,
    NULL,
};
static strEvPline sAEScene17PLineTable_60Hz =
{
{
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {525,1200,1024, 0, 0, 0},  //TV = 10.90(50 lines)  AV=2.28  SV=5.23  BV=7.94
    {557,1200,1024, 0, 0, 0},  //TV = 10.81(53 lines)  AV=2.28  SV=5.23  BV=7.86
    {599,1200,1024, 0, 0, 0},  //TV = 10.71(57 lines)  AV=2.28  SV=5.23  BV=7.75
    {641,1200,1024, 0, 0, 0},  //TV = 10.61(61 lines)  AV=2.28  SV=5.23  BV=7.65
    {683,1216,1024, 0, 0, 0},  //TV = 10.52(65 lines)  AV=2.28  SV=5.25  BV=7.54
    {735,1200,1024, 0, 0, 0},  //TV = 10.41(70 lines)  AV=2.28  SV=5.23  BV=7.46
    {788,1200,1024, 0, 0, 0},  //TV = 10.31(75 lines)  AV=2.28  SV=5.23  BV=7.36
    {840,1216,1024, 0, 0, 0},  //TV = 10.22(80 lines)  AV=2.28  SV=5.25  BV=7.24
    {903,1200,1024, 0, 0, 0},  //TV = 10.11(86 lines)  AV=2.28  SV=5.23  BV=7.16
    {977,1200,1024, 0, 0, 0},  //TV = 10.00(93 lines)  AV=2.28  SV=5.23  BV=7.05
    {1040,1200,1024, 0, 0, 0},  //TV = 9.91(99 lines)  AV=2.28  SV=5.23  BV=6.96
    {1113,1200,1024, 0, 0, 0},  //TV = 9.81(106 lines)  AV=2.28  SV=5.23  BV=6.86
    {1197,1200,1024, 0, 0, 0},  //TV = 9.71(114 lines)  AV=2.28  SV=5.23  BV=6.75
    {1292,1200,1024, 0, 0, 0},  //TV = 9.60(123 lines)  AV=2.28  SV=5.23  BV=6.64
    {1376,1200,1024, 0, 0, 0},  //TV = 9.51(131 lines)  AV=2.28  SV=5.23  BV=6.55
    {1481,1200,1024, 0, 0, 0},  //TV = 9.40(141 lines)  AV=2.28  SV=5.23  BV=6.45
    {1575,1200,1024, 0, 0, 0},  //TV = 9.31(150 lines)  AV=2.28  SV=5.23  BV=6.36
    {1701,1200,1024, 0, 0, 0},  //TV = 9.20(162 lines)  AV=2.28  SV=5.23  BV=6.25
    {1827,1200,1024, 0, 0, 0},  //TV = 9.10(174 lines)  AV=2.28  SV=5.23  BV=6.14
    {1953,1200,1024, 0, 0, 0},  //TV = 9.00(186 lines)  AV=2.28  SV=5.23  BV=6.05
    {2090,1200,1024, 0, 0, 0},  //TV = 8.90(199 lines)  AV=2.28  SV=5.23  BV=5.95
    {2258,1200,1024, 0, 0, 0},  //TV = 8.79(215 lines)  AV=2.28  SV=5.23  BV=5.84
    {2415,1200,1024, 0, 0, 0},  //TV = 8.69(230 lines)  AV=2.28  SV=5.23  BV=5.74
    {2583,1200,1024, 0, 0, 0},  //TV = 8.60(246 lines)  AV=2.28  SV=5.23  BV=5.64
    {2762,1200,1024, 0, 0, 0},  //TV = 8.50(263 lines)  AV=2.28  SV=5.23  BV=5.55
    {2972,1200,1024, 0, 0, 0},  //TV = 8.39(283 lines)  AV=2.28  SV=5.23  BV=5.44
    {3182,1200,1024, 0, 0, 0},  //TV = 8.30(303 lines)  AV=2.28  SV=5.23  BV=5.34
    {3402,1200,1024, 0, 0, 0},  //TV = 8.20(324 lines)  AV=2.28  SV=5.23  BV=5.25
    {3644,1200,1024, 0, 0, 0},  //TV = 8.10(347 lines)  AV=2.28  SV=5.23  BV=5.15
    {3896,1200,1024, 0, 0, 0},  //TV = 8.00(371 lines)  AV=2.28  SV=5.23  BV=5.05
    {4179,1200,1024, 0, 0, 0},  //TV = 7.90(398 lines)  AV=2.28  SV=5.23  BV=4.95
    {4505,1200,1024, 0, 0, 0},  //TV = 7.79(429 lines)  AV=2.28  SV=5.23  BV=4.84
    {4820,1200,1024, 0, 0, 0},  //TV = 7.70(459 lines)  AV=2.28  SV=5.23  BV=4.74
    {5156,1200,1024, 0, 0, 0},  //TV = 7.60(491 lines)  AV=2.28  SV=5.23  BV=4.65
    {5523,1200,1024, 0, 0, 0},  //TV = 7.50(526 lines)  AV=2.28  SV=5.23  BV=4.55
    {5912,1200,1024, 0, 0, 0},  //TV = 7.40(563 lines)  AV=2.28  SV=5.23  BV=4.45
    {6332,1200,1024, 0, 0, 0},  //TV = 7.30(603 lines)  AV=2.28  SV=5.23  BV=4.35
    {6836,1200,1024, 0, 0, 0},  //TV = 7.19(651 lines)  AV=2.28  SV=5.23  BV=4.24
    {7319,1200,1024, 0, 0, 0},  //TV = 7.09(697 lines)  AV=2.28  SV=5.23  BV=4.14
    {7833,1200,1024, 0, 0, 0},  //TV = 7.00(746 lines)  AV=2.28  SV=5.23  BV=4.04
    {8338,1200,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.23  BV=3.95
    {8338,1296,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.34  BV=3.84
    {8338,1376,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.43  BV=3.75
    {8338,1488,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.54  BV=3.64
    {8338,1584,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.63  BV=3.55
    {8338,1696,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.73  BV=3.45
    {8338,1824,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.83  BV=3.35
    {8338,1952,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.93  BV=3.25
    {8338,2096,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.03  BV=3.15
    {8338,2240,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.13  BV=3.05
    {16664,1200,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.23  BV=2.95
    {16664,1296,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.34  BV=2.84
    {16664,1376,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.43  BV=2.76
    {16664,1488,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.54  BV=2.64
    {16664,1600,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.64  BV=2.54
    {16664,1696,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.73  BV=2.45
    {25001,1216,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.25  BV=2.35
    {25001,1296,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.34  BV=2.26
    {25001,1408,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.46  BV=2.14
    {25001,1488,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.54  BV=2.06
    {33327,1200,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.23  BV=1.95
    {33327,1296,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.34  BV=1.84
    {33327,1376,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.43  BV=1.76
    {33327,1488,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.54  BV=1.64
    {33327,1584,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.63  BV=1.55
    {33327,1696,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.73  BV=1.45
    {33327,1824,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.83  BV=1.35
    {33327,1952,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.93  BV=1.25
    {33327,2096,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.03  BV=1.15
    {33327,2256,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.14  BV=1.04
    {33327,2416,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.24  BV=0.94
    {33327,2576,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.33  BV=0.85
    {33327,2784,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.44  BV=0.74
    {33327,2976,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.54  BV=0.64
    {33327,3184,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.64  BV=0.55
    {33327,3408,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.73  BV=0.45
    {33327,3648,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.83  BV=0.35
    {33327,3920,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.94  BV=0.25
    {33327,4208,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.04  BV=0.14
    {33327,4496,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.13  BV=0.05
    {33327,4816,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.23  BV=-0.05
    {33327,5152,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.33  BV=-0.15
    {33327,5520,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.43  BV=-0.25
    {33327,5952,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.54  BV=-0.36
    {33327,6384,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.64  BV=-0.46
    {33327,6832,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.74  BV=-0.56
    {33327,7312,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.84  BV=-0.65
    {33327,7840,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.94  BV=-0.75
    {33327,8400,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.04  BV=-0.85
    {33327,8992,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.13  BV=-0.95
    {33327,9632,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.23  BV=-1.05
    {33327,10320,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.33  BV=-1.15
    {33327,11056,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.43  BV=-1.25
    {33327,11840,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.53  BV=-1.35
    {33327,12720,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.63  BV=-1.45
    {33327,13632,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.73  BV=-1.55
    {41664,11680,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=8.51  BV=-1.65
    {41664,12544,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=8.61  BV=-1.75
    {41664,13440,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=8.71  BV=-1.85
    {41664,14400,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=8.81  BV=-1.95
    {41664,15424,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=8.91  BV=-2.05
    {41664,15872,1064, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.01  BV=-2.15
    {41664,15872,1136, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.10  BV=-2.24
    {41664,15872,1224, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.21  BV=-2.35
    {41664,15872,1312, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.31  BV=-2.45
    {41664,15872,1400, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.41  BV=-2.55
    {41664,15872,1504, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.51  BV=-2.65
    {41664,15872,1608, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.61  BV=-2.75
    {41664,15872,1728, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.71  BV=-2.85
    {41664,15872,1848, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.81  BV=-2.95
    {41664,15872,2000, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.92  BV=-3.06
    {41664,15872,2136, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=10.01  BV=-3.15
    {41664,15872,2296, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=10.12  BV=-3.26
    {41664,15872,2456, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=10.22  BV=-3.36
    {41664,15872,2632, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=10.32  BV=-3.46
    {41664,15872,2824, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=10.42  BV=-3.56
    {41664,15872,3008, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=10.51  BV=-3.65
    {41664,15872,3224, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=10.61  BV=-3.75
    {50001,15872,2880, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=10.45  BV=-3.85
    {50001,15872,3088, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=10.55  BV=-3.95
    {58328,15872,2832, 0, 0, 0},  //TV = 4.10(5555 lines)  AV=2.28  SV=10.42  BV=-4.05
    {58328,15872,3040, 0, 0, 0},  //TV = 4.10(5555 lines)  AV=2.28  SV=10.52  BV=-4.15
    {66665,15872,2848, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=10.43  BV=-4.25
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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

static strEvPline sAEScene17PLineTable_50Hz =
{
{
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {525,1200,1024, 0, 0, 0},  //TV = 10.90(50 lines)  AV=2.28  SV=5.23  BV=7.94
    {557,1200,1024, 0, 0, 0},  //TV = 10.81(53 lines)  AV=2.28  SV=5.23  BV=7.86
    {599,1200,1024, 0, 0, 0},  //TV = 10.71(57 lines)  AV=2.28  SV=5.23  BV=7.75
    {641,1200,1024, 0, 0, 0},  //TV = 10.61(61 lines)  AV=2.28  SV=5.23  BV=7.65
    {683,1216,1024, 0, 0, 0},  //TV = 10.52(65 lines)  AV=2.28  SV=5.25  BV=7.54
    {735,1200,1024, 0, 0, 0},  //TV = 10.41(70 lines)  AV=2.28  SV=5.23  BV=7.46
    {788,1200,1024, 0, 0, 0},  //TV = 10.31(75 lines)  AV=2.28  SV=5.23  BV=7.36
    {840,1216,1024, 0, 0, 0},  //TV = 10.22(80 lines)  AV=2.28  SV=5.25  BV=7.24
    {903,1200,1024, 0, 0, 0},  //TV = 10.11(86 lines)  AV=2.28  SV=5.23  BV=7.16
    {977,1200,1024, 0, 0, 0},  //TV = 10.00(93 lines)  AV=2.28  SV=5.23  BV=7.05
    {1040,1200,1024, 0, 0, 0},  //TV = 9.91(99 lines)  AV=2.28  SV=5.23  BV=6.96
    {1113,1200,1024, 0, 0, 0},  //TV = 9.81(106 lines)  AV=2.28  SV=5.23  BV=6.86
    {1197,1200,1024, 0, 0, 0},  //TV = 9.71(114 lines)  AV=2.28  SV=5.23  BV=6.75
    {1292,1200,1024, 0, 0, 0},  //TV = 9.60(123 lines)  AV=2.28  SV=5.23  BV=6.64
    {1376,1200,1024, 0, 0, 0},  //TV = 9.51(131 lines)  AV=2.28  SV=5.23  BV=6.55
    {1481,1200,1024, 0, 0, 0},  //TV = 9.40(141 lines)  AV=2.28  SV=5.23  BV=6.45
    {1575,1200,1024, 0, 0, 0},  //TV = 9.31(150 lines)  AV=2.28  SV=5.23  BV=6.36
    {1701,1200,1024, 0, 0, 0},  //TV = 9.20(162 lines)  AV=2.28  SV=5.23  BV=6.25
    {1827,1200,1024, 0, 0, 0},  //TV = 9.10(174 lines)  AV=2.28  SV=5.23  BV=6.14
    {1953,1200,1024, 0, 0, 0},  //TV = 9.00(186 lines)  AV=2.28  SV=5.23  BV=6.05
    {2090,1200,1024, 0, 0, 0},  //TV = 8.90(199 lines)  AV=2.28  SV=5.23  BV=5.95
    {2258,1200,1024, 0, 0, 0},  //TV = 8.79(215 lines)  AV=2.28  SV=5.23  BV=5.84
    {2415,1200,1024, 0, 0, 0},  //TV = 8.69(230 lines)  AV=2.28  SV=5.23  BV=5.74
    {2583,1200,1024, 0, 0, 0},  //TV = 8.60(246 lines)  AV=2.28  SV=5.23  BV=5.64
    {2762,1200,1024, 0, 0, 0},  //TV = 8.50(263 lines)  AV=2.28  SV=5.23  BV=5.55
    {2972,1200,1024, 0, 0, 0},  //TV = 8.39(283 lines)  AV=2.28  SV=5.23  BV=5.44
    {3182,1200,1024, 0, 0, 0},  //TV = 8.30(303 lines)  AV=2.28  SV=5.23  BV=5.34
    {3402,1200,1024, 0, 0, 0},  //TV = 8.20(324 lines)  AV=2.28  SV=5.23  BV=5.25
    {3644,1200,1024, 0, 0, 0},  //TV = 8.10(347 lines)  AV=2.28  SV=5.23  BV=5.15
    {3896,1200,1024, 0, 0, 0},  //TV = 8.00(371 lines)  AV=2.28  SV=5.23  BV=5.05
    {4179,1200,1024, 0, 0, 0},  //TV = 7.90(398 lines)  AV=2.28  SV=5.23  BV=4.95
    {4505,1200,1024, 0, 0, 0},  //TV = 7.79(429 lines)  AV=2.28  SV=5.23  BV=4.84
    {4820,1200,1024, 0, 0, 0},  //TV = 7.70(459 lines)  AV=2.28  SV=5.23  BV=4.74
    {5156,1200,1024, 0, 0, 0},  //TV = 7.60(491 lines)  AV=2.28  SV=5.23  BV=4.65
    {5523,1200,1024, 0, 0, 0},  //TV = 7.50(526 lines)  AV=2.28  SV=5.23  BV=4.55
    {5912,1200,1024, 0, 0, 0},  //TV = 7.40(563 lines)  AV=2.28  SV=5.23  BV=4.45
    {6332,1200,1024, 0, 0, 0},  //TV = 7.30(603 lines)  AV=2.28  SV=5.23  BV=4.35
    {6836,1200,1024, 0, 0, 0},  //TV = 7.19(651 lines)  AV=2.28  SV=5.23  BV=4.24
    {7319,1200,1024, 0, 0, 0},  //TV = 7.09(697 lines)  AV=2.28  SV=5.23  BV=4.14
    {7833,1200,1024, 0, 0, 0},  //TV = 7.00(746 lines)  AV=2.28  SV=5.23  BV=4.04
    {8400,1200,1024, 0, 0, 0},  //TV = 6.90(800 lines)  AV=2.28  SV=5.23  BV=3.94
    {8999,1200,1024, 0, 0, 0},  //TV = 6.80(857 lines)  AV=2.28  SV=5.23  BV=3.84
    {9639,1200,1024, 0, 0, 0},  //TV = 6.70(918 lines)  AV=2.28  SV=5.23  BV=3.74
    {9996,1232,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.27  BV=3.65
    {9996,1328,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.38  BV=3.54
    {9996,1424,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.48  BV=3.44
    {9996,1520,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.57  BV=3.35
    {9996,1632,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.67  BV=3.25
    {9996,1744,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.77  BV=3.15
    {9996,1872,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.87  BV=3.05
    {9996,2000,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.97  BV=2.95
    {9996,2144,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.07  BV=2.85
    {9996,2304,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.17  BV=2.75
    {20003,1232,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.27  BV=2.65
    {20003,1328,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.38  BV=2.54
    {20003,1424,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.48  BV=2.44
    {20003,1520,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.57  BV=2.35
    {20003,1632,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.67  BV=2.25
    {20003,1760,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.78  BV=2.14
    {29999,1248,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.29  BV=2.05
    {29999,1344,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.39  BV=1.94
    {29999,1440,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.49  BV=1.84
    {29999,1536,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.58  BV=1.75
    {29999,1648,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.69  BV=1.65
    {29999,1760,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.78  BV=1.55
    {29999,1888,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.88  BV=1.45
    {29999,2032,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.99  BV=1.35
    {29999,2176,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.09  BV=1.25
    {29999,2320,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.18  BV=1.15
    {29999,2496,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.29  BV=1.05
    {29999,2672,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.38  BV=0.95
    {29999,2864,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.48  BV=0.85
    {29999,3072,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.58  BV=0.75
    {29999,3296,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.69  BV=0.65
    {29999,3536,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.79  BV=0.55
    {29999,3776,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.88  BV=0.45
    {29999,4048,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.98  BV=0.35
    {29999,4368,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.09  BV=0.24
    {29999,4672,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.19  BV=0.14
    {29999,5008,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.29  BV=0.04
    {29999,5360,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.39  BV=-0.05
    {29999,5728,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.48  BV=-0.15
    {29999,6144,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.58  BV=-0.25
    {29999,6624,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.69  BV=-0.36
    {29999,7088,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.79  BV=-0.46
    {29999,7584,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.89  BV=-0.55
    {29999,8128,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.99  BV=-0.65
    {29999,8704,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.09  BV=-0.75
    {29999,9328,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.19  BV=-0.85
    {29999,9984,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.29  BV=-0.95
    {29999,10704,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.39  BV=-1.05
    {29999,11472,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.49  BV=-1.15
    {29999,12288,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.58  BV=-1.25
    {29999,13248,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.69  BV=-1.36
    {29999,14128,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.79  BV=-1.45
    {40005,11360,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=8.47  BV=-1.55
    {40005,12160,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=8.57  BV=-1.65
    {40005,13056,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=8.67  BV=-1.75
    {40005,14000,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=8.77  BV=-1.85
    {40005,14992,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=8.87  BV=-1.95
    {40005,15872,1032, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=8.97  BV=-2.05
    {40005,15872,1104, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.06  BV=-2.14
    {40005,15872,1184, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.16  BV=-2.24
    {40005,15872,1272, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.27  BV=-2.35
    {40005,15872,1360, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.36  BV=-2.44
    {40005,15872,1464, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.47  BV=-2.55
    {40005,15872,1568, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.57  BV=-2.65
    {40005,15872,1680, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.67  BV=-2.75
    {40005,15872,1800, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.77  BV=-2.85
    {40005,15872,1928, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.87  BV=-2.95
    {40005,15872,2080, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.98  BV=-3.06
    {40005,15872,2232, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=10.08  BV=-3.16
    {40005,15872,2392, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=10.18  BV=-3.26
    {40005,15872,2560, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=10.28  BV=-3.36
    {40005,15872,2744, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=10.38  BV=-3.46
    {40005,15872,2944, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=10.48  BV=-3.56
    {40005,15872,3136, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=10.57  BV=-3.65
    {50001,15872,2688, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=10.35  BV=-3.75
    {50001,15872,2880, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=10.45  BV=-3.85
    {50001,15872,3088, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=10.55  BV=-3.95
    {59997,15872,2760, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=10.38  BV=-4.05
    {59997,15872,2952, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=10.48  BV=-4.15
    {59997,15872,3168, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=10.58  BV=-4.25
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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

static strAETable g_AE_SceneTable17 =
{
    AETABLE_SCENE_INDEX17, //eAETableID
    147, //u4TotalIndex
    20, //i4StrobeTrigerBV
    104, //i4MaxBV
    -43, //i4MinBV
    90, //i4EffectiveMaxBV
    -50, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_AUTO, //ISO
    sAEScene17PLineTable_60Hz,
    sAEScene17PLineTable_50Hz,
    NULL,
};
static strEvPline sAEScene18PLineTable_60Hz =
{
{
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {525,1200,1024, 0, 0, 0},  //TV = 10.90(50 lines)  AV=2.28  SV=5.23  BV=7.94
    {557,1200,1024, 0, 0, 0},  //TV = 10.81(53 lines)  AV=2.28  SV=5.23  BV=7.86
    {599,1200,1024, 0, 0, 0},  //TV = 10.71(57 lines)  AV=2.28  SV=5.23  BV=7.75
    {641,1200,1024, 0, 0, 0},  //TV = 10.61(61 lines)  AV=2.28  SV=5.23  BV=7.65
    {683,1216,1024, 0, 0, 0},  //TV = 10.52(65 lines)  AV=2.28  SV=5.25  BV=7.54
    {735,1200,1024, 0, 0, 0},  //TV = 10.41(70 lines)  AV=2.28  SV=5.23  BV=7.46
    {788,1200,1024, 0, 0, 0},  //TV = 10.31(75 lines)  AV=2.28  SV=5.23  BV=7.36
    {840,1216,1024, 0, 0, 0},  //TV = 10.22(80 lines)  AV=2.28  SV=5.25  BV=7.24
    {903,1200,1024, 0, 0, 0},  //TV = 10.11(86 lines)  AV=2.28  SV=5.23  BV=7.16
    {977,1200,1024, 0, 0, 0},  //TV = 10.00(93 lines)  AV=2.28  SV=5.23  BV=7.05
    {1040,1200,1024, 0, 0, 0},  //TV = 9.91(99 lines)  AV=2.28  SV=5.23  BV=6.96
    {1113,1200,1024, 0, 0, 0},  //TV = 9.81(106 lines)  AV=2.28  SV=5.23  BV=6.86
    {1197,1200,1024, 0, 0, 0},  //TV = 9.71(114 lines)  AV=2.28  SV=5.23  BV=6.75
    {1292,1200,1024, 0, 0, 0},  //TV = 9.60(123 lines)  AV=2.28  SV=5.23  BV=6.64
    {1376,1200,1024, 0, 0, 0},  //TV = 9.51(131 lines)  AV=2.28  SV=5.23  BV=6.55
    {1481,1200,1024, 0, 0, 0},  //TV = 9.40(141 lines)  AV=2.28  SV=5.23  BV=6.45
    {1575,1200,1024, 0, 0, 0},  //TV = 9.31(150 lines)  AV=2.28  SV=5.23  BV=6.36
    {1701,1200,1024, 0, 0, 0},  //TV = 9.20(162 lines)  AV=2.28  SV=5.23  BV=6.25
    {1827,1200,1024, 0, 0, 0},  //TV = 9.10(174 lines)  AV=2.28  SV=5.23  BV=6.14
    {1953,1200,1024, 0, 0, 0},  //TV = 9.00(186 lines)  AV=2.28  SV=5.23  BV=6.05
    {2090,1200,1024, 0, 0, 0},  //TV = 8.90(199 lines)  AV=2.28  SV=5.23  BV=5.95
    {2258,1200,1024, 0, 0, 0},  //TV = 8.79(215 lines)  AV=2.28  SV=5.23  BV=5.84
    {2415,1200,1024, 0, 0, 0},  //TV = 8.69(230 lines)  AV=2.28  SV=5.23  BV=5.74
    {2583,1200,1024, 0, 0, 0},  //TV = 8.60(246 lines)  AV=2.28  SV=5.23  BV=5.64
    {2762,1200,1024, 0, 0, 0},  //TV = 8.50(263 lines)  AV=2.28  SV=5.23  BV=5.55
    {2972,1200,1024, 0, 0, 0},  //TV = 8.39(283 lines)  AV=2.28  SV=5.23  BV=5.44
    {3182,1200,1024, 0, 0, 0},  //TV = 8.30(303 lines)  AV=2.28  SV=5.23  BV=5.34
    {3402,1200,1024, 0, 0, 0},  //TV = 8.20(324 lines)  AV=2.28  SV=5.23  BV=5.25
    {3644,1200,1024, 0, 0, 0},  //TV = 8.10(347 lines)  AV=2.28  SV=5.23  BV=5.15
    {3896,1200,1024, 0, 0, 0},  //TV = 8.00(371 lines)  AV=2.28  SV=5.23  BV=5.05
    {4179,1200,1024, 0, 0, 0},  //TV = 7.90(398 lines)  AV=2.28  SV=5.23  BV=4.95
    {4505,1200,1024, 0, 0, 0},  //TV = 7.79(429 lines)  AV=2.28  SV=5.23  BV=4.84
    {4820,1200,1024, 0, 0, 0},  //TV = 7.70(459 lines)  AV=2.28  SV=5.23  BV=4.74
    {5156,1200,1024, 0, 0, 0},  //TV = 7.60(491 lines)  AV=2.28  SV=5.23  BV=4.65
    {5523,1200,1024, 0, 0, 0},  //TV = 7.50(526 lines)  AV=2.28  SV=5.23  BV=4.55
    {5912,1200,1024, 0, 0, 0},  //TV = 7.40(563 lines)  AV=2.28  SV=5.23  BV=4.45
    {6332,1200,1024, 0, 0, 0},  //TV = 7.30(603 lines)  AV=2.28  SV=5.23  BV=4.35
    {6836,1200,1024, 0, 0, 0},  //TV = 7.19(651 lines)  AV=2.28  SV=5.23  BV=4.24
    {7319,1200,1024, 0, 0, 0},  //TV = 7.09(697 lines)  AV=2.28  SV=5.23  BV=4.14
    {7833,1200,1024, 0, 0, 0},  //TV = 7.00(746 lines)  AV=2.28  SV=5.23  BV=4.04
    {8338,1200,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.23  BV=3.95
    {8338,1296,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.34  BV=3.84
    {8338,1376,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.43  BV=3.75
    {8338,1488,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.54  BV=3.64
    {8338,1584,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.63  BV=3.55
    {8338,1696,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.73  BV=3.45
    {8338,1824,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.83  BV=3.35
    {8338,1952,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=5.93  BV=3.25
    {8338,2096,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.03  BV=3.15
    {8338,2240,1024, 0, 0, 0},  //TV = 6.91(794 lines)  AV=2.28  SV=6.13  BV=3.05
    {16664,1200,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.23  BV=2.95
    {16664,1296,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.34  BV=2.84
    {16664,1376,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.43  BV=2.76
    {16664,1488,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.54  BV=2.64
    {16664,1600,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.64  BV=2.54
    {16664,1696,1024, 0, 0, 0},  //TV = 5.91(1587 lines)  AV=2.28  SV=5.73  BV=2.45
    {25001,1216,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.25  BV=2.35
    {25001,1296,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.34  BV=2.26
    {25001,1408,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.46  BV=2.14
    {25001,1488,1024, 0, 0, 0},  //TV = 5.32(2381 lines)  AV=2.28  SV=5.54  BV=2.06
    {33327,1200,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.23  BV=1.95
    {33327,1296,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.34  BV=1.84
    {33327,1376,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.43  BV=1.76
    {33327,1488,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.54  BV=1.64
    {33327,1584,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.63  BV=1.55
    {33327,1696,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.73  BV=1.45
    {33327,1824,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.83  BV=1.35
    {33327,1952,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=5.93  BV=1.25
    {33327,2096,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.03  BV=1.15
    {33327,2256,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.14  BV=1.04
    {33327,2416,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.24  BV=0.94
    {33327,2576,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.33  BV=0.85
    {33327,2784,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.44  BV=0.74
    {33327,2976,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.54  BV=0.64
    {33327,3184,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.64  BV=0.55
    {33327,3408,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.73  BV=0.45
    {33327,3648,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.83  BV=0.35
    {33327,3920,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=6.94  BV=0.25
    {33327,4208,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.04  BV=0.14
    {33327,4496,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.13  BV=0.05
    {33327,4816,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.23  BV=-0.05
    {33327,5152,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.33  BV=-0.15
    {33327,5520,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.43  BV=-0.25
    {33327,5952,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.54  BV=-0.36
    {33327,6384,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.64  BV=-0.46
    {33327,6832,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.74  BV=-0.56
    {33327,7312,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.84  BV=-0.65
    {33327,7840,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=7.94  BV=-0.75
    {33327,8400,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.04  BV=-0.85
    {33327,8992,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.13  BV=-0.95
    {33327,9632,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.23  BV=-1.05
    {33327,10320,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.33  BV=-1.15
    {33327,11056,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.43  BV=-1.25
    {33327,11840,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.53  BV=-1.35
    {33327,12720,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.63  BV=-1.45
    {33327,13632,1024, 0, 0, 0},  //TV = 4.91(3174 lines)  AV=2.28  SV=8.73  BV=-1.55
    {41664,11680,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=8.51  BV=-1.65
    {41664,12544,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=8.61  BV=-1.75
    {41664,13440,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=8.71  BV=-1.85
    {41664,14400,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=8.81  BV=-1.95
    {41664,15424,1024, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=8.91  BV=-2.05
    {41664,15872,1064, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.01  BV=-2.15
    {41664,15872,1136, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.10  BV=-2.24
    {41664,15872,1224, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.21  BV=-2.35
    {41664,15872,1312, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.31  BV=-2.45
    {41664,15872,1400, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.41  BV=-2.55
    {41664,15872,1504, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.51  BV=-2.65
    {41664,15872,1608, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.61  BV=-2.75
    {41664,15872,1728, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.71  BV=-2.85
    {41664,15872,1848, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.81  BV=-2.95
    {41664,15872,2000, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=9.92  BV=-3.06
    {41664,15872,2136, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=10.01  BV=-3.15
    {41664,15872,2296, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=10.12  BV=-3.26
    {41664,15872,2456, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=10.22  BV=-3.36
    {41664,15872,2632, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=10.32  BV=-3.46
    {41664,15872,2824, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=10.42  BV=-3.56
    {41664,15872,3008, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=10.51  BV=-3.65
    {41664,15872,3224, 0, 0, 0},  //TV = 4.59(3968 lines)  AV=2.28  SV=10.61  BV=-3.75
    {50001,15872,2880, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=10.45  BV=-3.85
    {50001,15872,3088, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=10.55  BV=-3.95
    {58328,15872,2832, 0, 0, 0},  //TV = 4.10(5555 lines)  AV=2.28  SV=10.42  BV=-4.05
    {58328,15872,3040, 0, 0, 0},  //TV = 4.10(5555 lines)  AV=2.28  SV=10.52  BV=-4.15
    {66665,15872,2848, 0, 0, 0},  //TV = 3.91(6349 lines)  AV=2.28  SV=10.43  BV=-4.25
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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

static strEvPline sAEScene18PLineTable_50Hz =
{
{
    {95,1264,1024, 0, 0, 0},  //TV = 13.36(9 lines)  AV=2.28  SV=5.30  BV=10.33
    {105,1248,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.29  BV=10.21
    {105,1312,1024, 0, 0, 0},  //TV = 13.22(10 lines)  AV=2.28  SV=5.36  BV=10.13
    {116,1264,1024, 0, 0, 0},  //TV = 13.07(11 lines)  AV=2.28  SV=5.30  BV=10.04
    {126,1248,1024, 0, 0, 0},  //TV = 12.95(12 lines)  AV=2.28  SV=5.29  BV=9.94
    {137,1248,1024, 0, 0, 0},  //TV = 12.83(13 lines)  AV=2.28  SV=5.29  BV=9.82
    {147,1248,1024, 0, 0, 0},  //TV = 12.73(14 lines)  AV=2.28  SV=5.29  BV=9.72
    {158,1264,1024, 0, 0, 0},  //TV = 12.63(15 lines)  AV=2.28  SV=5.30  BV=9.60
    {168,1232,1024, 0, 0, 0},  /* TV = 12.54(16 lines)  AV=2.28  SV=5.27  BV=9.55 */
    {179,1248,1024, 0, 0, 0},  //TV = 12.45(17 lines)  AV=2.28  SV=5.29  BV=9.44
    {200,1200,1024, 0, 0, 0},  //TV = 12.29(19 lines)  AV=2.28  SV=5.23  BV=9.33
    {210,1216,1024, 0, 0, 0},  //TV = 12.22(20 lines)  AV=2.28  SV=5.25  BV=9.24
    {221,1232,1024, 0, 0, 0},  //TV = 12.14(21 lines)  AV=2.28  SV=5.27  BV=9.15
    {242,1200,1024, 0, 0, 0},  //TV = 12.01(23 lines)  AV=2.28  SV=5.23  BV=9.06
    {252,1248,1024, 0, 0, 0},  //TV = 11.95(24 lines)  AV=2.28  SV=5.29  BV=8.94
    {273,1232,1024, 0, 0, 0},  //TV = 11.84(26 lines)  AV=2.28  SV=5.27  BV=8.85
    {294,1216,1024, 0, 0, 0},  //TV = 11.73(28 lines)  AV=2.28  SV=5.25  BV=8.76
    {315,1216,1024, 0, 0, 0},  //TV = 11.63(30 lines)  AV=2.28  SV=5.25  BV=8.66
    {336,1232,1024, 0, 0, 0},  //TV = 11.54(32 lines)  AV=2.28  SV=5.27  BV=8.55
    {368,1200,1024, 0, 0, 0},  //TV = 11.41(35 lines)  AV=2.28  SV=5.23  BV=8.45
    {389,1216,1024, 0, 0, 0},  //TV = 11.33(37 lines)  AV=2.28  SV=5.25  BV=8.36
    {420,1216,1024, 0, 0, 0},  //TV = 11.22(40 lines)  AV=2.28  SV=5.25  BV=8.24
    {452,1200,1024, 0, 0, 0},  //TV = 11.11(43 lines)  AV=2.28  SV=5.23  BV=8.16
    {483,1216,1024, 0, 0, 0},  //TV = 11.02(46 lines)  AV=2.28  SV=5.25  BV=8.04
    {525,1200,1024, 0, 0, 0},  //TV = 10.90(50 lines)  AV=2.28  SV=5.23  BV=7.94
    {557,1200,1024, 0, 0, 0},  //TV = 10.81(53 lines)  AV=2.28  SV=5.23  BV=7.86
    {599,1200,1024, 0, 0, 0},  //TV = 10.71(57 lines)  AV=2.28  SV=5.23  BV=7.75
    {641,1200,1024, 0, 0, 0},  //TV = 10.61(61 lines)  AV=2.28  SV=5.23  BV=7.65
    {683,1216,1024, 0, 0, 0},  //TV = 10.52(65 lines)  AV=2.28  SV=5.25  BV=7.54
    {735,1200,1024, 0, 0, 0},  //TV = 10.41(70 lines)  AV=2.28  SV=5.23  BV=7.46
    {788,1200,1024, 0, 0, 0},  //TV = 10.31(75 lines)  AV=2.28  SV=5.23  BV=7.36
    {840,1216,1024, 0, 0, 0},  //TV = 10.22(80 lines)  AV=2.28  SV=5.25  BV=7.24
    {903,1200,1024, 0, 0, 0},  //TV = 10.11(86 lines)  AV=2.28  SV=5.23  BV=7.16
    {977,1200,1024, 0, 0, 0},  //TV = 10.00(93 lines)  AV=2.28  SV=5.23  BV=7.05
    {1040,1200,1024, 0, 0, 0},  //TV = 9.91(99 lines)  AV=2.28  SV=5.23  BV=6.96
    {1113,1200,1024, 0, 0, 0},  //TV = 9.81(106 lines)  AV=2.28  SV=5.23  BV=6.86
    {1197,1200,1024, 0, 0, 0},  //TV = 9.71(114 lines)  AV=2.28  SV=5.23  BV=6.75
    {1292,1200,1024, 0, 0, 0},  //TV = 9.60(123 lines)  AV=2.28  SV=5.23  BV=6.64
    {1376,1200,1024, 0, 0, 0},  //TV = 9.51(131 lines)  AV=2.28  SV=5.23  BV=6.55
    {1481,1200,1024, 0, 0, 0},  //TV = 9.40(141 lines)  AV=2.28  SV=5.23  BV=6.45
    {1575,1200,1024, 0, 0, 0},  //TV = 9.31(150 lines)  AV=2.28  SV=5.23  BV=6.36
    {1701,1200,1024, 0, 0, 0},  //TV = 9.20(162 lines)  AV=2.28  SV=5.23  BV=6.25
    {1827,1200,1024, 0, 0, 0},  //TV = 9.10(174 lines)  AV=2.28  SV=5.23  BV=6.14
    {1953,1200,1024, 0, 0, 0},  //TV = 9.00(186 lines)  AV=2.28  SV=5.23  BV=6.05
    {2090,1200,1024, 0, 0, 0},  //TV = 8.90(199 lines)  AV=2.28  SV=5.23  BV=5.95
    {2258,1200,1024, 0, 0, 0},  //TV = 8.79(215 lines)  AV=2.28  SV=5.23  BV=5.84
    {2415,1200,1024, 0, 0, 0},  //TV = 8.69(230 lines)  AV=2.28  SV=5.23  BV=5.74
    {2583,1200,1024, 0, 0, 0},  //TV = 8.60(246 lines)  AV=2.28  SV=5.23  BV=5.64
    {2762,1200,1024, 0, 0, 0},  //TV = 8.50(263 lines)  AV=2.28  SV=5.23  BV=5.55
    {2972,1200,1024, 0, 0, 0},  //TV = 8.39(283 lines)  AV=2.28  SV=5.23  BV=5.44
    {3182,1200,1024, 0, 0, 0},  //TV = 8.30(303 lines)  AV=2.28  SV=5.23  BV=5.34
    {3402,1200,1024, 0, 0, 0},  //TV = 8.20(324 lines)  AV=2.28  SV=5.23  BV=5.25
    {3644,1200,1024, 0, 0, 0},  //TV = 8.10(347 lines)  AV=2.28  SV=5.23  BV=5.15
    {3896,1200,1024, 0, 0, 0},  //TV = 8.00(371 lines)  AV=2.28  SV=5.23  BV=5.05
    {4179,1200,1024, 0, 0, 0},  //TV = 7.90(398 lines)  AV=2.28  SV=5.23  BV=4.95
    {4505,1200,1024, 0, 0, 0},  //TV = 7.79(429 lines)  AV=2.28  SV=5.23  BV=4.84
    {4820,1200,1024, 0, 0, 0},  //TV = 7.70(459 lines)  AV=2.28  SV=5.23  BV=4.74
    {5156,1200,1024, 0, 0, 0},  //TV = 7.60(491 lines)  AV=2.28  SV=5.23  BV=4.65
    {5523,1200,1024, 0, 0, 0},  //TV = 7.50(526 lines)  AV=2.28  SV=5.23  BV=4.55
    {5912,1200,1024, 0, 0, 0},  //TV = 7.40(563 lines)  AV=2.28  SV=5.23  BV=4.45
    {6332,1200,1024, 0, 0, 0},  //TV = 7.30(603 lines)  AV=2.28  SV=5.23  BV=4.35
    {6836,1200,1024, 0, 0, 0},  //TV = 7.19(651 lines)  AV=2.28  SV=5.23  BV=4.24
    {7319,1200,1024, 0, 0, 0},  //TV = 7.09(697 lines)  AV=2.28  SV=5.23  BV=4.14
    {7833,1200,1024, 0, 0, 0},  //TV = 7.00(746 lines)  AV=2.28  SV=5.23  BV=4.04
    {8400,1200,1024, 0, 0, 0},  //TV = 6.90(800 lines)  AV=2.28  SV=5.23  BV=3.94
    {8999,1200,1024, 0, 0, 0},  //TV = 6.80(857 lines)  AV=2.28  SV=5.23  BV=3.84
    {9639,1200,1024, 0, 0, 0},  //TV = 6.70(918 lines)  AV=2.28  SV=5.23  BV=3.74
    {9996,1232,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.27  BV=3.65
    {9996,1328,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.38  BV=3.54
    {9996,1424,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.48  BV=3.44
    {9996,1520,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.57  BV=3.35
    {9996,1632,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.67  BV=3.25
    {9996,1744,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.77  BV=3.15
    {9996,1872,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.87  BV=3.05
    {9996,2000,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=5.97  BV=2.95
    {9996,2144,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.07  BV=2.85
    {9996,2304,1024, 0, 0, 0},  //TV = 6.64(952 lines)  AV=2.28  SV=6.17  BV=2.75
    {20003,1232,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.27  BV=2.65
    {20003,1328,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.38  BV=2.54
    {20003,1424,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.48  BV=2.44
    {20003,1520,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.57  BV=2.35
    {20003,1632,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.67  BV=2.25
    {20003,1760,1024, 0, 0, 0},  //TV = 5.64(1905 lines)  AV=2.28  SV=5.78  BV=2.14
    {29999,1248,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.29  BV=2.05
    {29999,1344,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.39  BV=1.94
    {29999,1440,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.49  BV=1.84
    {29999,1536,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.58  BV=1.75
    {29999,1648,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.69  BV=1.65
    {29999,1760,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.78  BV=1.55
    {29999,1888,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.88  BV=1.45
    {29999,2032,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=5.99  BV=1.35
    {29999,2176,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.09  BV=1.25
    {29999,2320,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.18  BV=1.15
    {29999,2496,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.29  BV=1.05
    {29999,2672,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.38  BV=0.95
    {29999,2864,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.48  BV=0.85
    {29999,3072,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.58  BV=0.75
    {29999,3296,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.69  BV=0.65
    {29999,3536,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.79  BV=0.55
    {29999,3776,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.88  BV=0.45
    {29999,4048,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=6.98  BV=0.35
    {29999,4368,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.09  BV=0.24
    {29999,4672,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.19  BV=0.14
    {29999,5008,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.29  BV=0.04
    {29999,5360,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.39  BV=-0.05
    {29999,5728,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.48  BV=-0.15
    {29999,6144,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.58  BV=-0.25
    {29999,6624,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.69  BV=-0.36
    {29999,7088,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.79  BV=-0.46
    {29999,7584,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.89  BV=-0.55
    {29999,8128,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=7.99  BV=-0.65
    {29999,8704,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.09  BV=-0.75
    {29999,9328,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.19  BV=-0.85
    {29999,9984,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.29  BV=-0.95
    {29999,10704,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.39  BV=-1.05
    {29999,11472,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.49  BV=-1.15
    {29999,12288,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.58  BV=-1.25
    {29999,13248,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.69  BV=-1.36
    {29999,14128,1024, 0, 0, 0},  //TV = 5.06(2857 lines)  AV=2.28  SV=8.79  BV=-1.45
    {40005,11360,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=8.47  BV=-1.55
    {40005,12160,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=8.57  BV=-1.65
    {40005,13056,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=8.67  BV=-1.75
    {40005,14000,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=8.77  BV=-1.85
    {40005,14992,1024, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=8.87  BV=-1.95
    {40005,15872,1032, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=8.97  BV=-2.05
    {40005,15872,1104, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.06  BV=-2.14
    {40005,15872,1184, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.16  BV=-2.24
    {40005,15872,1272, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.27  BV=-2.35
    {40005,15872,1360, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.36  BV=-2.44
    {40005,15872,1464, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.47  BV=-2.55
    {40005,15872,1568, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.57  BV=-2.65
    {40005,15872,1680, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.67  BV=-2.75
    {40005,15872,1800, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.77  BV=-2.85
    {40005,15872,1928, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.87  BV=-2.95
    {40005,15872,2080, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=9.98  BV=-3.06
    {40005,15872,2232, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=10.08  BV=-3.16
    {40005,15872,2392, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=10.18  BV=-3.26
    {40005,15872,2560, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=10.28  BV=-3.36
    {40005,15872,2744, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=10.38  BV=-3.46
    {40005,15872,2944, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=10.48  BV=-3.56
    {40005,15872,3136, 0, 0, 0},  //TV = 4.64(3810 lines)  AV=2.28  SV=10.57  BV=-3.65
    {50001,15872,2688, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=10.35  BV=-3.75
    {50001,15872,2880, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=10.45  BV=-3.85
    {50001,15872,3088, 0, 0, 0},  //TV = 4.32(4762 lines)  AV=2.28  SV=10.55  BV=-3.95
    {59997,15872,2760, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=10.38  BV=-4.05
    {59997,15872,2952, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=10.48  BV=-4.15
    {59997,15872,3168, 0, 0, 0},  //TV = 4.06(5714 lines)  AV=2.28  SV=10.58  BV=-4.25
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
    {0, 0, 0, 0, 0, 0},  //reserved no used
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

static strAETable g_AE_SceneTable18 =
{
    AETABLE_SCENE_INDEX18, //eAETableID
    147, //u4TotalIndex
    20, //i4StrobeTrigerBV
    104, //i4MaxBV
    -43, //i4MinBV
    90, //i4EffectiveMaxBV
    -50, //i4EffectiveMinBV
    LIB3A_AE_ISO_SPEED_AUTO, //ISO
    sAEScene18PLineTable_60Hz,
    sAEScene18PLineTable_50Hz,
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
	20,    //u4StrobeTrigerBV
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
    {LIB3A_AE_SCENE_AUTO, {AETABLE_RPEVIEW_AUTO, AETABLE_CAPTURE_AUTO, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX5}},
    {LIB3A_AE_SCENE_NIGHT, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX1, AETABLE_VIDEO_NIGHT, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX1}},
    {LIB3A_AE_SCENE_ACTION, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX2, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX2}},
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
    {LIB3A_AE_SCENE_ISO1600, {AETABLE_RPEVIEW_AUTO, AETABLE_CAPTURE_ISO1600, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_SCENE_INDEX6, AETABLE_SCENE_INDEX7, AETABLE_SCENE_INDEX8, AETABLE_SCENE_INDEX9, AETABLE_CUSTOM5_AUTO, AETABLE_CAPTURE_ISO1600}},
    {LIB3A_AE_SCENE_ISO3200, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX6, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX6}},
    {LIB3A_AE_SCENE_ISO6400, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX7, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX7}},
    {LIB3A_AE_SCENE_ISO12800, {AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX8, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX8}},
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
    g_AE_SceneTable6,
    g_AE_SceneTable7,
    g_AE_SceneTable8,
    g_AE_SceneTable9,
    g_AE_SceneTable14,
    g_AE_SceneTable15,
    g_AE_SceneTable16,
    g_AE_SceneTable17,
    g_AE_SceneTable18,
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
    -50,
    {
        {1,10000,30,1024,1024},
        {2,30,30,1024,4096},
        {3,30,20,4096,4096},
        {4,20,20,4096,8192},
        {5,20,15,8192,8192},
        {6,15,15,8192,15872},
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

static strAEPLineNumInfo g_strAECaptureAutoPLineInfo =
{
    AETABLE_CAPTURE_AUTO,
    90,
    -30,
    {
        {1,10000,30,1024,1024},
        {2,30,30,1024,4096},
        {3,30,24,4096,4096},
        {4,24,24,4096,8192},
        {5,24,15,8192,8192},
        {6,15,15,8192,10240},
        {7,15,10,10240,10240},
        {8,10,10,10240,15872},
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
    -30,
    {
        {1,10000,30,1024,1024},
        {2,30,30,1024,3072},
        {3,30,24,3072,3072},
        {4,24,24,3072,8192},
        {5,24,15,8192,8192},
        {6,15,15,8192,15872},
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

static strAEPLineNumInfo g_strAEVideo1AutoPLineInfo =
{
    AETABLE_VIDEO1_AUTO,
    90,
    0,
    {
        {1,10000,121,1195,1195},
        {2,121,121,1195,20480},
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
    -30,
    {
        {1,10000,60,1195,1195},
        {2,60,60,1195,3072},
        {3,60,30,3072,3072},
        {4,30,30,3072,10240},
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
    -30,
    {
        {1,10000,60,1195,1195},
        {2,60,60,1195,3072},
        {3,60,30,3072,3072},
        {4,30,30,3072,10240},
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

static strAEPLineNumInfo g_strAECustom2AutoPLineInfo =
{
    AETABLE_CUSTOM2_AUTO,
    90,
    -30,
    {
        {1,10000,60,1195,1195},
        {2,60,60,1195,3072},
        {3,60,30,3072,3072},
        {4,30,30,3072,10240},
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

static strAEPLineNumInfo g_strAECustom3AutoPLineInfo =
{
    AETABLE_CUSTOM3_AUTO,
    90,
    -30,
    {
        {1,10000,60,1195,1195},
        {2,60,60,1195,3072},
        {3,60,30,3072,3072},
        {4,30,30,3072,10240},
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

static strAEPLineNumInfo g_strAECustom4AutoPLineInfo =
{
    AETABLE_CUSTOM4_AUTO,
    90,
    -30,
    {
        {1,10000,60,1195,1195},
        {2,60,60,1195,3072},
        {3,60,30,3072,3072},
        {4,30,30,3072,10240},
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

static strAEPLineNumInfo g_strAECustom5AutoPLineInfo =
{
    AETABLE_CUSTOM5_AUTO,
    90,
    -30,
    {
        {1,10000,60,1195,1195},
        {2,60,60,1195,3072},
        {3,60,30,3072,3072},
        {4,30,30,3072,10240},
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

static strAEPLineNumInfo g_strAEVideoNightPLineInfo =
{
    AETABLE_VIDEO_NIGHT,
    90,
    -10,
    {
        {1,10000,30,1195,1195},
        {2,30,30,1195,12288},
        {3,30,15,12288,12288},
        {4,15,15,12288,35200},
        {5,15,10,35200,35200},
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

static strAEPLineNumInfo g_strAECaptureISO100PLineInfo =
{
    AETABLE_CAPTURE_ISO100,
    90,
    0,
    {
        {1,10000,10,1195,1195},
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
    -30,
    {
        {1,10000,2000,1195,1195},
        {2,2000,2000,1195,2048},
        {3,2000,10,2048,2048},
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
    -30,
    {
        {1,10000,2000,1195,1195},
        {2,2000,2000,1195,4096},
        {3,2000,10,4096,4096},
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
        {1,10000,2000,1195,1195},
        {2,2000,2000,1195,8192},
        {3,2000,5,8192,8192},
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
        {1,10000,2000,1195,1195},
        {2,2000,2000,1195,16384},
        {3,2000,5,16384,16384},
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
    -30,
    {
        {1,10000,30,1195,1195},
        {2,30,30,1195,12288},
        {3,30,15,12288,12288},
        {4,15,15,12288,46080},
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

static strAEPLineNumInfo g_strAEScene1PLineInfo =
{
    AETABLE_SCENE_INDEX1,
    90,
    -50,
    {
        {1,10000,30,1195,1195},
        {2,30,30,1195,12288},
        {3,30,15,12288,12288},
        {4,15,15,12288,46080},
        {5,15,10,46080,46080},
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

static strAEPLineNumInfo g_strAEScene2PLineInfo =
{
    AETABLE_SCENE_INDEX2,
    90,
    -30,
    {
        {1,10000,100,1195,1195},
        {2,100,100,1195,4096},
        {3,100,30,4096,4096},
        {4,30,30,4096,16384},
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

static strAEPLineNumInfo g_strAEScene3PLineInfo =
{
    AETABLE_SCENE_INDEX3,
    90,
    -30,
    {
        {1,10000,30,1195,1195},
        {2,30,30,1195,2048},
        {3,30,15,2048,2048},
        {4,15,15,2048,4096},
        {5,15,10,4096,4096},
        {6,10,10,4096,12288},
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

static strAEPLineNumInfo g_strAEScene4PLineInfo =
{
    AETABLE_SCENE_INDEX4,
    90,
    -30,
    {
        {1,5,5,1706,1706},
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
    -30,
    {
        {1,10000,30,1195,1195},
        {2,30,30,1195,12288},
        {3,30,24,12288,12288},
        {4,24,24,12288,46080},
        {5,24,15,46080,46080},
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

static strAEPLineNumInfo g_strAEScene6PLineInfo =
{
    AETABLE_SCENE_INDEX6,
    90,
    0,
    {
        {1,10000,2000,1195,1195},
        {2,2000,2000,1195,32768},
        {3,2000,5,32768,32768},
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

static strAEPLineNumInfo g_strAEScene7PLineInfo =
{
    AETABLE_SCENE_INDEX7,
    90,
    0,
    {
        {1,10000,2000,1195,1195},
        {2,2000,2000,1195,65536},
        {3,2000,10,65536,65536},
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

static strAEPLineNumInfo g_strAEScene8PLineInfo =
{
    AETABLE_SCENE_INDEX8,
    90,
    0,
    {
        {1,10000,2000,1195,1195},
        {2,2000,2000,1195,131072},
        {3,2000,20,131072,131072},
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

static strAEPLineNumInfo g_strAEScene9PLineInfo =
{
    AETABLE_SCENE_INDEX9,
    90,
    0,
    {
        {1,10000,60,1195,1195},
        {2,60,60,1195,2048},
        {3,60,40,2048,2048},
        {4,40,40,2048,46080},
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

static strAEPLineNumInfo g_strAEScene14PLineInfo =
{
    AETABLE_SCENE_INDEX14,
    0,
    0,
    {
        {1,10000,60,1136,1136},
        {2,60,60,1136,2048},
        {3,60,50,2048,2048},
        {4,50,50,2048,5120},
        {5,50,33,5120,5120},
        {6,33,33,5120,16384},
        {7,33,25,16384,16384},
        {8,25,25,16384,22528},
        {9,25,17,22528,22528},
        {10,17,17,22528,32768},
        {11,0,0,0,0},
        {12,0,0,0,0},
        {13,0,0,0,0},
        {14,0,0,0,0},
        {15,0,0,0,0},
    }
};

static strAEPLineNumInfo g_strAEScene15PLineInfo =
{
    AETABLE_SCENE_INDEX15,
    90,
    -50,
    {
        {1,10000,30,1195,1195},
        {2,30,30,1195,12288},
        {3,30,24,12288,12288},
        {4,24,24,12288,46080},
        {5,24,15,46080,46080},
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

static strAEPLineNumInfo g_strAEScene16PLineInfo =
{
    AETABLE_SCENE_INDEX16,
    90,
    -50,
    {
        {1,10000,30,1195,1195},
        {2,30,30,1195,12288},
        {3,30,24,12288,12288},
        {4,24,24,12288,46080},
        {5,24,15,46080,46080},
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

static strAEPLineNumInfo g_strAEScene17PLineInfo =
{
    AETABLE_SCENE_INDEX17,
    90,
    -50,
    {
        {1,10000,30,1195,1195},
        {2,30,30,1195,12288},
        {3,30,24,12288,12288},
        {4,24,24,12288,46080},
        {5,24,15,46080,46080},
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

static strAEPLineNumInfo g_strAEScene18PLineInfo =
{
    AETABLE_SCENE_INDEX18,
    90,
    -50,
    {
        {1,10000,30,1195,1195},
        {2,30,30,1195,12288},
        {3,30,24,12288,12288},
        {4,24,24,12288,46080},
        {5,24,15,46080,46080},
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
        g_strAEScene6PLineInfo,
        g_strAEScene7PLineInfo,
        g_strAEScene8PLineInfo,
        g_strAEScene9PLineInfo,
        g_strAEScene14PLineInfo,
        g_strAEScene15PLineInfo,
        g_strAEScene16PLineInfo,
        g_strAEScene17PLineInfo,
        g_strAEScene18PLineInfo,
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
MinGain,1024
MaxGain,15872
MiniISOGain,100
GainStepUnitInTotalRange,16
PreviewExposureLineUnit,10500
PreviewMaxFrameRate,30
VideoExposureLineUnit,10371
VideoMaxFrameRate,30
VideoToPreviewSensitivityRatio,1024
CaptureExposureLineUnit,10500
CaptureMaxFrameRate,30
CaptureToPreviewSensitivityRatio,1024
Video1ExposureLineUnit,10364
Video1MaxFrameRate,120
Video1ToPreviewSensitivityRatio,1024
Video2ExposureLineUnit,10364
Video2MaxFrameRate,120
Video2ToPreviewSensitivityRatio,1024
Custom1ExposureLineUnit,0
Custom1MaxFrameRate,0
Custom1ToPreviewSensitivityRatio,0
Custom2ExposureLineUnit,0
Custom2MaxFrameRate,0
Custom2ToPreviewSensitivityRatio,0
Custom3ExposureLineUnit,0
Custom3MaxFrameRate,0
Custom3ToPreviewSensitivityRatio,0
Custom4ExposureLineUnit,0
Custom4MaxFrameRate,0
Custom4ToPreviewSensitivityRatio,0
Custom5ExposureLineUnit,0
Custom5MaxFrameRate,0
Custom5ToPreviewSensitivityRatio,0
FocusLength,346
Fno,22

// Preview table -- Use preview sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_RPEVIEW_AUTO
90,-50
1,10000,30,1024,1024
2,30,30,1024,4096
3,30,20,4096,4096
4,20,20,4096,8192
5,20,15,8192,8192
6,15,15,8192,15872
AETABLE_END

// Capture table -- Use capture sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_CAPTURE_AUTO
90,-30
1,10000,30,1024,1024
2,30,30,1024,4096
3,30,24,4096,4096
4,24,24,4096,8192
5,24,15,8192,8192
6,15,15,8192,10240
7,15,10,10240,10240
8,10,10,10240,15872
AETABLE_END

// Video table -- Use video sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_VIDEO_AUTO
90,-30
1,10000,30,1024,1024
2,30,30,1024,3072
3,30,24,3072,3072
4,24,24,3072,8192
5,24,15,8192,8192
6,15,15,8192,15872
AETABLE_END

// Video1 table -- Use video1 sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_VIDEO1_AUTO
90,0
1,10000,121,1195,1195
2,121,121,1195,20480
AETABLE_END

// Video2 table -- Use video2 sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_VIDEO2_AUTO
90,-30
1,10000,60,1195,1195
2,60,60,1195,3072
3,60,30,3072,3072
4,30,30,3072,10240
AETABLE_END

// Custom1 table -- Use custom1 sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_CUSTOM1_AUTO
90,-30
1,10000,60,1195,1195
2,60,60,1195,3072
3,60,30,3072,3072
4,30,30,3072,10240
AETABLE_END

// Custom2 table -- Use custom2 sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_CUSTOM2_AUTO
90,-30
1,10000,60,1195,1195
2,60,60,1195,3072
3,60,30,3072,3072
4,30,30,3072,10240
AETABLE_END

// Custom3 table -- Use custom3 sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_CUSTOM3_AUTO
90,-30
1,10000,60,1195,1195
2,60,60,1195,3072
3,60,30,3072,3072
4,30,30,3072,10240
AETABLE_END

// Custom4 table -- Use custom4 sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_CUSTOM4_AUTO
90,-30
1,10000,60,1195,1195
2,60,60,1195,3072
3,60,30,3072,3072
4,30,30,3072,10240
AETABLE_END

// Custom5 table -- Use custom5 sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_CUSTOM5_AUTO
90,-30
1,10000,60,1195,1195
2,60,60,1195,3072
3,60,30,3072,3072
4,30,30,3072,10240
AETABLE_END

// Video Night table -- Use video sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_VIDEO_NIGHT
90,-10
1,10000,30,1195,1195
2,30,30,1195,12288
3,30,15,12288,12288
4,15,15,12288,35200
5,15,10,35200,35200
AETABLE_END

// Capture ISO100 -- Use capture sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_CAPTURE_ISO100
90,0
1,10000,10,1195,1195
AETABLE_END

// Capture ISO200 -- Use capture sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_CAPTURE_ISO200
90,-30
1,10000,2000,1195,1195
2,2000,2000,1195,2048
3,2000,10,2048,2048
AETABLE_END

// Capture ISO400 -- Use capture sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_CAPTURE_ISO400
90,-30
1,10000,2000,1195,1195
2,2000,2000,1195,4096
3,2000,10,4096,4096
AETABLE_END

// Capture ISO800 -- Use capture sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_CAPTURE_ISO800
90,0
1,10000,2000,1195,1195
2,2000,2000,1195,8192
3,2000,5,8192,8192
AETABLE_END

// Capture ISO1600 -- Use capture sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_CAPTURE_ISO1600
90,0
1,10000,2000,1195,1195
2,2000,2000,1195,16384
3,2000,5,16384,16384
AETABLE_END

// Strobe -- Use capture sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_STROBE
90,-30
1,10000,30,1195,1195
2,30,30,1195,12288
3,30,15,12288,12288
4,15,15,12288,46080
AETABLE_END

// Table1 PLine -- Use capture sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_SCENE_INDEX1
90,-50
1,10000,30,1195,1195
2,30,30,1195,12288
3,30,15,12288,12288
4,15,15,12288,46080
5,15,10,46080,46080
AETABLE_END

// Table2 PLine -- Use capture sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_SCENE_INDEX2
90,-30
1,10000,100,1195,1195
2,100,100,1195,4096
3,100,30,4096,4096
4,30,30,4096,16384
AETABLE_END

// Table3 PLine -- Use capture sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_SCENE_INDEX3
90,-30
1,10000,30,1195,1195
2,30,30,1195,2048
3,30,15,2048,2048
4,15,15,2048,4096
5,15,10,4096,4096
6,10,10,4096,12288
AETABLE_END

// Table4 PLine -- Use capture sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_SCENE_INDEX4
90,-30
1,5,5,1706,1706
AETABLE_END

// Table5 PLine -- Use capture sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_SCENE_INDEX5
90,-30
1,10000,30,1195,1195
2,30,30,1195,12288
3,30,24,12288,12288
4,24,24,12288,46080
5,24,15,46080,46080
AETABLE_END

// Table6 PLine -- Use capture sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_SCENE_INDEX6
90,0
1,10000,2000,1195,1195
2,2000,2000,1195,32768
3,2000,5,32768,32768
AETABLE_END

// Table7 PLine -- Use capture sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_SCENE_INDEX7
90,0
1,10000,2000,1195,1195
2,2000,2000,1195,65536
3,2000,10,65536,65536
AETABLE_END

// Table8 PLine -- Use capture sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_SCENE_INDEX8
90,0
1,10000,2000,1195,1195
2,2000,2000,1195,131072
3,2000,20,131072,131072
AETABLE_END

// Table9 PLine -- Use capture sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_SCENE_INDEX9
90,0
1,10000,60,1195,1195
2,60,60,1195,2048
3,60,40,2048,2048
4,40,40,2048,46080
AETABLE_END

// Table14 PLine -- Use capture sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_SCENE_INDEX14
0,0
1,10000,60,1136,1136
2,60,60,1136,2048
3,60,50,2048,2048
4,50,50,2048,5120
5,50,33,5120,5120
6,33,33,5120,16384
7,33,25,16384,16384
8,25,25,16384,22528
9,25,17,22528,22528
10,17,17,22528,32768
AETABLE_END

// Table15 PLine -- Use capture sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_SCENE_INDEX15
90,-50
1,10000,30,1195,1195
2,30,30,1195,12288
3,30,24,12288,12288
4,24,24,12288,46080
5,24,15,46080,46080
AETABLE_END

// Table16 PLine -- Use capture sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_SCENE_INDEX16
90,-50
1,10000,30,1195,1195
2,30,30,1195,12288
3,30,24,12288,12288
4,24,24,12288,46080
5,24,15,46080,46080
AETABLE_END

// Table17 PLine -- Use capture sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_SCENE_INDEX17
90,-50
1,10000,30,1195,1195
2,30,30,1195,12288
3,30,24,12288,12288
4,24,24,12288,46080
5,24,15,46080,46080
AETABLE_END

// Table18 PLine -- Use capture sensor setting
// Index MiniExp1/T MaxExp1/T MinGain MaxGain
AETABLE_SCENE_INDEX18
90,-50
1,10000,30,1195,1195
2,30,30,1195,12288
3,30,24,12288,12288
4,24,24,12288,46080
5,24,15,46080,46080
AETABLE_END

AE_SCENE_AUTO, AETABLE_RPEVIEW_AUTO, AETABLE_CAPTURE_AUTO, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX5
AE_SCENE_NIGHT, AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX1, AETABLE_VIDEO_NIGHT, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX1
AE_SCENE_ACTION, AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX2, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX2
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
AE_SCENE_ISO1600, AETABLE_RPEVIEW_AUTO, AETABLE_CAPTURE_ISO1600, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_SCENE_INDEX6, AETABLE_SCENE_INDEX7, AETABLE_SCENE_INDEX8, AETABLE_SCENE_INDEX9, AETABLE_CUSTOM5_AUTO, AETABLE_CAPTURE_ISO1600
AE_SCENE_ISO3200, AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX6, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX6
AE_SCENE_ISO6400, AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX7, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX7
AE_SCENE_ISO12800, AETABLE_RPEVIEW_AUTO, AETABLE_SCENE_INDEX8, AETABLE_VIDEO_AUTO, AETABLE_VIDEO1_AUTO, AETABLE_VIDEO2_AUTO, AETABLE_CUSTOM1_AUTO, AETABLE_CUSTOM2_AUTO, AETABLE_CUSTOM3_AUTO, AETABLE_CUSTOM4_AUTO, AETABLE_CUSTOM5_AUTO, AETABLE_SCENE_INDEX8
#endif
