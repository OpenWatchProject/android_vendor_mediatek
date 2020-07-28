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
#ifndef _AESYNC_TUNING_PARA_H
#define _AESYNC_TUNING_PARA_H

#include "camera_custom_nvram.h"

const AESYNC_NVRAM_T g_rAESyncTuningParam = {
    MTRUE,  // bGainRegression
    0,  // u2RegressionType, 0: cwv; 1: avgY
    3,  // u2AlignMode
    {0, 0, 0, 0, 0}, // i4EvOffset
    {10000, 10000, 10000},  //u4RGB2YCoef
    1057, //u4FixSyncGain
    {   //AESYNC_DENOISE_BMDN
        {
        1000,  // EV   0
        1072,   //EV    0.1
        1149,   //EV    0.2
        1231,   //EV    0.3
        1320,   //EV    0.4
        1414,   //EV    0.5
        1516,   //EV    0.6
        1625,   //EV    0.7
        1625,   //EV    0.8
        1625,   //EV    0.9
        1625,   //EV    1
        1625,   //EV    1.1
        1625,   //EV    1.2
        1625,   //EV    1.3
        1625,   //EV    1.4
        1625,   //EV    1.5
        1625,   //EV    1.6
        1625,   //EV    1.7
        1625,   //EV    1.8
        1625,   //EV    1.9
        1625,   //EV    2
        1625,   //EV    2.1
        1625,   //EV    2.2
        1625,   //EV    2.3
        1625,   //EV    2.4
        1625,   //EV    2.5
        1625,   //EV    2.6
        1625,   //EV    2.7
        1625,   //EV    2.8
        1625,   //EV    2.9   
        },
        //AESYNC_DENOISE_MFNR
        {
        1000,  // EV   0
        1072,   //EV    0.1
        1149,   //EV    0.2
        1231,   //EV    0.3
        1320,   //EV    0.4
        1414,   //EV    0.5
        1516,   //EV    0.6
        1625,   //EV    0.7
        1625,   //EV    0.8
        1625,   //EV    0.9
        1625,   //EV    1
        1625,   //EV    1.1
        1625,   //EV    1.2
        1625,   //EV    1.3
        1625,   //EV    1.4
        1625,   //EV    1.5
        1625,   //EV    1.6
        1625,   //EV    1.7
        1625,   //EV    1.8
        1625,   //EV    1.9
        1625,   //EV    2
        1625,   //EV    2.1
        1625,   //EV    2.2
        1625,   //EV    2.3
        1625,   //EV    2.4
        1625,   //EV    2.5
        1625,   //EV    2.6
        1625,   //EV    2.7
        1625,   //EV    2.8
        1625,   //EV    2.9   
        },
    },
    {0},    //pReserved
};

#endif