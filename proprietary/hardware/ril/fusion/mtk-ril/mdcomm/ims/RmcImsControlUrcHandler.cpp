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

#include "RmcImsControlUrcHandler.h"
#include "RfxSipRegInfoData.h"
#include "RfxEiregData.h"
#include <telephony/mtk_ril.h>
#include <vector>

#include "rfx_properties.h"

#define PROPERTY_IMS_EXTINFO     "ril.ims.extinfo"

// register handler to channel
RFX_IMPLEMENT_HANDLER_CLASS(RmcImsControlUrcHandler, RIL_CMD_PROXY_URC);

// register data
RFX_REGISTER_DATA_TO_URC_ID(RfxIntsData, RFX_MSG_UNSOL_IMS_DISABLE_START);
RFX_REGISTER_DATA_TO_URC_ID(RfxIntsData, RFX_MSG_UNSOL_IMS_ENABLE_START);
RFX_REGISTER_DATA_TO_URC_ID(RfxIntsData, RFX_MSG_UNSOL_IMS_DISABLE_DONE);
RFX_REGISTER_DATA_TO_URC_ID(RfxIntsData, RFX_MSG_UNSOL_IMS_ENABLE_DONE);
RFX_REGISTER_DATA_TO_URC_ID(RfxIntsData, RFX_MSG_UNSOL_IMS_REGISTRATION_INFO);
RFX_REGISTER_DATA_TO_URC_ID(RfxVoidData, RFX_MSG_UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED);
RFX_REGISTER_DATA_TO_URC_ID(RfxIntsData, RFX_MSG_UNSOL_IMS_DEREG_DONE);
RFX_REGISTER_DATA_TO_URC_ID(RfxStringsData, RFX_MSG_UNSOL_SIP_CALL_PROGRESS_INDICATOR);
RFX_REGISTER_DATA_TO_URC_ID(RfxIntsData, RFX_MSG_UNSOL_IMS_SUPPORT_ECC);
RFX_REGISTER_DATA_TO_URC_ID(RfxStringsData, RFX_MSG_UNSOL_IMS_RTP_INFO);
RFX_REGISTER_DATA_TO_URC_ID(RfxIntsData, RFX_MSG_UNSOL_IMS_MULTIIMS_COUNT);
RFX_REGISTER_DATA_TO_URC_ID(RfxIntsData, RFX_MSG_UNSOL_VOPS_INDICATION);
RFX_REGISTER_DATA_TO_URC_ID(RfxSipRegInfoData, RFX_MSG_UNSOL_SIP_REG_INFO);
RFX_REGISTER_DATA_TO_URC_ID(RfxImsRegInfoData, RFX_MSG_UNSOL_IMS_REGISTRATION_STATE_IND);
RFX_REGISTER_DATA_TO_URC_ID(RfxEiregData, RFX_MSG_UNSOL_EIREG_INFO_IND);

RmcImsControlUrcHandler::RmcImsControlUrcHandler(int slot_id, int channel_id) :
        RfxBaseHandler(slot_id, channel_id) {
    logD(RFX_LOG_TAG, "RmcImsControlUrcHandler constructor");

    mPreferCireg = false;
    int m_slot_id = slot_id;
    int m_channel_id = channel_id;
    const char* urc[] = {
        "+EIMS:",
        "+EIMCFLAG:",
        "+CIREGU:",
        "+EIMSDEREG:",
        "+ESIPCPI",
        // "+EIMSESS",
        "+EIMSRTPRPT",
        "+EIREG",
        "+CIREPI",
        "+ESIPREGINFO",
        "+EIMSREGURI",
        "+EIMSREGRESP",
    };

    registerToHandleURC(urc, sizeof(urc)/sizeof(char *));
}

RmcImsControlUrcHandler::~RmcImsControlUrcHandler() {
}

void RmcImsControlUrcHandler::onHandleUrc(const sp<RfxMclMessage>& msg) {
    if (!strStartsWith(msg->getRawUrc()->getLine(), "+EIMSREGURI")) {
        logD(RFX_LOG_TAG, "onHandleUrc: %s", msg->getRawUrc()->getLine());
    } else {
        logI(RFX_LOG_TAG, "onHandleUrc: got an unsolicited message");
    }
    if (strStartsWith(msg->getRawUrc()->getLine(), "+EIMS: 0")) {
        handleImsDisabling(msg);
    } else if (strStartsWith(msg->getRawUrc()->getLine(), "+EIMS: 1")) {
        handleImsEnabling(msg);
    } else if (strStartsWith(msg->getRawUrc()->getLine(), "+EIMCFLAG: 0")) {
        handleImsDisabled(msg);
    } else if (strStartsWith(msg->getRawUrc()->getLine(), "+EIMCFLAG: 1")) {
        handleImsEnabled(msg);
    } else if (strStartsWith(msg->getRawUrc()->getLine(), "+CIREGU")) {
        handleImsRegistrationInfo(msg);
    } else if (strStartsWith(msg->getRawUrc()->getLine(), "+EIMSDEREG")) {
        handleImsDereg(msg);
    } else if (strStartsWith(msg->getRawUrc()->getLine(), "+ESIPCPI")) {
        handleSipMsgIndication(msg);
    } else if (strStartsWith(msg->getRawUrc()->getLine(), "+EIMSESS")) {
        handleImsEccSupportInfo(msg);
    } else if (strStartsWith(msg->getRawUrc()->getLine(), "+EIMSRTPRPT")) {
        handleImsRtpInfo(msg);
    } else if (strStartsWith(msg->getRawUrc()->getLine(), "+EIREG")) {
        handleImsEiregInfo(msg);
    } else if (strStartsWith(msg->getRawUrc()->getLine(), "+CIREPI")) {
        onImsVopsIndication(msg);
    } else if (strStartsWith(msg->getRawUrc()->getLine(), "+ESIPREGINFO")) {
        handleSipRegInfoInd(msg);
    } else if (strStartsWith(msg->getRawUrc()->getLine(), "+EIMSREGURI")) {
        handleImsRegInfoInd(msg);
    } else if (strStartsWith(msg->getRawUrc()->getLine(), "+EIMSREGRESP")) {
        handleImsRegRespInd(msg);
    }
}

void RmcImsControlUrcHandler::handleImsDisabling(const sp<RfxMclMessage>& msg) {
    RFX_UNUSED(msg);
    int response = m_slot_id;
    sp<RfxMclMessage> urc;

    urc = RfxMclMessage::obtainUrc(RFX_MSG_UNSOL_IMS_DISABLE_START,
            m_slot_id, RfxIntsData(&response, 1));
    responseToTelCore(urc);
}

void RmcImsControlUrcHandler::handleImsEnabling(const sp<RfxMclMessage>& msg) {
    RFX_UNUSED(msg);
    int response = m_slot_id;
    sp<RfxMclMessage> urc;

    urc = RfxMclMessage::obtainUrc(RFX_MSG_UNSOL_IMS_ENABLE_START,
            m_slot_id, RfxIntsData(&response, 1));
    responseToTelCore(urc);
}

void RmcImsControlUrcHandler::handleImsDisabled(const sp<RfxMclMessage>& msg) {
    RFX_UNUSED(msg);
    int response = m_slot_id;
    sp<RfxMclMessage> urc;

    urc = RfxMclMessage::obtainUrc(RFX_MSG_UNSOL_IMS_DISABLE_DONE,
            m_slot_id, RfxIntsData(&response, 1));
    responseToTelCore(urc);
}

void RmcImsControlUrcHandler::handleImsEnabled(const sp<RfxMclMessage>& msg) {
    RFX_UNUSED(msg);
    int response = m_slot_id;
    sp<RfxMclMessage> urc;

    urc = RfxMclMessage::obtainUrc(RFX_MSG_UNSOL_IMS_ENABLE_DONE,
            m_slot_id, RfxIntsData(&response, 1));
    responseToTelCore(urc);
}

void RmcImsControlUrcHandler::handleImsRegistrationInfo(const sp<RfxMclMessage>& msg) {
    int err;
    int response[3] = {0};
    char* tokenStr = NULL;
    RfxAtLine* line = msg->getRawUrc();
    sp<RfxMclMessage> urc;
    sp<RfxMclMessage> urc2;
    int wfcState;
    const int WFC_STATE_ON = 1 << 4;

    // go to start position
    line->atTokStart(&err);
    if (err < 0) goto error;

    // get reg_info
    response[0] = line->atTokNextint(&err);
    if (err < 0) goto error;

    // get ext_info , value range is 1~FFFFFFFF
    tokenStr = line->atTokNextstr(&err); //hex string
    if (err < 0) {
        // report mode is 1 , no ext_info available
        // set invalid value 0 for upper layer to distinguish if ext_info is availble or not
        response[1] = 0;
    } else if (strlen(tokenStr) > 0) {
        response[1] = (int)strtol(tokenStr, NULL, 16);
    }

    // Fix DSDS bug : transfer rid socket information to IMS Service
    // to judgement for valid sim/phone id
    response[2] = m_slot_id;

    // 93MD IMS framework can NOT get ims RAN type, transfer EWFC state
    // to it. Indicate current RAN type IWLAN or not.
    wfcState = getMclStatusManager()->getIntValue(RFX_STATUS_KEY_WFC_STATE, -1);
    logD(RFX_LOG_TAG, "get WFC state : %d", wfcState);
    if (wfcState == 1) {
        // enable VoWifi bit in ext_info if wfc state was 1
        response[1] |= WFC_STATE_ON;
    }

    /// M: ALPS03862134 Add for hVoLTE case.
    getMclStatusManager()->setIntValue(RFX_STATUS_KEY_IMS_REGISTRATION_STATE, response[1]);

    logD(RFX_LOG_TAG, "handleImsRegistrationInfo reg_info = %d, ext_info = %d, m_slot_id = %d",
            response[0], response[1], response[2]);

    // MTK defined UNSOL EVENT (with IMS data info attached)
    urc = RfxMclMessage::obtainUrc(RFX_MSG_UNSOL_IMS_REGISTRATION_INFO,
            m_slot_id, RfxIntsData(response, 3));
    responseToTelCore(urc);
    // for AOSP defined UNSOL EVENT (no data)
    urc2 = RfxMclMessage::obtainUrc(RFX_MSG_UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED,
            m_slot_id, RfxVoidData());
    responseToTelCore(urc2);

    mPreferCireg = true;
    return;

error:
    logE(RFX_LOG_TAG, "There is something wrong with the +CIREGU");
}

void RmcImsControlUrcHandler::handleImsDereg(const sp<RfxMclMessage>& msg) {
    RFX_UNUSED(msg);
    int response = m_slot_id;
    sp<RfxMclMessage> urc;

    urc = RfxMclMessage::obtainUrc(RFX_MSG_UNSOL_IMS_DEREG_DONE,
            m_slot_id, RfxIntsData(&response, 1));
    responseToTelCore(urc);
}

void RmcImsControlUrcHandler::handleImsEccSupportInfo(const sp<RfxMclMessage>& msg) {
    int err, ratType, supportEmc;
    int response[2] = {0};
    char* tokenStr = NULL;
    RfxAtLine* line = msg->getRawUrc();
    sp<RfxMclMessage> urc;

    // go to start position
    line->atTokStart(&err);
    if (err < 0) goto error;

    // get rat type
    ratType = line->atTokNextint(&err);
    if (err < 0) goto error;

    // get emc support value
    supportEmc = line->atTokNextint(&err);
    if (err < 0) goto error;

    logD(RFX_LOG_TAG, "onImsEccUpdated: rat :%d, support_emc : %d", ratType, supportEmc);
    if ((ratType == 3 || ratType == 4) && supportEmc == 1) {
        response[0] = 1;
    } else {
        response[0] = 0;
    }
    response[1] = m_slot_id;
    getMclStatusManager()->setIntValue(RFX_STATUS_KEY_IMS_EMERGENCY_SUPPORT_STATE,
            response[0]);

    urc = RfxMclMessage::obtainUrc(RFX_MSG_UNSOL_IMS_SUPPORT_ECC,
            m_slot_id, RfxIntsData(response, 2));
    responseToTelCore(urc);
    return;

error:
    logE(RFX_LOG_TAG, "There is something wrong with the +EIMSESS");
}

void RmcImsControlUrcHandler::handleSipMsgIndication(const sp<RfxMclMessage>& msg) {
    /*
    * +ESIPCPI: <call_id>,<dir>,<SIP_msg_type>,<method>,<response_code>[,<reason_text>]
    * <call_id>: 0 = incoming call; 1~32 = call id
    * <SIP_msg_type>: 0 = request; 1 = response
    * <method>: 1~32 and mapping to INVITE, PRACK, UPDATE, ACK, CANCEL, BYE, REFER, OK
    * <response_code>: 0-only used when SIP_msg_type is 0(request), else 100~600
    * [<reason_text>]: Optional, The text in the SIP response reason header.
    */
    const int maxLen = 6;
    int rfxMsg = RFX_MSG_UNSOL_SIP_CALL_PROGRESS_INDICATOR;
    bool appendPhoneId = true;
    notifyStringsDataToTcl(msg, rfxMsg, maxLen, appendPhoneId);
}

void RmcImsControlUrcHandler::onHandleTimer() {
    // do something
}

void RmcImsControlUrcHandler::onHandleEvent(const sp<RfxMclMessage>& msg) {
    RFX_UNUSED(msg);
    // handle event
}

void RmcImsControlUrcHandler::handleImsRtpInfo(const sp<RfxMclMessage>& msg) {
    /* +EIMSRTPRPT=<default_ebi>, <network_id>, <timer>, <send pkt lost>, <recv pkt lost>
            , <jitter>, <delay>
     */
    const int maxLen = 7;

    RfxAtLine *line = msg->getRawUrc();
    char* responseStr[maxLen];
    int idx, i, err;
    line->atTokStart(&err);
    if (err < 0) {
        logE(RFX_LOG_TAG, "handleImsRtpInfo: parse : error!");
        return;
    }

    for (idx = 0; idx < maxLen; idx++) {
        responseStr[idx] = line->atTokNextstr(&err);
        if (err < 0) {
            logE(RFX_LOG_TAG, "handleImsRtpInfo: not support idx [%d]",idx);
            break;
        }
    }

    // The maxLen of old version would be 5, so fill the empty string in the char*.
    for (i = idx; i < maxLen; i++) {
        asprintf(&responseStr[i], "");
    }

    sp<RfxMclMessage> urc = RfxMclMessage::obtainUrc(RFX_MSG_UNSOL_IMS_RTP_INFO, m_slot_id,
            RfxStringsData(responseStr, maxLen));

    responseToTelCore(urc);

    for (i = idx; i < maxLen; i++) {
        free(responseStr[i]);
    }
}

void RmcImsControlUrcHandler::handleImsEiregInfo(const sp<RfxMclMessage>& msg) {
    /* +EIREG= <reg_state>,<reg_type>[,<ext_info>,<dereg_cause>,<ims_retry>,<rat>,<sip_uri_type>] */
    int err;
    int info[7] = {0};
    char key[32] = {0};
    char* tokenStr = NULL;
    RfxAtLine* line = msg->getRawUrc();
    sp<RfxMclMessage> urc;
    RIL_EiregData response;

    if (mPreferCireg) {
        mtkLogI(RFX_LOG_TAG, "CIREGU is enabled so we ignore +EIREG");
        return;
    }

    // go to start position
    line->atTokStart(&err);
    if (err < 0) goto error;

    // get reg_state
    response.reg_state = line->atTokNextint(&err);
    if (err < 0) goto error;

    // get reg_type
    response.reg_type = line->atTokNextint(&err);
    if (err < 0) goto error;

    // get ext_info , value range is 1~FFFFFFFF
    response.ext_info = line->atTokNextint(&err);
    if (err < 0) goto error;

    /// M: ALPS03862134 Add for hVoLTE case.
    getMclStatusManager()->setIntValue(RFX_STATUS_KEY_IMS_REGISTRATION_STATE, response.ext_info);

    // get dereg_cause
    response.dereg_cause = line->atTokNextint(&err);
    if (err < 0) goto error;

    // get ims_retry
    response.ims_retry = line->atTokNextint(&err);
    if (err < 0) goto error;

    // get rat
    response.rat = line->atTokNextint(&err);
    if (err < 0) goto error;

    // get sip_uri_type
    response.sip_uri_type = line->atTokNextint(&err);
    if (err < 0) goto error;

    logI(RFX_LOG_TAG, "handleImsEiregInfo:reg_state<%d>reg_type<%d>ext_info<%d>dereg_cause<%d>"
            "ims_retry<%d>rat<%d>sip_uri_type<%d>slot<%d>",
            response.reg_state, response.reg_type, response.ext_info, response.dereg_cause,
            response.ims_retry, response.rat, response.sip_uri_type, m_slot_id);

    urc = RfxMclMessage::obtainUrc(RFX_MSG_UNSOL_EIREG_INFO_IND,
            m_slot_id, RfxEiregData(&response, sizeof(RIL_EiregData)));
    responseToTelCore(urc);

    return;

    error:
        logE(RFX_LOG_TAG, "There is something wrong with the +EIREG");
}

int RmcImsControlUrcHandler::getIntFromTokenStr(char* tokenStr, int err, int base) {
    if (strlen(tokenStr) > 0 && err >= 0) {
        return (int)strtol(tokenStr, NULL, base);
    }
    else {
        // token no info available
        return -1;
    }
}

void RmcImsControlUrcHandler::onImsVopsIndication(const sp<RfxMclMessage>& msg) {
    int rfxMsg = RFX_MSG_UNSOL_VOPS_INDICATION;
    int err;
    RfxAtLine *line = msg->getRawUrc();

    line->atTokStart(&err);
    if (err < 0) return;

    int val = line->atTokNextint(&err);
    if (err < 0) return;
    std::vector<int> params;
    params.push_back(val);

    getMclStatusManager()->setIntValue(RFX_STATUS_KEY_VOPS, params[0]);
    logD(RFX_LOG_TAG, "RFX_STATUS_KEY_VOPS=> %d",
            getMclStatusManager()->getIntValue(RFX_STATUS_KEY_VOPS));

    sp<RfxMclMessage> urc = RfxMclMessage::obtainUrc(
        rfxMsg, m_slot_id, RfxIntsData(params.data(), (int)params.size()));
    responseToTelCore(urc);
}

void RmcImsControlUrcHandler::handleSipRegInfoInd(const sp<RfxMclMessage>& msg) {
    int err;
    char* tokenStr = NULL;
    RfxAtLine* line = msg->getRawUrc();
    sp<RfxMclMessage> urc;
    RIL_SipRegInfo response;

    // +ESIPREGINFO : <account_id>,"<direction>","<SIP_msg_type>","<method>",<response_code>,
    // "<reason_phrase>","<warn_text>"

    // go to start position
    line->atTokStart(&err);
    if (err < 0) goto error;

    // get account id
    response.account_id = line->atTokNextint(&err);
    if (err < 0) goto error;

    // get direction
    response.direction = line->atTokNextstr(&err);
    if (err < 0) goto error;

    // get SIP_msg_type
    response.sip_msg_type = line->atTokNextstr(&err);
    if (err < 0) goto error;

    // get method
    response.method = line->atTokNextstr(&err);
    if (err < 0) goto error;

    // get response_code
    response.response_code = line->atTokNextint(&err);
    if (err < 0) goto error;

    // get reason_phrase
    response.reason_phrase = line->atTokNextstr(&err);
    if (err < 0) goto error;

    // get warn_text
    response.warn_text = line->atTokNextstr(&err);
    if (err < 0) goto error;

    logD(RFX_LOG_TAG, "handleSipRegInfoInd: (%d, %s, %s, %s, %d, %s, %s)", response.account_id,
            response.direction, response.sip_msg_type, response.method, response.response_code,
            response.reason_phrase, response.warn_text);

    urc = RfxMclMessage::obtainUrc(RFX_MSG_UNSOL_SIP_REG_INFO,
            m_slot_id, RfxSipRegInfoData(&response, sizeof(RIL_SipRegInfo)));
    responseToTelCore(urc);
    return;

error:
    logE(RFX_LOG_TAG, "There is something wrong with the +ESIPREGINFO");
}

void RmcImsControlUrcHandler::handleImsRegInfoInd(const sp<RfxMclMessage>& msg) {
    /*  + EIMSREGURI:<account_id>,<URI>,<expire_time>  */
    int err;
    RfxAtLine *p_cur = msg->getRawUrc();
    sp<RfxMclMessage> unsol;
    RIL_ImsRegInfo response;

    memset(&response, 0, sizeof(RIL_ImsRegInfo));

    // go to start position
    p_cur->atTokStart(&err);
    if (err < 0) goto error;

    // got account id
    response.account_id = p_cur->atTokNextint(&err);
    if (err < 0) goto error;

    // got uri
    response.uri = p_cur->atTokNextstr(&err);
    if (err < 0) goto error;

    // got expire time
    response.expire_time = p_cur->atTokNextint(&err);
    if (err < 0) goto error;

    if (response.expire_time == 0) {
        response.report_type = IMS_REGISTERING;
    } else {
        response.report_type = IMS_REGISTERED;
    }

    unsol = RfxMclMessage::obtainUrc(RFX_MSG_UNSOL_IMS_REGISTRATION_STATE_IND, m_slot_id,
            RfxImsRegInfoData(&response, sizeof(RfxImsRegInfoData)));
    responseToTelCore(unsol);
    return;

error:
    logE(RFX_LOG_TAG, "There is something wrong with the +EIMSREGURI");
}

void RmcImsControlUrcHandler::handleImsRegRespInd(const sp<RfxMclMessage>& msg) {
    /* + EIMSREGRESP:<account_id>,<URI>,<errCode>,<ErrorMessage> */
    int err;
    RfxAtLine *p_cur = msg->getRawUrc();
    sp<RfxMclMessage> unsol;
    RIL_ImsRegInfo response;

    memset(&response, 0, sizeof(RIL_ImsRegInfo));

    // go to start position
    p_cur->atTokStart(&err);
    if (err < 0) goto error;

    // got account id
    response.account_id = p_cur->atTokNextint(&err);
    if (err < 0) goto error;

    // got uri
    response.uri = p_cur->atTokNextstr(&err);
    if (err < 0) goto error;

    // got error code
    response.error_code = p_cur->atTokNextint(&err);
    if (err < 0) goto error;

    // got error message
    response.error_msg = p_cur->atTokNextstr(&err);
    if (err < 0) goto error;

    response.report_type = IMS_REGISTER_FAIL;

    unsol = RfxMclMessage::obtainUrc(RFX_MSG_UNSOL_IMS_REGISTRATION_STATE_IND, m_slot_id,
            RfxImsRegInfoData(&response, sizeof(RfxImsRegInfoData)));
    responseToTelCore(unsol);
    return;

error:
    logE(RFX_LOG_TAG, "There is something wrong with the +EIMSREGRESP");
}