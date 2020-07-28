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

#define LOG_TAG "af_flow_custom"

#include "af_flow_custom.h"
#include <isp_tuning.h>

/* -----------------------------------------------------
                    checkSendCallback
--------------------------------------------------------
@@ isSearch Types:
 AF_SEARCH_DONE        = 0,
 AF_SEARCH_TARGET_MOVE = 1,  // PDAF Move
 AF_SEARCH_TARGET_FS   = 2,  // PDAF Fine Search
 AF_SEARCH_CONTRAST    = 3,

@@ Default Callback Rules:
 - ContrastAF
    #1. CAF, ContrastAF, Default                                            ==> MTRUE
    #2. CAF, ContrastAF, (PDCL.Value>=PDCL.Target)||(ISO.Value<=ISO.Target) ==> MFALSE
    #3. CAF, ContrastAF, Former AF is triggered by Touch AF                 ==> MTRUE
 - PD + FS
    #4. CAF,    Move+FS, Default                                            ==> MFALSE
    #5. CAF,    Move+FS, FPS.Value<=FPS.Target                              ==> MTRUE
    #6. CAF,    Move+FS, Former AF is triggered by Touch AF                 ==> MTRUE

@@ Notice
Please contact to the AF owner if more CompareSets need to be included.
----------------------------------------------------- */
MBOOL checkSendCallback(CHECK_AF_CALLBACK_INFO_T CBInfo)
{
    MBOOL isNeedSendCallback = MTRUE;

    if(CBInfo.isSearching==AF_SEARCH_CONTRAST) // Contrast
    {
        // Callback rules: #1
        isNeedSendCallback = MTRUE;

        // Callback rules: #2
        if((CBInfo.CompSet_PDCL.Target > 0 && CBInfo.CompSet_PDCL.Value >= CBInfo.CompSet_PDCL.Target) ||
           (CBInfo.CompSet_ISO.Target  > 0 && CBInfo.CompSet_ISO.Value  <= CBInfo.CompSet_ISO.Target) )
        {
            isNeedSendCallback = MFALSE;
        }

        // Callback rules: #3
        if(CBInfo.isAfterAutoMode)
        {
            isNeedSendCallback = MTRUE;
        }

    }
    else if(CBInfo.isSearching==AF_SEARCH_TARGET_MOVE || CBInfo.isSearching==AF_SEARCH_TARGET_FS || CBInfo.isSearching==AF_SEARCH_READY) // Move + Fine Search
    {
        // Callback rules: #4
        isNeedSendCallback = MFALSE;

        // Callback rules: #5
        if(CBInfo.CompSet_FPS.Target > 0 && CBInfo.CompSet_FPS.Value <= CBInfo.CompSet_FPS.Target)
        {
            isNeedSendCallback = MTRUE;
        }

        // Callback rules: #6
        if(CBInfo.isAfterAutoMode)
        {
            isNeedSendCallback = MTRUE;
        }

    }

    return isNeedSendCallback;
}

MBOOL ForceEnableFVInFixedFocus(MINT32 i4SensorDev)
{
    MBOOL ret = MFALSE;

    switch (i4SensorDev)
    {
    case NSIspTuning::ESensorDev_Main :
        ret = MFALSE;
        break;
    case NSIspTuning::ESensorDev_Sub:
        ret = MFALSE;
        break;
    case NSIspTuning::ESensorDev_MainSecond:
        ret = MFALSE;
        break;
    case NSIspTuning::ESensorDev_SubSecond:
        ret = MFALSE;
        break;
    default:
        ret = MFALSE;
        break;
    }

    return ret;
}
