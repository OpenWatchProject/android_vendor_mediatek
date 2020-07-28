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
#include "RmcGsmSmsRequestHandler.h"
#include <telephony/mtk_ril.h>
#include <stdio.h>
#include <string.h>
#include <cstring>
#include "RfxMessageId.h"
#include "RmcGsmSmsDefs.h"
#include "RfxLog.h"
#include "RfxStatusDefs.h"
#include "RfxSmsRspData.h"
#include "RfxVoidData.h"
#include "RfxIntsData.h"
#include "RfxSmsSimMemStatusCnfData.h"
#include "RfxStringData.h"
#include "RfxStringsData.h"
#include "RfxSmsWriteData.h"
#include "RfxGsmCbSmsCfgData.h"
#include "RfxSmsParamsData.h"
#include "RfxRilUtils.h"

using ::android::String8;

static const int non_realtime_req_list[] = {
        RFX_MSG_REQUEST_SEND_SMS,
        RFX_MSG_REQUEST_SEND_SMS_EXPECT_MORE,
        RFX_MSG_REQUEST_IMS_SEND_GSM_SMS,
        RFX_MSG_REQUEST_IMS_SEND_GSM_SMS_EX,
};

static const int channel1_req_list[] = {
        RFX_MSG_REQUEST_ACKNOWLEDGE_INCOMING_GSM_SMS_WITH_PDU,
        RFX_MSG_REQUEST_WRITE_SMS_TO_SIM,
        RFX_MSG_REQUEST_DELETE_SMS_ON_SIM,
        RFX_MSG_REQUEST_GSM_SET_BROADCAST_SMS_CONFIG,
        RFX_MSG_REQUEST_REPORT_SMS_MEMORY_STATUS,
        RFX_MSG_REQUEST_GET_SMS_SIM_MEM_STATUS,
        RFX_MSG_REQUEST_SMS_ACKNOWLEDGE,
        RFX_MSG_REQUEST_SMS_ACKNOWLEDGE_INTERNAL,
        RFX_MSG_REQUEST_SMS_ACKNOWLEDGE_EX,
        RFX_MSG_REQUEST_GSM_GET_BROADCAST_SMS_CONFIG,
        RFX_MSG_REQUEST_GET_SMSC_ADDRESS,
        RFX_MSG_REQUEST_SET_SMSC_ADDRESS,
        RFX_MSG_REQUEST_GSM_GET_BROADCAST_LANGUAGE,
        RFX_MSG_REQUEST_GSM_SET_BROADCAST_LANGUAGE,
        RFX_MSG_REQUEST_GSM_SMS_BROADCAST_ACTIVATION,
        RFX_MSG_REQUEST_SET_ETWS,
        RFX_MSG_REQUEST_REMOVE_CB_MESSAGE,
        RFX_MSG_REQUEST_GET_SMS_PARAMS,
        RFX_MSG_REQUEST_SET_SMS_PARAMS,
        RFX_MSG_REQUEST_GET_GSM_SMS_BROADCAST_ACTIVATION
};

static const int event_list[] = {
    RFX_MSG_EVENT_SMS_NEW_SMS_ERR_ACK,
};

// Register handler
RFX_IMPLEMENT_HANDLER_CLASS(RmcGsmSmsRequestHandler, RIL_CMD_PROXY_8);
RFX_REGISTER_HANDLER_CLASS(RmcGsmSmsRequestHandler, RIL_CMD_PROXY_1);

// Register dispatch and response class
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringsData, RfxSmsRspData, \
        RFX_MSG_REQUEST_SEND_SMS);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringsData, RfxSmsRspData, \
        RFX_MSG_REQUEST_SEND_SMS_EXPECT_MORE);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, \
        RFX_MSG_REQUEST_SMS_ACKNOWLEDGE);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, \
        RFX_MSG_REQUEST_SMS_ACKNOWLEDGE_EX);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, \
        RFX_MSG_REQUEST_SMS_ACKNOWLEDGE_INTERNAL);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxSmsWriteData, RfxIntsData, \
        RFX_MSG_REQUEST_WRITE_SMS_TO_SIM);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, \
        RFX_MSG_REQUEST_DELETE_SMS_ON_SIM);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, \
        RFX_MSG_REQUEST_REPORT_SMS_MEMORY_STATUS);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxSmsSimMemStatusCnfData, \
        RFX_MSG_REQUEST_GET_SMS_SIM_MEM_STATUS);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxStringData, \
        RFX_MSG_REQUEST_GET_SMSC_ADDRESS);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringData, RfxVoidData, \
        RFX_MSG_REQUEST_SET_SMSC_ADDRESS);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringsData, RfxVoidData, \
        RFX_MSG_REQUEST_ACKNOWLEDGE_INCOMING_GSM_SMS_WITH_PDU);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxGsmCbSmsCfgData, \
        RFX_MSG_REQUEST_GSM_GET_BROADCAST_SMS_CONFIG);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxGsmCbSmsCfgData, RfxVoidData, \
        RFX_MSG_REQUEST_GSM_SET_BROADCAST_SMS_CONFIG);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxStringData, \
        RFX_MSG_REQUEST_GSM_GET_BROADCAST_LANGUAGE);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringData, RfxVoidData, \
        RFX_MSG_REQUEST_GSM_SET_BROADCAST_LANGUAGE);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, \
        RFX_MSG_REQUEST_GSM_SMS_BROADCAST_ACTIVATION);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxIntsData, \
        RFX_MSG_REQUEST_GET_GSM_SMS_BROADCAST_ACTIVATION);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, \
        RFX_MSG_REQUEST_SET_ETWS);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, \
        RFX_MSG_REQUEST_REMOVE_CB_MESSAGE);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxSmsParamsData, \
        RFX_MSG_REQUEST_GET_SMS_PARAMS);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxSmsParamsData, RfxVoidData, \
        RFX_MSG_REQUEST_SET_SMS_PARAMS);


// Register event
RFX_REGISTER_DATA_TO_EVENT_ID(RfxIntsData, RFX_MSG_EVENT_SMS_NEW_SMS_ERR_ACK);



/*****************************************************************************
 * Class RmcGsmSmsRequestHandler
 *****************************************************************************/


RmcGsmSmsRequestHandler::RmcGsmSmsRequestHandler(int slot_id, int channel_id) :
        RmcGsmSmsBaseHandler(slot_id, channel_id) {
    setTag(String8("RmcGsmSmsRequest"));
    if (channel_id == RIL_CMD_PROXY_8 && RfxRilUtils::isSmsSupport()) {
        registerToHandleRequest(non_realtime_req_list, sizeof(non_realtime_req_list)/sizeof(int));
    } else if (channel_id == RIL_CMD_PROXY_1 && RfxRilUtils::isSmsSupport()) {
        registerToHandleRequest(channel1_req_list, sizeof(channel1_req_list)/sizeof(int));
        registerToHandleEvent(event_list, sizeof(event_list)/sizeof(int));
    }
}

void RmcGsmSmsRequestHandler::onHandleRequest(const sp<RfxMclMessage>& msg) {
    int request = msg->getId();
    switch(request) {
        case RFX_MSG_REQUEST_SEND_SMS:
        case RFX_MSG_REQUEST_SEND_SMS_EXPECT_MORE:
        case RFX_MSG_REQUEST_IMS_SEND_GSM_SMS:
        case RFX_MSG_REQUEST_IMS_SEND_GSM_SMS_EX:
            handleSendSms(msg);
            break;
        case RFX_MSG_REQUEST_SMS_ACKNOWLEDGE:
        case RFX_MSG_REQUEST_SMS_ACKNOWLEDGE_EX:
        case RFX_MSG_REQUEST_SMS_ACKNOWLEDGE_INTERNAL:
            handleSmsAcknowledge(msg);
            break;
        case RFX_MSG_REQUEST_WRITE_SMS_TO_SIM:
            handleWriteSmsToSim(msg);
            break;
        case RFX_MSG_REQUEST_DELETE_SMS_ON_SIM:
            handleDeleteSmsOnSim(msg);
            break;
        case RFX_MSG_REQUEST_REPORT_SMS_MEMORY_STATUS:
            handleReportSmsMemStatus(msg);
            break;
        case RFX_MSG_REQUEST_GET_SMS_SIM_MEM_STATUS:
            handleGetSmsSimMemStatus(msg);
            break;
        case RFX_MSG_REQUEST_GET_SMSC_ADDRESS:
            handleGetSmscAddress(msg);
            break;
        case RFX_MSG_REQUEST_SET_SMSC_ADDRESS:
            handleSetSmscAddress(msg);
            break;
        case RFX_MSG_REQUEST_ACKNOWLEDGE_INCOMING_GSM_SMS_WITH_PDU:
            handleSmsAcknowledgeWithPdu(msg);
            break;
        case RFX_MSG_REQUEST_GSM_GET_BROADCAST_SMS_CONFIG:
            handleGetGsmCbSmsCfg(msg, GSM_CB_QUERY_CHANNEL);
            break;
        case RFX_MSG_REQUEST_GSM_SET_BROADCAST_SMS_CONFIG:
            handleSetGsmCbSmsCfg(msg);
            break;
        case RFX_MSG_REQUEST_GSM_GET_BROADCAST_LANGUAGE:
            handleGetGsmCbSmsCfg(msg, GSM_CB_QUERY_LANGUAGE);
            break;
        case RFX_MSG_REQUEST_GET_GSM_SMS_BROADCAST_ACTIVATION:
            handleGsmGetCbSmsActivation(msg);
            break;
        case RFX_MSG_REQUEST_GSM_SET_BROADCAST_LANGUAGE:
            handleSetGsmCbSmsLanguage(msg);
            break;
        case RFX_MSG_REQUEST_GSM_SMS_BROADCAST_ACTIVATION:
            handleGsmCbSmsActivation(msg);
            break;
        case RFX_MSG_REQUEST_SET_ETWS:
            handleGsmEtwsActivation(msg);
            break;
        case RFX_MSG_REQUEST_REMOVE_CB_MESSAGE:
            handleRemoveCbMsg(msg);
            break;
        case RFX_MSG_REQUEST_GET_SMS_PARAMS:
            handleGetSmsParams(msg);
            break;
        case RFX_MSG_REQUEST_SET_SMS_PARAMS:
            handleSetSmsParams(msg);
            break;
        default:
            logE(mTag, "Not support the request!");
            break;
    }
}

void RmcGsmSmsRequestHandler::onHandleEvent(const sp < RfxMclMessage > & msg) {
    int event = msg->getId();
    switch(event) {
        case RFX_MSG_EVENT_SMS_NEW_SMS_ERR_ACK:
            sendNewSmsErrorAck(msg);
            break;
        default:
            logE(mTag, "Not support the event!");
            break;
    }
}

void RmcGsmSmsRequestHandler::handleSendSms(const sp<RfxMclMessage>& msg) {
    char** data = (char**)msg->getData()->getData();
    int err, len=0, cause;
    String8 smsc("");
    String8 pdu("");
    String8 cmd1("");
    RfxAtLine *line = NULL;
    char buffer[(MAX_SMSC_LENGTH+MAX_TPDU_LENGTH+1)*2+1];   // +2: one for <length of SMSC>, one for \0
    RIL_SMS_Response response;
    memset(&response, 0, sizeof(response));
    sp<RfxAtResponse> p_response = NULL;
    sp<RfxMclMessage> rsp;
    RIL_Errno ril_errno = RIL_E_GENERIC_FAILURE;

    // check card status
    if (getMclStatusManager()->getIntValue(RFX_STATUS_KEY_CARD_TYPE) == 0) {
        logE(mTag, "SIM card was removed!");
        rsp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SIM_ABSENT,
                RfxVoidData(), msg, false);
        responseToTelCore(rsp);
        return;
    }

    if (data[0] != NULL) {
        smsc.append(String8::format("%s", data[0]));
    }
    if (data[1] != NULL) {
        pdu.append(String8::format("%s", data[1]));
    }

    /* fill error code first */
    response.errorCode = ril_errno;

    len = smsPackPdu(((!smsc.isEmpty())? smsc.string() : NULL),
            ((!pdu.isEmpty())? pdu.string() : NULL), buffer, true);
    if(len < 0) goto error;

    /********************************************
     * if the request is RIL_REQUEST_SEND_SMS_EXPECT_MORE
     * We should send AT+CMMS=1 to enable keeping relay protocol link
     *********************************************/
    if (msg->getId() == RFX_MSG_REQUEST_SEND_SMS_EXPECT_MORE) {
        p_response = atSendCommand("AT+CMMS=1");
        err = p_response->getError();
        if (err < 0 || p_response->getSuccess() == 0) {
            logE(mTag, "Error occurs while executing AT+CMMS=1");
            goto error;
        }
        p_response = NULL;
    }

    cmd1.append(String8::format("AT+CMGS=%d, \"%s\"", len, buffer));
    p_response = atSendCommandSingleline(cmd1, "+CMGS:");
    err = p_response->getError();

    if (err < 0) {
        logE(mTag, "Error occurs while executing AT+CMGS=%d", len);
        goto error;
    }

    if (p_response->getSuccess() == 0) {
        /****************************************
         * FIXME
         * Need to check what cause should be retry latter.
         ****************************************/
        char value[RFX_PROPERTY_VALUE_MAX] = {0};
        int smsRetry = 0;
        rfx_property_get("ro.vendor.enable.sms.fail.retry", value, "0");
        smsRetry = atoi(value);

        cause = atGetCmsError(p_response);
        if (cause == CMS_MTK_FDN_CHECK_FAILURE) {
            ril_errno = RIL_E_FDN_CHECK_FAILURE;
        } else if (cause == CMS_MTK_REQ_RETRY ||
                (cause == CMS_CM_SMS_CONNECTION_BROKEN && smsRetry == 1)) {
            ril_errno = RIL_E_SMS_SEND_FAIL_RETRY;
        }

        /********************************************
         * free because it needs to send another AT
         * commands again
         ********************************************/
        p_response = NULL;


        /********************************************
         * if the request is RIL_REQUEST_SEND_SMS_EXPECT_MORE
         * We should send AT+CMMS=0 to disable keeping relay protocol link
         *********************************************/
        if (msg->getId() == RFX_MSG_REQUEST_SEND_SMS_EXPECT_MORE) {
            p_response = atSendCommand("AT+CMMS=0");
            err = p_response->getError();
            if (err != 0 || p_response->getSuccess() == 0) {
                logW(mTag, "Warning: error occurs while executing AT+CMMS=0");
            }
        }

        response.errorCode = ril_errno;

        goto error;
    }

// Get message reference and ackPDU
    line = p_response->getIntermediates();
    line->atTokStart(&err);

    response.messageRef = line->atTokNextint(&err);
    logD(mTag, "message reference number: %d ", response.messageRef);
    if (err < 0) goto error;

    if (line->atTokHasmore()) {
        response.ackPDU = line->atTokNextstr(&err);
        if (err < 0) goto error;
    } else {
        response.ackPDU = NULL;
    }

    /* fill success code */
    response.errorCode = RIL_E_SUCCESS;
    rsp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
                RfxSmsRspData((void*)&response, sizeof(response)), msg, false);

    responseToTelCore(rsp);
    return;
error:
    rsp = RfxMclMessage::obtainResponse(msg->getId(), ril_errno, RfxSmsRspData(NULL, 0), msg, false);
    responseToTelCore(rsp);
}

void RmcGsmSmsRequestHandler::handleSmsAcknowledge(const sp<RfxMclMessage>& msg) {
    void *pData = msg->getData()->getData();
    int ackSuccess, cause;
    int err = 0;
    int smsType = getMclStatusManager()->getIntValue(RFX_STATUS_KEY_GSM_INBOUND_SMS_TYPE,
            SMS_INBOUND_NONE);
    sp<RfxAtResponse> p_response;
    String8 cmd("");
    sp<RfxMclMessage> rsp;

    ackSuccess = ((int *)pData)[0];
    cause = ((int *)pData)[1] & 0xFF;

    showCurrIncomingSmsType();
    if (ackSuccess == 1) {
        if (smsType == SMS_INBOUND_3GPP_CMT || smsType == SMS_INBOUND_3GPP_CMTI) {
            p_response = atSendCommand("AT+CNMA=1, 2, \"0000\"");
            err = p_response->getError();
        } else if (smsType == SMS_INBOUND_3GPP_CDS) {
            p_response = atSendCommand("AT+ECDSCNMA=1, 2, \"0000\"");
            err = p_response->getError();
        } else {
            logE(mTag, "SMS type is SMS_NONE");
            goto error;
        }
    } else if (ackSuccess == 0) {
        if (smsType == SMS_INBOUND_3GPP_CMT || smsType == SMS_INBOUND_3GPP_CMTI) {
            cmd.append(String8::format("AT+CNMA=2, 3, \"00%02X00\"", cause));
        } else if (smsType == SMS_INBOUND_3GPP_CDS) {
            cmd.append(String8::format("AT+ECDSCNMA=2, 3, \"00%02X00\"", cause));
        } else {
            logE(mTag, "SMS type is SMS_NONE");
            goto error;
        }
        p_response = atSendCommand(cmd);
        err = p_response->getError();
    } else {
        logE(mTag, "unsupported arg to RFX_MSG_REQUEST_SMS_ACKNOWLEDGE");
        goto error;
    }

    if (err < 0 || p_response->getSuccess() == 0)
        goto error;

    getMclStatusManager()->setIntValue(RFX_STATUS_KEY_GSM_INBOUND_SMS_TYPE, SMS_INBOUND_NONE);
    rsp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS, RfxVoidData(), msg, false);
    responseToTelCore(rsp);
    return;
error:
    getMclStatusManager()->setIntValue(RFX_STATUS_KEY_GSM_INBOUND_SMS_TYPE, SMS_INBOUND_NONE);
    rsp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
            RfxVoidData(), msg, false);
    responseToTelCore(rsp);
    return;
}

void RmcGsmSmsRequestHandler::handleWriteSmsToSim(const sp<RfxMclMessage>& msg) {
    RIL_SMS_WriteArgs *p_args = (RIL_SMS_WriteArgs*)(msg->getData()->getData());
    String8 cmd("");
    char buffer[(MAX_SMSC_LENGTH+MAX_TPDU_LENGTH+1)*2+1];   // +2: one for <length of SMSC>, one for \0;
    int length, err, response[1], ret = 0;
    sp<RfxAtResponse> p_response;
    RfxAtLine *line;
    sp<RfxMclMessage> rsp;

    // check card status
    if (getMclStatusManager()->getIntValue(RFX_STATUS_KEY_CARD_TYPE) == 0) {
        logE(mTag, "SIM card was removed!");
        rsp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SIM_ABSENT,
            RfxIntsData(), msg, false);
        responseToTelCore(rsp);
        return;
    }
    //check stat
    if (p_args->status < RIL_SMS_REC_UNREAD || p_args->status >= RIL_SMS_MESSAGE_MAX) {
        logE(mTag, "The status is invalid: %d", p_args->status);
        goto error;
    }

    // pack PDU with SMSC
    length = smsPackPdu(p_args->smsc, p_args->pdu, buffer, false);
    if (length < 0) goto error;

    cmd.append(String8::format("AT+CMGW=%d,%d, \"%s\"", length, p_args->status, buffer));
    p_response = atSendCommandSingleline(cmd, "+CMGW:");
    err = p_response->getError();

    ret = atGetCmsError(p_response);
    if (err < 0 || p_response->getSuccess() == 0) goto error;

    // get +CMGW: <index>
    line = p_response->getIntermediates();

    line->atTokStart(&err);
    if( err < 0) goto error;

    response[0] = line->atTokNextint(&err);
    if( err < 0) goto error;

    rsp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxIntsData((void*)response, sizeof(response)), msg, false);

    responseToTelCore(rsp);
    return;
error:
    if(CMS_CM_SIM_IS_FULL == ret) {
        rsp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SIM_FULL,
            RfxIntsData(), msg, false);
    } else {
        rsp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
            RfxIntsData(), msg, false);
    }
    responseToTelCore(rsp);
}

void RmcGsmSmsRequestHandler::handleDeleteSmsOnSim(const sp<RfxMclMessage>& msg) {
    int *pData = (int*)(msg->getData()->getData());
    String8 cmd("");
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> rsp;
    int err, index;
    RIL_Errno ril_errno = RIL_E_GENERIC_FAILURE;

    // check card status
    if (getMclStatusManager()->getIntValue(RFX_STATUS_KEY_CARD_TYPE) == 0) {
        logE(mTag, "SIM card was removed!");
        rsp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SIM_ABSENT,
            RfxVoidData(), msg, false);
        responseToTelCore(rsp);
        return;
    }
    index = ((int *)pData)[0];

    if(index > 0) {
        cmd.append(String8::format("AT+CMGD=%d", index));
        p_response = atSendCommand(cmd);
        err = p_response->getError();

        if ( !(err < 0 || p_response->getSuccess() == 0) ) {
            ril_errno = RIL_E_SUCCESS;
        }
        p_response = NULL;
    } else if(-1 == index) {
        // delete all sms on SIM
        cmd.append(String8::format("AT+CMGD=0,4"));
        p_response = atSendCommand(cmd);
        err = p_response->getError();

        if ( !(err < 0 || p_response->getSuccess() == 0) ) {
            ril_errno = RIL_E_SUCCESS;
        }
        p_response = NULL;
    }
    rsp = RfxMclMessage::obtainResponse(msg->getId(), ril_errno, RfxVoidData(), msg, false);
    responseToTelCore(rsp);
}

void RmcGsmSmsRequestHandler::handleReportSmsMemStatus(const sp<RfxMclMessage>& msg) {
    int *pData = (int*)(msg->getData()->getData());
    int available;
    int err;
    String8 line("");
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> rsp;
    RIL_Errno ril_errno = RIL_E_GENERIC_FAILURE;

    // check card status
    if (getMclStatusManager()->getIntValue(RFX_STATUS_KEY_CARD_TYPE) == 0) {
        logE(mTag, "SIM card was removed!");
        rsp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SIM_ABSENT,
            RfxVoidData(), msg, false);
        responseToTelCore(rsp);
        return;
    }

    available = ((int *)pData)[0];
    if (available == 1 || available == 0) {
        line.append(String8::format("AT+EMEMS=1, %d", available==1 ? 0 : 1));
        p_response = atSendCommand(line);
        err = p_response->getError();
        if (! (err < 0 || p_response->getSuccess() <= 0) ) {
            ril_errno = RIL_E_SUCCESS;
            getMclStatusManager()->setIntValue(RFX_STATUS_KEY_SMS_PHONE_STORAGE,
                    (available==1 ? 0 : 1));
        }
        logD(mTag, "Phone storage status: %d",
                getMclStatusManager()->getIntValue(RFX_STATUS_KEY_SMS_PHONE_STORAGE, 0));
    }

    rsp = RfxMclMessage::obtainResponse(msg->getId(), ril_errno, RfxVoidData(), msg, false);
    responseToTelCore(rsp);
}

void RmcGsmSmsRequestHandler::handleGetSmsSimMemStatus(const sp<RfxMclMessage>& msg) {
    RIL_Errno ril_errno = RIL_E_GENERIC_FAILURE;
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> rsp;
    RIL_SMS_Memory_Status mem_status = {0, 0};
    RfxAtLine *line = NULL;
    int err;
    int i;

    p_response = atSendCommandSingleline("AT+CPMS?", "+CPMS:");
    err = p_response->getError();

    if (err < 0 || p_response->getSuccess() <= 0)
        goto done;

    line = p_response->getIntermediates();
    line->atTokStart(&err);
    if (err < 0) goto done;

    for (i = 0; i < RIL_SMS_MEM_TYPE_TOTAL; i++) {
        char *mem_type_str;

        mem_type_str = line->atTokNextstr(&err);
        if (err < 0) goto done;

        if (strcmp(mem_type_str, "SM") == 0 || strcmp(mem_type_str, "SM_P") == 0) {
            mem_status.used = line->atTokNextint(&err);
            if (err < 0) goto done;

            mem_status.total = line->atTokNextint(&err);
            if (err < 0) goto done;

            ril_errno = RIL_E_SUCCESS;
            break;
        }
    }

done:
    rsp = RfxMclMessage::obtainResponse(msg->getId(), ril_errno,
            RfxSmsSimMemStatusCnfData((void*)&mem_status, sizeof(mem_status)), msg, false);
    responseToTelCore(rsp);
}

void RmcGsmSmsRequestHandler::sendNewSmsErrorAck(const sp<RfxMclMessage>& msg) {
    int* errAckType = (int*)(msg->getData()->getData());

    atSendCommand("AT+CNMA=2, 3, \"00FF00\"");

    showCurrIncomingSmsType();
    if ((*errAckType) == ERR_ACK_CMT_ACK_RESET || (*errAckType) == ERR_ACK_CDS_ACK_RESET) {
        getMclStatusManager()->setIntValue(RFX_STATUS_KEY_GSM_INBOUND_SMS_TYPE, SMS_INBOUND_NONE);
    }
}

void RmcGsmSmsRequestHandler::handleGetSmscAddress(const sp<RfxMclMessage>& msg) {
    int err, tosca;
    char *sca;
    RfxAtLine *line;
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> rsp;

    // check card status
    if (getMclStatusManager()->getIntValue(RFX_STATUS_KEY_CARD_TYPE) == 0) {
        logE(mTag, "SIM card was removed!");
        rsp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SIM_ABSENT,
            RfxStringData(NULL, 0), msg, false);
        responseToTelCore(rsp);
        return;
    }

    p_response = atSendCommandSingleline("AT+CSCA?", "+CSCA:");
    err = p_response->getError();
    if (err < 0 || p_response->getSuccess() <= 0) goto error;

    line = p_response->getIntermediates();

    line->atTokStart(&err);
    if (err < 0) goto error;

    // get <SCA>
    sca = line->atTokNextstr(&err);
    if (err < 0) goto error;

    // get <TOSCA>
    tosca = line->atTokNextint(&err);
    if (err < 0) goto error;

    if (tosca == 0x91 && sca[0] != '+')
    {
        // add '+' in front of the sca
        sca--;
        sca[0] = '+';
    }

    rsp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxStringData(sca, strlen(sca)), msg, false);

    responseToTelCore(rsp);
    return;

error:
    rsp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
            RfxStringData(NULL, 0), msg, false);

    responseToTelCore(rsp);
}

void RmcGsmSmsRequestHandler::handleSetSmscAddress(const sp<RfxMclMessage>& msg) {
    int err, tosca = 0x81, len;
    char *sca;
    char *tempsca = NULL;
    String8 cmd("");
    String8 empty("");
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> rsp;
    RIL_Errno ril_errno = RIL_E_GENERIC_FAILURE;

    // check card status
    if (getMclStatusManager()->getIntValue(RFX_STATUS_KEY_CARD_TYPE) == 0) {
        logE(mTag, "SIM card was removed!");
        rsp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SIM_ABSENT,
            RfxVoidData(), msg, false);
        responseToTelCore(rsp);
        return;
    }

    sca = ((msg->getData()->getData() != NULL)?
            (char*)(msg->getData()->getData()) : (char*)empty.string());

    if (sca[0] == '\"') {
        int idx = 0;
        int len = strlen(sca);
        //we need remove " and ", we just copy context, so malloc len - 2 + 1
        //such as, change "+12345" to +12345
        tempsca = (char *) malloc ((len - 1) * sizeof(char));
        RFX_ASSERT(tempsca != NULL);
        for (int i = 1; i < len - 1; i++) {
            tempsca[idx++] = sca[i];
        }
        tempsca[idx] = 0;
        sca = tempsca;
    }
    // get <tosca>
    if (sca[0] == '+')
    {
        tosca = 0x91;
        sca++;
    }

    len = strlen(sca);
    if (len > (MAX_SMSC_LENGTH-1)*2)
    {
        logE(mTag, "The input length of the SCA is too long: %d", len);

    }
    else
    {
        cmd.append(String8::format("AT+CSCA=\"%s\", %d", sca, tosca));
        p_response = atSendCommand(cmd);
        err = p_response->getError();
        if ( !(err < 0 || p_response->getSuccess() <= 0) )
        {
            /********************************************
            * free because it needs to send another AT
            * commands again
            ********************************************/
            p_response = NULL;
            cmd.clear();
            // ril_errno = RIL_E_SUCCESS;
            cmd.append("AT+CSAS");
            p_response = atSendCommand(cmd);
            err = p_response->getError();
            if ( !(err < 0 || p_response->getSuccess() <= 0) ) {
                ril_errno = RIL_E_SUCCESS;
            }
        }
    }

    if (tempsca != NULL) {
        free(tempsca);
    }

    rsp = RfxMclMessage::obtainResponse(msg->getId(), ril_errno, RfxVoidData(), msg, false);
    responseToTelCore(rsp);
}

void RmcGsmSmsRequestHandler::handleSmsAcknowledgeWithPdu(const sp<RfxMclMessage>& msg) {
    const char *ackSuccess;
    const char *pdu;
    int err;
    int smsType = getMclStatusManager()->getIntValue(RFX_STATUS_KEY_GSM_INBOUND_SMS_TYPE,
            SMS_INBOUND_NONE);
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> rsp;
    String8 cmd("");

    ackSuccess = ((const char **)(msg->getData()->getData()))[0];
    pdu = ((const char **)(msg->getData()->getData()))[1];

    showCurrIncomingSmsType();
    if (*ackSuccess == '1') {
        if (smsType == SMS_INBOUND_3GPP_CMT || smsType == SMS_INBOUND_3GPP_CMTI) {
            cmd.append(String8::format("AT+CNMA=1, 2, \"%s\"", pdu));
        } else if (smsType == SMS_INBOUND_3GPP_CDS) {
            cmd.append(String8::format("AT+ECDSCNMA=1, 2, \"%s\"", pdu));
        } else {
            logE(mTag, "SMS type is SMS_NONE");
            goto error;
        }
        p_response = atSendCommand(cmd);
        err = p_response->getError();
    } else if (*ackSuccess == '0') {
        if (smsType == SMS_INBOUND_3GPP_CMT || smsType == SMS_INBOUND_3GPP_CMTI) {
            cmd.append(String8::format("AT+CNMA=2, 3, \"%s\"", pdu));
        } else if (smsType == SMS_INBOUND_3GPP_CDS) {
            cmd.append(String8::format("AT+ECDSCNMA=2, 3, \"%s\"", pdu));
        } else {
            logE(mTag, "SMS type is SMS_NONE");
            goto error;
        }
        p_response = atSendCommand(cmd);
        err = p_response->getError();
    } else {
        logE(mTag, "unsupported arg to RFX_MSG_REQUEST_SMS_ACKNOWLEDGE\n");
        goto error;
    }

    if (err < 0 || p_response->getSuccess() == 0)
        goto error;

    getMclStatusManager()->setIntValue(RFX_STATUS_KEY_GSM_INBOUND_SMS_TYPE, SMS_INBOUND_NONE);
    rsp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS, RfxVoidData(), msg, false);
    responseToTelCore(rsp);
    return;
error:
    getMclStatusManager()->setIntValue(RFX_STATUS_KEY_GSM_INBOUND_SMS_TYPE, SMS_INBOUND_NONE);
    rsp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
            RfxVoidData(), msg, false);
    responseToTelCore(rsp);
    return;
}

void RmcGsmSmsRequestHandler::handleGetGsmCbSmsCfg(const sp<RfxMclMessage>& msg,
        GSM_CB_QUERY_MODE query) {
    int err = RIL_E_GENERIC_FAILURE;
    char *channels = NULL, *langs = NULL, *settings = NULL;
    RIL_GSM_BroadcastSmsConfigInfo *info = NULL;
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> rsp;
    RfxAtLine *line = NULL;
    String8 cmd("AT+ECSCBCFG?");

    // 1. Query the configuration from modem
    p_response = atSendCommandSingleline(cmd, "+ECSCBCFG:");
    err = p_response->getError();

    if (err < 0 || p_response->getSuccess() == 0) {
        logE(mTag, "Fail to query channel configuration");
        goto error;
    }

    line = p_response->getIntermediates();

    line->atTokStart(&err);
    if(err < 0) goto error;

    // get channels
    channels = line->atTokNextstr(&err);
    if(err < 0) goto error;

    // get languages
    langs = line->atTokNextstr(&err);
    if(err < 0) goto error;

    logD(mTag, "Current CB channels %s, langs %s, query for %d", channels, langs, query);


    if (query == GSM_CB_QUERY_CHANNEL) {
        asprintf(&settings, "%s", channels);
        if (strlen(channels) > 0) {
            // At least one channel id

            // 2. Convert string to RIL_GSM_BroadcastSmsConfigInfo
            int count = 0, i = 0;
            RIL_GSM_BroadcastSmsConfigInfo **gsmBciPtrs = NULL;

            gsmBciPtrs = convertChStringToGsmCbCfgInfo(settings, &count);

            // 3. Send AP channel configuration
            rsp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
                    RfxGsmCbSmsCfgData((void*)gsmBciPtrs,
                    sizeof(RIL_GSM_BroadcastSmsConfigInfo*)*count), msg, false);
            responseToTelCore(rsp);

            // free the output data
            if (gsmBciPtrs != NULL) {
                for (i = 0; i < count ; i++)
                {
                    if (gsmBciPtrs[i] != NULL) {
                        free(gsmBciPtrs[i]);
                    }
                }
                free(gsmBciPtrs);
            }
        } else {
            rsp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
                    RfxGsmCbSmsCfgData(NULL, 0), msg, false);
            responseToTelCore(rsp);
        }

    } else {
        // 2. Send AP languages configuration
        asprintf(&settings, "%s", langs);
        if (strlen(langs) > 0) {
            rsp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
                    RfxStringData((void*)settings, strlen(settings)), msg, false);
            responseToTelCore(rsp);
        } else {
            rsp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
                    RfxStringData(), msg, false);
            responseToTelCore(rsp);
        }
    }
    if (settings != NULL) {
        free(settings);
    }

    return;
error:
    rsp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
                    RfxStringData(), msg, false);
    responseToTelCore(rsp);
}

void RmcGsmSmsRequestHandler::handleSetGsmCbSmsCfg(const sp<RfxMclMessage>& msg) {
    int length = msg->getData()->getDataLength();
    int num = length/sizeof(RIL_GSM_BroadcastSmsConfigInfo *);
    int start = -1, end = -1;
    int err = RIL_E_GENERIC_FAILURE;
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> rsp;
    RIL_GSM_BroadcastSmsConfigInfo **info =
            (RIL_GSM_BroadcastSmsConfigInfo**)msg->getData()->getData();
    String8 settings("");
    String8 cmd("");

    logD(mTag, "Set CB channel array num %d", num);

    // 1. Convert RIL_GSM_BroadcastSmsConfigInfo to string

    for (int i = 0; i < num ; i++) {
        start = info[i]->fromServiceId;
        end = info[i]->toServiceId;

        if (i != 0) {
            settings.append(",");
        }

        if (start == end) {
            // Single channel id
            settings.append(String8::format("%d", start));
        } else {
            // A range of channel id
            settings.append(String8::format("%d-%d", start, end));
        }
    }

    logD(mTag, "Set CB channels %s", settings.string());

    // 2. Send modem the new configuration
    // the special string "--" means  we change nothing
    cmd.append(String8::format("AT+ECSCBCFG=\"%s\",\"--\"", settings.string()));

    p_response = atSendCommand(cmd);
    err = p_response->getError();
    if (err < 0 || p_response->getSuccess() == 0) {
        logE(mTag, "Fail to setup channel");
        goto error;
    }

    rsp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
                    RfxVoidData(), msg, false);
    responseToTelCore(rsp);
    return;
error:
    rsp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
                    RfxVoidData(), msg, false);
    responseToTelCore(rsp);
}

RIL_GSM_BroadcastSmsConfigInfo **RmcGsmSmsRequestHandler::convertChStringToGsmCbCfgInfo(
        char* channels, int *count)
{
    int num = 0, i = 0;
    char key[] = ",-";
    char *pHead = channels, *pCh = NULL, *tmpChStr = NULL;
    RIL_SMS_GSM_CB_CHANNEL_LIST *pCbListHead = NULL, *pCbList = NULL;
    RIL_GSM_BroadcastSmsConfigInfo **gsmBciPtrs = NULL;
    char ch[50];

    asprintf(&tmpChStr, "%s", channels);

    pHead = pCh = tmpChStr;

    // 1. Split string
    pCh = strpbrk(tmpChStr, key);
    while (pCh != NULL) {
        // New a RIL_SMS_GSM_CB_CHANNEL_list object
        if (pCbListHead == NULL) {
            // Point to the first element
            pCbListHead = (RIL_SMS_GSM_CB_CHANNEL_LIST*)calloc(
                    1, sizeof(RIL_SMS_GSM_CB_CHANNEL_LIST));
            pCbList = pCbListHead;
        } else {
            pCbList->pNext = (RIL_SMS_GSM_CB_CHANNEL_LIST*)calloc(
                    1, sizeof(RIL_SMS_GSM_CB_CHANNEL_LIST));
            pCbList = pCbList->pNext;
        }

        if (pCbList == NULL) {
            logE(mTag, "Failed to allocate memory for pCbList");
            break;
        }
        memset(ch, 0, sizeof(char)*50);
        strncpy(ch, pHead, (pCh - pHead));
        // Move pHead to next position
        pHead = pCh + 1;
        if ((*pCh) == ',') {
            // Match the key comma
            num++;
            pCbList->end = pCbList->start = atoi(ch);
            logD(mTag, "Single channel id, Start %d, End %d, num %d", pCbList->start, pCbList->end,
                    num);
        } else {
            // Match the key desh
            num++;
            pCbList->start = atoi(ch);
            pCh = strpbrk(pHead, key);
            memset(ch, 0, sizeof(char)*50);
            if (pCh != NULL) {
                strncpy(ch, pHead, (pCh - pHead));
            } else {
                // This must be the last channel id
                strncpy(ch, pHead, 49);
            }
            pCbList->end = atoi(ch);
            // Move pHead to next position
            pHead = ((pCh != NULL)? pCh + 1 : pCh);
            logD(mTag, "Range channel id, Start %d, End %d, num %d", pCbList->start, pCbList->end,
                    num);
        }

        if (pHead != NULL) {
            pCh = strpbrk(pHead, key);
        }
    }

    if (pHead != NULL) {
        // The case is the last channel id. e.g. "...,1234". Or only one channel id. e.g. "1234"
        if (pCbListHead == NULL) {
            pCbListHead = (RIL_SMS_GSM_CB_CHANNEL_LIST*)calloc(
                    1, sizeof(RIL_SMS_GSM_CB_CHANNEL_LIST));
            // Keep the first element
            pCbList = pCbListHead;
        } else if (pCbList != NULL) {
            pCbList->pNext = (RIL_SMS_GSM_CB_CHANNEL_LIST*)calloc(
                    1, sizeof(RIL_SMS_GSM_CB_CHANNEL_LIST));
            pCbList = pCbList->pNext;
        }

        if (pCbList != NULL) {
            num++;
            memset(ch, 0, sizeof(char)*50);
            strncpy(ch, pHead, 49);
            pCbList->end = pCbList->start = atoi(ch);
            logD(mTag, "The last channel id, Start %d, End %d, num %d", pCbList->start, pCbList->end,
                    num);
        } else {
            logE(mTag, "Failed to allocate memory for pCbList");
        }
    }

    // 2. Create RIL_GSM_BroadcastSmsConfigInfo and set value
    gsmBciPtrs = (RIL_GSM_BroadcastSmsConfigInfo**)calloc(
            1, sizeof(RIL_GSM_BroadcastSmsConfigInfo *)*num);

    if (gsmBciPtrs != NULL) {
        pCbList = pCbListHead;
        while (pCbList != NULL) {
            gsmBciPtrs[i] = (RIL_GSM_BroadcastSmsConfigInfo*)calloc(
                    1, sizeof(RIL_GSM_BroadcastSmsConfigInfo));
            if (gsmBciPtrs[i] == NULL) {
                logE(mTag, "Failed to allocate memory for gsmBciPtrs[%d]", i);
                break;
            }
            gsmBciPtrs[i]->fromServiceId = pCbList->start;
            gsmBciPtrs[i]->toServiceId = pCbList->end;
            gsmBciPtrs[i]->selected = 1;
            i++;
            pCbList = pCbList->pNext;
        }
    } else {
        logE(mTag, "Failed to allocate memory for gsmBciPtrs");
    }
    logD(mTag, "gsmBciPtrs count %d", i);

    // 3. Release
    pCbList = pCbListHead;
    while (pCbList != NULL) {
        pCbListHead = pCbList->pNext;
        free(pCbList);
        pCbList = pCbListHead;
    }

    free(tmpChStr);

    (*count) = i;
    return gsmBciPtrs;
}

void RmcGsmSmsRequestHandler::handleSetGsmCbSmsLanguage(const sp<RfxMclMessage>& msg) {
    int err = RIL_E_GENERIC_FAILURE;
    char *langs = (char*)msg->getData()->getData();
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> rsp;
    String8 cmd("");

    logD(mTag, "Set CB langs %s", langs);

    // Send modem the new configuration
    // the special string "--" means  we change nothing
    cmd.append(String8::format("AT+ECSCBCFG=\"--\", \"%s\"", langs));
    p_response = atSendCommand(cmd);
    err = p_response->getError();

    if (err < 0 || p_response->getSuccess() == 0) {
        logE(mTag, "Fail to setup language");
        goto error;
    }

    rsp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
                    RfxVoidData(), msg, false);
    responseToTelCore(rsp);

    return;
error:
    rsp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
                    RfxVoidData(), msg, false);
    responseToTelCore(rsp);
}

void RmcGsmSmsRequestHandler::handleGsmCbSmsActivation(const sp<RfxMclMessage>& msg) {
    int activation = ((int *)msg->getData()->getData())[0];
    int err = RIL_E_GENERIC_FAILURE;
    String8 cmd(String8::format("AT+ECSCBSW=%d", ((activation == 0)? 1 : 0)));
    String8 etwsCmd(String8::format("AT+ETWS=%d", ((activation == 0)? 5 : 0)));
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> rsp;

    logD(mTag, "Turn GSM CB %s", ((activation == 0)? "on" : "off"));

    p_response = atSendCommand(cmd);
    err = p_response->getError();

    if (err < 0 || p_response->getSuccess() == 0) {
        logE(mTag, "Fail to change CB activation");
        goto error;
    }

    p_response = atSendCommand(etwsCmd);
    err = p_response->getError();

    if (err < 0 || p_response->getSuccess() == 0)  {
        logE(mTag, "Fail to change ETWS activation");
        goto error;
    }

    rsp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
                    RfxVoidData(), msg, false);
    responseToTelCore(rsp);

    return;
error:
    rsp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
                    RfxVoidData(), msg, false);
    responseToTelCore(rsp);
}

void RmcGsmSmsRequestHandler::handleGsmGetCbSmsActivation(const sp<RfxMclMessage>& msg) {
    int activation[1] = {0};
    int err = RIL_E_GENERIC_FAILURE;
    String8 cmd("AT+ECSCBSW?");
    RfxAtLine *line = NULL;
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> rsp;

    // 1. Query the configuration from modem
    p_response = atSendCommandSingleline(cmd, "+ECSCBSW:");
    err = p_response->getError();

    if (err < 0 || p_response->getSuccess() == 0) {
        logE(mTag, "Fail to query gsm cb activation");
        goto error;
    }

    line = p_response->getIntermediates();

    line->atTokStart(&err);
    if(err < 0) goto error;

    // get channels
    activation[0] = line->atTokNextint(&err);
    if(err < 0) goto error;

    rsp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxIntsData((void*)activation, sizeof(activation)), msg, false);
    responseToTelCore(rsp);

    return;
error:
    rsp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
            RfxIntsData((void*)activation, sizeof(activation)), msg, false);
    responseToTelCore(rsp);
}

void RmcGsmSmsRequestHandler::handleGsmEtwsActivation(const sp<RfxMclMessage>& msg) {
    int err = RIL_E_GENERIC_FAILURE;
    int setting = ((int *)msg->getData()->getData())[0];
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> rsp;
    String8 cmd(String8::format("AT+ETWS=%d", setting));

    p_response = atSendCommand(cmd);
    err = p_response->getError();

    if (err < 0 || p_response->getSuccess() == 0) goto error;

    rsp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
                    RfxVoidData(), msg, false);
    responseToTelCore(rsp);
    return;

error:
    rsp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
                    RfxVoidData(), msg, false);
    responseToTelCore(rsp);
}

void RmcGsmSmsRequestHandler::handleRemoveCbMsg(const sp<RfxMclMessage>& msg) {
    int ret;
    int channelId = ((int *)msg->getData()->getData())[0];
    int serailId = ((int *)msg->getData()->getData())[1];
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> rsp;
    String8 cmd(String8::format("AT+ECBMR=%d,%d", channelId, serailId));

    p_response = atSendCommand(cmd);
    ret = p_response->getError();
    if (ret < 0 || p_response->getSuccess() == 0) goto error;

    rsp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
                    RfxVoidData(), msg, false);
    responseToTelCore(rsp);
    return;

error:
    rsp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
                    RfxVoidData(), msg, false);
    responseToTelCore(rsp);
}

void RmcGsmSmsRequestHandler::handleGetSmsParams(const sp<RfxMclMessage>& msg) {
    int ret;
    RfxAtLine *line = NULL;
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> rsp;
    RIL_SmsParams smsParams;

    p_response = atSendCommandSingleline("AT+CSMP?", "+CSMP:");
    ret = p_response->getError();
    if (ret < 0 || p_response->getSuccess() == 0) goto error;

    memset(&smsParams, 0, sizeof(RIL_SmsParams));
    line = p_response->getIntermediates();

    line->atTokStart(&ret);
    if (ret < 0) goto error;

    smsParams.format = line->atTokNextint(&ret);
    if (ret < 0) goto error;

    smsParams.vp = line->atTokNextint(&ret);
    if (ret < 0) goto error;

    smsParams.pid = line->atTokNextint(&ret);
    if (ret < 0) goto error;

    smsParams.dcs = line->atTokNextint(&ret);
    if (ret < 0) goto error;

    rsp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
                    RfxSmsParamsData((void*)&smsParams, sizeof(smsParams)), msg, false);
    responseToTelCore(rsp);
    return;

error:
    rsp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
                    RfxSmsParamsData(NULL, 0), msg, false);
    responseToTelCore(rsp);
}

void RmcGsmSmsRequestHandler::handleSetSmsParams(const sp<RfxMclMessage>& msg) {
    int err;
    RIL_SmsParams *p_smsParams = (RIL_SmsParams *)(msg->getData()->getData());
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> rsp;
    String8 cmd("");

    if(p_smsParams->format != 0x00 && p_smsParams->format != 0x02) {
        logE(mTag, "invalid validity period format %d", p_smsParams->format);
        goto error;
    }

    cmd.append(String8::format("AT+CSMP=%d,%d,%d,%d", p_smsParams->format, p_smsParams->vp,
            p_smsParams->pid, p_smsParams->dcs));

    p_response = atSendCommand(cmd);
    err = p_response->getError();
    if (err < 0 || p_response->getSuccess() == 0) goto error;

    // confirm modification
    p_response = atSendCommand("AT+CSAS");
    err = p_response->getError();
    if (err < 0 || p_response->getSuccess() <= 0) goto error;

    rsp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
                    RfxVoidData(), msg, false);
    responseToTelCore(rsp);
    return;

error:
    rsp = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
                    RfxVoidData(), msg, false);
    responseToTelCore(rsp);
}
