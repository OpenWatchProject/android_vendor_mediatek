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

#ifndef __LMV_STATE_H__
#define __LMV_STATE_H__


#include <utils/Mutex.h>


class LmvStateMachine
{
public:
    enum SensorId {
        WIDE = 0,
        TELE,
        SENSOR_ID_MAX
    };

    enum LmvState {
        STATE_ON_WIDE,
        STATE_GOING_TO_TELE,
        STATE_ON_TELE,
        STATE_GOING_TO_WIDE
    };

    enum SwitchAction {
        DO_NOTHING,
        SEND_SWITCH_OUT_TO_WIDE, // Send switch-out to the wide pipeline
        SEND_SWITCH_OUT_TO_TELE  // Send switch-out to the tele pipeline
    };

    // The result of switch_out command from P1 output metadata
    enum SwitchResult {
        RESULT_OK,
        RESULT_FAILED,
        RESULT_SWITCHING
    };

    struct Transition {
        LmvState oldState;
        LmvState newState;
    };

    struct Action {
        SwitchAction cmd;
    };

public:

    // Create singleton when system init
    // initState    [IN]  The initial state of LMV
    static LmvStateMachine *createInstance(LmvState initState = STATE_ON_WIDE);

    // Destroy the singleton when system deinit
    static void destroyInstance();

    // Get the singleton of LMV state machine.
    static LmvStateMachine *getInstance();

    // Query the action which can switch LMV to expected pipeline path.
    // Client MUST follow the action to send switch_out command.
    //
    // Parameters:
    //  sensor      [IN]  Expected working pipeline, wide or tele
    //  action      [OUT] Action must be followed
    //  transition  [OUT] Optional. The transition of the state machine on this call
    virtual void switchTo(SensorId sensor, Action *action, Transition *transition = NULL) = 0;

    // Notify the switch_out result which carried by the output metadata of P1
    //
    // Parameters:
    //  sourcePath  [IN] From which pipeline, wide or tele
    //  result      [IN] Switch_out result
    //  transition  [OUT] Optional. The transition of the state machine on this call
    virtual void notifySwitchResult(SensorId sourcePath, SwitchResult result, Transition *transition = NULL) = 0;

    // Notify the LMV data validity which carried by the output metadata of P1
    //
    // Parameters:
    //  sourcePath  [IN] From which pipeline, wide or tele
    //  isValid     [IN] LMV data validity
    //  transition  [OUT] Optional. The transition of the state machine on this call
    virtual void notifyLmvValidity(SensorId sourcePath, bool isValid, Transition *transition = NULL) = 0;

    // Get current LMV state
    LmvState getCurrentState();

    // LMV state needs extra enques to get stable.
    // FlowControl has to continue enqueing until this API return false
    // Parameters:
    //  sensor      [IN] Does the sensor need enque?
    //  willFollow  [IN] FlowControl guarantees the enque will be performed
    //                   If true, needEnque() will decrease internal counter
    virtual bool needEnque(SensorId sensor, bool willFollow) = 0;

    // Not allow copy-construction
    LmvStateMachine(const LmvStateMachine &) = delete;

    // Not allow copy-assignment
    LmvStateMachine& operator = (const LmvStateMachine &) = delete;

protected:

    android::Mutex mStateMutex;
    LmvState mCurrentState;

    LmvStateMachine(LmvState initState);
    virtual ~LmvStateMachine();

private:

    static LmvStateMachine* spInstance;
    static android::Mutex sSingletonMutex;
};


#endif

