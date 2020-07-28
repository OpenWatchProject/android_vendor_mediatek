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

#ifndef AAA_COMMON_CUSTOM_H_
#define AAA_COMMON_CUSTOM_H_

#include "camera_custom_nvram.h"    //CAM_SCENARIO_T;
#include "aaa/ae_param.h"               //AE_MODE;
#include <isp_tuning.h>
#include <vector>

MBOOL CUST_ENABLE_PRECAPTURE_AF(void);
MBOOL CUST_PRECAPTURE_AF_AFTER_PREFLASH(void);
MBOOL CUST_ENABLE_VIDEO_AUTO_FLASH(void);
MBOOL CUST_CAF_WINDOW_FOLLOW_TAF_WINDOW(void);
MBOOL CUST_ONE_SHOT_AE_BEFORE_TAF(void);
MBOOL CUST_SKIP_ONE_SHOT_AE_FOR_TAF(void);
MBOOL CUST_ENABLE_TOUCH_AE(void);
MBOOL CUST_ENABLE_FACE_AE(void);
MBOOL CUST_ENABLE_FACE_AWB(void);
MBOOL CUST_LOCK_AE_DURING_CAF(void);
MBOOL CUST_ENABLE_VIDEO_DYNAMIC_FRAME_RATE(void);
MBOOL CUST_ENABLE_FLASH_DURING_TOUCH(void);
MUINT32 CUST_FACE_AWB_CLEAR_COUNT(void);
MUINT32 CUST_GET_SYNC3A_AESTABLE_MAGIC(void);
MINT32 CUST_GET_SKIP_PRECAP_FLASH_FRAME_COUNT(void);
MBOOL CUST_LENS_COVER_COUNT(MINT32 i4LvMaster, MINT32 i4LvSlave, MINT32 i4Count);

struct ScenarioParam{
    NSIspTuning::EIspProfile_T eIspProfile;
    unsigned char CaptureIntent;
    unsigned char HdrMode;
    int Sync2AMode;
    int TargetMode;
    unsigned int SensorMode;

    ScenarioParam(): eIspProfile(NSIspTuning::EIspProfile_Preview), CaptureIntent(1), HdrMode(0), Sync2AMode(0), TargetMode(0), SensorMode(0){}
    ScenarioParam(NSIspTuning::EIspProfile_T IspProfile, unsigned char cap, unsigned char hdr, int sync2a, int target, unsigned int sensor)
    : eIspProfile(IspProfile)
    , CaptureIntent(cap)
    , HdrMode(hdr)
    , Sync2AMode(sync2a)
    , TargetMode(target)
    , SensorMode(sensor)
    {}
};

unsigned int Scenario4AE(const ScenarioParam&);
unsigned int Scenario4AWB(const ScenarioParam&);
unsigned int Scenario4AF(const ScenarioParam&);

std::vector<int> getShortExpFrame(void);

void cust_initSpecialLongExpOnOff(const MINT32 &i4AEEffectiveFrame);
void cust_setSpecialLongExpOnOff(const MINT64 &i8ExposureTime);
MBOOL cust_getIsSpecialLongExpOn();

#endif /* AAA_COMMON_CUSTOM_H_ */

