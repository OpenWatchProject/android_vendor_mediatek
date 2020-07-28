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
#include "RtstMRil.h"
#include "RtstEnv.h"
#include "RtstData.h"
#include "RtstUtils.h"
#include "RtstLog.h"
#include "atchannel.h"

/*****************************************************************************
 * Define
 *****************************************************************************/
#define TAG "RTF"

/*****************************************************************************
 * Local Function
 *****************************************************************************/
extern "C"
void onRequestComplete(
        RIL_Token t, RIL_Errno e, void *response, size_t responselen) {
    RequestInfo *pRI = (RequestInfo *)t;
    RIL_SOCKET_ID socket_id = pRI->socket_id;
    int32_t id = pRI->pCI->requestNumber;
    RTST_LOG_E(TAG, "onRequestComplete id = %d, slot = %d, e = %d, responselen = %zu, response = %p",
        id, socket_id, e, responselen, response);
    Parcel p;
    p.writeInt32(id);
    p.writeInt32(e);
    if (response != NULL) {
        pRI->pCI->responseFunction(p, response, responselen);
    }
    RtstEnv::get()->getRilSocket2((int)socket_id)
            .write((void *)p.data(), p.dataSize());
}


extern "C"
#if defined(ANDROID_MULTI_SIM)
void onUnsolicitedResponse(
        int unsolResponse, const void *data,
        size_t datalen, RIL_SOCKET_ID socket_id) {
#else
void onUnsolicitedResponse(
        int unsolResponse, const void *data, size_t datalen) {
    RIL_SOCKET_ID socket_id = (RIL_SOCKET_ID)0;
#endif
    Parcel p;
    p.writeInt32(unsolResponse);
    RTST_LOG_D(TAG, "onUnsolicitedResponse id = %d, slot = %d, datalen = %zu, %p", unsolResponse,
        socket_id, datalen, data);
    UnsolResponseInfo *pURI = RtstGRil::getUrcInfo(unsolResponse);
    if (pURI == NULL) {
        AssertionFailure() << "String value mismatch!"
                                  << " Expected: UrcInfo is NULL";
        return;
    }
    if (data != NULL) {
        pURI->responseFunction(p, (void *)data, datalen);
    }
    RtstEnv::get()->getRilSocket2((int)socket_id)
            .write((void *)p.data(), p.dataSize());
}

static void handleFinalResponse(const char *line, RILChannelCtx *p_channel)
{
    ATResponse *p_response = p_channel->p_response;
    p_response->finalResponse = strdup(line);
}

static const char * s_finalResponsesSuccess[] = {
    "OK",
    "CONNECT"
};

static int strStartsWith(const char *line, const char *prefix)
{
    if(line == NULL || prefix == NULL)
    {
        return 0;
    }
    for( ; *line != '\0' && *prefix != '\0' ; line++, prefix++)
    {
        if(tolower(*line) != tolower(*prefix))
        {
            return 0;
        }
    }

    return *prefix == '\0';
}

static void addIntermediate(const char *line, RILChannelCtx *p_channel)
{
    ATResponse *p_response = p_channel->p_response;

    ATLine *p_new;
    p_new = (ATLine *) malloc(sizeof(ATLine));
    assert(p_new != NULL);
    p_new->line = strdup(line);

    /* note: this adds to the head of the list, so the list
     * will be in reverse order of lines received. the order is flipped
     * again before passing on to the command issuer */
    p_new->p_next = p_response->p_intermediates;
    p_response->p_intermediates = p_new;
}

static int isFinalResponseSuccess(const char *line)
{
    size_t i;

    for (i = 0 ; i < NUM_ELEMS(s_finalResponsesSuccess) ; i++) {
        if (strStartsWith(line, s_finalResponsesSuccess[i])) {
            return 1;
        }
    }

    return 0;
}

static const char *s_finalResponsesError[] = {
    "ERROR",
    "+CMS ERROR:",
    "+CME ERROR:",
    "NO CARRIER", /* sometimes! */
    "NO ANSWER",
    "NO DIALTONE",
    "BUSY" };

static int isFinalResponseError(const char *line, RILChannelCtx *p_channel)
{
    size_t i;
    for (i = 0; i < NUM_ELEMS(s_finalResponsesError); i++) {
        if (strStartsWith(p_channel->p_response->current_cmd, "ATD") == 0
                && strStartsWith(p_channel->p_response->current_cmd, "AT+CDV=") == 0
                && strStartsWith(line, "NO CARRIER"))
            continue;

        if (strStartsWith(line, s_finalResponsesError[i])) {
            return 1;
        }
    }

    return 0;
}

static ATResponse * at_response_new()
{
    ATResponse *r = (ATResponse *) calloc(1, sizeof(ATResponse));
    assert(r != NULL);
    return r;
}

void at_response_free(ATResponse *p_response)
{
    ATLine *p_line;

    if (p_response == NULL) return;

    p_line = p_response->p_intermediates;

    while (p_line != NULL) {
        ATLine *p_toFree;

        p_toFree = p_line;
        p_line = p_line->p_next;

        free(p_toFree->line);
        free(p_toFree);
    }

    free (p_response->finalResponse);
    free (p_response);
}

static void reverseIntermediates(ATResponse *p_response)
{
    ATLine *pcur, *pnext;

    pcur = p_response->p_intermediates;
    p_response->p_intermediates = NULL;

    while (pcur != NULL) {
        pnext = pcur->p_next;
        pcur->p_next = p_response->p_intermediates;
        p_response->p_intermediates = pcur;
        pcur = pnext;
    }
}

extern "C" int onAtSendCommand(
    const char *command, ATCommandType type,
    const char *responsePrefix,
    const char *smspdu,
    long long timeoutMsec,
    ATResponse **pp_outResponse,
    RILChannelCtx *p_channel,
    RIL_Token ackToken) {
    RTST_UNUSED(timeoutMsec);
    RTST_UNUSED(ackToken);
    int channelId = p_channel->id;
    int slotId = p_channel->fd; // will use the fd to store the slot id in test
    char buf[1024];
    String8 cmd = String8(command) + String8("\r");
    RtstEnv::get()->getAtSocket2(channelId, slotId).write((void *)cmd.string(), cmd.length());
    p_channel->type = type;
    p_channel->responsePrefix = responsePrefix;
    p_channel->smsPDU = smspdu;
    p_channel->p_response = at_response_new();
    p_channel->p_response->current_cmd = command;
    ATResponse *p_response = p_channel->p_response;
    p_response->p_intermediates = NULL;
    int len;
    RTST_LOG_D(TAG, "C2K_AT > %s\n", command);
    while ((len = RtstUtils::readLine(
        RtstEnv::get()->getAtSocket2(channelId, slotId), buf, 1024)) > 0) {
        RTST_LOG_D(TAG, "C2K_AT < %s\n", buf);
        if (isFinalResponseSuccess(buf)) {
             handleFinalResponse(buf, p_channel);
             p_response->success = 1;
             break;
        } else if (strStartsWith(buf, "^HCMGSS:") || strStartsWith(buf, "^HCMGSF:")) {
             handleFinalResponse(buf, p_channel);
             p_response->success = 1;
             break;
        } else if (isFinalResponseError(buf, p_channel)) {
            p_response->success = 0;
            RTST_LOG_D(TAG, "Final Response Error %s", buf);
            handleFinalResponse(buf, p_channel);
            break;
        }
        switch (type) {
            case NO_RESULT:
                break;
            case SINGLELINE:
                if (p_response->p_intermediates == NULL
                        && strStartsWith(buf, p_channel->responsePrefix)) {
                    addIntermediate(buf, p_channel);
                }
                break;
            case MULTILINE:
                if (strStartsWith(buf, p_channel->responsePrefix)) {
                    addIntermediate(buf, p_channel);
                }
                break;
            case NUMERIC:
                break;
            case RAW:
                break;
        }
    }
    if (pp_outResponse == NULL) {
        at_response_free(p_channel->p_response);
    } else {
        /* line reader stores intermediate responses in reverse order */
        reverseIntermediates(p_channel->p_response);
        *pp_outResponse = p_channel->p_response;
    }

    p_channel->p_response = NULL;
    return 0;
}


extern "C" RILChannelId onQueryMyChannelId(RIL_Token t) {
    return ((RequestInfo *)t)->cid;
}


/*****************************************************************************
 * Structure
 *****************************************************************************/
extern "C"  {

static struct RIL_Env s_rtstRilEnv = {
    onRequestComplete,
    onUnsolicitedResponse,
    NULL,
    NULL,
    NULL,
    onQueryMyChannelId,
    NULL
};

}

/*****************************************************************************
 * External Functions Declaration
 *****************************************************************************/
extern "C"  void RIL_setRilEnvForGT(const struct RIL_Env *env);

/*****************************************************************************
 * class RtstMRil
 *****************************************************************************/
void RtstMRil::setEmulatorMode() {
    //RfxRilUtils::setRilRunMode(RilRunMode::RIL_RUN_MODE_MOCK);
}

void RtstMRil::setRilEnv() {
    RIL_setRilEnvForGT(&s_rtstRilEnv);
}

void RtstMRil::setAtHooker() {
    at_set_hook(onAtSendCommand);
}