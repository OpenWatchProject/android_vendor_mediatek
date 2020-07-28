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


const int LmvStateMachineUni::N_EXTRA_ENQUES = 1;


LmvStateMachineUni::LmvStateMachineUni(LmvState initState) : LmvStateMachine(initState)
{
    mWrongStateCount = 0;
    mEnqueRemain[WIDE] = 0;
    mEnqueRemain[TELE] = 0;
}


void LmvStateMachineUni::switchTo(SensorId sensor, Action *action, Transition *transition)
{
    Mutex::Autolock lock(mStateMutex);
    LmvState newState = mCurrentState;

    // CAM_LOGD("[LmvState] LmvStateMachine::switchTo(%d)", int(sensor));

    action->cmd = DO_NOTHING;

    switch (mCurrentState) {
    case STATE_ON_WIDE:
        if (sensor == TELE)
        {
            newState = STATE_GOING_TO_TELE;
            action->cmd = SEND_SWITCH_OUT_TO_WIDE;
            mEnqueRemain[WIDE] = N_EXTRA_ENQUES;
        }
        // else ignore
        break;
    case STATE_GOING_TO_TELE:
        // Ignore any switch-to query, we are waiting for the result
        break;
    case STATE_ON_TELE:
        if (sensor == WIDE)
        {
            newState = STATE_GOING_TO_WIDE;
            action->cmd = SEND_SWITCH_OUT_TO_TELE;
            mEnqueRemain[TELE] = N_EXTRA_ENQUES;
        }
        // else ignore
        break;
    case STATE_GOING_TO_WIDE:
        // Ignore any switch-to query, we are waiting for the result
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


bool LmvStateMachineUni::needEnque(SensorId sensor, bool willFollow)
{
    Mutex::Autolock lock(mStateMutex);

    bool need = (mEnqueRemain[sensor] > 0);

    if (need && willFollow) {
        CAM_ULOGMD("[LmvState] needEnque(): Will enque (mEnqueRemain[%d] = %d)", int(sensor), mEnqueRemain[sensor]);
        mEnqueRemain[sensor]--;
    }

    return need;
}


void LmvStateMachineUni::notifySwitchResult(
        SensorId sourcePath, SwitchResult result, Transition *transition)
{
    Mutex::Autolock lock(mStateMutex);
    LmvState newState = mCurrentState;

    switch (mCurrentState) {
    case STATE_GOING_TO_TELE:
        if (result == RESULT_OK)
        {
            newState = STATE_ON_TELE;
        }
        else if (result == RESULT_FAILED)
        {
            newState = STATE_ON_TELE;
        }
        else if (result == RESULT_SWITCHING)
        {
            newState = STATE_ON_WIDE;
        }
        break;
    case STATE_GOING_TO_WIDE:
        if (result == RESULT_OK)
        {
            newState = STATE_ON_WIDE;
        }
        else if (result == RESULT_FAILED)
        {
            newState = STATE_ON_WIDE;
        }
        else if (result == RESULT_SWITCHING)
        {
            newState = STATE_ON_TELE;
        }
        break;
    default:
        CAM_ULOGME("[LmvState] State: %d, Source path: %d, Result: %d",
                int(mCurrentState), int(sourcePath), int(result));
        break;
    }

    if (transition != NULL)
    {
        transition->oldState = mCurrentState;
        transition->newState = newState;
    }

    if (mCurrentState != newState)
    {
        CAM_ULOGMD("[LmvState] Result: %d, State: %d -> %d",
                int(result), int(mCurrentState), int(newState));
    }

    mCurrentState = newState;
}


void LmvStateMachineUni::notifyLmvValidity(
        SensorId sourcePath, bool isValid, Transition *transition)
{
    if (!isValid)
        return;

    // Auto-recovery machanism

    Mutex::Autolock lock(mStateMutex);

    switch (mCurrentState) {
    case STATE_ON_WIDE:
        if (sourcePath == WIDE)
        {
            mWrongStateCount = 0; // correct
        }
        else
        {
            mWrongStateCount += 2;
            CAM_ULOGMW("[LmvState] State: %d, but valid LMV from %d", mCurrentState, sourcePath);
        }
        break;
    case STATE_ON_TELE:
        if (sourcePath == TELE)
        {
            mWrongStateCount = 0; // correct
        }
        else
        {
            mWrongStateCount += 2;
            CAM_ULOGMW("[LmvState] State: %d, but valid LMV from %d", mCurrentState, sourcePath);
        }
        break;
    case STATE_GOING_TO_TELE:
        // temporary state
        mWrongStateCount = (sourcePath == TELE) ? 0 : (mWrongStateCount + 1);
        break;
    case STATE_GOING_TO_WIDE:
        // temporary state
        mWrongStateCount = (sourcePath == WIDE) ? 0 : (mWrongStateCount + 1);
        break;
    }

    if (transition != NULL)
        transition->oldState = mCurrentState;

    if (mWrongStateCount >= 10) // too many wrongs
    {
        LmvState newState = mCurrentState;

        if (sourcePath == WIDE)
            newState = STATE_ON_WIDE;
        else
            newState = STATE_ON_TELE;

        mWrongStateCount = 0;

        CAM_ULOGME("[LmvState] State recovery: %d -> %d because received too many LMV validity from wrong path",
                int(mCurrentState), int(newState));

        mCurrentState = newState;
    }

    if (transition != NULL)
        transition->newState = mCurrentState;
}

