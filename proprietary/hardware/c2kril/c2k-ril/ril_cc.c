/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

/* //hardware/viatelecom/ril/viatelecom-withuim-ril/viatelecom-withuim-ril.c
**
** Copyright 2009, Viatelecom Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#include <telephony/mtk_ril.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <alloca.h>
#include <getopt.h>
#include <sys/socket.h>
#include <cutils/sockets.h>
#include <termios.h>
#include "atchannel.h"
#include "at_tok.h"
#include "misc.h"
#include "ril_callbacks.h"
#include <utils/Log.h>

#include <cutils/properties.h>
#include <netdb.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <linux/route.h>

#undef LOG_TAG
#define LOG_TAG "C2K_RIL-CC"

#ifndef UNUSED
#define UNUSED(x) (x)   // Eliminate "warning: unused parameter"
#endif

#define CC_CHANNEL_CTX getRILChannelCtxFromToken(t)

#define CALL_ID_ASSIGN_MSG              130
#define CALL_CONNECTED_MSG              6
#define CALL_DISCONNECTED_MSG           133

#define DISPLAY_TAG_BLANK               0x80
#define DISPLAY_TAG_SKIP                0x81

#define DISPLAY_TAG_CALLING_PARTY_NAME  0x8D
#define DISPLAY_TAG_ORIG_CALLED_NAME    0x8F

#define MAX_EF_ECC_LEN   255

extern int inemergency;
extern RIL_UNSOL_Msg_Cache s_unsol_msg_cache;
extern int invoicecall;
extern int voicetype;
extern int s_md3_off;

typedef enum {
   IDLE,
   CONNECTED,
   DIALING,
   INCOMING,
   WAITING,
} CallState;

CallState callState = IDLE;
char cachedCnap[CDMA_ALPHA_INFO_BUFFER_LENGTH] = { 0 };
char cachedClccName[MAX_GSMCALL_CONNECTIONS][CDMA_ALPHA_INFO_BUFFER_LENGTH] = {{0}};

extern int isUserLoad();

static void requestGetCurrentCalls(void *data, size_t datalen, RIL_Token t);
static void sendCallStateChanged(void *param);
static void requestDial(void *data, size_t datalen, RIL_Token t);
static void requestHangup(void *data, size_t datalen, RIL_Token t);
static void requestGetLastCallFailCause(void *data, size_t datalen, RIL_Token t);
static void requestSetMute(void *data, size_t datalen, RIL_Token t);
static void requestQueryPreferredVoicePrivacy(void *data, size_t datalen, RIL_Token t);
static void requestSendFlash(void *data, size_t datalen, RIL_Token t);
static void requestExitEmergencyCallbackMode(void *data, size_t datalen, RIL_Token t);
static void requestLocalEmergencyDial(void *data, size_t datalen, RIL_Token t);
static void requestSwitchHPF(void *data, size_t datalen, RIL_Token t);
static int callFromCLCCLine(char *line, RIL_Call *p_call, int validIndex);
static int clccStateToRILState(int state, RIL_CallState *p_state);
static void onSpeechCodecInfo(char *urc);
static void onCallingPartyNumberInfo(char *urc);
static void onRedirectingNumberInfo(char *urc);
static void onLineControlInfo(char *urc);
static void onExtendedDisplayInfo(char *urc);
static void onDisplayAndSignalsInfo(char *urc);
static void onCallControlStatus(const char *s);

static void requestRedial(void *data, size_t datalen, RIL_Token t, int isEmergency);
static void notifyCallInformationForEccRedial(const char *s, int msgType, int cause);

/* current playing continuous dtmf character */
static char s_vtc[4] = { 0 };
/*times of loopback call is queried*/
static int loopbackcall_query_times = 0;
static const struct timeval TIMEVAL_CALLSTATEPOLL = {0, 500000};

// Support EF ECC @{
static void onHandleEfEccUrc(const char *s);
// @}

extern int rilCcMain(int request, void *data, size_t datalen, RIL_Token t) {
    switch (request) {
        case RIL_REQUEST_GET_CURRENT_CALLS:
            requestGetCurrentCalls(data, datalen, t);
            break;
        case RIL_REQUEST_DIAL:
            requestDial(data, datalen, t);
            break;
        case RIL_REQUEST_HANGUP:
        case RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND:
        case RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND:
        case RIL_REQUEST_HANGUP_ALL:
            requestHangup(data, datalen, t);
            break;
        case RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE:
            // 3GPP 22.030 6.5.5
            // "Places all active calls (if any exist) on hold and accepts
            //  the other (held or waiting) call."
            at_send_command("AT+CHLD=2", NULL, CC_CHANNEL_CTX);
            /* success or failure is ignored by the upper layer here.
             it will call GET_CURRENT_CALLS and determine success that way */
            RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
            break;
        case RIL_REQUEST_UDUB:
            /* user determined user busy */
            /* sometimes used: ATH */
            at_send_command("ATH", NULL, CC_CHANNEL_CTX);

            /* set cp rm interface protocol - Relay Layer Rm interface, PPP */
            at_send_command("AT+CRM=1", NULL, CC_CHANNEL_CTX);

            /* success or failure is ignored by the upper layer here.
             it will call GET_CURRENT_CALLS and determine success that way */
            RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
            break;
        case RIL_REQUEST_LAST_CALL_FAIL_CAUSE:
            requestGetLastCallFailCause(data, datalen, t);
            break;
        case RIL_REQUEST_DTMF: {
            char c = ((char*) data)[0];
            char *cmd = NULL;
            asprintf(&cmd, "AT+VTS=%c", (int) c);
            if (cmd != NULL) {
                at_send_command(cmd, NULL, CC_CHANNEL_CTX);
                free(cmd);
            }
            RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
            break;
        }
        case RIL_REQUEST_ANSWER:
            at_send_command("ATA", NULL, CC_CHANNEL_CTX);

            /* success or failure is ignored by the upper layer here.
             it will call GET_CURRENT_CALLS and determine success that way */
            RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
            break;
        case RIL_REQUEST_DTMF_START: {
            char c[4] = { 0 };
            c[0] = ((char*) data)[0];
            char *cmd = NULL;
            asprintf(&cmd, "AT+VTC=1,%s", c);
            if (cmd != NULL) {
                at_send_command(cmd, NULL, CC_CHANNEL_CTX);
                free(cmd);
            }
            RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
            strncpy(s_vtc, c, 1); /* save the char for RIL_REQUEST_DTMF_STOP */
            break;
        }
        case RIL_REQUEST_DTMF_STOP:
            if (s_vtc[0] == 0) {
                RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
            } else {
                char *cmd = NULL;
                asprintf(&cmd, "AT+VTC=0,%s", s_vtc);
                if (cmd != NULL) {
                    at_send_command(cmd, NULL, CC_CHANNEL_CTX);
                    free(cmd);
                }
                RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
                memset(s_vtc, 0, sizeof(s_vtc));
            }
            break;
        case RIL_REQUEST_SET_MUTE:
            requestSetMute(data, datalen, t);
            break;
        case RIL_REQUEST_CDMA_SET_PREFERRED_VOICE_PRIVACY_MODE: {
            char *cmd = NULL;
            ATResponse *p_response = NULL;
            int err = 0;
            asprintf(&cmd, "AT+VP=%d", ((int*) data)[0]);
            if (cmd != NULL) {
                err = at_send_command(cmd, &p_response, CC_CHANNEL_CTX);
                free(cmd);
            }
            if (err < 0 || p_response == NULL || p_response->success == 0) {
                RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
            } else {
                RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
            }
            at_response_free(p_response);
            break;
        }
        case RIL_REQUEST_CDMA_QUERY_PREFERRED_VOICE_PRIVACY_MODE:
            requestQueryPreferredVoicePrivacy(data, datalen, t);
            break;
        case RIL_REQUEST_CDMA_FLASH:
            requestSendFlash(data, datalen, t);
            break;
        case RIL_REQUEST_CDMA_BURST_DTMF: {
            char *c = ((char**) data)[0];
            char *cmd = NULL;
            int i = 0;
            int len = strlen(c);
            for (i = 0; i < len; i++) {
                asprintf(&cmd, "AT+VTS=%c", *(c + i));
                if (cmd != NULL) {
                    at_send_command(cmd, NULL, CC_CHANNEL_CTX);
                    free(cmd);
                    cmd = NULL;
                }
            }
            RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
            break;
        }
        case RIL_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE:
            requestExitEmergencyCallbackMode(data, datalen, t);
            break;
        case RIL_LOCAL_C2K_REQUEST_EMERGENCY_DIAL:
            requestLocalEmergencyDial(data, datalen, t);
            break;
        case RIL_LOCAL_C2K_REQUEST_SWITCH_HPF:
            requestSwitchHPF(data, datalen, t);
            break;
        case RIL_REQUEST_CONFERENCE:
            // 3GPP 22.030 6.5.5
            // "Adds a held call to the conversation"
            at_send_command("AT+CHLD=3", NULL, CC_CHANNEL_CTX);

            /* success or failure is ignored by the upper layer here.
             it will call GET_CURRENT_CALLS and determine success that way */
            RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
            break;
        case RIL_REQUEST_SEPARATE_CONNECTION: {
            char cmd[12] = { 0 };
            int party = ((int*) data)[0];

            // Make sure that party is in a valid range.
            // (Note: The Telephony middle layer imposes a range of 1 to 7.
            // It's sufficient for us to just make sure it's single digit.)
            if (party > 0 && party < 10) {
                sprintf(cmd, "AT+CHLD=2%d", party);
                at_send_command(cmd, NULL, CC_CHANNEL_CTX);
                RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
            } else {
                RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
            }
            break;
        }

        /// C2K redial requirement. @{
        /* C2K normal call redial */
        case RIL_LOCAL_C2K_REQUEST_REDIAL:
           requestRedial(data, datalen, t, 0);
           break;

        /* C2K emergency call redial */
        case RIL_LOCAL_C2K_REQUEST_EMERGENCY_REDIAL:
           requestRedial(data, datalen, t, 1);
           break;
        /// @}

        default:
            return 0; /* no matched request */
    }
    return 1;
}

extern int rilCcUnsolicited(const char *s, const char *sms_pdu) {
    char *line = NULL;
    int err;

    UNUSED(sms_pdu);

    if (strStartsWith(s, "+CCWA:")) {
        callState = WAITING;
        char *number = NULL;
        int callType = 0;
        RIL_CDMA_CallWaiting_v6 *cdmawait = NULL;
        char *dup = strdup(s);
        cdmawait = (RIL_CDMA_CallWaiting_v6*) alloca(
                sizeof(RIL_CDMA_CallWaiting_v6));
        assert(cdmawait != NULL);
        memset(cdmawait, 0, sizeof(RIL_CDMA_CallWaiting_v6));
        if (!dup) {
            LOGD("CCWA strdup error");
            return 1;
        }
        line = dup;
        err = at_tok_start(&line);
        if (err < 0) {
            free(dup); return 1;
        }
        err = at_tok_nextstr(&line, &number);
        if (err < 0) {
            free(dup); return 1;
        }
        cdmawait->number = (char*) alloca(strlen(number) + 1);
        assert(cdmawait->number != NULL);
        memset(cdmawait->number, 0, strlen(number) + 1);
        strncpy(cdmawait->number, number, strlen(number));

        err = at_tok_nextint(&line, &callType);
        if (err < 0) {
            free(dup); return 1;
        }
        if (145 == callType) {
            cdmawait->number_type = 1;
        } else if (129 == callType) {
            cdmawait->number_type = 2;
        }

        if (strlen(cachedCnap) > 0) {
            cdmawait->name = (char*) alloca(strlen(cachedCnap) + 1);
            assert(cdmawait->name != NULL);
            memset(cdmawait->name, 0, strlen(cachedCnap) + 1);
            strncpy(cdmawait->name, cachedCnap, strlen(cachedCnap));
            memset(cachedCnap, 0, CDMA_ALPHA_INFO_BUFFER_LENGTH);
        }

        if (!strcmp(cdmawait->number, "Restricted")) {
            cdmawait->numberPresentation = 1;
        }
        // Actually other unnormal number string may be distinguished as well.
        else if (!strcmp(cdmawait->number, "UNKNOWN")
                || !strcmp(cdmawait->number, "Unknown")
                || !strcmp(cdmawait->number, "Unavailable")
                || !strcmp(cdmawait->number, "NotAvailable")
                || !strcmp(cdmawait->number, "LOOPBACK CALL")) {
            cdmawait->numberPresentation = 2;
        }
        if (cdmawait->numberPresentation != 0) {
            cdmawait->number = NULL;
        }

        RIL_onUnsolicitedResponse(RIL_UNSOL_CDMA_CALL_WAITING, cdmawait,
                sizeof(RIL_CDMA_CallWaiting_v6));
        free(dup);
        return 1;
    } else if (strStartsWith(s, "+CRING:") || strStartsWith(s, "RING")) {
        callState = INCOMING;
        RIL_onUnsolicitedResponse(RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED,
                NULL, 0);

        //To send ring notification to Telephony FW
        RIL_onUnsolicitedResponse(RIL_UNSOL_CALL_RING, NULL, 0);
        return 1;
    } else if (strStartsWith(s, "^CEND:")) {
        callState = IDLE;
        int end_status = 0;
        int skip = 0;

        char *dup = strdup(s);
        if (!dup) {
            return 1;
        }
        line = dup;
        err = at_tok_start(&line);
        if (err < 0) {
            free(dup); return 1;
        }
        err = at_tok_nextint(&line, &skip);
        if (err < 0) {
            free(dup); return 1;
        }
        err = at_tok_nextint(&line, &skip);
        if (err < 0) {
            free(dup); return 1;
        }
        err = at_tok_nextint(&line, &end_status);
        if (err < 0) {
            free(dup); return 1;
        }

        if (end_status == 1) {  // Phone is locked
            s_unsol_msg_cache.last_call_fail_cause = CALL_FAIL_CDMA_LOCKED_UNTIL_POWER_CYCLE;
        } else if (end_status == 22) {  // Call faded/dropped
            s_unsol_msg_cache.last_call_fail_cause = CALL_FAIL_CDMA_DROP;
        } else if (end_status == 23) {  // Received Intercept from base station
            s_unsol_msg_cache.last_call_fail_cause = CALL_FAIL_CDMA_INTERCEPT;
        } else if (end_status == 24) {  // Received Reorder from base station
            s_unsol_msg_cache.last_call_fail_cause = CALL_FAIL_CDMA_REORDER;
        } else if (end_status == 26) {  // Service option rejected by base station
            s_unsol_msg_cache.last_call_fail_cause = CALL_FAIL_CDMA_SO_REJECT;
        } else if (end_status == 241) {
            s_unsol_msg_cache.last_call_fail_cause = CALL_FAIL_FDN_BLOCKED;
        } else if (end_status == 0 ||  // Phone is offline
                end_status == 12 ||   // Internal SW aborted the origination
                end_status == 21 ||   // Phone is out of service
                end_status == 25 ||   // Received Release Order from base station
                end_status == 27 ||   // There is incoming call
                end_status == 28 ||   // Received an alert stop from base station
                end_status == 30 ||   // Received end activation -OTASP calls only
                end_status == 32 ||   // RUIM is not available
                end_status == 99 ||   // NW directed system selection error
                end_status == 100 ||  // Released by lower layer
                end_status == 101 ||  // After a MS initiates a call, NW fails to respond
                end_status == 102 ||  // Phone rejects an incoming call
                end_status == 103 ||  // A call is rejected during the put-through process
                end_status == 104 ||  // The release is from NW
                end_status == 105 ||  // The phone fee is used up
                end_status == 106) {  // The MS is out of the service area
            s_unsol_msg_cache.last_call_fail_cause = CALL_FAIL_ERROR_UNSPECIFIED;
        } else {
            s_unsol_msg_cache.last_call_fail_cause = CALL_FAIL_NORMAL;
        }

        free(dup);

        invoicecall = 0;
        /*Clear loopback call query times*/
        if (loopbackcall_query_times) {
            loopbackcall_query_times = 0;
        }

        /* Notify call info for ECC redial */
        notifyCallInformationForEccRedial(s, CALL_DISCONNECTED_MSG, end_status);

        RIL_onUnsolicitedResponse(
                RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED,
                NULL, 0);
        return 1;
    } else if (strStartsWith(s, "NO CARRIER")) {
        RIL_onUnsolicitedResponse(
                RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED,
                NULL, 0);
        return 1;
    } else if (strStartsWith(s, "^ORIG")) {
        //"%s:%d,%d", Msg.cmdName, callID, callType
        //LOGD("Call Originated callid = %d, calltype = ", callID, callType);
        LOGD("Call Originating...");
        callState = DIALING;

        /* Notify call info for ECC redial */
        notifyCallInformationForEccRedial(s, CALL_ID_ASSIGN_MSG, 0);

        RIL_onUnsolicitedResponse(
                RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED,
                NULL, 0);
        return 1;
    } else if (strStartsWith(s, "^CONN:")) {
        LOGD("Call connected voicetype = %d", voicetype);
        callState = CONNECTED;
        invoicecall = 1;

        /* Notify call info for ECC redial */
        notifyCallInformationForEccRedial(s, CALL_CONNECTED_MSG, 0);

        RIL_onUnsolicitedResponse(
                RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED,
                NULL, 0);
        return 1;
    } else if (strStartsWith(s, "+EVOCD")) {
        onSpeechCodecInfo((char*) s);
        return 1;
    } else if (strStartsWith(s, "+CLIP")) {
        onCallingPartyNumberInfo((char*) s);
        return 1;
    } else if (strStartsWith(s, "+REDIRNUM")) {
        onRedirectingNumberInfo((char*) s);
        return 1;
    } else if (strStartsWith(s, "+LINECON")) {
        onLineControlInfo((char*) s);
        return 1;
    } else if (strStartsWith(s, "+CEXTD")) {
        onExtendedDisplayInfo((char*) s);
        return 1;
    } else if (strStartsWith(s, "+CFNM")) {
        onDisplayAndSignalsInfo((char*) s);
        return 1;
    } else if (strStartsWith(s, "+CIEV:")) {
        onCallControlStatus(s);
        return 1;
    } else if (strStartsWith(s, "+EVPMODE")) {
        RIL_onUnsolicitedResponse(
                RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED,
                NULL, 0);
        return 1;
    // Support EF ECC @{
    } else if (strStartsWith(s, "+CECC:")) {
        onHandleEfEccUrc(s);
        return 1;
    }
    // @}
    return 0;
}

void clearCnap() {
    int i;
    memset(cachedCnap, 0, CDMA_ALPHA_INFO_BUFFER_LENGTH);
    for (i = 0; i < MAX_GSMCALL_CONNECTIONS; i++) {
        memset(cachedClccName[i], 0, CDMA_ALPHA_INFO_BUFFER_LENGTH);
    }
}

static void requestGetCurrentCalls(void *data, size_t datalen, RIL_Token t) {
    int err = 0;
    ATResponse *p_response = NULL;
    ATLine *p_cur = NULL;
    int countCalls = 0;
    int countValidCalls = 0;
    RIL_Call *p_calls = NULL;
    RIL_Call **pp_calls = NULL;
    int i = 0;
    int datacall_active = 0;

    UNUSED(data);
    UNUSED(datalen);

    err = at_send_command_multiline ("AT+CLCC", "+CLCC:", &p_response, CC_CHANNEL_CTX);

    if (err < 0 || p_response == NULL || p_response->success == 0) {
        goto error;
    }

    /* count the calls */
    for (countCalls = 0, p_cur = p_response->p_intermediates
            ; p_cur != NULL
            ; p_cur = p_cur->p_next) {
        countCalls++;
    }

    /* yes, there's an array of pointers and then an array of structures */

    pp_calls = (RIL_Call**) alloca(countCalls * sizeof(RIL_Call*));
    p_calls = (RIL_Call*) alloca(countCalls * sizeof(RIL_Call));
    assert(pp_calls != NULL);
    assert(p_calls != NULL);
    memset (p_calls, 0, countCalls * sizeof(RIL_Call));
    for (countValidCalls = 0, p_cur = p_response->p_intermediates
            ; p_cur != NULL
            ; p_cur = p_cur->p_next) {
        p_calls[countValidCalls].name = (char*) alloca(CDMA_ALPHA_INFO_BUFFER_LENGTH);
        assert(p_calls[countValidCalls].name != NULL);
        memset(p_calls[countValidCalls].name, 0, CDMA_ALPHA_INFO_BUFFER_LENGTH);
        err = callFromCLCCLine(p_cur->line, p_calls + countValidCalls, countValidCalls);
        if (err != 0) {
            continue;
        }

        if (!p_calls[countValidCalls].isVoice && (p_calls[countValidCalls].number != NULL)) {
            if (!strcmp(p_calls[countValidCalls].number, "#777")) {
                datacall_active = 1;
            }
        }
        countValidCalls++;
    }

    for (i = 0; i < countValidCalls; i++) {
        pp_calls[i] = &(p_calls[i]);
    }

    if (countValidCalls == 0) {
        clearCnap();
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, pp_calls,
            countValidCalls * sizeof(RIL_Call*));

    at_response_free(p_response);

    return;

error:
    clearCnap();
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

static void sendCallStateChanged(void *param) {
    UNUSED(param);
    RIL_onUnsolicitedResponse(RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, NULL, 0);
}

static void requestDial(void *data, size_t datalen, RIL_Token t) {
    ATResponse *p_response = NULL;
    RIL_Dial *p_dial = (RIL_Dial*) data;
    char *cmd;
    const char *clir;
    int ret;
    char *formatNumber = NULL;
    char *origNumber = NULL;
    int len = strlen(p_dial->address);

    UNUSED(datalen);

    if (len == 0) {
        LOGE("%s, invalid number", __FUNCTION__);
        goto error;
    }

    switch (p_dial->clir) {
        case 1: clir = "I"; break;  /*invocation*/
        case 2: clir = "i"; break;  /*suppression*/
        default:
        case 0: clir = ""; break;   /*subscription default*/
    }
    char *tmp = strchr(p_dial->address, ',');
    char *addrNoPlusCode = NULL;
    if (tmp != NULL) {
        int pos = (tmp - p_dial->address) / sizeof(char);
        formatNumber = (char*) alloca(pos + 1);
        origNumber = (char*) alloca(len - pos);
        assert(formatNumber != NULL);
        assert(origNumber != NULL);
        memset(formatNumber, 0, pos + 1);
        memset(origNumber, 0, len - pos);
        strncpy(formatNumber, p_dial->address, pos);
        strncpy(origNumber, tmp + 1, len - pos - 1);
        addrNoPlusCode = strchr(formatNumber, '+');
    } else {
        formatNumber = p_dial->address;
        addrNoPlusCode = strchr(formatNumber, '+');
    }

    if (addrNoPlusCode != NULL) {
        addrNoPlusCode = addrNoPlusCode + 1;
    } else {
        addrNoPlusCode = formatNumber;
    }

    if (strlen(addrNoPlusCode) == 0) {
        LOGE("%s, invalid number", __FUNCTION__);
        goto error;
    }

    if (origNumber == NULL) {
        asprintf(&cmd, "AT+CDV=%s", addrNoPlusCode);
    } else {
        asprintf(&cmd, "AT+CDV=%s,,,,%s", addrNoPlusCode, origNumber);
    }

    ret = at_send_command(cmd, &p_response, CC_CHANNEL_CTX);
    free(cmd);
    if (ret < 0 || p_response == NULL || p_response->success == 0) {
        goto error;
    }
    /* success or failure is ignored by the upper layer here.
       it will call GET_CURRENT_CALLS and determine success that way */
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    rilCcUnsolicited(p_response->finalResponse, NULL);
    at_response_free(p_response);
    return;

error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

static void requestHangup(void *data, size_t datalen, RIL_Token t) {
    int ret;
    int i = 0;
    ATResponse *p_response = NULL;
    int waitInterval = 400; //ms
    int maxWaitCount = 25;  //waitInterval*maxWaitCount = 10s;
    RIL_CALL_STATUS callStatus;

    UNUSED(data);
    UNUSED(datalen);

    ret = at_send_command("AT+CHV", &p_response, CC_CHANNEL_CTX);
    if (ret < 0 || p_response == NULL || p_response->success == 0) {
        goto error;
    }

    /* success or failure is ignored by the upper layer here.
       it will call GET_CURRENT_CALLS and determine success that way */
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    at_response_free(p_response);
    return;
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

static void requestGetLastCallFailCause(void *data, size_t datalen, RIL_Token t) {
    int responses = 0;

    UNUSED(data);
    UNUSED(datalen);

    if (s_unsol_msg_cache.last_call_fail_cause == 0) {
        responses = CALL_FAIL_NORMAL;
    } else {
        responses = s_unsol_msg_cache.last_call_fail_cause;
    }

    /* To avoid tone playing if the radio is unavailable */
    if (getRadioState() == RADIO_STATE_UNAVAILABLE) {
        responses = CALL_FAIL_CDMA_ACCESS_FAILURE;
    }
    LOGD("Call exit code is %d, convert to fail cause %d",
            s_unsol_msg_cache.last_call_fail_cause, responses);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, &responses, sizeof(int));
}

static void requestSetMute(void *data, size_t datalen, RIL_Token t) {
    int err;
    char *cmd;
    assert(datalen >= sizeof(int*));

    UNUSED(datalen);

    LOGD("%s, ((int*)data)[0]:%d", __FUNCTION__, ((int*) data)[0]);
    asprintf(&cmd, "AT+CMUT=%d", ((int*) data)[0]);
    err = at_send_command(cmd, NULL, CC_CHANNEL_CTX);
    free(cmd);
    if (err != 0) { goto error; }
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;

error:
    LOGE("%s failed", __FUNCTION__);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

static void requestQueryPreferredVoicePrivacy(
                void *data, size_t datalen, RIL_Token t) {
    int err = 0;
    ATResponse *p_response = NULL;
    int response = 0;
    char *line = NULL;

    UNUSED(data);
    UNUSED(datalen);

    err = at_send_command_singleline("AT+VP?", "+VP:", &p_response, CC_CHANNEL_CTX);

    if (err < 0 || p_response == NULL || p_response->success == 0) {
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);

    if (err < 0) {
        goto error;
    }

    err = at_tok_nextint(&line, &response);

    if (err < 0) {
        goto error;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(int));
    at_response_free(p_response);
    return;
error:
    at_response_free(p_response);
    LOGE("%s failed", __FUNCTION__);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

static void requestSendFlash(void *data, size_t datalen, RIL_Token t) {
    char *number = (char*) data;
    int len = 0;
    char *cmd = NULL;
    int ret;
    char formatString[16] = {0};
    char origString[16] = {0};
    char *formatNumber = NULL;
    char *origNumber = NULL;
    char *tmp = NULL;

    UNUSED(datalen);

    if (number != NULL) {
        len = strlen(number);
        tmp = strchr(number, ',');
        if (tmp != NULL) {
            int pos = (tmp - number) / sizeof(char);
            formatNumber = (char*) alloca(pos + 1);
            origNumber = (char*) alloca(len - pos);
            assert(formatNumber != NULL);
            assert(origNumber != NULL);
            memset(formatNumber, 0, pos + 1);
            memset(origNumber, 0, len - pos);
            strncpy(formatNumber, number, pos);
            strncpy(origNumber, tmp + 1, len - pos - 1);
        }
    }

    if (len == 0) {
        asprintf(&cmd, "AT+CFSH=");
    } else if (tmp == NULL) {
        if (len < 16) {
            asprintf(&cmd, "AT+CFSH=%s", number);
        } else {
            strncpy(formatString, number, 15);
            asprintf(&cmd, "AT+CFSH=%s", formatString);
        }
    } else {
        if (strlen(formatNumber) < 16) {
            asprintf(&cmd, "AT+CFSH=%s", number);
        } else {
            strncpy(formatString, formatNumber, 15);
            strncpy(origString, origNumber, 15);
            asprintf(&cmd, "AT+CFSH=%s,%s", formatString, origString);
        }
    }
    if (cmd != NULL) {
        ret = at_send_command(cmd, NULL, CC_CHANNEL_CTX);
        free(cmd);
    }
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
}

static void requestExitEmergencyCallbackMode(void *data, size_t datalen, RIL_Token t) {
    int ret = 0;

    UNUSED(data);
    UNUSED(datalen);

    LOGD("%s", __FUNCTION__);
    if (s_md3_off) {
        LOGD("%s MD off, just return success", __FUNCTION__);
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        return;
    }
    ret = at_send_command("AT+VMEMEXIT", NULL, CC_CHANNEL_CTX);
    if (ret != 0) {
        goto error;
    }
    //inemergency = 0;
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;
error:
    LOGD("%s failed", __FUNCTION__);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

static void requestLocalEmergencyDial(void *data, size_t datalen, RIL_Token t) {
    ATResponse *p_response = NULL;
    RIL_Dial *p_dial = (RIL_Dial*) data;
    char *cmd = NULL;
    const char *clir;
    int ret;
    char *formatNumber = NULL;
    char *origNumber = NULL;
    int len = strlen(p_dial->address);

    UNUSED(datalen);

    if (len == 0) {
        LOGE("%s, invalid number", __FUNCTION__);
        goto error;
    }

    switch (p_dial->clir) {
        case 1: clir = "I"; break;  /*invocation*/
        case 2: clir = "i"; break;  /*suppression*/
        default:
        case 0: clir = ""; break;   /*subscription default*/
    }
    char *tmp = strchr(p_dial->address, ',');
    if (tmp != NULL) {
        int pos = (tmp - p_dial->address) / sizeof(char);
        formatNumber = (char*) alloca(pos + 1);
        origNumber = (char*) alloca(len - pos);
        assert(formatNumber != NULL);
        assert(origNumber != NULL);
        memset(formatNumber, 0, pos + 1);
        memset(origNumber, 0, len - pos);
        strncpy(formatNumber, p_dial->address, pos);
        strncpy(origNumber, tmp + 1, len - pos - 1);
    }

    inemergency = 1;

    if (origNumber == NULL) {
        asprintf(&cmd, "AT+CDV=%s,1", p_dial->address);
    } else {
        asprintf(&cmd, "AT+CDV=%s,1,,,%s", formatNumber, origNumber);
    }

    ret = at_send_command(cmd, &p_response, CC_CHANNEL_CTX);
    free(cmd);
    if (ret < 0 || p_response == NULL || p_response->success == 0) {
        goto error;
    }
    /* success or failure is ignored by the upper layer here.
       it will call GET_CURRENT_CALLS and determine success that way */
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    at_response_free(p_response);
    return;

error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

/*
 * Test command: AT+VGSMST=?
 * Response: OK
 *
 * Write command: AT+VGSMST =<status>
 * Response: 1) If successful: OK
 *           2) If failed: ERROR
 *
 * Parameters:
 * <value>   0       GSM in no call state
 *           1       GSM in CALL state.
 *           Other   reserved
 */
static void requestSwitchHPF(void *data, size_t datalen, RIL_Token t) {
    int onOff;
    int err = 0;
    char *cmd = NULL;
    ATResponse *p_response = NULL;

    UNUSED(datalen);

    onOff = ((int*) data)[0];

    LOGD("%s, onOff:%d", __FUNCTION__, onOff);
    asprintf(&cmd, "AT+VGSMST=%d", onOff);
    err = at_send_command(cmd, &p_response, CC_CHANNEL_CTX);
    free(cmd);
    if (err < 0 || p_response == NULL || p_response->success == 0) {
        goto error;
    }

    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;

error:
    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

/**
 * Note: directly modified line and has *p_call point directly into
 * modified line
 */
static int callFromCLCCLine(char *line, RIL_Call *p_call, int validIndex) {
    int err;
    int state;
    int mode;
    err = at_tok_start(&line);
    if (err < 0) {
        goto error;
    }

    err = at_tok_nextint(&line, &(p_call->index));
    if (err < 0) {
        goto error;
    }
    // MD always return invalid index, not starting from 0, so replace it.
    p_call->index = validIndex + 1;

    err = at_tok_nextbool(&line, &(p_call->isMT));
    if (err < 0) {
        goto error;
    }
    err = at_tok_nextint(&line, &state);
    if (err < 0) {
        goto error;
    }
    err = clccStateToRILState(state, &(p_call->state));
    if (err < 0) {
        goto error;
    }

    // CNAP URC is sent after RING and before CLCC polling,
    // therefore we create a cached array to store the name of MT call, with the cached CNAP,
    // so that the name in subsequent CLCC polling can be filled with the cached CNAP array,
    // even the call becomes ACTIVE later.
    if (p_call->state == RIL_CALL_INCOMING) {
        if ((p_call->index >= 1) && (p_call->index <= MAX_GSMCALL_CONNECTIONS)) {
            if (strlen(cachedCnap) > 0) {
                memset(cachedClccName[p_call->index-1], 0, CDMA_ALPHA_INFO_BUFFER_LENGTH);
                strncpy(cachedClccName[p_call->index-1], cachedCnap,
                        CDMA_ALPHA_INFO_BUFFER_LENGTH - 1);
                memset(cachedCnap, 0, CDMA_ALPHA_INFO_BUFFER_LENGTH);
            }
        }
    }

    // Fill CNAP in individual CLCC parsing and clear in ECPI:133
    if ((p_call->index >= 1) && (p_call->index <= MAX_GSMCALL_CONNECTIONS)) {
        if (strlen(cachedClccName[p_call->index-1]) > 0) {
            strncpy(p_call->name, cachedClccName[p_call->index-1],
                    CDMA_ALPHA_INFO_BUFFER_LENGTH - 1);
        }
    }

    err = at_tok_nextint(&line, &mode);
    if (err < 0) {
        goto error;
    }

    p_call->isVoice = (mode == 0);
    if (!p_call->isVoice) {
        LOGE("Filter data call line!");
        goto error;
    }

    err = at_tok_nextbool(&line, &(p_call->isMpty));
    if (err < 0) {
        goto error;
    }
    if (at_tok_hasmore(&line)) {
        err = at_tok_nextstr(&line, &(p_call->number));
        /* tolerate null here */
        if (err < 0) { return 0; }
        // Some lame implementations return strings
        // like "NOT AVAILABLE" in the CLCC line
        if (p_call->number != NULL && 0 == strspn(p_call->number, "+*#pw0123456789")) {
            if (strcmp(p_call->number, "Restricted")
                    && strcmp(p_call->number, "NotAvailable")
                    && strcmp(p_call->number, "UNKNOWN")
                    && strcmp(p_call->number, "LOOPBACK CALL")) {
                LOGD("Set number to null!");
                p_call->number = NULL;
            }
        }
        err = at_tok_nextint(&line, &p_call->toa);
        if (err < 0) { goto error; }
    }
    if (p_call->number != NULL && strcmp(p_call->number, "LOOPBACK CALL") == 0) {
        /*For SO55 test, */
        if (p_call->state == 0 && loopbackcall_query_times == 0) {
            p_call->state = 4;
        }
        loopbackcall_query_times++;
    }

    //for fix CRTS#19922
    if (p_call->number != NULL && !strcmp(p_call->number, "Restricted")) {
        p_call->numberPresentation = 1;
    }
    /* Actually other unnormal number string may be
        distinguished as well.
    */
    else if (p_call->number != NULL && (!strcmp(p_call->number, "UNKNOWN")
        || !strcmp(p_call->number, "Unknown")
        || !strcmp(p_call->number, "Unavailable")
        || !strcmp(p_call->number, "NotAvailable")
        || !strcmp(p_call->number, "LOOPBACK CALL"))) {
        p_call->numberPresentation = 2;
    }
    if (p_call->numberPresentation != 0) {
        p_call->number = NULL;
    }

    return 0;

error:
    LOGE("Invalid CLCC line");
    return -1;
}

int clccStateToRILState(int state, RIL_CallState *p_state) {
    switch (state) {
        case 0: *p_state = RIL_CALL_ACTIVE;   return 0;
        case 1: *p_state = RIL_CALL_HOLDING;  return 0;
        case 2: *p_state = RIL_CALL_DIALING;  return 0;
        case 3: *p_state = RIL_CALL_ALERTING; return 0;
        case 4: *p_state = RIL_CALL_INCOMING; return 0;
        case 5: *p_state = RIL_CALL_WAITING;  return -1;
        default: return -1;
    }
}

void onSpeechCodecInfo(char *urc) {
    int ret;
    int info = 0;
    LOGD("%s, urc:%s", __FUNCTION__, urc);
    if (strStartsWith(urc, "+EVOCD")) {
        ret = at_tok_start(&urc);
        if (ret < 0) { goto error; }

        ret = at_tok_nextint(&urc, &info);
        if (ret < 0) { goto error; }
    } else {
        goto error;
    }

    LOGD("%s, value:%d", __FUNCTION__, info);
    RIL_onUnsolicitedResponse(RIL_UNSOL_SPEECH_CODEC_INFO, &info, sizeof(int));
    return;

error:
    LOGE("Error on %s", __FUNCTION__);
}

void onCallingPartyNumberInfo(char *urc) {
    int ret;
    RIL_CDMA_InformationRecords *cdmainfo = NULL;
    cdmainfo = (RIL_CDMA_InformationRecords*) alloca(
            sizeof(RIL_CDMA_InformationRecords));
    assert(cdmainfo != NULL);
    memset(cdmainfo, 0, sizeof(RIL_CDMA_InformationRecords));

    int numOfRecs = 0;

    // +CLI:<number>,<type>,,,,<cli_validity>
    if (strStartsWith(urc, "+CLIP")) {
        int numberType, cliValidity;
        int dummy;
        char *number = NULL;
        char *line = NULL;
        char *dup = strdup(urc);
        line = dup;

        ret = at_tok_start(&line);
        if (ret < 0) { free(dup); goto error;}

        ret = at_tok_nextstr(&line, &number);
        if (ret < 0) { free(dup); goto error; }

        ret = at_tok_nextint(&line, &numberType);
        if (ret < 0) { free(dup); goto error; }

        ret = at_tok_nextint(&line, &dummy);
        ret = at_tok_nextint(&line, &dummy);
        ret = at_tok_nextint(&line, &dummy);
        ret = at_tok_nextint(&line, &cliValidity);

        cdmainfo->infoRec[numOfRecs].name = RIL_CDMA_CALLING_PARTY_NUMBER_INFO_REC;
        if (number != NULL &&
                strlen(number) > 0 && strlen(number) < CDMA_NUMBER_INFO_BUFFER_LENGTH) {
            LOGD("%s, number != NULL", __FUNCTION__);
            cdmainfo->infoRec[numOfRecs].rec.number.len = strlen(number);
            strncpy(cdmainfo->infoRec[numOfRecs].rec.number.buf, number,
                    CDMA_NUMBER_INFO_BUFFER_LENGTH - 1);
        } else {
            LOGD("%s, number = NULL or invalid", __FUNCTION__);
            cdmainfo->infoRec[numOfRecs].rec.number.len = 0;
            strncpy(cdmainfo->infoRec[numOfRecs].rec.number.buf, "",
                    CDMA_NUMBER_INFO_BUFFER_LENGTH - 1);
        }
        cdmainfo->infoRec[numOfRecs].rec.number.number_type = ((char)numberType>>4) & 0x07;
        cdmainfo->infoRec[numOfRecs].rec.number.number_plan = ((char)numberType) & 0x07;
        switch (cliValidity) {
            case 0:
                cdmainfo->infoRec[numOfRecs].rec.number.pi = 0;  // Presentation allowed
                break;
            case 1:
            case 2:
                cdmainfo->infoRec[numOfRecs].rec.number.pi = 1;  // Presentation restricted
                break;
            default:
                cdmainfo->infoRec[numOfRecs].rec.number.pi = 2;  // Number not available
                break;
        }
        cdmainfo->infoRec[numOfRecs].rec.number.si = 0;  // User provided, not screened

        //Do not show private information in user load, ex: phone number
        LOGD("%s, RIL_CDMA_CALLING_PARTY_NUMBER_INFO_REC: len:%d number_type:%d \
            number_plan:%d pi:%d si:%d", __FUNCTION__,
            cdmainfo->infoRec[numOfRecs].rec.number.len,
            cdmainfo->infoRec[numOfRecs].rec.number.number_type,
            cdmainfo->infoRec[numOfRecs].rec.number.number_plan,
            cdmainfo->infoRec[numOfRecs].rec.number.pi,
            cdmainfo->infoRec[numOfRecs].rec.number.si);

        numOfRecs++;

        free(dup);
    } else {
        goto error;
    }

    cdmainfo->numberOfInfoRecs = numOfRecs;
    LOGD("%s, RIL_UNSOL_CDMA_INFO_REC: numberOfInfoRecs:%d", __FUNCTION__,
             cdmainfo->numberOfInfoRecs);
    RIL_onUnsolicitedResponse(RIL_UNSOL_CDMA_INFO_REC,
            cdmainfo, sizeof(RIL_CDMA_InformationRecords));
    return;

error:
    LOGE("Error on %s", __FUNCTION__);
}

void onRedirectingNumberInfo(char *urc) {
    int ret;
    RIL_CDMA_InformationRecords *cdmainfo = NULL;
    cdmainfo = (RIL_CDMA_InformationRecords*) alloca(
            sizeof(RIL_CDMA_InformationRecords));
    assert(cdmainfo != NULL);
    memset(cdmainfo, 0, sizeof(RIL_CDMA_InformationRecords));

    int numOfRecs = 0;

    // +REDIRNUM:<ext_bit_1>,<number_type>,<number_plan>,<ext_bit_2>,<pi>,<si>,
    //                    <ext_bit_3>,<redirection_reason>,<number>
    if (strStartsWith(urc, "+REDIRNUM")) {
        int ext1, numberType, numberPlan, ext2, pi, si, ext3, redirReason;
        char *number = NULL;
        char *line = NULL;
        char *dup = strdup(urc);
        line = dup;

        ret = at_tok_start(&line);
        if (ret < 0) { free(dup); goto error;}

        ret = at_tok_nextint(&line, &ext1);
        if (ret < 0) { free(dup); goto error; }

        ret = at_tok_nextint(&line, &numberType);
        if (ret < 0) { free(dup); goto error; }

        ret = at_tok_nextint(&line, &numberPlan);
        if (ret < 0) { free(dup); goto error; }

        ret = at_tok_nextint(&line, &ext2);
        if (ret < 0) { free(dup); goto error; }

        ret = at_tok_nextint(&line, &pi);
        if (ret < 0) { free(dup); goto error; }

        ret = at_tok_nextint(&line, &si);
        if (ret < 0) { free(dup); goto error; }

        ret = at_tok_nextint(&line, &ext3);
        if (ret < 0) { free(dup); goto error; }

        ret = at_tok_nextint(&line, &redirReason);
        if (ret < 0) { free(dup); goto error; }

        ret = at_tok_nextstr(&line, &number);
        if (ret < 0) { free(dup); goto error; }

        cdmainfo->infoRec[numOfRecs].name = RIL_CDMA_REDIRECTING_NUMBER_INFO_REC;
        if (number != NULL &&
                strlen(number) > 0 && strlen(number) < CDMA_NUMBER_INFO_BUFFER_LENGTH) {
            LOGD("%s, number != NULL", __FUNCTION__);
            cdmainfo->infoRec[numOfRecs].rec.redir.redirectingNumber.len = strlen(number);
            strncpy(cdmainfo->infoRec[numOfRecs].rec.redir.redirectingNumber.buf, number,
                    CDMA_NUMBER_INFO_BUFFER_LENGTH - 1);
        } else {
            LOGD("%s, number = NULL or invalid", __FUNCTION__);
            cdmainfo->infoRec[numOfRecs].rec.redir.redirectingNumber.len = 0;
            strncpy(cdmainfo->infoRec[numOfRecs].rec.redir.redirectingNumber.buf, "",
                    CDMA_NUMBER_INFO_BUFFER_LENGTH - 1);
        }
        cdmainfo->infoRec[numOfRecs].rec.redir.redirectingNumber.number_type = (char) numberType;
        cdmainfo->infoRec[numOfRecs].rec.redir.redirectingNumber.number_plan = (char) numberPlan;
        cdmainfo->infoRec[numOfRecs].rec.redir.redirectingNumber.pi = (char) pi;
        cdmainfo->infoRec[numOfRecs].rec.redir.redirectingNumber.si = (char) si;
        cdmainfo->infoRec[numOfRecs].rec.redir.redirectingReason =
                (RIL_CDMA_RedirectingReason) redirReason;

        LOGD("%s, RIL_CDMA_REDIRECTING_NUMBER_INFO_REC: len:%d number_type:%d \
                number_plan:%d pi:%d si:%d redirectingReason:%d", __FUNCTION__,
                cdmainfo->infoRec[numOfRecs].rec.redir.redirectingNumber.len,
                cdmainfo->infoRec[numOfRecs].rec.redir.redirectingNumber.number_type,
                cdmainfo->infoRec[numOfRecs].rec.redir.redirectingNumber.number_plan,
                cdmainfo->infoRec[numOfRecs].rec.redir.redirectingNumber.pi,
                cdmainfo->infoRec[numOfRecs].rec.redir.redirectingNumber.si,
                cdmainfo->infoRec[numOfRecs].rec.redir.redirectingReason);
        numOfRecs++;

        free(dup);
    } else {
        goto error;
    }

    cdmainfo->numberOfInfoRecs = numOfRecs;
    LOGD("%s, RIL_UNSOL_CDMA_INFO_REC: numberOfInfoRecs:%d", __FUNCTION__,
             cdmainfo->numberOfInfoRecs);
    RIL_onUnsolicitedResponse(RIL_UNSOL_CDMA_INFO_REC,
            cdmainfo, sizeof(RIL_CDMA_InformationRecords));
    return;

error:
    LOGE("Error on %s", __FUNCTION__);
}

void onLineControlInfo(char *urc) {
    int ret;
    RIL_CDMA_InformationRecords *cdmainfo = NULL;
    cdmainfo = (RIL_CDMA_InformationRecords*) alloca(
            sizeof(RIL_CDMA_InformationRecords));
    assert(cdmainfo != NULL);

    int numOfRecs = 0;

    // +LINECON:<polarity_included>,<toggle_mode>,<reverse_polarity>,<power_denial_time>
    if (strStartsWith(urc, "+LINECON")) {
        int polarityIncluded, toggleMode, reversePolarity, powerDenialTime;
        char *line = NULL;
        char *dup = strdup(urc);
        line = dup;

        ret = at_tok_start(&line);
        if (ret < 0) { free(dup); goto error;}

        ret = at_tok_nextint(&line, &polarityIncluded);
        if (ret < 0) { free(dup); goto error; }

        ret = at_tok_nextint(&line, &toggleMode);
        if (ret < 0) { free(dup); goto error; }

        ret = at_tok_nextint(&line, &reversePolarity);
        if (ret < 0) { free(dup); goto error; }

        ret = at_tok_nextint(&line, &powerDenialTime);
        if (ret < 0) { free(dup); goto error; }

        cdmainfo->infoRec[numOfRecs].name = RIL_CDMA_LINE_CONTROL_INFO_REC;
        cdmainfo->infoRec[numOfRecs].rec.lineCtrl.lineCtrlPolarityIncluded =
                (char) polarityIncluded;
        cdmainfo->infoRec[numOfRecs].rec.lineCtrl.lineCtrlToggle = (char) toggleMode;
        cdmainfo->infoRec[numOfRecs].rec.lineCtrl.lineCtrlReverse = (char) reversePolarity;
        cdmainfo->infoRec[numOfRecs].rec.lineCtrl.lineCtrlPowerDenial = (char) powerDenialTime;

        LOGD("%s, RIL_CDMA_LINE_CONTROL_INFO_REC: lineCtrlPolarityIncluded:%d lineCtrlToggle:%d \
                lineCtrlReverse:%d lineCtrlPowerDenial:%d", __FUNCTION__,
                cdmainfo->infoRec[numOfRecs].rec.lineCtrl.lineCtrlPolarityIncluded,
                cdmainfo->infoRec[numOfRecs].rec.lineCtrl.lineCtrlToggle,
                cdmainfo->infoRec[numOfRecs].rec.lineCtrl.lineCtrlReverse,
                cdmainfo->infoRec[numOfRecs].rec.lineCtrl.lineCtrlPowerDenial);

        numOfRecs++;

        free(dup);
    } else {
        goto error;
    }

    cdmainfo->numberOfInfoRecs = numOfRecs;
    LOGD("%s, RIL_UNSOL_CDMA_INFO_REC: numberOfInfoRecs:%d", __FUNCTION__,
             cdmainfo->numberOfInfoRecs);
    RIL_onUnsolicitedResponse(RIL_UNSOL_CDMA_INFO_REC,
            cdmainfo, sizeof(RIL_CDMA_InformationRecords));
    return;

error:
    LOGE("Error on %s", __FUNCTION__);
}

void onExtendedDisplayInfo(char *urc) {
    int ret;
    RIL_CDMA_InformationRecords *cdmainfo = NULL;
    cdmainfo = (RIL_CDMA_InformationRecords*) alloca(
            sizeof(RIL_CDMA_InformationRecords));
    assert(cdmainfo != NULL);
    memset(cdmainfo, 0, sizeof(RIL_CDMA_InformationRecords));

    int numOfRecs = 0;

    // +CEXTD:<display_tag>,<info>
    if (strStartsWith(urc, "+CEXTD")) {
        char *info = NULL;
        int displaytag;
        char *line = NULL;
        char *dup = strdup(urc);
        line = dup;

        ret = at_tok_start(&line);
        if (ret < 0) { free(dup); goto error;}

        ret = at_tok_nexthexint(&line, &displaytag);
        if (ret < 0) { free(dup); goto error; }

        if (displaytag == DISPLAY_TAG_BLANK || displaytag == DISPLAY_TAG_SKIP) {
            free(dup);
            goto error;
        }

        ret = at_tok_nextstr(&line, &info);
        if (ret < 0) { free(dup); goto error; }

        if (info != NULL &&
                strlen(info) > 0 && strlen(info) < CDMA_ALPHA_INFO_BUFFER_LENGTH &&
                strcmp(info, "N/A") != 0) {
            LOGD("callState=%d, displaytag=%d", callState, displaytag);
            if (displaytag == DISPLAY_TAG_CALLING_PARTY_NAME
                    || displaytag == DISPLAY_TAG_ORIG_CALLED_NAME) {
                snprintf(cachedCnap, CDMA_ALPHA_INFO_BUFFER_LENGTH - 1, "%s", info);

                // RING -> +CFNM/+CEXTD
                // +CFNM/+CEXTD -> +CCWA
                // No need to handle WAITING callState, since it +CCWA comes later than +CEXTD
                if (callState == INCOMING) {
                    RIL_onUnsolicitedResponse(RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, NULL, 0);
                    LOGD("skip RIL_CDMA_EXTENDED_DISPLAY_INFO_REC for there's incoming call");
                    free(dup);
                    goto error;
                }
            }

            cdmainfo->infoRec[numOfRecs].name = RIL_CDMA_EXTENDED_DISPLAY_INFO_REC;
            cdmainfo->infoRec[numOfRecs].rec.display.alpha_len = strlen(info);
            strncpy(cdmainfo->infoRec[numOfRecs].rec.display.alpha_buf, info,
                    CDMA_ALPHA_INFO_BUFFER_LENGTH - 1);

            LOGD("%s, RIL_CDMA_EXTENDED_DISPLAY_INFO_REC: alpha_len:%d",
                    __FUNCTION__,
                    cdmainfo->infoRec[numOfRecs].rec.display.alpha_len);

            numOfRecs++;

            free(dup);
        } else {
            free(dup);
            goto error;
        }
    } else {
        goto error;
    }

    cdmainfo->numberOfInfoRecs = numOfRecs;
    LOGD("%s, RIL_UNSOL_CDMA_INFO_REC: numberOfInfoRecs:%d", __FUNCTION__,
             cdmainfo->numberOfInfoRecs);
    RIL_onUnsolicitedResponse(RIL_UNSOL_CDMA_INFO_REC,
            cdmainfo, sizeof(RIL_CDMA_InformationRecords));
    return;

error:
    LOGE("Error on %s", __FUNCTION__);
}

void onDisplayAndSignalsInfo(char *urc) {
    int ret;
    RIL_CDMA_InformationRecords *cdmainfo = NULL;
    cdmainfo = (RIL_CDMA_InformationRecords*) alloca(
            sizeof(RIL_CDMA_InformationRecords));
    assert(cdmainfo != NULL);
    memset(cdmainfo, 0, sizeof(RIL_CDMA_InformationRecords));

    int numOfRecs = 0;

    // +CFNM:<display>,<signal_type>,<alert_pitch>,<signal>
    if (strStartsWith(urc, "+CFNM")) {
        char *display = NULL;
        int signalType, alertPitch, signal;
        char *line = NULL;
        char *dup = strdup(urc);
        line = dup;

        ret = at_tok_start(&line);
        if (ret < 0) { free(dup); goto error;}

        ret = at_tok_nextstr(&line, &display);
        if (ret < 0) { free(dup); goto error; }

        if (display != NULL &&
                strlen(display) > 0 && strlen(display) < CDMA_ALPHA_INFO_BUFFER_LENGTH &&
                strcmp(display, "N/A") != 0) {
            snprintf(cachedCnap, CDMA_ALPHA_INFO_BUFFER_LENGTH - 1, "%s", display);

            LOGD("callState=%d", callState);

            // RING -> +CFNM/+CEXTD
            // +CFNM/+CEXTD -> +CCWA
            // No need to handle WAITING callState, since it +CCWA comes later than +CFNM
            if (callState == INCOMING) {
                RIL_onUnsolicitedResponse(RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, NULL, 0);
                LOGD("skip RIL_CDMA_DISPLAY_INFO_REC for there's incoming call");
                free(dup);
                goto error;
            }

            cdmainfo->infoRec[numOfRecs].name = RIL_CDMA_DISPLAY_INFO_REC;
            cdmainfo->infoRec[numOfRecs].rec.display.alpha_len = strlen(display);
            strncpy(cdmainfo->infoRec[numOfRecs].rec.display.alpha_buf, display,
                    CDMA_ALPHA_INFO_BUFFER_LENGTH - 1);

            LOGD("%s, RIL_CDMA_DISPLAY_INFO_REC: alpha_len:%d", __FUNCTION__,
                    cdmainfo->infoRec[numOfRecs].rec.display.alpha_len);
            numOfRecs++;
        } else {
            LOGD("%s, skip since display is invalid", __FUNCTION__);
            if (callState == IDLE) {
                // For timing issue. There's always a +CFNM:N/A,2,0,1 after ringing,
                // but because of timing it can be handled before callState set to INCOMING.
                free(dup);
                goto error;
            }
        }

        if (callState == DIALING || callState == IDLE  // for WPS call...sgpark.08.19.
                // Busy tone can be played after state changed to active. [ALPS04700040]
                || callState == CONNECTED) {
            ret = at_tok_nextint(&line, &signalType);
            if (ret < 0) { free(dup); goto update; }

            ret = at_tok_nextint(&line, &alertPitch);
            if (ret < 0) { free(dup); goto update; }

            ret = at_tok_nextint(&line, &signal);
            if (ret < 0) { free(dup); goto update; }

            cdmainfo->infoRec[numOfRecs].name = RIL_CDMA_SIGNAL_INFO_REC;
            cdmainfo->infoRec[numOfRecs].rec.signal.isPresent = 1;
            cdmainfo->infoRec[numOfRecs].rec.signal.signalType = (char) signalType;
            cdmainfo->infoRec[numOfRecs].rec.signal.alertPitch = (char) alertPitch;
            cdmainfo->infoRec[numOfRecs].rec.signal.signal = (char) signal;

            LOGD("%s, RIL_CDMA_SIGNAL_INFO_REC: signalType:%d alertPitch:%d signal:%d",
                     __FUNCTION__,
                    cdmainfo->infoRec[numOfRecs].rec.signal.signalType,
                    cdmainfo->infoRec[numOfRecs].rec.signal.alertPitch,
                    cdmainfo->infoRec[numOfRecs].rec.signal.signal);

            numOfRecs++;
        } else {   // do not report SIGNAL_INFO_REC for incoming/call waiting call.
            LOGD("[LGE] Block to generate Signal Info Record");
        }
        free(dup);
    } else {
        goto error;
    }

update:
    cdmainfo->numberOfInfoRecs = numOfRecs;
    LOGD("%s, RIL_UNSOL_CDMA_INFO_REC: numberOfInfoRecs:%d", __FUNCTION__,
             cdmainfo->numberOfInfoRecs);
    RIL_onUnsolicitedResponse(RIL_UNSOL_CDMA_INFO_REC,
            cdmainfo, sizeof(RIL_CDMA_InformationRecords));
    return;

error:
    LOGE("Error on %s", __FUNCTION__);
}

/**
 * AT command format: +CIEV:<ind>,<value>
 */
void onCallControlStatus(const char *s) {
    char *line = NULL;
    int ind, err;
    int value;

    char *dup = strdup(s);
    if (!dup) {
       return;
    }
    line = dup;
    err = at_tok_start(&line);
    if (err < 0) {
       free(dup);
       return;
    }
    err = at_tok_nextint(&line, &ind);
    if (err < 0) {
       free(dup);
       return;
    }
    err = at_tok_nextint(&line, &value);
    if (err < 0) {
       free(dup);
       return;
    }
    LOGD("%s, ind:%d, value:%d", __FUNCTION__, ind, value);
    /* 102: ECBM Indicator */
    if (ind == 102) {
       /* value = 1: ECBM on
          value = 0: ECBM off */
       if (value == 1) {
          RIL_onUnsolicitedResponse(RIL_UNSOL_ENTER_EMERGENCY_CALLBACK_MODE, NULL, 0);
       } else if (value == 0) {
          RIL_onUnsolicitedResponse(RIL_UNSOL_EXIT_EMERGENCY_CALLBACK_MODE, NULL, 0);
       }
    } else if (ind == 13) {
        RIL_onUnsolicitedResponse(RIL_UNSOL_CDMA_CALL_ACCEPTED, NULL, 0);
    }
    free(dup);
}

/**
 * To redial normal/emergency call.
 *
 * AT command format: AT+CDV=<dial_digits>[,<n>[,<redial_mode>,<redial_call_id>]]
 * <n>: 0 = normal call, 1 = emergency call
 *
 */
void requestRedial(void *data, size_t datalen, RIL_Token t, int isEmergency) {
    RIL_Redial *p_reDial;
    char *cmd = NULL;
    int ret;
    ATResponse *p_response = NULL;

    UNUSED(datalen);

    p_reDial = (RIL_Redial*) data;

    char *formatNumber = NULL;
    char *origNumber = NULL;
    int len = strlen(p_reDial->address);
    char *tmp = strchr(p_reDial->address, ',');
    char *addrNoPlusCode = NULL;
    if (tmp != NULL) {
        int pos = (tmp - p_reDial->address) / sizeof(char);
        formatNumber = (char*) alloca(pos + 1);
        origNumber = (char*) alloca(len - pos);
        assert(formatNumber != NULL);
        assert(origNumber != NULL);
        memset(formatNumber, 0, pos + 1);
        memset(origNumber, 0, len - pos);
        strncpy(formatNumber, p_reDial->address, pos);
        strncpy(origNumber, tmp + 1, len - pos - 1);
        addrNoPlusCode = strchr(formatNumber, '+');
    } else {
        formatNumber = p_reDial->address;
        addrNoPlusCode = strchr(formatNumber, '+');
    }

    if (addrNoPlusCode != NULL) {
        addrNoPlusCode = addrNoPlusCode + 1;
    } else {
        addrNoPlusCode = formatNumber;
    }

    if (origNumber == NULL) {
        asprintf(&cmd, "AT+CDV=%s,%d,%d,%d", addrNoPlusCode, isEmergency,
                p_reDial->mode, p_reDial->callId);
    } else {
        asprintf(&cmd, "AT+CDV=%s,%d,%d,%d,%s", addrNoPlusCode, isEmergency,
                p_reDial->mode, p_reDial->callId, origNumber);
    }

    ret = at_send_command(cmd, &p_response, CC_CHANNEL_CTX);
    free(cmd);

    if (ret < 0 || p_response == NULL || p_response->success == 0) {
        goto error;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    at_response_free(p_response);
    return;

error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

/**
 * To notify call information to RIL proxy for ECC redial.
 *
 */
void notifyCallInformationForEccRedial(const char *s, int msgType, int cause) {
    char *callInfo[10] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
    char *dup = strdup(s);
    char *line = NULL;
    int callId, err;

    /* Retrieve call id */
    if (!dup) {
        return;
    }
    line = dup;
    err = at_tok_start(&line);
    if (err < 0) {
        free(dup);
        return;
    }
    err = at_tok_nextint(&line, &callId);
    if (err < 0) {
        free(dup);
        return;
    }

    /* Call id */
    if ((callId >= 0) && (callId < 10)) {
        callInfo[0] = calloc(1, 2);
        assert(callInfo[0] != NULL);
        sprintf(callInfo[0], "%d", callId);
    }

    /* Message type */
    callInfo[1] = calloc(1, 10);
    assert(callInfo[1] != NULL);
    sprintf(callInfo[1], "%d", msgType);

    /* Cause */
    callInfo[9] = calloc(1, 10);
    assert(callInfo[9] != NULL);
    sprintf(callInfo[9], "%d", cause);

    RIL_onUnsolicitedResponse(RIL_UNSOL_CALL_INFO_INDICATION,
             callInfo, 10 * sizeof(char*));
    free(dup);
}

// Support EF ECC @{
static void onHandleEfEccUrc(const char *s) {
    int count = 0, i = 0, remain = MAX_EF_ECC_LEN;
    int err;
    char* num_ptr = NULL;
    char* tempStr = NULL;
    char* line = (char*) s;
    char data[MAX_EF_ECC_LEN + 1] = {0};
    int totalLen = strlen(line);

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &count);
    if (err < 0) goto error;

    RLOGD("[C2K] onHandleEfEccUrc, EF_ECC count %d", count);
    snprintf(data, MAX_EF_ECC_LEN, "%d", count);

    // ecc numbers will be appended
    for (i = 0; i < count; i++) {
        // There is at least one ECC number in EF_ECC
        err = at_tok_nextstr(&line, &num_ptr);
        if (err < 0) goto error;
        RLOGD("[C2K] onHandleEfEccUrc, EF_ECC number %s", num_ptr);

        // Prepare the new ECC string for strcat
        asprintf(&tempStr,  ",%s", num_ptr);
        remain = MAX_EF_ECC_LEN - strlen(data);
        if (remain >= (int) strlen(tempStr)) {
            // append it
            strncat(data, tempStr, strlen(tempStr));
        } else {
            free(tempStr);
            break;
        }
        free(tempStr);
        tempStr = NULL;

        num_ptr = NULL;

        // Check parse string end
        if (strlen(data) + strlen("+CECC: ") >= totalLen) {
           break;
        }
    }

    // debug log
    RLOGD("[C2K] onHandleEfEccUrc, EF_ECC %s, %d", data, strlen(data));
    RIL_onUnsolicitedResponse(RIL_LOCAL_C2K_UNSOL_EF_ECC, data, strlen(data));
    return;
error:
    RLOGE("[C2K] Parse the URC of EF_ECC fail: %s/n", s);
}
// @}

void onMccMncIndication(char* mccmnc) {
    // Notify RIL Proxy to update ECC list
    RLOGD("[%s] Send to RIL Proxy mccmnc: %s", __FUNCTION__, mccmnc);
    RIL_onUnsolicitedResponse(RIL_UNSOL_EMERGENCY_NUMBER_LIST, mccmnc, strlen(mccmnc));
}
