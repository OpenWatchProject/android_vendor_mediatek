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
#include "sms.h"
#include "at_tok.h"
#include "misc.h"
#include "atchannel.h"
#include <libmtkrilutils.h>

#define SMS_CHANNEL_CTX getRILChannelCtxFromToken(t)
#define MAX_ADDRES_LEN  (RIL_CDMA_SMS_ADDRESS_MAX + 4)
#define MAX_STORE_TYPE_LEN  4

typedef struct{
    int sms_index;
    char sms_storeType[MAX_STORE_TYPE_LEN + 1];
} NewMsgStorageStruct;

static int sReplyOptSeqNo = -1;
static unsigned char ackAddress[MAX_ADDRES_LEN];
static const struct timeval TIMEVAL_0 = { 0, 0 };

static void requestCdmaWriteSmsToRuim(void *data, size_t datalen, RIL_Token t);
static void oemDispatchNewCmdaSMS(const char *sms_pdu);
static void oemRequestSendCdmaSMS(void *data, size_t datalen, RIL_Token t);
static void requesCdmatSMSAcknowledge(void *data, size_t datalen, RIL_Token t);
static void requestGetSmsSimMemStatus(void *data, size_t datalen, RIL_Token t);
static int getSmsSimMemStatus(int *totalMem, int *usedMem);
static void requestReadDelNewSms(void* param);
static void requestReportSmsMemoryStatus(void *data, size_t datalen, RIL_Token t);
static void readAllUnreadSmsOnME(void);
static void requestCdmaSmsBroadcastActivation(void *data, size_t datalen, RIL_Token t);
static void requestCdmaSetBroadcastSmsConfig(void *data, size_t datalen, RIL_Token t);
static void requestCdmaGetBroadcastSmsConfig(void *data, size_t datalen, RIL_Token t);
static int queryCdmaBroadcastActivation(int *mode, RIL_Token t);
static int queryCdmaBroadcastConfig(char *cmd, char* responsePrefix,
        int *mode, char **config, RIL_Token t);
static int queryCdmaSmsBroadcastCatConfig(int *mode, char **category, RIL_Token t);
static int queryCdmaSmsBroadcastLanConfig(int *mode, char **language, RIL_Token t);
static int disableRange(int mode, char *rangeString, char* cmd, RIL_Token t);
static int disableAllCategory(RIL_Token t);
static int disableAllLanguage(RIL_Token t);
static int sortCategoryAndLanguage(
        RIL_CDMA_BroadcastSmsConfigInfo **infos,
        int *category,
        int *language,
        int count,
        int selected);
static int getConfig(RIL_CDMA_BroadcastSmsConfigInfo **infos, int *count,
        int startCheck, int endCheck, char *cmd, char* responsePrefix, RIL_Token t);
static int getCdmaBroadcastCategoryConfig(RIL_CDMA_BroadcastSmsConfigInfo **infos,
        int *count, RIL_Token t);
static int getCdmaBroadcastLanguageConfig(RIL_CDMA_BroadcastSmsConfigInfo **infos,
        int *count, RIL_Token t);

extern RILChannelCtx *getChannelCtxbyProxy();

void rilSmsOnSmsReady(void *arg)
{
    SMS_UNUSED(arg);
    /*  SMS PDU mode */
    at_send_command("AT+CMGF=0", NULL, getChannelCtxbyProxy());
    if (isSvlteSupport() == 1) {
        at_send_command("AT+CNMI=1,1,0,2,0", NULL, getChannelCtxbyProxy());
        readAllUnreadSmsOnME();
    } else {
        at_send_command("AT+CNMI=1,2,0,2,0", NULL, getChannelCtxbyProxy());
    }
}

int rilSmsMain(int request, void *data, size_t datalen, RIL_Token t)
{
    ATResponse *p_response;
    int err = 0;
    // CDMA broadcast config is not supported in SVLTE platform
    if (isSvlteSupport() == 1) {
         if (request == RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION ||
             request == RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG ||
             request == RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG) {
             return 0;
         }
    }

    switch (request) {

        case RIL_REQUEST_CDMA_SEND_SMS:
            oemRequestSendCdmaSMS(data, datalen, t);
            break;

        case RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE:
        case RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE_EX:
            requesCdmatSMSAcknowledge(data, datalen, t);
            break;

        case RIL_REQUEST_CDMA_WRITE_SMS_TO_RUIM:
            requestCdmaWriteSmsToRuim(data, datalen, t);
            break;

        case RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM: {
            char * cmd = NULL;
            int index = 0;
            p_response = NULL;
            err = -1;

            index = ((int *)data)[0] - 1;
            if ( index >= 0) {
                at_send_command("AT+CPMS=\"SM\"", NULL, SMS_CHANNEL_CTX);
                asprintf(&cmd, "AT+CMGD=%d", ((int *)data)[0] - 1);
                if (cmd == NULL) {
                    break;
                }
                err = at_send_command(cmd, &p_response, SMS_CHANNEL_CTX);
                at_send_command("AT+CPMS=\"ME\"", NULL, SMS_CHANNEL_CTX);
                free(cmd);
            } else if (((int *)data)[0] == -1) {
              at_send_command("AT+CPMS=\"SM\"", NULL, SMS_CHANNEL_CTX);
              err = at_send_command("AT+CMGD=,4", &p_response, SMS_CHANNEL_CTX);
              at_send_command("AT+CPMS=\"ME\"", NULL, SMS_CHANNEL_CTX);
            }

            if (err < 0 || p_response == NULL || p_response->success == 0) {
                RIL_onRequestComplete(t, RIL_E_SYSTEM_ERR, NULL, 0);
            } else {
                RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
            }
            at_response_free(p_response);
            break;
        }

        case RIL_REQUEST_GET_SMS_RUIM_MEM_STATUS:
            requestGetSmsSimMemStatus(data, datalen, t);
            break;

        case RIL_REQUEST_REPORT_SMS_MEMORY_STATUS:
            requestReportSmsMemoryStatus(data, datalen, t);
            break;

        case RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION:
            requestCdmaSmsBroadcastActivation(data, datalen, t);
            break;

        case RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG:
            requestCdmaSetBroadcastSmsConfig(data, datalen, t);
            break;

        case RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG:
            requestCdmaGetBroadcastSmsConfig(data, datalen, t);
            break;

        default:
            return 0;  /* no matched request */
    }

    return 1;
}

static int findPduPosition(const char *cmd)
{
    const char *p = cmd;
    int comma = -1;
    int prevQuote = -1;
    int quote = -1;
    int i;
    if (cmd == NULL) {
        return -1;
    }

    for (i = 0; p[i] != '\0'; i++) {
        if (p[i] == ',') {
            comma = i;
        }
        if (p[i] == '\"') {
            prevQuote = quote;
            quote = i;
        }
    }
    if (comma > 0 && prevQuote > comma && quote > prevQuote) {
        int j;
        if (p[quote + 1] != '\0') {
            return -1;
        }
        for (j = comma + 1; j < prevQuote; j++) {
            if (p[j] != ' ') {
                return -1;
            }
        }
        return prevQuote;
    }
    return -1;
}

int rilSmsUnsolicited(const char *s, const char *sms_pdu)
{
    char *line = NULL;
    int err = 0;
    SMS_UNUSED(sms_pdu);
    if (strStartsWith(s, "^SMMEMFULL:")) {
        char *memType = NULL;
        char *dup = strdup(s);
        if (!dup) {
            return 1;
        }
        line = dup;
        err = at_tok_start(&line);
        if (err < 0) {
            free(dup);
            return 1;
        }
        err = at_tok_nextstr(&line, &memType);
        if (err < 0) {
            free(dup);
            return 1;
        }
        if (!strncmp(memType, "SM", 2)) {
            RIL_onUnsolicitedResponse(RIL_UNSOL_CDMA_RUIM_SMS_STORAGE_FULL, NULL, 0);
        } else if (!strncmp(memType, "ME", 2)) {
            RIL_onUnsolicitedResponse(RIL_UNSOL_ME_SMS_STORAGE_FULL, NULL, 0);
        }
        free(dup);
        return 1;
    } else if (strStartsWith(s, "+CMT:")) {
        /**
         * +CMT format:
         * +CMT:<mem>,<index>,<length>,<pdu>
         * or
         * +CMT:<length>,<pdu>
         */
        int pos = findPduPosition(s);
        if (pos < 0) {
            LOGD("Cannot find SMS PDU");
            return 1;
        }
        oemDispatchNewCmdaSMS(&s[pos]);
        return 1;
    } else if (strStartsWith(s, "+CMGR:")) {
        oemDispatchNewCmdaSMS(sms_pdu);
        return 1;
    } else if (strStartsWith(s, "+CVMI:")) {
        char *dup = strdup(s);
        int num = 0;
        char pdu[64];
        memset(pdu, '\0', 64);
        if (dup == NULL) {
            return 1;
        }
        line = dup;
        err = at_tok_start(&line);
        if (err < 0) {
            free(dup);
            return 1;
        }
        err = at_tok_nextint(&line, &num);
        free(dup);
        if (err < 0) {
            LOGD("Handle +CVMI error!");
            return 1;
        }
        LOGD("Handle +CVMI: num = 0x%x", num);
        if (num > 99) {
            num = 99;
        }
        // to bcd code
        num = ((num / 10) << 4) | (num % 10);
        sprintf(pdu, "000002100302020040080c0003100000010210000B01%02x", num);
        LOGD("%s", pdu);
        oemDispatchNewCmdaSMS(pdu);
        return 1;
    } else if (strStartsWith(s, "+CMTI") || strStartsWith(s, "+CBMI")) {
        /**
         * +CMTI  format:
         * +CMTI: <mem>,<index>
         */
        char *storeType = NULL;

        char* dup = strdup(s);
        if (!dup) {
            return 1;
        }
        line = dup;
        err = at_tok_start(&line);
        if (err < 0) {
            free(dup);
            return 1;
        }
        /** We can ignore storeType because SM is default storage type*/
        err = at_tok_nextstr(&line, &storeType);
        if (err < 0) {
            free(dup);
            return 1;  // error at command format
        }
        NewMsgStorageStruct *sms_newMsgStore;
        sms_newMsgStore = (NewMsgStorageStruct *) malloc(sizeof(NewMsgStorageStruct));
        assert(sms_newMsgStore != NULL);
        memset(sms_newMsgStore, 0, sizeof(NewMsgStorageStruct));
        strncpy(sms_newMsgStore->sms_storeType, storeType, MAX_STORE_TYPE_LEN);
        err = at_tok_nextint(&line, &(sms_newMsgStore->sms_index));
        if (err < 0) {
            free(sms_newMsgStore);
            free(dup);
            return 1;  // error at command format
        }

        RIL_requestProxyTimedCallback(requestReadDelNewSms,
                (void*) sms_newMsgStore, NULL, SMS_CHANNEL);
        free(dup);
        return 1;
    } else if (strStartsWith(s, "+ECARDESNME")) {
        char *dup = strdup(s);
        if (dup == NULL) {
            return 1;
        }
        line = dup;
        err = at_tok_start(&line);
        if (err < 0) {
            free(dup);
            return 1;
        }
        RIL_onUnsolicitedResponse(RIL_UNSOL_CDMA_CARD_INITIAL_ESN_OR_MEID, line, strlen(line));
        free(dup);
        return 1;
    } else if (strStartsWith(s, "+CIEV:")) {
        int ind;
        int val;
        char *dup = strdup(s);
        if (!dup) {
            return 1;
        }
        line = dup;
        err = at_tok_start(&line);
        if (err < 0) {
            free(dup);
            return 1;
        }
        err = at_tok_nextint(&line, &ind);
        if (err < 0) {
            free(dup);
            return 1;
        }
        err = at_tok_nextint(&line, &val);
        if (err < 0) {
            free(dup);
            return 1;
        }
        /* <ind> 101: SMS ready/unready */
        if (ind == 101) {
            if (val == 1) {
                RIL_onUnsolicitedResponse(RIL_UNSOL_SMS_READY_NOTIFICATION, NULL, 0);
                RIL_requestProxyTimedCallback(rilSmsOnSmsReady,
                        NULL,
                        &TIMEVAL_0, SMS_CHANNEL);
            }
        }
        free(dup);
    }
    return 0;
}


/* Liqi: Apollo for writing SMS into UIM*/
static void requestCdmaWriteSmsToRuim(void *data, size_t datalen, RIL_Token t)
{
    // char * s_pdu = (char *)data;
    char *cmd = NULL, *line = NULL;
    char *storage = NULL;
    int err = 0, response = 0;
    ATResponse *p_response = NULL;
    RIL_CDMA_SMS_WriteArgs* p_args = (RIL_CDMA_SMS_WriteArgs *)data;

    char* s_pdu = NULL;
    char* s_number = NULL;

    char *indexStr = NULL;

    SMS_UNUSED(datalen);
    /* funcs malloced memory for s_pdu & s_number should free at end of these piece of code */
    err = RILEncodeCdmaSmsPdu(&(p_args->message), &s_pdu, &s_number);

    if (err < 0) {
        goto end;
    }
    at_send_command("AT+CPMS=\"ME\", \"SM\"", NULL, SMS_CHANNEL_CTX);

    asprintf(&cmd, "AT+CMGW=\"0\",\"%s\",%d" , s_pdu, p_args->status);
    if (cmd == NULL) {
        goto end;
    }
    err = at_send_command_singleline(cmd, "+CMGW:", &p_response, SMS_CHANNEL_CTX);

    if (err != 0 || p_response == NULL || p_response->success == 0) {
        err = -1;
        goto end;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto end;

    err = at_tok_nextstr(&line, &storage);
    if (err < 0) goto end;

    err = at_tok_nextint(&line, &response);
    if (err < 0) goto end;

end:
    at_send_command("AT+CPMS=\"ME\", \"ME\"", NULL, SMS_CHANNEL_CTX);

    if (!err) {
        LOGD("[CDMA SMS] write sms into UIM which index=%d", response);

        // transfer int index to string
        /*asprintf(&indexStr, "%d" , response);
        result[0] = indexStr;
        result[1] = s_pdu;*/
        RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(response));

    } else {
        RIL_onRequestComplete(t, RIL_E_SYSTEM_ERR, NULL, 0);
    }

    at_response_free(p_response);

    if(indexStr != NULL) {
        free(indexStr);
        indexStr = NULL;
    }

    if (s_pdu)
        free(s_pdu);

    if (s_number)
        free(s_number);

    if (cmd)
        free(cmd);
}


static void oemRequestSendCdmaSMS(void *data, size_t datalen, RIL_Token t)
{
    int err = 0;
    int err_class = -1;
    ATResponse *p_response = NULL;
    RIL_SMS_Response response;
    RIL_CDMA_SMS_Message* p_message = (RIL_CDMA_SMS_Message*)data;
    char* cmd = NULL;
    char* s_pdu = NULL;
    char* s_number = NULL;
    SMS_UNUSED(datalen);
    /* funcs malloced memory for s_pdu & s_number should free at end of these piece of code */
    err = RILEncodeCdmaSmsPdu(p_message, &s_pdu, &s_number);
    if (err < 0) {
        goto error;
    }

    at_send_command("at+cmgf = 0", NULL, SMS_CHANNEL_CTX);

    asprintf(&cmd, "AT+CMGS=\"0\", \"%s\"", s_pdu);
    if (cmd == NULL) {
        goto error;
    }
    // err = at_send_command_notimeout(cmd, &p_response);
    /* Use a big enough value to make sure we can get the HCMGSS/HCMGSF after CMGS */
    err = at_send_command_with_specified_timeout(cmd, MAX_TIMEOUTSECS_TO_RECEIVE_HCMGSS,
            &p_response, SMS_CHANNEL_CTX);
    free(cmd);
    if (s_pdu != NULL) {
        free(s_pdu);
        s_pdu = NULL;
    }
    if (s_number != NULL) {
        free(s_number);
        s_number = NULL;
    }

    memset(&response, 0, sizeof(response));
    if (err < 0 || p_response == NULL || p_response->success == 0) {
        goto error;
    }

    char* dup = strdup(p_response->finalResponse);
    if (!dup) goto error;

    char* respline = dup;
    if (strStartsWith(respline, "^HCMGSF:")) {
        err = at_tok_start(&respline);
        if (err < 0) {
            free(dup);
            goto error;
        }

        err = at_tok_nextint(&respline, &(response.errorCode));
        if (err < 0) {
            free(dup);
            goto error;
        }
        err = at_tok_nextint(&respline, &err_class);
        // In different modem version, it may not contains the error class variable and cause the
        // error happening. We will not trigger to error case but only returns the genereic error
        // back to sms frameworks.
        RIL_onRequestComplete(t, RIL_E_SYSTEM_ERR, &response, sizeof(response));
    }
    else
    {
        response.messageRef = atoi(respline + 8);
        RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(response));
    }

    free(dup);
    at_response_free(p_response);
    return;

error:
    if (s_pdu != NULL) free(s_pdu);
    if (s_number != NULL) free(s_number);
    int rilError = RIL_E_SYSTEM_ERR;
    if ((p_response != NULL) &&
            strStartsWith(p_response->finalResponse, "+CMS ERROR:")) {
        err = at_tok_start(&p_response->finalResponse);
        if (err == 0) {
            err = at_tok_nextint(&p_response->finalResponse, &response.errorCode);
            if (err == 0 && response.errorCode == 518) {
                rilError = RIL_E_FDN_CHECK_FAILURE;
            }
        }
    }
    RIL_onRequestComplete(t, rilError, &response, sizeof(response));
    at_response_free(p_response);
}

static void oemDispatchNewCmdaSMS(const char *sms_pdu)
{
    char* dup  = strdup(sms_pdu);
    assert(dup != NULL);
    char* pdu  = dup;
    int pdulen = strlen(pdu);
    RIL_CDMA_SMS_Message *cdma_sms = NULL;
    unsigned char* bytePdu = NULL;

    cdma_sms = (RIL_CDMA_SMS_Message *)alloca(sizeof(RIL_CDMA_SMS_Message));
    assert(cdma_sms != NULL);
    memset(cdma_sms, 0, sizeof(RIL_CDMA_SMS_Message));

    // must remove the 2 quotes char
    // 2 "
    if (pdu[0] == 0x22 && pdu[pdulen-1] == 0x22) {
        pdu[pdulen -1] = '\0';
        pdu++;
        pdulen -= 2;
    }

    bytePdu = (unsigned char*)alloca(pdulen / 2);
    assert(bytePdu != NULL);
    memset(bytePdu, 0, pdulen / 2);

    ConvertByte((unsigned char *)pdu, bytePdu, pdulen / 2);

    ProcessCdmaIncomingSms(bytePdu, pdulen / 2, cdma_sms, &sReplyOptSeqNo, ackAddress);

    RIL_onUnsolicitedResponse (RIL_UNSOL_RESPONSE_CDMA_NEW_SMS,
                cdma_sms, sizeof(RIL_CDMA_SMS_Message));
    free(dup);
}

static void requestGetSmsSimMemStatus(void *data, size_t datalen, RIL_Token t)
{
    int memStatu[2] = {0};
    SMS_UNUSED(data);
    SMS_UNUSED(datalen);
    if(!getSmsSimMemStatus(&memStatu[1], &memStatu[0]))
    {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, memStatu, sizeof(memStatu));
    }
    else
    {
        RIL_onRequestComplete(t, RIL_E_SYSTEM_ERR, NULL, 0);
    }
}

static void requesCdmatSMSAcknowledge(void *data, size_t datalen, RIL_Token t)
{
    int err = 0;
    ATResponse *p_response = NULL;
    RIL_CDMA_SMS_Ack* p_message = (RIL_CDMA_SMS_Ack*) data;
    char* cmd = NULL;
    char* s_pdu = NULL;
    int len = 0;
    SMS_UNUSED(datalen);
    if (sReplyOptSeqNo == -1) {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        return;
    }

    /* funcs malloced memory for s_pdu should free at end of these piece of code */
    err = RILEncodeCdmaSmsAckPdu(p_message, sReplyOptSeqNo, ackAddress, &s_pdu, &len);
    sReplyOptSeqNo = -1;
    if(err < 0) {
        LOGD("RILEncodeCdmaSmsAckPdu() err = %d", err);
        goto error;
    }

    at_send_command("at+cmgf = 0", NULL, SMS_CHANNEL_CTX);

    asprintf(&cmd, "AT+CNMA=%d, \"%s\"", len, s_pdu);
    if (cmd == NULL) {
        goto error;
    }
    err = at_send_command(cmd, &p_response, SMS_CHANNEL_CTX);
    free(cmd);
    if (s_pdu != NULL) {
        free(s_pdu);
        s_pdu = NULL;
    }

    if (err < 0 || p_response == NULL || p_response->success == 0) {
        LOGD("requesCdmatSMSAcknowledge() response err = %d", err);
        goto error;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    at_response_free(p_response);

    return;
error:
    if (s_pdu != NULL) {
        free(s_pdu);
    }
    RIL_onRequestComplete(t, RIL_E_SYSTEM_ERR, NULL, 0);
    at_response_free(p_response);
    return;
}


static int getSmsSimMemStatus(int *totalMem, int *usedMem)
{
    int err = 0;
    ATResponse *p_response = NULL;
    ATResponse *p_sim_response = NULL;
    char *line = NULL;
    char * sim_line = NULL;
    char *mem[3] = {NULL, NULL, NULL};
    int skip_int = 0;
    char * skip_str = NULL;
    int i = 0;
    char* cmd = NULL;
    int simStatusChanged = 0;

    if (NULL == usedMem || NULL == totalMem) {
        LOGD("param error!");
        return -1;
    }

    /* get initial info */
    err = at_send_command_singleline("AT+CPMS?", "+CPMS:", &p_response, getChannelCtxbyProxy());
    if ((err < 0) || (p_response == NULL) || (p_response->success == 0)) {
        err = -1;
        goto end;
    }
    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    if (err < 0) {
        goto end;
    }
    for(i = 0; i< 3; i++) {
        err = at_tok_nextstr(&line, &mem[i]);
        if (err < 0) {
            goto end;
        }
        err = at_tok_nextint(&line, &skip_int);
        if (err < 0) {
            goto end;
        }
        err = at_tok_nextint(&line, &skip_int);
        if (err < 0) {
            goto end;
        }
    }

    /* get sim info */
    at_send_command("AT+CPMS=\"SM\"", NULL, getChannelCtxbyProxy());

    simStatusChanged = 1;

    err = at_send_command_singleline ("AT+CPMS?", "+CPMS:", &p_sim_response,
            getChannelCtxbyProxy());
    if ((err < 0) || (p_sim_response == NULL) || (p_sim_response->success == 0)) {
        err = -1;
        goto end;
    }
    sim_line = p_sim_response->p_intermediates->line;
    err = at_tok_start(&sim_line);
    if (err < 0) {
        goto end;
    }
    err = at_tok_nextstr(&sim_line, &skip_str);
    if (err < 0) {
        goto end;
    }
    err = at_tok_nextint(&sim_line, &skip_int);
    if (err < 0) {
        goto end;
    }
    *usedMem = skip_int;
    err = at_tok_nextint(&sim_line, &skip_int);
    if (err < 0) {
        goto end;
    }
    *totalMem = skip_int;
    LOGD("uim used is %d, total is %d", *usedMem, *totalMem);

end:
    /* recover initial setting */
    if (simStatusChanged) {
        asprintf(&cmd, "AT+CPMS=\"%s\",\"%s\",\"%s\"", mem[0], mem[1], mem[2]);
        if (cmd != NULL) {
            at_send_command(cmd, NULL, getChannelCtxbyProxy());
            free(cmd);
            cmd = NULL;
        }
    }

    at_response_free(p_sim_response);
    at_response_free(p_response);
    return err;
}


static void requestReadDelNewSms(void* param)
{
    NewMsgStorageStruct *pNewMsgStorage = (NewMsgStorageStruct*)(param);
    char* cmd1 = NULL;
    char* cmd2 = NULL;
    char* cmd3 = NULL;

    if (NULL != pNewMsgStorage) {
      asprintf(&cmd1, "AT+CPMS=\"%s\"", pNewMsgStorage->sms_storeType);
      if (cmd1 != NULL) {
          at_send_command(cmd1, NULL, getChannelCtxbyProxy());
          free(cmd1);
      }

      asprintf(&cmd2, "AT+CMGR=%d", pNewMsgStorage->sms_index);
      if (cmd2 != NULL) {
          at_send_command(cmd2, NULL, getChannelCtxbyProxy());
          free(cmd2);
      }

      asprintf(&cmd3, "AT+CMGD=%d", pNewMsgStorage->sms_index);
      if (cmd3 != NULL) {
          at_send_command(cmd3, NULL, getChannelCtxbyProxy());
          free(cmd3);
      }

      at_send_command("AT+CPMS=\"ME\"", NULL, getChannelCtxbyProxy());
      free(pNewMsgStorage);
    }
}


static void requestReportSmsMemoryStatus(void *data, size_t datalen, RIL_Token t) {
    int err;
    int status;
    SMS_UNUSED(datalen);
    status = ((int *)data)[0];
    LOGD("requestReportSmsMemoryStatus, status = %d", status);
    if (status == 1) {
        err = at_send_command("AT+VMEFL=0", NULL, SMS_CHANNEL_CTX);
    } else if (status == 0) {
        err = at_send_command("AT+VMEFL=1", NULL, SMS_CHANNEL_CTX);
    } else {
        goto error;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;

error:
    RIL_onRequestComplete(t, RIL_E_SYSTEM_ERR, NULL, 0);
}


/*
 * Because we use +CMTI for new CDMA sms reporting, when ril received the CMTI URC,
 * it will send CMGR and CMGD on receiving it, to read and delete this sms.
 * One word, the reading process is asynchronous. If the modem have reported the CMTI,
 * but some exceptions appeared before it send CMGR to read this sms, the modem sms space
 * will be occupyed forever.
 * So, in order to avoid this condition, we read all unread sms in CBP sms memory when
 * device is powered on, and delete them.
 *
 * NOTE: when we changed to use the +CMT to report new smss, we maybe not do this work,
 * for the +CMT is synchronous processing.
 *
 */
static void readAllUnreadSmsOnME(void) {
    int err = 0;
    ATResponse *p_response = NULL;
    ATLine *p_cur = NULL;
    char* cmd = NULL;
    int i = 0;
    int indexCount = 0;
    int indexTemp = -1;
    int indexs[60];

    LOGD("to readAllUnreadSmsOnME");

    err = at_send_command_multiline ("AT^HCMGL=0", "^HCMGL:", &p_response,
            getChannelCtxbyProxy());
    if ((err < 0) || (p_response == NULL) || (p_response->success == 0))
    {
        LOGD("readAllUnreadSmsOnME error, err = %d", err);
        goto end;
    }

    for ( p_cur = p_response->p_intermediates;
          p_cur != NULL;
          p_cur = p_cur->p_next) {
        char *line = p_cur->line;
        err = at_tok_start(&line);
        if (err < 0)
            goto end;

        err = at_tok_nextint(&line, &indexTemp);
        if (err < 0)
            goto end;

        if (indexTemp >= 0 && indexTemp < 60) {
            indexs[indexCount] = indexTemp;
            indexCount++;
        }
        indexTemp = -1;
    }

    LOGD("indexCount = %d", indexCount);

    for (i = 0; i < indexCount; i++) {
        LOGD("index[%d] = %d", i, indexs[i]);
        at_send_command("AT+CPMS=\"ME\"", NULL, getChannelCtxbyProxy());

        asprintf(&cmd, "AT+CMGR=%d", indexs[i]);
        if (cmd == NULL) {
            break;
        }
        at_send_command(cmd, NULL, getChannelCtxbyProxy());
        free(cmd);
        cmd = NULL;
        asprintf(&cmd, "AT+CMGD=%d", indexs[i]);
        if (cmd == NULL) {
            break;
        }
        at_send_command(cmd, NULL, getChannelCtxbyProxy());
        free(cmd);
        cmd = NULL;
    }

end:
    at_response_free(p_response);
}


static void requestCdmaSmsBroadcastActivation(void *data, size_t datalen, RIL_Token t) {
    int activation;
    int err;
    char* line = NULL;
    ATResponse *p_response = NULL;
    RIL_Errno ril_errno = RIL_E_SUCCESS;
    activation = ((int *)data)[0];
    SMS_UNUSED(datalen);
    LOGD("requestCdmaSmsBroadcastActivation: %d", activation);
    activation = (activation == 0) ? 1 : 0;
    asprintf(&line, "AT+ECSCB=%d", activation);
    if (line != NULL) {
        err = at_send_command(line, &p_response, SMS_CHANNEL_CTX);
        free(line);
        if (err < 0 || p_response == NULL || p_response->success <= 0) {
            ril_errno = RIL_E_SYSTEM_ERR;
        }
        RIL_onRequestComplete(t, ril_errno, NULL, 0);
        at_response_free(p_response);
    }
    return;
}


static int queryCdmaBroadcastActivation(int *mode, RIL_Token t) {
    ATResponse *p_response = NULL;
    int err = 0;
    char *line = NULL;
    err = at_send_command_singleline("AT+ECSCB?", "+ECSCB:", &p_response,
            SMS_CHANNEL_CTX);
    if (err < 0 || p_response == NULL || p_response->success == 0) {
        err = -1;
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if(err < 0) {
        goto error;
    }

    err = at_tok_nextint(&line, mode);
    if(err < 0 || (*mode) < 0 || (*mode) > 1) {
        goto error;
    }
error:
    if (p_response != NULL) {
        at_response_free(p_response);
    }
    return err;
}


static int queryCdmaBroadcastConfig(char *cmd, char* responsePrefix,
        int *mode, char **config, RIL_Token t) {

    ATResponse *p_response = NULL;
    int err = 0;
    char *line = NULL;
    char *rangeString = NULL;
    *config = NULL;
    err = at_send_command_singleline(cmd, responsePrefix, &p_response,
            SMS_CHANNEL_CTX);
    if (err < 0 || p_response == NULL || p_response->success == 0) {
        err = -1;
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if(err < 0) {
        goto error;
    }

    err = at_tok_nextint(&line, mode);
    if(err < 0 || (*mode) < 0 || (*mode) > 1) {
        goto error;
    }

    if (*mode == 1) {
        err = at_tok_nextstr(&line, &rangeString);
        if(err < 0) {
            goto error;
        }

    } else {
        rangeString = "\"0-0\"";
    }
    asprintf(config, "%s", rangeString);
error:
    if (p_response != NULL) {
        at_response_free(p_response);
    }
    return err;
}


static int queryCdmaSmsBroadcastCatConfig(int *mode, char **category, RIL_Token t) {
    return queryCdmaBroadcastConfig("AT+ECSCBCHA?", "+ECSCBCHA:", mode, category, t);
}


static int queryCdmaSmsBroadcastLanConfig(int *mode, char **language, RIL_Token t) {
    return queryCdmaBroadcastConfig("AT+ECSCBLAN?", "+ECSCBLAN:", mode, language, t);
}

static int disableRange(int mode, char *rangeString, char* cmd, RIL_Token t) {
    ATResponse *response = NULL;
    int err = 0;
    if (mode == 1) {
        Range ranges[MAX_RANGE];
        memset(ranges, 0, sizeof(ranges));
        int num = 0;
        int i = 0;
        num = getRangeFromModem(rangeString, ranges);
        for (i = 0; i < num; i++) {
            char* line = NULL;
            asprintf(&line, cmd, ranges[i].start, ranges[i].end);
            if (line == NULL) {
                break;
            }
            err = at_send_command(line, &response, SMS_CHANNEL_CTX);
            free(line);
            if ((response != NULL) && (response->success <= 0)) {
                err = -1;
            }
            if (response != NULL) {
                at_response_free(response);
            }
            if (err < 0) {
                break;
            }
        }
    }
    return err;
}


static int disableAllCategory(RIL_Token t) {
    int mode;
    char *categorys = NULL;
    int err;

    err = queryCdmaSmsBroadcastCatConfig(&mode, &categorys, t);
    if (err < 0) {
        goto error;
    }
    disableRange(mode, categorys, "AT+ECSCBCHA=0,\"%d-%d\"", t);
error:
    if (categorys != NULL) {
        free(categorys);
    }
    return err;
}


static int disableAllLanguage(RIL_Token t) {
    int mode;
    char *languages = NULL;
    int err;

    err = queryCdmaSmsBroadcastLanConfig(&mode, &languages, t);
    if (err < 0) {
        goto error;
    }
    disableRange(mode, languages, "AT+ECSCBLAN=0,\"%d-%d\"", t);
error:
    if (languages != NULL) {
        free(languages);
    }
    return err;
}


static int setRanges(char* cmd, int* config, int count, int selected, RIL_Token t) {
    int num = 0;
    int i;
    Range *r = NULL;
    ATResponse *response = NULL;
    int err = -1;
    r = (Range *)malloc(count * sizeof(Range));
    if (r == NULL) {
        return err;
    }
    num = getRange(config, count, r);
    for (i = 0; i < num; i++) {
        char* line = NULL;
        asprintf(&line, cmd, selected, r[i].start, r[i].end);
        if (line == NULL) {
            break;
        }
        err = at_send_command(line, &response, SMS_CHANNEL_CTX);
        free(line);
        if ((response != NULL) && (response->success <= 0)) {
            err = -1;
        }
        if (response != NULL) {
            at_response_free(response);
        }
        if (err < 0) {
            break;
        }
    }
    free(r);
    return err;
}


static int setCategorys(int *category, int count, int selected, RIL_Token t) {
    return setRanges("AT+ECSCBCHA=%d,\"%d-%d\"", category, count, selected, t);
}


static int setLanguages(int *language, int count, int selected, RIL_Token t) {
    return setRanges("AT+ECSCBLAN=%d,\"%d-%d\"", language, count, selected, t);
}


static int sortCategoryAndLanguage(
        RIL_CDMA_BroadcastSmsConfigInfo **infos,
        int *category,
        int *language,
        int count,
        int selected) {
    int i;
    for (i = 0; i < count; i++) {
        LOGD("category = %d, language = %d, selected = %d",
            infos[i]->service_category,
            infos[i]->language,
            infos[i]->selected);
        if (selected != infos[i]->selected) {
            LOGE("Don't support different selected in the broadcast config array");
            return -1;
        }
        category[i] = infos[i]->service_category;
        language[i] = infos[i]->language;
    }
    sort(category, count);
    sort(language, count);
    return 0;
}


static void requestCdmaSetBroadcastSmsConfig(void *data, size_t datalen, RIL_Token t) {

    RIL_CDMA_BroadcastSmsConfigInfo **infos = (RIL_CDMA_BroadcastSmsConfigInfo **)data;
    int count = datalen / sizeof(RIL_CDMA_BroadcastSmsConfigInfo*);
    RIL_Errno ril_errno = RIL_E_SYSTEM_ERR;
    int *category = NULL;
    int *language = NULL;
    int selected;
    int err;

    category = (int *)malloc(count * sizeof(int));
    if (category == NULL) {
        goto error;
    }
    language = (int *)malloc(count * sizeof(int));
    if (language == NULL) {
        goto error;
    }
    selected = infos[0]->selected;
    LOGD("requestCdmaSetBroadcastSmsConfig: count = %d", count);
    err = sortCategoryAndLanguage(infos, category, language, count, selected);
    if (err < 0) {
        goto error;
    }

    if (selected == 1) {
        err = disableAllCategory(t);
        if (err < 0) {
            goto error;
        }
        err = disableAllLanguage(t);
        if (err < 0) {
            goto error;
        }
    }

    err = setCategorys(category, count, selected, t);
    if (err < 0) {
        goto error;
    }

    err = setLanguages(language, count, selected, t);
    if (err < 0) {
        goto error;
    }

    ril_errno = RIL_E_SUCCESS;
error:
    if (category != NULL) {
        free(category);
    }
    if (language != NULL) {
        free(language);
    }
    RIL_onRequestComplete(t, ril_errno, NULL, 0);
    return;
}


static int getConfig(RIL_CDMA_BroadcastSmsConfigInfo **infos, int *count,
        int startCheck, int endCheck, char *cmd, char* responsePrefix, RIL_Token t) {

    char *rangeString = NULL;
    Range ranges[MAX_RANGE];
    int num = 0;
    int i = 0;
    int err = 0;
    int mode = 0;

    *count = 0;
    memset(ranges, 0, sizeof(ranges));
    err = queryCdmaBroadcastConfig(cmd, responsePrefix, &mode, &rangeString, t);
    if (mode == 1) {
        num = getRangeFromModem(rangeString, ranges);
        for (i = 0; i < num; i++) {
            int j;
            if (ranges[i].start < startCheck || ranges[i].end > endCheck) {
                continue;
            }
            for (j = ranges[i].start; j <= ranges[i].end; j++) {
                if (infos != NULL) {
                    RIL_CDMA_BroadcastSmsConfigInfo *info = (RIL_CDMA_BroadcastSmsConfigInfo *)
                            malloc(sizeof(RIL_CDMA_BroadcastSmsConfigInfo));
                    if (info != NULL) {
                        if (strncmp("AT+ECSCBCHA", cmd, strlen("AT+ECSCBCHA")) == 0) {
                            info->service_category = j;
                            info->language = 0;
                        } else {
                            info->service_category = 0;
                            info->language = j;
                        }
                        info->selected = 1;
                        infos[*count] = info;
                    } else {
                        int k;
                        for (k = 0; k < *count; k++) {
                            free(infos[k]);
                        }
                        return -1;
                    }
                }
                (*count)++;
            }
        }
    }
    if (rangeString != NULL) {
        free(rangeString);
    }
    return err;
}


static int getCdmaBroadcastCategoryConfig(RIL_CDMA_BroadcastSmsConfigInfo **infos,
        int *count, RIL_Token t) {
    return getConfig(infos, count, 0, 0xFFFF, "AT+ECSCBCHA?", "+ECSCBCHA:", t);
}


static int getCdmaBroadcastLanguageConfig(RIL_CDMA_BroadcastSmsConfigInfo **infos,
        int *count, RIL_Token t) {
    return getConfig(infos, count, 0, 7, "AT+ECSCBLAN?", "+ECSCBLAN:", t);
}

static void requestCdmaGetBroadcastSmsConfig(void *data, size_t datalen, RIL_Token t) {
    RIL_CDMA_BroadcastSmsConfigInfo **infos = NULL;
    int count = 0;
    int err = 0;
    int mode = 0;
    int categoryCount = 0;
    int languageCount = 0;
    int i = 0;

    RIL_Errno ril_errno = RIL_E_SYSTEM_ERR;

    SMS_UNUSED(data);
    SMS_UNUSED(datalen);
    err = queryCdmaBroadcastActivation(&mode, t);
    if (err < 0) {
        goto error;
    }
    if (mode != 0) {
        err = getCdmaBroadcastCategoryConfig(NULL, &categoryCount, t);
        if (err < 0) {
            goto error;
        }
        err = getCdmaBroadcastLanguageConfig(NULL, &languageCount, t);
        if (err < 0) {
            goto error;
        }
        count = categoryCount + languageCount;
        if (count != 0) {
            infos = (RIL_CDMA_BroadcastSmsConfigInfo **)
                    malloc(sizeof(RIL_CDMA_BroadcastSmsConfigInfo*) * count);
            if (infos == NULL) {
                goto error;
            }
            err = getCdmaBroadcastCategoryConfig(infos, &categoryCount, t);
            if (err < 0) {
                free(infos);
                goto error;
            }
            err = getCdmaBroadcastLanguageConfig(&infos[categoryCount], &languageCount, t);
            if (err < 0) {
                for (i = 0; i < categoryCount; i++) {
                    free(infos[i]);
                }
                free(infos);
                goto error;
            }
        }
    }
    if (mode == 0 || count == 0)  {
        RIL_CDMA_BroadcastSmsConfigInfo *info = NULL;
        infos = (RIL_CDMA_BroadcastSmsConfigInfo **)
                malloc(sizeof(RIL_CDMA_BroadcastSmsConfigInfo*) * 1);
        if (infos == NULL) {
            goto error;
        }
        info = (RIL_CDMA_BroadcastSmsConfigInfo *)malloc(sizeof(RIL_CDMA_BroadcastSmsConfigInfo));
        if (info == NULL) {
            free(infos);
            goto error;
        }
        infos[0] = info;
        info->service_category = 0;
        info->language = 0;
        info->selected = 0;
        count = 1;
    }
    RIL_onRequestComplete(t, RIL_E_SUCCESS, infos, sizeof(RIL_CDMA_BroadcastSmsConfigInfo*)*count);
    for (i = 0; i < count; i++) {
        free(infos[i]);
    }
    free(infos);
    return;

error:

    RIL_onRequestComplete(t, ril_errno, NULL, 0);
    return;
}
