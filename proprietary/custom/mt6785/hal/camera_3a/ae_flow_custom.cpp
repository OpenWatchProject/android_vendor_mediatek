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

/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#define LOG_TAG "ae_flow_custom"

#include <ae_flow_custom.h>
#include <cutils/log.h>

#define EV_TO_INDEX 11

MUINT8 uEVInfex[EV_TO_INDEX] = {100, 107, 115, 123, 132, 141, 152, 162, 174, 187, 200};

MBOOL calculateNewExposureSetting(AE_Cust_Param_T *a_AeCustParam, AE_EXP_SETTING_INPUT_T *a_strExpInput, AE_EXP_SETTING_OUTPUT_T *a_strExpOutput)
{ 
    a_strExpOutput->bEquivalent = MTRUE;
    a_strExpOutput->i4DeltaIndex = 0;
    a_strExpOutput->u4NewExposureTime_us= a_strExpInput->u4AlgoExposureTime_us;
    a_strExpOutput->u4NewSensorGain_x1024= a_strExpInput->u4AlgoSensorGain_x1024;
    a_strExpOutput->u4NewISPGain_x1024= a_strExpInput->u4AlgoISPGain_x1024;

    ALOGD("[%s] SensorID:0x%0X Shutter:%d->%d Sensor Gain:%d->%d ISP Gain:%d->%d\n",
        __FUNCTION__, a_strExpInput->i4SensorId, a_strExpInput->u4AlgoExposureTime_us, a_strExpOutput->u4NewExposureTime_us, a_strExpInput->u4AlgoSensorGain_x1024, a_strExpOutput->u4NewSensorGain_x1024, a_strExpInput->u4AlgoISPGain_x1024,    a_strExpOutput->u4NewISPGain_x1024);
    
    return MTRUE;
}

MINT32 GetDeltaEVIndex(MUINT32 u4ReduceRatio)
{
MINT32 i4Idx = 0;
MINT32 i4ReduceIndex = 0;

    while(u4ReduceRatio >= 200) {
        i4ReduceIndex = i4ReduceIndex + 10;
        u4ReduceRatio = u4ReduceRatio / 2;
    }
    for(i4Idx = 0; i4Idx < EV_TO_INDEX; i4Idx++) {
        if(uEVInfex[i4Idx] >= u4ReduceRatio) break;
    }
    return i4ReduceIndex + i4Idx;
}

MBOOL calculateExpSettingByShutterISOpriority(AE_EXP_MANUAL_INPUT_T *a_strExpInput, AE_EXP_SETTING_OUTPUT_T *a_strExpOutput)
{
MUINT32 u4MaxGainThreshold = 0;
MUINT32 u4MinGainThreshold = 0;
MUINT32 u4ReduceRatio = 0;

    a_strExpOutput->bEquivalent = MFALSE;
    a_strExpOutput->i4DeltaIndex = 0;

    if((a_strExpInput->u4TargetExposureTime != 0) && (a_strExpInput->u4TargetSensitivity == 0)) {              // ISO auto
        a_strExpOutput->u4NewExposureTime_us = a_strExpInput->u4TargetExposureTime;
        a_strExpOutput->u4NewSensorGain_x1024 = ((MINT64)a_strExpInput->u4OriExposureTime * a_strExpInput->u4OriSensorGain * a_strExpInput->u4OriISPGain) / 1024 / a_strExpInput->u4TargetExposureTime;
        u4MaxGainThreshold = ((MINT64)MAX_MANUAL_ISO_VALUE * 1024) / a_strExpInput->u41xGainISOvalue;
        u4MinGainThreshold = ((MINT64)MIN_MANUAL_ISO_VALUE * 1024) / a_strExpInput->u41xGainISOvalue;
        if(a_strExpOutput->u4NewSensorGain_x1024 > u4MaxGainThreshold) {
            a_strExpOutput->bEquivalent = MTRUE;
            u4ReduceRatio = 100 * a_strExpOutput->u4NewSensorGain_x1024 / u4MaxGainThreshold;

            a_strExpOutput->i4DeltaIndex = -1*GetDeltaEVIndex(u4ReduceRatio);
            ALOGD("[%s] New Gain over spec:%d->%d MaxISO:%d deltaIdx:%d ReduceRatio:%d\n", __FUNCTION__, a_strExpOutput->u4NewSensorGain_x1024, u4MaxGainThreshold, MAX_MANUAL_ISO_VALUE, a_strExpOutput->i4DeltaIndex, u4ReduceRatio);
            a_strExpOutput->u4NewSensorGain_x1024 = u4MaxGainThreshold;
        } else if(a_strExpOutput->u4NewSensorGain_x1024 < u4MinGainThreshold) {
            a_strExpOutput->bEquivalent = MTRUE;
            if(a_strExpOutput->u4NewSensorGain_x1024 != 0) {
                u4ReduceRatio = 100 * a_strExpInput->u4MinSensorGain / a_strExpOutput->u4NewSensorGain_x1024;
            }
            a_strExpOutput->i4DeltaIndex = GetDeltaEVIndex(u4ReduceRatio);
            ALOGD("[%s] New Gain under spec:%d->%d MinISO:%d deltaIdx:%d ReduceRatio:%d\n", __FUNCTION__, a_strExpOutput->u4NewSensorGain_x1024, MIN_MANUAL_ISO_VALUE, a_strExpOutput->i4DeltaIndex, u4ReduceRatio);
            a_strExpOutput->u4NewSensorGain_x1024 = u4MinGainThreshold;
        }

        a_strExpOutput->u4NewISPGain_x1024 = 1024;
        if(a_strExpOutput->u4NewSensorGain_x1024 < a_strExpInput->u4MinSensorGain) {
            ALOGD("[%s] Input Shutter:%d Sensor Gain:%d ISP Gain:%d, Target Shutter:%d Sensor Gain too Low:%d < %d\n",
                __FUNCTION__, a_strExpInput->u4OriExposureTime, a_strExpInput->u4OriSensorGain, a_strExpInput->u4OriISPGain, a_strExpInput->u4TargetExposureTime, a_strExpOutput->u4NewSensorGain_x1024, a_strExpInput->u4MinSensorGain);
            a_strExpOutput->u4NewSensorGain_x1024 = a_strExpInput->u4MinSensorGain;
        } else if(a_strExpOutput->u4NewSensorGain_x1024 > a_strExpInput->u4MaxSensorGain){
            ALOGD("[%s] Input Shutter:%d Sensor Gain:%d ISP Gain:%d, Target Shutter:%d Sensor Gain too high:%d < %d\n",
                __FUNCTION__, a_strExpInput->u4OriExposureTime, a_strExpInput->u4OriSensorGain, a_strExpInput->u4OriISPGain, a_strExpInput->u4TargetExposureTime, a_strExpOutput->u4NewSensorGain_x1024, a_strExpInput->u4MaxSensorGain);
            a_strExpOutput->u4NewISPGain_x1024 = 1024*a_strExpOutput->u4NewSensorGain_x1024/a_strExpInput->u4MaxSensorGain;
            a_strExpOutput->u4NewSensorGain_x1024 = a_strExpInput->u4MaxSensorGain;
        }
    } else if((a_strExpInput->u4TargetExposureTime == 0) && (a_strExpInput->u4TargetSensitivity != 0)) {     // Exp auto
        a_strExpOutput->u4NewSensorGain_x1024 = 1024* a_strExpInput->u4TargetSensitivity / a_strExpInput->u41xGainISOvalue;
        a_strExpOutput->u4NewISPGain_x1024 = 1024;
        if(a_strExpOutput->u4NewSensorGain_x1024 < a_strExpInput->u4MinSensorGain) {
            ALOGD("[%s] Input Shutter:%d Sensor Gain:%d ISP Gain:%d, Target ISO:%d Sensor Gain too Low:%d < %d\n",
                __FUNCTION__, a_strExpInput->u4OriExposureTime, a_strExpInput->u4OriSensorGain, a_strExpInput->u4OriISPGain, a_strExpInput->u4TargetSensitivity, a_strExpOutput->u4NewSensorGain_x1024, a_strExpInput->u4MinSensorGain);
            a_strExpOutput->u4NewSensorGain_x1024 = a_strExpInput->u4MinSensorGain;
        } else if(a_strExpOutput->u4NewSensorGain_x1024 > a_strExpInput->u4MaxSensorGain){
            ALOGD("[%s] Input Shutter:%d Sensor Gain:%d ISP Gain:%d, Target ISO:%d Sensor Gain too high:%d < %d\n",
                __FUNCTION__, a_strExpInput->u4OriExposureTime, a_strExpInput->u4OriSensorGain, a_strExpInput->u4OriISPGain, a_strExpInput->u4TargetSensitivity, a_strExpOutput->u4NewSensorGain_x1024, a_strExpInput->u4MaxSensorGain);
            a_strExpOutput->u4NewISPGain_x1024 = 1024*a_strExpOutput->u4NewSensorGain_x1024/a_strExpInput->u4MaxSensorGain;
            a_strExpOutput->u4NewSensorGain_x1024 = a_strExpInput->u4MaxSensorGain;
        }
        a_strExpOutput->u4NewExposureTime_us = ((MINT64)a_strExpInput->u4OriExposureTime * a_strExpInput->u4OriSensorGain * a_strExpInput->u4OriISPGain) / a_strExpOutput->u4NewSensorGain_x1024 / a_strExpOutput->u4NewISPGain_x1024;
        if(a_strExpOutput->u4NewExposureTime_us > MAX_MANUAL_EXP_VALUE) {
            a_strExpOutput->bEquivalent = MTRUE;
            u4ReduceRatio = 100 * a_strExpOutput->u4NewExposureTime_us / MAX_MANUAL_EXP_VALUE;

            a_strExpOutput->i4DeltaIndex = -1*GetDeltaEVIndex(u4ReduceRatio);
            ALOGD("[%s] New Shutter over spec:%d->%d New Sensor Gain:%d deltaIdx:%d ReduceRatio:%d\n", __FUNCTION__, a_strExpOutput->u4NewExposureTime_us, MAX_MANUAL_EXP_VALUE, a_strExpOutput->u4NewSensorGain_x1024, a_strExpOutput->i4DeltaIndex, u4ReduceRatio);
            a_strExpOutput->u4NewExposureTime_us = MAX_MANUAL_EXP_VALUE;
        } else if(a_strExpOutput->u4NewExposureTime_us < MIN_EXP_VALUE) {
            a_strExpOutput->bEquivalent = MTRUE;
            u4ReduceRatio = 100 * MIN_EXP_VALUE / a_strExpOutput->u4NewExposureTime_us;
            a_strExpOutput->i4DeltaIndex = GetDeltaEVIndex(u4ReduceRatio);
            ALOGD("[%s] New Shutter under spec:%d->%d New Sensor Gain:%d deltaIdx:%d ReduceRatio:%d\n", __FUNCTION__, a_strExpOutput->u4NewExposureTime_us, MIN_EXP_VALUE, a_strExpOutput->u4NewSensorGain_x1024, a_strExpOutput->i4DeltaIndex, u4ReduceRatio);
            a_strExpOutput->u4NewExposureTime_us = MIN_EXP_VALUE;
        }
    } else {
        a_strExpOutput->u4NewExposureTime_us = a_strExpInput->u4OriExposureTime;
        a_strExpOutput->u4NewSensorGain_x1024 = a_strExpInput->u4OriSensorGain;
        a_strExpOutput->u4NewISPGain_x1024 = a_strExpInput->u4OriISPGain;
    }

    ALOGD("[%s] Input Shutter:%d Sensor Gain:%d ISP Gain:%d, Target Shutter:%d ISO:%d Output Shutter:%d Output Sensor Gain:%d ISP Gain: %d\n",
        __FUNCTION__, a_strExpInput->u4OriExposureTime, a_strExpInput->u4OriSensorGain, a_strExpInput->u4OriISPGain, a_strExpInput->u4TargetExposureTime, a_strExpInput->u4TargetSensitivity, a_strExpOutput->u4NewExposureTime_us, a_strExpOutput->u4NewSensorGain_x1024, a_strExpOutput->u4NewISPGain_x1024);

    return MTRUE;
}

