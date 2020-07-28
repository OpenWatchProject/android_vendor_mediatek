/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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
#include "ril_c2k_sms.h"
#include "ril_callbacks.h"
#include <assert.h>

/*****************************************************************************
 * Function Declarations
 *****************************************************************************/
extern int strStartsWith(const char *line, const char *prefix);

/*****************************************************************************
 * Static Functions
 *****************************************************************************/
static void RequestLocalRilProxyToGsm(void *data, size_t datalen, RIL_Token t) {
    const char *user_data = NULL;
    const char *cmd = NULL;
    const char *cmd_wait_rsp = NULL;
    char **cmd_rsp = NULL;;
    int err = 0;
    ATResponse *p_response = NULL;
    int len = 0;
    CDMA_SMS_UNUSED(datalen);
    // user data of rilproxy
    user_data = ((const char **)data)[0];
    // AT commands send to gsm modem
    cmd = ((const char **)data)[1];
    // What's kind of at commands to wait the response
    cmd_wait_rsp = ((const char **)data)[2];

    assert(user_data != NULL);
    LOGD("localRequestCdmaSms(): user_data=%s", user_data);

    if (cmd == NULL) goto error;
    LOGD("localRequestCdmaSms(): cmd=%s", cmd);

    if (cmd_wait_rsp != NULL) {
        LOGD("localRequestCdmaSms(): cmd_wait_rsp=%s", cmd_wait_rsp);
    }

    if (cmd_wait_rsp == NULL || strcmp("", cmd_wait_rsp) == 0) {
        // No need to wait any special response
        err = at_send_command(cmd, &p_response, SMS_CHANNEL_CTX);
    } else {
        err = at_send_command_singleline(cmd, cmd_wait_rsp, &p_response, SMS_CHANNEL_CTX);
    }
    if (err < 0) goto error;

    // Response at commands from gsm modem
    cmd_rsp = (char **) calloc(2, sizeof(char*));  // allocate 2 char* memory
    if (cmd_rsp == NULL) {
        goto error;
    }
    cmd_rsp[0] = strdup(user_data);
    assert(cmd_rsp[0] != NULL);
    if (strcmp("", cmd_wait_rsp) == 0) {
        cmd_rsp[1] = strdup(p_response->finalResponse);
    } else {
        if (p_response->success == 1) {
            cmd_rsp[1] = strdup(p_response->p_intermediates->line);
        } else {
            cmd_rsp[1] = strdup(p_response->finalResponse);
        }
    }
    assert(cmd_rsp[1] != NULL);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, cmd_rsp, sizeof(char*)*2);
    free(cmd_rsp[0]);
    free(cmd_rsp[1]);
    free(cmd_rsp);
    at_response_free(p_response);
    return;

error:
    // Occur error but we still need to put back the user data
    cmd_rsp = (char **) calloc(2, sizeof(char*));  // allocate 2 char* memory
    assert(cmd_rsp != NULL);
    cmd_rsp[0] = strdup(user_data);
    assert(cmd_rsp[0] != NULL);
    cmd_rsp[1] = NULL;
    if (p_response != NULL) {
        cmd_rsp[1] = strdup(p_response->finalResponse);
        assert(cmd_rsp[1] != NULL);
    }
    len = sizeof(char*) * 2;

    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, cmd_rsp, len);
    free(cmd_rsp[0]);
    if (cmd_rsp[1] != NULL) {
        free(cmd_rsp[1]);
    }
    free(cmd_rsp);
    if (p_response != NULL) {
        at_response_free(p_response);
    }
    return;
}

static void onGsmUrcToRilProxy(const char *smspdu, RIL_SOCKET_ID rid) {
    RIL_onUnsolicitedResponse(RIL_LOCAL_GSM_UNSOL_CDMA_SMS_SPECIFIC_FROM_GSM, smspdu,
            strlen(smspdu)+1, rid);
}

/*****************************************************************************
 * External Functions
 *****************************************************************************/
int rilC2kSmsMain(int request, void *data, size_t datalen, RIL_Token t) {
    switch (request) {
        case RIL_LOCAL_REQUEST_CDMA_SMS_SPECIFIC_TO_GSM:
            RequestLocalRilProxyToGsm(data, datalen, t);
            break;

        default:
            return 0;
    }
    return 1;
}

int rilC2kSmsUnsolicited(const char *s, const char *sms_pdu, RILChannelCtx* p_channel) {
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);
    if (strStartsWith(s, "+C2KCMT:")) {
        onGsmUrcToRilProxy(sms_pdu, rid);
    } else {
        return 0;
    }
    return 1;
}
