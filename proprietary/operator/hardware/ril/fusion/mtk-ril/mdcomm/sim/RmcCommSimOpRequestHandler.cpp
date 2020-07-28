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
#include "RmcCommSimRequestHandler.h"
#include "RmcCommSimOpRequestHandler.h"
#include "RmcOpRsuSimDefs.h"
#include "RmcSimBaseHandler.h"
#include <telephony/mtk_ril.h>
#include <telephony/mtk_rilop.h>
#include "RfxMessageId.h"
#include "RfxStringData.h"
#include "RfxVoidData.h"

using ::android::String8;
using namespace std;

static const int ch1OpReqList[] = {
    RFX_MSG_REQUEST_ATT_GET_SHARED_KEY,
    RFX_MSG_REQUEST_ATT_UPDATE_SIM_LOCK_SETTINGS,
    RFX_MSG_REQUEST_ATT_GET_SIM_LOCK_VERSION,
    RFX_MSG_REQUEST_ATT_RESET_SIM_LOCK_SETTINGS,
    RFX_MSG_REQUEST_ATT_GET_MODEM_STATUS,
    RFX_MSG_REQUEST_ATT_GET_MODEM_CONFIG,
    RFX_MSG_REQUEST_ATT_GET_API_VERSION,
    RFX_MSG_REQUEST_ATT_GET_SIM_SLOTS,
    RFX_MSG_REQUEST_SEND_RSU_REQUEST,
};

RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxStringData,
        RFX_MSG_REQUEST_ATT_GET_SHARED_KEY);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringData, RfxVoidData,
        RFX_MSG_REQUEST_ATT_UPDATE_SIM_LOCK_SETTINGS);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxStringData,
        RFX_MSG_REQUEST_ATT_GET_SIM_LOCK_VERSION);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxVoidData,
        RFX_MSG_REQUEST_ATT_RESET_SIM_LOCK_SETTINGS);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxStringData,
        RFX_MSG_REQUEST_ATT_GET_MODEM_STATUS);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxStringData,
        RFX_MSG_REQUEST_ATT_GET_MODEM_CONFIG);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxStringData,
        RFX_MSG_REQUEST_ATT_GET_API_VERSION);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxStringData,
        RFX_MSG_REQUEST_ATT_GET_SIM_SLOTS);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxOpRsuRequestData, RfxOpRsuResponseData,
        RFX_MSG_REQUEST_SEND_RSU_REQUEST);

/*****************************************************************************
 * Class RmcCommSimOpRequestHandler
 *****************************************************************************/
RmcCommSimOpRequestHandler::RmcCommSimOpRequestHandler(int slot_id, int channel_id) :
        RmcCommSimRequestHandler(slot_id, channel_id) {
    setTag(String8("RmcCommSimOpReq"));
}

RmcCommSimOpRequestHandler::~RmcCommSimOpRequestHandler() {
    pthread_mutex_unlock(&simStatusMutex);
    pthread_mutex_destroy(&simStatusMutex);
}

const int* RmcCommSimOpRequestHandler::queryTable(int channel_id, int *record_num) {
    const int* superTable = RmcCommSimRequestHandler::queryTable(channel_id, record_num);
    int subRecordNumber = 0;
    int* bufTable = NULL;
    int index = 0;
    int supRecordNumber = *record_num;

    if (channel_id == RIL_CMD_PROXY_1) {
        subRecordNumber = sizeof(ch1OpReqList)/sizeof(int);
        bufTable = (int*)calloc(1, sizeof(int)* (subRecordNumber + supRecordNumber));
        RFX_ASSERT(bufTable != NULL);
        if (NULL != superTable) {
            memcpy(bufTable, superTable, sizeof(int) * supRecordNumber);
            index += supRecordNumber;
        } else {
            logE(mTag, "Null superTable", channel_id);
        }
        memcpy(bufTable + index, ch1OpReqList, sizeof(int) * subRecordNumber);
        *record_num += subRecordNumber;
    } else {
        bufTable = (int*)calloc(1, sizeof(int)* supRecordNumber);
        RFX_ASSERT(bufTable != NULL);
        if (NULL != superTable) {
            memcpy(bufTable, superTable, sizeof(int) * supRecordNumber);
        } else {
            logE(mTag, "Null superTable", channel_id);
        }
    }
    return bufTable;
}

RmcSimBaseHandler::SIM_HANDLE_RESULT RmcCommSimOpRequestHandler::needHandle(
        const sp<RfxMclMessage>& msg) {
    int request = msg->getId();
    RmcSimBaseHandler::SIM_HANDLE_RESULT result = RmcSimBaseHandler::RESULT_IGNORE;

    switch(request) {
        case RFX_MSG_REQUEST_ATT_GET_SHARED_KEY:
        case RFX_MSG_REQUEST_ATT_UPDATE_SIM_LOCK_SETTINGS:
        case RFX_MSG_REQUEST_ATT_GET_SIM_LOCK_VERSION:
        case RFX_MSG_REQUEST_ATT_RESET_SIM_LOCK_SETTINGS:
        case RFX_MSG_REQUEST_ATT_GET_MODEM_STATUS:
        case RFX_MSG_REQUEST_ATT_GET_MODEM_CONFIG:
        case RFX_MSG_REQUEST_ATT_GET_API_VERSION:
        case RFX_MSG_REQUEST_ATT_GET_SIM_SLOTS:
        case RFX_MSG_REQUEST_SEND_RSU_REQUEST:
            result = RmcSimBaseHandler::RESULT_NEED;
            break;
        default:
            result = RmcCommSimRequestHandler::needHandle(msg);
            break;
    }

    return result;
}

void RmcCommSimOpRequestHandler::handleRequest(const sp<RfxMclMessage>& msg) {
    int request = msg->getId();
    switch(request) {
        case RFX_MSG_REQUEST_ATT_GET_SHARED_KEY:
            handleAttGetSharedKey(msg);
            break;
        case RFX_MSG_REQUEST_ATT_UPDATE_SIM_LOCK_SETTINGS:
            handleAttUpdateSimLockSettings(msg);
            break;
        case RFX_MSG_REQUEST_ATT_GET_SIM_LOCK_VERSION:
            handleAttGetSimLockVersion(msg);
            break;
        case RFX_MSG_REQUEST_ATT_RESET_SIM_LOCK_SETTINGS:
            handleAttResetSimLockSettings(msg);
            break;
        case RFX_MSG_REQUEST_ATT_GET_MODEM_STATUS:
            handleAttGetModemStatus(msg);
            break;
        case RFX_MSG_REQUEST_ATT_GET_MODEM_CONFIG:
            handleAttGetModemConfig(msg);
            break;
        case RFX_MSG_REQUEST_ATT_GET_API_VERSION:
            handleAttGetApiVersion(msg);
            break;
        case RFX_MSG_REQUEST_ATT_GET_SIM_SLOTS:
            handleAttGetSimSlots(msg);
            break;
        case RFX_MSG_REQUEST_SEND_RSU_REQUEST:
            handleRsuRequest(msg);
            break;
        default:
            RmcCommSimRequestHandler::handleRequest(msg);
            break;
    }
}

void RmcCommSimOpRequestHandler::handleAttGetSharedKey(const sp<RfxMclMessage>& msg) {
    String8 cmd("");
    RfxAtLine *line = NULL;
    int err = -1;
    sp<RfxAtResponse> p_response = NULL;
    sp<RfxMclMessage> response;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    int key_len;
    char *key = NULL;

    logD(mTag, "[RSU-SIMLOCK] handleAttGetSharedKey.\n");

    cmd.append(String8::format("AT+EATTSK=0"));
    p_response = atSendCommandSingleline(cmd, "+EATTSK:");
    cmd.clear();
    err = p_response->getError();
    if (err < 0 || p_response->getSuccess() == 0) {
        goto done;
    }
    line = p_response->getIntermediates();
    line->atTokStart(&err);
    if (err < 0) {
        goto done;
    }

    // Get shared key len
    key_len = line->atTokNextint(&err);
    if (err < 0) {
        goto done;
    }

    // Get shared key
    key = line->atTokNextstr(&err);
    if (err < 0) {
        goto done;
    }
    ret = RIL_E_SUCCESS;
done:
    logD(mTag, "[RSU-SIMLOCK] done.\n");
    if (key != NULL) {
        logD(mTag, "[RSU-SIMLOCK] key %s.\n", key);
        response = RfxMclMessage::obtainResponse(msg->getId(), ret,
                RfxStringData((void*)key, strlen(key)), msg, false);
    } else {
        response = RfxMclMessage::obtainResponse(msg->getId(), ret, RfxStringData(), msg, false);
    }
    responseToTelCore(response);
}

void RmcCommSimOpRequestHandler::handleAttUpdateSimLockSettings(const sp<RfxMclMessage>& msg) {
    String8 pString((char*)(msg->getData()->getData()));
    String8 cmd("");
    RfxAtLine *line = NULL;
    int err = -1;
    sp<RfxAtResponse> p_response = NULL;
    sp<RfxMclMessage> response;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;

    logD(mTag, "[RSU-SIMLOCK] handleAttUpdateSimLockSettings strings %s\n", pString.string());

    cmd.append(String8::format("AT+EATTSLB=1,%d,\"%s\"",
            (unsigned int)((pString.size() + 1)/2), pString.string()));

    p_response = atSendCommand(cmd.string());
    cmd.clear();
    err = p_response->getError();
    if (err < 0) {
        goto done;
    }

    if (p_response->getSuccess() == 0) {
        switch (p_response->atGetCmeError()) {
            logD(mTag, "[RSU-SIMLOCK] p_response %d\n", p_response->atGetCmeError());
            case CME_VERIFICATION_FAILED:
                ret = RIL_E_VERIFICATION_FAILED;
                logD(mTag, "[RSU-SIMLOCK] p_response CME_SIM_LOCK_BLOB_VERIFICATION_FAILED\n");
                break;
            case CME_REBOOT_REQUEST:
                ret = RIL_E_REBOOT_REQUEST;
                logD(mTag, "[RSU-SIMLOCK] p_response CME_REBOOT_REQUEST\n");
                break;
            default:
                goto done;
        }
    } else {
        ret = RIL_E_SUCCESS;
        sp<RfxMclMessage> unsol = RfxMclMessage::obtainUrc(RFX_MSG_URC_RESPONSE_SIM_STATUS_CHANGED,
                m_slot_id, RfxVoidData());
        responseToTelCore(unsol);
    }

done:
    response = RfxMclMessage::obtainResponse(msg->getId(), ret, RfxVoidData(), msg, false);
    responseToTelCore(response);
}

void RmcCommSimOpRequestHandler::handleAttGetSimLockVersion(const sp<RfxMclMessage>& msg) {
    String8 cmd("");
    RfxAtLine *line = NULL;
    int err = -1;
    sp<RfxAtResponse> p_response = NULL;
    sp<RfxMclMessage> response;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    int info_len;
    char *info = NULL;

    logD(mTag, "[RSU-SIMLOCK] handleAttGetSimLockVersion\n");

    cmd.append(String8::format("AT+EATTSLB=3"));
    p_response = atSendCommandSingleline(cmd, "+EATTSLB:");
    cmd.clear();
    err = p_response->getError();
    if (err < 0 || p_response->getSuccess() == 0) {
        logD(mTag, "[RSU-SIMLOCK] handleAttGetSimLockVersion fail.\n");
        goto done;
    }
    line = p_response->getIntermediates();
    line->atTokStart(&err);
    if (err < 0) {
        goto done;
    }

    // Get info len
    info_len = line->atTokNextint(&err);
    if (err < 0) {
        goto done;
    }

    // Get info
    info = line->atTokNextstr(&err);
    if (err < 0) {
        goto done;
    }
    ret = RIL_E_SUCCESS;
done:
    logD(mTag, "[RSU-SIMLOCK] done.\n");
    if (info != NULL) {
        logD(mTag, "[RSU-SIMLOCK] info %s.\n", info);
        response = RfxMclMessage::obtainResponse(msg->getId(), ret,
                RfxStringData((void*)info, strlen(info)), msg, false);
    } else {
        response = RfxMclMessage::obtainResponse(msg->getId(), ret, RfxStringData(), msg, false);
    }
    responseToTelCore(response);
}

void RmcCommSimOpRequestHandler::handleAttResetSimLockSettings(const sp<RfxMclMessage>& msg) {
    String8 cmd("");
    RfxAtLine *line = NULL;
    int err = -1;
    sp<RfxAtResponse> p_response = NULL;
    sp<RfxMclMessage> response;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;

    logD(mTag, "[RSU-SIMLOCK] handleAttResetSimLockSettings\n");

    cmd.append(String8::format("AT+EATTSLB=2"));

    p_response = atSendCommand(cmd.string());
    cmd.clear();
    err = p_response->getError();
    if (err < 0) {
        goto done;
    }

    if (p_response->getSuccess() == 0) {
        switch (p_response->atGetCmeError()) {
            logD(mTag, "[RSU-SIMLOCK] p_response %d\n", p_response->atGetCmeError());
            case CME_VERIFICATION_FAILED:
                ret = RIL_E_VERIFICATION_FAILED;
                logD(mTag, "[RSU-SIMLOCK] p_response CME_SIM_LOCK_BLOB_VERIFICATION_FAILED\n");
                break;
            case CME_REBOOT_REQUEST:
                ret = RIL_E_REBOOT_REQUEST;
                logD(mTag, "[RSU-SIMLOCK] p_response CME_REBOOT_REQUEST\n");
                break;
            default:
                goto done;
        }
    } else {
        sp<RfxMclMessage> unsol = RfxMclMessage::obtainUrc(RFX_MSG_URC_RESPONSE_SIM_STATUS_CHANGED,
                m_slot_id, RfxVoidData());
        responseToTelCore(unsol);
    }
    ret = RIL_E_SUCCESS;
done:
    response = RfxMclMessage::obtainResponse(msg->getId(), ret, RfxVoidData(), msg, false);
    responseToTelCore(response);
}

void RmcCommSimOpRequestHandler::handleAttGetModemStatus(const sp<RfxMclMessage>& msg) {
    String8 cmd("");
    RfxAtLine *line = NULL;
    int err = -1;
    sp<RfxAtResponse> p_response = NULL;
    sp<RfxMclMessage> response;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    int status_len;
    char *status = NULL;

    logD(mTag, "[RSU-SIMLOCK] handleAttGetModemStatus\n");

    cmd.append(String8::format("AT+EATTSLB=4"));
    p_response = atSendCommandSingleline(cmd, "+EATTSLB:");
    cmd.clear();
    err = p_response->getError();
    if (err < 0 || p_response->getSuccess() == 0) {
        logD(mTag, "[RSU-SIMLOCK]handleAttGetModemStatus fail.\n");
        goto done;
    }
    line = p_response->getIntermediates();
    line->atTokStart(&err);
    if (err < 0) {
        goto done;
    }

    // Get status len
    status_len = line->atTokNextint(&err);
    if (err < 0) {
        goto done;
    }

    // Get status
    status = line->atTokNextstr(&err);
    if (err < 0) {
        goto done;
    }
    ret = RIL_E_SUCCESS;
done:
    logD(mTag, "[RSU-SIMLOCK] done.\n");
    if (status != NULL) {
        logD(mTag, "[RSU-SIMLOCK] status %s.\n", status);
        response = RfxMclMessage::obtainResponse(msg->getId(), ret,
                RfxStringData((void*)status, strlen(status)), msg, false);
    } else {
        response = RfxMclMessage::obtainResponse(msg->getId(), ret, RfxStringData(), msg, false);
    }
    responseToTelCore(response);
}

void RmcCommSimOpRequestHandler::handleAttGetModemConfig(const sp<RfxMclMessage>& msg) {
    String8 cmd("");
    RfxAtLine *line = NULL;
    int err = -1;
    sp<RfxAtResponse> p_response = NULL;
    sp<RfxMclMessage> response;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    int config_len;
    char *config = NULL;

    logD(mTag, "[RSU-SIMLOCK] handleAttGetModemConfig\n");

    cmd.append(String8::format("AT+EATTSLB=5"));
    p_response = atSendCommandSingleline(cmd, "+EATTSLB:");
    cmd.clear();
    err = p_response->getError();
    if (err < 0 || p_response->getSuccess() == 0) {
        logD(mTag, "[RSU-SIMLOCK]handleAttGetModemConfig fail.\n");
        goto done;
    }
    line = p_response->getIntermediates();
    line->atTokStart(&err);
    if (err < 0) {
        goto done;
    }

    // Get status len
    config_len = line->atTokNextint(&err);
    if (err < 0) {
        goto done;
    }

    // Get status
    config = line->atTokNextstr(&err);
    if (err < 0) {
        goto done;
    }
    ret = RIL_E_SUCCESS;
done:
    logD(mTag, "[RSU-SIMLOCK] done.\n");
    if (config != NULL) {
        logD(mTag, "[RSU-SIMLOCK] status %s.\n", config);
        response = RfxMclMessage::obtainResponse(msg->getId(), ret,
                RfxStringData((void*)config, strlen(config)), msg, false);
    } else {
        response = RfxMclMessage::obtainResponse(msg->getId(), ret, RfxStringData(), msg, false);
    }
    responseToTelCore(response);
}

void RmcCommSimOpRequestHandler::handleAttGetApiVersion(const sp<RfxMclMessage>& msg) {
    String8 cmd("");
    RfxAtLine *line = NULL;
    int err = -1;
    sp<RfxAtResponse> p_response = NULL;
    sp<RfxMclMessage> response;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    int version_len;
    char *version = NULL;

    logD(mTag, "[RSU-SIMLOCK] handleAttGetApiVersion\n");

    cmd.append(String8::format("AT+EATTSLB=6"));
    p_response = atSendCommandSingleline(cmd, "+EATTSLB:");
    cmd.clear();
    err = p_response->getError();
    if (err < 0 || p_response->getSuccess() == 0) {
        logD(mTag, "[RSU-SIMLOCK]handleAttGetApiVersion fail.\n");
        goto done;
    }
    line = p_response->getIntermediates();
    line->atTokStart(&err);
    if (err < 0) {
        goto done;
    }

    // Get version len
    version_len = line->atTokNextint(&err);
    if (err < 0) {
        goto done;
    }

    // Get status
    version = line->atTokNextstr(&err);
    if (err < 0) {
        goto done;
    }
    ret = RIL_E_SUCCESS;
done:
    logD(mTag, "[RSU-SIMLOCK] done.\n");
    if (version != NULL) {
        logD(mTag, "[RSU-SIMLOCK] version %s.\n", version);
        response = RfxMclMessage::obtainResponse(msg->getId(), ret,
                RfxStringData((void*)version, strlen(version)), msg, false);
    } else {
        response = RfxMclMessage::obtainResponse(msg->getId(), ret, RfxStringData(), msg, false);
    }
    responseToTelCore(response);
}

void RmcCommSimOpRequestHandler::handleAttGetSimSlots(const sp<RfxMclMessage>& msg) {
    String8 cmd("");
    RfxAtLine *line = NULL;
    int err = -1;
    sp<RfxAtResponse> p_response = NULL;
    sp<RfxMclMessage> response;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    int slots_len;
    char *slots = NULL;

    logD(mTag, "[RSU-SIMLOCK] handleAttGetSimSlots\n");

    cmd.append(String8::format("AT+EATTSLB=7"));
    p_response = atSendCommandSingleline(cmd, "+EATTSLB:");
    cmd.clear();
    err = p_response->getError();
    if (err < 0 || p_response->getSuccess() == 0) {
        logD(mTag, "[RSU-SIMLOCK]handleAttGetSimSlots fail.\n");
        goto done;
    }
    line = p_response->getIntermediates();
    line->atTokStart(&err);
    if (err < 0) {
        goto done;
    }

    // Get slots len
    slots_len = line->atTokNextint(&err);
    if (err < 0) {
        goto done;
    }

    // Get slots
    slots = line->atTokNextstr(&err);
    if (err < 0) {
        goto done;
    }
    ret = RIL_E_SUCCESS;
done:
    logD(mTag, "[RSU-SIMLOCK] done.\n");
    if (slots != NULL) {
        logD(mTag, "[RSU-SIMLOCK] slots %s.\n", slots);
        response = RfxMclMessage::obtainResponse(msg->getId(), ret,
                RfxStringData((void*)slots, strlen(slots)), msg, false);
    } else {
        response = RfxMclMessage::obtainResponse(msg->getId(), ret, RfxStringData(), msg, false);
    }
    responseToTelCore(response);
}

void RmcCommSimOpRequestHandler::handleRsuRequest(const sp<RfxMclMessage>& msg) {
    RIL_RsuRequestInfo* request = (RIL_RsuRequestInfo*)(msg->getData()->getData());
    logD(mTag, "[RSU-SIMLOCK] handleRsuRequest opId = %d, requestId = %d",
            request->opId, request->requestId);

    switch(request->opId) {
        case RSU_OP_VZW:
            handleVzwRsuRequest(msg);
            return;
        case RSU_OP_KDDI:
            handleKDDIRsuRequest(msg);
        default:
            break;
    }

    sp<RfxMclMessage> mclResponse = RfxMclMessage::obtainResponse(msg->getId(),
                    RIL_E_GENERIC_FAILURE, RfxVoidData(), msg);
    responseToTelCore(mclResponse);
}

void RmcCommSimOpRequestHandler::handleKDDIRsuRequest(const sp<RfxMclMessage>& msg) {
    RIL_RsuRequestInfo* request = (RIL_RsuRequestInfo*)(msg->getData()->getData());

    switch(request->requestId) {
        case RSU_REQUEST_SET_SIM_LOCK_DATA:
            handleKDDIRsuSetSimLockData(msg);
            return;
        case RSU_REQUEST_SET_GET_ALLOWED_LOCK_DATA:
        case RSU_REQUEST_SET_GET_EXCLUDED_LOCK_DATA:
            handleKDDIRsuGetLockData(msg, request->requestId);
            return;
        default:
            break;
    }

    sp<RfxMclMessage> mclResponse = RfxMclMessage::obtainResponse(msg->getId(),
                    RIL_E_REQUEST_NOT_SUPPORTED, RfxVoidData(), msg);
    responseToTelCore(mclResponse);
}

#define KDDI_MAX_LOCK_DATA_CHAR 400
void RmcCommSimOpRequestHandler::handleKDDIRsuSetSimLockData(const sp<RfxMclMessage>& msg) {
    int err = 0;
    int errCode = 0;

    String8 cmd("");
    sp<RfxAtResponse> atResponse;
    sp<RfxMclMessage> mclResponse;

    int requestId = RSU_MODEM_REQUEST_SET_LOCK_DATA_MORE;
    int offset = 0, length = 0;
    char* lock_data = NULL;
    char* cuttent_data = NULL;

    RIL_RsuRequestInfo* request = (RIL_RsuRequestInfo*)(msg->getData()->getData());
    RIL_RsuResponseInfo* response = (RIL_RsuResponseInfo*)calloc(1, sizeof(RIL_RsuResponseInfo));

    int data_size = (request->data != NULL ? strlen(request->data) : 0);

    RFX_ASSERT(response != NULL);
    response->opId = request->opId;
    response->requestId = request->requestId;

    logD(mTag, "[RSU-SIMLOCK] handleKDDIRsuSetSimLockData opId = %d, requestId = %d, data_size = %d",
            request->opId, requestId, data_size);
    cuttent_data = request->data;
    do {
        cuttent_data = cuttent_data + length;
        if (KDDI_MAX_LOCK_DATA_CHAR <= data_size) {
            length = KDDI_MAX_LOCK_DATA_CHAR;
        } else {
            requestId = RSU_MODEM_REQUEST_SET_LOCK_DATA_DONE;
            length = data_size;
        }
        logD(mTag, "[RSU-SIMLOCK] requestId %d, length %d, offset %d", requestId, data_size, offset);
        lock_data = (char*)calloc(1, length + 1);
        RFX_ASSERT(lock_data != NULL);
        strncpy(lock_data, cuttent_data, length);

        cmd.clear();
        cmd.append(String8::format("AT+ESMLRSU=%d,%d,%d,%d,\"%s\"",
                request->opId, requestId, offset, length, lock_data));
        atResponse = atSendCommand(cmd);
        free(lock_data);
        if (atResponse == NULL) {
            logE(mTag, "[RSU-SIMLOCK] atResponse is null");
            goto error;
        }

        err = atResponse->getError();
        if (err < 0) {
            logE(mTag, "[RSU-SIMLOCK] err = %d", err);
            goto error;
        }

        if (atResponse->getSuccess() == 0) {
            errCode = atResponse->atGetCmeError();
            logE(mTag, "[RSU-SIMLOCK] errCode = %d", errCode);
            // We get error cause 271 means the SIM Lock data is correct and save to nvram.
            if (errCode == CME_REBOOT_REQUEST) {
                break;
            } else {
                goto error;
            }
        }
        data_size = data_size - length;
        offset = offset + (length / 2);
        logD(mTag, "[RSU-SIMLOCK] data_size %d, %d", data_size, offset);
    } while (data_size > 0);

    mclResponse = RfxMclMessage::obtainResponse(msg->getId(),
            RIL_E_SUCCESS, RfxOpRsuResponseData(response, sizeof(RIL_RsuResponseInfo)), msg);
    responseToTelCore(mclResponse);
    free(response);
    return;

error:
    free(response);
    mclResponse = RfxMclMessage::obtainResponse(msg->getId(),
            RIL_E_GENERIC_FAILURE, RfxVoidData(), msg);
    responseToTelCore(mclResponse);
}

void RmcCommSimOpRequestHandler::handleKDDIRsuGetLockData(const sp<RfxMclMessage>& msg,
        int requestId) {
    int err = 0;
    int errCode = 0;

    String8 cmd("");
    sp<RfxAtResponse> atResponse;
    sp<RfxMclMessage> mclResponse;
    RfxAtLine *p_cur = NULL;

    int category = 0, triplet_length = 0;
    RIL_RsuRequestInfo* request = (RIL_RsuRequestInfo*)(msg->getData()->getData());
    RIL_RsuResponseInfo* response = (RIL_RsuResponseInfo*)calloc(1, sizeof(RIL_RsuResponseInfo));
    int sim_lock_group = 1; // 1: white list, 2: black list
    char* IMSI = NULL;
    char* GID1 = NULL;
    char* GID2 = NULL;
    char* temp_data = NULL;
    char* tmp_buffer1 = NULL;
    char* tmp_buffer2 = NULL;

    RFX_ASSERT(response != NULL);
    response->opId = request->opId;
    response->requestId = request->requestId;

    if (requestId == RSU_REQUEST_SET_GET_EXCLUDED_LOCK_DATA) {
        sim_lock_group = 2;
    }

    cmd.append(String8::format("AT+ESMLRSU=%d,%d,%d",
            request->opId, RSU_MODEM_REQUEST_GET_LOCK_DATA, sim_lock_group));
    atResponse = atSendCommandMultiline(cmd, "+ESMLRSU:");
    if (atResponse == NULL) {
        logE(mTag, "[RSU-SIMLOCK] handleKDDIRsuGetLockData atResponse is null");
        goto error;
    }

    if (atResponse->isAtResponseFail()) {
        errCode = atResponse->atGetCmeError();
        logE(mTag, "[RSU-SIMLOCK] handleKDDIRsuGetLockData errCode = %d", errCode);
        response->errCode = errCode;
        mclResponse = RfxMclMessage::obtainResponse(msg->getId(),
                RIL_E_GENERIC_FAILURE,
                RfxOpRsuResponseData(response, sizeof(RIL_RsuResponseInfo)), msg);
        responseToTelCore(mclResponse);
        free(response);
        return;
    }

    for (p_cur = atResponse->getIntermediates()
            ; p_cur != NULL
            ; p_cur = p_cur->getNext()) {
        p_cur->atTokStart(&err);
        if (err < 0) {
            logE(mTag, "[RSU-SIMLOCK] handleKDDIRsuGetLockData atTokStart error");
            goto error;
        }

        category = p_cur->atTokNextint(&err);
        if (err < 0) {
            logE(mTag, "[RSU-SIMLOCK] handleKDDIRsuGetLockData get category error");
            goto error;
        }
        triplet_length = p_cur->atTokNextint(&err);
        if (err < 0) {
            logE(mTag, "[RSU-SIMLOCK] handleKDDIRsuGetLockData get triplet_length error");
            goto error;
        }

        char category_str[2] = {0};
        sprintf(category_str, "%d", category);
        for (int i = 0; i < triplet_length; i++) {
            IMSI = p_cur->atTokNextstr(&err);
            if (err < 0) {
                logE(mTag, "[RSU-SIMLOCK] get IMSI error.");
                goto error;
            }
            GID1 = p_cur->atTokNextstr(&err);
            if (err < 0) {
                logE(mTag, "[RSU-SIMLOCK] get GID1 error.");
                goto error;
            }
            GID2 = p_cur->atTokNextstr(&err);
            if (err < 0) {
                logE(mTag, "[RSU-SIMLOCK] get GID2 error.");
                goto error;
            }
            //Construct IMSI:GID1:GID2,IMSI:GID1:GID2,...
            asprintf(&temp_data,"%s:%s:%s:%s", category_str, IMSI, GID1, GID2);
            logD(mTag, "[RSU-SIMLOCK] temp_data %s", temp_data);
            if (tmp_buffer1 == NULL) {
                tmp_buffer1 = (char*)calloc(1, (strlen(temp_data) * sizeof(char)) + 2);
                RFX_ASSERT(tmp_buffer1 != NULL);
                strncpy(tmp_buffer1, temp_data, strlen(temp_data));
            } else {
                tmp_buffer2 = (char*)calloc(1, ((strlen(tmp_buffer1) + strlen(temp_data)) * sizeof(char)) + 2);
                RFX_ASSERT(tmp_buffer2 != NULL);
                strncpy(tmp_buffer2, tmp_buffer1, strlen(tmp_buffer1));
                strncat(tmp_buffer2, ",", 1);
                strncat(tmp_buffer2, temp_data, strlen(temp_data));
                free(tmp_buffer1);
                tmp_buffer1 = tmp_buffer2;
            }
            logD(mTag, "[RSU-SIMLOCK] handleKDDIRsuGetLockData tmp_buffer1 %s, tmp_buffer2 %s",
                    tmp_buffer1, tmp_buffer2);
        }
    }
    response->data = tmp_buffer1;
    mclResponse = RfxMclMessage::obtainResponse(msg->getId(),
            RIL_E_SUCCESS, RfxOpRsuResponseData(response, sizeof(RIL_RsuResponseInfo)), msg);
    responseToTelCore(mclResponse);
    free(tmp_buffer1);
    free(response);
    return;

error:
    free(response);
    mclResponse = RfxMclMessage::obtainResponse(msg->getId(),
            RIL_E_GENERIC_FAILURE, RfxVoidData(), msg);
    responseToTelCore(mclResponse);
}

void RmcCommSimOpRequestHandler::handleVzwRsuRequest(const sp<RfxMclMessage>& msg) {
    RIL_RsuRequestInfo* request = (RIL_RsuRequestInfo*)(msg->getData()->getData());

    switch(request->requestId) {
        case RSU_REQUEST_INIT_REQUEST:
            handleVzwRsuInitiate(msg);
            return;
        case RSU_REQUEST_GET_SHARED_KEY:
            handleVzwRsuGetSharedKey(msg);
            return;
        case RSU_REQUEST_UPDATE_LOCK_DATA:
            handleVzwRsuUpdateLockData(msg);
            return;
        case RSU_REQUEST_GET_LOCK_VERSION:
            handleVzwRsuGetVersion(msg);
            return;
        case RSU_REQUEST_GET_LOCK_STATUS:
            handleVzwRsuGetStatus(msg);
            return;
        case RSU_REQUEST_UNLOCK_TIMER:
            handleVzwRsuUnlockTimer(msg);
            return;
        default:
            break;
    }

    sp<RfxMclMessage> mclResponse = RfxMclMessage::obtainResponse(msg->getId(),
                    RIL_E_GENERIC_FAILURE, RfxVoidData(), msg);
    responseToTelCore(mclResponse);
}

void RmcCommSimOpRequestHandler::handleVzwRsuInitiate(const sp<RfxMclMessage>& msg) {
    int err = 0;
    int errCode = 0;

    String8 cmd("");
    RfxAtLine *pLine = NULL;
    sp<RfxAtResponse> atResponse;
    sp<RfxMclMessage> mclResponse;

    int opId = -1;
    int requestId = -1;
    char* generateRequest = NULL;

    RIL_RsuRequestInfo* request = (RIL_RsuRequestInfo*)(msg->getData()->getData());
    RIL_RsuResponseInfo* response = (RIL_RsuResponseInfo*)calloc(1, sizeof(RIL_RsuResponseInfo));
    RFX_ASSERT(response != NULL);
    response->opId = request->opId;
    response->requestId = request->requestId;

    cmd.append(String8::format("AT+ESMLRSU=%d,%d", request->opId, RSU_MODEM_REQUEST_INIT_REQUEST));
    atResponse = atSendCommandSingleline(cmd, "+ESMLRSU:");
    if (atResponse == NULL) {
        logE(mTag, "[RSU-SIMLOCK] handleVzwRsuInitiate atResponse is null");
        goto error;
    }

    if (atResponse->isAtResponseFail()) {
        errCode = atResponse->atGetCmeError();
        logE(mTag, "[RSU-SIMLOCK] handleVzwRsuInitiate errCode = %d", errCode);
        response->errCode = errCode;
        mclResponse = RfxMclMessage::obtainResponse(msg->getId(),
                RIL_E_GENERIC_FAILURE,
                RfxOpRsuResponseData(response, sizeof(RIL_RsuResponseInfo)), msg);
        responseToTelCore(mclResponse);
        free(response);
        return;
    }

    pLine = atResponse->getIntermediates();
    if (pLine == NULL) {
        logE(mTag, "[RSU-SIMLOCK] handleVzwRsuInitiate pLine is null");
        goto error;
    }
    pLine->atTokStart(&err);
    if (err < 0) {
        logE(mTag, "[RSU-SIMLOCK] handleVzwRsuInitiate atTokStart error");
        goto error;
    }

    opId = pLine->atTokNextint(&err);
    if (err < 0 || opId != request->opId) {
        logE(mTag, "[RSU-SIMLOCK] handleVzwRsuInitiate err < 0 or opId = %d"
                " not equal ot request opId = %d", opId, request->opId);
        goto error;
    }
    requestId = pLine->atTokNextint(&err);
    if (err < 0 || requestId != RSU_MODEM_REQUEST_INIT_REQUEST) {
        logE(mTag, "[RSU-SIMLOCK] handleVzwRsuInitiate err < 0 or requestId = %d"
                " not equal ot request requestId = %d", requestId, request->requestId);
        goto error;
    }

    generateRequest = pLine->atTokNextstr(&err);
    if (err < 0 || generateRequest == NULL) {
        logE(mTag, "[RSU-SIMLOCK] handleVzwRsuInitiate err < 0 or generateRequest is null");
        goto error;
    }
    response->data = generateRequest;
    logD(mTag, "[RSU-SIMLOCK] handleVzwRsuInitiate generateRequest = %s", response->data);

    mclResponse = RfxMclMessage::obtainResponse(msg->getId(),
            RIL_E_SUCCESS, RfxOpRsuResponseData(response, sizeof(RIL_RsuResponseInfo)), msg);
    responseToTelCore(mclResponse);
    free(response);
    return;

error:
    free(response);
    mclResponse = RfxMclMessage::obtainResponse(msg->getId(),
            RIL_E_GENERIC_FAILURE, RfxVoidData(), msg);
    responseToTelCore(mclResponse);
}

void RmcCommSimOpRequestHandler::handleVzwRsuGetSharedKey(const sp<RfxMclMessage>& msg) {
    int err = 0;
    int errCode = 0;

    String8 cmd("");
    RfxAtLine *pLine = NULL;
    sp<RfxAtResponse> atResponse;
    sp<RfxMclMessage> mclResponse;

    int opId = -1;
    int requestId = -1;
    char* sharedKey = NULL;

    RIL_RsuRequestInfo* request = (RIL_RsuRequestInfo*)(msg->getData()->getData());
    RIL_RsuResponseInfo* response = (RIL_RsuResponseInfo*)calloc(1, sizeof(RIL_RsuResponseInfo));
    RFX_ASSERT(response != NULL);
    response->opId = request->opId;
    response->requestId = request->requestId;

    cmd.append(String8::format("AT+ESMLRSU=%d,%d,\"%s\"",
            request->opId, RSU_MODEM_REQUEST_GET_SHARED_KEY, request->data));
    atResponse = atSendCommandSingleline(cmd, "+ESMLRSU:");
    if (atResponse == NULL) {
        logE(mTag, "[RSU-SIMLOCK] handleVzwRsuGetSharedKey atResponse is null");
        goto error;
    }

    if (atResponse->isAtResponseFail()) {
        errCode = atResponse->atGetCmeError();
        logE(mTag, "[RSU-SIMLOCK] handleVzwRsuGetSharedKey errCode = %d", errCode);
        response->errCode = errCode;
        mclResponse = RfxMclMessage::obtainResponse(msg->getId(),
                RIL_E_GENERIC_FAILURE,
                RfxOpRsuResponseData(response, sizeof(RIL_RsuResponseInfo)), msg);
        responseToTelCore(mclResponse);
        free(response);
        return;
    }

    pLine = atResponse->getIntermediates();
    if (pLine == NULL) {
        logE(mTag, "[RSU-SIMLOCK] handleVzwRsuGetSharedKey pLine is null");
        goto error;
    }
    pLine->atTokStart(&err);
    if (err < 0) {
        logE(mTag, "[RSU-SIMLOCK] handleVzwRsuGetSharedKey atTokStart error");
        goto error;
    }

    opId = pLine->atTokNextint(&err);
    if (err < 0 || opId != request->opId) {
        logE(mTag, "[RSU-SIMLOCK] handleVzwRsuGetSharedKey err < 0 or opId = %d"
                " not equal ot request opId = %d", opId, request->opId);
        goto error;
    }
    requestId = pLine->atTokNextint(&err);
    if (err < 0 || requestId != RSU_MODEM_REQUEST_GET_SHARED_KEY) {
        logE(mTag, "[RSU-SIMLOCK] handleVzwRsuGetSharedKey err < 0 or requestId = %d"
                " not equal ot request requestId = %d", requestId, request->requestId);
        goto error;
    }

    sharedKey = pLine->atTokNextstr(&err);
    if (err < 0 || sharedKey == NULL) {
        logE(mTag, "[RSU-SIMLOCK] handleVzwRsuGetSharedKey err < 0 or sharedKey is null");
        goto error;
    }
    response->data = sharedKey;
    logD(mTag, "[RSU-SIMLOCK] handleVzwRsuGetSharedKey sharedKey = %s", response->data);

    mclResponse = RfxMclMessage::obtainResponse(msg->getId(),
            RIL_E_SUCCESS, RfxOpRsuResponseData(response, sizeof(RIL_RsuResponseInfo)), msg);
    responseToTelCore(mclResponse);
    free(response);
    return;

error:
    free(response);
    mclResponse = RfxMclMessage::obtainResponse(msg->getId(),
            RIL_E_GENERIC_FAILURE, RfxVoidData(), msg);
    responseToTelCore(mclResponse);
}

void RmcCommSimOpRequestHandler::handleVzwRsuUpdateLockData(const sp<RfxMclMessage>& msg) {
    int err = 0;
    int errCode = 0;

    String8 cmd("");
    RfxAtLine *pLine = NULL;
    sp<RfxAtResponse> atResponse;
    sp<RfxMclMessage> mclResponse;

    int opId = -1;
    int requestId = -1;
    char* lockData = NULL;

    RIL_RsuRequestInfo* request = (RIL_RsuRequestInfo*)(msg->getData()->getData());
    RIL_RsuResponseInfo* response = (RIL_RsuResponseInfo*)calloc(1, sizeof(RIL_RsuResponseInfo));
    RFX_ASSERT(response != NULL);
    response->opId = request->opId;
    response->requestId = request->requestId;

    cmd.append(String8::format("AT+ESMLRSU=%d,%d,\"%s\"",
            request->opId, RSU_MODEM_REQUEST_UPDATE_LOCK_DATA, request->data));
    atResponse = atSendCommandSingleline(cmd, "+ESMLRSU:");
    if (atResponse == NULL) {
        logE(mTag, "[RSU-SIMLOCK] handleVzwRsuUpdateLockData atResponse is null");
        goto error;
    }

    if (atResponse->isAtResponseFail()) {
        errCode = atResponse->atGetCmeError();
        logE(mTag, "[RSU-SIMLOCK] handleVzwRsuUpdateLockData errCode = %d", errCode);
        response->errCode = errCode;
        mclResponse = RfxMclMessage::obtainResponse(msg->getId(),
                RIL_E_GENERIC_FAILURE,
                RfxOpRsuResponseData(response, sizeof(RIL_RsuResponseInfo)), msg);
        responseToTelCore(mclResponse);
        free(response);
        return;
    }

    pLine = atResponse->getIntermediates();
    if (pLine == NULL) {
        logE(mTag, "[RSU-SIMLOCK] handleVzwRsuUpdateLockData pLine is null");
        goto error;
    }
    pLine->atTokStart(&err);
    if (err < 0) {
        logE(mTag, "[RSU-SIMLOCK] handleVzwRsuUpdateLockData atTokStart error");
        goto error;
    }

    opId = pLine->atTokNextint(&err);
    if (err < 0 || opId != request->opId) {
        logE(mTag, "[RSU-SIMLOCK] handleVzwRsuUpdateLockData err < 0 or opId = %d"
                " not equal ot request opId = %d", opId, request->opId);
        goto error;
    }
    requestId = pLine->atTokNextint(&err);
    if (err < 0 || requestId != RSU_MODEM_REQUEST_UPDATE_LOCK_DATA) {
        logE(mTag, "[RSU-SIMLOCK] handleVzwRsuUpdateLockData err < 0 or requestId = %d"
                " not equal ot request requestId = %d", requestId, request->requestId);
        goto error;
    }

    lockData = pLine->atTokNextstr(&err);
    if (err < 0 || lockData == NULL) {
        logE(mTag, "[RSU-SIMLOCK] handleVzwRsuUpdateLockData err < 0 or lockData is null");
        goto error;
    }
    response->data = lockData;
    logD(mTag, "[RSU-SIMLOCK] handleVzwRsuUpdateLockData lockData = %s", response->data);

    mclResponse = RfxMclMessage::obtainResponse(msg->getId(),
            RIL_E_SUCCESS, RfxOpRsuResponseData(response, sizeof(RIL_RsuResponseInfo)), msg);
    responseToTelCore(mclResponse);
    free(response);
    return;

error:
    free(response);
    mclResponse = RfxMclMessage::obtainResponse(msg->getId(),
            RIL_E_GENERIC_FAILURE, RfxVoidData(), msg);
    responseToTelCore(mclResponse);
}

void RmcCommSimOpRequestHandler::handleVzwRsuGetVersion(const sp<RfxMclMessage>& msg) {
    int err = 0;
    int errCode = 0;
    String8 cmd("");

    RfxAtLine *pLine = NULL;
    sp<RfxAtResponse> atResponse;
    sp<RfxMclMessage> mclResponse;

    int opId = -1;
    int requestId = -1;
    char* majorVersion = NULL;
    char* minorVersion = NULL;

    RIL_RsuRequestInfo* request = (RIL_RsuRequestInfo*)(msg->getData()->getData());
    RIL_RsuResponseInfo* response = (RIL_RsuResponseInfo*)calloc(1, sizeof(RIL_RsuResponseInfo));
    RFX_ASSERT(response != NULL);
    response->opId = request->opId;
    response->requestId = request->requestId;

    cmd.append(String8::format("AT+ESMLRSU=%d,%d",
            request->opId, RSU_MODEM_REQUEST_GET_LOCK_VERSION));
    atResponse = atSendCommandSingleline(cmd, "+ESMLRSU:");
    if (atResponse == NULL) {
        logE(mTag, "[RSU-SIMLOCK] handleVzwRsuGetVersion atResponse is null");
        goto error;
    }

    if (atResponse->isAtResponseFail()) {
        errCode = atResponse->atGetCmeError();
        logE(mTag, "[RSU-SIMLOCK] handleVzwRsuGetVersion errCode = %d", errCode);
        response->errCode = errCode;
        mclResponse = RfxMclMessage::obtainResponse(msg->getId(),
                RIL_E_GENERIC_FAILURE,
                RfxOpRsuResponseData(response, sizeof(RIL_RsuResponseInfo)), msg);
        responseToTelCore(mclResponse);
        free(response);
        return;
    }

    pLine = atResponse->getIntermediates();
    if (pLine == NULL) {
        logE(mTag, "[RSU-SIMLOCK] handleVzwRsuGetVersion pLine is null");
        goto error;
    }
    pLine->atTokStart(&err);
    if (err < 0) {
        logE(mTag, "[RSU-SIMLOCK] handleVzwRsuGetVersion atTokStart error");
        goto error;
    }

    opId = pLine->atTokNextint(&err);
    if (err < 0 || opId != request->opId) {
        logE(mTag, "[RSU-SIMLOCK] handleVzwRsuGetVersion err < 0 or opId = %d"
                " not equal ot request opId = %d", opId, request->opId);
        goto error;
    }
    requestId = pLine->atTokNextint(&err);
    if (err < 0 || requestId != RSU_MODEM_REQUEST_GET_LOCK_VERSION) {
        logE(mTag, "[RSU-SIMLOCK] handleVzwRsuGetVersion err < 0 or requestId = %d"
                " not equal ot request requestId = %d", requestId, request->requestId);
        goto error;
    }

    majorVersion = pLine->atTokNextstr(&err);
    if (err < 0 || majorVersion == NULL) {
        logE(mTag, "[RSU-SIMLOCK] handleVzwRsuGetVersion err < 0 or majorVersion is NULL");
        goto error;
    }
    if (pLine->atTokHasmore()) {
        minorVersion = pLine->atTokNextstr(&err);
        if (err < 0 || minorVersion == NULL) {
            logE(mTag, "[RSU-SIMLOCK] handleVzwRsuGetVersion err < 0 or minorVersion is NULL");
            goto error;
        }
    } else {
        logE(mTag, "[RSU-SIMLOCK] handleVzwRsuGetVersion no minorVersion");
        goto error;
    }
    response->version = ((atoi(majorVersion) << 16) + atoi(minorVersion));
    logD(mTag, "[RSU-SIMLOCK] handleVzwRsuGetVersion version = %d", response->version);

    mclResponse = RfxMclMessage::obtainResponse(msg->getId(),
            RIL_E_SUCCESS, RfxOpRsuResponseData(response, sizeof(RIL_RsuResponseInfo)), msg);
    responseToTelCore(mclResponse);
    free(response);
    return;

error:
    free(response);
    mclResponse = RfxMclMessage::obtainResponse(msg->getId(),
            RIL_E_GENERIC_FAILURE, RfxVoidData(), msg);
    responseToTelCore(mclResponse);
}

void RmcCommSimOpRequestHandler::handleVzwRsuGetStatus(const sp<RfxMclMessage>& msg) {
    int err = 0;
    int errCode = 0;
    String8 cmd("");

    RfxAtLine *pLine = NULL;
    sp<RfxAtResponse> atResponse;
    sp<RfxMclMessage> mclResponse;

    int opId = -1;
    int requestId = -1;
    char* unlockStatus = NULL;
    char* unlockTime = NULL;

    RIL_RsuRequestInfo* request = (RIL_RsuRequestInfo*)(msg->getData()->getData());
    RIL_RsuResponseInfo* response = (RIL_RsuResponseInfo*)calloc(1, sizeof(RIL_RsuResponseInfo));
    RFX_ASSERT(response != NULL);
    response->opId = request->opId;
    response->requestId = request->requestId;

    cmd.append(String8::format("AT+ESMLRSU=%d,%d",
            request->opId, RSU_MODEM_REQUEST_GET_LOCK_STATUS));
    atResponse = atSendCommandSingleline(cmd, "+ESMLRSU:");
    if (atResponse == NULL) {
        logE(mTag, "[RSU-SIMLOCK] handleVzwRsuGetStatus atResponse is null");
        goto error;
    }

    if (atResponse->isAtResponseFail()) {
        errCode = atResponse->atGetCmeError();
        logE(mTag, "[RSU-SIMLOCK] handleVzwRsuGetStatus errCode = %d", errCode);
        response->errCode = errCode;
        mclResponse = RfxMclMessage::obtainResponse(msg->getId(),
                RIL_E_GENERIC_FAILURE,
                RfxOpRsuResponseData(response, sizeof(RIL_RsuResponseInfo)), msg);
        responseToTelCore(mclResponse);
        free(response);
        return;
    }

    pLine = atResponse->getIntermediates();
    if (pLine == NULL) {
        logE(mTag, "[RSU-SIMLOCK] handleVzwRsuGetStatus pLine is null");
        goto error;
    }
    pLine->atTokStart(&err);
    if (err < 0) {
        logE(mTag, "[RSU-SIMLOCK] handleVzwRsuGetStatus atTokStart error");
        goto error;
    }

    opId = pLine->atTokNextint(&err);
    if (err < 0 || opId != request->opId) {
        logE(mTag, "[RSU-SIMLOCK] handleVzwRsuGetStatus err < 0 or opId = %d"
                " not equal ot request opId = %d", opId, request->opId);
        goto error;
    }
    requestId = pLine->atTokNextint(&err);
    if (err < 0 || requestId != RSU_MODEM_REQUEST_GET_LOCK_STATUS) {
        logE(mTag, "[RSU-SIMLOCK] handleVzwRsuGetStatus err < 0 or requestId = %d"
                " not equal ot request requestId = %d", requestId, request->requestId);
        goto error;
    }

    unlockStatus = pLine->atTokNextstr(&err);
    if (err < 0 || unlockStatus == NULL) {
        logE(mTag, "[RSU-SIMLOCK] handleVzwRsuGetStatus err < 0 or unlockStatus is NULL");
        goto error;
    }
    response->status = atoi(unlockStatus);
    if (pLine->atTokHasmore()) {
        unlockTime = pLine->atTokNextstr(&err);
        if (err < 0 || unlockTime == NULL) {
            logE(mTag, "[RSU-SIMLOCK] handleVzwRsuGetStatus err < 0 or unlockTime is null");
            goto error;
        }
        response->time = atoll(unlockTime);
    } else {
        logE(mTag, "[RSU-SIMLOCK] handleVzwRsuGetStatus no unlockTime");
        goto error;
    }
    logD(mTag, "[RSU-SIMLOCK] handleVzwRsuGetStatus unlockStatus = %d unlockTime = %lld",
            response->status, response->time);

    mclResponse = RfxMclMessage::obtainResponse(msg->getId(),
            RIL_E_SUCCESS, RfxOpRsuResponseData(response, sizeof(RIL_RsuResponseInfo)), msg);
    responseToTelCore(mclResponse);
    free(response);
    return;

error:
    free(response);
    mclResponse = RfxMclMessage::obtainResponse(msg->getId(),
            RIL_E_GENERIC_FAILURE, RfxVoidData(), msg);
    responseToTelCore(mclResponse);
}

void RmcCommSimOpRequestHandler::handleVzwRsuUnlockTimer(const sp<RfxMclMessage>& msg) {
    int err = 0;
    int errCode = 0;

    String8 cmd("");
    RfxAtLine *pLine = NULL;
    sp<RfxAtResponse> atResponse;
    sp<RfxMclMessage> mclResponse;

    int opId = -1;
    int requestId = -1;
    char* timerStatus = NULL;
    char* passedTime = NULL;

    RIL_RsuRequestInfo* request = (RIL_RsuRequestInfo*)(msg->getData()->getData());
    RIL_RsuResponseInfo* response = (RIL_RsuResponseInfo*)calloc(1, sizeof(RIL_RsuResponseInfo));
    RFX_ASSERT(response != NULL);
    response->opId = request->opId;
    response->requestId = request->requestId;

    cmd.append(String8::format("AT+ESMLRSU=%d,%d,%d",
            request->opId, RSU_MODEM_REQUEST_UNLOCK_TIMER, (request->requestType == 2 ? 0 : 1)));
    atResponse = atSendCommandSingleline(cmd, "+ESMLRSU:");
    if (atResponse == NULL) {
        logE(mTag, "[RSU-SIMLOCK] handleVzwRsuUnlockTimer atResponse is null");
        goto error;
    }

    if (atResponse->isAtResponseFail()) {
        errCode = atResponse->atGetCmeError();
        logE(mTag, "[RSU-SIMLOCK] handleVzwRsuUnlockTimer errCode = %d", errCode);
        response->errCode = errCode;
        mclResponse = RfxMclMessage::obtainResponse(msg->getId(),
                RIL_E_GENERIC_FAILURE,
                RfxOpRsuResponseData(response, sizeof(RIL_RsuResponseInfo)), msg);
        responseToTelCore(mclResponse);
        free(response);
        return;
    }

    pLine = atResponse->getIntermediates();
    if (pLine == NULL) {
        logE(mTag, "[RSU-SIMLOCK] handleVzwRsuUnlockTimer pLine is null");
        goto error;
    }
    pLine->atTokStart(&err);
    if (err < 0) {
        logE(mTag, "[RSU-SIMLOCK] handleVzwRsuUnlockTimer atTokStart error");
        goto error;
    }

    opId = pLine->atTokNextint(&err);
    if (err < 0 || opId != request->opId) {
        logE(mTag, "[RSU-SIMLOCK] handleVzwRsuUnlockTimer er < 0 or opId = %d"
                " not equal ot request opId = %d", opId, request->opId);
        goto error;
    }
    requestId = pLine->atTokNextint(&err);
    if (err < 0 || requestId != RSU_MODEM_REQUEST_UNLOCK_TIMER) {
        logE(mTag, "[RSU-SIMLOCK] handleVzwRsuUnlockTimer err < 0 or requestId = %d"
                " not equal ot request requestId = %d", requestId, request->requestId);
        goto error;
    }

    timerStatus = pLine->atTokNextstr(&err);
    if (err < 0 || timerStatus == NULL) {
        logE(mTag, "[RSU-SIMLOCK] handleVzwRsuUnlockTimer err < 0 or timerStatus is NULL");
        goto error;
    }
    response->errCode = atoi(timerStatus);
    if (pLine->atTokHasmore()) {
        passedTime = pLine->atTokNextstr(&err);
        if (err < 0 || passedTime == NULL) {
            logE(mTag, "[RSU-SIMLOCK] handleVzwRsuUnlockTimer err < 0 or passedTime is null");
            goto error;
        }
        response->time = atoll(passedTime);
    } else {
        logE(mTag, "[RSU-SIMLOCK] handleVzwRsuUnlockTimer no passedTime");
        goto error;
    }
    logD(mTag, "[RSU-SIMLOCK] handleVzwRsuUnlockTimer timerStatus = %d passedTime = %lld",
            response->status, response->time);

    mclResponse = RfxMclMessage::obtainResponse(msg->getId(),
            RIL_E_SUCCESS, RfxOpRsuResponseData(response, sizeof(RIL_RsuResponseInfo)), msg);
    responseToTelCore(mclResponse);
    free(response);
    return;

error:
    free(response);
    mclResponse = RfxMclMessage::obtainResponse(msg->getId(),
            RIL_E_GENERIC_FAILURE, RfxVoidData(), msg);
    responseToTelCore(mclResponse);
}
