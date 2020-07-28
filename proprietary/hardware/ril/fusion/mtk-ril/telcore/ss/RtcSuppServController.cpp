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
#include <string>
#include <vector>

#include "RtcSuppServController.h"
#include "RtcSuppServUtil.h"
#include "RfxRilUtils.h"
#include "RtcSuppServQueue.h"
#include "RfxMainThread.h"
#include <mtkconfigutils.h>
#include <libmtkrilutils.h>
#include <unistd.h>

#ifdef RFX_LOG_TAG
#undef RFX_LOG_TAG
#endif
#define RFX_LOG_TAG "RtcSuppServController"
/*****************************************************************************
 * Class RfxController
 *****************************************************************************/

static const char PROPERTY_HASHED_LAST_ICCID[4][MTK_PROPERTY_VALUE_MAX] = {
    "persist.vendor.radio.ss.hashed_last_iccid1",
    "persist.vendor.radio.ss.hashed_last_iccid2",
    "persist.vendor.radio.ss.hashed_last_iccid3",
    "persist.vendor.radio.ss.hashed_last_iccid4",
};

RFX_IMPLEMENT_CLASS("RtcSuppServController", RtcSuppServController, RfxController);

RtcSuppServController::RtcSuppServController() {
    mQueue = RtcSuppServQueue::getInstance();

    // We send USSD URC to GSM by default
    mUssdDestination = USSD_URC_TO_GSM;
}

RtcSuppServController::~RtcSuppServController() {
    logD(RFX_LOG_TAG, "~RtcSuppServController");
}

void RtcSuppServController::onInit() {
    // Required: invoke super class implementation
    logD(RFX_LOG_TAG, "RtcSuppServController onInit!");
    RfxController::onInit();

    const int request_id_list[] = {
        RFX_MSG_REQUEST_SET_CALL_FORWARD,
        RFX_MSG_REQUEST_QUERY_CALL_FORWARD_STATUS,
        RFX_MSG_REQUEST_GET_CLIR,
        RFX_MSG_REQUEST_SET_CLIR,
        RFX_MSG_REQUEST_QUERY_CALL_WAITING,
        RFX_MSG_REQUEST_SET_CALL_WAITING,
        RFX_MSG_REQUEST_QUERY_CALL_BARRING,
        RFX_MSG_REQUEST_SET_CALL_BARRING,
        RFX_MSG_REQUEST_QUERY_CLIP,
        RFX_MSG_REQUEST_SET_CLIP,
        RFX_MSG_REQUEST_GET_COLR,
        RFX_MSG_REQUEST_SET_COLR,
        RFX_MSG_REQUEST_GET_COLP,
        RFX_MSG_REQUEST_SET_COLP,
        RFX_MSG_REQUEST_QUERY_CALL_FORWARD_IN_TIME_SLOT,
        RFX_MSG_REQUEST_SET_CALL_FORWARD_IN_TIME_SLOT,
        RFX_MSG_REQUEST_SET_SUPP_SVC_NOTIFICATION,
        RFX_MSG_REQUEST_SEND_USSD,
        RFX_MSG_REQUEST_CANCEL_USSD,
        RFX_MSG_REQUEST_SEND_USSI,
        RFX_MSG_REQUEST_CANCEL_USSI,
        RFX_MSG_REQUEST_GET_XCAP_STATUS,
        RFX_MSG_REQUEST_SET_CALL_WAITING_ATCI,
        RFX_MSG_REQUEST_RESET_SUPP_SERV,
        RFX_MSG_REQUEST_SETUP_XCAP_USER_AGENT_STRING,
        RFX_MSG_REQUEST_SET_SS_PROPERTY,
        RFX_MSG_REQUEST_USSD_DOMAIN_INFO_REQ
    };

    const int atci_request_id_list[] = {
        RFX_MSG_REQUEST_OEM_HOOK_ATCI_INTERNAL,
    };

    const int urc_id_list[] = {
        RFX_MSG_UNSOL_ON_USSD,
        RFX_MSG_UNSOL_ON_USSI
    };

    // register request & URC id list
    // NOTE. one id can only be registered by one controller
    registerToHandleRequest(request_id_list, sizeof(request_id_list)/sizeof(const int));
    registerToHandleRequest(atci_request_id_list,
            sizeof(atci_request_id_list)/sizeof(const int), MEDIUM);
    registerToHandleUrc(urc_id_list, sizeof(urc_id_list)/sizeof(const int));

    // Register for the status changed of SIM ICCID. Then RIL SS can internally trigger a
    // request to reset SS setting (e.g. terminal based call waiting)
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_SIM_ICCID,
            RfxStatusChangeCallback(this, &RtcSuppServController::onSimIccidChanged));
}

void RtcSuppServController::onDeinit() {
    logD(RFX_LOG_TAG, "onDeinit");
    mQueue->clear();
    RfxController::onDeinit();
}

bool RtcSuppServController::onHandleRequest(const sp<RfxMessage>& message) {
    int msg_id = message->getId();
    int token = message->getPToken();

    // For some requests, no need to add them into the queue. Directly send it to mdcomm layer
    switch (msg_id) {
        case RFX_MSG_REQUEST_USSD_DOMAIN_INFO_REQ:
            requestToMcl(message);
            return true;
        default:
            break;
    }

    // Check if current project is a data only project by CONFIG_SS_MODE
    FeatureValue featurevalue;
    memset(featurevalue.value, 0, sizeof(featurevalue.value));
    mtkGetFeature(CONFIG_SS_MODE, &featurevalue);
    logD(RFX_LOG_TAG, "Check data only project, CONFIG_SS_MODE = %s", featurevalue.value);
    if (strcmp(featurevalue.value, "1") != 0) {
        sp<RfxMessage> responseMsg = RfxMessage::obtainResponse(RIL_E_REQUEST_NOT_SUPPORTED,
                message, false);
        responseToRilj(responseMsg);
        return true;
    }

    logD(RFX_LOG_TAG, "onHandleRequest - %s", RFX_ID_TO_STR(msg_id));
    switch (msg_id) {
        case RFX_MSG_REQUEST_SEND_USSD:
            if (getStatusManager()->getBoolValue(RFX_STATUS_KEY_USSD_SESSION_ONGOING)) {
            logD(RFX_LOG_TAG, "USSD session is ongoing, rejected.");

                // Return SUCCESS first
                logD(RFX_LOG_TAG, "sendFailureReport, Return SUCCESS first by response");
                sp<RfxMessage> responseMsg = RfxMessage::obtainResponse(RIL_E_SUCCESS,
                            message, false);
                responseToRilj(responseMsg);

                // Let the UI have time to show up the "USSD code running" dialog
                usleep(300);

                // And then report the FAILIRUE by URC
                logD(RFX_LOG_TAG, "sendFailureReport, Report the FAILIRUE by URC");
                // Generate a generic failure USSD URC
                char *genericUssdFail[2] = {(char *) "4", (char *) ""};
                sp<RfxMessage> urc = RfxMessage::obtainUrc(getSlotId(), RFX_MSG_UNSOL_ON_USSD,
                        RfxStringsData(genericUssdFail, 2));
                responseToRilj(urc);
                return true;
            } else {
                logD(RFX_LOG_TAG, "USSD session started, token = %d", token);
                getStatusManager()->setBoolValue(RFX_STATUS_KEY_USSD_SESSION_ONGOING, true);
            }
            break;
        case RFX_MSG_REQUEST_SEND_USSI:
            if (getStatusManager()->getBoolValue(RFX_STATUS_KEY_USSD_SESSION_ONGOING)) {
                logD(RFX_LOG_TAG, "USSI session is ongoing, rejected.");

                // Return SUCCESS first
                logD(RFX_LOG_TAG, "sendFailureReport, Return SUCCESS first by response");
                sp<RfxMessage> responseMsg = RfxMessage::obtainResponse(RIL_E_SUCCESS,
                            message, false);
                responseToRilj(responseMsg);

                // Let the UI have time to show up the "USSD code running" dialog
                usleep(300);

                // And then report the FAILIRUE by URC
                logD(RFX_LOG_TAG, "sendFailureReport, Report the FAILIRUE by URC");
                // Generate a generic failure USSI URC
                char *genericUssdFail[2] = {(char *) "4", (char *) ""};
                sp<RfxMessage> urc = RfxMessage::obtainUrc(getSlotId(), RFX_MSG_UNSOL_ON_USSI,
                        RfxStringsData(genericUssdFail, 2));
                responseToRilj(urc);
                return true;
            } else {
                logD(RFX_LOG_TAG, "USSI session started, token = %d", token);
                getStatusManager()->setBoolValue(RFX_STATUS_KEY_USSD_SESSION_ONGOING, true);
            }
            break;
        case RFX_MSG_REQUEST_CANCEL_USSD:
        case RFX_MSG_REQUEST_CANCEL_USSI:
            if (getStatusManager()->getBoolValue(RFX_STATUS_KEY_USSD_SESSION_ONGOING)) {
                logD(RFX_LOG_TAG, "USSD session canceled.");
                getStatusManager()->setBoolValue(RFX_STATUS_KEY_USSD_SESSION_ONGOING, false);
            }
            break;
        default:
            break;
    }
    handleSSRequest(message);
    return true;
}

bool RtcSuppServController::onHandleUrc(const sp<RfxMessage>& message) {
    int msg_id = message->getId();
    int token = message->getPToken();
    switch(msg_id) {
        case RFX_MSG_UNSOL_ON_USSD:  // fall through
        case RFX_MSG_UNSOL_ON_USSI: {
            char** params = (char**) message->getData()->getData();
            int ussdMode = atoi(params[0]);
            int newId = (getUssdDestination() == USSD_URC_TO_GSM) ? RFX_MSG_UNSOL_ON_USSD
                    : RFX_MSG_UNSOL_ON_USSI;

            // Do not display any MMI dialog when +CUSD:3 and no session ongoing.
            // If there is a session ongoing, +CUSD:3 should be sent to UI and
            // the ression should be reset.
            if (ussdMode == 3 &&
                    !getStatusManager()->getBoolValue(RFX_STATUS_KEY_USSD_SESSION_ONGOING)) {
                return true;
            }

            // Change to comply with AOSP
            if (ussdMode == 2 || ussdMode == 3) {
                strncpy(params[0], "0", strlen(params[0]));
            }

            sp<RfxMessage> urcToRilj = RfxMessage::obtainUrc(m_slot_id, newId, message, true);

            // If USSD mode equals to 1, it means further user action is required.
            // Keep the UssdDestination the same. Otherwise, change mUssdDestination to
            // default value. Because the session come to an end.
            if (ussdMode == 1) {
                setUssdDestination(getUssdDestination());
            } else {
                setUssdDestination(USSD_URC_TO_GSM);
            }

            if (getStatusManager()->getBoolValue(RFX_STATUS_KEY_USSD_SESSION_ONGOING)) {
                logD(RFX_LOG_TAG, "USSD session completed, token = %d", token);
                getStatusManager()->setBoolValue(RFX_STATUS_KEY_USSD_SESSION_ONGOING, false);
            }

            responseToRilj(urcToRilj);
            break;
        }
        default:
            responseToRilj(message);
            break;
    }
    return true;
}

bool RtcSuppServController::onHandleResponse(const sp<RfxMessage>& message) {
    int msg_id = message->getId();
    int token = message->getPToken();
    logD(RFX_LOG_TAG, "onHandleResponse - %s", RFX_ID_TO_STR(msg_id));
    switch (msg_id) {
        case RFX_MSG_REQUEST_SET_CALL_WAITING_ATCI:
            responseToAtci(message);
            return true;
        case RFX_MSG_REQUEST_USSD_DOMAIN_INFO_REQ:
            handleUssdDomainInfoReqResponse(message);
            return true;
        case RFX_MSG_REQUEST_SEND_USSD:
            if(message->getError() != RIL_E_SUCCESS) {
                logD(RFX_LOG_TAG, "USSD session failed, end session, token = %d", token);
                getStatusManager()->setBoolValue(RFX_STATUS_KEY_USSD_SESSION_ONGOING, false);
            }
            break;
        case RFX_MSG_REQUEST_SEND_USSI:
            if(message->getError() != RIL_E_SUCCESS) {
                logD(RFX_LOG_TAG, "USSI session failed, end session, token = %d", token);
                getStatusManager()->setBoolValue(RFX_STATUS_KEY_USSD_SESSION_ONGOING, false);
            }
            break;
        default:
            break;
    }
    handleSSResponse(message);
    return true;
}

bool RtcSuppServController::onCheckIfRejectMessage(const sp<RfxMessage>& message,
        bool isModemPowerOff, int radioState) {
    RFX_UNUSED(message);
    int wfcState = getStatusManager()->getIntValue(RFX_STATUS_KEY_WFC_STATE, -1);
    logD(RFX_LOG_TAG, "onCheckIfRejectMessage isModemPowerOff %d, radioState: %d, wfcState: %d",
            (isModemPowerOff == false) ? 0 : 1, radioState, wfcState);

    /* If WFC is enabled, bypass all SS requests. */
    if (wfcState == 1) {
        return false;
    }

    int msgId = message->getId();
    if (msgId == RFX_MSG_REQUEST_SETUP_XCAP_USER_AGENT_STRING ||
        msgId == RFX_MSG_REQUEST_SET_SS_PROPERTY) {
        return false;
    }

    /* If WFC is not enabled and radio is off. reject the request in request_id_list*/
    if (radioState == (int)RADIO_STATE_UNAVAILABLE ||
            radioState == (int)RADIO_STATE_OFF ||
            isModemPowerOff == true) {
        return true;
    }

    /* If WFC is not enabled and radio is not off. Bypass all SS request */
    return false;
}

bool RtcSuppServController::onHandleAtciRequest(const sp<RfxMessage>& message) {
    int msg_id = message->getId();

    switch (msg_id) {
        case RFX_MSG_REQUEST_OEM_HOOK_ATCI_INTERNAL: {
            const char *data = (const char *)message->getData()->getData();
            logD(RFX_LOG_TAG, "ATCI string = %s", data);
            if (strncmp(data, "AT+CCWA=", strlen("AT+CCWA=")) == 0) {
                return handleAtciCallWaitingRequest(message);
            }
            break;
        }
        default:
            break;
    }
    return false;
}

void RtcSuppServController::handleSSRequest(const sp<RfxMessage>& message) {
    int msg_id = message->getId();

    if (mQueue->getSSLock() == SS_LOCKED) {
        logD(RFX_LOG_TAG, "[%d]%s, a SS request is in process, add it into queue",
                message->getPToken(), RFX_ID_TO_STR(message->getId()));
        mQueue->add(RtcSSEntry(message));
    } else {
        logD(RFX_LOG_TAG, "[%d]%s, No SS request in process, directly execute it",
                message->getPToken(), RFX_ID_TO_STR(message->getId()));
        mQueue->setSSLock(SS_LOCKED);

        switch(msg_id) {
            case RFX_MSG_REQUEST_SEND_USSD:
                setUssdDestination(USSD_URC_TO_GSM);
                requestToMcl(message);
                break;
            case RFX_MSG_REQUEST_SEND_USSI:
                setUssdDestination(USSD_URC_TO_IMS);
                requestToMcl(message);
                break;
            case RFX_MSG_REQUEST_CANCEL_USSD:
                setUssdDestination(USSD_URC_TO_GSM);
                requestToMcl(message);
                break;
            case RFX_MSG_REQUEST_CANCEL_USSI:
                setUssdDestination(USSD_URC_TO_GSM);
                requestToMcl(message);
                break;
            default:
                requestToMcl(message);
                break;
        }
    }
}

void RtcSuppServController::handleSSResponse(const sp<RfxMessage>& message) {
    const RtcSSEntry *msgEntry;

    logD(RFX_LOG_TAG, "[%d]%s, SS request's response", message->getPToken(),
            RFX_ID_TO_STR(message->getId()));
    if (!mQueue->isEmpty()) {
        msgEntry = &(mQueue->itemAt(0));
        logD(RFX_LOG_TAG, "the queue is not empty, pick [%d]%s to process",
                msgEntry->message->getPToken(), RFX_ID_TO_STR(msgEntry->message->getId()));

        requestToMcl(msgEntry->message);
        mQueue->removeFront();
    } else {
        // Release the lock if there is no message in the suspended queue
        logD(RFX_LOG_TAG, "No more SS request, release SS lock");
        mQueue->setSSLock(SS_UNLOCKED);
    }
    responseToRilj(message);
}

bool RtcSuppServController::handleAtciCallWaitingRequest(const sp<RfxMessage>& message) {
    string command = (const char *)message->getData()->getData();
    string subStr = "AT+CCWA=";
    command = command.substr(subStr.length());
    logD(RFX_LOG_TAG, "CCWA data = %s", command.c_str());
    vector<string> parameter = RtcSuppServUtil::split(command, ",");
    if (parameter.size() == 2) {
        parameter[0] = RtcSuppServUtil::trim(parameter[0]);
        parameter[1] = RtcSuppServUtil::trim(parameter[1]);
        if ((parameter[0] != "0" && parameter[0] != "1") ||
                (parameter[1] != "1" && parameter[1] != "512")) {
            return false;
        }
        int msg_data[2];
        msg_data[0] = atoi(parameter[0].c_str());
        msg_data[1] = atoi(parameter[1].c_str());
        logD(RFX_LOG_TAG, "CCWA msg_data[0] = %d, msg_data[1] = %d",
                msg_data[0], msg_data[1]);

        sp<RfxMessage> newMsg = RfxMessage::obtainRequest(
                RFX_MSG_REQUEST_SET_CALL_WAITING_ATCI, RfxIntsData(msg_data, 2), message, false);
        requestToMcl(newMsg);
        return true;
    }
    return false;
}

void RtcSuppServController::responseToAtci(const sp<RfxMessage>& message) {
    String8 responseStr = String8("\r\nOK\r\n");
    logD(RFX_LOG_TAG, "responseToSocket:%s", responseStr.string());
    sp<RfxMessage> newMsg = RfxMessage::obtainResponse(message->getSlotId(),
                                    RFX_MSG_REQUEST_OEM_HOOK_ATCI_INTERNAL,
                                    message->getError(),
                                    RfxStringData((char *)responseStr.string()),
                                    message);
    responseToRilj(newMsg);
}

void RtcSuppServController::onSimIccidChanged(RfxStatusKeyEnum key, RfxVariant old_value,
        RfxVariant value) {
    RFX_UNUSED(key);
    RFX_UNUSED(old_value);
    char hashedCurrentIccid[MTK_PROPERTY_VALUE_MAX] = {0};
    char hashedLastIccid[MTK_PROPERTY_VALUE_MAX] = {0};

    // If the ICCID is not valid (SIM not ready / absent), then just return directly
    if (value.asString8().isEmpty() || strcmp(value.asString8().string(), "N/A") == 0) {
        logD(RFX_LOG_TAG, "onSimIccidChanged: ICCID is not valid, return directly");
        return;
    }

    // Reset USSD session when SIM changed
    if (getStatusManager()->getBoolValue(RFX_STATUS_KEY_USSD_SESSION_ONGOING)) {
        logD(RFX_LOG_TAG, "USSD session reset.");
        getStatusManager()->setBoolValue(RFX_STATUS_KEY_USSD_SESSION_ONGOING, false);
    }

    // Hash current ICCID to avoid sensitive log/property issues
    snprintf(hashedCurrentIccid, sizeof(hashedCurrentIccid), "%s",
            RtcSuppServUtil::sha256(std::string(value.asString8())).c_str());
    rfx_property_get(PROPERTY_HASHED_LAST_ICCID[getSlotId()], hashedLastIccid, "");

    logD(RFX_LOG_TAG, "onSimIccidChanged: hashedLastIccid->%s, hashedCurrentIccid->%s",
            hashedLastIccid, hashedCurrentIccid);

    // Compare the ICCID to know if SIM is changed
    if (strcmp(hashedCurrentIccid, hashedLastIccid) != 0) {
        logD(RFX_LOG_TAG, "onSimIccidChanged: trigger a RESET_SUPP_SERV RIL request");
        sp<RfxMessage> newMsg = RfxMessage::obtainRequest(getSlotId(),
                RFX_MSG_REQUEST_RESET_SUPP_SERV, RfxVoidData());
        requestToMcl(newMsg);

        // Update hashed ICCID
        rfx_property_set(PROPERTY_HASHED_LAST_ICCID[getSlotId()], hashedCurrentIccid);
    } else {
        logD(RFX_LOG_TAG, "onSimIccidChanged: do nothing");
    }
}

void RtcSuppServController::handleUssdDomainInfoReqResponse(const sp<RfxMessage>& message) {
    int domain = ((int *) message->getData()->getData())[0];
    logD(RFX_LOG_TAG, "handleUssdDomainInfoReqResponse, domain = %s", (domain==0) ? "CS" : "IMS");

    sp<RfxMessage> newMsg = RfxMessage::obtainRequest(m_slot_id,
            RFX_MSG_REQUEST_USSD_DOMAIN_INFO_ACK, RfxIntsData(&domain, 1));
    RfxMainThread::enqueueMessage(newMsg);
}

UssdDestination RtcSuppServController::getUssdDestination() {
    logD(RFX_LOG_TAG, "getUssdDestination(): mUssdDestination = %s",
            ussdDestinationToString(mUssdDestination));
    return mUssdDestination;
}

void RtcSuppServController::setUssdDestination(UssdDestination destination) {
    logD(RFX_LOG_TAG, "setUssdDestination(): %s -> %s", ussdDestinationToString(mUssdDestination),
            ussdDestinationToString(destination));
    if (mUssdDestination == destination) {
        return;
    }
    mUssdDestination = destination;
}

const char *RtcSuppServController::ussdDestinationToString(UssdDestination destination) {
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
