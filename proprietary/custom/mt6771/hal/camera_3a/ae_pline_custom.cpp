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

#include "ae_pline_custom.h"
#include <string.h>
#include <utils/Errors.h>
#include <cutils/log.h>
#include <fcntl.h>
#include <cutils/properties.h>
#include <stdlib.h>


#define LOG_TAG "ae_pline_custom"
#define CustomModifyFourCell
#define CustomModifyGetExpo 1

void getAEManualPline(const ManualPlineParam& manualPlineParam, eAETableID& a_AEManualPreTableID, eAETableID& a_AEManualCapTableID)
{
    if ((manualPlineParam.MinFps == LIB3A_AE_FRAMERATE_MODE_30FPS) && (manualPlineParam.MaxFps == LIB3A_AE_FRAMERATE_MODE_30FPS)){ // fix 15 fps
        a_AEManualPreTableID = AETABLE_SCENE_INDEX22;
        a_AEManualCapTableID = AETABLE_SCENE_INDEX22;
    } else if ((manualPlineParam.MinFps == LIB3A_AE_FRAMERATE_MODE_15FPS) && (manualPlineParam.MaxFps == LIB3A_AE_FRAMERATE_MODE_15FPS)){ // fix 30 fps
        a_AEManualPreTableID = AETABLE_SCENE_INDEX14;
        a_AEManualCapTableID = AETABLE_SCENE_INDEX14;
    } else if ((manualPlineParam.MinFps == 240) && (manualPlineParam.MaxFps == 240)){ // fix 24 fps
        a_AEManualPreTableID = AETABLE_SCENE_INDEX4;
        a_AEManualCapTableID = AETABLE_SCENE_INDEX4;
    } else if (manualPlineParam.SensorMode == 2){ // sensor mode: video
        a_AEManualPreTableID = AETABLE_VIDEO1_AUTO;
        a_AEManualCapTableID = AETABLE_VIDEO1_AUTO;
    } else if (manualPlineParam.SceneMode == LIB3A_AE_SCENE_SNOW){ // scene mode: snow
        a_AEManualPreTableID = AETABLE_VIDEO_NIGHT;
        a_AEManualCapTableID = AETABLE_VIDEO_NIGHT;
    } else if (manualPlineParam.ISOSpeed == 150){ // ISO speed: 150
        a_AEManualPreTableID = AETABLE_CAPTURE_ISO100;
        a_AEManualCapTableID = AETABLE_CAPTURE_ISO100;
    }
    else {
        ALOGD("[%s] no manual pline matching, MinFps/MaxFps/SensorMode/SceneMode/ISOSpeed: %d/%d/%d/%d/%d\n", __FUNCTION__,
              manualPlineParam.MinFps, manualPlineParam.MaxFps, manualPlineParam.SensorMode, manualPlineParam.SceneMode, manualPlineParam.ISOSpeed);
        return;
    }
    ALOGD("[%s] a_AEManualPreTableID/a_AEManualCapTableID:%d/%d, MinFps/MaxFps/SensorMode/SceneMode/ISOSpeed: %d/%d/%d/%d/%d\n", __FUNCTION__, a_AEManualPreTableID, a_AEManualCapTableID,
          manualPlineParam.MinFps, manualPlineParam.MaxFps, manualPlineParam.SensorMode, manualPlineParam.SceneMode, manualPlineParam.ISOSpeed);
}

void getAEManualPline(const EAEManualPline_T& e_AEManualPline, eAETableID& a_AEManualPreTableID, eAETableID& a_AEManualCapTableID)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    switch(e_AEManualPline) {
        case EAEManualPline_EISRecord:
            a_AEManualPreTableID = AETABLE_SCENE_INDEX9;
            a_AEManualCapTableID = AETABLE_SCENE_INDEX9;
            break;
        case EAEManualPline_SM240FPS:
            a_AEManualPreTableID = AETABLE_SCENE_INDEX10;
            a_AEManualCapTableID = AETABLE_SCENE_INDEX10;
            break;
        case EAEManualPline_AIS1Capture:
        case EAEManualPline_AIS2Capture:
            a_AEManualPreTableID = AETABLE_SCENE_INDEX14;
            a_AEManualCapTableID = AETABLE_SCENE_INDEX14;
            break;
        case EAEManualPline_MFHRCapture:
            a_AEManualPreTableID = AETABLE_SCENE_INDEX19;
            a_AEManualCapTableID = AETABLE_SCENE_INDEX19;
            break;
        case EAEManualPline_BMDNCapture:
            a_AEManualPreTableID = AETABLE_SCENE_INDEX20;
            a_AEManualCapTableID = AETABLE_SCENE_INDEX20;
            break;
        case EAEManualPline_ADBCtrol:
            property_get("vendor.debug.ae_pline.preview", value, "0");
            a_AEManualPreTableID = static_cast<eAETableID>(atoi(value));
            property_get("vendor.debug.ae_pline.capture", value, "0");
            a_AEManualCapTableID = static_cast<eAETableID>(atoi(value));
            ALOGD( "[%s()] ADB assigns P-lineID manually: %d/%d \n", __FUNCTION__, a_AEManualPreTableID, a_AEManualCapTableID);
            return;
        case EAEManualPline_Default:
        default:
            ALOGD("[%s] The EAEManualPline_T enum value is default or incorrect:%d\n", __FUNCTION__, e_AEManualPline);
            return;
    }
    ALOGD("[%s] e_AEManualPline:%d, a_AEManualPreTableID/a_AEManualCapTableID:%d/%d\n", __FUNCTION__, e_AEManualPline, a_AEManualPreTableID, a_AEManualCapTableID);
}

void getAEManualCapPline(const EAEManualPline_T& e_AEManualPline, eAETableID& a_AEManualTableID)
{
    switch(e_AEManualPline) {
        case EAEManualPline_AIS1Capture:
        case EAEManualPline_AIS2Capture:
            a_AEManualTableID = AETABLE_SCENE_INDEX14;
            break;
        case EAEManualPline_MFHRCapture:
            a_AEManualTableID = AETABLE_SCENE_INDEX19;
            break;
        case EAEManualPline_BMDNCapture:
            a_AEManualTableID = AETABLE_SCENE_INDEX20;
            break;
        case EAEManualPline_Default:
        default:
            ALOGD("[%s] The EAEManualPline_T enum value is default or incorrect:%d\n", __FUNCTION__, e_AEManualPline);
            return;
    }
    ALOGD("[%s] e_AEManualPline:%d, a_AEManualTableID:%d\n", __FUNCTION__, e_AEManualPline, a_AEManualTableID);
}

void ModifyCaptureParamByCustom(MUINT32 binning_ratio,AE_MODE_CFG_T *sAEInputInfo,strAEOutput *sAEOutputInfo,MUINT32 maxAfeGain)
{
 #ifdef CustomModifyFourCell
 ALOGD("[%s] u4Eposuretime:%d, u4AfeGain:%d u4IspGain:%d u4RealISO:%d\n", __FUNCTION__, sAEInputInfo->u4Eposuretime, sAEInputInfo->u4AfeGain,sAEInputInfo->u4IspGain,sAEInputInfo->u4RealISO);
 if(sAEInputInfo->u4Eposuretime < 200000){
    sAEOutputInfo->EvSetting.u4Eposuretime = sAEInputInfo->u4Eposuretime * binning_ratio;
    sAEOutputInfo->EvSetting.u4AfeGain = sAEInputInfo->u4AfeGain;
    sAEOutputInfo->EvSetting.u4IspGain = sAEInputInfo->u4IspGain;
    sAEOutputInfo->u4ISO = sAEInputInfo->u4RealISO;
 }else {
    MUINT32 u4AfeGain = sAEInputInfo->u4AfeGain;
    MUINT32 u4ISPGain = sAEInputInfo->u4IspGain;

    sAEOutputInfo->EvSetting.u4Eposuretime = sAEInputInfo->u4Eposuretime;
    if(u4AfeGain*binning_ratio <= maxAfeGain){
      sAEOutputInfo->EvSetting.u4AfeGain = u4AfeGain*binning_ratio;
      sAEOutputInfo->EvSetting.u4IspGain = u4ISPGain;
    } else {
      sAEOutputInfo->EvSetting.u4AfeGain = maxAfeGain;
      sAEOutputInfo->EvSetting.u4IspGain = u4AfeGain*binning_ratio*u4ISPGain/maxAfeGain;
    }

    sAEOutputInfo->u4ISO = sAEInputInfo->u4RealISO*binning_ratio;
 }
 ALOGD("[%s] PrepareCap u4Eposuretime:%d, u4AfeGain:%d, u4ISPGain:%d, u4RealISO:%d, maxGain = %d\n", __FUNCTION__, sAEOutputInfo->EvSetting.u4Eposuretime,sAEOutputInfo->EvSetting.u4AfeGain,sAEOutputInfo->EvSetting.u4IspGain,sAEOutputInfo->u4ISO,maxAfeGain);
 #endif
}

void ModifyCaptureParamByCustom(MUINT32 binning_ratio,AE_MODE_CFG_T *sAEInputInfo,AE_MODE_CFG_T *sAEOutputInfo,MUINT32 maxAfeGain)
{
 #ifdef CustomModifyFourCell
 ALOGD("[%s] u4Eposuretime:%d, u4AfeGain:%d u4IspGain:%d u4RealISO:%d\n", __FUNCTION__, sAEInputInfo->u4Eposuretime, sAEInputInfo->u4AfeGain,sAEInputInfo->u4IspGain,sAEInputInfo->u4RealISO);
 if(CustomModifyGetExpo || sAEInputInfo->u4Eposuretime < 200000){
    sAEOutputInfo->u4Eposuretime = sAEInputInfo->u4Eposuretime * binning_ratio;
    sAEOutputInfo->u4AfeGain = sAEInputInfo->u4AfeGain;
    sAEOutputInfo->u4IspGain = sAEInputInfo->u4IspGain;
    sAEOutputInfo->u4RealISO = sAEInputInfo->u4RealISO;
 }else {
    MUINT32 u4AfeGain = sAEInputInfo->u4AfeGain;
    MUINT32 u4ISPGain = sAEInputInfo->u4IspGain;

    sAEOutputInfo->u4Eposuretime = sAEInputInfo->u4Eposuretime;
    if(u4AfeGain*binning_ratio <= maxAfeGain){
      sAEOutputInfo->u4AfeGain = u4AfeGain*binning_ratio;
      sAEOutputInfo->u4IspGain = u4ISPGain;
    } else {
      sAEOutputInfo->u4AfeGain = maxAfeGain;
      sAEOutputInfo->u4IspGain = u4AfeGain*binning_ratio*u4ISPGain/maxAfeGain;
    }

    sAEOutputInfo->u4RealISO = sAEInputInfo->u4RealISO*binning_ratio;
 }
 ALOGD("[%s] GetExpo u4Eposuretime:%d, u4AfeGain:%d, u4ISPGain:%d, u4RealISO:%d, maxGain = %d\n", __FUNCTION__, sAEOutputInfo->u4Eposuretime,sAEOutputInfo->u4AfeGain,sAEOutputInfo->u4IspGain,sAEOutputInfo->u4RealISO,maxAfeGain);

 #endif
}