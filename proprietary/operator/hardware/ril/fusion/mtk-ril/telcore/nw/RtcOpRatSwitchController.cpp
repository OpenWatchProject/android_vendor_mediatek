/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RtcOpRatSwitchController.h"
#include "power/RtcRadioController.h"
#include "RfxRilUtils.h"

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/

#define RAT_CTRL_TAG "RtcOpRatSwitchCtrl"

RFX_IMPLEMENT_CLASS("RtcOpRatSwitchController", RtcOpRatSwitchController, RfxController);

RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_SET_DISABLE_2G);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxIntsData, RFX_MSG_REQUEST_GET_DISABLE_2G);

RtcOpRatSwitchController::RtcOpRatSwitchController() :
    mMessage(NULL),
    mError(RIL_E_SUCCESS),
    radioCount(0) {
}

RtcOpRatSwitchController::~RtcOpRatSwitchController() {
}

void RtcOpRatSwitchController::onInit() {
    // Required: invoke super class implementation
    RfxController::onInit();
    logV(RAT_CTRL_TAG, "[onInit]");
    const int request_id_list[] = {
        RFX_MSG_REQUEST_SET_DISABLE_2G
    };

    // register request & URC id list
    // NOTE. one id can only be registered by one controller
    registerToHandleRequest(request_id_list, sizeof(request_id_list)/sizeof(const int), DEFAULT);
}

bool RtcOpRatSwitchController::onHandleRequest(const sp<RfxMessage>& message) {
    int msg_id = message->getId();

    switch (msg_id) {
        case RFX_MSG_REQUEST_SET_DISABLE_2G:
            if (isOp08Support()) {
                // pend the request
                mMessage = message;
                requestRadioPower(false);
            } else {
                requestToMcl(message);
            }
            break;
        default:
            break;
    }
    return true;
}

bool RtcOpRatSwitchController::onHandleResponse(const sp<RfxMessage>& response) {
    int msg_id = response->getId();

    switch (msg_id) {
        case RFX_MSG_REQUEST_SET_DISABLE_2G:
            if (isOp08Support()) {
                mError = response->getError();
                requestRadioPower(true);
            } else {
                responseToRilj(response);
            }
            return true;
        default:
            logW(RAT_CTRL_TAG, "[onHandleResponse] %s", RFX_ID_TO_STR(msg_id));
            break;
    }
    return false;
}

void RtcOpRatSwitchController::requestRadioPower(bool state) {
    for (int slotId = RFX_SLOT_ID_0; slotId < RfxRilUtils::rfxGetSimCount(); slotId++) {
        sp<RfxAction> action0;
        bool power = state;
        RtcRadioController* radioController = (RtcRadioController *)findController(
                slotId, RFX_OBJ_CLASS_INFO(RtcRadioController));
        if (false == state) {
            action0 = new RfxAction1<int>(this, &RtcOpRatSwitchController::onRequestRadioOffDone, slotId);
            backupRadioPower[slotId] = getStatusManager(slotId)->getBoolValue(RFX_STATUS_KEY_REQUEST_RADIO_POWER, false);
            logD(RAT_CTRL_TAG, "backupRadioPower slotid=%d %d", slotId, backupRadioPower[slotId]);
        } else {
            action0 = new RfxAction1<int>(this, &RtcOpRatSwitchController::onRequestRadioOnDone, slotId);
            power = backupRadioPower[slotId];
            logD(RAT_CTRL_TAG, "restoreRadioPower slotid=%d %d", slotId, backupRadioPower[slotId]);
        }
        radioController->moduleRequestRadioPower(power, action0, RFOFF_CAUSE_UNSPECIFIED);
    }
}

void RtcOpRatSwitchController::onRequestRadioOffDone(int slotId) {
    radioCount++;
    logD(RAT_CTRL_TAG, "radioPowerOffDone slotid=%d RadioCount=%d", slotId, radioCount);
    if (radioCount == RfxRilUtils::rfxGetSimCount()) {
        radioCount = 0;
        requestToMcl(mMessage);
    }
}

void RtcOpRatSwitchController::onRequestRadioOnDone(int slotId) {
    radioCount++;
    logD(RAT_CTRL_TAG, "radioPowerOffDone slotid=%d RadioCount=%d", slotId, radioCount);
    if (radioCount == RfxRilUtils::rfxGetSimCount()) {
        radioCount = 0;
        if (mMessage != NULL) {
            sp<RfxMessage> resToRilj = RfxMessage::obtainResponse(mError, mMessage, false);
            responseToRilj(resToRilj);
            mMessage = NULL;
            mError = RIL_E_SUCCESS;
        }
    }
}

bool RtcOpRatSwitchController::onPreviewMessage(const sp<RfxMessage>& message) {
    if (message->getType() == REQUEST &&
            mMessage != NULL &&
            message->getId() == RFX_MSG_REQUEST_SET_DISABLE_2G) {
        logD(RAT_CTRL_TAG, "onPreviewMessage, put %s into pending list",
                RFX_ID_TO_STR(message->getId()));
        return false;
    } else {
        return true;
    }
}

bool RtcOpRatSwitchController::onCheckIfResumeMessage(const sp<RfxMessage>& message) {
    if (mMessage == NULL) {
        logD(RAT_CTRL_TAG, "resume the request %s",
                RFX_ID_TO_STR(message->getId()));
        return true;
    } else {
        return false;
    }
}

bool RtcOpRatSwitchController::onCheckIfRejectMessage(const sp<RfxMessage>& message,
        bool isModemPowerOff,int radioState) {
    /* Reject the request in radio unavailable or modem off */
    if (radioState == (int)RADIO_STATE_UNAVAILABLE ||
            isModemPowerOff == true) {
        logD(RAT_CTRL_TAG, "onCheckIfRejectMessage, id = %d, isModemPowerOff = %d, rdioState = %d",
                message->getId(), isModemPowerOff, radioState);
        return true;
    }
    return false;
}
