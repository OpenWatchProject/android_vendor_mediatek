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
#include "RtcCatController.h"
#include "RfxMainThread.h"
#include <mtk_properties.h>

#define RFX_LOG_TAG "RtcCatController"

/*****************************************************************************
 * Class RtcCatController
 *****************************************************************************/

RFX_IMPLEMENT_CLASS("RtcCatController", RtcCatController, RfxController);

// register request and urc to RfxData

RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringData, RfxStringData, \
    RFX_MSG_REQUEST_STK_SEND_ENVELOPE_COMMAND);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringData, RfxVoidData, \
    RFX_MSG_REQUEST_STK_SEND_TERMINAL_RESPONSE);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, \
    RFX_MSG_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, \
    RFX_MSG_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM_WITH_RESULT_CODE);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxVoidData, \
    RFX_MSG_REQUEST_REPORT_STK_SERVICE_IS_RUNNING);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringData, RfxSimIoRspData, \
    RFX_MSG_REQUEST_STK_SEND_ENVELOPE_WITH_STATUS);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, \
    RFX_MSG_REQUEST_STK_SEND_RESPONSE_BY_CMDTYPE);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, \
    RFX_MSG_REQUEST_STK_EVENT_NOTIFY);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxIntsData, \
    RFX_MSG_REQUEST_STK_QUERY_CPIN_STATE);
// BIP @{
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, \
    RFX_MSG_REQUEST_BIP_SEND_CONFIRM_INFO);
// BIP @}
// BTSAP @{
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxRawData, RfxRawData, \
    RFX_MSG_REQUEST_SIM_SAP_TRANSFER_CARD_READER_STATUS);
// BTSAP @}
// End
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, \
    RFX_MSG_REQUEST_USSD_DOMAIN_INFO_ACK);

RFX_REGISTER_DATA_TO_URC_ID(RfxStringData, RFX_MSG_URC_STK_PROACTIVE_COMMAND);
RFX_REGISTER_DATA_TO_URC_ID(RfxStringData, RFX_MSG_URC_STK_EVENT_NOTIFY);
RFX_REGISTER_DATA_TO_URC_ID(RfxVoidData, RFX_MSG_URC_STK_SESSION_END);
RFX_REGISTER_DATA_TO_URC_ID(RfxIntsData, RFX_MSG_URC_STK_CALL_SETUP);
RFX_REGISTER_DATA_TO_URC_ID(RfxSimRefreshData, RFX_MSG_URC_SIM_REFRESH);
RFX_REGISTER_DATA_TO_URC_ID(RfxStringData, RFX_MSG_URC_STK_CC_ALPHA_NOTIFY);  //maybe modify
RFX_REGISTER_DATA_TO_URC_ID(RfxVoidData, RFX_MSG_URC_STK_SETUP_MENU_RESET);  //maybe modify
RFX_REGISTER_DATA_TO_URC_ID(RfxStringsData, RFX_MSG_URC_STK_CALL_CTRL);  //maybe modify
RFX_REGISTER_DATA_TO_URC_ID(RfxVoidData, RFX_MSG_URC_STK_TRIGGER_OTASP);
RFX_REGISTER_DATA_TO_URC_ID(RfxIntsData,RFX_MSG_URC_STK_CPIN_STATE);




RtcCatController::RtcCatController() :
    mCardType(-1) {
        mCPinRetry = 0;
        mTimeoutHandle = NULL;
        isStkServiceRunning = false;
        isProaCmdQueued = false;
        isEventNotifyQueued = false;
        pProactive_cmd = NULL;
        pCachedMenu = NULL;
        pSimRefreshRspWithType = NULL;
        mProCmdRec.cmdType = 0;
        mProCmdRec.cmdQualifier = 0;
        mProCmdRec.needResponse = false;
        for (int i = 0; i< EVENT_NOTIFY_BUFFER_LEN; i++) {
            pEvent_notify[i] = NULL;
        }
}

RtcCatController::~RtcCatController() {
    if (mTimeoutHandle != NULL) {
        RfxTimer::stop(mTimeoutHandle);
        mTimeoutHandle = NULL;
    }
    freeAllStkQueuedEventNotify();
    freeStkQueuedProactivCmd();
    if (NULL != pSimRefreshRspWithType) {
        if (NULL != pSimRefreshRspWithType->simRefreshRsp) {
            if (NULL!= pSimRefreshRspWithType->simRefreshRsp->aid) {
                free(pSimRefreshRspWithType->simRefreshRsp->aid);
                pSimRefreshRspWithType->simRefreshRsp->aid = NULL;
            }
            free(pSimRefreshRspWithType->simRefreshRsp);
            pSimRefreshRspWithType->simRefreshRsp = NULL;
        }
        if (NULL != pSimRefreshRspWithType->efId) {
            free(pSimRefreshRspWithType->efId);
            pSimRefreshRspWithType->efId = NULL;
        }
        free(pSimRefreshRspWithType);
        pSimRefreshRspWithType = NULL;
    }
}

void RtcCatController::onInit() {
    RfxController::onInit();  // Required: invoke super class implementation

    isProaCmdQueued = false;
    isStkServiceRunning = false;
    resetProCmd();

    const int request_id_list[] = {
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
        RFX_MSG_REQUEST_SIM_SAP_TRANSFER_CARD_READER_STATUS,
        // BTSAP @}
        RFX_MSG_REQUEST_USSD_DOMAIN_INFO_ACK
    };

    const int urc_id_list[] = {
        RFX_MSG_URC_STK_SESSION_END,
        RFX_MSG_URC_STK_PROACTIVE_COMMAND,
        RFX_MSG_URC_STK_EVENT_NOTIFY,
        RFX_MSG_URC_STK_CALL_SETUP,
        RFX_MSG_URC_SIM_REFRESH,
        RFX_MSG_URC_STK_CC_ALPHA_NOTIFY,
        RFX_MSG_URC_STK_SETUP_MENU_RESET,
        RFX_MSG_URC_STK_CALL_CTRL,
        RFX_MSG_URC_STK_TRIGGER_OTASP,
        RFX_MSG_URC_STK_CPIN_STATE,
    };

    // register request & URC id list
    // NOTE. one id can only be registered by one controller
    registerToHandleRequest(request_id_list, sizeof(request_id_list)/sizeof(const int));
    registerToHandleUrc(urc_id_list, sizeof(urc_id_list)/sizeof(const int));

    // Register callbacks for card type
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_CARD_TYPE,
        RfxStatusChangeCallback(this, &RtcCatController::onCardTypeChanged));

    // register callbacks to get required information
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_RADIO_STATE,
        RfxStatusChangeCallback(this, &RtcCatController::onRadioStateChanged));
}

void RtcCatController::onCardTypeChanged(RfxStatusKeyEnum key,
    RfxVariant oldValue, RfxVariant newValue) {
    RFX_UNUSED(key);
    //bool isStkServiceRunning = false;
    //int cmdType = 0;
    int oldState = -1, newState = -1;

    oldState = oldValue.asInt();
    newState = newValue.asInt();

    mCardType = getStatusManager()->getIntValue(RFX_STATUS_KEY_CARD_TYPE);
    if ((oldState > 0) && (newState <= 0)) {
        logD(RFX_LOG_TAG, "onCardTypeChanged  cmdType 0x%02X, cmdQual %d",
                mProCmdRec.cmdType, mProCmdRec.cmdQualifier);
        if (mProCmdRec.needResponse && mProCmdRec.cmdType != 0) {
            sendResponseForCardMissing(mProCmdRec.cmdType, mProCmdRec.cmdQualifier);
            resetProCmd();
        }

        logD(RFX_LOG_TAG, "card missing and reset the RFX_STATUS_KEY_IS_CAT_RUNNING");
        isStkServiceRunning = false;
        freeAllStkQueuedEventNotify();
        freeStkQueuedProactivCmd();
        freeCachedMenu();
        getStatusManager()->setBoolValue(RFX_STATUS_KEY_IS_CAT_RUNNING, false);
    }
}

void RtcCatController::onRadioStateChanged(RfxStatusKeyEnum key,
    RfxVariant old_value, RfxVariant value) {
    int oldState = -1, newState = -1;

    RFX_UNUSED(key);
    oldState = old_value.asInt();
    newState = value.asInt();

    if (newState == RADIO_STATE_UNAVAILABLE) {
        //For radio unavailable, CatService will be created when sim ready
        if (mProCmdRec.needResponse && mProCmdRec.cmdType != 0) {
            sendResponseForCardMissing(mProCmdRec.cmdType, mProCmdRec.cmdQualifier);
            resetProCmd();
        }

        isStkServiceRunning = false;
        freeAllStkQueuedEventNotify();
        freeStkQueuedProactivCmd();
        getStatusManager()->setBoolValue(RFX_STATUS_KEY_IS_CAT_RUNNING, false);
    }
}

void RtcCatController::onDeinit() {
    logD(RFX_LOG_TAG, "RtcCatController: onDeinit ");

    // Unregister callbacks for card type
    getStatusManager()->unRegisterStatusChanged(RFX_STATUS_KEY_CARD_TYPE,
            RfxStatusChangeCallback(this, &RtcCatController::onCardTypeChanged));

    getStatusManager()->unRegisterStatusChanged(RFX_STATUS_KEY_RADIO_STATE,
            RfxStatusChangeCallback(this, &RtcCatController::onRadioStateChanged));
    // Required: invoke super class implementation
    RfxController::onDeinit();
}

bool RtcCatController::onHandleRequest(const sp<RfxMessage>& message) {
    int msg_id = message->getId();
    int cmdType = 0;
    int resCode = 0;
    bool isIgnoredCmd = false;

    logD(RFX_LOG_TAG, "onHandleRequest: msg_id=%d is %s", msg_id, RFX_ID_TO_STR(msg_id));

    switch (msg_id) {
        case RFX_MSG_REQUEST_STK_SEND_ENVELOPE_COMMAND:
            //Filter and ignore the command which has handled by modem
            isIgnoredCmd = isIgnoredEnvelopeCommand(message);
            if (isIgnoredCmd) {
                logD(RFX_LOG_TAG, "onHandleRequest: ignore the cmd which is handled by modem");
                sp<RfxMessage> rsp =
                        RfxMessage::obtainResponse(RIL_E_SUCCESS, message, false);
                responseToRilj(rsp);
                break;
            }
            requestToMcl(message);
            break;
        case RFX_MSG_REQUEST_STK_SEND_TERMINAL_RESPONSE:
            parseStkCmdType(message, &cmdType);

            logD(RFX_LOG_TAG, "mProCmdRec: cmdType:0x%02X, current response type:0x%02X ",
                mProCmdRec.cmdType, cmdType);

            if (0 != mProCmdRec.cmdType && mProCmdRec.cmdType == cmdType && mProCmdRec.needResponse) {
                requestToMcl(message);
                resetProCmd();
            //Setup Event list may cover current cmd type and cause response block
            } else if (CMD_SETUP_EVENT_LIST == cmdType) {
                //requestToMcl(message);
                //resetProCmd();
                sp<RfxMessage> rsp =
                            RfxMessage::obtainResponse(RIL_E_SUCCESS, message, false);
                responseToRilj(rsp);
            // BIP @{
            } else if (CMD_OPEN_CHAN == cmdType) {
                parseStkResultCode(message, &resCode);
                if (resCode >= 0) {
                    int bip_Data[2];
                    bip_Data[0] = mProCmdRec.cmdNum;
                    bip_Data[1] = resCode;
                    sp<RfxMessage> msg = RfxMessage::obtainRequest(m_slot_id,
                            RFX_MSG_REQUEST_BIP_SEND_CONFIRM_INFO, RfxIntsData(bip_Data, 2));
                    requestToMcl(msg);

                    sp<RfxMessage> rsp =
                            RfxMessage::obtainResponse(RIL_E_SUCCESS, message, false);
                    responseToRilj(rsp);
                } else {
                    logE(RFX_LOG_TAG, "parse bip result code fail");
                    sp<RfxMessage> rsp =
                            RfxMessage::obtainResponse(RIL_E_GENERIC_FAILURE, message, false);
                    responseToRilj(rsp);
                }
                resetProCmd();
            // BIP @}
            } else {
                logD(RFX_LOG_TAG, "Not Expected TR. Return directly!!!");
                sp<RfxMessage> rsp =
                        RfxMessage::obtainResponse(RIL_E_SUCCESS, message, false);
                responseToRilj(rsp);
            }
            break;
        case RFX_MSG_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM:
        case RFX_MSG_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM_WITH_RESULT_CODE:
            if (CMD_SETUP_CALL == mProCmdRec.cmdType) {
                //note: For bianco, it is no used to config audio path
                if (false == mProCmdRec.needResponse) {
                    logD(RFX_LOG_TAG, "Not need to send call request, just return!");
                    sp<RfxMessage> rsp =
                            RfxMessage::obtainResponse(RIL_E_SUCCESS, message, false);
                    responseToRilj(rsp);
                } else {
                    requestToMcl(message);
                }
            // BIP @{
            } else if (CMD_OPEN_CHAN == mProCmdRec.cmdType) {
                RfxIntsData *intsData = (RfxIntsData*)message->getData();
                int *data = NULL;
                if (intsData != NULL) {
                    data = (int*)intsData->getData();
                    logD(RFX_LOG_TAG, "parse bip result code: %d ", data[0]);
                    int bip_Data[2];
                    bip_Data[0] = mProCmdRec.cmdNum;
                    bip_Data[1] = data[0];
                    sp<RfxMessage> msg = RfxMessage::obtainRequest(m_slot_id,
                            RFX_MSG_REQUEST_BIP_SEND_CONFIRM_INFO, RfxIntsData(bip_Data, 2));
                    requestToMcl(msg);

                    sp<RfxMessage> rsp =
                            RfxMessage::obtainResponse(RIL_E_SUCCESS, message, false);
                    responseToRilj(rsp);
                } else {
                    logE(RFX_LOG_TAG, "parse bip result code fail");
                    sp<RfxMessage> rsp =
                            RfxMessage::obtainResponse(RIL_E_GENERIC_FAILURE, message, false);
                    responseToRilj(rsp);
                }
            // BIP @}
            } else {
                logD(RFX_LOG_TAG, "Unexpected CALL_SETUP_REQUESTED. Warning!!!");
                requestToMcl(message);
            }
            // reset mProCmdRec
            resetProCmd();
            break;
        case RFX_MSG_REQUEST_REPORT_STK_SERVICE_IS_RUNNING:
            requestToMcl(message);
            mCardType = getStatusManager()->getIntValue(RFX_STATUS_KEY_CARD_TYPE, -1);
            getStatusManager()->setBoolValue(RFX_STATUS_KEY_IS_CAT_RUNNING, true);
            requestHandleStkServiceIsRunning();
            break;
        // BTSAP :{
        case RFX_MSG_REQUEST_SIM_SAP_TRANSFER_CARD_READER_STATUS:
            //Currently we send the envelope cmd directly
            requestToMcl(message);
            break;
        // BTSAP :}
        case RFX_MSG_REQUEST_USSD_DOMAIN_INFO_ACK:
            sendStkUssdConfirmInfo(message);
            break;
        default:
            logE(RFX_LOG_TAG, "Unsupport cmd. Waring!!!");
            requestToMcl(message);
            break;
    }
    return true;
}

bool RtcCatController::onHandleUrc(const sp<RfxMessage>& message) {
    int msg_id = message->getId();
    int cmdType = 0;
    int cmdNum = 0;

    logD(RFX_LOG_TAG,"onHandleUrc: urc id %d is %s", msg_id, RFX_ID_TO_STR(msg_id));
    switch (msg_id) {
        case RFX_MSG_URC_STK_PROACTIVE_COMMAND:
            handleProactiveCommand(message);
            break;
        case RFX_MSG_URC_STK_EVENT_NOTIFY:
            handleEventNotify(message);
            break;
        case RFX_MSG_URC_STK_CPIN_STATE:
            handleSimRefreshAfterQueryCpin();
            break;
        default:
            responseToRilj(message);
            break;
    }
    return true;
}

bool RtcCatController::onHandleResponse(const sp<RfxMessage>& message) {
    int msg_id = message->getId();
    switch (msg_id) {
        case RFX_MSG_REQUEST_REPORT_STK_SERVICE_IS_RUNNING:
            responseToRilj(message);
            break;
        // BIP @{
        case RFX_MSG_REQUEST_BIP_SEND_CONFIRM_INFO:
            logD(RFX_LOG_TAG, "onHandleResponse: for bip response, no need to handle");
            break;
        // BIP @}
        case RFX_MSG_REQUEST_STK_SEND_RESPONSE_BY_CMDTYPE:
            logD(RFX_LOG_TAG, "onHandleResponse: for stk abnormal response, no need to handle");
            break;
        case RFX_MSG_REQUEST_STK_EVENT_NOTIFY:
            logD(RFX_LOG_TAG, "onHandleResponse: for stk event notify response, no need to handle");
            break;
        case RFX_MSG_REQUEST_STK_QUERY_CPIN_STATE:
            handleQueryCpinResponse(message);
            break;
        case RFX_MSG_REQUEST_SIM_SAP_TRANSFER_CARD_READER_STATUS:
            // Send BT directly
            logD(RFX_LOG_TAG, "onHandleResponse: return response to BT");
            responseToBT(message);
            break;
        default:
            responseToRilj(message);
            break;
    }
    return true;
}

int RtcCatController::checkEventNotifyFreeBuffer() {
    int i = 0;
    int freeBufferNum = 0;

    for (i = 0 ; i < EVENT_NOTIFY_BUFFER_LEN ; i++) {
        if (NULL == pEvent_notify[i]) {
            freeBufferNum++;
        }
    }

    logD(RFX_LOG_TAG, "checkEventNotifyFreeBuffer: freeBufferNum:%d", freeBufferNum);
    return freeBufferNum;
}

int RtcCatController::checkStkCmdDisplay(char *cmd_str)
{
    int is_alpha_id_existed = 0;
    int is_icon_existed_not_self_explanatory = 0;
    int index = 0;
    int cmd_length = 0;
    char temp_str[3] = {0};
    char *end;

    cmd_length = strlen(cmd_str);

    while (cmd_length > index) {
        if (cmd_str[index + 1] == '5' && (cmd_str[index] == '0' || cmd_str[index] == '8') ) {

            index += 2;
            if (cmd_str[index] != '0' || cmd_str[index + 1] != '0' ) {
                is_alpha_id_existed = 1;
            }
            if (cmd_str[index] <= '7') {
                memcpy(temp_str, &(cmd_str[index]), 2);
                index += (strtoul(temp_str, &end, 16) + 1) * 2;
            } else {
                memcpy(temp_str, &(cmd_str[index + 2]), 2);
                index += (strtoul(temp_str, &end, 16) + 2) * 2;
            }
        } else if((cmd_str[index + 1] == 'E' || cmd_str[index + 1] == 'e') &&
                                    (cmd_str[index] == '1' || cmd_str[index] == '9')) {
            int icon_qualifier = 0;

            index += 4;
            memset(temp_str, 0, 3);
            memcpy(temp_str, &(cmd_str[index + 1]), 1);
            icon_qualifier = strtoul(temp_str, &end, 16);

            if((icon_qualifier & 0x01) == 0x01) {
                if(is_alpha_id_existed == 0) {
                    return 1;
                }
            }
            index += 4;
        } else {
            index += 2;
            if (cmd_str[index] <= '7') {
                memcpy(temp_str, &(cmd_str[index]), 2);
                index += (strtoul(temp_str, &end, 16) + 1) * 2;
            } else {
                memcpy(temp_str, &(cmd_str[index + 2]), 2);
                index += (strtoul(temp_str, &end, 16) + 2) * 2;
            }
        }
    }
    return 0;
}

int RtcCatController::decodeStkRefreshAid(char *cmd, char **paid)
{
    int offset = 18; //cmd_details & device identifies
    int refresh_length = strlen(cmd) / 2;
    int files_length;
    int files_offset;
    int temp;   // For reading data

    if (offset >= refresh_length) {
        *paid = NULL;
        return 0;
    }

    temp = rild_sms_hexCharToDecInt(&cmd[offset], 2);
    offset += 2;
    if (temp == 0x12 || temp == 0x92) { //file list tag
        temp = rild_sms_hexCharToDecInt(&cmd[offset], 2);   // Length of bytes following (bytes)
        if (temp < 0x7F) {
            offset += (2 + temp * 2);
        } else {
            offset += 2;
            temp = rild_sms_hexCharToDecInt(&cmd[offset], 2);
            offset += (2 + temp * 2);
        }
        temp = rild_sms_hexCharToDecInt(&cmd[offset], 2);   // Number of files
        offset += 2;
    }

    logD(RFX_LOG_TAG, "decodeStkRefreshAid temp = %02x, offset = %d", temp, offset);

    if (temp == 0x2F || temp == 0xAF) { // aid tag
        temp = rild_sms_hexCharToDecInt(&cmd[offset], 2);  // read the length of AID
        offset += 2;
        strncpy(*paid, &cmd[offset], temp * 2);
        return temp * 2;
    }

    return -1;
}

char* RtcCatController::decodeStkRefreshFileChange(char *str, int **cmd, int *cmd_length)
{
    int str_length = 0, file_num = 0, offset = 20, cmdoffset = 0;
    /*offset 20 including cmd_detail tlv: 10, device id tlv:8, file list tag:2*/
    char temp_str[5] = {0};
    char *end;
    str_length = strlen(str);
    char *efId_str;
    int file_idx = 0;

    if(str[offset] <= '7') { //file list length: if length < 7F it will use 2 bytes else it will use 4 bytes
        offset += 2;
    } else {
        offset += 4;
    }
    memcpy(temp_str, str + offset, 2); //copy number of files in file list to temp_str
    offset += 2;

    file_num = strtoul(temp_str, &end, 16);
    efId_str = (char*)calloc(1, (file_num * (sizeof(char) * 4)) + 1);

#ifndef MTK_WIFI_CALLING_RIL_SUPPORT
    *cmd_length = (file_num + 1) * sizeof(int);
    *cmd = (int*)calloc(1, *cmd_length);
#else
    *cmd_length = (file_num + 2) * sizeof(int);
    *cmd = (int*)calloc(1, *cmd_length);
    cmdoffset++;
#endif

    if (*cmd == NULL) {
        logD(RFX_LOG_TAG, "decodeStkRefreshFileChange: calloc error!");
        return efId_str;
    }
    *(*cmd + cmdoffset) = SIM_FILE_UPDATE;

    cmdoffset++;
    logD(RFX_LOG_TAG, "decodeStkRefreshFileChange file_num = %d", file_num);

    while( offset < str_length && file_num > file_idx) {
        if(((str[offset] == '6') || (str[offset] == '2') || (str[offset] == '4'))
           && ((str[offset + 1] == 'F') || (str[offset+1] == 'f'))) {
            memcpy(temp_str, str + offset, 4); //copy EFID to temo_str
            strncat(efId_str, temp_str, 4);
            *(*cmd + cmdoffset) = strtoul(temp_str, &end, 16);

            cmdoffset++;
            file_idx++;
        }
        offset += 4;
    }
    return efId_str;
}

unsigned int RtcCatController::findStkCallDuration(char* str)
{
    int length = 0, offset = 0, temp = 0;
    unsigned int duration = 0;
    char temp_str[3] = {0};
    char *end;

    length = strlen(str);

    while(length > 0) {

        if(str[offset] == '8' && str[offset + 1] == '4') {
            memcpy(temp_str, &(str[offset + 6]), 2);
            if(str[offset + 5] == '0') {
                temp = strtoul(temp_str, &end, 16);
                duration = temp * 60000;
            } else if(str[offset + 5] == '1') {
                temp = strtoul(temp_str, &end, 16);
                duration = temp * 1000;
            } else if(str[offset + 5] == '2') {
                temp = strtoul(temp_str, &end, 16);
                duration = temp * 100;
            }
            break;
        } else {
            length -= 2;
            offset += 2;
            memcpy(temp_str, &(str[offset]), 2);
            temp = strtoul(temp_str, &end, 16);
            length -= (2 * temp + 2);
            offset += (2 * temp + 2);

        }
    }
    return duration;
}

void RtcCatController::freeStkQueuedEventNotify(int index) {
    if (index >= EVENT_NOTIFY_BUFFER_LEN) {
        logE(RFX_LOG_TAG, "freeStkQueuedEventNotify: Invalid index:%d !!!", index);
        return;
    }

    if (NULL != pEvent_notify[index]) {
        free(pEvent_notify[index]);
        pEvent_notify[index] = NULL;
    }
}

void RtcCatController::freeAllStkQueuedEventNotify() {
    for (int i = 0; i < EVENT_NOTIFY_BUFFER_LEN; i++) {
        if (NULL != pEvent_notify[i]) {
            free(pEvent_notify[i]);
            pEvent_notify[i] = NULL;
        }
    }
}

void RtcCatController::freeStkQueuedProactivCmd() {
    if (NULL != pProactive_cmd) {
        free(pProactive_cmd);
        pProactive_cmd = NULL;
    }
}

void RtcCatController::freeCachedMenu() {
    if (NULL != pCachedMenu) {
        free(pCachedMenu);
        pCachedMenu = NULL;
    }
}

int RtcCatController::getStkCommandType(char *cmd) {
    char temp[3] = {0};
    int cmdType = 0;

    strncpy(temp, cmd, 2);
    cmdType = strtoul(temp, NULL, 16);
    cmdType = 0xFF & cmdType;

    return cmdType;
}

char* RtcCatController::getStkQueuedEventNotify(int index) {
    if (index >= EVENT_NOTIFY_BUFFER_LEN) {
        logE(RFX_LOG_TAG, "getStkQueuedEventNotify: Invalid index:%d !!!", index);
        return NULL;
    }

    return pEvent_notify[index];
}
void RtcCatController::handleSimRefresh(char* urc) {
    int aid_len = 0;
    int *cmd = NULL;
    int cmd_length = 0;
    int files_num = 0;
    int i = 0;
    int readIdx = 0;
    int efId = 0;
    char *aid = NULL;
    char *efId_str = NULL;
    char temp_str[5] = {0};   // for read EFID
    int refreshRspMemLen = 0;
    sp<RfxMessage> msg = NULL;
    RIL_SimRefreshResponse_v7 simRefreshRspV7;

    memset(&simRefreshRspV7, 0, sizeof(RIL_SimRefreshResponse_v7));
    aid = (char *)alloca(strlen(urc) + 1);
    logD(RFX_LOG_TAG, "handleSimRefresh: type:%c cmd:%s", urc[9], urc);

    switch(urc[9]) {
        case '0':
            aid_len = decodeStkRefreshAid(urc, &aid);
            simRefreshRspV7.aid = aid;
            simRefreshRspV7.result = SIM_INIT_FULL_FILE_CHANGE;
            break;
        case '1':
            efId_str = decodeStkRefreshFileChange(urc, &cmd , &cmd_length);
            aid_len = decodeStkRefreshAid(urc, &aid);
            simRefreshRspV7.aid = aid;
            simRefreshRspV7.result = SIM_FILE_UPDATE;
            break;
        case '2':
            efId_str = decodeStkRefreshFileChange(urc, &cmd , &cmd_length);
            aid_len = decodeStkRefreshAid(urc, &aid);
            simRefreshRspV7.aid = aid;
            simRefreshRspV7.result = SIM_INIT_FILE_CHANGE;
            break;
        case '3':
            aid_len = decodeStkRefreshAid(urc, &aid);
            simRefreshRspV7.aid = aid;
            simRefreshRspV7.result = SIM_INIT;
            break;
        case '4':
            aid_len = decodeStkRefreshAid(urc, &aid);
            logD(RFX_LOG_TAG, "handleSimRefresh: aid = %s, %d", aid, aid_len);
            pSimRefreshRspWithType = (SimRefreshResponse*)malloc(sizeof(SimRefreshResponse));
            if (pSimRefreshRspWithType == NULL) {
                logD(RFX_LOG_TAG, "SimRefreshResponse malloc fail");
                return;
            }
            memset(pSimRefreshRspWithType, 0, sizeof(SimRefreshResponse));
            pSimRefreshRspWithType->simRefreshRsp =
                (RIL_SimRefreshResponse_v7*)malloc(sizeof(RIL_SimRefreshResponse_v7));
            if (pSimRefreshRspWithType->simRefreshRsp == NULL) {
                logD(RFX_LOG_TAG, "SimRefreshResponse->simRefreshRsp malloc fail");
                free(pSimRefreshRspWithType);
                pSimRefreshRspWithType = NULL;
                return;
            }
            memset(pSimRefreshRspWithType->simRefreshRsp, 0, sizeof(RIL_SimRefreshResponse_v7));
            if (NULL != aid) {
                pSimRefreshRspWithType->simRefreshRsp->aid = (char*)malloc(strlen(aid) + 1);
                if (pSimRefreshRspWithType->simRefreshRsp->aid == NULL) {
                    logD(RFX_LOG_TAG, "SimRefreshResponse->simRefreshRsp->aid malloc fail");
                    free(pSimRefreshRspWithType->simRefreshRsp);
                    free(pSimRefreshRspWithType);
                    pSimRefreshRspWithType = NULL;
                    return;
                }
                strncpy(pSimRefreshRspWithType->simRefreshRsp->aid, aid, strlen(aid));
                pSimRefreshRspWithType->simRefreshRsp->aid[strlen(aid)] = '\0';
            }
            pSimRefreshRspWithType->simRefreshRsp->result = SIM_RESET;

            if (aid != NULL) {
                getStatusManager()->setString8Value(RFX_STATUS_KEY_UICC_RESET_APPLICATION,
                        String8(aid), true);
            } else {
                getStatusManager()->setString8Value(RFX_STATUS_KEY_UICC_RESET_APPLICATION,
                        String8(""), true);
            }

            msg = RfxMessage::obtainRequest(m_slot_id,
                            RFX_MSG_REQUEST_STK_QUERY_CPIN_STATE, RfxVoidData());
            requestToMcl(msg);
            return;
        case '5': // ISIM app_reset
            aid_len = decodeStkRefreshAid(urc, &aid);
            simRefreshRspV7.aid = aid;
            simRefreshRspV7.result = SIM_INIT;
            break;
        case '6':
            efId_str = decodeStkRefreshFileChange(urc, &cmd , &cmd_length);
            logD(RFX_LOG_TAG, "handleSimRefresh: efId = %s", efId_str);
            aid_len = decodeStkRefreshAid(urc, &aid);
            logD(RFX_LOG_TAG, "handleSimRefresh: aid = %s, %d", aid, aid_len);

            pSimRefreshRspWithType = (SimRefreshResponse*) malloc(sizeof(SimRefreshResponse));
            if (pSimRefreshRspWithType == NULL) {
                logD(RFX_LOG_TAG, "pSimRefreshRspWithType malloc fail");
                if (NULL != cmd) {
                    free(cmd);
                }
                if (NULL != efId_str) {
                    free(efId_str);
                }
                return;
            }
            memset(pSimRefreshRspWithType, 0, sizeof(SimRefreshResponse));
            pSimRefreshRspWithType->simRefreshRsp = (RIL_SimRefreshResponse_v7*) malloc(
                    sizeof(RIL_SimRefreshResponse_v7));
            if (pSimRefreshRspWithType->simRefreshRsp == NULL) {
                logD(RFX_LOG_TAG, "SimRefreshResponse->simRefreshRsp malloc fail");
                free(pSimRefreshRspWithType);
                pSimRefreshRspWithType = NULL;
                if (NULL != cmd) {
                    free(cmd);
                }
                if (NULL != efId_str) {
                    free(efId_str);
                }
                return;
            }
            memset(pSimRefreshRspWithType->simRefreshRsp, 0, sizeof(RIL_SimRefreshResponse_v7));
            if (aid != NULL) {
                pSimRefreshRspWithType->simRefreshRsp->aid = (char*) malloc(strlen(aid) + 1);
                if (pSimRefreshRspWithType->simRefreshRsp->aid == NULL) {
                    logD(RFX_LOG_TAG, "SimRefreshResponse->simRefreshRsp->aid malloc fail");
                    free(pSimRefreshRspWithType->simRefreshRsp);
                    free(pSimRefreshRspWithType);
                    pSimRefreshRspWithType = NULL;
                    if (NULL != cmd) {
                        free(cmd);
                    }
                    if (NULL != efId_str) {
                        free(efId_str);
                    }
                    return;
                }
                strncpy(pSimRefreshRspWithType->simRefreshRsp->aid, aid, strlen(aid));
                pSimRefreshRspWithType->simRefreshRsp->aid[strlen(aid)] = '\0';
            }
            if (efId_str != NULL) {
                pSimRefreshRspWithType->efId = (char*) malloc(strlen(efId_str) + 1);
                if (pSimRefreshRspWithType->efId == NULL) {
                    logD(RFX_LOG_TAG, "SimRefreshResponse->efId malloc fail");
                    if (pSimRefreshRspWithType->simRefreshRsp->aid != NULL) {
                        free(pSimRefreshRspWithType->simRefreshRsp->aid);
                    }
                    free(pSimRefreshRspWithType->simRefreshRsp);
                    free(pSimRefreshRspWithType);
                    pSimRefreshRspWithType = NULL;
                    free(efId_str);
                    free(cmd);
                    return;
                }
                strncpy(pSimRefreshRspWithType->efId, efId_str, strlen(efId_str));
                pSimRefreshRspWithType->efId[strlen(efId_str)] = '\0';
            }
            pSimRefreshRspWithType->simRefreshRsp->result = SIM_FILE_UPDATE;

            msg = RfxMessage::obtainRequest(m_slot_id,
                            RFX_MSG_REQUEST_STK_QUERY_CPIN_STATE, RfxVoidData());
            requestToMcl(msg);

            if (NULL != cmd) {
                free(cmd);
            }
            if (efId_str != NULL) {
                free(efId_str);
                efId_str = NULL;
            }
            return;
        default:
            logE(RFX_LOG_TAG, "Refresh type does not support.");
            return;
    }

    RFX_LOG_D(RFX_LOG_TAG, "aid = %s, %d.", simRefreshRspV7.aid, aid_len);
    simRefreshRspV7.ef_id = 0;   // default EFID
    if (NULL != efId_str && 4 <= strlen(efId_str)) {
        files_num = strlen(efId_str)/4;
    }
    // Try to read each EFID and send the URC for SIM REFRESH
    if (NULL != efId_str && 0 < strlen(efId_str)) {
        for (i = 0; i < files_num; i++) {
            memcpy(temp_str, efId_str + readIdx, 4);
            temp_str[4] = '\0';
            efId = strtol(temp_str, NULL, 16);
            simRefreshRspV7.ef_id = efId;
            logD(RFX_LOG_TAG, "onSimRefresh: efId = %X, file numbers = %d", simRefreshRspV7.ef_id, files_num);

            //Todo: Add the data structure for RIL_SimRefreshResponse_v7
            sp<RfxMessage> refresh_urc = RfxMessage::obtainUrc(m_slot_id, RFX_MSG_URC_SIM_REFRESH,
                RfxSimRefreshData((void*)&simRefreshRspV7, sizeof(RIL_SimRefreshResponse_v7)));
            responseToRilj(refresh_urc);
            readIdx += 4;  // go to next EFID
        }
    } else {
            //Todo: Add the data structure for RIL_SimRefreshResponse_v7
            sp<RfxMessage> refresh_urc = RfxMessage::obtainUrc(m_slot_id, RFX_MSG_URC_SIM_REFRESH,
                RfxSimRefreshData((void*)&simRefreshRspV7, sizeof(RIL_SimRefreshResponse_v7)));
            responseToRilj(refresh_urc);
    }

    if (NULL != cmd) {
        free(cmd);
    }
    if (efId_str != NULL) {
        free(efId_str);
        efId_str = NULL;
    }
}

void RtcCatController::handleStkCommand(char* cmd, int cmdClass) {
    int urc_len = 0, type_pos = 0, cmdType = -1, cmdNum = 0;
    int cmdQual = 0;
    int cmd_not_understood = 0;
    int duration[1] = {0};
    int notify_info[3];
    int feature_version = 0;
    char optr[RFX_PROPERTY_VALUE_MAX] = {0};
    char seg[RFX_PROPERTY_VALUE_MAX] = {0};

    if (cmd == NULL) {
        logE(RFX_LOG_TAG, "handleStkCommand error: cmd is null!");
        return;
    }
    parseStkCmdType(cmd, &cmdType);
    parseStkCmdQualifier(cmd, &cmdQual);
    mProCmdRec.cmdType = cmdType;
    mProCmdRec.cmdQualifier = cmdQual;

    if (CMD_TYPE_PROACTIVE == cmdClass) {
        switch(cmdType) {
            case CMD_SETUP_MENU:
            case CMD_LAUNCH_BROWSER:
            case CMD_PLAY_TONE:
            case CMD_DSPL_TXT:
            case CMD_GET_INKEY:
            case CMD_GET_INPUT:
            case CMD_SELECT_ITEM:
            case CMD_PROVIDE_LOCAL_INFO:
            case CMD_IDLE_MODEL_TXT:
            case CMD_LANGUAGE_NOTIFY:
                mProCmdRec.needResponse = true;
                break;
            // BIP @{
            case CMD_OPEN_CHAN:
                mProCmdRec.needResponse = false;
                parseStkCmdNum(cmd, &cmdNum);
                mProCmdRec.cmdNum = cmdNum;
                logD(RFX_LOG_TAG,"handleStkCommand: cmd num is %d", mProCmdRec.cmdNum);
                break;
            // BIP @}
            default:
                mProCmdRec.needResponse = false;
                break;
        }
        if ((CMD_CLOSE_CHAN == cmdType) || (CMD_RECEIVE_DATA == cmdType)
                                                    || (CMD_SEND_DATA == cmdType)) {
            logD(RFX_LOG_TAG,"onHandleUrc: cmd has no need to send to stk fwk currently!");
            return;
        }
        sp<RfxMessage> urc = RfxMessage::obtainUrc(getSlotId(), RFX_MSG_URC_STK_PROACTIVE_COMMAND,
                RfxStringData(cmd));
        responseToRilj(urc);
    } else if (CMD_TYPE_NOTIFY == cmdClass) {
        mProCmdRec.needResponse = false;
        sp<RfxMessage> msg = NULL;
        if(cmd[2] <= '7' ) {
            type_pos = 10;
        } else if(cmd[2] > '7' ) {
            type_pos = 12;
        }
        /*temp_str[type_pos -6] points to cmd_detail tag*/
        cmd_not_understood = checkStkCmdDisplay(&(cmd[type_pos - 6]));
        logD(RFX_LOG_TAG, "cmd_not_understood Cmd: %d", cmd_not_understood);
        switch(cmdType) {
            case CMD_REFRESH:
                //handle sim refresh
                logD(RFX_LOG_TAG, "EventNotify cmd :%s ", cmd);
                handleSimRefresh(&(cmd[type_pos - 6]));
                break;
            case CMD_SETUP_CALL:
                //handle setup call
                logD(RFX_LOG_TAG, "handleEventNotify : CMD_SETUP_CALL");
                if(cmd_not_understood == 0) {
                    /*temp_str[type_pos -6] points to cmd_detail tag*/
                    duration[0] = (int)findStkCallDuration(&cmd[type_pos - 6]);
                    logD(RFX_LOG_TAG, "duration: %d", duration[0]);
                    mProCmdRec.cmdType = cmdType;
                    mProCmdRec.needResponse = true;
                } else {
                    notify_info[0] = CMD_SETUP_CALL;
                    notify_info[1] = 50;
                    msg = RfxMessage::obtainRequest(m_slot_id,
                            RFX_MSG_REQUEST_STK_EVENT_NOTIFY, RfxIntsData(notify_info, 2));
                    requestToMcl(msg);
                }
                break;
            case CMD_SEND_SMS:
                //handle send sms
                logD(RFX_LOG_TAG, "handleEventNotify : CMD_SEND_SMS");
                notify_info[0] = CMD_SEND_SMS;
                notify_info[1] = 0;
                msg = RfxMessage::obtainRequest(m_slot_id,
                            RFX_MSG_REQUEST_STK_EVENT_NOTIFY, RfxIntsData(notify_info, 2));
                requestToMcl(msg);
                // For CT A and C lab test, CDMA SIM card not show send sms toast
                rfx_property_get("persist.vendor.operator.optr", optr, "");
                rfx_property_get("persist.vendor.operator.seg", seg, "");
                if (((mCardType & RFX_CARD_TYPE_RUIM)
                       || (mCardType & RFX_CARD_TYPE_CSIM))
                       && strncmp(optr, "OP09", strlen("OP09")) == 0
                       && (strncmp(seg, "SEGC", strlen("SEGC")) == 0)) {
                    logD(RFX_LOG_TAG,"Send sms notify will not send to AP~~~");
                    return;
                }
                break;
            case CMD_SEND_SS:
                //handle send ss
                logD(RFX_LOG_TAG, "handleEventNotify :%s ", cmd);
                notify_info[0] = CMD_SEND_SS;
                if(cmd_not_understood == 0) {
                    notify_info[1] = 0;
                } else {
                    notify_info[1] = 50;
                }
                msg = RfxMessage::obtainRequest(m_slot_id,
                            RFX_MSG_REQUEST_STK_EVENT_NOTIFY, RfxIntsData(notify_info, 2));
                requestToMcl(msg);
                break;
            case CMD_SEND_USSD:
                //handle ussd
                logD(RFX_LOG_TAG, "handleEventNotify :%s ", cmd);
                notify_info[0] = CMD_SEND_USSD;
                if(cmd_not_understood == 0) {
                    char feature[] = "STK_USSD_OVER_IMS";
                    notify_info[1] = 0;
                    //check whether modem supports STK_USSD_OVER_IMS
                    feature_version = getFeatureVersion(feature, 0);
                    if (feature_version == 1) {
                        logD(RFX_LOG_TAG,"Begin to query ussd domain info");
                        //Send request to SS module
                        msg = RfxMessage::obtainRequest(m_slot_id,
                                    RFX_MSG_REQUEST_USSD_DOMAIN_INFO_REQ, RfxVoidData());
                        RfxMainThread::enqueueMessage(msg);
                        break;
                    } else {
                        logD(RFX_LOG_TAG,"Not support STK_USSD_OVER_IMS");
                    }
                } else {
                    notify_info[1] = 50;
                }
                //The 3rd vaule is domain info, 0 means over CS, and 1 means over IMS
                notify_info[2] = 0;
                msg = RfxMessage::obtainRequest(m_slot_id,
                            RFX_MSG_REQUEST_STK_EVENT_NOTIFY, RfxIntsData(notify_info, 3));
                requestToMcl(msg);
                break;
            case CMD_DTMF:
                //handle dtmf
                logD(RFX_LOG_TAG, "handleEventNotify :%s ", cmd);
                notify_info[0] = CMD_DTMF;
                notify_info[1] = 0;
                msg = RfxMessage::obtainRequest(m_slot_id,
                            RFX_MSG_REQUEST_STK_EVENT_NOTIFY, RfxIntsData(notify_info, 2));
                requestToMcl(msg);
                break;
            default:
                break;
        }
        // BIP @{
        //Need to compatible with the AP bip solution
        if (CMD_OPEN_CHAN == cmdType) {
            sp<RfxMessage> urc = RfxMessage::obtainUrc(getSlotId(), RFX_MSG_URC_STK_PROACTIVE_COMMAND,
                    RfxStringData(cmd));
            responseToRilj(urc);
            return;
        }
        // BIP @}
        sp<RfxMessage> urc = RfxMessage::obtainUrc(getSlotId(), RFX_MSG_URC_STK_EVENT_NOTIFY,
                RfxStringData(cmd));
        responseToRilj(urc);

        if(CMD_SETUP_CALL == cmdType) {
            sp<RfxMessage> setup_call_urc = RfxMessage::obtainUrc(getSlotId(), RFX_MSG_URC_STK_CALL_SETUP,
                RfxIntsData(duration, 1));
            responseToRilj(setup_call_urc);
        }
    }
}

void RtcCatController::parseStkCmdQualifier(char* cmd, int* cmdQual) {
    int cmd_len = strlen(cmd);
    int typePos = 0;

    if (cmd_len < 14) {
        logD(RFX_LOG_TAG, "parseStkCmdQualifier exception!");
        return;
    }

    if(cmd[2] <= '7' ) {
        typePos = 12;
    } else {
        typePos = 14;
    }

    // get command qualifier
    *cmdQual = getStkCommandType(&cmd[typePos]);
}

void RtcCatController::parseStkCmdType(char* cmd, int* cmdType) {
    int cmd_len = strlen(cmd);
    int typePos = 0;

    if (cmd_len < 14) {
        logD(RFX_LOG_TAG, "parseStkCmdType exception!");
        return;
    }

    if(cmd[2] <= '7' ) {
        typePos = 10;
    } else {
        typePos = 12;
    }

    // get command type
    *cmdType = getStkCommandType(&cmd[typePos]);
}

void RtcCatController::parseStkCmdType(const sp<RfxMessage>& message, int* cmdType) {
    int msgId = message->getId();
    int typePos = 0;
    RfxBaseData* stkCmd = NULL;
    char* cmd = NULL;

    // Get command from message
    stkCmd = message->getData();
    if (stkCmd != NULL) {
        cmd = (char*)stkCmd->getData();
    } else {
        logD(RFX_LOG_TAG, "parseStkCmdType cmd fail!");
        return;
    }

    if (cmd) {

        // decide typePos value
        switch (msgId) {
            case RFX_MSG_URC_STK_PROACTIVE_COMMAND:
            case RFX_MSG_URC_STK_EVENT_NOTIFY:
                if (cmd[2] <= '7') {
                    typePos = 10;
                } else {
                    typePos = 12;
                }
                break;

            case RFX_MSG_REQUEST_STK_SEND_TERMINAL_RESPONSE:
                typePos = 6;
                break;

            default:
                //logD(RFX_LOG_TAG,"Not Support msgId:%d", msgId);
                break;
        }

        // check command type
        *cmdType = getStkCommandType(&cmd[typePos]);
        //free(cmd);
    } else {
        logE(RFX_LOG_TAG, "cmd is NULL. Error!!!");
    }
}

int RtcCatController::getStkCommandNumber(char *cmd) {
    char temp[3] = {0};
    int cmdNum = 0;

    strncpy(temp, cmd, 2);
    cmdNum = strtoul(temp, NULL, 16);
    cmdNum = 0xFF & cmdNum;

    return cmdNum;
}

void RtcCatController::parseStkCmdNum(char* cmd, int* cmdNum) {
    int cmd_len = strlen(cmd);
    int typePos = 0;

    if (cmd_len < 12) {
        logD(RFX_LOG_TAG, "parseStkCmdNum exception!");
        return;
    }

    if (cmd[2] <= '7') {
        typePos = 8;
    } else {
        typePos = 10;
    }

    // check command num
    *cmdNum = getStkCommandNumber(&cmd[typePos]);
    logD(RFX_LOG_TAG,"parseStkCmdNum cmdNum:%d", *cmdNum);
}

int RtcCatController::getStkResultCode(char *cmd) {
    char temp[3] = {0};
    int resCode = 0;

    strncpy(temp, cmd, 2);
    resCode = strtoul(temp, NULL, 16);
    resCode = 0xFF & resCode;

    return resCode;
}

void RtcCatController::parseStkResultCode(const sp<RfxMessage>& message, int* resCode) {
    int msgId = message->getId();
    int typePos = 0;
    int cmd_len = 0;
    RfxBaseData* stkCmd = NULL;
    char* cmd = NULL;
    *resCode = -1;

    // Get command from message
    stkCmd = message->getData();
    if (stkCmd != NULL) {
        cmd = (char*)stkCmd->getData();
    } else {
        logD(RFX_LOG_TAG, "parseStkResultCode cmd:%s fail", cmd);
        return;
    }
    if (cmd != NULL) {
        cmd_len = strlen(cmd);
        logD(RFX_LOG_TAG, "parseStkResultCode cmd:%s ", cmd);
        // decide typePos value
        switch (msgId) {
            case RFX_MSG_REQUEST_STK_SEND_TERMINAL_RESPONSE:
                //For the TR from CatService, result code is at the 12th Bytes
                typePos = 22;
                break;
            default:
                break;
        }
        if (cmd_len < typePos) {
            logE(RFX_LOG_TAG, "parseStkResultCode exception!");
            return;
        }
        // check command type
        *resCode = getStkResultCode(&cmd[typePos]);
        logD(RFX_LOG_TAG,"parseStkResultCode cmdNum:%d", *resCode);
    } else {
        logE(RFX_LOG_TAG, "cmd is NULL. Error!!!");
    }
}

void RtcCatController::handleEventNotify(const sp<RfxMessage>& message) {
    int urc_len = 0;
    char* cmd = NULL;
    char* pEventNotify = NULL;
    char* pProCmd = NULL;
    RfxBaseData* stkCmd = NULL;

    // Get command from message
    stkCmd = message->getData();
    if (stkCmd != NULL) {
        cmd = (char*)stkCmd->getData();
    } else {
        logE(RFX_LOG_TAG, "handleEventNotify fail!");
        return;
    }

    if (cmd != NULL && strstr(cmd, "BIP")) {
        logD(RFX_LOG_TAG, "report BIP: START urc");
        sp<RfxMessage> urc = RfxMessage::obtainUrc(getSlotId(), RFX_MSG_URC_STK_EVENT_NOTIFY,
                RfxStringData(cmd));
        responseToRilj(urc);
        return;
    }

    if (false == isStkServiceRunning || pSimRefreshRspWithType != NULL) {
        if (cmd != NULL) {
            urc_len = strlen(cmd);
        } else {
            logE(RFX_LOG_TAG,"handleEventNotify: urc_len error!");
            return;
        }
        if (checkEventNotifyFreeBuffer() > 0) {
            pEventNotify = (char*)calloc(1, urc_len + 1);
            if (NULL == pEventNotify) {
                logE(RFX_LOG_TAG,"handleEventNotify: pEventNotify is NULL !!!");
                return;
            }
            memset(pEventNotify, 0x0, urc_len + 1);
            memcpy(pEventNotify, cmd, urc_len);
            setStkCachedEventNotify(pEventNotify);
            isEventNotifyQueued = true;
            logD(RFX_LOG_TAG, "STK service is not running yet.[%p]", pEventNotify);
        }
        return;
    }

    handleStkCommand(cmd, CMD_TYPE_NOTIFY);
}

void RtcCatController::handleProactiveCommand(const sp<RfxMessage>& message) {
    int urc_len = 0;
    int cmdType = 0;
    int cmdQual = 0;
    char* cmd = NULL;
    char* pProCmd = NULL;
    char* cachedMenu = NULL;
    RfxBaseData* stkCmd = NULL;

    // Get command from message
    stkCmd = message->getData();
    if (stkCmd != NULL) {
        cmd = (char*)stkCmd->getData();
    } else {
        logE(RFX_LOG_TAG, "handleProactiveCommand fail!");
        return;
    }

    if (cmd != NULL) {
        urc_len = strlen(cmd);
    } else {
        logE(RFX_LOG_TAG, "handleProactiveCommand: urc_len error!");
        return;
    }

    parseStkCmdType(cmd, &cmdType);
    mProCmdRec.cmdType = cmdType;
    parseStkCmdQualifier(cmd, &cmdQual);
    mProCmdRec.cmdQualifier= cmdQual;
    logD(RFX_LOG_TAG, " mProCmdRec.cmdType : 0x%02X, mProCmdRec.cmdQualifier : %d",
            mProCmdRec.cmdType, mProCmdRec.cmdQualifier);

    switch(cmdType) {
        case CMD_SETUP_MENU:
            cachedMenu = (char*)calloc(1, urc_len + 1);
            if (NULL == cachedMenu) {
                logE(RFX_LOG_TAG, "handleProactiveCommand: cachedMenu is NULL !!!");
                return;
            }
            memset(cachedMenu, 0x0, urc_len + 1);
            memcpy(cachedMenu, cmd, urc_len);
            logD(RFX_LOG_TAG, "cachedMenu:[%s] ", cachedMenu);
            pCachedMenu = cachedMenu;
            mProCmdRec.needResponse = true;
            break;
        case CMD_LAUNCH_BROWSER:
        case CMD_PLAY_TONE:
        case CMD_DSPL_TXT:
        case CMD_GET_INKEY:
        case CMD_GET_INPUT:
        case CMD_SELECT_ITEM:
        case CMD_PROVIDE_LOCAL_INFO:
        case CMD_IDLE_MODEL_TXT:
        case CMD_LANGUAGE_NOTIFY:
            mProCmdRec.needResponse = true;
            break;
        default:
             mProCmdRec.needResponse = false;
             break;
    }

    if (false == isStkServiceRunning || pSimRefreshRspWithType != NULL) {
        pProCmd = (char*)calloc(1, urc_len + 1);
        if (NULL == pProCmd) {
            logE(RFX_LOG_TAG, "handleProactiveCommand: pProCmd is NULL !!!");
            return;
        }

        memset(pProCmd, 0x0, urc_len + 1);
        memcpy(pProCmd, cmd, urc_len);
        pProactive_cmd = pProCmd;
        isProaCmdQueued = true;
        logD(RFX_LOG_TAG, "STK service is not running yet. cmd:[%p]", pProactive_cmd);
        return;
    }

    handleStkCommand(cmd, CMD_TYPE_PROACTIVE);
}

void RtcCatController::handleQueryCpinResponse(const sp<RfxMessage>& message) {
    int msg_id = 0;
    int *data = NULL;
    bool isCpinReady = false;
    RfxIntsData *intsData = NULL;

    logD(RFX_LOG_TAG,  "handleQueryCpinResponse");
    msg_id = message->getId();
    intsData = (RfxIntsData*)message->getData();
    if (intsData != NULL) {
        data = (int*)intsData->getData();
        if (data != NULL) {
            isCpinReady = (data[0] == 1) ? true : false;
            logD(RFX_LOG_TAG,  "handleQueryCpinResponse: isCpinReady = %d", isCpinReady);
        }
    }

    if (isCpinReady) {
        mCPinRetry = 0;
        handleSimRefreshAfterQueryCpin();
    } else {
        if (mCPinRetry < 14) {
            mCPinRetry ++;
            sendEventWithDelay(msg_id, STK_REFRESH_DELAY_TIME);
            logD(RFX_LOG_TAG, "handleQueryCpinResponse: cpin not ready, after 200ms retry again!");
        } else {
            handleSimRefreshAfterQueryCpin();
        }
    }
}

void RtcCatController::handleSimRefreshAfterQueryCpin() {
    char* pEfIdString;
    char* pAid;
    char temp_str[5] = { 0 }; // for read EFID
    int files_num = 0;
    int i = 0;
    int readIdx = 0;
    int efId = 0;
    SimRefreshResponse *simRefreshResponse = pSimRefreshRspWithType;

    logD(RFX_LOG_TAG, "handleSimRefreshAfterSimReady");

    pEfIdString = simRefreshResponse->efId;
    if (pEfIdString) {
        if (NULL != pEfIdString && 4 <= strlen(pEfIdString)) {
            files_num = strlen(pEfIdString) / 4;
        }
        // Try to read each EFID and send the URC for SIM REFRESH
        for (i = 0; i < files_num; i++) {
            memcpy(temp_str, pEfIdString + readIdx, 4);
            temp_str[4] = '\0';
            efId = strtol(temp_str, NULL, 16);
            simRefreshResponse->simRefreshRsp->ef_id = efId;
            logD(RFX_LOG_TAG, "queryCpinStatus: efId = %X, file numbers = %d", efId, files_num);
            //Todo: Add the data structure for RIL_SimRefreshResponse_v7
            sp<RfxMessage> refresh_urc = RfxMessage::obtainUrc(m_slot_id, RFX_MSG_URC_SIM_REFRESH,
                RfxSimRefreshData((void*)(simRefreshResponse->simRefreshRsp), sizeof(RIL_SimRefreshResponse_v7)));
            responseToRilj(refresh_urc);
            readIdx += 4;  // go to next EFID
        }
    } else {
        sp<RfxMessage> refresh_urc = RfxMessage::obtainUrc(m_slot_id, RFX_MSG_URC_SIM_REFRESH,
                RfxSimRefreshData((void*)(simRefreshResponse->simRefreshRsp), sizeof(RIL_SimRefreshResponse_v7)));
        responseToRilj(refresh_urc);
    }

    if (simRefreshResponse->simRefreshRsp->aid != NULL) {
        free(simRefreshResponse->simRefreshRsp->aid);
    }
    free(simRefreshResponse->simRefreshRsp);
    if (simRefreshResponse->efId != NULL) {
        free(simRefreshResponse->efId);
    }
    free(simRefreshResponse);
    simRefreshResponse = NULL;
    pSimRefreshRspWithType = NULL;

    sendStkQueuedCmd(1);
    return;
}

void RtcCatController::requestHandleStkServiceIsRunning() {
    isStkServiceRunning = true;
    sendStkQueuedCmd(0);
}

void RtcCatController::sendStkQueuedCmd(int flag) {

    if (true == isEventNotifyQueued) {
        logD(RFX_LOG_TAG, "sendStkQueuedCmd: isEventNotifyQueued: true");
        int index = 0;
        while (index < EVENT_NOTIFY_BUFFER_LEN) {
            if (NULL != pSimRefreshRspWithType) {
                logD(RFX_LOG_TAG, "sendStkQueuedCmd: wait for sim refresh done!");
                return;
            }
            char *cmd = getStkQueuedEventNotify(index);
            if (NULL != cmd) {
                logD(RFX_LOG_TAG, "send queued event to framework.");
                handleStkCommand(cmd, CMD_TYPE_NOTIFY);
                freeStkQueuedEventNotify(index);
            }
            index++ ;
        }
        isEventNotifyQueued = false;
    }

    if (true == isProaCmdQueued) {
        logD(RFX_LOG_TAG, "sendStkQueuedCmd: isProaCmdQueued: true");
        char *cmd = pProactive_cmd;
        if (NULL != cmd) {
            logD(RFX_LOG_TAG, "Proactive Cmd:[%s]", cmd);
            handleStkCommand(cmd, CMD_TYPE_PROACTIVE);
            freeStkQueuedProactivCmd();
            isProaCmdQueued = false;
        }
    } else {
       // For STK service running, need report set up menu to framework
        if (flag == 0) {
            if (mProCmdRec.needResponse && mProCmdRec.cmdType != 0) {
                sendResponseForSpecial(mProCmdRec.cmdType, mProCmdRec.cmdQualifier);
            }
            char *cachedMenu = pCachedMenu;
            logD(RFX_LOG_TAG, "sendStkQueuedCmd, will report menu, cachedMenu :[%s]" , cachedMenu);
            if (NULL != cachedMenu) {
                sp<RfxMessage> urc = RfxMessage::obtainUrc(getSlotId(), RFX_MSG_URC_STK_PROACTIVE_COMMAND,
                        RfxStringData(cachedMenu));
                responseToRilj(urc);
            }
        }
    }
}

int RtcCatController::rild_sms_hexCharToDecInt(char *hex, int length)
{
    int i = 0;
    int value, digit;

    for (i = 0, value = 0; i < length && hex[i] != '\0'; i++)
    {
        if (hex[i]>='0' && hex[i]<='9')
        {
            digit = hex[i] - '0';
        }
        else if ( hex[i]>='A' && hex[i] <= 'F')
        {
            digit = hex[i] - 'A' + 10;
        }
        else if ( hex[i]>='a' && hex[i] <= 'f')
        {
            digit = hex[i] - 'a' + 10;
        }
        else
        {
            return -1;
        }
        value = value*16 + digit;
    }

    return value;
}

void RtcCatController::setStkCachedEventNotify(char* cmd) {
    int i = 0;
    for (i = 0 ; i < EVENT_NOTIFY_BUFFER_LEN ; i++) {
        if (NULL == pEvent_notify[i]) {
            pEvent_notify[i] = cmd;
            break;
        }
    }
    if (EVENT_NOTIFY_BUFFER_LEN == i) {
        logE(RFX_LOG_TAG, "setStkCachedEventNotify: No free buffer !!!");
    } else {
        logD(RFX_LOG_TAG, "setStkCachedEventNotify: [index:%d][%p]", i, pEvent_notify[i]);
    }
}

void RtcCatController::sendEventWithDelay(int id, int delayTime) {
    logD(RFX_LOG_TAG,  "sendEventWithDelay: id = %d , delayTime = %d.", id, delayTime);

    if (mTimeoutHandle != NULL) {
        RfxTimer::stop(mTimeoutHandle);
        mTimeoutHandle = NULL;
    }

    switch (id) {
        case RFX_MSG_REQUEST_STK_QUERY_CPIN_STATE:
            mTimeoutHandle = RfxTimer::start(RfxCallback0(this,
                    &RtcCatController::retryQueryCpinState),
                    ms2ns(delayTime));
            break;
        default:
            logE(RFX_LOG_TAG, "sendEventWithDelay: should not be here");
            break;
    }
}

void RtcCatController::retryQueryCpinState() {
    sp<RfxMessage> msg = NULL;

    logD(RFX_LOG_TAG,  "retryQueryCpinState");

    msg = RfxMessage::obtainRequest(m_slot_id,
                            RFX_MSG_REQUEST_STK_QUERY_CPIN_STATE, RfxVoidData());
    requestToMcl(msg);
}


void RtcCatController::sendResponseForCardMissing(int cmdType, int cmdQual) {
    int cmdInfo[2];
    cmdInfo[0] = cmdType;
    cmdInfo[1] = cmdQual;

    sp<RfxMessage> msg = RfxMessage::obtainRequest(m_slot_id,
            RFX_MSG_REQUEST_STK_SEND_RESPONSE_BY_CMDTYPE, RfxIntsData(cmdInfo, 2));
    requestToMcl(msg);
}

// For the special case, such as phone process is killed, need send TR to modem
void RtcCatController::sendResponseForSpecial(int cmdType, int cmdQual) {
    int cmdInfo[2];
    cmdInfo[0] = cmdType;
    cmdInfo[1] = cmdQual;

    sp<RfxMessage> msg = RfxMessage::obtainRequest(m_slot_id,
            RFX_MSG_REQUEST_STK_SEND_RESPONSE_BY_CMDTYPE, RfxIntsData(cmdInfo, 2));
    requestToMcl(msg);
}

void RtcCatController::sendStkUssdConfirmInfo(const sp<RfxMessage>& message) {
    RfxIntsData *intsData = (RfxIntsData*)message->getData();
    int *data = NULL;
    int ussdInfo[3];
    sp<RfxMessage> msg = NULL;

    //default domain info is 0 which means ussd over CS
    ussdInfo[0] = CMD_SEND_USSD;
    ussdInfo[1] = 0;
    ussdInfo[2] = 0;
    if (intsData != NULL) {
        data = (int*)intsData->getData();
        if (data != NULL) {
            logD(RFX_LOG_TAG, "parse ussd domain info: %d ", data[0]);
            //value 1 means ussd over IMS
            ussdInfo[2] = (data[0] > 0) ? 1 : 0;
        }
    }
    msg = RfxMessage::obtainRequest(m_slot_id,
                            RFX_MSG_REQUEST_STK_EVENT_NOTIFY, RfxIntsData(ussdInfo, 3));
    requestToMcl(msg);
}

void RtcCatController::resetProCmd() {
    mProCmdRec.cmdType = 0;
    mProCmdRec.cmdQualifier = 0;
    mProCmdRec.needResponse = false;
}

bool RtcCatController::onCheckIfRejectMessage(const sp<RfxMessage>& message,
            bool isModemPowerOff, int radioState) {
    RFX_UNUSED(isModemPowerOff);

    int msg_id = message->getId();
    int slot_id = message->getSlotId();

    if (RADIO_STATE_UNAVAILABLE == radioState) {
        if (msg_id == RFX_MSG_REQUEST_STK_SEND_TERMINAL_RESPONSE) {
            return false;
        } else {
            return true;
        }
    } else {
        return false;
    }
}

void RtcCatController::parseStkEnvelopeCommandType(char* cmd, int* cmdType) {
    int typePos = 0;

    // As TS 102 223, the lenth of event download message is more than 9 Bytes
    if (cmd == NULL || strlen(cmd) < 18) {
        logE(RFX_LOG_TAG, "Envelop cmd length error!");
        return;
    }

    // Currently only check BER-TLV tag and D6 means this is an event download message
    if ((cmd[0] == 'D' || cmd[0] == 'd') && cmd[1] == '6') {
        if (cmd[2] <= '7') {
            // As TS 102 223, the length field will take 1 byte if the first value is less than 8
            // so the command type will start from the 5th byte.
            typePos = 8;
        } else {
            typePos = 10;
        }
        *cmdType = getStkCommandType(&cmd[typePos]);
        logD(RFX_LOG_TAG, "parseEnvelopeType event type:%d", *cmdType);
    } else {
        // For other envelope command, just return
        logE(RFX_LOG_TAG, "Not Event download cmd and return!");
    }
}

bool RtcCatController::isIgnoredEnvelopeCommand(const sp<RfxMessage>& message) {
    bool bIgnored = false;
    char* cmd = NULL;
    int eventType = -1;
    RfxBaseData* stkCmd = NULL;

    // Get command from message
    stkCmd = message->getData();
    if (stkCmd != NULL) {
        cmd = (char*)stkCmd->getData();
    } else {
        logD(RFX_LOG_TAG, "parse stk envelope command :%s fail", cmd);
        return false;
    }

    parseStkEnvelopeCommandType(cmd, &eventType);
    switch(eventType) {
        case EVENT_MT_CALL:
        case EVENT_CALL_CONNECTED:
        case EVENT_CALL_DISCONNECTED:
        case EVENT_LOCATION_STATUS:
        case EVENT_ACCESS_TECHNOLOGY_CHANGE:
        case EVENT_NETWORK_SEARCH_MODE_CHANGE:
        case EVENT_NETWORK_REJECTION:
            bIgnored = true;
            break;
        default:
            break;
    }
    logD(RFX_LOG_TAG, "Event type:%d need be ignored: %d", eventType, bIgnored);
    return bIgnored;
}
