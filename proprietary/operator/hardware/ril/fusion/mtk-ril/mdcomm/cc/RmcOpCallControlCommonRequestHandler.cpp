/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
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

#include "RmcOpCallControlCommonRequestHandler.h"

#include "RfxCallListData.h"
#include "RfxIntsData.h"
#include "RfxRilUtils.h"
#include "RfxStringsData.h"
#include "RfxVoidData.h"
#include "libmtkrilutils.h"

#define RFX_LOG_TAG "RmcOpCCReqHandler"

RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringsData, RfxVoidData, RFX_MSG_REQUEST_SET_INCOMING_VIRTUAL_LINE);
RFX_REGISTER_DATA_TO_EVENT_ID(RfxIntsData, RFX_MSG_EVENT_VIRTUAL_LINE_TIMEOUT);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringsData, RfxVoidData, RFX_MSG_REQUEST_SET_TRN);

RFX_IMPLEMENT_OP_HANDLER_CLASS(RmcOpCallControlCommonRequestHandler, RIL_CMD_PROXY_2);

char RmcOpCallControlCommonRequestHandler::cachedVirtualFrom[MAX_INCOMING_LINE_LENGTH] = {0};
char RmcOpCallControlCommonRequestHandler::cachedVirtualTo[MAX_INCOMING_LINE_LENGTH] = {0};
int RmcOpCallControlCommonRequestHandler::cachedVirtualToken = 0;
int RmcOpCallControlCommonRequestHandler::mtCallCount = 0;

RmcOpCallControlCommonRequestHandler::RmcOpCallControlCommonRequestHandler(int slot_id,
        int channel_id) : RmcCallControlCommonRequestHandler(slot_id, channel_id) {
    const int requests[] = {
        RFX_MSG_REQUEST_GET_CURRENT_CALLS,         //AT+CLCC
        RFX_MSG_REQUEST_SET_INCOMING_VIRTUAL_LINE,
        RFX_MSG_REQUEST_SET_TRN
    };

    const int events[] = {
        RFX_MSG_EVENT_VIRTUAL_LINE_TIMEOUT
    };

    registerToHandleRequest(requests, sizeof(requests)/sizeof(int));
    registerToHandleEvent(events, sizeof(events) / sizeof(int));
}

RmcOpCallControlCommonRequestHandler::~RmcOpCallControlCommonRequestHandler() {
}

void RmcOpCallControlCommonRequestHandler::onHandleRequest(const sp<RfxMclMessage>& msg) {
    int id = msg->getId();
    //logD(RFX_LOG_TAG, "onHandleRequest: %s", RFX_ID_TO_STR(id));
    switch(id) {
        case RFX_MSG_REQUEST_GET_CURRENT_CALLS:
            if (isOp08Support() && RfxRilUtils::isDigitsSupport()) {
                requestOp08GetCurrentCalls(msg);
            } else {
                RmcCallControlCommonRequestHandler::requestGetCurrentCalls(msg);
            }
            break;
        case RFX_MSG_REQUEST_SET_INCOMING_VIRTUAL_LINE:
            if (isOp08Support() && RfxRilUtils::isDigitsSupport()) {
                setIncomingVirtualLine(msg);
            }
            break;
        case RFX_MSG_REQUEST_SET_TRN:
            setTrn(msg);
            break;
        default:
            RmcCallControlCommonRequestHandler::onHandleRequest(msg);
            break;
    }
}

void RmcOpCallControlCommonRequestHandler::onHandleEvent(const sp<RfxMclMessage>& msg) {
    int id = msg->getId();
    //logD(RFX_LOG_TAG, "onHandleEvent: %s", RFX_ID_TO_STR(id));
    switch (id) {
        case RFX_MSG_EVENT_VIRTUAL_LINE_TIMEOUT:
            handleVirtualLineTimeout(msg);
            break;
        default:
            RmcCallControlCommonRequestHandler::onHandleEvent(msg);
            break;
    }
}

void RmcOpCallControlCommonRequestHandler::requestOp08GetCurrentCalls(const sp<RfxMclMessage>& msg) {
    sp<RfxAtResponse> p_response;
    RfxAtLine *p_cur;
    RIL_Call *p_calls;
    RIL_Call **pp_calls;
    int countCalls = 0;
    int countValidCalls = 0;
    int ret = -1;
    bool tryAsClcc = false;

    /// Digits specific
    mtCallCount = 0;

    // TODO: Will use AT+CLCCS instead for supporting SIP uri
    if (isGsmRat()) {
        p_response = atSendCommandMultiline(String8::format("AT+CLCCS"), "+CLCCS:");
        // retry with AT+CLCC if fails on AT+CLCCS
        if (p_response->isAtResponseFail()) {
            p_response = NULL;
            tryAsClcc = true;
        }
    } else {  // Rat is Cdma or no-service
        tryAsClcc = true;
    }

    if (tryAsClcc) {
        p_response = atSendCommandMultiline(String8::format("AT+CLCC"), "+CLCC:");
    }

    // set result
    if (p_response->getError() != 0 || p_response->getSuccess() != 1) {
        clearCnap();
        responseVoidDataToTcl(msg, RIL_E_INTERNAL_ERR);
        return;
    }

    // Calculate the count of "+CLCC" response
    for (p_cur = p_response->getIntermediates(); p_cur != NULL; p_cur = p_cur->getNext()) {
        countCalls++;
    }

    pp_calls = (RIL_Call **)alloca(countCalls * sizeof(RIL_Call *));
    RFX_ASSERT(pp_calls != NULL);
    p_calls = (RIL_Call *)alloca(countCalls * sizeof(RIL_Call));
    RFX_ASSERT(p_calls != NULL);
    memset(p_calls, 0, countCalls * sizeof(RIL_Call));

    /* init the pointer array */
    for (int i = 0; i < countCalls; i++) {
        pp_calls[i] = &(p_calls[i]);
    }

    for (p_cur = p_response->getIntermediates(); p_cur != NULL; p_cur = p_cur->getNext()) {
        char *line = p_cur->getLine();
        if (RfxRilUtils::isUserLoad() != 1) {
            logD(RFX_LOG_TAG, "line:%s", line);
        }
        p_calls[countValidCalls].name = (char *)alloca(MAX_CNAP_LENGTH);
        RFX_ASSERT(p_calls[countValidCalls].name != NULL);
        memset(p_calls[countValidCalls].name, 0, MAX_CNAP_LENGTH);
        if (tryAsClcc) {
            ret = callFromCLCCLineOp08(p_cur, p_calls + countValidCalls);
        } else {
            ret = callFromCLCCSLineOp08(p_cur, p_calls + countValidCalls);
        }

        if (ret != 0) {
            continue;
        }

        countValidCalls++;
    }

    /// Digits specific start @{
    // Clear cache only when no MT call exists, becasue CallTracker keeps polling call state,
    // and only the last poll is used to update call state.
    if (mtCallCount == 0) {
        memset(cachedVirtualFrom, 0, MAX_INCOMING_LINE_LENGTH);
        memset(cachedVirtualTo, 0, MAX_INCOMING_LINE_LENGTH);
        cachedVirtualToken++;
    }
    /// Digits specific end @}

    if (countValidCalls == 0) {
        clearCnap();
    }

    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxCallListData(pp_calls, sizeof(RIL_Call *) * countValidCalls), msg);
    responseToTelCore(response);
}

/**
 * Note: directly modified line and has *p_call point directly into
 * modified line
 */
int RmcOpCallControlCommonRequestHandler::callFromCLCCSLineOp08(
        RfxAtLine *pLine, RIL_Call *p_call) {
    //+CLCCS: <ccid1>,<dir>,<neg_status_present>,<neg_status>,<SDP_md>,<cs_mode>,<ccstatus>,<mpty>
    //              [,<numbertype>,<ton>,<number>[,<priority_present>,<priority>
    //              [,<CLI_validity_present>,<CLI_validity>]]]
    //+CLCCS: 1,0,,,,1,2,0,2,129,"0800000123",,,0,0

    int ret;
    int state;
    int mode;
    int dummy;

    pLine->atTokStart(&ret);
    if (ret < 0) { goto error; }

    p_call->index = pLine->atTokNextint(&ret);  // <ccid>
    if (ret < 0) { goto error; }
    //logD(RFX_LOG_TAG, "ccid=%d", p_call->index);

    p_call->isMT = pLine->atTokNextint(&ret);  // <dir>
    if (ret < 0) { goto error; }
    //logD(RFX_LOG_TAG, "dir=%d", p_call->isMT);

    dummy = pLine->atTokNextint(&ret);  // <neg_status_present>
    dummy = pLine->atTokNextint(&ret);  // <neg_status>
    dummy = pLine->atTokNextint(&ret);  // <SDP_md>

    /*
    0   no relevant information about bearer/teleservice
    1   voice
    2   data
    3   fax
    255 unknown
    */
    mode = pLine->atTokNextint(&ret);  // <cs_mode>
    if (ret < 0) { goto error; }
    //logD(RFX_LOG_TAG, "cs_mode=%d", mode);

    p_call->isVoice = (mode == 1);

    if (mode >= 20) {
        logD(RFX_LOG_TAG, "Skip call with mode>=20, i.e. IMS/WFC call");
        return -1;
    }

    state = pLine->atTokNextint(&ret);  // <ccstatus>
    if (ret < 0) { goto error; }
    //logD(RFX_LOG_TAG, "ccstatus=%d", state);

    ret = clccsStateToRILState(state, &(p_call->state));
    if (ret < 0) { goto error; }

    // CNAP URC is sent after RING and before CLCC polling,
    // therefore we create a cached array to store the name of MT call, with the cached CNAP,
    // so that the name in subsequent CLCC polling can be filled with the cached CNAP array,
    // even the call becomes ACTIVE later.
    if (p_call->state == RIL_CALL_INCOMING || p_call->state == RIL_CALL_WAITING) {
        if ((p_call->index >= 1) && (p_call->index <= MAX_GSMCALL_CONNECTIONS)) {
            memset(cachedClccName[p_call->index-1], 0, MAX_CNAP_LENGTH);
            if (strlen(cachedCnap) > 0) {
                copyString(cachedClccName[p_call->index-1], cachedCnap, MAX_CNAP_LENGTH);
                memset(cachedCnap, 0, MAX_CNAP_LENGTH);
            }
        }
    }

    // Fill CNAP in individual CLCC parsing and clear in ECPI:133
    if ((p_call->index >= 1) && (p_call->index <= MAX_GSMCALL_CONNECTIONS)) {
        if (strlen(cachedClccName[p_call->index-1]) > 0) {
            copyString(p_call->name, cachedClccName[p_call->index-1], MAX_CNAP_LENGTH);
        }
    }


    p_call->isMpty = pLine->atTokNextint(&ret);  // <mpty>
    if (ret < 0) { goto error; }
    //logD(RFX_LOG_TAG, "mpty=%d", p_call->isMpty);

    if (pLine->atTokHasmore()) {
        int numbertype = pLine->atTokNextint(&ret);  // <numbertype>
        if (ret < 0) { goto error; }
        //logD(RFX_LOG_TAG, "numbertype=%d", numbertype);

        p_call->toa = pLine->atTokNextint(&ret);  // <ton>
        if (ret < 0) { goto error; }
        //logD(RFX_LOG_TAG, "ton=%d", p_call->toa);

        /// Digits specific start @{
        char *origNum = pLine->atTokNextstr(&ret);

        /* tolerate RfxStringData("") here */
        if (ret < 0) return 0;

        origNum = extractSipUri(origNum);

        if (origNum == NULL) {
            origNum = "";
        }

        logD(RFX_LOG_TAG, "number=%s", origNum);
        if (p_call->state == RIL_CALL_INCOMING || p_call->state == RIL_CALL_WAITING) {
            mtCallCount++;
            // TODO:
            // If MT side makes a Digits call first & ends at once, then makes a non-Digits call,
            // Cached virtual from_number matches MT info, but should not be taken as Digits call.
            if (strlen(cachedVirtualTo) != 0 &&
                    strncmp(cachedVirtualFrom, origNum, strlen(origNum)) == 0) {
                int len = strlen(origNum) + strlen(cachedVirtualTo) + 4;
                p_call->number = (char *)calloc(len + 1, sizeof(char));
                RFX_ASSERT(p_call->number != NULL);
                strncpy(p_call->number, origNum, strlen(origNum));
                strncat(p_call->number, "_to_", 4);
                strncat(p_call->number, cachedVirtualTo, len - strlen(p_call->number));
                logD(RFX_LOG_TAG, "number(converted)=%s", p_call->number);
            } else {
                p_call->number = origNum;
            }
        } else {
            p_call->number = origNum;
        }
        /// Digits specific end @}
    }

    if (pLine->atTokHasmore()) {
        dummy = pLine->atTokNextint(&ret);  // <priority_present>
        dummy = pLine->atTokNextint(&ret);  // <priority>
    }

    if (pLine->atTokHasmore()) {
        int cliPresent = pLine->atTokNextint(&ret);  // <cli_validity_present>
        if (ret < 0) { goto error; }
        //logD(RFX_LOG_TAG, "cli_validity_present=%d", cliPresent);

        /*
        0 CLI valid
        1 CLI has been withheld by the originator"Reject by user"
        2 CLI is not available due to interworking problems or limitations of originating network
        3 CLI is not available due to calling party being of type payphone
        4 CLI is not available due to other reasons
        */
        p_call->numberPresentation = pLine->atTokNextint(&ret);  // <cli_validity>
        if (ret < 0) { goto error; }
        /* Issue: ALPS03500092
           Google AOSP only supports 0~3, so re-map 4 or later value to 2(UNKNOWN)
         */
        if (p_call->numberPresentation > 3) {
            p_call->numberPresentation = 2;
        }
    }

    p_call->uusInfo = NULL;

    return 0;

error:
    logE(RFX_LOG_TAG, "invalid CLCC line");
    return -1;
}


int RmcOpCallControlCommonRequestHandler::callFromCLCCLineOp08(
        RfxAtLine *pLine, RIL_Call *p_call) {
    // +CLCC: 1,0,2,0,0,\"+18005551212\",145
    // +CLCC: <id>,<dir>,<stat>,<mode>,<mpty>,<number>,<toa>,,,<CLI validity>
    // Optional parameters <alpha> and <priority> not present.

    int ret;
    int state;
    int mode;

    pLine->atTokStart(&ret);
    if (ret < 0) {
        logE(RFX_LOG_TAG, "invalid CLCC line");
        return -1;
    }

    p_call->index = pLine->atTokNextint(&ret);
    if (ret < 0) {
        logE(RFX_LOG_TAG, "invalid CLCC line");
        return -1;
    }

    p_call->isMT = pLine->atTokNextint(&ret);
    if (ret < 0) {
        logE(RFX_LOG_TAG, "invalid CLCC line");
        return -1;
    }

    state = pLine->atTokNextint(&ret);
    if (ret < 0) {
        logE(RFX_LOG_TAG, "invalid CLCC line");
        return -1;
    }

    ret = clccStateToRILState(state, &(p_call->state));
    if (ret < 0) {
        logE(RFX_LOG_TAG, "invalid CLCC line");
        return -1;
    }

    // CNAP URC is sent after RING and before CLCC polling,
    // therefore we create a cached array to store the name of MT call, with the cached CNAP,
    // so that the name in subsequent CLCC polling can be filled with the cached CNAP array,
    // even the call becomes ACTIVE later.
    if (p_call->state == RIL_CALL_INCOMING || p_call->state == RIL_CALL_WAITING) {
        if ((p_call->index >= 1) && (p_call->index <= MAX_GSMCALL_CONNECTIONS)) {
            //TODO: Skip MT if ECPI0 or ECPI4 not received
            /*
            logD(RFX_LOG_TAG, "ecpiValue[%d] = %d", p_call->index-1, ecpiValue[p_call->index-1]);
            if ((ecpiValue[p_call->index-1] != 0) && (ecpiValue[p_call->index-1] != 4)) {
                logD(RFX_LOG_TAG, "Skip MT call if ECPI0 or ECPI4 is not received");
                goto error;
            }
            */

            memset(cachedClccName[p_call->index-1], 0, MAX_CNAP_LENGTH);
            if (strlen(cachedCnap) > 0) {
                copyString(cachedClccName[p_call->index-1], cachedCnap, MAX_CNAP_LENGTH);
                memset(cachedCnap, 0, MAX_CNAP_LENGTH);
            }
        }
    }

    // Fill CNAP in individual CLCC parsing and clear in ECPI:133
    if ((p_call->index >= 1) && (p_call->index <= MAX_GSMCALL_CONNECTIONS)) {
        if (strlen(cachedClccName[p_call->index-1]) > 0) {
            copyString(p_call->name, cachedClccName[p_call->index-1], MAX_CNAP_LENGTH);
        }
    }

    mode = pLine->atTokNextint(&ret);
    if (ret < 0) {
        logE(RFX_LOG_TAG, "invalid CLCC line");
        return -1;
    }

    p_call->isVoice = (mode == 0);

    if (mode >= 20) {
        logD(RFX_LOG_TAG, "Skip call with mode>=20, i.e. IMS/WFC call");
        return -1;
    }

    p_call->isMpty = pLine->atTokNextint(&ret);
    if (ret < 0) {
        logE(RFX_LOG_TAG, "invalid CLCC line");
        return -1;
    }

    if (pLine->atTokHasmore()) {
        /// Digits specific start @{
        char *origNum = pLine->atTokNextstr(&ret);

        /* tolerate RfxStringData("") here */
        if (ret < 0) return 0;

        // Some lame implementations return strings
        // like "NOT AVAILABLE" in the CLCC line
        if ((origNum != NULL) &&
            (strspn(origNum, "+*#pw0123456789") != strlen(origNum))) {
            origNum = NULL;
        }

        if (origNum == NULL) {
            origNum = "";
        }

        logD(RFX_LOG_TAG, "number=%s", origNum);
        if (p_call->state == RIL_CALL_INCOMING || p_call->state == RIL_CALL_WAITING) {
            mtCallCount++;
            // TODO:
            // If MT side makes a Digits call first & ends at once, then makes a non-Digits call,
            // Cached virtual from_number matches MT info, but should not be taken as Digits call.
            if (strlen(cachedVirtualTo) != 0 &&
                    strncmp(cachedVirtualFrom, origNum, strlen(origNum)) == 0) {
                int len = strlen(origNum) + strlen(cachedVirtualTo) + 4;
                p_call->number = (char *)calloc(len + 1, sizeof(char));
                RFX_ASSERT(p_call->number != NULL);
                strncpy(p_call->number, origNum, strlen(origNum));
                strncat(p_call->number, "_to_", 4);
                strncat(p_call->number, cachedVirtualTo, len - strlen(p_call->number));
                logD(RFX_LOG_TAG, "number(converted)=%s", p_call->number);
            } else {
                p_call->number = origNum;
            }
        } else {
            p_call->number = origNum;
        }
        /// Digits specific end @}

        p_call->toa = pLine->atTokNextint(&ret);
        if (ret < 0) {
            logE(RFX_LOG_TAG, "invalid CLCC line");
            return -1;
        }
    }

    if (pLine->atTokHasmore()) {
        int dummy;
        dummy = pLine->atTokNextint(&ret);  // <alpha>
        dummy = pLine->atTokNextint(&ret);  // <priority>
        p_call->numberPresentation = pLine->atTokNextint(&ret);
        if (ret < 0) {
            logE(RFX_LOG_TAG, "invalid CLCC line");
            return -1;
        }
        /* Issue: ALPS03500092
           Google AOSP only supports 0~3, so re-map 4 or later value to 2(UNKNOWN)
         */
        if (p_call->numberPresentation > 3) {
            p_call->numberPresentation = 2;
        }
    }

    p_call->uusInfo = NULL;

    return 0;
}

void RmcOpCallControlCommonRequestHandler::setIncomingVirtualLine(const sp<RfxMclMessage>& msg) {
    char** data = (char**)msg->getData()->getData();

    // data[0] is fromMsisdn
    // data[1] is toMsisdn

    // Remarks#1:
    // RCS set MT call destination virtual Line number to RILD.
    // RILD caches the number, and return OK to RCS.
    // Upon receiving OK from RILD, RCS proceeds SIP 200 OK to network server.
    // Network server thens proceed MT call as a normal CS call (sending notification to CS modem)
    // The sequence is guaranteed by network server, not by DUT

    // Remarks#2: Only handle "+" sign mapping
    // The fromMsisdn from RCS has "+" sign while number in CLCC has no "+" sign,
    // even the TOA is international, so remove "+" sign before comparison

    // Remarks#3:
    // TODO: Other number mapping, such as "00886" to "886" is not handled here

    // fromMsisdn
    if (data[0] != NULL && strlen(data[0]) > 0) {
        if (!strncmp((const char *)data[0], "+", 1)) {
            // erase "+" sign
            snprintf(cachedVirtualFrom, MAX_INCOMING_LINE_LENGTH - 1, "%s", data[0] + 1);
        } else {
            snprintf(cachedVirtualFrom, MAX_INCOMING_LINE_LENGTH - 1, "%s", data[0]);
        }
    }
    //toMsisdn
    if (data[1] != NULL && strlen(data[1]) > 0) {
        snprintf(cachedVirtualTo, MAX_INCOMING_LINE_LENGTH - 1, "%s", data[1]);
    }
    cachedVirtualToken++;
    logD(RFX_LOG_TAG, "virtualLine from=%s, to=%s, token=%d",
            cachedVirtualFrom, cachedVirtualTo, cachedVirtualToken);


    int token[1];
    token[0] = cachedVirtualToken;
    sendEvent(RFX_MSG_EVENT_VIRTUAL_LINE_TIMEOUT, RfxIntsData(token, 1), RIL_CMD_PROXY_2, m_slot_id,
        -1, -1, ms2ns(INCOMING_LINE_TIMEOUT));

    responseVoidDataToTcl(msg, RIL_E_SUCCESS);
}

void RmcOpCallControlCommonRequestHandler::handleVirtualLineTimeout(const sp<RfxMclMessage>& msg) {
    int token = ((int *) msg->getData()->getData())[0];
    if (token == cachedVirtualToken) {
        logD(RFX_LOG_TAG, "handleVirtualLineTimeout from=%s, to=%s, token=%d",
                cachedVirtualFrom, cachedVirtualTo, cachedVirtualToken);
        memset(cachedVirtualFrom, 0, MAX_INCOMING_LINE_LENGTH);
        memset(cachedVirtualTo, 0, MAX_INCOMING_LINE_LENGTH);
    }
}

// For Modem to query TRN from AP during CSFB
void RmcOpCallControlCommonRequestHandler::setTrn(const sp<RfxMclMessage>& msg) {
    char** params = (char**)msg->getData()->getData();
    char* fromMsisdn = params[0];
    char* toMsisdn = params[1];
    char* trn = params[2];

    int callId = getMclStatusManager()->getIntValue(RFX_STATUS_KEY_TRN_CALLID);
    getMclStatusManager()->setIntValue(RFX_STATUS_KEY_TRN_CALLID, -1);

    char* atCmd = "AT+DIGITSTRN";

    /* AT+DIGITSTRN=<call_id>,<TRN>,<toMsisdn>,<fromMsisdn> */
    String8 cmd = String8::format("%s=%d,\"%s\",\"%s\",\"%s\"",
            atCmd, callId, trn, toMsisdn, fromMsisdn);
    handleCmdWithVoidResponse(msg, cmd);
}
