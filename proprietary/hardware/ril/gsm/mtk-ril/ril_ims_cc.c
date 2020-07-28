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
#include <assert.h>
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
#include <cutils/properties.h>
#include <termios.h>
#include <signal.h>
#include <ctype.h>

#include <ril_callbacks.h>

#include <hardware_legacy/power.h>

#include "ril_cc.h"
#include "ril_ims_cc.h"

#define CC_CHANNEL_CTX getRILChannelCtxFromToken(t)

void notifyImsCallExist(RIL_SOCKET_ID rid);
void setRequestFromImsIfNeeded(RIL_Token t);

extern void rejectDial();
extern void requestDial(void * data, size_t datalen, RIL_Token t, int isEmergency, int isVt);
dispatch_flg dispatch_flag = GSM;
volte_call_state call_state = none;
extern int hasReceivedRing;
extern bool isRecvECPI0;
extern char *setupCpiData[];
extern bool checkAndSetActiveSocket(RIL_SOCKET_ID rid);
extern int activeRilSktID;

void requestImsDial(void * data, size_t datalen, RIL_Token t, int isEmergency, int isVt) {
    if (inCallNumber > 0 && dispatch_flag == GSM) {
        LOGD("[requestImsDial] failed, CS call exist");
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    }
    setRequestFromImsIfNeeded(t);
    requestDial(data, datalen, t, isEmergency, isVt);
}

/**
* To add a new member(user) to existed conference call.
*
*/
void requestAddImsConferenceCallMember(void *data, RIL_Token t) {
    /*
       data contains the following informations,
       1. confCallId IMS(VoLTE) conference call id
       2. address The address(phone number or SIP URI)
       3. callIdToAdd The call id of call which is preparing to merge to current conference call.
    */
    const char**  strings = (const char**)data;
    int ret;
    ATResponse *p_response = NULL;
    char *line = NULL;
    int confCallId = atoi(strings[0]);
    int callIdToAdd = atoi(strings[2]);

    LOGD("[requestAddImsConferenceCallMember]%d, %s, %d", confCallId, "[hidden]", callIdToAdd);

    if (callIdToAdd > 0) {
        asprintf(&line, "AT+ECONF=%d,0,\"%s\",%d", confCallId, strings[1], callIdToAdd);
    }
    else {
        asprintf(&line, "AT+ECONF=%d,0,\"%s\"", confCallId, strings[1]);
    }
    // LOGD("[requestAddImsConferenceCallMember]AT command = %s", line);

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
    LOGD("[requestAddImsConferenceCallMember]Failed!");
}

/**
* To remove a member(user) from existed conference call.
*
*/
void requestRemoveImsConferenceCallMember(void *data, RIL_Token t) {
    /*
       data contains the following informations,
       1. confCallId IMS(VoLTE) conference call id
       2. address The address(phone number or SIP URI)
       3. callIdToRemove The call id of call which is preparing to remove from current conference call.
    */
    const char**  strings = (const char**)data;
    int ret;
    ATResponse *p_response = NULL;
    char *line = NULL;
    int confCallId = atoi(strings[0]);
    int callIdToRemove = atoi(strings[2]);

    LOGD("[requestRemoveImsConferenceCallMember]%d, %s, %d", confCallId, "[hidden]", callIdToRemove);

    asprintf(&line, "AT+ECONF=%d,1,\"%s\"", confCallId, strings[1]);
    // LOGD("[requestRemoveImsConferenceCallMember]AT command = %s", line);

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
    LOGD("[requestRemoveImsConferenceCallMember]Failed!");
}

/**
* To make call with SIP uri.
*
*/
void requestDialWithSipUri(void *data, RIL_Token t) {
    char *sipUri = (char*)data;
    char *cmd = NULL;
    int ret;
    ATResponse *p_response = NULL;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));

    // LOGD("[requestDialWithSipUri]SIP uri = %s", sipUri);

    if (checkAndSetActiveSocket(rid)) {
        rejectDial();
        goto error;
    }

    setRequestFromImsIfNeeded(t);

    if (strncmp(sipUri, "sip:", 4) == 0) {
        asprintf(&cmd, "AT+CDU=1,\"%s\"", sipUri);
    } else {
        asprintf(&cmd, "AT+CDU=1,\"sip:%s\"", sipUri);
    }

    ret = at_send_command(cmd, &p_response, CC_CHANNEL_CTX);
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
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

/**
* To hold the active call.
*
*/
void requestHoldCall(void *data, RIL_Token t) {
    int callId = ((int *)data)[0];
    char *cmd = NULL;
    int ret;
    ATResponse *p_response = NULL;
    RIL_Errno err = RIL_E_GENERIC_FAILURE;

    LOGD("[requestHoldCall]call id to hold = %d", callId);

    asprintf(&cmd, "AT+ECCTRL=%d,131", callId);
    ret = at_send_command(cmd, &p_response, CC_CHANNEL_CTX);
    free(cmd);

    if (ret < 0 || p_response == NULL || p_response->success == 0) {
        goto error;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    at_response_free(p_response);
    return;

error:
    if (NULL != p_response && p_response->success == 0) {
        switch (at_get_cme_error(p_response)) {
            case CME_HOLD_FAILED_CAUSED_BY_TERMINATED:
                // +CME ERROR: 1000
                RLOGD("requestHoldCall: CME_HOLD_FAILED_CAUSED_BY_TERMINATED");
                err =  RIL_E_OEM_ERROR_1; // OEM_ERROR_1: hold failed due to terminated
                break;
            default:
                RLOGD("requestHoldCall: errno is %d\n", at_get_cme_error(p_response));
                err = RIL_E_GENERIC_FAILURE;
                break;
        }
    }

    RIL_onRequestComplete(t, err, NULL, 0);
    at_response_free(p_response);
}

/**
* To resume held call.
*
*/
void requestResumeCall(void *data, RIL_Token t) {
    int callId = ((int *)data)[0];
    char *cmd = NULL;
    int ret;
    ATResponse *p_response = NULL;

    LOGD("[requestResumeCall]call id to resume = %d", callId);

    asprintf(&cmd, "AT+ECCTRL=%d,132", callId);
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

void onVoLteEconfSrvccIndication(char *urc, RIL_SOCKET_ID rid) {
    int i, ret;
    int response[6] = { 0, 0, 0, 0, 0, 0 };

    ret = at_tok_start(&urc);
    if (ret < 0) goto error;

    // get count of call ids
    ret = at_tok_nextint(&urc, &response[0]);
    if (ret < 0) goto error;

    if (response[0] > 5) {
        LOGE("[onVoLteEconfSrvccIndication]The count of call ids is greater than 5!");
    } else {
        for (i = 0; i < response[0]; i++) {
            ret = at_tok_nextint(&urc, &response[i+1]);
            if (ret < 0) {
                LOGD("[onVoLteEconfSrvccIndication]Can not get correct call id!");
                break;
            }
        }
        LOGD("Send RIL_UNSOL_ECONF_SRVCC_INDICATION");
        RIL_UNSOL_RESPONSE(RIL_UNSOL_ECONF_SRVCC_INDICATION, response, 6 * sizeof(int *), rid);
    }
    return;

error:
    LOGE("There is something wrong with the +ECONFSRVCC");
}

/**
 *  Send the result of "AT+ECONF" to framework.
 *  URC format: +ECONF:<conf_call_id>,<op>,<num>,<result>,<cause>[,<joined_call_id>]
 */
void onVoLteEconfResultIndication(char *urc, RIL_SOCKET_ID rid) {
    int i, ret;
    char *result[6];

    // LOGD("[onVoLteEconfResultIndication] urc = %s", urc);

    ret = at_tok_start(&urc);
    if (ret < 0) goto error;

    for (i = 0; i < 5; i++) {
        ret = at_tok_nextstr(&urc, &result[i]);
        if (ret < 0) {
            break;
        }
    }

    if (at_tok_hasmore(&urc)) {
        ret = at_tok_nextstr(&urc, &result[5]);
    }

    LOGD("IMS: Send RIL_UNSOL_ECONF_RESULT_INDICATION");
    RIL_UNSOL_RESPONSE(RIL_UNSOL_ECONF_RESULT_INDICATION, result, 6 * sizeof(char *), rid);
    return;

error:
    LOGE("There is something wrong with the +ECONFSRVCC");
}

void onSipCallProgressIndicator(char *urc, const RIL_SOCKET_ID rid) {
    /*
    * +ESIPCPI: <call_id>,<dir>,<SIP_msg_type>,<method>,<response_code>[,<reason_text>]
    * <call_id>: 0-incoming call, 1~32-call id
    * <SIP_msg_type>: 0-request, 1-response
    * <method>: 1~32 and mapping to INVITE, PRACK, UPDATE, ACK, CANCEL, BYE, REFER, K
    * <response_code>: 0-only used when SIP_msg_type is 0(request), else 100~600
    * <reason_text>: Optional, The text in the SIP response reason header.
    */

    int ret, i;
    char *data[7] = {NULL};
    char *sip_msg = 0;
    LOGD("onSipCallProgressIndicator:%s", urc);

    ret = at_tok_start(&urc);
    if (ret < 0) goto error;

    for (i = 0; i < 5; i++) {
        ret = at_tok_nextstr(&urc, &data[i]);
        if (ret < 0) goto error;
    }

    // For data[5], it is optional.
    if (at_tok_nextstr(&urc, &data[5]) < 0) {
        data[5] = "";
    }

    sip_msg = data[2];
    asprintf(&data[6], "%d", rid);
    LOGD("onSipCallProgressIndicator: sip_msg = %s ", sip_msg);

    /*Only cares about response sip msg*/
    RIL_UNSOL_RESPONSE(RIL_UNSOL_SIP_CALL_PROGRESS_INDICATOR, data, 7 * sizeof(char *), rid);
    return;

error:
    LOGE("error on onSipCallProgressIndicator");
}

void onAdsReport(char *urc, const RIL_SOCKET_ID rid) {
    /*
    * +EVADSREP: <domain>
    * <domain>: The ADS result for the MO voice call
    * 0: CS domain (2/3G CS Call)
    * 1: IMS domain (VoLTE)
    * 2: IMS domain (Wi-Fi Calling) <------- TBC in WFC task force
    */
    int ret = 0;
    int data = 0;
    LOGD("onAdsReport: ");

    ret = at_tok_start(&urc);
    if (ret < 0) goto error;

    ret = at_tok_nextint(&urc, &data);
    if (ret < 0) goto error;

    LOGD("onAdsReport: domain = %d", data);
    switch (data) {
        case 0:
            if (dispatch_flag == IMS) {
                int srvcc_start = 0;
                int srvcc_complete = 1;
                RIL_UNSOL_RESPONSE(RIL_UNSOL_SRVCC_STATE_NOTIFY, &srvcc_start, sizeof(int), rid);
                sleep(1);
                RIL_UNSOL_RESPONSE(RIL_UNSOL_SRVCC_STATE_NOTIFY, &srvcc_complete, sizeof(int), rid);
                RIL_UNSOL_RESPONSE(RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, NULL, 0, rid);
                dispatch_flag = GSM;
            }
            break;
        case 1:
            if (dispatch_flag == GSM) {
                LOGD("onAdsReport: Error, ADS report IMS but dispatch_flag == GSM");
            }
            dispatch_flag = IMS;

            break;
        default:
            LOGD("onAdsReport: domain type not support");
            break;
    }
    notifyImsCallExist(rid);
    return;

error:
    LOGE("error on onAdsReport");
}

void onCallModeChangeIndicator(char *urc, const RIL_SOCKET_ID rid)
{
    /*
    * +EIMSCMODE: <call id>,<call_mode>,<video_state>,<audio direction>,<PAU>
    * <call_id>:
    *      0-incoming call, 1~32-call id
    * <call_mode>:
    *      <20 for voice call, 20 for volte call, 21 for vilte call, 22 for volte conference call.
    * <video_state>:
    * <PAU>: P-Asserted-Identity
    *
    */

    int ret, i;
    char *data[5] = {NULL};

    // LOGD("onCallModeChangeIndicator:%s", urc);

    ret = at_tok_start(&urc);
    if (ret < 0) goto error;

    for (i = 0; i < 5; i++) {
        ret = at_tok_nextstr(&urc, &data[i]);
        if (ret < 0) goto error;
    }

    /// M: ALPS02519283 EIMSCMODE before CRING @{
    if (isRecvECPI0 && !hasReceivedRing) {
        LOGD("Receive EIMSCMODE before CRING");
        bool isCallIdEqual = true;
        if (setupCpiData[0] != NULL && data[0] != NULL &&
                strcmp(setupCpiData[0], data[0]) != 0) {
            isCallIdEqual = false;
        }

        if (isCallIdEqual && setupCpiData[5] != NULL && data[1] != NULL &&
                strcmp(setupCpiData[5], data[1]) != 0) {
            LOGD("Update call mode: %s", data[1]);
            strcpy(setupCpiData[5], data[1]);
        }
    }
    /// @}

    RIL_UNSOL_RESPONSE(RIL_UNSOL_CALLMOD_CHANGE_INDICATOR, data, 5 * sizeof(char *), rid);

    return;

error:
    LOGE("error on onCallModeChangeIndicator");
}

void onVideoCapabilityIndicator(char *urc, const RIL_SOCKET_ID rid)
{
    /*
    * +EIMSVCAP: <call ID>, <local video capability>, <remote video capability>
    * <call_id>:
    *      0-incoming call, 1~32-call id
    * <local video capability>:
    *      true: has video capability /false: no video capability.
    * <remote video capability>:
    *      true: has video capability /false: no video capability.
    *
    */

    int ret, i;
    char *data[3];

    LOGD("onVideoCapabilityIndicator:%s", urc);

    ret = at_tok_start(&urc);
    if (ret < 0) goto error;

    for (i = 0; i < 3; i++) {
        ret = at_tok_nextstr(&urc, &data[i]);
        if (ret < 0) goto error;
    }

    RIL_UNSOL_RESPONSE(RIL_UNSOL_VIDEO_CAPABILITY_INDICATOR, data, 3 * sizeof(char *), rid);

    return;

error:
    LOGE("error on onVideoCapabilityIndicator");
}

void requestApproveEccRedial(void *data, RIL_Token t)
{
    // No need to implement for 90/91 MD
    LOGD("[requestApproveEccRedial] no implementation for 90/91 MD");
    return;
}

void requestConferenceDial(void *data, RIL_Token t) {
    /*
     * data contains the following informations,
     * 1. dial method: 0: voice call  1:video call
     * 2. number of Participants
     * 3. Participants' dialing numbers
     * 4. clir mode.
     */
    if (data == NULL) {
        LOGD("[requestConferenceDial] data=NULL, return directly");
        return;
    }
    const char**  strings = (const char**)data;

    if (strings[0] == NULL || strings[1] == NULL) {
        LOGD("[requestConferenceDial] abnormal parameters, return directly");
        return;
    }
    int dialMethod = atoi(strings[0]); /* 0: voice call  1: video call*/
    int numberOfParticipants = atoi(strings[1]);

    if (strings[numberOfParticipants+2] == NULL) {
        LOGD("[requestConferenceDial] abnormal parameters, return directly");
        return;
    }
    int clirMode = atoi(strings[numberOfParticipants+2]);

    int ret;
    ATResponse *p_response = NULL;
    char *line = NULL;
    char *old_line = NULL;
    const char *clir;
    int i = 0;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));

    LOGD("[requestConferenceDial]%d, %d, %d", dialMethod, numberOfParticipants, clirMode);

    if (checkAndSetActiveSocket(rid)) {
        rejectDial();
        goto error;
    }

    setRequestFromImsIfNeeded(t);

    switch (clirMode) {
        case 1: /*invocation*/
        {
            clir = "#31#";
            break;
        }
        case 2: /*suppression*/
        {
            clir = "*31#";
            break;
        }
        case 0:
        default: /*subscription default*/
        {
            clir = "";
            break;
        }
    }

    // IMS VoLTE refactoring
    LOGE("IMS: requestConferenceDial from IMS !!");
    ret = at_send_command("AT+EVADSMOD=0", &p_response, CC_CHANNEL_CTX);
    if (ret < 0 || p_response == NULL || p_response->success == 0) {
        LOGE("IMS: AT+EVADSMOD=0 Fail !!");
    }
    dispatch_flag = IMS;
    notifyImsCallExist(rid);

    at_response_free(p_response);
    p_response = NULL;
    if (numberOfParticipants == 0) {
        LOGE("PCT TC_15.17/TC_15.19a: launch an empty one-key conference");
        asprintf(&line, "AT+EDCONF=%d,0", dialMethod);
    } else {
        asprintf(&line, "AT+EDCONF=%d,%d,", dialMethod, numberOfParticipants);
        for (i = 2; i < numberOfParticipants + 1; i++) {
            old_line = line;
            asprintf(&line, "%s\"%s%s\",", old_line, clir, strings[i]);
            free(old_line);
        }
        old_line = line;
        asprintf(&line, "%s\"%s%s\"", old_line, clir, strings[i]);
        free(old_line);
    }
    /// M: ALPS02338098 @{
    /// Fix coverity 67183 "Forward null check"
    /// Do not send AT-cmd if allocate buffer failed.
    /// Goto error handler to set request complete as GENERIC_FAILURE.
    if (line == NULL) {
        LOGD("[requestConferenceDial] failed to allocate buffer for AT command");
        goto error;
    }
    // LOGD("[requestConferenceDial] AT command = %s", line);
    /// @}
    ret = at_send_command(line, &p_response, CC_CHANNEL_CTX);
    free(line);

    if (ret < 0 || p_response == NULL || p_response->success == 0) {
        goto error;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    at_response_free(p_response);
    return;

error:
    if (call_state == pending_mo) {
        LOGE("IMS: requestConferenceDial fail !!");
        call_state = none;
        if (inCallNumber == 0) {
            dispatch_flag = GSM;
            notifyImsCallExist(rid);
        }
    }
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
    LOGD("[requestConferenceDial]Failed!");
}

void requestVideoAccept(void *data, RIL_Token t)
{
    int *p_line;
    char *cmd = NULL;

    p_line = (int *)data;

    if(p_line[0] == 0) {
        /* We use ATA to accept the video call as video call */
        at_send_command("ATA", NULL, CC_CHANNEL_CTX);
    } else {
        /* We use AT+EVTA command to accept the video call as audio call, video_rx or video_tx.
         * AT+EVTA=<mode>,<call id>
         * Mode  =1 , accept as audio
         * Mode  =2 , accept as one way only video (Rx)
         * Mode  =3 , accept as one way only video (Tx)
         */
        asprintf(&cmd, "AT+EVTA=%d,%d", p_line[0],p_line[1]);
        at_send_command(cmd, NULL, CC_CHANNEL_CTX);
        free(cmd);
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
}

/**
* To make a video call with SIP uri.
*
*/
void requestVtDialWithSipUri(void *data, RIL_Token t) {
    char *sipUri = (char*)data;
    char *cmd = NULL;
    int ret;
    ATResponse *p_response = NULL;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));

    // LOGD("[requestVtDialWithSipUri]SIP uri = %s", sipUri);

    if (checkAndSetActiveSocket(rid)) {
        rejectDial();
        goto error;
    }

    setRequestFromImsIfNeeded(t);

    if (strncmp(sipUri, "sip:", 4) == 0) {
        asprintf(&cmd, "AT+CDU=1,\"%s\",1,2", sipUri);
    } else {
        asprintf(&cmd, "AT+CDU=1,\"sip:%s\",1,2", sipUri);
    }

    ret = at_send_command(cmd, &p_response, CC_CHANNEL_CTX);
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
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

/**
 *  Send the result of "AT+CHLD=4" to framework.
 *  URC format: +ECT: <call_id>, <ect_result>, <cause>
 */
void onImsEctResultIndication(char *urc, RIL_SOCKET_ID rid) {
    int i, err;
    int numofElements=3;
    int response[numofElements];

    LOGD("[onImsEctResultIndication] urc = %s", urc);

    err = at_tok_start(&urc);
    if (err < 0) goto error;

    for (i = 0; i < numofElements; i++) {
        err = at_tok_nextint(&urc, &response[i]);
        if (err < 0) goto error;
    }

    LOGD("IMS: Send RIL_UNSOL_ECT_INDICATION");
    RIL_UNSOL_RESPONSE(RIL_UNSOL_ECT_INDICATION, response, sizeof(response), rid);
    return;

error:
    LOGE("There is something wrong with the +ECT");
}

/**
* IMS assured/blind ECT.
*
*/
void requestImsEct(void *data, size_t datalen, RIL_Token t) {
    const char**  strings = (const char**)data;
    char *cmd = NULL;
    int ret;
    ATResponse *p_response = NULL;

    if (datalen < 2*sizeof(char*)) {
        goto error;
    }
    const char *number = strings[0];
    const char *type = strings[1];
    LOGD("[requestImsEct] ect type = %s, target number = %s", type, "[hidden]");

    asprintf(&cmd, "AT+ECT=%s,\"%s\"", type, number);

    ret = at_send_command(cmd, &p_response, CC_CHANNEL_CTX);
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
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

/**
 * To pull an external call.
 *
 */
void requestPullCall(void *data, RIL_Token t) {
    const char**  strings = (const char**)data;
    const char *targetUri = strings[0];
    const char *callMode = strings[1];
    char *cmd = NULL;
    int ret;
    ATResponse *p_response = NULL;
    // get rid
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));

    LOGD("[requestPullCall] target uri = %s, call mode = %s", "[hidden]", callMode);

    if (activeRilSktID != -1 && activeRilSktID != (int)rid) {
        goto error;
    }
    dispatch_flag = IMS;
    notifyImsCallExist(rid);

    asprintf(&cmd, "AT+ECALLPULL=\"%s\",%s", targetUri, callMode);

    ret = at_send_command(cmd, &p_response, CC_CHANNEL_CTX);
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
        dispatch_flag = GSM;
        notifyImsCallExist(rid);
    }
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

/**
 * Set voice domain preference to modem
 *
 * AT+CEVDP=<setting>
 * setting: integer
 *  1:  CS voice only
 *  2:  CS voice preferred, IMS PS voice as secondary
 *  3:  IMS PS voice preferred, CS voice as secondary
 *  4:  IMS PS voice only
 */
void requestSetVoiceDomainPreference(void *data, size_t datalen, RIL_Token t) {
    int ret;
    int vdpSetting = ((int *)data)[0];
    char cmd[255];
    ATResponse *p_response = NULL;
    sprintf(cmd, "AT+CEVDP=%d", vdpSetting);
    LOGD("[requestSetVoiceDomainPreference]cmd = %s", cmd);
    ret = at_send_command(cmd, &p_response, CC_CHANNEL_CTX);
    if (ret < 0 || p_response->success == 0) {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }
    at_response_free(p_response);
    return;
}

void requestSetE911State(void *data, size_t datalen, RIL_Token t) {
    /**
      E911_ENTER = 0,           // Emergency call start
      E911_EXIT_WITH_ECBM = 1,  // Emergency call end and enter ECBM
      E911_EXIT = 2,            // Emergency call end
      E911_ENTER_ECBM = 3       // Enter ECBM
     **/
    int state = ((int *)data)[0];
    int ret = 0;
    int mode = 0;
    // Check whether second parameter(RAT) exist.
    // RAT not exist => LTE E911 state update, set RAT = 4(LTE) directly.
    int rat = (datalen > (sizeof(int))) ? ((int *)data)[1] : 4;
    LOGD("requestSetE911State: %d, rat: %d", state, rat);
    switch (state) {
        case 0:
            RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
            return;
        case 1:
            mode = 1;
            break;
        case 2:
            mode = 0;
            break;
        case 3:
            mode = 1;
            break;
        default:
            RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
            return;
    }
    char cmd[255];
    ATResponse *p_response = NULL;
    sprintf(cmd, "AT+ECBM=%d,%d", mode, rat);
    LOGD("[requestSetE911State]cmd = %s", cmd);
    ret = at_send_command(cmd, &p_response, CC_CHANNEL_CTX);
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
 * Handle for SRVCC handover event
 *
 * +CIREPH: <srvcc_state>
 *    srvcc_state=0 : start
 *    srvcc_state=1 : completed
 */
void onSrvccStateNotify(char *urc, RIL_SOCKET_ID rid) {
    int ret;
    int info = 0;

    LOGD("onSrvccStateNotify:%s", urc);

    if (strStartsWith(urc, "+CIREPH")) {
        ret = at_tok_start(&urc);
        if (ret < 0) goto error;

        //
        ret = at_tok_nextint(&urc, &info);
        if (ret < 0) goto error;

    } else {
        goto error;
    }

    LOGD("onSrvccStateNotify srvcch = %d, ", info);
    RIL_UNSOL_RESPONSE(RIL_UNSOL_SRVCC_STATE_NOTIFY, &info, sizeof(int), rid);
    if (info == 1) {
        RIL_UNSOL_RESPONSE(RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, NULL, 0, rid);
        dispatch_flag = GSM;
        notifyImsCallExist(rid);
    }

    return;

error:
    LOGE("error on onSrvccStateNotify");
}

void notifyImsCallExist(RIL_SOCKET_ID rid) {
    int ims_call_exist = 0;
    if (dispatch_flag == IMS) {
        ims_call_exist = 1;
    }
    RIL_UNSOL_RESPONSE(RIL_LOCAL_GSM_UNSOL_IMS_CALL_EXIST, &ims_call_exist, sizeof(int), rid);
}

void setRequestFromImsIfNeeded(RIL_Token t) {
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    RLOGD("IMS: setRequestFromImsIfNeeded socket id is %d", rid);
    dispatch_flag = IMS;
    call_state = pending_mo;
}

bool hasImsPendingMO() {
    if (dispatch_flag == IMS && (call_state == pending_mo || call_state == mo)) {
        return true;
    }
    return false;
}

extern int rilImsCcMain(int request, void *data, size_t datalen, RIL_Token t) {
    switch (request) {
    case RIL_REQUEST_IMS_DIAL:
        requestImsDial(data, datalen, t, 0, 0);
        break;
    case RIL_REQUEST_IMS_VT_DIAL:
        requestImsDial(data, datalen, t, 0, 1);
        break;
    case RIL_LOCAL_REQUEST_IMS_EMERGENCY_DIAL:
        requestImsDial(data, datalen, t, 1, 0);
        break;
    case RIL_REQUEST_ADD_IMS_CONFERENCE_CALL_MEMBER:
        requestAddImsConferenceCallMember(data, t);
        break;
    case RIL_REQUEST_REMOVE_IMS_CONFERENCE_CALL_MEMBER:
        requestRemoveImsConferenceCallMember(data, t);
        break;
    case RIL_REQUEST_DIAL_WITH_SIP_URI:
        requestDialWithSipUri(data, t);
        break;
    case RIL_REQUEST_HOLD_CALL:
        requestHoldCall(data, t);
        break;
    case RIL_REQUEST_RESUME_CALL:
        requestResumeCall(data, t);
        break;
    case RIL_REQUEST_CONFERENCE_DIAL:
        requestConferenceDial(data, t);
        break;
    case RIL_REQUEST_VIDEO_CALL_ACCEPT:
        requestVideoAccept(data, t);
        break;
    case RIL_REQUEST_ECC_REDIAL_APPROVE:
        requestApproveEccRedial(data, t);
        break;
    case RIL_REQUEST_VT_DIAL_WITH_SIP_URI:
        requestVtDialWithSipUri(data, t);
        break;
    case RIL_REQUEST_IMS_ECT:
        requestImsEct(data, datalen, t);
        break;
    case RIL_REQUEST_PULL_CALL:
        requestPullCall(data, t);
        break;
    case RIL_REQUEST_SET_VOICE_DOMAIN_PREFERENCE:
        requestSetVoiceDomainPreference(data, datalen, t);
        break;
    case RIL_REQUEST_SET_E911_STATE:
        requestSetE911State(data, datalen, t);
        break;
    default:
        return 0; /* no matched request */
        break;
    }

    return 1; /* request found and handled */
}

extern int rilImsCcUnsolicited(const char *s, RILChannelCtx *p_channel) {
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);

    if (strStartsWith(s, "+ECONFSRVCC")) {
        onVoLteEconfSrvccIndication((char *)s, rid);
        return 1;
    } else if (strStartsWith(s, "+ECONFCHECK")) {
        /* +ECONFCHECK: */
        return 1;
    } else if (strStartsWith(s, "+ECONF")) {
        /* +ECONF:<conf_call_id>,<op>,<num>,<result>,<cause>[,<joined_call_id>] */
        onVoLteEconfResultIndication((char *)s, rid);
        return 1;
    } else if (strStartsWith(s, "+CIREPH")) {
        /* Provides SRVCC and vSRVCC handover information */
        onSrvccStateNotify((char*)s, rid);
        return 1;
    } else if (strStartsWith(s, "+ESIPCPI")) {
        onSipCallProgressIndicator((char*)s, rid);
        return 1;
    } else if (strStartsWith(s, "+EVADSREP")) {
        onAdsReport((char*)s, rid);
        return 1;
    } else if (strStartsWith(s, "+EIMSCMODE")) {
        onCallModeChangeIndicator((char*)s, rid);
        return 1;
    } else if (strStartsWith(s, "+EIMSVCAP")) {
        onVideoCapabilityIndicator((char*)s, rid);
        return 1;
    } else if (strStartsWith(s,"+ECT:")) {
        /* +CSSU is MT */
        onImsEctResultIndication((char *)s, rid);
        return 1;
    }

    return 0;
}
