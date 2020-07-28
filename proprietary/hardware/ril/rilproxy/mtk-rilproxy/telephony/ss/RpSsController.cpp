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
#include "RpSsController.h"
#include "RfxLog.h"
#include "util/RpFeatureOptionUtils.h"
#include <string.h>
#include <cutils/jstring.h>
#include <libmtkrilutils.h>
#include <unistd.h>

#ifdef RFX_LOG_TAG
#undef RFX_LOG_TAG
#endif
#define RFX_LOG_TAG "RpSsController"

using ::android::String16;

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/

RFX_IMPLEMENT_CLASS("RpSsController", RpSsController, RfxController);

RpSsController::RpSsController() {
    // We send USSD URC to GSM by default
    mUssdDestination = USSD_URC_TO_GSM;
}

RpSsController::~RpSsController() {
}

void RpSsController::onInit() {
    RfxController::onInit();  // Required: invoke super class implementation
    RFX_LOG_D(RFX_LOG_TAG, "onInit E! (slot %d)", getSlotId());

    const int request_id_list[] = {
        RIL_REQUEST_SEND_USSD,
        RIL_REQUEST_CANCEL_USSD,
        RIL_REQUEST_SEND_USSI,
        RIL_REQUEST_CANCEL_USSI
    };

    const int urc_id_list[] = {
        RIL_UNSOL_ON_USSD,
        RIL_UNSOL_ON_USSI
    };

    // register request & URC id list
    // NOTE. one id can only be registered by one controller
    if (RpFeatureOptionUtils::isC2kSupport()) {
        registerToHandleRequest(request_id_list, sizeof(request_id_list)/sizeof(const int));
        registerToHandleUrc(urc_id_list, sizeof(urc_id_list)/sizeof(const int));
        RFX_LOG_D(RFX_LOG_TAG, "request size:%zu urc size:%zu!",
            sizeof(request_id_list)/sizeof(const int), sizeof(urc_id_list)/sizeof(const int));
    }
}

void RpSsController::onDeInit() {
    RFX_LOG_D(RFX_LOG_TAG, "onDeinit X! (slot %d)", getSlotId());

    getStatusManager()->setBoolValue(RFX_STATUS_KEY_USSD_SESSION_ONGOING, false);

    RfxController::onDeinit();
}

bool RpSsController::onHandleRequest(const sp<RfxMessage>& message) {
    RFX_LOG_D(RFX_LOG_TAG, "[RpSsController] handle req %d (slot %d)", message->getId(),
            getSlotId());

    int msg_id = message->getId();
    int token = message->getPToken();
    switch(msg_id) {
        case RIL_REQUEST_SEND_USSD:  // fall through
        case RIL_REQUEST_SEND_USSI:
            if (getStatusManager()->getBoolValue(RFX_STATUS_KEY_USSD_SESSION_ONGOING)) {
                RFX_LOG_D(RFX_LOG_TAG, "USSD session is ongoing, rejected.");
                sendFailureReport(message);
                return true;
            } else {
                RFX_LOG_D(RFX_LOG_TAG, "USSI session started, token = %d", token);
                getStatusManager()->setBoolValue(RFX_STATUS_KEY_USSD_SESSION_ONGOING, true);
            }

            if (msg_id == RIL_REQUEST_SEND_USSD) {
                setUssdDestination(USSD_URC_TO_GSM);
            } else {
                setUssdDestination(USSD_URC_TO_IMS);
            }
            break;
        case RIL_REQUEST_CANCEL_USSD:  // fall through
        case RIL_REQUEST_CANCEL_USSI:
            if (getStatusManager()->getBoolValue(RFX_STATUS_KEY_USSD_SESSION_ONGOING)) {
                RFX_LOG_D(RFX_LOG_TAG, "USSD session canceled.");
                getStatusManager()->setBoolValue(RFX_STATUS_KEY_USSD_SESSION_ONGOING, false);
            }
            setUssdDestination(USSD_URC_TO_GSM);
            break;
        default:
            break;
    }
    requestToRild(message);
    return true;
}

bool RpSsController::onHandleUrc(const sp<RfxMessage>& message) {
    RFX_LOG_D(RFX_LOG_TAG, "[RpSsController] handle urc %d (slot %d)", message->getId(),
            getSlotId());
    int msg_id = message->getId();

    switch(msg_id) {
        case RIL_UNSOL_ON_USSD:  // fall through
        case RIL_UNSOL_ON_USSI:
            handleUssdUrc(message);
            break;
        default:
            responseToRilj(message);
            break;
    }
    return true;
}

bool RpSsController::onHandleResponse(const sp<RfxMessage>& message) {
    int msg_id = message->getId();
    int token = message->getPToken();

    RFX_LOG_D(RFX_LOG_TAG, "[RpSsController] handle %d response (slot %d)", msg_id, getSlotId());

    switch (msg_id) {
        case RIL_REQUEST_SEND_USSD:  // fall through
        case RIL_REQUEST_SEND_USSI:
            if(message->getError() != RIL_E_SUCCESS) {
                logD(RFX_LOG_TAG, "USSD session failed, end session, token = %d", token);
                getStatusManager()->setBoolValue(RFX_STATUS_KEY_USSD_SESSION_ONGOING, false);
            }
            break;
        default:
            break;
    }

    responseToRilj(message);
    return true;
}

char* RpSsController::strdupReadString(Parcel* p) {
    size_t stringlen;
    const char16_t *s16;

    s16 = p->readString16Inplace(&stringlen);

    return strndup16to8(s16, stringlen);
}

void RpSsController::handleUssdUrc(const sp<RfxMessage>& message) {
    Parcel *p = message->getParcel();
    int token = message->getPToken();
    int32_t count = -1;
    char *ussdModeStr = NULL;
    int newId = RIL_UNSOL_ON_USSD;

    p->readInt32(&count);
    ussdModeStr = strdupReadString(p);
    RFX_LOG_D(RFX_LOG_TAG, "handleUssdUrc, count = %d, ussdModeStr = %s",
            count, ussdModeStr);

    newId = (getUssdDestination() == USSD_URC_TO_GSM) ? RIL_UNSOL_ON_USSD
            : RIL_UNSOL_ON_USSI;
    sp<RfxMessage> urcToRilj = RfxMessage::obtainUrc(getSlotId(), newId, message, true);

    // If USSD mode equals to 1, it means further user action is required.
    // Keep the UssdDestination the same. Otherwise, change mUssdDestination to
    // default value. Because the session come to an end.
    if (strncmp(ussdModeStr, "1", 1) == 0) {
        setUssdDestination(getUssdDestination());
    } else {
        setUssdDestination(USSD_URC_TO_GSM);
    }

    if (getStatusManager()->getBoolValue(RFX_STATUS_KEY_USSD_SESSION_ONGOING)) {
        RFX_LOG_D(RFX_LOG_TAG, "USSD session completed, token = %d", token);
        getStatusManager()->setBoolValue(RFX_STATUS_KEY_USSD_SESSION_ONGOING, false);
    }

    responseToRilj(urcToRilj);
    free(ussdModeStr);
}

UssdDestination RpSsController::getUssdDestination() {
    RFX_LOG_D(RFX_LOG_TAG, "getUssdDestination(): mUssdDestination = %s",
            ussdDestinationToString(mUssdDestination));
    return mUssdDestination;
}

void RpSsController::setUssdDestination(UssdDestination destination) {
    RFX_LOG_D(RFX_LOG_TAG, "setUssdDestination(): %s -> %s",
            ussdDestinationToString(mUssdDestination), ussdDestinationToString(destination));
    if (mUssdDestination == destination) {
        return;
    }
    mUssdDestination = destination;
}

const char *RpSsController::ussdDestinationToString(UssdDestination destination) {
    switch (destination) {
        case USSD_URC_TO_GSM:
            return "USSD_URC_TO_GSM";
        case USSD_URC_TO_IMS:
            return "USSD_URC_TO_IMS";
        default:
            // not possible here!
            return NULL;
    }
}

void RpSsController::sendFailureReport(const sp<RfxMessage>& message) {
    // Return SUCCESS first
    RFX_LOG_D(RFX_LOG_TAG, "sendFailureReport, Return SUCCESS first by response");
    sp<RfxMessage> responseMsg = RfxMessage::obtainResponse(RIL_E_SUCCESS,
                message, false);
    responseToRilj(responseMsg);

    // Let the UI have time to show up the "USSD code running" dialog
    usleep(300);

    // And then report the FAILIRUE by URC
    RFX_LOG_D(RFX_LOG_TAG, "sendFailureReport, Report the FAILIRUE by URC");
    // Generate a generic failure USSD URC
    Parcel *parcel = new Parcel();
    parcel->writeString16((String16) "4");
    parcel->writeString16((String16) "");
    sp<RfxMessage> urc = NULL;

    if (message->getId() == RIL_REQUEST_SEND_USSD) {
        urc = RfxMessage::obtainUrc(getSlotId(), RIL_UNSOL_ON_USSD, 0, parcel);
    } else if (message->getId() == RIL_REQUEST_SEND_USSI) {
        urc = RfxMessage::obtainUrc(getSlotId(), RIL_UNSOL_ON_USSI, 0, parcel);
    } else {
        RFX_LOG_E(RFX_LOG_TAG, "sendFailureReport, should not be here!");
    }

    responseToRilj(urc);
}