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

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxMessageId.h"
#include "RmcGsmSmsUrcHandler.h"
#include <telephony/mtk_ril.h>
#include "rfx_properties.h"
#include "RfxStatusDefs.h"
#include "RfxIntsData.h"
#include "RfxStringData.h"
#include "RfxVoidData.h"
#include "RfxEtwsNotiData.h"
#include "RfxRawData.h"
#include "RfxRilUtils.h"


using ::android::String8;

static const char* gsmUrcList[] = {
    "+CMT:",
    "+EIMSCMT:",
    "+CDS:",
    "+EIMSCDS:",
    "+CMTI:",
    "+CIEV: 7",
    "+CBM:",
    "+ETWS:",
    "+ECBMWAC:",
};

RFX_IMPLEMENT_HANDLER_CLASS(RmcGsmSmsUrcHandler, RIL_CMD_PROXY_URC);

RFX_REGISTER_DATA_TO_URC_ID(RfxStringData, RFX_MSG_URC_RESPONSE_NEW_SMS);
RFX_REGISTER_DATA_TO_URC_ID(RfxRawData, RFX_MSG_URC_RESPONSE_NEW_BROADCAST_SMS);
RFX_REGISTER_DATA_TO_URC_ID(RfxEtwsNotiData, RFX_MSG_URC_RESPONSE_ETWS_NOTIFICATION);
RFX_REGISTER_DATA_TO_URC_ID(RfxVoidData, RFX_MSG_URC_SIM_SMS_STORAGE_FULL);
RFX_REGISTER_DATA_TO_URC_ID(RfxVoidData, RFX_MSG_URC_ME_SMS_STORAGE_FULL);
RFX_REGISTER_DATA_TO_URC_ID(RfxIntsData, RFX_MSG_URC_RESPONSE_NEW_SMS_ON_SIM);
RFX_REGISTER_DATA_TO_URC_ID(RfxStringData, \
        RFX_MSG_URC_RESPONSE_NEW_SMS_STATUS_REPORT);

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/
RmcGsmSmsUrcHandler::RmcGsmSmsUrcHandler(int slot_id, int channel_id) :
        RmcGsmSmsBaseHandler(slot_id, channel_id) {
        setTag(String8("RmcGsmSmsUrc"));
        const char **p = gsmUrcList;
        if (RfxRilUtils::isSmsSupport()) {
            registerToHandleURC(p, sizeof(gsmUrcList)/sizeof(char*));
        }
}

void RmcGsmSmsUrcHandler::onHandleUrc(const sp<RfxMclMessage>& msg) {
    char* ss = msg->getRawUrc()->getLine();

    if (strStartsWith(ss, "+CMT:")) {
        handleNewSms(msg);
    } else if (strStartsWith(ss, "+EIMSCMT:")) {
        handleNewImsSms(msg);
    } else if (strStartsWith(ss, "+CDS:")) {
        handleNewSmsStatusReport(msg);
    } else if (strStartsWith(ss, "+EIMSCDS:")) {
        handleNewImsSmsStatusReport(msg);
    } else if (strStartsWith(ss, "+CMTI:")) {
        handleNewSmsOnSim(msg);
    } else if (strStartsWith(ss, "+CIEV: 7")) {
        handleSimSmsStorageStatus(msg);
    } else if (strStartsWith(ss, "+CBM:") || strStartsWith(ss, "+ECBMWAC:")) {
        handleNewBroadcastSms(msg);
    } else if (strStartsWith(ss, "+ETWS:")) {
        handleNewEtwsNotification(msg);
    }
}

void RmcGsmSmsUrcHandler::onNewSms(const sp<RfxMclMessage>& msg) {
    int length;
    int err;
    RfxAtLine *pUrc = msg->getRawUrc();
    RfxAtLine *smsPdu = msg->getRawUrc2();
    sp<RfxMclMessage> unsol;

    pUrc->atTokStart(&err);

    // skip <alpha>
    length = pUrc->atTokNextint(&err);

    // get <length>
    length = pUrc->atTokNextint(&err);

    err = smsCheckReceivedPdu(length, smsPdu->getLine(), true);
    if (err < 0) goto error;

    unsol = RfxMclMessage::obtainUrc(RFX_MSG_URC_RESPONSE_NEW_SMS,
            m_slot_id, RfxStringData(smsPdu->getLine(), strlen(smsPdu->getLine())));
    responseToTelCore(unsol);

    return;

error:
    // Use the timer callback to send CNMA fail to modem and will reset the
    // sms_type value while the CNMA is sent
    int errAckType[1] = {ERR_ACK_CMT_ACK_RESET};
    sendEvent(RFX_MSG_EVENT_SMS_NEW_SMS_ERR_ACK, RfxIntsData(errAckType, 1),
            RIL_CMD_PROXY_1, m_slot_id);

    logE(mTag, "onNewSms check fail.");
}

void RmcGsmSmsUrcHandler::handleNewSms(const sp<RfxMclMessage>& msg) {
    int smsType = getMclStatusManager()->getIntValue(RFX_STATUS_KEY_GSM_INBOUND_SMS_TYPE,
            SMS_INBOUND_NONE);

    showCurrIncomingSmsType();
    if (smsType == SMS_INBOUND_NONE) {
        getMclStatusManager()->setIntValue(RFX_STATUS_KEY_GSM_INBOUND_SMS_TYPE, SMS_INBOUND_3GPP_CMT);
        onNewSms(msg);
    } else {
        logE(mTag, "One 3G or IMS SMS on AP, reject");
        // Use the timer callback to send CNMA fail to modem and will reset the
        // sms_type value while the CNMA is sent
        int errAckType[1] = {ERR_ACK_CMT_ACK_ONLY};
        sendEvent(RFX_MSG_EVENT_SMS_NEW_SMS_ERR_ACK, RfxIntsData(errAckType, 1),
                RIL_CMD_PROXY_1, m_slot_id);
    }
}

void RmcGsmSmsUrcHandler::handleNewImsSms(const sp<RfxMclMessage>& msg) {
    int smsType = getMclStatusManager()->getIntValue(RFX_STATUS_KEY_GSM_INBOUND_SMS_TYPE,
            SMS_INBOUND_NONE);

    showCurrIncomingSmsType();
    if (smsType == SMS_INBOUND_NONE) {
        getMclStatusManager()->setIntValue(RFX_STATUS_KEY_GSM_INBOUND_SMS_TYPE,
                SMS_INBOUND_3GPP_CMT);
        onNewSms(msg);
    } else {
        logE(mTag, "One 3G or IMS SMS on AP, reject");
        // Use the timer callback to send CNMA fail to modem and will reset the
        // sms_type value while the CNMA is sent
        int errAckType[1] = {ERR_ACK_CMT_ACK_ONLY};
        sendEvent(RFX_MSG_EVENT_SMS_NEW_SMS_ERR_ACK, RfxIntsData(errAckType, 1),
                RIL_CMD_PROXY_1, m_slot_id);
    }
}

void RmcGsmSmsUrcHandler::onNewSmsStatusReport(const sp<RfxMclMessage>& msg) {
    int length;
    int err;
    RfxAtLine *pUrc = msg->getRawUrc();
    RfxAtLine *smsPdu = msg->getRawUrc2();
    sp<RfxMclMessage> unsol;

    pUrc->atTokStart(&err);

    length = pUrc->atTokNextint(&err);

    err = smsCheckReceivedPdu(length, smsPdu->getLine(), false);
    if (err < 0) goto error;

    unsol = RfxMclMessage::obtainUrc(RFX_MSG_URC_RESPONSE_NEW_SMS_STATUS_REPORT,
            m_slot_id, RfxStringData(smsPdu->getLine(), strlen(smsPdu->getLine())));
    responseToTelCore(unsol);

    return;

error:
    // Use the timer callback to send CNMA fail to modem and will reset the
    // sms_type value while the CNMA is sent
    int errAckType[1] = {ERR_ACK_CDS_ACK_RESET};
    sendEvent(RFX_MSG_EVENT_SMS_NEW_SMS_ERR_ACK, RfxIntsData(errAckType, 1),
            RIL_CMD_PROXY_1, m_slot_id);

    logE(mTag, "onNewSmsStatusReport check fail.");
}

void RmcGsmSmsUrcHandler::handleNewSmsStatusReport(const sp<RfxMclMessage>& msg) {
    int smsType = getMclStatusManager()->getIntValue(RFX_STATUS_KEY_GSM_INBOUND_SMS_TYPE,
            SMS_INBOUND_NONE);

    showCurrIncomingSmsType();
    if (smsType == SMS_INBOUND_NONE) {
        getMclStatusManager()->setIntValue(RFX_STATUS_KEY_GSM_INBOUND_SMS_TYPE, SMS_INBOUND_3GPP_CDS);
        onNewSmsStatusReport(msg);
    } else {
        logE(mTag, "One 3G or IMS SMS report on AP, reject");
        // Use the timer callback to send CNMA fail to modem and will reset the
        // sms_type value while the CNMA is sent
        int errAckType[1] = {ERR_ACK_CDS_ACK_ONLY};
        sendEvent(RFX_MSG_EVENT_SMS_NEW_SMS_ERR_ACK, RfxIntsData(errAckType, 1),
                RIL_CMD_PROXY_1, m_slot_id);
    }
}

void RmcGsmSmsUrcHandler::handleNewImsSmsStatusReport(const sp<RfxMclMessage>& msg) {
    int smsType = getMclStatusManager()->getIntValue(RFX_STATUS_KEY_GSM_INBOUND_SMS_TYPE,
            SMS_INBOUND_NONE);

    showCurrIncomingSmsType();
    if (smsType == SMS_INBOUND_NONE) {
        getMclStatusManager()->setIntValue(RFX_STATUS_KEY_GSM_INBOUND_SMS_TYPE,
                SMS_INBOUND_3GPP_CDS);
        onNewSmsStatusReport(msg);
    } else {
        logE(mTag, "One 3G or IMS SMS report on AP, reject");
        // Use the timer callback to send CNMA fail to modem and will reset the
        // sms_type value while the CNMA is sent
        int errAckType[1] = {ERR_ACK_CDS_ACK_ONLY};
        sendEvent(RFX_MSG_EVENT_SMS_NEW_SMS_ERR_ACK, RfxIntsData(errAckType, 1),
                RIL_CMD_PROXY_1, m_slot_id);
    }
}

void RmcGsmSmsUrcHandler::handleNewSmsOnSim(const sp<RfxMclMessage>& msg) {
    int err, index[1];
    char *strPara = NULL;
    String8 mem("");
    String8 storage("SM");
    RfxAtLine *pUrc = msg->getRawUrc();
    sp<RfxMclMessage> unsol;

    pUrc->atTokStart(&err);
    if (err < 0) goto error;

    strPara = pUrc->atTokNextstr(&err);
    if (err < 0) goto error;

    mem.append(strPara);
    if (mem != storage) {
        logE(mTag, "The specified storage is not SIM storage");
        return;
    }

    index[0] = pUrc->atTokNextint(&err);
    if (err < 0) goto error;

    unsol = RfxMclMessage::obtainUrc(RFX_MSG_URC_RESPONSE_NEW_SMS_ON_SIM,
            m_slot_id, RfxIntsData(index, 1));
    responseToTelCore(unsol);

    return;

error:
    logE(mTag, "There is something wrong with the input URC");
}

void RmcGsmSmsUrcHandler::handleSimSmsStorageStatus(const sp<RfxMclMessage>& msg) {
    int err;
    int status, ciev_id;
    int urc_id = -1;
    RfxAtLine *pUrc = msg->getRawUrc();

    pUrc->atTokStart(&err);

    ciev_id = pUrc->atTokNextint(&err);
    if (err < 0) return;

    status = pUrc->atTokNextint(&err);

    if (err < 0) {
        logE(mTag, "There is something wrong with the URC: +CIEV:7, <%d>", status);
    } else {
        switch(status) {
            case 0: // available
                break;
            case 1: // full
            case 2: // SIM exceed
                /* for mem1, mem2 and mem3, all are SIM card storage due to set as AT+CPMS="SM", "SM", "SM" */
                urc_id = RFX_MSG_URC_SIM_SMS_STORAGE_FULL;
                break;
            case 3: // ME exceed
            {
                int phone_storage_status = getMclStatusManager()->getIntValue(
                        RFX_STATUS_KEY_SMS_PHONE_STORAGE, 0);
                logD(mTag, "Phone storage status: %d", phone_storage_status);
                if (phone_storage_status == 1) {
                    /*********************************************************
                     * It is a workaround solution here.
                     * for mem1, mem2 and mem3, even if all are SIM card
                     * storage due to set as AT+CPMS="SM", "SM", "SM", we still
                     * need to check the phone_storage_status.
                     * It is the current limitation on modem, it should
                     * upgrate the +CIEV for better usage.
                     ********************************************************/
                    urc_id = RFX_MSG_URC_ME_SMS_STORAGE_FULL;
                } else {
                    /* for mem1, mem2 and mem3, all are SIM card storage due to set as AT+CPMS="SM", "SM", "SM" */
                    urc_id = RFX_MSG_URC_SIM_SMS_STORAGE_FULL;
                }
                break;
            }
            default:
                logW(mTag, "The received <status> is invalid: %d", status);
                break;
        }
        if (urc_id >= 0) {
            sp<RfxMclMessage> unsol = RfxMclMessage::obtainUrc(urc_id, m_slot_id, RfxVoidData());
            responseToTelCore(unsol);
        }
    }
}
void RmcGsmSmsUrcHandler::handleNewBroadcastSms(const sp<RfxMclMessage>& msg) {
    int length;
    int err;
    char *line = NULL;
    char *byteSmsPdu;
    RfxAtLine *pUrc = msg->getRawUrc();
    RfxAtLine *pSmsPdu = msg->getRawUrc2();
    sp<RfxMclMessage> unsol;

    pUrc->atTokStart(&err);

    length = pUrc->atTokNextint(&err);

    asprintf(&line, "00%s", pSmsPdu->getLine());
    err = smsCheckReceivedPdu(length, line, false);
    free(line);
    if (err < 0) goto error;
    /* Transfer to byte array for responseRaw */
    byteSmsPdu = (char*)calloc(1, sizeof(char)*length);
    hexStringToBytes(pSmsPdu->getLine(), length*2, byteSmsPdu, length);
    logD(mTag, "New CB bytes len %d", length);

    unsol = RfxMclMessage::obtainUrc(RFX_MSG_URC_RESPONSE_NEW_BROADCAST_SMS, m_slot_id,
            RfxRawData(byteSmsPdu, sizeof(char)*length));
    responseToTelCore(unsol);

    free(byteSmsPdu);
    return;

error:
    logE(mTag, "handleNewBroadcastSms check fail");
}

void RmcGsmSmsUrcHandler::handleNewEtwsNotification(const sp<RfxMclMessage>& msg) {
    int err;
    RfxAtLine *p_cur = msg->getRawUrc();
    sp<RfxMclMessage> unsol;
    RIL_CBEtwsNotification response;

    logD(mTag, "[ETWS: enter handleNewEtwsNotification");


    /*
     * +ETWS: <warning type>,<message ID>,<serial number>,<PLMN_ID>[,<security info>]
     */
    p_cur->atTokStart(&err);
    if (err < 0) goto error;

    response.warningType = p_cur->atTokNextint(&err);
    if(err < 0) goto error;

    response.messageId = p_cur->atTokNextint(&err);
    if(err < 0) goto error;

    response.serialNumber = p_cur->atTokNextint(&err);
    if(err < 0) goto error;

    response.plmnId = p_cur->atTokNextstr(&err);
    if(err < 0) goto error;

    if (p_cur->atTokHasmore()) {
        response.securityInfo = p_cur->atTokNextstr(&err);
        if (err < 0) goto error;
    } else {
        response.securityInfo = NULL;
    }

    unsol = RfxMclMessage::obtainUrc(RFX_MSG_URC_RESPONSE_ETWS_NOTIFICATION, m_slot_id,
            RfxEtwsNotiData(&response, sizeof(RIL_CBEtwsNotification)));
    responseToTelCore(unsol);
    return;

error:
    logE(mTag, "[ETWS: There is something wrong with ETWS URC");
}
