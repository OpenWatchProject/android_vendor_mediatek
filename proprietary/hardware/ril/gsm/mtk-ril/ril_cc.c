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

#include <telephony/mtk_ril.h>
#include <libmtkrilutils.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <alloca.h>
#include "atchannels.h"
#include "at_tok.h"
#include "misc.h"
#include <getopt.h>
#include <sys/socket.h>
#include <cutils/sockets.h>
#include <termios.h>

#include <ril_callbacks.h>

#ifdef LOG_TAG
    #undef LOG_TAG
#endif
#define LOG_TAG "RIL-CC"

#include <log/log.h>
#include <cutils/properties.h>
#include <hardware_legacy/power.h>
#include "ril_ims_cc.h"
#include "ratconfig.h"

#ifndef UNUSED
#define UNUSED(x) (x)   // Eliminate "warning: unused parameter"
#endif

/* MTK proprietary start */
#define CC_CHANNEL_CTX getRILChannelCtxFromToken(t)

#define CRSS_CALL_WAITING             0
#define CRSS_CALLED_LINE_ID_PREST     1
#define CRSS_CALLING_LINE_ID_PREST    2
#define CRSS_CONNECTED_LINE_ID_PREST  3

#define MAX_EF_ECC_LEN   255

// For AT&T ECC
static const struct timeval TIMEVAL_0 = {0,0};

char *setupCpiData[9] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
int callWaiting = 0;
int isAlertSet = 0;
int hasReceivedRing = 0;
int inCallNumber = 0;
int inCallNumberPerSim[MAX_SIM_COUNT] = {0};
// BEGIN mtk03923 [20120210][ALPS00114093]
int inDTMF = 0;
// END mtk03923 [20120210][ALPS00114093]
bool isRecvECPI0 = false;
int ringCallID = 0;
RIL_SOCKET_ID ridRecvECPI0 = MAX_SIM_COUNT;
char cachedCnap[MAX_CNAP_LENGTH] = { 0 };
char cachedClccName[MAX_GSMCALL_CONNECTIONS][MAX_CNAP_LENGTH] = {{0}};
int ecpiValue[MAX_GSMCALL_CONNECTIONS] = {-1, -1, -1, -1, -1, -1, -1}; //MAX_GSMCALL_CONNECTIONS=7

// M: CC: to keep current RIL_UNSOL_SPEECH_CODEC_INFO information.
int mSpeechCodec = 0;

// [ALPS00242104]Invalid number show but cannot call drop when dial VT call in 2G network
// mtk04070, 2012.02.24
int bUseLocalCallFailCause = 0;
int dialLastError = 0;
/* MTK proprietary end */

int mServiceCategory = 0;

void notifyCallInformationForRedial(int callId, int msgType, int cause, RIL_SOCKET_ID rid);
void rejectDial();

/// For DS MO/MT conflict
extern pthread_mutex_t s_ActiveSocketMutex;
extern int activeRilSktID;
extern bool checkAndSetActiveSocket(RIL_SOCKET_ID rid);
extern void resetActiveSocket();

// Support EF ECC @{
static void onHandleEfEccUrc(const char *s, RIL_SOCKET_ID rid);
static char* sEccStr[SIM_COUNT] = {NULL};
// IMS Start @{
extern bool hasImsPendingMO();
extern dispatch_flg dispatch_flag;
extern volte_call_state call_state;
extern void notifyImsCallExist(RIL_SOCKET_ID rid);
// @}

/// M: CC: Switch antenna
static void requestSwitchAntenna(void * data, size_t datalen, RIL_Token t);

// For AT&T ECC
bool bEmergencyMode = false;
void stopEmergencySession(void *param);

static int clccStateToRILState(int state, RIL_CallState *p_state) {
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

/**
 * Note: directly modified line and has *p_call point directly into
 * modified line
 */
static int callFromCLCCLine(char *line, RIL_Call *p_call) {
    // +CLCC: 1,0,2,0,0,\"+18005551212\",145
    // +CLCC: <id>,<dir>,<stat>,<mode>,<mpty>,<number>,<toa>,,,<CLI validity>
    // Optional parameters <alpha> and <priority> not present.

    int err;
    int state;
    int mode;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &(p_call->index));
    if (err < 0) goto error;

    err = at_tok_nextbool(&line, &(p_call->isMT));
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &state);
    if (err < 0) goto error;

    err = clccStateToRILState(state, &(p_call->state));
    if (err < 0) goto error;

    // CNAP URC is sent after RING and before CLCC polling,
    // therefore we create a cached array to store the name of MT call, with the cached CNAP,
    // so that the name in subsequent CLCC polling can be filled with the cached CNAP array,
    // even the call becomes ACTIVE later.
    if (p_call->state == RIL_CALL_INCOMING || p_call->state == RIL_CALL_WAITING) {
        if ((p_call->index >= 1) && (p_call->index <= MAX_GSMCALL_CONNECTIONS)) {
            LOGD("ecpiValue[%d] = %d", p_call->index-1, ecpiValue[p_call->index-1]);
            if ((ecpiValue[p_call->index-1] != 0) && (ecpiValue[p_call->index-1] != 4)) {
                LOGD("Skip MT call if ECPI0 or ECPI4 is not received\n");
                goto error;
            }

            if (strlen(cachedCnap) > 0) {
                memset(cachedClccName[p_call->index-1], 0, MAX_CNAP_LENGTH);
                strncpy(cachedClccName[p_call->index-1], cachedCnap, strlen(cachedCnap));
                memset(cachedCnap, 0, MAX_CNAP_LENGTH);
            }
        }
    }

    // Fill CNAP in individual CLCC parsing and clear in ECPI:133
    if ((p_call->index >= 1) && (p_call->index <= MAX_GSMCALL_CONNECTIONS)) {
        if (strlen(cachedClccName[p_call->index-1]) > 0) {
            strncpy(p_call->name, cachedClccName[p_call->index-1],
                    strlen(cachedClccName[p_call->index-1]));
        }
    }

    err = at_tok_nextint(&line, &mode);
    if (err < 0) goto error;

    p_call->isVoice = (mode == 0);

    if (mode >= 20) {
        LOGD("Skip call with mode>=20, i.e. IMS/WFC call\n");
        goto error;
    }

    err = at_tok_nextbool(&line, &(p_call->isMpty));
    if (err < 0) goto error;

    if (at_tok_hasmore(&line)) {
        err = at_tok_nextstr(&line, &(p_call->number));

        /* tolerate null here */
        if (err < 0) return 0;

        // Some lame implementations return strings
        // like "NOT AVAILABLE" in the CLCC line
        if ((p_call->number != NULL) &&
            (strspn(p_call->number, "+*#pw0123456789") != strlen(p_call->number))) {
            p_call->number = NULL;
        }

        err = at_tok_nextint(&line, &p_call->toa);
        if (err < 0) goto error;
    }

    if (at_tok_hasmore(&line)) {
        int dummy;
        err = at_tok_nextint(&line, &dummy);  // <alpha>
        err = at_tok_nextint(&line, &dummy);  // <priority>
        err = at_tok_nextint(&line, &(p_call->numberPresentation));
        if (err < 0) goto error;

        /* Issue: ALPS02866394
           Google AOSP only supports 0~3, so re-map 4 or later value to 2(UNKNOWN)
         */
        if (p_call->numberPresentation > 3) {
            p_call->numberPresentation = 2;
        }
    }

    p_call->uusInfo = NULL;
    p_call->speechCodec = mSpeechCodec;

    return 0;

error:
    LOGE("invalid CLCC line\n");
    return -1;
}

void clearCnap() {
    int i;
    memset(cachedCnap, 0, MAX_CNAP_LENGTH);
    for (i = 0; i < MAX_GSMCALL_CONNECTIONS; i++) {
        memset(cachedClccName[i], 0, MAX_CNAP_LENGTH);
    }
}

extern void requestGetCurrentCalls(void *data, size_t datalen, RIL_Token t) {
    int err = -1;
    ATResponse *p_response = NULL;
    ATLine *p_cur;
    int countCalls;
    int countValidCalls;
    RIL_Call *p_calls;
    RIL_Call **pp_calls;
    int i;

    UNUSED(data);
    UNUSED(datalen);

    err = at_send_command_multiline("AT+CLCC", "+CLCC:", &p_response, CC_CHANNEL_CTX);

    if (err < 0 || p_response == NULL || p_response->success == 0) {
        goto error;
    }

    /* count the calls */
    for (countCalls = 0, p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next)
        countCalls++;

    /* yes, there's an array of pointers and then an array of structures */

    pp_calls = (RIL_Call **)alloca(countCalls * sizeof(RIL_Call *));
    if (pp_calls == NULL) {
        LOGE("Failed to allocate memory for AT+CLCC handling!! (pp_calls)");
        goto error;
    }
    p_calls = (RIL_Call *)alloca(countCalls * sizeof(RIL_Call));
    if (p_calls == NULL) {
        LOGE("Failed to allocate memory for AT+CLCC handling!! (p_calls)");
        goto error;
    }
    memset(p_calls, 0, countCalls * sizeof(RIL_Call));

    if (countCalls == 0) {
        mSpeechCodec = 0; // M: CC: reset speech codec information when call count is zero.
    }
    /* init the pointer array */
    for (i = 0; i < countCalls; i++)
        pp_calls[i] = &(p_calls[i]);

    for (countValidCalls = 0, p_cur = p_response->p_intermediates
         ; p_cur != NULL
         ; p_cur = p_cur->p_next
         ) {
        p_calls[countValidCalls].name = (char *)alloca(MAX_CNAP_LENGTH);
        if (p_calls[countValidCalls].name == NULL) {
            LOGE("Failed to allocate memory for AT+CLCC handling!! (p_calls[%d].name)",
                    countValidCalls);
            goto error;
        }
        memset(p_calls[countValidCalls].name, 0, MAX_CNAP_LENGTH);
        err = callFromCLCCLine(p_cur->line, p_calls + countValidCalls);
        if (err != 0)
            continue;

        countValidCalls++;
    }

    if (countValidCalls == 0) {
        clearCnap();
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, pp_calls, countValidCalls * sizeof(RIL_Call *));

    at_response_free(p_response);
    return;

error:
    clearCnap();
    RIL_onRequestComplete(t, RIL_E_INTERNAL_ERR, NULL, 0);
    at_response_free(p_response);
}

void requestDial(void *data, size_t datalen, RIL_Token t, int isEmergency, int isVt) {
    RIL_Dial *p_dial;
    char *cmd = NULL;
    const char *clir;
    int ret;
    ATResponse *p_response = NULL;

    UNUSED(datalen);

    // get rid
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    LOGD("[requestDial] isEmergency = %d, isVt = %d", isEmergency, isVt);

    if (checkAndSetActiveSocket(rid)) {
        rejectDial();
        goto error;
    }

    /*mtk00924: ATDxxxI can not used for FDN check, therefore, change to #31# or *31#*/

    p_dial = (RIL_Dial *)data;

    switch (p_dial->clir) {
    case 1: /*invocation*/
        clir = "#31#";
        break;
    case 2: /*suppression*/
        clir = "*31#";
        break;
    case 0:
    default: /*subscription default*/
        clir = "";
        break;
    }

    // IMS VoLTE refactoring
    if (dispatch_flag == IMS) {
        LOGE("IMS: requestDial from IMS !!");
        ret = at_send_command("AT+EVADSREP=1", &p_response, CC_CHANNEL_CTX);
        ret = at_send_command("AT+EVADSMOD=0", &p_response, CC_CHANNEL_CTX);
        if (ret < 0 || p_response == NULL || p_response->success == 0) {
            LOGE("IMS: AT+EVADSMOD=0 Fail !!");
        }
    } else {
        LOGE("IMS: AT+EVADSMOD=1");
        ret = at_send_command("AT+EVADSMOD=1", &p_response, CC_CHANNEL_CTX);
        if (ret < 0 || p_response == NULL || p_response->success == 0) {
            LOGE("IMS: AT+EVADSMOD=1 Fail !!");
        }
        dispatch_flag = GSM;
    }
    notifyImsCallExist(rid);
    at_response_free(p_response);
    p_response = NULL;

    if (isEmergency) {
        /* If an incoming call exists(+CRING is not received yet), hang it up before dial ECC */
        if ((setupCpiData[0] != NULL) && isRecvECPI0) {
            LOGD("To hang up incoming call(+CRING is not received yet) before dialing ECC!");

            asprintf(&cmd, "AT+CHLD=1%s", setupCpiData[0]);

            isRecvECPI0 = false;
            int i;
            for (i = 0; i < 9; i++) {
                free(setupCpiData[i]);
                setupCpiData[i] = NULL;
            }

            at_send_command(cmd, NULL, CC_CHANNEL_CTX);
            free(cmd);

            sleep(1);
        }

        char *formatNumber = NULL;
        char *tmp = strchr(p_dial->address, ',');
        if (tmp != NULL) {
            int pos = (tmp - p_dial->address) / sizeof(char);
            formatNumber = (char *)alloca(pos + 1);
            if (formatNumber == NULL) {
                LOGE("Failed to allocate memory for ATDE handling!! (formatNumber)");
                goto error;
            }
            memset(formatNumber, 0, pos + 1);
            strncpy(formatNumber, p_dial->address, pos);
        } else {
            formatNumber = p_dial->address;
        }

        asprintf(&cmd, "ATDE%s%s;", clir, formatNumber);
    } else {
        // BEGIN mtk03923 [20111004][ALPS00077405]
        // CC operation will fail when dialing number exceed 40 character due to modem capability limitation.
        int addrLen = strlen(p_dial->address);
        if (addrLen > 40) {
            LOGD("[Error]phone number exceeds 40 character\n");

            RIL_onRequestComplete(t, RIL_E_CANCELLED, NULL, 0);

            // [ALPS00251057][Call]It didn't pop FDN dialog when dial an invalid number
            // But this is not related to FDN issue, it returned to AP since number is too long.
            // mtk04070, 2012.03.12
            bUseLocalCallFailCause = 1;
            dialLastError = CALL_FAIL_INVALID_NUMBER_FORMAT;
            /* Refer to CallFailCause.java - INVALID_NUMBER_FORMAT */

            return;
        }
        // END mtk03923 [20111004][ALPS00077405]

        if (isVt) {
            if (inCallNumberPerSim[rid] && dispatch_flag != IMS) {
                LOGE("Reject dial, not allow MO VT if a call already exists");

                bUseLocalCallFailCause = 1;
                dialLastError = CALL_FAIL_ERROR_UNSPECIFIED;

                goto error;
            }
            asprintf(&cmd, "ATD%s%s", clir, p_dial->address);
        } else {
            asprintf(&cmd, "ATD%s%s;", clir, p_dial->address);
        }
    }
    ret = at_send_command(cmd, &p_response, CC_CHANNEL_CTX);

    // [ALPS00242104]Invalid number show but cannot call drop when dial VT call in 2G network
    // mtk04070, 2012.02.24
    bUseLocalCallFailCause = 0;
    dialLastError = 0;

    free(cmd);

    if (ret < 0 || p_response == NULL || p_response->success == 0) {
        goto error;
    }
    /* success or failure is ignored by the upper layer here.
     * it will call GET_CURRENT_CALLS and determine success that way */
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    at_response_free(p_response);
    return;

error:
    if (inCallNumber == 0) {
        resetActiveSocket();
    }
    if (call_state == pending_mo) {
        LOGE("IMS: requestDial fail !!");
        call_state = none;
        if (inCallNumber == 0) {
            dispatch_flag = GSM;
            notifyImsCallExist(rid);
        }
    }

    /// M: CC: Special VTS handling for normal call ATD when no SIM (ALPS03598187)
    ///        Call fail cause will be CM_CALL_REJECTED (21)
    if (at_get_cme_error(p_response) == CME_OPERATION_NOT_ALLOWED_ERR) {
        RIL_onRequestComplete(t, RIL_E_OPERATION_NOT_ALLOWED, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    }

    // For AT&T ECC
    // Not allow to send EMCS for OP07 if C2K supported
    if (isOp07Support() && !RatConfig_isC2kSupported()) {
        if (bEmergencyMode == true) {
            TimedCallbackParam *param = (TimedCallbackParam *)malloc(sizeof(TimedCallbackParam));
            if (param != NULL) {
                param->rid = rid;
                RIL_requestProxyTimedCallback(stopEmergencySession, param, &TIMEVAL_0,
                        getRILChannelId(RIL_CC, rid), "stopEmergencySession");
            } else {
                LOGE("requestDial param malloc failed.");
            }
            bEmergencyMode = false;
        }
    }

    if (isRedialFeatureEnabled() && !isImsSupport()) {
        notifyCallInformationForRedial(0, 133, CALL_FAIL_ERROR_UNSPECIFIED, rid);
    }

    at_response_free(p_response);
}

void requestHangup(void *data, size_t datalen, RIL_Token t) {
    int *p_line = NULL;
    char *cmd = NULL;
    int ret = -1;
    ATResponse *p_response = NULL;

    UNUSED(datalen);

    p_line = (int *)data;

    // 3GPP 22.030 6.5.5
    // "Releases a specific active call X"
    asprintf(&cmd, "AT+CHLD=1%d", p_line[0]);
    ret = at_send_command(cmd, &p_response, CC_CHANNEL_CTX);
    free(cmd);
    if (ret < 0 || p_response == NULL || p_response->success == 0) {
        RLOGE("requestHangup CHLD fail");
        goto error;
    }

    /* success or failure is ignored by the upper layer here.
     * it will call GET_CURRENT_CALLS and determine success that way */
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    at_response_free(p_response);
    return;

error:
    RIL_onRequestComplete(t, RIL_E_INTERNAL_ERR, NULL, 0);
    at_response_free(p_response);
}

void requestHangupWaitingOrBackground(void *data, size_t datalen, RIL_Token t) {
    int ret = -1;
    ATResponse *p_response = NULL;

    UNUSED(data);
    UNUSED(datalen);

    // 3GPP 22.030 6.5.5
    // "Releases all held calls or sets User Determined User Busy
    //  (UDUB) for a waiting call."
    ret = at_send_command("AT+CHLD=0", &p_response, CC_CHANNEL_CTX);
    if (ret < 0 || p_response == NULL || p_response->success == 0) {
        RLOGE("requestHangupWaitingOrBackground CHLD fail");
        goto error;
    }

    /* success or failure is ignored by the upper layer here.
     * it will call GET_CURRENT_CALLS and determine success that way */
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    at_response_free(p_response);

    return;

error:
    RIL_onRequestComplete(t, RIL_E_INTERNAL_ERR, NULL, 0);
    at_response_free(p_response);
}

void requestHangupForegroundResumeBackground(void *data, size_t datalen, RIL_Token t) {

    int err = -1;
    ATResponse *p_response = NULL;

    UNUSED(data);
    UNUSED(datalen);

    // 3GPP 22.030 6.5.5
    // "Releases all active calls (if any exist) and accepts
    //  the other (held or waiting) call."
    // at_send_command("AT+CHLD=1", NULL, CC_CHANNEL_CTX);
    err = at_send_command_multiline("AT+CHLD=1", "NO CARRIER", &p_response, CC_CHANNEL_CTX);
    if (err < 0 || p_response == NULL || p_response->success == 0) {
        RLOGE("requestHangupForegroundResumeBackground CHLD fail");
        goto error;
    }

    /* success or failure is ignored by the upper layer here.
     * it will call GET_CURRENT_CALLS and determine success that way */
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    at_response_free(p_response);
    return;

error:
    RIL_onRequestComplete(t, RIL_E_INTERNAL_ERR, NULL, 0);
    at_response_free(p_response);
}

void requestSwitchWaitingOrHoldingAndActive(void *data, size_t datalen, RIL_Token t) {
    int ret = -1;
    ATResponse *p_response = NULL;

    UNUSED(data);
    UNUSED(datalen);

        // BEGIN mtk03923 [20120210][ALPS00114093]
        if (inDTMF) {
        RIL_onRequestComplete(t, RIL_E_CANCELLED, NULL, 0);     // RIL_E_GENERIC_FAILURE
            return;
        }
        // END mtk03923 [20120210][ALPS00114093]


    ret = at_send_command("AT+CHLD=2", &p_response, CC_CHANNEL_CTX);

    if (ret < 0 || p_response == NULL || p_response->success == 0) {
        goto error;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    at_response_free(p_response);
    return;

error:
    RIL_onRequestComplete(t, RIL_E_INTERNAL_ERR, NULL, 0);
    at_response_free(p_response);
}

void requestHangupWithReason(void *data, size_t datalen, RIL_Token t) {
    int *p_line = NULL;
    char *cmd = NULL;
    int ret = -1;
    ATResponse *p_response = NULL;

    UNUSED(datalen);

    p_line = (int *)data;

    if (dispatch_flag == GSM) {
        // Not support AT+CHLD=8
        asprintf(&cmd, "AT+CHLD=1%d", p_line[0]);
    } else {
        asprintf(&cmd, "AT+ECHLD=1%d,%d", p_line[0], p_line[1]);
    }
    ret = at_send_command(cmd, &p_response, CC_CHANNEL_CTX);
    free(cmd);
    if (ret < 0 || p_response == NULL || p_response->success == 0) {
        RLOGE("requestHangupWithReason ECHLD fail");
        goto error;
    }

    /* success or failure is ignored by the upper layer here.
     * it will call GET_CURRENT_CALLS and determine success that way */
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    at_response_free(p_response);
    return;

error:
    RIL_onRequestComplete(t, RIL_E_INTERNAL_ERR, NULL, 0);
    at_response_free(p_response);
}

void requestAnswer(void *data, size_t datalen, RIL_Token t) {
    int ret = -1;
    ATResponse *p_response = NULL;

    UNUSED(data);
    UNUSED(datalen);

    ret = at_send_command("ATA", &p_response, CC_CHANNEL_CTX);

    if (ret < 0 || p_response == NULL || p_response->success == 0) {
        goto error;
    }

    /* success or failure is ignored by the upper layer here.
     * it will call GET_CURRENT_CALLS and determine success that way */
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    at_response_free(p_response);
    return;

error:
    RIL_onRequestComplete(t, RIL_E_INTERNAL_ERR, NULL, 0);
    at_response_free(p_response);
}

void requestConference(void *data, size_t datalen, RIL_Token t) {
    int ret = -1;
    ATResponse *p_response = NULL;

    UNUSED(data);
    UNUSED(datalen);

    // BEGIN mtk03923 [20120210][ALPS00114093]
    if (inDTMF) {
        RIL_onRequestComplete(t, RIL_E_CANCELLED, NULL, 0);     // RIL_E_GENERIC_FAILURE
        return;
    }
    // END mtk03923 [20120210][ALPS00114093]

    ret = at_send_command("AT+CHLD=3", &p_response, CC_CHANNEL_CTX);

    if (ret < 0 || p_response == NULL || p_response->success == 0) {
        goto error;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    at_response_free(p_response);
    return;

error:
    RIL_onRequestComplete(t, RIL_E_INTERNAL_ERR, NULL, 0);
    at_response_free(p_response);
}

void requestUdub(void *data, size_t datalen, RIL_Token t) {
    int ret = -1;
    ATResponse *p_response = NULL;

    // get rid
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));

    UNUSED(data);
    UNUSED(datalen);

    /* user determined user busy */
    /* sometimes used: ATH */
    ret = at_send_command("ATH", &p_response, CC_CHANNEL_CTX);
    if (ret < 0 || p_response == NULL || p_response->success == 0) {
        RLOGE("requestUdub fail (rid: %d)", rid);
        goto error;
    }
    if (!RatConfig_isC2kSupported() && inCallNumberPerSim[rid] == 0 && isSimInserted(rid) == 0) {
        // special handling for VTS UDUB response. (ALPS03615752)
        // RILD modify only applied to non-c2k projects. (c2k projects handled by rilproxy)
        RLOGI("no SIM inserted, update UDUB response error code (rid:%d, callCount:%d)",
                rid, inCallNumberPerSim[rid]);
        RIL_onRequestComplete(t, RIL_E_INVALID_STATE, NULL, 0);
        at_response_free(p_response);
    } else {
        /* success or failure is ignored by the upper layer here.
         * it will call GET_CURRENT_CALLS and determine success that way */
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        at_response_free(p_response);
    }
    return;

error:
    RIL_onRequestComplete(t, RIL_E_INTERNAL_ERR, NULL, 0);
    at_response_free(p_response);
}

void requestSeparateConnection(void *data, size_t datalen, RIL_Token t) {
    char cmd[12];
    int party = ((int *)data)[0];
    int ret;
    ATResponse *p_response = NULL;

    UNUSED(datalen);

        // BEGIN mtk03923 [20120210][ALPS00114093]
        if (inDTMF) {
        RIL_onRequestComplete(t, RIL_E_CANCELLED, NULL, 0);     // RIL_E_GENERIC_FAILURE
            return;
        }
        // END mtk03923 [20120210][ALPS00114093]


    // Make sure that party is in a valid range.
    // (Note: The Telephony middle layer imposes a range of 1 to 7.
    // It's sufficient for us to just make sure it's single digit.)
    if (party > 0 && party < 10) {
        sprintf(cmd, "AT+CHLD=2%d", party);
        ret = at_send_command(cmd, &p_response, CC_CHANNEL_CTX);

        if (ret < 0 || p_response == NULL || p_response->success == 0) {
            goto error;
        }

        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        at_response_free(p_response);
        return;
    }

error:
    RIL_onRequestComplete(t, RIL_E_INTERNAL_ERR, NULL, 0);
    at_response_free(p_response);
}

void requestExplicitCallTransfer(void *data, size_t datalen, RIL_Token t) {
    /* MTK proprietary start */
    int ret = -1;
    ATResponse *p_response = NULL;

    UNUSED(data);
    UNUSED(datalen);

    // BEGIN mtk03923 [20120210][ALPS00114093]
    if (inDTMF) {
        RIL_onRequestComplete(t, RIL_E_CANCELLED, NULL, 0);     // RIL_E_GENERIC_FAILURE
        return;
    }
    // END  mtk03923 [20120210][ALPS00114093]


    ret = at_send_command("AT+CHLD=4", &p_response, CC_CHANNEL_CTX);

    if (ret < 0 || p_response == NULL || p_response->success == 0) {
        goto error;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    at_response_free(p_response);
    return;

error:
    RIL_onRequestComplete(t, RIL_E_INTERNAL_ERR, NULL, 0);
    at_response_free(p_response);
    /* MTK proprietary end */
}

void requestLastCallFailCause(void *data, size_t datalen, RIL_Token t) {
    RIL_LastCallFailCauseInfo callFailCause;
    char *line = NULL;
    int ret;
    ATResponse *p_response = NULL;

    UNUSED(data);
    UNUSED(datalen);

    memset(&callFailCause, 0, sizeof(RIL_LastCallFailCauseInfo));

    // [ALPS00242104]Invalid number show but cannot call drop when dial VT call in 2G network
    // mtk04070, 2012.02.24
    if (bUseLocalCallFailCause == 1) {
       callFailCause.cause_code = dialLastError;
       LOGD("Use local call fail cause = %d", callFailCause.cause_code);
    }
    else {
        ret = at_send_command_singleline("AT+CEER", "+CEER:", &p_response, CC_CHANNEL_CTX);

        if (ret < 0 || p_response == NULL || p_response->success == 0) {
           goto error;
        }

       line = p_response->p_intermediates->line;

       ret = at_tok_start(&line);

       if (ret < 0)
           goto error;

       ret = at_tok_nextint(&line, (int *)&(callFailCause.cause_code));
       if (ret < 0)
           goto error;

       ret = at_tok_nextstr(&line, &(callFailCause.vendor_cause));
       if (ret < 0)
           goto error;

       LOGD("MD fail cause_code = %d, vendor_cause = %s",
               callFailCause.cause_code, callFailCause.vendor_cause);
    }

    /*if there are more causes need to be translated in the future,
     * discussing with APP owner to implement this in upper layer.
     * For the hard coded value, please refer to modem code.*/

    if (callFailCause.cause_code == 10)
        callFailCause.cause_code = CALL_FAIL_CALL_BARRED;
    else if (callFailCause.cause_code == 2600)
        callFailCause.cause_code = CALL_FAIL_FDN_BLOCKED;
    else if (callFailCause.cause_code == 2052)
        callFailCause.cause_code = CALL_FAIL_IMSI_UNKNOWN_IN_VLR;
    else if (callFailCause.cause_code == 2053)
        callFailCause.cause_code = CALL_FAIL_IMEI_NOT_ACCEPTED;
    else if ((callFailCause.cause_code <= 0) ||
           (callFailCause.cause_code > 127
           && callFailCause.cause_code != 2165
           && callFailCause.cause_code != 380
           && callFailCause.cause_code != 999))
        callFailCause.cause_code = CALL_FAIL_ERROR_UNSPECIFIED;
    else if (callFailCause.cause_code == 380)
        mServiceCategory = atoi(callFailCause.vendor_cause);

    LOGD("RIL fail cause_code = %d, vendor_cause = %s",
            callFailCause.cause_code, callFailCause.vendor_cause);

    RIL_onRequestComplete(t, RIL_E_SUCCESS, &callFailCause, sizeof(RIL_LastCallFailCauseInfo));
    if (NULL != p_response) {
        at_response_free(p_response);
    }
    return;

error:
    RIL_onRequestComplete(t, RIL_E_INTERNAL_ERR, NULL, 0);
    at_response_free(p_response);
}

void requestDtmf(void *data, size_t datalen, RIL_Token t) {
    char c = ((char *)data)[0];
    char *cmd = NULL;

    UNUSED(datalen);

    asprintf(&cmd, "AT+VTS=%c", (int)c);
    at_send_command(cmd, NULL, CC_CHANNEL_CTX);

    free(cmd);

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
}

static void onCnapNotification(char *s, RIL_SOCKET_ID rid) {
    char* p_data[2] = {"", ""};
    char* line = s;

    UNUSED(rid);

    /**
     * CNAP presentaion from the network
     * +CNAP: <name>[,<CNI validity>] to the TE.
     *
     *   <name> : GSM 7bit encode
     *
     *   <CNI validity>: integer type
     *   0 CNI valid
     *   1 CNI has been withheld by the originator.
     *   2 CNI is not available due to interworking problems or limitations of originating network.
     */

    if (at_tok_start(&line) < 0) {
        goto error;
    }

    /* Get <name> */
    if (at_tok_nextstr(&line, &p_data[0]) < 0) {
       goto error;
    }

    /* Get <CNI validity> */
    if (at_tok_nextstr(&line, &p_data[1]) < 0) {
        goto error;
    }

    snprintf(cachedCnap, MAX_CNAP_LENGTH, "%s", p_data[0]);

    return;

error:
    LOGE("There is something wrong with the +CNAP");
}

/**
 * RIL_UNSOL_CRSS_NOTIFICATION
 *
 * Reports supplementary service related notification from the network.
 *
 * "data" is a const RIL_CrssNotification *
 *
 */
static void onCrssNotification(char *s, int code, RIL_SOCKET_ID rid)
{
    RIL_CrssNotification crssNotify;
    char* line = s;
    char* pStrTmp = NULL;
    int  toa = 0;

    memset(&crssNotify, 0, sizeof(RIL_CrssNotification));
    crssNotify.code = code;

    if (at_tok_start(&line) < 0) {
        goto error;
    }

    /* Get <number> */
    if (at_tok_nextstr(&line, &(crssNotify.number)) < 0) {
        LOGE("CRSS: number fail!");
        goto error;
    }

    /* Get <type> */
    if (at_tok_nextint(&line, &(crssNotify.type)) < 0) {
        LOGE("CRSS: type fail!");
        goto error;
    }

    if (at_tok_hasmore(&line)) {
        /*skip subaddr*/
        if (at_tok_nextstr(&line, &(pStrTmp)) < 0) {
            LOGE("CRSS: sub fail!");
        }

        /*skip satype*/
        if (at_tok_nextint(&line, &(toa)) < 0) {
            LOGE("CRSS: sa type fail!");
        }

        if (at_tok_hasmore(&line)) {
            /* Get alphaid */
            if (at_tok_nextstr(&line, &(crssNotify.alphaid)) < 0) {
                LOGE("CRSS: alphaid fail!");
            }

            /* Get cli_validity */
            if (at_tok_nextint(&line, &(crssNotify.cli_validity)) < 0) {
                LOGE("CRSS: cli_validity fail!");
            }
            /* Issue: ALPS02866394
             Google AOSP only supports 0~3, so re-map 4 or later value to 2(UNKNOWN)
             */
            if (crssNotify.cli_validity > 3) {
                crssNotify.cli_validity = 2;
            }
            LOGD("crssNotify.cli_validity = %d", crssNotify.cli_validity);
        }
    }

    RIL_UNSOL_RESPONSE (
            RIL_UNSOL_CRSS_NOTIFICATION,
            &crssNotify, sizeof(RIL_CrssNotification),
            rid);

    return;

error:
    LOGE("error on onCrssNotification");
}

extern int rilCcMain(int request, void *data, size_t datalen, RIL_Token t) {
    switch (request) {
    case RIL_REQUEST_GET_CURRENT_CALLS:
        requestGetCurrentCalls(data, datalen, t);
        break;
    case RIL_REQUEST_DIAL:
        requestDial(data, datalen, t, 0, 0);
        break;
    case RIL_REQUEST_HANGUP:
        requestHangup(data, datalen, t);
        break;
    case RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND:
        requestHangupWaitingOrBackground(data, datalen, t);
        break;
    case RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND:
        requestHangupForegroundResumeBackground(data, datalen, t);
        break;
    case RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE:
        requestSwitchWaitingOrHoldingAndActive(data, datalen, t);
        break;
    case RIL_REQUEST_ANSWER:
        requestAnswer(data, datalen, t);
        break;
    case RIL_REQUEST_CONFERENCE:
        requestConference(data, datalen, t);
        break;
    case RIL_REQUEST_UDUB:
        requestUdub(data, datalen, t);
        break;
    case RIL_REQUEST_SEPARATE_CONNECTION:
        requestSeparateConnection(data, datalen, t);
        break;
    case RIL_REQUEST_EXPLICIT_CALL_TRANSFER:
        requestExplicitCallTransfer(data, datalen, t);
        break;
    case RIL_REQUEST_LAST_CALL_FAIL_CAUSE:
        requestLastCallFailCause(data, datalen, t);
        break;
    case RIL_REQUEST_DTMF:
        requestDtmf(data, datalen, t);
        break;
    case RIL_REQUEST_DTMF_START:
        requestDtmfStart(data, datalen, t);
        break;
    case RIL_REQUEST_DTMF_STOP:
        requestDtmfStop(data, datalen, t);
        break;
    case RIL_REQUEST_SET_TTY_MODE:
        requestSetTTYMode(data, datalen, t);
        break;
    case RIL_REQUEST_QUERY_TTY_MODE:
        requestQueryTTYMode(data, datalen, t);
        break;
    /// M: CC: Proprietary call control hangup all
    case RIL_REQUEST_HANGUP_ALL:
        requestHangupAll(data, datalen, t);
        break;
     case RIL_REQUEST_HANGUP_WITH_REASON:
        requestHangupWithReason(data, datalen, t);
        break;
    /// M: CC: Proprietary incoming call handling @{
    case RIL_REQUEST_SET_CALL_INDICATION:
        requestSetCallIndication(data, datalen, t);
        break;

    case RIL_LOCAL_REQUEST_SET_ECC_SERVICE_CATEGORY:
        requestLocalSetEccServiceCategory(data, datalen, t);
        break;

    case RIL_LOCAL_REQUEST_EMERGENCY_DIAL:
        requestDial(data, datalen, t, 1, 0);
        break;

    /// M: CC: Call control force release call
    case RIL_REQUEST_FORCE_RELEASE_CALL:
        requestForceReleaseCall(data, datalen, t);
        break;
    /// @}

    /// M: CC: Switch antenna @{
    case RIL_LOCAL_REQUEST_SWITCH_ANTENNA:
        requestSwitchAntenna(data, datalen, t);
        break;
    /// @}

    // Verizon E911
    case RIL_REQUEST_SET_ECC_MODE:
        requestSetEccMode(data, datalen, t);
        break;

    /// M: ECBM requirement. @{
    case RIL_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE:
        requestExitEmergencyCallbackMode(data, datalen, t);
        break;
    /// @}

    /// M: CC: Normal/Emergency call redial
    case RIL_LOCAL_REQUEST_EMERGENCY_REDIAL:
        requestRedial(data, datalen, t);
        break;

    case RIL_LOCAL_REQUEST_EMERGENCY_SESSION_BEGIN:
        requestNotifyEmergencySessionStatus(data, datalen, t, 1);
        break;

    case RIL_LOCAL_REQUEST_EMERGENCY_SESSION_END:
        requestNotifyEmergencySessionStatus(data, datalen, t, 0);
        break;

    case RIL_LOCAL_REQUEST_SET_HVOLTE_MODE:
        requestSetHvolteMode(data, datalen, t);
        break;

    // M: Set ECC ist to MD @{
    case RIL_REQUEST_SET_ECC_LIST:
        requestSetEccList(data, datalen, t);
        break;
    /// @}
    default:
        return 0; /* no matched request */
        break;
    }

    return 1; /* request found and handled */
}

extern int rilCcUnsolicited(const char *s, const char *sms_pdu, RILChannelCtx *p_channel) {
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);

    UNUSED(sms_pdu);

    /* MTK proprietary start */
    if (strStartsWith(s, "RING") || strStartsWith(s, "+CRING")) {
        LOGD("receiving RING!!!!!!");
        // Notify RIL SS that CRING is coming
        onCRINGReceived();

        if (!hasReceivedRing) {
            LOGD("receiving first RING!!!!!!");
            hasReceivedRing = 1;
        }

        if (!isRecvECPI0) {
            LOGD("we havn't receive ECPI0, skip this RING!");
            return 1;
        }
        if (!hasReceivedRing) {
            LOGD("receiving first RING!!!!!!");
            hasReceivedRing = 1;
        }

        if (setupCpiData[0] != NULL) {
            LOGD("sending STATE CHANGE dispatch_flag = %d ", dispatch_flag);

            // IMS VoLTE refactoring
            if (dispatch_flag != IMS) {
                RIL_UNSOL_RESPONSE(
                        RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED,
                        NULL, 0, rid);
            } else {
                LOGD("IMS: sending ECPI : 0 When RING");
                RIL_UNSOL_RESPONSE(RIL_UNSOL_CALL_INFO_INDICATION,
                        setupCpiData, 9 * sizeof(char *), rid);
            }

            int i;
            for (i = 0; i < 9; i++) {
                free(setupCpiData[i]);
                setupCpiData[i] = NULL;
            }
            sleep(1);
        }

        RIL_onUnsolicitedResponse(RIL_UNSOL_CALL_RING, NULL, 0, rid);
        return 1;
    } else if (strStartsWith(s,"+CCWA:")) {
        callWaiting = 1;
        onCrssNotification((char *) s, CRSS_CALL_WAITING, rid);
        return 1;
    } else if (strStartsWith(s,"+CDIP:")) {
        onCrssNotification((char *) s, CRSS_CALLED_LINE_ID_PREST, rid);
        return 1;
    } else if (strStartsWith(s,"+CLIP:")) {
        onCrssNotification((char *) s, CRSS_CALLING_LINE_ID_PREST, rid);
        return 1;
    } else if (strStartsWith(s,"+COLP:")) {
        onCrssNotification((char *) s, CRSS_CONNECTED_LINE_ID_PREST, rid);
        return 1;
    } else if (strStartsWith(s, "+CNAP:")) {
        onCnapNotification((char *)s, rid);
        return 1;
    } else if (strStartsWith(s, "+ECPI:")) {
        onCallProgressInfoCallStateChange((char *)s, rid);
        return 1;
    } else if (strStartsWith(s, "+ESPEECH")) {
        onSpeechInfo((char *)s, rid);
        return 1;
    } else if (strStartsWith(s, "+EAIC")) {
        onIncomingCallIndication((char *)s, rid);
        return 1;
    } else if (strStartsWith(s, "+ECIPH")) {
        onCipherIndication((char *)s, rid);
        return 1;
    } else if (strStartsWith(s, "+CEN1") || strStartsWith(s, "+CEN2")) {
        onEmergencyCallServiceCategoryIndication((char *)s, rid);
        return 1;
    } else if (strStartsWith(s, "+EVOCD")) {
        onSpeechCodecInfo((char*)s, rid);
        return 1;
    } else if (strStartsWith(s, "+CIEV: 102")) {
        onEmergencyCallBackMode((char*)s, rid);
        return 1;
    } else if (strStartsWith(s,"+CSSI:")) {
        /* +CSSI is MO */
        onSuppSvcNotification((char *)s, 0, rid);
        return 1;
    } else if (strStartsWith(s,"+CSSU:")) {
        /* +CSSU is MT */
        onSuppSvcNotification((char *)s, 1, rid);
        return 1;
    } else if (strStartsWith(s, "+ESIMECC:")) {
        onHandleEfEccUrc(s, rid);
        return 1;
    }
    return 0;
    /* MTK proprietary end */
}

void rejectDial() {
    LOGE("Reject dial due to active socket conflict");
    bUseLocalCallFailCause = 1;
    dialLastError = CALL_FAIL_ERROR_UNSPECIFIED;
}


/**
 * To notify call information to RIL proxy for ECC/Hvolte Redial.
 *
 */
void notifyCallInformationForRedial(int callId, int msgType, int cause,
        RIL_SOCKET_ID rid) {
    char *callInfo[10] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
            NULL };

    /* Call id */
    callInfo[0] = calloc(1, 2);
    if (callInfo[0] == NULL) {
        LOGE("notifyCallInformationForRedial, calloc fail");
        return;
    }
    sprintf(callInfo[0], "%d", callId);

    /* Message type */
    callInfo[1] = calloc(1, 10);
    if (callInfo[1] == NULL) {
        LOGE("notifyCallInformationForRedial, calloc fail");
        return;
    }
    sprintf(callInfo[1], "%d", msgType);

    /* Cause */
    callInfo[9] = calloc(1, 10);
    if (callInfo[9] == NULL) {
        LOGE("notifyCallInformationForRedial, calloc fail");
        return;
    }
    sprintf(callInfo[9], "%d", cause);

    RIL_UNSOL_RESPONSE(RIL_UNSOL_CALL_INFO_INDICATION, callInfo,
            10 * sizeof(char *), rid);
}

/// M: CC: Switch antenna @{
static void requestSwitchAntenna(void *data, size_t datalen, RIL_Token t) {
    LOGD("requestSwitchAntenna, callState:%d, ratMode:%d", ((int*) data)[0], ((int*) data)[1]);
    char *cmd = NULL;

    UNUSED(datalen);

    asprintf(&cmd, "AT+ECDOMAIN=%d,%d", ((int*) data)[0], ((int*) data)[1]);
    at_send_command(cmd, NULL, getRILChannelCtxFromToken(t));
    free(cmd);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
}
/// @

// Support EF ECC @{
static void onHandleEfEccUrc(const char *s, RIL_SOCKET_ID rid) {
    int count = 0, i = 0, remain = MAX_EF_ECC_LEN, tempStrLen = 0;
    int service_category = 0;
    int err;
    char* num_ptr = NULL;
    char* tempStr = NULL;
    char* line = (char*)s;
    char data[MAX_EF_ECC_LEN + 1] = {0};
    int totalLen = strlen(line);

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &count);
    if (err < 0) goto error;

    RLOGD("onHandleEfEccUrc[%d], EF_ECC count %d", rid, count);
    snprintf(data, MAX_EF_ECC_LEN, "%d", count);

    // ecc numbers will be appended
    if (sEccStr[rid] != NULL) {
        free(sEccStr[rid]);
        sEccStr[rid] = NULL;
    }

    for (i = 0; i < count; i++) {
        // There is at least one ECC number in EF_ECC
        err = at_tok_nextstr(&line, &num_ptr);
        if (err < 0) goto error;
        err = at_tok_nextint(&line, &service_category);
        if (err < 0) goto error;
        RLOGD("onHandleEfEccUrc[%d], EF_ECC number %s, service category %d", rid, num_ptr,
                service_category);

        // Prepare the new ECC string for strcat
        asprintf(&tempStr,  ",%s,%d", num_ptr, service_category);
        remain = MAX_EF_ECC_LEN - strlen(data);
        tempStrLen = strlen(tempStr);
        if (remain >= tempStrLen) {
            // append it
            strncat(data, tempStr, tempStrLen);
        } else {
            free(tempStr);
            break;
        }
        free(tempStr);
        tempStr = NULL;

        num_ptr = NULL;
        service_category = 0;
        // Check parse string end
        if (strlen(data) + strlen("+ESIMECC: ") >= totalLen) {
           break;
        }
    }

    sEccStr[rid] = calloc(1, sizeof(char)*(strlen(data) + 1));
    if (sEccStr[rid] == NULL) {
        goto error;
    }
    strncpy(sEccStr[rid], data, strlen(data));

    RIL_UNSOL_RESPONSE (
                RIL_LOCAL_GSM_UNSOL_EF_ECC,
                sEccStr[rid], strlen(sEccStr[rid]),
                rid);
    return;
error:
    RLOGE("Parse the URC of EF_ECC fail: %s/n", s);
}

// M: Set ECC ist to MD @{
void requestSetEccList(void *data, size_t datalen, RIL_Token t) {
    const char** strings = (const char**)data;
    ATResponse* p_response = NULL;
    char *cmd = NULL;
    int ret = 0;

    UNUSED(datalen);

    LOGD("requestSetEccList list1: %s, list2: %s", strings[0], strings[1]);

    for (int i = 0; i < 2; i++) {
        if (strings[i] != NULL && strlen(strings[i]) > 0) {
            asprintf(&cmd, "AT+EECCUD=%s", strings[i]);
            ret = at_send_command(cmd, &p_response, CC_CHANNEL_CTX);
            free(cmd);
            if (ret < 0 || p_response == NULL || p_response->success == 0) {
                goto error;
            }
            at_response_free(p_response);
        }
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;

error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}
// @}

static int disallow_mt_call_ID = -1;
static int disallow_mt_call_SeqNo = -1;

extern void rejectDial();
/* Solve [ALPS00846094]general PR: In india,make a call from Mumbai to Delhi using 985
   at callers end, the caller hear a DTMF tone in addition to RBT */
int bEspeechIsTurnedOn = 0;

bool toRefreshNwECC[4] = {false, false, false, false};

/// M: CC: Proprietary incoming call handling @{

typedef struct {
    RIL_SOCKET_ID rid;
    char *callId;
    char *seqNo;
    bool approve;
} IncomingCallParam;

void approveIncomingCall(void *param);
bool isActiveSocketConflict(RIL_SOCKET_ID rid);
bool checkAndSetActiveSocket(RIL_SOCKET_ID rid);
void resetActiveSocket();
/// @}

void requestHangupAll(void *data, size_t datalen, RIL_Token t) {

    UNUSED(data);
    UNUSED(datalen);

    // release all calls
    // For ATH, the channel usd to setup call and release must be the same.
    // So we use this proprietary command to release all calls
    // AT+CHLD=6 means to release all calls without channel src limitation.
    at_send_command("AT+CHLD=6", NULL, CC_CHANNEL_CTX);

    /* success or failure is ignored by the upper layer here.
     * it will call GET_CURRENT_CALLS and determine success that way */
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
}

void requestSetCallIndication(void *data, size_t datalen, RIL_Token t) {
    int *c = (int *)data;
    char *cmd = NULL;
    int ret;
    ATResponse *p_response = NULL;

    UNUSED(datalen);

    asprintf(&cmd, "AT+EAIC=%d,%d,%d", c[0], c[1], c[2]);
    // c[0]:mode, c[1]:callID, c[2]:seqNo
    // -> In this case, it will not have +ECPI: for active call is reported
    // Should not decrease by one of the multi_md_inCallNum
    if (c[0] == 1) {
        // 1: DisAllow incoming call
        disallow_mt_call_ID = c[1];
        disallow_mt_call_SeqNo = c[2];
        LOGD("Disallow incoming call. (set disallow_mt_call_ID=%d, disallow_mt_call_SeqNo=%d)",
                disallow_mt_call_ID, disallow_mt_call_SeqNo);
    }
    LOGD("Set EAIC:mode=%d,callID=%d,seqNo=%d", c[0], c[1], c[2]);
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

/* MTK proprietary start */

/* ==========================================================================
 EAIC case
*/
void onIncomingCallIndication(char *urc, RIL_SOCKET_ID rid) {
    int ret, i;
    char *data[7] = {"", "", "", "", "", "", ""};

    ret = at_tok_start(&urc);
    if (ret < 0) goto error;

    for (i = 0; i < 5; i++) {
        ret = at_tok_nextstr(&urc, &data[i]);
        if (ret < 0) goto error;
    }
    LOGD("onIncomingCallIndication");

    /// Enhance for DSDS behavior, reject SIM2 call while SIM1 active
    if (((atoi(data[3]) < IMS_CALL_MODE_BOUNDARY) && hasImsPendingMO()) ||
            checkAndSetActiveSocket(rid)) {
        IncomingCallParam *param = (IncomingCallParam *)malloc(sizeof(IncomingCallParam));
        if (param == NULL) {
            LOGE("onIncomingCallIndication param malloc failed.");
            goto error;
        }
        param->rid = rid;
        // EAIC: <callId>, <number>, <type>, <call mode>, <seq no>
        param->callId = data[0];
        param->seqNo = data[4];
        param->approve = false;
        RIL_requestProxyTimedCallback(approveIncomingCall, param, &TIMEVAL_0,
                    getRILChannelId(RIL_CC, rid), "approveIncomingCall");
        return;
    }
    inCallNumber++;
    inCallNumberPerSim[rid]++;
    LOGD("inCallNumber: SIM%d=%d, total=%d", rid, inCallNumberPerSim[rid], inCallNumber);

    /* Check if contains forwarding address */
    if (at_tok_hasmore(&urc)) {
        ret = at_tok_nextstr(&urc, &data[5]);
        LOGD("contains forwarding address");
    }

    // IMS VoLTE refactoring
    if (atoi(data[3]) >= IMS_CALL_MODE_BOUNDARY) {  // URC call_mode >=20 means ims call else gsm call.
        LOGD("IMS: onIncomingCallIndication set dispatch_flag = IMS !!");
        call_state = mt;
        dispatch_flag = IMS;
    } else {
        LOGD("IMS: onIncomingCallIndication set dispatch_flag = GSM !!");
        dispatch_flag = GSM;
    }
    notifyImsCallExist(rid);

    if (dispatch_flag == IMS) {
        LOGD("MultiIMS: dispatch onIncomingCallIndication to IMS phone. socket id: %d", rid);
        asprintf(&data[6], "%d", rid);
    }
    /// @}

    /// M: CC: terminal based call waiting handling for non-c2k project
    if (!RatConfig_isC2kSupported() && dispatch_flag == GSM) {
        int callCount = inCallNumberPerSim[rid];
        RLOGD("non-c2k project onIncomingCallIndication, callCount:%d, ", callCount);

        IncomingCallParam *param = (IncomingCallParam *)malloc(sizeof(IncomingCallParam));
        if (param == NULL) {
            LOGE("onIncomingCallIndication param malloc failed.");
            goto error;
        }
        param->rid = rid;
        // EAIC: <callId>, <number>, <type>, <call mode>, <seq no>
        param->callId = data[0];
        param->seqNo = data[4];
        param->approve = true;

        if (callCount > 1) {
            char cwMode[PROPERTY_VALUE_MAX] = { 0 };
            getMSimProperty(rid, (char*)"persist.vendor.radio.terminal-based.cw", cwMode);
            if (!strcmp(cwMode, "enabled_tbcw_off")) {
                param->approve = false;
            }
        } /* (callCount > 0) */
        RIL_requestProxyTimedCallback(approveIncomingCall, param, &TIMEVAL_0,
                    getRILChannelId(RIL_CC, rid), "approveIncomingCall");
        return;
    }

    RIL_UNSOL_RESPONSE(RIL_UNSOL_INCOMING_CALL_INDICATION, data, 7 * sizeof(char *), rid);

    return;

error:
    LOGE("There is something wrong with the +EAIC");
}
/* ========================================================================== */

void approveIncomingCall(void *param)
{
    if(param == NULL) return;

    IncomingCallParam *data = (IncomingCallParam *) param;
    RIL_SOCKET_ID rid = data->rid;
    char *callId = data->callId;
    char *seqNo = data->seqNo;
    bool approve = data->approve;

    char *cmd = NULL;
    ATResponse *p_response = NULL;
    int err;
    LOGD("Incoming call (approve:%d) from skt id: %d, active skt is: %d", approve, (int)rid, activeRilSktID);

    // EAIC=1X, disallow incoming call
    // EAIC=0X, sallow incoming call
    asprintf(&cmd, "AT+EAIC=%d,%s,%s", (approve ? 0 : 1), callId, seqNo);
    err = at_send_command(cmd, &p_response, getChannelCtxbyProxy());

    if (err < 0 || p_response->success == 0) {
        LOGD("Incoming call (approve:%d) failed: send AT failed", approve);
    }

    free(cmd);
    free(data);
    at_response_free(p_response);
}

bool isActiveSocketConflict(RIL_SOCKET_ID rid) {
    int ret;
    ret = pthread_mutex_lock(&s_ActiveSocketMutex);
    if (ret != 0) {
        LOGE("isActiveSocketConflict, pthread_mutex_lock fail");
        return false;
    }
    if (activeRilSktID != INVALID_SOCKET && activeRilSktID != rid) {
        ret = pthread_mutex_unlock(&s_ActiveSocketMutex);
        if (ret != 0) {
            LOGE("isActiveSocketConflict, pthread_mutex_unlock fail");
            return false;
        }
        LOGD("IMS: ActiveSocket is conflict. activeRilSkt: %d, rid: %d", activeRilSktID, rid);
        return true;
    }
    ret = pthread_mutex_unlock(&s_ActiveSocketMutex);
    if (ret != 0) {
        LOGE("isActiveSocketConflict, pthread_mutex_unlock fail");
        return false;
    }
    LOGD("IMS: ActiveSocket isn't conflict activeRilSktID: %d", activeRilSktID);
    return false;
}

bool checkAndSetActiveSocket(RIL_SOCKET_ID rid) {
    int ret;
    ret = pthread_mutex_lock(&s_ActiveSocketMutex);
    if (ret != 0) {
        LOGE("checkAndSetActiveSocket, pthread_mutex_lock fail");
        return false;
    }
    if (isActiveSocketConflict(rid)) {
        ret = pthread_mutex_unlock(&s_ActiveSocketMutex);
        if (ret != 0) {
            LOGE("checkAndSetActiveSocket, pthread_mutex_unlock fail");
            return false;
        }
        return true;
    }
    LOGD("IMS: checkAndSetActiveSocket set activeRilSktID to %d", rid);
    activeRilSktID = rid;
    ret = pthread_mutex_unlock(&s_ActiveSocketMutex);
    if (ret != 0) {
        LOGE("checkAndSetActiveSocket, pthread_mutex_unlock fail");
        return false;
    }
    return false;
}

void resetActiveSocket() {
    LOGD("IMS: resetActiveSocket");
    int ret;
    ret = pthread_mutex_lock(&s_ActiveSocketMutex);
    if (ret != 0) {
        LOGE("resetActiveSocket, pthread_mutex_lock fail");
        return;
    }
    activeRilSktID = INVALID_SOCKET;
    ret = pthread_mutex_unlock(&s_ActiveSocketMutex);
    if (ret != 0) {
        LOGE("resetActiveSocket, pthread_mutex_unlock fail");
        return;
    }
}

void onCipherIndication(char *urc, RIL_SOCKET_ID rid) {
    int ret, i;
    char *data[4] = {"", "", "", ""};

    ret = at_tok_start(&urc);
    if (ret < 0) goto error;

    for (i = 0; i < 4; i++) {
        ret = at_tok_nextstr(&urc, &data[i]);
        if (ret < 0) goto error;
    }

    RIL_UNSOL_RESPONSE(RIL_UNSOL_CIPHER_INDICATION, data, 4 * sizeof(char *), rid);

    return;

error:
    LOGE("There is something wrong with the +ECIPH");
}

/*
As specificed in 3GPP 24.301, 5.3.7 Handling of the Local Emergency Numbers
*/
void onEmergencyCallServiceCategoryIndication(char *urc, RIL_SOCKET_ID rid) {
    /*
    This command allows for reading and dynamical reporting of emergency numbers as received
    from the network.The emergency numbers are not necessarily received for the same <mcc> and
    <mnc> as currently registered to.
    Read command returns one line of intermediate result code +CEN1: <reporting>,<mcc> with the
    current <reporting> setting and the Mobile Country Code <mcc>. Then follows zero or more
    occurrences of the emergency numbers with intermediate result code +CEN2: <cat>,<number>.
    Set command enables reporting of new emergency numbers received from the network with
    unsolicited result codes equal to the intermediate result codes of the read form of the
    command.
    Test command returns values supported as compound values.

    URC format:
        +CEN1: <reporting>[,<mcc>,<mnc>]
        +CEN2: <cat>,<number>
    */

    char *ptr = NULL;
    char list[PROPERTY_VALUE_MAX] = {0};
    char newList[PROPERTY_VALUE_MAX] = {0};
    char oldMcc[PROPERTY_VALUE_MAX] = {0};
    char findStr[50] = {0};
    int category = -1, ret;
    int len, reporting;
    char *mcc = NULL;
    char *number = NULL;

    /* For test
    char *urc = (char*)malloc(1024);
    property_get("vendor.ril.test", urc, "");
    */
    int simId = getMappingSIMByCurrentMode(rid);

    // If the MCC is changed, clear emergency number/category mapping list.
    if (strStartsWith(urc, "+CEN1")) {
        ret = at_tok_start(&urc);
        if (ret < 0) goto error;

        // Get reporting
        ret = at_tok_nextint(&urc, &reporting);
        if (ret >= 0) {
            // Get mcc
            ret = at_tok_nextstr(&urc, &mcc);
        }

        if (ret >= 0) {
            property_get(PROPERTY_ECC_MCC[simId], oldMcc, "0");
            if (strcmp(oldMcc, mcc) != 0) {
                // MCC is changed, clear mapping list (ril_sim.c also clear mapping when PLUG OUT)
                property_set(PROPERTY_NW_ECC[simId], "");
                // Update MCC
                property_set(PROPERTY_ECC_MCC[simId], mcc);
                LOGD("[onEmergencyCallServiceCategoryIndication]reset %s. MCC[%d] from %s to %s",
                        PROPERTY_NW_ECC[simId], simId, oldMcc, mcc);

                // Notify RIL Proxy to update ECC list
                RIL_UNSOL_RESPONSE(RIL_UNSOL_EMERGENCY_NUMBER_LIST, NULL, 0, rid);
            }
            toRefreshNwECC[simId] = true;
        }
        return;
    }

    if (strStartsWith(urc, "+CEN2")) {
        // Update service category and emergency number
        ret = at_tok_start(&urc);
        if (ret < 0) goto error;

        // Get category
        ret = at_tok_nextint(&urc, &category);
        if (ret >= 0) {
            // Get number
            ret = at_tok_nextstr(&urc, &number);
        }

        if (ret >= 0) {
            LOGD("[onEmergencyCallServiceCategoryIndication]number = %s, category = %d",
                    number, category);

            if (toRefreshNwECC[simId]) {
                // New NW ECC update, clear mapping list
                property_set(PROPERTY_NW_ECC[simId], "");
                toRefreshNwECC[simId] = false;
            }

            // Get current category/number mapping list
            property_get(PROPERTY_NW_ECC[simId], list, "");

            sprintf(findStr, ";%s,", number);
            ptr = strstr(list, findStr);
            if (ptr == NULL) {  // Not found
                sprintf(newList, "%s;%s,%d", list, number, category);
                LOGD("[onEmergencyCallServiceCategoryIndication][Not found]list = %s", newList);
            } else {
                char tmpList[PROPERTY_VALUE_MAX] = {0};

                memset(newList, 0, PROPERTY_VALUE_MAX);
                memset(tmpList, 0, PROPERTY_VALUE_MAX);

                // Copy the mapping string before finding number
                len = ptr - list;
                strncpy(tmpList, list, len);

                ptr = strstr(ptr + 1, ";");
                if (ptr != NULL) {
                    // Copy the mapping string after finding number
                    len = sizeof(tmpList) - strlen(tmpList);
                    strncat(tmpList, ptr, len - 1);
                }

                sprintf(newList, "%s;%s,%d", tmpList, number, category);
                LOGD("[onEmergencyCallServiceCategoryIndication][Found]list = %s", newList);
            }

            // Update mapping list
            property_set(PROPERTY_NW_ECC[simId], newList);

            // Notify RIL Proxy to update ECC list
            RIL_UNSOL_RESPONSE(RIL_UNSOL_EMERGENCY_NUMBER_LIST, NULL, 0, rid);

            /* For test */
            // free(urc);

            return;
        }
    }

error:
    LOGE("There is something wrong with the +CEN");
}

void onMccMncIndication(RIL_SOCKET_ID slotId, char* mccmnc) {
    char oldMcc[PROPERTY_VALUE_MAX] = {0};
    char mcc[PROPERTY_VALUE_MAX] = {0};

    // The sequence of modem report network ecc list and camp on cell is not fixed.
    // So there will be cases when modem report network ecc list (+CEN1/+CEN2) and
    // AP will save the network ECC then before camp on cell, EOPS will be 000000.
    // Thus if AP query PLMN at this time, onMccMncIndication() will be called and
    // previous network ECC saved will be cleared. To fix this timing issue, we'll
    // not reset network ECC when PLMN change to "000000".
    if (strcmp(mccmnc, "000000") == 0) {
        LOGD("[onMccMncIndication] don't reset before camp on cell");
        return;
    }

    if (strlen(mccmnc) >= 3) {
        strncpy(mcc, mccmnc, 3);
    }
    // Check if the latest MCC is different from the value stored in system property,
    // and if they are different then clear emergency number and service category
    property_get(PROPERTY_ECC_MCC[slotId], oldMcc, "");
    if (strcmp(oldMcc, mcc) != 0) {
        property_set(PROPERTY_NW_ECC[slotId], "");
        property_set(PROPERTY_ECC_MCC[slotId], mcc);
        LOGD("[%s][%d] reset network ECC due to MCC change from %s to %s",
                __FUNCTION__, slotId, oldMcc, mcc);

        // Notify RIL Proxy to update ECC lists
        RIL_UNSOL_RESPONSE(RIL_UNSOL_EMERGENCY_NUMBER_LIST, mccmnc, strlen(mccmnc), slotId);
    }
}

void onSpeechInfo(char *urc, RIL_SOCKET_ID rid) {
    int ret;
    int response[3] = { 0 };

    ret = at_tok_start(&urc);
    if (ret < 0) goto error;

    // get on_off
    ret = at_tok_nextint(&urc, &response[0]);
    if (ret < 0) goto error;

    // get rat
    ret = at_tok_nextint(&urc, &response[1]);
    if (ret < 0) goto error;

    // get irho_on_off
    ret = at_tok_nextint(&urc, &response[2]);
    if (ret < 0) goto error;

    /* Solve [ALPS00846094]general PR: In india,make a call from Mumbai to Delhi using 985
       at callers end, the caller hear a DTMF tone in addition to RBT */
    bEspeechIsTurnedOn = (response[0] != 0);

    /* To solve [ALPS01154499][A10-TE-MY-Celcom-N/A][FT]Using Celcom number MO call to
    Maxis number DUT play 2 kind of ringback tone(100%) */
    if (isAlertSet && bEspeechIsTurnedOn) {
        LOGD("Send in band tone notification!!!Stop since speech is turned on!!!");
        int result[1] = { 0 };
        RIL_onUnsolicitedResponse(RIL_UNSOL_RINGBACK_TONE, result, sizeof(result), rid);
        isAlertSet = 0;
    }
    return;

error:
    LOGE("There is something wrong with the +ESPEECH");
}

void onCallProgressInfoCallStateChange(char *urc, RIL_SOCKET_ID rid) {
    int ret, i;
    char *data[10] = {"", "", "", "", "", "", "", "", "", ""};
    int msg_type = -1;
    int need_to_report_event = 1;
    int call_id = -1;
    int call_mode = -1;

    /* +ECPI:<call_id>, <msg_type>, <is_ibt>, <is_tch>, <dir>, <call_mode>[, <number>, <toa>], "",<cause>
     *
     * if msg_type = DISCONNECT_MSG or ALL_CALLS_DISC_MSG,
     * +ECPI:<call_id>, <msg_type>, <is_ibt>, <is_tch>,,,"",,"",<cause>
     *
     * if others,
     * +ECPI:<call_id>, <msg_type>, <is_ibt>, <is_tch>, <dir>, <call_mode>[, <number>, <toa>], ""
     *
     *      0  O  CSMCC_SETUP_MSG
     *      1  X  CSMCC_DISCONNECT_MSG
     *      2  O  CSMCC_ALERT_MSG
     *      3  X  CSMCC_CALL_PROCESS_MSG
     *      4  X  CSMCC_SYNC_MSG
     *      5  X  CSMCC_PROGRESS_MSG
     *      6  O  CSMCC_CALL_CONNECTED_MSG
     *   129  X  CSMCC_ALL_CALLS_DISC_MSG
     *   130  O  CSMCC_MO_CALL_ID_ASSIGN_MSG
     *   131  O  CSMCC_STATE_CHANGE_HELD
     *   132  O  CSMCC_STATE_CHANGE_ACTIVE
     *   133  O  CSMCC_STATE_CHANGE_DISCONNECTED
     *   134  X  CSMCC_STATE_CHANGE_MO_DISCONNECTING
     *   135  O  CSMCC_STATE_CHANGE_REMOTE_HELD
     *   136  O  CSMCC_STATE_CHANGE_REMOTE_RESUMED
     *   137  O  RTT_EMC_REDIAL_EVENT
     */

    ret = at_tok_start(&urc);
    if (ret < 0) return;

    for (i = 0; i < 10; i++) {
        at_tok_nextstr(&urc, &data[i]);
    }

    call_id = atoi(data[0]);
    msg_type = atoi(data[1]);
    if ((call_id >= 1) && (call_id <= MAX_GSMCALL_CONNECTIONS)) {
        ecpiValue[call_id-1] = msg_type;
    }

    switch (msg_type) {
        case 0:

            /* APP will display MT call screen when receiving newRingConnection,
             * but MD doesn't allow ATA before RING URC is sent, therefore delay
             * this CALL_PROGRESS_INFO to receiving RING URC
             */
            if (!hasReceivedRing && callWaiting == 0) {
                need_to_report_event = 0;

                LOGD("queue setup CPI!!!!!!");

                for (i = 0; i < 9; i++) {
                    if (setupCpiData[i] != NULL) {
                        free(setupCpiData[i]);
                        setupCpiData[i] = NULL;
                    }

                    size_t copySize = strlen(data[i]) + 1;
                    setupCpiData[i] = malloc(copySize);
                    if (setupCpiData[i] == NULL) {
                        LOGE("onCallProgressInfoCallStateChange setupCpiData[%d] alloc fail.", i);
                        goto error;
                    }
                    memcpy(setupCpiData[i], data[i], copySize);
                }

                isRecvECPI0 = true;
                ringCallID = call_id;
                ridRecvECPI0 = rid;
                LOGD("set RecvECPI rid=%d", ridRecvECPI0);
            } else {
                if (!hasReceivedRing && callWaiting != 0) {
                    callWaiting = 0;
                }
            }

            // [ALPS00370557]DTMF and RBT tone are played simulataneously.
            bEspeechIsTurnedOn = 0;
            // LOGD("[onCallProgressInfo]Reset bEspeechIsTurnedOn = %d", bEspeechIsTurnedOn);

            // [ALPS00347908][MT6517JB][VT Call]The "Invalid number" pop up,
            // when end MT VT call by remote side.
            bUseLocalCallFailCause = 0;
            dialLastError = 0;

            break;

        case 130:
            if (dispatch_flag == IMS) {
                data[5] = "20";            // set call mode for ECPI 130
            }
            if (call_id != 254) {
                if (checkAndSetActiveSocket(rid)) {
                    LOGD("Failed to make call from different skt: %d", (int)rid);
                    break;
                }
                inCallNumber++;
                inCallNumberPerSim[rid]++;
                LOGD("inCallNumber: SIM%d=%d, total=%d", rid,
                        inCallNumberPerSim[rid], inCallNumber);

                if (call_state == pending_mo) {
                    LOGD("IMS: this call is PS MO call");
                    call_state = mo;
                }

                // [ALPS00370557]DTMF and RBT tone are played simulataneously.
                bEspeechIsTurnedOn = 0;
                // LOGD("[onCallProgressInfo]Reset bEspeechIsTurnedOn = %d", bEspeechIsTurnedOn);

                // [ALPS00347908][MT6517JB][VT Call]The "Invalid number" pop up,
                // when end MT VT call by remote side.
                bUseLocalCallFailCause = 0;
                dialLastError = 0;
            }
            break;

        case 6:
            if (hasReceivedRing) {
                LOGD("clear RING flag!!!!!!");
                hasReceivedRing = 0;
            }
            break;

        case 133:
            // Notify RIL SS that ECPI 133 is coming
            onECPI133Received();
            if (hasReceivedRing) {
                LOGD("clear RING flag!!!!!!");
                hasReceivedRing = 0;
            }

            /// EAIC from SIM(b) should be rejected while SIM(a) is activated,
            /// the paired +ECPI 133 should be ignored, either. @{
            if (isActiveSocketConflict(rid)) {
                LOGD("+ECPI 133 ignored, activeRil: %d, Ril: %d", activeRilSktID, (int)rid);
                return;
            }
            /// @}

            int disc_callID = call_id;

            if ((disc_callID == ringCallID) && (ridRecvECPI0 == rid)) {
                LOGD("clear RecvECPI rid=%d", ridRecvECPI0);
                isRecvECPI0 = false;
                ringCallID = 0;
                ridRecvECPI0 = MAX_SIM_COUNT;

                for (i = 0; i < 9; i++) {
                    if (setupCpiData[i] != NULL) {
                       free(setupCpiData[i]);
                       setupCpiData[i] = NULL;
                    }
                }
            }

            // Fill CNAP in individual CLCC parsing and clear in ECPI:133
            if ((disc_callID >= 1) && (disc_callID <= MAX_GSMCALL_CONNECTIONS)) {
                memset(cachedClccName[disc_callID-1], 0, MAX_CNAP_LENGTH);
            }

            if (inCallNumber > 0) {
                inCallNumber--;
            } else {
                // 133 and EAIC/130 should be paired, if inCallNumber is less than 0, it may be
                // something wrong, mtk04070, 2012.02.20
                LOGD("[Error]Something wrong (inCallNumber is less than 0)!");
            }
            if (inCallNumberPerSim[rid] > 0) {
                inCallNumberPerSim[rid]--;
            } else {
                // 133 and EAIC/130 should be paired, if inCallNumber is less than 0, it may be
                // something wrong, mtk04070, 2012.02.20
                LOGD("[Error]Something wrong (inCallNumber for SIM%d is less than 0)!", rid);
            }

            if (disc_callID == disallow_mt_call_ID) {
                disallow_mt_call_ID = -1;
                disallow_mt_call_SeqNo = -1;
            }

            // [ALPS00303910]The second call can't be added. mtk04070, 20120619
            inDTMF = 0;
            // [ALPS00303910]The second call can't be added. mtk04070, 20120619

            if (isAlertSet) {
                int response[1] = { 0 };

                LOGD("Send in band tone notification!!!Stop!!!");
                response[0] = 0;
                RIL_UNSOL_RESPONSE(RIL_UNSOL_RINGBACK_TONE,
                        response, sizeof(response), rid);
                isAlertSet = 0;
            }

            /// M: For ECC redial requirement, need report cause to RIL proxy. @{
            if (at_tok_hasmore(&urc)) {
               ret = at_tok_nextstr(&urc, &data[9]);
            }
            /// @}

            // For AT&T ECC
            // Not allow to send EMCS for OP07 if C2K supported
            if (isOp07Support() && !RatConfig_isC2kSupported()) {
                if (bEmergencyMode == true) {
                    TimedCallbackParam *param =
                            (TimedCallbackParam *)malloc(sizeof(TimedCallbackParam));
                    if (param != NULL) {
                        param->rid = rid;
                        RIL_requestProxyTimedCallback(stopEmergencySession, param, &TIMEVAL_0,
                                getRILChannelId(RIL_CC, rid), "stopEmergencySession");
                    } else {
                        LOGE("onCallProgressInfoCallStateChange param malloc failed.");
                    }
                    bEmergencyMode = false;
                }
            }
            break;

        case 2:
          // [ALPS00846094]general PR: In india,make a call from Mumbai to Delhi using 985
          // at callers end, the caller hear a DTMF tone in addition to RBT
          // Ringback tone should be played by PhoneAPP only
          // when "+ECPI: x,2..." is received and "+ESPEECH: 1..." is not received yet.
            if ((strcmp(data[2], "0") == 0) && !bEspeechIsTurnedOn) {
                int response[1] = { 0 };

                LOGD("Send in band tone notification!!!Start!!!");
                response[0] = 1;
                RIL_UNSOL_RESPONSE(RIL_UNSOL_RINGBACK_TONE,
                        response, sizeof(response), rid);
                isAlertSet = 1;
            }
            break;

        case 132:
            if (isAlertSet) {
                int response[1] = { 0 };

                LOGD("Send in band tone notification!!!Stop!!!");
                response[0] = 0;
                RIL_UNSOL_RESPONSE(RIL_UNSOL_RINGBACK_TONE,
                        response, sizeof(response), rid);
                isAlertSet = 0;
            }
            break;
    }

    // Notify call mode and pau information.
    if (strcmp(data[1], "2") == 0 ||
            strcmp(data[1], "6") == 0 ||
            strcmp(data[1], "130") == 0 ||
            strcmp(data[1], "131") == 0 ||
            strcmp(data[1], "132") == 0 ||
            strcmp(data[1], "133") == 0 ||
            strcmp(data[1], "135") == 0 ||
            strcmp(data[1], "136") == 0 ||
            strcmp(data[1], "137") == 0) {
        if (dispatch_flag == IMS) {
            RIL_UNSOL_RESPONSE(
                   RIL_UNSOL_CALL_INFO_INDICATION,
                   data, 10 * sizeof(char *), rid);
        }
    }

    /*if disconnect related message, parsing <cause> field*/
    if (need_to_report_event &&
            (msg_type == 0 ||
            msg_type == 2 ||
            msg_type == 6 ||
            msg_type == 130 ||
            msg_type == 131 ||
            msg_type == 132 ||
            msg_type == 133 ||
            msg_type == 254)) {
        if (dispatch_flag == IMS) {
            // remove this redundant URC for IMS
        } else {
            RIL_UNSOL_RESPONSE(
                    RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, NULL, 0, rid);
        }
    }

    if (strcmp(data[1], "133") == 0 && inCallNumber == 0 && call_state != pending_mo) {
        LOGD("IMS: disconnect call and call number is 0 and call state is not pending MO, then reset dispatch_flag = GSM");
        dispatch_flag = GSM;
        notifyImsCallExist(rid);
        call_state = none;
        resetActiveSocket();
    }

    return;

error:
    LOGE("There is something wrong with onCallProgressInfoCallStateChange.");
}

/**
 * RIL_UNSOL_SUPP_SVC_NOTIFICATION
 *
 * Reports supplementary service related notification from the network.
 *
 * "data" is a const RIL_SuppSvcNotification *
 *
 */
void onSuppSvcNotification(char *s, int isMT, RIL_SOCKET_ID rid)
{
    RIL_SuppSvcNotification svcNotify;
    char* line = s;

    memset(&svcNotify, 0, sizeof(RIL_SuppSvcNotification));

    /**
     * +CSSN=[<n>[,<m>]]
     * +CSSN?  +CSSN: <n>,<m>
     * +CSSN=? +CSSN: (list of supported <n>s),(list of supported <m>s)
     */
    /**
     * When <n>=1 and a supplementary service notification is received
     * after a mobile originated call setup, intermediate result code
     * +CSSI: <code1>[,<index>] is sent to TE before any other MO call
     * setup result codes presented in the present document or in V.25ter [14].
     * When several different <code1>s are received from the network,
     * each of them shall have its own +CSSI result code.
     * <code1> (it is manufacturer specific, which of these codes are supported):
     * 0   unconditional call forwarding is active
     * 1   some of the conditional call forwardings are active
     * 2   call has been forwarded
     * 3   call is waiting
     * 4   this is a CUG call (also <index> present)
     * 5   outgoing calls are barred
     * 6   incoming calls are barred
     * 7   CLIR suppression rejected
     * 8   call has been deflected
     * <index>: refer "Closed user group +CCUG"
     */
     /**
      * When <m>=1 and a supplementary service notification is received
      * during a mobile terminated call setup or during a call, or when
      * a forward check supplementary service notification is received,
      * unsolicited result code +CSSU: <code2>[,<index>[,<number>,<type>[,<subaddr>,<satype>]]]
      * is sent to TE. In case of MT call setup, result code is sent after every +CLIP result code
      * (refer command "Calling line identification presentation +CLIP")
      * and when several different <code2>s are received from the network,
      * each of them shall have its own +CSSU result code.
      * <code2> (it is manufacturer specific, which of these codes are supported):
      * 0   this is a forwarded call (MT call setup)
      * 1   this is a CUG call (also <index> present) (MT call setup)
      * 2   call has been put on hold (during a voice call)
      * 3   call has been retrieved (during a voice call)
      * 4   multiparty call entered (during a voice call)
      * 5   call on hold has been released (this is not a SS notification) (during a voice call)
      * 6   forward check SS message received (can be received whenever)
      * 7   call is being connected (alerting) with the remote party in alerting state in explicit call transfer operation (during a voice call)
      * 8   call has been connected with the other remote party in explicit call transfer operation (also number and subaddress parameters may be present) (during a voice call or MT call setup)
      * 9   this is a deflected call (MT call setup)
      * 10  additional incoming call forwarded
      * 11  MT is a forwarded call (CF)
      * 12  MT is a forwarded call (CFU)
      * 13  MT is a forwarded call (CFC)
      * 14  MT is a forwarded call (CFB)
      * 15  MT is a forwarded call (CFNRy)
      * 16  MT is a forwarded call (CFNRc)
      * <number>: string type phone number of format specified by <type>
      * <type>: type of address octet in integer format (refer GSM 04.08 [8] subclause 10.5.4.7)
      */

    svcNotify.notificationType = isMT;

    if (at_tok_start(&line) < 0) {
        goto error;
    }

    if (at_tok_nextint(&line, &(svcNotify.code)) < 0) {
        goto error;
    }

    if (isMT == 1 && dispatch_flag == IMS) {
        // code 2 = remote hold, code 3 = remote resume
        // IMS use ECPI 135/136, CSSU:2, 3 should be ignore.
        if (svcNotify.code == 2 || svcNotify.code == 3) {
            LOGD("SUPP notificatioin: IMS ignore CSSU - remote hold / resume");
            return;
        }
    }

    if (at_tok_hasmore(&line)) {
        /* Get <index> field */
        at_tok_nextint(&line, &(svcNotify.index));
    }

    if (isMT) {
        if(at_tok_hasmore(&line)) {
            /* Get <number> */
            at_tok_nextstr(&line, &(svcNotify.number));

            /* Get <type> */
            at_tok_nextint(&line, &(svcNotify.type));
        }
    }

    // M: CC: to adapt to AOSP framework
    if (isMT && (svcNotify.code == 0 || (svcNotify.code >= 11 && svcNotify.code <= 16))) {
        LOGD("convert proprietary CSSU <code2> for MT forwarded call to 0");
        RIL_UNSOL_RESPONSE (
                RIL_UNSOL_SUPP_SVC_NOTIFICATION_EX,
                &svcNotify, sizeof(svcNotify),
                rid);
        svcNotify.code = 0;
    }


    RIL_UNSOL_RESPONSE (
            RIL_UNSOL_SUPP_SVC_NOTIFICATION,
            &svcNotify, sizeof(svcNotify),
            rid);

    return;

error:
    LOGE("Parse RIL_UNSOL_SUPP_SVC_NOTIFICATION fail: %s/n", s);
}

/// M: Set service category before making Emergency call. @{
/**
* To set service category before making Emergency call.
*/
void requestLocalSetEccServiceCategory(void *data, size_t datalen, RIL_Token t) {
    int ret;
    int serviceCategory = ((int *)data)[0];
    ATResponse *p_response = NULL;
    char *line = NULL;

    UNUSED(datalen);

    if (serviceCategory == 0 && mServiceCategory != 0) {
        serviceCategory = mServiceCategory;
        mServiceCategory = 0;
    }
    asprintf(&line, "AT+ESVC=%d", serviceCategory);
    ret = at_send_command(line, &p_response, CC_CHANNEL_CTX);
    free(line);

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
/// @}

void requestDtmfStart(void *data, size_t datalen, RIL_Token t) {
    /* MTK proprietary start */
    char c = ((char *)data)[0];
    char *cmd = NULL;
    int ret;
    ATResponse *p_response = NULL;

    UNUSED(datalen);

    if (inDTMF == 1) {
        LOGD("[DTMF Start Error]inDTMF is already 1!!");
    }

    asprintf(&cmd, "AT+EVTS=0,\"%c\"", c);
    inDTMF = 1;

    ret = at_send_command(cmd, &p_response, CC_CHANNEL_CTX);

    free(cmd);

    // [ALPS00281513] make sure DTMF start request is executed successfully before set inDTMF to 1
    // mtk04070, 20120511
    // TODO: When AT command send fail, return SUCCESS or GENERIC_FAILURE?
    if (ret < 0 || p_response == NULL || p_response->success == 0) {
       inDTMF = 0;
       LOGD("DTMF Start Error!!");
    }

    LOGD("DTMF Start done!!");

    at_response_free(p_response);


    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    /* MTK proprietary end */
}

void requestDtmfStop(void *data, size_t datalen, RIL_Token t) {
    int ret;
    ATResponse *p_response = NULL;

    UNUSED(data);
    UNUSED(datalen);

    /* MTK proprietary start */
    ret = at_send_command("AT+EVTS=1", &p_response, CC_CHANNEL_CTX);
    // TODO: When AT command send fail, return SUCCESS or GENERIC_FAILURE?
    if (ret < 0 || p_response == NULL || p_response->success == 0) {
        LOGD("DTMF Stop Error!!");
    }

    inDTMF = 0;
    LOGD("DTMF Stop done!!");

    at_response_free(p_response);


    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    /* MTK proprietary end */
}

void requestSetTTYMode(void *data, size_t datalen, RIL_Token t) {
    /* MTK proprietary start */
    int *c = (int *)data;
    char *cmd = NULL;
    int ret;
    ATResponse *p_response = NULL;

    UNUSED(datalen);

    asprintf(&cmd, "AT+CTMCALL=%d", c[0]);
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
    /* MTK proprietary end */
}

void requestQueryTTYMode(void *data, size_t datalen, RIL_Token t) {
    int ret;
    ATResponse *p_response = NULL;
    int response = 0;
    char *line = NULL;

    UNUSED(data);
    UNUSED(datalen);

    ret = at_send_command_singleline("AT+CTMCALL?", "+CTMCALL:", &p_response, CC_CHANNEL_CTX);

    if (ret < 0 || p_response == NULL || p_response->success == 0) {
        goto error;
    }

    line = p_response->p_intermediates->line;

    ret = at_tok_start(&line);

    if (ret < 0) {
        goto error;
    }

    ret = at_tok_nextint(&line, &response);

    if (ret < 0) {
        goto error;
    }

    LOGD("TtyMode = %d", response);

    RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(int));
    at_response_free(p_response);
    return;
error:
    response = 0;
    RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(int));
    at_response_free(p_response);
}

void onSpeechCodecInfo(char *urc, RIL_SOCKET_ID rid) {
    int ret;
    int info = 0;

    LOGD("onSpeechCodecInfo:%s", urc);

    if (strStartsWith(urc, "+EVOCD")) {
        ret = at_tok_start(&urc);
        if (ret < 0) goto error;

        ret = at_tok_nextint(&urc, &info);
        if (ret < 0) goto error;

    } else {
        goto error;
    }

    if (mSpeechCodec != info) {
        LOGD("onSpeechCodecInfo value = %d, (mSpeechCodec: %d->%d)", info, mSpeechCodec, info);
        mSpeechCodec = info;
    } else {
        LOGD("onSpeechCodecInfo value = %d, ", info);
    }
    if (dispatch_flag == IMS) {
        RIL_UNSOL_RESPONSE(RIL_UNSOL_SPEECH_CODEC_INFO, &info, sizeof(int), rid);
    } else if (inCallNumberPerSim[rid] > 0) {
        RIL_UNSOL_RESPONSE(RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, NULL, 0, rid);
    }
    return;

error:
    LOGE("error on onSpeechCodecInfo");
}

void requestForceReleaseCall(void *data, size_t datalen, RIL_Token t) {
    int *c = (int *)data;
    char *cmd = NULL;
    int ret;
    ATResponse *p_response = NULL;

    UNUSED(datalen);

    asprintf(&cmd, "AT+ECHUP=%d", c[0]);

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

// For AT&T ECC
void stopEmergencySession(void *param) {
    if(param == NULL) return;

    TimedCallbackParam *data = (TimedCallbackParam *) param;
    RIL_SOCKET_ID rid = data->rid;

    ATResponse *p_response = NULL;
    int err;

    LOGD("stopEmergencySession");

    err = at_send_command("AT+EMCS=0", &p_response, getChannelCtxbyProxy());

    if (err < 0 || p_response->success == 0) {
        LOGD("stopEmergencySession failed: send AT failed");
    }

    free(data);
    at_response_free(p_response);
}

// Verizon E911
/* Let modem know start of E911 and deliver some information
 *
 * AT+EMCS:<emc_session>,<airplane_mode>
 * emc_session: integer
 *  1 : emergency session begin
 *
 * airplaine_mode: integer
 *  0 : off
 *  1 : on
 */
void requestSetEccMode(void *data, size_t datalen, RIL_Token t) {
    int ret;
    const char** strings = (const char**)data;
    int enable = atoi(strings[1]);
    int airplaneMode = atoi(strings[2]);
    char *cmd = NULL;
    ATResponse *p_response = NULL;

    UNUSED(datalen);
    // For AT&T ECC
    // Not allow to send EMCS for OP07 if C2K supported
    if (isOp07Support() && RatConfig_isC2kSupported()) {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
        return;
    }

    if (enable) {
        asprintf(&cmd, "AT+EMCS=1,%d", airplaneMode);

        LOGD("[requestSetEccMode]cmd = %s", cmd);

        ret = at_send_command(cmd, &p_response, CC_CHANNEL_CTX);

        if (ret < 0 || p_response->success == 0) {
            RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
        } else {
            RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
            // For AT&T ECC
            // Not allow to send EMCS for OP07 if C2K supported
            if (isOp07Support() && !RatConfig_isC2kSupported()) {
               bEmergencyMode = true;
            }
        }
    } else {
        asprintf(&cmd, "AT+EMCS=0");

        LOGD("[requestSetEccMode]cmd = %s", cmd);

        ret = at_send_command(cmd, &p_response, CC_CHANNEL_CTX);

        if (ret < 0 || p_response->success == 0) {
            RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
        } else {
            RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
            // For AT&T ECC
            // Not allow to send EMCS for OP07 if C2K supported
            if (isOp07Support() && !RatConfig_isC2kSupported()) {
               bEmergencyMode = false;
            }
        }
    }

    at_response_free(p_response);

    return;
}

/**
 * To notify emergency call back mode status.
 *
 * AT command format: +CIEV:<ind>,<value>
*/
void onEmergencyCallBackMode(const char *s, const RIL_SOCKET_ID rid) {
    char *line = NULL;
    int ind, err;
    int value;

    LOGD("onEmergencyCallBackMode:%s", s);
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

    LOGD("onEmergencyCallBackMode ind=%d, value=%d", ind, value);
    /* 102: E911 Mode Indicator */
    if (ind == 102) {
        /* value = 1: E911 Mode on
           value = 0: E911 Mode off */
        int unsol_response = -1;
        if (value == 1) {
            unsol_response = RIL_UNSOL_ENTER_EMERGENCY_CALLBACK_MODE;
        } else if (value == 0) {
            unsol_response = RIL_UNSOL_EXIT_EMERGENCY_CALLBACK_MODE;
        }
        if (unsol_response < 0) {
            free(dup);
            return;
        }
        RIL_UNSOL_RESPONSE(unsol_response, NULL,  0, rid);
    }

    free(dup);
}

/// M: ECBM requirement. @{
void requestExitEmergencyCallbackMode(void *data, size_t datalen, RIL_Token t)
{
    int ret = 0;
    LOGD("%s", "request exit ECBM");
    ATResponse *p_response = NULL;

    UNUSED(data);
    UNUSED(datalen);

    ret = at_send_command("AT+VMEMEXIT", &p_response, CC_CHANNEL_CTX);
    if (ret < 0 || p_response == NULL || p_response->success == 0) {
        goto error;
    }
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    at_response_free(p_response);
    return;
error:
    RIL_onRequestComplete(t, RIL_E_INTERNAL_ERR, NULL, 0);
    at_response_free(p_response);
}
/// @}

/**
 * Normal/Emergency call redial
 *
 * AT command format: ATDER="<call_number>",<redial_call_id>
 *
*/
void requestRedial(void *data, size_t datalen, RIL_Token t) {
    RIL_Redial *p_reDial;
    char *cmd = NULL;
    int ret;
    ATResponse *p_response = NULL;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));

    if (checkAndSetActiveSocket(rid)) {
        rejectDial();
        goto error;
    }

    LOGD("requestRedial set ADS to CS domain");
    ret = at_send_command("AT+EVADSMOD=1", &p_response, CC_CHANNEL_CTX);
    if (ret < 0 || p_response == NULL || p_response->success == 0) {
        LOGE("requestRedial: AT+EVADSMOD=1 Fail !!");
    }
    at_response_free(p_response);
    p_response = NULL;

    p_reDial = (RIL_Redial *)data;

    char *formatNumber = NULL;
    char *tmp = strchr(p_reDial->address, ',');
    if (tmp != NULL) {
        int pos = (tmp - p_reDial->address) / sizeof(char);
        formatNumber = (char *)alloca(pos + 1);
        if (formatNumber == NULL) {
            LOGE("requestRedial: alloc fail (formatNumber)");
            goto error;
        }
        memset(formatNumber, 0, pos + 1);
        strncpy(formatNumber, p_reDial->address, pos);
    } else {
        formatNumber = p_reDial->address;
    }

    asprintf(&cmd, "ATDER=\"%s\",%d", formatNumber, p_reDial->callId);

    LOGD("requestRedial, command:%s", cmd);
    ret = at_send_command(cmd, &p_response, CC_CHANNEL_CTX);
    free(cmd);

    if (ret < 0 || p_response == NULL || p_response->success == 0) {
        goto error;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    at_response_free(p_response);
    return;

error:
    if (inCallNumber == 0) {
        resetActiveSocket();
    }
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

/**
 * To notify emergency call session start or stop.
 *
 * AT command format: AT+EMCS=<emergency_mode>
 * emergency_mode: integer
 *  1: emergency call session start (deprecated)
 *  0: emergency call session stop
 *
*/
void requestNotifyEmergencySessionStatus(void *data, size_t datalen, RIL_Token t, int isStarted)
{
    char *cmd = NULL;
    int ret;
    ATResponse *p_response = NULL;

    asprintf(&cmd, "AT+EMCS=%d", isStarted);
    LOGD("[GSM RILD]requestNotifyEmergencySessionStatus, command:%s", cmd);
    ret = at_send_command(cmd, &p_response, CC_CHANNEL_CTX);
    free(cmd);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    at_response_free(p_response);
}

/**
 * Set hVoLTE mode to modem
 *
 * AT+EHVOLTE=<hvolte_mode>
 * hvolte_mode: integer
 *  0: SRLTE mode
 *  1: LTE only mode
 */
void requestSetHvolteMode(void *data, size_t datalen, RIL_Token t) {
    int ret;
    int hvolteMode = ((int *)data)[0];
    int subMode = (datalen > 1) ? ((int *)data)[1] : 0;
    char cmd[255];
    ATResponse *p_response = NULL;

    sprintf(cmd, "AT+EHVOLTE=%d,%d", hvolteMode, subMode);

    LOGD("[requestSetHvolteMode]cmd = %s", cmd);

    ret = at_send_command(cmd, &p_response, CC_CHANNEL_CTX);

    if (ret < 0 || p_response->success == 0) {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }

    at_response_free(p_response);

    return;
}

