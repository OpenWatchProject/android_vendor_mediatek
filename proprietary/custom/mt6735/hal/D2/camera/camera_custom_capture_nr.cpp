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

#include "camera_custom_capture_nr.h"
#include <mtkcam/Modes.h>
#include <mtkcam/hal/IHalSensor.h>
#include <math.h>

using namespace NSCam;

static swnr_param custom_params_nr_main[eNUM_OF_SWNR_IDX]
{
    // iso 100
   {0  , 8     , 32    , 28    , 24  , 20  ,55552 , 64   , 128 , 192 ,
    0  , 0     , 16    , 0     , 0   , 40  , 40   , 0    , 220 , 220 ,
    0  , 55552 , 56905 , 46189 , 256 , 64  , 64   , 256  , 0   , 0   ,
    40 , 40    , 10    , 0     , 0   , 1   , 0    , 0    , 0   , -4  ,
    -4 , -4    , -4    , 0     , 2   , 32  , 0    , 0    , 2   , 0   },
    // iso 200
    {0  , 8     , 32    , 28    , 24  , 20  ,55552 , 64   , 128 , 192 ,
     0  , 0     , 16    , 0     , 0   , 40  , 40   , 0    , 220 , 220 ,
     0  , 55552 , 56905 , 46189 , 256 , 64  , 64   , 256  , 0   , 0   ,
     40 , 40    , 10    , 0     , 0   , 1   , 0    , 0    , 0   , -4  ,
     -4 , -4    , -4    , 0     , 2   , 32  , 0    , 0    , 2   , 0   },
    // iso 400
    {0  , 8     , 32    , 28    , 24  , 20  ,55552 , 64   , 128 , 192 ,
     0  , 0     , 16    , 0     , 0   , 40  , 40   , 0    , 220 , 220 ,
     0  , 55552 , 56905 , 46189 , 256 , 64  , 64   , 256  , 0   , 0   ,
     40 , 40    , 10    , 0     , 0   , 1   , 0    , 0    , 0   , -4  ,
     -4 , -4    , -4    , 0     , 2   , 32  , 0    , 0    , 2   , 0   },
    // iso 800
    {0  , 8     , 32    , 28    , 24  , 20  ,55552 , 64   , 128 , 192 ,
     0  , 0     , 16    , 0     , 0   , 40  , 40   , 0    , 220 , 220 ,
     0  , 55552 , 56905 , 46189 , 256 , 64  , 64   , 256  , 0   , 0   ,
     40 , 40    , 10    , 0     , 0   , 1   , 0    , 0    , 0   , -4  ,
     -4 , -4    , -4    , 0     , 2   , 32  , 0    , 0    , 2   , 0   },
    // iso 1200
    {0  , 8     , 32    , 28    , 24  , 20  ,55552 , 64   , 128 , 192 ,
     0  , 0     , 16    , 0     , 0   , 40  , 40   , 0    , 220 , 220 ,
     0  , 55552 , 56905 , 46189 , 256 , 64  , 64   , 256  , 0   , 0   ,
     40 , 40    , 10    , 0     , 0   , 1   , 0    , 0    , 0   , -4  ,
     -4 , -4    , -4    , 0     , 2   , 32  , 0    , 0    , 2   , 0   },
    // iso 1600
    {0  , 8     , 32    , 28    , 24  , 20  ,55552 , 64   , 128 , 192 ,
     0  , 0     , 16    , 0     , 0   , 40  , 40   , 0    , 220 , 220 ,
     0  , 55552 , 56905 , 46189 , 256 , 64  , 64   , 256  , 0   , 0   ,
     40 , 40    , 10    , 0     , 0   , 1   , 0    , 0    , 0   , -4  ,
     -4 , -4    , -4    , 0     , 2   , 32  , 0    , 0    , 2   , 0   },
    // iso 2000
    {0  , 8     , 32    , 28    , 24  , 20  ,55552 , 64   , 128 , 192 ,
     0  , 0     , 16    , 0     , 0   , 40  , 40   , 0    , 220 , 220 ,
     0  , 55552 , 56905 , 46189 , 256 , 64  , 64   , 256  , 0   , 0   ,
     40 , 40    , 10    , 0     , 0   , 1   , 0    , 0    , 0   , -4  ,
     -4 , -4    , -4    , 0     , 2   , 32  , 0    , 0    , 2   , 0   },
    // iso 2400
    {0  , 8     , 32    , 28    , 24  , 20  ,55552 , 64   , 128 , 192 ,
     0  , 0     , 16    , 0     , 0   , 40  , 40   , 0    , 220 , 220 ,
     0  , 55552 , 56905 , 46189 , 256 , 64  , 64   , 256  , 0   , 0   ,
     40 , 40    , 10    , 0     , 0   , 1   , 0    , 0    , 0   , -4  ,
     -4 , -4    , -4    , 0     , 2   , 32  , 0    , 0    , 2   , 0   },
    // iso 2800
    {0  , 8     , 32    , 28    , 24  , 20  ,55552 , 64   , 128 , 192 ,
     0  , 0     , 16    , 0     , 0   , 40  , 40   , 0    , 220 , 220 ,
     0  , 55552 , 56905 , 46189 , 256 , 64  , 64   , 256  , 0   , 0   ,
     40 , 40    , 10    , 0     , 0   , 1   , 0    , 0    , 0   , -4  ,
     -4 , -4    , -4    , 0     , 2   , 32  , 0    , 0    , 2   , 0   },
    // iso 3200
    {0  , 8     , 32    , 28    , 24  , 20  ,55552 , 64   , 128 , 192 ,
     0  , 0     , 16    , 0     , 0   , 40  , 40   , 0    , 220 , 220 ,
     0  , 55552 , 56905 , 46189 , 256 , 64  , 64   , 256  , 0   , 0   ,
     40 , 40    , 10    , 0     , 0   , 1   , 0    , 0    , 0   , -4  ,
     -4 , -4    , -4    , 0     , 2   , 32  , 0    , 0    , 2   , 0   },
};

static swnr_param custom_params_nr_sub[eNUM_OF_SWNR_IDX]
{
    // iso 100
   {0  , 8     , 32    , 28    , 24  , 20  ,55552 , 64   , 128 , 192 ,
    0  , 0     , 16    , 0     , 0   , 40  , 40   , 0    , 220 , 220 ,
    0  , 55552 , 56905 , 46189 , 256 , 64  , 64   , 256  , 0   , 0   ,
    40 , 40    , 10    , 0     , 0   , 1   , 0    , 0    , 0   , -4  ,
    -4 , -4    , -4    , 0     , 2   , 32  , 0    , 0    , 2   , 0   },
    // iso 200
    {0  , 8     , 32    , 28    , 24  , 20  ,55552 , 64   , 128 , 192 ,
     0  , 0     , 16    , 0     , 0   , 40  , 40   , 0    , 220 , 220 ,
     0  , 55552 , 56905 , 46189 , 256 , 64  , 64   , 256  , 0   , 0   ,
     40 , 40    , 10    , 0     , 0   , 1   , 0    , 0    , 0   , -4  ,
     -4 , -4    , -4    , 0     , 2   , 32  , 0    , 0    , 2   , 0   },
    // iso 400
    {0  , 8     , 32    , 28    , 24  , 20  ,55552 , 64   , 128 , 192 ,
     0  , 0     , 16    , 0     , 0   , 40  , 40   , 0    , 220 , 220 ,
     0  , 55552 , 56905 , 46189 , 256 , 64  , 64   , 256  , 0   , 0   ,
     40 , 40    , 10    , 0     , 0   , 1   , 0    , 0    , 0   , -4  ,
     -4 , -4    , -4    , 0     , 2   , 32  , 0    , 0    , 2   , 0   },
    // iso 800
    {0  , 8     , 32    , 28    , 24  , 20  ,55552 , 64   , 128 , 192 ,
     0  , 0     , 16    , 0     , 0   , 40  , 40   , 0    , 220 , 220 ,
     0  , 55552 , 56905 , 46189 , 256 , 64  , 64   , 256  , 0   , 0   ,
     40 , 40    , 10    , 0     , 0   , 1   , 0    , 0    , 0   , -4  ,
     -4 , -4    , -4    , 0     , 2   , 32  , 0    , 0    , 2   , 0   },
    // iso 1200
    {0  , 8     , 32    , 28    , 24  , 20  ,55552 , 64   , 128 , 192 ,
     0  , 0     , 16    , 0     , 0   , 40  , 40   , 0    , 220 , 220 ,
     0  , 55552 , 56905 , 46189 , 256 , 64  , 64   , 256  , 0   , 0   ,
     40 , 40    , 10    , 0     , 0   , 1   , 0    , 0    , 0   , -4  ,
     -4 , -4    , -4    , 0     , 2   , 32  , 0    , 0    , 2   , 0   },
    // iso 1600
    {0  , 8     , 32    , 28    , 24  , 20  ,55552 , 64   , 128 , 192 ,
     0  , 0     , 16    , 0     , 0   , 40  , 40   , 0    , 220 , 220 ,
     0  , 55552 , 56905 , 46189 , 256 , 64  , 64   , 256  , 0   , 0   ,
     40 , 40    , 10    , 0     , 0   , 1   , 0    , 0    , 0   , -4  ,
     -4 , -4    , -4    , 0     , 2   , 32  , 0    , 0    , 2   , 0   },
    // iso 2000
    {0  , 8     , 32    , 28    , 24  , 20  ,55552 , 64   , 128 , 192 ,
     0  , 0     , 16    , 0     , 0   , 40  , 40   , 0    , 220 , 220 ,
     0  , 55552 , 56905 , 46189 , 256 , 64  , 64   , 256  , 0   , 0   ,
     40 , 40    , 10    , 0     , 0   , 1   , 0    , 0    , 0   , -4  ,
     -4 , -4    , -4    , 0     , 2   , 32  , 0    , 0    , 2   , 0   },
    // iso 2400
    {0  , 8     , 32    , 28    , 24  , 20  ,55552 , 64   , 128 , 192 ,
     0  , 0     , 16    , 0     , 0   , 40  , 40   , 0    , 220 , 220 ,
     0  , 55552 , 56905 , 46189 , 256 , 64  , 64   , 256  , 0   , 0   ,
     40 , 40    , 10    , 0     , 0   , 1   , 0    , 0    , 0   , -4  ,
     -4 , -4    , -4    , 0     , 2   , 32  , 0    , 0    , 2   , 0   },
    // iso 2800
    {0  , 8     , 32    , 28    , 24  , 20  ,55552 , 64   , 128 , 192 ,
     0  , 0     , 16    , 0     , 0   , 40  , 40   , 0    , 220 , 220 ,
     0  , 55552 , 56905 , 46189 , 256 , 64  , 64   , 256  , 0   , 0   ,
     40 , 40    , 10    , 0     , 0   , 1   , 0    , 0    , 0   , -4  ,
     -4 , -4    , -4    , 0     , 2   , 32  , 0    , 0    , 2   , 0   },
    // iso 3200
    {0  , 8     , 32    , 28    , 24  , 20  ,55552 , 64   , 128 , 192 ,
     0  , 0     , 16    , 0     , 0   , 40  , 40   , 0    , 220 , 220 ,
     0  , 55552 , 56905 , 46189 , 256 , 64  , 64   , 256  , 0   , 0   ,
     40 , 40    , 10    , 0     , 0   , 1   , 0    , 0    , 0   , -4  ,
     -4 , -4    , -4    , 0     , 2   , 32  , 0    , 0    , 2   , 0   },
};

static swnr_param custom_params_nr_main_2[eNUM_OF_SWNR_IDX]
{
    // iso 100
   {0  , 8     , 32    , 28    , 24  , 20  ,55552 , 64   , 128 , 192 ,
    0  , 0     , 16    , 0     , 0   , 40  , 40   , 0    , 220 , 220 ,
    0  , 55552 , 56905 , 46189 , 256 , 64  , 64   , 256  , 0   , 0   ,
    40 , 40    , 10    , 0     , 0   , 1   , 0    , 0    , 0   , -4  ,
    -4 , -4    , -4    , 0     , 2   , 32  , 0    , 0    , 2   , 0   },
    // iso 200
    {0  , 8     , 32    , 28    , 24  , 20  ,55552 , 64   , 128 , 192 ,
     0  , 0     , 16    , 0     , 0   , 40  , 40   , 0    , 220 , 220 ,
     0  , 55552 , 56905 , 46189 , 256 , 64  , 64   , 256  , 0   , 0   ,
     40 , 40    , 10    , 0     , 0   , 1   , 0    , 0    , 0   , -4  ,
     -4 , -4    , -4    , 0     , 2   , 32  , 0    , 0    , 2   , 0   },
    // iso 400
    {0  , 8     , 32    , 28    , 24  , 20  ,55552 , 64   , 128 , 192 ,
     0  , 0     , 16    , 0     , 0   , 40  , 40   , 0    , 220 , 220 ,
     0  , 55552 , 56905 , 46189 , 256 , 64  , 64   , 256  , 0   , 0   ,
     40 , 40    , 10    , 0     , 0   , 1   , 0    , 0    , 0   , -4  ,
     -4 , -4    , -4    , 0     , 2   , 32  , 0    , 0    , 2   , 0   },
    // iso 800
    {0  , 8     , 32    , 28    , 24  , 20  ,55552 , 64   , 128 , 192 ,
     0  , 0     , 16    , 0     , 0   , 40  , 40   , 0    , 220 , 220 ,
     0  , 55552 , 56905 , 46189 , 256 , 64  , 64   , 256  , 0   , 0   ,
     40 , 40    , 10    , 0     , 0   , 1   , 0    , 0    , 0   , -4  ,
     -4 , -4    , -4    , 0     , 2   , 32  , 0    , 0    , 2   , 0   },
    // iso 1200
    {0  , 8     , 32    , 28    , 24  , 20  ,55552 , 64   , 128 , 192 ,
     0  , 0     , 16    , 0     , 0   , 40  , 40   , 0    , 220 , 220 ,
     0  , 55552 , 56905 , 46189 , 256 , 64  , 64   , 256  , 0   , 0   ,
     40 , 40    , 10    , 0     , 0   , 1   , 0    , 0    , 0   , -4  ,
     -4 , -4    , -4    , 0     , 2   , 32  , 0    , 0    , 2   , 0   },
    // iso 1600
    {0  , 8     , 32    , 28    , 24  , 20  ,55552 , 64   , 128 , 192 ,
     0  , 0     , 16    , 0     , 0   , 40  , 40   , 0    , 220 , 220 ,
     0  , 55552 , 56905 , 46189 , 256 , 64  , 64   , 256  , 0   , 0   ,
     40 , 40    , 10    , 0     , 0   , 1   , 0    , 0    , 0   , -4  ,
     -4 , -4    , -4    , 0     , 2   , 32  , 0    , 0    , 2   , 0   },
    // iso 2000
    {0  , 8     , 32    , 28    , 24  , 20  ,55552 , 64   , 128 , 192 ,
     0  , 0     , 16    , 0     , 0   , 40  , 40   , 0    , 220 , 220 ,
     0  , 55552 , 56905 , 46189 , 256 , 64  , 64   , 256  , 0   , 0   ,
     40 , 40    , 10    , 0     , 0   , 1   , 0    , 0    , 0   , -4  ,
     -4 , -4    , -4    , 0     , 2   , 32  , 0    , 0    , 2   , 0   },
    // iso 2400
    {0  , 8     , 32    , 28    , 24  , 20  ,55552 , 64   , 128 , 192 ,
     0  , 0     , 16    , 0     , 0   , 40  , 40   , 0    , 220 , 220 ,
     0  , 55552 , 56905 , 46189 , 256 , 64  , 64   , 256  , 0   , 0   ,
     40 , 40    , 10    , 0     , 0   , 1   , 0    , 0    , 0   , -4  ,
     -4 , -4    , -4    , 0     , 2   , 32  , 0    , 0    , 2   , 0   },
    // iso 2800
    {0  , 8     , 32    , 28    , 24  , 20  ,55552 , 64   , 128 , 192 ,
     0  , 0     , 16    , 0     , 0   , 40  , 40   , 0    , 220 , 220 ,
     0  , 55552 , 56905 , 46189 , 256 , 64  , 64   , 256  , 0   , 0   ,
     40 , 40    , 10    , 0     , 0   , 1   , 0    , 0    , 0   , -4  ,
     -4 , -4    , -4    , 0     , 2   , 32  , 0    , 0    , 2   , 0   },
    // iso 3200
    {0  , 8     , 32    , 28    , 24  , 20  ,55552 , 64   , 128 , 192 ,
     0  , 0     , 16    , 0     , 0   , 40  , 40   , 0    , 220 , 220 ,
     0  , 55552 , 56905 , 46189 , 256 , 64  , 64   , 256  , 0   , 0   ,
     40 , 40    , 10    , 0     , 0   , 1   , 0    , 0    , 0   , -4  ,
     -4 , -4    , -4    , 0     , 2   , 32  , 0    , 0    , 2   , 0   },
};

static swnr2_param custom_params_nr2_main[eNUM_OF_SWNR_IDX]
{
    // iso 100, 200, 400, 800
    {300, 50, 0, 8, 7, 0},  {300, 50, 0, 8, 7, 0},  {300, 50, 0, 8, 7, 0},  {300, 50, 0, 8, 7, 0},
    // iso 1200, 1600, 2000, 2400
    {300, 80, 0, 8, 7, 0},  {300, 100, 0, 8, 7, 0}, {300, 100, 0, 8, 7, 0}, {300, 120, 0, 8, 7, 0},
    // iso 2800, 3200
    {300, 120, 0, 8, 7, 0}, {300, 140, 0, 8, 7, 0},
};

static swnr2_param custom_params_nr2_sub[eNUM_OF_SWNR_IDX]
{
    // iso 100, 200, 400, 800
    {300, 50, 0, 8, 7, 0},  {300, 50, 0, 8, 7, 0},  {300, 50, 0, 8, 7, 0},  {300, 50, 0, 8, 7, 0},
    // iso 1200, 1600, 2000, 2400
    {300, 80, 0, 8, 7, 0},  {300, 100, 0, 8, 7, 0}, {300, 100, 0, 8, 7, 0}, {300, 120, 0, 8, 7, 0},
    // iso 2800, 3200
    {300, 120, 0, 8, 7, 0}, {300, 140, 0, 8, 7, 0},

};

static swnr2_param custom_params_nr2_main_2[eNUM_OF_SWNR_IDX]
{
    // iso 100, 200, 400, 800
    {300, 50, 0, 8, 7, 0},  {300, 50, 0, 8, 7, 0},  {300, 50, 0, 8, 7, 0},  {300, 50, 0, 8, 7, 0},
    // iso 1200, 1600, 2000, 2400
    {300, 80, 0, 8, 7, 0},  {300, 100, 0, 8, 7, 0}, {300, 100, 0, 8, 7, 0}, {300, 120, 0, 8, 7, 0},
    // iso 2800, 3200
    {300, 120, 0, 8, 7, 0}, {300, 140, 0, 8, 7, 0},

};

static hfg_param custom_params_hfg_main[eNUM_OF_SWNR_IDX]
{
    // iso 100, 200
    {1, 0, 55552, 56905, 46189, 16, 0, 64, 128, 192, 32, 28, 24, 20, -4, -4, -4, -4}, {1, 0, 55552, 56905, 46189, 16, 0, 64, 128, 192, 32, 28, 24, 20, -4, -4, -4, -4},
    // iso 400, 800
    {1, 0, 55552, 56905, 46189, 16, 0, 64, 128, 192, 32, 28, 24, 20, -4, -4, -4, -4}, {1, 0, 55552, 56905, 46189, 16, 0, 64, 128, 192, 32, 28, 24, 20, -4, -4, -4, -4},
    // iso 1200, 1600
    {1, 0, 55552, 56905, 46189, 16, 0, 64, 128, 192, 32, 28, 24, 20, -4, -4, -4, -4}, {1, 0, 55552, 56905, 46189, 16, 0, 64, 128, 192, 32, 28, 24, 20, -4, -4, -4, -4},
	// iso 2000, 2400
    {1, 0, 55552, 56905, 46189, 16, 0, 64, 128, 192, 32, 28, 24, 20, -4, -4, -4, -4}, {1, 0, 55552, 56905, 46189, 16, 0, 64, 128, 192, 32, 28, 24, 20, -4, -4, -4, -4},
    // iso 2800, 3200
    {1, 0, 55552, 56905, 46189, 16, 0, 64, 128, 192, 32, 28, 24, 20, -4, -4, -4, -4}, {1, 0, 55552, 56905, 46189, 16, 0, 64, 128, 192, 32, 28, 24, 20, -4, -4, -4, -4},
};

static hfg_param custom_params_hfg_sub[eNUM_OF_SWNR_IDX]
{
    // iso 100, 200
    {1, 0, 55552, 56905, 46189, 16, 0, 64, 128, 192, 32, 28, 24, 20, -4, -4, -4, -4}, {1, 0, 55552, 56905, 46189, 16, 0, 64, 128, 192, 32, 28, 24, 20, -4, -4, -4, -4},
    // iso 400, 800
    {1, 0, 55552, 56905, 46189, 16, 0, 64, 128, 192, 32, 28, 24, 20, -4, -4, -4, -4}, {1, 0, 55552, 56905, 46189, 16, 0, 64, 128, 192, 32, 28, 24, 20, -4, -4, -4, -4},
    // iso 1200, 1600
    {1, 0, 55552, 56905, 46189, 16, 0, 64, 128, 192, 32, 28, 24, 20, -4, -4, -4, -4}, {1, 0, 55552, 56905, 46189, 16, 0, 64, 128, 192, 32, 28, 24, 20, -4, -4, -4, -4},
	// iso 2000, 2400
    {1, 0, 55552, 56905, 46189, 16, 0, 64, 128, 192, 32, 28, 24, 20, -4, -4, -4, -4}, {1, 0, 55552, 56905, 46189, 16, 0, 64, 128, 192, 32, 28, 24, 20, -4, -4, -4, -4},
    // iso 2800, 3200
    {1, 0, 55552, 56905, 46189, 16, 0, 64, 128, 192, 32, 28, 24, 20, -4, -4, -4, -4}, {1, 0, 55552, 56905, 46189, 16, 0, 64, 128, 192, 32, 28, 24, 20, -4, -4, -4, -4},
};

static hfg_param custom_params_hfg_main_2[eNUM_OF_SWNR_IDX]
{
    // iso 100, 200
    {1, 0, 55552, 56905, 46189, 16, 0, 64, 128, 192, 32, 28, 24, 20, -4, -4, -4, -4}, {1, 0, 55552, 56905, 46189, 16, 0, 64, 128, 192, 32, 28, 24, 20, -4, -4, -4, -4},
    // iso 400, 800
    {1, 0, 55552, 56905, 46189, 16, 0, 64, 128, 192, 32, 28, 24, 20, -4, -4, -4, -4}, {1, 0, 55552, 56905, 46189, 16, 0, 64, 128, 192, 32, 28, 24, 20, -4, -4, -4, -4},
    // iso 1200, 1600
    {1, 0, 55552, 56905, 46189, 16, 0, 64, 128, 192, 32, 28, 24, 20, -4, -4, -4, -4}, {1, 0, 55552, 56905, 46189, 16, 0, 64, 128, 192, 32, 28, 24, 20, -4, -4, -4, -4},
	// iso 2000, 2400
    {1, 0, 55552, 56905, 46189, 16, 0, 64, 128, 192, 32, 28, 24, 20, -4, -4, -4, -4}, {1, 0, 55552, 56905, 46189, 16, 0, 64, 128, 192, 32, 28, 24, 20, -4, -4, -4, -4},
    // iso 2800, 3200
    {1, 0, 55552, 56905, 46189, 16, 0, 64, 128, 192, 32, 28, 24, 20, -4, -4, -4, -4}, {1, 0, 55552, 56905, 46189, 16, 0, 64, 128, 192, 32, 28, 24, 20, -4, -4, -4, -4},
};

static ccr_param custom_params_ccr_main[eNUM_OF_SWNR_IDX]
{
    // iso 100
    {0  , 0     , 0     , 39    , 103 , 14  , 21   , 5   , 35  , 255 ,
     64 , 0     , 68    , 0     , 224 , 0   , 56   , 11  , 25  , 1   ,
     0  , 0     , 0     , 16    , 270 , 286 , -128 , 127 , 64  , 0   },
    // iso 200
    {0  , 0     , 0     , 39    , 103 , 14  , 21   , 5   , 35  , 255 ,
     64 , 0     , 68    , 0     , 224 , 0   , 56   , 11  , 25  , 1   ,
     0  , 0     , 0     , 16    , 270 , 286 , -128 , 127 , 64  , 0   },
    // iso 400
    {0  , 0     , 0     , 39    , 103 , 14  , 21   , 5   , 35  , 255 ,
     64 , 0     , 68    , 0     , 224 , 0   , 56   , 11  , 25  , 1   ,
     0  , 0     , 0     , 16    , 270 , 286 , -128 , 127 , 64  , 0   },
    // iso 800
    {0  , 0     , 0     , 39    , 103 , 14  , 21   , 5   , 35  , 255 ,
     64 , 0     , 68    , 0     , 224 , 0   , 56   , 11  , 25  , 1   ,
     0  , 0     , 0     , 16    , 270 , 286 , -128 , 127 , 64  , 0   },
    // iso 1200
    {0  , 0     , 0     , 39    , 103 , 14  , 21   , 5   , 35  , 255 ,
     64 , 0     , 68    , 0     , 224 , 0   , 56   , 11  , 25  , 1   ,
     0  , 0     , 0     , 16    , 270 , 286 , -128 , 127 , 64  , 0   },
    // iso 1600
    {0  , 0     , 0     , 39    , 103 , 14  , 21   , 5   , 35  , 255 ,
     64 , 0     , 68    , 0     , 224 , 0   , 56   , 11  , 25  , 1   ,
     0  , 0     , 0     , 16    , 270 , 286 , -128 , 127 , 64  , 0   },
    // iso 2000
    {0  , 0     , 0     , 39    , 103 , 14  , 21   , 5   , 35  , 255 ,
     64 , 0     , 68    , 0     , 224 , 0   , 56   , 11  , 25  , 1   ,
     0  , 0     , 0     , 16    , 270 , 286 , -128 , 127 , 64  , 0   },
    // iso 2400
    {0  , 0     , 0     , 39    , 103 , 14  , 21   , 5   , 35  , 255 ,
     64 , 0     , 68    , 0     , 224 , 0   , 56   , 11  , 25  , 1   ,
     0  , 0     , 0     , 16    , 270 , 286 , -128 , 127 , 64  , 0   },
    // iso 2800
    {0  , 0     , 0     , 39    , 103 , 14  , 21   , 5   , 35  , 255 ,
     64 , 0     , 68    , 0     , 224 , 0   , 56   , 11  , 25  , 1   ,
     0  , 0     , 0     , 16    , 270 , 286 , -128 , 127 , 64  , 0   },
    // iso 3200
    {0  , 0     , 0     , 39    , 103 , 14  , 21   , 5   , 35  , 255 ,
     64 , 0     , 68    , 0     , 224 , 0   , 56   , 11  , 25  , 1   ,
     0  , 0     , 0     , 16    , 270 , 286 , -128 , 127 , 64  , 0   },
};

static ccr_param custom_params_ccr_sub[eNUM_OF_SWNR_IDX]
{
    // iso 100
    {0  , 0     , 0     , 39    , 103 , 14  , 21   , 5   , 35  , 255 ,
     64 , 0     , 68    , 0     , 224 , 0   , 56   , 11  , 25  , 1   ,
     0  , 0     , 0     , 16    , 270 , 286 , -128 , 127 , 64  , 0   },
    // iso 200
    {0  , 0     , 0     , 39    , 103 , 14  , 21   , 5   , 35  , 255 ,
     64 , 0     , 68    , 0     , 224 , 0   , 56   , 11  , 25  , 1   ,
     0  , 0     , 0     , 16    , 270 , 286 , -128 , 127 , 64  , 0   },
    // iso 400
    {0  , 0     , 0     , 39    , 103 , 14  , 21   , 5   , 35  , 255 ,
     64 , 0     , 68    , 0     , 224 , 0   , 56   , 11  , 25  , 1   ,
     0  , 0     , 0     , 16    , 270 , 286 , -128 , 127 , 64  , 0   },
    // iso 800
    {0  , 0     , 0     , 39    , 103 , 14  , 21   , 5   , 35  , 255 ,
     64 , 0     , 68    , 0     , 224 , 0   , 56   , 11  , 25  , 1   ,
     0  , 0     , 0     , 16    , 270 , 286 , -128 , 127 , 64  , 0   },
    // iso 1200
    {0  , 0     , 0     , 39    , 103 , 14  , 21   , 5   , 35  , 255 ,
     64 , 0     , 68    , 0     , 224 , 0   , 56   , 11  , 25  , 1   ,
     0  , 0     , 0     , 16    , 270 , 286 , -128 , 127 , 64  , 0   },
    // iso 1600
    {0  , 0     , 0     , 39    , 103 , 14  , 21   , 5   , 35  , 255 ,
     64 , 0     , 68    , 0     , 224 , 0   , 56   , 11  , 25  , 1   ,
     0  , 0     , 0     , 16    , 270 , 286 , -128 , 127 , 64  , 0   },
    // iso 2000
    {0  , 0     , 0     , 39    , 103 , 14  , 21   , 5   , 35  , 255 ,
     64 , 0     , 68    , 0     , 224 , 0   , 56   , 11  , 25  , 1   ,
     0  , 0     , 0     , 16    , 270 , 286 , -128 , 127 , 64  , 0   },
    // iso 2400
    {0  , 0     , 0     , 39    , 103 , 14  , 21   , 5   , 35  , 255 ,
     64 , 0     , 68    , 0     , 224 , 0   , 56   , 11  , 25  , 1   ,
     0  , 0     , 0     , 16    , 270 , 286 , -128 , 127 , 64  , 0   },
    // iso 2800
    {0  , 0     , 0     , 39    , 103 , 14  , 21   , 5   , 35  , 255 ,
     64 , 0     , 68    , 0     , 224 , 0   , 56   , 11  , 25  , 1   ,
     0  , 0     , 0     , 16    , 270 , 286 , -128 , 127 , 64  , 0   },
    // iso 3200
    {0  , 0     , 0     , 39    , 103 , 14  , 21   , 5   , 35  , 255 ,
     64 , 0     , 68    , 0     , 224 , 0   , 56   , 11  , 25  , 1   ,
     0  , 0     , 0     , 16    , 270 , 286 , -128 , 127 , 64  , 0   },
};

static ccr_param custom_params_ccr_main_2[eNUM_OF_SWNR_IDX]
{
    // iso 100
    {0  , 0     , 0     , 39    , 103 , 14  , 21   , 5   , 35  , 255 ,
     64 , 0     , 68    , 0     , 224 , 0   , 56   , 11  , 25  , 1   ,
     0  , 0     , 0     , 16    , 270 , 286 , -128 , 127 , 64  , 0   },
    // iso 200
    {0  , 0     , 0     , 39    , 103 , 14  , 21   , 5   , 35  , 255 ,
     64 , 0     , 68    , 0     , 224 , 0   , 56   , 11  , 25  , 1   ,
     0  , 0     , 0     , 16    , 270 , 286 , -128 , 127 , 64  , 0   },
    // iso 400
    {0  , 0     , 0     , 39    , 103 , 14  , 21   , 5   , 35  , 255 ,
     64 , 0     , 68    , 0     , 224 , 0   , 56   , 11  , 25  , 1   ,
     0  , 0     , 0     , 16    , 270 , 286 , -128 , 127 , 64  , 0   },
    // iso 800
    {0  , 0     , 0     , 39    , 103 , 14  , 21   , 5   , 35  , 255 ,
     64 , 0     , 68    , 0     , 224 , 0   , 56   , 11  , 25  , 1   ,
     0  , 0     , 0     , 16    , 270 , 286 , -128 , 127 , 64  , 0   },
    // iso 1200
    {0  , 0     , 0     , 39    , 103 , 14  , 21   , 5   , 35  , 255 ,
     64 , 0     , 68    , 0     , 224 , 0   , 56   , 11  , 25  , 1   ,
     0  , 0     , 0     , 16    , 270 , 286 , -128 , 127 , 64  , 0   },
    // iso 1600
    {0  , 0     , 0     , 39    , 103 , 14  , 21   , 5   , 35  , 255 ,
     64 , 0     , 68    , 0     , 224 , 0   , 56   , 11  , 25  , 1   ,
     0  , 0     , 0     , 16    , 270 , 286 , -128 , 127 , 64  , 0   },
    // iso 2000
    {0  , 0     , 0     , 39    , 103 , 14  , 21   , 5   , 35  , 255 ,
     64 , 0     , 68    , 0     , 224 , 0   , 56   , 11  , 25  , 1   ,
     0  , 0     , 0     , 16    , 270 , 286 , -128 , 127 , 64  , 0   },
    // iso 2400
    {0  , 0     , 0     , 39    , 103 , 14  , 21   , 5   , 35  , 255 ,
     64 , 0     , 68    , 0     , 224 , 0   , 56   , 11  , 25  , 1   ,
     0  , 0     , 0     , 16    , 270 , 286 , -128 , 127 , 64  , 0   },
    // iso 2800
    {0  , 0     , 0     , 39    , 103 , 14  , 21   , 5   , 35  , 255 ,
     64 , 0     , 68    , 0     , 224 , 0   , 56   , 11  , 25  , 1   ,
     0  , 0     , 0     , 16    , 270 , 286 , -128 , 127 , 64  , 0   },
    // iso 3200
    {0  , 0     , 0     , 39    , 103 , 14  , 21   , 5   , 35  , 255 ,
     64 , 0     , 68    , 0     , 224 , 0   , 56   , 11  , 25  , 1   ,
     0  , 0     , 0     , 16    , 270 , 286 , -128 , 127 , 64  , 0   },
};

bool get_capture_nr_th(
        MUINT32 const sensorDev,
        MUINT32 const shotmode,
        //MBOOL const isMfll,
        Capture_NR_Th_t* pTh
        )
{
    if( sensorDev == SENSOR_DEV_MAIN   ||
        sensorDev == SENSOR_DEV_SUB    ||
        sensorDev == SENSOR_DEV_MAIN_2
            )
    {
        switch(shotmode)
        {
            case eShotMode_NormalShot:
			    pTh->swth = 800;
                break;
            case eShotMode_ZsdShot:
                pTh->swth = 800;
                break;
            default:
                pTh->swth = DISABLE_CAPTURE_NR;
                break;
                // note: special case
                //  eShotMode_SmileShot, eShotMode_AsdShot
                //      --> NormalShot or ZsdShot
        }
    }
    else
    {
        pTh->swth = DISABLE_CAPTURE_NR;
    }

    return MTRUE;
}

bool get_tuning_param_NR(
        MUINT32 const sensorDev,
        MUINT32 const u4Idx,
        swnr_param_t& param
        )
{
    if( u4Idx < 0 || u4Idx >= eNUM_OF_SWNR_IDX )
        return false;
    //
    switch(sensorDev)
    {
        case SENSOR_DEV_MAIN:
            param = custom_params_nr_main[u4Idx];
            break;
        case SENSOR_DEV_SUB:
            param = custom_params_nr_sub[u4Idx];
            break;
        case SENSOR_DEV_MAIN_2:
            param = custom_params_nr_main_2[u4Idx];
            break;
        default:
            return false;
    }
    return true;
}

bool get_tuning_param_NR2(
        MUINT32 const sensorDev,
        MUINT32 const u4Idx,
        swnr2_param_t& param  
        )
{
    if( u4Idx < 0 || u4Idx >= eNUM_OF_SWNR_IDX )
        return false;
    //
    switch(sensorDev)
    {
        case SENSOR_DEV_MAIN:
            param = custom_params_nr2_main[u4Idx];
            break;
        case SENSOR_DEV_SUB:
            param = custom_params_nr2_sub[u4Idx];
            break;
        case SENSOR_DEV_MAIN_2:
            param = custom_params_nr2_main_2[u4Idx];
            break;
        default:
            return false;
    }
    return true;
}

bool get_tuning_param_HFG(
        MUINT32 const sensorDev,
        MUINT32 const u4Idx,
        hfg_param& param    
        )
{
    if( u4Idx < 0 || u4Idx >= eNUM_OF_SWNR_IDX )
        return false;
    //
    switch(sensorDev)
    {
        case SENSOR_DEV_MAIN:
			param = custom_params_hfg_main[u4Idx];
            break;
        case SENSOR_DEV_SUB:
			param = custom_params_hfg_sub[u4Idx];
            break;
        case SENSOR_DEV_MAIN_2:
			param = custom_params_hfg_main_2[u4Idx];
            break;
        default:
            return false;
    }
    return true;
}

bool get_tuning_param_CCR(
        MUINT32 const sensorDev,
        MUINT32 const u4Idx,
        ccr_param& param    
        )
{
    if( u4Idx < 0 || u4Idx >= eNUM_OF_SWNR_IDX )
        return false;
    //
    switch(sensorDev)
    {
        case SENSOR_DEV_MAIN:
			param = custom_params_ccr_main[u4Idx];
            break;
        case SENSOR_DEV_SUB:
			param = custom_params_ccr_sub[u4Idx];
            break;
        case SENSOR_DEV_MAIN_2:
			param = custom_params_ccr_main_2[u4Idx];
            break;
        default:
            return false;
    }
    return true;
}

static const MUINT32 tab_ISO_value_to_index[] =
{
    100, 200, 400, 800, 1200, 1600, 2000, 2400, 2800, 3200
};

// return value: performance 2 > 1 > 0, -1: default
MINT32 get_performance_level(
        MUINT32 const /*sensorDev*/,
        MUINT32 const shotmode
        )
{
    switch(shotmode)
    {
        case eShotMode_NormalShot:
            return -1;
            break;
        case eShotMode_ZsdShot:
            return -1;
            break;
        default:
            return -1;
            break;
            // note: special case
            //  eShotMode_SmileShot, eShotMode_AsdShot
            //      --> NormalShot or ZsdShot
    }
    return -1;
}

MUINT32
map_ISO_value_to_index(MUINT32 u4Iso)
    {
    MUINT32 i;
    for (i=0; i<sizeof(tab_ISO_value_to_index)/sizeof(tab_ISO_value_to_index[0]); i++) {
        if (u4Iso < tab_ISO_value_to_index[i]) {
            return i>0 ? (i-1) : i; //lower bound
        }
    }
    return sizeof(tab_ISO_value_to_index)/sizeof(tab_ISO_value_to_index[0])-1;
}


MUINT32
map_index_to_ISO_value(MUINT32 u4Idx)
{
    if (u4Idx >= sizeof(tab_ISO_value_to_index)/sizeof(tab_ISO_value_to_index[0]))
        return (MUINT32)-1;
    return tab_ISO_value_to_index[u4Idx];
}

MBOOL
is_to_invoke_swnr_interpolation(__attribute__((unused)) MUINT32 const u4Iso)
{
    return MTRUE;
}

//	"return value":	Interpolated register value
//	"ISO":			ISO value of current image
//	"ISOpre":		A pre-defined ISO (100, 200, 400, ...) which is the closest but smaller than or equal to "ISO"
//	"ISOpos":		A pre-defined ISO (100, 200, 400, ...) which is the closest but larger than or equal to "ISO"
//	"PARAMpre":		Corresponding register value from "ISOpre"
//	"PARAMpos":		Corresponding register value from "ISOpos"
//	"Method":		0 for linear interpolation, 1 for closest one
MINT32 InterParam_NR(MINT32 ISO, MINT32 ISOpre, MINT32 ISOpos, MINT32 PARAMpre, MINT32 PARAMpos, MINT32 Method){
	MINT32 InterPARAM = 0;

	if(ISOpre == ISOpos){
		//	To prevent someone sets different PARAMpre and PARAMpos
		return (MINT32)((PARAMpre + PARAMpos + 1) / 2);
	}

	switch(Method){
	case 1:
		InterPARAM = (abs((double)(ISO - ISOpre)) > abs((double)(ISOpos - ISO))) ? PARAMpos : PARAMpre;
		break;
	case 0:
	default:
		double RATIOpre = (double)(ISOpos - ISO) / (double)(ISOpos - ISOpre);
		double RATIOpos = 1.0 - (double)(RATIOpre);
		double temp =  RATIOpre * PARAMpre + RATIOpos * PARAMpos;
		if(temp>0)
			InterPARAM = (MINT32)(RATIOpre * PARAMpre + RATIOpos * PARAMpos + 0.5);
		else
			InterPARAM = (MINT32)(RATIOpre * PARAMpre + RATIOpos * PARAMpos - 0.5);
		break;
	}

	return InterPARAM;
}
bool smooth_param(
        MUINT32 const u4RealISO,
        MUINT32 const u4UpperISO,
        MUINT32 const u4LowerISO,
        swnr_param_t const& rUpperParam,
        swnr_param_t const& rLowerParam,
        swnr_param_t & rSmoothParam
        )
{
    if( u4RealISO > u4UpperISO || u4RealISO < u4LowerISO || u4UpperISO < u4LowerISO )
        return false;
    //
#define InterSWNR(item, method)            \
    do{                                    \
        rSmoothParam.item = InterParam_NR( \
                u4RealISO,                 \
                u4LowerISO,                \
                u4UpperISO,                \
                rLowerParam.item,          \
                rUpperParam.item,          \
                method);                   \
    } while(0)
    //
    InterSWNR(ANR_Y_LUMA_SCALE_RANGE , 0);
    InterSWNR(ANR_C_CHROMA_SCALE     , 0);
    //
    return true;
}

bool smooth_param_NR2(
        MUINT32 const u4RealISO,
        MUINT32 const u4UpperISO,
        MUINT32 const u4LowerISO,
        swnr2_param_t const& rUpperParam,
        swnr2_param_t const& rLowerParam,
        swnr2_param_t & rSmoothParam
        )
{
    if( u4RealISO > u4UpperISO || u4RealISO < u4LowerISO || u4UpperISO < u4LowerISO )
        return false;
    //
#define InterSWNR2(item, method)           \
    do{                                    \
        rSmoothParam.item = InterParam_NR( \
                u4RealISO,                 \
                u4LowerISO,                \
                u4UpperISO,                \
                rLowerParam.item,          \
                rUpperParam.item,          \
                method);                   \
    } while(0)
    //
    InterSWNR2(NR_K, 0);
    InterSWNR2(NR_S, 0);
    InterSWNR2(NR_SD, 0);	
    InterSWNR2(NR_BLD_W, 0);
    InterSWNR2(NR_BLD_TH, 0);
    InterSWNR2(NR_SMTH, 0);
    //
    return true;
}

bool refineByScale(
        MUINT32 const sensorDev,
        MUINT32 const scale_x100,
        swnr_param_t & rSmoothParam
        )
{
    return true;
}

bool refineByScaleNR2(
        MUINT32 const sensorDev,
        MUINT32 const scale_x100,
        swnr2_param_t & rSmoothParam
        )
{
    return true;
}

MINT32 get_swnr_type(MUINT32 const sensorDev)
{
    switch ( sensorDev )
    {
        case SENSOR_DEV_MAIN:
        case SENSOR_DEV_SUB:
        case SENSOR_DEV_MAIN_2:
        default:
            return eSWNRType_SW2;
            //return eSWNRType_SW;
    }
}
