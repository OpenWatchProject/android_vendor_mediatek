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
// MTK fusion include
#include <telephony/mtk_ril.h>
#include "RfxCallListData.h"
#include "RfxRedialData.h"
#include "RfxVoidData.h"
#include "RfxCallFailCauseData.h"
#include "RfxIntsData.h"
#include "RfxRilUtils.h"

#include "RfxViaUtils.h"

#include "RfxGwsdUtils.h"

// local include
#include "RmcCallControlCommonRequestHandler.h"
#include "RmcCallControlImsRequestHandler.h"
#include "RmcCallControlInterface.h"
#include "RmcCallControlUrcHandler.h"

#include <mtkconfigutils.h>


#define RFX_LOG_TAG "RmcCCReqHandler"

// register handler to channel
RFX_IMPLEMENT_OP_PARENT_HANDLER_CLASS(RmcCallControlCommonRequestHandler, RIL_CMD_PROXY_2);

RmcCallControlCommonRequestHandler::RmcCallControlCommonRequestHandler(int slot_id,
        int channel_id) : RmcCallControlBaseHandler(slot_id, channel_id){
    const int requests[] = {
        RFX_MSG_REQUEST_GET_CURRENT_CALLS,         //AT+CLCC
        RFX_MSG_REQUEST_DIAL,                      //ATD
        RFX_MSG_REQUEST_ANSWER,                    //ATA
        RFX_MSG_REQUEST_UDUB,                      //ATH
        RFX_MSG_REQUEST_LAST_CALL_FAIL_CAUSE,      //AT+CEER
        RFX_MSG_REQUEST_DTMF,                      //AT+VTS
        RFX_MSG_REQUEST_DTMF_START,                //AT+EVTS
        RFX_MSG_REQUEST_DTMF_STOP,                 //AT+EVTS
        RFX_MSG_REQUEST_SET_MUTE,                  //AT+CMUT
        RFX_MSG_REQUEST_GET_MUTE,                  //AT+CMUT?
        RFX_MSG_REQUEST_SET_TTY_MODE,              //AT+CTMCALL
        RFX_MSG_REQUEST_QUERY_TTY_MODE,            //AT+CTMCALL?
        RFX_MSG_REQUEST_SET_CALL_INDICATION,       //AT+EAIC
        RFX_MSG_REQUEST_LOCAL_SET_ECC_SERVICE_CATEGORY,  //AT+ESVC
        RFX_MSG_REQUEST_LOCAL_EMERGENCY_DIAL,            //ATDE
        RFX_MSG_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE,//AT+VMEMEXIT
        /// C2K specific start
        RFX_MSG_REQUEST_CDMA_SET_PREFERRED_VOICE_PRIVACY_MODE,      //AT+VP
        RFX_MSG_REQUEST_CDMA_QUERY_PREFERRED_VOICE_PRIVACY_MODE,    //AT+VP?
        RFX_MSG_REQUEST_CDMA_FLASH,                                 //AT+CFSH
        RFX_MSG_REQUEST_CDMA_BURST_DTMF,                            //AT+VTS
        /// C2K specific end
        RFX_MSG_REQUEST_ECC_PREFERRED_RAT,
        /// redial message @{
        RFX_MSG_REQUEST_EMERGENCY_REDIAL,          //ATDER
        RFX_MSG_REQUEST_EMERGENCY_SESSION_END,     //AT+EMCS=0
        /// @}
        RFX_MSG_REQUEST_LOCAL_CURRENT_STATUS,      //AT+EMCS=1,x
        RFX_MSG_REQUEST_IMS_DIAL,
        RFX_MSG_REQUEST_IMS_VT_DIAL,
        RFX_MSG_REQUEST_LOCAL_IMS_EMERGENCY_DIAL,
    };

    const int events[] = {
        RFX_MSG_EVENT_CNAP_UPDATE,
        RFX_MSG_EVENT_CLEAR_CLCCNAME,
    };

    /* To check if voica call feature is disabled */
    FeatureValue featurevalue;
    memset(featurevalue.value, 0, sizeof(featurevalue.value));
    mtkGetFeature(CONFIG_CC, &featurevalue);
    if (strcmp(featurevalue.value, "1")) {   //Not equal to "1" means disable call feature
        logE(RFX_LOG_TAG, "CONFIG_CC is set to 0, disable voice call feature!");
        atSendCommand("AT+ECFGSET=\"disable_cs_call\",\"1\"");
        return;
    } else {
        atSendCommand("AT+ECFGSET=\"disable_cs_call\",\"0\"");
        registerToHandleRequest(requests, sizeof(requests)/sizeof(int));
        registerToHandleEvent(events, sizeof(events) / sizeof(int));
        if (RfxGwsdUtils::getCallControlHandler() != NULL) {
            RfxGwsdUtils::getCallControlHandler()->registerForGwsdRequest(this);
        }
        if (RfxGwsdUtils::getCallControlHandler() != NULL) {
            RfxGwsdUtils::getCallControlHandler()->registerForGwsdEvent(this);
        }
    }

    queryEmciSupport();

    // Todo: if MTK IMS support
    mImsCCReqHdlr = new RmcCallControlImsRequestHandler(slot_id, channel_id);

    /* Todo: Send the following commands to modem */
    /* Alternating voice/data off */
    atSendCommand("AT+CMOD=0");
    /* No auto-answer */
    atSendCommand("ATS0=0");
    /* Call Waiting notifications */
    atSendCommand("AT+CCWA=1");
    /* Not muted */
    atSendCommand("AT+CMUT=0");
    /* Enable Call Progress notifications */
    atSendCommand("AT+ECPI=4294967295");
    /* Enable Approval Incoming Call notifications */
    atSendCommand("AT+EAIC=2");
    /// M: CC: GSA HD Voice for 2/3G network support
    atSendCommand("AT+EVOCD=1");
    /* Enable of disable VT according to feature option */
#ifdef MTK_VT3G324M_SUPPORT
    atSendCommand("AT+ECCP=0");
    atSendCommand("AT+CRC=1");
    atSendCommand("AT+CBST=134,1,0");
#else
    atSendCommand("AT+ECCP=1");
#endif

    // Set AP full control ECC mode to MD
    // After send this command, AP will not sync ECC table to MD by (+EECCUD)
    atSendCommand("AT+EECCFC=1");

    bUseLocalCallFailCause = 0;
    dialLastError = 0;
}

RmcCallControlCommonRequestHandler::~RmcCallControlCommonRequestHandler() {
    if (mImsCCReqHdlr != NULL) {
        delete(mImsCCReqHdlr);
    }
}

void RmcCallControlCommonRequestHandler::onHandleRequest(const sp<RfxMclMessage>& msg) {
    if (RfxRilUtils::isUserLoad() != 1) {
        logD(RFX_LOG_TAG, "onHandleRequest: %s", RFX_ID_TO_STR(msg->getId()));
    }

    if (RfxGwsdUtils::getCallControlHandler() != NULL) {
        if (RfxGwsdUtils::getCallControlHandler()->handleGwsdRequest(this, msg)) {
            return;
        }
    }

    int request = msg->getId();
    switch (request) {
        case RFX_MSG_REQUEST_GET_CURRENT_CALLS:
            requestGetCurrentCalls(msg);
            break;

        case RFX_MSG_REQUEST_DIAL:
        case RFX_MSG_REQUEST_IMS_DIAL:
            requestDial(msg, false, false, (request == RFX_MSG_REQUEST_IMS_DIAL));
            break;

        case RFX_MSG_REQUEST_ANSWER:
            requestAnswer(msg);
            break;

        case RFX_MSG_REQUEST_UDUB:
            requestUdub(msg);
            break;

        case RFX_MSG_REQUEST_LAST_CALL_FAIL_CAUSE:
            requestLastCallFailCause(msg);
            break;

        case RFX_MSG_REQUEST_DTMF:
            requestDtmf(msg);
            break;

        case RFX_MSG_REQUEST_DTMF_START:
            requestDtmfStart(msg);
            break;

        case RFX_MSG_REQUEST_DTMF_STOP:
            requestDtmfStop(msg);
            break;

        case RFX_MSG_REQUEST_SET_MUTE:
            requestSetMute(msg);
            break;

        case RFX_MSG_REQUEST_GET_MUTE:
            requestGetMute(msg);
            break;

        case RFX_MSG_REQUEST_SET_TTY_MODE:
            requestSetTtyMode(msg);
            break;

        case RFX_MSG_REQUEST_QUERY_TTY_MODE:
            requestQueryTtyMode(msg);
            break;

        case RFX_MSG_REQUEST_SET_CALL_INDICATION:
            requestSetCallIndication(msg);
            break;

        case RFX_MSG_REQUEST_LOCAL_SET_ECC_SERVICE_CATEGORY:
            requestLocalSetEccServiceCategory(msg);
            break;

        case RFX_MSG_REQUEST_LOCAL_EMERGENCY_DIAL:
        case RFX_MSG_REQUEST_LOCAL_IMS_EMERGENCY_DIAL:
            requestDial(msg, true, false, (request == RFX_MSG_REQUEST_LOCAL_IMS_EMERGENCY_DIAL));
            break;

        case RFX_MSG_REQUEST_IMS_VT_DIAL:
            requestDial(msg, false, true, true);
            break;

        case RFX_MSG_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE:
            requestExitEmergencyCallbackMode(msg);
            break;

        /// C2K specific start
        case RFX_MSG_REQUEST_CDMA_SET_PREFERRED_VOICE_PRIVACY_MODE:
            requestSetPreferredVoicePrivacyMode(msg);
            break;

        case RFX_MSG_REQUEST_CDMA_QUERY_PREFERRED_VOICE_PRIVACY_MODE:
            requestQueryPreferredVoicePrivacyMode(msg);
            break;

        case RFX_MSG_REQUEST_CDMA_FLASH:
            requestFlash(msg);
            break;

        case RFX_MSG_REQUEST_CDMA_BURST_DTMF:
            requestBurstDtmf(msg);
            break;
        /// C2K specific end

        /// redial message @{
        case RFX_MSG_REQUEST_EMERGENCY_REDIAL:
            requestEmergencyRedial(msg);
            break;

        case RFX_MSG_REQUEST_EMERGENCY_SESSION_END:
            requestNotifyEmergencySession(msg, false);
            break;
        /// @}

        case RFX_MSG_REQUEST_ECC_PREFERRED_RAT:
            requestEccPreferredRat(msg);
            break;

        case RFX_MSG_REQUEST_LOCAL_CURRENT_STATUS:
            requestLocalCurrentStatus(msg);
            break;

        default:
            logE(RFX_LOG_TAG, "Should not be here");
            break;
    }
}

void RmcCallControlCommonRequestHandler::onHandleEvent(const sp<RfxMclMessage>& msg) {
    if (RfxRilUtils::isUserLoad() != 1) {
        logD(RFX_LOG_TAG, "onHandleEvent: %d", msg->getId());
    }

    if (RfxGwsdUtils::getCallControlHandler() != NULL) {
        if (RfxGwsdUtils::getCallControlHandler()->handleGwsdEvent(this, msg)) {
            return;
        }
    }

    int id = msg->getId();
    switch (id) {
        case RFX_MSG_EVENT_CNAP_UPDATE:
            handleCnapUpdate(msg);
            break;
        case RFX_MSG_EVENT_CLEAR_CLCCNAME:
            handleClearClccName(msg);
            break;
        default:
            logE(RFX_LOG_TAG, "Should not be here");
            break;
    }
}

void RmcCallControlCommonRequestHandler::onHandleTimer() {
    // do something
}

void RmcCallControlCommonRequestHandler::requestGetCurrentCalls(const sp<RfxMclMessage>& msg) {
    // GWSD
    if (RfxGwsdUtils::getCallControlHandler() != NULL) {
        if (RfxGwsdUtils::getCallControlHandler()->fakeGetCurrentCalls(this, msg,
                RmcCallControlUrcHandler::getSpeechCodec())) {
            return;
        }
    }

    sp<RfxAtResponse> p_response;
    RfxAtLine *p_cur;
    RIL_Call *p_calls;
    RIL_Call **pp_calls;
    int countCalls = 0;
    int countValidCalls = 0;
    int ret = -1;
    bool tryAsClcc = false;

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

    if (countCalls == 0) {
        RmcCallControlUrcHandler::resetSpeechCodec();
    }

    /* init the pointer array */
    for (int i = 0; i < countCalls; i++) {
        pp_calls[i] = &(p_calls[i]);
    }

    for (p_cur = p_response->getIntermediates(); p_cur != NULL; p_cur = p_cur->getNext()) {
        char *line = p_cur->getLine();

        //if (RfxRilUtils::isUserLoad() != 1) {
        //    logD(RFX_LOG_TAG, "line:%s", line);
        //}
        p_calls[countValidCalls].name = (char *)alloca(MAX_CNAP_LENGTH);
        RFX_ASSERT(p_calls[countValidCalls].name != NULL);
        memset(p_calls[countValidCalls].name, 0, MAX_CNAP_LENGTH);
        if (tryAsClcc) {
            ret = callFromCLCCLine(p_cur, p_calls + countValidCalls);
        } else {
            ret = callFromCLCCSLine(p_cur, p_calls + countValidCalls);
        }

        if (ret != 0) {
            continue;
        }

        countValidCalls++;
    }

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
int RmcCallControlCommonRequestHandler::callFromCLCCSLine(RfxAtLine *pLine, RIL_Call *p_call) {
    //+CLCCS: <ccid1>,<dir>,<neg_status_present>,<neg_status>,<SDP_md>,<cs_mode>,<ccstatus>,<mpty>
    //              [,<numbertype>,<ton>,<number>[,<priority_present>,<priority>
    //              [,<CLI_validity_present>,<CLI_validity>]]]
    //+CLCCS: 1,0,,,,1,2,0,2,129,"0800000123",,,0,0

    int ret;
    int state;
    int mode;
    int dummy;
    char intdata[10];

    pLine->atTokStart(&ret);
    if (ret < 0) { goto error; }

    memset(atLog, 0, MAX_AT_RESPONSE);
    strncpy(atLog, "AT< +CLCC: ", 11);

    p_call->index = pLine->atTokNextint(&ret);  // <ccid>
    if (ret < 0) { goto error; }
    //logD(RFX_LOG_TAG, "ccid=%d", p_call->index);
    sprintf(intdata, "%d", p_call->index); // put the int into a string
    strncat(atLog, intdata, strlen(intdata));
    strncat(atLog, ", ", 2);

    p_call->isMT = pLine->atTokNextint(&ret);  // <dir>
    if (ret < 0) { goto error; }
    //logD(RFX_LOG_TAG, "dir=%d", p_call->isMT);
    sprintf(intdata, "%d", p_call->isMT); // put the int into a string
    strncat(atLog, intdata, strlen(intdata));
    strncat(atLog, ", ", 2);

    dummy = pLine->atTokNextint(&ret);  // <neg_status_present>
    if (ret < 0) {
        strncat(atLog, ", ", 2);
    } else {
        sprintf(intdata, "%d", dummy); // put the int into a string
        strncat(atLog, intdata, strlen(intdata));
        strncat(atLog, ", ", 2);
    }

    dummy = pLine->atTokNextint(&ret);  // <neg_status>
    if (ret < 0) {
        strncat(atLog, ", ", 2);
    } else {
        sprintf(intdata, "%d", dummy); // put the int into a string
        strncat(atLog, intdata, strlen(intdata));
        strncat(atLog, ", ", 2);
    }

    dummy = pLine->atTokNextint(&ret);  // <SDP_md>
    if (ret < 0) {
        strncat(atLog, ", ", 2);
    } else {
        sprintf(intdata, "%d", dummy); // put the int into a string
        strncat(atLog, intdata, strlen(intdata));
        strncat(atLog, ", ", 2);
    }

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
    sprintf(intdata, "%d", mode); // put the int into a string
    strncat(atLog, intdata, strlen(intdata));
    strncat(atLog, ", ", 2);

    p_call->isVoice = (mode == 1);

    state = pLine->atTokNextint(&ret);  // <ccstatus>
    if (ret < 0) { goto error; }
    //logD(RFX_LOG_TAG, "ccstatus=%d", state);
    sprintf(intdata, "%d", state); // put the int into a string
    strncat(atLog, intdata, strlen(intdata));
    strncat(atLog, ", ", 2);

    ret = clccsStateToRILState(state, &(p_call->state));
    if (ret < 0) { goto error; }

    if (mode >= 20) {
        logD(RFX_LOG_TAG, "Skip call with mode>=20, i.e. IMS/WFC call");
        return -1;
    } else if (hasImsCall(m_slot_id)) {
        logI(RFX_LOG_TAG, "Skip when hasImsCall true.");
        return -1;
    }

    // CNAP URC is sent after RING and before CLCC polling,
    // therefore we create a cached array to store the name of MT call, with the cached CNAP,
    // so that the name in subsequent CLCC polling can be filled with the cached CNAP array,
    // even the call becomes ACTIVE later.
    if (p_call->state == RIL_CALL_INCOMING || p_call->state == RIL_CALL_WAITING) {
        if ((p_call->index >= 1) && (p_call->index <= MAX_GSMCALL_CONNECTIONS)) {
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
    sprintf(intdata, "%d", p_call->isMpty); // put the int into a string
    strncat(atLog, intdata, strlen(intdata));
    strncat(atLog, ", ", 2);

    if (pLine->atTokHasmore()) {
        int numbertype = pLine->atTokNextint(&ret);  // <numbertype>
        if (ret < 0) { goto error; }
        //logD(RFX_LOG_TAG, "numbertype=%d", numbertype);
        sprintf(intdata, "%d", numbertype); // put the int into a string
        strncat(atLog, intdata, strlen(intdata));
        strncat(atLog, ", ", 2);

        // Adjust for different Modem format: TON(Gen97) or TOA(Gen93,95)
        int ton = pLine->atTokNextint(&ret);
        sprintf(intdata, "%d", ton); // put the int into a string
        strncat(atLog, intdata, strlen(intdata));
        strncat(atLog, ", ", 2);

        //TOA = 128 + TON*16 + NPI (NPI=1)
        if (ton <= 7) {
            p_call->toa = 128 + ton*16 + 1;
        } else {
            p_call->toa = ton;
        }

        if (ret < 0) { goto error; }
        //logD(RFX_LOG_TAG, "ton=%d", p_call->toa);

        p_call->number = pLine->atTokNextstr(&ret);
        const char *numberToPrint = RfxRilUtils::pii(RFX_LOG_TAG, p_call->number);
        strncat(atLog, numberToPrint, strlen(numberToPrint));
        strncat(atLog, ", ", 2);

        /* tolerate RfxStringData("") here */
        if (ret < 0) return 0;
        //logD(RFX_LOG_TAG, "number=%s", p_call->number);

        p_call->number = extractSipUri(p_call->number);
    }

    if (pLine->atTokHasmore()) {
        dummy = pLine->atTokNextint(&ret);  // <priority_present>
        if (ret < 0) {
            strncat(atLog, ", ", 2);
        } else {
            sprintf(intdata, "%d", dummy); // put the int into a string
            strncat(atLog, intdata, strlen(intdata));
            strncat(atLog, ", ", 2);
        }

        dummy = pLine->atTokNextint(&ret);  // <priority>
        if (ret < 0) {
            strncat(atLog, ", ", 2);
        } else {
            sprintf(intdata, "%d", dummy); // put the int into a string
            strncat(atLog, intdata, strlen(intdata));
            strncat(atLog, ", ", 2);
        }
    }

    if (pLine->atTokHasmore()) {
        int cliPresent = pLine->atTokNextint(&ret);  // <cli_validity_present>
        if (ret < 0) { goto error; }
        //logD(RFX_LOG_TAG, "cli_validity_present=%d", cliPresent);
        sprintf(intdata, "%d", cliPresent); // put the int into a string
        strncat(atLog, intdata, strlen(intdata));
        strncat(atLog, ", ", 2);

        /*
        0 CLI valid
        1 CLI has been withheld by the originator"Reject by user"
        2 CLI is not available due to interworking problems or limitations of originating network
        3 CLI is not available due to calling party being of type payphone
        4 CLI is not available due to other reasons
        */
        // [ALPS04692108]
        // When CLI_validity_present is 0, and VDM is disabled, no CLI_validity is returned by L4C
        if (cliPresent == 0) {
            p_call->numberPresentation = 0;
        } else {
            p_call->numberPresentation = pLine->atTokNextint(&ret);  // <cli_validity>
            if (ret < 0) { goto error; }
            sprintf(intdata, "%d", p_call->numberPresentation); // put the int into a string
            strncat(atLog, intdata, strlen(intdata));
            strncat(atLog, ", ", 2);
        }

        /* Issue: ALPS03500092
           Google AOSP only supports 0~3, so re-map 4 or later value to 2(UNKNOWN)
         */
        if (p_call->numberPresentation > 3) {
            p_call->numberPresentation = 2;
        }
    }

    p_call->uusInfo = NULL;
    p_call->speechCodec = RmcCallControlUrcHandler::getSpeechCodec();

    logD(RFX_LOG_TAG, "%s", atLog);

    return 0;

error:
    logE(RFX_LOG_TAG, "invalid CLCCS line");
    return -1;
}


int RmcCallControlCommonRequestHandler::callFromCLCCLine(RfxAtLine *pLine, RIL_Call *p_call) {
    // +CLCC: 1,0,2,0,0,\"+18005551212\",145
    // +CLCC: <id>,<dir>,<stat>,<mode>,<mpty>,<number>,<toa>,,,<CLI validity>
    // Optional parameters <alpha> and <priority> not present.

    int ret;
    int state;
    int mode;
    char intdata[10];

    pLine->atTokStart(&ret);
    if (ret < 0) { goto error; }

    memset(atLog, 0, MAX_AT_RESPONSE);
    strncpy(atLog, "AT< +CLCC: ", 11);

    p_call->index = pLine->atTokNextint(&ret);
    if (ret < 0) { goto error; }
    sprintf(intdata, "%d", p_call->index); // put the int into a string
    strncat(atLog, intdata, strlen(intdata));
    strncat(atLog, ", ", 2);

    p_call->isMT = pLine->atTokNextint(&ret);
    if (ret < 0) { goto error; }
    sprintf(intdata, "%d", p_call->isMT); // put the int into a string
    strncat(atLog, intdata, strlen(intdata));
    strncat(atLog, ", ", 2);

    state = pLine->atTokNextint(&ret);
    if (ret < 0) { goto error; }
    sprintf(intdata, "%d", state); // put the int into a string
    strncat(atLog, intdata, strlen(intdata));
    strncat(atLog, ", ", 2);

    ret = clccStateToRILState(state, &(p_call->state));
    if (ret < 0) { goto error; }

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
    if (ret < 0) { goto error; }
    sprintf(intdata, "%d", mode); // put the int into a string
    strncat(atLog, intdata, strlen(intdata));
    strncat(atLog, ", ", 2);

    p_call->isVoice = (mode == 0);

    if (mode >= 20) {
        logD(RFX_LOG_TAG, "Skip call with mode>=20, i.e. IMS/WFC call");
        return -1;
    } else if (hasImsCall(m_slot_id)) {
        logI(RFX_LOG_TAG, "Skip when hasImsCall true.");
        return -1;
    }

    p_call->isMpty = pLine->atTokNextint(&ret);
    if (ret < 0) { goto error; }
    sprintf(intdata, "%d", p_call->isMpty); // put the int into a string
    strncat(atLog, intdata, strlen(intdata));
    strncat(atLog, ", ", 2);

    if (pLine->atTokHasmore()) {
        p_call->number = pLine->atTokNextstr(&ret);
        const char *numberToPrint = RfxRilUtils::pii(RFX_LOG_TAG, p_call->number);
        strncat(atLog, numberToPrint, strlen(numberToPrint));
        strncat(atLog, ", ", 2);

        /* tolerate RfxStringData("") here */
        if (ret < 0) return 0;

        // Some lame implementations return strings
        // like "NOT AVAILABLE" in the CLCC line
        if ((p_call->number != NULL) &&
            (strspn(p_call->number, "+*#pw0123456789") != strlen(p_call->number))) {
            p_call->number = NULL;
        }

        p_call->toa = pLine->atTokNextint(&ret);
        if (ret < 0) { goto error; }
        sprintf(intdata, "%d", p_call->toa); // put the int into a string
        strncat(atLog, intdata, strlen(intdata));
        strncat(atLog, ", ", 2);
    }

    if (pLine->atTokHasmore()) {
        int dummy;
        dummy = pLine->atTokNextint(&ret);  // <alpha>
        if (ret < 0) {
            strncat(atLog, ", ", 2);
        } else {
            sprintf(intdata, "%d", dummy); // put the int into a string
            strncat(atLog, intdata, strlen(intdata));
            strncat(atLog, ", ", 2);
        }

        dummy = pLine->atTokNextint(&ret);  // <priority>
        if (ret < 0) {
            strncat(atLog, ", ", 2);
        } else {
            sprintf(intdata, "%d", dummy); // put the int into a string
            strncat(atLog, intdata, strlen(intdata));
            strncat(atLog, ", ", 2);
        }

        p_call->numberPresentation = pLine->atTokNextint(&ret);
        if (ret < 0) { goto error; }
        sprintf(intdata, "%d", p_call->numberPresentation); // put the int into a string
        strncat(atLog, intdata, strlen(intdata));
        strncat(atLog, ", ", 2);

        /* Issue: ALPS03500092
           Google AOSP only supports 0~3, so re-map 4 or later value to 2(UNKNOWN)
         */
        if (p_call->numberPresentation > 3) {
            p_call->numberPresentation = 2;
        }
    }

    p_call->uusInfo = NULL;
    p_call->speechCodec = RmcCallControlUrcHandler::getSpeechCodec();

    logD(RFX_LOG_TAG, "%s", atLog);

    return 0;

error:
    logE(RFX_LOG_TAG, "invalid CLCC line");
    return -1;
}

int RmcCallControlCommonRequestHandler::clccsStateToRILState(int state, RIL_CallState *p_state) {
    /*
    1   Idle
    2   Calling (MO); the call setup has been started
    3   Connecting (MO); the call is in progress
    4   Alerting (MO): an alert indication has been received
    5   Alerting (MT); an alert indication has been sent
    6   Active; the connection is established
    7   Released; an outgoing (MO) call is released.
    8   Released; an incoming (MT) call is released
    9   User Busy
    10  User Determined User Busy
    11  Call Waiting (MO)
    12  Call Waiting (MT)
    13  Call Hold (MO)
    14  Call Hold (MT)
    */
    switch (state) {
        case 2:
        case 3:
            *p_state = RIL_CALL_DIALING;
            return 0;
        case 4: *p_state = RIL_CALL_ALERTING; return 0;
        case 5: *p_state = RIL_CALL_INCOMING; return 0;
        case 6: *p_state = RIL_CALL_ACTIVE;   return 0;
        case 12:
            *p_state = RIL_CALL_WAITING;
            return 0;
        case 13:
        case 14:
            *p_state = RIL_CALL_HOLDING;
            return 0;
        default: return -1;
    }
}

int RmcCallControlCommonRequestHandler::clccStateToRILState(int state, RIL_CallState *p_state) {
    switch (state) {
        case 0: *p_state = RIL_CALL_ACTIVE;   return 0;
        case 1: *p_state = RIL_CALL_HOLDING;  return 0;
        case 2: *p_state = RIL_CALL_DIALING;  return 0;
        case 3: *p_state = RIL_CALL_ALERTING; return 0;
        case 4: *p_state = RIL_CALL_INCOMING; return 0;
        case 5: *p_state = RIL_CALL_WAITING;  return 0;
        default: return -1;
    }
}

void RmcCallControlCommonRequestHandler::requestDial(const sp<RfxMclMessage>& msg,
        bool isEcc, bool isVt, bool isImsDial) {
    RIL_Dial *pDial = (RIL_Dial*)(msg->getData()->getData());
    String8 cmd;
    int clirMode = pDial->clir;
    int request = msg->getId();

    if (request == RFX_MSG_REQUEST_IMS_DIAL ||
            request == RFX_MSG_REQUEST_IMS_VT_DIAL ||
            request == RFX_MSG_REQUEST_LOCAL_IMS_EMERGENCY_DIAL) {
        clirMode = handleClirSpecial(isEcc, pDial->clir, pDial->address);
    }
    const char *clir = getClirPrefix(clirMode);
    String8 dialStringWithPause = handleNumberWithPause(pDial->address);
    String8 dialString;

    if (isImsDial) {
        dialString = dialStringWithPause;
    } else {
        /* Convert *67 and *82 to #31# and *31# only for CS call */
        dialString = handleNumberWithClirString(dialStringWithPause);
    }

    char* atCmd = AT_DIAL;

    if (isEcc) {
        //TODO: If an incoming call exists(+CRING is not received yet), hang it up before dial ECC
        atCmd = AT_EMERGENCY_DIAL;
    } else if (!isValidDialString(dialString.string())) {
        bUseLocalCallFailCause = 1;
        dialLastError = CALL_FAIL_INVALID_NUMBER_FORMAT;
        responseVoidDataToTcl(msg, RIL_E_CANCELLED);
        return;
    }

    cmd = String8::format("%s%s%s", atCmd, clir, dialString.string());
    logD(RFX_LOG_TAG, "AT> %s%s%s", atCmd, clir,
            RfxRilUtils::pii(RFX_LOG_TAG, dialString.string()));

    if (isVt) {
        cmd = String8::format("%s", cmd.string());
    } else {
        cmd = String8::format("%s;", cmd.string());
    }

    setVDSAuto(hasImsCall(msg->getSlotId()), isEcc);
    handleCmdWithVoidResponse(msg, cmd);

    bUseLocalCallFailCause = 0;
    dialLastError = 0;
}

void RmcCallControlCommonRequestHandler::requestAnswer(const sp<RfxMclMessage>& msg) {
    handleCmdWithVoidResponse(msg, String8(AT_ANSWER));
}

void RmcCallControlCommonRequestHandler::requestUdub(const sp<RfxMclMessage>& msg) {
    atSendCommand(String8::format("ATH"));

    if (isCdmaRat()) {
        atSendCommand(String8::format("AT+CRM=1"));
    }

    int callCount = getMclStatusManager()->getIntValue(RFX_STATUS_KEY_VOICE_CALL_COUNT, 0);
    int card_types = getMclStatusManager()->getIntValue(RFX_STATUS_KEY_CARD_TYPE, -1);
    if (callCount == 0 && card_types <= 0) {
        responseVoidDataToTcl(msg, RIL_E_INVALID_STATE);
        return;
    }

    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxVoidData(), msg);
    responseToTelCore(response);
}

void RmcCallControlCommonRequestHandler::requestDtmf(const sp<RfxMclMessage>& msg) {
    char dtmf = ((char *)msg->getData()->getData())[0];
    sp<RfxAtResponse> p_response;

    p_response = atSendCommand(String8::format("AT+VTS=%c", dtmf));
    if (p_response->getError() != 0 || p_response->getSuccess() != 1) {
        responseVoidDataToTcl(msg, RIL_E_INTERNAL_ERR);
        return;
    }

    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxVoidData(), msg);
    responseToTelCore(response);
}

void RmcCallControlCommonRequestHandler::requestDtmfStart(const sp<RfxMclMessage>& msg) {
    char dtmf = ((char *)msg->getData()->getData())[0];
    sp<RfxAtResponse> p_response;

    p_response = atSendCommand(String8::format("AT+EVTS=0,\"%c\"", dtmf));
    logD(RFX_LOG_TAG, "AT> AT+EVTS=0, *");

    if (p_response->getError() != 0 || p_response->getSuccess() != 1) {
        responseVoidDataToTcl(msg, RIL_E_INTERNAL_ERR);
        return;
    }

    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxVoidData(), msg);
    responseToTelCore(response);
}

void RmcCallControlCommonRequestHandler::requestDtmfStop(const sp<RfxMclMessage>& msg) {
    sp<RfxAtResponse> p_response;

    p_response = atSendCommand(String8::format("AT+EVTS=1"));
    logD(RFX_LOG_TAG, "AT> AT+EVTS=1");

    if (p_response->getError() != 0 || p_response->getSuccess() != 1) {
        responseVoidDataToTcl(msg, RIL_E_INTERNAL_ERR);
        return;
    }

    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxVoidData(), msg);
    responseToTelCore(response);
}

void RmcCallControlCommonRequestHandler::requestSetMute(const sp<RfxMclMessage>& msg) {
    int mute = ((int *)msg->getData()->getData())[0];
    sp<RfxAtResponse> p_response;

    p_response = atSendCommand(String8::format("AT+CMUT=%d", mute));
    if (p_response->getError() != 0 || p_response->getSuccess() != 1) {
        responseVoidDataToTcl(msg, RIL_E_INTERNAL_ERR);
        return;
    }

    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxVoidData(), msg);
    responseToTelCore(response);
}

void RmcCallControlCommonRequestHandler::requestGetMute(const sp<RfxMclMessage>& msg) {
    sp<RfxAtResponse> p_response;
    int ret;
    int mute = 0;
    RfxAtLine *pLine = NULL;

    p_response = atSendCommandMultiline(String8::format("AT+CMUT?"), "+CMUT:");
    if (p_response->getError() != 0 || p_response->getSuccess() != 1) {
        responseVoidDataToTcl(msg, RIL_E_INTERNAL_ERR);
        return;
    }

    pLine = p_response->getIntermediates();
    if (pLine == NULL) {
        responseVoidDataToTcl(msg, RIL_E_INTERNAL_ERR);
        return;
    }

    pLine->atTokStart(&ret);
    if (ret < 0) {
        responseVoidDataToTcl(msg, RIL_E_INTERNAL_ERR);
        return;
    }

    mute = pLine->atTokNextint(&ret);
    if (ret < 0) {
        responseVoidDataToTcl(msg, RIL_E_INTERNAL_ERR);
        return;
    }

    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxIntsData(&mute, 1), msg, false);
    responseToTelCore(response);
}

void RmcCallControlCommonRequestHandler::requestSetTtyMode(const sp<RfxMclMessage>& msg) {
    int mode = ((int *)msg->getData()->getData())[0];
    sp<RfxAtResponse> p_response;

    p_response = atSendCommand(String8::format("AT+CTMCALL=%d", mode));
    if (p_response->getError() != 0 || p_response->getSuccess() != 1) {
        responseVoidDataToTcl(msg, RIL_E_GENERIC_FAILURE);
        return;
    }

    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxVoidData(), msg);
    responseToTelCore(response);
}

void RmcCallControlCommonRequestHandler::requestQueryTtyMode(const sp<RfxMclMessage>& msg) {
    sp<RfxAtResponse> p_response;
    int ret;
    int mode = 0;
    RfxAtLine *pLine = NULL;

    p_response = atSendCommandMultiline(String8::format("AT+CTMCALL?"), "+CTMCALL:");
    if (p_response->getError() != 0 || p_response->getSuccess() != 1) {
        // VTS expects no error even SIM absent, so return RIL_E_SUCCESS always.
        sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
                RfxIntsData(&mode, 1), msg, false);
        responseToTelCore(response);
        return;
    }

    pLine = p_response->getIntermediates();
    if (pLine == NULL) {
        responseVoidDataToTcl(msg, RIL_E_GENERIC_FAILURE);
        return;
    }

    pLine->atTokStart(&ret);
    if (ret < 0) {
        responseVoidDataToTcl(msg, RIL_E_GENERIC_FAILURE);
        return;
    }

    mode = pLine->atTokNextint(&ret);
    if (ret < 0) {
        responseVoidDataToTcl(msg, RIL_E_GENERIC_FAILURE);
        return;
    }

    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxIntsData(&mode, 1), msg, false);
    responseToTelCore(response);
}

void RmcCallControlCommonRequestHandler::requestLastCallFailCause(const sp<RfxMclMessage>& msg) {
    RIL_LastCallFailCauseInfo callFailCause;
    RfxAtLine *p_cur;
    int ret;
    sp<RfxAtResponse> p_response;

    memset(&callFailCause, 0, sizeof(RIL_LastCallFailCauseInfo));

    if (bUseLocalCallFailCause == 1) {
       callFailCause.cause_code = (RIL_LastCallFailCause)dialLastError;
       bUseLocalCallFailCause = 0;
       dialLastError = 0;
       logD(RFX_LOG_TAG, "Use local call fail cause = %d", callFailCause.cause_code);
    } else {
        p_response = atSendCommandSingleline(String8::format("AT+CEER"), "+CEER:");
        // set result
        if (p_response->getError() != 0 || p_response->getSuccess() != 1) {
            responseVoidDataToTcl(msg, RIL_E_INTERNAL_ERR);
            return;
        } else {
            p_cur = p_response->getIntermediates();
            if (p_cur == NULL) {
                responseVoidDataToTcl(msg, RIL_E_INTERNAL_ERR);
                return;
            }
            char *line = p_cur->getLine();
            p_cur->atTokStart(&ret);
            if (ret < 0) {
                responseVoidDataToTcl(msg, RIL_E_INTERNAL_ERR);
                return;
            }

            callFailCause.cause_code = (RIL_LastCallFailCause)p_cur->atTokNextint(&ret);
            if (ret < 0) {
                responseVoidDataToTcl(msg, RIL_E_INTERNAL_ERR);
                return;
            }

            callFailCause.vendor_cause = p_cur->atTokNextstr(&ret);
            if (ret < 0) {
                responseVoidDataToTcl(msg, RIL_E_INTERNAL_ERR);
                return;
            }
            logD(RFX_LOG_TAG, "MD fail cause_code = %d, vendor_cause = %s",
                    callFailCause.cause_code, callFailCause.vendor_cause);
        }
    }

    /*if there are more causes need to be translated in the future,
     * discussing with APP owner to implement this in upper layer.
     * For the hard coded value, please refer to modem code.*/
    if (isCdmaRat()) {
        if (callFailCause.cause_code == 1) {  // Phone is locked
            callFailCause.cause_code = CALL_FAIL_CDMA_LOCKED_UNTIL_POWER_CYCLE;
        } else if (callFailCause.cause_code == 22) {  // Call faded/dropped
            callFailCause.cause_code = CALL_FAIL_CDMA_DROP;
        } else if (callFailCause.cause_code == 23) {  // Received Intercept from base station
            callFailCause.cause_code = CALL_FAIL_CDMA_INTERCEPT;
        } else if (callFailCause.cause_code == 24) {  // Received Reorder from base station
            callFailCause.cause_code = CALL_FAIL_CDMA_REORDER;
        } else if (callFailCause.cause_code == 26) {  // Service option rejected by base station
            callFailCause.cause_code = CALL_FAIL_CDMA_SO_REJECT;
        } else if (callFailCause.cause_code == 241) {
            callFailCause.cause_code = CALL_FAIL_FDN_BLOCKED;
        } else if (callFailCause.cause_code == 0 ||  // Phone is offline
                callFailCause.cause_code == 12 ||   // Internal SW aborted the origination
                callFailCause.cause_code == 21 ||   // Phone is out of service
                callFailCause.cause_code == 27 ||   // There is incoming call
                callFailCause.cause_code == 28 ||   // Received an alert stop from base station
                callFailCause.cause_code == 30 ||   // Received end activation -OTASP calls only
                callFailCause.cause_code == 32 ||   // RUIM is not available
                callFailCause.cause_code == 99 ||   // NW directed system selection error
                callFailCause.cause_code == 100 ||  // Released by lower layer
                callFailCause.cause_code == 101 ||  // After a MS initiates a call, NW fails to respond
                callFailCause.cause_code == 102 ||  // Phone rejects an incoming call
                callFailCause.cause_code == 103 ||  // A call is rejected during the put-through process
                callFailCause.cause_code == 104 ||  // The release is from NW
                callFailCause.cause_code == 105 ||  // The phone fee is used up
                callFailCause.cause_code == 106) {  // The MS is out of the service area
            callFailCause.cause_code = CALL_FAIL_ERROR_UNSPECIFIED;
        } else {
            callFailCause.cause_code = CALL_FAIL_NORMAL;
        }
    } else {
        if (callFailCause.cause_code == 10)
            callFailCause.cause_code = CALL_FAIL_CALL_BARRED;
        else if (callFailCause.cause_code == 2600)
            callFailCause.cause_code = CALL_FAIL_FDN_BLOCKED;
        else if (callFailCause.cause_code == 2052)
            callFailCause.cause_code = CALL_FAIL_IMSI_UNKNOWN_IN_VLR;
        else if (callFailCause.cause_code == 2053)
            callFailCause.cause_code = CALL_FAIL_IMEI_NOT_ACCEPTED;
        else if (callFailCause.cause_code == 2004)
            callFailCause.cause_code = CALL_FAIL_ACCESS_CLASS_BLOCKED;
        else if ((callFailCause.cause_code < 0) ||
               (callFailCause.cause_code > 127
               && callFailCause.cause_code != 2165
               && callFailCause.cause_code != 380
               && callFailCause.cause_code != 999))
            callFailCause.cause_code = CALL_FAIL_ERROR_UNSPECIFIED;
    }
    logD(RFX_LOG_TAG, "RIL fail cause_code = %d, vendor_cause = %s",
            callFailCause.cause_code, callFailCause.vendor_cause);

    /// M: CC: ECC disconnection special handling @{
    // Report DisconnectCause.NORMAL for IMEI_NOT_ACCEPTED
    // For GCF test, ECC might be rejected and dont trigger ECC retry in this case.
    if (getMclStatusManager()->getBoolValue(RFX_STATUS_KEY_ECC_DISCONNECTED, false)) {
        if (callFailCause.cause_code == CALL_FAIL_IMEI_NOT_ACCEPTED) {
            logD(RFX_LOG_TAG, "Convert fail cause_code to NORMAL");
            callFailCause.cause_code = CALL_FAIL_NORMAL;
        }
    }
    getMclStatusManager()->setBoolValue(RFX_STATUS_KEY_ECC_DISCONNECTED, false);
    /// @}

    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxCallFailCauseData(&callFailCause, sizeof(RIL_LastCallFailCauseInfo)), msg);
    responseToTelCore(response);
}

void RmcCallControlCommonRequestHandler::requestSetCallIndication(const sp<RfxMclMessage>& msg) {
    int *pInt = (int *)msg->getData()->getData();
    sp<RfxAtResponse> p_response;

    String8 cmd;
    int causeToModem = -1;
    if (pInt[0] == 1) {
        if (pInt[3] == CALL_INDICATION_CAUSE_INCOMING_REJECT) {
            causeToModem = 21;
        }
        logD(RFX_LOG_TAG, "RIL call indication cause %d, causeToModem=%d",
                pInt[3], causeToModem);
    }
    if (pInt[0] == 1 && causeToModem != -1) {
        cmd = String8::format("%s=%d,%d,%d,%d", AT_ALLOW_MT, pInt[0], pInt[1], pInt[2],
                causeToModem);
    } else {
        cmd = String8::format("%s=%d,%d,%d", AT_ALLOW_MT, pInt[0], pInt[1], pInt[2]);
    }
    handleCmdWithVoidResponse(msg, cmd);
}

void RmcCallControlCommonRequestHandler::requestLocalSetEccServiceCategory(
        const sp<RfxMclMessage>& msg) {
    int serviceCategory = ((int *)msg->getData()->getData())[0];
    sp<RfxAtResponse> p_response;

    p_response = atSendCommand(String8::format("AT+ESVC=%d", serviceCategory));
    if (p_response->getError() != 0 || p_response->getSuccess() != 1) {
        responseVoidDataToTcl(msg, RIL_E_GENERIC_FAILURE);
        return;
    }

    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxVoidData(), msg);
    responseToTelCore(response);
}

/// C2K specific start
void RmcCallControlCommonRequestHandler::requestSetPreferredVoicePrivacyMode(const sp<RfxMclMessage>& msg) {
    RIL_Errno err;
    int *pInt = (int *)msg->getData()->getData();

    if (RfxViaUtils::getViaHandler() != NULL) {
        RfxViaUtils::getViaHandler()->requestSetPreferredVoicePrivacyMode(this, pInt[0], &err);
    } else {
        err = RIL_E_REQUEST_NOT_SUPPORTED;
        logE(RFX_LOG_TAG, "handleCdmaSubscription RIL_E_REQUEST_NOT_SUPPORTED!/n");
    }

    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), err,
            RfxVoidData(), msg);
    responseToTelCore(response);
}

void RmcCallControlCommonRequestHandler::requestQueryPreferredVoicePrivacyMode(const sp<RfxMclMessage>& msg) {
    RIL_Errno err;
    int result = 0;

    if (RfxViaUtils::getViaHandler() != NULL) {
        RfxViaUtils::getViaHandler()->requestQueryPreferredVoicePrivacyMode(this, &result, &err);
    } else {
        err = RIL_E_REQUEST_NOT_SUPPORTED;
        logE(RFX_LOG_TAG, "handleCdmaSubscription RIL_E_REQUEST_NOT_SUPPORTED!/n");
    }

    if (err != RIL_E_SUCCESS) {
        responseVoidDataToTcl(msg, err);
        return;
    }

    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxIntsData((void *)&result, sizeof(int)), msg);
    responseToTelCore(response);
}

void RmcCallControlCommonRequestHandler::requestFlash(const sp<RfxMclMessage>& msg) {
    char *number = (char *)msg->getData()->getData();
    char origString[16] = {0};
    sp<RfxAtResponse> p_response;

    // for HIDL string copy, "" string will convert to null and passed
    // to RILD, so we should take NULL as empty after O.
    if (number == NULL || strlen(number) == 0) {
        p_response = atSendCommand(String8::format("AT+CFSH="));
        logD(RFX_LOG_TAG, "AT> AT+CFSH=");
    } else {
        String8 origNumber = String8();
        if (isNumberIncludePause(number)) {
            origNumber = handleNumberWithPause(number);
        } else {
            origNumber = String8(number);
        }
        if (strlen(origNumber.string()) < 16) {
            p_response = atSendCommand(String8::format("AT+CFSH=%s", origNumber.string()));
            logD(RFX_LOG_TAG, "AT> AT+CFSH=%s",
                    RfxRilUtils::pii(RFX_LOG_TAG, origNumber.string()));
        } else {
            strncpy(origString, origNumber.string(), 15);
            p_response = atSendCommand(String8::format("AT+CFSH=%s", origString));
            logD(RFX_LOG_TAG, "AT> AT+CFSH=%s", RfxRilUtils::pii(RFX_LOG_TAG, origString));
        }
    }
    if (p_response->getError() != 0 || p_response->getSuccess() != 1) {
        responseVoidDataToTcl(msg, RIL_E_INVALID_ARGUMENTS);
        return;
    }
    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxVoidData(), msg);
    responseToTelCore(response);
    return;
}

void RmcCallControlCommonRequestHandler::requestBurstDtmf(const sp<RfxMclMessage>& msg) {
    char *burstDtmf = ((char **)msg->getData()->getData())[0];
    int len = strlen(burstDtmf);
    sp<RfxAtResponse> p_response;

    for (int i = 0; i < len; i++) {
        p_response = atSendCommand(String8::format("AT+VTS=%c", *(burstDtmf + i)));
        if (p_response->getError() != 0 || p_response->getSuccess() != 1) {
            responseVoidDataToTcl(msg, RIL_E_INTERNAL_ERR);
            return;
        }
    }

    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxVoidData(), msg);
    responseToTelCore(response);
}

void RmcCallControlCommonRequestHandler::requestExitEmergencyCallbackMode(const sp<RfxMclMessage>& msg) {
    // When MD is off, respond success directly.
    if (getNonSlotMclStatusManager()->getBoolValue(RFX_STATUS_KEY_MODEM_POWER_OFF, false)) {
        logE(RFX_LOG_TAG, "%s MD off, just return success!", __FUNCTION__);
        responseVoidDataToTcl(msg, RIL_E_SUCCESS);
        return;
    }
    handleCmdWithVoidResponse(msg, String8(AT_EXIT_ECBM));
}
/// C2K specific end

bool RmcCallControlCommonRequestHandler::isNumberIncludePause(char* number) {
    char* tmp = strchr(number, ',');
    if (tmp == NULL) {
        return false;
    }
    return true;
}

bool RmcCallControlCommonRequestHandler::isValidDialString(const char* dialString) {
    if (dialString == NULL || strlen(dialString) > 40) return false;
    return true;
}

void RmcCallControlCommonRequestHandler::handleCnapUpdate(const sp<RfxMclMessage>& msg) {
    char *cnap = (char*)msg->getData()->getData();
    if (cnap != NULL && strlen(cnap) > 0) {
        snprintf(cachedCnap, MAX_CNAP_LENGTH, "%s", cnap);
    }
}

void RmcCallControlCommonRequestHandler::handleClearClccName(const sp<RfxMclMessage>& msg) {
    int *callId = (int *)(msg->getData()->getData());
    //logD(RFX_LOG_TAG, "callId = %d", *callId);
    memset(cachedClccName[*callId-1], 0, MAX_CNAP_LENGTH);
}

void RmcCallControlCommonRequestHandler::clearCnap() {
    int i;
    memset(cachedCnap, 0, MAX_CNAP_LENGTH);
    for (i = 0; i < MAX_GSMCALL_CONNECTIONS; i++) {
        memset(cachedClccName[i], 0, MAX_CNAP_LENGTH);
    }
}

/// redial @{
void RmcCallControlCommonRequestHandler::requestEmergencyRedial(const sp<RfxMclMessage>& msg) {
    RFX_Redial *pRedial = (RFX_Redial *)(msg->getData()->getData());
    // FIXME: Due to change 1544458, for IMS ECC will ingore CLIR information.
    // ECC redial will ingore CLIR information both IMS ECC and CS ECC.
    // const char *clir = getClirPrefix(pRedial->dial_data->clir);
    const char *clir = "";
    String8 dialString = handleNumberWithPause(pRedial->dial_data->address);

    String8 cmd = String8::format("%s=\"%s%s\",%d", "ATDER",
                                        clir, dialString.string(), pRedial->call_id);
    logD(RFX_LOG_TAG, "AT> ATDER=\"%s%s\",%d",
            clir, RfxRilUtils::pii(RFX_LOG_TAG, dialString.string()), pRedial->call_id);

    handleCmdWithVoidResponse(msg, cmd);

    bUseLocalCallFailCause = 0;
    dialLastError = 0;
}

void RmcCallControlCommonRequestHandler::requestNotifyEmergencySession(
                                                    const sp<RfxMclMessage>& msg, bool isStarted) {
    String8 cmd = String8::format("AT+EMCS=%s", (isStarted ? "1" : "0"));
    handleCmdWithVoidResponse(msg, cmd);
}

/// @}


void RmcCallControlCommonRequestHandler::requestEccPreferredRat(const sp<RfxMclMessage>& msg) {
    int rat = ((int*)msg->getData()->getData())[0];
    getMclStatusManager()->setIntValue(RFX_STATUS_KEY_ECC_PREFERRED_RAT, rat);
    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxVoidData(), msg);
    responseToTelCore(response);
}

void RmcCallControlCommonRequestHandler::copyString(char* destStr, char* srcStr, int bufSize) {
    int len = strlen(srcStr);
    if (len > 0) {
        memset(destStr, 0, bufSize);
        if (len >= bufSize) {
            len = bufSize - 1;
        }
        strncpy(destStr, srcStr, len);
    }
}

void RmcCallControlCommonRequestHandler::requestLocalCurrentStatus(const sp<RfxMclMessage>& msg) {
    int *pInt = (int*)msg->getData()->getData();
    int airplaneMode = pInt[0];
    //int imsReg = pInt[1];
    /*
    AT+EMCS:<emc_session>,<airplane_mode>
    <emc_session> :  1 : emergency session begin
    <airplaine_mode> : 0 : off, 1 : on
    */

    atSendCommand(String8::format("AT+EMCS=1,%d", airplaneMode));

    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxVoidData(), msg);
    responseToTelCore(response);
}

void RmcCallControlCommonRequestHandler::queryEmciSupport() {
    char feature[] = "EMCI";
    int support = getFeatureVersion(feature);
    //no reset support, 0:disable, 1:enable
    logD(RFX_LOG_TAG, "queryEmciSupport, %s=%d",feature, support);
    if (support == 1) {
        rfx_property_set("vendor.ril.call.emci_support",
                String8::format("%d", support).string());
    } else {
        rfx_property_set("vendor.ril.call.emci_support",
                String8::format("%d", 0).string());
    }
}
