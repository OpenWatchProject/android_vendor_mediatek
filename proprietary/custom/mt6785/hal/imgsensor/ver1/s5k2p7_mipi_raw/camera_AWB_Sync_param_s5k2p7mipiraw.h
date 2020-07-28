/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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
#ifndef _AWBSYNC_TUNING_PARA_H
#define _AWBSYNC_TUNING_PARA_H

#include "camera_custom_nvram.h"

const AWBSYNC_NVRAM_T g_rAWBSyncTuningParam = {
    SYNC_AWB_ADV_PP_METHOD,
    {
        MTRUE,
        10,
        230,
        5,
        10, //10%
        20, //diff should less than 20%
        96, //Enhance Sub gain change base = 512, range = 0~512

    },

    {8, 10, 5, 80},
    {
        {80,120},  //100 = 1x
        {80,120},

        {80,120},
        {80,120},

        10,

        {1,5},
        {5,250},
    },
    MTRUE,
    32,  //zoom ratio 32 = 1x
    {
        MTRUE,
        {1,2,4},
    },
    {
        {512, 512, 512}, //Tungsten WB sync preference, base = 512
        {512, 512, 512}, //WARM_FLUORESCENT sync preference, base = 512
        {512, 512, 512}, //FLUORESCENT WB sync preference, base = 512
        {512, 512, 512}, //CWF WB sync preference, base = 512
        {512, 512, 512}, //DAYLIGHT WB sync preference, base = 512
        {512, 512, 512}, //Shade WB sync preference, base = 512
        {512, 512, 512}, //DAYLIGHT FLUORESCENT WB sync preference, base = 512
    },
    {
        {50,100},
        {512, 512}, //Tungsten WB sync preference channel R, base = 512
        {512, 512}, //Tungsten WB sync preference channel G, base = 512
        {512, 512}, //Tungsten WB sync preference channel B, base = 512
        {512, 512}, //WARM_FLUORESCENT sync preference channel R, base = 512
        {512, 512}, //WARM_FLUORESCENT sync preference channel G, base = 512
        {512, 512}, //WARM_FLUORESCENT sync preference channel B, base = 512
        {512, 512}, //FLUORESCENT WB sync preference channel R, base = 512
        {512, 512}, //FLUORESCENT WB sync preference channel G, base = 512
        {512, 512}, //FLUORESCENT WB sync preference channel B, base = 512
        {512, 512}, //CWF WB sync preference channel R, base = 512
        {512, 512}, //CWF WB sync preference channel G, base = 512
        {512, 512}, //CWF WB sync preference channel B, base = 512
        {512, 512}, //DAYLIGHT WB sync preference channel R, base = 512
        {512, 512}, //DAYLIGHT WB sync preference channel G, base = 512
        {512, 512}, //DAYLIGHT WB sync preference channel B, base = 512
        {512, 512}, //Shade WB sync preference channel R, base = 512
        {512, 512}, //Shade WB sync preference channel G, base = 512
        {512, 512}, //Shade WB sync preference channel B, base = 512
        {512, 512}, //DAYLIGHT FLUORESCENT WB sync preference channel R, base = 512
        {512, 512}, //DAYLIGHT FLUORESCENT WB sync preference channel G, base = 512
        {512, 512}, //DAYLIGHT FLUORESCENT WB sync preference channel B, base = 512
    },
    {
        //LV   0    1    2    3    4    5     6    7    8    9   10   11   12   13   14   15   16   17   18
        {  100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100},
    },    
    {
        STANDBY_SYNC_AWB_Geometric_Smooth,
        {
            80,	// Geometric Smooth Radius
            3,	// Temporary Smooth Speed
            1,	// Smooth Convergence Threshold
        },
    },
    // CCT directly mapping tuning param
    {
        {380, -450}, // extend light  to support >D65

        {
            //Y mapping upper reference point
            {-473, -328}, // L
            { -81, -418}, // M
            { 280, -340}, // H

        },

        {
            //Y mapping lower reference point
            {-313, -637}, // L
            { -86, -637}, // M
            { 280, -630}, // H
        },
 
        {
        //X    Y  offset
        { 0,   0}, // HORIZON
        { 0,   0}, // A
        { 0,   0}, // TL84
        { 0,   0}, // DNP
        { 0,   0}, // D65
        { 0,   0}, // Extd-light
        },
        {
            //-100  -80  -60  -40  -20    0   20   40   60    80  100
            {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, // HORIZON   
            {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, // A            
            {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, // TL84
            {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, // DNP
            {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, // D65
            {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, // SHADE
        },
    },
    // Reserved
    {
     0,
    },
};

#endif