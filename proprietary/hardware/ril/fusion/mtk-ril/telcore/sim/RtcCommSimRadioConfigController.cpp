/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2018. All rights reserved.
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
#include "RtcCommSimRadioConfigController.h"
#include "RfxRilUtils.h"
#include "RfxLog.h"
#include "RfxAction.h"
#include "RfxMainThread.h"
#include "RfxSimSlotStatusData.h"

#define RC_LOG_TAG "RtcRadioConfig"

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/
RFX_IMPLEMENT_CLASS("RtcCommSimRadioConfigController", RtcCommSimRadioConfigController,
        RfxController);

RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxSimSlotStatusData,
        RFX_MSG_REQUEST_GET_SLOT_STATUS);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData,
        RFX_MSG_REQUEST_SET_LOGICAL_TO_PHYSICAL_SLOT_MAPPING);
RFX_REGISTER_DATA_TO_URC_ID(RfxSimSlotStatusData, RFX_MSG_URC_ICC_SLOT_STATUS);

RtcCommSimRadioConfigController::RtcCommSimRadioConfigController() {
    pp_slotStatus = NULL;
    sending_count = 0;
}

RtcCommSimRadioConfigController::~RtcCommSimRadioConfigController() {
}

void::RtcCommSimRadioConfigController::onDeinit() {
}

void RtcCommSimRadioConfigController::onInit() {
    // Required: invoke super class implementation
    RfxController::onInit();

    const int request_id_list[] = {
        RFX_MSG_REQUEST_GET_SLOT_STATUS,
        RFX_MSG_REQUEST_SET_LOGICAL_TO_PHYSICAL_SLOT_MAPPING,
    };

    const int urc_id_list[] = {
        RFX_MSG_URC_ICC_SLOT_STATUS,
    };

    // register request & URC id list
    // NOTE. one id can only be registered by one controller
    for (int i = 0; i < RfxRilUtils::rfxGetSimCount(); i++) {
        registerToHandleRequest(i, request_id_list, sizeof(request_id_list)/sizeof(const int));
        registerToHandleUrc(i, urc_id_list, sizeof(urc_id_list)/sizeof(const int));
    }

    // register callbacks to get required information
    logD(RC_LOG_TAG, "RtcCommSimRadioConfigController::onInit");
    pp_slotStatus = (RIL_SimSlotStatus **)malloc(RfxRilUtils::rfxGetSimCount() * sizeof(RIL_SimSlotStatus *));
}

bool RtcCommSimRadioConfigController::onPreviewMessage(const sp<RfxMessage>& message) {
    if (message->getType() == URC) {
        return true;
    }

    // Send withouting check card state. It would return "RIL_E_SIM_ERR" if card is busy.
    /*
    if (message->getType() == REQUEST && message->getId() == RFX_MSG_REQUEST_GET_SLOT_STATUS) {
        for (int i = 0; i < RfxRilUtils::rfxGetSimCount(); i++) {
            if (getStatusManager(i)->getBoolValue(RFX_STATUS_KEY_MODEM_SIM_TASK_READY) == false) {
                logD(RC_LOG_TAG, "onPreviewMessage  return false");
                 return false;
            }
        }
        return true;
    } else {
        return true;
    } */

    return true;
}

bool RtcCommSimRadioConfigController::onCheckIfResumeMessage(const sp<RfxMessage>& message) {
    if (message->getType() == URC) {
        return true;
    }

    // Send without checking card state. It would return "RIL_E_SIM_ERR" if card is busy.
    /*
    if (message->getType() == REQUEST && message->getId() == RFX_MSG_REQUEST_GET_SLOT_STATUS) {
        for (int i = 0; i < RfxRilUtils::rfxGetSimCount(); i++) {
            if (getStatusManager(i)->getBoolValue(RFX_STATUS_KEY_MODEM_SIM_TASK_READY) == false) {
                logD(RC_LOG_TAG, "onCheckIfResumeMessage return false");
                return false;
            }
        }
        logD(RC_LOG_TAG, "onCheckIfResumeMessage return true");
        return true;
    } else {
        return true;
    } */

    return true;
}

bool RtcCommSimRadioConfigController::onHandleRequest(const sp<RfxMessage>& message) {
    int msgId = message->getId();
    int mainSlotId = 0;

   // sp<RfxMessage> msg = NULL;
    switch (msgId) {
    case RFX_MSG_REQUEST_GET_SLOT_STATUS:
    case RFX_MSG_REQUEST_SET_LOGICAL_TO_PHYSICAL_SLOT_MAPPING:
        logD(RC_LOG_TAG, "onHandleRequest, slot id = %d", message->getSlotId());
        requestToMcl(message);
        break;
    default:
        break;
    }
    return true;
}

bool RtcCommSimRadioConfigController::onHandleUrc(const sp<RfxMessage>& message) {
    int msgId = message->getId();
    responseToRilj(message);
    return true;
}

bool RtcCommSimRadioConfigController::onHandleResponse(const sp<RfxMessage>& response) {
    int msgId = response->getId();
    int slotId = response->getSlotId();
    switch (msgId) {
    case RFX_MSG_REQUEST_GET_SLOT_STATUS:
    case RFX_MSG_REQUEST_SET_LOGICAL_TO_PHYSICAL_SLOT_MAPPING:
        responseToRilj(response);
        return true;
    default:
        break;
    }
    return true;
}

bool RtcCommSimRadioConfigController::onCheckIfRejectMessage(const sp<RfxMessage>& message,
        bool isModemPowerOff, int radioState) {
    int msgId = message->getId();

    if (!isModemPowerOff && (radioState == (int)RADIO_STATE_OFF) &&
            (msgId == RFX_MSG_REQUEST_GET_SLOT_STATUS ||
            msgId == RFX_MSG_REQUEST_SET_LOGICAL_TO_PHYSICAL_SLOT_MAPPING)) {
        return false;
    }

    return RfxController::onCheckIfRejectMessage(message, isModemPowerOff, radioState);
}

