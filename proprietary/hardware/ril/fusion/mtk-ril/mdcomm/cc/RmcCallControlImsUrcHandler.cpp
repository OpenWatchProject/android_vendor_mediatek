/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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
// Std include
#include <vector>
#include <string>

// MTK Fusion Fwk include
#include "RfxStringsData.h"
#include "RfxIntsData.h"
#include "RfxVoidData.h"

// CC Local include
#include "RmcCallControlImsUrcHandler.h"
#include "RmcCallControlInterface.h"

#define RFX_LOG_TAG "RmcImsCCUrcHandler"

#define SRVCC_START 0
#define SRVCC_COMPLETED 1
#define CONFERENCE_MEMBERS_LIMIT 5  // not include the host
#define MODE_ENRICHED_CALLING 3
// register handler to channel
RFX_IMPLEMENT_HANDLER_CLASS(RmcCallControlImsUrcHandler, RIL_CMD_PROXY_URC);

static const char* urc[] = {
    URC_CONF_SRVCC,
    URC_CONF_MODIFIED_RESULT,
    URC_VDM_SELECT_RESULT,
    URC_IMS_CALL_MODE,
    URC_IMS_VIDEO_CAP,
    URC_ECPLICIT_CALL_TRANSFER,
    URC_IMS_CALL_CONTROL_RESULT,
    URC_IMS_EVENT_PACKAGE,
    URC_SRVCC,
    URC_ECC_BEARER,
    URC_REDIAL_ECC_INDICATION,
    URC_IMS_HEADER,
    URC_CALL_ADDITIONAL_INFO,
};

RmcCallControlImsUrcHandler::RmcCallControlImsUrcHandler(
    int slot_id, int channel_id) : RmcCallControlBaseHandler(slot_id, channel_id) {

    registerToHandleURC(urc, sizeof(urc) / sizeof(char *));
}

RmcCallControlImsUrcHandler::~RmcCallControlImsUrcHandler() {
}

void RmcCallControlImsUrcHandler::onHandleUrc(const sp<RfxMclMessage>& msg) {
    char *urc = msg->getRawUrc()->getLine();
    // logD(RFX_LOG_TAG, "[onHandleUrc]%s", urc);
    if(strStartsWith(urc, URC_CONF_SRVCC)) {
        handleConfSrvcc(msg);
    } else if (strStartsWith(urc, URC_CONF_MODIFIED_RESULT)) {
        handleConfModifiedResult(msg);
    } else if (strStartsWith(urc, URC_VDM_SELECT_RESULT)) {
        handleVoiceDomainSelectResult(msg);
    } else if (strStartsWith(urc, URC_IMS_CALL_MODE)) {
        handleCallModeChanged(msg);
    } else if (strStartsWith(urc, URC_IMS_VIDEO_CAP)) {
        handleVideoCapabilityChanged(msg);
    } else if (strStartsWith(urc, URC_ECPLICIT_CALL_TRANSFER)) {
        handleECT(msg);
    } else if (strStartsWith(urc, URC_IMS_CALL_CONTROL_RESULT)) {
        handleImsCallControlResult(msg);
    } else if (strStartsWith(urc, URC_IMS_EVENT_PACKAGE)) {
        handleEventPackage(msg);
    } else if (strStartsWith(urc, URC_SRVCC)) {
        handleSrvccStateChange(msg);
    } else if (strStartsWith(urc, URC_ECC_BEARER)) {
        handleEmergencyBearerSupportInfo(msg);
    } else if (strStartsWith(urc, URC_REDIAL_ECC_INDICATION)) {
        handleRedialEmergencyIndication(msg);
    } else if (strStartsWith(urc, URC_IMS_HEADER)) {
        handleImsHeaderInfo(msg);
    } else if (strStartsWith(urc, URC_CALL_ADDITIONAL_INFO)) {
        handleCallAdditionalInfo(msg);
    }
}

void RmcCallControlImsUrcHandler::onHandleTimer() {
    // do something
}

void RmcCallControlImsUrcHandler::handleConfSrvcc(const sp<RfxMclMessage>& msg) {
    /*
    * +ECONFSRVCC: <number_of_calls>, <call_id>, ...
    * <number_of_calls>: number of calls, the limitation is 5 (not include the host)
    * <call_id>...: 0 = incoming call; 1~32 = call id
    */
    const int maxDataLen = 6;
    int err;
    RfxAtLine *line = msg->getRawUrc();

    line->atTokStart(&err);
    if (err < 0) return;

    int index = 0;
    int data[maxDataLen] = {0, 0, 0, 0, 0, 0};
    while(line->atTokHasmore() && index < maxDataLen) {
        data[index] = line->atTokNextint(&err);
        if (err < 0) break;
        ++index;
    }

    if (data[0] > CONFERENCE_MEMBERS_LIMIT) {
        logE(RFX_LOG_TAG, "The count of calls to SRVCC is greater than limitation");
        return;
    }

    sp<RfxMclMessage> urc = RfxMclMessage::obtainUrc(
        RFX_MSG_UNSOL_ECONF_SRVCC_INDICATION, m_slot_id, RfxIntsData(data, maxDataLen));
    responseToTelCore(urc);
}

void RmcCallControlImsUrcHandler::handleConfModifiedResult(const sp<RfxMclMessage>& msg) {
    /* +ECONF=<conf_call_id>,<operation>,<num>,<result>,<cause>,[<joined_call_id>]
     * <conf_call_id>: host id
     * <operation>: 0 = add; 1 = remove
     * <num>: modified call number
     * <result>: 0 = success; 1 = failed
     * <cause>: failed cause
     * [<joined_call_id>]: optional, modified call id
     */
    const int maxLen = 6;
    int rfxMsg = RFX_MSG_UNSOL_ECONF_RESULT_INDICATION;
    notifyStringsDataToTcl(msg, rfxMsg, maxLen);
}

void RmcCallControlImsUrcHandler::handleImsCallControlResult(const sp<RfxMclMessage>& msg) {
    /* +ECCTRL: <call_id>, <cmd>, <result>, [<failed cause>]
     * call_id: ignore
     * cmd: 131 (hold), 132 (resume)
     * result: 0 (success), 1 (failed)
     */
    const int maxLen = 4;
    int rfxMsg = RFX_MSG_UNSOL_IMS_ASYNC_CALL_CONTROL_RESULT;
    notifyIntsDataToTcl(msg, rfxMsg, maxLen);
}

void RmcCallControlImsUrcHandler::handleVoiceDomainSelectResult(const sp<RfxMclMessage>& msg) {
    /*
    * +EVADSREP: <domain>, [<call rat>]
    * <domain>: 0 = CS; 1 = IMS
    * <call rat>: The rat of IMS call
    * 1: LTE
    * 2: WiFi
    * 3: NR
    */
    if (!hasImsCall(msg->getSlotId())) {
        logD(RFX_LOG_TAG, "No ImsCall, ignore +EVADSREP");
        return;
    }

    int err;
    RfxAtLine *line = msg->getRawUrc();

    line->atTokStart(&err);
    if (err < 0) return;

    int voiceDomain = line->atTokNextint(&err);
    if (err < 0) return;

    logD(RFX_LOG_TAG, "EVADSREP: %s", (voiceDomain == 0)? (char*)"cs": (char*)"ims");

    int callRat = -1;
    if (line->atTokHasmore()) {
        callRat = line->atTokNextint(&err);
        if(err < 0) return;

        logD(RFX_LOG_TAG, "EVADSREP rat: %d", callRat);
    }
    int data[2];
    data[0] = voiceDomain;
    data[1] = callRat;
    sp<RfxMclMessage> urc = RfxMclMessage::obtainUrc(RFX_MSG_URC_CALL_RAT_INDICATION,
            m_slot_id, RfxIntsData(data, 2));
    responseToTelCore(urc);

    if (voiceDomain != 0) return;

    notifySrvccState(SRVCC_START);
    notifySrvccState(SRVCC_COMPLETED);
}

void RmcCallControlImsUrcHandler::handleCallModeChanged(const sp<RfxMclMessage>& msg) {
    /*
    * +EIMSCMODE: <call id>,<call_mode>,<camera_direction>,<audio_direction>,<PAU>,<audio_codec>
    * <call_id>: 0-incoming call, 1~32-call id
    * <call_mode>: call mode, should between 20 ~ 25
    * <video_state>: video state (send / recv)
    * <audio_direction>: audio state (send / recv)
    * <PAU>: P-Asserted-Identity
    * <audio_codec>: codec
    */
    const int maxLen = 6;
    int rfxMsg = RFX_MSG_UNSOL_CALLMOD_CHANGE_INDICATOR;
    notifyStringsDataToTcl(msg, rfxMsg, maxLen);
}


void RmcCallControlImsUrcHandler::handleECT(const sp<RfxMclMessage>& msg) {
    /**
     *  Send the result of "AT+CHLD=4" to framework.
     *  +ECT: <call_id>, <ect_result>, <cause>
     */
    const int maxLen = 3;
    int rfxMsg = RFX_MSG_UNSOL_ECT_INDICATION;
    bool appendPhoneId = true;
    notifyIntsDataToTcl(msg, rfxMsg, maxLen, appendPhoneId);
}


void RmcCallControlImsUrcHandler::handleVideoCapabilityChanged(const sp<RfxMclMessage>& msg) {
    /*
    * +EIMSVCAP: <call ID>, <local video capability>, <remote video capability>
    * <call_id>: 0-incoming call, 1~32-call id
    * <local video capability>: true = has video capability; false = no video capability
    * <remote video capability>: true = has video capabilit; false = no video capability
    */
    const int maxLen = 3;
    int rfxMsg = RFX_MSG_UNSOL_VIDEO_CAPABILITY_INDICATOR;
    notifyStringsDataToTcl(msg, rfxMsg, maxLen);
}

void RmcCallControlImsUrcHandler::handleEventPackage(const sp<RfxMclMessage>& msg) {
    /*
     * +EIMSEVTPKG: <call_id>,<type>,<urc_index>,<total_urc_count>,<data>
     * <call_id>:  0~255
     * <type>: 1 = Conference Event Package; 2 = Dialog Event Package
     * <urc_index>: 1~255, the index of URC part
     * <total_urc_count>: 1~255
     * <data>: xml raw data, max length = 1950
     */
    const int maxLen = 5;
    int rfxMsg = RFX_MSG_URC_IMS_EVENT_PACKAGE_INDICATION;
    bool appendPhoneId = true;
    notifyStringsDataToTcl(msg, rfxMsg, maxLen, appendPhoneId);
}

void RmcCallControlImsUrcHandler::handleSrvccStateChange(const sp<RfxMclMessage>& msg) {
    int err;
    RfxAtLine *line = msg->getRawUrc();

    line->atTokStart(&err);
    if (err < 0) return;

    int state = line->atTokNextint(&err);
    if (err < 0) return;

    notifySrvccState(state);
}

void RmcCallControlImsUrcHandler::notifySrvccState(int state) {
    logE(RFX_LOG_TAG, "Notify SRVCC state: %d", state);
    int data[1];
    data[0] = state;
    sp<RfxMclMessage> urc =
        RfxMclMessage::obtainUrc(RFX_MSG_UNSOL_SRVCC_STATE_NOTIFY, m_slot_id, RfxIntsData(data, 1));
    responseToTelCore(urc);

    // Trigger GsmFwk poll call after SRVCC
    if (state == SRVCC_COMPLETED) {
        sp<RfxMclMessage> callStateChangeUrc = RfxMclMessage::obtainUrc(
            RFX_MSG_UNSOL_RESPONSE_CALL_STATE_CHANGED, m_slot_id, RfxVoidData());
        responseToTelCore(callStateChangeUrc);
    }
}

/**
 * Notify the indicator of emergency bearer services for S1 mode.
 * +CNEMS1: <s1_support>
 *    s1_support=0 : Emergency bearer services for S1 mode not support
 *    s1_support=1 : Emergency bearer services for S1 mode support
 */
void RmcCallControlImsUrcHandler::handleEmergencyBearerSupportInfo(const sp<RfxMclMessage>& msg) {
    logD(RFX_LOG_TAG, "onEmergencyBearerSupportInfo");
    const int maxLen = 1;
    int rfxMsg = RFX_MSG_URC_EMERGENCY_BEARER_SUPPORT_NOTIFY;
    // int rfxMsg = RFX_MSG_UNSOL_IMS_SUPPORT_ECC;
    // get <s1_support> and send Urc
    notifyIntsDataToTcl(msg, rfxMsg, maxLen);
}

/**
 * Notify the indicator of redial as emergency call
 * +ERDECCIND: <call_id>
 */
void RmcCallControlImsUrcHandler::handleRedialEmergencyIndication(const sp<RfxMclMessage>& msg) {
    logD(RFX_LOG_TAG, "onRedialEmergencyIndication");
    const int maxLen = 1;
    int rfxMsg = RFX_MSG_URC_REDIAL_EMERGENCY_INDICATION;
    notifyStringsDataToTcl(msg, rfxMsg, maxLen);
}

void RmcCallControlImsUrcHandler::handleImsHeaderInfo(const sp<RfxMclMessage>& msg) {
    /**
     *  Send the result of AT+EIMSHEADER=<call_id>,<header_type> to framework.
     *  +EIMSHEADER:<call_id>,<header_type>,<total_count>,<index>,"<value>"
     */
    logD(RFX_LOG_TAG, "handleImsHeaderInfo");
    const int maxLen = 5;
    int rfxMsg = RFX_MSG_URC_SIP_HEADER;
    notifyStringsDataToTcl(msg, rfxMsg, maxLen);
}

/**
 *  +EIMSCAI: <call_id>, <mode>, <type>, <total>, <index>, <count>, [additional Info]
 */
void RmcCallControlImsUrcHandler::handleCallAdditionalInfo(const sp<RfxMclMessage>& msg) {
    logD(RFX_LOG_TAG, "handleCallAdditionalInfo");
    const int maxLen = 5;
    int rfxMsg = RFX_MSG_UNSOL_CALL_ADDITIONAL_INFO;

    RfxAtLine *urc = msg->getRawUrc();
    if (urc == NULL) return;
    // token init
    int ret;
    urc->atTokStart(&ret);
    if (ret < 0) return;

    std::vector<char*> data;

    char* callId = urc->atTokNextstr(&ret);
    if (ret < 0) return;

    int mode = urc->atTokNextint(&ret);
    if (ret < 0) return;

    char intdata[5];
    if (mode == MODE_ENRICHED_CALLING) {
        sprintf(intdata, "%d", MT_CALL_ENRICHED_CALLING);
        data.push_back(intdata);
    } else {
        return;
    }

    data.push_back(callId);

    // start to access token
    int index = 0;
    while (maxLen == 0 || index < maxLen) {
        char* str = urc->atTokNextstr(&ret);
        if (ret < 0) break;
        if (str == NULL) {
            str = (char*)"";  // assign an empty string
        }
        data.push_back(str);
        ++index;
    }

    // token more than expected
    if (urc->atTokHasmore()) {
        logD(RFX_LOG_TAG, "Wrong URC format: params more than limit");
    }

    sp<RfxMclMessage> unsol = RfxMclMessage::obtainUrc(rfxMsg, m_slot_id,
            RfxStringsData((char**)data.data(), (int)data.size()));
    responseToTelCore(unsol);
}
