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

#include "RmcCatCommonRequestHandler.h"

static const int request_id_list[] = {
        RFX_MSG_REQUEST_STK_SEND_ENVELOPE_COMMAND,
        RFX_MSG_REQUEST_STK_SEND_TERMINAL_RESPONSE,
        RFX_MSG_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM,
        RFX_MSG_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM_WITH_RESULT_CODE,
        RFX_MSG_REQUEST_REPORT_STK_SERVICE_IS_RUNNING,
        RFX_MSG_REQUEST_STK_SEND_ENVELOPE_WITH_STATUS,
        RFX_MSG_REQUEST_STK_SEND_RESPONSE_BY_CMDTYPE,
        RFX_MSG_REQUEST_STK_EVENT_NOTIFY,
        RFX_MSG_REQUEST_STK_QUERY_CPIN_STATE,
        // BIP @{
        RFX_MSG_REQUEST_BIP_SEND_CONFIRM_INFO,
        // BIP @}
        // BTSAP @{
        RFX_MSG_REQUEST_SIM_SAP_TRANSFER_CARD_READER_STATUS
        // BTSAP @}
};

static const int event_list[] = {
        RFX_MSG_EVENT_STK_NOTIFY,
        RFX_MSG_EVENT_STK_QUERY_CPIN_STATE
};


// register handler to channel
RFX_IMPLEMENT_HANDLER_CLASS(RmcCatCommonRequestHandler, RIL_CMD_PROXY_1);
RFX_REGISTER_DATA_TO_EVENT_ID(RfxStringData, RFX_MSG_EVENT_STK_IS_RUNNING);


RmcCatCommonRequestHandler::RmcCatCommonRequestHandler(int slot_id,
        int channel_id): RfxBaseHandler(slot_id, channel_id) {
    registerToHandleRequest(request_id_list, sizeof(request_id_list)/sizeof(int));
    registerToHandleEvent(event_list, sizeof(event_list)/sizeof(int));

    //init member variables
    isEventNotifyQueued = false;
    isProaCmdQueued = false;
    pEventNotifyCmd = NULL;
    pProactiveCmd = NULL;
}

RmcCatCommonRequestHandler::~RmcCatCommonRequestHandler() {
}

void RmcCatCommonRequestHandler::onHandleRequest(const sp<RfxMclMessage>& msg) {
    int request = msg->getId();
    logD(RFX_LOG_TAG, "onHandleRequest: %s", RFX_ID_TO_STR(request));

    switch(request) {
        case RFX_MSG_REQUEST_REPORT_STK_SERVICE_IS_RUNNING:
            requestReportStkServiceIsRunning(msg);
            break;
        case RFX_MSG_REQUEST_STK_SEND_ENVELOPE_COMMAND:
            requestStkSendEnvelopeCommand(msg);
            break;
        case RFX_MSG_REQUEST_STK_SEND_TERMINAL_RESPONSE:
            requestStkSendTerminalResponse(msg);
            break;
        case RFX_MSG_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM:
        case RFX_MSG_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM_WITH_RESULT_CODE:
            requestStkHandleCallSetupRequestedFromSim(msg);
            break;
        case RFX_MSG_REQUEST_STK_SEND_ENVELOPE_WITH_STATUS:
            requestStkSendEnvelopeCommandWithStatus(msg);
            break;
        case RFX_MSG_REQUEST_STK_SEND_RESPONSE_BY_CMDTYPE:
            requestStkSendResponseByCmdType(msg);
            break;
        case RFX_MSG_REQUEST_STK_EVENT_NOTIFY:
            requestStkEventNotify(msg);
            break;
        case RFX_MSG_REQUEST_STK_QUERY_CPIN_STATE:
            requestStkQeryCpinState(msg);
            break;
        // BIP @{
        case RFX_MSG_REQUEST_BIP_SEND_CONFIRM_INFO:
            requestBipSendConfirmInfo(msg);
            break;
        // BIP @}
        // BTSAP @{
        case RFX_MSG_REQUEST_SIM_SAP_TRANSFER_CARD_READER_STATUS:
            requestBtSapTransferCardReaderStatus(msg);
            break;
        // BTSAP @}
        default:
            logE(RFX_LOG_TAG, "Should not be here");
            break;
    }
}

void RmcCatCommonRequestHandler::onHandleEvent(const sp<RfxMclMessage>& msg) {
    int id = msg->getId();
    logD(RFX_LOG_TAG, "onHandleEvent: msg id: %d", id);
    switch (id) {
        default:
            logE(RFX_LOG_TAG, "onHandleEvent: should not be here");
            break;
    }
}


void RmcCatCommonRequestHandler::setStkFlag(bool* source, bool flag)
{
    if (NULL == source) {
        logE(RFX_LOG_TAG,  "setStkFlag source is null.");
        return;
    }

    *(source) = flag;
    logD(RFX_LOG_TAG,  "setStkFlag isStkServiceRunning to %d.", *(source));
}

void RmcCatCommonRequestHandler::requestReportStkServiceIsRunning(const sp<RfxMclMessage>& msg) {
    logD(RFX_LOG_TAG,  "requestReportStkServiceIsRunning");
    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxVoidData(), msg, false);

    // response to TeleCore
    responseToTelCore(response);
}
void RmcCatCommonRequestHandler::requestStkEventNotify(const sp<RfxMclMessage>& msg) {
    sp<RfxAtResponse> p_response;
    RIL_Errno rilErrNo = RIL_E_SUCCESS;
    int *value = (int *) msg->getData()->getData();
    if (value == NULL) {
        logE(RFX_LOG_TAG,  "requestStkEventNotify value is NULL!!!!");
        return;
    }
    int cmdType = value[0];
    int cmdData = value[1];

    logD(RFX_LOG_TAG, "requestStkEventNotify: cmdType is %d, cmdData is %d", cmdType, cmdData);

    switch(cmdType) {
        case CMD_SEND_SS:
            if (50 == cmdData) {
                p_response = atSendCommand("AT+STKSS=50");
            } else if (0 == cmdData) {
                p_response = atSendCommand("AT+STKSS=0");
                if (p_response->getSuccess() != 1) {
                    p_response = atSendCommand("AT+STKSS=32,3");
                }
            }
            break;
        case CMD_SETUP_CALL:
            if (50 == cmdData) {
                p_response = atSendCommand("AT+STKCALL=50");
            }
            break;
        case CMD_SEND_USSD:
            if (50 == cmdData) {
                p_response = atSendCommand("AT+STKUSSD=50");
            } else if (0 == cmdData) {
                int domainInfo = ((int *) msg->getData()->getData())[2];
                if (1 == domainInfo) {
                    p_response = atSendCommand("AT+STKUSSI=0");
                    if (p_response->getSuccess() != 1) {
                        p_response = atSendCommand("AT+STKUSSI=32,8");
                    }
                } else {
                    p_response = atSendCommand("AT+STKUSSD=0");
                    if (p_response->getSuccess() != 1) {
                        p_response = atSendCommand("AT+STKUSSD=32,8");
                    }
                }
            }
            break;
        case CMD_SEND_SMS:
            p_response = atSendCommand("AT+STKSMS=0");
            break;
        case CMD_DTMF: {
            int inCallNumber = getInCallNumber();
            if(inCallNumber != 0) {
                p_response = atSendCommand("AT+STKDTMF=0");
                if (p_response->getSuccess() != 1) {
                    p_response = atSendCommand("AT+STKDTMF=32,9");
                }
            } else {
                p_response = atSendCommand("AT+STKDTMF=32,7");
            } }
            break;
        default:
            RFX_LOG_E(RFX_LOG_TAG, "Should not be here");
            break;
    }
    if (p_response != NULL) {
        if (p_response->getSuccess() != 1) {
            rilErrNo = RIL_E_GENERIC_FAILURE;
        }
        p_response = NULL;
    }
    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), rilErrNo,
            RfxVoidData(), msg, false);
    // response to TeleCore
    responseToTelCore(response);
}

void RmcCatCommonRequestHandler::requestStkQeryCpinState(const sp<RfxMclMessage>& msg) {
    bool isReady = false;
    int *data = NULL;
    int retryNum = 0;
    int result_Data[1];

    logD(RFX_LOG_TAG,  "requestStkQeryCpinState");

    isReady = isCpinReady();
    logD(RFX_LOG_TAG,  "requestStkQeryCpinState: isCpinReady: %d", isReady);

    result_Data[0] = (isReady == true) ? 1 : 0;

    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxIntsData(result_Data, 1), msg, false);
    // response to TeleCore
    responseToTelCore(response);
}

void RmcCatCommonRequestHandler::requestStkSendEnvelopeCommand (const sp<RfxMclMessage>& msg)
{
    char* cmd = NULL;
    char* data = (char *)msg->getData()->getData();
    sp<RfxAtResponse> p_response;
    RIL_Errno rilErrNo = RIL_E_SUCCESS;

     if (getMclStatusManager()->getIntValue(RFX_STATUS_KEY_CARD_TYPE) <=  0) {
         rilErrNo = RIL_E_SIM_ABSENT;
         sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), rilErrNo,
                 RfxStringData(), msg, false);
          // response to TeleCore
         responseToTelCore(response);
         return;
    }

    // From this version, use AT+CUSATE instead of AT+STKENV
    data = (data == NULL) ? ((char *)("")) : data;
    asprintf(&cmd, "AT+CUSATE=\"%s\"", data);

    // send AT command
    p_response = atSendCommand(cmd);

    free(cmd);
    cmd = NULL;

    // set result
    if (p_response->getSuccess() != 1) {
        rilErrNo = RIL_E_GENERIC_FAILURE;
    }

    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), rilErrNo,
            RfxStringData(), msg, false);

    // response to TeleCore
    responseToTelCore(response);
}

void RmcCatCommonRequestHandler::requestStkSendTerminalResponse (const sp<RfxMclMessage>& msg) {
    char* cmd = NULL;
    char* data = (char *)msg->getData()->getData();
    sp<RfxAtResponse> p_response;
    RIL_Errno rilErrNo = RIL_E_SUCCESS;
    int cmdId = -1;

    data = (data == NULL) ? ((char *)("")) : data;
    cmdId = getMclStatusManager()->getIntValue(RFX_STATUS_KEY_STK_CMD_ID);
    if (cmdId < 0) {
        asprintf(&cmd, "AT+STKTR=\"%s\"", data);
    } else {
        asprintf(&cmd, "AT+STKTR=\"%s\",%d", data, cmdId);
    }

    // send AT command
    p_response = atSendCommand(cmd);

    free(cmd);
    cmd = NULL;

    // set result
    if (p_response->getSuccess() != 1) {
        rilErrNo = RIL_E_GENERIC_FAILURE;
    }

    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), rilErrNo,
            RfxVoidData(), msg, false);

    // response to TeleCore
    responseToTelCore(response);
}

void RmcCatCommonRequestHandler::requestStkHandleCallSetupRequestedFromSim (const sp<RfxMclMessage>& msg) {
    char* cmd = NULL;
    int user_confirm = 0;
    int addtional_info = 0;
    RfxIntsData *intsData = (RfxIntsData*)msg->getData();
    int *data = (int*)intsData->getData();
    sp<RfxAtResponse> p_response;
    RIL_Errno rilErrNo = RIL_E_SUCCESS;
    int radio_state = getMclStatusManager()->getIntValue(RFX_STATUS_KEY_RADIO_STATE);

    logD(RFX_LOG_TAG,  "requestStkHandleCallSetupRequestedFromSim");

    if (data[0] == 1) {
        if (RADIO_STATE_UNAVAILABLE == radio_state || RADIO_STATE_OFF == radio_state) {
            logD(RFX_LOG_TAG, "radio state is off or unavailable, no send AT+STKCALL = 0");
            user_confirm = 32;
            addtional_info = 4;
        } else {
            user_confirm = 0;
        }
    } else if (data[0] == 32) {
        user_confirm = 32;
        addtional_info = 2;
    } else if (data[0] == 33) {
        user_confirm = 33;
        addtional_info = 0x9d;
    } else if (data[0] == 0) {
        user_confirm = 34;
    } else {
        assert(0);
    }

    if( addtional_info == 0) {
        asprintf(&cmd, "AT+STKCALL=%d", user_confirm);
    } else {
        asprintf(&cmd, "AT+STKCALL=%d, %d", user_confirm, addtional_info);
    }

    // send AT command
    p_response = atSendCommand(cmd);

    free(cmd);
    cmd = NULL;

    // set result
    if (p_response->getSuccess() != 1) {
        rilErrNo = RIL_E_GENERIC_FAILURE;
    }

    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), rilErrNo,
            RfxVoidData(), msg, false);

    // response to TeleCore
    responseToTelCore(response);
}

void RmcCatCommonRequestHandler::onHandleTimer() {
    // do something
}


void RmcCatCommonRequestHandler::requestStkSendEnvelopeCommandWithStatus (const sp<RfxMclMessage>& msg) {
    char* cmd = NULL;
    char* line = NULL;
    char* data = (char *)msg->getData()->getData();
    bool headIntermediate = true;
    int err = -1;
    sp<RfxAtResponse> p_response;
    RIL_Errno rilErrNo = RIL_E_SUCCESS;
    RIL_SIM_IO_Response sr;
    RfxAtLine *atLine = NULL;

    sp<RfxMclMessage> response_ok;
    sp<RfxMclMessage> response_error;
    memset(&sr, 0, sizeof(sr));

     if (getMclStatusManager()->getIntValue(RFX_STATUS_KEY_CARD_TYPE) <=  0) {
         rilErrNo = RIL_E_SIM_ABSENT;
        goto error;
    }

    data = (data == NULL) ? ((char *)("")) : data;
    asprintf(&cmd, "AT+CUSATE=\"%s\"", data);

    // send AT command
    p_response = atSendCommandRaw(cmd);

    free(cmd);
    cmd = NULL;

    // set result
    if (p_response->getSuccess() != 1) {
        rilErrNo = RIL_E_GENERIC_FAILURE;
        goto error;
    }

    atLine = p_response->getIntermediates();
    for (atLine; atLine != NULL; atLine = atLine->getNext()) {
        line = atLine->getLine();
        if (NULL == line) {
            logE(RFX_LOG_TAG,  "requestStkSendEnvelopeCommand ok but no intermediates.");
            goto done;
        /*
            sp<RfxMclMessage> response_ok = RfxMclMessage::obtainResponse(msg->getId(), rilErrNo,
            RfxSimIoRspData((void*)&sr, sizeof(sr)), msg, true);

            // response to TeleCore
            responseToTelCore(response_ok);
            p_response = NULL;
            if (sr.simResponse != NULL) {
                free(sr.simResponse);
            }
            //To do: goto error fix
            return;*/
        }

        if (headIntermediate) {
            headIntermediate = false;
            logD(RFX_LOG_TAG,  "requestStkSendEnvelopeCommand CUSATE [%s].", line);

            atLine->atTokStart(&err);
            if (err < 0) {
                logE(RFX_LOG_TAG,  "get +CUSATE: error.");
                goto error;
            }
            sr.simResponse = atLine->atTokNextstr(&err);
            if (err < 0) {
                logE(RFX_LOG_TAG,  "response data is null.");
                goto error;
            }
        } else {
            logD(RFX_LOG_TAG,  "CUSATE2,[%s]", line);

            atLine->atTokStart(&err);
            if (err < 0) {
                logE(RFX_LOG_TAG,  "get +CUSATE2: error.");
                goto error;
            }
            sr.sw1 = atLine->atTokNextint(&err);
            if (err < 0) {
                logE(RFX_LOG_TAG,  "get +CUSATE2: sr.sw1 error.");
                goto error;
            }
            sr.sw2 = atLine->atTokNextint(&err);
            if (err < 0) {
                logE(RFX_LOG_TAG,  "get +CUSATE2: sr.sw2 error.");
                goto error;
            }
            logD(RFX_LOG_TAG,  "requestStkSendEnvelopeCommand sw: %02x, %02x", sr.sw1, sr.sw2);
        }
    }

done:
    response_ok = RfxMclMessage::obtainResponse(msg->getId(), rilErrNo,
            RfxSimIoRspData((void*)&sr, sizeof(sr)), msg, false);

    // response to TeleCore
    responseToTelCore(response_ok);
    p_response = NULL;
    //To do: goto error fix
    return;

error:
    response_error = RfxMclMessage::obtainResponse(msg->getId(), rilErrNo,
            RfxSimIoRspData(NULL, 0), msg, false);

    // response to TeleCore
    responseToTelCore(response_error);
    p_response = NULL;
    return;
}

int RmcCatCommonRequestHandler::getInCallNumber() {
    int inCallNumber = 0;
    inCallNumber = getMclStatusManager()->getIntValue(RFX_STATUS_KEY_VOICE_CALL_COUNT, 0);
    RFX_LOG_E(RFX_LOG_TAG, "getInCallNumber: %d ", inCallNumber);
    return inCallNumber;
}

bool RmcCatCommonRequestHandler::isCpinReady() {
    int err;
    int query_result[1];
    query_result[0] = 0;
    RfxAtLine *line;
    char *cpinResult = NULL;
    sp<RfxAtResponse> p_response = NULL;

    logD(RFX_LOG_TAG, "isCpinReady: stk detect cpin state at slot %d", m_slot_id);

    p_response = atSendCommandSingleline("AT+CPIN?", "+CPIN:");
    err = p_response->getError();
    if ((err != 0) || (p_response->getSuccess() == 0)) {
        logE(RFX_LOG_TAG, "isCpinReady: stk detectSim fail at slot %d and need retry later", m_slot_id);
        return false;
    }
    line = p_response->getIntermediates();
    line->atTokStart(&err);
    if (err < 0) {
        logE(RFX_LOG_TAG, "isCpinReady: atTokStart: err: %d and need retry later", err);
        return false;
    }
    cpinResult = line->atTokNextstr(&err);
    if (err < 0) {
        logE(RFX_LOG_TAG,  "isCpinReady: atTokNextstr: err: %d and need retry later", err);
        return false;
    }
    String8 cpinStr(cpinResult);
    if (cpinStr != String8::format("READY"))  {
        logE(RFX_LOG_TAG,  "stk query cpin state at slot %d: fail and need retry later", m_slot_id);
        return false;
    } else {
        logD(RFX_LOG_TAG,  "stk query cpin state at slot %d: success", m_slot_id);
        return true;
    }
}

void RmcCatCommonRequestHandler::requestStkSendResponseByCmdType (const sp<RfxMclMessage>& msg) {
    char* cmd = NULL;
    int cmd_type = 0;
    int cmd_qualifier = 0;
    bool err_flag = true;
    RfxIntsData *intsData = NULL;
    int *data = NULL;
    sp<RfxAtResponse> p_response;
    RIL_Errno rilErrNo = RIL_E_SUCCESS;

    logD(RFX_LOG_TAG,  "requestStkSendResponseByCmdType");

    intsData = (RfxIntsData*)msg->getData();
    if (intsData != NULL) {
        data = (int*)intsData->getData();
        if (data != NULL) {
            cmd_type = data[0];
            cmd_qualifier = data[1];
            err_flag = false;
        }
    }

    if (err_flag) {
        //For parsing error, we just send response to telcore
        rilErrNo = RIL_E_GENERIC_FAILURE;
        sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), rilErrNo,
            RfxVoidData(), msg, false);
        responseToTelCore(response);
        return;
    }

    logD(RFX_LOG_TAG,  "requestStkSendResponseByCmdType: cmd_type = %d, cmd_qual = %d .",
        cmd_type, cmd_qualifier);

    if (CMD_SETUP_CALL == cmd_type) {
        asprintf(&cmd, "AT+STKCALL=%d", 34);
    } else if (CMD_SETUP_MENU == cmd_type || CMD_IDLE_MODEL_TXT == cmd_type) {
        asprintf(&cmd, "AT+STKTR=\"810301%02x%02x82028281830100\"", cmd_type, cmd_qualifier);
    } else {
        asprintf(&cmd, "AT+STKTR=\"810301%02x%02x82028281830110\"", cmd_type, cmd_qualifier);
    }

    // send AT command
    p_response = atSendCommand(cmd);

    free(cmd);
    cmd = NULL;
    // set result
    if (p_response->getSuccess() != 1) {
        rilErrNo = RIL_E_GENERIC_FAILURE;
    }

    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), rilErrNo,
            RfxVoidData(), msg, false);
    // response to TeleCore
    responseToTelCore(response);
}

// BIP @{
void RmcCatCommonRequestHandler::requestBipSendConfirmInfo (const sp<RfxMclMessage>& msg) {
    char* cmd = NULL;
    int cmd_num = 0;
    int result_code = 0;
    bool err_flag = true;
    RfxIntsData *intsData = NULL;
    int *data = NULL;
    sp<RfxAtResponse> p_response;
    RIL_Errno rilErrNo = RIL_E_SUCCESS;

    logD(RFX_LOG_TAG,  "requestBipSendConfirmInfo");

    intsData = (RfxIntsData*)msg->getData();
    if (intsData != NULL) {
        data = (int*)intsData->getData();
        if (data != NULL) {
            cmd_num = data[0];
            //Mapping result code according to spec here
            //Currently we can get 1 or 0 from framework
            result_code = (data[1] == 1) ? RESULT_OK : RESULT_USER_NOT_ACCEPT;
            err_flag = false;
        }
    }

    if (err_flag) {
        //For parsing error, we set cmd_num to default 1, and result_code to 0x3a
        cmd_num = 1;
        result_code = RESULT_BIP_ERROR;
    }

    logD(RFX_LOG_TAG,  "requestBipSendConfirmInfo: cmd_num = %d, result_code = %d .", cmd_num, result_code);

    asprintf(&cmd, "AT+BIPCONF=%d,%d", cmd_num, result_code);

    // send AT command
    p_response = atSendCommand(cmd);

    free(cmd);
    cmd = NULL;
    // set result
    if (p_response->getSuccess() != 1) {
        rilErrNo = RIL_E_GENERIC_FAILURE;
    }

    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), rilErrNo,
            RfxVoidData(), msg, false);
    // response to TeleCore
    responseToTelCore(response);
}
// BIP @}

//BTSAP @{
void RmcCatCommonRequestHandler::requestBtSapTransferCardReaderStatus(const sp<RfxMclMessage>& msg) {
    RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_REQ *req = NULL;
    RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_RSP rsp;
    //sp<RfxAtResponse> p_response;
    int status = getMclStatusManager()->getIntValue(RFX_STATUS_KEY_BTSAP_STATUS);
    void *data = msg->getData()->getData();
    int datalen = msg->getData()->getDataLength();

    logD(RFX_LOG_TAG, "[BTSAP] requestBtSapTransferCardReaderStatus start, BTSAP status: %d .", status);
    req = (RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_REQ*)calloc(1, sizeof(RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_REQ));

    rsp.CardReaderStatus = BT_SAP_CARDREADER_RESPONSE_DEFAULT;
    rsp.response = RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_RSP_Response_RIL_E_SUCCESS;
    rsp.has_CardReaderStatus = true;   //  always true

    if (getMclStatusManager()->getIntValue(RFX_STATUS_KEY_CARD_TYPE) > 0) {
        logD(RFX_LOG_TAG, "[BTSAP] requestBtSapGetCardStatus, Sim inserted .");
        rsp.CardReaderStatus = rsp.CardReaderStatus | BT_SAP_CARDREADER_RESPONSE_SIM_INSERT;
    } else {
        logD(RFX_LOG_TAG, "[BTSAP] requestBtSapGetCardStatus, But sim not inserted");
        rsp.response = RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_RSP_Response_RIL_E_GENERIC_FAILURE;
        sendStkBtSapResponseComplete(msg, RIL_E_GENERIC_FAILURE,
                RFX_MSG_REQUEST_SIM_SAP_TRANSFER_CARD_READER_STATUS, &rsp);
        if (req != NULL) {
            free(req);
        }
        return;
    }

    if (status == BT_SAP_CONNECTION_SETUP || status == BT_SAP_ONGOING_CONNECTION
        || status == BT_SAP_POWER_ON) {
        rsp.CardReaderStatus = rsp.CardReaderStatus | BT_SAP_CARDREADER_RESPONSE_READER_POWER;
    } else {
        // For BT_SAP_INIT, BT_SAP_DISCONNECT and BT_SAP_POWER_OFF, return generic fail
        rsp.response = RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_RSP_Response_RIL_E_GENERIC_FAILURE;
    }

    logD(RFX_LOG_TAG, "[BTSAP] requestBtSapGetCardStatus, CardReaderStatus result : %x .", rsp.CardReaderStatus);

    sendStkBtSapResponseComplete(msg, RIL_E_SUCCESS, RFX_MSG_REQUEST_SIM_SAP_TRANSFER_CARD_READER_STATUS, &rsp);

    free(req);
    logD(RFX_LOG_TAG, "[BTSAP] requestBtSapGetCardStatus end .");
}

void RmcCatCommonRequestHandler::sendStkBtSapResponseComplete(const sp<RfxMclMessage>& msg,
        RIL_Errno ret, int msgId, void *data) {
    const pb_field_t *fields = NULL;
    size_t encoded_size = 0;
    uint32_t written_size = 0;
    size_t buffer_size = 0;
    pb_ostream_t ostream;
    bool success = false;
    ssize_t written_bytes;
    int i = 0;
    sp<RfxMclMessage> response = NULL;

    logD(RFX_LOG_TAG, "[BTSAP] sendStkBtSapResponseComplete, start .");
    switch (msgId) {
        case RFX_MSG_REQUEST_SIM_SAP_TRANSFER_CARD_READER_STATUS:
            fields = RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_RSP_fields;
            break;
        default:
            logD(RFX_LOG_TAG, "[BTSAP] sendStkBtSapResponseComplete, MsgId is mistake!");
            return;
    }

    if ((success = pb_get_encoded_size(&encoded_size, fields, data)) &&
            encoded_size <= INT32_MAX) {
        buffer_size = encoded_size;
        uint8_t buffer[buffer_size];
        ostream = pb_ostream_from_buffer(buffer, buffer_size);
        success = pb_encode(&ostream, fields, data);

        if(success) {
            logD(RFX_LOG_TAG, "[BTSAP] sendStkBtSapResponseComplete, Size: %zu (0x%zx) Size as written: 0x%x",
                encoded_size, encoded_size, written_size);
            response = RfxMclMessage::obtainSapResponse(msgId, ret,
                    RfxRawData((void*)buffer, buffer_size), msg, false);
            responseToTelCore(response);
        } else {
            logD(RFX_LOG_TAG, "[BTSAP] sendStkBtSapResponseComplete, Encode failed!");
        }
    } else {
        logD(RFX_LOG_TAG, "Not sending response type %d: encoded_size: %zu. encoded size result: %d",
            msgId, encoded_size, success);
    }
}
// BTSAP @}
