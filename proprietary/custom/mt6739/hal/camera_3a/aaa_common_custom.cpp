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

#define LOG_TAG "aaa_common_custom.cpp"
#include <cutils/log.h>

#include "camera_custom_types.h"
#include "aaa_common_custom.h"
#include <vector>
#include "aaa_log.h"

/********************************************************************************************
 * DO NOT CHANGE!!!DO NOT CHANGE!!!DO NOT CHANGE!!!DO NOT CHANGE!!!DO NOT CHANGE!!!
 *******************************************************************************************/
#define ON       MTRUE
#define OFF      MFALSE




/********************************************************************************************
 ********************************************************************************************
 ****************************** Customize 3A options below **********************************
 ********************************************************************************************
 *******************************************************************************************/


/********************************************************************************************
 * ENABLE_PRECAPTURE_AF:
 * [ON]: During precatpure, AF is executed when environment is dark.
 * [OFF]: No precapture AF
 *******************************************************************************************/
#define ENABLE_PRECAPTURE_AF                ON



/********************************************************************************************
 * PRECAPTURE_AF_AFTER_PREFLASH:
 * [ON] Precapture AF is after preflash
 * [OFF] Precapture AF is before preflash
 *******************************************************************************************/
#define PRECAPTURE_AF_AFTER_PREFLASH        ON



/********************************************************************************************
 * ENABLE_VIDEO_AUTO_FLASH:
 * [ON]: Enable video auto flash
 * (1. When user selects auto mode,  flash on/off is determined by BV.
 *  2. when user selects off mode, flash off.
 *  3. when user selects on mode, flash on. )
 * [OFF]: Disable video auto flash
 *******************************************************************************************/
#define ENABLE_VIDEO_AUTO_FLASH             ON



/********************************************************************************************
 * CAF_WINDOW_FOLLOW_TAF_WINDOW:
 * [ON]: CAF and Monitor window follow touch AF window.
 * [OFF]: CAF and monitor window is set to center
 *******************************************************************************************/
#define CAF_WINDOW_FOLLOW_TAF_WINDOW        OFF



/********************************************************************************************
 * ONE_SHOT_AE_BEFORE_TAF:
 * [ON] : Do one-shot AE before touch AF
 * [OFF] : Do one-shot AE after touch AF
 *******************************************************************************************/
#define ONE_SHOT_AE_BEFORE_TAF              ON



/********************************************************************************************
 * SKIP_ONE_SHOT_AE_FOR_TAF:
 * [ON] : Skip one-shot AE for touch AF if overall exposure is acceptable
 * [OFF] : Don't skip one-shot AE for  touch AF
 *******************************************************************************************/
#define SKIP_ONE_SHOT_AE_FOR_TAF            ON



/********************************************************************************************
 * ENABLE_TOUCH_AE:
 * [ON] : Enable the touch AE
 * [OFF] : Disable the touch AE.
 *******************************************************************************************/
#define ENABLE_TOUCH_AE                     ON



/********************************************************************************************
 * ENABLE_FACE_AE:
 * [ON] : Enable the touch AE
 * [OFF] : Disable the touch AE.
 *******************************************************************************************/
#define ENABLE_FACE_AE                      ON



/********************************************************************************************
 * LOCK_AE_DURING_CAF:
 * [ON]: Lock AE when doing CAF(conti' AF)
 * [OFF]: AE can work when doing CAF
 *******************************************************************************************/
#define LOCK_AE_DURING_CAF                  ON

/********************************************************************************************
 * ENABLE_VIDEO_DYNAMIC_FRAME_RATE:
 * [ON] : Enable the video dynamic frame rate
 * [OFF] : Disable the video dynamic frame rate.
 *******************************************************************************************/
#define ENABLE_VIDEO_DYNAMIC_FRAME_RATE ON

/********************************************************************************************
 * ENABLE_FLASH_DURING_TOUCH:
 * [ON] : Enable FlashTask during touch.
 * [OFF] : Disable FlashTask during touch.
 *******************************************************************************************/
#define ENABLE_FLASH_DURING_TOUCH ON

/********************************************************************************************
 * SYNC3A_AESTABLE_MAGIC:
 *******************************************************************************************/
#define SYNC3A_AESTABLE_MAGIC 5
#define SKIP_PRECAP_FLASH_FRAME_COUNT 12

/********************************************************************************************
 * SENSOR_S5K3L6_LONG_EXP_THRES:
 *******************************************************************************************/
#define SENSOR_LONG_EXP_THRES 668437 // S5K3L6 THRES

/********************************************************************************************
 * IS_SPECIAL_LONG_EXP_ON:
 * g_bIsSpecialLongExpOn:
 * [ON] : Enable Special LongExp flow.
 * [OFF] : Disable Special LongExp flow.
 * g_i4IsSpecialLongExpEffectiveFrame:
 * [0] : sensor not implement
 * [1] : N+1 Effective frame
 * [2] : N+2 Effective frame
 *******************************************************************************************/
static MBOOL g_bIsSpecialLongExpOn = MFALSE;
static MINT32 g_i4IsSpecialLongExpEffectiveFrame = 0;

MBOOL CUST_ENABLE_PRECAPTURE_AF(void)
{
    return ENABLE_PRECAPTURE_AF;
}
MBOOL CUST_PRECAPTURE_AF_AFTER_PREFLASH(void)
{
    return PRECAPTURE_AF_AFTER_PREFLASH;
}
MBOOL CUST_ENABLE_VIDEO_AUTO_FLASH(void)
{
    return ENABLE_VIDEO_AUTO_FLASH;
}
MBOOL CUST_CAF_WINDOW_FOLLOW_TAF_WINDOW(void)
{
    return CAF_WINDOW_FOLLOW_TAF_WINDOW;
}
MBOOL CUST_ONE_SHOT_AE_BEFORE_TAF(void)
{
    return ONE_SHOT_AE_BEFORE_TAF;
}
MBOOL CUST_SKIP_ONE_SHOT_AE_FOR_TAF(void)
{
    return SKIP_ONE_SHOT_AE_FOR_TAF;
}
MBOOL CUST_ENABLE_TOUCH_AE(void)
{
    return ENABLE_TOUCH_AE;
}
MBOOL CUST_ENABLE_FACE_AE(void)
{
    return ENABLE_FACE_AE;
}
MBOOL CUST_LOCK_AE_DURING_CAF(void)
{
    return LOCK_AE_DURING_CAF;
}

MBOOL CUST_ENABLE_VIDEO_DYNAMIC_FRAME_RATE(void)
{
    return ENABLE_VIDEO_DYNAMIC_FRAME_RATE;
}

MBOOL CUST_ENABLE_FLASH_DURING_TOUCH(void)
{
    return ENABLE_FLASH_DURING_TOUCH;
}

MUINT32 CUST_GET_SYNC3A_AESTABLE_MAGIC(void)
{
    return SYNC3A_AESTABLE_MAGIC;
}

MINT32 CUST_GET_SKIP_PRECAP_FLASH_FRAME_COUNT(void)
{
    return SKIP_PRECAP_FLASH_FRAME_COUNT;
}
unsigned int Scenario4AE(const ScenarioParam& sParam)
{
#if CAM3_STEREO_FEATURE_EN
    if (sParam.Sync2AMode == 2 ||
        sParam.Sync2AMode == 3 ||
        sParam.Sync2AMode == 4)
        return CAM_SCENARIO_CUSTOM4;
#endif
    if(sParam.HdrMode == 1 && sParam.TargetMode != AE_MODE_NORMAL)
        return CAM_SCENARIO_CUSTOM1;
    else if(sParam.HdrMode == 2 && sParam.TargetMode != AE_MODE_NORMAL)
        return CAM_SCENARIO_CUSTOM2;
    else{
        switch(sParam.SensorMode){
            case 0:
                return CAM_SCENARIO_PREVIEW;
            case 1:
                return CAM_SCENARIO_CAPTURE;
            case 2:
                return CAM_SCENARIO_VIDEO;
            default:
                return CAM_SCENARIO_PREVIEW;
        }
    }
}

unsigned int Scenario4AWB(const ScenarioParam& sParam)
{
#if CAM3_STEREO_FEATURE_EN
    if (sParam.Sync2AMode == 2 ||
        sParam.Sync2AMode == 3 ||
        sParam.Sync2AMode == 4)
        return CAM_SCENARIO_CUSTOM4;
#endif
    if(sParam.HdrMode == 1 && sParam.TargetMode != AE_MODE_NORMAL)
        return CAM_SCENARIO_CUSTOM1;
    else if(sParam.HdrMode == 2 && sParam.TargetMode != AE_MODE_NORMAL)
        return CAM_SCENARIO_CUSTOM2;
    else{
        switch(sParam.SensorMode){
            case 0:
                return CAM_SCENARIO_PREVIEW;
            case 1:
                return CAM_SCENARIO_CAPTURE;
            case 2:
                return CAM_SCENARIO_VIDEO;
            default:
                return CAM_SCENARIO_PREVIEW;
        }
    }
}

unsigned int Scenario4AF(const ScenarioParam& sParam)
{
#if CAM3_STEREO_FEATURE_EN
    if (sParam.Sync2AMode == 2)
        return CAM_SCENARIO_CUSTOM4;
#endif
    switch(sParam.SensorMode){
        case 0:
        {
            if (sParam.CaptureIntent == 3)
                return CAM_SCENARIO_CUSTOM3;
            else
                return CAM_SCENARIO_PREVIEW;
        }
        case 1:
            return CAM_SCENARIO_CAPTURE;
        case 2:
            return CAM_SCENARIO_VIDEO;
        default:
            return CAM_SCENARIO_PREVIEW;
    }
}

std::vector<int> getShortExpFrame(void)
{
    std::vector<int> ExpFrame = {1, 1, 1, -1}; // 0: Skipped frame, 1: Short Exposure frame, the fourth is to prevent segmentation fault

    return ExpFrame;
}

void cust_initSpecialLongExpOnOff(const MINT32 &i4AEEffectiveFrame)
{
    g_bIsSpecialLongExpOn = 0;
    g_i4IsSpecialLongExpEffectiveFrame = i4AEEffectiveFrame;
    ALOGI("[%s] g_bIsSpecialLongExpOn(%d) AEEffectiveFrame(%d)", __FUNCTION__, g_bIsSpecialLongExpOn, g_i4IsSpecialLongExpEffectiveFrame);
}

void cust_setSpecialLongExpOnOff(const MINT64 &i8ExposureTime)
{
    MINT64 i8LongExpThres = 0;
    switch(g_i4IsSpecialLongExpEffectiveFrame){
        case 1:
            i8LongExpThres = static_cast<MINT64>(SENSOR_LONG_EXP_THRES);
            break;
        case 2:
        case 0:
        default:
            return;
    }
    MBOOL bIsLongExpOn = (i8ExposureTime > i8LongExpThres) ? 1: 0;
    AAA_LOGI_IF(g_bIsSpecialLongExpOn != bIsLongExpOn, "[%s] AEEffectiveFrame(%d), i8ExposureTime(%ld), i8LongExpThres(%ld), Long Exposure OnOff(%d)", __FUNCTION__, g_i4IsSpecialLongExpEffectiveFrame, i8ExposureTime, i8LongExpThres, g_bIsSpecialLongExpOn);
    if(g_bIsSpecialLongExpOn != bIsLongExpOn)
    {
        g_bIsSpecialLongExpOn = bIsLongExpOn;
    }
}

MBOOL cust_getIsSpecialLongExpOn()
{
    return g_bIsSpecialLongExpOn;
}