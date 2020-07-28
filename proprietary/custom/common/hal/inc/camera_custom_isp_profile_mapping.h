/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
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

#ifndef _CAMERA_ISP_PROFILE_MAPPING_HEADER_H_
#define _CAMERA_ISP_PROFILE_MAPPING_HEADER_H_

#include <map>
#include <vector>
#include <mtkcam/def/BuiltinTypes.h>
#include <isp_tuning/isp_tuning.h>
using std::map;

enum ESensorCombination
{
    eSensorComb_Invalid,
    eSensorComb_Single,
    eSensorComb_BayerBayer,
    eSensorComb_BayerMono
};

enum EProfileMappinScenario
{
    eMappingScenario_Preview,
    eMappingScenario_Capture,
    eMappingScenario_Video
};

enum EProfileMappingStages
{
    eStage_Y2Y_Main,
    eStage_R2Y_Main,
    eStage_R2YDCE_1stRun,
    eStage_Y2YDCE_2ndRun,
    eStage_R2Y_Sub,
    eStage_MDP,
    eStage_RRZ_R2Y_Main,
    eStage_RRZ_R2Y_Sub,
    eStage_AIBC_FE,
    eStage_chromaNR,
    // only for ISP5.x
    eStage_DSDN10_1stRun,
    eStage_DSDN10_2ndRun,
    // only for ISP6.0
    eStage_DSDN20_2ndRun
};

struct ProfileMappingItemKeys
{
    MUINT64 miFeatureID;
    EProfileMappinScenario mScenario;
    ESensorCombination mSensorComb = eSensorComb_Single;
};

struct ProfileMappingItem
{
    ProfileMappingItemKeys mKey;
    std::map<EProfileMappingStages, NSIspTuning::EIspProfile_T> mProfileMap;
};

extern const std::vector<ProfileMappingItem> gISPProfileMappingArray;
extern const std::map<EProfileMappingStages, NSIspTuning::EIspProfile_T> gDefaultProfileMap;
// ISP HIDL version
extern const std::vector<ProfileMappingItem> gISPProfileMappingArray_ISPHIDL;

#endif
