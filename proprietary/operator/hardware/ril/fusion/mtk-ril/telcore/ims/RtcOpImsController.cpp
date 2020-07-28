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
#include <vector>
#include "RtcOpImsController.h"
#include "RfxRootController.h"

#include "RfxIntsData.h"
#include "RfxStringsData.h"
#include "RfxVoidData.h"
#include "RfxMessageId.h"

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/

#define RFX_LOG_TAG "RtcOpIms"


RFX_IMPLEMENT_CLASS("RtcOpImsController", RtcOpImsController, RfxController);

RFX_REGISTER_DATA_TO_URC_ID(RfxStringsData, RFX_MSG_UNSOL_GET_TRN_INDICATION);

RtcOpImsController::RtcOpImsController() {
}

RtcOpImsController::~RtcOpImsController() {
}

void RtcOpImsController::onInit() {
    // Required: invoke super class implementation
    RfxController::onInit();
    const int request_id_list[] = {
        RFX_MSG_REQUEST_SET_DIGITS_LINE,
        RFX_MSG_REQUEST_SET_TRN,
        RFX_MSG_REQUEST_SWITCH_RCS_ROI_STATUS,
        RFX_MSG_REQUEST_UPDATE_RCS_CAPABILITIES,
        RFX_MSG_REQUEST_UPDATE_RCS_SESSION_INFO
    };

    const int urc_id_list[] = {
        RFX_MSG_UNSOL_DIGITS_LINE_INDICATION,
        RFX_MSG_UNSOL_GET_TRN_INDICATION,
        RFX_MSG_UNSOL_RCS_DIGITS_LINE_INFO
    };

    // register request & URC id list
    // NOTE. one id can only be registered by one controller
    registerToHandleRequest(request_id_list, sizeof(request_id_list)/sizeof(const int), DEFAULT);
    registerToHandleUrc(urc_id_list, sizeof(urc_id_list)/sizeof(const int));
}

void RtcOpImsController::onDeinit() {
    RfxController::onDeinit();
}

bool RtcOpImsController::onHandleRequest(const sp<RfxMessage>& message) {
    int msg_id = message->getId();

    switch (msg_id) {
        case RFX_MSG_REQUEST_SET_TRN:
            handleSetTrn(message);
            return true;
        default:
            break;
    }
    requestToMcl(message);
    return true;
}

bool RtcOpImsController::onHandleUrc(const sp<RfxMessage>& message) {
    responseToRilj(message);
    return true;
}

bool RtcOpImsController::onHandleResponse(const sp<RfxMessage>& response) {
    responseToRilj(response);
    return true;
}

bool RtcOpImsController::onCheckIfRejectMessage(const sp<RfxMessage>& message,
        bool isModemPowerOff,int radioState) {

    /* Reject the request when modem off */
    if (isModemPowerOff == true) {
        return true;
    }
    return false;
}

void RtcOpImsController::handleSetTrn(const sp<RfxMessage>& message) {
    char** data = (char**)message->getData()->getData();

    // data[0] is fromMsisdn
    // data[1] is toMsisdn
    // data[2] is TRN
    // When TRN is invalid as NULL or empty, set TRN as original toMsisdn.
    if (data[2] == NULL || strlen(data[2]) == 0) {
        String8 trn(data[1]);
        getStatusManager()->setString8Value(RFX_STATUS_KEY_TRN, trn);
    } else {
        String8 trn(data[2]);
        getStatusManager()->setString8Value(RFX_STATUS_KEY_TRN, trn);
    }

    // create the KEY for callback query with format as <fromMsisdn>_to_<toMsisdn>
    int len = strlen(data[0]) + strlen(data[1]) + 4;
    char* address = (char *)alloca(len + 1);
    memset(address, 0, len + 1);
    strncpy(address, data[0], strlen(data[0]));
    strncat(address, "_to_", 4);
    strncat(address, data[1], len - strlen(data[1]));
    std::string addrString(address);

    // If KEY not matched, won't callback
    sp<RfxAction> action = findAction(addrString);
    if ((sp<RfxAction>) NULL != action) {
        action->act();
        sp<RfxMessage> responseMsg = RfxMessage::obtainResponse(RIL_E_SUCCESS, message, true);
        responseToRilj(responseMsg);
    } else {
        sp<RfxMessage> responseMsg = RfxMessage::obtainResponse(RIL_E_GENERIC_FAILURE, message, true);
        responseToRilj(responseMsg);
    }
}

sp<RfxAction> RtcOpImsController::findAction(std::string addrString) {
    std::unordered_map<std::string, sp<RfxAction>>::const_iterator result =
            mActionMap.find(addrString);

    if (result == mActionMap.end()) {
        return NULL;
    }
    sp<RfxAction> action = result->second;
    mActionMap.erase(addrString);
    return action;
}

void RtcOpImsController::getTrn(const char* fromMsisdn, const char* toMsisdn, const char* address,
        const sp<RfxAction>& action) {
    // create the KEY for callback query with format as <fromMsisdn>_to_<toMsisdn>
    char *data[2];
    data[0] = strdup(fromMsisdn);
    data[1] = strdup(toMsisdn);
    sp<RfxMessage> urc = RfxMessage::obtainUrc(m_slot_id, RFX_MSG_UNSOL_GET_TRN_INDICATION,
            RfxStringsData(data, 2));
    responseToRilj(urc);

    std::string addrString(address);

    if ((sp<RfxAction>) NULL != action) {
        mActionMap.insert({addrString, action});
    }
}
