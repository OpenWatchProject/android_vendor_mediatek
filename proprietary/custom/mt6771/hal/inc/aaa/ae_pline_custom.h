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

#ifndef AE_MANUAL_PLINE_CUSTOM_H_
#define AE_MANUAL_PLINE_CUSTOM_H_

#include "../../../../common/hal/inc/custom/aaa/ae_param.h"

#include <ae_feature.h>
#include <camera_custom_nvram.h>
#include <camera_custom_AEPlinetable.h>
#include "camera_custom_types.h"
#include "aaa/ae_param.h"

#define CUST_AE_MP_PARAM MFALSE

enum EAEManualPline_T
{
    EAEManualPline_Default           = 0x0000,
    EAEManualPline_ADBCtrol,

    // feature manual pline
    EAEManualPline_EISRecord         = 0x0010,
    EAEManualPline_SM240FPS,
    EAEManualPline_AIS1Capture,
    EAEManualPline_AIS2Capture,
    EAEManualPline_MFHRCapture,
    EAEManualPline_BMDNCapture,

    // customer manual pline
    EAEManualPline_Custom1           = 0x0100,
    EAEManualPline_Custom2,
    EAEManualPline_Custom3,
    EAEManualPline_Custom4,
    EAEManualPline_Custom5,

    EAEManualPline_Num
};

typedef struct MPParam{
    int MinFps;
    int MaxFps;
    int SensorMode;
    int SceneMode;
    int ISOSpeed;
    MPParam(): MinFps(0), MaxFps(0), SensorMode(0), SceneMode(0), ISOSpeed(0){}
    MPParam(int minFps, int maxFps, int sensorMode, int sceneMode, int isoMode)
    : MinFps(minFps)
    , MaxFps(maxFps)
    , SensorMode(sensorMode)
    , SceneMode(sceneMode)
    , ISOSpeed(isoMode)
    {}
} ManualPlineParam;

void getAEManualPline(const ManualPlineParam& manualPlineParam, eAETableID& a_AEManualPreTableID, eAETableID& a_AEManualCapTableID);
void getAEManualPline(const EAEManualPline_T& e_AEManualPline, eAETableID& a_AEManualPreTableID, eAETableID& a_AEManualCapTableID);
void getAEManualCapPline(const EAEManualPline_T& e_AEManualPline, eAETableID& a_AEManualTableID);
void ModifyCaptureParamByCustom(MUINT32 binning_ratio,AE_MODE_CFG_T *sAEInputInfo,strAEOutput *sAEOutputInfo,MUINT32 maxAfeGain);
void ModifyCaptureParamByCustom(MUINT32 binning_ratio,AE_MODE_CFG_T *sAEInputInfo,AE_MODE_CFG_T *sAEOutputInfo,MUINT32 maxAfeGain);

#endif /* AE_MANUAL_PLINE_CUSTOM_H_ */

