/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#define LOG_TAG "LmvState"

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam3/feature/3dnr/lmv_state.h>
#include <lmv_state_impl.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_3DNR_HAL);

using namespace android;


LmvStateMachineDummy::LmvStateMachineDummy(LmvState initState) : LmvStateMachine(initState)
{
    // A dummy state machine, do nothing
}


void LmvStateMachineDummy::switchTo(SensorId sensor, Action *action, Transition *transition)
{
    Mutex::Autolock lock(mStateMutex);
    LmvState newState = mCurrentState;

    action->cmd = DO_NOTHING;

    switch (sensor)
    {
    case WIDE:
        newState = STATE_ON_WIDE;
        break;
    case TELE:
        newState = STATE_ON_TELE;
        break;
    default:
        CAM_ULOGMD("[LmvState] Invalid sensor ID: %d", int(sensor));
        break;
    }

    if (transition != NULL)
    {
        transition->oldState = mCurrentState;
        transition->newState = newState;
    }

    if (mCurrentState != newState)
    {
        CAM_ULOGMD("[LmvState] Switch-to: %d, State: %d -> %d, Action: %d",
                int(sensor), int(mCurrentState), int(newState), int(action->cmd));
    }

    mCurrentState = newState;
}


bool LmvStateMachineDummy::needEnque(SensorId sensor, bool willFollow)
{
    (void)sensor; // unused
    (void)willFollow; // unused

    return false;
}


void LmvStateMachineDummy::notifySwitchResult(
        SensorId sourcePath, SwitchResult result, Transition *transition)
{
    // There should be no switch result!
    CAM_ULOGME("[LmvState] notifySwitchResult: sourcePath: %d, result = %d",
            int(sourcePath), int(result));

    if (transition != NULL)
    {
        Mutex::Autolock lock(mStateMutex);
        transition->oldState = mCurrentState;
        transition->newState = mCurrentState;
    }
}


void LmvStateMachineDummy::notifyLmvValidity(
        SensorId sourcePath, bool isValid, Transition *transition)
{
    if (!isValid)
    {
        // Incorrect, both LMVs should be valid
        CAM_ULOGME("[LmvState] notifyLmvValidity: sourcePath: %d, isValid = %d",
                int(sourcePath), int(isValid));
    }

    if (transition != NULL)
    {
        Mutex::Autolock lock(mStateMutex);
        transition->oldState = mCurrentState;
        transition->newState = mCurrentState;
    }
}

