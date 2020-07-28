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
#include "ov13855main2mipiraw_Scene_Capture.h"

const FLASH_AE_NVRAM_T ov13855main2mipiraw_Flash_AE_0000 = {
    .tuningPara = {
        188,   // yTarget
        10,    // fgWIncreaseLevelbySize
        5,     // fgWIncreaseLevelbyRef
        5,     // ambientRefAccuracyRatio
        0,     // flashRefAccuracyRatio
        18,    // backlightAccuracyRatio
        40,    // backlightUnderY
        32,    // backlightWeakRefRatio
        66644, // safetyExp
        1200,  // maxUsableISO
        0,     // yTargetWeight
        13,    // lowReflectanceThreshold
        0,     // flashReflectanceWeight
        20,    // bgSuppressMaxDecreaseEV
        6,     // bgSuppressMaxOverExpRatio
        50,    // fgEnhanceMaxIncreaseEV
        2,     // fgEnhanceMaxOverExpRatio
        1,     // isFollowCapPline
        300,   // histStretchMaxFgYTarget
        480,   // histStretchBrightestYTarget
        0,     // fgSizeShiftRatio
        90,    // backlitPreflashTriggerLV
        90,    // backlitMinYTarget
        80,    // minstameanpass
       188,    // yDecreEVTarget
       188,    // yFaceTarget
         5,    // cfgFlashPolicy
    },
    .dualTuningPara = {
        30,                // toleranceEV_pos
        30,                // toleranceEV_neg
        64,                // XYWeighting
        1,                 // useAwbPreferenceGain
        // envOffsetIndex[4]
        {
            -200,
            -100,
            50,
            150,
        },
        // envXrOffsetValue[4]
        {
            0,
            0,
            0,
            0,
        },
        // envYrOffsetValue[4]
        {
            0,
            0,
            0,
            0,
        },
        1,                 // VarianceTolerance
        FLASH_CHOOSE_COLD, // ChooseColdOrWarm
    },
    .engLevel = {
        6,    // torchDuty
        {},   // torchDutyEx[20]
        6,    // afDuty
        6,    // pfDuty
        25,   // mfDutyMax
        -1,   // mfDutyMin
        0,    // IChangeByVBatEn
        3600, // vBatL
        6,    // pfDutyL
        6,    // mfDutyMaxL
        -1,   // mfDutyMinL
        1,    // IChangeByBurstEn
        6,    // pfDutyB
        6,    // mfDutyMaxB
        -1,   // mfDutyMinB
        1,    // decSysIAtHighEn
        20,   // dutyH
    },
    .engLevelLT = {
        0,    // torchDuty
        {},   // torchDutyEx[20]
        0,    // afDuty
        0,    // pfDuty
        25,   // mfDutyMax
        -1,   // mfDutyMin
        0,    // pfDutyL
        6,    // mfDutyMaxL
        -1,   // mfDutyMinL
        0,    // pfDutyB
        6,    // mfDutyMaxB
        -1,   // mfDutyMinB
    },
};
