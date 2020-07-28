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

#include "RmcSuppServUssdBaseHandler.h"
#include "RfxBaseHandler.h"
#include "RfxIntsData.h"
#include "RfxVoidData.h"
#include "RfxStringsData.h"


RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxIntsData, RFX_MSG_REQUEST_USSD_DOMAIN_INFO_REQ);

RFX_REGISTER_DATA_TO_EVENT_ID(RfxStringsData, RFX_MSG_EVENT_UNSOL_ON_USSD);
RFX_REGISTER_DATA_TO_EVENT_ID(RfxStringsData, RFX_MSG_EVENT_UNSOL_ON_USSI);

static const int requests[] = {
    RFX_MSG_REQUEST_USSD_DOMAIN_INFO_REQ
};

static const int event[] = {
    RFX_MSG_EVENT_UNSOL_ON_USSD,
    RFX_MSG_EVENT_UNSOL_ON_USSI
};

RmcSuppServUssdBaseHandler::RmcSuppServUssdBaseHandler(int slot_id,
        int channel_id):RfxBaseHandler(slot_id, channel_id){
    registerToHandleRequest(requests, sizeof(requests)/sizeof(int));
    registerToHandleEvent(event, sizeof(event)/sizeof(int));
}

RmcSuppServUssdBaseHandler::~RmcSuppServUssdBaseHandler() {
    // do nothing
}

void RmcSuppServUssdBaseHandler::onHandleRequest(const sp<RfxMclMessage>& msg) {
    int id = msg->getId();
    switch(id) {
        case RFX_MSG_REQUEST_USSD_DOMAIN_INFO_REQ:
            handleUssdDomainInfoReq(msg);
            break;
        default:
            logE(TAG, "should not be here");
            break;
    }
}

void RmcSuppServUssdBaseHandler::onHandleEvent(const sp<RfxMclMessage>& msg) {
    int id = msg->getId();
    switch(id) {
        case RFX_MSG_EVENT_UNSOL_ON_USSD:
            handleOnUssd(msg);
            break;
        case RFX_MSG_EVENT_UNSOL_ON_USSI:
            handleOnUssi(msg);
            break;
        default:
            logE(TAG, "should not be here");
            break;
    }
}

/**
 * SS module provide an API to let other module (e.g. STK) knows which domain of STK USSD should use
 */
void RmcSuppServUssdBaseHandler::handleUssdDomainInfoReq(const sp<RfxMclMessage>& msg) {
    sp<RfxMclMessage> response;
    int wfcState = getMclStatusManager()->getIntValue(RFX_STATUS_KEY_WFC_STATE, -1);
    int domain = 0;  // 0: CS domain
                     // 1: IMS domain

    if (isImsRegOn()) {
        if (wfcState == 1) {
            domain = 1;
        } else {
            if (!isVopsOn()) {
                domain = 0;
            } else {
                domain = 1;
            }
        }
    }

    logD(TAG, "handleUssdDomainInfoReq, domain = %s", (domain == 0) ? "CS" : "IMS");

    response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxIntsData(&domain, 1), msg, false);
    responseToTelCore(response);
}

/**
 * Some uppper USSI Framework (ex. Q version of AOSP) only accept SUCCES in request phase.
 * To adapt to this design in this case, we return SUCCESS first and then generate an URC
 * to report the FAILIRUE if any error occurs during request phase.
 */
void RmcSuppServUssdBaseHandler::sendFailureReport(const sp<RfxMclMessage>& msg, RIL_Errno ret) {
    sp<RfxMclMessage> response;
    sp<RfxMclMessage> urc;
    char *genericUssdFail[2] = {(char *) "4", (char *) ""};  // Generate a generic failure USSD URC

    logD(TAG, "sendFailureReport, ret = %d", ret);

    // For FDN check failure, we "directly" return FDN error cause when the request comes from
    // GSM pipe (ex. AOSP's GsmMmiCode). Because if USSD request is sent from GSM pipe, it can
    // accept error during request phase. And we should report it to let UI shows correspondnig
    // error meesage
    if (msg->getId() == RFX_MSG_REQUEST_SEND_USSD && ret == RIL_E_FDN_CHECK_FAILURE) {
        // Let the UI have time to show up the "USSD code running" dialog
        sleep(1);

        response = RfxMclMessage::obtainResponse(msg->getId(), ret, RfxVoidData(), msg, false);
        responseToTelCore(response);
        return;
    }

    // Return SUCCESS first
    logD(TAG, "sendFailureReport, Return SUCCESS first by response");
    response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS, RfxVoidData(),
            msg, false);
    responseToTelCore(response);

    // Let the UI have time to show up the "USSD code running" dialog
    sleep(1);

    // And then report the FAILIRUE by URC
    logD(TAG, "sendFailureReport, Report the FAILIRUE by URC");
    urc = RfxMclMessage::obtainUrc(RFX_MSG_UNSOL_ON_USSD, m_slot_id,
            RfxStringsData(genericUssdFail, 2));
    responseToTelCore(urc);
}

/**
 * The API is used for FDN check and emergnecy number approved.
 */
bool RmcSuppServUssdBaseHandler::isFdnAllowed(const char* ussi) {
    sp<RfxAtResponse> p_response;
    int err;
    char* cmd = NULL;
    RfxAtLine *line;
    RIL_Errno ret = RIL_E_INTERNAL_ERR;
    int responses[2] = {0};

    if (ussi == NULL || strlen(ussi) == 0) {
        logE(TAG, "isFdnAllowed: Null parameters.");
        goto error;
    }

    /**
     * AT+EAPPROVE=<dial_number>
     * <dial_number>: string, dialing number.
     */
    asprintf(&cmd, "AT+EAPPROVE=\"%s\"", ussi);
    p_response = atSendCommandSingleline(cmd, "+EAPPROVE:");
    free(cmd);

    err = p_response->getError();
    if (err < 0 || p_response == NULL) {
        logE(TAG, "isFdnAllowed Fail");
        goto error;
    }

    switch (p_response->atGetCmeError()) {
        case CME_SUCCESS:
            break;
        default:     // AT CMD format error, should not be here
            goto error;
    }

    if (p_response->getIntermediates() != NULL) {
        line = p_response->getIntermediates();
        line->atTokStart(&err);
        if (err < 0) {
            goto error;
        }

        /**
         * <is_allowed> : integer
         * 0   The number is not allowed
         * 1   The number is allowed
         */
        responses[0] = line->atTokNextint(&err);
        if (err < 0) {
            goto error;
        }

        /**
         * <is_emergency> : integer
         * 0   The number is not emergnecy number
         * 1   The number is emergency number
         */
        responses[1] = line->atTokNextint(&err);
        if (err < 0) {
            goto error;
        }
    }

    ret = RIL_E_SUCCESS;

error:
    if (ret == RIL_E_SUCCESS) {
        if (responses[1] == 0) {       // not an ECC number, determined by MD PHB
            return (responses[0] == 1);
        } else if (responses[1] == 1){ // Approve if it is an ECC number
            return true;
        }
    }
    return true;                       // Approve if we get any kind of CME error
}

/**
 * The API is used for IMS VoPS check.
 */
bool RmcSuppServUssdBaseHandler::isVopsOn() {
    sp<RfxAtResponse> p_response;
    int err;
    RfxAtLine *line;
    RIL_Errno ret = RIL_E_INTERNAL_ERR;
    int responses[2] = {0};

    /**
     * Query IMS network reporting
     * AT+CIREP?
     * +CIREP: <reporting>,<nwimsvops>
     */
    p_response = atSendCommandSingleline("AT+CIREP?", "+CIREP:");

    err = p_response->getError();
    if (err < 0 || p_response == NULL) {
        logE(TAG, "isVopsOn Fail");
        goto error;
    }

    switch (p_response->atGetCmeError()) {
        case CME_SUCCESS:
            break;
        default:     // AT CMD format error, should not be here
            goto error;
    }

    if (p_response->getIntermediates() != NULL) {
        line = p_response->getIntermediates();
        line->atTokStart(&err);
        if (err < 0) {
            goto error;
        }

        /**
         * <reporting> : integer
         * Enables or disables reporting of changes in the IMSVOPS supported
         * indication received from the network and reporting of PS to CS SRVCC,
         * PS to CS vSRVCC and CS to PS SRVCC handover information.
         *
         * 0   Disable reporting
         * 1   Enable reporting
         */
        responses[0] = line->atTokNextint(&err);
        if (err < 0) {
            goto error;
        }

        /**
         * <nwimsvops> : integer
         * Gives the last IMS Voice Over PS sessions (IMSVOPS) supported
         * indication received from network.
         *
         * 0   IMSVOPS support indication is not received from network, or is negative
         * 1   IMSVOPS support indication as received from network is positive
         */
        responses[1] = line->atTokNextint(&err);
        if (err < 0) {
            goto error;
        }
    }

    ret = RIL_E_SUCCESS;

error:
    if (ret == RIL_E_SUCCESS) {
        return (responses[1] == 1);
    }
    return true;    // Assume VoPS is on by default
}

/**
 * The API is used for IMS registration status check.
 */
bool RmcSuppServUssdBaseHandler::isImsRegOn() {
    sp<RfxAtResponse> p_response;
    RfxAtLine *line;
    RIL_Errno ret = RIL_E_INTERNAL_ERR;
    int err;
    int skip;
    int response[2] = {0};


    /**
     * Query IMS registration information
     * AT+CIREG?
     * +CIREG: <n>,<reg_info>[,<ext_info>]
     */
    p_response = atSendCommandSingleline("AT+CIREG?", "+CIREG:");

    err = p_response->getError();
    if (err != 0 ||
            p_response == NULL ||
            p_response->getSuccess() == 0 ||
            p_response->getIntermediates() == NULL) {
        logE(TAG, "isImsRegOn reg_info Fail");
        goto error;
    }

    // handle intermediate
    line = p_response->getIntermediates();

    // go to start position
    line->atTokStart(&err);
    if (err < 0) {
        goto error;
    }

    // go to start position
    line->atTokStart(&err);
    if (err < 0) goto error;

    // <mode>
    skip = line->atTokNextint(&err);
    if (err < 0 || skip < 0 ) {
        logE(TAG, "The <mode> is an invalid value!!!");
        goto error;
    } else {
        /**
         * <reg_info> : integer
         * Indicates the IMS registration status. The UE is seen as registered as long as
         * one or more of its public user identities are registered with any of its
         * contact addresses
         *
         * 0   not registered
         * 1   registered
         */
        response[0] = line->atTokNextint(&err);
        if (err < 0 ) goto error;

        response[1] = 1; // RADIO_TECH_3GPP
    }

    return (response[0] == 1);

error:
    logE(TAG, "There is something wrong with the AT+CIREG?, return false for isImsRegOn()");
    return false;   // Assume no IMS registration by default
}

bool RmcSuppServUssdBaseHandler::isInImsCall() {
    bool r = getMclStatusManager()->getBoolValue(RFX_STATUS_KEY_IMS_CALL_EXIST, false);
    logD(TAG, "isInImsCall: r = %s", (r ? "True" : "False"));
    return r;
}
